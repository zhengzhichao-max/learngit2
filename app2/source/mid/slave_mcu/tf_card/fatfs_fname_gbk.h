#ifndef  __fatfs_fname_gbk_h__
#define  __fatfs_fname_gbk_h__




#include "ckp_mcu_file.h"








typedef struct
{
    uint (*gbk_unicode)(bit_enum typ, uchar* d_buff, uchar* s_buff, uint lgth);
	//GBK编码与UNICODE码转换函数
    //注意1:GBK编码可与ASCII码混排输入，而UNICODE则不能
    //注意2:输入的数据源为大端模式
    //参数typ表示  TRUE表示GBK转成UNI  FALSE表示UNI转成GBK
    //函数返回值表示转换之后的数据长度
}
fatfs_gbk_struct;






extern void fatfs_fname_gbk_init(fatfs_gbk_struct src);












#endif




