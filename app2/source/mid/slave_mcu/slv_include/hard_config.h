#ifndef  __hard_config_h__
#define  __hard_config_h__






//extern const u8  slave_app_ver[];


extern void system_config(void);

extern void tim3_config(u16 freq);

extern void lcd_bl_ctrl(bool en);

extern void read_flash_fonts(u16 addr, u8 *code_buff);

extern bit_enum ic_card_write_self(ic_card_date_struct *src);

extern void ex_flash_init(bool rst);
	
#endif







