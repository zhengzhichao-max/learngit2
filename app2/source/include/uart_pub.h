/**
  ******************************************************************************
  * @file    uart_pub.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  串口管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

#ifndef __UART_PUB_H
#define __UART_PUB_H

typedef union {
    u32 reg;
    struct {
        u32 tt : 1;         //透传		//透传??-升级字库及BOOT
        u32 debug : 1;      //打印上线信息或TR9数据
        u32 rcd_3c : 1;     //lcd 		//默认功能
        u32 speedlimit : 1; //限速
        u32 tr9 : 1;//透传显示RK
        u32 weight : 1;    //载重
        u32 handset : 1;   //
        u32 read_card : 1; //读卡器
        u32 yd_oil : 1;    //油标

        //u32 led		 	:1;		//显示屏
        u32 navigation : 1; //导航
        //u32 oil_mass		:1;		//油量
        //u32 ethyl_alcohol	:1;		//酒精

        u32 res2 : 6;
        u32 res4 : 8;
        u32 res3 : 8;
    } b;
} uart_func_list;

typedef struct {
    u16 crc;           //必须添加该变量, ee_pwrite 添加crc
    u16 flag;          //必须添加该变量, ee_pwrite 添加8700
    uart_func_list u1; //串口1 管理
    uart_func_list u4; //串口4 管理
    u8 u5_func;        //兼容8002 功能
    u8 res1;
    u8 res[12];
} s_any_uart_set;

extern s_any_uart_set us;

typedef struct {
    u32 jiff; //被忽略的时间
} S_BYPASS_DEBUG;
extern S_BYPASS_DEBUG bypass_debug;

typedef struct {
    bool delay_en;  //延时使能
    bool delay_err; //延时错误
    u16 res;
    u32 delay_jiff; //延时的时间
} any_uart_remap_struct;
extern any_uart_remap_struct uart_remap;

typedef struct {
    bool news; //新的消息
    bool ack;  //已处理完成，等待挪动指针
    u8 *cur_p; //当前帧位置
    u8 res;    //保留
    u16 c_len; //当前帧长度
    u16 res1;  //保留
    u32 n_jfs; //news jiffies
} UART_MANAGE_STRUCT;

extern u8 get_flag_7e_cnt(u8 *buf_t, u16 b_len_t, u8 flag);
extern s16 get_flag_7e_pointer(u8 *buf_t, u16 b_len_t, u8 flag);

extern bool verify_any_uart_func(u8 source, u16 fun1);
extern void send_any_uart(u8 source, u8 *data1, u16 len1);
extern void load_any_uart_remap(void);
extern u8 get_uart_news(UART_MANAGE_STRUCT *c);
extern u8 get_uart_tr9(UART_MANAGE_STRUCT *c);
extern u8 get_uart_3c(UART_MANAGE_STRUCT *c);

extern u16 copy_uart_news(u8 uart_id_t, u8 *p, u8 *s, u16 s_len);
extern void set_uart_ack(u8 from);
extern void uart_manage_task(void);
#endif /* __UART_PUB_H */
