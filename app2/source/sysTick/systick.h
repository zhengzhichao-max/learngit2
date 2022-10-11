#ifndef   _SYSTICK_H_
#define  _SYSTICK_H_

#include "stm32f10x.h"

void systick_time_init(u8 SYSCLK);
void sys_delay_ms(u16 nms);
void sys_delay_us(u16 nus);



#endif


