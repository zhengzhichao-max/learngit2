/**
  ******************************************************************************
  * @file    handset.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  调度屏
  ******************************************************************************
  * @attention
  ******************************************************************************
*/   
#ifndef	__HANDSET_H
#define	__HANDSET_H

#define HANDSET_MAX_SIZE			600
#define HANDSET_MIN_SIZE			3

#define UPDATE_FROM_SD				0xC5 //SD卡升级数据 

#define H_LEN_I						0 //IDLE 无效
#define H_LEN_N						1 // 无长度
#define H_LEN_W						2 //有长度
#define H_LEN_E						3 //END 无效


typedef void (*funcp_f)(u8 from, u8 *str, u16 len);

typedef struct {
	u8 index;
	funcp_f func;
} cmd_handset;


typedef struct
{
    u8 uGprsU;
    u8 uGsmU;
    u8 uGpsU;
    u8 uMainBoardU;
    u8 uSimCardU;
    u8 uHardU;
    u8 uRecordU;
    u8 uComU;
    u16 speed;
    u16 uiOil;
    u16 uiVoltage;
    u8 uSoftVer[5];
} S_Self_Test;
extern S_Self_Test self_test;


extern u8  test_speed ;



extern uc8 HAnsData[]; //正确接收到手柄数据应答数据
extern uc8 hd_phone_off[]; //当呼叫结束或通话结束返回该信令到手柄
extern uc8 hd_phone_on[];
extern bool ic_driver_lisence; 
#if(0)
extern u8   sHi3520Ver[];
#endif


extern void handle_send_base_msg(u8 from_t, u8 id_t, u8* str_t , u16 len_t);
extern void handle_send_ext_msg(u8 from_t, u16 id_t, u8 *str_t, u16 len_t);
extern void handle_down_sms(u8 from_t, u8 type_t, u8* phone_t, u16 p_len, u8* str_t, u16 s_len_t);
extern u8 handset_any_parse(u8 from_t, u8 *str_t, u16 len_t);
extern void handle_send_0xa0_info(u8 from_t);
extern void send_handset_data(u8 from_t, u8 type_t, u8 resend_t, u8 *buf_t, u16 b_len_t);

#ifdef USING_HANDSET
extern void handset_parse_proc(void);
#endif

extern void init_self_test(void);

#endif	/* __HANDSET_H */

