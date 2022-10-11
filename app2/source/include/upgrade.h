/**
  ******************************************************************************
  * @file    upgrade.h
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  远程升级管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef __UPGRADE_H
#define __UPGRADE_H
///#include "define.h"

#include "ckp_mcu_file.h"

#define font_lib_max_lgth ((ulong)0x100000)     //字库文件最大长度
#define font_lib_once_max_lgth ((uint)2080)     //字库文件升级一次读取的最大长度
#define update_tf_once_max_lgth ((uint)2048)    //TF卡从机升级一次读取的最大长度
#define parameter_cfg_once_max_lgth ((uint)512) //参数自动配置一次读取数据的最大长度

#define PAGE_SIZE (1024 * 2)

#define BootStartAddress ((u32)0x08000000)   //BOOT开始地址
#define ApplicationAddress ((u32)0x08004000) //应用程序开始地址

#if (TR_USING_IAP)
#define MCU_NORMAL_CFG_ADDR ((u32)0x0803D000) //普通参数存放地址
#define MCU_IMPORT_CFG_ADDR ((u32)0x0803D800)
#define BLParaAddress ((u32)0x0803E000) //升级标志0x0800F000
#else
/*
512K
STM32F105VC(256K)
IROM1:
0x8000000
0x7F400
*/
#define MCU_NORMAL_CFG_ADDR ((u32)0x08080000 - 1024 * 6) //普通参数存放地址
#define MCU_IMPORT_CFG_ADDR ((u32)0x08080000 - 1024 * 4)
#define BLParaAddress ((u32)0x08080000 - 1024 * 2) //升级标志0x0800F000
#endif

#define APP_MAX_DATA_LEN ((BLParaAddress - ApplicationAddress) * 1ul)
/*
IAP flash 空间最大256K
*/
//spi_flash.h//那边有定义，会重复定义
//#define SECT_IAP_ADDR (SECT_IAP_TMP_ADDR + SECT_IAP_TMP_SIZE)
//#define SECT_IAP_SIZE 64

#define ADDR_IAP_TMP (SECT_IAP_TMP_ADDR * 0x1000 * 1ul)
#define ADDR_IAP_MIN (SECT_IAP_ADDR * 0x1000 * 1ul)
#define ADDR_IAP_MAX ((SECT_IAP_ADDR + SECT_IAP_SIZE + 1) * 0x1000 * 1ul)

///#define FLASH_TOTAL_ADDR			0x04000000-0x00002000

#define UPGRADE_AUTO 0
#define UPGRADE_BYPASS 1
#define UPGRADE_IGNORE_ALL 2

#define _upg_addr(ptr) ((ADDR_IAP_MIN + ptr) * 1ul)

typedef struct {
    u16 crc;
    u16 flag;
    u32 ApplicationFlag;      //标志位
    u32 ProgramDataLen;       //无线等待更新的程序文件长度
    u32 ProgramAddCheckValue; //无线等待更新的程序文件加和校验值
    u8 ProgramXorValue;       //无线等待更新的程序文件异或校验值
    u8 FilePageNum;           //等待更新的程序文件总页数
    u8 res[2];
} SPI_FLASH_SAVE;

extern SPI_FLASH_SAVE sp;

#define UPGRADE_SD 0
#define UPGRADE_OTA 1
#define UPGRADE_ERR 2

typedef struct {
    u16 crc;
    u16 flag;
    bool enable; //升级使能
    bool ack;    //数据包应答
    bool newest; //升级到最新版本
    bool banben; //

    u8 mode;         //升级模式1: 无线  0: SD卡
    u8 err_cnt;      //连续溢出次数视作失败
    u8 xor ;         //服务器异或值
    u8 xor_tmp;      //服务器异或值
    u8 filename_len; //文件名长度
    u16 pack_num;    //总包数
    u16 pack_no;     //已读包数
    u16 time_out;    //60秒 溢出时间 1包数据时间
    u16 ack_tick;    //应答时间
    u16 buf_len;     //长度
    u32 enable_tick; //使能的时间
    u32 file_len;    //当前升级文件的总长度
    u32 read_len;    //当前已读升级文件大小
    u32 d_len;       //数据总长度
    u32 sum;         //服务器累加和
    u32 sum_tmp;     //服务器累加和
    u8 filename[36]; //升级文件的文件名称
    bool slv_en;
} s_UPDATAECONTROL;

typedef struct
{
    bool start;
    bool start_m;
    bool boot;
    uchar step; //字库升级步骤
    uchar typ;  //字库版本类型  0老版本  1新版本131209 (暂时未使用)
    u8 total_pack;
    u8 write_pack;
    u16 PackLen;
    ulong wr_lgth; //待升级字节总数
    ulong cnt;     //已写入字节数计数
    ulong tmr;
} font_lib_update_struct;

typedef struct
{
    uchar step; //升级步骤

    uchar check_xor; //异或校验
    ulong check_sum; //和校验

    ulong wr_lgth;       //待升级字节总数
    ulong cnt;           //已写入字节数计数
} update_tf_data_struct; //升级数据结构

typedef struct
{
    u8 step;            //参数配置步骤
    u32 wr_lgth;        //待升级的字节数
    u32 cnt;            //已写入的字节计数
} parameter_cfg_struct; //参数配置结构体 2017-12-6

typedef struct
{
    u8 step;            //行驶记录仪读取步骤
    u32 rd_lgth;        //带读取的字节数
    u32 cnt;            //以读取的字节数
} read_rcd_data_struct; //U盘读取行驶记录仪结构体 2017-12-15

typedef struct //2017-12-6
{
    bool ic_platform_flag; //平台开卡标志位

} upgrade_status_flag;

extern upgrade_status_flag usb_upgrade; //usb升级标志结构体

extern s_UPDATAECONTROL upgrade_m; //升级控制结构体
extern font_lib_update_struct font_lib_update;
extern update_tf_data_struct update_tf_data;
extern parameter_cfg_struct parameter_auto_cfg;
extern read_rcd_data_struct read_rcd_data_cfg; //记录仪数据读取结构体 2017-12-15
extern u8 update_buf[];                        //升级数据缓存
extern void auto_ack_tr9_data(void);

extern void init_upgrade(void);
extern u8 check_update_version(u8 type, u8 *buf, u16 len);
extern void update_from_sd_mess(u8 *buf, u16 len);
extern void upgrade_program_task(void);
extern void com1_down_font_lib(void);
extern void update_font_lib(u8 *lib, u16 len);
extern void update_tf_main(u8 *lib, u16 len);

#endif /* __UPGRADE_H */
