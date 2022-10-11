#ifndef  __at24cxx_driver_h__
#define  __at24cxx_driver_h__










#define    at24cxx_write_page        ((uchar)8)    //写入页的大小




//*****************************************************************************
//*****************************************************************************
//-----------             供给外部调用的变量及函数            -----------------
//-----------------------------------------------------------------------------


//at24cxx读数据操作
//参数sla_addr为从器件地址，忽略最低位的八位地址
//参数addr内部寄存器/存储器起始地址
//参数d_buff待操作的缓冲区
//参数lgth待操作的缓冲区长度
//函数返回值  TRUE为成功  FALSE为失败
extern bit_enum at24cxx_read_data(uchar sla_addr, uchar addr, uchar *d_buff, uchar lgth);


//at24cxx写数据操作
//参数sla_addr为从器件地址，忽略最低位的八位地址
//参数addr内部寄存器/存储器起始地址
//参数s_buff待操作的缓冲区
//参数lgth待操作的缓冲区长度
//函数返回值  TRUE为成功  FALSE为失败
extern bit_enum at24cxx_write_data(uchar sla_addr, uchar addr, uchar *s_buff, uchar lgth);






#endif




