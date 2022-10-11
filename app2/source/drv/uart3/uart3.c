/**
  ******************************************************************************
  * tr9
  * 娴锋?濅覆鍙ｉ?氳?
  ******************************************************************************
*/  
#include "include_all.h"

//u8 r3_buf[R3_BUF_SIZE]; //接收缓存
//u8* r3_head = r3_buf;
//u8* r3_tail = r3_buf;

u8 t3_buf[T3_BUF_SIZE];  //发送缓存
u8* t3_head = t3_buf;
u8* t3_tail = t3_buf;

bool tr9_show   = true ;	//串口串口数据打印
bool uart3_busy = false; 	//串口数据忙
u32 jif_r3 = 0;  			//串口接收的时间
u32 jif_t3 = 0; 			//串口发送的时间



uart_manage u3_m; //串口管理

UART_MANAGE_STRUCT read_uar3; //串口读

/****************************************************************************
* 名称:    write_uart3 ()
* 功能：带长度的任意字符发送
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
void write_uart3(u8* buf, u16 len)
{
	u16 i;
	
	for(i = 0;i <len;i++) 
	{
		if (over_t3())
			break;
		
		*t3_head = *buf++;	
		incp_t3(t3_head, 1);
	}
}

/****************************************************************************
* 名称:    write_uart3 ()
* 功能：单个字符发送
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
void write_uart3_hex(u8 hex)
{
	if (over_t3())
		return;

	*t3_head = hex;
	incp_t3(t3_head, 1);
}




/****************************************************************************
* 名称:    Uart_tran_proc ()
* 功能：发送相应数据到终端
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void uart3_tran_task(void)
{
	//防错
	if (subp_t3(t3_head, t3_tail) > T3_BUF_SIZE)	
	{
		t3_head = t3_buf;
		t3_tail = t3_buf;
        loge("uart5 tran err: over");
    }

	if (t3_tail!=t3_head&&uart3_busy==false)//
	{
		jif_t3 = jiffies;
		uart3_busy = true;	
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);		
	}

	if (t3_tail!=t3_head&&uart3_busy==true&&_pastn(jif_t3)>800)
	{
		uart3_busy = false; //超时，清除标志
	}	
}

#if 0
/****************************************************************************
* 名称:    uart3_recive_task ()
* 功能：数据接收任务
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
void uart3_recive_task(void)
{
	typedef enum{
		E_UART_IDLE, //等待有新的串口数据
		E_UART_WITING, //等待外部程序解析
		E_UART_DEL, //删除已经处理的数据
		E_UART_ERR	//出现错误
	}E_UART_RECIVE_STEP;
	
	E_UART_RECIVE_STEP old_step = E_UART_ERR;
	static E_UART_RECIVE_STEP  step=E_UART_IDLE;

	static bool new_step = false;
	static u32 ot_jfs = 0; //overtime_jiffies

	u16 len_t;
	old_step = step;    ////
	switch(step){
	case E_UART_IDLE:
		if(!read_uar3.news)
		{
			if(new_r3() && _pastn(jif_r3) >= 30)
			{
				len_t = new_r3();			
				if(len_t >= (R3_BUF_SIZE-30))
				{
					step = E_UART_ERR;
					break;
				}

				read_uar3.cur_p = r3_tail;
				read_uar3.c_len = len_t;
				read_uar3.n_jfs = jiffies;				
				read_uar3.ack = false;	
				read_uar3.news = true;				
				step =  E_UART_WITING;
				break;				
			}
		}
		else
		{
			step =  E_UART_DEL;
			break;			
		}
		break;
	case E_UART_WITING:
		if(new_step)
		{
			ot_jfs = jiffies;
		}

		if(over_r3() || _pastn(ot_jfs) >= 1000)
		{
			loge("uart3_recive_task err: 1");
			step = E_UART_ERR;
			break;
		}

		if(read_uar3.ack)
		{
			step = E_UART_DEL;
			break;			
		}
		break;
	case E_UART_DEL:
		len_t = new_r3();			
		if(len_t < read_uar3.c_len) //错误，要删除的长度大于BUF中的实际长度
		{
			step = E_UART_ERR;
			break;
		}

		incp_r3(r3_tail, len_t);
		_memset((u8*)&read_uar3, 0x00, sizeof(read_uar3));
		step = E_UART_IDLE;		
		break;		
	case E_UART_ERR:
		clr_buf_r3();
		_memset((u8*)&read_uar3, 0x00, sizeof(read_uar3));
		step = E_UART_IDLE;		
		break;
	default:
		step = E_UART_IDLE;
		break;		
	}

	new_step = (old_step != step) ? true:false;
}
#endif



