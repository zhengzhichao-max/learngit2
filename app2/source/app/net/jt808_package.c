/**
  ******************************************************************************
  * @file    jt808_package.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   协议封装
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

/*********************************************************
名    称：init_send_info
功    能：初始化
输入参数：无
输    出：无
**********************************************************/
void init_send_info(void) {
    _memset((u8 *)&send_info, 0, sizeof(send_info));
    cur_send_water = 0;
}

/*********************************************************
名	  称：pack_general_answer
功	  能：终端通用应答
输入参数：运行结果 0 成功  1 失败  2  消息有误  3  不支持
输出参数：
返	  回：
注意事项：
*********************************************************/
void pack_general_answer(u8 rst) {
    u8 buf[16] = {0x00};
    u8 b_len;

    b_len = 0;
    _memset(buf, 0, 16);

    b_len += _sw_endian(&buf[b_len], (u8 *)&rev_inf.run_no, 2);
    b_len += _sw_endian(&buf[b_len], (u8 *)&rev_inf.id, 2);
    buf[b_len++] = rst;

    pack_any_data(CMD_UP_COMM_ANSWER, buf, b_len, PK_HEX, (netx));
}

/*********************************************************
名	  称：pack_register_info
功	  能：注册
功 能 号: 0x0100
输入参数：
输出参数：
返	  回：
注意事项：需要修改   
*********************************************************/
void pack_register_info(u8 link) {
    u8 buf[120] = {0};

    u16 len = 0;

    len = 0;
    len += _sw_endian(&buf[len], (u8 *)&register_cfg.province, 2);
    len += _sw_endian(&buf[len], (u8 *)&register_cfg.city, 2);

#if defined(JTT_808_2019)
    //if(JTT_808_2019)
    //{
    len += _memcpy_len(&buf[len], (u8 *)&register_cfg.mfg_id, 11);
    len += _memcpy_len(&buf[len], (u8 *)&register_cfg.terminal_type, 30);
    len += _memcpy_len(&buf[len], (u8 *)&register_cfg.terminal_id, 30);
//}
#else
    //else

    len += _memcpy_len(&buf[len], (u8 *)&register_cfg.mfg_id, rec_manufacturer_id_max_lgth);
    len += _memcpy_len(&buf[len], (u8 *)&register_cfg.terminal_type, rec_terminal_type_max_lgth);
    len += _memcpy_len(&buf[len], (u8 *)&register_cfg.terminal_id, 7);

#endif

    buf[len++] = register_cfg.color;

    len += _memcpy_len(&buf[len], (u8 *)&car_cfg.license, 12);

    pack_any_data(CMD_UP_LOGIN, buf, len, PK_HEX, (link));
}

/*********************************************************
名    称：pack_gps_base_info
功    能：封装GPS基本位置数据
参    数：无
输    出：无
注意事项：每发送一次数据都要重新封装协议头, 返回转换后的长度
*********************************************************/
u16 pack_gps_base_info(u8 *p) //
{
    u16 len1;
    u16 ID = 1;
    u8 buf1[256] = {0};
#if (COMPANY == C_EXLIVE_T808)
    u8 stat = 0;
#endif
    u16 tmp;
#if 0
        static u32 lati = 0x18C3070; 
        static u32 lngi = 0x71C3758;

#endif
    _memset(buf1, 0, 256);

    update_mix_infomation();

    len1 = 0;
    //len1 += _sw_endian(buf1, (u8 *)&mix.alarm.reg, 4);
    len1 += _sw_endian(buf1, (u8 *)&car_alarm.reg, 4);
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.car_state.reg, 4);
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.locate.lati, 4);
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.locate.lngi, 4);
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.heigh, 2);
#if 0 //防止预警时速度为超速值,功能待测试
	if(alarm_cfg.sms_sw.bit.speed_near)
	{
      if(mix.speed > alarm_cfg.speed_over_km)
         mix.speed = alarm_cfg.speed_over_km - 2;
	}
#endif
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.speed, 2);
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.direction, 2);
    len1 += _memcpy_len(&buf1[len1], (u8 *)&mix.time, 6);

    //  里程数据放在附加信息体上
    buf1[len1++] = 0x01;
    buf1[len1++] = 0x04;
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.dist, 4);

    //  油量数据放在附加信息体上
    buf1[len1++] = 0x02;
    buf1[len1++] = 0x02;
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.oil_L, 2); //

    //  传感器速度数据放在附加信息体上
    buf1[len1++] = 0x03;
    buf1[len1++] = 0x02;
    len1 += _sw_endian(&buf1[len1], (u8 *)&mix.complex_speed01, 2);

    //警报时间ID
    buf1[len1++] = 0x04;
    buf1[len1++] = 0x02;
    len1 += _sw_endian(&buf1[len1], (u8 *)&ID, 2);
#if defined(JTT_808_2019)
    buf1[len1++] = 0x05;
    buf1[len1++] = 0x1E;
    len1 += 30;

    buf1[len1++] = 0x06;
    buf1[len1++] = 0x02;
    len1 += 2;
#endif

    if (car_alarm.bit.speed_over) {
#if (P_AREA == AREA_BASE)
        if (area_info.speed > 7) {
            buf1[len1++] = 0x11;
            buf1[len1++] = 0x05;
            buf1[len1++] = area_info.type;
            _sw_endian(&buf1[len1], (u8 *)&area_info.id, 4);
            len1 += 4;
        } else
#endif
        {
            buf1[len1++] = 0x11;
            buf1[len1++] = 0x01;
            buf1[len1++] = 0x00;
        }
    }

#if (P_AREA == AREA_BASE || P_LINE == LINE_BASE)
    if (mix.alarm.bit.road_in_out || mix.alarm.bit.area_in_out) {
        buf1[len1++] = 0x12;
        buf1[len1++] = 0x06;
        buf1[len1++] = area_info.type;
        _sw_endian(&buf1[len1], (u8 *)&area_info.id, 4);
        len1 += 4;
        if ((area_info.alarm & 0x03) != 0x00)
            buf1[len1++] = 0x00; //0: 进 1:出
        else
            buf1[len1++] = 0x01;
    }

    if (car_alarm.bit.drive_over) {
        buf1[len1++] = 0x13;
        buf1[len1++] = 0x07;
        len1 += 3; //路段ID
        buf1[len1++] = 0x01;
        _sw_endian(&buf1[len1], (u8 *)&alarm_cfg.day_time, 2);
        len1 += 2;
        buf1[len1++] = 0x01;
    }
#endif

#if (COMPANY == C_2011)
    goto T808_2011;
#endif
#if (P_RCD == RCD_BASE)
    buf1[len1++] = 0x25;
    buf1[len1++] = 0x04;
    len1 += _sw_endian(&buf1[len1], (u8 *)&bd_rcd_io, 4);
#else
    //8023 和8011 没有这项，兼容平台解析
    buf1[len1++] = 0x25;
    buf1[len1++] = 0x04;
    buf1[len1++] = 0x00;
    buf1[len1++] = 0x00;
    buf1[len1++] = 0x00;
    buf1[len1++] = 0x00;
#endif

    buf1[len1++] = 0x2a;
    buf1[len1++] = 0x02;
    buf1[len1++] = (mix.placeIostate & 0xff00) >> 8;
    buf1[len1++] = (mix.placeIostate & 0x00ff);

    buf1[len1++] = 0x2b;
    buf1[len1++] = 0x04;
    buf1[len1++] = (ADC_Die[1] & 0xff00) >> 8; //模拟量
    buf1[len1++] = (ADC_Die[1] & 0x00ff);
    buf1[len1++] = (ADC_Die[2] & 0xff00) >> 8; //模拟量
    buf1[len1++] = (ADC_Die[2] & 0x00ff);

    ///信号量数值

    buf1[len1++] = 0x30;
    buf1[len1++] = 0x01;
    buf1[len1++] = gs.gsm.squality;

    buf1[len1++] = 0x31;
    buf1[len1++] = 0x01;
    buf1[len1++] = gps_base.fix_num;

#if defined(JTT_808_2019)

    buf1[len1++] = 0xe1; //
    buf1[len1++] = 0x02;
    tmp = 100;

    len1 += _sw_endian(&buf1[len1], (u8 *)&tmp, 2); //

    buf1[len1++] = 0xe5; //
    buf1[len1++] = 0x04;
    buf1[len1++] = 0xff;
    buf1[len1++] = 0xff;
    buf1[len1++] = 0xff;
    buf1[len1++] = 0xff;
#endif

#if (COMPANY == C_2011)
T808_2011:
#endif
    _memcpy(p, buf1, len1);

    return len1;
}

u16 run_no;

/*********************************************************
名    称：pack_mess_head
功    能：封装协议头
参    数：无
输    出：无
注意事项：每发送一次数据都要重新封装协议头，并进行大端转换
*********************************************************/
u8 pack_mess_head(SEND_MESS_INFOMATION_STRUCT *p, u16 uiMId_t, u16 uiMessLen_t, u16 type_t) {
    SEND_MESS_INFOMATION_STRUCT inf;

    bool is_sub = false;
    u8 i;
    //	u8 n;
    u8 r_len;
    u8 tmp = 0;
    is_sub = (type_t & PK_SUB) ? true : false;
    _memset((u8 *)&inf, 0x00, sizeof(inf));

    inf.id = uiMId_t;
#if defined(JTT_808_2019)
    inf.pro.bit.version = 1;
#endif
    inf.pro.bit.encry_type = 0;
    inf.pro.bit.mess_len = uiMessLen_t;
    if (is_sub) {
        inf.pro.bit.if_pack = 1;
        //inf.run_no = send_info.run_no;
        inf.run_no = run_no;
        inf.sub_no = send_info.sub_no;
        inf.sub_tatal = send_info.sub_tatal;
    } else {
        if (++cur_send_water >= 0x8fff) {
            cur_send_water = 0;
        }

        inf.pro.bit.if_pack = 0;
        inf.run_no = cur_send_water;
    }

    _sw_endian((u8 *)&p->id, (u8 *)&inf.id, 2);
    _sw_endian((u8 *)&p->pro, (u8 *)&inf.pro, 2);
    _sw_endian((u8 *)&p->run_no, (u8 *)&inf.run_no, 2);
    _sw_endian((u8 *)&p->sub_no, (u8 *)&inf.sub_no, 2);
    _sw_endian((u8 *)&p->sub_tatal, (u8 *)&inf.sub_tatal, 2);

#if defined(JTT_808_2019)

    p->version = 1;

    for (i = 0; i < 4; i++) {
        _str2tobcd((u8 *)&p->number[i], &tmp);
    }

    if (_strlen(server_cfg.terminal_id) > 11) {
        p->number[4] = (server_cfg.terminal_id[0] - 0x30 << 4) + (server_cfg.terminal_id[1] - 0x30 & 0x0f);
        p->number[5] = (server_cfg.terminal_id[2] - 0x30 << 4) + (server_cfg.terminal_id[3] - 0x30 & 0x0f);
        p->number[6] = (server_cfg.terminal_id[4] - 0x30 << 4) + (server_cfg.terminal_id[5] - 0x30 & 0x0f);
        p->number[7] = (server_cfg.terminal_id[6] - 0x30 << 4) + (server_cfg.terminal_id[7] - 0x30 & 0x0f);
        p->number[8] = (server_cfg.terminal_id[8] - 0x30 << 4) + (server_cfg.terminal_id[9] - 0x30 & 0x0f);
        p->number[9] = (server_cfg.terminal_id[10] - 0x30 << 4) + (server_cfg.terminal_id[11] - 0x30 & 0x0f);
    } else {
        p->number[4] = (server_cfg.terminal_id[0] - 0x30 & 0x0f);
        p->number[5] = (server_cfg.terminal_id[1] - 0x30 << 4) + (server_cfg.terminal_id[2] - 0x30 & 0x0f);
        p->number[6] = (server_cfg.terminal_id[3] - 0x30 << 4) + (server_cfg.terminal_id[4] - 0x30 & 0x0f);
        p->number[7] = (server_cfg.terminal_id[5] - 0x30 << 4) + (server_cfg.terminal_id[6] - 0x30 & 0x0f);
        p->number[8] = (server_cfg.terminal_id[7] - 0x30 << 4) + (server_cfg.terminal_id[8] - 0x30 & 0x0f);
        p->number[9] = (server_cfg.terminal_id[9] - 0x30 << 4) + (server_cfg.terminal_id[10] - 0x30 & 0x0f);
    }
    /*
		for(i = n ;i < 10;i++)
		{
			, &server_cfg.terminal_id[i*2 - 2*4]);
		}
		*/

#else
    if (_strlen(server_cfg.terminal_id) > 11) {
        p->number[0] = (server_cfg.terminal_id[0] - 0x30 << 4) + (server_cfg.terminal_id[1] - 0x30 & 0x0f);
        p->number[1] = (server_cfg.terminal_id[2] - 0x30 << 4) + (server_cfg.terminal_id[3] - 0x30 & 0x0f);
        p->number[2] = (server_cfg.terminal_id[4] - 0x30 << 4) + (server_cfg.terminal_id[5] - 0x30 & 0x0f);
        p->number[3] = (server_cfg.terminal_id[6] - 0x30 << 4) + (server_cfg.terminal_id[7] - 0x30 & 0x0f);
        p->number[4] = (server_cfg.terminal_id[8] - 0x30 << 4) + (server_cfg.terminal_id[9] - 0x30 & 0x0f);
        p->number[5] = (server_cfg.terminal_id[10] - 0x30 << 4) + (server_cfg.terminal_id[11] - 0x30 & 0x0f);
    } else {
        p->number[0] = (server_cfg.terminal_id[0] - 0x30 & 0x0f);
        p->number[1] = (server_cfg.terminal_id[1] - 0x30 << 4) + (server_cfg.terminal_id[2] - 0x30 & 0x0f);
        p->number[2] = (server_cfg.terminal_id[3] - 0x30 << 4) + (server_cfg.terminal_id[4] - 0x30 & 0x0f);
        p->number[3] = (server_cfg.terminal_id[5] - 0x30 << 4) + (server_cfg.terminal_id[6] - 0x30 & 0x0f);
        p->number[4] = (server_cfg.terminal_id[7] - 0x30 << 4) + (server_cfg.terminal_id[8] - 0x30 & 0x0f);
        p->number[5] = (server_cfg.terminal_id[9] - 0x30 << 4) + (server_cfg.terminal_id[10] - 0x30 & 0x0f);
    }

#endif

    if (is_sub)
        r_len = sizeof(SEND_MESS_INFOMATION_STRUCT);
    else
        r_len = sizeof(SEND_MESS_INFOMATION_STRUCT) - 4;

    return r_len;
}

/*********************************************************
名	  称：pack_any_data
功	  能：事件分发程序
输入参数：缓冲区地址，
输出参数：规格化后的缓冲区长度
全局变量:   
返	  回：
注意事项：
1.管理NET, REPORT两种数据分发
2.net 数据，若没有重发，直接发送到模块，若
有重发，进入send_net_proc程序管理
3.report 数据，0X0200位置信息汇报，直接进入send_report_proc
程序管理，send_report_proc具备参数重发机制和盲区管理机制
no :  发送的链路号  0: 主服务器  1: 备服务器   
*********************************************************/
void pack_any_data(u16 uId, u8 *buf, u16 len, u16 type, u8 netn) {
    SEND_MESS_INFOMATION_STRUCT send_head; //已转换消息头
    u8 h_len;
    //	u8 i;
    bool is_report = false;
    bool is_net = false;
    u8 packet[MAX_NET_PACKAGE];
    u16 p_len = 0;
    u8 xor = 0;

    if (buf == NULL || len > MAX_NET_PACKAGE) { return; }

    is_report = (type & PK_RPT) ? true : false;
    is_net = (type & PK_NET) ? true : false;

    h_len = pack_mess_head(&send_head, uId, len, type);

    p_len = 0;
    packet[p_len++] = 0x7e;
    p_len += _memcpy_len(&packet[p_len], (u8 *)&send_head, h_len);
    p_len += _memcpy_len(&packet[p_len], buf, len);
    xor = get_check_xor(packet, 1, p_len);
    packet[p_len++] = xor;
    p_len += _add_filt_char(p_len - 1, &packet[1]);
    packet[p_len++] = 0x7e;

    if (is_report) {
        add_report_queue(uId, cur_send_water, packet, p_len, netn);
        logi("add_report_queue");
    } else if (is_net) {
        add_net_queue(uId, cur_send_water, packet, p_len, netn);
        logi("add_net_queue");
    } else {
        net_send_hex(packet, p_len, netn);
        logi("net_send_hex");
    }
}
