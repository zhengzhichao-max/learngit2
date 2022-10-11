/*
  ******************************************************************************
  * @file    app_cfg.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  配置与管理中间层初始化和配置
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

#include "include_all.h"

REGISTER_CONFIG_STRUCT register_cfg; //终端注册消息体
SYSTERM_CONFIG_STRUCT sys_cfg;       //用户设置的系统参数

#ifdef USE_SYSTERM_BITS1_UNION
SYSTERM_BITS1_UNION sys_bits1 = {
#ifdef USE_MID_TEST
    .bits.lsm6ds3trc_ok_cnt = 0,
    .bits.rx8025t_lsm6ds3trc_report = 0,
#endif

#if (1)
    .bits.rx8025t_init = 0,
    .bits.rx8025t_set_time = 0,
    .bits.rx8025t_ok_cnt = 0,
#endif

    .bits.flg_lsm_pwr_on = 0,
    .bits.flg_lsm_read = 0,

    .bits.flg_rk_1717_first = 0,
};
#endif

#pragma region //APP软件编译时间
#if defined(TR_DEBUG)
//调试版本
const char *str_app_build_time = __DATE__ " " __TIME__ " D";
#elif defined(TR_TEST)
//测试版本
const char *str_app_build_time = __DATE__ " " __TIME__ " T";
#else
//发布版本
const char *str_app_build_time = __DATE__ " " __TIME__ "";
#endif            /*TR_DEBUG*/
#pragma endregion //APP软件编译时间

char *get_build_time_need_free(const char *ver) {
    #define DEBUG_HERE_202210071509 1
    char *ret = malloc(16);
    if (ret == NULL) {
        return NULL;
    }
    sprintf(ret, "null");

    int len = strlen(ver);
    if (!(len >= 20 && len <= 32)) {
        return ret;
    }

    char *buf = malloc(len + 1);
    if (buf == NULL) {
        return ret;
    }
    memcpy(buf, ver, len + 1);
    // memcpy(buf, "aep 20 2022 08:45:26", strlen("Sep 20 2022 08:45:26") + 1);//测试

    typedef struct {
        const char *str_mon;
        const unsigned char mon;
    } month_st;

    const month_st month[] = {
        {"Ja", 1}, //Jan
        {"Fe", 2}, //Feb
        {"Ma", 3}, //Mar
        {"Ap", 4}, //Apr
        {"Ma", 5}, //May
        {"Jun", 6},
        {"Jul", 7},
        {"Au", 8},  //Aug
        {"Se", 9},  //Sep
        {"Oc", 10}, //Oct
        {"No", 11}, //Nov
        {"De", 12}, //Dec
        {NULL, NULL},
    }; //Sep 20 2022 08:45:26

    const month_st *p_month = month;

#if (DEBUG_HERE_202210071509)
    logd("%s", buf);
#endif

    int val_mon = -1;
    int flg_found = -1;
    for (; p_month->str_mon != NULL; p_month++) {
        if (_strnicmp(buf, p_month->str_mon, strlen(p_month->str_mon)) == 0) {
            val_mon = (int)p_month->mon;
            flg_found = 0;
            break;
        }
    }

    if (flg_found == -1) {
        free(buf);
        return ret;
    }

    int val_day = -1;
    int val_year = -1;
    int val_hour = -1;
    int val_min = -1;
    int val_sec = -1;
    char val_DT = ' ';

    //"Sep 20 2022 08:45:26"
    //"Sep 20 2022 08:45:26 D/T"
    int cnt = sscanf(buf, "%*[^ ] %d 20%d %d:%d:%d %c", &val_day, &val_year, &val_hour, &val_min, &val_sec, &val_DT);

    if (cnt == 5) {
        sprintf(ret, "%02d%02d%02d%02d%02d%02d", val_year, val_mon, val_day, val_hour, val_min, val_sec); //len max = 15
    } else if (cnt == 6) {
        sprintf(ret, "%02d%02d%02d%02d%02d%02d%c", val_year, val_mon, val_day, val_hour, val_min, val_sec, val_DT); //len max = 15
    } else {
        ;
    }

    free(buf);
    return ret;
}

/****************************************************************************
* 名称:    init_app_cfg ()
* 功能：配置init_app_cfg参数
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void init_app_cfg(void) {
#if (P_RCD == RCD_BASE)
    load_rcd_manage_infomation();
#endif

#if (P_AREA == AREA_BASE)
    area_initialize();
#endif
#if (P_SOUND == SOUND_BASE)
    _memset((u8 *)&tts_m, 0x00, sizeof(tts_m));
#endif

    logi("APP SYS START");

    _memset((u8 *)&can_s_cali, 0, sizeof(can_s_cali));
    can_s_cali.swi = false;
    _memset((u8 *)&dvr_power, 0, sizeof(DVR_POWER_CTRL));
    pub_io.b.dvr_open = false;
    dvr_power.cmd_ack = false;
}

#if (0)
/****************************************************************************
* 名称:    custom_100ms_task ()
* 功能：定制，100毫秒的任务
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void custom_100ms_task(void) {
    static u32 task_100ms_t = 0;

    if (_pastn(task_100ms_t) >= 100) {
        task_100ms_t = jiffies;
    }
}

void food_dog(void) {
    static u32 WatchDogTime = 0;

    if (_pastn(WatchDogTime) < 800) {
        return;
    }

    WatchDogTime = jiffies;
}

void print_gsm_char(u8 *buf, u16 len) {
    u8 dh, dl;
    u16 i;

    if (u1_m.cur.b.debug == false) {
        return;
    }

    if (len > 600) { len = 600; }

    for (i = 0; i < len; i++) {
        dh = buf[i] >> 4;   //取高四位
        dl = buf[i] & 0x0f; //取低四位

        if (dh > 0x09)
            dh = dh + 0x37;
        else
            dh = dh + 0x30;

        if (dl > 0x09) {
            dl = dl + 0x37;
        } else {
            dl = dl + 0x30;
        }

        write_uart1_hex(dh);
        write_uart1_hex(dl);
    }

    write_uart1_hex(0x0d);
    write_uart1_hex(0x0a);
}

void print_char(u8 chr) {
    if (u1_m.cur.b.debug)
        write_uart1_hex((u8)chr);
}

void print_buf(u8 *buf, u16 len) {
#if 1
    u16 i;
    if (len > 500)
        len = 500;
    for (i = 0; i < len; i++)
        print_char(*buf++);
    print_char(0x0d);
    print_char(0x0a);
#endif
}

void printb_tr9(u8 chr) {
    if (u1_m.cur.b.debug) {
        write_uart1_hex((u8)chr);
    }
}

void print_TR9(u8 *buf, u16 len) {
#if 1
    u16 i;
    u8 dl, dh;

    if (len > 2100)
        len = 2100;
    for (i = 0; i < len; i++) {
        dh = *buf >> 4;
        dl = *buf & 0x0f;

        if (dh > 0x09)
            dh = dh + 0x37;
        else
            dh = dh + 0x30;

        if (dl > 0x09)
            dl = dl + 0x37;
        else
            dl = dl + 0x30;

        printb_tr9(dh);
        printb_tr9(dl);

        buf++;
    }
    printb_tr9(0x0d);
    printb_tr9(0x0a);
#endif
}

void print_can(u8 *buf, u16 len) {
#if 0
    u16 i;
    for (i = 0; i < len; i++)
        print_char(*buf++);
        print_char(0x0d);
    //print_char(0x0a);
#endif
}
#endif
