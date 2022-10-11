/**
  ******************************************************************************
  * @file    send.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ����GPRS���ݷ��͹���
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

static REPORT_MANAGE_STRUCT net_m; //�������ݹ���
static REPORT_MANAGE_STRUCT rpt_m; //ʵʱ�������
static u8 ack_lost = 0;            //ACK �����ۼƴ���

u16 cur_send_water; //��ˮ��

/*
NET ������Ӧ����
*/
/****************************************************************************
* ����:    clear_cur_net ()
* ���ܣ���ʼ��
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
static void clear_cur_net(u8 index) {
    if (index >= NET_QUE_SIZE)
        return;

    _memset((u8 *)&net_m.que[index], 0x00, sizeof(report_queue));
}

/****************************************************************************
* ����:    search_net_oldest ()
* ���ܣ����Ҷ����У�NET����ʱ�����ȵ���Ϣ
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    search_net_overtime ()
* ���ܣ�NET ���ݷ���ʱ�䳬ʱ
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    get_net_empty ()
* ���ܣ���ǰ�������Ƿ�Ϊ���
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
static bool get_net_empty(void) {
    if (net_m.cnt == NET_QUE_SIZE)
        return true;
    else
        return false;
}

/****************************************************************************
* ����:    get_net_empty ()
* ���ܣ���ǰ�������Ƿ�Ϊ��
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
static bool get_net_over(void) {
    if (net_m.cnt == 0)
        return true;
    else
        return false;
}

/****************************************************************************
* ����:    verify_net_info ()
* ���ܣ�NET��ϢУ��
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    init_net_info ()
* ���ܣ���ʼ��
* ��ڲ�������                         
* ���ڲ�������
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
��	  �ƣ�search_net_ack
��	  �ܣ�Ѱ��NET����ACKӦ��
���������
���������
ȫ�ֱ���:   
��	  �أ�
˵��:����Ӧ��send_net_proc�������  
if(old<0 && net_m.que[i].event !=0 && net_m.que[i].resend > 0)
*********************************************************/
bool get_net_ack(u16 id_t, u16 water_t, u8 netn) {
    u16 i = 0;
    u8 no = 0;
    for (i = 0; i < NET_QUE_SIZE; i++) {
        if (id_t == net_m.que[i].event && water_t == net_m.que[i].water) {
            no = (netn);
            net_m.que[i].netn &= ~no; //�����Ӧ�ı�־λ
            net_m.index = i;
            net_m.ack = true;
            return true;
        }
    }

    return false;
}

/*********************************************************
��	  �ƣ�add_net_queue
��	  �ܣ����ʵʱ���浽������
1.��3�������ڻ������α��棬�ն��յ�ƽ̨�ظ�����Ϣ�Զ��˳�����
2.�����������ӣ�û���յ��ظ����Զ�����ä����������
3.�������������Σ�û���յ��ظ����Զ�����ä����������
4.�ն˷���10��sector, ����30�����ݣ����ۼ����㹻�洢��ͨ����
5.��ҳ����ǰ���Զ����¸�SECTOR	����
6.ѭ���洢
�����������������ַ��
�����������񻯺�Ļ���������
ȫ�ֱ���:   
��	  �أ�
ע�����
1.�������������ն�ʵʱ�ϴ���Ϣ��оƬ��λһ�㲻�ᶪʧ����
2.��оƬ��λ���������ն���ඪʧ3�����ڵı���
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
            net_m.que[i].netn = netn; //ͨ����¼��
            net_m.que[i].event = id;
            net_m.que[i].water = wnum;
            net_m.que[i].tick = tick;
            net_m.que[i].resend = 2;
            net_m.que[i].len = len;
            net_m.que[i].addr = net_m.n_addr;

            c_sector = net_m.n_addr / 0x1000; //��ǰ��Ϣ����

            if ((net_m.n_addr + len + 0x800) > NET_MAM_ADDR)
                net_m.n_addr = NET_MIN_ADDR; //FLASH ������
            else
                net_m.n_addr = net_m.n_addr + len + 8;
            n_sector = net_m.n_addr / 0x1000; //������Ϣ����

            if (c_sector != n_sector)
                spi_flash_erase(_sect_addr(n_sector));

            spi_flash_write(buf, net_m.que[i].addr, len);
            break;
        }
    }
    return true;
}

/****************************************************************************
* ����:    send_net_proc ()
* ���ܣ�����NET ����
* ��ڲ�������                         
* ���ڲ�������
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
                //����δ���ӵ�IP �����б���
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
                gc.gprs = NET_LOST; //140417 ����
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
report ���ݹ���
*/

/****************************************************************************
* ����:    verify_report_info ()
* ���ܣ���ϢУ��
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
static bool verify_report_info(report_queue *rp, u8 *p) {
    u8 v_buf[128];

    _memset(v_buf, 0x00, 128);
    if (rp->len < 5 || rp->len > 128) {
        return false;
    }

#if 0
	//����������ֹͣ״̬��������ä������ʡ����
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
* ����:    clear_cur_report ()
* ���ܣ���ʼ��
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
static void clear_cur_report(u8 index) {
    if (index >= RPT_QUE_SIZE)
        return;

    _memset((u8 *)&rpt_m.que[index], 0x00, sizeof(report_queue));
}

/****************************************************************************
* ����:    search_reprot_oldest ()
* ���ܣ�Ѱ�Ҷ�������ɵ���Ϣ
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    search_reprot_overtime ()
* ���ܣ�Ѱ�������Ƿ��ͳ�ʱ
* ��ڲ�������                         
* ���ڲ�������
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
            //���ת�浽ä������
            //save_history_data(i);
            loge("***** timeout clear report index:%d*****", i);
            //_memset((u8*)&rpt_m.que[i], 0x00, sizeof(report_queue));
            clear_cur_report(i);
        }
    }

    return empty;
}

/****************************************************************************
* ����:    get_report_empty ()
* ���ܣ��ж϶����Ƿ�Ϊ��
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
static bool get_report_empty(void) {
    if (rpt_m.cnt == RPT_QUE_SIZE)
        return true;
    else
        return false;
}

/****************************************************************************
* ����:    get_report_over ()
* ���ܣ��ж϶����Ƿ����
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
static bool get_report_over(void) {
    if (rpt_m.cnt < 2)
        return true;
    else
        return false;
}

/****************************************************************************
* ����:    init_report_info ()
* ���ܣ���ʼ��
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    get_reprot_ack ()
* ���ܣ��ȴ�Ӧ��
* ��ڲ�������                         
* ���ڲ�������
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
��	  �ƣ�add_report_queue
��	  �ܣ����ʵʱ���浽������
1.��3�������ڻ������α��棬�ն��յ�ƽ̨�ظ�����Ϣ�Զ��˳�����
2.�����������ӣ�û���յ��ظ����Զ�����ä����������
3.�������������Σ�û���յ��ظ����Զ�����ä����������
4.�ն˷���10��sector, ����30�����ݣ����ۼ����㹻�洢��ͨ����
5.��ҳ����ǰ���Զ����¸�SECTOR	����
6.ѭ���洢
�����������������ַ��
�����������񻯺�Ļ���������
ȫ�ֱ���:   
��	  �أ�
ע�����
1.�������������ն�ʵʱ�ϴ���Ϣ��оƬ��λһ�㲻�ᶪʧ����
2.��оƬ��λ���������ն���ඪʧ3�����ڵı���
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

            c_sector = rpt_m.n_addr / 0x1000; //��ǰ��Ϣ����
            if ((rpt_m.n_addr + len + 0x800) > RPT_MAM_ADDR)
                rpt_m.n_addr = RPT_MIN_ADDR; //FLASH ������
            else
                rpt_m.n_addr = rpt_m.n_addr + len + 8;

            n_sector = rpt_m.n_addr / 0x1000; //������Ϣ����

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
* ����:    send_report_proc ()
* ���ܣ�����ʵʱ����
* ��ڲ�������                         
* ���ڲ�������
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

        if ((gc.gprs < NET_ONLINE)) //�ź�����GPRS��������
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
                rpt_m.index = tmp; //���浱ǰ�������ȴ�ACK�ظ�
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
            //����δ���ӵ�IP �����б���
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
batch �������ݷ���
*/

/*********************************************************
��	  �ƣ�report_manage_task
��	  �ܣ����ͱ��浽GSMģ��
1.����NET, REPORT, HISTROY�������ݵ�ƽ̨
2.net���ݰ�����Ӧƽ̨���ݣ�͸������
3.report���ݰ�����ͨ�㱨���ԣ�������Ϣ
	��û���յ�ƽ̨��Ӧ��������ʷ��������
4.history	ä����������
5.batch �������ݷ��ͣ�����ż�����������ݴ���������������ͷ��
6.FLASH ������Ϊ����10���
�����������������ַ��
�����������񻯺�Ļ���������
ȫ�ֱ���:   
��	  �أ�
ע�����
*********************************************************/
void report_manage_task(void) {
    net_m.cnt = search_net_overtime();
    rpt_m.cnt = search_reprot_overtime();

    send_net_proc();
    send_report_proc();
}
