/**
  ******************************************************************************
  * @file    send.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  所有GPRS数据发送管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void) {
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */

    log_write(event_mcu_boot_HardFault);

    while (1) {
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */

    log_write(event_mcu_boot_MemManage);

    while (1) {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */

    log_write(event_mcu_boot_BusFault);

    while (1) {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */

    log_write(event_mcu_boot_UsageFault);

    while (1) {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void) {
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void) {
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void) {
    u8 i;
    for (i = 0; i < 250; i++)
        ;
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void) {
    if (s_TIMER_Delay_ms > 0)
        s_TIMER_Delay_ms--;
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : 
* Input          :  None
* Output         : None
* Return         :  None
*******************************************************************************/
void EXTI15_10_IRQHandler(void) {
    u8 uAf;

    if (EXTI_GetITStatus(EXTI_Line15) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line15);
        uAf = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_15);
        if (uAf == 0) {
            speed_plus++;
        }
    }
}

//5毫秒
//六轴LSM6DS3TR-C:
//硬件定时器2:
void TIM2_IRQHandler(void) {
    //static u32 state_jif = 0;
    static unsigned char tic = 0;

    if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
        jiffies += 5;

        if (++tic >= 4) {
            tic = 0;
            sys_bits1.bits.flg_lsm_read = 1;
        }
    }
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : 定时器3更新中断
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void) { //硬件定时器4:
    if (TIM_GetITStatus(TIM4, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        //buzzer_int_hdl();
    }
}

void TIM6_IRQHandler(void) { //硬件定时器6:
    if (TIM_GetITStatus(TIM6, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        print_base_task_hdl();
    }
}

void USART1_IRQHandler(void) {
    vu16 r1_dat;
    vu16 t1_dat;

    //发送中断
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET) {
        USART_ClearITPendingBit(USART1, USART_IT_TXE);
        if (t1_tail != t1_head) {
            t1_dat = *t1_tail;
            USART_SendData(USART1, t1_dat & 0x00ff);
            incp_t1(t1_tail, 1);
            jif_t1 = jiffies;
        } else {
            uart1_busy = false;
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
            USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        }
    }

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) { //判断发生接收中断
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);      //清除中断标志
        //GPIO_WriteBit(GPIOA, GPIO_Pin_10, (BitAction)0x01);         //开始传输
        r1_dat = USART_ReceiveData(USART1) & 0x00FF; //接收数据，整理除去前两位
        if (over_r1())
            return;
        *r1_head = (u8)r1_dat;
        incp_r1(r1_head, 1);
        jif_r1 = jiffies;
    }
}

//3G妯″潡涓?  4G EC200
void USART2_IRQHandler(void) {
    vu16 r2_dat;
    vu16 t2_dat;

    //发送中断
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET) {
        USART_ClearITPendingBit(USART2, USART_IT_TXE);
        if (gsm_send_tail != gsm_send_head) {
            t2_dat = *gsm_send_tail;
            USART_SendData(USART2, t2_dat & 0x00ff);
            incp_gsm_send(gsm_send_tail, 1);

            gsm_send_jfs = jiffies;
        } else {
            gsm_send_busy = false;
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
            USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        }
    }

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) { //判断发生接收中断
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);      //清除中断标志
        r2_dat = USART_ReceiveData(USART2) & 0x00FF;         //接收数据，整理除去前两位

        if (over_gsm_rev()) {
            gsm_rev_jfs = jiffies;
            return;
        }
        *gsm_rev_head = (u8)r2_dat;
        incp_gsm_rev(gsm_rev_head, 1); //gprs_rev_head   gsm_rev_tail
        gsm_rev_jfs = jiffies;

#if (1)
        //if (u1_m.cur.b.debug) { //||(!u4_m.cur.b.debug&&u1_m.cur.b.handset)
        write_uart1_hex((u8)r2_dat); //向串口打印调试信息//休眠位置上报:
//}
#endif
    }
}

//ADAS
void USART3_IRQHandler(void) {
    vu16 r3_dat;
    vu16 t3_dat;

    //发送中断
    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET) {
        USART_ClearITPendingBit(USART3, USART_IT_TXE);
        if (t3_tail != t3_head) {
            t3_dat = *t3_tail;
            USART_SendData(USART3, t3_dat & 0x00ff);
            incp_t3(t3_tail, 1);
            jif_t3 = jiffies;
        } else {
            uart3_busy = false;
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
            USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
        }
    }

    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) { //判断发生接收中断
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);      //清除中断标志
        //GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)0x01);       //开始传输
        r3_dat = USART_ReceiveData(USART3) & 0x00FF; //接收数据，整理除去前两位

        s_Tr9Com.buf[s_Tr9Com.uiRxdIn++] = (u8)r3_dat;
        if (s_Tr9Com.uiRxdIn >= TR9_COM_BUF_LEN)
            s_Tr9Com.uiRxdIn = 0;
        s_Tr9Com.uiTimeOut = jiffies;
    }
}

//澶栭儴鎺ュ彛
//外部串口
void UART4_IRQHandler(void) {
    vu16 r4_dat;
    vu16 t4_dat;

    //发送中断
    if (USART_GetITStatus(UART4, USART_IT_TXE) != RESET) {
        USART_ClearITPendingBit(UART4, USART_IT_TXE);
        if (t4_tail != t4_head) {
            t4_dat = *t4_tail;
            USART_SendData(UART4, t4_dat & 0x00ff);
            incp_t4(t4_tail, 1);
            jif_t4 = jiffies;
        } else {
            uart4_busy = false;
            USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
            USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
        }
    }

    if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) { //判断发生接收中断
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);      //清除中断标志
        //GPIO_WriteBit(GPIOC, GPIO_Pin_11, (BitAction)0x01); //开始传输
        r4_dat = USART_ReceiveData(UART4) & 0x00FF; //接收数据，整理除去前两位
        if (over_r4())
            return;
        *r4_head = (u8)r4_dat;
        incp_r4(r4_head, 1);
        jif_r4 = jiffies;
    }
}

//GPS
void UART5_IRQHandler(void) {
    vu16 r5_dat;
    vu16 t5_dat;

    if (USART_GetITStatus(UART5, USART_IT_TXE) != RESET) {
        USART_ClearITPendingBit(UART5, USART_IT_TXE);

        if (gps_send_tail != gps_send_head) {
            t5_dat = *gps_send_tail;
            USART_SendData(UART5, t5_dat & 0x00ff);
            incp_gps_send(gps_send_tail, 1);
            gps_send_jfs = jiffies;
        } else {
            gps_send_busy = false;
            USART_ITConfig(UART5, USART_IT_TXE, DISABLE);
            USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
        }
    }

    if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET) { //判断发生接收中断
        USART_ClearITPendingBit(UART5, USART_IT_RXNE);      //清除中断标志

        r5_dat = USART_ReceiveData(UART5) & 0x00FF; //接收数据，整理除去前两位

        s_GpsCom.buf[s_GpsCom.uiRxdIn++] = (u8)r5_dat;
        if (s_GpsCom.uiRxdIn >= G_RECE_SIZE)
            s_GpsCom.uiRxdIn = 0;

        if (u1_m.cur.b.tt) {
            write_uart1_hex(r5_dat);
        }
        s_GpsCom.uiTimeOut = jiffies;
        gps_rev_jfs = jiffies;
    }
}

/*******************************************************************************
* Function Name  : CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None		  18fe08d0
*******************************************************************************/

void CAN1_RX0_IRQHandler(void) {
/*
	vu32 tid = 0;
	if(SET == CAN_GetITStatus(CAN1,CAN_IT_FF0))         //FIFO0满
	{
		CAN_ClearITPendingBit(CAN1,CAN_IT_FF0);         //清标志位
	}
	else if(SET == CAN_GetITStatus(CAN1,CAN_IT_FOV0))   //FIFO0溢出
	{
		CAN_ClearITPendingBit(CAN1,CAN_IT_FOV0);        //清标志位
	}
	else
	{
		
	}		*/
#if 0
	vu32 tid = 0;
  
	if(SET == CAN_GetITStatus(CAN1,CAN_IT_FF0))         //FIFO0满
	{
		CAN_ClearITPendingBit(CAN1,CAN_IT_FF0);         //清标志位
	}
	else if(SET == CAN_GetITStatus(CAN1,CAN_IT_FOV0))   //FIFO0溢出
	{
		CAN_ClearITPendingBit(CAN1,CAN_IT_FOV0);        //清标志位
	}
	else
	{
		RxMessage.StdId=0x00 ;							//标准帧标识符
		RxMessage.ExtId=0x00 ;							//扩展帧标识符
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);       //接收数据
		Can_param.CAN1_data_Flag = true ;
		
		/*
		tid = RxMessage.ExtId;			                //读取接收到扩展帧的ID
		switch( tid )
		{
			case 0x18fef100:	//提取车速信号  -- 不定位情况下使用 
			  	get_wheel_speed_from_can(RxMessage.Data);
				break;
			case 0x0cf00400:	//提取转速信号
				break;   
			case 0x0c000000:

				break;
			default:
				break;	
		}	 
		*/
		
		//print_can((u8*)&RxMessage, sizeof(RxMessage) );
	}
#endif
    //    u8 mse;
    u16 msec;
    //static vu32 gather_time ;

    if (SET == CAN_GetITStatus(CAN1, CAN_IT_FF0)) //FIFO0满
    {
        CAN_ClearITPendingBit(CAN1, CAN_IT_FF0);          //清标志位
    } else if (SET == CAN_GetITStatus(CAN1, CAN_IT_FOV0)) //FIFO0溢出
    {
        CAN_ClearITPendingBit(CAN1, CAN_IT_FOV0); //清标志位
    } else {
        RxMessage.StdId = 0x00; //标准帧标识符
        RxMessage.ExtId = 0x00; //扩展帧标识符

        CAN_Receive(CAN1, CAN_FIFO0, &RxMessage); //接收数据
        Can_id_config.bit.chnnal = 0;             //CAN1通道号
        Can_param.CAN1_data_Flag = true;          //

#if 0
		Can_rev_tail = 	Can_rev_head ;
		_memcpy(Can_rev_head,(u8*)&RxMessage.ExtId, 	4 ) ;
		
		incp_Can(Can_rev_head,4);
		
		_memcpy(Can_rev_head,RxMessage.Data, 	8 ) ;
		
		incp_Can(Can_rev_head,8);

#else

        if (false == Can_param_get.CAN1_send_flag) {
            msec = jiffies % 1000;
            msec = _HexToBCD(msec);

            cansendbuf.datbuf[cansendbuf.cnt].time[0] = mix.time.hour;
            cansendbuf.datbuf[cansendbuf.cnt].time[1] = mix.time.min;
            cansendbuf.datbuf[cansendbuf.cnt].time[2] = mix.time.sec;
            cansendbuf.datbuf[cansendbuf.cnt].time[3] = (u8)((msec & 0xff00) >> 8);
            cansendbuf.datbuf[cansendbuf.cnt].time[4] = (u8)(msec & 0xff);
            cansendbuf.datbuf[cansendbuf.cnt].can_id = RxMessage.ExtId;
            _memcpy(cansendbuf.datbuf[cansendbuf.cnt].data, RxMessage.Data, 8);

            if (cansendbuf.cnt >= 79)
                cansendbuf.cnt = 79;
            else
                cansendbuf.cnt++;

#endif
    }
}
}

/*******************************************************************************
* Function Name  : CAN1_TX_IRQHandler
* Description    : 
*                  requests.
* Input          : None
* Output         : None
* Return         : None		  18fe08d0
*******************************************************************************/
void CAN1_TX_IRQHandler(void) {
    if (CAN_msg_num[0]) {
        if (SET == CAN_GetITStatus(CAN1, CAN_IT_RQCP0)) //邮箱不等于空
        {
            CAN_ClearITPendingBit(CAN1, CAN_IT_RQCP0); //清理等待位
            CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);   //失能邮箱空
            CAN_msg_num[0] = 0;
        }
    }

    if (CAN_msg_num[1]) {
        if (SET == CAN_GetITStatus(CAN1, CAN_IT_RQCP1)) {
            CAN_ClearITPendingBit(CAN1, CAN_IT_RQCP1);
            CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);
            CAN_msg_num[1] = 0;
        }
    }

    if (CAN_msg_num[2]) {
        if (SET == CAN_GetITStatus(CAN1, CAN_IT_RQCP2)) {
            CAN_ClearITPendingBit(CAN1, CAN_IT_RQCP2);
            CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);
            CAN_msg_num[2] = 0;
        }
    }
}

/*******************************************************************************
* Function Name  : CAN1_SCE_IRQHandler
* Description    : ERRR
*                  requests.
* Input          : None
* Output         : None
* Return         : None		
*******************************************************************************/
void CAN1_SCE_IRQHandler(void) {
    CAN_ClearITPendingBit(CAN1, CAN_IT_SLK);
    CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);
    CAN_ClearITPendingBit(CAN1, CAN_IT_BOF);
    CAN_ClearITPendingBit(CAN1, CAN_IT_LEC);
    CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);
    CAN_ClearITPendingBit(CAN1, CAN_IT_EWG);
    CAN_ClearITPendingBit(CAN1, CAN_IT_EPV);
    CAN_ClearITPendingBit(CAN1, CAN_IT_LEC);
    CAN_ClearITPendingBit(CAN1, CAN_IT_EPV);
    //CAN_MSR_ERRI
    CAN_ClearITPendingBit(CAN1, CAN_MSR_ERRI);

    CAN_ClearITPendingBit(CAN1, CAN_MSR_ERRI);
}

#if 1

void CAN2_RX0_IRQHandler(void) {
    //    u8 mse;
    u16 msec;

    if (SET == CAN_GetITStatus(CAN2, CAN_IT_FF0)) //FIFO0满
    {
        CAN_ClearITPendingBit(CAN2, CAN_IT_FF0);          //清标志位
    } else if (SET == CAN_GetITStatus(CAN2, CAN_IT_FOV0)) //FIFO0溢出
    {
        CAN_ClearITPendingBit(CAN2, CAN_IT_FOV0); //清标志位
    } else {
        RxMessage2.StdId = 0x00; //标准帧标识符
        RxMessage2.ExtId = 0x00; //扩展帧标识符

        CAN_Receive(CAN2, CAN_FIFO0, &RxMessage2); //接收数据
        Can_id_config2.bit.chnnal = 1;             //CAN2通道号

        Can_param.CAN2_data_Flag = true; //

        if (false == Can_param_get.CAN2_send_flag) {
            msec = jiffies % 1000;
            msec = _HexToBCD(msec);

            cansendbuf2.datbuf[cansendbuf2.cnt].time[0] = mix.time.hour;
            cansendbuf2.datbuf[cansendbuf2.cnt].time[1] = mix.time.min;
            cansendbuf2.datbuf[cansendbuf2.cnt].time[2] = mix.time.sec;
            cansendbuf2.datbuf[cansendbuf2.cnt].time[3] = (u8)((msec & 0xff00) >> 8);
            cansendbuf2.datbuf[cansendbuf2.cnt].time[4] = (u8)(msec & 0xff);
            cansendbuf2.datbuf[cansendbuf2.cnt].can_id = RxMessage2.ExtId;
            _memcpy(cansendbuf2.datbuf[cansendbuf2.cnt].data, RxMessage2.Data, 8);

            if (cansendbuf2.cnt >= 79)
                cansendbuf2.cnt = 79;
            else
                cansendbuf2.cnt++;
        }
    }
}

void CAN2_TX_IRQHandler(void) {
    if (CAN_msg_num2[0]) {
        if (SET == CAN_GetITStatus(CAN2, CAN_IT_RQCP0)) //邮箱不等于空
        {
            CAN_ClearITPendingBit(CAN2, CAN_IT_RQCP0); //清理等待位
            CAN_ITConfig(CAN2, CAN_IT_TME, DISABLE);   //失能邮箱空
            CAN_msg_num2[0] = 0;
        }
    }

    if (CAN_msg_num2[1]) {
        if (SET == CAN_GetITStatus(CAN2, CAN_IT_RQCP1)) {
            CAN_ClearITPendingBit(CAN2, CAN_IT_RQCP1);
            CAN_ITConfig(CAN2, CAN_IT_TME, DISABLE);
            CAN_msg_num2[1] = 0;
        }
    }

    if (CAN_msg_num2[2]) {
        if (SET == CAN_GetITStatus(CAN2, CAN_IT_RQCP2)) {
            CAN_ClearITPendingBit(CAN2, CAN_IT_RQCP2);
            CAN_ITConfig(CAN2, CAN_IT_TME, DISABLE);
            CAN_msg_num2[2] = 0;
        }
    }
}

void CAN2_SCE_IRQHandler(void) {
    CAN_ClearITPendingBit(CAN2, CAN_IT_SLK);
    CAN_ClearITPendingBit(CAN2, CAN_IT_WKU);
    CAN_ClearITPendingBit(CAN2, CAN_IT_BOF);
    CAN_ClearITPendingBit(CAN2, CAN_IT_LEC);
    CAN_ClearITPendingBit(CAN2, CAN_IT_ERR);
    CAN_ClearITPendingBit(CAN2, CAN_IT_EWG);
    CAN_ClearITPendingBit(CAN2, CAN_IT_EPV);
    CAN_ClearITPendingBit(CAN2, CAN_IT_LEC);
    CAN_ClearITPendingBit(CAN2, CAN_IT_EPV);
    //CAN_MSR_ERRI
    CAN_ClearITPendingBit(CAN2, CAN_MSR_ERRI);
    CAN_ClearITPendingBit(CAN2, CAN_MSR_ERRI);
}

#endif

/******************************************************************************/
/*                 STM32F10x 外设 中断 服务函数                               */
/*  增加需要的外设中断函数在下面。中断的函数名字都已经在startup_stm32f10x_xx.s*/
/*  的文件中定义好了，请参照它来写。                                          */
/******************************************************************************/

/**
  * @brief  This function handles USB-On-The-Go FS global interrupt request.
  * @note	None
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void) {
    //g_SYS_IRQCnt.nOTG_FS_Cnt++;

    //BSP_USB_IRQHandle();
}

/**
  * @}
  */
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
