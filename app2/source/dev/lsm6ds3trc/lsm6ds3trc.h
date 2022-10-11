/*
 * @author: rtrobot<admin@rtrobot.org>
 * @website:rtrobot.org
 * @licence: GPL v3
 */

#ifndef __LSM6DS3TRC_H__
#define __LSM6DS3TRC_H__

#if (MODULE_USING_lsm6ts3trc)
//#include "c_common.h"
// #include <stdint.h>
// #include <stdbool.h>

#pragma region 程序移植在哪个环境下
//1.STM32_HAL
//#define LSM6DS3TRC_USE_STM32_HAL
#ifndef LSM6DS3TRC_USE_STM32_HAL
//2.公务车
//#define LSM6DS3TRC_USE_ZHT8
#ifndef LSM6DS3TRC_USE_ZHT8
//3.同人伟业
#define USE_TONG_REN_WEI_YE
#endif
#endif
#pragma endregion 程序移植在哪个环境下

#include "lsm6ds3trc_type.h"

typedef struct {
    float gry_x;
    float gry_y;
    float gry_z;
    float acc_x;
    float acc_y;
    float acc_z;
} lsm_dat_st;

typedef struct {
    unsigned char slave_addr;
    LSM6DS3TRC_ACC_FSXL_enum fsxl;
    LSM6DS3TRC_GYR_FSG_enum fsg;
    LSM6DS3TRC_ACC_RATE_enum acc_rate;
    LSM6DS3TRC_GYR_RATE_enum gyr_rate;
} lsm_cfg_st;

#if (0)
void LSM6DS3TRC_Init(void);
unsigned char LSM6DS3TRC_Get_Status(void);
void LSM6DS3TRC_get_acc_data(LSM6DS3TRC_ACC_FSXL_enum fsxl, float *acc_float);
void LSM6DS3TRC_get_gyr_data(LSM6DS3TRC_GYR_FSG_enum fsg, float *acc_float);
float LSM6DS3TRC_get_temperature(void);
#endif

int LSM6DS3TRC_Init(void);
int LSM6DS3TRC_Initialize(unsigned char slave_addr, LSM6DS3TRC_ACC_FSXL_enum fsxl, LSM6DS3TRC_GYR_FSG_enum fsg, LSM6DS3TRC_ACC_RATE_enum acc_rate, LSM6DS3TRC_GYR_RATE_enum gyr_rate);
int LSM6DS3TRC_get_acc_gyr_ok(lsm_dat_st *lsm_dat);
#endif //#if (MODULE_USING_lsm6ts3trc)

#endif /* __LSM6DS3TRC_H__ */
