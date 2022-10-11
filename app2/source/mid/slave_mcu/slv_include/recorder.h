#ifndef  __recorder_h__
#define  __recorder_h__
///#include "ckp_mcu_file.h"

#define   at24_i2c_clk               ((u32)50000)    //����ʱ��Ƶ��  50KHz
#define   at24_i2c_own_addr         ((u8)0xf1)      //�����ַ
#define   at24_i2c_slave_addr       ((u8)0xa0)      //��оƬ��ַ

#define   eeprom_start_addr    ((u8)0x00)  //оƬ�ڲ�����ʼ��ַ
#define   eeprom_data_lgth     ((u8)128)   //�����ܳ���


#define  print_speed_set      ((uint)180)    //��ӡ�ٶ�����
#define  print_auto_buff_lgth_max   ((uint)900)    //�Զ���ӡ��󻺳�������



typedef struct
{
	bit_enum  req_flag;    //����״̬	TRUEΪ����	 FALSEΪδ����

	uint   lgth;		   //����ӡ����
	uint   cnt; 		   //�Ѿ���ӡ�ĳ���
	uchar  buff[print_auto_buff_lgth_max];	//������
}
print_auto_data_struct;



//*****************************************************************************
//*****************************************************************************
//-----------                 �����ⲿ��������                -----------------
//-----------------------------------------------------------------------------

extern void recorder_init(void);

extern void load_print_data(u8* src, u16 lgth);
extern void print_auto_process(void);

#endif




