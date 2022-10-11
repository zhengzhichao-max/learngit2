/**
  ******************************************************************************
  * @file    uart_pub.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ���ڹ���
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

s_any_uart_set us;                //��������
S_BYPASS_DEBUG bypass_debug;      //����
any_uart_remap_struct uart_remap; //������ӳ��

/****************************************************************************
* ����:    get_flag_7e_cnt ()
* ���ܣ���ȡ��־λ����
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
u8 get_flag_7e_cnt(u8 *buf_t, u16 b_len_t, u8 flag) {
    u8 n;
    u16 len_t;

    n = 0;
    if (buf_t == NULL || b_len_t == 0 || b_len_t > 200) {
        return 0;
    }

    len_t = 0;
    while (len_t < b_len_t) {
        if (*buf_t == flag) {
            n++;
        }

        buf_t++;
        len_t++;
    }

    return n;
}

/****************************************************************************
* ����:    get_flag_7e_pointer ()
* ���ܣ���ȡ��Ӧ��־λ��ָ��
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
s16 get_flag_7e_pointer(u8 *buf_t, u16 b_len_t, u8 flag) {
    s16 len_t;

    if (buf_t == NULL || b_len_t == 0) {
        return -1;
    }

    len_t = 0;
    while (len_t < b_len_t) {
        if (*buf_t == flag) {
            return len_t;
        }

        buf_t++;
        len_t++;
    }

    return -1;
}

/****************************************************************************
* ����:    get_uart_news ()
* ���ܣ���ȡ�����µ���Ϣ
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
u8 get_uart_tr9(UART_MANAGE_STRUCT *c) {
    {
        return FROM_NULL;
    }
}

u8 get_uart_3c(UART_MANAGE_STRUCT *c) {
    if (read_uar1.news) {
        _memcpy((u8 *)c, (u8 *)&read_uar1, sizeof(read_uar1));
        return FROM_U1;
    } else {
        return FROM_NULL;
    }
}

u8 get_uart_news(UART_MANAGE_STRUCT *c) {
    if (read_uar1.news) {
        _memcpy((u8 *)c, (u8 *)&read_uar1, sizeof(read_uar1));
        return FROM_U1;
    } else {
        return FROM_NULL;
    }
}

/****************************************************************************
* ����:    copy_uart_news ()
* ���ܣ�����������Ϣ
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
u16 copy_uart_news(u8 uart_id_t, u8 *p, u8 *s, u16 s_len) {
    u16 p_len;

    p_len = 0;
    if (uart_id_t == FROM_U1) {
        if (p == NULL || s_len >= R1_BUF_SIZE)
            return 0;

        if (s == NULL || s < r1_buf || s > (r1_buf + R1_BUF_SIZE))
            return 0;

        p_len = read_uart1(p, s, s_len);
    } else if (uart_id_t == FROM_U4) {
        if (p == NULL || s_len >= R4_BUF_SIZE)
            return 0;

        if (s == NULL || s < r4_buf || s > (r4_buf + R4_BUF_SIZE))
            return 0;

        p_len = read_uart4(p, s, s_len);
    } else {
        return 0;
    }

    return p_len;
}

/**************************************************************************
* ����:    set_uart_ack ()
* ���ܣ���ϢӦ��
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
void set_uart_ack(u8 from_t) {
    if (from_t == FROM_U1) {
        read_uar1.news = false;
        read_uar1.ack = true;
    }
#if (MCU == STM32F103VCT6)
    else if (from_t == FROM_U4) {
        read_uar4.news = false;
        read_uar4.ack = true;
    }
#endif
}

/****************************************************************************
* ����:    verify_any_uart_func ()
* ���ܣ�����У��
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
u8 verify_any_uart_func(u8 source, u16 fun1) {
    u16 v;
    u16 t;

    u8 cnt;
    u8 cnt1;
    u16 i = 0;
    u8 index = 0xff;

    cnt = 0;
    cnt1 = 0;
    if (source == FROM_U1) {
        t = get_uart1_func();
        v = fun1 & t;
    }
#if (MCU == STM32F103VCT6)
    else if (source == FROM_U4) {
        t = get_uart4_func();
        v = fun1 & t;
    }
#endif
    else {
        return 0xff;
    }

    for (i = 0; i < 16; i++) {
        if (fun1 & (1 << i)) {
            if (++cnt1 > 1)
                return 0xff;
        }

        if (v & (1 << i)) {
            cnt++;
            index = i;
        }
    }

    if (cnt == 1) {
        return index;
    } else {
        return 0xff;
    }
}

/****************************************************************************
* ����:    send_any_uart ()
* ���ܣ������������ݵ����⴮��
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
void send_any_uart(u8 source, u8 *data1, u16 len1) {
    if (source == FROM_U1) {
        write_uart1(data1, len1);
    }
#if (MCU == STM32F103VCT6)
    else if (source == FROM_U4) {
        write_uart4(data1, len1);
    }
#endif
    else {
        loge("send any uart err");
    }
}

/****************************************************************************
* ����:    load_any_uart_remap ()
* ���ܣ����ش�����ӳ��
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
void load_any_uart_remap(void) {
    //USART0: PA9 & PA10
    //USART1: PA2 & PA3
    //USART2: PB10 & PB11
    //UART3: PC10 & PC11
    //UART4: PC12 & PD2

    load_uart1_remap();
    load_uart2_remap();
    load_uart4_remap();
}

/****************************************************************************
* ����:    uart_manage_task ()
* ���ܣ����ڹ��ܹ�������
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
void uart_manage_task(void) {
    uart1_recive_task(); //USART0: PA9 & PA10 ����
    uart1_tran_task();   //USART0: PA9 & PA10 ����

    //4G
    gsm_drv_tran_task(); //USART1: PA2 & PA3 ����

    //RK:
    tr9_parse_task(); //USART2: PB10 & PB11

#ifdef USING_HANDSET
    handset_parse_proc(); //USART0: PA9 & PA10 ����ҵ��//u1_m.cur.b.handset
#endif

#if (P_RCD == RCD_BASE)
        //���ڶ�ȡ��¼������
    uart_rcd_parse_proc(); //USART0: PA9 & PA10 ����ҵ��//u1_m.cur.b.rcd_3c
#endif

    com1_down_font_lib(); //USART0: PA9 & PA10 ����ҵ��//u1_m.cur.b.tr9

    //RK:
    uart3_tran_task(); //USART2: PB10 & PB11 ����

    //�ն˺��ӿڣ����贮��
    uart4_recive_task(); //UART3: PC10 & PC11 ����
    uart4_tran_task();   //UART3: PC10 & PC11 ����

    //��λģ��
    gps_drv_tran_task(); //UART4: PC12 & PD2 ����
    uart1_debug_proc();  //USART0: PA9 & PA10 ����ҵ��//u1_m.cur.b.debug
}
