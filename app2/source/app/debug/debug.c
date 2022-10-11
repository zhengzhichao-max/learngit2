/**
  ******************************************************************************
  1、-$$para  导出设置的参数信息
  2、-$$speed 设置超速报警阀值
  3、-$$gps stop 模拟命令打开不再更新卫星的数据    
  4、-$$gps open 继续打开卫星数据更新
  5、-$$pos 0x022F9B7F,0x06FB332A,0x64	更新到当前的经纬度;速度100码

  ******************************************************************************
**/
#include "include_all.h"

#if (0)
/*********************************************************
名    称：parse_debug_func
功    能：调试函数
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
void parse_debug_func(u8 *cmdbuf, u16 len) {
}

/****************************************************************************
* 名称:    debug_parse_proc ()
* 功能：调试信息的控制界面
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void debug_parse_proc(void) {
}

//////////////////////////////////////////////////////////////////////////
//    工装测试程序
//////////////////////////////////////////////////////////////////////////
void fixture_read_com(u8 from) {
}

//工厂测试串口控制
void factory_uart_test(void) {
}

u32 factory_read_io(void) {
    return 0;
}

//////////////////////////////////////////////////////////
void factory_function_process(void) {
}

void ansys_factory_bug(void) {
}

void ansys_factory_success(void) {
}
#endif

/*********************************************************

                           测试工装版本V1.00
1、上电后10 秒内判断是否进入工装模式
2、进入工装模式后；factory_test.tst 
*********************************************************/
void debug_task(void) {
#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && 1
    if (1) {
        static u8 tic = 0;

        if (++tic % TIC_USE_DEBUG_FUNCTION != 0) {
            return;
        }

        tic = 0;
        gsm_see_state();
    }
#endif

#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && 0
    if (1) {
        static u8 tic = 0;

        if (++tic % TIC_USE_DEBUG_FUNCTION != 0) {
            return;
        }

        tic = 0;
    }
#endif //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION)

#pragma region //硬盘 + SD卡//自检功能
#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && MODULE_DEBUG_self_chk && 1
    selfChk_HD_SD_statePromt();
#endif            //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && MODULE_DEBUG_self_chk
#pragma endregion //硬盘 + SD卡//自检功能

#pragma region //天线断开 + 天线短路
#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && 0
    // car_alarm.bit.sos = !car_alarm.bit.sos;
    logf("undef, reg, 0x%08X", car_alarm.reg);
#endif            //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION)
#pragma endregion //天线断开 + 天线短路

#pragma region //编译时间 + 速度模式 + 系统看护
#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && 0
    promt_time(&mix.time, __func__, NULL);

    logdNoNewLine("BUILD_TIME<%s>, ", str_app_build_time);

    //220828速度类型:
    if (sys_cfg.speed_mode <= enum_speed_mode_null) {
        logdNoTimeNoNewLine("speed_mode, err <%d>, too small, ", sys_cfg.speed_mode);
    } else if (sys_cfg.speed_mode >= enum_speed_mode_cnt) {
        logdNoTimeNoNewLine("speed_mode, err <%d>, too large, ", sys_cfg.speed_mode);
    } else {
        extern const u8 *menu_1st_1st_2nd_dis[];
        logdNoTimeNoNewLine("speed_mode<%s>, ", menu_1st_1st_2nd_dis[menu_line(sys_cfg.speed_mode)]);
    }

    //RST-RK:系统看护//重启RK功能
    if (sys_cfg.foster_care >= 0 && sys_cfg.foster_care <= 9) {
        extern const u8 *menu_1st_7th_1st_dis_val[];
        logdNoTimeNoNewLine("foster_care<%s>, ", menu_1st_7th_1st_dis_val[sys_cfg.foster_care]);
    } else {
        logdNoTimeNoNewLine("foster_care<err>, is %d, ", sys_cfg.foster_care);
    }

    logdNoTimeNoNewLine("\r\n");
#endif            //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION)
#pragma endregion //编译时间 + 速度模式 + 系统看护
}

/*
mode:0 => [i][promt time, 106]: func, str, 2016-09-15 09:12:13\r\n //promt time(tm, 0, func, str)
mode:1 => [i][promt time, 106]: func, str, 2016-09-15 09:12:13, //promt time(tm, 1, func, str)
mode:2 => str[2016-09-15 09:12:13],  //promt time(tm, 2, lv, NULL, str, NULL)
mode:3 => str[2016-09-15 09:12:13]\r\n //promt time(tm, 3, lv, NULL, str, NULL)
*/
void promt_time(time_t *time, int mode, log_level_enum log_level, const char *func, int line, const char *str, const char *end) {
    if (time == NULL || mode < 0 || mode > 3 || str == NULL) {
        loge("err%d%d%d", time == NULL, mode<0, mode> 3, str == NULL);
        return;
    }

    if (mode == 0 || mode == 1) {
        if (func == NULL) {
            loge("err");
            return;
        }

        logb_NoNewLn(log_level, "[%s, %d], [%s], %s[20%02X-%02X-%02X %02X:%02X:%02X]", func, line, _tm(), str, time->year, time->month, time->date, time->hour, time->min, time->sec);

        if (end != NULL) {
            logb_empty(log_level, ", %s", end);
        }

        if (mode == 0) {
            logb_endColor_newLn(log_level, );
        } else {
            logb_empty(log_level, ", ");
        }
    }

    if (mode == 2 || mode == 3) {
        logb_empty(log_level, "%s[20%02X-%02X-%02X %02X:%02X:%02X]", str, time->year, time->month, time->date, time->hour, time->min, time->sec);

        if (end != NULL) {
            logb_empty(log_level, ", %s", end);
        }

        if (mode == 2) {
            logb_empty(log_level, ", ");
        } else {
            logb_endColor_newLn(log_level, );
        }
    }
    /*
#include "jt808_type.h"
typedef struct {
    u8 year;
    u8 month;
    u8 date;
    u8 hour;
    u8 min;
    u8 sec;
} time_t;
    */
}

void chk_err(void) {
#pragma region 检查APP版本号信息长度长度合法性
    int len = strlen(TR9_SOFT_VERSION_APP);
    log_level_enum log_lv = log_level_info;
    if (len > TR9_SOFT_VERSION_MAX_LEN) {
        log_lv = log_level_err;
    }
    logb(log_lv, "appLen%d", len);
#pragma endregion 检查APP版本号信息长度长度合法性

#pragma region 检查是否启用了六轴lsm6ds3tr - c模块
#if (!MODULE_USING_lsm6ts3trc)
    logw("lsmNoUse");
#endif //#if (MODULE_USING_lsm6ts3trc)
#pragma endregion 检查是否启用了六轴lsm6ds3tr - c模块
}
