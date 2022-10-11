#ifndef  __fatfs_fname_gbk_h__
#define  __fatfs_fname_gbk_h__




#include "ckp_mcu_file.h"








typedef struct
{
    uint (*gbk_unicode)(bit_enum typ, uchar* d_buff, uchar* s_buff, uint lgth);
	//GBK������UNICODE��ת������
    //ע��1:GBK�������ASCII��������룬��UNICODE����
    //ע��2:���������ԴΪ���ģʽ
    //����typ��ʾ  TRUE��ʾGBKת��UNI  FALSE��ʾUNIת��GBK
    //��������ֵ��ʾת��֮������ݳ���
}
fatfs_gbk_struct;






extern void fatfs_fname_gbk_init(fatfs_gbk_struct src);












#endif




