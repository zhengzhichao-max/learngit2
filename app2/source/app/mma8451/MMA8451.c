/*************************************************************************************************************
 * 文件名:			MMA8452Q.c
 * 功能:			MMA8452Q驱动
 * 作者:			
 * 创建时间:		2018-04-09
 * 最后修改时间:	2018-04-09
 * 详细:			MMA8452Q三轴加速度传感器
					依赖SoftwareIIC
*************************************************************************************************************/
#include "include_all.h"

#if (0)
u8 IIC_addr;
GRAY_XYZ Acce_XYZ;

#define PI 3.1415926535898

/*************************************************************************************************************************
*函数        	:	bool MMA8452Q_Init(u8 I2cAddr, u8 SlaveAddr)
*功能        	:	MMA8452Q初始化
*参数        	:	pHandle:句柄；SlaveAddr：芯片IIC地址
*返回        	:	TRUE:初始化成功；FALSE:初始化失败
*依赖			: 	底层宏定义
*作者       	:	
*时间     		:	2018-04-09
*最后修改时间	:	2018-04-09
*说明        	:	
*************************************************************************************************************************/
bool MMA8452Q_Init(void) {
    u8 temp;

    temp = 0x01;
    IIC_Write(MMA8541, 0x2A, &temp, 0x01);
    temp = 0x02;
    IIC_Write(MMA8541, 0x2B, &temp, 0x02);

    IIC_Read(MMA8541, 0x0D, &temp, 1); //读取器件ID
    if (temp != 0x2A) {                //ID不对
        logd("初始化失败，ID错误：%#02x", temp);
        return false;
    }

    logd("ID：0x%02X", temp);

    return true;
}

/*************************************************************************************************************************
*函数        	:	bool MMA8452Q_ReadAcceleration(u8 I2cAddr, s16 *pXa,s16 *pYa, s16 *pZa)
*功能        	:	MMA8452Q 读取三轴加速度
*参数        	:	pHandle:句柄；pXa：返回X轴加速度；pYa：返回Y轴加速度；pZa：返回Z轴加速度
*返回        	:	TRUE:成功；FALSE:失败
*依赖			: 	底层宏定义
*作者       	:	
*时间     		:	2018-04-09
*最后修改时间	:	2018-04-09
*说明        	:	
*************************************************************************************************************************/
bool MMA8452Q_ReadAcceleration(u8 ic, s16 *pXa, s16 *pYa, s16 *pZa) {
    u8 buff[6];
    s16 temp;
    u8 dat = 0;
    IIC_Read(MMA8541, 0x0D, &dat, 1); //读取器件ID
    if (dat != 0x2A)                  //ID不对
    {
        return false;
    }
    IIC_Read(MMA8541, 1, buff, 6); //读取数据
    temp = buff[0];
    temp <<= 4;
    temp |= buff[1] >> 4;
    if (buff[0] & 0x80) //负数
    {
        temp |= 0xF000;
    }
    *pXa = temp; //X轴

    temp = buff[2];
    temp <<= 4;
    temp |= buff[3] >> 4;
    if (buff[2] & 0x80) //负数
    {
        temp |= 0xF000;
    }
    *pYa = temp; //Y轴

    temp = buff[4];
    temp <<= 4;
    temp |= buff[5] >> 4;
    if (buff[4] & 0x80) //负数
    {
        temp |= 0xF000;
    }
    *pZa = temp; //Z轴

    return true;
}

/*************************************************************************************************************************
*函数        	:	void ADXL362_CalAngle(s16 Xa,s16 Ya, s16 Za, float *pAngleX, float *pAngleY, float *pAngleZ)
*功能        	:	ADXL362 通过加速度计算角度信息
*参数        	:	Xa：X轴加速度；Ya：Y轴加速度；Za：Z轴加速度，pAngleX:X方向倾角；pAngleY：Y方向倾角；pAngleZ：Z方向倾角
*返回        	:	无
*依赖			: 	底层宏定义
*作者       	:	
*时间     		:	2016-04-06
*最后修改时间	:	2018-03-13
*说明        	:	需要进行浮点，反正切运算
*************************************************************************************************************************/
void MMA8452Q_CalAngle(s16 Xa, s16 Ya, s16 Za, float *pAngleX, float *pAngleY, float *pAngleZ) {
    double A;

    //X方向
    A = (double)Ya * Ya + (double)Za * Za;
    A = sqrt(A);
    A = (double)Xa / A;
    A = atan(A);
    A = A * 180 / PI;
    if (Za < 0) { //将坐标转换为±180度
        if (A < 0)
            A = -90 - (A + 90);
        else
            A = 90 + (90 - A);
    }
    if (A < 0) { //将坐标转换为360度
        A = fabs(A);
        A = 180 + 180 - A;
    }
    *pAngleX = A;

    //Y方向
    A = (double)Xa * Xa + (double)Za * Za;
    A = sqrt(A);
    A = (double)Ya / A;
    A = atan(A);
    A = A * 180 / PI;
    if (Za < 0) { //将坐标转换为±180度
        if (A < 0)
            A = -90 - (A + 90);
        else
            A = 90 + (90 - A);
    }
    if (A < 0) { //将坐标转换为360度
        A = fabs(A);
        A = 180 + 180 - A;
    }
    *pAngleY = A;

    //Z方向
    A = (double)Xa * Xa + (double)Ya * Ya;
    A = sqrt(A);
    A = (double)A / abs(Za);
    A = atan(A);
    A = A * 180 / PI;
    logd("\tXA:%d\tYA:%d\tZA:%d", Xa, Ya, Za);
    /*if(Za<0)//将坐标转换为±180度
	{
		if(A <0)A=-90-(A+90);
		else A=90+(90-A);
	}
	if(A <0)	//将坐标转换为360度
	{
		A=fabs(A);
		A = 180+180-A;
	}*/
    *pAngleZ = A;
}

/*************************************************************************************************************************
*函数        	:	bool MMA8452Q_GetZAxisAngle(u8 I2cAddr,s16 AcceBuff[3], float *pAngleZ)
*功能        	:	MMA8452Q 获取Z轴倾角
*参数        	:	pHandle:句柄；AcceBuff:3个轴的加速度；pAngleZ：Y方向倾角
*返回        	:	TRUE:成功；FALSE:失败
*依赖			: 	底层宏定义
*作者       	:	
*时间     		:	2018-04-09
*最后修改时间	:	2018-04-09
*说明        	:	
*************************************************************************************************************************/
bool MMA8452Q_GetZAxisAngle(GRAY_XYZ *xyz) {
    double fx, fy, fz;
    double A;
    s16 Xa, Ya, Za;

    if (MMA8452Q_ReadAcceleration(MMA8541, &Xa, &Ya, &Za) == false) return false; //ADXL362 读取加速度数据
    //uart_printf("Xa:%d \tYa:%d \tZa:%d \r\n",Xa,Ya,Za);

    fx = Xa;
    fx *= 10.0 / 1024;
    fy = Ya;
    fy *= 10.0 / 1024;
    fz = Za;
    fz *= 10.0 / 1024;

    //logd("\tfx：%.04f\tfy：%.04f\tfz：%.04f",fx,fy,fz);

    //Z方向
    A = fx * fx + fy * fy;
    A = sqrt(A);
    A = (double)A / fz;
    A = atan(A);
    A = A * 180 / PI;

    xyz->angle = A;
    //logd("=======角度：%.04f",*pAngleZ);
    xyz->x = Xa;
    xyz->y = Ya;
    xyz->z = Za;

    return true;
}

void Mma8452Q_Read(void) {
    static u32 rtm = 0;

    if (_pastn(rtm) > 30000) {
        rtm = jiffies;
        if (MMA8452Q_GetZAxisAngle(&Acce_XYZ)) //ADXL362 获取角度信息
        {
            //角度读取失败
            //logd("^^^ XX=%5d, YY=%5d, ZZ=%5d, Angle=%9.04f ^^^",Acce_XYZ.x,Acce_XYZ.y,Acce_XYZ.z,Acce_XYZ.angle);
        }
    }
}

#if 0
    g_SysFlag.ADXL362_Status = MMA8452Q_Init(&g_SysFlag.MMA8452Q_Handle, 0x1C<<1);
	if(g_SysFlag.ADXL362_Status == false)
	{
		DEBUG("MMA8452Q初始化失败！\r\n");
	}

#endif
#endif

