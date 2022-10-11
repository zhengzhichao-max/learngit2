#ifndef __TR9_AGREE_H
#define __TR9_AGREE_H

#define TR9_COM_BUF_LEN 1600
#define TR9_COM_DEC_LEN 1100

#define TRANS_F0

typedef enum {
    tr9_cmd_1717 = 0x1717,
    tr9_cmd_1810 = 0x1810, //RK->MCU
    tr9_cmd_1818 = 0x1818, //MCU->RK//请求MCU关闭RK电源
    tr9_cmd_1819 = 0x1819,
    tr9_cmd_2010 = 0x2010,
    tr9_cmd_2011 = 0x2011,
    tr9_cmd_2012 = 0x2012,
    tr9_cmd_2013 = 0x2013,
    tr9_cmd_2014 = 0x2014,
    tr9_cmd_1415 = 0x1415, //请求MCU版本信息【RK?MCU】
    tr9_cmd_1616 = 0x1616,
    tr9_cmd_2021 = 0x2021,
    tr9_cmd_2022 = 0x2022,
    tr9_cmd_4001 = 0x4001,
    tr9_cmd_4040 = 0x4040,
    tr9_cmd_4041 = 0x4041,
    tr9_cmd_5001 = 0x5001,
    tr9_cmd_5003 = 0x5003,
    tr9_cmd_5004 = 0x5004,
    tr9_cmd_5005 = 0x5005,
    //tr9_cmd_5008 = 0x5008,
    tr9_cmd_5012 = 0x5012, //MCU_LOCK_ALARM
    tr9_cmd_5013 = 0x5013, //当MCU上报5013 IC卡信息，RK下发5013响应
    tr9_cmd_5014 = 0x5014, //IC卡信息上报//RK询问IC卡//RK->MCU 下行命令
    tr9_cmd_5016 = 0x5016,
    tr9_cmd_5017 = 0x5017,
    tr9_cmd_5018 = 0x5018,
    tr9_cmd_5019 = 0x5019,
    tr9_cmd_6001 = 0x6001,
    tr9_cmd_6002 = 0x6002,
    tr9_cmd_6003 = 0x6003, //文本信息下发【RK->MCU】
    tr9_cmd_6007 = 0x6007,
    tr9_cmd_6008 = 0x6008,
    tr9_cmd_6010 = 0x6010,
    tr9_cmd_6012 = 0x6012,
    tr9_cmd_6014 = 0x6014,
    tr9_cmd_6015 = 0x6015,
    tr9_cmd_6016 = 0x6016,
    tr9_cmd_6017 = 0x6017,
    tr9_cmd_6020 = 0x6020,
    tr9_cmd_6031 = 0x6031,
    tr9_cmd_6032 = 0x6032,
    tr9_cmd_6033 = 0x6033, //行驶记录仪			【MCU->SOC】
    tr9_cmd_6034 = 0x6034,
    tr9_cmd_6038 = 0x6038, //6.48 定位模式切换指令【RK->MCU】
    tr9_cmd_6036 = 0x6036, //RK->MCU
    tr9_cmd_6039 = 0x6039, //鉴权码下发			【SOC->MCU】
    tr9_cmd_603A = 0x603A, //设置电话本			【SOC->MCU】
    tr9_cmd_603B = 0x603B, //事件设置		    【SOC->MCU】
    tr9_cmd_603D = 0x603D, //提问下发           【SOC->MCU】
    tr9_cmd_603F = 0x603F, //消息点播菜单设置       【SOC->MCU】
    tr9_cmd_6041 = 0x6041, //信息服务				【SOC->MCU】
    tr9_cmd_6043 = 0x6043, //音量调节应答 			【SOC->MCU】
    tr9_cmd_6045 = 0x6045, //终端休眠唤醒设置	    【SOC->MCU】
    tr9_cmd_6047 = 0x6047, //同意关闭电源			【SOC->MCU】
    tr9_cmd_6050 = 0x6050,
    tr9_cmd_6051 = 0x6051, //挂载U盘					【SOC->MCU】
    tr9_cmd_6052 = 0x6052, //U盘导出完成通知		【SOC->MCU】
    tr9_cmd_6054 = 0x6054, //设置CAN采集时间		【SOC->MCU】
    tr9_cmd_6055 = 0x6055, //CAN总线数据上传        【MCU->SOC】
    tr9_cmd_6056 = 0x6056, //终端里程上报:【RK->MCU】；
    tr9_cmd_6057 = 0x6057, //主电压查询://终端主电压查询【RK->MCU】；新增
    tr9_cmd_6058 = 0x6058, //开关控制://VCC_OUT1 开关控制(5V_OUT_SW开关)【RK->MCU】//VOUT2，12V，由RK，adb.exe协议，去控制。不在MCU
    tr9_cmd_6059 = 0x6059, //继电器开关://继电器开关【RK->MCU】；
    tr9_cmd_6062 = 0x6062, //6.79 MCU复位【RK->MCU】
    tr9_cmd_6063 = 0x6063, //6.80 摄像头状态【RK->MCU】
    tr9_cmd_6064 = 0x6064, //6.81 存储设备状态【RK->MCU】
    tr9_cmd_6065 = 0x6065, //6.82 六轴数据上报【MCU->RK】//六轴LSM6DS3TR-C:
    tr9_cmd_6066 = 0x6066, //清除行驶记录仪数据【SOC->MCU】 2022-03-18 为方便测试部标测试
    tr9_cmd_6037 = 0x6037,
    tr9_cmd_6049 = 0x6049, //热点开关
    tr9_cmd_6042 = 0x6042, //音量调节				【MCU->SOC】
    tr9_cmd_6040 = 0x6040, //消息点播		  		【MCU->SOC】
    tr9_cmd_603E = 0x603E, //提问应答		  		【MCU->SOC】
    tr9_cmd_603C = 0x603C, //事件报告		  		【MCU->SOC】
} tr9_cmd_typedef;

#if (0)
typedef void (*fwncp_net)(u8 *str, u16 s_len, u8 from);

typedef struct {
    u16 index;
    fwncp_net func;
} wcmd_net;
#endif

typedef struct {
    bool video;       //0:不在录像?1：录偿
    bool hard_format; //0 未格式化?1格式势
    bool alarm;       //0：未报警
} DVR_STATUS;

typedef struct {
    bool icc; //hand 变化触发
    u32 tmr;  //每隔2秒发送一檿
} IC_TO_TR9;

typedef union {
    u32 reg;
    struct {
        bool near_light : 1; //近光灯信卿
        bool far_light : 1;  //远光灯信卿
        bool right : 1;      //右转向灯信号
        bool left : 1;       //左转向灯信号
        bool brake : 1;      //制动信号
        bool res1 : 1;       //雾灯
        bool res2 : 1;       //倒车
        bool res3 : 1;       //安全嶿

        bool beel : 1;       //喇叭信号
        bool res4 : 1;       //车门
        bool limitraise : 1; //
        bool illieage : 1;
        bool res5 : 1;
        bool left_cover : 1;  //0: 密闭? 1打开
        bool right_cover : 1; //
        bool all_rising : 1;  //完全举升

        bool car_lock : 1;   //
        bool lift_limit : 1; //
        bool weight : 1;     //18	0：车辆载重监测未接，1：连挿
        bool car_box : 1;    //19	0：车厢举升监测未接；1：连挿
        bool car_cover : 1;  //20	0：厢门闭合监测未接；1：连挿
        bool speeding : 1;   //21	0：车辆未限翟，1：车辆限速；
        bool covering : 1;   //22	密闭状使(即顶蓬状徿)
        bool rising : 1;     //		举升状使

        bool loading : 1; //		空重状使
        bool l_value : 1; //		重空载模拟量
        bool back : 1;    //26		#自定义高（第7脚）庿关门抓拍
        bool high8 : 1;   //27	8#自定义高（第8脚）庿关门抓拍
        bool low : 1;     //28	9#自定义低（第9脚）举报抓拍
        bool high6 : 1;   //29	10#自定义高
        bool res7 : 1;    //
        bool res8 : 1;    //
    } bit;
} TR9_VEHICLE_STATE;

//取消限翟控冿
typedef struct
{
    bool ctrl;
    u16 tmr;
} CANCEL_SPEED_CTRL;

typedef struct
{
    u32 tmr;
    bool clear;

} DELAY_CLEAR_TR9_COMM;

typedef struct {
    unsigned char NetMode;     /*涓婄綉妯″紡:0  IP鍦板潃妯″紡钿?1 DNS妯?紡*/
    unsigned char NetProtocol; /*缃戠粶鍗忚?0 TCP,1 UDP*/

    char SeverDomainName[64];          /*域名服务嗿7*/
    char SeverBackDomainName[64];      /*备份域名服务嗿*/
    unsigned long DomainNameIP;        /*由域名得到的ip，给单片机用 135*/
    unsigned long BackDomainNameToMcu; /*由备份域名得到的ip给单片机璿139*/
    unsigned short SeverPort1;         /*端口1 2字节 	143 144 */
    unsigned short SeverPort2;         /*端口2 2字节 	145 146*/

    char ProvinceCode[2];   /*省区卿147*/
    char CityCode[2];       /*城市区号149*/
    char VendorNum[5];      /*厂商编号151 */
    char DeviceType[20];    /*设备类型156*/
    char CarIdColor;        /*车牌颜色176*/
    char CarId[12];         /*车牌号码177*/
    char CarIdentity[24];   /*车辆识别卿189*/
    char DrvLicenseNum[24]; /*驾驶证号213*/
    char CarType[24];       /*车辆类型237*/
    char SIMCardNum[20];    /*SIM卡号261*/
    unsigned short Pulse;   //脉冲系数 2字节取?范噿360-9999 /273?274
    char termnalId[7];      /*终端ID 7字节*/
    unsigned long ip3;
    unsigned short port3;
} JT808_2018_PARA;

typedef struct
{
    bool lock;
    u32 tmr;
} KEY_LOCK_STATUS;

typedef struct
{
    u16 OnTime;
    u16 OffTime;
} Timing_day;

typedef struct
{
    u8 state_rouse;   //条件唤醒
    u8 timer_rouse;   //定时唤醒
    u8 note_rouse;    //短信唤醒
    u8 update_timr;   //更新定时设置
    u8 Revc_sms_flag; //短信标志

    u8 AwakenState;
    struct {
        bool Sos_Flag : 1;
        bool Crash_Flag : 1;
        bool Car_Door_Flag : 1;
        u8 ret : 5; //保留
    } bit;          //条件唤醒标志
    u8 timerRouse;
    struct {
        bool Monday : 1; //周一
        bool Tuesday : 1;
        bool Wednesday : 1;
        bool Thursday : 1;
        bool Friday : 1;
        bool Saturday : 1;
        bool weekend : 1; //周末
        bool ret1 : 1;    //保留
    } week;               //唤醒日期
    u8 time_Bucket;       //时间段
    u16 sms_wakeup_Time;  //短信获取到的定时时间
    u16 sms_start_time;   //短信唤醒的开启时间
    //Timing_day  sms_colse_time;		//短信唤醒的关闭时间
    Timing_day timing_Awaken[4]; //定时唤醒

} Set_Awaken_t;

typedef struct
{
    bool RK_1717_state;
    bool RK_6051_state;
    bool RK_record_print_log; //2022-03-17
    bool RK_uart_auto_flag;   //2022-03-23
    bool RK_send_0200_flag;
    bool RK_send_0200;
    bool RK_WaitingSend;
    bool RK_6047_flag;
    u8 RK_time_rang_NG_flag; //2022-03-23  行驶记录仪读取时间段标志
                             //            如果有时间段则=0; 如果无时间段则=1; 当=1时 表示读所有节点的数据
    u8 RK_nack_only_delay;   //2022-03-23  此标志是为串口测试而用, 不理会RK的应答,延迟时间到后,就发送下一包数据。
    u8 RK_read_all;          //2022-03-23  此标志是为串口测试而用, =0 读单个CMD; =1 读所有CMD.
    u8 resetCount;
    u32 resetTime; //22/5/26 Q ????
} Rk_Task_Manage_t;

extern u8 BBstatus;
extern Rk_Task_Manage_t Rk_Task_Manage;
extern bool rouse_flag;
extern Set_Can_t tr9_Set_Can[28];
extern u8 versions[20]; //?汾??
extern TR9_VEHICLE_STATE tr9_car_status;
extern DVR_STATUS dvr_status;
extern IC_TO_TR9 ic_tr9;
extern bool reset_hi3520_open;
extern u8 tr9_www_status;
extern u16 uPolygon;
extern s_GPSCOM s_Tr9Com;
extern Set_Awaken_t Set_Awaken; //设置唤醒参数

extern void send_base_msg_to_tr9(void);
extern void tr9_parse_task(void);
extern void report_key_status_to_tr9(void);
extern void ic_card_to_tr9(void);
extern void mcu_ask_tr9_www_status(void);
extern void tr9_net_parse(u8 *p, u16 p_len, u8 from);
extern void tr9_comm_init(void);
extern void tr9_comm_pass(u8 comm, u8 bh, u8 *str, u16 s_len);
extern void tr9_6044_task(void);
extern void AwakenSet(void);
extern void tr9_6032_task(void);

//extern void tr9_1819(void);
//extern  void usb_6051_task(void);
extern void HG_heartbeat(void); //海格心跳

void tr9_Auto_start(u8 *str, u16 s_len);

#endif
