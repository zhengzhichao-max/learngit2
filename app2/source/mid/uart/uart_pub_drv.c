#include "include_all.h"





/******************************************************************
 *函 数 名: uart_cfg
 *函数功能: 配置串口,启动串口, 该函数需要根据不同型号的芯片来修改GPIO配置
 *入口参数: 串口对象
 *返 回 值: 无
******************************************************************/
static void uart_cfg(Uart_t *p)
{
        USART_InitTypeDef USART_InitStructure = {0};
        GPIO_InitTypeDef GPIO_InitStructure   = {0};
        USART_TypeDef* USART_tmp = NULL;
        if(p->id == STM32_UART_1)  /*USART1 GPIO配置*/
        {//LED串口, 行驶记录仪串口,调试串口 复用
                USART_tmp = USART1;
               // GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   /* Connect PA9 to USART1_Tx */
               // GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  /* Connect PA10 to USART1_Rx*/

                RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //开启时钟
                GPIO_InitStructure.GPIO_Pin          = GPIO_Pin_9;
                GPIO_InitStructure.GPIO_Speed        = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode         = GPIO_Mode_AF_PP;
                GPIO_Init(GPIOA, &GPIO_InitStructure);   //PA9 USART1_TX

                GPIO_InitStructure.GPIO_Pin         = GPIO_Pin_10;
                GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IN_FLOATING;
                GPIO_Init(GPIOA, &GPIO_InitStructure);  //PA10 USART1_RX

        }
        else if(p->id == STM32_UART_2)    /*USART2 GPIO配置*/
        {//EC200A
                USART_tmp = USART2;
               // GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);  /* Connect PA2 to USART2_Tx */
               // GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);  /* Connect PA3 to USART2_Rx*/

                RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
                GPIO_InitStructure.GPIO_Pin          = GPIO_Pin_2;
                GPIO_InitStructure.GPIO_Speed        = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode         = GPIO_Mode_AF_PP;
                GPIO_Init(GPIOA, &GPIO_InitStructure); //PA2 USART2_TX

                GPIO_InitStructure.GPIO_Pin         = GPIO_Pin_3;
                GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IN_FLOATING;
                GPIO_Init(GPIOA, &GPIO_InitStructure); //PA3 USART2_RX
        }
        else if(p->id == STM32_UART_3) /*USART3 GPIO配置*/
        {//RK 串口
                USART_tmp = USART3;
               // GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);  /* Connect PB10 to USART2_Tx */
               // GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);  /* Connect PB11 to USART2_Rx*/

                RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
                GPIO_InitStructure.GPIO_Pin          = GPIO_Pin_10;
                GPIO_InitStructure.GPIO_Speed        = GPIO_Speed_10MHz;
                GPIO_InitStructure.GPIO_Mode         = GPIO_Mode_AF_PP;
                GPIO_Init(GPIOB, &GPIO_InitStructure); //PB10 USART3_TX

                GPIO_InitStructure.GPIO_Pin         = GPIO_Pin_11;
                GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_10MHz;
                GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IN_FLOATING;
                GPIO_Init(GPIOB, &GPIO_InitStructure); //PB11 USART3_RX
        }
        else if(p->id == STM32_UART_4) /*UART4 GPIO配置*/
        {//本项目用于 外部串口
                USART_tmp = UART4;
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
                GPIO_InitStructure.GPIO_Pin          = GPIO_Pin_10;
                GPIO_InitStructure.GPIO_Speed        = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode         = GPIO_Mode_AF_PP;
                GPIO_Init(GPIOC, &GPIO_InitStructure);  //PC10  UART4_TX

                GPIO_InitStructure.GPIO_Pin         = GPIO_Pin_11;
                GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IN_FLOATING;
                GPIO_Init(GPIOC, &GPIO_InitStructure); //PC11   UART4_RX

        }
        else if(p->id == STM32_UART_5) /*UART5 GPIO配置*/
        {//本项目为GPS 串口
                USART_tmp = UART5;
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
                
                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
                GPIO_Init(GPIOC, &GPIO_InitStructure);

                GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
                GPIO_Init(GPIOD, &GPIO_InitStructure);

        }
        else
        {
                nop();
                return;
        }
        USART_DeInit(USART_tmp);
        USART_Cmd(USART_tmp, DISABLE);
        USART_InitStructure.USART_BaudRate              = p->BaudRate;
        USART_InitStructure.USART_WordLength            = p->WordLength;
        USART_InitStructure.USART_StopBits              = p->StopBits;
        USART_InitStructure.USART_Parity                = p->Parity;
        USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None; //默认无硬件流控
        USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;  //默认为收发模式

        USART_Init(USART_tmp, & USART_InitStructure);
        USART_ITConfig(USART_tmp, USART_IT_RXNE, ENABLE);      //开启接收中断
        USART_ITConfig(USART_tmp, USART_IT_IDLE, ENABLE);      //开启USART1空闲中断
        USART_Cmd(USART_tmp, ENABLE);                          //启动串口

}



/******************************************************************
 *函 数 名: uart_cfg
 *函数功能: 关闭串口
 *入口参数: 串口对象
 *返 回 值: 无
******************************************************************/
static void uart_close(Uart_t *p)
{
        if(p->id == STM32_UART_1)
        {
                USART_DeInit(USART1);
                USART_Cmd(USART1, DISABLE);
        }
        else if(p->id == STM32_UART_2)
        {
                USART_DeInit(USART2);
                USART_Cmd(USART2, DISABLE);
        }
        else if(p->id == STM32_UART_3)
        {
                USART_DeInit(USART3);
                USART_Cmd(USART3, DISABLE);
        }
        else if(p->id == STM32_UART_4)
        {
                USART_DeInit(UART4);
                USART_Cmd(UART4, DISABLE);
        }
        else if(p->id == STM32_UART_5)
        {
                USART_DeInit(UART5);
                USART_Cmd(UART5, DISABLE);
        }
        else
        {
                nop();
        }

}




/******************************************************************
 *函 数 名: rbuf_input_byte
 *函数功能: 向串口接收队列缓存写入1个字节数据
 *入口参数: 要写入的字节
 *返 回 值: 无
******************************************************************/
static void rbuf_input_byte(Uart_t *p, u8 data)
{
        p->queue.func.que_input_byte(&p->queue.recv, data);
}


/******************************************************************
 *函 数 名: sbuf_input_byte
 *函数功能: 向串口发送队列缓存写入1个字节数据
 *入口参数: 要写入的字节
 *返 回 值: 无
******************************************************************/
static void sbuf_input_byte(Uart_t *p, u8 data)
{

        p->queue.func.que_input_byte(&p->queue.send, data);
}


/******************************************************************
 *函 数 名: rbuf_output_byte
 *函数功能: 向串口接收队列缓存读出1个字节数据
 *入口参数: 要写入的字节
 *返 回 值: 无
******************************************************************/
static bool rbuf_output_byte(Uart_t *p, u8 *out)
{

        return p->queue.func.que_output_byte(&p->queue.recv, out);

}



/******************************************************************
 *函 数 名: sbuf_input_byte
 *函数功能: 向串口发送队列缓存读出1个字节数据
 *入口参数: 要写入的字节
 *返 回 值: 无
******************************************************************/
static bool sbuf_output_byte(Uart_t *p, u8 *out)
{

        return p->queue.func.que_output_byte(&p->queue.send, out);

}


/******************************************************************
 *函 数 名: clear_recv
 *函数功能: 清空接收队列缓存
 *入口参数: 无
 *返 回 值: 无
*******************************************************************/
static void clear_recv(Uart_t *p)
{
        p->queue.func.que_clear(&p->queue.recv);
}


/*******************************************************************
 *函 数 名: clear_send
 *函数功能: 清空发送队列缓存
 *入口参数: 无
 *返 回 值: 无
********************************************************************/
static void clear_send(Uart_t *p)
{
        p->queue.func.que_clear(&p->queue.send);

}


/********************************************************************
 *函 数 名: set_recv_sta
 *函数功能: 设置队列接收状态
 *入口参数: tmp : true 收到一帧数据, false 处理完一帧数据
 *返 回 值: 无
*********************************************************************/
static void set_recv_sta(Uart_t *p, bool tmp)
{
        p->queue.new_recv          = tmp;
}

/********************************************************************
 *函 数 名: set_recv_sta
 *函数功能: 更新队列最新接收数据的长度
 *入口参数:
 *返 回 值: 无
*********************************************************************/
static void set_recv_len(Uart_t *p, u16 len)
{
        p->queue.new_recv_len    = len;
}

/********************************************************************
 *函 数 名: get_recv_sta
 *函数功能: 获取队列中最新接收的一帧数据的长度
 *入口参数: 无
 *返 回 值: 无
*********************************************************************/
static u16 get_recv_len(Uart_t *p)
{
        return p->queue.new_recv_len;
}


/********************************************************************
 *函 数 名: uart1_get_recv_sta
 *函数功能: 获取串口队列接收状态
 *入口参数: 无
 *返 回 值: bool 收到一帧新数据, false,没有收到一帧新数据
*********************************************************************/
static bool get_recv_sta(Uart_t *p)
{
        return p->queue.new_recv;
}


/********************************************************************
 *函 数 名: send_data
 *函数功能: 发送数据,注意这里只是将数据拷贝到串口发送队列中,并未启动
            串口发送数据
 *入口参数: *sbuf : 发送的数据, len :发送数据的长度
 *返 回 值: turn 成功,false 失败
*********************************************************************/
static bool send_data(Uart_t *p, u8 *sbuf, u16 len)
{
        if((sbuf == NULL) || (len == 0))
                return false;
        return p->queue.func.que_input_len(&p->queue.send, sbuf, len);
}



/********************************************************************
 *函 数 名: read_data
 *函数功能: 接收数据,从串口队列中读取一帧数据;
 *入口参数: rbuf : 读取的数据, len :实际读取到的数据的长度
 *返 回 值: turn 成功,false 失败
*********************************************************************/
static bool read_data(Uart_t *p, u8 *rbuf, u16 *len)
{
        u8 data_t;
        u16 i = 0;
        bool ret = false;
        if(get_recv_sta(p))     //串口收到数据
        {
                set_recv_sta(p, false);
                *len = get_recv_len(p);
                while(!p->queue.func.que_is_empty(&p->queue.recv))
                {
                        p->queue.func.que_output_byte(&p->queue.recv, &data_t);
                        rbuf[i++] = data_t;
                }
                *len = i;
                ret = true;
        }
        else
        {
                ret = false;
        }
        return ret;
}



/********************************************************************
 *函 数 名: send_task
 *函数功能: 串口发送任务,启动串口发送数据
 *入口参数: 无
 *返 回 值: 无
*********************************************************************/
static void send_task(Uart_t *p)
{

        if(!p->queue.func.que_is_empty(&p->queue.send))
        {
                if(p->id == STM32_UART_1)
                {
                        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
                }
                else if(p->id == STM32_UART_2)
                {
                        USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
                }
                else if(p->id == STM32_UART_3)
                {
                        USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
                }
                else if(p->id == STM32_UART_4)
                {
                        USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
                }
                else if(p->id == STM32_UART_5)
                {
                        USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
                }
                else
                {
                        nop();
                }
        }

}

/********************************************************************
 *函 数 名: base_uart_func_init
 *函数功能: 初始化串口操作函数
 *入口参数: 串口驱动实例化对象
 *返 回 值: 无
*********************************************************************/

void base_uart_func_init(Base_uart_func_t *pfunc)
{
        if(pfunc != NULL)
        {
                pfunc->clear_recv             = clear_recv;
                pfunc->clear_send             = clear_send;
                pfunc->get_recv_len           = get_recv_len;
                pfunc->get_recv_sta           = get_recv_sta;
                pfunc->rbuf_input_byte        = rbuf_input_byte;
                pfunc->rbuf_output_byte       = rbuf_output_byte;
                pfunc->sbuf_input_byte        = sbuf_input_byte;
                pfunc->sbuf_output_byte       = sbuf_output_byte;
                pfunc->send_data              = send_data;
                pfunc->read_data              = read_data;
                pfunc->send_task              = send_task;
                pfunc->set_recv_len           = set_recv_len;
                pfunc->set_recv_sta           = set_recv_sta;
                pfunc->uart_cfg               = uart_cfg;
                pfunc->uart_close             = uart_close;
        }
}





















