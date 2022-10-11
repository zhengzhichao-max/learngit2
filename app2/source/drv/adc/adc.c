#include "include_all.h"
AUTO_WEIGHT         auto_weight;

  uint32_t		s_TIMER_Delay_ms;
volatile u16 ADCConvertedValue[3];      //ç”¨æ¥å­˜æ”¾ADCè½?¢ç»“æžœï¼Œä¹Ÿæ˜¯DMAçš„ç›®æ ‡åœ°å?,3é€šé“ï¼Œæ¯é€šé“é‡‡é›†10æ¬¡åŽé¢å–å¹³å‡æ•?
u16 ADC_Die[3];       
void DMA1_Init(void)
{
	
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//ä½¿èƒ½æ—¶é’Ÿ

    DMA_DeInit(DMA1_Channel1);                                          //å°†é?šé“ä¸?å¯„å­˜å™¨è?ä¸ºé»˜è®¤å??
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(ADC1->DR);	    //è¯¥å‚æ•°ç”¨ä»¥å®šä¹‰DMAå¤–è?åŸºåœ°å?
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADCConvertedValue;	    //è¯¥å‚æ•°ç”¨ä»¥å®šä¹‰DMAå†…å­˜åŸºåœ°å?(è½?¢ç»“æžœä¿å­˜çš„åœ°å?)
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;	                //è¯¥å‚æ•°è?å®šäº†å¤–è?æ˜?½œä¸ºæ•°æ?¼ è¾“çš„ç›?š„åœ°è¿˜æ˜?¥æºï¼Œæ­¤å?æ˜?½œä¸ºæ¥æº?
    DMA_InitStructure.DMA_BufferSize = 3;	                            //å®šä¹‰æŒ‡å®šDMAé€šé“çš„DMAç¼“å­˜çš„å¤§å°?å•ä½ä¸ºæ•°æ?•ä½ã?‚è¿™é‡Œä¹Ÿå°±æ˜¯ADCConvertedValueçš„å¤§å°?
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	//è®¾å®šå¤–è?åœ°å€å¯„å­˜å™¨é?’å?ä¸Žå¦,æ­¤å?è®¾ä¸ºä¸å˜ Disable
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	            //ç”¨æ¥è®¾å®šå†…å­˜åœ°å€å¯„å­˜å™¨é?’å?ä¸Žå¦,æ­¤å?è®¾ä¸ºé€’å?ï¼ŒEnable
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//æ•°æ®å®½åº¦ä¸?6ä½?
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//æ•°æ®å®½åº¦ä¸?6ä½?
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 	                //å·¥ä½œåœ¨å¾ªçŽ?¼“å­˜æ¨¡å¼?
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;	                //DMAé€šé“æ‹¥æœ‰é«˜ä¼˜å…ˆçº§ åˆ†åˆ«4ä¸?­‰çº?ä½Žã?ä¸­ã€é«˜ã€éžå¸¸é«˜
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	//ä½¿èƒ½DMAé€šé“çš„å†…å­˜åˆ°å†…å­˜ä¼ è¾“
    
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);	//æ ¹æ®DMA_InitStructä¸?Œ‡å®šçš„å‚æ•°åˆå?åŒ–DMAçš„é?šé“

    DMA_Cmd(DMA1_Channel1, ENABLE);		//å?Š¨DMAé€šé“ä¸?
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
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);		//72M/6=12, ADCçš„é‡‡æ ·æ—¶é’Ÿæœ€å¿?4MHz  
    //g_delay_us(20);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	 	//ç‹?«‹æ¨¡å¼
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;			//æ‰?æ¨¡å¼
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 		//è¿žç»­è½?¢
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//è½?»¶è½?¢
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//æ•°æ®å³å?é½?
    ADC_InitStructure.ADC_NbrOfChannel = ADC_Channel_3;    	//é€‰æ‹©é€šé“3
    ADC_Init(ADC1, &ADC_InitStructure);
    //g_delay_us(20);
    
    ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_55Cycles5);//ADC_SampleTime_239Cycles5é€šé“ä¸?è½?¢ç»“æžœä¿å­˜åˆ°ADCConvertedValue[0~10][0]
	ADC_RegularChannelConfig(ADC1,ADC_Channel_12,2,ADC_SampleTime_55Cycles5);//é€šé“äºŒè½¬æ¢ç»“æžœä¿å­˜åˆ°ADCConvertedValue[0~10][1]
	ADC_RegularChannelConfig(ADC1,ADC_Channel_13,3,ADC_SampleTime_55Cycles5);//é€šé“ä¸‰è½¬æ¢ç»“æžœä¿å­˜åˆ°ADCConvertedValue[0~10][2]
	  
    ADC_DMACmd(ADC1, ENABLE);	//å¼?å¯ADCçš„DMAæ”?Œ
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
	Power_AD=(Power_AD *330)/(4096);		//æ±‚å¹³å‡å?¼å¹¶è½?¢æˆç”µåŽ‹å??
	Power_AD = (Power_AD * 956) / 47;		//Ó²¼þ·ÖÑ¹ÏµÊý			
	ADC_Die[2]	= Power_AD; 

	Power_AD = ADCConvertedValue[1];
	Power_AD=(Power_AD *330)/(4096);        	//æ±‚å¹³å‡å?¼å¹¶è½?¢æˆç”µåŽ‹å??
	Power_AD = (Power_AD * 956) / 47;	    	//Ó²¼þ·ÖÑ¹ÏµÊý			
	ADC_Die[1]  = Power_AD; 
	
	Power_AD = ADCConvertedValue[0];
	Power_AD = (Power_AD * 330)/(4096);        //æ±‚å¹³å‡å?¼å¹¶è½?¢æˆç”µåŽ‹å??
	Power_AD = (Power_AD * 956) / 47;	    //Ó²¼þ·ÖÑ¹ÏµÊý		
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



