/*************************************************************************************************************
 * �ļ���:			MMA8452Q.c
 * ����:			MMA8452Q����
 * ����:			
 * ����ʱ��:		2018-04-09
 * ����޸�ʱ��:	2018-04-09
 * ��ϸ:			MMA8452Q������ٶȴ�����
					����SoftwareIIC
*************************************************************************************************************/
#include "include_all.h"

#if (0)
u8 IIC_addr;
GRAY_XYZ Acce_XYZ;

#define PI 3.1415926535898

/*************************************************************************************************************************
*����        	:	bool MMA8452Q_Init(u8 I2cAddr, u8 SlaveAddr)
*����        	:	MMA8452Q��ʼ��
*����        	:	pHandle:�����SlaveAddr��оƬIIC��ַ
*����        	:	TRUE:��ʼ���ɹ���FALSE:��ʼ��ʧ��
*����			: 	�ײ�궨��
*����       	:	
*ʱ��     		:	2018-04-09
*����޸�ʱ��	:	2018-04-09
*˵��        	:	
*************************************************************************************************************************/
bool MMA8452Q_Init(void) {
    u8 temp;

    temp = 0x01;
    IIC_Write(MMA8541, 0x2A, &temp, 0x01);
    temp = 0x02;
    IIC_Write(MMA8541, 0x2B, &temp, 0x02);

    IIC_Read(MMA8541, 0x0D, &temp, 1); //��ȡ����ID
    if (temp != 0x2A) {                //ID����
        logd("��ʼ��ʧ�ܣ�ID����%#02x", temp);
        return false;
    }

    logd("ID��0x%02X", temp);

    return true;
}

/*************************************************************************************************************************
*����        	:	bool MMA8452Q_ReadAcceleration(u8 I2cAddr, s16 *pXa,s16 *pYa, s16 *pZa)
*����        	:	MMA8452Q ��ȡ������ٶ�
*����        	:	pHandle:�����pXa������X����ٶȣ�pYa������Y����ٶȣ�pZa������Z����ٶ�
*����        	:	TRUE:�ɹ���FALSE:ʧ��
*����			: 	�ײ�궨��
*����       	:	
*ʱ��     		:	2018-04-09
*����޸�ʱ��	:	2018-04-09
*˵��        	:	
*************************************************************************************************************************/
bool MMA8452Q_ReadAcceleration(u8 ic, s16 *pXa, s16 *pYa, s16 *pZa) {
    u8 buff[6];
    s16 temp;
    u8 dat = 0;
    IIC_Read(MMA8541, 0x0D, &dat, 1); //��ȡ����ID
    if (dat != 0x2A)                  //ID����
    {
        return false;
    }
    IIC_Read(MMA8541, 1, buff, 6); //��ȡ����
    temp = buff[0];
    temp <<= 4;
    temp |= buff[1] >> 4;
    if (buff[0] & 0x80) //����
    {
        temp |= 0xF000;
    }
    *pXa = temp; //X��

    temp = buff[2];
    temp <<= 4;
    temp |= buff[3] >> 4;
    if (buff[2] & 0x80) //����
    {
        temp |= 0xF000;
    }
    *pYa = temp; //Y��

    temp = buff[4];
    temp <<= 4;
    temp |= buff[5] >> 4;
    if (buff[4] & 0x80) //����
    {
        temp |= 0xF000;
    }
    *pZa = temp; //Z��

    return true;
}

/*************************************************************************************************************************
*����        	:	void ADXL362_CalAngle(s16 Xa,s16 Ya, s16 Za, float *pAngleX, float *pAngleY, float *pAngleZ)
*����        	:	ADXL362 ͨ�����ٶȼ���Ƕ���Ϣ
*����        	:	Xa��X����ٶȣ�Ya��Y����ٶȣ�Za��Z����ٶȣ�pAngleX:X������ǣ�pAngleY��Y������ǣ�pAngleZ��Z�������
*����        	:	��
*����			: 	�ײ�궨��
*����       	:	
*ʱ��     		:	2016-04-06
*����޸�ʱ��	:	2018-03-13
*˵��        	:	��Ҫ���и��㣬����������
*************************************************************************************************************************/
void MMA8452Q_CalAngle(s16 Xa, s16 Ya, s16 Za, float *pAngleX, float *pAngleY, float *pAngleZ) {
    double A;

    //X����
    A = (double)Ya * Ya + (double)Za * Za;
    A = sqrt(A);
    A = (double)Xa / A;
    A = atan(A);
    A = A * 180 / PI;
    if (Za < 0) { //������ת��Ϊ��180��
        if (A < 0)
            A = -90 - (A + 90);
        else
            A = 90 + (90 - A);
    }
    if (A < 0) { //������ת��Ϊ360��
        A = fabs(A);
        A = 180 + 180 - A;
    }
    *pAngleX = A;

    //Y����
    A = (double)Xa * Xa + (double)Za * Za;
    A = sqrt(A);
    A = (double)Ya / A;
    A = atan(A);
    A = A * 180 / PI;
    if (Za < 0) { //������ת��Ϊ��180��
        if (A < 0)
            A = -90 - (A + 90);
        else
            A = 90 + (90 - A);
    }
    if (A < 0) { //������ת��Ϊ360��
        A = fabs(A);
        A = 180 + 180 - A;
    }
    *pAngleY = A;

    //Z����
    A = (double)Xa * Xa + (double)Ya * Ya;
    A = sqrt(A);
    A = (double)A / abs(Za);
    A = atan(A);
    A = A * 180 / PI;
    logd("\tXA:%d\tYA:%d\tZA:%d", Xa, Ya, Za);
    /*if(Za<0)//������ת��Ϊ��180��
	{
		if(A <0)A=-90-(A+90);
		else A=90+(90-A);
	}
	if(A <0)	//������ת��Ϊ360��
	{
		A=fabs(A);
		A = 180+180-A;
	}*/
    *pAngleZ = A;
}

/*************************************************************************************************************************
*����        	:	bool MMA8452Q_GetZAxisAngle(u8 I2cAddr,s16 AcceBuff[3], float *pAngleZ)
*����        	:	MMA8452Q ��ȡZ�����
*����        	:	pHandle:�����AcceBuff:3����ļ��ٶȣ�pAngleZ��Y�������
*����        	:	TRUE:�ɹ���FALSE:ʧ��
*����			: 	�ײ�궨��
*����       	:	
*ʱ��     		:	2018-04-09
*����޸�ʱ��	:	2018-04-09
*˵��        	:	
*************************************************************************************************************************/
bool MMA8452Q_GetZAxisAngle(GRAY_XYZ *xyz) {
    double fx, fy, fz;
    double A;
    s16 Xa, Ya, Za;

    if (MMA8452Q_ReadAcceleration(MMA8541, &Xa, &Ya, &Za) == false) return false; //ADXL362 ��ȡ���ٶ�����
    //uart_printf("Xa:%d \tYa:%d \tZa:%d \r\n",Xa,Ya,Za);

    fx = Xa;
    fx *= 10.0 / 1024;
    fy = Ya;
    fy *= 10.0 / 1024;
    fz = Za;
    fz *= 10.0 / 1024;

    //logd("\tfx��%.04f\tfy��%.04f\tfz��%.04f",fx,fy,fz);

    //Z����
    A = fx * fx + fy * fy;
    A = sqrt(A);
    A = (double)A / fz;
    A = atan(A);
    A = A * 180 / PI;

    xyz->angle = A;
    //logd("=======�Ƕȣ�%.04f",*pAngleZ);
    xyz->x = Xa;
    xyz->y = Ya;
    xyz->z = Za;

    return true;
}

void Mma8452Q_Read(void) {
    static u32 rtm = 0;

    if (_pastn(rtm) > 30000) {
        rtm = jiffies;
        if (MMA8452Q_GetZAxisAngle(&Acce_XYZ)) //ADXL362 ��ȡ�Ƕ���Ϣ
        {
            //�Ƕȶ�ȡʧ��
            //logd("^^^ XX=%5d, YY=%5d, ZZ=%5d, Angle=%9.04f ^^^",Acce_XYZ.x,Acce_XYZ.y,Acce_XYZ.z,Acce_XYZ.angle);
        }
    }
}

#if 0
    g_SysFlag.ADXL362_Status = MMA8452Q_Init(&g_SysFlag.MMA8452Q_Handle, 0x1C<<1);
	if(g_SysFlag.ADXL362_Status == false)
	{
		DEBUG("MMA8452Q��ʼ��ʧ�ܣ�\r\n");
	}

#endif
#endif

