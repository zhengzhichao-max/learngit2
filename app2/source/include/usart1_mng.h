#ifndef _USART1_MNG_H
#define _USART1_MNG_H

#include "include_all.h"


extern void uart1_dev_init(void);


extern void my_printf(const char* fmt,...);

extern void uart1_input_byte(u8 data);
extern bool uart1_output_byte(u8 *out);

extern void uart1_set_recv_sta(bool sta);
extern void uart1_set_recv_len(u16 len);



extern void uart1_SendAndRecv_task(void);


#endif


























































