#include "IIC.h"
#include "MPU6050.h"
#include"stdio.h"
u8 MPU6050ReadID(void)
{
  unsigned char Re = 0;
  unsigned char addr;
  addr = MPU6050_ADDRESS;
  addr = addr << 1;
  
  Re =  IIC_ReadData(addr, 0x75);
  return Re;
}


void MPU6050AccInit(void)
{
  u8 acc = 0xf1;
  u8 addr= MPU6050_ADDRESS << 1;
  IIC_WriteData(addr, 0x1C,&acc, 1);         				//ACC设置 自测模式 量程 +-16G s 
}
   
char MPU6050ReadAcc(int* accData)
{
  u8 buf[6];
  u8 addr= MPU6050_ADDRESS << 1;

  IIC_ReadMPU(addr, MPU6050_ACC_OUT, buf, 6);
  accData[0] = (buf[0] << 8) | buf[1];
  accData[1] = (buf[2] << 8) | buf[3];
  accData[2] = (buf[4] << 8) | buf[5];
    
  if(accData[0]& 0x8000) accData[0]-=65536;
  if(accData[1]& 0x8000) accData[1]-=65536;
  if(accData[2]& 0x8000) accData[2]-=65536;
   
  return 1;
}


void MPU6050GyroInit(void)
{
   
  u8 addr= MPU6050_ADDRESS << 1;
  u8 inData[7]={0x80,0x00,0x00,0x03,0xf8,0x00,0x02};//0xe0是配置为+-250°/s

  IIC_WriteData(addr, 0x19 , &inData[1],1);             //SMPLRT_DIV    -- SMPLRT_DIV = 0  Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
  
  IIC_WriteData(addr, 0x1A , &inData[2],1); //CONFIG        -- EXT_SYNC_SET 0 (disable input pin for data sync) ; default DLPF_CFG = 0 => ACC bandwidth = 260Hz  GYRO bandwidth = 256Hz)
  IIC_WriteData(addr, 0x6B , &inData[3],1);             //PWR_MGMT_1    -- SLEEP 0; CYCLE 0; TEMP_DIS 0; CLKSEL 3 (PLL with Z Gyro reference)
  IIC_WriteData(addr, 0x1B , &inData[4],1);             //gyro配置 自测模式  量程  0-2000度每秒
  IIC_WriteData(addr, 0x6A , &inData[5],1);							// 0x6A  的 I2C_MST_EN  设置成0  默认为0
  IIC_WriteData(addr, 0x37,  &inData[6],1);							// 0x37  的 I2C_BYPASS_EN  设置成1 使能 pass through 功能
}

void MPU6050ReadGyro(int* gyroData)
{
  u8 buf[6];
      
  u8 addr= MPU6050_ADDRESS << 1;
  IIC_ReadMPU(addr, MPU6050_GYRO_OUT,  buf,6);
  gyroData[0] = (buf[0] << 8) | buf[1];
  gyroData[1] = (buf[2] << 8) | buf[3];
  gyroData[2] = (buf[4] << 8) | buf[5];
  
  if(gyroData[0]& 0x8000) gyroData[0]-=65536;
  if(gyroData[1]& 0x8000) gyroData[1]-=65536;
  if(gyroData[2]& 0x8000) gyroData[2]-=65536; 
}
			   


