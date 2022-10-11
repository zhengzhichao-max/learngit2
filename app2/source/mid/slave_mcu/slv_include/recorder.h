#ifndef  __recorder_h__
#define  __recorder_h__
///#include "ckp_mcu_file.h"

#define   at24_i2c_clk               ((u32)50000)    //定义时钟频率  50KHz
#define   at24_i2c_own_addr         ((u8)0xf1)      //自身地址
#define   at24_i2c_slave_addr       ((u8)0xa0)      //从芯片地址

#define   eeprom_start_addr    ((u8)0x00)  //芯片内部的起始地址
#define   eeprom_data_lgth     ((u8)128)   //数据总长度


#define  print_speed_set      ((uint)180)    //打印速度设置
#define  print_auto_buff_lgth_max   ((uint)900)    //自动打印最大缓冲器长度



typedef struct
{
	bit_enum  req_flag;    //请求状态	TRUE为请求	 FALSE为未请求

	uint   lgth;		   //待打印长度
	uint   cnt; 		   //已经打印的长度
	uchar  buff[print_auto_buff_lgth_max];	//缓冲器
}
print_auto_data_struct;



//*****************************************************************************
//*****************************************************************************
//-----------                 供给外部函数调用                -----------------
//-----------------------------------------------------------------------------

extern void recorder_init(void);

extern void load_print_data(u8* src, u16 lgth);
extern void print_auto_process(void);

#endif




