#ifndef  __card_scan_h__
#define  __card_scan_h__


#define    card_scan_time_max       ((u16)500)    //500毫秒
#define    ic_card_state_default   ((u8)0x01)  //IC卡接口缺省状态
#define    tf_card_state_default   ((u8)0x01)  //TF卡接口缺省状态


typedef struct {	 
	time_t IC_enter;//插入IC卡的时间
	time_t IC_exit ;//拔出时间
}IC_Time_t ;


extern IC_Time_t IC_Time;


extern void card_scan_init(void);

extern void ic_card_scan(void);
extern void tf_card_scan(void);







#endif
//*****************************************************************************
//*****************************************************************************




