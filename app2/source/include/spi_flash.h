/**
  ******************************************************************************
  * @file    spi_flash.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  spi flash 数据管理
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
SPI FLASH 分配(sector0~sector1023)//SECT SIZE = 4096 = 0x1000
1.SST25VF032B 4M byte, erase mim size 4096 byte(SECT), max read or write 256 byte
2.集中分配FLASH空间，便于管理和兼容
3.注意操作，FLASH寿命为擦除10万次
======================================================================================*/

//注意IAP功能的地址分配表不能随意修改
/*
IAP_TMP:IAP功能临时变量
*/

#define SECT_MASTER_BASE			0

#define SECT_IAP_TMP_ADDR			(SECT_MASTER_BASE)
#define SECT_IAP_TMP_SIZE			2

/*
IAP flash 空间最大256K
*/
#define SECT_IAP_ADDR				(SECT_IAP_TMP_ADDR+SECT_IAP_TMP_SIZE)		//saddr:  2
#define SECT_IAP_SIZE				64
//注意IAP功能的地址分配表结束

/*
HIS:历史报告存储区
预留日后芯片FLASH 空间扩展到512K或升级第二个芯片时临时共用
*/
#define SECT_HIS_ADDR				(SECT_IAP_ADDR+SECT_IAP_SIZE)				//saddr:  66
#define SECT_HIS_SIZE				100
/*
SWAP:扇区交换
*/
#define SECT_SWAP_ADDR				(SECT_HIS_ADDR+SECT_HIS_SIZE)				//saddr:  166
#define SECT_SWAP_SIZE				3
/*
RES0:保留
*/
#define SECT_RES0_ADDR				(SECT_SWAP_ADDR+SECT_SWAP_SIZE)				//saddr:  169
#define SECT_RES0_SIZE				2
/*
RES0:保留
*/
#define SECT_GPS_INFO_ADDR			(SECT_RES0_ADDR+SECT_RES0_SIZE)				//saddr:  171
#define SECT_GPS_INFO_SIZE			1
/*
SYS:系统运行状态，非频繁操作的临时数据
*/
#define SECT_SYS_ADDR				(SECT_GPS_INFO_ADDR+SECT_GPS_INFO_SIZE)		//saddr:  172
#define SECT_SYS_SIZE				1
/*
SET:客户参数设置
*/
#define SECT_SET_IMPORT_ADDR		(SECT_SYS_ADDR+SECT_SYS_SIZE)				//saddr:  173
#define SECT_SET_IMPORT_SIZE		1

#define SECT_SET_NORMAL_ADDR		(SECT_SET_IMPORT_ADDR+SECT_SET_IMPORT_SIZE)	//saddr:  174
#define SECT_SET_NORMAL_SIZE		1
/*
FACTORY出厂设置或初次设置
*/
#define SECT_FACTORY_ADDR			(SECT_SET_NORMAL_ADDR+SECT_SET_NORMAL_SIZE)	//saddr:  175
#define SECT_FACTORY_SIZE			1
/*
MILE:里程
*/
#define SECT_MILE_ADDR				(SECT_FACTORY_ADDR+SECT_FACTORY_SIZE)		//saddr:  176
#define SECT_MILE_SIZE				1
/*
R1:客户参数设置（行驶记录仪相关）
*/
#define SECT_R1_ADDR				(SECT_MILE_ADDR+SECT_MILE_SIZE)				//saddr:  177
#define SECT_R1_SIZE				1
/*
RES1:保留
*/
#define SECT_RES1_ADDR				(SECT_R1_ADDR+SECT_R1_SIZE)					//saddr:  178
#define SECT_RES1_SIZE				3
/*
H_P:历史数据循环保存指针
*/
#define SECT_H_P_ADDR				(SECT_RES1_ADDR+SECT_RES1_SIZE)				//saddr:  181
#define SECT_H_P_SIZE				1
/*
NET:网络数据暂存区
*/
#define SECT_NET_ADDR				(SECT_H_P_ADDR+SECT_H_P_SIZE)				//saddr:  182
#define SECT_NET_SIZE				10
/*
RPT:实时报告暂存区
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
RES2:保留size 306
*/
#define SECT_RES23_ADDR				(SECT_RETRY_ADDR+SECT_RETRY_SIZE)				//saddr:  205
#define SECT_RES23_SIZE				3


////////第二份SPI核心参数备份区///////306
#define SECT_SET_IMPORT_ADDR_2		(SECT_RETRY_ADDR+SECT_RETRY_SIZE)				//saddr:  173
#define SECT_SET_IMPORT_SIZE_2		1

////////第二份SPI普通参数备份区///////307
#define SECT_SET_NORMAL_ADDR_2		(SECT_SET_IMPORT_ADDR_2+SECT_SET_IMPORT_SIZE_2)	//saddr:  174
#define SECT_SET_NORMAL_SIZE_2		1


/*
R2:预留行驶记录仪一共使用300个sector(208~507)
*/
///360小时数据循环保存指针, 指针起始地址为第208扇区,大小为一个扇区
#define SECT_RCD360_P_ADDR			(SECT_RES23_ADDR+SECT_RES23_SIZE)				//saddr:  208
#define SECT_RCD360_P_SIZE			1
//360小时数据起始地址为第209扇区,大小为63个扇区
#define SECT_RCD360_D_ADDR			(SECT_RCD360_P_ADDR+SECT_RCD360_P_SIZE)			//saddr:  209
#define SECT_RCD360_D_SIZE			63


//连续48小时速度记录
#define SECT_RCD48_P_ADDR			(SECT_RCD360_D_ADDR+SECT_RCD360_D_SIZE)		//saddr:  272
#define SECT_RCD48_P_SIZE			1
#define SECT_RCD48_D_ADDR			(SECT_RCD48_P_ADDR+SECT_RCD48_P_SIZE)		//saddr:  273
#define SECT_RCD48_D_SIZE			100

//事故疑点记录
#define SECT_RCD_ACCIDENT_P_ADDR			(SECT_RCD48_D_ADDR+SECT_RCD48_D_SIZE)	//saddr:  373
#define SECT_RCD_ACCIDENT_P_SIZE			1
#define SECT_RCD_ACCIDENT_D_ADDR			(SECT_RCD_ACCIDENT_P_ADDR+SECT_RCD_ACCIDENT_P_SIZE)	//saddr:  374
#define SECT_RCD_ACCIDENT_D_SIZE			10

//超时驾驶记录
#define SECT_RCD_OT_P_ADDR			(SECT_RCD_ACCIDENT_D_ADDR+SECT_RCD_ACCIDENT_D_SIZE)			//saddr:  384
#define SECT_RCD_OT_P_SIZE			1
#define SECT_RCD_OT_D_ADDR			(SECT_RCD_OT_P_ADDR+SECT_RCD_OT_P_SIZE)						//saddr:  385
#define SECT_RCD_OT_D_SIZE			3

//驾驶人身份记录
#define SECT_RCD_DRIVER_P_ADDR		(SECT_RCD_OT_D_ADDR+SECT_RCD_OT_D_SIZE)			//saddr:  388
#define SECT_RCD_DRIVER_P_SIZE		1
#define SECT_RCD_DRIVER_D_ADDR		(SECT_RCD_DRIVER_P_ADDR+SECT_RCD_DRIVER_P_SIZE)	//saddr:  389
#define SECT_RCD_DRIVER_D_SIZE		3

//外部供电记录
#define SECT_RCD_POWER_P_ADDR		(SECT_RCD_DRIVER_D_ADDR+SECT_RCD_DRIVER_D_SIZE)	//saddr:  392
#define SECT_RCD_POWER_P_SIZE		1
#define SECT_RCD_POWER_D_ADDR		(SECT_RCD_POWER_P_ADDR+SECT_RCD_POWER_P_SIZE)	//saddr:  393
#define SECT_RCD_POWER_D_SIZE		3

//参数更改记录
#define SECT_RCD_PARA_P_ADDR		(SECT_RCD_POWER_D_ADDR+SECT_RCD_POWER_D_SIZE)	//saddr:  396
#define SECT_RCD_PARA_P_SIZE		1
#define SECT_RCD_PARA_D_ADDR		(SECT_RCD_PARA_P_ADDR+SECT_RCD_PARA_P_SIZE)	//saddr:  397
#define SECT_RCD_PARA_D_SIZE		3

//速度状态日志记录
#define SECT_RCD_SPEED_P_ADDR		(SECT_RCD_PARA_D_ADDR+SECT_RCD_PARA_D_SIZE)	//saddr:  400
#define SECT_RCD_SPEED_P_SIZE		1
#define SECT_RCD_SPEED_D_ADDR		(SECT_RCD_SPEED_P_ADDR+SECT_RCD_SPEED_P_SIZE)	//saddr:  401
#define SECT_RCD_SPEED_D_SIZE		40  //3   2022-04-16 modify by hj

/////////////////////////////
#define SECT_RCD_PRINT_ADDR			(SECT_RCD_SPEED_D_ADDR+SECT_RCD_SPEED_D_SIZE)	//saddr:  404
#define SECT_RCD_PRINT_SIZE			4
#define SECT_AREA_ADDR				(SECT_RCD_PRINT_ADDR+SECT_RCD_PRINT_SIZE)	//saddr:  408
#define SECT_AREA_SIZE				60 //100  2022-04-16 modify by hj

//508扇区到 800 扇区未使用
//分配给从机使用的空间为(800-27-509)*4K = 1056K 空间


//-----------------------------------------//
//短信区域       0x100000       预留600K空间  (0x100000---0x1aefff)
#define  flash_sms_center_addr    	(508*0x1000*1ul)//0x100000    //中心(调度)(文本)短信存储绝对地址   预留总长度为100K  (0x100000-0x118fff)
#define  flash_sms_phone_addr     	(533*0x1000*1ul)    //手机短信存储绝对地址               预留总长度为100K  (0x119000-0x131fff)
#define  flash_sms_serve_addr     	(558*0x1000*1ul)    //服务信息存储绝对地址               预留总长度为100K  (0x132000-0x14afff)
#define  flash_sms_fix_addr       	(583*0x1000*1ul)    //固定(事件)短信存储绝对地址         预留总长度为100K  (0x14b000-0x163fff)
#define  flash_sms_vod_addr       	(608*0x1000*1ul)    //点播信息存储绝对地址               预留总长度为100K  (0x164000-0x17cfff)
#define  flash_sms_ask_addr       	(633*0x1000*1ul)    //提问信息存储绝对地址               预留总长度为100K  (0x17d000-0x195fff)

//电话本区域     0x1af000       预留120K 空间  (0x1af000---0x1eefff)
#define  flash_phone_book_addr    	(658*0x1000*1ul)    //0x1af000    //电话存储绝对地址     预留总长度为256K  (0x1af000---0x1eefff)

//通话记录区域   0x1ef000    预留150k空间 (0x1ef000---0x2139f0)
#define  flash_call_out_record_addr   		(688*0x1000*1ul)     //呼出记录存储绝对地址        预留总长度为50K  (0x1ef000---1fb34f)
#define  flash_received_call_record_addr   	(698*0x1000*1ul)     //已接电话存储绝对地址    预留总长度为50K  (0x1fb350---0x20769f)
#define  flash_missed_call_record_addr   	(708*0x1000*1ul)     //未接电话存储绝对地址      预留总长度为50K  (0x2076a0---0x2139f0)


//主机参数区域    0x3ff000      预留4K空间    (0x3ff000---0x3fffff)
#define  flash_parameter_addr        (718*0x1000*1ul)       //flash存储的参数绝对基地址(0x3ff000--0x3fffff) 共4K 最后一页

//唤醒参数设置
#define  flash_Awaken_param_addr     (730*0x1000*1ul)		



#define SECT_MAX_LIMIT				(1023)
#define ADD_MAX_LIMIT				(SECT_MAX_LIMIT*0x1000*1ul)
#define CHI_ENG_LIB_SIZE          (110) //占用110个扇区地址

//-----------------------------------------//
//-----------------------------------------//
//-----------------------------------------//
//字库相对地址
//液晶国标码          范围    0x000000 --- 0x03fe5f (261728)  (255.59375K)
//液晶ASCII码         范围    0x03fe60 --- 0x04044f (1520)    (1.484375K)
//国标码UNICODE码     范围    0x040450 --- 0x0558df (87184)   (85.140625K)
//UNICODE码国标码     范围    0x0558e0 --- 0x06ad6f (87184)   (85.140625K)

//预留最后110个扇区的空间存储字库数据，即4M存储空间地址范围(0x392000---0x3fffff)，即第914扇区到1023扇区

#define    font_base_start_addr      (0x392000)     //字库文件的绝对起始地址，也即预留信息的起始地址
#define    font_reserve_max_lgth     (2*0x1000)     //字库存储区域预留存储空间大小，两个扇区即8K      

#define    font_gb2312_start_addr  	(font_base_start_addr+font_reserve_max_lgth)  //国标码字库绝对基地址
#define    font_ascii_start_addr   	(font_gb2312_start_addr+261728)             //ASCII字库绝对基地址
#define    font_gb2uni_start_addr  	(font_ascii_start_addr+1520)                 //国标码与UNICODE码对照表
#define    font_uni2gb_start_addr  	(font_gb2uni_start_addr+87184)               //UNICODE码与国标码对照表

#define    font_gb_uni_table_lgth  	(87184)        //国标码与UNICODE码对照表长度


#define    software_iap_addr  		(SECT_IAP_ADDR * 0x1000)

#if 1
//从机参数区域    0x3ff000      预留4K空间    (0x3ff000---0x3fffff)
//#define  flash_parameter_addr        0x3ff000       //flash存储的参数绝对基地址(0x3ff000--0x3fffff) 共4K 最后一页
#define  flash_parameter_max_lgth   ((u16)(2048))  //参数长度

#define  flash_password_addr         100       //密码数据相对地址(总长度共200字节)
#define  flash_set_para_addr         300       //设置参数相对地址(总长度共700字节)

#define  flash_recorder_para_addr   	1100     //记录仪相对地址(总长度共900字节)

#define  flash_flag_1_addr           0         //擦除判断标志相对地址1
#define  flash_flag_2_addr           1023     //擦除判断标志相对地址2
#define  flash_flag_3_addr           2047     //擦除判断标志相对地址3

#define  spi_25flash_flag_1        0x15      //初始化标志1
#define  spi_25flash_flag_2        0x5a      //初始化标志2
#define  spi_25flash_flag_3        0x3a      //初始化标志3
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

extern void SPI_FLASH_Init(void);     //SPI_FLASH初始化
extern void flash25_program_auto_save(u32 startaddr, u8 * buffer, u32 length);
// extern void food_dog(void);

#endif	/* __SPI_FALSH_H */


