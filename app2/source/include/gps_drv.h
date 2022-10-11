/**
  ******************************************************************************
  * @file    gps_drv.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GPS 串口底层驱动
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef	__GPS_DRV_H
#define	__GPS_DRV_H

#define  G_RECE_SIZE			1000
#define  G_SEND_SIZE			100
#define  G_PROC_SIZE			100

#define  GPS_TIMEOUT       10     //溢出时间*10ms


typedef	struct 
{
	u8  *buf;	
	u8  step;
	u16 uiRxdIn;		//读数指针
	u16 uiRxdOut;		//取数指针
	u32 uiTimeOut;
}s_GPSCOM;

extern u32 Gps_time_sec ;

extern u8 gps_rev_buf[];
extern u8 gps_send_buf[];
extern u8  g_u8GPGSABuff[];	

extern u8* gps_send_head;
extern u8* gps_send_tail;
extern u8  g_u8GPGSAFlag;		

extern u32 gps_rev_jfs;
extern u32 gps_send_jfs;
extern bool gps_send_busy;

extern s_GPSCOM  s_GpsCom;
extern u8  g_u8GPGSVBuff[];	
extern u8  g_u8GPRMCBuff[];	
extern u8  g_u8GPRMCFlag;		
extern u8  g_u8GPGGAFlag;		


#define incp_gps_send(ptr, n) \
	do { \
		if ((ptr += n) >= gps_send_buf + G_SEND_SIZE) \
			ptr -= G_SEND_SIZE;\
	} while (0)
#define decp_gps_send(ptr, n) \
	do { \
		if ((ptr -= n) < gps_send_buf) \
			ptr += G_SEND_SIZE;\
	} while (0)


#define over_gps_send() \
	(gps_send_head == gps_send_tail - 1 || gps_send_head == gps_send_tail + G_SEND_SIZE - 1)


extern void write_gps(u8* buf, u16 len);
extern void write_gps_hex(u8 hex);
extern void read_gps(u8* buf, u8* rbuf_ptr, u16 n);
extern void gps_drv_tran_task(void);
extern void gps_struct_init(void);
#endif	

