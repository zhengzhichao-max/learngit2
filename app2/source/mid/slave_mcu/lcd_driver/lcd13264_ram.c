//*****************************************************************************//
//*****************************************************************************//
//��ʹ�ñ��ļ�ʱ������ռ��1K���ҵ�RAM�ռ�
//
//  CKP  2015-04-24
//
//  CKP  2015-05-07  ͨ������������ʾ��ʼ�У�ʵ���������¹�����ʾ����
//
//  CKP  2015-07-15  ������ʾ�Զ��巶Χ�ĵ�����
//*****************************************************************************//
//*****************************************************************************//
#include "string.h"
#include "ckp_mcu_file.h"
#include "lcd13264_ram.h"
#include "include_all.h"

//*****************************************************************************
//*****************************************************************************
//-----------             LCD13264Һ������������              -----------------
//-----------------------------------------------------------------------------

#define lcd_comm_nop ((uchar)0xe3)      //������
#define lcd_comm_bias ((uchar)0xa2)     //����ƫ��  1/9
#define lcd_comm_power_on ((uchar)0x2f) //�����õ�Դ

#define lcd_comm_display_on ((uchar)0xaf)  //����ʾ����
#define lcd_comm_display_off ((uchar)0xae) //����ʾ����

#define lcd_comm_start_line(n) (((uchar)0x40) + ((n) % 64))   //������ʾ����ʼ�� ��Χ(0-63)
#define lcd_comm_set_page(n) (((uchar)0xb0) + ((n) % 8))      //������ʾҳ   ��Χ(0-7)
#define lcd_comm_set_column_H(n) (((uchar)0x10) + ((n) % 16)) //������ʾ�иߵ�ַ   ��Χ(0-15)
#define lcd_comm_set_column_L(n) ((n) % 16)                   //������ʾ�е͵�ַ   ��Χ(0-15)

#define lcd_comm_inverse_display_on ((uchar)0xa7)  //���ص㷴ɫ��ʾ��
#define lcd_comm_inverse_display_off ((uchar)0xa6) //���ص㷴ɫ��ʾ��
#define lcd_comm_reverse_seg_on ((uchar)0xa1)      //���뷴������ʹ��
#define lcd_comm_reverse_seg_off ((uchar)0xa0)     //���뷴������ر�
#define lcd_comm_reverse_com_on ((uchar)0xc8)      //�����˷�������ʹ��
#define lcd_comm_reverse_com_off ((uchar)0xc0)     //�����˷�������ر�
#define lcd_comm_all_pixel_on ((uchar)0xa5)        //�������ص㿪
#define lcd_comm_all_pixel_off ((uchar)0xa4)       //�ָ�һ����ʾ

static bit_enum lcd_refresh; //��Ļˢ�±�־    TRUE��ʾ�и��£�FALSE��ʾ�޸���

static uchar lcd_buff[lcd_max_x_page][lcd_max_y_addr]; //Һ����ʾ������
//ÿ��132�����ص�
//����ÿ8�����ص�Ϊһ���ֽ�Ҳ��Ϊҳ������8ҳ

static lcd13264_ram_need_struct lcd13264_ram_dat; //�ⲿ�ṩ�ĺ���

//��ʼ��
void lcd13264_data_init(lcd13264_ram_need_struct src) {
    lcd13264_ram_dat = src;
}

//*****************************************************//
//*******      ��Һ������д��һ���ֽ�����       *******//
//����comm_ram    TRUE��ʾ��������     FALSE��ʾ��ʾ����
//����dat         Ϊ��д�������
//*****************************************************//

//static

void lcd_write_byte(bit_enum comm_ram, uchar dat) {
    uchar i;

    lcd13264_ram_dat.lcd_cs_set(_false_);

    if (comm_ram) //��������
        lcd13264_ram_dat.lcd_a0_set(_false_);
    else
        lcd13264_ram_dat.lcd_a0_set(_true_); //��������

    for (i = 0x00; i < 8; i++) {
        lcd13264_ram_dat.lcd_scl_set(_false_);

        if (dat & 0x80)
            lcd13264_ram_dat.lcd_io_set(_true_);
        else
            lcd13264_ram_dat.lcd_io_set(_false_);

        dat <<= 1;

        lcd13264_ram_dat.lcd_scl_set(_true_);
    }

    lcd13264_ram_dat.lcd_cs_set(_true_); //�������
}

//*****************************************************//
//*******           Һ������ʼ������            *******//
//����typΪ��ʼ����ʽ    FALSEΪ����  TRUEΪ����
//*****************************************************//
void lcd_init(bit_enum typ, bit_enum ic) {
    lcd13264_ram_dat.lcd_cs_set(_true_);  //��ֹƬѡ��ȷ���´�ͨ�ŵ���������
    lcd13264_ram_dat.lcd_scl_set(_true_); //ʱ������Ϊ�ߵ�ƽ״̬
    lcd13264_ram_dat.lcd_rst_set(_true_); //�ͷŸ�λ
    lcd13264_ram_dat.lcd_delayms(20);     //��ʱ20���룬׼����λ

    lcd13264_ram_dat.lcd_rst_set(_false_);
    lcd13264_ram_dat.lcd_delayms(2);
    lcd13264_ram_dat.lcd_rst_set(_true_);
    lcd13264_ram_dat.lcd_delayms(10); //��ʱ10�������LCD��λ

    //-----------------------------
    lcd_write_byte(_true_, lcd_comm_nop);                 //������
    lcd_write_byte(_true_, lcd_comm_bias);                //����ƫ��
    lcd_write_byte(_true_, lcd_comm_inverse_display_off); //�������ص�������ʾ
    //-----------------------------

    lcd_write_byte(_true_, lcd_comm_reverse_seg_off);  //��������
    lcd_write_byte(_true_, lcd_comm_reverse_com_on);   //��������
    lcd_write_byte(_true_, lcd_comm_start_line(0x20)); //������ʾ��ʼ��

    //-----------------------------
    EN_LCD_BL_PWR;

    if (typ) {                        //����
        lcd_write_byte(_true_, 0x27); //���÷�ѹ����  (RR)    0x27
        lcd_write_byte(_true_, 0x81);
        //
        if (set_para.LCD_Contrast == 0) {
            lcd_write_byte(_true_, 0x28); //���õ�ѹ  (EV)
        } else {
            lcd_write_byte(_true_, set_para.LCD_Contrast); //���õ�ѹ  (EV)
        }
    } else {                          //����
        lcd_write_byte(_true_, 0x24); //���÷�ѹ����(���õ��ڵ�����)
        lcd_write_byte(_true_, 0x81);
        lcd_write_byte(_true_, 0x2d); //���õ�ѹ
    }

    lcd_write_byte(_true_, lcd_comm_power_on);   //�����õ�Դ
    lcd_write_byte(_true_, lcd_comm_display_on); //����ʾ

    if (ic) {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //������ʾ  /* ?????????????????   */
    }
}

//*****************************************************//
//*******            ������ʾ��ʼ��             *******//
//����sΪ������ţ���ΧΪ0-63
//*****************************************************//
void lcd_set_start_line(uchar s) {
    lcd_write_byte(_true_, lcd_comm_start_line(0x20 + s)); //������ʾ��ʼ��
}

//*****************************************************//
//*******    ���ض���ʾ������д��һ���ض���ֵ   *******//
//����x_stΪ������ʼ��ַ����ΧΪ0-7
//����x_endΪ������ֹ��ַ
//����y_stΪ������ʼ��ַ����ΧΪ0-131
//����y_endΪ������ֹ��ַ
//����dat         Ϊ��д�������
//*****************************************************//
void lcd_area_dis_set(uchar x_st, uchar x_end, uchar y_st, uchar y_end, uchar dat) {
    uchar x, y;

    if ((x_st >= lcd_max_x_page) || (y_st >= lcd_max_y_addr)) //��ʼ��ַ������Χ
    {
        return;
    }

    if (x_end >= lcd_max_x_page) //������ֹ��ַ�Ĵ���
    {
        x_end = (lcd_max_x_page - 1);
    }

    if (y_st >= lcd_max_y_addr) //������ֹ��ַ�Ĵ���
    {
        y_end = (lcd_max_y_addr - 1);
    }

    for (x = x_st; x <= x_end; x++) {
        for (y = y_st; y <= y_end; y++) {
            lcd_buff[x][y] = dat;
        }
    }

    lcd_refresh = _true_; //��ˢ������
}

//*****************************************************//
//*******                LCD������ʾ            *******//
//*****************************************************//
void lcd_dis_all(void) {
    uchar x, y;

    if (print_auto_data.req_flag)
        return;

    if (lcd_refresh == _false_) //��ˢ������
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
//*******         LCD������ʾһ������           *******//
//*****************************************************//
void lcd_dis_at_once(void) {
    lcd_refresh = _true_;
    lcd_dis_all();
}

//*****************************************************************************************
//�ú����Ĺ�������ʾһ���������ַ�������16*16,8*16,16*8,8*8���ִ�С
//
//����x_pageָ����ʾ�ַ�������λ�ã���ΧΪ0-7ҳ���˲�����LCD�������ֲ��е�ҳ���Ӧ
//����y_addrָ����ʾ�ַ��ĺ���λ�ã���ΧΪ0-131���˲�����LCD�������ֲ��е�Y��ַ���Ӧ
//����*dis_buffָ����ʾ�Ļ�����
//����dis_typeָ����ʾ�ַ��Ĵ�С��  0��ʾ8*16ASCII   1��ʾ16*16����  2��ʾ16*8ͼ��   3��ʾ8*8ͼ��
//����flagָ����ʾ�ַ��ĵ�����ʾ��ʽ,    FALSE��ʾ������ʾ
//                         TRUE��ʾ�ѵ�����Ϣȡ��������ʾ����һ����ҪӦ����ѡ�в˵���
//*****************************************************************************************
void lcd_dis_one_word(uchar x_page, uchar y_addr, const uchar *dis_buff, lcd_dis_type_enum dis_type, bool flg_reverse) {
    uchar x, y;
    uchar ix, iy;

    if ((x_page >= lcd_max_x_page) || (y_addr >= lcd_max_y_addr)) { //������ʾ��Χ��ֱ���˳�
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
        for (iy = 0x00; iy < y; iy++) { //���ʹ���ʾ������
            if (flg_reverse) {          //������ʾ
                lcd_buff[x_page + ix][y_addr + iy] = (~(*(dis_buff++)));
            } else { //������ʾ
                lcd_buff[x_page + ix][y_addr + iy] = (*(dis_buff++));
            }
        }
    }

    lcd_refresh = _true_; //��ˢ������
}

//*****************************************************************************************
//�ú����Ĺ�������ʾһ���������ַ�������16*16,8*16,16*8,8*8���ִ�С
//
//����x_pageָ����ʾ�ַ�������λ�ã���ΧΪ0-7ҳ���˲�����LCD�������ֲ��е�ҳ���Ӧ
//����y_addrָ����ʾ�ַ��ĺ���λ�ã���ΧΪ0-131���˲�����LCD�������ֲ��е�Y��ַ���Ӧ
//����*dis_buffָ����ʾ�Ļ�����
//����dis_typeָ����ʾ�ַ��Ĵ�С��  0��ʾ8*16ASCII   1��ʾ16*16����  2��ʾ16*8ͼ��   3��ʾ8*8ͼ��
//����flagָ����ʾ�ַ��ĵ�����ʾ��ʽ,    FALSE��ʾ������ʾ
//                         TRUE��ʾ�ѵ�����Ϣȡ��������ʾ����һ����ҪӦ����ѡ�в˵���
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
//�ú����Ĺ�������ʾһ���Զ��巶Χ��ͼƬ
//
//����x_pageָ����ʾ�ַ�������λ�ã���ΧΪ0-7ҳ���˲�����LCD�������ֲ��е�ҳ���Ӧ
//����y_addrָ����ʾ�ַ��ĺ���λ�ã���ΧΪ0-131���˲�����LCD�������ֲ��е�Y��ַ���Ӧ
//����*dis_buffָ����ʾ�Ļ�����
//����x_cntָ����ʾͼƬҳ����С����λΪҳ��ע�ⲻҪ������ʾ��Χ
//����y_cntָ����ʾͼƬ����Χ����λΪһ�����أ�ע�ⲻҪ������ʾ��Χ
//*****************************************************************************************
void lcd_dis_custom(uchar x_page, uchar y_addr, const uchar *dis_buff, uchar x_cnt, uchar y_cnt) {
    uchar ix, iy;

    if ((x_page >= lcd_max_x_page) || (y_addr >= lcd_max_y_addr)) //������ʾ��Χ��ֱ���˳�
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

    lcd_refresh = _true_; //��ˢ������
}

//*****************************************************************************************
//�ú����Ĺ�������ʾ���һ���ַ�  �߶�Ϊ������ʾҳ
//�ú����Զ�������趨����ʾλ�ÿ�ʼ���н��������е����ص�
//
//����x_posiָ����ʾ�ַ�������λ�ã���ΧΪ0-3
//����y_addrָ����ʾ�ַ��ĺ���λ�ã���ΧΪ0-131���˲�����LCD�������ֲ��е�Y��ַ���Ӧ
//����*dis_buffָ����ʾ�Ļ����������һ���ֽڱ�����0x00��β
//����flagָ����ʾ�ַ��ĵ�����ʾ��ʽ,    FALSE��ʾ������ʾ
//                         TRUE��ʾ�ѵ�����Ϣȡ��������ʾ����һ����ҪӦ����ѡ�в˵���
//*****************************************************************************************
void lcd_dis_one_line(uchar x_posi, uchar y_addr, const uchar *dis_buff, bool flg_reverse) {
    uint word_addr;
    uchar point;
    uchar temp_buff[32];

    lcd_area_dis_set((x_posi * 2), ((x_posi * 2) + 1), y_addr, (lcd_max_y_addr - 1), 0x00); //��ʾһ��ʱ��������е�����

    point = 0x00;                                                        //�Ƚ���ʾ����ָ������
    while ((*(dis_buff + point) != 0x00) && (y_addr < lcd_max_y_addr)) { //���д���ʾ������
        if (*(dis_buff + point) >= 0xa1) {
            word_addr = (*(dis_buff + point)) * 0x100 + (*(dis_buff + point + 1));
            lcd13264_ram_dat.lcd_font(word_addr, temp_buff); //������
            lcd_dis_one_word((x_posi * 2), y_addr, temp_buff, lcd_dis_type_16x16chn, flg_reverse /*0,false ������ʾ; 1,true ����;*/);

            point += 2;
            y_addr += 16; //�޸���ʾָ��
        } else {
            word_addr = *(dis_buff + point);
            lcd13264_ram_dat.lcd_font(word_addr, temp_buff); //ASCII��
            lcd_dis_one_word((x_posi * 2), y_addr, temp_buff, lcd_dis_type_8x16ascii, flg_reverse /*0,false ������ʾ; 1,true ����;*/);

            point += 1;
            y_addr += 8; //�޸���ʾָ��
        }
    }
}
//********************************************************************//
//********************************************************************//

//*****************************************************//
//*******   ������뻺�����й�������ֽڸ���    *******//
//����srcΪ������Ļ�����
//����cntΪ����������ݳ���
//�������ظ���
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
//������ʾһҳ�����е�����������������Ϊ0�����н�����ʾ�������޸�
//
//����buffΪ������
//����lgthΪ��ʾ����Ч����
//*****************************************************************************************
static void display_page_line_check(uchar *buff, uchar lgth) {
    uchar i;

    for (i = 0x00; i < lgth; i++) {
        if (buff[i] == 0x00) {
            buff[i] = 0x20; //�ѷǷ�������־��Ϊ�ո��ַ�
        }
    }
}

//*****************************************************************************************
//��ʾһҳ��Ϣ
//
//��һ�������ݵ���һ�����壬 ���������ʾ(8*4)-1=31�����ֻ���62���ַ�
//����������ʾ15*3+13=58���ַ�
//��ʾλ�����ַ�Ϊ��λ��λ�ñ�ŷ�ΧΪ0-61
//
//����start��ʾ��ʼλ�ã���Χ0-61
//����lgth��ʾ����ʾ�����ݵĳ��ȣ����Դ��ڵ�����ʾ�����Χ���������з���ֵ������û��ʾ����ֽ���
//����buff��ʾ����ʾ�Ļ�����
//��������ֵΪ�β��д���ʾ���Ȳ���ͨ�����ε��øú����󣬻�ʣ�µĴ���ʾ���ַ��ĸ���
//*****************************************************************************************
uint lcd_dis_one_page(unsigned char start, unsigned char len, unsigned char *buf) {
    uchar line, addr;
    uchar n;
    uchar temp[20];

    if (start >= lcd_page_char_max_num) //������ʾ��Χ��������ʾ�κ�����
        return 0x00;

    line = (start / lcd_line_char_max_num); //������ʼ��ʾ���к�
    addr = (start % lcd_line_char_max_num); //�����ڸ��е���ʼλ��

    do {
        if ((addr + len) > lcd_line_char_max_num)
            n = (lcd_line_char_max_num - addr); //���㱾����ʾ���ַ���
        else
            n = len;

        if (line == 3) //���һ�з�ָ���ͷ��־
        {
            if (n > (lcd_line_char_max_num - 2)) {
                n = (lcd_line_char_max_num - 2);
            }
        }

        if (data_cnt_gb((buf), n) % 2) //���������һ
        {
            n -= 1;
        }

        memcpy(temp, buf, n);
        display_page_line_check(temp, n); //����������ݵķǷ�������־

        *(temp + n) = '\x0'; //������־
        lcd_dis_one_line(line, (addr * 8), temp, false);

        buf += n;  //�޸Ĵ���ʾ�Ļ�����ָ��
        len -= n;  //�޸Ĵ���ʾ�ĳ���

        addr = 0x00;
        line++;
    } while ((line < 4) && (len > 0));

    return len; //��ǰҳû��ʾ������ݸ���
}
//********************************************************************//
//********************************************************************//
