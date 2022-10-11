#ifndef  __data_interaction_h__
#define  __data_interaction_h__

typedef void (*funcp_slv)(void);

typedef struct {
	u32 index;
	funcp_slv func;
} slave_rev_handset;

//-----------------------------------
//-----        命令定义      --------
#define      serial_comm_rdsr         0x05     //读状态
#define      serial_comm_read         0x03     //读数据
#define      serial_comm_write        0x02     //写数据
//-----------------------------------

//-----------------------------------
//-----     扩展协议号定义   --------
#define      expand_num         ((u8)(0xee))
//-----------------------------------

//-----------------------------------
//------------空闲状态---------------
#define      comm_free         ((u32)(0x00))
//-----------------------------------

#define    single_packet_max_lgth        ((u16)1024)   //单包数据长度
#define    spi_rec_valid_start           ((u8)3)      //实际有效数据起始位置
#define    spi_rec_expand_valid_start   ((u8)6)      //基于部标协议扩展原有协议实际有效数据起始位置

#define    host_tf_usb_file_name_max_lgth  ((u8)60)   //主机操作TF卡和U盘文件名长度
#define    host_tf_usb_file_max_lgth        ((u16)768)  //主机操作TF卡和U盘文件内容长度

typedef enum
{
    comm_average  =  0,   //一般类型
	comm_set,          //设置类型  如设置IP等
	comm_com,          //命令类型  如查询参数等
}
spi_comm_type_enum;  //命令类型


typedef struct
{
	spi_ack_struct    buff[spi1_up_comm_team_max];

	u8  head;  //队列头位置
	u8  tail;  //队列尾位置
}
spi1_up_data_struct;


typedef struct
{
	spi_ack_struct    current_comm;  //当前待发送的命令的参数

	u16    resend_time;       //重发时间计时
	u8   resend_cnt;

	u8   step;              //发送步骤
}
spi_send_struct;          //从芯片与主芯片上行数据结构体


typedef struct
{
	u8   name_lgth;      //文件名长度
	u8   name_buff[host_tf_usb_file_name_max_lgth+2];  //文件名

	u32   file_lgth;  //文件内容总长度
	u32   op_ptr;     //操作指针
	u16    op_lgth;    //操作数据长度

	u8   buff[host_tf_usb_file_max_lgth];  //操作内容
}
file_infor_struct;       //文件信息结构体


typedef void (*array_move_base)(void*, u16);           //数组移动函数指针
typedef u8 (*compare_base)(void*, short int, void*);   //比较算法
typedef u8 (*gbk_unicode_compare_base)(u32, void*);         //比较算法

typedef struct
{
	u32  function_comm;           //函数命令号
	void   (*function_ptr)(void);    //执行函数的指针
}
function_table_struct;


typedef enum
{
	s_gb   = 0,  //数据源为国标码
	s_un,        //数据源为UNICODE码
}
gb_unicode_source_type_enum;  //国标码与UNICODE码转换数据源类型


#define  gbk_unicode_buff_max_lgth  ((u16)256)  //国标码与UNICODE码转换数据缓存最大长度
typedef struct
{
	u16   lgth;
	u8  buff[gbk_unicode_buff_max_lgth*2];
}
gbk_unicode_buff_struct;  //国标码与UNICODE码转换数据源类型


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




