/**
  ******************************************************************************
  * @file    gsm_drv.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GSM 串口底层驱动
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

u8 gsm_rev_buf[GSM_REV_SIZE]; //接收来自串口的所有数据
u8 *gsm_rev_head = gsm_rev_buf;
u8 *gsm_rev_tail = gsm_rev_buf;

u8 gsm_send_buf[GSM_SEND_SIZE]; //发送AT命令
u8 *gsm_send_head = gsm_send_buf;
u8 *gsm_send_tail = gsm_send_buf;

u8 gprs_send_buf[GPRS_SEND_SIZE]; //用于GPRS 发送数据
u8 *gprs_send_head = gprs_send_buf;
u8 *gprs_send_tail = gprs_send_buf;

u8 gprs_rev_buf[GPRS_REV_SIZE]; //接收来自串口的所有数据
u8 *gprs_rev_head = gprs_rev_buf;
u8 *gprs_rev_tail = gprs_rev_buf;

u8 gprs2_send_buf[GPRS_SEND_SIZE]; //用于GPRS 发送数据
u8 *gprs2_send_head = gprs2_send_buf;
u8 *gprs2_send_tail = gprs2_send_buf;

u8 gprs3_send_buf[GPRS_SEND_SIZE]; //用于GPRS 发送数据
u8 *gprs3_send_head = gprs3_send_buf;
u8 *gprs3_send_tail = gprs3_send_buf;

u8 netx = 0; //网络端口号
bool gsm_send_busy = false;
u32 gsm_rev_jfs = 0;
u32 gsm_send_jfs = 0;

u16 gprs_lac = 0;
u16 gprs_celli = 0;

sAtEngine AtEngine1;
sAtEngine AtEngine2;
sAtEngine AtEngine3;

/**************************************************************************
************************     输出调试信息     *****************************
**************************************************************************/
//HTTP:
void sync_net_online(void) {
    if (gs.gprs1.stat >= NET_ONLINE) {
        gc.gprs = NET_ONLINE;
    }
    if (gs.gprs2.stat >= NET_ONLINE) {
        gc.gprs = NET_ONLINE;
    }
    if (gs.gprs3.stat >= NET_ONLINE) {
        gc.gprs = NET_ONLINE;
    }
}

bool login_net_123(void) {
    if (gs.gprs1.login) {
        return true;
    }
    if (gs.gprs2.login) {
        return true;
    }
    if (gs.gprs3.login) {
        return true;
    }
    return false;
}

void sync_net_offline(void) {
    gs.gprs1.stat = NET_LOST;
    gs.gprs2.stat = NET_LOST;
    gs.gprs3.stat = NET_LOST;
    gs.gprs1.login = false;
    gs.gprs2.login = false;
    gs.gprs3.login = false;
}

/****************************************************************************
* 名称:    write_gsm_txt ()
* 功能：串口数据发送，发送字符串
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void write_gsm_txt(u8 *strp) {
    u16 i;

    for (i = 0; i < 400; i++) {
        if (*strp == '\0')
            break;
        if (over_gsm_send())
            break;

        *gsm_send_head = *strp++;
        incp_gsm_send(gsm_send_head, 1);
    }
}

/****************************************************************************
* 名称:    write_gsm ()
* 功能：串口数据发送，带长度，任意字符
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void write_gsm(u8 *buf, u16 len) {
    u16 i;

    for (i = 0; i < len; i++) {
        if (over_gsm_send())
            break;

        *gsm_send_head = *buf++;
        incp_gsm_send(gsm_send_head, 1);
    }
}

/****************************************************************************
* 名称:    write_gsm_hex ()
* 功能：串口数据发送，单个任意字符
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void write_gsm_hex(u8 hex) {
    if (over_gsm_send())
        return;
    *gsm_send_head = hex;
    incp_gsm_send(gsm_send_head, 1);
}

/****************************************************************************
* 名称:    read_gsm ()
* 功能：串口数据接收
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void read_gsm(u8 *buf, u8 *rbuf_ptr, u16 n) {
    while (n--) {
        *buf++ = *rbuf_ptr;
        incp_gsm_rev(rbuf_ptr, 1);
    }
}

/****************************************************************************
* 名称:    read_gprs ()
* 功能：GPRS数据接收，已从GSM 串口中解析，并存入GPRS 数据BUF中
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void read_gprs(u8 *buf, u8 *rbuf_ptr, u16 n) {
    while (n--) {
        *buf++ = *rbuf_ptr;
        incp_gprs_rev(rbuf_ptr, 1);
    }
}

/****************************************************************************
* 名称:    printf_gsm ()
* 功能：打印数据到GSM串口
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
int printf_gsm(uc8 *fmt, ...) {
    int n;
    char buf[400] = {0};
    va_list args;

    va_start(args, fmt);
    n = vsprintf(buf, (const char *)fmt, args);
    write_gsm_txt((u8 *)&buf);
    va_end(args);

#if (0)
    if (u1_m.cur.b.debug == true) {
        print_buf((u8 *)&buf, n);
    }
#else
    logd("%s", buf);
#endif

    return n;
}

/****************************************************************************
* 名称:    get_gsm_resp ()
* 功能：在GSM BUF中，查找指定字符串
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
u8 *get_gsm_resp(u8 *str) {
    u16 i = 0;
    u8 *pt;
    u8 *tail = gsm_rev_tail;

    if (*str == '\0'){
        return gsm_rev_tail;
    }

    while (tail != gsm_rev_head) {
        pt = tail;

        for (i = 0; *pt == *(str + i); i++) {
            if (*(str + i + 1) == '\0'){
                return tail;
            }

            incp_gsm_rev(pt, 1);
        }

        incp_gsm_rev(tail, 1);
    }
	
    return NULL;
}

/****************************************************************************
* 名称:    get_gsm_field ()
* 功能：截取指定标志位的信息，并返回对应指针
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
u8 *get_gsm_field(u8 n) {
    u8 *pt = gsm_rev_tail;
    u8 mark = 0;

    if (n == 0) return gsm_rev_tail;
    while (pt != gsm_rev_head) {
        if (*pt == '"')
            mark = ~mark;
        if (!mark && *pt == ',')
            n--;

        incp_gsm_rev(pt, 1);

        if (n == 0)
            return pt;
    }
    return NULL;
}

/****************************************************************************
* 名称:    get_gsm_line ()
* 功能：截取指定行
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
u8 *get_gsm_line(u8 n) {
    u8 *pt = gsm_rev_tail;

    while (pt != gsm_rev_head) {
        if (*pt == 0x0A)
            n--;

        incp_gsm_rev(pt, 1);

        if (n == 0)
            return pt;
    }
    return NULL;
}

/****************************************************************************
* 名称:    get_gsm_line ()
* 功能：截取指定行
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
/*
u8* get_domain_line(void)
{
	u8* pt = gsm_rev_tail;
	u8  buf[50]={0};
	
	while (pt != gsm_rev_head)
	{
		if (*pt == 0x0A)
		{
			read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
			buf[sizeof(buf) - 1] = '\0';
			
			if ((pt = _strstr(buf, "+CDNSGIP")) != NULL) 	
				return pt;
			else
			{
				clr_gsm_rev();
				return NULL;
			}	
		}
		incp_gsm_rev(pt, 1);

		if (pt == 0)
			return NULL;
	}
	return NULL;
}
*/

/****************************************************************************
* 名称:    search_gsm_recv ()
* 功能：在指定的区间，查找指定字符串
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
u8 *search_gsm_recv(u8 *tail, u8 *head, u8 *str) {
    u16 i;
    u8 *pt;

    if (*str == '\0')
        return tail;

    while (tail != head) {
        pt = tail;
        for (i = 0; *pt == *(str + i); i++) {
            if (*(str + i + 1) == '\0')
                return tail;
            incp_gsm_rev(pt, 1);
        }
        incp_gsm_rev(tail, 1);
    }
    return NULL;
}

/****************************************************************************
* 名称:    gsm_drv_tran_task ()
* 功能：GSM 串口发送任务
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void gsm_drv_tran_task(void) {
    if (gsm_send_tail != gsm_send_head && gsm_send_busy == false) //
    {
        gsm_send_jfs = jiffies;
        gsm_send_busy = true;
        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

        //print_buf(buf, n);	  //chwsh
    }

    if (gsm_send_tail != gsm_send_head && gsm_send_busy == true && _pastn(gsm_send_jfs) > 200) {
        gsm_send_busy = false; //超时，清除标志
    }
}

////////////////////////////////////////////////////////////////////
//转换中文到 英文模式
//判断双字；前面字节为0 的去掉
u8 pdu_sms_to_txt(u8 *str, u16 len) {
    u16 i;
    u16 j = 0;
    u8 *pf1, *pf2;

    //len = _strlen(str);

    if (len > 200)
        return 0;
    pf1 = str;
    pf2 = str;
    for (i = 0; i < len; i++) {
        if (*pf1 != 0x00) {
            *pf2++ = *pf1;
            j++;
        }
        pf1++;
    }
    return j;
}

//查找字符在字符串中的位置
void AtStringFind1(u8 *str1, u8 *str2, u8 *str3) {
    AtEngine1.catchstr1 = str1;
    AtEngine1.catchstr2 = str2;
    AtEngine1.catchstr3 = str3;
    AtEngine1.exe_status = STATUS_RUNNING;
}

//查找字符在字符串中的位置
void AtStringFind2(u8 *str1, u8 *str2, u8 *str3) {
    AtEngine2.catchstr1 = str1;
    AtEngine2.catchstr2 = str2;
    AtEngine2.catchstr3 = str3;
    AtEngine2.exe_status = STATUS_RUNNING;
}

//查找字符在字符串中的位置
void AtStringFind3(u8 *str1, u8 *str2, u8 *str3) {
    AtEngine3.catchstr1 = str1;
    AtEngine3.catchstr2 = str2;
    AtEngine3.catchstr3 = str3;
    AtEngine3.exe_status = STATUS_RUNNING;
}
