#ifndef _UART_PUB_DRV_H
#define _UART_PUB_DRV_H


#include "include_all.h"

typedef enum
{
    STM32_UART_1 = 0,
    STM32_UART_2,
    STM32_UART_3,
    STM32_UART_4,
    STM32_UART_5,
    STM32_UART_MAX,
}STM32_COM_E;



typedef struct
{
        bool                         new_recv;           //收到一帧新数据
        u16                          new_recv_len;       //收到一帧新数据长度
        QUEUE_S                      send;               //发送队列
        QUEUE_S                      recv;               //接收队列
        Base_Queue_func_t            func;               //队列操作函数
       
}Queue_t;

typedef struct 
{ 
   Queue_t                      queue;         //队列操作相关
   STM32_COM_E                  id;            //串口ID
   u32                          BaudRate;      //波特率
   u16                          WordLength;    //数据位字长
   u16                          StopBits;      //停止位长
   u16                          Parity;        //校验类型
}Uart_t;

typedef struct
{
    void (*uart_cfg)(Uart_t *);
    void (*uart_close)(Uart_t *);
    void (*rbuf_input_byte)(Uart_t *, u8 );
    void (*sbuf_input_byte)(Uart_t *, u8 );
    bool (*rbuf_output_byte)(Uart_t *, u8*);
    bool (*sbuf_output_byte)(Uart_t *, u8*);
    void (*clear_recv)(Uart_t *);
    void (*clear_send)(Uart_t *);
    void (*set_recv_sta)(Uart_t *,bool );
    void (*set_recv_len)(Uart_t *,u16 );
     u16 (*get_recv_len)(Uart_t *);
    bool (*get_recv_sta)(Uart_t *);
    bool (*send_data)(Uart_t *,u8 *,u16 );
    bool (*read_data)(Uart_t *,u8 *, u16 *);
    void (*send_task)(Uart_t *);
} Base_uart_func_t;



typedef struct 
{
  Uart_t   uart;
  Base_uart_func_t func;
  
}Uart_obj_t;


extern void base_uart_func_init(Base_uart_func_t *pfunc);


#endif












































