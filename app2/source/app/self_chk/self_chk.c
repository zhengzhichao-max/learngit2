

#include "include_all.h"

typedef union {
    unsigned char val;
    struct {
        unsigned char hd : 2;  //硬盘//sdhd_sta_enum
        unsigned char sd : 2;  //SD卡//sdhd_sta_enum
        unsigned char cam : 1; //摄像头//cam_sta_enum
    } state;
} n_dev_state_st;

static n_dev_state_st n_dev_state = {.state.hd = sdhd_sta_unknown, .state.sd = sdhd_sta_unknown, .state.cam = cam_sta_unknown};

#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && MODULE_DEBUG_self_chk
static void help(void) {
    logd("hdsd, 0-1(HD,SD), 0-2(ok,err,full)"); //设置【SD卡和硬盘】状态
    logd("hdsd, 2(CAM), 04X(sta)");             //设置【SD卡和硬盘】状态
    logd("alarm, 1-30(ticDlyS)");               //设置报警，令报警//每隔5秒，报一种警，直到所以种类报完//延时播报

#if (MODULE_DEBUG_self_chk && 0)
    logd("2, x, rst addr & erase");
#endif //#if (MODULE_DEBUG_self_chk)
}

int cmd_self_chk(argv_list_st *argv_list) {
    if (argv_list->argc == 1) {
        help();
        return 0;
    }

    if (argv_list->argc != 3) {
        loge("err");
        return -1;
    }
    int who = -1, state = -1;
    if (sscanf(argv_list->argv[1], "%d", &who) != 1) {
        loge("err");
        return -1;
    }
    if (sscanf(argv_list->argv[2], "%d", &state) != 1) {
        loge("err");
        return -1;
    }

    selfChk_HD_SD_stateSet(who, state);
    return 0;
}
#endif //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && defined(MODULE_DEBUG_self_chk)

/*
设置【硬盘】【SD卡】状态：
6.81 存储设备状态【RK?MCU】
消息ID：0x6064。
RK运行过程发现存储设备状态变化，若设备未接或满载，告知MCU状态变化，存储设备状态变化消息体见表93，MCU收到后需要应答。
表93 存储设备状态消息体数据格式
起始字节	字段	类型	描述及要求
0	储存设备类型	BYTE	0-硬盘； 1-SD卡
1	设备状态	BYTE	0-正常； 1-连接异常；2-磁盘满载
*/
void selfChk_HD_SD_stateSet(unsigned char who, unsigned char state) {
    if (who >= 2 || state >= 3) {
        loge("err");
        return;
    }

    if (who == 0) {
        n_dev_state.state.hd = state;
    } else {
        n_dev_state.state.sd = state;
    }
}

//消息体 =  byte + word//byte = cnt_cam,//state, bit 0异常,1正常
//cnt_cam,只关心【有多少个正在使用的摄像头】。且是否正常。
void selfChk_cam_stateSet(unsigned char cnt_cam, unsigned short state) {
#define DEBUG_HERE_202209271112 1
    if (cnt_cam > 16 || cnt_cam == 0) {
        loge("err%d%d", (cnt_cam > 16), (cnt_cam == 0));
        return;
    }
    int i;
    unsigned short mask = 0x0000;
    for (i = 0; i < cnt_cam; i++) {
        mask += (1 << i);
    }

#if (DEBUG_HERE_202209271112)
    logf("cnt %d, mask 0x%04X, state 0x%04X, state & mask = 0x%04X", cnt_cam, mask, state, state & mask);
#endif

    if ((state & mask) == mask) {
        n_dev_state.state.cam = cam_sta_ok;
#if (DEBUG_HERE_202209271112)
        logi("ok");
#endif
    } else {
        n_dev_state.state.cam = cam_sta_err;
#if (DEBUG_HERE_202209271112)
        loge("err");
#endif
    }
}

#define isOK(hd_sd) (hd_sd == sdhd_sta_ok || hd_sd == sdhd_sta_full)
sdhd_staSel_enum selfChk_HD_SD_stateGet(void) {
    if (n_dev_state.state.sd == sdhd_sta_unknown && n_dev_state.state.hd == sdhd_sta_unknown) {
        return sdhd_staSel_unknown;
    } else if (isOK(n_dev_state.state.sd) && !isOK(n_dev_state.state.hd)) {
        return sdhd_staSel_ok_null;
    } else if (!isOK(n_dev_state.state.sd) && isOK(n_dev_state.state.hd)) {
        return sdhd_staSel_null_ok;
    } else if (isOK(n_dev_state.state.sd) && isOK(n_dev_state.state.hd)) {
        return sdhd_staSel_ok_ok;
    } else {
        return sdhd_staSel_null_null;
    }
}

cam_sta_enum selfChk_cam_stateGet(void) {
    return (cam_sta_enum)n_dev_state.state.cam;
}

#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && defined(MODULE_DEBUG_self_chk)
void selfChk_HD_SD_statePromt(void) {
    const char *str_state[] = {"ok", "er", "fu", "un"};
    logf("hd %s%d, sd %s%d", str_state[n_dev_state.state.hd], n_dev_state.state.hd, str_state[n_dev_state.state.sd], n_dev_state.state.sd);
}
#endif //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && defined(MODULE_DEBUG_self_chk)

#pragma region 通电自检

typedef enum {
    n_alarm_step_null = 0,
    n_alarm_step_alarm,
    n_alarm_step_alarm_already,
} n_alarm_step_enum;

typedef struct {
    /*
        开机30秒报警一次
        第一次与RK交互后10秒，报警一次
        周期1小时报警一次
    */
    unsigned int tic_boot;
    unsigned int tic_1st1717;
    unsigned int tic_alarm_gap;

    unsigned char en : 1;

#pragma region 报警项目
    unsigned char ant : 2;  //天线//n_alarm_step_enum
    unsigned char hdsd : 2; //硬盘//SD卡//n_alarm_step_enum
    unsigned char cam : 2;  //摄像头//n_alarm_step_enum
#pragma endregion 报警项目
} n_self_chk_alarm_st;

static n_self_chk_alarm_st n_alarm = {0};

void selfChk_SetTicBoot(void) {
    n_alarm.tic_boot = tick;
}

void selfChk_SetTic1st1717(void) {
    n_alarm.tic_1st1717 = tick;
}

typedef enum {
    beep_step_null = 0,
    beep_step_beep_on1,       //1
    beep_step_beep_on1_wait,  //2
    beep_step_beep_off1,      //3
    beep_step_beep_off1_wait, //4
    beep_step_beep_on2,       //5
    beep_step_beep_on2_wait,  //6
    beep_step_beep_off2,      //7
    beep_step_beep_off2_wait, //8
    beep_step_beep_on3,       //9
    beep_step_beep_on3_wait,  //10
    beep_step_beep_off3,      //11
    beep_step_beep_off3_wait, //12
    beep_step_beep_on4,       //13
    beep_step_beep_on4_wait,  //14
    beep_step_beep_off4,      //15
    beep_step_beep_off4_wait, //16
    beep_step_beep_cnt,       //17
} beep_step_enum;

typedef struct {
    unsigned short jiff;
    unsigned char en : 1;
    unsigned char step : 4;
} beep_par_st;
static beep_par_st beep_par = {0};

static void selfChk_Beep4Cnt(void) {
    if (beep_par.en == 0) {
        return;
    }

    switch (beep_par.step) {
    case beep_step_null:
    case beep_step_beep_cnt: {
        beep_par.en = 0;
        break;
    }
    case beep_step_beep_on1:
    case beep_step_beep_on2:
    case beep_step_beep_on3:
    case beep_step_beep_on4: {
        beep_on();
        beep_par.jiff = jiffies;
        beep_par.step++;
        break;
    }
    case beep_step_beep_off1:
    case beep_step_beep_off2:
    case beep_step_beep_off3:
    case beep_step_beep_off4: {
        beep_off();
        beep_par.jiff = jiffies;
        beep_par.step++;
        break;
    }
    case beep_step_beep_on1_wait:
    case beep_step_beep_on2_wait:
    case beep_step_beep_on3_wait:
    case beep_step_beep_on4_wait:
    case beep_step_beep_off1_wait:
    case beep_step_beep_off2_wait:
    case beep_step_beep_off3_wait:
    case beep_step_beep_off4_wait: {
        if (_pasti(beep_par.jiff) > 200) {
            beep_par.step++;
        }
        break;
    }
    default: {
        beep_par.en = 0;
        break;
    }
    }
}

static void beep_enable(void) {
    beep_par.en = 1;
    beep_par.jiff = jiffies;
    beep_par.step = beep_step_beep_on1;
}

static void set_alarm_string(const char *str) {
#pragma region 显示内容初始化
    _memset(((u8 *)(&mw_dis)), '\0', (sizeof(mw_dis)));

    const char *title = "通电自检";
    mw_dis.p_dis_2ln = (unsigned char *)title;
    mw_dis.p_dis_3ln = (unsigned char *)str;

    mw_dis.bg_2ln_pixel = 8 * (16 - strlen(title)) / 2; //信息提示窗口设置
    mw_dis.bg_3ln_pixel = 8 * (16 - strlen(str)) / 2;   //信息提示窗口设置
#pragma endregion 显示内容初始化

#pragma region 显示内容
    menu_cnt.menu_other = mo_message_window;
    menu_cnt.menu_flag = true;
    menu_auto_exit_set(4500, true); //退回上一级菜单
#pragma endregion 显示内容
}

static void selfChk_SetUpChk(void) {
    /*每秒进来一次*/
    if (_covern(n_alarm.tic_boot) == 30) { //开机30秒，报警
        n_alarm.ant = (unsigned char)n_alarm_step_alarm;
        n_alarm.en = 1;
        n_alarm.tic_alarm_gap = tick;
    } else if (_covern(n_alarm.tic_boot) == 1000) {
        n_alarm.tic_boot = tick - 500;
    }

    if (sys_bits1.bits.flg_rk_1717_first) {
        if (_covern(n_alarm.tic_1st1717) == 15) { //与RK通信10秒，报警
            n_alarm.hdsd = (unsigned char)n_alarm_step_alarm;
            n_alarm.cam = (unsigned char)n_alarm_step_alarm;
            n_alarm.en = 1;
            n_alarm.tic_alarm_gap = tick;
        } else if (_covern(n_alarm.tic_1st1717) == 1000) {
            n_alarm.tic_1st1717 = tick - 500;
        }
    }

    if (n_alarm.en) {
        if (_covern(n_alarm.tic_alarm_gap) < 5) { //每隔5秒报警一个内容
            return;
        }
        n_alarm.tic_alarm_gap = tick;

        if (n_alarm.ant == (unsigned char)n_alarm_step_alarm) {
            n_alarm.ant = (unsigned char)n_alarm_step_alarm_already;
            if (car_alarm.bit.ant_open || car_alarm.bit.ant_short) {
                set_alarm_string("卫星定位自检异常");
                logw("warn");
                beep_enable();
                return;
            }
        }

        if (n_alarm.hdsd == (unsigned char)n_alarm_step_alarm) {
            n_alarm.hdsd = (unsigned char)n_alarm_step_alarm_already;
            if (n_dev_state.state.hd == (unsigned char)sdhd_sta_err || n_dev_state.state.sd == (unsigned char)sdhd_sta_err) {
                set_alarm_string("存储模块自检异常");
                logw("warn");
                beep_enable();
                return;
            }
        }

        if (n_alarm.cam == (unsigned char)n_alarm_step_alarm) {
            n_alarm.cam = (unsigned char)n_alarm_step_alarm_already;
            if (n_dev_state.state.cam == (unsigned char)cam_sta_err) {
                set_alarm_string("通道视频丢失");
                logw("warn");
                beep_enable();
                return;
            }
        }

        //全部报警完毕
        if (n_alarm.hdsd != n_alarm_step_alarm
            && n_alarm.ant != n_alarm_step_alarm
            && n_alarm.cam != n_alarm_step_alarm) {
            n_alarm.en = 0;
        }
    }
}
#pragma endregion 通电自检

#pragma region 人机交互-- RUN指示灯

#define DEBUG_run_led_mode_st 0
typedef struct {
    unsigned char sta : 1;
    unsigned char cnt : 5; // 100ms//[0-31] = [0, 3.1秒]
    unsigned char dly : 1;
    unsigned char end : 1;
} run_led_mode_st;

#define run_led_item_set(onoff /*0/1*/, count /*0-31*/) \
    {.sta = onoff, .cnt = 0, .dly = 0, .end = 0}, {     \
        .sta = 0, .cnt = count, .dly = 1, .end = 0      \
    }
#define run_led_item_end() \
    { .sta = 0, .cnt = 0, .dly = 0, .end = 1 }

static run_led_mode_st run_led_mode0[] = {
    //常亮
    run_led_item_set(1, 31), //
    run_led_item_end(),      //
};

static run_led_mode_st run_led_mode1[] = {
    //秒闪
    run_led_item_set(1, 1), //
    run_led_item_set(0, 9), //
    run_led_item_end(),     //
};

static run_led_mode_st run_led_mode2[] = {
    //快闪3下
    run_led_item_set(1, 1), //
    run_led_item_set(0, 2), //

    run_led_item_set(1, 1), //
    run_led_item_set(0, 2), //

    run_led_item_set(1, 1), //
    run_led_item_set(0, 2), //

    run_led_item_set(0, 18), //
    run_led_item_end(),      //
};

static run_led_mode_st *run_led_mode[] = {
    run_led_mode0,
    run_led_mode1,
    run_led_mode2,
};
#define size_run_led_mode() (sizeof(run_led_mode) / 4)

typedef struct {
    unsigned char en : 1;     //使能 run led 指令灯 模式 运行
    unsigned char repeat : 1; //指令灯 模式 只运行一次//指令灯 模式 重复运行
    unsigned char cnt : 6;    //时间片计数//单位：100ms

    unsigned short step : 6;
    unsigned short total : 6;
    unsigned short tic : 4; //0-15
    run_led_mode_enum mode_now;
    run_led_mode_st *mode;
} run_led_par_st;
#define DEBUG_HERE_chk_size_run_led_par_st 0
static run_led_par_st run_led_par = {.mode_now = run_led_mode_null};

#define item (p->mode + p->step)
static void selfChk_RunLed_do(void) {
    run_led_par_st *p = &run_led_par;

    if (p->en == 0) {
        // loge("err");
        return;
    }

    if (p->mode == NULL || p->step >= p->total) {
        p->en = 0;
        loge("err%d%d", p->mode == NULL, p->step >= p->total);
        return;
    }

    if (item->end == 1) {
        if (p->repeat == 1) {
            p->step = 0;
            p->cnt = 0;
        } else {
            p->en = 0;
        }
    }

    if (item->dly == 0) { //执行【开/关】IO
        if (item->sta) {
#if (DEBUG_run_led_mode_st)
            loge("on led");
#endif
            run_led_on();
        } else {
#if (DEBUG_run_led_mode_st)
            logf("off led");
#endif
            run_led_off();
        }
        p->cnt = 0;
        p->step++;
    } else {
        ++p->cnt;
#if (DEBUG_run_led_mode_st)
        logi("dly%d", p->cnt);
#endif
        if (p->cnt >= item->cnt) {
            p->cnt = 0;
            p->step++;
        }
    }
}

int selfChk_RunLed_initMode(run_led_mode_enum mode, unsigned char en, unsigned char repeat) {
    //状态未改变
    if (mode == run_led_par.mode_now) {
        return -1;
    }

    if ((int)mode >= size_run_led_mode()) {
        loge("err");
        return -1;
    }

    memset(&run_led_par, 0, sizeof(run_led_par_st));

    run_led_par.mode = run_led_mode[(int)mode];

    int total = 0;
    while ((run_led_par.mode + total)->end == 0) {
        total++;
    }
    total++;

    if (total >= (1 << 6)) {
        logf("fatal%d", total);
        return -1;
    }

    run_led_par.en = en;
    run_led_par.repeat = repeat;
    run_led_par.total = total;
    run_led_par.mode_now = mode;
    return 0;
}

static void selfChk_RunLed(void) {
    /*每秒进来一次*/

    //*********---------------*********---------------*********---------------*********---------------
    //硬盘锁                        //if (pub_io.b.key_s == true)
    //天线断开 || 天线短路          //if (car_alarm.bit.ant_open || car_alarm.bit.ant_short)
    //*********---------------*********---------------*********---------------*********---------------
    //主电电压过低 || 终端断电
    //SIM卡
    //*********---------------*********---------------*********---------------*********---------------
#if (DEBUG_HERE_chk_size_run_led_par_st)
    logf("%d", sizeof(run_led_par_st));
#endif

    if (++run_led_par.tic < 3) {
        return;
    }
    run_led_par.tic = 0;

    if (pub_io.b.key_s == true || car_alarm.bit.ant_open || car_alarm.bit.ant_short) {
        selfChk_RunLed_initMode(run_led_mode_flip3, 1, 1);
    } else {
        selfChk_RunLed_initMode(run_led_mode_on_forever, 1, 1);
    }
}
#pragma endregion 人机交互-- RUN指示灯

#pragma region 自检结果展示
void selfChk_do(void) {
    selfChk_Beep4Cnt();
    selfChk_RunLed_do();
}
#pragma endregion 自检结果展示

#pragma region 自检
void selfChk(void) {
    selfChk_SetUpChk();
    selfChk_RunLed();
}
#pragma endregion 自检
