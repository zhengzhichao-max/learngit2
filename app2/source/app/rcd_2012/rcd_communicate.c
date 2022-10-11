/********************************************************************************
  * @file       rcd_communicate.c
  * @author  c.w.s
  * @Email
  * @version  V1.0.0
  * @date     2012-07-03
  * @brief   行驶记录仪信息管理
 1. 本文件包括数据接收(解析) 和数据发送(组包)两个模块
 2. 本文件适合UART 提取，NET 平台提取，兼容USB提取
    ********************************************************************************/
#include "include_all.h"
#include <stdlib.h>
//#include <time.h>

#if (P_RCD == RCD_BASE)
rcd_send_pointor_struct rsp; //,rk_rsp;	 //数据传输控制结构体
usb_read_rcd_struct usb_rsp; //usb数据传输结构体

usb_read_rcd_data_p usb_rcd_pTmp;

rec_c3_t c3_data; //3C标准

u8 Data_Direction;
Allot_data_t Allot;            //分包信息
tr9_6033_task_t tr9_6033_task; //发送结果

rk_6033_task_t rk_6033_task;
u8 next_speed_log_cnt = 0;
u8 next_speed_log_offset = 0;
bool refreshTime = true;
/****************************************************************************
* 名称:    _verify_time_range ()
* 功能：时间校验
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool _verify_time_range(time_t t) {
    s32 ts;
    s32 te;
    s32 t0;

    ts = get_standard_sec_num(rsp.ask.start);
    te = get_standard_sec_num(rsp.ask.end);
    t0 = get_standard_sec_num(t);

    if (ts == -1 || te == -1 || t0 == -1)
        return false;

    if ((t0 >= ts) && (t0 <= te))
        return true;

    return false;
}

/****************************************************************************
* 名称:    big_convert ()
* 功能：RCD2012行驶记录仪，数据大小端转换
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void big_convert(u8 type, u8 *cvt) {
    S_RCD_360HOUR_INFOMATION r360_b;
    S_RCD_360HOUR_INFOMATION r360_s;

    S_RCD_POSINTION_INFOMATION point_b;
    S_RCD_POSINTION_INFOMATION point_s;

    switch (type) {
    case T_RCD_360:
        _memcpy((u8 *)&r360_s, cvt, sizeof(r360_s));
        _memcpy((u8 *)&r360_b, cvt, sizeof(r360_b));
        _sw_endian((u8 *)&r360_b.locate.lati, (u8 *)&r360_s.locate.lati, 4);
        _sw_endian((u8 *)&r360_b.locate.lngi, (u8 *)&r360_s.locate.lngi, 4);
        _sw_endian((u8 *)&r360_b.heigh, (u8 *)&r360_s.heigh, 2);
        _memcpy(cvt, (u8 *)&r360_b, sizeof(r360_b));
        break;
    case T_RCD_ACCIDENT:
        _memcpy((u8 *)&point_s, cvt, sizeof(point_b));
        _memcpy((u8 *)&point_b, cvt, sizeof(point_b));
        _sw_endian((u8 *)&point_b.locate.lati, (u8 *)&point_s.locate.lati, 4);
        _sw_endian((u8 *)&point_b.locate.lngi, (u8 *)&point_s.locate.lngi, 4);
        _sw_endian((u8 *)&point_b.heigh, (u8 *)&point_s.heigh, 2);
        _memcpy(cvt, (u8 *)&point_b, sizeof(point_b));
        break;
    case T_RCD_DRIVER_OT:
        _memcpy((u8 *)&point_s, cvt, sizeof(point_b));
        _memcpy((u8 *)&point_b, cvt, sizeof(point_b));
        _sw_endian((u8 *)&point_b.locate.lati, (u8 *)&point_s.locate.lati, 4);
        _sw_endian((u8 *)&point_b.locate.lngi, (u8 *)&point_s.locate.lngi, 4);
        _sw_endian((u8 *)&point_b.heigh, (u8 *)&point_s.heigh, 2);
        _memcpy(cvt, (u8 *)&point_b, sizeof(point_b));
        break;
    default:
        break;
    }
}

/****************************************************************************
* 名称:    get_rcd_pack_no ()
* 功能：得到指定包号，部标要求补传时，流水号等所有消息体需要一致
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void get_rcd_pack_no(void) {
    u16 offset;

    offset = 0;

    if (rsp.type == T_RCD_360) { //每包 1条数据   每条 666 byte
#if 1
        rsp.cell_num = 1;
        offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD360_NODE_MAX_SIZE);
#else
        rsp.cell_num = rsp.packnum; //2022-06-13 modify by hj  6--> 5
        offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD360_NODE_MAX_SIZE);
        if (1 < rsp.packnum)
            offset /= rsp.packnum;
#endif
        rsp.cur_no = offset + 1;
    } else if (rsp.type == T_RCD_48) {
#if 0
    	//每包 5条数据 
        rsp.cell_num = 5;//2022-05-08 modify by hj  6--> 5
        offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD48_NODE_MAX_SIZE);
        offset /= 5;
#else
        if (rsp.packnum < 7) {
            rsp.cell_num = rsp.packnum; //2022-06-13 modify by hj  6--> 5
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD48_NODE_MAX_SIZE);
            offset /= rsp.packnum; //2022-06-13  modify by hj  6--> 5
        } else {
            rsp.cell_num = 7; //2022-06-13 modify by hj  6--> 5
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD48_NODE_MAX_SIZE);
            offset /= 7; //2022-06-13  modify by hj  6--> 5
        }
#endif
        rsp.cur_no = offset + 1;
    } else if (rsp.type == T_RCD_ACCIDENT) {
#if 0
	rsp.cell_num = 3;
	offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_ACCIDENT_NODE_MAX_SIZE);
	offset /= 3;
#else
        if (rsp.packnum < 3) {
            rsp.cell_num = rsp.packnum; //2022-06-13
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_ACCIDENT_NODE_MAX_SIZE);
            offset /= rsp.packnum; //2022-06-13
        } else {
            rsp.cell_num = 3; //2022-06-13
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_ACCIDENT_NODE_MAX_SIZE);
            offset /= 3; //2022-06-13
        }

#endif
        rsp.cur_no = offset + 1;
    } else if (rsp.type == T_RCD_DRIVER_OT) {
#if 0
	rsp.cell_num = 10;
	offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_OT_NODE_MAX_SIZE);
	offset /= 10;
#else
        if (rsp.packnum < 10) {
            rsp.cell_num = rsp.packnum;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_OT_NODE_MAX_SIZE);
            offset /= rsp.packnum; //2022-07-06
        } else {
            rsp.cell_num = 10;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_OT_NODE_MAX_SIZE);
            offset /= 10; //2022-07-06
        }
#endif
        rsp.cur_no = offset + 1;
    } else if (rsp.type == T_RCD_DRIVER_LOG) {
#if 0
	rsp.cell_num = 20;
	offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_DRIVER_NODE_MAX_SIZE);
	offset /= 20;
#else
        if (rsp.packnum < 20) {
            rsp.cell_num = rsp.packnum;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_DRIVER_NODE_MAX_SIZE);
            offset /= rsp.packnum;
        } else {
            rsp.cell_num = 20;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_DRIVER_NODE_MAX_SIZE);
            offset /= 20;
        }
#endif
        rsp.cur_no = offset + 1;
    } else if (rsp.type == T_RCD_POWER_LOG) {
#if 0
        rsp.cell_num = 100;
        offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_POWER_NODE_MAX_SIZE);
        offset /= 100;
#else //2022-06-13
        if (rsp.packnum < 50) {
            rsp.cell_num = rsp.packnum;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_POWER_NODE_MAX_SIZE);
            offset /= rsp.packnum;
        } else {
            rsp.cell_num = 50;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_POWER_NODE_MAX_SIZE);
            offset /= 50;
        }

#endif
        rsp.cur_no = offset + 1;
    } else if (rsp.type == T_RCD_PARAMETER_LOG) {
#if 0
	rsp.cell_num = 100;
	offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_PARA_NODE_MAX_SIZE);
	offset /= 100;
#else
        if (rsp.packnum < 50) {
            rsp.cell_num = rsp.packnum;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_PARA_NODE_MAX_SIZE);
            offset /= rsp.packnum;
        } else {
            rsp.cell_num = 50;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_PARA_NODE_MAX_SIZE);
            offset /= 50;
        }

#endif
        rsp.cur_no = offset + 1;
    } else if (rsp.type == T_RCD_SPEED_LOG) {
#if 0
        rsp.cell_num = 5;//2022-05-08 modify by hj  6--> 5
        offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_SPEED_NODE_MAX_SIZE);
        offset /= 5;//2022-05-08 modify by hj  6--> 5
#else
        if (rsp.packnum < 5) {
            rsp.cell_num = rsp.packnum; //2022-05-08 modify by hj  6--> 5
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_SPEED_NODE_MAX_SIZE);
            offset /= rsp.packnum; //2022-05-08 modify by hj  6--> 5
        } else {
            rsp.cell_num = 5;
            offset = subp_rcd_p(rsp.start_node, rsp.cur.head, RCD_PARA_NODE_MAX_SIZE);
            offset /= 5;
        }

#endif
        rsp.cur_no = offset + 1;
    }
}

/****************************************************************************
* 名称:    check_start_time ()
* 功能：得到开始传送的时间
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u8 check_start_time(time_t t, bool ht) {
    s32 s1;
    s32 s2;
    s32 s3;

    s1 = get_standard_sec_num(t);             //存储时间
    s2 = get_standard_sec_num(rsp.ask.end);   //结束时间
    s3 = get_standard_sec_num(rsp.ask.start); //开始时间

    // if (s1 == -1 || s2 == -1 || s3 == -1)
    if ((s1 == (s32)-1) || (s2 == (s32)-1) || (s3 == (s32)-1))
        return false;

    if ((s1 <= s2) && (ht == true)) //查找发送头
    {
        if (s1 >= s3)
            return 1;
        else
            return 0;
    } else if ((s1 >= s3) && (ht == false)) //提取时间< 保存时间
    {
        if (s1 <= s2)
            return 1;
        else
            return 0;
    }

    return 0;
}

/****************************************************************************
* 名称:    get_rcd48h_index ()
* 功能：得到两个日天平均车速记录的索引
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u16 get_rcd48h_index(bool ht) {
    u16 low;

    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd48_m.head.node;
    } else {
        low = rcd48_m.tail.node;
    }

    cnt = subp_rcd_p(rcd48_m.head.node, rcd48_m.tail.node, RCD48_NODE_MAX_SIZE) + 1;

    while (i <= cnt) {
        spi_flash_read((u8 *)&time, _rcd48_t_addr(low), 6);
        if (ht)
            rst = check_start_time(time, ht);
        else
            rst = check_start_time(time, ht);
        if (rst == 1) //当前有效区域
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_48, low, 1); //指针减
        } else {
            low = incp_rcd_p(T_RCD_48, low, 1); //指针加
        }

        i++;
    }
    return 0xffff;
}

/****************************************************************************
* 名称:    get_rcd360h_index ()
* 功能：得到360小时的位置信息记录的索引
* 入口参数：无
* 出口参数：无
*
****************************************************************************/
static u16 get_rcd360h_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd360_m.head.node; //头结点
                                  //		high = rcd360_m.tail.node;  //尾节点
    } else {
        //		high= rcd360_m.head.node;
        low = rcd360_m.tail.node;
    }

    cnt = subp_rcd_p(rcd360_m.head.node, rcd360_m.tail.node, RCD360_NODE_MAX_SIZE) + 1;
    //logd("********rcd360_m.head.node = %d  rcd360_m.tail.node = %d", rcd360_m.head.node, rcd360_m.tail.node);
    //logd("********cnt = %d", cnt);
    while (i <= cnt) {
        spi_flash_read((u8 *)&time, _rcd360_t_addr(low), 6);
        //logd("data save time = %02d年%02d月%02d日%02d时%02d分%02d秒", time.year, time.month, time.date, time.hour, time.min, time.sec);
        //logd("***************low = %d", low);
        if (ht)
            rst = check_start_time(time, ht); //检索是否为有效区域
        else
            rst = check_start_time(time, ht);
        if (rst == 1) //当前有效区域
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_360, low, 1); //指针减
        } else {
            low = incp_rcd_p(T_RCD_360, low, 1); //指针加
        }

        i++;
    }
    return 0xffff;
}

/****************************************************************************
* 名称:    get_rcd_accident_index ()
* 功能：得到事故疑点记录的索引
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u16 get_rcd_accident_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd_accident_m.head.node;
        //		high = rcd_accident_m.tail.node;
    } else {
        //		high= rcd_accident_m.head.node;
        low = rcd_accident_m.tail.node;
    }

    cnt = subp_rcd_p(rcd_accident_m.head.node, rcd_accident_m.tail.node, RCD_ACCIDENT_NODE_MAX_SIZE) + 1;

    while (i <= cnt) {
        spi_flash_read((u8 *)&time, ACCI_ADDRD(low), 6);
        if (ht)
            rst = check_start_time(time, ht);
        else
            rst = check_start_time(time, ht);
        if (rst == 1) {
            //当前有效区域
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_ACCIDENT, low, 1);
        } else {
            low = incp_rcd_p(T_RCD_ACCIDENT, low, 1);
        }

        i++;
    }
    return 0xffff;
}

/****************************************************************************
* 名称:    get_rcd_fatigue_index ()
* 功能：得到疲劳驾驶记录的索引
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u16 get_rcd_fatigue_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
    u16 i = 0;
    u8 buf[30] = {0};
    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd_ot_m.head.node;
        //	high = rcd_ot_m.tail.node;
    } else {
        //	high= rcd_ot_m.head.node;
        low = rcd_ot_m.tail.node;
    }

    cnt = subp_rcd_p(rcd_ot_m.head.node, rcd_ot_m.tail.node, RCD_OT_NODE_MAX_SIZE) + 1;

    while (i <= cnt) {
        spi_flash_read((u8 *)&buf, RCDOT_ADDRD(low), 30); //wuxi
        _memcpy((u8 *)&time, &buf[18], 6);
        if (ht)
            rst = check_start_time(time, ht);
        else
            rst = check_start_time(time, ht);
        if (rst == 1) //当前有效区域
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_DRIVER_OT, low, 1);
        } else {
            low = incp_rcd_p(T_RCD_DRIVER_OT, low, 1);
        }

        i++;
    }
    return 0xffff;
}

/****************************************************************************
* 名称:    get_rcd_driver_index ()
* 功能：得到驾驶员记录的索引
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u16 get_rcd_driver_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
//    u8 buff[30] = {0};
    u16 i = 0;

    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd_driver_m.head.node;
    } else {
        low = rcd_driver_m.tail.node;
    }

    cnt = subp_rcd_p(rcd_driver_m.head.node, rcd_driver_m.tail.node, RCD_DRIVER_NODE_MAX_SIZE) + 1;

    while (i <= cnt) {
        //memset(buff,0x00,30);
        //spi_flash_read(buff, DRIVER_ADDRD(low), 25);
        spi_flash_read((u8 *)&time, DRIVER_ADDRD(low), 6);
        if (ht)
            rst = check_start_time(time, ht);
        else
            rst = check_start_time(time, ht);
        if (rst == 1) //当前有效区域
        {
            return low;
        }

        if (ht) {
            low = decp_rcd_p(T_RCD_DRIVER_LOG, low, 1);
        } else {
            low = incp_rcd_p(T_RCD_DRIVER_LOG, low, 1);
        }
        i++;
    }
    return 0xffff;
}

/****************************************************************************
* 名称:    get_rcd_power_index ()
* 功能：得到电源记录的索引
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u16 get_rcd_power_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd_power_m.head.node;
        //		high = rcd_power_m.tail.node;
    } else {
        //		high= rcd_power_m.head.node;
        low = rcd_power_m.tail.node;
    }

    cnt = subp_rcd_p(rcd_power_m.head.node, rcd_power_m.tail.node, RCD_POWER_NODE_MAX_SIZE) + 1;

    while (i <= cnt) {
        spi_flash_read((u8 *)&time, POWER_ADDRD(low), 6);
        if (ht)
            rst = check_start_time(time, ht);
        else
            rst = check_start_time(time, ht);
        if (rst == 1) //当前有效区域
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_POWER_LOG, low, 1);
        } else {
            low = incp_rcd_p(T_RCD_POWER_LOG, low, 1);
        }
        i++;
    }
    return 0xffff;
}

/****************************************************************************
* 名称:    get_rcd_param_index ()
* 功能：得到参数修改记录的索引
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u16 get_rcd_param_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd_para_m.head.node;
    } else {
        low = rcd_para_m.tail.node;
    }

    cnt = subp_rcd_p(rcd_para_m.head.node, rcd_para_m.tail.node, RCD_PARA_NODE_MAX_SIZE) + 1;

    while (i <= cnt) {
        spi_flash_read((u8 *)&time, _rcd_para_t_addr(low), 6);
        if (ht) {
            rst = check_start_time(time, ht);
        } else
            rst = check_start_time(time, ht);
        if (rst == 1) //当前有效区域
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_PARAMETER_LOG, low, 1);
        } else {
            low = incp_rcd_p(T_RCD_PARAMETER_LOG, low, 1);
        }

        i++;
    }
    return 0xffff;
}

static u16 get_rcd_15H_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 小于 =1 等于 =2 大于
    time_t time;
    u16 i = 0;
    u8 buf[30];

    cnt = 0;
    if (ht) //开始时间点；
    {
        low = rcd_speed_m.head.node;
    } else {
        low = rcd_speed_m.tail.node;
    }

    cnt = subp_rcd_p(rcd_speed_m.head.node, rcd_speed_m.tail.node, RCD_SPEED_NODE_MAX_SIZE) + 1;

    while (i <= cnt) {
        spi_flash_read((u8 *)&buf, RSPEED_ADDRD(low), 7);
        _memcpy((u8 *)&time, &buf[1], 6);

        if (ht) {
            rst = check_start_time(time, ht);
        } else
            rst = check_start_time(time, ht);
        if (rst == 1) //当前有效区域
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_SPEED_LOG, low, 1);
        } else {
            low = incp_rcd_p(T_RCD_SPEED_LOG, low, 1);
        }

        i++;
    }
    return 0xffff;
}

/***************************************************************************
* 名称:    rcd_get_send_node ()
* 功能：得到记录仪发送节点的索引
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd_get_send_node(void) {
    rsp.cur_no = 1;

    switch (rsp.type) {
    case T_RCD_48:
        if (pos48.ifnew == false) {
            rsp.cur.head = get_rcd48h_index(true); //找到有效信息头
            rsp.cur.tail = get_rcd48h_index(false);

            logdr("..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....", rsp.cur.head, rsp.cur.tail);

            if (rsp.cur.head > RCD48_NODE_MAX_SIZE || rsp.cur.tail > RCD48_NODE_MAX_SIZE) {
                logdr("rcd_get_send_node err :48");
                return false;
            }
            rsp.cur.tail = decp_rcd_p(T_RCD_48, rsp.cur.tail, 1);
            rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD48_NODE_MAX_SIZE);
            rsp.total = rsp.all_num * RCD48_MINUTE_INF_SIZE;
        } else {
            logdr("..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....", rsp.cur.head, rsp.cur.tail);
            rsp.cur.head = pos48.head;
            rsp.cur.tail = pos48.tail;
        }
        break;
    case T_RCD_360: //360小时位置信息
        if (pos360.ifnew == false) {
            rsp.cur.head = get_rcd360h_index(true);  //得到头结点
            rsp.cur.tail = get_rcd360h_index(false); //得到尾结点
            if (rsp.cur.head > RCD360_NODE_MAX_SIZE || rsp.cur.tail > RCD360_NODE_MAX_SIZE) {
                logdr("rcd_get_send_node err :360");
                return false;
            }
            rsp.cur.tail = decp_rcd_p(T_RCD_360, rsp.cur.tail, 1);                      //获取尾指针索引
            rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD360_NODE_MAX_SIZE); //+1 得到位置信息总包数
            rsp.total = rsp.all_num * RCD360_HOUR_INF_SIZE;                             //360小时位置信息的总字节数
        } else {
            rsp.cur.head = pos360.head;
            rsp.cur.tail = pos360.tail;
        }
        break;
    case T_RCD_ACCIDENT: //事故疑点记录
        rsp.cur.head = get_rcd_accident_index(true);
        rsp.cur.tail = get_rcd_accident_index(false);
        if (rsp.cur.head > RCD_ACCIDENT_NODE_MAX_SIZE || rsp.cur.tail > RCD_ACCIDENT_NODE_MAX_SIZE) {
            logdr("rcd_get_send_node err :accident");
            return false;
        }
        rsp.cur.tail = decp_rcd_p(T_RCD_ACCIDENT, rsp.cur.tail, 1);
        rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD_ACCIDENT_NODE_MAX_SIZE);
        rsp.total = rsp.all_num * RCD_ACCIDENT_INF_SIZE;
        break;
    case T_RCD_DRIVER_OT: //超时驾驶记录
        rsp.cur.head = get_rcd_fatigue_index(true);
        rsp.cur.tail = get_rcd_fatigue_index(false);
        if (rsp.cur.head > RCD_OT_NODE_MAX_SIZE || rsp.cur.tail > RCD_OT_NODE_MAX_SIZE) {
            logdr("rcd_get_send_node err :ot");
            return false;
        }
        rsp.cur.tail = decp_rcd_p(T_RCD_DRIVER_OT, rsp.cur.tail, 1);
        rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD_OT_NODE_MAX_SIZE);
        rsp.total = rsp.all_num * RCD_OT_INF_SIZE;
        break;
    case T_RCD_DRIVER_LOG: //驾驶人信息记录
        rsp.cur.head = get_rcd_driver_index(true);
        rsp.cur.tail = get_rcd_driver_index(false);
        if (rsp.cur.head > RCD_DRIVER_NODE_MAX_SIZE || rsp.cur.tail > RCD_DRIVER_NODE_MAX_SIZE) {
            logdr("rcd_get_send_node err :dirver log");
            return false;
        }
        rsp.cur.tail = decp_rcd_p(T_RCD_DRIVER_LOG, rsp.cur.tail, 1);
        rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD_DRIVER_NODE_MAX_SIZE);
        rsp.total = rsp.all_num * RCD_DRIVER_INF_SIZE;
        break;
    case T_RCD_POWER_LOG:
        rsp.cur.head = get_rcd_power_index(true);
        rsp.cur.tail = get_rcd_power_index(false);
        if (rsp.cur.head > RCD_POWER_NODE_MAX_SIZE || rsp.cur.tail > RCD_POWER_NODE_MAX_SIZE) {
            logdr("rcd_get_send_node err :power");
            return false;
        }
        rsp.cur.tail = decp_rcd_p(T_RCD_POWER_LOG, rsp.cur.tail, 1);
        rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD_POWER_NODE_MAX_SIZE);
        rsp.total = rsp.all_num * RCD_POWER_INF_SIZE;
        break;

    case T_RCD_PARAMETER_LOG:
        rsp.cur.head = get_rcd_param_index(true);
        rsp.cur.tail = get_rcd_param_index(false);
        if (rsp.cur.head > RCD_PARA_NODE_MAX_SIZE || rsp.cur.tail > RCD_PARA_NODE_MAX_SIZE) {
            logdr("rcd_get_send_node err :parameter");
            return false;
        }
        rsp.cur.tail = decp_rcd_p(T_RCD_PARAMETER_LOG, rsp.cur.tail, 1);
        rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD_PARA_NODE_MAX_SIZE);
        rsp.total = rsp.all_num * RCD_PARA_INF_SIZE;
        break;

    case T_RCD_SPEED_LOG:
        rsp.cur.head = get_rcd_15H_index(true);
        rsp.cur.tail = get_rcd_15H_index(false);
        if (rsp.cur.head > RCD_SPEED_NODE_MAX_SIZE || rsp.cur.tail > RCD_SPEED_NODE_MAX_SIZE) {
            logdr("rcd_get_send_node err :15H speed");
            return false;
        }
        rsp.cur.tail = decp_rcd_p(T_RCD_SPEED_LOG, rsp.cur.tail, 1);
        rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD_SPEED_NODE_MAX_SIZE);
        rsp.total = rsp.all_num * RCD_SPEED_INF_SIZE;
        break;
    }

    rsp.start_node = rsp.cur.head; //开始发送的节点
    get_rcd_pack_no();             //得到单帧合包个数
    rsp.all_num = ((rsp.all_num % rsp.cell_num) == 0) ? (rsp.all_num / rsp.cell_num) : ((rsp.all_num / rsp.cell_num) + 1);
    if (rsp.all_num > 5000) {
        logdr("rcd_get_send_node err :2");
        return false;
    }

    rsp.start_water_no = cur_send_water + 1;
    cur_send_water += (rsp.all_num + 2);

    return true;
}

/***************************************************************************
* 名称:    rcd_assemble_body ()
* 功能：记录仪信息体组包
* 入口参数：无
* 出口参数：无
****************************************************************************/
s16 rcd_assemble_body(u16 index_t, u8 *p) {
    bool ret = false;
    u16 m_len;
    s16 p_len;
    time_t time;
    u8 buf_t[RCD_COM_MAX_SIZE];
    u8 msg_t[RCD_COM_MAX_SIZE];
    u16 i = 0;

//    u32 unusual;
//    u16 unusualBak;

    S_RCD_360HOUR_INFOMATION rcd360_inf;
    S_RCD_POSINTION_INFOMATION cvt_point;

    p_len = 0;
    m_len = 0;
    if (rsp.type == T_RCD_NULL || rsp.type >= T_RCD_END) {
        return 0;
    }

    switch (rsp.type) {
    case T_RCD_48: {
        spi_flash_read(msg_t, _rcd48_t_addr(index_t), 126);
        { //如果有时间范围,在部标过检时, 不会下发时间范围, 故增加这个标志进行 区分。
            /*校验开始时间 与 结束时间范围: 2022-03-24 add by hj*/
            _memcpy((u8 *)&time, msg_t, 6);
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }

            if (Rk_Task_Manage.RK_time_rang_NG_flag == 0) {
                ret = _verify_time_range(time);
                if (ret == false) {
                    return 0;
                }
            }
        }

        _nbintobcd(msg_t, 6); //时间
        msg_t[5] = 0;         //wuxi 0827
        p_len = _memcpy_len(p, msg_t, 126);
    } break;
    case T_RCD_360: {
        spi_flash_read(buf_t, _rcd360_t_addr(index_t), sizeof(time_t));
        {
            _memcpy((u8 *)&time, buf_t, 6);
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }

            if (Rk_Task_Manage.RK_time_rang_NG_flag == 0) {
                ret = _verify_time_range(time);
                if (ret == false)
                    return 0;
            }
        }
        _nbintobcd(buf_t, 6);
        m_len = _memcpy_len(msg_t, buf_t, 6);

        for (i = 0; i < 60; i++) {
            spi_flash_read((u8 *)&rcd360_inf, _rcd360_d_addr(index_t, i), sizeof(rcd360_inf));
            big_convert(T_RCD_360, (u8 *)&rcd360_inf);
            /*	   
			if( 0xffffffff == rcd360_inf.locate.lati)	
				rcd360_inf.locate.lati = 0xffffff7f;
			
			if(0xffffffff == rcd360_inf.locate.lngi )
				rcd360_inf.locate.lngi = 0xffffff7f;

			if(0xffff ==rcd360_inf.heigh)
				rcd360_inf.heigh = 0xff7f;
				
			m_len += _memcpy_len(&msg_t[m_len],(u8 *)&rcd360_inf.locate.lngi,4);
			m_len += _memcpy_len(&msg_t[m_len],(u8 *)&rcd360_inf.locate.lati,4);
			m_len += _memcpy_len(&msg_t[m_len],(u8 *)&rcd360_inf.heigh,2);
			msg_t[m_len++] = rcd360_inf.speed ;		
			*/

            m_len += _memcpy_len(&msg_t[m_len], (u8 *)&rcd360_inf, 11);
        }

        p_len = _memcpy_len(p, msg_t, 666);
    } break;
    case T_RCD_ACCIDENT: {
        spi_flash_read(msg_t, ACCI_ADDRD(index_t), 234);
        _memcpy((u8 *)&time, msg_t, 6);
        //if(Rk_Task_Manage.RK_time_rang_NG_flag == 0)
        {
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }
        }

        _nbintobcd(msg_t, 6);
        _memcpy((u8 *)&cvt_point, &msg_t[224], 10);
        big_convert(T_RCD_ACCIDENT, (u8 *)&cvt_point);
        _memcpy(&msg_t[224], (u8 *)&cvt_point, 10);
        p_len = _memcpy_len(p, msg_t, 234);
    } break;
    case T_RCD_DRIVER_OT: {
        spi_flash_read(msg_t, RCDOT_ADDRD(index_t), 50);
        //if(Rk_Task_Manage.RK_time_rang_NG_flag == 0)
        {
            _memcpy((u8 *)&time, &msg_t[18], 6);

            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }

            _memcpy((u8 *)&time, &msg_t[24], 6); //问题点时间存储不对
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }
        }

        _nbintobcd(&msg_t[18], 6);
        _nbintobcd(&msg_t[24], 6);

        _memcpy((u8 *)&cvt_point, &msg_t[30], 10);
        big_convert(T_RCD_DRIVER_OT, (u8 *)&cvt_point);
        _memcpy(&msg_t[30], (u8 *)&cvt_point, 10);
        _memcpy((u8 *)&cvt_point, &msg_t[40], 10);
        big_convert(T_RCD_DRIVER_OT, (u8 *)&cvt_point);
        _memcpy(&msg_t[40], (u8 *)&cvt_point, 10);

        p_len = _memcpy_len(p, msg_t, 50);
    } break;
    case T_RCD_DRIVER_LOG: {
        spi_flash_read(msg_t, DRIVER_ADDRD(index_t), 25);
        //if(Rk_Task_Manage.RK_time_rang_NG_flag == 0)
        { //校验时间是否正确。
            _memcpy((u8 *)&time, msg_t, 6);
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }
        }

        _nbintobcd(msg_t, 6);
        p_len = _memcpy_len(p, msg_t, 25);
    } break;
    case T_RCD_POWER_LOG: {
        spi_flash_read(msg_t, POWER_ADDRD(index_t), 7);
        //if(Rk_Task_Manage.RK_time_rang_NG_flag == 0)
        {
            _memcpy((u8 *)&time, msg_t, 6);
            ret = _verify_time(&time);
            if (ret == false) {
                //logd("1....... i am here %02x-%02x-%02x %02x:%02x:%02x.....",msg_t[0],msg_t[1],msg_t[2],msg_t[3],msg_t[4],msg_t[5]);
                return 0;
            }
        }
        //logd("2....... i am here index_t =%d, %02x-%02x-%02x %02x:%02x:%02x.....",index_t,msg_t[0],msg_t[1],msg_t[2],msg_t[3],msg_t[4],msg_t[5]);
        _nbintobcd(msg_t, 6);
        p_len = _memcpy_len(p, msg_t, 7);
    } break;
    case T_RCD_PARAMETER_LOG: {
        spi_flash_read(msg_t, _rcd_para_t_addr(index_t), 7);
        //if(Rk_Task_Manage.RK_time_rang_NG_flag == 0)
        {
            _memcpy((u8 *)&time, msg_t, 6);
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }
        }
        _nbintobcd(msg_t, 6);
        p_len = _memcpy_len(p, msg_t, 7);
    } break;
    case T_RCD_SPEED_LOG: {
        spi_flash_read(msg_t, RSPEED_ADDRD(index_t), 133);

/*2022-04-21 add by hj -start*/
#if 0
               if(next_speed_log_offset == 0)
               	{
                   logd("记录15H  index_t=[%d]  rk_6033_task.tail=[%d]",index_t,rk_6033_task.tail);                 
                   spi_flash_read(msg_t, RSPEED_ADDRD(index_t), 133);
                   for(i=2;i>0;i--)
                   {//每天重复日期最多5条
				             if((index_t-i) == rk_6033_task.tail)
				             	{
                        logd("等于尾, 退出");                      
                        break;
                      }

                   	 spi_flash_read(buf_t, RSPEED_ADDRD(index_t-i), 133);

                   logd("buf_t[1]=%02x  buf_t[2]=%02x  buf_t[3]=0x%02X",buf_t[1],buf_t[2],buf_t[3]);                 
                   logd("msg_t[1]=%02x  msg_t[2]=%02x  msg_t[3]=0x%02X",msg_t[1],msg_t[2],msg_t[3]);                 
                   	 if((buf_t[1]== msg_t[1]) && (buf_t[2]== msg_t[2]) && (buf_t[3]== msg_t[3]))
                   	 	{   
                         next_speed_log_cnt ++;
                         next_speed_log_offset ++;                        	 		
                   	 	}
                   	 else
                   	 	{
                   	 		break;
                   	 	}	                  	 	
                   }                            	                
                }
                
               //logd("记录15H  next_speed_log_offset=[%d]  next_speed_log_cnt=[%d]",next_speed_log_offset,next_speed_log_cnt);                 
               spi_flash_read(msg_t, RSPEED_ADDRD(index_t - next_speed_log_offset ), 133);
#endif

        //if(Rk_Task_Manage.RK_time_rang_NG_flag == 0)
        {
            _memcpy((u8 *)&time, &msg_t[1], 6);
#if 1
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }
#endif
            _memcpy((u8 *)&time, &msg_t[7], 6);
#if 1
            ret = _verify_time(&time);
            if (ret == false) {
                return 0;
            }
#endif
        }
        _nbintobcd(&msg_t[1], 6);
        _nbintobcd(&msg_t[7], 6);

        p_len = _memcpy_len(p, msg_t, 133);
    } break;
    }

/*2022-03-17 add by hj */
#if 0
    if(Rk_Task_Manage.RK_time_rang_NG_flag == 1 )
    	{
         logdNoNewLine("cmd=%0x02  当包数据 index node =-%d-  read_Len=-%d-",index_t,p_len);
         		{
         		   u16 k;
         		   for(k=0;k<p_len;k++)
         		    {
         		    	 _printf("%02x ",*(msg_t+k));
         		    }
         		   _printf("\r\n");
         		}
    	}
#endif

    return p_len;
}

/****************************************************************************
* 名称:    load_new_rcd_data ()
* 功能：加载数据提取时的信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool load_new_rcd_data(u8 from_t, u8 id_t) {
    _memset((u8 *)&rsp, 0x00, sizeof(rsp));

    rsp.id = id_t;
    rsp.from = from_t;
    rsp.ack_no = rev_inf.run_no;

    return true;
}

/****************************************************************************
* 名称:   rcd_send_data () ////RCD_C_OK
* 功能：记录仪数据发送
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool rcd_send_data(u8 from, u8 result, u8 id, u8 *str, u16 len) {
    u8 flag;

    static u8 xor = 0;
    u16 i;

    u16 idx = 0;
    u16 packet = 0;
    u16 ack_len = 0;
    u16 msg_len = 0;

    u8 ack_buf[RCD_COM_MAX_SIZE + 50] = {0};
    u8 msg_buf[RCD_COM_MAX_SIZE + 50] = {0};
    //u8 ack_buf[RCD_COM_MAX_SIZE + 200] = {0};
    // u8 msg_buf[RCD_COM_MAX_SIZE + 200] = {0};

    if (str == NULL || len > RCD_COM_MAX_SIZE)
        return false;

    ack_len = 0;
    if (from == FROM_NET)
        get_rcd_pack_no();
    //if(  from != FROM_LCD )
    {
        if (result == RCD_C_SET_ERR) //参数命令帧接收出错
        {
            ack_buf[ack_len++] = 0x55;
            ack_buf[ack_len++] = 0x7a;
            ack_buf[ack_len++] = 0xfb;
            ack_buf[ack_len++] = 0x00;
            xor = 0;
        } else if (result == RCD_C_GET_ERR) //采集数据命令帧接收出错
        {
            ack_buf[ack_len++] = 0x55;
            ack_buf[ack_len++] = 0x7a;
            ack_buf[ack_len++] = 0xfa;
            ack_buf[ack_len++] = 0x00;
            xor = 0;
        } else if (result == RCD_C_OK) {
            if (from == FROM_U4) {
                ack_buf[ack_len++] = id;
            }
#if 1
            ack_buf[ack_len++] = 0x55;
            ack_buf[ack_len++] = 0x7a;
            ack_buf[ack_len++] = id;
            ack_buf[ack_len++] = len >> 8;
            ack_buf[ack_len++] = len;
            ack_buf[ack_len++] = 0x00;
            ack_len += _memcpy_len(&ack_buf[ack_len], str, len);
            xor = 0;
#endif
        }
#if 1
        for (i = 0; i < ack_len; i++) //异或校验
        {
            xor ^= ack_buf[i];
        }
        ack_buf[ack_len++] = xor;
#endif
    }

    if (from == FROM_U1 || from == FROM_U3) {
        send_any_uart(from, ack_buf, ack_len); //向串口发送数据
        if ((FROM_U1 == from) && (u1_m.cur.b.rcd_3c)) {
            set_uart_ack(from);
        }

    } else if (from == FROM_NET) //向网络发送数据
    {
        //第一包包含应答流水号和命令ID
        msg_len = 0;
#if 1
        msg_buf[msg_len++] = rsp.ack_no >> 8;
        msg_buf[msg_len++] = rsp.ack_no;
        msg_buf[msg_len++] = id;
#endif
        msg_len += _memcpy_len(&msg_buf[msg_len], ack_buf, ack_len);

        send_info.run_no = rsp.start_water_no + rsp.cur_no;
        send_info.sub_no = rsp.cur_no;
        send_info.sub_tatal = rsp.all_num;

        flag = (send_info.sub_tatal > 1) ? (PK_SUB | PK_HEX) : PK_HEX;
        pack_any_data(CMD_UP_RDR_REP, msg_buf, msg_len, flag, (netx));

        logd("rcd send:ID:%02d,cur_head:%04d,all_num:%04d,cur_no:%04d", id, rsp.cur.head, rsp.all_num, rsp.cur_no);
    } else if (from == FROM_U4) {
        msg_len = 0;

#if 0
		idx	   = Allot.idx + 1    ;						//序号从一开始
		packet = Allot.packet     ;						//序号从一开始
#endif

#if 1                                 //2022-03-17
        idx = rk_6033_task.idx + 1;   //序号从一开始
        packet = rk_6033_task.packet; //序号从一开始
#endif

        //msg_buf[msg_len++] = Allot.Data_Direction 		;
        //msg_buf[msg_len++] = Data_Direction 			    ;
        /*2022-03-17 modify by hj*/
        msg_buf[msg_len++] = rk_6033_task.Data_Direction;
        msg_buf[msg_len++] = (u8)((packet & 0xff00) >> 8);
        msg_buf[msg_len++] = (u8)(packet & 0x00ff);
        msg_buf[msg_len++] = (u8)((idx & 0xff00) >> 8);
        msg_buf[msg_len++] = (u8)(idx & 0x00ff);

        msg_len += _memcpy_len(&msg_buf[msg_len], ack_buf, ack_len);

#if 0
    logd("cmd=%02x  总包数=-%d-  当前包=-%d-  头节点=-%d-  尾节点=-%d- 发送长度 Len=%d",rk_6033_task.cmd,\
             rk_6033_task.packet,\
             rk_6033_task.idx+1,\
             rk_6033_task.head,\
             rk_6033_task.tail,\
             msg_len);

    		{//01 00 44 00 1F 00 55 7A 00 02 9A 00 
    		   u16 k;
    		   for(k=0;k<msg_len-12;k++)
    		    {
    		    	 _printf("%02X ",*(msg_buf+12+k));
    		    }
    		   _printf("\r\n\r\n");
    		}

#endif

        //logd("19056数据导出:[	 时间:%2x:%2x 指令:%02x 总包:%d 序号:%d 数据长度:%d]",mix.time.hour,mix.time.min,id,packet,idx,msg_len);
        tr9_frame_pack2rk(tr9_cmd_6033, msg_buf, msg_len); //行驶记录采集都是用6033的消息ID
    }
    return true;
}

/****************************************************************************
* 名称:    analyse_rcd0x00_ver ()
* 功能：解析记录仪版本
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x00_ver(u8 from, u8 *str, u16 len) {
    u8 s_buf[64] = {0};
    u16 s_len = 0;

    if (str == NULL || len > D_BUF_LEN)
        return;

    s_buf[s_len++] = 0x12;
    s_buf[s_len++] = 0x32;
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rsp.total = s_len;

    rcd_send_data(from, RCD_C_OK, 0x00, s_buf, s_len);

    _memset((u8 *)&pos48, 0, sizeof(pos48));
    _memset((u8 *)&pos360, 0, sizeof(pos360));
}

/****************************************************************************
* 名称:    analyse_rcd0x01_driver ()
* 功能：解析驾驶员信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x01_driver(u8 from, u8 *str, u16 len) {
    u8 s_buf[64] = {0};
    u16 s_len;

    if (str == NULL || len > D_BUF_LEN)
        return;

    s_len = 0;
    s_len += _memcpy_len(&s_buf[s_len], (u8 *)&driver_cfg.license, 18); //从业资格证号 驾驶证号
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rsp.total = s_len;
    rcd_send_data(from, RCD_C_OK, 0x01, s_buf, s_len);
}

/****************************************************************************
* 名称:    analyse_rcd0x02_real_time ()
* 功能：解析实时时间
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x02_real_time(u8 from, u8 *str, u16 len) {
    u8 s_buf[64] = {0};
    u16 s_len;

    if (str == NULL || len > D_BUF_LEN)
        return;

    s_len = 0;
    s_buf[s_len++] = _bintobcd(sys_time.year);
    s_buf[s_len++] = _bintobcd(sys_time.month);
    s_buf[s_len++] = _bintobcd(sys_time.date);
    s_buf[s_len++] = _bintobcd(sys_time.hour);
    s_buf[s_len++] = _bintobcd(sys_time.min);
    s_buf[s_len++] = _bintobcd(sys_time.sec);
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rsp.total = s_len;
    rcd_send_data(from, RCD_C_OK, 0x02, s_buf, s_len);
    _memset((u8 *)&pos48, 0, sizeof(pos48));
    _memset((u8 *)&pos360, 0, sizeof(pos360));
}

/****************************************************************************
* 名称:    analyse_rcd0x03_mileage ()
* 功能：解析里程相关参数
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x03_mileage(u8 from, u8 *str, u16 len) {
    u8 s_buf[128] = {0};
    u16 s_len;
//    u8 i = 0;
    u32 total_tmp;
    u32 set_mileage;
    if (str == NULL || len > D_BUF_LEN)
        return;

    read_mfg_parameter();
    total_tmp = run.total_dist / 100;
    set_mileage = factory_para.set_mileage / 100;

    s_len = 0;
    s_buf[s_len++] = _bintobcd(sys_time.year);
    s_buf[s_len++] = _bintobcd(sys_time.month);
    s_buf[s_len++] = _bintobcd(sys_time.date);
    s_buf[s_len++] = _bintobcd(sys_time.hour);
    s_buf[s_len++] = _bintobcd(sys_time.min);
    s_buf[s_len++] = _bintobcd(sys_time.sec);
    /*
    s_buf[s_len++] = _bintobcd(factory_para.setting_time.year);
    s_buf[s_len++] = _bintobcd(factory_para.setting_time.month);
    s_buf[s_len++] = _bintobcd(factory_para.setting_time.date);
    s_buf[s_len++] = _bintobcd(factory_para.setting_time.hour);
    s_buf[s_len++] = _bintobcd(factory_para.setting_time.min);
    s_buf[s_len++] = _bintobcd(factory_para.setting_time.sec);

    s_buf[s_len++] = _bintobcd((u8)((factory_para.set_mileage % 100000000) / 1000000));
    s_buf[s_len++] = _bintobcd((u8)((factory_para.set_mileage % 1000000) / 10000));
    s_buf[s_len++] = _bintobcd((u8)((factory_para.set_mileage % 10000) / 100));
    s_buf[s_len++] = _bintobcd((u8)(  factory_para.set_mileage % 100));
	
    s_buf[s_len++] = _bintobcd((u8)((total_tmp % 100000000) / 1000000));
    s_buf[s_len++] = _bintobcd((u8)((total_tmp % 1000000) / 10000));
    s_buf[s_len++] = _bintobcd((u8)((total_tmp % 10000) / 100));
    s_buf[s_len++] = _bintobcd((u8)(total_tmp % 100));
*/
    s_buf[s_len++] = (factory_para.setting_time.year);
    s_buf[s_len++] = (factory_para.setting_time.month);
    s_buf[s_len++] = (factory_para.setting_time.date);
    s_buf[s_len++] = (factory_para.setting_time.hour);
    s_buf[s_len++] = (factory_para.setting_time.min);
    s_buf[s_len++] = (factory_para.setting_time.sec);

    s_buf[s_len++] = _bintobcd((u8)((set_mileage % 100000000) / 1000000));
    s_buf[s_len++] = _bintobcd((u8)((set_mileage % 1000000) / 10000));
    s_buf[s_len++] = _bintobcd((u8)((set_mileage % 10000) / 100));
    s_buf[s_len++] = _bintobcd((u8)(set_mileage % 100));

    s_buf[s_len++] = _bintobcd((u8)((total_tmp % 100000000) / 1000000));
    s_buf[s_len++] = _bintobcd((u8)((total_tmp % 1000000) / 10000));
    s_buf[s_len++] = _bintobcd((u8)((total_tmp % 10000) / 100));
    s_buf[s_len++] = _bintobcd((u8)(total_tmp % 100));

    rsp.total = s_len;
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rcd_send_data(from, RCD_C_OK, 0x03, s_buf, s_len);
}

/****************************************************************************
* 名称:    analyse_rcd0x04_plus ()
* 功能：解析脉冲参数
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x04_plus(u8 from, u8 *str, u16 len) {
    u8 s_buf[128] = {0};
    u16 s_len;

    if (str == NULL || len > D_BUF_LEN)
        return;

    //read_mfg_parameter();
    s_len = 0;
    s_buf[s_len++] = _bintobcd(sys_time.year);
    s_buf[s_len++] = _bintobcd(sys_time.month);
    s_buf[s_len++] = _bintobcd(sys_time.date);
    s_buf[s_len++] = _bintobcd(sys_time.hour);
    s_buf[s_len++] = _bintobcd(sys_time.min);
    s_buf[s_len++] = _bintobcd(sys_time.sec);
    s_buf[s_len++] = ((u8)(factory_para.set_speed_plus >> 8));
    s_buf[s_len++] = ((u8)(factory_para.set_speed_plus));
    rsp.total = s_len;
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rcd_send_data(from, RCD_C_OK, 0x04, s_buf, s_len);
}

/****************************************************************************
* 名称:    analyse_rcd0x05_vehicle_info ()
* 功能：解析车辆信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x05_vehicle_info(u8 from, u8 *str, u16 len) {
    u8 s_buf[128] = {0};
    u16 s_len;

    if (str == NULL || len > D_BUF_LEN)
        return;

    read_mfg_parameter();
    s_len = 0;
    s_len += _memcpy_len(&s_buf[s_len], car_cfg.vin, 17);
    s_len += _memcpy_len(&s_buf[s_len], car_cfg.license, 12);
    s_len += _memcpy_len(&s_buf[s_len], car_cfg.type, 12);
    rsp.total = s_len;
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rcd_send_data(from, RCD_C_OK, 0x05, s_buf, s_len);
}

/****************************************************************************
* 名称:    analyse_rcd0x06_signal_cfg ()
* 功能：解析管脚配置信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x06_signal_cfg(u8 from, u8 *str, u16 len) {
//    signal_define_union sig_state;
    u8 s_buf[256] = {0};
    u16 s_len;

    if (str == NULL || len > D_BUF_LEN)
        return;

    read_mfg_parameter();

    s_len = 0;
    s_buf[s_len++] = _bintobcd(sys_time.year);
    s_buf[s_len++] = _bintobcd(sys_time.month);
    s_buf[s_len++] = _bintobcd(sys_time.date);
    s_buf[s_len++] = _bintobcd(sys_time.hour);
    s_buf[s_len++] = _bintobcd(sys_time.min);
    s_buf[s_len++] = _bintobcd(sys_time.sec);

#if 0
	/*
	if(0 != car_state.bit.carriage )
		sig_state.b.s0 = 1;
	else
		sig_state.b.s0 = 0;

	sig_state.b.s1 = tr9_car_status.bit.res4 ;	//车门状态
	sig_state.b.s2 = tr9_car_status.bit.res3 ;	
	sig_state.b.s3 = tr9_car_status.bit.near_light;	//近光
	sig_state.b.s4 = tr9_car_status.bit.far_light ;	//远光
	sig_state.b.s5 = tr9_car_status.bit.right ;
	sig_state.b.s6 = tr9_car_status.bit.left ;
	sig_state.b.s7 = tr9_car_status.bit.brake;		//制动

	*/
   

    //应付检查
    /*
    _memset((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
    _memcpy((u8 *)&sig_cfg.b0, "载货状态", 8);
    _memcpy((u8 *)&sig_cfg.b1, "车门锁定", 8);
    _memcpy((u8 *)&sig_cfg.b2, "油路断开", 8);
    _memcpy((u8 *)&sig_cfg.b3, "近光", 6);
    _memcpy((u8 *)&sig_cfg.b4, "远光", 6);
    _memcpy((u8 *)&sig_cfg.b5, "右转向", 4);
    _memcpy((u8 *)&sig_cfg.b6, "左转向", 4);
    _memcpy((u8 *)&sig_cfg.b7, "制动", 4);
	*/

#endif

    s_buf[s_len++] = 0x01;

    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b0, 10);
    s_len += 10;
    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b1, 10);
    s_len += 10;
    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b2, 10);
    s_len += 10;
    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b3, 10);
    s_len += 10;
    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b4, 10);
    s_len += 10;
    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b5, 10);
    s_len += 10;
    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b6, 10);
    s_len += 10;
    _memcpy_len(&s_buf[s_len], (u8 *)&sig_cfg.b7, 10);
    s_len += 10;
    rsp.total = s_len;
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rcd_send_data(from, RCD_C_OK, 0x06, s_buf, s_len);
}

/****************************************************************************
* 名称:    analyse_rcd0x06_signal_cfg ()
* 功能：解析记录仪唯一ID
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x07_unique_ID(u8 from, u8 *str, u16 len) {
    u8 s_buf[128] = {0};
    u16 s_len;
    u8 sn_buf[9] = {"285212807"};
    if (str == NULL || len > D_BUF_LEN)
        return;

    read_mfg_parameter();

    _memset((u8 *)&unique_id, 0x00, sizeof(unique_id));
    _memcpy((u8 *)&unique_id.ccc, "C000000", 7);

    _memcpy((u8 *)&unique_id.model, "Z/HT-9C                   ", 16);

    unique_id.time[0] = 0x22;
    unique_id.time[1] = 0x03;
    unique_id.time[2] = 0x01;
    unique_id.time[3] = 0x16;
    unique_id.time[4] = 0x10;
    unique_id.time[5] = 0x38;
    _str2tobcd(&unique_id.sn[0], &sn_buf[0]);
    _str2tobcd(&unique_id.sn[1], &sn_buf[2]);
    _str2tobcd(&unique_id.sn[2], &sn_buf[4]);
    _str2tobcd(&unique_id.sn[3], &sn_buf[6]);
    unique_id.res = 0x72345;

    s_len = 0;
    s_len += _memcpy_len(&s_buf[s_len], (u8 *)&unique_id.ccc, 7);
    s_len += _memcpy_len(&s_buf[s_len], (u8 *)&unique_id.model, 16);
    s_len += _memcpy_len(&s_buf[s_len], (u8 *)&unique_id.time, 3);
    s_len += _memcpy_len(&s_buf[s_len], (u8 *)&unique_id.sn, 4);
    s_len += _memcpy_len(&s_buf[s_len], (u8 *)&unique_id.res, 4);
    rsp.total = 35;
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rcd_send_data(from, RCD_C_OK, 0x07, s_buf, 35);
}

/****************************************************************************
* 名称:    analyse_rcd0x08_speed_record ()
* 功能：提取2个日历天的平均速度记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x08_speed_record(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x08;
    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, &str[0], 6);
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6);
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t; //最大单位数据块个数

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x08;
    }
    pos48.ifnew = false;

    rsp.type = T_RCD_48; //传输
    rsp.from = from;     //传输类型

    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6); //开始时间
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);     //结束时间

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x08:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x08, s_buf, 0);
        logd("rcd err: 08 send node");
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x09_position ()
* 功能：提取360 小时的位置信息记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x09_position(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x09;

    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, str, 6); //起始时间
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6); //结束时间
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t;

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x09;
    }
    pos360.ifnew = false;
    rsp.type = T_RCD_360;
    rsp.from = from;
    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6);
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x09:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x09, s_buf, 0);
        logd("rcd err: 09 send node");
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x10_accident ()
* 功能：提取事故疑点信息记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x10_accident(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x10;

    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, str, 6);
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6);
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t;

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x10;
    }

    rsp.type = T_RCD_ACCIDENT;
    rsp.from = from;
    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6);
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x10:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x10, s_buf, 0);
        logd("rcd err: 10 send node");
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x11_drive_OT ()
* 功能：提取疲劳驾驶信息记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x11_drive_OT(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x11;

    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, str, 6);
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6);
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t;

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x11;
    }

    rsp.type = T_RCD_DRIVER_OT;
    rsp.from = from;
    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6);
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x11:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x11, s_buf, 0);
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x12_driver_login ()
* 功能：提取驾驶员信息记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x12_driver_login(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x12;

    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, str, 6);
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6);
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t;

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x12;
    }

    rsp.type = T_RCD_DRIVER_LOG;
    rsp.from = from;
    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6);
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x12:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x12, s_buf, 0);
        logd("rcd err: 12 send node");
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x13_power_log ()
* 功能：提取电源信息记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x13_power_log(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x13;

    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, &str[0], 6);
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6);
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t;

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x13;
    }

    rsp.type = T_RCD_POWER_LOG;
    rsp.from = from;
    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6);
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x13:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x13, s_buf, 0);
        logd("rcd err: 10 send node");
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x14_parameter_log ()
* 功能：提取参数修改记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x14_parameter_log(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x14;

    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, &str[0], 6);
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6);
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t;

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x14;
    }

    rsp.type = T_RCD_PARAMETER_LOG;
    rsp.from = from;
    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6);
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x14:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x14, s_buf, 0);
        logd("rcd err: 14 send node");
    }
}

/*********************************************************
名    称：analyse_rcd0x15_speed_log
功    能：采集外部供电记录
输入参数：无
输    出：无
编写日期：2013-01-08
**********************************************************/

static void analyse_rcd0x15_speed_log(u8 from, u8 *str, u16 len) {
    time_t start_t;
    time_t end_t;
    u16 max_len_t;
    u8 s_buf[128] = {0x00};
    bool ret = false;

    if (str == NULL || len > D_BUF_LEN)
        goto err_rcd0x15;

    str[5] = 0;
    str[11] = 0;
    _memcpy((u8 *)&start_t, &str[0], 6);
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6);
    _nbcdtobin((u8 *)&end_t, 6);
    max_len_t = (str[12] << 8) | str[13];
    rsp.packnum = max_len_t;

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x15;
    }

    rsp.type = T_RCD_SPEED_LOG;
    rsp.from = from;
    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6);
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x15:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x15, s_buf, 0);
        logd("rcd err: 15 send node");
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x81_license ()
* 功能：设置驾驶员证号
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x81_license(u8 from, u8 *str, u16 len) {
    u8 r_buf[128] = {0};
    u8 s_buf[128] = {0};
    u16 s_len = 0;
    u32 code = 0;

    if (str == NULL || len > 128)
        return;

    _memcpy(r_buf, str, len);
    read_mfg_parameter();
    code = (str[0] << 16) | (str[1] << 8) | (str[2]);
    DriverCodeBcd((u32 *)&code, driver_cfg.driver_code, 6, 0);

    _memset((u8 *)&driver_cfg.license, 0, 20);
    s_len = 0;
    s_len += _memcpy_len((u8 *)&driver_cfg.license, &r_buf[3], 18);

    _memset((u8 *)&mix.ic_driver, 0, 18);
    _memcpy((u8 *)&mix.ic_driver, (u8 *)&driver_cfg.license, 18); //2018-1-22,陈工添加,解决更换司机,疲劳驾驶报警不消除问题
    flash_write_normal_parameters();

    rcd_para_inf.en = true;
    rcd_para_inf.type = 0x81;

    //设置的车辆信息要发给从机
    rcd_send_data(from, RCD_C_OK, 0x81, s_buf, 0);
}

/****************************************************************************
* 名称:    analyse_rcd0x82_set_vehicle_info ()
* 功能：设置车辆信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x82_set_vehicle_info(u8 from, u8 *str, u16 len) {
    u8 r_buf[128];
    u8 s_buf[128];
    u16 s_len;

    if (str == NULL || len > 128)
        return;

    _memcpy(r_buf, str, len);

    read_mfg_parameter();
    _memset((u8 *)&car_cfg.vin, 0, 17);
    _memset((u8 *)&car_cfg.license, 0, 12);
    _memset((u8 *)&car_cfg.type, 0, 16);
    s_len = 0;
    s_len += _memcpy_len((u8 *)&car_cfg.vin, &r_buf[s_len], 17);
    s_len += _memcpy_len((u8 *)&car_cfg.license, &r_buf[s_len], 12);
    _strncpy((u8 *)&car_cfg.type, &r_buf[s_len], 16);

    //_memset( (u8*)&register_cfg.license, 0, 10 );
    //_memcpy((u8 *)&register_cfg.license, (u8*)&car_cfg.license, 10);

    flash_write_import_parameters();

    rcd_para_inf.en = true;
    rcd_para_inf.type = 0x82;

    //设置的车辆信息要发给从机
    rcd_send_data(from, RCD_C_OK, 0x82, s_buf, 0);
}

/****************************************************************************
* 名称:    analyse_rcd0x83_set_setting_time ()
* 功能：设置安装时间
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x83_set_setting_time(u8 from, u8 *str, u16 len) {
    u8 s_buf[64];
//    u8 i;
    time_t t1;
    time_t t2;
    bool ret = false;

    if (str == NULL || len > 64)
        return;

    read_mfg_parameter();
    _memcpy((u8 *)&t1, str, 6);

    t2.year = _bcdtobin(t1.year);
    t2.month = _bcdtobin(t1.month);
    t2.date = _bcdtobin(t1.date);
    t2.hour = _bcdtobin(t1.hour);
    t2.min = _bcdtobin(t1.min);
    t2.sec = _bcdtobin(t1.sec);

    ret = _verify_time(&t2);
    if (ret == false) {
        rcd_send_data(from, RCD_C_SET_ERR, 0x83, s_buf, 0);
    } else {
        _memcpy((u8 *)&factory_para.setting_time, (u8 *)&t1, 6);
        write_mfg_parameter();

        rcd_para_inf.en = true;
        rcd_para_inf.type = 0x83;

        rcd_send_data(from, RCD_C_OK, 0x83, s_buf, 0);
    }
}

/****************************************************************************
* 名称:    analyse_rcd0x84_set_signal_cfg ()
* 功能：设置管脚端口信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0x84_set_signal_cfg(u8 from, u8 *str, u16 len) {
    u8 r_buf[128];
    u8 s_buf[128];
    u16 r_len = 0;
    u8 ll = 0;

    if (str == NULL || len > 128)
        return;

    _memcpy(r_buf, str, len);

    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b0, &r_buf[r_len], 10);
    else
        r_len += 10;
    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b1, &r_buf[r_len], 10);
    else
        r_len += 10;
    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b2, &r_buf[r_len], 10);
    else
        r_len += 10;
    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b3, &r_buf[r_len], 10);
    else
        r_len += 10;
    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b4, &r_buf[r_len], 10);
    else
        r_len += 10;
    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b5, &r_buf[r_len], 10);
    else
        r_len += 10;
    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b6, &r_buf[r_len], 10);
    else
        r_len += 10;
    ll = _strlen(r_buf + r_len);
    if (ll > 0)
        r_len += _memcpy_len((u8 *)&sig_cfg.b7, &r_buf[r_len], 10);

    write_mfg_parameter();

    rcd_para_inf.en = true;
    rcd_para_inf.type = 0x84;

    rcd_send_data(from, RCD_C_OK, 0x84, s_buf, 0);
}

/****************************************************************************
* 名称:    analyse_rcd0xc2_set_real_time ()
* 功能：设置实时时间
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0xc2_set_real_time(u8 from, u8 *str, u16 len) {
    u8 s_buf[64];
    time_t t1;
    time_t t2;
    bool ret = false;

    if (str == NULL || len > 64)
        return;

    _memcpy((u8 *)&t1, str, 6);

    t2.year = _bcdtobin(t1.year);
    t2.month = _bcdtobin(t1.month);
    t2.date = _bcdtobin(t1.date);
    t2.hour = _bcdtobin(t1.hour);
    t2.min = _bcdtobin(t1.min);
    t2.sec = _bcdtobin(t1.sec);

    ret = _verify_time(&t2);
    if (ret == false) {
        rcd_send_data(from, RCD_C_SET_ERR, 0xc2, s_buf, 0);
    } else {
        refreshTime = false;
        _memcpy((u8 *)&sys_time, (u8 *)&t2, 6);
        rcd_para_inf.en = true;
        rcd_para_inf.type = 0xc2;

        rcd_send_data(from, RCD_C_OK, 0xc2, s_buf, 0);

        save_gps_basemation(&gps_base, &sys_time);
    }
}

/****************************************************************************
* 名称:    analyse_rcd0xc3_set_plus ()
* 功能：设置记录仪脉冲系数
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0xc3_set_plus(u8 from, u8 *str, u16 len) {
    u8 r_buf[128];
    u8 s_buf[16];

    u16 plus1;

    if (str == NULL || len > 32)
        return;

    read_mfg_parameter();

    _memcpy(r_buf, str, 8);

    plus1 = 0;
    plus1 = r_buf[6];
    plus1 <<= 8;
    plus1 += r_buf[7];

    if (plus1 > 50000) {
        rcd_send_data(from, RCD_C_SET_ERR, 0xc3, s_buf, 0);
    } else {
        factory_para.set_speed_plus = plus1;
        factory_para.swit = 0x0;
        if (plus1 == 0x00) {
            factory_para.speed_cala = true;
        } else if (plus1 < 10) {
            factory_para.swit = 0x55;
            write_mfg_parameter();
        } else
            write_mfg_parameter();

        ///rcd_speed_m.state = SPEED_STATE_SET;

        rcd_para_inf.en = true;
        rcd_para_inf.type = 0xc3;

        rcd_send_data(from, RCD_C_OK, 0xc3, s_buf, 0);
    }
}

/****************************************************************************
* 名称:    analyse_rcd0xc4_set_mileage ()
* 功能：设置记录仪起始里程
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0xc4_set_mileage(u8 from, u8 *str, u16 len) {
    u8 r_buf[128];
    u8 s_buf[16];
    u32 mileage;

    if (str == NULL || len > 32)
        return;

    read_mfg_parameter();

    _memcpy(r_buf, str, 20);

    mileage = 0;
    mileage += _bcdtobin(r_buf[0]) * 1000000;
    mileage += _bcdtobin(r_buf[1]) * 10000;
    mileage += _bcdtobin(r_buf[2]) * 100;
    mileage += _bcdtobin(r_buf[3]);

    if (mileage > 99999999) {
        rcd_send_data(from, RCD_C_SET_ERR, 0xc4, s_buf, 0);
    } else {
        factory_para.set_mileage = mileage * 100;
        write_mfg_parameter();

        run.total_dist = mileage * 100;
        save_run_parameterer();

        rcd_para_inf.en = true;
        rcd_para_inf.type = 0xc4;

        rcd_send_data(from, RCD_C_OK, 0xc4, s_buf, 0);
    }
}

/****************************************************************************
* 名称:    analyse_rcd0xE0_docimasy ()
* 功能：检定
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void analyse_rcd0xE0_docimasy(u8 from, u8 *str, u16 len) {
    u8 buff = 0xE0;
    rcd_send_data(from, RCD_C_OK, 0xE0, &buff, 0);
}
static void analyse_rcd0xE1_docimasy(u8 from, u8 *str, u16 len) {
//    u8 sdf = 0;
    u8 uLen = 0;
    u8 uBuff[100] = {0};
    //u8 sn_buf[9] = {"285212807"};
    u32 sn = 285212807;

    read_mfg_parameter();

    sprintf((char *)uBuff, "%7s", "C000000");
    uLen = 7;

    sprintf((char *)(uBuff + uLen), "%16s", "Z/HT-9C");
    uLen += 16;

    uLen += _memcpy_len(uBuff + uLen, (u8 *)&factory_para.setting_time, 3);

    unique_id.res = 0x72345;
    uLen += _sw_endian(uBuff + uLen, (u8 *)&sn, 4);
    uLen += _sw_endian(uBuff + uLen, (u8 *)&unique_id.res, 4);
    uLen += 1; //备用是五个字节
    //脉冲
    uLen += _sw_endian(uBuff + uLen, (u8 *)&factory_para.set_speed_plus, 2);
    //速度
    uLen += _sw_endian(uBuff + uLen, (u8 *)&mix.speed, 2);
    //总里程
    uLen += _sw_endian(uBuff + uLen, (u8 *)&run.total_dist, 4);
    uBuff[uLen++] = rcd_io.reg;

    rcd_send_data(from, RCD_C_OK, 0xE1, uBuff, uLen);
}

static void analyse_rcd0xE2_docimasy(u8 from, u8 *str, u16 len) {
    u8 buff = 0xE2;
    rcd_send_data(from, RCD_C_OK, 0xE2, &buff, 0);
}

static void analyse_rcd0xE3_docimasy(u8 from, u8 *str, u16 len) {
    u8 buff = 0xE3;
    rcd_send_data(from, RCD_C_OK, 0xE3, &buff, 0);
}

static void analyse_rcd0xE4_docimasy(u8 from, u8 *str, u16 len) {
    u8 buff = 0xE4;
    rcd_send_data(from, RCD_C_OK, 0xE4, &buff, 0);
}

/****************************************************************************
* 名称:    CmdRcdTab ()
* 功能：2012行驶记录仪协议
* 入口参数：无
* 出口参数：无
****************************************************************************/
cmd_handset const CmdRcdTab[] =
    {
        {0x00, analyse_rcd0x00_ver},
        {0x01, analyse_rcd0x01_driver},
        {0x02, analyse_rcd0x02_real_time},
        {0x03, analyse_rcd0x03_mileage},
        {0x04, analyse_rcd0x04_plus},
        {0x05, analyse_rcd0x05_vehicle_info},
        {0x06, analyse_rcd0x06_signal_cfg},
        {0x07, analyse_rcd0x07_unique_ID},
        {0x08, analyse_rcd0x08_speed_record},
        {0x09, analyse_rcd0x09_position},
        {0x10, analyse_rcd0x10_accident},
        {0x11, analyse_rcd0x11_drive_OT},
        {0x12, analyse_rcd0x12_driver_login},
        {0x13, analyse_rcd0x13_power_log},
        {0x14, analyse_rcd0x14_parameter_log},
        {0x15, analyse_rcd0x15_speed_log},
        {0x81, analyse_rcd0x81_license},
        {0x82, analyse_rcd0x82_set_vehicle_info},
        {0x83, analyse_rcd0x83_set_setting_time},
        {0x84, analyse_rcd0x84_set_signal_cfg},
        {0xc2, analyse_rcd0xc2_set_real_time},
        {0xc3, analyse_rcd0xc3_set_plus},
        {0xc4, analyse_rcd0xc4_set_mileage},
        {0xe0, analyse_rcd0xE0_docimasy},
        {0xe1, analyse_rcd0xE1_docimasy},
        {0xe2, analyse_rcd0xE2_docimasy},
        {0xe3, analyse_rcd0xE3_docimasy},
        {0xe4, analyse_rcd0xE4_docimasy},
        {NULL, NULL}};

/****************************************************************************
* 名称:    rcd_communicate_parse ()
* 功能：记录仪数据解析
* 入口参数：无
* 出口参数：无
****************************************************************************/
u8 rcd_communicate_parse(u8 from, u8 id, u8 *str, u16 len) {
    u8 i;

    if (str == NULL || len > D_BUF_LEN){
        loge("3c err");//USART0:3c
        return CMD_ERROR;
    }

    for (i = 0; CmdRcdTab[i].func != NULL; i++) {
        if (id == CmdRcdTab[i].index) {
            load_new_rcd_data(from, id);
            logd("3c do"); //USART0:3c
            (*CmdRcdTab[i].func)(from, str, len);
            return CMD_ACCEPT;
        }
    }
    return CMD_ERROR;
}

/****************************************************************************
* 名称:    rcd_data_send ()
* 功能：记录仪数据发送管理
* 入口参数：无
* 出口参数：无
****************************************************************************/
void rcd_data_send(void) {
    u8 msg[RCD_COM_MAX_SIZE];
    static u16 msg_len = 0; //整个组包长度
    u16 cur_index = 0;      //临时发送头指针
    u16 i = 0;
    u8 buf[32];
    u16 tmp;
//    u16 sum = 0;

    static enum E_LINK_MANAGE {
        E_RCD_IDLE, //空闲
        E_RCD_SEND, //发送
        E_RCD_SACK, //等待回复
        E_RCD_ERR   //错误
    } step = E_RCD_IDLE;

    switch (step) {
    case E_RCD_IDLE:
        if (rsp.type >= T_RCD_END) {
            _memset((u8 *)&rsp, 0x00, sizeof(rsp));
            return;
        }

        if (rsp.type != T_RCD_NULL) {
            if ((rsp.cur.tail != rsp.cur.head) && (rsp.send_len < rsp.total)) {
                step = E_RCD_SEND;
                break;
            } else {
                //logdr("[0]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....rsp.send_len[%d]....rsp.total[%d]",rsp.cur.head,rsp.cur.tail,rsp.cur_no,rsp.send_len,rsp.total);
                //数据发送结束
                _memset((u8 *)&rsp, 0x00, sizeof(rsp));
                return;
            }
        }

        //logdr("[1]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);

        if (rsp.retry_en) {
            if (rsp.retry_no < rsp.retry_cnt) {
                //logdr("[2]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);
                spi_flash_read(buf, _retry_addr(rsp.retry_no), 2);
                tmp = 0;
                tmp = buf[0];
                tmp <<= 8;
                tmp += buf[1];
                tmp -= 1;
                tmp *= rsp.cell_num;
                rsp.cur.head = decp_rcd_p(rsp.type, rsp.start_node, tmp);
                rsp.retry_no++;
                rsp.retry_delay_tick = tick;
                step = E_RCD_SEND;
                break;
            } else {
                //清除
                if (_coveri(rsp.retry_delay_tick) > 600) {
                    //logdr("[3]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);
                    rsp.retry_delay_tick = tick;
                    _memset((u8 *)&rsp, 0x00, sizeof(rsp));
                    return;
                }
            }
            break;
        }
        break;
    case E_RCD_SEND:
        if ((rsp.cur.tail == rsp.cur.head)) {
            //logdr("[4]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);
            step = E_RCD_IDLE;
            break;
        }

        if (rsp.from == FROM_NET) {
            if (!car_login || !gc.gprs >= NET_ONLINE)
                break;

            if (!free_gsm() || free_gprs_send() < RCD_COM_MAX_SIZE)
                break;
        }

        cur_index = rsp.cur.head;
        msg_len = 0;
        /*********22-06-14********修改此段一下内容********************/
#if 0		
            for (i = 0; i < rsp.cell_num; i++)
            {
                if (rsp.cur.tail == cur_index)
		{
			//logdr("[10]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);
                    break;
		}                    
                msg_len += rcd_assemble_body(cur_index, &msg[msg_len]);
                cur_index = decp_rcd_p(rsp.type, cur_index, 1);
            }
#else

        for (i = 0; i < rsp.cell_num; i++) {
            if (rsp.cur.tail == cur_index) {
                break;
            } else if (1000 > msg_len) {
                msg_len += rcd_assemble_body(cur_index, &msg[msg_len]);
                cur_index = decp_rcd_p(rsp.type, cur_index, 1);
            } else
                logd("[%s->%s :%d]数据溢出", __FILE__, __FUNCTION__, __LINE__);
        }
        rsp.cur.tail = rsp.cur.head;

#endif

        /*********22-06-14********修改以上内容********************/

        if (msg_len >= RCD_COM_MAX_SIZE) {
            loge("rcd send err : msg all len");
            rsp.cur.head = decp_rcd_p(rsp.type, rsp.cur.head, 1);
            step = E_RCD_IDLE;
            break;
        }

        rsp.send_jif = jiffies;
        rsp.ack = false;

        rcd_send_data(rsp.from, RCD_C_OK, rsp.id, msg, (u16)msg_len);
        //rcd_send_data(FROM_U4, RCD_C_OK, rsp.id, msg, (u16)msg_len);//22-06-06@秦屏蔽,理由:数据需要从串口输出
        step = E_RCD_SACK;
        break;
    case E_RCD_SACK:
        if (rsp.from != FROM_NET) {
            if (_pasti(rsp.send_jif) < 250) {
                //logdr("[9]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....rsp.send_len[%d]....rsp.total[%d]",rsp.cur.head,rsp.cur.tail,rsp.cur_no,rsp.send_len,rsp.total);
                break;
            }
        } else {
            if (_pasti(rsp.send_jif) < 3000) {
                //logdr("[8]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);
                break;
            }
        }

        rsp.err_cnt = 0;
        for (i = 0; i < rsp.cell_num; i++) {
            if (rsp.cur.tail != rsp.cur.head) {
                rsp.cur.head = decp_rcd_p(rsp.type, rsp.cur.head, 1);
            }
        }

        if (rsp.type == T_RCD_48) //加速传输
        {
            pos48.ifnew = true;
            pos48.head = rsp.cur.head;
            pos48.tail = rsp.cur.tail;
        }
        if (rsp.type == T_RCD_360) //加速传输
        {
            pos360.ifnew = true;
            pos360.head = rsp.cur.head;
            pos360.tail = rsp.cur.tail;
        }

        //logdr("..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);
        /*
            if( ( rsp.cur.tail==rsp.cur.head) && ( usbc.step > 0 ) )
            {
            	_memset( (u8 *)&pos48 ,0,sizeof(pos48) );
            	_memset( (u8 *)&pos360,0,sizeof(pos360) );
            	step = E_RCD_ERR;
            	break;
            }
            */

        rsp.send_len += msg_len;
        step = E_RCD_IDLE;
        break;
    case E_RCD_ERR:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        //logdr("[6]..... get 48hour rsp.cur.head[%d] rsp.cur.tail[%d]....rsp.cur_no=[%d]....",rsp.cur.head,rsp.cur.tail,rsp.cur_no);
        step = E_RCD_IDLE;
        break;
    default:
        step = E_RCD_IDLE;
    }
}

/****************************************************************************
* 名称:    get_uart_is_rcd ()
* 功能：判断串口是否是行驶记录仪功能
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool get_uart_is_rcd(u8 from_t) {
    if (u1_m.cur.b.rcd_3c == true && from_t == FROM_U1) {
        return true;
    } else {
        return false;
    }
}

/****************************************************************************
* 名称:    uart_rcd_parse_proc ()
* 功能：串口提取RCD 程序(串口提取行驶记录仪数据)
* 入口参数：无
* 出口参数：无
****************************************************************************/
void uart_rcd_parse_proc(void) {
    typedef enum {
        E_UART_RCD_IDLE,
        E_UART_RCD_RECV,
        E_UART_RCD_EXIT
    } E_UART_RCD_STEP;

    static E_UART_RCD_STEP step = E_UART_RCD_IDLE;
    bool ret;
    u8 ret1;

    u8 r_buf[R1_BUF_SIZE];
    u8 pack[T1_BUF_SIZE];
    u8 xvalue;

    u16 r_len = 0;
    u16 pack_len = 0;
    s16 start_pos = 0;
    u16 i = 0;

    static u8 from_id = 0;
    static UART_MANAGE_STRUCT present;

    switch (step) {
    case E_UART_RCD_IDLE:
        from_id = get_uart_news(&present); //获取当前是哪个串口有新数据,并且返回当前的串口ID
        ret = get_uart_is_rcd(from_id);    //判断串口是否为RCD功能
        if (ret)  {                         //如果是RCD功能
            step = E_UART_RCD_RECV;
            logd("3c recv");
        }
        break;
    case E_UART_RCD_RECV:
        _memset(r_buf, 0x00, R1_BUF_SIZE);
        r_len = copy_uart_news(from_id, r_buf, present.cur_p, present.c_len); //拷贝串口信息,返回信息长度
        if (r_len < 5 || r_len > R1_BUF_SIZE) {
            step = E_UART_RCD_EXIT;
            break;
        }

        start_pos = 0;
        while (start_pos < r_len) //在大循环中找到命令帧数据
        {
            if ((r_len - start_pos) < 7) //最小帧长度
            {
            EXIT_UART_RCD_PARSE_ERR:
                step = E_UART_RCD_EXIT;
                return;
            }

            if (r_buf[start_pos] == 0xaa && r_buf[start_pos + 1] == 0x75) //判断是否为命令帧
            {
                pack_len = 0;
                pack_len = r_buf[start_pos + 3]; //获取数据块长度,高字节
                pack_len <<= 8;
                pack_len += r_buf[start_pos + 4]; //获取数据块长度,低字节
                pack_len += 7;

                if ((r_len - start_pos) < pack_len) //长度不足，舍弃(实际接收到的数据长度小于数据包中的长度)
                    goto EXIT_UART_RCD_PARSE_ERR;

                _memcpy(pack, &r_buf[start_pos], pack_len);

                xvalue = 0x00;
                for (i = 0; i < (pack_len - 1); i++) {
                    xvalue ^= pack[i];
                }

                // if( xvalue != pack_len[u1_len])
                // goto EXIT_UART_RCD_PARSE_ERR;

                ret1 = rcd_communicate_parse(from_id, pack[2], &pack[6], pack_len - 7);

                if (ret1 == CMD_ERROR) {
                    loge("3c err"); //USART0:3c
                }

                start_pos += pack_len;

            } else {
                start_pos++;
            }
        }
        step = E_UART_RCD_EXIT;
        break;
    case E_UART_RCD_EXIT:
        set_uart_ack(from_id);
        step = E_UART_RCD_IDLE;
        break;
    default:
        step = E_UART_RCD_IDLE;
        break;
    }
}

/********************************************************
* 函 数 名: USB_get_rcd_ver
* 函数功能: U盘获取行驶记录仪版本
* 入口参数: 无
* 返 回 值: 无
* 日    期: 2017
*********************************************************/
void USB_get_rcd_ver(void) {
    u8 i;
    u8 DataCode = 0x00; //数据代码
    u32 DataLen = 2;    //数据长度
    u8 RcdVer = 12;     //记录仪执行标准年号
    u8 RcdRev = 0;      //修改单号
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;

    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "执行标准版本年号");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    *pf++ = RcdVer; //执行标准
    *pf++ = RcdRev; //修改单号
    TotalLen += 6;
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_driver
* 函数功能: U盘获取当前驾驶人信息
* 入口参数: 无
* 返 回 值: 无
* 日    期: 2017-12-15
*********************************************************/
void USB_get_driver(void) {
    u8 i;
    u8 DataCode = 0x01; //数据代码
    u32 DataLen = 18;   //数据长度
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据

    u8 *pf;

    pf = usb_rsp.rcdDataBuf;

    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "当前驾驶人信息");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    TotalLen += _memcpy_len(pf, (u8 *)&driver_cfg.license, DataLen); //从业资格证号 驾驶证号
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_real_time
* 函数功能: U盘获取记录仪实时时间
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
void USB_get_real_time(void) {
    u8 i;
    u8 DataCode = 0x02; //数据代码
    u32 DataLen = 6;    //数据长度
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据
    u8 *pf;
    //    u8 * ptmp;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "实时时间");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;

    //实时时间
    *pf++ = _bintobcd(sys_time.year);
    *pf++ = _bintobcd(sys_time.month);
    *pf++ = _bintobcd(sys_time.date);
    *pf++ = _bintobcd(sys_time.hour);
    *pf++ = _bintobcd(sys_time.min);
    *pf++ = _bintobcd(sys_time.sec);
    TotalLen += 6;
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_mileage
* 函数功能: U盘获取累计行驶里程
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
void USB_get_mileage(void) {
    u8 i;
    u8 DataCode = 0x03; //数据代码
    u32 DataLen = 20;   //数据长度
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u8 *pf;
    //    u8 * ptmp;
    u32 total_tmp;

    pf = usb_rsp.rcdDataBuf;

    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    read_mfg_parameter(); //读取系统配置参数
    total_tmp = run.total_dist / 100;

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "累计行驶里程");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //系统实时时间
    *pf++ = _bintobcd(sys_time.year);
    *pf++ = _bintobcd(sys_time.month);
    *pf++ = _bintobcd(sys_time.date);
    *pf++ = _bintobcd(sys_time.hour);
    *pf++ = _bintobcd(sys_time.min);
    *pf++ = _bintobcd(sys_time.sec);
    //记录仪初次安装时间
    *pf++ = _bintobcd(factory_para.setting_time.year);
    *pf++ = _bintobcd(factory_para.setting_time.month);
    *pf++ = _bintobcd(factory_para.setting_time.date);
    *pf++ = _bintobcd(factory_para.setting_time.hour);
    *pf++ = _bintobcd(factory_para.setting_time.min);
    *pf++ = _bintobcd(factory_para.setting_time.sec);
    //初始里程
    *pf++ = _bintobcd((u8)((factory_para.set_mileage % 100000000) / 1000000));
    *pf++ = _bintobcd((u8)((factory_para.set_mileage % 1000000) / 10000));
    *pf++ = _bintobcd((u8)((factory_para.set_mileage % 10000) / 100));
    *pf++ = _bintobcd((u8)(factory_para.set_mileage % 100));
    //累计行驶里程
    *pf++ = _bintobcd((u8)((total_tmp % 100000000) / 1000000));
    *pf++ = _bintobcd((u8)((total_tmp % 1000000) / 10000));
    *pf++ = _bintobcd((u8)((total_tmp % 10000) / 100));
    *pf++ = _bintobcd((u8)(total_tmp % 100));
    TotalLen += 20;
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_pulse
* 函数功能: U盘获取脉冲系数
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
void USB_get_pulse(void) {
    u8 i;
    u8 DataCode = 0x04; //数据代码
    u32 DataLen = 8;    //数据长度
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u8 *pf;
    //    u8 * ptmp;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    read_mfg_parameter(); //读取系统配置参数

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "脉冲系数");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //系统实时时间
    *pf++ = _bintobcd(sys_time.year);
    *pf++ = _bintobcd(sys_time.month);
    *pf++ = _bintobcd(sys_time.date);
    *pf++ = _bintobcd(sys_time.hour);
    *pf++ = _bintobcd(sys_time.min);
    *pf++ = _bintobcd(sys_time.sec);

    *pf++ = ((u8)(factory_para.set_speed_plus >> 8));
    *pf++ = ((u8)(factory_para.set_speed_plus));
    TotalLen += 8;
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_vehicle_ifno
* 函数功能: U盘获取车辆信息
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
void USB_get_vehicle_ifno(void) {
    u8 i;
    u8 DataCode = 0x05; //数据代码
    u32 DataLen = 41;   //数据长度
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u8 *pf;
    u8 tmp_len = 0;
    //    recorder_para_struct recorder_para_tmp; //行驶记录仪临时结构体

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "车辆信息");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //spi_flash_read((u8 *)(&recorder_para_tmp), (flash_parameter_addr + flash_recorder_para_addr), (sizeof(recorder_para_tmp)));//读取行驶记录仪参数
    TotalLen += _memcpy_len(pf, ((u8 *)&car_cfg.vin), 17); //获取车辆VIN码
    pf += 17;
    tmp_len = _strlen((u8 *)&car_cfg.license);
    if ((tmp_len >= 8) && (tmp_len <= 12)) //如果车牌号码长度 ==8,表示用户设置过车牌
    {
        TotalLen += _memcpy_len(pf, ((u8 *)&car_cfg.license), tmp_len); //获取车牌号码
        pf += 8;
        for (i = 0; i < (12 - tmp_len); i++) //车牌号码不足12位用 0 填充
        {
            *pf++ = 0x00;
        }
        TotalLen += (12 - tmp_len);
    } else //没有设置车牌号码,用0来填充
    {
        for (i = 0; i < 12; i++) {
            *pf++ = 0x00;
            TotalLen += 1;
        }
    }
    TotalLen += _memcpy_len(pf, car_cfg.type, 12);
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_cfg_status_info
* 函数功能: U盘获取车辆状态信息
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
void USB_get_vehicle_status_info(void) {
    u8 i;
    u8 DataCode = 0x06; //数据代码
    u32 DataLen = 87;   //数据长度
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "状态信号配置信息");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //系统实时时间
    *pf++ = _bintobcd(sys_time.year);
    *pf++ = _bintobcd(sys_time.month);
    *pf++ = _bintobcd(sys_time.date);
    *pf++ = _bintobcd(sys_time.hour);
    *pf++ = _bintobcd(sys_time.min);
    *pf++ = _bintobcd(sys_time.sec);

    *pf++ = 0x01; //信号状态字节个数
    TotalLen += 7;
    _memset((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
    _memcpy((u8 *)&sig_cfg.b0, "信号1", 4);
    _memcpy((u8 *)&sig_cfg.b1, "信号2", 4);
    _memcpy((u8 *)&sig_cfg.b2, "信号3", 4);
    _memcpy((u8 *)&sig_cfg.b3, "近光灯", 6);
    _memcpy((u8 *)&sig_cfg.b4, "远光灯", 6);
    _memcpy((u8 *)&sig_cfg.b5, "右转", 4);
    _memcpy((u8 *)&sig_cfg.b6, "左转", 4);
    _memcpy((u8 *)&sig_cfg.b7, "刹车", 4);

    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b0, 10);
    pf += 10;
    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b1, 10);
    pf += 10;
    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b2, 10);
    pf += 10;
    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b3, 10);
    pf += 10;
    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b4, 10);
    pf += 10;
    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b5, 10);
    pf += 10;
    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b6, 10);
    pf += 10;
    TotalLen += _memcpy_len(pf, (u8 *)&sig_cfg.b7, 10);
    pf += 10;
    //logd("TotaLen = %d", TotalLen);
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_rcd_unique_ID
* 函数功能: U盘获取记录仪唯一性编号
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
void USB_get_rcd_unique_ID(void) {
    u8 i;
    u8 DataCode = 0x07; //数据代码
    u32 DataLen = 87;   //数据长度
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u8 *pf;
    u8 unique_time[3] = {0}; //记录仪生产日期BCD码2018-6-13
    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "记录仪唯一性编号");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //添加3C认证代码
    *pf++ = 'C';
    *pf++ = 0x00;
    *pf++ = 0x00;
    *pf++ = 0x00;
    *pf++ = 0x07;
    *pf++ = 0x02;
    *pf++ = 0x03;
    //产品认证型号
    _memcpy_len(pf, (u8 *)"TR9", sizeof("TR9"));
    pf += sizeof("TR9");
    for (i = 0; i < (16 - sizeof("TR9")); i++) //认证产品型号16个字节,不够补0
    {
        *pf++ = 0x00;
    }

    //记录仪生产日期年月日
    _str2tobcd(&unique_time[0], &unique_id.time[0]); //转换年2018-6-13
    _str2tobcd(&unique_time[1], &unique_id.time[2]); //转换月
    _str2tobcd(&unique_time[2], &unique_id.time[4]); //转换日
    *pf++ = unique_time[0];
    *pf++ = unique_time[1];
    *pf++ = unique_time[2];
    //生产流水号
    *pf++ = unique_id.sn[0];
    *pf++ = unique_id.sn[1];
    *pf++ = unique_id.sn[2];
    *pf++ = unique_id.sn[3];
    //备用5个字节
    *pf++ = 00;
    *pf++ = 00;
    *pf++ = 00;
    *pf++ = 00;
    *pf++ = 00;
    TotalLen += 35;
    usb_rsp.len = TotalLen;
}

/********************************************************
* 函 数 名: USB_get_rcd_48speed
* 函数功能: U盘获48小时速度信息
* 入口参数: 无
* 返 回 值: 无
* 日    期: 2017
*********************************************************/
bool USB_get_rcd_48speed(void) {
    u8 i;
    u8 DataCode = 0x08; //数据代码
    u32 DataLen;        //数据长度 = 数据包数 *
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u16 DataNum = 0;    //数据包数
    time_t time;
    // u8 buf_t[200];
    u8 ret;
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.rcd48_step_flg == true) {
        usb_rcd_pTmp.rcd48_step_flg = false;
        *pf++ = DataCode; //数据代码
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "行驶速度记录");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;

        DataNum = subp_rcd_p(usb_rcd_pTmp.rcd48_p_head, 0, RCD48_NODE_MAX_SIZE);
        //logd("DataNum = %d", DataNum);
        //获取数据总长度
        DataLen = DataNum * 126;
        //logd("TotalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //如果没有数据
            return true;
    }
    spi_flash_read(pf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_head), 126);
    TotalLen += 126;
    _memcpy((u8 *)&time, pf, 6);
    //logd("%02d-%02d-%02d %02d:%02d:%02d", time.year, time.month, time.date, time.hour, time.min, time.sec);
    // logd("48 = %d", usb_rcd_pTmp.rcd48_p_head);
    ret = _verify_time(&time);
    if (ret) {
        _nbintobcd(pf, 6);
        usb_rsp.len = TotalLen;
        return true;

    } else {
        return false;
    }
}

/********************************************************
* 函 数 名: USB_get_rcd_360loction
* 函数功能: U盘获360小时位置信息
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
bool USB_get_rcd_360loction(void) {
    u8 i;
    u8 DataCode = 0x09; //数据代码
    u32 DataLen;        //数据长度 = 数据包数 * 666
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u16 DataNum = 0;    //数据包数
    time_t time;
    u8 buf_t[20];
    u8 ret;
    u8 *pf;
    S_RCD_360HOUR_INFOMATION rcd360_inf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.rcd360_step_flg == true) //第一次要写入数据代码,数据名称,数据长度信息
    {
        usb_rcd_pTmp.rcd360_step_flg = false;
        *pf++ = DataCode; //数据代码
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "位置信息记录");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //获取数据总包数
        DataNum = subp_rcd_p(usb_rcd_pTmp.rcd360_p_head, usb_rcd_pTmp.rcd360_p_tail, RCD360_NODE_MAX_SIZE);
        logd("DataNum = %d", DataNum);
        //获取数据总长度
        DataLen = DataNum * RCD360_HOUR_INF_SIZE;
        //logd("TotalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //如果没有360位置数据
            return true;
    }

    spi_flash_read(buf_t, _rcd360_t_addr(usb_rcd_pTmp.rcd360_p_head), sizeof(time_t)); //打印2017-12-26 14:00:00
    _memcpy((u8 *)&time, buf_t, 6);
    // logd("%2d年%2d月%2d日%2d时%2d分%2d秒", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time); //时间校验
    if (ret) {
        //添加时间
        *pf++ = _bintobcd(time.year);
        *pf++ = _bintobcd(time.month);
        *pf++ = _bintobcd(time.date);
        *pf++ = _bintobcd(time.hour);
        *pf++ = _bintobcd(time.min);
        *pf++ = _bintobcd(time.sec);
        TotalLen += 6;

        for (i = 0; i <= 59; i++) {
            spi_flash_read((u8 *)&rcd360_inf, _rcd360_d_addr(usb_rcd_pTmp.rcd360_p_head, i), sizeof(rcd360_inf));
            _sw_endian(pf, (u8 *)&rcd360_inf.locate.lngi, 4); //经纬度数据要进行大小端转换
            pf += 4;
            _sw_endian(pf, (u8 *)&rcd360_inf.locate.lati, 4); //纬度
            pf += 4;
            _memcpy_len(pf, (u8 *)&rcd360_inf.heigh, 2); //海拔高度
            pf += 2;
            _memcpy_len(pf, (u8 *)&rcd360_inf.speed, 1); //速度
            pf += 1;
        }
        TotalLen += 660;
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* 函 数 名: USB_get_accident
* 函数功能: U盘获取事故疑点记录
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
bool USB_get_accident(void) {
    u8 i;
    u8 DataCode = 0x10; //数据代码
    u32 DataLen = 0;    //数据长度 = 数据包数 *
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u16 DataNum = 0;    //数据包数
    time_t time;
    u8 ret;
    u8 *pf;
    S_RCD_POSINTION_INFOMATION cvt_point;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.rcd_accident_flag == true) {
        usb_rcd_pTmp.rcd_accident_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "事故疑点记录");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //获取数据总包数
        DataNum = subp_rcd_p(usb_rcd_pTmp.rcd_accident_p_head, usb_rcd_pTmp.rcd_accident_p_tail, RCD_ACCIDENT_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_ACCIDENT_INF_SIZE;
        //logd("TotalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
    }

    spi_flash_read(pf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_head), RCD_ACCIDENT_INF_SIZE);
    _memcpy((u8 *)&time, pf, 6);
    //logd("%02d年%02d月%02d日%02d时%02d分%02d秒", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time); //时间校验
    if (ret) {
        TotalLen += 234;
        _nbintobcd(pf, 6); //时间转换
        _memcpy((u8 *)&cvt_point, (pf + 224), 10);
        _sw_endian((pf + 224), (u8 *)&cvt_point.locate.lngi, 4); //经纬度数据要进行大小端转换
        _sw_endian((pf + 228), (u8 *)&cvt_point.locate.lati, 4); //纬度
        _sw_endian((pf + 232), (u8 *)&cvt_point.heigh, 2);       //高度
        usb_rsp.len = TotalLen;
        return true;
    } else //如果时间错误
    {
        return false;
    }
}

/********************************************************
* 函 数 名: USB_get_driver_OT
* 函数功能: U盘获取超时驾驶记录
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
bool USB_get_driver_OT(void) {
    u8 i;
    u8 DataCode = 0x11; //数据代码
    u32 DataLen = 0;    //数据长度 = 数据包数 *
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u16 DataNum = 0;    //数据包数
    time_t time_start, time_end;
    u8 ret1, ret2;
    u8 *pf;
    S_RCD_POSINTION_INFOMATION cvt_point;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.OT_flag == true) {
        usb_rcd_pTmp.OT_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "超时驾驶记录");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //获取数据总包数
        DataNum = subp_rcd_p(usb_rcd_pTmp.OT_pHead, usb_rcd_pTmp.OT_pTail, RCD_OT_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_ACCIDENT_INF_SIZE;
        //logd("driver timeout datalen  = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //如果没有超时驾驶记录
            return true;
    }
    spi_flash_read(pf, RCDOT_ADDRD(usb_rcd_pTmp.OT_pHead), 50);
    _memcpy((u8 *)&time_start, (pf + 18), 6);
    //logd("start %2d年%2d月%2d日%2d时%2d分%2d秒", time_start.year, time_start.month, time_start.date, time_start.hour, time_start.min, time_start.sec);
    ret1 = _verify_time(&time_start);
    _memcpy((u8 *)&time_end, (pf + 24), 6);
    //logd("end %2d年%2d月%2d日%2d时%2d分%2d秒", time_end.year, time_end.month, time_end.date, time_end.hour, time_end.min, time_end.sec);
    ret2 = _verify_time(&time_end);
    if (ret1 && ret2) {
        _nbintobcd((pf + 18), 6); //超时驾驶开始时间转换
        _nbintobcd((pf + 24), 6); //超时驾驶结束时间转换

        _memcpy((u8 *)&cvt_point, (pf + 30), 10);

        _sw_endian((pf + 30), (u8 *)&cvt_point.locate.lati, 4); //经纬度数据要进行大小端转换
        _sw_endian((pf + 34), (u8 *)&cvt_point.locate.lngi, 4); //纬度
        _sw_endian((pf + 38), (u8 *)&cvt_point.heigh, 2);       //高度

        _memcpy((u8 *)&cvt_point, (pf + 40), 10);
        _sw_endian((pf + 40), (u8 *)&cvt_point.locate.lati, 4); //经纬度数据要进行大小端转换
        _sw_endian((pf + 44), (u8 *)&cvt_point.locate.lngi, 4); //纬度
        _sw_endian((pf + 48), (u8 *)&cvt_point.heigh, 2);       //高度

        TotalLen += 50;
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* 函 数 名: USB_get_driver_log
* 函数功能: U盘获取驾驶人身份记录
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
bool USB_get_driver_log(void) {
    u8 i;
    u8 DataCode = 0x12; //数据代码
    u32 DataLen = 0;    //数据长度 = 数据包数 *
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u16 DataNum = 0;    //数据包数
    time_t time;
    u8 ret;
    u8 *pf;
    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
    if (usb_rcd_pTmp.driver_log_flag == true) {
        usb_rcd_pTmp.driver_log_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "驾驶人身份记录");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        DataNum = subp_rcd_p(usb_rcd_pTmp.driver_log_pHead, usb_rcd_pTmp.driver_log_pTail, RCD_DRIVER_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_OT_INF_SIZE;
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //如果没有驾驶人信息记录
            return true;
    }

    spi_flash_read(pf, DRIVER_ADDRD(usb_rcd_pTmp.driver_log_pHead), 25);
    TotalLen += 25;
    _memcpy((u8 *)&time, pf, 6);
    //logd("%2d年%2d月%2d日%2d时%2d分%2d秒", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time);
    if (ret) {
        _nbintobcd(pf, 6);
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* 函 数 名: USB_get_power_log
* 函数功能: U盘获外部供电记录
* 入口参数: 无
* 返 回 值: true 数据正确 false 数据错误
* 日    期:  2017-12-15
*********************************************************/
bool USB_get_power_log(void) {
    u8 i;
    u8 DataCode = 0x13; //数据代码
    u32 DataLen = 0;    //数据长度 = 数据包数 *
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u16 DataNum = 0;    //数据包数
    time_t time;
    u8 ret;
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.power_flag == true) {
        usb_rcd_pTmp.power_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "外部供电记录");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //获取数据总包数
        DataNum = subp_rcd_p(usb_rcd_pTmp.power_log_pHead, usb_rcd_pTmp.power_log_pTail, RCD_POWER_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_POWER_INF_SIZE;
        logd("pwrTatalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
    }
    spi_flash_read(pf, POWER_ADDRD(usb_rcd_pTmp.power_log_pHead), 7);
    _memcpy((u8 *)&time, pf, 6);
    logd("%d-%d-%d %d:%d:%d", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time);
    if (ret) {
        TotalLen += 7;
        _nbintobcd(pf, 6); //时间转换
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* 函 数 名: USB_get_para_rcd
* 函数功能: U盘获取参数修改记录
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
bool USB_get_para_rcd(void) {
    u8 i;
    u8 DataCode = 0x14; //数据代码
    u32 DataLen = 0;    //数据长度 = 数据包数 *
    u16 TotalLen = 0;   //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u16 DataNum = 0;    //数据包数
    time_t time;
    u8 ret;
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
    if (usb_rcd_pTmp.para_change_flag == true) {
        usb_rcd_pTmp.para_change_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "参数修改记录");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //获取数据总包数
        DataNum = subp_rcd_p(usb_rcd_pTmp.para_change_pHead, usb_rcd_pTmp.para_change_pTail, RCD_PARA_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_PARA_INF_SIZE;
        logd("pwrTatalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //没有参数修改记录
            return true;
    }
    spi_flash_read(pf, _rcd_para_t_addr(usb_rcd_pTmp.para_change_pHead), 7);
    _memcpy((u8 *)&time, pf, 6);
    //logd("%d-%d-%d %d:%d:%d", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time);
    if (ret) {
        TotalLen += 7;
        _nbintobcd(pf, 6); //时间转换
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* 函 数 名: USB_get_speed_log
* 函数功能: U盘获取速度状态日志
* 入口参数: 无
* 返 回 值: 无
* 日    期:  2017-12-15
*********************************************************/
void USB_get_speed_log(void) {
    u8 i;
    u8 DataCode = 15; //数据代码
    u32 DataLen = 0;  //数据长度
    u16 TotalLen = 0; //数据总长度 = 数据代码  + 数据名称 + 数据长度 + 数据据
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = _bintobcd(DataCode);
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "速度状态日志");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //数据名称固定为18个字节,不足补0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    usb_rsp.len = TotalLen;
}

/****************************************************************************************************************
* 函 数 名: Hg_Tachographs_Head
* 函数功能: 海格行驶记录仪数据采集协议头
* 入口参数: pcs：分包数    			idx:当前索引    	cmd:命令字
* 返 回 值: 数据长度
* 日    期:  2022-01-22
*****************************************************************************************************************/
u16 Hg_Tachographs_Head(u16 pcs, u16 idx, u8 cmd) {
    u8 *pf = NULL;
    _memset(usb_rsp.rcdDataBuf, 0x00, sizeof(usb_rsp.rcdDataBuf));
    usb_rsp.len = 0;
    pf = usb_rsp.rcdDataBuf;

    /* 海格协议头*/
    //	pf[usb_rsp.len++] =  Allot.Data_Direction ;				//数据去向
    pf[usb_rsp.len++] = (u8)((pcs & 0xff00) >> 8); //分包总数 高字节
    pf[usb_rsp.len++] = (u8)(pcs & 0x00ff);        //分包总数 低字节
    pf[usb_rsp.len++] = (u8)((idx & 0xff00) >> 8); //分包序号 高字节
    pf[usb_rsp.len++] = (u8)(idx & 0x00ff);        //分包序号 低字节
    pf[usb_rsp.len++] = cmd;                       //命令字
    /* 19056协议头   */
    pf[usb_rsp.len++] = 0x55; //数据标识位
    pf[usb_rsp.len++] = 0x7a; //数据标识位
    pf[usb_rsp.len++] = cmd;  //命令字
    pf[usb_rsp.len++] = 0x00; //数据长度 高字节
    pf[usb_rsp.len++] = 126;  //数据长度 低字节
    pf[usb_rsp.len++] = 0x00; //保留位
    return usb_rsp.len;
}

#if 0	
/*************************************************************************************************
* 函数名称：tr9_Speed_derive(u8 *export_data)
* 函数功能：从flash中导出48小时数据
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.11
* 备     注：flash中只能存储50个小时的数据，3000分钟 	
***************************************************************************************************/

bool tr9_48H_Speed_derive(usb_read_rcd_struct *export_data)
{
	
	static enum E_LINK_MANAGE
    {
        E_48H_IDLE,
        E_48H_WRITE_NORMAL,
        E_48H_ERR
    } step = E_48H_IDLE;
		
	u8 i ,j;
	u8  packet = 5;
	u16 data_len   = 126 ;
	u16 packet_len =  630 ;
	static bool Special_state_8 = false ;


	
	if((rcd48_m.head.node == 0) || ( rcd48_m.head.node ==  rcd48_m.tail.node ) )
	return false ;	
	
	switch(step)	
	{
		case E_48H_IDLE:
			{
				if(rcd48_m.tail.node > rcd48_m.head.node)
				{
					usb_rcd_pTmp.rcd48_p_head = rcd48_m.head.node + (RCD48_P_SAVE_SIZE - rcd48_m.tail.node);
					usb_rcd_pTmp.rcd48_p_tail =  rcd48_m.tail.node ;
					Special_state_8 = true ;										// 代表数据又重头开始写了 
				}
				else
				{
					usb_rcd_pTmp.rcd48_p_head  = rcd48_m.head.node; 	
					usb_rcd_pTmp.rcd48_p_tail  = rcd48_m.tail.node; 
					Special_state_8 = false ;										// 正常 
				}
				
				Allot.idx = 0 ;									
				tr9_6033_task.state = 0 ;	
				tr9_6033_task.cmd = 0x08 ;	

				if(Special_state_8)
				{
					Allot.packet = ( RCD48_P_SAVE_SIZE - rcd48_m.tail.node ) / packet ;
					Allot.packet += ( rcd48_m.head.node / packet );
					
					if(((RCD48_P_SAVE_SIZE - rcd48_m.tail.node)% packet )!= 0)
						Allot.packet ++ ;
					if((rcd48_m.head.node % packet) != 0 )
						Allot.packet ++ ;	
				}
				else
				{
					Allot.packet = (usb_rcd_pTmp.rcd48_p_head / packet)  ; 	//获得总包数
					if((usb_rcd_pTmp.rcd48_p_head % packet)	!= 0 )
						Allot.packet += 1;	
				}

				step = E_48H_WRITE_NORMAL;
			}break ;	
					
		case E_48H_WRITE_NORMAL:
			{ 

				if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x08)) //应答标志
				{

					if( Special_state_8)
					{
						if(( RCD_ACCIDENT_P_SAVE_SIZE - usb_rcd_pTmp.rcd48_p_tail) >= packet)
						{
							spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), packet_len);		//一分行驶速度记录数据
							export_data->len = packet_len ;
							usb_rcd_pTmp.rcd48_p_tail += packet;
						}
						else
						{
							export_data->len = 0 ;
							j = ( RCD_ACCIDENT_P_SAVE_SIZE - usb_rcd_pTmp.rcd48_p_tail) ;
							
							for(i = 0 ;i < j ; i++)
							{
								spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), data_len);		//一分行驶速度记录数据
								export_data->len += data_len ;
								usb_rcd_pTmp.rcd48_p_tail += 1;
							}	
						}

						if( RCD_ACCIDENT_P_SAVE_SIZE == usb_rcd_pTmp.rcd48_p_tail )
						{
							Special_state_8 = false ;
							usb_rcd_pTmp.rcd48_p_tail = 0 ;
							usb_rcd_pTmp.rcd48_p_head  = rcd48_m.head.node ;
						}	
					}
					else
					{
						if(( usb_rcd_pTmp.rcd48_p_head - usb_rcd_pTmp.rcd48_p_tail) >= packet)
						{
							spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), packet_len);		//一分行驶速度记录数据
							export_data->len = packet_len ;
							usb_rcd_pTmp.rcd48_p_tail += packet;

						}
						else
						{
							export_data->len = 0 ;
							j = ( usb_rcd_pTmp.rcd48_p_head - usb_rcd_pTmp.rcd48_p_tail) ;
							
							for(i = 0 ;i < j ; i++)
							{
								spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), data_len);		//一分行驶速度记录数据
								export_data->len += data_len ;
								usb_rcd_pTmp.rcd48_p_tail += 1;
							}
						}
						if(usb_rcd_pTmp.rcd48_p_tail >= usb_rcd_pTmp.rcd48_p_head)
						{
							step = E_48H_IDLE ;
						}
					}
					tr9_6033_task.state = 1 ;
				}				
			}break ;
				
		case E_48H_ERR:
				step = E_48H_IDLE;
				break ;
		
		default:
	            step = E_48H_IDLE;
				break;		
	}
	return true ;
}

/**************************************************************************************************************************
* 函数名称：tr9_Location_derive(u8 *export_data)
* 函数功能：从flash中导出360小时位置信息
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.11
* 备     注：
***************************************************************************************************************************/

bool tr9_360H_Location_derive(usb_read_rcd_struct *export_data)
{
	static enum E_LINK_MANAGE
    {
		E_360H_IDLE,
		E_360H_WRITE_NORMAL,
    } step = E_360H_IDLE;
	static bool Special_state_9 = false;
		
	if((rcd360_m.head.node == 0) || ( rcd360_m.head.node ==  rcd360_m.tail.node ) )
		return false ;
	
	switch(step)
	{
		case E_360H_IDLE:
			{
				if(rcd360_m.tail.node > rcd360_m.head.node)
				{
					usb_rcd_pTmp.rcd360_p_head =  rcd360_m.head.node + (RCD360_P_SAVE_SIZE - rcd360_m.tail.node);
					usb_rcd_pTmp.rcd360_p_tail =  rcd360_m.tail.node;
					Special_state_9 = true ;										// 代表数据又重头开始写了 
				}
				else
				{
					usb_rcd_pTmp.rcd360_p_head	= rcd360_m.head.node;	//节索引，小时的个数
					usb_rcd_pTmp.rcd360_p_tail	= rcd360_m.tail.node;	
					Special_state_9 = false ;										// 正常 
				}

				Allot.idx = 0 ;
				tr9_6033_task.state = 0 ;
				tr9_6033_task.cmd = 0x09 ;				
				
				Allot.packet = (usb_rcd_pTmp.rcd360_p_head - usb_rcd_pTmp.rcd360_p_tail);	//获得总包数

				step = E_360H_WRITE_NORMAL;				
			}break ;	
					
		case E_360H_WRITE_NORMAL:
			{	
				if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x09)) //应答标志
				{

					if( Special_state_9)
					{
						if(( RCD360_P_SAVE_SIZE - usb_rcd_pTmp.rcd360_p_tail ) > 0)
						{
							spi_flash_read(export_data->rcdDataBuf, _rcd360_t_addr(usb_rcd_pTmp.rcd360_p_tail), 666); //1小时行驶速度记录数据
							export_data->len = 666 ;
							usb_rcd_pTmp.rcd360_p_tail += 1;
						}

						if( RCD360_P_SAVE_SIZE == usb_rcd_pTmp.rcd360_p_tail )
						{
							Special_state_9 = false ;
							usb_rcd_pTmp.rcd48_p_tail = 0 ;
							usb_rcd_pTmp.rcd360_p_head = rcd360_m.head.node ;
							
						}	
					}
					else
					{
						if(( usb_rcd_pTmp.rcd360_p_head  - usb_rcd_pTmp.rcd360_p_tail ) > 0)
						{
							spi_flash_read(export_data->rcdDataBuf, _rcd360_t_addr(usb_rcd_pTmp.rcd360_p_tail), 666); //1小时行驶速度记录数据
							export_data->len = 666 ;
							usb_rcd_pTmp.rcd360_p_tail += 1 ;
						}
											
						if(usb_rcd_pTmp.rcd360_p_tail == usb_rcd_pTmp.rcd360_p_head )
						{
							step = E_360H_IDLE ;
						}
					}

					tr9_6033_task.state = 1 ;			
					
				}
					
			}break ;
				
		default:
	            step = E_360H_IDLE;
				break;		
	}
	return true ;
}


/**************************************************************************************************************************
* 函数名称：tr9_get_accident(u8 *export_data)
* 函数功能：从flash中导事故疑点记录(10H)
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.11
* 备     注：
***************************************************************************************************************************/

bool tr9_get_accident(usb_read_rcd_struct *export_data)
{
	u8 i ,j ;
	static bool Special_state_10;
	static enum 
    {
		E_accident_IDLE,
		E_accident_WRITE_NORMAL,
	}step = E_accident_IDLE ;
		
	if((rcd_accident_m.head.node == 0) || ( rcd_accident_m.head.node ==  rcd_accident_m.tail.node ) )
		return false ;

	switch(step) 
		{
			case E_accident_IDLE:
				{
					if(rcd_accident_m.tail.node > rcd_accident_m.head.node)
					{
						usb_rcd_pTmp.rcd_accident_p_head = rcd_accident_m.head.node + (RCD_ACCIDENT_P_SAVE_SIZE - rcd_accident_m.tail.node);
						usb_rcd_pTmp.rcd_accident_p_tail =  rcd_accident_m.tail.node ;
						Special_state_10 = true ;										// 代表数据又重头开始写了 
					}
					else
					{
						usb_rcd_pTmp.rcd_accident_p_head = rcd_accident_m.head.node ;
						usb_rcd_pTmp.rcd_accident_p_tail = rcd_accident_m.tail.node ;
						Special_state_10 = false ;										// 正常 
					}	
					
					Allot.idx = 0 ;
					tr9_6033_task.state = 0 ;
					tr9_6033_task.cmd = 0x10 ;		

					if( Special_state_10 )
					{	
						Allot.packet = (RCD_ACCIDENT_P_SAVE_SIZE - rcd_accident_m.tail.node) / 2 ;
						Allot.packet += (rcd_accident_m.head.node / 2) ;
						if(((RCD_ACCIDENT_P_SAVE_SIZE - rcd_accident_m.tail.node) % 2) != 0)
							Allot.packet++ ;
						if(((rcd_accident_m.tail.node) %2) != 0 )		//不足一包数据
							Allot.packet++;
						
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.rcd_accident_p_head / 2) ;
						if((usb_rcd_pTmp.rcd_accident_p_head % 2) != 0 )		//不足一包数据
							Allot.packet++;
					}
				//	logd("事故头指针 = 0x%02X, 事故尾指针 = 0x%02X",rcd_accident_m.head.node,rcd_accident_m.tail.node);
				//	logd("事故总包数 = %d",Allot.packet,rcd_accident_m.tail.node);
					
				step = E_accident_WRITE_NORMAL ;
				}break ;

			case E_accident_WRITE_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x10)) 	//Rk接收成功
					{
						if( Special_state_10)
						{
							if(( RCD_ACCIDENT_P_SAVE_SIZE - usb_rcd_pTmp.rcd_accident_p_tail) >= 2)
							{
								spi_flash_read(export_data->rcdDataBuf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), 468);  //2个疑点一包数据
								export_data->len = 468 ;
								usb_rcd_pTmp.rcd_accident_p_tail += 2;
							}
							else
							{
								spi_flash_read(export_data->rcdDataBuf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), 234);  //1个疑点一包数据
								export_data->len = 234 ;
								usb_rcd_pTmp.rcd_accident_p_tail += 1;
							}

							if( RCD_ACCIDENT_P_SAVE_SIZE == usb_rcd_pTmp.rcd_accident_p_tail )
							{
								Special_state_10 = false ;
								usb_rcd_pTmp.rcd_accident_p_tail = 0 ;
							}	
						}
						else
						{
							if((usb_rcd_pTmp.rcd_accident_p_head - usb_rcd_pTmp.rcd_accident_p_tail) >= 2)
							{
								spi_flash_read(export_data->rcdDataBuf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), 468);  //1个疑点一包数据
								export_data->len = 468 ;
								usb_rcd_pTmp.rcd_accident_p_tail += 2;
							}
							else
							{
								export_data->len = 0 ;
								j = (usb_rcd_pTmp.rcd_accident_p_head - usb_rcd_pTmp.rcd_accident_p_tail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), RCD_ACCIDENT_INF_SIZE); //一个疑点234个字节
									export_data->len += 234 ;
									usb_rcd_pTmp.rcd_accident_p_tail += 1;
								}
								//Allot.idx++;
							}
							if(usb_rcd_pTmp.rcd_accident_p_tail == rcd_accident_m.head.node )			//数据完成
							{
								step = E_accident_IDLE ;
							}	
						}
					tr9_6033_task.state = 1;
					}
					if(Allot.packet == 0)
						step = E_accident_IDLE ;
				}break ;
			default:
					step = E_accident_IDLE;	
					break;
		}
	return true ;
}
/**************************************************************************************************************************
* 函数名称：tr9_get_driver_OT(u8 *export_data)
* 函数功能：从flash中导出超时驾驶记录(11H)
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.12
* 备     注：
***************************************************************************************************************************/

bool tr9_get_driver_OT(usb_read_rcd_struct *export_data)
{
	u8 i ,j ;
	
	static bool Special_state_11 = false ;
	static enum 
    {
		E_driver_IDLE,
		E_driver_WRITE_NORMAL,
	}step = E_driver_IDLE ;

	if((rcd_ot_m.head.node == 0) || ( rcd_ot_m.head.node ==  rcd_ot_m.tail.node ) )
		return false ;
	
	switch(step)
		{
			case E_driver_IDLE:
				{	
					if(rcd_ot_m.tail.node > rcd_ot_m.head.node)
					{
						usb_rcd_pTmp.OT_pHead = rcd_ot_m.head.node + (RCD_OT_P_SAVE_SIZE - rcd_ot_m.tail.node);
						usb_rcd_pTmp.OT_pTail =  rcd_ot_m.tail.node ;
						Special_state_11 = true ;										// 代表数据又重头开始写了 
					}
					else
					{
						usb_rcd_pTmp.OT_pHead = rcd_ot_m.head.node ;
						usb_rcd_pTmp.OT_pTail = rcd_ot_m.tail.node ;
						Special_state_11 = false ;										// 正常 
					}	
					
					Allot.idx = 0 ;
					tr9_6033_task.state = 0  ;
					tr9_6033_task.cmd = 0x11 ;
					if(Special_state_11)
					{
						Allot.packet  =	((RCD_OT_P_SAVE_SIZE - rcd_ot_m.tail.node)/15);
						Allot.packet += (rcd_ot_m.head.node / 15 ) ;					//15个超时记录组一包

						if((rcd_ot_m.head.node  %  15 ) != 0 )							//不足一包数据
							Allot.packet++ ;
						if(((RCD_OT_P_SAVE_SIZE - rcd_ot_m.tail.node)%15)!= 0)
							Allot.packet++ ;
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.OT_pHead / 15 ) ;					//15个超时记录组一包
						if((usb_rcd_pTmp.OT_pHead  %  15 ) != 0 )						//不足一包数据
							Allot.packet++ ;
					}
					

						
					step = E_driver_WRITE_NORMAL ;
					
				}break ;
			case E_driver_WRITE_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x11)) 	//Rk接收成功
					{
						if(Special_state_11)
						{		
							if( RCD_OT_P_SAVE_SIZE - usb_rcd_pTmp.OT_pTail >= 15 )
							{
								spi_flash_read(export_data->rcdDataBuf,RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 750);  //15个超时记录组一包
								export_data->len = 750 ;
								usb_rcd_pTmp.OT_pTail += 15 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( RCD_OT_P_SAVE_SIZE  - usb_rcd_pTmp.OT_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 50); //一个超时记录50个字节
									export_data->len += 50 ;
									usb_rcd_pTmp.OT_pTail += 1;
								}
							}

							if( RCD_OT_P_SAVE_SIZE == usb_rcd_pTmp.OT_pTail  )
							{
								Special_state_11 		= false ;
								usb_rcd_pTmp.OT_pTail 	=  0 ;
								usb_rcd_pTmp.OT_pHead   = rcd_ot_m.head.node ;
							}
						}
						else
						{
							if( usb_rcd_pTmp.OT_pHead  - usb_rcd_pTmp.OT_pTail >= 15 )
							{
								spi_flash_read(export_data->rcdDataBuf,RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 750);  //15个超时记录组一包
								export_data->len = 750 ;
								usb_rcd_pTmp.OT_pTail += 15 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( usb_rcd_pTmp.OT_pHead   - usb_rcd_pTmp.OT_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 50); //一个超时记录50个字节
									export_data->len += 50 ;
									usb_rcd_pTmp.OT_pTail += 1;
								}
							}
						if(usb_rcd_pTmp.OT_pTail >= usb_rcd_pTmp.OT_pHead )			//数据完成
							step = E_driver_IDLE ;	
						}
						tr9_6033_task.state = 1;
					}	
				}break;	
				
			default:
					step = E_driver_IDLE;	
					break;				
		}
	return true ;
}

/**************************************************************************************************************************
* 函数名称：tr9_get_driver_log(u8 *export_data)
* 函数功能：从flash中导出驾驶人身份记录(12H)
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.12
* 备     注：
***************************************************************************************************************************/

bool tr9_get_driver_log(usb_read_rcd_struct *export_data)
{
	u8 i ,j ;
	static bool Special_state_12 = false ;
	
	static enum 
    {
		E_driver_log_IDLE,
		E_driver_log_NORMAL,
	}step = E_driver_log_IDLE ;
		
	if((rcd_driver_m.head.node == 0) || ( rcd_driver_m.head.node ==  rcd_driver_m.tail.node ) )
			return false ;
	switch(step)
		{
			case E_driver_log_IDLE:
				{	

					if(rcd_driver_m.tail.node > rcd_driver_m.head.node)
					{
						usb_rcd_pTmp.driver_log_pHead = rcd_driver_m.head.node + (RCD_DRIVER_P_SAVE_SIZE - rcd_driver_m.tail.node);
						usb_rcd_pTmp.driver_log_pTail =  rcd_driver_m.tail.node ;
						Special_state_12 = true ;										// 代表数据又重头开始写了 
					}
					else
					{
						usb_rcd_pTmp.driver_log_pHead = rcd_driver_m.head.node ;
						usb_rcd_pTmp.driver_log_pTail = rcd_driver_m.tail.node ;
						Special_state_12 = false ;										// 正常 
					}	
					
					Allot.idx = 0 ;
					usb_rsp.len = 0  ;
					tr9_6033_task.state = 0 ;	
					tr9_6033_task.cmd == 0x12 ;

					if( Special_state_12 )
					{

						Allot.packet  = ( (RCD_DRIVER_P_SAVE_SIZE - rcd_driver_m.tail.node) / 30 );
						Allot.packet += (rcd_driver_m.head.node / 30)  ;		//30个驾驶员身份记录组一包
						if((rcd_driver_m.head.node % 30 ) != 0 ) 			//不足一包数据
							Allot.packet++; 
						if(((RCD_DRIVER_P_SAVE_SIZE - rcd_driver_m.tail.node) % 30) != 0)
							Allot.packet++; 
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.driver_log_pHead / 30)  ;		//30个驾驶员身份记录组一包
						if((usb_rcd_pTmp.driver_log_pHead % 30 ) != 0 )				//不足一包数据
							Allot.packet++;						
					}
					step = E_driver_log_NORMAL ;
				}break ;
			case E_driver_log_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x12)) 	//Rk接收成功
					{					

						if(Special_state_12)
						{
							if( RCD_DRIVER_P_SAVE_SIZE - usb_rcd_pTmp.driver_log_pTail >= 30)
							{
								spi_flash_read(export_data->rcdDataBuf,DRIVER_ADDRD(usb_rcd_pTmp.driver_log_pTail), 750);  //30个驾驶员身份记录组一包
								export_data->len = 750 ;
								usb_rcd_pTmp.driver_log_pTail += 30 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( RCD_DRIVER_P_SAVE_SIZE - usb_rcd_pTmp.driver_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.driver_log_pTail), 25); //一个驾驶员身份记录25个字节
									export_data->len += 25 ;
									usb_rcd_pTmp.driver_log_pTail += 1;
								}
							}
							
							if(RCD_DRIVER_P_SAVE_SIZE == usb_rcd_pTmp.driver_log_pTail )
							{
								Special_state_12 = false ;
								usb_rcd_pTmp.driver_log_pTail = 0 ;
							}
						}
						else
						{
							if( rcd_driver_m.head.node - usb_rcd_pTmp.driver_log_pTail >= 30)
							{
								spi_flash_read(export_data->rcdDataBuf,DRIVER_ADDRD(usb_rcd_pTmp.driver_log_pTail), 750);  //30个驾驶员身份记录组一包
								export_data->len = 750 ;
								usb_rcd_pTmp.driver_log_pTail += 30 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( rcd_driver_m.head.node - usb_rcd_pTmp.driver_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.driver_log_pTail), 25); //一个驾驶员身份记录25个字节
									export_data->len += 25 ;
									usb_rcd_pTmp.driver_log_pTail += 1;
								}
							}
							if(usb_rcd_pTmp.driver_log_pTail >= usb_rcd_pTmp.driver_log_pHead)			//数据完成
								step = E_driver_log_IDLE ;	
						}
						
						tr9_6033_task.state = 1 ; 
					}
				}break;	
				
			default:
					step = E_driver_log_IDLE;	
					break;				
		}
	return true ;	

}

/**************************************************************************************************************************
* 函数名称：tr9_get_power_log(u8 *export_data)
* 函数功能：从flash中导出记录仪外部供电记录(13H)
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.12
* 备     注：
***************************************************************************************************************************/
bool tr9_get_power_log(usb_read_rcd_struct *export_data)
{
	u8 i ,j ;
	static bool Special_state_13 = false ;	
	static enum 
    {
		E_power_log_IDLE,
		E_power_log_NORMAL,
	}step = E_power_log_IDLE ;

	if((rcd_power_m.head.node == 0) || ( rcd_power_m.head.node ==  rcd_power_m.tail.node ) )
		return false ;

	switch(step)
		{
			case E_power_log_IDLE:
				{	

				
					if(rcd_power_m.tail.node > rcd_power_m.head.node)
					{
						usb_rcd_pTmp.power_log_pHead =  rcd_power_m.head.node + (RCD_POWER_P_SAVE_SIZE - rcd_power_m.tail.node);
						usb_rcd_pTmp.power_log_pTail =  rcd_power_m.tail.node ;
						Special_state_13 = true ;										// 代表数据又重头开始写了 
					}
					else
					{
						usb_rcd_pTmp.power_log_pHead = rcd_power_m.head.node ;
						usb_rcd_pTmp.power_log_pTail = rcd_power_m.tail.node ;
						Special_state_13 = false ;										// 正常 
					}	
					
					
					Allot.idx = 0 ;
					tr9_6033_task.state = 0 ;
					tr9_6033_task.cmd = 0x13;
					if(Special_state_13)
					{
						Allot.packet  = ((RCD_POWER_P_SAVE_SIZE - rcd_power_m.tail.node) / 50); 
						Allot.packet += (usb_rcd_pTmp.power_log_pHead / 50 );		//100个记录仪外部供电组一包
						if((usb_rcd_pTmp.power_log_pHead % 50 ) != 0 )			//不足一包数据
							Allot.packet++;
						if(((RCD_POWER_P_SAVE_SIZE - rcd_power_m.tail.node) / 50) != 0)
							Allot.packet++;		
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.power_log_pHead / 50 );		//100个记录仪外部供电组一包
						if((usb_rcd_pTmp.power_log_pHead % 50 ) != 0 )			//不足一包数据
							Allot.packet++;

					}
					
					step = E_power_log_NORMAL ;
				}break ;
			case E_power_log_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x13)) 	//Rk接收成功
					{

						if( Special_state_13 )
						{
							if( RCD_POWER_P_SAVE_SIZE - usb_rcd_pTmp.power_log_pTail >= 50)
							{		
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 350);  //100个记录仪外部供电组一包						
								export_data->len = 350 ;
								usb_rcd_pTmp.power_log_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (RCD_POWER_P_SAVE_SIZE - usb_rcd_pTmp.power_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 7); //一个驾驶员身份记录25个字节
									export_data->len += 7 ;
									usb_rcd_pTmp.power_log_pTail += 1;
								}
							}

							if( RCD_POWER_P_SAVE_SIZE == usb_rcd_pTmp.power_log_pTail )
							{
								Special_state_13 = false ;
								usb_rcd_pTmp.power_log_pTail = 0 ;
								usb_rcd_pTmp.power_log_pHead = rcd_power_m.head.node  ;
							}

						}
						else
						{
							if( usb_rcd_pTmp.power_log_pHead - usb_rcd_pTmp.power_log_pTail >= 50)
							{		
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 350);  //50个记录仪外部供电组一包
								export_data->len = 350 ;
								usb_rcd_pTmp.power_log_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (usb_rcd_pTmp.power_log_pHead  - usb_rcd_pTmp.power_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 7); //一个驾驶员身份记录25个字节
									export_data->len += 7 ;
									usb_rcd_pTmp.power_log_pTail += 1;
								}	
							}

							if(usb_rcd_pTmp.power_log_pTail >= usb_rcd_pTmp.power_log_pHead )			//数据完成
								step = E_power_log_IDLE ;

						}
						tr9_6033_task.state = 1 ;
					}
					
				}break;	
				
			default:
					step = E_power_log_IDLE;	
					break;				
		}
	return true ;	

}




/**************************************************************************************************************************
* 函数名称：tr9_get_para_rcd(u8 *export_data)
* 函数功能：从flash中导出记录仪参数修改记录(14H)
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.12
* 备     注：
***************************************************************************************************************************/
bool tr9_get_para_rcd(usb_read_rcd_struct *export_data)
{
	u8 i ,j ;
	static bool Special_state_14 = false ;
	static enum 
    {
		E_para_IDLE,
		E_para_NORMAL,
	}step = E_para_IDLE ;
		
	if((rcd_para_m.head.node == 0) || ( rcd_para_m.head.node ==  rcd_para_m.tail.node ) )
		return false ;	
	switch(step)
		{
			case E_para_IDLE:
				{
					if(rcd_para_m.tail.node > rcd_para_m.head.node)
					{
						usb_rcd_pTmp.para_change_pHead = rcd_para_m.head.node + (RCD_PARA_P_SAVE_SIZE - rcd_para_m.tail.node);
						usb_rcd_pTmp.para_change_pTail =  rcd_para_m.tail.node ;
						Special_state_14 = true ;										// 代表数据又重头开始写了 
					}
					else
					{
						usb_rcd_pTmp.para_change_pHead = rcd_para_m.head.node ;
						usb_rcd_pTmp.para_change_pTail = rcd_para_m.tail.node ;
						Special_state_14 = false ;										// 正常 
					}	
					
					Allot.idx = 0 ;
					Allot.packet = 0 ;
					tr9_6033_task.state = 0 ;
					tr9_6033_task.cmd = 0x14 ;
			
					if( Special_state_14 )
					{
						Allot.packet  = ((RCD_PARA_P_SAVE_SIZE - rcd_para_m.tail.node)/50);
						Allot.packet += (rcd_para_m.head.node / 50) ;		//50个记录仪外部供电组一包
						if((rcd_para_m.head.node % 50 ) != 0 )			//不足一包数据
							Allot.packet++;
						if(((RCD_PARA_P_SAVE_SIZE - rcd_para_m.tail.node) % 50 ) != 0)
							Allot.packet++;
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.para_change_pHead / 50) ;		//50个记录仪外部供电组一包
						if((usb_rcd_pTmp.para_change_pHead % 50 ) != 0 )			//不足一包数据
							Allot.packet++;
					}

					
					step = E_para_NORMAL ;
				}break ;
			case E_para_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x14)) 	//Rk接收成功
					{
						if( Special_state_14 )
						{	
							if((RCD_PARA_P_SAVE_SIZE  - usb_rcd_pTmp.para_change_pTail) >= 50)
							{	
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 350);  //50个记录仪外部供电组一包	
								export_data->len = 350 ;
								usb_rcd_pTmp.para_change_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( RCD_PARA_P_SAVE_SIZE  - usb_rcd_pTmp.para_change_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 7); //一个供电记录25个字节
									export_data->len += 7 ;
									usb_rcd_pTmp.para_change_pTail += 1;
								}
							}
							if(RCD_PARA_P_SAVE_SIZE == usb_rcd_pTmp.para_change_pTail )
							{
								Special_state_14 = false ;
								usb_rcd_pTmp.para_change_pTail = 0 ;
								usb_rcd_pTmp.para_change_pHead = rcd_para_m.head.node ;
							}
						}
						else
						{
							if((usb_rcd_pTmp.para_change_pHead - usb_rcd_pTmp.para_change_pTail ) >= 50)
							{	
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 350);  //100个记录仪外部供电组一包
								export_data->len = 350 ;
								usb_rcd_pTmp.para_change_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (usb_rcd_pTmp.para_change_pHead - usb_rcd_pTmp.para_change_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 7); //一个供电记录25个字节
									export_data->len += 7 ;
									usb_rcd_pTmp.para_change_pTail += 1;
								}
							
							}
							if(usb_rcd_pTmp.para_change_pTail >= usb_rcd_pTmp.para_change_pHead)			//数据完成
								step = E_para_IDLE ;
						}
						
						tr9_6033_task.state = 1;
					}
				}break;	
				
			default:
					step = E_para_IDLE;	
					break;				
		}
	return true ;	

}

/**************************************************************************************************************************
* 函数名称：tr9_get_para_rcd(u8 *export_data)
* 函数功能：从flash中导出记录仪参数修改记录(15H)
* 函数参数：export_data做出口参数使用，把参数传出去
* 时     间:	2022.2.12
* 备     注：
***************************************************************************************************************************/
bool tr9_get_spped_rcd(usb_read_rcd_struct *export_data)
{

	u8 i ,j ;
	static bool Special_state_15 = false ;
	static enum 
	{
		E_SPEED_IDLE,
		E_SPEED_NORMAL,
	}step = E_SPEED_IDLE ;

	if((rcd_speed_m.head.node == 0) || ( rcd_speed_m.head.node ==  rcd_speed_m.tail.node ) )
		return false ;

	switch(step)
		{
			case E_SPEED_IDLE:
				{	
					if(rcd_speed_m.tail.node > rcd_speed_m.head.node)
					{
						usb_rcd_pTmp.speed_log_pHead = rcd_speed_m.head.node + (RCD_SPEED_P_SAVE_SIZE - rcd_speed_m.tail.node);
						usb_rcd_pTmp.speed_log_pTail =  rcd_speed_m.tail.node ;
						Special_state_15 = true ;										// 代表数据又重头开始写了 
					}
					else
					{
						usb_rcd_pTmp.speed_log_pHead = rcd_speed_m.head.node ;
						usb_rcd_pTmp.speed_log_pTail = rcd_speed_m.tail.node ;
						Special_state_15 = false ;										// 正常 
					}	

					
					Allot.idx = 0 ;	
					tr9_6033_task.state = 0  ;
					tr9_6033_task.cmd = 0x15 ;
					if( Special_state_15 )
					{

						Allot.packet  = (RCD_SPEED_P_SAVE_SIZE - rcd_speed_m.tail.node) / 5 ;
						Allot.packet += (usb_rcd_pTmp.speed_log_pHead / 5)      ;//100个记录仪外部供电组一包
						if((usb_rcd_pTmp.speed_log_pHead % 5 ) != 0 )			//不足一包数据
							Allot.packet++;
						if(((RCD_SPEED_P_SAVE_SIZE - rcd_speed_m.tail.node) % 5 ) != 0)
							Allot.packet++;
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.speed_log_pHead / 5)      ;//100个记录仪外部供电组一包
						if((usb_rcd_pTmp.speed_log_pHead % 5 ) != 0 )			//不足一包数据
							Allot.packet++;

					}

					step = E_SPEED_NORMAL ;
				}break ;
			case E_SPEED_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x15)) 	//Rk接收成功
					{

						if( Special_state_15 )
						{
							if( RCD_SPEED_P_SAVE_SIZE - usb_rcd_pTmp.speed_log_pTail >= 5)
							{	
								spi_flash_read(export_data->rcdDataBuf,RSPEED_ADDRD(usb_rcd_pTmp.speed_log_pTail), 665);  //665个字节组一包
								export_data->len = 665 ;
								usb_rcd_pTmp.speed_log_pTail += 5 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (RCD_SPEED_P_SAVE_SIZE - usb_rcd_pTmp.speed_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RSPEED_ADDRD(usb_rcd_pTmp.speed_log_pTail), 133); 
									export_data->len += 133 ;
									usb_rcd_pTmp.speed_log_pTail += 1;
								}
							}

							if(RCD_SPEED_P_SAVE_SIZE == usb_rcd_pTmp.speed_log_pTail)
							{
								Special_state_15 = false ;
								usb_rcd_pTmp.speed_log_pTail = 0 ;
								usb_rcd_pTmp.speed_log_pHead = rcd_speed_m.head.node ;
							}
						}
						else
						{
							if(usb_rcd_pTmp.speed_log_pHead - usb_rcd_pTmp.speed_log_pTail >= 5)
							{
								
								spi_flash_read(export_data->rcdDataBuf,RSPEED_ADDRD(usb_rcd_pTmp.speed_log_pTail), 665);  //798个字节组一包
								export_data->len = 665 ;
								usb_rcd_pTmp.speed_log_pTail += 5 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (usb_rcd_pTmp.speed_log_pHead - usb_rcd_pTmp.speed_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RSPEED_ADDRD(usb_rcd_pTmp.speed_log_pTail), 133); 
									export_data->len += 133 ;
									usb_rcd_pTmp.speed_log_pTail += 1;
								}
							}
							
							if(usb_rcd_pTmp.speed_log_pTail == usb_rcd_pTmp.speed_log_pHead)			//数据完成
								step = E_SPEED_IDLE ;
						}

						tr9_6033_task.state = 1;
					}
	
				}break; 
				
			default:
					step = E_SPEED_IDLE; 
				break;				
	}
	return true ;	

}


void tr9_Driving_Record_update_Rk(u8 command,u8 dion)
{



	u8 i   = 0 ;
	u8 ret = 0 ;
	u8 *pa = NULL ;
	
	bool no_record = false;
	static bool Export_single = false  ; 	//导出单个的全部数据
	static u8 cmd  = 0 , cmd_bak = 0 ;
	static u8 id =  0 ;
	static u16 timer_send = 0 ;
	//data_package_t  send_buf ;
	
	
	/**********************导出单个指令的全部数据*************************/
	if(2 == dion)
	{
		id  = command;
		cmd_bak = id    ;
		Ack_Export 			= true ;
		tr9_show 			= false;	
		Export_single 		= true ;
		Data_Direction = 0 ;
		tr9_6033_task.state = 0 ;
		tr9_6033_task.cmd	= command ;	
		Rk_Task_Manage.RK_1717_state = true ;
	}
	
	if (Export_single)							
	{
		//if(cmd != tr9_6033_task.cmd)
		
		if(id != cmd_bak)
		{
			//Allot.idx++ ;
			//tr9_6033_task.state = 0 ;
			
			id = 0 ;
			cmd_bak = 0 ;
			usb_rsp.len = 0 ;
			tr9_show   = true ;	
			Ack_Export = false ;
			Export_single = false;
			Rk_Task_Manage.RK_1717_state = false;
			_memset(usb_rsp.rcdDataBuf, 0x00,sizeof(usb_read_rcd_struct));
			
		}
	}
	/*************************************************/

	if(tr9_6033_task.state != 0)
		return ;
			
	timer_send = (u16)tick ;
		
	if(Ack_Export == true)
	{
		switch(id)
		{
			case Cmd_0x00H:   	
					Rk_Task_Manage.RK_1717_state = true;	//暂时关闭1717的发送
					//tr9_show = false  ;						//关闭串口三的数据显示
			case Cmd_0x01H:   ;
			case Cmd_0x02H:   ;
			case Cmd_0x03H:   ;
			case Cmd_0x04H:   ;
			case Cmd_0x05H:   ;	
			case Cmd_0x06H:   ;
			case Cmd_0x07H:
				{
					Allot.idx = 0 ; 				//当前包
					Allot.packet = 1 ;				//总包数
					rcd_communicate_parse(FROM_U4,id,&command,1);//2012行驶记录仪协议 	
					if(0 == tr9_6033_task.state)
						id ++ ;
					tr9_6033_task.state = 1 ;
				}break;

			case Cmd_0x08H:	
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_48H_Speed_derive(&usb_rsp);//08速度采集
					if(false == ret)
					{
						no_record 	 = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;	
						logd("0x08 无48小时数据");
					}
					
				}break;
			case Cmd_0x09H:  
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_360H_Location_derive(&usb_rsp); 			 //09位置采集 
					if(false == ret)
					{
						no_record 	 = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;	
						logd("0x09 无360小时位置数据");
					}
				}break ;
			case Cmd_0x10H:  
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					logd("进入指令10");
					ret = tr9_get_accident(&usb_rsp); 
					if(false == ret)
					{
						no_record 	 = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;
						logd("0x10 未发生事故");
					}
						
				}	break ;
			case Cmd_0x11H: 
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_driver_OT(&usb_rsp);
					if(false == ret)
					{
						no_record 	 = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;
						logd("0x11 司机未超时");
					}
					
				}break ;
			case Cmd_0x12H:  
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_driver_log(&usb_rsp);
					if(false == ret)
					{
						no_record  = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;
						logd("0x12 无驾驶人身份记录");
					}

				}break;	
			case Cmd_0x13H:
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_power_log(&usb_rsp);
					if(false == ret)
					{
						no_record 	 = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;
						logd("0x13 无记录仪外部供电记录");
					}
				}break;		
			case Cmd_0x14H: 
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_para_rcd(&usb_rsp);			//
					if(false == ret)
					{
						no_record 	 = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;
						logd("0x14 无记录仪参数修改记录");
					}
				}break;	
			case Cmd_0x15H: 
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_spped_rcd(&usb_rsp);			//速度差异采集
					if(false == ret)
					{
						no_record 	 = true;			//无数据执行下一条指令
						Allot.idx	 = 0 ; 				//当前包
						Allot.packet = 1 ;				//总包数
						usb_rsp.len  = 0 ;
						logd("0x15 无速度状态日志");
				    }		
				}break;		
			default :
				{
					id = 0 ;
					Allot.idx  = 0 ;
					Allot.packet = 0 ;
					usb_rsp.len  = 0 ;
					Ack_Export = false ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);

				}break ;
				
				
		}
		if( id >= 8)
		{
			if( (usb_rsp.len != 0))
			{

				rcd_send_data(FROM_U4,RCD_C_OK,id,usb_rsp.rcdDataBuf,usb_rsp.len);  //发送有效数据
				if(Allot.idx  ==  (Allot.packet - 1))
				{
					if(id >= 16)
					{
						id ++ ;
					}
					else
					{
						if(id >= 9)
							id+= 7 ;
						else
							id ++;
					}
				
					Allot.idx = 0 ;
					Allot.packet = 0 ;
				}
				else if(Allot.idx > (Allot.packet))
				{

					Allot.idx  = 0 ;
					Allot.packet = 0 ;
					id = 0 ;
					logd("错误:索引大于总包数");
				}
			}
			
			if(no_record)
			{
				no_record = false ;

				rcd_send_data(FROM_U4,RCD_C_OK,id,usb_rsp.rcdDataBuf,usb_rsp.len);  //发送有效数据

					if(id >= 16)
					{
						id ++ ;
					}
					else
					{
						if(id >= 9)
							id+= 7 ;
						else
							id ++;
					}
				
				Allot.idx = 0 ;
				Allot.packet = 0 ;
				usb_rsp.len = 0  ;
				_memset(usb_rsp.rcdDataBuf, 0x00,sizeof(usb_read_rcd_struct));
				
			}
				
		}
		
		if(id > 21)
		{
			id = 0 ;
			Allot.idx = 0 ;
			usb_rsp.len = 0  ;
			Allot.packet = 0 ;
			tr9_show = true ;				//在调试状态显示串口三的数据	
			Ack_Export = false ;
 			Export_finish  = 1;				//导出结束
 			tr9_6033_task.state = 0 ;
 			Rk_Task_Manage.RK_1717_state = false ;
			Rk_Task_Manage.RK_6051_state = false ;
			
			
			_memset(usb_rsp.rcdDataBuf, 0x00,sizeof(usb_read_rcd_struct));
 			logi("导出完成");
		}
	}


}
#endif

/*2022-03-17 add by hj  
重点修改的函数如下:
   1. tr9_Driving_record();
   2. tr9_Export_state();
   3. tr9_6051_task();
   4. rcd_send_data();
   5. menu_process.c 文件中 增加 extern rk_6033_task_t  rk_6033_task; 定义
               同时 在按下确认键 后 rk_6033_task_t.u_read_flag =2;
   6. 增加 tr9_6066_task 任务协议,为测试来清除行驶记录数据            
   6. 在 main()函数中,增加一个50ms 的任务,来调度u 盘传输程序。            
   7. 修改部分 可通过 2022-03-17 来搜索
   
行驶记录仪数据传输需要注意一下几点
   1. 本文件主要是应对 过检要求
   2. 过检处理, 再下发读取行驶记录仪数据时,不会下发 开始时间和结束时间
   3. 行驶记录溢的数据是通过RK 上传的, 故USB读取数据和ACCON网络读取数据时,都是通过RK来中转
      故必须满足： 6033协议要求： 增加了一个当前包数和总包数 
   4. 由于行驶记录仪保存时的时间为16进制的时间参数,形成19056数据时,时间参数必须是转换成BCD码。
   5. 如果读取行驶记录仪的指令没有带时间参数时,MCU默认是传递所有的数据, 
   6. 重点了解 subp_rcd_p() 、此函数。   
*/

void Rk_Driving_Record_Up(u8 cmd) {
    u8 buf_t_1[RCD_COM_MAX_SIZE];
    u8 msg_t_1[RCD_COM_MAX_SIZE];
    /*step.1 如果没有收到 6051 并按下确认键 则不进行处理*/
    /*       由于LED的确认键是与其它公用, 故在确认键处理部分,需要对 tr9_6033_task.u_read_flag = 1*/
    /*       进行判断; 如果按下确认键时,tr9_6033_task.u_read_flag = 1 ,则置该变量=2*/
    /*       rk_6033_task.u_read_flag = 0x10 为单独读取 CMD的标志 */
    //if((rk_6033_task.u_read_flag != 0x02)&&(rk_6033_task.u_read_flag != 0x10))
    //	return;
    if (rk_6033_task.u_read_flag != 0x02) {     //读所有数据
        if (rk_6033_task.u_read_flag != 0x10) { //读单个CMD
            return;
        }
    }

/*step.2 如果发送失败 或 超时无应答, 则重传*/
/*这里是处理未应答或接收错误的应答 进行重发的处理 最大延迟为250ms*/
/*需要注意: 这里如果 没有收到正确应答,是不会往下走的, 因为u_send_flag置位后,只在正确应答部分才清零*/
#if 1
    //if(rk_6033_task.u_send_flag && ((jiffies - rk_6033_task.u8_delay )>250))//2022-03-31 modify
    //if(rk_6033_task.u_send_flag && ((jiffies - rk_6033_task.u8_delay )>1000))
    if (rk_6033_task.u_send_flag) {
        {                                                 //重发：usb_rsp 这个结构体是全局变量,只有发送成功或发送完成后才清零。
            if (Rk_Task_Manage.RK_nack_only_delay == 1) { //无需应答, 向下
                rk_6033_task.u_send_flag = 0;
                if (Rk_Task_Manage.RK_read_all == 0)
                //if(rk_6033_task.u_read_flag == 0x10)
                { //读单条
                    if (cmd >= 0x08) {
                        if (rk_6033_task.idx >= (rk_6033_task.packet - 1)) { //如果分包个数等于 总包个数 ,则启动下一个cmd的读取
                            {                                                //所有数据传输完成, 并收到正确应答
                                tr9_show = true;                             //在调试状态显示串口三的数据
                                Ack_Export = false;
                                Export_finish = 1; //导出结束
                                tr9_6033_task.state = 0;
                                Rk_Task_Manage.RK_1717_state = false;
                                Rk_Task_Manage.RK_6051_state = false;
                                Rk_Task_Manage.RK_record_print_log = false;
                                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                                Rk_Task_Manage.RK_nack_only_delay = 0;
                                Rk_Task_Manage.RK_time_rang_NG_flag = 0;
                                return;
                            }

                        } else { //进入下一个分包的处理
                            rk_6033_task.idx++;
                            rk_6033_task.u_send_flag = 0;
                            rk_6033_task.u8_delay = 0;
                        }
                    } else {
                        tr9_show = true; //在调试状态显示串口三的数据
                        Ack_Export = false;
                        Export_finish = 1; //导出结束
                        tr9_6033_task.state = 0;
                        Rk_Task_Manage.RK_1717_state = false;
                        Rk_Task_Manage.RK_6051_state = false;
                        Rk_Task_Manage.RK_record_print_log = false;
                        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                        Rk_Task_Manage.RK_nack_only_delay = 0;
                        Rk_Task_Manage.RK_time_rang_NG_flag = 0;
                        return;
                    }
                } else { //读所有
                    if (cmd >= 0x08) {
                        if (rk_6033_task.idx >= (rk_6033_task.packet - 1)) { //如果分包个数等于 总包个数 ,则启动下一个cmd的读取
                            {                                                //所有数据传输完成, 并收到正确应答

                                if (cmd == 0x15) {
                                    tr9_show = true; //在调试状态显示串口三的数据
                                    Ack_Export = false;
                                    Export_finish = 1; //导出结束
                                    tr9_6033_task.state = 0;
                                    Rk_Task_Manage.RK_1717_state = false;
                                    Rk_Task_Manage.RK_6051_state = false;
                                    Rk_Task_Manage.RK_record_print_log = false;
                                    _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                                    Rk_Task_Manage.RK_nack_only_delay = 0;
                                    Rk_Task_Manage.RK_time_rang_NG_flag = 0;
                                    return;
                                }

                                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                                if (cmd == 0x09)
                                    rk_6033_task.cmd = 0x10;
                                else {
                                    rk_6033_task.cmd = cmd;
                                    rk_6033_task.cmd++;
                                }

                                rk_6033_task.u_read_flag = 0x10; // 读单挑
                                rk_6033_task.Data_Direction = 1; //数据方向
                                //Rk_Task_Manage.RK_nack_only_delay = 1;//设置无需RK应答标志
                                Rk_Task_Manage.RK_time_rang_NG_flag = 1;
                                Rk_Task_Manage.RK_read_all = 1;
                                rk_6033_task.u_send_flag = 0;
                                rk_6033_task.u8_delay = 0;
                                //tr9_show = true  ;
                                return;
                            }

                        } else { //进入下一个分包的处理
                            rk_6033_task.idx++;
                            rk_6033_task.u_send_flag = 0;
                            rk_6033_task.u8_delay = 0;
                        }
                    } else {
                        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                        rk_6033_task.cmd = cmd; //初始化 cmd = 0x00等待 “确认”键按下。
                        rk_6033_task.cmd++;
                        rk_6033_task.u_read_flag = 0x10;       // 读单挑
                        rk_6033_task.Data_Direction = 1;       //数据方向
                        Rk_Task_Manage.RK_nack_only_delay = 1; //设置无需RK应答标志
                        Rk_Task_Manage.RK_time_rang_NG_flag = 1;
                        Rk_Task_Manage.RK_read_all = 1;
                        return;
                    }
                }
            } else {                                            //需要应答
                if ((jiffies - rk_6033_task.u8_delay) > 2500) { //重发
                    rk_6033_task.u_send_flag = 1;
                    rk_6033_task.u8_delay = jiffies;
                    //rcd_send_data(FROM_U4, RCD_C_OK, rsp.id, usb_rsp.rcdDataBuf, (u16)usb_rsp.len); //2022-03-31 modify by hj
                    rcd_send_data(FROM_U4, RCD_C_OK, rk_6033_task.cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);
                }
                return;
            }
        }
    }
#endif

    /*step.4 组织相关需发送的数据, 注意: 发送的 cmd 是在收到 6033的正确应答后 才进行下一个cmd的处理*/
    switch (cmd) {
    case 0x00:                               //采集记录仪执行标准版本
    case 0x01:                               //采集当前驾驶人信息
    case 0x02:                               //采集记录仪实时时间
    case 0x03:                               //采集累计行驶里程
    case 0x04:                               //采集记录仪脉冲系数
    case 0x05:                               //采集车辆信息
    case 0x06:                               //采集记录仪状态信号配置信息
    case 0x07:                               //采集记录仪唯一性编号
    {                                        // 注意这里是沿用 原来的处理
        Rk_Task_Manage.RK_1717_state = true; //暂时关闭1717的发送
        tr9_show = false;                    //关闭串口三的数据显示
        //Rk_Task_Manage.RK_nack_only_delay = 1;
        if (rk_6033_task.u_read_flag != 0x02)
            rk_6033_task.u_read_flag = 0x10;
        rk_6033_task.u_send_flag = 1;
        rk_6033_task.u8_delay = jiffies;
        rk_6033_task.idx = 0;                         //当前包
        rk_6033_task.packet = 1;                      //总包数
        rcd_communicate_parse(FROM_U4, cmd, &cmd, 1); //2012行驶记录仪协议
    } break;

    case 0x08: //48小时 rcd48_m  T_RCD_48
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd48_m.head.node == rcd48_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;

                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: rk_6033_task.frame=800/126 = 6个节点 */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            rk_6033_task.frame = 4;     //2022-03-31 modify by hj
            //rk_6033_task.frame = 3;
            //get 节点个数
            index_t = subp_rcd_p(rcd48_m.head.node, rcd48_m.tail.node, RCD48_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                  //分包初始包从0开始
            rk_6033_task.head = rcd48_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd48_m.tail.node;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_48;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            rk_6033_task.head = decp_rcd_p(T_RCD_48, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        /*                           这个rsp.id 在程序相关部分中未使用*/
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;

    case 0x09: //采集指定的位置信息记录  360小时 数据采集 rcd360_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd360_m.head.node == rcd360_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd = rk_6033_task.cmd + 7;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: 800/666 = 1个节点                    */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            rk_6033_task.frame = 1;
            //get 节点个数
            index_t = subp_rcd_p(rcd360_m.head.node, rcd360_m.tail.node, RCD360_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                   //分包初始包从0开始
            rk_6033_task.head = rcd360_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd360_m.tail.node;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_360;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            rk_6033_task.head = decp_rcd_p(T_RCD_360, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x10: //采集指定的事故疑点记录  T_RCD_ACCIDENT rcd_accident_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd_accident_m.head.node == rcd_accident_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: 800/234 = 3个节点 不能占满800字节    */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            rk_6033_task.frame = 2;     //2022-03-31 modify by hj
            //rk_6033_task.frame = 2;
            //get 节点个数
            index_t = subp_rcd_p(rcd_accident_m.head.node, rcd_accident_m.tail.node, RCD_ACCIDENT_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                         //分包初始包从0开始
            rk_6033_task.head = rcd_accident_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd_accident_m.tail.node;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_ACCIDENT;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            rk_6033_task.head = decp_rcd_p(T_RCD_ACCIDENT, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x11: //采集指定的超时驾驶记录 T_RCD_DRIVER_OT   rcd_ot_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd_ot_m.head.node == rcd_ot_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: 800/50 = 15个节点 不能占满800字节    */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            rk_6033_task.frame = 12;    //2022-03-31 modify by hj
            //rk_6033_task.frame = 8;
            //get 节点个数
            index_t = subp_rcd_p(rcd_ot_m.head.node, rcd_ot_m.tail.node, RCD_OT_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                   //分包初始包从0开始
            rk_6033_task.head = rcd_ot_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd_ot_m.tail.node;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_DRIVER_OT;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            rk_6033_task.head = decp_rcd_p(T_RCD_DRIVER_OT, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x12: //采集指定的驾驶人身份记录      T_RCD_DRIVER_LOG       rcd_driver_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd_driver_m.head.node == rcd_driver_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: 800/25 = 31个节点 不能占满800字节    */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            //rk_6033_task.frame = 31;//2022-03-31 modify by hj
            rk_6033_task.frame = 20;
            //get 节点个数
            index_t = subp_rcd_p(rcd_driver_m.head.node, rcd_driver_m.tail.node, RCD_DRIVER_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                       //分包初始包从0开始
            rk_6033_task.head = rcd_driver_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd_driver_m.tail.node;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_DRIVER_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            rk_6033_task.head = decp_rcd_p(T_RCD_DRIVER_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x13: //采集指定的外部供电记录    T_RCD_POWER_LOG  rcd_power_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd_power_m.head.node == rcd_power_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: 800/7 = 113个节点 不能占满800字节    */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            //rk_6033_task.frame = 113; //2022-03-31 modify by hj
            rk_6033_task.frame = 80;
            //get 节点个数
            index_t = subp_rcd_p(rcd_power_m.head.node, rcd_power_m.tail.node, RCD_POWER_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                      //分包初始包从0开始
            rk_6033_task.head = rcd_power_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd_power_m.tail.node;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_POWER_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            rk_6033_task.head = decp_rcd_p(T_RCD_POWER_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x14: //采集指定的参数修改记录    T_RCD_PARAMETER_LOG rcd_para_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd_para_m.head.node == rcd_para_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: 800/7 = 113个节点 不能占满800字节    */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            //rk_6033_task.frame = 113;//2022-03-31 modify by hj
            rk_6033_task.frame = 80;
            //get 节点个数
            index_t = subp_rcd_p(rcd_para_m.head.node, rcd_para_m.tail.node, RCD_PARA_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                     //分包初始包从0开始
            rk_6033_task.head = rcd_para_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd_para_m.tail.node;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_PARAMETER_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            rk_6033_task.head = decp_rcd_p(T_RCD_PARAMETER_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x15: //采集指定的速度状态日志   T_RCD_SPEED_LOG  rcd_speed_m
    {
        u16 index_t = 0;
        u8 i = 0;
        u8 j = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*如果头、尾指针相等, 表示无记录,则需要转向下一个cmd */
        if (rcd_speed_m.head.node == rcd_speed_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("无记录 cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*所有参数清0, 进入下一个cmd 处理*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //置发送标志
                rk_6033_task.u8_delay = jiffies; //设置等待应答延迟
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*取节点总数, 每个节点大小为128byte, 每次发送缓冲区最大为800 */
        /*故每次发送的节点数为: 800/133= 6个节点 不能占满800字节     */
        /*这里主要是按6033协议进行总包数计算                         */
        if (rk_6033_task.packet == 0) { //如果已经get 节点总数 则跳过这步, packet 第一次进来之前已经是清0的
            //rk_6033_task.frame = 6;//2022-03-31 modify by hj
            rk_6033_task.frame = 5;
            //get 节点个数
            index_t = subp_rcd_p(rcd_speed_m.head.node, rcd_speed_m.tail.node, RCD_SPEED_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //计算总包数
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // 除不尽 总包数要加一包余量
                rk_6033_task.packet++;                              //有剩余包
            }
            rk_6033_task.idx = 0;                      //分包初始包从0开始
            rk_6033_task.head = rcd_speed_m.head.node; //保存当前的 头尾指针。
            rk_6033_task.tail = rcd_speed_m.tail.node;
            next_speed_log_cnt = 0;
            next_speed_log_offset = 0;
        }

        /*数据组织: 按要求是按结束时间进行组织,故数据***从头向尾***进行组织 */
        /*同时每个节点的时间需要进行BCD码转换                               */
        /*针对6033 的分包数和总包数的处理在 6033的应答中进行处理            */
        /*  即 rk_6033_task.packet 和 rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

#if 1
            //if(next_speed_log_offset == 0)
            if (next_speed_log_cnt == 0) {
                //logd("记录15H  index_t=[%d] rk_6033_task.head=[%d] rk_6033_task.tail=[%d]",index_t,rk_6033_task.head,rk_6033_task.tail);
                spi_flash_read(msg_t_1, RSPEED_ADDRD(rk_6033_task.head), 7);
                rk_6033_task.head_offset = rk_6033_task.head;
                //logd("时间: %02x-%02x-%02x %02x:%02x:%02x",msg_t_1[1],msg_t_1[2],msg_t_1[3],msg_t_1[4],msg_t_1[5],msg_t_1[6]);
                for (j = 1; j < 5; j++) { //每天重复日期最多5条

                    if ((rk_6033_task.head - j) == rk_6033_task.tail) {
                        //logd("等于尾, 退出");
                        break;
                    }

                    //spi_flash_read(buf_t_1, RSPEED_ADDRD(rk_6033_task.head-j), 7);
                    rk_6033_task.head_offset = decp_rcd_p(T_RCD_SPEED_LOG, rk_6033_task.head_offset, 1);
                    spi_flash_read(buf_t_1, RSPEED_ADDRD(rk_6033_task.head_offset), 7);

                    //logd("j=[%d]  %02x-%02x-%02x %02x:%02x:%02x",j,buf_t_1[1],buf_t_1[2],buf_t_1[3],buf_t_1[4],buf_t_1[5],buf_t_1[6]);

                    if ((buf_t_1[1] == msg_t_1[1]) && (buf_t_1[2] == msg_t_1[2]) && (buf_t_1[3] == msg_t_1[3])) {
                        next_speed_log_cnt++;
                        next_speed_log_offset++;
                    } else {
                        break;
                    }
                }
            }

//logd("查询同一天:记录15H  next_speed_log_offset=[%d]  next_speed_log_cnt=[%d]",next_speed_log_offset,next_speed_log_cnt);
//spi_flash_read(msg_t, RSPEED_ADDRD(index_t - next_speed_log_offset ), 133);
#endif

            /*注意: rcd_assemble_body 里面有个变量是:rsp.type ,故为保证程序执行需要：赋值rsp.type*/
            rsp.type = T_RCD_SPEED_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - next_speed_log_offset, &usb_rsp.rcdDataBuf[usb_rsp.len]);
//usb_rsp.len += rcd_assemble_body(rk_6033_task.head, &usb_rsp.rcdDataBuf[usb_rsp.len]);
/*2022-04-21 处理时间正序*/

//logd("记录15H  next_speed_log_offset=[%d]  next_speed_log_cnt=[%d]",next_speed_log_offset,next_speed_log_cnt);
#if 1
            if (next_speed_log_offset > 0) {
                next_speed_log_offset--;
                //logd("...... 我在这里 .....1");
            } else {
                //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
                //默认从最新的数据往下传,即从头开始
                //if(next_speed_log_cnt = 0)
                {
                    if (next_speed_log_cnt == 0) {
                        rk_6033_task.head = decp_rcd_p(T_RCD_SPEED_LOG, rk_6033_task.head, 1);
                        next_speed_log_offset = 0;
                        next_speed_log_cnt = 0;
                        //logd("...... 我在这里 .....2");
                    } else {
                        rk_6033_task.head = decp_rcd_p(T_RCD_SPEED_LOG, rk_6033_task.head, next_speed_log_cnt + 1);
                        next_speed_log_offset = 0;
                        next_speed_log_cnt = 0;
                        //logd("...... 我在这里 .....3");
                    }
                }
            }
#endif

            //注意： decp_rcd_p这个函数已经考虑了指针溢出的问题,故档尾大于头时,也能正常拨指针。
            //默认从最新的数据往下传,即从头开始
            //rk_6033_task.head = decp_rcd_p(T_RCD_SPEED_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //置发送标志
        rk_6033_task.u8_delay = jiffies;      //设置等待应答延迟
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //做下容错
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    }
}

#endif
