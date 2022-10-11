
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include_all.h"

#if (0)
static void test_mem(void) {
}
#endif

static void help(void) {
    logd("901, 1, see");       //休眠位置上报:
    logd("901, 2, 10~300, s"); //休眠位置上报:
#if (0)
    logd("101, 100-1200, mem");
    logd("102, mem");
    if (menu_cnt.menu_other != mo_message_window) {
        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_message_window;
        menu_cnt.menu_flag = true;
    }
#else
#endif

#if (0)
    // logd("1, get log size");
    logd("2, new a log & w");

    logd("3, x, rst addr");
    logd("3, promt log");

    logd("4, promt log all");

    logd("6, init state");

    // logd("7, 1, mcu boot");
    // logd("7, 2, rk boot");
    // logd("7, 3, rk rst");
#endif //#if (0)

    // logd("2, x, rst addr & erase");
    // logd("5, promt log all");
    // logd("8, insert");
}

int cmd_util(argv_list_st *argv_list) {
    if (argv_list->argc == 0) {
        loge("err");
        return -1;
    }
    if (argv_list->argc == 1) {
        help();
        return 0;
    }

    int argv1 = -1;
    int cnt = sscanf(argv_list->argv[1], "%d", &argv1);

    if (cnt == 1) {
        switch (argv1) {
        case 901: {
            int argv2 = -1;
            cnt = sscanf(argv_list->argv[2], "%d", &argv2);
            if (cnt == 1) {
                switch (argv2) {
                case 1: { //休眠位置上报:logd("901, 1, see");
                    gsm_see_state();
                    break;
                }
                case 2: { //休眠位置上报:logd("901, 2, 10~300, s");
                    int argv3 = -1;
                    cnt = sscanf(argv_list->argv[3], "%d", &argv3);
                    if (cnt == 1 && (argv3 >= 10 && argv3 <= 300)) {
                        report_cfg.sleep_time = argv3;
                        logi("set cfg.sleep.rpt %d", report_cfg.sleep_time);
                    } else {
                        loge("e");
                    }
                    break;
                }
                default: {
                    break;
                }
                }
            } else {
                loge("e");
            }
            break;
        }
#if (0)
        case 101: {
            int argv2 = -1;
            cnt = sscanf(argv_list->argv[2], "%d", &argv2);
            if (cnt == 1) {
                if (argv2 >= 100 && argv2 <= 1200) {
                    char *p = malloc(argv2);
                    if (p != NULL) {
                        loge("ok, %d", argv2);
                        free(p);
                    } else {
                        loge("fail");
                    }
                } else {
                    loge("e");
                }
            } else {
                loge("e");
            }
            break;
        }
        case 102: {
            test_mem();
            break;
        }
        case 0: {
            DIS_LCD_BL_PWR;
            break;
        }
        case 1: {
            EN_LCD_BL_PWR;
            break;
        }
        case 4: {
#pragma region 显示内容初始化
            _memset(((u8 *)(&mw_dis)), '\0', (sizeof(mw_dis)));

            mw_dis.p_dis_1ln = report_dis_sel(arr, 0, 0);
            mw_dis.p_dis_2ln = report_dis_sel(arr, 0, 1);
            mw_dis.p_dis_3ln = report_dis_sel(arr, 1, 0);
            mw_dis.p_dis_4ln = report_dis_sel(arr, 1, 1);

            mw_dis.bg_1ln_pixel = (8 * 3);  //信息提示窗口设置
            mw_dis.bg_2ln_pixel = (8 * 4);  //信息提示窗口设置
            mw_dis.bg_3ln_pixel = (16 * 2); //信息提示窗口设置
            mw_dis.bg_4ln_pixel = (16 * 1); //信息提示窗口设置
#pragma endregion 显示内容初始化

#pragma region 显示内容
            menu_cnt.menu_other = mo_message_window;
            menu_cnt.menu_flag = true;
            menu_auto_exit_set(1000 * 10, true); //退回上一级菜单
#pragma endregion 显示内容
            break;
        }
        case 1: {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
            mw_dis.bg_2ln_pixel = (8 * 3); //信息提示窗口设置

            menu_auto_exit_set(1000 * 5, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
            break;
        }
        case 2: {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 1) + 1)));
            mw_dis.p_dis_3ln = ((u8 *)(*(*(menu_report_dis + 2) + 1)));
            mw_dis.bg_2ln_pixel = (8 * 3); //信息提示窗口设置
            mw_dis.bg_3ln_pixel = (8 * 3); //信息提示窗口设置

            menu_auto_exit_set(1000 * 3, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
            break;
        }
        case 3: {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 3) + 1)));
            mw_dis.bg_2ln_pixel = (8 * 3); //信息提示窗口设置

            menu_auto_exit_set(1000 * 10, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
            break;
        }
        case 4: {
            if (argv_list->argc == 2) { //logd("4, promt log all");
                log_boot_rst_st *p_read = (log_boot_rst_st *)ADDR_TEST_MCU_FLASH;
                int ret = -1;
                do {
                    log_boot_rst_st log;
                    ret = read_write_log(log_rw_read, (unsigned int)p_read++, &log, __func__, __LINE__, ENABLE);
                } while (ret == 0);
            } else {
                loge("undef");
            }
            break;
        }
#endif //#if (0)
#if (0)
        case 5: {
            if (argv_list->argc == 2) { //logd("5, promt log all");
                log_boot_rst_st *p_read = (log_boot_rst_st *)ADDR_TEST_MCU_FLASH;
                int ret = -1;
                do {
                    log_boot_rst_st log;
                    ret = read_write_log(log_rw_read, (unsigned int)p_read++, &log, __func__, __LINE__, DISABLE);
                    if (ret == 0) {
                        promt_log(&log, __func__, __LINE__);
                        uart1_tran_task_nms(5);
                        // logd(_ok_, _ok());
                    } else {
                        logd(_word_("end"), _word_color(log_level_fatal));
                    }
                } while (ret == 0);
            } else {
                loge("undef");
            }
            break;
        }
#endif //#if (0)
#if (0)
        case 6: { //logd("6, init state");
            init_par_log_write();
            break;
        }
#endif //#if (0)
#if (0)
        case 7: {
            /*
                logd("7, 1, mcu boot");
                logd("7, 2, rk boot");
                logd("7, 3, rk rst");
                */

            /*
            typedef enum {
    event_mcu_boot = 0,
    event_rk_boot,
    event_rk_rst,
    event_rk_cnt,
} event_boot_rst_enum;
            */
            /*
           jiffies//ms
          mix.time
           */
            /*

          */

            // log_boot_rst_st *p_read = (log_boot_rst_st *)ADDR_TEST_MCU_FLASH;
            // int ret = -1;
            // do {
            //     log_boot_rst_st log;
            //     ret = read_write_log(log_rw_read, (unsigned int)p_read++, &log, __func__, __LINE__, DISABLE);
            //     if (ret == 0) {
            //         promt_log(&log, __func__, __LINE__);
            //         // logd(_ok_, _ok());
            //     } else {
            //         logd(_word_("end"), _word_color(log_level_fatal));
            //     }
            // } while (ret == 0);
            loge("undef");
            break;
        }
#endif //#if (0)
#if (0)
        case 8: { //logd("8, insert");
            //log_write(event_insert);
            break;
        }
#endif //#if (0)
        default: {
            loge("undef");
            break;
        }
        }

        return 0;
    } else {
        loge("err");
        return 0;
    }
}
