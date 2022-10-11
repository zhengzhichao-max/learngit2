#include "include_all.h"

const u8 data_format_error[] = "IC卡数据格式错误";

//static state_struct  tf_card_state;  //TF卡状态检测
static state_struct ic_card_state; //IC卡状态检测

IC_Time_t IC_Time;

//初始化函数
void card_scan_init(void) {
    ic_type = ic_none; //卡未插入

    //	tf_card_state.last = tf_card_state_default;  //默认状态
    //	tf_card_state.filt_tim = jiffies;
    //	tf_card_state.tim = jiffies;

    ic_card_state.last = ic_card_state_default; //默认状态
    ic_card_state.filt_tim = jiffies;
    ic_card_state.tim = jiffies;
}

//IC卡数据转换
static void load_driver_data(ic_card_save_format_struct *src, ic_card_date_struct *dsc) {
    (dsc->work_num_lgth) = str_len((src->work_num), 6); //工号  公司IC卡定义最长为6字节，但实际设备支持最长7字节  CKP  20140618
    _memcpy((dsc->work_num), (src->work_num), (dsc->work_num_lgth));

    //logd("<%s>", src->name);
    (dsc->name_lgth) = str_len(src->name, 12); //姓名
    _memcpy((dsc->name), (src->name), (dsc->name_lgth));

    (dsc->id_card_lgth) = str_len((src->id_card), 20); //身份证号码
    _memcpy((dsc->id_card), (src->id_card), (dsc->id_card_lgth));

    (dsc->driv_lic_lgth) = str_len((src->driv_lic), 18); //驾驶证号码
    _memcpy((dsc->driv_lic), (src->driv_lic), (dsc->driv_lic_lgth));
    _memcpy((dsc->driv_lic_validity), (src->driv_lic_validity), 3);

    (dsc->certificate_lgth) = str_len((src->certificate), 18); //从业资格证
    _memcpy((dsc->certificate), (src->certificate), (dsc->certificate_lgth));

    (dsc->license_organ_lgth) = str_len((src->license_organ), 40); //从业资格证发证机构
    _memcpy((dsc->license_organ), (src->license_organ), (dsc->license_organ_lgth));
    _memcpy((dsc->license_organ_validity), (src->license_organ_validity), 4);
}

//IC卡扫描并初始化IC卡
//插卡//判断卡类型//读卡//数据赋值结构体//END
//Rk//插卡上报.标志位
void ic_card_scan(void) {
    static u8 last = ic_card_state_default; //上一次的状态  0表示卡槽中有卡插入  1表示卡槽中没有
    u8 temp;
    u32 dat;
    u8 i;
    //u8 xorr = 0;
    u8 ic_data[260];
    //ic_card_save_format_struct  ic_data;
		//static state_struct ic_card_state; //IC卡状态检测
    temp = state_scan((&ic_card_state), ic_insert_read(), card_scan_time_max);
    if (temp == last) //插入状态无变化则退出函数
    {
        return;
    }

    last = temp;
    if (temp == ic_card_state_default) {     //卡被拔出
        IC_Time.IC_exit = mix.time;          //记录拔卡时间
        if (state_data.state.flag.ic_flag) { //判断是否有必要提示卡拔出
            ic_type = ic_none;               //卡拔出
            _memset(((u8 *)(&ic_card)), 0x00, (sizeof(ic_card)));

            state_data.state.flag.ic_flag = false;
            menu_cnt.menu_flag = true; //菜单更新操作

            if ((gps_data.speed[0] == 0x00) && (gps_data.speed[1] == 0x00)) {         //判断退签条件
                slave_send_msg((0xee0702), 0x00, 0x00, false, spi1_up_comm_team_max); //驾驶员信息上报
            }
            lcd_bl_ctrl(true);
        }

        mix.ic_login = false;
        ic_driver_lisence = true;
    } else {                                                                                            //有卡插入
        dat = ic4442_init();                                                                            //读取复位值
        IC_Time.IC_enter = mix.time;                                                                    //记录插卡时间
        if ((dat == 0xffffffff) || (dat == 0x00)) {                                                     //IC2402卡或者其他
            iic_bus_init();                                                                             //初始化I2C总线
            if (at24cxx_read_data(at24_i2c_slave_addr, eeprom_start_addr, ic_data, eeprom_data_lgth)) { //IC卡插入，读取数据
                ic_type = ic_2402;                                                                      //24C02卡
            } else {
                ic_type = ic_error; //未知卡片
            }
        } else { //卡类型为4442卡
            ic4442_read_data(read_main_mem, 0x00, ic_data);
            ic_type = ic_4442; //4442卡
        }

        if ((ic_type == ic_2402) || (ic_type == ic_4442)) {
            lcd_bl_ctrl(true);
            menu_cnt.menu_flag = true; //菜单更新操作
#if (0)
            xorr = _get_xor(ic_data, eeprom_data_lgth);
#else
            _get_xor(ic_data, eeprom_data_lgth);
#endif
            if (((*ic_data) != 0xFF)) {
                load_driver_data(((ic_card_save_format_struct *)(&ic_data)), (&ic_card));

                state_data.state.flag.ic_flag = true;

                if ((gps_data.speed[0] == 0x00) && (gps_data.speed[1] == 0x00)) {
                    _memset((u8 *)&driver_cfg.driver_code, 0, 6);
                    for (i = 0; i < ic_card.work_num_lgth; i++) {
                        driver_cfg.driver_code[i] = ic_card.work_num[i];
                    }
                    _memset((u8 *)&driver_cfg.license, 0x00, 20);
                    _memcpy((u8 *)&driver_cfg.license, (ic_card.id_card), ic_card.id_card_lgth);
										logd("------test---ic_card.driv_lic:[%s]-----", ic_card.id_card);
                    //COMT:驾驶员信息//关键数据
                    _memset((u8 *)&mix.ic_driver, 0, 18);
                    _memcpy((u8 *)&mix.ic_driver, (u8 *)&driver_cfg.license, 18); //2018-1-22,陈工添加,解决更换司机,疲劳驾驶报警不消除问题

                    slave_send_msg((0xee0702), 0x00, 0x00, false, spi1_up_comm_team_max); //驾驶员信息上报
                }
            } else {
                state_data.state.flag.ic_flag = false; //读卡失败，清除插卡标志
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(data_format_error));
                mw_dis.bg_2ln_pixel = 0x00; //信息提示窗口设置
            }
        } else {
            state_data.state.flag.ic_flag = false; //读卡失败，清除插卡标志
        }
        mix.ic_login = true;
        ic_driver_lisence = true;
    }
}
