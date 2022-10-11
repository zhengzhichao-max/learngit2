
#include "include_all.h"
#include "lsm6ds3trc_type.h"
#include "lsm6ds3trc.h"

#if (MODULE_USING_lsm6ts3trc)
#pragma region 接口定义
#define flag_lsm_pwr_on sys_bits1.bits.flg_lsm_pwr_on
#define flag_lsm_read sys_bits1.bits.flg_lsm_read

//void tr9_frame_pack2rk(u16 uId, u8 *buf, u16 len)
#define pack_2_rk tr9_frame_pack2rk
#pragma endregion 接口定义

/*
    固定内存
    1000毫秒50包
    200ms发一次，【10包/次】
    1秒发5次
    时间累加器，单位ms
    非时间戳
*/
typedef struct {
    unsigned int time;
    //200ms
    //【1包/20ms】
    //10包
    //[0-9][10-19][10-19][20-29][30-39][40-49]
    lsm_dat_st arr[10];
} lsm_data_st;
#endif //#if (MODULE_USING_lsm6ts3trc)

void lsm6ds3tr_c_task(void) {
#if (MODULE_USING_lsm6ts3trc)
#define DEBUG_HERE_2022009081843
#undef DEBUG_HERE_2022009081843

    static struct {
        unsigned short init : 1;
        unsigned short tic_init : 4;
        unsigned short index_slave : 1;
        unsigned short repeat : 2;
        unsigned short cnt_err : 2;
        //unsigned short ic_slave_addr_ok : 2;
        unsigned short frame : 3;
        unsigned short pack : 4;
    } lsm = {.init = 0, .tic_init = 0, .index_slave = 0, .frame = 4, .pack = 9}; // .ic_slave_addr_ok = 0,

    if (sys_bits1.bits.flg_rk_1717_first == 0) {
        return;
    }
#if (0)
    else {
        static unsigned int tic_here = 0;
        if (_covern(tic_here) > 5) {
            loge("1");
        }
    }
#endif

    if (flag_lsm_pwr_on) {
        if (flag_lsm_read) {
#if (1)
            static unsigned int dat_old = 0;
            static lsm_dat_st lsm_dat_old = {0};
            static unsigned int pack = 0;
            static lsm_data_st lsm_data = {0};
            // static unsigned char slave_addr = 0x6A;
            const unsigned char slave_addr[] = {0x6A, 0x6B};
#endif

            flag_lsm_read = 0;

            pack++;
            if (++lsm.pack >= 10) {
                lsm.pack = 0;
            }

            if (lsm.init != 1) {
                if (++lsm.tic_init == 1) {
                    if (LSM6DS3TRC_Initialize(slave_addr[++lsm.index_slave], LSM6DS3TRC_ACC_FSXL_2G, LSM6DS3TRC_GYR_FSG_2000, LSM6DS3TRC_ACC_RATE_104HZ, LSM6DS3TRC_GYR_RATE_104HZ) == 0) {
                        logi("lsm init ok");
                        lsm.init = 1;
                        lsm.cnt_err = 0;
                    }
                }
            }

            if (lsm.init == 1) {
                lsm_dat_st lsm_dat_read = {0};
                lsm.repeat = 0;
                do {
                    if (LSM6DS3TRC_get_acc_gyr_ok(&lsm_dat_read) == 0) {
                        break;
                    }
                } while (++lsm.repeat != 3);

                if (lsm.repeat < 3) {
                    memcpy((void *)&lsm_data.arr[lsm.pack], (const void *)&lsm_dat_read, sizeof(lsm_dat_st));
                    memcpy((void *)&lsm_dat_old, (const void *)&lsm_dat_read, sizeof(lsm_dat_st));

                    lsm.cnt_err = 0;
                } else {
                    memcpy((void *)&lsm_data.arr[lsm.pack], (const void *)&lsm_dat_old, sizeof(lsm_dat_st));
                    loge("lsm_dat_old %d/%d, 1", ++dat_old, pack);

                    if (++lsm.cnt_err >= 3) {
                        loge("(++lsm.cnt_err >= 3)");
                        lsm.init = 0;
                    }
                }
            } else { //(lsm.init == 1) {
                memcpy((void *)&lsm_data.arr[lsm.pack], (const void *)&lsm_dat_old, sizeof(lsm_dat_st));
                loge("lsm_dat_old %d/%d, 2", ++dat_old, pack);
            }

            if (lsm.pack == 9) {
                if (++lsm.frame >= 5) {
                    lsm.frame = 0;
                }

                lsm_data.time = jiffies;

                pack_2_rk(tr9_cmd_6065, (u8 *)&lsm_data, sizeof(lsm_data_st));
                if (lsm.frame == 4) {
                    logir("lsm 2 rk %u", pack);
                }
            }
        }
    } else { // if (flag_lsm_pwr_on) {
        lsm.init = 0;
    }
#endif //#if (MODULE_USING_lsm6ts3trc)
}

#if (0)
void lsm6ds3tr_c_task_2(void) {
#define DEBUG_HERE_2022009081843
#undef DEBUG_HERE_2022009081843

    static lsm_data_st lsm_data = {0};
    // static unsigned char slave_addr = 0x6A;
    const unsigned char slave_addr[] = {0x6A, 0x6B};
    static unsigned int pack = 0;
    static lsm_dat_st lsm_dat_old = {0};
    static struct {
        unsigned short init : 1;
        unsigned short tic_init : 4;
        unsigned short index_slave : 1;
        unsigned short repeat : 2;
        unsigned short cnt_err : 2;
        //unsigned short ic_slave_addr_ok : 2;
        unsigned short frame : 3;
        unsigned short pack : 4;
    } lsm = {.init = 0, .tic_init = 0, .index_slave = 0, .frame = 4, .pack = 9}; // .ic_slave_addr_ok = 0,

    if (flag_lsm_pwr_on) {
        if (flag_lsm_read) {
            flag_lsm_read = 0;

            if (lsm.init != 1) {
                if (++lsm.tic_init == 1) {
                    if (LSM6DS3TRC_Initialize(slave_addr[++lsm.index_slave], LSM6DS3TRC_ACC_FSXL_2G, LSM6DS3TRC_GYR_FSG_2000, LSM6DS3TRC_ACC_RATE_104HZ, LSM6DS3TRC_GYR_RATE_104HZ) == 0) {
                        logi("lsm init ok");
                        lsm.init = 1;
                        lsm.cnt_err = 0;
                    }
                }
            }

            pack++;
            if (++lsm.pack >= 10) {
                lsm.pack = 0;
            }

            if (lsm.init == 1 /*&& lsm.ic_slave_addr_ok == 3*/) {
                //unsigned char buf_read[12] = {0};
                lsm_dat_st lsm_dat_read = {0};
                lsm.repeat = 0;
                do {
                    if (LSM6DS3TRC_get_acc_gyr_ok(&lsm_dat_read) == 0) {
                        break;
                    }
                } while (++lsm.repeat != 3);

                if (lsm.repeat < 3) {
                    memcpy((void *)&lsm_data.arr[lsm.pack], (const void *)&lsm_dat_read, sizeof(lsm_dat_st));
                    memcpy((void *)&lsm_dat_old, (const void *)&lsm_dat_read, sizeof(lsm_dat_st));
                    lsm.cnt_err = 0;
                } else {
                    static unsigned int dat_old = 0;
                    memcpy((void *)&lsm_data.arr[lsm.pack], (const void *)&lsm_dat_old, sizeof(lsm_dat_st));
                    loge("lsm_dat_old %d/%d", ++dat_old, pack);
                    if (++lsm.cnt_err >= 3) {
                        loge("(++lsm.cnt_err >= 3)");
                        lsm.init = 0;
                    }
                }

                if (lsm.pack == 9) {
                    if (++lsm.frame >= 5) {
                        lsm.frame = 0;
                    }

                    lsm_data.time = jiffies;

                    pack_2_rk(tr9_cmd_6065, (u8 *)&lsm_data, sizeof(lsm_data_st));
                    if (lsm.frame == 4) {
                        logir("lsm 2 tr9 %u", pack);
                    }
                }
            } else if (lsm.init != 1) {
                if (++lsm.pack == 15) { //借用 lsm.pack 控制初始化节奏
                    lsm.frame++;        //借用 lsm.frame 选择从机地址
                    //loge("sizeof(slave_addr) = %d", sizeof(slave_addr));//测长
                    if (LSM6DS3TRC_Initialize(slave_addr[lsm.frame % sizeof(slave_addr)], LSM6DS3TRC_ACC_FSXL_2G, LSM6DS3TRC_GYR_FSG_2000, LSM6DS3TRC_ACC_RATE_104HZ, LSM6DS3TRC_GYR_RATE_104HZ) == 0) {
                        logi("-------------------------lsm init ok");
                        lsm.init = 1;
                        lsm.frame = 14;
                        lsm.pack = 14;
                        lsm.cnt_err = 0;
                    } /*else {
                    //loge("lsm init err");
                    if (slave_addr == 0x6A) {
                        slave_addr = 0x6B;
                    } else {
                        slave_addr = 0x6A;
                    }
                }*/
                }
            } /*else if (lsm.ic_slave_addr_ok != 3) {
            if (++lsm.pack == 15) {
                if (LSM6DS3TRC_GetChipID() == 0) {
                    if (++lsm.ic_slave_addr_ok == 3) {
                        lsm.pack = 0;
                    }
                } else {
                    lsm.ic_slave_addr_ok = 0;
                }
            }
        }*/
        }
    } else if (flg_lsm_pwr_on) {
        if (flg_lsm_read) {
            flg_lsm_read = 0;

            if (lsm.init == 1 /*&& lsm.ic_slave_addr_ok == 3*/) {
                if (++lsm.pack >= 10) {
                    lsm.pack = 0;
                }

                pack++;

#ifndef DEBUG_HERE_2022009081843
                //unsigned char buf_read[12] = {0};
                lsm_dat_st lsm_dat_read = {0};
                lsm.repeat = 0;
                do {
                    if (LSM6DS3TRC_get_acc_gyr_ok(&lsm_dat_read) == 0) {
                        break;
                    }
                } while (++lsm.repeat != 3);

                if (lsm.repeat < 3) {
                    memcpy((void *)&lsm_data.arr[lsm.pack], (const void *)&lsm_dat_read, sizeof(lsm_dat_st));
                    memcpy((void *)&lsm_dat_old, (const void *)&lsm_dat_read, sizeof(lsm_dat_st));
                    lsm.cnt_err = 0;
                } else {
                    static unsigned int dat_old = 0;
                    memcpy((void *)&lsm_data.arr[lsm.pack], (const void *)&lsm_dat_old, sizeof(lsm_dat_st));
                    loge("lsm_dat_old %d/%d", ++dat_old, pack);
                    if (++lsm.cnt_err >= 3) {
                        loge("(++lsm.cnt_err >= 3)");
                        lsm.init = 0;
                    }
                }
#else
                if (1) {
                    int i = 0;

                    for (; i < 12; i++) {
                        buf[i] = lsm.pack;
                    }

                    memcpy((void *)&lsm_data.buf[lsm.pack][0], (const void *)buf, 12);
                }
#endif

                if (lsm.pack == 9) {
#if (1)
                    if (++lsm.frame >= 5) {
                        lsm.frame = 0;
                    }

                    lsm_data.time = jiffies;
#else
                    lsm_data.time = jiffies;
#endif

#ifdef DEBUG_HERE_2022009081843
                    int i = 0;
                    for (; i < 10; i++) {
                        logi("lsm_data.buf[%d][0] = %d", i, lsm_data.buf[i][0]);
                    }
                    //logi("2 tr9 data lsm, frame %d, jiffies %u", lsm_data.frame, jiffies);
#endif

                    /*
            如果出现两次frame数据相同，请仅使用第一份
            */
                    pack_2_rk(tr9_cmd_6065, (u8 *)&lsm_data, sizeof(lsm_data_st));
                    if (lsm.frame == 4) {
                        logir("lsm 2 tr9 %u", pack);
                    }

#ifdef DEBUG_HERE_2022009081843
                    memset((void *)&lsm_data, 0, sizeof(lsm_data));
                    for (i = 0; i < 10; i++) {
                        logi("2.lsm_data.buf[%d][0] = %d", i, lsm_data.buf[i][0]);
                    }
#endif
                }
            } else if (lsm.init != 1) {
                if (++lsm.pack == 15) { //借用 lsm.pack 控制初始化节奏
                    lsm.frame++;        //借用 lsm.frame 选择从机地址
                    //loge("sizeof(slave_addr) = %d", sizeof(slave_addr));//测长
                    if (LSM6DS3TRC_Initialize(slave_addr[lsm.frame % sizeof(slave_addr)], LSM6DS3TRC_ACC_FSXL_2G, LSM6DS3TRC_GYR_FSG_2000, LSM6DS3TRC_ACC_RATE_104HZ, LSM6DS3TRC_GYR_RATE_104HZ) == 0) {
                        logi("-------------------------lsm init ok");
                        lsm.init = 1;
                        lsm.frame = 14;
                        lsm.pack = 14;
                        lsm.cnt_err = 0;
                    } /*else {
                    //loge("lsm init err");
                    if (slave_addr == 0x6A) {
                        slave_addr = 0x6B;
                    } else {
                        slave_addr = 0x6A;
                    }
                }*/
                }
            } /*else if (lsm.ic_slave_addr_ok != 3) {
            if (++lsm.pack == 15) {
                if (LSM6DS3TRC_GetChipID() == 0) {
                    if (++lsm.ic_slave_addr_ok == 3) {
                        lsm.pack = 0;
                    }
                } else {
                    lsm.ic_slave_addr_ok = 0;
                }
            }
        }*/
        }

    } else if (!flg_lsm_pwr_on && lsm.init == 1) {
        lsm.init = 0;
    }
}
#endif
