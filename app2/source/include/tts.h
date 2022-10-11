/**
  ******************************************************************************
  * @file    tts.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V1.0.0
  * @date    2012-06-28
  * @brief   语音播报管理
  ******************************************************************************
  * @attention

  ******************************************************************************
*/  
#ifndef	__TTS_H
#define	__TTS_H

#define TTS_PLAY_CUT_OIL_DTAE		"智能限制举升功能已经启动，请注意安全"
#define TTS_PLAY_CUT_OIL_LEN		36
#define TTS_PLAY_OUT_ROAD_DTAE		"您已偏离路线行驶，请注意限速!"
#define TTS_PLAY_OUT_ROAD_LEN		29
#define TTS_SET_LIMIT_CMD			"收到平台限速指令，请注意行驶!"
#define TTS_SET_LIMIT_CMD_LEN		29
#define TTS_SET_POLOGON_CMD			"收到平台设置多边形指令!"
#define TTS_SET_POLOGON_CMD_LEN		23
#define TTS_DEL_ROAD_POLOGON_CMD		"收到平台删除电子围栏指令!"
#define TTS_DEL_ROAD_POLOGON_CMD_LEN		25
#define TTS_SET_ROAD_CMD			"收到平台设置路线指令!"
#define TTS_SET_ROAD_CMD_LEN		21
#define TTS_PLAY_CALI_SPEED_OPEN		"转速校准已打开"
#define TTS_PLAY_CALI_SPEED_CLOSE		"转速校准已关闭"

#define TTS_ALM_MSK					0x0008c00e;		//

typedef struct{
	bool open;	 
	bool ring;	 //不响铃
	u32  tick;   //30分钟

	u8   cnt;    //5次
	u8   iccnt;  //10次  
	u8   ictmr;  //10秒
}tts_alarm_manage;


typedef struct{
	bool new_tts; //新的TTS 信息
	u8 flag; //标志位
	bool playing;	//已经播报
	bool aud_end; //播放结束
	u16 exit_time; //退出时间，根据报告长度来定
	u16 p_tick; //播报时间		
	u16 len; //长度
	u16 n_tick;	//新的TTS 信息时间
	u16 total;
	u16 crc; //校验
}TTS_MANAGE_STRUCT;
extern TTS_MANAGE_STRUCT tts_m;
extern tts_alarm_manage  tts_c;

extern void tts_accopen_voice( void );
/*
TTS 管理函数
*/
extern void tts_alarm_task(void);
extern void tts_ICCard_voice( void );

extern void tts_speed_limit( void );
/*
TTS 接口函数
*/
extern bool write_tts(u8 *str , u16 len);
#endif	/* __TTS_H */

