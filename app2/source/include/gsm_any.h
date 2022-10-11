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
	1.控制和查询GSM_COMMAND_STRUCT和GSM_STATE_STRUCT
	2.参数管理MODEM_CONTEXT_STRUCT
	3.串口数据管理 gsm_rev_buf[], gsm_send_buf[], gprs_send_buf[]
	4.不同模块之间采用文件独立，如gsm_g610.c, gsm_sim340.c
	5.为了方便移植，不能随意直接添加外部变量到GSM.C
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

#define   get_ip_team_max  3    //解析IP的最大组数

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
	u8 addr[24]; //当前IP地址
	u8 apn[24]; //当前APN
	u8 usr[32]; //当前用户信息
	u8 pwd[32]; //密码
	u8 ID[12]; //当前ID
	u8 imei[20]; //模块IMEI 编号
	u8 phone_number[32]; //呼出或呼入的电话号码
	phone_monitor monitor	; //电话监听
	u16 port; //当前端口号
	u8 protocol; //当前是TCP 或UDP 链接
	u8 type; //类型
	u8 source; //保留
	u8 sim_insert; //SIM卡是否有效
	bool update; //更新
	u16 crc;
}MODEM_CONTEXT_STRUCT;
extern MODEM_CONTEXT_STRUCT mx;

/*
1.GSM模块接受外部控制命令接口
*/
typedef enum {
	PC_NULL, //无效
	PC_ANSWER,  //电话应答
	PC_DIAL, //电话拨号
	PC_HANGUP //电话挂机
} PHONE_CMD_STRUCT;

typedef struct{
	bool pwr_down; //模块断电
	bool reset; //模块复位
	bool res;
}GSM_CMD_STRUCT;

typedef struct{
	u8 type; //短信类型
	u8 format; //短信格式
}SMS_CMD_STRUCT;


typedef struct{
	GSM_CMD_STRUCT	 	gsm; 
	PHONE_CMD_STRUCT 	phone; 
	SMS_CMD_STRUCT 		sms; 				
	GPRS_STATE		 	gprs; 
}GSM_COMMAND_STRUCT;
extern GSM_COMMAND_STRUCT gc;


/*
1.GSM模块接受外部状态查询命令接口
*/
typedef struct {
	u8 squality; 	//CSQ 信号强度
	u8 ph_ch; 	//预留
	bool tts_ready; //TTS 语音播报准备就绪
} GSM_BASE_STATE_STRUCT;

typedef enum {
	PS_IDLE,  //电话空闲状态
	PS_RING,  //响铃
	PS_CALL_OUT,  //呼出
	PS_CONNECTING,  //连接
	PS_SPEAKING,  //通话
	PS_ANSWERING //接听
} PHONE_STATE_STRUCT;

typedef struct{
	u8 ring; //有新短信
	bool mode; //TXT 或PDU
}SMS_STATE_STRUCT;

typedef struct{
	SERVER_TYPE server_type; 	//当前连接的是MAIN IP 或BAKUP IP 或远程升级IP
	u8      connect_times;
	bool 	login;  
	bool    quick_first;
	bool    new_step;
	GPRS_STATE stat; 	//GPRS 空闲，可以发送GPRS 数据
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
	bool sms_new; //新短信
	bool cmp_en; //是否是权限号码
	bool is_txt; //是否是文本模式
	u8   res; //保留
	u8 phone[PHONELEN]; //SMS 号码
	u8 msg[MAX_SMS_PDU]; //信息
	u16 len; //长度
	u16 tick; //产生的时间
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

