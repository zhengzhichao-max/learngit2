#ifndef  __ic4442_driver_h__
#define  __ic4442_driver_h__










//*****************************************************************************
//*****************************************************************************
//-----------		               特殊值定义   	          -----------------
//-----------------------------------------------------------------------------

#define  ic4442_reset_rec_code      ((ulong)0x911013a2)    //IC4442卡默认复位应答代码
#define  ic4442_default_password    ((ulong)0x00ffffff)    //缺省密码
#define  ic4442_wait_busy_max_clk   ((uint)280)             //等待命令执行时间，以时钟脉冲计数

#define  ic4442_user_start_addr     ((uchar)0x20)           //4442卡用户使用的起始地址




//*****************************************************************************//
//*****************************************************************************//
//-----------						4442卡命令			      -----------------//
//-----------------------------------------------------------------------------//
#define  ic4442_comm_read_main_mem     ((uchar)0x30)  //读主存储区
#define  ic4442_comm_read_prot_mem     ((uchar)0x34)  //读保护存储区
#define  ic4442_comm_read_secu_mem     ((uchar)0x31)  //读安全存储区

#define  ic4442_comm_write_main_mem    ((uchar)0x38)  //更新主存储区
#define  ic4442_comm_write_prot_mem    ((uchar)0x3c)  //写保护存储区
#define  ic4442_comm_write_secu_mem    ((uchar)0x39)  //更新安全存储区

#define  ic4442_comm_comp_ver_data     ((uchar)0x33)  //比较认证数据




typedef  enum
{
	read_main_mem = ic4442_comm_read_main_mem,  //读取256个字节
	read_prot_mem = ic4442_comm_read_prot_mem,  //读取4个字节
	read_secu_mem = ic4442_comm_read_secu_mem,  //读取4个字节

	write_main_mem = ic4442_comm_write_main_mem, 
	write_prot_mem = ic4442_comm_write_prot_mem, 
	write_secu_mem = ic4442_comm_write_secu_mem, 

	comp_ver_data = ic4442_comm_comp_ver_data,    
}
ic4442_comm_enum;  //4442卡命令类型






//*****************************************************************************
//*****************************************************************************
//-----------           需要外部函数及变量支持                -----------------
//-----------------------------------------------------------------------------
typedef struct
{
	void (*dly_us)(uchar t);    //微秒延时

	void (*rst_set)(bit_enum s);    //复位脚端口控制  TRUE为高  FALSE为低
	void (*io_set)(bit_enum s);     //数据脚端口控制  TRUE为高  FALSE为低
	void (*clk_set)(bit_enum s);    //时钟脚端口控制  TRUE为高  FALSE为低

	bit_enum (*io_read)(void);      //返回数据脚状态  TRUE为高  FALSE为低
}
ic4442_driver_need_struct;










//*****************************************************************************
//*****************************************************************************
//-----------             供给外部调用的变量及函数            -----------------
//-----------------------------------------------------------------------------




extern ulong	ic4442_reset_data;    //4442卡复位应答信息




//初始化外部提供的变量及函数
extern void ic4442_data_init(ic4442_driver_need_struct s);




//******************************************************//
//初始化操作  包括端口初始化以及4442卡的复位操作
//函数返回值为复位应答信息值
//******************************************************//
extern ulong ic4442_init(void);




//******************************************************//
//读4442卡数据
//参数comm为操作命令
//参数addr为操作地址
//参数buff为操作数据缓冲区，三条读命令的缓冲区的长度有特别要求
//主存区必须读到第255个字节
//保护存储区必须读完4个字节
//安全存储区必须读完4个字节
//
//该函数无需指定读取长度，由程序根据起始地址自动读完需要读取的剩下的内容
//但要保证缓冲区大小合适，否则如果小于待读取的长度，程序将会崩溃
//
//******************************************************//
extern void ic4442_read_data(ic4442_comm_enum comm, uchar addr, uchar* buff);




//******************************************************//
//向4442卡中写数据
//参数pass为密码
//参数comm为操作命令
//参数addr为操作地址
//参数buff为待写入的数据的缓冲区
//参数lgth为待写入的数据的长度
//函数返回值    TRUE为完成    FALSE为失败
//******************************************************//
extern bit_enum ic4442_write_data(ulong pass, ic4442_comm_enum comm, uchar addr, uchar* buff, uint lgth);






#endif




