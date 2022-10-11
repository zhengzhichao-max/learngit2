#ifndef  __buzzer_hdl_h__
#define  __buzzer_hdl_h__






#define      buzzer_mute_flag    ((u8)0xffff)  //蜂鸣器静音
#define      buzzer_end_flag     ((u8)0x0000)  //蜂鸣器停止




typedef enum
{
	s_key  =  0,
	s_short,
	s_long,

	s_sound_1,
	s_sound_2,
	s_sound_3,

	s_sound_cus,  //自定义鸣叫控制
}
buzzer_voice_type_eunm;


typedef struct
{
	uint  voice;  //音调
	uint  tim;    //节拍  //以25毫秒为时间基准，最大值可以达到1000秒
}
voice_source_struct;


typedef struct
{
	const voice_source_struct *ptr;

	bool      end_flag;  //结束标志    TRUE表示结束    FALSE表示未结束正在播放中
	uint      cnt;    //以25毫秒为时间基准，最大值可以达到1000秒
}
buzzer_voice_struct;


typedef struct
{
    ulong  cycle_cnt;
	ulong  cycle_lgth;

	ulong  all_cnt;
	ulong  all_lgth;

    bool   flag;   //自定义鸣叫任务  TRUE有任务  FALSE无任务
	uchar  sta;    //鸣叫标志  0还未开始鸣叫  1开始鸣叫
}
buzzer_cus_time_struct;  //蜂鸣器自定义鸣叫时间




extern void buzzer_init(buzzer_voice_type_eunm type);
extern void buzzer_int_hdl(void);



#endif




