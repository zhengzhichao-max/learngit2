/**
  ******************************************************************************
  * @file    pub.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   共用功能或函数管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

u32 tick = 0;    //系统时间，秒
u32 jiffies = 0; //系统时间，毫秒
//unsigned long jiffies = 0 ;
bool rouse_state; //唤醒状态

#if (P_RCD == RCD_BASE)
u32 speed_plus = 0;   //获取脉冲速度
u32 plus_speed = 0;   //记录脉冲速度
u16 wheel_speed = 0;  //CAN 提取速度
u16 engine_speed = 0; //CAN 提取速度
u16 average_1km = 0;

u16 speed_jiffes = 0;
//u32 speed_jiffes_s = 0;
//u32 speed_jiffes_ms = 0;

#endif

u16 start_tr9_tmr = 300;
u16 car_weight;

time_t sys_time; //system time 系统时钟，北京时间
uc8 MonthSizeTab[] =
    {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31};

MIX_GPS_DATA mix;                 //混合数据，用于各模块之间信息交互
TERMINAL_RESET_STRUCT t_rst;      //执行终端复位
VEHICLE_ALARM_UNION bypass_alarm; //报警取消
run_parameter run;                //运行的参数
VEHICLE_ALARM_UNION car_alarm;    //车辆报警
UNION_VEHICLE_STATE car_state;    //车辆状态

sCommunicationSet phone_cfg; //电话号码配置
gprs_link_manange lm;        //GPRS 链路管理
REMOTE_CAR rcmd;             //车辆远程管理命令
out_ctrl_block car;          //断油断电管理
u32 heart_beat_tick = 0;
DVR_POWER_CTRL dvr_power;
can_speed_cali can_s_cali;

HI3520_MONITTOR s_Hi3520_Monitor;
static u8 hi3520_rcnt = 0;

//*****************************************************************************
//*****************************************************************************
//-----------		            变量定义   	                  -----------------
//-----------------------------------------------------------------------------

//_extern_    gbk_unicode_buff_struct  gbk_unicode_buff;  //国标码与UNICODE码转换

user_time_struct user_time; //用户时间计数

mult_usart_set_struct mult_usart_set; //设置选定的多路串口

query_data_union query;                     //查询类结构
gps_data_struct gps_data;                   //GPS相关数据
state_data_struct state_data;               //状态数据
set_para_struct set_para;                   //设置主机参数
recorder_para_struct recorder_para;         //行驶记录仪参数
host_no_save_para_struct host_no_save_para; //不需要保存的设置参数
key_data_struct key_data;                   //按键操作
menu_cnt_struct menu_cnt;                   //菜单结构体
message_window_dis_struct mw_dis;           //消息显示结构体
menu_auto_exit_struct menu_auto_exit;       //菜单自动退出结构体
menu_report_struct menu_report;             //报告界面
u16 dis_multi_page_cnt;                     //多页显示界面中已经显示的内容

input_method_struct input_method;       //输入法
password_manage_struct password_manage; //密码管理

ic_type_enum ic_type;        //IC卡类型
ic_card_date_struct ic_card; //IC卡数据

buzzer_voice_struct buz_voice;       //蜂鸣器相关数据
voice_source_struct buz_cus[2];      //自定义蜂鸣器鸣叫数据
buzzer_cus_time_struct buz_cus_time; //自定义蜂鸣器鸣叫时间

call_record_struct call_record; //通话记录相关数据
phone_data_struct phone_data;   //电话相关数据

sms_up_center_struct sms_up_center;               //上传给中心的信息参数数据
sms_union_data_union sms_union_data;              //信息联合体
u8 sound_play_buff[sound_play_buff_max_lgth + 2]; //语音朗读缓冲区  前两个字节表示长度，短整形数据类型

//扩展多路串口usart0:功能变量定义
u8 uart_set_ok_flag[3];                 //多路串口扩展里面的串口设置成功标志位，=1表示设置成功；=0表示没有设置或者设置失败,各个串口设置后的‘*’显示
u8 extend_uart_cnt;                     //多路串口扩展里面的可扩展串口的个数
u8 ic_driver_ifo_num;                   //从U盘读取到的驾驶员信息数目 2018-1-6
u32 extend_uart_buff[9];                //多路串口扩展接收缓存
u8 tmp_cnt;                             //多路串口扩展里面的uartx可选功能的个数
u8 extend_uart_send_bit[7];             //用于记录多路串口扩展里面的uartx中的各个选项的位标志，比如extend_uart_send_bit[0]记录的是第1个显示项是第几位是1，该元素里面就填几
u8 current_uart_flag[3];                //多路串口扩展里面的当前选中串口的‘*’选择标志位,各个串口查询时的‘*’显示
u8 tmp_bit_cnt;                         //用于记录多路串口扩展里面的uartx中的各个选项的位标志，指向extend_uart_send_bit
unsigned char *tmp_uart_dis[3];         //临时串口的显示，主要用来存放显示多路串口扩展里面的USART1-3的显示
unsigned char *tmp_icDriverName_dis[5]; //用来记录5个司机姓名 2018-1-6

// TF卡存储图片时定义的变量信息
//_extern_  u8 time[50];                  //存放存储图片时候的GPS时间
//_extern_ u8 full_filename[50];          //存储图片时的文件名

//通话记录所需变量定义
phbk_data_struct phbk_call_in_data; //存储当前来电与电话薄对比后，从电话簿里面提取出来的相应的电话类型，姓名，号码信息

print_auto_data_struct print_auto_data; //自动打印数据

//域名查询所需变量定义
u8 realm_name_flag; //域名标志位，主域名:  0x01   ;备用域名:0x02
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//spi_data_struct      spi1_data;      //SPI接口数据交互
spi_ack_struct slv_send_flag;

SLAVE_RECIVE_DATA_STRUCT slv_rev_data;
SLAVE_SEND_DATA_STRUCT slv_send_data;

//日期，时间相关函数
/****************************************************************************
* 名称:    get_monsize ()
* 功能：计算闰月多出来的一天
* 入口参数：无
* 出口参数：无
****************************************************************************/
u8 get_monsize(u8 year, u8 month) {
    u8 n;

    n = MonthSizeTab[month - 1];
    if (month == 2 && year % 4 == 0)
        n += 1;
    return n;
}

/****************************************************************************
* 名称:    get_jiffies ()
* 功能：得到系统毫秒级时间
* 入口参数：无
* 出口参数：无
****************************************************************************/
u32 get_jiffies(void) {
    u32 n;

    cli();
    n = jiffies;
    sei();
    return n;
}

/****************************************************************************
* 名称:    rtime_cmp ()
* 功能：比较时间
* 入口参数：无
* 出口参数：无
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
* 名称:    delay10us ()
* 功能：延时
* 入口参数：无
* 出口参数：无
****************************************************************************/
void delay10us(void) {
    u8 i = 2;
    while (i--) {
        delay1us();
    }
}

/****************************************************************************
* 名称:    delay50us ()
* 功能：延时
* 入口参数：无
* 出口参数：无
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
//该函数的功能是提供微秒级的延时，本函数的延时时间大概是10微秒(@72MHz)
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
* 名称:    delay1ms ()
* 功能：延时
* 入口参数：无
* 出口参数：无
****************************************************************************/
void delay1ms(void) {
    u8 i = 20;
    while (i--) {
        delay50us();
    }
}

/****************************************************************************
* 名称:    delay20ms ()
* 功能：延时
* 入口参数：无
* 出口参数：无
****************************************************************************/
void delay20ms(void) {
    u8 i = 20;
    while (i--) {
        delay1ms();
    }
}

/****************************************************************************
* 名称:    delay_ms ()
* 功能：延时
* 入口参数：无
* 出口参数：无
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
* 名称:    delay50ms ()
* 功能：延时
* 入口参数：无
* 出口参数：无
****************************************************************************/
void delay50ms(void) {
    int i = 1000;
    while (i--) {
        delay50us();
    }
}

/****************************************************************************
* 名称:    modify_beijing_time_area ()
* 功能：格林威治时间转换为北京时间
* 入口参数：无
* 出口参数：无
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
* 名称:    update_timer_task ()
* 功能：北京时间，自动刷新系统时间
* 入口参数：无
* 出口参数：无
****************************************************************************/
void update_timer_task(void) {
    static u32 chase = 0;
    static u8 sec_bak = 0;
    //    static u8 Time = 0;
    /*
	  if(gps_base.ftime.sec != Time)
	  {//首先取GPS时间做为开始秒计时
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
    logd("实时时间--%02d:%02d:%02d", sys_time.hour, sys_time.min, sys_time.sec);
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
* 名称:    update_mix_infomation ()
* 功能：刷新参数
* 入口参数：无
* 出口参数：无
****************************************************************************/
void update_mix_infomation(void) {
    VEHICLE_ALARM_UNION bypass_tmp;
    //    u32 speed = 0;

    //u16 speed_temp = 0;
    //更新报警状态
    if (bypass_alarm.reg != 0 || alarm_cfg.bypass_sw.reg != 0) {
        //按位与，若相应状态变为零, 清除相应旁路数据位
        bypass_alarm.reg &= car_alarm.reg;
        //添加JT808报警平台屏蔽字
        //取反，被旁路的数据位为0
        bypass_tmp.reg = (~bypass_alarm.reg);
        bypass_tmp.reg &= (~alarm_cfg.bypass_sw.reg);
        mix.alarm.reg = bypass_tmp.reg & car_alarm.reg;
    } else
        mix.alarm.reg = car_alarm.reg;

    //更新车辆状态
    car_state.bit.fix = (gps_base.fixed == true) ? true : false;     //卫星定位是否锁定
    car_state.bit.lat = (gps_base.locate.lati < 0) ? true : false;   //纬度:0 北纬 ; 1 南纬
    car_state.bit.lng = (gps_base.locate.lngi < 0) ? true : false;   //经度:0 东经 ; 1 西经
    car_state.bit.cut_oil = (rcmd.b.cut_oil == true) ? true : false; //
    car_state.bit.cut_pwr = (rcmd.b.cut_oil == true) ? true : false;
    //2014-07-08
    car_state.bit.fix_gps = (gnss_cfg.select.b.GPS_EN == 1) ? true : false;      //使用 gps  定位
    car_state.bit.fix_bd = (gnss_cfg.select.b.BD_EN == 1) ? true : false;        //使用 北斗 定位
    car_state.bit.fix_glonass = (gnss_cfg.select.b.GLNS_EN == 1) ? true : false; //全球定位
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
    bd_rcd_io.b.far_light = (rcd_io.b.far_light == true) ? true : false;     //远光
    bd_rcd_io.b.brake = (rcd_io.b.brake == true) ? true : false;             //刹车
    bd_rcd_io.b.near_light = (rcd_io.b.near_light == true) ? true : false;   //近光
    bd_rcd_io.b.right_light = (rcd_io.b.right_light == true) ? true : false; //右转
    bd_rcd_io.b.left_light = (rcd_io.b.left_light == true) ? true : false;   //左转
#endif
    mix.fixed = gps_base.fixed == true ? true : false; //是否定位
    mix.car_state.reg = car_state.reg;                 //第一状态
    //更新车辆相关信息
    mix.csq = gs.gsm.squality; //信号强度

    //更新GPS相关信息
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
//AD ，电压采集，转换
/*********************************************************
名    称：Ic_Get_ExternalPowerValue
功    能：获取外部输入电源电压值
输入参数：无
输    出：当前外部输入电源电压值
编写日期：2011-02-14     \
910 - 62 
430 - 18
**********************************************************/
static u16 Get_External_Power_Value(void) {
    u32 u32_Power_AD = 0x00;
    u16 power_value = 0;

    u32_Power_AD = ADC_GetConversionValue(ADC1);

    u32_Power_AD = (u32_Power_AD * 330) / 0x1000; //单位0.01V

    u32_Power_AD = (u32_Power_AD * 956) / 47;                            //硬件分压系数
    power_value = u32_Power_AD > 700 ? u32_Power_AD + 30 : u32_Power_AD; //添加0.30V二极管压降
    return (u16)power_value;
}
#endif

/****************************************************************************
* 名称:    cal_car_power_type ()
* 功能：计算车系电压范围
* 入口参数：无
* 出口参数：无
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
* 名称:    cal_car_power_type ()
* 功能：计算汽车电平电压百分比
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u8 cal_power_per(u8 pwr_type, s32 vol) {
    u8 per;
    s32 value;

    per = 101;               // 锟斤拷锟斤拷锟斤拷效
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
* 名称:    cal_car_power_type ()
* 功能：计算当前汽车电压
* 入口参数：无
* 出口参数：无
****************************************************************************/
void adc_power_task(void) {
    //6057主电压查询://终端主电压查询【RK->MCU】
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
        adv = Read_Power_Adc_Value(); // 实际电压
        *vhead = adv;
        incp_vbuf(vhead);
        for (i = 0; i < VBUF_SIZE; i++)
            value += vbuf[i];
        value /= VBUF_SIZE; //平均电压
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
名	  称：power_manage_task
功	  能：电源管理
输入参数：
输出参数：
返	  回：
注意事项：
1.ACC OFF 系统自动进入休眠模式
2.低电，系统自动切断耗电模块
3.断电，预留，用于管理终端后备电池
4.唤醒，在非IDLE模式下，使用WAKE唤醒终端，完成对应功能，
当时间完成或超时，系统自动进入休眠模式
5. SLEEP>>PWR_DOWN>>PWR_OFF 模式, 不需要逆转，只有在ACC ON时汽车电池
才可能被充电。
*********************************************************/
E_POWER_MAMNAGE pm_state;
E_POWER_MAMNAGE pm_cmd;
E_ACC_MAMNAGE pm_acc;

void Init_False_ACC(void) {
    pm_acc = IACC_IDLE;
}

void Close_Extern_Pwr(void) {
    //DIS_DVR_PWR; 		//开关机
    DIS_RK_PWR; //海格：pk电源
    //beep_off();	//蜂鸣器
    //DIS_PRINT_PWR;		//打印机电源
    //DIS_CAM12V_PWR;		//海格：录像机电源//单片机电源
    //DIS_ADAS5V_PWR;
    //DIS_LCDVCC_PWR;
    //DIS_VBAT_PWR ;
    pm_acc = IACC_CLOSE;
    pub_io.b.dvr_open = false;
    s_Hi3520_Monitor.start = false;
    _printf_tr9("#### Close_Extern_Pwr ####\r\n");
    rouse_flag = true;   //清理休眠标志位
    rouse_state = false; //休眠
}

/*
*@秦
*修改时间：2021/11/19
*修改内容：屏蔽EN_USB5V_PWR 打开了：EN_HDD_PWR，EN_CAM12V_PWR，EN_PRINT_PWR，EN_LCDVCC_PWR，EN_VBAT_PWR
**/

void Open_Extern_Pwr(void) {
    EN_RK_PWR;
    //beep_on() ;	//蜂鸣器
    EN_CAM12V_PWR;
    EN_DVR_PWR;
    pwr_gps_lsm_lcd_inOut5v0(1, "O Ext Pwr");
    //EN_PRINT_PWR ;
    //EN_ADAS5V_PWR;
    //EN_LCDVCC_PWR; 	//自定义高
    //EN_VBAT_PWR;
    //DIS_DVR_PWR; 		//开关机
    //DIS_CAM12V_PWR;
    pm_acc = IACC_OPEN;
    _printf_tr9("#### Open_Extern_Pwr ####\r\n");

    rouse_state = true; //在线转态
    rouse_flag = false; //清理休眠标志位
}

void Close_Hi3520_Pwr(void) { //Open_Pwr_Close
    //DIS_DVR_PWR;
    DIS_RK_PWR; //RK电源
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
    DIS_RK_PWR; //RK电源
    delay20ms();
    delay20ms();
    //EN_VBAT_PWR;	刹车
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

    rouse_state = false; //休眠
    rouse_flag = true;   //休眠中
                         //Set_Awaken.asleep_state = true ; 	//进入睡眠状态
}

/*
*@秦
*修改时间：2021/11/19
*修改内容：屏蔽EN_PRINT_PWR,EN_USB5V_PWR
**/
void Open_Pwr_All(void) {
    EN_DVR_PWR; //soc电源开关//海格:音频电源PE14
    delay20ms();
    EN_RK_PWR; //海格：RK电源/摄像头电源	PE12
    delay20ms();
    //beep_on();	//aozuo蜂鸣器
    vout1_5v_enable(); //5V输出			PA1
    EN_CAM12V_PWR;     //单片机电源//主电源PD12
    delay20ms();
    //EN_ADAS5V_PWR;
    delay20ms();
    //EN_LCDVCC_PWR;	//屏幕电源  PC8自定义高1
    EN_GSM_PWR; //4G_PD1
    delay20ms();
    EN_PRINT_PWR; //打印机电源PB2
    delay20ms();
    delay20ms();
    //EN_VBAT_PWR;		//刹车
    //DIS_RK_PWR;		//关闭RK电源
    if (pm_acc == IACC_IDLE) {
        lcd_init(_false_, _true_);
    }
    pm_acc = IACC_OPEN;
    s_Hi3520_Monitor.start = false;

    /*2022-03-31 暂时打开*/
    //s_Hi3520_Monitor.start = true;//暂时设为开启状态,等平台能自行下发数据再恢复
    pub_io.b.dvr_open = true;
    s_Hi3520_Monitor.ack = false;

    logi("___ mcu open all power ___");
    s_Hi3520_Monitor.reset_tmr = jiffies;
    s_Hi3520_Monitor.reset = true;
    rouse_state = true; //已唤醒
    rouse_flag = false; //取消休眠
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
    if (_covern(p_debug) > 5) { //如果TR9未启动；则往下执行
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

            if (pub_io.b.key_s) //锁开状态
            {
                tr9_frame_pack2rk(tr9_cmd_5012, 0, 0);
                step = PWR_WAIT;
                break;
            }
            if (pm_acc == IACC_IDLE) {
                Open_Pwr_All();
                lcd_bl_ctrl(true); //打开一下背光
            } else if (pm_acc == IACC_CLOSE) {
                if (rouse_state == false) //如果ACC开RK还在休眠就打开重启RK电源
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
        } else //ACC关
        {
            if (pm_acc == IACC_IDLE) {
                /*
			if (coun == false)
			{
				delay = tick ;
				coun = true ;	
				logd("--1_请求关闭电源--");
				tr9_frame_pack2rk( tr9_cmd_1818, 0, 0);
			}
			
			if(s_dvr_time.time_flag)
			{
				if(0 == (tmr++))
				{
					RK_AgreedSleep = tick ;	
					logd("--1_RK 同意关闭电源--6秒后关闭");
				}				
			}
			else
				RK_AgreedSleep =  tick ;
					
			if ((_covern(delay) > 29)||(_covern(RK_AgreedSleep)>= 6)) 	//RK同意关闭电源，或者超时10秒
			{
				if( _covern(delay) >= 29){
					logd("--1_请求超时30秒,强制休眠--");
                }
				tmr = 0 ;
				s_dvr_time.time_flag = false ;
				dvr_power.cmd_ack = false;
				coun = false ;
			}
			*/
                if (pm_acc == IACC_IDLE) {
                    Close_Hi3520_Pwr();
                    bak_time_tick = tick - 1; //提前一秒
                }

                break;
            } else if (pm_acc == IACC_OPEN) {
                if (1 >= coum) {
                    if (rouse_flag == false) //false是未唤醒
                    {
                        if (coum == 0) {
#if 0 //22/04/12海格要求取消10分钟延时休眠	
					timer10 = tick ;	
					//	coum ++;			
					//}
					//else if(_covern(timer10) >= 600)//根据海格要求ACC关后10分钟再进入休眠
					//{
#endif
                            coum += 10;
                            step = PWR_WAIT;
                            _memset((u8 *)&Set_Awaken, 0x00, sizeof(Set_Awaken));
                            spi_flash_read((u8 *)&Set_Awaken, flash_Awaken_param_addr, sizeof(Set_Awaken)); //导出唤醒数据

                            logd("--state :%d --timer :%d --note %d--", Set_Awaken.state_rouse, Set_Awaken.timer_rouse, Set_Awaken.note_rouse);
                            pAwaken = (u8 *)&Set_Awaken;
                            if ((pAwaken[0] == 0xFF) && (pAwaken[1] == 0xFF) && (pAwaken[2] == 0xFF) && (pAwaken[3] == 0xFF)) {
                                _memset(pAwaken, 0x00, sizeof(Set_Awaken));
                            }

                            /*
						if(Set_Awaken.timer_rouse){
							logd("启用时间段%02x",Set_Awaken.time_Bucket +1);	
							logd("日期:0x%02X",Set_Awaken.timerRouse+1);
							for(iii =0 ;iii< 4; iii++)
							logd("时间段%d: --on :%02d--off :%02d--",iii+1,Set_Awaken.timing_Awaken[iii].OnTime,Set_Awaken.timing_Awaken[iii].OffTime);
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
#if 0 //22/04/12海格要求取消自动唤醒
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
				logd("自动唤醒");
				Close_Hi3520_Pwr();	
			}
				
		}
#endif
            if (pub_io.b.dvr_open == false) {
                if (pm_acc == IACC_IDLE) {
                    if (coun1 == false) {
                        delay = tick;
                        coun1 = true;
                        logi("--2_请求关闭电源--");
                        tr9_frame_pack2rk(tr9_cmd_1818, 0, 0);
                    }

                    if (s_dvr_time.time_flag) {
                        if (0 == (tmr++)) {
                            RK_AgreedSleep = tick;
                            logi("--2_RK 同意关闭电源--6秒后关闭");
                        }
                    } else {
                        RK_AgreedSleep = tick;
                    }

                    if ((_covern(delay) > 29) || (_covern(RK_AgreedSleep) >= 6)) { //RK同意关闭电源，或者超时10秒
                        if (_covern(delay) >= 29) {
                            loge("--2_请求超时30秒,强制休眠--");
                        }

                        Close_Hi3520_Pwr();
                        tmr = 0;
                        coun1 = false;
                        step = PWR_IDLE;
                        bak_time_tick = tick - 1; //提前一秒
                        dvr_power.cmd_ack = false;
                        s_dvr_time.time_flag = false;
                    }
                }

            } else {
                //step = PWR_WAIT;   		//暂时是全开状态
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
            if ((car_state.bit.acc == false) && s_dvr_time.flag) { //定时关闭
                tmr = (s_dvr_time.acc_off_time * 60) - 1;
                logi("ACC关闭[ %02x:%02x], 0x%X分钟后RK休眠", mix.time.min, mix.time.sec, s_dvr_time.acc_off_time);
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

        if ((_covern(delay) > tmr)) //进入睡眠
        {
#if 0
				if(1 == s_dvr_time.flag)		//定时关闭需要的应答
				{
					tr9_frame_pack2rk( tr9_cmd_1818, 0, 0);
					logi("--请求关闭电源--");
					s_dvr_time.flag = 0 ;
				}
#endif
            tr9_frame_pack2rk(tr9_cmd_1818, 0, 0);
            logi("--3_请求关闭电源--");
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
                logi("--3_RK 同意关闭电源--");
            }
        } else
            RK_AgreedSleep = tick;

        if ((_covern(delay) > 29) || (_covern(RK_AgreedSleep) >= 6)) //RK同意关闭电源，或者超时10秒
        {
            if (_covern(delay) >= 29) {
                loge("--3_请求超时30秒,强制休眠--");
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
            bak_time_tick = tick - 1; //提前一秒

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
            logi("Rk主动请求关闭电源");
        }
        ActivePowerOff = true;
    }
    if (ActivePowerOff && (_covern(RK_AgreedSleep) >= 6)) {
        Close_Hi3520_Pwr();
        step = PWR_IDLE;
        ActivePowerOff = false;
        bak_time_tick = tick - 1; //提前一秒
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
* 名称:    updata_link_mx ()
* 功能：刷新GPRS 的IP等链接信息
* 入口参数：无
* 出口参数：无
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
* 名称:    realtime_verify_server_cfg ()
* 功能：实时校验GPRS 上网变量是否正确
* 入口参数：无
* 出口参数：无
* 注意: 防错机制，防止全局变量被修改。如果出现全局变量被修改，MCU自动
复位，从SPI FLASH 中读取参数，若继续错误的话，将在MCU FLASH 中读取参数，若
参数依旧错误，将初始化参数。保证网络能够正常上线，接收短信修改参数
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
* 名称:    link_manage_task ()
* 功能：负责IP连接和链路检测
* 入口参数：无
* 出口参数：无
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
    case E_LINK_IDLE: //初始化装载IP ；包括升级IP等；明确监控链路的连接时间等。
        //IP 参数修改等；都要重新拨号
        //更新IP到 MX
        if (pub_io.b.dvr_open) {
            break;
        }
        updata_link_mx();

        if (lm.IP1_status == SERVER_NOMAL) {
            over_tick = 30 * 60;
        } else if (lm.IP1_status == SERVER_UPDATE) {
            over_tick = (16 * 60);
        } else { //重联
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

        //立即执行复位
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
名    称：calculation_sum_mile
功    能：计算车辆行驶的总里程(单位：米)
输入参数：无
输    出：车辆行驶的总里程
编写日期：2011-02-18
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
        if (car_state.bit.acc) //在定位和ACC开或处于运动状态的时候计算里程
        {
            /*********************************************************
            gps_base.void_speed 单位1000m/h
            temp_mil_value , mil_mod单位0.01m
            速度系数为1.03倍
            **********************************************************/
            if (gps_base.speed > 180) //如果速度超过280KM/H , 退出计算
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
                save_run_parameterer(); //寿命120*10万公里
            }
        }
    }
}

/*********************************************************
名    称：Ic_Relay_One_Control
功    能：继电器一控制函数
输入参数：flag 控制标志 1 断开继电器 0 闭合继电器
输    出：无
编写日期：2011-02-14
**********************************************************/
void Ic_Relay_One_Control(u8 flag) {
    if (flag == 0x01) {
        //关闭油路
        relay_enable();
    } else {
        //接通油路--0
        relay_disable();
        //relay_enable();
    }
}

/****************************************************************************
* 名称:    car_out ()
* 功能：油路控制
* 入口参数：无
* 出口参数：无
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
* 名称:    car_out_ctrl ()
* 功能：油路控制
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void car_out_ctrl(void) {
    static bool state = false;

    if (car.cycle == 0) //接通油路
    {
        Ic_Relay_One_Control(0);
        car.level = 0;
        return;
    }

    if (car.cycle > 0xf000) //断开油路
    {
        Ic_Relay_One_Control(1);
        car.level = 0;
        return;
    }

    if (state == false) {
        if (_pastn(car.phase) > car.time_off) //接通油路
        {
            state = true;
            Ic_Relay_One_Control(0);
            car.phase = jiffies;
        }
    } else if (_pastn(car.phase) > car.time_on) //关闭油路
    {
        state = false;
        Ic_Relay_One_Control(1);
        car.phase = jiffies;
        car.cycle--;
    }
}

void ctrl_relay_status(u8 stat) {
    if (stat > 0) { //智能关闭油路
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
* 名称:    init_car_out ()
* 功能：油路控制
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_car_out(void) {
    car_out(0, 1000, 0, LEVEL_LST);
}

/****************************************************************************
* 名称:    cut_oil_ctrl_task ()
* 功能：油路控制任务
* 入口参数：无
* 出口参数：无
****************************************************************************/
void cut_oil_ctrl_task(void) {
    static u32 step = 0;

    if (rcmd.b.cut_oil) {
        step = 130;
        car_out(1000, 0, 0xffff, LEVEL_JACKING); //断开油路

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
* 名称:    check_plus_speed ()
* 功能：计算脉冲速度
* 入口参数：无
* 出口参数：无
****************************************************************************/
//220828速度类型:脉冲速度，GPS速度，生产测试：不让GPS速度覆盖脉冲速度，要测试脉冲速度硬件
void check_plus_speed(void) {
    u16 cur_speed = 0;
    static u32 cur_plus;

    static u8 s_index = 0;

    static u32 delay_check = 0;
    static u32 plus_buf[10] = {0};
    static u8 t_index;

    //每秒计算一次
    if (_pastn(delay_check) < 50) {
        return;
    }
    delay_check = jiffies;
    //delay_check += 50;

    if (gps_base.speed > 1) { //tongren888	GPS速度
        cur_speed = gps_base.speed;
        speed_state.gps_s = gps_base.speed;
        speed_state.pulse_s = plus_speed;
    } else if (plus_speed > 1) { //脉冲车速大于0
        cur_speed = plus_speed;
        speed_state.gps_s = gps_base.speed;
        speed_state.pulse_s = cur_speed;
    } else if (wheel_speed > 1) { //CAN速度
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

    //要修改滤波算法
    cur_plus -= plus_buf[s_index];
    plus_buf[s_index] = speed_plus;
    cur_plus += speed_plus;
    s_index = (s_index + 1) % 10;
    speed_plus = 0;

    if (t_index < 10) {
        t_index++;
        return;
    }

    if (cur_plus >= 5) { //假??：???度为V，车辆特征系数为X，即每公里转X次，时间T小时内采集到M????冲，每转??周产??8??????
                         //  不一定正确：    V = M /(8*X*T）（千米/小时)
                         // plus_speed = (3600 * cur_plus) / factory_para.set_speed_plus;
                         // plus_speed = (360 * cur_plus) / factory_para.set_speed_plus;
                         //22-06-15 	一秒一次反应速度慢，换成50毫秒
        plus_speed = ((3600 * 2) * cur_plus) / factory_para.set_speed_plus;
    } else {
        plus_speed = 0;
    }
}
#endif

/****************************************************************************
* 名称:    public_manage_task ()
* 功能：公共功能管理任务
* 入口参数：无
* 出口参数：无
****************************************************************************/
void public_manage_task(void) {
    check_plus_speed();

    // custom_100ms_task();
}

//RST-RK:系统看护
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
/*定义：1-30分钟，对应的重启RK次数，多少次之后，接管4G模块*/
typedef struct {
    int seconds;
    s8 takeOver4gCnt;
} min_rstCnt_fosterCare_st;
//不重启、1分钟、2分钟、3分钟、5分钟、8分钟、12分钟、17分钟、23分钟、30分钟
static const min_rstCnt_fosterCare_st min_rstCnt_arr[] = {
    //{n, 不小于2}
    {0, 0},       //0//不接管，不复位，不处理
    {60 * 1, 20}, //1//1分钟
    {60 * 2, 10}, //2//2分钟
    {60 * 3, 7},  //3//3分钟
    {60 * 5, 4},  //4//5分钟
    {60 * 8, 3},  //5//8分钟
    {60 * 12, 2}, //6//12分钟
    {60 * 17, 2}, //7//17分钟
    {60 * 23, 2}, //8//23分钟
    {60 * 30, 2}, //9//30分钟
};
/*
通过系统看护变量，获取重启RK的时长（秒）
0-不重启
1-30分钟，换算成秒
其他-不重启
返回值，单位：秒
*/
static s8 getGapTimeS_Foster_care() {
    if (sys_cfg.foster_care >= sys_cfg_range.foster_care.min && sys_cfg.foster_care <= sys_cfg_range.foster_care.max) {
        if (sys_cfg.foster_care == 0) {
            return 0; //不接管，不复位，不处理
        } else {
            return min_rstCnt_arr[sys_cfg.foster_care].seconds;
        }
    } else {
        //RST-RK:系统看护
        if (!u1_m.cur.b.rcd_3c /*在不模式【串口读取19056等信息】*/) {
            loge("--foster_care val error--");
        }
        return 0; //不接管，不复位，不处理
    }
}
/*
获取接管4G的【复位次数+1】
根据不同的复位时间，制定不同的复位次下，接管4G时机
*/
static int getTakeOver4gCnt_Foster_care() {
    if (sys_cfg.foster_care >= sys_cfg_range.foster_care.min && sys_cfg.foster_care <= sys_cfg_range.foster_care.max) {
        if (sys_cfg.foster_care == 0) {
            return 0; //不接管，不复位，不处理
        } else {
            return min_rstCnt_arr[sys_cfg.foster_care].takeOver4gCnt;
        }
    } else {
        //RST-RK:系统看护
        loge("--foster_care val error--");

        return 0; //不接管，不复位，不处理
    }
}
#endif /*RK_RST_20220820*/

/**
 * @brief  ：监听 RK 的状态
 * @param  ：无
 * @return：无
 * @exception NSException 可能抛出的异常.
 * @author ：  
 * @date 2022-08-19 22:30:00 ：时间
 * @version 1.0 ：版本  
 * @property ：属性介绍
 * */
void Monitor_Hi3520_Status(void) {
    static u8 reset = 0;

    if (t_rst.rst) {
        return;
    }

    s8 takeOver4gCnt = getTakeOver4gCnt_Foster_care();
    if (takeOver4gCnt > 0) {
        if (Rk_Task_Manage.resetCount < takeOver4gCnt) { //次数
            int gapTimeS = getGapTimeS_Foster_care();
            if (gapTimeS > 0) {
                if ((car_state.bit.acc /*ACC存在，车在开*/) && (_covern(Rk_Task_Manage.resetTime) >= gapTimeS)) {
                    //s_Hi3520_Monitor.ack = false ;
                    //s_Hi3520_Monitor.reset = true ;
                    //s_Hi3520_Monitor.start = false ;
                    loge("--MCU 五分钟未收到RK心跳重启RK--");

                    Rk_Task_Manage.resetCount++;
                    goto RKreset;
                }
            }
        } else if (Rk_Task_Manage.resetCount == takeOver4gCnt /*RK有应答，会被清0*/) { //接管4G
            loge("RK无法启动MCU接管4G模块");

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

    if (_pastn(s_Hi3520_Monitor.reset_tmr) > 200 /*ms*/) { //判定时间
    RKreset:
        //RST-RK:RK复位功能
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
