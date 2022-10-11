/**
  ******************************************************************************
  * @file
  * @author
  * @Email
  * @version
  * @date    2018-05-28
  * @brief   EC20 模块管理
  ******************************************************************************
  * @attention
    1.控制和查询GSM_COMMAND_STRUCT和GSM_STATE_STRUCT
    2.参数管理gsm_context
    3.串口数据管理 gsm_rev_buf[], gsm_send_buf[], gprs_send_buf[]
    4.不同模块之间采用文件独立，如gsm_g610.c, gsm_sim340.c
    5.为了方便移植，不能随意直接添加外部变量到GSM.C
  ******************************************************************************
**/

#include "include_all.h"

#if (GSM == EC20)

//休眠位置上报:
enum E_GSM_STEP {
    GSM_POWER_UP = 0, //0
    GSM_OFF,          //1
    GSM_ENTER_PIN,    //2
    GSM_INIT,         //3
    GSM_CHECK,        //4
    GSM_SETUP,        //5
    GSM_CONNECT,      //6
    GSM_RECV_SMS,     //7
    GSM_SEND_SMS_TXT, //8
    GSM_SEND_SMS_PDU, //9
#if (P_SOUND == SOUND_BASE)
    GSM_TTS_HANDLE, //10
#endif
    GSM_REQUEST_ERROR, //11
    GSM_DISCONNECT,    //12
    GSM_END            //13
};

MODEM_CONTEXT_STRUCT mx; //GSM 模块信息管理
GSM_STATE_STRUCT gs;     //GSM 状态管理
GSM_COMMAND_STRUCT gc;   //GSM 控制管理

static u8 pwrup_error = 0; //上电，错误次数
//static u8 off_times = 0;   //AT 关机次数
static u8 turn_on_cnt = 0; //开机次数
static u16 qcheck = 0;     //CSQ 检测的时间
static u16 rcheck = 0;     //CREG 检测的时间

//休眠位置上报:
static bool new_step = true; //新状态
//休眠位置上报:
static enum E_GSM_STEP gsm_step = GSM_POWER_UP; //指针函数

SEND_SMS_STRUCT sms_s; //短信发送

static bool useless = false; //lugong

u8 sms_format_judge; //短信格式判断全局变量,在 sms_parse.c 文件中由set_card_message()函数调用 2018-1-16
u8 *urc_tail = gsm_rev_buf;

#define EXIT2_CMD "AT+QICLOSE=2\r\n" //退出GPRS功能
#define CLOSE_ACK "OK"

u32 read_sms_tick = 0; //定时读短信，需要避免批量发数据时，去读短信

void gsm_power_off(void) {
    gsm_step = GSM_OFF;
}

bool gsm_step_status(void) {
    if (gsm_step == GSM_CONNECT) {
        return true;
    }

    return false;
}

void config_gsm_port(void) {
    GSM_PWR_OUTPUT();
    EN_GSM_PWR;

    //#if (P_SOUND == SOUND_BASE)
    OPEN_PHONE;
    //#endif
}

#if 1 // (P_SOUND == SOUND_BASE)
void call_circuit_ctrl(u8 value) {
    if (value == CALL_OUT) {
        OPEN_PHONE;
    } else if (value == CALL_IN) {
        OPEN_PHONE;
    } else
        CLOSE_PHONE;
}

/****************************************************************************
* 名称:    play_tts_by_gsm ()
* 功能：GSM TTS 信息播放
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool play_tts_by_gsm(u8 *str, u16 len) {
    if (str == NULL)
        return false;
    if (len > 500)
        len = 500;
    tts_m.new_tts = true;
    tts_m.n_tick = tick;
    tts_m.len = len;
    if (tts_m.total == 0)
        spi_flash_erase((u32)(_sect_addr(SECT_TTS_ADDR)));
    spi_flash_write(str, (u32)(_sect_addr(SECT_TTS_ADDR)) + tts_m.total, tts_m.len);
    tts_m.total += tts_m.len;
    return true;
}
#endif

//24 02 21 30 32 37 00 11 00 0D 91 68 31 28 56 85 05 F2 00 08 01 0C 76 84 4E 00 4E 0D 66 2F 4E 86
//00 11 00 0D 91 68 31 28 56 85 05 F2 00 08 01 0C 76 84 4E 00 4E 0D 66 2F
u8 get_mc323_pdu_phone(u8 *ph, u8 *pdu, u16 p_len) {
    u8 pdu_t[MAX_SMS_PDU];
    u8 ph_t1[MAX_SMS_PDU];
    u8 ph_len1 = 0;

    u8 i = 0;
    u8 j = 0;

    if (p_len > MAX_SMS_PDU)
        return 0xff;

    _memset(pdu_t, 0x00, MAX_SMS_PDU);
    _memcpy(pdu_t, pdu, p_len);

    j = 5; //开始准备提取电话号码
    i = 0x00;
    while (i < 20) {
        if ((pdu_t[j] == 0x00) && (pdu_t[j + 1] == 0x08) && (pdu_t[j + 2] == 0x01)) {
            ph_t1[i] = '\0';
            break;
        } else {
            if ((pdu_t[j] & 0x0f) != 0x0f)
                ph_t1[i++] = (pdu_t[j] & 0x0f) + 0x30;
            if ((pdu_t[j] >> 4) != 0x0f)
                ph_t1[i++] = (pdu_t[j] >> 4) + 0x30;
            j++;
        }
    }

    ph_len1 = i;
    if (ph_len1 > 16 || ph_len1 < 5)
        return 0xff;

    for (i = 0; i < ph_len1; i++) {
        if (ph_t1[i] < '0' || ph_t1[i] > '9')
            return 0xff;
    }

    if (ph_t1[0] == '8' && ph_t1[1] == '6') {
        ph_len1 -= 2;
        _memcpy(ph, &ph_t1[2], ph_len1);
    } else {
        _memcpy(ph, &ph_t1[0], ph_len1);
    }

    return ph_len1;
}

//模式/号码/号码长度/信息/信息长度
void sms_send(u8 type, u8 *phone, u8 p_len, u8 *msg, u16 m_len) {
    u8 m_buf[MAX_SMS_PDU] = {0x00};
    u8 ph_t[32] = {0x00};
    u8 ph_len = 0;
    ///u8 i;
    if (msg == NULL || m_len == 0) {
        if (!u1_m.cur.b.tt) {
            loge("send sms err:1");
        }
        return;
    }

    _memset((u8 *)&sms_s, 0x00, sizeof(sms_s));

    if (type == SMS_TXT) {
        if (phone == NULL || m_len > MAX_SMS_TXT || p_len >= PHONELEN) {
            loge("send sms err:2");
            return;
        }

        sms_s.sms_new = true;
        sms_s.tick = tick;
        _strncpy((u8 *)&sms_s.phone, phone, p_len);
        sms_s.phone[p_len + 1] = '\0';
        sms_s.is_txt = true;
        sms_s.len = m_len;
        _memcpy((u8 *)&sms_s.msg, msg, m_len);
    } else if (type == SMS_PDU) {
        if (m_len > 155 || m_len < 16) {
            if (!u1_m.cur.b.tt) {
                loge("send sms err:3");
            }
            return;
        }
#if 1

        _memcpy(m_buf, msg, m_len);

        ph_len = get_mc323_pdu_phone(ph_t, msg, m_len);
        if (ph_len > 16) {
            if (!u1_m.cur.b.tt) {
                loge("send sms err:5");
            }
            return;
        }

        _strncpy((u8 *)&sms_s.phone, ph_t, ph_len);
        ph_len += ph_len;
        sms_s.phone[ph_len + 1] = '\0';
        sms_s.sms_new = true;
        sms_s.tick = tick;
        sms_s.is_txt = false;
        sms_s.len = m_len - 16;

        _memcpy((u8 *)&sms_s.msg, &m_buf[16], sms_s.len);
#else
        sms_s.len = _memcpy_len((u8 *)&sms_s.msg, msg, m_len);
        sms_s.sms_new = true;
        sms_s.tick = tick;
        sms_s.is_txt = false;
#endif
    }
}

void send_inc_gsm_data(u8 data) {
    *gsm_send_head = data;
    gsm_send_head++;
    if (gsm_send_head >= gsm_send_buf + GSM_SEND_SIZE)
        gsm_send_head = gsm_send_buf;
}

/****************************************************************************
* 名称:    net_send_hex ()
* 功能：发送GPRS数据
* 入口参数：netn: 网络端口号 0:
* 出口参数：无
****************************************************************************/
void net_send_hex(u8 *buf, u16 n, u8 netn) {
    u16 i;

    u8 *pf;
    pf = buf;

    if (gs.gprs1.stat >= NET_CONNECT) {
        if ((netn & 0x01) == 0x01) //IP0 锟斤拷 IP 锟角碉拷一路
        {
            for (i = 0; i < n; i++) {
                if (over_gprs_send())
                    break;

                *gprs_send_head = *buf++;
                incp_gprs_send(gprs_send_head, 1);
            }
        }
    }

    if (gs.gprs2.stat >= NET_CONNECT) {
        if ((netn & 0x02) == 0x02) {
            for (i = 0; i < n; i++) {
                if (over_gprs2_send())
                    break;

                *gprs2_send_head = *pf++;
                incp_gprs2_send(gprs2_send_head, 1);
            }
        }
    }
}

static void gsm_power_up(void) {
    static enum {
        PWR_OFF,
        PWR_ON,
        PWR_DELAY,
        PWR_FINISH
    } step = PWR_OFF;
    static u32 jbak;

    if (new_step) {
        gs.gprs1.login = false;
        gs.gsm.squality = 0;
        gs.gprs1.stat = NET_LOST;
        gc.gsm.reset = false;
        step = PWR_OFF;
        if (u1_m.cur.b.debug) {
            logi("EC20 gsm_power_up :1");
        }
    }

    switch (step) {
    case PWR_OFF:
        DIS_GSM_PWR; //关模块主电
        jbak = jiffies;
        step = PWR_ON;
        break;
    case PWR_ON:
        //if(gc.gsm.pwr_down)
        //    break;

        if (_pastn(jbak) >= 3000) //h 示波器
        {
            EN_GSM_PWR; //开启GSM模块电源
            jbak = jiffies;
            step = PWR_DELAY;
        }
        break;
    case PWR_DELAY:
        if (_pastn(jbak) >= 2000) {
            //off_times = 0;

            //EN_RESET_GSM;
            EN_RESET_HI35XX_PWR;
            step = PWR_FINISH;
        }
        break;
    case PWR_FINISH:
        if (_pastn(jbak) >= 1000) {
            gsm_step = GSM_ENTER_PIN;
            DIS_RESET_HI35XX_PWR;
        }
        break;
    default:
        step = PWR_OFF;
    }
}

#define WSTART_TIMES 3
#define PWROFF_SOCKET "AT+QICLOSE\r\n"
#define PWROFF_SACK "OK"
#define PWROFF_CMD "AT+QPOWD=1\r\n"
#define PWROFF_CACK "NORMAL POWER DOWN"

#define ASK_NET_STATUS "AT+QISTAT\r\n" //执行后是否需要执行  QICLOSE=1

//TR9 请求断电重启
void gsm_power_down(void) {
    ex_gsm_power_off();
}

//TR9 请求断电重启
void ex_gsm_power_off(void) {
    gsm_step = GSM_OFF;
}

//TR9 请求软件复位
void tr9_request_reset(void) {
    printf_gsm(PWROFF_CMD);
    ex_gsm_power_off();
    clr_gsm_rev();
}

static void gsm_off(void) {
    static enum {
        OFF_SOCKET,
        OFF_SACK,
        OFF_GSM,
        OFF_GACK,
        OFF_DELAY,
        OFF_HARDWARE,
    } step = OFF_SOCKET;

    static u16 jbak;

    if (new_step) {
        if (u1_m.cur.b.debug) {
            logi("EC20 gsm_off :0");
        }
        gs.gprs1.login = false;
        gs.gsm.squality = 0;
        gc.gsm.reset = false;
        gc.gprs = NET_LOST;
        sync_net_offline();
        step = OFF_SOCKET;
    }
    switch (step) {
    case OFF_SOCKET: //关闭SOCKET
        if (gs.gprs1.stat > NET_CONNECT) {
            step = OFF_GSM;
            break;
        }

        printf_gsm(PWROFF_SOCKET);
        clr_gsm_rev();
        jbak = jiffies;
        step = OFF_SACK;
        break;
    case OFF_SACK: //DIS_GSM_PWR;
        if (get_gsm_resp(PWROFF_SACK)) {
            jbak = jiffies;
            step = OFF_GSM;
        } else if (_pasti(jbak) > 5000) {
            jbak = jiffies;
            step = OFF_GSM; //OFF_HARDWARE
        }
        break;
    case OFF_GSM: //关闭模块信号
        printf_gsm(PWROFF_CMD);
        clr_gsm_rev();
        gc.gprs = NET_LOST;
        jbak = jiffies;
        step = OFF_GACK;
        break;
    case OFF_GACK:
        if (get_gsm_resp(PWROFF_CACK)) {
            jbak = jiffies;
            step = OFF_DELAY;
        } else if (_pasti(jbak) > 5000) {
            jbak = jiffies;
            step = OFF_HARDWARE;
        }
        break;
    case OFF_DELAY:
        if (_pasti(jbak) > 2000) {
            jbak = jiffies;
            gsm_step = GSM_POWER_UP;
        }
        break;
    case OFF_HARDWARE:
        //硬件控制关机，执行关机操作
        DIS_GSM_PWR; //关模块主电
        gc.gprs = NET_LOST;
        if (_pasti(jbak) > 5000) {
            jbak = jiffies;
            step = OFF_SOCKET;
            gsm_step = GSM_POWER_UP;
        }
        break;
    default:
        step = OFF_SOCKET;
    }
}

/****************************************************************************
* 名称:    gsm_enter_pin ()
* 功能：检测SIM 卡是否准备就绪
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void gsm_enter_pin(void) {
    static enum {
        LINK_QUERY,
        LINK_QACK,
        LINK_DELAY,
        PIN_QUERY,
        PIN_QACK,
        PIN_DELAY,
        PIN_ICCID_A,
        PIN_ICCID_R,
        PIN_IMEI_A,
        PIN_IMEI_B,
        PIN_IMEI_R,
    } step = LINK_QUERY;

    static u8 pin_error = 0;
    static u8 link_cnt = 0;

    u8 buf[100];
    u8 *pt;
    //    u8 i;
    static u32 jiff_cmd;

    if (new_step) {
        pin_error = 0;
        link_cnt = 0;
        step = LINK_QUERY;
    }

    if ((pub_io.b.dvr_open) && (s_Hi3520_Monitor.start)) //如果TR9未启动；则往下执行
    {
        run_no = 0;
        return;
    }

#if 0  
    if( register_cfg.acc_simulate == 0x02  ) 
        return;
    return;
#endif

    switch (step) {
    case LINK_QUERY:
        printf_gsm("AT\r\n");
        clr_gsm_rev();
        link_cnt++;
        jiff_cmd = jiffies;
        step = LINK_QACK;
        break;
    case LINK_QACK:
        if (_pastn(jiff_cmd) < 2000)
            break;

        if (get_gsm_resp("OK"))
            step = PIN_QUERY;

        else {
            jiff_cmd = jiffies;
            step = LINK_DELAY;
        }
        break;
    case LINK_DELAY:
        if (_pastn(jiff_cmd) > (150 * 1000 * 1ul)) {
            loge("enter pin err : ot");
        enter_pin_err:
            pwrup_error++;
            gsm_step = GSM_OFF;
            return;
        }

        if (turn_on_cnt > 30) //关机次数
        {
            if (link_cnt > 60) {
                logd("已等待模块文件恢复时间，2分钟");
                turn_on_cnt = 0;
                goto enter_pin_err;
            } else {
                if (u1_m.cur.b.debug) {
                    logd("EC20 file link 1");
                }
                step = LINK_QUERY;
            }
        } else {
            if (link_cnt > 50) {
                if (u1_m.cur.b.debug) {
                    logd("EC20 file link 5");
                }
                goto enter_pin_err;
            } else {
                if (u1_m.cur.b.debug) {
                    logd("EC20 file link 2");
                }
                step = LINK_QUERY;
            }
        }
        break;
    case PIN_QUERY:
        printf_gsm("AT+CPIN?\r\n");
        clr_gsm_rev();
        jiff_cmd = jiffies;
        step = PIN_QACK;
        break;
    case PIN_QACK:
        if (get_gsm_resp("OK")) {
            step = PIN_ICCID_A;
        } else if (get_gsm_resp("ERROR") || _pastn(jiff_cmd) > 5000) {
            jiff_cmd = jiffies;
            step = PIN_DELAY;
        }
        break;
    case PIN_DELAY:
        if (_pastn(jiff_cmd) < 2500)
            break;

        if (++pin_error >= 5) {
            if (u1_m.cur.b.debug) {
                loge("SIM 卡异常");
            }
            self_test.uGsmU = '1';
            goto enter_pin_err;
        } else {
            step = PIN_QUERY;
            break;
        }
    case PIN_ICCID_A:
        printf_gsm("AT+QCCID\r\n");
        jiff_cmd = jiffies;
        step = PIN_ICCID_R;
        break;
    case PIN_ICCID_R:
        if (_pastn(jiff_cmd) < 500) {
            break;
        }
        if (get_gsm_resp("OK")) {
            //提取ICCID 信息   Ccid data [ex. 898600810906F8048812]
            //read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
            // buf[sizeof(buf) - 1] = '\0';
            //if ((pt = _strstr(buf, "AT+CPIN")) != NULL)
            //{
            //_memcpy(mx.imei, pt + 6, 20);   ///866856034085371
            //   step = PIN_IMEI_A;
            // }
            //else
            //{
            clr_gsm_rev();
            //}
            step = PIN_IMEI_A;

            self_test.uGsmU = '0';
        } else if (get_gsm_resp("ERROR") || _pastn(jiff_cmd) > 5000) {
            jiff_cmd = jiffies;
            gsm_step = GSM_INIT;
        }
        break;
    case PIN_IMEI_A:
        //if (get_gsm_resp("OK"))
        {
            //if ((pt = _strstr(buf, "+QCCID")) != NULL)
            jiff_cmd = jiffies;
            printf_gsm("AT+GSN=?\r\n");
            clr_gsm_rev();
            step = PIN_IMEI_B;

            //}
        }
        break;
    case PIN_IMEI_B:
        if (get_gsm_resp("OK")) {
            printf_gsm("AT+GSN\r\n");
            step = PIN_IMEI_R;
            clr_gsm_rev();
        }
        break;
    case PIN_IMEI_R:
        if (_pastn(jiff_cmd) < 250)
            break;
        if (get_gsm_resp("OK")) {
            //提取ICCID 信息   Ccid data [ex. 898600810906F8048812]
            //AT+GSN
            //866856034085371
            read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if ((pt = _strstr(buf, "86")) != NULL) //收到IMEI码
            {
                if (mx.imei[0] != 8) {
                    _memcpy(mx.imei, pt, 15);
                    imei_flag = true;
                }
                gsm_step = GSM_INIT;
            } else {
                gsm_step = GSM_INIT;
            }
        } else if (get_gsm_resp("ERROR") || _pastn(jiff_cmd) > 5000) {
            jiff_cmd = jiffies;
            gsm_step = GSM_INIT;
        }

        break;
    default:
        step = PIN_QUERY;
    }
}

static u8 *const InitCmdTab[] =
    {
        "AT\r\n",
        "AT&F\r\n",      //恢复出厂设置
        "AT+CMEE=2\r\n", //设置错误消息提示格式 ERROR
        "ATE0\r\n",      //关闭命令字符回显
        "AT+GMR\r\n",    //
        "AT+CGREG=2\r\n",
        "AT+CLIP=1\r\n",

        "ATL3\r\n",      //设置监听speaker 音量
        "ATM2\r\n",      //设置监听 speaker 模式
        "AT+CLVL=3\r\n", // "AT+CLVL=90\r\n", 扬声器音量选择
                         //"AT+QMIC=0,14\r\n",                       // 0,12
        "AT+CNMI=0,1,0,1,0\r\n",
        //"AT+CNMI=3,1\r\n",

        "AT+CPMS=\"SM\",\"SM\",\"SM\"\r",

        "AT+QURCCFG=\"urcport\",\"uart1\"", //将URC端口设置为 uart1
        "AT+QSIDET=200\r\n",                //配置侧音增益等级
        "AT+CSDH=1\r\n",                    //显示短信文本模式参数
#if (P_SOUND == SOUND_BASE)
        "AT+QTTSETUP=1,1,0\r\n",     //设置语速  (正常)
        "AT+QTTSETUP=1,2,32767\r\n", //设置音量 (最大)
//"AT+QTTS=2, \"欢迎使用移远模块,深圳同人伟业科技有限公司\"\r\n",              //测试语音
#endif
        "AT+QDAI?\r\n",

        "AT+CSCS=\"GSM\"\r\n",
        "AT+CMGF=1\r\n",
        "AT+CSMP=17,167,0,241\r\n", //设置短信文本参数

        NULL};

static void gsm_init(void) {
    static enum {
        INIT_CMD,
        INIT_ACK,
        INIT_DELAY
    } step = INIT_CMD;
    static u8 n;
    static u8 init_error = 0;
    static u16 jiff_cmd = 0;
    static u16 jiff_delay = 0;

    if (new_step) {
        n = 0;
        init_error = 0;
        step = INIT_CMD;
        jiff_cmd = jiffies;
    }

    switch (step) {
    case INIT_CMD:
        if (_pasti(jiff_cmd) < 50)
            break;
        printf_gsm(InitCmdTab[n]);
        clr_gsm_rev();
        jiff_cmd = jiffies;
        step = INIT_ACK;
        break;
    case INIT_ACK:
        if (get_gsm_resp("OK")) {
        InitLoop:
            jiff_cmd = jiffies;
            if (InitCmdTab[++n] == NULL) {
                gsm_step = GSM_CHECK;
            } else {
                step = INIT_CMD;
            }
        } else if (_pasti(jiff_cmd) >= 4000) //调整时间
        {
            jiff_cmd = jiffies;
            if (++init_error >= 2) {
                pwrup_error++;
                init_error = 0;
                goto InitLoop;
            }
            jiff_delay = jiffies;
            step = INIT_DELAY;
        }
        break;
    case INIT_DELAY:
        if (_pasti(jiff_delay) > 5 * 1000)
            step = INIT_CMD;
        break;
    default:
        step = INIT_CMD;
    }
}

static void gsm_check(void) {
    u8 *pt;
    u8 csq_buf[3] = {0x00};
    u8 tmp[20];

    static enum {
        CHECK_CSQ,
        CHECK_CACK,
        CHECK_CHINA_SEL,
        CHECK_CHINA_SEL_R,
        CHECK_END
    } step = CHECK_CSQ;
    static u16 jiff_cmd;
    static u16 qweak;
    u8 buf[32];

    if (new_step) {
        qweak = 0;
        step = CHECK_CSQ;
    }
    if (pub_io.b.dvr_open && (s_Hi3520_Monitor.start))
        return;

    switch (step) {
    case CHECK_CSQ:
        printf_gsm("AT+CSQ\r\n"); // Check signal strength
        clr_gsm_rev();
        jiff_cmd = jiffies;
        step = CHECK_CACK;
        break;
    case CHECK_CACK:
        if (_pasti(jiff_cmd) < 2000)
            break;
        if (get_gsm_resp("OK")) {
            read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if ((pt = _strstr(buf, "+CSQ:")) != NULL) {
                logi("csq[%s]", buf);
                _strncpy(csq_buf, pt + 6, 2); ///6
                csq_buf[2] = '\0';
            }
            gs.gsm.squality = atoi((const char *)&csq_buf);
            if ((gs.gsm.squality == 99) || (gs.gsm.squality < 1)) {
                step = CHECK_CACK;
                if (++qweak > 100) {
                    qweak = 0;
                    step = CHECK_CHINA_SEL;
                }
                break;
            }
            step = CHECK_CHINA_SEL;
        } else if (get_gsm_resp("ERROR")) {
            loge("csq[err]");
            step = CHECK_CHINA_SEL;
        } else if (_pasti(jiff_cmd) > 10000) {
            loge("csq[tm.out]");
            jiff_cmd = jiffies;
            gsm_step = GSM_OFF;
        }
        break;
#if 1
    case CHECK_CHINA_SEL: //"AT+COPS=?\r\n",			//锟叫癸拷锟狡讹拷锟斤拷锟叫癸拷锟斤拷通锟斤拷锟叫癸拷锟斤拷锟斤拷  锟斤拷锟斤拷选锟斤拷
        printf_gsm("AT+COPS?\r\n");
        step = CHECK_CHINA_SEL_R;
        clr_gsm_rev();
        break;
    case CHECK_CHINA_SEL_R:
        if (_pasti(jiff_cmd) < 1000)
            break;
        /*
			+COPS:(1,"CHINA
			MOBILE","CMCC",锟斤拷46000锟斤拷,0),(3,"CHN-CUGSM","CU-GSM","46001",0),,(0-4),(0-2)
			OK
			+COPS: 1,2,"CHINA MOBILE",0
			+COPS: 0,0,"CHINA UNICOM GSM"	
			+COPS: 0,0,"CHN-CT",7
			*/
        if (get_gsm_resp("OK")) {
            read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if ((pt = _strstr(buf, "+COPS:")) != NULL) {
                logi("cops[%s]", buf);
                _memset(tmp, 0, 20);
                _memcpy(tmp, pt + 12, 10); ///866856034085371
                if ((_strncmp("CHINA MOBILE", tmp, 11) == 1))
                    sys_cfg.mobile = 1;
                else if ((_strncmp("CHN-UNICOM", tmp, 10) == 1)) //
                    sys_cfg.mobile = 2;
                else
                    sys_cfg.mobile = 3;
            }
            gsm_step = GSM_SETUP;
        } else if (get_gsm_resp("ERROR") || _pastn(jiff_cmd) > 5000) {
            loge("csq[err/tm.out]");
            jiff_cmd = jiffies;
            gsm_step = GSM_SETUP;
        }

        break;
#endif
    default:
        step = CHECK_CSQ;
    }
}

//添加sotket在增加一个场景ID
static u8 *const SetupCmdTab[] =
    {
        "AT+QIDEACT=1\r\n",         //关闭分组数据协议
        "AT+QIDEACT=2\r\n",         //关闭分组数据协议
        "AT+QICSGP=1,1,\"%s\"\r\n", //配置TCP/IP上下文参数 参数1 配置为IPv4协议,也就是配置APN,但是在配置之前必须执行 AT+QIDEACT=1
        "AT+QICSGP=2,1,\"%s\"\r\n", //配置TCP/IP上下文参数 参数2 配置为IPv4协议,也就是配置APN,但是在配置之前必须执行 AT+QIDEACT=2
        "AT+QIACT=1\r\n",           //打开分组数据协议
        "AT+QIACT=2\r\n",           //打开分组数据协议
        NULL};

static void gsm_setup(void) {
    static enum {
        SETUP_CMD,
        SETUP_ACK
    } step = SETUP_CMD;
    static u8 n;
    static u32 jbak;
    static u8 cnt = 0;

    if (new_step) {
        n = 0;
        cnt = 0;
        step = SETUP_CMD;
        gc.gsm.reset = false; //不需要重启模块
        gs.sms.ring = false;  //复位后读取短信息
    }

    switch (step) {
    case SETUP_CMD:
        if (_strncmp(SetupCmdTab[n], "AT+QICSGP", 9) == 0) {
            printf_gsm(SetupCmdTab[n], mx.apn);
        } else
            printf_gsm(SetupCmdTab[n]);
        //printf_gsm(SetupCmdTab[n]);
        clr_gsm_rev();
        jbak = jiffies;
        step = SETUP_ACK;
        break;
    case SETUP_ACK:
        if (_pasti(jbak) < 200)
            break;

        if (get_gsm_resp("OK")) {
            if (SetupCmdTab[++n] == NULL) {
                pwrup_error = 0;
                turn_on_cnt = 0;
                gsm_step = GSM_CONNECT;
            } else
                step = SETUP_CMD;
        } else if (get_gsm_resp("ERROR")) //一定要重启吗
        {
            if (++cnt > 2) {
                gsm_step = GSM_OFF;
                cnt = 0;
            } else {
                n = 0;
                step = SETUP_CMD;
            }
        } else if (_pastn(jbak) >= 90000) {
            gsm_step = GSM_OFF;
        }

        break;
    default:
        step = SETUP_CMD;
    }
}

#define connect1_succ() (get_gsm_resp("+QIOPEN: 1,0")) //OK
#define connect2_succ() (get_gsm_resp("+QIOPEN: 2,0")) //OK

#define connect_fail() (get_gsm_resp("ERROR"))
#define connect1_fail() (get_gsm_resp("1, CONNECT FAIL"))
#define connect2_fail() (get_gsm_resp("2, CONNECT FAIL"))
#define connect_ready() (get_gsm_resp("ALREADY CONNECT"))
#define connect2_linking() (get_gsm_resp("+QIOPEN: 2,563"))

//传递第n路IP；拨号
static void gprs1_connect(u8 netn) {
    static enum {
        DIA_STATUS,
        DIA_SACK,
        DIA_OPEN,
        DIA_OACK,
        DIA_END
    } step = DIA_STATUS;
    static u32 jbak;

    if (gs.gprs1.new_step) {
        step = DIA_STATUS;
    }
    if (server_cfg.main_port == 0)
        return;

    switch (step) {
    case DIA_STATUS:
        printf_gsm("AT+QICLOSE=1\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = DIA_SACK;
        break;
    case DIA_SACK:
        if (_pastn(jbak) <= 1000)
            break;

        if (get_gsm_resp("ERROR") || _pastn(jbak) >= 5000)
            goto cfail;
        else {
            if (upgrade_m.enable) {
                step = DIA_OPEN;
            } else {
                step = DIA_OPEN;
            }
        }
        break;
    case DIA_OPEN:
        if (mx.protocol == UDP) {
            printf_gsm("AT+QIOPEN=1,1,\"UDP\",\"%s\",%u,0,1\r\n", server_cfg.main_ip, server_cfg.main_port);
            //printf_gsm("AT+QIOPEN=1,1,\"UDP\",\"%s\",%u,0,1\r\n" , mx.addr, server_cfg.main_port);
        } else {
            printf_gsm("AT+QIOPEN=1,1,\"TCP\",\"%s\",%u,0,1\r\n", server_cfg.main_ip, server_cfg.main_port);
            //printf_gsm("AT+QIOPEN=1,1,\"TCP\",\"%s\",%u,0,1\r\n" , mx.addr, mx.port);
        }
        lm.link_cnt++;

        clr_gsm_rev();
        AtStringFind1("+QIOPEN: 1,", "1, CONNECT FAIL", "1, ALREADY CONNECT"); //+QIOPEN: 1,0
        jbak = jiffies;
        step = DIA_OACK;
        break;
    case DIA_OACK:
        if ((AtEngine1.exe_status == AT_EVENT_CATCHED1) || (AtEngine1.exe_status == AT_EVENT_CATCHED3)) {
            //off_times = 0;
            gs.gprs1.connect_times = 0;
            gs.gprs1.stat = NET_ONLINE;
            gs.gprs1.login = false;
            gs.sms.mode = false;
            gs.gprs1.login = false;
            gs.gprs1.quick_first = true; //140813
            clr_gsm_send();
            clr_gsm_rev();
            clr_gprs_send();
            step = DIA_STATUS;
            gsm_step = GSM_CONNECT;

            sync_net_online();
            break;
        } else if ((AtEngine1.exe_status == AT_EVENT_CATCHED2) || connect_fail() || _pastn(jbak) >= (60000 * 1uL)) //
        {
        cfail:
            jbak = jiffies;
            gs.gprs1.stat = NET_LOST;
            if (++gs.gprs1.connect_times % 6 == 0) {
                gsm_step = GSM_RECV_SMS;
            } else if ((gs.gprs1.connect_times % 3) == 0) {
                gs.gprs1.stat = NET_LOST;
            } else
                step = DIA_STATUS;
            break;
        }

        if (gs.phone > PS_IDLE || gc.phone > PC_NULL) {
            if (get_gsm_resp("OK") || _pastn(jbak) >= 10 * 1000uL)
                gsm_step = GSM_DISCONNECT;
        }
        break;
    default:
        break;
    }
}

#define NET_SEND_1_OK "OK"

/***************************************************************************
* 名称:    gsm_online_data ()
* 功能：GPRS 在线状态下的模块管理
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static void gprs1_send_data(void) {
    static u8 wfail;
    static u16 len;
    static u8 *datap;
    static u16 jbak;
    u8 dh, dl;

    static enum enum_gsm_data {
        DATA_IDLE,
        DATA_WRITE,
        DATA_SEND,
        DATA_WACK,
    } step = DATA_IDLE;

    if (gs.gprs1.new_step) {
        step = DATA_IDLE;
    }

    switch (step) {
    case DATA_IDLE:
        if (gs.gprs1.stat == NET_LOST) {
            gs.gprs1.stat = NET_CONNECT;
            break;
        }

        if (_pastn(gsm_rev_jfs) > 3500 && gsm_rev_tail != gsm_rev_head) {
            clr_gsm_rev();
        }

        if (!idle_gsm())
            break;

        if ((len = subp_gprs_send(gprs_send_head, gprs_send_tail)) > 0)
            step = DATA_WRITE;
        break;
    case DATA_WRITE:
        printf_gsm("AT+QISEND=1,%d\r", len); //AT+CIPSEND=26
        clr_gsm_rev();
        jbak = jiffies;
        AtStringFind1(">", "ERROR", 0);
        step = DATA_SEND;
        break;
    case DATA_SEND:
        if (AtEngine1.exe_status == AT_EVENT_CATCHED1) {
            unsigned int i;
            datap = gprs_send_tail;
            for (i = 0; i < len; i++) {
                if (over_gsm_send())
                    goto _w_fail;

                if (u1_m.cur.b.debug == true) {
                    dh = *datap >> 4;   //取高四位
                    dl = *datap & 0x0f; //取低四位

                    if (dh > 0x09)
                        dh = dh + 0x37;
                    else
                        dh = dh + 0x30;

                    if (dl > 0x09)
                        dl = dl + 0x37;
                    else
                        dl = dl + 0x30;

                    write_uart1_hex(dh);
                    write_uart1_hex(dl);
                }
                *gsm_send_head = *datap;

                incp_gsm_send(gsm_send_head, 1);
                incp_gprs_send(datap, 1);
            }
            if (u1_m.cur.b.debug == true) {
                write_uart1_hex(0x0d);
                write_uart1_hex(0x0a);
            }
            jbak = jiffies;
            step = DATA_WACK;
            AtStringFind1("OK", "ERROR", 0);
            //
        } else if (_pasti(jbak) >= 10 * 1000 || get_gsm_resp("ERROR"))
            goto _w_fail;
        break;
    case DATA_WACK:
        if (AtEngine1.exe_status == AT_EVENT_CATCHED1) {
            wfail = 0;
            gprs_send_tail = datap;
            step = DATA_IDLE;
            gs.gprs1.stat = NET_ONLINE;
            AtEngine1.exe_status = STATUS_IDLE;
        } else if (AtEngine1.exe_status == AT_EVENT_CATCHED2) {
        _w_fail:
            step = DATA_IDLE;
            AtEngine1.exe_status = STATUS_IDLE;
            if (++wfail > 3) {
                clr_gprs_send();
                clr_gsm_send();
                gs.gprs1.stat = NET_LOST;
                break;
            }
        } else if (_pasti(jbak) >= 10 * 1000) {
            step = DATA_IDLE;
            gs.gprs1.stat = NET_ONLINE;
            gprs_send_tail = datap;
            AtEngine1.exe_status = STATUS_IDLE;
        }

        break;
    default:
        step = DATA_IDLE;
        break;
    }
}

//203.086.009.246
//去前导名
void clear_ip_zero(u8 *dst, u8 *src) {
    u8 i;
    u8 len = 0;
    bool iz = false;

    len = _strlen(src);
    for (i = 0; i < len; i++) {
        if (iz == false) {
            if (*src == 0x30) {
                src++;
                continue;
            }
            if (*src == '.')
                iz = false;
            else
                iz = true;
        }
        if (*src == '.')
            iz = false;
        *dst = *src;
        dst++;
        src++;
    }
}

//锟斤拷锟捷碉拷n路IP锟斤拷锟斤拷锟斤拷
static void gprs2_connect(u8 netn) {
    static enum {
        DIA_STATUS,
        DIA_SACK,
        DIA_OPEN,
        DIA_OACK,
        DIA_END
    } step = DIA_STATUS;
    static u32 jbak;
    //    u8 clearip[24] = {0};

    if (gs.gprs2.new_step) {
        step = DIA_STATUS;
    }
    if (server_cfg.bak_port == 0)
        return;
    switch (step) {
    case DIA_STATUS:
        //printf_gsm("AT+QISTATE\r\n");
        printf_gsm("AT+QICLOSE=2\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = DIA_SACK;
        break;
    case DIA_SACK:
        if (_pastn(jbak) <= 1000)
            break;

        if (get_gsm_resp("ERROR") || _pastn(jbak) >= 5000)
            goto c2fail;
        else {
            if (upgrade_m.enable) {
                step = DIA_OPEN;
            } else {
                step = DIA_OPEN;
            }
        }
        break;
    case DIA_OPEN: {
        //clear_ip_zero(clearip, server_cfg.bak_ip);	//
        if (mx.protocol == UDP) {
            //printf_gsm("AT+QIOPEN=2,2,\"UDP\",\"%s\",%u,0,1\r\n" , server_cfg.bak_ip, server_cfg.bak_port);
            printf_gsm("AT+QIOPEN=2,2,\"UDP\",\"%s\",%u,0,1\r\n", server_cfg.bak_ip, server_cfg.bak_port);
        } else {
            //printf_gsm("AT+QIOPEN=2,2,\"TCP\",\"%s\",%u,0,1\r\n" , server_cfg.bak_ip, server_cfg.bak_port);
            printf_gsm("AT+QIOPEN=2,2,\"TCP\",\"%s\",%u,0,1\r\n", server_cfg.bak_ip, server_cfg.bak_port);
        }

        lm.link_cnt++;
        AtStringFind2("+QIOPEN: 2,", "2, CONNECT FAIL", "2, ALREADY CONNECT");
        clr_gsm_rev();
        jbak = jiffies;
        step = DIA_OACK;
        break;
    }
    case DIA_OACK:
        //if (connect2_succ())
        if ((AtEngine2.exe_status == AT_EVENT_CATCHED1) || (AtEngine2.exe_status == AT_EVENT_CATCHED3)) {
            //off_times = 0;
            gs.gprs2.connect_times = 0;
            gs.gprs2.stat = NET_ONLINE;
            gs.gprs2.login = false;
            gs.sms.mode = false;
            gs.gprs2.login = false;
            gs.gprs2.quick_first = true; //140813
            clr_gsm_send();
            clr_gsm_rev();
            clr_gprs_send();
            step = DIA_STATUS;
            lm.IP2_status = SERVER_COAL;
            gsm_step = GSM_CONNECT;
            AtEngine2.exe_status = STATUS_IDLE;
            sync_net_online();
            break;
        } else if ((AtEngine2.exe_status == AT_EVENT_CATCHED2) || connect_fail() || _pastn(jbak) >= (60000 * 1uL)) //
        {
        c2fail:
            gs.gprs2.stat = NET_LOST;
            AtEngine2.exe_status = STATUS_IDLE;
            gs.gprs2.connect_times++;
            if ((gs.gprs2.connect_times % 2) == 0) {
                printf_gsm("AT+QICLOSE=2\r\n");
                AtEngine2.exe_status = STATUS_IDLE;
                step = DIA_STATUS;
            } else if ((gs.gprs2.connect_times % 15) == 0) {
                gsm_step = GSM_REQUEST_ERROR;
            } else
                step = DIA_STATUS;
            break;
        }

        break;
    default:
        break;
    }
}

/***************************************************************************
* 锟斤拷锟斤拷:    gprs2_send_data ()
* 锟斤拷锟杰ｏ拷GPRS 锟斤拷锟斤拷状态锟铰碉拷模锟斤拷锟斤拷锟?
* 锟斤拷诓锟斤拷锟斤拷锟斤拷锟?                         
* 锟斤拷锟节诧拷锟斤拷锟斤拷锟斤拷
****************************************************************************/
static void gprs2_send_data(void) {
    static u8 wfail;
    static u16 len;
    static u8 *datap;
    static u16 jbak;
    u8 dh, dl;

    static enum enum_gsm_data {
        DATA_IDLE,
        DATA_WRITE,
        DATA_SEND,
        DATA_WACK,
    } step = DATA_IDLE;

    if (gs.gprs2.new_step) {
        step = DATA_IDLE;
    }

    switch (step) {
    case DATA_IDLE:
        if (gs.gprs2.stat == NET_LOST) {
            gs.gprs2.stat = NET_CONNECT;
            break;
        }

        if (_pastn(gsm_rev_jfs) > 3500 && gsm_rev_tail != gsm_rev_head) {
            clr_gsm_rev();
        }

        if (!idle_gsm())
            break;

        if ((len = subp_gprs_send(gprs2_send_head, gprs2_send_tail)) > 0)
            step = DATA_WRITE;
        break;
    case DATA_WRITE:
        printf_gsm("AT+QISEND=2,%d\r", len); //AT+CIPSEND=26
        clr_gsm_rev();
        jbak = jiffies;
        step = DATA_SEND;
        AtStringFind2(">", "ERROR", 0);
        break;
    case DATA_SEND:
        //if (get_gsm_resp(">") )   //&& _pastn(jbak) > 200
        if (AtEngine2.exe_status == AT_EVENT_CATCHED1) {
            unsigned int i;
            datap = gprs2_send_tail;
            for (i = 0; i < len; i++) {
                if (over_gsm_send())
                    goto _w2_fail;

                if (u1_m.cur.b.debug == true) {
                    dh = *datap >> 4;   //取锟斤拷锟斤拷位
                    dl = *datap & 0x0f; //取锟斤拷锟斤拷位

                    if (dh > 0x09)
                        dh = dh + 0x37;
                    else
                        dh = dh + 0x30;

                    if (dl > 0x09)
                        dl = dl + 0x37;
                    else
                        dl = dl + 0x30;

                    write_uart1_hex(dh);
                    write_uart1_hex(dl);
                }

                *gsm_send_head = *datap;

                incp_gsm_send(gsm_send_head, 1);
                incp_gprs2_send(datap, 1);
            }
            if (u1_m.cur.b.debug == true) {
                write_uart1_hex(0x0d);
                write_uart1_hex(0x0a);
            }
            jbak = jiffies;
            step = DATA_WACK;
            AtStringFind2("OK", "ERROR", 0);
            //
        } else if (_pasti(jbak) >= 10 * 1000 || (AtEngine2.exe_status == AT_EVENT_CATCHED2)) {
            AtEngine2.exe_status = STATUS_IDLE;
            goto _w2_fail;
        }
        break;
    case DATA_WACK:
        if (AtEngine2.exe_status == AT_EVENT_CATCHED1) {
            wfail = 0;
            gprs2_send_tail = datap;
            step = DATA_IDLE;
            gs.gprs2.stat = NET_ONLINE;
            AtEngine2.exe_status = STATUS_IDLE;
        } else if (AtEngine2.exe_status == AT_EVENT_CATCHED2) {
        _w2_fail:
            step = DATA_IDLE;
            AtEngine2.exe_status = STATUS_IDLE;
            if (++wfail > 3) {
                clr_gprs2_send();
                clr_gsm_send();
                gs.gprs2.stat = NET_LOST;
                break;
            }
        } else if (_pasti(jbak) >= 10 * 1000) {
            step = DATA_IDLE;
            gs.gprs2.stat = NET_ONLINE;
            loge("****GPRS2 send fail****");
            gprs2_send_tail = datap;
            AtEngine2.exe_status = STATUS_IDLE;
        }

        break;
    default:
        step = DATA_IDLE;
        break;
    }
}

void test_send_sms(void) {
    u8 phone[11] = {0};
    u8 phone_len = 0;
    static bool test = false;

    if (test == true) {
        _memcpy(phone, "16620820156", 11);
        phone_len = 11;
        logd("sms-x send save txt");
        sms_send(SMS_TXT, phone, phone_len, "trwy sms!", 9);
        test = false;
    }
}

/***************************************************************************
* 名     称:  gsm_connect
* 功     能：建立TCP/IP连接
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void gsm_run_task(void) {
    static enum {
        CON_IDLE,
        CON_CSQ,
        CON_QACK,
        CON_CREG,
        CON_RACK,
        CON_END
    } step = CON_IDLE;

    u8 csq_buf[3] = {0x00};
    u16 delay_time;
    static GPRS_STATE gs1_old;
    static GPRS_STATE gs2_old;
    static u32 jbak;
    //    u16 len;

    if (pub_io.b.dvr_open && (s_Hi3520_Monitor.start)) { //如果TR9未启动；则往下执行
        gsm_step = GSM_OFF;
        logi("hi3520 EC20 power up");
        return;
    }

    test_send_sms();

    if (new_step) {
        clr_gsm_rev();
        step = CON_IDLE;
        self_test.uGsmU = '0';
    }

    switch (step) {
    case CON_IDLE:
        if (_pastn(gsm_rev_jfs) > 200)
            clr_gsm_rev();

        if (!idle_gsm())
            break;

#if (P_SOUND == SOUND_BASE)
        if (tts_m.playing || tts_m.new_tts) {
            if (tts_m.new_tts == true && _coveri(tts_m.n_tick) > 15) {
                {
                    tts_m.new_tts = false;
                    goto con_tts_out;
                }
            }

            if (tts_m.new_tts == true && tts_m.playing == false && _coveri(tts_m.n_tick) > 1) {
                gsm_step = GSM_TTS_HANDLE;
                break;
            } else if (tts_m.playing) {
                delay_time = (tts_m.exit_time < 5 || tts_m.exit_time > 300) ? 5 : tts_m.exit_time;
                if (tts_m.aud_end || _coveri(tts_m.p_tick) > delay_time) {
                con_tts_out:
                    tts_m.aud_end = false;
                    tts_m.playing = false;
                    if (tts_m.new_tts == false) {
                        _memset((u8 *)&tts_m, 0x00, sizeof(tts_m));
                        call_circuit_ctrl(CALL_IDLE);
                        break;
                    }
                }
            }
        }
#endif
        if (gs.sms.ring) {
            gsm_step = GSM_RECV_SMS;
            break;
        }

        if (_coveri(qcheck) >= 90) {
            step = CON_CSQ;
            qcheck = tick;
            break;
        }

        if (_coveri(rcheck) >= 150) {
            step = CON_CREG;
            break;
        }

        if (gc.gsm.reset || gc.gsm.pwr_down) {
            gsm_step = GSM_OFF;
            break;
        }

        if (gs.gsm.squality < CSQ_V)
            break;

        if (sms_s.sms_new && sms_s.is_txt && _coveri(sms_s.tick) >= 1) {
            gsm_step = GSM_SEND_SMS_TXT;
            logd("sms-1 connect GSM_SEND_SMS_TXT enter");
            break;
        }

        if (sms_s.sms_new && !sms_s.is_txt && _coveri(sms_s.tick) >= 2) {
            gsm_step = GSM_SEND_SMS_PDU;
            break;
        }

        if (_covern(read_sms_tick) > 320 && wait_gsm()) //定时读短信
        {
            read_sms_tick = tick;
            //  gs.sms.ring = true;
            //gs.sms.ring = false ;
        }

        gs.gprs1.new_step = (gs.gprs1.stat == gs1_old) ? false : true;

        switch (gs.gprs1.stat) {
        case NET_LOST:
            gprs1_connect(1);
            break;
        case NET_CONNECT:
            gprs1_connect(1);
            break;
        case NET_ONLINE:
            if ((/*len = */ subp_gprs_send(gprs_send_head, gprs_send_tail)) > (GPRS_SEND_SIZE - 8)) //test
            {
                clr_gprs_send();
            }

            if (((/*len =*/subp_gprs_send(gprs_send_head, gprs_send_tail)) > 0) && (_pasti(gs.gprs2.stim) >= 50)) //
            {
                gs.gprs1.stat = NET_SEND;
                gs.gprs1.stim = jiffies;
                break;
            }

            break;
        case NET_SEND:
            gprs1_send_data();
            break;
        default:
            break;
        }
        gs1_old = gs.gprs1.stat;

#if 1
        gs.gprs2.new_step = (gs.gprs2.stat == gs2_old) ? false : true;

        switch (gs.gprs2.stat) {
        case NET_LOST:
            gprs2_connect(1);
            break;
        case NET_CONNECT:
            gprs2_connect(1);
            break;
        case NET_ONLINE:
            if ((/*len =*/subp_gprs_send(gprs2_send_head, gprs2_send_tail)) > (GPRS_SEND_SIZE - 8)) //test
            {
                clr_gprs_send();
            }

            if (((/*len =*/subp_gprs_send(gprs2_send_head, gprs2_send_tail)) > 0) && (_pasti(gs.gprs1.stim) >= 50)) {
                gs.gprs2.stat = NET_SEND;
                gs.gprs2.stim = jiffies;
                break;
            }

            break;
        case NET_SEND:
            gprs2_send_data();
            break;
        default:
            break;
        }
        gs2_old = gs.gprs2.stat;
#endif
        break;
    case CON_CSQ:
        printf_gsm("AT+CSQ\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = CON_QACK;
        break;
    case CON_QACK:
        if (get_gsm_resp("OK")) {
            u8 *pt;
            u8 buf[32];
            read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if ((pt = _strstr(buf, "+CSQ:")) != NULL) {
                _strncpy(csq_buf, pt + 6, 2);
                csq_buf[2] = '\0';
                gs.gsm.squality = atoi((const char *)&csq_buf);
                if (gs.gsm.squality < 5) {
                    self_test.uGsmU = '2';
                } else if ((self_test.uGsmU == '2') && (15 < gs.gsm.squality)) {
                    self_test.uGsmU = '0';
                }
            }

            qcheck = tick;
            step = CON_IDLE;
        } else if (get_gsm_resp("ERROR") || _pastn(jbak) > 5000)
            gsm_step = GSM_OFF;
        break;
    case CON_CREG:
        printf_gsm("AT+CREG?\r\n");
        clr_gsm_rev();
        rcheck = tick;
        jbak = jiffies;
        step = CON_RACK;
        break;
    case CON_RACK:
        if (get_gsm_resp("OK")) {
            u8 buf[32];
            read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
            step = CON_IDLE;
        } else if (get_gsm_resp("ERROR") || _pastn(jbak) > 1000)
            step = CON_IDLE;
        break;
    default:
        step = CON_IDLE;
    }
}

#if 0
static void gsm_recv_sms(void)
{
    u8* pt;
    u8* tok;
    u8 buf[32];
    u16 i;
    bool is_txt = false;
    u16 len = 0;

    u16 rbox_len;
    u8 phone_len;
    u16 ret_len;

    u8 rbox[MAX_SMS_PDU] = {0};
    u8 send[MAX_SMS_PDU] = {0};
    u8 phone[PHONELEN] = {0};

    static enum
    {
        TXT_SETUP,
        TXT_ACK,
      //  TXT_CSCS,			//
       // TXT_CSCS_ACK,		//
        TXT_RECV,
        TXT_GET,
        SMS_CREG,
        SMS_ACK,
        SMS_DEL,
        SMS_DELAY
    } step = TXT_SETUP;
    static u8 n;
    static u16 jbak;

    if (new_step)
    {
        n = 1;
        jbak = jiffies;
        step = TXT_SETUP;
    }

    switch (step)
    {
        case TXT_SETUP:
            printf_gsm("AT+CMGF=1\r\n");   //AT+CMGD	//文本模式
            clr_gsm_rev();
            jbak = jiffies;
            step = TXT_ACK;
            break;
        case TXT_ACK:
            if (get_gsm_resp("OK"))
				step = TXT_RECV;
            else if (_pasti(jbak) > 5*1000)
                gsm_step = GSM_OFF;	
            break;
#if 0
		case TXT_CSCS:	/**新加内容**/	
            printf_gsm("AT+CMGL=\"REC UNREAD\"\r\n");  //AT+CSCS=鈥淕SM鈥?"AT+CSCS=\"GSM\"\r\n"
            clr_gsm_rev();
            jbak = jiffies;
            step = TXT_CSCS_ACK;
            break;
        case TXT_CSCS_ACK:
            if (get_gsm_resp("OK"))
                step = TXT_RECV;
            else if (_pastn(jbak) > 5*1000)
                gsm_step = TXT_CSCS_ACK;
            break;   /****/
#endif 	
        case TXT_RECV:
            printf_gsm("AT+CMGR=%d\r\n", n);
            ini_gsm_gprs_rev();
			//AtStringFind1("OK\r\n","+CMGR:\r\n","ERROR");   //+QIOPEN: 1,0
            jbak = jiffies;
            step = TXT_GET;
            break;
        case TXT_GET:
			
            if ((get_gsm_resp("OK\r\n")) || get_gsm_resp("AT+CMGR:\r\n"))
            {
           
				/******/
				//AtEngine1.exe_status = STATUS_IDLE;
				//read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
				//buf[sizeof(buf) - 1] = '\0';
				/************/
               if ((pt = get_gsm_resp("+CMGR:")) == NULL)
				{
					goto rover;
				}
 
                gsm_rev_tail = pt;
                pt = get_gsm_field(1);
                read_gsm(buf, pt+1, sizeof(buf));
                buf[sizeof(buf) - 1] = '\0';
                tok = _strtok(buf, "\"");
                _strcpy(phone, tok);

                pt = get_gsm_field(7);

                is_txt = true;    //chwsh
                if ((pt = get_gsm_line(1)) == NULL)         // GET THE MESSAGE
                    goto rover;
				
                rbox_len= 0;
                _memset(rbox, 0x00, MAX_SMS_PDU);
                for (i = 0; i < sizeof(rbox) - 1; i++)
                {
                    read_gsm(buf, pt, 2);
                    if (_strncmp(buf, "\r\n", 2) == 0)
                        break;
                  
                    //1440193334069移动物联网卡在接收到PDU短信时,短信内容会以"FEFF",开头,2018-6-26
                     if( (rbox_len < 4)&&(!useless) )
                     {
                         rbox[rbox_len] = *pt;
                         rbox_len++;
                         incp_gsm_rev(pt, 1);
                         if(_strncmp((u8*)&rbox, (u8 *)&"FEFF",4) == 0)
                         {
                            rbox_len = 0;
                            useless = true;
                         } 
                          
                     }
                     else
                     {
                       rbox[rbox_len] = *pt;
                       incp_gsm_rev(pt, 1);
                      if (pt == gsm_rev_head)
                          break;
                       rbox_len++;
                     }                 
                }

                rbox[rbox_len] = '\0';

                if(rbox_len<3||rbox_len>MAX_SMS_PDU)    //长度超出设置范围
                {
                    goto rover;
                }
                del_phone_code86(phone);
                phone[PHONELEN-1] = '\0';
                phone_len = _strlen(phone);
                _memset( send, 0, MAX_SMS_PDU);
		
                if(is_txt)
                {
                    sms_s.cmp_en = true;
					logd("接收到信息");
                    if(parse_sms(rbox, send, SERVER)==CMD_ACCEPT)
                    {
                        ret_len = _strlen(send);
                        if(ret_len>200)
                            goto rover;
                        logd("sms-0 send save txt");
                        sms_send(SMS_TXT, phone, phone_len,send,ret_len);	
                        step = SMS_DEL;
                        sms_format_judge = 1 ; //TXT格式信息 2018-1-16
                    }
                    else
                    {
                        goto SMSPDU_FORMAT;
                    }
                }
                else
                {
                SMSPDU_FORMAT:
                    rbox_len = rbox_len-rbox_len%2;
                    len = rbox_len/2;
                    for(i=0; i<len; i++)
                    {
                        rbox[i] = _chartohex(rbox[2*i],rbox[(2*i)+1]);
                    }
                    rbox[i++] = 0;
                    rbox[i++] = 0;
                    rbox_len = len;
                    logd("sms not txt");
                    sms_format_judge = 0 ;   //PUD格式信息2018-1-16
                    //中文字符转换为 英文字符 rbox 内存区
                    len = pdu_sms_to_txt(rbox, rbox_len);
                    rbox[len++] = 0;
                    rbox[len++] = 0;
                    sms_s.cmp_en=  true;

                    if(parse_sms(rbox, send, SERVER)==CMD_ACCEPT)
                    {
                        ret_len = _strlen(send);
                        if(ret_len>160)
                            goto rover;

                        sms_send(SMS_TXT, phone, phone_len, send, ret_len);
                    }
                }

                step = SMS_DEL;
                //step = SMS_CREG;
                break;
            }
			
            if (get_gsm_resp("ERROR") || _pasti(jbak) >= 3000)
            {
            rover:
                jbak = jiffies;
                step = SMS_DEL;
            }
            break;
			/*
			if ((AtEngine1.exe_status==AT_EVENT_CATCHED3) || _pastn(jbak) >= 2500)//get_gsm_resp("ERROR")
            {
            rover:
				//esp8266_debug_print("$SIM RECE: timeout!\r\n",21);	
				AtEngine1.exe_status = STATUS_IDLE;
                jbak = jiffies;
                step = SMS_DEL;
            }
            break;*/
        case SMS_DEL:
            printf_gsm("AT+CMGD=%d\r\n", n);
            clr_gsm_rev();
            jbak = jiffies;
            step = SMS_DELAY;
            break;
        case SMS_DELAY:
            if(get_gsm_resp("OK"))
            {
                t_rst.del = true;
                logd("***** n = %d *****",n);
                if (n++ > 3)
                {
                    gs.sms.ring = false;
                    gsm_step = GSM_CONNECT;
                }
                else
                    step = TXT_SETUP;
            }
            else if((get_gsm_resp("ERROR"))|| (_pasti(jbak) >= 5000))
            {
                step = TXT_SETUP;
            }
            break;
        default:
            step = TXT_RECV;
    }
}
#else
static void gsm_recv_sms(void) {
    u8 *pt;
    u8 *tok;
    //u8 buf[32];
    u8 buf[100];
    u16 i;
    bool is_txt = false;
    u16 len = 0;

    u16 rbox_len;
    //    u8 phone_len;
    u16 ret_len;

    u8 rbox[MAX_SMS_PDU] = {0};
    u8 send[MAX_SMS_PDU] = {0};
    u8 phone[PHONELEN] = {0};

    static enum {
        TXT_SETUP,
        TXT_ACK,
        TXT_CSCS,
        TXT_CSCS_ACK,
        TXT_CSCS_ACK_1,
        TXT_RECV,
        TXT_GET,
        SMS_CREG,
        SMS_ACK,
        SMS_DEL,
        SMS_DELAY
    } step = TXT_SETUP;
    static u8 n;
    static u32 jbak;

    if (new_step) {
        n = 0;
        jbak = jiffies;
        step = TXT_SETUP;
    }

    switch (step) {
    case TXT_SETUP:
        printf_gsm("AT+CMGF=1\r\n"); //AT+CMGD
        clr_gsm_rev();
        jbak = jiffies;
        step = TXT_ACK;
        break;
    case TXT_ACK:
        if (get_gsm_resp("OK"))
            step = TXT_CSCS;
        else if (_pastn(jbak) > 5 * 1000)
            gsm_step = GSM_OFF;
        break;
    case TXT_CSCS:
        printf_gsm("AT+CMGL=\"REC UNREAD\"\r\n"); //AT+CSCS=“GSM” "AT+CSCS=\"GSM\"\r\n"
        clr_gsm_rev();
        jbak = jiffies;
        step = TXT_CSCS_ACK;
        break;
    case TXT_CSCS_ACK:
        if (get_gsm_resp("OK"))
            step = TXT_CSCS_ACK_1;
        if (_pastn(jbak) > 10 * 1000) {
            gsm_step = (enum E_GSM_STEP)TXT_CSCS_ACK_1;
        }
        break;

    case TXT_CSCS_ACK_1:
        /**********************获取最新短信索引*	***************************
			read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
			
            buf[sizeof(buf) - 1] = '\0';
            
            if((pt = _strstr(buf, "+CMGL:")) != NULL)
			{	
				_memcpy(&n, pt+7, 1);
				n -= 0x30 ; 
				step = TXT_RECV;
			}
			else
				step = TXT_CSCS ;
			 break;	
		*******************************************************************/
        read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        if ((pt = _strstr(buf, "+CMGL:")) != NULL) {
            _memcpy(&n, pt + 7, 1);
            n -= 0x30;
        } else {
            step = TXT_CSCS_ACK_1;
        }
    /***********************************************************************/
    case TXT_RECV:
        printf_gsm("AT+CMGR=%d\r\n", n); // n 短信索引
        ini_gsm_gprs_rev();
        //AtStringFind1("OK","+CMGR:","ERROR");   //+QIOPEN: 1,0

        jbak = jiffies;
        step = TXT_GET;
        break;
    case TXT_GET:
        //  if ( (get_gsm_resp("OK\r\n"))    || get_gsm_resp("+CMGR:\r\n"))
        if ((get_gsm_resp("OK\r\n")) || get_gsm_resp("+CMGR:\r\n"))
        //if( (AtEngine1.exe_status==AT_EVENT_CATCHED1) || (AtEngine1.exe_status==AT_EVENT_CATCHED2) )
        {
            AtEngine1.exe_status = STATUS_IDLE;
            //esp8266_debug_print("$SIM RECE: null!\r\n",18);
            read_gsm(buf, gsm_rev_tail, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            if ((pt = get_gsm_resp("+CMGR:")) == NULL) {
                goto rover;
            }
            gsm_rev_tail = pt;
            pt = get_gsm_field(1);
            read_gsm(buf, pt + 1, sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';
            tok = _strtok(buf, "\"");
            _strcpy(phone, tok);

            pt = get_gsm_field(7);

            is_txt = true;                      //chwsh
            if ((pt = get_gsm_line(1)) == NULL) // GET THE MESSAGE
                goto rover;

            rbox_len = 0;
            _memset(rbox, 0x00, MAX_SMS_PDU);

            for (i = 0; i < sizeof(rbox) - 1; i++) {
                read_gsm(buf, pt, 2);
                if (_strncmp(buf, "\r\n", 2) == 0)
                    break;

                //1440193334069?????????????????PDU?????,???????????"FEFF",???,2018-6-26
                if ((rbox_len < 4) && (!useless)) {
                    rbox[rbox_len] = *pt;
                    rbox_len++;
                    incp_gsm_rev(pt, 1);
                    if (_strncmp((u8 *)&rbox, (u8 *)&"FEFF", 4) == 0) {
                        rbox_len = 0;
                        useless = true;
                    }
                } else {
                    rbox[rbox_len] = *pt;
                    incp_gsm_rev(pt, 1);
                    if (pt == gsm_rev_head)
                        break;
                    rbox_len++;
                }
            }

            rbox[rbox_len] = '\0';

            if (rbox_len < 3 || rbox_len > MAX_SMS_PDU) //??????????÷?Χ
            {
                goto rover;
            }

            del_phone_code86(phone);
            phone[PHONELEN - 1] = '\0';
            //phone_len = _strlen(phone);
            _memset(send, 0, MAX_SMS_PDU);
            if (is_txt) {
                sms_s.cmp_en = true;
                if (parse_sms(rbox, send, SERVER) == CMD_ACCEPT) //调用接收函数
                {
                    ret_len = _strlen(send);
                    if (ret_len > 200) {
                        goto rover;
                    }
                    logd("sms-0 send save txt");
                    // sms_send(SMS_TXT, phone, phone_len, send, ret_len);		//发送数据
                    step = SMS_DEL;
                    sms_format_judge = 1;              //TXT?????? 2018-1-16
                    printf_gsm("AT+CMGD=%d,1\r\n", n); //删除所有已读短信
                    n = 0;
                } else {
                    goto SMSPDU_FORMAT;
                }
            } else {
            SMSPDU_FORMAT:

                rbox_len = rbox_len - rbox_len % 2;
                len = rbox_len / 2;
                for (i = 0; i < len; i++) {
                    rbox[i] = _chartohex(rbox[2 * i], rbox[(2 * i) + 1]);
                }
                rbox[i++] = 0;
                rbox[i++] = 0;
                rbox_len = len;
                logd("sms not txt");
                sms_format_judge = 0; //PUD??????2018-1-16

                len = pdu_sms_to_txt(rbox, rbox_len);
                rbox[len++] = 0;
                rbox[len++] = 0;
                sms_s.cmp_en = true;
                /* 
                    if(parse_sms(rbox, send, SERVER)==CMD_ACCEPT)
                    {
                        ret_len = _strlen(send);
                        if(ret_len>160)
                            goto rover;

                        sms_send(SMS_TXT, phone, phone_len, send, ret_len);
                    }
                    */
            }

            step = SMS_DEL;
            //step = SMS_CREG;
            break;
        }

        if ((AtEngine1.exe_status == AT_EVENT_CATCHED3) || _pastn(jbak) >= 2500) //get_gsm_resp("ERROR")
        {
        rover:
            //esp8266_debug_print("$SIM RECE: timeout!\r\n",21);
            AtEngine1.exe_status = STATUS_IDLE;
            jbak = jiffies;
            step = SMS_DEL;
        }
        break;

    case SMS_DEL:
        //printf_gsm("AT+CMGD=%d\r\n", n);		//按照索引删除单条短信
        //g_delay_ms(100);
        printf_gsm("AT+CMGD=1,4\r\n"); //删除所有短信
                                       //g_delay_ms(100);
        // printf_gsm("AT+QMGDA=\"DEL ALL\"\r\n");			//删除所有短信
        clr_gsm_rev();
        jbak = jiffies;
        step = SMS_DELAY;
        break;
    case SMS_DELAY:
        if (get_gsm_resp("OK")) {
            t_rst.del = true;
            logi("***** n = %d *****", n);
#if 1
            if (n++ >= 3) {
                gs.sms.ring = false;
                gsm_step = GSM_CONNECT;
                if (Revc_sms_flag) {
                    Set_Awaken.Revc_sms_flag = true; //接收到短信
                    Revc_sms_flag = false;
                }
            } else
                step = TXT_SETUP;
#else
            if (n++ >= 1) {
                gs.sms.ring = false;
                gsm_step = GSM_CONNECT;
                if (Revc_sms_flag) {
                    Set_Awaken.Revc_sms_flag = true; //接收到短信
                    Revc_sms_flag = false;
                }

            } else
                step = TXT_SETUP;

#endif
        } else if ((get_gsm_resp("ERROR")) || (_pastn(jbak) >= 5000)) {
            step = TXT_SETUP;
        }
        break;
    default:
        step = TXT_RECV;
    }
}

#endif

/****************************************************************************
* 名称:    gsm_send_sms_txt ()
* 功能：发送文本短信
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void gsm_send_sms_txt(void) {
    static enum {
        TXT_SETUP,
        TXT_GSM,
        TXT_CSMP,
        TXT_CNMI,
        TXT_SACK,
        TXT_SEND,
        TXT_TRAN,
        TXT_TACK,
    } step = TXT_SEND;
    static u16 jbak;
    static u8 fail;
    //    static u16 tsend;

    if (new_step) {
        fail = 0;
        //        tsend = 0;
        jbak = jiffies;
        step = TXT_SETUP;
    }

    switch (step) {
    case TXT_SETUP:
        printf_gsm("AT+CMGF=1\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = TXT_GSM;
        break;
    case TXT_GSM:
        if (get_gsm_resp("OK")) {
            printf_gsm("AT+CSCS=\"GSM\"\r\n");
            //printf_gsm("AT+CSCS=\"IRA\"\r\n");
            clr_gsm_rev();
            jbak = jiffies;
            step = TXT_CSMP;
        } else if (_pasti(jbak) > 10 * 1000) {
            logi("sms-S NO ok");
            step = TXT_SACK;
            gsm_step = GSM_CONNECT;
            _memset((u8 *)&sms_s, 0x00, sizeof(sms_s));
        }
        break;

        /**************************新加指令**************************
		case TXT_CSMP:		
			if (get_gsm_resp("OK"))
            {
            	printf_gsm("AT+CSMP=17,167,0,241\r\n");	
				clr_gsm_rev();
				jbak = jiffies;
				step = TXT_CNMI;
			}
			else if(_pasti(jbak) > 10*1000)
			{
				logi("sms-1 NO ok");
				step = TXT_SETUP;	
			}	
			break;
		case TXT_CNMI:
			if (get_gsm_resp("OK"))
            {
            	printf_gsm("AT+CNMI=0,1,0,0,0\r\n");	
				clr_gsm_rev();
				jbak = jiffies;
				step = TXT_SACK;
			}
			else if(_pasti(jbak) > 10*1000)
			{
				logi("sms-2 NO ok");
				step = TXT_SETUP;	
			}	
			break;
		***********************************************************/
    case TXT_SACK:
        if (get_gsm_resp("OK"))
            step = TXT_SEND;
        else if (_pasti(jbak) > 5 * 1000)
            gsm_step = GSM_OFF;
        break;

    case TXT_SEND:
        //if (_coveri(tsend) > 10)
        {
            printf_gsm("AT+CMGS=\"%s\"\r\n", &sms_s.phone[0]);
            AtStringFind1(">", "ERROR", 0);
            clr_gsm_rev();
            jbak = jiffies;
            step = TXT_TRAN;
        }
        break;
    case TXT_TRAN:
        //if (get_gsm_resp(">") || _pasti(jbak) >= 3000)
        if ((AtEngine1.exe_status == AT_EVENT_CATCHED1)) {
            clr_gsm_rev();
            write_gsm_txt(sms_s.msg);
            write_gsm_hex(0x1A);
            write_gsm_hex(0x1a);
            jbak = jiffies;
            step = TXT_TACK;
        }
        break;
    case TXT_TACK:
        if (get_gsm_resp("OK")) {
            logi("sms-3 gsm_send ok");
            t_rst.send = true;
        out:
            _memset((u8 *)&sms_s, 0x00, sizeof(sms_s));
            gsm_step = GSM_CONNECT;
            break;
        }
        if (get_gsm_resp("ERROR") || _pasti(jbak) >= 10 * 1000) {
            _memset((u8 *)&sms_s, 0x00, sizeof(sms_s));
            if (++fail < 3) {
                //                tsend = tick;
                gsm_step = GSM_CONNECT;
            } else
                goto out;
            break;
        }
        reduce_gsm_rev();
        break;
    default:
        step = TXT_SEND;
    }
}

/****************************************************************************
* 名称:    gsm_send_sms_pdu ()
* 功能：发送PDU 短信
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void gsm_send_sms_pdu(void) {
    static enum {
        PDU_CSMP,
        PDU_SACK0,
        PDU_CMGF,
        PDU_SACK1,
        PDU_CSCS,
        PDU_SACK2,
        PDU_SEND,
        PDU_TRAN,
        PDU_TACK
    } step = PDU_CSMP;
    static unsigned int jbak;
    u16 len = 0;
    u16 i;
    u8 phone[50] = {0};
    u8 to_bcd[3] = {0x00};

    if (new_step) {
        jbak = jiffies;
        step = PDU_CSMP;
    }

    switch (step) {
    case PDU_CSMP:
        printf_gsm("AT+CSMP=17,167,2,25\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = PDU_SACK0;
        break;
    case PDU_SACK0:
        if (get_gsm_resp("OK"))
            step = PDU_CMGF;
        else if (_pasti(jbak) > 5 * 1000)
            gsm_step = GSM_OFF;
        break;
    case PDU_CMGF:
        printf_gsm("AT+CMGF=1\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = PDU_SACK1;
        break;
    case PDU_SACK1:
        if (get_gsm_resp("OK"))
            step = PDU_CSCS;
        else if (_pasti(jbak) > 5 * 1000)
            gsm_step = GSM_OFF;
        break;
    case PDU_CSCS:
        printf_gsm("AT+CSCS=\"UCS2\"\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = PDU_SACK2;
        break;
    case PDU_SACK2:
        if (get_gsm_resp("OK"))
            step = PDU_SEND;
        else if (_pasti(jbak) > 5 * 1000)
            gsm_step = GSM_OFF;
        break;
    case PDU_SEND:
        len = _strlen(sms_s.phone);
        for (i = 0; i < len; i++) {
            phone[i * 4] = 0x30;
            phone[i * 4 + 1] = 0x30;
            phone[i * 4 + 2] = 0x33;
            phone[i * 4 + 3] = sms_s.phone[i];
        }

        printf_gsm("AT+CMGS=\"%s\"\r", phone);
        clr_gsm_rev();
        jbak = jiffies;
        step = PDU_TRAN;
        break;
    case PDU_TRAN:
        if (_pasti(jbak) <= 200)
            break;

        if (get_gsm_resp(">") || _pasti(jbak) >= 3000) {
            len = sms_s.len - sms_s.len % 2;
            if (len == 0 || len > 140) {
                goto pout;
            }

            for (i = 0; i < len; i++) {
                _htoa(to_bcd, sms_s.msg[i]);
                write_gsm_hex(to_bcd[0]);
                write_gsm_hex(to_bcd[1]);
            }

            write_gsm_hex(0x00);
            write_gsm_hex(0x1A);

            clr_gsm_rev();
            jbak = jiffies;
            step = PDU_TACK;
            break;
        }
        break;
    case PDU_TACK:
        if (get_gsm_resp("OK")) {
            goto pout;
        } else if (get_gsm_resp("ERROR") || _pasti(jbak) >= 10 * 1000) {
        pout:
            _memset((u8 *)&sms_s, 0x00, sizeof(sms_s));
            gsm_step = GSM_CONNECT;
        } else
            reduce_gsm_rev();
        break;
    default:
        step = PDU_CSMP;
    }
}

#define connect() (get_gsm_resp("OK"))
#define active() (get_gsm_resp("OK"))

#if (P_SOUND == SOUND_BASE)
/****************************************************************************
* 名称:    gsm_tts_handle ()
* 功能：TTS 功能管理
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void gsm_tts_handle(void) {
    u8 tts_buf[500] = {0};
    static u16 jbak = 0;
    static u8 fail = 0;
    u16 crc;
    u16 i = 0;
    u16 tts_len = 0;

    static enum {
        TTS_CH,
        TTS_CACK,
        TTS_SEND,
        TTS_EXIT
    } step = TTS_CH;

    if (new_step) {
        fail = 0;
        jbak = jiffies;
        step = TTS_CH;
    }

    switch (step) {
    case TTS_CH:
        printf_gsm("AT+QAUDMOD=0\r\n"); //AT+DTAM=0   外音
        call_circuit_ctrl(CALL_IN);

        clr_gsm_rev();
        jbak = jiffies;

        step = TTS_CACK;
        break;
    case TTS_CACK:
        if (get_gsm_resp("OK")) {
            fail = 0;
            step = TTS_SEND;
            break;
        }
        if (get_gsm_resp("ERROR") || _pasti(jbak) >= 3 * 1000) {
            if (++fail < 3) {
                loge("TTS fail n");
                step = TTS_CH;
                break;
            } else
                step = TTS_EXIT;
        }
        break;

    case TTS_SEND:
        if (tts_m.total > 500)
            tts_m.total = 500;
        spi_flash_read(tts_buf, (u32)(_sect_addr(SECT_TTS_ADDR)), tts_m.total);
        logd("TTS=%s", tts_buf);
        crc = 0;
        for (i = 0; i < tts_m.total; i++) {
            crc ^= tts_buf[i];
        }

        tts_len = tts_m.total;
        printf_gsm("AT+QTTS=2, \"%s\"\r\n", tts_buf);
        for (i = 0; i < tts_len; i++) {
            if (over_gsm_send())
                goto tts_out;

            send_inc_gsm_data(tts_buf[i]);
        }

        if (over_gsm_send())
            goto tts_out;
        send_inc_gsm_data(tts_buf[i]);

        send_inc_gsm_data(',');

        send_inc_gsm_data(0x31);
        send_inc_gsm_data(0x0d);
        send_inc_gsm_data(0x0a);

        clr_gsm_rev();
        jbak = jiffies;

        tts_m.new_tts = false;
        tts_m.exit_time = tts_m.total / 2; //根据语速估算
        tts_m.playing = true;
        tts_m.p_tick = tick;
        tts_m.aud_end = false;
        tts_m.total = 0;
        gsm_step = GSM_CONNECT;
        break;
    case TTS_EXIT:
    tts_out:
        call_circuit_ctrl(CALL_IDLE);

        _memset((u8 *)&tts_m, 0x00, sizeof(tts_m));
        if (pub_io.b.dvr_open)
            gsm_step = GSM_CHECK;
        else
            gsm_step = GSM_CONNECT;
        break;
    default:
        step = TTS_CH;
    }
}
#endif

static void gsm_request_error(void) {
    static u8 step;
    static u16 jbak;

    if (new_step) {
        jbak = jiffies;
        step = 0;
    }

    switch (step) {
    case 0:
        printf_gsm("AT\r\n");
        clr_gsm_rev();
        jbak = jiffies;
        step = 1;
        break;
    case 1:
        if (get_gsm_resp("OK"))
            gsm_step = GSM_DISCONNECT;
        else if (_pasti(jbak) >= 1000) {
            gsm_step = GSM_OFF;
        }
        break;
    default:
        step = 0;
    }
}

#define CLOSE_CMD "AT+QIDEACT\r\n" //关闭TCP 连接
#define DEATV_CMD "AT+QISTAT=?\r\n"
#define DEATV_ACK "^IPINIT:0" //代表socket已经断开，等待再次连接

static void gsm_disconnect(void) {
    static enum {
        DC_CLOSE,
        DC_CACK,
        DC_DEACTV,
        DC_DACK
    } step;
    static u32 jbak;
    static u8 err_cnt = 0;

    if (new_step) {
        gc.gprs = NET_LOST;
        sync_net_offline();
        gs.gprs1.login = false;
        jbak = jiffies;
        step = DC_CLOSE;
    }

    switch (step) {
    case DC_CLOSE:
        if (_pastn(jbak) >= 2000) {
            printf_gsm(CLOSE_CMD); // Close IP connection
            clr_gsm_rev();
            jbak = jiffies;
            step = DC_CACK;
        }
        break;
    case DC_CACK:
        if (get_gsm_resp(CLOSE_ACK)) {
            step = DC_DEACTV;
            //gsm_step = GSM_CONNECT;
            break;
        } else if (get_gsm_resp("ERROR") || _pastn(jbak) >= 15000) {
            if (++err_cnt > 3) {
                gsm_step = GSM_OFF;
            } else {
                step = DC_CLOSE;
            }
            break;
        }
        break;
    case DC_DEACTV:
        printf_gsm(DEATV_CMD);
        clr_gsm_rev();
        jbak = jiffies;
        step = DC_DACK;
        break;
    case DC_DACK:
        if (get_gsm_resp("OK")) {
            err_cnt = 0;
            gsm_step = GSM_CONNECT;
            break;
        } else if (get_gsm_resp("ERROR") || _pastn(jbak) >= 2000) {
            if (++err_cnt > 3) {
                gsm_step = GSM_OFF;
                break;
            } else {
                step = DC_DEACTV;
                break;
            }
        }
        break;
    default:
        step = DC_CLOSE;
    }
}

#define NET_CLOSED1 "+QIURC: \"closed\",1" // GPRS 断开 提示
#define NET_CLOSED2 "2, CLOSED"            //当GPRS在线时，无本地IP

static void ring_detc(void) {
    u16 flag_offset;

    if (urc_tail == gsm_rev_head)
        return;

    if (_pastn(gsm_rev_jfs) > 30) {
        flag_offset = subp_gsm_rev(gsm_rev_head, urc_tail);
        if (flag_offset > 1000) {
            urc_tail = gsm_rev_head;
            return;
        }

        if (flag_offset < 1)
            return;

        if (search_gsm_recv(urc_tail, gsm_rev_head, "+CMT") != NULL)
            gs.sms.ring = true;
        if (search_gsm_recv(urc_tail, gsm_rev_head, "+CMTI:") != NULL)
            gs.sms.ring = true;
        // gs.sms.ring = false;

        if (search_gsm_recv(urc_tail, gsm_rev_head, NET_CLOSED1) && (gs.gprs1.stat >= NET_ONLINE))
            gs.gprs1.stat = NET_LOST;
        if (search_gsm_recv(urc_tail, gsm_rev_head, NET_CLOSED2) && (gs.gprs2.stat >= NET_ONLINE))
            gs.gprs2.stat = NET_LOST;

#if (P_SOUND == SOUND_BASE)
        if (search_gsm_recv(urc_tail, gsm_rev_head, "+CTTS:") != NULL) {
            if (tts_m.playing)
                tts_m.aud_end = true;
        }
#endif

        if (AtEngine1.exe_status == STATUS_RUNNING) {
            if (search_gsm_recv(urc_tail, gsm_rev_head, AtEngine1.catchstr1) != NULL) {
                AtEngine1.exe_status = AT_EVENT_CATCHED1;
                AtEngine1.pcatch = urc_tail;
            }
            if (search_gsm_recv(urc_tail, gsm_rev_head, AtEngine1.catchstr2) != NULL) {
                AtEngine1.exe_status = AT_EVENT_CATCHED2;
                AtEngine1.pcatch = urc_tail;
            }
            if (search_gsm_recv(urc_tail, gsm_rev_head, AtEngine1.catchstr3) != NULL) {
                AtEngine1.exe_status = AT_EVENT_CATCHED3;
                AtEngine1.pcatch = urc_tail;
            }
        }
#if 1
        if (AtEngine2.exe_status == STATUS_RUNNING) {
            if (search_gsm_recv(urc_tail, gsm_rev_head, AtEngine2.catchstr1) != NULL) {
                AtEngine2.exe_status = AT_EVENT_CATCHED1;
                AtEngine2.pcatch = urc_tail;
            }
            if (search_gsm_recv(urc_tail, gsm_rev_head, AtEngine2.catchstr2) != NULL) {
                AtEngine2.exe_status = AT_EVENT_CATCHED2;
                AtEngine2.pcatch = urc_tail;
            }
            if (search_gsm_recv(urc_tail, gsm_rev_head, AtEngine2.catchstr3) != NULL) {
                AtEngine2.exe_status = AT_EVENT_CATCHED3;
                AtEngine2.pcatch = urc_tail;
            }
        }
#endif

        urc_tail = gsm_rev_head;
    }
}

//休眠位置上报:
static funcp const ModemTaskTab[] = {
    gsm_power_up,     //step 0 打开模块电源
    gsm_off,          //step 1 power_key关机
    gsm_enter_pin,    //step 2 检测SIM 卡是否准备就绪
    gsm_init,         //step 3 模块参数初始化
    gsm_check,        //step 4
    gsm_setup,        //step 5
    gsm_run_task,     //step 6  建立TCP/IP连接
    gsm_recv_sms,     //step 7
    gsm_send_sms_txt, //step 8	发送文本短信
    gsm_send_sms_pdu, //step 9	发送PDU 短信
#if (P_SOUND == SOUND_BASE)
    gsm_tts_handle, //step 10	TTS 功能管理
#endif
    gsm_request_error, //step 11
    gsm_disconnect     //step 12
};

void gsm_handle_task(void) {
    //    u8 tmp[40] = {0};
    enum E_GSM_STEP step;
    static u32 tmr = 0;
    step = gsm_step;

#if (1)
    (*ModemTaskTab[gsm_step % GSM_END])();
    new_step = (gsm_step == step) ? false : true;
    if (new_step) {
        gsm_see_state();
    }

    if (gsm_step >= GSM_CONNECT) {
        ring_detc();
    }
#endif

    if ((_pastn(tmr) > 30000) || (tmr == 0)) {
        //	tmr = jiffies;
        //	len = _strcpy_len(tmp, (u8 *)FW_VERSION) - 1;
        //	len = _sprintf_len((char *)&buff, "time = %02d,VER:%s\r\n", tick, tmp);
        //	write_uart1( buff, len);
    }
}

void gsm_see_state(void) {
    const char *str_sta[] = {
        "p.up",
        "off",
        "pin",
        "init",
        "chk",
        "setup",
        "con",
        "r.sms",
        "s.sms.t",
        "s.sms.pdu",
        "tts",
        "rq.err",
        "d.con",
        "???",
    };

    logf("%d.gsmStep%d[%s], acc%d, cfg.sleep.rpt%d", new_step, gsm_step, str_sta[(gsm_step < GSM_END) ? gsm_step : GSM_END], car_state.bit.acc, report_cfg.sleep_time);
    logf("gprs123.stat,%d,%d,%d;gprs123.login,%d,%d,%d;", gs.gprs1.stat, gs.gprs2.stat, gs.gprs3.stat, gs.gprs1.login, gs.gprs2.login, gs.gprs3.login);
}

#endif
