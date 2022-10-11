/**
  ******************************************************************************
  * @file    report.h 
  * @author  TRWY_TEAM
  * @Email     
  * @version V1.0.0
  * @date    2012-07-03
  * @brief   报告管理
  ******************************************************************************
  * @attention

  ******************************************************************************
*/  
#ifndef	__REPORT_H
#define	__REPORT_H

typedef union{
	u16 reg;
	struct{
		bool dvr_open			:1; //深度休眠
		bool dvr_ack				:1; //应答
		bool antn_open                 :1;
		bool dvr_link					:1;
		bool res0 					:4;

		bool high1					:1;
		bool high2					:1;
		bool car_box				:1;     //车厢状态0：平放；1：举升
		bool all_box				:1; 
		bool key_s					:1; 	//0：上锁；1：打开
		bool left_cover				:1; 	//环保盖   0：闭合；1：打开
		bool right_cover			:1;	    //0: 盖上；1：打开
		bool car_cover				:1;     //环保盖   0：闭合；1：打开

	}b;
}PUBLIC_IO_UNION;


typedef struct
{
	bool time_flag ;


	//u8  acc_off_time;   	//acc关闭延伸关机时间 分钟
	 
    u8  open_tick;
    u8  close_tick;
    u8  delay_time1;   	
    u8  delay_time2;   	
	u8  res1;
	u8  st[2];		//启动时间
	u8  et[2];		//结束时间
	u8  res[2];
 	u16 acc_off_time ;//acc关闭延伸关机时间 分钟
	u16 crc;
	u16 flag;
}DVRTimeSet;



extern u16 Delay_detection ;


extern PUBLIC_IO_UNION pub_io;
extern   DVRTimeSet  s_dvr_time;
extern REPORT_CONFIG_STRUCT  report_cfg;
extern ALARM_CONFIG_STRUCT  alarm_cfg;
extern TMP_TRACE_CONFIG_STRUCT tmp_trace_cfg;
extern u8 Night_Speed_Task( void  );
extern  u32   bak_time_tick;                     //ACC =0; =1:ACC 强制关  =2 强制开
//接口函数
extern void stop_rept(void) ;
extern void init_relay_gpio(void);
extern void send_base_report(void);
extern void report_create_task(void);
extern void tired_rept(void);
#endif	/* __REPORT_H */

