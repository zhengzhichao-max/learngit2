#ifndef __menu_process_h__
#define __menu_process_h__

//�˵������
#define menu_max_depth 5

#define dir_east_range_low ((u16)(90 - 15))
#define dir_east_range_high ((u16)(90 + 15))
#define dir_south_range_low ((u16)(180 - 15))
#define dir_south_range_high ((u16)(180 + 15))
#define dir_west_range_low ((u16)(270 - 15))
#define dir_west_range_high ((u16)(270 + 15))

#define dir_north_range_low ((u16)(360 - 15))
#define dir_north_range_high ((u16)(0 + 15))

//*****************************************************************************
//*****************************************************************************
//-----------		        	������ֵ����    	          -
//#define    key_p         (0x1d)  //��ӡ
//#define    key_ok        (0x1e)  //ȷ��
//#define    key_up        (0x0f)  //����
//#define    key_down      (0x17)  //����
//#define    key_esc       (0x1b)  //�˵�/�˳�
#define key_ok (0x07)   //ȷ��
#define key_up (0x0d)   //����
#define key_down (0x0b) //����
#define key_esc (0x0e)  //�˵�/�˳�
#define key_p (0x1d)    //��ӡ

//*****************************************************************************
//*****************************************************************************

extern bool Ack_Export;
extern u8 Export_finish;

extern const u8 *menu_other_lib_dis[];

#define report_dis_sel(arr /* char* arr[][2]*/, zu, line /*0-1*/) ((unsigned char *)(*(*(arr + zu) + line)));
extern const u8 *menu_report_dis[][2];
extern const u8 *menu_1st_1st_4th_dis[];

extern void dis_goto_standby(void);
extern void menu_auto_exit_set(u32 tim, bool dir);

extern void menu_init(void);
extern void key_processing(void);
extern void menu_processing(void);
extern void low_power_hdl(void);
extern u8 date_to_week(u16 year, u8 month, u8 date);

extern char *my_itoa(int var);
extern void beeper(u8 OnOff);

//�ϵ��ж������Ƿ��޸Ĺ�
void pwr_up_password_status(void);

#endif
