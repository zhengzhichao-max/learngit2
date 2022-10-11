/**
  ******************************************************************************
  * @file    uart1.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ����1����
  ******************************************************************************
  * @attention
	1.����1����֧�ֶ�������
	2.��������ƽ̨���ԶԴ��ڽ�������
	3.���ڱ�����Ϊ�ǵ������ӿڣ���λ���ǰ30���ӣ�Ĭ������������ӣ����յ�����������Ϣ
	����ʱ30���˳�������ģʽ���������õĴ���ģʽ��
  ******************************************************************************
*/
#ifndef __UART1_H
#define __UART1_H

#define R1_BUF_DEBUG_SIZE 512
#define R1_BUF_SIZE 2400
#define T1_BUF_SIZE 1600 //2018-3-5 ,�¹�����660���ֽڻ����������,��˸���Ϊ1024���ֽ�

#define JT808_MIM_SIZE 7
#define JT808_MAM_SIZE 128

extern uc32 uart1_baud_rate[];

typedef enum {
    BY_TEMP = 0,
    BY_SETUP
} uart_set_list;

typedef struct {
    bool update_baud; //ˢ�²�����
    bool bypass_sw;   //�ô�����ʱ����ǰ���õĹ���
    bool err;         //����
    bool res;
    u32 bypass_tick;    //�����Ե�ʱ��
    u32 cur_baud;       //��ǰ������
    uart_func_list cur; //��ǰ�Ĵ��ڹ���
    uart_set_list set;  //���õĴ��ڹ���
    uart_func_list bak; //���ݵĴ��ڹ���
} uart_manage;

extern uart_manage u1_m;

extern u8 r1_buf[];
extern u8 *r1_head;
extern u8 *r1_tail;

extern u8 t1_buf[];
extern u8 *t1_head;
extern u8 *t1_tail;

extern bool uart1_busy;
extern u32 jif_r1;
extern u32 jif_t1;

#define incp_r1(ptr, n)                         \
    do {                                        \
        if ((ptr += n) >= r1_buf + R1_BUF_SIZE) \
            ptr -= R1_BUF_SIZE;                 \
    } while (0)
#define decp_r1(ptr, n)          \
    do {                         \
        if ((ptr -= n) < r1_buf) \
            ptr += R1_BUF_SIZE;  \
    } while (0)
#define incp_t1(ptr, n)                         \
    do {                                        \
        if ((ptr += n) >= t1_buf + T1_BUF_SIZE) \
            ptr -= T1_BUF_SIZE;                 \
    } while (0)
#define decp_t1(ptr, n)          \
    do {                         \
        if ((ptr -= n) < t1_buf) \
            ptr += T1_BUF_SIZE;  \
    } while (0)

#define idle_uart1() \
    (_pastn(jif_t1) > 50 && _pastn(jif_r1) > 50)
#define free_uart1() \
    (_pastn(jif_t1) > 500 && _pastn(jif_r1) > 500) //jiff_tx 2000

#define new_r1() \
    (r1_head >= r1_tail ? r1_head - r1_tail : r1_head + R1_BUF_SIZE - r1_tail)
#define free_t1() \
    (t1_tail > t1_head ? t1_tail - t1_head - 1 : t1_tail + T1_BUF_SIZE - t1_head - 1)
#define over_r1() \
    (r1_head == r1_tail - 1 || r1_head == r1_tail + R1_BUF_SIZE - 1)
#define over_t1() \
    (t1_head == t1_tail - 1 || t1_head == t1_tail + T1_BUF_SIZE - 1)

#define free_rcd_t1() (t1_tail > t1_head ? t1_tail - t1_head - 1 : t1_tail + T1_BUF_SIZE - t1_head - 1)

#define clr_buf_t1() (t1_tail = t1_head)
#define clr_buf_r1() (r1_tail = r1_head)
#define subp_r1(p1, p2) (p1 >= p2 ? p1 - p2 : p1 + R1_BUF_SIZE - p2)
#define subp_t1(p1, p2) (p1 >= p2 ? p1 - p2 : p1 + T1_BUF_SIZE - p2)

extern UART_MANAGE_STRUCT read_uar1;

extern void load_uart1_remap(void);
extern u32 get_uart1_func(void);
extern bool uart1_update_func(u16 fun, u16 set);
extern void uart1_tran_task(void);
extern void uart1_tran_task_nms(int nms);
extern void uart1_recive_task(void);

extern void write_uart1(u8 *buf, u16 len);
extern void write_uart1_hex(u8 hex);
extern u16 read_uart1(u8 *buf, u8 *rbuf_ptr, u16 n);
extern void uart1_debug_proc(void);

#endif
