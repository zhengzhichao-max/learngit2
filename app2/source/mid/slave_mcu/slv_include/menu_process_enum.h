
#ifndef __MENU_PRCESS_ENUM_H__
#define __MENU_PRCESS_ENUM_H__

#define menu_count(a) (a - 1)
#define menu_line(a) (a - 1)

//速度模式
//220828速度类型://全局变量：speed_mode//枚举显示字符串
typedef enum {
    enum_speed_mode_null,         //无效值
    enum_speed_mode_pusle,        //脉冲速度//pulse
    enum_speed_mode_can,          //can//wheel_speed
    enum_speed_mode_gps,          //GPS//mix.speed
    enum_speed_mode_set_password, //设置密码
    enum_speed_mode_cnt,          //比长度多了1
} enum_speed_mode_typedef;

//终端控制//terminal_ctrl
typedef enum {
    enum_terminal_ctrl__null,        //无效值
    enum_terminal_ctrl_restart,      //终端重启
    enum_terminal_ctrl_register,     //终端注册
    enum_terminal_ctrl_authenticate, //终端鉴权
    enum_terminal_ctrl_logout,       //终端注销
    enum_terminal_ctrl_lock,         //终端锁定
    enum_terminal_ctrl_unlock,       //终端解锁
    enum_terminal_ctrl_foster_care,  //系统看护
    enum_terminal_ctrl_cnt,          //比长度多了1
} enum_terminal_ctl_typedef;

#endif //__MENU_PRCESS_ENUM_H__
