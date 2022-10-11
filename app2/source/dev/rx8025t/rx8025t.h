#ifndef __RX8025T__H__
#define __RX8025T__H__

// #include <rtthread.h>
// #include "gd32f30x_rcu.h"

/***********************************************************************************

		RX8025Tʵʱʱ����������(���ģ��IIC)

		�͸��� VR1.0 ���ڣ�2020-02-03
												 
		Ӳ�����ӣ�RX8025T_INT(PIN10)-->PB��RX8025T_SCL(PIN2)-->PB��RX8025T_SDA(PIN10)-->PB�� ע�⣺SDA��SCL������������衣
		
		BL8025T �Ĵӵ�ַΪ 7bit �̶������ݣ�0110 010����ͨ��ʱ���ӵ�ַ�Ǹ����� R/W �� 8bit ���ݷ��͵ġ�
		0110 0100Ϊдģʽ��0110 0101Ϊ��ģʽ����Ӧʮ����Ϊ��100��101����Ӧ16����Ϊ��0x64��0x65��
		
		BL8025T �е�ַ�Զ����ӹ��ܡ�ָ���Ĵӵ�ַһ����ʼ��֮��ֻ�������ֽڱ����͡�ÿ���ֽں�BL8025T �ĵ�ַ�Զ����ӡ�
		
***********************************************************************************/

#define RX8025T_SLAVE_ADDR 0x64

/* ********************************************************************** */
//RX8025T�Ĵ���
#define RX8025T_SEC_REG 0x00 //��
#define RX8025T_MIN_REG 0x01 //��
#define RX8025T_HOU_REG 0x02 //ʱ
#define RX8025T_WEE_REG 0x03 //���ڣ�bit0~bit7��Ӧ�ա�һ�����������ġ��塢������ӦֵΪ0x01,0x02,0x04,0x08,0x10,0x20,0x40�����ɳ���2λΪ1�������
#define RX8025T_DAY_REG 0x04 //����
#define RX8025T_MON_REG 0x05 //�·�
#define RX8025T_YEA_REG 0x06 //��
#define RX8025T_RAM_REG 0x07 //RAM
#define RX8025T_ALm_REG 0x08 //���ӷ֣������ǿ���Ϊramʹ�á�
#define RX8025T_ALh_REG 0x09 //����ʱ�������ǿ���Ϊramʹ�á�
#define RX8025T_ALw_REG 0x0a //�������ڣ������ǿ���Ϊramʹ�á�
#define RX8025T_CYl_REG 0x0b //���ڶ�ʱ���ĵ�8λ
#define RX8025T_CYm_REG 0x0c //���ڶ�ʱ���ĸ�4λ�����ڶ�ʱ������12λ��

#define RX8025T_EXT_REG 0x0d //��չ�Ĵ�����bit7-TEST=�������ԣ���Ӧ��д0��bit6-WADA=���ڻ���������ѡ��λ��bit5-USEL=ѡ�������Ӹ��´��������жϣ�0=����£�1=���Ӹ��£� \
                             //bit4-TE=���ڶ�ʱʹ�ܣ�bit3\2-FSEL1\0=оƬFOUT�������Ƶ��ѡ��λ��bit1\0-TSEL1\0=�����趨�̶����ڵ��ڲ�ʱ��Դ��

#define RX8025T_FLAG_REG 0x0e //��־�Ĵ�����bit5-UF��bit4-TF��bit3-AF���ֱ���ʱ������жϣ��̶����ڶ�ʱ�жϣ������жϵ��жϱ�־λ��bit1-VLF��ѹ�ͣ�bit0-VDET���ڵ�ѹ���²�ֹͣ������־λ��

#define RX8025T_CONT_REG 0x0f //���ƼĴ�����bit6~7(CSEL0��1)=�²�������ã�bit5(UIE)=ʱ������ж�ʹ��λ(����D�Ĵ�����USELλ����Ϊ1����»�1���Ӹ���)�� \
                              //bit4(TIE)=���ڶ�ʱʱ�����ж�ʹ��λ��bit3(AIE)=�����ж�ʹ��λ��bit0(RESET)=�ڲ���������λ������λȫ��Ϊ1��Ч��
/* ********************************************************************** */

typedef struct { // _TIME
    rt_uint8_t second;
    rt_uint8_t minute;
    rt_uint8_t hour;
    rt_uint8_t week;
    rt_uint8_t day;
    rt_uint8_t month;
    rt_uint8_t year;
} TIME_ST;

extern TIME_ST RTC_Time;       //�洢����ʱ��Ľṹ�������������ȫ�ֿ��á�
extern rt_uint8_t Time_Update; //ʱ����±�־

rt_err_t rx8025t_init(void);                                               //��ʼ��RX8025T_IIC��GPIO
rt_err_t Read8025TData(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len);  //��RX8025T�Ĵ�������
rt_err_t Write8025TData(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len); //дRX8025T�Ĵ�������
rt_err_t get_rtc_time(TIME_ST *t);                                         //��ȡʱ��
rt_err_t set_rtc_time(TIME_ST *t);                                         //����ʱ��
void rx8025t_show(TIME_ST *t);                                             //��ӡʱ��
rt_uint8_t BCD2HEX(rt_uint8_t bcd_data);                                   //BCD��ת��ֵ
rt_uint8_t HEX2BCD(rt_uint8_t hex_data);                                   //��ֵתBCD��
void RX8025T_Interrupt_Process(void);                                      //RX8025T�ж��¼�����

rt_err_t rx8025t_setRtcTime_test(void);

//void Time_CFG(void);             //��EC11���뿪������ʱ��
//void week_pro(rt_uint8_t weeks); //����ʱ��ʱ�����ڴ���
//void display(rt_uint8_t mode);   //ʱ����ʾ

#endif /* __RX8025T__H__ */
