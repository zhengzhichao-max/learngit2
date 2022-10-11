/**
  ******************************************************************************
  * @file    pub.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   ���ù��ܻ�������
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

u32 tick = 0;    //ϵͳʱ�䣬��
u32 jiffies = 0; //ϵͳʱ�䣬����
//unsigned long jiffies = 0 ;
bool rouse_state; //����״̬

#if (P_RCD == RCD_BASE)
u32 speed_plus = 0;   //��ȡ�����ٶ�
u32 plus_speed = 0;   //��¼�����ٶ�
u16 wheel_speed = 0;  //CAN ��ȡ�ٶ�
u16 engine_speed = 0; //CAN ��ȡ�ٶ�
u16 average_1km = 0;

u16 speed_jiffes = 0;
//u32 speed_jiffes_s = 0;
//u32 speed_jiffes_ms = 0;

#endif

u16 start_tr9_tmr = 300;
u16 car_weight;

time_t sys_time; //system time ϵͳʱ�ӣ�����ʱ��
uc8 MonthSizeTab[] =
    {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31};

MIX_GPS_DATA mix;                 //������ݣ����ڸ�ģ��֮����Ϣ����
TERMINAL_RESET_STRUCT t_rst;      //ִ���ն˸�λ
VEHICLE_ALARM_UNION bypass_alarm; //����ȡ��
run_parameter run;                //���еĲ���
VEHICLE_ALARM_UNION car_alarm;    //��������
UNION_VEHICLE_STATE car_state;    //����״̬

sCommunicationSet phone_cfg; //�绰��������
gprs_link_manange lm;        //GPRS ��·����
REMOTE_CAR rcmd;             //����Զ�̹�������
out_ctrl_block car;          //���Ͷϵ����
u32 heart_beat_tick = 0;
DVR_POWER_CTRL dvr_power;
can_speed_cali can_s_cali;

HI3520_MONITTOR s_Hi3520_Monitor;
static u8 hi3520_rcnt = 0;

//*****************************************************************************
//*****************************************************************************
//-----------		            ��������   	                  -----------------
//-----------------------------------------------------------------------------

//_extern_    gbk_unicode_buff_struct  gbk_unicode_buff;  //��������UNICODE��ת��

user_time_struct user_time; //�û�ʱ�����

mult_usart_set_struct mult_usart_set; //����ѡ���Ķ�·����

query_data_union query;                     //��ѯ��ṹ
gps_data_struct gps_data;                   //GPS�������
state_data_struct state_data;               //״̬����
set_para_struct set_para;                   //������������
recorder_para_struct recorder_para;         //��ʻ��¼�ǲ���
host_no_save_para_struct host_no_save_para; //����Ҫ��������ò���
key_data_struct key_data;                   //��������
menu_cnt_struct menu_cnt;                   //�˵��ṹ��
message_window_dis_struct mw_dis;           //��Ϣ��ʾ�ṹ��
menu_auto_exit_struct menu_auto_exit;       //�˵��Զ��˳��ṹ��
menu_report_struct menu_report;             //�������
u16 dis_multi_page_cnt;                     //��ҳ��ʾ�������Ѿ���ʾ������

input_method_struct input_method;       //���뷨
password_manage_struct password_manage; //�������

ic_type_enum ic_type;        //IC������
ic_card_date_struct ic_card; //IC������

buzzer_voice_struct buz_voice;       //�������������
voice_source_struct buz_cus[2];      //�Զ����������������
buzzer_cus_time_struct buz_cus_time; //�Զ������������ʱ��

call_record_struct call_record; //ͨ����¼�������
phone_data_struct phone_data;   //�绰�������

sms_up_center_struct sms_up_center;               //�ϴ������ĵ���Ϣ��������
sms_union_data_union sms_union_data;              //��Ϣ������
u8 sound_play_buff[sound_play_buff_max_lgth + 2]; //�����ʶ�������  ǰ�����ֽڱ�ʾ���ȣ���������������

//��չ��·����usart0:���ܱ�������
u8 uart_set_ok_flag[3];                 //��·������չ����Ĵ������óɹ���־λ��=1��ʾ���óɹ���=0��ʾû�����û�������ʧ��,�����������ú�ġ�*����ʾ
u8 extend_uart_cnt;                     //��·������չ����Ŀ���չ���ڵĸ���
u8 ic_driver_ifo_num;                   //��U�̶�ȡ���ļ�ʻԱ��Ϣ��Ŀ 2018-1-6
u32 extend_uart_buff[9];                //��·������չ���ջ���
u8 tmp_cnt;                             //��·������չ�����uartx��ѡ���ܵĸ���
u8 extend_uart_send_bit[7];             //���ڼ�¼��·������չ�����uartx�еĸ���ѡ���λ��־������extend_uart_send_bit[0]��¼���ǵ�1����ʾ���ǵڼ�λ��1����Ԫ��������
u8 current_uart_flag[3];                //��·������չ����ĵ�ǰѡ�д��ڵġ�*��ѡ���־λ,�������ڲ�ѯʱ�ġ�*����ʾ
u8 tmp_bit_cnt;                         //���ڼ�¼��·������չ�����uartx�еĸ���ѡ���λ��־��ָ��extend_uart_send_bit
unsigned char *tmp_uart_dis[3];         //��ʱ���ڵ���ʾ����Ҫ���������ʾ��·������չ�����USART1-3����ʾ
unsigned char *tmp_icDriverName_dis[5]; //������¼5��˾������ 2018-1-6

// TF���洢ͼƬʱ����ı�����Ϣ
//_extern_  u8 time[50];                  //��Ŵ洢ͼƬʱ���GPSʱ��
//_extern_ u8 full_filename[50];          //�洢ͼƬʱ���ļ���

//ͨ����¼�����������
phbk_data_struct phbk_call_in_data; //�洢��ǰ������绰���ԱȺ󣬴ӵ绰��������ȡ��������Ӧ�ĵ绰���ͣ�������������Ϣ

print_auto_data_struct print_auto_data; //�Զ���ӡ����

//������ѯ�����������
u8 realm_name_flag; //������־λ��������:  0x01   ;��������:0x02
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//spi_data_struct      spi1_data;      //SPI�ӿ����ݽ���
spi_ack_struct slv_send_flag;

SLAVE_RECIVE_DATA_STRUCT slv_rev_data;
SLAVE_SEND_DATA_STRUCT slv_send_data;

//���ڣ�ʱ����غ���
/****************************************************************************
* ����:    get_monsize ()
* ���ܣ��������¶������һ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u8 get_monsize(u8 year, u8 month) {
    u8 n;

    n = MonthSizeTab[month - 1];
    if (month == 2 && year % 4 == 0)
        n += 1;
    return n;
}

/****************************************************************************
* ����:    get_jiffies ()
* ���ܣ��õ�ϵͳ���뼶ʱ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u32 get_jiffies(void) {
    u32 n;

    cli();
    n = jiffies;
    sei();
    return n;
}

/****************************************************************************
* ����:    rtime_cmp ()
* ���ܣ��Ƚ�ʱ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
s16 rtime_cmp(void *time1, void *time2) {
    u8 i;
    u8 *timep1 = (u8 *)time1;
    u8 *timep2 = (u8 *)time2;

    for (i = 0; i < 6; i++) {
        if (*timep1 == *timep2) {
            timep1++;
            timep2++;
        } else
            return *timep1 - *timep2;
    }
    return 0;
}

/****************************************************************************
* ����:    delay10us ()
* ���ܣ���ʱ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void delay10us(void) {
    u8 i = 2;
    while (i--) {
        delay1us();
    }
}

/****************************************************************************
* ����:    delay50us ()
* ���ܣ���ʱ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void delay50us(void) {
    u8 i = 10;
    while (i--) {
        delay1us();
    }
}

void delay_us(u8 lgth) {
    u8 i;

    while (lgth) {
        for (i = 0x00; i < 9; i++)
            ;
        lgth--;
    }
}

//**************************************************************************//
//�ú����Ĺ������ṩ΢�뼶����ʱ������������ʱʱ������10΢��(@72MHz)
//
//**************************************************************************//
void delay_10us(u16 lgth) {
    u16 i;

    while (lgth) {
        for (i = 0x00; i < 120; i++)
            ;
        lgth--;
    }
}

/****************************************************************************
* ����:    delay1ms ()
* ���ܣ���ʱ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void delay1ms(void) {
    u8 i = 20;
    while (i--) {
        delay50us();
    }
}

/****************************************************************************
* ����:    delay20ms ()
* ���ܣ���ʱ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void delay20ms(void) {
    u8 i = 20;
    while (i--) {
        delay1ms();
    }
}

/****************************************************************************
* ����:    delay_ms ()
* ���ܣ���ʱ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void delay_ms(u8 i) {
    while (i--) {
        delay1ms();
    }
}

void delayms(u32 d_time) {
    u32 i, j;

    for (i = 0; i < d_time; i++)
        for (j = 0; j < 1600; j++)
            ;
}

/****************************************************************************
* ����:    delay50ms ()
* ���ܣ���ʱ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void delay50ms(void) {
    int i = 1000;
    while (i--) {
        delay50us();
    }
}

/****************************************************************************
* ����:    modify_beijing_time_area ()
* ���ܣ���������ʱ��ת��Ϊ����ʱ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void modify_beijing_time_area(time_t *t) {
    if (t->hour >= 16) {
        t->hour -= 16;
        if (++t->date > get_monsize(t->year, t->month)) {
            t->date = 1;
            if (++t->month > 12) {
                t->month = 1;
                if (++t->year > 99)
                    t->year = 0;
            }
        }
    } else
        t->hour += 8;
}

/****************************************************************************
* ����:    update_timer_task ()
* ���ܣ�����ʱ�䣬�Զ�ˢ��ϵͳʱ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void update_timer_task(void) {
    static u32 chase = 0;
    static u8 sec_bak = 0;
    //    static u8 Time = 0;
    /*
	  if(gps_base.ftime.sec != Time)
	  {//����ȡGPSʱ����Ϊ��ʼ���ʱ
	  	Time = gps_base.ftime.sec;
	  	//Gps_time_sec++;
	  	
	  }	
	  */
    if (s_Hi3520_Monitor.start) {
        if ((positioned_update && sec_bak != sys_time.sec) || (sec_bak != sys_time.sec)) {
            sec_bak = sys_time.sec;
            positioned_update = false;
            send_base_msg_to_tr9();
        }
    }

    if (xpastn(chase) < 1000) {
        return;
    }

    tick++;
    chase += 1000;

#if (0)
    logd("ʵʱʱ��--%02d:%02d:%02d", sys_time.hour, sys_time.min, sys_time.sec);
#else
    if (tick % 20 == 0) {
        promt_time(&sys_time, 1, log_level_info, __func__, __LINE__, "sys.tm", NULL);
        promt_time(&mix.time, 3, log_level_info, NULL, __LINE__, "mix.tm", NULL);
    }
#endif

    if (_pastn(gps_base.up_jifs) <= 1500) {
        return;
    }

    if (++sys_time.sec >= 60) {
        sys_time.sec = 0;
        if (++sys_time.min >= 60) {
            sys_time.min = 0;
            if (++sys_time.hour >= 24) {
                sys_time.hour = 0;
                if (++sys_time.date > get_monsize(sys_time.year, sys_time.month)) {
                    sys_time.date = 1;
                    if (++sys_time.month > 12) {
                        sys_time.month = 1;
                        if (++sys_time.year > 99)
                            sys_time.year = 0;
                    }
                }
            }
        }
    }
}

/****************************************************************************
* ����:    update_mix_infomation ()
* ���ܣ�ˢ�²���
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void update_mix_infomation(void) {
    VEHICLE_ALARM_UNION bypass_tmp;
    //    u32 speed = 0;

    //u16 speed_temp = 0;
    //���±���״̬
    if (bypass_alarm.reg != 0 || alarm_cfg.bypass_sw.reg != 0) {
        //��λ�룬����Ӧ״̬��Ϊ��, �����Ӧ��·����λ
        bypass_alarm.reg &= car_alarm.reg;
        //���JT808����ƽ̨������
        //ȡ��������·������λΪ0
        bypass_tmp.reg = (~bypass_alarm.reg);
        bypass_tmp.reg &= (~alarm_cfg.bypass_sw.reg);
        mix.alarm.reg = bypass_tmp.reg & car_alarm.reg;
    } else
        mix.alarm.reg = car_alarm.reg;

    //���³���״̬
    car_state.bit.fix = (gps_base.fixed == true) ? true : false;     //���Ƕ�λ�Ƿ�����
    car_state.bit.lat = (gps_base.locate.lati < 0) ? true : false;   //γ��:0 ��γ ; 1 ��γ
    car_state.bit.lng = (gps_base.locate.lngi < 0) ? true : false;   //����:0 ���� ; 1 ����
    car_state.bit.cut_oil = (rcmd.b.cut_oil == true) ? true : false; //
    car_state.bit.cut_pwr = (rcmd.b.cut_oil == true) ? true : false;
    //2014-07-08
    car_state.bit.fix_gps = (gnss_cfg.select.b.GPS_EN == 1) ? true : false;      //ʹ�� gps  ��λ
    car_state.bit.fix_bd = (gnss_cfg.select.b.BD_EN == 1) ? true : false;        //ʹ�� ���� ��λ
    car_state.bit.fix_glonass = (gnss_cfg.select.b.GLNS_EN == 1) ? true : false; //ȫ��λ
    car_state.bit.fix_calileo = (gnss_cfg.select.b.GLEO_EN == 1) ? true : false;

    car_state.bit.left_cover = (tr9_car_status.bit.left_cover == true) ? true : false;
    car_state.bit.right_cover = (tr9_car_status.bit.right_cover == true) ? true : false;
    car_state.bit.all_flag = true;

    car_state.bit.lock_car_sw = (run.lock_car_sw == 0x55) ? true : false;
    car_state.bit.speeding = (tr9_car_status.bit.speeding == true) ? true : false;
    car_state.bit.car_rise = false;
    car_state.bit.loading = (tr9_car_status.bit.loading == true) ? true : false;
    car_state.bit.car_cover = (tr9_car_status.bit.covering == true) ? true : false;
    car_state.bit.car_box = (pub_io.b.car_cover == true) ? true : false;

#if (P_RCD == RCD_BASE)
    bd_rcd_io.reg = 0;
    bd_rcd_io.b.far_light = (rcd_io.b.far_light == true) ? true : false;     //Զ��
    bd_rcd_io.b.brake = (rcd_io.b.brake == true) ? true : false;             //ɲ��
    bd_rcd_io.b.near_light = (rcd_io.b.near_light == true) ? true : false;   //����
    bd_rcd_io.b.right_light = (rcd_io.b.right_light == true) ? true : false; //��ת
    bd_rcd_io.b.left_light = (rcd_io.b.left_light == true) ? true : false;   //��ת
#endif
    mix.fixed = gps_base.fixed == true ? true : false; //�Ƿ�λ
    mix.car_state.reg = car_state.reg;                 //��һ״̬
    //���³��������Ϣ
    mix.csq = gs.gsm.squality; //�ź�ǿ��

    //����GPS�����Ϣ
    mix.time.year = _bintobcd(sys_time.year);
    mix.time.month = _bintobcd(sys_time.month);
    mix.time.date = _bintobcd(sys_time.date);
    mix.time.hour = _bintobcd(sys_time.hour);
    mix.time.min = _bintobcd(sys_time.min);
    mix.time.sec = _bintobcd(sys_time.sec);

    mix.locate.lati = gps_base.locate.lati;
    mix.locate.lngi = gps_base.locate.lngi;

    mix.speed = display_speed;

    mix.heigh = gps_base.heigh;
    mix.direction = gps_base.heading;
    mix.dist = run.total_dist / 100;

    mix.oil_L = 0;
    mix.fix_num = gps_base.fix_num;
#if 0
    mix.heigh = 0;
#endif
}

#if (0)
//AD ����ѹ�ɼ���ת��
/*********************************************************
��    �ƣ�Ic_Get_ExternalPowerValue
��    �ܣ���ȡ�ⲿ�����Դ��ѹֵ
�����������
��    ������ǰ�ⲿ�����Դ��ѹֵ
��д���ڣ�2011-02-14     \
910 - 62 
430 - 18
**********************************************************/
static u16 Get_External_Power_Value(void) {
    u32 u32_Power_AD = 0x00;
    u16 power_value = 0;

    u32_Power_AD = ADC_GetConversionValue(ADC1);

    u32_Power_AD = (u32_Power_AD * 330) / 0x1000; //��λ0.01V

    u32_Power_AD = (u32_Power_AD * 956) / 47;                            //Ӳ����ѹϵ��
    power_value = u32_Power_AD > 700 ? u32_Power_AD + 30 : u32_Power_AD; //���0.30V������ѹ��
    return (u16)power_value;
}
#endif

/****************************************************************************
* ����:    cal_car_power_type ()
* ���ܣ����㳵ϵ��ѹ��Χ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u8 cal_car_power_type(s32 vot) {
    if (vot < 300 || vot > 4200) {
        return PWR_UNKNOWN;
    } else if (vot >= 200 && vot < 1800) {
        return PWR_12V;
    } else if (vot >= 3000 && vot < 4200) {
        return PWR_36V;
    } else {
        return PWR_24V;
    }
}

/****************************************************************************
* ����:    cal_car_power_type ()
* ���ܣ�����������ƽ��ѹ�ٷֱ�
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u8 cal_power_per(u8 pwr_type, s32 vol) {
    u8 per;
    s32 value;

    per = 101;               // ������Ч
    if (pwr_type == PWR_12V) //8v: 33% 9v :50%  10v: 67%  10.5v:75%
    {
        value = (vol - 600) * 100 / (1200 - 600);
        per = value < 33 ? 0 : (value > 100 ? 100 : value);
    } else if (pwr_type == PWR_24V) //20v: 33% 21v :50% 22v: 67%  22.5v:75%
    {
        value = (vol - 1800) * 100 / (2400 - 1800);
        per = value < 33 ? 0 : (value > 100 ? 100 : value);
    } else if (pwr_type == PWR_36V) //32v: 33% 33v :50%  34v: 67%  34.5v:75%
    {
        value = (vol - 3000) * 100 / (3600 - 3000);
        per = value < 33 ? 0 : (value > 100 ? 100 : value);
    }

    return per;
}

bool adc_lock = false;

static u16 vbuf[VBUF_SIZE] = // Default value is 100%(12V)
    {
        800, 800, 800, 800, 800, 800, 800, 800, 800, 800};

static u16 *vhead = vbuf;
/****************************************************************************
* ����:    cal_car_power_type ()
* ���ܣ����㵱ǰ������ѹ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void adc_power_task(void) {
    //6057����ѹ��ѯ://�ն�����ѹ��ѯ��RK->MCU��
    u8 i;
    s32 value = 0;
    u16 adv = 0;
    static u8 type = PWR_12V;
    static u32 tmr = 0;

    static enum {
        BATV_SETCH,
        BATV_CONVERT
    } step = BATV_SETCH;

    switch (step) {
    case BATV_SETCH:
        if (adc_lock == false) {
            adc_lock = true;
            step = BATV_CONVERT;
        }
        break;
    case BATV_CONVERT:
        adv = Read_Power_Adc_Value(); // ʵ�ʵ�ѹ
        *vhead = adv;
        incp_vbuf(vhead);
        for (i = 0; i < VBUF_SIZE; i++)
            value += vbuf[i];
        value /= VBUF_SIZE; //ƽ����ѹ
                            /* if (tick < 5)
            {
                type = cal_car_power_type(value);
                adc_lock = false;
                step = BATV_SETCH;
                break;
            }*/
        if (_covern(tmr) > 10) {
            tmr = tick;
            type = cal_car_power_type(value);
            adc_lock = false;
            step = BATV_SETCH;
            break;
        }
        mix.pwr_type = type;
        mix.pwr_per = cal_power_per(type, value);
        mix.pwr_vol = value;
        adc_lock = false;
        step = BATV_SETCH;
        break;
    default:
        step = BATV_SETCH;
    }
}

/*********************************************************
��	  �ƣ�power_manage_task
��	  �ܣ���Դ����
���������
���������
��	  �أ�
ע�����
1.ACC OFF ϵͳ�Զ���������ģʽ
2.�͵磬ϵͳ�Զ��жϺĵ�ģ��
3.�ϵ磬Ԥ�������ڹ����ն˺󱸵��
4.���ѣ��ڷ�IDLEģʽ�£�ʹ��WAKE�����նˣ���ɶ�Ӧ���ܣ�
��ʱ����ɻ�ʱ��ϵͳ�Զ���������ģʽ
5. SLEEP>>PWR_DOWN>>PWR_OFF ģʽ, ����Ҫ��ת��ֻ����ACC ONʱ�������
�ſ��ܱ���硣
*********************************************************/
E_POWER_MAMNAGE pm_state;
E_POWER_MAMNAGE pm_cmd;
E_ACC_MAMNAGE pm_acc;

void Init_False_ACC(void) {
    pm_acc = IACC_IDLE;
}

void Close_Extern_Pwr(void) {
    //DIS_DVR_PWR; 		//���ػ�
    DIS_RK_PWR; //����pk��Դ
    //beep_off();	//������
    //DIS_PRINT_PWR;		//��ӡ����Դ
    //DIS_CAM12V_PWR;		//����¼�����Դ//��Ƭ����Դ
    //DIS_ADAS5V_PWR;
    //DIS_LCDVCC_PWR;
    //DIS_VBAT_PWR ;
    pm_acc = IACC_CLOSE;
    pub_io.b.dvr_open = false;
    s_Hi3520_Monitor.start = false;
    _printf_tr9("#### Close_Extern_Pwr ####\r\n");
    rouse_flag = true;   //�������߱�־λ
    rouse_state = false; //����
}

/*
*@��
*�޸�ʱ�䣺2021/11/19
*�޸����ݣ�����EN_USB5V_PWR ���ˣ�EN_HDD_PWR��EN_CAM12V_PWR��EN_PRINT_PWR��EN_LCDVCC_PWR��EN_VBAT_PWR
**/

void Open_Extern_Pwr(void) {
    EN_RK_PWR;
    //beep_on() ;	//������
    EN_CAM12V_PWR;
    EN_DVR_PWR;
    pwr_gps_lsm_lcd_inOut5v0(1, "O Ext Pwr");
    //EN_PRINT_PWR ;
    //EN_ADAS5V_PWR;
    //EN_LCDVCC_PWR; 	//�Զ����
    //EN_VBAT_PWR;
    //DIS_DVR_PWR; 		//���ػ�
    //DIS_CAM12V_PWR;
    pm_acc = IACC_OPEN;
    _printf_tr9("#### Open_Extern_Pwr ####\r\n");

    rouse_state = true; //����ת̬
    rouse_flag = false; //�������߱�־λ
}

void Close_Hi3520_Pwr(void) { //Open_Pwr_Close
    //DIS_DVR_PWR;
    DIS_RK_PWR; //RK��Դ
    delay20ms();
    delay20ms();
    delay20ms();
    DIS_GSM_PWR;
    delay20ms();
    delay20ms();
    delay20ms();
    EN_GSM_PWR;
    delay20ms();
    delay20ms();

    //EN_LCDVCC_PWR;
    DIS_RK_PWR; //RK��Դ
    delay20ms();
    delay20ms();
    //EN_VBAT_PWR;	ɲ��
    s_Hi3520_Monitor.start = false;
    //pub_io.b.dvr_open = true;
    pub_io.b.dvr_open = true;
    s_Hi3520_Monitor.ack = false;
    if (pm_acc == IACC_IDLE) {
        lcd_init(_false_, _true_);
    }
    pm_acc = IACC_CLOSE;
    _printf_tr9("#### Close_Hi3568Pwr ####\r\n");
    s_Hi3520_Monitor.reset_tmr = jiffies;
    s_Hi3520_Monitor.reset = true;

    rouse_state = false; //����
    rouse_flag = true;   //������
                         //Set_Awaken.asleep_state = true ; 	//����˯��״̬
}

/*
*@��
*�޸�ʱ�䣺2021/11/19
*�޸����ݣ�����EN_PRINT_PWR,EN_USB5V_PWR
**/
void Open_Pwr_All(void) {
    EN_DVR_PWR; //soc��Դ����//����:��Ƶ��ԴPE14
    delay20ms();
    EN_RK_PWR; //����RK��Դ/����ͷ��Դ	PE12
    delay20ms();
    //beep_on();	//aozuo������
    vout1_5v_enable(); //5V���			PA1
    EN_CAM12V_PWR;     //��Ƭ����Դ//����ԴPD12
    delay20ms();
    //EN_ADAS5V_PWR;
    delay20ms();
    //EN_LCDVCC_PWR;	//��Ļ��Դ  PC8�Զ����1
    EN_GSM_PWR; //4G_PD1
    delay20ms();
    EN_PRINT_PWR; //��ӡ����ԴPB2
    delay20ms();
    delay20ms();
    //EN_VBAT_PWR;		//ɲ��
    //DIS_RK_PWR;		//�ر�RK��Դ
    if (pm_acc == IACC_IDLE) {
        lcd_init(_false_, _true_);
    }
    pm_acc = IACC_OPEN;
    s_Hi3520_Monitor.start = false;

    /*2022-03-31 ��ʱ��*/
    //s_Hi3520_Monitor.start = true;//��ʱ��Ϊ����״̬,��ƽ̨�������·������ٻָ�
    pub_io.b.dvr_open = true;
    s_Hi3520_Monitor.ack = false;

    logi("___ mcu open all power ___");
    s_Hi3520_Monitor.reset_tmr = jiffies;
    s_Hi3520_Monitor.reset = true;
    rouse_state = true; //�ѻ���
    rouse_flag = false; //ȡ������
}

//bool Clos_Soc_flag =  false ;

void power_manage_task(void) {
    static E_POWER_MAMNAGE step = PWR_IDLE;
    E_POWER_MAMNAGE old = PWR_IDLE;
    //    E_ACC_MAMNAGE pm_accS;
    static bool new_step = false;
    //    static bool coun = false;
    static bool coun1 = false;
    static bool ActivePowerOff = false;
    //static bool   f0 = false;

    //    u8 iii;
    u8 *pAwaken = NULL;
    static bool abort = false;
    //    static u8 timer10_flag = 0;
    static u32 delay = 0;
    static u32 RK_AgreedSleep;
    static u32 tmr = 0;
    //    static u32 invc = 0;
    static u32 p_debug = 0;
    //    static u32 timer10 = 0;

    //static u32    close_time = 0 ;

    //    u8 acc = 0;
    //    u8 simulat_acc;
    static u16 coum = 0;

    if (tick < 1)
        return;

#if 1
    if (_covern(p_debug) > 5) { //���TR9δ������������ִ��
        if (car_state.bit.acc) {
            //acc = 1;
        }

        //simulat_acc = register_cfg.acc_simulate;
        p_debug = tick;
        //logd("$$$$acc_info:%d,pwr_per=%d,step=%d,pm_acc=%d,tick=%d$$$$",acc,mix.pwr_per,step,pm_acc,tick);
        if (u1_m.cur.b.debug) {
            logd("acc: %d, step: %d, (mcu)pm_acc: %d", car_state.bit.acc, step, pm_acc);
        }
    }
#endif
    //logd("mix.pwr_type = %d __ mix.pwr_per = %d __ mix.pwr_vol = %d",mix.pwr_type,mix.pwr_per,mix.pwr_vol );

    old = step;
    switch (step) {
    case PWR_IDLE: {
        pm_cmd = PWR_IDLE;
        pm_state = PWR_IDLE;

        if (car_state.bit.acc) {
            if (t_rst.rst) {
                if (pub_io.b.dvr_open) {
                    step = PWR_WAIT;
                    logd("t_rst.rst, reset_hi3560_open = %d", reset_hi3520_open);
                }
                break;
            }

            if (pub_io.b.key_s) //����״̬
            {
                tr9_frame_pack2rk(tr9_cmd_5012, 0, 0);
                step = PWR_WAIT;
                break;
            }
            if (pm_acc == IACC_IDLE) {
                Open_Pwr_All();
                lcd_bl_ctrl(true); //��һ�±���
            } else if (pm_acc == IACC_CLOSE) {
                if (rouse_state == false) //���ACC��RK�������߾ʹ�����RK��Դ
                {
                    if (login_net_123() == true) {
                        mix.placeIostate = 00;
                        Rk_Task_Manage.RK_send_0200_flag = true;
                        send_base_report();
                    }
                    //Open_Extern_Pwr();
                    Open_Pwr_All();
                    lcd_bl_ctrl(true);
                }
            }

            if (pub_io.b.dvr_open == false) {
                if (login_net_123() == true) {
                    mix.placeIostate = 00;
                    Rk_Task_Manage.RK_send_0200_flag = true;
                    send_base_report();
                }
                Open_Pwr_All();
            }
            coum = 0;
            abort = false;
            //            coun = false;
        } else //ACC��
        {
            if (pm_acc == IACC_IDLE) {
                /*
			if (coun == false)
			{
				delay = tick ;
				coun = true ;	
				logd("--1_����رյ�Դ--");
				tr9_frame_pack2rk( tr9_cmd_1818, 0, 0);
			}
			
			if(s_dvr_time.time_flag)
			{
				if(0 == (tmr++))
				{
					RK_AgreedSleep = tick ;	
					logd("--1_RK ͬ��رյ�Դ--6���ر�");
				}				
			}
			else
				RK_AgreedSleep =  tick ;
					
			if ((_covern(delay) > 29)||(_covern(RK_AgreedSleep)>= 6)) 	//RKͬ��رյ�Դ�����߳�ʱ10��
			{
				if( _covern(delay) >= 29){
					logd("--1_����ʱ30��,ǿ������--");
                }
				tmr = 0 ;
				s_dvr_time.time_flag = false ;
				dvr_power.cmd_ack = false;
				coun = false ;
			}
			*/
                if (pm_acc == IACC_IDLE) {
                    Close_Hi3520_Pwr();
                    bak_time_tick = tick - 1; //��ǰһ��
                }

                break;
            } else if (pm_acc == IACC_OPEN) {
                if (1 >= coum) {
                    if (rouse_flag == false) //false��δ����
                    {
                        if (coum == 0) {
#if 0 //22/04/12����Ҫ��ȡ��10������ʱ����	
					timer10 = tick ;	
					//	coum ++;			
					//}
					//else if(_covern(timer10) >= 600)//���ݺ���Ҫ��ACC�غ�10�����ٽ�������
					//{
#endif
                            coum += 10;
                            step = PWR_WAIT;
                            _memset((u8 *)&Set_Awaken, 0x00, sizeof(Set_Awaken));
                            spi_flash_read((u8 *)&Set_Awaken, flash_Awaken_param_addr, sizeof(Set_Awaken)); //������������

                            logd("--state :%d --timer :%d --note %d--", Set_Awaken.state_rouse, Set_Awaken.timer_rouse, Set_Awaken.note_rouse);
                            pAwaken = (u8 *)&Set_Awaken;
                            if ((pAwaken[0] == 0xFF) && (pAwaken[1] == 0xFF) && (pAwaken[2] == 0xFF) && (pAwaken[3] == 0xFF)) {
                                _memset(pAwaken, 0x00, sizeof(Set_Awaken));
                            }

                            /*
						if(Set_Awaken.timer_rouse){
							logd("����ʱ���%02x",Set_Awaken.time_Bucket +1);	
							logd("����:0x%02X",Set_Awaken.timerRouse+1);
							for(iii =0 ;iii< 4; iii++)
							logd("ʱ���%d: --on :%02d--off :%02d--",iii+1,Set_Awaken.timing_Awaken[iii].OnTime,Set_Awaken.timing_Awaken[iii].OffTime);
						}*/
                        }
                    }
                } else {
                    if (coum > 5000)
                        coum = 10;
                    else
                        coum++;
                }
            }
#if 0 //22/04/12����Ҫ��ȡ���Զ�����
		else if(pm_acc == IACC_CLOSE)	
		{
			if((1 == timer10_flag) && (_covern(timer10) >= 300))
			{
				Open_Pwr_All();
				timer10_flag = 2 ;
			}
			if((2 == timer10_flag) && s_Hi3520_Monitor.start )
			{
				timer10_flag = 0;
				send_base_msg_to_tr9(); 
				logd("�Զ�����");
				Close_Hi3520_Pwr();	
			}
				
		}
#endif
            if (pub_io.b.dvr_open == false) {
                if (pm_acc == IACC_IDLE) {
                    if (coun1 == false) {
                        delay = tick;
                        coun1 = true;
                        logi("--2_����رյ�Դ--");
                        tr9_frame_pack2rk(tr9_cmd_1818, 0, 0);
                    }

                    if (s_dvr_time.time_flag) {
                        if (0 == (tmr++)) {
                            RK_AgreedSleep = tick;
                            logi("--2_RK ͬ��رյ�Դ--6���ر�");
                        }
                    } else {
                        RK_AgreedSleep = tick;
                    }

                    if ((_covern(delay) > 29) || (_covern(RK_AgreedSleep) >= 6)) { //RKͬ��رյ�Դ�����߳�ʱ10��
                        if (_covern(delay) >= 29) {
                            loge("--2_����ʱ30��,ǿ������--");
                        }

                        Close_Hi3520_Pwr();
                        tmr = 0;
                        coun1 = false;
                        step = PWR_IDLE;
                        bak_time_tick = tick - 1; //��ǰһ��
                        dvr_power.cmd_ack = false;
                        s_dvr_time.time_flag = false;
                    }
                }

            } else {
                //step = PWR_WAIT;   		//��ʱ��ȫ��״̬
            }
        }
        abort = false;
        break;
    }
    case PWR_WAIT: {
        if (new_step) {
            delay = tick;
            if (pub_io.b.key_s) {
                tmr = 60;
            }
            if ((car_state.bit.acc == false) && s_dvr_time.flag) { //��ʱ�ر�
                tmr = (s_dvr_time.acc_off_time * 60) - 1;
                logi("ACC�ر�[ %02x:%02x], 0x%X���Ӻ�RK����", mix.time.min, mix.time.sec, s_dvr_time.acc_off_time);
                s_dvr_time.flag = 0;
            } else {
                tmr = 0;
            }

            if ((t_rst.rst)) {
                //s_dvr_time.flag = 1 ;
                tmr = 60;
            }
        }

        if (car_state.bit.acc) {
            if ((pub_io.b.key_s == false)) {
                if (pm_acc != IACC_OPEN) {
                    if (login_net_123() == true) {
                        mix.placeIostate = 00;
                        Rk_Task_Manage.RK_send_0200_flag = true;
                        send_base_report();
                    }
                    if (pm_acc != IACC_IDLE) {
                        Open_Extern_Pwr();
                        step = PWR_IDLE;
                    }

                    break;
                } else {
                    if (pm_acc != IACC_IDLE)
                        step = PWR_IDLE;
                    break;
                }
            }
        }

        if (abort && (car_alarm.bit.cut_volt == false)) {
            step = PWR_IDLE;
            break;
        }

        if (car_alarm.bit.cut_volt) {
            abort = true;
            if (mix.pwr_vol <= 600)
                Delay_detection = tick;
            break;
        }

        if ((_covern(delay) > tmr)) //����˯��
        {
#if 0
				if(1 == s_dvr_time.flag)		//��ʱ�ر���Ҫ��Ӧ��
				{
					tr9_frame_pack2rk( tr9_cmd_1818, 0, 0);
					logi("--����رյ�Դ--");
					s_dvr_time.flag = 0 ;
				}
#endif
            tr9_frame_pack2rk(tr9_cmd_1818, 0, 0);
            logi("--3_����رյ�Դ--");
            tmr = 0;
            delay = tick;
            step = PWR_SLEEP;
            pm_state = PWR_SLEEP;
            dvr_power.cmd_ack = false;

            break;
        }
        break;
    }
    case PWR_SLEEP: {
        if (new_step) {
            delay = tick;
        }

        if (car_state.bit.acc && (t_rst.rst == false)) {
            if ((pub_io.b.key_s == false)) {
                if (pm_acc != IACC_OPEN) {
                    if (login_net_123() == true) {
                        mix.placeIostate = 0x00;
                        Rk_Task_Manage.RK_send_0200_flag = true;
                        send_base_report();
                    }
                    if (pm_acc != IACC_IDLE)
                        Open_Extern_Pwr();
                }
                if (s_dvr_time.flag) {
                    step = PWR_IDLE;
                    break;
                }
            }
        }

        if (s_dvr_time.time_flag) {
            if (0 == (tmr++)) {
                RK_AgreedSleep = tick;
                logi("--3_RK ͬ��رյ�Դ--");
            }
        } else
            RK_AgreedSleep = tick;

        if ((_covern(delay) > 29) || (_covern(RK_AgreedSleep) >= 6)) //RKͬ��رյ�Դ�����߳�ʱ10��
        {
            if (_covern(delay) >= 29) {
                loge("--3_����ʱ30��,ǿ������--");
            }

            Close_Hi3520_Pwr();
            update_timer_task();

            //delay = tick;
            step = PWR_IDLE;
            abort = false;
            //timer10 = tick ;
            //timer10_flag = 1 ;
            dvr_power.cmd_ack = false;
            s_dvr_time.time_flag = false;
            bak_time_tick = tick - 1; //��ǰһ��

            break;
        }
        break;
    }
    default:
        step = PWR_IDLE;
        break;
    }

    new_step = (old == step) ? false : true;

    if (Rk_Task_Manage.RK_6047_flag) {
        if (!ActivePowerOff) {
            RK_AgreedSleep = tick;
            logi("Rk��������رյ�Դ");
        }
        ActivePowerOff = true;
    }
    if (ActivePowerOff && (_covern(RK_AgreedSleep) >= 6)) {
        Close_Hi3520_Pwr();
        step = PWR_IDLE;
        ActivePowerOff = false;
        bak_time_tick = tick - 1; //��ǰһ��
        dvr_power.cmd_ack = false;
        Rk_Task_Manage.RK_6047_flag = false;
    }

    if (t_rst.rst && (_pastn(t_rst.jiff) > 2000)) {
        if (font_lib_update.start)
            return;

        if (reset_hi3520_open == false)
            return;

        if (mix.moving == false) {
            _printf_tr9("sms_rst_into\r\n");
            _memset((u8 *)&t_rst, 0x00, sizeof(t_rst));
            save_run_parameterer();
            save_gps_basemation(&gps_base, &sys_time);

            log_write(event_mcu_boot_t_rst_rst);

            while (1) {
                t_rst.rst = false;
            }
        }
    }
}

/****************************************************************************
* ����:    updata_link_mx ()
* ���ܣ�ˢ��GPRS ��IP��������Ϣ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void updata_link_mx(void) {
    _memset((u8 *)&mx.addr, 0x00, 24);
    _memset((u8 *)&mx.apn, 0x00, 24);

    _strncpy((u8 *)&mx.apn, (u8 *)&server_cfg.apn, 24);

    server_cfg.select_tcp = 0x07;

    if (upgrade_m.enable) {
        gs.gprs1.server_type = SERVER_UPDATE;
        lm.IP1_status = SERVER_UPDATE;
        _strncpy((u8 *)&mx.addr, (u8 *)&server_cfg.ota_ip, 24);
        mx.port = (u16)server_cfg.ota_port;
        mx.protocol = (server_cfg.select_tcp & 0x04) ? TCP : UDP;
    } else {
        gs.gprs1.server_type = SERVER_NOMAL;
        lm.IP1_status = SERVER_NOMAL;
        _strncpy((u8 *)&mx.addr, (u8 *)&server_cfg.main_ip, 24);
        mx.port = (u16)server_cfg.main_port;
        mx.protocol = (server_cfg.select_tcp & 0x01) ? TCP : UDP;
    }

    gc.gsm.reset = true;
}

/****************************************************************************
* ����:    realtime_verify_server_cfg ()
* ���ܣ�ʵʱУ��GPRS ���������Ƿ���ȷ
* ��ڲ�������
* ���ڲ�������
* ע��: ������ƣ���ֹȫ�ֱ������޸ġ��������ȫ�ֱ������޸ģ�MCU�Զ�
��λ����SPI FLASH �ж�ȡ����������������Ļ�������MCU FLASH �ж�ȡ��������
�������ɴ��󣬽���ʼ����������֤�����ܹ��������ߣ����ն����޸Ĳ���
****************************************************************************/
bool verify_server_cfg(IP_SERVER_CONFIG_STRUCT *t2) {
    ///bool ret = false;
    u16 len = 0;

    if (t2->flag != 0x55aa)
        return false;

    len = _strlen((u8 *)t2->terminal_id);
    if (len == 0 || len >= 21)
        return false;

    len = _strlen((u8 *)t2->apn);
    if (len == 0 || len >= 24)
        return false;

    len = _strlen((u8 *)t2->main_ip);
    if (len == 0 || len >= 24)
        return false;

    len = _strlen((u8 *)t2->bak_ip);
    if (len == 0 || len >= 24)
        return false;

    if (t2->main_port == 0 || t2->main_port >= 0xfff0)
        return false;

    if (t2->bak_port == 0 || t2->bak_port >= 0xfff0)
        return false;

    return true;
}

/****************************************************************************
* ����:    link_manage_task ()
* ���ܣ�����IP���Ӻ���·���
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void link_manage_task(void) {
    static u32 over_tick = 0;

    static enum E_LINK_MANAGE {
        E_LINK_IDLE,
        E_LINK_DIAL,
        E_LINK_RUN,
        E_LINK_ERR
    } step = E_LINK_IDLE;

    if (lm.sms_rst_en == true) {
        if (_coveri(lm.sms_rst_tick) > 30) {
            lm.reset = false;
            lm.sms_rst_en = false;
            gc.gsm.reset = true;
            step = E_LINK_IDLE;
            logi("lm.sms_rst_en!!!");
            return;
        }
    } else if (lm.reset == true) {
        lm.reset = false;
        lm.sms_rst_en = false;
        gc.gsm.reset = true;
        step = E_LINK_IDLE;
        logi("exe_gsm_reset!!!");
        return;
    }

    switch (step) {
    case E_LINK_IDLE: //��ʼ��װ��IP ����������IP�ȣ���ȷ�����·������ʱ��ȡ�
        //IP �����޸ĵȣ���Ҫ���²���
        //����IP�� MX
        if (pub_io.b.dvr_open) {
            break;
        }
        updata_link_mx();

        if (lm.IP1_status == SERVER_NOMAL) {
            over_tick = 30 * 60;
        } else if (lm.IP1_status == SERVER_UPDATE) {
            over_tick = (16 * 60);
        } else { //����
            over_tick = 30 * 60;
        }

        lm.online_tick = tick;
        step = E_LINK_DIAL;
        break;
    case E_LINK_DIAL:
        lm.ack = false;
        if (pub_io.b.dvr_open) {
            step = E_LINK_IDLE;
            break;
        }

#if 0			
			if( _covern(lm.online_tick) > over_tick )
			{
				step = E_LINK_ERR;
				break;
			}
			
			if( lm.link_cnt >= 30 )
			{
				step = E_LINK_ERR;
				lm.link_cnt = 0;
				break;				
			}
#endif

        if (gc.gprs >= NET_CONNECT) {
            lm.online_tick = tick;
            lm.link_cnt = 0;
            step = E_LINK_RUN;
        }
        break;
    case E_LINK_RUN:
        if (pub_io.b.dvr_open) {
            step = E_LINK_IDLE;
            break;
        }

        if (_covern(lm.online_tick) > over_tick) {
            step = E_LINK_ERR;
            break;
        }

        if (lm.ack) {
            lm.ack = false;
            lm.online_tick = tick;
            lm.link_cnt = 0;
            step = E_LINK_RUN;
            break;
        }
        break;
    case E_LINK_ERR:

        //����ִ�и�λ
        _printf_tr9("LINK ERR : mcu reset\r\n");

        t_rst.rst = true;
        t_rst.del = true;
        t_rst.send = true;
        t_rst.jiff = jiffies;

        step = E_LINK_IDLE;
        break;
    default:
        step = E_LINK_IDLE;
    }
}

/*********************************************************
��    �ƣ�calculation_sum_mile
��    �ܣ����㳵����ʻ�������(��λ����)
�����������
��    ����������ʻ�������
��д���ڣ�2011-02-18
**********************************************************/
void calculation_sum_mile(void) {
    u32 temp_mil_value = 0x00;
    static u16 mil_mod = 0;
    static u8 nold = 0;
    static u32 mile_bak = 0;
    static u16 old_gps_save_pos = 0;

    temp_mil_value = 0;
    if (gps_active() && nold != (u8)gps_base.pnumber) {
        nold = (u8)gps_base.pnumber;
        if (car_state.bit.acc) //�ڶ�λ��ACC�������˶�״̬��ʱ��������
        {
            /*********************************************************
            gps_base.void_speed ��λ1000m/h
            temp_mil_value , mil_mod��λ0.01m
            �ٶ�ϵ��Ϊ1.03��
            **********************************************************/
            if (gps_base.speed > 180) //����ٶȳ���280KM/H , �˳�����
                return;

            temp_mil_value = (gps_base.speed * 1000) / 36;
            temp_mil_value = (temp_mil_value * 106) / 100; ///103

            run.total_dist += (temp_mil_value / 100);

            mil_mod += (temp_mil_value % 100);
            run.total_dist += (mil_mod / 100);

            mil_mod %= 100;
            if ((old_gps_save_pos == 0))
                old_gps_save_pos = save_gps_pos;
            if (mile_bak < run.total_dist && (run.total_dist - mile_bak) > 1000) {
                mile_bak = run.total_dist;
                save_run_parameterer(); //����120*10����
            }
        }
    }
}

/*********************************************************
��    �ƣ�Ic_Relay_One_Control
��    �ܣ��̵���һ���ƺ���
���������flag ���Ʊ�־ 1 �Ͽ��̵��� 0 �պϼ̵���
��    ������
��д���ڣ�2011-02-14
**********************************************************/
void Ic_Relay_One_Control(u8 flag) {
    if (flag == 0x01) {
        //�ر���·
        relay_enable();
    } else {
        //��ͨ��·--0
        relay_disable();
        //relay_enable();
    }
}

/****************************************************************************
* ����:    car_out ()
* ���ܣ���·����
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void car_out(u16 off, u16 on, u16 cycle, u8 level) {
    if (level < car.level) return;
    car.time_off = off;
    car.time_on = on;
    car.cycle = cycle;
    car.phase = 0;
    car.level = level;
}

/****************************************************************************
* ����:    car_out_ctrl ()
* ���ܣ���·����
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void car_out_ctrl(void) {
    static bool state = false;

    if (car.cycle == 0) //��ͨ��·
    {
        Ic_Relay_One_Control(0);
        car.level = 0;
        return;
    }

    if (car.cycle > 0xf000) //�Ͽ���·
    {
        Ic_Relay_One_Control(1);
        car.level = 0;
        return;
    }

    if (state == false) {
        if (_pastn(car.phase) > car.time_off) //��ͨ��·
        {
            state = true;
            Ic_Relay_One_Control(0);
            car.phase = jiffies;
        }
    } else if (_pastn(car.phase) > car.time_on) //�ر���·
    {
        state = false;
        Ic_Relay_One_Control(1);
        car.phase = jiffies;
        car.cycle--;
    }
}

void ctrl_relay_status(u8 stat) {
    if (stat > 0) { //���ܹر���·
        logi("$$$$cut_oil = 1$$$$");
        rcmd.b.cut_oil = true;
        rcmd.b.rels_oil = false;
        tr9_car_status.bit.limitraise = true;
    } else {
        logi("$$$$rels_oil = 0$$$$");
        rcmd.b.cut_oil = false;
        rcmd.b.rels_oil = true;
        tr9_car_status.bit.limitraise = false;
    }
}

/****************************************************************************
* ����:    init_car_out ()
* ���ܣ���·����
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void init_car_out(void) {
    car_out(0, 1000, 0, LEVEL_LST);
}

/****************************************************************************
* ����:    cut_oil_ctrl_task ()
* ���ܣ���·��������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void cut_oil_ctrl_task(void) {
    static u32 step = 0;

    if (rcmd.b.cut_oil) {
        step = 130;
        car_out(1000, 0, 0xffff, LEVEL_JACKING); //�Ͽ���·

        if (rcmd.b.cut_oil != run.car.b.cut_oil) {
            run.car.b.cut_oil = rcmd.b.cut_oil;
            save_run_parameterer();
        }
    } else {
        if (step || rcmd.b.rels_oil) {
            step = 0;
            rcmd.b.rels_oil = false;
            rcmd.b.cut_oil = false;
            car_out(0, 1000, 0, LEVEL_JACKING);
        }
    }

    car_out_ctrl();
}

#if (P_RCD == RCD_BASE)

/****************************************************************************
* ����:    check_plus_speed ()
* ���ܣ����������ٶ�
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
//220828�ٶ�����:�����ٶȣ�GPS�ٶȣ��������ԣ�����GPS�ٶȸ��������ٶȣ�Ҫ���������ٶ�Ӳ��
void check_plus_speed(void) {
    u16 cur_speed = 0;
    static u32 cur_plus;

    static u8 s_index = 0;

    static u32 delay_check = 0;
    static u32 plus_buf[10] = {0};
    static u8 t_index;

    //ÿ�����һ��
    if (_pastn(delay_check) < 50) {
        return;
    }
    delay_check = jiffies;
    //delay_check += 50;

    if (gps_base.speed > 1) { //tongren888	GPS�ٶ�
        cur_speed = gps_base.speed;
        speed_state.gps_s = gps_base.speed;
        speed_state.pulse_s = plus_speed;
    } else if (plus_speed > 1) { //���峵�ٴ���0
        cur_speed = plus_speed;
        speed_state.gps_s = gps_base.speed;
        speed_state.pulse_s = cur_speed;
    } else if (wheel_speed > 1) { //CAN�ٶ�
        cur_speed = wheel_speed;
    } else {
        cur_speed = gps_base.speed;
    }

    if (run.lock_gps == 0x55)
        cur_speed = 0;

    mix.complex_speed = cur_speed;
    mix.complex_speed01 = (cur_speed * 10);
    if (mix.complex_speed > 180) {
        mix.complex_speed = 180;
    }

    //Ҫ�޸��˲��㷨
    cur_plus -= plus_buf[s_index];
    plus_buf[s_index] = speed_plus;
    cur_plus += speed_plus;
    s_index = (s_index + 1) % 10;
    speed_plus = 0;

    if (t_index < 10) {
        t_index++;
        return;
    }

    if (cur_plus >= 5) { //��??��???��ΪV����������ϵ��ΪX����ÿ����תX�Σ�ʱ��TСʱ�ڲɼ���M????�壬ÿת??�ܲ�??8??????
                         //  ��һ����ȷ��    V = M /(8*X*T����ǧ��/Сʱ)
                         // plus_speed = (3600 * cur_plus) / factory_para.set_speed_plus;
                         // plus_speed = (360 * cur_plus) / factory_para.set_speed_plus;
                         //22-06-15 	һ��һ�η�Ӧ�ٶ���������50����
        plus_speed = ((3600 * 2) * cur_plus) / factory_para.set_speed_plus;
    } else {
        plus_speed = 0;
    }
}
#endif

/****************************************************************************
* ����:    public_manage_task ()
* ���ܣ��������ܹ�������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void public_manage_task(void) {
    check_plus_speed();

    // custom_100ms_task();
}

//RST-RK:ϵͳ����
void Hi3520_Power_Off_Up(void) {
    if (_covern(s_Hi3520_Monitor.deal_tmr) > 7200) { //240
        if (s_Hi3520_Monitor.power_off) {
            if (_covern(s_Hi3520_Monitor.off_time) >= 3) {
                EN_DVR_PWR;
                _printf_tr9("hi3568 PWR start -2-\r\n");
                _memset((u8 *)&s_Hi3520_Monitor, 0, sizeof(HI3520_MONITTOR));

                s_Hi3520_Monitor.off_time = tick;
                s_Hi3520_Monitor.power_off = false;
                s_Hi3520_Monitor.start = false;
                s_Hi3520_Monitor.ack = false;

                s_Hi3520_Monitor.reset = true;
                s_Hi3520_Monitor.reset_tmr = jiffies;

                s_Hi3520_Monitor.deal_tmr = tick;
                if (++hi3520_rcnt >= 3) {
                    t_rst.rst = true;
                    t_rst.del = false;
                    t_rst.send = false;
                    t_rst.jiff = jiffies;
                    reset_hi3520_open = true;
                }
            }
        } else {
            s_Hi3520_Monitor.power_off = true;
            s_Hi3520_Monitor.off_time = tick;
            s_Hi3520_Monitor.start = false;
            DIS_DVR_PWR;
            _printf_tr9("hi3568 PWR start -1-\r\n");
        }
    }
}

#define RK_RST_20220820 1
#if (RK_RST_20220820)
/*���壺1-30���ӣ���Ӧ������RK���������ٴ�֮�󣬽ӹ�4Gģ��*/
typedef struct {
    int seconds;
    s8 takeOver4gCnt;
} min_rstCnt_fosterCare_st;
//��������1���ӡ�2���ӡ�3���ӡ�5���ӡ�8���ӡ�12���ӡ�17���ӡ�23���ӡ�30����
static const min_rstCnt_fosterCare_st min_rstCnt_arr[] = {
    //{n, ��С��2}
    {0, 0},       //0//���ӹܣ�����λ��������
    {60 * 1, 20}, //1//1����
    {60 * 2, 10}, //2//2����
    {60 * 3, 7},  //3//3����
    {60 * 5, 4},  //4//5����
    {60 * 8, 3},  //5//8����
    {60 * 12, 2}, //6//12����
    {60 * 17, 2}, //7//17����
    {60 * 23, 2}, //8//23����
    {60 * 30, 2}, //9//30����
};
/*
ͨ��ϵͳ������������ȡ����RK��ʱ�����룩
0-������
1-30���ӣ��������
����-������
����ֵ����λ����
*/
static s8 getGapTimeS_Foster_care() {
    if (sys_cfg.foster_care >= sys_cfg_range.foster_care.min && sys_cfg.foster_care <= sys_cfg_range.foster_care.max) {
        if (sys_cfg.foster_care == 0) {
            return 0; //���ӹܣ�����λ��������
        } else {
            return min_rstCnt_arr[sys_cfg.foster_care].seconds;
        }
    } else {
        //RST-RK:ϵͳ����
        if (!u1_m.cur.b.rcd_3c /*�ڲ�ģʽ�����ڶ�ȡ19056����Ϣ��*/) {
            loge("--foster_care val error--");
        }
        return 0; //���ӹܣ�����λ��������
    }
}
/*
��ȡ�ӹ�4G�ġ���λ����+1��
���ݲ�ͬ�ĸ�λʱ�䣬�ƶ���ͬ�ĸ�λ���£��ӹ�4Gʱ��
*/
static int getTakeOver4gCnt_Foster_care() {
    if (sys_cfg.foster_care >= sys_cfg_range.foster_care.min && sys_cfg.foster_care <= sys_cfg_range.foster_care.max) {
        if (sys_cfg.foster_care == 0) {
            return 0; //���ӹܣ�����λ��������
        } else {
            return min_rstCnt_arr[sys_cfg.foster_care].takeOver4gCnt;
        }
    } else {
        //RST-RK:ϵͳ����
        loge("--foster_care val error--");

        return 0; //���ӹܣ�����λ��������
    }
}
#endif /*RK_RST_20220820*/

/**
 * @brief  ������ RK ��״̬
 * @param  ����
 * @return����
 * @exception NSException �����׳����쳣.
 * @author ��  
 * @date 2022-08-19 22:30:00 ��ʱ��
 * @version 1.0 ���汾  
 * @property �����Խ���
 * */
void Monitor_Hi3520_Status(void) {
    static u8 reset = 0;

    if (t_rst.rst) {
        return;
    }

    s8 takeOver4gCnt = getTakeOver4gCnt_Foster_care();
    if (takeOver4gCnt > 0) {
        if (Rk_Task_Manage.resetCount < takeOver4gCnt) { //����
            int gapTimeS = getGapTimeS_Foster_care();
            if (gapTimeS > 0) {
                if ((car_state.bit.acc /*ACC���ڣ����ڿ�*/) && (_covern(Rk_Task_Manage.resetTime) >= gapTimeS)) {
                    //s_Hi3520_Monitor.ack = false ;
                    //s_Hi3520_Monitor.reset = true ;
                    //s_Hi3520_Monitor.start = false ;
                    loge("--MCU �����δ�յ�RK��������RK--");

                    Rk_Task_Manage.resetCount++;
                    goto RKreset;
                }
            }
        } else if (Rk_Task_Manage.resetCount == takeOver4gCnt /*RK��Ӧ�𣬻ᱻ��0*/) { //�ӹ�4G
            loge("RK�޷�����MCU�ӹ�4Gģ��");

            s_Hi3520_Monitor.ack = false;
            s_Hi3520_Monitor.reset = true;
            s_Hi3520_Monitor.start = false;
            Rk_Task_Manage.resetCount++;
        }
    }

    if (s_Hi3520_Monitor.ack) {
        s_Hi3520_Monitor.reset = false;
        //s_Hi3520_Monitor.reset = jiffies;

        _memset((u8 *)&s_Hi3520_Monitor, 0, sizeof(HI3520_MONITTOR));
        s_Hi3520_Monitor.start = true;

        s_Hi3520_Monitor.deal_tmr = tick;
        s_Hi3520_Monitor.deal = false;
        hi3520_rcnt = 0;
        return;
    }

    if (car_state.bit.acc == false) {
        return;
    }

    Hi3520_Power_Off_Up();

    if (s_Hi3520_Monitor.reset == false) {
        s_Hi3520_Monitor.reset_tmr = jiffies;
        reset = 0;
        return;
    }

    if (_pastn(s_Hi3520_Monitor.reset_tmr) > 200 /*ms*/) { //�ж�ʱ��
    RKreset:
        //RST-RK:RK��λ����
        if (reset == 0) {
            EN_RESET_HI35XX;
            logd("RST hi3520 start -4-");
            reset = 1;
        } else if (reset == 1) {
            logd("RST hi3520 start -3-");
            //_memset((u8 *)&s_Mcu_Debug, 0, sizeof(USB_MONITTOR));
            s_Hi3520_Monitor.reset = false;
            s_Hi3520_Monitor.reset_tmr = jiffies;
            Rk_Task_Manage.resetTime = tick;
            DIS_RESET_HI35XX;
            reset = 0;
        }

        s_Hi3520_Monitor.reset_tmr = jiffies;
    }
}
