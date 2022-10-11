/**
  ******************************************************************************
  * @file    jt808_type.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   JT808�������Ͷ���
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

#define PK_RPT 0x0001 //package  report  , ���ط�����
#define PK_NET 0x0002 //package  net, ���ط�����
#define PK_HEX 0x0004 //package hex, ͸�����ײ����ط�
#define PK_SUB 0x0008 //package subpackage  �ְ�
#define PK_ENC 0x0008 //package encryption  ����

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
25	0������δ������1������������
26	0������δ���٣�1���������٣�
27	0�������ƾ�����1�����ƾ�����
28	0��������أ�1���������أ�
29	0�������ܱգ�1������򿪣�
30	0������ƽ�ţ�1�����������
31	0����ά��ģʽ��1��ά��ģʽ��
*/
typedef union {
    u32 reg;
    struct
    {
        bool acc : 1;    //ACC
        bool fix : 1;    //���Ƕ�λ����
        bool lat : 1;    //γ��  1 ��γ
        bool lng : 1;    //����  1 ����
        bool engine : 1; //������ 1: ON  0:Off
        bool res1 : 1;   //��γ�ȼ��ܲ��
        bool res2 : 1;
        bool res3 : 1;

        u32 carriage : 2;   //����״̬
        bool cut_oil : 1;   //��������/�Ͽ�
        bool cut_pwr : 1;   //��·����/�Ͽ�
        bool lock_door : 1; //���ż���/���Ž���
        u32 door_front : 1; //ǰ��״̬1
        u32 door_2 : 1;     //����״̬2
        u32 door_back : 1;  //����״̬3

        u32 door_4 : 1;       //��״̬4
        u32 door_5 : 1;       //��״̬5
        u32 fix_gps : 1;      //ʹ��GPS��λ
        u32 fix_bd : 1;       //ʹ�ñ�����λ
        u32 fix_glonass : 1;  //ʹ��glonass��λ
        u32 fix_calileo : 1;  //ʹ��calileo��λ
        bool left_cover : 1;  //0������ܱգ�1����Ǵ�
        bool right_cover : 1; //Ԥ��

        bool all_flag : 1;    //Ԥ��
        bool lock_car_sw : 1; //�����쳣����
        bool speeding : 1;    //��������
        bool car_rise : 1;    //0�������ƾ�����1�����ƾ�����
        bool loading : 1;     //0��������أ�1���������أ�
        bool car_cover : 1;   //0�������ܱգ�1������򿪣�
        bool car_box : 1;     //0������ƽ�ţ�1�����������
        bool right_turn : 1;  //Ԥ��
    } bit;
} UNION_VEHICLE_STATE;

/*lint -e46 */
/*lint -e768 */
typedef union {
    u32 reg;
    struct
    {                        //��������Ҫ������������˳��ԭ����ͬ�ˣ�������ã�����/�رչ��ܣ������˳���йء�������
        bool sos : 1;        //0	1�����������������������غ󴥷�
        bool speed_over : 1; //����
        bool fatigue : 1;    //ƣ�ͼ�ʻ����
        bool speed_near : 1; //����Ԥ��
        bool gps_err : 1;    //GPSģ�����
        bool ant_open : 1;   //5	1��GNSS ����δ�ӻ򱻼��ϱ���
        bool ant_short : 1;  //6	1��GNSS ���߶�·����
        bool low_volt : 1;   //7	1���ն�����ԴǷѹ����

        bool cut_volt : 1;     //8	1���ն�����Դ���籨��
        bool lcd_err : 1;      //���ϱ���
        bool tts_err : 1;      //���ϱ���
        bool camera_err : 1;   //���ϱ���
        bool oil_err : 1;      //���ϱ���
        bool gsm_err : 1;      //���ϱ���
        bool fatigue_near : 1; //res2					   ����������ʾ
        bool res3 : 1;

        bool res4 : 1;        //16λ-
        bool res5 : 1;        //
        bool drive_over : 1;  //��ʻ��ʱ����
        bool stop_over : 1;   //ͣ������
        bool area_in_out : 1; //�����������
        bool road_in_out : 1; //·�߽�������
        bool road_over : 1;   //·��ƫ�뱨��
        bool road_leave : 1;  //·���뿪����

        bool vss_err : 1;          //VSS����
        bool oil_alm : 1;          //�����쳣����
        bool vehi_lose : 1;        //��������
        bool acc_on_vehi_err : 1;  //�����Ƿ����
        bool move_vehi_err : 1;    //�����Ƿ�λ��
        bool collision : 1;        //��ײ����
        bool turn_on_one_side : 1; //�෭����
        bool arming_door : 1;      //�Ƿ����ű���
    } bit;
} VEHICLE_ALARM_UNION;

#if (P_RCD == RCD_BASE)
typedef union {
    u32 reg;
    struct {
        u32 near_light : 1;  //�����
        u32 far_light : 1;   //Զ���
        u32 right_light : 1; //��ת
        u32 left_light : 1;  //��ת
        u32 brake : 1;       //ɲ��
        u32 res05 : 1;       //���
        u32 res06 : 1;
        u32 res07 : 1;

        u32 beep : 1; //����
        u32 res10 : 7;
        u32 res20 : 8;
        u32 res30 : 8;
    } b;
} U_BD_RCD_STATUS;
#endif

#if defined(JTT_808_2019) //jt808����2019

typedef struct
{
    u16 crc;
    u16 flag;
    u16 province;         //ʡID
    u16 city;             //��ID
    u8 mfg_id[11];        //������ID
    u8 terminal_type[30]; //��Ʒ�ͺ�
    u8 terminal_id[30];   //�ն�ID
    u8 color;             //������ɫ
    u8 license[10];       //���Ʊ�ʶ    ���÷ϵ�
    u8 res1[1];           //sLoginCode[21]; //��Ȩ��
    u8 weight_auto_close; //=0x55 �س��Զ��ر�
    u8 bCarLogin;         //��Ȩ�ɹ���־λ

    u8 gps_no_star; //������
    u16 blank_car_weight;
    u8 all_open_switch; ///0x55 -- ���� ����ر�
    u8 ztb_func;        //=0x55 �ر������칦�ܣ�  �����������칦��
    u8 ztb_gk;          //������ܿ�
    u8 ill_sw;          //�Ƿ��ٶ�����  =0x55 �Ƿ���
    u8 lift_weight;     //�涷�س�
    u8 all_polar;       //ȫ�ټ���
    u8 raise_polar;     //ȫ�ټ���
    u8 left_polar;
    u8 right_polar;
    u8 speed;    //�ٶȱ���ϵ��
    u8 can_type; //0x00 - Ĭ�ϱ�׼�汾��01-����˹Э��
    u8 acc_simulate;
} REGISTER_CONFIG_STRUCT; //ע����Ϣ

/*lint -e46 */
/*lint -e768 */

typedef union {
    u16 property; //��Ϣ������
    struct
    {
        u16 mess_len : 10; //��Ϣ�峤�� ���1024�ֽ�
        u8 encry_type : 3; //��������
        u8 if_pack : 1;    //=1 ����Ϣ  =0 ����Ϣ
        u8 version : 1;    //�汾��ʶ
        u8 res : 1;
    } bit;
} NEWS_HEAD_PROPERTY_UNION; //��Ϣͷ�ṹ��

#pragma pack(1)
typedef struct
{
    u16 id;                       //��ϢID 2�ֽ�
    NEWS_HEAD_PROPERTY_UNION pro; //��Ϣͷ����
    u8 version;                   //2019����Ӱ汾��
    u8 number[10];                //�ֻ��� ����12λǰͷ����
    u16 run_no;                   //��Ϣ��ˮ��
    u16 sub_tatal;                //��Ϣ������
    u16 sub_no;                   //��Ϣ�����
} SEND_MESS_INFOMATION_STRUCT;    //��Ϣͷ����
#pragma pack()

#else /**************jt808����2013********************/

typedef struct
{
    u16 crc;
    u16 flag;
    u16 province;         //ʡID
    u16 city;             //��ID
    u8 mfg_id[5];         //������ID
    u8 terminal_type[20]; //��Ʒ�ͺ�
    u8 terminal_id[7];    //�ն�ID
    u8 color;             //������ɫ
    u8 license[10];       //���Ʊ�ʶ    ���÷ϵ�
    u8 res1[1];           //sLoginCode[21]; //��Ȩ��
    u8 weight_auto_close; //=0x55 �س��Զ��ر�
    u8 bCarLogin;         //��Ȩ�ɹ���־λ

    u8 gps_no_star; //������
    u16 blank_car_weight;
    u8 all_open_switch; ///0x55 -- ���� ����ر�
    u8 ztb_func;        //=0x55 �ر������칦�ܣ�  �����������칦��
    u8 ztb_gk;          //������ܿ�
    u8 ill_sw;          //�Ƿ��ٶ�����  =0x55 �Ƿ���
    u8 lift_weight;     //�涷�س�
    u8 all_polar;       //ȫ�ټ���
    u8 raise_polar;     //ȫ�ټ���
    u8 left_polar;
    u8 right_polar;
    u8 speed;    //�ٶȱ���ϵ��
    u8 can_type; //0x00 - Ĭ�ϱ�׼�汾��01-����˹Э��
    u8 acc_simulate;
} REGISTER_CONFIG_STRUCT; //ע����Ϣ

typedef union {
    u16 property; //��Ϣ������
    struct
    {
        u16 mess_len : 10; //��Ϣ�峤�� ���1024�ֽ�
        u8 encry_type : 3; //��������
        u8 if_pack : 1;    //=1 ����Ϣ  =0 ����Ϣ
        u8 res : 2;
    } bit;
} NEWS_HEAD_PROPERTY_UNION; //��Ϣͷ�ṹ��

typedef struct
{
    u16 id;                       //��ϢID 2�ֽ�
    NEWS_HEAD_PROPERTY_UNION pro; //��Ϣͷ����
    u8 number[6];                 //�ֻ��� ����12λǰͷ����
    u16 run_no;                   //��Ϣ��ˮ��
    u16 sub_tatal;                //��Ϣ������
    u16 sub_no;                   //��Ϣ�����
} SEND_MESS_INFOMATION_STRUCT;    //��Ϣͷ����

#endif

typedef struct
{
    u16 crc;
    u16 flag;
    u32 hear_beat_time;    //�������
    u32 tcp_over_time;     //TCP��ϢӦ��ʱʱ��
    u32 tcp_resend_cnt;    //TCP��Ϣ�ش�����
    u32 udp_over_time;     //UDP��ϢӦ��ʱʱ��
    u32 udp_resend_cnt;    //UDP��Ϣ�ش�����
    u32 sms_over_time;     //SMS��ϢӦ��ʱʱ��
    u32 sms_resend_cnt;    //SMS��Ϣ�ش�����
} LINK_KEEP_CONFIG_STRUCT; //��·���ֲ���

typedef enum {
    SERVER_NOMAL,
    SERVER_UPDATE,
    SERVER_COAL //���ɵ���ƽ̨���޻ظ�����
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
    u8 terminal_id[21]; //��������  �ն�ID��
    u8 select_tcp;      //0: ��������UDP  1:��������TCP    xx1λ �������� x1xλ ���÷�����  1xxλ����������
    u8 apn[24];         //APN
    u8 sim_user[10];    //�û���
    u8 sim_pwd[8];      //����
    u8 main_ip[24];     //Զ��IP��ַ
    u8 bak_ip[24];      //����Զ��IP��ַ
    u8 ip3[24];         // ��3·IP
    u8 fix_res1[24];    //u8BApn[24]
    u8 fix_res2[10];    //sim_user[10]
    u8 fix_res3[8];     //sim_pwd[8]
    u8 ota_ip[24];      //����Զ��IP��ַ
    u32 main_port;      //�˿ں�
    u32 bak_port;       //UDP�˿� -- ���÷������˿�
    u32 port3;
    u32 ota_port; //�����˿�
    u32 tt_baud;  //͸���ڲ�����
    u8 dbip;      //����20140412 doubleip	 0x5a: ���� ����:�ر�
    u8 res[16];
} IP_SERVER_CONFIG_STRUCT; //����������

typedef struct
{
    u16 crc;
    u16 flag;
    u16 heartbeat;   //����Ƶ�� Ĭ��ʱ��
    u32 sample_mode; //����ģʽ 0x00 ��ʱ���� 0x01 ������� 0x02 ��ʱ����㱨
    u32 acc_mode;    //0: ����ACC״̬   1: ���ݵ�¼�ٸ���ACC
    u32 no_dvr_time; //�޼�ʻԱ�㱨ʱ����
    u32 sleep_time;  //���߻㱨ʱ����
    u32 sos_time;    //�����㱨ʱ����
    u32 dft_time;    //ȱʡ�㱨ʱ����
    u32 dft_dst;     //ȱʡ����㱨���
    u32 no_dvr_dst;  //�޼�ʻԱ�㱨������
    u32 sleep_dst;   //���߻㱨����
    u32 sos_dst;     //������������
    u32 angle_dgree; //�յ㲹���Ƕ�
    u8 res[12];
} REPORT_CONFIG_STRUCT; //�㱨����

typedef struct
{
    u16 crc;
    u16 flag;
    u8 listen[PHONELEN];    //���ƽ̨�绰����
    u8 reset[PHONELEN];     //��λ�绰����
    u8 factory[PHONELEN];   //�ָ��������õ绰����
    u8 sms[PHONELEN];       //���ƽ̨SMS�ı���������	SmsAlaPhone
    u8 alarm_sms[PHONELEN]; //�ն˽���SMS�ı���������
    u32 ring_type;          //��ͨģʽ
    u32 each_time;          //ÿ��ͨ��ʱ��
    u32 month_time;         //�����ۼ�ͨ��ʱ��
    u8 user[PHONELEN];      //���ƽ̨�绰���� 1
    u8 center[PHONELEN];    //���ƽ̨��Ȩ���ź���
    u8 res[12];
} sCommunicationSet; //ͨ���������

typedef struct {
    u16 crc;
    u16 flag;
    VEHICLE_ALARM_UNION bypass_sw;   //������������ 1:����0:����
    VEHICLE_ALARM_UNION sms_sw;      //��������SMS
    VEHICLE_ALARM_UNION pic_sw;      //��Ӧ�������մ�
    VEHICLE_ALARM_UNION pic_save_sw; //��������ʱ�Ƿ���Ҫ�ϴ�
    u32 import_alarm_sw;             //�ؼ�����
    u32 speed_over_km;               //����ٶ�
    u32 speed_over_time;             //��������ʱ�䣬����������ʱ�䳬����ǰֵ��������ٱ���
    u32 fatigure_time;               //ƣ�ͼ�ʻʱ��
    u32 day_time;                    //�����ۼ�ʱ��
    u32 stop_rest_time;              //ͣ����Ϣ��Сʱ��
    u16 stoping_time;                //ͣ������ʱ����
    u16 pre_speed_over;              //Ԥ����
    u16 pre_fatigue;                 //Ԥƣ��
    u16 collision;                   //��ײ
    u16 turn_on_one_side;            //�෭
    u16 elc_geo;                     //����Χ��
    u8 res[12];
} ALARM_CONFIG_STRUCT; //����������

typedef struct {
    u16 crc;
    u16 flag;
    u8 call_mode;                       //�û�����ͨ��ģʽ 0x00 �ֱ�ͨ�� 0x01 ����ͨ��
    u8 call_set;                        //�绰��������	ͨ��Ȩ�� 0�������ƣ� 0x80���޲�����  0x40���޲��� 0xC0: ���Ʋ��벦��
    enum_speed_mode_typedef speed_mode; //220828�ٶ�����://�ٶ�ģʽ���������ٶȡ�GPS�ٶȡ�CAN�ٶ�
                                        //RST-RK:ϵͳ����
    s8 foster_care;                     //ϵͳ����//��������1���ӡ�2���ӡ�3���ӡ�5���ӡ�8���ӡ�12���ӡ�17���ӡ�23���ӡ�30����

    u32 ulUart5Baud; //����5���õĲ�����  9600/19200
    u8 highspeed0;   //�������
    u8 highspeed1;   //Υ������
    u8 lowspeed;     //�س���δ�ܱ�
    u8 mobile;

    u8 res[10];
} SYSTERM_CONFIG_STRUCT;

typedef struct {
    //220828�ٶ�����:�ٶ�ģʽ
    struct {
        enum_speed_mode_typedef min;
        enum_speed_mode_typedef max;
    } speed_mode; //ϵͳ����////�ٶ�ģʽ���������ٶȡ�GPS�ٶȡ�CAN�ٶ�

    //RST-RK:ϵͳ����
    struct {
        u8 min;
        u8 max;
    } foster_care; //ϵͳ����//��������1���ӡ�2���ӡ�3���ӡ�5���ӡ�8���ӡ�12���ӡ�17���ӡ�23���ӡ�30����
} SYSTERM_CONFIG_STRUCT_RANGE;

typedef struct {
    u16 interval; //��ʱ�����ϱ�ʱ����
    u32 time;     //��ʱ����ʱ��
    u8 open;      //�򿪱�־
} TMP_TRACE_CONFIG_STRUCT;

#define DRIVER_CONFIG_STRUCT_NAME_MAX 23
#define DRIVER_CONFIG_STRUCT_BIAN_HAO_MAX 7
typedef struct {
    u16 crc;
    u16 flag;
    u8 name_len;                                           //��ʻԱ��������
    u8 name[DRIVER_CONFIG_STRUCT_NAME_MAX + 1];            //��ʻԱ����
    u8 license[20];                                        //��ʻԱ���֤����
    u8 qualification[40];                                  //��ҵ�ʸ�֤�š���ʻ֤��
    u8 organization_len;                                   //����
    u8 organization[30];                                   //��˾��
    u8 driver_code[DRIVER_CONFIG_STRUCT_BIAN_HAO_MAX + 1]; //��ʻԱ����
    u8 date[4];                                            //��Ч��
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
    u8 vin[18];        //VIN��
    u8 license[12];    //���ƺ�
    u8 type[16];       //���Ʒ���
    u8 coefficient[3]; //��������ϵ��
    u8 highspeed;      //�������ֵ
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
    time_t setting_time; //����ʱ��
    u16 set_speed_plus;  //���������ٶ�
    u32 set_mileage;     //�������
    u8 print_type;       //��ӡ��ʽ 0x01 2003  ,0x02 2012
    u8 speed_type;       //�ٶ�ģʽ
    bool speed_cala;     //�ٶȱ궨
    u8 swit;             //�����ٶȿ���
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
} SIGNAL_CONFIG_STRUCT; //����ϵ��
#endif

typedef struct
{
    u16 crc;
    u16 flag;
    u8 ccc[7];    // 3c��֤����
    u8 model[16]; //��֤��Ʒ�ͺ�
    u8 time[6];   //����������
    u8 sn[4];     //BCD��ʽ
    u32 res;      //
} UNIQUE_ID_STRUCT;

typedef struct
{
    u16 crc;
    u16 flag;
    u8 type;      //���߼�Ȩ��ʶ�� = 0xee ��Ȩ�ɹ�
    u8 code[50];  //��Ȩ��
    u8 type2;     //���߼�Ȩ��ʶ�� = 0xee ��Ȩ�ɹ�
    u8 code2[50]; //��Ȩ��
    u8 type3;     //���߼�Ȩ��ʶ�� = 0xee ��Ȩ�ɹ�
    u8 code3[21]; //��Ȩ��
} authority_configuration_struct;

//�ͻ�����ID ʶ�����״̬�����仯ʱд��
typedef struct
{
    u16 crc;
    u16 flag;
    u8 date;            //����
    u16 lock;           //=0x55 ������Ĭ�Ϲر�
    u8 customer_id[11]; //�ͻ����룻Ĭ��TRWY
    u8 iGps3D2D;        //0x88 ��Ҫ����2D �ź�
    u8 res[19];
} sCustomer_Idc;

extern const SYSTERM_CONFIG_STRUCT sys_cfg_default;
extern const SYSTERM_CONFIG_STRUCT_RANGE sys_cfg_range;

#endif
