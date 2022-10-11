
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include_all.h"

#if (MODULE_USING_pwr_rst_rk_logSave) //定义
//FLASH_ErasePage 2K = 800h
//(8040000h - 803E000h) / 400h = 8k
//(8040000h - 803E800h) / 400h = 6k
#define ADDR_TEST_MCU_FLASH ((u32)0x0803E800)

typedef enum {
    log_rw_read = 0,
    log_rw_write,
} log_rw_enum;

typedef struct {
    unsigned short crc16;
    unsigned int addr;      //在存储中的位置
    unsigned int index;     //日志记录序号
    unsigned int tic_ms;    //系统时间，毫秒
    unsigned int tic_delta; //系统时间，毫秒
    time_t tm_mix;
    event_boot_rst_enum event;
    unsigned char reserve[3];
} log_boot_rst_st;
#define CRC_SIZE (sizeof(log_boot_rst_st) - sizeof(unsigned short))
#define CRC_CALC(addr) (addr + sizeof(unsigned short))
#define get_index_max() ((unsigned short)(2048 / sizeof(log_boot_rst_st)))

typedef struct {
    unsigned short index;
    unsigned short index_max; //允许最大日志数量
    unsigned int tic_delta;
    event_boot_rst_enum event;
    log_boot_rst_st *p_write;
} log_w_st;

//记录当前准备可以写的log信息，的，附加标签信息
static const log_w_st log_w_const = {
    .index = 1,
    .index_max = 0,
    .event = event_mcu_boot,
    .tic_delta = 0,
    .p_write = (log_boot_rst_st *)ADDR_TEST_MCU_FLASH};
static log_w_st log_w = {0};
#endif //#if (MODULE_USING_pwr_rst_rk_logSave)//定义

#if (MODULE_USING_pwr_rst_rk_logSave) //方法
static char arr_event_boot_rst_number[4];
static char *getStr_eventMcuRk(event_boot_rst_enum event) {
    const char *arr[] = {"mcu boot", "rk boot en", "rk boot un", "rk rst en", "rk rst un", "insert"};
    if (event >= event_rk_cnt) {
        if (event <= 999) {
            sprintf(arr_event_boot_rst_number, "%d", (int)event);
            return arr_event_boot_rst_number;
        } else {
            return "???";
        }
    } else {
        return (char *)(arr[(int)event]);
    }
}

//ret:-1,fail,0,success
static int has_log2(log_boot_rst_st *p) {
#if (0)
    return has_log((unsigned int)p);
#else
    if (crc16Modbus_Get(CRC_CALC((unsigned char *)p), CRC_SIZE) != p->crc16) {
        return -1;
    } else {
        return 0;
    }
#endif
}

//ret:-1,fail,0,success
static int has_log(unsigned int addr) {
#if (0)
    log_boot_rst_st *p = (log_boot_rst_st *)addr;
    if (crc16Modbus_Get(((unsigned char *)addr) + 1, CRC_SIZE) != p->crc16) {
        return -1;
    } else {
        return 0;
    }
#else
    return has_log2((log_boot_rst_st *)addr);
    // if (crc16Modbus_Get(CRC_CALC((unsigned char *)p), CRC_SIZE) != p->crc16) {
    //     return -1;
    // } else {
    //     return 0;
    // }
#endif
}

static void promt_log(log_boot_rst_st *p, const char *func, int line) {
#if (1)
    time_t *time = &p->tm_mix;
    logd_NoNewLn("%d. [%s", p->index, getLogTimeInfo(p->tic_ms));
    logb_endColor_newLn(log_level_debug, "(%s)], [20%02X-%02X-%02X %02X:%02X:%02X], %s", getLogTimeInfo(p->tic_delta), time->year, time->month, time->date, time->hour, time->min, time->sec, getStr_eventMcuRk(p->event));
#else
    logd_NoNewLn("[%s, %d], crc16 = 0x%04X, index = %d, ", func, line, p->crc16, p->index);
    logb_empty(log_level_debug, "[%s], ", getLogTimeInfo(p->tic));
    promt_time(&p->tm_mix, 2, log_level_debug, NULL, "tm", NULL);
    logb_endColor_newLn(log_level_debug, "event = %d", p->event);
#endif
}

//ret:0,success;-1,fail
static int read_write_log(log_rw_enum log_rw, unsigned int addr, log_boot_rst_st *log, const char *func, int line, FunctionalState en_log) {
    if (log == NULL) {
        loge("err");
        return -1;
    }

    if ((addr % 4) != 0) {
        loge("err");
        return -1;
    }

    if (!(addr >= ADDR_TEST_MCU_FLASH || addr < ADDR_TEST_MCU_FLASH + 1024 * 2)) {
        logd(_err_ ", 0x%p <> 0x%p, 0x%p", _err(), addr, ADDR_TEST_MCU_FLASH, ADDR_TEST_MCU_FLASH + 1024 * 2);
        return -1;
    }

    unsigned short len = sizeof(log_boot_rst_st);

    if ((len % 4) != 0) {
        loge("err");
        return -1;
    }

    if (addr + len >= ADDR_TEST_MCU_FLASH + 1024 * 2) {
        loge("err");
        return -1;
    }

    len /= 4;
    unsigned short i;

    if (log_rw == log_rw_read) {
#if (0)
        for (i = 0; i < len; i++) {
            *(((unsigned int *)log) + i) = *(unsigned int *)(addr + 4 * i);
        }
#else
        memcpy((void *)log, (const void *)addr, 4 * len);
#endif

        if (en_log == ENABLE) {
            logd_NoNewLn("r log done, 0x%p, ", addr);
        }
#if (1)
        if (has_log2(log) == 0) {
#else
        if (has_log((unsigned int)&log) == 0) {
#endif
            if (en_log == ENABLE) {
                logb_endColor_newLn(log_level_debug, _ok_ ", is log", _ok());
            }
        } else {
            if (en_log == ENABLE) {
                logb_endColor_newLn(log_level_debug, _err_, _err());
            }
            return -1;
        }
    } else if (log_rw == log_rw_write) {
        FLASH_Unlock();
#if (0)
        promt_log(log, func, line);
        uart1_tran_task_nms(5);
#endif

        for (i = 0; i < len; i++) {
            if (FLASH_ProgramWord(addr + 4 * i, *(((unsigned int *)log) + i)) == FLASH_COMPLETE) {
                //判断写入的数据是否正确
                if (*(unsigned int *)(addr + 4 * i) != *(((unsigned int *)log) + i)) {
                    loge("[%s, %d], w dat err", func, line);
                    FLASH_Lock();
                    return -1;
                }
            } else {
                loge("[%s, %d], w dat err, 0x%p[0x%08X, 0x%08X]", func, line, *(unsigned int *)(addr + 4 * i), *(((unsigned int *)log) + i));
                FLASH_Lock();
                return -1;
            }
        }

        if (en_log == ENABLE) {
            logd("w log done, 0x%p", addr);
        }

        FLASH_Lock();
    } else {
        loge("err");
    }

#if (1)
    if (en_log == ENABLE) {
        promt_log(log, func, line);
    }
#endif

    return 0;
}

static log_boot_rst_st *new_a_log_need_free(unsigned int index, event_boot_rst_enum event, log_boot_rst_st *p_write, unsigned int tic_delta) {
    log_boot_rst_st *ret = (log_boot_rst_st *)malloc(sizeof(log_boot_rst_st));
    if (ret == NULL) {
        return NULL;
    }

    ret->addr = (unsigned int)p_write;
    ret->index = index;
    ret->tic_ms = jiffies;
    ret->tic_delta = tic_delta;
    memcpy((void *)&ret->tm_mix, (const void *)&mix.time, sizeof(time_t));
    ret->event = event;

    ret->crc16 = crc16Modbus_Get(CRC_CALC((unsigned char *)ret), CRC_SIZE);

    return ret;
}

void init_par_log_write(void) {
    log_boot_rst_st *p_read = (log_boot_rst_st *)ADDR_TEST_MCU_FLASH;
    int ret = -1;
    int found = -1;
    do {
        log_boot_rst_st log = {0};
        ret = read_write_log(log_rw_read, (unsigned int)p_read++, &log, __func__, __LINE__, DISABLE);
        if (has_log2(&log) == 0) {
            found = 0;
            //是下一条要写的消息
            log_w.index = log.index + 1;
            log_w.index_max = get_index_max();
            log_w.tic_delta = log.tic_ms + jiffies;
            log_w.p_write = (log_boot_rst_st *)log.addr;
            log_w.p_write++;
        }
    } while (ret == 0);

    if (found == -1) {
        log_w = log_w_const;
        log_w.index_max = get_index_max();

        FLASH_Unlock();
        if (FLASH_ErasePage((unsigned int)log_w.p_write) == FLASH_COMPLETE) {
            logd("erase "_done_
                 ", ",
                 _done());
        } else {
            logd("erase "_err_
                 ", ",
                 _err());
        }
        FLASH_Lock();
    }

    logi("sz = %d, index = %d, index_max = %d, tic_delta["_word2_() "], p_write 0x%p", sizeof(log_boot_rst_st), log_w.index, log_w.index_max, _word2_color(log_level_fatal, log_level_info, getLogTimeInfo(log_w.tic_delta)), log_w.p_write);
}

static void help(void) {
#if (MODULE_DEBUG_pwr_rst_rk_logSave)
    // logd("1, get log size");
    logd("2, new a log & w");

    logd("3, x, rst addr");
    logd("3, promt log");

    logd("4, promt log all");

    logd("6, init state");

    // logd("7, 1, mcu boot");
    // logd("7, 2, rk boot");
    // logd("7, 3, rk rst");
#endif //#if (MODULE_DEBUG_pwr_rst_rk_logSave)

    logd("2, x, rst addr & erase");
    logd("5, promt log all");
    logd("8, insert");
}

int cmd_mcuflash(argv_list_st *argv_list) {
    if (argv_list->argc == 1) {
        help();
        return 0;
    }

    int argv1 = -1;
    int cnt = sscanf(argv_list->argv[1], "%d", &argv1);

    if (cnt == 1) {
        switch (argv1) {
        // case 1: {
        //     log_w.index = 1;
        //     int sz = sizeof(log_boot_rst_st);
        //     float valf = (float)2048 / sz;
        //     log_w.index_max = (unsigned short)valf;
        //     logd("sz %d, 2048 / %d = %f, index_max = %d", sz, valf, log_w.index_max);
        //     break;
        // }
        case 2: {
            if (argv_list->argc == 3) { //logd("2, x, rst addr & erase");
#if (0)
                int sz = sizeof(log_boot_rst_st);
                float valf = (float)2048 / sz;
                log_w = log_w_const;
                log_w.index_max = (unsigned short)valf;
#else
                log_w = log_w_const;
                log_w.index_max = get_index_max();
#endif

                FLASH_Unlock();
                log_level_enum log_level = log_level_debug;
                if (FLASH_ErasePage(ADDR_TEST_MCU_FLASH) == FLASH_COMPLETE) {
                    logd_NoNewLn("erase "_done_
                                 ", ",
                                 _done());
                } else {
                    loge_NoNewLn("erase "_err_
                                 ", ",
                                 _err());
                    log_level = log_level_err;
                }
                FLASH_Lock();
#if (0)
                logb_endColor_newLn(log_level, "sz %d, 2048 / %d = %f, index_max = %d, rst write addr 0x%p", sz, sz, valf, log_w.index_max, log_w.p_write);
#else
                logb_endColor_newLn(log_level, "sz %d, index_max = %d, rst write addr 0x%p", sizeof(log_boot_rst_st), log_w.index_max, log_w.p_write);
#endif
            } else if (argv_list->argc == 2) { //logd("2, new a log & w");
                if (log_w.index <= log_w.index_max) {
                    log_boot_rst_st *new_log = new_a_log_need_free(log_w.index++, log_w.event++, log_w.p_write, log_w.tic_delta);
                    if (log_w.event >= event_rk_cnt) {
                        log_w.event = event_mcu_boot;
                    }
                    if (new_log == NULL) {
                        loge("err");
                    } else {
                        read_write_log(log_rw_write, (unsigned int)log_w.p_write++, new_log, __func__, __LINE__, ENABLE);
                        FLASH_Lock();
                        free(new_log);
                    }
                } else {
                    logf("max");
                }
            } else {
                loge("undef");
            }
            break;
        }
#if (MODULE_DEBUG_pwr_rst_rk_logSave)
        case 3: {
            static log_boot_rst_st *p_read = (log_boot_rst_st *)ADDR_TEST_MCU_FLASH;
            if (argv_list->argc == 3) { //logd("3, x, rst addr");
                p_read = (log_boot_rst_st *)ADDR_TEST_MCU_FLASH;
                logd("rst read addr 0x%p", ADDR_TEST_MCU_FLASH);
            } else if (argv_list->argc == 2) { //logd("3, promt log");
                log_boot_rst_st log;
                if (read_write_log(log_rw_read, (unsigned int)p_read++, &log, __func__, __LINE__, ENABLE) != 0) {
                    p_read--;
                }
            } else {
                loge("undef");
            }
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
#endif //#if (MODULE_DEBUG_pwr_rst_rk_logSave)
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
#if (MODULE_DEBUG_pwr_rst_rk_logSave)
        case 6: { //logd("6, init state");
            init_par_log_write();
            break;
        }
#endif //#if (MODULE_DEBUG_pwr_rst_rk_logSave)
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
#endif
        case 8: { //logd("8, insert");
            log_write(event_insert);
            break;
        }
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
#endif //#if (MODULE_USING_pwr_rst_rk_logSave)//方法

void log_write(event_boot_rst_enum event) {
#if (MODULE_USING_pwr_rst_rk_logSave)
    promt_time(&mix.time, 0, log_level_fatal, __func__, __LINE__, "sys.tm", NULL);

    if (log_w.index <= log_w.index_max) {
        log_w.event = event;
        log_boot_rst_st *new_log = new_a_log_need_free(log_w.index++, log_w.event, log_w.p_write, log_w.tic_delta);
        if (new_log == NULL) {
            loge("err");
        } else {
            // promt_log(new_log, __func__, __LINE__);
            read_write_log(log_rw_write, (unsigned int)log_w.p_write++, new_log, __func__, __LINE__, ENABLE);
            free(new_log);
        }
    } else {
        logf("max");
    }
#else
    ;
#endif //#if (MODULE_USING_pwr_rst_rk_logSave)
}
