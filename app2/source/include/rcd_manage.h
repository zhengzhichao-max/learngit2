/**
  ******************************************************************************
  * @file    rcd_manage.h
  * @author  TRWY_TEAM
  * @Email
  * @version V1.0.0
  * @date    2012-07-03
  * @brief   行驶记录仪功能管理
  ******************************************************************************
  * @attention

  ******************************************************************************
*/
#ifndef	__RCD_MANAGE_H
#define	__RCD_MANAGE_H

#define KEEPCNT   10

#define T_RCD_NULL					0 //无效
#define T_RCD_NO					1 //版本号
#define T_RCD_DRIVER				2 //驾驶员信息
#define T_RCD_TIME					3 //时间
#define T_RCD_MILES					4 //里程
#define T_RCD_PLUS					5 //速度脉冲
#define T_RCD_CAR					6 //车辆信息
#define T_RCD_STATUS				7 //状态
#define T_RCD_UNION					8 //记录仪唯一性编码
#define T_RCD_48					9 //平均速度记录
#define T_RCD_360					10 //位置信息记录
#define T_RCD_ACCIDENT				11 //事故疑点记录
#define T_RCD_DRIVER_OT				12 //超时驾驶记录
#define T_RCD_DRIVER_LOG			13 //驾驶人身份记录
#define T_RCD_POWER_LOG				14 //外部供电记录
#define T_RCD_PARAMETER_LOG			15 //参数修改记录
#define T_RCD_SPEED_LOG				16 //速度状态记录

#define T_RCD_ACCIDENT_CELL			17 //当前事故点记录
#define T_RCD_END					18 //结束


#define RCD_COM_MAX_SIZE			1024 //UART 长度限制
#define RCD_COM_MIN_SIZE			3 //UART 长度限制

#define _retry_addr(ptr)	(u32)		((SECT_RETRY_ADDR*0x1000*1ul)+(ptr *2))

/*
360 小时行驶记录仪 cmd = 0x09H  
每条记录的大小：经纬度8byte+高程2+byte+速度1byte
每分钟保存一条. 
上传数据块包 单位每小时. 所以数据区最小为360个包
flash = 63 * 4 = 252k   
*/
#define RCD360_FILL_NULL				  1
#define RCD360_FILL_INVALID				2
#define RCD360_FILL_NORMAL				3

#define RCD360_START_TIME_SIZE		6   //节点开始记录时间
#define RCD360_EVERY_INF_SIZE			11  //每条记录的大小：经纬度8byte+高程2+byte+速度1byte
#define RCD360_NODE_PAGE_SIZE			6	  //每页保存6条节点数据, 1 page_size 只能保存6个节点.
#define RCD360_P_SAVE_SIZE				100 //保存空间 flash = 100 page * 4096 byte

#define RCD360_HOUR_INF_SIZE			(u16)((RCD360_EVERY_INF_SIZE*60)+RCD360_START_TIME_SIZE)//1个小时=666个字节
#define RCD360_NODE_MAX_SIZE			(u16)(SECT_RCD360_D_SIZE*RCD360_NODE_PAGE_SIZE)	//6*63 = 378个节点
#define RCD360_P_MIN_ADDR				  (u32)(SECT_RCD360_P_ADDR*0x1000*1ul) //RCD360的开始地址。

#define _rcd360_p_addr(ptr)				(u32)(RCD360_P_MIN_ADDR+(ptr *36*1uL))//节点数据保存区
//计算当前时间保存的位置
#define _rcd360_t_addr(ptr)				(u32)((SECT_RCD360_D_ADDR+(ptr/6))*0x1000+((ptr%6)*RCD360_HOUR_INF_SIZE))//获取360时间数据在SPI Flash中的地址
//计算当前数据保存的位置
#define _rcd360_d_addr(ptr1,ptr2)	(u32)((_rcd360_t_addr(ptr1))+RCD360_START_TIME_SIZE+(RCD360_EVERY_INF_SIZE*ptr2))//获取360位置数据在SPI_Flash中的地址



/*
48小时行驶记录仪
minute
*/
#define RCD48_FILL_NULL					1
#define RCD48_FILL_INVALID				2
#define RCD48_FILL_NORMAL				3

#define RCD48_START_TIME_SIZE			6
#define RCD48_EVERY_INF_SIZE			2   //每秒记录一次速度信息 速度信息一个字节,信号(转向灯)状态一个字节
#define RCD48_NODE_PAGE_SIZE			30	//每页保存32条节点数据
#define RCD48_P_SAVE_SIZE				100

#define RCD48_MINUTE_INF_SIZE			(u16)((RCD48_EVERY_INF_SIZE*60)+RCD48_START_TIME_SIZE)
#define RCD48_NODE_MAX_SIZE			(u16)(SECT_RCD48_D_SIZE*RCD48_NODE_PAGE_SIZE)	//100*30 = 3000个节点>>>50小时
#define RCD48_P_MIN_ADDR				(u32)(SECT_RCD48_P_ADDR*0x1000*1ul)

#define _rcd48_p_addr(ptr)				(u32)(RCD48_P_MIN_ADDR+(ptr *36*1uL))//保存节点数据 大小 4K
#define _rcd48_t_addr(ptr)				(u32)((SECT_RCD48_D_ADDR+(ptr/30))*0x1000+((ptr%30)*RCD48_MINUTE_INF_SIZE))//计算时间偏移
#define _rcd48_d_addr(ptr1,ptr2)			(u32)(_rcd48_t_addr(ptr1)+RCD48_START_TIME_SIZE+RCD48_EVERY_INF_SIZE*ptr2)//计算数据偏移

/*
事故疑点分析
*/
#define RCD_ACCIDENT_FILL_NULL			1
#define RCD_ACCIDENT_FILL_INVALID		2
#define RCD_ACCIDENT_FILL_NORMAL		3
#define RCD_ACCIDENT_P_SAVE_SIZE		100

#define RCD_ACCIDENT_NODE_PAGE_SIZE	    16		//每页保存16条节点数据
#define RCD_ACCIDENT_INF_SIZE			234     //每条数据234个字节
#define RCD_ACCIDENT_NODE_MAX_SIZE	(u16)(SECT_RCD_ACCIDENT_D_SIZE*RCD_ACCIDENT_NODE_PAGE_SIZE)	//16*10 = 160个节点
#define RCD_ACCIDENT_P_MIN_ADDR		(u32)(SECT_RCD_ACCIDENT_P_ADDR*0x1000*1ul)

#define ACCI_ADDRP(ptr)					(u32)(RCD_ACCIDENT_P_MIN_ADDR+(ptr *36*1uL))
#define ACCI_ADDRD(ptr)					(u32)((SECT_RCD_ACCIDENT_D_ADDR+(ptr/16))*0x1000+((ptr%16)*RCD_ACCIDENT_INF_SIZE))

/*
超时驾驶记录
*/
#define RCD_OT_FILL_NULL				1
#define RCD_OT_FILL_INVALID				2
#define RCD_OT_FILL_NORMAL				3

#define RCD_OT_P_SAVE_SIZE				100
#define RCD_OT_NODE_PAGE_SIZE			60		//每页保存60条节点数据
#define RCD_OT_INF_SIZE					50
#define RCD_OT_NODE_MAX_SIZE			(u16)(SECT_RCD_OT_D_SIZE*RCD_OT_NODE_PAGE_SIZE)	//60*3 = 180个节点
#define RCD_OT_P_MIN_ADDR				(u32)(SECT_RCD_OT_P_ADDR*0x1000*1ul)

#define RCDOT_ADDRP(ptr)				(u32)(RCD_OT_P_MIN_ADDR+(ptr *36*1uL))
#define RCDOT_ADDRD(ptr)				(u32)((SECT_RCD_OT_D_ADDR+(ptr/60))*0x1000+((ptr%60)*RCD_OT_INF_SIZE))

/*
驾驶人身份记录
*/
#define RCD_DRIVER_FILL_NULL			1
#define RCD_DRIVER_FILL_INVALID			2
#define RCD_DRIVER_FILL_NORMAL			3
#define RCD_DRIVER_P_SAVE_SIZE			100

#define RCD_DRIVER_NODE_PAGE_SIZE		100		//每页保存100条节点数据
#define RCD_DRIVER_INF_SIZE				25
#define RCD_DRIVER_NODE_MAX_SIZE		(u16)(SECT_RCD_DRIVER_D_SIZE*RCD_DRIVER_NODE_PAGE_SIZE)	//100*3 = 300个节点
#define RCD_DRIVER_P_MIN_ADDR			(u32)(SECT_RCD_DRIVER_P_ADDR*0x1000*1ul)

#define DRIVER_ADDRP(ptr)				(u32)(RCD_DRIVER_P_MIN_ADDR+(ptr *36*1uL))
#define DRIVER_ADDRD(ptr)				(u32)((SECT_RCD_DRIVER_D_ADDR+(ptr/100))*0x1000+((ptr%100)*RCD_DRIVER_INF_SIZE))

/*
电源管理记录
*/
#define RCD_POWER_FILL_NULL				1
#define RCD_POWER_FILL_INVALID			2
#define RCD_POWER_FILL_NORMAL			3
#define RCD_POWER_P_SAVE_SIZE				100

#define RCD_POWER_NODE_PAGE_SIZE		100		//每页保存100条节点数据
#define RCD_POWER_INF_SIZE				7
#define RCD_POWER_NODE_MAX_SIZE			(u16)(SECT_RCD_POWER_D_SIZE*RCD_POWER_NODE_PAGE_SIZE)	//60*3 = 180个节点
#define RCD_POWER_P_MIN_ADDR			(u32)(SECT_RCD_POWER_P_ADDR*0x1000*1ul)

#define POWER_ADDRP(ptr)				(u32)(RCD_POWER_P_MIN_ADDR+(ptr *36*1uL))
#define POWER_ADDRD(ptr)				(u32)((SECT_RCD_POWER_D_ADDR+(ptr/100))*0x1000+((ptr%100)*RCD_POWER_INF_SIZE))

/*
参数修改记录 
*/
#define RCD_PARA_FILL_NULL				1
#define RCD_PARA_FILL_INVALID			2
#define RCD_PARA_FILL_NORMAL			3

#define RCD_PARA_P_SAVE_SIZE			100

#define RCD_PARA_NODE_PAGE_SIZE			100		//每页保存100条节点数据
#define RCD_PARA_INF_SIZE				7
#define RCD_PARA_NODE_MAX_SIZE			(u16)(SECT_RCD_PARA_D_SIZE*RCD_PARA_NODE_PAGE_SIZE)	//100*3 = 300个节点
#define RCD_PARA_P_MIN_ADDR				(u32)(SECT_RCD_PARA_P_ADDR*0x1000*1ul)

#define _rcd_para_p_addr(ptr)			(u32)(RCD_PARA_P_MIN_ADDR+(ptr *36*1uL))
#define _rcd_para_t_addr(ptr)			(u32)((SECT_RCD_PARA_D_ADDR+(ptr/100))*0x1000+((ptr%100)*RCD_PARA_INF_SIZE))

//速度偏差记录
#define SPEED_MODE_GPS					1
#define SPEED_MODE_SET					2
#define SPEED_MODE_AUTO					3

#define SPEED_STATE_ERR					0	//错误
#define SPEED_STATE_SET					1	//参数设置，速度校验正确时SPEED_STATE_OK，否则SPEED_STATE_AUTO
#define SPEED_STATE_AUTO				2	//校验偏差大时为自动
#define SPEED_STATE_OK					3	//校验偏差小时为正确

#define RCD_SPEED_FILL_NULL				1
#define RCD_SPEED_FILL_INVALID			2
#define RCD_SPEED_FILL_NORMAL			3

#define RCD_SPEED_P_SAVE_SIZE			100   //50 2022-04-16 modify by hj

#define RCD_SPEED_NODE_PAGE_SIZE		30	//10 2022-04-16 modify by hj	//每页保存10条节点数据
#define RCD_SPEED_INF_SIZE				133
#define RCD_SPEED_NODE_MAX_SIZE			(u16)(SECT_RCD_SPEED_D_SIZE*RCD_SPEED_NODE_PAGE_SIZE)	//10*3 = 30个节点
#define RCD_SPEED_P_MIN_ADDR			(u32)(SECT_RCD_SPEED_P_ADDR*0x1000*1ul)

#define RSPEED_ADDRP(ptr)				(u32)(RCD_SPEED_P_MIN_ADDR+(ptr *36*1uL))
#define RSPEED_ADDRD(ptr)				(u32)((SECT_RCD_SPEED_D_ADDR+(ptr/30))*0x1000+((ptr%30)*RCD_SPEED_INF_SIZE))
#define subp_rcd_p(p1, p2 ,size)		(((p1) >= (p2)) ? (p1) - (p2) : (p1) + (size) - (p2))


typedef struct
{
    s32 lngi;							// Longitude(in degree * 10000000)
    s32 lati;							// Latitude (in degree * 10000000)
} RCD_POINT;


typedef struct
{
    bool touch;
    u32  tmr;
    
}LEFT_RIGHT_IO;


typedef union
{
    u8 reg;
    struct
    {
    	u8 safety_belt 	: 1; //安全带   
	u8 foglight		: 1; //雾灯
	u8 backing		: 1; //倒车	
	u8 near_light		: 1; //近光灯
	u8 far_light		: 1; //远光灯
	u8 right_light	: 1; //右转
	u8 left_light		: 1; //左转
	u8 brake			: 1; //刹车
	   	
       // u8 custom_h2		: 1; //后车门
       // u8 custom_h1		: 1; //前车门

    } b;
} U_RCD_STATUS;
extern U_RCD_STATUS	rcd_io; //行驶记录仪IO状态口//bit7: 刹车bit6: 左转bit5:  右转bit4: 远光灯bit3:近光灯

//360小时位置信息
typedef struct
{
    RCD_POINT	locate; // Latitude, Longitude
    u16 heigh; //高度
    u8 speed; //速度
    u8 res1;	 //存储时，无该位
} S_RCD_360HOUR_INFOMATION;

typedef struct
{
    u16 node; //节索引，计算整数小时个数
    u8 row; //行索引，计算整数分钟个数
    u8 res;
} S_RCD_360HOUR_CELL;

typedef struct  // 20 字节
{
    u16 crc;
    u16 flag;
    S_RCD_360HOUR_CELL head; //头指针
    S_RCD_360HOUR_CELL tail; //尾指针
    time_t h_time; //头指针时间
    u16 e_sect; //当前FLASH
    bool init;
} S_RCD_360HOUR_MANAGE_STRUCT;

//2个日历天平均速度
typedef struct
{
    u8 speed; //速度
    U_RCD_STATUS sig; //信号
} S_RCD48_SPEED_INFOMATION;

typedef struct
{
    u16 node; //节索引，计算整数分钟个数
    u8 row; //行索引，计算整数秒个数
    u8 res;
} S_RCD48_SPEED_CELL;

typedef struct
{
    u16 crc;
    u16 flag;
    S_RCD48_SPEED_CELL head; //头指针
    S_RCD48_SPEED_CELL tail; //尾指针
    time_t h_time; //头指针时间
    u16 e_sect; //当前FLASH
    bool init;
} S_RCD48_SPEED_MANAGE_STRUCT;

//事故疑点
#define RCD_ACCIDENT_CELL_SIZE			150

typedef struct
{
    u8 speed;
    U_RCD_STATUS sig;
} S_RCD_ACCIDENT_INFOMATION;

typedef struct
{
    u16 node; //节索引，计算整数分钟个数
} S_RCD_ACCIDENT_CELL;

typedef struct  // 20 字节
{
    u16 crc;
    u16 flag;
    S_RCD_ACCIDENT_CELL head; //头指针
    S_RCD_ACCIDENT_CELL tail; //尾指针
    u16 e_sect; //当前FLASH
} S_RCD_ACCIDENT_MANAGE_STRUCT;

typedef struct
{
    bool acc; //ACC 状态
    bool moving; //车辆是否行驶
    bool pwr_off; //车辆是否断电
    bool res;
} ACCIDENT_TRI_BAK_STRUCT;

//超时驾驶
typedef struct
{
    u8 speed; //速度
    U_RCD_STATUS sig; //信号
} S_RCD_OT_INFOMATION;

typedef struct
{
    u16 node; //节索引，计算整数分钟个数
} S_RCD_OT_SPEED_CELL;

typedef struct
{
    s32 lngi;	 // Longitude
    s32 lati;	 // Latitude
} point_rcd;

typedef struct
{
    point_rcd	 locate;
    u16 heigh;
} S_RCD_POSINTION_INFOMATION;

typedef struct 		// 20 字节
{
    u16 crc;
    u16 flag;
    S_RCD_OT_SPEED_CELL head; //头指针
    S_RCD_OT_SPEED_CELL tail; //尾指针
    u16 e_sect; //当前FLASH
} S_RCD_OT_MANAGE_STRUCT;

//驾驶员记录
typedef struct
{
    u16 node; //节索引，计算整数分钟个数
} S_RCD_DRIVER_CELL;

typedef struct 		// 20 字节
{
    u16 crc;
    u16 flag;
    S_RCD_DRIVER_CELL head; //头指针
    S_RCD_DRIVER_CELL tail; //尾指针
    u16 e_sect; //当前FLASH
} S_RCD_DRIVER_MANAGE_STRUCT;

//电源管理
typedef struct
{
    u16 node;		//节索引，计算整数分钟个数
} S_RCD_POWER_CELL;

typedef struct 		// 20 字节
{
    u16 crc;
    u16 flag;
    S_RCD_DRIVER_CELL head; //头指针
    S_RCD_DRIVER_CELL tail; //尾指针
    u16 e_sect; //当前FLASH
} S_RCD_POWER_MANAGE_STRUCT;

//参数修改记录(14H)
typedef struct
{
    u16 node; //节索引，计算整数分钟个数
} S_RCD_PARA_CELL;

typedef struct 		// 20 字节
{
    u16 crc;
    u16 flag;
    S_RCD_PARA_CELL head; //头指针
    S_RCD_PARA_CELL tail; //尾指针
    u16 e_sect; //当前FLASH
} S_RCD_PARA_MANAGE_STRUCT;

typedef struct
{
    bool en; //参数修改
    u8 type; //类型
    u16 res; //保留
} RCD_PARA_INFOMATION_STRUCT;

//速度偏差 (15H)
typedef struct
{
	u8 pulse_s ;
	u8 gps_s ;
	
}SPEED_STATE;

#pragma pack(1)	
typedef struct
{
	u8 state;
	time_t s_time ;
	time_t e_time ;
	SPEED_STATE sp_state[60];
}SPEED_TIME;

typedef struct
{
    u16 node; //节索引，计算整数分钟个数
} S_RCD_SPEED_STATE_CELL;

typedef struct 		// 20 字节
{
    u16 crc;
    u16 flag;
    S_RCD_PARA_CELL head; //头指针
    S_RCD_PARA_CELL tail; //尾指针
    bool init;
} S_RCD_SPEED_STATE_MANAGE_STRUCT;


//打印
typedef struct
{
    u8 drive[21];
    time_t start;
    time_t end;
    point_t s_point;
    u16 s_high;
    point_t e_point;
    u16 e_high;
    bool new_en;
    u8 res1;
} RCD_OT_INFOMATION_STRUCT;

extern RCD_OT_INFOMATION_STRUCT  rcd_ot_inf;

#define over_accident_cell() \
	(accident_head == accident_tail-1||accident_head == accident_tail+RCD_ACCIDENT_CELL_SIZE-1)


typedef struct
{
    bool write_ed;
    u8 res;
    u8  driver_lse[21]; //驾驶证号
    time_t  start; //疲劳驾驶开始时间
    time_t  end; //疲劳驾驶结束时间
} s_EEFtigue;

typedef struct
{
    u8    cur_index; //疲劳驾驶的总条数
    s_EEFtigue cell[RCD_PRINT_FATCNTS_MAX_SIZE]; //最大5次
} ALL_FATIGUE_STRUCT;

extern ALL_FATIGUE_STRUCT fatigue_print; //疲劳驾驶信息打印

#define RCD_PRINT_SPEED_MAX_SIZE	15
typedef struct
{
    u8  year;
    u8  month;
    u8  day;
    u8 	hour;
    u8 	minute;
    u16 speed;
} s_HourMinute;

typedef struct
{
    bool fill;
    u8 uCnt;
    s_HourMinute HMt[RCD_PRINT_SPEED_MAX_SIZE];
} s_DOUBTSpeed;
extern s_DOUBTSpeed  	stop_average_speed;	//停车前15分钟平均车速

typedef struct
{
    u8 uCnt;
    s_HourMinute HMt[RCD_PRINT_SPEED_MAX_SIZE];
} AVERAGE_SPEED_STRUCT;


typedef struct
{
    bool ifnew;
    u16  head;
    u16  tail;
} REMEB_POS;



typedef struct
{
    u8   flag;        //导入标志,  = 0 无效;  =1 开始导入 ;  =2导入完成
    u8   inport;      //记录追加标志 =0,清除原来记录; =1, 追加记录, 不清空原有的记录
    u8   cmd;         //导入的CMD
    time_t  start;    //导入开始时间段
    time_t  end;      //导入结束时间段
    u8   speed;       //导入时的参考车速
    u32  lon;         //导入的经度
    u32  lat;         //导入的纬度
    u16  altitude;    //导入的高程
    u8   dr;          //导入的时间方向,  即 当开始时间 > 结束时间 时 dr=0;  当开始时间 < 结束时间 时 dr=1
    u8  ic_driver[20];//导入行驶证号。
    
} RCD_DATA_INPORT_STRUCT;


extern REMEB_POS  pos48;
extern REMEB_POS  pos360;

extern SPEED_STATE  speed_state;
extern AVERAGE_SPEED_STRUCT speed_print;


extern SPEED_TIME speed_message;
extern S_RCD_SPEED_STATE_MANAGE_STRUCT rcd_speed_m;
extern RCD_PARA_INFOMATION_STRUCT rcd_para_inf;
extern S_RCD_PARA_MANAGE_STRUCT rcd_para_m;
extern S_RCD_POWER_MANAGE_STRUCT rcd_power_m;
extern S_RCD_DRIVER_MANAGE_STRUCT rcd_driver_m;
extern S_RCD_ACCIDENT_MANAGE_STRUCT rcd_accident_m;
extern S_RCD48_SPEED_MANAGE_STRUCT rcd48_m;
extern S_RCD_360HOUR_MANAGE_STRUCT rcd360_m;
extern S_RCD_OT_MANAGE_STRUCT rcd_ot_m;

extern s16 incp_rcd_p(u8 type, u16 index , u16 n);
extern s16 decp_rcd_p(u8 type , u16 index , u16 n);

extern void rcd_manage_task(void);
extern void load_rcd_manage_infomation(void);
extern void rcd_manage_init(void);
extern void init_printf_rcd_info(void);

extern bool search_rcd360_pointor(void);
extern bool search_rcd48_pointor(void);
extern bool search_rcd_accident_pointor(void);
extern bool search_rcd_ot_pointor(void);
extern bool search_rcd_driver_pointor(void);
extern bool search_rcd_para_pointor(void);
extern bool search_rcd_power_pointor(void);
void rcd_ot_judge(void);
time_t Cacl_Day_Hour_Sec( time_t stm, u8 pyz );
void rcd_data_inport(void);
time_t Cacl_Day_Hour( time_t stm, u8 pyz );
#endif	/* __RCD_MANAGE_H */

