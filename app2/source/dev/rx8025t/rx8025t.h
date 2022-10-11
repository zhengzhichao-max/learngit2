#ifndef __RX8025T__H__
#define __RX8025T__H__

// #include <rtthread.h>
// #include "gd32f30x_rcu.h"

/***********************************************************************************

		RX8025T实时时钟驱动程序(软件模拟IIC)

		和庚申 VR1.0 日期：2020-02-03
												 
		硬件连接：RX8025T_INT(PIN10)-->PB；RX8025T_SCL(PIN2)-->PB；RX8025T_SDA(PIN10)-->PB； 注意：SDA与SCL必须接上拉电阻。
		
		BL8025T 的从地址为 7bit 固定的数据（0110 010）在通信时，从地址是附加上 R/W 以 8bit 数据发送的。
		0110 0100为写模式，0110 0101为读模式，对应十进制为：100、101；对应16进制为：0x64、0x65。
		
		BL8025T 有地址自动增加功能。指定的从地址一旦开始，之后只有数据字节被发送。每个字节后，BL8025T 的地址自动增加。
		
***********************************************************************************/

#define RX8025T_SLAVE_ADDR 0x64

/* ********************************************************************** */
//RX8025T寄存器
#define RX8025T_SEC_REG 0x00 //秒
#define RX8025T_MIN_REG 0x01 //分
#define RX8025T_HOU_REG 0x02 //时
#define RX8025T_WEE_REG 0x03 //星期，bit0~bit7对应日、一、二、三、四、五、六，对应值为0x01,0x02,0x04,0x08,0x10,0x20,0x40，不可出现2位为1的情况。
#define RX8025T_DAY_REG 0x04 //日期
#define RX8025T_MON_REG 0x05 //月份
#define RX8025T_YEA_REG 0x06 //年
#define RX8025T_RAM_REG 0x07 //RAM
#define RX8025T_ALm_REG 0x08 //闹钟分，不用是可做为ram使用。
#define RX8025T_ALh_REG 0x09 //闹钟时，不用是可做为ram使用。
#define RX8025T_ALw_REG 0x0a //闹钟星期，不用是可做为ram使用。
#define RX8025T_CYl_REG 0x0b //周期定时器的低8位
#define RX8025T_CYm_REG 0x0c //周期定时器的高4位，周期定时器共计12位。

#define RX8025T_EXT_REG 0x0d //扩展寄存器，bit7-TEST=工厂测试，总应该写0；bit6-WADA=星期或日历报警选择位；bit5-USEL=选择秒或分钟更新触发更新中断，0=秒更新，1=分钟更新； \
                             //bit4-TE=周期定时使能；bit3\2-FSEL1\0=芯片FOUT引脚输出频率选择位；bit1\0-TSEL1\0=用来设定固定周期的内部时钟源。

#define RX8025T_FLAG_REG 0x0e //标志寄存器，bit5-UF，bit4-TF，bit3-AF，分别是时间更新中断，固定周期定时中断，闹钟中断的中断标志位；bit1-VLF电压低，bit0-VDET由于电压低温补停止工作标志位。

#define RX8025T_CONT_REG 0x0f //控制寄存器，bit6~7(CSEL0、1)=温补间隔设置；bit5(UIE)=时间更新中断使能位(可由D寄存器的USEL位配置为1秒更新或1分钟更新)； \
                              //bit4(TIE)=周期定时时功能中断使能位；bit3(AIE)=闹铃中断使能位；bit0(RESET)=内部计数器复位；以上位全部为1有效。
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

extern TIME_ST RTC_Time;       //存储日期时间的结构体变量，声明成全局可用。
extern rt_uint8_t Time_Update; //时间更新标志

rt_err_t rx8025t_init(void);                                               //初始化RX8025T_IIC的GPIO
rt_err_t Read8025TData(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len);  //读RX8025T寄存器数据
rt_err_t Write8025TData(rt_uint8_t addr, rt_uint8_t *buf, rt_uint8_t len); //写RX8025T寄存器数据
rt_err_t get_rtc_time(TIME_ST *t);                                         //获取时间
rt_err_t set_rtc_time(TIME_ST *t);                                         //设置时间
void rx8025t_show(TIME_ST *t);                                             //打印时间
rt_uint8_t BCD2HEX(rt_uint8_t bcd_data);                                   //BCD码转数值
rt_uint8_t HEX2BCD(rt_uint8_t hex_data);                                   //数值转BCD码
void RX8025T_Interrupt_Process(void);                                      //RX8025T中断事件处理

rt_err_t rx8025t_setRtcTime_test(void);

//void Time_CFG(void);             //用EC11编码开关设置时间
//void week_pro(rt_uint8_t weeks); //调整时钟时的星期处理
//void display(rt_uint8_t mode);   //时间显示

#endif /* __RX8025T__H__ */
