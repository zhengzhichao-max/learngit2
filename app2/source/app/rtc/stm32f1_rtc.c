//**************************************************************************************************************//
//**************************************************************************************************************//
//----------------                             STM32F1_RTC处理函数                             ----------------//
//
//
//CKP
//编写日期  2018-03-08
//
//**************************************************************************************************************//
#include "include_all.h"


const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static u8 is_leap_year(u16 year)
{
    if (year % 4 == 0)      /* 必须能被4整除 */
    {
        if (year % 100 == 0)
        {
            if (year % 400 == 0)
                return 1;   /* 如果以00结尾,还要能被400整除*/
            else
                return 0;
        }
        else
            return 1;
    }
    else
        return 0;
}

/* 设置RTC日历数据 */
void stm32f1_rtc_set(time_t * src_rtc)
{
    u16 t, tyear;
    u32 seccount = 0;

    tyear = (2000 + src_rtc->year);
    for (t = 1970; t < tyear; t++)      /* 把所有年份的秒钟相加 */
    {
        if (is_leap_year(t))
        {
            seccount += 31622400;       /* 闰年的秒钟数 */
        }
        else
        {
            seccount += 31536000;       /* 平年的秒钟数 */
        }
    }

    src_rtc->month -= 1;
    for (t = 0; t < src_rtc->month; t++)        /* 把前面月份的秒钟数相加 */
    {
        seccount += (u32)mon_table[t] * 86400;  /* 月份秒钟数相加 */
        if (is_leap_year(tyear) && (t == 1))
        {
            seccount += 86400;                  /* 闰年2月份增加一天的秒钟数 */
        }
    }

    seccount += (u32)(src_rtc->date - 1) * 86400;   /* 把前面日期的秒钟数相加 */
    seccount += (u32)src_rtc->hour * 3600;          /* 小时秒钟数 */
    seccount += (u32)src_rtc->min * 60;             /* 分钟秒钟数 */
    seccount += src_rtc->sec;                       /* 最后的秒钟加上去 */

    RTC_SetCounter(seccount);   /* 设置RTC计数器的值 */
    RTC_WaitForLastTask();      /* 等待最近一次对RTC寄存器的写操作完成 */
}

/* 获取RTC日历数据 */
void stm32f1_rtc_get(time_t * dst_rtc)
{
    u32 timecount = 0;
    u32 temp  = 0;
    u16 temp1 = 0, tyear = 0;

    timecount = RTC_GetCounter();

    temp = timecount / 86400;       /* 得到天数(秒钟数对应的) */
    temp1 = 1970;                   /* 从1970年开始 */
    while (temp >= 365)
    {
        if (is_leap_year(temp1))    /* 是闰年 */
        {
            if (temp >= 366)
            {
                temp -= 366;        /* 闰年的秒钟数 */
            }
            else
            {
                temp1++;
                break;
            }
        }
        else
        {
            temp -= 365;            /* 平年 */
        }
        temp1++;
    }
    tyear = temp1;          /* 得到年份 */

    temp1 = 0;
    while (temp >= 28)              /* 超过了一个月 */
    {
        if (is_leap_year(tyear) && (temp1 == 1))  /* 当年是不是闰年2月份 */
        {
            if (temp >= 29)
            {
                temp -= 29;         /* 闰年的秒钟数 */
            }
            else
            {
                break;
            }
        }
        else
        {
            if (temp >= mon_table[temp1])
            {
                temp -= mon_table[temp1];  /* 平年 */
            }
            else
            {
                break;
            }
        }
        temp1++;
    }
    dst_rtc->month  = temp1 + 1;        /* 得到月份 */
    dst_rtc->date   = temp + 1;         /* 得到日期 */

    temp = timecount % 86400;     		/* 得到秒钟数 */
    dst_rtc->hour = temp / 3600;     	/* 小时 */
    dst_rtc->min  = (temp % 3600) / 60; /* 分钟 */
    dst_rtc->sec  = (temp % 3600) % 60; /* 秒钟 */
    dst_rtc->year = (tyear - 2000);
}

/* RTC外设模块初始化 */
/* 函数返回值  TRUE执行成功  FALSE执行失败 */
bit_enum stm32f1_rtc_init(void)
{
    u8 w = 200;
    bit_enum sta = _true_;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  /* 使能PWR和BKP外设时钟 */
    PWR_BackupAccessCmd(ENABLE);                                              /* 使能后备寄存器访问 */

    if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)
    {
        BKP_DeInit();                               /* 复位备份区域 */
        RCC_LSEConfig(RCC_LSE_ON);                  /* 设置外部低速晶振(LSE),使用外设低速晶振 */
        while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (w))  /* 等待低速晶振就绪 */
        {
            w -= 1;
            delayms(10 * 9);
        }
        if (w == 0)
        {
            sta = _false_;                          /* LSE 开启失败 */
            BKP_WriteBackupRegister(BKP_DR1, 0);
        }

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);     /* 设置RTC时钟(RTCCLK),选择LSE作为RTC时钟 */
        RCC_RTCCLKCmd(ENABLE);                      /* 使能RTC时钟 */
        RTC_WaitForLastTask();                      /* 等待最近一次对RTC寄存器的写操作完成 */
        RTC_WaitForSynchro();                       /* 等待RTC寄存器同步 */

        RTC_SetPrescaler(32767);                    /* 设置RTC预分频的值 */
        RTC_WaitForLastTask();                      /* 等待最近一次对RTC寄存器的写操作完成 */

        if (_true_ == sta)
        {
            BKP_WriteBackupRegister(BKP_DR1, 0x5050);
        }
    }

    return sta;
}



