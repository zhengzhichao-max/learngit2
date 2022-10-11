/**
  ******************************************************************************
  * @file    gsm_drv.H 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GSM 串口底层驱动
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef	__UART2_H
#define	__UART2_H
 #define  GSM_REV_SIZE			2500
#define  GSM_SEND_SIZE			2500
#define  GPRS_SEND_SIZE			1200
#define  GPRS_REV_SIZE			1200



#define		STATUS_IDLE			0			//AT引擎处于等待状态//
#define		STATUS_RUNNING		1			//AT引擎处于运行状态//
#define		AT_EVENT_CATCHED1	2			//AT指令执行超时//
#define		AT_EVENT_CATCHED2	3			//指定的AT回应串捕捉到//
#define		AT_EVENT_CATCHED3	3			//指定的AT回应串捕捉到//

typedef struct{
	void (*Handle)(s8 *catch_str, u8 Event);		//回调函数//
	s16 timeout;									//超时时间//
	u8 *catchstr1;								    //捕捉字符串1//
	u8 *catchstr2;								    //捕捉字符串2//
	u8 *catchstr3;								    //捕捉字符串3//
	u8 exe_status;									//执行状态//
	u8 *pcatch;
}sAtEngine;


extern u8 gsm_rev_buf[];
extern u8* gsm_rev_head;
extern u8* gsm_rev_tail;

extern u8 gsm_send_buf[];
extern u8* gsm_send_head;
extern u8* gsm_send_tail;

extern u8 gprs_send_buf[]; //用于GPRS 发送数据
extern u8* gprs_send_head;
extern u8* gprs_send_tail;

extern u8 gprs_rev_buf[];
extern u8* gprs_rev_head;
extern u8* gprs_rev_tail;

extern u8 gprs2_send_buf[]; //??GPRS ????
extern u8* gprs2_send_head;
extern u8* gprs2_send_tail;

extern u8 gprs3_send_buf[]; //??GPRS ????
extern u8* gprs3_send_head;
extern u8* gprs3_send_tail;



extern bool gsm_send_busy;
extern u8  netn;   //?????  

//GSM 串口接收指针管理
#define incp_gsm_rev(ptr, n)\
	do { \
		if ((ptr += n) >= gsm_rev_buf + GSM_REV_SIZE) \
			ptr -= GSM_REV_SIZE; \
	} while (0)

#define decp_gsm_rev(ptr, n)\
	do { \
		if ((ptr -= n) < gsm_rev_buf) \
			ptr += GSM_REV_SIZE; \
	} while (0)

#define new_gsm_rev()  \
		(gsm_rev_head >= gsm_rev_tail ? gsm_rev_head - gsm_rev_tail : gsm_rev_head + GSM_REV_SIZE - gsm_rev_tail)

#define over_gsm_rev()  \
		(gsm_rev_head == gsm_rev_tail - 1 || gsm_rev_head == gsm_rev_tail + GSM_REV_SIZE - 1)

#define subp_gsm_rev(p1, p2) \
		(p1 >= p2 ? p1 - p2 : p1 + GSM_REV_SIZE - p2)

#define clr_gsm_rev()\
		(gsm_rev_tail = gsm_rev_head)

#define reduce_gsm_rev() \
	do { \
		if (subp_gsm_rev(gsm_rev_head, gsm_rev_tail) > 20) \
			incp_gsm_rev(gsm_rev_tail, 5);\
	} while (0)

//GSM 串口发送指针管理		
#define incp_gsm_send(ptr, n)\
	do { \
		if ((ptr += n) >= gsm_send_buf + GSM_SEND_SIZE) \
			ptr -= GSM_SEND_SIZE; \
	} while (0)

#define decp_gsm_send(ptr, n) \
	do { \
		if ((ptr -= n) < gsm_send_buf) \
			ptr += GSM_SEND_SIZE; \
	} while (0)

#define free_gsm_send() \
		(gsm_send_tail >  gsm_send_head ? gsm_send_tail - gsm_send_head - 1 : gsm_send_tail + GSM_SEND_SIZE - gsm_send_head - 1)

#define over_gsm_send()\
		(gsm_send_head == gsm_send_tail - 1 || gsm_send_head == gsm_send_tail + GSM_SEND_SIZE - 1)

#define clr_gsm_send() (gsm_send_tail = gsm_send_head)

//GPRS 数据接收指针管理	
#define incp_gprs_rev(ptr, n) \
	do { \
		if ((ptr += n) >= gprs_rev_buf + GPRS_REV_SIZE) \
			ptr -= GPRS_REV_SIZE;\
	} while (0)	

#define decp_gprs_rev(ptr, n) \
	do { \
		if ((ptr -= n) < gprs_rev_buf) \
			ptr += GPRS_REV_SIZE;\
	} while (0)

#define new_gprs_rev() \
		(gprs_rev_head >= gprs_rev_tail ? gprs_rev_head - gprs_rev_tail : gprs_rev_head +GPRS_REV_SIZE - gprs_rev_tail)

#define over_gprs_rev() \
		(gprs_rev_head == gprs_rev_tail - 1 || gprs_rev_head == gprs_rev_tail + GPRS_REV_SIZE - 1)

#define subp_gprs_rev(p1, p2)	 \
		(p1 >= p2 ? p1 - p2 : p1 + GPRS_REV_SIZE - p2)

#define clr_gprs_rev() (gprs_rev_tail = gprs_rev_head)

#define ini_gsm_gprs_rev() \
	do{\
		gsm_rev_head= gsm_rev_buf;\
		gsm_rev_tail = gsm_rev_buf;\
	}while(0)
	
//GPRS 数据发送指针管理	  //		urc_tail = gsm_rev_buf;
	
#define incp_gprs_send(pd, n)	do {if ((pd += n) >= gprs_send_buf + GPRS_SEND_SIZE)	pd -= GPRS_SEND_SIZE;} while (0)

#define subp_gprs_send(p1, p2)	(p1 >= p2 ? p1 - p2 : p1 + GPRS_SEND_SIZE - p2)

#define over_gprs_send()	(gprs_send_head == gprs_send_tail - 1 || gprs_send_head == gprs_send_tail + GPRS_SEND_SIZE - 1)

#define clr_gprs_send()	(gprs_send_tail = gprs_send_head)

#define free_gprs_send()	(gprs_send_tail > gprs_send_head ? gprs_send_tail - gprs_send_head - 1 : gprs_send_tail + GPRS_SEND_SIZE - gprs_send_head - 1)


#define incp_gprs2_send(pd, n)	do {if ((pd += n) >= gprs2_send_buf + GPRS_SEND_SIZE)	pd -= GPRS_SEND_SIZE;} while (0)
#define subp_gprs2_send(p1, p2)	(p1 >= p2 ? p1 - p2 : p1 + GPRS_SEND_SIZE - p2)
#define over_gprs2_send()	(gprs2_send_head == gprs2_send_tail - 1 || gprs2_send_head == gprs2_send_tail + GPRS_SEND_SIZE - 1)
#define clr_gprs2_send()	(gprs2_send_tail = gprs2_send_head)
#define free_gprs2_send()	(gprs2_send_tail > gprs2_send_head ? gprs2_send_tail - gprs2_send_head - 1 : gprs2_send_tail + GPRS_SEND_SIZE - gprs2_send_head - 1)

#define incp_gprs3_send(pd, n)	do {if ((pd += n) >= gprs3_send_buf + GPRS_SEND_SIZE)	pd -= GPRS_SEND_SIZE;} while (0)
#define subp_gprs3_send(p1, p2)	(p1 >= p2 ? p1 - p2 : p1 + GPRS_SEND_SIZE - p2)
#define over_gprs3_send()	(gprs3_send_head == gprs3_send_tail - 1 || gprs3_send_head == gprs3_send_tail + GPRS_SEND_SIZE - 1)
#define clr_gprs3_send()	(gprs3_send_tail = gprs3_send_head)
#define free_gprs3_send()	(gprs3_send_tail > gprs3_send_head ? gprs3_send_tail - gprs3_send_head - 1 : gprs3_send_tail + GPRS_SEND_SIZE - gprs3_send_head - 1)


#define idle_gsm() \
	(gsm_send_tail == gsm_send_head && _pastn(gsm_send_jfs) >  100 && _pastn(gsm_rev_jfs) > 100)

#define free_gsm() \
	(gsm_send_tail == gsm_send_head && _pastn(gsm_send_jfs) > 500 && _pastn(gsm_rev_jfs) > 500) //jiff_tx 2000

#define wait_gsm() \
	(gsm_send_tail == gsm_send_head && _pastn(gsm_send_jfs) > 500 && _pastn(gsm_rev_jfs) > 2000) //jiff_tx 2000

extern u32 gsm_rev_jfs;
extern u32 gsm_send_jfs;
extern u16 gprs_lac ;
extern u16 gprs_celli ;
extern u8  netx ;  
extern sAtEngine AtEngine1;
extern sAtEngine AtEngine2;
extern sAtEngine AtEngine3;

extern int printf_gsm(uc8* fmt, ...);
extern u8* get_gsm_resp(u8* str);
extern u8* get_gsm_field(u8 n);
extern u8* get_gsm_line(u8 n);
extern u8* search_gsm_recv(u8* tail, u8* head, u8* str);
extern void write_gsm_txt(u8* strp);
extern void write_gsm(u8* buf, u16 len);
extern void write_gsm_hex(u8 hex);
extern void read_gsm(u8* buf, u8* rbuf_ptr, u16 n);
extern void read_gprs(u8* buf, u8* rbuf_ptr, u16 n);
extern void gsm_drv_tran_task(void);
extern u8 pdu_sms_to_txt(u8 *str, u16 len);
extern void sync_net_online(void);
extern void sync_net_offline(void);
extern void  AtStringFind1(u8* str1,u8* str2, u8* str3 );
extern void  AtStringFind2(u8* str1,u8* str2, u8* str3 );
extern void  AtStringFind3(u8* str1,u8* str2, u8* str3 );
extern bool login_net_123(void);
#endif	



