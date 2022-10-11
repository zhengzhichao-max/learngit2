#ifndef  __card_scan_h__
#define  __card_scan_h__


#define    card_scan_time_max       ((u16)500)    //500����
#define    ic_card_state_default   ((u8)0x01)  //IC���ӿ�ȱʡ״̬
#define    tf_card_state_default   ((u8)0x01)  //TF���ӿ�ȱʡ״̬


typedef struct {	 
	time_t IC_enter;//����IC����ʱ��
	time_t IC_exit ;//�γ�ʱ��
}IC_Time_t ;


extern IC_Time_t IC_Time;


extern void card_scan_init(void);

extern void ic_card_scan(void);
extern void tf_card_scan(void);







#endif
//*****************************************************************************
//*****************************************************************************




