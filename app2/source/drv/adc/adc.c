#include "include_all.h"
AUTO_WEIGHT         auto_weight;

  uint32_t		s_TIMER_Delay_ms;
volatile u16 ADCConvertedValue[3];      //用来存放ADC�?��结果，也是DMA的目标地�?,3通道，每通道采集10次后面取平均�?
u16 ADC_Die[3];       
void DMA1_Init(void)
{
	
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能时钟

    DMA_DeInit(DMA1_Channel1);                                          //将�?�道�?寄存器�?为默认�??
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(ADC1->DR);	    //该参数用以定义DMA外�?基地�?
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;	    //该参数用以定义DMA内存基地�?(�?��结果保存的地�?)
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	                //该参数�?定了外�?�?��为数�?��输的�?��地还�?��源，此�?�?��为来�?
    DMA_InitStructure.DMA_BufferSize = 3;	                            //定义指定DMA通道的DMA缓存的大�?单位为数�?��位�?�这里也就是ADCConvertedValue的大�?
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//设定外�?地址寄存器�?��?与否,此�?设为不变 Disable
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	            //用来设定内存地址寄存器�?��?与否,此�?设为递�?，Enable
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//数据宽度�?6�?
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//数据宽度�?6�?
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 	                //工作在循�?��存模�?
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;	                //DMA通道拥有高优先级 分别4�?���?低�?�中、高、非常高
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	//使能DMA通道的内存到内存传输
    
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);	//根据DMA_InitStruct�?��定的参数初�?化DMA的�?�道

    DMA_Cmd(DMA1_Channel1, ENABLE);		//�?��DMA通道�?
}



void Adc1_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz ;
	GPIO_Init(GPIOC,&GPIO_InitStructure);	

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);		//72M/6=12, ADC的采样时钟最�?4MHz  
    //g_delay_us(20);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	 	//�?��模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;			//�?��模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 		//连续�?��
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//�?���?��
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//数据右�?�?
    ADC_InitStructure.ADC_NbrOfChannel = ADC_Channel_3;    	//选择通道3
    ADC_Init(ADC1, &ADC_InitStructure);
    //g_delay_us(20);
    
    ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_55Cycles5);//ADC_SampleTime_239Cycles5通道�?�?��结果保存到ADCConvertedValue[0~10][0]
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,2,ADC_SampleTime_55Cycles5);//通道二转换结果保存到ADCConvertedValue[0~10][1]
	ADC_RegularChannelConfig(ADC1,ADC_Channel_13,3,ADC_SampleTime_55Cycles5);//通道三转换结果保存到ADCConvertedValue[0~10][2]
	  
    ADC_DMACmd(ADC1, ENABLE);	//�?启ADC的DMA�?��
    ADC_Cmd(ADC1, ENABLE);

    //g_delay_us(20);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);   //????????????ADC???DMA???????????RAM????   

    //g_delay_us(20);
    
}


u16 Read_Power_Adc_Value(void)
{
	u32 Power_AD;
//    static u32 tmr = 0 ; 

	
	Power_AD = ADCConvertedValue[2];       	//ADC_GetConversionValue(ADC1);
	Power_AD=(Power_AD *330)/(4096);		//求平均�?�并�?��成电压�??
	Power_AD = (Power_AD * 956) / 47;		//Ӳ����ѹϵ��			
	ADC_Die[2]	= Power_AD; 

	Power_AD = ADCConvertedValue[1];
	Power_AD=(Power_AD *330)/(4096);        	//求平均�?�并�?��成电压�??
	Power_AD = (Power_AD * 956) / 47;	    	//Ӳ����ѹϵ��			
	ADC_Die[1]  = Power_AD; 
	
	Power_AD = ADCConvertedValue[0];
	Power_AD = (Power_AD * 330)/(4096);        //求平均�?�并�?��成电压�??
	Power_AD = (Power_AD * 956) / 47;	    //Ӳ����ѹϵ��		
	ADC_Die[0] = Power_AD ;

	#if 0
    if(( _covern(tmr) > 15) && (0 == u1_m.cur.b.tt))
    {
        tmr = tick;
        	logd("Power_AD1 = %d, Power_AD2 = %d, Power_AD3 = %d", ADC_Die[0], ADC_Die[1], ADC_Die[2]);
	}
	#endif
	
	return Power_AD;
}

u16 Read_ACC_Value(void){
	static u32 Power_AD=0;
    static u32 tmr = 0;
    
    if( _covern(tmr) > 15 )
    {
        tmr = tick;
        logd("Power_AD = %d", Power_AD);
    }

	return Power_AD;
}



