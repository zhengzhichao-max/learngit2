/**
  ******************************************************************************
  * @file    uart1.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  串口1管理
  ******************************************************************************
  * @attention
	1.串口1用于支持多种外设
	2.调度屏或平台可以对串口进行配置
	3.串口被配置为非调度屏接口，复位后的前30秒钟，默认与调度屏连接，若收到调度屏的信息
	，延时30秒退出调度屏模式，进入配置的串口模式。
  ******************************************************************************
*/
#ifndef __UART1_H
#define __UART1_H

#define R1_BUF_DEBUG_SIZE 512
#define R1_BUF_SIZE 2400
#define T1_BUF_SIZE 1600 //2018-3-5 ,陈工发现660个字节会有数组溢出,因此更改为1024个字节

#define JT808_MIM_SIZE 7
#define JT808_MAM_SIZE 128

extern uc32 uart1_baud_rate[];

typedef enum {
    BY_TEMP = 0,
    BY_SETUP
} uart_set_list;

typedef struct {
    bool update_baud; //刷新波特率
    bool bypass_sw;   //该串口暂时被当前设置的功能
    bool err;         //错误
    bool res;
    u32 bypass_tick;    //被忽略的时间
    u32 cur_baud;       //当前波特率
    uart_func_list cur; //当前的串口功能
    uart_set_list set;  //设置的串口功能
    uart_func_list bak; //备份的串口功能
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
