

#ifndef __SELF_CHK_H__
#define __SELF_CHK_H__

#include "include_all.h"

typedef enum {
    sdhd_sta_ok = 0,
    sdhd_sta_err,
    sdhd_sta_full,
    sdhd_sta_unknown,
} sdhd_sta_enum;

typedef enum {
    cam_sta_err = 0,
    cam_sta_ok,
    cam_sta_unknown,
} cam_sta_enum;

typedef enum {
    sdhd_staSel_ok_null = 0,
    sdhd_staSel_null_ok,
    sdhd_staSel_ok_ok,
    sdhd_staSel_null_null,
    sdhd_staSel_unknown,
} sdhd_staSel_enum;

int cmd_self_chk(argv_list_st *argv_list);

void selfChk_HD_SD_stateSet(unsigned char who, unsigned char state);
void selfChk_cam_stateSet(unsigned char cnt_cam, unsigned short state);

bool selfChk_ant_open_stateGet(void);
bool selfChk_ant_short_stateGet(void);

sdhd_staSel_enum selfChk_HD_SD_stateGet(void);
cam_sta_enum selfChk_cam_stateGet(void);

#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && defined(MODULE_DEBUG_self_chk)
void selfChk_HD_SD_statePromt(void);
#endif //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && defined(MODULE_DEBUG_self_chk)

#pragma region 通电自检
typedef enum {
    run_led_mode_on_forever = 0,
    run_led_mode_flip_1s,
    run_led_mode_flip3,
    run_led_mode_null,
} run_led_mode_enum;

int selfChk_RunLed_initMode(run_led_mode_enum mode, unsigned char en, unsigned char repeat);
void selfChk_SetTicBoot(void);
void selfChk_SetTic1st1717(void);
void selfChk_do(void);
void selfChk(void);
#pragma endregion 通电自检

#endif //#ifndef __SELF_CHK_H__
