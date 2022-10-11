/**
  ******************************************************************************
  * @file    jt808_manage.h 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   jt808 协议或功能管理
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
	bool rst; //是否需要执行复位
	bool del; //该条短信是否已经删除
	bool send; //复位短信回复是否已经发送
	bool res;
	u32  jiff; //时间
}TERMINAL_RESET_STRUCT;

extern TERMINAL_RESET_STRUCT t_rst;

typedef union{
	u8 reg;
	struct{
		u8 GPS_EN		 :1; //GPS 模式
		u8 BD_EN		 :1; //北斗模式
		u8 GLNS_EN		 :1; //GLNS 模式
		u8 GLEO_EN		 :1; //GLEO 模式
		u8 res			:4;
	}b;
}GNSS_SELECT_UION;

typedef struct{
	u16 crc;
	u16 flag;
	GNSS_SELECT_UION select;	//定位模式选择	
	u8 uart_baud;				//波特率选择
	u8 out_freq;				//数据帧输出频率
	u32 proc_freq;				//数据采集频率
	u8 up_mode;					//数据上传模式
	u32 up_para;				//数据上传参数设置
}GNSS_INF_STRUCT;

extern GNSS_INF_STRUCT gnss_cfg;

typedef struct{
	u16 crc;
	u16 flag;
	u32 ch1_gather;		//通道1，采集时间
	u16 ch1_upload;		//通道1，上传时间
	u32 ch2_gather;		//通道2，采集时间
	u16 ch2_upload;		//通道2，上传时间
	u8 alone_gather[8];	//单独ID采集设置
}CAN_FANC_INF_STRUCT;
extern CAN_FANC_INF_STRUCT can_cfg;

extern LINK_KEEP_CONFIG_STRUCT link_cfg;
extern IP_SERVER_CONFIG_STRUCT server_cfg; //server_cfg IP设置结构体 
extern DRIVER_CONFIG_STRUCT driver_cfg; //驾驶员信息
extern SEND_MESS_INFOMATION_STRUCT send_info; //最新发送消息?

extern VEHICLE_ALARM_UNION pwr_alm_chk;
extern VEHICLE_ALARM_UNION bypass_pwr_chk;
extern CAR_CONFIG_STRUCT car_cfg; //车辆系数 
extern authority_configuration_struct authority_cfg;

#if (P_RCD==RCD_BASE)
extern FACTORY_PARAMETER_STRUCT factory_para;
extern SIGNAL_CONFIG_STRUCT sig_cfg;
extern UNIQUE_ID_STRUCT unique_id;
extern U_BD_RCD_STATUS bd_rcd_io;
#endif

extern bool quick_into_enable;
extern bool quick2_into_enable; //快速注册或鉴权，不用等待时间
extern bool imei_flag ;

//extern void power_up_err_check(void);
extern void terminal_login_task(void);
 #endif	/* __JT808_MANAGE_H */

