#ifndef _ADC_H_
#define _ADC_H_




//重量处理
typedef struct{
	bool  read;
	bool  cali_full;
	bool  cali_empty;  
	bool  zf;
	u16   value;
	u32   tmr;
	u32   exisTmr;  //判断外设是否工作时间
}AUTO_WEIGHT;


extern u16 ADC_Die[3];

extern AUTO_WEIGHT         auto_weight;

extern   uint32_t		s_TIMER_Delay_ms;
extern void DMA1_Init(void);
extern void Adc1_Init(void);


extern u16 Read_Power_Adc_Value(void);
extern u16 Read_ACC_Value(void);
#endif

