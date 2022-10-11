#include "include_all.h"



//清空队列,逻辑清空,并非物理清空,实际数据还会在内存中
static void queue_clear(QUEUE_S * que)
{
    que->head = que->tail = 0;

}

//检测是否为空队列, 如果为空 返回 true ,不为空 返回 fase
static bool queue_is_empty(QUEUE_S * que)
{
    if (que->head == que->tail)
    {
        return true;
    }
    else
        return false;
}



//检测是否为满队列,满返回ture, 否返回 false
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
//向队列中存放一个字节数据,数据从队尾入列,成功返回 true,失败返回 false,
//该函数考虑了满队列情况,不会出现数据覆盖,但数据不能循环存储
static bool queue_input_byte(QUEUE_S * que, u8 dat)
{
    if(!queue_is_full(que)) //如果队列没有满
    {
        que->buf_ptr[que->tail] = dat;
        que->tail ++;
        if(que->tail == que->buf_len)         //防止数组下标越界
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
//向队列中存放一个字节数据,数据从队尾入列,成功返回 true,失败返回 false
//该函数为循环队列,当队列满时,会将之前数据覆盖
static void queue_input_byte(QUEUE_S * que, u8 dat)
{
    que->buf_ptr[que->tail] = dat;
    que->tail ++;
    que->tail = que->tail % que->buf_len;
}

#endif 

//从队列取出一个字节数据,数据从对头出列,成功返回 true,失败返回 false
static bool queue_output_byte(QUEUE_S * que, u8 *data)
{
    if(!queue_is_empty(que))            //如果队列不为空
    {
        *data = que->buf_ptr[que->head];
        que->head ++;
        que->head = que->head % que->buf_len; //防止越界
        if(que->head == que->tail)     //数据已经取完
        {
            que->head = que->tail  = 0; //将下标复位,清空队列
        }
        return true;
    }
    else
    {
        return false;
    }

}

//获取队列中的数据长度
static u16 queue_get_data_length(QUEUE_S * que)
{
    u16 len;
    if(!queue_is_empty(que)) //如果队列不为空
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


//向队列中写入指定长度的数据,成功返回 true ,失败返回 false
static bool queue_input_len(QUEUE_S *que, u8 *pbuf, u16 buf_len)
{
    u16 remain_data_len ;       //队列中剩余数据空间长度
    u16 queue_len;              //当前队列数据长度
    u16 tmp;
    if(queue_is_full(que))
    {
        return false;
    }
    queue_len = queue_get_data_length(que);
    remain_data_len = que->buf_len - queue_len;
    if(buf_len <= remain_data_len)//如果要写入队列的数据长度小于队列中剩余数据空间长度
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
    else //如果要写入队列的数据长度 大于               队列中剩余数据空间长度
    {
        return false;
    }
}


//取出队列中的所有数据,成功返回 true, 失败返回 false
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
    que->tail = que->head;      //数据取完,清空队列
    return true;
}



//从队列中取出指定长度数据,成功返回 true, 失败返回false
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





















