/**
  ******************************************************************************
  * 娉㈢壒鐜? 9600 8 1
  * 	
  * USART1  鎺ヨ浇閲嶄紶鎰熷櫒 
  ******************************************************************************
**/
#include "include_all.h"

/*
uc32 uart1_baud_rate[32]={	9600, 	115200, 	115200, 	9600, 
							115200, 	19200, 	19200, 	9600, 			
							19200, 	19200, 	19200, 	19200, 						
							19200, 	9600, 	9600, 	9600, 
							9600, 	19200, 	19200, 	19200, 
							19200, 	19200, 	19200, 	19200, 			
							19200, 	19200, 	19200, 	19200, 						
							19200, 	19200, 	19200, 	19200
};
*/

uc32 uart1_baud_rate[32] = {115200, 115200, 115200, 115200,
                            115200, 115200, 115200, 115200,
                            19200, 19200, 19200, 19200,
                            19200, 9600, 9600, 9600,
                            9600, 19200, 19200, 19200,
                            19200, 19200, 19200, 19200,
                            19200, 19200, 19200, 19200,
                            19200, 19200, 19200, 19200};

u8 r1_buf[R1_BUF_SIZE]; //接收
u8 *r1_head = r1_buf;
u8 *r1_tail = r1_buf;

u8 t1_buf[T1_BUF_SIZE]; //发送
u8 *t1_head = t1_buf;
u8 *t1_tail = t1_buf;

bool uart1_busy = false; //串口数据发送忙
u32 jif_r1 = 0;          //数据接收的时间
u32 jif_t1 = 0;          //数据发送的时间

uart_manage u1_m; //串口管理

/*********************************************************
名    称：write_uart1_txt
功    能：发送任意长度数据
输入参数：无
输    出：无
**********************************************************/
void write_uart1(u8 *buf, u16 len) {
    u16 i;

    for (i = 0; i < len; i++) {
        if (over_t1())
            break;

        *t1_head = *buf++;
        incp_t1(t1_head, 1);
    }
}

/*********************************************************
名    称：write_uart1_hex
功    能：发送任意单字节字符
输入参数：无
输    出：无
**********************************************************/
void write_uart1_hex(u8 hex) {
    if (over_t1())
        return;
    *t1_head = hex;
    incp_t1(t1_head, 1);
}

/*********************************************************
名    称：read_uart1
功    能：串口信息读
输入参数：无
输    出：无
**********************************************************/
u16 read_uart1(u8 *buf, u8 *rbuf_ptr, u16 n) {
    u16 ret_len = 0;

    ret_len = n;
    while (n--) {
        *buf++ = *rbuf_ptr;
        incp_r1(rbuf_ptr, 1);
    }

    return ret_len;
}

/*********************************************************
名    称：get_uart1_func
功    能：UART1 支持的功能
输入参数：无
输    出：无
**********************************************************/
u32 get_uart1_func(void) {
    uart_func_list t;

    t.reg = 0;

    t.b.tt = true;
    t.b.debug = true;
    t.b.rcd_3c = true;
    t.b.tr9 = true;
    return t.reg;
}

/*********************************************************
名    称：load_uart1_remap
功    能：重新配置UART信息
输入参数：无
输    出：无
**********************************************************/
void load_uart1_remap(void) {
    us.u1.reg = 0;

#pragma region 串口0(PA9 + PA10) 功能开关
    us.u1.b.tt = 0;
    us.u1.b.debug = 1;  //调试模块:debug of usart0
    us.u1.b.rcd_3c = 0; // 默认
    us.u1.b.tr9 = 0;
    us.u1.b.handset = 0;
#pragma endregion 串口0(PA9 + PA10) 功能开关

    u1_m.cur.reg = us.u1.reg;
    u1_m.bak.reg = us.u1.reg;

    u1_m.update_baud = false;
    u1_m.bypass_sw = false;
    u1_m.bypass_tick = tick;

    uart1_update_func(u1_m.cur.reg, BY_SETUP);
    logf("tt %d, hset %d, 3c %d,tr9 %d, deb %d", us.u1.b.tt, us.u1.b.handset, us.u1.b.rcd_3c, us.u1.b.tr9, us.u1.b.debug);
}

/*********************************************************
名    称：uart1_update_func
功    能：重新配置UART信息
输入参数：无
输    出：无
**********************************************************/
bool uart1_update_func(u16 fun, u16 set) {
    USART_InitTypeDef USART_InitStructure;
    static bool switch_flag = false;
    u8 index = 0;

    index = verify_any_uart_func(FROM_U1, fun);
    if (index == 0xFF) {
        loge("uart1_update_err");
        return false;
    }

    if (set == BY_SETUP) {
        u1_m.bak.reg = fun;
        u1_m.cur.reg = fun;
        u1_m.set = BY_SETUP;
    } else if (set == BY_TEMP) {
        u1_m.bak.reg = u1_m.cur.reg;
        u1_m.cur.reg = fun;
        u1_m.set = BY_TEMP;
    } else {
        return false;
    }
    if (u1_m.cur.b.tt) {
        tr9_show = false; //关闭TR9发送打印
        switch_flag = true;
    }
    if (switch_flag) {
        tr9_show = true; //关闭TR9发送打印
        switch_flag = false;
    }

    u1_m.cur_baud = uart1_baud_rate[index];

    //添加波特率和功能管脚的判断
    USART_DeInit(USART1);
    USART_InitStructure.USART_BaudRate = u1_m.cur_baud;

    USART_InitStructure.USART_StopBits = USART_StopBits_1;

    //22-06-06号添加判断,如果是3C功能就使用奇校验，平时都是无检验
    if (u1_m.cur.b.rcd_3c && DEBUG_EN_RCD_3C) {
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
        USART_InitStructure.USART_Parity = USART_Parity_Odd; //奇校验
    } else {
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    }

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);

    clr_buf_r1();
    clr_buf_t1();
    return true;
}

/****************************************************************************
* 名称:    Uart_tran_proc ()
* 功能：发送相应数据到终端
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void uart1_tran_task(void) {
    //防错
    if (subp_t1(t1_head, t1_tail) > T1_BUF_SIZE) {
        t1_head = t1_buf;
        t1_tail = t1_buf;
        loge("uart1 tran err: over");
    }

    if (t1_tail != t1_head && uart1_busy == false) {
        jif_t1 = jiffies;
        uart1_busy = true;
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
    }

    if (t1_tail != t1_head && uart1_busy == true && _pastn(jif_t1) > 3000) {
        uart1_busy = false; //超时，清除标志
    }
}

void uart1_tran_task_nms(int nms) {
    unsigned int tic = jiffies;

    while (_pastn(tic) < nms + 5) {
        uart1_tran_task();
    }
}

UART_MANAGE_STRUCT read_uar1;
/****************************************************************************
* 名称:    uart1_recive_task ()
* 功能：UART1 接收任务
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void uart1_recive_task(void) {
    typedef enum {
        E_UART_IDLE,   //等待有新的串口数据
        E_UART_WITING, //等待外部程序解析
        E_UART_DEL,    //删除已经处理的数据
        E_UART_ERR     //出现错误
    } E_UART_RECIVE_STEP;

    E_UART_RECIVE_STEP old_step = E_UART_ERR;
    static E_UART_RECIVE_STEP step = E_UART_IDLE;

    static bool new_step = false;
    static u32 ot_jfs = 0; //overtime_jiffies

    u16 len_t;
    u16 i;
    old_step = step;

    switch (step) {
    case E_UART_IDLE: {
        if (!read_uar1.news) {
            if (new_r1() && _pastn(jif_r1) >= 30) {
                len_t = new_r1();
                if (len_t >= (R1_BUF_SIZE - 30)) { //如果数据长度超过规定长度
                    step = E_UART_ERR;
                    break;
                }

                read_uar1.cur_p = r1_tail;
                read_uar1.c_len = len_t;
                read_uar1.n_jfs = jiffies;
                read_uar1.ack = false;
                read_uar1.news = true;

                if (true == u1_m.cur.b.rcd_3c) {
                    c3_data.c3_len = read_uart1(c3_data.c3_buf, read_uar1.cur_p, len_t);
                }
                step = E_UART_WITING;
                break;
            }
        } else {
            step = E_UART_DEL;
            break;
        }
        break;
    }
    case E_UART_WITING: {
        if (new_step) {
            ot_jfs = jiffies;
        }

        if (over_r1() || _pastn(ot_jfs) >= 1000) {
            loge("uart1_recive_task err: 1");
            step = E_UART_ERR;
            break;
        }

        if (read_uar1.ack) {
            step = E_UART_DEL;
            break;
        }
        break;
    }
    case E_UART_DEL: {
        len_t = new_r1();
        if (len_t < read_uar1.c_len) { //错误，要删除的长度大于BUF中的实际长度
            step = E_UART_ERR;
            break;
        }

        for (i = 0; i < read_uar1.c_len; i++) {
            incp_r1(r1_tail, 1);
        }

        _memset((u8 *)&read_uar1, 0x00, sizeof(read_uar1));
        step = E_UART_IDLE;
        break;
    }
    case E_UART_ERR: {
        clr_buf_r1();
        _memset((u8 *)&read_uar1, 0x00, sizeof(read_uar1));
        step = E_UART_IDLE;
        break;
    }
    default: {
        step = E_UART_IDLE;
        break;
    }
    }

    new_step = (old_step != step) ? true : false;
    //if(r1_buf!=NULL)
    //{

    //}
}

// TR9璋冭瘯鍙ｅ姛鑳?
static bool get_uart1_is_debug(u8 from_t) {
    if (u1_m.cur.b.debug == true && from_t == FROM_U1) {
        return true;
    } else {
        return false;
    }
}

/****************************************************************************
* 名称:    uart1_debug_proc ()
* 功能：接收摄像头数据
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void uart1_debug_proc(void) {
    typedef enum {
        E_UART1_IDLE,
        E_UART1_RECV,
        E_UART1_EXIT
    } E_UART1_STEP;

    static E_UART1_STEP step = E_UART1_IDLE;
    bool ret;
    u8 r_buf[R1_BUF_DEBUG_SIZE];
    u16 r_len = 0;
    // u8 xvalue = 0;

    static u8 from_id = 0;
    static UART_MANAGE_STRUCT present;

    switch (step) {
    case E_UART1_IDLE: {
        from_id = get_uart_news(&present);
        ret = get_uart1_is_debug(from_id);
        if (ret) {
            step = E_UART1_RECV;
            //logd("deb recv");
        }
        break;
    }
    case E_UART1_RECV: {
        if (present.c_len < 1 || present.c_len > (R1_BUF_DEBUG_SIZE - 3)) {
            loge("err, %d", present.c_len);
            step = E_UART1_EXIT;
            break;
        }

        //_memset(r_buf, 0x00, R1_BUF_DEBUG_SIZE);
        //调试模块:debug of usart0//有溢出风险//已解决
        r_len = copy_uart_news(from_id, r_buf, present.cur_p, present.c_len);
        r_buf[r_len] = '\0';

#if (0) //TEST
        if (r_len < 3) {
            loge("err, %d", r_len);
            step = E_UART1_EXIT;
            break;
        }

        putHexs_hex(r_buf, r_len);

        unsigned char arr_test[] = {0xF0, 0x17, 0x17, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x08, 0x4C, 0x00, 0x01, 0x01, 0x61, 0x75, 0x0E, 0x06, 0xC3, 0x0C, 0x25, 0x00, 0x00, 0x00, 0x00, 0x01, 0x67, 0x22, 0x09, 0x14, 0x22, 0x12, 0x31, 0x00, 0x00, 0x02, 0x36, 0x4A, 0x00, 0x00, 0x09, 0x4B, 0x00, 0x00, 0xF1, 0xF0};
        tr9_net_parse(arr_test, sizeof(arr_test), 0);
#endif

        //处理方式一:
        tr9_frame_st *frame = tr9_frame_get_and_need_free(__func__, r_buf, r_len,0);
        if (frame != NULL) {
            free(frame);
            //调试模块:debug of usart0//调试，模拟，RK的数据帧，执行相关动作。
            tr9_net_parse(r_buf, r_len, from_id);
            step = E_UART1_EXIT;
            break;
        }

        //处理方式二:
        /*2022-03-25 add by hj*/
        //*XCL
        //行车记录仪相关的功能
        //GB19056
        if ((r_len >= 4) && (strncmp((const char *)r_buf, "*XCL", 4) == 0)) {
            tr9_Auto_start(r_buf, r_len);
            step = E_UART1_EXIT;
            break;
        }

        //处理方式三:串口调试功能
        if (-1 != cmd_handle(r_buf, r_len)) {
            step = E_UART1_EXIT;
            break;
        }

        step = E_UART1_EXIT;
        break;
    }
    case E_UART1_EXIT: {
        set_uart_ack(from_id);
        step = E_UART1_IDLE;
        break;
    }
    default: {
        step = E_UART1_IDLE;
        break;
    }
    }
}
