/*
  ******************************************************************************
  * @file    app_cfg.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ����������м���ʼ��������
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

#ifndef __APP_CFG_H
#define __APP_CFG_H

typedef struct {
    u16 crc;
    u16 flag_t;
    u16 max;   //�û����õ������������
    u16 min;   //�û����õ���С��������
    u16 range; //�û����õ������쳣�仯��Χ���ֵ����ֵΪ10�����ʾ����˲���½�10ŷķ���򱨾���ʾ
    u8 flag;   //�û����õ��������������ֵ��־��0x01:������Ϊ��� 0x02:����Ϊ���ֵ
    u16 box;   //�û����õ�������������λ������
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

        u32 flg_rk_1717_first : 1;//mcu�������Ƿ��һ����RKͨ�š�������
    } bits;
} SYSTERM_BITS1_UNION;
extern SYSTERM_BITS1_UNION sys_bits1;
#endif

/*
(ZFZ_CUSTOMIZE)��Ϊ������SP11G01-XX-K1L2-XXP
��������ת�ĵ�λʱ����жԱ� A ��Ϊ��ת B ��Ϊ��ת��δ��⵽A B ��ֹͣ
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
