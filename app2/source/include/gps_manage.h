/**
  ******************************************************************************
  * @file    gps_manage.h 
  * @author TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GPS ģ�����
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef __GPS_MANAGE_H
#define __GPS_MANAGE_H

#define GPS_BAUD 9600
#define MAX_SPEED_LEN 6
#define MAX_COURSE_LEN 6

#define LEN_DATE 6
#define LEN_SIGN 1

#define LEN_TIME_MIN 6
#define LEN_LATD_MIN 9
#define LEN_LONG_MIN 10

#define LEN_TIME_MAX 10
#define LEN_LATD_MAX 11
#define LEN_LONG_MAX 12

#define F_GPS_INFO_ADDR (SECT_GPS_INFO_ADDR * 0x1000 * 1ul)

#define MAX_GPS_SIZE 100 //�ṹ������32���ֽ�
#define GPS_ADDR F_GPS_INFO_ADDR
#define _gps_addr(ptr) (GPS_ADDR + ptr * 36 * 1uL)

typedef struct {
    u16 crc;
    u16 flag;
    time_t ftime; //GPS ʱ��
    bool fixed;   // Fixed or Last known//�Ѷ�λ
    u8 if3d;
    point_t locate; //Latitude, Longitude
    u16 speed;      //�ٶȣ�km/h
    u16 heading;    //����
    u16 heigh;      //�߶�
    u8 fix_num;     //��λʱ�����ǿ���
    u32 pnumber;    // Packet number(0 ~ 4.2E9)
    u32 up_jifs;    //GPS ��������Ӧ��ʱ��(��λms )
} GPS_BASE_INFOMATION;

typedef struct {
    u16 crc;
    u16 flag;
    bool pwr_down; //ģ���Դ�Ͽ�
    bool reset;    //ģ�鸴λ
    bool sleep;    //ģ������
    bool res;
    bool lost_fix; //ģ��δ��λ
} GPS_EXT_INFOMATION;

//�ӿڱ���
#define gps_active() (gps_base.fixed == true)

extern bool positioned_update;
extern u16 save_gps_pos;
extern u16 display_speed;

extern GPS_BASE_INFOMATION gps_base; //GPS ����λ����Ϣ
extern GPS_EXT_INFOMATION gps_ext;   //GPS ��չ��Ϣ
extern void save_gps_basemation(GPS_BASE_INFOMATION *p, time_t *t);

//�ӿں���
extern s32 get_standard_sec_num(time_t t1);
extern u8 *get_gps_field(u8 *pf, u8 n);
extern void config_gps_port(void);
extern void init_gps_info(void);

extern void gps_parse_task(void);

#endif /* __GPS_MANAGE_H */
