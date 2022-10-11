/**
  ******************************************************************************
  * @file    jt808_type.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   JT808数据类型定义
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef __JTT808TYPE_
#define __JTT808TYPE_

#include "menu_process_enum.h"

#define PHONELEN 16

#define A_NO 0x00000000
#define A_EMERY 0x00000001
#define A_SPEEDING 0x00000002
#define A_FATIGUE 0x00000004
#define A_SPEEDNEAR 0x00000008
#define A_GPSERR 0x00000010
#define A_ANTEOPEN 0x00000020
#define A_ANTESHORT 0x00000040
#define A_VOLTLOW 0x00000080

#define A_VOLTOFF 0x00000100
#define A_lcd_err 0x00000200
#define A_TTS 0x00000400
#define A_CAMERA 0x00000800
#define A_OIL_ERR 0x00001000

#define A_DAYOVERTM 0x00040000
#define A_STOPOVERTM 0x00080000

#define A_AREAINOUT 0x00100000
#define A_ROADINOUT 0x00200000
#define A_ROADOVER 0x00400000
#define A_ROADLEAVE 0x00800000

#define A_VEHIVSS 0x01000000
#define A_OILALARM 0x02000000
#define A_VEHILOSE 0x04000000
#define A_ERRACCON 0x08000000
#define A_CARERRMOVE 0x10000000
#define A_COLLIDE 0x20000000

#define PK_RPT 0x0001 //package  report  , 有重发机制
#define PK_NET 0x0002 //package  net, 有重发机制
#define PK_HEX 0x0004 //package hex, 透传，底层无重发
#define PK_SUB 0x0008 //package subpackage  分包
#define PK_ENC 0x0008 //package encryption  加密

typedef struct {
    u8 year;
    u8 month;
    u8 date;
    u8 hour;
    u8 min;
    u8 sec;
} time_t;

typedef struct {
    s32 lati; // Latitude (in degree * 10000000)
    s32 lngi; // Longitude(in degree * 10000000)
} point_t;

/*lint -e46 */
/*lint -e768 
25	0：车辆未锁定，1：车辆锁定；
26	0：车辆未限速，1：车辆限速；
27	0：不限制举升，1：限制举升；
28	0：车厢空载，1：车厢重载；
29	0：车厢密闭，1：车厢打开；
30	0：车厢平放，1：车厢举升；
31	0：非维护模式，1：维护模式；
*/
typedef union {
    u32 reg;
    struct
    {
        bool acc : 1;    //ACC
        bool fix : 1;    //卫星定位锁定
        bool lat : 1;    //纬度  1 南纬
        bool lng : 1;    //经度  1 西精
        bool engine : 1; //发动机 1: ON  0:Off
        bool res1 : 1;   //经纬度加密插件
        bool res2 : 1;
        bool res3 : 1;

        u32 carriage : 2;   //载重状态
        bool cut_oil : 1;   //断油正常/断开
        bool cut_pwr : 1;   //电路正常/断开
        bool lock_door : 1; //车门加锁/车门解锁
        u32 door_front : 1; //前门状态1
        u32 door_2 : 1;     //中门状态2
        u32 door_back : 1;  //后门状态3

        u32 door_4 : 1;       //门状态4
        u32 door_5 : 1;       //门状态5
        u32 fix_gps : 1;      //使用GPS定位
        u32 fix_bd : 1;       //使用北斗定位
        u32 fix_glonass : 1;  //使用glonass定位
        u32 fix_calileo : 1;  //使用calileo定位
        bool left_cover : 1;  //0：左盖密闭；1：左盖打开
        bool right_cover : 1; //预留

        bool all_flag : 1;    //预留
        bool lock_car_sw : 1; //油量异常报警
        bool speeding : 1;    //车辆被盗
        bool car_rise : 1;    //0：不限制举升，1：限制举升；
        bool loading : 1;     //0：车厢空载，1：车厢重载；
        bool car_cover : 1;   //0：车厢密闭，1：车厢打开；
        bool car_box : 1;     //0：车厢平放，1：车厢举升；
        bool right_turn : 1;  //预留
    } bit;
} UNION_VEHICLE_STATE;

/*lint -e46 */
/*lint -e768 */
typedef union {
    u32 reg;
    struct
    {                        //！！！不要随意更改这里的顺序，原因是同人，相关配置，开启/关闭功能，跟这个顺序有关。！！！
        bool sos : 1;        //0	1：紧急报警，触动报警开关后触发
        bool speed_over : 1; //超速
        bool fatigue : 1;    //疲劳驾驶报警
        bool speed_near : 1; //超速预警
        bool gps_err : 1;    //GPS模块故障
        bool ant_open : 1;   //5	1：GNSS 天线未接或被剪断报警
        bool ant_short : 1;  //6	1：GNSS 天线短路报警
        bool low_volt : 1;   //7	1：终端主电源欠压报警

        bool cut_volt : 1;     //8	1：终端主电源掉电报警
        bool lcd_err : 1;      //故障报警
        bool tts_err : 1;      //故障报警
        bool camera_err : 1;   //故障报警
        bool oil_err : 1;      //故障报警
        bool gsm_err : 1;      //故障报警
        bool fatigue_near : 1; //res2					   黑龙江的提示
        bool res3 : 1;

        bool res4 : 1;        //16位-
        bool res5 : 1;        //
        bool drive_over : 1;  //驾驶超时报警
        bool stop_over : 1;   //停车报警
        bool area_in_out : 1; //区域进出报警
        bool road_in_out : 1; //路线进出报警
        bool road_over : 1;   //路线偏离报警
        bool road_leave : 1;  //路线离开报警

        bool vss_err : 1;          //VSS故障
        bool oil_alm : 1;          //油量异常报警
        bool vehi_lose : 1;        //车辆被盗
        bool acc_on_vehi_err : 1;  //车辆非法点火
        bool move_vehi_err : 1;    //车辆非法位移
        bool collision : 1;        //碰撞报警
        bool turn_on_one_side : 1; //侧翻报警
        bool arming_door : 1;      //非法开门报警
    } bit;
} VEHICLE_ALARM_UNION;

#if (P_RCD == RCD_BASE)
typedef union {
    u32 reg;
    struct {
        u32 near_light : 1;  //近光灯
        u32 far_light : 1;   //远光灯
        u32 right_light : 1; //右转
        u32 left_light : 1;  //左转
        u32 brake : 1;       //刹车
        u32 res05 : 1;       //雾灯
        u32 res06 : 1;
        u32 res07 : 1;

        u32 beep : 1; //喇叭
        u32 res10 : 7;
        u32 res20 : 8;
        u32 res30 : 8;
    } b;
} U_BD_RCD_STATUS;
#endif

#if defined(JTT_808_2019) //jt808——2019

typedef struct
{
    u16 crc;
    u16 flag;
    u16 province;         //省ID
    u16 city;             //市ID
    u8 mfg_id[11];        //制造商ID
    u8 terminal_type[30]; //产品型号
    u8 terminal_id[30];   //终端ID
    u8 color;             //车牌颜色
    u8 license[10];       //车牌标识    不用废掉
    u8 res1[1];           //sLoginCode[21]; //鉴权码
    u8 weight_auto_close; //=0x55 重车自动关闭
    u8 bCarLogin;         //鉴权成功标志位

    u8 gps_no_star; //区域内
    u16 blank_car_weight;
    u8 all_open_switch; ///0x55 -- 开； 其余关闭
    u8 ztb_func;        //=0x55 关闭渣土办功能；  其他打开渣土办功能
    u8 ztb_gk;          //渣土办管控
    u8 ill_sw;          //非法举斗开关  =0x55 非法打开
    u8 lift_weight;     //随斗重车
    u8 all_polar;       //全举极性
    u8 raise_polar;     //全举极性
    u8 left_polar;
    u8 right_polar;
    u8 speed;    //速度比列系数
    u8 can_type; //0x00 - 默认标准版本；01-康明斯协议
    u8 acc_simulate;
} REGISTER_CONFIG_STRUCT; //注册信息

/*lint -e46 */
/*lint -e768 */

typedef union {
    u16 property; //消息体属性
    struct
    {
        u16 mess_len : 10; //消息体长度 最大1024字节
        u8 encry_type : 3; //加密类型
        u8 if_pack : 1;    //=1 长消息  =0 短消息
        u8 version : 1;    //版本标识
        u8 res : 1;
    } bit;
} NEWS_HEAD_PROPERTY_UNION; //消息头结构体

#pragma pack(1)
typedef struct
{
    u16 id;                       //消息ID 2字节
    NEWS_HEAD_PROPERTY_UNION pro; //消息头属性
    u8 version;                   //2019版添加版本号
    u8 number[10];                //手机号 不足12位前头补零
    u16 run_no;                   //消息流水号
    u16 sub_tatal;                //消息包总数
    u16 sub_no;                   //消息包序号
} SEND_MESS_INFOMATION_STRUCT;    //消息头内容
#pragma pack()

#else /**************jt808——2013********************/

typedef struct
{
    u16 crc;
    u16 flag;
    u16 province;         //省ID
    u16 city;             //市ID
    u8 mfg_id[5];         //制造商ID
    u8 terminal_type[20]; //产品型号
    u8 terminal_id[7];    //终端ID
    u8 color;             //车牌颜色
    u8 license[10];       //车牌标识    不用废掉
    u8 res1[1];           //sLoginCode[21]; //鉴权码
    u8 weight_auto_close; //=0x55 重车自动关闭
    u8 bCarLogin;         //鉴权成功标志位

    u8 gps_no_star; //区域内
    u16 blank_car_weight;
    u8 all_open_switch; ///0x55 -- 开； 其余关闭
    u8 ztb_func;        //=0x55 关闭渣土办功能；  其他打开渣土办功能
    u8 ztb_gk;          //渣土办管控
    u8 ill_sw;          //非法举斗开关  =0x55 非法打开
    u8 lift_weight;     //随斗重车
    u8 all_polar;       //全举极性
    u8 raise_polar;     //全举极性
    u8 left_polar;
    u8 right_polar;
    u8 speed;    //速度比列系数
    u8 can_type; //0x00 - 默认标准版本；01-康明斯协议
    u8 acc_simulate;
} REGISTER_CONFIG_STRUCT; //注册信息

typedef union {
    u16 property; //消息体属性
    struct
    {
        u16 mess_len : 10; //消息体长度 最大1024字节
        u8 encry_type : 3; //加密类型
        u8 if_pack : 1;    //=1 长消息  =0 短消息
        u8 res : 2;
    } bit;
} NEWS_HEAD_PROPERTY_UNION; //消息头结构体

typedef struct
{
    u16 id;                       //消息ID 2字节
    NEWS_HEAD_PROPERTY_UNION pro; //消息头属性
    u8 number[6];                 //手机号 不足12位前头补零
    u16 run_no;                   //消息流水号
    u16 sub_tatal;                //消息包总数
    u16 sub_no;                   //消息包序号
} SEND_MESS_INFOMATION_STRUCT;    //消息头内容

#endif

typedef struct
{
    u16 crc;
    u16 flag;
    u32 hear_beat_time;    //心跳间隔
    u32 tcp_over_time;     //TCP消息应答超时时间
    u32 tcp_resend_cnt;    //TCP消息重传次数
    u32 udp_over_time;     //UDP消息应答超时时间
    u32 udp_resend_cnt;    //UDP消息重传次数
    u32 sms_over_time;     //SMS消息应答超时时间
    u32 sms_resend_cnt;    //SMS消息重传次数
} LINK_KEEP_CONFIG_STRUCT; //链路保持参数

typedef enum {
    SERVER_NOMAL,
    SERVER_UPDATE,
    SERVER_COAL //锟斤拷锟缴碉拷锟斤拷平台锟斤拷锟睫回革拷锟斤拷锟斤拷
} SERVER_TYPE;

typedef enum {
    NET_LOST,
    NET_CONNECT,
    NET_ONLINE,
    NET_SEND
} GPRS_STATE;

typedef struct
{
    u16 crc;
    u16 flag;
    u8 terminal_id[21]; //本机号码  终端ID号
    u8 select_tcp;      //0: 主服务器UDP  1:主服务器TCP    xx1位 主服务器 x1x位 备用服务器  1xx位升级服务器
    u8 apn[24];         //APN
    u8 sim_user[10];    //用户名
    u8 sim_pwd[8];      //密码
    u8 main_ip[24];     //远程IP地址
    u8 bak_ip[24];      //备份远程IP地址
    u8 ip3[24];         // 第3路IP
    u8 fix_res1[24];    //u8BApn[24]
    u8 fix_res2[10];    //sim_user[10]
    u8 fix_res3[8];     //sim_pwd[8]
    u8 ota_ip[24];      //升级远程IP地址
    u32 main_port;      //端口号
    u32 bak_port;       //UDP端口 -- 备用服务器端口
    u32 port3;
    u32 ota_port; //升级端口
    u32 tt_baud;  //透传口波特率
    u8 dbip;      //保留20140412 doubleip	 0x5a: 开启 其余:关闭
    u8 res[16];
} IP_SERVER_CONFIG_STRUCT; //服务器配置

typedef struct
{
    u16 crc;
    u16 flag;
    u16 heartbeat;   //心跳频率 默认时间
    u32 sample_mode; //采样模式 0x00 定时采样 0x01 定距采样 0x02 定时定距汇报
    u32 acc_mode;    //0: 根据ACC状态   1: 根据登录再根据ACC
    u32 no_dvr_time; //无驾驶员汇报时间间隔
    u32 sleep_time;  //休眠汇报时间间隔
    u32 sos_time;    //紧急汇报时间间隔
    u32 dft_time;    //缺省汇报时间间隔
    u32 dft_dst;     //缺省距离汇报间隔
    u32 no_dvr_dst;  //无驾驶员汇报距离间隔
    u32 sleep_dst;   //休眠汇报距离
    u32 sos_dst;     //紧急报警距离
    u32 angle_dgree; //拐点补传角度
    u8 res[12];
} REPORT_CONFIG_STRUCT; //汇报设置

typedef struct
{
    u16 crc;
    u16 flag;
    u8 listen[PHONELEN];    //监控平台电话号码
    u8 reset[PHONELEN];     //复位电话号码
    u8 factory[PHONELEN];   //恢复出厂设置电话号码
    u8 sms[PHONELEN];       //监管平台SMS文本报警号码	SmsAlaPhone
    u8 alarm_sms[PHONELEN]; //终端接收SMS文本报警号码
    u32 ring_type;          //接通模式
    u32 each_time;          //每次通话时间
    u32 month_time;         //当月累计通话时间
    u8 user[PHONELEN];      //监控平台电话号码 1
    u8 center[PHONELEN];    //监管平台特权短信号码
    u8 res[12];
} sCommunicationSet; //通话监控设置

typedef struct {
    u16 crc;
    u16 flag;
    VEHICLE_ALARM_UNION bypass_sw;   //报警功能屏蔽 1:屏蔽0:正常
    VEHICLE_ALARM_UNION sms_sw;      //报警发送SMS
    VEHICLE_ALARM_UNION pic_sw;      //相应报警拍照打开
    VEHICLE_ALARM_UNION pic_save_sw; //报警拍摄时是否需要上传
    u32 import_alarm_sw;             //关键报警
    u32 speed_over_km;               //最高速度
    u32 speed_over_time;             //持续超速时间，当持续超速时间超过当前值则产生超速报警
    u32 fatigure_time;               //疲劳驾驶时间
    u32 day_time;                    //当天累加时间
    u32 stop_rest_time;              //停车休息最小时间
    u16 stoping_time;                //停车报警时间间隔
    u16 pre_speed_over;              //预超速
    u16 pre_fatigue;                 //预疲劳
    u16 collision;                   //碰撞
    u16 turn_on_one_side;            //侧翻
    u16 elc_geo;                     //电子围栏
    u8 res[12];
} ALARM_CONFIG_STRUCT; //报警控制类

typedef struct {
    u16 crc;
    u16 flag;
    u8 call_mode;                       //用户设置通话模式 0x00 手柄通话 0x01 免提通话
    u8 call_set;                        //电话限制设置	通话权限 0：无限制； 0x80：限拨出；  0x40：限拨入 0xC0: 限制拨入拨出
    enum_speed_mode_typedef speed_mode; //220828速度类型://速度模式：传感器速度、GPS速度、CAN速度
                                        //RST-RK:系统看护
    s8 foster_care;                     //系统看护//不重启、1分钟、2分钟、3分钟、5分钟、8分钟、12分钟、17分钟、23分钟、30分钟

    u32 ulUart5Baud; //串口5配置的波特率  9600/19200
    u8 highspeed0;   //最高限速
    u8 highspeed1;   //违规限速
    u8 lowspeed;     //重车、未密闭
    u8 mobile;

    u8 res[10];
} SYSTERM_CONFIG_STRUCT;

typedef struct {
    //220828速度类型:速度模式
    struct {
        enum_speed_mode_typedef min;
        enum_speed_mode_typedef max;
    } speed_mode; //系统看护////速度模式：传感器速度、GPS速度、CAN速度

    //RST-RK:系统看护
    struct {
        u8 min;
        u8 max;
    } foster_care; //系统看护//不重启、1分钟、2分钟、3分钟、5分钟、8分钟、12分钟、17分钟、23分钟、30分钟
} SYSTERM_CONFIG_STRUCT_RANGE;

typedef struct {
    u16 interval; //临时跟踪上报时间间隔
    u32 time;     //临时控制时间
    u8 open;      //打开标志
} TMP_TRACE_CONFIG_STRUCT;

#define DRIVER_CONFIG_STRUCT_NAME_MAX 23
#define DRIVER_CONFIG_STRUCT_BIAN_HAO_MAX 7
typedef struct {
    u16 crc;
    u16 flag;
    u8 name_len;                                           //驾驶员姓名长度
    u8 name[DRIVER_CONFIG_STRUCT_NAME_MAX + 1];            //驾驶员姓名
    u8 license[20];                                        //驾驶员身份证号码
    u8 qualification[40];                                  //从业资格证号、驾驶证号
    u8 organization_len;                                   //长度
    u8 organization[30];                                   //公司名
    u8 driver_code[DRIVER_CONFIG_STRUCT_BIAN_HAO_MAX + 1]; //驾驶员编码
    u8 date[4];                                            //有效期
    u8 res[8];
} DRIVER_CONFIG_STRUCT;

typedef struct {
    struct {
        u8 min;
        u8 max;
    } name;

    struct {
        u8 min;
        u8 max;
    } bian_hao;
} DRIVER_CONFIG_STRUCT_RANGE;

typedef struct
{
    u16 crc;
    u16 flag;
    u8 vin[18];        //VIN码
    u8 license[12];    //车牌号
    u8 type[16];       //车牌分类
    u8 coefficient[3]; //车辆特征系数
    u8 highspeed;      //最高限速值
} CAR_CONFIG_STRUCT;

typedef struct {
    u16 crc;
    u16 flag;
    u8 buf[128];
    u8 res[12];
} flash_reserve;

#if (P_RCD == RCD_BASE)
typedef struct
{
    u16 crc;
    u16 flag;
    time_t setting_time; //设置时间
    u16 set_speed_plus;  //设置脉冲速度
    u32 set_mileage;     //设置里程
    u8 print_type;       //打印格式 0x01 2003  ,0x02 2012
    u8 speed_type;       //速度模式
    bool speed_cala;     //速度标定
    u8 swit;             //脉冲速度开关
} FACTORY_PARAMETER_STRUCT;

typedef union {
    u8 reg;
    struct
    {
        u8 s0 : 1;
        u8 s1 : 1;
        u8 s2 : 1;
        u8 s3 : 1;
        u8 s4 : 1;
        u8 s5 : 1;
        u8 s6 : 1;
        u8 s7 : 1;
    } b;
} signal_define_union;

typedef struct
{
    u16 crc;
    u16 flag;
    u8 b0[10];
    u8 b1[10];
    u8 b2[10];
    u8 b3[10];
    u8 b4[10];
    u8 b5[10];
    u8 b6[10];
    u8 b7[10];
} SIGNAL_CONFIG_STRUCT; //车辆系数
#endif

typedef struct
{
    u16 crc;
    u16 flag;
    u8 ccc[7];    // 3c认证代码
    u8 model[16]; //认证产品型号
    u8 time[6];   //生产年月日
    u8 sn[4];     //BCD格式
    u32 res;      //
} UNIQUE_ID_STRUCT;

typedef struct
{
    u16 crc;
    u16 flag;
    u8 type;      //上线鉴权码识别 = 0xee 鉴权成功
    u8 code[50];  //鉴权码
    u8 type2;     //上线鉴权码识别 = 0xee 鉴权成功
    u8 code2[50]; //鉴权码
    u8 type3;     //上线鉴权码识别 = 0xee 鉴权成功
    u8 code3[21]; //鉴权码
} authority_configuration_struct;

//客户编码ID 识别程序、状态发生变化时写入
typedef struct
{
    u16 crc;
    u16 flag;
    u8 date;            //日期
    u16 lock;           //=0x55 锁定、默认关闭
    u8 customer_id[11]; //客户编码；默认TRWY
    u8 iGps3D2D;        //0x88 需要过滤2D 信号
    u8 res[19];
} sCustomer_Idc;

extern const SYSTERM_CONFIG_STRUCT sys_cfg_default;
extern const SYSTERM_CONFIG_STRUCT_RANGE sys_cfg_range;

#endif
