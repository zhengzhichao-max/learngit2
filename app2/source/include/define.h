/**  74303540
  ******************************************************************************
  * @file    define.h
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  宏定义
  ******************************************************************************
*/
/**
超越平台
0x52	1	正反转(0:未知；1：正转（空车）2:反转（重车）；3：停转)
0x2B	4	二路油量，4字节，可表示两个模拟量
0x50	2	一路油量，2字节，单模拟量
**/
#ifndef __DEFINE_H__
#define __DEFINE_H__

#define XH7_EC20_ZTC_V300
/*======================================================================================*/
/*产品PCB硬件资源配置*/
//PRODUCT

#define M9C19A 770

//MAIN_BOARD
#define P8036_V100 200

//GSM
#define EC20 30   //C网模块
#define SLM790 20 //C网模块

//MCU
#define STM32F103VCT6 1

//DATA FALSH
#define SST25VF032B 1

//区域检测
#define AREA_FUN_NO 1
#define AREA_BASE 2
#define AREA_EXT 3
#define AREA_CUSTOMIZE 4

//路线检测
#define LINE_NO 1
#define LINE_BASE 2
#define LINE_EXT 3
#define LINE_CUSTOMIZE 4

//摄像头
#define CMA_NO 1
#define CMA_BASE 2
#define CMA_CUSTOMIZE 3

//批量数据发送
#define BATCH_NO 1
#define BATCH_BASE 2
#define BATCH_CUSTOMIZE 3

//记录仪功能
#define RCD_NO 1
#define RCD_BASE 2
#define RCD_CUSTOMIZE 3

//双IP 功能
#define DOUBLEIP_NO 1
#define DOUBLEIP_BASE 2
#define DOUBLE_CUSTOMIZE 3

//CAN 功能
#define _CAN_NO 1
#define _CAN_BASE 2
#define _CAN_CUSTOMIZE 3

//支持协议
#define NET_TR 1
#define NET_JTB 2  //交通部协议
#define NET_GB 3   //国标协议
#define NET_DB44 4 //广东省标协议

//电话功能
#define SOUND_NO 1
#define SOUND_BASE 2
#define SOUND_CUSTOMIZE 3

//一体机从机通讯
#define GNSS_NO 1
#define GNSS_BASE 2
#define GNSS_CUSTOMIZE 3

//基准参考电压
#define VOLT_25 1
#define VOLT_33 2

/*======================================================================================
客户功能配置
原则
1.程序在设计之初，已将DRV层和MID层尽量抽象，若有改动尽量在APP层修改
2.对于合理的改进，并具备所有产品共性的，直接在主程序重改动
3.对于个别功能轻微改动，并且客户改动不频繁的，直接使用宏定义方式修改
4.对于客户要求功能大量改动，或改动比较频繁的，采用副本文件改动
5.如果客户提出的功能，该模块化程序无法实现时，另建工程，项目重新开发
6.在修改过程中，修改不能影响其他标准化产品的功能
======================================================================================*/

/*-----------硬件版本-----硬件版本-----硬件版本------硬件版本-------------*/
#define PRODUCT_NAME "XH7_EC20"
#define PCB_VER "ZTC"
#define FW_VER "V300"
#define FW_TIME "211130"
//因调度屏显示限制，定长24个字节
#define FW_VERSION PRODUCT_NAME "_" PCB_VER "_" FW_VER "_" FW_TIME
/*-----------硬件版本-----硬件版本-----硬件版本------硬件版本-------------*/

#define C_TR9 1

//限速+防拆
#pragma region                //#ifdef XH7_EC20_ZTC_V300 //V600,小模块
#ifdef XH7_EC20_ZTC_V300      //V600,小模块
#define TRWY_PRODUCT (M9C19A) //2018-1-5 新版本带USB功能
#define MAIN_BOARD (P8036_V100)
#define GSM (EC20)
#define MCU (STM32F103VCT6)
#define P_CAN (_CAN_BASE) //需要CAN功能
#define P_DBIP (DOUBLEIP_NO)

#define P_AREA (AREA_FUN_NO)
#define P_LINE (LINE_NO) //线路检测
//#define P_LINE				(LINE_BASE)         //线路检测
#define P_CMA (CMA_NO)
#define P_BATCH (BATCH_NO)
#define P_RCD (RCD_BASE)
#define P_NET (NET_JTB)
#define P_SOUND (SOUND_BASE)
#define P_GNSS (GNSS_NO)
#define p_VOLT_BECH (VOLT_33)

#define JTT_808_2019 1

#define COMPANY (C_TR9)

#pragma region /*调试版本/测试版本/发布版本*/
//****************************************************
//M9C19A220905V0D1D4D
#define TR_DEBUG
// #undef TR_DEBUG //发版注意更改:【注释】 = 【DEBUG版本】

//****************************************************
#if !defined(TR_DEBUG)
//M9C19A220905V0D1D4T
#define TR_TEST
//#undef TR_TEST//发版注意更改:【注释】 = 【TEST版本】
#endif /*TR_DEBUG*/

//****************************************************
#if !defined(TR_DEBUG) && !defined(TR_TEST)
//M9C19A220905V0D1D4
#define TR_PROD
#endif //#if !defined(TR_DEBUG) && !defined(TR_TEST)

#define TR_USING_IAP 0 //发版注意更改:【是否使用IAP】
/*
512K
STM32F105VC(256K)
IROM1:
0x8000000
0x7F400
*/

#pragma endregion /*调试版本/测试版本/发布版本*/
/*调试版本/测试版本/发布版本*/

/*-----------软件版本-----软件版本-----软件版本------软件版本-------------*/
#if (1)                             //2022-09-23
#define TR9_SOFT_VERSION_MAX_LEN 30 //IAP & APP

#define TR9_APP_VERSION_V_XXX "V1.0.13" //发版注意更改:

#define TR9_IAP_VERSION ((const char *)(0x08001000))                               //限长<=30
#define TR9_IAP_BUILD_TIME ((const char *)(0x08001000 + TR9_SOFT_VERSION_MAX_LEN)) //限长12(get_build_time_need_free)//"Sep 20 2022 08:45:26 D/T"

#pragma region //软件版本定义//EC20+STM 软件版本
#if defined(TR_DEBUG)
//调试版本
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_V_XXX
#elif defined(TR_TEST)
//测试版本
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_V_XXX
#elif defined(TR_PROD)
//发布版本
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_V_XXX
#endif                              /*TR_DEBUG*/
#pragma endregion                   //软件版本定义//EC20+STM 软件版本
#else                               //2022-08-15
#define TR9_SOFT_VERSION_MAX_LEN 30 //IAP & APP

#define TR9_APP_VERSION_NAME "M9C19A"
#define TR9_APP_VERSION_TIME "220922"  //发版-注意更改:
#define TR9_APP_VERSION_V_XXX "V1.0.9" //发版-注意更改:

#define TR9_IAP_VERSION ((const char *)(0x08001000))                               //限长<=30
#define TR9_IAP_BUILD_TIME ((const char *)(0x08001000 + TR9_SOFT_VERSION_MAX_LEN)) //限长12(get_build_time_need_free)//"Sep 20 2022 08:45:26 D/T"

#pragma region //软件版本定义//EC20+STM 软件版本
#if defined(TR_DEBUG)
//调试版本
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_NAME "" TR9_APP_VERSION_TIME "" TR9_APP_VERSION_V_XXX "D"
#elif defined(TR_TEST)
//测试版本
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_NAME "" TR9_APP_VERSION_TIME "" TR9_APP_VERSION_V_XXX "T"
#elif defined(TR_PROD)
//发布版本
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_NAME "" TR9_APP_VERSION_TIME "" TR9_APP_VERSION_V_XXX
#endif            /*TR_DEBUG*/
#pragma endregion //软件版本定义//EC20+STM 软件版本
#endif
/*-----------软件版本-----软件版本-----软件版本------软件版本-------------*/
#endif

//**********各种调试接口****************************************
#define _CLK72M_
#define _STM32F105_
//**************************************************************

#ifndef TRWY_PRODUCT
#error "PRODUCT == ?"
#endif
#pragma endregion //#ifdef XH7_EC20_ZTC_V300 //V600,小模块

#define DEBUG_EC20
#define BOOT_MODEm

/*======================================================================================*/

/*======================================================================================*/
//2022-08-20
//设置上报给RK的【脉冲速度】，采用【卫星速度】覆盖
//220828速度类型:脉冲速度，采用GPS的速度，上报给RK（GPS速度，覆盖了，脉冲速度）
//脉冲速度，采用GPS的速度，上报给RK（GPS速度，覆盖了，脉冲速度）
#define PULSE_SPEED_2_RK_USE_GPS_SPEED 0
/*======================================================================================*/

/*======================================================================================*/
//0:菜单第0项为空函数//1：菜单第0项，为正常的，other菜单函数
//想要设置为【第0项为空函数】的原因是，other菜单函数，执行条件是index非0
//menu_cnt.menu_other//menu_other_point
#define MO_ZERO_IS_NULL_FUN 0 //0功能正常显示//1显示页面不正确(待进一步调试)
/*======================================================================================*/

/*======================================================================================*/
#define USE_SYSTERM_BITS1_UNION
//#undef USE_SYSTERM_BITS1_UNION
#define USE_MID_TEST
#undef USE_MID_TEST //不测试
/*======================================================================================*/

/*======================================================================================*/
//开发串口0（PA9 PA10）的handset功能
#define USING_HANDSET
//#undef USING_HANDSET
/*======================================================================================*/

/*======================================================================================*/
//>>>>>>>>>>>>> 日志模块 <<<<<<<<<<<<<<
/*======================================================================================*/
//启用日志模块
#define LOG_USING_LOGGER 1 //发版注意更改:
//启用日志模块颜色功能

#pragma region //配置日志模块
//启用日志模块颜色功能
#define LOG_USING_COLOR 1 //发版注意更改:
//启用日志模块等级功能//两种情况:1）= 0，无等级字符，可以有等级颜色；2）= 1，有等级字符，有颜色/无颜色；
#define LOG_USING_LEVEL 1 //发版注意更改:
//启用日志模块时间功能
#define LOG_USING_TIME_INFO 0 //发版注意更改:
/*=======*/
//【文件名】【函数名】【行号】
#define LOG_USING_FILE_INFO 0 //发版注意更改:
#define LOG_USING_FUNC_INFO 1 //发版注意更改:
#define LOG_USING_LINE_INFO 1 //发版注意更改:
/*=======*/
//填表+到模块中验证
//#elif (LOG_USING_COLOR && LOG_USING_TIME_INFO && !LOG_USING_LEVEL && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#pragma endregion //配置日志模块
/*======================================================================================*/

typedef enum {
    u8_bit0 = 0x01,
    u8_bit1 = 0x02,
    u8_bit2 = 0x04,
    u8_bit3 = 0x08,
    u8_bit4 = 0x10,
    u8_bit5 = 0x20,
    u8_bit6 = 0x40,
    u8_bit7 = 0x80,
} u8_bit_enum;

typedef enum {
    u16_bit0 = 0x0001,
    u16_bit1 = 0x0002,
    u16_bit2 = 0x0004,
    u16_bit3 = 0x0008,
    u16_bit4 = 0x0010,
    u16_bit5 = 0x0020,
    u16_bit6 = 0x0040,
    u16_bit7 = 0x0080,

    u16_bit8 = 0x0100,
    u16_bit9 = 0x0200,
    u16_bitA = 0x0400,
    u16_bitB = 0x0800,
    u16_bitC = 0x1000,
    u16_bitD = 0x2000,
    u16_bitE = 0x4000,
    u16_bitF = 0x8000,
} u16_bit_enum;

typedef enum {
    false = 0,
    true = !false,
} bool;

#ifndef rt_uint8_t
#define rt_uint8_t u8
#endif

#ifndef uint8_t
#define uint8_t u8
#endif

typedef enum {
    RT_EOK = 0,
    RT_ERROR,
    RT_ETIMEOUT,
    RT_EFULL,
    RT_EEMPTY,
    RT_ENOMEM,
    RT_ENOSYS,
    RT_EBUSY,
    RT_EIO,
    RT_EINTR,
    RT_EINVAL,
} rt_err_t;

#pragma region //调试开关宏定义
/*======================================================================================*/
//2022-08-22
//调试期间：禁用蜂鸣器
#ifdef TR_DEBUG
//调试版本
#define DEBUG_EN_BEEP 0
#else
//TR_TEST//测试版本
//TR_PROD//发布版本
#define DEBUG_EN_BEEP 1
#endif
/*==========================*/
//调试期间：禁用看门狗
#ifdef TR_DEBUG
//调试版本
#define DEBUG_EN_GPS_UN_dingWei_printf 0 //发版注意更改:
#else
//TR_TEST//测试版本
//TR_PROD//发布版本
#define DEBUG_EN_GPS_UN_dingWei_printf 0
#endif
/*==========================*/
//调试期间：禁用看门狗
#ifdef TR_DEBUG
//调试版本
#define DEBUG_EN_WATCH_DOG 0
#else
//TR_TEST//测试版本
//TR_PROD//发布版本
#define DEBUG_EN_WATCH_DOG 1
#endif
/*==========================*/
//rcd_3c是过标的时候使用的。
//u1_m.cur.b.rcd_3c
#if defined(TR_DEBUG) || defined(TR_TEST)
#define DEBUG_EN_RCD_3C 0
#else
//TR_PROD//发布版本
#define DEBUG_EN_RCD_3C 1
#endif
/*======================================================================================*/
#pragma endregion //调试开关宏定义

/*======================================================================================*/
//>>>>>>>>>>>>> 功能模块开关 bg<<<<<<<<<<<<<<
/*======================================================================================*/
//启用模块：【排查问题：复位RK、上电RK；增加IO操作日志】
#define MODULE_USING_pwr_rst_rk_logSave 1 //发版注意更改://开启/禁用本模块
#define MODULE_DEBUG_pwr_rst_rk_logSave 0 //发版注意更改://开启/禁用部分调试功能//保留基本调试功能
//>>>>>>>>>>
//启用模块：【排查问题：】
#define MODULE_USING_self_chk 1 //发版注意更改://开启/禁用本模块
#define MODULE_DEBUG_self_chk 1 //发版注意更改://开启/禁用部分调试功能//保留基本调试功能
//>>>>>>>>>>
//启用模块：【排查问题：】
#define MODULE_USING_lsm6ts3trc 0 //发版注意更改://开启/禁用本模块
#define MODULE_DEBUG_lsm6ts3trc 0 //发版注意更改://开启/禁用部分调试功能//保留基本调试功能
//>>>>>>>>>>
//启用模块：【排查问题：】
#define MODULE_DEBUG_ic_card 0 //发版注意更改://开启/禁用部分调试功能//保留基本调试功能
//>>>>>>>>>>
//启用模块：【排查问题：】
#define MODULE_DEBUG_ota 1 //发版注意更改://开启/禁用部分调试功能//保留基本调试功能
//>>>>>>>>>>

#ifdef TR_DEBUG
#define USE_DEBUG_FUNCTION
//#undef USE_DEBUG_FUNCTION
#define TIC_USE_DEBUG_FUNCTION 5
#endif
/*======================================================================================*/
//>>>>>>>>>>>>> 功能模块开关 end<<<<<<<<<<<<<<
/*======================================================================================*/

// typedef enum {
//     DISABLE = 0,
//     ENABLE = !DISABLE
// } FunctionalState;

#endif /* __DEFINE_H__ */
