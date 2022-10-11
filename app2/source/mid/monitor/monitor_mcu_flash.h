#ifndef __MONITOR_MCU_FLASH_H__
#define __MONITOR_MCU_FLASH_H__

typedef enum {
    event_mcu_boot = 0,

    event_rk_boot_en, //1
    event_rk_boot_un, //2
    event_rk_rst_en,  //3
    event_rk_rst_un,  //4

    event_insert, //5
    event_rk_cnt, //6

    event_mcu_boot_ota,      //7
    event_mcu_boot_tr9_6062, //8
    event_mcu_boot_lcd_menu, //9
    event_mcu_boot_err,      //10

    event_mcu_boot_assert_failed, //11
    event_mcu_boot_HardFault,     //12
    event_mcu_boot_MemManage,     //13
    event_mcu_boot_BusFault,      //14
    event_mcu_boot_UsageFault,    //15

    event_mcu_boot_monitor,   //16
    event_mcu_boot_t_rst_rst, //17
    event_mcu_boot_wait_here, //18

    event_rk_cnt2, //19
} event_boot_rst_enum;

#if (MODULE_USING_pwr_rst_rk_logSave)
int cmd_mcuflash(argv_list_st *argv_list);
void init_par_log_write(void);
#endif //#if (MODULE_USING_pwr_rst_rk_logSave)

void log_write(event_boot_rst_enum event);

#endif /* __MONITOR_MCU_FLASH_H__ */

/* ------------------ rubbish rubbish rubbish rubbish rubbish ------------------ */
/* ------------------ rubbish rubbish rubbish rubbish rubbish ------------------ */
/* ------------------ rubbish rubbish rubbish rubbish rubbish ------------------ */
/* ------------------ rubbish rubbish rubbish rubbish rubbish ------------------ */
/* ------------------ rubbish rubbish rubbish rubbish rubbish ------------------ */
/* ------------------ rubbish rubbish rubbish rubbish rubbish ------------------ */
/* ------------------ rubbish rubbish rubbish rubbish rubbish ------------------ */
#if (0)
/*********************************************************
名    称：mcu_flash_read_buffer
功    能：
输入参数：
输出参数：
返    回：无
注意事项：
*********************************************************/
//ret:0,success;-1,fail
static int read_log(unsigned int addr, log_boot_rst_st *log, const char *func, int line) {
    if (log == NULL) {
        loge("err");
        nop();
        return -1;
    }

    if (!(addr >= ADDR_TEST_MCU_FLASH || addr < ADDR_TEST_MCU_FLASH + 1024 * 2)) {
        loge("err, 0x%p <> 0x%p, 0x%p", addr, ADDR_TEST_MCU_FLASH, ADDR_TEST_MCU_FLASH + 1024 * 2);
        nop();
        return -1;
    }

    unsigned short len = sizeof(log_boot_rst_st);

    if ((len % 4) != 0) {
        loge("err");
        nop();
        return -1;
    }

    if (addr + len >= ADDR_TEST_MCU_FLASH + 1024 * 2) {
        loge("err");
        nop();
        return -1;
    }

    putHexs(__func__, __LINE__, (unsigned char *)&log, sizeof(log_boot_rst_st), 0, -1, -1);

    unsigned int *buf = (unsigned int *)log;
    len /= 4;

    unsigned short i;
    for (i = 0; i < len; i++) {
        buf[i] = *(unsigned int *)(addr + 2 * i);
    }

    putHexs(__func__, __LINE__, (unsigned char *)&log, sizeof(log_boot_rst_st), 0, -1, -1);

    logd("0x%p", addr);

#if (1)
    promt_log(log, func, line);
#endif

    return 0;
}

//ret:0,success;-1,fail
static int write_log(unsigned int addr, log_boot_rst_st *log, const char *func, int line) {
    if (log == NULL) {
        loge("err");
        nop();
        return -1;
    }

    if (!(addr >= ADDR_TEST_MCU_FLASH || addr < ADDR_TEST_MCU_FLASH + 1024 * 2)) {
        loge("err, 0x%p <> 0x%p, 0x%p", addr, ADDR_TEST_MCU_FLASH, ADDR_TEST_MCU_FLASH + 1024 * 2);
        nop();
        return -1;
    }

    unsigned short len = sizeof(log_boot_rst_st);

    if ((len % 4) != 0) {
        loge("err");
        nop();
        return -1;
    }

    if (addr + len >= ADDR_TEST_MCU_FLASH + 1024 * 2) {
        loge("err");
        nop();
        return -1;
    }

    uint32_t *tmp = (uint32_t *)malloc(sizeof(log_boot_rst_st));
    if (tmp == NULL) {
        loge("err");
        nop();
        return -1;
    }

    unsigned short i;

    memcpy((void *)tmp, (const void *)log, sizeof(log_boot_rst_st));

    for (i = 0; i < len; i++) {
        *((unsigned char *)tmp + i) = i + 1;
    }

    putHexs(__func__, __LINE__, (unsigned char *)tmp, sizeof(log_boot_rst_st), 0, -1, -1);

    uint32_t *buf = (uint32_t *)tmp;
    len /= 4;

    for (i = 0; i < len; i++) {
        logd("1. 0x%p[0x%08X]", addr + 4 * i, *(uint32_t *)(addr + 4 * i));
    }

    for (i = 0; i < len; i++) {
#if (0)
        FLASH_ProgramHalfWord(addr + 2 * i, buf[i]);
#else
        logd("2. 0x%p[0x%08X, 0x%08X]", addr + 4 * i, *(uint32_t *)(addr + 4 * i), *(uint32_t *)(buf + i));
        FLASH_Unlock();
        if (FLASH_ProgramWord(addr + 4 * i, *(uint32_t *)(buf + i)) == FLASH_COMPLETE) {
            logd("3. 0x%p[0x%08X, 0x%08X]", addr + 4 * i, *(uint32_t *)(addr + 4 * i), *(uint32_t *)(buf + i));
            //判断写入的数据是否正确
            if (*(uint32_t *)(addr + 4 * i) != *(uint32_t *)(buf + i)) {
                loge("w dat err");
                free(tmp);
                FLASH_Lock();
                return -1;
            }
        } else {
            loge("w dat err");
            free(tmp);
            FLASH_Lock();
            return -1;
        }
#endif
    }
    FLASH_Lock();

    free(tmp);

    logd("0x%p", addr);

#if (1)
    promt_log(log, func, line);
#endif

    return 0;
}
#endif
