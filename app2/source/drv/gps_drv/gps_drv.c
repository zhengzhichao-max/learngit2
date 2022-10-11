/**
  ******************************************************************************
  * @file    gps_drv.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GPS 串口底层驱动
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

u32 Gps_time_sec = 0;
u32 Gps_sec = 0;
u8 gps_rev_buf[G_RECE_SIZE]; //接收BUF
u8* gps_rev_head = gps_rev_buf; //接收头指针
u8* gps_rev_tail = gps_rev_buf;//接收尾指针

u8 gps_send_buf[G_SEND_SIZE]; //发送BUF
u8* gps_send_head = gps_send_buf; //发送头指针
u8* gps_send_tail = gps_send_buf; //发送尾指针

u32 gps_rev_jfs  = 0;  //接收时间，ms
u32 gps_send_jfs = 0; //发送时间，ms

bool gps_send_busy = false; //数据发送忙

s_GPSCOM  s_GpsCom;
u8  g_u8GPGSVBuff[G_PROC_SIZE];	
u8  g_u8GPRMCBuff[G_PROC_SIZE];	
u8  g_u8GPGSABuff[50];	

u8  g_u8GPRMCFlag;		
u8  g_u8GPGGAFlag;		
u8  g_u8GPGSAFlag;		



void gps_struct_init(void)
{
    _memset((u8 *)&s_GpsCom, 0, sizeof(s_GPSCOM));
    s_GpsCom.buf = gps_rev_buf;

}

/****************************************************************************
* 名称:    write_gps ()
* 功能：串口发送，带长度
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void write_gps(u8* buf, u16 len)
{
	
	#if 1
	u16 i;
	for(i = 0;i <len;i++) {
		if (over_gps_send())
			break;	
		*gps_send_head = *buf++;	
		incp_gps_send(gps_send_head,1);
	}
	#else
	  while(len)
		{
			while((USART_GetITStatus(UART5, USART_IT_TXE) != RESET));
			UART5->DR = *buf++;
			len --;
		}
	#endif
}

/****************************************************************************
* 名称:    write_gps_hex ()
* 功能：串口发送，单个字节
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void write_gps_hex(u8 hex)
{
	if (over_gps_send())
		return;
	*gps_send_head = hex;
	incp_gps_send(gps_send_head, 1);
}

/****************************************************************************
* 名称:    read_gps ()
* 功能：串口接收，单个字节
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void read_gps(u8* buf_s, u8* rbuf_ptr, u16 n)
{
	while (n--) {
		*buf_s++ = *rbuf_ptr;
		rbuf_ptr++;
	}
}

/****************************************************************************
* 名称:    gps_drv_tran_task ()
* 功能：启动数据发送
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void gps_drv_tran_task(void)
{
	if (gps_send_tail != gps_send_head && gps_send_busy == false)
	{
		gps_send_jfs = jiffies;
		gps_send_busy =true;	
		USART_ITConfig(UART5, USART_IT_TXE, ENABLE);		
	}

	if (gps_send_tail != gps_send_head && gps_send_busy == true && _pastn(gps_send_jfs)>3000){
		gps_send_busy = false; //超时，清除标志
	}	
}



