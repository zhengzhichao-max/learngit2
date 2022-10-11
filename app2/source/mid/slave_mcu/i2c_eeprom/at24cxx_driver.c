#include "ckp_mcu_file.h"
#include "iic_base_driver.h"
#include "at24cxx_driver.h"








//at24cxx�����ݲ���
//����sla_addrΪ��������ַ���������λ�İ�λ��ַ
//����addr�ڲ��Ĵ���/�洢����ʼ��ַ
//����d_buff�������Ļ�����
//����lgth�������Ļ���������
//��������ֵ  TRUEΪ�ɹ�  FALSEΪʧ��
bit_enum at24cxx_read_data(uchar sla_addr, uchar addr, uchar *d_buff, uchar lgth)
{
	return iic_device_read_data(sla_addr, addr, d_buff, lgth);
}




//at24cxxд���ݲ���
//����sla_addrΪ��������ַ���������λ�İ�λ��ַ
//����addr�ڲ��Ĵ���/�洢����ʼ��ַ
//����s_buff�������Ļ�����
//����lgth�������Ļ���������
//��������ֵ  TRUEΪ�ɹ�  FALSEΪʧ��
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
		i = (at24cxx_write_page-n);  //���㱾ҳ��д�������ֽ���

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

	return  _true_;    //�����ɹ�
}




