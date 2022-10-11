/**
  ******************************************************************************
  * @file    handset.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  调度屏
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

uc8 apn_cmnet[] = "CMNET"; //中国移动公网 APN 5

uc8 hd_set_ok[] = {0x07, 0x24, 0xb1, 0x03, 0x00, 0x96, 0x0a}; //返回参数设置成功或失败结果
uc8 hd_msg_ok[] = {0x06, 0x24, 0xa4, 0x02, 0x82, 0x0a};       //发送短信成功时返回该指令
uc8 hd_phone_on[] = {0x06, 0x24, 0xa7, 0x02, 0x81, 0x0a};     //电话接通时发送该指令到手柄
uc8 HAnsData[] = {0x06, 0x24, 0x0d, 0x02, 0x2b, 0x0a};        //正确接收到手柄数据应答数据
#if (P_SOUND == SOUND_BASE)
uc8 hd_phone_off[] = {0x06, 0x24, 0xa6, 0x02, 0x80, 0x0a};         //当呼叫结束或通话结束返回该信令到手柄
uc8 hd_limit_phone[] = {0x07, 0x24, 0xbc, 0x03, 0x01, 0x9a, 0x0a}; //电话限拨指令
#endif

bool ic_driver_lisence = false; //读到司机身份证号
u8 test_speed = 0;
S_Self_Test self_test; //自检

/****************************************************************************
* ????????:    init_self_test ()
* ????????????????????
* ????????????????????
* ??????????????????????
****************************************************************************/
void init_self_test(void) {
    _memset((u8 *)&self_test, 0x00, sizeof(self_test));
    self_test.uGprsU = '0';
    self_test.uGsmU = '0';
    self_test.uGpsU = '0';
    self_test.uMainBoardU = '0';
    self_test.uSimCardU = '0';
}

/*****************************************************
  函数名: 	   driver_login_in_out
  描  述:	   驾驶人登退签使用函数；赋值驾驶证号
  调  用:
  输  入:		   从业资格证号buf == (u8*)&mix.ic_driver;   len = 20
				  登退签标志
  输  出:
  返  回:       登签时，不是同一张卡返回TRUE , 是同张卡返回FALSE
  注  意:	   登退签事件时调用
******************************************************/
static bool driver_login_in_out(u8 *buf, u8 len, bool login) {
    static u8 id_bak[18];
    u8 ic_card[18];

    if (buf == NULL)
        return false;

    _memset(ic_card, 0x00, 18);
    _memcpy(ic_card, buf, 18);

    if (login == true && (ic_card[0] != 0) && (_strncmp(id_bak, ic_card, 18) != 0)) {
        _memcpy(id_bak, buf, 20);
        _memcpy((u8 *)&mix.ic_driver, buf, 18);
        return true;
    } else {
        return false;
    }
}

//01 3A 32 30 32 2E 31 30 36 2E 31 31 39 2E 32 34 39 2C 33 33 33 33
void set_car_mess_to_tr9(u8 tp, u8 *str, u8 slen) {
    u8 buff[60] = {0};
    u8 *pf;
    u8 len = 0;
    //    u16 tmp;

    pf = buff;
    *pf++ = tp;
    len = 1;

    _memcpy(pf, str, slen);
    len += slen;

    tr9_frame_pack2rk(0x6023, buff, len);
}

/****************************************************************************
* 名称:    send_handset_data ()
* 功能：发送手柄数据
* 入口参数：无
* 出口参数：无
****************************************************************************/
void send_handset_data(u8 from_t, u8 type_t, u8 resend_t, u8 *buf_t, u16 b_len_t) {
    u16 len_t = 0;

    if (type_t == H_LEN_I || type_t >= H_LEN_E || buf_t == NULL)
        return;

    if (type_t == H_LEN_N) {
        len_t = (*buf_t++) - 1;
    }

    if (type_t == H_LEN_W) {
        len_t = b_len_t;
    }

    if (len_t < 5 || len_t > 1200)
        return;
    if (from_t == FROM_INNER) //接收来自从机的数据
    {
        slave_recive_msg((u8 *)buf_t, len_t);
        return;
    }

    if (from_t == FROM_U1 || from_t == FROM_ALL) {
        if (u1_m.cur.b.handset == false)
            return;
        cli();
        write_uart1((u8 *)buf_t, len_t);
        sei();
    }

    if (from_t == FROM_U4 || from_t == FROM_ALL) {
        if (u4_m.cur.b.handset == false)
            return;
        cli();
        write_uart4((u8 *)buf_t, len_t);
        sei();
    }
}

/*********************************************************
名    称：handle_send_base_msg
功    能：发送基本命令字信息, type 不是0xee
输入参数：无
输    出：无
编写日期：2011-03-15
**********************************************************/
void handle_send_base_msg(u8 from_t, u8 id_t, u8 *str_t, u16 len_t) {
    u8 buf[256] = {0x00};
    u16 i = 0;
    u16 b_len;
    u8 xor ;

    if (str_t == NULL)
        return;
    if (len_t > 220)
        len_t = 220;
    xor = 0;
    b_len = 0;
    buf[b_len++] = 0x24;
    buf[b_len++] = id_t;
    buf[b_len++] = len_t + 2;
    b_len += _memcpy_len(&buf[b_len], str_t, len_t);
    for (i = 0; i < b_len; i++)
        xor ^= buf[i];
    buf[b_len++] = xor;
    buf[b_len++] = 0x0a;

    send_handset_data(from_t, H_LEN_W, 0, (u8 *)&buf, b_len);
}

/*********************************************************
名    称：handle_send_ext_msg
功    能：发送扩展命令字, cmd 为0xee
输入参数：无
输    出：无
编写日期：新国标
**********************************************************/
void handle_send_ext_msg(u8 from_t, u16 id_t, u8 *str_t, u16 len_t) {
    u16 i;
    u8 buf[800] = {0x00};
    u8 xor ;
    u16 tmp = 0;
    u16 b_len = 0;

    if (str_t == NULL || len_t > 1200)
        return;

    tmp = len_t + 4;
    xor = 0;
    b_len = 0;
    buf[b_len++] = 0x24;
    buf[b_len++] = 0xee;
    buf[b_len++] = tmp >> 8;
    buf[b_len++] = tmp & 0xff;
    buf[b_len++] = id_t >> 8;
    buf[b_len++] = id_t & 0xff;
    b_len += _memcpy_len(&buf[b_len], str_t, len_t);
    for (i = 0; i < b_len; i++)
        xor ^= buf[i];
    buf[b_len++] = xor;
    buf[b_len++] = 0x0a;

    send_handset_data(from_t, H_LEN_W, 0, (u8 *)&buf, b_len);
}

/*********************************************************
名    称：handle_down_sms
功    能：下发手机短信到手柄
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
void handle_down_sms(u8 from_t, u8 type_t, u8 *phone_t, u16 p_len, u8 *str_t, u16 s_len_t) {
#if (P_PDU == SMS_PDU_BASE)
    u8 buf[500];
    u8 t_buf[32];
    u8 t_len;
    u16 b_len = 0;
    u8 id;

    if (p_len > 18 || s_len_t > 400)
        return;

    if (s_len_t > 200) //协议只能支持256 个字节
        s_len_t = 200;

    if (type_t == SMS_PDU)
        id = 0xaf;
    else if (type_t == SMS_TXT)
        id = 0xb0;
    else
        return;

    _memset(t_buf, 0x00, 32);
    _sprintf_len((char *)&t_buf, "%02d-%02d-%02d %02d:%02d:%02d", sys_time.year, sys_time.month, sys_time.date, sys_time.hour, sys_time.min, sys_time.sec);
    t_len = _strlen(t_buf);

    b_len = 0;
    buf[b_len++] = '"'; //加载手机号码
    b_len += _memcpy_len(&buf[b_len], phone_t, p_len);
    buf[b_len++] = '"';

    buf[b_len++] = '"'; //加载时间信息
    b_len += _memcpy_len(&buf[b_len], t_buf, t_len);
    buf[b_len++] = '"';

    buf[b_len++] = '"'; //加载信息内容
    b_len += _memcpy_len(&buf[b_len], str_t, s_len_t);
    buf[b_len++] = '"';

    handle_send_base_msg(from_t, id, buf, b_len);
#endif
}

/****************************************************************************
* 名称:    updata_handset_trwy_date ()
* 功能：手柄数据格式转换
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void updata_handset_trwy_date(u8 *trwy, MIX_GPS_DATA *m) {
    point_t tr_locate;
    u32 dgr;
    u32 min;
    u16 speed;

    dgr = (m->locate.lati & 0x7fffffff) / 1000000;
    min = (m->locate.lati & 0x7fffffff) % 1000000;
    min = (min * 60) / 100;
    tr_locate.lati = dgr * 1000000 + min;
    tr_locate.lati /= 10;

    dgr = (m->locate.lngi & 0x7fffffff) / 1000000;
    min = (m->locate.lngi & 0x7fffffff) % 1000000;
    min = (min * 60) / 100;
    tr_locate.lngi = dgr * 1000000 + min;
    tr_locate.lngi /= 10;

    //兼容企标信息
    trwy[0] = m->time.year;
    trwy[1] = m->time.month;
    trwy[2] = m->time.date;
    trwy[3] = m->time.hour;
    trwy[4] = m->time.min;
    trwy[5] = m->time.sec;

#if 0	
    trwy[6] = m->locate.lati < 0  ? 0x80 : 0x00;
    trwy[6] |= _bintobcd((tr_locate.lati / 1000000) % 100);
    trwy[7] = _bintobcd((tr_locate.lati / 10000) % 100);
    trwy[8] = _bintobcd((tr_locate.lati / 100) % 100);
    trwy[9] = _bintobcd(tr_locate.lati % 100);
    trwy[10] = m->locate.lngi < 0  ? 0x80 : 0x00;
    trwy[10] |= _bintobcd((tr_locate.lngi / 1000000) % 100);
    trwy[11] = _bintobcd((tr_locate.lngi / 10000) % 100);
    trwy[12] = _bintobcd((tr_locate.lngi / 100) % 100);
    trwy[13] = _bintobcd(tr_locate.lngi % 100);
#endif
#if (P_CAN == _CAN_BASE)
    //speed = mix.pwr_vol;
    speed = can_limit_rabio / 8;
#else
    speed = 1600;
#endif
    trwy[6] = 0;
    trwy[7] = _bintobcd((speed / 10000) % 100);
    trwy[8] = _bintobcd((speed / 100) % 100);
    trwy[9] = _bintobcd(speed % 100);

    trwy[10] = 0;
    trwy[11] = _bintobcd((average_1km / 10000) % 100);
    trwy[12] = _bintobcd((average_1km / 100));
    trwy[13] = _bintobcd(average_1km % 100);

    //engine_speed
    {
        trwy[14] = _bintobcd(m->complex_speed / 100);
        trwy[15] = _bintobcd(m->complex_speed % 100);
    }

    trwy[16] = _bintobcd(m->direction / 100);
    trwy[17] = _bintobcd(m->direction % 100);
    if (mix.fix_num > 63)
        mix.fix_num = 63;                                               //2016
    trwy[18] = m->fixed == true ? ((mix.fix_num & 0x3f) | 0x80) : 0x00; //低四位表示收星数
    trwy[19] = m->dist >> 24;
    trwy[20] = m->dist >> 16;
    trwy[21] = m->dist >> 8;
    trwy[22] = m->dist;
}

/*********************************************************
名    称：Handle_Send_0xa0_Handle
功    能：间隔1秒发送定位数据到调度屏或手柄
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
void handle_send_0xa0_info(u8 from_t) {
    u8 buf[100] = {0x00};
    u16 b_len;
    u8 flag;

    u8 trwy_buf[32];

    update_mix_infomation();
    updata_handset_trwy_date(trwy_buf, &mix);

    flag = 0x10; //中国移动
    flag = (gc.gprs < NET_ONLINE) ? flag : (flag | 0x01);
    flag = (car_alarm.bit.speed_over) ? (flag | 0x80) : flag;

    b_len = 0;
    b_len += _memcpy_len(&buf[b_len], &trwy_buf[3], 16); //从时分秒开始
    buf[b_len++] = flag;
    buf[b_len++] = alarm_cfg.speed_over_km; //超速报警限速值
    buf[b_len++] = mix.pwr_vol >> 8;        //当前输入电源电压 高位
    buf[b_len++] = mix.pwr_vol & 0x00ff;    //当前输入电源电压 低位
    if (car_state.bit.acc)
        buf[b_len++] = 28; //GSM网络信号值
    else
        buf[b_len++] = mix.csq; //GSM网络信号值
    buf[b_len++] = (car_state.bit.acc) ? 0x01 : 0x00;
    buf[b_len++] = (rcmd.b.cut_oil) ? 0x01 : 0x00;
    b_len += _memcpy_len(&buf[b_len], &trwy_buf[0], 3);
    //engine_speed
    buf[b_len++] = bintohex((car_weight % 10000) / 100); //当前油量电阻值 高位_bcdtobin
    buf[b_len++] = bintohex(car_weight % 100);           //当前油量电阻值 低位  _bcdtobin

    buf[b_len++] = bd_rcd_io.reg; //记录仪状态

    handle_send_base_msg(from_t, 0xa0, buf, b_len);
}

#if 1
/*********************************************************
名    称：Handle_Send_0xae_Handle
功    能：返回主机当前网络参数值
输入参数：无
输    出：无
编写日期：2011-03-10
**********************************************************/
static void analyse_0x11_handle(u8 from, u8 *str, u16 len) {
    u8 send_hand[150] = {0x00};
    u8 i, y;
    u16 tmp;
    u8 *buf;
    u8 len1 = 0;
    u8 len_t = 0;
    u8 *p;

    _memset(send_hand, 0, 150);
    send_hand[1] = 0x24;
    send_hand[2] = 0xae;
    send_hand[3] = 0;
    send_hand[4] = 'M';

    for (i = 0; i < 12; i++) //协议问题，只能显示11位，终端去掉第一位0
        send_hand[i + 5] = server_cfg.terminal_id[i];

    send_hand[17] = 'C';

    p = &send_hand[18];
    len1 = _strlen((u8 *)&phone_cfg.sms);
    for (i = 0; i < phone_cfg.sms[PHONELEN - 1]; i++) //( phone_cfg.sms[PHONELEN-1] < 5 )
    {
        if (i < len1)
            *p = phone_cfg.sms[i];
        else
            *p = 0x20;
        p++;
    }

    *p++ = 'I';

    if ((server_cfg.select_tcp & 0x01) == 0x01)
        *p++ = '1';
    else
        *p++ = '0';
    buf = p;
    *buf++ = '"';
    len_t = _strlen((u8 *)&server_cfg.main_ip);
    len_t = (len_t > 15) ? 15 : len_t;
    for (i = 0; i < 15; i++) {
        if (i < len_t)
            *buf++ = server_cfg.main_ip[i];
        else
            break;
    }
    *buf++ = '"';
    *buf++ = ',';

    tmp = server_cfg.main_port;
    y = AsctoBCD((u16 *)&tmp, buf, 5, 1);
    buf += y;

    *buf++ = 'U';

    len1 = _strlen((u8 *)&phone_cfg.user);
    for (i = 0; i < 13; i++) {
        if (i < len1)
            *buf++ = phone_cfg.user[i];
        else
            *buf++ = 0x20;
    }

    *buf++ = 0x80;

    *buf++ = (u8)(report_cfg.dft_time / 256);
    *buf++ = (u8)(report_cfg.dft_time % 256);
    *buf++ = car_alarm.bit.cut_volt ? 0x01 : 0x00;
    *buf++ = alarm_cfg.speed_over_km;
    *buf++ = alarm_cfg.stoping_time;

    *buf++ = (u8)(mix.pwr_vol / 256); //电源电压
    *buf++ = (u8)(mix.pwr_vol % 256);
    *buf++ = (u8)(mix.ad_oil / 256); //油量电阻值
    *buf++ = (u8)(mix.ad_oil % 256);

    *buf++ = 0x8c; //版本
    *buf++ = 0x00;
    *buf++ = 0x00;
    *buf++ = 0x00;
    *buf++ = 0x00;
    *buf++ = 0x10;

    *buf++ = '7';

    len_t = _strlen((u8 *)&server_cfg.apn);
    len_t = (len_t >= 10) ? 10 : len_t;
    for (i = 0; i < 10; i++) {
        if (i < len_t)
            *buf++ = server_cfg.apn[i];
        else
            *buf++ = 0x20;
    }
    *buf++ = 'e'; //SIM1的APN
    *buf++ = '1'; //SIM2的APN

    *buf++ = (server_cfg.select_tcp & 0x02) ? '1' : '0';
    *buf++ = '"';

    len_t = _strlen((u8 *)&server_cfg.bak_ip);
    len_t = (len_t > 15) ? 15 : len_t;
    for (i = 0; i < 15; i++) {
        if (i < len_t)
            *buf++ = server_cfg.bak_ip[i];
        else
            *buf++ = 0x20;
    }
    *buf++ = '"';
    *buf++ = ',';

    tmp = server_cfg.bak_port;
    y = AsctoBCD((u16 *)&tmp, buf, 5, 1);
    buf += y;
    *buf++ = 'K';
    //sHi3520Ver
    //_strncpy(buf, sHi3520Ver, 9 );
    _strncpy(buf, TR9_SOFT_VERSION_APP, 14);
    loge("len err, 14");
    buf += 24;

    y = buf - &send_hand[0];

    send_hand[3] = y - 1; //包长
    send_hand[0] = y + 3; //数据总长度

    *buf++ = 0;

    for (i = 1; i < y; i++) {
        *buf ^= send_hand[i];
    }
    
    buf++;
    *buf = 0x0A;

    send_handset_data(from, H_LEN_N, 0, (u8 *)&send_hand, 0);
}
#endif

/*********************************************************
名    称：Analyse_0x01_SendMsg
功    能：解析用户发送的调度短信指令
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x01_send_msg(u8 from, u8 *str, u16 len) {
    u8 msg[HANDSET_MAX_SIZE] = {0x00};
    u16 m_len;
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u16 data_len;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);
    data_len = hc_buf[2] - 2;
    if (data_len < 2 || data_len > 250)
        return;

    m_len = 0;
    msg[m_len++] = 0x02; //调度屏标志位
    m_len += _memcpy_len(&msg[m_len], &hc_buf[3], data_len);

    pack_any_data(0x0300, msg, m_len, PK_NET, LINK_IP0 | LINK_IP1 | LINK_IP2);
}

/*********************************************************
名    称：Analyse_0x01_SendMsg
功    能：解析用户发送的手机短信指令
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x02_send_sms(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 buf[32];
    u16 pdu_len = 0;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    _memset(buf, 0x00, 32);
    _memcpy(buf, (u8 *)&hc_buf[3], 3);

    pdu_len = atoi((char *)&buf) + 1;
    if (pdu_len < 16 || pdu_len > 160)
        return;

    sms_send(SMS_PDU, NULL, 0, &hc_buf[6], pdu_len);
    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_msg_ok, 0);
}

//01 3A 32 30 32 2E 31 30 36 2E 31 31 39 2E 32 34 39 2C 33 33 33 33
void set_main_ip_to_tr9(bool mb) {
    u8 buff[60];
    u8 *pf;
    u8 len = 0;
    u8 i = 0;
    u16 tmp;

    pf = buff;
    if (mb)
        *pf++ = 01;
    else
        *pf++ = 02;
    *pf++ = ':';

    if (mb) {
        _strcpy_len(pf, (u8 *)&server_cfg.main_ip);
        len = _strlen((u8 *)&server_cfg.main_ip);
    } else {
        _strcpy_len(pf, (u8 *)&server_cfg.bak_ip);
        len = _strlen((u8 *)&server_cfg.bak_ip);
    }
    pf += len;
    *pf++ = ',';
    len += 2;

    if (mb)
        tmp = server_cfg.main_port;
    else
        tmp = server_cfg.bak_port;

    i = AsctoBCD((u16 *)&tmp, pf, 5, 1);
    pf += i;
    len += i;

    tr9_frame_pack2rk(0x6013, buff, pf - buff);
}

/*********************************************************
名    称：Analyse_0x03_SetIp
功    能：解析设置IP地址指令数据
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x03_set_ip(u8 from, u8 *str, u16 len) {
    char *tok;
    u8 hc_buf[HANDSET_MAX_SIZE];
    u8 buf[64] = {0};
    u16 b_len = 0;
    u8 *pf;
    u16 tmp;
    u8 i;
    if (str == NULL || len > HANDSET_MAX_SIZE) {
    set_03_ip_err:
        return;
    }

    _memset(hc_buf, 0x00, HANDSET_MAX_SIZE);
    _memset(buf, 0x00, 64);
    _memcpy(hc_buf, str, len);

    pf = buf;
    if (hc_buf[2] == 0x02) //查询状态
    {
        b_len = 0;
        *pf++ = '<';
        if (server_cfg.select_tcp & 0x01 == 0x01)
            *pf++ = '1';
        else
            *pf++ = '0';
        *pf++ = ',';
        *pf++ = '"';
        _strcpy_len(pf, (u8 *)&server_cfg.main_ip);
        b_len = _strlen((u8 *)&server_cfg.main_ip);
        pf += b_len;
        *pf++ = '"';
        *pf++ = ',';

        tmp = server_cfg.main_port;
        i = AsctoBCD((u16 *)&tmp, pf, 5, 1);
        pf += i;
        *pf++ = '>';
        b_len += i;
        b_len += 7;
        handle_send_base_msg(from, 0x03, buf, b_len);
        return;
    }
    //if (sms_phone_save)
    //    return;

    if (hc_buf[4] == 0x30) //主端口
        server_cfg.select_tcp &= 0xfe;
    else
        server_cfg.select_tcp |= 0x01;

    if ((tok = strtok((char *)&hc_buf, "\"")) == NULL) ///",\""
        goto set_03_ip_err;

    if ((tok = strtok(NULL, "\"")) != NULL) {
        strcpy((char *)&buf, tok);
        b_len = _strlen(buf);
        if (buf[b_len - 1] < 0x30)
            b_len--;
        if (b_len > 23)
            goto set_03_ip_err;

        _memset((u8 *)&server_cfg.main_ip, 0x00, 24);
        _memcpy((u8 *)&server_cfg.main_ip, &hc_buf[7], b_len);
    } else
        goto set_03_ip_err;

    if ((tok = strtok((char *)&hc_buf[b_len + 9], ">")) != NULL) {
        strcpy((char *)&buf, tok);
        b_len = _strlen(buf);
        if (b_len < 2 || b_len > 5)
            goto set_03_ip_err;
        server_cfg.main_port = atoi((const char *)&buf);
    } else
        goto set_03_ip_err;

    set_main_ip_to_tr9(true);
    menu_report.ok_flag = true; //收到设置成功应答标志
    menu_cnt.menu_flag = true;

    lm.reset = true;
    flash_write_import_parameters();
    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
#if (P_RCD == RCD_BASE)
    rcd_para_inf.en = true;
    rcd_para_inf.type = 0xcc;
#endif
}

//01 3A 32 30 32 2E 31 30 36 2E 30 31 31 39 2E 32 34 39 2C 33 33 33 33
void set_gps_id_to_tr9(void) {
    u8 buff[60];
    u8 *pf;

    _memset(buff, 0, 60);
    pf = buff;
    *pf++ = 03;
    *pf++ = ':';

    _memcpy(pf, (u8 *)&server_cfg.terminal_id[1], 11);
    buff[13] = 0;

    tr9_frame_pack2rk(0x6013, buff, 13);
}

/*********************************************************
名    称：Analyse_0x04_SetId
功    能：解析设置本机号码指令
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x04_set_id(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 buf[64] = {0};
    u16 b_len = 0;
    u8 *pf;
    u8 i;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (hc_buf[2] == 0x02) //查询状态
    {
        pf = buf;
        b_len = 0;
        *pf += 4;

        _strcpy_len(pf, (u8 *)&server_cfg.terminal_id);

        b_len += 16;
        handle_send_base_msg(from, 0x04, buf, b_len);
        return;
    }
    if (/*sms_phone_save  ||*/ (len > 21))
        return;
    _memset((u8 *)&server_cfg.terminal_id, 0x00, 21);

    len -= 9;
    if (len < 12) {
        for (i = 0; i < 12 - len; i++)
            server_cfg.terminal_id[i] = 0x30;
        _memcpy((u8 *)&server_cfg.terminal_id + i, &hc_buf[7], len);
    } else
        _memcpy((u8 *)&server_cfg.terminal_id, (u8 *)&hc_buf[7], 12);

    //兼容中交平台
    _memset((u8 *)&register_cfg.terminal_id, 0x00, 30);
    _memcpy((u8 *)&register_cfg.terminal_id, (u8 *)&server_cfg.terminal_id[5], 7); //2015-

    lm.reset = true;

    init_all_net_info();

    flash_write_import_parameters();
    menu_report.ok_flag = true; //收到设置成功应答标志
    menu_cnt.menu_flag = true;

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    set_gps_id_to_tr9();

#if (P_RCD == RCD_BASE)
    rcd_para_inf.en = true;
    rcd_para_inf.type = 0xcd;
#endif
}

/*********************************************************
名    称：Analyse_0x05_SetSMS
功    能：解析设置短信服务号码指令
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x05_set_sms(u8 from, u8 *str, u16 len) {
    char *tok;
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 phone[PHONELEN];
    u16 p_len = 0;
    u8 buf[64] = {0};
    u8 *pf;

    if (str == NULL || len > HANDSET_MAX_SIZE){
        return;
    }

    _memcpy(hc_buf, str, len);

    if (hc_buf[2] == 0x02) { //查询状态
        pf = buf;
        *pf = 0x3c;
        p_len = 0;
        *pf += 1;

        p_len = phone_cfg.sms[PHONELEN - 1];
        _strcpy_len(pf, (u8 *)&phone_cfg.sms);
        pf += p_len;
        p_len += 1;
        handle_send_base_msg(from, 0x05, buf, p_len);
        return;
    }

    // if (sms_phone_save)
    //     return;
    if ((tok = strtok((char *)&hc_buf[4], ">")) != NULL) {
        _strcpy(phone, (u8 *)tok);
        p_len = _strlen(phone);
        if (p_len < 5 || p_len >= (PHONELEN - 1))
            return;

        phone[p_len] = '\0';
        _memset((u8 *)&phone_cfg.sms, 0x00, PHONELEN);
        _strcpy((u8 *)&phone_cfg.sms, phone);
        phone_cfg.sms[PHONELEN - 1] = p_len;
        flash_write_normal_parameters();
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        sms_ip_if_ok();
    }
}

/*********************************************************
????    ??????analyse_0x0c_self_test
????    ????????????
??????????????????????
????    ????????????
????д??????????2013-12-1
**********************************************************/
static void analyse_0x0c_self_test(u8 from, u8 *str, u16 len) {
    handle_send_base_msg(from, 0xa8, (u8 *)&self_test, sizeof(self_test) - 1);
}

/*********************************************************
名    称：Analyse_0x14_SetApn
功    能：解析设置Apn指令
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x14_set_apn(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    // if (sms_phone_save)
    //     return;
    _memcpy(hc_buf, str, len);

    _memset((u8 *)&server_cfg.apn, 0x00, 24);
    switch (hc_buf[4]) {
    case 0x31:
        _strcpy((u8 *)&server_cfg.apn, apn_cmnet);
        break;
    default:
        _strcpy((u8 *)&server_cfg.apn, apn_cmnet);
        break;
    }

    lm.reset = true;
    menu_report.ok_flag = true; //收到设置成功应答标志
    menu_cnt.menu_flag = true;

    flash_write_import_parameters();

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/*********************************************************
名    称：Analyse_0x15_Initialization
功    能：模块初始化
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x15_initialization(u8 from, u8 *str, u16 len) {
    static u32 into_jiff = 0;

    if (_pastn(into_jiff) < 1500) {
        return;
    }
    // if (sms_phone_save)
    //    return;

    into_jiff = jiffies;

    reset_system_parameter(R_F_ALL); //恢复出厂默认

    flash_write_import_parameters();

    flash_write_normal_parameters();
#if (P_RCD == RCD_BASE)
    init_mfg_parameter();
#endif
    menu_report.ok_flag = true; //收到设置成功应答标志
    menu_cnt.menu_flag = true;

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

#if (P_RCD == RCD_BASE)
static void analyse_0x19_Set_speed_plus(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[128];
    u16 set_plus;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        msg[0] = register_cfg.blank_car_weight >> 8;
        msg[1] = register_cfg.blank_car_weight;

        handle_send_base_msg(from, 0x19, msg, 2);
    } else if (len == 7) //设置
    {
        set_plus = 0;
        set_plus = hc_buf[3];
        set_plus <<= 8;
        set_plus += hc_buf[4];
        if (set_plus > 50000)
            return;
        factory_para.swit = 0x00;
        if ((set_plus <= 19990)) {
            register_cfg.blank_car_weight = set_plus;
            flash_write_import_parameters();
            flash_write_normal_parameters();
        }

        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/*********************************************************
名    称：Analyse_0x1d_car_number
功    能：车牌号码
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x22_print(u8 from, u8 *str, u16 len) {
    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    if (!mix.moving) {
        if (car_cfg.coefficient[0] == 0x02) {
            print12_rcd_infomation();
        } else {
            print03_rcd_infomation();
        }
    }
}
#endif

/****************************************************************************
* 名称:    analyse_0x1c_set_car_vin ()
* 功能：车辆VIN号码
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x1c_set_car_vin(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[128];

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        _memcpy(msg, (u8 *)&car_cfg.vin, 17);
        handle_send_base_msg(from, 0x1c, msg, 17);
    } else if (len > 5 && len <= 22) {
        //设置
        _memset((u8 *)&car_cfg.vin, 0, 18);
        _memcpy((u8 *)&car_cfg.vin, &hc_buf[3], len - 5);

        lm.reset = true;

        flash_write_import_parameters();

        rcd_para_inf.en = true;
        rcd_para_inf.type = 0xCB;
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/*********************************************************
名    称：Analyse_0x1d_car_number
功    能：车牌号码
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x1d_car_number(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[32];
    u8 len_t;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        _memcpy(msg, (u8 *)&car_cfg.license, 10);
        _memcpy((recorder_para.car_plate + 2), (u8 *)&car_cfg.license, 9); //新加
        handle_send_base_msg(from, 0x1d, msg, 10);
    } else if (len > 5 && len <= 15) //设置
    {
        len_t = len - 5;
        if (len_t > 9)
            len_t = 9;

        _memset((u8 *)&car_cfg.license, 0x00, 12);
        _memcpy((u8 *)&car_cfg.license, &hc_buf[3], len_t);
        car_cfg.license[11] = '\0';
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        set_car_mess_to_tr9(0x02, (u8 *)&car_cfg.license, len_t);
        lm.reset = true;

        rcd_para_inf.en = true;
        rcd_para_inf.type = 0xCB;

        flash_write_import_parameters();

    } else {
        //错误
    }
}

/*********************************************************
名    称：analyse_0x1e_car_type
功    能：车牌分类
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x1e_car_type(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[32];
    u8 len_t;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) { //查询
        len_t = _strlen((u8 *)&car_cfg.type);
        _memcpy(msg, (u8 *)&car_cfg.type, len_t);
        handle_send_base_msg(from, 0x1e, msg, len_t);
    } else if (len > 5 && len <= 15) { //设置
        len_t = len - 5;
        _memset((u8 *)&car_cfg.type, 0x00, 10);
        _memcpy((u8 *)&car_cfg.type, &hc_buf[3], len_t);
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);

        flash_write_import_parameters();
    }
}

/*********************************************************
 名    称：analyse_0x1f_driver_code
 功    能：
 输入参数：无
 输    出：无
 编写日期：2013-12-1
 **********************************************************/
void analyse_0x1f_driver_code(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[32];
    u8 i;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        _memcpy(msg, (u8 *)&driver_cfg.driver_code, 6);
        handle_send_base_msg(from, 0x1f, msg, 6);
    } else if (len > 5 && len <= 20) {
        len = len;
        _memset((u8 *)&driver_cfg.driver_code, 0, 6);
        for (i = 0; i < len; i++) {
            if (hc_buf[0] == 0x24)
                driver_cfg.driver_code[i] = hc_buf[i + 3]; //_memcpy((u8*)&driver_cfg.qualification, buf,  18 );
            else
                driver_cfg.driver_code[i] = hc_buf[i];
        }
        flash_write_normal_parameters();

        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/*********************************************************
名    称：analyse_0x20_driver_qualification
功    能：车牌号码
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
void analyse_0x20_driver_qualification(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[32];
    u8 len_t;
    bool icc = false;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        _memcpy(msg, (u8 *)&driver_cfg.license, 18);
        handle_send_base_msg(from, 0x20, msg, 18);
    } else if (len > 8 && len <= 25) //设置
    {
        len_t = len;
        if (len_t > 18)
            len_t = 18;

        _memset((u8 *)&driver_cfg.license, 0x00, 20);
        if (hc_buf[0] == 0x24)
            _memcpy((u8 *)&driver_cfg.license, &hc_buf[3], 18);
        else
            _memcpy((u8 *)&driver_cfg.license, &hc_buf[0], 18);

        _memset((u8 *)&mix.ic_driver, 0, 18);
        _memcpy((u8 *)&mix.ic_driver, (u8 *)&driver_cfg.license, 18);

        if (state_data.state.flag.ic_flag)
            icc = true;

        mix.ic_exchange = driver_login_in_out(&hc_buf[0], 20, icc);

        ic_driver_lisence = true;
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        flash_write_normal_parameters();
    } else {
        //错误
    }
}

/*********************************************************
名    称：analyse_0x21_speed_type
功    能：速度模式
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x21_speed_type(u8 from, u8 *str, u16 len) {
    u8 speed_type_t = 0;

    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[32];

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) {
#if (P_RCD == RCD_BASE)
        speed_type_t = (factory_para.speed_type == 0x01) ? 0x01 : 0x02;
#else
        speed_type_t = 0x02;
#endif
        if (car_alarm.bit.vss_err)
            speed_type_t = 0x02;
        else
            speed_type_t = 0x01;
        //查询
        msg[0] = speed_type_t;
        handle_send_base_msg(from, 0x21, msg, 1);
    } else if (len > 5) {
#if (P_RCD == RCD_BASE)
        speed_type_t = hc_buf[3];
        factory_para.speed_type = (speed_type_t == 0x01) ? 0x01 : 0x02;
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        write_mfg_parameter();
#endif
    } else {
        //错误
    }
}

/*********************************************************
名    称：Analyse_0x26_car_class
功    能：车牌颜色
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x26_car_class(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[32];

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);
    if (len == 5) {
        //查询
        msg[0] = register_cfg.color;
        handle_send_base_msg(from, 0x26, msg, 1);
    } else if (len > 5 && len <= 15) {
        //设置
        if (hc_buf[3] > 9)
            return;

        register_cfg.color = hc_buf[3];
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);

        lm.reset = true;

        flash_write_import_parameters();
    } else {
        //错误
    }
}

/*********************************************************
名    称：Analyse_0x24_SetApn
功    能：解析设置Apn指令
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x24_set_apn(u8 from, u8 *str, u16 len) {
    u8 len_t = 0;
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) {
        _memset(hc_buf, 0, 30);
        hc_buf[0] = 0x01;
        len_t = _strlen((u8 *)&server_cfg.apn);
        _memcpy(hc_buf + 1, (u8 *)&server_cfg.apn, len_t);
        handle_send_base_msg(from, 0x24, hc_buf, len_t + 1);
        return;
    }
    //if (sms_phone_save)
    //    return;
    len_t = hc_buf[2] - 3;
    if (len_t < 2 || len_t > 23)
        return;

    _memset((u8 *)&server_cfg.apn, 0x00, 24);
    _memcpy((u8 *)&server_cfg.apn, (u8 *)&hc_buf[4], len_t);

    lm.reset = true;

    flash_write_import_parameters();

    menu_report.ok_flag = true; //收到设置成功应答标志
    menu_cnt.menu_flag = true;

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/*********************************************************
名    称：analyse_0x25_any_uart
功    能：任意串口定义
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
//扩展多路串口usart0:串口功能设置
static void analyse_0x25_any_uart(u8 from, u8 *str, u16 len) {
    u8 s_len;
    u8 ret;
    uart_func_list t2;
    u8 source;
    u32 tmp;

    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[HANDSET_MAX_SIZE] = {0x00};
    u16 m_len;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    s_len = hc_buf[3] * 9 + 1;
    if (hc_buf[2] == 0x02) //长度判断，数据空为查询
    {
        m_len = 0;
        msg[m_len++] = 0x02;
        msg[m_len++] = FROM_U1;
        tmp = get_uart1_func();
        msg[m_len++] = (tmp >> 24) & 0xff;
        msg[m_len++] = (tmp >> 16) & 0xff;
        msg[m_len++] = (tmp >> 8) & 0xff;
        msg[m_len++] = tmp & 0xff;
        msg[m_len++] = (u1_m.bak.reg >> 24) & 0xff;
        msg[m_len++] = (u1_m.bak.reg >> 16) & 0xff;
        msg[m_len++] = (u1_m.bak.reg >> 8) & 0xff;
        msg[m_len++] = u1_m.bak.reg & 0xff;

        msg[m_len++] = FROM_U4;
        tmp = get_uart4_func();
        msg[m_len++] = (tmp >> 24) & 0xff;
        msg[m_len++] = (tmp >> 16) & 0xff;
        msg[m_len++] = (tmp >> 8) & 0xff;
        msg[m_len++] = tmp & 0xff;
        msg[m_len++] = (u4_m.bak.reg >> 24) & 0xff;
        msg[m_len++] = (u4_m.bak.reg >> 16) & 0xff;
        msg[m_len++] = (u4_m.bak.reg >> 8) & 0xff;
        msg[m_len++] = u4_m.bak.reg & 0xff;

        handle_send_base_msg(from, 0x23, msg, m_len);
    } else if (hc_buf[2] == s_len + 2) {
        source = hc_buf[4]; //得到设置串口号

        t2.reg = hc_buf[9];
        t2.reg <<= 8;
        t2.reg |= hc_buf[10];
        t2.reg <<= 8;
        t2.reg |= hc_buf[11];
        t2.reg <<= 8;
        t2.reg |= hc_buf[12];
        ret = verify_any_uart_func(source, t2.reg);
        if (ret != 0xFF) {
            if (source == FROM_U1) {
                us.u1.reg = t2.reg;
                //扩展多路串口usart0:串口设置
                uart1_update_func(t2.reg, BY_SETUP);
            }
#if MCU == STM32F103VCT6
            else if (source == FROM_U4) {
                us.u4.reg = t2.reg;
                //扩展多路串口uart3:串口设置
                uart4_update_func(t2.reg, BY_SETUP);
            }
#endif
            else {
                loge("uart set error: 1");
                return;
            }

            flash_write_normal_parameters();
            send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        } else {
            loge("uart set error");
        }
    } else {
        loge("uart set or ask err");
    }
}

/****************************************************************************
* 名称:    analyse_0x27_mfg_code ()
* 功能：制造商ID
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x27_mfg_code(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[128];

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        _memcpy(msg, (u8 *)&register_cfg.mfg_id, 5);
        handle_send_base_msg(from, 0x27, msg, 5);
    } else if (len > 5 && len <= 10) {
        //设置
        _memset((u8 *)&register_cfg.mfg_id, 0x00, 5);
        _memcpy((u8 *)&register_cfg.mfg_id, &hc_buf[3], len - 5);

        flash_write_import_parameters();

        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/*********************************************************
名    称：Analyse_0x38_lock_oil
功    能： 
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x38_lock_oil(u8 from, u8 *str, u16 len) {
    write_tts(TTS_PLAY_CALI_SPEED_OPEN, 14);

    rcmd.b.cut_oil = true;
    rcmd.b.rels_oil = false;

    tr9_car_status.bit.lift_limit = true;

    logd("$$$$cut_oil = 1$$$$");

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/*********************************************************
名    称：Analyse_0x39_unlock_oil
功    能： 
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x39_unlock_oil(u8 from, u8 *str, u16 len) {
    write_tts(TTS_PLAY_CALI_SPEED_CLOSE, 14);

    rcmd.b.cut_oil = false;
    rcmd.b.rels_oil = true;

    tr9_car_status.bit.lift_limit = false;
    logd("$$$$rels_oil = 0$$$$");

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/*********************************************************
名    称：Analyse_0x41_SetBKIp
功    能：解析设置备用服务器IP地址指令数据
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x41_set_bak_ip(u8 from, u8 *str, u16 len) {
    char *tok;
    u8 hc_buf[HANDSET_MAX_SIZE];
    u8 buf[64];
    u16 b_len = 0;
    u8 *pf;
    u16 tmp;
    u8 i;
    if (str == NULL || len > HANDSET_MAX_SIZE) {
    set_41_ip_err:
        return;
    }

    _memset(buf, 0x00, 64);
    _memset(hc_buf, 0x00, HANDSET_MAX_SIZE);
    _memcpy(hc_buf, str, len);

    pf = buf;
    if (hc_buf[2] == 0x02) //查询状态
    {
        b_len = 0;
        *pf++ = '<';
        if (server_cfg.select_tcp & 0x02 == 0x02)
            *pf++ = '1';
        else
            *pf++ = '0';
        *pf++ = ',';
        *pf++ = '"';
        _strcpy_len(pf, (u8 *)&server_cfg.bak_ip);
        b_len = _strlen((u8 *)&server_cfg.bak_ip);
        pf += b_len;

        *pf++ = '"';
        *pf++ = ',';

        tmp = server_cfg.bak_port;
        i = AsctoBCD((u16 *)&tmp, pf, 5, 1);
        pf += i;
        *pf++ = '>';
        b_len += i;
        b_len += 7;

        handle_send_base_msg(from, 0x41, buf, b_len);
        return;
    }

    // if (sms_phone_save)
    //    return;

    if (hc_buf[4] == 0x30) //区分是UDP还是TCP
        server_cfg.select_tcp &= (~0x02);
    else
        server_cfg.select_tcp |= 0x02;

    if ((tok = strtok((char *)&hc_buf, ",\"")) == NULL)
        goto set_41_ip_err;

    if ((tok = strtok(NULL, "\"")) != NULL) {
        strcpy((char *)&buf, tok);
        b_len = _strlen(buf);
        if (b_len > 23)
            goto set_41_ip_err;

        _memset((u8 *)&server_cfg.bak_ip, 0x00, 24);
        _memcpy_len((u8 *)&server_cfg.bak_ip, buf, b_len);
    } else
        goto set_41_ip_err;

    if ((tok = strtok(NULL, ">")) != NULL) {
        strcpy((char *)&buf, tok);
        if ((buf[0] >= 0x30) && (buf[0] <= 0x39))

            b_len = _strlen(buf);
        else {
            b_len = _strlen(buf);
            _memcpy_len(buf, buf + 1, b_len);
        }
        if (b_len < 2 || b_len > 5)
            goto set_41_ip_err;
        server_cfg.bak_port = atoi((const char *)&buf);

    } else
        goto set_41_ip_err;

    lm.reset = true;
    set_main_ip_to_tr9(false);
    flash_write_import_parameters();

    menu_report.ok_flag = true; //收到设置成功应答标志
    menu_cnt.menu_flag = true;

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/****************************************************************************
* 名称:    Analyse_0x45_set_state ()
* 功能：车辆空重载设置
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void Analyse_0x45_set_state(u8 from, u8 *str, u16 len) {
    u8 hc_buf[50];

    if (len > 50 || len < HANDSET_MIN_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (hc_buf[3] == 1) {
        car_state.bit.carriage = 0x00;
        register_cfg.all_open_switch = 0x00;
    } else if (hc_buf[3] == 2) {
        car_state.bit.carriage = 0x03;
        register_cfg.all_open_switch = 0x00;
    } else if (hc_buf[3] == 3) {
        car_state.bit.carriage = 0x01;
        register_cfg.all_open_switch = 0x55;

        flash_write_import_parameters();
    }
    flash_write_import_parameters();
    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/****************************************************************************
* 名称:    Analyse_0x45_set_state ()
* 功能：车辆终端类型设置
* 入口参数：无
* 出口参数：测油版   标准版
****************************************************************************/
static void Analyse_0x46_set_type(u8 from, u8 *str, u16 len) {
    u8 hc_buf[50];

    if (len > 50 || len < HANDSET_MIN_SIZE)
        return;

    _memcpy(hc_buf, str, len);
    /*
    	if(hc_buf[3] == 1){
    		register_cfg.hardware_ver = 0x00;
    		flash_write_import_parameters();
    	}else if(hc_buf[3] == 2){
    		register_cfg.hardware_ver = 0x55;
    		flash_write_import_parameters();
    	}
    */
    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/*********************************************************
名    analyse_0x47_watch_car_set
功    能：锁车
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x47_watch_car_set(u8 from, u8 *str, u16 len) {
    ctrl_relay_status(1);

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/*********************************************************
名    analyse_0x47_watch_car_unset
功    能 ：解锁
输入参数：无
输    出 ：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0x48_watch_car_unset(u8 from, u8 *str, u16 len) {
    ctrl_relay_status(0);

    send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
}

/****************************************************************************
* 名称:    analyse_0x60_set_province_id ()
* 功能：设置省域ID
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x60_set_province_id(u8 from, u8 *str, u16 len) {
    u8 hc_buf[100] = {0x00};
    u8 msg[128];
    u16 m_len = 0;
    u16 province_id = 0;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        //m_len = AsctoBCD((u16 *)&register_cfg.province, msg, 2, 0);
        province_id = uPolygon;
        m_len = AsctoBCD((u16 *)&province_id, msg, 2, 0);
        handle_send_base_msg(from, 0x60, msg, m_len);
    } else if (len > 5 && len <= 10) //设置
    {
        _memset(msg, 0, 128);
        _memcpy(msg, &hc_buf[3], len - 5);
        province_id = atoi((const char *)&msg);
        register_cfg.province = province_id;

        lm.reset = true;

        flash_write_import_parameters();

        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/****************************************************************************
* 名称:    analyse_0x61_set_city_id ()
* 功能：设置省域ID
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x61_set_city_id(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[128];
    u16 m_len = 0;
    u16 city_id = 0;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        //city_id = s_AreaNum.uRoad;
        m_len = AsctoBCD((u16 *)&city_id, msg, 4, 0);
        //m_len = AsctoBCD((u16 *)&register_cfg.city, msg, 4, 0);
        handle_send_base_msg(from, 0x61, msg, m_len);
    } else if (len > 5 && len <= 10) //设置
    {
        _memset(msg, 0, 128);
        _memcpy(msg, &hc_buf[3], len - 5);
        city_id = atoi((const char *)&msg);
        register_cfg.city = city_id;

        lm.reset = true;

        flash_write_import_parameters();

        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/****************************************************************************
* 名称:    analyse_0x62_set_terminal_type ()
* 功能：设置终端型号
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x62_set_terminal_type(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        handle_send_base_msg(from, 0x62, (u8 *)&register_cfg.terminal_type, 20); //jtt2018
        //handle_send_base_msg(from, 0x62, (u8 *)&register_cfg.terminal_type, 30);//jtt2019
    } else if (len > 5 && len <= 25) //设置
    {
        _memset((u8 *)&register_cfg.terminal_type, 0, 20);
        _memcpy((u8 *)&register_cfg.terminal_type, &hc_buf[3], len - 5);
        register_cfg.terminal_type[19] = '\0';

        lm.reset = true;

        flash_write_import_parameters();
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/****************************************************************************
* 名称:    analyse_0x63_set_terminal_id ()
* 功能：设置终端型号
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x63_set_terminal_id(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        handle_send_base_msg(from, 0x63, (u8 *)&register_cfg.terminal_id, 7);
    } else if (len > 5 && len <= 12) //设置
    {
        _memset((u8 *)&register_cfg.terminal_id, 0, 7);
        _memcpy((u8 *)&register_cfg.terminal_id, &hc_buf[3], len - 5);

        lm.reset = true;

        flash_write_import_parameters();
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

#if (P_RCD == RCD_BASE)
/****************************************************************************
* 名称:    analyse_0x66_set_car_type ()
* 功能：设置终端型号
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x66_set_car_type(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        handle_send_base_msg(from, 0x66, (u8 *)&car_cfg.type, 16);
    } else if (len > 5 && len <= 17) //设置
    {
        _memset((u8 *)&car_cfg.type, 0, 16);
        _memcpy((u8 *)&car_cfg.type, &hc_buf[3], len - 5);

        flash_write_import_parameters();
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    }
}

/****************************************************************************
* 名称:    analyse_0x6a_set_print_type ()
* 功能：设置打印格式
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_0x6a_set_print_type(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 msg[32];
    u16 print_type = 0;

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;

    _memcpy(hc_buf, str, len);

    if (len == 5) //查询
    {
        _memset(msg, 0x00, 32);
        msg[0] = car_cfg.coefficient[0]; //factory_para.print_type;
        handle_send_base_msg(from, 0x6a, msg, 1);
    } else if (len == 6) //设置
    {
        print_type = hc_buf[3];
        if (print_type == 0x01 || print_type == 0x02) {
            factory_para.print_type = print_type;
            car_cfg.coefficient[0] = print_type;

            flash_write_import_parameters();

            send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        }
    }
}
#endif

/****************************************************************************
* 名称:    analyse_0x6a_set_print_type ()
* 功能：设置打印格式
* 入口参数：无
* 出口参数：无
****************************************************************************/
#if (P_RCD == RCD_BASE)
static void analyse_0xc2_iccard(u8 from, u8 *str, u16 len) {
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};

    if (str == NULL || len > HANDSET_MAX_SIZE)
        return;
    if (len > 5) {
        len -= 5;
        _memcpy(hc_buf, str + 3, len);
        handle_send_base_msg(FROM_INNER, 0xc2, hc_buf, len);
    }
}
#endif

#if 0
static void rec_report_electric_infomation(void)
{
    u8 msg[128];

    u16 m_len;
    u32 ulLen;

    ulLen = 30;

    _sw_endian(msg, (u8 *)&ulLen, 4);
    m_len = 4;

    m_len += _memcpy_len(&msg[m_len], "100123456700000000000000000000", 30);

    pack_any_data(CMD_UP_E_AWB, msg, m_len, PK_NET, LINK_IP0 | LINK_IP1 | LINK_IP2 );

}
#endif

void copy_driver_message(u8 *buf) {
    //DRIVER_CONFIG_STRUCT driver_tmp;
    u16 len = 0;

    _memset((u8 *)&driver_cfg.name, 0, 10);
    _memset((u8 *)&driver_cfg.qualification, 0, 40);
    _memset((u8 *)&driver_cfg.organization, 0, 30);
    //解析数据并且保存
    len = *buf++;
    if (len >= 10)
        driver_cfg.name_len = 10;
    else
        driver_cfg.name_len = len;
    _memcpy((u8 *)&driver_cfg.name, buf, driver_cfg.name_len);
    buf += len;
    _memcpy((u8 *)&driver_cfg.qualification, buf, 20);
    buf += 20;
    len = *buf++;
    if (len >= 30)
        driver_cfg.organization_len = 30;
    else
        driver_cfg.organization_len = len;
    _memcpy((u8 *)&driver_cfg.organization, buf, driver_cfg.organization_len);

    buf += len;

    _memcpy((u8 *)&driver_cfg.date, buf, 4);

    flash_write_normal_parameters();
}

/*********************************************************
名    称：Analyse_0xee_extend
功    能：解析退签指令指令
输入参数：无
输    出：无
编写日期：2013-12-1
**********************************************************/
static void analyse_0xee_extend(u8 from, u8 *str, u16 len) {
    u16 id;
    u8 msg[100];
    u8 hc_buf[HANDSET_MAX_SIZE] = {0x00};
    u8 *pf;

    if (str == NULL || len > HANDSET_MAX_SIZE) {
        return;
    }

    _memcpy(hc_buf, str, len);

    id = (hc_buf[4] << 8) | hc_buf[5];
    if (id == 0x0003) //注销终端
    {
        pack_any_data(id, hc_buf, 0, PK_NET, LINK_IP0 | LINK_IP1 | LINK_IP2);
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    } else if (id == 0x0100) //手动注册
    {
        pack_register_info(1);
        init_s_authority_cfg();
        flash_write_tmp_parameters();
        send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
    } else if (id == 0x0102) //终端鉴权
    {
#if defined(JTT_808_2019)
        pf = hc_buf;
        *pf = _strlen(authority_cfg.code);
        len = 1;
        len += _memcpy_len((pf + len), authority_cfg.code, *pf);
        len += _memcpy_len(pf + len, mx.imei, 15);
        len += _memcpy_len(pf + len, versions, 20);

        pack_any_data(CMD_UP_RELOGIN, hc_buf, len, PK_HEX, LINK_IP0);
        pack_any_data(CMD_UP_RELOGIN, hc_buf, len, PK_HEX, LINK_IP1);

#else //jt808_2011

        //code_len = (authority_cfg.code[20]>=20) ? 0 : authority_cfg.code[20];
        len = _strlen(authority_cfg.code);
        _memcpy(hc_buf, (u8 *)&authority_cfg.code, len);
        pack_any_data(CMD_UP_RELOGIN, hc_buf, len, PK_HEX, LINK_IP0);
        pack_any_data(CMD_UP_RELOGIN, hc_buf, len, PK_HEX, LINK_IP1);

        menu_report.ok_flag = true;
#endif
    } else if (id == 0x0301) //事件报告
    {
        pack_any_data(id, (u8 *)&hc_buf[6], len - 8, PK_NET, LINK_IP0 | LINK_IP1 | LINK_IP2);
    } else if (id == 0x0302) //提问应答
    {
    } else if (id == 0x0303) //信息点播/取消
    {
        if (car_login)
            pack_any_data(id, (u8 *)&hc_buf[6], len - 8, PK_NET, LINK_IP0 | LINK_IP1 | LINK_IP2);
    } else if (id == 0x0701) {
        if (car_login) {
#if 0
            rec_report_electric_infomation();
#endif
            send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        }
    } else if (id == 0x0702) //驾驶员信息上报
    {
        if (mix.moving) //车辆行驶中，不能登退签
        {
            return;
        }
        ic_tr9.icc = true;
        mix.ic_login = (hc_buf[6] == 0x01) ? true : false;
        if (mix.ic_login == true && len > 35) {
            copy_driver_message(&hc_buf[14]);
            mix.ic_exchange = driver_login_in_out((u8 *)&mix.ic_driver, 20, true);
            tts_ICCard_voice();
        } else {
            if (hc_buf[12] != 0x03)
                hc_buf[12] = 0x03;
            len = 16;
            ic_driver_lisence = true;
        }
        _memcpy((u8 *)&hc_buf[7], (u8 *)&mix.time, 6);
        pack_any_data(id, (u8 *)&hc_buf[6], len - 8, PK_NET, LINK_IP0 | LINK_IP1 | LINK_IP2);

    } else if (id == 0x1710) //查询跟设置行驶记录仪唯一性编码以及客户编码
    {
        _memset(msg, 0, 100);
        pf = msg;

        if (len < 6) //查询参数
        {
            len = 0;
            _memcpy(pf, (u8 *)&unique_id.ccc, 7);
            pf += 7;
            _memcpy(pf, (u8 *)&unique_id.model, 16);
            pf += 16;
            _memcpy(pf, (u8 *)&unique_id.time, 3);
            pf += 3;
            _memcpy(pf, (u8 *)&unique_id.sn, 4);
            pf += 4;
            //_memcpy(pf, (u8 *)&s_Customer_idc.customer_id, s_Customer_idc.customer_id[10]);
            //pf += s_Customer_idc.customer_id[10];
            len = pf - msg;
            handle_send_ext_msg(from, 0x1710, msg, len);
        } else {
            len -= 8;
            if (len > 50)
                return;
            pf = &hc_buf[6];
            _memcpy((u8 *)&unique_id.ccc, pf, 7);
            pf += 7;
            _memcpy((u8 *)&unique_id.model, pf, 16);
            pf += 16;
            _memcpy((u8 *)&unique_id.time, pf, 3);
            pf += 3;
            _sw_endian((u8 *)&unique_id.sn, pf, 4);
            pf += 4;
            len -= 30;
            if (len > 4 && len < 11) {
                _memset(msg, 0, 100);
                _memcpy(msg, pf, len);

                if ((0x30 <= msg[0] <= 0x39) || (0x40 <= msg[0] <= 0x5a) || (0x61 <= msg[0] <= 0x7a)) {
                    //_memset((u8 *)&s_Customer_idc.customer_id, 0, 11);
                    // _memcpy((u8 *)&s_Customer_idc.customer_id, msg, len);
                    //s_Customer_idc.customer_id[10] = len;
                    flash_write_normal_parameters();
                }
            }
            write_mfg_parameter();
            send_handset_data(from, H_LEN_N, 0, (u8 *)&hd_set_ok, 0);
        }
    }
}

cmd_handset const CmdHandTab[] =
    {
        {0x01, analyse_0x01_send_msg},
        {0x02, analyse_0x02_send_sms},
        {0x03, analyse_0x03_set_ip},
        {0x04, analyse_0x04_set_id},
        {0x05, analyse_0x05_set_sms},
        {0x0c, analyse_0x0c_self_test},

        {0x11, analyse_0x11_handle},
        {0x14, analyse_0x14_set_apn},
        {0x15, analyse_0x15_initialization},
        {0x1c, analyse_0x1c_set_car_vin},
        {0x1d, analyse_0x1d_car_number},
        {0x1e, analyse_0x1e_car_type},
        {0x1f, analyse_0x1f_driver_code},
        {0x20, analyse_0x20_driver_qualification},
        {0x21, analyse_0x21_speed_type},
        {0x24, analyse_0x24_set_apn},
        {0x25, analyse_0x25_any_uart},
        {0x26, analyse_0x26_car_class},
        {0x27, analyse_0x27_mfg_code},
        {0x38, analyse_0x38_lock_oil},
        {0x39, analyse_0x39_unlock_oil},
        {0x41, analyse_0x41_set_bak_ip}, //0x41
        {0x45, Analyse_0x45_set_state},
        {0x46, Analyse_0x46_set_type},

        {0x47, analyse_0x47_watch_car_set},
        {0x48, analyse_0x48_watch_car_unset},

        {0x60, analyse_0x60_set_province_id},
        {0x61, analyse_0x61_set_city_id},
        {0x62, analyse_0x62_set_terminal_type},
        {0x63, analyse_0x63_set_terminal_id},
        {0xee, analyse_0xee_extend},
#if (P_RCD == RCD_BASE)
        {0x19, analyse_0x19_Set_speed_plus},
        {0x22, analyse_0x22_print}, //0x19
        {0x66, analyse_0x66_set_car_type},
        {0x6a, analyse_0x6a_set_print_type},
        {0xc2, analyse_0xc2_iccard}, //0xC2
        {0xc1, analyse_0xc2_iccard}, //0xC2
#endif

        {NULL, NULL}};

/*********************************************************
名    称：handset_any_parse
功    能：手柄数据解析
输入参数：无
输    出：无
**********************************************************/
u8 handset_any_parse(u8 from_t, u8 *str_t, u16 len_t) {
    u8 i;
    u8 any_buf[10] = {0x00};

    if (len_t > HANDSET_MAX_SIZE || len_t < HANDSET_MIN_SIZE)
        return CMD_ERROR;

    _memcpy(any_buf, str_t, 3);
    if (any_buf[0] != 0x24)
        return CMD_ERROR;

    for (i = 0; CmdHandTab[i].func != NULL; i++) {
        if (any_buf[1] == CmdHandTab[i].index) {
            (*CmdHandTab[i].func)(from_t, str_t, len_t);
            return CMD_ACCEPT;
        }
    }
    return CMD_ERROR;
}

#ifdef USING_HANDSET
/////////调度屏串口???? //////////
/****************************************************************************
* 名称:    get_uart_is_speedlimit ()
* 功能：判????前串口是否是限???器功能
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool get_uart_is_handset(u8 from_t) {
    if (u1_m.cur.b.handset == true && from_t == FROM_U1) {
        return true;
    } else {
        return false;
    }
}

/****************************************************************************
*函数名称 :    speedlimit_parse_proc()
* 功能 ：解析平台下发的限???器功能
*入口参数 ：无
*出口参数 ：无
****************************************************************************/
void handset_parse_proc(void) {
    typedef enum {
        E_SPLIMIT_IDLE,
        E_SPLIMIT_RECV,
        E_SPLIMIT_EXIT
    } E_SPLIMIT_STEP;

    static E_SPLIMIT_STEP step = E_SPLIMIT_IDLE;
    //    u16 i;
    bool ret;
    u8 r_buf[TR9_COM_DEC_LEN];
    u16 r_len = 0;

    static u8 from_id = 0;

    static UART_MANAGE_STRUCT present;

    switch (step) {
    case E_SPLIMIT_IDLE:
        from_id = get_uart_news(&present);
        ret = get_uart_is_handset(from_id);
        if (ret)
            step = E_SPLIMIT_RECV;
        break;
    case E_SPLIMIT_RECV:
        _memset(r_buf, 0x00, 200);
        r_len = copy_uart_news(from_id, r_buf, present.cur_p, present.c_len);
        if (r_len < 5 || r_len > TR9_COM_DEC_LEN) {
            step = E_SPLIMIT_EXIT;
            break;
        }
        handset_any_parse(FROM_U1, r_buf, r_len); //鏄熸繝 ####锟ワ骏锟ワ骏锟ワ骏锟ワ骏锟ワ骏锟?

        step = E_SPLIMIT_EXIT;
        break;
    case E_SPLIMIT_EXIT:
        set_uart_ack(from_id);
        step = E_SPLIMIT_IDLE;
        break;
    default:
        step = E_SPLIMIT_IDLE;
        break;
    }
}
#endif
