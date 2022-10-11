//*****************************************************************************//
//*****************************************************************************//
//在使用本文件时，将会占用1K左右的RAM空间
//
//  CKP  2015-04-24
//
//  CKP  2015-05-07  通过增加设置显示起始行，实现整屏上下滚动显示功能
//
//  CKP  2015-07-15  增加显示自定义范围的点阵功能
//*****************************************************************************//
//*****************************************************************************//
#include "string.h"
#include "ckp_mcu_file.h"
#include "lcd13264_ram.h"
#include "include_all.h"

//*****************************************************************************
//*****************************************************************************
//-----------             LCD13264液晶屏控制命令              -----------------
//-----------------------------------------------------------------------------

#define lcd_comm_nop ((uchar)0xe3)      //空命令
#define lcd_comm_bias ((uchar)0xa2)     //设置偏置  1/9
#define lcd_comm_power_on ((uchar)0x2f) //打开内置电源

#define lcd_comm_display_on ((uchar)0xaf)  //开显示命令
#define lcd_comm_display_off ((uchar)0xae) //关显示命令

#define lcd_comm_start_line(n) (((uchar)0x40) + ((n) % 64))   //设置显示的起始行 范围(0-63)
#define lcd_comm_set_page(n) (((uchar)0xb0) + ((n) % 8))      //设置显示页   范围(0-7)
#define lcd_comm_set_column_H(n) (((uchar)0x10) + ((n) % 16)) //设置显示列高地址   范围(0-15)
#define lcd_comm_set_column_L(n) ((n) % 16)                   //设置显示列低地址   范围(0-15)

#define lcd_comm_inverse_display_on ((uchar)0xa7)  //像素点反色显示开
#define lcd_comm_inverse_display_off ((uchar)0xa6) //像素点反色显示关
#define lcd_comm_reverse_seg_on ((uchar)0xa1)      //段码反向排序使能
#define lcd_comm_reverse_seg_off ((uchar)0xa0)     //段码反向排序关闭
#define lcd_comm_reverse_com_on ((uchar)0xc8)      //公共端反向排序使能
#define lcd_comm_reverse_com_off ((uchar)0xc0)     //公共端反向排序关闭
#define lcd_comm_all_pixel_on ((uchar)0xa5)        //所有像素点开
#define lcd_comm_all_pixel_off ((uchar)0xa4)       //恢复一般显示

static bit_enum lcd_refresh; //屏幕刷新标志    TRUE表示有更新，FALSE表示无更新

static uchar lcd_buff[lcd_max_x_page][lcd_max_y_addr]; //液晶显示缓冲区
//每行132个像素点
//纵向每8个像素点为一个字节也称为页，纵向共8页

static lcd13264_ram_need_struct lcd13264_ram_dat; //外部提供的函数

//初始化
void lcd13264_data_init(lcd13264_ram_need_struct src) {
    lcd13264_ram_dat = src;
}

//*****************************************************//
//*******      向液晶驱动写入一个字节数据       *******//
//参数comm_ram    TRUE表示控制命令     FALSE表示显示数据
//参数dat         为待写入的数据
//*****************************************************//

//static

void lcd_write_byte(bit_enum comm_ram, uchar dat) {
    uchar i;

    lcd13264_ram_dat.lcd_cs_set(_false_);

    if (comm_ram) //发送命令
        lcd13264_ram_dat.lcd_a0_set(_false_);
    else
        lcd13264_ram_dat.lcd_a0_set(_true_); //发送数据

    for (i = 0x00; i < 8; i++) {
        lcd13264_ram_dat.lcd_scl_set(_false_);

        if (dat & 0x80)
            lcd13264_ram_dat.lcd_io_set(_true_);
        else
            lcd13264_ram_dat.lcd_io_set(_false_);

        dat <<= 1;

        lcd13264_ram_dat.lcd_scl_set(_true_);
    }

    lcd13264_ram_dat.lcd_cs_set(_true_); //操作完毕
}

//*****************************************************//
//*******           液晶屏初始化函数            *******//
//参数typ为初始化方式    FALSE为老屏  TRUE为新屏
//*****************************************************//
void lcd_init(bit_enum typ, bit_enum ic) {
    lcd13264_ram_dat.lcd_cs_set(_true_);  //禁止片选以确保下次通信的正常进行
    lcd13264_ram_dat.lcd_scl_set(_true_); //时钟设置为高电平状态
    lcd13264_ram_dat.lcd_rst_set(_true_); //释放复位
    lcd13264_ram_dat.lcd_delayms(20);     //延时20毫秒，准备复位

    lcd13264_ram_dat.lcd_rst_set(_false_);
    lcd13264_ram_dat.lcd_delayms(2);
    lcd13264_ram_dat.lcd_rst_set(_true_);
    lcd13264_ram_dat.lcd_delayms(10); //延时10毫秒完成LCD复位

    //-----------------------------
    lcd_write_byte(_true_, lcd_comm_nop);                 //空命令
    lcd_write_byte(_true_, lcd_comm_bias);                //设置偏置
    lcd_write_byte(_true_, lcd_comm_inverse_display_off); //设置像素点正反显示
    //-----------------------------

    lcd_write_byte(_true_, lcd_comm_reverse_seg_off);  //从左至右
    lcd_write_byte(_true_, lcd_comm_reverse_com_on);   //从上至下
    lcd_write_byte(_true_, lcd_comm_start_line(0x20)); //设置显示起始行

    //-----------------------------
    EN_LCD_BL_PWR;

    if (typ) {                        //新屏
        lcd_write_byte(_true_, 0x27); //设置分压电阻  (RR)    0x27
        lcd_write_byte(_true_, 0x81);
        //
        if (set_para.LCD_Contrast == 0) {
            lcd_write_byte(_true_, 0x28); //设置电压  (EV)
        } else {
            lcd_write_byte(_true_, set_para.LCD_Contrast); //设置电压  (EV)
        }
    } else {                          //旧屏
        lcd_write_byte(_true_, 0x24); //设置分压电阻(设置调节电阻率)
        lcd_write_byte(_true_, 0x81);
        lcd_write_byte(_true_, 0x2d); //设置电压
    }

    lcd_write_byte(_true_, lcd_comm_power_on);   //打开内置电源
    lcd_write_byte(_true_, lcd_comm_display_on); //开显示

    if (ic) {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //白屏显示  /* ?????????????????   */
    }
}

//*****************************************************//
//*******            设置显示起始行             *******//
//参数s为行数编号，范围为0-63
//*****************************************************//
void lcd_set_start_line(uchar s) {
    lcd_write_byte(_true_, lcd_comm_start_line(0x20 + s)); //设置显示起始行
}

//*****************************************************//
//*******    向特定显示缓冲区写入一个特定的值   *******//
//参数x_st为纵向起始地址，范围为0-7
//参数x_end为纵向终止地址
//参数y_st为横向起始地址，范围为0-131
//参数y_end为横向终止地址
//参数dat         为待写入的数据
//*****************************************************//
void lcd_area_dis_set(uchar x_st, uchar x_end, uchar y_st, uchar y_end, uchar dat) {
    uchar x, y;

    if ((x_st >= lcd_max_x_page) || (y_st >= lcd_max_y_addr)) //起始地址超过范围
    {
        return;
    }

    if (x_end >= lcd_max_x_page) //修正终止地址的错误
    {
        x_end = (lcd_max_x_page - 1);
    }

    if (y_st >= lcd_max_y_addr) //修正终止地址的错误
    {
        y_end = (lcd_max_y_addr - 1);
    }

    for (x = x_st; x <= x_end; x++) {
        for (y = y_st; y <= y_end; y++) {
            lcd_buff[x][y] = dat;
        }
    }

    lcd_refresh = _true_; //有刷屏请求
}

//*****************************************************//
//*******                LCD整屏显示            *******//
//*****************************************************//
void lcd_dis_all(void) {
    uchar x, y;

    if (print_auto_data.req_flag)
        return;

    if (lcd_refresh == _false_) //无刷屏请求
    {
        return;
    }

    for (x = 0x00; x < lcd_max_x_page; x++) {
        lcd_write_byte(_true_, lcd_comm_set_page(x));

        lcd_write_byte(_true_, lcd_comm_set_column_H(0));
        lcd_write_byte(_true_, lcd_comm_set_column_L(0));

        for (y = 0x00; y < lcd_max_y_addr; y++) {
            lcd_write_byte(_false_, lcd_buff[x][y]);
        }
    }

    lcd_refresh = _false_;
}

//*****************************************************//
//*******         LCD立即显示一屏数据           *******//
//*****************************************************//
void lcd_dis_at_once(void) {
    lcd_refresh = _true_;
    lcd_dis_all();
}

//*****************************************************************************************
//该函数的功能是显示一个完整的字符，包含16*16,8*16,16*8,8*8四种大小
//
//参数x_page指待显示字符的纵向位置，范围为0-7页，此参数与LCD屏数据手册中的页相对应
//参数y_addr指待显示字符的横向位置，范围为0-131，此参数与LCD屏数据手册中的Y地址相对应
//参数*dis_buff指待显示的缓冲器
//参数dis_type指待显示字符的大小，  0表示8*16ASCII   1表示16*16汉字  2表示16*8图标   3表示8*8图标
//参数flag指待显示字符的点阵显示方式,    FALSE表示正常显示
//                         TRUE表示把点阵信息取反后再显示，这一点主要应用在选中菜单上
//*****************************************************************************************
void lcd_dis_one_word(uchar x_page, uchar y_addr, const uchar *dis_buff, lcd_dis_type_enum dis_type, bool flg_reverse) {
    uchar x, y;
    uchar ix, iy;

    if ((x_page >= lcd_max_x_page) || (y_addr >= lcd_max_y_addr)) { //超过显示范围，直接退出
        return;
    }

    if (dis_type == lcd_dis_type_8x16ascii) {
        y = 8;
        x = 2;
    } else if (dis_type == lcd_dis_type_16x16chn) {
        y = 16;
        x = 2;
    } else if (dis_type == lcd_dis_type_8x16icon) {
        y = 16;
        x = 1;
    } else {
        y = 8;
        x = 1;
    }

    if ((x_page + x) > lcd_max_x_page) {
        x = (lcd_max_x_page - x_page);
    }

    if ((y_addr + y) > lcd_max_y_addr) {
        y = (lcd_max_y_addr - y_addr);
    }

    for (ix = 0x00; ix < x; ix++) {
        for (iy = 0x00; iy < y; iy++) { //发送待显示的数据
            if (flg_reverse) {          //反亮显示
                lcd_buff[x_page + ix][y_addr + iy] = (~(*(dis_buff++)));
            } else { //正常显示
                lcd_buff[x_page + ix][y_addr + iy] = (*(dis_buff++));
            }
        }
    }

    lcd_refresh = _true_; //有刷屏请求
}

//*****************************************************************************************
//该函数的功能是显示一个完整的字符，包含16*16,8*16,16*8,8*8四种大小
//
//参数x_page指待显示字符的纵向位置，范围为0-7页，此参数与LCD屏数据手册中的页相对应
//参数y_addr指待显示字符的横向位置，范围为0-131，此参数与LCD屏数据手册中的Y地址相对应
//参数*dis_buff指待显示的缓冲器
//参数dis_type指待显示字符的大小，  0表示8*16ASCII   1表示16*16汉字  2表示16*8图标   3表示8*8图标
//参数flag指待显示字符的点阵显示方式,    FALSE表示正常显示
//                         TRUE表示把点阵信息取反后再显示，这一点主要应用在选中菜单上
//*****************************************************************************************
void lcd_dis_num_val_8x8(uchar x_page, uchar y_addr, bool flg_reverse, u_16 val) {
    extern const u8 num88_dis[12][8];

    char arr[24];
    sprintf(arr, "%d", val);
    int len = strlen(arr);
    int i = 0;
    for (; i < len; i++) {
        lcd_dis_one_word(x_page, y_addr + 6 * i, num88_dis[arr[i] - '0'], lcd_dis_type_8x8icon, flg_reverse);
    }
}
//********************************************************************//
//********************************************************************//

//*****************************************************************************************
//该函数的功能是显示一个自定义范围的图片
//
//参数x_page指待显示字符的纵向位置，范围为0-7页，此参数与LCD屏数据手册中的页相对应
//参数y_addr指待显示字符的横向位置，范围为0-131，此参数与LCD屏数据手册中的Y地址相对应
//参数*dis_buff指待显示的缓冲器
//参数x_cnt指待显示图片页数大小，单位为页，注意不要超过显示范围
//参数y_cnt指待显示图片横向范围，单位为一个像素，注意不要超过显示范围
//*****************************************************************************************
void lcd_dis_custom(uchar x_page, uchar y_addr, const uchar *dis_buff, uchar x_cnt, uchar y_cnt) {
    uchar ix, iy;

    if ((x_page >= lcd_max_x_page) || (y_addr >= lcd_max_y_addr)) //超过显示范围，直接退出
    {
        return;
    }

    if ((x_page + x_cnt) > lcd_max_x_page) {
        x_cnt = (lcd_max_x_page - x_page);
    }

    if ((y_addr + y_cnt) > lcd_max_y_addr) {
        y_cnt = (lcd_max_y_addr - y_addr);
    }

    for (ix = 0x00; ix < x_cnt; ix++) {
        for (iy = 0x00; iy < y_cnt; iy++) {
            lcd_buff[x_page + ix][y_addr + iy] = (*(dis_buff++));
        }
    }

    lcd_refresh = _true_; //有刷屏请求
}

//*****************************************************************************************
//该函数的功能是显示最多一行字符  高度为两个显示页
//该函数自动清除从设定的显示位置开始到行结束区域中的像素点
//
//参数x_posi指待显示字符的纵向位置，范围为0-3
//参数y_addr指待显示字符的横向位置，范围为0-131，此参数与LCD屏数据手册中的Y地址相对应
//参数*dis_buff指待显示的缓冲器，最后一个字节必须以0x00结尾
//参数flag指待显示字符的点阵显示方式,    FALSE表示正常显示
//                         TRUE表示把点阵信息取反后再显示，这一点主要应用在选中菜单上
//*****************************************************************************************
void lcd_dis_one_line(uchar x_posi, uchar y_addr, const uchar *dis_buff, bool flg_reverse) {
    uint word_addr;
    uchar point;
    uchar temp_buff[32];

    lcd_area_dis_set((x_posi * 2), ((x_posi * 2) + 1), y_addr, (lcd_max_y_addr - 1), 0x00); //显示一行时先清除该行的内容

    point = 0x00;                                                        //先将显示数组指针清零
    while ((*(dis_buff + point) != 0x00) && (y_addr < lcd_max_y_addr)) { //还有待显示的数据
        if (*(dis_buff + point) >= 0xa1) {
            word_addr = (*(dis_buff + point)) * 0x100 + (*(dis_buff + point + 1));
            lcd13264_ram_dat.lcd_font(word_addr, temp_buff); //国标码
            lcd_dis_one_word((x_posi * 2), y_addr, temp_buff, lcd_dis_type_16x16chn, flg_reverse /*0,false 正常显示; 1,true 反显;*/);

            point += 2;
            y_addr += 16; //修改显示指针
        } else {
            word_addr = *(dis_buff + point);
            lcd13264_ram_dat.lcd_font(word_addr, temp_buff); //ASCII码
            lcd_dis_one_word((x_posi * 2), y_addr, temp_buff, lcd_dis_type_8x16ascii, flg_reverse /*0,false 正常显示; 1,true 反显;*/);

            point += 1;
            y_addr += 8; //修改显示指针
        }
    }
}
//********************************************************************//
//********************************************************************//

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

//*****************************************************************************************
//处理显示一页函数中单行数据因数据内容为0引起本行结束显示函数，修改
//
//参数buff为缓冲区
//参数lgth为显示的有效长度
//*****************************************************************************************
static void display_page_line_check(uchar *buff, uchar lgth) {
    uchar i;

    for (i = 0x00; i < lgth; i++) {
        if (buff[i] == 0x00) {
            buff[i] = 0x20; //把非法结束标志改为空格字符
        }
    }
}

//*****************************************************************************************
//显示一页信息
//
//把一整屏数据当成一个整体， 满屏最多显示(8*4)-1=31个汉字或者62个字符
//满屏最少显示15*3+13=58个字符
//显示位置以字符为单位，位置编号范围为0-61
//
//参数start表示起始位置，范围0-61
//参数lgth表示待显示的数据的长度，可以大于单屏显示的最大范围，函数会有返回值，返回没显示完的字节数
//参数buff表示待显示的缓冲区
//函数返回值为形参中待显示长度参数通过本次调用该函数后，还剩下的待显示的字符的个数
//*****************************************************************************************
uint lcd_dis_one_page(unsigned char start, unsigned char len, unsigned char *buf) {
    uchar line, addr;
    uchar n;
    uchar temp[20];

    if (start >= lcd_page_char_max_num) //超过显示范围，将不显示任何数据
        return 0x00;

    line = (start / lcd_line_char_max_num); //计算起始显示的行号
    addr = (start % lcd_line_char_max_num); //计算在该行的起始位置

    do {
        if ((addr + len) > lcd_line_char_max_num)
            n = (lcd_line_char_max_num - addr); //计算本行显示的字符数
        else
            n = len;

        if (line == 3) //最后一行放指向箭头标志
        {
            if (n > (lcd_line_char_max_num - 2)) {
                n = (lcd_line_char_max_num - 2);
            }
        }

        if (data_cnt_gb((buf), n) % 2) //有余数则减一
        {
            n -= 1;
        }

        memcpy(temp, buf, n);
        display_page_line_check(temp, n); //检查数据内容的非法结束标志

        *(temp + n) = '\x0'; //结束标志
        lcd_dis_one_line(line, (addr * 8), temp, false);

        buf += n;  //修改待显示的缓冲区指针
        len -= n;  //修改待显示的长度

        addr = 0x00;
        line++;
    } while ((line < 4) && (len > 0));

    return len; //当前页没显示完的内容个数
}
//********************************************************************//
//********************************************************************//
