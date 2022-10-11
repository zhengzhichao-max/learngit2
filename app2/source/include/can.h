#ifndef _CAN_H_
#define _CAN_H_

#if (P_CAN	== _CAN_BASE )

#define MAX_MAIL_NUM   3
#define CAN_DATA_SIZE  1024
//#define CAN_DATA_SIZE  8


typedef void (*funcp_nes)(void);

typedef struct {
	u32 index;
	funcp_nes func;
} cmd_nes;



//CAN_数据采集
typedef struct
{
	bool 	get_way		;			//采集方式
	bool 	data_type	;			//数据类型
	bool	pipe 		;			//通道
	u32		ID_STD		;			//标准帧ID
	u32 	ID_EXT		;			//扩展帧ID
	u32		time		;			//采集时间(ms)	 
}Set_Can_t;


typedef struct
{
	bool CAN1_data_Flag	;
	bool CAN2_data_Flag	;
	//u16  CAN1_count ;
	u32  CAN1_UploadingTime ;		//上传时间
	u32  CAN2_UploadingTime ;		//上传时间
	//u32  CAN1_getTime;
	//u32  CAN2_getTime;

}CAN_param_t;


typedef struct
{
	u32 ID1;
	u32 ID2;

}CAN_ID_FILTRATION_T;

typedef struct
{
	bool CAN1_send_flag;
	bool CAN2_send_flag;
	u8 ID_Quantity;
	

}CAN_PARAM_GET_T;



typedef struct
{  
	u32  can_id;
	u8 	 time[5];	
	u8   data[8];
	
}CAN_DATA_BUF_T;


#define MAX_CAN_BUF 80 //80  2022-03-23 modify by hj
typedef struct
{
    
	//bool chnnal;
	//bool type;
	//bool mode;
 	u16   cnt;          //个数
    //u8 	 reception_time[5];	
   // u32  start_time;
	//u32  can_set_id[32];
    CAN_DATA_BUF_T datbuf[MAX_CAN_BUF];
}CAN_SEND_DATA;
	
typedef union 
{
	u32 reg;	
	struct
	{
		u32 	ID   	:29 ;
		u8		mode	:1 	;	//采集模式  	0是原始数据
		u8		type	:1 	;	//帧类型		0是标准帧
		u8		chnnal	:1 	;	//通道		0是CAN1
	}bit;
	
}CAN_ID_CONFIG_T;


typedef struct
{
	bool lock ;  
	
}SOFTEWARE_LOCK_T;


extern SOFTEWARE_LOCK_T Wofteware_lock;

extern CAN_ID_CONFIG_T Can_id_config;
extern CAN_ID_CONFIG_T Can_id_config2;


extern CAN_SEND_DATA 	cansendbuf ;			//CAN1数据缓存区
extern CAN_SEND_DATA 	cansendbuf2 ;			//CAN2数据缓存区

extern CAN_PARAM_GET_T	Can_param_get;			//一些其他定义
extern CAN_param_t 		Can_param;				//一路CAN数据采集设置参数

extern CanRxMsg RxMessage;
extern CanRxMsg RxMessage2;

extern u8 CAN_msg_num[];   // 发送邮箱标记
extern u8 CAN_msg_num2[];   // 发送邮箱标记
extern u16	can_limit_rabio;



//extern  void 	Can_SetModel(u8 Mode);

extern  void    Set_CAN_ID_WORK(void);
extern  void 	CAN_ID_MANAGE(u32 index);
extern  void   	InitCan( void );
extern  void    InitCan2(void) ;

extern  void Can1_Data_Uploading(void);
extern  void Can2_Data_Uploading(void);


extern	void 	test_can_speed(void);
extern  void   	SendOneMessageCan( void );
extern  void 	can_task_jiffies(void);  //1111
extern  void 	get_wheel_speed_from_can(u8 *str);
extern  void 	get_engine_speed_from_can(u8 *str);
extern  void 	cali_can_speed_task(void);
//extern  void    tr9_Can_Data_manage(u8 bit);
extern  void tr9_CAN_Data_Upload(u8 coun);
extern  void tr9_6037_task_CAN(void);
extern  void Can_SetFilterID_Model(u8 CanNumber,u8 MsgId,u8 Mode,u32 FilterID,u32  MaskID);


#endif

#endif




