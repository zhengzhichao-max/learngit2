/**
  ******************************************************************************
  * @file    drv_cfg_tr9.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2016-6-6
  * @brief    配置与硬件相关的所有端口或者资源
  ******************************************************************************
  * @attention
  ******************************************************************************
**/

#ifndef __DRV_CFG_TR9_H
#define __DRV_CFG_TR9_H

//嵌入汇编配置
#define nop() __nop()
#define cli() __set_PRIMASK(1)
#define sei() __set_PRIMASK(0)
#define wdr() IWDG_ReloadCounter(); //DOG RESET EVERY 7S, FEED EVERY 200MS

//电源控制

//外音功放
#define TYPE_SPK_LM4903 GPIOD
#define PORT_SPK_LM4903 GPIO_Pin_0
#define OPEN_PHONE \
    { GPIO_SetBits(TYPE_SPK_LM4903, PORT_SPK_LM4903); }
#define CLOSE_PHONE \
    { GPIO_ResetBits(TYPE_SPK_LM4903, PORT_SPK_LM4903); }

#define TYPE_LCD_BL_PWR GPIOB
#define PORT_LCD_BL_PWR GPIO_Pin_15
#define EN_LCD_BL_PWR GPIO_ResetBits(TYPE_LCD_BL_PWR, PORT_LCD_BL_PWR)
#define DIS_LCD_BL_PWR GPIO_SetBits(TYPE_LCD_BL_PWR, PORT_LCD_BL_PWR)

//自检
#define EN_SELF_CHECK GPIO_ResetBits(GPIOA, GPIO_Pin_11)
#define DIS_SELF_CHECK GPIO_SetBits(GPIOA, GPIO_Pin_11)

//速度脉冲检测
#define TYPE_SPEED_PLUS GPIOE
#define PORT_SPEED_PLUS GPIO_Pin_15
#define PIN_SPEED_PLUS GPIO_ReadInputDataBit(TYPE_SPEED_PLUS, PORT_SPEED_PLUS)

//继电器控制
#define TYPE_RELAY_CTRL GPIOD
#define PORT_RELAY_CTRL GPIO_Pin_13
#define relay_enable() GPIO_SetBits(TYPE_RELAY_CTRL, PORT_RELAY_CTRL)
#define relay_disable() GPIO_ResetBits(TYPE_RELAY_CTRL, PORT_RELAY_CTRL)
#define relay_get_state() GPIO_ReadInputDataBit(TYPE_RELAY_CTRL, PORT_RELAY_CTRL)

#define TYPE_INPUT_SOS GPIOD
#define PORT_INPUT_SOS GPIO_Pin_8
#define PIN_INPUT_SOS GPIO_ReadInputDataBit(TYPE_INPUT_SOS, PORT_INPUT_SOS)

//自定义低电平检测
#define TYPE_COVER_LOW GPIOC
#define PORT_COVER_LOW GPIO_Pin_7
#define PIN_COVER_LOW GPIO_ReadInputDataBit(TYPE_COVER_LOW, PORT_COVER_LOW)

#pragma region RUN指示灯
#define TYPE_RUN_LED GPIOD
#define PORT_RUN_LED GPIO_Pin_7
#define run_led_on() GPIO_ResetBits(TYPE_RUN_LED, PORT_RUN_LED)
#define run_led_off() GPIO_SetBits(TYPE_RUN_LED, PORT_RUN_LED)
#define run_led_get() GPIO_ReadInputDataBit(TYPE_RUN_LED, PORT_RUN_LED)
#pragma endregion RUN指示灯

//海格：主电源电压检测
/*
#define TYPE_Voltage_Test		GPIOC
#define PORT_Voltage_Test		GPIO_Pin_0
#define PIN_Voltage_Test		GPIO_ReadInputDataBit(TYPE_Voltage_Test, PORT_Voltage_Test)
*/

#define TYPE_LIFT_LOW GPIOE
#define PORT_LIFT_LOW GPIO_Pin_13
#define PIN_LIFT_LOW GPIO_ReadInputDataBit(TYPE_LIFT_LOW, PORT_LIFT_LOW)

//自定义高电平检测		//海格:左转
#define TYPE_I1_HIGH GPIOD
#define PORT_I1_HIGH GPIO_Pin_14
#define PIN_I1_HIGH GPIO_ReadInputDataBit(TYPE_I1_HIGH, PORT_I1_HIGH)

//海格：右转
#define TYPE_I2_HIGH GPIOD
#define PORT_I2_HIGH GPIO_Pin_15
#define PIN_I2_HIGH GPIO_ReadInputDataBit(TYPE_I2_HIGH, PORT_I2_HIGH)

//海格：远光
#define TYPE_I3_HIGH GPIOC
#define PORT_I3_HIGH GPIO_Pin_13
#define PIN_I3_HIGH GPIO_ReadInputDataBit(TYPE_I3_HIGH, PORT_I3_HIGH)

//输入高电平检测
//海格：近光灯
#define TYPE_I4_HIGH GPIOC
#define PORT_I4_HIGH GPIO_Pin_14
#define PIN_I4_HIGH GPIO_ReadInputDataBit(TYPE_I4_HIGH, PORT_I4_HIGH)

//海格：雾灯 PC15
#define TYPE_I5_HIGH GPIOC
#define PORT_I5_HIGH GPIO_Pin_15
#define PIN_I5_FOGLIGHT GPIO_ReadInputDataBit(TYPE_I5_HIGH, PORT_I5_HIGH)

//海格:倒车
#define TYPE_H6_BACK GPIOC //高电平  倒车
#define PORT_H6_BACK GPIO_Pin_4
#define PIN_H6_BACK GPIO_ReadInputDataBit(TYPE_H6_BACK, PORT_H6_BACK)

//海格:安全带
#define TYPE_I7_HIGH GPIOC
#define PORT_I7_HIGH GPIO_Pin_5
#define PIN_I7_HIGH GPIO_ReadInputDataBit(TYPE_I7_HIGH, PORT_I7_HIGH)

//输入高电平检测,近光
//海格:车门
#define TYPE_I8_HIGH GPIOC
#define PORT_I8_HIGH GPIO_Pin_6
#define PIN_I8_HIGH GPIO_ReadInputDataBit(TYPE_I8_HIGH, PORT_I8_HIGH)

//低电平检测
#define TYPE_LOCK_HIGH GPIOC
#define PORT_LOCK_HIGH GPIO_Pin_9
#define PIN_LOCK_CHECK GPIO_ReadInputDataBit(TYPE_LOCK_HIGH, PORT_LOCK_HIGH)

//5V输出开关
#define TYPE_OUT_5V_VOUT1 GPIOA
#define PORT_OUT_5V_VOUT1 GPIO_Pin_1
#define vout1_5v_disable() GPIO_ResetBits(TYPE_OUT_5V_VOUT1, PORT_OUT_5V_VOUT1)
#define vout1_5v_enable() GPIO_SetBits(TYPE_OUT_5V_VOUT1, PORT_OUT_5V_VOUT1)
#define vout1_5v_get_state() GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)

////---TR9-V400
//海格 电源引脚定义 RK电源
#define TYPE_DVR_PWR GPIOE
#define PORT_DVR_PWR GPIO_Pin_14
//#define DIS_DVR_PWR			   GPIO_SetBits(TYPE_DVR_PWR, PORT_DVR_PWR)
//#define EN_DVR_PWR				GPIO_ResetBits(TYPE_DVR_PWR, PORT_DVR_PWR)
#define DIS_DVR_PWR GPIO_ResetBits(TYPE_DVR_PWR, PORT_DVR_PWR)
#define EN_DVR_PWR GPIO_SetBits(TYPE_DVR_PWR, PORT_DVR_PWR)

#define TYPE_BEEP GPIOB
#define PORT_BEEP GPIO_Pin_1
#define beep_off() GPIO_ResetBits(TYPE_BEEP, PORT_BEEP)
#define beep_on() GPIO_SetBits(TYPE_BEEP, PORT_BEEP)

#if 0
#define TYPE_PRINT_PWR GPIOB
#define PORT_PRINT_PWR GPIO_Pin_2
#define DIS_PRINT_PWR GPIO_ResetBits(TYPE_PRINT_PWR, PORT_PRINT_PWR)
#define EN_PRINT_PWR GPIO_SetBits(TYPE_PRINT_PWR, PORT_PRINT_PWR)
#endif

//打印机
#define TYPE_PRINT_PWR GPIOB
#define PORT_PRINT_PWR GPIO_Pin_2
#define DIS_PRINT_PWR GPIO_ResetBits(TYPE_PRINT_PWR, PORT_PRINT_PWR)
#define EN_PRINT_PWR GPIO_SetBits(TYPE_PRINT_PWR, PORT_PRINT_PWR)

//海格：自定义高1
#define TYPE_LCDVCC_PWR GPIOC
#define PORT_LCDVCC_PWR GPIO_Pin_8
#define DIS_RESET_HI35XX_PWR GPIO_ResetBits(TYPE_LCDVCC_PWR, PORT_LCDVCC_PWR)
#define EN_RESET_HI35XX_PWR GPIO_SetBits(TYPE_LCDVCC_PWR, PORT_LCDVCC_PWR)

#define TYPE_GSM_PWR GPIOD
#define PORT_GSM_PWR GPIO_Pin_1
#define DIS_GSM_PWR GPIO_ResetBits(TYPE_GSM_PWR, PORT_GSM_PWR)
#define EN_GSM_PWR GPIO_SetBits(TYPE_GSM_PWR, PORT_GSM_PWR)

//
#define TYPE_CAM12V_PWR GPIOD
#define PORT_CAM12V_PWR GPIO_Pin_12
#define DIS_CAM12V_PWR GPIO_ResetBits(TYPE_CAM12V_PWR, PORT_CAM12V_PWR)
#define EN_CAM12V_PWR GPIO_SetBits(TYPE_CAM12V_PWR, PORT_CAM12V_PWR)

//GPS 电源控制//六轴//高精度定位模块//...
#define TYPE_GPS_PWR GPIOE
#define PORT_GPS_PWR GPIO_Pin_0
#define pwr_off_gps_lsm_lcd_inOut5v0() GPIO_ResetBits(TYPE_GPS_PWR, PORT_GPS_PWR)
#define pwr_on_gps_lsm_lcd_inOut5v0() GPIO_SetBits(TYPE_GPS_PWR, PORT_GPS_PWR)

//海格：刹车
#define TYPE_BRAKE_PWR GPIOE
#define PORT_BRAKE_PWR GPIO_Pin_1
#define BRAKE GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1)

//#define DIS_VBAT_PWR			GPIO_ResetBits(TYPE_VBAT_PWR, PORT_VBAT_PWR)
//#define EN_VBAT_PWR				GPIO_SetBits(TYPE_VBAT_PWR, PORT_VBAT_PWR)

#pragma region RK复位 + RK上电
//海格：RK电源
#define TYPE_HDD_PWR GPIOE
#define PORT_HDD_PWR GPIO_Pin_12
#if (0)
#define EN_RK_PWR GPIO_SetBits(TYPE_HDD_PWR, PORT_HDD_PWR)
#define DIS_RK_PWR GPIO_ResetBits(TYPE_HDD_PWR, PORT_HDD_PWR)
#elif (1)
#define EN_RK_PWR rk_pwr_en()
#define DIS_RK_PWR rk_pwr_un()
#elif (1)
#define EN_RK_PWR GPIO_SetBits(TYPE_HDD_PWR, PORT_HDD_PWR)
#define DIS_RK_PWR EN_RK_PWR
#endif
#define rk_pwr_get() GPIO_ReadOutputDataBit(TYPE_HDD_PWR, PORT_HDD_PWR)

#define TYPE_RESET_HI35XX GPIOA
#define PORT_RESET_HI35XX GPIO_Pin_8
#if (0)
#define EN_RESET_HI35XX GPIO_SetBits(TYPE_RESET_HI35XX, PORT_RESET_HI35XX)
#define DIS_RESET_HI35XX GPIO_ResetBits(TYPE_RESET_HI35XX, PORT_RESET_HI35XX)
#elif (1)
#define EN_RESET_HI35XX rk_rst_en()
#define DIS_RESET_HI35XX rk_rst_un()
#elif (1)
#define EN_RESET_HI35XX GPIO_ResetBits(TYPE_RESET_HI35XX, PORT_RESET_HI35XX)
#define DIS_RESET_HI35XX GPIO_ResetBits(TYPE_RESET_HI35XX, PORT_RESET_HI35XX)
#endif
#define rk_rst_get() GPIO_ReadOutputDataBit(TYPE_RESET_HI35XX, PORT_RESET_HI35XX)
#pragma endregion RK复位 + RK上电

/*
#define TYPE_GSM_RST			GPIOE
#define PORT_GSM_RST			GPIO_Pin_12
#define EN_RESET_GSM			GPIO_SetBits(TYPE_GSM_RST, PORT_GSM_RST)	
#define DIS_RESET_GSM   		GPIO_ResetBits(TYPE_GSM_RST, PORT_GSM_RST)  
*/

//自定义高输出2
#define TYPE_TURN_OFF GPIOD
#define PORT_TURN_OFF GPIO_Pin_11
#define DIS_TURN_OFF GPIO_ResetBits(TYPE_TURN_OFF, PORT_TURN_OFF)
#define EN_TURN_OFF GPIO_SetBits(TYPE_TURN_OFF, PORT_TURN_OFF)

//ACC电平检测
#define TYPE_ACC GPIOC
#define PORT_ACC GPIO_Pin_1
#define PIN_ACC GPIO_ReadInputDataBit(TYPE_ACC, PORT_ACC)

//MCU配置
extern void
RCC_Configuration(void);
extern void NVIC_Configuration(void);
extern void Timer2_Configuration(void);

//INPUT IO 配置
extern void INPUT1_LOW1_INPUT(void);
extern void INPUT1_LOW2_INPUT(void);

extern void INPUT1_HIGH1_INPUT(void);
extern void INPUT_SOS_INPUT(void);
extern void RCD_INPUT(void);

//OUTPUT IO 配置
extern void DVR_IO_OUTPUT(void);
extern void DVR_PWR_OUTPUT(void);
extern void relay_output_init(void);
extern void GPS_PWR_OUTPUT(void);
extern void GSM_PWR_OUTPUT(void);
extern void GSM_SPK_LM4903(void);
extern void g_delay_us(uchar t);
extern void self_check(void);
//入口函数，外部调用

extern void port_dir_config(GPIO_TypeDef *GPIOx, u16 pinx, bool dir);
extern void spi_host_config(SPI_TypeDef *SPIx, uint bd_speed);
extern uchar spi_read_write_byte(SPI_TypeDef *SPIx, uchar s_dat);

extern void init_drv_cfg(void);

extern void print_start_ctrl(bit_enum en);
extern void g_delay_ms(uint t);
void pwr_gps_lsm_lcd_inOut5v0(unsigned char on, char *who);

void rk_pwr_en(void);
void rk_pwr_un(void);
void rk_rst_en(void);
void rk_rst_un(void);

#endif
