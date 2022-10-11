/**
  ******************************************************************************
  * @file    slave_mcu.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  从机数据收发
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
//-----        命令定义      --------
#define      serial_comm_rdsr         0x05     //读状态
#define      serial_comm_read         0x03     //读数据
#define      serial_comm_write        0x02     //写数据
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
}SLAVE_MCU_RECIVE_STRUCT; //从机数据结构体  
extern SLAVE_MCU_RECIVE_STRUCT slave_r;

typedef struct {
	bool en;	//1:该队列有效 0 : 无效
	bool ack; //应答
	u8 type; //类型
	u8 resend; //重发
	u16 jif; //发送时间
	u16 event;  //事件ID
	u16 water;  //事件流水号
	u16 len; //长度
	u32 addr; //信息存储首字节地址
}slave_queue;

typedef struct{
	u8   cnt;			//空队列个数
	u8   res;		
	u8 	index;		//当前发送的报告索引
	bool ack;			//平台回复
	u32	n_addr;	//下条报告的开始地址	
	slave_queue que[SLAVE_QUE_SIZE];
}SLAVE_MANAGE_SEND_STRUCT;

extern bool slave_mcu_is_ready(void);
extern bool slave_mcu_write(u8 *s_buff, u16 cnt);
extern s16 slave_mcu_read(u8 *buffer);
extern bool  add_slave_queue(u16 id, u16 wnum, u8 *buf, u16 len);
extern void recive_slave_task(void);
extern void send_slave_task(void);
#endif	//__SLAVE_MCU__

