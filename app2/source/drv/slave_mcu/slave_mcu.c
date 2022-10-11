/**
  ******************************************************************************
  * @file    slave_mcu.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  从机数据收发
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#include "include_all.h"

SLAVE_MCU_RECIVE_STRUCT slave_r; //从机读
SLAVE_MANAGE_SEND_STRUCT	sm; //从机写

/*
底层驱动函数
*/
/****************************************************************************
* 名称:    serial_send_recive_data ()
* 功能：SPI1从机发送一个字节并接收一个字节
* 入口参数：无                         
* 出口参数：无
****************************************************************************/	
static u8 serial_send_recive_data(u8 send_data)
{
	u8 temp;
    
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)==RESET); //等待上一包数据发送完成
	SPI_I2S_SendData(SPI1, send_data);        //开始向发送缓冲器中装载数据

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)==RESET); //等待一包数据接收完成
	temp=SPI_I2S_ReceiveData(SPI1);          //取出接收到的数据
    
	return temp; //返回读出的数据
}

/*****************************************************************************
**                 查询从机写数据完成状态
**
**函数返回值   为TRUE时表示忙状态
**函数返回值   为FALSE时表示空闲状态
*****************************************************************************/
bool slave_mcu_is_ready(void)
{
	u8 temp;
    
	EN_SLAVE_MCU_CE;
    
	serial_send_recive_data(serial_comm_rdsr);
	temp=serial_send_recive_data(0x00);
    
	DIS_SLAVE_MCU_CE;

	if ((temp&0x01)==0)  //芯片已经不忙
		return  true;
	else
		return  false;
}


/*****************************************************************************
**     读取从机数据
**     返回数据长度及内容
*****************************************************************************/
s16 slave_mcu_read(u8 *buffer)
{
	u16  i;
	s16  lgth;

	if(buffer == NULL)
		return false;

	EN_SLAVE_MCU_CE;

	serial_send_recive_data(serial_comm_read); //发送读数据命令
	delay_ms(3);

	lgth = 0;
	lgth = serial_send_recive_data(0x00); //先获取长度字节的高8位
	lgth = (lgth&0x00ff)<<8;
	lgth += serial_send_recive_data(0x00); //再获取长度字节的低8位

	if(lgth==0||lgth>1024)
		return -1;
	
	for (i=0;i<lgth;i++)
	{
		*buffer=serial_send_recive_data(0x00);
		buffer++;	
	}

	DIS_SLAVE_MCU_CE;

	return  lgth;
}


/*****************************************************************************
**     向从机写入数据(即向从机发送数据)
**函数返回值   为TRUE时表示忙状态
**函数返回值   为FALSE时表示写成功
*****************************************************************************/
bool slave_mcu_write(u8 *buffer, u16 len)
{
	u16 i;
	bool ret = false;

	if(buffer==NULL||len==0||len>1024)
		return false;

	for(i=0;i<12;i++)
	{
		ret = slave_mcu_is_ready();
		if(ret == false){
			if(i >10)
				return false;
			else
				delay_ms(10);
		}else
			break;
	}
	
	EN_SLAVE_MCU_CE;
    
	serial_send_recive_data(serial_comm_write);
	serial_send_recive_data((u8)((len>>8)&0x0ff)); //先发送长度的高字节
	serial_send_recive_data((u8)((len)&0x0ff));  //再发送长度的低字节
	delay_ms(3);
    
	for (i=0;i<len;i++)
	{
		serial_send_recive_data(*buffer);
		buffer++;	
	}
    
	DIS_SLAVE_MCU_CE;

	return  true;
}

/****************************************************************************
* 名称:    recive_slave_task ()
* 功能：接收任务
* 入口参数：无                         
* 出口参数：无
****************************************************************************/	
void recive_slave_task(void)
{
	typedef enum{
		E_SLAVE_IDLE, 
		E_SLAVE_RECV, 
		E_SLAVE_ERR	
	}E_SLAVE_RECIVE_STEP;
	
	E_SLAVE_RECIVE_STEP old_step = E_SLAVE_ERR;
	static E_SLAVE_RECIVE_STEP  step=E_SLAVE_IDLE;
	static u32 state_jif =0;

	bool ret = false;
	u8 ret1 = false;
	u8 xvalue;
	u16 i;
	
	static bool new_step = false;
	static u32 ot_jif = 0;
	static u16 delay_cnt;

	u8 r_buf[LEN_SLAVE_BUF];
	s16 r_len = 0;

	old_step = step;
	switch(step){
		case E_SLAVE_IDLE:
			if(slave_r.new_info){
				step = E_SLAVE_RECV;
				break;
			}

			if(PIN_SLV_READ == LOW)
			{
				if(++delay_cnt >= 3)
				{
					slave_r.new_info = true;
					delay_cnt = 0;
					step = E_SLAVE_RECV;
					break;
				}					
			}
			else
			{
				delay_cnt = 0;
			}

													
			break;
		case E_SLAVE_RECV:
			if(new_step)
				ot_jif = jiffies;

			if(_pastn(ot_jif)>1000)
			{
				step = E_SLAVE_ERR;			
				break;
			}
				
			ret = slave_mcu_is_ready();
			if(ret==false)
				break;

			_memset(r_buf, 0x00, LEN_SLAVE_BUF);
			r_len = slave_mcu_read(r_buf);

			slave_r.new_info = false;
			if(r_len < 5||r_len > LEN_SLAVE_BUF)
			{
				step = E_SLAVE_ERR;
				break;
			}

			xvalue = 0;
			for (i=0;i<r_len-2;i++)
				xvalue^=r_buf[i];
			
			if(r_buf[r_len-2] == xvalue)
			{
				if (r_buf[1]!=0x0d) //发送正确接收数据的应答指令到手柄
				{
					send_handset_data(FROM_SLAVE, H_LEN_N, 0, (u8*)&HAnsData, 0);
					ret1 = handset_any_parse(FROM_SLAVE, r_buf, r_len);
					if(ret1 == CMD_ERROR)
					{
						loge("handset err: slave parse");
						step = E_SLAVE_ERR;
						break;
					}		
				}
				else
				{		//不用解析调度屏的回复 

				}
				step = E_SLAVE_IDLE;
				break;
			}
			else
			{
				loge("SLV XOR ERR");
				step = E_SLAVE_ERR;
			}
			break;	
		case E_SLAVE_ERR:
			_memset((u8*)&slave_r, 0x00, sizeof(slave_r));					
			step = E_SLAVE_IDLE;
			break;
		default:
			step = E_SLAVE_IDLE;		
	}

	new_step = (old_step != step) ? true:false;

	if(_pastn(state_jif)>=3000&&_pastn(bypass_debug.jiff)>6000)
	{
		state_jif = jiffies;
		handle_send_0xa0_info(FROM_SLAVE);	
		//send_gnss_select_mode(FROM_SLAVE);	
	}

}

