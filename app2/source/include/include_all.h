#ifndef __INCLUDE_ALL_H
#define __INCLUDE_ALL_H

//系统定义
#include "stm32f10x.h"
#include "define.h"
#include "stdio.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "systick.h"
#include "jt808_type.h"
#include "pub.h"
#include "logger.h"
#include "clib.h"
#include "systick.h"
#include "ckp_mcu_file.h"

//ST LIB 库文件定义
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_can.h"

//USB
#if (0) //USB
#include "usb_core.h"
#include "usb_conf.h"
#include "usbd_desc.h"
#include "usb_defines.h"
#include "usb_dcd.h"
#include "usbd_req.h"
#include "usbd_ioreq.h"
#include "bsp_usb.h"
#endif

//mid/monitor
#if (1)
#include "monitor.h"
#include "monitor_table.h"
#include "monitor_mcu_flash.h"
#include "monitor_util.h"
#endif

//drv
#include "drv_test.h"
#include "drv_cfg_tr9.h"
#include "adc.h"
#include "uart_pub.h"
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "gsm_drv.h"
#include "gps_drv.h"
#include "I2C.h"
#include "spi_flash.h"

//device
#include "dev_test.h"
#include "rx8025t.h"
#include "lsm6ds3trc.h"

//mid
#include "mid_test.h"
#include "mid_cfg.h"
#include "hard_tr9_define.h"
#include "hard_config.h"
#include "lcd13264_ram.h"
#include "call_record.h"
#include "card_scan.h"
#include "data_interaction.h"
#include "integer.h"
#include "key_state.h"
#include "lcd13264_ram.h"
#include "menu_process_enum.h"
#include "menu_process.h"
#include "phone_book.h"
#include "recorder.h"
#include "sms_process.h"
#include "ic4442_driver.h"
#include "print_driver.h"
#include "recorder.h"
#include "iic_base_driver.h"
#include "at24cxx_driver.h"
#include "gbt_19056_2012_biao_c_1_ic_card.h"
#include "sms_parse.h"
#include "gps_manage.h"
#include "gsm_any.h"
#include "handset.h"
#include "send.h"
#include "report.h"
#include "fatfs_fname_gbk.h"
#include "upgrade.h"

//APP 应用层定义
#include "debug.h"
#include "app_cfg.h"
#include "jt808_recive.h"
#include "jt808_parse.h"
#include "jt808_package.h"
#include "jt808_manage.h"
#include "print.h"
#include "rcd_manage.h"
#include "rcd_communicate.h"
#include "speed_limit.h"
//#include "MMA8451.h"
#if (TRWY_PRODUCT == M9C19A)
#include "tts.h"
#endif
#include "can.h"
#include "weight.h"
#include "tr9_frame.h"
#include "tr9_agree.h"
#include "self_chk.h"

//task
#include "lsm6ds3trc_task.h"

//test
#include "test.h"

#endif /* __INCLUDE_ALL_H */
