//**************************************************************************************************************//
//**************************************************************************************************************//
//----------------                             STM32F1_RTC������                             ----------------//
//
//
//CKP
//��д����  2018-03-08
//
//**************************************************************************************************************//
#include "include_all.h"


const u8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static u8 is_leap_year(u16 year)
{
    if (year % 4 == 0)      /* �����ܱ�4���� */
    {
        if (year % 100 == 0)
        {
            if (year % 400 == 0)
                return 1;   /* �����00��β,��Ҫ�ܱ�400����*/
            else
                return 0;
        }
        else
            return 1;
    }
    else
        return 0;
}

/* ����RTC�������� */
void stm32f1_rtc_set(time_t * src_rtc)
{
    u16 t, tyear;
    u32 seccount = 0;

    tyear = (2000 + src_rtc->year);
    for (t = 1970; t < tyear; t++)      /* ��������ݵ�������� */
    {
        if (is_leap_year(t))
        {
            seccount += 31622400;       /* ����������� */
        }
        else
        {
            seccount += 31536000;       /* ƽ��������� */
        }
    }

    src_rtc->month -= 1;
    for (t = 0; t < src_rtc->month; t++)        /* ��ǰ���·ݵ���������� */
    {
        seccount += (u32)mon_table[t] * 86400;  /* �·���������� */
        if (is_leap_year(tyear) && (t == 1))
        {
            seccount += 86400;                  /* ����2�·�����һ��������� */
        }
    }

    seccount += (u32)(src_rtc->date - 1) * 86400;   /* ��ǰ�����ڵ���������� */
    seccount += (u32)src_rtc->hour * 3600;          /* Сʱ������ */
    seccount += (u32)src_rtc->min * 60;             /* ���������� */
    seccount += src_rtc->sec;                       /* �������Ӽ���ȥ */

    RTC_SetCounter(seccount);   /* ����RTC��������ֵ */
    RTC_WaitForLastTask();      /* �ȴ����һ�ζ�RTC�Ĵ�����д������� */
}

/* ��ȡRTC�������� */
void stm32f1_rtc_get(time_t * dst_rtc)
{
    u32 timecount = 0;
    u32 temp  = 0;
    u16 temp1 = 0, tyear = 0;

    timecount = RTC_GetCounter();

    temp = timecount / 86400;       /* �õ�����(��������Ӧ��) */
    temp1 = 1970;                   /* ��1970�꿪ʼ */
    while (temp >= 365)
    {
        if (is_leap_year(temp1))    /* ������ */
        {
            if (temp >= 366)
            {
                temp -= 366;        /* ����������� */
            }
            else
            {
                temp1++;
                break;
            }
        }
        else
        {
            temp -= 365;            /* ƽ�� */
        }
        temp1++;
    }
    tyear = temp1;          /* �õ���� */

    temp1 = 0;
    while (temp >= 28)              /* ������һ���� */
    {
        if (is_leap_year(tyear) && (temp1 == 1))  /* �����ǲ�������2�·� */
        {
            if (temp >= 29)
            {
                temp -= 29;         /* ����������� */
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
                temp -= mon_table[temp1];  /* ƽ�� */
            }
            else
            {
                break;
            }
        }
        temp1++;
    }
    dst_rtc->month  = temp1 + 1;        /* �õ��·� */
    dst_rtc->date   = temp + 1;         /* �õ����� */

    temp = timecount % 86400;     		/* �õ������� */
    dst_rtc->hour = temp / 3600;     	/* Сʱ */
    dst_rtc->min  = (temp % 3600) / 60; /* ���� */
    dst_rtc->sec  = (temp % 3600) % 60; /* ���� */
    dst_rtc->year = (tyear - 2000);
}

/* RTC����ģ���ʼ�� */
/* ��������ֵ  TRUEִ�гɹ�  FALSEִ��ʧ�� */
bit_enum stm32f1_rtc_init(void)
{
    u8 w = 200;
    bit_enum sta = _true_;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  /* ʹ��PWR��BKP����ʱ�� */
    PWR_BackupAccessCmd(ENABLE);                                              /* ʹ�ܺ󱸼Ĵ������� */

    if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)
    {
        BKP_DeInit();                               /* ��λ�������� */
        RCC_LSEConfig(RCC_LSE_ON);                  /* �����ⲿ���پ���(LSE),ʹ��������پ��� */
        while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (w))  /* �ȴ����پ������ */
        {
            w -= 1;
            delayms(10 * 9);
        }
        if (w == 0)
        {
            sta = _false_;                          /* LSE ����ʧ�� */
            BKP_WriteBackupRegister(BKP_DR1, 0);
        }

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);     /* ����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ�� */
        RCC_RTCCLKCmd(ENABLE);                      /* ʹ��RTCʱ�� */
        RTC_WaitForLastTask();                      /* �ȴ����һ�ζ�RTC�Ĵ�����д������� */
        RTC_WaitForSynchro();                       /* �ȴ�RTC�Ĵ���ͬ�� */

        RTC_SetPrescaler(32767);                    /* ����RTCԤ��Ƶ��ֵ */
        RTC_WaitForLastTask();                      /* �ȴ����һ�ζ�RTC�Ĵ�����д������� */

        if (_true_ == sta)
        {
            BKP_WriteBackupRegister(BKP_DR1, 0x5050);
        }
    }

    return sta;
}



