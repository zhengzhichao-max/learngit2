/**
  ******************************************************************************
  * @file    send.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  所有GPRS数据发送管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#ifndef	__SEND_H
#define	__SEND_H
//队列条数限制
#define RPT_QUE_SIZE				10
#define NET_QUE_SIZE				10
#define HIST_QUE_BASE				0
#define HIST_QUE_SIZE				3000  //仅保存位置数据3000*128/4096 = 93.75 sector

//历史数据指针循环存储条数限制
//该指针在HIST_POINT_ADDR 的SECTOR中循环存储，加长对应FLASH 空间的寿命

#define MAX_INFO_SIZE				1000

#define ACK_TIMEOUT					30

//SPI FLASH 地址分配;注意分配的空间是否够用;不能影响其他SECTOR
#define NET_MIN_ADDR				(u32)(SECT_NET_ADDR*0x1000*1ul)
#define NET_MAM_ADDR				(u32)((SECT_NET_ADDR+SECT_NET_SIZE)*0x1000*1ul)
#define RPT_MIN_ADDR				(u32)(SECT_RPT_ADDR*0x1000*1ul)
#define RPT_MAM_ADDR				(u32)((SECT_RPT_ADDR+SECT_RPT_SIZE)*0x1000*1ul)
#define HIST_POINT_ADDR				(u32)(SECT_H_P_ADDR*0x1000*1ul)
#define HIS_MIN_ADDR				(u32)(SECT_HIS_ADDR*0x1000*1ul)
#define HIS_MAM_ADDR				(u32)((SECT_HIS_ADDR+SECT_HIS_SIZE)*0x1000*1ul)

//每条报告所对应的地址
#define _sect_addr(ptr)				((u32)(ptr*0x1000*1ul))
#define _his_addr(ptr)				(HIS_MIN_ADDR+ptr *128*1uL)
#define _his_point_addr(ptr)			(HIST_POINT_ADDR+ptr*12*1uL)

#define incp_his(ptr) \
	do { \
		if (++ptr >= HIST_QUE_BASE + HIST_QUE_SIZE) \
			ptr = HIST_QUE_BASE; \
	} while (0)

//history pointer over
#define _his_overflow(head_t ,tail_t)	(head_t == tail_t - 1 || head_t == tail_t + HIST_QUE_SIZE - 1)

typedef struct
{
	u16 crc;		
	u16 flag;	
	u16 head; 		//头指针
	u16 tail; 		//尾指针 
	u16 storef; 	//当前发送指针
	u16 storef2; 	//当前发送指针2
}HISTORY_POINT_STRUCT;

typedef struct{
	bool ack;	 //盲区应答
	u8   res;		
	u16 id; //盲区消息ID
	u16 water; //盲区消息流水号
}HISTORY_ACK_MANAGE_STRUCT;
	
typedef struct {
	bool en;		//1:该队列有效 0 : 无效
	bool ack; 		//信息应答
	u8 type; 		//类型
	u8 resend; 		//重发次数
	u16 tick; 		//产生报告时对应的时间
	u16 event; 		//事件ID
	u16 water; 		//流水号
	u16 len; 		//长度
	u32 addr; 		//地址
	bool moving; 	//存储时，是否车辆处于行驶状态，若不是不保存到盲区
	u8  netn;    	//通道号 收到相应通道的应答后；进行处理。
	u8 res[2];	
}report_queue;

typedef struct{
	u8   cnt; //空队列个数
	u8   res;		
	u8 	index; //当前发送的报告索引
	bool ack;	 //平台回复
	u32	n_addr; //下条报告的开始地址	
	report_queue que[RPT_QUE_SIZE];
}REPORT_MANAGE_STRUCT;

extern u16 save_hisp_pos;	

//batch 批量信息，如拍照，录音等

extern u16 cur_send_water;


//接口函数
//net 实时网络数据
extern void init_net_info(void);
extern bool get_net_ack(u16 id_t , u16 water_t, u8 netn);
extern 	bool  add_net_queue(u16 id, u16 wnum, u8 *buf, u16 len, u8 netn );

//report 实时位置信息
extern void init_report_info(void);
extern bool get_reprot_ack(u16 id_t, u16 water_t, u8 netn);
extern bool add_report_queue(u16 id, u16 wnum, u8 *buf, u16 len, u8 netn);


//管理函数
extern void report_manage_task(void);
#endif	/* __SEND_H */

