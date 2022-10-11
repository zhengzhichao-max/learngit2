
#include "include_all.h"

#ifdef USE_MID_TEST
static void test_lsm6ds3trc(void) {
    if (sys_bits1.bits.lsm6ds3trc_ok_cnt != 3) {
        if (LSM6DS3TRC_GetChipID() == 0) {
            sys_bits1.bits.lsm6ds3trc_ok_cnt++;
        } else {
            sys_bits1.bits.lsm6ds3trc_ok_cnt = 0;
        }
    }
}
#endif

#if (1)
static void test_rx8025t(void) {
    static TIME_ST RTC_Time; //存储日期时间的结构体变量

    if (sys_bits1.bits.rx8025t_init != 1) {
        if (rx8025t_init() == RT_EOK) {
            sys_bits1.bits.rx8025t_init = 1;
            logi("rx8025t init, ok");
        } else {
            loge("rx8025t init, err");
        }
    } else if (sys_bits1.bits.rx8025t_set_time != 1) {
        if (rx8025t_setRtcTime_test() == RT_EOK) {
            sys_bits1.bits.rx8025t_set_time = 1;
            logi("rx8025t setRtcTime test, ok");
        } else {
            loge("rx8025t setRtcTime test, err");
        }
    } else {
        if (get_rtc_time(&RTC_Time) == RT_EOK) {
            if (++sys_bits1.bits.rx8025t_ok_cnt >= 12) {
                sys_bits1.bits.rx8025t_ok_cnt = 0;
                logi("20%02d-%02d-%02d %02d:%02d:%02d", RTC_Time.year, RTC_Time.month, RTC_Time.day, RTC_Time.hour, RTC_Time.minute, RTC_Time.second);
            }
        } else {
            loge("rx8025t read time, err");
        }
    }
}
#endif //#ifdef USE_MID_TEST

void mid_test_task(void) {
#if (1)
    static u8 tic = 0;
    tic++;

    if (tic % 5 == 0) {
        test_rx8025t();
    }
#endif

#ifdef USE_MID_TEST
    static u8 tic = 0;

    tic++;
    if (tic % 1 == 0) {
        if (sys_bits1.bits.rx8025t_lsm6ds3trc_report != 1) {
            test_lsm6ds3trc();
            test_rx8025t();

            if (sys_bits1.bits.rx8025t_ok_cnt == 3 && sys_bits1.bits.lsm6ds3trc_ok_cnt == 3) {
                logf("RTC, rx8025t, ok");
                logf("六轴，lsm6ds3trc, ok");
                sys_bits1.bits.rx8025t_lsm6ds3trc_report = 1;
            }
        }
    }

#if (0)
    if (tic % 2 == 0) {
    }
#endif
#endif //#ifdef USE_MID_TEST
}
