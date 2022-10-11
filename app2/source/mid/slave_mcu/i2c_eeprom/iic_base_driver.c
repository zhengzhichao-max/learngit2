//***********************************************************************//
//***********************************************************************//
//-----------------------------------------------------------------------//
//编写该模块的原则为每一步操作之后都要使时钟线还原为高电平状态!!!
//  CKP  2015-04-09
//-----------------------------------------------------------------------//
//***********************************************************************//
//***********************************************************************//
#include "ckp_mcu_file.h"
#include "iic_base_driver.h"

static iic_base_driver_need_struct  iic_driver;    //外部提供的函数及变量

//模块内部数据初始化
void iic_base_data_init(iic_base_driver_need_struct s)
{
	iic_driver = s;
}




//***********************************************************************//
//***********************************************************************//


//检查总线状态
//TRUE为空闲状态，可以发起起始信号控制从设备
//FALSE为忙状态，不可控制总线
static bit_enum iic_bus_check(void)
{
	bit_enum  ret;

	iic_driver.sda_set(_true_);  //释放数据线
	iic_driver.scl_set(_true_);  //释放时钟线
	iic_driver.dly_us(2);

	if ((iic_driver.sda_read()) && (iic_driver.scl_read()))
	{
		ret = _true_;  //总线空闲
	}
	else
	{
		ret = _false_;   //总线被占用
	}

	return  ret;
}


//发起起始信号
static void iic_bus_start(void)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //拉低时钟线

	iic_driver.dly_us(2);
	iic_driver.sda_set(_true_);  //释放数据线

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //释放时钟线

	iic_driver.dly_us(4);
	iic_driver.sda_set(_false_);  //拉低数据线
}


//发起停止信号
static void iic_bus_end(void)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //拉低时钟线

	iic_driver.dly_us(2);
	iic_driver.sda_set(_false_);  //拉低数据线

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //释放时钟线

	iic_driver.dly_us(4);
	iic_driver.sda_set(_true_);  //释放数据线
}


//发送应答信号
//参数F为TRUE表示应答  FALSE表示不应答
static void iic_bus_ack(bit_enum f)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //拉低时钟线

	iic_driver.dly_us(2);

	if (f)
	{
		iic_driver.sda_set(_false_);  //拉低数据线
	}
	else
	{
		iic_driver.sda_set(_true_);  //释放数据线
	}

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //释放时钟线

	iic_driver.dly_us(2);
}


//读取应答信号
//返回值TRUE表示从机应答  FALSE表示从机未应答
static bit_enum iic_bus_read_ack(void)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //拉低时钟线

	iic_driver.dly_us(2);
	iic_driver.sda_set(_true_);  //释放数据线

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //释放时钟线

	iic_driver.dly_us(2);

	return  (bit_enum)(!(iic_driver.sda_read()));
}


//发送一个字节数据
static void iic_bus_write_byte(uchar dat)
{
	uchar  i;

	for (i=0x00; i<8; i++)
	{
		iic_driver.dly_us(4);
		iic_driver.scl_set(_false_);  //拉低时钟线

		iic_driver.dly_us(2);

		if (dat & bit_move(7-i))
		{
			iic_driver.sda_set(_true_);  //释放数据线
		}
		else
		{
			iic_driver.sda_set(_false_);  //拉低数据线
		}

		iic_driver.dly_us(4);
		iic_driver.scl_set(_true_);  //释放时钟线

		iic_driver.dly_us(2);
	}
}


//读取一个字节数据
static uchar iic_bus_read_byte(void)
{
	uchar  i, ret;

	ret = 0x00;

	for (i=0x00; i<8; i++)
	{
		if (!iic_bus_read_ack())
		{
			ret |= bit_move(7-i);
		}
	}

	return  ret;
}


//***********************************************************************//
//***********************************************************************//




//总线初始化
//检测总线状态，如果总线锁死则自动解锁
//函数返回值返回最终的状态  TRUE为初始化成功  FALSE为初始化失败
bit_enum iic_bus_init(void)
{
	uchar  i, j;
	bit_enum  ret;

	ret = _false_;

	for (i=0x00; i<20; i++)
	{
		if (iic_bus_check())
		{
			ret = _true_;
			break;
		}

		for (j=0x00; j<9; j++) //发送9个脉冲
		{
			iic_bus_read_ack();
		}
	}

	return  ret;
}




//发送器件地址，等待器件应答
//参数dev_addr为器件的7位地址
//参数dir为数据流方向  TRUE为向器件写入  FALSE为从器件读出
//函数返回值  TRUE为器件应答  FALSE为无
//该函数会轮询最多200毫秒的时间
static bit_enum iic_device_wait_ack(uchar dev_addr, bit_enum dir)
{
	uint   i;
	uchar  dat;
	bit_enum  ret;

	ret = _false_;
	dat = dev_addr & 0xfe;  //取出高7位地址

	if (!dir)
	{
		dat |= 0x01;  //读器件
	}

	for (i=0x00; i<600; i++)
	{
		iic_bus_start();
		iic_bus_write_byte(dat);

		if (iic_bus_read_ack())
		{
			ret = _true_;
			break;
		}

		iic_driver.dly_us(230);
	}

	return  ret;
}




//重新发送起始信号，包含器件地址
//参数dev_addr为器件的7位地址
//参数dir为数据流方向  TRUE为向器件写入  FALSE为从器件读出
//函数返回值  TRUE为器件应答  FALSE为无
static bit_enum iic_device_restart(uchar dev_addr, bit_enum dir)
{
	uchar  dat;

	dat = dev_addr & 0xfe;  //取出高7位地址

	if (!dir)
	{
		dat |= 0x01;  //读器件
	}

	iic_bus_start();
	iic_bus_write_byte(dat);

	return  iic_bus_read_ack();
}




//从器件读数据操作
//参数sla_addr为从器件地址，忽略最低位的八位地址
//参数addr内部寄存器/存储器起始地址
//参数d_buff待操作的缓冲区
//参数lgth待操作的缓冲区长度
//函数返回值  TRUE为成功  FALSE为失败
bit_enum iic_device_read_data(uchar sla_addr, uchar addr, uchar *d_buff, uchar lgth)
{
	uchar 	 n;

	if (lgth == 0x00)
	{
		return  _true_;
	}

	if (!iic_device_wait_ack(sla_addr, _true_))
	{
		return  _false_;
	}

	iic_bus_write_byte(addr);	    //设置读指针
	if (!iic_bus_read_ack())
	{
		return  _false_;
	}

	if (!iic_device_restart(sla_addr, _false_))  //重启动
	{
		return  _false_;
	}

	for (n=0x00; n<lgth; n++)
	{
		*(d_buff+n) = iic_bus_read_byte();

		if (n >= (lgth-1))
		{
			iic_bus_ack(_false_);  //禁止应答
		}
		else
		{
			iic_bus_ack(_true_);  //使能应答
		}
	}

	iic_bus_end();
	return	 _true_;    //操作成功
}




//从器件写数据操作
//参数sla_addr为从器件地址，忽略最低位的八位地址
//参数addr内部寄存器/存储器起始地址
//参数s_buff待操作的缓冲区
//参数lgth待操作的缓冲区长度
//函数返回值  TRUE为成功  FALSE为失败
bit_enum iic_device_write_data(uchar sla_addr, uchar addr, uchar *s_buff, uchar lgth)
{
	uchar  i;

	if (!iic_device_wait_ack(sla_addr, _true_))
	{
		return  _false_;
	}

	iic_bus_write_byte(addr);    //设置写指针
	if (!iic_bus_read_ack())
	{
		return  _false_;
	}

	for (i=0x00; i<lgth; i++)
	{
		iic_bus_write_byte( *(s_buff+i));
		if (!iic_bus_read_ack())
		{
			return  _false_;
		}
	}

	iic_bus_end();
	return  _true_;    //操作成功
}




