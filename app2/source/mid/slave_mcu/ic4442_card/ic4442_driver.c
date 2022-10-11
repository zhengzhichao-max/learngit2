#include "ckp_mcu_file.h"
#include "ic4442_driver.h"



static ic4442_driver_need_struct  ic4442_driver;    //�ⲿ�ṩ�ı���������
ulong	ic4442_reset_data;    //4442����λӦ����Ϣ

void ic4442_data_init(ic4442_driver_need_struct s)
{
	ic4442_driver = s;
}


//*****************************************************************************************//
//---------------------------        �ײ���������        ----------------------------------//
//*****************************************************************************************//


//*************************************//
//����һ��ʱ���ź�
//*************************************//
static void send_one_clk(void)
{
	ic4442_driver.clk_set(_true_);   //����ʱ��
	ic4442_driver.dly_us(12);
	ic4442_driver.clk_set(_false_);   //�ָ�ʱ��
	ic4442_driver.dly_us(12);
}


//*************************************//
//���͸�λ�źŲ���ȡ��λӦ����
//��������ֵΪ��λӦ����Ϣֵ
//*************************************//
static ulong ic4442_reset(void)
{
	uchar  n;
	ulong  ret;

	ret = 0x00;

	ic4442_driver.io_set(_true_);    //�ͷ����ݶ˿�
	ic4442_driver.clk_set(_false_);   //����ʱ���ź�Ϊ�͵�ƽ

	ic4442_driver.rst_set(_true_);
	ic4442_driver.dly_us(20);
	send_one_clk();
	ic4442_driver.rst_set(_false_);   //��λ���̽���
	ic4442_driver.dly_us(5);

	for (n=0; n<32; n++)
	{
		if (ic4442_driver.io_read())
		{
			ret |= (bit_move(n));  //��������λ,��λ��ǰ
		}

		send_one_clk();
	}

	return  ret;
}


//*************************************//
//�������ʼ�ź�
//*************************************//
static void ic4442_start(void)
{
	ic4442_driver.io_set(_true_);    //�ͷ����ݶ˿�
	ic4442_driver.dly_us(5);
	ic4442_driver.clk_set(_true_);   //����ʱ��
	ic4442_driver.dly_us(8);
	ic4442_driver.io_set(_false_);
	ic4442_driver.dly_us(8);
	ic4442_driver.clk_set(_false_);   //�ָ�ʱ��
	ic4442_driver.dly_us(10);
}


//*************************************//
//������������ź�
//*************************************//
static void ic4442_end(void)
{
	ic4442_driver.io_set(_false_);
	ic4442_driver.dly_us(5);
	ic4442_driver.clk_set(_true_);   //����ʱ��
	ic4442_driver.dly_us(8);
	ic4442_driver.io_set(_true_);    //�ͷ����ݶ˿�
	ic4442_driver.dly_us(8);
	ic4442_driver.clk_set(_false_);   //�ָ�ʱ��
	ic4442_driver.dly_us(10);
}


//*************************************//
//�ȴ�����Ĵ���
//��������ֵ    TRUEΪ���    FALSEΪʧ��
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
//����һ���ֽڵ�����
//����datΪ�����͵��ֽ�
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

		ic4442_driver.clk_set(_true_);   //����ʱ��
		ic4442_driver.dly_us(12);
		ic4442_driver.clk_set(_false_);   //�ָ�ʱ��
		ic4442_driver.dly_us(6);
	}
	ic4442_driver.dly_us(5);
}


//*************************************//
//����һ���ֽڵ�����
//��������ֵΪ���յ���һ���ֽڵ�����
//*************************************//
static uchar ic4442_rec_byte(void)
{
	uchar  i, dat;

	dat = 0x00;
	ic4442_driver.io_set(_true_);    //�ͷ����ݶ˿�
	ic4442_driver.dly_us(1);

	for (i=0x00; i<8; i++)
	{
		if (ic4442_driver.io_read())
		{
			dat |= (bit_move(i));  //��������λ,��λ��ǰ
		}

		send_one_clk();
	}

	return  dat;
}


//*************************************//
//����һ������
//����commΪ��������
//����addrΪ������ַ
//����datΪ������ֵ
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
//��������
//����passΪ����
//��������ֵ    TRUEΪ���    FALSEΪʧ��
//*************************************//
static bit_enum ic4442_unlock(ulong pass)
{
	uchar  buff[4];
	uchar  i;

	ic4442_read_data(read_secu_mem, 0x00, buff);
	if ((buff[0]&0x07) == 0x00) //��Ƭ����
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
		ic4442_send_command(comp_ver_data, (i+1), (*(((uchar*)(&pass))+i)));  //�˶�����
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
	if ((buff[0]&0x07) != 0x07) //��Ƭ����
	{
		return  _false_;
	}

	return  _true_;
}


//*****************************************************************************************//
//*****************************************************************************************//




//******************************************************//
//��ʼ������  �����˿ڳ�ʼ���Լ�4442���ĸ�λ����
//��������ֵΪ��λӦ����Ϣֵ
//******************************************************//
ulong ic4442_init(void)
{
	ic4442_driver.rst_set(_false_);    //��λ������Ϊ�͵�ƽ
	ic4442_driver.dly_us(50);
	//------------------//

	//----IC����ʼ��----//
	ic4442_reset_data = ic4442_reset();
	//------------------//

	return  ic4442_reset_data;
}




//******************************************************//
//��4442������
//����commΪ��������
//����addrΪ������ַ
//����buffΪ�������ݻ�����������������Ļ������ĳ������ر�Ҫ��
//���������������255���ֽ�
//�����洢���������4���ֽ�
//��ȫ�洢���������4���ֽ�
//******************************************************//
void ic4442_read_data(ic4442_comm_enum comm, uchar addr, uchar* buff)
{
	uint  i, n;

	if (!((comm == read_main_mem) || (comm == read_prot_mem) || (comm == read_secu_mem)))
	{
		return;  //�˶������������ֱ���˳�
	}

	if (comm == read_main_mem)
	{
		n = (256-addr);
	}
	else
	{
		n = 4;         //�ܹ���4���ֽ�
	}

	ic4442_send_command(comm, addr, 0x00);
	for (i=0x00; i<n; i++)
	{
		*(buff+i) = ic4442_rec_byte();
	}
}




//******************************************************//
//��4442����д����
//����passΪ����
//����commΪ��������
//����addrΪ������ַ
//����buffΪ��д������ݵĻ�����
//����lgthΪ��д������ݵĳ���
//��������ֵ    TRUEΪ���    FALSEΪʧ��
//******************************************************//
bit_enum ic4442_write_data(ulong pass, ic4442_comm_enum comm, uchar addr, uchar* buff, uint lgth)
{
	uint  i;

	if (ic4442_unlock(pass) == _false_)
		return  _false_;

	if (!((comm == write_main_mem) || (comm == write_prot_mem) || (comm == write_secu_mem)))
	{
		return  _false_;  //�˶������������ֱ���˳�
	}

	if (comm == write_main_mem)
	{
		if ((addr+lgth) > 256) //������Χ
		{
			return  _false_;
		}
	}
	else if (comm == write_prot_mem)
	{
		if ((addr+lgth) > 32) //������Χ
		{
			return  _false_;
		}
	}
	else
	{
		if ((addr+lgth) > 4) //������Χ
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




