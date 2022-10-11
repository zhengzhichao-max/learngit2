#ifndef __TR9_AGREE_H
#define __TR9_AGREE_H

#define TR9_COM_BUF_LEN 1600
#define TR9_COM_DEC_LEN 1100

#define TRANS_F0

typedef enum {
    tr9_cmd_1717 = 0x1717,
    tr9_cmd_1810 = 0x1810, //RK->MCU
    tr9_cmd_1818 = 0x1818, //MCU->RK//����MCU�ر�RK��Դ
    tr9_cmd_1819 = 0x1819,
    tr9_cmd_2010 = 0x2010,
    tr9_cmd_2011 = 0x2011,
    tr9_cmd_2012 = 0x2012,
    tr9_cmd_2013 = 0x2013,
    tr9_cmd_2014 = 0x2014,
    tr9_cmd_1415 = 0x1415, //����MCU�汾��Ϣ��RK?MCU��
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
    tr9_cmd_5013 = 0x5013, //��MCU�ϱ�5013 IC����Ϣ��RK�·�5013��Ӧ
    tr9_cmd_5014 = 0x5014, //IC����Ϣ�ϱ�//RKѯ��IC��//RK->MCU ��������
    tr9_cmd_5016 = 0x5016,
    tr9_cmd_5017 = 0x5017,
    tr9_cmd_5018 = 0x5018,
    tr9_cmd_5019 = 0x5019,
    tr9_cmd_6001 = 0x6001,
    tr9_cmd_6002 = 0x6002,
    tr9_cmd_6003 = 0x6003, //�ı���Ϣ�·���RK->MCU��
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
    tr9_cmd_6033 = 0x6033, //��ʻ��¼��			��MCU->SOC��
    tr9_cmd_6034 = 0x6034,
    tr9_cmd_6038 = 0x6038, //6.48 ��λģʽ�л�ָ�RK->MCU��
    tr9_cmd_6036 = 0x6036, //RK->MCU
    tr9_cmd_6039 = 0x6039, //��Ȩ���·�			��SOC->MCU��
    tr9_cmd_603A = 0x603A, //���õ绰��			��SOC->MCU��
    tr9_cmd_603B = 0x603B, //�¼�����		    ��SOC->MCU��
    tr9_cmd_603D = 0x603D, //�����·�           ��SOC->MCU��
    tr9_cmd_603F = 0x603F, //��Ϣ�㲥�˵�����       ��SOC->MCU��
    tr9_cmd_6041 = 0x6041, //��Ϣ����				��SOC->MCU��
    tr9_cmd_6043 = 0x6043, //��������Ӧ�� 			��SOC->MCU��
    tr9_cmd_6045 = 0x6045, //�ն����߻�������	    ��SOC->MCU��
    tr9_cmd_6047 = 0x6047, //ͬ��رյ�Դ			��SOC->MCU��
    tr9_cmd_6050 = 0x6050,
    tr9_cmd_6051 = 0x6051, //����U��					��SOC->MCU��
    tr9_cmd_6052 = 0x6052, //U�̵������֪ͨ		��SOC->MCU��
    tr9_cmd_6054 = 0x6054, //����CAN�ɼ�ʱ��		��SOC->MCU��
    tr9_cmd_6055 = 0x6055, //CAN���������ϴ�        ��MCU->SOC��
    tr9_cmd_6056 = 0x6056, //�ն�����ϱ�:��RK->MCU����
    tr9_cmd_6057 = 0x6057, //����ѹ��ѯ://�ն�����ѹ��ѯ��RK->MCU��������
    tr9_cmd_6058 = 0x6058, //���ؿ���://VCC_OUT1 ���ؿ���(5V_OUT_SW����)��RK->MCU��//VOUT2��12V����RK��adb.exeЭ�飬ȥ���ơ�����MCU
    tr9_cmd_6059 = 0x6059, //�̵�������://�̵������ء�RK->MCU����
    tr9_cmd_6062 = 0x6062, //6.79 MCU��λ��RK->MCU��
    tr9_cmd_6063 = 0x6063, //6.80 ����ͷ״̬��RK->MCU��
    tr9_cmd_6064 = 0x6064, //6.81 �洢�豸״̬��RK->MCU��
    tr9_cmd_6065 = 0x6065, //6.82 ���������ϱ���MCU->RK��//����LSM6DS3TR-C:
    tr9_cmd_6066 = 0x6066, //�����ʻ��¼�����ݡ�SOC->MCU�� 2022-03-18 Ϊ������Բ������
    tr9_cmd_6037 = 0x6037,
    tr9_cmd_6049 = 0x6049, //�ȵ㿪��
    tr9_cmd_6042 = 0x6042, //��������				��MCU->SOC��
    tr9_cmd_6040 = 0x6040, //��Ϣ�㲥		  		��MCU->SOC��
    tr9_cmd_603E = 0x603E, //����Ӧ��		  		��MCU->SOC��
    tr9_cmd_603C = 0x603C, //�¼�����		  		��MCU->SOC��
} tr9_cmd_typedef;

#if (0)
typedef void (*fwncp_net)(u8 *str, u16 s_len, u8 from);

typedef struct {
    u16 index;
    fwncp_net func;
} wcmd_net;
#endif

typedef struct {
    bool video;       //0:����¼��?1��¼��
    bool hard_format; //0 δ��ʽ��?1��ʽ��
    bool alarm;       //0��δ����
} DVR_STATUS;

typedef struct {
    bool icc; //hand �仯����
    u32 tmr;  //ÿ��2�뷢��һ��
} IC_TO_TR9;

typedef union {
    u32 reg;
    struct {
        bool near_light : 1; //���������
        bool far_light : 1;  //Զ�������
        bool right : 1;      //��ת����ź�
        bool left : 1;       //��ת����ź�
        bool brake : 1;      //�ƶ��ź�
        bool res1 : 1;       //���
        bool res2 : 1;       //����
        bool res3 : 1;       //��ȫ�]

        bool beel : 1;       //�����ź�
        bool res4 : 1;       //����
        bool limitraise : 1; //
        bool illieage : 1;
        bool res5 : 1;
        bool left_cover : 1;  //0: �ܱ�? 1��
        bool right_cover : 1; //
        bool all_rising : 1;  //��ȫ����

        bool car_lock : 1;   //
        bool lift_limit : 1; //
        bool weight : 1;     //18	0���������ؼ��δ�ӣ�1������
        bool car_box : 1;    //19	0������������δ�ӣ�1������
        bool car_cover : 1;  //20	0�����űպϼ��δ�ӣ�1������
        bool speeding : 1;   //21	0������δ�޵ԣ�1���������٣�
        bool covering : 1;   //22	�ܱ�״ʹ(������״��)
        bool rising : 1;     //		����״ʹ

        bool loading : 1; //		����״ʹ
        bool l_value : 1; //		�ؿ���ģ����
        bool back : 1;    //26		#�Զ���ߣ���7�ţ�������ץ��
        bool high8 : 1;   //27	8#�Զ���ߣ���8�ţ�������ץ��
        bool low : 1;     //28	9#�Զ���ͣ���9�ţ��ٱ�ץ��
        bool high6 : 1;   //29	10#�Զ����
        bool res7 : 1;    //
        bool res8 : 1;    //
    } bit;
} TR9_VEHICLE_STATE;

//ȡ���޵Կ؃�
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
    unsigned char NetMode;     /*上网模式:0  IP地址模式��?1 DNS�?��*/
    unsigned char NetProtocol; /*网络协�?0 TCP,1 UDP*/

    char SeverDomainName[64];          /*���������7*/
    char SeverBackDomainName[64];      /*�������������*/
    unsigned long DomainNameIP;        /*�������õ���ip������Ƭ���� 135*/
    unsigned long BackDomainNameToMcu; /*�ɱ��������õ���ip����Ƭ���v139*/
    unsigned short SeverPort1;         /*�˿�1 2�ֽ� 	143 144 */
    unsigned short SeverPort2;         /*�˿�2 2�ֽ� 	145 146*/

    char ProvinceCode[2];   /*ʡ����147*/
    char CityCode[2];       /*��������149*/
    char VendorNum[5];      /*���̱��151 */
    char DeviceType[20];    /*�豸����156*/
    char CarIdColor;        /*������ɫ176*/
    char CarId[12];         /*���ƺ���177*/
    char CarIdentity[24];   /*����ʶ����189*/
    char DrvLicenseNum[24]; /*��ʻ֤��213*/
    char CarType[24];       /*��������237*/
    char SIMCardNum[20];    /*SIM����261*/
    unsigned short Pulse;   //����ϵ�� 2�ֽ�ȡ?����360-9999 /273?274
    char termnalId[7];      /*�ն�ID 7�ֽ�*/
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
    u8 state_rouse;   //��������
    u8 timer_rouse;   //��ʱ����
    u8 note_rouse;    //���Ż���
    u8 update_timr;   //���¶�ʱ����
    u8 Revc_sms_flag; //���ű�־

    u8 AwakenState;
    struct {
        bool Sos_Flag : 1;
        bool Crash_Flag : 1;
        bool Car_Door_Flag : 1;
        u8 ret : 5; //����
    } bit;          //�������ѱ�־
    u8 timerRouse;
    struct {
        bool Monday : 1; //��һ
        bool Tuesday : 1;
        bool Wednesday : 1;
        bool Thursday : 1;
        bool Friday : 1;
        bool Saturday : 1;
        bool weekend : 1; //��ĩ
        bool ret1 : 1;    //����
    } week;               //��������
    u8 time_Bucket;       //ʱ���
    u16 sms_wakeup_Time;  //���Ż�ȡ���Ķ�ʱʱ��
    u16 sms_start_time;   //���Ż��ѵĿ���ʱ��
    //Timing_day  sms_colse_time;		//���Ż��ѵĹر�ʱ��
    Timing_day timing_Awaken[4]; //��ʱ����

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
    u8 RK_time_rang_NG_flag; //2022-03-23  ��ʻ��¼�Ƕ�ȡʱ��α�־
                             //            �����ʱ�����=0; �����ʱ�����=1; ��=1ʱ ��ʾ�����нڵ������
    u8 RK_nack_only_delay;   //2022-03-23  �˱�־��Ϊ���ڲ��Զ���, �����RK��Ӧ��,�ӳ�ʱ�䵽��,�ͷ�����һ�����ݡ�
    u8 RK_read_all;          //2022-03-23  �˱�־��Ϊ���ڲ��Զ���, =0 ������CMD; =1 ������CMD.
    u8 resetCount;
    u32 resetTime; //22/5/26 Q ????
} Rk_Task_Manage_t;

extern u8 BBstatus;
extern Rk_Task_Manage_t Rk_Task_Manage;
extern bool rouse_flag;
extern Set_Can_t tr9_Set_Can[28];
extern u8 versions[20]; //?��??
extern TR9_VEHICLE_STATE tr9_car_status;
extern DVR_STATUS dvr_status;
extern IC_TO_TR9 ic_tr9;
extern bool reset_hi3520_open;
extern u8 tr9_www_status;
extern u16 uPolygon;
extern s_GPSCOM s_Tr9Com;
extern Set_Awaken_t Set_Awaken; //���û��Ѳ���

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
extern void HG_heartbeat(void); //��������

void tr9_Auto_start(u8 *str, u16 s_len);

#endif
