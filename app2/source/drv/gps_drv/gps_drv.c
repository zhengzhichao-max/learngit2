/**
  ******************************************************************************
  * @file    gps_drv.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GPS ���ڵײ�����
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

u32 Gps_time_sec = 0;
u32 Gps_sec = 0;
u8 gps_rev_buf[G_RECE_SIZE]; //����BUF
u8* gps_rev_head = gps_rev_buf; //����ͷָ��
u8* gps_rev_tail = gps_rev_buf;//����βָ��

u8 gps_send_buf[G_SEND_SIZE]; //����BUF
u8* gps_send_head = gps_send_buf; //����ͷָ��
u8* gps_send_tail = gps_send_buf; //����βָ��

u32 gps_rev_jfs  = 0;  //����ʱ�䣬ms
u32 gps_send_jfs = 0; //����ʱ�䣬ms

bool gps_send_busy = false; //���ݷ���æ

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
* ����:    write_gps ()
* ���ܣ����ڷ��ͣ�������
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    write_gps_hex ()
* ���ܣ����ڷ��ͣ������ֽ�
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
void write_gps_hex(u8 hex)
{
	if (over_gps_send())
		return;
	*gps_send_head = hex;
	incp_gps_send(gps_send_head, 1);
}

/****************************************************************************
* ����:    read_gps ()
* ���ܣ����ڽ��գ������ֽ�
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
void read_gps(u8* buf_s, u8* rbuf_ptr, u16 n)
{
	while (n--) {
		*buf_s++ = *rbuf_ptr;
		rbuf_ptr++;
	}
}

/****************************************************************************
* ����:    gps_drv_tran_task ()
* ���ܣ��������ݷ���
* ��ڲ�������                         
* ���ڲ�������
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
		gps_send_busy = false; //��ʱ�������־
	}	
}



