//*****************************************************************************//
//*****************************************************************************//
//
//  CKP  2015-07-15  优化底层驱动
//
//*****************************************************************************//
//*****************************************************************************//
#include "string.h"
#include "ckp_mcu_file.h"
#include "print_driver.h"

static moto_driv_para_struct moto_driv_para;   //电机驱动参数
static print_task_para_struct print_task_para; //打印任务参数
static pt487fb_driver_need_struct pt487fb_driver;

static uchar print_speed; //打印机打印速度

//*****************************************************//
//*************      关闭电机函数         *************//
//
//*****************************************************//
static void print_moto_stop(void) {
    pt487fb_driver.prt_ma_set(_false_);
    pt487fb_driver.prt_mb_set(_false_);
    pt487fb_driver.prt_mna_set(_false_);
    pt487fb_driver.prt_mnb_set(_false_);
}

//*****************************************************//
//*************        参数初始化         *************//
//
//*****************************************************//
void print_data_init(pt487fb_driver_need_struct src) {
    pt487fb_driver = src;
}

//*****************************************************//
//*************        驱动初始化         *************//
//参数SP为打印速度设置    取值范围(50-1200)，最好不要用临界值
//*****************************************************//
void print_driver_init(uint sp) {
    if (sp < 50) {
        sp = 50;
    } else if (sp > 1200) {
        sp = 1200;
    }

    print_speed = (5000 / sp);

    pt487fb_driver.prt_lat_set(_true_);
    pt487fb_driver.prt_clk_set(_true_);

    memset(((uchar *)(&moto_driv_para)), 0x00, (sizeof(moto_driv_para)));
    memset(((uchar *)(&print_task_para)), 0x00, (sizeof(print_task_para)));

    print_moto_stop();
    pt487fb_driver.prt_heat_set(_false_);
}

//*****************************************************//
//*************        读取打印状态       *************//
//函数返回值      TRUE表示忙状态      FALSE表示空闲状态
//若打印机正在忙，则不会响应其他命令
//*****************************************************//
bit_enum print_read_state(void) {
    if ((print_task_para.cnt == 0) && (print_task_para.paper_move_flag == _false_))
        return _false_; //空闲状态
    else
        return _true_; //忙状态
}

//*****************************************************//
//*************        加载点阵数据       *************//
//参数cnt为横向位置，范围为(0-15)
//
//*****************************************************//
#ifdef print_pixel_amp_en

static void print_latch_data(uchar cnt) {
    uchar i, j;

    if (cnt >= 16)
        return;

    pt487fb_driver.prt_lat_set(_true_);

    for (i = 0x00; i < 24; i++) {
        for (j = 0x00; j < 16; j++) {
            pt487fb_driver.prt_clk_set(_false_);

            if ((print_task_para.pixel_buff[cnt][i]) & bit_move((7 - (j / 2))))
                pt487fb_driver.prt_di_set(_true_);
            else
                pt487fb_driver.prt_di_set(_false_);

            pt487fb_driver.prt_clk_set(_true_);
        }
    }

    pt487fb_driver.prt_lat_set(_false_); //锁存信号
}

#else

static void print_latch_data(uchar cnt) {
    uchar i, j;

    if (cnt >= 16)
        return;

    pt487fb_driver.prt_lat_set(_true_);

    for (i = 0x00; i < 48; i++) {
        for (j = 0x00; j < 8; j++) {
            pt487fb_driver.prt_clk_set(_false_);

            if ((print_task_para.pixel_buff[cnt][i]) & bit_move(7 - j))
                pt487fb_driver.prt_di_set(_true_);
            else
                pt487fb_driver.prt_di_set(_false_);

            pt487fb_driver.prt_clk_set(_true_);
        }
    }

    pt487fb_driver.prt_lat_set(_false_); //锁存信号
}

#endif

//*****************************************************//
//*************     打印机字库点阵转换    *************//
//参数code_buff即为输入也为输出缓冲区
//参数flag为编码类型    TRUE为国标码类型  FALSE为ASCII码类型
//----//
//输入字库点阵类型:纵向取模，字节倒序(单字节高位在下，整体从左往右，从上往下)
//输出字库点阵类型:横向取模，字节顺序(单字节高位在左，整体从左往右，从上往下)
//----//
//*****************************************************//
static void print_font_convert(uchar *code_buff, bit_enum flag) {
    uchar buff[32];
    uchar i, j;

    memset(buff, 0x00, sizeof(buff)); //清零临时缓冲区

    if (flag == _false_) //ASCII码格式
    {
        for (i = 0x00; i < 8; i++) {
            for (j = 0x00; j < 8; j++) {
                if (code_buff[j] & bit_move(i)) {
                    buff[i] |= bit_move(7 - j);
                }
            }
        }

        for (i = 0x00; i < 8; i++) {
            for (j = 0x00; j < 8; j++) {
                if (code_buff[j + 8] & bit_move(i)) {
                    buff[i + 8] |= bit_move(7 - j);
                }
            }
        }

        memcpy(code_buff, buff, 16);
    }

    else //国标码格式
    {
        for (i = 0x00; i < 8; i++) {
            for (j = 0x00; j < 8; j++) {
                if (code_buff[j] & bit_move(i)) {
                    buff[2 * i] |= bit_move(7 - j);
                }
            }
        }

        for (i = 0x00; i < 8; i++) {
            for (j = 0x00; j < 8; j++) {
                if (code_buff[j + 8] & bit_move(i)) {
                    buff[2 * i + 1] |= bit_move(7 - j);
                }
            }
        }

        for (i = 0x00; i < 8; i++) {
            for (j = 0x00; j < 8; j++) {
                if (code_buff[j + 16] & bit_move(i)) {
                    buff[2 * i + 16] |= bit_move(7 - j);
                }
            }
        }

        for (i = 0x00; i < 8; i++) {
            for (j = 0x00; j < 8; j++) {
                if (code_buff[j + 24] & bit_move(i)) {
                    buff[2 * i + 17] |= bit_move(7 - j);
                }
            }
        }

        memcpy(code_buff, buff, 32);
    }
}

//*****************************************************//
//*************      配置电机函数         *************//
//参数m_cycle为本次驱动电机运行的周期数
//参数m_speed为单步长度，一个精度单位
//
//*****************************************************//
static void print_moto_config(ulong m_cycle, uint m_speed) {
    moto_driv_para.step_cycle = m_cycle;
    moto_driv_para.step_tim = m_speed;

    moto_driv_para.cnt = 0x00;
}

//*****************************************************//
//*************      电机驱动函数         *************//
//函数返回值     TRUE表示运行正常    FALSE表示该次运行周期结束
//
//*****************************************************//
static bit_enum print_moto_driver(void) {
    bit_enum ret;

    if (moto_driv_para.step_cycle == 0x00) //运行结束
    {
        return _false_;
    }

    moto_driv_para.cnt++;
    if (moto_driv_para.cnt < moto_driv_para.step_tim) //满足电机转动一步
    {
        return _true_;
    }

    moto_driv_para.cnt = 0x00;
    ret = _true_;

    switch (moto_driv_para.step) {
    case 0x00: //第一步
        pt487fb_driver.prt_ma_set(_false_);
        pt487fb_driver.prt_mna_set(_true_);
        pt487fb_driver.prt_mb_set(_false_);
        pt487fb_driver.prt_mnb_set(_true_);

        moto_driv_para.step = 1;
        moto_driv_para.step_cycle--;
        break;

    case 0x01: //第二步
        pt487fb_driver.prt_ma_set(_false_);
        pt487fb_driver.prt_mna_set(_true_);
        pt487fb_driver.prt_mb_set(_true_);
        pt487fb_driver.prt_mnb_set(_false_);

        moto_driv_para.step = 2;
        moto_driv_para.step_cycle--;
        break;

    case 0x02: //第三步
        pt487fb_driver.prt_ma_set(_true_);
        pt487fb_driver.prt_mna_set(_false_);
        pt487fb_driver.prt_mb_set(_true_);
        pt487fb_driver.prt_mnb_set(_false_);

        moto_driv_para.step = 3;
        moto_driv_para.step_cycle--;
        break;

    case 0x03: //第四步
        pt487fb_driver.prt_ma_set(_true_);
        pt487fb_driver.prt_mna_set(_false_);
        pt487fb_driver.prt_mb_set(_false_);
        pt487fb_driver.prt_mnb_set(_true_);

        moto_driv_para.step = 0;
        moto_driv_para.step_cycle--;
        break;

    default:
        moto_driv_para.step = 0x00;
        ret = _false_;
        moto_driv_para.step_cycle = 0x00;
        print_moto_stop();
    }

    return ret;
}

//*****************************************************//
//*************     打印任务调度处理      *************//
//开始打印时，调用该函数，该函数的调用频率为5KHz，周期为200微秒
//用定时器中断来做，精确控制步进电机
//
//函数返回值    TRUE表示正在运行    FALSE表示该次运行周期结束
//*****************************************************//
bit_enum print_base_task_hdl(void) {
    bit_enum ret;

    ret = _true_;

    if (print_task_para.paper_move_flag) //走纸任务优先处理
    {
        if (!print_moto_driver()) {
            print_task_para.paper_move_flag = _false_; //走纸任务处理完毕
            ret = _false_;
        }
    }

    else if (!print_moto_driver()) {
        pt487fb_driver.prt_heat_set(_false_); //停止加热

        if (print_task_para.cnt == 0x00) {
            ret = _false_;
        } else {
            print_latch_data(16 - print_task_para.cnt);
            print_moto_config(moto_print_step_cyc_volue, print_speed);

            pt487fb_driver.prt_heat_set(_true_); //开始加热
            print_task_para.cnt--;
        }
    }

    return ret;
}

//*****************************************************//
//*************       配置打印机走纸      *************//
//函数返回值返回设置走纸任务是否成功  TRUE设置成成功  FALSE设置失败(忙状态造成)
//*****************************************************//
bit_enum print_move_paper_config(void) //没有使用到
{
    if (print_read_state()) //忙状态，则设置走纸任务无效
    {
        return _false_;
    }

    print_moto_config(moto_noload_step_cyc_volue, print_speed);
    print_task_para.paper_move_flag = _true_; //启动走纸任务

    return _true_;
}

//*****************************************************//
//*******   计算编码缓冲区中国标码的字节个数    *******//
//参数src为待处理的缓冲区
//参数cnt为待处理的数据长度
//函数返回个数
//*****************************************************//
static uchar data_cnt_gb(uchar *src, uchar cnt) {
    uchar ret;

    ret = 0x00;

    while (cnt) {
        if (*src > 0xa0) {
            ret += 1;
        }

        cnt--;
        src++;
    }

    return ret;
}

//*****************************************************//
//*************        汉字点阵交换       *************//
//参数x为横向坐标，取值范围为0-47
//参数n为一次加载的列数，汉字为两列，ASCII为一列，所以n的取值范围为1或者2
//参数s_buff为源数据缓冲器
//参数d_buff为目标数据缓冲器
//*****************************************************//
static void pixel_swap(uchar x, uchar n, uchar (*d_buff)[48], uchar *s_buff) {
    uchar i;

    for (i = 0x00; i < 16; i++) {
        if (n == 2) {
            d_buff[i][x] = s_buff[2 * i];
            d_buff[i][x + 1] = s_buff[(2 * i) + 1];
        } else {
            d_buff[i][x] = s_buff[i];
        }
    }
}

//*****************************************************//
//*************        打印一行字符       *************//
//参数x为行打印的起始地址，取值范围为0-47，总打印的范围单行为48个字符或24个汉字
//参数lgth为待打印字符计数
//参数s_buff为待打印的字符编码
//参数cnt返回还剩下待打印的字节个数
//函数返回值返回本次操作命令是否被执行  TRUE成功  FALSE失败(忙状态造成)
//
//注意:本行实际打印的字节数可以是因为回车换行符或者是超过了打印范围
//注意:换行必须同时出现回车换行符，否则视为字符处理
//*****************************************************//
bit_enum print_one_line(uchar x, uint lgth, uchar *s_buff, uint *cnt) {
    uchar curr, f;
    uchar i, j, n;
    uchar temp_buff[32];
    uint word_addr;

    if (print_read_state()) //忙状态，则设置打印任务无效
    {
        return _false_;
    }

    if ((x > (print_line_range_max - 1)) || ((x == (print_line_range_max - 1)) && (*s_buff >= 0xa1))) //起始地址违规操作，不打印
    {
        *cnt = lgth;
    }

    if ((x + lgth) > print_line_range_max) //修正参数值，保留有效打印的区域
    {
        curr = (print_line_range_max - x);

        if (data_cnt_gb(s_buff, curr) % 2) //杜绝国标码显示一半的情况，同时也杜绝数组越界
        {
            curr -= 1;
        }
    } else {
        curr = lgth;
    }

    f = 0x00;
    for (i = 0x00; i < curr; i++) {
        if ((s_buff[i] == '\x0d') && (s_buff[i + 1] == '\x0a')) {
            f = 0x01; //换行标志
            break;
        }
    }

    curr = i; //重新赋值本次打印的实际有效字节数

    for (i = 0x00; i < x; i++) //清零未打印的区域前
    {
        for (j = 0x00; j < 16; j++) {
            print_task_para.pixel_buff[j][i] = 0x00;
        }
    }

    n = (print_line_range_max - (x + curr)); //清零未打印的区域后
    for (i = 0x00; i < n; i++) {
        for (j = 0x00; j < 16; j++) {
            print_task_para.pixel_buff[j][i + x + curr] = 0x00;
        }
    }

    i = 0x00;
    while (i < curr) {
        if (*(s_buff + i) >= 0xa1) {
            word_addr = (*(s_buff + i)) * 0x100 + (*(s_buff + i + 1));
            pt487fb_driver.prt_font(word_addr, temp_buff); //国标码
            print_font_convert(temp_buff, _true_);
            pixel_swap((x + i), 2, print_task_para.pixel_buff, temp_buff); //交换点阵信息

            i += 2; //修改显示指针
        } else {
            word_addr = *(s_buff + i);
            pt487fb_driver.prt_font(word_addr, temp_buff); //ASCII码
            print_font_convert(temp_buff, _false_);
            pixel_swap((x + i), 1, print_task_para.pixel_buff, temp_buff); //交换点阵信息

            i += 1; //修改显示指针
        }
    }

    print_task_para.cnt = print_task_cnt_cycle; //启动打印

    if (f) //换行符，要加2
    {
        curr += 2;
    }

    *cnt = (lgth - curr);

    return _true_;
}
