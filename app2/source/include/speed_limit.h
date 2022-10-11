#ifndef __SPEED_LIMIT_H__
#define __SPEED_LIMIT_H__

/*****************���ձ�׼ *******************
1����������
2�����ٿ��� ���������١�ƽ̨���� 
3����������¡���ָ�������޷������㵹
4����������¡�ƫ��·�߽���������ʾ
5�����ٱ�����ʾ�����ٱ�����ʾ
6����������¡����ܱ��Զ������޷����С�
**********************************************/

#define car_state_changed_dis_tim ((u16)5000)    //��ʾ����ͣ��ʱ��
#define car_state_changed_alarm_tim ((u16)10000) //��������
#define car_speed_fil_tim ((u16)10)              //���ٹ���ʱ��
#define car_speed_over_tim ((u16)100)            //����ָ���ʱ��

#define auto_report_ztc_time_inv 180

/*
1������·��
2���뿪·�ߣ�Խ�磩
3�����빤��  �����������¼��ϱ���
4���뿪����   (�뿪�����¼��ϱ�)
5���������ɳ�
6���뿪���ɳ�
7����������Ȧ
8���뿪����Ȧ
9������ͣ����
10���뿪ͣ����
11���������
12���뿪����
13���ٶ�
1���أ�2�����ɳ���3������Ȧ��4��������5��ͣ����
*/
#define eZTC_IN_ROAD 1
#define eZTC_OUT_ROAD 2
#define eZTC_IN_WORKSPACE 3
#define eZTC_OUT_WORKSPACE 4
#define eZTC_IN_DEALSPACE 5
#define eZTC_OUT_DEALSPACE 6
#define eZTC_IN_LIMIT_ROUND 7
#define eZTC_OUT_LIMIT_ROUND 8
#define eZTC_IN_STOP_LOT 9
#define eZTC_OUT_STOP_LOT 10
#define eZTC_IN_FORBID_SPACE 11
#define eZTC_OUT_FORBID_SPACE 12
#define eZTC_LIFT_EVENT 13

#define _ZB_SW_OPEN 0x00
#define _ZB_SW_CLOSE 0x55
#define _ZZ_SW_OPEN 0x55
#define _ZZ_SW_CLOSE 0x00
#define _AREA_IN 0x55
#define _AREA_OUT 0x00

#define _RELAY_OFF 0x55
#define _RELAY_ON 0x00

typedef struct
{
    bool car_cover;    //������״̬  0:�պϣ�1:��
    bool car_box;      //����״̬0���պϣ�1������
    bool car_max_load; //����״̬
    bool car_lock;     //����״̬
    bool car_rise;     //����״̬ -�̵���������
    bool monitor;      //�ܿ�  0���ر�   1����

    u8 speed_type; //���ٱ�־  0δ��������  1�ѷ��ͳ���
    u8 speed_flag; //��������  0��ͨ����    1���ⳬ��(��⵽�������仯)
    u16 speed_fil; //�ٶ��ȶ�ʱ��
    u16 speed_tim; //��������ʱ����
} ZTC_STATUS_STRUCT;

typedef struct
{
    bool bGPsErr;
    bool bLock;
    u32 tmr; //寤惰�?1鍒嗛挓鍚庨�?鍒朵妇鍗?
} GPS_MONITOR_ERR;

typedef struct
{
    bool car_speed_limit; //����״̬
    bool can_ack_flag;
    bool can_err_flag;
    u32 lock_time;    //��������ʱ����
    u32 can_err_time; //CAN �������ʱ��
} can_lock_monitor_struct;

/*ztc FUZHOU -e768 */
typedef union {
    u16 reg;
    struct
    {
        bool out_area : 1;   //Խ��
        bool prohibit : 1;   //����
        bool secretly : 1;   //͵��
        bool over_spd : 1;   //����
        bool no_license : 1; //��֤
        bool acc : 1;        //ACC���
        bool ant_open : 1;
        bool res3 : 1;

        bool car_box : 1;          //�Ƿ�����
        bool bigcar_nolicense : 1; //�س���׼��Ч֤

        bool res6 : 6; //Ԥ��
    } bit;
} ZTC_VEHICLE_STATE;

typedef struct
{
    bool open;     //����״̬
    bool all_flag; //��ȫ����״̬   0�� δ��ȫ������1����ȫ����
    u16 open_time; //��������ʱ����
    u16 ctin_time; //��ȫ�򿪳���ʱ��10��
} all_open_monitor_struct;

typedef struct
{
    u32 tmr;
    bool start;
} LIMIT_TEST;

typedef struct
{
    u16 crc;
    u16 flag;
    u8 lift;   //����    0��������״̬��1��������2δ����
    u8 weight; //����	0��������״̬��1�����أ�  2����
    u8 close;  //�ܱ�    0��������״̬��1���ܱգ�2δ�ܱ�
    u8 all;    //ȫ��
    u32 f_l_tmr;
    u32 f_w_tmr;
    u32 f_c_tmr;
    u32 f_a_tmr;
    u32 f_l_tick;
    u32 f_w_tick;
    u32 f_c_tick; //�ܱ�
    u32 f_a_tick; //ȫ��
} FALSE_CX_TMR;

typedef struct
{
    u16 crc;
    u16 flag;
    bool lock_gk; //�ܿش�����£�ҵ���߼�ʧЧ
    bool relay_gk;
    bool speed_gk;
    bool lock_s; //=0; ��������� =1���ƾ���
    bool relay_s;
    u8 speed_s;   //����ֵ
    u32 lock_tmr; //��Чʱ��
    u32 relay_tmr;
    u32 speed_tmr;
    u32 lock_tick; //��Чʱ��
    u32 relay_tick;
    u32 speed_tick;
} ZTB_CONTROL; //������ܿ�

extern u8 ztc_set_limit_speed; //������ �趨 ����
extern can_lock_monitor_struct can_lock_ctrl;
extern all_open_monitor_struct all_open_monitor;
extern GPS_MONITOR_ERR gps_monitor;
extern LIMIT_TEST limit_test;
extern u16 car_weight;
extern FALSE_CX_TMR cx_false_tmr;
extern u8 limit_speed;
extern bool ztb_if_gk(void);
extern void ctrl_all_open_signal(void);
extern ZTB_CONTROL ztb_ctrl;

extern ZTC_STATUS_STRUCT ztc_status;
extern void speed_lock_monitor(void);

extern void ztc_event_report_center(u8 evnt);
extern void ztc_status_report_center(void);
extern void ztc_message_task(void);
extern void speedlimit_parse_proc(void);
extern void close_spx_voice(void);
extern void can_speed_cacl_coefficient(void);
extern void gps_shelter_monitor(void);
extern bool judge_all_open(void);

extern bool out_if_raise(void);
#endif /* __SPEED_LIMIT_H__ */
