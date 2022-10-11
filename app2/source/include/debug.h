/**
  ******************************************************************************
  * @file    debug.h 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   测油功能,包含内部测油和外部测油盒功能
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef __DEBUG_H__
#define __DEBUG_H__

#if (0)
typedef union {
    u32 stus;
    struct {
        u32 sa : 1;          //???
        u32 sb : 1;          //???
        u32 sc : 1;          //??
        u32 near_light : 1;  //近光灯
        u32 far_light : 1;   //远光灯
        u32 right_light : 1; //右转
        u32 left_light : 1;  //左转
        u32 brake : 1;       //刹车

        u32 acc : 1; //??
        u32 ic : 1;  //IC
        u32 tf : 1;  //TF卡
        u32 sim : 1;
        u32 alarm : 1;
        u32 pwr : 1; //电压
        u32 res0 : 1;
        u32 print : 1;

        u32 u1 : 1;
        u32 u4 : 1;
        u32 u5 : 1;
        u32 gps : 1; //定位模块
        u32 gprs : 1;
        u32 plus : 1; //脉冲速度
        u32 regi : 1; //电阻式油针
        u32 tts : 1;

        u32 res08 : 8;
    } b;
} U_HARDWARE_STATUS;

typedef struct {
    bool pass;
    bool touch; //收到信息标志
    u8 cnt;
    u8 buff[20];
    u32 tick;
} Factory_comm;

typedef struct {
    bool tst; //启动测试
    bool rst;
    u8 net_stus; //网络状态
    u8 csq;      //信号量
    u16 tmr;     //检测时间

    U_HARDWARE_STATUS hardware; //

    Factory_comm uart1;
    Factory_comm uart4;
    Factory_comm uart5;
} Factory_Test;

#if (p_DEBUG == _DEBUG_BASE)
#define NET_NO_CARD 0x00
#define NET_TST_ASK 0x01
#define NET_TST_OK 0X02

extern Factory_Test factory_test;
#endif
#endif

// extern void debug_parse_proc(void);
// extern void fixture_read_com(u8 from);
// extern void test_fixture_task(void);

void debug_task(void);

/*
mode:0 => [i][promt time, 106]: func, str, 2016-09-15 09:12:13\r\n //promt time(tm, 0, func, str)
mode:1 => [i][promt time, 106]: func, str, 2016-09-15 09:12:13, //promt time(tm, 1, func, str)
mode:2 => str[2016-09-15 09:12:13],  //promt time(tm, 2, NULL, str)
mode:3 => str[2016-09-15 09:12:13]\r\n //promt time(tm, 3, NULL, str)
*/
void promt_time(time_t *time, int mode, log_level_enum log_level, const char *func, int line, const char *str, const char *end);
void chk_err(void);

#endif /*__DEBUG_H__*/
