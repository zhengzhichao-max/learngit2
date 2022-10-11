#ifndef __SPEED_LIMIT_H__
#define __SPEED_LIMIT_H__

/*****************验收标准 *******************
1、出厂内置
2、限速控制 、主控限速、平台限速 
3、重载情况下、非指定区域无法举升倾倒
4、重载情况下、偏离路线进行限速提示
5、超速报警提示、限速报警提示
6、重载情况下、非密闭自动限速无法运行。
**********************************************/

#define car_state_changed_dis_tim ((u16)5000)    //显示界面停留时间
#define car_state_changed_alarm_tim ((u16)10000) //报警周期
#define car_speed_fil_tim ((u16)10)              //超速过滤时间
#define car_speed_over_tim ((u16)100)            //限速指令发送时间

#define auto_report_ztc_time_inv 180

/*
1：进入路线
2：离开路线（越界）
3：进入工地  （进出工地事件上报）
4：离开工地   (离开工地事件上报)
5：进入消纳场
6：离开消纳场
7：进入限速圈
8：离开限速圈
9：进入停车场
10：离开停车场
11：进入禁区
12：离开禁区
13：举斗
1工地，2、消纳场，3、限速圈，4、禁区，5、停车场
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
    bool car_cover;    //环保盖状态  0:闭合；1:打开
    bool car_box;      //车厢状态0：闭合；1：举升
    bool car_max_load; //载重状态
    bool car_lock;     //锁车状态
    bool car_rise;     //举升状态 -继电器操作后
    bool monitor;      //管控  0：关闭   1、打开

    u8 speed_type; //超速标志  0未发生超速  1已发送超速
    u8 speed_flag; //超速类型  0普通超速    1特殊超速(检测到传感器变化)
    u16 speed_fil; //速度稳定时间
    u16 speed_tim; //发送限速时间间隔
} ZTC_STATUS_STRUCT;

typedef struct
{
    bool bGPsErr;
    bool bLock;
    u32 tmr; //瀵ゆ儼绻?1閸掑棝鎸撻崥搴ㄦ?閸掓湹濡囬崡?
} GPS_MONITOR_ERR;

typedef struct
{
    bool car_speed_limit; //限速状态
    bool can_ack_flag;
    bool can_err_flag;
    u32 lock_time;    //发送限速时间间隔
    u32 can_err_time; //CAN 溢出出错时间
} can_lock_monitor_struct;

/*ztc FUZHOU -e768 */
typedef union {
    u16 reg;
    struct
    {
        bool out_area : 1;   //越界
        bool prohibit : 1;   //闯禁
        bool secretly : 1;   //偷运
        bool over_spd : 1;   //超速
        bool no_license : 1; //无证
        bool acc : 1;        //ACC点火
        bool ant_open : 1;
        bool res3 : 1;

        bool car_box : 1;          //非法举升
        bool bigcar_nolicense : 1; //重车核准无效证

        bool res6 : 6; //预留
    } bit;
} ZTC_VEHICLE_STATE;

typedef struct
{
    bool open;     //锁车状态
    bool all_flag; //完全举升状态   0： 未完全举升；1：完全举升
    u16 open_time; //发送限速时间间隔
    u16 ctin_time; //完全打开持续时间10秒
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
    u8 lift;   //举升    0：传感器状态；1、举升；2未举升
    u8 weight; //载重	0：传感器状态；1、重载；  2空载
    u8 close;  //密闭    0：传感器状态；1、密闭；2未密闭
    u8 all;    //全举
    u32 f_l_tmr;
    u32 f_w_tmr;
    u32 f_c_tmr;
    u32 f_a_tmr;
    u32 f_l_tick;
    u32 f_w_tick;
    u32 f_c_tick; //密闭
    u32 f_a_tick; //全举
} FALSE_CX_TMR;

typedef struct
{
    u16 crc;
    u16 flag;
    bool lock_gk; //管控打开情况下；业务逻辑失效
    bool relay_gk;
    bool speed_gk;
    bool lock_s; //=0; 允许举升； =1限制举升
    bool relay_s;
    u8 speed_s;   //限速值
    u32 lock_tmr; //有效时长
    u32 relay_tmr;
    u32 speed_tmr;
    u32 lock_tick; //有效时长
    u32 relay_tick;
    u32 speed_tick;
} ZTB_CONTROL; //渣土办管控

extern u8 ztc_set_limit_speed; //渣土车 设定 车速
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
