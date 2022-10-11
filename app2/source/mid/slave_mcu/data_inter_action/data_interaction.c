#include "include_all.h"

const u8 hand_standard_ack[] = {0x24, 0x0D, 0x02, 0x2B, 0xFF}; //手柄协议标标准应答

//电子运单数据
const u8 gb_bill_ack[14] =
    {
        0x00, 0x00, 0x00, 0x0a,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const u8 driver_infor[] =
    {
        "\x4张三44010119790320123X\x0\x0\xc广东省交通厅\x20\x20\x02\x14"};

const u8 dial_in_dis[] = "电话呼入:";

#if 1

//无符号字符型数组左移一个元素
void array_uchar_move_left(void *s_buff, u16 cnt) {
    *(((u8 *)(s_buff)) + (cnt - 1)) = *(((u8 *)(s_buff)) + cnt);
}

//无符号短整型数组左移一个元素
void array_uint_move_left(void *s_buff, u16 cnt) {
    *(((u16 *)(s_buff)) + (cnt - 1)) = *(((u16 *)(s_buff)) + cnt);
}

//无符号短整型数组右移一个元素
void array_uint_move_right(void *s_buff, u16 cnt) {
    *(((u16 *)(s_buff)) + (cnt + 1)) = *(((u16 *)(s_buff)) + cnt);
}

//无符号长整型数组左移一个元素
void array_ulong_move_left(void *s_buff, u16 cnt) {
    *(((u32 *)(s_buff)) + (cnt - 1)) = *(((u32 *)(s_buff)) + cnt);
}

//信息使用地址类型数组左移一个元素
void array_sms_use_move_left(void *s_buff, u16 cnt) {
    use_infor_struct *ptr;

    ptr = ((use_infor_struct *)(s_buff));

    (ptr + (cnt - 1))->read_flag = (ptr + cnt)->read_flag;
    (ptr + (cnt - 1))->type_id_serial_num = (ptr + cnt)->type_id_serial_num;
    (ptr + (cnt - 1))->addr = (ptr + cnt)->addr;
}

//信息使用地址类型数组右移一个元素
void array_sms_use_move_right(void *s_buff, u16 cnt) {
    use_infor_struct *ptr;

    ptr = ((use_infor_struct *)(s_buff));

    (ptr + (cnt + 1))->read_flag = (ptr + cnt)->read_flag;
    (ptr + (cnt + 1))->type_id_serial_num = (ptr + cnt)->type_id_serial_num;
    (ptr + (cnt + 1))->addr = (ptr + cnt)->addr;
}

//比较判断指定位置的值是不是要查找的值
//参数addr指定的位置
//参数d_data要超找的值
//函数返回值含义
//0表示目标值等于搜索位置值
//1表示目标值大于搜索位置值
//2表示目标值小于搜索位置值
static u8 cmp_spi_flash_unigb(u32 addr, void *d_data) {
    u8 ret;
    u8 buff[2];
    u16 s, d;

    spi_flash_read(buff, addr, 2);
    s = (buff[1] * 0x100 + buff[0]);
    d = *((u16 *)d_data);

    if (d >= s) {
        if (d == s) {
            ret = 0; //目标值等于搜索位置值
        } else {
            ret = 1; //目标值大于搜索位置值
        }
    } else {
        ret = 2; //目标值小于搜索位置值
    }

    return ret;
}

//折半查找(二分法)
//参数dir为线性表的生长规律    TRUE为递增   FALSE为递减
//参数addr为线性表地址(可以是RAM或片内FLASH中的缓冲区也可以是SPI_FLASH中的区域)
//参数lgth为查找范围，注意这里是有符号的长整型
//参数siz为线性表步进值
//参数d_data为要查找的目标值指针
//参数ret_posi为查找成功后返回位置
//参数cmp为比较底层函数
//函数返回值    TRUE表示查找成功    FALSE表示查找失败
bool search_half(bool dir, u32 addr, signed long int lgth, u8 siz, void *d_data, u32 *ret_posi, gbk_unicode_compare_base cmp) {
    signed long int low, mid, high;
    u8 res;

    if (lgth <= siz)
        return false;

    low = 0x00;
    high = (lgth - siz);

    while (low <= high) {
        mid = ((high + low) / 2); //求中间值
        mid -= (mid % siz);       //数据对齐

        res = cmp((addr + mid), d_data); //计算比较结果
        if (res == 0) {
            *ret_posi = (addr + mid); //装载位置
            return true;              //搜索成功
        } else if (res == 1)          //目标值大于搜索位置值
        {
            if (dir == true) //递增规律
            {
                low = (mid + siz);
            } else {
                high = (mid - siz);
            }
        } else //目标值小于搜索位置值
        {
            if (dir == true) //递增规律
            {
                high = (mid - siz);
            } else {
                low = (mid + siz);
            }
        }
    }

    return false;
}

#endif

static u16 gbk_uni_table(u32 base_addr, u16 d_dat) {
    u8 buff[2];
    u32 posi; //二分法查找返回要查找的数据的位置
    u16 ret;

    if (search_half(true, base_addr, font_gb_uni_table_lgth, 4, (&d_dat), (&posi), cmp_spi_flash_unigb)) {
        spi_flash_read(buff, (posi + 2), 2);
        ret = (buff[1] * 0x100 + buff[0]);
    } else //查找失败
    {
        ret = 0x00;
    }

    return ret;
}

//GBK编码与UNICODE码转换函数
//注意1:GBK编码可与ASCII码混排输入，而UNICODE则不能
//注意2:输入的数据源为大端模式
//参数typ表示  TRUE表示GBK转成UNI  FALSE表示UNI转成GBK
//函数返回值表示转换之后的数据长度
u16 gbk_unicode(bool typ, u8 *d_buff, u8 *s_buff, u16 lgth) {
    u16 ret, dat;

    ret = 0x00;
    while (lgth) {
        if (typ == true) //数据源为国标码
        {
            if (*s_buff <= 0x7f) //ASCII码
            {
                *d_buff = 0x00;
                *(d_buff + 1) = *s_buff;

                s_buff += 1;
                lgth -= 1;
            } else {
                dat = (((*s_buff) * 0x100) + (*(s_buff + 1)));
                dat = gbk_uni_table(font_gb2uni_start_addr, dat);

                *d_buff = (dat / 0x100);
                *(d_buff + 1) = dat;

                s_buff += 2;
                lgth -= 2;
            }

            d_buff += 2;
            ret += 2;
        } else //数据源为UNICODE码
        {
            if ((*s_buff == 0x00) && (*(s_buff + 1) <= 0x7f)) //可转成ASCII码
            {
                *d_buff = *(s_buff + 1);

                d_buff += 1;
                ret += 1;
            } else {
                dat = (((*s_buff) * 0x100) + (*(s_buff + 1)));
                dat = gbk_uni_table(font_uni2gb_start_addr, dat);

                *d_buff = (dat / 0x100);
                *(d_buff + 1) = dat;

                d_buff += 2;
                ret += 2;
            }

            s_buff += 2;
            lgth -= 2;
        }
    }

    return ret;
}

//比较两个缓冲区数据
//函数返回值  0表示两个相等  1表示BUFF1大于BUFF2  2表示BUFF2大于BUFF1
//
u8 compare_buff(u8 *buff1, u16 lgth1, u8 *buff2, u16 lgth2) {
    u16 cnt;
    u8 res;

    res = 0x00;
    if (lgth1 > lgth2)
        cnt = lgth2;
    else
        cnt = lgth1;

    while (cnt) {
        if ((*buff1) != (*buff2)) {
            if ((*buff1) > (*buff2))
                res = 1;
            else
                res = 2;

            break;
        }

        buff1++;
        buff2++;
        cnt--;
    }

    if (res == 0x00) {
        if (lgth1 > lgth2)
            res = 1;

        else if (lgth1 < lgth2)
            res = 2;
    }

    return res;
}

//数组移动函数
//参数s_buff待移动缓冲区
//参数cnt为待移动的元素个数
//参数dir为方向    TRUE向右    FALSE向左
//参数function_base为底层操作函数指针
void array_move(void *s_buff, u16 cnt, bool dir, array_move_base function_base) {
    u16 i;

    if (!cnt)
        return;

    if (dir) //向右移动
    {
        while (cnt) {
            cnt--;
            function_base(s_buff, cnt);
        }
    } else //向左移动
    {
        for (i = 0x00; i < cnt; i++) {
            function_base(s_buff, i);
        }
    }
}

//******************************************************************************************************//
//----------------------------            协议解析数据处理               -------------------------------//
//******************************************************************************************************//
//下行命令
//通用应答
static void down_0x0d(void) {
    if (menu_report.dat_type == rpt_average) {
        menu_report.ok_flag = true; //收到设置成功应答标志
    }
}

//下行命令
//传感器系数
static void down_0x19(void) {
    u16 dat;

    if (query.rec.step == 1) {
        query.rec.step = 2;

        dat = *(slv_rev_data.buf + spi_rec_valid_start) * 0x100;
        dat += *(slv_rev_data.buf + spi_rec_valid_start + 1);

        data_mask_ascii((query.rec.ratio), dat, rec_para_ratio_max_lgth); //转成ASCII码
    }
}

//下行命令
//车辆VIN查询
static void down_0x1c(void) {
    if (query.regist.step == 0x0f) {
        query.regist.step = 0x10;

        _memset((query.regist.car_vin), 0x00, (rec_car_vin_max_lgth));
        _memcpy((query.regist.car_vin), (slv_rev_data.buf + spi_rec_valid_start), (rec_car_vin_max_lgth));
    }
}

//下行命令
//车牌号码查询
static void down_0x1d(void) {
    if (query.regist.step == 0x0d) {
        query.regist.step = 0x0e;

        _memset((query.regist.car_plate), 0x00, (rec_car_plate_max_lgth));
        _memcpy((query.regist.car_plate), (slv_rev_data.buf + spi_rec_valid_start), (rec_car_plate_max_lgth));
    }
}

//下行命令
//驾驶员代码查询
static void down_0x1f(void) {
    if (query.driver.step == 0x01) {
        query.driver.step = 0x02;

        ic_card.work_num_lgth = (*(slv_rev_data.buf + 2) - 2); //获取长度信息
        if (ic_card.work_num_lgth > 7)                         //驾驶员代码即工号最大长度为7字节
        {
            ic_card.work_num_lgth = 7;
        }

        _memcpy((ic_card.work_num), (slv_rev_data.buf + spi_rec_valid_start), (ic_card.work_num_lgth));
    }
}

//下行命令
//驾驶证号码查询
static void down_0x20(void) {
    if (query.driver.step == 0x03) {
        query.driver.step = 0x04;

        ic_card.driv_lic_lgth = (*(slv_rev_data.buf + 2) - 2); //获取长度信息
        if (ic_card.driv_lic_lgth > 18)                        //驾驶证号码最大长度为18字节
        {
            ic_card.driv_lic_lgth = 18;
        }

        _memcpy((ic_card.driv_lic), (slv_rev_data.buf + spi_rec_valid_start), (ic_card.driv_lic_lgth));
    }
}

//下行命令
//速度模式设置
//220828速度类型:设置【询问】返回的【记录值】
static void down_0x21(void) {
    if (query.rec.step == 3) {
        query.rec.step = 4;
        query.rec.speed_type = *(slv_rev_data.buf + spi_rec_valid_start);
        logd("query.rec.step == 3, is %d, set step = 4, set speed_type = %d", query.rec.step, query.rec.speed_type);
    } else {
        logd("query.rec.step != 3, is %d", query.rec.step);
    }
}

//下行命令
//扩展多路串口usart0:
static void down_0x23(void) {
    uart_set_ok_flag[0] = 0x00;
    uart_set_ok_flag[1] = 0x00;
    uart_set_ok_flag[2] = 0x00;

    memset(extend_uart_buff, 0, 36);
    extend_uart_cnt = *(slv_rev_data.buf + 3);

    if (extend_uart_cnt == 0x02) {
        extend_uart_buff[0] = *(slv_rev_data.buf + 4);
        extend_uart_buff[1] = *(slv_rev_data.buf + 5) * 0x1000000 + *(slv_rev_data.buf + 6) * 0x10000 + *(slv_rev_data.buf + 7) * 0x100 + *(slv_rev_data.buf + 8);
        extend_uart_buff[2] = *(slv_rev_data.buf + 9) * 0x1000000 + *(slv_rev_data.buf + 10) * 0x10000 + *(slv_rev_data.buf + 11) * 0x100 + *(slv_rev_data.buf + 12);
        extend_uart_buff[3] = *(slv_rev_data.buf + 13);
        extend_uart_buff[4] = *(slv_rev_data.buf + 14) * 0x1000000 + *(slv_rev_data.buf + 15) * 0x10000 + *(slv_rev_data.buf + 16) * 0x100 + *(slv_rev_data.buf + 17);
        extend_uart_buff[5] = *(slv_rev_data.buf + 18) * 0x1000000 + *(slv_rev_data.buf + 19) * 0x10000 + *(slv_rev_data.buf + 20) * 0x100 + *(slv_rev_data.buf + 21);
    } else if (extend_uart_cnt == 0x01) {
        extend_uart_buff[0] = *(slv_rev_data.buf + 4);
        extend_uart_buff[1] = *(slv_rev_data.buf + 5) * 0x1000000 + *(slv_rev_data.buf + 6) * 0x10000 + *(slv_rev_data.buf + 7) * 0x100 + *(slv_rev_data.buf + 8);
        extend_uart_buff[2] = *(slv_rev_data.buf + 9) * 0x1000000 + *(slv_rev_data.buf + 10) * 0x10000 + *(slv_rev_data.buf + 11) * 0x100 + *(slv_rev_data.buf + 12);
    }

    if (menu_report.dat_type == rpt_com) {
        menu_report.ok_flag = true; //收到设置成功应答标志
    }

    menu_cnt.menu_other = mo_reset;
    menu_cnt.next_regular = true;
    *(((uint *)(&menu_cnt)) + 3) = 0x01;

    menu_cnt.menu_flag = true; //更新菜单
}

//下行命令
//车牌颜色
static void down_0x26(void) {
    if (query.regist.step == 0x0b) {
        query.regist.step = 0x0c;

        ; //协议有待完善  CKP  20140306
        query.regist.car_plate_color = *(slv_rev_data.buf + spi_rec_valid_start);
    }
}

//下行命令
//厂商ID返回
static void down_0x27(void) {
    u8 n;

    if (query.regist.step == 0x05) {
        query.regist.step = 0x06;

        n = *(slv_rev_data.buf + spi_rec_valid_start - 1) - 2;
        _memset((query.regist.manufacturer_id), 0x00, (rec_manufacturer_id_max_lgth));
        _memcpy((query.regist.manufacturer_id), (slv_rev_data.buf + spi_rec_valid_start), n);
    }
}

//下行命令
//省域ID
static void down_0x60(void) {
    if (query.regist.step == 0x01) {
        query.regist.step = 0x02;

        query.regist.province_id[0] = *(slv_rev_data.buf + spi_rec_valid_start);
        query.regist.province_id[1] = *(slv_rev_data.buf + spi_rec_valid_start + 1);
    }
}

//下行命令
//市县域ID
static void down_0x61(void) {
    if (query.regist.step == 0x03) {
        query.regist.step = 0x04;

        query.regist.city_id[0] = *(slv_rev_data.buf + spi_rec_valid_start);
        query.regist.city_id[1] = *(slv_rev_data.buf + spi_rec_valid_start + 1);
        query.regist.city_id[2] = *(slv_rev_data.buf + spi_rec_valid_start + 2);
        query.regist.city_id[3] = *(slv_rev_data.buf + spi_rec_valid_start + 3);
    }
}

//下行命令
//终端型号
static void down_0x62(void) {
    u8 n;

    if (query.regist.step == 0x07) {
        query.regist.step = 0x08;

        n = *(slv_rev_data.buf + spi_rec_valid_start - 1) - 2;
        _memset((query.regist.terminal_type), 0x00, (rec_terminal_type_max_lgth));
        _memcpy((query.regist.terminal_type), (slv_rev_data.buf + spi_rec_valid_start), n);
    }
}

//下行命令
//终端ID
static void down_0x63(void) {
    if (query.regist.step == 0x09) {
        query.regist.step = 0x0a;

        _memcpy((query.regist.terminal_id), (slv_rev_data.buf + spi_rec_valid_start), (rec_terminal_id_max_lgth));
    }
}

//下行命令
//车辆号牌分类
static void down_0x66(void) {
    u8 n;

    if (query.rec.step == 7) {
        query.rec.step = 8;

        n = (*(slv_rev_data.buf + spi_rec_valid_start - 1) - 2);
        if (n > rec_car_type_max_lgth) {
            n = rec_car_type_max_lgth;
        }

        _memset((query.rec.car_type), 0x00, rec_car_type_max_lgth);
        _memcpy((query.rec.car_type), (slv_rev_data.buf + spi_rec_valid_start), n);
    }
}

//下行命令
//打印标准类型
static void down_0x6a(void) {
    if (query.rec.step == 5) {
        query.rec.step = 6;

        query.rec.print_type = *(slv_rev_data.buf + spi_rec_valid_start);
    }
}

//下行命令
//定位模式
static void down_0x6b(void) {
    u8 dat;

    dat = *(slv_rev_data.buf + spi_rec_valid_start); //获取状态值

    if ((testbit(dat, 0)) && (testbit(dat, 1))) {
        host_no_save_para.set_gps_module_type = 3; //双模模式
    } else if (testbit(dat, 1)) {
        host_no_save_para.set_gps_module_type = 2; //单北斗模式
    } else {
        host_no_save_para.set_gps_module_type = 1; //单GPS模式
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
}

//下行命令
//设置成功
static void down_0xb1(void) {
    menu_report.ok_flag = true; //收到设置成功应答标志
    menu_cnt.menu_flag = true;
}

//下行命令
//位置数据
static void down_0xa0(void) {
    _memcpy(((u8 *)(&gps_data)), (slv_rev_data.buf + spi_rec_valid_start), (sizeof(gps_data)));
    menu_cnt.menu_flag = true; //更新菜单

#ifdef sla_ack_0xa0_comm_en
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
#endif
}

//下行命令
//接收调度信息
static void down_0xa1(void) {
}

//下行命令
//接收中心信息
static void down_0xa2(void) {
    ;
}

//下行命令
//发送信息操作失败
static void down_0xa3(void) {
    ;
}

//下行命令
//发送信息操作成功
static void down_0xa4(void) {
    ;
}

//下行命令
//电话来电
static void down_0xa5(void) {
    u8 tmp_flag;
    u16 t1, t2, t3; //t1为最左边那个号码的位置，t2为最右边那个号码的位置，t3为中间那个号码的位置
    u16 cnt;
    phbk_count_struct phbk_count;
    phbk_data_struct phbk_data;
    phbk_index num_index;

    cnt = 0x00;
    tmp_flag = 0x00;
    t1 = 0x00;
    t3 = 0x00;

    _memset((void *)(&phbk_count), 0x00, sizeof(phbk_count_struct));
    _memset((void *)(&phbk_data), 0x00, sizeof(phbk_data_struct));
    if (phone_data.step == call_in) {
        slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
    } else {
        (*((u16 *)(&(phone_data.buff)))) = (slv_rev_data.len - 5);
        _memcpy((phone_data.buff + 2), (slv_rev_data.buf + spi_rec_valid_start), (slv_rev_data.len - 5));

        lcd_dis_one_line(0, 0, dial_in_dis, false);

        phone_data.step = call_in;
        menu_cnt.menu_other = mo_telephone;
        menu_cnt.menu_flag = true;
        //从电话簿里面去寻找该来电的名字，用二分法查找
        spi_flash_read(((u8 *)(&phbk_count)), phbk_start_addr_count, (sizeof(phbk_count_struct))); //提取总数
        t2 = phbk_count.total;
        spi_flash_read(((u8 *)(&num_index)), phbk_start_addr_num_index, (sizeof(phbk_index)));
        if (phbk_count.total > 0) {
            spi_flash_read(((u8 *)(&phbk_data)), phbk_data_abs_addr(*(num_index)), (sizeof(phbk_data_struct)));
            if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth - 1), ((phone_data.buff) + 2), (phone_data.buff[0])) == 0) {
                tmp_flag = 0x01;
            }
            while (tmp_flag != 0x01) {
                t3 = (t1 + t2 + 1) / 2;
                spi_flash_read(((u8 *)(&phbk_data)), phbk_data_abs_addr(*(num_index + t3)), (sizeof(phbk_data_struct)));

                if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth - 1), ((phone_data.buff) + 2), (phone_data.buff[0])) == 1) //在前半部分
                {
                    t2 = t3;
                } else if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth - 1), ((phone_data.buff) + 2), (phone_data.buff[0])) == 2) //在后半部分
                {
                    t1 = t3;
                }

                else if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth - 1), ((phone_data.buff) + 2), (phone_data.buff[0])) == 0) //正好是中间这个数
                {
                    tmp_flag = 0x01;
                }
                cnt++;
                if (cnt > phbk_count.total) {
                    break;
                }
            }
            if (tmp_flag == 0x01) //找到电话簿里对应的名字
            {
                if ((phbk_data.type == phbk_type_in_only) || (phbk_data.type == phbk_type_in_out)) //可呼入
                {
                    lcd_dis_one_page(16, (*((u16 *)(&(phbk_data.name.lgth)))), (phbk_data.name.buff));
                    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
                    write_data_to_call_record_flash(flash_missed_call_record_addr, phbk_data);
                    _memcpy((u8 *)(&phbk_call_in_data), (u8 *)&phbk_data, sizeof(phbk_data));
                } else //仅可呼出，不可呼入
                {
                    phone_data.step = call_halt;                                             //通知主机挂机
                    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
                }
            } else //没有找到电话簿里对应的名字
            {
                phone_data.step = call_halt;                                             //通知主机挂机
                slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
            }
        } else {
            lcd_dis_one_page(16, (*((u16 *)(&(phone_data.buff)))), (phone_data.buff + 2));
            slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
            phbk_call_in_data.num.lgth = phone_data.buff[0];
            _memcpy(phbk_call_in_data.num.buff, (&phone_data.buff[2]), phbk_call_in_data.num.lgth);
            write_data_to_call_record_flash(flash_missed_call_record_addr, phbk_call_in_data);
        }
        lcd_area_dis_set(4, 7, 0, 131, 0x00);
    }
}

//下行命令
//对方挂机
static void down_0xa6(void) {
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据

    phone_data.step = call_end;
    menu_cnt.menu_other = mo_telephone;
    menu_cnt.menu_flag = true;
}

//下行命令
//电话接通
static void down_0xa7(void) {
    static u8 tmp_call_in_cnt; //由于主机每次接听电话都会发两次0xa7命令给从机，所以必须用此计数器将第一次接收到的该命令屏蔽掉

    if (phone_data.step == call_out) {
        slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据

        phone_data.dtmf_ptr = 0x00;
        phone_data.dtmf_cnt = 0x00; //清零计数器
        phone_data.tim = jiffies;   //时间同步

        phone_data.step = call_connect;
        menu_cnt.menu_other = mo_telephone;
        menu_cnt.menu_flag = true;

    } else if (phone_data.step == call_in) //如果是呼入才为已接电话
    {
        tmp_call_in_cnt++;
        slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
        if (tmp_call_in_cnt == 0x02) {
            tmp_call_in_cnt = 0x00;
            //删除刚刚自加的未接电话记录
            spi_flash_read(&call_record.call_record_cnt, flash_missed_call_record_addr, sizeof(call_record.call_record_cnt));
            call_record.call_record_cnt--;
            flash25_program_auto_save(flash_missed_call_record_addr, &call_record.call_record_cnt, sizeof(call_record.call_record_cnt));
            //存储已接号码

            write_data_to_call_record_flash(flash_received_call_record_addr, phbk_call_in_data);
            _memset((void *)(&phbk_call_in_data), 0x00, sizeof(phbk_call_in_data));

            spi_flash_read(((u8 *)(&call_record)), flash_received_call_record_addr, (sizeof(call_record_struct)));
            if (call_record.call_record_cnt > call_record_num)
                call_record.call_record_cnt = call_record_num;
        }
    }
}

//下行命令
//自检信息下发
static void down_0xa8(void) {
    if (menu_report.dat_type == rpt_com) {
        menu_report.ok_flag = true; //收到设置成功应答标志
    }

    _memcpy(((u8 *)(&query.auto_check)), (slv_rev_data.buf + spi_rec_valid_start), (sizeof(query.auto_check))); //固定提取19个字节的数据
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max);                                    //装载应答数据

    menu_cnt.menu_other = mo_auto_check;
    menu_cnt.menu_flag = true; //更新菜单
}

//下行命令
//主机状态返回
static void down_0xae(void) {
    u8 i, j;
    u8 *ptr;
    u8 m, n;

    ptr = slv_rev_data.buf;
    i = spi_rec_valid_start;
    m = 0x00;
    n = 0x00;

    if (ptr[2] > 0x70)
        query.host.type = true;
    else
        query.host.type = false;

    if (*(ptr + i) == 'M') {
        i += 1;
        //本机ID
        for (j = 0x00; j < 19; j++) {
            if (*(ptr + i + j) == 'C') {
                break;
            }

            query.host.id_num[j + 1] = *(ptr + i + j);
        }
        query.host.id_num[0] = j;
        query.host.id_num[j + 1] = '\x0';

        i = i + j + 1;
        //短信服务号码
        for (j = 0x00; j < 19; j++) {
            if (*(ptr + i + j) == 'I') {
                break;
            }

            query.host.sms_num[j + 1] = *(ptr + i + j);
        }
        query.host.sms_num[0] = j;
        query.host.sms_num[j + 1] = '\x0';

        i = i + j + 1;
        //IP地址
        for (j = 0x00; j < 32; j++) {
            if (*(ptr + i + j) == 'U')
                break;

            if (*(ptr + i + j) == '"') {
                m++;
                if (m == 1)
                    query.host.ip[j] = ',';
                //将第一个引号改为逗号
                else
                    n = 1;
                //跳过第二个引号
            } else {
                query.host.ip[j - n] = *(ptr + i + j);
            }
        }
        query.host.ip_lgth = (j - 1);

        i = i + j + 1;
        //用户号码
        for (j = 0x00; j < 15; j++) {
            if (*(ptr + i + j) == 0x80) {
                break;
            }
            query.host.user_num[j + 1] = *(ptr + i + j);
        }
        query.host.user_num[0] = j;
        query.host.user_num[j + 1] = '\x0';

        i = i + j;
        ptr += i;
        query.host.transmit_mode = *ptr++;
        //发送方式
        query.host.transmit_time[0] = *ptr++;
        query.host.transmit_time[1] = *ptr++;
        //发送时间
        query.host.speed = *ptr++;
        //超速报警
        query.host.outage = *ptr++;
        //断电报警
        query.host.stop = *ptr++;
        //停车报警

        query.host.call_time[0] = *ptr++;
        query.host.call_time[1] = *ptr++;
        //拨打时间
        query.host.answer_time[0] = *ptr++;
        query.host.answer_time[1] = *ptr++;
        //接听时间

        //主机版本及日期
        for (j = 0x00; j < 6; j++)
            query.host.ver_date[j] = *ptr++;

        //除自定义的所有选项，该款机型将全部显示  CMNET
        if ((*(ptr) >= '1') && (*(ptr) <= '6')) {
            ptr++;
            _memcpy(query.host.apn, "CMNET", 5);
            query.host.apn_lgth = 5;
        } else {
            ptr++;
            for (j = 0x00; j < 32; j++) {
                if ((*ptr == 'e') && (*(ptr + 1) == '1'))
                    break;
                query.host.apn[j] = *ptr++;
            }
            query.host.apn_lgth = j;
        }

        m = 0x00;
        n = 0x00;
        ptr += 2;
        if (query.host.type == true) {
            //备用IP地址
            for (j = 0x00; j < 32; j++) {
                if (*ptr == 'K')
                    break;

                if (*ptr == '"') {
                    m++;
                    if (m == 1)
                        query.host.Backup_ip[j] = ',';
                    //将第一个引号改为逗号
                    else
                        n = 1;
                    //跳过第二个引号
                    ptr++;
                } else {
                    query.host.Backup_ip[j - n] = *ptr++;
                }
            }
            query.host.Backup_ip_lgth = (j - 1);

            ptr++;
            for (j = 0x00; j < 24; j++) {
                query.host.ver_date2[j] = *ptr++;
            }
            query.host.ver_date2[24] = '\x0'; //防止主机发送过来的数据没有结束标志

            m = 0x00;
            n = 0x00;

            //IP地址
            for (j = 0x00; j < 32; j++) {
                if (*(ptr + j) == 'U')
                    break;

                if (*(ptr + j) == '"') {
                    m++;
                    if (m == 1)
                        query.host.ip3[j] = ',';
                    //将第一个引号改为逗号
                    else
                        n = 1;
                    //跳过第二个引号
                } else {
                    query.host.ip3[j - n] = *(ptr + j);
                }
            }
            query.host.ip3_lgth = (j - 1);
        } else {
            query.host.Backup_ip_lgth = 0x00; //非扩展协议不包含备用IP
            query.host.ver_date2[0] = '\x0';  //非扩展协议不包含版本号字符串
        }

        if (menu_report.dat_type == rpt_com) {
            menu_report.ok_flag = true; //收到设置成功应答标志
        }

        slave_send_msg((0x0d0000), (0x00), 0x00, true, spi1_up_comm_team_max); //装载应答数据

        menu_cnt.menu_other = mo_host_para;
        menu_cnt.menu_flag = true; //更新菜单
    }
}

//下行命令
//手机短信pdu格式
static void down_0xaf(void) {
    u8 *ptr;
    sms_index_struct sms_index;
    u8 gb2312_buff[sms_phone_single_piece_lgth];
    u16 n, addr;
    u16 gb2312_cnt;

    n = *(slv_rev_data.buf + 2) - 2; //老协议无需长度限制判断
    ptr = (slv_rev_data.buf + spi_rec_valid_start);

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_phone_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max) {
        sms_index.total = sms_total_max;
    }

    addr = find_the_symbol(ptr, n, '"', 5); //找到第五个引号，为内容的起始处
    gb2312_cnt = gbk_unicode(false, gb2312_buff, (ptr + addr + 1), (n - addr - 2));
    _memcpy((ptr + addr), gb2312_buff, gb2312_cnt); //覆盖引号

    n = (addr + gb2312_cnt);               //待显示的所有信息内容的长度
    *(slv_rev_data.buf + 1) = (n % 0x100); //借用缓冲区存储长度
    *(slv_rev_data.buf + 2) = (n / 0x100);
    n += 2;

    sms_operate_hdl((&sms_index), sms_op_add, 0x00, n, (slv_rev_data.buf + 1));
    flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_phone;
        spi_flash_read(((u8 *)(&(sms_union_data.phone))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_phone_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据

    //----------------------//  语音朗读功能
    _memcpy(sound_play_buff, (slv_rev_data.buf + 1), n);
    slave_send_msg((0xee0000), 0x00, 0x00, false, spi1_up_comm_team_max);
    //----------------------//
}

//下行命令
//手机短信文本格式
static void down_0xb0(void) {
    u8 *ptr, buff[300]; //老协议300字节长度足矣  前两个字节存放长度  大端模式
    u16 n, addr;
    sms_index_struct sms_index;

    n = *(slv_rev_data.buf + 2) - 2; //老协议无需长度限制判断
    ptr = (slv_rev_data.buf + spi_rec_valid_start);

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_phone_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max) {
        sms_index.total = sms_total_max;
    }

    addr = find_the_symbol(ptr, n, '"', 5); //找到第五个引号，为内容的起始处
    n -= 2;                                 //去掉内容两端的引号
    _memcpy((buff + 2), ptr, addr);
    _memcpy((buff + 2 + addr), (ptr + addr + 1), (n - addr));

    *(buff + 0) = (n % 0x100);
    *(buff + 1) = (n / 0x100);
    n += 2;

    sms_operate_hdl((&sms_index), sms_op_add, 0x00, n, (buff));
    flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_phone;
        spi_flash_read(((u8 *)(&(sms_union_data.phone))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_phone_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据

    //----------------------//  语音朗读功能
    _memcpy(sound_play_buff, buff, n);
    slave_send_msg((0xee0000), 0x00, 0x00, false, spi1_up_comm_team_max);
    //----------------------//
}

//下行命令
//IC卡开卡命令
static void down_0xc2(void) {
    u8 ok_flag; //开卡执行成功标志  0执行失败  1执行成功
    u8 *ptr;    //数据指针

    /*
    if(state_data.state.flag.ic_flag == false)  //若无IC卡插入则直接退出
    {
    	return;
    }
    */
    //不判断状态位，因为状态位在有卡插入且数据校验通过后才置位  CKP  2014-06-20

    ptr = (slv_rev_data.buf + spi_rec_valid_start);
    ok_flag = 0x00;

    if (ic_type == ic_2402) { //IC2402
        logd("IC数据:%c", *(ptr + 7));
        if (at24cxx_write_data(at24_i2c_slave_addr, eeprom_start_addr, ptr, eeprom_data_lgth)) {
            ok_flag = 0x01;
        }
    } else if (ic_type == ic_4442) { //IC4442
        if (ic4442_write_data(ic4442_default_password, write_main_mem, 0x00, ptr, eeprom_data_lgth)) {
            ok_flag = 0x01;
        }
    }

    if (ok_flag) {
        slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max);
    }
}

//下行命令
//文本信息
static void down_0xee_0x8300(void) {
    u8 type;
    u16 n;
    sms_index_struct sms_index;

    n = ((*(slv_rev_data.buf + 2)) * 0x100) + (*(slv_rev_data.buf + 3)) - 5;
    if (n > sms_center_single_piece_lgth) //长度限制
    {
        n = sms_center_single_piece_lgth;
    }
    type = (*(slv_rev_data.buf + spi_rec_expand_valid_start));

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_center_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    *(slv_rev_data.buf + spi_rec_expand_valid_start - 1) = (n % 0x100);
    *(slv_rev_data.buf + spi_rec_expand_valid_start) = (n / 0x100);
    n += 2; //借用缓冲区存储长度

    sms_operate_hdl((&sms_index), sms_op_add, type, n, (slv_rev_data.buf + spi_rec_expand_valid_start - 1));
    flash25_program_auto_save(flash_sms_center_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_center;
        spi_flash_read(((u8 *)(&(sms_union_data.center))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_center_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_center_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据

    if (type & (bit_move(3))) //有朗读信息的请求
    {
        if ((n - 2) > sound_play_buff_max_lgth)
            n = sound_play_buff_max_lgth;
        else
            n -= 2;

        _memcpy((sound_play_buff + 2), (slv_rev_data.buf + spi_rec_expand_valid_start + 1), n);
        (*((u16 *)sound_play_buff)) = n;

        slave_send_msg((0xee0000), 0x00, 0x00, false, spi1_up_comm_team_max);
    }
}

//下行命令
//事件信息
static void down_0xee_0x8301(void) {
    u8 type;
    u8 flag, i, n;
    u8 *ptr;

    u8 id;
    u16 lgth, cnt;

    sms_index_struct sms_index;
    sms_op_enum sms_op;

    flag = 0x00; //删除标志清零
    sms_op = sms_op_add;
    type = (*(slv_rev_data.buf + spi_rec_expand_valid_start));  //设置类型
    n = (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //总条数
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start + 2);  //给指针赋值

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_fix_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    switch (type) {
    case 0x00: //删除全部
        flag = 1;
        sms_index.total = 0x00;
        break;

    case 0x01: //更新
        sms_index.total = 0x00;
        sms_op = sms_op_add;
        break;

    case 0x02: //追加
        sms_op = sms_op_add;
        break;

    case 0x03: //修改
        sms_op = sms_op_change;
        break;

    case 0x04: //删除
        sms_op = sms_op_del;
        break;

    default: //数据异常
        return;
    }

    if (flag == 0x00) //非全部删除操作
    {
        for (i = 0x00; i < n; i++) {
            id = *(ptr + 0);
            cnt = *(ptr + 1);

            if (cnt > (sms_fix_single_piece_lgth - 40)) //长度限制
            {
                lgth = (sms_fix_single_piece_lgth - 40);
            } else {
                lgth = cnt;
            }

            *(ptr + 0) = lgth;
            *(ptr + 1) = 0x00; //借用缓冲区存储长度
            lgth += 2;

            sms_operate_hdl((&sms_index), sms_op, id, lgth, ptr);

            ptr += (cnt + 2);
        }
    }

    flash25_program_auto_save(flash_sms_fix_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if ((menu_cnt.menu_other != mo_sms_display) && (sms_op == sms_op_add)) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_fix;
        sms_up_center.fix_id = ((sms_index.use_infor + 0)->type_id_serial_num);
        spi_flash_read(((u8 *)(&(sms_union_data.fix))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_fix_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_fix_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
}

//下行命令
//提问下发
static void down_0xee_0x8302(void) {
    sms_index_struct sms_index;
    sms_ask_content_struct sms_ask;

    u8 *ptr;
    u16 i, cnt;
    u16 n;
    u16 ser_num;

    cnt = ((*(slv_rev_data.buf + 2)) * 0x100) + (*(slv_rev_data.buf + 3)) - 4; //计算本包数据的有效数据长度
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start);
    ser_num = ((*(ptr + 0)) * 0x100) + (*(ptr + 1)); //记录流水号

    n = (*(ptr + 3)); //问题长度
    i = 4;            //修改指针，指向问题内容

    if (n > (sms_ask_content_lgth - 40)) //判断长度限制
    {
        sms_ask.lgth = (sms_ask_content_lgth - 40);
    } else {
        sms_ask.lgth = n;
    }

    _memcpy(sms_ask.dat, (ptr + i), sms_ask.lgth); //赋值内容
    i += n;                                        //修改指针，指向问题答案

    sms_ask.ans_cnt = 0x00;
    while (i < cnt) {
        if (sms_ask.ans_cnt >= sms_ask_answer_total_max) //答案数目超过范围
            break;

        (sms_ask.answer + sms_ask.ans_cnt)->ans_id = *(ptr + i); //答案ID
        n = ((*(ptr + i + 1)) * 0x100) + (*(ptr + i + 2));       //答案长度

        if (n > sms_ask_answer_content_lgth) //判断长度限制
        {
            (sms_ask.answer + sms_ask.ans_cnt)->ans_lgth = sms_ask_answer_content_lgth;
        } else {
            (sms_ask.answer + sms_ask.ans_cnt)->ans_lgth = n;
        }

        _memcpy(((sms_ask.answer + sms_ask.ans_cnt)->ans_dat), (ptr + i + 3), ((sms_ask.answer + sms_ask.ans_cnt)->ans_lgth)); //赋值内容

        i = (i + 3 + n);
        sms_ask.ans_cnt++;
    }

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_ask_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    sms_operate_hdl((&sms_index), sms_op_add, ser_num, (sizeof(sms_ask)), ((u8 *)(&sms_ask)));
    flash25_program_auto_save(flash_sms_ask_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_ask;
        sms_up_center.ask_num = ((sms_index.use_infor + 0)->type_id_serial_num);
        spi_flash_read(((u8 *)(&(sms_union_data.ask))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_ask_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_ask_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
}

//下行命令
//信息点播
static void down_0xee_0x8303(void) {
    u8 type;
    u8 flag, i, n;
    u8 *ptr;

    u8 id;
    u16 lgth, cnt;

    sms_index_struct sms_index;
    sms_op_enum sms_op;

    flag = 0x00; //删除标志清零
    sms_op = sms_op_add;
    type = (*(slv_rev_data.buf + spi_rec_expand_valid_start));  //设置类型
    n = (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //总条数
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start + 2);  //给指针赋值

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_vod_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    switch (type) {
    case 0x00: //删除全部
        flag = 1;
        sms_index.total = 0x00;
        break;

    case 0x01: //更新
        sms_index.total = 0x00;
        sms_op = sms_op_add;
        break;

    case 0x02: //追加
        sms_op = sms_op_add;
        break;

    case 0x03: //修改
        sms_op = sms_op_change;
        break;

    default: //数据异常
        return;
    }

    if (flag == 0x00) //非全部删除操作
    {
        for (i = 0x00; i < n; i++) {
            id = *(ptr + 0);
            cnt = ((*(ptr + 1)) * 0x100) + (*(ptr + 2));

            if (cnt > (sms_vod_single_piece_lgth - 40)) //长度限制
            {
                lgth = (sms_vod_single_piece_lgth - 40);
            } else {
                lgth = cnt;
            }

            *(ptr + 1) = (lgth % 0x100);
            *(ptr + 2) = (lgth / 0x100); //借用缓冲区存储长度
            lgth += 2;

            sms_operate_hdl((&sms_index), sms_op, id, lgth, (ptr + 1));

            ptr = (ptr + cnt + 3);
        }
    }

    flash25_program_auto_save(flash_sms_vod_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if ((menu_cnt.menu_other != mo_sms_display) && (sms_op == sms_op_add)) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_vod;
        sms_up_center.vod_type = ((sms_index.use_infor + 0)->type_id_serial_num);
        spi_flash_read(((u8 *)(&(sms_union_data.vod))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_vod_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_vod_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
}

//下行命令
//服务信息
static void down_0xee_0x8304(void) {
    u16 n;
    sms_index_struct sms_index;

    n = ((*(slv_rev_data.buf + 2)) * 0x100) + (*(slv_rev_data.buf + 3)) - 7;
    if (n > sms_serve_single_piece_lgth) //长度限制
    {
        n = sms_serve_single_piece_lgth;
    }

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_serve_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    *(slv_rev_data.buf + spi_rec_expand_valid_start + 1) = (n % 0x100);
    *(slv_rev_data.buf + spi_rec_expand_valid_start + 2) = (n / 0x100); //借用缓冲区存储长度
    n += 2;

    sms_operate_hdl((&sms_index), sms_op_add, (*(slv_rev_data.buf + spi_rec_expand_valid_start)), n, (slv_rev_data.buf + spi_rec_expand_valid_start + 1));
    flash25_program_auto_save(flash_sms_serve_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //只有在非信息显示界面下才会自动弹出信息显示
    {
        sms_up_center.sms_type = sms_type_serve;
        spi_flash_read(((u8 *)(&(sms_union_data.serve))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_serve_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_serve_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
}

//下行命令
//电话本
static void down_0xee_0x8401(void) {
    u8 *ptr;
    u8 op, cnt, lgth;
    u16 i, n;

    phbk_count_struct phbk_count;
    phbk_data_struct phbk_data;

    op = (*(slv_rev_data.buf + spi_rec_expand_valid_start));    //设置类型
    n = (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //总条数
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start + 2);  //给指针赋值

    if (op == 0x00) {
        phbk_data_flash_init();
    } else if (op != 0x00) //非全部删除操作
    {
        //------------// ////////////////
        if (op >= 0x03) //不处理修改操作，后续完善
            goto ret_0x8401;
        //------------//

        spi_flash_read(((u8 *)(&phbk_count)), phbk_start_addr_count, (sizeof(phbk_count_struct))); //提取总数
        (phbk_count.posi) = (phbk_count.posi) % phone_book_total_max;
        if ((phbk_count.total) > phone_book_total_max) {
            (phbk_count.total) = phone_book_total_max;
        }

        if (op == 0x01) //更新操作
        {
            phbk_count.total = 0x00;
            phbk_count.posi = 0x00;
        }

        for (i = 0x00; i < n; i++) {
            //------------// ////////////////
            if (phbk_count.total >= phone_book_total_max) //超过范围不做处理，后续完善
                break;
            //--------------------//

            phbk_data.type = (phbk_type_enum)(*(ptr++) - 1); //类型

            cnt = *(ptr++); //号码长度
            if (cnt > phone_book_num_max_lgth) {
                lgth = phone_book_num_max_lgth;
            } else {
                lgth = cnt;
            }
            phbk_data.num.lgth = lgth;
            _memcpy((phbk_data.num.buff), (ptr), lgth);
            ptr += cnt;

            cnt = *(ptr++); //姓名长度
            if (cnt > phone_book_name_max_lgth) {
                lgth = phone_book_name_max_lgth;
            } else {
                lgth = cnt;
            }
            phbk_data.name.lgth = lgth;
            _memcpy((phbk_data.name.buff), (ptr), lgth);
            ptr += cnt;

            if ((op == 0x01) || (op == 0x02))
                op = true;
            else
                op = false;

            phbk_operate_hdl(((bool)op), (&phbk_count), (&phbk_data));
        }

        flash25_program_auto_save(phbk_start_addr_count, ((uchar *)(&phbk_count)), (sizeof(phbk_count_struct))); //保存总数
    }

ret_0x8401:
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
}

//下行命令
//主动提取驾驶员信息
static void down_0xee_0x8702(void) {
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //装载应答数据
    slave_send_msg((0xee0702), 0x00, 0x00, false, spi1_up_comm_team_max);
}

//下行命令散转表(建在FLASH区)
const slave_rev_handset SLAVE_RECIVE_TAB[] =
    {
        {0x000d0000, down_0x0d},
        {0x00190000, down_0x19},
        {0x001c0000, down_0x1c},
        {0x001d0000, down_0x1d},
        {0x001f0000, down_0x1f},
        {0x00200000, down_0x20},
        {0x00210000, down_0x21},
        {0x00230000, down_0x23},
        {0x00260000, down_0x26},
        {0x00270000, down_0x27},
        {0x00600000, down_0x60},
        {0x00610000, down_0x61},
        {0x00620000, down_0x62},
        {0x00630000, down_0x63},
        {0x00660000, down_0x66},
        {0x006a0000, down_0x6a},
        {0x006b0000, down_0x6b},
        {0x00b10000, down_0xb1},
        {0x00a00000, down_0xa0},
        {0x00a10000, down_0xa1},
        {0x00a20000, down_0xa2},
        {0x00a30000, down_0xa3},
        {0x00a40000, down_0xa4},
        {0x00a50000, down_0xa5},
        {0x00a60000, down_0xa6},
        {0x00a70000, down_0xa7},
        {0x00a80000, down_0xa8},
        {0x00ae0000, down_0xae},
        {0x00af0000, down_0xaf},
        {0x00b00000, down_0xb0},
        {0x00c20000, down_0xc2},
        {0x00ee8300, down_0xee_0x8300},
        {0x00ee8301, down_0xee_0x8301},
        {0x00ee8302, down_0xee_0x8302},
        {0x00ee8303, down_0xee_0x8303},
        {0x00ee8304, down_0xee_0x8304},
        {0x00ee8401, down_0xee_0x8401},
        {0x00ee8702, down_0xee_0x8702},
        {NULL, NULL}};

/*********************************************************
名    称：slave_recive_msg
功    能：从机接收数据
输入参数：无
输    出：无
**********************************************************/
u8 slave_recive_msg(u8 *str_t, u16 len_t) {
    if (len_t > HANDSET_MAX_SIZE || len_t < HANDSET_MIN_SIZE || *str_t != 0x24)
        return CMD_ERROR;

    _memset((u8 *)&slv_rev_data.buf, 0x00, HANDSET_MIN_SIZE);
    slv_rev_data.len = _memcpy_len((u8 *)&slv_rev_data.buf, str_t, len_t);

    return CMD_ACCEPT;
}

/*********************************************************
名    称：slave_send_data_proc
功    能：处理从机数据的发送，考虑到嵌套函数级数过多
，采用数据共享方式传递数据
输入参数：无
输    出：无
**********************************************************/
void slave_recive_data_proc(void) {
    u32 index_t;
    u8 xvalue;
    u16 i;

    if (slv_rev_data.len == 0 || slv_rev_data.buf[0] != 0x24)
        return;

    if (slv_rev_data.len < 5 || slv_rev_data.len > HANDSET_MAX_SIZE) {
        goto slv_recive_data_err;
    }

    index_t = 0;
    if (slv_rev_data.buf[1] == 0xee) {
        index_t = ((slv_rev_data.buf[4] << 8) | slv_rev_data.buf[5]) | 0x00ee0000;
        index_t &= 0x00ffffff;
    } else {
        index_t = slv_rev_data.buf[1] << 16;
        index_t &= 0x00ff0000;
    }

    xvalue = 0;
    for (i = 0; i < slv_rev_data.len - 2; i++)
        xvalue ^= slv_rev_data.buf[i];

    if (slv_rev_data.buf[slv_rev_data.len - 2] == xvalue) {
        for (i = 0; SLAVE_RECIVE_TAB[i].func != NULL; i++) {
            if (index_t == SLAVE_RECIVE_TAB[i].index) {
                (*SLAVE_RECIVE_TAB[i].func)();
            }
        }
    } else {
        loge("SLV XOR ERR");
    }

slv_recive_data_err:
    _memset((u8 *)&slv_rev_data, 0x00, sizeof(slv_rev_data));
}

//上行包封装
//lgth是数据内容长度
void up_packeg(u32 main_comm, u8 *buff, u16 lgth) {
    u16 n;
    u16 m;
    u8 temp[single_packet_max_lgth];

    n = 0x00;
    *(temp + (n++)) = 0x24;  //包头标志
    m = main_comm / 0x10000; //命令号
    *(temp + (n++)) = m;

    if (m == expand_num)  { //扩展命令
        *(temp + (n++)) = ((lgth + 4) / 0x100);
        *(temp + (n++)) = ((lgth + 4) % 0x100); //长度

        *(temp + (n++)) = ((main_comm % 0x10000) / 0x100);
        *(temp + (n++)) = (main_comm % 0x100); //子信令
    } else {                                   //标准命令
        *(temp + (n++)) = (lgth + 2);
    }

    _memcpy((temp + n), buff, lgth);
    n += lgth;
    *(temp + n) = _get_xor(temp, n); //计算校验
    n++;

    *(temp + (n++)) = 0xff; //结束符

    if (n >= HANDSET_MAX_SIZE) {
        return;
    }

    _memset((u8 *)&slv_send_data.buf, 0x00, HANDSET_MAX_SIZE);
    slv_send_data.len = _memcpy_len((u8 *)&slv_send_data.buf, temp, n);
}

//上行命令
//发送平台调度信息
static void up_0x01(void) {
    ;
}

//上行命令
//发送手机信息
static void up_0x02(void) {
    ;
}

//上行命令
//设置终端IP地址
static void up_0x03(void) {
    u8 temp[100];
    u8 i, j, n;

    i = 0x00;
    temp[i++] = '<';
    n = (*((u16 *)(set_para.m_ip)) + 2);
    for (j = 0x00; j < n; j++) {
        if (j < 2)
            temp[i++] = set_para.m_ip[j + 2];
        else if (j == 2)
            temp[i++] = '"';
        else if (j < (set_para.m_ip[0] - 4))
            temp[i++] = set_para.m_ip[(j - 1) + 2];
        else if (j == (set_para.m_ip[0] - 4))
            temp[i++] = '"';
        else
            temp[i++] = set_para.m_ip[(j - 2) + 2];
    }
    temp[i++] = '>';

    up_packeg((0x030000), (temp), i);
}

//上行命令
//设置终端ID号
static void up_0x04(void) {
    u8 temp[100] = {0};
    u8 id_buff[12];
    u8 i, k;
    u8 *p;
    i = set_para.id[0];
    if (i < 12) {
        k = 12 - set_para.id[0];
        p = (set_para.id + 2);
        _memset(id_buff, 0x30, 12);
        _memcpy(id_buff + k, p, set_para.id[0]);
        //_memcpy( p, id_buff, 12);
        //_memset( id_buff, 0, 12);
    } else
        _memcpy(id_buff, (set_para.id + 2), 12);
    //_memcpy((temp+0), (id_buff+1), 4);
    //_memcpy((temp+4), (set_para.id+2), 12);
    _memcpy((temp + 4), id_buff, 12);

    up_packeg((0x040000), (temp), (12 + 4));
}

//上行命令
//设置短信服务号码
static void up_0x05(void) {
    u8 temp[100];
    u8 i;

    i = 0x00;
    *(temp + (i++)) = '<';
    _memcpy((temp + i), (set_para.num_s + 2), *((u16 *)(set_para.num_s)));
    i += *((u16 *)(set_para.num_s));
    *(temp + (i++)) = '>';

    up_packeg((0x050000), (temp), i);
}

//上行命令
//拨号
static void up_0x07(void) {
    up_packeg((0x070000), (phone_data.buff + 2), (*((u16 *)(&(phone_data.buff)))));
}

//上行命令
//挂机
static void up_0x08(void) {
    u8 temp;

    up_packeg((0x080000), (&temp), 0);
}

//上行命令
//发送DTMF信号
static void up_0x09(void) {
    up_packeg((0x090000), (phone_data.dtmf_buff + (phone_data.dtmf_cnt - 1)), 1);
}

//上行命令
//终端自检
static void up_0x0c(void) {
    u8 temp;

    up_packeg((0x0c0000), (&temp), 0);
}

//上行命令
//通用应答
static void up_0x0d(void) {
    u8 temp;

    up_packeg((0x0d0000), (&temp), 0);
}

//上行命令
//接听电话
static void up_0x0e(void) {
    u8 temp;

    up_packeg((0x0e0000), (&temp), 0);
}

//上行命令
//主机状态查询
static void up_0x11(void) {
    u8 temp;

    up_packeg((0x110000), (&temp), 0);
}

//上行命令
//主机参数初始化
static void up_0x15(void) {
    u8 temp;

    up_packeg((0x150000), (&temp), 0);
}

//上行命令
//传感器系数设置
static void up_0x19(void) {
    u8 temp[10];
    u16 i;
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        i = data_comm_hex((recorder_para.ratio + 2), recorder_para.ratio[0]);

        *(temp + 0) = i / 0x100;
        *(temp + 1) = i % 0x100;

        n = 0x02;
    }

    up_packeg((0x190000), (temp), n);
}

//上行命令
//车辆VIN设置
static void up_0x1c(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        n = (*((u16 *)(recorder_para.car_vin)));
    }

    up_packeg((0x1c0000), (recorder_para.car_vin + 2), n);
}

//上行命令
//车牌号码设置
static void up_0x1d(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        n = (*((u16 *)(recorder_para.car_plate)));
    }

    up_packeg((0x1d0000), (recorder_para.car_plate + 2), n);
}

//上行命令
//驾驶员代码设置
static void up_0x1f(void) {
    //u8  n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        //	n = 0x00;
    } else {
        //	n = ic_card.work_num_lgth;
    }

    //up_packeg((0x1f0000), (ic_card.work_num), n);
}

//上行命令
//驾驶证号码设置
static void up_0x20(void) {
    //u8  n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        //	n = 0x00;
    } else {
        //	n = ic_card.driv_lic_lgth;
    }

    //up_packeg((0x200000), (ic_card.driv_lic), n);
}

//上行命令
//速度方式选择
static void up_0x21(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) { //查询模式
        n = 0;
    } else {
        n = 1;
    }

    up_packeg((0x00210000), (&(slv_send_flag.status)), n);
}

//上行命令
//启动打印
static void up_0x22(void) {
    u8 temp;

    up_packeg((0x220000), (&temp), 0);
}

//上行命令
//自定义APN设置
static void up_0x24(void) {
    u8 temp[100];

    *(temp + 0) = 0x01;
    _memcpy((temp + 1), (set_para.apn + 2), (*((u16 *)(set_para.apn))));
    up_packeg((0x240000), temp, (*((u16 *)(set_para.apn)) + 1));
}

//上行命令
//扩展多路串口usart0:串口设置
static void up_0x25(void) {
    u8 len;
    u8 tmp_buff[10];
    len = _strlen((u8 *)(&mult_usart_set));

    tmp_buff[0] = mult_usart_set.num;
    tmp_buff[1] = mult_usart_set.uart_num;

    tmp_buff[2] = (mult_usart_set.mult_choose >> 24) & 0xff;
    tmp_buff[3] = (mult_usart_set.mult_choose >> 16) & 0xff;
    tmp_buff[4] = (mult_usart_set.mult_choose >> 8) & 0xff;
    tmp_buff[5] = (mult_usart_set.mult_choose) & 0xff;

    tmp_buff[6] = (mult_usart_set.only_choose >> 24) & 0xff;
    tmp_buff[7] = (mult_usart_set.only_choose >> 16) & 0xff;
    tmp_buff[8] = (mult_usart_set.only_choose >> 8) & 0xff;
    tmp_buff[9] = (mult_usart_set.only_choose) & 0xff;

    if (len == 0x00) {
        up_packeg((0x250000), ((u8 *)(&(mult_usart_set))), 0);
    } else {
        up_packeg((0x250000), ((u8 *)(tmp_buff)), 10);
    }
}

//上行命令
//车牌颜色设置
static void up_0x26(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        n = 1;
    }

    up_packeg((0x260000), (&(slv_send_flag.status)), n);
}

//上行命令
//查询/设置制造商ID
static void up_0x27(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        n = *((u16 *)(&(recorder_para.manufacturer_id)));
    }

    up_packeg((0x270000), (recorder_para.manufacturer_id + 2), n);
}

//上行命令
//校准打开
static void up_0x38(void) {
    up_packeg((0x380000), (&(host_no_save_para.set_load_status)), 0);
}

//上行命令
//校准关闭
static void up_0x39(void) {
    up_packeg((0x390000), (&(host_no_save_para.set_load_status)), 0);
}

//上行命令
//设置备用IP地址
static void up_0x41(void) {
    u8 temp[100];
    u8 i, j, n;

    i = 0x00;
    temp[i++] = '<';
    n = (*((u16 *)(set_para.b_ip)) + 2);
    for (j = 0x00; j < n; j++) {
        if (j < 2)
            temp[i++] = set_para.b_ip[j + 2];
        else if (j == 2)
            temp[i++] = '"';
        else if (j < (set_para.b_ip[0] - 4))
            temp[i++] = set_para.b_ip[(j - 1) + 2];
        else if (j == (set_para.b_ip[0] - 4))
            temp[i++] = '"';
        else
            temp[i++] = set_para.b_ip[(j - 2) + 2];
    }
    temp[i++] = '>';

    up_packeg((0x410000), (temp), i);
}

//上行命令
//空重载
static void up_0x45(void) {
    up_packeg((0x450000), (&(host_no_save_para.set_load_status)), 1);
}

//上行命令
//看车打开
static void up_0x47(void) {
    up_packeg((0x470000), (&(host_no_save_para.set_load_status)), 0);
}

//上行命令
//看车关闭
static void up_0x48(void) {
    up_packeg((0x480000), (&(host_no_save_para.set_load_status)), 0);
}

//上行命令
//省域ID设置
static void up_0x60(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        n = *((u16 *)(&(recorder_para.province_id)));
    }

    up_packeg((0x600000), (recorder_para.province_id + 2), n);
}

//上行命令
//市县域ID设置
static void up_0x61(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        n = *((u16 *)(&(recorder_para.city_id)));
    }

    up_packeg((0x610000), (recorder_para.city_id + 2), n);
}

//上行命令
//终端型号设置
static void up_0x62(void) {
    u8 i, n;
    u8 temp[rec_terminal_type_max_lgth];

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        i = *((u16 *)(&(recorder_para.terminal_type)));
        _memcpy(temp, (recorder_para.terminal_type + 2), i);

        n = (rec_terminal_type_max_lgth - i);
        _memset((temp + i), 0x00, n);

        n = rec_terminal_type_max_lgth;
    }

    up_packeg((0x620000), temp, n);
}

//上行命令
//终端ID设置
static void up_0x63(void) {
    u8 i, n;
    u8 temp[rec_terminal_id_max_lgth];

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        i = *((u16 *)(&(recorder_para.terminal_id)));
        _memcpy(temp, (recorder_para.terminal_id + 2), i);

        n = (rec_terminal_id_max_lgth - i);
        _memset((temp + i), 0x00, n);

        n = rec_terminal_id_max_lgth;
    }

    up_packeg((0x630000), temp, n);
}

//上行命令
//车牌号码分类设置
static void up_0x66(void) {
    u8 n = 0;
    u8 *ptr = 0;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        ptr = (u8 *)(menu_1st_1st_4th_dis[slv_send_flag.status - 1]);

        if (slv_send_flag.status <= 9) //去掉位标
        {
            ptr += 2;
        } else {
            ptr += 3;
        }

        n = _strlen((u8 *)ptr);
    }

    up_packeg((0x660000), ptr, n);
}

//上行命令
//打印选择
static void up_0x6a(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //查询模式
    {
        n = 0x00;
    } else {
        n = 1;
    }

    up_packeg((0x6a0000), (&(slv_send_flag.status)), n);
}

//上行命令
//发送朗读信息
static void up_0xee_0x0000(void) {
    if ((*((u16 *)sound_play_buff)) > sound_play_buff_max_lgth)
        (*((u16 *)sound_play_buff)) = sound_play_buff_max_lgth;

    up_packeg((0xee0000), (sound_play_buff + 2), (*((u16 *)sound_play_buff)));
}

//上行命令
//终端注销
static void up_0xee_0x0003(void) {
    u8 temp;

    up_packeg((0xee0003), (&temp), 0);
}

//上行命令
//终端注册
static void up_0xee_0x0100(void) {
    u8 temp;

    up_packeg((0xee0100), (&temp), 0);
}

//上行命令
//终端鉴权
static void up_0xee_0x0102(void) {
    u8 temp;

    up_packeg((0xee0102), (&temp), 0);
}

//上行命令
//事件报告
static void up_0xee_0x0301(void) {
    up_packeg((0xee0301), (&sms_up_center.fix_id), 1);
}

//上行命令
//提问应答
static void up_0xee_0x0302(void) {
    u8 temp[3];

    *(temp + 0) = (sms_up_center.ask_num) / 0x100;
    *(temp + 1) = (sms_up_center.ask_num) % 0x100;
    *(temp + 2) = *((sms_up_center.ask_ans) + (sms_up_center.ask_id_point));

    up_packeg((0xee0302), (temp), 3);
}

//上行命令
//信息点播
static void up_0xee_0x0303(void) {
    u8 temp[2];

    *(temp + 0) = sms_up_center.vod_type;
    *(temp + 1) = sms_up_center.vod_status;

    up_packeg((0xee0303), (temp), 2);
}

//上行命令
//电子运单
static void up_0xee_0x0701(void) {
    up_packeg((0xee0701), ((u8 *)(&gb_bill_ack)), (sizeof(gb_bill_ack)));
}

//上行命令
//驾驶员信息上报
void up_0xee_0x0702(void) {
    u8 temp[200];
    u16 i;

#ifdef jtbb_test_ver //部标测试用
    i = 0x00;
    if (state_data.state.flag.ic_flag) {
        *(temp + (i++)) = 0x01;
        _memcpy((temp + i), gps_data.date, 3);
        _memcpy((temp + i + 3), gps_data.time, 3);
        i += 6;

        *(temp + (i++)) = 0x00;
        _memcpy((temp + i), driver_infor, ((sizeof(driver_infor)) - 1));
        i += ((sizeof(driver_infor)) - 1);
    } else {
        *(temp + (i++)) = 0x02;
        _memcpy((temp + i), gps_data.date, 3);
        _memcpy((temp + i + 3), gps_data.time, 3);
        i += 6;
    }

#else //生产发行用
    i = 0x00;
    if (state_data.state.flag.ic_flag) //ic卡插入
    {
        *(temp + (i++)) = 0x01;
        _memcpy((temp + i), gps_data.date, 3);
        _memcpy((temp + i + 3), gps_data.time, 3);
        i += 6;

        *(temp + (i++)) = 0x00; //读卡成功

        *(temp + (i++)) = (ic_card.name_lgth);
        _memcpy((temp + i), (ic_card.name), (ic_card.name_lgth));
        i += (ic_card.name_lgth);

        _memcpy((temp + i), (ic_card.certificate), (ic_card.certificate_lgth));
        i += (ic_card.certificate_lgth);
        _memset((temp + i), '\x0', (20 - (ic_card.certificate_lgth)));
        i += (20 - (ic_card.certificate_lgth));

        *(temp + (i++)) = (ic_card.license_organ_lgth);
        _memcpy((temp + i), (ic_card.license_organ), (ic_card.license_organ_lgth));
        i += (ic_card.license_organ_lgth);

        _memcpy((temp + i), (ic_card.license_organ_validity), 4);
        i += 4;
    } else {
        *(temp + (i++)) = 0x02;
        _memcpy((temp + i), gps_data.date, 3);
        _memcpy((temp + i + 3), gps_data.time, 3);
        i += 6;

        *(temp + (i++)) = 0x00; //读卡成功

        *(temp + (i++)) = (ic_card.name_lgth);
        _memcpy((temp + i), (ic_card.name), (ic_card.name_lgth));
        i += (ic_card.name_lgth);

        _memcpy((temp + i), (ic_card.certificate), (ic_card.certificate_lgth));
        i += (ic_card.certificate_lgth);
        _memset((temp + i), '\x0', (20 - (ic_card.certificate_lgth)));
        i += (20 - (ic_card.certificate_lgth));

        *(temp + (i++)) = (ic_card.license_organ_lgth);
        _memcpy((temp + i), (ic_card.license_organ), (ic_card.license_organ_lgth));
        i += (ic_card.license_organ_lgth);

        _memcpy((temp + i), (ic_card.license_organ_validity), 4);
        i += 4;
    }
#endif

    up_packeg((0xee0702), (temp), i);
}

//上行命令散转表(建在FLASH区)
const slave_rev_handset SLAVE_SEND_TAB[] =
    {
        {0x00010000, up_0x01},
        {0x00020000, up_0x02},
        {0x00030000, up_0x03},
        {0x00040000, up_0x04},
        {0x00050000, up_0x05},
        {0x00070000, up_0x07},
        {0x00080000, up_0x08},
        {0x00090000, up_0x09},
        {0x000c0000, up_0x0c},
        {0x000d0000, up_0x0d},
        {0x000e0000, up_0x0e},
        {0x00110000, up_0x11},
        {0x00150000, up_0x15},
        {0x00190000, up_0x19},
        {0x001c0000, up_0x1c},
        {0x001d0000, up_0x1d},
        {0x001f0000, up_0x1f},
        {0x00200000, up_0x20},
        {0x00210000, up_0x21},
        {0x00220000, up_0x22},
        {0x00240000, up_0x24},
        {0x00250000, up_0x25},
        {0x00260000, up_0x26},
        {0x00270000, up_0x27},
        {0x00380000, up_0x38},
        {0x00390000, up_0x39},

        {0x00410000, up_0x41},
        {0x00450000, up_0x45},

        {0x00470000, up_0x47},
        {0x00480000, up_0x48},

        {0x00600000, up_0x60},
        {0x00610000, up_0x61},
        {0x00620000, up_0x62},
        {0x00630000, up_0x63},
        {0x00660000, up_0x66},
        {0x006a0000, up_0x6a},
        {0x00ee0000, up_0xee_0x0000},
        {0x00ee0003, up_0xee_0x0003},
        {0x00ee0100, up_0xee_0x0100},
        {0x00ee0102, up_0xee_0x0102},
        {0x00ee0301, up_0xee_0x0301},
        {0x00ee0302, up_0xee_0x0302},
        {0x00ee0303, up_0xee_0x0303},
        {0x00ee0701, up_0xee_0x0701},
        {0x00ee0702, up_0xee_0x0702},
        {NULL, NULL}};

/*********************************************************
名    称：slave_send_msg
功    能：从机发送数据
输入参数：无
输    出：无
**********************************************************/
//COMT:手柄协议解析
void slave_send_msg(u32 comm, u16 sla_comm, u8 sta, bool ack_flag, u8 que_max) {
    u8 i;

    _memset((u8 *)&slv_send_data.buf, 0x00, HANDSET_MAX_SIZE);
    _memset((u8 *)&slv_send_flag, 0x00, sizeof(slv_send_flag));
    slv_send_flag.comm = comm;
    slv_send_flag.sla_comm = sla_comm;
    slv_send_flag.status = sta;

    for (i = 0; SLAVE_SEND_TAB[i].func != NULL; i++) {
        if (comm == SLAVE_SEND_TAB[i].index) {
            (*SLAVE_SEND_TAB[i].func)();
            return;
        }
    }
    return;
}

/*********************************************************
名    称：slave_send_data_proc
功    能：处理从机数据的发送，考虑到嵌套函数级数过多
，采用数据共享方式传递数据
输入参数：无
输    出：无
**********************************************************/
void slave_send_data_proc(void) {
    u8 ret1 = false;
    u8 xvalue;
    u16 i;

    if (slv_send_data.len == 0){
        return;
    }

    if (slv_send_data.len < 5 || slv_send_data.len > HANDSET_MAX_SIZE) {
        goto slv_send_data_err;
    }

    xvalue = 0;
    for (i = 0; i < slv_send_data.len - 2; i++)
        xvalue ^= slv_send_data.buf[i];

    if (slv_send_data.buf[slv_send_data.len - 2] == xvalue) {
        if (slv_send_data.buf[1] != 0x0d) //发送正确接收数据的应答指令到手柄
        {
            //send_handset_data(FROM_INNER, H_LEN_N, 0, (u8*)&HAnsData, 0);
            ret1 = handset_any_parse(FROM_INNER, (u8 *)&slv_send_data.buf, slv_send_data.len);
            if (ret1 == CMD_ERROR) {
                loge("handset err: inner parse");
            }
        }
    } else {
        loge("SLV XOR ERR");
    }

slv_send_data_err:
    _memset((u8 *)&slv_send_data, 0x00, sizeof(slv_send_data));
}

//******************************************************************************************************//
//******************************************************************************************************//
