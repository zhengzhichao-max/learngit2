/*************************************************************************************************************
 * 文件�?:			MMA8452Q.c
 * 功能:			MMA8452Q驱动
 * 作�?:			cp1300@139.com
 * 创建时间:		2018-04-09
 * 最后修改时�?:	2018-04-09
 * 详细:			MMA8452Q三轴加速度传感�?
*************************************************************************************************************/
#ifndef _MMA8452Q_H_
#define _MMA8452Q_H_
 
 #if(0)
#define MMA8541    	0x38

typedef struct
{
	float angle;	//角度
	s16	x;		//0.001g
	s16 y;		//0.001g
	s16 z;		//0.001g
}GRAY_XYZ;


extern 	u8 IIC_addr;
extern  GRAY_XYZ	Acce_XYZ;

 
extern bool MMA8452Q_Init(void);											//MMA8452Q初�?�?
extern void Mma8452Q_Read(void);				//MMA8452Q 获取Z轴�?度信�?
#endif
 
 
#endif /*_MMA8452Q_H_*/


