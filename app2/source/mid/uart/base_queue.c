#include "include_all.h"



//��ն���,�߼����,�����������,ʵ�����ݻ������ڴ���
static void queue_clear(QUEUE_S * que)
{
    que->head = que->tail = 0;

}

//����Ƿ�Ϊ�ն���, ���Ϊ�� ���� true ,��Ϊ�� ���� fase
static bool queue_is_empty(QUEUE_S * que)
{
    if (que->head == que->tail)
    {
        return true;
    }
    else
        return false;
}



//����Ƿ�Ϊ������,������ture, �񷵻� false
static bool queue_is_full(QUEUE_S* que)
{
    if(((que->tail + 1) % que->buf_len) == que->head)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#if 0
//������д��һ���ֽ�����,���ݴӶ�β����,�ɹ����� true,ʧ�ܷ��� false,
//�ú������������������,����������ݸ���,�����ݲ���ѭ���洢
static bool queue_input_byte(QUEUE_S * que, u8 dat)
{
    if(!queue_is_full(que)) //�������û����
    {
        que->buf_ptr[que->tail] = dat;
        que->tail ++;
        if(que->tail == que->buf_len)         //��ֹ�����±�Խ��
        {
            que->tail = que->tail % que->buf_len;
        }
        return true;
    }
    else
    {
        return  false;
    }
}
#else
//������д��һ���ֽ�����,���ݴӶ�β����,�ɹ����� true,ʧ�ܷ��� false
//�ú���Ϊѭ������,��������ʱ,�Ὣ֮ǰ���ݸ���
static void queue_input_byte(QUEUE_S * que, u8 dat)
{
    que->buf_ptr[que->tail] = dat;
    que->tail ++;
    que->tail = que->tail % que->buf_len;
}

#endif 

//�Ӷ���ȡ��һ���ֽ�����,���ݴӶ�ͷ����,�ɹ����� true,ʧ�ܷ��� false
static bool queue_output_byte(QUEUE_S * que, u8 *data)
{
    if(!queue_is_empty(que))            //������в�Ϊ��
    {
        *data = que->buf_ptr[que->head];
        que->head ++;
        que->head = que->head % que->buf_len; //��ֹԽ��
        if(que->head == que->tail)     //�����Ѿ�ȡ��
        {
            que->head = que->tail  = 0; //���±긴λ,��ն���
        }
        return true;
    }
    else
    {
        return false;
    }

}

//��ȡ�����е����ݳ���
static u16 queue_get_data_length(QUEUE_S * que)
{
    u16 len;
    if(!queue_is_empty(que)) //������в�Ϊ��
    {
        if(que->head < que->tail )
        {
            len = que->tail - que->head;
        }
        else
        {
            len = (que->buf_len - que->head + que->tail);
        }
    }
    else
    {
        len = 0;
    }
    return len;
}


//�������д��ָ�����ȵ�����,�ɹ����� true ,ʧ�ܷ��� false
static bool queue_input_len(QUEUE_S *que, u8 *pbuf, u16 buf_len)
{
    u16 remain_data_len ;       //������ʣ�����ݿռ䳤��
    u16 queue_len;              //��ǰ�������ݳ���
    u16 tmp;
    if(queue_is_full(que))
    {
        return false;
    }
    queue_len = queue_get_data_length(que);
    remain_data_len = que->buf_len - queue_len;
    if(buf_len <= remain_data_len)//���Ҫд����е����ݳ���С�ڶ�����ʣ�����ݿռ䳤��
    {
        if((que->buf_len - que->tail) > buf_len)
        {
            memcpy(&(que->buf_ptr[que->tail]), pbuf, buf_len);
            que->tail += buf_len;
        }
        else
        {
            tmp =  que->buf_len - que->tail;
            memcpy(&(que->buf_ptr[que->tail]), pbuf, tmp);
            que->tail = 0;
            memcpy(&(que->buf_ptr[que->tail]), (pbuf + tmp), buf_len - tmp);
            que->tail += (buf_len - tmp);
        }
        return true;
    }
    else //���Ҫд����е����ݳ��� ����               ������ʣ�����ݿռ䳤��
    {
        return false;
    }
}


//ȡ�������е���������,�ɹ����� true, ʧ�ܷ��� false
static bool queue_output_all(QUEUE_S *que, u8 *pbuf)
{
    u16 que_len;
    u16 tmp;
    if(queue_is_empty(que))
    {
        return false;
    }
    que_len = queue_get_data_length(que);
    if(que->tail > que->head)
    {
        memcpy(pbuf,&(que->buf_ptr[que->head]), que_len);
    }
    else
    {
        tmp = que->buf_len - que->head;
        memcpy(pbuf, &(que->buf_ptr[que->head]), tmp);
        que->head = 0;
        memcpy((pbuf + tmp), &(que->buf_ptr[que->head]), (que_len - tmp));
    }
    que->head = 0;
    que->tail = que->head;      //����ȡ��,��ն���
    return true;
}



//�Ӷ�����ȡ��ָ����������,�ɹ����� true, ʧ�ܷ���false
static bool queue_output_len(QUEUE_S *que, u8 *pbuf, u16 *out_len)
{
    u16 que_len;
    u16 tmp;
    if(queue_is_empty(que))
    {
        *out_len = 0;
        return false;
    }
    que_len = queue_get_data_length(que);
    if((*out_len) > que_len )
    {
        *out_len = que_len;
    }

    if(que->tail > que->head)
    {
        memcpy(pbuf,&(que->buf_ptr[que->head]), (u16)(*out_len));
        que->head += *out_len;
        que->head %= que->buf_len;
    }
    else
    {
        tmp = que->buf_len - que->head;
        memcpy( pbuf,&(que->buf_ptr[que->head]), tmp);
        que->head = 0;
        memcpy( (pbuf + tmp), &(que->buf_ptr[que->head]),((*out_len) - tmp));
        que->head  = (*out_len) - tmp;
        que->head %= que->buf_len;
    }

    return true;
}



void base_queue_func_init(Base_Queue_func_t * p)
{
    p->que_clear         =       queue_clear;
    p->que_get_data_len  =       queue_get_data_length;
    p->que_input_byte    =       queue_input_byte;
    p->que_input_len     =       queue_input_len;
    p->que_is_empty      =       queue_is_empty;
    p->que_is_full       =       queue_is_full;
    p->que_output_all    =       queue_output_all;
    p->que_output_byte   =       queue_output_byte;
    p->que_output_len    =       queue_output_len;
}





















