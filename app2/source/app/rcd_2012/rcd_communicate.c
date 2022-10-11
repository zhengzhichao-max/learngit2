/********************************************************************************
  * @file       rcd_communicate.c
  * @author  c.w.s
  * @Email
  * @version  V1.0.0
  * @date     2012-07-03
  * @brief   ��ʻ��¼����Ϣ����
 1. ���ļ��������ݽ���(����) �����ݷ���(���)����ģ��
 2. ���ļ��ʺ�UART ��ȡ��NET ƽ̨��ȡ������USB��ȡ
    ********************************************************************************/
#include "include_all.h"
#include <stdlib.h>
//#include <time.h>

#if (P_RCD == RCD_BASE)
rcd_send_pointor_struct rsp; //,rk_rsp;	 //���ݴ�����ƽṹ��
usb_read_rcd_struct usb_rsp; //usb���ݴ���ṹ��

usb_read_rcd_data_p usb_rcd_pTmp;

rec_c3_t c3_data; //3C��׼

u8 Data_Direction;
Allot_data_t Allot;            //�ְ���Ϣ
tr9_6033_task_t tr9_6033_task; //���ͽ��

rk_6033_task_t rk_6033_task;
u8 next_speed_log_cnt = 0;
u8 next_speed_log_offset = 0;
bool refreshTime = true;
/****************************************************************************
* ����:    _verify_time_range ()
* ���ܣ�ʱ��У��
* ��ڲ�������
* ���ڲ�������
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
* ����:    big_convert ()
* ���ܣ�RCD2012��ʻ��¼�ǣ����ݴ�С��ת��
* ��ڲ�������
* ���ڲ�������
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
* ����:    get_rcd_pack_no ()
* ���ܣ��õ�ָ�����ţ�����Ҫ�󲹴�ʱ����ˮ�ŵ�������Ϣ����Ҫһ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void get_rcd_pack_no(void) {
    u16 offset;

    offset = 0;

    if (rsp.type == T_RCD_360) { //ÿ�� 1������   ÿ�� 666 byte
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
    	//ÿ�� 5������ 
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
* ����:    check_start_time ()
* ���ܣ��õ���ʼ���͵�ʱ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u8 check_start_time(time_t t, bool ht) {
    s32 s1;
    s32 s2;
    s32 s3;

    s1 = get_standard_sec_num(t);             //�洢ʱ��
    s2 = get_standard_sec_num(rsp.ask.end);   //����ʱ��
    s3 = get_standard_sec_num(rsp.ask.start); //��ʼʱ��

    // if (s1 == -1 || s2 == -1 || s3 == -1)
    if ((s1 == (s32)-1) || (s2 == (s32)-1) || (s3 == (s32)-1))
        return false;

    if ((s1 <= s2) && (ht == true)) //���ҷ���ͷ
    {
        if (s1 >= s3)
            return 1;
        else
            return 0;
    } else if ((s1 >= s3) && (ht == false)) //��ȡʱ��< ����ʱ��
    {
        if (s1 <= s2)
            return 1;
        else
            return 0;
    }

    return 0;
}

/****************************************************************************
* ����:    get_rcd48h_index ()
* ���ܣ��õ���������ƽ�����ټ�¼������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u16 get_rcd48h_index(bool ht) {
    u16 low;

    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //��ʼʱ��㣻
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
        if (rst == 1) //��ǰ��Ч����
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_48, low, 1); //ָ���
        } else {
            low = incp_rcd_p(T_RCD_48, low, 1); //ָ���
        }

        i++;
    }
    return 0xffff;
}

/****************************************************************************
* ����:    get_rcd360h_index ()
* ���ܣ��õ�360Сʱ��λ����Ϣ��¼������
* ��ڲ�������
* ���ڲ�������
*
****************************************************************************/
static u16 get_rcd360h_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //��ʼʱ��㣻
    {
        low = rcd360_m.head.node; //ͷ���
                                  //		high = rcd360_m.tail.node;  //β�ڵ�
    } else {
        //		high= rcd360_m.head.node;
        low = rcd360_m.tail.node;
    }

    cnt = subp_rcd_p(rcd360_m.head.node, rcd360_m.tail.node, RCD360_NODE_MAX_SIZE) + 1;
    //logd("********rcd360_m.head.node = %d  rcd360_m.tail.node = %d", rcd360_m.head.node, rcd360_m.tail.node);
    //logd("********cnt = %d", cnt);
    while (i <= cnt) {
        spi_flash_read((u8 *)&time, _rcd360_t_addr(low), 6);
        //logd("data save time = %02d��%02d��%02d��%02dʱ%02d��%02d��", time.year, time.month, time.date, time.hour, time.min, time.sec);
        //logd("***************low = %d", low);
        if (ht)
            rst = check_start_time(time, ht); //�����Ƿ�Ϊ��Ч����
        else
            rst = check_start_time(time, ht);
        if (rst == 1) //��ǰ��Ч����
        {
            return low;
        }
        if (ht) {
            low = decp_rcd_p(T_RCD_360, low, 1); //ָ���
        } else {
            low = incp_rcd_p(T_RCD_360, low, 1); //ָ���
        }

        i++;
    }
    return 0xffff;
}

/****************************************************************************
* ����:    get_rcd_accident_index ()
* ���ܣ��õ��¹��ɵ��¼������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u16 get_rcd_accident_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //��ʼʱ��㣻
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
            //��ǰ��Ч����
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
* ����:    get_rcd_fatigue_index ()
* ���ܣ��õ�ƣ�ͼ�ʻ��¼������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u16 get_rcd_fatigue_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
    u16 i = 0;
    u8 buf[30] = {0};
    cnt = 0;
    if (ht) //��ʼʱ��㣻
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
        if (rst == 1) //��ǰ��Ч����
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
* ����:    get_rcd_driver_index ()
* ���ܣ��õ���ʻԱ��¼������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u16 get_rcd_driver_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
//    u8 buff[30] = {0};
    u16 i = 0;

    cnt = 0;
    if (ht) //��ʼʱ��㣻
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
        if (rst == 1) //��ǰ��Ч����
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
* ����:    get_rcd_power_index ()
* ���ܣ��õ���Դ��¼������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u16 get_rcd_power_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //��ʼʱ��㣻
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
        if (rst == 1) //��ǰ��Ч����
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
* ����:    get_rcd_param_index ()
* ���ܣ��õ������޸ļ�¼������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static u16 get_rcd_param_index(bool ht) {
    u16 low;
    //	u16 high;
    u16 cnt;
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
    u16 i = 0;

    cnt = 0;
    if (ht) //��ʼʱ��㣻
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
        if (rst == 1) //��ǰ��Ч����
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
    u8 rst = 0; //=0 С�� =1 ���� =2 ����
    time_t time;
    u16 i = 0;
    u8 buf[30];

    cnt = 0;
    if (ht) //��ʼʱ��㣻
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
        if (rst == 1) //��ǰ��Ч����
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
* ����:    rcd_get_send_node ()
* ���ܣ��õ���¼�Ƿ��ͽڵ������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static bool rcd_get_send_node(void) {
    rsp.cur_no = 1;

    switch (rsp.type) {
    case T_RCD_48:
        if (pos48.ifnew == false) {
            rsp.cur.head = get_rcd48h_index(true); //�ҵ���Ч��Ϣͷ
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
    case T_RCD_360: //360Сʱλ����Ϣ
        if (pos360.ifnew == false) {
            rsp.cur.head = get_rcd360h_index(true);  //�õ�ͷ���
            rsp.cur.tail = get_rcd360h_index(false); //�õ�β���
            if (rsp.cur.head > RCD360_NODE_MAX_SIZE || rsp.cur.tail > RCD360_NODE_MAX_SIZE) {
                logdr("rcd_get_send_node err :360");
                return false;
            }
            rsp.cur.tail = decp_rcd_p(T_RCD_360, rsp.cur.tail, 1);                      //��ȡβָ������
            rsp.all_num = subp_rcd_p(rsp.cur.head, rsp.cur.tail, RCD360_NODE_MAX_SIZE); //+1 �õ�λ����Ϣ�ܰ���
            rsp.total = rsp.all_num * RCD360_HOUR_INF_SIZE;                             //360Сʱλ����Ϣ�����ֽ���
        } else {
            rsp.cur.head = pos360.head;
            rsp.cur.tail = pos360.tail;
        }
        break;
    case T_RCD_ACCIDENT: //�¹��ɵ��¼
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
    case T_RCD_DRIVER_OT: //��ʱ��ʻ��¼
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
    case T_RCD_DRIVER_LOG: //��ʻ����Ϣ��¼
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

    rsp.start_node = rsp.cur.head; //��ʼ���͵Ľڵ�
    get_rcd_pack_no();             //�õ���֡�ϰ�����
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
* ����:    rcd_assemble_body ()
* ���ܣ���¼����Ϣ�����
* ��ڲ�������
* ���ڲ�������
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
        { //�����ʱ�䷶Χ,�ڲ������ʱ, �����·�ʱ�䷶Χ, �����������־���� ���֡�
            /*У�鿪ʼʱ�� �� ����ʱ�䷶Χ: 2022-03-24 add by hj*/
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

        _nbintobcd(msg_t, 6); //ʱ��
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

            _memcpy((u8 *)&time, &msg_t[24], 6); //�����ʱ��洢����
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
        { //У��ʱ���Ƿ���ȷ��
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
                   logd("��¼15H  index_t=[%d]  rk_6033_task.tail=[%d]",index_t,rk_6033_task.tail);                 
                   spi_flash_read(msg_t, RSPEED_ADDRD(index_t), 133);
                   for(i=2;i>0;i--)
                   {//ÿ���ظ��������5��
				             if((index_t-i) == rk_6033_task.tail)
				             	{
                        logd("����β, �˳�");                      
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
                
               //logd("��¼15H  next_speed_log_offset=[%d]  next_speed_log_cnt=[%d]",next_speed_log_offset,next_speed_log_cnt);                 
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
         logdNoNewLine("cmd=%0x02  �������� index node =-%d-  read_Len=-%d-",index_t,p_len);
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
* ����:    load_new_rcd_data ()
* ���ܣ�����������ȡʱ����Ϣ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static bool load_new_rcd_data(u8 from_t, u8 id_t) {
    _memset((u8 *)&rsp, 0x00, sizeof(rsp));

    rsp.id = id_t;
    rsp.from = from_t;
    rsp.ack_no = rev_inf.run_no;

    return true;
}

/****************************************************************************
* ����:   rcd_send_data () ////RCD_C_OK
* ���ܣ���¼�����ݷ���
* ��ڲ�������
* ���ڲ�������
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
        if (result == RCD_C_SET_ERR) //��������֡���ճ���
        {
            ack_buf[ack_len++] = 0x55;
            ack_buf[ack_len++] = 0x7a;
            ack_buf[ack_len++] = 0xfb;
            ack_buf[ack_len++] = 0x00;
            xor = 0;
        } else if (result == RCD_C_GET_ERR) //�ɼ���������֡���ճ���
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
        for (i = 0; i < ack_len; i++) //���У��
        {
            xor ^= ack_buf[i];
        }
        ack_buf[ack_len++] = xor;
#endif
    }

    if (from == FROM_U1 || from == FROM_U3) {
        send_any_uart(from, ack_buf, ack_len); //�򴮿ڷ�������
        if ((FROM_U1 == from) && (u1_m.cur.b.rcd_3c)) {
            set_uart_ack(from);
        }

    } else if (from == FROM_NET) //�����緢������
    {
        //��һ������Ӧ����ˮ�ź�����ID
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
		idx	   = Allot.idx + 1    ;						//��Ŵ�һ��ʼ
		packet = Allot.packet     ;						//��Ŵ�һ��ʼ
#endif

#if 1                                 //2022-03-17
        idx = rk_6033_task.idx + 1;   //��Ŵ�һ��ʼ
        packet = rk_6033_task.packet; //��Ŵ�һ��ʼ
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
    logd("cmd=%02x  �ܰ���=-%d-  ��ǰ��=-%d-  ͷ�ڵ�=-%d-  β�ڵ�=-%d- ���ͳ��� Len=%d",rk_6033_task.cmd,\
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

        //logd("19056���ݵ���:[	 ʱ��:%2x:%2x ָ��:%02x �ܰ�:%d ���:%d ���ݳ���:%d]",mix.time.hour,mix.time.min,id,packet,idx,msg_len);
        tr9_frame_pack2rk(tr9_cmd_6033, msg_buf, msg_len); //��ʻ��¼�ɼ�������6033����ϢID
    }
    return true;
}

/****************************************************************************
* ����:    analyse_rcd0x00_ver ()
* ���ܣ�������¼�ǰ汾
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x01_driver ()
* ���ܣ�������ʻԱ��Ϣ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
static void analyse_rcd0x01_driver(u8 from, u8 *str, u16 len) {
    u8 s_buf[64] = {0};
    u16 s_len;

    if (str == NULL || len > D_BUF_LEN)
        return;

    s_len = 0;
    s_len += _memcpy_len(&s_buf[s_len], (u8 *)&driver_cfg.license, 18); //��ҵ�ʸ�֤�� ��ʻ֤��
    rsp.all_num = 1;
    rsp.cur_no = 1;
    rsp.total = s_len;
    rcd_send_data(from, RCD_C_OK, 0x01, s_buf, s_len);
}

/****************************************************************************
* ����:    analyse_rcd0x02_real_time ()
* ���ܣ�����ʵʱʱ��
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x03_mileage ()
* ���ܣ����������ز���
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x04_plus ()
* ���ܣ������������
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x05_vehicle_info ()
* ���ܣ�����������Ϣ
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x06_signal_cfg ()
* ���ܣ������ܽ�������Ϣ
* ��ڲ�������
* ���ڲ�������
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

	sig_state.b.s1 = tr9_car_status.bit.res4 ;	//����״̬
	sig_state.b.s2 = tr9_car_status.bit.res3 ;	
	sig_state.b.s3 = tr9_car_status.bit.near_light;	//����
	sig_state.b.s4 = tr9_car_status.bit.far_light ;	//Զ��
	sig_state.b.s5 = tr9_car_status.bit.right ;
	sig_state.b.s6 = tr9_car_status.bit.left ;
	sig_state.b.s7 = tr9_car_status.bit.brake;		//�ƶ�

	*/
   

    //Ӧ�����
    /*
    _memset((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
    _memcpy((u8 *)&sig_cfg.b0, "�ػ�״̬", 8);
    _memcpy((u8 *)&sig_cfg.b1, "��������", 8);
    _memcpy((u8 *)&sig_cfg.b2, "��·�Ͽ�", 8);
    _memcpy((u8 *)&sig_cfg.b3, "����", 6);
    _memcpy((u8 *)&sig_cfg.b4, "Զ��", 6);
    _memcpy((u8 *)&sig_cfg.b5, "��ת��", 4);
    _memcpy((u8 *)&sig_cfg.b6, "��ת��", 4);
    _memcpy((u8 *)&sig_cfg.b7, "�ƶ�", 4);
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
* ����:    analyse_rcd0x06_signal_cfg ()
* ���ܣ�������¼��ΨһID
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x08_speed_record ()
* ���ܣ���ȡ2���������ƽ���ٶȼ�¼
* ��ڲ�������
* ���ڲ�������
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
    rsp.packnum = max_len_t; //���λ���ݿ����

    if ((!_verify_time(&start_t)) || (!_verify_time(&end_t))) {
        goto err_rcd0x08;
    }
    pos48.ifnew = false;

    rsp.type = T_RCD_48; //����
    rsp.from = from;     //��������

    _memcpy((u8 *)&rsp.ask.start, (u8 *)&start_t, 6); //��ʼʱ��
    _memcpy((u8 *)&rsp.ask.end, (u8 *)&end_t, 6);     //����ʱ��

    ret = rcd_get_send_node();
    if (ret == false) {
    err_rcd0x08:
        _memset((u8 *)&rsp, 0x00, sizeof(rsp));
        rcd_send_data(from, RCD_C_OK, 0x08, s_buf, 0);
        logd("rcd err: 08 send node");
    }
}

/****************************************************************************
* ����:    analyse_rcd0x09_position ()
* ���ܣ���ȡ360 Сʱ��λ����Ϣ��¼
* ��ڲ�������
* ���ڲ�������
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
    _memcpy((u8 *)&start_t, str, 6); //��ʼʱ��
    _nbcdtobin((u8 *)&start_t, 6);
    _memcpy((u8 *)&end_t, &str[6], 6); //����ʱ��
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
* ����:    analyse_rcd0x10_accident ()
* ���ܣ���ȡ�¹��ɵ���Ϣ��¼
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x11_drive_OT ()
* ���ܣ���ȡƣ�ͼ�ʻ��Ϣ��¼
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x12_driver_login ()
* ���ܣ���ȡ��ʻԱ��Ϣ��¼
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x13_power_log ()
* ���ܣ���ȡ��Դ��Ϣ��¼
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x14_parameter_log ()
* ���ܣ���ȡ�����޸ļ�¼
* ��ڲ�������
* ���ڲ�������
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
��    �ƣ�analyse_rcd0x15_speed_log
��    �ܣ��ɼ��ⲿ�����¼
�����������
��    ������
��д���ڣ�2013-01-08
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
* ����:    analyse_rcd0x81_license ()
* ���ܣ����ü�ʻԱ֤��
* ��ڲ�������
* ���ڲ�������
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
    _memcpy((u8 *)&mix.ic_driver, (u8 *)&driver_cfg.license, 18); //2018-1-22,�¹����,�������˾��,ƣ�ͼ�ʻ��������������
    flash_write_normal_parameters();

    rcd_para_inf.en = true;
    rcd_para_inf.type = 0x81;

    //���õĳ�����ϢҪ�����ӻ�
    rcd_send_data(from, RCD_C_OK, 0x81, s_buf, 0);
}

/****************************************************************************
* ����:    analyse_rcd0x82_set_vehicle_info ()
* ���ܣ����ó�����Ϣ
* ��ڲ�������
* ���ڲ�������
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

    //���õĳ�����ϢҪ�����ӻ�
    rcd_send_data(from, RCD_C_OK, 0x82, s_buf, 0);
}

/****************************************************************************
* ����:    analyse_rcd0x83_set_setting_time ()
* ���ܣ����ð�װʱ��
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0x84_set_signal_cfg ()
* ���ܣ����ùܽŶ˿���Ϣ
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0xc2_set_real_time ()
* ���ܣ�����ʵʱʱ��
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0xc3_set_plus ()
* ���ܣ����ü�¼������ϵ��
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0xc4_set_mileage ()
* ���ܣ����ü�¼����ʼ���
* ��ڲ�������
* ���ڲ�������
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
* ����:    analyse_rcd0xE0_docimasy ()
* ���ܣ��춨
* ��ڲ�������
* ���ڲ�������
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
    uLen += 1; //����������ֽ�
    //����
    uLen += _sw_endian(uBuff + uLen, (u8 *)&factory_para.set_speed_plus, 2);
    //�ٶ�
    uLen += _sw_endian(uBuff + uLen, (u8 *)&mix.speed, 2);
    //�����
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
* ����:    CmdRcdTab ()
* ���ܣ�2012��ʻ��¼��Э��
* ��ڲ�������
* ���ڲ�������
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
* ����:    rcd_communicate_parse ()
* ���ܣ���¼�����ݽ���
* ��ڲ�������
* ���ڲ�������
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
* ����:    rcd_data_send ()
* ���ܣ���¼�����ݷ��͹���
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void rcd_data_send(void) {
    u8 msg[RCD_COM_MAX_SIZE];
    static u16 msg_len = 0; //�����������
    u16 cur_index = 0;      //��ʱ����ͷָ��
    u16 i = 0;
    u8 buf[32];
    u16 tmp;
//    u16 sum = 0;

    static enum E_LINK_MANAGE {
        E_RCD_IDLE, //����
        E_RCD_SEND, //����
        E_RCD_SACK, //�ȴ��ظ�
        E_RCD_ERR   //����
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
                //���ݷ��ͽ���
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
                //���
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
        /*********22-06-14********�޸Ĵ˶�һ������********************/
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
                logd("[%s->%s :%d]�������", __FILE__, __FUNCTION__, __LINE__);
        }
        rsp.cur.tail = rsp.cur.head;

#endif

        /*********22-06-14********�޸���������********************/

        if (msg_len >= RCD_COM_MAX_SIZE) {
            loge("rcd send err : msg all len");
            rsp.cur.head = decp_rcd_p(rsp.type, rsp.cur.head, 1);
            step = E_RCD_IDLE;
            break;
        }

        rsp.send_jif = jiffies;
        rsp.ack = false;

        rcd_send_data(rsp.from, RCD_C_OK, rsp.id, msg, (u16)msg_len);
        //rcd_send_data(FROM_U4, RCD_C_OK, rsp.id, msg, (u16)msg_len);//22-06-06@������,����:������Ҫ�Ӵ������
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

        if (rsp.type == T_RCD_48) //���ٴ���
        {
            pos48.ifnew = true;
            pos48.head = rsp.cur.head;
            pos48.tail = rsp.cur.tail;
        }
        if (rsp.type == T_RCD_360) //���ٴ���
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
* ����:    get_uart_is_rcd ()
* ���ܣ��жϴ����Ƿ�����ʻ��¼�ǹ���
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
bool get_uart_is_rcd(u8 from_t) {
    if (u1_m.cur.b.rcd_3c == true && from_t == FROM_U1) {
        return true;
    } else {
        return false;
    }
}

/****************************************************************************
* ����:    uart_rcd_parse_proc ()
* ���ܣ�������ȡRCD ����(������ȡ��ʻ��¼������)
* ��ڲ�������
* ���ڲ�������
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
        from_id = get_uart_news(&present); //��ȡ��ǰ���ĸ�������������,���ҷ��ص�ǰ�Ĵ���ID
        ret = get_uart_is_rcd(from_id);    //�жϴ����Ƿ�ΪRCD����
        if (ret)  {                         //�����RCD����
            step = E_UART_RCD_RECV;
            logd("3c recv");
        }
        break;
    case E_UART_RCD_RECV:
        _memset(r_buf, 0x00, R1_BUF_SIZE);
        r_len = copy_uart_news(from_id, r_buf, present.cur_p, present.c_len); //����������Ϣ,������Ϣ����
        if (r_len < 5 || r_len > R1_BUF_SIZE) {
            step = E_UART_RCD_EXIT;
            break;
        }

        start_pos = 0;
        while (start_pos < r_len) //�ڴ�ѭ�����ҵ�����֡����
        {
            if ((r_len - start_pos) < 7) //��С֡����
            {
            EXIT_UART_RCD_PARSE_ERR:
                step = E_UART_RCD_EXIT;
                return;
            }

            if (r_buf[start_pos] == 0xaa && r_buf[start_pos + 1] == 0x75) //�ж��Ƿ�Ϊ����֡
            {
                pack_len = 0;
                pack_len = r_buf[start_pos + 3]; //��ȡ���ݿ鳤��,���ֽ�
                pack_len <<= 8;
                pack_len += r_buf[start_pos + 4]; //��ȡ���ݿ鳤��,���ֽ�
                pack_len += 7;

                if ((r_len - start_pos) < pack_len) //���Ȳ��㣬����(ʵ�ʽ��յ������ݳ���С�����ݰ��еĳ���)
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
* �� �� ��: USB_get_rcd_ver
* ��������: U�̻�ȡ��ʻ��¼�ǰ汾
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��: 2017
*********************************************************/
void USB_get_rcd_ver(void) {
    u8 i;
    u8 DataCode = 0x00; //���ݴ���
    u32 DataLen = 2;    //���ݳ���
    u8 RcdVer = 12;     //��¼��ִ�б�׼���
    u8 RcdRev = 0;      //�޸ĵ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ����
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;

    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "ִ�б�׼�汾���");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    *pf++ = RcdVer; //ִ�б�׼
    *pf++ = RcdRev; //�޸ĵ���
    TotalLen += 6;
    usb_rsp.len = TotalLen;
}

/********************************************************
* �� �� ��: USB_get_driver
* ��������: U�̻�ȡ��ǰ��ʻ����Ϣ
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��: 2017-12-15
*********************************************************/
void USB_get_driver(void) {
    u8 i;
    u8 DataCode = 0x01; //���ݴ���
    u32 DataLen = 18;   //���ݳ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ����

    u8 *pf;

    pf = usb_rsp.rcdDataBuf;

    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "��ǰ��ʻ����Ϣ");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    TotalLen += _memcpy_len(pf, (u8 *)&driver_cfg.license, DataLen); //��ҵ�ʸ�֤�� ��ʻ֤��
    usb_rsp.len = TotalLen;
}

/********************************************************
* �� �� ��: USB_get_real_time
* ��������: U�̻�ȡ��¼��ʵʱʱ��
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
void USB_get_real_time(void) {
    u8 i;
    u8 DataCode = 0x02; //���ݴ���
    u32 DataLen = 6;    //���ݳ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ����
    u8 *pf;
    //    u8 * ptmp;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "ʵʱʱ��");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;

    //ʵʱʱ��
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
* �� �� ��: USB_get_mileage
* ��������: U�̻�ȡ�ۼ���ʻ���
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
void USB_get_mileage(void) {
    u8 i;
    u8 DataCode = 0x03; //���ݴ���
    u32 DataLen = 20;   //���ݳ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u8 *pf;
    //    u8 * ptmp;
    u32 total_tmp;

    pf = usb_rsp.rcdDataBuf;

    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    read_mfg_parameter(); //��ȡϵͳ���ò���
    total_tmp = run.total_dist / 100;

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "�ۼ���ʻ���");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //ϵͳʵʱʱ��
    *pf++ = _bintobcd(sys_time.year);
    *pf++ = _bintobcd(sys_time.month);
    *pf++ = _bintobcd(sys_time.date);
    *pf++ = _bintobcd(sys_time.hour);
    *pf++ = _bintobcd(sys_time.min);
    *pf++ = _bintobcd(sys_time.sec);
    //��¼�ǳ��ΰ�װʱ��
    *pf++ = _bintobcd(factory_para.setting_time.year);
    *pf++ = _bintobcd(factory_para.setting_time.month);
    *pf++ = _bintobcd(factory_para.setting_time.date);
    *pf++ = _bintobcd(factory_para.setting_time.hour);
    *pf++ = _bintobcd(factory_para.setting_time.min);
    *pf++ = _bintobcd(factory_para.setting_time.sec);
    //��ʼ���
    *pf++ = _bintobcd((u8)((factory_para.set_mileage % 100000000) / 1000000));
    *pf++ = _bintobcd((u8)((factory_para.set_mileage % 1000000) / 10000));
    *pf++ = _bintobcd((u8)((factory_para.set_mileage % 10000) / 100));
    *pf++ = _bintobcd((u8)(factory_para.set_mileage % 100));
    //�ۼ���ʻ���
    *pf++ = _bintobcd((u8)((total_tmp % 100000000) / 1000000));
    *pf++ = _bintobcd((u8)((total_tmp % 1000000) / 10000));
    *pf++ = _bintobcd((u8)((total_tmp % 10000) / 100));
    *pf++ = _bintobcd((u8)(total_tmp % 100));
    TotalLen += 20;
    usb_rsp.len = TotalLen;
}

/********************************************************
* �� �� ��: USB_get_pulse
* ��������: U�̻�ȡ����ϵ��
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
void USB_get_pulse(void) {
    u8 i;
    u8 DataCode = 0x04; //���ݴ���
    u32 DataLen = 8;    //���ݳ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u8 *pf;
    //    u8 * ptmp;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    read_mfg_parameter(); //��ȡϵͳ���ò���

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "����ϵ��");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //ϵͳʵʱʱ��
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
* �� �� ��: USB_get_vehicle_ifno
* ��������: U�̻�ȡ������Ϣ
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
void USB_get_vehicle_ifno(void) {
    u8 i;
    u8 DataCode = 0x05; //���ݴ���
    u32 DataLen = 41;   //���ݳ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u8 *pf;
    u8 tmp_len = 0;
    //    recorder_para_struct recorder_para_tmp; //��ʻ��¼����ʱ�ṹ��

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "������Ϣ");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //spi_flash_read((u8 *)(&recorder_para_tmp), (flash_parameter_addr + flash_recorder_para_addr), (sizeof(recorder_para_tmp)));//��ȡ��ʻ��¼�ǲ���
    TotalLen += _memcpy_len(pf, ((u8 *)&car_cfg.vin), 17); //��ȡ����VIN��
    pf += 17;
    tmp_len = _strlen((u8 *)&car_cfg.license);
    if ((tmp_len >= 8) && (tmp_len <= 12)) //������ƺ��볤�� ==8,��ʾ�û����ù�����
    {
        TotalLen += _memcpy_len(pf, ((u8 *)&car_cfg.license), tmp_len); //��ȡ���ƺ���
        pf += 8;
        for (i = 0; i < (12 - tmp_len); i++) //���ƺ��벻��12λ�� 0 ���
        {
            *pf++ = 0x00;
        }
        TotalLen += (12 - tmp_len);
    } else //û�����ó��ƺ���,��0�����
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
* �� �� ��: USB_get_cfg_status_info
* ��������: U�̻�ȡ����״̬��Ϣ
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
void USB_get_vehicle_status_info(void) {
    u8 i;
    u8 DataCode = 0x06; //���ݴ���
    u32 DataLen = 87;   //���ݳ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "״̬�ź�������Ϣ");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //ϵͳʵʱʱ��
    *pf++ = _bintobcd(sys_time.year);
    *pf++ = _bintobcd(sys_time.month);
    *pf++ = _bintobcd(sys_time.date);
    *pf++ = _bintobcd(sys_time.hour);
    *pf++ = _bintobcd(sys_time.min);
    *pf++ = _bintobcd(sys_time.sec);

    *pf++ = 0x01; //�ź�״̬�ֽڸ���
    TotalLen += 7;
    _memset((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
    _memcpy((u8 *)&sig_cfg.b0, "�ź�1", 4);
    _memcpy((u8 *)&sig_cfg.b1, "�ź�2", 4);
    _memcpy((u8 *)&sig_cfg.b2, "�ź�3", 4);
    _memcpy((u8 *)&sig_cfg.b3, "�����", 6);
    _memcpy((u8 *)&sig_cfg.b4, "Զ���", 6);
    _memcpy((u8 *)&sig_cfg.b5, "��ת", 4);
    _memcpy((u8 *)&sig_cfg.b6, "��ת", 4);
    _memcpy((u8 *)&sig_cfg.b7, "ɲ��", 4);

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
* �� �� ��: USB_get_rcd_unique_ID
* ��������: U�̻�ȡ��¼��Ψһ�Ա��
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
void USB_get_rcd_unique_ID(void) {
    u8 i;
    u8 DataCode = 0x07; //���ݴ���
    u32 DataLen = 87;   //���ݳ���
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u8 *pf;
    u8 unique_time[3] = {0}; //��¼����������BCD��2018-6-13
    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = DataCode;
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "��¼��Ψһ�Ա��");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
    {
        *pf++ = 0x00;
    }
    TotalLen += 18;
    _sw_endian(pf, (u8 *)&DataLen, 4);
    pf += 4;
    TotalLen += 4;
    //���3C��֤����
    *pf++ = 'C';
    *pf++ = 0x00;
    *pf++ = 0x00;
    *pf++ = 0x00;
    *pf++ = 0x07;
    *pf++ = 0x02;
    *pf++ = 0x03;
    //��Ʒ��֤�ͺ�
    _memcpy_len(pf, (u8 *)"TR9", sizeof("TR9"));
    pf += sizeof("TR9");
    for (i = 0; i < (16 - sizeof("TR9")); i++) //��֤��Ʒ�ͺ�16���ֽ�,������0
    {
        *pf++ = 0x00;
    }

    //��¼����������������
    _str2tobcd(&unique_time[0], &unique_id.time[0]); //ת����2018-6-13
    _str2tobcd(&unique_time[1], &unique_id.time[2]); //ת����
    _str2tobcd(&unique_time[2], &unique_id.time[4]); //ת����
    *pf++ = unique_time[0];
    *pf++ = unique_time[1];
    *pf++ = unique_time[2];
    //������ˮ��
    *pf++ = unique_id.sn[0];
    *pf++ = unique_id.sn[1];
    *pf++ = unique_id.sn[2];
    *pf++ = unique_id.sn[3];
    //����5���ֽ�
    *pf++ = 00;
    *pf++ = 00;
    *pf++ = 00;
    *pf++ = 00;
    *pf++ = 00;
    TotalLen += 35;
    usb_rsp.len = TotalLen;
}

/********************************************************
* �� �� ��: USB_get_rcd_48speed
* ��������: U�̻�48Сʱ�ٶ���Ϣ
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��: 2017
*********************************************************/
bool USB_get_rcd_48speed(void) {
    u8 i;
    u8 DataCode = 0x08; //���ݴ���
    u32 DataLen;        //���ݳ��� = ���ݰ��� *
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u16 DataNum = 0;    //���ݰ���
    time_t time;
    // u8 buf_t[200];
    u8 ret;
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.rcd48_step_flg == true) {
        usb_rcd_pTmp.rcd48_step_flg = false;
        *pf++ = DataCode; //���ݴ���
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "��ʻ�ٶȼ�¼");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;

        DataNum = subp_rcd_p(usb_rcd_pTmp.rcd48_p_head, 0, RCD48_NODE_MAX_SIZE);
        //logd("DataNum = %d", DataNum);
        //��ȡ�����ܳ���
        DataLen = DataNum * 126;
        //logd("TotalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //���û������
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
* �� �� ��: USB_get_rcd_360loction
* ��������: U�̻�360Сʱλ����Ϣ
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
bool USB_get_rcd_360loction(void) {
    u8 i;
    u8 DataCode = 0x09; //���ݴ���
    u32 DataLen;        //���ݳ��� = ���ݰ��� * 666
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u16 DataNum = 0;    //���ݰ���
    time_t time;
    u8 buf_t[20];
    u8 ret;
    u8 *pf;
    S_RCD_360HOUR_INFOMATION rcd360_inf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.rcd360_step_flg == true) //��һ��Ҫд�����ݴ���,��������,���ݳ�����Ϣ
    {
        usb_rcd_pTmp.rcd360_step_flg = false;
        *pf++ = DataCode; //���ݴ���
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "λ����Ϣ��¼");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //��ȡ�����ܰ���
        DataNum = subp_rcd_p(usb_rcd_pTmp.rcd360_p_head, usb_rcd_pTmp.rcd360_p_tail, RCD360_NODE_MAX_SIZE);
        logd("DataNum = %d", DataNum);
        //��ȡ�����ܳ���
        DataLen = DataNum * RCD360_HOUR_INF_SIZE;
        //logd("TotalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //���û��360λ������
            return true;
    }

    spi_flash_read(buf_t, _rcd360_t_addr(usb_rcd_pTmp.rcd360_p_head), sizeof(time_t)); //��ӡ2017-12-26 14:00:00
    _memcpy((u8 *)&time, buf_t, 6);
    // logd("%2d��%2d��%2d��%2dʱ%2d��%2d��", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time); //ʱ��У��
    if (ret) {
        //���ʱ��
        *pf++ = _bintobcd(time.year);
        *pf++ = _bintobcd(time.month);
        *pf++ = _bintobcd(time.date);
        *pf++ = _bintobcd(time.hour);
        *pf++ = _bintobcd(time.min);
        *pf++ = _bintobcd(time.sec);
        TotalLen += 6;

        for (i = 0; i <= 59; i++) {
            spi_flash_read((u8 *)&rcd360_inf, _rcd360_d_addr(usb_rcd_pTmp.rcd360_p_head, i), sizeof(rcd360_inf));
            _sw_endian(pf, (u8 *)&rcd360_inf.locate.lngi, 4); //��γ������Ҫ���д�С��ת��
            pf += 4;
            _sw_endian(pf, (u8 *)&rcd360_inf.locate.lati, 4); //γ��
            pf += 4;
            _memcpy_len(pf, (u8 *)&rcd360_inf.heigh, 2); //���θ߶�
            pf += 2;
            _memcpy_len(pf, (u8 *)&rcd360_inf.speed, 1); //�ٶ�
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
* �� �� ��: USB_get_accident
* ��������: U�̻�ȡ�¹��ɵ��¼
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
bool USB_get_accident(void) {
    u8 i;
    u8 DataCode = 0x10; //���ݴ���
    u32 DataLen = 0;    //���ݳ��� = ���ݰ��� *
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u16 DataNum = 0;    //���ݰ���
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
        usb_rsp.len = _sprintf_len((char *)pf, "�¹��ɵ��¼");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //��ȡ�����ܰ���
        DataNum = subp_rcd_p(usb_rcd_pTmp.rcd_accident_p_head, usb_rcd_pTmp.rcd_accident_p_tail, RCD_ACCIDENT_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_ACCIDENT_INF_SIZE;
        //logd("TotalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
    }

    spi_flash_read(pf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_head), RCD_ACCIDENT_INF_SIZE);
    _memcpy((u8 *)&time, pf, 6);
    //logd("%02d��%02d��%02d��%02dʱ%02d��%02d��", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time); //ʱ��У��
    if (ret) {
        TotalLen += 234;
        _nbintobcd(pf, 6); //ʱ��ת��
        _memcpy((u8 *)&cvt_point, (pf + 224), 10);
        _sw_endian((pf + 224), (u8 *)&cvt_point.locate.lngi, 4); //��γ������Ҫ���д�С��ת��
        _sw_endian((pf + 228), (u8 *)&cvt_point.locate.lati, 4); //γ��
        _sw_endian((pf + 232), (u8 *)&cvt_point.heigh, 2);       //�߶�
        usb_rsp.len = TotalLen;
        return true;
    } else //���ʱ�����
    {
        return false;
    }
}

/********************************************************
* �� �� ��: USB_get_driver_OT
* ��������: U�̻�ȡ��ʱ��ʻ��¼
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
bool USB_get_driver_OT(void) {
    u8 i;
    u8 DataCode = 0x11; //���ݴ���
    u32 DataLen = 0;    //���ݳ��� = ���ݰ��� *
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u16 DataNum = 0;    //���ݰ���
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
        usb_rsp.len = _sprintf_len((char *)pf, "��ʱ��ʻ��¼");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //��ȡ�����ܰ���
        DataNum = subp_rcd_p(usb_rcd_pTmp.OT_pHead, usb_rcd_pTmp.OT_pTail, RCD_OT_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_ACCIDENT_INF_SIZE;
        //logd("driver timeout datalen  = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //���û�г�ʱ��ʻ��¼
            return true;
    }
    spi_flash_read(pf, RCDOT_ADDRD(usb_rcd_pTmp.OT_pHead), 50);
    _memcpy((u8 *)&time_start, (pf + 18), 6);
    //logd("start %2d��%2d��%2d��%2dʱ%2d��%2d��", time_start.year, time_start.month, time_start.date, time_start.hour, time_start.min, time_start.sec);
    ret1 = _verify_time(&time_start);
    _memcpy((u8 *)&time_end, (pf + 24), 6);
    //logd("end %2d��%2d��%2d��%2dʱ%2d��%2d��", time_end.year, time_end.month, time_end.date, time_end.hour, time_end.min, time_end.sec);
    ret2 = _verify_time(&time_end);
    if (ret1 && ret2) {
        _nbintobcd((pf + 18), 6); //��ʱ��ʻ��ʼʱ��ת��
        _nbintobcd((pf + 24), 6); //��ʱ��ʻ����ʱ��ת��

        _memcpy((u8 *)&cvt_point, (pf + 30), 10);

        _sw_endian((pf + 30), (u8 *)&cvt_point.locate.lati, 4); //��γ������Ҫ���д�С��ת��
        _sw_endian((pf + 34), (u8 *)&cvt_point.locate.lngi, 4); //γ��
        _sw_endian((pf + 38), (u8 *)&cvt_point.heigh, 2);       //�߶�

        _memcpy((u8 *)&cvt_point, (pf + 40), 10);
        _sw_endian((pf + 40), (u8 *)&cvt_point.locate.lati, 4); //��γ������Ҫ���д�С��ת��
        _sw_endian((pf + 44), (u8 *)&cvt_point.locate.lngi, 4); //γ��
        _sw_endian((pf + 48), (u8 *)&cvt_point.heigh, 2);       //�߶�

        TotalLen += 50;
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* �� �� ��: USB_get_driver_log
* ��������: U�̻�ȡ��ʻ����ݼ�¼
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
bool USB_get_driver_log(void) {
    u8 i;
    u8 DataCode = 0x12; //���ݴ���
    u32 DataLen = 0;    //���ݳ��� = ���ݰ��� *
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u16 DataNum = 0;    //���ݰ���
    time_t time;
    u8 ret;
    u8 *pf;
    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
    if (usb_rcd_pTmp.driver_log_flag == true) {
        usb_rcd_pTmp.driver_log_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "��ʻ����ݼ�¼");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
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
        if (DataNum == 0) //���û�м�ʻ����Ϣ��¼
            return true;
    }

    spi_flash_read(pf, DRIVER_ADDRD(usb_rcd_pTmp.driver_log_pHead), 25);
    TotalLen += 25;
    _memcpy((u8 *)&time, pf, 6);
    //logd("%2d��%2d��%2d��%2dʱ%2d��%2d��", time.year, time.month, time.date, time.hour, time.min, time.sec);
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
* �� �� ��: USB_get_power_log
* ��������: U�̻��ⲿ�����¼
* ��ڲ���: ��
* �� �� ֵ: true ������ȷ false ���ݴ���
* ��    ��:  2017-12-15
*********************************************************/
bool USB_get_power_log(void) {
    u8 i;
    u8 DataCode = 0x13; //���ݴ���
    u32 DataLen = 0;    //���ݳ��� = ���ݰ��� *
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u16 DataNum = 0;    //���ݰ���
    time_t time;
    u8 ret;
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    if (usb_rcd_pTmp.power_flag == true) {
        usb_rcd_pTmp.power_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "�ⲿ�����¼");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //��ȡ�����ܰ���
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
        _nbintobcd(pf, 6); //ʱ��ת��
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* �� �� ��: USB_get_para_rcd
* ��������: U�̻�ȡ�����޸ļ�¼
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
bool USB_get_para_rcd(void) {
    u8 i;
    u8 DataCode = 0x14; //���ݴ���
    u32 DataLen = 0;    //���ݳ��� = ���ݰ��� *
    u16 TotalLen = 0;   //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u16 DataNum = 0;    //���ݰ���
    time_t time;
    u8 ret;
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
    if (usb_rcd_pTmp.para_change_flag == true) {
        usb_rcd_pTmp.para_change_flag = false;
        *pf++ = DataCode;
        TotalLen += 1;
        usb_rsp.len = _sprintf_len((char *)pf, "�����޸ļ�¼");
        pf += usb_rsp.len;
        for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
        {
            *pf++ = 0x00;
        }
        TotalLen += 18;
        //��ȡ�����ܰ���
        DataNum = subp_rcd_p(usb_rcd_pTmp.para_change_pHead, usb_rcd_pTmp.para_change_pTail, RCD_PARA_NODE_MAX_SIZE);
        DataLen = DataNum * RCD_PARA_INF_SIZE;
        logd("pwrTatalLen = %d", DataLen);
        _sw_endian(pf, (u8 *)&DataLen, 4);
        pf += 4;
        TotalLen += 4;
        usb_rsp.len = TotalLen;
        if (DataNum == 0) //û�в����޸ļ�¼
            return true;
    }
    spi_flash_read(pf, _rcd_para_t_addr(usb_rcd_pTmp.para_change_pHead), 7);
    _memcpy((u8 *)&time, pf, 6);
    //logd("%d-%d-%d %d:%d:%d", time.year, time.month, time.date, time.hour, time.min, time.sec);
    ret = _verify_time(&time);
    if (ret) {
        TotalLen += 7;
        _nbintobcd(pf, 6); //ʱ��ת��
        usb_rsp.len = TotalLen;
        return true;
    } else {
        return false;
    }
}

/********************************************************
* �� �� ��: USB_get_speed_log
* ��������: U�̻�ȡ�ٶ�״̬��־
* ��ڲ���: ��
* �� �� ֵ: ��
* ��    ��:  2017-12-15
*********************************************************/
void USB_get_speed_log(void) {
    u8 i;
    u8 DataCode = 15; //���ݴ���
    u32 DataLen = 0;  //���ݳ���
    u16 TotalLen = 0; //�����ܳ��� = ���ݴ���  + �������� + ���ݳ��� + ���ݾ�
    u8 *pf;

    pf = usb_rsp.rcdDataBuf;
    _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));

    *pf++ = _bintobcd(DataCode);
    TotalLen += 1;
    usb_rsp.len = _sprintf_len((char *)pf, "�ٶ�״̬��־");
    pf += usb_rsp.len;
    for (i = 0; i < (18 - usb_rsp.len); i++) //�������ƹ̶�Ϊ18���ֽ�,���㲹0x00
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
* �� �� ��: Hg_Tachographs_Head
* ��������: ������ʻ��¼�����ݲɼ�Э��ͷ
* ��ڲ���: pcs���ְ���    			idx:��ǰ����    	cmd:������
* �� �� ֵ: ���ݳ���
* ��    ��:  2022-01-22
*****************************************************************************************************************/
u16 Hg_Tachographs_Head(u16 pcs, u16 idx, u8 cmd) {
    u8 *pf = NULL;
    _memset(usb_rsp.rcdDataBuf, 0x00, sizeof(usb_rsp.rcdDataBuf));
    usb_rsp.len = 0;
    pf = usb_rsp.rcdDataBuf;

    /* ����Э��ͷ*/
    //	pf[usb_rsp.len++] =  Allot.Data_Direction ;				//����ȥ��
    pf[usb_rsp.len++] = (u8)((pcs & 0xff00) >> 8); //�ְ����� ���ֽ�
    pf[usb_rsp.len++] = (u8)(pcs & 0x00ff);        //�ְ����� ���ֽ�
    pf[usb_rsp.len++] = (u8)((idx & 0xff00) >> 8); //�ְ���� ���ֽ�
    pf[usb_rsp.len++] = (u8)(idx & 0x00ff);        //�ְ���� ���ֽ�
    pf[usb_rsp.len++] = cmd;                       //������
    /* 19056Э��ͷ   */
    pf[usb_rsp.len++] = 0x55; //���ݱ�ʶλ
    pf[usb_rsp.len++] = 0x7a; //���ݱ�ʶλ
    pf[usb_rsp.len++] = cmd;  //������
    pf[usb_rsp.len++] = 0x00; //���ݳ��� ���ֽ�
    pf[usb_rsp.len++] = 126;  //���ݳ��� ���ֽ�
    pf[usb_rsp.len++] = 0x00; //����λ
    return usb_rsp.len;
}

#if 0	
/*************************************************************************************************
* �������ƣ�tr9_Speed_derive(u8 *export_data)
* �������ܣ���flash�е���48Сʱ����
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.11
* ��     ע��flash��ֻ�ܴ洢50��Сʱ�����ݣ�3000���� 	
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
					Special_state_8 = true ;										// ������������ͷ��ʼд�� 
				}
				else
				{
					usb_rcd_pTmp.rcd48_p_head  = rcd48_m.head.node; 	
					usb_rcd_pTmp.rcd48_p_tail  = rcd48_m.tail.node; 
					Special_state_8 = false ;										// ���� 
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
					Allot.packet = (usb_rcd_pTmp.rcd48_p_head / packet)  ; 	//����ܰ���
					if((usb_rcd_pTmp.rcd48_p_head % packet)	!= 0 )
						Allot.packet += 1;	
				}

				step = E_48H_WRITE_NORMAL;
			}break ;	
					
		case E_48H_WRITE_NORMAL:
			{ 

				if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x08)) //Ӧ���־
				{

					if( Special_state_8)
					{
						if(( RCD_ACCIDENT_P_SAVE_SIZE - usb_rcd_pTmp.rcd48_p_tail) >= packet)
						{
							spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), packet_len);		//һ����ʻ�ٶȼ�¼����
							export_data->len = packet_len ;
							usb_rcd_pTmp.rcd48_p_tail += packet;
						}
						else
						{
							export_data->len = 0 ;
							j = ( RCD_ACCIDENT_P_SAVE_SIZE - usb_rcd_pTmp.rcd48_p_tail) ;
							
							for(i = 0 ;i < j ; i++)
							{
								spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), data_len);		//һ����ʻ�ٶȼ�¼����
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
							spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), packet_len);		//һ����ʻ�ٶȼ�¼����
							export_data->len = packet_len ;
							usb_rcd_pTmp.rcd48_p_tail += packet;

						}
						else
						{
							export_data->len = 0 ;
							j = ( usb_rcd_pTmp.rcd48_p_head - usb_rcd_pTmp.rcd48_p_tail) ;
							
							for(i = 0 ;i < j ; i++)
							{
								spi_flash_read(export_data->rcdDataBuf, _rcd48_t_addr(usb_rcd_pTmp.rcd48_p_tail), data_len);		//һ����ʻ�ٶȼ�¼����
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
* �������ƣ�tr9_Location_derive(u8 *export_data)
* �������ܣ���flash�е���360Сʱλ����Ϣ
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.11
* ��     ע��
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
					Special_state_9 = true ;										// ������������ͷ��ʼд�� 
				}
				else
				{
					usb_rcd_pTmp.rcd360_p_head	= rcd360_m.head.node;	//��������Сʱ�ĸ���
					usb_rcd_pTmp.rcd360_p_tail	= rcd360_m.tail.node;	
					Special_state_9 = false ;										// ���� 
				}

				Allot.idx = 0 ;
				tr9_6033_task.state = 0 ;
				tr9_6033_task.cmd = 0x09 ;				
				
				Allot.packet = (usb_rcd_pTmp.rcd360_p_head - usb_rcd_pTmp.rcd360_p_tail);	//����ܰ���

				step = E_360H_WRITE_NORMAL;				
			}break ;	
					
		case E_360H_WRITE_NORMAL:
			{	
				if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x09)) //Ӧ���־
				{

					if( Special_state_9)
					{
						if(( RCD360_P_SAVE_SIZE - usb_rcd_pTmp.rcd360_p_tail ) > 0)
						{
							spi_flash_read(export_data->rcdDataBuf, _rcd360_t_addr(usb_rcd_pTmp.rcd360_p_tail), 666); //1Сʱ��ʻ�ٶȼ�¼����
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
							spi_flash_read(export_data->rcdDataBuf, _rcd360_t_addr(usb_rcd_pTmp.rcd360_p_tail), 666); //1Сʱ��ʻ�ٶȼ�¼����
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
* �������ƣ�tr9_get_accident(u8 *export_data)
* �������ܣ���flash�е��¹��ɵ��¼(10H)
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.11
* ��     ע��
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
						Special_state_10 = true ;										// ������������ͷ��ʼд�� 
					}
					else
					{
						usb_rcd_pTmp.rcd_accident_p_head = rcd_accident_m.head.node ;
						usb_rcd_pTmp.rcd_accident_p_tail = rcd_accident_m.tail.node ;
						Special_state_10 = false ;										// ���� 
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
						if(((rcd_accident_m.tail.node) %2) != 0 )		//����һ������
							Allot.packet++;
						
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.rcd_accident_p_head / 2) ;
						if((usb_rcd_pTmp.rcd_accident_p_head % 2) != 0 )		//����һ������
							Allot.packet++;
					}
				//	logd("�¹�ͷָ�� = 0x%02X, �¹�βָ�� = 0x%02X",rcd_accident_m.head.node,rcd_accident_m.tail.node);
				//	logd("�¹��ܰ��� = %d",Allot.packet,rcd_accident_m.tail.node);
					
				step = E_accident_WRITE_NORMAL ;
				}break ;

			case E_accident_WRITE_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x10)) 	//Rk���ճɹ�
					{
						if( Special_state_10)
						{
							if(( RCD_ACCIDENT_P_SAVE_SIZE - usb_rcd_pTmp.rcd_accident_p_tail) >= 2)
							{
								spi_flash_read(export_data->rcdDataBuf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), 468);  //2���ɵ�һ������
								export_data->len = 468 ;
								usb_rcd_pTmp.rcd_accident_p_tail += 2;
							}
							else
							{
								spi_flash_read(export_data->rcdDataBuf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), 234);  //1���ɵ�һ������
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
								spi_flash_read(export_data->rcdDataBuf, ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), 468);  //1���ɵ�һ������
								export_data->len = 468 ;
								usb_rcd_pTmp.rcd_accident_p_tail += 2;
							}
							else
							{
								export_data->len = 0 ;
								j = (usb_rcd_pTmp.rcd_accident_p_head - usb_rcd_pTmp.rcd_accident_p_tail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], ACCI_ADDRD(usb_rcd_pTmp.rcd_accident_p_tail), RCD_ACCIDENT_INF_SIZE); //һ���ɵ�234���ֽ�
									export_data->len += 234 ;
									usb_rcd_pTmp.rcd_accident_p_tail += 1;
								}
								//Allot.idx++;
							}
							if(usb_rcd_pTmp.rcd_accident_p_tail == rcd_accident_m.head.node )			//�������
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
* �������ƣ�tr9_get_driver_OT(u8 *export_data)
* �������ܣ���flash�е�����ʱ��ʻ��¼(11H)
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.12
* ��     ע��
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
						Special_state_11 = true ;										// ������������ͷ��ʼд�� 
					}
					else
					{
						usb_rcd_pTmp.OT_pHead = rcd_ot_m.head.node ;
						usb_rcd_pTmp.OT_pTail = rcd_ot_m.tail.node ;
						Special_state_11 = false ;										// ���� 
					}	
					
					Allot.idx = 0 ;
					tr9_6033_task.state = 0  ;
					tr9_6033_task.cmd = 0x11 ;
					if(Special_state_11)
					{
						Allot.packet  =	((RCD_OT_P_SAVE_SIZE - rcd_ot_m.tail.node)/15);
						Allot.packet += (rcd_ot_m.head.node / 15 ) ;					//15����ʱ��¼��һ��

						if((rcd_ot_m.head.node  %  15 ) != 0 )							//����һ������
							Allot.packet++ ;
						if(((RCD_OT_P_SAVE_SIZE - rcd_ot_m.tail.node)%15)!= 0)
							Allot.packet++ ;
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.OT_pHead / 15 ) ;					//15����ʱ��¼��һ��
						if((usb_rcd_pTmp.OT_pHead  %  15 ) != 0 )						//����һ������
							Allot.packet++ ;
					}
					

						
					step = E_driver_WRITE_NORMAL ;
					
				}break ;
			case E_driver_WRITE_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x11)) 	//Rk���ճɹ�
					{
						if(Special_state_11)
						{		
							if( RCD_OT_P_SAVE_SIZE - usb_rcd_pTmp.OT_pTail >= 15 )
							{
								spi_flash_read(export_data->rcdDataBuf,RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 750);  //15����ʱ��¼��һ��
								export_data->len = 750 ;
								usb_rcd_pTmp.OT_pTail += 15 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( RCD_OT_P_SAVE_SIZE  - usb_rcd_pTmp.OT_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 50); //һ����ʱ��¼50���ֽ�
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
								spi_flash_read(export_data->rcdDataBuf,RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 750);  //15����ʱ��¼��һ��
								export_data->len = 750 ;
								usb_rcd_pTmp.OT_pTail += 15 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( usb_rcd_pTmp.OT_pHead   - usb_rcd_pTmp.OT_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.OT_pTail), 50); //һ����ʱ��¼50���ֽ�
									export_data->len += 50 ;
									usb_rcd_pTmp.OT_pTail += 1;
								}
							}
						if(usb_rcd_pTmp.OT_pTail >= usb_rcd_pTmp.OT_pHead )			//�������
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
* �������ƣ�tr9_get_driver_log(u8 *export_data)
* �������ܣ���flash�е�����ʻ����ݼ�¼(12H)
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.12
* ��     ע��
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
						Special_state_12 = true ;										// ������������ͷ��ʼд�� 
					}
					else
					{
						usb_rcd_pTmp.driver_log_pHead = rcd_driver_m.head.node ;
						usb_rcd_pTmp.driver_log_pTail = rcd_driver_m.tail.node ;
						Special_state_12 = false ;										// ���� 
					}	
					
					Allot.idx = 0 ;
					usb_rsp.len = 0  ;
					tr9_6033_task.state = 0 ;	
					tr9_6033_task.cmd == 0x12 ;

					if( Special_state_12 )
					{

						Allot.packet  = ( (RCD_DRIVER_P_SAVE_SIZE - rcd_driver_m.tail.node) / 30 );
						Allot.packet += (rcd_driver_m.head.node / 30)  ;		//30����ʻԱ��ݼ�¼��һ��
						if((rcd_driver_m.head.node % 30 ) != 0 ) 			//����һ������
							Allot.packet++; 
						if(((RCD_DRIVER_P_SAVE_SIZE - rcd_driver_m.tail.node) % 30) != 0)
							Allot.packet++; 
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.driver_log_pHead / 30)  ;		//30����ʻԱ��ݼ�¼��һ��
						if((usb_rcd_pTmp.driver_log_pHead % 30 ) != 0 )				//����һ������
							Allot.packet++;						
					}
					step = E_driver_log_NORMAL ;
				}break ;
			case E_driver_log_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x12)) 	//Rk���ճɹ�
					{					

						if(Special_state_12)
						{
							if( RCD_DRIVER_P_SAVE_SIZE - usb_rcd_pTmp.driver_log_pTail >= 30)
							{
								spi_flash_read(export_data->rcdDataBuf,DRIVER_ADDRD(usb_rcd_pTmp.driver_log_pTail), 750);  //30����ʻԱ��ݼ�¼��һ��
								export_data->len = 750 ;
								usb_rcd_pTmp.driver_log_pTail += 30 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( RCD_DRIVER_P_SAVE_SIZE - usb_rcd_pTmp.driver_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.driver_log_pTail), 25); //һ����ʻԱ��ݼ�¼25���ֽ�
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
								spi_flash_read(export_data->rcdDataBuf,DRIVER_ADDRD(usb_rcd_pTmp.driver_log_pTail), 750);  //30����ʻԱ��ݼ�¼��һ��
								export_data->len = 750 ;
								usb_rcd_pTmp.driver_log_pTail += 30 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( rcd_driver_m.head.node - usb_rcd_pTmp.driver_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], RCDOT_ADDRD(usb_rcd_pTmp.driver_log_pTail), 25); //һ����ʻԱ��ݼ�¼25���ֽ�
									export_data->len += 25 ;
									usb_rcd_pTmp.driver_log_pTail += 1;
								}
							}
							if(usb_rcd_pTmp.driver_log_pTail >= usb_rcd_pTmp.driver_log_pHead)			//�������
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
* �������ƣ�tr9_get_power_log(u8 *export_data)
* �������ܣ���flash�е�����¼���ⲿ�����¼(13H)
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.12
* ��     ע��
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
						Special_state_13 = true ;										// ������������ͷ��ʼд�� 
					}
					else
					{
						usb_rcd_pTmp.power_log_pHead = rcd_power_m.head.node ;
						usb_rcd_pTmp.power_log_pTail = rcd_power_m.tail.node ;
						Special_state_13 = false ;										// ���� 
					}	
					
					
					Allot.idx = 0 ;
					tr9_6033_task.state = 0 ;
					tr9_6033_task.cmd = 0x13;
					if(Special_state_13)
					{
						Allot.packet  = ((RCD_POWER_P_SAVE_SIZE - rcd_power_m.tail.node) / 50); 
						Allot.packet += (usb_rcd_pTmp.power_log_pHead / 50 );		//100����¼���ⲿ������һ��
						if((usb_rcd_pTmp.power_log_pHead % 50 ) != 0 )			//����һ������
							Allot.packet++;
						if(((RCD_POWER_P_SAVE_SIZE - rcd_power_m.tail.node) / 50) != 0)
							Allot.packet++;		
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.power_log_pHead / 50 );		//100����¼���ⲿ������һ��
						if((usb_rcd_pTmp.power_log_pHead % 50 ) != 0 )			//����һ������
							Allot.packet++;

					}
					
					step = E_power_log_NORMAL ;
				}break ;
			case E_power_log_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x13)) 	//Rk���ճɹ�
					{

						if( Special_state_13 )
						{
							if( RCD_POWER_P_SAVE_SIZE - usb_rcd_pTmp.power_log_pTail >= 50)
							{		
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 350);  //100����¼���ⲿ������һ��						
								export_data->len = 350 ;
								usb_rcd_pTmp.power_log_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (RCD_POWER_P_SAVE_SIZE - usb_rcd_pTmp.power_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 7); //һ����ʻԱ��ݼ�¼25���ֽ�
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
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 350);  //50����¼���ⲿ������һ��
								export_data->len = 350 ;
								usb_rcd_pTmp.power_log_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (usb_rcd_pTmp.power_log_pHead  - usb_rcd_pTmp.power_log_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.power_log_pTail), 7); //һ����ʻԱ��ݼ�¼25���ֽ�
									export_data->len += 7 ;
									usb_rcd_pTmp.power_log_pTail += 1;
								}	
							}

							if(usb_rcd_pTmp.power_log_pTail >= usb_rcd_pTmp.power_log_pHead )			//�������
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
* �������ƣ�tr9_get_para_rcd(u8 *export_data)
* �������ܣ���flash�е�����¼�ǲ����޸ļ�¼(14H)
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.12
* ��     ע��
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
						Special_state_14 = true ;										// ������������ͷ��ʼд�� 
					}
					else
					{
						usb_rcd_pTmp.para_change_pHead = rcd_para_m.head.node ;
						usb_rcd_pTmp.para_change_pTail = rcd_para_m.tail.node ;
						Special_state_14 = false ;										// ���� 
					}	
					
					Allot.idx = 0 ;
					Allot.packet = 0 ;
					tr9_6033_task.state = 0 ;
					tr9_6033_task.cmd = 0x14 ;
			
					if( Special_state_14 )
					{
						Allot.packet  = ((RCD_PARA_P_SAVE_SIZE - rcd_para_m.tail.node)/50);
						Allot.packet += (rcd_para_m.head.node / 50) ;		//50����¼���ⲿ������һ��
						if((rcd_para_m.head.node % 50 ) != 0 )			//����һ������
							Allot.packet++;
						if(((RCD_PARA_P_SAVE_SIZE - rcd_para_m.tail.node) % 50 ) != 0)
							Allot.packet++;
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.para_change_pHead / 50) ;		//50����¼���ⲿ������һ��
						if((usb_rcd_pTmp.para_change_pHead % 50 ) != 0 )			//����һ������
							Allot.packet++;
					}

					
					step = E_para_NORMAL ;
				}break ;
			case E_para_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x14)) 	//Rk���ճɹ�
					{
						if( Special_state_14 )
						{	
							if((RCD_PARA_P_SAVE_SIZE  - usb_rcd_pTmp.para_change_pTail) >= 50)
							{	
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 350);  //50����¼���ⲿ������һ��	
								export_data->len = 350 ;
								usb_rcd_pTmp.para_change_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = ( RCD_PARA_P_SAVE_SIZE  - usb_rcd_pTmp.para_change_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 7); //һ�������¼25���ֽ�
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
								spi_flash_read(export_data->rcdDataBuf,POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 350);  //100����¼���ⲿ������һ��
								export_data->len = 350 ;
								usb_rcd_pTmp.para_change_pTail += 50 ;
							}
							else
							{
								export_data->len = 0 ;
								j = (usb_rcd_pTmp.para_change_pHead - usb_rcd_pTmp.para_change_pTail) ;
								
								for(i = 0 ;i < j ; i++)
								{
									spi_flash_read(&export_data->rcdDataBuf[export_data->len], POWER_ADDRD(usb_rcd_pTmp.para_change_pTail), 7); //һ�������¼25���ֽ�
									export_data->len += 7 ;
									usb_rcd_pTmp.para_change_pTail += 1;
								}
							
							}
							if(usb_rcd_pTmp.para_change_pTail >= usb_rcd_pTmp.para_change_pHead)			//�������
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
* �������ƣ�tr9_get_para_rcd(u8 *export_data)
* �������ܣ���flash�е�����¼�ǲ����޸ļ�¼(15H)
* ����������export_data�����ڲ���ʹ�ã��Ѳ�������ȥ
* ʱ     ��:	2022.2.12
* ��     ע��
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
						Special_state_15 = true ;										// ������������ͷ��ʼд�� 
					}
					else
					{
						usb_rcd_pTmp.speed_log_pHead = rcd_speed_m.head.node ;
						usb_rcd_pTmp.speed_log_pTail = rcd_speed_m.tail.node ;
						Special_state_15 = false ;										// ���� 
					}	

					
					Allot.idx = 0 ;	
					tr9_6033_task.state = 0  ;
					tr9_6033_task.cmd = 0x15 ;
					if( Special_state_15 )
					{

						Allot.packet  = (RCD_SPEED_P_SAVE_SIZE - rcd_speed_m.tail.node) / 5 ;
						Allot.packet += (usb_rcd_pTmp.speed_log_pHead / 5)      ;//100����¼���ⲿ������һ��
						if((usb_rcd_pTmp.speed_log_pHead % 5 ) != 0 )			//����һ������
							Allot.packet++;
						if(((RCD_SPEED_P_SAVE_SIZE - rcd_speed_m.tail.node) % 5 ) != 0)
							Allot.packet++;
					}
					else
					{
						Allot.packet = (usb_rcd_pTmp.speed_log_pHead / 5)      ;//100����¼���ⲿ������һ��
						if((usb_rcd_pTmp.speed_log_pHead % 5 ) != 0 )			//����һ������
							Allot.packet++;

					}

					step = E_SPEED_NORMAL ;
				}break ;
			case E_SPEED_NORMAL:
				{
					if((tr9_6033_task.state == 0) && (tr9_6033_task.cmd == 0x15)) 	//Rk���ճɹ�
					{

						if( Special_state_15 )
						{
							if( RCD_SPEED_P_SAVE_SIZE - usb_rcd_pTmp.speed_log_pTail >= 5)
							{	
								spi_flash_read(export_data->rcdDataBuf,RSPEED_ADDRD(usb_rcd_pTmp.speed_log_pTail), 665);  //665���ֽ���һ��
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
								
								spi_flash_read(export_data->rcdDataBuf,RSPEED_ADDRD(usb_rcd_pTmp.speed_log_pTail), 665);  //798���ֽ���һ��
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
							
							if(usb_rcd_pTmp.speed_log_pTail == usb_rcd_pTmp.speed_log_pHead)			//�������
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
	static bool Export_single = false  ; 	//����������ȫ������
	static u8 cmd  = 0 , cmd_bak = 0 ;
	static u8 id =  0 ;
	static u16 timer_send = 0 ;
	//data_package_t  send_buf ;
	
	
	/**********************��������ָ���ȫ������*************************/
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
					Rk_Task_Manage.RK_1717_state = true;	//��ʱ�ر�1717�ķ���
					//tr9_show = false  ;						//�رմ�������������ʾ
			case Cmd_0x01H:   ;
			case Cmd_0x02H:   ;
			case Cmd_0x03H:   ;
			case Cmd_0x04H:   ;
			case Cmd_0x05H:   ;	
			case Cmd_0x06H:   ;
			case Cmd_0x07H:
				{
					Allot.idx = 0 ; 				//��ǰ��
					Allot.packet = 1 ;				//�ܰ���
					rcd_communicate_parse(FROM_U4,id,&command,1);//2012��ʻ��¼��Э�� 	
					if(0 == tr9_6033_task.state)
						id ++ ;
					tr9_6033_task.state = 1 ;
				}break;

			case Cmd_0x08H:	
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_48H_Speed_derive(&usb_rsp);//08�ٶȲɼ�
					if(false == ret)
					{
						no_record 	 = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;	
						logd("0x08 ��48Сʱ����");
					}
					
				}break;
			case Cmd_0x09H:  
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_360H_Location_derive(&usb_rsp); 			 //09λ�òɼ� 
					if(false == ret)
					{
						no_record 	 = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;	
						logd("0x09 ��360Сʱλ������");
					}
				}break ;
			case Cmd_0x10H:  
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					logd("����ָ��10");
					ret = tr9_get_accident(&usb_rsp); 
					if(false == ret)
					{
						no_record 	 = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;
						logd("0x10 δ�����¹�");
					}
						
				}	break ;
			case Cmd_0x11H: 
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_driver_OT(&usb_rsp);
					if(false == ret)
					{
						no_record 	 = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;
						logd("0x11 ˾��δ��ʱ");
					}
					
				}break ;
			case Cmd_0x12H:  
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_driver_log(&usb_rsp);
					if(false == ret)
					{
						no_record  = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;
						logd("0x12 �޼�ʻ����ݼ�¼");
					}

				}break;	
			case Cmd_0x13H:
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_power_log(&usb_rsp);
					if(false == ret)
					{
						no_record 	 = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;
						logd("0x13 �޼�¼���ⲿ�����¼");
					}
				}break;		
			case Cmd_0x14H: 
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_para_rcd(&usb_rsp);			//
					if(false == ret)
					{
						no_record 	 = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;
						logd("0x14 �޼�¼�ǲ����޸ļ�¼");
					}
				}break;	
			case Cmd_0x15H: 
				{
					usb_rsp.len = 0  ;
					_memset(usb_rsp.rcdDataBuf, 0x00,800);
					
					ret = tr9_get_spped_rcd(&usb_rsp);			//�ٶȲ���ɼ�
					if(false == ret)
					{
						no_record 	 = true;			//������ִ����һ��ָ��
						Allot.idx	 = 0 ; 				//��ǰ��
						Allot.packet = 1 ;				//�ܰ���
						usb_rsp.len  = 0 ;
						logd("0x15 ���ٶ�״̬��־");
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

				rcd_send_data(FROM_U4,RCD_C_OK,id,usb_rsp.rcdDataBuf,usb_rsp.len);  //������Ч����
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
					logd("����:���������ܰ���");
				}
			}
			
			if(no_record)
			{
				no_record = false ;

				rcd_send_data(FROM_U4,RCD_C_OK,id,usb_rsp.rcdDataBuf,usb_rsp.len);  //������Ч����

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
			tr9_show = true ;				//�ڵ���״̬��ʾ������������	
			Ack_Export = false ;
 			Export_finish  = 1;				//��������
 			tr9_6033_task.state = 0 ;
 			Rk_Task_Manage.RK_1717_state = false ;
			Rk_Task_Manage.RK_6051_state = false ;
			
			
			_memset(usb_rsp.rcdDataBuf, 0x00,sizeof(usb_read_rcd_struct));
 			logi("�������");
		}
	}


}
#endif

/*2022-03-17 add by hj  
�ص��޸ĵĺ�������:
   1. tr9_Driving_record();
   2. tr9_Export_state();
   3. tr9_6051_task();
   4. rcd_send_data();
   5. menu_process.c �ļ��� ���� extern rk_6033_task_t  rk_6033_task; ����
               ͬʱ �ڰ���ȷ�ϼ� �� rk_6033_task_t.u_read_flag =2;
   6. ���� tr9_6066_task ����Э��,Ϊ�����������ʻ��¼����            
   6. �� main()������,����һ��50ms ������,������u �̴������            
   7. �޸Ĳ��� ��ͨ�� 2022-03-17 ������
   
��ʻ��¼�����ݴ�����Ҫע��һ�¼���
   1. ���ļ���Ҫ��Ӧ�� ����Ҫ��
   2. ���촦��, ���·���ȡ��ʻ��¼������ʱ,�����·� ��ʼʱ��ͽ���ʱ��
   3. ��ʻ��¼���������ͨ��RK �ϴ���, ��USB��ȡ���ݺ�ACCON�����ȡ����ʱ,����ͨ��RK����ת
      �ʱ������㣺 6033Э��Ҫ�� ������һ����ǰ�������ܰ��� 
   4. ������ʻ��¼�Ǳ���ʱ��ʱ��Ϊ16���Ƶ�ʱ�����,�γ�19056����ʱ,ʱ�����������ת����BCD�롣
   5. �����ȡ��ʻ��¼�ǵ�ָ��û�д�ʱ�����ʱ,MCUĬ���Ǵ������е�����, 
   6. �ص��˽� subp_rcd_p() ���˺�����   
*/

void Rk_Driving_Record_Up(u8 cmd) {
    u8 buf_t_1[RCD_COM_MAX_SIZE];
    u8 msg_t_1[RCD_COM_MAX_SIZE];
    /*step.1 ���û���յ� 6051 ������ȷ�ϼ� �򲻽��д���*/
    /*       ����LED��ȷ�ϼ�������������, ����ȷ�ϼ�������,��Ҫ�� tr9_6033_task.u_read_flag = 1*/
    /*       �����ж�; �������ȷ�ϼ�ʱ,tr9_6033_task.u_read_flag = 1 ,���øñ���=2*/
    /*       rk_6033_task.u_read_flag = 0x10 Ϊ������ȡ CMD�ı�־ */
    //if((rk_6033_task.u_read_flag != 0x02)&&(rk_6033_task.u_read_flag != 0x10))
    //	return;
    if (rk_6033_task.u_read_flag != 0x02) {     //����������
        if (rk_6033_task.u_read_flag != 0x10) { //������CMD
            return;
        }
    }

/*step.2 �������ʧ�� �� ��ʱ��Ӧ��, ���ش�*/
/*�����Ǵ���δӦ�����մ����Ӧ�� �����ط��Ĵ��� ����ӳ�Ϊ250ms*/
/*��Ҫע��: ������� û���յ���ȷӦ��,�ǲ��������ߵ�, ��Ϊu_send_flag��λ��,ֻ����ȷӦ�𲿷ֲ�����*/
#if 1
    //if(rk_6033_task.u_send_flag && ((jiffies - rk_6033_task.u8_delay )>250))//2022-03-31 modify
    //if(rk_6033_task.u_send_flag && ((jiffies - rk_6033_task.u8_delay )>1000))
    if (rk_6033_task.u_send_flag) {
        {                                                 //�ط���usb_rsp ����ṹ����ȫ�ֱ���,ֻ�з��ͳɹ�������ɺ�����㡣
            if (Rk_Task_Manage.RK_nack_only_delay == 1) { //����Ӧ��, ����
                rk_6033_task.u_send_flag = 0;
                if (Rk_Task_Manage.RK_read_all == 0)
                //if(rk_6033_task.u_read_flag == 0x10)
                { //������
                    if (cmd >= 0x08) {
                        if (rk_6033_task.idx >= (rk_6033_task.packet - 1)) { //����ְ��������� �ܰ����� ,��������һ��cmd�Ķ�ȡ
                            {                                                //�������ݴ������, ���յ���ȷӦ��
                                tr9_show = true;                             //�ڵ���״̬��ʾ������������
                                Ack_Export = false;
                                Export_finish = 1; //��������
                                tr9_6033_task.state = 0;
                                Rk_Task_Manage.RK_1717_state = false;
                                Rk_Task_Manage.RK_6051_state = false;
                                Rk_Task_Manage.RK_record_print_log = false;
                                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                                Rk_Task_Manage.RK_nack_only_delay = 0;
                                Rk_Task_Manage.RK_time_rang_NG_flag = 0;
                                return;
                            }

                        } else { //������һ���ְ��Ĵ���
                            rk_6033_task.idx++;
                            rk_6033_task.u_send_flag = 0;
                            rk_6033_task.u8_delay = 0;
                        }
                    } else {
                        tr9_show = true; //�ڵ���״̬��ʾ������������
                        Ack_Export = false;
                        Export_finish = 1; //��������
                        tr9_6033_task.state = 0;
                        Rk_Task_Manage.RK_1717_state = false;
                        Rk_Task_Manage.RK_6051_state = false;
                        Rk_Task_Manage.RK_record_print_log = false;
                        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                        Rk_Task_Manage.RK_nack_only_delay = 0;
                        Rk_Task_Manage.RK_time_rang_NG_flag = 0;
                        return;
                    }
                } else { //������
                    if (cmd >= 0x08) {
                        if (rk_6033_task.idx >= (rk_6033_task.packet - 1)) { //����ְ��������� �ܰ����� ,��������һ��cmd�Ķ�ȡ
                            {                                                //�������ݴ������, ���յ���ȷӦ��

                                if (cmd == 0x15) {
                                    tr9_show = true; //�ڵ���״̬��ʾ������������
                                    Ack_Export = false;
                                    Export_finish = 1; //��������
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

                                rk_6033_task.u_read_flag = 0x10; // ������
                                rk_6033_task.Data_Direction = 1; //���ݷ���
                                //Rk_Task_Manage.RK_nack_only_delay = 1;//��������RKӦ���־
                                Rk_Task_Manage.RK_time_rang_NG_flag = 1;
                                Rk_Task_Manage.RK_read_all = 1;
                                rk_6033_task.u_send_flag = 0;
                                rk_6033_task.u8_delay = 0;
                                //tr9_show = true  ;
                                return;
                            }

                        } else { //������һ���ְ��Ĵ���
                            rk_6033_task.idx++;
                            rk_6033_task.u_send_flag = 0;
                            rk_6033_task.u8_delay = 0;
                        }
                    } else {
                        _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
                        rk_6033_task.cmd = cmd; //��ʼ�� cmd = 0x00�ȴ� ��ȷ�ϡ������¡�
                        rk_6033_task.cmd++;
                        rk_6033_task.u_read_flag = 0x10;       // ������
                        rk_6033_task.Data_Direction = 1;       //���ݷ���
                        Rk_Task_Manage.RK_nack_only_delay = 1; //��������RKӦ���־
                        Rk_Task_Manage.RK_time_rang_NG_flag = 1;
                        Rk_Task_Manage.RK_read_all = 1;
                        return;
                    }
                }
            } else {                                            //��ҪӦ��
                if ((jiffies - rk_6033_task.u8_delay) > 2500) { //�ط�
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

    /*step.4 ��֯����跢�͵�����, ע��: ���͵� cmd �����յ� 6033����ȷӦ��� �Ž�����һ��cmd�Ĵ���*/
    switch (cmd) {
    case 0x00:                               //�ɼ���¼��ִ�б�׼�汾
    case 0x01:                               //�ɼ���ǰ��ʻ����Ϣ
    case 0x02:                               //�ɼ���¼��ʵʱʱ��
    case 0x03:                               //�ɼ��ۼ���ʻ���
    case 0x04:                               //�ɼ���¼������ϵ��
    case 0x05:                               //�ɼ�������Ϣ
    case 0x06:                               //�ɼ���¼��״̬�ź�������Ϣ
    case 0x07:                               //�ɼ���¼��Ψһ�Ա��
    {                                        // ע������������ ԭ���Ĵ���
        Rk_Task_Manage.RK_1717_state = true; //��ʱ�ر�1717�ķ���
        tr9_show = false;                    //�رմ�������������ʾ
        //Rk_Task_Manage.RK_nack_only_delay = 1;
        if (rk_6033_task.u_read_flag != 0x02)
            rk_6033_task.u_read_flag = 0x10;
        rk_6033_task.u_send_flag = 1;
        rk_6033_task.u8_delay = jiffies;
        rk_6033_task.idx = 0;                         //��ǰ��
        rk_6033_task.packet = 1;                      //�ܰ���
        rcd_communicate_parse(FROM_U4, cmd, &cmd, 1); //2012��ʻ��¼��Э��
    } break;

    case 0x08: //48Сʱ rcd48_m  T_RCD_48
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd48_m.head.node == rcd48_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;

                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: rk_6033_task.frame=800/126 = 6���ڵ� */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            rk_6033_task.frame = 4;     //2022-03-31 modify by hj
            //rk_6033_task.frame = 3;
            //get �ڵ����
            index_t = subp_rcd_p(rcd48_m.head.node, rcd48_m.tail.node, RCD48_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                  //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd48_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd48_m.tail.node;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_48;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            rk_6033_task.head = decp_rcd_p(T_RCD_48, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        /*                           ���rsp.id �ڳ�����ز�����δʹ��*/
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;

    case 0x09: //�ɼ�ָ����λ����Ϣ��¼  360Сʱ ���ݲɼ� rcd360_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd360_m.head.node == rcd360_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd = rk_6033_task.cmd + 7;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: 800/666 = 1���ڵ�                    */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            rk_6033_task.frame = 1;
            //get �ڵ����
            index_t = subp_rcd_p(rcd360_m.head.node, rcd360_m.tail.node, RCD360_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                   //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd360_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd360_m.tail.node;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_360;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            rk_6033_task.head = decp_rcd_p(T_RCD_360, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x10: //�ɼ�ָ�����¹��ɵ��¼  T_RCD_ACCIDENT rcd_accident_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd_accident_m.head.node == rcd_accident_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: 800/234 = 3���ڵ� ����ռ��800�ֽ�    */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            rk_6033_task.frame = 2;     //2022-03-31 modify by hj
            //rk_6033_task.frame = 2;
            //get �ڵ����
            index_t = subp_rcd_p(rcd_accident_m.head.node, rcd_accident_m.tail.node, RCD_ACCIDENT_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                         //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd_accident_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd_accident_m.tail.node;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_ACCIDENT;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            rk_6033_task.head = decp_rcd_p(T_RCD_ACCIDENT, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x11: //�ɼ�ָ���ĳ�ʱ��ʻ��¼ T_RCD_DRIVER_OT   rcd_ot_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd_ot_m.head.node == rcd_ot_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: 800/50 = 15���ڵ� ����ռ��800�ֽ�    */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            rk_6033_task.frame = 12;    //2022-03-31 modify by hj
            //rk_6033_task.frame = 8;
            //get �ڵ����
            index_t = subp_rcd_p(rcd_ot_m.head.node, rcd_ot_m.tail.node, RCD_OT_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                   //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd_ot_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd_ot_m.tail.node;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_DRIVER_OT;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            rk_6033_task.head = decp_rcd_p(T_RCD_DRIVER_OT, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x12: //�ɼ�ָ���ļ�ʻ����ݼ�¼      T_RCD_DRIVER_LOG       rcd_driver_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd_driver_m.head.node == rcd_driver_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: 800/25 = 31���ڵ� ����ռ��800�ֽ�    */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            //rk_6033_task.frame = 31;//2022-03-31 modify by hj
            rk_6033_task.frame = 20;
            //get �ڵ����
            index_t = subp_rcd_p(rcd_driver_m.head.node, rcd_driver_m.tail.node, RCD_DRIVER_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                       //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd_driver_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd_driver_m.tail.node;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_DRIVER_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            rk_6033_task.head = decp_rcd_p(T_RCD_DRIVER_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x13: //�ɼ�ָ�����ⲿ�����¼    T_RCD_POWER_LOG  rcd_power_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd_power_m.head.node == rcd_power_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: 800/7 = 113���ڵ� ����ռ��800�ֽ�    */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            //rk_6033_task.frame = 113; //2022-03-31 modify by hj
            rk_6033_task.frame = 80;
            //get �ڵ����
            index_t = subp_rcd_p(rcd_power_m.head.node, rcd_power_m.tail.node, RCD_POWER_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                      //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd_power_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd_power_m.tail.node;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_POWER_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            rk_6033_task.head = decp_rcd_p(T_RCD_POWER_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x14: //�ɼ�ָ���Ĳ����޸ļ�¼    T_RCD_PARAMETER_LOG rcd_para_m
    {
        u16 index_t = 0;
        u8 i = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd_para_m.head.node == rcd_para_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: 800/7 = 113���ڵ� ����ռ��800�ֽ�    */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            //rk_6033_task.frame = 113;//2022-03-31 modify by hj
            rk_6033_task.frame = 80;
            //get �ڵ����
            index_t = subp_rcd_p(rcd_para_m.head.node, rcd_para_m.tail.node, RCD_PARA_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                     //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd_para_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd_para_m.tail.node;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_PARAMETER_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - 1, &usb_rsp.rcdDataBuf[usb_rsp.len]);

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            rk_6033_task.head = decp_rcd_p(T_RCD_PARAMETER_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    case 0x15: //�ɼ�ָ�����ٶ�״̬��־   T_RCD_SPEED_LOG  rcd_speed_m
    {
        u16 index_t = 0;
        u8 i = 0;
        u8 j = 0;
//        u8 t_time[6] = {0};
        usb_rsp.len = 0;

        /*���ͷ��βָ�����, ��ʾ�޼�¼,����Ҫת����һ��cmd */
        if (rcd_speed_m.head.node == rcd_speed_m.tail.node) {
            rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, 0); //2022-03-30
            logd("�޼�¼ cmd = 0x%02X", rk_6033_task.cmd);
            if (Rk_Task_Manage.RK_read_all == 0)
            //if(rk_6033_task.u_read_flag == 0x10)
            {
                _memset((u8 *)&rk_6033_task, 0x00, sizeof(rk_6033_task));
            } else {
                //rk_6033_task.cmd ++;
                /*���в�����0, ������һ��cmd ����*/
                rk_6033_task.packet = 0;
                rk_6033_task.idx = 0;
                rk_6033_task.head = 0;
                rk_6033_task.tail = 0;
                //rk_6033_task.u_send_flag = 0;
                //rk_6033_task.u8_delay = 0;
                rk_6033_task.u_send_flag = 1;    //�÷��ͱ�־
                rk_6033_task.u8_delay = jiffies; //���õȴ�Ӧ���ӳ�
            }
            break;
        }

        _memset((u8 *)&usb_rsp, 0x00, sizeof(usb_rsp));
        /*ȡ�ڵ�����, ÿ���ڵ��СΪ128byte, ÿ�η��ͻ��������Ϊ800 */
        /*��ÿ�η��͵Ľڵ���Ϊ: 800/133= 6���ڵ� ����ռ��800�ֽ�     */
        /*������Ҫ�ǰ�6033Э������ܰ�������                         */
        if (rk_6033_task.packet == 0) { //����Ѿ�get �ڵ����� �������ⲽ, packet ��һ�ν���֮ǰ�Ѿ�����0��
            //rk_6033_task.frame = 6;//2022-03-31 modify by hj
            rk_6033_task.frame = 5;
            //get �ڵ����
            index_t = subp_rcd_p(rcd_speed_m.head.node, rcd_speed_m.tail.node, RCD_SPEED_NODE_MAX_SIZE);
            if ((index_t % rk_6033_task.frame) == 0) {
                rk_6033_task.packet = index_t / rk_6033_task.frame; //�����ܰ���
            } else {
                rk_6033_task.packet = index_t / rk_6033_task.frame; // ������ �ܰ���Ҫ��һ������
                rk_6033_task.packet++;                              //��ʣ���
            }
            rk_6033_task.idx = 0;                      //�ְ���ʼ����0��ʼ
            rk_6033_task.head = rcd_speed_m.head.node; //���浱ǰ�� ͷβָ�롣
            rk_6033_task.tail = rcd_speed_m.tail.node;
            next_speed_log_cnt = 0;
            next_speed_log_offset = 0;
        }

        /*������֯: ��Ҫ���ǰ�����ʱ�������֯,������***��ͷ��β***������֯ */
        /*ͬʱÿ���ڵ��ʱ����Ҫ����BCD��ת��                               */
        /*���6033 �ķְ������ܰ����Ĵ����� 6033��Ӧ���н��д���            */
        /*  �� rk_6033_task.packet �� rk_6033_task.idx                      */
        for (i = 0; i < rk_6033_task.frame; i++) {
            if (rk_6033_task.head == rk_6033_task.tail)
                break;

#if 1
            //if(next_speed_log_offset == 0)
            if (next_speed_log_cnt == 0) {
                //logd("��¼15H  index_t=[%d] rk_6033_task.head=[%d] rk_6033_task.tail=[%d]",index_t,rk_6033_task.head,rk_6033_task.tail);
                spi_flash_read(msg_t_1, RSPEED_ADDRD(rk_6033_task.head), 7);
                rk_6033_task.head_offset = rk_6033_task.head;
                //logd("ʱ��: %02x-%02x-%02x %02x:%02x:%02x",msg_t_1[1],msg_t_1[2],msg_t_1[3],msg_t_1[4],msg_t_1[5],msg_t_1[6]);
                for (j = 1; j < 5; j++) { //ÿ���ظ��������5��

                    if ((rk_6033_task.head - j) == rk_6033_task.tail) {
                        //logd("����β, �˳�");
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

//logd("��ѯͬһ��:��¼15H  next_speed_log_offset=[%d]  next_speed_log_cnt=[%d]",next_speed_log_offset,next_speed_log_cnt);
//spi_flash_read(msg_t, RSPEED_ADDRD(index_t - next_speed_log_offset ), 133);
#endif

            /*ע��: rcd_assemble_body �����и�������:rsp.type ,��Ϊ��֤����ִ����Ҫ����ֵrsp.type*/
            rsp.type = T_RCD_SPEED_LOG;
            usb_rsp.len += rcd_assemble_body(rk_6033_task.head - next_speed_log_offset, &usb_rsp.rcdDataBuf[usb_rsp.len]);
//usb_rsp.len += rcd_assemble_body(rk_6033_task.head, &usb_rsp.rcdDataBuf[usb_rsp.len]);
/*2022-04-21 ����ʱ������*/

//logd("��¼15H  next_speed_log_offset=[%d]  next_speed_log_cnt=[%d]",next_speed_log_offset,next_speed_log_cnt);
#if 1
            if (next_speed_log_offset > 0) {
                next_speed_log_offset--;
                //logd("...... �������� .....1");
            } else {
                //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
                //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
                //if(next_speed_log_cnt = 0)
                {
                    if (next_speed_log_cnt == 0) {
                        rk_6033_task.head = decp_rcd_p(T_RCD_SPEED_LOG, rk_6033_task.head, 1);
                        next_speed_log_offset = 0;
                        next_speed_log_cnt = 0;
                        //logd("...... �������� .....2");
                    } else {
                        rk_6033_task.head = decp_rcd_p(T_RCD_SPEED_LOG, rk_6033_task.head, next_speed_log_cnt + 1);
                        next_speed_log_offset = 0;
                        next_speed_log_cnt = 0;
                        //logd("...... �������� .....3");
                    }
                }
            }
#endif

            //ע�⣺ decp_rcd_p��������Ѿ�������ָ�����������,�ʵ�β����ͷʱ,Ҳ��������ָ�롣
            //Ĭ�ϴ����µ��������´�,����ͷ��ʼ
            //rk_6033_task.head = decp_rcd_p(T_RCD_SPEED_LOG, rk_6033_task.head, 1);
        }

        rk_6033_task.u_send_flag = 1;         //�÷��ͱ�־
        rk_6033_task.u8_delay = jiffies;      //���õȴ�Ӧ���ӳ�
        if (usb_rsp.len >= FRAME_DATA_SIZE) { //�����ݴ�
            usb_rsp.len = FRAME_DATA_SIZE;
        }
        rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);

    } break;
    }
}

#endif
