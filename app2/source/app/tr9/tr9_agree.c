#include "include_all.h"
#include "usbd_cdc_core.h"

TR9_VEHICLE_STATE tr9_car_status;
JT808_2018_PARA jt808_para;
KEY_LOCK_STATUS key_lcok;
DVR_STATUS dvr_status;
CANCEL_SPEED_CTRL cancel_speed_ctrl;
IC_TO_TR9 ic_tr9;
u16 uPolygon;
//u8 sHi3520Ver[20];
bool reset_hi3520_open = true; //两分钟机制开启设置
u8 tr9_www_status = 0;
s_GPSCOM s_Tr9Com;
u8 tr9_comm_buf[TR9_COM_BUF_LEN]; //接收BUF
u8 BBstatus;
u8 versions[20];
Set_Awaken_t Set_Awaken; //设置唤醒
bool rouse_flag;
Rk_Task_Manage_t Rk_Task_Manage;

//U_BD_RCD_STATUS		rcd_status;		//汽车行驶状态
//VEHICLE_ALARM_UNION Car_Alarm;		//警报
///GPS_BASE_INFOMATION GPS_Base ; 		//Gps基础信息
extern RCD_DATA_INPORT_STRUCT data_inport_para;

const u8 SUB_Insertion[] = "U盘挂载";
const u8 SUB_ack[] = "确认:导出数据";
const u8 SUB_export[] = "菜单:退出";

extern rk_6033_task_t rk_6033_task; //2022-03-17 add by hj

extern void init_rcd48_pointer(void);
extern void init_rcd360_pointer(void);
extern void init_rcd_speed_pointer(void);
extern void init_rcd_power_pointer(void);
extern void init_rcd_para_pointer(void);
extern void init_rcd_ot_pointer(void);
extern void init_rcd_driver_pointer(void);
extern void init_rcd_accident_pointer(void);

bool ExInport = false;

void tr9_comm_init(void) {
    _memset((u8 *)&s_Tr9Com, 0, sizeof(s_GPSCOM));
    s_Tr9Com.buf = tr9_comm_buf;
}

static void tr9_noromal_ack(u16 index, u8 *str, u16 s_len) {
    if (sys_bits1.bits.flg_rk_1717_first == 0) {
        sys_bits1.bits.flg_rk_1717_first = 1;
        selfChk_SetTic1st1717();
        promt_time(&mix.time, 0, log_level_fatal, __func__, __LINE__, "mix.tm", "1'st rk 1717");
    }

    //0x1717:心跳&状态
    //logd("1717 ack");
    s_Hi3520_Monitor.ack = true;
    Rk_Task_Manage.resetCount = 0;
    s_Hi3520_Monitor.reset_tmr = jiffies;
    Rk_Task_Manage.resetTime = tick;
}

//3.3	设置关闭DVR电源模式 1810
static void tr9_pwr_set(u16 index, u8 *str, u16 s_len) {
    //	u16 time = 0 ;
    //u8 dataSend[8] = {0} ;
    if (*str == 0x00) {
        s_dvr_time.acc_off_time &= (str[1]) << 8;
        s_dvr_time.acc_off_time |= (str[2]);

        if (0 == s_dvr_time.acc_off_time) {
            logi("不设置延时休眠");
            s_dvr_time.flag = 0;
        } else {
            s_dvr_time.flag = 1;
        }

    } else if (*str == 0x01) {
        str++;
        _memcpy((u8 *)&s_dvr_time.st, str, 2);        //起始时间
        _sw_endian((u8 *)&s_dvr_time.et, str + 2, 2); //结束时间
        flash_write_normal_parameters();              //保存数据到flashs
    }
    tr9_frame_pack2rk(tr9_cmd_1810, 0, 0);
}

#if (0)
static void tr9_pwr_off(u16 index, u8 *str, u16 s_len) {
    dvr_power.cmd_ack = true;
    //tr9_frame_pack2rk(0x1818,0,0);
}
#endif

//3.5	鍚屾剰鍏抽棴鐢垫簮
static void tr9_agree_pwr_close(u16 index, u8 *str, u16 s_len) {
    dvr_power.cmd_ack = true;
    //s_Hi3520_Monitor.start = false;

    s_dvr_time.flag = 0;
    s_dvr_time.time_flag = true; //同意关闭电源
                                 //logd("time:%x分%x秒,同意关闭电源",mix.time.min,mix.time.sec);
}

#if (0)
void tr9_1819(void) {
    u8 buf[10] = {0};
    tr9_agree_pwr_close(tr9_cmd_1819, buf, 0);
}
#endif

//2010 硬盘录像状态【SOC ->	MCU】
static void tr9_video_status(u16 index, u8 *str, u16 s_len) {
    _memcpy((u8 *)&dvr_status, str, 3);
}

//0x2011
static void tr9_ec20_ctrl(u16 index, u8 *str, u16 s_len) {
    if (*str == 0x00) {
        tr9_request_reset();
    } else if (*str == 0x01) {
        gsm_power_down();
    }
}

//3.14	0x2012设置服务器IP端口
static void tr9_set_server_para(u16 index, u8 *str, u16 s_len) {
    tr9_frame_pack2rk(index, 0, 0);
}

//3.15	设置设备ID
static void tr9_set_sim_id(u16 index, u8 *str, u16 s_len) {
    tr9_frame_pack2rk(index, 0, 0);
}

static void tr9_send_temperature(u16 index, u8 *str, u16 s_len) {
    loge("undef: static void tr9_send_temperature(u8 *str, u16 s_len)");
}

//TRZT190711-Q5 版本信息
static void tr9_ask_mcu_version(u16 index, u8 *str, u16 s_len) {
    u8 pack[36];
    u8 plen = 0;

    int len = strlen(TR9_SOFT_VERSION_APP);
    if (len > TR9_SOFT_VERSION_MAX_LEN) {
        loge("VERSION len err, is %d", len);
        return;
    }

    plen = _strcpy_len(pack, TR9_SOFT_VERSION_APP);
    tr9_frame_pack2rk(tr9_cmd_1415, pack, plen);
}

static void tr9_trans_t808_data(u16 index, u8 *str, u16 s_len) {
    loge("undef: static void tr9_trans_t808_data(u8 *str, u16 s_len)");
}

static void tr9_ask_gps_data(u16 index, u8 *str, u16 s_len) {
    loge("undef: static void tr9_ask_gps_data(u8 *str, u16 s_len)");
}

static void tr9_ask_water_no(u16 index, u8 *str, u16 s_len) {
    loge("undef: static void tr9_ask_water_no(u8 *str, u16 s_len)");
}

//
static void tr9_copy_jt808_to_mcu(void) {
    //bool change = false;
    IP_SERVER_CONFIG_STRUCT iss;
    //	u8 i = 0;
    u8 code[15];
    u8 ip[4];
    u16 len;
    u32 ippi;
    _memset((u8 *)&iss, 0, sizeof(IP_SERVER_CONFIG_STRUCT));
    if (jt808_para.NetProtocol == 1) //主端口
        iss.select_tcp &= 0xfc;
    else
        iss.select_tcp |= 0x03;
    if (jt808_para.SIMCardNum[0] == 0)
        jt808_para.SIMCardNum[0] = 0x30;

    len = _strlen((u8 *)jt808_para.SIMCardNum);

    _memcpy((u8 *)iss.terminal_id, (u8 *)jt808_para.SIMCardNum, len);

    _memcpy(ip, (u8 *)&jt808_para.DomainNameIP, 4);
    ippi = ip[0];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[0], 3, 0);
    code[3] = '.';
    ippi = ip[1];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[4], 3, 0);
    code[7] = '.';
    ippi = ip[2];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[8], 3, 0);
    code[11] = '.';
    ippi = ip[3];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[12], 3, 0);

    len = _memcpy_len((u8 *)iss.main_ip, code, 15);

    /*******************刷新小屏数据********************/
    _memset(set_para.m_ip, 0x00, sizeof(set_para.m_ip));
    set_para.m_ip[2] = '1';
    set_para.m_ip[3] = ',';
    len += _memcpy_len(&set_para.m_ip[4], code, len);
    len += 2;
    set_para.m_ip[len++] = (jt808_para.SeverPort1 / 1000) + 0x30;
    set_para.m_ip[len++] = ((jt808_para.SeverPort1 % 1000) / 100) + 0x30;
    set_para.m_ip[len++] = (((jt808_para.SeverPort1 % 1000) % 100) / 10) + 0x30;
    set_para.m_ip[len++] = (jt808_para.SeverPort1 % 10) + 0x30;
    set_para.m_ip[0] = len;
    /**********************************************************/
    _memcpy(ip, (u8 *)&jt808_para.BackDomainNameToMcu, 4);
    ippi = ip[0];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[0], 3, 0);
    code[3] = '.';
    ippi = ip[1];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[4], 3, 0);
    code[7] = '.';
    ippi = ip[2];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[8], 3, 0);
    code[11] = '.';
    ippi = ip[3];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[12], 3, 0);

    len = _memcpy_len((u8 *)iss.bak_ip, code, 15);

    /*******************刷新小屏数据************************/
    _memset(set_para.b_ip, 0x00, sizeof(set_para.b_ip));
    set_para.b_ip[2] = '1';
    set_para.b_ip[3] = ',';
    len += _memcpy_len(&set_para.b_ip[4], code, len);
    len += 2;
    set_para.b_ip[len++] = (jt808_para.SeverPort2 / 1000) + 0x30; //加0x30转为ASCII码
    set_para.b_ip[len++] = ((jt808_para.SeverPort2 % 1000) / 100) + 0x30;
    set_para.b_ip[len++] = (((jt808_para.SeverPort2 % 1000) % 100) / 10) + 0x30;
    set_para.b_ip[len++] = (jt808_para.SeverPort2 % 10) + 0x30;
    set_para.b_ip[0] = len;
    /************************************************************/

    _memcpy(ip, (u8 *)&jt808_para.ip3, 4);
    ippi = ip[0];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[0], 3, 0);
    code[3] = '.';
    ippi = ip[1];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[4], 3, 0);
    code[7] = '.';
    ippi = ip[2];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[8], 3, 0);
    code[11] = '.';
    ippi = ip[3];
    DriverCodeBcd((u32 *)&ippi, (u8 *)&code[12], 3, 0);

    _memcpy((u8 *)iss.ip3, code, 15);

    //iss.main_port =  jt808_para.SeverPort1;
    //iss.bak_port =   jt808_para.SeverPort2;
    _sw_endian((u8 *)&iss.main_port, (u8 *)&jt808_para.SeverPort1, 2);
    _sw_endian((u8 *)&iss.bak_port, (u8 *)&jt808_para.SeverPort2, 2);
    iss.port3 = jt808_para.port3;

    _memcpy((u8 *)car_cfg.license, (u8 *)&jt808_para.CarId, 8);

#if defined(JTT_808_2019)
    _memset((u8 *)register_cfg.terminal_id, 0x00, 30);
    //_memcpy((u8 *)register_cfg.terminal_id, (u8 *)&server_cfg.terminal_id[5], 7);  //2015-
    _memcpy((u8 *)register_cfg.terminal_id, (u8 *)&server_cfg.terminal_id[5], 7);
#else

    _memset((u8 *)register_cfg.terminal_id, 0x00, 7);
    //_memcpy((u8 *)register_cfg.terminal_id, (u8 *)&server_cfg.terminal_id[5], 7);  //2015-
    _memcpy((u8 *)register_cfg.terminal_id, (u8 *)&server_cfg.terminal_id[5], 7);
#endif

    register_cfg.color = jt808_para.CarIdColor;
    if ((_strncmp((u8 *)&server_cfg.terminal_id, (u8 *)&iss.terminal_id, 12) != 0) || (_strncmp((u8 *)&server_cfg.main_ip, (u8 *)&iss.main_ip, 14) != 0)) {
        server_cfg = iss;
        _memset((u8 *)&server_cfg.apn, 0x00, 24);
        _strcpy((u8 *)&server_cfg.apn, "CMNET");

        flash_write_import_parameters(); //
    } else
        server_cfg = iss;

    tr9_frame_pack2rk(0x4002, 0, 0);
}

//0x4001
static void tr9_set_jt808_2018_para(u16 index, u8 *str, u16 s_len) {
    //	u8 i = 0;
    //  REGISTER_CONFIG_STRUCT   register;
    //_memcpy( (u8*)&jt808_para, str, s_len );
    jt808_para.NetMode = *str++;     //上网模式
    jt808_para.NetProtocol = *str++; //网络协议

    _memcpy((u8 *)jt808_para.SeverDomainName, str, 64); //域名服务器
    str += 64;
    _memcpy((u8 *)jt808_para.SeverBackDomainName, str, 64); //备份域名服务器
    str += 64;

    _memcpy((u8 *)&jt808_para.DomainNameIP, str, 4); //域名IP地址
    str += 4;
    _memcpy((u8 *)&jt808_para.BackDomainNameToMcu, str, 4); //备份域名IP地址
    str += 4;
    _memcpy((u8 *)&jt808_para.SeverPort1, str, 2); //端口号
    str += 2;
    _memcpy((u8 *)&jt808_para.SeverPort2, str, 2); //备份端口号
    str += 2;

    _memcpy((u8 *)&register_cfg.province, str, 2); //省域ID
    str += 2;
    _memcpy((u8 *)&register_cfg.city, str, 2); //市县域ID
    str += 2;
#if defined(JTT_808_2019)

    _memcpy((u8 *)register_cfg.mfg_id, str, 11); //制造商编号
    str += 11;
    _memcpy((u8 *)register_cfg.terminal_type, str, 30); //终端型号
    str += 30;
#else
    _memcpy((u8 *)register_cfg.mfg_id, str, 5); //制造商编号
    str += 11;
    _memcpy((u8 *)register_cfg.terminal_type, str, 20); //终端型号
    str += 30;

#endif
    jt808_para.CarIdColor = *str++;           //车牌颜色
    _memcpy((u8 *)jt808_para.CarId, str, 12); //车牌号
    str += 12;

    _memcpy((u8 *)jt808_para.CarIdentity, str, 24); //车辆识别号VIN
    str += 24;

    _memcpy((u8 *)jt808_para.DrvLicenseNum, str, 24); //驾驶证号
    str += 24;

    _memcpy((u8 *)jt808_para.CarType, str, 24); //车辆类型
    str += 24;

    _memcpy((u8 *)jt808_para.SIMCardNum, str, 20); //SIM卡号
    str += 20;

    _sw_endian((u8 *)&jt808_para.Pulse, str, 2); //速度脉冲系数
    factory_para.set_speed_plus = jt808_para.Pulse;
    str += 2;
#if defined(JTT_808_2019)
    _memcpy((u8 *)register_cfg.terminal_id, str, 30); //终端ID
    str += 30;
#else
    _memcpy((u8 *)register_cfg.terminal_id, str, 7); //终端ID
    str += 30;
#endif
    _memset(mx.imei, 0x00, 20);
    _memcpy(mx.imei, str, 15); //新添加IMEI 码
    str += 15;
    _memset(versions, 0x00, 20);
    _memcpy(versions, str, 20);
    imei_flag = true;

    //_memcpy((u8 *)&jt808_para.ip3, str, 4);
    //str += 4;
    //_memcpy((u8 *)&jt808_para.port3, str, 2);

    _memset((u8 *)car_cfg.type, 0, 16);
    _memset((u8 *)car_cfg.vin, 0, 16);

    _memcpy(car_cfg.type, (uc8 *)jt808_para.CarType, 16);

    _memcpy(car_cfg.vin, (uc8 *)jt808_para.CarIdentity, 17);

    tr9_copy_jt808_to_mcu();
}

void RK_4001_trak(void) {
    u8 RK_buf[500] = {0};
    RK_buf[130] = 0x3A;
    RK_buf[131] = 0xf8;
    RK_buf[132] = 0x6A;
    RK_buf[133] = 0xA4;

    RK_buf[138] = 0x50;
    RK_buf[139] = 0x18;

    tr9_set_jt808_2018_para(tr9_cmd_4001, RK_buf, 0);
}

//3.36
//5001
static void tr9_ask_jt808_2018_para(u16 index, u8 *str, u16 s_len) {
    u8 pack[324] = {0};
    u8 *pf;
    u16 len = 0;

    pf = pack;
    pf[len++] = jt808_para.NetMode;
    pf[len++] = jt808_para.NetProtocol;

    len += _memcpy_len(pf + len, (u8 *)jt808_para.SeverDomainName, 64);     //域名
    len += _memcpy_len(pf + len, (u8 *)jt808_para.SeverBackDomainName, 64); //域名备份
    len += _memcpy_len(pf + len, (u8 *)&jt808_para.DomainNameIP, 4);        //域名IP
    len += _memcpy_len(pf + len, (u8 *)&jt808_para.BackDomainNameToMcu, 4); //域名IP备份
    len += _memcpy_len(pf + len, (u8 *)&jt808_para.SeverPort1, 2);          //域名IP地址的端口号
    len += _memcpy_len(pf + len, (u8 *)&jt808_para.SeverPort2, 2);          //备份域名IP地址的端口号
    len += _memcpy_len(pf + len, (u8 *)&register_cfg.province, 2);          //省域ID
    len += _memcpy_len(pf + len, (u8 *)&register_cfg.city, 2);              //市区ID
    len += _memcpy_len(pf + len, (u8 *)register_cfg.mfg_id, 11);            //制造商编号 11bit
    len += _memcpy_len(pf + len, (u8 *)register_cfg.terminal_type, 30);     //终端型号 22 bit
    len += _memcpy_len(pf + len, (u8 *)&register_cfg.color, 1);             //车牌颜色
    len += _memcpy_len(pf + len, (u8 *)car_cfg.license, 12);                //车牌号
    len += _memcpy_len(pf + len, (u8 *)car_cfg.vin, 18);                    //VIN码 24bit
    len += 6;
    len += _memcpy_len(pf + len, (u8 *)driver_cfg.qualification, 24); //驾驶证号
    len += _memcpy_len(pf + len, (u8 *)car_cfg.type, 16);             //车牌类型 24bit
    len += 8;
    len += _memcpy_len(pf + len, (u8 *)jt808_para.SIMCardNum, 20);    //SIM号
    len += _memcpy_len(pf + len, (u8 *)jt808_para.Pulse, 2);          //脉冲信号
    len += _memcpy_len(pf + len, (u8 *)register_cfg.terminal_id, 30); //终端ID
    tr9_frame_pack2rk(0x5002, pack, len);
}

//0x4040 MCU升级
static void tr9_update_sd_data(u16 index, u8 *str, u16 s_len) {
    u8 pack = 1;
    if (str[1] > str[0]) //头包数据
    {
        if (s_len > 30) {
            logd("len : %d 数据长度太长 !!!", s_len);
            logd("头包数据只需发总包数即可总长度保持在30个byte以内 ");
        }
        font_lib_update.start_m = false;
    }
    s_Hi3520_Monitor.reset_tmr = jiffies;
    if (t_rst.rst == true) {
        pack = 0;
        tr9_frame_pack2rk(tr9_cmd_4041, &pack, 1);
        return;
    }
    if ((s_len < 30) && (font_lib_update.start_m == false)) {
        s_Hi3520_Monitor.ack = true; //124.226.216.43   6001
        //tr9_frame_pack2rk( tr9_cmd_4041, &pack, 1);
        _memset((u8 *)&font_lib_update, 0, sizeof(font_lib_update));
        font_lib_update.total_pack = *str++;
        font_lib_update.write_pack = 1;
        font_lib_update.start_m = true;
        logd("upgrade start!!!");
        return;
    }
    update_tf_main(str, s_len); //SD卡升级
}

void qg_update(u16 con) {
    u8 buf[3] = {0};
    buf[0] = 160;
    buf[1] = con;
    tr9_update_sd_data(tr9_cmd_4040, buf, 2);
}

static void tr9_tts_data(u16 index, u8 *str, u16 s_len) {
    loge("undef: static void tr9_rece_limit_speed(u8 *str, u16 s_len)");
}

//DVR 发送锁车指令  0x5004  ;  CAN 锁车不发心跳；但是限速器需要继电器辅助进行锁车。
static void tr9_request_lock_car(u16 index, u8 *str, u16 s_len) {
    ctrl_relay_status(*str);
    //tr9_frame_pack2rk(index, 0, 0);
}

//5005 取消紧急报警
static void tr9_request_cancel_alarm(u16 index, u8 *str, u16 s_len) {
    alarm_cfg.sms_sw.bit.sos = false;
    car_alarm.bit.sos = false;
    logd("取消紧急报警");
}

#if (0)
//举升控制  0x5008    1限制举升     0解除举升
static void tr9_request_raise_ctrl(u16 index, u8 *str, u16 s_len) {
    u8 cmd;
    tr9_frame_pack2rk(index, 0, 0); //应答

    cmd = *str;

    if (1 == cmd) {
        relay_enable();
    } else {
        relay_disable();
    }

    cmd = relay_get_state();

    tr9_frame_pack2rk(tr9_cmd_6058, &cmd, 1); //发送业务数据
}
#endif

//IC-card://司机插拔卡信息【MCU->RK】//应答
static void tr9_ic_card_pull_out(u16 index, u8 *str, u16 s_len) {
    ic_tr9.icc = false;
}

//6.26	//3.49	司机插卡拔卡应答//RK主动向MCU查询卡信息。MCU收到消息后需要回复5013。
static void tr9_ic_card_5014(u16 index, u8 *str, u16 s_len) {
    ic_tr9.icc = true;
}

//0x5016  对讲喇叭（TTS喇叭）电源控制 0关闭,1开启
static void tr9_tele_power_ctrl(u16 index, u8 *str, u16 s_len) {
    if (str[0] == 0x00) {
        CLOSE_PHONE;
        run.voice_sw = 0x55;
    } else if (str[0] == 0x01) {
        OPEN_PHONE;
        run.voice_sw = 0x00;
    }
}

//0x5017  TTS喇叭电源控制 0关闭,1开启
static void tr9_tts_power_ctrl(u16 index, u8 *str, u16 s_len) {
    if (str[0] == 0x00) {
        CLOSE_PHONE;
        run.voice_sw = 0x55;
    } else if (str[0] == 0x01) {
        OPEN_PHONE;
        run.voice_sw = 0x00;
    }
}

//0x5018
static void tr9_send_jt808_status(u16 index, u8 *str, u16 s_len) { //部标上线状态
    u8 status = 0;

    if (str[0] == 0x00) {
        BBstatus = 0;
        gc.gprs = NET_CONNECT;
    } else {
        BBstatus = 1;
        gc.gprs = NET_ONLINE;
    }

    mix.csq = str[1];
    if ((str[2] == 0x00) || (str[2] == 0x03)) {
        run.area_in_out = _AREA_OUT;
        status = 0;
    } else {
        run.area_in_out = _AREA_IN; //重启是否会触发非法举斗报警
        status = 1;
    }
/***************************************************************************/
#if 1 //海格定制版
    if (str[3] == 0)
        run.illeage = 0; //无举升状态
    else
        run.illeage = 1; //非法举升
#endif
    str[4] = 0x00;
    /**************************************************************************/
    _sw_endian((u8 *)&uPolygon, str, 2);
    logd("#### TR9 STATUS=%x####", status);
}

//0x5019
static void tr9_set_sleep_rpt_time(u16 index, u8 *str, u16 s_len) {
    _sw_endian((u8 *)&report_cfg.sleep_time, str, 4);
    logd("休眠位置上报间隔:%ds", report_cfg.sleep_time);
    flash_write_normal_parameters();
}

//0x6001 设置限速值0~255，0：不限速，解除限速；其他：限制的具体时速，单位km/h
static void tr9_set_limit_speed(u16 index, u8 *str, u16 s_len) {
    //	u16 tmr;
    //u8  cmd;
    /************************海格定制版****************************/
    alarm_cfg.speed_over_km = *str; //设置高时速
    /*************************************************************/
    // cmd = *str++;
    tr9_frame_pack2rk(0x6001, 0, 0);
}

//
static void tr9_car_control_set(u16 index, u8 *str, u16 s_len) {
    register_cfg.ztb_gk = *str; //管控
    flash_write_import_parameters();
    tr9_frame_pack2rk(0x6002, 0, 0);
}

/************************************************************************
A、	内容：#set weight=xxx#  ；  设置空车重量，应答内容：weight=xxx
B、	内容：#set low speed=xx# ； 设置车辆最低限速，应答内容：low speed=xx
C、	内容：#set high speed=xx# ； 设置车辆最高限速，应答内容：high speed=xx
D、	内容：#speed=?# ；查询当前限速速度，转速比，载重，环保盖状态；应答内容：
speed=xx,rate=xxx,cover=0,carry=1,weight=xxx,blank=xxx.
*************************************************************************/
static void tr9_6003_task(u16 index, u8 *str, u16 s_len) {
    u8 baud[6] = {0};
    u8 msg[300] = {0};
    u8 i;
    u8 len;
    //	u8  *pf;
    //	u16 tmr;
    //	u8  buf[3];
    //	u16 speed;

#pragma region
    //logd("6003:<%s><%d>", str, s_len);

    char *tmp = malloc(s_len + 1);
    if (tmp == NULL) {
        loge("6003: ERR");
        tr9_frame_pack2rk(index, 0, 0);
        return;
    }

    memcpy(tmp, str, s_len);
    tmp[s_len] = '\0';
    //s_len = strlen(tmp);
#pragma endregion

#if (0)
    pf = ++str;
#else
    ++str;
#endif

    if (_strncmp(str, "#voice open#", 12) == 0) {
        run.voice_sw = 0x00;
        OPEN_PHONE;
        //save_run_parameterer();
    } else if (_strncmp(str, "#voice close#", 13) == 0) {
        CLOSE_PHONE;
        run.voice_sw = 0x55;
        //car_state.bit.acc = false;
        //save_run_parameterer();
    }
#if (0) //应该是同人以前的代码，6022我们没有使用。
    else if (_strncmp(str, "#CX CX#", 7) == 0) {
        len = get_ztc_status_to_center(msg);
        tr9_frame_pack2rk(0x6022, msg, len);
    }
#endif
    else if (_strncmp(str, "#TRSET:", 7) == 0) //#TRSET:ACC1<15>ACC0<180>#
    {
        str += 7;
        if (_strncmp(str, "ACC1<", 5) == 0) {
            str += 5;

            for (i = 0; i < s_len; i++) {
                if ((*str <= 0x39) && (*str >= 0x30))
                    baud[i] = *str++;
                else
                    break;
            }
            report_cfg.dft_time = atoi((char *)baud);
            str++;
            if (_strncmp(str, "ACC0<", 5) == 0) {
                str += 5;

                for (i = 0; i < s_len; i++) {
                    if ((*str <= 0x39) && (*str >= 0x30))
                        baud[i] = *str++;
                    else
                        break;
                }
                report_cfg.sleep_time = atoi((char *)baud);
            }
            flash_write_normal_parameters();
        }
    } else if (_strncmp(str, "#1008#", 9) == 0) {
        t_rst.rst = true;
        t_rst.del = false;
        t_rst.send = false;
        t_rst.jiff = jiffies;
    } else if (_strncmp((u8 *)tmp, "#dr:", 4) == 0) { //新版本更改
                                                      //logd("//新版本更改//文件信息-写-IC-card://#dr:");//0x6003
                                                      //logd("<%s>, len = %d", tmp, s_len);
#if (0)
        khd_set_card_message((u8 *)tmp, s_len);
#else
        khd_set_ic_card_message(tmp);
#endif
    } else {
        loge("6003: unknown...<%s>", tmp);
    }

    tr9_frame_pack2rk(index, msg, len);
    free(tmp);
}

static void tr9_update_font_lib(u16 index, u8 *str, u16 s_len) {
    //update_font_lib(str, s_len);
}

static void tr9_rece_set_ip(u16 index, u8 *str, u16 s_len) {
    //run.ip_to_tr9 = 0x00;
    run.ip_to_tr9 = 0x01; //转发配置上线短信
    save_run_parameterer();
}

//TR9收到限速器数据
static void tr9_rece_limit_speed(u16 index, u8 *str, u16 s_len) {
    loge("undef: static void tr9_rece_limit_speed(u8 *str, u16 s_len)");
}

u16 average_weight(u16 weight) {
    u16 oil_value = 0x00;
    static u32 oil_sum = 0x00;       //外部输入油量电阻值30次求和变量
    static u8 oil_first_flag = 0x00; //外部输入油量电阻首次采集标志 0x00 第一次输入

    if (weight >= 10000)
        weight = 10000;
    if (oil_first_flag == 0x00) //第一次采集数据
    {
        oil_sum = weight * 9;
        oil_first_flag = 0x01;
        oil_value = oil_value;
    } else {
        oil_sum = oil_sum + weight;
        oil_value = oil_sum / 10;
        oil_sum = oil_sum - oil_value;
    }
    return oil_value;
}

//TR9收到载重传感器数据
//$0.GD#    $0.+D1038#
static void tr9_rece_weight_data(u16 index, u8 *str, u16 s_len) {
    if (s_len > 10)
        s_len = 10;
}

//F060140009 4432303033333130313BF0
static void tr9_version_mess(u16 index, u8 *str, u16 s_len) {
#if (0)
    _memcpy(sHi3520Ver, str, 20);
    str += 20;
    sHi3520Ver[9] = 0;
    sHi3520Ver[10] = 0;
    sHi3520Ver[11] = 0;
    sHi3520Ver[12] = 0;
#endif
    tr9_frame_pack2rk(index, 0, 0);
}

//报价清单
static void tr9_set_close_time(u16 index, u8 *str, u16 s_len) {
    u8 open;

    open = *str;
    if (open == 0)
        reset_hi3520_open = false;
    else
        reset_hi3520_open = true;
    tr9_frame_pack2rk(0x6015, 0, 0);
}

//0x6016
static void tr9_ask_close_time(u16 index, u8 *str, u16 s_len) {
    u8 buff[2];

    if (reset_hi3520_open)
        buff[0] = 1;
    else
        buff[0] = 0;

    tr9_frame_pack2rk(0x6016, buff, 1);
}

//收到6017指令后；不再往海思发送数据
static void tr9_ask_restart(u16 index, u8 *str, u16 s_len) {
    u8 buff[2];

    s_Hi3520_Monitor.start = false;
    tr9_frame_pack2rk(0x6017, buff, 0);
}

//告知模块及上线状??
/*
0：上线????  1：未读到SIM??   2：无网络 (????欠费)   3：注册不?? （IP1??  4：鉴权不??   (IP1)   5：EC20通??故障??
*/
static void tr9_send_wangluo(u16 index, u8 *str, u16 s_len) {
    tr9_www_status = *str;
    self_test.uGsmU = tr9_www_status + 0x30;
    if (tr9_www_status == 0)
        gc.gprs = NET_ONLINE;
    /****************************************/
    tr9_frame_pack2rk(0x6020, 0, 0); //海格需要应答
    /****************************************/
}

//透传口功能
//收到0x6030 发送串口号+编号+内容
void tr9_comm_pass(u8 comm, u8 bh, u8 *str, u16 s_len) {
    u8 buff[100] = {0};
    u8 *pf;

    pf = buff;
#if 0
    *pf++ = comm;
    *pf++ = bh;
	s_len += 2 ;
#else //海格定制版   （编号+内容）
    *pf++ = bh;
    s_len += 1;
#endif
    _memcpy_len(pf, str, s_len);
    tr9_frame_pack2rk(0x6030, buff, s_len);
}

//0x6031	外接串口数据透传
static void tr9_uart_SeriaNet(u16 index, u8 *str, u16 s_len) {
    write_uart4(str, s_len);
    tr9_frame_pack2rk(0x6031, 0, 0);
}
/*
static u8 checking_code(u16 index, u8* buf, u16 len)
{
	int i;
	u8 checking = buf[0] ;
	for(i = 1 ; i < len  ; i++)
		checking ^= buf[i];	

	return checking ;
}
*/

/*通过串口启动读取行驶记录仪数据采集
  通过调试串口发送指令 *XCL#r,X1,  
                        X1 = 0x00\0x01\0x02\0x03\0x04\0x05\0x06\0x07\0x08\0x09\0x10\0x11\0x12\0x13\0x14\0x15
                        X1 = 0X16, 表示读取全部
                        
*/
#if 1
/**********************************************************************
//说明:在字符串中查找字符
//参数1:输入字符串
//参数2:查找字符值
//参数3:输入字符串长度
//返回:-1:没有查找到，>=0:起始位置
**********************************************************************/
int AsciiSearch(const u8 *pd, u8 d, u32 len) {
    int i;
    i = 0;
    while (len-- > 0) {
        if (*pd++ == d) {
            return i;
        }
        i++;
    }

    return -1;
}

/**********************************************************************
//说明:计算字符串长度
//参数1:输入指针
//返回:字符串长度
**********************************************************************/
u16 ht_strlen(const u8 *pStr) {
    u16 len;
    len = 0;
    while (*pStr++ != '\0') {
        len++;
    }
    return len;
}

u8 *ht_memcpy(u8 *pdest, const u8 *psrc, u32 count) {
    while (count-- > 0) {
        *pdest++ = *psrc++;
    }
    return pdest;
}
//计算字符串长度
int ht_strlen1(const u8 *str) {
    u8 len = 0;
    if (str == NULL) return 0;
    while ((*str++) != '\0')
        len++;
    return len;
}

/**********************************************************************
//说明:在指定内存中找出以t1开始t2结束包裹着(不含T1 T2)的段落
//参数1:输入指针
//参数2:输出指针
//参数3:输入数据长度
//参数4:起始字段t1
//参数5:结束字段t2
//返回:返回字段长度
**********************************************************************/
u16 ht_GetField(u8 *pBuf, u8 *pOut, u16 len, u8 t1, u8 t2) {
    int offset, offset2;
    offset = AsciiSearch(pBuf, t1, len);
    if (offset >= 0) {
        offset++;
        offset2 = AsciiSearch(pBuf + offset, t2, len - offset);
        if (offset2 > 0) {
            ht_memcpy(pOut, pBuf + offset, offset2);
            return offset2;
        }
    }
    return 0;
}

#endif
// 将字符串转换成整数
u32 char_to_int(u8 *name) {
    int i = 0, len, j;
    u32 a = 0, value;

    len = ht_strlen(name);

    while (name[i] != '\0') {
        value = name[i] - '0';
        for (j = 1; j < len - i; j++) {
            value *= 10;
        }
        a += value;
        i++;
    }
    return a;
}

// 字符串转换成浮点数
// 参数：str
// 输出：浮点数
float string_to_wfloat(u8 *str) {
    u8 i, j, k, negative = 0;
#define s_temp str
    double result = 0, result_1 = 0;
    for (i = 0; i < 15; i++) {
        j = str[i];
        if (j == 0 || ((j < '0' || j > '9') && (j != '.') && (j != '-'))) break;
    }
    k = j = i;              //数值的个数
    for (i = 0; i < j; i++) //查找小数点的位置，结束后小数点位于第i+1位
    {
        if (s_temp[i] == '.') break;
    }

    for (j = 0; j < i; j++) {
        if (s_temp[j] == '-') {
            negative = 1;
            continue;
        }
        result = result * 10 + (s_temp[j] - '0');
    }
    j++;   //加1后j=i+1，即小数点的位置
    i = j; //第一个小数的位置
    for (; j < k; j++) {
        if (s_temp[j] < '0' || s_temp[j] > '9') break; //非法字符，返回
        result_1 = result_1 * 10 + (s_temp[j] - '0');
    }
    for (j = 0; j < (k - i); j++) result_1 *= 0.1;
    result += result_1;

    if (negative) result = -result;
    return result;
}

u8 ASCII2HEX(const u8 *pd) {
    if ((*pd >= '0') && (*pd <= '9')) {
        return (*pd - '0');
    } else if ((*pd >= 'A') && (*pd <= 'F')) {
        return (*pd - 'A' + 10);
    } else if ((*pd >= 'a') && (*pd <= 'f')) {
        return (*pd - 'a' + 10);
    }
    return *pd;
}

u8 U32ToU8(u32 x, u8 p) {
    union uu {
        u32 i;
        u8 y[4];
    } uc;
    uc.i = x;
    return (uc.y[3 - p]);
}

/**********************************************************************
//说明:uint32 转换uint8字符串
//参数1:uint32 型待转换数据
//参数2:转换后uint8字符串指针
//返回:转换的长度
**********************************************************************/
u32 CopyU32ToU8(u32 x, u8 *pc) {
    u32 i;
    for (i = 0; i < 4; i++) {
        pc[i] = U32ToU8(x, i);
    }
    return i;
}

/**********************************************************************
//说明	:BCD to Dec
//参数1	:BCD字符
//返回	:Dec
**********************************************************************/
u8 BCDToDec(u8 pdata) {
    return ((pdata / 16) * 10 + pdata % 16);
}

void tr9_Auto_start(u8 *str, u16 s_len) {
    u8 cmdbuf[10] = {0};

    u8 ret;
    u8 cmd = 0;
    u8 *pt = NULL;

    logd("..... %s  .....", str);

    _memset(cmdbuf, 0, sizeof(cmdbuf));
    pt = str + 5;

    ret = ht_GetField(pt, cmdbuf, s_len, ',', ',');
    s_len -= (ret + 1);
    pt += (ret + 1);

    if (ht_strlen1(cmdbuf) == 0) {
        return;
    }

    cmd = char_to_int(cmdbuf);
    logd("..... cmd: buf_%02dH", cmd);
    if (cmd <= 0x15) { //读取单条指令
        u8 buf_00H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x00, 0xAA, 0x75, 0x00, 0x00, 0x00, 0x00, 0xDF, 0x5A, 0xF0};
        u8 buf_01H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x01, 0xAA, 0x75, 0x01, 0x00, 0x00, 0x00, 0xDE, 0x5B, 0xF0};
        u8 buf_02H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x02, 0xAA, 0x75, 0x02, 0x00, 0x00, 0x00, 0xDD, 0x58, 0xF0};
        u8 buf_03H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x03, 0xAA, 0x75, 0x03, 0x00, 0x00, 0x00, 0xDC, 0x59, 0xF0};
        u8 buf_04H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x04, 0xAA, 0x75, 0x04, 0x00, 0x00, 0x00, 0xDB, 0x5E, 0xF0};
        u8 buf_05H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x05, 0xAA, 0x75, 0x05, 0x00, 0x00, 0x00, 0xDA, 0x5F, 0xF0};
        u8 buf_06H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x06, 0xAA, 0x75, 0x06, 0x00, 0x00, 0x00, 0xD9, 0x5C, 0xF0};
        u8 buf_07H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x07, 0xAA, 0x75, 0x07, 0x00, 0x00, 0x00, 0xD8, 0x5D, 0xF0};
        u8 buf_08H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x08, 0xAA, 0x75, 0x08, 0x00, 0x00, 0x00, 0xD7, 0x7E, 0xF0};
        u8 buf_09H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x09, 0xAA, 0x75, 0x09, 0x00, 0x00, 0x00, 0xD6, 0x7F, 0xF0};
        u8 buf_10H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x10, 0xAA, 0x75, 0x10, 0x00, 0x00, 0x00, 0xCF, 0x08, 0xF0};
        u8 buf_11H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x11, 0xAA, 0x75, 0x11, 0x00, 0x00, 0x00, 0xCE, 0x0B, 0xF0};
        u8 buf_12H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x12, 0xAA, 0x75, 0x12, 0x00, 0x00, 0x00, 0xCD, 0x0A, 0xF0};
        u8 buf_13H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x13, 0xAA, 0x75, 0x13, 0x00, 0x00, 0x00, 0xCC, 0x0D, 0xF0};
        u8 buf_14H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x14, 0xAA, 0x75, 0x14, 0x00, 0x00, 0x00, 0xCB, 0x0C, 0xF0};
        u8 buf_15H[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x15, 0xAA, 0x75, 0x15, 0x00, 0x00, 0x00, 0xCA, 0x0F, 0xF0};

#if 1
        /*如果收到 6051信息表示 RK需要进行U盘下载行驶记录数据*/
        /*这里先把与行驶记录仪相关的信息初始化               */
        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));

        rk_6033_task.cmd = cmd;          //初始化 cmd = 0x00等待 “确认”键按下。
        rk_6033_task.u_read_flag = 0x10; // 读单挑
        rk_6033_task.Data_Direction = 1; //数据方向
        //Rk_Task_Manage.RK_nack_only_delay = 1;//设置无需RK应答标志
        Rk_Task_Manage.RK_time_rang_NG_flag = 1;
        Rk_Task_Manage.RK_read_all = 0;
        logd("..... in this way, cmd: %02dd", rk_6033_task.cmd);
        tr9_show = true;
        switch (rk_6033_task.cmd) {
        case 0x00:
            tr9_net_parse(buf_00H, 15, 0);
            break;
        case 0x01:
            tr9_net_parse(buf_01H, 15, 0);
            break;
        case 0x02:
            tr9_net_parse(buf_02H, 15, 0);
            break;
        case 0x03:
            tr9_net_parse(buf_03H, 15, 0);
            break;
        case 0x04:
            tr9_net_parse(buf_04H, 15, 0);
            break;
        case 0x05:
            tr9_net_parse(buf_05H, 15, 0);
            break;
        case 0x06:
            tr9_net_parse(buf_06H, 15, 0);
            break;
        case 0x07:
            tr9_net_parse(buf_07H, 15, 0);
            break;
        case 0x08:
            tr9_net_parse(buf_08H, 15, 0);
            break;
        case 0x09:
            tr9_net_parse(buf_09H, 15, 0);
            break;
        case 0x10:
            tr9_net_parse(buf_10H, 15, 0);
            break;
        case 0x11:
            tr9_net_parse(buf_11H, 15, 0);
            break;
        case 0x12:
            tr9_net_parse(buf_12H, 15, 0);
            break;
        case 0x13:
            tr9_net_parse(buf_13H, 15, 0);
            break;
        case 0x14:
            tr9_net_parse(buf_14H, 15, 0);
            break;
        case 0x15:
            tr9_net_parse(buf_15H, 15, 0);
            break;
        }

#endif
    } else if (cmd == 0x16) { //读全部指令.
#if 1
        /*如果收到 6051信息表示 RK需要进行U盘下载行驶记录数据*/
        /*这里先把与行驶记录仪相关的信息初始化               */
        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
        rk_6033_task.cmd = 0x00;         //初始化 cmd = 0x00等待 “确认”键按下。
        rk_6033_task.u_read_flag = 0x10; // 这里无需等待 跳过“确认”键按下
        rk_6033_task.Data_Direction = 1; //数据方向
        //Rk_Task_Manage.RK_nack_only_delay = 1;//设置无需RK应答标志
        Rk_Task_Manage.RK_time_rang_NG_flag = 1;
        Rk_Task_Manage.RK_read_all = 1;
        tr9_show = true;
#endif
    }

    else if (cmd == 0x17) { //清除形式记录仪数据.
#if 0
	         /*如果收到 6051信息表示 RK需要进行U盘下载行驶记录数据*/
	         /*这里先把与行驶记录仪相关的信息初始化               */
	         _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
	         rk_6033_task.cmd = 0x00;      //初始化 cmd = 0x00等待 “确认”键按下。
	         rk_6033_task.u_read_flag = 0x02; // 这里无需等待 跳过“确认”键按下
	         rk_6033_task.Data_Direction = 1; //数据方向
	         Rk_Task_Manage.RK_nack_only_delay = 1;//设置无需RK应答标志
#endif
    }

    else if (cmd == 0x20) { //读取单条指令
        u8 buf_08H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x08, 0xAA, 0x75, 0x08, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x06, 0x23, 0x59, 0x59, 0x00, 0x63, 0x98, 0x4C, 0xF0};
        tr9_net_parse(buf_08H, 29, 0);
    } else if (cmd == 0x21) {
        u8 buf_09H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x09, 0xAA, 0x75, 0x09, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x06, 0x23, 0x59, 0x59, 0x00, 0x63, 0x99, 0x4D, 0xF0};
        tr9_net_parse(buf_09H, 29, 0);
    } else if (cmd == 0x22) {
        u8 buf_10H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x10, 0xAA, 0x75, 0x10, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x06, 0x23, 0x59, 0x59, 0x00, 0x63, 0x80, 0x54, 0xF0};
        tr9_net_parse(buf_10H, 29, 0);
    } else if (cmd == 0x23) {
        u8 buf_11H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x11, 0xAA, 0x75, 0x11, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x05, 0x23, 0x59, 0x59, 0x00, 0x63, 0x82, 0x55, 0xF0};
        tr9_net_parse(buf_11H, 29, 0);
    } else if (cmd == 0x24) {
        u8 buf_12H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x12, 0xAA, 0x75, 0x12, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x05, 0x23, 0x59, 0x59, 0x00, 0x63, 0x81, 0x56, 0xF0};
        tr9_net_parse(buf_12H, 29, 0);
    } else if (cmd == 0x25) {
        u8 buf_13H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x13, 0xAA, 0x75, 0x13, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x05, 0x23, 0x59, 0x59, 0x00, 0x63, 0x80, 0x57, 0xF0};
        tr9_net_parse(buf_13H, 29, 0);
    } else if (cmd == 0x26) {
        u8 buf_14H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x14, 0xAA, 0x75, 0x14, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x05, 0x23, 0x59, 0x59, 0x00, 0x63, 0x87, 0x50, 0xF0};
        tr9_net_parse(buf_14H, 29, 0);
    } else if (cmd == 0x27) {
        u8 buf_15H[] = {0xF0, 0x60, 0x32, 0x00, 0x16, 0x15, 0xAA, 0x75, 0x15, 0x00, 0x0E, 0x00, 0x22, 0x03, 0x01, 0x00, 0x00, 0x00, 0x22, 0x05, 0x05, 0x23, 0x59, 0x59, 0x00, 0x63, 0x86, 0x51, 0xF0};
        tr9_net_parse(buf_15H, 29, 0);
    }

    else if (cmd == 0x82) { //读取VIN、行驶证号、和车辆类型
        u8 buf[20];
        //          u8 vin[32]={0};
        //          u8 driver_num[32]={0};
        //          u8 vehicle_num[32]={0};
        _memset(cmdbuf, 0, sizeof(cmdbuf));

        ret = ht_GetField(pt, cmdbuf, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        _memset(((u8 *)(&buf)), 0x00, (sizeof(buf)));
        _memcpy(buf, car_cfg.vin, sizeof(car_cfg.vin));
        logd("VIN码:%s", buf);

        _memset(((u8 *)(&buf)), 0x00, (sizeof(buf)));
        _memcpy(buf, car_cfg.license, sizeof(car_cfg.license));
        logd("行驶证号:%s", buf);

        _memset(((u8 *)(&buf)), 0x00, (sizeof(buf)));
        _memcpy(buf, car_cfg.type, sizeof(car_cfg.type));
        logd("行驶证号:%s", buf);

    } else if (cmd == 0x83) { //读取 初始安装时间
        logd("初次安装时间: %02x-%02x-%02x %02x:%02x:%02x", factory_para.setting_time.year, factory_para.setting_time.month, factory_para.setting_time.date, factory_para.setting_time.hour, factory_para.setting_time.min, factory_para.setting_time.sec);
    } else if (cmd == 0x84) {
#if 0
				  _memcpy_len((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
				  str += _memcpy_len((u8 *)&sig_cfg.b0, str, 10);
				  str += _memcpy_len((u8 *)&sig_cfg.b1, str, 10);
				  str += _memcpy_len((u8 *)&sig_cfg.b2, str, 10);
				  str += _memcpy_len((u8 *)&sig_cfg.b3, str, 10);
				  str += _memcpy_len((u8 *)&sig_cfg.b4, str, 10);
				  str += _memcpy_len((u8 *)&sig_cfg.b5, str, 10);
				  str += _memcpy_len((u8 *)&sig_cfg.b6, str, 10);
				  str += _memcpy_len((u8 *)&sig_cfg.b7, str, 10);
#endif
    }

    else if (cmd == 0xC2) { //设置记录仪时间
                            //_memcpy_len((u8 *)&sys_time.year, str, 6);
                            //*XCL#d,194,F06034000fc2AA75c2000600160405132800D752F0,
        u8 date[64];
        u8 buf_time[64];

        _memset(date, 0, sizeof(date));
        ret = ht_GetField(pt, date, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        if (ht_strlen1(date) == 0)
            return;
        s_len = ht_strlen1(date);
        data_ascii_to_hex(buf_time, date, s_len);
        tr9_net_parse(buf_time, s_len / 2, 0);

    } else if (cmd == 0xC3) { //设置脉冲信息
#if 0
 				  str += _memcpy_len((u8 *)&sys_time.year, str, 6);
				  factory_para.set_speed_plus = (*str++) << 8 ;
				  factory_para.set_speed_plus |= *str ;
#endif

        logd("脉冲系数:0x%x", factory_para.set_speed_plus);
    } else if (cmd == 0xC4) { //初始里程
        logd("初始里程:0x%x公里", factory_para.set_mileage);

#if 0
 				   factory_para.set_mileage |= (u8)_bcdtobin(*str++) << 24	;
				   factory_para.set_mileage |= (u8)_bcdtobin(*str++) << 16	;
				   factory_para.set_mileage |= (u8)_bcdtobin(*str++) << 8	;
				   factory_para.set_mileage |= (u8)_bcdtobin(*str)  ;
#endif

    }

    else if (cmd == 0xa7) { //导入08H 速度数据  //*XCL#d,168,8,0,220408220000,220406215959,60,113.123456,24.123456,30,
        u8 inport_start[16];

        _memset(inport_start, 0, sizeof(inport_start));

        ret = ht_GetField(pt, inport_start, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        if (ht_strlen1(inport_start)) {
            ExInport = true;
            logd("..... 外部导入开始, 请注意导入完成后取消.....");
        } else {
            ExInport = false;
            logd("..... 外部导入取消.....");
        }

    } else if (cmd == 0xa8) { //导入08H 速度数据  //*XCL#d,168,8,0,220408220000,220406215959,60,113.123456,24.123456,30,440112199309263745,
        u8 cmd_01[16];
        u8 inport[16];
        u8 start[16];
        u8 end[16];
        u8 speed[10];
        u8 lon[16];
        u8 lat[16];
        u8 altitude[10];
        //           u8 tmp[16];
        //           u32 dr = 0;
        u8 ic_driver[20];

        if (ExInport == false) {
            logd("..... 外部导入开关 指令167 未打开,不能导入.....");
        }

        _memset(cmd_01, 0, sizeof(cmd_01));
        _memset(inport, 0, sizeof(inport));
        _memset(start, 0, sizeof(start));
        _memset(end, 0, sizeof(end));
        _memset(speed, 0, sizeof(speed));
        _memset(lon, 0, sizeof(lon));
        _memset(lat, 0, sizeof(lat));
        _memset(altitude, 0, sizeof(altitude));
        _memset(ic_driver, 0, sizeof(ic_driver));

        _memset((u8 *)&data_inport_para, 0, sizeof(RCD_DATA_INPORT_STRUCT));

        ret = ht_GetField(pt, cmd_01, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, inport, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, start, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, end, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, speed, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, lon, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, lat, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, altitude, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        ret = ht_GetField(pt, ic_driver, s_len, ',', ',');
        s_len -= (ret + 1);
        pt += (ret + 1);

        if (ht_strlen1(cmd_01)) {
            data_inport_para.cmd = char_to_int(cmd_01);
            if (data_inport_para.cmd >= 10)
                data_inport_para.cmd = data_inport_para.cmd + 6;
        }

        if (ht_strlen1(inport)) {
            data_inport_para.inport = char_to_int(inport);
        }

        if (ht_strlen1(start)) {
            data_inport_para.start.year = (ASCII2HEX(start) << 4) | ASCII2HEX(start + 1);
            data_inport_para.start.month = (ASCII2HEX(start + 2) << 4) | ASCII2HEX(start + 3);
            data_inport_para.start.date = (ASCII2HEX(start + 4) << 4) | ASCII2HEX(start + 5);
            data_inport_para.start.hour = (ASCII2HEX(start + 6) << 4) | ASCII2HEX(start + 7);
            data_inport_para.start.min = (ASCII2HEX(start + 8) << 4) | ASCII2HEX(start + 9);
            data_inport_para.start.sec = (ASCII2HEX(start + 10) << 4) | ASCII2HEX(start + 11);

            /*转换成系统时间*/
            data_inport_para.start.year = BCDToDec(data_inport_para.start.year);
            data_inport_para.start.month = BCDToDec(data_inport_para.start.month);
            data_inport_para.start.date = BCDToDec(data_inport_para.start.date);
            data_inport_para.start.hour = BCDToDec(data_inport_para.start.hour);
            data_inport_para.start.min = BCDToDec(data_inport_para.start.min);
            data_inport_para.start.sec = BCDToDec(data_inport_para.start.sec);
        }

        if (ht_strlen1(end)) {
            data_inport_para.end.year = (ASCII2HEX(end) << 4) | ASCII2HEX(end + 1);
            data_inport_para.end.month = (ASCII2HEX(end + 2) << 4) | ASCII2HEX(end + 3);
            data_inport_para.end.date = (ASCII2HEX(end + 4) << 4) | ASCII2HEX(end + 5);
            data_inport_para.end.hour = (ASCII2HEX(end + 6) << 4) | ASCII2HEX(end + 7);
            data_inport_para.end.min = (ASCII2HEX(end + 8) << 4) | ASCII2HEX(end + 9);
            data_inport_para.end.sec = (ASCII2HEX(end + 10) << 4) | ASCII2HEX(end + 11);

            /*转换成系统时间*/
            data_inport_para.end.year = BCDToDec(data_inport_para.end.year);
            data_inport_para.end.month = BCDToDec(data_inport_para.end.month);
            data_inport_para.end.date = BCDToDec(data_inport_para.end.date);
            data_inport_para.end.hour = BCDToDec(data_inport_para.end.hour);
            data_inport_para.end.min = BCDToDec(data_inport_para.end.min);
            data_inport_para.end.sec = BCDToDec(data_inport_para.end.sec);
        }

        if (ht_strlen1(speed)) {
            data_inport_para.speed = char_to_int(speed);
        }

        if (ht_strlen1(lon)) { //单位:度.分 模式
            //data_inport_para.lon = (u32)(string_to_wfloat(lon) * 10000 *60);
            data_inport_para.lon = string_to_wfloat(lon);
        }

        if (ht_strlen1(lat)) { //单位:度.分 模式
            //data_inport_para.lat = (u32)(string_to_wfloat(lat) * 10000 * 60) ;
            data_inport_para.lat = string_to_wfloat(lat);
        }

        if (ht_strlen1(altitude)) {
            data_inport_para.altitude = char_to_int(altitude);
        } //ic_driver

        if (ht_strlen1(ic_driver)) {
            _memcpy_len(data_inport_para.ic_driver, ic_driver, ht_strlen1(ic_driver));
        } else {
            _memcpy_len(data_inport_para.ic_driver, "440112199309263745", 18);
        }

        //logd("..... 数据导入指令=0x%02X   导入方式=0x%02X  开始时间=%s  结束时间=%s  速度=%s  经度=%s  纬度=%s  高程=%s.....",data_inport_para.cmd,data_inport_para.inport,start,end,speed,lon,lat,altitude);

        //logd("..... 命令[0x%02X].....",data_inport_para.inport);
        if (data_inport_para.inport == 1) {
            logd("..... 记录追加方式[0x%02X].....", data_inport_para.inport);
        } else {
            logd("..... 记录清空方式[0x%02X].....", data_inport_para.inport);

            if (data_inport_para.cmd == 0x08) {
                init_rcd48_pointer();

                logd("..... 我在这里.....");

            } else if (data_inport_para.cmd == 0x09) {
                init_rcd360_pointer();
                logd("..... 我在这里....命令[0x%02X].", data_inport_para.cmd);
            }

            else if (data_inport_para.cmd == 0x10) {
                init_rcd_accident_pointer();
                logd("..... 我在这里....命令[0x%02X].", data_inport_para.cmd);
            }

            else if (data_inport_para.cmd == 0x11) {
                init_rcd_ot_pointer();
            } else if (data_inport_para.cmd == 0x12) {
                init_rcd_driver_pointer();
            } else if (data_inport_para.cmd == 0x13) {
                init_rcd_power_pointer();
            } else if (data_inport_para.cmd == 0x14) {
                init_rcd_para_pointer();
            } else if (data_inport_para.cmd == 0x15) {
                init_rcd_speed_pointer();
            }
        }

        logd("..... 命令[0x%X].....", data_inport_para.cmd);
        logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....", data_inport_para.start.year, data_inport_para.start.month, data_inport_para.start.date, data_inport_para.start.hour, data_inport_para.start.min, data_inport_para.start.sec);
        logd("..... 结束时间[%02x-%02x-%02x %02x:%02x:%02x].....", data_inport_para.end.year, data_inport_para.end.month, data_inport_para.end.date, data_inport_para.end.hour, data_inport_para.end.min, data_inport_para.end.sec);
        logd("..... 车速[%02d].....", data_inport_para.speed);
        logd("..... 经度[%d]....[0x%X].", data_inport_para.lon, data_inport_para.lon);
        logd("..... 纬度[%d]....[0x%X].", data_inport_para.lat, data_inport_para.lat);
        logd("..... 高程[%d].....", data_inport_para.altitude);
        logd("..... 驾驶证号[%s].....", data_inport_para.ic_driver);
        data_inport_para.flag = 1;

        /*经纬度转换成分*/
        data_inport_para.lon = (u32)(string_to_wfloat(lon) * 10000 * 60);
        data_inport_para.lat = (u32)(string_to_wfloat(lat) * 10000 * 60);

        /*计算记录方向*/
        if (data_inport_para.start.month >= data_inport_para.end.month) {
            if (data_inport_para.start.date >= data_inport_para.end.date) {
                if (data_inport_para.start.hour >= data_inport_para.end.hour) {
                    if (data_inport_para.start.min >= data_inport_para.end.min) {
                        data_inport_para.dr = 0;
                    } else {
                        data_inport_para.dr = 1;
                    }
                } else {
                    data_inport_para.dr = 1;
                }
            } else {
                data_inport_para.dr = 1;
            }
        } else {
            data_inport_para.dr = 1;
        }

        rcd_data_inport();

    }

    else if (cmd == 0xF4) { //init rcd
        for (;;)
            ;
    }

    else if (cmd == 0xF5) { //init rcd
        rcd_manage_init();
    }

    else if (cmd == 0xF6) { //6051
        s_Hi3520_Monitor.start = true;
        Rk_Task_Manage.RK_nack_only_delay = 1;
    }

    else if (cmd == 0xF7) { //6051
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x08, 0xAA, 0x75, 0x08, 0x00, 0x00, 0x00, 0xD7, 0x52, 0xF0};
        tr9_net_parse(buf, 15, 0);

        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xF8) { //初始里程  F06032000808AA7508000000D752F0
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x08, 0xAA, 0x75, 0x08, 0x00, 0x00, 0x00, 0xD7, 0x52, 0xF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xF9) { //初始里程  F06032000808AA7508000000D752F0
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x09, 0xAA, 0x75, 0x09, 0x00, 0x00, 0x00, 0xD6, 0x53, 0xF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xFA) { //初始里程  F06032000810AA7510000000CF4AF0
        u8 buf[] = {0XF0, 0X60, 0X32, 0X00, 0X08, 0X10, 0XAA, 0X75, 0X10, 0X00, 0X00, 0X00, 0XCF, 0X4A, 0XF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xFB) { //初始里程  F06032000810AA7510000000CF4AF0
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x11, 0xAA, 0x75, 0x11, 0x00, 0x00, 0x00, 0xCE, 0x4B, 0xF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xFC) { //初始里程  F06032000812AA7512000000CD48F0
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x12, 0xAA, 0x75, 0x12, 0x00, 0x00, 0x00, 0xCD, 0x48, 0xF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xFD) { //初始里程  F06032000813AA7513000000CC49F0
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x13, 0xAA, 0x75, 0x13, 0x00, 0x00, 0x00, 0xCC, 0x49, 0xF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xFE) { //初始里程  F06032000814AA7514000000CB4EF0
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x14, 0xAA, 0x75, 0x14, 0x00, 0x00, 0x00, 0xCB, 0x4E, 0xF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else if (cmd == 0xFF) { //初始里程  F06032000815AA7515000000CA4FF0
        u8 buf[] = {0xF0, 0x60, 0x32, 0x00, 0x08, 0x15, 0xAA, 0x75, 0x15, 0x00, 0x00, 0x00, 0xCA, 0x4F, 0xF0};
        tr9_net_parse(buf, 15, 0);
        //01 00 44 00 1F 00 55 7A 00 02 9A 00
        rk2mcu_hex(0x6032, buf, 15, -1, -1);
    }

    else { //不处理
        loge("undo");
    }
}

// 0x6032  行驶记录数据采集
static void tr9_Driving_record(u16 index, u8 *str, u16 s_len) {
    if (s_len > 12) {
        /*2012行驶记录仪协议--> 有开始时间和结束时间段的 数据读取*/
        /*注意:FROM_U4 是串口4*/
        //rcd_communicate_parse(FROM_U4,*str,str,s_len);
        rcd_communicate_parse(FROM_U4, *str, str + 7, s_len - 7); //2022-05-05
    } else {
        /*2012行驶记录仪协议--> 无时间段的数据读取, 这里默认读取所有保存在flash里面的数据*/

        /*数据区清0*/
        _memset(((u8 *)(&rk_6033_task)), 0x00, (sizeof(rk_6033_task)));

        /*等待确认件按下*/
        if (rk_6033_task.u_read_flag == 1) { // 注意 有U盘挂载的时候, 该标志为 1

            rk_6033_task.u_read_flag = 1;

            /*数据方向是 u盘*/
            rk_6033_task.Data_Direction = 1;

            /*从cmd = 0x00开始*/
            rk_6033_task.cmd = 0;

            Rk_Task_Manage.RK_time_rang_NG_flag = 1;
            Rk_Task_Manage.RK_read_all = 1;
        } else { //RK 单独读 行驶记录仪 cmd 数据
            rk_6033_task.u_read_flag = 0x10;
            rk_6033_task.Data_Direction = 0;
            rk_6033_task.cmd = str[0];
            Rk_Task_Manage.RK_time_rang_NG_flag = 1;
            Rk_Task_Manage.RK_read_all = 0;
        }

        Rk_Driving_Record_Up(rk_6033_task.cmd);
    }
}

// 0x6033  行驶记录数据上传
static void tr9_Export_state(u16 index, u8 *str, u16 s_len) {
    u8 *pa = str;
    rk_6033_task_t ack_6033;

    ack_6033.state = 0xff;
    ack_6033.idx = *pa++ << 8;
    ack_6033.idx |= *pa++;
    ack_6033.cmd = *pa++;
    ack_6033.state = *pa++;

#if 0
    logdNoNewLine("cmd=0x%02X  当前包=-%d-  状态= -%d- 接收长度 Len=%d  rk_6033_task.u_read_flag=-0x%02X-",ack_6033.cmd,\
             ack_6033.idx,\
             ack_6033.state,\
             s_len,rk_6033_task.u_read_flag);
    		{
    		   u16 k;
    		   for(k=0;k<s_len;k++)
    		    {
    		    	 logdNoTimeNoNewLine("%02X ",*(str+k));
    		    }
    		   logdNoTimeNoNewLine("\r\n");
    		}
#endif

    if (Rk_Task_Manage.RK_nack_only_delay == 1)
        return;

    if (rk_6033_task.u_read_flag == 0x10) {                                          //单独指令传输
        if (Rk_Task_Manage.RK_read_all == 0) {                                       //读单条
            if ((ack_6033.state == 0) && ((ack_6033.idx - 1) == rk_6033_task.idx)) { // 这个是 应答成功 分包序号也正确
                logd("收到0x6033应答_成功 [指令-0x%02X-] ,[第-%d-包], 结果: OK", ack_6033.cmd, ack_6033.idx);
                if (rk_6033_task.idx >= (rk_6033_task.packet - 1)) { //如果分包个数等于 总包个数 ,则启动下一个cmd的读取
                    {                                                //所有数据传输完成, 并收到正确应答
                        tr9_show = true;                             //在调试状态显示串口三的数据
                        Ack_Export = false;
                        Export_finish = 1; //导出结束
                        tr9_6033_task.state = 0;
                        Rk_Task_Manage.RK_1717_state = false;
                        Rk_Task_Manage.RK_6051_state = false;
                        Rk_Task_Manage.RK_record_print_log = false;
                        Rk_Task_Manage.RK_read_all = 0;
                        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                        return;
                    }

                } else { //进入下一个分包的处理
                    rk_6033_task.idx++;
                    rk_6033_task.u_send_flag = 0;
                    rk_6033_task.u8_delay = 0;
                }
            } else {
                logd("收到0x6033应答[指令:-0x%02X- ,第-%d-包, 结果: NG,需要重传", ack_6033.cmd, ack_6033.idx);
                /*在持续250ms 溢出后,重新传输*/
            }
        } else {                                                                     //读所有
            if ((ack_6033.state == 0) && ((ack_6033.idx - 1) == rk_6033_task.idx)) { // 这个是 应答成功 分包序号也正确
                logd("收到0x6033应答_成功:[指令:-0x%02X- ,第-%d-包, 结果: OK  u_read_flag=0x%02X", ack_6033.cmd, ack_6033.idx, rk_6033_task.u_read_flag);

                if (rk_6033_task.idx >= (rk_6033_task.packet - 1)) { //如果分包个数等于 总包个数 ,则启动下一个cmd的读取
                    u8 cmd_temp = 0;
                    if (rk_6033_task.cmd == 0x15) { //所有数据传输完成, 并收到正确应答
                        tr9_show = true;            //在调试状态显示串口三的数据
                        Ack_Export = false;
                        Export_finish = 1; //导出结束
                        tr9_6033_task.state = 0;
                        Rk_Task_Manage.RK_1717_state = false;
                        Rk_Task_Manage.RK_6051_state = false;
                        Rk_Task_Manage.RK_record_print_log = false;
                        Rk_Task_Manage.RK_read_all = 0;
                        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                        return;
                    }

                    if (rk_6033_task.cmd == 0x09) { //注意命令字0x09 之后是十六进制的0x10 , 而非十进制10
                        rk_6033_task.cmd = 0x10;
                    } else {
                        rk_6033_task.cmd++;
                    }

                    cmd_temp = rk_6033_task.cmd;
                    /*所有参数清0, 进入下一个cmd 处理*/
                    _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                    rk_6033_task.cmd = cmd_temp; //初始化 cmd = 0x00等待 “确认”键按下。
                    //rk_6033_task.u_read_flag = 0x10; // 读单挑
                    rk_6033_task.u_read_flag = 0x02;       // 读单挑
                    rk_6033_task.Data_Direction = 1;       //数据方向
                    Rk_Task_Manage.RK_nack_only_delay = 0; //设置无需RK应答标志
                    Rk_Task_Manage.RK_time_rang_NG_flag = 1;
                    Rk_Task_Manage.RK_read_all = 1;

                    rk_6033_task.packet = 0;
                    rk_6033_task.idx = 0;
                    rk_6033_task.head = 0;
                    rk_6033_task.tail = 0;
                    rk_6033_task.u_send_flag = 0;
                    rk_6033_task.u8_delay = 0;
                } else { //进入下一个分包的处理
                    rk_6033_task.idx++;
                    rk_6033_task.u_send_flag = 0;
                    rk_6033_task.u8_delay = 0;
                }
            } else {
                logd("收到0x6033应答[指令:-0x%02X- ,第-%d-包, 结果: NG,需要重传", ack_6033.cmd, ack_6033.idx);
                /*在持续250ms 溢出后,重新传输*/
            }
        }
    }
#if 1
    else if (rk_6033_task.u_read_flag == 0x02) {                                 //u盘传输
        if ((ack_6033.state == 0) && ((ack_6033.idx - 1) == rk_6033_task.idx)) { // 这个是 应答成功 分包序号也正确
            logd("收到0x6033应答_成功:[指令:-0x%02X- ,第-%d-包, 结果: OK  u_read_flag=0x%02X", ack_6033.cmd, ack_6033.idx, rk_6033_task.u_read_flag);

            //logd("....... i am here ........1");
            if (rk_6033_task.idx >= (rk_6033_task.packet - 1)) { //如果分包个数等于 总包个数 ,则启动下一个cmd的读取
                u8 cmd_temp = 0;
                //logd("....... i am here ........2");
                if (rk_6033_task.cmd == 0x15) { //所有数据传输完成, 并收到正确应答
                    tr9_show = true;            //在调试状态显示串口三的数据
                    Ack_Export = false;
                    Export_finish = 1; //导出结束
                    tr9_6033_task.state = 0;
                    Rk_Task_Manage.RK_1717_state = false;
                    Rk_Task_Manage.RK_6051_state = false;
                    Rk_Task_Manage.RK_record_print_log = false;
                    Rk_Task_Manage.RK_read_all = 0;
                    _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                    return;
                }

                if (rk_6033_task.cmd == 0x09) { //注意命令字0x09 之后是十六进制的0x10 , 而非十进制10
                    rk_6033_task.cmd = 0x10;
                } else {
                    rk_6033_task.cmd++;
                }

                cmd_temp = rk_6033_task.cmd;
                /*所有参数清0, 进入下一个cmd 处理*/
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                rk_6033_task.cmd = cmd_temp; //初始化 cmd = 0x00等待 “确认”键按下。
                //rk_6033_task.u_read_flag = 0x10; // 读单挑
                rk_6033_task.u_read_flag = 0x02;       // 读单条
                rk_6033_task.Data_Direction = 1;       //数据方向
                Rk_Task_Manage.RK_nack_only_delay = 0; //设置无需RK应答标志
                Rk_Task_Manage.RK_time_rang_NG_flag = 1;
                Rk_Task_Manage.RK_read_all = 1;

                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                rk_6033_task.u_send_flag = 0;
                rk_6033_task.u8_delay = 0;
            } else { //进入下一个分包的处理
                rk_6033_task.idx++;
                rk_6033_task.u_send_flag = 0;
                rk_6033_task.u8_delay = 0;
            }
        } else {
            logd("收到0x6033应答[指令:-0x%02X- ,第-%d-包, 结果: NG,需要重传", ack_6033.cmd, ack_6033.idx);
            /*在持续250ms 溢出后,重新传输*/
        }
    }
#endif
    else {
    }
}

void tr9_6032_task(void) {
    //static u8 coun_6032 = 0 ;
    //static u8 buf_6032[3] = {0};
    //if((coun_6032%10)==0)
    //{
    //	buf_6032[0] = coun_6032/10;
    //	tr9_Driving_record(buf_6032,1);
    //}
    //coun_6032++;
}

//6034  设置行驶记录参数
static void tr9_Set_Driving_record(u16 index, u8 *str, u16 s_len) {
    u8 cmd = 0;

    str++; //跳过RK命令字
    if ((*str != 0xAA) || (*(str + 1) != 0x75)) {
        logd("不符合19056命令帧格式");
        return;
    }
    str += 2;     //跳过19056表示位
    cmd = *str++; //记录命令字
    str += 3;     //跳过数据长度和保留字

    switch (cmd) {
    //str++;			//跳过命令字
    case 0x82: {
        str += _memcpy_len(car_cfg.vin, str, 16);
        str += _memcpy_len(car_cfg.license, str, 11);
        str += _memcpy_len(car_cfg.type, str, 11);
        rcd_para_inf.en = true;
        rcd_para_inf.type = cmd;

    } break; //设置车辆信息
    case 0x83: {
        str += _memcpy_len(&factory_para.setting_time.year, str, 6);
        rcd_para_inf.en = true;
        rcd_para_inf.type = cmd;
    } break; //设置初次安装时间
    case 0x84: {
        _memcpy_len((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
        str += _memcpy_len((u8 *)&sig_cfg.b0, str, 10);
        str += _memcpy_len((u8 *)&sig_cfg.b1, str, 10);
        str += _memcpy_len((u8 *)&sig_cfg.b2, str, 10);
        str += _memcpy_len((u8 *)&sig_cfg.b3, str, 10);
        str += _memcpy_len((u8 *)&sig_cfg.b4, str, 10);
        str += _memcpy_len((u8 *)&sig_cfg.b5, str, 10);
        str += _memcpy_len((u8 *)&sig_cfg.b6, str, 10);
        str += _memcpy_len((u8 *)&sig_cfg.b7, str, 10);
        rcd_para_inf.en = true;
        rcd_para_inf.type = cmd;
    } break; //设置状态量配置
    case 0xC2: {
        /*由于RK下发的是BCD码, 而MCU的系统时钟是16进制码,    */
        /*故注意这里需要把时间BCD码转换成16进制码才是系统时间*/
        u8 buf_time[6] = {0};

        _memcpy_len((u8 *)&buf_time, str, 6);
        _nbcdtobin(buf_time, 6);
        _memcpy_len((u8 *)&sys_time.year, buf_time, 6);
        //_memcpy_len((u8 *)&sys_time.year, str, 6);
        //logd("MCU 收到时间设置指令");
        rcd_para_inf.en = true;
        rcd_para_inf.type = cmd;
    } break; //设置记录仪时间
    case 0xC3: {
        str += _memcpy_len((u8 *)&sys_time.year, str, 6); //这里时间是不正确的,虽然这个时间不处理。
        factory_para.set_speed_plus = (*str++) << 8;
        factory_para.set_speed_plus |= *str;
        rcd_para_inf.en = true;
        rcd_para_inf.type = cmd;
    } break; //设置脉冲系数
    case 0xC4: {
        factory_para.set_mileage |= (u8)_bcdtobin(*str++) << 24;
        factory_para.set_mileage |= (u8)_bcdtobin(*str++) << 16;
        factory_para.set_mileage |= (u8)_bcdtobin(*str++) << 8;
        factory_para.set_mileage |= (u8)_bcdtobin(*str);

        run.total_dist = factory_para.set_mileage * 100;
        save_run_parameterer(); //寿命120*10万公里

        rcd_para_inf.en = true;
        rcd_para_inf.type = cmd;
    } break; //设置初始里程
    default:
        logd("不符合GB/T 19056的规范");
        break;
    }
    write_mfg_parameter();
    tr9_frame_pack2rk(0x6034, 0, 0); //应答数据帧
}

//0x6035	总线采集设置【SOC->MCU】
/***************************************************************************************
* 函数名称：tr9_6054_task
* 函数功能：设置CAN的上传时间
* 函数参数：
* 时     间:	2022.1.8
* 备     注：无
*************************************************************************************/
static void tr9_6054_task(u16 index, u8 *str, u16 s_len) {
    _memset((u8 *)&Can_param, 0x00, sizeof(CAN_param_t));

    Can_param.CAN1_UploadingTime &= (*str++ & 0xff) << 24;
    Can_param.CAN1_UploadingTime |= (*str++ & 0xff) << 16;
    Can_param.CAN1_UploadingTime |= (*str++ & 0xff) << 8;
    Can_param.CAN1_UploadingTime |= (*str++ & 0xff);

    Can_param.CAN2_UploadingTime &= (*str++ & 0xff) << 24;
    Can_param.CAN2_UploadingTime |= (*str++ & 0xff) << 16;
    Can_param.CAN2_UploadingTime |= (*str++ & 0xff) << 8;
    Can_param.CAN2_UploadingTime |= (*str & 0xff);

    //Can_SetFilterID_Model(1,1,3,0,0);		//无掩码模式

    logd("[RK -> MCU] ID:6054 CAN1上传时间:%02d ms  CAN2上传时间:%02d ms", Can_param.CAN1_UploadingTime, Can_param.CAN2_UploadingTime);
    tr9_frame_pack2rk(index, 0, 0); //应答数据帧
}

//0x6036	CAN总线ID单独采集设置【SOC->MCU】
static void tr9_6036_task(u16 index, u8 *str, u16 s_len) {
    u8 i;
    u8 *pa = str;

    Can_param_get.ID_Quantity = *pa++; //设置ID数量

    _memset((u8 *)tr9_Set_Can, 0x00, (sizeof(Set_Can_t) * 28));

    for (i = 0; i < Can_param_get.ID_Quantity; i++) {
        //采集时间
        tr9_Set_Can[i].time = (*pa++ & 0xff) << 24; //63~32bit	32bit
        tr9_Set_Can[i].time |= (*pa++ & 0xff) << 16;
        tr9_Set_Can[i].time |= (*pa++ & 0xff) << 8;
        tr9_Set_Can[i].time |= (*pa++ & 0xff);

        tr9_Set_Can[i].pipe = (bool)((*pa & 0x80) >> 7);      //通道号		31bit	0是通道一
        tr9_Set_Can[i].data_type = (bool)((*pa & 0x40) >> 6); //数据类型		30bit	0是标准帧
        tr9_Set_Can[i].get_way = (bool)((*pa & 0x20) >> 5);   //采集方式		29bit  	0是原始数据

        if (tr9_Set_Can[i].data_type == 0x00) //标准帧
        {
            tr9_Set_Can[i].ID_STD = (*pa++ & 0x1f) << 24;  //ID 		8bit
            tr9_Set_Can[i].ID_STD |= (*pa++ & 0xff) << 16; //ID 		8bit
            tr9_Set_Can[i].ID_STD |= (*pa++ & 0xff) << 8;  //ID 		8bit
            tr9_Set_Can[i].ID_STD |= (*pa++ & 0xff);       //ID 		8bit

            if (0 == tr9_Set_Can[i].pipe)
                Can_SetFilterID_Model(1, i, 1, tr9_Set_Can[i].ID_STD, tr9_Set_Can[i].ID_STD); //通道一标准帧
            else
                Can_SetFilterID_Model(2, i, 1, tr9_Set_Can[i].ID_STD, tr9_Set_Can[i].ID_STD); //通道二标准帧
        } else                                                                                //扩展帧
        {
            tr9_Set_Can[i].ID_EXT = (*pa++ & 0x1f) << 24;  //ID 		8bit
            tr9_Set_Can[i].ID_EXT |= (*pa++ & 0xff) << 16; //ID 		8bit
            tr9_Set_Can[i].ID_EXT |= (*pa++ & 0xff) << 8;  //ID 		8bit
            tr9_Set_Can[i].ID_EXT |= (*pa++ & 0xff);       //ID 		8bit

            if (0 == tr9_Set_Can[i].pipe)
                Can_SetFilterID_Model(1, i, 2, tr9_Set_Can[i].ID_EXT, tr9_Set_Can[i].ID_EXT); //通道一扩展帧
            else
                Can_SetFilterID_Model(2, i, 2, tr9_Set_Can[i].ID_EXT, tr9_Set_Can[i].ID_EXT); //通道二扩展帧
        }
    }

    tr9_frame_pack2rk(0x6036, 0, 0); //应答数据
}

#if 0 // 设置6036对单个ID的采集
void tr9_6036_set(void)
{
	u8 len = 0 , i =0  ;
	u8 buf[200] = { 0 } ;
	u32 CAN_time = 200;
	CAN_ID_CONFIG_T ID_type[14] ;
	
	buf[len++] =  14  ;	
	for(i = 0 ; i < 14 ; i++ )
	{
	
		ID_type[i].bit.ID 		= 	0x18fef100 + i ;
		ID_type[i].bit.type 	=	1 ;
		ID_type[i].bit.mode 	=	0 ;
		ID_type[i].bit.chnnal	=   0 ;

		
		buf[len++] = (u8 )((CAN_time  & 0xff000000)>>24) ;
		buf[len++] = (u8 )((CAN_time  & 0xff0000)>>16) ;
		buf[len++] = (u8 )((CAN_time  & 0xff00)>>8 );
		buf[len++] = (u8 )((CAN_time  & 0xff) );
		
		buf[len++] = (u8 )((ID_type[i].reg  & 0xff000000)>>24 );
		buf[len++] = (u8 )((ID_type[i].reg  & 0xff0000)>>16 );
		buf[len++] = (u8 )((ID_type[i].reg  & 0xff00)>>8 );
		buf[len++] = (u8 )((ID_type[i].reg  & 0xff) );
	
		//len +=	_memcpy_len(&buf[len],(u8*)&CAN_time ,4 ) ;
	
		//len +=  _memcpy_len( &buf[len] ,(u8 *)&ID_type[i].reg , 4);
		//logd("ID:0x%04X",ID_type[i].bit.ID);
		
	}


	tr9_6036_task(buf,len);
}

#endif

//0x6037	CAN总线数据上传【MCU->SOC】
//在can.c中

//0x6038		定位模式切换指令
static void tr9_Gps_model(u16 index, u8 *str, u16 s_len) {
    u8 i;
    //	u8 n = 0 ;
    gnss_cfg.select.reg = 0;
    if (*str == 0) {
        logd("无效操作");
        return;
    }

    for (i = 0; i < 4; i++) {
        if ((*str & (0x01 << i)) != 0) {
            switch (i) {
            case 0: gnss_cfg.select.b.GPS_EN = 1; break;
            case 1: gnss_cfg.select.b.BD_EN = 1; break;
            case 2: gnss_cfg.select.b.GLNS_EN = 1; break;
            case 3: gnss_cfg.select.b.GLEO_EN = 1; break;
            }
        }
    }
    if (gnss_cfg.select.b.GPS_EN) {
        logd("设置为GPS模式");
        host_no_save_para.set_gps_module_type = 1;
    } else if (gnss_cfg.select.b.BD_EN) {
        logd("设置为北斗模式");
        host_no_save_para.set_gps_module_type = 2;
    } else if (gnss_cfg.select.b.GLNS_EN) {
        logd("设置为GLONA-SS模式");
        host_no_save_para.set_gps_module_type = 3;
    } else if (gnss_cfg.select.b.GLEO_EN) {
        logd("设置为Galileo模式");
        //host_no_save_para.set_gps_module_type = 3;
    }

    tr9_frame_pack2rk(index, 0, 0); //应答数据帧
}

//	鉴权码发送 6039
static void tr9_6039_task(u16 index, u8 *str, u16 s_len) {
    u8 *pa = str;
    u8 len = 0, len2 = 0;
    _memset(authority_cfg.code, 0x00, 50);

    len = *pa++;
    if (0 == len)
        logd("鉴权码格式错误");

    authority_cfg.type = 0xcc;
    pa += _memcpy_len(authority_cfg.code, pa, len);
    logd("收到主平台鉴权码:%s", authority_cfg.code);

    len2 = *pa++;
    if (0 != len2) {
        _memset(authority_cfg.code2, 0x00, 50);
        authority_cfg.type2 = 0xcc;
        len2 += _memcpy_len(authority_cfg.code2, pa, len2);
        logd("收到备用平台鉴权码:%s", authority_cfg.code2);
    }

    tr9_frame_pack2rk(index, 0, 0); //应答数据帧
}

//设置电话本
static void tr9_603A_task(u16 index, u8 *str, u16 s_len) {
    bool set_type;
    short i;
    phbk_count_struct phbk_count;
    phbk_data_struct phbk_data;

    if (*str == 0) {
        phbk_data_flash_init(); //删除全部联系人
    } else if (*str > 0) {
        spi_flash_read(((u8 *)(&phbk_count)), phbk_start_addr_count, (sizeof(phbk_count_struct))); //提取总数
        (phbk_count.posi) = (phbk_count.posi) % phone_book_total_max;
        if ((phbk_count.total) > phone_book_total_max) {
            (phbk_count.total) = phone_book_total_max;
        }

        if (*str == 0x01) //更新操作
        {
            phbk_count.total = 0x00;
            phbk_count.posi = 0x00;
        }
        if (*str == 0x02 || *str == 0x01)
            set_type = true;
        else
            set_type = false;

        for (i = 0; i < *(str++); ++i) //总长度
        {
            phbk_data.type = (phbk_type_enum)(*(str++)); //类型
            phbk_data.num.lgth = *(str++);               //号码长度

            if (phbk_data.num.lgth > phone_book_num_max_lgth)
                phbk_data.num.lgth = phone_book_num_max_lgth;

            str += _memcpy_len(phbk_data.num.buff, str, phbk_data.num.lgth);
            phbk_data.name.lgth = *str++; //名字长度

            if (phbk_data.name.lgth > phone_book_name_max_lgth)
                phbk_data.name.lgth = phone_book_name_max_lgth;
            _memcpy_len(phbk_data.name.buff, str, phbk_data.name.lgth);

            phbk_operate_hdl(((bool)set_type), (&phbk_count), (&phbk_data));
        }
        flash25_program_auto_save(phbk_start_addr_count, ((uchar *)(&phbk_count)), (sizeof(phbk_count_struct))); //保存总数
    }
    tr9_frame_pack2rk(index, 0, 0); //应答数据帧
}

//事件设置
static void tr9_603B_task(u16 index, u8 *str, u16 s_len) {
    u8 type;
    u8 flag, i, n;
    u8 id;
    u8 buf[512] = {0};

    u16 lgth, cnt;
    sms_index_struct sms_index;
    sms_op_enum sms_op;

    //flag = 0x00;  		//删除标志清零
    sms_op = sms_op_add; //赋初值
    type = str[0];       //类型
    n = str[1];          //总数

    //type =  (*(slv_rev_data.buf + spi_rec_expand_valid_start)); //设置类型
    //n =	 (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //总条数
    //ptr =	 (	slv_rev_data.buf + spi_rec_expand_valid_start + 2); //给指针赋值

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_fix_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    switch (type) {
    case 0x00: //删除全部
        flag = 1;
        sms_index.total = 0x00;
        break;

    case 0x01: //更新
        sms_index.total = 0x00;
        sms_op = sms_op_add;
        break;

    case 0x02: //追加
        sms_op = sms_op_add;
        break;

    case 0x03: //修改
        sms_op = sms_op_change;
        break;

    case 0x04: //删除
        sms_op = sms_op_del;
        break;

    default: //数据异常
        return;
    }

    if (flag == 0x00) //非全部删除操作
    {
        for (i = 0x00; i < n; i++) {
            id = str[3];  //ID
            cnt = str[4]; //长度

            if (cnt > (sms_fix_single_piece_lgth - 40)) //长度限制
            {
                lgth = (sms_fix_single_piece_lgth - 40);
            } else {
                lgth = cnt;
            }
            buf[0] = lgth;
            buf[1] = 0x00;
            lgth += 2;
            _memcpy_len(&buf[2], &str[4], lgth);
            sms_operate_hdl((&sms_index), sms_op, id, lgth, buf);

            //  ptr += (cnt + 2);
        }
    }

    flash25_program_auto_save(flash_sms_fix_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if ((menu_cnt.menu_other != mo_sms_display) && (sms_op == sms_op_add)) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_fix;
        sms_up_center.fix_id = ((sms_index.use_infor + 0)->type_id_serial_num);
        spi_flash_read(((u8 *)(&(sms_union_data.fix))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_fix_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_fix_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }
    tr9_frame_pack2rk(index, 0, 0); //应答数据帧
}

#if (0)
//消息报告  0x603C
static void tr9_603C_task(u16 index, u8 *str, u16 s_len) {
    tr9_frame_pack2rk(0x603C, (&sms_up_center.fix_id), 1); //应答数据帧
}
#endif

//提问下发0x603D
static void tr9_603D_task(u16 index, u8 *str, u16 s_len) {
    /*
	u8 	static con = 0;
	u8  type = 0;
	u8  answer_Len	= 0;
	u8  problem_Len = 0;
	u8 	answer_ID	= 0;
	u16 problem_ID 	= 0;
	u8 	problem[160]= {0};
	u8	answer[160]	= {0};
	

    sms_index_struct  sms_index;
    sms_ask_content_struct  sms_ask;
	
	type 		=	str[0];								//类型		
	problem_ID 	=	str[1] << 8;						//问题ID
	problem_ID |=	(str[2] & 0xff);
	
	sms_ask.lgth =  str[3]; 							//问题长度

	sms_ask.answer[con].ans_id   = str[sms_ask.lgth+4];	//答案ID
	sms_ask.answer[con].ans_lgth = str[sms_ask.lgth+5];	//答案长度
	
	sms_ask.ans_cnt = con++ ;							//答案总数			

	if(sms_ask.lgth >sms_ask_content_lgth - 40)
		sms_ask.lgth = sms_ask_content_lgth;
	
	if(sms_ask.answer[con].ans_lgth >sms_ask_content_lgth )
		sms_ask.answer[con].ans_lgth = sms_ask_content_lgth;

	_memcpy(sms_ask.dat,(u8*)&str[4],sms_ask.lgth);		//问题内容
	_memcpy(sms_ask.answer[con].ans_dat,(u8*)&str[sms_ask.lgth+6],answer_Len);//问题答案

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_ask_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    sms_operate_hdl((&sms_index), sms_op_add, problem_ID, (sizeof(sms_ask)), ((u8 *)(&sms_ask)));
    flash25_program_auto_save(flash_sms_ask_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
*/
    //	u8  TY_ACK[32]	= {0} ;
    sms_index_struct sms_index;
    sms_ask_content_struct sms_ask;

    u8 *ptr;
    u16 i;
    //    u16  cnt;
    u16 n;
    //    u16  ser_num;

    ptr = str;
    //	ser_num = ((*(ptr + 1)) * 0x100) + (*(ptr + 2)); //记录流水号

    n = (*(ptr + 3)); //问题长度
    i = 4;            //修改指针，指向问题内容

    if (n > (sms_ask_content_lgth - 40)) //判断长度限制
    {
        sms_ask.lgth = (sms_ask_content_lgth - 40);
    } else {
        sms_ask.lgth = n;
    }

    _memcpy(sms_ask.dat, (ptr + i), sms_ask.lgth); //赋值内容
    i += n;                                        //修改指针，指向问题答案

    sms_ask.ans_cnt = 0x00;
    // while (i < cnt)
    {
        // if (sms_ask.ans_cnt >= sms_ask_answer_total_max) //答案数目超过范围
        //break;

        (sms_ask.answer + sms_ask.ans_cnt)->ans_id = *(ptr + i); //答案ID
        n = ((*(ptr + i + 1)) * 0x100) + (*(ptr + i + 2));       //答案长度

        if (n > sms_ask_answer_content_lgth) //判断长度限制
        {
            (sms_ask.answer + sms_ask.ans_cnt)->ans_lgth = sms_ask_answer_content_lgth;
        } else {
            (sms_ask.answer + sms_ask.ans_cnt)->ans_lgth = n;
        }

        _memcpy(((sms_ask.answer + sms_ask.ans_cnt)->ans_dat), (ptr + i + 3), ((sms_ask.answer + sms_ask.ans_cnt)->ans_lgth)); //赋值内容

        i = (i + 3 + n);
        sms_ask.ans_cnt++;
    }
    if (menu_cnt.menu_other != mo_sms_display) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_ask;
        sms_up_center.ask_num = ((sms_index.use_infor + 0)->type_id_serial_num);
        spi_flash_read(((u8 *)(&(sms_union_data.ask))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_ask_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_ask_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    pack_general_answer(0); //通用应答
                            //tr9_frame_pack2rk(index,0,0);
}

#if (0)
//提问应答0x603E
static void tr9_603E_task(u16 index, u8 *str, u16 s_len) {
    u8 temp[3];

    *(temp + 0) = (sms_up_center.ask_num) / 0x100;
    *(temp + 1) = (sms_up_center.ask_num) % 0x100;
    *(temp + 2) = *((sms_up_center.ask_ans) + (sms_up_center.ask_id_point));
    tr9_frame_pack2rk(index, temp, 0x603E);
}
#endif

//信息点播菜单设置 【SOC->MCU】
static void tr9_603F_task(u16 index, u8 *str, u16 s_len) {
    u8 type;
    u8 flag, i, n;
    u8 *ptr;

    u8 id;
    u16 lgth, cnt;

    sms_index_struct sms_index;
    sms_op_enum sms_op;

    flag = 0x00; //删除标志清零
    sms_op = sms_op_add;

    type = str[0];
    n = str[1];
    ptr = &str[2];

    //type = (*(slv_rev_data.buf + spi_rec_expand_valid_start));  //设置类型
    //n = (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //总条数
    //ptr =(slv_rev_data.buf + spi_rec_expand_valid_start + 2);  //给指针赋值

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_vod_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    switch (type) {
    case 0x00: //删除全部
        flag = 1;
        sms_index.total = 0x00;
        break;

    case 0x01: //更新
        sms_index.total = 0x00;
        sms_op = sms_op_add;
        break;

    case 0x02: //追加
        sms_op = sms_op_add;
        break;

    case 0x03: //修改
        sms_op = sms_op_change;
        break;

    default: //数据异常
        return;
    }

    if (flag == 0x00) //非全部删除操作
    {
        for (i = 0x00; i < n; i++) {
            id = *(ptr + 0);
            cnt = ((*(ptr + 1)) * 0x100) + (*(ptr + 2));

            if (cnt > (sms_vod_single_piece_lgth - 40)) //长度限制
            {
                lgth = (sms_vod_single_piece_lgth - 40);
            } else {
                lgth = cnt;
            }

            *(ptr + 1) = (lgth % 0x100);
            *(ptr + 2) = (lgth / 0x100); //借用缓冲区存储长度
            lgth += 2;

            sms_operate_hdl((&sms_index), sms_op, id, lgth, (ptr + 1));

            ptr = (ptr + cnt + 3);
        }
    }

    flash25_program_auto_save(flash_sms_vod_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if ((menu_cnt.menu_other != mo_sms_display) && (sms_op == sms_op_add)) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_vod;
        sms_up_center.vod_type = ((sms_index.use_infor + 0)->type_id_serial_num);
        spi_flash_read(((u8 *)(&(sms_union_data.vod))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_vod_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_vod_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }
}

#if (0)
// 0x6040 消息点播/取消
static void tr9_6040_task(u16 index, u8 *str, u16 s_len) {
    u8 temp[2];

    *(temp + 0) = sms_up_center.vod_type;
    *(temp + 1) = sms_up_center.vod_status;

    up_packeg((0xee0303), (temp), 2);
}
#endif

//0x6041 信息服务
static void tr9_6041_task(u16 index, u8 *str, u16 s_len) {
    sms_index_struct sms_index;
    u8 type = str[0];
    u16 len = 0, n = 0;
    u8 buf[200] = {0};

    len = str[1] & 0xff << 8;
    len |= str[2] & 0xff;

    if (len > sms_serve_single_piece_lgth) //长度限制
    {
        len = sms_serve_single_piece_lgth;
    }
    spi_flash_read(((u8 *)(&sms_index)), flash_sms_serve_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    buf[0] = (len % 0x100);
    buf[1] = (len / 0x100);
    len = _memcpy_len(&buf[2], &str[3], len);

    sms_operate_hdl((&sms_index), sms_op_add, (*(slv_rev_data.buf + spi_rec_expand_valid_start)), len, buf);
    flash25_program_auto_save(flash_sms_serve_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_serve;
        spi_flash_read(((u8 *)(&(sms_union_data.serve))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_serve_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_serve_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }
    if (type & (bit_move(3))) //有朗读信息的请求
    {
        if ((n - 2) > sound_play_buff_max_lgth)
            n = sound_play_buff_max_lgth;
        else
            n -= 2;

        _memcpy((sound_play_buff + 2), (slv_rev_data.buf + spi_rec_expand_valid_start + 1), n);
        (*((u16 *)sound_play_buff)) = n;

        slave_send_msg((0xee0000), 0x00, 0x00, false, spi1_up_comm_team_max);
    }
    tr9_frame_pack2rk(index, 0, 0);
}

#if (0)
static void tr9_6042_task(u16 index, u8 *str, u16 s_len) {
    printf_gsm("AT+CLVL? \r\n");
}
#endif

static void tr9_6043_task(u16 index, u8 *str, u16 s_len) {
    //	u8 cmd[24] = {0} ;

    loge("undef tr9_6043_task");
    /*
	if( *str>= 0 && *str <=15)
	{
		printf_gsm("AT+CLVL= %d \r\n",(*str)*6);
	}
	*/
}
/***************************************************************************************
* 函数名称：AwakenSet
* 函数功能：定时唤醒功能,条件唤醒功能，短信唤醒功能
* 函数参数：
* 时     间:	2022.1.15
* 备     注：触发设置条件,定时唤醒时间，就会唤醒
*修改记录: 22.04.15 修改定时唤醒的部分
***************************************************************************************/
void AwakenSet(void) { //终端唤醒设置
    u8 i = 0;
    //	u8 j = 0 ;
    //u8 bit =  0 ;
    u8 count = 0;
    u8 date = 0;
    //	u8 record = 255;
    /*
	u8 nowTime_hour = 0 ;
	u8 min_s =  0, min_e  = 0 ;
	u8 hour_s = 0, hour_e = 0 ;
	*/
    static u16 min = 0;
    static bool time_Bucket = false;

    static bool sms_flag = false;
    static bool state_flag = false;
    static u8 buf[4] = {0};
    static u8 yesterday = 0;
    static u8 rouse_step[4] = {0};
    //	static u8 	rouse_step1[4] = {0} ;
    //static u16 nowTime_minute = 0 ;
    //22/04/15  修改
    //	static u16 powerOffTime= 0 ;
    //	static u16 powerOnTime= 0 ;
    static u16 real_time = 0;

    u16 rouse_time = 5; //默认5分钟

    if (Set_Awaken.Revc_sms_flag && Set_Awaken.note_rouse) {
        if (!rouse_state) {
            if (Rk_Task_Manage.RK_send_0200) {
                gsm_power_down(); //给EC200A断电重启
                Open_Pwr_All();
                sms_flag = true;
                Rk_Task_Manage.RK_send_0200 = false;
            } else if (login_net_123() == true) // else if (!Rk_Task_Manage.RK_send_0200_flag)
            {
                mix.placeIostate = 00;
                Rk_Task_Manage.RK_send_0200 = true;
                Rk_Task_Manage.RK_send_0200_flag = true;
                send_base_report();
                logd("_使用短信换醒,%d分钟后休眠_", Set_Awaken.sms_wakeup_Time);

            } else {
                logd("_使用短信换醒,%d分钟后休眠_", Set_Awaken.sms_wakeup_Time);
                Rk_Task_Manage.RK_send_0200 = true;
            }
        }
    }

    /**********************************条件唤醒*****************************************/

    if (Set_Awaken.state_rouse) //条件唤醒
    {
        if (!rouse_state) {
            if (Set_Awaken.bit.Sos_Flag || Set_Awaken.bit.Crash_Flag || Set_Awaken.bit.Car_Door_Flag) {
                if (car_alarm.bit.sos || car_alarm.bit.turn_on_one_side || car_alarm.bit.collision || tr9_car_status.bit.res4) {
                    if (!Rk_Task_Manage.RK_WaitingSend) {
                        if (login_net_123()) {
                            mix.placeIostate = 00;
                            Rk_Task_Manage.RK_WaitingSend = true;
                            Rk_Task_Manage.RK_send_0200_flag = true;
                            send_base_report();
                            goto vip;
                        }
                    }
                    state_flag = true;
                    Open_Pwr_All(); //打开全部电源
                    Rk_Task_Manage.RK_WaitingSend = false;
                    logd("触发条件唤醒");
                    //rouse_flag = true ;							//处于唤醒状态中
                    //Set_Awaken.bit.Sos_Flag	= false ;
                    //Set_Awaken.bit.Crash_Flag = false ;
                    //Set_Awaken.bit.Car_Door_Flag = false ;
                }
            }
        }
    vip:;
        //logd("等待RK唤醒");
    }

    /********************************************定时唤醒*******************************************/

    /******* 如果今天是新的一天,或者是更新了唤醒时间,就判断今天需不需要唤醒RK **********/

    if (Set_Awaken.timer_rouse && (yesterday != sys_time.date || Set_Awaken.update_timr)) //定时唤醒
    {
        date = date_to_week(sys_time.year, sys_time.month, sys_time.date); //计算星期
        logd(" 今天是周%d,", date);
        switch (date) {
        case 0:
            if (Set_Awaken.week.weekend)
                time_Bucket = true;
            break;
        case 1:
            if (Set_Awaken.week.Monday)
                time_Bucket = true;
            break;
        case 2:
            if (Set_Awaken.week.Tuesday)
                time_Bucket = true;
            break;
        case 3:
            if (Set_Awaken.week.Wednesday)
                time_Bucket = true;
            break;
        case 4:
            if (Set_Awaken.week.Thursday)
                time_Bucket = true;
            break;
        case 5:
            if (Set_Awaken.week.Friday)
                time_Bucket = true;
            break;
        case 6:
            if (Set_Awaken.week.Saturday)
                time_Bucket = true;
            break;
        }
        yesterday = sys_time.date;      //如果今天不是指定唤醒日期就等到明天
        Set_Awaken.update_timr = false; //重新设置定时 完成
//spi_flash_write((u8*)&Set_Awaken ,flash_Awaken_param_addr,sizeof(Set_Awaken)); //保存唤醒数据
#if 1
        if (time_Bucket) { //今天有定时任务
            /* 刷新定时任务 */
            logd("有定时任务");
        } else {
            logd("无定时任务");
        }
#endif

        for (i = 0; i < 4; i++) {
            if (Set_Awaken.time_Bucket & 0x01 << i)
                rouse_step[i] = 1;
        }
    }

    if ((time_Bucket) && (!car_state.bit.acc)) //到了指定日期
    {
        if (min != sys_time.min) {
            min = sys_time.min;
            real_time = sys_time.hour * 60 + sys_time.min; //把时间转换成分钟来比较
            //logd("实时分:%d ,唤醒分:%d",real_time,Set_Awaken.timing_Awaken[0].OnTime);
            for (i = 0; i < 4; i++) {
                if (Set_Awaken.time_Bucket & (0x01 << i)) //定时唤醒时间段
                {
                    if ((rouse_step[i] == 1) && (!rouse_state)) {
                        if ((real_time >= Set_Awaken.timing_Awaken[i].OnTime) && (real_time < Set_Awaken.timing_Awaken[i].OffTime)) {
                            if (Rk_Task_Manage.RK_WaitingSend == false) {
                                if (login_net_123() == true) {
                                    mix.placeIostate = 00;
                                    Rk_Task_Manage.RK_WaitingSend = true;
                                    Rk_Task_Manage.RK_send_0200_flag = true;
                                    send_base_report();
                                    min = 100;
                                    continue;
                                }
                            }

                            rouse_step[i] = 2;
                            Open_Pwr_All();
                            Rk_Task_Manage.RK_WaitingSend = false;
                            logd("时间段%d 定时唤醒时间到", i + 1);
                        }
                    } else if (rouse_step[i] == 2) {
                        if (s_Hi3520_Monitor.start /*&&  s_Hi3520_Monitor.ack*/) //代表RK上线
                        {
                            rouse_step[i] = 0;
                            logd("RK上线发送6046");

                            rouse_time = (Set_Awaken.timing_Awaken[i].OffTime - Set_Awaken.timing_Awaken[i].OnTime);
                            buf[0] = 0x02; //定时唤醒模式
                            buf[1] = (rouse_time & 0xff00) >> 8;
                            buf[2] = (rouse_time & 0x00ff);
                            tr9_frame_pack2rk(0x6046, buf, 3);
                        }
                    }
                }
                if (rouse_step[i] == 0) //判断今天任务是否全部完成
                {
                    count++;
                }
            }
            logd("count = %d", count);
        }
    }
    if (count == 4) {
        time_Bucket = false;
        logd("今日定时任务完成");
    }
    count = 0;

    if (sms_flag || state_flag) {
        if (s_Hi3520_Monitor.start /* &&  s_Hi3520_Monitor.ack*/) //代表RK上线
        {
            if (sms_flag) {
                logd("RK上线发送6046");
                tr9_show = true;
                sms_flag = false;
                Set_Awaken.Revc_sms_flag = false;

                buf[0] = 0x04;                                         //短信唤醒
                buf[1] = ((Set_Awaken.sms_wakeup_Time & 0xff00) >> 8); //唤醒时长_高字节
                buf[2] = (Set_Awaken.sms_wakeup_Time & 0x00ff);        //唤醒时长_低字节
                tr9_frame_pack2rk(0x6046, buf, 3);
            } else if (state_flag) {
                logd("RK上线发送6046");
                tr9_show = true;
                state_flag = false;
                rouse_time = 0x05;                     //默认五分钟
                buf[0] = 0x01;                         //条件唤醒
                buf[1] = ((rouse_time & 0xff00) >> 8); //唤醒时长_高字节
                buf[2] = (rouse_time & 0x00ff);        //唤醒时长_低字节
                tr9_frame_pack2rk(0x6046, buf, 3);
            }
        }
    }

    /************************************短信唤醒****************************/
}

//终端唤醒设置

/***************************************************************************************
* 函数名称：tr9_6045_task
* 函数功能：设置定时唤醒参数
* 函数参数：1.定时数据 2.数据长度
* 工 程 师：@秦
* 时     间:	2022.1.8
* 备     注：无
***************************************************************************************/
static void tr9_6045_task(u16 index, u8 *str, u16 s_len) {
    u8 i, n = 0;
    u8 len = 0;
    //	u8 *pa  = NULL ;
    u8 OffTime[2] = {0};
    u8 OnTime[2] = {0};
    ////	u16 OffT = 0;
    //	u16 OnT = 0;

    memset((u8 *)&Set_Awaken, 0x00, sizeof(Set_Awaken_t));

    for (i = 0; i < 3; i++) {
        if (str[0] & (0x01 << i)) {
            switch (i) {
            case 0: {
                len++;
                Set_Awaken.state_rouse = 1; //条件唤醒
                logd("条件唤醒设置");
                for (n = 0; n < 3; n++) {
                    if (str[len] & (0x1 << n)) {
                        switch (n) {
                        case 0:
                            Set_Awaken.bit.Sos_Flag = true;
                            logd("开启紧急唤醒功能");
                            break;
                        case 1:
                            Set_Awaken.bit.Crash_Flag = true;
                            logd("开启侧翻唤醒功能");
                            break;
                        case 2:
                            Set_Awaken.bit.Car_Door_Flag = true;
                            logd("开启车门打开唤醒功能");
                            break;
                        }
                    }
                }
            } break;
            case 1: {
                if (len == 0)
                    len++;
                len++;
                //定时设置
                Set_Awaken.update_timr = true; //更新定时设置
                Set_Awaken.timer_rouse = true; //定时唤醒标志
                logd("定时唤醒设置");
#if (1)
                logd_NoNewLn("星期:");
                for (n = 0; n < 7; n++) {
                    if (str[len] & (1 << n)) {
                        switch (n) {
                        case 0: Set_Awaken.week.Monday = true; break;
                        case 1: Set_Awaken.week.Tuesday = true; break;
                        case 2: Set_Awaken.week.Wednesday = true; break;
                        case 3: Set_Awaken.week.Thursday = true; break;
                        case 4: Set_Awaken.week.Friday = true; break;
                        case 5: Set_Awaken.week.Saturday = true; break;
                        case 6: Set_Awaken.week.weekend = true; break;
                        }
                        logb_endColor_newLn(log_level_debug, "%d", n + 1);
                    }
                }
#endif //#if (1)
                logd("有定时任务");
                len++;
                Set_Awaken.time_Bucket = str[len++]; //唤醒时间段标志
                for (i = 0; i < 4; i++) {
                    len += _memcpy_len(OnTime, &str[len], 2);  //开始时间
                    len += _memcpy_len(OffTime, &str[len], 2); //关闭时间

                    OnTime[0] = _bcdtobin(OnTime[0]);
                    OnTime[1] = _bcdtobin(OnTime[1]);

                    OffTime[0] = _bcdtobin(OffTime[0]);
                    OffTime[1] = _bcdtobin(OffTime[1]);

                    logd("时间段%d, 开机时间:%02d:%02d, 关机时间:%02d:%02d", i + 1, OnTime[0], OnTime[1], OffTime[0], OffTime[1]);

                    //转成分钟方便比较
                    Set_Awaken.timing_Awaken[i].OnTime = OnTime[0] * 60 + OnTime[1];
                    Set_Awaken.timing_Awaken[i].OffTime = OffTime[0] * 60 + OffTime[1];
                    //logd("--on: %02d --off: %02d",Set_Awaken.timing_Awaken[i].OnTime,Set_Awaken.timing_Awaken[i].OffTime);
                }
                /*
					len += _memcpy_len(Set_Awaken.timing_Awaken[1].start_time, &str[len], 2); //开始时间
					len += _memcpy_len(Set_Awaken.timing_Awaken[1].colse_time, &str[len], 2); //关闭时间
					
					len += _memcpy_len(Set_Awaken.timing_Awaken[2].start_time, str+len, 2); //开始时间	
					len += _memcpy_len(Set_Awaken.timing_Awaken[2].colse_time, str+len, 2); //关闭时间
					
					len += _memcpy_len(Set_Awaken.timing_Awaken[3].start_time, str+len, 2); //开始时间
					len += _memcpy_len(Set_Awaken.timing_Awaken[3].colse_time, str+len, 2); //关闭时间		
					*/
            } break;
            case 2: {
                Set_Awaken.note_rouse = true; //短信唤醒
                logd("设置短信唤醒");
            } break;
            }
        }
    }
    tr9_frame_pack2rk(0x6045, 0, 0);
    flash25_program_auto_save(flash_Awaken_param_addr, (u8 *)&Set_Awaken, sizeof(Set_Awaken)); //保存参数
                                                                                               //spi_flash_write((u8*)&Set_Awaken ,flash_Awaken_param_addr,sizeof(Set_Awaken)); //保存唤醒数据
    //date_to_week(); //计算星期
    //spi_flash_write((u8*)&Set_Awaken,);
    //logd("Set_Awaken = 0x%X",Set_Awaken);
}

//主动关闭电源
static void tr9_6047_task(u16 index, u8 *str, u16 s_len) {
    tr9_frame_pack2rk(0x6047, 0, 0);
    Rk_Task_Manage.RK_6047_flag = true;
}

/*********************************
*函数功能：从平台地址设置 
*********************************/
void tr9_6044_task(void) {
    //	u8 i,n=0,mm=0;
    u8 len = 0;
    u16 op = 0;
    u8 buf[35] = {0};

    op = (set_para.b_ip[20] - 48) * 1000;
    op += (set_para.b_ip[21] - 48) * 100;
    op += (set_para.b_ip[22] - 48) * 10;
    op += (set_para.b_ip[23] - 48);

    buf[0] = 0x03;
    _memcpy_len(&buf[1], &set_para.b_ip[4], 15);
    buf[16] = ((u8)((op & 0xff00) >> 8));
    buf[17] = ((u8)(op & 0xff));
    len += _memcpy_len(&buf[18], server_cfg.bak_ip, 15);
    buf[33] = ((u8)((server_cfg.bak_port & 0xff00) >> 8));
    buf[34] = ((u8)(server_cfg.bak_port & 0xff));
    tr9_frame_pack2rk(0x6044, buf, 35);
}

#if (0)
//热点开关
static void tr9_6049_task(u16 index, u8 *str, u16 s_len) {
    tr9_frame_pack2rk(0x6049, str, s_len);
}
#endif

//载货状态
static void tr9_6050_task(u16 index, u8 *str, u16 s_len) {
    if (*str < 4) {
        if (0x00 == *str)
            car_state.bit.carriage = 0x01; //空载
        else if (0x01 == *str)
            car_state.bit.carriage = 0x02; //半载
        else if (0x03 == *str)
            car_state.bit.carriage = 0x03; //满载
        else
            car_state.bit.carriage = 0x01;
    } else
        logd("6050设置失败!没有这个选项");

    tr9_frame_pack2rk(0x6050, 0, 0);
}
//挂载U盘
static void tr9_6051_task(u16 index, u8 *str, u16 s_len) {
    Rk_Task_Manage.RK_6051_state = true;
    Rk_Task_Manage.RK_record_print_log = true;
    menu_cnt.menu_other = mo_message_window;
    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
    mw_dis.p_dis_1ln = ((u8 *)(SUB_Insertion));
    mw_dis.p_dis_3ln = ((u8 *)(SUB_export));
    mw_dis.p_dis_4ln = ((u8 *)(SUB_ack));

    mw_dis.bg_1ln_pixel = 16 * 2; //信息提示窗口设置
    mw_dis.bg_3ln_pixel = 16;
    mw_dis.bg_4ln_pixel = 16;

//Allot.Data_Direction = 1 ;		//数据去向
//Data_Direction = 1 ;
//2022-03-17 modify by hj
#if 1
    /*如果收到 6051信息表示 RK需要进行U盘下载行驶记录数据*/
    /*这里先把与行驶记录仪相关的信息初始化               */
    _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
    rk_6033_task.cmd = 0x00;         //初始化 cmd = 0x00等待 “确认”键按下。
    rk_6033_task.u_read_flag = 1;    //等待 “确认”键按下。
    rk_6033_task.Data_Direction = 1; //数据方向
#endif

    tr9_frame_pack2rk(0x6051, 0, 0); //应答RK;
}
//导出完成
static void tr9_6052_task(u16 index, u8 *str, u16 s_len) {
    if ((*str) == 0) { //导出成功
        Export_finish = 1;
    } else { //导出失败
        Export_finish = 2;
    }

    tr9_frame_pack2rk(0x6052, 0, 0);
}

//6056终端里程上报:【RK->MCU】；
static void tr9_6056_task(u16 index, u8 *str, u16 s_len) {
    //u32 mileage  = 0;
    mix.dist = *str;
    tr9_frame_pack2rk(index, 0, 0);
}

//6057主电压查询://6057， 终端主电压查询【RK->MCU】；新增
static void tr9_6057_task(u16 index, u8 *str, u16 s_len) {
    u16 pwr_vol = mix.pwr_vol;
    _sw_endian((u8 *)&pwr_vol, (u8 *)&mix.pwr_vol, 2); //大小端转接
    logd("pwr_vol: %0.1fV", (float)mix.pwr_vol / 100);
    tr9_frame_pack2rk(index, (u8 *)&pwr_vol, 2); //发送业务
}

//6058开关控制://VCC_OUT1 开关控制(5V_OUT_SW开关)【RK->MCU】
static void tr9_6058_task(u16 index, u8 *str, u16 s_len) {
    tr9_frame_pack2rk(index, 0, 0); //应答

    u8 cmd = *str;

    if (1 == cmd) {
        vout1_5v_enable();
    } else if (0 == cmd) {
        vout1_5v_disable();
    } else {
        ; //只读
    }

    cmd = vout1_5v_get_state();
    tr9_frame_pack2rk(index, &cmd, 1); //发送业务数据
}

//6059继电器开关://继电器开关【RK->MCU】；
//0,关//1，开//其他，仅读取
static void tr9_6059_task(u16 index, u8 *str, u16 s_len) {
    tr9_frame_pack2rk(index, 0, 0); //应答

    u8 cmd = *str;

    if (1 == cmd) {
        relay_enable();
    } else if (0 == cmd) {
        relay_disable();
    } else {
        ; //只读
    }

    cmd = relay_get_state();
    tr9_frame_pack2rk(index, &cmd, 1); //发送业务数据
}

//0x6062//6.79 MCU复位【RK->MCU】
static void tr9_6062_task(u16 index, u8 *str, u16 s_len) {
#if (DEBUG_EN_WATCH_DOG)

    log_write(event_mcu_boot_tr9_6062);

    while (1) { ; }
#else
    logf("ret mcu, undef");
#endif
}

//6063, //6.80 摄像头状态【RK->MCU】
static void tr9_6063_task(u16 index, u8 *str, u16 s_len) {
    if (s_len == 2) { //当前版本，只有摄像头状态//当前有7路摄像头配置
        selfChk_cam_stateSet(7, reverse_u16(*(unsigned short *)str));
        tr9_frame_pack2rk(index, NULL, 0); //应答
    }
}

//6064, //6.81 存储设备状态【RK->MCU】
static void tr9_6064_task(u16 index, u8 *str, u16 s_len) {
    if (s_len == 2) {
        selfChk_HD_SD_stateSet((unsigned char)str[0], (unsigned char)str[1]);
        tr9_frame_pack2rk(index, NULL, 0); //应答
    }
}

//6065, //六轴LSM6DS3TR-C:
static void tr9_6065_task(u16 index, u8 *str, u16 s_len) {
    // logd("lsm ack");
}

//清除行驶记录仪数据导出完 不带参数 2022-03-17 add by hj
static void tr9_6066_task(u16 index, u8 *str, u16 s_len) {
    rcd_manage_init();
    tr9_frame_pack2rk(0x6066, 0, 0);
}

//TR9下发
static cmd_net const Tr9Tab[] = {
    {tr9_cmd_1717, tr9_noromal_ack, 0, NULL},
    {tr9_cmd_1810, tr9_pwr_set, 0, NULL}, //soc下发
    // {tr9_cmd_1818, tr9_pwr_off,0,NULL},		//单片机上传数据
    {tr9_cmd_1819, tr9_agree_pwr_close, 0, NULL},

    {tr9_cmd_2010, tr9_video_status, 0, "video"},
    {tr9_cmd_2011, tr9_ec20_ctrl, 0, "ec20"},
    {tr9_cmd_2012, tr9_set_server_para, 0, NULL},
    {tr9_cmd_2013, tr9_set_sim_id, 0, NULL},
    {tr9_cmd_2014, tr9_send_temperature, 0, NULL},

    {tr9_cmd_1415, tr9_ask_mcu_version, 0, NULL},
    {tr9_cmd_1616, tr9_trans_t808_data, 0, NULL},
    {tr9_cmd_2021, tr9_ask_gps_data, 0, NULL},
    {tr9_cmd_2022, tr9_ask_water_no, 0, NULL},
    {tr9_cmd_4001, tr9_set_jt808_2018_para, 0, NULL},

    {tr9_cmd_4040, tr9_update_sd_data, 0, NULL},

    {tr9_cmd_5001, tr9_ask_jt808_2018_para, 0, NULL},
    {tr9_cmd_5003, tr9_tts_data, 0, NULL},
    {tr9_cmd_5004, tr9_request_lock_car, 0, "锁车"},
    {tr9_cmd_5005, tr9_request_cancel_alarm, 0, "取消报警"},
    //{tr9_cmd_5008, tr9_request_raise_ctrl,0,NULL},

    {tr9_cmd_5013, tr9_ic_card_pull_out, 0, "插拔卡"}, //IC-card://司机插拔卡信息【MCU->RK】
    {tr9_cmd_5014, tr9_ic_card_5014, 0, "查询IC卡"},   //0x5014://IC卡信息上报//RK询问IC卡//RK->MCU 下行命令
    {tr9_cmd_5016, tr9_tele_power_ctrl, 0, "tele pwr"},
    {tr9_cmd_5017, tr9_tts_power_ctrl, 0, "tts pwr"},
    {tr9_cmd_5018, tr9_send_jt808_status, 0, "部标上线状态"},
    {tr9_cmd_5019, tr9_set_sleep_rpt_time, 0, "休眠位置上报间隔"},

    {tr9_cmd_6001, tr9_set_limit_speed, 0, NULL},
    {tr9_cmd_6002, tr9_car_control_set, 0, NULL},
    {tr9_cmd_6003, tr9_6003_task, 1, NULL},

    {tr9_cmd_6007, tr9_update_font_lib, 0, NULL},
    {tr9_cmd_6008, tr9_rece_set_ip, 0, NULL},
    {tr9_cmd_6010, tr9_rece_weight_data, 0, NULL},
    {tr9_cmd_6012, tr9_rece_limit_speed, 0, NULL},
    {tr9_cmd_6014, tr9_version_mess, 0, NULL},
    {tr9_cmd_6015, tr9_set_close_time, 0, NULL},
    {tr9_cmd_6016, tr9_ask_close_time, 0, NULL},
    {tr9_cmd_6017, tr9_ask_restart, 0, NULL},
    {tr9_cmd_6020, tr9_send_wangluo, 0, NULL},

    //海格定制版 :以下的协议是海格特别添加的
    {tr9_cmd_6031, tr9_uart_SeriaNet, 0, NULL},
    {tr9_cmd_6032, tr9_Driving_record, 0, "行驶记录数据采集命令"},
    {tr9_cmd_6033, tr9_Export_state, 0, NULL},
    {tr9_cmd_6034, tr9_Set_Driving_record, 0, "设置行驶记录参数"},
    {tr9_cmd_6038, tr9_Gps_model, 0, NULL},
    {tr9_cmd_6036, tr9_6036_task, 0, "CAN总线ID单独采集设置"}, //【SOC->MCU】
    {tr9_cmd_6039, tr9_6039_task, 0, NULL},                    //鉴权码下发			【SOC->MCU】
    {tr9_cmd_603A, tr9_603A_task, 0, NULL},                    //设置电话本			【SOC->MCU】
    {tr9_cmd_603B, tr9_603B_task, 0, "事件设置"},              //事件设置			【SOC->MCU】
    {tr9_cmd_603D, tr9_603D_task, 0, "提问下发"},              //提问下发 			【SOC->MCU】
    {tr9_cmd_603F, tr9_603F_task, 0, NULL},                    //消息点播菜单设置 	【SOC->MCU】
    {tr9_cmd_6041, tr9_6041_task, 0, "信息服务"},              //信息服务			【SOC->MCU】
    {tr9_cmd_6043, tr9_6043_task, 0, "TTS音量调节应答"},       //音量调节应答 		【SOC->MCU】
    {tr9_cmd_6045, tr9_6045_task, 0, "终端休眠唤醒设置"},      //终端休眠唤醒设置	【SOC->MCU】
    {tr9_cmd_6047, tr9_6047_task, 0, NULL},                    //同意关闭电源		【SOC->MCU】
    {tr9_cmd_6050, tr9_6050_task, 0, NULL},
    {tr9_cmd_6051, tr9_6051_task, 0, NULL},                  //挂载U盘				【SOC->MCU】
    {tr9_cmd_6052, tr9_6052_task, 0, NULL},                  //U盘导出完成通知		【SOC->MCU】
    {tr9_cmd_6054, tr9_6054_task, 0, "CAN总线采集上传设置"}, //设置CAN采集时间		【SOC->MCU】
    {tr9_cmd_6056, tr9_6056_task, 0, "里程上报"},            //6056终端里程上报:【RK->MCU】；
    {tr9_cmd_6057, tr9_6057_task, 0, "主电压查询"},          //6057主电压查询://6057， 终端主电压查询【RK->MCU】；新增
    {tr9_cmd_6058, tr9_6058_task, 0, "VCC_OUT1开关"},        //6058开关控制://VCC_OUT1 开关控制(5V_OUT_SW开关)【RK->MCU】
    {tr9_cmd_6062, tr9_6062_task, 0, "MCU-RST"},             //0x6062//6.79 MCU复位【RK->MCU】
    {tr9_cmd_6063, tr9_6063_task, 0, "CAM"},
    {tr9_cmd_6064, tr9_6064_task, 0, "SD-HD"},
    {tr9_cmd_6065, tr9_6065_task, 0, "六轴"},         //六轴LSM6DS3TR-C:
    {tr9_cmd_6059, tr9_6059_task, 0, "继电器"},       //6059继电器开关://继电器开关【RK->MCU】；
    {tr9_cmd_6066, tr9_6066_task, 0, "清除行驶记录"}, //清除行驶记录仪数据【SOC->MCU】 2022-03-18 为方便测试部标测试
                                                      //{tr9_cmd_6037, tr9_6037_task,0,NULL},
                                                      //{tr9_cmd_6049,tr9_6049_task,0,NULL},		//热点开关
                                                      //{tr9_cmd_6042,tr9_6042_task,0,NULL},		//音量调节				【MCU->SOC】
                                                      //{tr9_cmd_6040,tr9_6040_task,0,NULL},	  	//消息点播		  		【MCU->SOC】
                                                      //{tr9_cmd_603E,tr9_603E_task,0,NULL},	  	//提问应答		  		【MCU->SOC】
                                                      //{tr9_cmd_603C,tr9_603C_task,0,NULL},	  	//事件报告		  		【MCU->SOC】
                                                      //{tr9_cmd_6033, tr9_Driving_record,0,NULL},	//行驶记录仪			【MCU->SOC】
    {NULL, NULL, 0, NULL}};

#if (0)
//鍙戦?佸懡浠ゆ垨鏁版嵁鍒癟R9
//发命令或数据到TR9
u16 _add_tr9_char(u32 len, u8 *chr) {
    u16 i, j, k;
    u8 *move;

    j = 0;
    for (i = 0; i < len; i++) {
        if (*chr == 0xf0 || *chr == 0xf1) {
            j++;
            move = chr + (len - i);
            *(move + 1) = '\0';
            for (k = 0; k < (len - i); k++) {
                *move = *(move - 1);
                move--;
            }
            move++;
            if (*chr == 0xf1)
                *move = 0x01;
            else
                *move = 0x02;

            *chr = 0xf1;
            chr++;
        }
        chr++;
    }
    return (j);
}

void monitor_usb_state(void) {
    //	static u8 ecnt = 0;
}

u8 gps_num_in_area(void) {
    if ((gps_base.fix_num == 0)) {
        if (run.area_in_out == _AREA_IN)
            return 15;
    }
    return gps_base.fix_num;
}

void u32_u8(u8 *buf, u32 data) {
    *buf++ = data >> 24;
    *buf++ = data >> 16;
    *buf++ = data >> 8;
    *buf++ = data;
}

//0xf1 0x02 = 0xf0     0xf1 0x01 = 0xf1
//0x7d 0x02 = 0x7e     0x7d 0x01 = 0x7d
u16 _del_tr9_char(u16 len, u8 *chr) {
    u16 i, j, k;
    u8 *move;

    j = 0;
    for (i = 0; i < len; i++) {
        if (*chr++ == 0xf1) {
            j++;
            if (*chr == 0x02)
                *(chr - 1) = 0xf0;

            move = chr;
            for (k = 0; k < (len - i - 2); k++) {
                *move = *(move + 1);
                move++;
            }
            *move = '\0';
        }
    }
    return (j);
}
#endif

#if (1)
#pragma region /* alarm */
typedef enum {
    alarmFunc_sos = 0,            //0	1：紧急报警，触动报警开关后触发
    alarmFunc_speed_over = 1,     //1	1：超速报警
    alarmFunc_fatigue = 2,        //2	1：疲劳驾驶预警
    alarmFunc_antOpen = 5,        //5	1：GNSS 天线未接或被剪断报警
    alarmFunc_antShort = 6,       //6	1：GNSS 天线短路报警
    alarmFunc_lowVolt = 7,        //7	1：终端主电源欠压报警
    alarmFunc_cutVolt = 8,        //8	1：终端主电源掉电报警
    alarmFunc_driveOver = 18,     //18	1：当天累计驾驶超时报警
    alarmFunc_stopOver = 19,      //19	1：超时停车报警
    alarmFunc_collision = 29,     //29	1：碰撞报警
    alarmFunc_turnOnOneSide = 30, //30	1：侧翻预警
} alarmFunc_enum;                 //1717//表5 报警状态位定义
#define alarm(a, b, c) \
    if (a) {           \
        b |= (1 << c); \
    }
#pragma endregion /* alarm */
#endif

/*
汇报基本位置信息到TR9
0xf0	0x17  0x17	 	终??状???	GPS信息	??????	里程	扩展状???位	脉冲速度	NOR	0xf0
帧头	命令??	数据长度						km/h	校验字节	帧尾
1字节	2字节	2字节	9字节	12字节	1字节	4字节	4字节	1字节	1字节	1字节
*/
void send_base_msg_to_tr9(void) {
    //	u8 i;
    u16 len1;
    static u8 timert = 0;
    u8 buf1[200];
    //	u8  stat = 0;
    //	u8  pwr = 0;
    //	u8  tempture = 0;
    //	time_t Real_time;
    //	u16 Uint16_Uint8 ;
    u16 pulse = 0;
    u32 dist = 0;
    u32 Data_splicing = 0;
    //	static u32 lati = 0 ,lngi = 0;

    update_mix_infomation();

    len1 = 0;
    _memset(buf1, 0, 200);

#if (0)
    /* begin
        报警标志:
        表5 报警状态位定义
    */
    len1 += _sw_endian(&buf1[len1], (u8 *)&car_alarm.reg, 4);
    /* begin
        报警标志:
        表5 报警状态位定义
    */
#endif

    //报警标志
    Data_splicing = 0;

#if (1)
#pragma region 1717 表5 报警状态位定义
    alarm(car_alarm.bit.sos, Data_splicing, alarmFunc_sos);               //0	1：紧急报警，触动报警开关后触发
    alarm(car_alarm.bit.speed_over, Data_splicing, alarmFunc_speed_over); //1	1：超速报警
    alarm(car_alarm.bit.fatigue, Data_splicing, alarmFunc_fatigue);       //2	1：疲劳驾驶预警

    alarm(car_alarm.bit.ant_open, Data_splicing, alarmFunc_antOpen);   //5	1：GNSS 天线未接或被剪断报警
    alarm(car_alarm.bit.ant_short, Data_splicing, alarmFunc_antShort); //6	1：GNSS 天线短路报警

    if (car_alarm.bit.ant_short) {
        logf("antShort = %d", car_alarm.bit.ant_short);
    }

    alarm(car_alarm.bit.low_volt, Data_splicing, alarmFunc_lowVolt); //7	1：终端主电源欠压报警
    alarm(car_alarm.bit.cut_volt, Data_splicing, alarmFunc_cutVolt); //8	1：终端主电源掉电报警

    // alarm(car_alarm.bit., Data_splicing, 16); //胎压报警

    alarm(car_alarm.bit.drive_over, Data_splicing, alarmFunc_driveOver); //18	1：当天累计驾驶超时报警
    alarm(car_alarm.bit.stop_over, Data_splicing, alarmFunc_stopOver);   //19	1：超时停车报警

    // alarm(car_alarm.bit.area_in_out, Data_splicing, 20); //
    // alarm(car_alarm.bit.road_in_out, Data_splicing, 21); //
    // alarm(car_alarm.bit.road_over, Data_splicing, 23); //

    alarm(car_alarm.bit.collision, Data_splicing, alarmFunc_collision);            //29	1：碰撞报警
    alarm(car_alarm.bit.turn_on_one_side, Data_splicing, alarmFunc_turnOnOneSide); //30	1：侧翻预警

    //logf("0x%08X", Data_splicing);//打印报警标志

    len1 += _sw_endian(&buf1[len1], (u8 *)&Data_splicing, 4);
#pragma endregion 1717 表5 报警状态位定义
#endif

    //状态
    Data_splicing = 0;
    if (tr9_car_status.bit.covering) //SENSOR-IN9-L
        Data_splicing |= (0x01 << 27);
    if (tr9_car_status.bit.res1) //雾灯
        Data_splicing |= (0x01 << 26);
    if (tr9_car_status.bit.res2) //倒车
        Data_splicing |= (0x01 << 25);
    if (tr9_car_status.bit.res3) //安全带
        Data_splicing |= (0x01 << 24);
    if (tr9_car_status.bit.res4) //车门
        Data_splicing |= (0x01 << 23);
    if (car_state.bit.acc) //行驶状态
        Data_splicing |= (0x01 << 22);
    if (car_state.bit.fix_calileo) //Calileo 定位
        Data_splicing |= (0x01 << 21);
    if (car_state.bit.fix_glonass) //glonass 定位
        Data_splicing |= (0x01 << 20);
    if (car_state.bit.fix_bd) //北斗定位
        Data_splicing |= (0x01 << 19);
    if (car_state.bit.fix_gps) //GPS定位
        Data_splicing |= (0x01 << 18);
    if (tr9_car_status.bit.brake) //制动
        Data_splicing |= (0x01 << 14);
    if (tr9_car_status.bit.left) //左转
        Data_splicing |= (0x01 << 13);
    if (tr9_car_status.bit.right) //右转
        Data_splicing |= (0x01 << 12);
    if (tr9_car_status.bit.far_light) //远光
        Data_splicing |= (0x01 << 11);
    if (tr9_car_status.bit.near_light) //近光
        Data_splicing |= (0x01 << 10);
    if (car_state.bit.carriage == 0x02) //半载
        Data_splicing |= (0x01 << 8);
    else if (car_state.bit.carriage == 0x03) //满载
        Data_splicing |= (0x03 << 8);
    if (car_state.bit.lng) //西经
        Data_splicing |= (0x01 << 3);
    if (car_state.bit.lat) //南纬
        Data_splicing |= (0x01 << 2);
    if (gps_base.fixed) //定位
    {
        Data_splicing |= (0x01 << 1);
        //if(u1_m.cur.b.debug)
        //logd("已定位- ");
    }
    if (car_state.bit.acc) //ACC点火
    {
        Data_splicing |= 0x01;
        //if(u1_m.cur.b.debug)
        //logd("ACC开-");
    }

    len1 += _sw_endian(&buf1[len1], (u8 *)&Data_splicing, 4);

    Data_splicing = 0;
    Data_splicing |= ((ADC_Die[2]) << 16); //两个输入模拟量
    Data_splicing |= (ADC_Die[1]);

    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.locate.lati, 4); //纬度
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.locate.lngi, 4); //经度
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.heigh, 2);       //高度
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.speed, 2);       //速度//GPS速度
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.direction, 2);   //方向
    //logd("1717上传高度%d,经度%d,纬度%d",mix.heigh,mix.locate.lngi,mix.locate.lati);
    if (!u1_m.cur.b.tt) {
        if (timert == sys_time.sec) {
            logd("1717时间重复");
        } else if ((sys_time.sec - timert) >= 2) {
            logd("1717时间跳格");
        }
    }
    timert = sys_time.sec;
    dist = mix.dist * 10;

#pragma region //220828速度类型:
    if (sys_cfg.speed_mode == enum_speed_mode_gps) {
        if (1 < mix.speed) {
            if (mix.speed < 35) { //临时用定位速度代替
                pulse = (u16)(mix.speed + 1) * 10;
            } else if (mix.speed < 60) {
                pulse = (u16)(mix.speed + 2) * 10;
            } else {
                pulse = (u16)(mix.speed + 3) * 10;
            }
        }
    } else if (sys_cfg.speed_mode == enum_speed_mode_can) {
        pulse = wheel_speed * 10; //CAN速度
    } else /*if (sys_cfg.speed_mode == enum_speed_mode_gps)*/ {
        pulse = (u16)plus_speed * 10; //暂时屏蔽,出现车辆系数不正确的问题
    }
#pragma endregion

    len1 += _memcpy_len(&buf1[len1], (u8 *)&mix.time, 6); //时间

    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.fix_num, 1); //卫星数量
    len1 += _sw_endian(&buf1[len1], (u8 *)&dist, 4);        //里程
    //脉冲//脉冲速度//220828速度类型:脉冲速度上报RK的位置
    len1 += _sw_endian(&buf1[len1], (u8 *)&pulse, 2);         //脉冲//脉冲速度
    len1 += _sw_endian(&buf1[len1], (u8 *)&Data_splicing, 4); //模拟量

    //if((mix.speed ||plus_speed)&&(u1_m.cur.b.debug))
    //logd("GPS速度: %02d -脉冲速度-%02d",mix.speed,plus_speed);
    // putHexs(__func__, __LINE__, (u8 *)buf1, 46, 0, -1, -1);
    // uart1_tran_task_nms(5);
    tr9_frame_pack2rk(tr9_cmd_1717, buf1, len1);
    //logd("2 tr9, 1717");
}

//收到RK的命令
static void tr9_netParseSub(tr9_frame_st *frame, unsigned char *bArr) {
    int flgFound = -1;
    int i;
    tr9_frame_st *f = frame;

    for (i = 0; Tr9Tab[i].func != NULL; i++) {
        if (f->cmd == Tr9Tab[i].index) {
            (*Tr9Tab[i].func)(f->cmd, f->data.arr, f->data.len);
            flgFound = 0;
            break;
        }
    }

    if (flgFound == -1) {
        loge("cmd<0x%04X> not found, xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", f->cmd);
    } else {
        tr9_frame_promt(tr9_frame_promt_dir_rx, f, Tr9Tab[i].comment, __func__, __LINE__);
    }
}

//TR9//RK->MCU
void tr9_net_parse(u8 *p, u16 p_len, u8 from) {
    from = from;

    tr9_frame_st *frame = tr9_frame_get_and_need_free(__func__, p, p_len, 1);
    if (frame == NULL) {
        return;
    }

    tr9_netParseSub(frame, p);
    free(frame);

    start_tr9_tmr = 180;
    s_Hi3520_Monitor.start = true;
}

//数据反转??
/*
0xf0<—???>0xf1后紧跟一??0x02??
0xf1<—???>0xf1后紧跟一??0x01??
*/
u16 tr9_data_transfer(u8 *dst, u8 *buf, u16 len) {
    u16 i, j;
    u8 *pf;

    pf = buf;
    j = 0;
    for (i = 0; i < len; i++) {
        if (*pf == 0xf1) {
            pf++;
            if (*pf == 0x02)
                *dst = 0xf0;
            if (*pf == 0x01)
                *dst = 0xf1;
            pf++;
            dst++;
            j++;
        } else {
            *dst++ = *pf++;
            j++;
        }
        if (*pf == 0xF0) {
            *dst++ = *pf++;
            j++;
            return j;
        }
    }
    return (j);
}

void update_tr9_ztc_status(void) {
}

/*********************************************************
跟TR9 主机进??通信；波特率115200
**********************************************************/
void tr9_parse_task(void) {
    //    static u32 state_jif = 0;
    u8 xvalue = 0;
    static u8 r_buf[TR9_COM_DEC_LEN];
    u8 t_buf[TR9_COM_DEC_LEN];
    static u16 r_len = 0;
    u16 cmd = 0;
    u8 cData;
    //    u8  *pf;

#define DEBUG_HERE_202209261526 0

    while (1) {
        if (s_Tr9Com.uiRxdIn == s_Tr9Com.uiRxdOut) {
            break;
        }

        cData = s_Tr9Com.buf[s_Tr9Com.uiRxdOut]; //copy to sCmaControl buffer
        if (++s_Tr9Com.uiRxdOut >= TR9_COM_BUF_LEN) {
            s_Tr9Com.uiRxdOut = 0;
        }

        switch (s_Tr9Com.step) {
        case 0:
            if (cData == 0xF0) {
                s_Tr9Com.step++;
                r_buf[0] = cData;
                r_len = 1;
            }
            break;
        case 1:
            if (cData == 0xF0) { //0x0a
                r_buf[r_len++] = cData;

                if (r_len > 6) {
                    r_len = tr9_data_transfer(t_buf, r_buf, r_len);

#if (DEBUG_HERE_202209261526)
                    putHexs_hex(t_buf, r_len);
#endif

                    //logd("r_len %d", r_len);
                    if ((t_buf[0] == 0xf0) && (t_buf[r_len - 1] == 0xf0)) {
                        xvalue = get_check_xor(t_buf, 1, r_len - 2);
                        //logd("xvalue:%d, t_buf = %s", xvalue , t_buf);
                        if ((xvalue == t_buf[r_len - 2]))
                            tr9_net_parse(t_buf, r_len, 0);
                        else if ((cmd == 0x4001) || (cmd == 0x4040)) {
                            tr9_net_parse(t_buf, r_len, 0);
                        }
                    }
                } else {
#if (DEBUG_HERE_202209261526)
                    putHexs_hex(r_buf, r_len);
#endif
                    r_buf[0] = cData;
                    r_len = 1;
                    break;
                }

                s_Tr9Com.step = 0;
            } else {
                r_buf[r_len++] = cData;
            }
            break;
        default:
            s_Tr9Com.step = 0;
            break;
        }
    }

#if 0
	if(  pub_io.b.dvr_open  == false )
		return;
#endif

    if (font_lib_update.start || font_lib_update.start_m) {
        return;
    }

    //保持1HZ的频率
    /*
	if (_pastn(state_jif) >= 1000 && _pastn(bypass_debug.jiff) >= 1000) 	//1000
	{
	   // update_tr9_ztc_status();
		state_jif = jiffies;
	  // if(Rk_Task_Manage.RK_1717_state == false)
	  // {
			send_base_msg_to_tr9(); 
	  // }
		
	}
	*/
}

//寮?閿?-鍏抽攣-姹囨姤缁橳R9
//寮?鏈哄悗涔熻?涓婃姤鐘舵?佺粰TR9; TR9宸蹭笂鐢靛悗鎿嶄綔
void report_key_status_to_tr9(void) {
    u8 ipb[100];
    u8 len = 0;

    if (car_state.bit.acc == false)
        return;

    if (pub_io.b.key_s && (pub_io.b.dvr_open)) {
        if (_pasti(key_lcok.tmr) >= 2000) {
            key_lcok.tmr = jiffies;
            tr9_frame_pack2rk(tr9_cmd_5012, 0, 0);
        }
    }
    if (run.ip_to_tr9 == 0x01) { //TRSET:ip1<218.066.042.161>port1<8888>ip2<113.012.081.069>port2<6608>sim<40036521243>
        if (_pasti(key_lcok.tmr) >= 1900) {
            key_lcok.tmr = jiffies;
            len = 0;
            _memset(ipb, 0, 100);
            len = _sprintf_len((char *)&ipb, "TRSET:ip1<%s>port1<%d>ip2<%s>port2<%d>sim<%s>", server_cfg.main_ip, server_cfg.main_port, server_cfg.bak_ip, server_cfg.bak_port, &server_cfg.terminal_id[1]);

            tr9_frame_pack2rk(0x6008, ipb, len);
        }
    }
}

/*
卡状态1字节（1为插卡，2为拔卡）+姓名(12字节) （汉字内码,不足在后面以0x00 补齐） +身份证（20字节）
（ACSII 码 不足在后面以0x00 补齐）+机动车驾驶证号码（18）（ACSII 码,不足在后面以0x00 补齐）	
注：拔卡时内容只有1个字节（卡状态）
每隔2秒发送一次；直到响应
*/
void ic_card_to_tr9(void) {
    u8 card[120] = {0};
    u8 len = 0;

    if (ic_tr9.icc == false)
        return;
    if (_pastn(ic_tr9.tmr) > 3900) {
        ic_tr9.tmr = jiffies;
        if (mix.ic_login) {
#if 0
			card[len++] = 1 ;
			_memcpy((u8 *)&card[1], (u8*)&driver_cfg.name , 12);
			_memcpy((u8 *)&card[13], (u8*)&driver_cfg.license , 20);
			_memcpy((u8 *)&card[33], (u8*)&driver_cfg.qualification , 18);
			len = 51;
#else //海格定制版 \
      //card[len++] = 0x01;										//插卡标志
            card[len++] = 0x01;
            len += _memcpy_len(&card[1], (u8 *)&IC_Time.IC_enter, 6); //插卡时间
            if (state_data.state.flag.ic_flag == true) {              //插卡状态
                card[7] = 0x00;
                len += 1;
                logd("<%s>", ic_card.name);
                len += _memcpy_len(&card[8], (u8 *)ic_card.name, 12);                   //驾驶员名字
                len += _memcpy_len(&card[20], (u8 *)ic_card.certificate, 20);           //从业资格证编码
                len += _memcpy_len(&card[40], (u8 *)ic_card.license_organ, 30);         //发证机构名称
                len += _memcpy_len(&card[70], (u8 *)ic_card.license_organ_validity, 4); //证件有效期
                len += _memcpy_len(&card[74], (u8 *)ic_card.id_card, 20);               //司机身份证号
            } else {
                card[7] = 0x03;
                len += 1;
            }

#endif
        } else {
            card[len++] = 0x02;                                      //海格定制版
            len += _memcpy_len(&card[1], (u8 *)&IC_Time.IC_exit, 6); //拔卡时间
        }

        //IC-card://司机插拔卡信息【MCU->RK】

        tr9_frame_pack2rk(tr9_cmd_5013, card, len);
    }
}

void mcu_ask_tr9_www_status(void) {
    u32 tmr = 0;

    if (pub_io.b.dvr_open == false)
        return;

    if (reset_hi3520_open == false) //debug
        return;

    if (s_Hi3520_Monitor.start == false)
        return;

    if (font_lib_update.start_m)
        return;

    if (_covern(tmr) > 300) {
        tmr = tick;
        //tr9_frame_pack2rk(0x6020, 0, 0);
    }
}
