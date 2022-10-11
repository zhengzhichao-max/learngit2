/**
  ******************************************************************************
  * @file    uart5.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ´®¿Ú5¹ÜÀí
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#ifndef	__UART5_H
#define	__UART5_H

#define  T3_BUF_SIZE			1200

extern u32 uart3_baud_rate[];

extern uart_manage u3_m;


extern u8 t3_buf[];
extern u8* t3_head;
extern u8* t3_tail;
extern bool tr9_show;
extern bool uart3_busy;
extern u32 jif_t3;


#define incp_t3(ptr, n) \
	do { \
		if ((ptr += n) >= t3_buf + T3_BUF_SIZE) \
			ptr -= T3_BUF_SIZE;\
	} while (0)
#define decp_t3(ptr, n) \
	do { \
		if ((ptr -= n) < t3_buf) \
			ptr += T3_BUF_SIZE;\
	} while (0)

#define uart3_free() \
	(_pastn(jif_t3) > 500 && _pastn(jif_r3) > 500) //jiff_tx 2000

#define free_t3() \
	(t3_tail >  t3_head ? t3_tail - t3_head - 1 : t3_tail + T3_BUF_SIZE - t3_head - 1)
#define over_t3() \
	(t3_head == t3_tail - 1 || t3_head == t3_tail + T3_BUF_SIZE - 1)

#define clr_buf_t3()		(t3_tail = t3_head)
#define subp_t3(p1, p2)	(p1 >= p2 ? p1 - p2 : p1 + T3_BUF_SIZE - p2)

extern u16 u3_len;
extern UART_MANAGE_STRUCT read_uar3;

extern u32 get_uart3_func(void);
extern void uart3_remap_manage(void);
extern void uart3_tran_task(void);
extern void uart3_recive_task(void);

extern void write_uart3(u8* buf, u16 len);
extern void write_uart3_hex(u8 hex);
extern u16 read_uart3(u8* buf, u8* rbuf_ptr, u16 n);
#endif	

