/**
  ******************************************************************************
  * @file    debug.h 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   ���͹���,�����ڲ����ͺ��ⲿ���ͺй���
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
        u32 near_light : 1;  //�����
        u32 far_light : 1;   //Զ���
        u32 right_light : 1; //��ת
        u32 left_light : 1;  //��ת
        u32 brake : 1;       //ɲ��

        u32 acc : 1; //??
        u32 ic : 1;  //IC
        u32 tf : 1;  //TF��
        u32 sim : 1;
        u32 alarm : 1;
        u32 pwr : 1; //��ѹ
        u32 res0 : 1;
        u32 print : 1;

        u32 u1 : 1;
        u32 u4 : 1;
        u32 u5 : 1;
        u32 gps : 1; //��λģ��
        u32 gprs : 1;
        u32 plus : 1; //�����ٶ�
        u32 regi : 1; //����ʽ����
        u32 tts : 1;

        u32 res08 : 8;
    } b;
} U_HARDWARE_STATUS;

typedef struct {
    bool pass;
    bool touch; //�յ���Ϣ��־
    u8 cnt;
    u8 buff[20];
    u32 tick;
} Factory_comm;

typedef struct {
    bool tst; //��������
    bool rst;
    u8 net_stus; //����״̬
    u8 csq;      //�ź���
    u16 tmr;     //���ʱ��

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
