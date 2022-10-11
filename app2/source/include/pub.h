/**
  ******************************************************************************
  * @file    pub.h
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   ���ù��ܻ�������
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
    } while (0) //nop(); //h ע��

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
    VEHICLE_ALARM_UNION alarm;     //�ڶ�״̬��־�� (������)
    UNION_VEHICLE_STATE car_state; //��һ״̬��־�� (������)
    point_t locate;                //��γ��
    u16 heigh;                     //�߶�����
    u16 speed;                     //�ٶ�����    1/10 km/h
    u16 direction;                 //��λ������  0~360
    time_t time;                   //ʱ������
    bool fixed;                    //�Ƿ�λ

    //0x0200 ��Ϣ�壬������Ϣ
    u32 dist;            //���� meter;
    u16 ad_oil;          //����AD ֵ
    u16 oil_L;           //������λL
    u16 complex_speed;   //GPS �������ٶȣ� 1 km/h
    u16 complex_speed01; //GPS �������ٶȣ� 0.1 km/h
    u16 vot_V;           //��ѹ
    u8 csq;              //CSQ �ź�
    u8 fix_num;          //��λ���ǿ���

    //������Ϣ
    u16 average_speed; //ƽ���ٶ�
    u16 pwr_vol;       //��ѹ��ѹ
    u8 pwr_per;        //��Դ�ٷֱ�
    u8 pwr_type;       //��ϵ��ѹ
    bool moving;       //�����Ƿ���ʻ
    bool power;        //������ƿ�Ƿ���Ч

    bool ic_login;    //IC ����ǩ
    bool ic_exchange; //������¼��
    u8 ic_driver[18]; //��ʻԱ��Ϣ
    u16 box_oil;
    u16 placeIostate;
    u8 res[31];
} MIX_GPS_DATA;

//0x1818  0x1819
typedef struct
{
    u32 tmr; //超时后溢�? 10秒强制关�?

    bool pwr_open; //关闭时使�?
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

#define LINK_IP0 0x01 //��IP
#define LINK_IP1 0x02 //��IP
#define LINK_IP2 0x04 //����IP
#define LINK_INIT 0x08
#define LINK_IP3 0x10 //����IP

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
    u8 cur_ip;   //��ǰIP ����
    u8 link_cnt; //��ǰ���Ӵ���
    u8 domain_cnt;
    u8 IP0_status;    //��IP
    u8 IP1_status;    //����IP
    u8 IP2_status;    //����IP
    u8 IP1_cnt;       //����IP
    bool reset;       //������λ
    bool ack;         //Ӧ��
    bool sms_rst_en;  //��ʱ����λ
    u8 dbip;          //����20140412 doubleip	 0x5a: ���� ����:�ر�
    u16 sms_rst_tick; //��ʱ��λʱ��ʼʱ��
    u32 online_tick;  //GPRS ����ʱ��ʱ��
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
        u8 cut_oil : 1;      //�Ͽ��͵��·
        u8 rels_oil : 1;     //�ָ��͵��·
        u8 from_net : 1;     //Զ�̿���
        u8 from_handset : 1; //�ֱ�����
        u8 day_en : 1;
        u8 res0 : 3;

        u8 res1 : 8;
        u8 res2 : 8;
        u8 res3 : 8;
    } b;
} REMOTE_CAR;

typedef struct
{
    u16 crc;        //������Ӹñ���, ee_pwrite ���crc
    u16 flag;       //������Ӹñ���, ee_pwrite ���8700
    u32 total_dist; //��λ��
    REMOTE_CAR car;
    u8 cover;        //��ǰ����
    u8 relay_switch; //����SMS ���ŷ�������
    u8 iccard;       //=1  �Ѳ���IC ����=0 δ ����IC ��
    u8 ip_to_tr9;    //=1 ��д��
    u8 voice_sw;
    u8 lock_gps;
    u8 lock_car_sw; //
    u8 area_in_out; //0x55  Χ����
    u8 illeage;     //�Ƿ��ٶ�״̬

} run_parameter;

typedef struct
{
    unsigned int time_on;  //��ͨʱ��
    unsigned int time_off; //�Ͽ�ʱ��
    unsigned int cycle;    //����
    unsigned long phase;   //��ʱ
    unsigned char level;   //����
} out_ctrl_block;

typedef enum {
    PWR_IDLE,  //����ģʽ
    PWR_WAIT,  //��ʱ����
    PWR_SLEEP, //����ģʽ
    PWR_ERR
} E_POWER_MAMNAGE;

typedef enum {
    IACC_IDLE,  //����ģʽ
    IACC_CLOSE, //����ģʽ
    IACC_OPEN,  //��ʱ����

} E_ACC_MAMNAGE;

typedef struct
{
    bool swi;
    bool open;
    bool one;
    u32 time_on; //��ͨʱ��
    u32 one_time;
} can_speed_cali;

typedef struct
{
    bool reset; //��λ//trueʱ���ᴥ������RK�Ķ���
    bool ack;   //
    bool start; //����90��û��Ӧ����Ϊ����������
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
extern UNION_VEHICLE_STATE car_state; //��һ״̬��־�� (������)
extern sCommunicationSet phone_cfg;
extern VEHICLE_ALARM_UNION bypass_alarm;
extern REMOTE_CAR rcmd;
extern out_ctrl_block car;
extern DVR_POWER_CTRL dvr_power;
extern u16 start_tr9_tmr;
//*****************************************************************************
//*****************************************************************************
//-----------		        	    ��������   	              -----------------
//-----------------------------------------------------------------------------

#define usart_send_buff_max_lgth 1100      //����1���ͻ�������󳤶ȶ���  //1100
#define usart_rec_queue_max_lgth 2200      //����1���ն��л�������󳤶ȶ���  //2200
#define usart_rec_valid_buff_max_lgth 1024 //����1������Ч���ݻ�������󳤶ȶ���  //1024

#define sound_play_buff_max_lgth 512 //�����ʶ����������ֵ

#define spi1_up_comm_team_max 64 //����������ֵ

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
//*****************************************************************************
//-----------		             ���ⳤ�ȶ���   	          -----------------
//-----------------------------------------------------------------------------

#define password_para_max_lgth ((u8)8)         //��������������󳤶�
#define password_oil_max_lgth ((u8)8)          //��·����������󳤶�
#define password_car_max_lgth ((u8)8)          //��������������󳤶�
#define password_sensor_max_lgth ((u8)8)       //������ϵ���޸�������󳤶�
#define password_speed_max_lgth ((u8)8)        //�ٶȷ�ʽѡ��������󳤶�
#define password_lcd_para_rst_max_lgth ((u8)8) //Һ����������ʼ��������󳤶�
//#define     password_regist_max_lgth    ((u8)8)        //ע����ز���������󳤶�

#define para_main_ip_max_lgth ((u8)32)    //��IP/��������󳤶�
#define para_backup_ip_max_lgth ((u8)32)  //����IP/����������󳤶�
#define para_id_max_lgth ((u8)12)         //����ID��󳤶�
#define para_apn_max_lgth ((u8)32)        //APN��󳤶�
#define para_num_server_max_lgth ((u8)15) //���������󳤶�   chwsh 2017
#define para_num_user_max_lgth ((u8)13)   //�û�������󳤶�

#define rec_para_ratio_max_lgth ((u8)6)  //�ٶȴ�����ϵ��ASCII�볤��
#define rec_car_vin_max_lgth ((u8)17)    //����VIN�볤��
#define rec_car_plate_max_lgth ((u8)8)   //���ƺ��볤��
#define rec_province_id_max_lgth ((u8)2) //ʡ��ID����
#define rec_city_id_max_lgth ((u8)4)     //����ID����

#if defined(JTT_808_2019)

#define rec_manufacturer_id_max_lgth ((u8)11) //������ID����
#define rec_terminal_type_max_lgth ((u8)30)   //�ն��ͺų���
#define rec_terminal_id_max_lgth ((u8)30)     //�ն�ID����

#else

#define rec_manufacturer_id_max_lgth ((u8)5) //������ID����
#define rec_terminal_type_max_lgth ((u8)20)  //�ն��ͺų���
#define rec_terminal_id_max_lgth ((u8)7)     //�ն�ID����

#endif
#define rec_car_type_max_lgth ((u8)12) //�������Ʒ��೤��

#define telephone_num_max_lgth 16 //�ֶ�������󳤶�

#define usart1_rec_overtime_max 2000 //���ڽ��ճ�ʱ���ʱ��2��

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//*****************************************************************************
//*****************************************************************************
//-----------		            ��������   	                  -----------------
//-----------------------------------------------------------------------------

//#define     test_debug                      //����ģʽ����
//#define     def_idwt_en                     //ʹ�ܶ������Ź�

//------------------------------//

//#define       dis_all_flag_test       //������ʾ���б�־

//------------------------------//

//#define       jtbb_test_ver       //��ͨ�����԰汾
#define gb19056_ver //��ʻ��¼�ǰ汾

//------------------------------//

//#define       key_no_sound           //������������    //������Խ��鰴��������
//#define       notice_no_sound        //��ʾ������      //�ʺϵ���ʹʹ��

//------------------------------//

//#define      sla_ack_0xa0_comm_en          //�ӻ�Ӧ������A0����ʹ�ܿ���

#define input_telephone_num_en //ʹ���ֶ����Ź���
#define start_print_infor_en   //��ӡ������Ϣ

//#define      spi1_send_data_to_usart1_en     //SPI1�˿ڷ��͵����ݷ�������1
//#define      spi1_rec_data_to_usart1_en      //SPI1�˿ڽ��յ����ݷ��ʹ���1

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//--------------------------------------------------------------
//------------            �������Ͷ���              ------------
//--------------------------------------------------------------

typedef struct
{
    u8 buff[usart_rec_queue_max_lgth]; //���л�����

    u16 head; //����ͷλ��
    u16 tail; //����βλ��
} usart_queue_struct;

typedef struct
{
    usart_queue_struct que;

    bool busy_flag;                           //���ݴ�����ɱ�־
    u8 rec_step;                              //������Ч���ݵĲ���
    u16 cnt;                                  //�������ݼ�����
    u8 v_data[usart_rec_valid_buff_max_lgth]; //��Ч����
    u16 lgth;                                 //���յ���Ч���ݵĳ���

    u32 rec_overtime; //���ճ�ʱ������
} usart_rec_struct;   //���ڽ������ݽṹ��

typedef struct
{
    bool busy;       //æ��־   TRUE��ʾæ   FALSE��ʾ����
    u8 *ptr;         //����������ָ��
    u16 lgth;        //�����������ܳ���
    u16 cnt;         //���������ݼ���
} usart_send_struct; //���ڷ������ݽṹ��

typedef struct
{
    bool active;   //TRUE��ʾ���µİ��������д�����    FALSE��ʾ�����������
    bool lg;       //TRUE��ʾ����              FALSE��ʾ�̰�
    u8 key;        //�������µļ�ֵ
} key_data_struct; //���������ṹ��

typedef struct
{
    u8 para[password_para_max_lgth + 2]; //������������      ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 oil[password_oil_max_lgth + 2];   //������������      ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 car[password_car_max_lgth + 2];   //��������          ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ

    u8 sensor[password_sensor_max_lgth + 2];             //������ϵ������    ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 speed[password_speed_max_lgth + 2];               //�ٶȷ�ʽѡ������  ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 lcd_para_rst[password_lcd_para_rst_max_lgth + 2]; //Һ����������ʼ������  ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ

    //u8  regist[password_regist_max_lgth+2];   //ע����ز�������  ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
} password_manage_struct;

typedef struct
{
    u8 m_ip[para_main_ip_max_lgth + 2];   //��IP /������ ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 b_ip[para_backup_ip_max_lgth + 2]; //����IP  /��������ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ

    u8 m_ip2[para_main_ip_max_lgth + 2];   //�ڶ�·��IP /������ ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 b_ip2[para_backup_ip_max_lgth + 2]; //�ڶ�·����IP  /��������ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ

    u8 id[para_id_max_lgth + 2];            //����ID  ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 apn[para_apn_max_lgth + 2];          //APN  ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 num_s[para_num_server_max_lgth + 2]; //�������  ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ
    u8 num_u[para_num_user_max_lgth + 2];   //�û�����  ǰ�����ֽڱ�ʾ���ȣ��������������ͣ���������ΪASCII��ʽ

    u8 bl_type;      //��������    (0\5\10\30\60\255) ����0��ʾһֱ����255��ʾʡ��ģʽ��������ʾʱ�䵥λΪ��
    u8 buzzer_type;  //����������  (0\1\2)
    u8 LCD_Contrast; //LCD�Աȶ�
} set_para_struct;

typedef struct
{
    u8 province_id[rec_province_id_max_lgth + 2];         //ʡ��ID  ǰ�����ֽڱ�ʾ����
    u8 city_id[rec_city_id_max_lgth + 2];                 //����ID  ǰ�����ֽڱ�ʾ����
    u8 manufacturer_id[rec_manufacturer_id_max_lgth + 2]; //������ID  ǰ�����ֽڱ�ʾ����
    u8 terminal_type[rec_terminal_type_max_lgth + 2];     //�ն��ͺ�  ǰ�����ֽڱ�ʾ����
    u8 terminal_id[rec_terminal_id_max_lgth + 2];         //�ն�ID  ǰ�����ֽڱ�ʾ����
    u8 car_plate[rec_car_plate_max_lgth + 2];             //���ƺ���  ǰ�����ֽڱ�ʾ����
    u8 car_vin[rec_car_vin_max_lgth + 2];                 //����VIN��  ǰ�����ֽڱ�ʾ����
    u8 ratio[rec_para_ratio_max_lgth + 2];                //�ٶȴ�����ϵ��ֵ   HEX���ݸ�ʽ  ǰ�����ֽڱ�ʾ����

    u8 pr_driver; //��ӡ��ʻԱ����  0��ӡ��ʻԱ����	�����ӡ��ʻԱ����

} recorder_para_struct;

typedef struct
{
    u8 set_load_status;     //����װ��״̬   1Ϊ����    2Ϊ����    3����
    u8 set_gps_module_type; //���ö�λģ��ģʽ   1Ϊ��GPS  2Ϊ��BD    3ΪB+D
} host_no_save_para_struct; //����Ҫ������������ò���

typedef struct
{
    u32 bl; //����ʱ�����
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

#define feed_wdt IWDG_ReloadCounter(); //ι������

//*****************************************************************************
//*****************************************************************************
//-----------		        	�������Ͷ���    	          -----------------
//-----------------------------------------------------------------------------

typedef enum {
#if (MO_ZERO_IS_NULL_FUN == 1)
    mo_null_fun = 0, //��λ���˿̽�����ʾ����˵�����
    mo_reset,        //��λ���˿̽�����ʾ����˵�����
#else
    mo_reset = 0, //��λ���˿̽�����ʾ����˵�����
#endif
    mo_standby,        //��������
    mo_gps_infor1,     //GPSȫ��Ϣ����1
    mo_gps_infor2,     //GPSȫ��Ϣ����2
    mo_recorder_state, //��ʻ��¼��״̬����
    mo_check_state,    //״̬������

    mo_print,        //��ӡ����
    mo_lcd_para_rst, //Һ����ز�����ʼ������

    mo_message_window, //��Ϣ���ڽ���

    mo_1st_5th_1st_password, //���뷨�������� ͨ�Ų�������
    mo_1st_5th_4th_password, //���뷨�������� ��·����
    mo_1st_5th_6th_password, //���뷨�������� ��������
    mo_1st_5th_8th_password, //���뷨�������� Һ����ز�����ʼ��
    mo_1st_6th_1st_password, //???????????????????????? ??????????????????????????

    mo_1st_1st_1st_password, //���뷨�������� ������ϵ������
    mo_1st_1st_2nd_password, //���뷨�������� �ٶ�ģʽ

    mo_password_change, //�����޸Ľ���

    mo_1st_5th_1st_1st, //��IP���û���IP����
    mo_1st_5th_1st_3rd, //����ID����ŷ����������
    mo_1st_5th_1st_4th, //APN����

    mo_1st_5th_9th_1st, //ע������ز�����������
    mo_1st_5th_9th_7th, //ע������ز������ƺ�������

    mo_1st_1st_1st_1st, //��������������
    mo_1st_1st_5th_1st, //��ʻԱ��������

    mo_host_para,   //����ͨ�Ų�������
    mo_ic_card_dis, //IC����Ϣ��ʾ����
    //mo_slave_ver_check,   //�ӻ��汾�Ų�ѯ
    mo_csq_vol, //��ѯCSQ�ź�ֵ

    mo_auto_check,   //�Լ���Ϣ
    mo_query_rec,    //��ʻ��¼�ǲ�����ѯ
    mo_query_reg,    //ע�������ѯ
    mo_query_driver, //��ʻԱ������ѯ

    //mo_update_remete,     //Զ����������
    //mo_update_tf_slave,   //TF�������ӻ�
    mo_update_tf_main,  //TF����������
    mo_update_font_lib, //�ֿ���½���

    mo_oil_test,  //��������
    mo_telephone, //�绰�������

    mo_sms_fix_input, //�ϴ��¼�
    mo_sms_vod_input, //��Ϣ�㲥
    mo_sms_ask_input, //����Ӧ��
    mo_sms_display,   //��Ϣ��ʾ����

    mo_phbk_display,       //�绰����ʾ����
    mo_expand_mult_usart,  //��չ��·����usart0:�˵���ʾ
    mo_auto_parameter_cfg, //�Զ��������� 2017-12-6 Lu
    mo_read_rcd_data,      //U�̶�ȡ��¼�����ݽ���
    mo_auto_init,          //�Զ���ʼ������ 2018-3-13
    mo_timeout_driving,
    mo_speed_record,
    mo_id_cfg, //ID�Զ�����
    mo_phone,  //�绰

} menu_other_enum;

typedef struct
{
    u16 menu_1st; //����˵���һ��
    u16 menu_2nd; //����˵��ڶ���
    u16 menu_3rd; //����˵�������
    u16 menu_4th; //����˵����ļ�
    u16 menu_5th; //����˵����弶

    menu_other_enum menu_other; //������˵��б�

    bool menu_flag;    //�˵��и���  TRUE��ʾ�˵��и���   FALSE��ʾ�˵�û����
    bool next_regular; //����˵�����һ���Ƿ��ǹ���˵�  TRUE��ʾΪ����˵�   FALSEΪ������˵�
    u16 last_start;    //��һ�����һ����ʾ�Ĳ˵���λ�ڲ˵��е�λ��
} menu_cnt_struct;

typedef struct {
    // unsigned char buf_1ln[17]; //��һ����ʾ���ݻ����� ��һ���ֽ����ڴ�Ž�����־
    // unsigned char buf_2ln[17]; //�ڶ�����ʾ���ݻ����� ��һ���ֽ����ڴ�Ž�����־
    // unsigned char buf_3ln[17]; //��������ʾ���ݻ����� ��һ���ֽ����ڴ�Ž�����־
    // unsigned char buf_4ln[17]; //��������ʾ���ݻ����� ��һ���ֽ����ڴ�Ž�����־

    unsigned char *p_dis_1ln; //��һ����ʾ����ָ��
    unsigned char *p_dis_2ln; //�ڶ�����ʾ����ָ��
    unsigned char *p_dis_3ln; //��������ʾ����ָ��
    unsigned char *p_dis_4ln; //��������ʾ����ָ��

    unsigned char bg_1ln_pixel; //��һ����ʾ��ʼλ�ã���λΪһ�����ص�
    unsigned char bg_2ln_pixel; //�ڶ�����ʾ��ʼλ�ã���λΪһ�����ص�
    unsigned char bg_3ln_pixel; //��������ʾ��ʼλ�ã���λΪһ�����ص�
    unsigned char bg_4ln_pixel; //��������ʾ��ʼλ�ã���λΪһ�����ص�
} message_window_dis_struct;    //��Ϣ��ʾ������ʾ���ݽṹ��

typedef struct
{
    bool dir_flag;  //�˳���Ĳ˵�   TRUE��ʾ�˳�����һ������˵�   FALSE��ʾ��������
    bool auto_flag; //�Զ��˳���־   TRUE��ʾ�Զ��˳�       FALSE��ʾ���Զ��˳�

    u32 cnt;             //�Զ��˳�ʱ���ʱ��  ��λ1����
    u32 lgth;            //�Զ��˳�ʱ�䳤��    ��λ1����
} menu_auto_exit_struct; //�Զ��˳�

typedef enum {
    rpt_average = 0, //һ������
    rpt_set,         //��������  ������IP��
    rpt_com,         //��������  ���ѯ������
    rpt_more,        //����������  ���ѯע���������
} report_type_enum;

typedef struct
{
    bool en_flag; //���浯������ʹ�ܱ�־
    bool rpt;     //��������	 TRUE����ɹ���ʧ��    FALSE������ʧ��

    report_type_enum dat_type; //���͵���������
    bool ok_flag;              //���ݴ�����ɱ�־    TRUE����ɹ�    FALSE����ʧ��

    const u8 **ptr;   //��ʾ�������ݵ�ָ��
    u32 cnt;          //���浯��ʱ���ʱ��  ��λ1����
    u32 lgth;         //���浯��ʱ�䳤��    ��λ1����
} menu_report_struct; //״̬����

typedef struct
{
    u8 time[3];          //ʱ�����ݣ�ѹ��BCD��ʽ  �ο����˵���ļ�
    u8 lat_data[4];      //γ�����ݣ�ѹ��BCD��ʽ  �ο����˵���ļ�
    u8 long_data[4];     //�������ݣ�ѹ��BCD��ʽ  �ο����˵���ļ�
    u8 speed[2];         //�ٶ����ݣ�ѹ��BCD��ʽ  �ο����˵���ļ�
    u8 direction[2];     //�������ݣ�ѹ��BCD��ʽ  �ο����˵���ļ�
    u8 gps_state;        //GPS״̬�����λ��ʾ��λ״̬ ��5λΪ0��ʾGPS�ٶ�Ϊ1��ʾ�������ٶ� ��5λ��ʾ����������
    u8 gprs_alarm_state; //GPRS������״̬��λ����ο����˵���ļ�
    u8 speed_max;        //��������ֵ
    u8 reser[2];         //�����������ֽ�
    u8 csq_state;        //CSQ״̬  ���λΪ1��ʾSIM������   0��ʾδ���������
    u8 acc_state;        //ACC״̬��λ����ο����˵���ļ�
    u8 lock_car_state;   //����״̬
    u8 date[3];          //����ʱ�����ݣ�ѹ��BCD��ʽ  �ο����˵���ļ�
    u8 oil_data[2];      //�������ݣ�ѹ��BCD��ʽ  �ο����˵���ļ�
    u8 car_state;        //����״̬������ʻ��¼��״̬��λ����ο����˵���ļ�
} gps_data_struct;

typedef struct
{
    bool type;         //Э��汾����  FALSEΪ������Э��汾  TRUEΪ�µİ�������IP�İ汾
    u8 menu_dis_count; //�˵���ʾ������������ʾ״̬ʱ��������

    u8 ip_lgth; //IP��PORT����
    u8 ip[30];  //IP��PORT����  ��ʽΪ  (0"220.231.155.085",8888) 0,220.231.155.085,8888

    u8 apn_lgth; //APN����
    u8 apn[30];  //APN����

    u8 id_num[16];   //ID����         //�����Ԫ��   [0]�洢����
    u8 user_num[16]; //�û�����       //�����Ԫ��   [0]�洢����
    u8 sms_num[16];  //���ŷ������   //�����Ԫ��   [0]�洢����

    u8 ver_date[6]; //�汾������     //�ͺš��汾���ꡢ�¡��ա�ʱ
    u8 speed;       //���ٱ���   //��λΪ����
    u8 outage;      //�ϵ籨��   //��ֵ�����ʾ����
    u8 stop;        //ͣ������   //��λΪ��

    u8 transmit_mode;    //����ģʽ  //0x80��ʾ��Ĭ    0x81��ʾ���㷢��
    u8 transmit_time[2]; //���ͼ��  //��λΪ��

    u8 call_time[2];   //�̻�����ʱ��    //��λΪ��
    u8 answer_time[2]; //�绰����ʱ��    //��λΪ��

    u8 Backup_ip_lgth; //����IP��PORT����
    u8 Backup_ip[30];  //����IP��PORT����  ��ʽΪ  (0"220.231.155.085",8888) 0,220.231.155.085,8888

    u8 ver_date2[25]; //�汾������     //�ͺš��汾���ꡢ�¡��ա�ʱ

    u8 ip3_lgth; //IP��PORT����
    u8 ip3[30];  //IP��PORT����  ��ʽΪ  (0"220.231.155.085",8888) 0,220.231.155.085,8888

} host_state_struct; //����״̬��ѯ���ݽṹ��

typedef struct
{
    u8 gprs_state;  //GPRS״̬
    u8 gsm_state;   //GSM״̬
    u8 gps_state;   //GPS״̬
    u8 board_state; //����״̬
    u8 sim_state;   //SIM��״̬

    u8 hard_state;   //Ӳ�����״̬
    u8 record_state; //��¼��״̬
    u8 serial_state; //����״̬

    u8 speed[2];     //�ٶȴ�����ֵ
    u8 oil[2];       //��������ֵ
    u8 power[2];     //��Դ��ѹֵ
} auto_check_struct; //�Լ����ݰ�

typedef struct
{
    u8 step;
    u16 tim;

    u8 ratio[rec_para_ratio_max_lgth];  //������ϵ��
    u8 speed_type;                      //�ٶ�����
    u8 print_type;                      //��ӡ����
    u8 car_type[rec_car_type_max_lgth]; //��������
} query_rec_struct;                     //��ʻ��¼�ǲ�ѯ���ݽṹ��

typedef struct
{
    u8 step;
    u16 tim;

    u8 province_id[rec_province_id_max_lgth];         //ʡ��ID
    u8 city_id[rec_city_id_max_lgth];                 //����ID
    u8 manufacturer_id[rec_manufacturer_id_max_lgth]; //������ID
    u8 terminal_type[rec_terminal_type_max_lgth];     //�ն��ͺ�
    u8 terminal_id[rec_terminal_id_max_lgth];         //�ն�ID
    u8 car_plate_color;                               //������ɫ
    u8 car_plate[rec_car_plate_max_lgth];             //���ƺ���
    u8 car_vin[rec_car_vin_max_lgth];                 //����VIN��
} query_reg_struct;                                   //ע�������ѯ���ݽṹ��

typedef struct
{
    u8 step;
    u16 tim;
} query_driver_struct; //��ʻԱ������ѯ���ݽṹ��

typedef union {
    host_state_struct host;       //����״̬
    auto_check_struct auto_check; //�Լ�״̬
    query_rec_struct rec;         //��ʻ��¼��
    query_reg_struct regist;      //ע��
    query_driver_struct driver;   //��ʻԱ
} query_data_union;

//------------------------------------

typedef struct
{
    u8 ic_flag : 1;  //IC�������־   TRUE��ʾ�Ѿ�����  FLASH��ʾδ����
    u8 sd_flag : 1;  //SD�������־   TRUE��ʾ�Ѿ�����  FLASH��ʾδ����
    u8 usb_flag : 1; //U�̲����־   TRUE��ʾ�Ѿ�����  FLASH��ʾδ����

    u8 nread_sms : 1;  //δ����Ϣ��־   TRUE��ʾ��δ��  FLASH��ʾû��
    u8 nack_phone : 1; //δ�ӵ绰��־   TRUE��ʾ��δ��  FLASH��ʾû��

    u8 up_flag : 1;   //���б�־   TRUE��ʾ����    FLASH��ʾû��
    u8 down_flag : 1; //���б�־   TRUE��ʾ����  FLASH��ʾû��

    u8 custom_flag : 1; //���������Զ����־
} state_flag_struct;

typedef union {
    state_flag_struct flag;
    u16 word_data;
} state_flag_union;

typedef struct
{
    state_flag_union state; //SD��IC����������־״̬

    u8 custom_buff[18]; //�Զ����������  ��β��־Ϊ0x00  ����ʾ�������ݲ��ó���8������
    u8 job_num[20];     //˾������
} state_data_struct;

//------------------------------------

typedef struct
{
    u8 page_max; //ҳ����
    u8 sig_size; //��Ԫ��С  ������һ���ֽ���ʾASCII��Ҳ�����������ֽ���ʾһ������

    u8 crisis;            //�ٽ�ֵ  Ϊ0ʱ��ʾ�������뷨���������������л�
    u8 front_crisis_size; //�ٽ�ֵǰ�ĵ�Ԫ��С

    const u8 **ptr_lib; //����Դ
} input_lib_struct;     //���뷨�Ŀ��ļ�

typedef struct
{
    input_lib_struct input_lib; //���뷨����Դ(���ļ�)

    u8 page;  //���뷨��ҳ����
    u8 point; //ÿҳ�е�ָ��

    bool encrypt; //��ʾ����״̬    FALSE��ʾ������    TRUE��ʾ����
    u8 lgth;      //������������ַ���������
    u8 cnt;       //������������ַ�����

    const u8 *ptr_dis; //���뷨����������������
    u8 *ptr_dest;      //���뷨���ս���д�뵽��Ŀ���ַ

    u8 buff[60];       //������������ַ�������
} input_method_struct; //���뷨�����ṹ��

//------------------------------------

typedef enum {
    call_free = 0, //����

    call_dial,    //����
    call_out,     //�绰����
    call_in,      //�绰����
    call_connect, //�绰��ͨ
    call_halt,    //�绰�Ҷ�

    call_end, //ͨ������
} phone_step_enum;

typedef struct
{
    phone_step_enum step;                //ͨ������
    u8 buff[telephone_num_max_lgth + 2]; //�绰���뻺����,buff[0]Ϊ���볤�ȣ�buff[1]Ϊ0x00

    u8 dtmf_cnt;                          //����DTMF������
    u8 dtmf_buff[telephone_num_max_lgth]; //����DTMF������
    u8 dtmf_ptr;                          //��ʾDTMFλ��

    u32 tim; //ͨ��ʱ��
} phone_data_struct;

typedef void (*dis_multi_page_input_point)(void); //��ҳ����ʾ��������չ���뷨�������ú���ָ��

//*****************************************************************************
//*****************************************************************************
//-------------------           �������Ͷ���               --------------------
//-----------------------------------------------------------------------------

typedef enum {
    ic_none = 0,  //δ���뿨Ƭ
    ic_error = 1, //��Ƭ�������ʶ����������ݲ����Ϲ涨

    ic_2402 = 2, //24Щ�п�Ƭ
    ic_4442 = 3, //4442ϵ�п�Ƭ
} ic_type_enum;  //IC������

typedef struct
{
    //--------Э��Ԥ������------//
    u8 name[12];    //��ʻԱ����   ���ֹ������ַ�ASCII���ʽ  //���û���ʱ����0x00
    u8 id_card[20]; //��ʻԱ���֤��   ASCII��ʽ   //���û���ʱ����0x00
    //--------------------------//

    //--------Э�鶨�岿��------//
    u8 driv_lic[18];         //��ʻ֤����   ASCII��ʽ  //���û���ʱ����0x00
    u8 driv_lic_validity[3]; //��ʻ֤��Ч��  BCD��ʽ    -��-��-��
    u8 certificate[18];      //��ҵ�ʸ�֤��  ASCII��ʽ  //���û���ʱ����0x00
    //--------------------------//

    //--------Э���׼��չԤ������------//
    u8 license_organ[40];         //��֤��������   ���ֹ������ַ�ASCII���ʽ  //���û���ʱ����0x00
    u8 license_organ_validity[4]; //֤����Ч��   BCD��ʽ    --��-��-��
    u8 work_num[6];               //����   ASCII��ʽ  //���û���ʱ����0x00

    u8 reserve[6]; //���û���ʱ����0x00
    //--------------------------//

    u8 xor ;                  //ǰ���ֽڵ����У��ֵ
} ic_card_save_format_struct; //IC��оƬ�洢���ݸ�ʽ����

typedef struct
{
    u8 work_num_lgth; //����    //�ݶ���󳤶Ȳ�����7�ֽ�  CKP  2014-06-18
    u8 work_num[12];

    u8 name_lgth; //����
    u8 name[12];

    u8 id_card_lgth; //���֤��
    u8 id_card[20];

    u8 driv_lic_lgth; //��ʻ֤
    u8 driv_lic[18];
    u8 driv_lic_validity[3]; //��ʻ֤��Ч����

    u8 certificate_lgth; //��ҵ�ʸ�֤
    u8 certificate[18];

    u8 license_organ_lgth; //��ҵ�ʸ�֤��֤����
    u8 license_organ[40];
    u8 license_organ_validity[4]; //��ҵ�ʸ�֤֤����Ч��
} ic_card_date_struct;            //IC�����ݸ�ʽ

#define call_record_num 30 //ͨ����¼��ÿһ��ļ�¼����

typedef enum {
    call_out_type = 0,  //������¼
    received_call_type, //�ѽӵ绰
    missed_call_type,   //δ�ӵ绰
} call_record_type_enum;

/*

typedef struct
{
  u8 call_record_type;              //�绰����
  u8 call_record_name_lgth;
  u8 call_record_name_buff[phone_book_name_max_lgth];
  u8 call_record_num_lgth;
  u8 call_record_num_buff[phone_book_num_max_lgth];
}
call_record_sub_struct;

*/

#define phone_book_num_max_lgth ((u8)16)  //�绰�����ݵ绰���볤������
#define phone_book_name_max_lgth ((u8)16) //�绰������������������

typedef enum {
    phbk_type_in_only = 0, //���ɺ���
    phbk_type_out_only,    //���ɺ���
    phbk_type_in_out,      //�ɺ������
} phbk_type_enum;          //�绰�����Ͷ���

typedef struct
{
    u8 lgth;                          //�绰���볤��
    u8 buff[phone_book_num_max_lgth]; //�绰��������
} phbk_num_struct;                    //����ṹ��

typedef struct
{
    u8 lgth;                           //�������ݳ���
    u8 buff[phone_book_name_max_lgth]; //��������
} phbk_name_struct;                    //���ֽṹ��

typedef struct
{
    phbk_type_enum type; //�绰����

    phbk_num_struct num;
    phbk_name_struct name;
} phbk_data_struct; //�绰������

//������¼���ѽӵ绰��δ�ӵ绰���ô˽ṹ��
typedef struct
{
    u8 call_record_cnt; //ͨ����¼�еĵ绰��
    phbk_data_struct call_record_buff[call_record_num];
} call_record_struct;

//*****************************************************************************
//*****************************************************************************

#define phone_book_total_max ((u16)1000)               //�绰������������
#define phone_book_index_content_lgth ((u16)16 * 1024) //�����ܳ�������

#define phone_book_index_short_name_max_lgth ((u8)6) //Ŀ¼���ʾ��������

typedef struct
{
    u16 total;       //�洢����
    u16 posi;        //��һ�����ݽ�Ҫ�洢��λ��,��ʱ�绰������ĵ绰�����ļ���
} phbk_count_struct; //�绰������ֵ

typedef u16 phbk_index[phone_book_total_max]; //�������ݽṹ

#define sms_total_max ((u8)80)             //ÿ����Ϣ����������
#define sms_index_content_lgth ((u16)2048) //�����ܳ�������

#define sms_center_single_piece_lgth ((u16)1024) //�������ĵ���(�ı�)��Ϣ�ܳ�������
#define sms_serve_single_piece_lgth ((u16)512)   //����������Ϣ�ܳ�������
#define sms_fix_single_piece_lgth ((u16)512)     //�����̶�(�¼�)��Ϣ�ܳ�������
#define sms_phone_single_piece_lgth ((u16)1024)  //�����ֻ���Ϣ�ܳ�������
#define sms_vod_single_piece_lgth ((u16)512)     //�����㲥��Ϣ�ܳ�������

#define sms_ask_content_lgth ((u8)200)        //���������ܳ�������
#define sms_ask_answer_total_max ((u8)8)      //���ʴ���������
#define sms_ask_answer_content_lgth ((u8)100) //���ʴ��ܳ�������

typedef enum {
    sms_type_fix = 0, //�̶�(�¼�)��Ϣ
    sms_type_ask,     //������Ϣ
    sms_type_vod,     //�㲥��Ϣ
    sms_type_serve,   //������Ϣ
    sms_type_center,  //���ĵ���(�ı�)��Ϣ
    sms_type_phone,   //�ֻ���Ϣ
} sms_type_enum;      //��Ϣ���Ͷ���

typedef enum {
    sms_op_add = 0, //������Ϣ
    sms_op_del,     //ɾ����Ϣ
    sms_op_change,  //�޸���Ϣ
} sms_op_enum;      //��Ϣ�������Ͷ���

typedef struct
{
    u16 lgth;
    u8 dat[sms_center_single_piece_lgth];
} sms_center_content_struct; //�ı���Ϣ

typedef struct
{
    u16 lgth;
    u8 dat[sms_serve_single_piece_lgth];
} sms_serve_content_struct; //������Ϣ

typedef struct
{
    u16 lgth;
    u8 dat[sms_fix_single_piece_lgth];
} sms_fix_content_struct; //�¼���Ϣ

typedef struct
{
    u16 lgth;
    u8 dat[sms_phone_single_piece_lgth];
} sms_phone_content_struct; //�ֻ���Ϣ

typedef struct
{
    u8 dat[70];
    u16 lgth;
} sms_phone_struct; //�ֻ���Ϣ

typedef struct
{
    u16 lgth;
    u8 dat[sms_vod_single_piece_lgth];
} sms_vod_content_struct; //�㲥��Ϣ

typedef struct
{
    u8 ans_cnt;                   //����𰸵��ܸ���
    u16 lgth;                     //���ⳤ��
    u8 dat[sms_ask_content_lgth]; //��������

    struct
    {
        u8 ans_id;                               //��ID
        u16 ans_lgth;                            //�𰸳���
        u8 ans_dat[sms_ask_answer_content_lgth]; //������
    } answer[sms_ask_answer_total_max];
} sms_ask_content_struct; //����Ӧ��

typedef struct
{
    bool read_flag;         //�Ķ���־  TRUE��ʾδ��    FALSE��ʾ�Ѷ�
    u16 type_id_serial_num; //�洢���ԡ�ID�š����͵���Ϣ
    u32 addr;               //��Ϣ���ݵĵ�ַ    ǰ�����ֽڷŶ��ų��ȣ��ӵ�2���ֽ��Ƕ�������
} use_infor_struct;         //ʹ����Ϣ

typedef struct
{
    u16 total; //�Ѵ洢��������

    use_infor_struct use_infor[sms_total_max]; //��ʹ�õ�ַ����Ϣ��

} sms_index_struct; //��Ϣ�����ṹ�壬���е���Ϣ���ͽ��øýṹ���ļ�

typedef union {
    sms_center_content_struct center;
    sms_serve_content_struct serve;
    sms_fix_content_struct fix;
    sms_phone_content_struct phone;
    sms_vod_content_struct vod;
    sms_ask_content_struct ask;
} sms_union_data_union; //��Ϣ����������

typedef struct
{
    sms_type_enum sms_type; //��ǰ��������Ϣ����

    u8 fix_id;     //�¼�ID��
    u8 fix_status; //�¼�����״̬

    u16 ask_num;                          //����Ӧ����ˮ��
    u8 ask_id_point;                      //����Ӧ���ָ��
    u8 ask_ans[sms_ask_answer_total_max]; //���ʴ�ID��

    u8 vod_type;        //��Ϣ�㲥��Ϣ����
    u8 vod_status;      //��Ϣ�㲥����״̬
} sms_up_center_struct; //�ϴ������ĵ���Ϣ��������

/*
typedef struct
{
	u16 rec_len;
	u16 send_len;
	u8  rec_buff[HANDSET_MAX_SIZE];    //�������ݻ�����
	u8  send_buff[HANDSET_MAX_SIZE];   //���������ݻ�����
}
spi_data_struct;       //SPI�ӿ����ݽṹ��
*/

typedef struct
{
    u16 len;
    u16 res;
    u8 buf[HANDSET_MAX_SIZE]; //�������ݻ�����
} SLAVE_RECIVE_DATA_STRUCT;

typedef struct
{
    u16 len;
    u16 res;
    u8 buf[HANDSET_MAX_SIZE]; //�������ݻ�����
} SLAVE_SEND_DATA_STRUCT;

extern SLAVE_RECIVE_DATA_STRUCT slv_rev_data;
extern SLAVE_SEND_DATA_STRUCT slv_send_data;

typedef struct
{
    u32 comm; //���ͻ�Ӧ�������

    u8 sla_comm;  //���ͻ�Ӧ��������
    u8 status;    //���ͻ�Ӧ��ǰ������״̬
} spi_ack_struct; //�ӻ�׼�����͸����������ݻ���Ӧ��

//*****************************************************************************
//*****************************************************************************
//-----------		            ��������   	                  -----------------
//-----------------------------------------------------------------------------

//extern     gbk_unicode_buff_struct  gbk_unicode_buff;  //��������UNICODE��ת��

extern user_time_struct user_time; //�û�ʱ�����
extern u16 wheel_speed;            //�����ٶ�
extern u16 engine_speed;           //������ת��
extern u16 speed_jiffes;
extern u16 average_1km;

extern mult_usart_set_struct mult_usart_set; //����ѡ���Ķ�·����

//extern     spi1_up_data_struct  spi1_up_queue;  //SPI1�����������
//extern     spi_send_struct      spi1_send;      //SPI�ӿڷ���

//extern     usart_send_struct  usart1_send;  //����1����
//extern     usart_send_struct  usart2_send;  //����2����
//extern     usart_rec_struct   usart1_rec;      //����1����
//extern     usart_rec_struct   usart2_rec;      //����2����
//extern     u8  usart1_send_buff[usart_send_buff_max_lgth];  //����1�������ݻ�����
//extern     u8  usart2_send_buff[usart_send_buff_max_lgth];  //����2�������ݻ�����

extern query_data_union query;                     //��ѯ��ṹ
extern gps_data_struct gps_data;                   //GPS�������
extern state_data_struct state_data;               //״̬����
extern set_para_struct set_para;                   //������������
extern recorder_para_struct recorder_para;         //��ʻ��¼�ǲ���
extern host_no_save_para_struct host_no_save_para; //����Ҫ��������ò���

extern key_data_struct key_data;             //��������
extern menu_cnt_struct menu_cnt;             //�˵��ṹ��
extern message_window_dis_struct mw_dis;     //��Ϣ��ʾ�ṹ��
extern menu_auto_exit_struct menu_auto_exit; //�˵��Զ��˳��ṹ��
extern menu_report_struct menu_report;       //�������
extern u16 dis_multi_page_cnt;               //��ҳ��ʾ�������Ѿ���ʾ������

extern input_method_struct input_method;       //���뷨
extern password_manage_struct password_manage; //�������

extern ic_type_enum ic_type;        //IC������
extern ic_card_date_struct ic_card; //IC������

extern buzzer_voice_struct buz_voice;
extern voice_source_struct buz_cus[];
extern buzzer_cus_time_struct buz_cus_time;

extern call_record_struct call_record; //ͨ����¼�������
extern phone_data_struct phone_data;   //�绰�������

extern sms_up_center_struct sms_up_center;               //�ϴ������ĵ���Ϣ��������
extern sms_union_data_union sms_union_data;              //��Ϣ������
extern u8 sound_play_buff[sound_play_buff_max_lgth + 2]; //�����ʶ�������  ǰ�����ֽڱ�ʾ���ȣ���������������

//��չ��·���ڹ��ܱ�������
extern u8 uart_set_ok_flag[3];         //��·������չ����Ĵ������óɹ���־λ��=1��ʾ���óɹ���=0��ʾû�����û�������ʧ��,�����������ú�ġ�*����ʾ
extern u8 extend_uart_cnt;             //��·������չ����Ŀ���չ���ڵĸ���
extern u8 ic_driver_ifo_num;           //��U�̶�ȡ���ļ�ʻԱ��Ϣ��Ŀ 2018-1-6
extern u32 extend_uart_buff[9];        //��·������չ���ջ���
extern u8 tmp_cnt;                     //��·������չ�����uartx��ѡ���ܵĸ���
extern u8 extend_uart_send_bit[7];     //���ڼ�¼��·������չ�����uartx�еĸ���ѡ���λ��־������extend_uart_send_bit[0]��¼���ǵ�1����ʾ���ǵڼ�λ��1����Ԫ��������
extern u8 current_uart_flag[3];        //��·������չ����ĵ�ǰѡ�д��ڵġ�*��ѡ���־λ,�������ڲ�ѯʱ�ġ�*����ʾ
extern u8 tmp_bit_cnt;                 //���ڼ�¼��·������չ�����uartx�еĸ���ѡ���λ��־��ָ��extend_uart_send_bit
extern unsigned char *tmp_uart_dis[3]; //��ʱ���ڵ���ʾ����Ҫ���������ʾ��·������չ�����USART1-3����ʾ

// TF���洢ͼƬʱ����ı�����Ϣ
//extern   u8 time[50];                  //��Ŵ洢ͼƬʱ���GPSʱ��
//extern  u8 full_filename[50];          //�洢ͼƬʱ���ļ���

//ͨ����¼�����������
extern phbk_data_struct phbk_call_in_data;     //�洢��ǰ������绰���ԱȺ󣬴ӵ绰��������ȡ��������Ӧ�ĵ绰���ͣ�������������Ϣ
extern print_auto_data_struct print_auto_data; //�Զ���ӡ����

//������ѯ�����������
extern u8 realm_name_flag; //������־λ��������:  0x01   ;��������:0x02
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//extern spi_data_struct      spi1_data;      //SPI�ӿ����ݽ���
extern spi_ack_struct slv_send_flag;

//extern  bool Clos_Soc_flag ;	//����soc�رձ�־

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
