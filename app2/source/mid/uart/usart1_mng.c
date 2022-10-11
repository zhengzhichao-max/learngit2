#include "include_all.h"


//#define UART1_RECV_BUFF_MAX_LGTH        ((uint)512)   //串口1接收缓冲区最大长度
//#define UART1_SEND_BUFF_MAX_LGTH        ((uint)512)   //串口1发送缓冲区最大长度

#define UART1_RECV_BUFF_MAX_LGTH        ((uint)1024)   //串口1接收缓冲区最大长度
#define UART1_SEND_BUFF_MAX_LGTH        ((uint)1024)   //串口1发送缓冲区最大长度

static bool  debug_flag = false;


uchar   uart1_recv_buff[UART1_RECV_BUFF_MAX_LGTH] = {0x00};      //串口1接收缓冲区
uchar   uart1_send_buff[UART1_SEND_BUFF_MAX_LGTH] = {0x00};      //串口1发送缓冲区


Uart_obj_t uart1_obj;

//串口1初始化
void uart1_dev_init(void)
{
    uart1_obj.uart.queue.new_recv        =  false;
    uart1_obj.uart.queue.new_recv_len    =  0;

    uart1_obj.uart.queue.recv.buf_ptr    =  uart1_recv_buff;
    uart1_obj.uart.queue.recv.buf_len    =  UART1_RECV_BUFF_MAX_LGTH;
    uart1_obj.uart.queue.recv.head       =  0;
    uart1_obj.uart.queue.recv.tail       =  0;

    uart1_obj.uart.queue.send.buf_ptr    =  uart1_send_buff;
    uart1_obj.uart.queue.send.buf_len    =  UART1_SEND_BUFF_MAX_LGTH;
    uart1_obj.uart.queue.send.head       =  0;
    uart1_obj.uart.queue.send.tail       =  0;

    uart1_obj.uart.id                    = STM32_UART_1;
    uart1_obj.uart.BaudRate              = 115200;
    uart1_obj.uart.Parity                = USART_Parity_No;
    uart1_obj.uart.StopBits              = USART_StopBits_1;
    uart1_obj.uart.WordLength            = USART_WordLength_8b;

    base_queue_func_init(&uart1_obj.uart.queue.func);  //初始化队列操作函数
    base_uart_func_init(&uart1_obj.func);              //初始化串口操作函数
    uart1_obj.func.uart_cfg(&uart1_obj.uart);          //调用串口配置函数,初始化串口

}

/*****************************************************************
函 数 名: my_printf()
功能描述: 向串口1输出打印信息,输出信息不能超过235个字节
入口参数:
返回  值:
*****************************************************************/
void my_printf(const char* fmt,...)
{
    int n = 0;
    char buf[UART1_RECV_BUFF_MAX_LGTH];

   if(debug_flag == true)
   {
       va_list args;
       
       va_start(args, fmt);
       n = vsprintf(buf, fmt, args);
       
       if(n>0 && n<500)
       {
           uart1_obj.func.send_data(&uart1_obj.uart,(u8 *)buf,n);
       }
       va_end(args);
   }
}



void uart1_input_byte(u8 data)
{
    uart1_obj.func.rbuf_input_byte(&uart1_obj.uart,data);
}


bool uart1_output_byte(u8 *out)
{
    return uart1_obj.func.sbuf_output_byte(&uart1_obj.uart,out);
}



void uart1_set_recv_sta(bool sta)
{
    uart1_obj.func.set_recv_sta(&uart1_obj.uart,sta);
}

void uart1_set_recv_len(u16 len)
{
    uart1_obj.func.set_recv_len(&uart1_obj.uart,len);
}



static void uart1_send_task(void)
{
    uart1_obj.func.send_task(&uart1_obj.uart);
}



static void uart1_recv_task(void)
{
    u8 rbuf[512] = {0x00};
    u8 sbuf[512] = {0x00};
    u16 rlen = 0;
    if(uart1_obj.func.read_data(&uart1_obj.uart,rbuf,&rlen))
    {
       if( NULL != strstr((const char *)&rbuf,"DEBUG_CLOSE"))
       {
            debug_flag = false;
       }

      if( NULL != strstr((const char *)&rbuf,"DEBUG_OPEN"))
       {
            debug_flag = true;
       }
       
        if( NULL != strstr((const char *)&rbuf,"SET_SN"))
        {
            Parameter_process_handle(rbuf,sbuf);
            uart1_obj.func.send_data(&uart1_obj.uart,sbuf,strlen((const char *)&sbuf));
        }
        else if(NULL != strstr((const char *)&rbuf,"AT+"))
        {
           Parameter_process_handle((rbuf+3),sbuf);
           uart1_obj.func.send_data(&uart1_obj.uart,sbuf,strlen((const char *)&sbuf));
        }
        else
        {
            uart1_obj.func.send_data(&uart1_obj.uart,rbuf,rlen);
        }
    }
}



void uart1_SendAndRecv_task(void)
{
    uart1_send_task();
    uart1_recv_task();
}

































