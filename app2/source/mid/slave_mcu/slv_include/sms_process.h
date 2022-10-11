#ifndef  __sms_process_h__
#define  __sms_process_h__


extern void sms_data_flash_init(void);
extern bit_enum sms_unread_scan(void);;

extern bool sms_operate_hdl(sms_index_struct* ptr, sms_op_enum op, u16 type, u16 ct_lgth, u8* ct_ptr);









#endif




