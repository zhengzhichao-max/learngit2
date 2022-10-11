/**
  ******************************************************************************
  * 形式记录�?���? 
  ******************************************************************************
**/
#include "include_all.h"

u8 r4_buf[R4_BUF_SIZE]; //���ջ���
u8 *r4_head = r4_buf;
u8 *r4_tail = r4_buf;

u8 t4_buf[T4_BUF_SIZE]; //���ͻ���
u8 *t4_head = t4_buf;
u8 *t4_tail = t4_buf;

bool uart4_busy = false; //���ڷ���æ
u32 jif_r4 = 0;          //���ڽ���ʱ��
u32 jif_t4 = 0;          //���ڷ���ʱ��

uart_manage u4_m;             //���ڹ���
UART_MANAGE_STRUCT read_uar4; //���ڶ�
u8 comm_func = 0;
/****************************************************************************
* ����:    write_uart4 ()
* ���ܣ������ȵ��������ݷ���
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    write_uart2_hex ()
* ���ܣ������ַ����ݷ���
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
void write_uart4_hex(u8 hex) {
    if (over_t4())
        return;

    *t4_head = hex;
    incp_t4(t4_head, 1);
}

/****************************************************************************
* ����:    read_uart2 ()
* ���ܣ����ݶ�
* ��ڲ�������                         
* ���ڲ�������
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
* ����:    get_uart2_func ()
* ���ܣ���ȡ������֧�ֵĹ���
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
u32 get_uart4_func(void) {
    uart_func_list t4;

    t4.reg = 0;
    t4.b.speedlimit = true;
    t4.b.yd_oil = true;
    t4.b.tt = true;
    t4.b.weight = true;
    t4.b.read_card = true;
    //	t4.b.led		= true ; 	//��ʾ��
    t4.b.navigation = true; //����
                            //	t4.b.oil_mass	= true ;	//����
                            //	t4.b.ethyl_alcohol= true ;	//�ƾ�

    return t4.reg;
}

/****************************************************************************
* ����:    load_uart2_remap ()
* ���ܣ����ش���ӳ��Ĺ���
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
void load_uart2_remap(void) {
#pragma region ����1(PA2 + PA3) ���ܿ���       //��λģ��
//4G
#pragma endregion ����1(PA2 + PA3) ���ܿ���
}

/****************************************************************************
* ����:    load_uart4_remap ()
* ���ܣ����ش���ӳ��Ĺ���
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
void load_uart4_remap(void) {
#pragma region ����3(PC10 + PC11) ���ܿ���
    us.u4.reg = 0;
    us.u4.b.navigation = 1; //��չ��·����uart3:Ĭ��Ϊ��6.���ص�����
#pragma endregion ����3(PC10 + PC11) ���ܿ���

    u4_m.cur.reg = us.u4.reg;
    u4_m.bak.reg = us.u4.reg;

    u4_m.update_baud = false;
    u4_m.bypass_sw = false;
    u4_m.bypass_tick = tick;

    uart4_update_func(u4_m.cur.reg, BY_SETUP);
}

/****************************************************************************
* ����:    uart2_update_func ()
* ���ܣ�ˢ�´���ӳ��Ĺ���
* ��ڲ�������                         
* ���ڲ�������
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
    //��Ӳ����ʺ͹��ܹܽŵ��ж�
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
* ����:    Uart_tran_proc ()
* ���ܣ�������Ӧ���ݵ��ն�
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
void uart4_tran_task(void) {
    //����
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
        uart4_busy = false; //��ʱ�������־
    }
}

/****************************************************************************
* ����:    uart2_recive_task()
* ���ܣ��������ݷ�������
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
void uart4_recive_task(void) {
    typedef enum {
        E_UART_IDLE,   //�ȴ����µĴ�������
        E_UART_WITING, //�ȴ��ⲿ�������
        E_UART_DEL,    //ɾ���Ѿ����������
        E_UART_ERR     //���ִ���
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

                if (u4_m.cur.b.read_card == true) //������
                {
                    _memset(r_buf, 0x00, 200);
                    /*r_len = */ read_uart4(r_buf, read_uar4.cur_p, len_t);
                    tr9_comm_pass(2, 1, r_buf, len_t);
                }
#if 1
                if (1 == u4_m.cur.b.navigation) { //��ʾ��
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
        if (len_t < read_uar4.c_len) //����Ҫɾ���ĳ��ȴ���BUF�е�ʵ�ʳ���
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
