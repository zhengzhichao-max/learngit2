/**
  ******************************************************************************
  * @file    report.h 
  * @author  TRWY_TEAM
  * @Email     
  * @version V1.0.0
  * @date    2012-07-03
  * @brief   �������
  ******************************************************************************
  * @attention

  ******************************************************************************
*/  
#ifndef	__REPORT_H
#define	__REPORT_H

typedef union{
	u16 reg;
	struct{
		bool dvr_open			:1; //�������
		bool dvr_ack				:1; //Ӧ��
		bool antn_open                 :1;
		bool dvr_link					:1;
		bool res0 					:4;

		bool high1					:1;
		bool high2					:1;
		bool car_box				:1;     //����״̬0��ƽ�ţ�1������
		bool all_box				:1; 
		bool key_s					:1; 	//0��������1����
		bool left_cover				:1; 	//������   0���պϣ�1����
		bool right_cover			:1;	    //0: ���ϣ�1����
		bool car_cover				:1;     //������   0���պϣ�1����

	}b;
}PUBLIC_IO_UNION;


typedef struct
{
	bool time_flag ;


	//u8  acc_off_time;   	//acc�ر�����ػ�ʱ�� ����
	 
    u8  open_tick;
    u8  close_tick;
    u8  delay_time1;   	
    u8  delay_time2;   	
	u8  res1;
	u8  st[2];		//����ʱ��
	u8  et[2];		//����ʱ��
	u8  res[2];
 	u16 acc_off_time ;//acc�ر�����ػ�ʱ�� ����
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
extern  u32   bak_time_tick;                     //ACC =0; =1:ACC ǿ�ƹ�  =2 ǿ�ƿ�
//�ӿں���
extern void stop_rept(void) ;
extern void init_relay_gpio(void);
extern void send_base_report(void);
extern void report_create_task(void);
extern void tired_rept(void);
#endif	/* __REPORT_H */

