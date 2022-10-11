//***********************************************************************//
//***********************************************************************//
//-----------------------------------------------------------------------//
//��д��ģ���ԭ��Ϊÿһ������֮��Ҫʹʱ���߻�ԭΪ�ߵ�ƽ״̬!!!
//  CKP  2015-04-09
//-----------------------------------------------------------------------//
//***********************************************************************//
//***********************************************************************//
#include "ckp_mcu_file.h"
#include "iic_base_driver.h"

static iic_base_driver_need_struct  iic_driver;    //�ⲿ�ṩ�ĺ���������

//ģ���ڲ����ݳ�ʼ��
void iic_base_data_init(iic_base_driver_need_struct s)
{
	iic_driver = s;
}




//***********************************************************************//
//***********************************************************************//


//�������״̬
//TRUEΪ����״̬�����Է�����ʼ�źſ��ƴ��豸
//FALSEΪæ״̬�����ɿ�������
static bit_enum iic_bus_check(void)
{
	bit_enum  ret;

	iic_driver.sda_set(_true_);  //�ͷ�������
	iic_driver.scl_set(_true_);  //�ͷ�ʱ����
	iic_driver.dly_us(2);

	if ((iic_driver.sda_read()) && (iic_driver.scl_read()))
	{
		ret = _true_;  //���߿���
	}
	else
	{
		ret = _false_;   //���߱�ռ��
	}

	return  ret;
}


//������ʼ�ź�
static void iic_bus_start(void)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //����ʱ����

	iic_driver.dly_us(2);
	iic_driver.sda_set(_true_);  //�ͷ�������

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //�ͷ�ʱ����

	iic_driver.dly_us(4);
	iic_driver.sda_set(_false_);  //����������
}


//����ֹͣ�ź�
static void iic_bus_end(void)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //����ʱ����

	iic_driver.dly_us(2);
	iic_driver.sda_set(_false_);  //����������

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //�ͷ�ʱ����

	iic_driver.dly_us(4);
	iic_driver.sda_set(_true_);  //�ͷ�������
}


//����Ӧ���ź�
//����FΪTRUE��ʾӦ��  FALSE��ʾ��Ӧ��
static void iic_bus_ack(bit_enum f)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //����ʱ����

	iic_driver.dly_us(2);

	if (f)
	{
		iic_driver.sda_set(_false_);  //����������
	}
	else
	{
		iic_driver.sda_set(_true_);  //�ͷ�������
	}

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //�ͷ�ʱ����

	iic_driver.dly_us(2);
}


//��ȡӦ���ź�
//����ֵTRUE��ʾ�ӻ�Ӧ��  FALSE��ʾ�ӻ�δӦ��
static bit_enum iic_bus_read_ack(void)
{
	iic_driver.dly_us(4);
	iic_driver.scl_set(_false_);  //����ʱ����

	iic_driver.dly_us(2);
	iic_driver.sda_set(_true_);  //�ͷ�������

	iic_driver.dly_us(4);
	iic_driver.scl_set(_true_);  //�ͷ�ʱ����

	iic_driver.dly_us(2);

	return  (bit_enum)(!(iic_driver.sda_read()));
}


//����һ���ֽ�����
static void iic_bus_write_byte(uchar dat)
{
	uchar  i;

	for (i=0x00; i<8; i++)
	{
		iic_driver.dly_us(4);
		iic_driver.scl_set(_false_);  //����ʱ����

		iic_driver.dly_us(2);

		if (dat & bit_move(7-i))
		{
			iic_driver.sda_set(_true_);  //�ͷ�������
		}
		else
		{
			iic_driver.sda_set(_false_);  //����������
		}

		iic_driver.dly_us(4);
		iic_driver.scl_set(_true_);  //�ͷ�ʱ����

		iic_driver.dly_us(2);
	}
}


//��ȡһ���ֽ�����
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




//���߳�ʼ��
//�������״̬����������������Զ�����
//��������ֵ�������յ�״̬  TRUEΪ��ʼ���ɹ�  FALSEΪ��ʼ��ʧ��
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

		for (j=0x00; j<9; j++) //����9������
		{
			iic_bus_read_ack();
		}
	}

	return  ret;
}




//����������ַ���ȴ�����Ӧ��
//����dev_addrΪ������7λ��ַ
//����dirΪ����������  TRUEΪ������д��  FALSEΪ����������
//��������ֵ  TRUEΪ����Ӧ��  FALSEΪ��
//�ú�������ѯ���200�����ʱ��
static bit_enum iic_device_wait_ack(uchar dev_addr, bit_enum dir)
{
	uint   i;
	uchar  dat;
	bit_enum  ret;

	ret = _false_;
	dat = dev_addr & 0xfe;  //ȡ����7λ��ַ

	if (!dir)
	{
		dat |= 0x01;  //������
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




//���·�����ʼ�źţ�����������ַ
//����dev_addrΪ������7λ��ַ
//����dirΪ����������  TRUEΪ������д��  FALSEΪ����������
//��������ֵ  TRUEΪ����Ӧ��  FALSEΪ��
static bit_enum iic_device_restart(uchar dev_addr, bit_enum dir)
{
	uchar  dat;

	dat = dev_addr & 0xfe;  //ȡ����7λ��ַ

	if (!dir)
	{
		dat |= 0x01;  //������
	}

	iic_bus_start();
	iic_bus_write_byte(dat);

	return  iic_bus_read_ack();
}




//�����������ݲ���
//����sla_addrΪ��������ַ���������λ�İ�λ��ַ
//����addr�ڲ��Ĵ���/�洢����ʼ��ַ
//����d_buff�������Ļ�����
//����lgth�������Ļ���������
//��������ֵ  TRUEΪ�ɹ�  FALSEΪʧ��
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

	iic_bus_write_byte(addr);	    //���ö�ָ��
	if (!iic_bus_read_ack())
	{
		return  _false_;
	}

	if (!iic_device_restart(sla_addr, _false_))  //������
	{
		return  _false_;
	}

	for (n=0x00; n<lgth; n++)
	{
		*(d_buff+n) = iic_bus_read_byte();

		if (n >= (lgth-1))
		{
			iic_bus_ack(_false_);  //��ֹӦ��
		}
		else
		{
			iic_bus_ack(_true_);  //ʹ��Ӧ��
		}
	}

	iic_bus_end();
	return	 _true_;    //�����ɹ�
}




//������д���ݲ���
//����sla_addrΪ��������ַ���������λ�İ�λ��ַ
//����addr�ڲ��Ĵ���/�洢����ʼ��ַ
//����s_buff�������Ļ�����
//����lgth�������Ļ���������
//��������ֵ  TRUEΪ�ɹ�  FALSEΪʧ��
bit_enum iic_device_write_data(uchar sla_addr, uchar addr, uchar *s_buff, uchar lgth)
{
	uchar  i;

	if (!iic_device_wait_ack(sla_addr, _true_))
	{
		return  _false_;
	}

	iic_bus_write_byte(addr);    //����дָ��
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
	return  _true_;    //�����ɹ�
}




