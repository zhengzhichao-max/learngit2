/**
  ******************************************************************************
  * @file    sms_parse.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   短信功能解析
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#ifndef	__SMS_RECIVE_H
#define	__SMS_RECIVE_H

#define CHNSM				1						
#define ENGSM				2						
#define OTHERSM				0

#define  ENGLISH				0xbb
#define  CHINESE				0xdd
#define  Ctrl_Z				0x1a
#define  SM_BUFFER_LONG 	400		// 短消息内容的最大长度

#define  NEWMSG 					0x01
#define  LISTMSG					0x02

#define  MANAGEMSGDATA				1
#define  PHONEMSGDATA				2

#define  AUTO_PICK					'0'
#define  HANDSET_PICK				'1'

extern bool sms_format_judge ;  // 2018-1-16 gsm_sim800.c 与 gsm_sim26.c中定义
//extern u16 sms_wakeup_Time ;	//获取短信唤醒时间	//海格功能



typedef struct {
	u8 index;
	u16 unicode;
	u8 name[2];
}province_id;

/*
接口函数
*/

extern u8 Revc_sms_flag ;

extern u8 cmp_number(u8 *p , u8 p_len);
extern void del_phone_code86 ( u8 *p_phonecode);
extern s16 parse_sms(u8* cmdbuf, u8* retbuf, u16 from);
extern void khd_set_card_message(u8 *str, u16 len);

s8 khd_set_ic_card_message(char *str);

#endif	/* __SMS_RECIVE_H */

