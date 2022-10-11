/**
  ******************************************************************************
  * @file    mid_cfg_tr9.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   中间层参数配置
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

#if 1

/**
  ******************************************************************************
  * @file    mid_cfg_tr9.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   中间层参数配置
  ******************************************************************************
  * @attention
  胡工 
  ******************************************************************************
*/
#include "include_all.h"

#define F_D0_BASE (SECT_SET_IMPORT_ADDR * 0x1000 * 1ul) //设置重要参数，8103	相关
#define F_D2_BASE (SECT_SET_NORMAL_ADDR * 0x1000 * 1ul) //设置普通参数，8103	相关

#define F_D0_BASE_2 (SECT_SET_IMPORT_ADDR_2 * 0x1000 * 1ul) //设置重要参数，8103	相关
#define F_D2_BASE_2 (SECT_SET_NORMAL_ADDR_2 * 0x1000 * 1ul) //设置普通参数，8103	相关

#define F_D1_BASE (SECT_FACTORY_ADDR * 0x1000 * 1ul) //出厂设置或初次安装
#define F_D3_BASE (SECT_SYS_ADDR * 0x1000 * 1ul)     //系统运行过程中的参数

flash_reserve f_res; //保留

bool car_login = false;
bool car_login2 = false;
bool sms_phone_save = false;

static bool open_para_read_ok = true; //开机参数是否读完整
//static bool open_time_positon_ok = true;

/***************************************************************************
* 名称:    add_struct_crc ()
* 功能：添加结构体校验
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool add_struct_crc(u8 *p, u16 len) {
    u8 buf[520];
    u16 crc;
    u16 i;

    if (len < 4 || len > 518) {
        loge("crc err: add struct crc");
        return false;
    }

    _memset(buf, 0x00, 520);
    _memcpy(buf, p, len);

    buf[2] = 0xaa;
    buf[3] = 0x55;

    crc = 0;
    for (i = 2; i < len; i++) {
        crc += buf[i];
    }

    buf[0] = (crc & 0xff00) >> 8; //加入CRC
    buf[1] = (u8)crc;             //加入CRC

    _memcpy(p, buf, len);
    return true;
}

/****************************************************************************
* 名称:    verify_struct_crc ()
* 功能：校验结构体信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool verify_struct_crc(u8 *p, u16 len) {
    u16 crc;
    u16 crc1;
    u16 flag;
    u16 i;

    crc = 0;
    crc1 = 0;
    flag = 0;

    if (len < 4 || len > 520) {
        loge("crc err: verify_struct_crc");
        return false;
    }

    crc1 = p[0];
    crc1 <<= 8;
    crc1 += p[1];

    flag = p[2];
    flag <<= 8;
    flag += p[3];

    crc = 0;
    for (i = 2; i < len; i++) {
        crc += p[i];
    }

    if (crc1 == crc && flag == 0xaa55) {
        return true;
    } else {
        return false;
    }
}

/*********************************************************
名    称：check_parameter_form
功    能：检查参数列表
参    数：无
输    出：无
编写日期：默认参数
*********************************************************/
void check_parameter_form(void) {
    if ((link_cfg.hear_beat_time <= 30) || (link_cfg.hear_beat_time >= 300)) {
        link_cfg.hear_beat_time = 90;
    }
    if ((link_cfg.tcp_over_time == 0x00) || (link_cfg.tcp_over_time >= 3600)) {
        link_cfg.tcp_over_time = 100;
    }
    if ((link_cfg.tcp_resend_cnt == 0x00) || (link_cfg.tcp_resend_cnt >= 10)) {
        link_cfg.tcp_resend_cnt = 3; //3次重发
    }
    if ((link_cfg.udp_over_time == 0x00) || (link_cfg.udp_over_time >= 500)) {
        link_cfg.udp_over_time = 100;
    }
    if ((link_cfg.udp_resend_cnt == 0x00) || (link_cfg.udp_resend_cnt >= 10)) {
        link_cfg.udp_resend_cnt = 3;
    }
    if ((link_cfg.sms_over_time == 0x00) || (link_cfg.sms_over_time >= 500)) {
        link_cfg.sms_over_time = 100;
    }
    if ((link_cfg.sms_resend_cnt == 0x00) || (link_cfg.sms_resend_cnt >= 10)) {
        link_cfg.sms_resend_cnt = 3;
    }

    if (report_cfg.sample_mode >= 0x03) {
        report_cfg.sample_mode = 0;
    }
    if (report_cfg.acc_mode >= 0x03) {
        report_cfg.acc_mode = 0;
    }
    if ((report_cfg.no_dvr_time > 1800) || (report_cfg.no_dvr_time < 5)) {
        report_cfg.no_dvr_time = 30; //s单位
    }
    if ((report_cfg.sleep_time == 0xffffffff) || (report_cfg.sleep_time <= 30)) {
        report_cfg.sleep_time = 120; //300s 5分钟
    }
    if ((report_cfg.sos_time >= 1800) || (report_cfg.sos_time < 5)) {
        report_cfg.sos_time = 10;
    }
    if ((report_cfg.dft_time >= 1800) || (report_cfg.dft_time < 5)) {
        report_cfg.dft_time = 20;
    }

    if ((report_cfg.dft_dst >= 5000) || (report_cfg.dft_dst < 5)) {
        report_cfg.dft_dst = 200;
    }
    if ((report_cfg.no_dvr_dst >= 600) || (report_cfg.no_dvr_dst < 5)) {
        report_cfg.no_dvr_dst = 300;
    }
    if ((report_cfg.sleep_dst >= 600) || (report_cfg.sleep_dst < 5)) {
        report_cfg.sleep_dst = 500;
    }
    if ((report_cfg.sos_dst >= 5000) || (report_cfg.sos_dst < 5)) {
        report_cfg.sos_dst = 100;
    }
    if ((report_cfg.angle_dgree >= 600) || (report_cfg.angle_dgree < 5)) {
        report_cfg.angle_dgree = 60;
    }

    if (alarm_cfg.speed_over_km >= 250) {
        alarm_cfg.speed_over_km = 80; //30KM/H
    }
    if (alarm_cfg.speed_over_time >= 600) {
        alarm_cfg.speed_over_time = 10; //10s
    }

    if (sys_cfg.call_mode > 2) { //默认通话模式	  0 手柄接听 1 免提接听
        sys_cfg.call_mode = 0;   //默认免提接听
    }
    //220828速度类型://速度模式：传感器速度、GPS速度、CAN速度
    if (!(sys_cfg.speed_mode >= sys_cfg_range.speed_mode.min && sys_cfg.speed_mode <= sys_cfg_range.speed_mode.max)) {
        sys_cfg.speed_mode = sys_cfg_default.speed_mode;
    }
    //RST-RK:系统看护//修正参数
    if (!(sys_cfg.foster_care >= sys_cfg_range.foster_care.min && sys_cfg.foster_care <= sys_cfg_range.foster_care.max)) {
        sys_cfg.foster_care = sys_cfg_default.foster_care;
    }
    if ((sys_cfg.ulUart5Baud != 9600) && (sys_cfg.ulUart5Baud != 19200) && (sys_cfg.ulUart5Baud != 115200)) {
        sys_cfg.ulUart5Baud = 19200; //摄像头采用9600
    }
    if ((sys_cfg.highspeed0 < 20) || (sys_cfg.highspeed0 > 100)) {
        sys_cfg.highspeed0 = 60;
    }
    if ((sys_cfg.highspeed1 < 20) || (sys_cfg.highspeed1 > 100)) {
        sys_cfg.highspeed1 = 30;
    }
    if ((sys_cfg.lowspeed < 10) || (sys_cfg.lowspeed > 100)) {
        sys_cfg.lowspeed = 10;
    }
    if (!alarm_cfg.bypass_sw.bit.fatigue) {
        alarm_cfg.bypass_sw.bit.fatigue_near = false;
    }

    _memset((u8 *)&register_cfg.terminal_id, 0x00, 30);
    _memcpy((u8 *)&register_cfg.terminal_id, (u8 *)&server_cfg.terminal_id[5], 7); //2015-
}

#if (P_RCD == RCD_BASE)
/****************************************************************************
* 名称:    verify_flash_factory ()
* 功能：数据校验
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool verify_flash_factory(void) {
    bool ret_t = false;

    ret_t = verify_struct_crc((u8 *)&factory_para, sizeof(factory_para));
    if (!ret_t)
        return false;

    ret_t = verify_struct_crc((u8 *)&sig_cfg, sizeof(sig_cfg));
    if (!ret_t)
        return false;

    ret_t = verify_struct_crc((u8 *)&unique_id, sizeof(unique_id));
    if (!ret_t)
        return false;
    return true;
}

/*********************************************************
名    称：read_mfg_parameter
功    能：读取系统配置参数数据
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
void read_mfg_parameter(void) {
    u16 u16_addr;

    u16_addr = 0x00;

    spi_flash_read((u8 *)&factory_para, (u32)(F_D1_BASE), sizeof(factory_para));
    u16_addr += sizeof(factory_para);
    spi_flash_read((u8 *)&sig_cfg, (u32)(F_D1_BASE + u16_addr), sizeof(sig_cfg));
    u16_addr += sizeof(sig_cfg);
    spi_flash_read((u8 *)&unique_id, (u32)(F_D1_BASE + u16_addr), sizeof(unique_id));
    u16_addr += sizeof(unique_id);
}

/*********************************************************
名    称：write_mfg_parameter
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
void write_mfg_parameter(void) {
    u16 u16_addr;

    u16_addr = 0x00;
    spi_flash_erase(F_D1_BASE);

    add_struct_crc((u8 *)&factory_para, sizeof(factory_para));
    spi_flash_write((u8 *)&factory_para, (u32)(F_D1_BASE), sizeof(factory_para));
    u16_addr += sizeof(factory_para);

    add_struct_crc((u8 *)&sig_cfg, sizeof(sig_cfg));
    spi_flash_write((u8 *)&sig_cfg, (u32)(F_D1_BASE + u16_addr), sizeof(sig_cfg));
    u16_addr += sizeof(sig_cfg);

    add_struct_crc((u8 *)&unique_id, sizeof(unique_id));
    spi_flash_write((u8 *)&unique_id, (u32)(F_D1_BASE + u16_addr), sizeof(unique_id));
    u16_addr += sizeof(unique_id);
}

/****************************************************************************
* 名称:    init_mfg_parameter ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_mfg_parameter(void) {
    u8 sn_buf[8] = {"00004133"};
    _memset((u8 *)&factory_para, 0x00, sizeof(factory_para));
    factory_para.setting_time.year = 22;
    factory_para.setting_time.month = 3;
    factory_para.setting_time.date = 5;
    factory_para.setting_time.hour = 10;
    factory_para.setting_time.min = 10;
    factory_para.setting_time.sec = 15;
    factory_para.set_mileage = 0;
    factory_para.set_speed_plus = 3600; //8000

    factory_para.swit = 0x55;
    factory_para.print_type = 0x01; //默认2003格式
    factory_para.speed_type = 0x02;

    _memset((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
    _memcpy((u8 *)&sig_cfg.b0, "载货状态", 8);
    _memcpy((u8 *)&sig_cfg.b1, "车门锁定", 8);
    _memcpy((u8 *)&sig_cfg.b2, "油路断开", 8);
    _memcpy((u8 *)&sig_cfg.b3, "近光", 4);
    _memcpy((u8 *)&sig_cfg.b4, "远光", 4);
    _memcpy((u8 *)&sig_cfg.b5, "右转向", 6);
    _memcpy((u8 *)&sig_cfg.b6, "左转向", 6);
    _memcpy((u8 *)&sig_cfg.b7, "制动", 4);

    _memset((u8 *)&unique_id, 0x00, sizeof(unique_id));
    _memcpy((u8 *)&unique_id.ccc, "A098335", 7);
    _memcpy((u8 *)&unique_id.model, "0000000000Z/HT-9", 16);
    unique_id.time[0] = 21;
    unique_id.time[1] = 3;
    unique_id.time[2] = 1;
    unique_id.time[3] = 16;
    unique_id.time[4] = 10;
    unique_id.time[5] = 38;

    _str2tobcd(&unique_id.sn[0], &sn_buf[0]);
    _str2tobcd(&unique_id.sn[1], &sn_buf[2]);
    _str2tobcd(&unique_id.sn[2], &sn_buf[4]);
    _str2tobcd(&unique_id.sn[3], &sn_buf[6]);

    unique_id.res = 0;

    write_mfg_parameter();
}
#endif

/****************************************************************************
* 名称:    init_mid_parameter ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_mid_parameter(void) {
    _memset((u8 *)&mix, 0x00, sizeof(mix));

    _memset((u8 *)&car_state, 0x00, sizeof(car_state));

    _memset((u8 *)&car_alarm, 0x00, sizeof(car_alarm));

    _memset((u8 *)&t_rst, 0x00, sizeof(t_rst));

    car_state.bit.carriage = 0x01; //默认空载转态
}

/****************************************************************************
* 名称:    init_sys_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_sys_cfg(void) {
    _memset((u8 *)&sys_cfg, 0x00, sizeof(sys_cfg));
    sys_cfg.call_mode = 0;                             //默认免提接听
    sys_cfg.speed_mode = sys_cfg_default.speed_mode;   //220828速度类型://速度模式：传感器速度、GPS速度、CAN速度
    sys_cfg.foster_care = sys_cfg_default.foster_care; //RST-RK:系統看护//恢复出厂设置
    sys_cfg.call_set = 0x00;
    sys_cfg.ulUart5Baud = 19200; //摄像头采用9600
    sys_cfg.highspeed0 = 60;

    sys_cfg.highspeed1 = 30;
    sys_cfg.lowspeed = 10;
}

/****************************************************************************
* 名称:    init_driver_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_driver_cfg(void) {
    _memset((u8 *)&driver_cfg, 0x00, sizeof(driver_cfg));
    _memset((u8 *)&driver_cfg, 0, sizeof(driver_cfg));
    driver_cfg.name_len = 6;
    _memcpy((u8 *)&driver_cfg.name, "刘强强", 6);
    _memcpy((u8 *)&driver_cfg.license, "350122190001012224", 18);
    _memcpy((u8 *)&driver_cfg.qualification, "GD0000A080005", 14);
    driver_cfg.organization_len = 20;
    _memcpy((u8 *)&driver_cfg.organization, "深圳市同人伟业科技公司", 20);
    _memcpy((u8 *)&driver_cfg.driver_code, "123456", 6);
}

/****************************************************************************
* 名称:    init_register_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_register_cfg(void) {
    _memset((u8 *)&register_cfg, 0, sizeof(register_cfg));

    register_cfg.city = 0;
    register_cfg.province = 0;
#if defined(JTT_808_2019)
    //if(JTT_808_2019)

    _memcpy((u8 *)&register_cfg.mfg_id, "44010000000", 11); //广东广州
    _memcpy((u8 *)&register_cfg.terminal_type, "TR9", 9);

    _memcpy((u8 *)&register_cfg.terminal_id, "1108001", 7);
#else

    _memcpy((u8 *)&register_cfg.mfg_id, "71903", 5);
    _memcpy((u8 *)&register_cfg.terminal_type, "TR9", 9);

    _memcpy((u8 *)&register_cfg.terminal_id, "1108001", 7);

#endif

    register_cfg.color = 0x01;
    _memcpy((u8 *)&car_cfg.license, "粤B00008", 9);
    car_cfg.highspeed = 60;
    register_cfg.speed = 80;
    register_cfg.blank_car_weight = 500;
    init_s_authority_cfg();
}

#define MAX_RUN_SIZE 100 //结构体限制32个字节
#define RUN_ADDR (u32)(SECT_MILE_ADDR * 0x1000 * 1ul)
#define _run_addr(ptr) (RUN_ADDR + ptr * 32 * 1uL)
u16 save_run_pos = MAX_RUN_SIZE;

/****************************************************************************
* 名称:    load_run_parameteror ()
* 功能：在指定的SECTOR中寻找有效的指针
1.因flash的寿命只有10万次，需要循环存储
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool load_run_parameteror(void) {
    s16 low;
    s16 mid;
    s16 high;

    run_parameter ps;
    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = MAX_RUN_SIZE;

    while (low <= high) {
    search_start:
        if (++cnt > 10)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, _run_addr(mid), sizeof(ps));
        if (ps.flag == 0x55aa) //当前有效区域
        {
            //添加校验
            save_run_pos = mid;
            _memcpy((u8 *)&run, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) //未使用区域
        {
            high = mid - 1;
        } else //已使用过的区域
        {
            low = mid + 1;
        }
        goto search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    save_run_parameterer ()
* 功能：参数保存
* 入口参数：无
* 出口参数：无
****************************************************************************/
void save_run_parameterer(void) {
    u8 buf[64];

    if (save_run_pos >= MAX_RUN_SIZE - 1) {
        spi_flash_erase(RUN_ADDR);
        save_run_pos = 0;
    } else {
        _memset(buf, 0x00, 64);
        spi_flash_write(buf, _run_addr(save_run_pos), sizeof(run_parameter));
        save_run_pos++;
    }

    add_struct_crc((u8 *)&run, sizeof(run));
    spi_flash_write((u8 *)&run, _run_addr(save_run_pos), sizeof(run_parameter));
}

/****************************************************************************
* 名称:    init_run_parameterer ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_run_parameterer(void) {
    save_run_pos = MAX_RUN_SIZE;
    run.total_dist = 0;
    run.car.reg = 0;

    run.relay_switch = _RELAY_ON;

    run.voice_sw = 0;
    run.ip_to_tr9 = 0;
    save_run_parameterer();
}

/****************************************************************************
* 名称:    init_server_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_server_cfg(void) {
    _memset((u8 *)&server_cfg, 0x00, sizeof(server_cfg));
    _memcpy((u8 *)&server_cfg.terminal_id, "013100000007", 12); //013826558501

    _memcpy((u8 *)&server_cfg.apn, "CMNET", 6);

    _memcpy((u8 *)&server_cfg.main_ip, "113.012.081.069", 15); //
    server_cfg.select_tcp |= 0x01;
    server_cfg.main_port = 6608;
    _memcpy((u8 *)&server_cfg.bak_ip, "203.086.008.066", 15); //
    server_cfg.select_tcp |= 0x02;
    server_cfg.bak_port = 9999;

    _memcpy((u8 *)&server_cfg.ip3, "123.056.077.130", 15); //
    server_cfg.select_tcp |= 0x04;
    server_cfg.port3 = 6973;

    _memcpy((u8 *)&server_cfg.ota_ip, "203.086.008.005", 15); //
    server_cfg.select_tcp |= 0x04;
    server_cfg.ota_port = 9000;

    init_s_authority_cfg();
}

/****************************************************************************
* 名称:    init_link_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_link_cfg(void) {
    _memset((u8 *)&link_cfg, 0x00, sizeof(link_cfg));
#if (COMPANY == C_XINDALU)
    link_cfg.hear_beat_time = 59;
#else
    link_cfg.hear_beat_time = 60;
#endif
    link_cfg.tcp_over_time = 100;
    link_cfg.tcp_resend_cnt = 3; //3次重发
    link_cfg.udp_over_time = 100;
    link_cfg.udp_resend_cnt = 3;
    link_cfg.sms_over_time = 100;
    link_cfg.sms_resend_cnt = 3;
}

/****************************************************************************
* 名称:    init_report_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_report_cfg(void) {
    _memset((u8 *)&report_cfg, 0x00, sizeof(report_cfg));
    report_cfg.sample_mode = 0;
    report_cfg.acc_mode = 0;
    report_cfg.no_dvr_time = 21; //s单位

    report_cfg.sleep_time = 180; //300s 3分钟
    report_cfg.sos_time = 30;
    report_cfg.dft_time = 18;
    report_cfg.dft_dst = 200;
    report_cfg.no_dvr_dst = 300;
    report_cfg.sleep_dst = 500;
    report_cfg.sos_dst = 100;
    report_cfg.angle_dgree = 30;
}

/****************************************************************************
* 名称:    init_alarm_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_alarm_cfg(void) {
    _memset((u8 *)&alarm_cfg, 0x00, sizeof(alarm_cfg));

    alarm_cfg.bypass_sw.reg = 0xfe03be40;
    alarm_cfg.sms_sw.reg = 0;
    alarm_cfg.pic_sw.reg = 1;
    alarm_cfg.pic_save_sw.reg = 0;
    alarm_cfg.import_alarm_sw = 0;
    alarm_cfg.speed_over_km = 0;     //30KM/H
    alarm_cfg.speed_over_time = 10;  //10s
    alarm_cfg.fatigure_time = 14400; //300; //默认连续四小时就认为是超时
    alarm_cfg.day_time = 43200;

    //alarm_cfg.stop_rest_time = 60;
    alarm_cfg.stop_rest_time = 1200; //默认休息20分钟
    alarm_cfg.stoping_time = 0;      //172800;		//默认疲劳驾驶缓冲值为 30 分钟
}

/****************************************************************************
* 名称:    init_phone_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_phone_cfg(void) {
    _memset((u8 *)&phone_cfg, 0x00, sizeof(phone_cfg));
    _memcpy((u8 *)&phone_cfg.listen, "13827403559", 12);
    _memcpy((u8 *)&phone_cfg.reset, "13827403559", 12);
    _memcpy((u8 *)&phone_cfg.factory, "13827403559", 12);
    _memcpy((u8 *)&phone_cfg.user, "13827403559", 12);
    _memcpy((u8 *)&phone_cfg.center, "13827403559", 12);

    phone_cfg.ring_type = 0x03; //修改为非自动接听
    phone_cfg.each_time = 66;
    phone_cfg.month_time = 60000;
}

/****************************************************************************
* 名称:    init_us ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_us(u8 type) {
    _memset((u8 *)&us, 0x00, sizeof(us));

    us.u1.reg = 0;
    us.u1.b.rcd_3c = true;

    us.u4.reg = 0;
    us.u4.b.yd_oil = true;

    if (type == 0) {
        load_any_uart_remap(); //重新映射串口功能
    } else {
        uart_remap.delay_en = true;
        uart_remap.delay_jiff = jiffies;
    }
}

/****************************************************************************
* 名称:    init_rev_inf ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_rev_inf(void) {
    _memset((u8 *)&rev_inf, 0x00, sizeof(rev_inf));
}

/****************************************************************************
* 名称:    init_s_NightSpeed ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_dvr_set_time(void) {
#if 0    
    _memset((u8 *)&s_dvr_time, 0x00, sizeof(s_dvr_time));
	s_dvr_time.acc_off_time = 2;
    s_dvr_time.close_tick = 0;
    s_dvr_time.open_tick = 0;
    s_dvr_time.delay_time1 = 8;
    s_dvr_time.delay_time2 = 2;
#endif
}

/****************************************************************************
* 名称:    init_car_cfg ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_car_cfg(void) {
    _memset((u8 *)&car_cfg, 0x00, sizeof(car_cfg));
    _memcpy((u8 *)&car_cfg.vin, "1G1BL52P7TR115520", 17);
    _memcpy((u8 *)&car_cfg.license, "粤A18004", 9);
    _memcpy((u8 *)&car_cfg.type, "大型货车", 8);
}

/****************************************************************************
* 名称:    init_all_net_info ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_all_net_info(void) {
    init_s_authority_cfg();
    init_send_info();

    init_net_info();
    init_report_info();

    clr_gsm_send();
    clr_gprs_send();
}

/****************************************************************************
* 名称:    init_gprs_link ()
* 功能：初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_gprs_link(void) {
    lm.IP1_status = SERVER_NOMAL;

    lm.IP2_status = SERVER_COAL;
}

void init_gnss_cfg(void) {
    _memset((u8 *)&gnss_cfg, 0x00, sizeof(gnss_cfg));
    gnss_cfg.select.reg = 0x04;
    gnss_cfg.uart_baud = 0x02;
    gnss_cfg.out_freq = 0x01;
    gnss_cfg.proc_freq = 1;
    gnss_cfg.up_mode = 0x00;
    gnss_cfg.up_para = 30;
}

void init_s_authority_cfg(void) {
    car_login = false;
    _memset((u8 *)&authority_cfg, 0, sizeof(authority_cfg));
}

/****************************************************************************
* 名称:    reset_system_parameter ()
* 功能：恢复出厂默认
* 入口参数：无
* 出口参数：无
****************************************************************************/
void reset_system_parameter(u8 type) {
    if (type == R_F_ALL) {
        init_server_cfg();
        init_register_cfg();
#if (P_RCD == RCD_BASE)
        rcd_para_inf.en = true;
        rcd_para_inf.type = 0x83;
#endif
        init_car_cfg();
        ex_flash_init(true);
        rcd_manage_init();
        init_driver_cfg();
    }
#if 0    
    init_dvr_set_time();
#endif
    init_sys_cfg();
    init_gnss_cfg();
    init_link_cfg();
    init_report_cfg();
    init_alarm_cfg();
    init_phone_cfg();
    init_us(1);
    init_rev_inf();

    init_run_parameterer();
    init_all_net_info();
#if (P_CMA == CMA_BASE)
    init_pic_cfg();
    init_other_pic_cfg();
#endif
    //#if (P_RCD == RCD_BASE)
    //rcd_manage_init();
    //#endif
    lm.reset = true;
}

/*********************************************************
名    称：mcu_flash_read_buffer
功    能：
输入参数：
输出参数：
返    回：无
注意事项：
*********************************************************/
static void mcu_flash_read_buffer(u8 *buf, u32 addr, u16 len) {
    u16 i;
    u16 len_t;

    u16 buf_t[260];

    if (addr < MCU_NORMAL_CFG_ADDR || addr > BLParaAddress) {
        nop();
        return;
    }

    if (buf == NULL || len > 510 || (len % 2) != 0) {
        nop();
        return;
    }

    len_t = len / 2;

    for (i = 0; i < len_t; i++) {
        buf_t[i] = *(vu16 *)(addr);
        addr += 2;
    }

    _memcpy(buf, (u8 *)&buf_t, len);
}

/*********************************************************
名    称：mcu_flash_write_buffer
功    能：
输入参数：
输出参数：
返    回：无
注意事项：
*********************************************************/
static void mcu_flash_write_buffer(u8 *buf, u32 addr, u16 len) {
    u16 i;
    u16 buf_t[256];
    u16 len_t = 0;

    if ((buf == NULL)) {
        nop();
        return;
    }

    if (addr > BLParaAddress || addr < MCU_NORMAL_CFG_ADDR) {
        nop();
        return;
    }

    if ((len > 510) || (len % 2) != 0) {
        nop();
        return;
    }

    //bondary control
    if ((addr < MCU_IMPORT_CFG_ADDR) && (addr + len) > MCU_IMPORT_CFG_ADDR) {
        nop();
        return;
    } else if ((addr < BLParaAddress) && (addr + len) > BLParaAddress) {
        nop();
        return;
    }

    _memcpy((u8 *)&buf_t, buf, len);
    len_t = len / 2;

    for (i = 0; i < len_t; i++) {
        FLASH_ProgramHalfWord(addr, buf_t[i]);
        addr += 2;
    }
}

static bool write_import_judge_legal(void) {
    u8 i;

    //if (car_alarm.bit.cut_volt)
    //  return false;

    for (i = 0; i < 6; i++) {
        if ((server_cfg.terminal_id[i] < '0') || (server_cfg.terminal_id[i] > '9'))
            return false;
    }
    /*for (i = 0; i < 2; i++)
    {
        if ((car_cfg.license[i] < '0') || (car_cfg.license[i] == 0xff))
            return false;
    }
    for (i = 0; i < 6; i++)
    {
        if ((server_cfg.main_ip[i] < 0x20) || (server_cfg.main_ip[i] == 0xff))
            return false;
    }*/
    /*for (i = 0; i < 7; i++)
    {
        if ((register_cfg.terminal_id[i] < '0') || (register_cfg.terminal_id[i] > '9'))
            return false;
    }*/
    return true;
}

static bool mcu_flash_write_compare(const u32 op_addr) {
    u16 u16_addr;
    u16 struct_len;
    //    u8  mcu_buf[512];

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(op_addr);
    u16_addr = 0x00;

    struct_len = sizeof(f_res);
    add_struct_crc((u8 *)&f_res, struct_len);
    mcu_flash_write_buffer((u8 *)&f_res, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;

    struct_len = sizeof(server_cfg);
    add_struct_crc((u8 *)&server_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&server_cfg, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;

    struct_len = sizeof(register_cfg);
    add_struct_crc((u8 *)&register_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&register_cfg, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;

    struct_len = sizeof(car_cfg);
    add_struct_crc((u8 *)&car_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&car_cfg, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;
    FLASH_Lock(); //锁定FLASH编写擦除控制器

    return true;
}

/*********************************************************
名    称：mcu_flash_write_import_parameters
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
static void mcu_flash_write_import_parameters(void) {
    u8 buf[21] = {0x00};

    if (write_import_judge_legal() == false)
        return;

    //考虑到兼容问题，系统自动将ID前补零
    if ((server_cfg.terminal_id[0] < '0') || (server_cfg.terminal_id[0] > '9')) {
        _memset(buf, 0x00, 20);
        buf[0] = '0';
        _memcpy(&buf[1], (u8 *)&server_cfg.terminal_id, 11);
        _memset((u8 *)&server_cfg.terminal_id, 0x00, 21);
        _memcpy((u8 *)&server_cfg.terminal_id, buf, 12);
    }
    server_cfg.terminal_id[12] = '\0';

    //重要参数修改，需要重新注册和鉴权
    init_s_authority_cfg();
    flash_write_tmp_parameters();
    cli();

    mcu_flash_write_compare(MCU_IMPORT_CFG_ADDR);

    sei();
}

static bool spi_flash_write_compare(const u32 op_addr) {
    u16 u16_addr;
    u16 struct_len;
    //    u8  spi_buf[512];

    spi_flash_erase(op_addr);

    u16_addr = 0x00;
    struct_len = sizeof(f_res);
    add_struct_crc((u8 *)&f_res, struct_len);
    spi_flash_write((u8 *)&f_res, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;

    struct_len = sizeof(server_cfg);
    add_struct_crc((u8 *)&server_cfg, struct_len);
    spi_flash_write((u8 *)&server_cfg, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;

    struct_len = sizeof(register_cfg);
    add_struct_crc((u8 *)&register_cfg, struct_len);
    spi_flash_write((u8 *)&register_cfg, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;

    struct_len = sizeof(car_cfg);
    add_struct_crc((u8 *)&car_cfg, struct_len);
    spi_flash_write((u8 *)&car_cfg, (u32)(op_addr + u16_addr), struct_len);
    u16_addr += struct_len;

    return true;
}

/*********************************************************
名    称：spi1_flash_write_import_parameters
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
static void spi1_flash_write_import_parameters(void) {
    if (write_import_judge_legal() == false)
        return;

    //SPI flash 参数写
    spi_flash_write_compare(F_D0_BASE);
}

/*********************************************************
名    称：spi2_flash_write_import_parameters
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
static void spi2_flash_write_import_parameters(void) {
    if (write_import_judge_legal() == false)
        return;

    //SPI flash 参数写
    spi_flash_write_compare(F_D0_BASE_2);
}

///返回数据长度 =0 失败
bool mcu_two_time_read_import(u8 *buf, u16 star_a, u16 len) {
    bool ret1 = false;

    mcu_flash_read_buffer(buf, (u32)(MCU_IMPORT_CFG_ADDR + star_a), len);

    ret1 = verify_struct_crc(buf, len);

    return ret1;
}

/*********************************************************
名    称：flash_read_import_parameters
功    能：读取重要参数，8103相关
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
bool mcu_flash_read_import_parameters(void) {
    bool ret1 = true;
    bool rst = true;
    u16 u16_addr;
    u16 strcut_len;

    u8 mcu_buf[512];

    u16_addr = 0x00;

    strcut_len = sizeof(f_res);
    u16_addr += sizeof(f_res);

    //读取IP 参数
    strcut_len = sizeof(server_cfg);
    ret1 = mcu_two_time_read_import(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&server_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("mcu server_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(server_cfg);

    //读取注册信息
    strcut_len = sizeof(register_cfg);
    ret1 = mcu_two_time_read_import(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&register_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("mcu register_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(register_cfg);

    //读取车辆参数信息
    strcut_len = sizeof(car_cfg);
    ret1 = mcu_two_time_read_import(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&car_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("mcu car_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(car_cfg);

    return rst;
}

///返回数据长度 =0 失败
bool spi1_two_time_read_import(u8 *buf, u16 star_a, u16 len) {
    bool ret1 = false;

    spi_flash_read(buf, (u32)(F_D0_BASE + star_a), len);

    ret1 = verify_struct_crc(buf, len);

    return ret1;
}

/*********************************************************
名    称：spi1_flash_read_import_parameters
功    能：读取重要参数，8103相关
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
bool spi1_flash_read_import_parameters(void) {
    bool ret2 = false;
    bool rst = true;
    u16 u16_addr;
    u16 strcut_len;
    //	CAR_CONFIG_STRUCT car_cfg_1;
    u8 spi_buf[512];

    u16_addr = 0x00;

    strcut_len = sizeof(f_res);
    spi_flash_read((u8 *)&f_res, (u32)(F_D0_BASE), strcut_len);
    u16_addr += sizeof(f_res);

    //读取IP 参数
    strcut_len = sizeof(server_cfg);

    ret2 = spi1_two_time_read_import(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&server_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("spi1 server_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(server_cfg);

    //读取注册信息
    strcut_len = sizeof(register_cfg);
    ret2 = spi1_two_time_read_import(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&register_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("spi1 register_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(register_cfg);

    //读取车辆参数信息
    strcut_len = sizeof(car_cfg);
    ret2 = spi1_two_time_read_import(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&car_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("spi1 car_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(car_cfg);

    return rst;
}

///返回数据长度 =0 失败
bool spi2_two_time_read_import(u8 *buf, u16 star_a, u16 len) {
    bool ret1 = false;

    spi_flash_read(buf, (u32)(F_D0_BASE_2 + star_a), len);
    ret1 = verify_struct_crc(buf, len);

    return ret1;
}

/*********************************************************
名    称：spi2_flash_read_import_parameters
功    能：读取重要参数，8103相关
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
bool spi2_flash_read_import_parameters(void) {
    bool ret2 = false;
    u16 u16_addr;
    u16 strcut_len;
    bool rst = true;
    u8 spi_buf[512];

    u16_addr = 0x00;

    strcut_len = sizeof(f_res);
    spi_flash_read((u8 *)&f_res, (u32)(F_D0_BASE_2), strcut_len);
    u16_addr += sizeof(f_res);

    //读取IP 参数
    strcut_len = sizeof(server_cfg);
    ret2 = spi2_two_time_read_import(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&server_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("spi2 server_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(server_cfg);

    //读取注册信息
    strcut_len = sizeof(register_cfg);
    ret2 = spi2_two_time_read_import(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&register_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("spi2 register_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(register_cfg);

    //读取车辆参数信息
    strcut_len = sizeof(car_cfg);
    ret2 = spi2_two_time_read_import(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&car_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        rst = false;
        _printf_tmp("spi2 car_cfg parameters failed!\r\n");
    }
    u16_addr += sizeof(car_cfg);

    return rst;
}

static bool write_normal_judge_legal(void) {
    //u8 i;

    //判断电压是否断电
    //if (car_alarm.bit.cut_volt)
    //    return false;
    /*
    for (i = 0; i < 3; i++)
    {
        if ((driver_cfg.name[i] < 0x10) || (driver_cfg.name[i] == 0xff))
            return false;
    }
	*/

    return true;
}

/*********************************************************
名    称：mcu_flash_write_normal_parameters
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
static void mcu_flash_write_normal_parameters(void) {
    u16 u16_addr;
    u16 struct_len;

    if (write_normal_judge_legal() == false)
        return;

    //MCU flash 内部写
    FLASH_Unlock();
    cli();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(MCU_NORMAL_CFG_ADDR);
    u16_addr = 0x00;

    struct_len = sizeof(sys_cfg);
    add_struct_crc((u8 *)&sys_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&sys_cfg, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(sys_cfg);

    struct_len = sizeof(driver_cfg);
    add_struct_crc((u8 *)&driver_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&driver_cfg, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(driver_cfg);

    struct_len = sizeof(link_cfg);
    add_struct_crc((u8 *)&link_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&link_cfg, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(link_cfg);

    struct_len = sizeof(report_cfg);
    add_struct_crc((u8 *)&report_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&report_cfg, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(report_cfg);

    struct_len = sizeof(alarm_cfg);
    add_struct_crc((u8 *)&alarm_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&alarm_cfg, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(alarm_cfg);

    struct_len = sizeof(phone_cfg);
    add_struct_crc((u8 *)&phone_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&phone_cfg, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(phone_cfg);

    struct_len = sizeof(us);
    add_struct_crc((u8 *)&us, struct_len);
    mcu_flash_write_buffer((u8 *)&us, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(us);

    struct_len = sizeof(gnss_cfg);
    add_struct_crc((u8 *)&gnss_cfg, struct_len);
    mcu_flash_write_buffer((u8 *)&gnss_cfg, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(gnss_cfg);

    struct_len = sizeof(s_dvr_time);
    add_struct_crc((u8 *)&s_dvr_time, struct_len);
    mcu_flash_write_buffer((u8 *)&s_dvr_time, (u32)(MCU_NORMAL_CFG_ADDR + u16_addr), struct_len);
    u16_addr += sizeof(s_dvr_time);

    FLASH_Lock(); //锁定FLASH编写擦除控制器
    sei();
}

/*********************************************************
名    称：spi1_flash_write_normal_parameters
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
static void spi1_flash_write_normal_parameters(void) {
    u16 u16_addr;
    u16 struct_len;

    if (write_normal_judge_legal() == false)
        return;

    //SPI flash 芯片写
    spi_flash_erase(F_D2_BASE);
    u16_addr = 0x00;

    struct_len = sizeof(sys_cfg);
    add_struct_crc((u8 *)&sys_cfg, struct_len);
    spi_flash_write((u8 *)&sys_cfg, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(sys_cfg);

    struct_len = sizeof(driver_cfg);
    add_struct_crc((u8 *)&driver_cfg, struct_len);
    spi_flash_write((u8 *)&driver_cfg, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(driver_cfg);

    struct_len = sizeof(link_cfg);
    add_struct_crc((u8 *)&link_cfg, struct_len);
    spi_flash_write((u8 *)&link_cfg, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(link_cfg);

    struct_len = sizeof(report_cfg);
    add_struct_crc((u8 *)&report_cfg, struct_len);
    spi_flash_write((u8 *)&report_cfg, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(report_cfg);

    struct_len = sizeof(alarm_cfg);
    add_struct_crc((u8 *)&alarm_cfg, struct_len);
    spi_flash_write((u8 *)&alarm_cfg, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(alarm_cfg);

    struct_len = sizeof(phone_cfg);
    add_struct_crc((u8 *)&phone_cfg, struct_len);
    spi_flash_write((u8 *)&phone_cfg, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(phone_cfg);

    struct_len = sizeof(us);
    add_struct_crc((u8 *)&us, struct_len);
    spi_flash_write((u8 *)&us, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(us);

    struct_len = sizeof(gnss_cfg);
    add_struct_crc((u8 *)&gnss_cfg, struct_len);
    spi_flash_write((u8 *)&gnss_cfg, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(gnss_cfg);

    struct_len = sizeof(s_dvr_time);
    add_struct_crc((u8 *)&s_dvr_time, struct_len);
    spi_flash_write((u8 *)&s_dvr_time, (u32)(F_D2_BASE + u16_addr), struct_len);
    u16_addr += sizeof(s_dvr_time);
}

/*********************************************************
名    称：spi2_flash_write_normal_parameters
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
static void spi2_flash_write_normal_parameters(void) {
    u16 u16_addr;
    u16 struct_len;

    if (write_normal_judge_legal() == false)
        return;

    //SPI flash 芯片写
    spi_flash_erase(F_D2_BASE_2);
    u16_addr = 0x00;

    struct_len = sizeof(sys_cfg);
    add_struct_crc((u8 *)&sys_cfg, struct_len);
    spi_flash_write((u8 *)&sys_cfg, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(sys_cfg);

    struct_len = sizeof(driver_cfg);
    add_struct_crc((u8 *)&driver_cfg, struct_len);
    spi_flash_write((u8 *)&driver_cfg, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(driver_cfg);

    struct_len = sizeof(link_cfg);
    add_struct_crc((u8 *)&link_cfg, struct_len);
    spi_flash_write((u8 *)&link_cfg, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(link_cfg);

    struct_len = sizeof(report_cfg);
    add_struct_crc((u8 *)&report_cfg, struct_len);
    spi_flash_write((u8 *)&report_cfg, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(report_cfg);

    struct_len = sizeof(alarm_cfg);
    add_struct_crc((u8 *)&alarm_cfg, struct_len);
    spi_flash_write((u8 *)&alarm_cfg, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(alarm_cfg);

    struct_len = sizeof(phone_cfg);
    add_struct_crc((u8 *)&phone_cfg, struct_len);
    spi_flash_write((u8 *)&phone_cfg, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(phone_cfg);

    struct_len = sizeof(us);
    add_struct_crc((u8 *)&us, struct_len);
    spi_flash_write((u8 *)&us, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(us);

    struct_len = sizeof(gnss_cfg);
    add_struct_crc((u8 *)&gnss_cfg, struct_len);
    spi_flash_write((u8 *)&gnss_cfg, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(gnss_cfg);

    struct_len = sizeof(s_dvr_time);
    add_struct_crc((u8 *)&s_dvr_time, struct_len);
    spi_flash_write((u8 *)&s_dvr_time, (u32)(F_D2_BASE_2 + u16_addr), struct_len);
    u16_addr += sizeof(s_dvr_time);
}

///返回数据长度 =0 失败
bool mcu_two_time_read_normal(u8 *buf, u16 star_a, u16 len) {
    bool ret1 = false;

    mcu_flash_read_buffer(buf, (u32)(MCU_NORMAL_CFG_ADDR + star_a), len);
    ret1 = verify_struct_crc(buf, len);

    return ret1;
}

/*********************************************************
名    称：flash_read_normal_parameters
功    能：读取普通参数，8103 相关
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
bool mcu_flash_read_normal_parameters(void) {
    bool ret1 = false;
    u16 u16_addr;
    u16 strcut_len;
    u8 mcu_buf[512];

    u16_addr = 0x00;

    //读取系统参数
    strcut_len = sizeof(sys_cfg);

    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&sys_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(sys_cfg);

    //读取驾驶员信息
    strcut_len = sizeof(driver_cfg);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&driver_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(driver_cfg);

    //读取链路信息
    strcut_len = sizeof(link_cfg);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&link_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(link_cfg);

    //读取报告上报信息
    strcut_len = sizeof(report_cfg);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&report_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(report_cfg);

    //读取报警控制信息
    strcut_len = sizeof(alarm_cfg);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&alarm_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(alarm_cfg);

    //读取电话号码信息
    strcut_len = sizeof(phone_cfg);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&phone_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(phone_cfg);

    //读取串口配置信息
    strcut_len = sizeof(us);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&us, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(us);

    //读取GNSS 信息
    strcut_len = sizeof(gnss_cfg);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&gnss_cfg, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(gnss_cfg);

    //读取DVR时间设置
    strcut_len = sizeof(s_dvr_time);
    ret1 = mcu_two_time_read_normal(mcu_buf, u16_addr, strcut_len);
    if (ret1 == true) {
        _memcpy((u8 *)&s_dvr_time, (u8 *)&mcu_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(s_dvr_time);

    check_parameter_form();

    return true;
}

///返回数据长度 =0 失败
bool spi1_two_time_read_normal(u8 *buf, u16 star_a, u16 len) {
    bool ret1 = false;

    spi_flash_read(buf, (u32)(F_D2_BASE + star_a), len);
    ret1 = verify_struct_crc(buf, len);

    return ret1;
}

/*********************************************************
名    称：spi1_flash_read_normal_parameters
功    能：读取普通参数，8103 相关
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
bool spi1_flash_read_normal_parameters(void) {
    bool ret2 = false;
    u16 u16_addr;
    u16 strcut_len;
    u8 spi_buf[512];

    u16_addr = 0x00;

    //读取系统参数
    strcut_len = sizeof(sys_cfg);

    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&sys_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(sys_cfg);

    //读取驾驶员信息
    strcut_len = sizeof(driver_cfg);
    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&driver_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(driver_cfg);

    //读取链路信息
    strcut_len = sizeof(link_cfg);
    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&link_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(link_cfg);

    //读取报告上报信息
    strcut_len = sizeof(report_cfg);
    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&report_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(report_cfg);

    //读取报警控制信息
    strcut_len = sizeof(alarm_cfg);

    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&alarm_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(alarm_cfg);

    //读取电话号码信息
    strcut_len = sizeof(phone_cfg);
    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&phone_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(phone_cfg);

    //读取串口配置信息
    strcut_len = sizeof(us);

    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&us, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(us);

    //读取GNSS 信息
    strcut_len = sizeof(gnss_cfg);

    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&gnss_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(gnss_cfg);

    //读取夜间模式限速问题
    strcut_len = sizeof(s_dvr_time);

    ret2 = spi1_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&s_dvr_time, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(s_dvr_time);

    check_parameter_form();

    return true;
}

///返回数据长度 =0 失败
bool spi2_two_time_read_normal(u8 *buf, u16 star_a, u16 len) {
    bool ret1 = false;

    spi_flash_read(buf, (u32)(F_D2_BASE_2 + star_a), len);
    ret1 = verify_struct_crc(buf, len);

    return ret1;
}

/*********************************************************
名    称：spi2_flash_read_normal_parameters
功    能：读取普通参数，8103 相关
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
bool spi2_flash_read_normal_parameters(void) {
    bool ret2 = false;
    u16 u16_addr;
    u16 strcut_len;
    u8 spi_buf[512];

    u16_addr = 0x00;

    //读取系统参数
    strcut_len = sizeof(sys_cfg);
    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&sys_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(sys_cfg);

    //读取驾驶员信息
    strcut_len = sizeof(driver_cfg);
    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&driver_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(driver_cfg);

    //读取链路信息
    strcut_len = sizeof(link_cfg);

    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&link_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        init_link_cfg();
    }
    u16_addr += sizeof(link_cfg);

    //读取报告上报信息
    strcut_len = sizeof(report_cfg);

    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&report_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(report_cfg);

    //读取报警控制信息
    strcut_len = sizeof(alarm_cfg);
    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&alarm_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(alarm_cfg);

    //读取电话号码信息
    strcut_len = sizeof(phone_cfg);
    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&phone_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(phone_cfg);

    //读取串口配置信息
    strcut_len = sizeof(us);
    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&us, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(us);

    //读取GNSS 信息
    strcut_len = sizeof(gnss_cfg);

    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&gnss_cfg, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(gnss_cfg);

    //读取夜间模式限速问题
    strcut_len = sizeof(s_dvr_time);

    ret2 = spi2_two_time_read_normal(spi_buf, u16_addr, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&s_dvr_time, (u8 *)&spi_buf, strcut_len);
    } else {
        return false;
    }
    u16_addr += sizeof(s_dvr_time);

    check_parameter_form();

    return true;
}

/*********************************************************
名    称：Flash_Read_tmp_Parameters
功    能：读取系统运行时，临时保存的参数
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
void flash_read_tmp_parameters(void) {
    bool ret2 = false;

    u16 u16_addr;
    u16 strcut_len;

    u8 spi_buf[512];

    u16_addr = 0x00;

    //读取鉴权码
    strcut_len = sizeof(authority_cfg);
    spi_flash_read(spi_buf, (u32)(F_D3_BASE + u16_addr), strcut_len);
    //u16_addr += sizeof(authority_cfg);

    ret2 = verify_struct_crc(spi_buf, strcut_len);
    if (ret2 == true) {
        _memcpy((u8 *)&authority_cfg, (u8 *)&spi_buf, strcut_len);
    }
}

/*********************************************************
名    称：flash_write_tmp_parameters
功    能：保存系统配置参数数据到FLASH
参    数：无
输    出：无
编写日期：2011-01-22
*********************************************************/
void flash_write_tmp_parameters(void) {
    u16 u16_addr;
    u16 struct_len;

    //SPI flash 参数写
    spi_flash_erase(F_D3_BASE);
    u16_addr = 0x00;

    struct_len = sizeof(authority_cfg);
    add_struct_crc((u8 *)&authority_cfg, struct_len);
    spi_flash_write((u8 *)&authority_cfg, (u32)(F_D3_BASE + u16_addr), struct_len);
    u16_addr += struct_len;
}

/****************************************************************************
* 名称:    init_mid_cfg ()
* 功能：配置init_mid_cfg参数
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_mid_cfg(void) {
    bool ret = false;

    system_config();
    load_any_uart_remap();
    config_gps_port();
    //open_time_positon_ok = init_gps_info();
    init_gps_info();
    config_gsm_port();

    init_gprs_link();
    init_net_info();
    init_report_info();
    init_upgrade();
    init_rev_inf();

    init_mid_parameter();
    init_relay_gpio();

    INPUT1_HIGH1_INPUT();
    INPUT1_LOW1_INPUT();
    INPUT1_LOW2_INPUT();
    
    //init_led();
    sms_ip_if_ok();

    CLOSE_PHONE;

    if (run.voice_sw == 0x55)
        CLOSE_PHONE;

    init_car_out();

#if (P_RCD == RCD_BASE)
    read_mfg_parameter();
    ret = verify_flash_factory();
    if (!ret) {
        init_mfg_parameter();
    }
#endif

    key_state_scan_init();
    card_scan_init();
    menu_init();
    recorder_init();

    if (mcu_flash_read_import_parameters() == false) {
        if (spi1_flash_read_import_parameters() == false) {
            if (spi2_flash_read_import_parameters() == false)
                open_para_read_ok = false;
        }
    }

    if (mcu_flash_read_normal_parameters() == false) {
        if (spi1_flash_read_normal_parameters() == false) {
            spi2_flash_read_normal_parameters();
        }
    }
    check_parameter_form();

    flash_read_tmp_parameters();

    ret = load_run_parameteror();
    if (!ret) {
        init_run_parameterer();
    }

    ///屏蔽自动接听功能
    if (phone_cfg.each_time == 60) {
        phone_cfg.ring_type = 0x03;
    }

    if (register_cfg.blank_car_weight > 19000)
        register_cfg.blank_car_weight = 19000;

    if (factory_para.set_speed_plus < 300)
        factory_para.set_speed_plus = 3600;

    gnss_cfg.select.reg = 0x04;
    _memset((u8 *)&Can_param, 0x00, sizeof(CAN_param_t));
    _memset((u8 *)&Allot, 0x00, sizeof(Allot_data_t));
    _memset((u8 *)&Rk_Task_Manage, 0x00, sizeof(Rk_Task_Manage_t));
    _memset((u8 *)&tr9_6033_task, 0x00, sizeof(tr9_6033_task_t));
    _memset((u8 *)&auto_weight, 0x00, sizeof(AUTO_WEIGHT));
    _memset((u8 *)&font_lib_update, 0x00, sizeof(font_lib_update_struct));
    _memset((u8 *)&s_Hi3520_Monitor, 0x00, sizeof(HI3520_MONITTOR));

    init_self_test();

    tr9_comm_init();

    run.voice_sw = 0;

    if (register_cfg.speed < 50)
        register_cfg.speed = 50;
    if (register_cfg.speed > 99)
        register_cfg.speed = 100;

    report_cfg.heartbeat = 20;   //心跳
    report_cfg.sleep_time = 300; //默认不到两分钟//300=5分钟

#if 0

    _memcpy((u8 *)&server_cfg.ip3, "123.056.077.130", 15);		//
    server_cfg.select_tcp |= 0x04;
    server_cfg.port3 = 6973;

#endif
}

//判断
void sms_ip_if_ok(void) {
    //    u8 i;
    //    u8 tmp;
    sms_phone_save = false;
    /*
    if ((phone_cfg.sms[PHONELEN - 1] < 5) || (phone_cfg.sms[PHONELEN - 1] > 15))
        return;

    tmp = _strlen(phone_cfg.sms);
    if (tmp == 0)
    {
        sms_phone_save = false;
        return;
    }
    for (i = 0; i < phone_cfg.sms[PHONELEN - 1]; i++)
    {
        tmp = phone_cfg.sms[i];
        if ((tmp != '+')  && (tmp < '0' || tmp > '9')) // 短信号码无效，默认可以设置
        {
            sms_phone_save = false;
            return;
        }
    }
    sms_phone_save = true;
	*/
}

static void synchronize_flash_operation(void) {
    bool mcu_b = false;
    bool spi1_b = false;
    bool spi2_b = false;

    mcu_b = mcu_flash_read_import_parameters();

    spi1_b = spi1_flash_read_import_parameters();

    spi2_b = spi2_flash_read_import_parameters();

    if (!mcu_b && !spi1_b && !spi2_b)
        return;

    _printf_tmp("Para synchronize!\r\n");

    if (!mcu_b)
        mcu_flash_write_import_parameters();
    if (!spi1_b)
        spi1_flash_write_import_parameters();
    if (!spi2_b)
        spi2_flash_write_import_parameters();
}

///上电后过30秒后进行同步操作
///每天进行一次同步操作
void synchronize_flash_spi_para(void) {
    static bool sync = false;
    static u8 hourr = 0;

    if (tick < 15) {
        hourr = sys_time.hour;
        return;
    }

    if (open_para_read_ok == false)
        goto _read_para_synchronize;

    /*
	if ( open_time_positon_ok == false )
	{
		init_gps_info();
		open_time_positon_ok = true;
	}
	*/
    if (hourr == sys_time.hour)
        return;

    if (car_state.bit.acc)
        return;
    if (sync)
        return;

    hourr = sys_time.hour;

_read_para_synchronize:
    sync = true;
    synchronize_flash_operation();
    open_para_read_ok = true;
    //_memset( (u8*)&ztb_ctrl, 0, sizeof(ztb_ctrl) );

    server_cfg.dbip = 0x55;
}

void flash_write_import_parameters(void) {
    if (tick < 3)
        return;
    mcu_flash_write_import_parameters();
    spi1_flash_write_import_parameters();
    spi2_flash_write_import_parameters();
}

void flash_write_normal_parameters(void) {
    if (tick < 3)
        return;
    mcu_flash_write_normal_parameters();
    spi1_flash_write_normal_parameters();
    spi2_flash_write_normal_parameters();
}

#endif
