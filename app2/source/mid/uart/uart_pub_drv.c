#include "include_all.h"





/******************************************************************
 *�� �� ��: uart_cfg
 *��������: ���ô���,��������, �ú�����Ҫ���ݲ�ͬ�ͺŵ�оƬ���޸�GPIO����
 *��ڲ���: ���ڶ���
 *�� �� ֵ: ��
******************************************************************/
static void uart_cfg(Uart_t *p)
{
        USART_InitTypeDef USART_InitStructure = {0};
        GPIO_InitTypeDef GPIO_InitStructure   = {0};
        USART_TypeDef* USART_tmp = NULL;
        if(p->id == STM32_UART_1)  /*USART1 GPIO����*/
        {//LED����, ��ʻ��¼�Ǵ���,���Դ��� ����
                USART_tmp = USART1;
               // GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);   /* Connect PA9 to USART1_Tx */
               // GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);  /* Connect PA10 to USART1_Rx*/

                RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); //����ʱ��
                GPIO_InitStructure.GPIO_Pin          = GPIO_Pin_9;
                GPIO_InitStructure.GPIO_Speed        = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode         = GPIO_Mode_AF_PP;
                GPIO_Init(GPIOA, &GPIO_InitStructure);   //PA9 USART1_TX

                GPIO_InitStructure.GPIO_Pin         = GPIO_Pin_10;
                GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_2MHz;
                GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_IN_FLOATING;
                GPIO_Init(GPIOA, &GPIO_InitStructure);  //PA10 USART1_RX

        }
        else if(p->id == STM32_UART_2)    /*USART2 GPIO����*/
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
        else if(p->id == STM32_UART_3) /*USART3 GPIO����*/
        {//RK ����
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
        else if(p->id == STM32_UART_4) /*UART4 GPIO����*/
        {//����Ŀ���� �ⲿ����
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
        else if(p->id == STM32_UART_5) /*UART5 GPIO����*/
        {//����ĿΪGPS ����
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
        USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None; //Ĭ����Ӳ������
        USART_InitStructure.USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx;  //Ĭ��Ϊ�շ�ģʽ

        USART_Init(USART_tmp, & USART_InitStructure);
        USART_ITConfig(USART_tmp, USART_IT_RXNE, ENABLE);      //���������ж�
        USART_ITConfig(USART_tmp, USART_IT_IDLE, ENABLE);      //����USART1�����ж�
        USART_Cmd(USART_tmp, ENABLE);                          //��������

}



/******************************************************************
 *�� �� ��: uart_cfg
 *��������: �رմ���
 *��ڲ���: ���ڶ���
 *�� �� ֵ: ��
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
 *�� �� ��: rbuf_input_byte
 *��������: �򴮿ڽ��ն��л���д��1���ֽ�����
 *��ڲ���: Ҫд����ֽ�
 *�� �� ֵ: ��
******************************************************************/
static void rbuf_input_byte(Uart_t *p, u8 data)
{
        p->queue.func.que_input_byte(&p->queue.recv, data);
}


/******************************************************************
 *�� �� ��: sbuf_input_byte
 *��������: �򴮿ڷ��Ͷ��л���д��1���ֽ�����
 *��ڲ���: Ҫд����ֽ�
 *�� �� ֵ: ��
******************************************************************/
static void sbuf_input_byte(Uart_t *p, u8 data)
{

        p->queue.func.que_input_byte(&p->queue.send, data);
}


/******************************************************************
 *�� �� ��: rbuf_output_byte
 *��������: �򴮿ڽ��ն��л������1���ֽ�����
 *��ڲ���: Ҫд����ֽ�
 *�� �� ֵ: ��
******************************************************************/
static bool rbuf_output_byte(Uart_t *p, u8 *out)
{

        return p->queue.func.que_output_byte(&p->queue.recv, out);

}



/******************************************************************
 *�� �� ��: sbuf_input_byte
 *��������: �򴮿ڷ��Ͷ��л������1���ֽ�����
 *��ڲ���: Ҫд����ֽ�
 *�� �� ֵ: ��
******************************************************************/
static bool sbuf_output_byte(Uart_t *p, u8 *out)
{

        return p->queue.func.que_output_byte(&p->queue.send, out);

}


/******************************************************************
 *�� �� ��: clear_recv
 *��������: ��ս��ն��л���
 *��ڲ���: ��
 *�� �� ֵ: ��
*******************************************************************/
static void clear_recv(Uart_t *p)
{
        p->queue.func.que_clear(&p->queue.recv);
}


/*******************************************************************
 *�� �� ��: clear_send
 *��������: ��շ��Ͷ��л���
 *��ڲ���: ��
 *�� �� ֵ: ��
********************************************************************/
static void clear_send(Uart_t *p)
{
        p->queue.func.que_clear(&p->queue.send);

}


/********************************************************************
 *�� �� ��: set_recv_sta
 *��������: ���ö��н���״̬
 *��ڲ���: tmp : true �յ�һ֡����, false ������һ֡����
 *�� �� ֵ: ��
*********************************************************************/
static void set_recv_sta(Uart_t *p, bool tmp)
{
        p->queue.new_recv          = tmp;
}

/********************************************************************
 *�� �� ��: set_recv_sta
 *��������: ���¶������½������ݵĳ���
 *��ڲ���:
 *�� �� ֵ: ��
*********************************************************************/
static void set_recv_len(Uart_t *p, u16 len)
{
        p->queue.new_recv_len    = len;
}

/********************************************************************
 *�� �� ��: get_recv_sta
 *��������: ��ȡ���������½��յ�һ֡���ݵĳ���
 *��ڲ���: ��
 *�� �� ֵ: ��
*********************************************************************/
static u16 get_recv_len(Uart_t *p)
{
        return p->queue.new_recv_len;
}


/********************************************************************
 *�� �� ��: uart1_get_recv_sta
 *��������: ��ȡ���ڶ��н���״̬
 *��ڲ���: ��
 *�� �� ֵ: bool �յ�һ֡������, false,û���յ�һ֡������
*********************************************************************/
static bool get_recv_sta(Uart_t *p)
{
        return p->queue.new_recv;
}


/********************************************************************
 *�� �� ��: send_data
 *��������: ��������,ע������ֻ�ǽ����ݿ��������ڷ��Ͷ�����,��δ����
            ���ڷ�������
 *��ڲ���: *sbuf : ���͵�����, len :�������ݵĳ���
 *�� �� ֵ: turn �ɹ�,false ʧ��
*********************************************************************/
static bool send_data(Uart_t *p, u8 *sbuf, u16 len)
{
        if((sbuf == NULL) || (len == 0))
                return false;
        return p->queue.func.que_input_len(&p->queue.send, sbuf, len);
}



/********************************************************************
 *�� �� ��: read_data
 *��������: ��������,�Ӵ��ڶ����ж�ȡһ֡����;
 *��ڲ���: rbuf : ��ȡ������, len :ʵ�ʶ�ȡ�������ݵĳ���
 *�� �� ֵ: turn �ɹ�,false ʧ��
*********************************************************************/
static bool read_data(Uart_t *p, u8 *rbuf, u16 *len)
{
        u8 data_t;
        u16 i = 0;
        bool ret = false;
        if(get_recv_sta(p))     //�����յ�����
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
 *�� �� ��: send_task
 *��������: ���ڷ�������,�������ڷ�������
 *��ڲ���: ��
 *�� �� ֵ: ��
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
 *�� �� ��: base_uart_func_init
 *��������: ��ʼ�����ڲ�������
 *��ڲ���: ��������ʵ��������
 *�� �� ֵ: ��
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





















