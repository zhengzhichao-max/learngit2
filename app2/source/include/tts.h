/**
  ******************************************************************************
  * @file    tts.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V1.0.0
  * @date    2012-06-28
  * @brief   ������������
  ******************************************************************************
  * @attention

  ******************************************************************************
*/  
#ifndef	__TTS_H
#define	__TTS_H

#define TTS_PLAY_CUT_OIL_DTAE		"�������ƾ��������Ѿ���������ע�ⰲȫ"
#define TTS_PLAY_CUT_OIL_LEN		36
#define TTS_PLAY_OUT_ROAD_DTAE		"����ƫ��·����ʻ����ע������!"
#define TTS_PLAY_OUT_ROAD_LEN		29
#define TTS_SET_LIMIT_CMD			"�յ�ƽ̨����ָ���ע����ʻ!"
#define TTS_SET_LIMIT_CMD_LEN		29
#define TTS_SET_POLOGON_CMD			"�յ�ƽ̨���ö����ָ��!"
#define TTS_SET_POLOGON_CMD_LEN		23
#define TTS_DEL_ROAD_POLOGON_CMD		"�յ�ƽ̨ɾ������Χ��ָ��!"
#define TTS_DEL_ROAD_POLOGON_CMD_LEN		25
#define TTS_SET_ROAD_CMD			"�յ�ƽ̨����·��ָ��!"
#define TTS_SET_ROAD_CMD_LEN		21
#define TTS_PLAY_CALI_SPEED_OPEN		"ת��У׼�Ѵ�"
#define TTS_PLAY_CALI_SPEED_CLOSE		"ת��У׼�ѹر�"

#define TTS_ALM_MSK					0x0008c00e;		//

typedef struct{
	bool open;	 
	bool ring;	 //������
	u32  tick;   //30����

	u8   cnt;    //5��
	u8   iccnt;  //10��  
	u8   ictmr;  //10��
}tts_alarm_manage;


typedef struct{
	bool new_tts; //�µ�TTS ��Ϣ
	u8 flag; //��־λ
	bool playing;	//�Ѿ�����
	bool aud_end; //���Ž���
	u16 exit_time; //�˳�ʱ�䣬���ݱ��泤������
	u16 p_tick; //����ʱ��		
	u16 len; //����
	u16 n_tick;	//�µ�TTS ��Ϣʱ��
	u16 total;
	u16 crc; //У��
}TTS_MANAGE_STRUCT;
extern TTS_MANAGE_STRUCT tts_m;
extern tts_alarm_manage  tts_c;

extern void tts_accopen_voice( void );
/*
TTS ������
*/
extern void tts_alarm_task(void);
extern void tts_ICCard_voice( void );

extern void tts_speed_limit( void );
/*
TTS �ӿں���
*/
extern bool write_tts(u8 *str , u16 len);
#endif	/* __TTS_H */

