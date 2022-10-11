/*
  ******************************************************************************
  * @file    app_cfg.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  配置与管理中间层初始化和配置
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

#ifndef __APP_CFG_H
#define __APP_CFG_H

typedef struct {
    u16 crc;
    u16 flag_t;
    u16 max;   //用户设置的最大油量电阻
    u16 min;   //用户设置的最小油量电阻
    u16 range; //用户设置的油量异常变化范围检测值，例值为10，则表示油量瞬间下降10欧姆，则报警提示
    u8 flag;   //用户设置的油量空满箱最大值标志，0x01:即空箱为最大 0x02:满箱为最大值
    u16 box;   //用户设置的满箱有数量单位（升）
} OIL_CONFIG_STRUCT;

#ifdef USE_SYSTERM_BITS1_UNION
typedef union {
    u32 val;

    struct {
#ifdef USE_MID_TEST
        u32 lsm6ds3trc_ok_cnt : 2;
        u32 rx8025t_lsm6ds3trc_report : 1;
#endif

#if (1)
        u32 rx8025t_init : 1;
        u32 rx8025t_set_time : 1;
        u32 rx8025t_ok_cnt : 4;
#endif

        u32 flg_lsm_pwr_on : 1;
        u32 flg_lsm_read : 1;

        u32 flg_rk_1717_first : 1;//mcu开机后，是否第一次与RK通信【心跳】
    } bits;
} SYSTERM_BITS1_UNION;
extern SYSTERM_BITS1_UNION sys_bits1;
#endif

/*
(ZFZ_CUSTOMIZE)信为传感器SP11G01-XX-K1L2-XXP
根据正反转的到位时间进行对比 A 快为正转 B 快为反转；未检测到A B 则停止
*/

extern REGISTER_CONFIG_STRUCT register_cfg;
extern SYSTERM_CONFIG_STRUCT sys_cfg;

extern const char *str_app_build_time;

// extern void print_char(u8 chr);
// extern void print_buf(u8 *buf, u16 len);
// extern void print_gsm_char(u8 *buf, u16 len);

extern char *get_build_time_need_free(const char *ver);

extern void init_app_cfg(void);
//extern void custom_100ms_task(void);
// extern void food_dog(void);
extern void Voice_ReadMsg(u8 *str, u16 len);

#endif /* __APP_CFG_H */
