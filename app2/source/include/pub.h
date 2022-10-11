/**
  ******************************************************************************
  * @file    pub.h
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   共用功能或函数管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef __DRV_PUB_H_
#define __DRV_PUB_H_

#include "include_all.h"

#include "ckp_mcu_file.h"
#include "recorder.h"
#include "buzzer_hdl.h"
#include "menu_process_enum.h"

#define TRUE 1
#define FALSE 0

#define ON 1
#define OFF 0
#define _ON 0
#define _OFF 1
#define _ACTIVE 0
#define _INACTV 1
#define OPEN 1
#define CLOSE 0

#define INPUT 0
#define OUTPUT 1
#define TRI_STATE 0
#define PULL_UP 1
#define LOW 0
#define HIGH 1
#define _LOW 1
#define _HIGH 0

#define FROM_NULL 0
#define FROM_U1 1
#define FROM_U4 2
#define FROM_U3 3
#define FROM_SLAVE 4
#define FROM_ALL 5
#define FROM_NET 6
#define FROM_INNER 7

#define CALL_IDLE 0
#define CALL_OUT 1
#define CALL_IN 2
#define CALL_ERR 3

#define VOICE_EXT 1
#define VOICE_HANDER 2

extern u32 tick;
extern u32 jiffies;
extern bool rouse_state;

typedef void (*funcp)(void);

#define delay1us() \
    do {           \
        nop();     \
        nop();     \
        nop();     \
        nop();     \
        nop();     \
        nop();     \
    } while (0) //nop(); //h 注意

#define xdelay() \
    do {         \
        nop();   \
        nop();   \
        nop();   \
        nop();   \
        nop();   \
        nop();   \
    } while (0)

#define _bool(v) ((v) ? true : false)
#define _max(a, b) ((a) > (b) ? (a) : (b))
#define _min(a, b) ((a) < (b) ? (a) : (b))

#define _coverc(c) ((u8)(tick - c))
#define _coveri(i) ((u16)(tick - i))
#define _covern(n) ((u32)(tick - n))

#define _pastc(c) ((u8)(jiffies - c))
#define _pasti(i) ((u16)(jiffies - i))
#define _pastn(n) ((u32)(jiffies - n))

extern u32 get_jiffies(void);

#define xpastc(c) ((u8)(get_jiffies() - c))
#define xpasti(i) ((u16)(get_jiffies() - i))
#define xpastn(n) ((u32)(get_jiffies() - n))

#define HANDSET_MAX_SIZE 600
#define HANDSET_MIN_SIZE 3

typedef struct
{
    VEHICLE_ALARM_UNION alarm;     //第二状态标志字 (长整形)
    UNION_VEHICLE_STATE car_state; //第一状态标志字 (长整形)
    point_t locate;                //经纬度
    u16 heigh;                     //高度数据
    u16 speed;                     //速度数据    1/10 km/h
    u16 direction;                 //方位角数据  0~360
    time_t time;                   //时间数据
    bool fixed;                    //是否定位

    //0x0200 信息体，附加信息
    u32 dist;            //距离 meter;
    u16 ad_oil;          //油量AD 值
    u16 oil_L;           //油量单位L
    u16 complex_speed;   //GPS 或脉冲速度， 1 km/h
    u16 complex_speed01; //GPS 或脉冲速度， 0.1 km/h
    u16 vot_V;           //电压
    u8 csq;              //CSQ 信号
    u8 fix_num;          //定位卫星颗数

    //其他信息
    u16 average_speed; //平均速度
    u16 pwr_vol;       //电压电压
    u8 pwr_per;        //电源百分比
    u8 pwr_type;       //车系电压
    bool moving;       //车辆是否行驶
    bool power;        //汽车电瓶是否有效

    bool ic_login;    //IC 卡登签
    bool ic_exchange; //更换登录卡
    u8 ic_driver[18]; //驾驶员信息
    u16 box_oil;
    u16 placeIostate;
    u8 res[31];
} MIX_GPS_DATA;

//0x1818  0x1819
typedef struct
{
    u32 tmr; //瓒呮椂鍚庢孩鍑? 10绉掑己鍒跺叧闂?

    bool pwr_open; //鍏抽棴鏃朵娇鐢?
    bool cmd_ack;
    bool link;
} DVR_POWER_CTRL;

extern MIX_GPS_DATA mix;
extern time_t sys_time;
#if (P_RCD == RCD_BASE)
extern u32 speed_plus;
extern u32 plus_speed;
#endif

#define GPS_SPEED 0x02
#define SENSOR_SPEED 0x01

#define LINK_VOID 0
#define LINK_DIAL 1
#define LINK_RUN 2

#define LINK_IP0 0x01 //主IP
#define LINK_IP1 0x02 //从IP
#define LINK_IP2 0x04 //升级IP
#define LINK_INIT 0x08
#define LINK_IP3 0x10 //保护IP

#define LEVEL_LST 0
#define LEVEL_COMMON 1
#define LEVEL_JACKING 6
#define LEVEL_RELEASE 7
#define LEVEL_SPEDOVER 8
#define LEVEL_HST 9

#define R_F_ALL 1
#define R_F_EXCEPT_IP 2

typedef struct
{
    u8 cur_ip;   //当前IP 类型
    u8 link_cnt; //当前链接次数
    u8 domain_cnt;
    u8 IP0_status;    //主IP
    u8 IP1_status;    //备用IP
    u8 IP2_status;    //升级IP
    u8 IP1_cnt;       //备用IP
    bool reset;       //立即复位
    bool ack;         //应答
    bool sms_rst_en;  //延时，复位
    u8 dbip;          //保留20140412 doubleip	 0x5a: 开启 其余:关闭
    u16 sms_rst_tick; //延时复位时开始时间
    u32 online_tick;  //GPRS 链接时的时间
} gprs_link_manange;

extern gprs_link_manange lm;

//extern bool sleep;
extern u32 heart_beat_tick;

#define PWR_UNKNOWN 0 //<6V || >42
#define PWR_12V 1     //6V~18V
#define PWR_24V 2     //18V~30V
#define PWR_36V 3     //30V~42V

#define VBUF_SIZE 10
#define incp_vbuf(p)                           \
    do {                                       \
        if (++p >= vbuf + VBUF_SIZE) p = vbuf; \
    } while (0)

typedef union {
    u32 reg;
    struct
    {
        u8 cut_oil : 1;      //断开油电电路
        u8 rels_oil : 1;     //恢复油电电路
        u8 from_net : 1;     //远程控制
        u8 from_handset : 1; //手柄控制
        u8 day_en : 1;
        u8 res0 : 3;

        u8 res1 : 8;
        u8 res2 : 8;
        u8 res3 : 8;
    } b;
} REMOTE_CAR;

typedef struct
{
    u16 crc;        //必须添加该变量, ee_pwrite 添加crc
    u16 flag;       //必须添加该变量, ee_pwrite 添加8700
    u32 total_dist; //单位米
    REMOTE_CAR car;
    u8 cover;        //当前日期
    u8 relay_switch; //当天SMS 短信发送条数
    u8 iccard;       //=1  已插入IC 卡；=0 未 插入IC 卡
    u8 ip_to_tr9;    //=1 待写入
    u8 voice_sw;
    u8 lock_gps;
    u8 lock_car_sw; //
    u8 area_in_out; //0x55  围栏内
    u8 illeage;     //非法举斗状态

} run_parameter;

typedef struct
{
    unsigned int time_on;  //开通时间
    unsigned int time_off; //断开时间
    unsigned int cycle;    //次数
    unsigned long phase;   //延时
    unsigned char level;   //级别
} out_ctrl_block;

typedef enum {
    PWR_IDLE,  //正常模式
    PWR_WAIT,  //临时唤醒
    PWR_SLEEP, //休眠模式
    PWR_ERR
} E_POWER_MAMNAGE;

typedef enum {
    IACC_IDLE,  //休眠模式
    IACC_CLOSE, //正常模式
    IACC_OPEN,  //临时唤醒

} E_ACC_MAMNAGE;

typedef struct
{
    bool swi;
    bool open;
    bool one;
    u32 time_on; //开通时间
    u32 one_time;
} can_speed_cali;

typedef struct
{
    bool reset; //复位//true时，会触发重启RK的动作
    bool ack;   //
    bool start; //超过90秒没响应；认为死机；重启
    bool deal;
    bool power_off;
    u32 reset_tmr;
    u32 deal_tmr;
    u32 off_time;

} HI3520_MONITTOR;

extern HI3520_MONITTOR s_Hi3520_Monitor;
extern u16 car_weight;

extern E_POWER_MAMNAGE pm_state;
extern E_POWER_MAMNAGE pm_cmd;
extern can_speed_cali can_s_cali;

extern run_parameter run;
extern VEHICLE_ALARM_UNION car_alarm;
extern UNION_VEHICLE_STATE car_state; //第一状态标志字 (长整形)
extern sCommunicationSet phone_cfg;
extern VEHICLE_ALARM_UNION bypass_alarm;
extern REMOTE_CAR rcmd;
extern out_ctrl_block car;
extern DVR_POWER_CTRL dvr_power;
extern u16 start_tr9_tmr;
//*****************************************************************************
//*****************************************************************************
//-----------		        	    常数定义   	              -----------------
//-----------------------------------------------------------------------------

#define usart_send_buff_max_lgth 1100      //串口1发送缓冲器最大长度定义  //1100
#define usart_rec_queue_max_lgth 2200      //串口1接收队列缓冲器最大长度定义  //2200
#define usart_rec_valid_buff_max_lgth 1024 //串口1接收有效数据缓冲器最大长度定义  //1024

#define sound_play_buff_max_lgth 512 //语音朗读缓冲区最大值

#define spi1_up_comm_team_max 64 //命令缓冲区最大值

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
//*****************************************************************************
//-----------		             特殊长度定义   	          -----------------
//-----------------------------------------------------------------------------

#define password_para_max_lgth ((u8)8)         //参数设置密码最大长度
#define password_oil_max_lgth ((u8)8)          //油路控制密码最大长度
#define password_car_max_lgth ((u8)8)          //锁车控制密码最大长度
#define password_sensor_max_lgth ((u8)8)       //传感器系数修改密码最大长度
#define password_speed_max_lgth ((u8)8)        //速度方式选择密码最大长度
#define password_lcd_para_rst_max_lgth ((u8)8) //液晶屏驱动初始化密码最大长度
//#define     password_regist_max_lgth    ((u8)8)        //注册相关参数密码最大长度

#define para_main_ip_max_lgth ((u8)32)    //主IP/主域名最大长度
#define para_backup_ip_max_lgth ((u8)32)  //备用IP/备用域名最大长度
#define para_id_max_lgth ((u8)12)         //本机ID最大长度
#define para_apn_max_lgth ((u8)32)        //APN最大长度
#define para_num_server_max_lgth ((u8)15) //服务号码最大长度   chwsh 2017
#define para_num_user_max_lgth ((u8)13)   //用户号码最大长度

#define rec_para_ratio_max_lgth ((u8)6)  //速度传感器系数ASCII码长度
#define rec_car_vin_max_lgth ((u8)17)    //车辆VIN码长度
#define rec_car_plate_max_lgth ((u8)8)   //车牌号码长度
#define rec_province_id_max_lgth ((u8)2) //省域ID长度
#define rec_city_id_max_lgth ((u8)4)     //市域ID长度

#if defined(JTT_808_2019)

#define rec_manufacturer_id_max_lgth ((u8)11) //制造商ID长度
#define rec_terminal_type_max_lgth ((u8)30)   //终端型号长度
#define rec_terminal_id_max_lgth ((u8)30)     //终端ID长度

#else

#define rec_manufacturer_id_max_lgth ((u8)5) //制造商ID长度
#define rec_terminal_type_max_lgth ((u8)20)  //终端型号长度
#define rec_terminal_id_max_lgth ((u8)7)     //终端ID长度

#endif
#define rec_car_type_max_lgth ((u8)12) //车辆号牌分类长度

#define telephone_num_max_lgth 16 //手动拨号最大长度

#define usart1_rec_overtime_max 2000 //串口接收超时最大时间2秒

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
//*****************************************************************************
//-----------		            条件编译   	                  -----------------
//-----------------------------------------------------------------------------

//#define     test_debug                      //调试模式定义
//#define     def_idwt_en                     //使能独立看门狗

//------------------------------//

//#define       dis_all_flag_test       //测试显示所有标志

//------------------------------//

//#define       jtbb_test_ver       //交通部测试版本
#define gb19056_ver //行驶记录仪版本

//------------------------------//

//#define       key_no_sound           //按键不发声音    //部标测试建议按键不发声
//#define       notice_no_sound        //提示不发声      //适合调试使使用

//------------------------------//

//#define      sla_ack_0xa0_comm_en          //从机应答主机A0命令使能开关

#define input_telephone_num_en //使能手动拨号功能
#define start_print_infor_en   //打印开机信息

//#define      spi1_send_data_to_usart1_en     //SPI1端口发送的数据发往串口1
//#define      spi1_rec_data_to_usart1_en      //SPI1端口接收的数据发送串口1

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//--------------------------------------------------------------
//------------            数据类型定义              ------------
//--------------------------------------------------------------

typedef struct
{
    u8 buff[usart_rec_queue_max_lgth]; //队列缓冲区

    u16 head; //队列头位置
    u16 tail; //队列尾位置
} usart_queue_struct;

typedef struct
{
    usart_queue_struct que;

    bool busy_flag;                           //数据处理完成标志
    u8 rec_step;                              //接收有效数据的步骤
    u16 cnt;                                  //接收数据计数器
    u8 v_data[usart_rec_valid_buff_max_lgth]; //有效数据
    u16 lgth;                                 //接收到有效数据的长度

    u32 rec_overtime; //接收超时计数器
} usart_rec_struct;   //串口接收数据结构体

typedef struct
{
    bool busy;       //忙标志   TRUE表示忙   FALSE表示空闲
    u8 *ptr;         //待发送数据指针
    u16 lgth;        //待发送数据总长度
    u16 cnt;         //待发送数据计数
} usart_send_struct; //串口发送数据结构体

typedef struct
{
    bool active;   //TRUE表示有新的按键操作有待处理    FALSE表示操作处理完毕
    bool lg;       //TRUE表示长按              FALSE表示短按
    u8 key;        //按键按下的键值
} key_data_struct; //按键操作结构体

typedef struct
{
    u8 para[password_para_max_lgth + 2]; //参数设置密码      前两个字节表示长度，短整形数据类型，密码内容为ASCII格式
    u8 oil[password_oil_max_lgth + 2];   //油量控制密码      前两个字节表示长度，短整形数据类型，密码内容为ASCII格式
    u8 car[password_car_max_lgth + 2];   //锁车密码          前两个字节表示长度，短整形数据类型，密码内容为ASCII格式

    u8 sensor[password_sensor_max_lgth + 2];             //传感器系数密码    前两个字节表示长度，短整形数据类型，密码内容为ASCII格式
    u8 speed[password_speed_max_lgth + 2];               //速度方式选择密码  前两个字节表示长度，短整形数据类型，密码内容为ASCII格式
    u8 lcd_para_rst[password_lcd_para_rst_max_lgth + 2]; //液晶屏参数初始化密码  前两个字节表示长度，短整形数据类型，密码内容为ASCII格式

    //u8  regist[password_regist_max_lgth+2];   //注册相关参数密码  前两个字节表示长度，短整形数据类型，密码内容为ASCII格式
} password_manage_struct;

typedef struct
{
    u8 m_ip[para_main_ip_max_lgth + 2];   //主IP /主域名 前两个字节表示长度，短整形数据类型，数据内容为ASCII格式
    u8 b_ip[para_backup_ip_max_lgth + 2]; //备用IP  /备用域名前两个字节表示长度，短整形数据类型，数据内容为ASCII格式

    u8 m_ip2[para_main_ip_max_lgth + 2];   //第二路主IP /主域名 前两个字节表示长度，短整形数据类型，数据内容为ASCII格式
    u8 b_ip2[para_backup_ip_max_lgth + 2]; //第二路备用IP  /备用域名前两个字节表示长度，短整形数据类型，数据内容为ASCII格式

    u8 id[para_id_max_lgth + 2];            //本机ID  前两个字节表示长度，短整形数据类型，数据内容为ASCII格式
    u8 apn[para_apn_max_lgth + 2];          //APN  前两个字节表示长度，短整形数据类型，数据内容为ASCII格式
    u8 num_s[para_num_server_max_lgth + 2]; //服务号码  前两个字节表示长度，短整形数据类型，数据内容为ASCII格式
    u8 num_u[para_num_user_max_lgth + 2];   //用户号码  前两个字节表示长度，短整形数据类型，数据内容为ASCII格式

    u8 bl_type;      //背光类型    (0\5\10\30\60\255) 其中0表示一直亮，255表示省电模式，其他表示时间单位为秒
    u8 buzzer_type;  //蜂鸣器类型  (0\1\2)
    u8 LCD_Contrast; //LCD对比度
} set_para_struct;

typedef struct
{
    u8 province_id[rec_province_id_max_lgth + 2];         //省域ID  前两个字节表示长度
    u8 city_id[rec_city_id_max_lgth + 2];                 //市域ID  前两个字节表示长度
    u8 manufacturer_id[rec_manufacturer_id_max_lgth + 2]; //制造商ID  前两个字节表示长度
    u8 terminal_type[rec_terminal_type_max_lgth + 2];     //终端型号  前两个字节表示长度
    u8 terminal_id[rec_terminal_id_max_lgth + 2];         //终端ID  前两个字节表示长度
    u8 car_plate[rec_car_plate_max_lgth + 2];             //车牌号码  前两个字节表示长度
    u8 car_vin[rec_car_vin_max_lgth + 2];                 //车辆VIN码  前两个字节表示长度
    u8 ratio[rec_para_ratio_max_lgth + 2];                //速度传感器系数值   HEX数据格式  前两个字节表示长度

    u8 pr_driver; //打印驾驶员类型  0打印驾驶员姓名	否则打印驾驶员工号

} recorder_para_struct;

typedef struct
{
    u8 set_load_status;     //设置装载状态   1为空载    2为满载    3半载
    u8 set_gps_module_type; //设置定位模块模式   1为单GPS  2为单BD    3为B+D
} host_no_save_para_struct; //不需要保存的主机设置参数

typedef struct
{
    u32 bl; //背光时间计数
} user_time_struct;

typedef struct
{
    u8 num;
    u8 uart_num;
    u32 mult_choose;
    u32 only_choose;

} mult_usart_set_struct;

//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

#define feed_wdt IWDG_ReloadCounter(); //喂狗操作

//*****************************************************************************
//*****************************************************************************
//-----------		        	数据类型定义    	          -----------------
//-----------------------------------------------------------------------------

typedef enum {
#if (MO_ZERO_IS_NULL_FUN == 1)
    mo_null_fun = 0, //复位，此刻将会显示规则菜单界面
    mo_reset,        //复位，此刻将会显示规则菜单界面
#else
    mo_reset = 0, //复位，此刻将会显示规则菜单界面
#endif
    mo_standby,        //待机界面
    mo_gps_infor1,     //GPS全信息界面1
    mo_gps_infor2,     //GPS全信息界面2
    mo_recorder_state, //行驶记录仪状态界面
    mo_check_state,    //状态检测界面

    mo_print,        //打印界面
    mo_lcd_para_rst, //液晶相关参数初始化界面

    mo_message_window, //消息窗口界面

    mo_1st_5th_1st_password, //输入法操作界面 通信参数设置
    mo_1st_5th_4th_password, //输入法操作界面 油路控制
    mo_1st_5th_6th_password, //输入法操作界面 看车设置
    mo_1st_5th_8th_password, //输入法操作界面 液晶相关参数初始化
    mo_1st_6th_1st_password, //???????????????????????? ??????????????????????????

    mo_1st_1st_1st_password, //输入法操作界面 传感器系数设置
    mo_1st_1st_2nd_password, //输入法操作界面 速度模式

    mo_password_change, //密码修改界面

    mo_1st_5th_1st_1st, //主IP设置或备用IP设置
    mo_1st_5th_1st_3rd, //本机ID或短信服务号码设置
    mo_1st_5th_1st_4th, //APN设置

    mo_1st_5th_9th_1st, //注册项相关参数基本设置
    mo_1st_5th_9th_7th, //注册项相关参数车牌号码设置

    mo_1st_1st_1st_1st, //传感器参数设置
    mo_1st_1st_5th_1st, //驾驶员参数设置

    mo_host_para,   //主机通信参数界面
    mo_ic_card_dis, //IC卡信息显示界面
    //mo_slave_ver_check,   //从机版本号查询
    mo_csq_vol, //查询CSQ信号值

    mo_auto_check,   //自检信息
    mo_query_rec,    //行驶记录仪参数查询
    mo_query_reg,    //注册参数查询
    mo_query_driver, //驾驶员参数查询

    //mo_update_remete,     //远程升级界面
    //mo_update_tf_slave,   //TF卡升级从机
    mo_update_tf_main,  //TF卡升级主机
    mo_update_font_lib, //字库更新界面

    mo_oil_test,  //油量测试
    mo_telephone, //电话处理界面

    mo_sms_fix_input, //上传事件
    mo_sms_vod_input, //信息点播
    mo_sms_ask_input, //提问应答
    mo_sms_display,   //信息显示界面

    mo_phbk_display,       //电话本显示界面
    mo_expand_mult_usart,  //扩展多路串口usart0:菜单显示
    mo_auto_parameter_cfg, //自动参数配置 2017-12-6 Lu
    mo_read_rcd_data,      //U盘读取记录仪数据界面
    mo_auto_init,          //自动初始化参数 2018-3-13
    mo_timeout_driving,
    mo_speed_record,
    mo_id_cfg, //ID自动配置
    mo_phone,  //电话

} menu_other_enum;

typedef struct
{
    u16 menu_1st; //规则菜单第一级
    u16 menu_2nd; //规则菜单第二级
    u16 menu_3rd; //规则菜单第三级
    u16 menu_4th; //规则菜单第四级
    u16 menu_5th; //规则菜单第五级

    menu_other_enum menu_other; //不规则菜单列表

    bool menu_flag;    //菜单有更新  TRUE表示菜单有更新   FALSE表示菜单没更新
    bool next_regular; //进入菜单的下一级是否是规则菜单  TRUE表示为规则菜单   FALSE为不规则菜单
    u16 last_start;    //上一界面第一栏显示的菜单项位于菜单中的位置
} menu_cnt_struct;

typedef struct {
    // unsigned char buf_1ln[17]; //第一行显示内容缓冲区 多一个字节用于存放结束标志
    // unsigned char buf_2ln[17]; //第二行显示内容缓冲区 多一个字节用于存放结束标志
    // unsigned char buf_3ln[17]; //第三行显示内容缓冲区 多一个字节用于存放结束标志
    // unsigned char buf_4ln[17]; //第四行显示内容缓冲区 多一个字节用于存放结束标志

    unsigned char *p_dis_1ln; //第一行显示内容指针
    unsigned char *p_dis_2ln; //第二行显示内容指针
    unsigned char *p_dis_3ln; //第三行显示内容指针
    unsigned char *p_dis_4ln; //第四行显示内容指针

    unsigned char bg_1ln_pixel; //第一行显示起始位置，单位为一个像素点
    unsigned char bg_2ln_pixel; //第二行显示起始位置，单位为一个像素点
    unsigned char bg_3ln_pixel; //第三行显示起始位置，单位为一个像素点
    unsigned char bg_4ln_pixel; //第四行显示起始位置，单位为一个像素点
} message_window_dis_struct;    //信息提示窗口显示内容结构体

typedef struct
{
    bool dir_flag;  //退出后的菜单   TRUE表示退出到上一级规则菜单   FALSE表示待机界面
    bool auto_flag; //自动退出标志   TRUE表示自动退出       FALSE表示不自动退出

    u32 cnt;             //自动退出时间计时器  单位1毫秒
    u32 lgth;            //自动退出时间长度    单位1毫秒
} menu_auto_exit_struct; //自动退出

typedef enum {
    rpt_average = 0, //一般类型
    rpt_set,         //设置类型  如设置IP等
    rpt_com,         //命令类型  如查询参数等
    rpt_more,        //多命令类型  如查询注册项参数等
} report_type_enum;

typedef struct
{
    bool en_flag; //报告弹出界面使能标志
    bool rpt;     //报告类型	 TRUE报告成功和失败    FALSE仅报告失败

    report_type_enum dat_type; //发送的数据类型
    bool ok_flag;              //数据处理完成标志    TRUE报告成功    FALSE报告失败

    const u8 **ptr;   //显示报告内容的指针
    u32 cnt;          //报告弹出时间计时器  单位1毫秒
    u32 lgth;         //报告弹出时间长度    单位1毫秒
} menu_report_struct; //状态报告

typedef struct
{
    u8 time[3];          //时间数据，压缩BCD格式  参考相关说明文件
    u8 lat_data[4];      //纬度数据，压缩BCD格式  参考相关说明文件
    u8 long_data[4];     //经度数据，压缩BCD格式  参考相关说明文件
    u8 speed[2];         //速度数据，压缩BCD格式  参考相关说明文件
    u8 direction[2];     //方向数据，压缩BCD格式  参考相关说明文件
    u8 gps_state;        //GPS状态，最高位表示定位状态 第5位为0表示GPS速度为1表示传感器速度 低5位表示可视卫星数
    u8 gprs_alarm_state; //GPRS、报警状态，位含义参考相关说明文件
    u8 speed_max;        //超速限制值
    u8 reser[2];         //保留的两个字节
    u8 csq_state;        //CSQ状态  最高位为1表示SIM卡正常   0表示未插入或已损坏
    u8 acc_state;        //ACC状态，位含义参考相关说明文件
    u8 lock_car_state;   //锁车状态
    u8 date[3];          //日期时间数据，压缩BCD格式  参考相关说明文件
    u8 oil_data[2];      //油量数据，压缩BCD格式  参考相关说明文件
    u8 car_state;        //汽车状态，即行驶记录仪状态，位含义参考相关说明文件
} gps_data_struct;

typedef struct
{
    bool type;         //协议版本类型  FALSE为兼容老协议版本  TRUE为新的包含备用IP的版本
    u8 menu_dis_count; //菜单显示计数，用于显示状态时按键翻动

    u8 ip_lgth; //IP及PORT长度
    u8 ip[30];  //IP及PORT内容  格式为  (0"220.231.155.085",8888) 0,220.231.155.085,8888

    u8 apn_lgth; //APN长度
    u8 apn[30];  //APN内容

    u8 id_num[16];   //ID号码         //第零个元素   [0]存储长度
    u8 user_num[16]; //用户号码       //第零个元素   [0]存储长度
    u8 sms_num[16];  //短信服务号码   //第零个元素   [0]存储长度

    u8 ver_date[6]; //版本及日期     //型号、版本、年、月、日、时
    u8 speed;       //超速报警   //单位为公里
    u8 outage;      //断电报警   //该值非零表示报警
    u8 stop;        //停车报警   //单位为分

    u8 transmit_mode;    //发送模式  //0x80表示静默    0x81表示单点发送
    u8 transmit_time[2]; //发送间隔  //单位为秒

    u8 call_time[2];   //短话呼出时间    //单位为分
    u8 answer_time[2]; //电话接听时间    //单位为分

    u8 Backup_ip_lgth; //备用IP及PORT长度
    u8 Backup_ip[30];  //备用IP及PORT内容  格式为  (0"220.231.155.085",8888) 0,220.231.155.085,8888

    u8 ver_date2[25]; //版本及日期     //型号、版本、年、月、日、时

    u8 ip3_lgth; //IP及PORT长度
    u8 ip3[30];  //IP及PORT内容  格式为  (0"220.231.155.085",8888) 0,220.231.155.085,8888

} host_state_struct; //主机状态查询数据结构体

typedef struct
{
    u8 gprs_state;  //GPRS状态
    u8 gsm_state;   //GSM状态
    u8 gps_state;   //GPS状态
    u8 board_state; //主板状态
    u8 sim_state;   //SIM卡状态

    u8 hard_state;   //硬件检测状态
    u8 record_state; //记录仪状态
    u8 serial_state; //串口状态

    u8 speed[2];     //速度传感器值
    u8 oil[2];       //油量电阻值
    u8 power[2];     //电源电压值
} auto_check_struct; //自检数据包

typedef struct
{
    u8 step;
    u16 tim;

    u8 ratio[rec_para_ratio_max_lgth];  //传感器系数
    u8 speed_type;                      //速度类型
    u8 print_type;                      //打印类型
    u8 car_type[rec_car_type_max_lgth]; //号牌类型
} query_rec_struct;                     //行驶记录仪查询数据结构体

typedef struct
{
    u8 step;
    u16 tim;

    u8 province_id[rec_province_id_max_lgth];         //省域ID
    u8 city_id[rec_city_id_max_lgth];                 //市域ID
    u8 manufacturer_id[rec_manufacturer_id_max_lgth]; //制造商ID
    u8 terminal_type[rec_terminal_type_max_lgth];     //终端型号
    u8 terminal_id[rec_terminal_id_max_lgth];         //终端ID
    u8 car_plate_color;                               //车牌颜色
    u8 car_plate[rec_car_plate_max_lgth];             //车牌号码
    u8 car_vin[rec_car_vin_max_lgth];                 //车辆VIN码
} query_reg_struct;                                   //注册参数查询数据结构体

typedef struct
{
    u8 step;
    u16 tim;
} query_driver_struct; //驾驶员参数查询数据结构体

typedef union {
    host_state_struct host;       //主机状态
    auto_check_struct auto_check; //自检状态
    query_rec_struct rec;         //行驶记录仪
    query_reg_struct regist;      //注册
    query_driver_struct driver;   //驾驶员
} query_data_union;

//------------------------------------

typedef struct
{
    u8 ic_flag : 1;  //IC卡插入标志   TRUE表示已经插入  FLASH表示未插入
    u8 sd_flag : 1;  //SD卡插入标志   TRUE表示已经插入  FLASH表示未插入
    u8 usb_flag : 1; //U盘插入标志   TRUE表示已经插入  FLASH表示未插入

    u8 nread_sms : 1;  //未读信息标志   TRUE表示有未读  FLASH表示没有
    u8 nack_phone : 1; //未接电话标志   TRUE表示有未接  FLASH表示没有

    u8 up_flag : 1;   //上行标志   TRUE表示上行    FLASH表示没有
    u8 down_flag : 1; //下行标志   TRUE表示上行  FLASH表示没有

    u8 custom_flag : 1; //待机界面自定义标志
} state_flag_struct;

typedef union {
    state_flag_struct flag;
    u16 word_data;
} state_flag_union;

typedef struct
{
    state_flag_union state; //SD、IC卡等其他标志状态

    u8 custom_buff[18]; //自定义待机界面  结尾标志为0x00  因显示限制内容不得超过8个汉字
    u8 job_num[20];     //司机工号
} state_data_struct;

//------------------------------------

typedef struct
{
    u8 page_max; //页总数
    u8 sig_size; //单元大小  可以是一个字节显示ASCII，也可以是两个字节显示一个汉字

    u8 crisis;            //临界值  为0时表示整个输入法过程中无输入库的切换
    u8 front_crisis_size; //临界值前的单元大小

    const u8 **ptr_lib; //数据源
} input_lib_struct;     //输入法的库文件

typedef struct
{
    input_lib_struct input_lib; //输入法数据源(库文件)

    u8 page;  //输入法的页设置
    u8 point; //每页中的指针

    bool encrypt; //显示加密状态    FALSE表示不加密    TRUE表示加密
    u8 lgth;      //本次所输入的字符长度限制
    u8 cnt;       //本次所输入的字符计数

    const u8 *ptr_dis; //输入法待输入项描述内容
    u8 *ptr_dest;      //输入法最终将会写入到的目标地址

    u8 buff[60];       //本次所输入的字符缓冲区
} input_method_struct; //输入法操作结构体

//------------------------------------

typedef enum {
    call_free = 0, //空闲

    call_dial,    //拨号
    call_out,     //电话呼出
    call_in,      //电话呼入
    call_connect, //电话接通
    call_halt,    //电话挂断

    call_end, //通话结束
} phone_step_enum;

typedef struct
{
    phone_step_enum step;                //通话步骤
    u8 buff[telephone_num_max_lgth + 2]; //电话号码缓冲区,buff[0]为号码长度，buff[1]为0x00

    u8 dtmf_cnt;                          //输入DTMF计数器
    u8 dtmf_buff[telephone_num_max_lgth]; //输入DTMF缓冲区
    u8 dtmf_ptr;                          //显示DTMF位置

    u32 tim; //通话时间
} phone_data_struct;

typedef void (*dis_multi_page_input_point)(void); //多页面显示界面下扩展输入法界面配置函数指针

//*****************************************************************************
//*****************************************************************************
//-------------------           数据类型定义               --------------------
//-----------------------------------------------------------------------------

typedef enum {
    ic_none = 0,  //未插入卡片
    ic_error = 1, //卡片错误或不能识别或数据内容不符合规定

    ic_2402 = 2, //24些列卡片
    ic_4442 = 3, //4442系列卡片
} ic_type_enum;  //IC卡类型

typedef struct
{
    //--------协议预留部分------//
    u8 name[12];    //驾驶员姓名   汉字国标码字符ASCII码格式  //不用或不足时填入0x00
    u8 id_card[20]; //驾驶员身份证号   ASCII格式   //不用或不足时填入0x00
    //--------------------------//

    //--------协议定义部分------//
    u8 driv_lic[18];         //驾驶证号码   ASCII格式  //不用或不足时填入0x00
    u8 driv_lic_validity[3]; //驾驶证有效期  BCD格式    -年-月-日
    u8 certificate[18];      //从业资格证号  ASCII格式  //不用或不足时填入0x00
    //--------------------------//

    //--------协议标准扩展预留部分------//
    u8 license_organ[40];         //发证机构名称   汉字国标码字符ASCII码格式  //不用或不足时填入0x00
    u8 license_organ_validity[4]; //证件有效期   BCD格式    --年-月-日
    u8 work_num[6];               //工号   ASCII格式  //不用或不足时填入0x00

    u8 reserve[6]; //不用或不足时填入0x00
    //--------------------------//

    u8 xor ;                  //前面字节的异或校验值
} ic_card_save_format_struct; //IC卡芯片存储数据格式定义

typedef struct
{
    u8 work_num_lgth; //工号    //暂定最大长度不超过7字节  CKP  2014-06-18
    u8 work_num[12];

    u8 name_lgth; //姓名
    u8 name[12];

    u8 id_card_lgth; //身份证号
    u8 id_card[20];

    u8 driv_lic_lgth; //驾驶证
    u8 driv_lic[18];
    u8 driv_lic_validity[3]; //驾驶证有效期限

    u8 certificate_lgth; //从业资格证
    u8 certificate[18];

    u8 license_organ_lgth; //从业资格证发证机构
    u8 license_organ[40];
    u8 license_organ_validity[4]; //从业资格证证件有效期
} ic_card_date_struct;            //IC卡数据格式

#define call_record_num 30 //通话记录里每一项的记录数量

typedef enum {
    call_out_type = 0,  //呼出记录
    received_call_type, //已接电话
    missed_call_type,   //未接电话
} call_record_type_enum;

/*

typedef struct
{
  u8 call_record_type;              //电话类型
  u8 call_record_name_lgth;
  u8 call_record_name_buff[phone_book_name_max_lgth];
  u8 call_record_num_lgth;
  u8 call_record_num_buff[phone_book_num_max_lgth];
}
call_record_sub_struct;

*/

#define phone_book_num_max_lgth ((u8)16)  //电话本内容电话号码长度限制
#define phone_book_name_max_lgth ((u8)16) //电话本内容姓名长度限制

typedef enum {
    phbk_type_in_only = 0, //仅可呼入
    phbk_type_out_only,    //仅可呼出
    phbk_type_in_out,      //可呼入呼出
} phbk_type_enum;          //电话本类型定义

typedef struct
{
    u8 lgth;                          //电话号码长度
    u8 buff[phone_book_num_max_lgth]; //电话号码内容
} phbk_num_struct;                    //号码结构体

typedef struct
{
    u8 lgth;                           //姓名内容长度
    u8 buff[phone_book_name_max_lgth]; //姓名内容
} phbk_name_struct;                    //名字结构体

typedef struct
{
    phbk_type_enum type; //电话类型

    phbk_num_struct num;
    phbk_name_struct name;
} phbk_data_struct; //电话本内容

//呼出记录，已接电话，未接电话公用此结构体
typedef struct
{
    u8 call_record_cnt; //通话记录中的电话数
    phbk_data_struct call_record_buff[call_record_num];
} call_record_struct;

//*****************************************************************************
//*****************************************************************************

#define phone_book_total_max ((u16)1000)               //电话本总条数上限
#define phone_book_index_content_lgth ((u16)16 * 1024) //索引总长度上限

#define phone_book_index_short_name_max_lgth ((u8)6) //目录最长显示的姓名数

typedef struct
{
    u16 total;       //存储总数
    u16 posi;        //下一条数据将要存储的位置,此时电话本里面的电话数量的技术
} phbk_count_struct; //电话本计数值

typedef u16 phbk_index[phone_book_total_max]; //索引数据结构

#define sms_total_max ((u8)80)             //每种信息总条数上限
#define sms_index_content_lgth ((u16)2048) //索引总长度上限

#define sms_center_single_piece_lgth ((u16)1024) //单条中心调度(文本)信息总长度上限
#define sms_serve_single_piece_lgth ((u16)512)   //单条服务信息总长度上限
#define sms_fix_single_piece_lgth ((u16)512)     //单条固定(事件)信息总长度上限
#define sms_phone_single_piece_lgth ((u16)1024)  //单条手机信息总长度上限
#define sms_vod_single_piece_lgth ((u16)512)     //单条点播信息总长度上限

#define sms_ask_content_lgth ((u8)200)        //提问内容总长度上限
#define sms_ask_answer_total_max ((u8)8)      //提问答案总数上限
#define sms_ask_answer_content_lgth ((u8)100) //提问答案总长度上限

typedef enum {
    sms_type_fix = 0, //固定(事件)信息
    sms_type_ask,     //提问信息
    sms_type_vod,     //点播信息
    sms_type_serve,   //服务信息
    sms_type_center,  //中心调度(文本)信息
    sms_type_phone,   //手机信息
} sms_type_enum;      //信息类型定义

typedef enum {
    sms_op_add = 0, //增加信息
    sms_op_del,     //删除信息
    sms_op_change,  //修改信息
} sms_op_enum;      //信息操作类型定义

typedef struct
{
    u16 lgth;
    u8 dat[sms_center_single_piece_lgth];
} sms_center_content_struct; //文本信息

typedef struct
{
    u16 lgth;
    u8 dat[sms_serve_single_piece_lgth];
} sms_serve_content_struct; //服务信息

typedef struct
{
    u16 lgth;
    u8 dat[sms_fix_single_piece_lgth];
} sms_fix_content_struct; //事件信息

typedef struct
{
    u16 lgth;
    u8 dat[sms_phone_single_piece_lgth];
} sms_phone_content_struct; //手机信息

typedef struct
{
    u8 dat[70];
    u16 lgth;
} sms_phone_struct; //手机信息

typedef struct
{
    u16 lgth;
    u8 dat[sms_vod_single_piece_lgth];
} sms_vod_content_struct; //点播信息

typedef struct
{
    u8 ans_cnt;                   //问题答案的总个数
    u16 lgth;                     //问题长度
    u8 dat[sms_ask_content_lgth]; //问题内容

    struct
    {
        u8 ans_id;                               //答案ID
        u16 ans_lgth;                            //答案长度
        u8 ans_dat[sms_ask_answer_content_lgth]; //答案内容
    } answer[sms_ask_answer_total_max];
} sms_ask_content_struct; //提问应答

typedef struct
{
    bool read_flag;         //阅读标志  TRUE表示未读    FALSE表示已读
    u16 type_id_serial_num; //存储属性、ID号、类型等信息
    u32 addr;               //信息内容的地址    前两个字节放短信长度，从第2个字节是短信内容
} use_infor_struct;         //使用信息

typedef struct
{
    u16 total; //已存储的总条数

    use_infor_struct use_infor[sms_total_max]; //已使用地址及信息表

} sms_index_struct; //信息索引结构体，所有的信息类型皆用该结构体文件

typedef union {
    sms_center_content_struct center;
    sms_serve_content_struct serve;
    sms_fix_content_struct fix;
    sms_phone_content_struct phone;
    sms_vod_content_struct vod;
    sms_ask_content_struct ask;
} sms_union_data_union; //信息内容联合体

typedef struct
{
    sms_type_enum sms_type; //当前操作的信息类型

    u8 fix_id;     //事件ID号
    u8 fix_status; //事件输入状态

    u16 ask_num;                          //提问应答流水号
    u8 ask_id_point;                      //提问应答答案指针
    u8 ask_ans[sms_ask_answer_total_max]; //提问答案ID表

    u8 vod_type;        //信息点播信息类型
    u8 vod_status;      //信息点播输入状态
} sms_up_center_struct; //上传给中心的信息参数数据

/*
typedef struct
{
	u16 rec_len;
	u16 send_len;
	u8  rec_buff[HANDSET_MAX_SIZE];    //接收数据缓冲器
	u8  send_buff[HANDSET_MAX_SIZE];   //待发送数据缓冲器
}
spi_data_struct;       //SPI接口数据结构体
*/

typedef struct
{
    u16 len;
    u16 res;
    u8 buf[HANDSET_MAX_SIZE]; //接收数据缓冲器
} SLAVE_RECIVE_DATA_STRUCT;

typedef struct
{
    u16 len;
    u16 res;
    u8 buf[HANDSET_MAX_SIZE]; //发送数据缓冲器
} SLAVE_SEND_DATA_STRUCT;

extern SLAVE_RECIVE_DATA_STRUCT slv_rev_data;
extern SLAVE_SEND_DATA_STRUCT slv_send_data;

typedef struct
{
    u32 comm; //发送或应答命令号

    u8 sla_comm;  //发送或应答子信令
    u8 status;    //发送或应答当前所处的状态
} spi_ack_struct; //从机准备发送给主机的数据或者应答

//*****************************************************************************
//*****************************************************************************
//-----------		            变量定义   	                  -----------------
//-----------------------------------------------------------------------------

//extern     gbk_unicode_buff_struct  gbk_unicode_buff;  //国标码与UNICODE码转换

extern user_time_struct user_time; //用户时间计数
extern u16 wheel_speed;            //车轮速度
extern u16 engine_speed;           //发动机转速
extern u16 speed_jiffes;
extern u16 average_1km;

extern mult_usart_set_struct mult_usart_set; //设置选定的多路串口

//extern     spi1_up_data_struct  spi1_up_queue;  //SPI1上行命令队列
//extern     spi_send_struct      spi1_send;      //SPI接口发送

//extern     usart_send_struct  usart1_send;  //串口1发送
//extern     usart_send_struct  usart2_send;  //串口2发送
//extern     usart_rec_struct   usart1_rec;      //串口1接收
//extern     usart_rec_struct   usart2_rec;      //串口2接收
//extern     u8  usart1_send_buff[usart_send_buff_max_lgth];  //串口1发送数据缓冲器
//extern     u8  usart2_send_buff[usart_send_buff_max_lgth];  //串口2发送数据缓冲器

extern query_data_union query;                     //查询类结构
extern gps_data_struct gps_data;                   //GPS相关数据
extern state_data_struct state_data;               //状态数据
extern set_para_struct set_para;                   //设置主机参数
extern recorder_para_struct recorder_para;         //行驶记录仪参数
extern host_no_save_para_struct host_no_save_para; //不需要保存的设置参数

extern key_data_struct key_data;             //按键操作
extern menu_cnt_struct menu_cnt;             //菜单结构体
extern message_window_dis_struct mw_dis;     //消息显示结构体
extern menu_auto_exit_struct menu_auto_exit; //菜单自动退出结构体
extern menu_report_struct menu_report;       //报告界面
extern u16 dis_multi_page_cnt;               //多页显示界面中已经显示的内容

extern input_method_struct input_method;       //输入法
extern password_manage_struct password_manage; //密码管理

extern ic_type_enum ic_type;        //IC卡类型
extern ic_card_date_struct ic_card; //IC卡数据

extern buzzer_voice_struct buz_voice;
extern voice_source_struct buz_cus[];
extern buzzer_cus_time_struct buz_cus_time;

extern call_record_struct call_record; //通话记录相关数据
extern phone_data_struct phone_data;   //电话相关数据

extern sms_up_center_struct sms_up_center;               //上传给中心的信息参数数据
extern sms_union_data_union sms_union_data;              //信息联合体
extern u8 sound_play_buff[sound_play_buff_max_lgth + 2]; //语音朗读缓冲区  前两个字节表示长度，短整形数据类型

//扩展多路串口功能变量定义
extern u8 uart_set_ok_flag[3];         //多路串口扩展里面的串口设置成功标志位，=1表示设置成功；=0表示没有设置或者设置失败,各个串口设置后的‘*’显示
extern u8 extend_uart_cnt;             //多路串口扩展里面的可扩展串口的个数
extern u8 ic_driver_ifo_num;           //从U盘读取到的驾驶员信息数目 2018-1-6
extern u32 extend_uart_buff[9];        //多路串口扩展接收缓存
extern u8 tmp_cnt;                     //多路串口扩展里面的uartx可选功能的个数
extern u8 extend_uart_send_bit[7];     //用于记录多路串口扩展里面的uartx中的各个选项的位标志，比如extend_uart_send_bit[0]记录的是第1个显示项是第几位是1，该元素里面就填几
extern u8 current_uart_flag[3];        //多路串口扩展里面的当前选中串口的‘*’选择标志位,各个串口查询时的‘*’显示
extern u8 tmp_bit_cnt;                 //用于记录多路串口扩展里面的uartx中的各个选项的位标志，指向extend_uart_send_bit
extern unsigned char *tmp_uart_dis[3]; //临时串口的显示，主要用来存放显示多路串口扩展里面的USART1-3的显示

// TF卡存储图片时定义的变量信息
//extern   u8 time[50];                  //存放存储图片时候的GPS时间
//extern  u8 full_filename[50];          //存储图片时的文件名

//通话记录所需变量定义
extern phbk_data_struct phbk_call_in_data;     //存储当前来电与电话薄对比后，从电话簿里面提取出来的相应的电话类型，姓名，号码信息
extern print_auto_data_struct print_auto_data; //自动打印数据

//域名查询所需变量定义
extern u8 realm_name_flag; //域名标志位，主域名:  0x01   ;备用域名:0x02
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//extern spi_data_struct      spi1_data;      //SPI接口数据交互
extern spi_ack_struct slv_send_flag;

//extern  bool Clos_Soc_flag ;	//请求soc关闭标志

extern u8 get_monsize(u8 year, u8 month);
extern s16 rtime_cmp(void *time1, void *time2);
extern void update_timer_task(void);
extern void delay10us(void);
extern void delay50us(void);
extern void delay_us(u8 lgth);
extern void delay_10us(u16 lgth);
extern void delay1ms(void);
extern void delay20ms(void);
extern void delay50ms(void);
extern void delay_ms(u8 i);
extern void delayms(u32 d_time);

extern void updata_link_mx(void);

extern void uart_manage_task(void);

//extern void init_led(void);
extern void init_car_out(void);

extern void adc_power_task(void);
extern void modify_beijing_time_area(time_t *t);
extern void update_mix_infomation(void);

extern void power_manage_task(void);
extern void calculation_sum_mile(void);
extern void link_manage_task(void);
//extern void led_manage_task(void);

extern void ctrl_relay_status(u8 stat);

extern void cut_oil_ctrl_task(void);

extern void Ic_Relay_One_Control(u8 flag);
extern bool verify_server_cfg(IP_SERVER_CONFIG_STRUCT *t1);

// extern void acc_timing_led(void);
extern void public_manage_task(void);
// extern void print_TR9(u8 *buf, u16 len);
// extern void print_can(u8 *buf, u16 len);

extern void Monitor_Hi3520_Status(void);
extern void Open_Pwr_All(void);

#endif //__DRV_PUB_H_
