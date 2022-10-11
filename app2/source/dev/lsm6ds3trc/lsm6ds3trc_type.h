/*
 * @author: rtrobot<admin@rtrobot.org>
 * @website:rtrobot.org
 * @licence: GPL v3
 */

#ifndef _LSM6DS3TRC_TYPE_H_
#define _LSM6DS3TRC_TYPE_H_

#if (MODULE_USING_lsm6ts3trc)
//#include "c_common.h"
// #include <stdint.h>
// #include <stdbool.h>

//#define LSM6DS3TRC_I2CADDR 0x6B //0x6A//0x6B//����LSM6DS3TR-C:

#pragma region ͨ��ģʽ����
#define LSM6DS3TRC_MODE_I2C 0x00
#define LSM6DS3TRC_MODE_SPI 0x01
//-------------------------------
#define LSM6DS3TRC_MODE LSM6DS3TRC_MODE_I2C
#pragma endregion ͨ��ģʽ����

#define LSM6DS3TRC_FUNC_CFG_ACCESS 0x01
#define LSM6DS3TRC_SENSOR_SYNC_TIME_FRAME 0x02
#define LSM6DS3TRC_FIFO_CTRL1 0x06
#define LSM6DS3TRC_FIFO_CTRL2 0x07
#define LSM6DS3TRC_FIFO_CTRL3 0x08
#define LSM6DS3TRC_FIFO_CTRL4 0x09
#define LSM6DS3TRC_FIFO_CTRL5 0x0A
#define LSM6DS3TRC_ORIENT_CFG_G 0x0B
#define LSM6DS3TRC_INT1_CTRL 0x0D
#define LSM6DS3TRC_INT2_CTRL 0x0E
#define LSM6DS3TRC_WHO_AM_I 0x0F
#define LSM6DS3TRC_CTRL1_XL 0x10
#define LSM6DS3TRC_CTRL2_G 0x11
#define LSM6DS3TRC_CTRL3_C 0x12
#define LSM6DS3TRC_CTRL4_C 0x13
#define LSM6DS3TRC_CTRL5_C 0x14
#define LSM6DS3TRC_CTRL6_C 0x15
#define LSM6DS3TRC_CTRL7_G 0x16
#define LSM6DS3TRC_CTRL8_XL 0x17
#define LSM6DS3TRC_CTRL9_XL 0x18
#define LSM6DS3TRC_CTRL10_C 0x19
#define LSM6DS3TRC_MASTER_CONFIG 0x1A
#define LSM6DS3TRC_WAKE_UP_SRC 0x1B
#define LSM6DS3TRC_TAP_SRC 0x1C
#define LSM6DS3TRC_D6D_SRC 0x1D
#define LSM6DS3TRC_STATUS_REG 0x1E
#define LSM6DS3TRC_OUT_TEMP_L 0x20
#define LSM6DS3TRC_OUT_TEMP_H 0x21
#define LSM6DS3TRC_OUTX_L_G 0x22
#define LSM6DS3TRC_OUTX_H_G 0x23
#define LSM6DS3TRC_OUTY_L_G 0x24
#define LSM6DS3TRC_OUTY_H_G 0x25
#define LSM6DS3TRC_OUTZ_L_G 0x26
#define LSM6DS3TRC_OUTZ_H_G 0x27
#define LSM6DS3TRC_OUTX_L_XL 0x28
#define LSM6DS3TRC_OUTX_H_XL 0x29
#define LSM6DS3TRC_OUTY_L_XL 0x2A
#define LSM6DS3TRC_OUTY_H_XL 0x2B
#define LSM6DS3TRC_OUTZ_L_XL 0x2C
#define LSM6DS3TRC_OUTZ_H_XL 0x2D
#define LSM6DS3TRC_SENSORHUB1_REG 0x2E
#define LSM6DS3TRC_SENSORHUB2_REG 0x2F
#define LSM6DS3TRC_SENSORHUB3_REG 0x30
#define LSM6DS3TRC_SENSORHUB4_REG 0x31
#define LSM6DS3TRC_SENSORHUB5_REG 0x32
#define LSM6DS3TRC_SENSORHUB6_REG 0x33
#define LSM6DS3TRC_SENSORHUB7_REG 0x34
#define LSM6DS3TRC_SENSORHUB8_REG 0x35
#define LSM6DS3TRC_SENSORHUB9_REG 0x36
#define LSM6DS3TRC_SENSORHUB10_REG 0x37
#define LSM6DS3TRC_SENSORHUB11_REG 0x38
#define LSM6DS3TRC_SENSORHUB12_REG 0x39
#define LSM6DS3TRC_FIFO_STATUS1 0x3A
#define LSM6DS3TRC_FIFO_STATUS2 0x3B
#define LSM6DS3TRC_FIFO_STATUS3 0x3C
#define LSM6DS3TRC_FIFO_STATUS4 0x3D
#define LSM6DS3TRC_FIFO_DATA_OUT_L 0x3E
#define LSM6DS3TRC_FIFO_DATA_OUT_H 0x3F
#define LSM6DS3TRC_TIMESTAMP0_REG 0x40
#define LSM6DS3TRC_TIMESTAMP1_REG 0x41
#define LSM6DS3TRC_TIMESTAMP2_REG 0x42
#define LSM6DS3TRC_STEP_TIMESTAMP_L 0x49
#define LSM6DS3TRC_STEP_TIMESTAMP_H 0x4A
#define LSM6DS3TRC_STEP_COUNTER_L 0x4B
#define LSM6DS3TRC_STEP_COUNTER_H 0x4C
#define LSM6DS3TRC_SENSORHUB13_REG 0x4D
#define LSM6DS3TRC_SENSORHUB14_REG 0x4E
#define LSM6DS3TRC_SENSORHUB15_REG 0x4F
#define LSM6DS3TRC_SENSORHUB16_REG 0x50
#define LSM6DS3TRC_SENSORHUB17_REG 0x51
#define LSM6DS3TRC_SENSORHUB18_REG 0x52
#define LSM6DS3TRC_FUNC_SRC 0x53
#define LSM6DS3TRC_TAP_CFG 0x58
#define LSM6DS3TRC_TAP_THS_6D 0x59
#define LSM6DS3TRC_INT_DUR2 0x5A
#define LSM6DS3TRC_WAKE_UP_THS 0x5B
#define LSM6DS3TRC_WAKE_UP_DUR 0x5C
#define LSM6DS3TRC_FREE_FALL 0x5D
#define LSM6DS3TRC_MD1_CFG 0x5E
#define LSM6DS3TRC_MD2_CFG 0x5F
#define LSM6DS3TRC_OUT_MAG_RAW_X_L 0x66
#define LSM6DS3TRC_OUT_MAG_RAW_X_H 0x67
#define LSM6DS3TRC_OUT_MAG_RAW_Y_L 0x68
#define LSM6DS3TRC_OUT_MAG_RAW_Y_H 0x69
#define LSM6DS3TRC_OUT_MAG_RAW_Z_L 0x6A
#define LSM6DS3TRC_OUT_MAG_RAW_Z_H 0x6B
#define LSM6DS3TRC_X_OFS_USR 0x73
#define LSM6DS3TRC_Y_OFS_USR 0x74
#define LSM6DS3TRC_Z_OFS_USR 0x75

    //Linear acceleration out data rate
    typedef enum {
        LSM6DS3TRC_ACC_RATE_0 = 0x00,
        LSM6DS3TRC_ACC_RATE_1HZ6 = 0xB0,
        LSM6DS3TRC_ACC_RATE_12HZ5 = 0x10,
        LSM6DS3TRC_ACC_RATE_26HZ = 0x20,
        LSM6DS3TRC_ACC_RATE_52HZ = 0x30,
        LSM6DS3TRC_ACC_RATE_104HZ = 0x40,
        LSM6DS3TRC_ACC_RATE_208HZ = 0x50,
        LSM6DS3TRC_ACC_RATE_416HZ = 0x60,
        LSM6DS3TRC_ACC_RATE_833HZ = 0x70,
        LSM6DS3TRC_ACC_RATE_1660HZ = 0x80,
        LSM6DS3TRC_ACC_RATE_3330HZ = 0x90,
        LSM6DS3TRC_ACC_RATE_6660HZ = 0xA0
    } LSM6DS3TRC_ACC_RATE_enum;

//Linear gyroscope out data rate
typedef enum {
    LSM6DS3TRC_GYR_RATE_0 = 0x00,
    LSM6DS3TRC_GYR_RATE_1HZ6 = 0xB0,
    LSM6DS3TRC_GYR_RATE_12HZ5 = 0x10,
    LSM6DS3TRC_GYR_RATE_26HZ = 0x20,
    LSM6DS3TRC_GYR_RATE_52HZ = 0x30,
    LSM6DS3TRC_GYR_RATE_104HZ = 0x40,
    LSM6DS3TRC_GYR_RATE_208HZ = 0x50,
    LSM6DS3TRC_GYR_RATE_416HZ = 0x60,
    LSM6DS3TRC_GYR_RATE_833HZ = 0x70,
    LSM6DS3TRC_GYR_RATE_1660HZ = 0x80,
    LSM6DS3TRC_GYR_RATE_3330HZ = 0x90,
    LSM6DS3TRC_GYR_RATE_6660HZ = 0xA0
} LSM6DS3TRC_GYR_RATE_enum;

//Accelerometer full-scale.
typedef enum {
    LSM6DS3TRC_ACC_FSXL_2G = 0x00,
    LSM6DS3TRC_ACC_FSXL_16G = 0x04,
    LSM6DS3TRC_ACC_FSXL_4G = 0x08,
    LSM6DS3TRC_ACC_FSXL_8G = 0x0C
} LSM6DS3TRC_ACC_FSXL_enum;

//Gyroscope full-scale.
typedef enum { LSM6DS3TRC_GYR_FSG_245 = 0x00,
               LSM6DS3TRC_GYR_FSG_500 = 0x04,
               LSM6DS3TRC_GYR_FSG_1000 = 0x08,
               LSM6DS3TRC_GYR_FSG_2000 = 0x0C } LSM6DS3TRC_GYR_FSG_enum;

//Accelerometer analog chain bandwidth
#define LSM6DS3TRC_ACC_BW0XL_1500HZ 0x00
#define LSM6DS3TRC_ACC_BW0XL_400HZ 0x01
#define LSM6DS3TRC_ACC_BW0XL_50HZ 0x11

//Accelerometer bandwidth selection
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_50 0x88
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_100 0xA8
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_9 0xC8
#define LSM6DS3TRC_ACC_LOW_PASS_ODR_400 0xE8

#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_50 0x04
#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_100 0x24
#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_9 0x44
#define LSM6DS3TRC_ACC_HIGH_PASS_ODR_400 0x64

//CTRL4_C register
#define LSM6DS3TRC_CTRL4_DEN_XL_EN_DISABLE 0x00
#define LSM6DS3TRC_CTRL4_DEN_XL_EN_ENABLE 0x80
#define LSM6DS3TRC_CTRL4_SLEEP_ENABLE 0x40
#define LSM6DS3TRC_CTRL4_SLEEP_DISABLE 0x00
#define LSM6DS3TRC_CTRL4_DEN_DRDY_INT1_DISBALE 0x00
#define LSM6DS3TRC_CTRL4_DEN_DRDY_INT1_ENABLE 0x20
#define LSM6DS3TRC_CTRL4_DRDY_MASK_DISABLE 0x00
#define LSM6DS3TRC_CTRL4_DRDY_MASK_ENABLE 0x08
#define LSM6DS3TRC_CTRL4_I2C_DISABLE 0x04
#define LSM6DS3TRC_CTRL4_I2C_ENABLE 0x00
#define LSM6DS3TRC_CTRL4_LPF1_SELG_ENABLE 0x02
#define LSM6DS3TRC_CTRL4_LPF1_SELG_DISABLE 0x00

//CTRL6_C register
#define LSM6DS3TRC_CTRL6_C_EDGE_TRIGGER 0x80
#define LSM6DS3TRC_CTRL6_C_LEVEL_TRIGGER 0x40
#define LSM6DS3TRC_CTRL6_C_LEVEL_LATCHED 0x60
#define LSM6DS3TRC_CTRL6_C_LEVEL_FIFO 0xC0
#define LSM6DS3TRC_CTRL6_C_XL_HM_MODE_ENABLE 0x00
#define LSM6DS3TRC_CTRL6_C_XL_HM_MODE_DISABLE 0x10
#define LSM6DS3TRC_CTRL6_C_FTYPE_1 0x00
#define LSM6DS3TRC_CTRL6_C_FTYPE_2 0x01
#define LSM6DS3TRC_CTRL6_C_FTYPE_3 0x02
#define LSM6DS3TRC_CTRL6_C_FTYPE_4 0x03

//CTRL7_G register
#define LSM6DS3TRC_CTRL7_G_HM_MODE_ENABLE 0x00
#define LSM6DS3TRC_CTRL7_G_HM_MODE_DISABLE 0x80
#define LSM6DS3TRC_CTRL7_G_HP_EN_DISABLE 0x00
#define LSM6DS3TRC_CTRL7_G_HP_EN_ENABLE 0x40
#define LSM6DS3TRC_CTRL7_G_HPM_16MHZ 0x00
#define LSM6DS3TRC_CTRL7_G_HPM_65MHZ 0x10
#define LSM6DS3TRC_CTRL7_G_HPM_260MHZ 0x20
#define LSM6DS3TRC_CTRL7_G_HPM_1HZ04 0x30
#define LSM6DS3TRC_CTRL7_G_ROUNDING_STATUS_DISABLE 0x04
#define LSM6DS3TRC_CTRL7_G_ROUNDING_STATUS_ENABLE 0x00

#define LSM6DS3TRC_STATUS_TEMPERATURE 0x04
#define LSM6DS3TRC_STATUS_GYROSCOPE 0x02
#define LSM6DS3TRC_STATUS_ACCELEROMETER 0x01

typedef enum { BDU_MODE0_CONTINUOUS_UPDATE = 0,
               BDU_MODE1_NOT_UPDATED_UNTIL_READ } BDU_MODE_enum;
#endif //#if (MODULE_USING_lsm6ts3trc)

#endif /* _LSM6DS3TRC_TYPE_H_ */