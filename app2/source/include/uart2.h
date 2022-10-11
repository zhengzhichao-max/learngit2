/**
  ******************************************************************************
  * @file    uart4.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ´®¿Ú4¹ÜÀí
*/  
#ifndef	__UART4_H
#define	__UART4_H

#define  R4_BUF_SIZE				2200
#define  T4_BUF_SIZE				1200		///800

extern u32 uart4_baud_rate[];

extern uart_manage u4_m;

extern u8 r4_buf[];
extern u8* r4_head;
extern u8* r4_tail;

extern u8 t4_buf[];
extern u8* t4_head;
extern u8* t4_tail;

extern bool uart4_busy;
extern u32 jif_r4;
extern u32 jif_t4;

#define incp_r4(ptr, n) \
	do { \
		if ((ptr += n) >= r4_buf + R4_BUF_SIZE) \
			ptr -= R4_BUF_SIZE;\
	} while (0)
#define decp_r4(ptr, n) \
	do { \
		if ((ptr -= n) < r4_buf) \
			ptr += R4_BUF_SIZE;\
	} while (0)
#define incp_t4(ptr, n) \
	do { \
		if ((ptr += n) >= t4_buf + T4_BUF_SIZE) \
			ptr -= T4_BUF_SIZE;\
	} while (0)
#define decp_t4(ptr, n) \
	do { \
		if ((ptr -= n) < t4_buf) \
			ptr += T4_BUF_SIZE;\
	} while (0)

#define new_r4() \
	(r4_head >= r4_tail ? r4_head - r4_tail : r4_head + R4_BUF_SIZE - r4_tail)
#define free_t4() \
	(t4_tail >  t4_head ? t4_tail - t4_head - 1 : t4_tail + T4_BUF_SIZE - t4_head - 1)
#define over_r4() \
	(r4_head == r4_tail - 1 || r4_head == r4_tail + R4_BUF_SIZE - 1)
#define over_t4() \
	(t4_head == t4_tail - 1 || t4_head == t4_tail + T4_BUF_SIZE - 1)

#define clr_buf_t4()		(t4_tail = t4_head)
#define clr_buf_r4()		(r4_tail = r4_head)
#define subp_r4(p1, p2)	(p1 >= p2 ? p1 - p2 : p1 + R4_BUF_SIZE - p2)
#define subp_t4(p1, p2)	(p1 >= p2 ? p1 - p2 : p1 + T4_BUF_SIZE - p2)

#define uart4_free() \
	(_pastn(jif_t4) > 500 && _pastn(jif_r4) > 500) //jiff_tx 2000

extern UART_MANAGE_STRUCT read_uar4;

extern void load_uart2_remap(void);
extern void load_uart4_remap(void);
extern u32 get_uart4_func(void);
extern bool uart4_update_func(u16 fun, u16 set);
extern void uart4_tran_task(void);
extern void uart4_recive_task(void);
extern void write_uart4(u8* buf, u16 len);
extern void write_uart4_hex(u8 hex);
extern u16 read_uart4(u8* buf, u8* rbuf_ptr, u16 n);
extern u8 get_uart4_news(UART_MANAGE_STRUCT *c);

extern void tr9_uart4(void);



#endif	

