#ifndef __I2C_H__
#define __I2C_H__

#if 0
#define IIC_SCL PBout(6) //SCL
#define IIC_SDA PBout(7) //SDA
#define READ_SDA PBin(7) //ÊäÈëSDA
#endif

#define SCL_1 GPIOB->BSRR = GPIO_Pin_6
#define SCL_0 GPIOB->BRR = GPIO_Pin_6

#define SDA_1 GPIOB->BSRR = GPIO_Pin_7
#define SDA_0 GPIOB->BRR = GPIO_Pin_7

#define SCL_read GPIOB->IDR &GPIO_Pin_6
#define SDA_read GPIOB->IDR &GPIO_Pin_7

extern void I2C1_Init(void);
extern rt_err_t IIC_Read(u8 ic, u8 addr, u8 *pData, u8 len, char *who);
extern rt_err_t IIC_Write(u8 ic, u8 addr, u8 *pData, u8 len, char *who);

#endif
