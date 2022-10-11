#include "systick.h"

static u8 fac_us = 0;//us��ʱ����
static u16 fac_ms = 0;// ms��ʱ����

#define	BSP_SYS_CLOCK_HZ		(72 * 1000 * 1000)		//72MHz


/**********************************************
* �� �� ����systick_time_init
* �������ܣ�ϵͳ�δ�ʱ�ӳ�ʼ��
* ��ڲ�����ϵͳ��ʱ��Ƶ�ʣ���λΪHMz
* �� �� ֵ����
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
* �� �� ����delay_ms
* �������ܣ����뼶��ʱ
* ��ڲ�����nms ��ʱ�ĺ����� ����72Mhzϵͳʱ���£�nms <= 1864
* �� �� ֵ����
***********************************************************/
void sys_delay_ms(u16 nms)
{
    
	 u32 temp;
	SysTick->LOAD = (u32)nms*fac_ms;//װ�س�ֵ
	SysTick->VAL = 0x00;//��SysTick��ǰֵ�Ĵ���(SysTick current value register)����
	SysTick->CTRL |=0x01;//����������
	do
	{
	    temp = SysTick->CTRL;
	}
	while(temp&0x01 && !(temp&(1<<16)));
		
   SysTick->CTRL = 0X00; //�رռ�����
   SysTick->VAL = 0x00;	

}




/*********************************************************
* �� �� ����delay_us
* �������ܣ�΢���ʱ
* ��ڲ�����nus ��ʱ��΢���� 
* �� �� ֵ����
**********************************************************/
void sys_delay_us(u16 nus)
{
    
	 u32 temp;
	SysTick->LOAD = (u32)nus*fac_us;//װ�س�ֵ
	SysTick->VAL = 0x00;//��SysTick��ǰֵ�Ĵ���(SysTick current value register)����
	SysTick->CTRL |=0x01;//����������
	do
	{
	    temp = SysTick->CTRL;
	}
	while(temp&0x01 && !(temp&(1<<16)));
		
   SysTick->CTRL = 0X00; //�رռ�����
   SysTick->VAL = 0x00;	

}

















