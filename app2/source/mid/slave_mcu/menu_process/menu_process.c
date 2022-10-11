#include "include_all.h"

#include "menu_process.font"

#if (0)
static void wait_here(void) {
    beeper(0); //关动作

    log_write(event_mcu_boot_wait_here);

    while (1) { ; }
}
#endif

//***********************************************************************************************//
//***********************************************************************************************//
//-------------------------------      显示底层函数     -----------------------------------------//
//*************************************************************
//该函数实现的功能是根据年、月、日来计算星期
//函数返回值  0表示星期天  1表示星期一  以此类推
//
//*************************************************************
u8 date_to_week(u16 year, u8 month, u8 date) {
    u8 week, c, temp_y;

    if (month <= 2) {
        month |= 4; //1月2月同5月6月表
        year--;
    }

    c = year / 100;
    c &= 0x03; //百年%4
    temp_y = (char)(year % 100);

    week = ((c | (c << 2)) + (temp_y + (temp_y >> 2)) + (13 * month + 8) / 5 + date) % 7; //星期=(百年%4*5+年+年/4+(13*月+8)/5+日)%7

    return week; //返回星期
}

//*************************************************************
//数据由60进制转换成10进制
//
//*************************************************************
/*static u32 data_60_10_conver(u32 dat)
{
    dat *= 10 ;
    dat /= 60 ;

    return dat ;
}
*/

//*************************************************************
//显示计时时间
//显示位置在显示屏的右上角
//参数m_s为时间值，单位为毫秒
//
//*************************************************************
static void dis_time(u32 m_s) {
    u8 tim_m, tim_s;
    u8 dis_buff[6];

    m_s = m_s / 1000; //秒

    tim_m = m_s / 60;
    tim_m = tim_m % 60; //不能超过60
    tim_s = m_s % 60;

    *(dis_buff + 0) = (tim_m / 10) + 0x30;
    *(dis_buff + 1) = (tim_m % 10) + 0x30;
    *(dis_buff + 2) = ':';
    *(dis_buff + 3) = (tim_s / 10) + 0x30;
    *(dis_buff + 4) = (tim_s % 10) + 0x30;
    *(dis_buff + 5) = '\x0';

    lcd_dis_one_line(0, (8 * 11), dis_buff, false);
}

//*************************************************************
//检查显示的数字字符有没有超过数字ASCII码范围
//
//函数返回值   TRUE表示数据格式错误    FALSE表示数据格式正确
//*************************************************************
static bool check_num_ascii_error(u8 *ptr, u16 lgth) {
    u16 i;

    for (i = 0x00; i < lgth; i++) {
        if ((*(ptr + i) > 0x39) || (*(ptr + i) < 0x30)) {
            return true;
        }
    }

    return false;
}

//*************************************************************
//检查显示的数字字符有没有超过数字ASCII码范围，如有超过，则强制改为0x30
//
//*************************************************************
static void check_dis_data_error(u8 *ptr, u16 lgth) {
    u16 i;

    for (i = 0x00; i < lgth; i++) {
        if ((*(ptr + i) > 0x39) || (*(ptr + i) < 0x30)) {
            *(ptr + i) = 0x30;
        }
    }
}

//*************************************************************
//
//显示退回到待机界面
//
//*************************************************************
void dis_goto_standby(void) {
    _memset((u8 *)(&menu_auto_exit), 0x00, (sizeof(menu_auto_exit)));
    _memset((u8 *)(&menu_report), 0x00, (sizeof(menu_report)));
    _memset((u8 *)(&menu_cnt), 0x00, (sizeof(menu_cnt)));

    menu_cnt.menu_other = mo_standby;
    menu_cnt.menu_flag = true;
}

//计算菜单翻页显示的第一行位置
static u16 menu_move_first_line(u16 last, u16 point) {
    u16 i;

    point -= 1;

    if (point > (last + 2)) //确定显示第一项菜单值
    {
        i = (point - 2);
    } else if (point < last) {
        i = point;
    } else {
        i = last;
    }

    return i;
}

//*************************************************************
//该函数的功能是菜单翻页显示
//
//参数buff指待显示菜单的菜单内容缓冲区，为二维指针
//参数first_line为待显示内容位于整个菜单项中的位置
//参数point指当前显示菜单界面下选中的一项    从第1项开始计数
//参数length指待显示菜单下的所有子菜单的个数
//
//*************************************************************
static void menu_move_display(const u8 **buff, u16 first_line, u16 point, u16 length) {
    u16 m;
    u8 j, n;

    if (length == 2) //仅两行，则清除倒数一行
        lcd_area_dis_set(6, 7, 0, 131, 0x00);
    else if (length == 1) //仅一行，则清除倒数两行
        lcd_area_dis_set(4, 7, 0, 131, 0x00);
    else if (length == 0) //无数据，则清除倒数三行
        lcd_area_dis_set(2, 7, 0, 131, 0x00);

    if (point <= length) //判断数据的合法性
    {
        for (j = 0x00; j < 3; j++) {
            m = (first_line + j);

            if (m < length) //判断显示是否超过范围为
            {
                if (m == (point - 1)) {
                    n = _strlen((u8 *)(*(buff + j)));
                    lcd_dis_one_line((j + 1), 0, (*(buff + j)), true);
                    lcd_area_dis_set(((j + 1) * 2), (((j + 1) * 2) + 1), (8 * n), 131, 0xff);
                } else {
                    lcd_dis_one_line((j + 1), 0, (*(buff + j)), false);
                }
            } else {
                break;
            }
        }

        menu_cnt.last_start = first_line; //记录菜单显示第一项内容的位置
    }
}

//*************************************************************
//功能键的处理  包括    (1)上方向键   (2)下方向键    (3)确定键    (4)退格键
//参数menu_type表示菜单级数，  参数menu_length表示该级菜单所包含的子菜单个数
//函数返回值含义   0不规则菜单按下了确认键需要进一步处理
//                 1按键功能处理完成    2函数参数错误
//
//*************************************************************
static u8 function_key_hdl(u8 menu_type, u16 menu_length) {
    u16 *ptr;
    u8 flag;

    flag = 1;

    if (menu_type <= menu_max_depth) {
        menu_type -= 1;
        ptr = (((u16 *)(&menu_cnt)) + menu_type); //给指针赋值

        if (key_data.key == key_down) { //下翻页
            if (menu_length > 0) {
                (*ptr) += 1;

                if ((*ptr) > menu_length)
                    (*ptr) = 0x01;

                menu_cnt.menu_flag = true; //菜单界面更新
            }
        } else if (key_data.key == key_up) { //上翻页
            if (menu_length > 0) {
                (*ptr) -= 1;

                if ((*ptr) < 0x01)
                    (*ptr) = menu_length;

                menu_cnt.menu_flag = true; //菜单界面更新
            }
        } else if (key_data.key == key_ok) { //进入子菜单
            if (menu_length > 0) {
                if (menu_cnt.next_regular) { //规则菜单则进入下一级
                    *(ptr + 1) = 0x01;

                    menu_cnt.menu_flag = true; //菜单界面更新
                } else {                       //不规则菜单
                    flag = 0;
                }
            }
        } else if (key_data.key == key_esc) { //退出该级菜单
            (*ptr) = 0x00;

            menu_cnt.last_start = 0x00;
            menu_cnt.menu_flag = true; //菜单界面更新
        }
    } else {
        flag = 2; // 2函数参数错误
    }

    return flag;
}

//*************************************************************
//输入法在使用前的初始化操作
//
//参数ptr_dis表示待显示输入法项内容指针
//参数ptr_d表示输入法目标地址
//参数lgth表示上一次设置内容长度
//参数max_lgth表示本次设置内容的极限长度
//参数encrypt表示显示是否加密   TRUE表示加密   FALSE表示不加密
//
//*************************************************************
static void input_method_init(const u8 *ptr_dis, u8 *ptr_d, u16 lgth, u16 max_lgth, bool encrypt) {
    //--------------//
    input_method.input_lib.page_max = ascii_input_page_max;
    input_method.input_lib.sig_size = ascii_input_size;

    input_method.input_lib.crisis = 0;
    input_method.input_lib.front_crisis_size = ascii_input_size;

    input_method.input_lib.ptr_lib = ascii_char_table;
    //--------------//输入法默认设置  数据源为ASCII码

    input_method.lgth = max_lgth;   //设置本次输入内容的最大长度
    input_method.encrypt = encrypt; //设置加密状态

    input_method.ptr_dis = ptr_dis; //加载待显示输入法项内容指针
    input_method.ptr_dest = ptr_d;  //加载输入法目标地址

    input_method.page = 0x00;  //复位值
    input_method.point = 0x00; //复位值

    if (lgth > 60)
        lgth = 60;
    input_method.cnt = lgth; //恢复上一次设置内容长度

    _memcpy(input_method.buff, ptr_d, lgth); //恢复上一次设置内容
}

//*************************************************************
//自动退出界面设置
//
//参数TIM     界面停留时间   单位1毫秒
//参数DIR     TRUE表示退出到上一级规则菜单   FALSE表示待机界面
//
//*************************************************************
void menu_auto_exit_set(u32 tim, bool dir) {
    menu_auto_exit.lgth = tim;     //界面显示时间
    menu_auto_exit.dir_flag = dir; //退出方向

    menu_auto_exit.cnt = jiffies;    //时间同步
    menu_auto_exit.auto_flag = true; //自动退出设置
}

//*************************************************************
//报告界面设置
//
//参数TIM     界面停留时间   单位1毫秒
//参数PTR     报告界面显示内容
//参数TYP     为发送报告的数据类型
//参数f       报告类型    TRUE报告成功和失败    FALSE仅报告失败
//
//*************************************************************
//COMT:报告，弹窗，成功，失败
static void menu_report_set(u32 tim, const u8 *ptr[], report_type_enum typ, bool f) {
    menu_report.lgth = tim; //超时时间
    menu_report.ptr = ptr;  //显示内容

    menu_report.dat_type = typ; //数据类型
    menu_report.rpt = f;        //报告类型

    menu_report.cnt = jiffies; //时间同步
    menu_report.ok_flag = false;
    menu_report.en_flag = true;
}

//*************************************************************
//输入法处理
//
//函数返回值表示是否按下确认状态   TRUE表示有按下   FALSE表示没有按下
//
//*************************************************************
static bool input_method_hdl(void) {
    u8 n;
    u16 m;
    u8 temp[36];
    bool ret;

    ret = false;
    lcd_dis_one_line(0, (8 * 0), input_method.ptr_dis, false); //输入项描述

    lcd_area_dis_set(4, 5, 0, 131, 0x00);
    if (input_method.encrypt) { //加密状态
        _memset(temp, '*', input_method.cnt);
        lcd_dis_one_page(16, input_method.cnt, temp);              //第二行开始显示
    } else {                                                       //不加密状态
        lcd_dis_one_page(16, input_method.cnt, input_method.buff); //第二行开始显示
    }

    if (input_method.page >= input_method.input_lib.page_max) { //防止越界
        input_method.page = 0x00;
    }
    n = _strlen((u8 *)((*(input_method.input_lib.ptr_lib + (input_method.page)))));
    if (input_method.point >= n) { //防止越界
        input_method.point = 0x00;
    }
    lcd_dis_one_line(3, (8 * 0), (*(input_method.input_lib.ptr_lib + (input_method.page))), false); //输入法页显示

    input_method.point -= (input_method.point % input_method.input_lib.sig_size); //防止显示错位
    if (input_method.input_lib.sig_size == 1) {                                   //ASCII码
        m = *(input_method.input_lib.ptr_lib[input_method.page] + input_method.point);
        read_flash_fonts(m, temp);
        lcd_dis_one_word((3 * 2), (8 * input_method.point), temp, lcd_dis_type_8x16ascii, true);
    } else if (input_method.input_lib.sig_size == 2) { //国标码
        m = ((input_method.input_lib.ptr_lib[input_method.page][input_method.point]) * 0x100)
            + (input_method.input_lib.ptr_lib[input_method.page][input_method.point + 1]);
        read_flash_fonts(m, temp);
        lcd_dis_one_word((3 * 2), (8 * input_method.point), temp, lcd_dis_type_16x16chn, true);
    }

    //----------------------------------------------

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) {                //按键长按操作
            if (key_data.key == key_ok) { //此次输入操作完成
                ret = true;               //本次输入法操作完成
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_down) {
                input_method.page++;
                if (input_method.page >= input_method.input_lib.page_max) {
                    input_method.page = 0x00;
                }
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) {
                if (input_method.page > 0) {
                    input_method.page--;
                }
                menu_cnt.menu_flag = true;
            }
        } else {                          //按键短按操作
            if (key_data.key == key_ok) { //选中
                if (input_method.cnt < input_method.lgth) {
                    _memcpy((input_method.buff + input_method.cnt),
                            (*(input_method.input_lib.ptr_lib + (input_method.page)) + (input_method.point)),
                            (input_method.input_lib.sig_size));

                    input_method.cnt += input_method.input_lib.sig_size;
                }
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) { //退格
                if (input_method.cnt > 0) {
                    if ((input_method.input_lib.crisis != 0x00) && (input_method.cnt == input_method.input_lib.crisis)) {
                        input_method.cnt -= input_method.input_lib.front_crisis_size;
                    } else {
                        input_method.cnt -= input_method.input_lib.sig_size;
                    }
                } else {
                    menu_cnt.menu_other = mo_reset; //退回到上一级，规则菜单
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) { //上翻
                if (input_method.point > 0) {
                    input_method.point -= input_method.input_lib.sig_size;
                } else {
                    input_method.point = (n - input_method.input_lib.sig_size);
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_down) { //下翻
                input_method.point += input_method.input_lib.sig_size;
                if (input_method.point > (n - input_method.input_lib.sig_size)) {
                    input_method.point = 0x00;
                }

                menu_cnt.menu_flag = true;
            }
            /*
            else if (key_data.key == key_p) //切换输入页
            {
                input_method.page++;
                if (input_method.page >= input_method.input_lib.page_max)
                {
                    input_method.page = 0x00;
                }

                menu_cnt.menu_flag = true;
            }
            */
        }
    }
    //------------------------------------------//

    return ret; //返回按键的确认状态
}

//*************************************************************
//显示多页数据处理

//参数buff为待显示的缓冲区
//参数lgth为显示的总长度
//参数input_flag为输入法使能  TRUE使能输入法操作  FALSE禁止输入法操作
//参数input_dis实现输入法的界面
//
//*************************************************************
#define dis_multi_page_max 36 //多页显示翻页的级数
static void dis_multi_page(u8 *buff, u16 lgth, bool input_flag, dis_multi_page_input_point input_dis_hdl) {
    static u8 last_point, last_cnt[dis_multi_page_max]; //36级存储，最少能记住36页显示长度，最少显示1044个汉字

    u8 current_cnt, state;
    u16 n;

    lcd_area_dis_set(0, 7, 0, 131, 0x00);
    if (dis_multi_page_cnt > lgth)
        dis_multi_page_cnt = 0x00; //防止错误的值影响显示

    n = lcd_dis_one_page(0, (lgth - dis_multi_page_cnt), (buff + dis_multi_page_cnt));
    current_cnt = ((lgth - dis_multi_page_cnt) - n); //当前页面显示的字符个数

    if ((n == 0x00) && (dis_multi_page_cnt == 0x00)) {
        state = 0x00; //单页就显示完毕

        if (input_flag) //使能输入法界面
        {
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
        }
    } else {
        if (n == 0x00) //已经显示到了最后一页
        {
            state = 0x03;

            if (input_flag) //使能输入法界面
            {
                lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
            } else {
                lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
            }
        } else if (dis_multi_page_cnt == 0x00) //当前显示的是第一页
        {
            state = 0x01;
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);

            last_point = 0x00; //每次显示第一页时都清零指针
        } else                 //当前显示的是中间页面
        {
            state = 0x02;
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        }
    }

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (!key_data.lg) //按键短按操作
        {
            if (key_data.key == key_esc) //退出当前的操作
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) //上翻页操作
            {
                if ((state == 0x03) || (state == 0x02)) {
                    dis_multi_page_cnt -= *(last_cnt + (last_point - 1));

                    if (last_point) {
                        last_point--;
                        ;
                    }
                    menu_cnt.menu_flag = true;
                }
            } else if (key_data.key == key_down) //下翻页操作
            {
                if ((state == 0x01) || (state == 0x02)) {
                    if (last_point < dis_multi_page_max) {
                        dis_multi_page_cnt += current_cnt;      //下翻页操作
                        *(last_cnt + last_point) = current_cnt; //记录上一次的操作值

                        last_point++;
                        menu_cnt.menu_flag = true;
                    }
                } else if (input_flag) //使能输入法界面
                {
                    if (input_dis_hdl) {
                        input_dis_hdl();
                        menu_cnt.menu_flag = true;
                    }
                }
            }
        }
    }
    //------------------------------------------//
}

//*************************************************************
//上传事件设置
//
//*************************************************************
void sms_fix_input_set(void) {
    menu_cnt.menu_other = mo_sms_fix_input;
    input_method_init((*(sms_input_dis + 0)), (&(sms_up_center.fix_status)), 0, 1, false);
}

//*************************************************************
//信息点播设置
//
//*************************************************************
void sms_vod_input_set(void) {
    menu_cnt.menu_other = mo_sms_vod_input;
    input_method_init((*(sms_input_dis + 2)), (&(sms_up_center.vod_status)), 0, 1, false);
}

//*************************************************************
//提问应答设置
//
//*************************************************************
void sms_ask_input_set(void) {
    menu_cnt.menu_other = mo_sms_ask_input;
    input_method_init((*(sms_input_dis + 1)), (&(sms_up_center.ask_id_point)), 0, 1, false);
}

//***********************************************************************************************//
//***********************************************************************************************//

//***********************************************************************************************//
//***********************************************************************************************//
//-------------------------------    不规则菜单的处理  -----------------------------------------//

static void dis_standby_speed_base(void) {
    uint dat16;
    uchar i, n;
    uchar temp_buff[20];
    const uchar *ptr;

    dat16 = (gps_data.speed[0] & 0x0f) * 100;
    dat16 += (gps_data.speed[1] >> 4) * 10;
    dat16 += (gps_data.speed[1] & 0x0f);

    //dat16 = 93;

    if (dat16 == 0x00) {
        lcd_area_dis_set(2, 3, 0, 131, 0x00);

        if (state_data.state.flag.custom_flag) //显示自定义内容
        {
            ;
        } else //显示网络运营商
        {
#if (COMPANY == C_YW)
            i = sys_cfg.mobile;
#else
            i = 1;
#endif

            n = _strlen((u8 *)(work_ser_dis[i]));
            n = ((16 - n) / 2);
            lcd_dis_one_line(1, (8 * n), work_ser_dis[i], false);
        }

        //--------------------

        if (state_data.state.flag.ic_flag) //IC卡插入，显示工号
        {
            lcd_dis_one_line(2, (8 * 0), job_dis, false);

            for (i = 0x00; i < (ic_card.work_num_lgth); i++) {
                *(temp_buff + i) = *(ic_card.work_num + i);
            }
            *(temp_buff + i) = '\x0';

            lcd_dis_one_line(2, (8 * 4), temp_buff, false);
        } else {
            temp_buff[0] = '0';
            temp_buff[1] = 'K';
            temp_buff[2] = 'm';
            temp_buff[3] = '/';
            temp_buff[4] = 'h';
            temp_buff[5] = 0x00; //结束标志

            lcd_dis_one_line(2, (8 * 0), temp_buff, false);
        }
    } else {
        lcd_area_dis_set(2, 5, 0, (lcd_max_y_addr - 1), 0x00); //清除两行

        n = dat16 / 100;
        lcd_dis_custom(2, (8 * 1), (&num24_32_dis[n][0][0]), 4, (8 * 3));
        n = dat16 % 100 / 10;
        lcd_dis_custom(2, (8 * 4), (&num24_32_dis[n][0][0]), 4, (8 * 3));
        n = dat16 % 10;
        lcd_dis_custom(2, (8 * 7), (&num24_32_dis[n][0][0]), 4, (8 * 3));

        temp_buff[0] = 'K';
        temp_buff[1] = 'm';
        temp_buff[2] = '/';
        temp_buff[3] = 'h';
        temp_buff[4] = 0x00; //结束标志
        lcd_dis_one_line(1, (8 * 11 + 4), temp_buff, false);
    }

    //-----------------------------//
    //-------     方位显示    -----//
    dat16 = (gps_data.direction[0] & 0x0f) * 100;
    dat16 += (gps_data.direction[1] >> 4) * 10;
    dat16 += (gps_data.direction[1] & 0x0f);

    if ((dat16 >= dir_north_range_high) && (dat16 < dir_east_range_low)) {
        ptr = dir_lib_dis[0]; //东北
    } else if ((dat16 >= dir_east_range_low) && (dat16 < dir_east_range_high)) {
        ptr = dir_lib_dis[1]; //正东
    } else if ((dat16 >= dir_east_range_high) && (dat16 < dir_south_range_low)) {
        ptr = dir_lib_dis[2]; //东南
    } else if ((dat16 >= dir_south_range_low) && (dat16 < dir_south_range_high)) {
        ptr = dir_lib_dis[3]; //正南
    } else if ((dat16 >= dir_south_range_high) && (dat16 < dir_west_range_low)) {
        ptr = dir_lib_dis[4]; //西南
    } else if ((dat16 >= dir_west_range_low) && (dat16 < dir_west_range_high)) {
        ptr = dir_lib_dis[5]; //正西
    } else if ((dat16 >= dir_west_range_high) && (dat16 < dir_north_range_low)) {
        ptr = dir_lib_dis[6]; //西北
    } else {
        ptr = dir_lib_dis[7]; //正北
    }

    lcd_dis_one_line(2, (8 * 11 + 4), ptr, false);
    //-----------------------------//
    //-----------------------------//
}

//主界面:mo_standby_hdl
static void mo_standby_hdl(void) {
    u8 n;
    u8 i;
    u8 temp_buff[20];
    const u8 *ptr;

    //--------------------//

    lcd_area_dis_set(0, 1, 0, 131, 0x00);
    DIS_SELF_CHECK;

#pragma region CSQ, 信号强度
    n = (gps_data.csq_state & 0x7F);
    if (n > 99) //防止超过范围
        n = 99;

    if (n >= 20)
        ptr = state_flag_dis[3]; //满格信号(大于等于20)
    else if (n >= 9)
        ptr = state_flag_dis[17]; //一半信号(大于等于9小于20)
    else if (n >= 5)
        ptr = state_flag_dis[18]; //一格信号(大于等于5小于9)
    else
        ptr = state_flag_dis[19]; //无信号(小于5)

    lcd_dis_one_word(0, (16 * 0), ptr, lcd_dis_type_8x16icon, false);
#pragma endregion CSQ, 信号强度

#pragma region GP
    //   if (testbit((gps_data.gprs_alarm_state), 0)) //最低位表示上线状态
    if (gs.gprs1.login || gs.gprs2.login || BBstatus) {
        lcd_dis_one_word(0, (16 * 1), state_flag_dis[4], lcd_dis_type_8x16icon, false);
    }
#pragma endregion GP

#pragma region gps_type
    if (testbit((gps_data.gps_state), 7)) {                                             //最高位表示定位状态
        lcd_dis_one_word(0, (16 * 2), state_flag_dis[5], lcd_dis_type_8x16icon, false); //定位模式

        if (host_no_save_para.set_gps_module_type == 0x02) //单BD
            ptr = state_flag_dis[21];
        else if (host_no_save_para.set_gps_module_type == 0x03) //BD+GPS
            ptr = state_flag_dis[22];
        else //单GPS
            ptr = state_flag_dis[20];

        lcd_dis_one_word(0, (16 * 3), ptr, lcd_dis_type_8x16icon, false);
    }
#pragma endregion gps_type

#pragma region 硬盘锁
    if (pub_io.b.key_s == false) {
        lcd_dis_one_word(0, (16 * 4), state_flag_dis[10], lcd_dis_type_8x16icon, false);
    }
#pragma endregion 硬盘锁

#pragma region 未读信息
    if (state_data.state.flag.nread_sms) { //未读信息
        lcd_dis_one_word(0, (16 * 5), state_flag_dis[11], lcd_dis_type_8x16icon, false);
    }
#pragma endregion 未读信息

#pragma region 可视卫星标志 + 星数
    lcd_dis_one_word(0, (16 * 6), state_flag_dis[15], lcd_dis_type_8x16icon, false); //可视卫星标志
    n = ((gps_data.gps_state) & 0x3F);                                               //低5位表示可视卫星数
    lcd_dis_one_word(0, (16 * 7), num88_dis[(n / 10)], lcd_dis_type_8x8icon, false);
    lcd_dis_one_word(0, (16 * 7 + 8), num88_dis[(n % 10)], lcd_dis_type_8x8icon, false); //可视卫星数
#pragma endregion 可视卫星标志 + 星数

#pragma region IC卡状态
    if (state_data.state.flag.ic_flag) { //IC卡状态
        lcd_dis_one_word(lcd_pos_8x16(1, 0), state_flag_dis[6], lcd_dis_type_8x16icon, false);
    }
#pragma endregion IC卡状态

#pragma region SD卡状态 + HD状态
    const uchar *sd_hd_state[] = {state_flag_dis[7], state_flag_dis[8], state_flag_dis[12], state_flag_dis[23]};
    sdhd_staSel_enum sd_hd_sta = selfChk_HD_SD_stateGet();
    if (sd_hd_sta != sdhd_staSel_unknown) {
        lcd_dis_one_word(lcd_pos_8x16(1, 1), sd_hd_state[(int)sd_hd_sta], lcd_dis_type_8x16icon, false);
    }
#pragma endregion SD卡状态 + HD状态

#pragma region 摄像头状态
    const uchar *cam_state = state_flag_dis[15];
    cam_sta_enum cam_sta = selfChk_cam_stateGet();
    // if (sd_hd_sta != sdhd_staSel_unknown) {
    if (sd_hd_sta == cam_sta_err) {
        lcd_dis_one_word(lcd_pos_8x16(1, 2), cam_state, lcd_dis_type_8x16icon, false);
    }
    // }
#pragma endregion 摄像头状态

#pragma region USB状态
    if (state_data.state.flag.usb_flag) { //USB状态//未使用
        lcd_dis_one_word(1, (16 * 2), state_flag_dis[8], lcd_dis_type_8x16icon, false);
    }
#pragma endregion USB状态

    //--空了一格--//1, (16 * 3)//未使用

    if ((state_data.state.flag.up_flag) && (!(state_data.state.flag.down_flag))) { //上下行状态//未使用
        lcd_dis_one_word(1, (16 * 4), state_flag_dis[13], lcd_dis_type_8x16icon, false);
    } else if ((!(state_data.state.flag.up_flag)) && (state_data.state.flag.down_flag)) {
        lcd_dis_one_word(1, (16 * 4), state_flag_dis[14], lcd_dis_type_8x16icon, false);
    }
    if (state_data.state.flag.nack_phone) { //未接电话标志
        lcd_dis_one_word(1, (16 * 5), state_flag_dis[12], lcd_dis_type_8x16icon, false);
    }

    //--空了一格--//1, (16 * 6)//未使用
    //载货状态

    //if (host_no_save_para.set_load_status != 0x00)
    if (car_state.bit.carriage != 0x00) {
        if (car_state.bit.carriage == 0x01)
            ptr = state_flag_dis[23];
        else if (car_state.bit.carriage == 0x02)
            ptr = state_flag_dis[24];
        else
            ptr = state_flag_dis[25];

        lcd_dis_one_word(1, (16 * 7), ptr, lcd_dis_type_8x16icon, false);
    }

    //--------------------

    dis_standby_speed_base();

    //--------------------

    i = 0x00; //显示日期跟时间
    temp_buff[i++] = '2';
    temp_buff[i++] = '0';
    temp_buff[i++] = (gps_data.date[0] >> 4) + 0x30; //年
    temp_buff[i++] = (gps_data.date[0] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i++] = '-';
    temp_buff[i++] = (gps_data.date[1] >> 4) + 0x30; //月
    temp_buff[i++] = (gps_data.date[1] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i++] = '-';
    temp_buff[i++] = (gps_data.date[2] >> 4) + 0x30; //日
    temp_buff[i++] = (gps_data.date[2] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i] = 0x00; //结束标志
    lcd_dis_one_line(3, (8 * 0), temp_buff, false);

    i = 0x00;
    temp_buff[i++] = (gps_data.time[0] >> 4) + 0x30; //时
    temp_buff[i++] = (gps_data.time[0] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i++] = ':';
    temp_buff[i++] = (gps_data.time[1] >> 4) + 0x30; //分
    temp_buff[i++] = (gps_data.time[1] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i] = 0x00; //结束标志
    lcd_dis_one_line(3, (8 * 11 + 2), temp_buff, false);

    //--------------------

    //--------------   按键操作   --------------//
    if (key_data.active) //如果有按键按下
    {
        if (key_data.lg) //按键长按操作
        {
#ifdef gb19056_ver //行驶记录仪版本
            //-------------------------------------------------//
            if (key_data.key == key_ok) //进入打印界面
            {
                menu_cnt.menu_other = mo_print;
                menu_cnt.menu_flag = true;       //启动菜单更新
            } else if (key_data.key == key_down) //固件选项快捷界面
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 5;
                menu_cnt.menu_2nd = 5;
                menu_cnt.menu_3rd = 1; //固件选项

                menu_cnt.menu_flag = true; //启动菜单更新
            }
            //  else if (key_data.key == key_ok)  	//拨号界面
            else if (key_data.key == key_esc) //拨号界面
            {
                input_method_init((*(dial_lib_dis + 0)), (phone_data.buff + 2), (*((u16 *)(phone_data.buff))), telephone_num_max_lgth, false);
                phone_data.step = call_dial;
                //
                menu_cnt.menu_other = mo_telephone;
                menu_cnt.menu_flag = true; //启动菜单更新
                _memset((void *)(&phbk_call_in_data), 0x00, sizeof(phbk_call_in_data));
                //
                // menu_cnt.menu_other = mo_print;
                //menu_cnt.menu_flag = true;    	//启动菜单更新
            }

            //-------------------------------------------------//
#else //普通版本 \
      //-------------------------------------------------//
            if (key_data.key == key_p) //进入打印界面
            {
                menu_cnt.menu_other = mo_print;
                menu_cnt.menu_flag = true;       //启动菜单更新
            } else if (key_data.key == key_down) //固件选项快捷界面
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 5;
                menu_cnt.menu_2nd = 5;
                menu_cnt.menu_3rd = 1; //固件选项

                menu_cnt.menu_flag = true;     //启动菜单更新
            } else if (key_data.key == key_ok) //拨号界面
            {
                input_method_init((*(dial_lib_dis + 0)), (phone_data.buff + 2), (*((u16 *)(phone_data.buff))), telephone_num_max_lgth, false);
                phone_data.step = call_dial;

                menu_cnt.menu_other = mo_telephone;
                menu_cnt.menu_flag = true; //启动菜单更新
                _memset((void *)(&phbk_call_in_data), 0x00, sizeof(phbk_call_in_data));
            }
            //-------------------------------------------------//
#endif
        } else //按键短按操作
        {
#ifdef gb19056_ver //行驶记录仪版本
            //-------------------------------------------------//
            if ((key_data.key == key_esc) || (key_data.key == key_ok)) //进入主菜单
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 1;

                menu_cnt.menu_flag = true;     //启动菜单更新
            } else if (key_data.key == key_up) //进入GPS信息界面
            {
                menu_cnt.menu_other = mo_gps_infor1;
                menu_cnt.menu_flag = true; //启动菜单更新
            }
            //-------------------------------------------------//
#else //普通版本 \
      //-------------------------------------------------//
            if (key_data.key == key_ok) //进入主菜单
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 1;

                menu_cnt.menu_flag = true;     //启动菜单更新
            } else if (key_data.key == key_up) //进入GPS信息界面
            {
                // menu_cnt.menu_other = mo_gps_infor1;
                menu_cnt.menu_flag = true; //启动菜单更新
            }
            //-------------------------------------------------//
#endif
        }
    }
    //------------------------------------------//

#ifdef dis_all_flag_test
    gps_data.gprs_alarm_state = gprs_alarm_state_temp;
    gps_data.gps_state = gps_state_temp;
    gps_data.lock_car_state = lock_car_state_temp;
    state_data.state.word_data = state_temp;
#endif
}

static void mo_gps_infor1_hdl(void) {
    static u8 cnt;
    u8 buff[32] = {0};
    //    u16 i;

    switch (cnt) {
    case 0x01: {
        lcd_dis_one_line(0, 1, "经度:", false);

        if (car_state.bit.lng)
            lcd_dis_one_line(1, 1, "西经", false);
        else
            lcd_dis_one_line(1, 1, "东经", false);

        sprintf((char *)buff, "%d", mix.locate.lngi);
        lcd_dis_one_line(1, 40, buff, false);

        lcd_dis_one_line(2, 1, "纬度:", false);
        if (car_state.bit.lat)
            lcd_dis_one_line(3, 1, "南纬", false);
        else
            lcd_dis_one_line(3, 1, "北纬", false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%d", mix.locate.lati);
        lcd_dis_one_line(3, 40, buff, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
    } break;
    case 0x02: {
        if (bd_rcd_io.b.left_light)
            lcd_dis_one_line(0, 1, "左转向:已打开", false);
        else
            lcd_dis_one_line(0, 1, "左转向:已关闭", false);

        if (bd_rcd_io.b.right_light)
            lcd_dis_one_line(1, 1, "右转向:已打开", false);
        else
            lcd_dis_one_line(1, 1, "右转向:已关闭", false);

        if (bd_rcd_io.b.far_light)
            lcd_dis_one_line(2, 1, "近光灯:已打开", false);
        else
            lcd_dis_one_line(2, 1, "近光灯:已关闭", false);

        if (bd_rcd_io.b.near_light)
            lcd_dis_one_line(3, 1, "远光灯:已打开", false);
        else
            lcd_dis_one_line(3, 1, "远光灯:已关闭", false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
    } break;
    case 0x03: //本机ID    短信服务号码
    {
        if (bd_rcd_io.b.brake)
            lcd_dis_one_line(0, 1, "制动:已打开", false);
        else
            lcd_dis_one_line(0, 1, "制动灯:关闭", false);

        sprintf((char *)buff, "%02d", mix.dist);
        lcd_dis_one_line(1, 1, "里程:", false);
        lcd_dis_one_line(1, 40, buff, false);

        if (gnss_cfg.select.b.GPS_EN)
            lcd_dis_one_line(2, 1, "定位模式: GPS", false);
        else if (gnss_cfg.select.b.BD_EN)
            lcd_dis_one_line(2, 1, "定位模式: 北斗", false);
        else
            lcd_dis_one_line(2, 1, "定位模式: 双模", false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%02d", factory_para.set_speed_plus);
        lcd_dis_one_line(3, 0, "脉冲系数: ", false);
        lcd_dis_one_line(3, 72, buff, false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
    } break;

    default:
        cnt = 0x00;
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
        lcd_dis_one_line(0, 1, "定位状态:", false);
        if (car_state.bit.fix)
            lcd_dis_one_line(0, 72, "定位", false);
        else
            lcd_dis_one_line(0, 72, "未定位", false);

        lcd_dis_one_line(1, 1, "定位星数:", false);
        sprintf((char *)buff, "%d", mix.fix_num);
        lcd_dis_one_line(1, 72, buff, false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%d", mix.speed);
        lcd_dis_one_line(2, 1, "GPS速度:", false);
        lcd_dis_one_line(2, 72, buff, false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%d", mix.heigh);
        lcd_dis_one_line(3, 1, "海拔高度:", false);
        lcd_dis_one_line(3, 72, buff, false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
    }

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_down) {
                cnt++;

                if (cnt > 3) {
                    cnt = 3;
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) {
                if (cnt > 0) {
                    cnt--;
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) {
                cnt = 0x00; //退出时设置到第一页
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
    //------------------------------------------//
}

//GPS全信息界面2
static void mo_gps_infor2_hdl(void) {
    u8 buff[20] = {0};
    u16 i;
    //    u16 j;
    u16 n;
    //bool flag;

    i = _strlen((u8 *)(*(gps_dis + 4)));
    _memcpy(buff, (*(gps_dis + 4)), i); //“限速速度”

    n = (0);
    if (n >= 99)
        n = 99;
    buff[i++] = n / 10 + 0x30; //限速速度数值
    buff[i++] = n % 10 + 0x30;
    buff[i++] = 0x00;
    lcd_dis_one_line(0, 0, buff, false);
    /**********************************************************************/
    _memset(buff, 0, 20);
    i = _strlen((u8 *)(*(gps_dis + 5)));
    _memcpy(buff, (*(gps_dis + 5)), i); //“限制举升”
    n = 0x00;
    n += 5;
    if (1) {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n++] = ' ';
    i = _strlen((u8 *)(*(gps_dis + 12)));
    _memcpy((buff + n), (*(gps_dis + 12)), i); //围栏
    n += i;
    if (run.area_in_out == _AREA_IN) {
        i = strlen((char *)(*(state_lib_dis[6] + 1)));
        memcpy((buff + n), (*(state_lib_dis[6] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[6] + 0)));
        memcpy((buff + n), (*(state_lib_dis[6] + 0)), i);
    }
    n += i;
    buff[n] = 0x00;

    lcd_dis_one_line(1, 0, buff, false);

    /**********************************************************************/
    //左盖   右盖
    i = _strlen((u8 *)(*(gps_dis + 2)));
    _memcpy(buff, (*(gps_dis + 2)), i); //左转

    n = 0x00;
    n += i;
    if (pub_io.b.left_cover) {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n++] = ' ';
    i = _strlen((u8 *)(*(gps_dis + 3)));
    _memcpy((buff + n), (*(gps_dis + 3)), i); //右转
    n += i;
    if (pub_io.b.right_cover) {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n] = 0x00;

    lcd_dis_one_line(2, 0, buff, false);

    /**********************************************************************/

    i = _strlen((u8 *)(*(gps_dis + 8)));
    _memcpy(buff, (*(gps_dis + 8)), i); //左转

    n = 0x00;
    n += i;
    if (rcd_io.b.left_light || tr9_car_status.bit.left) {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n++] = ' ';
    i = _strlen((u8 *)(*(gps_dis + 9)));
    _memcpy((buff + n), (*(gps_dis + 9)), i); //右转
    n += i;
    if (rcd_io.b.right_light || tr9_car_status.bit.right) {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n] = 0x00;
    lcd_dis_one_line(3, 0, buff, false);
    lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_esc) {
                dis_goto_standby();
            } else if (key_data.key == key_up) {
                menu_cnt.menu_other = mo_recorder_state;
                menu_cnt.menu_flag = true; //启动菜单更新
            }
        }
    }
    //------------------------------------------//
}

static void mo_recorder_state_hdl(void) {
    uchar buff[20] = {0};
    uint i, n;

    //lcd_area_dis_set(6, 7, 0, 131, 0x00);    //清零最后一行

    n = 0x00;
    i = strlen((char *)(*(recorder_lib_dis + 0)));
    memcpy(buff, (*(recorder_lib_dis + 0)), i);
    n += i;
    if (run.lock_car_sw != 0x55) {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    }
    n += i;
    buff[n++] = ' ';
    i = strlen((char *)(*(recorder_lib_dis + 1)));
    memcpy((buff + n), (*(recorder_lib_dis + 1)), i);
    n += i;

    if (register_cfg.ztb_func == 0x55) {
        if (register_cfg.ztb_gk == 0x55) {
            i = strlen((char *)(*(state_lib_dis[0] + 1)));
            memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
        } else {
            i = strlen((char *)(*(state_lib_dis[0] + 0)));
            memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
        }
    } else {
        i = strlen((char *)(*(state_lib_dis[1] + 0)));
        memcpy((buff + n), (*(state_lib_dis[1] + 0)), i);
    }
    n += i;
    buff[n++] = ' ';
    buff[n++] = 0x00;
    lcd_dis_one_line(0, 0, buff, false); //第一行显示

    //--------------------------------
    n = 0x00;
    i = strlen((char *)(*(recorder_lib_dis + 2)));
    memcpy(buff, (*(recorder_lib_dis + 2)), i);
    n += i;
    if (gps_data.acc_state == 0x00) {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    }
    n += i;
    buff[n++] = ' ';
    i = strlen((char *)(*(recorder_lib_dis + 3)));
    memcpy((buff + n), (*(recorder_lib_dis + 3)), i);
    n += i;
    if (pub_io.b.key_s) {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n++] = ' ';
    buff[n++] = 0x00;
    lcd_dis_one_line(1, 0, buff, false); //第二行显示
                                         //--------------------------------

    n = 0x00;
    i = strlen((char *)(*(recorder_lib_dis + 4)));
    memcpy(buff, (*(recorder_lib_dis + 4)), i); //空重状态
    n += i;
    if (1) //(!testbit(gps_data.car_state, 4))
    {
        i = strlen((char *)(*(state_lib_dis[4] + 0))); //空
        memcpy((buff + n), (*(state_lib_dis[4] + 0)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[4] + 1))); //重
        memcpy((buff + n), (*(state_lib_dis[4] + 1)), i);
    }
    n += i;
    buff[n++] = ' ';
    i = strlen((char *)(*(recorder_lib_dis + 5)));
    memcpy((buff + n), (*(recorder_lib_dis + 5)), i);
    n += i;
    if (pub_io.b.left_cover) //(pub_io.b.left_cover==false)&&(pub_io.b.right_cover==false) ) 		// (ztc_status.car_door)(pub_io.b.car_box == false)
    {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n++] = ' ';
    buff[n++] = 0x00;
    lcd_dis_one_line(2, 0, buff, false); //第三行显示
                                         //--------------------------------

    n = 0x00;
    i = strlen((char *)(*(recorder_lib_dis + 6)));
    memcpy(buff, (*(recorder_lib_dis + 6)), i);
    n += i;

    if (pub_io.b.car_box == true) //显示开
    {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);

    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    }
    n += i;
    buff[n++] = ' ';
    i = strlen((char *)(*(recorder_lib_dis + 7)));
    memcpy((buff + n), (*(recorder_lib_dis + 7)), i);

    n += i;
    if (pub_io.b.high2 == false) {
        i = strlen((char *)(*(state_lib_dis[0] + 0)));
        memcpy((buff + n), (*(state_lib_dis[0] + 0)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[0] + 1)));
        memcpy((buff + n), (*(state_lib_dis[0] + 1)), i);
    }

    n += i;
    buff[n++] = ' ';
    buff[n++] = 0x00;
    lcd_dis_one_line(3, 0, buff, false); //第四行显示

    lcd_dis_one_word(6, (8 * 15 + 4), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_esc) {
                dis_goto_standby();
            } else if (key_data.key == key_up) {
                menu_cnt.menu_other = mo_gps_infor1;
                menu_cnt.menu_flag = true; //启动菜单更新
            }
        }
    }
    //------------------------------------------//
}

static void mo_check_state_hdl(void) {
    ;

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            ;
        }
    }
    //------------------------------------------//
}

static void mo_print_hdl(void) {
    lcd_area_dis_set(0, 7, 0, 131, 0x00);

    lcd_dis_one_line(1, (8 * 1), *(ask_again_dis + 0), false);
    lcd_dis_one_line(2, (8 * 2), *(ask_again_dis + 1), false);

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (!key_data.lg) //按键长按操作
        {
            if (key_data.key == key_ok) //跳转到打印界面
            {
                /*
                print_data.step = step_req;  //启动打印任务

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8*)(&mw_dis)), '\x0', (sizeof(mw_dis)));

                mw_dis.p_dis_2ln = ((u8*)(*(menu_other_lib_dis+17)));
                mw_dis.bg_2ln_pixel = (8*1);
                */

                slave_send_msg((0x220000), 0x00, 0x00, false, spi1_up_comm_team_max);
                dis_goto_standby();
            } else if (key_data.key == key_esc) //退出
            {
                dis_goto_standby();
            }

            menu_cnt.menu_flag = true;
        }
    }
    //------------------------------------------//
}

static void mo_lcd_para_rst_hdl(void) {
    lcd_area_dis_set(0, 7, 0, 131, 0x00);

    lcd_dis_one_line(1, (8 * 1), *(ask_again_dis + 0), false);
    lcd_dis_one_line(2, (8 * 2), *(ask_again_dis + 1), false);

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (!key_data.lg) //按键长按操作
        {
            if (key_data.key == key_ok) //操作成功
            {
                spi_flash_erase(flash_parameter_addr); //擦除参数扇区
                t_rst.rst = true;
                t_rst.del = false;
                t_rst.send = false;
                t_rst.jiff = jiffies;
            } else if (key_data.key == key_esc) //退出
            {
                dis_goto_standby();
            }

            menu_cnt.menu_flag = true;
        }
    }
    //------------------------------------------//
}

static void mo_message_window_hdl(void) {
    lcd_area_dis_set(0, 7, 0, 131, 0x00);

    if (mw_dis.p_dis_1ln) {
        lcd_dis_one_line(0, mw_dis.bg_1ln_pixel, mw_dis.p_dis_1ln, false);
    }
    if (mw_dis.p_dis_2ln) {
        lcd_dis_one_line(1, mw_dis.bg_2ln_pixel, mw_dis.p_dis_2ln, false);
    }
    if (mw_dis.p_dis_3ln) {
        lcd_dis_one_line(2, mw_dis.bg_3ln_pixel, mw_dis.p_dis_3ln, false);
    }
    if (mw_dis.p_dis_4ln) {
        lcd_dis_one_line(3, mw_dis.bg_4ln_pixel, mw_dis.p_dis_4ln, false);
    }

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_esc) //退出界面到上一级规则菜单
            {
                _memset((u8 *)(&menu_auto_exit), 0x00, (sizeof(menu_auto_exit)));

                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
            if (key_data.key == key_ok) //确认
            {
                if (Rk_Task_Manage.RK_6051_state) {
                    Ack_Export = true;
                    rk_6033_task.u_read_flag = 2; //2022-03-17 add by hj
                }
            }
        }
    }
    //------------------------------------------//
}

//通信查询密码
static void mo_1st_6th_1th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;
#if 0 //chwsh
        menu_cnt.menu_other = mo_reset;
        menu_cnt.menu_3rd = 1;	 //进入下一级菜单
#else
        if (/*(!sms_phone_save) && */ (input_method.cnt == (*((u16 *)(&password_manage.para))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.para)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //进入下一级菜单
        } else                     //密码输入错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(2000, true); //退回上一级菜单
        }
#endif
        menu_cnt.menu_flag = true;
    }
}

//通信参数设置密码
static void mo_1st_5th_1st_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;
#if 0 //chwsh
        menu_cnt.menu_other = mo_reset;
        menu_cnt.menu_3rd = 1;	 //进入下一级菜单
#else
        if (/*(!sms_phone_save) && */ (input_method.cnt == (*((u16 *)(&password_manage.para))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.para)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //进入下一级菜单
        } else                     //密码输入错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(2000, true); //退回上一级菜单
        }
#endif
        menu_cnt.menu_flag = true;
    }
}

//油路控制密码
static void mo_1st_5th_4th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.oil))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.oil)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //进入下一级菜单
        } else                     //密码输入错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(2000, true); //退回上一级菜单
        }

        menu_cnt.menu_flag = true;
    }
}

//看车设置密码
static void mo_1st_5th_6th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.car))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.car)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //进入下一级菜单
        } else                     //密码输入错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(2000, true); //退回上一级菜单
        }

        menu_cnt.menu_flag = true;
    }
}

//驱动参数初始化密码
static void mo_1st_5th_8th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.para))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.para)))) == 0)) {
            menu_cnt.menu_other = mo_lcd_para_rst;
        } else //密码输入错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(2000, true); //退回上一级菜单
        }

        menu_cnt.menu_flag = true;
    }
}

/*
//通信参数设置密码
static void mo_1st_5th_9th_password_hdl(void)
{
	if (input_method_hdl())
	{
		if (!(input_method.cnt)) //长度不为0
			return;

		if ((!sms_phone_save)&&(input_method.cnt == (*((u16*)(&password_manage.regist))))
				&&(memcmp((input_method.buff), (input_method.ptr_dest), (*((u16*)(&password_manage.regist)))) == 0))
		{
			menu_cnt.menu_other = mo_reset;
			menu_cnt.menu_3rd = 1;	 //进入下一级菜单
		}
		else	 //密码输入错误
		{
			menu_cnt.menu_other = mo_message_window;
			_memset(((u8*)(&mw_dis)), '\x0', (sizeof(mw_dis)));
			mw_dis.p_dis_2ln = ((u8*)(*(menu_other_lib_dis+1)));
			mw_dis.bg_2ln_pixel = (8*2);		//信息提示窗口设置

			menu_auto_exit_set(2000, true);	//退回上一级菜单
		}

		menu_cnt.menu_flag = true;
	}
}
*/

//传感器系数密码
static void mo_1st_1st_1st_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.sensor))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.sensor)))) == 0)) {
            //  menu_cnt.menu_other = mo_1st_1st_1st_1st;
            //    input_method_init(((*(menu_1st_1st_dis + 0)) + 2), (recorder_para.ratio + 2), (*((u16 *)(recorder_para.ratio))), rec_para_ratio_max_lgth, false);
            //    menu_cnt.menu_flag = true;  //显示更新  //进入传感器系数设置界面
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 0x01; //进入下一级菜单

        } else //密码输入错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(2000, true); //退回上一级菜单
        }

        menu_cnt.menu_flag = true;
    }
}

//速度模式密码//220828速度类型:
static void mo_1st_1st_2nd_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) { //长度不为0
            return;
        }

        if ((input_method.cnt == (*((u16 *)(&password_manage.speed))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.speed)))) == 0)) {
            logd("password of speed type, enter menu ***************************************");
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //进入下一级菜单
        } else {                   //密码输入错误
            loge("password of speed, password err ***************************************");
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(2000, true); //退回上一级菜单
        }

        menu_cnt.menu_flag = true;
    }
}

//上电判断密码是否被修改过
void pwr_up_password_status(void) {
    //mc_password_change = false;
}

static void mo_password_change_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //保存长度
        _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //保存内容
        flash25_program_auto_save((flash_parameter_addr + flash_password_addr), ((u8 *)(&password_manage)), (sizeof(password_manage)));

        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
        mw_dis.bg_2ln_pixel = (8 * 4); //信息提示窗口设置

        menu_auto_exit_set(2000, true); //退回上一级菜单
        menu_cnt.menu_flag = true;
        pwr_up_password_status();
    }
}

//主或备用IP或域名设置
static void mo_1st_5th_1st_1st_hdl(void) {
    u8 tp1;
    u8 set_ok;
    u8 i;
    u8 b_ipBuf[para_backup_ip_max_lgth + 2];

    _strcpy(b_ipBuf, set_para.b_ip);
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        set_ok = 0;

        if (input_method.cnt >= 7) {
            if (input_method.buff[1] == ',') {
                tp1 = 0x00;

                for (i = 0x00; i < (input_method.cnt); i++) {
                    if (input_method.buff[i] == ',')
                        tp1 += 1;
                }

                if (tp1 == 2) {
                    set_ok = 1; //设置的格式正确
                }
            }
        }

        if (set_ok) //设置成功
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //保存长度
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //保存内容
            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

            if ((input_method.ptr_dest) == (set_para.m_ip + 2)) //主IP设置
            {
                slave_send_msg((0x030000), 0x00, 0x00, false, spi1_up_comm_team_max); //设置主IP地址
            } else                                                                    //备用IP设置
            {
                tr9_6044_task();                                                      //RK从平台地址上传
                slave_send_msg((0x410000), 0x00, 0x00, false, spi1_up_comm_team_max); //设置备用IP地址
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //设置类型  报告成功和失败
            menu_cnt.menu_flag = true;
        } else //格式错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
    }
}

//本机ID或短信服务号码设置
static void mo_1st_5th_1st_3rd_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if (!check_num_ascii_error((input_method.buff), (input_method.cnt))) //设置成功
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //保存长度
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //保存内容
            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

            if ((input_method.ptr_dest) == (set_para.id + 2)) //本机ID设置
            {
                slave_send_msg((0x040000), 0x00, 0x00, false, spi1_up_comm_team_max); //设置本机ID
            } else {
                slave_send_msg((0x050000), 0x00, 0x00, false, spi1_up_comm_team_max); //设置短信服务号码
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //设置类型  报告成功和失败
            menu_cnt.menu_flag = true;
        } else //设置格式错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
    }
}

//APN设置
static void mo_1st_5th_1st_4th_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //保存长度
        _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //保存内容
        flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

        slave_send_msg((0x240000), 0x00, 0x00, false, spi1_up_comm_team_max); //设置APN

        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
        mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //设置类型  报告成功和失败
        menu_cnt.menu_flag = true;
    }
}

//注册项相关参数基本设置
static void mo_1st_5th_9th_1st_hdl(void) {
    u8 set_ok;

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        set_ok = 0x00;

        if (((input_method.ptr_dest) == (recorder_para.manufacturer_id + 2))   //制造商ID
            || ((input_method.ptr_dest) == (recorder_para.terminal_type + 2))) //终端型号
        {
            set_ok = 0x01;
        } else if ((input_method.cnt) == (input_method.lgth)) //长度必须符合要求
        {
            if (((input_method.ptr_dest) == (recorder_para.province_id + 2)) //省域ID
                || ((input_method.ptr_dest) == (recorder_para.city_id + 2))) //市县域ID
            {
                if (!check_num_ascii_error((input_method.buff), (input_method.cnt))) {
                    set_ok = 0x01; //必须为数字格式
                }
            } else {
                set_ok = 0x01;
            }
        }

        if (set_ok) //设置成功
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //保存长度
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //保存内容
            flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), ((u8 *)(&recorder_para)), (sizeof(recorder_para)));

            if ((input_method.ptr_dest) == (recorder_para.province_id + 2)) //省域ID
            {
                slave_send_msg((0x600000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.city_id + 2)) //市域ID
            {
                slave_send_msg((0x610000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.terminal_id + 2)) //终端ID
            {
                slave_send_msg((0x630000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.car_vin + 2)) //车辆VIN
            {
                slave_send_msg((0x1c0000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.manufacturer_id + 2)) //制造商ID
            {
                slave_send_msg((0x270000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else //终端型号
            {
                slave_send_msg((0x620000), 0x01, 0x00, false, spi1_up_comm_team_max);
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
            menu_cnt.menu_flag = true;
        } else //设置格式错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
    }
}

//注册项相关参数车牌号码设置
static void mo_1st_5th_9th_7th_hdl(void) {
    if (input_method.cnt < 2) //输入区域简称
    {
        input_method.input_lib.page_max = area_short_input_page_max;
        input_method.input_lib.sig_size = area_short_input_size;
        input_method.input_lib.ptr_lib = area_short_name;
    } else //输入ASCII码
    {
        input_method.input_lib.page_max = ascii_input_page_max;
        input_method.input_lib.sig_size = ascii_input_size;
        input_method.input_lib.ptr_lib = ascii_char_table;
    }

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if (input_method.cnt == rec_car_plate_max_lgth) //设置成功
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //保存长度
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //保存内容
            flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), (u8 *)(&recorder_para), (sizeof(recorder_para)));

            slave_send_msg((0x1d0000), 0x01, 0x00, false, spi1_up_comm_team_max); //设置车牌号码

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
            menu_cnt.menu_flag = true;
        } else //设置格式错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
    }
}

//传感器系数
static void mo_1st_1st_1st_1st_hdl(void) {
    u32 n;

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        n = data_comm_hex((input_method.buff), (input_method.cnt));

        factory_para.set_speed_plus = n;
        menu_cnt.menu_other = mo_message_window;

        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
        mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
        menu_cnt.menu_flag = true;
        menu_report.ok_flag = true;

#if 0
        if (!check_num_ascii_error((input_method.buff), (input_method.cnt))
                && (n <= 65535))  //设置成功
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt; //保存长度
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt);  //保存内容
            flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), ((u8 *)(&recorder_para)), (sizeof(recorder_para)));

            slave_send_msg((0x190000), 0x01, 0x00, false, spi1_up_comm_team_max); //设置传感器系数

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1);    //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
            menu_cnt.menu_flag = true;
        }
        else  //设置格式错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2);    //信息提示窗口设置

            menu_auto_exit_set(1500, true);   //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
#endif
    }
}

//驾驶员相关设置
static void mo_1st_1st_5th_1st_hdl(void) {
    u8 set_ok;

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        set_ok = 0x01;
        if (input_method.ptr_dest == ic_card.driv_lic) {
            if ((input_method.cnt) != 18) //驾驶证号码长度必须为18字节
            {
                set_ok = 0x00;
            }
        }

        if (set_ok) {
            if (input_method.ptr_dest == ic_card.work_num) //驾驶员代码即工号
            {
                ic_card.work_num_lgth = input_method.cnt;
                _memcpy((ic_card.work_num), (input_method.buff), (input_method.cnt));

                up_packeg((0x1f0000), (ic_card.work_num), ic_card.work_num_lgth);
                //slave_send_msg((0x1f0000),0x01,0x00,false,spi1_up_comm_team_max);
            } else //驾驶证号码
            {
                ic_card.driv_lic_lgth = input_method.cnt;
                _memcpy((ic_card.driv_lic), (input_method.buff), (input_method.cnt));
                up_packeg((0x200000), (ic_card.driv_lic), ic_card.driv_lic_lgth);
                //slave_send_msg((0x200000),0x01,0x00,false,spi1_up_comm_team_max);
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
            menu_cnt.menu_flag = true;
        } else //设置格式错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
    }
}

#if (0)
static void initStr_SoftVersion(char *buff, char *version) {
#if (0)
    const char *tmp = "1234567812345678123456781234567";
    int i = strlen(tmp);
    memcpy(buff, tmp, strlen(tmp) + 1);
#else
    int i = strlen(TR9_SOFT_VERSION_APP);
    //logd("VERSION, len = %d", i);
    sprintf((char *)buff, "%s", TR9_SOFT_VERSION_APP);
#endif
}
#endif

static void mo_host_para_hdl(void) {
    static u8 cnt;
    u8 buff[32];
    u16 i;

    if (cnt >= 1 && cnt <= 8) {
        memset(buff, 0x20, 16);
        buff[16] = '\0';
        lcd_dis_one_line(0, 0, buff, false);
        lcd_dis_one_line(1, 0, buff, false);
        lcd_dis_one_line(2, 0, buff, false);
        lcd_dis_one_line(3, 0, buff, false);
    }

    switch (cnt) {
    case 1: //备用IP及端口号
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 1)), false);

        lcd_area_dis_set(4, 7, 0, 131, 0x00);
        lcd_dis_one_page(16, query.host.Backup_ip_lgth, query.host.Backup_ip);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 2: //APN
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 2)), false);

        lcd_area_dis_set(4, 7, 0, 131, 0x00);
        lcd_dis_one_page(16, query.host.apn_lgth, query.host.apn);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 3: //本机ID    短信服务号码
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 3)), false);
        lcd_dis_one_line(1, 0, (query.host.id_num + 1), false);
        lcd_dis_one_line(2, 0, (*(host_state_lib_dis + 4)), false);
        lcd_dis_one_line(3, 0, (query.host.sms_num + 1), false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 4: //数据上传方式    上传间隔
        //lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 5)), false);
        //lcd_dis_one_line(1, 0, *(transmit_mode_lib_dis + (query.host.transmit_mode - 0x80)), false);
        //lcd_dis_one_line(2, 0, (*(host_state_lib_dis + 6)), false); //发送间隔秒

        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 5)), false);

        lcd_area_dis_set(4, 7, 0, 131, 0x00);
        _sprintf_len((char *)query.host.ip3, "暂无");
        query.host.ip3_lgth = 4;
        lcd_dis_one_page(16, query.host.ip3_lgth, query.host.ip3);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 5: //超速报警    断电报警
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 7)), false);
        if (query.host.speed != 0x00) {
            data_mask_ascii(buff, query.host.speed, 3);
            *(buff + 3) = '\x0';
            lcd_dis_one_line(1, 0, data_check_valid(buff), false);
        } else {
            lcd_dis_one_line(1, 1, (*(*(state_lib_dis + 1) + 0)), false);
        }

        lcd_dis_one_line(2, 0, (*(host_state_lib_dis + 8)), false);
        if (query.host.outage != 0x00)
            i = 0x00;
        else
            i = 0x01;
        lcd_dis_one_line(3, 1, (*(*(state_lib_dis + 1) + i)), false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 6: //停车报警    //用户号码
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 9)), false);
        if (query.host.stop != 0x00) {
            data_mask_ascii(buff, query.host.stop, 3);
            *(buff + 3) = '\x0';
            lcd_dis_one_line(1, 1, data_check_valid(buff), false);
        } else {
            lcd_dis_one_line(1, 1, (*(*(state_lib_dis + 1) + 0)), false);
        }

        lcd_dis_one_line(2, 0, (*(host_state_lib_dis + 10)), false);
        lcd_dis_one_line(3, 0, (query.host.user_num + 1), false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 7: {
        //IAP版本
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 11)), false);

#if (TR_USING_IAP)
#if (1) //IAP版本
        if (str_visible(TR9_IAP_VERSION, TR9_SOFT_VERSION_MAX_LEN) == 0) {
            //调度屏扩展协议版本号显示
            lcd_dis_one_page(lcd_dis_one_page_line(1), strlen(TR9_IAP_VERSION), (unsigned char *)TR9_IAP_VERSION);
        } else {
            lcd_dis_one_page(lcd_dis_one_page_line(1), 4, (unsigned char *)"null");
        }
#endif

#if (1) //IAP版本-BUILD TIME
        char *iap_build_time = get_build_time_need_free(TR9_IAP_BUILD_TIME);
        if (iap_build_time != NULL) {
            //调度屏扩展协议版本号显示
            lcd_dis_one_page(lcd_dis_one_page_line(3), strlen(iap_build_time), (unsigned char *)iap_build_time);
            free(iap_build_time);
        } else if (str_visible((const char *)TR9_IAP_BUILD_TIME, 12) == 0) {
            lcd_dis_one_page(lcd_dis_one_page_line(3), 12, (unsigned char *)TR9_IAP_BUILD_TIME);
        } else {
            lcd_dis_one_page(lcd_dis_one_page_line(3), 4, (unsigned char *)"null");
        }
#endif
#else
        lcd_dis_one_page(lcd_dis_one_page_line(1), 4, (unsigned char *)"null");
        lcd_dis_one_page(lcd_dis_one_page_line(3), 4, (unsigned char *)"null");
#endif

#if (1) //显示上下键
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
#endif
        break;
    }
    case 8: {
#pragma region APP版本
        //APP版本
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 12)), false);

#if (1) //APP版本
        if (str_visible(TR9_SOFT_VERSION_APP, TR9_SOFT_VERSION_MAX_LEN) == 0) {
            //调度屏扩展协议版本号显示
            lcd_dis_one_page(lcd_dis_one_page_line(1), strlen(TR9_SOFT_VERSION_APP), (unsigned char *)TR9_SOFT_VERSION_APP);
        } else {
            lcd_dis_one_page(lcd_dis_one_page_line(1), 4, (unsigned char *)"null");
        }
#endif

#if defined(TR_DEBUG) || defined(TR_TEST) //IO: RK.PWR + RK.RST
        unsigned char rk_pin_state[5];
        rk_pin_state[0] = rk_pwr_get() + '0';
        rk_pin_state[1] = rk_rst_get() + '0';
        rk_pin_state[2] = (pub_io.b.key_s == true) + '0';
        rk_pin_state[3] = car_alarm.bit.ant_open + '0';
        rk_pin_state[4] = car_alarm.bit.ant_short + '0';

        lcd_dis_one_page(lcd_dis_one_page_line(2), 5, rk_pin_state);
#endif //#if defined(TR_DEBUG) || defined(TR_TEST) //IO: RK.PWR + RK.RST

#if (1) //APP版本-BUILD TIME
        char *app_build_time = get_build_time_need_free(str_app_build_time);
        if (app_build_time != NULL) {
            //调度屏扩展协议版本号显示
            lcd_dis_one_page(lcd_dis_one_page_line(3), strlen(app_build_time), (unsigned char *)app_build_time);
            free(app_build_time);
        } else if (str_visible(str_app_build_time, 12) == 0) {
            //调度屏扩展协议版本号显示
            lcd_dis_one_page(lcd_dis_one_page_line(3), 12, (unsigned char *)str_app_build_time);
        } else {
            lcd_dis_one_page(lcd_dis_one_page_line(3), 4, (unsigned char *)"null");
        }
#endif

#if (0) //版本
        char *pVersion = malloc(16 * 3);

        if (pVersion != NULL) {
#if (1) //APP版本
            char *p = (char *)TR9_SOFT_VERSION_APP;
            p = "12345678876543211234567887654321";  //32//测试
            p = "12345678876543211234567887654321*"; //33//测试
            int len = strlen(p);

            char *ret_build_time = get_build_time_need_free(str_app_build_time);

            memset((void *)pVersion, ' ', 16 * 3);
            if (len < 30) {
                sprintf(pVersion, "%s", p);
                if (ret_build_time != NULL) {
                    pVersion[len] = ' ';
                }
            } else {
                memcpy((void *)pVersion, (const void *)p, 30);
                if (ret_build_time == NULL) {
                    pVersion[30] = '\0';
                }
            }

            if (ret_build_time != NULL) {
                sprintf(&pVersion[16 * 2], ret_build_time);
                free(ret_build_time);
            }

            len = strlen(pVersion);

            //调度屏扩展协议版本号显示
            lcd_dis_one_page(16, len, (unsigned char *)pVersion);
#endif //APP版本

#if (0)
            //RK版本
            //F060140009 4432303033333130313BF0
            i = 0x00;
            buff[i++] = sHi3520Ver[0];
            buff[i++] = sHi3520Ver[1];
            buff[i++] = sHi3520Ver[2];
            buff[i++] = sHi3520Ver[3];
            buff[i++] = sHi3520Ver[4];
            buff[i++] = sHi3520Ver[5];
            buff[i++] = sHi3520Ver[6];
            buff[i++] = sHi3520Ver[7];
            buff[i++] = sHi3520Ver[8];
            buff[i++] = sHi3520Ver[9];
            buff[i] = '\x0';

            lcd_dis_one_line(1, 0, buff, false); //调度屏标准协议版本号显示
            lcd_area_dis_set(4, 7, 0, 131, 0x00);

            //	lcd_area_dis_set(0, 0, 6, 131, 0x00);
            //i = _strlen((u8 *)(query.host.ver_date2));
            //lcd_dis_one_page(32, i, query.host.ver_date2);  //调度屏扩展协议版本号显示
#endif
        }
#endif  //版本
#if (1) //显示【翻页箭头】
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
#endif
#pragma endregion APP版本
        break;
    }
    default:
        loge("err ln %d", cnt);
#if (1)
        //主IP及端口号
        cnt = 0; //将值改到正确的范围
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 0)), false);

        lcd_area_dis_set(4, 7, 0, 131, 0x00);
        lcd_dis_one_page(16, query.host.ip_lgth, query.host.ip);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
#endif
        break;
    }

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_down) {
                cnt++;

                if (cnt > 8) {
                    cnt = 8;
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) {
                if (cnt > 0) {
                    cnt--;
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) {
                cnt = 0x00; //退出时设置到第一页
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
    //------------------------------------------//
}

static void mo_ic_card_dis_hdl(void) {
    u8 buff[300];
    u8 i;
    u16 n;

    n = 0x00;
    i = _strlen((u8 *)(*(ic_card_dis + 0)));
    _memcpy((buff + n), (*(ic_card_dis + 0)), (i));
    n += i;
    _memcpy((buff + n), (ic_card.name), (ic_card.name_lgth));
    n += (ic_card.name_lgth);
    *(buff + (n++)) = ';'; //姓名
		logd("------test222---姓名,ic_card.name:[%s]-----", ic_card.name);

    i = _strlen((u8 *)(*(ic_card_dis + 1)));
    _memcpy((buff + n), (*(ic_card_dis + 1)), (i));
    n += i;
    _memcpy((buff + n), (ic_card.work_num), (ic_card.work_num_lgth));
    n += (ic_card.work_num_lgth);
    *(buff + (n++)) = ';'; //工号
		logd("------test222---工号,ic_card.work_num:[%s]-----", ic_card.work_num);

    i = _strlen((u8 *)(*(ic_card_dis + 2)));
    _memcpy((buff + n), (*(ic_card_dis + 2)), (i));
    n += i;
    _memcpy((buff + n), (ic_card.id_card), (ic_card.id_card_lgth));
    n += (ic_card.id_card_lgth);
    *(buff + (n++)) = ';'; //身份证	
		logd("------test222---驾驶证,ic_card.id_card:[%s]-----", ic_card.id_card);
#if (0) //驾驶证号：
    i = _strlen((u8 *)(*(ic_card_dis + 3)));
    _memcpy((buff + n), (*(ic_card_dis + 3)), (i));
    n += i;
#endif
		
    _memcpy((buff + n), (ic_card.driv_lic), (ic_card.driv_lic_lgth));
    n += (ic_card.driv_lic_lgth);
    // *(buff + (n++)) = ' ';
		logd("------test222---，ic_card.driv_lic:[%s]-----", ic_card.driv_lic);

#if (0) //有效期：
    i = _strlen((u8 *)(*(ic_card_dis + 6)));
    _memcpy((buff + n), (*(ic_card_dis + 6)), (i));
    n += i;

    data_hex_to_ascii((buff + n), (ic_card.driv_lic_validity), 3);
    n += 6;

    *(buff + (n++)) = ';'; //驾驶证
#endif

    i = _strlen((u8 *)(*(ic_card_dis + 4)));
    _memcpy((buff + n), (*(ic_card_dis + 4)), (i));
    n += i;
    _memcpy((buff + n), (ic_card.certificate), (ic_card.certificate_lgth));
    n += (ic_card.certificate_lgth);
    *(buff + (n++)) = ';'; //从业资格证
		logd("------test222---从业资格证号，ic_card.certificate:[%s]-----", ic_card.certificate);

#if (0) //发证机构：
    i = _strlen((u8 *)(*(ic_card_dis + 5)));
    _memcpy((buff + n), (*(ic_card_dis + 5)), (i));
    n += i;
#endif

    _memcpy((buff + n), (ic_card.license_organ), (ic_card.license_organ_lgth));
    n += (ic_card.license_organ_lgth);
    //*(buff + (n++)) = ' ';
    i = _strlen((u8 *)(*(ic_card_dis + 6)));
    _memcpy((buff + n), (*(ic_card_dis + 6)), (i));
    n += i;
    data_hex_to_ascii((buff + n), (ic_card.license_organ_validity), 3);
    n += 6;
    *(buff + (n++)) = ';'; //发证机构

    dis_multi_page(buff, n, false, NULL);
}

/*
static void mo_slave_ver_check_hdl(void)
{
	static u8  cnt;
	inflash_para_struct  para;

	stm32_flash_read(boot_para_address, ((u8*)(&para)), (sizeof(para)));
	lcd_area_dis_set(6, 7, 0, 131, 0x00);

	if (cnt == 0x00) //第一页
	{
		lcd_dis_one_line(0, 0, (*(slave_ver_dis+0)), false);
		lcd_dis_one_page(16, para.bl_name_lgth, para.bl_buff);

		lcd_dis_one_word(6, (8*15), (*(up_down_flag+1)), lcd_dis_type_8x16ascii, false);
	}
	else  //第二页
	{
		lcd_dis_one_line(0, 0, (*(slave_ver_dis+1)), false);
		lcd_dis_one_page(16, para.ap_name_lgth, para.ap_buff);

		lcd_dis_one_word(6, (8*15), (*(up_down_flag+0)), lcd_dis_type_8x16ascii, false);
	}

	//--------------   按键操作   --------------//
	if (key_data.active)
	{
		if (key_data.lg) //按键长按操作
		{
			;
		}
		else   //按键短按操作
		{
			if (key_data.key == key_down)
			{
				if (cnt == 0)
				{
					cnt = 1;
				}

				menu_cnt.menu_flag = true;
			}
			else if (key_data.key == key_up)
			{
				if (cnt == 1)
				{
					cnt = 0;
				}

				menu_cnt.menu_flag = true;
			}
			else if (key_data.key == key_esc)
			{
				cnt = 0x00;  //退出时设置到第一页

				menu_cnt.menu_other = mo_reset;
				menu_cnt.menu_flag = true;
			}
		}
	}
	//------------------------------------------//
}
*/

//查询CSQ信号值
static void mo_csq_vol_hdl(void) {
    u8 tmp;
    u8 buff[3];

    tmp = (gps_data.csq_state & 0x7f);
    if (tmp > 99) //防止超过范围
    {
        tmp = 99;
    }

    buff[0] = tmp / 10 + '0';
    buff[1] = tmp % 10 + '0';
    buff[2] = 0x00; //放置结束标志  CKP  2015-03-30

    lcd_dis_one_line(0, 0, "CSQ:", false);
    lcd_dis_one_line(0, 8 * 5, buff, false);

    //--------------   按键操作   --------------//

    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_esc) {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
}

//自检信息
static void mo_auto_check_hdl(void) {
    static u8 cnt;
    const u8 *ptr;
    //   u8  temp, buff[5];
    EN_SELF_CHECK;
    switch (cnt) {
    case 0x01:
        lcd_dis_one_line(0, (8 * 0), *(auto_check_lib_dis + 2), false);
        lcd_dis_one_line(2, (8 * 0), *(auto_check_lib_dis + 1), false);

        if (query.auto_check.gps_state == '1')
            ptr = *(auto_check_lib_dis + 24);
        else
            ptr = *(auto_check_lib_dis + 25);
        lcd_dis_one_line(1, (8 * 0), ptr, false);
        /*
            if (query.auto_check.board_state == '1')
                ptr = *(auto_check_lib_dis + 26);
            else
                ptr = *(auto_check_lib_dis + 27);
            lcd_dis_one_line(3, (8 * 0), ptr, false);
*/
        switch (query.auto_check.gsm_state) {
        case '0':
            ptr = *(auto_check_lib_dis + 18);
            break;
        case '1':
            ptr = *(auto_check_lib_dis + 19);
            break;
        case '2':
            ptr = *(auto_check_lib_dis + 20);
            break;
        case '3':
            ptr = *(auto_check_lib_dis + 21);
            break;
        case '4':
            ptr = *(auto_check_lib_dis + 22);
            break;
        case '5':
            ptr = *(auto_check_lib_dis + 23);
            break;
        default:
            ptr = *(auto_check_lib_dis + 18);
        }
        lcd_dis_one_line(3, (8 * 0), ptr, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    default:
        lcd_dis_one_line(0, (8 * 0), *(auto_check_lib_dis + 2), false);
        lcd_dis_one_line(2, (8 * 0), *(auto_check_lib_dis + 1), false);

        if (query.auto_check.gps_state == '1')
            ptr = *(auto_check_lib_dis + 24);
        else
            ptr = *(auto_check_lib_dis + 25);
        lcd_dis_one_line(1, (8 * 0), ptr, false);
        /*
			if (query.auto_check.board_state == '1')
				ptr = *(auto_check_lib_dis + 26);
			else
				ptr = *(auto_check_lib_dis + 27);
			lcd_dis_one_line(3, (8 * 0), ptr, false);
*/
        switch (query.auto_check.gsm_state) {
        case '0':
            ptr = *(auto_check_lib_dis + 18);
            break;
        case '1':
            ptr = *(auto_check_lib_dis + 19);
            break;
        case '2':
            ptr = *(auto_check_lib_dis + 20);
            break;
        case '3':
            ptr = *(auto_check_lib_dis + 21);
            break;
        case '4':
            ptr = *(auto_check_lib_dis + 22);
            break;
        case '5':
            ptr = *(auto_check_lib_dis + 23);
            break;
        default:
            ptr = *(auto_check_lib_dis + 18);
        }
        lcd_dis_one_line(3, (8 * 0), ptr, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;
    }

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_down) {
                cnt++;

                if (cnt > 3) {
                    cnt = 3;
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) {
                if (cnt > 0) {
                    cnt--;
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) {
                cnt = 0x00; //退出时设置到第一页
                DIS_SELF_CHECK;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
    //------------------------------------------//
}

//行驶记录仪参数查询
//共四项
#define query_rec_wait_time_max ((u16)5000) //单步等待时间
static void mo_query_rec_hdl(void) {
    u8 buff[20], *ptr;

    switch (query.rec.step) {
        //-----------    查询操作      ---------//

    case 0x00:                                                          //显示查询界面并开始查询第一个参数  //传感器系数
        lcd_dis_one_line(1, (8 * 1), *(menu_other_lib_dis + 4), false); //正在查询，请稍候

        slave_send_msg((0x190000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "4", false);
        query.rec.step = 0x01; //等待应答
        break;

    case 0x01: //等待
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x02: //速度模式
        slave_send_msg((0x00210000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "3", false);
        query.rec.step = 0x03; //等待应答
        break;

    case 0x03: //等待
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x04: //打印标准选择
        slave_send_msg((0x6a0000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "2", false);
        query.rec.step = 0x05; //等待应答
        break;

    case 0x05: //等待
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x06: //号牌类型
        slave_send_msg((0x660000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "1", false);
        query.rec.step = 0x07; //等待应答
        break;

    case 0x07: //等待
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

        //----------    显示操作       ---------//

    case 0x08: //显示第一界面
    case 0x20:
        query.rec.step = 0x20;

        lcd_dis_one_line(0, 0, (menu_1st_1st_dis[0] + 2), false);

        _memcpy(buff, (query.rec.ratio), rec_para_ratio_max_lgth);
        buff[rec_para_ratio_max_lgth] = 0x00;
        ptr = data_check_valid(buff);
        lcd_dis_one_line(1, 0, (ptr), false);

        lcd_dis_one_line(2, 0, (menu_1st_1st_dis[1] + 2), false);

        lcd_dis_one_line(3, 0, (menu_ztc_1st_2nd_dis[query.rec.speed_type - 1] + 2), false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
        break;

    case 0x21: //显示第二界面
        lcd_dis_one_line(0, 0, (menu_1st_1st_dis[2] + 2), false);

        lcd_dis_one_line(1, 0, (menu_1st_1st_3rd_dis[query.rec.print_type - 1] + 2), false);

        lcd_dis_one_line(2, 0, (menu_1st_1st_dis[3] + 2), false);

        _memcpy(buff, (query.rec.car_type), rec_car_type_max_lgth);
        buff[rec_car_type_max_lgth] = 0x00;
        lcd_dis_one_line(3, 0, (buff), false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
        break;

        //-----------    超时处理    ---------//

    case 0xf0: //超时处理
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[1][0]));
        mw_dis.bg_2ln_pixel = (8 * 4); //信息提示窗口设置

        menu_auto_exit_set(2000, true); //退回上一级菜单
        menu_cnt.menu_flag = true;
        break;

    default:
        query.rec.step = 0x00;
    }

    if (query.rec.step < 0x20) //开始显示后不用每次都刷新
    {
        menu_cnt.menu_flag = true;
    } else //按键的处理
    {
        if ((key_data.active) && (!key_data.lg)) //仅有短按有效
        {
            if (key_data.key == key_up) //上翻页
            {
                if (query.rec.step == 0x21) {
                    query.rec.step = 0x20;
                }
            } else if (key_data.key == key_down) //下翻页
            {
                if (query.rec.step == 0x20) {
                    query.rec.step = 0x21;
                }
            } else if (key_data.key == key_esc) //退出操作
            {
                menu_cnt.menu_other = mo_reset;
            }

            menu_cnt.menu_flag = true;
        }
    }
}

//注册参数查询
//共八项
#define query_regist_wait_time_max ((u16)5000) //单步等待时间
static void mo_query_reg_hdl(void) {
    u8 buff[40];

    //    char* ptm_1;
    //    u8 len1;

    switch (query.regist.step) {
        //-----------	 查询操作	 ---------//

    case 0x00:                                                          //显示查询界面并开始查询第一个参数  //省域ID
        lcd_dis_one_line(1, (8 * 1), *(menu_other_lib_dis + 4), false); //正在查询，请稍候

        slave_send_msg((0x600000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "8", false);
        query.regist.step = 0x01; //等待应答
        break;

    case 0x01: //等待
        if (sub_u16(jiffies, (query.regist.tim)) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x02: //市县域ID
        slave_send_msg((0x610000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "7", false);
        query.regist.step = 0x03; //等待应答
        break;

    case 0x03: //等待
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x04: //制造商ID
        slave_send_msg((0x270000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "6", false);
        query.regist.step = 0x05; //等待应答
        break;

    case 0x05: //等待
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x06: //终端型号
        slave_send_msg((0x620000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "5", false);
        query.regist.step = 0x07; //等待应答
        break;

    case 0x07: //等待
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x08: //终端ID
        slave_send_msg((0x630000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "4", false);
        query.regist.step = 0x09; //等待应答
        break;

    case 0x09: //等待
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x0a: //车牌颜色
        slave_send_msg((0x260000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "3", false);
        query.regist.step = 0x0b; //等待应答
        break;

    case 0x0b: //等待
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x0c: //车牌号码
        slave_send_msg((0x1d0000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "2", false);
        query.regist.step = 0x0d; //等待应答
        break;

    case 0x0d: //等待
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x0e: //车辆VIN
        slave_send_msg((0x1c0000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "1", false);
        query.regist.step = 0x0f; //等待应答
        break;

    case 0x0f: //等待
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

        //----------    显示操作    ---------//

    case 0x10: //第一显示界面  省域ID，市县域ID
    case 0x20:
        query.regist.step = 0x20;

        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[0] + 2), false);

        _memcpy(buff, (query.regist.province_id), rec_province_id_max_lgth);
        buff[rec_province_id_max_lgth] = 0x00;
        lcd_dis_one_line(1, 0, buff, false);

        lcd_dis_one_line(2, 0, (menu_1st_5th_9th_dis[1] + 2), false);

        _memcpy(buff, (query.regist.city_id), rec_city_id_max_lgth);
        buff[rec_city_id_max_lgth] = 0x00;
        lcd_dis_one_line(3, 0, buff, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
        break;

    case 0x21: //第二显示界面  制造商ID，终端ID
        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[2] + 2), false);

        _memcpy(buff, (query.regist.manufacturer_id), rec_manufacturer_id_max_lgth);
        buff[rec_manufacturer_id_max_lgth] = 0x00;
        lcd_dis_one_line(1, 0, buff, false);

        lcd_dis_one_line(2, 0, (menu_1st_5th_9th_dis[4] + 2), false);

        _memcpy(buff, (query.regist.terminal_id), rec_terminal_id_max_lgth);
        buff[rec_terminal_id_max_lgth] = 0x00;
        lcd_dis_one_line(3, 0, buff, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 0x22: //第三显示界面  车牌颜色，车牌号码
        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[5] + 2), false);

        lcd_dis_one_line(1, 0, (menu_1st_5th_9th_6th_dis[query.regist.car_plate_color - 1] + 2), false);

        lcd_dis_one_line(2, 0, (menu_1st_5th_9th_dis[6] + 2), false);

        _memcpy(buff, (query.regist.car_plate), rec_car_plate_max_lgth); //车牌号码
        buff[rec_car_plate_max_lgth] = 0x00;
        lcd_dis_one_line(3, 0, buff, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 0x23: //第四显示界面  终端型号
        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[3] + 2), false);
        lcd_area_dis_set(2, 7, 0, 131, 0x00);

        _memcpy(buff, (query.regist.terminal_type), rec_terminal_type_max_lgth);
        buff[rec_terminal_type_max_lgth] = 0x00;
        lcd_dis_one_page(16, rec_terminal_type_max_lgth, buff);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 0x24: //第五显示界面  车辆VIN码
        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[7] + 2), false);
        lcd_area_dis_set(2, 7, 0, 131, 0x00);

        _memcpy(buff, (query.regist.car_vin), rec_car_vin_max_lgth);
        buff[rec_car_vin_max_lgth] = 0x00;
        lcd_dis_one_page(16, rec_car_vin_max_lgth, buff);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
        break;

        //-----------    超时处理    ---------//

    case 0xf0: //超时处理
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[1][0]));
        mw_dis.bg_2ln_pixel = (8 * 4); //信息提示窗口设置

        menu_auto_exit_set(2000, true); //退回上一级菜单
        menu_cnt.menu_flag = true;
        break;

    default:
        query.rec.step = 0x00;
    }

    if (query.regist.step < 0x20) //开始显示后不用每次都刷新
    {
        menu_cnt.menu_flag = true;
    } else //按键的处理
    {
        if ((key_data.active) && (!key_data.lg)) //仅有短按有效
        {
            if (key_data.key == key_up) //上翻页
            {
                if (query.regist.step > 0x20) {
                    query.regist.step -= 1;
                }
            } else if (key_data.key == key_down) //下翻页
            {
                if (query.regist.step < 0x24) {
                    query.regist.step += 1;
                }
            } else if (key_data.key == key_esc) //退出操作
            {
                menu_cnt.menu_other = mo_reset;
            }

            menu_cnt.menu_flag = true;
        }
    }
}

//驾驶员参数查询
//共两项
#define query_driver_wait_time_max ((u16)5000) //单步等待时间
static void mo_query_driver_hdl(void) {
    u8 i, n;
    u8 buff[65];

    switch (query.driver.step) {
        //-----------    查询操作    ---------//

    case 0x00:                             //显示查询界面并开始查询第一个参数  //驾驶员代码查询
        if (state_data.state.flag.ic_flag) //若IC卡已经插入
        {
            query.driver.step = 0x20;
            menu_cnt.menu_flag = true;
        } else //若未插入IC卡，则需要从主机查询
        {
            lcd_dis_one_line(1, (8 * 1), *(menu_other_lib_dis + 4), false); //正在查询，请稍候

            slave_send_msg((0x1f0000), 0x00, 0x00, false, spi1_up_comm_team_max);
            query.driver.tim = jiffies; //时间同步

            lcd_dis_one_line(3, 0, "2", false);
            query.driver.step = 0x01; //等待应答
        }
        break;

    case 0x01: //等待
        if (sub_u16(jiffies, (query.driver.tim)) >= query_driver_wait_time_max) {
            query.driver.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x02: //驾驶证号码查询
        slave_send_msg((0x200000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.driver.tim = jiffies; //时间同步

        lcd_dis_one_line(3, 0, "1", false);
        query.driver.step = 0x03; //等待应答
        break;

    case 0x03: //等待
        if (_pasti(query.driver.tim) >= query_driver_wait_time_max) {
            query.driver.step = 0xf0; //超时处理
            menu_cnt.menu_flag = true;
        }
        break;

        //----------    显示操作    ---------//

    case 0x04:
    case 0x20:
        query.driver.step = 0x20;
        n = 0x00;

        i = str_len((u8 *)(*(menu_1st_1st_5th_dis + 0)), 16);
        i -= 2;
        _memcpy((buff + n), ((*(menu_1st_1st_5th_dis + 0)) + 2), (i));
        n += i;
        *(buff + (n++)) = ':';
        _memcpy((buff + n), (ic_card.work_num), (ic_card.work_num_lgth));
        n += (ic_card.work_num_lgth);
        *(buff + (n++)) = ';'; //驾驶员代码

        i = str_len((u8 *)(*(menu_1st_1st_5th_dis + 1)), 16);
        i -= 2;
        _memcpy((buff + n), ((*(menu_1st_1st_5th_dis + 1)) + 2), (i));
        n += i;
        *(buff + (n++)) = ':';
        _memcpy((buff + n), (ic_card.driv_lic), (ic_card.driv_lic_lgth));
        n += (ic_card.driv_lic_lgth);
        *(buff + (n++)) = ';'; //驾驶员代码

        lcd_area_dis_set(4, 7, 0, 131, 0x00); //清屏后两行
        lcd_dis_one_page(0x00, n, buff);
        break;

        //-----------    超时处理    ---------//

    case 0xf0: //超时处理
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[1][0]));
        mw_dis.bg_2ln_pixel = (8 * 4); //信息提示窗口设置

        menu_auto_exit_set(2000, true); //退回上一级菜单
        menu_cnt.menu_flag = true;
        break;

    default:
        query.driver.step = 0x00;
    }

    if (query.driver.step < 0x20) //开始显示后不用每次都刷新
    {
        menu_cnt.menu_flag = true;
    } else //按键的处理  //仅有退出键有效
    {
        if ((key_data.active) && (!key_data.lg)) //仅有短按有效
        {
            if (key_data.key == key_esc) //退出键
            {
                menu_cnt.menu_other = mo_reset;
            }
            menu_cnt.menu_flag = true;
        }
    }
}

static void mo_update_tf_main_hdl(void) {
    u32 y;

    if (update_tf_data.step > 0x01) {
        y = (update_tf_data.cnt * 16);
        y = (y / update_tf_data.wr_lgth);
        ;
        lcd_dis_one_word(4, (y * 8), (*(state_flag_dis + 1)), lcd_dis_type_8x16ascii, false);
    }
    //update_tf_main();
}

static void mo_update_font_lib_hdl(void) {
    u32 y;

    if (font_lib_update.step == 0x02) {
        y = (font_lib_update.cnt * 16);
        y = (y / font_lib_update.wr_lgth);
        lcd_dis_one_word(4, (y * 8), (*(state_flag_dis + 1)), lcd_dis_type_8x16ascii, false);
    }

    //update_font_lib();
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/* 自动参数配置函数 2017-12-6*/
static void mo_cfg_parameter_hdl(void) {
}

static void mo_read_rcd_data_hal(void) {
}

static void mo_auto_init_hal(void) {
}

//超时记录
static void mo_timeout_driving_hal(void) {
    static u8 cnt;
    u8 buff[32] = {0};
    //u16 i, len;
    u16 ret;
    bool flag = false;

    switch (cnt) {
    case 0: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
        if (fatigue_print.cell[0].write_ed != 0) {
            ret = _verify_time(&fatigue_print.cell[0].start);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "无超时记录", false);
                flag = true;
                break;
            }
            ret = _verify_time(&fatigue_print.cell[0].end);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "无超时记录", false);
                flag = true;
                break;
            }
            if (ret) {
                lcd_dis_one_line(0, 1, "超时记录一:", false);
                lcd_dis_one_line(1, 1, "驾驶证号:", false);
                lcd_dis_one_line(2, 1, fatigue_print.cell[0].driver_lse, false);
                lcd_dis_one_line(3, 1, "开始驾驶时间:", false);
            }
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
        } else {
            lcd_dis_one_line(0, 1, "无超时记录", false);
            flag = true;
        }
    } break;
    case 1: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00);
        _memset(buff, 0, 32);
        _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[0].start.year, fatigue_print.cell[0].start.month,
                     fatigue_print.cell[0].start.date, fatigue_print.cell[0].start.hour, fatigue_print.cell[0].start.min, fatigue_print.cell[0].start.sec);

        lcd_dis_one_line(0, 1, buff, false);
        lcd_dis_one_line(1, 1, "结束驾驶时间:", false);

        _memset(buff, 0x00, 32);
        _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[0].end.year, fatigue_print.cell[0].end.month,
                     fatigue_print.cell[0].end.date, fatigue_print.cell[0].end.hour, fatigue_print.cell[0].end.min, fatigue_print.cell[0].end.sec);
        lcd_dis_one_line(2, 1, buff, false);
        lcd_dis_one_line(3, 1, "超时记录二", false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);

    } break;
    case 2: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00);
        if (fatigue_print.cell[1].write_ed != 0) {
            ret = _verify_time(&fatigue_print.cell[1].start);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "超时记录二:无记录", false);
                flag = true;
                break;
            }
            ret = _verify_time(&fatigue_print.cell[1].end);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "超时记录二:无记录", false);
                flag = true;
                break;
            }
            if (ret) {
                lcd_dis_one_line(0, 1, "驾驶证号:", false);
                lcd_dis_one_line(1, 1, fatigue_print.cell[1].driver_lse, false);
                lcd_dis_one_line(2, 1, "开始驾驶时间:", false);

                _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[1].start.year, fatigue_print.cell[1].start.month,
                             fatigue_print.cell[1].start.date, fatigue_print.cell[1].start.hour, fatigue_print.cell[1].start.min, fatigue_print.cell[1].start.sec);
                lcd_dis_one_line(3, 1, buff, false);
                lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
            }
        } else {
            lcd_dis_one_line(0, 1, "超时记录二:无记录", false);
            flag = true;
            break;
        }

    } break;
    case 3: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00);
        lcd_dis_one_line(1, 1, "结束驾驶时间:", false);

        _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[1].end.year, fatigue_print.cell[1].end.month,
                     fatigue_print.cell[1].end.date, fatigue_print.cell[1].end.hour, fatigue_print.cell[1].end.min, fatigue_print.cell[1].end.sec);
        lcd_dis_one_line(2, 1, buff, false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
    } break;
    default:
        break;
    }
    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_down) {
                if (!flag)
                    cnt++;

                if (cnt > 3) {
                    cnt = 3;
                }
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) {
                if ((cnt > 0) && (!flag)) {
                    cnt--;
                }
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) {
                cnt = 0x00; //退出时设置到第一页
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
}
//速度记录
static void mo_speed_record_hal(void) {
    static u8 cnt;
    u8 buff[70] = {0};
    u16 i, len = 0;
    //u16 ret;
    bool flag = false;

    switch (cnt) {
    case 0: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
        for (i = 0; i < 4; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_速度:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);
        lcd_dis_one_line(2, 1, buff + 32, false);
        lcd_dis_one_line(3, 1, buff + 48, false);
    } break;
    case 1: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
        for (i = 4; i < 8; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_速度:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);
        lcd_dis_one_line(2, 1, buff + 32, false);
        lcd_dis_one_line(3, 1, buff + 48, false);

    } break;
    case 2: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
        for (i = 8; i < 12; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_速度:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);
        lcd_dis_one_line(2, 1, buff + 32, false);
        lcd_dis_one_line(3, 1, buff + 48, false);
    } break;
    case 3: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
        for (i = 12; i < 15; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_速度:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);

    } break;
    default: break;
    }
    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_down) {
                if (!flag)
                    cnt++;

                if (cnt > 3) {
                    cnt = 3;
                }
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) {
                if ((cnt > 0) && (!flag)) {
                    cnt--;
                }
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) {
                cnt = 0x00; //退出时设置到第一页
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
}

#if (0)
static void mo_sms_phone(void) {
    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_down) {
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) {
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) {
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
}
#endif

//重量检测显示 油量显示
static void mo_oil_test_hdl(void) {
    u16 da;
    u8 temp[6];

    _memset(temp, 0x00, 6);
    da = (gps_data.oil_data[0] >> 4) * 1000 + (gps_data.oil_data[0] & 0x0f) * 100;
    da += (gps_data.oil_data[1] >> 4) * 10 + (gps_data.oil_data[1] & 0x0f);

    data_mask_ascii(temp + 1, da, 4);
    if (auto_weight.zf)
        temp[0] = '+';
    else
        temp[0] = '+';
    lcd_dis_one_line(2, (6 * 8), data_check_valid(temp), false);

    //--------------   按键操作   --------------//
    if (key_data.active) {
        if (key_data.lg) //按键长按操作
        {
            ;
        } else //按键短按操作
        {
            if (key_data.key == key_esc) {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
    //------------------------------------------//
}

static void mo_telephone_hdl(void) {
    u8 temp[36];
    u8 m;
    //u8 i;
    phbk_count_struct phbk_cnt;

    switch (phone_data.step) {
    case call_free: //空闲状态下立即返回到菜单界面
        menu_cnt.menu_other = mo_reset;
        menu_cnt.menu_flag = true;
        break;

    case call_dial: //手动拨号处理
        if (input_method_hdl()) {
            if (!(input_method.cnt)) //长度不为0
                return;              //当手动输入的号码为0时返回

            if (check_num_ascii_error(input_method.buff, input_method.cnt)) //输入格式错误
            {
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 30)));
                mw_dis.bg_2ln_pixel = (8 * 4); //信息提示窗口设置

                menu_auto_exit_set(1500, false); //退回待机界面
            } else                               //启动拨号
            {
                spi_flash_read(((u8 *)(&phbk_cnt)), phbk_start_addr_count, (sizeof(phbk_count_struct)));
                if ((phbk_cnt.total == 0x00) || (memcmp(&input_method.buff[0], "10086", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "10000", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "110", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "119", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "120", input_method.cnt) == 0x00)) {
                    /*
                        *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt; //保存长度
                        _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt);  //保存内容
                        */

                    *(input_method.ptr_dest) = input_method.cnt;                                 //保存长度
                    _memcpy((input_method.ptr_dest + 2), (input_method.buff), input_method.cnt); //保存内容

                    lcd_area_dis_set(6, 7, 0, 131, 0x00);
                    lcd_dis_one_line(0, 0, (*(dial_lib_dis + 1)), false);
                    phone_data.step = call_out;
                    phbk_call_in_data.num.lgth = phone_data.buff[0];
                    _memcpy(phbk_call_in_data.num.buff, (&phone_data.buff[2]), phbk_call_in_data.num.lgth);
                    write_data_to_call_record_flash(flash_call_out_record_addr, phbk_call_in_data);
                    // slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //通知主机拨号
                    logd("拨打号码 %s", input_method.buff);
                    tr9_frame_pack2rk(0x6048, (u8 *)input_method.buff, input_method.cnt);
                } else {
                    menu_cnt.menu_other = mo_standby;
                    menu_cnt.menu_flag = true;
                    phone_data.step = call_free;
                }
            }
            menu_cnt.menu_flag = true;
        }
        break;

    case call_out:

        //--------------   按键操作   --------------//
        if (key_data.active) {
            if (!(key_data.lg)) //按键短按
            {
                if (key_data.key == key_esc) {
                    phone_data.step = call_halt;
                    menu_cnt.menu_flag = true;
                }
            }
        }
        //------------------------------------------//
        break;

    case call_in:

        //--------------   按键操作   --------------//
        if (key_data.active) {
            if (!(key_data.lg)) //按键短按
            {
                if (key_data.key == key_esc) {
                    phone_data.step = call_halt;
                    menu_cnt.menu_flag = true;
                } else if (key_data.key == key_ok) {
                    phone_data.dtmf_ptr = 0x00;
                    phone_data.dtmf_cnt = 0x00; //清零计数器
                    phone_data.tim = jiffies;   //时间同步

                    phone_data.step = call_connect;
                    menu_cnt.menu_flag = true;

                    slave_send_msg((0x0e0000), 0x00, 0x00, false, spi1_up_comm_team_max); //通知主机接电话
                }
            }
        }
        //------------------------------------------//
        break;
    case call_connect:
        dis_time(sub_u32(jiffies, phone_data.tim));
        lcd_dis_one_page(32, phone_data.dtmf_cnt, phone_data.dtmf_buff);
        lcd_dis_one_line(3, 0, (*(dial_lib_dis + 2)), false);

        m = *((*(dial_lib_dis + 2)) + phone_data.dtmf_ptr);
        read_flash_fonts(m, temp);
        lcd_dis_one_word((3 * 2), (8 * phone_data.dtmf_ptr), temp, lcd_dis_type_8x16ascii, true);

        //--------------   按键操作   --------------//
        if (key_data.active) {
            if (!(key_data.lg)) //按键短按
            {
                if (key_data.key == key_esc) {
                    phone_data.step = call_halt;
                    menu_cnt.menu_flag = true;
                } else if (key_data.key == key_ok) {
                    if (phone_data.dtmf_cnt >= telephone_num_max_lgth) {
                        phone_data.dtmf_cnt = (telephone_num_max_lgth - 1);
                        array_move((phone_data.dtmf_buff + 1), 15, false, array_uchar_move_left);
                    }

                    *(phone_data.dtmf_buff + phone_data.dtmf_cnt) = *((*(dial_lib_dis + 2)) + phone_data.dtmf_ptr);
                    phone_data.dtmf_cnt++;

                    menu_cnt.menu_flag = true;
                    slave_send_msg((0x090000), 0x00, 0x00, false, spi1_up_comm_team_max); //发送DTMF
                } else if (key_data.key == key_up) {
                    phone_data.dtmf_ptr--;
                    if (phone_data.dtmf_ptr > 11) {
                        phone_data.dtmf_ptr = 11;
                    }

                    menu_cnt.menu_flag = true;
                } else if (key_data.key == key_down) {
                    phone_data.dtmf_ptr++;
                    if (phone_data.dtmf_ptr > 11) {
                        phone_data.dtmf_ptr = 0x00;
                    }

                    menu_cnt.menu_flag = true;
                }
            }
        }
        //------------------------------------------//
        break;

    case call_halt:
        m = 0;
        slave_send_msg((0x080000), 0x00, 0x00, false, spi1_up_comm_team_max); //通知主机挂机
        tr9_frame_pack2rk(0x6048, &m, 1);
    case call_end:
    default:
        phone_data.step = call_free;
        menu_cnt.menu_other = mo_reset;
        menu_cnt.menu_flag = true;
    }
}

static void mo_sms_fix_input_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if (input_method.buff[0] == '1') //只有输入1才有效
        {
            slave_send_msg((0xee0301), 0x00, 0x00, false, spi1_up_comm_team_max); //上传事件

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 6)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 4), rpt_average, true); //一般类型  报告成功和失败
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        } else //输入内容错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        }
    }
}

static void mo_sms_vod_input_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if ((input_method.buff[0] == '1') || (input_method.buff[0] == '0')) //只有输入1或0才有效
        {
            slave_send_msg((0xee0303), 0x00, 0x00, false, spi1_up_comm_team_max); //信息点播

            if (input_method.buff[0] == '1') {
                sms_up_center.vod_status = 0x01; //确认点播
            } else {
                sms_up_center.vod_status = 0x00; //取消点播
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 7)));
            mw_dis.bg_2ln_pixel = (8 * 0); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 4), rpt_average, true); //一般类型  报告成功和失败
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        } else //输入内容错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
    }
}

static void mo_sms_ask_input_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
            return;

        if ((input_method.buff[0] >= '0') && (input_method.buff[0] < (0x30 + sms_union_data.ask.ans_cnt))) //只有输入数字且在范围之内才有效
        {
            slave_send_msg((0xee0302), 0x00, 0x00, false, spi1_up_comm_team_max); //信息点播

            sms_up_center.ask_id_point = (input_method.buff[0] - 0x30); //提交问题答案

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 9)));
            mw_dis.bg_2ln_pixel = (8 * 0); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 4), rpt_average, true); //一般类型  报告成功和失败
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        } else //输入内容错误
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

            menu_auto_exit_set(1500, true); //退回上一级菜单
            menu_cnt.menu_flag = true;
        }
    }
}

static void mo_sms_display_hdl(void) {
    u8 buff[1200];
    u8 n, i;
    u16 lgth;

    switch (sms_up_center.sms_type) {
    case sms_type_fix: //事件信息
        n = _strlen((u8 *)(*(sms_key_op_dis + 0)));
        _memcpy((sms_union_data.fix.dat + sms_union_data.fix.lgth), (*(sms_key_op_dis + 0)), n);

        dis_multi_page((sms_union_data.fix.dat), (sms_union_data.fix.lgth + n), true, sms_fix_input_set);
        break;

    case sms_type_ask:                                                        //提问信息
        _memcpy((buff), (sms_union_data.ask.dat), (sms_union_data.ask.lgth)); //问题内容
        lgth = (sms_union_data.ask.lgth);

        *(buff + (lgth++)) = '('; //答案内容
        for (i = 0x00; i < (sms_union_data.ask.ans_cnt); i++) {
            *(sms_up_center.ask_ans + i) = ((sms_union_data.ask.answer + i)->ans_id); //装载答案ID号

            *(buff + (lgth++)) = (i + 0x30);
            *(buff + (lgth++)) = '.';

            _memcpy((buff + lgth), ((sms_union_data.ask.answer + i)->ans_dat), ((sms_union_data.ask.answer + i)->ans_lgth));
            lgth += ((sms_union_data.ask.answer + i)->ans_lgth);

            *(buff + (lgth++)) = ';';
        }
        *(buff + (lgth++)) = ')';

        n = _strlen((u8 *)(*(sms_key_op_dis + 1)));
        _memcpy((buff + lgth), (*(sms_key_op_dis + 1)), n);
        lgth += n;

        dis_multi_page((buff), (lgth), true, sms_ask_input_set);
        break;

    case sms_type_vod: //点播信息
        n = _strlen((u8 *)(*(sms_key_op_dis + 2)));
        _memcpy((sms_union_data.vod.dat + sms_union_data.vod.lgth), (*(sms_key_op_dis + 2)), n);

        dis_multi_page((sms_union_data.vod.dat), (sms_union_data.vod.lgth + n), true, sms_vod_input_set);
        break;

    case sms_type_serve: //服务信息
        dis_multi_page((sms_union_data.serve.dat), (sms_union_data.serve.lgth), false, NULL);
        break;

    case sms_type_center: //文本信息
        dis_multi_page((sms_union_data.center.dat), (sms_union_data.center.lgth), false, NULL);
        break;

    case sms_type_phone: //手机信息

        dis_multi_page(sms_union_data.phone.dat, sms_union_data.phone.lgth, false, NULL);
        //  dis_multi_page(sms_union_data.phone.dat, sms_union_data.phone.lgth, false, NULL);
        break;
    }
}

static void mo_phbk_display_hdl(void) {
    u8 temp[100];
    u8 i, n;

    phbk_data_struct phbk_data;
    phbk_index phbk_name_index;

    spi_flash_read(((u8 *)(&phbk_name_index)), phbk_start_addr_name_index, (sizeof(phbk_index)));
    spi_flash_read(((u8 *)(&phbk_data)), phbk_data_abs_addr(*(phbk_name_index + (menu_cnt.menu_2nd - 1))), (sizeof(phbk_data_struct)));

    if ((phbk_data.name.lgth) > phone_book_name_max_lgth)
        phbk_data.name.lgth = phone_book_name_max_lgth;

    if ((phbk_data.num.lgth) > phone_book_num_max_lgth)
        phbk_data.num.lgth = phone_book_num_max_lgth;

    i = 0x00;
    _memcpy((temp + i), (phbk_data.name.buff), (phbk_data.name.lgth));
    i += (phbk_data.name.lgth);

    *(temp + (i++)) = ':';
    _memcpy((temp + i), (phbk_data.num.buff), (phbk_data.num.lgth));
    i += (phbk_data.num.lgth);

    *(temp + (i++)) = ' ';
    *(temp + (i++)) = '(';

    n = _strlen((u8 *)(*(phone_type_lib_dis + phbk_data.type)));
    _memcpy((temp + i), (*(phone_type_lib_dis + phbk_data.type)), n);
    i += n;

    *(temp + (i++)) = ')';

    dis_multi_page((temp), (i), false, NULL);

    //--------------   按键操作   --------------//
    if (key_data.lg) //按键长按操作
    {
        if (key_data.key == key_ok) //按下了确定键，拨号
        {
            if (phbk_data.type == phbk_type_in_only)
                return;

            *((u16 *)(phone_data.buff)) = (phbk_data.num.lgth);
            _memcpy((phone_data.buff + 2), (phbk_data.num.buff), (phbk_data.num.lgth));

            lcd_area_dis_set(4, 7, 0, 131, 0x00);
            lcd_dis_one_line(0, 0, (*(dial_lib_dis + 1)), false);
            lcd_dis_one_page(16, (*((u16 *)(phone_data.buff))), (phone_data.buff + 2));
            phone_data.step = call_out;

            menu_cnt.menu_other = mo_telephone;
            menu_cnt.menu_flag = true;
            write_data_to_call_record_flash(flash_call_out_record_addr, phbk_data);
            slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //通知主机拨号
        }
    }
    //------------------------------------------//
}

static void mo_expand_mult_usart_hdl(void) {
    u8 tmp_uart_dis_length;
    //u8 tmp_uart_dis;
    u8 k;
    u8 cnt;
    cnt = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);
    tmp_uart_dis_length = extend_uart_cnt;
    //扩展多路串口uart3:默认为：6.车载导航屏
    for (k = 0; k < extend_uart_cnt; k++) {
        if (extend_uart_buff[3 * k] == 1) {
            if (k == 0) {
                tmp_uart_dis[k] = "1.usart0";
            }
        } else if (extend_uart_buff[3 * k] == 2) {
            if (k == 0) {
                tmp_uart_dis[k] = "1.usart2";
            } else if (k == 1) {
                tmp_uart_dis[k] = "2.uart3";
            }
        }
    }

    menu_move_display(((const u8 **)tmp_uart_dis), cnt, menu_cnt.menu_4th, tmp_uart_dis_length);
}

//***********************************************************************************************//
//***********************************************************************************************//

//***********************************************************************************************//
//***********************************************************************************************//
//-------------------------------      菜单处理框架     -----------------------------------------//

//菜单显示电话本
static void menu_dis_phone_book(void) {
    u16 i, n;
    u8 buff[10];
    u8 index_buff[3][18];
    u8 *index_ptr[3];

    phbk_count_struct phbk_cnt;

    lcd_dis_one_line(0, (8 * 0), ((menu_1st_dis[3]) + 2), false);
    for (i = 0x00; i < 3; i++) //目录表指针赋值
    {
        *(index_ptr + i) = *(index_buff + i);
    }

    spi_flash_read(((u8 *)(&phbk_cnt)), phbk_start_addr_count, (sizeof(phbk_count_struct)));
    (phbk_cnt.posi) = (phbk_cnt.posi) % phone_book_total_max;
    if ((phbk_cnt.total) > phone_book_total_max) {
        (phbk_cnt.total) = phone_book_total_max;
    }

    n = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_2nd);
    phbk_dis_build((index_ptr), (phbk_cnt.total), n, 3);
    menu_move_display(((const u8 **)(index_ptr)), n, menu_cnt.menu_2nd, (phbk_cnt.total));

    buff[0] = '(';
    data_mask_ascii((buff + 1), (phbk_cnt.total), 4);
    buff[5] = ')';
    buff[6] = '\x0';
    lcd_dis_one_line(0, (8 * 10), buff, false);
}

//菜单显示信息
static void menu_dis_sms(void) {
    u8 i, n;
    u8 buff[10];
    u8 cnt; //第一行的序号

    u8 index_buff[3][18];
    u8 *index_ptr[3];

    //    sms_index_struct  sms_index;

    lcd_dis_one_line(0, (8 * 0), ((menu_1st_2nd_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
    for (i = 0x00; i < 3; i++) //目录表指针赋值
    {
        *(index_ptr + i) = *(index_buff + i);
    }

    n = 0;
    cnt = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);
    i = 0;

    menu_move_display(((const u8 **)(index_ptr)), cnt, menu_cnt.menu_3rd, n);

    buff[0] = '(';
    data_mask_ascii((buff + 1), i, 2);
    buff[3] = '/';
    data_mask_ascii((buff + 4), n, 2);
    buff[6] = ')';
    buff[7] = '\x0';
    lcd_dis_one_line(0, (8 * 9), buff, false);
}

//菜单显示通话记录
static void menu_dis_phone_records(void) {
#if 1
    u8 i, n;
    u8 buff[10];
    u8 cnt;

    u8 index_buff[3][18];
    u8 *index_ptr[3];
    // sms_index_struct	sms_index;

    _memset((u8 *)&index_buff, 0x00, 54);
    lcd_dis_one_line(0, (8 * 0), ((menu_1st_3rd_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
    //n = call_record_load_index(((call_record_type_enum)(menu_cnt.menu_2nd-1)), call_record);
    n = call_record.call_record_cnt;
    cnt = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);
    if (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + cnt].name.lgth == 0x00) {
        call_record_dis_build((*index_buff), (cnt + 1), 2);
        _strcat((u8 *)&index_buff[0][3], (u8 *)call_record.call_record_buff[(call_record_num - call_record.call_record_cnt + cnt) % call_record_num].num.buff);
    } else {
        call_record_dis_build(*index_buff, (cnt + 1), 2);
        _strcat((u8 *)&index_buff[0][3], (u8 *)call_record.call_record_buff[(call_record_num - call_record.call_record_cnt + cnt) % call_record_num].name.buff);
    }

    if (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + cnt + 1].name.lgth == 0x00) {
        call_record_dis_build(*(index_buff + 1), (cnt + 2), 2);
        _strcat((u8 *)&index_buff[1][3], (u8 *)call_record.call_record_buff[(call_record_num - call_record.call_record_cnt + cnt + 1) % call_record_num].num.buff);
    } else {
        call_record_dis_build(*(index_buff + 1), (cnt + 2), 2);
        _strcat((u8 *)&index_buff[1][3], (u8 *)call_record.call_record_buff[(call_record_num - call_record.call_record_cnt + cnt + 1) % call_record_num].name.buff);
    }

    if (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + cnt + 2].name.lgth == 0x00) {
        call_record_dis_build(*(index_buff + 2), (cnt + 3), 2);
        _strcat((u8 *)&index_buff[2][3], (u8 *)call_record.call_record_buff[(call_record_num - call_record.call_record_cnt + cnt + 2) % call_record_num].num.buff);
    } else {
        call_record_dis_build(*(index_buff + 2), (cnt + 3), 2);
        _strcat((u8 *)&index_buff[2][3], (u8 *)call_record.call_record_buff[(call_record_num - call_record.call_record_cnt + cnt + 2) % call_record_num].name.buff);
    }

    for (i = 0x00; i < 3; i++) //目录表指针赋值
    {
        *(index_ptr + i) = *(index_buff + i);
    }

    //	call_record_dis_build(cnt,index_buff,call_record.call_record_cnt);
    menu_move_display(((const u8 **)(index_ptr)), cnt, menu_cnt.menu_3rd, n);

    buff[0] = '(';
    data_mask_ascii((buff + 1), n, 2);
    buff[3] = ')';
    buff[4] = '\x0';
    lcd_dis_one_line(0, (8 * 12), buff, false);
#endif
}

//menu与key都是同级的，都是必须进入该级后才能进到该级函数去处理
static void menu_1st_hdl(void) {
    u16 i;

    i = _strlen((u8 *)(menu_dis));
    lcd_dis_one_line(0, (8 * 0), menu_dis, false);
    lcd_dis_one_line(0, (8 * i), menu_str, false);

    i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_1st);
    menu_move_display((menu_1st_dis + i), i, menu_cnt.menu_1st, menu_1st_length);
}

static void menu_2nd_hdl(void) {
    u16 i;

    i = _strlen((u8 *)(menu_1st_dis[(menu_cnt.menu_1st - 1)]));
    lcd_dis_one_line(0, (8 * 0), ((menu_1st_dis[(menu_cnt.menu_1st - 1)]) + 2), false);
    lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

    i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_2nd);

    switch (menu_cnt.menu_1st) {
    case 0x01: //记录仪设置
        menu_move_display((menu_1st_1st_dis + i), i, menu_cnt.menu_2nd, menu_1st_1st_length);
        break;

    case 0x02: //信息
        menu_move_display((menu_1st_2nd_dis + i), i, menu_cnt.menu_2nd, menu_1st_2nd_length);
        break;

    case 0x03: //通话记录
        menu_move_display((menu_1st_3rd_dis + i), i, menu_cnt.menu_2nd, menu_1st_3rd_length);
        break;

    case 0x04: //电话薄
               //menu_move_display((menu_1st_4rd_dis + i), i, menu_cnt.menu_2nd, menu_1st_4rd_length);
        menu_dis_phone_book();
        break;

    case 0x05: //参数设置
        menu_move_display((menu_1st_5th_dis + i), i, menu_cnt.menu_2nd, menu_1st_5th_length);
        break;

    case 0x06: //查询
        menu_move_display((menu_1st_6th_dis + i), i, menu_cnt.menu_2nd, menu_1st_6th_length);
        break;

    case 0x07: //高级选项
               //COMT:显示
        menu_move_display((menu_1st_7th_dis + i), i, menu_cnt.menu_2nd, menu_1st_7th_length);
        break;

    case 0x08: //对比度设置
        menu_move_display((menu_1st_8th_dis + i), i, menu_cnt.menu_2nd, menu_1st_8th_length);
        break;
    case 0x09: //拨打电话
        menu_move_display((menu_1st_9th_dis + i), i, menu_cnt.menu_2nd, menu_1st_9th_length);
        break;
    default:
        menu_cnt.menu_1st = 0x01; //强制修改到第一项菜单
    }
}

static void menu_3rd_hdl(void) {
    u16 i;
    //    u16 da;
    //    u8 temp[8];
    switch (menu_cnt.menu_1st) {
    case 0x01: //记录仪设置
        i = _strlen((u8 *)(menu_1st_1st_dis[(menu_cnt.menu_2nd - 1)]));
        lcd_dis_one_line(0, (8 * 0), ((menu_1st_1st_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
        lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

        i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);

        switch (menu_cnt.menu_2nd) {
        case 0x01: //传感器系数
            menu_move_display((menu_1st_1st_1nd_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_1nd_length);
            break;
        case 0x02: //220828速度类型://速度模式，显示：传感器速度、CAN速度、GPS速度、修改密码
            menu_move_display((menu_1st_1st_2nd_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_2nd_length);
            break;

        case 0x03: //打印标准类型
            menu_move_display((menu_1st_1st_3rd_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_3rd_length);
            break;

        case 0x04: //车辆号牌分类
            menu_move_display((menu_1st_1st_4th_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_4th_length);
            break;

        case 0x05: //驾驶员设置
            menu_move_display((menu_1st_1st_5th_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_5th_length);
            break;

        default:;
        }
        break;

    case 0x02: //信息
        menu_dis_sms();
        break;

    case 0x03: //通话记录
        switch (menu_cnt.menu_2nd) {
        case 0x01:
            spi_flash_read(((u8 *)(&call_record)), flash_call_out_record_addr, (sizeof(call_record_struct)));
            if (call_record.call_record_cnt > call_record_num)
                call_record.call_record_cnt = call_record_num;
            menu_dis_phone_records();

            break;
        case 0x02:
            spi_flash_read(((u8 *)(&call_record)), flash_received_call_record_addr, (sizeof(call_record_struct)));
            if (call_record.call_record_cnt > call_record_num)
                call_record.call_record_cnt = call_record_num;
            menu_dis_phone_records();
            break;
        case 0x03:
            spi_flash_read(((u8 *)(&call_record)), flash_missed_call_record_addr, (sizeof(call_record_struct)));
            if (call_record.call_record_cnt > call_record_num)
                call_record.call_record_cnt = call_record_num;
            menu_dis_phone_records();
            break;
        }
        break;

    case 0x05: //参数设置
        i = _strlen((u8 *)(menu_1st_5th_dis[(menu_cnt.menu_2nd - 1)]));
        lcd_dis_one_line(0, (8 * 0), ((menu_1st_5th_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
        lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

        i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);

        switch (menu_cnt.menu_2nd) {
        case 0x01: //通信参数设置
            menu_move_display((menu_1st_5th_1st_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_1st_length);
            break;

        case 0x02: //铃声选择
            menu_move_display((menu_1st_5th_2nd_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_2nd_length);
            break;

        case 0x03: //背光设置
            menu_move_display((menu_1st_5th_3rd_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_3rd_length);
            break;

        case 0x04: //油路控制
            menu_move_display((menu_1st_5th_4th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_4th_length);
            break;

        case 0x05: //固件选项
            menu_move_display((menu_1st_5th_5th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_5th_length);
            break;

        case 0x06: //看车设置
            menu_move_display((menu_1st_5th_6th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_6th_length);
            break;

        case 0x07: //定位模式设置
            menu_move_display((menu_1st_5th_7th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_7th_length);
            break;

        case 0x09: //注册参数设置
            menu_move_display((menu_1st_5th_9th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_9th_length);
            break;
        case 0x0A: //IC开卡功能2018-1-8
            menu_move_display((const u8 **)(menu_driver_name_dis + i), i, menu_cnt.menu_3rd, menu_driver_info_cnt);
            break;

        default:;
        }
        break;
    case 0x06: //参数查询

        break;
    case 0x07: //高级选项//RST-RK-REF:
        i = _strlen((u8 *)(menu_1st_7th_dis[(menu_cnt.menu_2nd - 1)]));
        lcd_dis_one_line(0, (8 * 0), ((menu_1st_7th_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
        lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

        i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);

        switch (menu_cnt.menu_2nd) {
        case 0x01: //终端控制
            menu_move_display((menu_1st_7th_1st_dis_item + i), i, menu_cnt.menu_3rd, menu_count(enum_terminal_ctrl_cnt));
            break;

        case 0x03: //WIFI热点
            menu_move_display((menu_1st_7th_6th_1th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_6th_1th_length);
            break;
            /*
				case 0x03:   //WIFI热点
					   //空重载设置 
                    menu_move_display((menu_1st_7th_3th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_3th_length);
				    da = (gps_data.oil_data[0] >> 4) * 1000 + (gps_data.oil_data[0] & 0x0f) * 100 ;
				    da += (gps_data.oil_data[1] >> 4) * 10 + (gps_data.oil_data[1] & 0x0f);

				    data_mask_ascii(temp, da, 4);
					_memcpy( (u8*)&temp[5], "mV", 2);
				    lcd_dis_one_line(4, (3 * 8), data_check_valid(temp), false);		
				*/

        case 0x06: //音量调节
            menu_move_display((menu_1st_7th_6th_2th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_6th_2th_length);
            break;
        case 0x07: //车辆载重//RST-RK-REF:
            menu_move_display((menu_1st_7th_6th_3th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_6th_3th_length);
            break;
        case 0x08: //待机界面
            menu_move_display((menu_1st_7th_6th_4th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_6th_4th_length);
            break;

        default:;
        }
        break;
    default:;
    }
}

static void menu_4th_hdl(void) {
    u16 i;
    u8 tmp_uart_dis_length;
    u8 k;
    u8 cnt;

    switch (menu_cnt.menu_1st) {
    case 0x05: //参数设置
        switch (menu_cnt.menu_2nd) {
        case 0x05: //固件选型
            switch (menu_cnt.menu_3rd) {
            case 0x03: //扩展多路串口usart0:
                i = _strlen((u8 *)(menu_1st_5th_5th_dis[2]));
                lcd_dis_one_line(0, (8 * 0), ((menu_1st_5th_5th_dis[2]) + 2), false);
                lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

                cnt = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_4th);
                tmp_uart_dis_length = extend_uart_cnt;
                for (k = 0; k < extend_uart_cnt; k++) {
                    if (extend_uart_buff[3 * k] == 0x01) {
                        if (k == 0) {
                            tmp_uart_dis[k] = "1.usart0";
                        }
                    } else if (extend_uart_buff[3 * k] == 0x02) {
                        if (k == 0) {
                            tmp_uart_dis[k] = "1.usart2";
                        } else if (k == 1) {
                            tmp_uart_dis[k] = "2.uart3";
                        }
                    }
                }
                menu_move_display(((const u8 **)tmp_uart_dis), cnt, menu_cnt.menu_4th, tmp_uart_dis_length);
                tmp_cnt = 0x00;
                tmp_bit_cnt = 0x00;
                break;
            }
            break;

        case 0x09: //注册参数设置
            switch (menu_cnt.menu_3rd) {
            case 0x06: //车牌颜色
                i = _strlen((u8 *)(menu_1st_5th_9th_dis[5]));
                lcd_dis_one_line(0, (8 * 0), ((menu_1st_5th_9th_dis[5]) + 2), false);
                lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

                i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_4th);
                menu_move_display((menu_1st_5th_9th_6th_dis + i), i, menu_cnt.menu_4th, menu_1st_5th_9th_6th_length);
                break;
            }
            break;
        default:
            break;
        }
        break;
    case 07: //高级选项
        switch (menu_cnt.menu_2nd) {
        case 0x01: //终端控制
            switch (menu_cnt.menu_3rd) {
            case 0x07: //系统看护
                       //RST-RK:系统看护菜单显示
#if (0)                //系统看护
                i = _strlen((u8 *)(menu_1st_5th_9th_dis[5]));
                lcd_dis_one_line(0, (8 * 0), ((menu_1st_5th_9th_dis[5]) + 2), false);
                lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

                i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_4th);
                menu_move_display((menu_1st_7th_1st_dis_val + i), i, menu_cnt.menu_4th, menu_1st_7th_1st_val_length);
#elif (1)
                if (1) { //显示:系统看护->
                    i = _strlen((u8 *)(menu_1st_7th_1st_dis_item[menu_line(enum_terminal_ctrl_foster_care)]));
                    lcd_dis_one_line(0, (8 * 0), ((menu_1st_7th_1st_dis_item[menu_line(enum_terminal_ctrl_foster_care)]) + 2), false); //系统看护
                    lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);                                                               //->
                }

                //logd("move_first_line %d, %d", menu_cnt.last_start, menu_cnt.menu_4th);
                i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_4th);
                menu_move_display((menu_1st_7th_1st_dis_val + i), i, menu_cnt.menu_4th, menu_1st_7th_1st_val_length);
#elif (1) //参考：扩展多路串口，代码
                i = _strlen((u8 *)(menu_1st_5th_5th_dis[2]));
                lcd_dis_one_line(0, (8 * 0), ((menu_1st_5th_5th_dis[2]) + 2), false);
                lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

                cnt = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_4th);
                tmp_uart_dis_length = extend_uart_cnt;
                for (k = 0; k < extend_uart_cnt; k++) {
                    if (extend_uart_buff[3 * k] == 0x01) {
                        if (k == 0) {
                            tmp_uart_dis[k] = "1.usart1";
                        }
                    } else if (extend_uart_buff[3 * k] == 0x02) {
                        if (k == 0)
                            tmp_uart_dis[k] = "1.usart2";
                        else if (k == 1)
                            tmp_uart_dis[k] = "2.usart2";
                    }
                }
                menu_move_display(((const u8 **)tmp_uart_dis), cnt, menu_cnt.menu_4th, tmp_uart_dis_length);
                tmp_cnt = 0x00;
                tmp_bit_cnt = 0x00;
#endif
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        break;
    }
}

static void menu_5th_hdl(void) {
    u8 i, j;
    static u8 *tmp_uart_content_dis[menu_1st_5th_5th_4th_length];
    u8 cnt;

    i = _strlen((u8 *)(tmp_uart_dis[menu_cnt.menu_4th - 1]));
    lcd_dis_one_line(0, (8 * 0), ((tmp_uart_dis[menu_cnt.menu_4th - 1]) + 2), false);
    lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);
    cnt = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_5th);

    switch (menu_cnt.menu_1st) //第一级菜单
    {
    case 0x05:                     //参数设置
        switch (menu_cnt.menu_2nd) //第二级菜单
        {
        case 0x05:                     //第二级菜单第五项: 固件选项
            switch (menu_cnt.menu_3rd) //第三级菜单
            {
            case 0x03:                     //第三级菜单第三项://扩展多路串口usart0:
                switch (menu_cnt.menu_4th) //第四级菜单
                {
                case 0x01: //第四级菜单第一项: usart1
                    if (tmp_cnt == 0x00) {
                        for (j = 0; j < menu_1st_5th_5th_4th_length; j++) {
                            if (testbit(extend_uart_buff[1], j)) {
                                tmp_uart_content_dis[tmp_cnt++] = (u8 *)(&(menu_1st_5th_5th_4th_dis[j][0]));
                                extend_uart_send_bit[tmp_bit_cnt++] = j;
                                if (uart_set_ok_flag[0] == 0x00) {
                                    if (testbit(extend_uart_buff[2], j)) {
                                        current_uart_flag[0] = tmp_cnt;
                                    }
                                }
                            }
                        }

                        // for (j = 0; j < menu_1st_5th_5th_4th_length; j++) {
                        //     logd("<%s>", tmp_uart_content_dis[j]);
                        // }
                    }
                    mult_usart_set.uart_num = extend_uart_buff[0];
                    mult_usart_set.mult_choose = extend_uart_buff[1];
                    break;

                case 0x02: //第四级菜单第二项:usart2
                    if (tmp_cnt == 0x00) {
                        for (j = 0; j < menu_1st_5th_5th_4th_length; j++) {
                            if (testbit(extend_uart_buff[4], j)) {
                                tmp_uart_content_dis[tmp_cnt++] = (u8 *)(&(menu_1st_5th_5th_4th_dis[j][0]));
                                extend_uart_send_bit[tmp_bit_cnt++] = j;
                                if (uart_set_ok_flag[1] == 0x00) {
                                    if (testbit(extend_uart_buff[5], j)) {
                                        current_uart_flag[1] = tmp_cnt;
                                        //logd("current_uart_flag[1] = tmp_cnt, is %d", current_uart_flag[1]);
                                    }
                                }
                            }
                        }
                    }

                    mult_usart_set.uart_num = extend_uart_buff[3];
                    mult_usart_set.mult_choose = extend_uart_buff[4];
                    break;
                }

                menu_move_display(((const u8 **)(tmp_uart_content_dis + cnt)), cnt, menu_cnt.menu_5th, tmp_cnt);
                if (1) { //选中图标'*'
                    if (current_uart_flag[menu_cnt.menu_4th - 1] >= cnt) {
                        if (menu_cnt.menu_5th == current_uart_flag[menu_cnt.menu_4th - 1]) {
                            lcd_dis_one_word(((current_uart_flag[menu_cnt.menu_4th - 1] - 1 - cnt) * 2 + 2), 116, state_flag_dis[16], lcd_dis_type_8x16ascii, true);
                            lcd_area_dis_set(((current_uart_flag[menu_cnt.menu_4th - 1] - 1 - cnt) * 2 + 2), ((current_uart_flag[menu_cnt.menu_4th - 1] - 1 - cnt) * 2 + 3), 116, 131, 0xff);
                        } else {
                            lcd_dis_one_word(((current_uart_flag[menu_cnt.menu_4th - 1] - 1 - cnt) * 2 + 2), 116, state_flag_dis[16], lcd_dis_type_8x16ascii, false);
                        }
                    }
                }
                break;
            }
            break;
        }
        break;
    }
}

//自动退出界面的处理
static void menu_auto_exit_hdl(void) {
    if (menu_auto_exit.auto_flag) {
        if (_pastn(menu_auto_exit.cnt) >= menu_auto_exit.lgth) {
            menu_auto_exit.auto_flag = false; //自动退出操作处理完成

            if (menu_auto_exit.dir_flag) //退出到上一级菜单
            {
                menu_cnt.menu_other = mo_reset;
            } else //退出到待机界面
            {
                dis_goto_standby();
            }
            _memset((u8 *)(&menu_auto_exit), 0x00, (sizeof(menu_auto_exit)));
            menu_cnt.menu_flag = true; //启动菜单更新
        }
    }
}

//返回报告处理
static void menu_report_hdl(void) {
    u8 flag;
    u16 n;

    if (menu_report.en_flag) {
        flag = 0x00;

        if (menu_report.ok_flag) //报告成功界面
        {
            if (menu_report.rpt) {
                flag = 2;
            }
        } else if (_pastn(menu_report.cnt) >= menu_report.lgth) //超时报告失败界面
        {
            flag = 1;
        }

        if (flag) {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));

            mw_dis.p_dis_2ln = (u8 *)(menu_report.ptr[flag - 1]); //显示的内容
            n = _strlen((u8 *)(menu_report.ptr[flag - 1]));
            mw_dis.bg_2ln_pixel = (((16 - n) / 2) * 8); //信息提示窗口设置

            menu_auto_exit_set(1000, true); //退回上一级菜单

            menu_cnt.menu_flag = true;   //启动菜单更新
            menu_report.en_flag = false; //报告界面处理完成
        }
    }
}

//***********************************************************************************************//
//***********************************************************************************************//

//***********************************************************************************************//
//***********************************************************************************************//
//--------------------------------         按键处理框架        ----------------------------------//

//电话本按键操作
static void key_2nd_phone_book_hdl(void) {
    phbk_count_struct phbk_cnt;

    spi_flash_read(((u8 *)(&phbk_cnt)), phbk_start_addr_count, (sizeof(phbk_count_struct)));
    (phbk_cnt.posi) = (phbk_cnt.posi) % phone_book_total_max;
    if ((phbk_cnt.total) > phone_book_total_max) {
        (phbk_cnt.total) = phone_book_total_max;
    }

    if (function_key_hdl(2, (phbk_cnt.total)) == 0x00) {
        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_phbk_display;
        menu_cnt.menu_flag = true;
    }
}

#if (0)
static void key_2nd_phone_dial_hdl(void) {
    static u8 i = 0;
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //长度不为0
                                 //return;

            if (input_method.ptr_dest == phone_number) {
                _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //保存内容

                call_record.call_record_cnt++;
                call_record.call_record_buff[i].name.lgth = 6;
                //		call_record.call_record_buff[i].name.buff = "未知";
                _memcpy_len(call_record.call_record_buff[i].name.buff, "未知", 4);
                call_record.call_record_buff[i].num.lgth = input_method.cnt;
                _memcpy_len(call_record.call_record_buff[i].num.buff, input_method.buff, input_method.cnt);
                call_record.call_record_buff[i].type = 1;

                flash25_program_auto_save(flash_call_out_record_addr, (u8 *)&call_record, 1);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 5), rpt_average, true); //设置类型  报告成功和失败
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
                i++;
            }
    }
    /*
	input_method.ptr_dest
	input_method.ptr_dis
	input_method.buff
	input_method.cnt
	*/
}
#endif

//信息按键操作
static void key_3rd_sms_hdl(void) {
    sms_index_struct sms_index;

    switch (menu_cnt.menu_2nd) {
    case 0x01:
        spi_flash_read(((u8 *)(&sms_index)), flash_sms_fix_addr, (sizeof(sms_index)));
        if (sms_index.total > sms_total_max)
            sms_index.total = sms_total_max;
        if (function_key_hdl(3, (sms_index.total)) == 0x00) {
            sms_up_center.sms_type = sms_type_fix;
            sms_up_center.fix_id = ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->type_id_serial_num);
            spi_flash_read(((u8 *)(&(sms_union_data.fix))), ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->addr), (sizeof(sms_fix_content_struct)));

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //即将读取信息，将标志设置为已读状态
            {
                ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) = false;
                flash25_program_auto_save(flash_sms_fix_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
            }

            dis_multi_page_cnt = 0x00;
            menu_cnt.menu_other = mo_sms_display;
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x02:
        spi_flash_read(((u8 *)(&sms_index)), flash_sms_ask_addr, (sizeof(sms_index)));
        if (sms_index.total > sms_total_max)
            sms_index.total = sms_total_max;
        if (function_key_hdl(3, (sms_index.total)) == 0x00) {
            sms_up_center.sms_type = sms_type_ask;
            sms_up_center.ask_num = ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->type_id_serial_num);
            spi_flash_read(((u8 *)(&(sms_union_data.ask))), ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->addr), (sizeof(sms_ask_content_struct)));

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //即将读取信息，将标志设置为已读状态
            {
                ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) = false;
                flash25_program_auto_save(flash_sms_fix_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
            }

            dis_multi_page_cnt = 0x00;
            menu_cnt.menu_other = mo_sms_display;
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x03:
        spi_flash_read(((u8 *)(&sms_index)), flash_sms_vod_addr, (sizeof(sms_index)));
        if (sms_index.total > sms_total_max)
            sms_index.total = sms_total_max;
        if (function_key_hdl(3, (sms_index.total)) == 0x00) {
            sms_up_center.sms_type = sms_type_vod;
            sms_up_center.vod_type = ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->type_id_serial_num);
            spi_flash_read(((u8 *)(&(sms_union_data.vod))), ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->addr), (sizeof(sms_vod_content_struct)));

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //即将读取信息，将标志设置为已读状态
            {
                ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) = false;
                flash25_program_auto_save(flash_sms_vod_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
            }

            dis_multi_page_cnt = 0x00;
            menu_cnt.menu_other = mo_sms_display;
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x04:
        spi_flash_read(((u8 *)(&sms_index)), flash_sms_serve_addr, (sizeof(sms_index)));
        if (sms_index.total > sms_total_max)
            sms_index.total = sms_total_max;
        if (function_key_hdl(3, (sms_index.total)) == 0x00) {
            sms_up_center.sms_type = sms_type_serve;
            spi_flash_read(((u8 *)(&(sms_union_data.serve))), ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->addr), (sizeof(sms_serve_content_struct)));

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //即将读取信息，将标志设置为已读状态
            {
                ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) = false;
                flash25_program_auto_save(flash_sms_serve_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
            }

            dis_multi_page_cnt = 0x00;
            menu_cnt.menu_other = mo_sms_display;
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x05:
        spi_flash_read(((u8 *)(&sms_index)), flash_sms_center_addr, (sizeof(sms_index)));
        if (sms_index.total > sms_total_max)
            sms_index.total = sms_total_max;
        if (function_key_hdl(3, (sms_index.total)) == 0x00) {
            sms_up_center.sms_type = sms_type_center;
            spi_flash_read(((u8 *)(&(sms_union_data.center))), ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->addr), (sizeof(sms_center_content_struct)));

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //即将读取信息，将标志设置为已读状态
            {
                ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) = false;
                flash25_program_auto_save(flash_sms_center_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
            }

            dis_multi_page_cnt = 0x00;
            menu_cnt.menu_other = mo_sms_display;
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x06: //手机短信

        spi_flash_read(((u8 *)(&sms_index)), flash_sms_phone_addr, (sizeof(sms_index)));
        if (sms_index.total > sms_total_max)
            sms_index.total = sms_total_max;
        if (function_key_hdl(3, (sms_index.total)) == 0x00) {
            sms_up_center.sms_type = sms_type_phone;
            spi_flash_read(((u8 *)(&(sms_union_data.phone))), ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->addr), (sizeof(sms_phone_content_struct)));

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //即将读取信息，将标志设置为已读状态
            {
                ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) = false;
                flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
            }

            dis_multi_page_cnt = 0x00;
            menu_cnt.menu_other = mo_sms_display;
            menu_cnt.menu_flag = true;
        }

        break;
    }
}

//通话记录按键操作
static void key_3rd_phone_records_hdl(void) {
    switch (menu_cnt.menu_2nd) {
    case 0x01: //已拨电话

        if (function_key_hdl(3, (call_record.call_record_cnt)) == 0x00) {
            *((u16 *)(phone_data.buff)) = call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1].num.lgth;
            _memcpy((phone_data.buff + 2), (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1].num.buff), (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1].num.lgth));

            lcd_area_dis_set(4, 7, 0, 131, 0x00);
            lcd_dis_one_line(0, 0, (*(dial_lib_dis + 1)), false);
            lcd_dis_one_page(16, (*((u16 *)(phone_data.buff))), (phone_data.buff + 2));
            phone_data.step = call_out;

            menu_cnt.menu_other = mo_telephone;
            menu_cnt.menu_flag = true;
            write_data_to_call_record_flash(flash_call_out_record_addr, call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1]);
            slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //通知主机拨号
        }
        break;
    case 0x02: //已接电话
    case 0x03: //未接电话
        if (function_key_hdl(3, (call_record.call_record_cnt)) == 0x00) {
            if (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1].type == phbk_type_in_only) {
                return;
            } else {
                *((u16 *)(phone_data.buff)) = call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1].num.lgth;
                _memcpy((phone_data.buff + 2), (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1].num.buff), (call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1].num.lgth));

                lcd_area_dis_set(4, 7, 0, 131, 0x00);
                lcd_dis_one_line(0, 0, (*(dial_lib_dis + 1)), false);
                lcd_dis_one_page(16, (*((u16 *)(phone_data.buff))), (phone_data.buff + 2));
                phone_data.step = call_out;

                menu_cnt.menu_other = mo_telephone;
                menu_cnt.menu_flag = true;
                write_data_to_call_record_flash(flash_call_out_record_addr, call_record.call_record_buff[call_record_num - call_record.call_record_cnt + menu_cnt.menu_3rd - 1]);
                slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //通知主机拨号
            }
        }

        break;

    default:
        break;
    }
}

//key函数主要是对该级的所有的按键操作进行处理以及非规则菜单的显示，而menu则是对该级规则的菜单进行显示
//第一级菜单
static void key_1st_hdl(void) {
    menu_cnt.next_regular = true;
    function_key_hdl(1, menu_1st_length);
}

//第二级菜单
static void key_2nd_hdl(void) { //二级菜单处理函数,主要是处理按键
    //u8 i;
    //    u8 ret;
    switch (menu_cnt.menu_1st) {
    case 0x01: //记录仪设置
        if ((menu_cnt.menu_2nd == 3) || (menu_cnt.menu_2nd == 4)
            || (menu_cnt.menu_2nd == 5)) //规则菜单
        {
            menu_cnt.next_regular = true;
        } else {
            menu_cnt.next_regular = false;
        }

        if (function_key_hdl(2, menu_1st_1st_length) == 0x00) {
            if (menu_cnt.menu_2nd == 1) { //传感器系数
                menu_cnt.menu_other = mo_1st_1st_1st_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.sensor + 2), 0, password_sensor_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 2) { //速度模式
                menu_cnt.menu_other = mo_1st_1st_2nd_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.speed + 2), 0, password_speed_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 6) { //下载历史数据
                ;                                //功能屏蔽，暂不实现  CKP  20140219
            }
        }
        break;

    case 0x02: //信息
        menu_cnt.next_regular = true;
        function_key_hdl(2, menu_1st_2nd_length);
        break;

    case 0x03: //通话记录
        menu_cnt.next_regular = true;
        if (menu_cnt.menu_2nd == 0x04) //如果是删除所有记录选项则下一个界面为不规则界面
        {
            menu_cnt.next_regular = false;
        }
        if (function_key_hdl(2, menu_1st_3rd_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x04:
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[4][1]));
                mw_dis.bg_2ln_pixel = (8 * 2); //信息提示窗口设置

                call_record_data_flash_init();  //通话记录初始化
                menu_auto_exit_set(2000, true); //退回上一级菜单
                menu_cnt.menu_flag = true;
                break;
            }
        }
        _memset((void *)&call_record, 0x00, sizeof(call_record_struct));
        break;

    case 0x04: //电话簿
        menu_cnt.next_regular = false;
        key_2nd_phone_book_hdl();
        break;

    case 0x05: //参数设置
        if ((menu_cnt.menu_2nd == 2) || (menu_cnt.menu_2nd == 3) || (menu_cnt.menu_2nd == 5)
            || (menu_cnt.menu_2nd == 7) || menu_cnt.menu_2nd == 10 /*IC开卡*/) //规则菜单
        {
            menu_cnt.next_regular = true;
        } else {
            menu_cnt.next_regular = false;
        }

        if (function_key_hdl(2, menu_1st_5th_length) == 0x00) //当前界面的所有的按键操作都会由此函数处理
        {
            if (menu_cnt.menu_2nd == 1) //通信参数设置
            {
                menu_cnt.menu_other = mo_1st_5th_1st_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 4) //油路控制
            {
                menu_cnt.menu_other = mo_1st_5th_4th_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.oil + 2), 0, password_oil_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 6) //看车设置
            {
                menu_cnt.menu_other = mo_1st_5th_6th_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.car + 2), 0, password_car_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 8) //驱动参数初始化
            {
                if (sms_phone_save) //不让进入驱动参数初始化
                    return;
                menu_cnt.menu_other = mo_1st_5th_8th_password;
                menu_cnt.menu_flag = true;

                //input_method_init(menu_password_dis, (password_manage.lcd_para_rst+2), 0, password_lcd_para_rst_max_lgth, true);
                input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 9) //注册参数设置
            {
                menu_cnt.menu_other = mo_1st_5th_1st_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
            }
        }
        break;

    case 0x06: //查询

        menu_cnt.next_regular = false;

        if (function_key_hdl(2, menu_1st_6th_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x01: //通信参数查询
                       /*
                        menu_cnt.menu_other = mo_1st_6th_1st_password;
                        menu_cnt.menu_flag = true;
                        
                        input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
			*/

                slave_send_msg((0x110000), 0x00, 0x00, false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 4)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 1), rpt_com, false); //命令类型  仅报告失败
                menu_cnt.menu_flag = true;

                break;

            case 0x02:                                //注册参数查询
                lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
                menu_cnt.menu_other = mo_query_reg;

                query.regist.step = 0x00; //启动查询参数
                menu_cnt.menu_flag = true;
                break;

            case 0x03:                                //记录仪参数查询
                lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
                menu_cnt.menu_other = mo_query_rec;

                query.rec.step = 0x00; //启动查询参数
                menu_cnt.menu_flag = true;
                break;

            case 0x04:                                //驾驶员信息
                lcd_area_dis_set(0, 7, 0, 131, 0x00); //清屏
                menu_cnt.menu_other = mo_query_driver;

                query.driver.step = 0x00; //启动查询驾驶员参数
                menu_cnt.menu_flag = true;
                break;

            case 0x05: //IC卡信息
                if (state_data.state.flag.ic_flag) {
                    menu_cnt.menu_other = mo_ic_card_dis;
                    dis_multi_page_cnt = 0x00;
                } else //未插入IC卡
                {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 31)));
                    mw_dis.bg_2ln_pixel = (8 * 0); //信息提示窗口设置

                    menu_auto_exit_set(1500, true); //退回上一级菜单
                }

                menu_cnt.menu_flag = true;
                break;

            case 0x06: //平均车速记录
                menu_cnt.menu_other = mo_speed_record;
                menu_cnt.menu_flag = true;
                break;

            case 0x07: //疲劳驾驶记录
            {
                menu_cnt.menu_other = mo_timeout_driving;
                menu_cnt.menu_flag = true;
            } break;

                /*
                    case 0x08:  //软件版本查询
                    	menu_cnt.menu_other = mo_slave_ver_check;
                    	menu_cnt.menu_flag = true;
                    	break;
                    */

            case 0x08: //查询CSQ信号值
                menu_cnt.menu_other = mo_csq_vol;
                menu_cnt.menu_flag = true;
                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                break;
            }
        }
        break;

    case 0x07:                                                                                                                                        //高级选项
        if ((menu_cnt.menu_2nd == 1) || (menu_cnt.menu_2nd == 3) || (menu_cnt.menu_2nd == 6) || (menu_cnt.menu_2nd == 7) || (menu_cnt.menu_2nd == 8)) //规则菜单
        {
            menu_cnt.next_regular = true;
        } else {
            menu_cnt.next_regular = false;
        }

        if (function_key_hdl(2, menu_1st_7th_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x02: //电子运单
                slave_send_msg((0xee0701), 0x00, 0x00, false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 8)));
                mw_dis.bg_2ln_pixel = (8 * 0); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 4), rpt_average, false); //一般类型  仅报告失败
                menu_cnt.menu_flag = true;
                break;

            case 0x04: //终端自检
                slave_send_msg((0x0c0000), 0x00, 0x00, false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 5)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 4), rpt_com, false); //命令类型  仅报告失败
                menu_cnt.menu_flag = true;
                break;

            case 0x05: //油量测试
                menu_cnt.menu_other = mo_oil_test;
                menu_cnt.menu_flag = true;

                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                lcd_dis_one_line(1, 0, oil_data_dis, false);
                break;
            default:;
            }
        }
        break;

    case 0x08:                         //对比度调节
        menu_cnt.next_regular = false; //第三级菜单为不规则菜单，显示设置成功

        if (function_key_hdl(2, menu_1st_8th_length) == 0x00) {
            //保存设置的对比度参数
            set_para.LCD_Contrast = lcd_contrast_table[menu_cnt.menu_2nd - 1];
            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

            //显示设置成功
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
            mw_dis.bg_2ln_pixel = (8 * 3); //信息提示窗口设置

            menu_auto_exit_set(1000, true); //退回上一级菜单
            menu_cnt.menu_flag = true;

            //更新LCD对比度
            //lcd_init(true);  /* ??????????? */
        }

        break;
    case 0x09: //打电话
        menu_cnt.next_regular = false;
        if (function_key_hdl(2, menu_1st_1st_5th_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x01:
                phone_data.step = call_halt; //电话挂断
                menu_cnt.menu_other = mo_telephone;
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0) + 1, rpt_average, true); //设置类型  报告成功和失败
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
                break;
            case 0x02:
                phone_data.step = call_dial; //手动拨号
                menu_cnt.menu_other = mo_telephone;
                input_method_init(phoneDial_dis, (u8 *)phone_data.buff, 0, 11, false);
                break;
            }
        }
        break;
    default:
        menu_cnt.menu_1st = 0x01; //强制修改到第一项菜单
    }
}

static void key_3rd_hdl(void) { //三级菜单按键处理函数
    u8 cun = 0;
    switch (menu_cnt.menu_1st) {
    case 0x01: //记录仪设置
        switch (menu_cnt.menu_2nd) {
        case 0x01:
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_2nd_length) == 0x00) {
                cun = (u8)menu_cnt.menu_3rd;
                if (cun == 1) {
                    menu_cnt.menu_other = mo_1st_1st_1st_1st;
                    input_method_init(((*(menu_1st_1st_dis + 0)) + 2), (recorder_para.ratio + 2), (*((u16 *)(recorder_para.ratio))), rec_para_ratio_max_lgth, false);
                    menu_cnt.menu_flag = true;
                } else {
                    menu_cnt.menu_other = mo_password_change;
                    input_method_init(menu_new_password_dis, (password_manage.sensor + 2), 0, password_sensor_max_lgth, false);
                }
            }

            break;

        case 0x02: //220828速度类型://速度模式//执行动作0x00210000
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_2nd_length) == 0x00) {
                enum_speed_mode_typedef enum_speed_mode = (enum_speed_mode_typedef)menu_cnt.menu_3rd;

                if (enum_speed_mode > enum_speed_mode_null && cun < enum_speed_mode_cnt) {
                    if (enum_speed_mode == enum_speed_mode_set_password) { //220828速度类型://修改密码//请输入新密码:
                        menu_cnt.menu_other = mo_password_change;
                        input_method_init(menu_new_password_dis, (password_manage.speed + 2), 0, password_speed_max_lgth, false);
                    } else {
                        slave_send_msg((0x00210000), 0x01, (menu_cnt.menu_3rd), false, spi1_up_comm_team_max);

                        //信息提示窗口设置
                        /*
                        menu_cnt.menu_other = mo_message_window;
                        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                        mw_dis.bg_2ln_pixel = (8 * 1);    //信息提示窗口设置

                        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                        menu_cnt.menu_flag = true;
                        */

                        sys_cfg.speed_mode = enum_speed_mode;
                    }
                }
            }
            break;

        case 0x03: //打印标准类型
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_3rd_length) == 0x00) {
                if ((menu_cnt.menu_3rd == 0x01) || (menu_cnt.menu_3rd == 0x02)) {
                    slave_send_msg((0x6a0000), 0x01, (menu_cnt.menu_3rd), false, spi1_up_comm_team_max);
                } else if (menu_cnt.menu_3rd == 0x03) {
                    recorder_para.pr_driver = 0x00;
                } else {
                    recorder_para.pr_driver = 0x01;
                }

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败

                if ((menu_cnt.menu_3rd == 0x03) || (menu_cnt.menu_3rd == 0x04)) {
                    menu_report.ok_flag = true;
                    flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), ((u8 *)(&recorder_para)), (sizeof(recorder_para)));
                }

                menu_cnt.menu_flag = true;
            }
            break;

        case 0x04: //车辆号牌分类
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_4th_length) == 0x00) {
                slave_send_msg((0x660000), 0x01, (menu_cnt.menu_3rd), false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                menu_cnt.menu_flag = true;
            }
            break;

        case 0x05: //驾驶员设置
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_5th_length) == 0x00) {
                if (state_data.state.flag.ic_flag) //IC卡插入则无法手动设置
                {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(menu_other_lib_dis[33]));
                    mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                    menu_auto_exit_set(2000, true); //退回上一级菜单
                } else {
                    menu_cnt.menu_other = mo_1st_1st_5th_1st;

                    if (menu_cnt.menu_3rd == 0x01) //驾驶员代码即工号设置
                    {
                        input_method_init(((*(menu_1st_1st_5th_dis + 0)) + 2), (ic_card.work_num), (ic_card.work_num_lgth), 7, false);
                    } else //驾驶证号码设置
                    {
                        input_method_init(((*(menu_1st_1st_5th_dis + 1)) + 2), (ic_card.driv_lic), (ic_card.driv_lic_lgth), 18, false);
                    }
                }

                menu_cnt.menu_flag = true;
            }
            break;

        default:;
        }
        break;

        //----------------------------------//

    case 0x02: //信息
        menu_cnt.next_regular = false;
        key_3rd_sms_hdl();
        break;

        //----------------------------------//

    case 0x03: //通话记录
        menu_cnt.next_regular = false;
        key_3rd_phone_records_hdl();
        break;

        //----------------------------------//

    case 0x05: //参数设置
        menu_cnt.next_regular = false;
        switch (menu_cnt.menu_2nd) {
        case 0x01: //通信参数设置
            if (function_key_hdl(3, menu_1st_5th_1st_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //主IP设置
                    menu_cnt.menu_other = mo_1st_5th_1st_1st;
                    input_method_init(((*(menu_1st_5th_1st_dis + 0)) + 2), (set_para.m_ip + 2), (*((u16 *)(set_para.m_ip))), para_main_ip_max_lgth, false);
                    break;

                case 0x02: //备用IP设置
                    menu_cnt.menu_other = mo_1st_5th_1st_1st;
                    input_method_init(((*(menu_1st_5th_1st_dis + 1)) + 2), (set_para.b_ip + 2), (*((u16 *)(set_para.b_ip))), para_backup_ip_max_lgth, false);
                    break;

                case 0x03: //本机ID设置
                    menu_cnt.menu_other = mo_1st_5th_1st_3rd;
                    input_method_init(((*(menu_1st_5th_1st_dis + 2)) + 2), (set_para.id + 2), (*((u16 *)(set_para.id))), para_id_max_lgth, false);
                    break;

                case 0x04: //APN设置
                    menu_cnt.menu_other = mo_1st_5th_1st_4th;
                    input_method_init(((*(menu_1st_5th_1st_dis + 3)) + 2), (set_para.apn + 2), (*((u16 *)(set_para.apn))), para_apn_max_lgth, false);
                    break;

                case 0x05: //服务号码设置
                    menu_cnt.menu_other = mo_1st_5th_1st_3rd;
                    input_method_init(((*(menu_1st_5th_1st_dis + 4)) + 2), (set_para.num_s + 2), (*((u16 *)(set_para.num_s))), para_num_server_max_lgth, false);
                    break;

                case 0x06: //参数初始化
                    slave_send_msg((0x150000), 0x00, 0x00, false, spi1_up_comm_team_max);

                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 5)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //设置类型  报告成功和失败
                    break;

                case 0x07: //修改设置密码
                    menu_cnt.menu_other = mo_password_change;
                    input_method_init(menu_new_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, false);
                    break;
                }

                menu_cnt.menu_flag = true;
            }
            break;

        case 0x02: //铃声选择
            if ((key_data.key == key_up) || (key_data.key == key_down)) {
#if 0
                        if (menu_cnt.menu_3rd == 1)
                        {
                            buzzer_init(s_sound_1);
                        }
                        else if (menu_cnt.menu_3rd == 2)
                        {
                            buzzer_init(s_sound_2);
                        }
                        else if (menu_cnt.menu_3rd == 3)
                        {
                            buzzer_init(s_sound_3);
                        }
#endif
            }

            if (function_key_hdl(3, menu_1st_5th_2nd_length) == 0x00) {
                set_para.buzzer_type = (menu_cnt.menu_3rd - 1);

                flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
                mw_dis.bg_2ln_pixel = (8 * 3); //信息提示窗口设置

                menu_auto_exit_set(1000, true); //退回上一级菜单
                menu_cnt.menu_flag = true;
            }
            break;

        case 0x03: //背光设置
            if (function_key_hdl(3, menu_1st_5th_3rd_length) == 0x00) {
                set_para.bl_type = lcd_bl_time_table[menu_cnt.menu_3rd - 1];

                flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
                mw_dis.bg_2ln_pixel = (8 * 3); //信息提示窗口设置

                menu_auto_exit_set(1000, true); //退回上一级菜单
                menu_cnt.menu_flag = true;
            }
            break;

        case 0x04: //油路控制
            if (function_key_hdl(3, menu_1st_5th_4th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //功能屏蔽，暂不实现  CKP  20140219
                    slave_send_msg((0x380000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x02: //功能屏蔽，暂不实现  CKP  20140219
                    slave_send_msg((0x390000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x03:
                    menu_cnt.menu_other = mo_password_change;
                    input_method_init(menu_new_password_dis, (password_manage.oil + 2), 0, password_oil_max_lgth, false);
                    break;
                }
                if ((menu_cnt.menu_3rd == 1) || (menu_cnt.menu_3rd == 2)) {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                    menu_cnt.menu_flag = true;
                }
            }
            break;

        case 0x05: //固件选项
            if (function_key_hdl(3, menu_1st_5th_5th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //字库更新

#if 0
                                menu_cnt.menu_other = mo_update_font_lib;
                                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                                _memset(((u8 *)(&font_lib_update)), '\x0', (sizeof(font_lib_update)));

                                lcd_dis_one_line(1, (8 * 0), ((u8 *)(*(menu_other_lib_dis + 14))), false);
                                lcd_dis_at_once();  //立即显示界面
#endif
                    us.u1.reg = 0;
                    us.u1.b.tr9 = 1;
                    uart1_update_func(us.u1.reg, BY_SETUP);

                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                    menu_cnt.menu_flag = true;
                    menu_report.ok_flag = true;

                    //     menu_auto_exit_set(1000, true);   //退回上一级菜单

                    break;

                case 0x02: //固件更新
#if 0
                                menu_cnt.menu_other = mo_update_tf_main;
                                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                                _memset(((u8 *)(&update_tf_data)), '\x0', (sizeof(update_tf_data)));

                                lcd_dis_one_line(1, (8 * 0), ((u8 *)(*(menu_other_lib_dis + 14))), false);
                                lcd_dis_at_once();  //立即显示界面
                                menu_cnt.menu_flag = true;
#endif
                    menu_auto_exit_set(1000, true); //退回上一级菜单
                    menu_cnt.menu_flag = true;
                    break;

                case 0x03: //扩展多路串口usart0:
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 4)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                    menu_report_set(6000, *(menu_report_dis + 1), rpt_com, false); //命令类型  仅报告失败
                    menu_cnt.menu_flag = true;
                    slave_send_msg((0x250000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    _memset((u8 *)(&mult_usart_set), 0x00, sizeof(mult_usart_set));
                    break;
                }
            }
            break;

        case 0x06: //看车设置
            if (function_key_hdl(3, menu_1st_5th_6th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //功能屏蔽，暂不实现  CKP  20140219
                    slave_send_msg((0x470000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x02: //功能屏蔽，暂不实现  CKP  20140219
                    slave_send_msg((0x480000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x03: {
                    menu_cnt.menu_other = mo_password_change;
                    input_method_init(menu_new_password_dis, (password_manage.car + 2), 0, password_car_max_lgth, false);
                } break;
                }
                /*
                        menu_cnt.menu_other = mo_message_window;
                        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                        mw_dis.bg_2ln_pixel = (8 * 1);		//信息提示窗口设置

                        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                        menu_cnt.menu_flag = true;
			*/
            }
            break;

        case 0x07: //定位模块模式选择
            if (function_key_hdl(3, menu_1st_5th_7th_length) == 0x00) {
                host_no_save_para.set_gps_module_type = menu_cnt.menu_3rd;

                menu_cnt.menu_other = mo_message_window;

                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
            break;

        case 0x09: //注册参数设置
            if (menu_cnt.menu_3rd == 6) {
                menu_cnt.next_regular = true;
            } else {
                menu_cnt.next_regular = false;
            }

            if (function_key_hdl(3, menu_1st_5th_9th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //省域ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 0)) + 2), (recorder_para.province_id + 2), (*((u16 *)(recorder_para.province_id))), rec_province_id_max_lgth, false);
                    break;

                case 0x02: //市县域ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 1)) + 2), (recorder_para.city_id + 2), (*((u16 *)(recorder_para.city_id))), rec_city_id_max_lgth, false);
                    break;

                case 0x03: //制造商ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 2)) + 2), (recorder_para.manufacturer_id + 2), (*((u16 *)(recorder_para.manufacturer_id))), rec_manufacturer_id_max_lgth, false);
                    break;

                case 0x04: //终端型号
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 3)) + 2), (recorder_para.terminal_type + 2), (*((u16 *)(recorder_para.terminal_type))), rec_terminal_type_max_lgth, false);
                    break;

                case 0x05: //终端ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 4)) + 2), (recorder_para.terminal_id + 2), (*((u16 *)(recorder_para.terminal_id))), rec_terminal_id_max_lgth, false);
                    break;

                case 0x07: //车牌号码
                    menu_cnt.menu_other = mo_1st_5th_9th_7th;
                    input_method_init(((*(menu_1st_5th_9th_dis + 6)) + 2), (recorder_para.car_plate + 2), (*((u16 *)(recorder_para.car_plate))), rec_car_plate_max_lgth, false);

                    input_method.input_lib.crisis = 2;
                    input_method.input_lib.front_crisis_size = 2; //输入法会发生切换
                    break;

                case 0x08: //车辆VIN码
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 7)) + 2), (recorder_para.car_vin + 2), (*((u16 *)(recorder_para.car_vin))), rec_car_vin_max_lgth, false);
                    break;

                    /*
                                case 0x09:  //修改密码
                                	menu_cnt.menu_other = mo_password_change;
                                	input_method_init(menu_new_password_dis, (password_manage.regist+2), 0, password_regist_max_lgth, false);
                                	break;
                                */
                }

                menu_cnt.menu_flag = true; //显示页面刷新
            }
            break;
        case 0x0A: //IC开卡2018-1-8
            if (function_key_hdl(3, menu_driver_info_cnt) == 0x00) {
                if ((menu_driver_info_cnt > 0)) {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 5) + 0)));
                    mw_dis.bg_2ln_pixel = (8 * 3); //信息提示窗口设置
                }
                menu_auto_exit_set(1000, true); //退回上一级菜单
                menu_cnt.menu_flag = true;
            }
            break;
        default:
            break;
        }
        break;

    //----------------------------------//
    case 0x06:
        menu_cnt.next_regular = false;
        switch (menu_cnt.menu_2nd) {
        case 0x01: //终端控制
            slave_send_msg((0x110000), 0x00, 0x00, false, spi1_up_comm_team_max);

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 4)));
            mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

            menu_report_set(6000, *(menu_report_dis + 1), rpt_com, false); //命令类型  仅报告失败
            menu_cnt.menu_flag = true;
            break;

        case 0x09: {
            logd("段行功能");
            break;
        }
        default:
            break;
        }
        break;
    case 0x07: //高级选项
        menu_cnt.next_regular = false;
        switch (menu_cnt.menu_2nd) {
        case 0x01: //终端控制
#if (1)            //RST-RK:终端复位//没有这段，进不了菜单【7.系统看护】
            if (menu_cnt.menu_3rd == 7) {
                menu_cnt.next_regular = true;
            } else {
                menu_cnt.next_regular = false;
            }
#endif
            if (function_key_hdl(3, menu_count(enum_terminal_ctrl_cnt)) == 0) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: { //RST-RK:终端复位
#if (DEBUG_EN_WATCH_DOG)

                    log_write(event_mcu_boot_lcd_menu);

                    while (1) { ; }
#else
                    logf("ret mcu, undef");
                    break;
#endif
                }
                case 0x02: //注册
                    slave_send_msg((0xee0100), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x03: //鉴权
                    slave_send_msg((0xee0102), 0x00, 0x00, false, spi1_up_comm_team_max);
                    menu_report.ok_flag = true;
                    break;

                case 0x04: //注销
                    slave_send_msg((0xee0003), 0x00, 0x00, false, spi1_up_comm_team_max);
                    menu_report.ok_flag = true;
                    break;

                case 0x05: //锁定  //功能屏蔽，暂不实现  CKP  20140219
                    break;

                case 0x06: //解锁  //功能屏蔽，暂不实现  CKP  20140219
                    break;

                case 0x07: //RST-RK:系统看护
                    break;

                default:;
                }

#if (1) //设置中:设置中，请稍后//哪个标志参数，会成功？
                if ((menu_cnt.menu_3rd == 1) || (menu_cnt.menu_3rd == 2) || (menu_cnt.menu_3rd == 3) || (menu_cnt.menu_3rd == 4)) {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                    menu_cnt.menu_flag = true;
                }
#endif
            }
            break;

        case 0x03: //热点功能
            if (function_key_hdl(3, menu_1st_7th_6th_1th_length) == 0x00) {
                cun = (u8)menu_cnt.menu_3rd - 1;
                tr9_frame_pack2rk(0x6049, &cun, 0x01);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
            break;

        case 0x06: //音量调节
        {
            if (function_key_hdl(3, menu_1st_7th_6th_2th_length) == 0x00) {
                cun = (u8)menu_cnt.menu_3rd - 1;
                tr9_frame_pack2rk(0x6042, &cun, 0x01);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
        } break;
        case 0x07: //车辆载货状态
        {
            if (function_key_hdl(3, menu_1st_7th_6th_3th_length) == 0x00) {
                car_state.bit.carriage = menu_cnt.menu_3rd;

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
        } break;
        case 0x08: //待机界面
        {
            if (function_key_hdl(3, menu_1st_7th_6th_4th_length) == 0x00) {
                cun = (u8)menu_cnt.menu_3rd;
                if (1 == cun) {
                    AutomaticStandbyScreen = true;
                } else if (2 == cun) {
                    AutomaticStandbyScreen = false;
                }

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
        }
            /*
						 if (function_key_hdl(3, menu_1st_7th_6th_length) == 0x00)
	                    {
	                  						
							 //空重载
							host_no_save_para.set_load_status = menu_cnt.menu_3rd;  //载重状态
	                        slave_send_msg((0x450000), 0x00, 0x00, false, spi1_up_comm_team_max);
	                  		
	                        menu_cnt.menu_other = mo_message_window;
	                        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
	                        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
	                        mw_dis.bg_2ln_pixel = (8 * 1);		//信息提示窗口设置

	                        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
	                        menu_cnt.menu_flag = true ;
							menu_report.ok_flag = true ;
							
					
	                    }
	                    break;*/

        default:
            break;
        }
        break;

        //---------------------------------//

    default:;
    }
}

static void key_4th_hdl(void) {
    u8 temp;

    switch (menu_cnt.menu_1st) {
    case 0x05: //参数设置
        switch (menu_cnt.menu_2nd) {
        case 0x05: //固件选项
            switch (menu_cnt.menu_3rd) {
            case 0x03: //扩展多路串口usart0:
                menu_cnt.next_regular = true;
                if (function_key_hdl(4, extend_uart_cnt) == 0x00) {
                    ;
                }
                break;
            }
            break;

        case 0x09: //注册参数设置
            switch (menu_cnt.menu_3rd) {
            case 0x06: //车牌颜色选择
                menu_cnt.next_regular = false;
                if (function_key_hdl(4, menu_1st_5th_9th_6th_length) == 0x00) {
                    if (menu_cnt.menu_4th <= 4)
                        temp = (menu_cnt.menu_4th);
                    else if (menu_cnt.menu_4th == 5)
                        temp = 9;
                    else
                        temp = 0x00;

                    slave_send_msg((0x260000), 0x01, temp, false, spi1_up_comm_team_max); //设置车牌颜色

                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1);                                    //信息提示窗口设置
                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                    menu_cnt.menu_flag = true;
                }
                break;
            }
            break;
        }
        break;
    case 0x07: //高级选项
        switch (menu_cnt.menu_2nd) {
        case 0x01: //终端控制
            switch (menu_cnt.menu_3rd) {
            case 0x07: //系统看护
#if (1)                //有它//RST-RK:系統看护--才可以【上】【下】跳转，以及执行【返回】【确认】
                menu_cnt.next_regular = false;
                if (function_key_hdl(4, menu_1st_7th_1st_val_length) == 0x00 /*存在【确认】动作*/) {
                    //RST-RK:系統看护//重启后生效
                    sys_cfg.foster_care = menu_cnt.menu_4th - 1; //设置【系统变量】【系统看护】值。

                    Rk_Task_Manage.resetCount = 0;
                    s_Hi3520_Monitor.reset_tmr = jiffies;
                    s_Hi3520_Monitor.reset = false;
                    //logd("menu_cnt.menu_4th = %d, foster_care = %d", menu_cnt.menu_4th, sys_cfg.foster_care);

#if (1) //设置中:设置中，请稍后//哪个标志参数，会成功？
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1);                                    //信息提示窗口设置
                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //一般类型  报告成功和失败
                    menu_cnt.menu_flag = true;

                    //设置成功
                    menu_report.ok_flag = true;
#endif
                }
#endif
                break;
            }
            break;
        }
        break;
    }
}

static void key_5th_hdl(void) {
    mult_usart_set.only_choose = 0x00;

    switch (menu_cnt.menu_1st) {
    case 0x05: //参数设置
        switch (menu_cnt.menu_2nd) {
        case 0x05: //固件选项
            switch (menu_cnt.menu_3rd) {
            case 0x03: //扩展多路串口usart0:
                switch (menu_cnt.menu_4th) {
                case 0x01:
                case 0x02:
                case 0x03:
                    menu_cnt.next_regular = false;
                    if (function_key_hdl(5, tmp_cnt) == 0x00) {
                        mult_usart_set.num = 0x01;
                        setbit(mult_usart_set.only_choose, extend_uart_send_bit[menu_cnt.menu_5th - 1]);
                        menu_cnt.menu_other = mo_message_window;
                        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                        mw_dis.bg_2ln_pixel = (8 * 1); //信息提示窗口设置

                        //COMT:报告什么，报到哪里去
                        //按键操作，成功，或，失败，会报告到屏幕，弹窗
                        menu_report_set(6000, *(menu_report_dis /*报告的内容*/), rpt_average, true); //一般类型  报告成功或失败
                        menu_cnt.menu_flag = true;
                        //手柄数据，
                        //
                        slave_send_msg((0x250000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    }
                    break;
                }
                break;
            }
            break;
        }
        break;
    }
}

//***********************************************************************************************//
//***********************************************************************************************//

void menu_init(void) {
    _memset((u8 *)(&key_data), 0x00, (sizeof(key_data)));
    _memset((u8 *)(&menu_cnt), 0x00, (sizeof(menu_cnt)));

    _memset((u8 *)(&mw_dis), 0x00, (sizeof(mw_dis)));

    _memset((u8 *)(&menu_auto_exit), 0x00, (sizeof(menu_auto_exit)));
    _memset((u8 *)(&menu_report), 0x00, (sizeof(menu_report)));

    _memset((u8 *)(&gps_data), 0x00, (sizeof(gps_data)));
    _memset((u8 *)(&query), 0x00, (sizeof(query)));
    _memset((u8 *)(&state_data), 0x00, (sizeof(state_data)));
    _memset((u8 *)(&input_method), 0x00, (sizeof(input_method)));
    _memset((u8 *)(&call_record), 0x00, (sizeof(call_record)));

    _memset((u8 *)(&phone_data), 0x00, (sizeof(phone_data)));

    menu_cnt.menu_flag = true; //启动菜单更新
}

//临时使用蜂鸣器响一下
//2022年05月06日海格要求添加
void beeper(u8 OnOff) {
    static u32 Qi = 0;
    if (OnOff) {
        beep_on();
        Qi = jiffies;
    } else if (Qi != 0) {
        if ((jiffies - Qi) >= 100) {
            Qi = 0;
            beep_off();
        }
    }
}

//**********************************************************************//
//-----------               按键处理函数               -----------------//
//该函数仅处理规则菜单下的按键操作
//不规则菜单(其他菜单)相关部分的按键处理则在相应的菜单处理函数中
//
//**********************************************************************//
void key_processing(void) {
    static bool menuFlag = false;
    static u32 countTime = 0;
    if (AutomaticStandbyScreen) {
        if (menuFlag) {
            if ((tick - countTime) >= 60) {
                menuFlag = false;
                countTime = tick;
                dis_goto_standby();
            }
        }
    }

    beeper(0);                //COMT:关动作
    if (!(key_data.active)) { //键值没有更新
        return;
    }

    lcd_bl_ctrl(true);

#if (DEBUG_EN_BEEP)
    // buzzer_init(s_key); //调试时屏蔽蜂鸣器
    beeper(1); //COMT:开动作
#endif

    if ((key_data.lg) && (key_data.key == key_esc)) //在任何界面下，长按ESC键都会返回到待机界面
    {
        dis_goto_standby(); //回到待机界面
    }

    key_data.active = false; //清除按键更新标志

    if (menu_cnt.menu_other) {      //不规则菜单处理
        key_data.active = true;     //恢复按键标志
        return;                     //不规则菜单的处理，保留按键状态，等待相关菜单函数处理
    } else if (menu_cnt.menu_5th) { //第五级菜单按键处理
        menuFlag = true;
        countTime = tick;
        key_5th_hdl();
    } else if (menu_cnt.menu_4th) { //第四级菜单按键处理
        menuFlag = true;
        countTime = tick;
        key_4th_hdl();
    } else if (menu_cnt.menu_3rd) { //第三级菜单按键处理
        menuFlag = true;
        countTime = tick;
        key_3rd_hdl();
    } else if (menu_cnt.menu_2nd) { //第二级菜单按键处理
        menuFlag = true;
        countTime = tick;
        key_2nd_hdl();
    } else if (menu_cnt.menu_1st) { //第一级菜单按键处理
        menuFlag = true;
        countTime = tick;
        key_1st_hdl();
    } else {
        menuFlag = false;
    }
}

/**************************************************************************************************
* 函 数 名:  analysis_IC_platform_info
* 函数功能:  判断平台下发的透传数据中是否有开卡信息  dr: 开头   *星号结尾
* 入口参数:  存储平台下行透传的原始数据
* 返 回 值:  0,无平台开卡信息   1，有平台开卡信息
* 编写日期:  2018-3-29  新增平台开卡功能
***************************************************************************************************/
u8 analysis_IC_platform_info(u8 *buf) {
    u8 *ptr1;
    u8 *ptr2;
    ptr1 = _strstr(buf, "dr:"); //查找dr:
    ptr2 = _strstr(buf, "*");   //查找 *

    //数据中同时存在dr:  和   * ,则认为有平台开卡数据
    if ((ptr1 != NULL) && (ptr2 != NULL)) {
        return 1; //有开卡信息
    }
    return 0; //无开卡信息
}

/**************************************************************************************************
* 函 数 名:  Platform_set_card_message
* 函数功能:  用平台下发的开卡信息自动完成开卡操作
* 入口参数:  无
* 返 回 值:  无
* 编写日期:  2018-3-29
***************************************************************************************************/
void Platform_set_card_message(void) {
    if ((menu_driver_info_cnt > 0)) //有开卡信息
    {
    }
    usb_upgrade.ic_platform_flag = false;
    menu_auto_exit_set(2000, false); //退回待机界面
}

/*************************************************
* 函 数 名:analyse_driver_name
* 函数功能:解析驾驶员姓名信息,用于菜单显示
* 入口参数:无
* 返 回 值:无
* 编写日期:2017-12-25
**************************************************/
void analyse_driver_name(void) {
    u8 *ptr;
    u8 *pBuf;
    u8 i;
    static u8 tmpBuf[5][20];
    for (i = 0; i < 5; i++) {
        _memset(tmpBuf[i], 0x00, sizeof(tmpBuf[i]));
    }
    for (i = 0; i < menu_driver_info_cnt; i++) {
        ptr = IC_driver_info[i];
        pBuf = tmpBuf[i];
        ptr += 3; //去掉头" dr: "
        pBuf += 2;
        tmpBuf[i][0] = (char)(i + 1 + '0');
        tmpBuf[i][1] = '.';
        while ((*ptr) != ',') {
            *pBuf++ = *ptr++;
        }
        *pBuf++ = '\0';
        menu_driver_name_dis[i] = tmpBuf[i];
    }
}

/*************************************************
* 函 数 名:read_ic_info
* 函数功能:读取IC卡配置信息
* 入口参数:无
* 返 回 值:无
* 编写日期:2017-12-25
**************************************************/
/*

void read_ic_info(void)
{
    FIL file;
    UINT btr = 512;
    UINT br;
    static u8 IC_step = 0x00;
    static u8 res = 2;
    switch (IC_step)
    {
        case 0x00:
            _memset(IC_info_tmp, 0x00, sizeof(IC_info_tmp)); //清空缓存
            if ((f_open(&file, ic_creation_path, FA_OPEN_EXISTING | FA_READ)) == FR_OK)
            {
                if ((f_read(&file, IC_info_tmp, btr, &br)) == FR_OK)
                {
                    logd("IC read success");
                }
                else
                {
                    loge("IC read err");
                }
                f_close(&file);
            }
            IC_step = 0x01;
            break;

        case 0x01:
            res = analysis_IC_driver_info();
            IC_step = 0x02;
            break;
        case 0x02: //开始提取驾驶员姓名信息
            if (res == 0) //信息条数大于0
            {
                analyse_driver_name();
            }
            usb_upgrade.ic_creation_flag = false;
            IC_step = 0x00;
            break;

        default:
            break;

    }

}

*/

/*************************************************
* 函 数 名:my_itoa()
* 函数功能:将一个10进制整数转换为一个字符串
* 入口参数:需要转换的正整数
* 返 回 值:无
* 编写日期:2018-4-3
**************************************************/
char *my_itoa(int var) {
    static char local[33] = {0};
    const char table[] = {"0123456789"};
    char *plocal = &local[32];
    int tmp;
    tmp = var;
    *plocal-- = '\0';
    do {
        *plocal-- = table[tmp % 10];
        tmp = tmp / 10;
    } while (tmp > 0);
    return plocal + 1;
}

/*************************************************
* 函 数 名:mo_id_auto_cfg_hal
* 函数功能:读取ID配置信息
* 入口参数:无
* 返 回 值:无
* 编写日期:2018-4-3
* 数据格式:id,数量,
*  例如: 13800000001,100,
*  表示从1380000001开始增量加一连续写100个id,第二个
*  id为1380000002,最后一个ID为1380000100
*
**************************************************/

//static void mo_id_auto_cfg_hal(void)
//{
//    FIL file;
//    UINT btr = 32; //一次读取最多32个字节数据
//    UINT br;
//	static u8 res;
//    static enum
//    {
//        STEP_INIT = 0,   //初始化相关指针,打开并读取文件
//        STEP_ANALYZE ,   //解析ID,及ID数量
//        STEP_DISPLAY_ID, //显示ID
//        STEP_WRITE_ID,   //将ID写入终端设备
//        STEP_UPDATE_ID,  //更新ID
//        STEP_WRITE_U_DISK, //将更新后的数据写入u盘
//        STEP_FINISH,       //完成
//        STEP_ERR,          //错误处理

//    } cfg_step = STEP_INIT;

//    switch (cfg_step)
//    {
//        case STEP_INIT: //打开文件
//            _memset(id_cfg_buf, 0x00, sizeof(id_cfg_buf)); //清空缓存
//            if ((f_open(&file, ID_auto_cfg_path, FA_OPEN_EXISTING | FA_READ)) == FR_OK)
//            {
//                if ((f_read(&file, (u8 *)id_cfg_buf, btr, &br)) == FR_OK)
//                {
//                    logd("Id read success");
//                }
//                else
//                {
//                    loge("Id read err");
//                }
//                f_close(&file);
//            }
//            cfg_step = STEP_ANALYZE;
//            break;

//        case STEP_ANALYZE:
//            res = analyse_judge_id_cfg();
//            if (0 == res)
//            {
//              cfg_step = STEP_DISPLAY_ID;
//            }
//            else
//            {
//              cfg_step = STEP_ERR;
//            }
//            break;
//
//       case STEP_DISPLAY_ID:
//        lcd_dis_one_line(0, (8 * 0), ((uchar *)(*(menu_other_lib_dis + 35))), false);
//        lcd_dis_one_line(1, (8 * 0), ((uchar *)id_buf_tmp), false); //显示当前配置的ID
//        /*以下两行显示空白,主要是为了清除自动升级时下面两行还有显示字符,而使得界面不美观问题*/
//        lcd_dis_one_line(2, (8 * 0), ((uchar *)(*(menu_other_lib_dis + 35))), false);
//        lcd_dis_one_line(3, (8 * 0), ((uchar *)(*(menu_other_lib_dis + 35))), false);
//          cfg_step = STEP_WRITE_ID;
//
//            break;
//
//       case STEP_WRITE_ID:
//            set_para.id[0] = _strlen(id_buf_tmp);                //保存终端ID长度
//            _memcpy((set_para.id + 2), id_buf_tmp, _strlen(id_buf_tmp)); //保存内容
//            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));
//            slave_send_msg((0x040000), 0x00, 0x00, false, spi1_up_comm_team_max); //设置本机ID
//            cfg_step = STEP_UPDATE_ID;
//            break;
//
//       case STEP_UPDATE_ID:
//            update_id();
//            cfg_step = STEP_WRITE_U_DISK;
//            break;
//       case STEP_WRITE_U_DISK:
//           if ((f_open(&file, ID_auto_cfg_path, FA_OPEN_EXISTING | FA_WRITE)) == FR_OK)
//            {
//                if ((f_write(&file, (u8 *)id_cfg_buf, btr, &br)) == FR_OK)
//                {
//                    logd("Id write success");
//                }
//                else
//                {
//                    loge("Id write err");
//                }
//                f_close(&file);
//            }
//            cfg_step = STEP_FINISH;
//            break;

//       case STEP_FINISH:
//            usb_upgrade.id_auto_cfg_flag = false;
//            dis_cfg_report(44, 2);
//            cfg_step = STEP_INIT;
//            break;
//
//       case STEP_ERR:
//           usb_upgrade.id_auto_cfg_flag = false;
//            dis_cfg_report(45, 2);
//            cfg_step = STEP_INIT;
//          	break;
//    }
//}

bool Ack_Export;
u8 Export_finish;

void USB_data_extern(void) {
    static bool Usb_Export = false;
    static u16 exit = 0;
    //static u8 update_show = 0;

    if (Ack_Export == true) { //确人导出数据
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(USB_ack_export));
        mw_dis.bg_2ln_pixel = 16; //信息提示窗口设置
        Usb_Export = true;
    }

    if (Usb_Export == true) {
        if (1 == Export_finish) {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(USB_Export_finish));
            mw_dis.bg_2ln_pixel = 16 * 2; //显示位置
            Usb_Export = false;
            exit = tick;
        } else if (2 == Export_finish) {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(USB_Export_failure));
            mw_dis.bg_2ln_pixel = 16 * 2; //显示位置
            Usb_Export = false;
            exit = tick;
        }
    }

    if (0 != Export_finish) {
        if (_coveri(exit) >= 120) {
            dis_goto_standby(); //回到待机界面
            Export_finish = 0;
        }
    }
}

#if (MO_ZERO_IS_NULL_FUN == 1)
static void mo_null_fun_hdl(void) {
}
#endif

static void (*menu_other_point[])(void) = {
#if (MO_ZERO_IS_NULL_FUN == 1)
    mo_null_fun_hdl, //空函数
#endif
    mo_standby_hdl,        //待机画面
    mo_gps_infor1_hdl,     //GPS全信息界面1
    mo_gps_infor2_hdl,     //GPS全信息界面2
    mo_recorder_state_hdl, //行驶记录仪状态界面
    mo_check_state_hdl,    //状态检测界面

    mo_print_hdl,        //打印界面
    mo_lcd_para_rst_hdl, //液晶相关参数初始化提示界面

    mo_message_window_hdl, //消息窗口界面

    mo_1st_5th_1st_password_hdl, //输入法界面
    mo_1st_5th_4th_password_hdl,
    mo_1st_5th_6th_password_hdl,
    mo_1st_5th_8th_password_hdl,
    mo_1st_6th_1th_password_hdl,

    mo_1st_1st_1st_password_hdl,
    mo_1st_1st_2nd_password_hdl, //速度模式密码//220828速度类型:

    mo_password_change_hdl, //密码修改界面

    mo_1st_5th_1st_1st_hdl, //主或备用IP或域名设置
    mo_1st_5th_1st_3rd_hdl, //本机ID或短信服务号码设置
    mo_1st_5th_1st_4th_hdl, //APN设置

    mo_1st_5th_9th_1st_hdl, //注册项相关参数基本设置
    mo_1st_5th_9th_7th_hdl, //注册项相关参数车牌号码设置

    mo_1st_1st_1st_1st_hdl, //传感器系数设置
    mo_1st_1st_5th_1st_hdl, //驾驶员参数设置

    mo_host_para_hdl,   //主机参数界面
    mo_ic_card_dis_hdl, //IC卡信息
    //mo_slave_ver_check_hdl,   //从机版本号查询
    mo_csq_vol_hdl, //查询CSQ值

    mo_auto_check_hdl,   //自检信息
    mo_query_rec_hdl,    //行驶记录仪参数查询
    mo_query_reg_hdl,    //注册参数查询
    mo_query_driver_hdl, //驾驶员参数查询

    mo_update_tf_main_hdl,  //TF卡升级主机
    mo_update_font_lib_hdl, //字库升级

    mo_oil_test_hdl,  //油量测试
    mo_telephone_hdl, //通话

    mo_sms_fix_input_hdl, //上传事件
    mo_sms_vod_input_hdl, //信息点播
    mo_sms_ask_input_hdl, //提问应答
    mo_sms_display_hdl,   //信息内容显示

    mo_phbk_display_hdl,      //电话本内容显示
    mo_expand_mult_usart_hdl, //扩展多路串口显示
    mo_cfg_parameter_hdl,     //自动参数配置
    mo_read_rcd_data_hal,     //读取行驶记录仪数据
    mo_auto_init_hal,         //自动初始参数界面
    mo_timeout_driving_hal,
    mo_speed_record_hal,
    //mo_id_auto_cfg_hal,     //ID自动配置界面
    //key_2nd_phone_dial_hdl,		//打电话

};

//**********************************************************************//
//-----------               菜单处理函数               -----------------//
//该函数仅处理规则菜单和不规则菜单下的显示
//
//**********************************************************************//
//COMT:菜单处理函数//显示
void menu_processing(void) {
    //    static u8 mm = 1;
    static u32 state_jif = 0;
    static u32 mainInterface = 0;

    if ((menu_cnt.menu_flag) || ((menu_cnt.menu_other) && (key_data.active))) { //菜单更新需求
        menu_cnt.menu_flag = false;                                             //清零菜单更新操作标志

        if (menu_cnt.menu_other) { //不规则菜单处理
#if (MO_ZERO_IS_NULL_FUN == 1)
            menu_other_point[(menu_cnt.menu_other)]();
#else
            menu_other_point[(menu_cnt.menu_other - 1)]();
#endif
            key_data.active = false;    //清除按键更新标志，因不规则菜单需要按键标志，固标志在此处清零
        } else if (menu_cnt.menu_5th) { //第五级菜单界面处理
            mainInterface = tick;
            menu_5th_hdl();
        } else if (menu_cnt.menu_4th) { //第四级菜单界面处理
            mainInterface = tick;
            menu_4th_hdl();
        } else if (menu_cnt.menu_3rd) { //第三级菜单界面处理
            mainInterface = tick;
            menu_3rd_hdl();
        } else if (menu_cnt.menu_2nd) { //第二级菜单界面处理
            mainInterface = tick;
            menu_2nd_hdl();
        } else if (menu_cnt.menu_1st) { //第一级菜单界面处理
            mainInterface = tick;
            menu_1st_hdl();
        } else { //返回待机界面
            mainInterface = tick;
            dis_goto_standby();
        }
    }

    if (AutomaticStandbyScreen) {
        if (_covern(mainInterface) >= 60) {
            mainInterface = tick;
            _memset((u8 *)&menu_cnt, 0x00, 0x0A);
            dis_goto_standby();
        }
    }

    menu_auto_exit_hdl();
    menu_report_hdl();

    if (_pastn(state_jif) >= 1000) {
        state_jif = jiffies;
        handle_send_0xa0_info(FROM_INNER);
    }

    USB_data_extern(); //数据导出提示
}

//**********************************************************************//
//**********************************************************************//
//-----------             低功耗处理函数               -----------------//
//
//**********************************************************************//
void low_power_hdl(void) {
    //----液晶背光处理----//
    if (gps_data.acc_state && (set_para.bl_type == 0)) //如果ACC开且背光类型为常亮2018-3-29
    {
        lcd_bl_ctrl(true);

        return;
    } else {
        if (!gps_data.acc_state) //ACC关的情况下,默认30秒关闭背光
        {
            if (sub_u32(jiffies, user_time.bl) > 30000) {
                lcd_bl_ctrl(false);
                return;
            }
        } else //如果ACC开,且背光类型不为常亮设置
        {
            if (set_para.bl_type != 0) //如果不是常亮设置
            {
                if (set_para.bl_type != 255) //常规设置
                {
                    if (sub_u32(jiffies, user_time.bl) > (set_para.bl_type * 1000)) {
                        lcd_bl_ctrl(false);
                    }
                } else //省电模式
                {
                    set_para.bl_type = 5; //省电模式暂按照背光亮5秒钟执行  CKP  2014-10-08  //////////
                }
            }
        }
    }
}
