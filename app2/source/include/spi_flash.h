/**
  ******************************************************************************
  * @file    spi_flash.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  spi flash ���ݹ���
  ******************************************************************************
  * @attention

  ******************************************************************************
**/
#ifndef	__SPI_FALSH_H
#define	__SPI_FALSH_H

#ifdef  DEBUG
#define assert(expr) ((expr) ? (void)0 : assert_failed((u8 *)__FILE__, __LINE__))
void assert_failed(u8 * file, u32 line);
#else
#define assert(expr) ((void)0)
#endif /* DEBUG */

/*======================================================================================
SPI FLASH ����(sector0~sector1023)//SECT SIZE = 4096 = 0x1000
1.SST25VF032B 4M byte, erase mim size 4096 byte(SECT), max read or write 256 byte
2.���з���FLASH�ռ䣬���ڹ���ͼ���
3.ע�������FLASH����Ϊ����10���
======================================================================================*/

//ע��IAP���ܵĵ�ַ������������޸�
/*
IAP_TMP:IAP������ʱ����
*/

#define SECT_MASTER_BASE			0

#define SECT_IAP_TMP_ADDR			(SECT_MASTER_BASE)
#define SECT_IAP_TMP_SIZE			2

/*
IAP flash �ռ����256K
*/
#define SECT_IAP_ADDR				(SECT_IAP_TMP_ADDR+SECT_IAP_TMP_SIZE)		//saddr:  2
#define SECT_IAP_SIZE				64
//ע��IAP���ܵĵ�ַ��������

/*
HIS:��ʷ����洢��
Ԥ���պ�оƬFLASH �ռ���չ��512K�������ڶ���оƬʱ��ʱ����
*/
#define SECT_HIS_ADDR				(SECT_IAP_ADDR+SECT_IAP_SIZE)				//saddr:  66
#define SECT_HIS_SIZE				100
/*
SWAP:��������
*/
#define SECT_SWAP_ADDR				(SECT_HIS_ADDR+SECT_HIS_SIZE)				//saddr:  166
#define SECT_SWAP_SIZE				3
/*
RES0:����
*/
#define SECT_RES0_ADDR				(SECT_SWAP_ADDR+SECT_SWAP_SIZE)				//saddr:  169
#define SECT_RES0_SIZE				2
/*
RES0:����
*/
#define SECT_GPS_INFO_ADDR			(SECT_RES0_ADDR+SECT_RES0_SIZE)				//saddr:  171
#define SECT_GPS_INFO_SIZE			1
/*
SYS:ϵͳ����״̬����Ƶ����������ʱ����
*/
#define SECT_SYS_ADDR				(SECT_GPS_INFO_ADDR+SECT_GPS_INFO_SIZE)		//saddr:  172
#define SECT_SYS_SIZE				1
/*
SET:�ͻ���������
*/
#define SECT_SET_IMPORT_ADDR		(SECT_SYS_ADDR+SECT_SYS_SIZE)				//saddr:  173
#define SECT_SET_IMPORT_SIZE		1

#define SECT_SET_NORMAL_ADDR		(SECT_SET_IMPORT_ADDR+SECT_SET_IMPORT_SIZE)	//saddr:  174
#define SECT_SET_NORMAL_SIZE		1
/*
FACTORY�������û��������
*/
#define SECT_FACTORY_ADDR			(SECT_SET_NORMAL_ADDR+SECT_SET_NORMAL_SIZE)	//saddr:  175
#define SECT_FACTORY_SIZE			1
/*
MILE:���
*/
#define SECT_MILE_ADDR				(SECT_FACTORY_ADDR+SECT_FACTORY_SIZE)		//saddr:  176
#define SECT_MILE_SIZE				1
/*
R1:�ͻ��������ã���ʻ��¼����أ�
*/
#define SECT_R1_ADDR				(SECT_MILE_ADDR+SECT_MILE_SIZE)				//saddr:  177
#define SECT_R1_SIZE				1
/*
RES1:����
*/
#define SECT_RES1_ADDR				(SECT_R1_ADDR+SECT_R1_SIZE)					//saddr:  178
#define SECT_RES1_SIZE				3
/*
H_P:��ʷ����ѭ������ָ��
*/
#define SECT_H_P_ADDR				(SECT_RES1_ADDR+SECT_RES1_SIZE)				//saddr:  181
#define SECT_H_P_SIZE				1
/*
NET:���������ݴ���
*/
#define SECT_NET_ADDR				(SECT_H_P_ADDR+SECT_H_P_SIZE)				//saddr:  182
#define SECT_NET_SIZE				10
/*
RPT:ʵʱ�����ݴ���
*/
#define SECT_RPT_ADDR				(SECT_NET_ADDR+SECT_NET_SIZE)				//saddr:  192
#define SECT_RPT_SIZE				10

#define SECT_TTS_ADDR				(SECT_RPT_ADDR+SECT_RPT_SIZE)				//saddr:  202
#define SECT_TTS_SIZE				1

#define SECT_TAPE_ADDR				(SECT_TTS_ADDR+SECT_TTS_SIZE)				//saddr:  203
#define SECT_TAPE_SIZE				1

#define SECT_RETRY_ADDR				(SECT_TAPE_ADDR+SECT_TAPE_SIZE)					//saddr:  204
#define SECT_RETRY_SIZE				1

/*
RES2:����size 306
*/
#define SECT_RES23_ADDR				(SECT_RETRY_ADDR+SECT_RETRY_SIZE)				//saddr:  205
#define SECT_RES23_SIZE				3


////////�ڶ���SPI���Ĳ���������///////306
#define SECT_SET_IMPORT_ADDR_2		(SECT_RETRY_ADDR+SECT_RETRY_SIZE)				//saddr:  173
#define SECT_SET_IMPORT_SIZE_2		1

////////�ڶ���SPI��ͨ����������///////307
#define SECT_SET_NORMAL_ADDR_2		(SECT_SET_IMPORT_ADDR_2+SECT_SET_IMPORT_SIZE_2)	//saddr:  174
#define SECT_SET_NORMAL_SIZE_2		1


/*
R2:Ԥ����ʻ��¼��һ��ʹ��300��sector(208~507)
*/
///360Сʱ����ѭ������ָ��, ָ����ʼ��ַΪ��208����,��СΪһ������
#define SECT_RCD360_P_ADDR			(SECT_RES23_ADDR+SECT_RES23_SIZE)				//saddr:  208
#define SECT_RCD360_P_SIZE			1
//360Сʱ������ʼ��ַΪ��209����,��СΪ63������
#define SECT_RCD360_D_ADDR			(SECT_RCD360_P_ADDR+SECT_RCD360_P_SIZE)			//saddr:  209
#define SECT_RCD360_D_SIZE			63


//����48Сʱ�ٶȼ�¼
#define SECT_RCD48_P_ADDR			(SECT_RCD360_D_ADDR+SECT_RCD360_D_SIZE)		//saddr:  272
#define SECT_RCD48_P_SIZE			1
#define SECT_RCD48_D_ADDR			(SECT_RCD48_P_ADDR+SECT_RCD48_P_SIZE)		//saddr:  273
#define SECT_RCD48_D_SIZE			100

//�¹��ɵ��¼
#define SECT_RCD_ACCIDENT_P_ADDR			(SECT_RCD48_D_ADDR+SECT_RCD48_D_SIZE)	//saddr:  373
#define SECT_RCD_ACCIDENT_P_SIZE			1
#define SECT_RCD_ACCIDENT_D_ADDR			(SECT_RCD_ACCIDENT_P_ADDR+SECT_RCD_ACCIDENT_P_SIZE)	//saddr:  374
#define SECT_RCD_ACCIDENT_D_SIZE			10

//��ʱ��ʻ��¼
#define SECT_RCD_OT_P_ADDR			(SECT_RCD_ACCIDENT_D_ADDR+SECT_RCD_ACCIDENT_D_SIZE)			//saddr:  384
#define SECT_RCD_OT_P_SIZE			1
#define SECT_RCD_OT_D_ADDR			(SECT_RCD_OT_P_ADDR+SECT_RCD_OT_P_SIZE)						//saddr:  385
#define SECT_RCD_OT_D_SIZE			3

//��ʻ����ݼ�¼
#define SECT_RCD_DRIVER_P_ADDR		(SECT_RCD_OT_D_ADDR+SECT_RCD_OT_D_SIZE)			//saddr:  388
#define SECT_RCD_DRIVER_P_SIZE		1
#define SECT_RCD_DRIVER_D_ADDR		(SECT_RCD_DRIVER_P_ADDR+SECT_RCD_DRIVER_P_SIZE)	//saddr:  389
#define SECT_RCD_DRIVER_D_SIZE		3

//�ⲿ�����¼
#define SECT_RCD_POWER_P_ADDR		(SECT_RCD_DRIVER_D_ADDR+SECT_RCD_DRIVER_D_SIZE)	//saddr:  392
#define SECT_RCD_POWER_P_SIZE		1
#define SECT_RCD_POWER_D_ADDR		(SECT_RCD_POWER_P_ADDR+SECT_RCD_POWER_P_SIZE)	//saddr:  393
#define SECT_RCD_POWER_D_SIZE		3

//�������ļ�¼
#define SECT_RCD_PARA_P_ADDR		(SECT_RCD_POWER_D_ADDR+SECT_RCD_POWER_D_SIZE)	//saddr:  396
#define SECT_RCD_PARA_P_SIZE		1
#define SECT_RCD_PARA_D_ADDR		(SECT_RCD_PARA_P_ADDR+SECT_RCD_PARA_P_SIZE)	//saddr:  397
#define SECT_RCD_PARA_D_SIZE		3

//�ٶ�״̬��־��¼
#define SECT_RCD_SPEED_P_ADDR		(SECT_RCD_PARA_D_ADDR+SECT_RCD_PARA_D_SIZE)	//saddr:  400
#define SECT_RCD_SPEED_P_SIZE		1
#define SECT_RCD_SPEED_D_ADDR		(SECT_RCD_SPEED_P_ADDR+SECT_RCD_SPEED_P_SIZE)	//saddr:  401
#define SECT_RCD_SPEED_D_SIZE		40  //3   2022-04-16 modify by hj

/////////////////////////////
#define SECT_RCD_PRINT_ADDR			(SECT_RCD_SPEED_D_ADDR+SECT_RCD_SPEED_D_SIZE)	//saddr:  404
#define SECT_RCD_PRINT_SIZE			4
#define SECT_AREA_ADDR				(SECT_RCD_PRINT_ADDR+SECT_RCD_PRINT_SIZE)	//saddr:  408
#define SECT_AREA_SIZE				60 //100  2022-04-16 modify by hj

//508������ 800 ����δʹ��
//������ӻ�ʹ�õĿռ�Ϊ(800-27-509)*4K = 1056K �ռ�


//-----------------------------------------//
//��������       0x100000       Ԥ��600K�ռ�  (0x100000---0x1aefff)
#define  flash_sms_center_addr    	(508*0x1000*1ul)//0x100000    //����(����)(�ı�)���Ŵ洢���Ե�ַ   Ԥ���ܳ���Ϊ100K  (0x100000-0x118fff)
#define  flash_sms_phone_addr     	(533*0x1000*1ul)    //�ֻ����Ŵ洢���Ե�ַ               Ԥ���ܳ���Ϊ100K  (0x119000-0x131fff)
#define  flash_sms_serve_addr     	(558*0x1000*1ul)    //������Ϣ�洢���Ե�ַ               Ԥ���ܳ���Ϊ100K  (0x132000-0x14afff)
#define  flash_sms_fix_addr       	(583*0x1000*1ul)    //�̶�(�¼�)���Ŵ洢���Ե�ַ         Ԥ���ܳ���Ϊ100K  (0x14b000-0x163fff)
#define  flash_sms_vod_addr       	(608*0x1000*1ul)    //�㲥��Ϣ�洢���Ե�ַ               Ԥ���ܳ���Ϊ100K  (0x164000-0x17cfff)
#define  flash_sms_ask_addr       	(633*0x1000*1ul)    //������Ϣ�洢���Ե�ַ               Ԥ���ܳ���Ϊ100K  (0x17d000-0x195fff)

//�绰������     0x1af000       Ԥ��120K �ռ�  (0x1af000---0x1eefff)
#define  flash_phone_book_addr    	(658*0x1000*1ul)    //0x1af000    //�绰�洢���Ե�ַ     Ԥ���ܳ���Ϊ256K  (0x1af000---0x1eefff)

//ͨ����¼����   0x1ef000    Ԥ��150k�ռ� (0x1ef000---0x2139f0)
#define  flash_call_out_record_addr   		(688*0x1000*1ul)     //������¼�洢���Ե�ַ        Ԥ���ܳ���Ϊ50K  (0x1ef000---1fb34f)
#define  flash_received_call_record_addr   	(698*0x1000*1ul)     //�ѽӵ绰�洢���Ե�ַ    Ԥ���ܳ���Ϊ50K  (0x1fb350---0x20769f)
#define  flash_missed_call_record_addr   	(708*0x1000*1ul)     //δ�ӵ绰�洢���Ե�ַ      Ԥ���ܳ���Ϊ50K  (0x2076a0---0x2139f0)


//������������    0x3ff000      Ԥ��4K�ռ�    (0x3ff000---0x3fffff)
#define  flash_parameter_addr        (718*0x1000*1ul)       //flash�洢�Ĳ������Ի���ַ(0x3ff000--0x3fffff) ��4K ���һҳ

//���Ѳ�������
#define  flash_Awaken_param_addr     (730*0x1000*1ul)		



#define SECT_MAX_LIMIT				(1023)
#define ADD_MAX_LIMIT				(SECT_MAX_LIMIT*0x1000*1ul)
#define CHI_ENG_LIB_SIZE          (110) //ռ��110��������ַ

//-----------------------------------------//
//-----------------------------------------//
//-----------------------------------------//
//�ֿ���Ե�ַ
//Һ��������          ��Χ    0x000000 --- 0x03fe5f (261728)  (255.59375K)
//Һ��ASCII��         ��Χ    0x03fe60 --- 0x04044f (1520)    (1.484375K)
//������UNICODE��     ��Χ    0x040450 --- 0x0558df (87184)   (85.140625K)
//UNICODE�������     ��Χ    0x0558e0 --- 0x06ad6f (87184)   (85.140625K)

//Ԥ�����110�������Ŀռ�洢�ֿ����ݣ���4M�洢�ռ��ַ��Χ(0x392000---0x3fffff)������914������1023����

#define    font_base_start_addr      (0x392000)     //�ֿ��ļ��ľ�����ʼ��ַ��Ҳ��Ԥ����Ϣ����ʼ��ַ
#define    font_reserve_max_lgth     (2*0x1000)     //�ֿ�洢����Ԥ���洢�ռ��С������������8K      

#define    font_gb2312_start_addr  	(font_base_start_addr+font_reserve_max_lgth)  //�������ֿ���Ի���ַ
#define    font_ascii_start_addr   	(font_gb2312_start_addr+261728)             //ASCII�ֿ���Ի���ַ
#define    font_gb2uni_start_addr  	(font_ascii_start_addr+1520)                 //��������UNICODE����ձ�
#define    font_uni2gb_start_addr  	(font_gb2uni_start_addr+87184)               //UNICODE�����������ձ�

#define    font_gb_uni_table_lgth  	(87184)        //��������UNICODE����ձ���


#define    software_iap_addr  		(SECT_IAP_ADDR * 0x1000)

#if 1
//�ӻ���������    0x3ff000      Ԥ��4K�ռ�    (0x3ff000---0x3fffff)
//#define  flash_parameter_addr        0x3ff000       //flash�洢�Ĳ������Ի���ַ(0x3ff000--0x3fffff) ��4K ���һҳ
#define  flash_parameter_max_lgth   ((u16)(2048))  //��������

#define  flash_password_addr         100       //����������Ե�ַ(�ܳ��ȹ�200�ֽ�)
#define  flash_set_para_addr         300       //���ò�����Ե�ַ(�ܳ��ȹ�700�ֽ�)

#define  flash_recorder_para_addr   	1100     //��¼����Ե�ַ(�ܳ��ȹ�900�ֽ�)

#define  flash_flag_1_addr           0         //�����жϱ�־��Ե�ַ1
#define  flash_flag_2_addr           1023     //�����жϱ�־��Ե�ַ2
#define  flash_flag_3_addr           2047     //�����жϱ�־��Ե�ַ3

#define  spi_25flash_flag_1        0x15      //��ʼ����־1
#define  spi_25flash_flag_2        0x5a      //��ʼ����־2
#define  spi_25flash_flag_3        0x3a      //��ʼ����־3
#endif
/******************************************************************************************/

#define SPI_FLASH				SPI1
#define SPI_FLASH_CLK			RCC_APB2Periph_SPI1

#define SPI_FLASH_GPIO			GPIOB
#define SPI_FLASH_SCK			GPIO_Pin_3
#define SPI_FLASH_MISO			GPIO_Pin_4
#define SPI_FLASH_MOSI			GPIO_Pin_5

#define SPI_FLASH_CS			GPIO_Pin_15
#define SPI_FLASH_CS_GPIO		GPIOA

//#define SPI_FLASH_WP			GPIO_Pin_8
//#define SPI_FLASH_WP_GPIO       GPIOE
//#define FLASH_TOTAL_ADDR		0x04000000-0x00002000

/* Define to prevent recursive inclusion -------------------------------------*/
//#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

/* Select SPI FLASH: ChipSelect pin low  */
#define SPI_FLASH_CS_LOW()     GPIO_ResetBits(SPI_FLASH_CS_GPIO, SPI_FLASH_CS)
/* Deselect SPI FLASH: ChipSelect pin high */
#define SPI_FLASH_CS_HIGH()    GPIO_SetBits(SPI_FLASH_CS_GPIO, SPI_FLASH_CS)

/*----- High layer function -----*/
void spi_flash_erase(u32 SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(u8 * pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void spi_flash_write(u8 * pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void spi_flash_read(u8 * pBuffer, u32 ReadAddr, u16 NumByteToRead);




/*----- Low layer function -----*/
u8 SPI_FLASH_SendByte(u8 byte);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

#define SPI_FLAG_RXNE                      ((u16)0x0001)
#define SPI_FLAG_TXE                       ((u16)0x0002)
#define SPI_FLAG_OVR                       ((u16)0x0040)
#define SPI_FLAG_BSY                       ((u16)0x0080)

/* Private typedef -----------------------------------------------------------*/
#define SPI_FLASH_PageSize 256
//#define SPI_FLASH_PageSize 4096

#define CMD_WRITE      0x02
#define CMD_WRSR       0x01
#define CMD_WREN       0x06

#define CMD_READ       0x03
#define CMD_RDSR       0x05
#define CMD_RDID       0x9F
#define CMD_EBMA       0x20
#define CMD_EB32       0x52
#define CMD_SE         0xd8
#define CMD_BE         0xC7

#define WIP_Flag   0x01

#define Dummy_Byte 0xA5

extern void SPI_FLASH_Init(void);     //SPI_FLASH��ʼ��
extern void flash25_program_auto_save(u32 startaddr, u8 * buffer, u32 length);
// extern void food_dog(void);

#endif	/* __SPI_FALSH_H */


