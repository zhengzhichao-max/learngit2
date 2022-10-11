/**
  ******************************************************************************
  * @file    uart_pub.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ���ڹ���
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

#ifndef __UART_PUB_H
#define __UART_PUB_H

typedef union {
    u32 reg;
    struct {
        u32 tt : 1;         //͸��		//͸��??-�����ֿ⼰BOOT
        u32 debug : 1;      //��ӡ������Ϣ��TR9����
        u32 rcd_3c : 1;     //lcd 		//Ĭ�Ϲ���
        u32 speedlimit : 1; //����
        u32 tr9 : 1;//͸����ʾRK
        u32 weight : 1;    //����
        u32 handset : 1;   //
        u32 read_card : 1; //������
        u32 yd_oil : 1;    //�ͱ�

        //u32 led		 	:1;		//��ʾ��
        u32 navigation : 1; //����
        //u32 oil_mass		:1;		//����
        //u32 ethyl_alcohol	:1;		//�ƾ�

        u32 res2 : 6;
        u32 res4 : 8;
        u32 res3 : 8;
    } b;
} uart_func_list;

typedef struct {
    u16 crc;           //������Ӹñ���, ee_pwrite ���crc
    u16 flag;          //������Ӹñ���, ee_pwrite ���8700
    uart_func_list u1; //����1 ����
    uart_func_list u4; //����4 ����
    u8 u5_func;        //����8002 ����
    u8 res1;
    u8 res[12];
} s_any_uart_set;

extern s_any_uart_set us;

typedef struct {
    u32 jiff; //�����Ե�ʱ��
} S_BYPASS_DEBUG;
extern S_BYPASS_DEBUG bypass_debug;

typedef struct {
    bool delay_en;  //��ʱʹ��
    bool delay_err; //��ʱ����
    u16 res;
    u32 delay_jiff; //��ʱ��ʱ��
} any_uart_remap_struct;
extern any_uart_remap_struct uart_remap;

typedef struct {
    bool news; //�µ���Ϣ
    bool ack;  //�Ѵ�����ɣ��ȴ�Ų��ָ��
    u8 *cur_p; //��ǰ֡λ��
    u8 res;    //����
    u16 c_len; //��ǰ֡����
    u16 res1;  //����
    u32 n_jfs; //news jiffies
} UART_MANAGE_STRUCT;

extern u8 get_flag_7e_cnt(u8 *buf_t, u16 b_len_t, u8 flag);
extern s16 get_flag_7e_pointer(u8 *buf_t, u16 b_len_t, u8 flag);

extern bool verify_any_uart_func(u8 source, u16 fun1);
extern void send_any_uart(u8 source, u8 *data1, u16 len1);
extern void load_any_uart_remap(void);
extern u8 get_uart_news(UART_MANAGE_STRUCT *c);
extern u8 get_uart_tr9(UART_MANAGE_STRUCT *c);
extern u8 get_uart_3c(UART_MANAGE_STRUCT *c);

extern u16 copy_uart_news(u8 uart_id_t, u8 *p, u8 *s, u16 s_len);
extern void set_uart_ack(u8 from);
extern void uart_manage_task(void);
#endif /* __UART_PUB_H */
