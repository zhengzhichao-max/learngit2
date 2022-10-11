/**
  ******************************************************************************
  * @file    drv_cfg_tr9.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief    配置与硬件相关的所有端口或者资源
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

#include "diskio.h"

//----------------------------------------//
//----------------------------------------//
//硬件资源分配表  CKP  2015-03-25
//
//定时器
//SYSTICK:  未使用
//TIM2:     系统时钟
//TIM3:     蜂鸣器
//TIM4:     蜂鸣器
//TIM6:     打印机
//----------------------------------------//
//----------------------------------------//

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;
ErrorStatus HSEStartUpStatus;

//***********************************************************************************************//
void g_delay_us(uchar t) {
    delay_us(t + 2);
}

//毫秒级延时
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
        RCC_PCLK2Config(RCC_HCLK_Div4); //(72M/4=18M  CKP)   APB2时钟为系统时钟

        /* PCLK1 = HCLK/4 */
        RCC_PCLK1Config(RCC_HCLK_Div4); //(72M/4=18M  CKP)   APB1时钟为系统时钟

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
* 名称:    NVIC_Configuration ()
* 功能：中断向量优先级别配置
* 入口参数：无
* 出口参数：无
****************************************************************************/
void NVIC_Configuration(void) {
    NVIC_InitTypeDef NVIC_InitStructure;

#if (TR_USING_IAP)
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2800);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
#endif

    //优先级别组 II
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
    NVIC_Init(&NVIC_InitStructure); //设置端口中断 PA2

    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); //设置定时器4中断

    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure); //设置定时器6中断

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

    //CAN2中断初始化
    NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn; //USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

#endif
}

/****************************************************************************
* 名称:    Timer2_Configuration ()
* 功能：每5ms 产生一次中断
* 入口参数：无
* 出口参数：无
****************************************************************************/
void Timer2_Configuration(void) { //硬件定时器2:
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

//蜂鸣器节拍配置     //溢出周期  25毫秒
static void Timer4_Configuration(void) { //硬件定时器4:
    TIM_TimeBaseInitTypeDef tim_base_init_struct;

    tim_base_init_struct.TIM_Period = 249;
    tim_base_init_struct.TIM_Prescaler = 3599;

    tim_base_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;     //不分频
    tim_base_init_struct.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式

    //tim_base_init_struct.TIM_RepetitionCounter = ;	 //无关参数
    TIM_TimeBaseInit(TIM4, (&tim_base_init_struct));
}

//36MHz
//打印机定时配置
static void Timer6_Configuration(void) { //硬件定时器6:
    TIM_TimeBaseInitTypeDef tim_base_init_struct;

    tim_base_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;     //不分频
    tim_base_init_struct.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    tim_base_init_struct.TIM_Period = 40;                      //39 ?
    tim_base_init_struct.TIM_Prescaler = 35;                   //35与分屏器

    TIM_TimeBaseInit(TIM6, (&tim_base_init_struct));

    print_start_ctrl(_false_); //关闭打印
}

/****************************************************************************
* 名称:    Can_Configuration ()
* 功能：CAN1 功能配置
* 入口参数：无
* 出口参数：无
****************************************************************************/
void Can_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    //重定义地址1模式
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
    /* Configure CAN Remap   重影射 */
    GPIO_PinRemapConfig(GPIO_Remap1_CAN1, ENABLE);
}
/****************************************************************************
* 名称:    Can2_Configuration ()
* 功能：CAN2 功能配置
* 入口参数：无
* 出口参数：无
****************************************************************************/
void Can2_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    //重定义地址1模式
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
    /* Configure CAN Remap   重影射 */
    //GPIO_PinRemapConfig(GPIO_Remap_CAN2, ENABLE);
}

/****************************************************************************
* 名称:    Uart_Configuration ()
* 功能：串口功能配置
* 入口参数：无
* 出口参数：无
****************************************************************************/
void Uart_Configuration(void) {
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#pragma region //USART0: PA9 & PA10
    /*
    配置串口1 MIX UART
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
    配置串口2  GSM
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
    配置串口3  TR9-HISEE
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
    配置串口4
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
 * @brief 高精度定位模组【千寻MC180M】page11
 *          [3]TXD0 为输出管脚（【千寻MC180M】），
 *              在模组上电100ms内需保持低电平；
 *              否则模组将进入工程模式无法正常工作。
 *              建议：与TXD0相连的管脚在模组上电100ms内维持下拉，或始终配置为（下拉）输入。
 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //原来是GPIO_Mode_IN_FLOATING//高精度【千寻MC180M】GPIO_Mode_IPD
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
* 名称：INPUT1_high_INPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void INPUT1_HIGH1_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_I7_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_I7_HIGH, &GPIO_InitStructure);
}

/****************************************************************************
* 名称：INPUT1_LOW1_INPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void INPUT1_LOW1_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_COVER_LOW;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_COVER_LOW, &GPIO_InitStructure);
}

/****************************************************************************
* 名称：INPUT1_LOW2_INPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void INPUT1_LOW2_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_LIFT_LOW;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_LIFT_LOW, &GPIO_InitStructure);
}

/****************************************************************************
* 锟斤拷锟狡ｏ拷EXTI_Configuration
* 锟斤拷锟杰ｏ拷锟叫讹拷锟斤拷锟斤拷锟斤拷锟斤拷
* 锟斤拷诓锟斤拷锟斤拷锟斤拷锟?
* 锟斤拷锟节诧拷锟斤拷锟斤拷锟斤拷
***************************************************************************/
static void EXTI15_Configuration(void) {
    EXTI_InitTypeDef EXTI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    ///speed
    GPIO_InitStructure.GPIO_Pin = PORT_SPEED_PLUS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_SPEED_PLUS, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource15); //锟剿匡拷PA2
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;                  //EXTI0_IRQn
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;     //锟叫断凤拷式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //锟铰斤拷锟截达拷锟斤拷
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
* 名称：SOS_INPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void SOS_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_INPUT_SOS;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_INPUT_SOS, &GPIO_InitStructure);
}

/****************************************************************************
* 名称：DVR_IO_INPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void DVR_IO_INPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    port_dir_config(ic_card_in_port, bit_move(ic_card_in_pin), false); //IC卡插入检测

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

    //锁输入
    GPIO_InitStructure.GPIO_Pin = PORT_LOCK_HIGH;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(TYPE_LOCK_HIGH, &GPIO_InitStructure);
}

/****************************************************************************
* 名称：DVR_IO_OUTPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
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

    //-------   IC卡端口配置   -----------------
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

    //-------   输入端口配置   -----------------
    //------------------------------------------

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = (bit_move(key_up_pin) | bit_move(key_down_pin) | bit_move(key_menu_pin) | bit_move(key_print_pin) | bit_move(key_ok_pin));
    GPIO_Init(key_port, &GPIO_InitStructure); //配置按键检测端口

#pragma region //5V VOUT1 输出接口初始化
    GPIO_InitStructure.GPIO_Pin = PORT_OUT_5V_VOUT1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_OUT_5V_VOUT1, &GPIO_InitStructure);
#pragma endregion //5V VOUT1 输出接口初始化

#pragma region //复位RK
    GPIO_InitStructure.GPIO_Pin = PORT_RESET_HI35XX;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_RESET_HI35XX, &GPIO_InitStructure);
#pragma endregion //复位RK
}

/****************************************************************************
* 名称：RED_LED_OUTPUT
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
    //海格：RK电源
    GPIO_InitStructure.GPIO_Pin = PORT_HDD_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(TYPE_HDD_PWR, &GPIO_InitStructure);
}

/****************************************************************************
* 名称：GPS_PWR_OUTPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void GPS_PWR_OUTPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_GPS_PWR;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_GPS_PWR, &GPIO_InitStructure);
}

/****************************************************************************
* 名称： relay_output_init
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void relay_output_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_RELAY_CTRL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_RELAY_CTRL, &GPIO_InitStructure);
}

/****************************************************************************
* 名称：GSM_PWR_OUTPUT
* 功能：端口配置
* 入口参数：无
* 出口参数：无
***************************************************************************/
void GSM_PWR_OUTPUT(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = PORT_GSM_PWR;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(TYPE_GSM_PWR, &GPIO_InitStructure);
}

/****************************************************************************
* 名称：GSM_SPK_LM4903
* 功能：端口配置
* 入口参数：无
* 出口参数：无
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
//-----                    端口配置函数                       ----//
//*********//
//该函数仅能设置端口的输入与输出，而复用功能及AD检测功能不能实现
//*********//
//参数GPIOx为端口号，详见库函数
//参数pinx为引脚号
//参数dir为控制的方向    TRUE为输出方向    FALSE为输入方向
//----------------------------------------------------------------//
//****************************************************************//
void port_dir_config(GPIO_TypeDef *GPIOx, u16 pinx, bool dir) {
    GPIO_InitTypeDef GPIO_InitStructure;

    if (dir) //输出模式
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    } else //输入模式
    {
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    }

    GPIO_InitStructure.GPIO_Pin = pinx;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

//****************************************************************//
//----------------------------------------------------------------//
//-----              SPI接口配置成主模式                      ----//
//注意!!!  CS端口为手动控制而非自动控制
//
//参数SPIx为端口号其值应调用库文件如SPI1格式
//参数bd_speed为分频系数，其值应调用库文件如SPI_BaudRatePrescaler_4格式
//----------------------------------------------------------------//
//****************************************************************//
void spi_host_config(SPI_TypeDef *SPIx, uint bd_speed) {
    SPI_InitTypeDef spi_init_struct;

    spi_init_struct.SPI_BaudRatePrescaler = bd_speed;                //设置分频系数
    spi_init_struct.SPI_CPHA = SPI_CPHA_2Edge;                       //数据采集从第二个时钟沿开始
    spi_init_struct.SPI_CPOL = SPI_CPOL_High;                        //空闲状态为高电平
    spi_init_struct.SPI_CRCPolynomial = 7;                           //默认值
    spi_init_struct.SPI_DataSize = SPI_DataSize_8b;                  //8位数据格式
    spi_init_struct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //两线全双工模式
    spi_init_struct.SPI_FirstBit = SPI_FirstBit_MSB;                 //高位在前
    spi_init_struct.SPI_Mode = SPI_Mode_Master;                      //主机模式
    spi_init_struct.SPI_NSS = SPI_NSS_Soft;                          //SS端口由软件控制
    SPI_Init(SPIx, &spi_init_struct);                                //SPI_FLASH芯片接口

    SPI_NSSInternalSoftwareConfig(SPIx, SPI_NSSInternalSoft_Set); //软件置位，主机
    SPI_SSOutputCmd(SPIx, DISABLE);                               //禁止主机硬件输出控制SS端口
    SPI_Cmd(SPIx, ENABLE);
}

//*****************************************************//
//*******          发送或接收一个字节           *******//
//参数SPIx为端口号
//参数s_dat为待发送的一个字节
//函数返回值为接收到的一个字节
//*****************************************************//
uchar spi_read_write_byte(SPI_TypeDef *SPIx, uchar s_dat) {
    uchar temp;

    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET) {
        ; //等待上一包数据发送完成
    }

    SPI_I2S_SendData(SPIx, s_dat); //开始向发送缓冲器中装载数据

    while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET) {
        ; //等待一包数据接收完成
    }

    temp = SPI_I2S_ReceiveData(SPIx); //取出接收到的数据

    return temp; //返回读出的数据
}

void slv_mcu_io_config() {
    GPIO_InitTypeDef GPIO_InitStructure;

    //-------   输出端口配置   -----------------
    //------------------------------------------

    port_dir_config(lcd_io_port, bit_move(lcd_io_pin), true);
    port_dir_config(lcd_scl_port, bit_move(lcd_scl_pin), true);
    port_dir_config(lcd_a0_port, bit_move(lcd_a0_pin), true);
    port_dir_config(lcd_cs_port, bit_move(lcd_cs_pin), true);
    port_dir_config(lcd_rst_port, bit_move(lcd_rst_pin), true);
    port_dir_config(lcd_bl_port, bit_move(lcd_bl_pin), true);

    //-------   输入端口配置   -----------------
    //------------------------------------------

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = (bit_move(key_up_pin) | bit_move(key_down_pin) | bit_move(key_menu_pin) | bit_move(key_print_pin) | bit_move(key_ok_pin));
    GPIO_Init(key_port, &GPIO_InitStructure); //配置按键检测端口

#if 0
    //-------     打印机接口   ----------------
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

    port_dir_config(p_det_port, bit_move(p_det_pin), false);  //纸张检测

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_Pin = bit_move(p_rtm_pin);   //温度检测
    GPIO_Init(p_rtm_port, &GPIO_InitStructure);

    print_power_dis;
    p_lat_high;
#endif
}

/****************************************************************************
* 名称:    config_free_gpio ()
* 功能：空闲口配置成为输入，下拉
* 入口参数：无
* 出口参数：无
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
名    称：WatchDog_Configuration
功    能：看门狗初始化配置
参    数：无
输    出：无
编写日期：2011-01-21
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

//液晶scl端口控制
static void lcd13264_scl_set(bit_enum s) {
    if (s) {
        lcd_scl_H;
    } else {
        lcd_scl_L;
    }
}

//液晶cs端口控制
static void lcd13264_cs_set(bit_enum s) {
    if (s) {
        lcd_cs_H;
    } else {
        lcd_cs_L;
    }
}

//液晶rst端口控制
static void lcd13264_rst_set(bit_enum s) {
    if (s) {
        lcd_rst_H;
    } else {
        lcd_rst_L;
    }
}

//液晶a0端口控制
static void lcd13264_a0_set(bit_enum s) {
    if (s) {
        lcd_a0_H;
    } else {
        lcd_a0_L;
    }
}

//液晶io端口控制
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

//加热控制
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
        ic_card_io_high; //GPIOD_PIN_10 :输出高电平
    } else {
        ic_card_io_low; //GPIOD_PIN_10 :输出低电平
    }
}

//寄存器操作
//疑问:同一个IO口同时做输入和输出?????
static bit_enum ic_io_read(void) {
    ic_card_io_high; //GPIOD_PIN_10 :输出高电平
    g_delay_us(1);
    return ((bit_enum)(ic_card_dat_read())); //GPIOD_PIN_10 输入采集
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
* 名称:    init_drv_cfg ()
* 功能：配置STM3210X GPIO 参数
* 入口参数：无
* 出口参数：无
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
    WatchDog_Configuration(); //开启看门狗        chwsh
#else
#endif

    self_check();
    EN_TURN_OFF;

#if (DEBUG_EN_WATCH_DOG)
    //EN_WATCH_DOG;
#else
#endif
}

//打印控制
//参数en    TRUE表示打开    FALSE表示关闭
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

//高精度定位模组【千寻MC180M】page11
/**
 * @brief [3]TXD0 为输出管脚（【千寻MC180M】），
 *              在模组上电100ms内需保持低电平；
 *              否则模组将进入工程模式无法正常工作。
 *              建议：与TXD0相连的管脚在模组上电100ms内维持下拉，或始终配置为（下拉）输入。
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
