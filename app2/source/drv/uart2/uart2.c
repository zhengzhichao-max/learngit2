/**
  ******************************************************************************
  * 褰㈠紡璁板綍浠?帴鍙? 
  ******************************************************************************
**/
#include "include_all.h"

u8 r4_buf[R4_BUF_SIZE]; //接收缓存
u8 *r4_head = r4_buf;
u8 *r4_tail = r4_buf;

u8 t4_buf[T4_BUF_SIZE]; //发送缓存
u8 *t4_head = t4_buf;
u8 *t4_tail = t4_buf;

bool uart4_busy = false; //串口发送忙
u32 jif_r4 = 0;          //串口接收时间
u32 jif_t4 = 0;          //串口发送时间

uart_manage u4_m;             //串口管理
UART_MANAGE_STRUCT read_uar4; //串口读
u8 comm_func = 0;
/****************************************************************************
* 名称:    write_uart4 ()
* 功能：带长度的任意数据发送
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void write_uart4(u8 *buf, u16 len) {
    u16 i;
    for (i = 0; i < len; i++) {
        if (over_t4())
            break;

        *t4_head = *buf++;
        incp_t4(t4_head, 1);
    }
}

/****************************************************************************
* 名称:    write_uart2_hex ()
* 功能：单个字符数据发送
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void write_uart4_hex(u8 hex) {
    if (over_t4())
        return;

    *t4_head = hex;
    incp_t4(t4_head, 1);
}

/****************************************************************************
* 名称:    read_uart2 ()
* 功能：数据读
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
u16 read_uart4(u8 *buf, u8 *rbuf_ptr, u16 n) {
    u16 ret_len = 0;

    ret_len = n;
    while (n--) {
        *buf++ = *rbuf_ptr;
        incp_r4(rbuf_ptr, 1);
    }

    return ret_len;
}

/****************************************************************************
* 名称:    get_uart2_func ()
* 功能：获取串口所支持的功能
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
u32 get_uart4_func(void) {
    uart_func_list t4;

    t4.reg = 0;
    t4.b.speedlimit = true;
    t4.b.yd_oil = true;
    t4.b.tt = true;
    t4.b.weight = true;
    t4.b.read_card = true;
    //	t4.b.led		= true ; 	//显示屏
    t4.b.navigation = true; //导航
                            //	t4.b.oil_mass	= true ;	//油量
                            //	t4.b.ethyl_alcohol= true ;	//酒精

    return t4.reg;
}

/****************************************************************************
* 名称:    load_uart2_remap ()
* 功能：加载串口映射的功能
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
void load_uart2_remap(void) {
#pragma region 串口1(PA2 + PA3) 功能开关       //定位模块
//4G
#pragma endregion 串口1(PA2 + PA3) 功能开关
}

/****************************************************************************
* 名称:    load_uart4_remap ()
* 功能：加载串口映射的功能
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
void load_uart4_remap(void) {
#pragma region 串口3(PC10 + PC11) 功能开关
    us.u4.reg = 0;
    us.u4.b.navigation = 1; //扩展多路串口uart3:默认为：6.车载导航屏
#pragma endregion 串口3(PC10 + PC11) 功能开关

    u4_m.cur.reg = us.u4.reg;
    u4_m.bak.reg = us.u4.reg;

    u4_m.update_baud = false;
    u4_m.bypass_sw = false;
    u4_m.bypass_tick = tick;

    uart4_update_func(u4_m.cur.reg, BY_SETUP);
}

/****************************************************************************
* 名称:    uart2_update_func ()
* 功能：刷新串口映射的功能
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
bool uart4_update_func(u16 fun, u16 set) {
    USART_InitTypeDef USART_InitStructure;

    u8 index = 0;

    index = verify_any_uart_func(FROM_U4, fun);
    comm_func = index;
    if (index == 0xFF) {
        loge("uart4_update_err");
        return false;
    }

    if (set == BY_SETUP) {
        u4_m.bak.reg = fun;
        u4_m.cur.reg = fun;
        u4_m.set = BY_SETUP;
    } else if (set == BY_TEMP) {
        u4_m.bak.reg = u4_m.cur.reg;
        u4_m.cur.reg = fun;
        u4_m.set = BY_TEMP;
    } else {
        return false;
    }

    //u4_m.cur_baud = uart1_baud_rate[index];
    u4_m.cur_baud = 115200;

#pragma endregion //UART3: PC10 & PC11
    //添加波特率和功能管脚的判断
    USART_DeInit(UART4);
    USART_InitStructure.USART_BaudRate = u4_m.cur_baud;

    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(UART4, &USART_InitStructure);
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_Cmd(UART4, ENABLE);
#pragma endregion //UART3: PC10 & PC11

    clr_buf_r4();
    clr_buf_t4();
    return true;
}

/****************************************************************************
* 名称:    Uart_tran_proc ()
* 功能：发送相应数据到终端
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void uart4_tran_task(void) {
    //防错
    if (subp_t4(t4_head, t4_tail) > T4_BUF_SIZE) {
        t4_head = t4_buf;
        t4_tail = t4_buf;
        loge("uart4 tran err: over");
    }

    if (t4_tail != t4_head && uart4_busy == false) //
    {
        jif_t4 = jiffies;
        uart4_busy = true;
        USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
    }

    if (t4_tail != t4_head && uart4_busy == true && _pastn(jif_t4) > 3000) {
        uart4_busy = false; //超时，清除标志
    }
}

/****************************************************************************
* 名称:    uart2_recive_task()
* 功能：串口数据发送任务
* 入口参数：无                         
* 出口参数：无
***************************************************************************/
void uart4_recive_task(void) {
    typedef enum {
        E_UART_IDLE,   //等待有新的串口数据
        E_UART_WITING, //等待外部程序解析
        E_UART_DEL,    //删除已经处理的数据
        E_UART_ERR     //出现错误
    } E_UART_RECIVE_STEP;

    E_UART_RECIVE_STEP old_step = E_UART_ERR;
    static E_UART_RECIVE_STEP step = E_UART_IDLE;
    //    u16 r_len;
    //    static u16 svn = 1;
    static bool new_step = false;
    static u32 ot_jfs = 0; //overtime_jiffies
    u8 r_buf[200] = {0};
    u16 len_t;
    old_step = step; ////
    switch (step) {
    case E_UART_IDLE:
        if (!read_uar4.news) {
            if (new_r4() && _pastn(jif_r4) >= 100) {
                len_t = new_r4();
                if (len_t >= (R4_BUF_SIZE - 30)) {
                    step = E_UART_ERR;
                    break;
                }

                read_uar4.cur_p = r4_tail;
                read_uar4.c_len = len_t;
                read_uar4.n_jfs = jiffies;
                read_uar4.ack = false;
                read_uar4.news = true;

                if (u4_m.cur.b.yd_oil == true) {
                    _memset(r_buf, 0x00, 200);
                    /*r_len = */ read_uart4(r_buf, read_uar4.cur_p, len_t);
                    tr9_comm_pass(2, 4, r_buf, len_t);
                }

                if (u4_m.cur.b.read_card == true) //读卡器
                {
                    _memset(r_buf, 0x00, 200);
                    /*r_len = */ read_uart4(r_buf, read_uar4.cur_p, len_t);
                    tr9_comm_pass(2, 1, r_buf, len_t);
                }
#if 1
                if (1 == u4_m.cur.b.navigation) { //显示屏
                    _memset(r_buf, 0x00, 200);

                    /*r_len = */ read_uart4(r_buf, read_uar4.cur_p, len_t);
                    tr9_comm_pass(2, 3, r_buf, len_t);
                }
#endif
                step = E_UART_WITING;
                break;
            }
        } else {
            step = E_UART_DEL;
            break;
        }
        break;
    case E_UART_WITING:
        if (new_step) {
            ot_jfs = jiffies;
        }
        if (over_r4() || _pastn(ot_jfs) >= 2000) {
            loge("uart2_recive_task err: 1");
            step = E_UART_ERR;
            break;
        }

        if (read_uar4.ack) {
            step = E_UART_DEL;
            break;
        }
        break;
    case E_UART_DEL:
        len_t = new_r4();
        if (len_t < read_uar4.c_len) //错误，要删除的长度大于BUF中的实际长度
        {
            step = E_UART_ERR;
            break;
        }

        incp_r4(r4_tail, len_t);
        _memset((u8 *)&read_uar4, 0x00, sizeof(read_uar4));
        step = E_UART_IDLE;
        break;
    case E_UART_ERR:
        clr_buf_r4();
        _memset((u8 *)&read_uar4, 0x00, sizeof(read_uar4));
        step = E_UART_IDLE;
        break;
    default:
        step = E_UART_IDLE;
        break;
    }

    new_step = (old_step != step) ? true : false;
}

u8 get_uart4_news(UART_MANAGE_STRUCT *c) {
    if (read_uar4.news) {
        _memcpy((u8 *)c, (u8 *)&read_uar4, sizeof(read_uar4));
        return FROM_U4;
    } else {
        return FROM_NULL;
    }
}
