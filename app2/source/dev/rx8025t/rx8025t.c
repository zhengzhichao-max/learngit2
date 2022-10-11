
/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-20     BruceOu      first implementation
 */

#include "include_all.h"

#ifndef LOG_D
#define LOG_D logd
#endif

#ifndef LOG_I
#define LOG_I logi
#endif

#ifndef LOG_W
#define LOG_W logw
#endif

#ifndef LOG_E
#define LOG_E loge
#endif

// #include <stdio.h>
// #include <string.h>
// #include <rtthread.h>
// #include <rtdevice.h>
// #include <board.h>
// #include "i2c.h"
// #include "qst_sw_i2c.h"
// #include "rx8025t.h"
// #include "gd32f30x_rcu.h"

/* ------------------------------------ */
// #define LOG_TAG "rx8025t"
// #define LOG_LVL LOG_LVL_DBG //LOG_LVL_ERROR//LOG_LVL_DBG
// #include <ulog.h>
/* ------------------------------------ */

/* ------------------------------------ */
//#define THREAD_PRIORITY         8
//#define THREAD_TIMESLICE        5

//ALIGN(RT_ALIGN_SIZE)
//static char thread1_stack[1024];
//static struct rt_thread thread1;
//static void rt_thread_entry1(void *parameter){
//	int count = 0;
//    while (1){
//        rt_thread_mdelay(1000*3);
//		LOG_E("rt_thread_entry1 %d.", ++count);
//    }
//}
/* ------------------------------------ */

TIME_ST RTC_Time; //存储日期时间的结构体变量
uint8_t Time_Update = 0;
//char a[10] = "";

static void i2c_sw_gpio_config(void) {
    loge("undef, and, no need");
}

//--------------------------------RX8025T的GPIO初始化，不含中断引脚，中断初始化在exti.c中---------------------------------
//rt_err_t//RT_EOK//RT_ERROR
rt_err_t rx8025t_init(void) {
    static int err = 0, ok = 0;
    rt_uint8_t val[3] = {0x00, 0x00, 0x60}; //0x0D、0x0E、0x0F、三个寄存器的值，设置时间更新为“秒”更新，关闭所有闹钟，温补时间为2秒，打开时间更新中断，关闭其他中断。

    i2c_sw_gpio_config(); //iic接口初始化

    rt_err_t ret = Write8025TData(RX8025T_EXT_REG, val, 3);
    if (ret == RT_EOK) {
        LOG_I("rx8025t init ok (ok, err) = (%d, %d), ", ++ok, err);
        return RT_EOK;
    }

    LOG_E("rx8025t init err (ok, err) = (%d, %d), ", ok, ++err);
    return ret;
}

static rt_err_t rx8025t_read(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len) {
    IIC_Read(RX8025T_SLAVE_ADDR, (addr << 4 | 0x00), buf, len,"rx8025t");
    return RT_EOK;
}

static rt_err_t rx8025t_write(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len) {
    IIC_Write(RX8025T_SLAVE_ADDR, (addr << 4 | 0x00), buf, len, "rx8025t");
    return RT_EOK;
}

/*******************************************************************************
* 函数名: rt_uint8_t Write8025TData(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len)
* 描述  : 写RX8025T寄存器
* 参数  : addr寄存器地址，*buf写入的数据，len写入的长度
* 返回值: //rt_err_t//RT_EOK//RT_ERROR
*******************************************************************************/
rt_err_t Write8025TData(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len) {
    return rx8025t_write(addr, buf, len);
}

/*******************************************************************************
* 函数名: rt_uint8_t Read8025TData(rt_uint8_t addr,rt_uint8_t *buf,rt_uint8_t len)
* 描述  : 读RX8025T寄存器
* 参数  : addr寄存器地址，*buf存储位置，len读取的长度
* 返回值: //rt_err_t//RT_EOK//RT_ERROR
*******************************************************************************/
rt_err_t Read8025TData(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len) {
    return rx8025t_read(addr, buf, len);
}

/*******************************************************************************
* 函数名: rt_uint8_t get_rtc_time(TIME_ST* t)
* 描述  : 从RX8025T获取时间 
* 参数  : 存储时间的结构体
* 返回值: //rt_err_t//RT_EOK//RT_ERROR
*******************************************************************************/
rt_err_t get_rtc_time(TIME_ST *t) {
    rt_uint8_t rtc_str[7] = {0};

    if (Read8025TData(RX8025T_SEC_REG, rtc_str, 7) == RT_EOK) { //获取日期与时间
        t->second = ((rtc_str[0] >> 4) * 10) + (rtc_str[0] & 0x0f);
        t->minute = ((rtc_str[1] >> 4) * 10) + (rtc_str[1] & 0x0f);
        t->hour = ((rtc_str[2] >> 4) * 10) + (rtc_str[2] & 0x0f);
        t->week = rtc_str[3];
        t->day = ((rtc_str[4] >> 4) * 10) + (rtc_str[4] & 0x0f);
        t->month = ((rtc_str[5] >> 4) * 10) + (rtc_str[5] & 0x0f);
        t->year = ((rtc_str[6] >> 4) * 10) + (rtc_str[6] & 0x0f);
        return RT_EOK;
    } else {
        memset((void *)t, 0x00, sizeof(TIME_ST));
        return RT_ERROR; //读取出错
    }
}
/*******************************************************************************
* 函数名: rt_uint8_t set_rtc_time(TIME_ST* t)
* 描述  : 设置RX8025T时间 
* 参数  : 存储时间的结构体
* 返回值: 0成功，1失败。
*******************************************************************************/
rt_err_t set_rtc_time(TIME_ST *t) {
    rt_uint8_t rtc_str[7];

    rtc_str[0] = ((t->second / 10) << 4) | (t->second % 10);
    rtc_str[1] = ((t->minute / 10) << 4) | (t->minute % 10);
    rtc_str[2] = ((t->hour / 10) << 4) | (t->hour % 10);
    rtc_str[3] = t->week;
    rtc_str[4] = ((t->day / 10) << 4) | (t->day % 10);
    rtc_str[5] = ((t->month / 10) << 4) | (t->month % 10);
    rtc_str[6] = ((t->year / 10) << 4) | (t->year % 10);

    return Write8025TData(RX8025T_SEC_REG, rtc_str, 7);
}

void rx8025t_show(TIME_ST *t) { //打印时间
    LOG_W("20%02d-%02d-%02d %02d:%02d:%02d", t->year, t->month, t->day, t->hour, t->minute, t->second);
}

rt_err_t rx8025t_setRtcTime_test(void) {
    TIME_ST t = {.year = 22, .month = 8, .day = 6, .hour = 17, .minute = 36, .second = 0};
    TIME_ST *tt = &t;
    LOG_I("set time test: 20%02d-%02d-%02d %02d:%02d:%02d", tt->year, tt->month, tt->day, tt->hour, tt->minute, tt->second);
    rt_err_t ret = set_rtc_time(&t);

#if (0)
    if (ret == RT_EOK) {
        LOG_D("setRtcTime ok");
    } else {
        LOG_E("setRtcTime err");
    }
#endif

    return ret;
}
//MSH_CMD_EXPORT(setRtcTime, set rx8025t time);
//FINSH_FUNCTION_EXPORT_ALIAS("cThreadSample", create_thread_sample, create thread sample);
//FINSH_FUNCTION_EXPORT_ALIAS(create_thread_sample, cThreadSample, create thread sample);

/*******************************************************************************
* 函数名: rt_uint8_t BCD2HEX(rt_uint8_t bcd_data) 
* 描述  : BCD转为HEX 
* 参数  : BCD码
* 返回值: HEX码
*******************************************************************************/
rt_uint8_t BCD2HEX(rt_uint8_t bcd_data) {
    rt_uint8_t temp;
    temp = (bcd_data >> 4) * 10 + (bcd_data & 0x0F);
    return temp;
}

/*******************************************************************************
* 函数名: rt_uint8_t HEX2BCD(rt_uint8_t hex_data) 
* 描述  : HEX转为BCD  
* 参数  : -hex_data:十六进制数
* 返回值: BCD码
*******************************************************************************/
rt_uint8_t HEX2BCD(rt_uint8_t hex_data) {
    rt_uint8_t temp;
    temp = ((hex_data / 10) << 4) + (hex_data % 10);
    return temp;
}
