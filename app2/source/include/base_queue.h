#ifndef _BASE_QUEUE_H_
#define _BASE_QUEUE_H_

#include "include_all.h"

typedef struct
{
  u16 buf_len;          //队列长度
  u8 *buf_ptr;          //队列指针
  u16 head;             //对头
  u16 tail;             //对尾 
}QUEUE_S; 


typedef struct 
{
   void(*que_clear)(QUEUE_S *  que);
   bool(*que_is_empty)(QUEUE_S * que);
   bool(*que_is_full)(QUEUE_S * que);
   bool(*que_output_byte)(QUEUE_S * que, u8 *buf);
   void(*que_input_byte)(QUEUE_S * que,u8 data);
   u16(*que_get_data_len)(QUEUE_S * que);
   bool(*que_input_len)(QUEUE_S* que,u8 *pbuf,u16 len);
   bool(*que_output_all)(QUEUE_S * que,u8 *pbuf);
   bool(*que_output_len)(QUEUE_S * que,u8 *pbuf,u16 *len);

}Base_Queue_func_t;


extern void base_queue_func_init(Base_Queue_func_t * p);



#endif


