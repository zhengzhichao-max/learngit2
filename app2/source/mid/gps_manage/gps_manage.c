/**
  ******************************************************************************
  * @file    gps_manage.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GPS ģ�����
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

static bool gps_reset = false; //GPS ��λ
u16 display_speed = 0;

GPS_BASE_INFOMATION gps_base; //GPS ����λ��
GPS_EXT_INFOMATION gps_ext;   //GPS ��չ��Ϣ
u16 save_gps_pos = MAX_GPS_SIZE;
bool positioned_update = false;
/*
 ******�п�΢ģ������ָ��******
��Ϣ CAS03
���� ����Ҫ�������ֹͣ����� NMEA ��䡣
���� ����
��ʽ $PCAS03,nGGA,nGLL,nGSA,nGSV,nRMC,nVTG,nZDA,nTXT*CS<CR><LF>
ʾ�� $PCAS03,1,1,1,1,1,1,0,1*03

����˵��
�ֶ� ���� ��ʽ ����˵��
1 $PCAS03 �ַ��� ��Ϣ ID�����ͷ
2 nGGA ��ֵ GGA ���Ƶ�ʣ� ������Ƶ�����Զ�λ������Ϊ��
׼�ģ� n��0~9����ʾÿ n �ζ�λ���һ�Σ� 0 ��ʾ
���������䣬���򱣳�ԭ�����á�
3 nGLL ��ֵ GLL ���Ƶ�ʣ� ͬ nGGA
4 nGSA ��ֵ GSA ���Ƶ�ʣ� ͬ nGGA
5 nGSV ��ֵ GSV ���Ƶ�ʣ� ͬ nGGA
6 nRMC ��ֵ RMC ���Ƶ�ʣ� ͬ nGGA
7 nVTG ��ֵ VTG ���Ƶ�ʣ� ͬ nGGA
8 nZDA ��ֵ ZDA ���Ƶ�ʣ� ͬ nGGA
9 nTXT ��ֵ TXT ���Ƶ�ʣ� ͬ nGGA
10 CS 16 ������ֵ У��ͣ� $��*֮�䣨������$��*�� �����ַ��������
11 <CR><LF> �ַ� �س��뻻�з�
*/
const u8 ZKW_SET_GNSS[] = {"$PCAS03,1,0,1,0,1,0,0,1*02\r\n"};

const u8 location_mode[3][12] =
    {
        {0xF1, 0xD9, 0x06, 0x0C, 0x04, 0x00, 0x01, 0x02, 0x00, 0x00, 0x19, 0xA6},
        {0xF1, 0xD9, 0x06, 0x0C, 0x04, 0x00, 0x04, 0xC0, 0x00, 0x00, 0xDA, 0xEC}, //������
        {0xF1, 0xD9, 0x06, 0x0C, 0x04, 0x00, 0x05, 0xC2, 0x00, 0x04, 0xE1, 0xFA},
};

/****************************************************************************
* ����:    config_gps_port ()
* ���ܣ�GPS�˿�����
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void config_gps_port(void) {
    GPS_PWR_OUTPUT();
    pwr_gps_lsm_lcd_inOut5v0(1, "config_gps_port");
}

/****************************************************************************
* ����:    init_gps_info ()
* ���ܣ���ָ����SECTOR��Ѱ����Ч��ָ��
1.��flash������ֻ��10��Σ���Ҫѭ���洢
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void init_gps_info(void) {
    s16 low;
    s16 mid;
    s16 high;

    GPS_BASE_INFOMATION g1;
    u16 cnt;

    gps_struct_init();
    _memset((u8 *)&gps_base, 0x00, sizeof(gps_base));
    _memset((u8 *)&gps_ext, 0x00, sizeof(gps_ext));

    g_u8GPRMCFlag = 0;
    g_u8GPGGAFlag = 0;

    sys_time.year = 21;
    sys_time.month = 11;
    sys_time.date = 23;
    sys_time.hour = 12;
    sys_time.min = 0;
    sys_time.sec = 0;

    //stm32f1_rtc_get( &sys_time);

    cnt = 0;
    low = 0;
    mid = 0;
    high = MAX_GPS_SIZE;

    while (low <= high) {
    search_start:
        if (++cnt > 10) {
            return;
        }
        mid = (low + high) / 2;
        if (mid > MAX_GPS_SIZE) {
            mid = 0;
            low = 0;
            high = MAX_GPS_SIZE / 2;
        }
        spi_flash_read((u8 *)&g1, _gps_addr(mid), sizeof(g1));
        if (g1.flag == 0x55aa) //��ǰ��Ч����
        {
            //���У��
            save_gps_pos = mid;

            _memcpy((u8 *)&gps_base, (u8 *)&g1, sizeof(g1));
            _memcpy((u8 *)&sys_time, (u8 *)&g1.ftime, sizeof(sys_time));
            gps_base.fixed = false;
            gps_base.speed = 0;
            gps_base.heading = 0;
            gps_base.heigh = 0;
            gps_base.fix_num = 0;
            gps_base.pnumber = 0;
            gps_base.up_jifs = 0;
            return;
        } else if (g1.flag == 0xffff) //δʹ������
        {
            high = mid - 1;
        } else //��ʹ�ù�������
        {
            low = mid + 1;
        }
        goto search_start;
    }
}

/****************************************************************************
* ����:    save_gps_parameterer ()
* ���ܣ���������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void save_gps_basemation(GPS_BASE_INFOMATION *p, time_t *t) {
    GPS_BASE_INFOMATION gps_base_s;
    u8 buf[64];

    if ((gps_base.locate.lati == 0) || (gps_base.locate.lngi == 0))
        return;

    _memcpy((u8 *)&gps_base_s, (u8 *)p, sizeof(gps_base_s));
    _memcpy((u8 *)&gps_base_s.ftime, (u8 *)t, sizeof(time_t));
#if 0
	if( gps_base_s.fixed )
	{
		if( (gps_base_s.ftime.year != sys_time.year )||(gps_base_s.ftime.month != sys_time.month )||\
			(gps_base_s.ftime.date != sys_time.date )||(gps_base_s.ftime.hour != sys_time.hour )||\
			(gps_base_s.ftime.min != sys_time.min )||(gps_base_s.ftime.sec != sys_time.sec ))
		{
			stm32f1_rtc_set(&gps_base_s.ftime);
		}
	}
	//_memcpy((u8 *)&gps_base_s.ftime, (u8 *)t, sizeof(time_t));
#endif

    if (save_gps_pos >= MAX_GPS_SIZE) {
        spi_flash_erase(GPS_ADDR);
        save_gps_pos = 0;
    } else {
        _memset(buf, 0x00, 64);
        //spi_flash_write(buf, _gps_addr(save_gps_pos), sizeof(GPS_BASE_INFOMATION));
        spi_flash_write(buf, _gps_addr(save_gps_pos), sizeof(GPS_BASE_INFOMATION));
        save_gps_pos++;
    }

    add_struct_crc((u8 *)&gps_base_s, sizeof(gps_base_s));
    spi_flash_write((u8 *)&gps_base_s, _gps_addr(save_gps_pos), sizeof(GPS_BASE_INFOMATION));
}

/****************************************************************************
* ����:    get_gps_field ()
* ���ܣ���������֡�б�־λ(,)�ĸ���
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u8 *get_gps_field(u8 *pf, u8 n) {
    u8 *pt = pf;
    u8 len = 99;
    if (n == 0)
        return pf;

    while ((len != 0) && (*pt != 0)) {
        if (*pt == ',')
            n--;
        pt++;
        if (n == 0)
            break;
        if (len > 0)
            len--;
    }
    return pt;
}

/****************************************************************************
* ����:    get_standard_sec_num ()
* ���ܣ�ʱ��ת����ʱ��ת�����룬13/01/01 00:00:00 Ϊ��׼ʱ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
s32 get_standard_sec_num(time_t t1) {
    s32 day = 0;
    s32 minute = 0;
    s32 sec = 0;

    time_t t2;
    u8 i = 0;

    if ((t1.year < 13) || (t1.year > 60) || (t1.month == 0) || (t1.month > 12))
        return -1;

    if (t1.date == 0 || t1.date > 31 || t1.hour > 23 || t1.min > 59)
        return -1;

    t2.year = t1.year - 13;
    t2.month = t1.month;
    t2.date = t1.date;
    t2.hour = t1.hour;
    t2.min = t1.min;
    t2.sec = t1.sec;

    day = (s32)(t2.year * 365);
    day += (t2.year / 4); //��Ӵ�13�꿪ʼ���ۼ���������

    for (i = 1; i < t2.month; i++) {
        day += get_monsize(t1.year, i);
    }

    day += t2.date;

    minute = (s32)(day * 1440);
    minute += (s32)(t2.hour * 60);
    minute += t2.min;

    sec = (minute * 60) + t2.sec;
    return (sec);
}

/****************************************************************************
* ����:    get_gps_field ()
* ���ܣ��ҵ���Ӧ��־λ��������֮�����ĳ���
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u8 get_gps_flen(u8 *pf) {
    u8 len = 0;
    u8 *pt = pf;

    while ((*pt != ',') && (*pt != 0)) {
        len++;
        pt++;
    }
    return len;
}

/****************************************************************************
* ����:    gps_realtime_write ()
* ���ܣ���ʱ�ж�GPS��Ϣ�Ƿ���Ҫ����
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void gps_realtime_write(void) {
    static u32 twrite = 0;
    static bool acc_off_save = true;

    if (!gps_active())
        return;

    if (!car_state.bit.acc) {
        if (acc_off_save) {
            acc_off_save = false;
            twrite = tick;
            save_gps_basemation(&gps_base, &sys_time);

            //�ж�RTCʱ�Ӹ�GPSʱ����죻���궨
        }
        return;
    }

    acc_off_save = true;
    if (_covern(twrite) >= (2 * 60) || twrite == 0) //ÿ15���ӱ���һ��(ACC OFF ���������棬����12h)��flash����6��
    {
        twrite = tick;
        save_gps_basemation(&gps_base, &sys_time);
    }
}

/****************************************************************************
* ����:    monitor_1_gps ()
* ���ܣ�ʵʱ���GPS ģ��״̬
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void monitor_1_gps(void) {
    static u32 update_time = 0;
    static u32 update_cnt = 10;
    static u32 tactive = 0;
    static u32 bak_pnum = 0;

    // ��λ״̬��GPS ���ݰ��ۼӸ����и���ʱ����Ϊ��λ����
    if ((!car_state.bit.acc) || (gps_active() && bak_pnum != gps_base.pnumber)) {
        bak_pnum = gps_base.pnumber;
        // BUG��
        update_time = jiffies;
        update_cnt = 0;
    }

// GPS δ��λ10 ���ˢ����Ӧ����������5��
#if 1
    if (_pastn(update_time) >= 1000) {
        update_time = jiffies;
        if (++update_cnt > 5) //5
        {
            update_cnt = 0;
            if (gps_base.fixed) {
                gps_base.fixed = false;
                gps_ext.lost_fix = true;
                save_gps_basemation(&gps_base, &sys_time);
            }
        }
    }
#else
    //������Ҫ��ʱ�ж�
    if (_pastn(update_time) >= 700) {
        update_time = jiffies;
        //if (++update_cnt > 1)		//5
        // {
        update_cnt = 0;
        if (gps_base.fixed) {
            gps_base.fixed = false;
            gps_ext.lost_fix = true;
            save_gps_basemation(&gps_base, &sys_time);
        }
        //}
    }
#endif

    //GPS ��������ֹ��̬Ư��       xinhao888
    if (!gps_active() || !car_state.bit.acc || gps_base.speed < 5) {
        gps_base.speed = 0;
        display_speed = 0;
    }

    //�쳣�������λ
    if (gps_base.fixed || gps_ext.sleep || gps_ext.pwr_down) {
        tactive = tick;
    } else if (_covern(tactive) >= (5 * 60)) {
        //5���Ӻ�λ
        //��������ʱ��ÿ5���ӣ�����һ��GPSģ��
        gps_reset = true;
        tactive = tick;
    }
}

/****************************************************************************
* ����:    monitor_gps ()
* ���ܣ�ÿ����GPS ģ��״̬
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void monitor_2_gps(void) {
    static u8 cnt = 0;

    //ִ�����߻�ϵ�
    if (gps_ext.sleep || gps_ext.pwr_down) {
        if (!car_state.bit.acc) {
            gps_reset = false;
            pwr_gps_lsm_lcd_inOut5v0(0, "gps s1");
        }
    } else {
        if (!gps_reset) {
            pwr_gps_lsm_lcd_inOut5v0(1, "gps s2");
        }
    }

    //ִ�и�λ����
    if (gps_reset) {
        if (++cnt <= 10) {
            pwr_gps_lsm_lcd_inOut5v0(0, "gps s3");
        } else {
            pwr_gps_lsm_lcd_inOut5v0(1, "gps s4");
            gps_reset = false;
        }
        gps_base.fixed = false;
    } else {
        cnt = 0;
    }
}

/****************************************************************************
* ����:    lock_gps_posiont ()
* ���ܣ�GPS��������ֹ��̬Ư��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static bool lock_gps_posiont(u8 *p) {
    static point_t lock_locate = {0, 0};
    point_t cur;

    _memcpy((u8 *)&cur, p, sizeof(point_t));

    if (!mix.moving) {
        if (!lock_locate.lati || !lock_locate.lngi) //!gps_base.fixed ||
        {
            if ((gps_base.fix_num >= 5)) {
                _memcpy((u8 *)&lock_locate, (u8 *)&cur, sizeof(point_t));
                logd("gps locate: 3d");
                return true;
            }
        } else {
            if (!lock_locate.lati || !lock_locate.lngi)
                return false;
            else
                _memcpy(p, (u8 *)&lock_locate, sizeof(point_t));
            return true;
        }
    } else {
        if (gps_base.speed < 5) {
            if (!lock_locate.lati || !lock_locate.lngi) {
                _memcpy((u8 *)&lock_locate, (u8 *)&cur, sizeof(point_t));
            }
            _memcpy(p, (u8 *)&lock_locate, sizeof(point_t));
        } else
            _memcpy((u8 *)&lock_locate, (u8 *)&cur, sizeof(point_t));

        return true;
    }

    if (!cur.lati || !cur.lngi)
        return false;

    return true;
}

void gps_buf_copy(u8 *dst, u16 src, u16 n) {
    while (n--) {
        *dst++ = s_GpsCom.buf[src];
        if (++src >= G_RECE_SIZE)
            src = 0;
    }
}

/*********************************************************
��    �ƣ�Uart3_Gps_Proc
��    �ܣ�����������
��    ����
��    ������
ע�����
*********************************************************/
void gps_idle(void) {
    u8 cData;
    static u16 pf = 0;
    u8 buf[10] = {0};
    static u8 len = 0;
#if (P_GNSS == GNSS_BASE)
    bool ret;
#endif
    while (1) {
        if (s_GpsCom.uiRxdIn == s_GpsCom.uiRxdOut) {
            break;
        }

        cData = s_GpsCom.buf[s_GpsCom.uiRxdOut]; //copy to sCmaControl buffer

        switch (s_GpsCom.step) {
        case 0: {
            if (cData == '$') {
                s_GpsCom.step++;
                pf = s_GpsCom.uiRxdOut;
                len = 1;
            }
            break;
        }
        case 1: {
            if (cData == '\r') { //0x0a
                s_GpsCom.step = 0;
                gps_buf_copy(buf, pf, 6);
                if (len > G_PROC_SIZE)
                    len = G_PROC_SIZE;
                if ((_strncmp(buf + 3, "RMC", 3) == 0)) {
                    if (g_u8GPRMCFlag != 0x55) {
                        _memset(g_u8GPRMCBuff, 0, G_PROC_SIZE);
                        gps_buf_copy(g_u8GPRMCBuff, pf, len);
                        g_u8GPRMCFlag = 0x55;

                        self_test.uGpsU = '2';
                    }
                } else if ((_strncmp(buf + 3, "GGA", 3) == 0)) {
                    if (g_u8GPGGAFlag != 0x55) {
                        _memset(g_u8GPGSVBuff, 0, G_PROC_SIZE);
                        gps_buf_copy(g_u8GPGSVBuff, pf, len);
                        g_u8GPGGAFlag = 0x55;
                    }
                } else if ((_strncmp(buf + 3, "TXT", 3) == 0)) {
                    if (g_u8GPGSAFlag != 0x55) {
                        _memset(g_u8GPGSABuff, 0, 50);
                        gps_buf_copy(g_u8GPGSABuff, pf, len);
                        g_u8GPGSAFlag = 0x00;
                        //if ((_strncmp(g_u8GPGSABuff + 24, "OPEN", 4) == 0))
                        if ((_strncmp(g_u8GPGSABuff + 20, "OPEN", 4) == 0)) { //��·
                            pub_io.b.antn_open = true;

                            car_alarm.bit.ant_open = true;
                            car_alarm.bit.ant_short = false;

                            self_test.uGpsU = '1';
                        }
                        //else if ((_strncmp(g_u8GPGSABuff + 24, "SHORT", 5) == 0))
                        else if ((_strncmp(g_u8GPGSABuff + 20, "SHORT", 5) == 0)) { //��·
                            // pub_io.b.antn_open = false;

                            car_alarm.bit.ant_open = false;
                            car_alarm.bit.ant_short = true;
                        }
                        //else if ((_strncmp(g_u8GPGSABuff + 24, "OK", 2) == 0))
                        else if ((_strncmp(g_u8GPGSABuff + 20, "OK", 2) == 0)) { //����
                            pub_io.b.antn_open = false;

                            car_alarm.bit.ant_open = false;
                            car_alarm.bit.ant_short = false;
                        }
                    }
                } else if ((_strncmp(buf + 3, "GSA", 3) == 0)) //�ڽ��˷�Ư���ж�
                {
                    if (g_u8GPGSAFlag != 0x55) {
                        gps_buf_copy(g_u8GPGSABuff, pf, 28);
                        g_u8GPGSAFlag = 0x55;
                    }
                }
            } else {
                len++;
            }
            break;
        }
        default: {
            s_GpsCom.step = 0;
            break;
        }
        }

        if (++s_GpsCom.uiRxdOut >= G_RECE_SIZE) {
            s_GpsCom.uiRxdOut = 0;
        }
    }
}

/****************************************************************************
* ����:    gps_verify_gga ()
* ���ܣ������߶���Ϣ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void gps_verify_gga(void) {
    u8 *pt;
    u8 len;
    u8 buf[32] = {0};
    float heigh_t = 0;

    if (g_u8GPGGAFlag == 0x55) {
        g_u8GPGGAFlag = 0x00;

        pt = get_gps_field(g_u8GPGSVBuff, 7);
        read_gps(buf, pt, 2);
        gps_base.fix_num = atoi((const char *)&buf);

        pt = get_gps_field(g_u8GPGSVBuff, 9);
        //len = get_gps_flen(pt);
        //read_gps(buf, pt, len);
        sscanf((const char *)pt, "%[^,]", buf);
        buf[len] = '\0';
        heigh_t = (atof((const char *)&buf));
        //if (heigh_t >= 0.0 && heigh_t < 10000.0)
        if (heigh_t >= 0.0 && heigh_t < 10000.0) //2022/04/08 �������ͱȽ�
        {
            gps_base.heigh = floor(heigh_t);
        }
    }
}

static void gps_verify_gSA(void) {
    u8 *pt;
    u8 buf[10] = {0};

    if (g_u8GPGSAFlag == 0x55) {
        g_u8GPGSAFlag = 0x00;

        pt = get_gps_field(g_u8GPGSABuff, 2);
        read_gps(buf, pt, 2);
        gps_base.if3d = atoi((const char *)&buf); //ֻ��3d ����3d  ����
    }
}

/****************************************************************************
* ����:    gps_debonce ()
* ���ܣ�����λ����Ϣ֡
* ��ڲ�������
* ���ڲ�������
$GPRMC,095543.000,A,2234.3793,N,11357.6129,E,0.04,225.05,190314,,,D*69
****************************************************************************/
static void gps_debonce(void) {
    // u8 i = 0;
    u8 *pt;
    static u8 sec = 0;
    u8 str[10];
    u8 buf[32];
    //    u16 tnew;
    int vnew = 0;
    //    u16 dt;
    point_t pnew;
    //    float dist;
    float v_t;
    u16 len;
    u32 dgr;
    u32 min;
    //    static u16 told = 0;
    //    static int vold = 0;
    //    static point_t pold = {0, 0};
    time_t gps_time;
    float d; //����

    /*
    if (g_u8GPRMCFlag == 0x55)
    {
        g_u8GPRMCFlag = 0x00;
		
        pt = get_gps_field(g_u8GPRMCBuff, 2);

        if (*pt != 'A')
        {
            return;
        }
    }
    else
        return;
*/
    if (g_u8GPRMCFlag == 0x55) {
        g_u8GPRMCFlag = 0x00;

        //pt = get_gps_field(g_u8GPRMCBuff, 2);
    } else
        return;

    //UTCʱ��
    pt = get_gps_field(g_u8GPRMCBuff, 9);
    read_gps(buf, pt, 6);
    gps_time.date = atoi((const char *)_strncpy(str, buf, 2));
    gps_time.month = atoi((const char *)_strncpy(str, buf + 2, 2));
    gps_time.year = atoi((const char *)_strncpy(str, buf + 4, 2));

    pt = get_gps_field(g_u8GPRMCBuff, 1);
    read_gps(buf, pt, 6);
    gps_time.hour = atoi((const char *)_strncpy(str, buf, 2));
    gps_time.min = atoi((const char *)_strncpy(str, buf + 2, 2));
    gps_time.sec = atoi((const char *)_strncpy(str, buf + 4, 2));

    pt = get_gps_field(g_u8GPRMCBuff, 2);

    if (*pt != 'A') {
        if (u1_m.cur.b.debug) {
#if DEBUG_EN_GPS_UN_dingWei_printf
            logw("����-%02d:%02d:%02d ��Ч��λ����<%s>", gps_time.hour, gps_time.min, gps_time.sec, g_u8GPRMCBuff);
            logb_empty(log_level_warn, "%c", '\0');
#endif
            // logw("����RMC����:");
            // for (i = 0; i < _strlen(g_u8GPRMCBuff); i++){
            //     _printf2("%c", g_u8GPRMCBuff[i]);
            // }
        }

        return;
    }
    /*
	tnew = (gps_time.min*60)+gps_time.sec;

    if (tnew > told)
        dt = tnew - told;
    else
        dt = tnew + 3600 - told;

    if (dt == 0)    {
    	loge("ʱ������ظ�");
  	goto  abort;
	
	}
      
    told = tnew;
*/

    pt = get_gps_field(g_u8GPRMCBuff, 7);
    len = get_gps_flen(pt);
    //    if (len > 7)
    //    {
    //   	   loge("����GPS��ȡ�ٶȳ��Ȳ���");
    //	  goto abort;
    //	}

    read_gps(buf, pt, len);
    buf[len] = '\0';
    v_t = (atof((const char *)&buf)) * 1.85;
    //    if (v_t > 160) //����120��� 65 * 1.85 = 120km/h
    //    {
    //    		loge("����GPS�ٶȹ���");
    //			goto abort;
    //   	}

    if (v_t > 3)
        display_speed = v_t * 10;
    else
        display_speed = 0;
    vnew = floor(v_t); //floor���ز�����v_t���������

    //   if (vnew > vold)
    //    {
    //        if(vnew - vold >= (9 * dt)) //���ٶ� 26 * 1.85 = 30km/h
    //        {
    //            if(gps_base.fixed)
    //		{
    //			loge("GPS�ٶ�����");
    //			  goto abort;
    //		}
    //        }
    //    }

    gps_base.speed = vnew;

    /*2022-03-25 ������, ���ó���Ϊ50km/h*/
    //gps_base.speed = 125;

    if (gps_base.speed >= 150) {
        gps_base.speed = 150;
        display_speed = 1500;
    }
    //if( run.lock_gps == 0x55 )
    //	gps_base.speed = 0;
    pt = get_gps_field(g_u8GPRMCBuff, 8);
    len = get_gps_flen(pt);
    read_gps(buf, pt, len);
    buf[len] = '\0';
    if (len > 0) {
        d = (atof((const char *)&buf));
        if (d >= 0 && d < 360) {
            gps_base.heading = floor(d);
        }
    }

    pt = get_gps_field(g_u8GPRMCBuff, 3);
    read_gps(buf, pt, 10);
    dgr = atoi((const char *)_strncpy(str, buf, 2)) * 1000000uL;
    //min  = atoi((const char *)_strncpy(str, buf + 2, 2)) * 10000uL;
    //min += atoi((const char *)_strncpy(str, buf + 5, 4)) * 1uL;
    min = atoi((const char *)_strncpy(str, buf + 2, 2)) * 100000uL;
    min += atoi((const char *)_strncpy(str, buf + 5, 5)) * 1uL;
    dgr += (min / 6);
    pt = get_gps_field(g_u8GPRMCBuff, 4);
    pnew.lati = (*pt == 'N') ? dgr : -dgr;

    pt = get_gps_field(g_u8GPRMCBuff, 5);
    read_gps(buf, pt, 11);
    dgr = atoi((const char *)_strncpy(str, buf, 3)) * 1000000uL;
    // min  = atoi((const char *)_strncpy(str, buf + 3, 2)) * 10000uL;
    // min += atoi((const char *)_strncpy(str, buf + 6, 4)) * 1uL;
    min = atoi((const char *)_strncpy(str, buf + 3, 2)) * 100000uL;
    min += atoi((const char *)_strncpy(str, buf + 6, 5)) * 1uL;
    dgr += (min / 6);
    pt = get_gps_field(g_u8GPRMCBuff, 6);
    pnew.lngi = (*pt == 'E') ? dgr : -dgr;

    //120ǧ��/ʱ(km/h)=33��/��(m/s)

    //  dist = _cal_distance(pnew.lati, pold.lati,pnew.lngi, pold.lngi);
    // if (dist >= (float)30 * 30 * dt * dt  && pold.lati)
    //  {
    //abort:
    //     vold = vnew;
    //      pold = pnew;
    //      gps_base.pnumber++;
    //loge("��λ������������");
    //     return;
    // }

    //    vold = vnew;
    //    pold = pnew;

    if (car_state.bit.acc) {
        if (lock_gps_posiont((u8 *)&pnew))
            gps_base.locate = pnew;
    }

    gps_base.pnumber++;

    gps_base.fixed = true;

    modify_beijing_time_area(&gps_time);

    gps_base.ftime = gps_time;
    if (refreshTime) {
        gps_base.up_jifs = jiffies;
        sys_time = gps_time;
        //        i = 0;
        if (u1_m.cur.b.debug) {
            if (sec == sys_time.sec) {
                loge("����λ�ظ�ʱ��-%02d:%02d:", sys_time.min, sys_time.sec);
                // i = 10;
            } else if ((sys_time.sec - sec) > 1) {
                // i = 10;
                loge("����λ����ʱ��-%02d:%02d:", sys_time.min, sys_time.sec);
            }
        }
    }

    sec = sys_time.sec;
    positioned_update = true;
}

/****************************************************************************
* ����:    set_gnss_mode ()
* ���ܣ����ö�λģ��ģʽ��BD,GPS,BD+GPS
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void set_gnss_mode(void) {
    //    static u8 cun = 0;
    static u8 module_type = 0;

    static u32 time_out = 0;
    if (_pastn(time_out) >= 1000) {
        time_out = jiffies;
        // write_gps((u8*)&ZKW_SET_GNSS, sizeof(ZKW_SET_GNSS));

        if (module_type != host_no_save_para.set_gps_module_type) {
            module_type = host_no_save_para.set_gps_module_type;
            if (module_type < 4) {
                write_gps((u8 *)location_mode[(module_type - 1)], 12);
                if (0 == u1_m.cur.b.tt)
                    logd("���ö�λģʽ%d", module_type);
            }

            gnss_cfg.select.reg = 0;

            if (host_no_save_para.set_gps_module_type == 1)
                gnss_cfg.select.b.GPS_EN = 1;
            else if (host_no_save_para.set_gps_module_type == 2)
                gnss_cfg.select.b.BD_EN = 1;
            else
                gnss_cfg.select.b.GLNS_EN = 1;
        }
        //else if(++cun ==32 )
        //{
        //	cun = 0 ;
        //	logd("��λģʽ%d",module_type);
        //}
    }
}

/****************************************************************************
* ����:    gps_parse_task ()
* ���ܣ�GPS ���ݴ�������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void gps_parse_task(void) {
    static u32 jtask = 0;

    gps_idle(); //ģ�����ݽ���
    gps_verify_gga();
    gps_verify_gSA();

    gps_debonce(); //��γ�Ƚ���

    monitor_1_gps();

    if (_pastn(jtask) >= 1000) {
        monitor_2_gps();

        gps_realtime_write();

        jtask = jiffies;

        set_gnss_mode();
    }
}
