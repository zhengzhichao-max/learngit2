#ifndef __sd_driver_h__
#define __sd_driver_h__




#include "ckp_mcu_file.h"
#include "ffconf.h"
#include "diskio.h"










//*****************************************************************************
//*****************************************************************************
//-----------           需要外部函数及变量支持                -----------------
//-----------------------------------------------------------------------------

typedef  struct
{
	uchar  *sd_insert_state;  //SD卡插入状态    0未插入  1SD卡插入
	//注意fat_user提供的所有调用函数都是基于该状态为1的条件下，即SD卡必须是插入的状态//

	ulong  *sd_sys_time;    //系统时钟
	uchar  *sd_date_src;    //日期值，BCD格式，三个字节长度  年月日  若不需要，则初始化成NULL即可
	uchar  *sd_time_src;    //时间值，BCD格式，三个字节长度  时分秒  若不需要，则初始化成NULL即可
    
	void (*sd_cs_set)(bit_enum s);  //片选使能控制  TRUE使能  FALSE禁止
	void (*sd_feed_wdt)(void);     //看门狗喂狗操作

	void (*sd_spi_config)(bit_enum fast_mode);  //SPI接口配置  参数FALSE慢速100K左右，TRUE快速20M以内
	uchar (*sd_wr_rd)(uchar dat);    //SPI接口交互一个字节的数据
}
sd_driver_need_struct;















//初始化








#endif
//*****************************************************************************
//*****************************************************************************




