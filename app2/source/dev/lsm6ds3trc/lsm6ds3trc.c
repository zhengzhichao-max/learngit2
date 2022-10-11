/*
 * @author: rtrobot<admin@rtrobot.org>
 * @website:rtrobot.org
 * @licence: GPL v3
 */

#if (MODULE_USING_lsm6ts3trc)
//#include "c_common.h"
#include "lsm6ds3trc.h"
#include "lsm6ds3trc_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "include_all.h"

#define M_PI (3.14159265358979323846f)
#define ONE_G (9.807f) //  m/s2

#pragma region 程序移植在哪个环境下
#ifdef LSM6DS3TRC_USE_STM32_HAL
#include <stm32f1xx_hal.h>
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
#endif
#ifdef LSM6DS3TRC_USE_ZHT8
#include "ucos_ii.h"
#include "qst_sw_i2c.h"
#endif
#ifdef USE_TONG_REN_WEI_YE
#include "include_all.h"
#endif
#pragma endregion 程序移植在哪个环境下

static lsm_cfg_st lsm_cfg = {.slave_addr = 0x6A, .fsxl = LSM6DS3TRC_ACC_FSXL_2G, .fsg = LSM6DS3TRC_GYR_FSG_2000, .acc_rate = LSM6DS3TRC_ACC_RATE_104HZ, .gyr_rate = LSM6DS3TRC_GYR_RATE_104HZ};

#ifdef LSM6DS3TRC_USE_ZHT8
/*
ret: 0 = fail, 1 = success,
*/
static unsigned char
lsm6ds3trc_read_reg(unsigned char reg, unsigned char *buf, int len) {
    unsigned char ret = 0;
    unsigned int retry = 0;

    while ((!ret) && (retry++ < 5)) {
        if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
            ret = qst_sw_readreg(LSM6DS3TRC_I2CADDR << 0x01, reg, buf, len);
        } else {
            //ret = qst_8658_spi_read(reg, buf, len);
            printf("err!!!\r\n");
        }
    }

    return ret;
}

/*
ret: 0 = fail, 1 = success,
*/
static unsigned char qmi8658_write_regs(unsigned char reg, unsigned char *value, int len) {
    int i, ret = 0;

    for (i = 0; i < len; i++) {
        if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
            ret = qst_sw_writeregs(LSM6DS3TRC_I2CADDR << 0x01, reg, value, len);
        } else {
            //ret = qst_imu_spi_write_bytes(reg, value, len);
            printf("err!!!\r\n");
        }
    }

    return ret;
}
#endif

/***************************************************************************************************************
LSM6DS3TRC Read Command
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_ReadCommand(unsigned char reg_addr, unsigned char *rev_data, int length) {
#pragma region 程序移植在哪个环境下
#ifdef LSM6DS3TRC_USE_STM32_HAL
    if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
        HAL_I2C_Mem_Read(&hi2c1, LSM6DS3TRC_I2CADDR << 0x01, reg_addr, 1, rev_data, length, 100);
    } else {
        unsigned char reg_spi[1] = {reg_addr | 0x80};
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi1, reg_spi, 1, 100);
        HAL_SPI_Receive(&hspi1, rev_data, length, 100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    }
#endif

#ifdef LSM6DS3TRC_USE_ZHT8
    if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
        lsm6ds3trc_read_reg(reg_addr, rev_data, length);
    } else {
    }
#endif

#ifdef USE_TONG_REN_WEI_YE
    if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
        //六轴LSM6DS3TR-C:
        if (IIC_Read(lsm_cfg.slave_addr << 1, reg_addr, rev_data, length, "lsm") == RT_EOK) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
#endif
#pragma endregion 程序移植在哪个环境下
}

/***************************************************************************************************************
LSM6DS3TRC Write Command
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_WriteCommand(unsigned char reg_addr, unsigned char *send_data, int length) {
#pragma region 程序移植在哪个环境下
#ifdef LSM6DS3TRC_USE_STM32_HAL
    if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
        HAL_I2C_Mem_Write(&hi2c1, LSM6DS3TRC_I2CADDR << 0x01, reg_addr, 1, send_data, length, 100);
    } else {
        unsigned char reg_spi[1] = {reg_addr & 0x7f};
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        while (HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_TX)
            ;
        HAL_SPI_Transmit(&hspi1, reg_spi, 1, 100);
        HAL_SPI_Transmit(&hspi1, send_data, length, 100);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    }
#endif

#ifdef LSM6DS3TRC_USE_ZHT8
    if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
        qmi8658_write_regs(reg_addr, send_data, length);
    } else {
    }
#endif

#ifdef USE_TONG_REN_WEI_YE
    if (LSM6DS3TRC_MODE == LSM6DS3TRC_MODE_I2C) {
        //六轴LSM6DS3TR-C:
        if (IIC_Write(lsm_cfg.slave_addr << 1, reg_addr, send_data, length, "lsm") == RT_EOK) {
            return 0;
        } else {
            return -1;
        }
    } else {
        return -1;
    }
#endif
#pragma endregion 程序移植在哪个环境下
}

/***************************************************************************************************************
LSM6DS3TRC Get id
****************************************************************************************************************/
//ret: 0 success, -1 fail
static int LSM6DS3TRC_GetChipID(void) {
    unsigned char buf = 0;

    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_WHO_AM_I, &buf, 1);
    //六轴LSM6DS3TR-C:

    if (buf != 0x6A) {
        loge("reg.id = 0x%02X, != 0x6A", buf);
        return -1;
    }

    logi("reg.id = 0x%02X, ok", buf);
    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC reboot and reset register
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Reset(void) {
    //reboot modules
    unsigned char buf = 0x80;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL3_C, &buf, 1) != 0) {
        return -1;
    }

#pragma region 程序移植在哪个环境下
#ifdef LSM6DS3TRC_USE_STM32_HAL
    //延时函数
    HAL_Delay(15);
#endif

#ifdef LSM6DS3TRC_USE_ZHT8
    //延时函数
    OSTimeDlyHMSM(0, 0, 0, 15);
#endif

#ifdef USE_TONG_REN_WEI_YE
    //延时函数
    delayms(15); //ms
#endif
#pragma endregion 程序移植在哪个环境下

    //reset register
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL3_C, &buf, 1) != 0) {
        return -1;
    }

    buf |= 0x01;

    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL3_C, &buf, 1) != 0) {
        return -1;
    }

    int cnt = 0;
    do {
        delayms(2);
        if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL3_C, &buf, 1) != 0) {
            continue;
        }
        if ((buf & 0x01) == 0) {
            logi("lsm rst, ok %d", cnt);
            return 0;
        }
    } while (++cnt < 255);

    loge("lsm rst, err %d", cnt);
    return -1;
}

/***************************************************************************************************************
LSM6DS3TRC Set Block Data Update
BDU:
Block Data Update. Default value: 0
(0: continuous update; 1: output registers not updated until MSB and LSB have 
been read)
****************************************************************************************************************/
//入参：
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_BDU(BDU_MODE_enum mode) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL3_C, &buf, 1) != 0) {
        return -1;
    }

    if (mode == BDU_MODE1_NOT_UPDATED_UNTIL_READ) {
        buf |= (1 << 6);
        if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL3_C, &buf, 1) != 0) {
            return -1;
        }
    } else {
        buf &= (~(1 << 6));
        if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL3_C, &buf, 1) != 0) {
            return -1;
        }
    }

#if (0)
    unsigned char buf2 = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL3_C, &buf2, 1) != 0) {
        return -1;
    }
#endif

    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC Set accelerometer data rate
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Accelerometer_Rate(unsigned char rate) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL1_XL, &buf, 1) != 0) {
        return -1;
    }
    buf |= rate;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL1_XL, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC Set gyroscope data rate
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Gyroscope_Rate(unsigned char rate) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL2_G, &buf, 1) != 0) {
        return -1;
    }
    buf |= rate;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL2_G, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC Set accelerometer full-scale selection.
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Accelerometer_Fullscale(unsigned char value) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL1_XL, &buf, 1) != 0) {
        return -1;
    }
    buf |= value;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL1_XL, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC Set gyroscope full-scale selection.
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Gyroscope_Fullscale(unsigned char value) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL2_G, &buf, 1) != 0) {
        return -1;
    }
    buf |= value;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL2_G, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC Set accelerometer analog chain bandwidth.
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Accelerometer_Bandwidth(unsigned char BW0XL, unsigned char ODR) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL1_XL /*10h*/, &buf, 1) != 0) {
        return -1;
    }
    buf |= BW0XL;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL1_XL /*10h*/, &buf, 1) != 0) {
        return -1;
    }

    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL8_XL /*17h*/, &buf, 1) != 0) {
        return -1;
    }
    buf |= ODR;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL8_XL /*17h*/, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC Set register 6
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Register4(unsigned char reg4) {
    unsigned char buf = 0;

    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL4_C, &buf, 1) != 0) {
        return -1;
    }
    buf |= reg4;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL4_C, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

#if (0)
/***************************************************************************************************************
LSM6DS3TRC Set register 5
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Register5(unsigned char reg5) {
    unsigned char buf = 0;

    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL5_C, &buf, 1) != 0) {
        return -1;
    }
    buf |= reg5;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL5_C, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}
#endif

/***************************************************************************************************************
LSM6DS3TRC Set register 6
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Register6(unsigned char reg6) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL6_C, &buf, 1) != 0) {
        return -1;
    }
    buf |= reg6;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL6_C, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

/***************************************************************************************************************
LSM6DS3TRC Set register 7
****************************************************************************************************************/
//ret//0 success//1 fail
static int LSM6DS3TRC_Set_Register7(unsigned char reg7) {
    unsigned char buf = 0;

    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_CTRL7_G, &buf, 1) != 0) {
        return -1;
    }
    buf |= reg7;
    if (LSM6DS3TRC_WriteCommand(LSM6DS3TRC_CTRL7_G, &buf, 1) != 0) {
        return -1;
    }
    return 0;
}

#if (0)
/***************************************************************************************************************
LSM6DS3TRC Get data status
****************************************************************************************************************/
unsigned char LSM6DS3TRC_Get_Status(void) {
    unsigned char buf[1] = {0};
    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_STATUS_REG, buf, 1);
    return buf[0];
}

/***************************************************************************************************************
LSM6DS3TRC Get Acceleration Value
****************************************************************************************************************/
void LSM6DS3TRC_get_acc_data(LSM6DS3TRC_ACC_FSXL_enum fsxl, float *acc_float) {
    unsigned char buf[6];
    short int acc[3];
    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_OUTX_L_XL, buf, 6);
    acc[0] = (buf[1] << 8) | buf[0];
    acc[1] = (buf[3] << 8) | buf[2];
    acc[2] = (buf[5] << 8) | buf[4]; //acc[2] = buf[4] << 8 | buf[5];//acc[2] = buf[5] << 8 | buf[4];

    switch (fsxl) {
    case LSM6DS3TRC_ACC_FSXL_2G: //  m/s2
        acc_float[0] = (((float)acc[0] * 0.061f) * ONE_G) / 1000.0f;
        acc_float[1] = (((float)acc[1] * 0.061f) * ONE_G) / 1000.0f;
        acc_float[2] = (((float)acc[2] * 0.061f) * ONE_G) / 1000.0f;
        break;

    case LSM6DS3TRC_ACC_FSXL_16G: //  m/s2
        acc_float[0] = (((float)acc[0] * 0.488f) * ONE_G) / 1000.0f;
        acc_float[1] = (((float)acc[1] * 0.488f) * ONE_G) / 1000.0f;
        acc_float[2] = (((float)acc[2] * 0.488f) * ONE_G) / 1000.0f;
        break;

    case LSM6DS3TRC_ACC_FSXL_4G: //  m/s2
        acc_float[0] = (((float)acc[0] * 0.122f) * ONE_G) / 1000.0f;
        acc_float[1] = (((float)acc[1] * 0.122f) * ONE_G) / 1000.0f;
        acc_float[2] = (((float)acc[2] * 0.122f) * ONE_G) / 1000.0f;
        break;

    case LSM6DS3TRC_ACC_FSXL_8G: //  m/s2
        acc_float[0] = (((float)acc[0] * 0.244f) * ONE_G) / 1000.0f;
        acc_float[1] = (((float)acc[1] * 0.244f) * ONE_G) / 1000.0f;
        acc_float[2] = (((float)acc[2] * 0.244f) * ONE_G) / 1000.0f;
        break;
    }
}

/***************************************************************************************************************
LSM6DS3TRC Get Gyroscope Value
****************************************************************************************************************/
void LSM6DS3TRC_get_gyr_data(LSM6DS3TRC_GYR_FSG_enum fsg, float *gry_float) {
    unsigned char buf[6];
    short int gyr[3];
    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_OUTX_L_G, buf, 6);
    gyr[0] = buf[1] << 8 | buf[0];
    gyr[1] = buf[3] << 8 | buf[2];
    gyr[2] = buf[5] << 8 | buf[4];

#if (1)
    switch (fsg) {
    case LSM6DS3TRC_GYR_FSG_245:
        gyr_float[0] = ((float)gyr[0] * 8.750f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[1] = ((float)gyr[1] * 8.750f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[2] = ((float)gyr[2] * 8.750f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        break;
    case LSM6DS3TRC_GYR_FSG_500:
        gyr_float[0] = ((float)gyr[0] * 17.50f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[1] = ((float)gyr[1] * 17.50f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[2] = ((float)gyr[2] * 17.50f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        break;
    case LSM6DS3TRC_GYR_FSG_1000:
        gyr_float[0] = ((float)gyr[0] * 35.00f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[1] = ((float)gyr[1] * 35.00f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[2] = ((float)gyr[2] * 35.00f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        break;
    case LSM6DS3TRC_GYR_FSG_2000:
        gyr_float[0] = ((float)gyr[0] * 70.00f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[1] = ((float)gyr[1] * 70.00f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        gyr_float[2] = ((float)gyr[2] * 70.00f /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
        break;
    }
#else
    switch (fsg) {
    case LSM6DS3TRC_GYR_FSG_245:
        gyr_float[0] = ((float)gyr[0] * 8.750f /*m(du)/s*/);
        gyr_float[1] = ((float)gyr[1] * 8.750f /*m(du)/s*/);
        gyr_float[2] = ((float)gyr[2] * 8.750f /*m(du)/s*/);
        break;
    case LSM6DS3TRC_GYR_FSG_500:
        gyr_float[0] = ((float)gyr[0] * 17.50f /*m(du)/s*/);
        gyr_float[1] = ((float)gyr[1] * 17.50f /*m(du)/s*/);
        gyr_float[2] = ((float)gyr[2] * 17.50f /*m(du)/s*/);
        break;
    case LSM6DS3TRC_GYR_FSG_1000:
        gyr_float[0] = ((float)gyr[0] * 35.00f /*m(du)/s*/);
        gyr_float[1] = ((float)gyr[1] * 35.00f /*m(du)/s*/);
        gyr_float[2] = ((float)gyr[2] * 35.00f /*m(du)/s*/);
        break;
    case LSM6DS3TRC_GYR_FSG_2000:
        gyr_float[0] = ((float)gyr[0] * 70.00f /*m(du)/s*/);
        gyr_float[1] = ((float)gyr[1] * 70.00f /*m(du)/s*/);
        gyr_float[2] = ((float)gyr[2] * 70.00f /*m(du)/s*/);
        break;
    }
#endif
}

float LSM6DS3TRC_get_temperature(void) {
    unsigned char buf[2];
    short int temp;
    LSM6DS3TRC_ReadCommand(LSM6DS3TRC_OUT_TEMP_L, buf, 2);
    temp = buf[1] << 8 | buf[0];
    return (((float)temp / 256.0) + 25.0);
}
#endif

static void promt_lsm_cfg(lsm_cfg_st *lsm_cfg) {
    int fsxl = -1;
    switch (lsm_cfg->fsxl) {
    case LSM6DS3TRC_ACC_FSXL_2G: fsxl = 2; break;
    case LSM6DS3TRC_ACC_FSXL_4G: fsxl = 4; break;
    case LSM6DS3TRC_ACC_FSXL_8G: fsxl = 8; break;
    case LSM6DS3TRC_ACC_FSXL_16G: fsxl = 16; break;
    default: break;
    }

    int fsg = -1;
    switch (lsm_cfg->fsg) {
    case LSM6DS3TRC_GYR_FSG_245: fsg = 245; break;
    case LSM6DS3TRC_GYR_FSG_500: fsg = 500; break;
    case LSM6DS3TRC_GYR_FSG_1000: fsg = 1000; break;
    case LSM6DS3TRC_GYR_FSG_2000: fsg = 2000; break;
    default: break;
    }

    float acc_rate = -1.0f;
    switch (lsm_cfg->acc_rate) {
    case LSM6DS3TRC_ACC_RATE_0: acc_rate = 0.0f; break;
    case LSM6DS3TRC_ACC_RATE_1HZ6: acc_rate = 1.6f; break;
    case LSM6DS3TRC_ACC_RATE_12HZ5: acc_rate = 12.5f; break;
    case LSM6DS3TRC_ACC_RATE_26HZ: acc_rate = 26.0f; break;
    case LSM6DS3TRC_ACC_RATE_52HZ: acc_rate = 52.0f; break;
    case LSM6DS3TRC_ACC_RATE_104HZ: acc_rate = 104.0f; break;
    case LSM6DS3TRC_ACC_RATE_208HZ: acc_rate = 208.0f; break;
    case LSM6DS3TRC_ACC_RATE_416HZ: acc_rate = 416.0f; break;
    case LSM6DS3TRC_ACC_RATE_833HZ: acc_rate = 833.0f; break;
    case LSM6DS3TRC_ACC_RATE_1660HZ: acc_rate = 1660.0f; break;
    case LSM6DS3TRC_ACC_RATE_3330HZ: acc_rate = 3330.0f; break;
    case LSM6DS3TRC_ACC_RATE_6660HZ: acc_rate = 6660.0f; break;
    default: break;
    }

    float gyr_rate = -1.0f;
    switch (lsm_cfg->gyr_rate) {
    case LSM6DS3TRC_GYR_RATE_0: gyr_rate = 0.0f; break;
    case LSM6DS3TRC_GYR_RATE_1HZ6: gyr_rate = 1.6f; break;
    case LSM6DS3TRC_GYR_RATE_12HZ5: gyr_rate = 12.5f; break;
    case LSM6DS3TRC_GYR_RATE_26HZ: gyr_rate = 26.0f; break;
    case LSM6DS3TRC_GYR_RATE_52HZ: gyr_rate = 52.0f; break;
    case LSM6DS3TRC_GYR_RATE_104HZ: gyr_rate = 104.0f; break;
    case LSM6DS3TRC_GYR_RATE_208HZ: gyr_rate = 208.0f; break;
    case LSM6DS3TRC_GYR_RATE_416HZ: gyr_rate = 416.0f; break;
    case LSM6DS3TRC_GYR_RATE_833HZ: gyr_rate = 833.0f; break;
    case LSM6DS3TRC_GYR_RATE_1660HZ: gyr_rate = 1660.0f; break;
    case LSM6DS3TRC_GYR_RATE_3330HZ: gyr_rate = 3330.0f; break;
    case LSM6DS3TRC_GYR_RATE_6660HZ: gyr_rate = 6660.0f; break;
    default: break;
    }

    if (fsxl == -1) { loge("fsxl err"); }
    if (fsg == -1) { loge("fsg err"); }
    if (acc_rate == -1.0) { loge("acc_rate err"); }
    if (gyr_rate == -1.0) { loge("gyr_rate err"); }

    logd("set slave 0x%02X, scale(%dG, %dfsg), frate(%0.1f, %0.1f)", lsm_cfg->slave_addr, fsxl, fsg, acc_rate, gyr_rate);
}

//ret//0 success//1 fail
static int LSM6DS3TRC_acc_gyr_ready(void) {
    unsigned char buf = 0;
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_STATUS_REG, &buf, 1) != 0) {
        loge("1");
        return -1;
    }

    //acc //gyr
    if (!((buf & LSM6DS3TRC_STATUS_ACCELEROMETER) && (buf & LSM6DS3TRC_STATUS_GYROSCOPE))) {
        loge("acc%d gyr%d", buf & LSM6DS3TRC_STATUS_ACCELEROMETER, buf & LSM6DS3TRC_STATUS_GYROSCOPE);
        return -1;
    }

    return 0;
}

//0 success//-1 fail
static int acc_gyr_buf_to_lsm_data(unsigned char *buf /*[12]*/, lsm_dat_st *lsm) {
    short int acc[3];
    short int gyr[3];

    int index = 0;
    gyr[0] = buf[index + 1] << 8 | buf[index];
    index += 2;
    gyr[1] = buf[index + 1] << 8 | buf[index];
    index += 2;
    gyr[2] = buf[index + 1] << 8 | buf[index];
    index += 2;
    acc[0] = (buf[index + 1] << 8) | buf[index];
    index += 2;
    acc[1] = (buf[index + 1] << 8) | buf[index];
    index += 2;
    acc[2] = (buf[index + 1] << 8) | buf[index];

    float acc_coef = -0.1f;
    switch (lsm_cfg.fsxl) {
    case LSM6DS3TRC_ACC_FSXL_2G: acc_coef = 0.061f; break;
    case LSM6DS3TRC_ACC_FSXL_16G: acc_coef = 0.488f; break;
    case LSM6DS3TRC_ACC_FSXL_4G: acc_coef = 0.122f; break;
    case LSM6DS3TRC_ACC_FSXL_8G: acc_coef = 0.244f; break;
    default: break;
    }

    lsm->acc_x = (((float)acc[0] * acc_coef) * ONE_G) / 1000.0f; //  m/s2
    lsm->acc_y = (((float)acc[1] * acc_coef) * ONE_G) / 1000.0f; //  m/s2
    lsm->acc_z = (((float)acc[2] * acc_coef) * ONE_G) / 1000.0f; //  m/s2

    float gyr_coef = -0.1f;
    switch (lsm_cfg.fsg) {
    case LSM6DS3TRC_GYR_FSG_245: gyr_coef = 8.750f; break;
    case LSM6DS3TRC_GYR_FSG_500: gyr_coef = 17.50f; break;
    case LSM6DS3TRC_GYR_FSG_1000: gyr_coef = 35.00f; break;
    case LSM6DS3TRC_GYR_FSG_2000: gyr_coef = 70.00f; break;
    default: break;
    }

    lsm->gry_x = ((float)gyr[0] * gyr_coef /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
    lsm->gry_y = ((float)gyr[1] * gyr_coef /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s
    lsm->gry_z = ((float)gyr[2] * gyr_coef /*m(du)/s*/ * M_PI / 180.0f / 1000); //rad/s

    if (acc_coef == -0.1f) {
        loge("acc coef err");
        return -1;
    }

    if (gyr_coef == -0.1f) {
        loge("gyr coef err");
        return -1;
    }
    
    return 0;
}

int LSM6DS3TRC_Init(void) {
    if (LSM6DS3TRC_GetChipID() != 0) { return -1; }
    //reboot and reset register
    if (LSM6DS3TRC_Reset() != 0) { return -1; }
    //enable Block Data Update
    if (LSM6DS3TRC_Set_BDU(BDU_MODE1_NOT_UPDATED_UNTIL_READ) != 0) { return -1; }
    //Set Data Rate
    if (LSM6DS3TRC_Set_Accelerometer_Rate(lsm_cfg.acc_rate /* 10h[7..4] */) != 0) { return -1; }
    if (LSM6DS3TRC_Set_Gyroscope_Rate(lsm_cfg.gyr_rate /* 11h[7..4] */) != 0) { return -1; }
    //Set full-scale selection.
    if (LSM6DS3TRC_Set_Accelerometer_Fullscale(lsm_cfg.fsxl /* 10h[3..2] */) != 0) { return -1; }
    if (LSM6DS3TRC_Set_Gyroscope_Fullscale(lsm_cfg.fsg /* 11[1..0] */) != 0) { return -1; }
    //set accelerometer analog chain bandwidth.
    if (LSM6DS3TRC_Set_Accelerometer_Bandwidth(LSM6DS3TRC_ACC_BW0XL_50HZ /* 10h[1..0] */, LSM6DS3TRC_ACC_LOW_PASS_ODR_100 /* 17h */) != 0) { return -1; }
    if (LSM6DS3TRC_Set_Register7(LSM6DS3TRC_CTRL7_G_HM_MODE_DISABLE | LSM6DS3TRC_CTRL7_G_HPM_260MHZ) != 0) { return -1; }
    if (LSM6DS3TRC_Set_Register6(LSM6DS3TRC_CTRL6_C_FTYPE_1) != 0) { return -1; }
    if (LSM6DS3TRC_Set_Register4(LSM6DS3TRC_CTRL4_LPF1_SELG_ENABLE) != 0) { return -1; }
    if (LSM6DS3TRC_GetChipID() != 0) { return -1; }
    return 0;
}

//0 success//-1 fail
int LSM6DS3TRC_Initialize(unsigned char slave_addr, LSM6DS3TRC_ACC_FSXL_enum fsxl, LSM6DS3TRC_GYR_FSG_enum fsg, LSM6DS3TRC_ACC_RATE_enum acc_rate, LSM6DS3TRC_GYR_RATE_enum gyr_rate) {
    lsm_cfg.slave_addr = slave_addr;
    lsm_cfg.fsxl = fsxl;
    lsm_cfg.fsg = fsg;
    lsm_cfg.acc_rate = acc_rate;
    lsm_cfg.gyr_rate = gyr_rate;
    promt_lsm_cfg(&lsm_cfg);
    return LSM6DS3TRC_Init();
}

//ret//0 success//1 fail
int LSM6DS3TRC_get_acc_gyr_ok(lsm_dat_st *lsm_dat) {
    if (LSM6DS3TRC_acc_gyr_ready() != 0) {
        //loge("1");
        return -1;
    }

    unsigned char buf[12] = {0};
    if (LSM6DS3TRC_ReadCommand(LSM6DS3TRC_OUTX_L_G, buf, 12) != 0) {
        loge("2");
        return -1;
    }

    if (acc_gyr_buf_to_lsm_data(buf, lsm_dat) != 0) {
        //loge("3");
        return -1;
    }

    return 0;
}
#endif //#if (MODULE_USING_lsm6ts3trc)

#if (0)
    /***************************************************************************************************************
LSM6DS3TRC Init
****************************************************************************************************************/
    //ret: 0:  成功；  -1：  失败
    void
    LSM6DS3TRC_Init(void) {
#if (1)
    int cnt = 0;

    do {
        if (LSM6DS3TRC_Initialize() == 0) {
            return;
        }
        uart_manage_task();
        print_auto_process();
        delayms(500);
    } while (++cnt < 10);

    loge("lsm init fail");
#else
    if (sys_bits1.bits.init_lsm == 1) {
        if (LSM6DS3TRC_Initialize() == 0) {
            sys_bits1.bits.init_lsm = 0;
        }
    }
#endif
}
#endif
