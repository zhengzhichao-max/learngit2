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

void SDA_IN(void) //sda������ ,PA11
{
    GPIO_InitTypeDef GPIO_InitStructure; //����GPIO��ʼ���ṹ��
    // Configure I2C1 pins:SDA
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void SDA_OUT(void) //sda�����
{
    GPIO_InitTypeDef GPIO_InitStructure; //����GPIO��ʼ���ṹ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//��ʱ��ʱ��=cnt*16+24us
void delay(u16 cnt) {
    u8 i;
    for (; cnt > 0; cnt--)
        for (i = 0; i < 10; i++)
            ;
}

//����IIC��ʼ�ź�
void IIC_Start(void) {
    SDA_OUT(); //sda�����
    SDA_1;
    SCL_1;
    delay(4);
    SDA_0; //START:when CLK is high,DATA change form high to low
    delay(4);
    SCL_0; //ǯסI2C���ߣ�׼�����ͻ��������
}

//����IICֹͣ�ź�
void IIC_Stop(void) {
    SDA_OUT(); //sda�����
    SCL_0;
    SDA_0; //STOP:when CLK is high DATA change form low to high
    delay(4);
    SCL_1;
    SDA_1; //����I2C���߽����ź�
    delay(4);
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
// 0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void) {
    u8 ucErrTime = 0;
    SDA_IN(); //SDA����Ϊ����
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
    SCL_0; //ʱ�����0
    return 0;
}

//����ACKӦ��
void IIC_ACK(void) {
    SCL_0;
    SDA_OUT();
    SDA_0;
    delay(2);
    SCL_1;
    delay(2);
    SCL_0;
}

//������ACKӦ��
void IIC_NAck(void) {
    SCL_0;
    SDA_OUT();
    SDA_1;
    delay(2);
    SCL_1;
    delay(2);
    SCL_0;
}

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
u8 IIC_Send_Byte(u8 txd) {
    u8 t;

    SDA_OUT();
    SCL_0; //����ʱ�ӿ�ʼ���ݴ���

    for (t = 0; t < 8; t++) {
        if (txd & 0x80) { //( (txd&0x80)>>7 == 0x01 )
            SDA_1;
        } else {
            SDA_0;
        }

        //IIC_SDA=(txd&0x80)>>7;
        txd <<= 1;

        delay(2); //��TEA5767��������ʱ���Ǳ����
        SCL_1;
        delay(2);
        SCL_0;
        delay(2);
    }

    t = IIC_Wait_Ack();
    return (t);
}

//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
u8 IIC_Read_Byte(u8 ack) {
    u8 i, receive = 0;
    SDA_IN(); //SDA����Ϊ����
    for (i = 0; i < 8; i++) {
        SCL_0;
        delay(2);
        SCL_1;
        receive <<= 1;
        if (SDA_read) receive++;
        delay(1);
    }

    if (!ack) {
        IIC_NAck(); //����nACK
    } else {
        IIC_ACK(); //����ACK
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
