/**
  ******************************************************************************
  * @file    sms_parse.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   短信功能解析
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

/****************************************************************************
* 名称：del_phone_code86
* 功能：删除电话号码中的国家代码86
* 入口参数：p_phonecode指向电话号码的指针
* 出口参数：无
****************************************************************************/
void del_phone_code86(u8 *p_phonecode) {
    u8 i;

    if ((*p_phonecode == '+') && (*(p_phonecode + 1) == '8') && (*(p_phonecode + 2) == '6')) {
        for (i = 0; i < 20; i++) {
            *p_phonecode = *(p_phonecode + 3);
            if (*p_phonecode == '\0')
                break;
            else
                p_phonecode++;
        }
    } else if ((*p_phonecode == '8') && (*(p_phonecode + 1) == '6')) {
        for (i = 0; i < 20; i++) {
            *p_phonecode = *(p_phonecode + 2);
            if (*p_phonecode == '\0')
                break;
            else
                p_phonecode++;
        }
    }
}

/****************************************************************************
* 名称：get_pdu_content
* 功能：解析PDU 实际信息内容
* 入口参数：p_phonecode指向电话号码的指针
* 出口参数：无
****************************************************************************/
int get_pdu_content(u8 *pSm, u8 *buf) {
    int code_len, i, uMsgLen;

    code_len = _chartohex(pSm[20], pSm[21]);

    if (code_len % 2) {
        uMsgLen = _chartohex(pSm[code_len + 43], pSm[code_len + 44]);
        for (i = 0; i < uMsgLen; i++)
            buf[i] = _chartohex(pSm[code_len + 45 + 2 * i], pSm[code_len + 46 + 2 * i]);
    } else {
        uMsgLen = _chartohex(pSm[code_len + 42], pSm[code_len + 43]);
        for (i = 0; i < uMsgLen; i++)
            buf[i] = _chartohex(pSm[code_len + 44 + 2 * i], pSm[code_len + 45 + 2 * i]);
    }

    return uMsgLen;
}

/*
短信接收和解析程序
*/
static u8 *cbuf;
static u8 *rbuf;

/*********************************************************
名    称：sms_query_parameter
功    能：提取IP 参数
输入参数：无
输    出：无
编写日期：2013-12-01
///  8002 Update
///  8002 Update:"1, 203.86.8.5, 9000"
**********************************************************/
static void sms_ota_new_ver(void) {
    IP_SERVER_CONFIG_STRUCT m1;
    u8 buf[250];
    char *tok;
    u8 len = 0;
    bool is_tcp = false;

    len = _strlen(cbuf);
    if (len > 50 || len < 15)
        goto OTA_new_ver_err;

    _memcpy(buf, cbuf, len);
    buf[len] = '\0';

    if (!sms_s.cmp_en) {
        _strcpy(rbuf, "You do not have permissions");
        return;
    }
    if (_strncmp(buf, "TongRen Update:", 15) == 0) {
        /*TongRen Update:"  */
        if ((tok = strtok((char *)&buf, ":\"")) == NULL)
            goto OTA_new_ver_err;

        /*解析TCP/UDP , 去掉", "*/
        if ((tok = strtok(NULL, ", ")) != NULL) {
            if (*tok == '\"')
                tok++;
            if (*tok == '0' || *tok == '1') {
                if (*tok == '0')
                    is_tcp = false;
                else
                    is_tcp = true;
            } else
                goto OTA_new_ver_err;
        } else
            goto OTA_new_ver_err;

        /*解析IP , 去掉", "*/
        if ((tok = strtok(NULL, ",\"")) != NULL)
            strcpy((char *)&m1.ota_ip, tok);
        else
            goto OTA_new_ver_err;

        /*解析端口号, 去掉", " */
        if ((tok = strtok(NULL, ",\"")) != NULL) {
            m1.ota_port = atoi((const char *)tok);

            if (m1.ota_port < 1000 || m1.ota_port > 50000)
                goto OTA_new_ver_err;
            server_cfg.ota_port = m1.ota_port;

            len = _strlen(m1.ota_ip);
            if (len >= 23)
                goto OTA_new_ver_err;
            _memset((u8 *)&server_cfg.ota_ip, 0x00, 24);
            _memcpy((u8 *)&server_cfg.ota_ip, (u8 *)&m1.ota_ip, len);

            if (is_tcp)
                server_cfg.select_tcp |= 0x04;
            else
                server_cfg.select_tcp &= (~0x04);
            goto OTA_new_ver_ok;
        } else {
            goto OTA_new_ver_err;
        }
    } else if (_strncmp(buf, "TongRen Update", 15) == 0) {
        _memset((u8 *)&server_cfg.ota_ip, 0x00, 24);
        _memcpy((u8 *)&server_cfg.ota_ip, "203.086.008.005", 15);
        server_cfg.ota_port = 9000;
        server_cfg.select_tcp |= 0x04;
    OTA_new_ver_ok:
        _memset((u8 *)&upgrade_m, 0x00, sizeof(upgrade_m));
        upgrade_m.filename_len = _memcpy_len((u8 *)&upgrade_m.filename, FW_VERSION, 24);
        upgrade_m.enable = true;
        upgrade_m.mode = UPGRADE_OTA;
        upgrade_m.newest = true;
        lm.sms_rst_en = true;
        lm.sms_rst_tick = tick;

        len = 0;
        is_tcp = ((server_cfg.select_tcp & 0x01) == 0x00) ? false : true;
        len = _sprintf_len((char *)&buf, "Update OK, MAIN_IP:\"%s\",\"%d,%s\",\"%d\" ", server_cfg.apn, is_tcp, server_cfg.main_ip, server_cfg.main_port);
        is_tcp = ((server_cfg.select_tcp & 0x02) == 0x00) ? false : true;
        len += _sprintf_len((char *)&buf[len], "BACKUP_IP:\"%s\",\"%d,%s\",\"%d\" ", server_cfg.apn, is_tcp, server_cfg.bak_ip, server_cfg.bak_port);
        len += _sprintf_len((char *)&buf[len], "ID:\"%s\" ", server_cfg.terminal_id);
        buf[len] = '\0';

        if (len > 140)
            goto OTA_new_ver_err;
        _strcpy(rbuf, buf);
        return;
    } else {
    OTA_new_ver_err:
        _strcpy(rbuf, "Update Set Error");
        return;
    }
}

/*********************************************************
名    称：sms_query_parameter
功    能：提取IP 参数
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
static void sms_query_parameter(void) {
    u8 buf[250] = {0x00};
    u16 len = 0;
    u8 is_tcp = 0;
    u8 *pf;
    u8 len1 = 0;

    len = 0;
    is_tcp = ((server_cfg.select_tcp & 0x01) == 0x00) ? 0 : 1;
    len = _sprintf_len((char *)&buf, "IP0:\"%s\",\"%d,%s\",\"%d\" ", server_cfg.apn, is_tcp, server_cfg.main_ip, server_cfg.main_port);

    is_tcp = ((server_cfg.select_tcp & 0x02) == 0x00) ? 0 : 1;
    len += _sprintf_len((char *)&buf[len], "IP1:\"%s\",\"%d,%s\",\"%d\"", server_cfg.apn, is_tcp, server_cfg.bak_ip, server_cfg.bak_port);
    len += _sprintf_len((char *)&buf[len], "ID:\"%s\"", server_cfg.terminal_id);

    if (server_cfg.dbip == 0x55) {
        _memcpy(buf + len, ",double ip open,", 16);
        len += 16;
    } else {
        _memcpy(buf + len, ",double ip close,", 17);
        len += 17;
    }
    pf = buf + len;

    len1 = _strlen(TR9_SOFT_VERSION_APP);
    _strncpy(pf, TR9_SOFT_VERSION_APP, len1);
    len += len1;

    buf[len] = '\0';

    if (len > 200)
        return;

    _strcpy(rbuf, buf);
    return;
}

/*********************************************************
名    称：sms_query_speed_mode
功    能：提取速度模式
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
/*
static void sms_query_speed_mode(void)
{
	u8 buf[250]={0x00};

	_memset(buf, 0 , 100);
	_sprintf_len((char*)&buf, "SPEED_MODE:GPS ");
	_strcpy(rbuf, buf);
}
*/

/*********************************************************
名    称：sms_terminal_reset
功    能：终端复位
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
static void sms_terminal_reset(void) {
    gps_ext.reset = true;
    lm.sms_rst_en = true;
    lm.sms_rst_tick = tick;

    logd("EXE SMS : mcu reset");

    t_rst.rst = true;
    t_rst.del = false;
    t_rst.send = false;
    t_rst.jiff = jiffies;

    _strcpy(rbuf, "TERMINAL RESET OK ");
    return;
}

//TRSET:ip1<218.066.042.161>port1<8888>ip2<113.012.081.069>port2<6608>sim<40036521243>
static void sms_trset_msg(void) {
    u8 buf[250];
    u8 len = 0;
    IP_SERVER_CONFIG_STRUCT m1;
    u8 t_buf[36]; //本机号码  终端ID号
    u8 *pf;
    u8 i;

    _memset((u8 *)&m1, 0x00, sizeof(m1));

    len = _strlen(cbuf);
    if (len > 140)
        goto sms_set_ip0_err;

    _memcpy(buf, cbuf, len);
    buf[len] = '\0';
    pf = buf + 6;

    if (_strncmp(pf, "ip1", 3) == 0) {
        pf += 4;
        i = _memcpyCH(t_buf, '>', pf, 24);
        {
            _memcpy((u8 *)&m1.main_ip, t_buf, i - 1);
        }
        pf += i;
    } else
        goto sms_set_ip0_err;

    if (_strncmp(pf, "port1", 5) == 0) {
        pf += 6;
        _memset(t_buf, 0, 36);
        i = _memcpyCH(t_buf, '>', pf, 6);
        {
            m1.main_port = atoi((const char *)t_buf);
        }
        pf += i;
    } else
        goto sms_set_ip0_err;

    if (_strncmp(pf, "ip2", 3) == 0) {
        pf += 4;
        i = _memcpyCH(t_buf, '>', pf, 24);
        {
            _memcpy((u8 *)&m1.bak_ip, t_buf, i - 1);
        }
        pf += i;
    } else
        goto sms_set_ip0_err;

    if (_strncmp(pf, "port2", 5) == 0) {
        pf += 6;
        _memset(t_buf, 0, 36);
        i = _memcpyCH(t_buf, '>', pf, 6);
        {
            m1.bak_port = atoi((const char *)t_buf);
        }
        pf += i;
    } else
        goto sms_set_ip0_err;

    if (_strncmp(pf, "sim", 3) == 0) {
        pf += 4;
        i = _memcpyCH(t_buf, '>', pf, 15);
        if (i < 13) {
            m1.terminal_id[0] = 0x30;
            _memcpy((u8 *)&m1.terminal_id[1], pf, i - 1);
        } else
            _memcpy((u8 *)&m1.terminal_id, pf, i - 1);

        pf += i;
    } else
        goto sms_set_ip0_err;

    _memcpy((u8 *)&server_cfg.terminal_id, (u8 *)&m1.terminal_id, 12);
    server_cfg.bak_port = m1.bak_port;
    _memcpy((u8 *)&server_cfg.bak_ip, (u8 *)&m1.bak_ip, sizeof(m1.bak_ip));
    _memcpy((u8 *)&server_cfg.main_ip, (u8 *)&m1.main_ip, sizeof(m1.main_ip));
    server_cfg.main_port = m1.main_port;

    _memset((u8 *)&register_cfg.terminal_id, 0x00, 30);
    _memcpy((u8 *)&register_cfg.terminal_id, (u8 *)&server_cfg.terminal_id[5], 7);
    flash_write_import_parameters();

    init_all_net_info();
    lm.sms_rst_en = true;
    lm.sms_rst_tick = tick;

    run.ip_to_tr9 = true;
    save_run_parameterer();

    _strcpy(rbuf, "Parameter Set OK ");
    return;

sms_set_ip0_err:
    _strcpy(rbuf, "Parameter Set Error");
}

/*
  平台短信设置IC 卡功能
  英文姓名、身份证号、驾驶证有效日期(年月日)、从业资格证号、工号
#dr:,350122198001012222,301231,111222333444555666,123456,#
*/
//loge("//新版本更改//文件信息-写-IC-card://#dr:");//0x6003
void khd_set_card_message(u8 *str, u16 len) {
    //loge("//新版本更改//文件信息-写-IC-card://#dr://<%s>6003", (char *)str);
    //ic4442写卡:据说，没有写卡？？？
    u8 xor = 0;
    u8 card[160] = {0}; //本机号码	终端ID号
    u8 num;
    u8 buf[24] = {0};
    u8 *pf;
    u16 i;
    u8 total = 0;
    u8 *dst;
    DRIVER_CONFIG_STRUCT driver;

    logd("MCU写IC卡：<%s>", (char *)str);

    if (len < 20 || len > 120) {
        return;
    }

    total = len;

    _memset((u8 *)&driver, 0, sizeof(DRIVER_CONFIG_STRUCT));

    _memset(card, 0, 160);
    dst = card;

    num = get_flag_cnt(str, total, ','); //10个
    if (num < 3)
        return;
    pf = str;
    pf += 3;
    total -= 3;
    if (*pf < 0x30) {
        if (*pf != 0x2C)
            return;
    }

    _memset(buf, 0, 24); //姓名
    num = _memcpyCH(buf, ',', pf, 24);
    if (total < num)
        return;
    else
        total -= num;
    if ((num > 0) && (num < 24) && (_strlen(buf) != 0)) {
        _memcpy(dst, buf, 12);
        _memcpy((u8 *)&driver.name, buf, num - 1);
        driver.name_len = num - 1;
#if (0)
        logd("name_len %d", driver.name_len);
#endif
    } else if (num != 1)
        return;
    pf += num;

    dst = card + 12; //身份证号
    _memset(buf, 0, 24);
    num = _memcpyCH(buf, ',', pf, 21);
    if (total < num)
        return;
    else
        total -= num;
    if ((num > 0) && (_strlen(buf) != 0) && (num < 21)) {
        _memcpy(dst, buf, _strlen(buf));
        _memcpy(card + 32, buf, 18);

        _memcpy((u8 *)&driver.license, buf, 18);
    } else if (num != 1)
        return;
    pf += num;

    dst = card + 111; //有效期
    _memset(buf, 0, 24);
    num = _memcpyCH(buf, ',', pf, 8);
    if (total < num)
        return;
    else
        total -= num;
    if ((num == 7) && (_strlen(buf) != 0)) {
        for (i = 0; i < 6; i++) {
            if (i % 2 == 0)
                *dst = (buf[i] - 0x30) << 4;
            else {
                *dst |= (buf[i] - 0x30);
                driver.date[i % 2] = *dst;
                dst++;
            }
        }
    } else if (num != 1)
        return;
    pf += num;

    dst = card + 53; //从业资格证号
    _memset(buf, 0, 24);
    num = _memcpyCH(buf, ',', pf, 25);
    if (total < num)
        return;
    else
        total -= num;
    if ((num > 0) && (_strlen(buf) != 0) && (num < 22)) {
        _memcpy(dst, buf, _strlen(buf));
        _memcpy((u8 *)&driver.qualification, buf, _strlen(buf));
    } else if (num != 1)
        return;
    pf += num;

    dst = card + 115; //工号
    _memset(buf, 0, 24);
    num = _memcpyCH(buf, ',', pf, 8);
    if (total < num)
        return;
    else
        total -= num;
    if ((num > 0) && (_strlen(buf) != 0) && (num < 8)) {
        _memcpy(dst, buf, _strlen(buf));
        _memcpy((u8 *)&driver.driver_code, buf, _strlen(buf));
    } else if (num != 1)
        return;

    for (i = 0; i < 127; i++)
        xor ^= card[i];
    card[127] = xor;

#if (0)
    logdNoNewLine("card[160] = <");
    for (i = 0; i < 160; i++) {
        if (i == 0) {
            _printf2("\r\n xing ming[12]: <%s>", &card[i]);
            _printf2("\r\n xing ming[12]: %02X ", card[i]);
        } else if (i == 12) {
            _printf2("\r\n shen fen zheng[24]: <%s>", &card[i]);
            _printf2("\r\n shen fen zheng[24]: %02X ", card[i]);
        } else if (i == 53) {
            _printf2("\r\n cong ye zi ge zheng[24]: <%s>", &card[i]);
            _printf2("\r\n cong ye zi ge zheng[24]: %02X ", card[i]);
        } else if (i == 111) {
            //_printf2("\r\n you xiao qi[24]: <%s>", &card[i]);
            _printf2("\r\n you xiao qi[24]: %02X ", card[i]);
        } else if (i == 115) {
            _printf2("\r\n bian hao[24]: <%s>", &card[i]);
            _printf2("\r\n bian hao[24]: %02X ", card[i]);
        } else if (i == 127) {
            _printf2("\r\n xor[1]: %02X ", card[i]);
        } else if (i == 128) {
            _printf2("\r\n other: %02X ", card[i]);
        } else {
            _printf2("%02X ", card[i]);
        }
    }
    _printf2(">\r\n");
#endif

    handle_send_base_msg(FROM_INNER, 0xC2, card, 128);
    //无卡情况下；拷贝数据到各个字段
    driver_cfg = driver;
}

/*
GB/T 19056--2012
附录C
表C.1 IC卡信息存储格式定义
---------------------------------------
[d][tr9_agree.c, tr9_6003_task, 711]:
<#dr:123,350122198001012222,301231,111222333444555666,654321,#>, len = 61
-------
[d][sms_parse.c, khd_set_card_message, 488]: card[160] =
<
 card[0]
 xing ming[12]: <:123>
 xing ming[12]: 3A 31 32 33 00 00 00 00 00 00 00 00

 card[12]
 shen fen zheng[24]: <350122198001012222>
 shen fen zheng[24]: 33 35 30 31 32 32 31 39 38 30 30 31 30 31 32 32 32 32 00
00 33 35 30 31 32 32 31 39 38 30 30 31 30 31 32 32 32 32 00 00 00

 card[53]
 cong ye zi ge zheng[24]: <111222333444555666>
 cong ye zi ge zheng[24]: 31 31 31 32 32 32 33 33 33 34 34 34 35 35 35 36 36
36 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

 card[111]
 you xiao qi[24]: 30 12 31 00

 card[115]
 bian hao[24]: <654321>
 bian hao[24]: 36 35 34 33 32 31 00 00 00 00 00 00

 card[127]
 xor[1]: 19

 card[128]
 other: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00
 >
---------------------------------------
*/
/*
#dr:123,450122199709042813,301231,450122199709042813,654321,#
#dr:  张三123  ,   450122199709042813 ,  301215  ,  450122199709042813  ,  123654  ,#
*/
//ic4442写卡:据说，没有写卡？？？
s8 khd_set_ic_card_message(char *str) {
    /*
  规则：
  1、姓名长度1-23
  2、身份证15/18位，数字/字母
  */
    int len = strlen(str);
    u16 i;

    logd("9c rk->mcu: <%s>", (char *)str);

    if (len < 20 || len > 120) {
        loge("(len < 20 || len > 120), is %d", len);
        return -1;
    }

    char *name = malloc(len);
    if (name == NULL) {
        loge("name NULL");
        return -1;
    }
    char *jiaShiZheng = malloc(len);
    if (jiaShiZheng == NULL) {
        loge("jiaShiZheng NULL");
        free(name);
        return -1;
    }
    char *youXiaoQi = malloc(len);
    if (youXiaoQi == NULL) {
        loge("youXiaoQi NULL");
        free(name);
        free(jiaShiZheng);
        return -1;
    }
    char *shenFenZheng = malloc(len);
    if (shenFenZheng == NULL) {
        loge("shenFenZheng NULL");
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        return -1;
    }
    char *biaoHan = malloc(len);
    if (biaoHan == NULL) {
        loge("biaoHan NULL");
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        return -1;
    }

    char bgstr[5] = {0};
    char endstr[2] = {0};

    /*  name：司机姓名，长度【1-9】  */
    char *pattern = "%3s:%[^,],%[^,],%[^,],%[^,],%[^,],%1s";
    //***************bg  name ,jiaSZ,youXQ,sfz  ,biaoH,end
    int ret = sscanf(str, pattern, bgstr, name, jiaShiZheng, youXiaoQi,
                     shenFenZheng, biaoHan, endstr);

    if (ret != 7) {
        loge("ret != 7, %d", ret);
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }

    if (strcmp(bgstr, "#dr") != 0) {
        loge("bgstr err, <%s>", bgstr);
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }

    if (strcmp(endstr, "#") != 0) {
        loge("bgstr err, <%s>", endstr);
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }

#if (1)
    logd("<%s> name", name);
#endif

    trim(name);
    if (isName_ic_card(name) != 0) {
        loge("error name");
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }

#if (1)
    logd("<%s> name", name);
#endif

#if (1)
    /*大车司机的驾驶证号，听说，不是身份证号*/
    trim(jiaShiZheng);
    if (isJiaShiZheng_ic_card(jiaShiZheng) != 0) {
        loge("error JSZ");
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }
#endif

    trim(shenFenZheng);
    if (isShenFenZheng_ic_card((const char *)shenFenZheng) != 0) {
        loge("error SFZ");
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }

#if (1)
    int year_youXiaoQi = -1;
    int month_youXiaoQi = -1;
    int day_youXiaoQi = -1;
    trim(youXiaoQi);
    if (isYouXiaoQi_ic_card((const char *)youXiaoQi, &year_youXiaoQi, &month_youXiaoQi,
                            &day_youXiaoQi)
        != 0) {
        loge("error YXQ");
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }
#endif

#if (1)
    trim(biaoHan);
    if (isBianhao_ic_card((const char *)biaoHan) != 0) {
        loge("error BH");
        free(name);
        free(jiaShiZheng);
        free(youXiaoQi);
        free(shenFenZheng);
        free(biaoHan);
        return -1;
    }
#endif

    loge("ok,...");

    logd("<%s> bgstr**********************", bgstr);
    logd("<%s> name", name);
    logd("<%s> jiaShiZheng", jiaShiZheng);
    logd("<%s> youXiaoQi", youXiaoQi);
    logd("<%s> shenFenZheng", shenFenZheng);
    logd("<%s> biaoHan", biaoHan);
    logd("<%s> endstr**********************", endstr);

#pragma region driver driver_cfg
    DRIVER_CONFIG_STRUCT driver;
    memset((u8 *)&driver, 0, sizeof(DRIVER_CONFIG_STRUCT));

    sprintf((char *)driver.name, "%s", name); //姓名[0]
    driver.name_len = strlen(name);
    sprintf((char *)driver.license, "%s", jiaShiZheng); //机动车驾驶证号码[12]
    //有效期[111]
    driver.date[0] = dec2bcd(year_youXiaoQi);
    driver.date[0] = dec2bcd(month_youXiaoQi);
    driver.date[0] = dec2bcd(day_youXiaoQi);
    sprintf((char *)driver.qualification, "%s", shenFenZheng); //[53]
    sprintf((char *)driver.driver_code, "%s", biaoHan);        //[115]
    driver_cfg = driver;
#pragma endregion driver driver_cfg

#pragma region 写card
    u8 card[160] = {0};

    sprintf((char *)(&card[0]), "%s", name);         //姓名
    sprintf((char *)(&card[12]), "%s", jiaShiZheng); //机动车驾驶证号码
    //有效期
    int pos_you_xiao_qi = 111;
    card[pos_you_xiao_qi + 0] = dec2bcd(year_youXiaoQi);
    card[pos_you_xiao_qi + 1] = dec2bcd(month_youXiaoQi);
    card[pos_you_xiao_qi + 2] = dec2bcd(day_youXiaoQi);
    sprintf((char *)(&card[53]), "%s", shenFenZheng); //[53]
    sprintf((char *)(&card[115]), "%s", biaoHan);     //[115]

    u8 xor = 0;
    for (i = 0; i < 127; i++) {
        xor ^= card[i];
    }

    card[127] = xor;
#if (1)
    if (1) {
        log_level_enum log_level = log_level_debug;

        logb_NoNewLn(log_level, "card[128]");

        for (i = 0; i < 128; i++) {
            if (i == 0) {
                logb_empty(log_level, "\r\n\t xing ming[12]: <%s>", &card[i]);
                logb_empty(log_level, "\r\n\t xing ming[12]: %02X ", card[i]);
            } else if (i == 12) {
                logb_empty(log_level, "\r\n\t shen fen zheng[24]: <%s>", &card[i]);
                logb_empty(log_level, "\r\n\t shen fen zheng[24]: %02X ", card[i]);
            } else if (i == 53) {
                logb_empty(log_level, "\r\n\t cong ye zi ge zheng[24]: <%s>", &card[i]);
                logb_empty(log_level, "\r\n\t cong ye zi ge zheng[24]: %02X ", card[i]);
            } else if (i == 111) {
                //logb_empty("\r\n you xiao qi[24]: <%s>", &card[i]);
                logb_empty(log_level, "\r\n\t you xiao qi[24]: %02X ", card[i]);
            } else if (i == 115) {
                logb_empty(log_level, "\r\n\t bian hao[24]: <%s>", &card[i]);
                logb_empty(log_level, "\r\n\t bian hao[24]: %02X ", card[i]);
            } else if (i == 127) {
                logb_empty(log_level, "\r\n\t xor[1]: %02X ", card[i]);
            } else {
                logb_empty(log_level, "%02X ", card[i]);
            }
        }

        logb_empty(log_level, "\r\n\t >");
        logb_endColor_newLn(log_level, );
    }
#endif

#if (1)
    handle_send_base_msg(FROM_INNER, 0xC2, card, 128);
#endif
#pragma endregion 写card

    free(name);
    free(jiaShiZheng);
    free(youXiaoQi);
    free(shenFenZheng);
    free(biaoHan);

    return 0; //成功
}

//password20050215:"666888"
void set_password_msg(void) {
    u8 buf[32];
    u8 phone_tmp[16];
    char *tok;
    u8 len = 0;
    u8 phone_len = 0;
    u8 *pf;

    len = _strlen(cbuf);
    if (len > 30)
        goto set_server_sms_err;

    _memcpy(buf, cbuf, len);
    buf[len] = '\0';

    pf = cbuf + 8;
    if (!sms_s.cmp_en) {
        if (_strncmp(pf, (password_manage.para + 2), (*((u16 *)(&password_manage.para)))) != 0) {
            _strcpy(rbuf, "password ERR!");
            return;
        } else {
            _strcpy(rbuf, "You do not have permissions");
            return;
        }
    }

    /*鍘绘帀PASSWORD:"  */
    if ((tok = strtok((char *)&buf, ":\"")) == NULL)
        goto set_server_sms_err;

    /*瑙ｆ瀽鏈嶅姟鐢佃瘽鍙风爜, 鍘绘帀"  */
    if ((tok = strtok(NULL, "\"")) != NULL) {
        strcpy((char *)&phone_tmp, tok);
        phone_len = _strlen(phone_tmp);
        if ((phone_len != 8))
            goto set_server_sms_err;

        phone_tmp[phone_len] = '\0';

        _memset((u8 *)&password_manage.para, 0x00, password_para_max_lgth + 2);
        _strcpy((u8 *)&password_manage.para + 2, phone_tmp);
        (*((u16 *)(&password_manage.para))) = phone_len;
        flash25_program_auto_save((flash_parameter_addr + flash_password_addr), ((u8 *)(&password_manage)), (sizeof(password_manage)));

        _strcpy(rbuf, "sms set OK");
        pwr_up_password_status();
        return;
    } else {
    set_server_sms_err:
        _strcpy(rbuf, "Parameter Set Error");
        return;
    }
}

//u16 sms_wakeup_Time ;

bool Revc_sms_flag; //短信标志位

void sms_wakeup_msg(void) {
    //    u8 nowTime = 0;
    u8 i, len = 0;
    u8 *wakeup_buf = cbuf;

    len = (u8)_strlen(wakeup_buf);
    wakeup_buf += 6;
    len -= 6;

    Set_Awaken.sms_wakeup_Time = 0; //重置时间

    for (i = 0; i < len; i++) {
        Set_Awaken.sms_wakeup_Time *= 10;
        Set_Awaken.sms_wakeup_Time += (wakeup_buf[i] - 0x30);
    }
    logd("短信唤醒 %d分钟", Set_Awaken.sms_wakeup_Time);
    Revc_sms_flag = true;
    //Set_Awaken.Revc_sms_flag = true;		//接收到短信
}

cmd_sms const SMS_CMD_Tabel[] =
    {
        {"TongRen Update:", sms_ota_new_ver},
        {"#1002#", sms_query_parameter},
        {"#1008#", sms_terminal_reset},
        {"password", set_password_msg},
        {"TRSET:", sms_trset_msg},  //TRSET:ip1<218.066.042.161>port1<8888>ip2<113.012.081.069>port2<6608>sim<40036521243>
        {"wakeup", sms_wakeup_msg}, //海格短信唤醒
        {"WAKEUP", sms_wakeup_msg}, //海格短信唤醒
        {NULL, NULL}};

/*********************************************************
名    称：parse_sms
功    能：短信解析
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
s16 parse_sms(u8 *cmdbuf, u8 *retbuf, u16 from) {
    u8 i;

    cbuf = cmdbuf;
    rbuf = retbuf;
    _strcpy(rbuf, "Receive");

    //短信存储
    for (i = 0; SMS_CMD_Tabel[i].func != NULL; i++) {
        if (_strncmp(cbuf, SMS_CMD_Tabel[i].name, _strlen(SMS_CMD_Tabel[i].name)) == 0) {
            (*SMS_CMD_Tabel[i].func)();
            return CMD_ACCEPT;
        }
    }
    return CMD_ERROR;
}
