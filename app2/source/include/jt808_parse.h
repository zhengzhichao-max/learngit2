/**
  ******************************************************************************
  * @file    jt808_parse.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   解析GPRS数据
  ******************************************************************************
  * @attention

  ******************************************************************************
*/
#ifndef __JT808_PARSE_H
#define __JT808_PARSE_H

#define MIM_NET_PACKAGE 5
#define MAX_NET_PACKAGE 1000

#define VIEW_PARA_JTB_MIN 0x0001
#define VIEW_PARA_JTB_MAX 0x009f

#define VIEW_PARA_TR_MIN 0x0100 //需要修改
#define VIEW_PARA_TR_MAX 0x0110 //需要修改

typedef void (*funcp_net)(u16 index, u8 *str, u16 s_len);

//u16  heartbeat = 20 ;	//心跳频率

//【cmd_net】【模块】日志/其他功能，配置
typedef enum {
    cmd_net_flag_ = 0x01,
} cmd_net_flag_enum;

typedef struct {
    const u16 index;
    const funcp_net func;
    const u8 type; //type: 0,hex; 1,str;
    const char *comment;
} cmd_net;

extern bool net_working;

typedef struct {
    u8 *name;
    funcp func;
} cmd_sms;

extern void gprs_parse_task(void);

#endif /* __JT808_PARSE_H */
