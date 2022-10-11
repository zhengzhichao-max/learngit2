#include "ckp_mcu_file.h"
#include "ic4442_driver.h"



static ic4442_driver_need_struct  ic4442_driver;    //外部提供的变量及函数
ulong	ic4442_reset_data;    //4442卡复位应答信息

void ic4442_data_init(ic4442_driver_need_struct s)
{
	ic4442_driver = s;
}


//*****************************************************************************************//
//---------------------------        底层驱动函数        ----------------------------------//
//*****************************************************************************************//


//*************************************//
//发送一个时钟信号
//*************************************//
static void send_one_clk(void)
{
	ic4442_driver.clk_set(_true_);   //驱动时钟
	ic4442_driver.dly_us(12);
	ic4442_driver.clk_set(_false_);   //恢复时钟
	ic4442_driver.dly_us(12);
}


//*************************************//
//发送复位信号并获取复位应答字
//函数返回值为复位应答信息值
//*************************************//
static ulong ic4442_reset(void)
{
	uchar  n;
	ulong  ret;

	ret = 0x00;

	ic4442_driver.io_set(_true_);    //释放数据端口
	ic4442_driver.clk_set(_false_);   //设置时钟信号为低电平

	ic4442_driver.rst_set(_true_);
	ic4442_driver.dly_us(20);
	send_one_clk();
	ic4442_driver.rst_set(_false_);   //复位过程结束
	ic4442_driver.dly_us(5);

	for (n=0; n<32; n++)
	{
		if (ic4442_driver.io_read())
		{
			ret |= (bit_move(n));  //保存数据位,低位在前
		}

		send_one_clk();
	}

	return  ret;
}


//*************************************//
//发送命令开始信号
//*************************************//
static void ic4442_start(void)
{
	ic4442_driver.io_set(_true_);    //释放数据端口
	ic4442_driver.dly_us(5);
	ic4442_driver.clk_set(_true_);   //驱动时钟
	ic4442_driver.dly_us(8);
	ic4442_driver.io_set(_false_);
	ic4442_driver.dly_us(8);
	ic4442_driver.clk_set(_false_);   //恢复时钟
	ic4442_driver.dly_us(10);
}


//*************************************//
//发送命令结束信号
//*************************************//
static void ic4442_end(void)
{
	ic4442_driver.io_set(_false_);
	ic4442_driver.dly_us(5);
	ic4442_driver.clk_set(_true_);   //驱动时钟
	ic4442_driver.dly_us(8);
	ic4442_driver.io_set(_true_);    //释放数据端口
	ic4442_driver.dly_us(8);
	ic4442_driver.clk_set(_false_);   //恢复时钟
	ic4442_driver.dly_us(10);
}


//*************************************//
//等待命令的处理
//函数返回值    TRUE为完成    FALSE为失败
//*************************************//
static bit_enum ic4442_wait_busy(void)
{
	uint  n;

	ic4442_driver.dly_us(5);

	for (n=0x00; n<ic4442_wait_busy_max_clk; n++)
	{
		send_one_clk();

		if (ic4442_driver.io_read())
		{
			return  _true_;
		}
	}

	return  _false_;
}


//*************************************//
//发送一个字节的数据
//参数dat为待发送的字节
//*************************************//
static void ic4442_send_byte(uchar dat)
{
	uchar  i;

	ic4442_driver.dly_us(5);
	
	for (i=0x00; i<8; i++)
	{
		if (dat&(bit_move(i)))
			ic4442_driver.io_set(_true_);
		else
			ic4442_driver.io_set(_false_);

		ic4442_driver.dly_us(6);

		ic4442_driver.clk_set(_true_);   //驱动时钟
		ic4442_driver.dly_us(12);
		ic4442_driver.clk_set(_false_);   //恢复时钟
		ic4442_driver.dly_us(6);
	}
	ic4442_driver.dly_us(5);
}


//*************************************//
//接收一个字节的数据
//函数返回值为接收到的一个字节的数据
//*************************************//
static uchar ic4442_rec_byte(void)
{
	uchar  i, dat;

	dat = 0x00;
	ic4442_driver.io_set(_true_);    //释放数据端口
	ic4442_driver.dly_us(1);

	for (i=0x00; i<8; i++)
	{
		if (ic4442_driver.io_read())
		{
			dat |= (bit_move(i));  //保存数据位,低位在前
		}

		send_one_clk();
	}

	return  dat;
}


//*************************************//
//发送一条命令
//参数comm为操作命令
//参数addr为操作地址
//参数dat为操作数值
//*************************************//
static void ic4442_send_command(ic4442_comm_enum comm, uchar addr, uchar dat)
{
	ic4442_start();

	ic4442_send_byte(comm);
	ic4442_send_byte(addr);
	ic4442_send_byte(dat);

	ic4442_end();
}

//*************************************//
//解锁操作
//参数pass为密码
//函数返回值    TRUE为完成    FALSE为失败
//*************************************//
static bit_enum ic4442_unlock(ulong pass)
{
	uchar  buff[4];
	uchar  i;

	ic4442_read_data(read_secu_mem, 0x00, buff);
	if ((buff[0]&0x07) == 0x00) //卡片锁死
	{
		return  _false_;
	}

	ic4442_send_command(write_secu_mem, 0x00, 0xfe);
	if (ic4442_wait_busy() == _false_)
	{
		return  _false_;
	}

	for (i=0x00; i<3; i++)
	{
		ic4442_send_command(comp_ver_data, (i+1), (*(((uchar*)(&pass))+i)));  //核对密码
		if (ic4442_wait_busy() == _false_)
		{
			return  _false_;
		}
	}

	ic4442_send_command(write_secu_mem, 0x00, 0xff);
	if (ic4442_wait_busy() == _false_)
	{
		return  _false_;
	}

	ic4442_read_data(read_secu_mem, 0x00, buff);
	if ((buff[0]&0x07) != 0x07) //卡片锁死
	{
		return  _false_;
	}

	return  _true_;
}


//*****************************************************************************************//
//*****************************************************************************************//




//******************************************************//
//初始化操作  包括端口初始化以及4442卡的复位操作
//函数返回值为复位应答信息值
//******************************************************//
ulong ic4442_init(void)
{
	ic4442_driver.rst_set(_false_);    //复位脚设置为低电平
	ic4442_driver.dly_us(50);
	//------------------//

	//----IC卡初始化----//
	ic4442_reset_data = ic4442_reset();
	//------------------//

	return  ic4442_reset_data;
}




//******************************************************//
//读4442卡数据
//参数comm为操作命令
//参数addr为操作地址
//参数buff为操作数据缓冲区，三条读命令的缓冲区的长度有特别要求
//主存区必须读到第255个字节
//保护存储区必须读完4个字节
//安全存储区必须读完4个字节
//******************************************************//
void ic4442_read_data(ic4442_comm_enum comm, uchar addr, uchar* buff)
{
	uint  i, n;

	if (!((comm == read_main_mem) || (comm == read_prot_mem) || (comm == read_secu_mem)))
	{
		return;  //核对命令，不符合则直接退出
	}

	if (comm == read_main_mem)
	{
		n = (256-addr);
	}
	else
	{
		n = 4;         //总共读4个字节
	}

	ic4442_send_command(comm, addr, 0x00);
	for (i=0x00; i<n; i++)
	{
		*(buff+i) = ic4442_rec_byte();
	}
}




//******************************************************//
//向4442卡中写数据
//参数pass为密码
//参数comm为操作命令
//参数addr为操作地址
//参数buff为待写入的数据的缓冲区
//参数lgth为待写入的数据的长度
//函数返回值    TRUE为完成    FALSE为失败
//******************************************************//
bit_enum ic4442_write_data(ulong pass, ic4442_comm_enum comm, uchar addr, uchar* buff, uint lgth)
{
	uint  i;

	if (ic4442_unlock(pass) == _false_)
		return  _false_;

	if (!((comm == write_main_mem) || (comm == write_prot_mem) || (comm == write_secu_mem)))
	{
		return  _false_;  //核对命令，不符合则直接退出
	}

	if (comm == write_main_mem)
	{
		if ((addr+lgth) > 256) //超过范围
		{
			return  _false_;
		}
	}
	else if (comm == write_prot_mem)
	{
		if ((addr+lgth) > 32) //超过范围
		{
			return  _false_;
		}
	}
	else
	{
		if ((addr+lgth) > 4) //超过范围
		{
			return  _false_;
		}
	}

	for (i=0x00; i<lgth; i++)
	{
		ic4442_send_command(comm, (addr+i), (*(buff+i)));
		if (ic4442_wait_busy() == _false_)
		{
			return  _false_;
		}
	}

	return  _true_;
}




