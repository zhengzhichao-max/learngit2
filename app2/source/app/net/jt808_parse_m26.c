/**
  ******************************************************************************
  * @file    jt808_parse.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   解析GPRS数据
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

#include "include_all.h"

enum net_step {
    NET_IDLE,
    NET_COPY,
    NET_WAIT_ENDING,
    NET_END
};

static enum net_step net_step = NET_IDLE; //指针函数管理
static bool new_step = false;             //函数跳转
static u16 org_len = 0;                   //原始GPRS 数据长度

/****************************************************************************
* 名称:    get_net_7e_cnt ()
* 功能：获得标志位个数
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u8 get_net_7e_cnt(u8 *head_t, u8 *tail_t) {
    u8 n;

    n = 0;
    if (head_t == NULL || tail_t == NULL) {
        return n;
    }

    while (head_t != tail_t) {
        if (*tail_t == 0x7e) {
            n++;
        }
        incp_gprs_rev(tail_t, 1);
    }

    return n;
}

/****************************************************************************
* 名称:    get_net_7e_cnt ()
* 功能：获得对应标志位的指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u8 *get_net_7e_pointer(u8 *head_t, u8 *tail_t) {
    u8 *p;

    p = NULL;

    if (head_t == NULL || tail_t == NULL) {
        return p;
    }

    while (head_t != tail_t) {
        if (*tail_t == 0x7e) {
            return tail_t;
        }
        incp_gprs_rev(tail_t, 1);
    }

    return p;
}

#if (GSM == M26)
#define GPRS_REV_FLAG "IPD"
#define GPRS_REV_LEN 3
#elif (GSM == SIM2000C)
#define GPRS_REV_FLAG "+RECEIVE"
#define GPRS_REV_LEN 8
#elif (GSM == SIM800 || GSM == QW200)
#define GPRS_REV_FLAG "+RECEIVE"
#define GPRS_REV_LEN 8
#else
#error "GSM == ?"
#endif

/*
MG323  %IPDATA:531, "7E
MC323  ^IPDATA:1, 20, (0x7e)
*/
/****************************************************************************
* 名称:    net_idle ()
* 功能：等待接收GPRS 数据
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void net_idle(void) {
    u8 buf[32] = {0};
    u8 *pt;

    _memset(buf, 0x00, 32);

    while (new_gsm_rev() > 20) {
        read_gsm(buf, gsm_rev_tail, 12); //GPRS_REV_LEN
        if (_strncmp(buf, GPRS_REV_FLAG, GPRS_REV_LEN) == 0) {
            incp_gsm_rev(gsm_rev_tail, GPRS_REV_LEN); // 去掉IPDATA:
            //是否带通道号
            _memcpy_len(buf, gsm_rev_tail, 3);

            read_gsm(buf, gsm_rev_tail, 5);
            buf[4] = '\0';
            org_len = 0;
            org_len = atoi((const char *)&buf);
            if (org_len == 0 || org_len > MAX_NET_PACKAGE) {
                logd("net_idle err: 0");
                incp_gsm_rev(gsm_rev_tail, 1);
                return;
            }

            ///logd("net_copy:%04d", org_len);
            pt = gsm_rev_tail;

            if ((pt = get_gsm_resp(":")) == NULL) {
                logd("net_idle err: 2");
                incp_gsm_rev(gsm_rev_tail, 1);
                return;
            }
            gsm_rev_tail = pt;
            incp_gsm_rev(gsm_rev_tail, 1);

            net_step = NET_COPY;
            return;
        } else if (_strncmp(buf, "+RECEIVE", 8) == 0) {
            incp_gsm_rev(gsm_rev_tail, 10); // 去掉IPDATA:
            //是否带通道号
            _memcpy_len(buf, gsm_rev_tail, 3);
            if (buf[1] == ',') {
                netx = buf[0] - 0x30;          //端口号
                incp_gsm_rev(gsm_rev_tail, 2); // 去掉1,
            } else if (buf[0] == ',') {
                netx = buf[1] - 0x30;          //端口号
                incp_gsm_rev(gsm_rev_tail, 3); // 去掉,1,
            }
        }
        incp_gsm_rev(gsm_rev_tail, 1);
    }
}

/****************************************************************************
* 名称:    net_copy ()
* 功能：将GSM串口的数据接收到GPRS缓存中
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void net_copy(void) {
    static u16 jbak;
    u16 flag_offset = 0;
    u16 i = 0;

    if (new_step) {
        jbak = jiffies;
    }

    if (_pasti(jbak) >= 3000) {
        logd("net_copy err: 0");
        net_step = NET_IDLE;
        return;
    }

    if (_pastn(gsm_rev_jfs) < 5)
        return;

    flag_offset = subp_gsm_rev(gsm_rev_head, gsm_rev_tail);

    if (flag_offset >= org_len) {
        if (*gsm_rev_tail != 0x7e) {
            logd("net_copy:%04d", org_len);
            logd("head:0x%02X, 0x%p", *gsm_rev_tail, gsm_rev_tail);
        }

        for (i = 0; i < org_len; i++) {
            *gprs_rev_head = *gsm_rev_tail;
            incp_gsm_rev(gsm_rev_tail, 1);
            incp_gprs_rev(gprs_rev_head, 1);
        }

        //logd("tail:0x%02X, 0x%p",*(gsm_rev_tail-1),gsm_rev_tail-1);

        net_step = NET_WAIT_ENDING;
        return;
    }
}

/****************************************************************************
* 名称:    net_wait_ending ()
* 功能：在GPRS 缓存中解析数据
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void net_wait_ending(void) {
    static u16 wbak;
    int xvalue = -1;
    u8 *pt;
    u8 flag_cnt;
    u16 len;
    u16 nc_len = 0;
    u8 nc_buf[MAX_NET_PACKAGE];

    if (new_step) {
        wbak = jiffies;
    }

    if (_pasti(wbak) >= 500) {
        logd("net_copy err: 0");
        net_step = NET_IDLE;
        return;
    }

    //找到第一个7E 的位置
    pt = get_net_7e_pointer(gprs_rev_head, gprs_rev_tail);
    if (pt == NULL) {
        clr_gprs_rev();
        logd("net_wait_ending: 1");
        goto wait_ending_out;
    }
    gprs_rev_tail = pt;

    //print_buf(nc_buf, nc_len);    //chwsh

    //统计7E 个数
    flag_cnt = get_net_7e_cnt(gprs_rev_head, gprs_rev_tail);
    if (flag_cnt >= 2) {
        //去掉第一个7E
        incp_gprs_rev(gprs_rev_tail, 1);

        //寻找下个7E的位置
        pt = get_net_7e_pointer(gprs_rev_head, gprs_rev_tail);
        len = subp_gprs_rev(pt, gprs_rev_tail) + 1;
        if (len < MIM_NET_PACKAGE || len > MAX_NET_PACKAGE) {
            logd("net_wait_ending: 2");
            return;
        }

        //不COPY 尾7E
        nc_len = len - 1;
        //已去掉头尾标示符
        _memset(nc_buf, 0x00, 32);
        read_gprs(nc_buf, gprs_rev_tail, nc_len);
        nc_len -= _del_filt_char(nc_len, &nc_buf[0]);

        xvalue = get_check_xor(nc_buf, 0, nc_len - 1);
        if (xvalue != nc_buf[nc_len - 1]) {
            logd("net_wait_ending: 3");
            clr_gprs_rev(); //多包问题处理 2014-09-09
            return;
        }

        jt808_net_parse(nc_buf, nc_len);
        incp_gprs_rev(gprs_rev_tail, len);

        if (gprs_rev_head == gprs_rev_tail) {
            goto wait_ending_out;
        }
    } else {
    wait_ending_out:
        net_step = NET_IDLE;
        return;
    }
}

static funcp const NetTaskTab[] =
    {
        net_idle,
        net_copy,
        net_wait_ending,
};

/*
const u8 debug_ZTC_dbx_area[63] = 
{0x8F,0x08,0x00,0x32,0x06,0x48,0x89,0x28,0x11,0x23,0x0B,0x91,0x8F,0x08,0x00,0x00,0x3B,0xB3,0x01,0x00,0x00,0x05,0x07,0x1C,0x46,0x30,0x01,0x8D,0x3A,0x6E,0x07,0x1C,0x44,0x80,0x01,0x8D,0x39,0x1A,0x07,0x1C,0x47,0x00,0x01,0x8D,0x37,0x12,0x07,0x1C,0x48,0xA0,0x01,0x8D,0x38,0x66,0x07,0x1C,0x46,0x30,0x01,0x8D,0x3A,0x6E,0x5D};

//const u8 debug_ZTC_dbx_area[87] = 
//{0x8F,0x0B,0x00,0x4A,0x01,0x38,0x00,0x00,0x22,0x22,0x49,0x4C,0x8F,0x0B,0x00,0x00,0x67,0x3B,0x64,0x14,0x00,0x08,0x07,0x1C,0x76,0x78,0x01,0x8D,0xAE,0x1C,0x07,0x1C,0x77,0x78,0x01,0x8D,0xB1,0xF2,0x07,0x1C,0x79,0xE8,0x01,0x8D,0xB3,0x88,0x07,0x1C,0x7F,0x88,0x01,0x8D,0xB3,0x9C,0x07,0x1C,0x7F,0xF0,0x01,0x8D,0xAE,0x56,0x07,0x1C,0x80,0x00,0x01,0x8D,0xAA,0xB8,0x07,0x1C,0x7C,0x80,0x01,0x8D,0xAA,0x44,0x07,0x1C,0x78,0xA8,0x01,0x8D,0xA8,0x76,0x29};


//const u8 debug_ZTC_add_area[87] = 
//{0x8F,0x0B,0x00,0x4A,0x01,0x38,0x00,0x00,0x22,0x22,0x49,0x4C,0x8F,0x0B,0x00,0x00,0x67,0x3B,0x64,0x14,0x00,0x08,0x07,0x1C,0x76,0x78,0x01,0x8D,0xAE,0x1C,0x07,0x1C,0x77,0x78,0x01,0x8D,0xB1,0xF2,0x07,0x1C,0x79,0xE8,0x01,0x8D,0xB3,0x88,0x07,0x1C,0x7F,0x88,0x01,0x8D,0xB3,0x9C,0x07,0x1C,0x7F,0xF0,0x01,0x8D,0xAE,0x56,0x07,0x1C,0x80,0x00,0x01,0x8D,0xAA,0xB8,0x07,0x1C,0x7C,0x80,0x01,0x8D,0xAA,0x44,0x07,0x1C,0x78,0xA8,0x01,0x8D,0xA8,0x76,0x29};

const u8 debug_ZTC_add_area[145] =   
{0x8F,0x08,0x00,0x3A,0x01,0x38,0x00,0x00,0x22,0x22,0x46,0x18,0x8F,0x08,0x00,0x00,0x67,0x3A,0x05,0x00,0x00,0x06,0x07,0x1C,0x7C,0x10,0x01,0x8D,0xAF,0xC6,0x07,0x1C,0x7E,0x70,0x01,0x8D,0xAF,0xCE,0x07,0x1C,0x81,0x20,0x01,0x8D,0xAF,0xD8,0x07,0x1C,0x81,0x58,0x01,0x8D,0xAD,0x8C,0x07,0x1C,0x7E,0x68,0x01,0x8D,0xCB,0x38,0x07,0x1C,0x79,0xE0,0x01,0x8D,0xC8,0x7E,0x14};


const u8 debug_ZTC_del_area[21] =	
{0x8f,0x09,0x00,0x15,0x01,0x38,0x00,0x00,0x22,0x22,0x00,0x5F,0x8f,0x09,0x01,0x01,0x00,0x00,0x67,0x3A,0xff};
*/

/*********************************************************
名    称：gprs_parse_task
功    能：GPRS 数据解析任务
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
void gprs_parse_task(void) {
    static bool debug_tmp = false;
    enum net_step step;

    if (gs.gprs.online_data) {
        step = net_step;
        (*NetTaskTab[net_step % NET_END])();
        new_step = (net_step == step) ? false : true;
    }
    /*
	if( (tick == 55) && ( false == debug_tmp ) )
	{
		start_printf_road_data();
		debug_tmp = true;
	}
	*/
    /*
	if( (tick == 60) && ( true == debug_tmp ) )
	{
		jt808_net_parse( (u8*)&debug_ZTC_add_area, 145);
		debug_tmp = false;	
	}

	*/
}
