/**
  ******************************************************************************
  * @file    jt808_package.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   协议封装
  ******************************************************************************
  * @attention

  ******************************************************************************
*/  
#ifndef __JT808_PACKAGE_H
#define __JT808_PACKAGE_H

#include "define.h"

/* Includes ------------------------------------------------------------------*/
#define CMD_UP_COMM_ANSWER				0x0001		//终端通用应答
#define CMD_UP_HEART_BEAT				0x0002		//心跳
#define CMD_UP_LOGOUT					0x0003
#define CMD_UP_LOGIN					0x0100
#define CMD_DN_LOGIN					0x8100
#define CMD_UP_RELOGIN					0x0102
#define CMD_UP_AUTO_REPORT				0x0200 //0x00
#define CMD_DN_ANSWER					0x8001

#define CMD_DN_SET_PARA					0x8103
#define CMD_DN_ASK_PARA					0x8104
#define CMD_UP_ASK_PARA					0x0104
#define CMD_UP_GET_PROPERTY				0x0107

#define CMD_DN_UPDATE					0x8180
#define CMD_UP_UPDATE					0x0180

#define CMD_DN_ASK_POS					0x8201
#define CMD_UP_ASK_POS					0x0201 //位置信息查询应答
#define CMD_DN_POS_TRACE				0x8202

#define CMD_DN_SMS_TXT					0x8300
#define CMD_DN_EVENT_SET				0x8301
#define CMD_UP_EVENT_REP				0x0301
#define CMD_DN_QUESTION					0x8302 //提问下发
#define CMD_UP_QUESTION					0x0302 //提问应答
#define CMD_DN_MENU_SET					0x8303
#define CMD_UP_MENU_SET					0x0303
#define CMD_DN_SMS_SERVE				0x8304

#define CMD_DN_TEL_BACK					0x8400 //电话回拨
#define CMD_DN_TEL_SET					0x8401 //电话回拨

#define CMD_DN_CAR_CTRL					0x8500 //车辆控制
#define CMD_UP_CAR_CTRL					0x0500 //车辆控制
#define CMD_DN_OIL_CTRL					0x8580
#define CMD_UP_OIL_CTRL					0x0580

#define CMD_DN_SET_ROUND				0x8600 //设置圆形区域
#define CMD_DN_DEL_ROUND				0x8601 //删除圆形区域
#define CMD_DN_SET_RECTA				0x8602 //设置距形区域
#define CMD_DN_DEL_RECTA				0x8603 //删除距形区域
#define CMD_DN_SET_POLYGON				0x8604 //设置多边形区域
#define CMD_DN_DEL_POLYGON				0x8605 //删除多边形区域 
#define CMD_DN_SET_ROUTE				0x8606 //设置路线
#define CMD_DN_DEL_ROUTE				0x8607 //删除路线

#define CMD_DN_RUN_RECORD				0x8700 //行驶记录
#define CMD_UP_RUN_RECORD				0x0700 //行驶记录

#define CMD_DN_RUN_PARA					0x8701 //行驶参数下传
#define CMD_UP_RDR_REP					0x0700		
#define CMD_UP_E_AWB					0x0701 //单子运单上报

#define CMD_DOWN_DRIVER_STATE			0x8702 //登签
#define CMD_UP_DRIVER_IN				0x0702 //登签
#define CMD_UP_DRIVER_OUT				0x0780 //退签

#define CMD_UP_HISTORY					0x0704 //批量数据上传
#define CMD_UP_BLIND					0x0782 //盲区数据 

#define CMD_UP_MEDIA_EVENT				0x0800 //报警触发事件
#define CMD_UP_MEDIA_DATA				0x0801 //多媒体数据上传
#define CMD_DN_MEDIA_DATA				0x8800
#define CMD_DN_CAMERA_PHOTO				0x8801
#define CMD_DN_MEDIA_SEARCH				0x8802
#define CMD_UP_MEDIA_SEARCH				0x0802
#define CMD_DN_MEDIA_UPLOAD				0x8803
#define CMD_DN_TAPE_START				0x8804
#define CMD_DN_AUTO_PHOTO				0x8880
#define CMD_UP_CAMERA_PHOTO_ANS  		0x0805

#define CMD_DN_DATA_PASS				0x8900
#define CMD_UP_DATA_PASS				0x0900
#define CMD_UP_DATA_ZIP					0x0901

#define CMD_DN_RSA_KEY					0x8a00
#define CMD_UP_RSA_KEY					0x0a00

///渣土车协议
#define CMD_UP_ZTC_STATUS_REPORT		0x0F01
#define CMD_UP_ZTC_EVENT_REPORT			0x0F02
#define CMD_UP_ZTC_SELF_CHECK			0x0F03




extern u16 run_no ;
extern void init_send_info(void);
extern void pack_general_answer(u8 rst);
extern u16 pack_gps_base_info(u8 *p);
extern void pack_register_info(u8 link);
extern void pack_any_data(u16 uId, u8 *buf, u16 len, u16 type, u8 netn );


#endif 
//__JT808_PACKAGE_H

