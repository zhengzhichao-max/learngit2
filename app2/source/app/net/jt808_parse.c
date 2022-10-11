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


#if  (GSM == EC20)
#define GPRS_REV_FLAG           "+QIURC"
#define GPRS_REV_LEN            6
//#define	GPRS_REV_FLAG			"+RECEIVE"
//#define	GPRS_REV_LEN			8

#elif ( GSM == SLM790 )

#define GPRS_REV_FLAG           "+MIPRTCP"
#define GPRS_REV_LEN            8

#else
#error "GSM == ?"

#endif


enum net_step
{
    NET_IDLE,
    NET_COPY,
    NET_WAIT_ENDING,
    NET_END
};

static enum net_step net_step = NET_IDLE; //指针函数管理
static bool new_step = false; //函数跳转
static u16 org_len = 0; //原始GPRS 数据长度

/****************************************************************************
* 名称:    get_net_7e_cnt ()
* 功能：获得标志位个数
* 入口参数：无
* 出口参数：无
****************************************************************************/
static u8 get_net_7e_cnt(u8 * head_t , u8 * tail_t)
{
    u8 n;

    n = 0;
    if (head_t == NULL || tail_t == NULL)
    {
        return n;
    }

    while (head_t != tail_t)
    {
        if (*tail_t == 0x7e)
        {
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
static u8 * get_net_7e_pointer(u8 * head_t , u8 * tail_t)
{
    u8 * p;

    p = NULL;

    if (head_t == NULL || tail_t == NULL)
    {
        return p;
    }

    while (head_t != tail_t)
    {
        if (*tail_t == 0x7e)
        {
            return  tail_t;
        }
        incp_gprs_rev(tail_t, 1);
    }

    return p;
}




/*
+QIURC: "recv",1,20
*/
/****************************************************************************
* 名称:    net_idle ()
* 功能：等待接收GPRS 数据
* 入口参数：无
* 出口参数：无
****************************************************************************/
#if (GSM == EC20)
static void net_idle(void)
{
    u8 buf[32] = {0};
    u8 * pt;


    while (new_gsm_rev() > 21)
    {
        _memset(buf, 0x00, 32);
        read_gsm(buf, gsm_rev_tail, GPRS_REV_LEN);
        if (_strncmp(buf, GPRS_REV_FLAG, GPRS_REV_LEN) == 0)//查找关键字 +QIURC:"recv",0,530
        {

            incp_gsm_rev(gsm_rev_tail, GPRS_REV_LEN+8);       //去掉关键字 +QIURC:"recev"   
            //是否带通道号
            _memcpy_len(buf,  gsm_rev_tail, 3);
            if (buf[1] == ',')
            {
                netx = buf[0] - 0x30;                   //端口号
                incp_gsm_rev(gsm_rev_tail, 2);           // 去掉1,
            }
            else if (buf[0] == ',')
            {
                netx = buf[1] - 0x30;                   //端口号
                incp_gsm_rev(gsm_rev_tail, 3);          // 去掉,1,
            }
            read_gsm(buf, gsm_rev_tail, 4);
            buf[3] = '\0';
            org_len = 0;
            org_len =  atoi((const char *)&buf);
			
		#if 0	
			if(upgrade_m.enable)	
            	logd("********* DATA LEN = %d*******",org_len); //调试时发现,此处需要换行,否则远程升级时,数据解析会经常出错
        #endif
        
            if (org_len == 0 || org_len > MAX_NET_PACKAGE)
            {
                logd("net_idle err:%d, 0", org_len);
                incp_gsm_rev(gsm_rev_tail, 1);
                return;
            }
            if(org_len > 99)
            {
               incp_gsm_rev(gsm_rev_tail, 3); //如果长度是3位数
            }
            else
            {
               incp_gsm_rev(gsm_rev_tail, 2); //如果长度是2位数
            }
            pt = gsm_rev_tail;

			
			if( netx == 2 )
			{
				net_step = NET_COPY;
				incp_gsm_rev(gsm_rev_tail, 2);
				return;
			}			
			
            //判断标志位是否是\r\n，
            read_gsm(buf, pt, 2);
            if (_strncmp(buf, "\r\n", 2) != 0)
            {
                logd("net_idle err: 4");
                incp_gsm_rev(gsm_rev_tail, 2);
				return;

            }
            //得到实际数据
            gsm_rev_tail  = pt;
            incp_gsm_rev(gsm_rev_tail, 2); //去掉\r\n
            net_step = NET_COPY;
            return;
        }
        incp_gsm_rev(gsm_rev_tail, 1);
    }
}


#elif ( GSM == SLM790 )

static void net_idle(void)
{
    u8 buf[32] = {0};
    u8 * pt;
    u8  len = 0;
    u8 baud[5] = {0};
    u8 i=0;
    
    pt = buf;
    while (new_gsm_rev() > 24)
    {
        _memset(buf, 0x00, 32);
        read_gsm(buf, gsm_rev_tail, 24);
        if (_strncmp(buf, GPRS_REV_FLAG, GPRS_REV_LEN) == 0)//查找关键字 +MIPRTCP: 1,20,41233,~
        {
            pt += GPRS_REV_LEN+2;
            len += GPRS_REV_LEN+2;
            
            //_memcpy_len(buf,  gsm_rev_tail, 3);
            if (pt[1] == ',')
            {
                netx = pt[0] - 0x30;                   //端口号
                pt += 2;
                len += 2;
            }
            else if (pt[0] == ',')
            {
                netx = pt[1] - 0x30;                   //端口号
                pt += 3;
                len += 3;
            }
            i = _memcpyCH(baud, ',', pt, 5);
            baud[i] = '\0';
            org_len = 0;
            org_len =  atoi((const char *)&baud);
			pt += i;
            len += i;

        
            if (org_len == 0 || org_len > MAX_NET_PACKAGE)
            {
                logd("net_idle err:%d 0", org_len);
                incp_gsm_rev(gsm_rev_tail, 1);
                return;
            }
            
            i = _memcpyCH(baud, ',', pt, 10);
            if( i > 10 )
            {
                logd("net_idle err: 4");
                incp_gsm_rev(gsm_rev_tail, 2);
				return;
            }
            pt += i;
            len += i;
            
            incp_gsm_rev(gsm_rev_tail, len); //去掉\r\n
            net_step = NET_COPY;
            return;
        }
        incp_gsm_rev(gsm_rev_tail, 1);
    }
}
#endif


/****************************************************************************
* 名称:    net_copy ()
* 功能：将GSM串口的数据接收到GPRS缓存中
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void net_copy(void)
{
    static u16 jbak;
    u16 flag_offset = 0;
    u16 i = 0;

    if (new_step)
    {
        jbak = jiffies;
    }

    if (_pasti(jbak) >= 3000)
    {
        logd("net_copy err: 0");
        net_step = NET_IDLE;
        return;
    }

    if (_pastn(gsm_rev_jfs) < 10)
        return;

    flag_offset = subp_gsm_rev(gsm_rev_head, gsm_rev_tail);


    if (flag_offset >= org_len)
    {
        if (*gsm_rev_tail != 0x7e)
        {
            logd("net_copy:%04d", org_len);
            logd("head:0x%02X, 0x%p", *gsm_rev_tail, gsm_rev_tail);
        }

        for (i = 0; i < org_len; i++)
        {
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
static void net_wait_ending(void){
    static u16 wbak;
    u8 xvalue = 0;
    u8 * pt;
    u8 flag_cnt;
    u16 len;
    u16 nc_len  = 0;
    u8 nc_buf[MAX_NET_PACKAGE];

    if (new_step)
    {
        wbak = jiffies;
    }

    if (_pasti(wbak) >= 500)
    {
        logd("net_copy err: 0");
        net_step = NET_IDLE;
        return;
    }

#if 0	
	if( netx == 2 )
	{
		//发送数据到 千寻SDK
		nc_len = new_gprs_rev();
		read_gprs(nc_buf, gprs_rev_tail, nc_len);
		qxwz_notice_recv(nc_buf,nc_len);
		clr_gprs_rev();
		gprs_rev_tail = gprs_rev_head;
		net_step = NET_IDLE;
		return;
	}	
#endif

    //找到第一个7E 的位置
    pt = get_net_7e_pointer(gprs_rev_head, gprs_rev_tail);
    if (pt == NULL)
    {
        clr_gprs_rev();
        logd("net_wait_ending: 1");
        goto wait_ending_out;
    }
    gprs_rev_tail = pt;

    //print_buf(nc_buf, nc_len);    //chwsh

    //统计7E 个数
    flag_cnt = get_net_7e_cnt(gprs_rev_head, gprs_rev_tail);
    if (flag_cnt >= 2)
    {
        //去掉第一个7E
        incp_gprs_rev(gprs_rev_tail, 1);

        //寻找下个7E的位置
        pt = get_net_7e_pointer(gprs_rev_head, gprs_rev_tail);
        len = subp_gprs_rev(pt, gprs_rev_tail) + 1;
        if (len < MIM_NET_PACKAGE || len > MAX_NET_PACKAGE)   //有问题数据2018-10-16   
        {
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
        if (xvalue != nc_buf[nc_len - 1])
        {
            logd("net_wait_ending: 3");
            clr_gprs_rev();    //多包问题处理 2014-09-09
            return;
        }

        jt808_net_parse(nc_buf, nc_len);
        incp_gprs_rev(gprs_rev_tail, len);

        if (gprs_rev_head == gprs_rev_tail)
        {
            goto wait_ending_out;
        }
    }
    else
    {
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


/*********************************************************
名    称：gprs_parse_task
功    能：GPRS 数据解析任务
输入参数：无
输    出：无
编写日期：2013-12-01
**********************************************************/
void gprs_parse_task(void)
{
//	static bool debug_tmp = false;
    enum net_step step;

    if( gsm_step_status() ) 
    {
        step = net_step;
        (*NetTaskTab[net_step % NET_END])();
        new_step = (net_step == step) ? false : true;
    }

}



