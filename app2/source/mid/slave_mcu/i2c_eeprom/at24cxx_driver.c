#include "ckp_mcu_file.h"
#include "iic_base_driver.h"
#include "at24cxx_driver.h"








//at24cxx读数据操作
//参数sla_addr为从器件地址，忽略最低位的八位地址
//参数addr内部寄存器/存储器起始地址
//参数d_buff待操作的缓冲区
//参数lgth待操作的缓冲区长度
//函数返回值  TRUE为成功  FALSE为失败
bit_enum at24cxx_read_data(uchar sla_addr, uchar addr, uchar *d_buff, uchar lgth)
{
	return iic_device_read_data(sla_addr, addr, d_buff, lgth);
}




//at24cxx写数据操作
//参数sla_addr为从器件地址，忽略最低位的八位地址
//参数addr内部寄存器/存储器起始地址
//参数s_buff待操作的缓冲区
//参数lgth待操作的缓冲区长度
//函数返回值  TRUE为成功  FALSE为失败
bit_enum at24cxx_write_data(uchar sla_addr, uchar addr, uchar *s_buff, uchar lgth)
{
	uchar  n, i;

	if (lgth == 0x00)
	{
		return  _true_;
	}

	n = (addr%at24cxx_write_page);
	if (n != 0x00)
	{
		i = (at24cxx_write_page-n);  //计算本页能写入的最大字节数

		if (i > lgth)
		{
			i = lgth;
		}

		if (!iic_device_write_data(sla_addr, addr, s_buff, i))
		{
			return  _false_;
		}

		addr += i;
		s_buff += i;
		lgth -= i;
	}

	while (lgth)
	{
		if (lgth > at24cxx_write_page)
		{
			i = at24cxx_write_page;
		}
		else
		{
			i = lgth;
		}

		if (!iic_device_write_data(sla_addr, addr, s_buff, i))
		{
			return  _false_;
		}

		addr += i;
		s_buff += i;
		lgth -= i;
	}

	return  _true_;    //操作成功
}




