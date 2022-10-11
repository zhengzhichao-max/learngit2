#ifndef  __buzzer_hdl_h__
#define  __buzzer_hdl_h__






#define      buzzer_mute_flag    ((u8)0xffff)  //����������
#define      buzzer_end_flag     ((u8)0x0000)  //������ֹͣ




typedef enum
{
	s_key  =  0,
	s_short,
	s_long,

	s_sound_1,
	s_sound_2,
	s_sound_3,

	s_sound_cus,  //�Զ������п���
}
buzzer_voice_type_eunm;


typedef struct
{
	uint  voice;  //����
	uint  tim;    //����  //��25����Ϊʱ���׼�����ֵ���Դﵽ1000��
}
voice_source_struct;


typedef struct
{
	const voice_source_struct *ptr;

	bool      end_flag;  //������־    TRUE��ʾ����    FALSE��ʾδ�������ڲ�����
	uint      cnt;    //��25����Ϊʱ���׼�����ֵ���Դﵽ1000��
}
buzzer_voice_struct;


typedef struct
{
    ulong  cycle_cnt;
	ulong  cycle_lgth;

	ulong  all_cnt;
	ulong  all_lgth;

    bool   flag;   //�Զ�����������  TRUE������  FALSE������
	uchar  sta;    //���б�־  0��δ��ʼ����  1��ʼ����
}
buzzer_cus_time_struct;  //�������Զ�������ʱ��




extern void buzzer_init(buzzer_voice_type_eunm type);
extern void buzzer_int_hdl(void);



#endif




