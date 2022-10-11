/**
  ******************************************************************************
  * @file    mid_cfg.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V1.0.0
  * @date    2012-07-03
  * @brief   配置与管理中间层初始化和配置
  ******************************************************************************
  * @attention

  ******************************************************************************
*/
#ifndef __MID_CFG_H
#define __MID_CFG_H

extern bool car_login;
extern bool car_login2;

extern bool sms_phone_save;

extern bool add_struct_crc(u8 *p, u16 len);
extern bool verify_struct_crc(u8 *p, u16 len);

extern void save_run_parameterer(void);
extern void init_all_net_info(void);

extern void reset_system_parameter(u8 type);

extern void flash_write_import_parameters(void);
extern void flash_write_normal_parameters(void);
extern void init_run_parameterer(void);

extern void flash_write_tmp_parameters(void);
extern void init_s_authority_cfg(void);

#if (P_RCD == RCD_BASE)
extern void read_mfg_parameter(void);
extern void write_mfg_parameter(void);
extern void init_mfg_parameter(void);
#endif
extern void init_dvr_set_time(void);
extern void init_server_cfg(void);
extern void init_mid_cfg(void);
extern void synchronize_flash_spi_para(void);
extern void sms_ip_if_ok(void);
extern void init_alarm_cfg(void);
void init_car_cfg(void);

#endif /* __MID_CFG_H */
