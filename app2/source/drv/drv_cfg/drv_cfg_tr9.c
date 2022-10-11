/**
  ******************************************************************************
  * @file    drv_cfg_tr9.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief    ������Ӳ����ص����ж˿ڻ�����Դ
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

#include "diskio.h"

//----------------------------------------//
//----------------------------------------//
//Ӳ����Դ�����  CKP  2015-03-25
//
//��ʱ��
//SYSTICK:  δʹ��
//TIM2:     ϵͳʱ��
//TIM3:     ������
//TIM4:     ������
//TIM6:     ��ӡ��
//----------------------------------------//
//----------------------------------------//

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;
ErrorStatus HSEStartUpStatus;

//***********************************************************************************************//
void g_delay_us(uchar t) {
    delay_us(t + 2);
}

//���뼶��ʱ
void g_delay_ms(uint t) {
    delayms(t * 9);
}

void RCC_Configuration(void) {
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS) {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);

        //        RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);    //   PLL 8M * 9 = 72 M
        RCC_PREDIV2Config(RCC_PREDIV2_Div5);
        RCC_PLL2Config(RCC_PLL2Mul_8);
        RCC_PLL2Cmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET) {}
        RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);
        RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9); //PLLCLK = 8MHz * 9 = 72 MHz

        /* Enable PLL */
        RCC_PLLCmd(ENABLE); //hcy ENABLE
        /* HCLK = SYSCLK*/
        RCC_HCLKConfig(RCC_SYSCLK_Div1); //(72M  CKP)

        /* PCLK2 = HCLK /4*/
        RCC_PCLK2Config(RCC_HCLK_Div4); //(72M/4=18M  CKP)   APB2ʱ��Ϊϵͳʱ��

        /* PCLK1 = HCLK/4 */
        RCC_PCLK1Config(RCC_HCLK_Div4); //(72M/4=18M  CKP)   APB1ʱ��Ϊϵͳʱ��

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08) {}
    }

    /* TIM2 & TIM3 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4 | RCC_APB1Periph_TIM6 | RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3 | RCC_APB1Periph_UART4 | RCC_APB1Periph_UART5 | RCC_APB1Periph_CAN2 | RCC_APB1Periph_CAN1, ENABLE);

    /* GPIOA & ADC1 clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_SPI1 | RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}

/****************************************************************************
* ����:    NVIC_Configuration ()
* ���ܣ��ж��������ȼ�������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void NVIC_Configuration(void) {
    NVIC_InitTypeDef NVIC_InitStructure;

#if (TR_USING_IAP)
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2800);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
#endif

    //���ȼ����� II
    /* 2 bits for pre-emption priority      2 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; //UART4_IRQChannel
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //EXTI0_IRQn
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); //���ö˿��ж� PA2

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); //���ö�ʱ��4�ж�

    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); //���ö�ʱ��6�ж�

#if (P_CAN == _CAN_BASE)

    NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn; //USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn; //USB_HP_CAN1_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = CAN1_SCE_IRQn; //USB_HP_CAN1_TX_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //CAN2�жϳ�ʼ��
    NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn; //USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

#endif
}

/****************************************************************************
* ����:    Timer2_Configuration ()
* ���ܣ�ÿ5ms ����һ���ж�
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void Timer2_Configuration(void) { //Ӳ����ʱ��2:
    /* ---------------------------------------------------------------
      18M / 1800 = 
    --------------------------------------------------------------- */
    TIM_DeInit(TIM2); /* deinitiate */

    TIM_TimeBaseStructure.TIM_Period = (100 - 1);     //99;/* Time base configuration */
    TIM_TimeBaseStructure.TIM_Prescaler = (1800 - 1); //; 0.025ms

    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing; /* Timing Mode :Channel1 */
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2, ENABLE);

    TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE); /* TIM IT enable */

    TIM_Cmd(TIM2, ENABLE); /* TIM2 enable counter */
}

//��������������     //�������  25����
static void Timer4_Configuration(void) { //Ӳ����ʱ��4:
    TIM_TimeBaseInitTypeDef tim_base_init_struct;

    tim_base_init_struct.TIM_Period = 249;
    tim_base_init_struct.TIM_Prescaler = 3599;

    tim_base_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;     //����Ƶ
    tim_base_init_struct.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ

    //tim_base_init_struct.TIM_RepetitionCounter = ;	 //�޹ز���
    TIM_TimeBaseInit(TIM4, (&tim_base_init_struct));
}

//36MHz
//��ӡ����ʱ����
static void Timer6_Configuration(void) { //Ӳ����ʱ��6:
    TIM_TimeBaseInitTypeDef tim_base_init_struct;

    tim_base_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;     //����Ƶ
    tim_base_init_struct.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ
    tim_base_init_struct.TIM_Period = 40;                      //39 ?
    tim_base_init_struct.TIM_Prescaler = 35;                   //35�������

    TIM_TimeBaseInit(TIM6, (&tim_base_init_struct));

    print_start_ctrl(_false_); //�رմ�ӡ
}

/****************************************************************************
* ����:    Can_Configuration ()
* ���ܣ�CAN1 ��������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void Can_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    //�ض����ַ1ģʽ
    /* Configure CAN pin: RX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure CAN pin: TX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure CAN Remap   ��Ӱ�� */
    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
}
/****************************************************************************
* ����:    Can2_Configuration ()
* ���ܣ�CAN2 ��������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void Can2_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    //�ض����ַ1ģʽ
    /* Configure CAN pin: RX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure CAN pin: TX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    /* Configure CAN Remap   ��Ӱ�� */
    //GPIO_PinRemapConfig(GPIO_Remap_CAN2, ENABLE);
}

/****************************************************************************
* ����:    Uart_Configuration ()
* ���ܣ����ڹ�������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void Uart_Configuration(void) {
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#pragma region //USART0: PA9 & PA10
    /*
    ���ô���1 MIX UART
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_DeInit(USART1);
    USART_Cmd(USART1, DISABLE);
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
#pragma endregion //USART0: PA9 & PA10

#pragma region //USART1: PA2 & PA3
    /*
    ���ô���2  GSM
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_DeInit(USART2);
    USART_InitStructure.USART_BaudRate = 115200; //9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
#pragma endregion //USART1: PA2 & PA3

#pragma endregion //USART2: PB10 & PB11
    /*
    ���ô���3  TR9-HISEE
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_DeInit(USART3);
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART3, ENABLE);
#pragma endregion //USART2: PB10 & PB11

#pragma endregion //UART3: PC10 & PC11
    /*
    ���ô���4
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    USART_DeInit(UART4);
    // USART_InitStructure.USART_BaudRate = 19200;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART4, &USART_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE);
#pragma endregion //UART3: PC10 & PC11

#pragma region //UART4: PC12 & PD2
    /*
    GPS
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    //
    /**
 * @brief �߾��ȶ�λģ�顾ǧѰMC180M��page11
 *          [3]TXD0 Ϊ����ܽţ���ǧѰMC180M������
 *              ��ģ���ϵ�100ms���豣�ֵ͵�ƽ��
 *              ����ģ�齫���빤��ģʽ�޷�����������
 *              ���飺��TXD0�����Ĺܽ���ģ���ϵ�100ms��ά����������ʼ������Ϊ�����������롣
 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //ԭ����GPIO_Mode_IN_FLOATING//�߾��ȡ�ǧѰMC180M��GPIO_Mode_IPD
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_DeInit(UART5);
    USART_InitStructure.USART_BaudRate = 115200; //9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART5, &USART_InitStructure);
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART5, ENABLE);
#pragma endregion //UART4: PC12 & PD2
}

/****************************************************************************
* ���ƣ�INPUT1_high_INPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void INPUT1_HIGH1_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_I7_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I7_HIGH, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�INPUT1_LOW1_INPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void INPUT1_LOW1_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_COVER_LOW;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_COVER_LOW, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�INPUT1_LOW2_INPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void INPUT1_LOW2_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_LIFT_LOW;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_LIFT_LOW, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�EXTI_Configuration
* ���ܣ��ж���������
* ��ڲ�������?
* ���ڲ�������
***************************************************************************/
static void EXTI15_Configuration(void) {
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    ///speed
    GPIO_InitStructure.GPIO_Pin = PORT_SPEED_PLUS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_SPEED_PLUS, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource15); //�˿�PA2
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;                  //EXTI0_IRQn
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //�жϷ�ʽ
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
    EXTI_Init(&EXTI_InitStructure);
}

/****************************************************************************
* ?????ACC_INPUT
* ????????????
* ??????????
* ???????????
***************************************************************************/
void ACC_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_ACC;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(TYPE_ACC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_I8_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I8_HIGH, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(TYPE_ACC, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�SOS_INPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void SOS_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_INPUT_SOS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_INPUT_SOS, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�DVR_IO_INPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void DVR_IO_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    port_dir_config(ic_card_in_port, bit_move(ic_card_in_pin), false); //IC��������

    GPIO_InitStructure.GPIO_Pin = PORT_I1_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I1_HIGH, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_I2_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I2_HIGH, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_I3_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I3_HIGH, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_I4_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I4_HIGH, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_I5_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I5_HIGH, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_H6_BACK;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_H6_BACK, &GPIO_InitStructure);

    //������
    GPIO_InitStructure.GPIO_Pin = PORT_LOCK_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_LOCK_HIGH, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�DVR_IO_OUTPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void DVR_IO_OUTPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_LCD_BL_PWR;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_LCD_BL_PWR, &GPIO_InitStructure);

    port_dir_config(lcd_io_port, bit_move(lcd_io_pin), true);
    port_dir_config(lcd_scl_port, bit_move(lcd_scl_pin), true);
    port_dir_config(lcd_a0_port, bit_move(lcd_a0_pin), true);
    port_dir_config(lcd_cs_port, bit_move(lcd_cs_pin), true);
    port_dir_config(lcd_rst_port, bit_move(lcd_rst_pin), true);
    port_dir_config(lcd_bl_port, bit_move(lcd_bl_pin), true);

    port_dir_config(pow_ctrl_print_port, bit_move(pow_ctrl_print_pin), true);
    port_dir_config(print_mtnb_port, bit_move(print_mtnb_pin), true);
    port_dir_config(print_mtb_port, bit_move(print_mtb_pin), true);
    port_dir_config(print_mtna_port, bit_move(print_mtna_pin), true);
    port_dir_config(print_mta_port, bit_move(print_mta_pin), true);
    port_dir_config(print_di_port, bit_move(print_di_pin), true);
    port_dir_config(print_clk_port, bit_move(print_clk_pin), true);
    port_dir_config(print_lat_port, bit_move(print_lat_pin), true);

    port_dir_config(print_hot_port1, (bit_move(print_hot_1pin) | bit_move(print_hot_2pin) | bit_move(print_hot_3pin)), true);
    port_dir_config(print_hot_port2, bit_move(print_hot_4pin) | bit_move(print_hot_5pin) | bit_move(print_hot_6pin), true);

    //-------   IC���˿�����   -----------------
    //------------------------------------------
    port_dir_config((ic_card_rst_port), (bit_move(ic_card_rst_pin)), (true));

    //    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;	//GPIO_Mode_Out_OD
    //    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    //    GPIO_InitStructure.GPIO_Pin   = bit_move(ic_card_clk_pin);
    //   GPIO_Init(ic_card_io_port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; //GPIO_Mode_Out_OD
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Pin = (bit_move(ic_card_io_pin) | bit_move(ic_card_clk_pin)); //
    GPIO_Init(ic_card_io_port, &GPIO_InitStructure);

    //-------   ����˿�����   -----------------
    //------------------------------------------

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = (bit_move(key_up_pin) | bit_move(key_down_pin) | bit_move(key_menu_pin) | bit_move(key_print_pin) | bit_move(key_ok_pin));
    GPIO_Init(key_port, &GPIO_InitStructure); //���ð������˿�

#pragma region //5V VOUT1 ����ӿڳ�ʼ��
    GPIO_InitStructure.GPIO_Pin = PORT_OUT_5V_VOUT1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_OUT_5V_VOUT1, &GPIO_InitStructure);
#pragma endregion //5V VOUT1 ����ӿڳ�ʼ��

#pragma region //��λRK
    GPIO_InitStructure.GPIO_Pin = PORT_RESET_HI35XX;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_RESET_HI35XX, &GPIO_InitStructure);
#pragma endregion //��λRK
}

/****************************************************************************
* ���ƣ�RED_LED_OUTPUT
USB5V_SW
ADAS_5V_SW
IC_LCDVCC_SW
PWR_3GPP_SW
TPRN7V5
CAM12V_SW
PWR_GNSS_SW
VBAT_SW
HDDPWR_SW
SYS_VCC_SW
***************************************************************************/
void DVR_PWR_OUTPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_DVR_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_DVR_PWR, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_BEEP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_BEEP, &GPIO_InitStructure);

    ///5V POWER CTRL
    /*
	GPIO_InitStructure.GPIO_Pin = PORT_GSM_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_GSM_RST, &GPIO_InitStructure);
	*/

    GPIO_InitStructure.GPIO_Pin = PORT_LCDVCC_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_LCDVCC_PWR, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PORT_TURN_OFF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_TURN_OFF, &GPIO_InitStructure);

    ///CAM COM POWER CTRL
    GPIO_InitStructure.GPIO_Pin = PORT_CAM12V_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_CAM12V_PWR, &GPIO_InitStructure);

    /*
    GPIO_InitStructure.GPIO_Pin = PORT_VBAT_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_VBAT_PWR, &GPIO_InitStructure);
*/
    //����RK��Դ
    GPIO_InitStructure.GPIO_Pin = PORT_HDD_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_HDD_PWR, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�GPS_PWR_OUTPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void GPS_PWR_OUTPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_GPS_PWR;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_GPS_PWR, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ� relay_output_init
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void relay_output_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_RELAY_CTRL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_RELAY_CTRL, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�GSM_PWR_OUTPUT
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void GSM_PWR_OUTPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_GSM_PWR;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_GSM_PWR, &GPIO_InitStructure);
}

/****************************************************************************
* ���ƣ�GSM_SPK_LM4903
* ���ܣ��˿�����
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void GSM_SPK_LM4903(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_SPK_LM4903;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_SPK_LM4903, &GPIO_InitStructure);
}

void self_check(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

//****************************************************************//
//----------------------------------------------------------------//
//-----                    �˿����ú���                       ----//
//*********//
//�ú����������ö˿ڵ�����������������ù��ܼ�AD��⹦�ܲ���ʵ��
//*********//
//����GPIOxΪ�˿ںţ�����⺯��
//����pinxΪ���ź�
//����dirΪ���Ƶķ���    TRUEΪ�������    FALSEΪ���뷽��
//----------------------------------------------------------------//
//****************************************************************//
void port_dir_config(GPIO_TypeDef *GPIOx, u16 pinx, bool dir) {
    GPIO_InitTypeDef GPIO_InitStructure;

    if (dir) //���ģʽ
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    } else //����ģʽ
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    }

    GPIO_InitStructure.GPIO_Pin = pinx;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

//****************************************************************//
//----------------------------------------------------------------//
//-----              SPI�ӿ����ó���ģʽ                      ----//
//ע��!!!  CS�˿�Ϊ�ֶ����ƶ����Զ�����
//
//����SPIxΪ�˿ں���ֵӦ���ÿ��ļ���SPI1��ʽ
//����bd_speedΪ��Ƶϵ������ֵӦ���ÿ��ļ���SPI_BaudRatePrescaler_4��ʽ
//----------------------------------------------------------------//
//****************************************************************//
void spi_host_config(SPI_TypeDef *SPIx, uint bd_speed) {
    SPI_InitTypeDef spi_init_struct;

    spi_init_struct.SPI_BaudRatePrescaler = bd_speed;                //���÷�Ƶϵ��
    spi_init_struct.SPI_CPHA = SPI_CPHA_2Edge;                       //���ݲɼ��ӵڶ���ʱ���ؿ�ʼ
    spi_init_struct.SPI_CPOL = SPI_CPOL_High;                        //����״̬Ϊ�ߵ�ƽ
    spi_init_struct.SPI_CRCPolynomial = 7;                           //Ĭ��ֵ
    spi_init_struct.SPI_DataSize = SPI_DataSize_8b;                  //8λ���ݸ�ʽ
    spi_init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //����ȫ˫��ģʽ
    spi_init_struct.SPI_FirstBit = SPI_FirstBit_MSB;                 //��λ��ǰ
    spi_init_struct.SPI_Mode = SPI_Mode_Master;                      //����ģʽ
    spi_init_struct.SPI_NSS = SPI_NSS_Soft;                          //SS�˿����������
    SPI_Init(SPIx, &spi_init_struct);                                //SPI_FLASHоƬ�ӿ�

    SPI_NSSInternalSoftwareConfig(SPIx, SPI_NSSInternalSoft_Set); //�����λ������
    SPI_SSOutputCmd(SPIx, DISABLE);                               //��ֹ����Ӳ���������SS�˿�
    SPI_Cmd(SPIx, ENABLE);
}

//*****************************************************//
//*******          ���ͻ����һ���ֽ�           *******//
//����SPIxΪ�˿ں�
//����s_datΪ�����͵�һ���ֽ�
//��������ֵΪ���յ���һ���ֽ�
//*****************************************************//
uchar spi_read_write_byte(SPI_TypeDef *SPIx, uchar s_dat) {
    uchar temp;

    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) {
        ; //�ȴ���һ�����ݷ������
    }

    SPI_I2S_SendData(SPIx, s_dat); //��ʼ���ͻ�������װ������

    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) {
        ; //�ȴ�һ�����ݽ������
    }

    temp = SPI_I2S_ReceiveData(SPIx); //ȡ�����յ�������

    return temp; //���ض���������
}

void slv_mcu_io_config() {
    GPIO_InitTypeDef GPIO_InitStructure;

    //-------   ����˿�����   -----------------
    //------------------------------------------

    port_dir_config(lcd_io_port, bit_move(lcd_io_pin), true);
    port_dir_config(lcd_scl_port, bit_move(lcd_scl_pin), true);
    port_dir_config(lcd_a0_port, bit_move(lcd_a0_pin), true);
    port_dir_config(lcd_cs_port, bit_move(lcd_cs_pin), true);
    port_dir_config(lcd_rst_port, bit_move(lcd_rst_pin), true);
    port_dir_config(lcd_bl_port, bit_move(lcd_bl_pin), true);

    //-------   ����˿�����   -----------------
    //------------------------------------------

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = (bit_move(key_up_pin) | bit_move(key_down_pin) | bit_move(key_menu_pin) | bit_move(key_print_pin) | bit_move(key_ok_pin));
    GPIO_Init(key_port, &GPIO_InitStructure); //���ð������˿�

#if 0
    //-------     ��ӡ���ӿ�   ----------------
    //------------------------------------------

    port_dir_config(print_power_port, bit_move(print_power_pin), true);

    port_dir_config(pstb1_port, bit_move(pstb1_pin), true);
    port_dir_config(pstb2_port, bit_move(pstb2_pin), true);
    port_dir_config(pstb3_port, bit_move(pstb3_pin), true);
    port_dir_config(pstb4_port, bit_move(pstb4_pin), true);
    port_dir_config(pstb5_port, bit_move(pstb5_pin), true);
    port_dir_config(pstb6_port, bit_move(pstb6_pin), true);

    port_dir_config(pb_port, bit_move(pb_pin), true);
    port_dir_config(pnb_port, bit_move(pnb_pin), true);
    port_dir_config(pa_port, bit_move(pa_pin), true);
    port_dir_config(pnb_port, bit_move(pna_pin), true);

    port_dir_config(p_di_port, bit_move(p_di_pin), true);
    port_dir_config(p_clk_port, bit_move(p_clk_pin), true);
    port_dir_config(p_lat_port, bit_move(p_lat_pin), true);

    port_dir_config(p_det_port, bit_move(p_det_pin), false);  //ֽ�ż��

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = bit_move(p_rtm_pin);   //�¶ȼ��
    GPIO_Init(p_rtm_port, &GPIO_InitStructure);

    print_power_dis;
    p_lat_high;
#endif
}

/****************************************************************************
* ����:    config_free_gpio ()
* ���ܣ����п����ó�Ϊ���룬����
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void config_free_gpio(void) {
    slv_mcu_io_config();
    GSM_PWR_OUTPUT();
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(uint8_t *file, uint32_t line) {
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    log_write(event_mcu_boot_assert_failed);

    while (1) {
    }
}
#endif

/*********************************************************
��    �ƣ�WatchDog_Configuration
��    �ܣ����Ź���ʼ������
��    ������
��    ������
��д���ڣ�2011-01-21
**********************************************************/
void WatchDog_Configuration(void) {
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_128);
    IWDG_SetReload(500); //2000
    IWDG_ReloadCounter();
    IWDG_Enable();
}

//***********************************************************************************************//

//-------------//

//Һ��scl�˿ڿ���
static void lcd13264_scl_set(bit_enum s) {
    if (s) {
        lcd_scl_H;
    } else {
        lcd_scl_L;
    }
}

//Һ��cs�˿ڿ���
static void lcd13264_cs_set(bit_enum s) {
    if (s) {
        lcd_cs_H;
    } else {
        lcd_cs_L;
    }
}

//Һ��rst�˿ڿ���
static void lcd13264_rst_set(bit_enum s) {
    if (s) {
        lcd_rst_H;
    } else {
        lcd_rst_L;
    }
}

//Һ��a0�˿ڿ���
static void lcd13264_a0_set(bit_enum s) {
    if (s) {
        lcd_a0_H;
    } else {
        lcd_a0_L;
    }
}

//Һ��io�˿ڿ���
static void lcd13264_io_set(bit_enum s) {
    if (s) {
        lcd_io_H;
    } else {
        lcd_io_L;
    }
}

//-------------//

static void print_mb_set(bit_enum s) {
    if (s) {
        print_mtb_high;
    } else {
        print_mtb_low;
    }
}

static void print_mnb_set(bit_enum s) {
    if (s) {
        print_mtnb_high;
    } else {
        print_mtnb_low;
    }
}

static void print_ma_set(bit_enum s) {
    if (s) {
        print_mta_high;
    } else {
        print_mta_low;
    }
}

static void print_mna_set(bit_enum s) {
    if (s) {
        print_mtna_high;
    } else {
        print_mtna_low;
    }
}

static void print_di_set(bit_enum s) {
    if (s) {
        print_di_high;
    } else {
        print_di_low;
    }
}

static void print_clk_set(bit_enum s) {
    if (s) {
        print_clk_high;
    } else {
        print_clk_low;
    }
}

static void print_lat_set(bit_enum s) {
    if (s) {
        print_lat_high;
    } else {
        print_lat_low;
    }
}

//���ȿ���
static void print_heat_set(bit_enum s) {
    //    bool ab = false;

    if (s) {
        print_hot_port2->BSRR = (bit_move(print_hot_4pin) | bit_move(print_hot_1pin)); //| bit_move(print_hot_5pin) | bit_move(print_hot_6pin)
                                                                                       //print_hot_port1->BSRR = (bit_move(print_hot_1pin) );//| bit_move(print_hot_2pin) | bit_move(print_hot_3pin)
    } else {
        print_hot_port1->BRR = (bit_move(print_hot_1pin) | bit_move(print_hot_4pin)); // | bit_move(print_hot_2pin) | bit_move(print_hot_3pin)
        //print_hot_port1->BRR = (bit_move(print_hot_4pin) );//| bit_move(print_hot_5pin) | bit_move(print_hot_6pin)
    }
}

//-------------//

//
static void ic_rst_set(bit_enum s) {
    if (s) {
        ic_card_rst_high;
    } else {
        ic_card_rst_low;
    }
}

static void ic_clk_set(bit_enum s) {
    if (s) {
        ic_card_clk_high;
    } else {
        ic_card_clk_low;
    }
}

static void ic_io_set(bit_enum s) {
    if (s) {
        ic_card_io_high; //GPIOD_PIN_10 :����ߵ�ƽ
    } else {
        ic_card_io_low; //GPIOD_PIN_10 :����͵�ƽ
    }
}

//�Ĵ�������
//����:ͬһ��IO��ͬʱ����������?????
static bit_enum ic_io_read(void) {
    ic_card_io_high; //GPIOD_PIN_10 :����ߵ�ƽ
    g_delay_us(1);
    return ((bit_enum)(ic_card_dat_read())); //GPIOD_PIN_10 ����ɼ�
}

static bit_enum ic_clk_read(void) {
    ic_card_clk_high;

    g_delay_us(1);

    return ((bit_enum)(ic_card_clk_read()));
}

//***********************************************************************************************//
//***********************************************************************************************//
static void hard_memory_init(void) {
    pt487fb_driver_need_struct pt487fb_driver_need;
    fatfs_gbk_struct fatfs_gbk;
    ic4442_driver_need_struct ic4442_driver_need;
    iic_base_driver_need_struct iic_base_driver_need;
    lcd13264_ram_need_struct lcd13264_ram_need;

    pt487fb_driver_need.prt_mb_set = print_mb_set;
    pt487fb_driver_need.prt_mnb_set = print_mnb_set;
    pt487fb_driver_need.prt_ma_set = print_ma_set;
    pt487fb_driver_need.prt_mna_set = print_mna_set;
    pt487fb_driver_need.prt_di_set = print_di_set;
    pt487fb_driver_need.prt_clk_set = print_clk_set;
    pt487fb_driver_need.prt_lat_set = print_lat_set;
    pt487fb_driver_need.prt_heat_set = print_heat_set;
    pt487fb_driver_need.prt_font = ((void (*)(uint, uchar *))read_flash_fonts);
    print_data_init(pt487fb_driver_need);

    fatfs_gbk.gbk_unicode = ((uint(*)(bit_enum, uchar *, uchar *, uint))gbk_unicode);
    fatfs_fname_gbk_init(fatfs_gbk);

    ic4442_driver_need.dly_us = g_delay_us;
    ic4442_driver_need.rst_set = ic_rst_set;
    ic4442_driver_need.io_set = ic_io_set;
    ic4442_driver_need.clk_set = ic_clk_set;
    ic4442_driver_need.io_read = ic_io_read;
    ic4442_data_init(ic4442_driver_need);

    iic_base_driver_need.dly_us = g_delay_us;
    iic_base_driver_need.sda_set = ic_io_set;
    iic_base_driver_need.scl_set = ic_clk_set;
    iic_base_driver_need.sda_read = ic_io_read;
    iic_base_driver_need.scl_read = ic_clk_read;
    iic_base_data_init(iic_base_driver_need);

    lcd13264_ram_need.lcd_scl_set = lcd13264_scl_set;
    lcd13264_ram_need.lcd_cs_set = lcd13264_cs_set;
    lcd13264_ram_need.lcd_rst_set = lcd13264_rst_set;
    lcd13264_ram_need.lcd_a0_set = lcd13264_a0_set;
    lcd13264_ram_need.lcd_io_set = lcd13264_io_set;
    lcd13264_ram_need.lcd_delayms = g_delay_ms;
    lcd13264_ram_need.lcd_font = ((void (*)(uint, uchar *))read_flash_fonts);
    lcd13264_data_init(lcd13264_ram_need);
}

/****************************************************************************
* ����:    init_drv_cfg ()
* ���ܣ�����STM3210X GPIO ����
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void init_drv_cfg(void) {
    hard_memory_init();

    RCC_Configuration();
    NVIC_Configuration();

    Timer2_Configuration();
    Timer4_Configuration();
    Timer6_Configuration();

    Uart_Configuration();
    Can_Configuration();
    Can2_Configuration();
    EXTI15_Configuration();

    DVR_IO_INPUT();
    DVR_IO_OUTPUT();
    DVR_PWR_OUTPUT();
    SPI_FLASH_Init();
    GSM_SPK_LM4903();
    config_free_gpio();

    I2C1_Init();
    print_driver_init(print_speed_set);

#if (P_CAN == _CAN_BASE)
    InitCan();
    InitCan2();
#endif

    EN_RESET_HI35XX;
    //DIS_RESET_HI35XX;

    Adc1_Init();
    DMA1_Init();
    ACC_INPUT();
    SOS_INPUT();

#if (DEBUG_EN_WATCH_DOG)
    WatchDog_Configuration(); //�������Ź�        chwsh
#else
#endif

    self_check();
    EN_TURN_OFF;

#if (DEBUG_EN_WATCH_DOG)
    //EN_WATCH_DOG;
#else
#endif
}

//��ӡ����
//����en    TRUE��ʾ��    FALSE��ʾ�ر�
void print_start_ctrl(bit_enum en) {
    if (en) {
        TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
        TIM_Cmd(TIM6, ENABLE);
        pow_print_on;
    } else {
        TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM6, DISABLE);
        pow_print_off;
    }
}

//�߾��ȶ�λģ�顾ǧѰMC180M��page11
/**
 * @brief [3]TXD0 Ϊ����ܽţ���ǧѰMC180M������
 *              ��ģ���ϵ�100ms���豣�ֵ͵�ƽ��
 *              ����ģ�齫���빤��ģʽ�޷�����������
 *              ���飺��TXD0�����Ĺܽ���ģ���ϵ�100ms��ά����������ʼ������Ϊ�����������롣
 */
// static void uart4_rx_low_cfg_gps

//0,off, //1, on
void pwr_gps_lsm_lcd_inOut5v0(unsigned char on, char *who) {
    if (who == NULL) {
        return;
    }

#define DEBUG_HERE_202209121606
    //#undef DEBUG_HERE_202209121606

#ifdef DEBUG_HERE_202209121606
    static unsigned char state = 0xFF;
    if (on != state) {
        state = on;
        logi("pwr_gps_lsm_lcd_inOut5v0 %d, %s", state, who);
#endif
        if (on) {
            pwr_on_gps_lsm_lcd_inOut5v0();
            sys_bits1.bits.flg_lsm_pwr_on = 1;
        } else {
            pwr_off_gps_lsm_lcd_inOut5v0();
            sys_bits1.bits.flg_lsm_pwr_on = 0;
        }
#ifdef DEBUG_HERE_202209121606
    }
#endif
}

void rk_pwr_en(void) {
    GPIO_SetBits(TYPE_HDD_PWR, PORT_HDD_PWR);
    log_write(event_rk_boot_en);
    sys_bits1.bits.flg_rk_1717_first = 0;
}
void rk_pwr_un(void) {
    GPIO_ResetBits(TYPE_HDD_PWR, PORT_HDD_PWR);
    log_write(event_rk_boot_un);
    sys_bits1.bits.flg_rk_1717_first = 0;
}

void rk_rst_en(void) {
    GPIO_SetBits(TYPE_RESET_HI35XX, PORT_RESET_HI35XX);
    log_write(event_rk_rst_en);
    sys_bits1.bits.flg_rk_1717_first = 0;
}
void rk_rst_un(void) {
    GPIO_ResetBits(TYPE_RESET_HI35XX, PORT_RESET_HI35XX);
    log_write(event_rk_rst_un);
    sys_bits1.bits.flg_rk_1717_first = 0;
}
