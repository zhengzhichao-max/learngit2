/********************************************************************************
  * @file       rcd_communicate.c
  * @author  TRWY_TEAM
  * @Email
  * @version  V1.0.0
  * @date     2013-12-01
  * @brief   行驶记录仪信息管理
 1. 本文件包括记录仪数据管理，初始化，指针管理等
    ********************************************************************************/
#include "include_all.h"

#if (P_RCD == RCD_BASE)
//行驶记录仪控制变量
static u16 save_rcd360_p_pos = RCD360_P_SAVE_SIZE; //指针保存位置，循环保存
S_RCD_360HOUR_MANAGE_STRUCT rcd360_m;              //360 小时位置信息管理

static u16 save_rcd48_p_pos = RCD48_P_SAVE_SIZE; //指针保存位置，循环保存
S_RCD48_SPEED_MANAGE_STRUCT rcd48_m;             //48小时平均速度信息管理

static u16 save_rcd_accident_p_pos = RCD_ACCIDENT_P_SAVE_SIZE;           //指针保存位置，循环保存
S_RCD_ACCIDENT_MANAGE_STRUCT rcd_accident_m;                             //事故疑点信息管理
S_RCD_ACCIDENT_INFOMATION rcd_accident_cell[RCD_ACCIDENT_CELL_SIZE + 1]; //事故点信息，环形当前速度信息
static s16 accident_head = 0;                                            //事故点头指针
static s16 accident_tail = 0;                                            //事故点尾指针
static bool accident_offset = false;                                     //事故点偏移，去除停车判断时间3 秒

S_RCD_OT_MANAGE_STRUCT rcd_ot_m;                   //疲劳记录管理
RCD_OT_INFOMATION_STRUCT rcd_ot_inf;               //疲劳记录信息
static u16 save_rcd_ot_p_pos = RCD_OT_P_SAVE_SIZE; //指针保存位置，循环保存

S_RCD_DRIVER_MANAGE_STRUCT rcd_driver_m;                   //驾驶员记录管理
static u16 save_rcd_driver_p_pos = RCD_DRIVER_P_SAVE_SIZE; //指针保存位置，循环保存

S_RCD_POWER_MANAGE_STRUCT rcd_power_m;                   //电源记录管理
static u16 save_rcd_power_p_pos = RCD_POWER_P_SAVE_SIZE; //指针保存位置，循环保存

S_RCD_PARA_MANAGE_STRUCT rcd_para_m;                   //参数记录管理
RCD_PARA_INFOMATION_STRUCT rcd_para_inf;               //参数记录信息
static u16 save_rcd_para_p_pos = RCD_PARA_P_SAVE_SIZE; //指针保存位置，循环保存

SPEED_STATE speed_state;                                 //速度状态
SPEED_TIME speed_message;                                //速度记录信息
S_RCD_SPEED_STATE_MANAGE_STRUCT rcd_speed_m;             //速度状态记录管理
static u16 save_rcd_speed_p_pos = RCD_SPEED_P_SAVE_SIZE; //指针保存位置，循环保存

U_RCD_STATUS rcd_io; //行驶记录仪IO状态口//bit7: 刹车bit6: 左转bit5:  右转bit4: 远光灯bit3:近光灯

s_DOUBTSpeed stop_average_speed;  //停车前15分钟平均车速
ALL_FATIGUE_STRUCT fatigue_print; //疲劳信息打印
AVERAGE_SPEED_STRUCT speed_print; //速度信息打印

REMEB_POS pos48;
REMEB_POS pos360;

LEFT_RIGHT_IO left_io;
LEFT_RIGHT_IO right_io;

RCD_DATA_INPORT_STRUCT data_inport_para;

extern u32 CopyU32ToU8(u32 x, u8 *pc);

//宸﹀彸杞?悜銆佸�掕溅淇″彿鐙?珛鏀惧ぇ寤惰繜3绉?
void control_left_right_mess(void) {
    if (left_io.touch) {
        if (rcd_io.b.right_light) {
            _memset((u8 *)&left_io, 0, sizeof(left_io));
            tr9_car_status.bit.left = false;
        }
        if (_covern(left_io.tmr) > 3) {
            _memset((u8 *)&left_io, 0, sizeof(left_io));
            tr9_car_status.bit.left = false;
        }
    }
    if (right_io.touch) {
        if (rcd_io.b.left_light) {
            _memset((u8 *)&right_io, 0, sizeof(right_io));
            tr9_car_status.bit.right = false;
        }
        if (_covern(right_io.tmr) > 3) {
            _memset((u8 *)&right_io, 0, sizeof(right_io));
            tr9_car_status.bit.right = false;
        }
    }
}

/****************************************************************************
* 名称:    check_rcd_io ()
* 功能：IO 管脚高低电平检测
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void check_rcd_io(void) {
    static u8 cnt_b1 = 0;
    static u8 cnt_b2 = 0;
    static u8 cnt_b3 = 0;
    static u8 cnt_b4 = 0;
    static u8 cnt_b5 = 0;
//    static u8 cnt_b6 = 0;
    //    static u8 cnt_b7 = 0;

    // if ( tr9_car_status.bit.left = false )
    if (rcd_io.b.left_light == false) {
        if (PIN_I1_HIGH == _HIGH) //海格:左转
        {
            if (++cnt_b1 >= KEEPCNT) {
                rcd_io.b.left_light = true;
                tr9_car_status.bit.left = true;
            }
        } else
            cnt_b1 = 0;
    } else //if( tr9_car_status.bit.left = true )
    {
        if (PIN_I1_HIGH == _LOW) {
            if (++cnt_b1 >= KEEPCNT) {
                rcd_io.b.left_light = false;
                tr9_car_status.bit.left = false;
            }
        } else
            cnt_b1 = 0;
    }

    // if (rcd_io.b.right_light == false)
    if (tr9_car_status.bit.right == false) {
        if (PIN_I2_HIGH == _HIGH) //海格：右转
        {
            if (++cnt_b2 >= KEEPCNT) {
                rcd_io.b.right_light = true;
                tr9_car_status.bit.right = true;
            }
        } else
            cnt_b2 = 0;
    } else {
        if (PIN_I2_HIGH == _LOW) {
            if (++cnt_b2 >= KEEPCNT) {
                rcd_io.b.right_light = false;
                tr9_car_status.bit.right = false;
            }
        } else
            cnt_b2 = 0;
    }

    //if (rcd_io.b.near_light == false) //近光灯
    if (tr9_car_status.bit.far_light == false) {
        if (PIN_I3_HIGH == _HIGH) //海格：远光灯
        {
            if (++cnt_b3 >= KEEPCNT) {
                rcd_io.b.near_light = true;
                tr9_car_status.bit.far_light = true;
                //logd("打开远光");
            }
        } else
            cnt_b3 = 0;
    } else {
        if (PIN_I3_HIGH == _LOW) {
            if (++cnt_b3 >= KEEPCNT) {
                rcd_io.b.near_light = false;
                tr9_car_status.bit.far_light = false;
                //logd("关闭远光");
            }
        } else {
            cnt_b3 = 0;
        }
    }

    // if (rcd_io.b.far_light == false) //远光灯
    if (tr9_car_status.bit.near_light == false) {
        if (PIN_I4_HIGH == _HIGH) //海格：近光灯
        {
            if (++cnt_b4 >= KEEPCNT) {
                rcd_io.b.far_light = true;
                tr9_car_status.bit.near_light = true;
            }
        } else
            cnt_b4 = 0;
    } else {
        if (PIN_I4_HIGH == _LOW) {
            if (++cnt_b4 >= KEEPCNT) {
                rcd_io.b.far_light = false;
                tr9_car_status.bit.near_light = false;
            }
        } else {
            cnt_b4 = 0;
        }
    }

    //if (rcd_io.b.brake == false)  //刹车
    if (false == tr9_car_status.bit.brake) {
        if (BRAKE == _HIGH) {
            if (++cnt_b5 >= KEEPCNT) {
                rcd_io.b.brake = true;
                tr9_car_status.bit.brake = true;
                //logd("刹车");
            }
        } else
            cnt_b5 = 0;
    } else {
        if (BRAKE == _LOW) {
            if (++cnt_b5 >= KEEPCNT) {
                rcd_io.b.brake = false;
                tr9_car_status.bit.brake = false;
                //logd("取消刹车");
            }
        } else {
            cnt_b5 = 0;
        }
    }
    /*
	//倒车
    if (rcd_io.b.backing == false) //右转
    {
        if (PIN_H6_BACK == _HIGH)
        {
        	
            if (++cnt_b6 >= KEEPCNT)
            {
                rcd_io.b.backing = true;
				
            }
        }
        else
            cnt_b6 = 0;
    }
    else
    {
        if (PIN_H6_BACK == _LOW)
        {
            if (++cnt_b6 >= KEEPCNT)
            {
                rcd_io.b.backing = false;
				//tr9_car_status.bit.back = false;
		
            }
        }
        else
        {
            cnt_b6 = 0;
        }
    }	
    */
#if 0
    if (pub_io.b.high2 == false) //右转
    {
        if (PIN_I7_HIGH == _HIGH)
        {
            if (++cnt_b7 >= KEEPCNT)
            {
                pub_io.b.high2= true;
                tr9_car_status.bit.high6 = true;
				
            }
        }
        else
            cnt_b7 = 0;
    }
    else
    {
        if (PIN_I7_HIGH == _LOW)
        {
            if (++cnt_b7 >= KEEPCNT)
            {
                pub_io.b.high2= false;
                tr9_car_status.bit.high6 = false;
		
            }
        }
        else
        {
            cnt_b7 = 0;
        }
    }
#endif
}

/****************************************************************************
* 名称:    init_printf_rcd_info ()
* 功能：初始化疲劳打印的信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_printf_rcd_info(void) {
    _memset((u8 *)&fatigue_print, 0x00, sizeof(fatigue_print));
    _memset((u8 *)&stop_average_speed, 0x00, sizeof(stop_average_speed));
    _memset((u8 *)&speed_print, 0x00, sizeof(speed_print));
}

/****************************************************************************
* 名称:    incp_rcd_p ()
* 功能：指针管理，指针加
* 入口参数：无
* 出口参数：无
****************************************************************************/
s16 incp_rcd_p(u8 type, u16 index, u16 n) {
    u16 index_t;

    index_t = 0;
    if (type == T_RCD_360) {
        if (index >= RCD360_NODE_MAX_SIZE || n >= RCD360_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD360_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD360_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_48) {
        if (index >= RCD48_NODE_MAX_SIZE || n >= RCD48_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD48_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD48_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_ACCIDENT_CELL) {
        if (index >= RCD_ACCIDENT_CELL_SIZE || n >= RCD_ACCIDENT_CELL_SIZE)
            return 0;
        if ((index + n) >= RCD_ACCIDENT_CELL_SIZE) {
            index_t = (index + n) - RCD_ACCIDENT_CELL_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_ACCIDENT) {
        if (index >= RCD_ACCIDENT_NODE_MAX_SIZE || n >= RCD_ACCIDENT_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD_ACCIDENT_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD_ACCIDENT_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_DRIVER_OT) {
        if (index >= RCD_OT_NODE_MAX_SIZE || n >= RCD_OT_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD_OT_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD_OT_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_DRIVER_LOG) {
        if (index >= RCD_DRIVER_NODE_MAX_SIZE || n >= RCD_DRIVER_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD_DRIVER_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD_DRIVER_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_POWER_LOG) {
        if (index >= RCD_POWER_NODE_MAX_SIZE || n >= RCD_POWER_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD_POWER_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD_POWER_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_PARAMETER_LOG) {
        if (index >= RCD_PARA_NODE_MAX_SIZE || n >= RCD_PARA_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD_PARA_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD_PARA_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else if (type == T_RCD_SPEED_LOG) {
        if (index >= RCD_SPEED_NODE_MAX_SIZE || n >= RCD_SPEED_NODE_MAX_SIZE)
            return 0;
        if ((index + n) >= RCD_SPEED_NODE_MAX_SIZE) {
            index_t = (index + n) - RCD_SPEED_NODE_MAX_SIZE;
        } else {
            index_t = index + n;
        }
    } else {
        return 0;
    }

    return index_t;
}

/****************************************************************************
* 名称:    decp_rcd_p ()
* 功能：指针管理，指针减
* 入口参数：无
* 出口参数：无
****************************************************************************/
s16 decp_rcd_p(u8 type, u16 index, u16 n) {
    u16 index_t;

    index_t = 0;
    if (type == T_RCD_360) {
        if (index >= RCD360_NODE_MAX_SIZE || n >= RCD360_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD360_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_48) {
        if (index >= RCD48_NODE_MAX_SIZE || n >= RCD48_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD48_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_ACCIDENT_CELL) {
        if (index >= RCD_ACCIDENT_CELL_SIZE || n >= RCD_ACCIDENT_CELL_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD_ACCIDENT_CELL_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_ACCIDENT) {
        if (index >= RCD_ACCIDENT_NODE_MAX_SIZE || n >= RCD_ACCIDENT_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD_ACCIDENT_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_DRIVER_OT) {
        if (index >= RCD_OT_NODE_MAX_SIZE || n >= RCD_OT_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD_OT_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_DRIVER_LOG) {
        if (index >= RCD_DRIVER_NODE_MAX_SIZE || n >= RCD_DRIVER_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD_DRIVER_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_POWER_LOG) {
        if (index >= RCD_POWER_NODE_MAX_SIZE || n >= RCD_POWER_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD_POWER_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_PARAMETER_LOG) {
        if (index >= RCD_PARA_NODE_MAX_SIZE || n >= RCD_PARA_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD_PARA_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else if (type == T_RCD_SPEED_LOG) {
        if (index >= RCD_SPEED_NODE_MAX_SIZE || n >= RCD_SPEED_NODE_MAX_SIZE)
            return 0;
        if (index < n) {
            index_t = (index + RCD_SPEED_NODE_MAX_SIZE) - n;
        } else {
            index_t = index - n;
        }
    } else {
        return 0;
    }

    return index_t;
}

/***********************************************************
函数名称: average_print_order
功    能: 停车前15分钟平均速度打印前 排序
输入参数:
输出参数:
注意事项: 打印前调用
***********************************************************/
void average_print_order(void) {
    u8 i;
    u8 index;

    if (stop_average_speed.uCnt >= 15)
        stop_average_speed.uCnt = 0;

    if (stop_average_speed.uCnt == 0)
        index = 14;
    else
        index = stop_average_speed.uCnt - 1;

    for (i = 0; i < 15; i++) //进行排序
    {
        speed_print.HMt[i].year = stop_average_speed.HMt[index].year;
        speed_print.HMt[i].month = stop_average_speed.HMt[index].month;
        speed_print.HMt[i].day = stop_average_speed.HMt[index].day;
        speed_print.HMt[i].hour = stop_average_speed.HMt[index].hour;
        speed_print.HMt[i].minute = stop_average_speed.HMt[index].minute;
        speed_print.HMt[i].speed = stop_average_speed.HMt[index].speed;

        if (index > 0)
            index--;
        else
            index = 14;
    }
    stop_average_speed.fill = true;
}

/****************************************************************************
* 名称:    calculate_minute_average_speed ()
* 功能：计算一分钟内的平均速度
* 入口参数：无
* 出口参数：无
****************************************************************************/
void calculate_minute_average_speed(void) {
    //	static bool moving = false;
    //	static bool flag = false ;

    static u8 bak_sec;
    static u8 bak_min;
    static u8 total_cnt = 0;
    //static u8 speedRecord[60] = {0};

    static u32 total_speed = 0;
#if 0 //chwsh
    if (!gps_base.fixed)
        return;
#endif

#if 0	
    if (!mix.moving)
    {
	flag = false ;
	total_cnt = 0 ;
	total_speed = 0;
	mix.average_speed = 0;
	memset(speedRecord,0x00,60);			
         return ;  
    }
#endif

    //else
    //    moving = true;

    if (bak_sec == sys_time.sec)
        return;
    bak_sec = sys_time.sec;
#if 0
	//最近一分钟的平均速度
	
	total_speed -= speedRecord[total_cnt ] ;  			
	speedRecord[total_cnt] = mix.complex_speed ;		
	total_speed += mix.complex_speed;				
	total_cnt = (total_cnt + 1)%60 ;					

	if(flag)
		mix.average_speed = total_speed / 60;	
	else
		mix.average_speed = total_speed / total_cnt;

#else //海格要求:平均速度应为:某一分钟的0秒到59秒的和除以60得到

    total_cnt++;
    total_speed += mix.complex_speed;

    if (59 == sys_time.sec) {
        mix.average_speed = total_speed / total_cnt;

        total_cnt = 0;
        total_speed = 0;
    }

#endif

    if (mix.average_speed > 180) {
        mix.average_speed = 180;
    } else if (mix.average_speed < 1) {
        mix.average_speed = 0;
    }

    if (bak_min == sys_time.min) {
        return;
    }

    bak_min = sys_time.min;
    //   moving = false;
#if 1 //chwsh
    if (!gps_base.fixed)
        return;
#endif

    if (stop_average_speed.uCnt >= 15)
        stop_average_speed.uCnt = 0;
    stop_average_speed.HMt[stop_average_speed.uCnt].hour = sys_time.hour;
    stop_average_speed.HMt[stop_average_speed.uCnt].minute = sys_time.min;
    stop_average_speed.HMt[stop_average_speed.uCnt].speed = mix.average_speed;
    stop_average_speed.uCnt++;
}

/****************************************************************************
* 名称:    rcd48_fill_in_infomation ()
* 功能：填充记录仪两个日历天平均车速信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd48_fill_in_infomation(u8 type, S_RCD48_SPEED_INFOMATION *f) {
    if (type == RCD48_FILL_NORMAL) {
#if 0
       // f->speed = mix.average_speed;
       // f->sig.reg = rcd_io.reg;
#else
        f->speed = mix.complex_speed;
        f->sig.reg = rcd_io.reg;
#endif
    } else if (type == RCD48_FILL_NULL) {
#if 0
        f->speed = mix.complex_speed;
        f->sig.reg = rcd_io.reg;
#else
        f->speed = mix.complex_speed;
        f->sig.reg = rcd_io.reg;
#endif

    } else {
        f->speed = 0;
        f->sig.reg = 0;
    }
}

/****************************************************************************
* 名称:    save_rcd48_pointer ()
* 功能：保存环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void save_rcd48_pointer(void) {
    u8 buf[128];

    if (save_rcd48_p_pos >= RCD48_P_SAVE_SIZE - 1) {
        spi_flash_erase((SECT_RCD48_P_ADDR * 0x1000 * 1ul));
        save_rcd48_p_pos = 0;
    } else {
        _memset(buf, 0x00, 128);
        spi_flash_write(buf, _rcd48_p_addr(save_rcd48_p_pos), sizeof(rcd48_m));
        save_rcd48_p_pos++;
    }

    add_struct_crc((u8 *)&rcd48_m, sizeof(rcd48_m));
    spi_flash_write((u8 *)&rcd48_m, _rcd48_p_addr(save_rcd48_p_pos), sizeof(rcd48_m));
}

/****************************************************************************
* 名称:    search_rcd48_pointor ()
* 功能：寻找环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd48_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD48_SPEED_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD48_P_SAVE_SIZE;

    while (low <= high) {
    search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, _rcd48_p_addr(mid), sizeof(ps));
        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            save_rcd48_p_pos = mid;
            if (ps.head.node > RCD48_NODE_MAX_SIZE || ps.tail.node > RCD48_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd48_m, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    init_rcd48_pointer ()
* 功能：初始化环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
//static void init_rcd48_pointer(void)
void init_rcd48_pointer(void) {
    save_rcd48_p_pos = RCD48_P_SAVE_SIZE;

    _memset((u8 *)&rcd48_m, 0x00, sizeof(rcd48_m));

    rcd48_m.h_time.year = sys_time.year;
    rcd48_m.h_time.month = sys_time.month;
    rcd48_m.h_time.date = sys_time.date;
    rcd48_m.h_time.hour = sys_time.hour;
    rcd48_m.h_time.min = sys_time.min;
    rcd48_m.h_time.sec = sys_time.sec;

    rcd48_m.init = true;

    spi_flash_erase(_rcd48_t_addr(rcd48_m.head.node));

    save_rcd48_pointer();
}

/****************************************************************************
* 名称:    judge_rcd48_save ()
* 功能：判断记录仪两个日历天信息是否需要保存
* 入口参数：无
* 出口参数：无
****************************************************************************/

/****************************************************************************
* 名称:    judge_rcd48_save ()
* 功能：启动数据保存判断
* 记录仪存储偏差
* 入口参数：无
* 出口参数：无
****************************************************************************/
static s16 judge_rcd48_save(void) {
    static u8 sec = 0;
    // static u8 io = 0;
    // static bool last = false;

    //考虑到GPS 无效时，速度或当前时间是无效值，程序直接过滤
    //#ifndef DEBUG_RCD
    //  if (!mix.fixed)
    //        return 0;
    //#endif
    //一秒存储一次

    if (sec == sys_time.sec)
        return 0;
    sec = sys_time.sec;

    /*
    if ((mix.complex_speed == 0) && (io == rcd_io.reg))
    {
        if (last == false)
            return 0;
        last = false;
    }
    if (mix.complex_speed > 0)
        last = true;
*/
    rcd48_m.h_time = sys_time;
    // io = rcd_io.reg;

    return -1; //正常写
}

#if(0)
/****************************************************************************
* 名称:    rcd48_data_write ()
* 功能：保存记录仪两个日历天平均车速信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd48_data_inport_write(u8 *p, u16 len) {
    static u8 bak_min = 0xff;
    u16 index_t = 0;

    _memcpy((u8 *)&rcd48_m.h_time, (u8 *)&data_inport_para.start, sizeof(sys_time));

    if (data_inport_para.start.sec == 0 || bak_min != data_inport_para.start.min || rcd48_m.init == true) //一分钟后进入
    {
        bak_min = data_inport_para.start.min;
        rcd48_m.init = false;

        rcd48_m.head.node = incp_rcd_p(T_RCD_48, rcd48_m.head.node, 1); //指针加加
        if ((rcd48_m.head.node % 30) == 0) {
            spi_flash_erase(_rcd48_t_addr(rcd48_m.head.node));
            index_t = subp_rcd_p(rcd48_m.head.node, rcd48_m.tail.node, RCD48_NODE_MAX_SIZE); //计算大小
            if (index_t > (RCD48_NODE_MAX_SIZE - 60)) {
                //超界
                rcd48_m.tail.node = incp_rcd_p(T_RCD_48, rcd48_m.tail.node, 30);
            }
        }

        rcd48_m.h_time.sec = 0;
        spi_flash_write((u8 *)&rcd48_m.h_time, _rcd48_t_addr(rcd48_m.head.node), sizeof(time_t));
    }

    spi_flash_write(p, _rcd48_d_addr(rcd48_m.head.node, data_inport_para.start.sec), len);

    save_rcd48_pointer();
    return true;
}
#endif

/****************************************************************************
* 名称:    rcd48_data_write ()
* 功能：保存记录仪两个日历天平均车速信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd48_data_write(u8 *p, u16 len) {
    static u8 bak_min = 0xff;
    u16 index_t = 0;

    _memcpy((u8 *)&rcd48_m.h_time, (u8 *)&sys_time, sizeof(sys_time));

    if (sys_time.sec == 0 || bak_min != sys_time.min || rcd48_m.init == true) //一分钟后进入
    {
        bak_min = sys_time.min;
        rcd48_m.init = false;

        rcd48_m.head.node = incp_rcd_p(T_RCD_48, rcd48_m.head.node, 1); //指针加加
        if ((rcd48_m.head.node % 30) == 0) {
            spi_flash_erase(_rcd48_t_addr(rcd48_m.head.node));
            index_t = subp_rcd_p(rcd48_m.head.node, rcd48_m.tail.node, RCD48_NODE_MAX_SIZE); //计算大小
            if (index_t > (RCD48_NODE_MAX_SIZE - 60)) {
                //超界
                rcd48_m.tail.node = incp_rcd_p(T_RCD_48, rcd48_m.tail.node, 30);
            }
        }

        rcd48_m.h_time.sec = 0;
        spi_flash_write((u8 *)&rcd48_m.h_time, _rcd48_t_addr(rcd48_m.head.node), sizeof(time_t));
    }

    spi_flash_write(p, _rcd48_d_addr(rcd48_m.head.node, sys_time.sec), len);

    save_rcd48_pointer();
    return true;
}

/****************************************************************************
* 名称:    rcd48_speed_manage ()
* 功能：管理记录仪两个日历天平均车速信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd48_speed_manage(void) {
    static enum E_LINK_MANAGE {
        E_48_IDLE,
        E_48_WRITE_NORMAL,
        E_48_ERR
    } step = E_48_IDLE;

    s16 ret1 = 0;
    //	static u32 sec =0 ;
    static S_RCD48_SPEED_INFOMATION rcd48_inf;

    switch (step) {
    case E_48_IDLE:
        ret1 = judge_rcd48_save();
        if (ret1 == -1) {
//            sec = sys_time.sec;
            step = E_48_WRITE_NORMAL;
            break;
        }
        break;
    case E_48_WRITE_NORMAL:
        rcd48_fill_in_infomation(RCD48_FILL_NORMAL, &rcd48_inf); //记录速度
        rcd48_data_write((u8 *)&rcd48_inf, 2);                   //保存速度
        step = E_48_IDLE;
        break;
    case E_48_ERR:
        step = E_48_IDLE;
        break;
    default:
        step = E_48_IDLE;
    }
}

/****************************************************************************
* 名称:    rcd360_fill_in_infomation ()
* 功能：填充记录仪位置信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
void rcd360_fill_in_infomation(u8 type, u8 *f) {
    S_RCD_360HOUR_INFOMATION inf_t;

    if (type == RCD360_FILL_NORMAL) //如果是正常的数据
    {
        inf_t.locate.lati = _jtb_to_gb(mix.locate.lati); //保存纬度
        inf_t.locate.lngi = _jtb_to_gb(mix.locate.lngi); //保存经度
        inf_t.heigh = mix.heigh;
        inf_t.speed = (u8)mix.average_speed; //当前平均速度
        memcpy(f, (u8 *)&inf_t, 11);
    } else //如果非正常数据,用 0x7fffffff 来填充
    {
        inf_t.locate.lati = 0x7fffffff;
        inf_t.locate.lngi = 0x7fffffff;
        inf_t.heigh = 0x7fff;
        inf_t.speed = mix.average_speed;
        memcpy(f, (u8 *)&inf_t, 11);
    }
}

/****************************************************************************
* 名称:    save_rcd360_pointer ()
* 功能：保存环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
void save_rcd360_pointer(void) {
    u8 buf[128];

    if (save_rcd360_p_pos >= RCD360_P_SAVE_SIZE - 1) //如果环形指针保存到最大值时(99),将扇区擦除,开始下一个循环存储
    {                                                //如果指针计数记录 大于 最大记录 100 // save_rcd360_p_pos 初始化是 100,故一开始就删除扇区
        spi_flash_erase((SECT_RCD360_P_ADDR * 0x1000 * 1ul));
        save_rcd360_p_pos = 0;
    } else {
        _memset(buf, 0x00, 128);
        spi_flash_write(buf, _rcd360_p_addr(save_rcd360_p_pos), sizeof(rcd360_m));
        save_rcd360_p_pos++;
    }

    add_struct_crc((u8 *)&rcd360_m, sizeof(rcd360_m)); //结构体校验
    spi_flash_write((u8 *)&rcd360_m, _rcd360_p_addr(save_rcd360_p_pos), sizeof(rcd360_m));
}

/****************************************************************************
* 名称:    search_rcd360_pointor ()
* 功能：搜索环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd360_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD_360HOUR_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD360_P_SAVE_SIZE;

    while (low <= high) {
    search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, _rcd360_p_addr(mid), sizeof(ps));
        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            save_rcd360_p_pos = mid;
            if (ps.head.node > RCD360_NODE_MAX_SIZE || ps.tail.node > RCD360_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd360_m, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    init_rcd360_pointer ()
* 功能：初始化环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
void init_rcd360_pointer(void) {
    u16 node;
    u16 i;
    save_rcd360_p_pos = RCD360_P_SAVE_SIZE;

    _memset((u8 *)&rcd360_m, 0x00, sizeof(rcd360_m));

    rcd360_m.h_time.year = sys_time.year;
    rcd360_m.h_time.month = sys_time.month;
    rcd360_m.h_time.date = sys_time.date;
    rcd360_m.h_time.hour = sys_time.hour;
    rcd360_m.h_time.min = sys_time.min;
    rcd360_m.h_time.sec = 0;

    rcd360_m.init = true;

    spi_flash_erase(_rcd360_t_addr(rcd360_m.head.node));
    node = rcd360_m.head.node;
    for (i = 0; i < 4; i++) {
        node = incp_rcd_p(T_RCD_360, node, 1);
        spi_flash_erase(_rcd360_t_addr(node));
    }
    save_rcd360_pointer();
}

/****************************************************************************
* 名称:    judge_rcd360_save ()
* 功能：判断数据保存
* 入口参数：无
* 出口参数：无
****************************************************************************/
s16 judge_rcd360_save(void) {
    static u32 o_min = 0;
    //  static bool last = true;
#if (1)                     //chwsh
    if (mix.fixed == false) //如果没有定位
        return 0;
#endif
    if (o_min == sys_time.min)
        return 0;
    o_min = sys_time.min;

#if (1)
    return -1; //正常写
#else
    // if (mix.moving == true || last == true)
    {
        //last = (mix.moving == true) ? true : false;
        return -1; //正常写
    }

    return 0;
#endif
}

/****************************************************************************
* 名称:    rcd360_data_write ()
* 功能：位置信息写
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool rcd360_data_write(u8 *p, u16 len) {
    u16 index_t;
    static u8 hour = 0xff;

    /*这里需要注意!!!! 此处的时间是16进制数据,而非BCD码哦......*/
    if (((rcd360_m.h_time.hour != sys_time.hour) && (sys_time.min == 00 || hour != sys_time.hour)) || rcd360_m.init == true) {
        rcd360_m.init = false;
        hour = sys_time.hour;

        rcd360_m.head.node = incp_rcd_p(T_RCD_360, rcd360_m.head.node, 1);
        if (((rcd360_m.head.node % 6) == 0)) { //判断是否为下一页
            spi_flash_erase(_rcd360_t_addr(rcd360_m.head.node));
            /*计算出头尾指针的差值, 即有效数据条数, 这里一个节点是以小时为单位,共计666个字节*/
            /*重点注意该函数*/
            index_t = subp_rcd_p(rcd360_m.head.node, rcd360_m.tail.node, RCD360_NODE_MAX_SIZE);

            if (index_t > (RCD360_NODE_MAX_SIZE - 12)) {
                rcd360_m.tail.node = incp_rcd_p(T_RCD_360, rcd360_m.tail.node, 6);
            }
        }
        /*.....注意这里记录的时间不是BCD码.....*/
        _memcpy((u8 *)&rcd360_m.h_time, (u8 *)&sys_time, sizeof(sys_time));
        rcd360_m.h_time.sec = 00;
        rcd360_m.h_time.min = 0;
        /*先写下一小时的时间, 注意 1 pagesize 能保存 6小时数据 */
        spi_flash_write((u8 *)&rcd360_m.h_time, _rcd360_t_addr(rcd360_m.head.node), sizeof(time_t));
    }
    //保存一条位置信息数据,11Byte,(经度,纬度,海拔,平均速度)
    spi_flash_write(p, _rcd360_d_addr(rcd360_m.head.node, sys_time.min), RCD360_EVERY_INF_SIZE);

    save_rcd360_pointer();
    return true;
}

/****************************************************************************
* 名称:    rcd360_position_manage ()
* 功能：管理记录仪位置信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd360_position_manage(void) {
    static enum E_LINK_MANAGE {
        E_360_IDLE,
        E_360_WRITE_NORMAL,
        E_360_ERR
    } step = E_360_IDLE;

    s16 ret1 = 0;
    static s32 min = 0;
    static u8 rcd360_inf[15] = {0}; //经纬度、高程、速度

    switch (step) {
    case E_360_IDLE:

        ret1 = judge_rcd360_save(); //判断本分钟的位置数据是否保存.
        if (ret1 == -1) {           //未保存 则执行保存
            min = sys_time.min;
            step = E_360_WRITE_NORMAL;
            rcd360_fill_in_infomation(RCD360_FILL_NORMAL, rcd360_inf);
            break;
        } else {
            if ((min != sys_time.min) && (!mix.fixed)) {
                min = sys_time.min;
                step = E_360_WRITE_NORMAL;
                rcd360_fill_in_infomation(RCD360_FILL_NULL, rcd360_inf);
            }
        }
        break;
    case E_360_WRITE_NORMAL:
        //   rcd360_fill_in_infomation(RCD360_FILL_NORMAL, &rcd360_inf);
        rcd360_data_write(rcd360_inf, 11);
        step = E_360_IDLE;
        break;
    case E_360_ERR:
        step = E_360_IDLE;
        break;
    default:
        step = E_360_IDLE;
    }
}

/****************************************************************************
* 名称:    save_rcd_accident_pointer ()
* 功能：保存环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void save_rcd_accident_pointer(void) {
    u8 buf[128];

    if (save_rcd_accident_p_pos >= RCD_ACCIDENT_P_SAVE_SIZE - 1) {
        spi_flash_erase((SECT_RCD_ACCIDENT_P_ADDR * 0x1000 * 1ul));
        save_rcd_accident_p_pos = 0;
    } else {
        _memset(buf, 0x00, 128);
        spi_flash_write(buf, ACCI_ADDRP(save_rcd_accident_p_pos), sizeof(rcd_accident_m));
        save_rcd_accident_p_pos++;
    }

    add_struct_crc((u8 *)&rcd_accident_m, sizeof(rcd_accident_m));
    spi_flash_write((u8 *)&rcd_accident_m, ACCI_ADDRP(save_rcd_accident_p_pos), sizeof(rcd_accident_m));
}

/****************************************************************************
* 名称:    search_rcd_accident_pointor ()
* 功能：寻找环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd_accident_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD_ACCIDENT_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD_ACCIDENT_P_SAVE_SIZE;

    while (low <= high) {
    search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, ACCI_ADDRP(mid), sizeof(ps));
        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            save_rcd_accident_p_pos = mid;
            if (ps.head.node > RCD_ACCIDENT_NODE_MAX_SIZE || ps.tail.node > RCD_ACCIDENT_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd_accident_m, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    init_rcd_accident_pointer ()
* 功能：初始化环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
//static void init_rcd_accident_pointer(void)
void init_rcd_accident_pointer(void) {
    save_rcd_accident_p_pos = RCD_ACCIDENT_P_SAVE_SIZE;
    _memset((u8 *)&rcd_accident_m, 0x00, sizeof(rcd_accident_m));
    spi_flash_erase(ACCI_ADDRD(rcd_accident_m.head.node));
    save_rcd_accident_pointer();
}

/****************************************************************************
* 名称:    rcd_accident_cell_write ()
* 功能：事故疑点实时速度/IO 信息保存
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd_accident_cell_write(void) {
    if (accident_head >= RCD_ACCIDENT_CELL_SIZE || accident_tail >= RCD_ACCIDENT_CELL_SIZE) {
        accident_head = 0;
        accident_tail = 0;
    }

    rcd_accident_cell[accident_head].speed = (u8)mix.complex_speed;
    rcd_accident_cell[accident_head].sig.reg = rcd_io.reg;

    accident_head = incp_rcd_p(T_RCD_ACCIDENT_CELL, accident_head, 1);

    if (over_accident_cell()) {
        incp_rcd_p(T_RCD_ACCIDENT_CELL, accident_tail, 1);
    }

    return true;
}

/****************************************************************************
* 名称:    rcd_accident_data_write ()
* 功能：保存事故疑点信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd_accident_data_write(u8 *p, u16 len) {
    u16 index_t;
    static u16 erase_node_bak = 0;

    if (len != RCD_ACCIDENT_INF_SIZE || p == NULL) {
        logd("rcd accident err: len");
        return false;
    }

    if ((rcd_accident_m.head.node % 16) == 0) {
        if (erase_node_bak != rcd_accident_m.head.node) {
            erase_node_bak = rcd_accident_m.head.node;
            spi_flash_erase(ACCI_ADDRD(rcd_accident_m.head.node));
        }

        index_t = subp_rcd_p(rcd_accident_m.head.node, rcd_accident_m.tail.node, RCD_ACCIDENT_NODE_MAX_SIZE);
        if (index_t > (RCD_ACCIDENT_NODE_MAX_SIZE - 32)) {
            rcd_accident_m.tail.node = incp_rcd_p(T_RCD_ACCIDENT, rcd_accident_m.tail.node, 16);
        }
    }

    spi_flash_write(p, ACCI_ADDRD(rcd_accident_m.head.node), len);
    rcd_accident_m.head.node = incp_rcd_p(T_RCD_ACCIDENT, rcd_accident_m.head.node, 1);

    save_rcd_accident_pointer();
    return true;
}

/****************************************************************************
* 名称:    rcd_accident_data_write ()
* 功能：填充事故疑点信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static s16 rcd_accident_fill_in_infomation(u8 type, u8 *p) {
    u8 buf[500];
    u16 b_len;
    u16 i;
    s16 index;

    S_RCD_360HOUR_INFOMATION inf_t;

    if (p == NULL) {
        loge("rcd accident err : pointer");
        return 0;
    }

    index = accident_head;
    if (accident_offset) //去除判断MOVING 的3秒
    {
        accident_offset = false;
        /// 测试index = decp_rcd_p(T_RCD_ACCIDENT_CELL,index,30);
    }

    inf_t.locate.lngi = _jtb_to_gb(mix.locate.lngi);
    inf_t.locate.lati = _jtb_to_gb(mix.locate.lati);
    inf_t.heigh = mix.heigh;

    b_len = 0;
    b_len += _memcpy_len(&buf[b_len], (u8 *)&sys_time, 6);
    b_len += _memcpy_len(&buf[b_len], (u8 *)&mix.ic_driver, 18);
    for (i = 0; i < 100; i++) {
        index = decp_rcd_p(T_RCD_ACCIDENT_CELL, index, 1);
        buf[b_len++] = rcd_accident_cell[index].speed;
        buf[b_len++] = rcd_accident_cell[index].sig.reg;
    }

    b_len += _memcpy_len(&buf[b_len], (u8 *)&inf_t, 10);

    logd("index = %d __ b_len = %d", index, b_len);

    _memcpy(p, buf, b_len);

    return b_len;
}

/****************************************************************************
* 名称:    rcd_accident_manage ()
* 功能：管理事故疑点信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd_accident_manage(void) {
    static enum E_LINK_MANAGE {
        E_ACCIDENT_IDLE,
        E_ACCIDENT_NORMAL,
        E_ACCIDENT_WRITE,
        E_ACCIDENT_INIT,
        E_ACCIDENT_ERR
    } step = E_ACCIDENT_IDLE;

    bool ack = false;
    static bool OverturnedFlag = false;
    static u8 msg[RCD_ACCIDENT_INF_SIZE + 8] = {0};
    static u16 msg_len = 0;
    static u32 rcd_accident_jiff = 0;

    static ACCIDENT_TRI_BAK_STRUCT tri_bak;
    static char saveFlag = RCD_ACCIDENT_FILL_NULL;
    static u32 sec = 0, secBak = 0;

    switch (step) {
    case E_ACCIDENT_IDLE:
        if (tick < 5) //系统复位后5 秒不保存，并刷新状态
        {
            break;
        }

#if 1 //翻车疑点
        if (mix.moving && (false == OverturnedFlag))
        //if(false == OverturnedFlag)
        {
            if (gps_base.speed < 5) //定位速度＜5km/h可以认为是位置信息无变化
            {
                if (sec != sys_time.sec) {
                    sec = sys_time.sec;
                    secBak++;
                }
            } else {
                secBak = 0;
            }

            if (20 == secBak) //翻车疑点
            {
                secBak = 0;
                OverturnedFlag = true;
                msg_len = rcd_accident_fill_in_infomation(RCD_ACCIDENT_FILL_NORMAL, msg);
                step = E_ACCIDENT_WRITE;
                break;
            }
        } else if (!mix.moving) {
            sec = 61;
            secBak = 0;
            OverturnedFlag = false;
        }
#endif

        accident_offset = false;
        if (tri_bak.moving != mix.moving) { //这里是处理 车辆从移动 --> 停止的 数据保存。
            if (mix.moving == false) {      //停车前20s 进行保存, 停车标志 是 false
                accident_offset = true;
                step = E_ACCIDENT_WRITE; //保存数据到flash
                break;
            } else {
                tri_bak.moving = (mix.moving == true) ? true : false;
            }
        }

        if (tri_bak.pwr_off != mix.power) { //这个地方是处理 从掉电 --> 上电的处理
            if (mix.power == true) {
                step = E_ACCIDENT_WRITE; //保存数据--> flash
                break;
            } else {
                tri_bak.pwr_off = (mix.power == true) ? true : false;
            }
        }

        if (_pastn(rcd_accident_jiff) >= 200) { //满足200ms 后写 200ms的数据到 缓存
            rcd_accident_jiff = jiffies;
            ack = rcd_accident_cell_write(); //写疑点数据到缓存
            if (ack == false) {
                step = E_ACCIDENT_ERR;
                break;
            }
            //停车疑点数据
            if ((mix.complex_speed < 1) && mix.moving && (RCD_ACCIDENT_FILL_NULL == saveFlag)) {
                saveFlag = RCD_ACCIDENT_FILL_INVALID;
                msg_len = rcd_accident_fill_in_infomation(RCD_ACCIDENT_FILL_NORMAL, msg);
            } else if ((mix.complex_speed > 1) && (RCD_ACCIDENT_FILL_INVALID == saveFlag)) {
                saveFlag = RCD_ACCIDENT_FILL_NULL;
                memset(msg, 0x00, sizeof(msg));
            }

            // step = E_ACCIDENT_NORMAL;
            break;
        }

        break;
    case E_ACCIDENT_NORMAL:
        //ack = rcd_accident_cell_write();		//写疑点数据到缓存
        // if (ack == false)
        //  {
        //    step = E_ACCIDENT_ERR;
        //   break;
        //  }
        step = E_ACCIDENT_IDLE;
        break;
    case E_ACCIDENT_WRITE:
        tri_bak.acc = (mix.car_state.bit.acc == true) ? true : false;
        tri_bak.moving = (mix.moving == true) ? true : false;
        tri_bak.pwr_off = (mix.power == true) ? true : false;

        average_print_order(); //拷贝到打印区

        //msg_len = rcd_accident_fill_in_infomation(RCD_ACCIDENT_FILL_NORMAL, msg);
        rcd_accident_data_write(msg, msg_len);
        step = E_ACCIDENT_IDLE;
        break;
    case E_ACCIDENT_INIT:
        init_rcd_accident_pointer();
        step = E_ACCIDENT_IDLE;
        break;
    case E_ACCIDENT_ERR:
        step = E_ACCIDENT_IDLE;
        break;
    default:
        step = E_ACCIDENT_IDLE;
    }
}

/****************************************************************************
* 名称:    save_rcd_ot_pointer ()
* 功能：保存环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void save_rcd_ot_pointer(void) {
    u8 buf[128];

    if (save_rcd_ot_p_pos >= RCD_OT_P_SAVE_SIZE - 1) {
        spi_flash_erase((SECT_RCD_OT_P_ADDR * 0x1000 * 1ul));
        save_rcd_ot_p_pos = 0;
    } else {
        _memset(buf, 0x00, 128);
        spi_flash_write(buf, RCDOT_ADDRP(save_rcd_ot_p_pos), sizeof(rcd_ot_m));
        save_rcd_ot_p_pos++;
    }

    add_struct_crc((u8 *)&rcd_ot_m, sizeof(rcd_ot_m));
    spi_flash_write((u8 *)&rcd_ot_m, RCDOT_ADDRP(save_rcd_ot_p_pos), sizeof(rcd_ot_m));
}

/****************************************************************************
* 名称:    search_rcd_ot_pointor ()
* 功能：搜索超时驾驶环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd_ot_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD_OT_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD_OT_P_SAVE_SIZE;

    while (low <= high) {
    rcd_ot_search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, RCDOT_ADDRP(mid), sizeof(ps));
        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            save_rcd_ot_p_pos = mid;
            if (ps.head.node > RCD_OT_NODE_MAX_SIZE || ps.tail.node > RCD_OT_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd_ot_m, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto rcd_ot_search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    init_rcd_ot_pointer ()
* 功能：初始化环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
//static void init_rcd_ot_pointer(void)
void init_rcd_ot_pointer(void) {
    save_rcd_ot_p_pos = RCD_OT_P_SAVE_SIZE;
    _memset((u8 *)&rcd_ot_m, 0x00, sizeof(rcd_ot_m));
    spi_flash_erase(RCDOT_ADDRD(rcd_ot_m.head.node));
    save_rcd_ot_pointer();
}

/****************************************************************************
* 名称:    rcd_ot_fill_in_infomation ()
* 功能：填充记录仪疲劳驾驶信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static s16 rcd_ot_fill_in_infomation(u8 type, u8 *p) {
    u8 buf[128];
    u16 b_len;

    if (p == NULL) {
        loge("rcd accident err : pointer");
        return 0;
    }
    _memset(buf, 0, 128);

    fatigue_print.cell[fatigue_print.cur_index].write_ed = true;
    _memcpy_len((u8 *)&fatigue_print.cell[fatigue_print.cur_index].driver_lse, (u8 *)&rcd_ot_inf.drive, 21);
    _memcpy_len((u8 *)&fatigue_print.cell[fatigue_print.cur_index].start, (u8 *)&rcd_ot_inf.start, 6);
    _memcpy_len((u8 *)&fatigue_print.cell[fatigue_print.cur_index].end, (u8 *)&rcd_ot_inf.end, 6);
    if (++fatigue_print.cur_index >= RCD_PRINT_FATCNTS_MAX_SIZE)
        fatigue_print.cur_index = 0;

    b_len = 0;
    b_len += _memcpy_len(&buf[b_len], (u8 *)&rcd_ot_inf.drive, 18);
    b_len += _memcpy_len(&buf[b_len], (u8 *)&rcd_ot_inf.start, 6);
    b_len += _memcpy_len(&buf[b_len], (u8 *)&rcd_ot_inf.end, 6);

    b_len += _memcpy_len(&buf[b_len], (u8 *)&rcd_ot_inf.s_point, 8);

    b_len += _memcpy_len(&buf[b_len], (u8 *)&rcd_ot_inf.s_high, 2);

    b_len += _memcpy_len(&buf[b_len], (u8 *)&rcd_ot_inf.e_point, 8);

    b_len += _memcpy_len(&buf[b_len], (u8 *)&rcd_ot_inf.e_high, 2);

    _memcpy(p, buf, b_len);

    return b_len;
}

/****************************************************************************
* 名称:    rcd_ot_data_write ()
* 功能：保存记录仪疲劳驾驶信息
* 入口参数：无
* 出口参数：无
*  22-06-17 添加moveFlag 
****************************************************************************/
static bool rcd_ot_data_write(u8 *p, u16 len, bool saveFlag) {
    u16 index_t;
    static u16 erase_node_bak = 0;

    if (len != RCD_OT_INF_SIZE || p == NULL) {
        loge("rcd accident err: len");
        return false;
    }

    if ((rcd_ot_m.head.node % 60) == 0) {
        if (erase_node_bak != rcd_ot_m.head.node) {
            erase_node_bak = rcd_ot_m.head.node;
            spi_flash_erase(RCDOT_ADDRD(rcd_ot_m.head.node));
        }

        index_t = subp_rcd_p(rcd_ot_m.head.node, rcd_ot_m.tail.node, RCD_OT_NODE_MAX_SIZE);
        if (index_t > (RCD_OT_NODE_MAX_SIZE - 60)) {
            rcd_ot_m.tail.node = incp_rcd_p(T_RCD_DRIVER_OT, rcd_ot_m.tail.node, 60);
        }
    }

    if (saveFlag)
        spi_flash_write(p, RCDOT_ADDRD(rcd_ot_m.head.node), len);

    else
        flash25_program_auto_save(RCDOT_ADDRD(rcd_ot_m.head.node), p, len);

    save_rcd_ot_pointer();

    logd("... 保存疲劳驾驶节点: rcd_ot_m.head.node =%d   save_rcd_ot_p_pos=%d !!!", rcd_ot_m.head.node, save_rcd_ot_p_pos);
    return true;
}

/****************************************************************************
* 名称:    rcd_ot_judge ()
* 功能：处理ot 判断条件
    u8 drive[21];
    time_t start;
    time_t end;
    point_t s_point;
    u16 s_high;
    point_t e_point;
    u16 e_high;
    bool new_en;
* 入口参数：无
* 出口参数：无
****************************************************************************/
void rcd_ot_judge(void) {
    static u32 t_driving_time_tick = 0; //连续驾驶时长
    static u32 r_driving_time_tick = 0; //休息时间

    static bool t_run = false;  //本次驾驶时车辆移动标志
    static bool t_rest = false; //本次驾驶时车辆车辆停止标志;

    static bool tired = false;      //=true  已经达到疲劳驾驶的标志
//    static bool tired_near = false; //=true  已经达到接近疲劳驾驶预警标志。

    static bool sTime = false;
    static bool eTime = false;

    u32 fatigure_time = 0;
    u32 stop_rest_time = 0;

    if (alarm_cfg.fatigure_time == 0)
        return;

#if 1
    fatigure_time = alarm_cfg.fatigure_time;   //赋予默认初值;
    stop_rest_time = alarm_cfg.stop_rest_time; //停车休息时间
#else
    fatigure_time = 300;  //赋予默认初值;
    stop_rest_time = 120; //停车休息时间
#endif

    /*增加 驾驶途中更换 IC 卡, 如果中途更换则重新计算疲劳驾驶时间*/
    if (mix.ic_exchange) { //是否更换驾驶员, 如果更换则,重新计算疲劳驾驶时间。
        mix.ic_exchange = false;
        _memset((u8 *)&rcd_ot_inf, 0x00, sizeof(rcd_ot_inf)); //清空 开始时的数据
        t_driving_time_tick = 0;                              //连续驾驶时长
        r_driving_time_tick = 0;                              //连续驾驶开始时间
        t_run = false;                                        //每次车辆移动,则开始对本次驾驶的时间进行初始化
        t_rest = false;                                       //每次车辆车辆停止时, 开始计时;
        tired = false;                                        //=true  已经达到疲劳驾驶的标志
//        tired_near = false;                                   //=true  已经达到接近疲劳驾驶预警标志。
        return;
    }

    if (mix.moving == false) {
        if (0 == t_driving_time_tick) {
            if ((1 < mix.complex_speed) && (false == sTime)) //第一个不为零的速度
            {
                sTime = true;
                t_driving_time_tick = tick; //连续驾驶时间 秒计时
                r_driving_time_tick = 0;

                //_memset((u8*)&rcd_ot_inf, 0x00, sizeof(rcd_ot_inf));             	//清空疲劳驾驶数据区
                _memcpy((u8 *)&rcd_ot_inf.drive, (u8 *)&mix.ic_driver, 18); //填充驾驶证号
                _memcpy((u8 *)&rcd_ot_inf.start, (u8 *)&sys_time, 6);       //填充开始时间
                rcd_ot_inf.s_point.lati = _jtb_to_gb(mix.locate.lati);      //填充开始经度
                rcd_ot_inf.s_point.lngi = _jtb_to_gb(mix.locate.lngi);      //填充开始纬度
                rcd_ot_inf.s_high = mix.heigh;                              //记录开始高程
            } else if ((1 > mix.complex_speed) && (true == sTime)) {
                sTime = false;
            }
        }

    } else {
        //if(t_rest == false)
        {
            if ((1 > mix.complex_speed) && (false == eTime)) //第一个为零的速度
            {
                eTime = true;
                r_driving_time_tick = tick; //设置休息时间初值tick;
                rcd_ot_inf.e_high = mix.heigh;
                _memcpy((u8 *)&rcd_ot_inf.end, (u8 *)&sys_time, 6);    //填充开始时间
                rcd_ot_inf.e_point.lati = _jtb_to_gb(mix.locate.lati); //填充开始经度
                rcd_ot_inf.e_point.lngi = _jtb_to_gb(mix.locate.lngi); //填充开始纬度					 		                   	//记录开始高程
            } else if ((1 < mix.complex_speed) && (eTime == true)) {
                eTime = false;
            }
        }
    }

    if (mix.moving) { //车辆行驶 则赋予
        if (t_run == false) {
            t_run = true;
        }
        /*
		if(t_run == false)
		{
			t_driving_time_tick = tick;   //连续驾驶时间 秒计时
			r_driving_time_tick =  0;

			_memset((u8*)&rcd_ot_inf, 0x00, sizeof(rcd_ot_inf));             	//清空疲劳驾驶数据区
			_memcpy((u8*)&rcd_ot_inf.drive, (u8 *)&mix.ic_driver, 18);      //填充驾驶证号
			_memcpy((u8*)&rcd_ot_inf.start, (u8*)&sys_time, 6);       	//填充开始时间
			rcd_ot_inf.s_point.lati = _jtb_to_gb(mix.locate.lati);     		//填充开始经度
			rcd_ot_inf.s_point.lngi = _jtb_to_gb(mix.locate.lngi);     		//填充开始纬度
			rcd_ot_inf.s_high = mix.heigh;			 		                 //记录开始高程
			t_run =  true;
			t_rest = false;
		}
		else//停车休息时间未到又驾驶,则t_rest 需要重置
			t_rest = false; 	
		*/

#if 0
		if(_covern(t_driving_time_tick) >= (fatigure_time-1200))
		{//达到疲劳驾驶时间, 置 已经疲劳驾驶标志。
			tired_near = true;
		}
#endif
        if (_covern(t_driving_time_tick) >= fatigure_time) { //达到疲劳驾驶时间, 置 已经疲劳驾驶标志。

            //if(tired == false)
            //logd("..... 疲劳驾驶触发.....");
            tired = true;

            //22-06-11号按照何总指到添加以下内容
            //_memcpy((u8*)&rcd_ot_inf.drive, (u8*)&mix.ic_driver, 18);
            //_memcpy((u8*)&rcd_ot_inf.end, (u8*)&sys_time, 6);
            //rcd_ot_inf.e_point.lati = _jtb_to_gb(mix.locate.lati);
            //rcd_ot_inf.e_point.lngi = _jtb_to_gb(mix.locate.lngi);
            //rcd_ot_inf.e_high = gps_base.heigh;
        }
    } else { //车辆未行驶, 停车时 需要对休息时间进行判断, 如果休息时间达到规定的20min , 这不对本次行程进行记录
             //如果休息时间未达到20分钟, 则这段休息时间还是计入 连续驾驶时间内。

        /*如果车辆在停车前未开动过, 则不进行处理*/
        if (t_run == false)
            return;

        /*车辆从移动 到 静止 处理*/
        if (t_rest == false) { //初始化 休息时间参数
            t_rest = true;     //表示开始进行休息时间计时
            //r_driving_time_tick = tick;   //设置休息时间初值tick;
            return; //退出, 下次进入时跳过此步。
        }

        if (tired) {
            tired = false;
            rcd_ot_inf.new_en = true;
        }

        /*针对停车休息时间进行处理*/
        if (_covern(r_driving_time_tick) >= stop_rest_time) { //判断停车休息时间是否满足标准,

            rcd_ot_m.head.node = incp_rcd_p(T_RCD_DRIVER_OT, rcd_ot_m.head.node, 1);
            add_struct_crc((u8 *)&rcd_ot_m, sizeof(rcd_ot_m));
            if (tired == true) { //如果在停车休息之前 就已经触发了疲劳驾驶, 则应该在行驶记录仪中记录
                if (rcd_ot_inf.new_en == false) {
                    /*  //22-06-11按照何总指示屏蔽
					_memcpy((u8*)&rcd_ot_inf.drive, (u8*)&mix.ic_driver, 18);
					_memcpy((u8*)&rcd_ot_inf.end, (u8*)&sys_time, 6);               
					rcd_ot_inf.e_point.lati = _jtb_to_gb(mix.locate.lati);
					rcd_ot_inf.e_point.lngi = _jtb_to_gb(mix.locate.lngi);
					rcd_ot_inf.e_high = gps_base.heigh;
					
					rcd_ot_inf.new_en = true;      				//该标志在rcd_ot_manage() 使用
					*/
                    t_driving_time_tick = 0; //连续驾驶时长
                    r_driving_time_tick = 0; //休息时间
                    t_run = false;           //每次车辆移动,则开始对本次驾驶的时间进行初始化
                    t_rest = false;          //每次车辆车辆停止时, 开始计时;
                    tired = false;           //=true  已经达到疲劳驾驶的标志
//                    tired_near = false;      //=true  已经达到接近疲劳驾驶预警标志。

                    //logd("..... 停车休息时间到 保存数据.....");
                }
            } else {                                                  //如果在停车休息之前, 未触发疲劳驾驶条件, 则重新初始化参数。进入下一次记录。
                _memset((u8 *)&rcd_ot_inf, 0x00, sizeof(rcd_ot_inf)); //清空 开始时的数据
                t_driving_time_tick = 0;                              //连续驾驶时长
                r_driving_time_tick = 0;                              //连续驾驶开始时间
                t_run = false;                                        //每次车辆移动,则开始对本次驾驶的时间进行初始化
                t_rest = false;                                       //每次车辆车辆停止时, 开始计时;
                tired = false;                                        //=true  已经达到疲劳驾驶的标志
//                tired_near = false;                                   //=true  已经达到接近疲劳驾驶预警标志。
            }
            rcd_ot_inf.res1 = 1;

        } else { //休息时间小于规定时间, 则继续保持 连续驾驶时间计时
        }
    }
}
/****************************************************************************
* 名称:    rcd_ot_manage ()
* 功能：管理记录仪疲劳驾驶信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd_ot_manage(void) {
    static enum E_LINK_MANAGE {
        E_OT_IDLE,
        E_OT_WRITE,
        E_OT_INIT,
        E_OT_ERR
    } step = E_OT_IDLE;

    u8 msg[RCD_OT_INF_SIZE + 8];
    u16 msg_len = 0;

    switch (step) {
    case E_OT_IDLE:
        rcd_ot_judge();
        if (rcd_ot_inf.new_en) {
            rcd_ot_inf.new_en = false;
            step = E_OT_WRITE;
            //logd("..... 疲劳驾驶写数据.....");
        }
        break;
    case E_OT_WRITE:
        msg_len = rcd_ot_fill_in_infomation(RCD_OT_FILL_NORMAL, msg);
        rcd_ot_data_write(msg, msg_len, (bool)rcd_ot_inf.res1);
        rcd_ot_inf.res1 = 0;
        step = E_OT_IDLE;
        break;
    case E_OT_INIT:
        init_rcd_ot_pointer();
        step = E_OT_IDLE;
        break;
    case E_OT_ERR:
        step = E_OT_IDLE;
        break;
    default:
        step = E_OT_IDLE;
    }
}

/****************************************************************************
* 名称:    save_rcd_driver_pointer ()
* 功能：保存环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void save_rcd_driver_pointer(void) {
    u8 buf[128];

    if (save_rcd_driver_p_pos >= RCD_DRIVER_P_SAVE_SIZE - 1) {
        spi_flash_erase((SECT_RCD_DRIVER_P_ADDR * 0x1000 * 1ul));
        save_rcd_driver_p_pos = 0;
    } else {
        _memset(buf, 0x00, 128);
        spi_flash_write(buf, DRIVER_ADDRP(save_rcd_driver_p_pos), sizeof(rcd_driver_m));
        save_rcd_driver_p_pos++;
    }

    add_struct_crc((u8 *)&rcd_driver_m, sizeof(rcd_driver_m));
    spi_flash_write((u8 *)&rcd_driver_m, DRIVER_ADDRP(save_rcd_driver_p_pos), sizeof(rcd_driver_m));

    //logd("... 保存节点: rcd_driver_m.head.node =%d   save_rcd_driver_p_pos=%d !!!",rcd_driver_m.head.node,save_rcd_driver_p_pos);
}

/****************************************************************************
* 名称:    search_rcd_driver_pointor ()
* 功能：搜索环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd_driver_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD_DRIVER_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD_DRIVER_P_SAVE_SIZE;

    while (low <= high) {
    rcd_driver_search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;
        spi_flash_read((u8 *)&ps, DRIVER_ADDRP(mid), sizeof(ps));
        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            save_rcd_driver_p_pos = mid;
            if (ps.head.node > RCD_DRIVER_NODE_MAX_SIZE || ps.tail.node > RCD_DRIVER_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd_driver_m, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto rcd_driver_search_start;
    }

    //logd("... 保存节点: rcd_driver_m.head.node =%d   save_rcd_driver_p_pos=%d !!!",rcd_driver_m.head.node,save_rcd_driver_p_pos);
    return false;
}

/****************************************************************************
* 名称:    init_rcd_driver_pointer ()
* 功能：初始化环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
//static void init_rcd_driver_pointer(void)
void init_rcd_driver_pointer(void) {
    logi("驾驶员初始化");
    save_rcd_driver_p_pos = RCD_DRIVER_P_SAVE_SIZE;

    _memset((u8 *)&rcd_driver_m, 0x00, sizeof(rcd_driver_m));

    spi_flash_erase(DRIVER_ADDRD(rcd_driver_m.head.node));

    save_rcd_driver_pointer();
}

/****************************************************************************
* 名称:    rcd_driver_fill_in_infomation ()
* 功能：填充记录仪驾驶员记录信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static s16 rcd_driver_fill_in_infomation(u8 type, u8 *p) {
    u8 buf[50];
    u16 b_len;

    if (p == NULL) {
        loge("rcd driver err : pointer");
        return 0;
    }

    b_len = 0;

    b_len += _memcpy_len(&buf[b_len], (u8 *)&sys_time, 6);
    b_len += _memcpy_len(&buf[b_len], (u8 *)&mix.ic_driver, 18); //driver_cfg.license
    buf[b_len++] = (mix.ic_login == true) ? 0x01 : 0x02;

    _memcpy(p, buf, b_len);
    return b_len;
}

/****************************************************************************
* 名称:    rcd_driver_data_write ()
* 功能：保存记录仪驾驶员记录信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd_driver_data_write(u8 *p, u16 len) {
    u8 first = 0x50;
//    u8 buff[32] = {0};
    u16 index_t;

    static u16 erase_node_bak = 0;

    if (len != RCD_DRIVER_INF_SIZE || p == NULL) {
        loge("rcd accident err: len");
        return false;
    }

    if ((rcd_driver_m.head.node % 100) == 0) {
        if (erase_node_bak != rcd_driver_m.head.node) {
            erase_node_bak = rcd_driver_m.head.node;
            spi_flash_erase(DRIVER_ADDRD(rcd_driver_m.head.node));
            if (rcd_driver_m.head.node >= 200) {
                first = 0x50;
                flash25_program_auto_save(0x187A00, &first, 1);
            }
        }

        index_t = subp_rcd_p(rcd_driver_m.head.node, rcd_driver_m.tail.node, RCD_DRIVER_NODE_MAX_SIZE);
        if (index_t > (RCD_DRIVER_NODE_MAX_SIZE - 100)) {
            rcd_driver_m.tail.node = incp_rcd_p(T_RCD_DRIVER_LOG, rcd_driver_m.tail.node, 100);
        }
    }

    spi_flash_write(p, DRIVER_ADDRD(rcd_driver_m.head.node), len);
    rcd_driver_m.head.node = incp_rcd_p(T_RCD_DRIVER_LOG, rcd_driver_m.head.node, 1);
    save_rcd_driver_pointer();

    return true;
}

/****************************************************************************
* 名称:    rcd_driver_manage ()
* 功能：管理记录仪驾驶员记录信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd_driver_manage(void) {
    static enum E_LINK_MANAGE {
        E_DRIVER_IDLE,
        E_DRIVER_WRITE,
        E_DRIVER_INIT,
        E_DRIVER_ERR
    } step = E_DRIVER_IDLE;

    u8 msg[RCD_OT_INF_SIZE + 8];
    static u8 qin = 0;
    u16 msg_len = 0;
    static bool login_bak = false;
    static bool moving = false;
//    static bool fal = false;

    if (!qin) {
        qin = 1;
        init_rcd_driver_pointer();
    }
    switch (step) { //登录和退出应在行驶结束状态下进行
    case E_DRIVER_IDLE:

        if ((login_bak != mix.ic_login) && ic_driver_lisence) //登退签并且已经读到驾驶员证号
        {
            login_bak = mix.ic_login;
            step = E_DRIVER_WRITE;
            /*2022-04-04 note by hj*/
            ic_driver_lisence = false;
        }

        if (!mix.ic_login && (moving != mix.moving) && (!moving)) {
            //登退签记录; 刚启动时提示
            if ((mix.complex_speed > 0)) {
                moving = mix.moving;
            }
        } else {
            //有速度；没有登签记录就提示；
            moving = mix.moving;
        }
        break;
    case E_DRIVER_WRITE: {
        if (0 != mix.ic_driver[0]) {
            msg_len = rcd_driver_fill_in_infomation(RCD_DRIVER_FILL_NORMAL, msg);
            rcd_driver_data_write(msg, msg_len);
            step = E_DRIVER_IDLE;
        }
    }

    break;
    case E_DRIVER_INIT:
        init_rcd_driver_pointer();
        step = E_DRIVER_IDLE;
        break;
    case E_DRIVER_ERR:
        step = E_DRIVER_IDLE;
        break;
    default:
        step = E_DRIVER_IDLE;
        break;
    }
}

/****************************************************************************
* 名称:    save_rcd_power_pointer ()
* 功能：保存指针记录
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void save_rcd_power_pointer(void) {
    u8 buf[128];

    if (save_rcd_power_p_pos >= RCD_POWER_P_SAVE_SIZE - 1) {
        spi_flash_erase(SECT_RCD_POWER_P_ADDR * 0x1000 * 1ul);
        save_rcd_power_p_pos = 0;
    } else {
        _memset(buf, 0x00, 128);
        spi_flash_write(buf, POWER_ADDRP(save_rcd_power_p_pos), sizeof(rcd_power_m));
        save_rcd_power_p_pos++;
    }

    add_struct_crc((u8 *)&rcd_power_m, sizeof(rcd_power_m));
    spi_flash_write((u8 *)&rcd_power_m, POWER_ADDRP(save_rcd_power_p_pos), sizeof(rcd_power_m));

    //logd("... 保存节点: rcd_power_m.head.node =%d   save_rcd_power_p_pos=%d !!!",rcd_power_m.head.node,save_rcd_power_p_pos);
}

/****************************************************************************
* 名称:    search_rcd_power_pointor ()
* 功能：需找环形保存的指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd_power_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD_POWER_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD_POWER_P_SAVE_SIZE;

    while (low <= high) {
    rcd_power_search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, POWER_ADDRP(mid), sizeof(ps));

        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            save_rcd_power_p_pos = mid;
            if (ps.head.node > RCD_POWER_NODE_MAX_SIZE || ps.tail.node > RCD_POWER_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd_power_m, (u8 *)&ps, sizeof(ps));

            //logd("... 搜索节点: rcd_power_m.head.node =%d  ps.flag=%x  save_rcd_power_p_pos=%d !!!",rcd_power_m.head.node,ps.flag,save_rcd_power_p_pos);
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto rcd_power_search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    init_rcd_power_pointer ()
* 功能：初始化指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
//static void init_rcd_power_pointer(void)
void init_rcd_power_pointer(void) {
    save_rcd_power_p_pos = RCD_POWER_P_SAVE_SIZE;

    _memset((u8 *)&rcd_power_m, 0x00, sizeof(rcd_power_m));

    spi_flash_erase(POWER_ADDRD(rcd_power_m.head.node));

    save_rcd_power_pointer();
}

/****************************************************************************
* 名称:    rcd_power_fill_in_infomation ()
* 功能：填充电源记录信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static s16 rcd_power_fill_in_infomation(u8 type, u8 *p) {
    u8 buf[50];
    u16 b_len;

    if (p == NULL) {
        loge("rcd power err : pointer");
        return 0;
    }

    b_len = 0;
    b_len += _memcpy_len(&buf[b_len], (u8 *)&sys_time, 6);
    /*2022-04-03 modify by hj */
    //buf[b_len++] = (mix.power == false) ? 0x01 : 0x02;
    buf[b_len++] = (car_alarm.bit.cut_volt == false) ? 0x01 : 0x02;

    _memcpy(p, buf, b_len);
    return b_len;
}

/****************************************************************************
* 名称:    rcd_power_data_write ()
* 功能：电源记录保存
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd_power_data_write(u8 *p, u16 len) {
    u16 index_t;
    static u16 erase_node_bak = 0;

    if (len != RCD_POWER_INF_SIZE || p == NULL) {
        loge(" rcd power err: len");
        return false;
    }

    if ((rcd_power_m.head.node % 100) == 0) {
        if (erase_node_bak != rcd_power_m.head.node) {
            erase_node_bak = rcd_power_m.head.node;
            spi_flash_erase(POWER_ADDRD(rcd_power_m.head.node));
        }

        index_t = subp_rcd_p(rcd_power_m.head.node, rcd_power_m.tail.node, RCD_POWER_NODE_MAX_SIZE);
        if (index_t > (RCD_POWER_NODE_MAX_SIZE - 100)) {
            rcd_power_m.tail.node = incp_rcd_p(T_RCD_POWER_LOG, rcd_power_m.tail.node, 100);
        }
    }

    spi_flash_write(p, POWER_ADDRD(rcd_power_m.head.node), len);
    rcd_power_m.head.node = incp_rcd_p(T_RCD_POWER_LOG, rcd_power_m.head.node, 1);

    save_rcd_power_pointer();
    return true;
}

/****************************************************************************
* 名称:    rcd_power_manage ()
* 功能：记录仪电源记录管理
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd_power_manage(void) {
    static enum E_LINK_MANAGE {
        E_POWER_IDLE,
        E_POWER_WRITE,
        E_POWER_INIT,
        E_POWER_ERR
    } step = E_POWER_IDLE;
    u8 msg[RCD_POWER_INF_SIZE + 8];
    u16 msg_len = 0;

    static bool pwr_bak = false;
    bool pwr_cur = true;

    switch (step) {
    case E_POWER_IDLE:
        /*2022-04-03 modify by hj*/
        // car_alarm.bit.cut_volt = false   上电
        // car_alarm.bit.cut_volt = ture    断电
        pwr_cur = (car_alarm.bit.cut_volt == false) ? true : false;
        if (pwr_bak != pwr_cur) {
            logd("rcd_power write :%d", pwr_cur);
            pwr_bak = pwr_cur;
            step = E_POWER_WRITE;
        }
        break;
    case E_POWER_WRITE:
        msg_len = rcd_power_fill_in_infomation(RCD_POWER_FILL_NORMAL, msg);
        rcd_power_data_write(msg, msg_len);

        step = E_POWER_IDLE;
        break;
    case E_POWER_INIT:
        init_rcd_power_pointer();
        step = E_POWER_IDLE;
        break;
    case E_POWER_ERR:
        step = E_POWER_IDLE;
        break;
    default:
        step = E_POWER_IDLE;
    }
}

/****************************************************************************
* 名称:    save_rcd_para_pointer ()
* 功能：保存环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void save_rcd_para_pointer(void) {
    u8 buf[128];

    if (save_rcd_para_p_pos >= RCD_PARA_P_SAVE_SIZE - 1) {
        spi_flash_erase((SECT_RCD_PARA_P_ADDR * 0x1000 * 1ul));
        save_rcd_para_p_pos = 0;
    } else {
        _memset(buf, 0x00, 128);
        spi_flash_write(buf, _rcd_para_p_addr(save_rcd_para_p_pos), sizeof(rcd_para_m));
        save_rcd_para_p_pos++;
    }

    add_struct_crc((u8 *)&rcd_para_m, sizeof(rcd_para_m));
    spi_flash_write((u8 *)&rcd_para_m, _rcd_para_p_addr(save_rcd_para_p_pos), sizeof(rcd_para_m));
}

/****************************************************************************
* 名称:    search_rcd_para_pointor ()
* 功能：搜索环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd_para_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD_PARA_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD_PARA_P_SAVE_SIZE;

    while (low <= high) {
    rcd_para_search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, _rcd_para_p_addr(mid), sizeof(ps));
        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            save_rcd_para_p_pos = mid;
            if (ps.head.node > RCD_PARA_NODE_MAX_SIZE || ps.tail.node > RCD_PARA_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd_para_m, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto rcd_para_search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    init_rcd_para_pointer ()
* 功能：初始化环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
//static void init_rcd_para_pointer(void)
void init_rcd_para_pointer(void) {
    save_rcd_para_p_pos = RCD_PARA_P_SAVE_SIZE;

    _memset((u8 *)&rcd_para_m, 0x00, sizeof(rcd_para_m));

    spi_flash_erase(_rcd_para_t_addr(rcd_para_m.head.node));

    save_rcd_para_pointer();
}

/****************************************************************************
* 名称:    rcd_para_fill_in_infomation ()
* 功能：填充记仪录参数修改信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static s16 rcd_para_fill_in_infomation(u8 type, u8 *p) {
    u8 buf[50];
    u16 b_len;

    if (p == NULL) {
        loge("rcd para err : pointer");
        return 0;
    }

    b_len = 0;
    b_len += _memcpy_len(&buf[b_len], (u8 *)&sys_time, 6);
    buf[b_len++] = rcd_para_inf.type;
    _memcpy(p, buf, b_len);
    return b_len;
}

/****************************************************************************
* 名称:    rcd_para_data_write ()
* 功能：保存记录仪参数修改信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd_para_data_write(u8 *p, u16 len) {
    u16 index_t;
    static u16 erase_node_bak = 0;

    if (len != RCD_PARA_INF_SIZE || p == NULL) {
        loge(" rcd power err: len");
        return false;
    }

    if ((rcd_para_m.head.node % 100) == 0) {
        if (erase_node_bak != rcd_para_m.head.node) {
            erase_node_bak = rcd_para_m.head.node;
            spi_flash_erase(_rcd_para_t_addr(rcd_para_m.head.node));
        }

        index_t = subp_rcd_p(rcd_para_m.head.node, rcd_para_m.tail.node, RCD_PARA_NODE_MAX_SIZE);
        if (index_t > (RCD_PARA_NODE_MAX_SIZE - 100)) {
            rcd_para_m.tail.node = incp_rcd_p(T_RCD_PARAMETER_LOG, rcd_para_m.tail.node, 100);
        }
    }

    spi_flash_write(p, _rcd_para_t_addr(rcd_para_m.head.node), len);
    rcd_para_m.head.node = incp_rcd_p(T_RCD_PARAMETER_LOG, rcd_para_m.head.node, 1);

    save_rcd_para_pointer();

    //logd(" ..... 行驶记录仪参数设置记录  rcd_para_m.head.node =%d,  save_rcd_para_p_pos=%d",rcd_para_m.head.node,save_rcd_para_p_pos);
    return true;
}

/****************************************************************************
* 名称:    rcd_para_manage ()
* 功能：管理记录仪参数修改信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd_para_manage(void) {
    static enum E_LINK_MANAGE {
        E_PARA_IDLE,
        E_PARA_WRITE,
        E_PARA_INIT,
        E_PARA_ERR
    } step = E_PARA_IDLE;

    u8 msg[RCD_PARA_INF_SIZE + 8];
    u16 msg_len = 0;

    switch (step) {
    case E_PARA_IDLE:
        if (rcd_para_inf.en) {
            rcd_para_inf.en = false;
            step = E_PARA_WRITE;
            //logd("写行驶记录仪参数设置记录");
        }
        break;
    case E_PARA_WRITE:
        msg_len = rcd_para_fill_in_infomation(RCD_PARA_FILL_NORMAL, msg);
        rcd_para_data_write(msg, msg_len);
        step = E_PARA_IDLE;
        break;
    case E_PARA_INIT:
        init_rcd_para_pointer();
        step = E_PARA_IDLE;
        break;
    case E_PARA_ERR:
        step = E_PARA_IDLE;
        break;
    default:
        step = E_PARA_IDLE;
    }
}

/****************************************************************************
* 名称:    save_rcd_speed_pointer ()
* 功能：保存速度环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void save_rcd_speed_pointer(void) {
    u8 buf[128] = {0};

    if (save_rcd_speed_p_pos >= RCD_SPEED_P_SAVE_SIZE - 1) {
        spi_flash_erase((SECT_RCD_SPEED_P_ADDR * 0x1000 * 1ul));
        save_rcd_speed_p_pos = 0;
    } else {
        spi_flash_write(buf, RSPEED_ADDRP(save_rcd_speed_p_pos), sizeof(rcd_speed_m));
        save_rcd_speed_p_pos++;
    }

    add_struct_crc((u8 *)&rcd_speed_m, sizeof(rcd_speed_m));
    spi_flash_write((u8 *)&rcd_speed_m, RSPEED_ADDRP(save_rcd_speed_p_pos), sizeof(rcd_speed_m));
}

/****************************************************************************
* 名称:    search_rcd_speed_pointor ()
* 功能：需找环形保存的指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
bool search_rcd_speed_pointor(void) {
    s16 low;
    s16 mid;
    s16 high;

    S_RCD_POWER_MANAGE_STRUCT ps;

    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = RCD_SPEED_P_SAVE_SIZE;

    while (low <= high) {
    rcd_speed_search_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, RSPEED_ADDRP(mid), sizeof(ps));
        if (ps.flag == 0x55aa) {
            //当前有效区域
            //添加校验
            //save_rcd_power_p_pos = mid;  //2022-04-04
            save_rcd_speed_p_pos = mid;
            if (ps.head.node > RCD_SPEED_NODE_MAX_SIZE || ps.tail.node > RCD_SPEED_NODE_MAX_SIZE)
                return false;
            _memcpy((u8 *)&rcd_speed_m, (u8 *)&ps, sizeof(ps));
            return true;
        } else if (ps.flag == 0xffff) {
            //未使用区域
            high = mid - 1;
        } else {
            //已使用过的区域
            low = mid + 1;
        }
        goto rcd_speed_search_start;
    }

    return false;
}

/****************************************************************************
* 名称:    init_rcd_para_pointer ()
* 功能：初始化环形指针
* 入口参数：无
* 出口参数：无
****************************************************************************/
//static void init_rcd_speed_pointer(void)
void init_rcd_speed_pointer(void) {
    save_rcd_speed_p_pos = RCD_SPEED_P_SAVE_SIZE;

    _memset((u8 *)speed_message.sp_state, 0x00, 120);
    _memset((u8 *)&rcd_speed_m, 0x00, sizeof(rcd_speed_m));
    spi_flash_erase(_rcd_para_t_addr(rcd_speed_m.head.node));

    save_rcd_speed_pointer();
}

static bool finish;

/****************************************************************************
* 名称:    rcd_speed_fill_in_infomation ()
* 功能：填充速度状态记录信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static void rcd_speed_fill_in_infomation(u8 type, SPEED_TIME *p) {
    static u8 i;
    static u8 bak1_sec = 0xFF;

    if (p == NULL) {
        loge("rcd driver err : pointer");
        //return 0;
    }

    if (bak1_sec != sys_time.sec) {
        if (0 == i) {
            p->s_time = sys_time;
        }
        bak1_sec = sys_time.sec;
        //p->sp_state[i].pulse_s	= 70;//记录仪速度
        //p->sp_state[i].gps_s	= 60;  //参考速度
        p->sp_state[i].pulse_s = speed_state.pulse_s; //记录仪速度
        p->sp_state[i].gps_s = speed_state.gps_s;     //参考速度
        if (i == 59) {
            i = 0;
            p->e_time = sys_time;
            finish = true;
        } else {
            i++;
        }
    }
}

/****************************************************************************
* 函数名称:  	judge_speed_normal ()
* 函数功能：计算记录仪速度与GPS速度的差值
* 编写时间：2022.2.14
* 备	    注：当记录速度（基于速度传感器信号的速度）与 参考速度的差值率（记录速度与参考速度之差除以参考速度均超过±11%,时判定速度状态为异常
****************************************************************************/

u8 judge_speed_normal(void) {
    u16 piancha = 0;
    u16 sum = 0;
    u8 i;

    for (i = 0; i < 60; i++) {
        //piancha = (speed_message.sp_state[i].pulse_s  - speed_message.sp_state[i].gps_s)*100 / speed_message.sp_state[i].gps_s;
        //if( piancha < 11 )
        //2022-06-16修改速度比较功能能

        if (speed_message.sp_state[i].pulse_s > speed_message.sp_state[i].gps_s) {
            piancha = (speed_message.sp_state[i].pulse_s * 100) / (float)0.11;
            if (((speed_message.sp_state[i].pulse_s * 100) - (speed_message.sp_state[i].gps_s * 100)) > piancha)
                sum++;
        } else if (speed_message.sp_state[i].pulse_s < speed_message.sp_state[i].gps_s) {
            piancha = (speed_message.sp_state[i].gps_s * 100) / (float)0.11;
            if (((speed_message.sp_state[i].gps_s * 100) - (speed_message.sp_state[i].pulse_s * 100)) > piancha)
                sum++;
        }
    }
    if (sum >= 10)
        return 0x02;

    return 0x01;
}

/****************************************************************************
* 名称:    rcd_speed_data_write ()
* 功能：保存速度差值记录信息
* 入口参数：无
* 出口参数：无
****************************************************************************/
static bool rcd_speed_data_write(u8 *p, u16 len) {
    static u8 bak_min = 0xff;
    u16 index_t = 0;

    if (sys_time.sec == 0 || bak_min != sys_time.min || rcd_speed_m.init == true) { //一分钟后进入
        bak_min = sys_time.min;
        rcd_speed_m.init = false;

        //rcd_speed_m.head.node = incp_rcd_p(T_RCD_SPEED_LOG, rcd_speed_m.head.node, 1); 		//指针加加
        if ((rcd_speed_m.head.node % 30) == 0) //差不多一个扇区(4k)的大小
        {
            spi_flash_erase(RSPEED_ADDRD(rcd_speed_m.head.node));
            index_t = subp_rcd_p(rcd_speed_m.head.node, rcd_speed_m.tail.node, RCD_SPEED_NODE_MAX_SIZE); //计算大小
            if (index_t > (RCD_SPEED_NODE_MAX_SIZE - 30)) {
                //超界
                rcd_speed_m.tail.node = incp_rcd_p(T_RCD_SPEED_LOG, rcd_speed_m.tail.node, 30); //一个扇区的大小
            }

            spi_flash_write((u8 *)&speed_message, RSPEED_ADDRD(rcd_speed_m.head.node), sizeof(speed_message));
        }
        spi_flash_write((u8 *)&speed_message, RSPEED_ADDRD(rcd_speed_m.head.node), sizeof(speed_message));

        spi_flash_write(p, _rcd48_d_addr(rcd_speed_m.head.node, sys_time.sec), len);
        rcd_speed_m.head.node = incp_rcd_p(T_RCD_SPEED_LOG, rcd_speed_m.head.node, 1); //指针加加
        save_rcd_speed_pointer();
        return true;
    }
    
    return false;
}
/****************************************************************************
* 名称:    rcd_para_manage ()
* 功能：管理记录仪参数速度差值信息
        速度状态日志，记录内容为速度状态（异常或正常）、速度状态判定的开始时间及结束时间、
        从开始时间起连续 60s 每秒钟的记录速度和参考速度。记录数量应不少于最近 10 条。 
        
        注： 在定位模块工作正常，记录仪的参考速度（基于卫星定位信号的速度）
             连续大于40km/h的5 min时间内：当记录速度（基于速度传感器信号的速度）与参考速度
             的差值率（记录速度与参考速度之差除以参考速度）均超过±11%时，判定速度状态为异常；
             当记录速度与参考速度的差值率均在±11%范围内时，判定速度状态为正常。
             每个日历天记录仪判定速度状态1次，速度状态判定时，同时存储速度状态日志。
             
        每条记录格式:  1. 记录仪的速度状态         1 byte
                       2. 速度状态判定的开始时间   6 byte bcd码
                       3. 速度状态判定的结束时间   6 byte bcd码
                       4. 开始时间对应的记录速度   1 byte 
                       5. 开始时间对应的参考速度   1 byte
                       6. 开始时间后第 1s 对应的记录速度  1 byte
                       7. 开始时间后第 1s 对应的参考速度  1 byte
                       .....
                       8. 开始时间后第 59s 对应的记录速度 1 byte      
                       9. 开始时间后第 59s 对应的参考速度 1 byte 
                       
                       共计: 133 byte / 条.     
* 入口参数：无
* 出口参数：无      
****************************************************************************/
static void rcd_speed_manage(void) {
    static enum E_LINK_MANAGE {
        E_SPEED_INIT,
        E_SPEED_IDLE,
        E_SPEED_WRITE,
        //E_SPEED_INIT,
        E_SPEED_ERR
    } step = E_SPEED_INIT;
    u8 ren = 0;
    static u8 coun = 0;
//    static u16 msg_len = 50;
    static u8 everyday_time = 0;
    switch (step) {
    case E_SPEED_INIT:
        _memset((u8 *)&speed_message, 0x00, 133);
        _memset((u8 *)speed_message.sp_state, 0x00, 120);

        _memcpy((u8 *)&speed_message.s_time, (u8 *)&sys_time, 6);

        /*2022-04-20 modify by hj*/
        Cacl_Day_Hour(speed_message.s_time, 1); //2022-04-20
        Cacl_Day_Hour(speed_message.s_time, 1);
        Cacl_Day_Hour(speed_message.s_time, 1);
        Cacl_Day_Hour(speed_message.s_time, 1);
        Cacl_Day_Hour(speed_message.s_time, 1);
        speed_message.e_time = speed_message.s_time;

        /*2022-04-20 note by hj*/
        //speed_message.e_time = speed_message.s_time ;
        //speed_message.s_time.sec = 0 ;
        //speed_message.e_time.sec = 59;

        if ((sys_time.sec == 0) && (sys_time.date != everyday_time)) //2022-04-05 add by hj
            step = E_SPEED_IDLE;

        break;

    case E_SPEED_IDLE:
        if (gps_base.fixed && (gps_base.speed > 40)) //参考速度 >40km/h
        {                                            //注意这里需要 持续定位且 车速大于 40km/h
            {
                /*填充速度信息*/
                rcd_speed_fill_in_infomation(RCD_SPEED_FILL_NORMAL, &speed_message);
                if (finish) {
                    finish = false;
                    ren = judge_speed_normal();
                    speed_message.state = ren;
                    step = E_SPEED_WRITE;

                    coun++;
                    if (coun == 5) { // 每天一次, 每次5分钟
                        coun = 0;
                        everyday_time = sys_time.date;
                    }
                }
            }
        } else {
            step = E_SPEED_INIT;
        }

        break;
    case E_SPEED_WRITE:
        rcd_speed_data_write((u8 *)&speed_message, sizeof(speed_message));
        step = E_SPEED_INIT;
        break;
        /*
        case E_SPEED_INIT:
            init_rcd_speed_pointer();
            step = E_SPEED_IDLE;
           break;
          */
    case E_SPEED_ERR:
        step = E_SPEED_IDLE;
        break;
    default:
        step = E_SPEED_IDLE;
    }
}

/****************************************************************************
* 名称:    load_rcd_manage_infomation ()
* 功能：行驶记录仪数据参数初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void load_rcd_manage_infomation(void) {
    bool ret = false;
//    static bool fal = false;
    //	u8 qin =100;

    ret = search_rcd48_pointor();
    if (!ret) {
        init_rcd48_pointer();
    }

    ret = search_rcd360_pointor();
    if (!ret) {
        init_rcd360_pointer();
    }

    ret = search_rcd_accident_pointor();
    if (!ret) {
        init_rcd_accident_pointer();
    }

    ret = search_rcd_ot_pointor();
    if (!ret) {
        init_rcd_ot_pointer();
    }

    ret = search_rcd_driver_pointor();
    if (!ret) {
        init_rcd_driver_pointer();
    }

    ret = search_rcd_power_pointor();
    if (!ret) {
        init_rcd_power_pointer();
    }

    ret = search_rcd_para_pointor();
    if (!ret) {
        init_rcd_para_pointer();
    }
    ret = search_rcd_speed_pointor();
    if (!ret) {
        init_rcd_speed_pointer();
    }

    init_printf_rcd_info();
}

/****************************************************************************
* 名称:    rcd_manage_init ()
* 功能：记录仪参数初始化
* 入口参数：无
* 出口参数：无
****************************************************************************/
void rcd_manage_init(void) {
    init_mfg_parameter();
    init_rcd48_pointer();
    init_rcd360_pointer();
    init_rcd_accident_pointer();
    init_rcd_ot_pointer();
    init_rcd_driver_pointer();
    init_rcd_power_pointer();
    init_rcd_para_pointer();
    init_rcd_speed_pointer();
    init_printf_rcd_info();
}

/************************************************************
函数名称: Cacl_Day
功    能: 计算1个小时前后的时间
输入参数: pyz 偏移值  0: 减  1: 加
输出参数:
注意事项: 根据年月日时 计算1个小时前后的时间
************************************************************/
time_t Cacl_Day(time_t stm, u8 pyz) {
    u8 hour;
    u8 month;
    u8 day;
    //	u8 min;

    month = (stm.month); //HextoBin
    day = (stm.date);
    hour = (stm.hour);
    if (pyz == 0) {
        if (hour > 1) //大于1号
        {
            hour--;
        } else //判断月份
        {
            if (day > 1)
                day--;
            hour = 23;
        }
    }

    else {
        if (hour < 23) //大于1号
        {
            hour++;
        } else //判断月份
        {
            if (day < 31)
                day++;
            hour = 00;
        }
    }

    stm.hour = (hour); //BintoHex
    stm.date = (day);
    stm.month = (month);
    return stm;
}

/************************************************************
函数名称: Cacl_Day_Hour
功    能: 计算1个小时前后的时间
输入参数: pyz 偏移值  0: 减  1: 加
输出参数:
注意事项: 根据年月日时 计算1个小时前后的时间
************************************************************/
time_t Cacl_Day_Hour(time_t stm, u8 pyz) {
    u8 hour, month, day, min;

    month = (stm.month); //HextoBin
    day = (stm.date);
    hour = (stm.hour);
    min = (stm.min);
    if (pyz == 0) {
        if (min >= 1)
            min--;
        else {
            if (hour > 1) //大于1号
            {
                hour--;
            } else //判断月份
            {
                if (day > 1)
                    day--;
                hour = 23;
            }
            min = 59;
        }
    }

    else {
        if (min <= 59)
            min++;
        else {
            if (hour < 23) //大于1号
            {
                hour++;
            } else //判断月份
            {
                if (day < 31)
                    day++;
                hour = 00;
            }
            min = 00;
        }
    }

    stm.hour = (hour); //BintoHex
    stm.date = (day);
    stm.month = (month);
    stm.min = (min);
    return stm;
}

/************************************************************
函数名称: Cacl_Day_Hour
功    能: 计算前后一秒的时间
输入参数: pyz 偏移值  0: 减  1: 加
输出参数:
注意事项: 根据年月日时 计算1分钟后的时间
************************************************************/
time_t Cacl_Day_Hour_Min(time_t stm, u8 pyz) {
    u8 hour, month, day, min, sec;

    month = (stm.month); //HextoBin
    day = (stm.date);
    hour = (stm.hour);
    min = (stm.min);
    sec = (stm.sec);
    if (pyz == 0) {
        if (sec >= 1)
            sec--;
        else {
            if (min >= 1)
                min--;
            else {
                if (hour > 1) //大于1号
                {
                    hour--;
                } else //判断月份
                {
                    if (day > 1)
                        day--;
                    hour = 23;
                }
                min = 59;
            }
            sec = 59;
        }
    }

    else {
        if (sec <= 59)
            sec++;
        else {
            if (min <= 59)
                min++;
            else {
                if (hour < 23) //大于1号
                {
                    hour++;
                } else //判断月份
                {
                    if (day < 31)
                        day++;
                    hour = 00;
                }
                min = 00;
            }
            sec = 00;
        }
    }

    stm.month = (month);
    stm.date = (day);
    stm.hour = (hour); //BintoHex
    stm.min = (min);
    stm.sec = (sec);

    //logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....",stm.year,stm.month,stm.date,stm.hour,stm.min,stm.sec);

    return stm;
}

/************************************************************
函数名称: Cacl_Day_Hour
功    能: 计算前后一秒的时间
输入参数: pyz 偏移值  0: 减  1: 加
输出参数:
注意事项: 根据年月日时 计算1个小时前后的时间
************************************************************/
time_t Cacl_Day_Hour_Sec(time_t stm, u8 pyz) {
    u8 hour, month, day, min, sec;

    month = (stm.month); //HextoBin
    day = (stm.date);
    hour = (stm.hour);
    min = (stm.min);
    sec = (stm.sec);
    if (pyz == 0) {
        if (sec >= 1)
            sec--;
        else {
            if (min >= 1)
                min--;
            else {
                if (hour > 1) //大于1号
                {
                    hour--;
                } else //判断月份
                {
                    if (day > 1)
                        day--;
                    hour = 23;
                }
                min = 59;
            }
            sec = 59;
        }
    }

    else {
        if (sec <= 59)
            sec++;
        else {
            if (min <= 59)
                min++;
            else {
                if (hour < 23) //大于1号
                {
                    hour++;
                } else //判断月份
                {
                    if (day < 31)
                        day++;
                    hour = 00;
                }
                min = 00;
            }
            sec = 00;
        }
    }

    stm.month = (month);
    stm.date = (day);
    stm.hour = (hour); //BintoHex
    stm.min = (min);
    stm.sec = (sec);

    //logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....",stm.year,stm.month,stm.date,stm.hour,stm.min,stm.sec);

    return stm;
}

/****************************************************************
功能:时间比较
参数1: 第一个时间格式YYMMDDHHMMSS
参数2: 第二个时间格式YYMMDDHHMMSS
参数3: 比较的长度
返回: 1 T1<T2
		2 T1>T2
		0 T1=T2
****************************************************************/
u32 TimeCmp(time_t start, time_t end) {
    //	u8 i;
    if (start.year == end.year) {
        if (start.month == end.month) {
            if (start.date == end.date) {
                if (start.hour == end.hour) {
                    if (start.min == end.min) {
                        if (start.sec == end.sec)
                            return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/*过标行驶记录仪数据导入   //data_inport_para */
void rcd_data_inport(void) {
    //	 u32 item = 0;
    u32 i;
    //u32 k=0;
    u8 rand_num = 0;
    //	 u8 rand_pos_num = 0;
    u16 index_t;
    static u32 cnt = 0;
    static u16 erase_node_bak = 0;
    static bool ic_login = false;
    static bool cut_volt = false;
    static bool para_type = false;

    S_RCD48_SPEED_INFOMATION rcd48_inf;
    S_RCD_360HOUR_INFOMATION rcd360_inf = {0}; //经纬度、高程、速度
    S_RCD_360HOUR_INFOMATION inf_accident;

    if (data_inport_para.flag == 0)
        return;

    if (data_inport_para.flag == 2) {
        logi("..... 本次导入完成  .....");

        data_inport_para.flag = 0;
        cnt = 0;
        ic_login = false;
        cut_volt = false;
        para_type = false;
        return;
    }

    switch (data_inport_para.cmd) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07: {
        u8 sn_buf[8] = {"00004133"};

        /*cmd = 0x01*/
        _memcpy_len((u8 *)&driver_cfg.license, "374650320087453485", 18);

        /*cmd = 0x03  初始安装时间*/
        factory_para.setting_time.year = 0x21;
        factory_para.setting_time.month = 0x04;
        factory_para.setting_time.date = 0x16;
        factory_para.setting_time.hour = 0x14;
        factory_para.setting_time.min = 0x35;
        factory_para.setting_time.sec = 0x47;

        factory_para.set_mileage = 2150;

        run.total_dist = factory_para.set_mileage * 100 + 100 * 100;
        save_run_parameterer();

        /*cmd = 0x05  车辆信息*/
        init_car_cfg();
        rcd_para_inf.en = true;

        /*cmd = 0x06  信号状态*/
        //载货状态 车门锁定 油路断开 近光 远光 右转向 左转向 制动
        _memset((u8 *)&sig_cfg, 0x00, sizeof(sig_cfg));
        _memcpy((u8 *)&sig_cfg.b0, "载货状态", 8);
        _memcpy((u8 *)&sig_cfg.b1, "车门锁定", 8);
        _memcpy((u8 *)&sig_cfg.b2, "油路断开", 8);
        _memcpy((u8 *)&sig_cfg.b3, "近光", 4);
        _memcpy((u8 *)&sig_cfg.b4, "远光", 4);
        _memcpy((u8 *)&sig_cfg.b5, "右转向", 6);
        _memcpy((u8 *)&sig_cfg.b6, "左转向", 6);
        _memcpy((u8 *)&sig_cfg.b7, "制动", 4);

        _memset((u8 *)&unique_id, 0x00, sizeof(unique_id));
        _memcpy((u8 *)&unique_id.ccc, "A098335", 7);
        _memcpy((u8 *)&unique_id.model, "0000000000Z/HT-9", 16);

        unique_id.time[0] = 21; //生产日期
        unique_id.time[1] = 3;
        unique_id.time[2] = 1;
        unique_id.time[3] = 16;
        unique_id.time[4] = 10;
        unique_id.time[5] = 38;

        _str2tobcd(&unique_id.sn[0], &sn_buf[0]);
        _str2tobcd(&unique_id.sn[1], &sn_buf[2]);
        _str2tobcd(&unique_id.sn[2], &sn_buf[4]);
        _str2tobcd(&unique_id.sn[3], &sn_buf[6]);

        unique_id.res = 0;

        write_mfg_parameter();

    } break;
    case 0x08: { //48小时速度记录
        u8 in_buf[120];
        if (TimeCmp(data_inport_para.start, data_inport_para.end) == 1) {
            logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....", data_inport_para.start.year, data_inport_para.start.month, data_inport_para.start.date, data_inport_para.start.hour, data_inport_para.start.min, data_inport_para.start.sec);
            logd("..... 结束时间[%02x-%02x-%02x %02x:%02x:%02x].....", data_inport_para.end.year, data_inport_para.end.month, data_inport_para.end.date, data_inport_para.end.hour, data_inport_para.end.min, data_inport_para.end.sec);
            logd("..... cnt=%d.....", cnt);
            cnt = 0;
            data_inport_para.flag = 2;
            break;
        }

#if 1
        /*step.1  每秒的时间及节点  类似 judge_rcd48_save() 函数调用*/
        rcd48_m.h_time = data_inport_para.start;

        /*step.2  填充每秒的车速和IO状态 类似 rcd48_fill_in_infomation() 函数调用*/
        rcd48_inf.speed = data_inport_para.speed;
        rcd48_inf.sig.reg = rcd_io.reg;

        /*step.3  写入数据并保存  类似于 */
        cnt = 60;
        for (i = 0; i < cnt; i++) {
            rand_num = rand() % (10 + 1);
            in_buf[2 * i] = rcd48_inf.speed + rand_num;
            in_buf[2 * i + 1] = (rcd48_inf.sig.reg | ((rand_num & 0x01) | 0x80));
        }

        _memcpy((u8 *)&rcd48_m.h_time, (u8 *)&data_inport_para.start, sizeof(sys_time));

        rcd48_m.head.node = incp_rcd_p(T_RCD_48, rcd48_m.head.node, 1); //指针加加
        if ((rcd48_m.head.node % 30) == 0) {
            spi_flash_erase(_rcd48_t_addr(rcd48_m.head.node));
            index_t = subp_rcd_p(rcd48_m.head.node, rcd48_m.tail.node, RCD48_NODE_MAX_SIZE); //计算大小
            if (index_t > (RCD48_NODE_MAX_SIZE - 30)) {
                //超界
                rcd48_m.tail.node = incp_rcd_p(T_RCD_48, rcd48_m.tail.node, 30);
            }
        }

        rcd48_m.h_time.sec = 0;
        /*2022-04-28 add by hj*/
        data_inport_para.start.sec = 0;
        data_inport_para.end.sec = 0;

        spi_flash_write((u8 *)&rcd48_m.h_time, _rcd48_t_addr(rcd48_m.head.node), sizeof(time_t));

        spi_flash_write(in_buf, _rcd48_d_addr(rcd48_m.head.node, rcd48_m.h_time.sec), 120);

        save_rcd48_pointer();
#endif

        /*step.4  写入数据并保存  跳秒 */
        {
            cnt = 0;
            //logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....rcd48_m.head.node=[%d]  save_rcd48_p_pos=[%d]",data_inport_para.start.year,data_inport_para.start.month,data_inport_para.start.date,data_inport_para.start.hour,data_inport_para.start.min,data_inport_para.start.sec,rcd48_m.head.node,save_rcd48_p_pos);
            if (data_inport_para.dr == 0)
                data_inport_para.start = Cacl_Day_Hour(data_inport_para.start, 0); //从开始时间向下减 分钟
            else
                data_inport_para.start = Cacl_Day_Hour(data_inport_para.start, 1); //从开始时间向上增加 分钟
        }

    } break;

    case 0x09: { //360小时位置记录
        u8 in_buf[660];
        //logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....",data_inport_para.start.year,data_inport_para.start.month,data_inport_para.start.date,data_inport_para.start.hour,data_inport_para.start.min,data_inport_para.start.sec);
        //logd("..... 结束时间[%02x-%02x-%02x %02x:%02x:%02x].....",data_inport_para.end.year,data_inport_para.end.month,data_inport_para.end.date,data_inport_para.end.hour,data_inport_para.end.min,data_inport_para.end.sec);
        //logd("..... data_inport_para.flag=%d, cnt=%d.....",data_inport_para.flag,cnt);

        if (TimeCmp(data_inport_para.start, data_inport_para.end) == 1) {
            logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....", data_inport_para.start.year, data_inport_para.start.month, data_inport_para.start.date, data_inport_para.start.hour, data_inport_para.start.min, data_inport_para.start.sec);
            logd("..... 结束时间[%02x-%02x-%02x %02x:%02x:%02x].....", data_inport_para.end.year, data_inport_para.end.month, data_inport_para.end.date, data_inport_para.end.hour, data_inport_para.end.min, data_inport_para.end.sec);
            logd("..... cnt=%d.....", cnt);
            cnt = 0;
            data_inport_para.flag = 2;
            break;
        }

#if 1
        /*step.1  每秒的时间及节点  类似 judge_rcd48_save() 函数调用*/
        rcd360_m.h_time = data_inport_para.start;

        /*step.2  填充每秒的车速和IO状态 类似 rcd48_fill_in_infomation() 函数调用*/
        //rand_num = rand()%(10+1);
        rcd360_inf.locate.lati = (data_inport_para.lat + rand_num); //保存纬度
        rcd360_inf.locate.lngi = (data_inport_para.lon + rand_num); //保存经度
        rcd360_inf.heigh = data_inport_para.altitude + rand_num;    //高程
        rcd360_inf.speed = (u8)data_inport_para.speed + rand_num;   //当前平均速度

/*step.3  写入数据并保存  类似于 
                  	 u8  rand_num = 0; 
	                   u16 rand_pos_num = 0;
                  */
#if 1
        cnt = 60;
        for (i = 0; i < cnt; i++) {
            rand_num = rand() % (10 + 1);
            in_buf[11 * i + 0] = (u8)((rcd360_inf.locate.lngi + rand_num) & 0xff);
            in_buf[11 * i + 1] = (u8)((rcd360_inf.locate.lngi + rand_num) >> 8);
            in_buf[11 * i + 2] = (u8)((rcd360_inf.locate.lngi + rand_num) >> 16);
            in_buf[11 * i + 3] = (u8)((rcd360_inf.locate.lngi + rand_num) >> 24);
            in_buf[11 * i + 4] = (u8)((rcd360_inf.locate.lati + rand_num) & 0xff);
            in_buf[11 * i + 5] = (u8)((rcd360_inf.locate.lati + rand_num) >> 8);
            in_buf[11 * i + 6] = (u8)((rcd360_inf.locate.lati + rand_num) >> 16);
            in_buf[11 * i + 7] = (u8)((rcd360_inf.locate.lati + rand_num) >> 24);
            in_buf[11 * i + 8] = (u8)((rcd360_inf.heigh + rand_num) & 0xff);
            in_buf[11 * i + 9] = (u8)((rcd360_inf.heigh + rand_num) >> 8);
            in_buf[11 * i + 10] = rcd360_inf.speed + rand_num;
        }
#endif

        rcd360_m.head.node = incp_rcd_p(T_RCD_360, rcd360_m.head.node, 1);
        if (((rcd360_m.head.node % 6) == 0)) { //判断是否为下一页
            spi_flash_erase(_rcd360_t_addr(rcd360_m.head.node));
            /*计算出头尾指针的差值, 即有效数据条数, 这里一个节点是以小时为单位,共计666个字节*/
            /*重点注意该函数*/
            index_t = subp_rcd_p(rcd360_m.head.node, rcd360_m.tail.node, RCD360_NODE_MAX_SIZE);

            if (index_t > (RCD360_NODE_MAX_SIZE - 6)) {
                rcd360_m.tail.node = incp_rcd_p(T_RCD_360, rcd360_m.tail.node, 6);
            }
        }
        /*.....注意这里记录的时间不是BCD码.....*/
        _memcpy((u8 *)&rcd360_m.h_time, (u8 *)&data_inport_para.start, sizeof(sys_time));
        rcd360_m.h_time.sec = 00;
        rcd360_m.h_time.min = 0;
        data_inport_para.start.sec = 0;
        data_inport_para.start.min = 0;
        data_inport_para.end.sec = 0;
        data_inport_para.end.min = 0;
        /*先写下一小时的时间, 注意 1 pagesize 能保存 6小时数据 */
        spi_flash_write((u8 *)&rcd360_m.h_time, _rcd360_t_addr(rcd360_m.head.node), sizeof(time_t));

        spi_flash_write(in_buf, _rcd360_d_addr(rcd360_m.head.node, rcd360_m.h_time.min), 660);

        save_rcd360_pointer();
#endif

        /*step.4  写入数据并保存  跳秒 */
        {
            cnt = 0;
            //logd("..... 开始时间[%02x-%02x-%02x %02x:%02x:%02x].....rcd48_m.head.node=[%d]  save_rcd48_p_pos=[%d]",data_inport_para.start.year,data_inport_para.start.month,data_inport_para.start.date,data_inport_para.start.hour,data_inport_para.start.min,data_inport_para.start.sec,rcd48_m.head.node,save_rcd48_p_pos);
            if (data_inport_para.dr == 0)
                data_inport_para.start = Cacl_Day(data_inport_para.start, 0); //从开始时间向下减 分钟
            else
                data_inport_para.start = Cacl_Day(data_inport_para.start, 1); //从开始时间向上增加 分钟
        }
    } break;

    case 0x10: { //疑似事故记录
        u8 in_buf[234];
        u16 b_len = 0;

#if 1
        /*step.2  填充每秒的车速和IO状态 类似 rcd48_fill_in_infomation() 函数调用*/
        inf_accident.locate.lati = (data_inport_para.lat); //保存纬度
        inf_accident.locate.lngi = (data_inport_para.lon); //保存经度
        inf_accident.heigh = data_inport_para.altitude;    //高程
        inf_accident.speed = (u8)data_inport_para.speed;   //当前平均速度

        b_len = 0;
        b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.start, 6);
        b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.ic_driver, 18);
        for (i = 0; i < 100; i++) {
            rand_num = rand() % (10 + 1);
            in_buf[b_len++] = inf_accident.speed + rand_num;
            in_buf[b_len++] = (rand_num & 0x01) | 0x80;
        }

        /*写停车时的位置和高程*/

#if 0
                  rand_num = rand()%(10+1);
                  in_buf[b_len++] = (u8)((inf_accident.locate.lngi+rand_num) >> 24);
                  in_buf[b_len++] = (u8)((inf_accident.locate.lngi+rand_num) >> 16);
                  in_buf[b_len++] = (u8)((inf_accident.locate.lngi+rand_num) >> 8);
                  in_buf[b_len++] = (u8)((inf_accident.locate.lngi+rand_num) &0xff);                                                                         
                  in_buf[b_len++] = (u8)((inf_accident.locate.lati+rand_num) >> 24);
                  in_buf[b_len++] = (u8)((inf_accident.locate.lati+rand_num) >> 16);
                  in_buf[b_len++] = (u8)((inf_accident.locate.lati+rand_num) >> 8);
                  in_buf[b_len++] = (u8)((inf_accident.locate.lati+rand_num) &0xff);                                    
                  in_buf[b_len++] = (u8)((inf_accident.heigh+ rand_num) >> 8) ;                                     
                  in_buf[b_len++] = (u8)((inf_accident.heigh+ rand_num) & 0xff) ;
#endif

#if 1
        rand_num = rand() % (10 + 1);
        in_buf[b_len++] = (u8)((inf_accident.locate.lngi + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((inf_accident.locate.lngi + rand_num) >> 8);
        in_buf[b_len++] = (u8)((inf_accident.locate.lngi + rand_num) >> 16);
        in_buf[b_len++] = (u8)((inf_accident.locate.lngi + rand_num) >> 24);
        in_buf[b_len++] = (u8)((inf_accident.locate.lati + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((inf_accident.locate.lati + rand_num) >> 8);
        in_buf[b_len++] = (u8)((inf_accident.locate.lati + rand_num) >> 16);
        in_buf[b_len++] = (u8)((inf_accident.locate.lati + rand_num) >> 24);
        in_buf[b_len++] = (u8)((inf_accident.heigh + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((inf_accident.heigh + rand_num) >> 8);
#endif

        if ((rcd_accident_m.head.node % 16) == 0) {
            if (erase_node_bak != rcd_accident_m.head.node) {
                erase_node_bak = rcd_accident_m.head.node;
                spi_flash_erase(ACCI_ADDRD(rcd_accident_m.head.node));
            }

            index_t = subp_rcd_p(rcd_accident_m.head.node, rcd_accident_m.tail.node, RCD_ACCIDENT_NODE_MAX_SIZE);
            if (index_t > (RCD_ACCIDENT_NODE_MAX_SIZE - 16)) {
                rcd_accident_m.tail.node = incp_rcd_p(T_RCD_ACCIDENT, rcd_accident_m.tail.node, 16);
            }
        }

        spi_flash_write(in_buf, ACCI_ADDRD(rcd_accident_m.head.node), b_len);
        rcd_accident_m.head.node = incp_rcd_p(T_RCD_ACCIDENT, rcd_accident_m.head.node, 1);

        save_rcd_accident_pointer();
#endif

        //logd("..... 我在这里....命令[%02x].",data_inport_para.cmd);
        /*step.4  写入数据并保存  跳秒 */
        {
            cnt = 0;
            data_inport_para.flag = 2;
        }
    } break;

    case 0x11: { //疲劳驾驶记录
        u8 in_buf[50];
        u16 b_len = 0;

#if 0
    fatigue_print.cell[fatigue_print.cur_index].write_ed = true;
    _memcpy_len((u8 *)&fatigue_print.cell[fatigue_print.cur_index].driver_lse, (u8 *)&rcd_ot_inf.drive, 21);
    _memcpy_len((u8 *)&fatigue_print.cell[fatigue_print.cur_index].start, (u8 *)&rcd_ot_inf.start, 6);
    _memcpy_len((u8 *)&fatigue_print.cell[fatigue_print.cur_index].end, (u8 *)&rcd_ot_inf.end, 6);
    if (++fatigue_print.cur_index >= RCD_PRINT_FATCNTS_MAX_SIZE)
        fatigue_print.cur_index = 0;
#endif

#if 1
        b_len = 0;
        b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.ic_driver, 18);
        b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.start, 6);
        b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.end, 6);

#if 0
                  /*写开始的位置和高程*/
                  rand_num = rand()%(10+1);
                  in_buf[b_len++] = (u8)((data_inport_para.lon +rand_num ) >> 24);
                  in_buf[b_len++] = (u8)((data_inport_para.lon +rand_num) >> 16);
                  in_buf[b_len++] = (u8)((data_inport_para.lon +rand_num) >> 8);
                  in_buf[b_len++] = (u8)((data_inport_para.lon +rand_num) &0xff);                                                       
                  in_buf[b_len++] = (u8)((data_inport_para.lat +rand_num) >> 24);
                  in_buf[b_len++] = (u8)((data_inport_para.lat +rand_num) >> 16);
                  in_buf[b_len++] = (u8)((data_inport_para.lat +rand_num) >> 8);
                  in_buf[b_len++] = (u8)((data_inport_para.lat+rand_num) &0xff);                                    
                  in_buf[b_len++] = (u8)((data_inport_para.altitude +rand_num ) >> 8) ;                                                       
                  in_buf[b_len++] = (u8)((data_inport_para.altitude +rand_num) & 0xff) ;                           
                  

                  /*写结束时的位置和高程*/
                  rand_num = rand()%(10+1);
                  in_buf[b_len++] = (u8)((data_inport_para.lon+rand_num) >> 24);
                  in_buf[b_len++] = (u8)((data_inport_para.lon+rand_num) >> 16);
                  in_buf[b_len++] = (u8)((data_inport_para.lon+rand_num) >> 8);
                  in_buf[b_len++] = (u8)((data_inport_para.lon+rand_num) &0xff);                                                       
                  in_buf[b_len++] = (u8)((data_inport_para.lat+rand_num) >> 24);
                  in_buf[b_len++] = (u8)((data_inport_para.lat+rand_num) >> 16);
                  in_buf[b_len++] = (u8)((data_inport_para.lat+rand_num) >> 8);
                  in_buf[b_len++] = (u8)((data_inport_para.lat+rand_num) &0xff);                                    
                  in_buf[b_len++] = (u8)((data_inport_para.altitude+ rand_num) >> 8) ;                                    
                  in_buf[b_len++] = (u8)((data_inport_para.altitude+ rand_num) & 0xff) ;
#endif

#if 1
        rand_num = rand() % (10 + 1);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) >> 8);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) >> 16);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) >> 24);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) >> 8);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) >> 16);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) >> 24);
        in_buf[b_len++] = (u8)((data_inport_para.altitude + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((data_inport_para.altitude + rand_num) >> 8);

        /*写结束时的位置和高程*/
        rand_num = rand() % (10 + 1);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) >> 8);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) >> 16);
        in_buf[b_len++] = (u8)((data_inport_para.lon + rand_num) >> 24);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) >> 8);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) >> 16);
        in_buf[b_len++] = (u8)((data_inport_para.lat + rand_num) >> 24);
        in_buf[b_len++] = (u8)((data_inport_para.altitude + rand_num) & 0xff);
        in_buf[b_len++] = (u8)((data_inport_para.altitude + rand_num) >> 8);
#endif

        if ((rcd_ot_m.head.node % 60) == 0) {
            if (erase_node_bak != rcd_ot_m.head.node) {
                erase_node_bak = rcd_ot_m.head.node;
                spi_flash_erase(RCDOT_ADDRD(rcd_ot_m.head.node));
            }

            index_t = subp_rcd_p(rcd_ot_m.head.node, rcd_ot_m.tail.node, RCD_OT_NODE_MAX_SIZE);
            if (index_t > (RCD_OT_NODE_MAX_SIZE - 60)) {
                rcd_ot_m.tail.node = incp_rcd_p(T_RCD_DRIVER_OT, rcd_ot_m.tail.node, 60);
            }
        }

        spi_flash_write(in_buf, RCDOT_ADDRD(rcd_ot_m.head.node), b_len);
        rcd_ot_m.head.node = incp_rcd_p(T_RCD_DRIVER_OT, rcd_ot_m.head.node, 1);

        save_rcd_ot_pointer();
#endif

        //logd("..... 我在这里....命令[%02x].",data_inport_para.cmd);

        /*step.4  写入数据并保存  跳秒 */
        { //
            cnt = 0;
            data_inport_para.flag = 2;
        }
    }

    break;

    case 0x12: { //驾驶原登签记录   //开始时间代表登签,   结束时间代表退签。
        u8 in_buf[25];
        u16 b_len = 0;

#if 1
        b_len = 0;

        if (ic_login == false) {
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.start, 6);
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.ic_driver, 18);
            in_buf[b_len++] = 0x01;
            ic_login = true;
        } else {
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.end, 6);
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.ic_driver, 18);
            in_buf[b_len++] = 0x02;
            ic_login = false;
        }

        if ((rcd_driver_m.head.node % 100) == 0) {
            if (erase_node_bak != rcd_driver_m.head.node) {
                erase_node_bak = rcd_driver_m.head.node;
                spi_flash_erase(DRIVER_ADDRD(rcd_driver_m.head.node));
            }

            index_t = subp_rcd_p(rcd_driver_m.head.node, rcd_driver_m.tail.node, RCD_DRIVER_NODE_MAX_SIZE);
            if (index_t > (RCD_DRIVER_NODE_MAX_SIZE - 100)) {
                rcd_driver_m.tail.node = incp_rcd_p(T_RCD_DRIVER_LOG, rcd_driver_m.tail.node, 100);
            }
        }

        spi_flash_write(in_buf, DRIVER_ADDRD(rcd_driver_m.head.node), b_len);
        rcd_driver_m.head.node = incp_rcd_p(T_RCD_DRIVER_LOG, rcd_driver_m.head.node, 1);
        save_rcd_driver_pointer();
#endif

        /*step.4  写入数据并保存  跳秒 */
        { //
            cnt++;
            if (cnt == 2) {
                cnt = 0;
                data_inport_para.flag = 2;
                ic_login = false;
            }
        }
    } break;

    case 0x13: { //电源记录   //开始时间代表上电   结束时间代表掉电
        u8 in_buf[25];
        u16 b_len = 0;

#if 1
        b_len = 0;

        if (cut_volt == false) {
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.start, 6);
            in_buf[b_len++] = 0x01;
            cut_volt = true;
        } else {
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.end, 6);
            in_buf[b_len++] = 0x02;
            cut_volt = false;
        }

        if ((rcd_power_m.head.node % 100) == 0) {
            if (erase_node_bak != rcd_power_m.head.node) {
                erase_node_bak = rcd_power_m.head.node;
                spi_flash_erase(POWER_ADDRD(rcd_power_m.head.node));
            }

            index_t = subp_rcd_p(rcd_power_m.head.node, rcd_power_m.tail.node, RCD_POWER_NODE_MAX_SIZE);
            if (index_t > (RCD_POWER_NODE_MAX_SIZE - 100)) {
                rcd_power_m.tail.node = incp_rcd_p(T_RCD_POWER_LOG, rcd_power_m.tail.node, 100);
            }
        }

        spi_flash_write(in_buf, POWER_ADDRD(rcd_power_m.head.node), b_len);
        rcd_power_m.head.node = incp_rcd_p(T_RCD_POWER_LOG, rcd_power_m.head.node, 1);

        save_rcd_power_pointer();
#endif

        //logd("..... 我在这里....命令[%02x]",data_inport_para.cmd);

        /*step.4  写入数据并保存  跳秒 */
        { //
            cnt++;
            if (cnt == 2) {
                cnt = 0;
                data_inport_para.flag = 2;
                cut_volt = false;
                //logd("..... rcd_power_m.head.node=[%02x]  rcd_power_m.tail.node=[%02x]",rcd_power_m.head.node,rcd_power_m.tail.node);
            }
        }
    }

    break;

    case 0x14: { //参数设置记录 开始时间代表c2   结束时间代表82
        u8 in_buf[25];
        u16 b_len = 0;

#if 1
        b_len = 0;

        if (para_type == false) {
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.start, 6);
            in_buf[b_len++] = 0xc2;
            para_type = true;
        } else {
            b_len += _memcpy_len(&in_buf[b_len], (u8 *)&data_inport_para.end, 6);
            in_buf[b_len++] = 0x82;
            para_type = false;
        }

        if ((rcd_para_m.head.node % 100) == 0) {
            if (erase_node_bak != rcd_para_m.head.node) {
                erase_node_bak = rcd_para_m.head.node;
                spi_flash_erase(_rcd_para_t_addr(rcd_para_m.head.node));
            }

            index_t = subp_rcd_p(rcd_para_m.head.node, rcd_para_m.tail.node, RCD_PARA_NODE_MAX_SIZE);
            if (index_t > (RCD_PARA_NODE_MAX_SIZE - 100)) {
                rcd_para_m.tail.node = incp_rcd_p(T_RCD_PARAMETER_LOG, rcd_para_m.tail.node, 100);
            }
        }

        spi_flash_write(in_buf, _rcd_para_t_addr(rcd_para_m.head.node), b_len);
        rcd_para_m.head.node = incp_rcd_p(T_RCD_PARAMETER_LOG, rcd_para_m.head.node, 1);

        save_rcd_para_pointer();
#endif

        //logd("..... 我在这里....命令[%02x]",data_inport_para.cmd);

        /*step.4  写入数据并保存  跳秒 */
        { //
            cnt++;
            if (cnt == 2) {
                cnt = 0;
                data_inport_para.flag = 2;
                para_type = false;
            }
        }
    }

    break;

    case 0x15: {
        //                  u8 in_buf[50];
        //                  u16 b_len = 0;
        float temp = 0;
        u8 min_cnt = 0;

        _memset((u8 *)&speed_message, 0x00, 133);
        _memset((u8 *)speed_message.sp_state, 0x00, 120);

        _memcpy((u8 *)&speed_message.s_time, (u8 *)&data_inport_para.start, 6);
        //speed_message.s_time.sec = 59 ;
        data_inport_para.start = Cacl_Day_Hour(data_inport_para.start, 1);
        data_inport_para.start = Cacl_Day_Hour(data_inport_para.start, 1);
        data_inport_para.start = Cacl_Day_Hour(data_inport_para.start, 1);
        data_inport_para.start = Cacl_Day_Hour(data_inport_para.start, 1);
        data_inport_para.start = Cacl_Day_Hour(data_inport_para.start, 1);
        _memcpy((u8 *)&speed_message.e_time, (u8 *)&data_inport_para.start, 6); //注意前面已经修改了秒计数了。

        //speed_message.s_time.sec = 0 ;
        //speed_message.e_time.sec = 59; //59秒

        for (i = 0; i < 60; i++) {
            rand_num = rand() % (5 + 1);
            speed_message.sp_state[i].pulse_s = data_inport_para.speed + rand_num; //记录仪速度
            rand_num = rand() % (5 + 1);
            speed_message.sp_state[i].gps_s = data_inport_para.speed + rand_num; //参考速度
        }

        speed_message.state = 0x01; //判断速度异常
        for (i = 0; i < 60; i++) {
            if (speed_message.sp_state[i].gps_s >= speed_message.sp_state[i].pulse_s) {
                temp = ((float)(speed_message.sp_state[i].gps_s - speed_message.sp_state[i].pulse_s) / (float)speed_message.sp_state[i].gps_s);
                if (temp > 0.11) {
                    min_cnt++;
                    //speed_message.state = 0x02 ;
                    //break;
                }
            } else {
                temp = ((float)(speed_message.sp_state[i].pulse_s - speed_message.sp_state[i].gps_s) / (float)speed_message.sp_state[i].gps_s);
                if (temp > 0.11) {
                    min_cnt++;
                    //speed_message.state = 0x02 ;
                    //break;
                }
            }
        }

        if (min_cnt > 30) {
            speed_message.state = 0x02;
        } else {
            speed_message.state = 0x01;
        }
        //if (data_inport_para.start == 0 ) 	//一分钟后进入
        //{
        //rcd_speed_m.head.node = incp_rcd_p(T_RCD_SPEED_LOG, rcd_speed_m.head.node, 1); 		//指针加加
        if ((rcd_speed_m.head.node % 30) == 0) //差不多一个扇区(4k)的大小
        {
            spi_flash_erase(RSPEED_ADDRD(rcd_speed_m.head.node));
            index_t = subp_rcd_p(rcd_speed_m.head.node, rcd_speed_m.tail.node, RCD_SPEED_NODE_MAX_SIZE); //计算大小
            if (index_t > (RCD_SPEED_NODE_MAX_SIZE - 30)) {
                //超界
                rcd_speed_m.tail.node = incp_rcd_p(T_RCD_SPEED_LOG, rcd_speed_m.tail.node, 30); //一个扇区的大小
            }

            spi_flash_write((u8 *)&speed_message, RSPEED_ADDRD(rcd_speed_m.head.node), sizeof(speed_message));
        }
        spi_flash_write((u8 *)&speed_message, RSPEED_ADDRD(rcd_speed_m.head.node), sizeof(speed_message));
        rcd_speed_m.head.node = incp_rcd_p(T_RCD_SPEED_LOG, rcd_speed_m.head.node, 1); //指针加加
        save_rcd_speed_pointer();

        /*step.4  写入数据并保存  跳秒 */
        { //
            cnt = 0;
            data_inport_para.flag = 2;

#if 0
	                 cnt ++ ;
	                 if(cnt == 5)
	                 	{//5分钟即5次
                     	  cnt = 0;
                     	  data_inport_para.flag = 2;	                 		
                     	  //logd("..... 本条导入完成....."); 
	                 	}	
	                 	
	                 data_inport_para.start = Cacl_Day_Hour(data_inport_para.start,1);
#endif
        }
    }

    break;
    }
}

/****************************************************************************
* 名称:    rcd_manage_task ()
* 功能：记录仪任务管理
* 入口参数：无
* 出口参数：无
****************************************************************************/
void rcd_manage_task(void) {
    static bool flag = false;
    u8 first = 0;
    control_left_right_mess();
    if (jiffies < 5000)
        return;

    check_rcd_io();
    calculate_minute_average_speed();

    rcd48_speed_manage();
    rcd360_position_manage();
    rcd_accident_manage();
    rcd_ot_manage();
    rcd_driver_manage();
    rcd_power_manage();
    rcd_para_manage();
    rcd_speed_manage();

    if (!flag) {
        spi_flash_read(&first, 0x187A00, 1);
        if (first != 0x50) {
            first = 0x50;
            rcd_manage_init();
            spi_flash_erase(DRIVER_ADDRD(200));
            flash25_program_auto_save(0x187A00, &first, 1);
        }
        flag = true;
    }
}

#endif
