/**
  ******************************************************************************
  * @file    slave_mcu.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  �ӻ������շ�
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#ifndef  __SLAVE_MCU__
#define  __SLAVE_MCU__

#define SLAVE_QUE_SIZE		10
#define SLV_MIN_ADDR		(u32)(SECT_SLAVE_ADDR*0x1000*1ul)
#define SLV_MAM_ADDR		(u32)((SECT_SLAVE_ADDR+SECT_SLAVE_SIZE+1)*0x1000*1ul)

#define SLAVE_SIZE			800
#define SLAVE_ACK_TIMEOUT		1000
/*
#define SPI_SLAVE				SPI1
#define SPI_SLAVE_CLK			RCC_APB2Periph_SPI1
#define SPI_SLAVE_GPIO			GPIOA
#define SPI_SLAVE_SCK			GPIO_Pin_5
#define SPI_SLAVE_MISO			GPIO_Pin_6
#define SPI_SLAVE_MOSI			GPIO_Pin_7

#define SPI_SLAVE_CS			GPIO_Pin_4
#define SPI_SLAVE_CS_GPIO		GPIOA

#define SPI_SLAVE_WP			GPIO_Pin_4
#define SPI_SLAVE_WP_GPIO         GPIOC

#define DIS_SLAVE_MCU_CE		GPIO_SetBits(SPI_SLAVE_CS_GPIO, SPI_SLAVE_CS)
#define EN_SLAVE_MCU_CE		GPIO_ResetBits(SPI_SLAVE_CS_GPIO, SPI_SLAVE_CS)

/* Select SPI FLASH: ChipSelect pin low  */
//#define SPI_SLAVE_CS_LOW()     GPIO_ResetBits(SPI_SLAVE_CS_GPIO, SPI_SLAVE_CS)
/* Deselect SPI FLASH: ChipSelect pin high */
//#define SPI_SLAVE_CS_HIGH()    GPIO_SetBits(SPI_SLAVE_CS_GPIO, SPI_SLAVE_CS)


//-----------------------------------
//-----        �����      --------
#define      serial_comm_rdsr         0x05     //��״̬
#define      serial_comm_read         0x03     //������
#define      serial_comm_write        0x02     //д����
//-----------------------------------
#define LEN_SLAVE_BUF				1200
#define LEN_MAX_SLAVE_APP			1050
#define LEN_MIN_SLAVE_APP			14

typedef enum{
	EGS_SLAVE_IDLE, 
	EGS_SLAVE_SEND
}E_SLAVE_SEND_GET;

typedef struct
{
	bool new_info;
	bool res;
	u8 res1;
	u8 res2;
	u8 res3[20];	
}SLAVE_MCU_RECIVE_STRUCT; //�ӻ����ݽṹ��  
extern SLAVE_MCU_RECIVE_STRUCT slave_r;

typedef struct {
	bool en;	//1:�ö�����Ч 0 : ��Ч
	bool ack; //Ӧ��
	u8 type; //����
	u8 resend; //�ط�
	u16 jif; //����ʱ��
	u16 event;  //�¼�ID
	u16 water;  //�¼���ˮ��
	u16 len; //����
	u32 addr; //��Ϣ�洢���ֽڵ�ַ
}slave_queue;

typedef struct{
	u8   cnt;			//�ն��и���
	u8   res;		
	u8 	index;		//��ǰ���͵ı�������
	bool ack;			//ƽ̨�ظ�
	u32	n_addr;	//��������Ŀ�ʼ��ַ	
	slave_queue que[SLAVE_QUE_SIZE];
}SLAVE_MANAGE_SEND_STRUCT;

extern bool slave_mcu_is_ready(void);
extern bool slave_mcu_write(u8 *s_buff, u16 cnt);
extern s16 slave_mcu_read(u8 *buffer);
extern bool  add_slave_queue(u16 id, u16 wnum, u8 *buf, u16 len);
extern void recive_slave_task(void);
extern void send_slave_task(void);
#endif	//__SLAVE_MCU__

