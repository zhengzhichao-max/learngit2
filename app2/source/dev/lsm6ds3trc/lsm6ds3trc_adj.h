/*
 * @author: rtrobot<admin@rtrobot.org>
 * @website:rtrobot.org
 * @licence: GPL v3
 */

#ifndef __LSM6DS3TRC_ADJ_H__
#define __LSM6DS3TRC_ADJ_H__

#pragma region 未定义
#if (0)
#include "c_common.h"
#include <stdint.h>
#include <stdbool.h>


/**
 * @brief   ：定义校准参数值：用于抵消误差部分
 * @brief   ：方法，通过固定位置，静止不动，采集数据100个及以上，求平均值，并与理想值作差。
 * @brief   ：    注意有方向性问题
 * @author  ：陈文源
 * @date    ：2022-07-04
 * @version ：1.0
**/
typedef struct lsm6ds3trc_adj{
	int a;
} lsm6ds3trc_adj_st;
#endif
#pragma endregion 未定义

#endif /* __LSM6DS3TRC_ADJ_H__ */
