#include "systick.h"

static u8 fac_us = 0;//us延时因子
static u16 fac_ms = 0;// ms延时因子

#define	BSP_SYS_CLOCK_HZ		(72 * 1000 * 1000)		//72MHz


/**********************************************
* 函 数 名：systick_time_init
* 函数功能：系统滴答时钟初始化
* 入口参数：系统主时钟频率，单位为HMz
* 返 回 值：无
***********************************************/
void systick_time_init(u8 SYSCLK)
{ 
/*	  SysTick->CTRL &= 0xfffffffb; 
     fac_us = SYSCLK/8;
     fac_ms = (u16)fac_us*1000;	
*/     

	if (SysTick_Config(BSP_SYS_CLOCK_HZ / 1000))
	{
		while (1);
	}


}





/**********************************************************
* 函 数 名：delay_ms
* 函数功能：毫秒级延时
* 入口参数：nms 延时的毫秒数 ，在72Mhz系统时钟下，nms <= 1864
* 返 回 值：无
***********************************************************/
void sys_delay_ms(u16 nms)
{
    
	 u32 temp;
	SysTick->LOAD = (u32)nms*fac_ms;//装载初值
	SysTick->VAL = 0x00;//将SysTick当前值寄存器(SysTick current value register)清零
	SysTick->CTRL |=0x01;//启动计数器
	do
	{
	    temp = SysTick->CTRL;
	}
	while(temp&0x01 && !(temp&(1<<16)));
		
   SysTick->CTRL = 0X00; //关闭计数器
   SysTick->VAL = 0x00;	

}




/*********************************************************
* 函 数 名：delay_us
* 函数功能：微妙级延时
* 入口参数：nus 延时的微妙数 
* 返 回 值：无
**********************************************************/
void sys_delay_us(u16 nus)
{
    
	 u32 temp;
	SysTick->LOAD = (u32)nus*fac_us;//装载初值
	SysTick->VAL = 0x00;//将SysTick当前值寄存器(SysTick current value register)清零
	SysTick->CTRL |=0x01;//启动计数器
	do
	{
	    temp = SysTick->CTRL;
	}
	while(temp&0x01 && !(temp&(1<<16)));
		
   SysTick->CTRL = 0X00; //关闭计数器
   SysTick->VAL = 0x00;	

}

















