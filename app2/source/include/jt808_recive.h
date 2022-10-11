/**
  ******************************************************************************
  * @file    jt808_recive.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   JT808数据接收
  ******************************************************************************
  * @attention

  ******************************************************************************
*/  
#ifndef	__JT808_RECIVE_H
#define	__JT808_RECIVE_H

#include "define.h"

#define _download(ptr)			(ptr * 256*1uL)
#define UP_DATE_FLAG_ADDR		300
#define THE_PACK_NO			119
#define UPDATE_LOC				0x07fe		//EEPROM


extern SEND_MESS_INFOMATION_STRUCT    	rev_inf;		//最新接收消息体

extern bool ack_ext_oil_serve;

extern void jt808_net_parse(u8* p, u16 p_len);
extern u8 get_check_xor(u8 *str, u16 st_addr, u16 end_addr);
extern void rec_send_customer_property(void);

extern u16 get_ztc_status_to_center(u8 *buf);
extern void txt_set_password(u8 * pStr, u16 iStrLen);

#endif	/* __JT808_RECIVE_H */



