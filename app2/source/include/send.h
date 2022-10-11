/**
  ******************************************************************************
  * @file    send.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ����GPRS���ݷ��͹���
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#ifndef	__SEND_H
#define	__SEND_H
//������������
#define RPT_QUE_SIZE				10
#define NET_QUE_SIZE				10
#define HIST_QUE_BASE				0
#define HIST_QUE_SIZE				3000  //������λ������3000*128/4096 = 93.75 sector

//��ʷ����ָ��ѭ���洢��������
//��ָ����HIST_POINT_ADDR ��SECTOR��ѭ���洢���ӳ���ӦFLASH �ռ������

#define MAX_INFO_SIZE				1000

#define ACK_TIMEOUT					30

//SPI FLASH ��ַ����;ע�����Ŀռ��Ƿ���;����Ӱ������SECTOR
#define NET_MIN_ADDR				(u32)(SECT_NET_ADDR*0x1000*1ul)
#define NET_MAM_ADDR				(u32)((SECT_NET_ADDR+SECT_NET_SIZE)*0x1000*1ul)
#define RPT_MIN_ADDR				(u32)(SECT_RPT_ADDR*0x1000*1ul)
#define RPT_MAM_ADDR				(u32)((SECT_RPT_ADDR+SECT_RPT_SIZE)*0x1000*1ul)
#define HIST_POINT_ADDR				(u32)(SECT_H_P_ADDR*0x1000*1ul)
#define HIS_MIN_ADDR				(u32)(SECT_HIS_ADDR*0x1000*1ul)
#define HIS_MAM_ADDR				(u32)((SECT_HIS_ADDR+SECT_HIS_SIZE)*0x1000*1ul)

//ÿ����������Ӧ�ĵ�ַ
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
	u16 head; 		//ͷָ��
	u16 tail; 		//βָ�� 
	u16 storef; 	//��ǰ����ָ��
	u16 storef2; 	//��ǰ����ָ��2
}HISTORY_POINT_STRUCT;

typedef struct{
	bool ack;	 //ä��Ӧ��
	u8   res;		
	u16 id; //ä����ϢID
	u16 water; //ä����Ϣ��ˮ��
}HISTORY_ACK_MANAGE_STRUCT;
	
typedef struct {
	bool en;		//1:�ö�����Ч 0 : ��Ч
	bool ack; 		//��ϢӦ��
	u8 type; 		//����
	u8 resend; 		//�ط�����
	u16 tick; 		//��������ʱ��Ӧ��ʱ��
	u16 event; 		//�¼�ID
	u16 water; 		//��ˮ��
	u16 len; 		//����
	u32 addr; 		//��ַ
	bool moving; 	//�洢ʱ���Ƿ���������ʻ״̬�������ǲ����浽ä��
	u8  netn;    	//ͨ���� �յ���Ӧͨ����Ӧ��󣻽��д���
	u8 res[2];	
}report_queue;

typedef struct{
	u8   cnt; //�ն��и���
	u8   res;		
	u8 	index; //��ǰ���͵ı�������
	bool ack;	 //ƽ̨�ظ�
	u32	n_addr; //��������Ŀ�ʼ��ַ	
	report_queue que[RPT_QUE_SIZE];
}REPORT_MANAGE_STRUCT;

extern u16 save_hisp_pos;	

//batch ������Ϣ�������գ�¼����

extern u16 cur_send_water;


//�ӿں���
//net ʵʱ��������
extern void init_net_info(void);
extern bool get_net_ack(u16 id_t , u16 water_t, u8 netn);
extern 	bool  add_net_queue(u16 id, u16 wnum, u8 *buf, u16 len, u8 netn );

//report ʵʱλ����Ϣ
extern void init_report_info(void);
extern bool get_reprot_ack(u16 id_t, u16 water_t, u8 netn);
extern bool add_report_queue(u16 id, u16 wnum, u8 *buf, u16 len, u8 netn);


//������
extern void report_manage_task(void);
#endif	/* __SEND_H */

