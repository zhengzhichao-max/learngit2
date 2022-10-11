/**
  ******************************************************************************
  * @file    GSM_ANY.H 
  * @author  TRWY_TEAM
  * @Email     
  * @version V1.0.0
  * @date    2013-12-01
  * @brief   Main program body
  ******************************************************************************
  * @attention
	1.���ƺͲ�ѯGSM_COMMAND_STRUCT��GSM_STATE_STRUCT
	2.��������MODEM_CONTEXT_STRUCT
	3.�������ݹ��� gsm_rev_buf[], gsm_send_buf[], gprs_send_buf[]
	4.��ͬģ��֮������ļ���������gsm_g610.c, gsm_sim340.c
	5.Ϊ�˷�����ֲ����������ֱ������ⲿ������GSM.C
  ******************************************************************************
*/  

#ifndef	__GSM_ANY_H
#define	__GSM_ANY_H

#if   GSM == EC20
#define	PWRON_DELAY			(10*1000)
#elif ( GSM == SLM790 )
#define	PWRON_DELAY			(10*1000)
#else 
#error "GSM == ?"
#endif

#define CSQ_V              3
#define CSQ_M              31
#define CSQ_SEND         5

#define   get_ip_team_max  3    //����IP���������

#define UDP					0
#define TCP					1

#define PC					0
#define SERVER				1

#define SMS_PDU				0
#define SMS_TXT				1

#define CPHONEN				3
#define SPHONEN				1

//#define PHONE_SIZE			20

#define IP_TO_MAIN_SERVER			0
#define IP_TO_BAK_SERVER			1

#define SEND_SMS_ASCLL		0
#define SEND_SMS_USC2		1

#define MAX_SMS_TXT		255	///161
#define MAX_SMS_PDU		400	///140




typedef struct{
	bool enter;
}phone_monitor;

typedef struct{
	u8 addr[24]; //��ǰIP��ַ
	u8 apn[24]; //��ǰAPN
	u8 usr[32]; //��ǰ�û���Ϣ
	u8 pwd[32]; //����
	u8 ID[12]; //��ǰID
	u8 imei[20]; //ģ��IMEI ���
	u8 phone_number[32]; //���������ĵ绰����
	phone_monitor monitor	; //�绰����
	u16 port; //��ǰ�˿ں�
	u8 protocol; //��ǰ��TCP ��UDP ����
	u8 type; //����
	u8 source; //����
	u8 sim_insert; //SIM���Ƿ���Ч
	bool update; //����
	u16 crc;
}MODEM_CONTEXT_STRUCT;
extern MODEM_CONTEXT_STRUCT mx;

/*
1.GSMģ������ⲿ��������ӿ�
*/
typedef enum {
	PC_NULL, //��Ч
	PC_ANSWER,  //�绰Ӧ��
	PC_DIAL, //�绰����
	PC_HANGUP //�绰�һ�
} PHONE_CMD_STRUCT;

typedef struct{
	bool pwr_down; //ģ��ϵ�
	bool reset; //ģ�鸴λ
	bool res;
}GSM_CMD_STRUCT;

typedef struct{
	u8 type; //��������
	u8 format; //���Ÿ�ʽ
}SMS_CMD_STRUCT;


typedef struct{
	GSM_CMD_STRUCT	 	gsm; 
	PHONE_CMD_STRUCT 	phone; 
	SMS_CMD_STRUCT 		sms; 				
	GPRS_STATE		 	gprs; 
}GSM_COMMAND_STRUCT;
extern GSM_COMMAND_STRUCT gc;


/*
1.GSMģ������ⲿ״̬��ѯ����ӿ�
*/
typedef struct {
	u8 squality; 	//CSQ �ź�ǿ��
	u8 ph_ch; 	//Ԥ��
	bool tts_ready; //TTS ��������׼������
} GSM_BASE_STATE_STRUCT;

typedef enum {
	PS_IDLE,  //�绰����״̬
	PS_RING,  //����
	PS_CALL_OUT,  //����
	PS_CONNECTING,  //����
	PS_SPEAKING,  //ͨ��
	PS_ANSWERING //����
} PHONE_STATE_STRUCT;

typedef struct{
	u8 ring; //���¶���
	bool mode; //TXT ��PDU
}SMS_STATE_STRUCT;

typedef struct{
	SERVER_TYPE server_type; 	//��ǰ���ӵ���MAIN IP ��BAKUP IP ��Զ������IP
	u8      connect_times;
	bool 	login;  
	bool    quick_first;
	bool    new_step;
	GPRS_STATE stat; 	//GPRS ���У����Է���GPRS ����
	u16     stim;
}GPRS_STATE_STRUCT;


typedef struct{
	GSM_BASE_STATE_STRUCT gsm; 
	PHONE_STATE_STRUCT phone; 
	SMS_STATE_STRUCT sms; 			
	GPRS_STATE_STRUCT gprs1; 	
	GPRS_STATE_STRUCT gprs2; 	
	GPRS_STATE_STRUCT gprs3; 	
}GSM_STATE_STRUCT;

extern GSM_STATE_STRUCT gs;


typedef struct{
	bool sms_new; //�¶���
	bool cmp_en; //�Ƿ���Ȩ�޺���
	bool is_txt; //�Ƿ����ı�ģʽ
	u8   res; //����
	u8 phone[PHONELEN]; //SMS ����
	u8 msg[MAX_SMS_PDU]; //��Ϣ
	u16 len; //����
	u16 tick; //������ʱ��
}SEND_SMS_STRUCT;
extern SEND_SMS_STRUCT sms_s;


extern u32 read_sms_tick;


extern bool play_tts_by_gsm(u8 *str , u16 len);

extern void call_circuit_ctrl(u8 value);

extern void sms_send(u8 type, u8* phone, u8 p_len, u8* msg, u16 m_len);
extern void net_send(u8* strp);
extern void net_send_hex(u8* buf, u16 n, u8 netn );
extern void config_gsm_port(void);
extern void gsm_handle_task(void);
extern void gsm_power_down(void);
extern bool gsm_step_status(void);
extern void tr9_request_reset(void);
extern void gsm_power_off(void);
extern void ex_gsm_power_off(void);

void gsm_see_state(void);

#endif	/* __GSM_ANY_H */

