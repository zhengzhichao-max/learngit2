#ifndef  __at24cxx_driver_h__
#define  __at24cxx_driver_h__










#define    at24cxx_write_page        ((uchar)8)    //д��ҳ�Ĵ�С




//*****************************************************************************
//*****************************************************************************
//-----------             �����ⲿ���õı���������            -----------------
//-----------------------------------------------------------------------------


//at24cxx�����ݲ���
//����sla_addrΪ��������ַ���������λ�İ�λ��ַ
//����addr�ڲ��Ĵ���/�洢����ʼ��ַ
//����d_buff�������Ļ�����
//����lgth�������Ļ���������
//��������ֵ  TRUEΪ�ɹ�  FALSEΪʧ��
extern bit_enum at24cxx_read_data(uchar sla_addr, uchar addr, uchar *d_buff, uchar lgth);


//at24cxxд���ݲ���
//����sla_addrΪ��������ַ���������λ�İ�λ��ַ
//����addr�ڲ��Ĵ���/�洢����ʼ��ַ
//����s_buff�������Ļ�����
//����lgth�������Ļ���������
//��������ֵ  TRUEΪ�ɹ�  FALSEΪʧ��
extern bit_enum at24cxx_write_data(uchar sla_addr, uchar addr, uchar *s_buff, uchar lgth);






#endif




