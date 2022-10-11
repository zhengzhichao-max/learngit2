
#ifndef __call_record_h__
#define   __call_record_h__

#include "phone_book.h"

extern void write_data_to_call_record_flash(u32 addr,phbk_data_struct  phbk_data);
extern u8 call_record_load_index(call_record_type_enum  call_record_type,call_record_struct call_record);
//extern void call_record_dis_build(u8 d_buff,u8 **s_buff ,u8 lgth);

extern void  call_record_dis_build(u8* dest, u32 s_data, u8 len);

#endif





