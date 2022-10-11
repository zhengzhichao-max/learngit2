/**
  ******************************************************************************
  * @file    send.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  所有GPRS数据发送管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

static REPORT_MANAGE_STRUCT net_m; //网络数据管理
static REPORT_MANAGE_STRUCT rpt_m; //实时报告管理
static u8 ack_lost = 0;            //ACK 错误累计次数

u16 cur_send_water; //流水号

/*
NET 发送相应函数
*/
/****************************************************************************
* 名称:    clear_cur_net ()
* 功能：初始化
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static void clear_cur_net(u8 index) {
    if (index >= NET_QUE_SIZE)
        return;

    _memset((u8 *)&net_m.que[index], 0x00, sizeof(report_queue));
}

/****************************************************************************
* 名称:    search_net_oldest ()
* 功能：查找队列中，NET数据时间最先的信息
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static s8 search_net_oldest(void) {
    s8 old;
    u8 i;
    u16 tick_old;

    old = -1;
    tick_old = 0;

    for (i = 0; i < NET_QUE_SIZE; i++) {
        if (old < 0 && net_m.que[i].event != 0 && net_m.que[i].resend > 0) {
            old = i;
            tick_old = net_m.que[i].tick;
        }

        if (old >= 0 && tick_old > net_m.que[i].tick && net_m.que[i].event != 0 && net_m.que[i].resend > 0) {
            old = i;
            tick_old = net_m.que[i].tick;
        }
    }
    return old;
}

/****************************************************************************
* 名称:    search_net_overtime ()
* 功能：NET 数据发送时间超时
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static u8 search_net_overtime(void) {
    u8 i;
    u8 empty;

    empty = 0;
    for (i = 0; i < NET_QUE_SIZE; i++) {
        if (!net_m.que[i].en)
            empty++;

        if (net_m.que[i].en && (_coveri(net_m.que[i].tick) > 180)) {
            empty++;
            _memset((u8 *)&net_m.que[i], 0x00, sizeof(report_queue));
        }
    }

    return empty;
}

/****************************************************************************
* 名称:    get_net_empty ()
* 功能：当前队列中是否为溢出
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static bool get_net_empty(void) {
    if (net_m.cnt == NET_QUE_SIZE)
        return true;
    else
        return false;
}

/****************************************************************************
* 名称:    get_net_empty ()
* 功能：当前队列中是否为空
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static bool get_net_over(void) {
    if (net_m.cnt == 0)
        return true;
    else
        return false;
}

/****************************************************************************
* 名称:    verify_net_info ()
* 功能：NET信息校验
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static bool verify_net_info(report_queue *rp, u8 *p) {
    if (rp->len < 5 || rp->len > 1000) {
        return false;
    }

    if (p[0] != 0x7e || p[rp->len - 1] != 0x7e) {
        return false;
    }

    return true;
}

/****************************************************************************
* 名称:    init_net_info ()
* 功能：初始化
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void init_net_info(void) {
    u8 i = 0;
    for (i = 0; i < NET_QUE_SIZE; i++) {
        clear_cur_net(i);
    }

    net_m.index = 0;
    net_m.cnt = NET_QUE_SIZE;
    net_m.n_addr = NET_MIN_ADDR;

    spi_flash_erase(NET_MIN_ADDR);
}

/*********************************************************
名	  称：search_net_ack
功	  能：寻找NET数据ACK应答
输入参数：
输出参数：
全局变量:   
返	  回：
说明:若有应答，send_net_proc负责清空  
if(old<0 && net_m.que[i].event !=0 && net_m.que[i].resend > 0)
*********************************************************/
bool get_net_ack(u16 id_t, u16 water_t, u8 netn) {
    u16 i = 0;
    u8 no = 0;
    for (i = 0; i < NET_QUE_SIZE; i++) {
        if (id_t == net_m.que[i].event && water_t == net_m.que[i].water) {
            no = (netn);
            net_m.que[i].netn &= ~no; //清除相应的标志位
            net_m.index = i;
            net_m.ack = true;
            return true;
        }
    }

    return false;
}

/*********************************************************
名	  称：add_net_queue
功	  能：添加实时报告到缓存区
1.在3分钟以内或发送三次报告，终端收到平台回复，信息自动退出队列
2.若超过三分钟，没有收到回复，自动进入盲区补报机制
3.若连续发送三次，没有收到回复，自动进入盲区补报机制
4.终端分配10个sector, 保存30组数据，理论计算足够存储普通报告
5.跨页操作前将自动对下个SECTOR	擦除
6.循环存储
输入参数：缓冲区地址，
输出参数：规格化后的缓冲区长度
全局变量:   
返	  回：
注意事项：
1.若网络正常，终端实时上传信息，芯片复位一般不会丢失报告
2.若芯片复位，最坏情况，终端最多丢失3分钟内的报告
*********************************************************/
bool add_net_queue(u16 id, u16 wnum, u8 *buf, u16 len, u8 netn) {
    u8 i = 0;
    u8 index = 0;
    u16 c_sector = 0;
    u16 n_sector = 0;

    if (net_m.n_addr < NET_MIN_ADDR) {
        init_net_info();
    }

    if (get_net_over()) {
        index = search_net_oldest();
        clear_cur_net(index);
    }

    for (i = 0; i < NET_QUE_SIZE; i++) {
        if (net_m.que[i].en == false) {
            net_m.que[i].en = true;
            net_m.que[i].ack = false;
            net_m.que[i].netn = netn; //通道记录号
            net_m.que[i].event = id;
            net_m.que[i].water = wnum;
            net_m.que[i].tick = tick;
            net_m.que[i].resend = 2;
            net_m.que[i].len = len;
            net_m.que[i].addr = net_m.n_addr;

            c_sector = net_m.n_addr / 0x1000; //当前信息扇区

            if ((net_m.n_addr + len + 0x800) > NET_MAM_ADDR)
                net_m.n_addr = NET_MIN_ADDR; //FLASH 将超出
            else
                net_m.n_addr = net_m.n_addr + len + 8;
            n_sector = net_m.n_addr / 0x1000; //下条信息扇区

            if (c_sector != n_sector)
                spi_flash_erase(_sect_addr(n_sector));

            spi_flash_write(buf, net_m.que[i].addr, len);
            break;
        }
    }
    return true;
}

/****************************************************************************
* 名称:    send_net_proc ()
* 功能：发送NET 数据
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static void send_net_proc(void) {
    static enum {
        NET_IDLE,
        NET_NET_SEND,
        NET_WAIT_ACK,
        NET_END
    } step = NET_IDLE;
    u8 tmp = 0;
    static u16 send = 0;
    u8 buf[MAX_INFO_SIZE];

    switch (step) {
    case NET_IDLE:
        if (get_net_empty() || (gc.gprs < NET_ONLINE) || (tick < 60))
            break;

        step = NET_NET_SEND;
        break;
    case NET_NET_SEND:
        if ((gc.gprs <= NET_CONNECT) || get_net_empty()) {
            step = NET_IDLE;
            break;
        }

        if (!free_gsm() || free_gprs_send() < (GPRS_SEND_SIZE - 100))
            break;

        tmp = search_net_oldest();
        if (tmp < RPT_QUE_SIZE) {
            spi_flash_read(buf, net_m.que[tmp].addr, net_m.que[tmp].len);
            if (verify_net_info(&net_m.que[tmp], buf)) {
                net_send_hex(buf, net_m.que[tmp].len, net_m.que[tmp].netn);
                net_m.ack = false;
                net_m.que[tmp].resend--;
                net_m.index = tmp;
                send = tick;
                step = NET_WAIT_ACK;
                break;
            } else {
                clear_cur_net(tmp);
            }
        } else {
            step = NET_IDLE;
            break;
        }
        break;
    case NET_WAIT_ACK:
        if (gc.gprs <= NET_CONNECT) {
            step = NET_IDLE;
            break;
        }

        if (net_m.ack) {
            ack_lost = 0;
            net_m.ack = false;
            //if( server_cfg.dbip == 0x55 )
            {
                if (net_m.que[net_m.index].netn == 0x00)
                    goto r_next;
                //处理未联接的IP 并进行保存
                if (((net_m.que[net_m.index].netn) & 0x01) == 0x01) {
                    if (!gs.gprs1.login)
                        goto r_next;
                }
                if (((net_m.que[net_m.index].netn) & 0x02) == 0x02) {
                    if (!gs.gprs2.login)
                        goto r_next;
                }
            }
        }

        if (_coveri(send) >= ACK_TIMEOUT) {
            send = tick;
            if (++ack_lost >= 15) {
                ack_lost = 0;
                gc.gprs = NET_LOST; //140417 问题
            }
            if (net_m.que[net_m.index].resend == 0) {
            r_next:
                clear_cur_net(net_m.index);
                net_m.ack = false;
                step = NET_IDLE;
                break;
            } else {
                step = NET_NET_SEND;
            }
        }
        break;
    default:
        step = NET_IDLE;
    }
}
/*
send net end:
*/

/*
report 数据管理
*/

/****************************************************************************
* 名称:    verify_report_info ()
* 功能：信息校验
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static bool verify_report_info(report_queue *rp, u8 *p) {
    u8 v_buf[128];

    _memset(v_buf, 0x00, 128);
    if (rp->len < 5 || rp->len > 128) {
        return false;
    }

#if 0
	//若车辆处于停止状态，不进入盲区，节省流量
	if(rp->moving == false)
	{
		return false; 
	}
#endif

    _memcpy(v_buf, p, rp->len);

    if (v_buf[0] != 0x7e || v_buf[rp->len - 1] != 0x7e) {
        return false;
    }

    return true;
}

/****************************************************************************
* 名称:    clear_cur_report ()
* 功能：初始化
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static void clear_cur_report(u8 index) {
    if (index >= RPT_QUE_SIZE)
        return;

    _memset((u8 *)&rpt_m.que[index], 0x00, sizeof(report_queue));
}

/****************************************************************************
* 名称:    search_reprot_oldest ()
* 功能：寻找队列中最旧的信息
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static s8 search_reprot_oldest(void) {
    u8 old;
    u8 i;
    u32 tick_old;

    old = 0xff;
    tick_old = 0;

    for (i = 0; i < RPT_QUE_SIZE; i++) {
        if (old == 0xff && rpt_m.que[i].event != 0 && rpt_m.que[i].resend > 0) {
            old = i;
            tick_old = rpt_m.que[i].tick;
        }

        if (old != 0xff && tick_old > rpt_m.que[i].tick && rpt_m.que[i].event != 0 && rpt_m.que[i].resend > 0) {
            old = i;
            tick_old = rpt_m.que[i].tick;
        }
    }
    return old;
}

/****************************************************************************
* 名称:    search_reprot_overtime ()
* 功能：寻找数据是否发送超时
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
u8 search_reprot_overtime(void) {
    u8 i;
    u8 empty;

    empty = 0;
    for (i = 0; i < RPT_QUE_SIZE; i++) {
        if (!rpt_m.que[i].en)
            empty++;

        if (rpt_m.que[i].en && (_coveri(rpt_m.que[i].tick) > 60)) {
            empty++;
            //添加转存到盲区数据
            //save_history_data(i);
            loge("***** timeout clear report index:%d*****", i);
            //_memset((u8*)&rpt_m.que[i], 0x00, sizeof(report_queue));
            clear_cur_report(i);
        }
    }

    return empty;
}

/****************************************************************************
* 名称:    get_report_empty ()
* 功能：判断队列是否为空
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static bool get_report_empty(void) {
    if (rpt_m.cnt == RPT_QUE_SIZE)
        return true;
    else
        return false;
}

/****************************************************************************
* 名称:    get_report_over ()
* 功能：判断队列是否溢出
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static bool get_report_over(void) {
    if (rpt_m.cnt < 2)
        return true;
    else
        return false;
}

/****************************************************************************
* 名称:    init_report_info ()
* 功能：初始化
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void init_report_info(void) {
    u8 i = 0;
    for (i = 0; i < RPT_QUE_SIZE; i++) {
        clear_cur_report(i);
    }

    rpt_m.index = 0;
    rpt_m.cnt = RPT_QUE_SIZE;
    rpt_m.n_addr = RPT_MIN_ADDR;
    spi_flash_erase(RPT_MIN_ADDR);
}

/****************************************************************************
* 名称:    get_reprot_ack ()
* 功能：等待应答
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
bool get_reprot_ack(u16 id_t, u16 water_t, u8 netn) {
    u16 i;
    //	u8  no = 0;
    for (i = 0; i < RPT_QUE_SIZE; i++) {
        if ((rpt_m.que[i].en) && (rpt_m.que[i].event == id_t) && (rpt_m.que[i].water == water_t)) {
            //		no = _LinktoNet(netn);			//LINK0-LINK1
            rpt_m.que[i].netn = 0;
            //logd("***** receive net %d,  index:%d!*****", netn, i);
            rpt_m.index = i;
            rpt_m.ack = true;
            return true;
        }
    }
    return false;
}

/*********************************************************
名	  称：add_report_queue
功	  能：添加实时报告到缓存区
1.在3分钟以内或发送三次报告，终端收到平台回复，信息自动退出队列
2.若超过三分钟，没有收到回复，自动进入盲区补报机制
3.若连续发送三次，没有收到回复，自动进入盲区补报机制
4.终端分配10个sector, 保存30组数据，理论计算足够存储普通报告
5.跨页操作前将自动对下个SECTOR	擦除
6.循环存储
输入参数：缓冲区地址，
输出参数：规格化后的缓冲区长度
全局变量:   
返	  回：
注意事项：
1.若网络正常，终端实时上传信息，芯片复位一般不会丢失报告
2.若芯片复位，最坏情况，终端最多丢失3分钟内的报告
*********************************************************/
bool add_report_queue(u16 id, u16 wnum, u8 *buf, u16 len, u8 netn) {
    u8 i = 0;
    u8 index = 0;
    u16 n_sector = 0;
    u16 c_sector = 0;

    if (rpt_m.n_addr < RPT_MIN_ADDR) {
        init_report_info();
    }

    if (get_report_over()) {
        index = search_reprot_oldest();
        clear_cur_report(index);
    }

    for (i = 0; i < RPT_QUE_SIZE; i++) {
        if (rpt_m.que[i].en == false) {
            //logd("***** add report index0 :%d*****", i);
            rpt_m.que[i].en = true;
            rpt_m.que[i].ack = false;
            rpt_m.que[i].netn = netn; //
            rpt_m.que[i].event = id;
            rpt_m.que[i].water = wnum;
            rpt_m.que[i].tick = tick;
            rpt_m.que[i].resend = 1;
            rpt_m.que[i].len = len;
            rpt_m.que[i].addr = rpt_m.n_addr;
            rpt_m.que[i].moving = (mix.moving == false) ? false : true;

            c_sector = rpt_m.n_addr / 0x1000; //当前信息扇区
            if ((rpt_m.n_addr + len + 0x800) > RPT_MAM_ADDR)
                rpt_m.n_addr = RPT_MIN_ADDR; //FLASH 将超出
            else
                rpt_m.n_addr = rpt_m.n_addr + len + 8;

            n_sector = rpt_m.n_addr / 0x1000; //下条信息扇区

            if (n_sector != c_sector) {
                spi_flash_erase(_sect_addr(n_sector));
            }

            spi_flash_write(buf, rpt_m.que[i].addr, len);
            break;
        }
    }
    return true;
}

/****************************************************************************
* 名称:    send_report_proc ()
* 功能：发送实时报告
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static void send_report_proc(void) {
    static enum {
        RPT_IDLE,
        RPT_NET_SEND,
        RPT_WAIT_ACK,
        RPT_STORE,
        RPT_END
    } step = RPT_IDLE;
    u8 tmp = 0;
    static u16 send = 0;
    static u16 store = 0;
    u8 buf[MAX_INFO_SIZE];

    switch (step) {
    case RPT_IDLE:
        if (get_report_empty() || tick < 30)
            break;

        if ((gc.gprs < NET_ONLINE)) //信号弱，GPRS不能联网
        {
            step = RPT_STORE;
            break;
        }

        if (upgrade_m.enable)
            break;

        step = RPT_NET_SEND;
        send = tick;
        break;
    case RPT_NET_SEND:
        if (get_report_empty()) {
            step = RPT_IDLE;
            break;
        }

        if (!free_gsm() || free_gprs_send() < (GPRS_SEND_SIZE - 100))
            break;

        tmp = search_reprot_oldest();
        if (tmp < RPT_QUE_SIZE) {
            spi_flash_read(buf, rpt_m.que[tmp].addr, rpt_m.que[tmp].len);
            if (verify_report_info(&rpt_m.que[tmp], buf)) {
                net_send_hex(buf, rpt_m.que[tmp].len, rpt_m.que[tmp].netn);
                rpt_m.ack = false;
                rpt_m.que[tmp].resend--;
                rpt_m.index = tmp; //保存当前索引，等待ACK回复
                send = tick;
                step = RPT_WAIT_ACK;
                break;
            } else {
                loge("err_rpt_verify:%d", tmp);
                clear_cur_report(tmp);
            }
        } else {
            step = RPT_IDLE;
        }
        break;
    case RPT_WAIT_ACK:

        if (rpt_m.ack) {
            rpt_m.ack = false;
            ack_lost = 0;

            if (rpt_m.que[rpt_m.index].netn == 0) {
                //logd("***** clear index:%d*****", rpt_m.index);
                goto rnext0;
            }
            //处理未联接的IP 并进行保存
            if (((rpt_m.que[rpt_m.index].netn) & 0x01) == 0x01) {
                if (gs.gprs1.stat < NET_CONNECT) {
                    logd("***** save net1 index:%d*****", rpt_m.index);
                    goto rsave0;
                }
            }
            if (((rpt_m.que[rpt_m.index].netn) & 0x02) == 0x02) {
                if (gs.gprs2.stat < NET_CONNECT) {
                    logd("***** save net2 index:%d*****", rpt_m.index);
                    goto rsave0;
                }
            }
        }

        if (_coveri(send) >= ACK_TIMEOUT) {
            send = tick;
            if (++ack_lost >= 30) {
                ack_lost = 0;
                if (((rpt_m.que[rpt_m.index].netn) & 0x01) == 0x01)
                    gs.gprs1.stat = NET_LOST;
                if (((rpt_m.que[rpt_m.index].netn) & 0x02) == 0x02)
                    gs.gprs2.stat = NET_LOST;
            }
            if (rpt_m.que[rpt_m.index].resend == 0) {
            rsave0:
                logd("***** save report index:%d, netn:%d*****", rpt_m.index, rpt_m.que[rpt_m.index].netn);
                //save_history_data(rpt_m.index);
            rnext0:
                clear_cur_report(rpt_m.index);
                rpt_m.ack = false;
                step = RPT_IDLE;
                break;
            } else {
                step = RPT_NET_SEND;
            }
        }
        break;
    case RPT_STORE:
        if (_pasti(store) > 200) {
            store = jiffies;
            tmp = search_reprot_oldest();
            if (tmp < RPT_QUE_SIZE) {
                spi_flash_read(buf, rpt_m.que[tmp].addr, rpt_m.que[tmp].len);
                if (verify_report_info(&rpt_m.que[tmp], buf)) {
                } else {
                    loge("err_rpt_verify:%d", tmp);
                }
                clear_cur_report(tmp);
            } else {
                step = RPT_IDLE;
            }
            step = RPT_IDLE;
        }
        break;
    default:
        step = RPT_IDLE;
    }
}
/*
send report end:
*/

/*
batch 批量数据发送
*/

/*********************************************************
名	  称：report_manage_task
功	  能：发送报告到GSM模块
1.发送NET, REPORT, HISTROY三种数据到平台
2.net数据包括回应平台数据，透传数据
3.report数据包括普通汇报策略，报警信息
	若没有收到平台回应，进入历史补报机制
4.history	盲区补报数据
5.batch 批量数据发送，用于偶尔大批量数据处理，如升级，摄像头等
6.FLASH 的寿命为擦除10万次
输入参数：缓冲区地址，
输出参数：规格化后的缓冲区长度
全局变量:   
返	  回：
注意事项：
*********************************************************/
void report_manage_task(void) {
    net_m.cnt = search_net_overtime();
    rpt_m.cnt = search_reprot_overtime();

    send_net_proc();
    send_report_proc();
}
