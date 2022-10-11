/**
  ******************************************************************************
  * @file    jt808_type.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   JT808数据类型、默认值等等
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

//用户设置的系统参数
const SYSTERM_CONFIG_STRUCT sys_cfg_default = {
    .speed_mode = enum_speed_mode_pusle,
    .foster_care = 0, //不重启//不复位//不处理
};

const SYSTERM_CONFIG_STRUCT_RANGE sys_cfg_range = {
    .speed_mode = {enum_speed_mode_pusle, enum_speed_mode_gps},
    .foster_care = {0, 9},
};
