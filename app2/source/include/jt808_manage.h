/**
  ******************************************************************************
  * @file    jt808_manage.h 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   jt808 Э����ܹ���
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#ifndef	__JT808_MANAGE_H
#define	__JT808_MANAGE_H
#define U8_LEN_ERR					0xff
#define U16_LEN_ERR					0xffff

//#define PWR_CHK_LIST				0x00002010

#define CMD_IGNORE					0
#define CMD_ERROR					1
#define CMD_ACCEPT					2

typedef struct{
	bool rst; //�Ƿ���Ҫִ�и�λ
	bool del; //���������Ƿ��Ѿ�ɾ��
	bool send; //��λ���Żظ��Ƿ��Ѿ�����
	bool res;
	u32  jiff; //ʱ��
}TERMINAL_RESET_STRUCT;

extern TERMINAL_RESET_STRUCT t_rst;

typedef union{
	u8 reg;
	struct{
		u8 GPS_EN		 :1; //GPS ģʽ
		u8 BD_EN		 :1; //����ģʽ
		u8 GLNS_EN		 :1; //GLNS ģʽ
		u8 GLEO_EN		 :1; //GLEO ģʽ
		u8 res			:4;
	}b;
}GNSS_SELECT_UION;

typedef struct{
	u16 crc;
	u16 flag;
	GNSS_SELECT_UION select;	//��λģʽѡ��	
	u8 uart_baud;				//������ѡ��
	u8 out_freq;				//����֡���Ƶ��
	u32 proc_freq;				//���ݲɼ�Ƶ��
	u8 up_mode;					//�����ϴ�ģʽ
	u32 up_para;				//�����ϴ���������
}GNSS_INF_STRUCT;

extern GNSS_INF_STRUCT gnss_cfg;

typedef struct{
	u16 crc;
	u16 flag;
	u32 ch1_gather;		//ͨ��1���ɼ�ʱ��
	u16 ch1_upload;		//ͨ��1���ϴ�ʱ��
	u32 ch2_gather;		//ͨ��2���ɼ�ʱ��
	u16 ch2_upload;		//ͨ��2���ϴ�ʱ��
	u8 alone_gather[8];	//����ID�ɼ�����
}CAN_FANC_INF_STRUCT;
extern CAN_FANC_INF_STRUCT can_cfg;

extern LINK_KEEP_CONFIG_STRUCT link_cfg;
extern IP_SERVER_CONFIG_STRUCT server_cfg; //server_cfg IP���ýṹ�� 
extern DRIVER_CONFIG_STRUCT driver_cfg; //��ʻԱ��Ϣ
extern SEND_MESS_INFOMATION_STRUCT send_info; //���·�����Ϣ?

extern VEHICLE_ALARM_UNION pwr_alm_chk;
extern VEHICLE_ALARM_UNION bypass_pwr_chk;
extern CAR_CONFIG_STRUCT car_cfg; //����ϵ�� 
extern authority_configuration_struct authority_cfg;

#if (P_RCD==RCD_BASE)
extern FACTORY_PARAMETER_STRUCT factory_para;
extern SIGNAL_CONFIG_STRUCT sig_cfg;
extern UNIQUE_ID_STRUCT unique_id;
extern U_BD_RCD_STATUS bd_rcd_io;
#endif

extern bool quick_into_enable;
extern bool quick2_into_enable; //����ע����Ȩ�����õȴ�ʱ��
extern bool imei_flag ;

//extern void power_up_err_check(void);
extern void terminal_login_task(void);
 #endif	/* __JT808_MANAGE_H */

