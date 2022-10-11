#include "include_all.h"

void I2C_Configuration(void);

/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/

void I2C1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* Configure I2C2 pins: PB15 -> SCL and PB14 -> SDA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void SDA_IN(void) //sda线输入 ,PA11
{
    GPIO_InitTypeDef GPIO_InitStructure; //声明GPIO初始化结构体
    // Configure I2C1 pins:SDA
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void SDA_OUT(void) //sda线输出
{
    GPIO_InitTypeDef GPIO_InitStructure; //声明GPIO初始化结构体
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//延时总时间=cnt*16+24us
void delay(u16 cnt) {
    u8 i;
    for (; cnt > 0; cnt--)
        for (i = 0; i < 10; i++)
            ;
}

//产生IIC起始信号
void IIC_Start(void) {
    SDA_OUT(); //sda线输出
    SDA_1;
    SCL_1;
    delay(4);
    SDA_0; //START:when CLK is high,DATA change form high to low
    delay(4);
    SCL_0; //钳住I2C总线，准备发送或接收数据
}

//产生IIC停止信号
void IIC_Stop(void) {
    SDA_OUT(); //sda线输出
    SCL_0;
    SDA_0; //STOP:when CLK is high DATA change form low to high
    delay(4);
    SCL_1;
    SDA_1; //发送I2C总线结束信号
    delay(4);
}

//等待应答信号到来
//返回值：1，接收应答失败
// 0，接收应答成功
u8 IIC_Wait_Ack(void) {
    u8 ucErrTime = 0;
    SDA_IN(); //SDA设置为输入
    SDA_1;
    delay(1);
    SCL_1;
    delay(1);
    while (SDA_read) {
        ucErrTime++;
        if (ucErrTime > 250) {
            IIC_Stop();
            return 1;
        }
    }
    SCL_0; //时钟输出0
    return 0;
}

//产生ACK应答
void IIC_ACK(void) {
    SCL_0;
    SDA_OUT();
    SDA_0;
    delay(2);
    SCL_1;
    delay(2);
    SCL_0;
}

//不产生ACK应答
void IIC_NAck(void) {
    SCL_0;
    SDA_OUT();
    SDA_1;
    delay(2);
    SCL_1;
    delay(2);
    SCL_0;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
u8 IIC_Send_Byte(u8 txd) {
    u8 t;

    SDA_OUT();
    SCL_0; //拉低时钟开始数据传输

    for (t = 0; t < 8; t++) {
        if (txd & 0x80) { //( (txd&0x80)>>7 == 0x01 )
            SDA_1;
        } else {
            SDA_0;
        }

        //IIC_SDA=(txd&0x80)>>7;
        txd <<= 1;

        delay(2); //对TEA5767这三个延时都是必须的
        SCL_1;
        delay(2);
        SCL_0;
        delay(2);
    }

    t = IIC_Wait_Ack();
    return (t);
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 IIC_Read_Byte(u8 ack) {
    u8 i, receive = 0;
    SDA_IN(); //SDA设置为输入
    for (i = 0; i < 8; i++) {
        SCL_0;
        delay(2);
        SCL_1;
        receive <<= 1;
        if (SDA_read) receive++;
        delay(1);
    }

    if (!ack) {
        IIC_NAck(); //发送nACK
    } else {
        IIC_ACK(); //发送ACK
    }

    return receive;
}

rt_err_t IIC_Write(u8 ic, u8 addr, u8 *pData, u8 len, char *who) {
    if (who == NULL) {
        loge("0");
        return RT_ERROR;
    }

    u8 i;
    IIC_Start();
    if (IIC_Send_Byte(ic) == 1) { //RX8025_ADDR_WRITE
        IIC_Stop();
        loge("1, %s", who);
        return RT_ERROR;
    }
    if (IIC_Send_Byte(addr) == 1) {
        IIC_Stop();
        loge("2, %s", who);
        return RT_ERROR;
    }
    for (i = 0; i < len; i++) {
        if (IIC_Send_Byte(pData[i]) == 1) {
            IIC_Stop();
            loge("3, %s", who);
            return RT_ERROR;
        }
    }
    IIC_Stop();
    return RT_EOK;
}

rt_err_t IIC_Read(u8 ic, u8 addr, u8 *pData, u8 len, char *who) {
    if (who == NULL) {
        loge("0");
        return RT_ERROR;
    }

    u8 i;
    IIC_Start();
    if (IIC_Send_Byte(ic) == 1) { //RX8025_ADDR_WRITE
        IIC_Stop();
        loge("1, %s", who);
        return RT_ERROR;
    }
    if (IIC_Send_Byte(addr) == 1) {
        IIC_Stop();
        loge("2, %s", who);
        return RT_ERROR;
    }
    IIC_Start();
    if (IIC_Send_Byte(ic | 0x01) == 1) { //RX8025_ADDR_READ
        IIC_Stop();
        loge("3, %s", who);
        return RT_ERROR;
    }
    for (i = 0; i < len - 1; i++) {
        pData[i] = IIC_Read_Byte(1);
        //IIC_ACK();
    }
    pData[i] = IIC_Read_Byte(0);
    //IIC_NAck();
    IIC_Stop();
    return RT_EOK;
}
