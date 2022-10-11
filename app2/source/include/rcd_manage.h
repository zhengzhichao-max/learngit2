/**
  ******************************************************************************
  * @file    rcd_manage.h
  * @author  TRWY_TEAM
  * @Email
  * @version V1.0.0
  * @date    2012-07-03
  * @brief   ��ʻ��¼�ǹ��ܹ���
  ******************************************************************************
  * @attention

  ******************************************************************************
*/
#ifndef	__RCD_MANAGE_H
#define	__RCD_MANAGE_H

#define KEEPCNT   10

#define T_RCD_NULL					0 //��Ч
#define T_RCD_NO					1 //�汾��
#define T_RCD_DRIVER				2 //��ʻԱ��Ϣ
#define T_RCD_TIME					3 //ʱ��
#define T_RCD_MILES					4 //���
#define T_RCD_PLUS					5 //�ٶ�����
#define T_RCD_CAR					6 //������Ϣ
#define T_RCD_STATUS				7 //״̬
#define T_RCD_UNION					8 //��¼��Ψһ�Ա���
#define T_RCD_48					9 //ƽ���ٶȼ�¼
#define T_RCD_360					10 //λ����Ϣ��¼
#define T_RCD_ACCIDENT				11 //�¹��ɵ��¼
#define T_RCD_DRIVER_OT				12 //��ʱ��ʻ��¼
#define T_RCD_DRIVER_LOG			13 //��ʻ����ݼ�¼
#define T_RCD_POWER_LOG				14 //�ⲿ�����¼
#define T_RCD_PARAMETER_LOG			15 //�����޸ļ�¼
#define T_RCD_SPEED_LOG				16 //�ٶ�״̬��¼

#define T_RCD_ACCIDENT_CELL			17 //��ǰ�¹ʵ��¼
#define T_RCD_END					18 //����


#define RCD_COM_MAX_SIZE			1024 //UART ��������
#define RCD_COM_MIN_SIZE			3 //UART ��������

#define _retry_addr(ptr)	(u32)		((SECT_RETRY_ADDR*0x1000*1ul)+(ptr *2))

/*
360 Сʱ��ʻ��¼�� cmd = 0x09H  
ÿ����¼�Ĵ�С����γ��8byte+�߳�2+byte+�ٶ�1byte
ÿ���ӱ���һ��. 
�ϴ����ݿ�� ��λÿСʱ. ������������СΪ360����
flash = 63 * 4 = 252k   
*/
#define RCD360_FILL_NULL				  1
#define RCD360_FILL_INVALID				2
#define RCD360_FILL_NORMAL				3

#define RCD360_START_TIME_SIZE		6   //�ڵ㿪ʼ��¼ʱ��
#define RCD360_EVERY_INF_SIZE			11  //ÿ����¼�Ĵ�С����γ��8byte+�߳�2+byte+�ٶ�1byte
#define RCD360_NODE_PAGE_SIZE			6	  //ÿҳ����6���ڵ�����, 1 page_size ֻ�ܱ���6���ڵ�.
#define RCD360_P_SAVE_SIZE				100 //����ռ� flash = 100 page * 4096 byte

#define RCD360_HOUR_INF_SIZE			(u16)((RCD360_EVERY_INF_SIZE*60)+RCD360_START_TIME_SIZE)//1��Сʱ=666���ֽ�
#define RCD360_NODE_MAX_SIZE			(u16)(SECT_RCD360_D_SIZE*RCD360_NODE_PAGE_SIZE)	//6*63 = 378���ڵ�
#define RCD360_P_MIN_ADDR				  (u32)(SECT_RCD360_P_ADDR*0x1000*1ul) //RCD360�Ŀ�ʼ��ַ��

#define _rcd360_p_addr(ptr)				(u32)(RCD360_P_MIN_ADDR+(ptr *36*1uL))//�ڵ����ݱ�����
//���㵱ǰʱ�䱣���λ��
#define _rcd360_t_addr(ptr)				(u32)((SECT_RCD360_D_ADDR+(ptr/6))*0x1000+((ptr%6)*RCD360_HOUR_INF_SIZE))//��ȡ360ʱ��������SPI Flash�еĵ�ַ
//���㵱ǰ���ݱ����λ��
#define _rcd360_d_addr(ptr1,ptr2)	(u32)((_rcd360_t_addr(ptr1))+RCD360_START_TIME_SIZE+(RCD360_EVERY_INF_SIZE*ptr2))//��ȡ360λ��������SPI_Flash�еĵ�ַ



/*
48Сʱ��ʻ��¼��
minute
*/
#define RCD48_FILL_NULL					1
#define RCD48_FILL_INVALID				2
#define RCD48_FILL_NORMAL				3

#define RCD48_START_TIME_SIZE			6
#define RCD48_EVERY_INF_SIZE			2   //ÿ���¼һ���ٶ���Ϣ �ٶ���Ϣһ���ֽ�,�ź�(ת���)״̬һ���ֽ�
#define RCD48_NODE_PAGE_SIZE			30	//ÿҳ����32���ڵ�����
#define RCD48_P_SAVE_SIZE				100

#define RCD48_MINUTE_INF_SIZE			(u16)((RCD48_EVERY_INF_SIZE*60)+RCD48_START_TIME_SIZE)
#define RCD48_NODE_MAX_SIZE			(u16)(SECT_RCD48_D_SIZE*RCD48_NODE_PAGE_SIZE)	//100*30 = 3000���ڵ�>>>50Сʱ
#define RCD48_P_MIN_ADDR				(u32)(SECT_RCD48_P_ADDR*0x1000*1ul)

#define _rcd48_p_addr(ptr)				(u32)(RCD48_P_MIN_ADDR+(ptr *36*1uL))//����ڵ����� ��С 4K
#define _rcd48_t_addr(ptr)				(u32)((SECT_RCD48_D_ADDR+(ptr/30))*0x1000+((ptr%30)*RCD48_MINUTE_INF_SIZE))//����ʱ��ƫ��
#define _rcd48_d_addr(ptr1,ptr2)			(u32)(_rcd48_t_addr(ptr1)+RCD48_START_TIME_SIZE+RCD48_EVERY_INF_SIZE*ptr2)//��������ƫ��

/*
�¹��ɵ����
*/
#define RCD_ACCIDENT_FILL_NULL			1
#define RCD_ACCIDENT_FILL_INVALID		2
#define RCD_ACCIDENT_FILL_NORMAL		3
#define RCD_ACCIDENT_P_SAVE_SIZE		100

#define RCD_ACCIDENT_NODE_PAGE_SIZE	    16		//ÿҳ����16���ڵ�����
#define RCD_ACCIDENT_INF_SIZE			234     //ÿ������234���ֽ�
#define RCD_ACCIDENT_NODE_MAX_SIZE	(u16)(SECT_RCD_ACCIDENT_D_SIZE*RCD_ACCIDENT_NODE_PAGE_SIZE)	//16*10 = 160���ڵ�
#define RCD_ACCIDENT_P_MIN_ADDR		(u32)(SECT_RCD_ACCIDENT_P_ADDR*0x1000*1ul)

#define ACCI_ADDRP(ptr)					(u32)(RCD_ACCIDENT_P_MIN_ADDR+(ptr *36*1uL))
#define ACCI_ADDRD(ptr)					(u32)((SECT_RCD_ACCIDENT_D_ADDR+(ptr/16))*0x1000+((ptr%16)*RCD_ACCIDENT_INF_SIZE))

/*
��ʱ��ʻ��¼
*/
#define RCD_OT_FILL_NULL				1
#define RCD_OT_FILL_INVALID				2
#define RCD_OT_FILL_NORMAL				3

#define RCD_OT_P_SAVE_SIZE				100
#define RCD_OT_NODE_PAGE_SIZE			60		//ÿҳ����60���ڵ�����
#define RCD_OT_INF_SIZE					50
#define RCD_OT_NODE_MAX_SIZE			(u16)(SECT_RCD_OT_D_SIZE*RCD_OT_NODE_PAGE_SIZE)	//60*3 = 180���ڵ�
#define RCD_OT_P_MIN_ADDR				(u32)(SECT_RCD_OT_P_ADDR*0x1000*1ul)

#define RCDOT_ADDRP(ptr)				(u32)(RCD_OT_P_MIN_ADDR+(ptr *36*1uL))
#define RCDOT_ADDRD(ptr)				(u32)((SECT_RCD_OT_D_ADDR+(ptr/60))*0x1000+((ptr%60)*RCD_OT_INF_SIZE))

/*
��ʻ����ݼ�¼
*/
#define RCD_DRIVER_FILL_NULL			1
#define RCD_DRIVER_FILL_INVALID			2
#define RCD_DRIVER_FILL_NORMAL			3
#define RCD_DRIVER_P_SAVE_SIZE			100

#define RCD_DRIVER_NODE_PAGE_SIZE		100		//ÿҳ����100���ڵ�����
#define RCD_DRIVER_INF_SIZE				25
#define RCD_DRIVER_NODE_MAX_SIZE		(u16)(SECT_RCD_DRIVER_D_SIZE*RCD_DRIVER_NODE_PAGE_SIZE)	//100*3 = 300���ڵ�
#define RCD_DRIVER_P_MIN_ADDR			(u32)(SECT_RCD_DRIVER_P_ADDR*0x1000*1ul)

#define DRIVER_ADDRP(ptr)				(u32)(RCD_DRIVER_P_MIN_ADDR+(ptr *36*1uL))
#define DRIVER_ADDRD(ptr)				(u32)((SECT_RCD_DRIVER_D_ADDR+(ptr/100))*0x1000+((ptr%100)*RCD_DRIVER_INF_SIZE))

/*
��Դ�����¼
*/
#define RCD_POWER_FILL_NULL				1
#define RCD_POWER_FILL_INVALID			2
#define RCD_POWER_FILL_NORMAL			3
#define RCD_POWER_P_SAVE_SIZE				100

#define RCD_POWER_NODE_PAGE_SIZE		100		//ÿҳ����100���ڵ�����
#define RCD_POWER_INF_SIZE				7
#define RCD_POWER_NODE_MAX_SIZE			(u16)(SECT_RCD_POWER_D_SIZE*RCD_POWER_NODE_PAGE_SIZE)	//60*3 = 180���ڵ�
#define RCD_POWER_P_MIN_ADDR			(u32)(SECT_RCD_POWER_P_ADDR*0x1000*1ul)

#define POWER_ADDRP(ptr)				(u32)(RCD_POWER_P_MIN_ADDR+(ptr *36*1uL))
#define POWER_ADDRD(ptr)				(u32)((SECT_RCD_POWER_D_ADDR+(ptr/100))*0x1000+((ptr%100)*RCD_POWER_INF_SIZE))

/*
�����޸ļ�¼ 
*/
#define RCD_PARA_FILL_NULL				1
#define RCD_PARA_FILL_INVALID			2
#define RCD_PARA_FILL_NORMAL			3

#define RCD_PARA_P_SAVE_SIZE			100

#define RCD_PARA_NODE_PAGE_SIZE			100		//ÿҳ����100���ڵ�����
#define RCD_PARA_INF_SIZE				7
#define RCD_PARA_NODE_MAX_SIZE			(u16)(SECT_RCD_PARA_D_SIZE*RCD_PARA_NODE_PAGE_SIZE)	//100*3 = 300���ڵ�
#define RCD_PARA_P_MIN_ADDR				(u32)(SECT_RCD_PARA_P_ADDR*0x1000*1ul)

#define _rcd_para_p_addr(ptr)			(u32)(RCD_PARA_P_MIN_ADDR+(ptr *36*1uL))
#define _rcd_para_t_addr(ptr)			(u32)((SECT_RCD_PARA_D_ADDR+(ptr/100))*0x1000+((ptr%100)*RCD_PARA_INF_SIZE))

//�ٶ�ƫ���¼
#define SPEED_MODE_GPS					1
#define SPEED_MODE_SET					2
#define SPEED_MODE_AUTO					3

#define SPEED_STATE_ERR					0	//����
#define SPEED_STATE_SET					1	//�������ã��ٶ�У����ȷʱSPEED_STATE_OK������SPEED_STATE_AUTO
#define SPEED_STATE_AUTO				2	//У��ƫ���ʱΪ�Զ�
#define SPEED_STATE_OK					3	//У��ƫ��СʱΪ��ȷ

#define RCD_SPEED_FILL_NULL				1
#define RCD_SPEED_FILL_INVALID			2
#define RCD_SPEED_FILL_NORMAL			3

#define RCD_SPEED_P_SAVE_SIZE			100   //50 2022-04-16 modify by hj

#define RCD_SPEED_NODE_PAGE_SIZE		30	//10 2022-04-16 modify by hj	//ÿҳ����10���ڵ�����
#define RCD_SPEED_INF_SIZE				133
#define RCD_SPEED_NODE_MAX_SIZE			(u16)(SECT_RCD_SPEED_D_SIZE*RCD_SPEED_NODE_PAGE_SIZE)	//10*3 = 30���ڵ�
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
    	u8 safety_belt 	: 1; //��ȫ��   
	u8 foglight		: 1; //���
	u8 backing		: 1; //����	
	u8 near_light		: 1; //�����
	u8 far_light		: 1; //Զ���
	u8 right_light	: 1; //��ת
	u8 left_light		: 1; //��ת
	u8 brake			: 1; //ɲ��
	   	
       // u8 custom_h2		: 1; //����
       // u8 custom_h1		: 1; //ǰ����

    } b;
} U_RCD_STATUS;
extern U_RCD_STATUS	rcd_io; //��ʻ��¼��IO״̬��//bit7: ɲ��bit6: ��תbit5:  ��תbit4: Զ���bit3:�����

//360Сʱλ����Ϣ
typedef struct
{
    RCD_POINT	locate; // Latitude, Longitude
    u16 heigh; //�߶�
    u8 speed; //�ٶ�
    u8 res1;	 //�洢ʱ���޸�λ
} S_RCD_360HOUR_INFOMATION;

typedef struct
{
    u16 node; //����������������Сʱ����
    u8 row; //�������������������Ӹ���
    u8 res;
} S_RCD_360HOUR_CELL;

typedef struct  // 20 �ֽ�
{
    u16 crc;
    u16 flag;
    S_RCD_360HOUR_CELL head; //ͷָ��
    S_RCD_360HOUR_CELL tail; //βָ��
    time_t h_time; //ͷָ��ʱ��
    u16 e_sect; //��ǰFLASH
    bool init;
} S_RCD_360HOUR_MANAGE_STRUCT;

//2��������ƽ���ٶ�
typedef struct
{
    u8 speed; //�ٶ�
    U_RCD_STATUS sig; //�ź�
} S_RCD48_SPEED_INFOMATION;

typedef struct
{
    u16 node; //�������������������Ӹ���
    u8 row; //���������������������
    u8 res;
} S_RCD48_SPEED_CELL;

typedef struct
{
    u16 crc;
    u16 flag;
    S_RCD48_SPEED_CELL head; //ͷָ��
    S_RCD48_SPEED_CELL tail; //βָ��
    time_t h_time; //ͷָ��ʱ��
    u16 e_sect; //��ǰFLASH
    bool init;
} S_RCD48_SPEED_MANAGE_STRUCT;

//�¹��ɵ�
#define RCD_ACCIDENT_CELL_SIZE			150

typedef struct
{
    u8 speed;
    U_RCD_STATUS sig;
} S_RCD_ACCIDENT_INFOMATION;

typedef struct
{
    u16 node; //�������������������Ӹ���
} S_RCD_ACCIDENT_CELL;

typedef struct  // 20 �ֽ�
{
    u16 crc;
    u16 flag;
    S_RCD_ACCIDENT_CELL head; //ͷָ��
    S_RCD_ACCIDENT_CELL tail; //βָ��
    u16 e_sect; //��ǰFLASH
} S_RCD_ACCIDENT_MANAGE_STRUCT;

typedef struct
{
    bool acc; //ACC ״̬
    bool moving; //�����Ƿ���ʻ
    bool pwr_off; //�����Ƿ�ϵ�
    bool res;
} ACCIDENT_TRI_BAK_STRUCT;

//��ʱ��ʻ
typedef struct
{
    u8 speed; //�ٶ�
    U_RCD_STATUS sig; //�ź�
} S_RCD_OT_INFOMATION;

typedef struct
{
    u16 node; //�������������������Ӹ���
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

typedef struct 		// 20 �ֽ�
{
    u16 crc;
    u16 flag;
    S_RCD_OT_SPEED_CELL head; //ͷָ��
    S_RCD_OT_SPEED_CELL tail; //βָ��
    u16 e_sect; //��ǰFLASH
} S_RCD_OT_MANAGE_STRUCT;

//��ʻԱ��¼
typedef struct
{
    u16 node; //�������������������Ӹ���
} S_RCD_DRIVER_CELL;

typedef struct 		// 20 �ֽ�
{
    u16 crc;
    u16 flag;
    S_RCD_DRIVER_CELL head; //ͷָ��
    S_RCD_DRIVER_CELL tail; //βָ��
    u16 e_sect; //��ǰFLASH
} S_RCD_DRIVER_MANAGE_STRUCT;

//��Դ����
typedef struct
{
    u16 node;		//�������������������Ӹ���
} S_RCD_POWER_CELL;

typedef struct 		// 20 �ֽ�
{
    u16 crc;
    u16 flag;
    S_RCD_DRIVER_CELL head; //ͷָ��
    S_RCD_DRIVER_CELL tail; //βָ��
    u16 e_sect; //��ǰFLASH
} S_RCD_POWER_MANAGE_STRUCT;

//�����޸ļ�¼(14H)
typedef struct
{
    u16 node; //�������������������Ӹ���
} S_RCD_PARA_CELL;

typedef struct 		// 20 �ֽ�
{
    u16 crc;
    u16 flag;
    S_RCD_PARA_CELL head; //ͷָ��
    S_RCD_PARA_CELL tail; //βָ��
    u16 e_sect; //��ǰFLASH
} S_RCD_PARA_MANAGE_STRUCT;

typedef struct
{
    bool en; //�����޸�
    u8 type; //����
    u16 res; //����
} RCD_PARA_INFOMATION_STRUCT;

//�ٶ�ƫ�� (15H)
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
    u16 node; //�������������������Ӹ���
} S_RCD_SPEED_STATE_CELL;

typedef struct 		// 20 �ֽ�
{
    u16 crc;
    u16 flag;
    S_RCD_PARA_CELL head; //ͷָ��
    S_RCD_PARA_CELL tail; //βָ��
    bool init;
} S_RCD_SPEED_STATE_MANAGE_STRUCT;


//��ӡ
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
    u8  driver_lse[21]; //��ʻ֤��
    time_t  start; //ƣ�ͼ�ʻ��ʼʱ��
    time_t  end; //ƣ�ͼ�ʻ����ʱ��
} s_EEFtigue;

typedef struct
{
    u8    cur_index; //ƣ�ͼ�ʻ��������
    s_EEFtigue cell[RCD_PRINT_FATCNTS_MAX_SIZE]; //���5��
} ALL_FATIGUE_STRUCT;

extern ALL_FATIGUE_STRUCT fatigue_print; //ƣ�ͼ�ʻ��Ϣ��ӡ

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
extern s_DOUBTSpeed  	stop_average_speed;	//ͣ��ǰ15����ƽ������

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
    u8   flag;        //�����־,  = 0 ��Ч;  =1 ��ʼ���� ;  =2�������
    u8   inport;      //��¼׷�ӱ�־ =0,���ԭ����¼; =1, ׷�Ӽ�¼, �����ԭ�еļ�¼
    u8   cmd;         //�����CMD
    time_t  start;    //���뿪ʼʱ���
    time_t  end;      //�������ʱ���
    u8   speed;       //����ʱ�Ĳο�����
    u32  lon;         //����ľ���
    u32  lat;         //�����γ��
    u16  altitude;    //����ĸ߳�
    u8   dr;          //�����ʱ�䷽��,  �� ����ʼʱ�� > ����ʱ�� ʱ dr=0;  ����ʼʱ�� < ����ʱ�� ʱ dr=1
    u8  ic_driver[20];//������ʻ֤�š�
    
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

