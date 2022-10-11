/**
  ******************************************************************************
  * @file    jt808_recive.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   JT808数据接收
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

SEND_MESS_INFOMATION_STRUCT rev_inf; //最新接收消息体

/*********************************************************
名    称：get_check_xor
功    能：计算校验值
参    数：*str 指针，st_addr 开始地址，end_addr 接收地址
输    出：返回校验值
编写日期：2011-01-18
*********************************************************/
u8 get_check_xor(u8 *str, u16 start_addr, u16 end_addr) {
    u16 i;
    u16 j;
    u8 xvalue;

    if (str == NULL) {
        loge("err start = %d, end = %d", start_addr, end_addr);
        return 0;
    }

    j = end_addr;

    xvalue = 0x00;
    for (i = start_addr; i < j; i++) {
        xvalue = xvalue ^ str[i];
    }

    return xvalue;
}

u16 rec_gbk_read(u8 *buf, u16 len) {
    u8 i;
    u8 *pf;
    u8 gbk;
    u16 j;

    j = 0;
    pf = buf;
    for (i = 0; i < len; i++) {
        if (*pf++ == 0xa3) {
            gbk = *pf++;
            if (gbk > 0x80) {
                *buf++ = gbk - 0x80;
                j++;
            }
        } else
            j++;
    }
    return j;
}

/*********************************************************
名	  称：Rec_Gets_Update_Ip
功	  能：远程升级IP获取
功 能 号: 0x8105
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项："1；CMNET；；；220.231.155.085；9000；；；；；；；"
"URL地址;拨号点名称;拨号用户名;拨号密码;地址;TCP端口;UDP端口;制造商ID;硬件版本;固件版本;连接到指定服务器时限"
*********************************************************/
static u8 Rec_Gets_Update_Ip(u16 index, u8 *str, u16 s_len) {
    u8 *buf;
    u8 tmp[100]; //"220.231.155.085"
    u8 ret = 0;
    u8 max_index = 0;
    u16 free_cnt = 0; //剩余待解析的个数
    bool use_default_para;
    IP_SERVER_CONFIG_STRUCT ip_t;

    u8 len_t;

    _memset((u8 *)&ip_t, 0x00, sizeof(ip_t));
    if (str == NULL || s_len == 0 || s_len > 512) {
        logd("UPAGEDE_ERR: value");
        return 0;
    }

    max_index = get_flag_cnt(str, s_len, 0x3b);
    if ((max_index < 9) || (max_index > 10)) {
        //解析出现错误
        //注意端口号数值不能带3B (';')
        logd("UPAGEDE_ERR: FLAG CNT");
        return 0;
    }

    free_cnt = s_len;

    //URL地址;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    //拨号点名称;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    //拨号用户名;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    //拨号密码;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    //IP地址;
    buf = (u8 *)&ip_t.ota_ip[0];
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        *buf++ = *str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    //TCP 端口
    _memset(tmp, 0x00, 100);
    buf = tmp;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        *buf++ = *str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    len_t = _strlen((u8 *)&tmp);
    if ((len_t <= 5) && (len_t > 0)) {
        ip_t.ota_port = atoi((char *)tmp);
        ip_t.select_tcp |= 0x04;
    }

    //UDP 端口
    _memset(tmp, 0x00, 100);
    buf = tmp;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        *buf++ = *str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    len_t = _strlen((u8 *)&tmp);
    if ((ip_t.select_tcp & 0x04) == 0) {
        if (len_t <= 5 && len_t > 0) {
            ip_t.ota_port = atoi((char *)tmp);
            ip_t.select_tcp &= 0xfb;
        }
    }

    //校验参数是否合法
    use_default_para = false;
    len_t = _strlen((u8 *)&ip_t.ota_ip[0]);
    if (len_t < 5 || len_t >= 23)
        use_default_para = true;

    if (ip_t.ota_port == 0)
        use_default_para = true;

    if (use_default_para) {
        _memset((u8 *)&server_cfg.ota_ip[0], 0x00, 24);
        _memcpy((u8 *)&server_cfg.ota_ip, "203.086.008.005", 15);
        server_cfg.ota_port = 9000;
        server_cfg.select_tcp |= 0x04;
    } else {
        _memset((u8 *)&server_cfg.ota_ip, 0x00, 24);
        _memcpy((u8 *)&server_cfg.ota_ip, (u8 *)&ip_t.ota_ip[0], 24);
        server_cfg.ota_port = ip_t.ota_port;
        server_cfg.select_tcp = ((ip_t.select_tcp & 0x04) != 0) ? (server_cfg.select_tcp | 0x04) : (server_cfg.select_tcp & 0xfb);
    }

    //制造商ID;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    //硬件版本;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    //软件版本;
    _memset(tmp, 0x00, 100);
    buf = tmp;
    while ((*str != ';') && (free_cnt > 0 && free_cnt <= s_len)) {
        *buf++ = *str++;
        free_cnt--;
    }
    str++;
    free_cnt--;

    len_t = _strlen((u8 *)&tmp);
    if (len_t == 0x00) {
        return 2;
    }

    if (len_t > 30 || len_t < 24) {
        logd("ota_err:0x8105 name len");
        return 0;
    }

    ret = check_update_version(UPGRADE_BYPASS, tmp, len_t);
    if (ret == 1) {
        return 1;
    } else {
        return 0;
    }
}

/*********************************************************
	截取传输数据的协议头
	返回消息体的头部
*********************************************************/
u8 *get_rec_center_head(u8 *buf, u16 *pl) {
    u8 i;

    _sw_endian((u8 *)&rev_inf.id, buf, 2);
    buf += 2;
    _sw_endian((u8 *)&rev_inf.pro.property, buf, 2);
    buf += 2;

#if defined(JTT_808_2019)

    _sw_endian((u8 *)&rev_inf.version, buf, 1); //2019版添加版本号
    buf += 1;
    for (i = 0; i < 10; i++) //2019版号码长度10个字节
        rev_inf.number[i] = *buf++;

#else

    for (i = 0; i < 6; i++)
        rev_inf.number[i] = *buf++;

#endif

    //终端手机号，不是本机的返回 0
    _sw_endian((u8 *)&rev_inf.run_no, buf, 2); //流水号
    buf += 2;

    *pl = rev_inf.pro.bit.mess_len; //

    //判断消息是否分包
    if (rev_inf.pro.bit.if_pack) {
        _sw_endian((u8 *)&rev_inf.sub_tatal, buf, 2);
        buf += 2;
        _sw_endian((u8 *)&rev_inf.sub_no, buf, 2);
        buf += 2;

        *pl -= 4;
    }
    return buf;
}

/*********************************************************
名	  称：rec_sure_response
功	  能：中心确认应答，
功 能 号: 0x8001
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：原命令0x21，在此添加登签退签判断，转发到手柄
 7E8001000501382655850200000010080100507E
*********************************************************/
static void rec_sure_response(u16 index, u8 *str, u16 s_len) {
    u16 uiWater, uId = 0;
    static u8 regist = 0;

    uiWater = *str++; //流水号
    uiWater <<= 8;
    uiWater += *str++;

    run_no = uiWater + 1;

    uId = *str++; //消息ID
    uId <<= 8;
    uId += *str++;

    if (*str == 0x00 || *str == 0x04) //成功
    {
        switch (uId) {
        case CMD_UP_MEDIA_DATA:
            //传输下一帧数据 140912

            break;
        case CMD_UP_RUN_RECORD:
#if (P_RCD == RCD_BASE)
            //                if (server_cfg.dbip == 0x55)
            {
                if (netx == 0x01)
                    rsp.ack = true;
                else
                    rsp.ack2 = true;
            }
#endif
            break;
        case CMD_UP_LOGOUT:
            authority_cfg.type = 0;
            car_login = false;
            lm.sms_rst_en = true;
            lm.sms_rst_tick = tick;
            break;
        case CMD_UP_RELOGIN: {
            if (netx == 0x01) {
                if (!u1_m.cur.b.tt)
                    logd("第一路IP链接成功");
                gs.gprs1.login = true;
            } else if (netx == 0x02) {
                if (!u1_m.cur.b.tt)
                    logd("第二路IP链接成功");
                gs.gprs2.login = true;
            } else
                gs.gprs3.login = true;

        } break;
        case CMD_UP_BLIND:
            break;
        case CMD_UP_DRIVER_IN:

            break;
        case CMD_UP_AUTO_REPORT:
            if (*str == 0x04) {
                //一天只报一次累计驾驶时间
                car_alarm.bit.drive_over = false;
                car_alarm.bit.sos = false;
                bypass_alarm.reg = car_alarm.reg;
            }
            break;
        case CMD_UP_ASK_POS:
            if (*str == 0x04) {
                //一天只报一次累计驾驶时间
                car_alarm.bit.drive_over = false;
                car_alarm.bit.sos = false;
                bypass_alarm.reg = car_alarm.reg;
            }
            break;
        default:
            break;
        }
    } else //不成功
    {
        switch (uId) {
        case CMD_UP_RELOGIN:
            //authority_cfg.type = 0x00; //重新注册
            //失败5次后重新切换IP
            if (++regist > 7) {
                //lm.sms_rst_en= true;
                lm.sms_rst_tick = tick;
                regist = 0;
            }
            break;
        case CMD_UP_LOGOUT:
            if (netx == 0x01) {
                authority_cfg.type = 0;
                gs.gprs1.login = true;
            } else if (netx == 0x02) {
                authority_cfg.type2 = 0;
                gs.gprs2.login = true;
            } else {
                authority_cfg.type3 = 0;
                gs.gprs3.login = true;
            }
            //lm.sms_rst_en = true;
            //lm.sms_rst_tick = tick;
            break;
        default:
            break;
        }
    }
    get_net_ack(uId, uiWater, netx);
    get_reprot_ack(uId, uiWater, netx);
#if (P_BATCH == BATCH_BASE)
    get_batch_ack(uId, uiWater);
#endif
}

#if (P_RCD == RCD_BASE)
static void Rec_retry_pack(u16 index, u8 *str, u16 s_len) {
    u8 num = 0;

    str++; //去掉起始流水号
    str++; //去掉起始流水号
    num = *str++;
    if (num > 100 || num > rsp.all_num) {
        pack_general_answer(1);
        return;
    }

    if (num == 0) {
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        pack_general_answer(0);
        return;
    }

    spi_flash_erase(_retry_addr(0));
    spi_flash_write(str, _retry_addr(0), (num * 2));

    rsp.retry_en = true;
    rsp.retry_cnt = num;
    rsp.retry_no = 0;
    rsp.retry_delay_tick = tick;
    pack_general_answer(0);
}
#endif

/*********************************************************
名	  称：Terminal_Login_Return
功	  能：得到终端鉴权码
输入参数：
输出参数：
返	  回：
注意事项：0x8100
*********************************************************/
static void rec_login_return(u16 index, u8 *str, u16 s_len) {
    if (str == NULL || s_len >= 50 || s_len < 2) {
        logd("rec_login_return err :1");
        return;
    }

    //去掉流水号
    str++;
    str++;

    if (*str == 0x02) {
        gs.gprs1.login = true; //完成鉴权
        return;
    }
    if (*str++ != 0) {
        logd("rec_login_return err :2");
        self_test.uGsmU = '3';
        return;
    }
    //if (server_cfg.dbip == 0x55)
    {
        if (netx <= 0x01) {
            _memset((u8 *)&authority_cfg.code, 0x00, 50);
            _memcpy((u8 *)&authority_cfg.code, str, s_len - 3);
            //  authority_cfg.code[20] = s_len - 3;
            authority_cfg.type = 0xcc;
            quick_into_enable = true;
        } else {
            if (s_len - 3 > 0) {
                _memset((u8 *)&authority_cfg.code2, 0x00, 50);
                _memcpy((u8 *)&authority_cfg.code2, str, s_len - 3);
                //  authority_cfg.code2[20] = s_len - 3;
                authority_cfg.type2 = 0xcc;
                quick2_into_enable = true;
            }
        }
    }
#if 0
    else
    {
        _memset((u8 *)&authority_cfg.code, 0x00, 21);
        _memcpy((u8 *)&authority_cfg.code, str, s_len - 3);
        authority_cfg.code[20] = s_len - 3;
        authority_cfg.type = 0xcc;
        quick_into_enable = true;
    }
#endif
    flash_write_tmp_parameters();
}

/*********************************************************
名	  称：Set_Terminal_Para
功	  能：设置终端参数
功 能 号: 0x8103
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：59个参数设置
*********************************************************/
static void rec_set_terminal_para(u16 index, u8 *str, u16 s_len) {
    u16 uiId, len = 0; //参数ID 长度
    u8 uParaNum = 0;   //包参数个数
    u32 tmp = 0;
    bool unlock_server_cfg = false;

    if (str == NULL || s_len < 5 || s_len > MAX_NET_PACKAGE)
        return;

    unlock_server_cfg = false;

    uParaNum = *str++;
    s_len -= 1;

    while ((s_len > 0) && (uParaNum > 0)) //等待参数设置完成
    {
        _sw_endian((u8 *)&uiId, str, 4);
        str += 4;

        len = *str++;
        switch (uiId) {
        case 0x0001:
            _sw_endian((u8 *)&link_cfg.hear_beat_time, str, 4);
            break;
        case 0x0002:
            _sw_endian((u8 *)&link_cfg.tcp_over_time, str, 4);
            break;
        case 0x0003:
            _sw_endian((u8 *)&link_cfg.tcp_resend_cnt, str, 4);
            break;
        case 0x0004:
            _sw_endian((u8 *)&link_cfg.udp_over_time, str, 4);
            break;
        case 0x0005:
            _sw_endian((u8 *)&link_cfg.udp_resend_cnt, str, 4);
            break;
        case 0x0006:
            _sw_endian((u8 *)&link_cfg.sms_over_time, str, 4);
            break;
        case 0x0007:
            _sw_endian((u8 *)&link_cfg.sms_resend_cnt, str, 4);
            break;
        case 0x0010:
            if (len < 7 || len > 23)
                return;
            _memset((u8 *)&server_cfg.apn, 0x00, 24);
            _memcpy((u8 *)&server_cfg.apn, str, len);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x0011:
            if (len > 9)
                return;
            _memset((u8 *)&server_cfg.sim_user, 0x00, 10);
            _memcpy((u8 *)&server_cfg.sim_user, str, len);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x0012:
            if (len > 7)
                return;
            _memset((u8 *)&server_cfg.sim_pwd, 0x00, 8);
            _memcpy((u8 *)&server_cfg.sim_pwd, str, len);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x0013:
            if (len < 7 || len > 15)
                return;
            // if (sms_phone_save)
            //     break;
            _memset((u8 *)&server_cfg.main_ip, 0x00, 24);
            _memset((u8 *)&server_cfg.bak_ip, 0x00, 24);
            _memcpy((u8 *)&server_cfg.main_ip, str, len);
            _memcpy((u8 *)&server_cfg.bak_ip, (u8 *)&server_cfg.main_ip, 24);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x0014:
            if (len > 23)
                return;
            //if (sms_phone_save)
            //    break;
            _memset((u8 *)&server_cfg.apn, 0x00, 24);
            _memcpy((u8 *)&server_cfg.apn, str, len);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x0015:
            if (len > 9)
                return;
            lm.reset = true;
            unlock_server_cfg = true;
            _memset((u8 *)&server_cfg.sim_user, 0x00, 10);
            _memcpy((u8 *)&server_cfg.sim_user, str, len);
            break;
        case 0x0016:
            if (len > 7)
                return;
            lm.reset = true;
            unlock_server_cfg = true;
            _memset((u8 *)&server_cfg.sim_pwd, 0x00, 8);
            _memcpy((u8 *)&server_cfg.sim_pwd, str, len);
            break;
        case 0x0017:
            if (len > 23)
                return;
            // if (sms_phone_save)
            //    break;
            _memset((u8 *)&server_cfg.bak_ip, 0x00, 24);
            _memcpy((u8 *)&server_cfg.bak_ip, str, len);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x0018:
            // if (sms_phone_save)
            //    break;
            _sw_endian((u8 *)&server_cfg.main_port, str, 4);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x0019:
            // if (sms_phone_save)
            //    break;
            _sw_endian((u8 *)&server_cfg.bak_port, str, 4);
            lm.reset = true;
            unlock_server_cfg = true;
            break;
        case 0x001A:
            if (len > 23)
                return;
            //lm.reset = true;
            //_memset((u8 *)&server_cfg.ic_main_ip, 0x00, 24);
            //_memcpy((u8 *)&server_cfg.ic_main_ip, str, len);
            break;
        case 0x001B:
            //lm.reset = true;
            //_sw_endian((u8 *)&server_cfg.ic_tcp_prot, str, 4);
            break;
        case 0x001C:
            //lm.reset = true;
            //_sw_endian((u8 *)&server_cfg.ic_udp_prot, str, 4);
            break;
        case 0x001D:
            if (len > 23)
                return;
            //lm.reset = true;
            //_memset((u8 *)&server_cfg.ic_bak_ip, 0x00, 24);
            //_memcpy((u8 *)&server_cfg.ic_bak_ip, str, len);
            break;
        case 0x0020:
            _sw_endian((u8 *)&report_cfg.sample_mode, str, 4);
            break;
        case 0x0021:
            _sw_endian((u8 *)&report_cfg.acc_mode, str, 4);
            break;
        case 0x0022:
            _sw_endian((u8 *)&report_cfg.no_dvr_time, str, 4);
            break;
        case 0x0027:
            _sw_endian((u8 *)&report_cfg.sleep_time, str, 4);
            break;
        case 0x0028:
            _sw_endian((u8 *)&report_cfg.sos_time, str, 4);
            break;
        case 0x0029:
            _sw_endian((u8 *)&report_cfg.dft_time, str, 4);
            break;
        case 0x002C:
            _sw_endian((u8 *)&report_cfg.dft_dst, str, 4);
            break;
        case 0x002D:
            _sw_endian((u8 *)&report_cfg.no_dvr_dst, str, 4);
            break;
        case 0x002E:
            _sw_endian((u8 *)&report_cfg.sleep_dst, str, 4);
            break;
        case 0x002F:
            _sw_endian((u8 *)&report_cfg.sos_dst, str, 4);
            break;
        case 0x0030:
            _sw_endian((u8 *)&report_cfg.angle_dgree, str, 4);
            break;
        case 0x0031:
            _sw_endian((u8 *)&alarm_cfg.elc_geo, str, 2);
            break;
        case 0x0040:
            if (len >= 15)
                return;
            _memcpy((u8 *)&phone_cfg.user, str, len);
            phone_cfg.user[len] = '\0';
            break;
#if (P_SOUND == SOUND_BASE)
        case 0x0041:
            if (len >= 15)
                return;
            _memcpy((u8 *)&phone_cfg.reset, str, len);
            phone_cfg.reset[len] = '\0';
            break;
        case 0x0042:
            if (len >= 15)
                return;
            _memcpy((u8 *)&phone_cfg.factory, str, len);
            phone_cfg.listen[len] = '\0';
            break;
#endif
        case 0x0043:
            if (len >= 15)
                return;
            // if (sms_phone_save)
            //   return;
            _memcpy((u8 *)&phone_cfg.sms, str, len);
            phone_cfg.sms[len] = '\0';
            phone_cfg.sms[PHONELEN - 1] = len;
            break;
        case 0x0044:
            if (len >= 15)
                return;
            alarm_cfg.sms_sw.bit.sos = true;
            _memcpy((u8 *)&phone_cfg.alarm_sms, str, len);
            phone_cfg.alarm_sms[len] = '\0';
            break;
#if (P_SOUND == SOUND_BASE)
        case 0x0045:
            _sw_endian((u8 *)&phone_cfg.ring_type, str, 4);
            break;
        case 0x0046:
            _sw_endian((u8 *)&phone_cfg.each_time, str, 4);
            if (phone_cfg.each_time == 0) {
                sys_cfg.call_set = 0xc0;
            } else {
                sys_cfg.call_set = 0x00;
            }
            break;
        case 0x0047:
            _sw_endian((u8 *)&phone_cfg.month_time, str, 4);
            break;
        case 0x0048:
            _memcpy((u8 *)&phone_cfg.listen, str, len);
            phone_cfg.listen[len] = '\0';
            break;
        case 0x0049:
            _memcpy((u8 *)&phone_cfg.center, str, len);
            phone_cfg.center[len] = '\0';
            break;
#endif
        case 0x0050:
            _sw_endian((u8 *)&alarm_cfg.bypass_sw, str, 4);
            break;
        case 0x0051:
            _sw_endian((u8 *)&alarm_cfg.sms_sw.reg, str, 4);
            break;
        case 0x0052:
            _sw_endian((u8 *)&alarm_cfg.pic_sw, str, 4);
            break;
        case 0x0053:
            _sw_endian((u8 *)&alarm_cfg.pic_save_sw, str, 4);
            break;
        case 0x0054:
            _sw_endian((u8 *)&alarm_cfg.import_alarm_sw, str, 4);
            break;
        case 0x0055:
            _sw_endian((u8 *)&alarm_cfg.speed_over_km, str, 4);
            break;
        case 0x0056:
            _sw_endian((u8 *)&alarm_cfg.speed_over_time, str, 4);
            break;
        case 0x0057:
            _sw_endian((u8 *)&alarm_cfg.fatigure_time, str, 4);
            break;
        case 0x0058:
            _sw_endian((u8 *)&alarm_cfg.day_time, str, 4);
            break;
        case 0x0059:
            _sw_endian((u8 *)&alarm_cfg.stop_rest_time, str, 4);
            break;
        case 0x005a:
            _sw_endian((u8 *)&alarm_cfg.stoping_time, str, 4);
            break;
        case 0x005b:
            _sw_endian((u8 *)&alarm_cfg.pre_speed_over, str, 2);
            break;
        case 0x005c:
            _sw_endian((u8 *)&alarm_cfg.pre_fatigue, str, 2);
            break;
        case 0x005d:
            _sw_endian((u8 *)&alarm_cfg.collision, str, 2);
            break;
        case 0x005e:
            _sw_endian((u8 *)&alarm_cfg.turn_on_one_side, str, 2);
            break;
#if (P_CMA == CMA_BASE)
        case 0x0064:
            _sw_endian((u8 *)&pic_cfg.auto_time.reg, str, 4);
            break;
        case 0x0065:
            _sw_endian((u8 *)&pic_cfg.auto_distance.reg, str, 4);
            break;
        case 0x0070:
            _sw_endian((u8 *)&pic_cfg.quality, str, 4);
            break;
        case 0x0071:
            _sw_endian((u8 *)&pic_cfg.bright, str, 4);
            break;
        case 0x0072:
            _sw_endian((u8 *)&pic_cfg.contrast, str, 4);
            break;
        case 0x0073:
            _sw_endian((u8 *)&pic_cfg.saturation, str, 4);
            break;
        case 0x0074:
            _sw_endian((u8 *)&pic_cfg.color, str, 4);
            break;
#endif
        case 0x0080:
            _sw_endian((u8 *)&tmp, str, 4);
            run.total_dist = tmp * 100;
            save_run_parameterer();
            break;
        case 0x0081:
            lm.reset = true;
            _sw_endian((u8 *)&register_cfg.province, str, 2);
            break;
        case 0x0082:
            lm.reset = true;
            _sw_endian((u8 *)&register_cfg.city, str, 2);
            break;
        case 0x0083:

            if (len > 9)
                len = 9;
            _memset((u8 *)&car_cfg.license, 0x00, 12);
            _memcpy((u8 *)&car_cfg.license, str, len);
            car_cfg.license[len] = '\0';
            //_memset((u8*)&register_cfg.license, 0x00, 10);
            //_memcpy((u8 *)&register_cfg.license, str, len);
            //register_cfg.license[len] = '\0';
            rcd_para_inf.en = true;
            rcd_para_inf.type = 0xCA;
            unlock_server_cfg = true;
            break;
        case 0x0084:
            lm.reset = true;
            register_cfg.color = *str;
            break;
            //扩展参数

        case 0x010b: //重新标定系数
            //_sw_endian((u8 *)&sys_cfg.rpm_sensor, str, 2);
            break;
        case 0x010c: //扩展串口功能选择
            _sw_endian((u8 *)&tmp, str, len);

            break;

        default:
            break;
        }

        s_len = s_len >= (len + 5) ? (s_len - len - 5) : 0;
        str += len;
        uParaNum--;
    }

    if (unlock_server_cfg) {
        unlock_server_cfg = false;
        flash_write_import_parameters();
    }

    flash_write_normal_parameters();

    pack_general_answer(0);
}

static u16 get_any_jtb_para(u16 id, u8 *buf_t) {
    u8 len;
    u8 buf[100];
    u32 tmp = 0;

    len = 0;

    switch (id) {
    case 0x0001:
        _sw_endian(buf, (u8 *)&link_cfg.hear_beat_time, 4);
        len = 4;
        break;
    case 0x0002:
        _sw_endian(buf, (u8 *)&link_cfg.tcp_over_time, 4);
        len = 4;
        break;
    case 0x0003:
        _sw_endian(buf, (u8 *)&link_cfg.tcp_resend_cnt, 4);
        len = 4;
        break;
    case 0x0004:
        _sw_endian(buf, (u8 *)&link_cfg.udp_over_time, 4);
        len = 4;
        break;
    case 0x0005:
        _sw_endian(buf, (u8 *)&link_cfg.udp_resend_cnt, 4);
        len = 4;
        break;
    case 0x0006:
        _sw_endian(buf, (u8 *)&link_cfg.sms_over_time, 4);
        len = 4;
        break;
    case 0x0007:
        _sw_endian(buf, (u8 *)&link_cfg.sms_resend_cnt, 4);
        len = 4;
        break;
    case 0x0010:
        len = _strcpy_len(buf, (u8 *)&server_cfg.apn);
        break;
    case 0x0011:
        len = _strcpy_len(buf, (u8 *)&server_cfg.sim_user);
        break;
    case 0x0012:
        len = _strcpy_len(buf, (u8 *)&server_cfg.sim_pwd);
        break;
    case 0x0013:
        if (_strcmp((u8 *)&server_cfg.bak_ip, "jt1.gghypt.net") == 1)
            len = _strcpy_len(buf, "jt1.gghypt.net");
        else
            len = _strcpy_len(buf, (u8 *)&server_cfg.main_ip);
        break;
    case 0x0014:
        len = _strcpy_len(buf, (u8 *)&server_cfg.apn);
        break;
    case 0x0015:
        len = _strcpy_len(buf, (u8 *)&server_cfg.sim_user);
        break;
    case 0x0016:
        len = _strcpy_len(buf, (u8 *)&server_cfg.sim_pwd);
        break;
    case 0x0017:
        if (_strcmp((u8 *)&server_cfg.main_ip, "jt1.gghypt.net") == 1)
            len = _strcpy_len(buf, "jt2.gghypt.net");
        else if (_strcmp((u8 *)&server_cfg.bak_ip, "jt1.gghypt.net") == 1)
            len = _strcpy_len(buf, "jt2.gghypt.net");
        else
            len = _strcpy_len(buf, (u8 *)&server_cfg.bak_ip);
        break;
    case 0x0018:
        if (_strcmp((u8 *)&server_cfg.bak_ip, "jt1.gghypt.net") == 1)
            _sw_endian(buf, (u8 *)&server_cfg.bak_port, 4);
        else
            _sw_endian(buf, (u8 *)&server_cfg.main_port, 4);
        len = 4;
        break;
    case 0x0019:
        if (_strcmp((u8 *)&server_cfg.main_ip, "jt1.gghypt.net") == 1)
            _sw_endian(buf, (u8 *)&server_cfg.main_port, 4);
        else
            _sw_endian(buf, (u8 *)&server_cfg.bak_port, 4);
        len = 4;
        break;
    case 0x001A:
        //len = _strcpy_len(buf, (u8 *)&server_cfg.ic_main_ip);
        break;
    case 0x001B:
        //_sw_endian(buf, (u8 *)&server_cfg.ic_tcp_prot, 4);
        //len =4;
        break;
    case 0x001C:
        //_sw_endian(buf, (u8 *)&server_cfg.ic_udp_prot, 4);
        //len =4;
        break;
    case 0x001D:
        //len = _strcpy_len(buf, (u8 *)&server_cfg.ic_bak_ip);
        break;
    case 0x0020:
        _sw_endian(buf, (u8 *)&report_cfg.sample_mode, 4);
        len = 4;
        break;
    case 0x0021:
        _sw_endian(buf, (u8 *)&report_cfg.acc_mode, 4);
        len = 4;
        break;
    case 0x0022:
        _sw_endian(buf, (u8 *)&report_cfg.no_dvr_time, 4);
        len = 4;
        break;
    case 0x0027:
        _sw_endian(buf, (u8 *)&report_cfg.sleep_time, 4);
        len = 4;
        break;
    case 0x0028:
        _sw_endian(buf, (u8 *)&report_cfg.sos_time, 4);
        len = 4;
        break;
    case 0x0029:
        _sw_endian(buf, (u8 *)&report_cfg.dft_time, 4);
        len = 4;
        break;
    case 0x002C:
        _sw_endian(buf, (u8 *)&report_cfg.dft_dst, 4);
        len = 4;
        break;
    case 0x002D:
        _sw_endian(buf, (u8 *)&report_cfg.no_dvr_dst, 4);
        len = 4;
        break;
    case 0x002E:
        _sw_endian(buf, (u8 *)&report_cfg.sleep_dst, 4);
        len = 4;
        break;
    case 0x002F:
        _sw_endian(buf, (u8 *)&report_cfg.sos_dst, 4);
        len = 4;
        break;
    case 0x0030:
        _sw_endian(buf, (u8 *)&report_cfg.angle_dgree, 4);
        len = 4;
        break;
    case 0x0031:
        _sw_endian(buf, (u8 *)&alarm_cfg.elc_geo, 2);
        len = 2;
        break;
    case 0x0040:
        len = _strcpy_len(buf, (u8 *)&phone_cfg.user);
        break;
#if (P_SOUND == SOUND_BASE)
    case 0x0041:
        len = _strcpy_len(buf, (u8 *)&phone_cfg.reset);
        break;
    case 0x0042:
        len = _strcpy_len(buf, (u8 *)&phone_cfg.factory);
        break;
#endif
    case 0x0043:
        len = _strcpy_len(buf, (u8 *)&phone_cfg.sms);
        break;
    case 0x0044:
        len = _strcpy_len(buf, (u8 *)&phone_cfg.alarm_sms);
        break;
#if (P_SOUND == SOUND_BASE)
    case 0x0045:
        _sw_endian(buf, (u8 *)&phone_cfg.ring_type, 4);
        len = 4;
        break;
    case 0x0046:
        _sw_endian(buf, (u8 *)&phone_cfg.each_time, 4);
        len = 4;
        break;
    case 0x0047:
        _sw_endian(buf, (u8 *)&phone_cfg.month_time, 4);
        len = 4;
        break;
    case 0x0048:
        len = _strcpy_len(buf, (u8 *)&phone_cfg.listen);
        break;
    case 0x0049:
        len = _strcpy_len(buf, (u8 *)&phone_cfg.center);
        break;
#endif
    case 0x0050:
        _sw_endian(buf, (u8 *)&alarm_cfg.bypass_sw, 4);
        len = 4;
        break;
    case 0x0051:
        _sw_endian(buf, (u8 *)&alarm_cfg.sms_sw.reg, 4);
        len = 4;
        break;
    case 0x0052:
        _sw_endian(buf, (u8 *)&alarm_cfg.pic_sw, 4);
        len = 4;
        break;
    case 0x0053:
        _sw_endian(buf, (u8 *)&alarm_cfg.pic_save_sw, 4);
        len = 4;
        break;
    case 0x0054:
        _sw_endian(buf, (u8 *)&alarm_cfg.import_alarm_sw, 4);
        len = 4;
        break;
    case 0x0055:
        _sw_endian(buf, (u8 *)&alarm_cfg.speed_over_km, 4);
        len = 4;
        break;
    case 0x0056:
        _sw_endian(buf, (u8 *)&alarm_cfg.speed_over_time, 4);
        len = 4;
        break;
    case 0x0057:
        _sw_endian(buf, (u8 *)&alarm_cfg.fatigure_time, 4);
        len = 4;
        break;
    case 0x0058:
        _sw_endian(buf, (u8 *)&alarm_cfg.day_time, 4);
        len = 4;
        break;
    case 0x0059:
        _sw_endian(buf, (u8 *)&alarm_cfg.stop_rest_time, 4);
        len = 4;
        break;
    case 0x005a:
        _sw_endian(buf, (u8 *)&alarm_cfg.stoping_time, 4);
        len = 4;
        break;
    case 0x005b:
        _sw_endian(buf, (u8 *)&alarm_cfg.pre_speed_over, 2);
        len = 2;
        break;
    case 0x005c:
        _sw_endian(buf, (u8 *)&alarm_cfg.pre_fatigue, 2);
        len = 2;
        break;
    case 0x005d:
        _sw_endian(buf, (u8 *)&alarm_cfg.collision, 2);
        len = 2;
        break;
    case 0x005e:
        _sw_endian(buf, (u8 *)&alarm_cfg.turn_on_one_side, 2);
        len = 2;
        break;
#if (P_CMA == CMA_BASE)
    case 0x0064:
        _sw_endian(buf, (u8 *)&pic_cfg.auto_time.reg, 4);
        len = 4;
        break;
    case 0x0065:
        _sw_endian(buf, (u8 *)&pic_cfg.auto_distance.reg, 4);
        len = 4;
        break;
    case 0x0070:
        _sw_endian(buf, (u8 *)&pic_cfg.quality, 4);
        len = 4;
        break;
    case 0x0071:
        _sw_endian(buf, (u8 *)&pic_cfg.bright, 4);
        len = 4;
        break;
    case 0x0072:
        _sw_endian(buf, (u8 *)&pic_cfg.contrast, 4);
        len = 4;
        break;
    case 0x0073:
        _sw_endian(buf, (u8 *)&pic_cfg.saturation, 4);
        len = 4;
        break;
    case 0x0074:
        _sw_endian(buf, (u8 *)&pic_cfg.color, 4);
        len = 4;
        break;
#endif
    case 0x0080:
        tmp = run.total_dist / 100;
        _sw_endian(buf, (u8 *)&tmp, 4);
        len = 4;
        break;
    case 0x0081:
        _sw_endian(buf, (u8 *)&register_cfg.province, 2);
        len = 2;
        break;
    case 0x0082:
        _sw_endian(buf, (u8 *)&register_cfg.city, 2);
        len = 2;
        break;
    case 0x0083:
        len = _strcpy_len(buf, (u8 *)&car_cfg.license);
        break;
    case 0x0084:
        buf[0] = register_cfg.color;
        len = 1;
        break;

    default:
        len = 0;
        break;
    }

    if (len > 100) {
        return 0;
    }

    _memcpy(buf_t, (u8 *)&buf, len);

    return len;
}

static u16 get_any_tr_para(u16 id, u8 *buf_t) {
    u8 len;
    u8 buf[100];
    //    u32 sw_tmp;

    len = 0;

    switch (id) {
    case 0x0104: //通话限制拨入拨出功能
        *buf = sys_cfg.call_set;
        len = 1;
        break;
    case 0x0105: //软件版本号
        len = _strcpy_len(buf, (u8 *)FW_VERSION) - 1;
        break;

    default:
        len = 0;
        break;
    }

    if (len > 100) {
        return 0;
    }

    _memcpy(buf_t, (u8 *)&buf, len);

    return len;
}

static u16 add_msg_view_para(u16 id, u8 *msg_t, u16 mlen_t, u8 *buf_t, u16 b_len_t) {
    u8 tmp[256];
    u16 t_len = 0;

    if (b_len_t > 250 || (mlen_t + b_len_t) > MAX_NET_PACKAGE) {
        return 0;
    }

    t_len = 0;
    tmp[t_len++] = 0x00;
    tmp[t_len++] = 0x00;
    tmp[t_len++] = id >> 8;
    tmp[t_len++] = id;
    tmp[t_len++] = b_len_t;
    t_len += _memcpy_len(&tmp[t_len], buf_t, b_len_t);

    _memcpy(msg_t, (u8 *)&tmp, t_len);

    return t_len;
}

/*********************************************************
名	  称：rec_view_terminal_all_para
功	  能：远程参数查看 汽车配置信息
功 能 号: 0x8104
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：//根据查询的命令项进行返回，默认59个参数
*********************************************************/
static void rec_view_terminal_all_para(u16 index, u8 *str, u16 s_len) {
    u8 id_num;
    u8 inf[100];
    u16 i_len;
    u8 msg[MAX_NET_PACKAGE];
    u16 m_len;
    u16 i;

    if (s_len >= 5) //按协议要求，长度位空
    {
        ///pack_general_answer(1);
        return;
    }

    i_len = 0;
    m_len = 0;
    id_num = 0;
    _sw_endian(msg, (u8 *)&rev_inf.run_no, 2);
    m_len += 2;
    msg[m_len++] = 0x00;
    for (i = VIEW_PARA_JTB_MIN; i <= VIEW_PARA_JTB_MAX; i++) {
        i_len = get_any_jtb_para(i, inf);
        if (i_len > 0 && i_len < 100) {
            m_len += add_msg_view_para(i, (u8 *)&msg[m_len], m_len, (u8 *)&inf, i_len);
            id_num++;
        }
    }
    for (i = 0xf000; i <= 0xf003; i++) {
        i_len = get_any_jtb_para(i, inf);
        if (i_len > 0 && i_len < 100) {
            m_len += add_msg_view_para(i, (u8 *)&msg[m_len], m_len, (u8 *)&inf, i_len);
            id_num++;
        }
    }

    for (i = VIEW_PARA_TR_MIN; i <= VIEW_PARA_TR_MAX; i++) {
        i_len = get_any_tr_para(i, inf);
        if (i_len > 0 && i_len < 100) {
            m_len += add_msg_view_para(i, (u8 *)&msg[m_len], m_len, (u8 *)&inf, i_len);
            id_num++;
        }
    }

    msg[2] = id_num;
    pack_any_data(CMD_UP_ASK_PARA, msg, m_len, PK_HEX, (netx));
}

/*********************************************************
名	  称：rec_view_terminal_all_para
功	  能：远程参数查看 汽车配置信息
功 能 号: 0x8104
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：//根据查询的命令项进行返回，默认59个参数
*********************************************************/
static void rec_view_terminal_any_para(u16 index, u8 *str, u16 s_len) {
    u8 msg[MAX_NET_PACKAGE];  //平台下发的信息
    u8 pack[MAX_NET_PACKAGE]; //待发送的数据
    u16 p_len = 0;
    u8 inf[100]; //单项信息
    u16 i_len = 0;

    u16 proc_len = 0; //已读平台下发信息的长度

    u8 all_num = 0;   //平台要求总包数
    u8 proc_num = 0;  //已处理的包数
    u16 asmb_num = 0; //实际组装的包数

    u16 uiId;
    u32 tmp;

    if (str == NULL || s_len > MAX_NET_PACKAGE || s_len < 5) {
        return;
    }

    _memset(msg, 0, MAX_NET_PACKAGE);
    _memcpy(msg, str, s_len);

    all_num = msg[0];
    proc_len = 1;

    if (all_num == 0 || all_num > 250) {
        return;
    }

    proc_num = 0;
    asmb_num = 0;
    p_len = 0;
    _sw_endian(pack, (u8 *)&rev_inf.run_no, 2);
    p_len += 2;
    pack[p_len++] = 0x00;
    while (proc_num < all_num) //等待参数查询完成
    {
        proc_num++;
        if (proc_len > s_len) {
            return; //错误，丢弃
        }

        _sw_endian((u8 *)&tmp, (u8 *)&msg[proc_len], 4);
        proc_len += 4;

        uiId = tmp;
        if (uiId >= VIEW_PARA_JTB_MIN && uiId <= VIEW_PARA_JTB_MAX) {
            i_len = get_any_jtb_para(uiId, inf);
            if (i_len > 0 && i_len < 100) {
                p_len += add_msg_view_para(uiId, (u8 *)&pack[p_len], p_len, (u8 *)&inf, i_len);
                asmb_num++;
            }
        }

        if (uiId >= VIEW_PARA_TR_MIN && uiId <= VIEW_PARA_TR_MAX) {
            i_len = get_any_tr_para(uiId, inf);
            if (i_len > 0 && i_len < 100) {
                p_len += add_msg_view_para(uiId, (u8 *)&pack[p_len], p_len, (u8 *)&inf, i_len);
                asmb_num++;
            }
        }
    }

    pack[2] = asmb_num; //总个数
    pack_any_data(CMD_UP_ASK_PARA, pack, p_len, PK_HEX, (netx));
}

/*********************************************************
名	  称：Control_Terminal
功	  能：远程终端控制
功 能 号: 0x8105
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：执行继电器
*********************************************************/
static void rec_terminal_control(u16 index, u8 *str, u16 s_len) {
    u8 uId;
    u8 ret = 0;

    uId = *str++;

    switch (uId) {
    case 0x01: //无线升级
        if (upgrade_m.enable)
            return;

        _memset((u8 *)&upgrade_m, 0x00, sizeof(upgrade_m));

        ///s_len = rec_gbk_read(str, s_len);
        //截取升级服务器IP及端口
        ret = Rec_Gets_Update_Ip(index, str, s_len - 1);
        if (ret < 1 || ret > 2) {
            //使用默认服务器进行升级
            return;
        }

        if (ret == 1) {
            //指定
            upgrade_m.newest = false;
        } else if (ret == 2) {
            //最新
            upgrade_m.newest = true;
        }

        upgrade_m.enable = true;
        upgrade_m.mode = UPGRADE_OTA;
        break;
    case 0x02: //终端连接
        break;
    case 0x03: //终端关机
        break;
    case 0x04: //终端复位、退出静默
        t_rst.rst = true;
        t_rst.del = false;
        t_rst.send = false;
        t_rst.jiff = jiffies;
        break;
    case 0x05: //恢复出厂设置
#if (P_RCD == RCD_BASE)
        init_mfg_parameter();
#endif
        reset_system_parameter(R_F_EXCEPT_IP);

        flash_write_normal_parameters();

        break;
    case 0x06:
        break;
    case 0x07:
        break;

    default:
        break;
    }

    pack_general_answer(0);
}

static void rec_get_terminal_property(u16 index, u8 *str, u16 s_len) {
    u8 msg[512] = {0};
    u16 m_len = 0;
    u8 i;
    u8 len = 0;
    u8 *pf;
    //    u8 ucl;

    m_len = 0;
    msg[m_len++] = 0x00;
    msg[m_len++] = 0x0f;

    m_len += _memcpy_len(&msg[m_len], (u8 *)&register_cfg.mfg_id, 5);
    _memcpy_len(&msg[m_len], "LH-16BX", 7); //2014-11-11 修改

    m_len += 20;
    m_len += _memcpy_len(&msg[m_len], (u8 *)&register_cfg.terminal_id, 7);

    for (i = 0; i < 10; i++)
        msg[m_len++] = _chartohex(mx.imei[i * 2], mx.imei[i * 2 + 1]);
    pf = msg + m_len; //msg[m_len++] = 160;
    m_len++;
    len = m_len;
#if 0    
    if( register_cfg.ztb_gk == 4 )    
    	m_len += _memcpy_len(&msg[m_len], "管控开,", 7);
    else
       	m_len += _memcpy_len(&msg[m_len], "管控关,", 7);
    
	if(ztc_status.car_rise)
		m_len += _memcpy_len(&msg[m_len], "限制举升,", 9);
	else
		m_len += _memcpy_len(&msg[m_len], "不限举升,", 9);

	if (pub_io.b.car_cover) 
		m_len += _memcpy_len(&msg[m_len], "环保盖关,", 9);
	else
		m_len += _memcpy_len(&msg[m_len], "环保盖开,", 9);		

	if (pub_io.b.car_box) 
		m_len += _memcpy_len(&msg[m_len], "车斗举升,", 9);		
	else
		m_len += _memcpy_len(&msg[m_len], "车斗平放,", 9);

	if (ztc_status.car_max_load == true)
		m_len += _memcpy_len(&msg[m_len], "重载,", 5);
	else
		m_len += _memcpy_len(&msg[m_len], "空载,", 5);	

	m_len += _sprintf_len(&msg[m_len], "限速速度=%3d,", limit_speed);		  //13
	m_len += _sprintf_len((char *)&msg[m_len], "多边形个数=%d\r\n", uPolygon);
#endif

    *pf = m_len - len;
    msg[m_len++] = 24;
    m_len += _memcpy_len(&msg[m_len], FW_VERSION, 24);

    msg[m_len++] = 3;
    msg[m_len++] = 0xff;

    pack_any_data(CMD_UP_GET_PROPERTY, msg, m_len, PK_HEX, (netx));
}

//自定义协议 0x0190 自动上报
void rec_send_customer_property(void) {
    u8 msg[512] = {0};
    u16 m_len = 0;
    u8 i;

    m_len = 0;

    _memcpy_len(&msg[m_len], "TR9", 3); //2014-11-11 修改

    m_len += 20;
    //_memcpy_len(&msg[m_len], (u8 *)&s_Customer_idc.customer_id, s_Customer_idc.customer_id[10]);
    m_len += 10;
    i = _strlen(server_cfg.main_ip);
    msg[m_len++] = i;
    m_len += _memcpy_len(&msg[m_len], (u8 *)&server_cfg.main_ip, i);
    _sw_endian(&msg[m_len], (u8 *)&server_cfg.main_port, 2);
    m_len += 2;

    msg[m_len++] = 13;
    m_len += _memcpy_len(&msg[m_len], TR9_SOFT_VERSION_APP, 13);
    loge("len err, 13");

    pack_any_data(0x0190, msg, m_len, PK_HEX, LINK_IP3);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*********************************************************
名	  称：rec_update_software
功	  能：升级数据或控制命令
功 能 号: 0x8180
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_update_software(u16 index, u8 *str, u16 s_len) {
    u8 ret = 0;
    u8 u_type = 0;
    u16 pack_no = 0;

    if (*str == 0x01) {
        str++;
        u_type = *str++;
        u_type = (u_type == 0x01) ? UPGRADE_BYPASS : UPGRADE_AUTO;
        if (u_type == 0x00) {
            _memset((u8 *)&upgrade_m, 0x00, sizeof(upgrade_m));
            lm.reset = true;
            lm.IP2_status = LINK_VOID;
            lm.cur_ip = LINK_IP0;
            return;
        }
        upgrade_m.filename_len = *str++;
        if (upgrade_m.filename_len < 24 || upgrade_m.filename_len > 30)
            return;

        _memcpy((u8 *)&upgrade_m.filename, str, upgrade_m.filename_len);
        ret = check_update_version(u_type, (u8 *)&upgrade_m.filename, upgrade_m.filename_len);

        upgrade_m.ack = true;

        if (ret == 0) {
            logd("不用升级或正在升级!");
        } else if (upgrade_m.enable == true) {
            upgrade_m.enable = true;
            upgrade_m.mode = UPGRADE_OTA;

            init_net_info(); //加快升级进度
            logd("重新开始升级，等待升级数据包0...");

        } else {
            upgrade_m.enable = true;
            upgrade_m.mode = UPGRADE_OTA;
            init_net_info(); //加快升级进度
            logd("开始升级，等待升级数据包1...");
        }
    } else if (*str == 0x02) {
        str++;
        _sw_endian((u8 *)&upgrade_m.pack_num, str, 2);
        str += 2;
        _sw_endian((u8 *)&upgrade_m.file_len, str, 4);
        str += 4;
        upgrade_m.xor = *str++;
        _sw_endian((u8 *)&upgrade_m.sum, str, 4);
        logd("开始升级，等待升级数据包2...");

        upgrade_m.banben = true;

    } else if (*str == 0x03) {
        str++;
        _memset((u8 *)&update_buf, 0, 530);
        if (upgrade_m.enable == true && upgrade_m.mode == UPGRADE_OTA) {
            upgrade_m.buf_len = s_len - 1;
            _memcpy((u8 *)&update_buf, str, upgrade_m.buf_len);

            pack_no = update_buf[0];
            pack_no <<= 8;
            pack_no += update_buf[1];

            upgrade_m.ack = (pack_no == upgrade_m.pack_no) ? true : false;
        }
    }
}

/*********************************************************
名	  称：Ask_Position
功	  能：查询当前位置
功 能 号: 0x8201
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_ask_position(u16 index, u8 *str, u16 s_len) {
    u8 base_buf[MAX_NET_PACKAGE];
    u16 len;

    len = 0;
    len += _sw_endian(base_buf, (u8 *)&rev_inf.run_no, 2);
    len += pack_gps_base_info(&base_buf[len]);

    pack_any_data(CMD_UP_ASK_POS, base_buf, len, PK_NET, (netx));
}

/*********************************************************
名	  称：rec_tmp_pos_trace
功	  能：平台临时位置跟踪控制
功 能 号: 0x8202
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：更新上报时间间隔、并把数值保存到跟踪时间体
*********************************************************/
static void rec_tmp_pos_trace(u16 index, u8 *str, u16 s_len) {
    _sw_endian((u8 *)&tmp_trace_cfg.interval, str, 2);
    str += 2;
    _sw_endian((u8 *)&tmp_trace_cfg.time, str, 4); //单位秒
    tmp_trace_cfg.open = (tmp_trace_cfg.time == 0 || tmp_trace_cfg.time == 0) ? 0 : 1;

    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_man_confirm_remove
功	  能：人工确认解除
功 能 号: 0x8203
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项
*********************************************************/
static void rec_man_confirm_remove(u16 index, u8 *str, u16 s_len) {
    VEHICLE_ALARM_UNION rc;

    str += 2; //人工取消报警流水号

    _sw_endian((u8 *)&rc.reg, str, 4);

    bypass_alarm.reg |= (car_alarm.reg & rc.reg);

    //添加报警取消
    //注意是否合理
    car_alarm.bit.drive_over = false;
    car_alarm.bit.sos = false;

    pack_general_answer(0);
}

//设置文本密码 #password=20150215,20200202#
void txt_set_password(u8 *pStr, u16 iStrLen) {
    u8 phone_tmp[16] = {0};
    u8 len = 0;
    u8 *pf;
    //    u8 dat;
    u8 i;

    len = _memcpyCH(phone_tmp, ',', pStr, 9);
    iStrLen -= len;
    if (_strncmp(phone_tmp, (password_manage.para + 2), (*((u16 *)(&password_manage.para)))) != 0) {
        logd("old password err! len = %d", len);
        return;
    }
    pStr += len;
    _memset(phone_tmp, 0, 16);
    len = _memcpyCH(phone_tmp, '#', pStr, 9);
    len--;
    if (pStr[len] == '#') {
        _memset((u8 *)&password_manage.para, 0x00, password_para_max_lgth + 2); //'2'
        pf = (u8 *)&password_manage.para + 2;
        for (i = 0; i < len; i++) {
            *pf++ = *pStr++;
        }
        (*((u16 *)(&password_manage.para))) = len;
        flash25_program_auto_save((flash_parameter_addr + flash_password_addr), ((u8 *)(&password_manage)), (sizeof(password_manage)));
    }

    logd("set password len = %d<%s>", len, phone_tmp);
}

/*********************************************************
名	  称：D_Send_Message_Handle
功	  能：下发短信息给手柄
功 能 号: 0x8300
输入参数：缓冲区地址，长度
输出参数：
返	  回：
注意事项：数据长度的确定(g_RecGprsBuffLen)
*********************************************************/
static void rec_send_message(u16 index, u8 *str, u16 s_len) {
    u8 msg[260] = {0};
    u8 baud[6] = {0};
    u8 i;
    //    u16 speed;
    //    u16 tmr;
    //    u16 len;

    str++;

    if (_strncmp(str, "#XS Value=", 10) == 0) {
        s_len -= 10;
        str += 10;
        if (s_len >= 5)
            s_len = 5;
        for (i = 0; i < s_len; i++) {
            if ((*str <= 0x39) && (*str >= 0x30))
                baud[i] = *str++;
        }
        register_cfg.blank_car_weight = atoi((char *)baud);

        if (register_cfg.blank_car_weight > 19000)
            register_cfg.blank_car_weight = 11000;
        flash_write_import_parameters();
    } else if (_strncmp(str, "#XS Value?#", 11) == 0) //请求版本
    {
        _memcpy(msg + 1, (uc8 *)&"#CX Value=", 10);
        s_len = 11;
        s_len += _sprintf_len((char *)&msg[s_len], "=%d,", register_cfg.blank_car_weight);
        msg[s_len] = '\0';
        pack_any_data(0x0300, msg, s_len, PK_NET, (netx));
    } else if (_strncmp(str, "#XS ACC open#", 13) == 0) {
        register_cfg.acc_simulate = 0x01;
        car_state.bit.acc = true;
        flash_write_import_parameters();
    } else if (_strncmp(str, "#XS ACC close#", 14) == 0) {
        register_cfg.acc_simulate = 0x02;
        car_state.bit.acc = false;
        flash_write_import_parameters();
    } else if (_strncmp(str, "#XS Car Lock#", 13) == 0) {
        run.lock_car_sw = 0x55;
        tr9_car_status.bit.car_lock = true;
        save_run_parameterer();
    } else if (_strncmp(str, "#XS Car Unlock#", 15) == 0) {
        run.lock_car_sw = 0x00;
        save_run_parameterer();
        tr9_car_status.bit.car_lock = false;
    } else if (_strncmp(str, "#dr:", 4) == 0) { //新版本更改//文件信息-写-IC-card://#dr://6003
        //loge("//新版本更改//文件信息-写-IC-card://#dr://6003");
        khd_set_card_message(str, s_len);
    }

    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_event_set
功	  能：事件设置
功 能 号: 0x8301
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_event_set(u16 index, u8 *str, u16 s_len) {
    handle_send_ext_msg(FROM_INNER, CMD_DN_EVENT_SET, str, s_len);
    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_question_down
功	  能：提问下发
功 能 号: 0x8302
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：添加消息流水号
*********************************************************/
static void rec_question_down(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_mess_menu_set
功	  能：信息点播菜单设置
功 能 号: 0x8303
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_mess_menu_set(u16 index, u8 *str, u16 s_len) {
    handle_send_ext_msg(FROM_INNER, CMD_DN_MENU_SET, str, s_len);
    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_mess_server
功	  能：信息服务
功 能 号: 0x8304
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_mess_server(u16 index, u8 *str, u16 s_len) {
    handle_send_ext_msg(FROM_INNER, CMD_DN_SMS_SERVE, str, s_len);
    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_vehi_control
功	  能：车辆控制
功 能 号: 0x8500
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_vehi_control(u16 index, u8 *str, u16 s_len) {
    u8 buf[250] = {0x00};
    u16 len;
    u8 state;

    state = *str;
    pack_general_answer(0);

    if ((state & 0x80) == 0x80) {
#if (P_SOUND == SOUND_BASE)
        if (!rcmd.b.cut_oil)
            write_tts(TTS_PLAY_CUT_OIL_DTAE, TTS_PLAY_CUT_OIL_LEN);
#endif
        rcmd.b.cut_oil = true;
        rcmd.b.rels_oil = false;
    } else {
        rcmd.b.cut_oil = false;
        rcmd.b.rels_oil = true;
    }

    len = 0;
    len += _sw_endian(buf, (u8 *)&rev_inf.run_no, 2);
    len += pack_gps_base_info(&buf[len]); //位置

    pack_any_data(CMD_UP_CAR_CTRL, buf, len, PK_NET, (netx));
}

/*********************************************************
名	  称：rec_oil_control
功	  能：油路控制
功 能 号: 0x8580
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
void rec_oil_control(u16 index, u8 *str, u16 s_len) {
    u16 len;
    u8 state;
    u8 buf[300];

    //if (s_Customer_idc.lock == 0x55)
    //    return;

    state = *str;
    pack_general_answer(0);

    if ((state & 0x01) == 0x01) //智能关闭油路
    {
#if (P_SOUND == SOUND_BASE)
        if (!rcmd.b.cut_oil)
            write_tts(TTS_PLAY_CUT_OIL_DTAE, TTS_PLAY_CUT_OIL_LEN);
#endif
        rcmd.b.cut_oil = true;
        rcmd.b.rels_oil = false;
    } else {
        rcmd.b.cut_oil = false;
        rcmd.b.rels_oil = true;
    }

    len = 0;
    len += _sw_endian(buf, (u8 *)&rev_inf.run_no, 2);
    len += pack_gps_base_info(&buf[len]); //位置

    pack_any_data(CMD_UP_OIL_CTRL, buf, len, PK_NET, (netx));
}

/*********************************************************
名	  称：rec_driver_state
功	  能：查询驾驶员身份信息
功 能 号: 0x8702
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_driver_state(u16 index, u8 *str, u16 s_len) {
    handle_send_ext_msg(FROM_INNER, CMD_DOWN_DRIVER_STATE, str, s_len);
    pack_general_answer(0);
}

//返回主机状态到平台
u16 get_ztc_status_to_center(u8 *buf) {
    u16 len = 0;
    u8 tmp[6];
    _memset(buf, 0, 250);

    buf[0] = '<';
    len = 1;

    if (mix.car_state.bit.acc == true)
        len += _sprintf_len((char *)&buf[len], "ACC ON,");
    else
        len += _sprintf_len((char *)&buf[len], "ACC OFF,");

    if (run.lock_car_sw == 0x55)
        len += _memcpy_len(&buf[len], "锁车开,", 7);
    else
        len += _memcpy_len(&buf[len], "锁车关,", 7);
    if (register_cfg.ill_sw == _ZB_SW_CLOSE)
        len += _memcpy_len(&buf[len], "管控开,", 7);
    else
        len += _memcpy_len(&buf[len], "管控关,", 7);

    if (pub_io.b.left_cover)
        len += _sprintf_len((char *)&buf[len], "左盖开,");
    else
        len += _sprintf_len((char *)&buf[len], "左盖关,");

    if (pub_io.b.right_cover)
        len += _sprintf_len((char *)&buf[len], "右盖开,");
    else
        len += _sprintf_len((char *)&buf[len], "右盖关,");

    if (pub_io.b.car_box)
        len += _sprintf_len((char *)&buf[len], "举升开,");
    else
        len += _sprintf_len((char *)&buf[len], "举升关,");

    if (run.area_in_out == _AREA_OUT)
        len += _sprintf_len((char *)&buf[len], "出围栏,");
    else
        len += _sprintf_len((char *)&buf[len], "进围栏,");

    if (car_alarm.bit.vss_err)
        len += _sprintf_len((char *)&buf[len], "ECU连接异常,");
    else
        len += _sprintf_len((char *)&buf[len], "ECU连接正常,");

    len += _sprintf_len((char *)&buf[len], "多边形个数=%d,", uPolygon);

    _strncpy((u8 *)&buf[len], (u8 *)FW_VERSION, 24);
    len += 24;

    len += _sprintf_len((char *)&buf[len], "%s,%s,%d,", server_cfg.apn, server_cfg.main_ip, server_cfg.main_port);
    len += _sprintf_len((char *)&buf[len], "%s,%d", server_cfg.bak_ip, server_cfg.bak_port);
    len += _sprintf_len((char *)&buf[len], ",ID:%s", server_cfg.terminal_id);

    if (1) {
        loge("err err err err err err err err err err err err ");
        _memcpy(tmp, (u8 *)&register_cfg.license[2], 6);
        tmp[5] = 0;
        len += _sprintf_len((char *)&buf[len], ":%s", tmp);
        buf[len++] = '>';
    }

    return len;
}

/*********************************************************
名	  称：rec_transparent_transmission
功	  能：透传数据到串口
功 能 号: 0x8900
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：透传到串口 5
*********************************************************/
static void rec_transparent_transmission(u16 index, u8 *str, u16 s_len) {
    u16 len = 0;

    str++; //透传类型---代表不同外设
    len = s_len - 1;
    if (len == 0 || len > 600) {
        goto D_Pass_out;
    }

    if (u1_m.cur.b.tt == true) {
        send_any_uart(FROM_U1, str, len);
    }

D_Pass_out:
    pack_general_answer(0);
}

#if (P_AREA == AREA_BASE)
/*********************************************************
名	  称：rec_set_round
功	  能：设置圆形区域
功 能 号: 0x8600
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_set_round(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(1);
}

/*********************************************************
名	  称：rec_del_round
功	  能：删除圆形区域
功 能 号: 0x8601
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_del_round(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_set_recta
功	  能：设置距形区域
功 能 号: 0x8602
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_set_recta(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(1);
}

/*********************************************************
名	  称：rec_del_recta
功	  能：删除距形区域
功 能 号: 0x8603
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_del_recta(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(0);
}

/*********************************************************
名	  称：rec_set_polygon
功	  能：设置多边形区域
功 能 号: 0x8604
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_set_polygon(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(1);
}

/*********************************************************
名	  称：rec_del_polygon
功	  能：删除多边形区域
功 能 号: 0x8605
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_del_polygon(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(0);
}

/*********************************************************
名	  称：Rec_Set_Route
功	  能：设置路线
功 能 号: 0x8606
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_set_road(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(0);
}

/*********************************************************
名	  称：Rec_Del_Route
功	  能：删除路线
功 能 号: 0x8607
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：
*********************************************************/
static void rec_del_road(u16 index, u8 *str, u16 s_len) {
    pack_general_answer(0);
}
#endif

#if (P_RCD == RCD_BASE)
static void rec_retry_pack(u16 index, u8 *str, u16 s_len) {
    u8 num = 0;

    if (str == NULL || s_len > 250) {
        logd("rec_retry_pack err :1");
        return;
    }

    //去掉流水号
    str++;
    str++;

    num = *str++;
    if (num > 100) {
        pack_general_answer(1);
        return;
    }

    if (num == 0) {
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        pack_general_answer(0);
        return;
    }

    spi_flash_erase(_retry_addr(0));
    spi_flash_write(str, _retry_addr(0), (num * 2));

    rsp.retry_en = true;
    rsp.retry_cnt = num;
    rsp.retry_no = 0;
    rsp.retry_delay_tick = tick;
    pack_general_answer(0);
}

/*********************************************************
名	  称：Rec_Pass_Through
功	  能：透传数据到串口
功 能 号: 0x8700  8701
输入参数：缓冲区地址，
输出参数：
返	  回：
注意事项：透传到串口 5
*********************************************************/
static void rec_rcd_data_get(u16 index, u8 *str, u16 s_len) {
    u8 id_t = 0;
    u16 limit = 0;
    u16 len_t = 0;

    limit = 7;

    if (str == NULL && s_len <= limit) {
        pack_general_answer(1);
        return;
    }

    id_t = *str;

    if (s_len >= (limit + 1)) {
        str += limit;
        len_t = s_len - limit;
    } else {
        len_t = 0;
    }

    rcd_communicate_parse(FROM_NET, id_t, str, len_t);
    pack_general_answer(0);
}
#endif

/////////////////////////////////////////////////
//6	命令值	BYTE	0：解锁  1：锁车
/////////////////////////////////////////////////
static void fz_ztc_lock_car(u16 index, u8 *str, u16 s_len) {
    //    u8 cmd;

    str += 6;
    /*cmd =*/*str++;

    pack_general_answer(0);
}

/////////////////////////////////////////////////
// 限速  0~255，0：不限速；其他：限制的具体时速。单位：km/h
/////////////////////////////////////////////////
static void fz_ztc_speed_limit(u16 index, u8 *str, u16 s_len) {
    //    u8 cmd;

    str += 6;
    /*cmd =*/*str++;

    pack_general_answer(0);
}

/////////////////////////////////////////////////
// 限制举升  0x8f03
/////////////////////////////////////////////////
static void fz_ztc_limit_rise(u16 index, u8 *str, u16 s_len) {
    //    u8 cmd;

    str += 6;
    /*cmd = */ *str++;

    pack_general_answer(0);
}

/////////////////////////////////////////////////
// 管控解锁  0x8f04
/////////////////////////////////////////////////
static void fz_ztc_control_lock(u16 index, u8 *str, u16 s_len) {
    u8 cmd;

    str += 6;
    cmd = *str++;

    if (cmd == 0x02) //关闭管控
    {
        logd("收到关闭管控指令=%d", cmd);
        register_cfg.ztb_gk = 0;
        flash_write_import_parameters();
    } else if (cmd == 0x04) {
        logd("收到打开管控指令=%d", cmd);
        register_cfg.ztb_gk = 0x55;
        flash_write_import_parameters();
    }
    pack_general_answer(0);
}

/////////////////////////////////////////////////
// 报警提示信息定义  0x8f06
/////////////////////////////////////////////////
static void fz_ztc_alarm_hint(u16 index, u8 *str, u16 s_len) {
    //	u8 cmd;
    u8 buf[500];
    str++;
    str += 7;

    if (s_len > 8)
        s_len -= 8;
#if (P_SOUND == SOUND_BASE)
    write_tts(str, s_len);
#endif
    _memset(buf, 0, 500);
    _memcpy(buf, str, s_len);
    //	logd("收到报警提示信息<%s>", buf);

    pack_general_answer(0);
}

/////////////////////////////////////////////////
/* 新增区域  0x8f08
	0	消息ID	WORD	0x8F08
2	标识ID	DWORD	区域的唯一标识（保证此ID唯一性且平台永久记录此ID。不能简单使用流水号）。
6	管理类型	BYTE	1工地，2、消纳场，3、限速圈，4、禁区，5、停车场
7	业务值	BYTE	0~255，0表示不限速，其他表示限速时速值。
8	点数	WORD	点的个数。
10	经度	DWORD	以度为单位的纬度值乘以10的6次方，精确到百万分之一度
14	纬度	DWORD	以度为单位的纬度值乘以10的6次方，精确到百万分之一度
8F08 003A 013800002222  4618  8F08 0000673A  01 00 0006 
071C7C10 018DAFC6
071C7E70 018DAFCE
071C8120 018DAFD8
071C8158 018DAD8C
071C7E68 018DABF8
071C79E0 018DAD26  14
*/
// 业务逻辑转换为 多边形区域
// 属性的高3位 13-14-15 用于标识 渣土办的 区域属性
/////////////////////////////////////////////////
static void fz_ztc_add_area(u16 index, u8 *str, u16 s_len) {
    u8 cmd;
    u16 messId;
    u8 polygon[300];
    u8 *pf;
    u8 *rp;
    u16 i;

    str += 2; //8f08

    pf = polygon;
    _memset(polygon, 0, 300);
    _memcpy(pf, str, 4); //区域ID
    str += 4;
    pf += 4;
    i = *str++; //管理类型  1工地，2、消纳场，3、限速圈，4、禁区，5、停车场
        //赋值给区域属性 用于限速、限举做相应的动作
    messId = 0x3E;
    //if( i == 0x04 )
    //messId |= 0x1000;
    i = i & 0x07;
    i = i << 9;
    messId |= i;
    _printf_tmp("Add messId = %d\r\n", messId);

    _sw_endian(pf, (u8 *)&messId, 2); //区域属性
    pf += 2;

    //pf += 12;  		//起止时间、结束时间
    cmd = *str++; //速度
    messId = cmd;
    _sw_endian(pf, (u8 *)&messId, 2); //最高速度
    pf += 2;
    *pf++ = 10; //持续时间

    //pf += 4;  				//速度、拍照、点火、熄火
    _memcpy(pf, str, 2);               //个数
    _sw_endian((u8 *)&messId, str, 2); //最高速度
    str += 2;
    pf += 2;

    s_len = 27;
    for (i = 0; i < messId; i++) {
        rp = pf + 4;
        _memcpy(rp, str, 4);
        str += 4;
        _memcpy(pf, str, 4);

        str += 4;
        pf += 8;
        s_len += 8;
    }

    //添加多边形区域
    //area_set_polygon(polygon, s_len);

    logd("收到多边形设置指令=%d", 1);
    pack_general_answer(0);

    //限制举升
    ctrl_relay_status(1);
}

/////////////////////////////////////////////////
// 删除区域线路  0x8f09
/*
0	消息ID	WORD	0x8F09
2	操作类型	BYTE	1删除；2清空（清空时无下述字段）
3	删除数量	BYTE	
4	标识ID	DWORD	待删除的区域线路的唯一标示ID
	……	……	……
	标识IDn	DWORD	待删除的区域线路的唯一标示ID
*/
/////////////////////////////////////////////////
static void fz_ztc_del_area_road(u16 index, u8 *str, u16 s_len) {
    //	u8 cmd;
    u8 polygon[200];
    //    u8 *pf;

    //    pf = polygon;
    str += 2;
    if (s_len > 2)
        s_len -= 2;
    _memset(polygon, 0, 200);

    if (*str == 2) //清空
        s_len = 1;
    else {
        str++;
        if (s_len > 1)
            s_len -= 1;
        if (s_len > 200)
            s_len = 200;
        _memcpy(polygon, str, s_len);
    }
    //area_del_polygon(polygon, s_len);
    //area_del_road(polygon, s_len);
    pack_general_answer(0);

    logd("收到删除多边形、路线指令=%d", 1);
    write_tts(TTS_DEL_ROAD_POLOGON_CMD, TTS_DEL_ROAD_POLOGON_CMD_LEN);
}

/////////////////////////////////////////////////
// 新增线路  0x8f0b
/*
8F0B 004A 013800002222 494C 8F0B 0000673B  64 14 0008
071C7678 018DAE1C
071C7778 018DB1F2
071C79E8 018DB388
071C7F88 018DB39C
071C7FF0 018DAE56
071C8000 018DAAB8
071C7C80 018DAA44
071C78A8 018DA876  29
*/
/////////////////////////////////////////////////
static void fz_ztc_add_road(u16 index, u8 *str, u16 s_len) {
    u8 cmd;
    u16 messId;
    u16 tmp16;
    u32 tmp32;
    u8 road[800];
    u8 *pf;
    u8 *rp;
    u8 i;
    u8 speed;

    _memset(road, 0, 800);
    str += 2; ///消息ID
    pf = road;
    _memcpy(pf, str, 4); //区域ID
    str += 4;
    pf += 4;

    messId = 0x3c;
    _sw_endian(pf, (u8 *)&messId, 2); //线路属性
    pf += 2;

    //pf += 12;
    cmd = *str++;   //线宽
    speed = *str++; //速度

    _sw_endian((u8 *)&messId, str, 2); //拐点个数
    _memcpy(pf, str, 2);
    str += 2;
    pf += 2;

    s_len = 20;
    ///拐点项
    for (i = 0; i < messId; i++) {
        tmp32 = i + 1;
        _sw_endian(pf, (u8 *)&tmp32, 4); //拐点ID
        pf += 4;
        _sw_endian(pf, (u8 *)&tmp32, 4); //路段ID
        pf += 4;

        rp = pf + 4;
        _memcpy(rp, str, 4); //拐点经度
        //pf += 4;
        str += 4;
        _memcpy(pf, str, 4); //拐点纬度
        pf += 8;
        str += 4;
        *pf++ = cmd;  //路宽属性
        *pf++ = 0x02; //路段属性  -- 限速

        tmp16 = speed;
        _sw_endian(pf, (u8 *)&tmp16, 2);
        pf += 2;

        s_len += 20;
    }

    //area_set_road(road, s_len);
    logd("收到新增路线指令=%d", 1);
    pack_general_answer(0);
    write_tts(TTS_SET_ROAD_CMD, TTS_SET_ROAD_CMD_LEN);
}

cmd_net const NetTab[] = {
    {0x8001, rec_sure_response, 0, NULL},
#if (P_RCD == RCD_BASE)
    {0x8003, Rec_retry_pack, 0, NULL},
#endif
    {0x8100, rec_login_return, 0, NULL},
    {0x8103, rec_set_terminal_para, 0, NULL},
    {0x8104, rec_view_terminal_all_para, 0, NULL},
    {0x8105, rec_terminal_control, 0, NULL},
    {0x8106, rec_view_terminal_any_para, 0, NULL},
    {0x8107, rec_get_terminal_property, 0, NULL},
    {0x8180, rec_update_software, 0, NULL},

    {0x8201, rec_ask_position, 0, NULL},
    {0x8202, rec_tmp_pos_trace, 0, NULL},
    {0x8203, rec_man_confirm_remove, 0, NULL},
    {0x8300, rec_send_message, 0, NULL},
    {0x8301, rec_event_set, 0, NULL},
    {0x8302, rec_question_down, 0, NULL},
    {0x8303, rec_mess_menu_set, 0, NULL},
    {0x8304, rec_mess_server, 0, NULL},
    {0x8500, rec_vehi_control, 0, NULL},
    {0x8580, rec_oil_control, 0, NULL},
#if (P_AREA == AREA_BASE)
    {0x8600, rec_set_round, 0, NULL},
    {0x8601, rec_del_round, 0, NULL},
    {0x8602, rec_set_recta, 0, NULL},
    {0x8603, rec_del_recta, 0, NULL},
    {0x8604, rec_set_polygon, 0, NULL},
    {0x8605, rec_del_polygon, 0, NULL},
    {0x8606, rec_set_road, 0, NULL},
    {0x8607, rec_del_road, 0, NULL},
#endif
#if (P_RCD == RCD_BASE) //行驶记录仪相关
    {0x8003, rec_retry_pack, 0, NULL},
    {0x8700, rec_rcd_data_get, 0, NULL},
    {0x8701, rec_rcd_data_get, 0, NULL},
#endif

    {0x8702, rec_driver_state, 0, NULL},
    {0x8900, rec_transparent_transmission, 0, NULL},

    {0x8f01, fz_ztc_lock_car, 0, NULL},
    {0x8f02, fz_ztc_speed_limit, 0, NULL},
    {0x8f03, fz_ztc_limit_rise, 0, NULL},
    {0x8f04, fz_ztc_control_lock, 0, NULL},
    {0x8f06, fz_ztc_alarm_hint, 0, NULL},    //报警提示
    {0x8f08, fz_ztc_add_area, 0, NULL},      //新增区域
    {0x8F08, fz_ztc_add_area, 0, NULL},      //新增区域
    {0x8f09, fz_ztc_del_area_road, 0, NULL}, //删除区域路线
    {0x8f0b, fz_ztc_add_road, 0, NULL},      //新增路线
    {NULL, NULL, 0, NULL}};

/*********************************************************
名    称：gprs_parse_task
功    能：GPRS 数据解析
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
void jt808_net_parse(u8 *p, u16 p_len) {
    u8 i;
    u16 index = 0;
    u8 *str;
    u16 s_len;
    u8 *pt;

    pt = p;
    putHexs_hex(p, p_len);

    index = *pt++;
    index = (index & 0x00ff) << 8;
    index += *pt++;

    str = get_rec_center_head(p, &p_len);
    s_len = p_len;

    for (i = 0; NetTab[i].func != NULL; i++) {
        if (index == NetTab[i].index) {
            lm.ack = true;
            //logd("net index = 0x%04X", index);
            (*NetTab[i].func)(index, str, s_len);
            break;
        }
    }
}
