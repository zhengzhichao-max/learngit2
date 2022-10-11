/**
  ******************************************************************************
  * @file    gps_manage.h 
  * @author TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief GPS 模块管理
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

#define MAX_GPS_SIZE 100 //结构体限制32个字节
#define GPS_ADDR F_GPS_INFO_ADDR
#define _gps_addr(ptr) (GPS_ADDR + ptr * 36 * 1uL)

typedef struct {
    u16 crc;
    u16 flag;
    time_t ftime; //GPS 时间
    bool fixed;   // Fixed or Last known//已定位
    u8 if3d;
    point_t locate; //Latitude, Longitude
    u16 speed;      //速度，km/h
    u16 heading;    //方向
    u16 heigh;      //高度
    u8 fix_num;     //定位时，卫星颗数
    u32 pnumber;    // Packet number(0 ~ 4.2E9)
    u32 up_jifs;    //GPS 更新所对应的时间(单位ms )
} GPS_BASE_INFOMATION;

typedef struct {
    u16 crc;
    u16 flag;
    bool pwr_down; //模块电源断开
    bool reset;    //模块复位
    bool sleep;    //模块休眠
    bool res;
    bool lost_fix; //模块未定位
} GPS_EXT_INFOMATION;

//接口变量
#define gps_active() (gps_base.fixed == true)

extern bool positioned_update;
extern u16 save_gps_pos;
extern u16 display_speed;

extern GPS_BASE_INFOMATION gps_base; //GPS 基本位置信息
extern GPS_EXT_INFOMATION gps_ext;   //GPS 扩展信息
extern void save_gps_basemation(GPS_BASE_INFOMATION *p, time_t *t);

//接口函数
extern s32 get_standard_sec_num(time_t t1);
extern u8 *get_gps_field(u8 *pf, u8 n);
extern void config_gps_port(void);
extern void init_gps_info(void);

extern void gps_parse_task(void);

#endif /* __GPS_MANAGE_H */
