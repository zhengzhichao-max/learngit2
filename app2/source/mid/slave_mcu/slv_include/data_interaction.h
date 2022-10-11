#ifndef  __data_interaction_h__
#define  __data_interaction_h__

typedef void (*funcp_slv)(void);

typedef struct {
	u32 index;
	funcp_slv func;
} slave_rev_handset;

//-----------------------------------
//-----        �����      --------
#define      serial_comm_rdsr         0x05     //��״̬
#define      serial_comm_read         0x03     //������
#define      serial_comm_write        0x02     //д����
//-----------------------------------

//-----------------------------------
//-----     ��չЭ��Ŷ���   --------
#define      expand_num         ((u8)(0xee))
//-----------------------------------

//-----------------------------------
//------------����״̬---------------
#define      comm_free         ((u32)(0x00))
//-----------------------------------

#define    single_packet_max_lgth        ((u16)1024)   //�������ݳ���
#define    spi_rec_valid_start           ((u8)3)      //ʵ����Ч������ʼλ��
#define    spi_rec_expand_valid_start   ((u8)6)      //���ڲ���Э����չԭ��Э��ʵ����Ч������ʼλ��

#define    host_tf_usb_file_name_max_lgth  ((u8)60)   //��������TF����U���ļ�������
#define    host_tf_usb_file_max_lgth        ((u16)768)  //��������TF����U���ļ����ݳ���

typedef enum
{
    comm_average  =  0,   //һ������
	comm_set,          //��������  ������IP��
	comm_com,          //��������  ���ѯ������
}
spi_comm_type_enum;  //��������


typedef struct
{
	spi_ack_struct    buff[spi1_up_comm_team_max];

	u8  head;  //����ͷλ��
	u8  tail;  //����βλ��
}
spi1_up_data_struct;


typedef struct
{
	spi_ack_struct    current_comm;  //��ǰ�����͵�����Ĳ���

	u16    resend_time;       //�ط�ʱ���ʱ
	u8   resend_cnt;

	u8   step;              //���Ͳ���
}
spi_send_struct;          //��оƬ����оƬ�������ݽṹ��


typedef struct
{
	u8   name_lgth;      //�ļ�������
	u8   name_buff[host_tf_usb_file_name_max_lgth+2];  //�ļ���

	u32   file_lgth;  //�ļ������ܳ���
	u32   op_ptr;     //����ָ��
	u16    op_lgth;    //�������ݳ���

	u8   buff[host_tf_usb_file_max_lgth];  //��������
}
file_infor_struct;       //�ļ���Ϣ�ṹ��


typedef void (*array_move_base)(void*, u16);           //�����ƶ�����ָ��
typedef u8 (*compare_base)(void*, short int, void*);   //�Ƚ��㷨
typedef u8 (*gbk_unicode_compare_base)(u32, void*);         //�Ƚ��㷨

typedef struct
{
	u32  function_comm;           //���������
	void   (*function_ptr)(void);    //ִ�к�����ָ��
}
function_table_struct;


typedef enum
{
	s_gb   = 0,  //����ԴΪ������
	s_un,        //����ԴΪUNICODE��
}
gb_unicode_source_type_enum;  //��������UNICODE��ת������Դ����


#define  gbk_unicode_buff_max_lgth  ((u16)256)  //��������UNICODE��ת�����ݻ�����󳤶�
typedef struct
{
	u16   lgth;
	u8  buff[gbk_unicode_buff_max_lgth*2];
}
gbk_unicode_buff_struct;  //��������UNICODE��ת������Դ����


extern void array_uchar_move_left(void* s_buff, u16 cnt);
extern void array_uint_move_left(void* s_buff, u16 cnt);
extern void array_uint_move_right(void* s_buff, u16 cnt);
extern void array_ulong_move_left(void* s_buff, u16 cnt);
extern void array_sms_use_move_left(void* s_buff, u16 cnt);
extern void array_sms_use_move_right(void* s_buff, u16 cnt);
extern void array_move(void* s_buff, u16 cnt, bool dir, array_move_base function_base);

extern u8 compare_buff(u8* buff1, u16 lgth1, u8* buff2, u16 lgth2);
extern bool search_half(bool dir, u32 addr, signed long int lgth, u8 siz, void* d_data, u32* ret_posi, gbk_unicode_compare_base cmp);
extern u16 gbk_unicode(bool typ, u8* d_buff, u8* s_buff, u16 lgth);

extern void slave_send_msg(u32 comm, u16 sla_comm, u8 sta, bool ack_flag, u8 que_max);
extern u8 slave_recive_msg(u8 *str_t, u16 len_t);

extern void slave_send_data_proc(void);
extern void slave_recive_data_proc(void);
extern void analyse_0x1f_driver_code(u8 from, u8 *str, u16 len);
extern void analyse_0x20_driver_qualification(u8 from, u8 *str, u16 len);
extern void up_packeg(u32 main_comm, u8* buff, u16 lgth);

extern void up_0xee_0x0702(void);

#endif




