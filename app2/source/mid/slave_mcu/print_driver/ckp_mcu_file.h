//***********************************************************************************************//
//***********************************************************************************************//
//-----                             公共程序模块H文件                                        ----//
//----
//----  CKP
//----  2013-09-14
//***********************************************************************************************//
//***********************************************************************************************//

#ifndef __ckp_mcu_file_h__
#define __ckp_mcu_file_h__


//****************************************************************************//
//****************************************************************************//

//----------------------------------------------------------------------------//
//------                        自定义数据类型                   -------------//
typedef enum
{
	_false_ = 0,
	_true_  = 1
}
bit_enum;       //位状态

typedef  unsigned char        uchar;          //无符号字符型
typedef  unsigned short int   uint;           //无符号整型
typedef  unsigned long int    ulong;          //无符号长整型

typedef  const uchar     cuchar;
typedef  const uint      cuint;
typedef  const ulong     culong;

typedef  volatile uchar  vuchar;
typedef  volatile uint   vuint;
typedef  volatile ulong  vulong;

//----------------//

typedef  uchar      u_8;      //无符号字符型
typedef  uint       u_16;    //无符号整型
typedef  ulong      u_32;    //无符号长整型

typedef  signed char        s_8;      //有符号字符型
typedef  signed short int   s_16;    //有符号整型
typedef  signed long int    s_32;    //有符号长整型

//-----------------------------------------------------------------------------//




//----------------------------------------------------------------------------//
//------                          常用宏定义                     -------------//

#define   setbit(d_byte,d_bit)    (d_byte|=(1<<d_bit))          //置位
#define   clrbit(d_byte,d_bit)    (d_byte&=(~(1<<d_bit)))      //清零
#define   testbit(d_byte,d_bit)   (d_byte&(1<<d_bit))           //测试
#define   cplbit(d_byte,d_bit)    (d_byte^=(1<<d_bit))          //取反

#define   sub_u8(dat, sub)      ((uchar)(dat-sub))
#define   sub_u16(dat, sub)     ((uint)(dat-sub))
#define   sub_u32(dat, sub)     ((ulong)(dat-sub))    //求差值

#define   bit_move(n)    (((ulong)0x01)<<(n))        //移位定义


//-----------------------------------------------------------------------------//

//****************************************************************************//
//****************************************************************************//














#endif

//---                                   文件结束                                              ---//
//***********************************************************************************************//
//***********************************************************************************************//

