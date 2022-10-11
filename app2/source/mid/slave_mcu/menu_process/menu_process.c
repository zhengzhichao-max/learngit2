#include "include_all.h"

#include "menu_process.font"

#if (0)
static void wait_here(void) {
    beeper(0); //�ض���

    log_write(event_mcu_boot_wait_here);

    while (1) { ; }
}
#endif

//***********************************************************************************************//
//***********************************************************************************************//
//-------------------------------      ��ʾ�ײ㺯��     -----------------------------------------//
//*************************************************************
//�ú���ʵ�ֵĹ����Ǹ����ꡢ�¡�������������
//��������ֵ  0��ʾ������  1��ʾ����һ  �Դ�����
//
//*************************************************************
u8 date_to_week(u16 year, u8 month, u8 date) {
    u8 week, c, temp_y;

    if (month <= 2) {
        month |= 4; //1��2��ͬ5��6�±�
        year--;
    }

    c = year / 100;
    c &= 0x03; //����%4
    temp_y = (char)(year % 100);

    week = ((c | (c << 2)) + (temp_y + (temp_y >> 2)) + (13 * month + 8) / 5 + date) % 7; //����=(����%4*5+��+��/4+(13*��+8)/5+��)%7

    return week; //��������
}

//*************************************************************
//������60����ת����10����
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
//��ʾ��ʱʱ��
//��ʾλ������ʾ�������Ͻ�
//����m_sΪʱ��ֵ����λΪ����
//
//*************************************************************
static void dis_time(u32 m_s) {
    u8 tim_m, tim_s;
    u8 dis_buff[6];

    m_s = m_s / 1000; //��

    tim_m = m_s / 60;
    tim_m = tim_m % 60; //���ܳ���60
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
//�����ʾ�������ַ���û�г�������ASCII�뷶Χ
//
//��������ֵ   TRUE��ʾ���ݸ�ʽ����    FALSE��ʾ���ݸ�ʽ��ȷ
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
//�����ʾ�������ַ���û�г�������ASCII�뷶Χ�����г�������ǿ�Ƹ�Ϊ0x30
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
//��ʾ�˻ص���������
//
//*************************************************************
void dis_goto_standby(void) {
    _memset((u8 *)(&menu_auto_exit), 0x00, (sizeof(menu_auto_exit)));
    _memset((u8 *)(&menu_report), 0x00, (sizeof(menu_report)));
    _memset((u8 *)(&menu_cnt), 0x00, (sizeof(menu_cnt)));

    menu_cnt.menu_other = mo_standby;
    menu_cnt.menu_flag = true;
}

//����˵���ҳ��ʾ�ĵ�һ��λ��
static u16 menu_move_first_line(u16 last, u16 point) {
    u16 i;

    point -= 1;

    if (point > (last + 2)) //ȷ����ʾ��һ��˵�ֵ
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
//�ú����Ĺ����ǲ˵���ҳ��ʾ
//
//����buffָ����ʾ�˵��Ĳ˵����ݻ�������Ϊ��άָ��
//����first_lineΪ����ʾ����λ�������˵����е�λ��
//����pointָ��ǰ��ʾ�˵�������ѡ�е�һ��    �ӵ�1�ʼ����
//����lengthָ����ʾ�˵��µ������Ӳ˵��ĸ���
//
//*************************************************************
static void menu_move_display(const u8 **buff, u16 first_line, u16 point, u16 length) {
    u16 m;
    u8 j, n;

    if (length == 2) //�����У����������һ��
        lcd_area_dis_set(6, 7, 0, 131, 0x00);
    else if (length == 1) //��һ�У��������������
        lcd_area_dis_set(4, 7, 0, 131, 0x00);
    else if (length == 0) //�����ݣ��������������
        lcd_area_dis_set(2, 7, 0, 131, 0x00);

    if (point <= length) //�ж����ݵĺϷ���
    {
        for (j = 0x00; j < 3; j++) {
            m = (first_line + j);

            if (m < length) //�ж���ʾ�Ƿ񳬹���ΧΪ
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

        menu_cnt.last_start = first_line; //��¼�˵���ʾ��һ�����ݵ�λ��
    }
}

//*************************************************************
//���ܼ��Ĵ���  ����    (1)�Ϸ����   (2)�·����    (3)ȷ����    (4)�˸��
//����menu_type��ʾ�˵�������  ����menu_length��ʾ�ü��˵����������Ӳ˵�����
//��������ֵ����   0������˵�������ȷ�ϼ���Ҫ��һ������
//                 1�������ܴ������    2������������
//
//*************************************************************
static u8 function_key_hdl(u8 menu_type, u16 menu_length) {
    u16 *ptr;
    u8 flag;

    flag = 1;

    if (menu_type <= menu_max_depth) {
        menu_type -= 1;
        ptr = (((u16 *)(&menu_cnt)) + menu_type); //��ָ�븳ֵ

        if (key_data.key == key_down) { //�·�ҳ
            if (menu_length > 0) {
                (*ptr) += 1;

                if ((*ptr) > menu_length)
                    (*ptr) = 0x01;

                menu_cnt.menu_flag = true; //�˵��������
            }
        } else if (key_data.key == key_up) { //�Ϸ�ҳ
            if (menu_length > 0) {
                (*ptr) -= 1;

                if ((*ptr) < 0x01)
                    (*ptr) = menu_length;

                menu_cnt.menu_flag = true; //�˵��������
            }
        } else if (key_data.key == key_ok) { //�����Ӳ˵�
            if (menu_length > 0) {
                if (menu_cnt.next_regular) { //����˵��������һ��
                    *(ptr + 1) = 0x01;

                    menu_cnt.menu_flag = true; //�˵��������
                } else {                       //������˵�
                    flag = 0;
                }
            }
        } else if (key_data.key == key_esc) { //�˳��ü��˵�
            (*ptr) = 0x00;

            menu_cnt.last_start = 0x00;
            menu_cnt.menu_flag = true; //�˵��������
        }
    } else {
        flag = 2; // 2������������
    }

    return flag;
}

//*************************************************************
//���뷨��ʹ��ǰ�ĳ�ʼ������
//
//����ptr_dis��ʾ����ʾ���뷨������ָ��
//����ptr_d��ʾ���뷨Ŀ���ַ
//����lgth��ʾ��һ���������ݳ���
//����max_lgth��ʾ�����������ݵļ��޳���
//����encrypt��ʾ��ʾ�Ƿ����   TRUE��ʾ����   FALSE��ʾ������
//
//*************************************************************
static void input_method_init(const u8 *ptr_dis, u8 *ptr_d, u16 lgth, u16 max_lgth, bool encrypt) {
    //--------------//
    input_method.input_lib.page_max = ascii_input_page_max;
    input_method.input_lib.sig_size = ascii_input_size;

    input_method.input_lib.crisis = 0;
    input_method.input_lib.front_crisis_size = ascii_input_size;

    input_method.input_lib.ptr_lib = ascii_char_table;
    //--------------//���뷨Ĭ������  ����ԴΪASCII��

    input_method.lgth = max_lgth;   //���ñ����������ݵ���󳤶�
    input_method.encrypt = encrypt; //���ü���״̬

    input_method.ptr_dis = ptr_dis; //���ش���ʾ���뷨������ָ��
    input_method.ptr_dest = ptr_d;  //�������뷨Ŀ���ַ

    input_method.page = 0x00;  //��λֵ
    input_method.point = 0x00; //��λֵ

    if (lgth > 60)
        lgth = 60;
    input_method.cnt = lgth; //�ָ���һ���������ݳ���

    _memcpy(input_method.buff, ptr_d, lgth); //�ָ���һ����������
}

//*************************************************************
//�Զ��˳���������
//
//����TIM     ����ͣ��ʱ��   ��λ1����
//����DIR     TRUE��ʾ�˳�����һ������˵�   FALSE��ʾ��������
//
//*************************************************************
void menu_auto_exit_set(u32 tim, bool dir) {
    menu_auto_exit.lgth = tim;     //������ʾʱ��
    menu_auto_exit.dir_flag = dir; //�˳�����

    menu_auto_exit.cnt = jiffies;    //ʱ��ͬ��
    menu_auto_exit.auto_flag = true; //�Զ��˳�����
}

//*************************************************************
//�����������
//
//����TIM     ����ͣ��ʱ��   ��λ1����
//����PTR     ���������ʾ����
//����TYP     Ϊ���ͱ������������
//����f       ��������    TRUE����ɹ���ʧ��    FALSE������ʧ��
//
//*************************************************************
//COMT:���棬�������ɹ���ʧ��
static void menu_report_set(u32 tim, const u8 *ptr[], report_type_enum typ, bool f) {
    menu_report.lgth = tim; //��ʱʱ��
    menu_report.ptr = ptr;  //��ʾ����

    menu_report.dat_type = typ; //��������
    menu_report.rpt = f;        //��������

    menu_report.cnt = jiffies; //ʱ��ͬ��
    menu_report.ok_flag = false;
    menu_report.en_flag = true;
}

//*************************************************************
//���뷨����
//
//��������ֵ��ʾ�Ƿ���ȷ��״̬   TRUE��ʾ�а���   FALSE��ʾû�а���
//
//*************************************************************
static bool input_method_hdl(void) {
    u8 n;
    u16 m;
    u8 temp[36];
    bool ret;

    ret = false;
    lcd_dis_one_line(0, (8 * 0), input_method.ptr_dis, false); //����������

    lcd_area_dis_set(4, 5, 0, 131, 0x00);
    if (input_method.encrypt) { //����״̬
        _memset(temp, '*', input_method.cnt);
        lcd_dis_one_page(16, input_method.cnt, temp);              //�ڶ��п�ʼ��ʾ
    } else {                                                       //������״̬
        lcd_dis_one_page(16, input_method.cnt, input_method.buff); //�ڶ��п�ʼ��ʾ
    }

    if (input_method.page >= input_method.input_lib.page_max) { //��ֹԽ��
        input_method.page = 0x00;
    }
    n = _strlen((u8 *)((*(input_method.input_lib.ptr_lib + (input_method.page)))));
    if (input_method.point >= n) { //��ֹԽ��
        input_method.point = 0x00;
    }
    lcd_dis_one_line(3, (8 * 0), (*(input_method.input_lib.ptr_lib + (input_method.page))), false); //���뷨ҳ��ʾ

    input_method.point -= (input_method.point % input_method.input_lib.sig_size); //��ֹ��ʾ��λ
    if (input_method.input_lib.sig_size == 1) {                                   //ASCII��
        m = *(input_method.input_lib.ptr_lib[input_method.page] + input_method.point);
        read_flash_fonts(m, temp);
        lcd_dis_one_word((3 * 2), (8 * input_method.point), temp, lcd_dis_type_8x16ascii, true);
    } else if (input_method.input_lib.sig_size == 2) { //������
        m = ((input_method.input_lib.ptr_lib[input_method.page][input_method.point]) * 0x100)
            + (input_method.input_lib.ptr_lib[input_method.page][input_method.point + 1]);
        read_flash_fonts(m, temp);
        lcd_dis_one_word((3 * 2), (8 * input_method.point), temp, lcd_dis_type_16x16chn, true);
    }

    //----------------------------------------------

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) {                //������������
            if (key_data.key == key_ok) { //�˴�����������
                ret = true;               //�������뷨�������
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
        } else {                          //�����̰�����
            if (key_data.key == key_ok) { //ѡ��
                if (input_method.cnt < input_method.lgth) {
                    _memcpy((input_method.buff + input_method.cnt),
                            (*(input_method.input_lib.ptr_lib + (input_method.page)) + (input_method.point)),
                            (input_method.input_lib.sig_size));

                    input_method.cnt += input_method.input_lib.sig_size;
                }
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_esc) { //�˸�
                if (input_method.cnt > 0) {
                    if ((input_method.input_lib.crisis != 0x00) && (input_method.cnt == input_method.input_lib.crisis)) {
                        input_method.cnt -= input_method.input_lib.front_crisis_size;
                    } else {
                        input_method.cnt -= input_method.input_lib.sig_size;
                    }
                } else {
                    menu_cnt.menu_other = mo_reset; //�˻ص���һ��������˵�
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) { //�Ϸ�
                if (input_method.point > 0) {
                    input_method.point -= input_method.input_lib.sig_size;
                } else {
                    input_method.point = (n - input_method.input_lib.sig_size);
                }

                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_down) { //�·�
                input_method.point += input_method.input_lib.sig_size;
                if (input_method.point > (n - input_method.input_lib.sig_size)) {
                    input_method.point = 0x00;
                }

                menu_cnt.menu_flag = true;
            }
            /*
            else if (key_data.key == key_p) //�л�����ҳ
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

    return ret; //���ذ�����ȷ��״̬
}

//*************************************************************
//��ʾ��ҳ���ݴ���

//����buffΪ����ʾ�Ļ�����
//����lgthΪ��ʾ���ܳ���
//����input_flagΪ���뷨ʹ��  TRUEʹ�����뷨����  FALSE��ֹ���뷨����
//����input_disʵ�����뷨�Ľ���
//
//*************************************************************
#define dis_multi_page_max 36 //��ҳ��ʾ��ҳ�ļ���
static void dis_multi_page(u8 *buff, u16 lgth, bool input_flag, dis_multi_page_input_point input_dis_hdl) {
    static u8 last_point, last_cnt[dis_multi_page_max]; //36���洢�������ܼ�ס36ҳ��ʾ���ȣ�������ʾ1044������

    u8 current_cnt, state;
    u16 n;

    lcd_area_dis_set(0, 7, 0, 131, 0x00);
    if (dis_multi_page_cnt > lgth)
        dis_multi_page_cnt = 0x00; //��ֹ�����ֵӰ����ʾ

    n = lcd_dis_one_page(0, (lgth - dis_multi_page_cnt), (buff + dis_multi_page_cnt));
    current_cnt = ((lgth - dis_multi_page_cnt) - n); //��ǰҳ����ʾ���ַ�����

    if ((n == 0x00) && (dis_multi_page_cnt == 0x00)) {
        state = 0x00; //��ҳ����ʾ���

        if (input_flag) //ʹ�����뷨����
        {
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
        }
    } else {
        if (n == 0x00) //�Ѿ���ʾ�������һҳ
        {
            state = 0x03;

            if (input_flag) //ʹ�����뷨����
            {
                lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
            } else {
                lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
            }
        } else if (dis_multi_page_cnt == 0x00) //��ǰ��ʾ���ǵ�һҳ
        {
            state = 0x01;
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);

            last_point = 0x00; //ÿ����ʾ��һҳʱ������ָ��
        } else                 //��ǰ��ʾ�����м�ҳ��
        {
            state = 0x02;
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        }
    }

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (!key_data.lg) //�����̰�����
        {
            if (key_data.key == key_esc) //�˳���ǰ�Ĳ���
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            } else if (key_data.key == key_up) //�Ϸ�ҳ����
            {
                if ((state == 0x03) || (state == 0x02)) {
                    dis_multi_page_cnt -= *(last_cnt + (last_point - 1));

                    if (last_point) {
                        last_point--;
                        ;
                    }
                    menu_cnt.menu_flag = true;
                }
            } else if (key_data.key == key_down) //�·�ҳ����
            {
                if ((state == 0x01) || (state == 0x02)) {
                    if (last_point < dis_multi_page_max) {
                        dis_multi_page_cnt += current_cnt;      //�·�ҳ����
                        *(last_cnt + last_point) = current_cnt; //��¼��һ�εĲ���ֵ

                        last_point++;
                        menu_cnt.menu_flag = true;
                    }
                } else if (input_flag) //ʹ�����뷨����
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
//�ϴ��¼�����
//
//*************************************************************
void sms_fix_input_set(void) {
    menu_cnt.menu_other = mo_sms_fix_input;
    input_method_init((*(sms_input_dis + 0)), (&(sms_up_center.fix_status)), 0, 1, false);
}

//*************************************************************
//��Ϣ�㲥����
//
//*************************************************************
void sms_vod_input_set(void) {
    menu_cnt.menu_other = mo_sms_vod_input;
    input_method_init((*(sms_input_dis + 2)), (&(sms_up_center.vod_status)), 0, 1, false);
}

//*************************************************************
//����Ӧ������
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
//-------------------------------    ������˵��Ĵ���  -----------------------------------------//

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

        if (state_data.state.flag.custom_flag) //��ʾ�Զ�������
        {
            ;
        } else //��ʾ������Ӫ��
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

        if (state_data.state.flag.ic_flag) //IC�����룬��ʾ����
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
            temp_buff[5] = 0x00; //������־

            lcd_dis_one_line(2, (8 * 0), temp_buff, false);
        }
    } else {
        lcd_area_dis_set(2, 5, 0, (lcd_max_y_addr - 1), 0x00); //�������

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
        temp_buff[4] = 0x00; //������־
        lcd_dis_one_line(1, (8 * 11 + 4), temp_buff, false);
    }

    //-----------------------------//
    //-------     ��λ��ʾ    -----//
    dat16 = (gps_data.direction[0] & 0x0f) * 100;
    dat16 += (gps_data.direction[1] >> 4) * 10;
    dat16 += (gps_data.direction[1] & 0x0f);

    if ((dat16 >= dir_north_range_high) && (dat16 < dir_east_range_low)) {
        ptr = dir_lib_dis[0]; //����
    } else if ((dat16 >= dir_east_range_low) && (dat16 < dir_east_range_high)) {
        ptr = dir_lib_dis[1]; //����
    } else if ((dat16 >= dir_east_range_high) && (dat16 < dir_south_range_low)) {
        ptr = dir_lib_dis[2]; //����
    } else if ((dat16 >= dir_south_range_low) && (dat16 < dir_south_range_high)) {
        ptr = dir_lib_dis[3]; //����
    } else if ((dat16 >= dir_south_range_high) && (dat16 < dir_west_range_low)) {
        ptr = dir_lib_dis[4]; //����
    } else if ((dat16 >= dir_west_range_low) && (dat16 < dir_west_range_high)) {
        ptr = dir_lib_dis[5]; //����
    } else if ((dat16 >= dir_west_range_high) && (dat16 < dir_north_range_low)) {
        ptr = dir_lib_dis[6]; //����
    } else {
        ptr = dir_lib_dis[7]; //����
    }

    lcd_dis_one_line(2, (8 * 11 + 4), ptr, false);
    //-----------------------------//
    //-----------------------------//
}

//������:mo_standby_hdl
static void mo_standby_hdl(void) {
    u8 n;
    u8 i;
    u8 temp_buff[20];
    const u8 *ptr;

    //--------------------//

    lcd_area_dis_set(0, 1, 0, 131, 0x00);
    DIS_SELF_CHECK;

#pragma region CSQ, �ź�ǿ��
    n = (gps_data.csq_state & 0x7F);
    if (n > 99) //��ֹ������Χ
        n = 99;

    if (n >= 20)
        ptr = state_flag_dis[3]; //�����ź�(���ڵ���20)
    else if (n >= 9)
        ptr = state_flag_dis[17]; //һ���ź�(���ڵ���9С��20)
    else if (n >= 5)
        ptr = state_flag_dis[18]; //һ���ź�(���ڵ���5С��9)
    else
        ptr = state_flag_dis[19]; //���ź�(С��5)

    lcd_dis_one_word(0, (16 * 0), ptr, lcd_dis_type_8x16icon, false);
#pragma endregion CSQ, �ź�ǿ��

#pragma region GP
    //   if (testbit((gps_data.gprs_alarm_state), 0)) //���λ��ʾ����״̬
    if (gs.gprs1.login || gs.gprs2.login || BBstatus) {
        lcd_dis_one_word(0, (16 * 1), state_flag_dis[4], lcd_dis_type_8x16icon, false);
    }
#pragma endregion GP

#pragma region gps_type
    if (testbit((gps_data.gps_state), 7)) {                                             //���λ��ʾ��λ״̬
        lcd_dis_one_word(0, (16 * 2), state_flag_dis[5], lcd_dis_type_8x16icon, false); //��λģʽ

        if (host_no_save_para.set_gps_module_type == 0x02) //��BD
            ptr = state_flag_dis[21];
        else if (host_no_save_para.set_gps_module_type == 0x03) //BD+GPS
            ptr = state_flag_dis[22];
        else //��GPS
            ptr = state_flag_dis[20];

        lcd_dis_one_word(0, (16 * 3), ptr, lcd_dis_type_8x16icon, false);
    }
#pragma endregion gps_type

#pragma region Ӳ����
    if (pub_io.b.key_s == false) {
        lcd_dis_one_word(0, (16 * 4), state_flag_dis[10], lcd_dis_type_8x16icon, false);
    }
#pragma endregion Ӳ����

#pragma region δ����Ϣ
    if (state_data.state.flag.nread_sms) { //δ����Ϣ
        lcd_dis_one_word(0, (16 * 5), state_flag_dis[11], lcd_dis_type_8x16icon, false);
    }
#pragma endregion δ����Ϣ

#pragma region �������Ǳ�־ + ����
    lcd_dis_one_word(0, (16 * 6), state_flag_dis[15], lcd_dis_type_8x16icon, false); //�������Ǳ�־
    n = ((gps_data.gps_state) & 0x3F);                                               //��5λ��ʾ����������
    lcd_dis_one_word(0, (16 * 7), num88_dis[(n / 10)], lcd_dis_type_8x8icon, false);
    lcd_dis_one_word(0, (16 * 7 + 8), num88_dis[(n % 10)], lcd_dis_type_8x8icon, false); //����������
#pragma endregion �������Ǳ�־ + ����

#pragma region IC��״̬
    if (state_data.state.flag.ic_flag) { //IC��״̬
        lcd_dis_one_word(lcd_pos_8x16(1, 0), state_flag_dis[6], lcd_dis_type_8x16icon, false);
    }
#pragma endregion IC��״̬

#pragma region SD��״̬ + HD״̬
    const uchar *sd_hd_state[] = {state_flag_dis[7], state_flag_dis[8], state_flag_dis[12], state_flag_dis[23]};
    sdhd_staSel_enum sd_hd_sta = selfChk_HD_SD_stateGet();
    if (sd_hd_sta != sdhd_staSel_unknown) {
        lcd_dis_one_word(lcd_pos_8x16(1, 1), sd_hd_state[(int)sd_hd_sta], lcd_dis_type_8x16icon, false);
    }
#pragma endregion SD��״̬ + HD״̬

#pragma region ����ͷ״̬
    const uchar *cam_state = state_flag_dis[15];
    cam_sta_enum cam_sta = selfChk_cam_stateGet();
    // if (sd_hd_sta != sdhd_staSel_unknown) {
    if (sd_hd_sta == cam_sta_err) {
        lcd_dis_one_word(lcd_pos_8x16(1, 2), cam_state, lcd_dis_type_8x16icon, false);
    }
    // }
#pragma endregion ����ͷ״̬

#pragma region USB״̬
    if (state_data.state.flag.usb_flag) { //USB״̬//δʹ��
        lcd_dis_one_word(1, (16 * 2), state_flag_dis[8], lcd_dis_type_8x16icon, false);
    }
#pragma endregion USB״̬

    //--����һ��--//1, (16 * 3)//δʹ��

    if ((state_data.state.flag.up_flag) && (!(state_data.state.flag.down_flag))) { //������״̬//δʹ��
        lcd_dis_one_word(1, (16 * 4), state_flag_dis[13], lcd_dis_type_8x16icon, false);
    } else if ((!(state_data.state.flag.up_flag)) && (state_data.state.flag.down_flag)) {
        lcd_dis_one_word(1, (16 * 4), state_flag_dis[14], lcd_dis_type_8x16icon, false);
    }
    if (state_data.state.flag.nack_phone) { //δ�ӵ绰��־
        lcd_dis_one_word(1, (16 * 5), state_flag_dis[12], lcd_dis_type_8x16icon, false);
    }

    //--����һ��--//1, (16 * 6)//δʹ��
    //�ػ�״̬

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

    i = 0x00; //��ʾ���ڸ�ʱ��
    temp_buff[i++] = '2';
    temp_buff[i++] = '0';
    temp_buff[i++] = (gps_data.date[0] >> 4) + 0x30; //��
    temp_buff[i++] = (gps_data.date[0] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i++] = '-';
    temp_buff[i++] = (gps_data.date[1] >> 4) + 0x30; //��
    temp_buff[i++] = (gps_data.date[1] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i++] = '-';
    temp_buff[i++] = (gps_data.date[2] >> 4) + 0x30; //��
    temp_buff[i++] = (gps_data.date[2] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i] = 0x00; //������־
    lcd_dis_one_line(3, (8 * 0), temp_buff, false);

    i = 0x00;
    temp_buff[i++] = (gps_data.time[0] >> 4) + 0x30; //ʱ
    temp_buff[i++] = (gps_data.time[0] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i++] = ':';
    temp_buff[i++] = (gps_data.time[1] >> 4) + 0x30; //��
    temp_buff[i++] = (gps_data.time[1] & 0x0f) + 0x30;
    check_dis_data_error((temp_buff + i - 2), 2);
    temp_buff[i] = 0x00; //������־
    lcd_dis_one_line(3, (8 * 11 + 2), temp_buff, false);

    //--------------------

    //--------------   ��������   --------------//
    if (key_data.active) //����а�������
    {
        if (key_data.lg) //������������
        {
#ifdef gb19056_ver //��ʻ��¼�ǰ汾
            //-------------------------------------------------//
            if (key_data.key == key_ok) //�����ӡ����
            {
                menu_cnt.menu_other = mo_print;
                menu_cnt.menu_flag = true;       //�����˵�����
            } else if (key_data.key == key_down) //�̼�ѡ���ݽ���
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 5;
                menu_cnt.menu_2nd = 5;
                menu_cnt.menu_3rd = 1; //�̼�ѡ��

                menu_cnt.menu_flag = true; //�����˵�����
            }
            //  else if (key_data.key == key_ok)  	//���Ž���
            else if (key_data.key == key_esc) //���Ž���
            {
                input_method_init((*(dial_lib_dis + 0)), (phone_data.buff + 2), (*((u16 *)(phone_data.buff))), telephone_num_max_lgth, false);
                phone_data.step = call_dial;
                //
                menu_cnt.menu_other = mo_telephone;
                menu_cnt.menu_flag = true; //�����˵�����
                _memset((void *)(&phbk_call_in_data), 0x00, sizeof(phbk_call_in_data));
                //
                // menu_cnt.menu_other = mo_print;
                //menu_cnt.menu_flag = true;    	//�����˵�����
            }

            //-------------------------------------------------//
#else //��ͨ�汾 \
      //-------------------------------------------------//
            if (key_data.key == key_p) //�����ӡ����
            {
                menu_cnt.menu_other = mo_print;
                menu_cnt.menu_flag = true;       //�����˵�����
            } else if (key_data.key == key_down) //�̼�ѡ���ݽ���
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 5;
                menu_cnt.menu_2nd = 5;
                menu_cnt.menu_3rd = 1; //�̼�ѡ��

                menu_cnt.menu_flag = true;     //�����˵�����
            } else if (key_data.key == key_ok) //���Ž���
            {
                input_method_init((*(dial_lib_dis + 0)), (phone_data.buff + 2), (*((u16 *)(phone_data.buff))), telephone_num_max_lgth, false);
                phone_data.step = call_dial;

                menu_cnt.menu_other = mo_telephone;
                menu_cnt.menu_flag = true; //�����˵�����
                _memset((void *)(&phbk_call_in_data), 0x00, sizeof(phbk_call_in_data));
            }
            //-------------------------------------------------//
#endif
        } else //�����̰�����
        {
#ifdef gb19056_ver //��ʻ��¼�ǰ汾
            //-------------------------------------------------//
            if ((key_data.key == key_esc) || (key_data.key == key_ok)) //�������˵�
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 1;

                menu_cnt.menu_flag = true;     //�����˵�����
            } else if (key_data.key == key_up) //����GPS��Ϣ����
            {
                menu_cnt.menu_other = mo_gps_infor1;
                menu_cnt.menu_flag = true; //�����˵�����
            }
            //-------------------------------------------------//
#else //��ͨ�汾 \
      //-------------------------------------------------//
            if (key_data.key == key_ok) //�������˵�
            {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_1st = 1;

                menu_cnt.menu_flag = true;     //�����˵�����
            } else if (key_data.key == key_up) //����GPS��Ϣ����
            {
                // menu_cnt.menu_other = mo_gps_infor1;
                menu_cnt.menu_flag = true; //�����˵�����
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
        lcd_dis_one_line(0, 1, "����:", false);

        if (car_state.bit.lng)
            lcd_dis_one_line(1, 1, "����", false);
        else
            lcd_dis_one_line(1, 1, "����", false);

        sprintf((char *)buff, "%d", mix.locate.lngi);
        lcd_dis_one_line(1, 40, buff, false);

        lcd_dis_one_line(2, 1, "γ��:", false);
        if (car_state.bit.lat)
            lcd_dis_one_line(3, 1, "��γ", false);
        else
            lcd_dis_one_line(3, 1, "��γ", false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%d", mix.locate.lati);
        lcd_dis_one_line(3, 40, buff, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
    } break;
    case 0x02: {
        if (bd_rcd_io.b.left_light)
            lcd_dis_one_line(0, 1, "��ת��:�Ѵ�", false);
        else
            lcd_dis_one_line(0, 1, "��ת��:�ѹر�", false);

        if (bd_rcd_io.b.right_light)
            lcd_dis_one_line(1, 1, "��ת��:�Ѵ�", false);
        else
            lcd_dis_one_line(1, 1, "��ת��:�ѹر�", false);

        if (bd_rcd_io.b.far_light)
            lcd_dis_one_line(2, 1, "�����:�Ѵ�", false);
        else
            lcd_dis_one_line(2, 1, "�����:�ѹر�", false);

        if (bd_rcd_io.b.near_light)
            lcd_dis_one_line(3, 1, "Զ���:�Ѵ�", false);
        else
            lcd_dis_one_line(3, 1, "Զ���:�ѹر�", false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
    } break;
    case 0x03: //����ID    ���ŷ������
    {
        if (bd_rcd_io.b.brake)
            lcd_dis_one_line(0, 1, "�ƶ�:�Ѵ�", false);
        else
            lcd_dis_one_line(0, 1, "�ƶ���:�ر�", false);

        sprintf((char *)buff, "%02d", mix.dist);
        lcd_dis_one_line(1, 1, "���:", false);
        lcd_dis_one_line(1, 40, buff, false);

        if (gnss_cfg.select.b.GPS_EN)
            lcd_dis_one_line(2, 1, "��λģʽ: GPS", false);
        else if (gnss_cfg.select.b.BD_EN)
            lcd_dis_one_line(2, 1, "��λģʽ: ����", false);
        else
            lcd_dis_one_line(2, 1, "��λģʽ: ˫ģ", false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%02d", factory_para.set_speed_plus);
        lcd_dis_one_line(3, 0, "����ϵ��: ", false);
        lcd_dis_one_line(3, 72, buff, false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
    } break;

    default:
        cnt = 0x00;
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
        lcd_dis_one_line(0, 1, "��λ״̬:", false);
        if (car_state.bit.fix)
            lcd_dis_one_line(0, 72, "��λ", false);
        else
            lcd_dis_one_line(0, 72, "δ��λ", false);

        lcd_dis_one_line(1, 1, "��λ����:", false);
        sprintf((char *)buff, "%d", mix.fix_num);
        lcd_dis_one_line(1, 72, buff, false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%d", mix.speed);
        lcd_dis_one_line(2, 1, "GPS�ٶ�:", false);
        lcd_dis_one_line(2, 72, buff, false);

        memset(buff, 0x00, 10);
        sprintf((char *)buff, "%d", mix.heigh);
        lcd_dis_one_line(3, 1, "���θ߶�:", false);
        lcd_dis_one_line(3, 72, buff, false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
    }

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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
                cnt = 0x00; //�˳�ʱ���õ���һҳ
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
    //------------------------------------------//
}

//GPSȫ��Ϣ����2
static void mo_gps_infor2_hdl(void) {
    u8 buff[20] = {0};
    u16 i;
    //    u16 j;
    u16 n;
    //bool flag;

    i = _strlen((u8 *)(*(gps_dis + 4)));
    _memcpy(buff, (*(gps_dis + 4)), i); //�������ٶȡ�

    n = (0);
    if (n >= 99)
        n = 99;
    buff[i++] = n / 10 + 0x30; //�����ٶ���ֵ
    buff[i++] = n % 10 + 0x30;
    buff[i++] = 0x00;
    lcd_dis_one_line(0, 0, buff, false);
    /**********************************************************************/
    _memset(buff, 0, 20);
    i = _strlen((u8 *)(*(gps_dis + 5)));
    _memcpy(buff, (*(gps_dis + 5)), i); //�����ƾ�����
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
    _memcpy((buff + n), (*(gps_dis + 12)), i); //Χ��
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
    //���   �Ҹ�
    i = _strlen((u8 *)(*(gps_dis + 2)));
    _memcpy(buff, (*(gps_dis + 2)), i); //��ת

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
    _memcpy((buff + n), (*(gps_dis + 3)), i); //��ת
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
    _memcpy(buff, (*(gps_dis + 8)), i); //��ת

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
    _memcpy((buff + n), (*(gps_dis + 9)), i); //��ת
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

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
        {
            if (key_data.key == key_esc) {
                dis_goto_standby();
            } else if (key_data.key == key_up) {
                menu_cnt.menu_other = mo_recorder_state;
                menu_cnt.menu_flag = true; //�����˵�����
            }
        }
    }
    //------------------------------------------//
}

static void mo_recorder_state_hdl(void) {
    uchar buff[20] = {0};
    uint i, n;

    //lcd_area_dis_set(6, 7, 0, 131, 0x00);    //�������һ��

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
    lcd_dis_one_line(0, 0, buff, false); //��һ����ʾ

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
    lcd_dis_one_line(1, 0, buff, false); //�ڶ�����ʾ
                                         //--------------------------------

    n = 0x00;
    i = strlen((char *)(*(recorder_lib_dis + 4)));
    memcpy(buff, (*(recorder_lib_dis + 4)), i); //����״̬
    n += i;
    if (1) //(!testbit(gps_data.car_state, 4))
    {
        i = strlen((char *)(*(state_lib_dis[4] + 0))); //��
        memcpy((buff + n), (*(state_lib_dis[4] + 0)), i);
    } else {
        i = strlen((char *)(*(state_lib_dis[4] + 1))); //��
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
    lcd_dis_one_line(2, 0, buff, false); //��������ʾ
                                         //--------------------------------

    n = 0x00;
    i = strlen((char *)(*(recorder_lib_dis + 6)));
    memcpy(buff, (*(recorder_lib_dis + 6)), i);
    n += i;

    if (pub_io.b.car_box == true) //��ʾ��
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
    lcd_dis_one_line(3, 0, buff, false); //��������ʾ

    lcd_dis_one_word(6, (8 * 15 + 4), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
        {
            if (key_data.key == key_esc) {
                dis_goto_standby();
            } else if (key_data.key == key_up) {
                menu_cnt.menu_other = mo_gps_infor1;
                menu_cnt.menu_flag = true; //�����˵�����
            }
        }
    }
    //------------------------------------------//
}

static void mo_check_state_hdl(void) {
    ;

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (!key_data.lg) //������������
        {
            if (key_data.key == key_ok) //��ת����ӡ����
            {
                /*
                print_data.step = step_req;  //������ӡ����

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8*)(&mw_dis)), '\x0', (sizeof(mw_dis)));

                mw_dis.p_dis_2ln = ((u8*)(*(menu_other_lib_dis+17)));
                mw_dis.bg_2ln_pixel = (8*1);
                */

                slave_send_msg((0x220000), 0x00, 0x00, false, spi1_up_comm_team_max);
                dis_goto_standby();
            } else if (key_data.key == key_esc) //�˳�
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

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (!key_data.lg) //������������
        {
            if (key_data.key == key_ok) //�����ɹ�
            {
                spi_flash_erase(flash_parameter_addr); //������������
                t_rst.rst = true;
                t_rst.del = false;
                t_rst.send = false;
                t_rst.jiff = jiffies;
            } else if (key_data.key == key_esc) //�˳�
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

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
        {
            if (key_data.key == key_esc) //�˳����浽��һ������˵�
            {
                _memset((u8 *)(&menu_auto_exit), 0x00, (sizeof(menu_auto_exit)));

                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
            if (key_data.key == key_ok) //ȷ��
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

//ͨ�Ų�ѯ����
static void mo_1st_6th_1th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;
#if 0 //chwsh
        menu_cnt.menu_other = mo_reset;
        menu_cnt.menu_3rd = 1;	 //������һ���˵�
#else
        if (/*(!sms_phone_save) && */ (input_method.cnt == (*((u16 *)(&password_manage.para))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.para)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //������һ���˵�
        } else                     //�����������
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(2000, true); //�˻���һ���˵�
        }
#endif
        menu_cnt.menu_flag = true;
    }
}

//ͨ�Ų�����������
static void mo_1st_5th_1st_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;
#if 0 //chwsh
        menu_cnt.menu_other = mo_reset;
        menu_cnt.menu_3rd = 1;	 //������һ���˵�
#else
        if (/*(!sms_phone_save) && */ (input_method.cnt == (*((u16 *)(&password_manage.para))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.para)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //������һ���˵�
        } else                     //�����������
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(2000, true); //�˻���һ���˵�
        }
#endif
        menu_cnt.menu_flag = true;
    }
}

//��·��������
static void mo_1st_5th_4th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.oil))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.oil)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //������һ���˵�
        } else                     //�����������
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(2000, true); //�˻���һ���˵�
        }

        menu_cnt.menu_flag = true;
    }
}

//������������
static void mo_1st_5th_6th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.car))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.car)))) == 0)) {
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //������һ���˵�
        } else                     //�����������
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(2000, true); //�˻���һ���˵�
        }

        menu_cnt.menu_flag = true;
    }
}

//����������ʼ������
static void mo_1st_5th_8th_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.para))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.para)))) == 0)) {
            menu_cnt.menu_other = mo_lcd_para_rst;
        } else //�����������
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(2000, true); //�˻���һ���˵�
        }

        menu_cnt.menu_flag = true;
    }
}

/*
//ͨ�Ų�����������
static void mo_1st_5th_9th_password_hdl(void)
{
	if (input_method_hdl())
	{
		if (!(input_method.cnt)) //���Ȳ�Ϊ0
			return;

		if ((!sms_phone_save)&&(input_method.cnt == (*((u16*)(&password_manage.regist))))
				&&(memcmp((input_method.buff), (input_method.ptr_dest), (*((u16*)(&password_manage.regist)))) == 0))
		{
			menu_cnt.menu_other = mo_reset;
			menu_cnt.menu_3rd = 1;	 //������һ���˵�
		}
		else	 //�����������
		{
			menu_cnt.menu_other = mo_message_window;
			_memset(((u8*)(&mw_dis)), '\x0', (sizeof(mw_dis)));
			mw_dis.p_dis_2ln = ((u8*)(*(menu_other_lib_dis+1)));
			mw_dis.bg_2ln_pixel = (8*2);		//��Ϣ��ʾ��������

			menu_auto_exit_set(2000, true);	//�˻���һ���˵�
		}

		menu_cnt.menu_flag = true;
	}
}
*/

//������ϵ������
static void mo_1st_1st_1st_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if ((input_method.cnt == (*((u16 *)(&password_manage.sensor))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.sensor)))) == 0)) {
            //  menu_cnt.menu_other = mo_1st_1st_1st_1st;
            //    input_method_init(((*(menu_1st_1st_dis + 0)) + 2), (recorder_para.ratio + 2), (*((u16 *)(recorder_para.ratio))), rec_para_ratio_max_lgth, false);
            //    menu_cnt.menu_flag = true;  //��ʾ����  //���봫����ϵ�����ý���
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 0x01; //������һ���˵�

        } else //�����������
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(2000, true); //�˻���һ���˵�
        }

        menu_cnt.menu_flag = true;
    }
}

//�ٶ�ģʽ����//220828�ٶ�����:
static void mo_1st_1st_2nd_password_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) { //���Ȳ�Ϊ0
            return;
        }

        if ((input_method.cnt == (*((u16 *)(&password_manage.speed))))
            && (memcmp((input_method.buff), (input_method.ptr_dest), (*((u16 *)(&password_manage.speed)))) == 0)) {
            logd("password of speed type, enter menu ***************************************");
            menu_cnt.menu_other = mo_reset;
            menu_cnt.menu_3rd = 1; //������һ���˵�
        } else {                   //�����������
            loge("password of speed, password err ***************************************");
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 1)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(2000, true); //�˻���һ���˵�
        }

        menu_cnt.menu_flag = true;
    }
}

//�ϵ��ж������Ƿ��޸Ĺ�
void pwr_up_password_status(void) {
    //mc_password_change = false;
}

static void mo_password_change_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //���泤��
        _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //��������
        flash25_program_auto_save((flash_parameter_addr + flash_password_addr), ((u8 *)(&password_manage)), (sizeof(password_manage)));

        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
        mw_dis.bg_2ln_pixel = (8 * 4); //��Ϣ��ʾ��������

        menu_auto_exit_set(2000, true); //�˻���һ���˵�
        menu_cnt.menu_flag = true;
        pwr_up_password_status();
    }
}

//������IP����������
static void mo_1st_5th_1st_1st_hdl(void) {
    u8 tp1;
    u8 set_ok;
    u8 i;
    u8 b_ipBuf[para_backup_ip_max_lgth + 2];

    _strcpy(b_ipBuf, set_para.b_ip);
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
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
                    set_ok = 1; //���õĸ�ʽ��ȷ
                }
            }
        }

        if (set_ok) //���óɹ�
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //���泤��
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //��������
            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

            if ((input_method.ptr_dest) == (set_para.m_ip + 2)) //��IP����
            {
                slave_send_msg((0x030000), 0x00, 0x00, false, spi1_up_comm_team_max); //������IP��ַ
            } else                                                                    //����IP����
            {
                tr9_6044_task();                                                      //RK��ƽ̨��ַ�ϴ�
                slave_send_msg((0x410000), 0x00, 0x00, false, spi1_up_comm_team_max); //���ñ���IP��ַ
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //��������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
        } else //��ʽ����
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;
        }
    }
}

//����ID����ŷ����������
static void mo_1st_5th_1st_3rd_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if (!check_num_ascii_error((input_method.buff), (input_method.cnt))) //���óɹ�
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //���泤��
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //��������
            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

            if ((input_method.ptr_dest) == (set_para.id + 2)) //����ID����
            {
                slave_send_msg((0x040000), 0x00, 0x00, false, spi1_up_comm_team_max); //���ñ���ID
            } else {
                slave_send_msg((0x050000), 0x00, 0x00, false, spi1_up_comm_team_max); //���ö��ŷ������
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //��������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
        } else //���ø�ʽ����
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;
        }
    }
}

//APN����
static void mo_1st_5th_1st_4th_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //���泤��
        _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //��������
        flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

        slave_send_msg((0x240000), 0x00, 0x00, false, spi1_up_comm_team_max); //����APN

        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
        mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //��������  ����ɹ���ʧ��
        menu_cnt.menu_flag = true;
    }
}

//ע������ز�����������
static void mo_1st_5th_9th_1st_hdl(void) {
    u8 set_ok;

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        set_ok = 0x00;

        if (((input_method.ptr_dest) == (recorder_para.manufacturer_id + 2))   //������ID
            || ((input_method.ptr_dest) == (recorder_para.terminal_type + 2))) //�ն��ͺ�
        {
            set_ok = 0x01;
        } else if ((input_method.cnt) == (input_method.lgth)) //���ȱ������Ҫ��
        {
            if (((input_method.ptr_dest) == (recorder_para.province_id + 2)) //ʡ��ID
                || ((input_method.ptr_dest) == (recorder_para.city_id + 2))) //������ID
            {
                if (!check_num_ascii_error((input_method.buff), (input_method.cnt))) {
                    set_ok = 0x01; //����Ϊ���ָ�ʽ
                }
            } else {
                set_ok = 0x01;
            }
        }

        if (set_ok) //���óɹ�
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //���泤��
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //��������
            flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), ((u8 *)(&recorder_para)), (sizeof(recorder_para)));

            if ((input_method.ptr_dest) == (recorder_para.province_id + 2)) //ʡ��ID
            {
                slave_send_msg((0x600000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.city_id + 2)) //����ID
            {
                slave_send_msg((0x610000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.terminal_id + 2)) //�ն�ID
            {
                slave_send_msg((0x630000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.car_vin + 2)) //����VIN
            {
                slave_send_msg((0x1c0000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else if ((input_method.ptr_dest) == (recorder_para.manufacturer_id + 2)) //������ID
            {
                slave_send_msg((0x270000), 0x01, 0x00, false, spi1_up_comm_team_max);
            } else //�ն��ͺ�
            {
                slave_send_msg((0x620000), 0x01, 0x00, false, spi1_up_comm_team_max);
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
        } else //���ø�ʽ����
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;
        }
    }
}

//ע������ز������ƺ�������
static void mo_1st_5th_9th_7th_hdl(void) {
    if (input_method.cnt < 2) //����������
    {
        input_method.input_lib.page_max = area_short_input_page_max;
        input_method.input_lib.sig_size = area_short_input_size;
        input_method.input_lib.ptr_lib = area_short_name;
    } else //����ASCII��
    {
        input_method.input_lib.page_max = ascii_input_page_max;
        input_method.input_lib.sig_size = ascii_input_size;
        input_method.input_lib.ptr_lib = ascii_char_table;
    }

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if (input_method.cnt == rec_car_plate_max_lgth) //���óɹ�
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt;              //���泤��
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //��������
            flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), (u8 *)(&recorder_para), (sizeof(recorder_para)));

            slave_send_msg((0x1d0000), 0x01, 0x00, false, spi1_up_comm_team_max); //���ó��ƺ���

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
        } else //���ø�ʽ����
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;
        }
    }
}

//������ϵ��
static void mo_1st_1st_1st_1st_hdl(void) {
    u32 n;

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        n = data_comm_hex((input_method.buff), (input_method.cnt));

        factory_para.set_speed_plus = n;
        menu_cnt.menu_other = mo_message_window;

        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
        mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
        menu_cnt.menu_flag = true;
        menu_report.ok_flag = true;

#if 0
        if (!check_num_ascii_error((input_method.buff), (input_method.cnt))
                && (n <= 65535))  //���óɹ�
        {
            *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt; //���泤��
            _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt);  //��������
            flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), ((u8 *)(&recorder_para)), (sizeof(recorder_para)));

            slave_send_msg((0x190000), 0x01, 0x00, false, spi1_up_comm_team_max); //���ô�����ϵ��

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1);    //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
        }
        else  //���ø�ʽ����
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2);    //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true);   //�˻���һ���˵�
            menu_cnt.menu_flag = true;
        }
#endif
    }
}

//��ʻԱ�������
static void mo_1st_1st_5th_1st_hdl(void) {
    u8 set_ok;

    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        set_ok = 0x01;
        if (input_method.ptr_dest == ic_card.driv_lic) {
            if ((input_method.cnt) != 18) //��ʻ֤���볤�ȱ���Ϊ18�ֽ�
            {
                set_ok = 0x00;
            }
        }

        if (set_ok) {
            if (input_method.ptr_dest == ic_card.work_num) //��ʻԱ���뼴����
            {
                ic_card.work_num_lgth = input_method.cnt;
                _memcpy((ic_card.work_num), (input_method.buff), (input_method.cnt));

                up_packeg((0x1f0000), (ic_card.work_num), ic_card.work_num_lgth);
                //slave_send_msg((0x1f0000),0x01,0x00,false,spi1_up_comm_team_max);
            } else //��ʻ֤����
            {
                ic_card.driv_lic_lgth = input_method.cnt;
                _memcpy((ic_card.driv_lic), (input_method.buff), (input_method.cnt));
                up_packeg((0x200000), (ic_card.driv_lic), ic_card.driv_lic_lgth);
                //slave_send_msg((0x200000),0x01,0x00,false,spi1_up_comm_team_max);
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
            mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
        } else //���ø�ʽ����
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
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
    case 1: //����IP���˿ں�
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

    case 3: //����ID    ���ŷ������
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 3)), false);
        lcd_dis_one_line(1, 0, (query.host.id_num + 1), false);
        lcd_dis_one_line(2, 0, (*(host_state_lib_dis + 4)), false);
        lcd_dis_one_line(3, 0, (query.host.sms_num + 1), false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 4: //�����ϴ���ʽ    �ϴ����
        //lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 5)), false);
        //lcd_dis_one_line(1, 0, *(transmit_mode_lib_dis + (query.host.transmit_mode - 0x80)), false);
        //lcd_dis_one_line(2, 0, (*(host_state_lib_dis + 6)), false); //���ͼ����

        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 5)), false);

        lcd_area_dis_set(4, 7, 0, 131, 0x00);
        _sprintf_len((char *)query.host.ip3, "����");
        query.host.ip3_lgth = 4;
        lcd_dis_one_page(16, query.host.ip3_lgth, query.host.ip3);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 5: //���ٱ���    �ϵ籨��
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

    case 6: //ͣ������    //�û�����
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
        //IAP�汾
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 11)), false);

#if (TR_USING_IAP)
#if (1) //IAP�汾
        if (str_visible(TR9_IAP_VERSION, TR9_SOFT_VERSION_MAX_LEN) == 0) {
            //��������չЭ��汾����ʾ
            lcd_dis_one_page(lcd_dis_one_page_line(1), strlen(TR9_IAP_VERSION), (unsigned char *)TR9_IAP_VERSION);
        } else {
            lcd_dis_one_page(lcd_dis_one_page_line(1), 4, (unsigned char *)"null");
        }
#endif

#if (1) //IAP�汾-BUILD TIME
        char *iap_build_time = get_build_time_need_free(TR9_IAP_BUILD_TIME);
        if (iap_build_time != NULL) {
            //��������չЭ��汾����ʾ
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

#if (1) //��ʾ���¼�
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
#endif
        break;
    }
    case 8: {
#pragma region APP�汾
        //APP�汾
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 12)), false);

#if (1) //APP�汾
        if (str_visible(TR9_SOFT_VERSION_APP, TR9_SOFT_VERSION_MAX_LEN) == 0) {
            //��������չЭ��汾����ʾ
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

#if (1) //APP�汾-BUILD TIME
        char *app_build_time = get_build_time_need_free(str_app_build_time);
        if (app_build_time != NULL) {
            //��������չЭ��汾����ʾ
            lcd_dis_one_page(lcd_dis_one_page_line(3), strlen(app_build_time), (unsigned char *)app_build_time);
            free(app_build_time);
        } else if (str_visible(str_app_build_time, 12) == 0) {
            //��������չЭ��汾����ʾ
            lcd_dis_one_page(lcd_dis_one_page_line(3), 12, (unsigned char *)str_app_build_time);
        } else {
            lcd_dis_one_page(lcd_dis_one_page_line(3), 4, (unsigned char *)"null");
        }
#endif

#if (0) //�汾
        char *pVersion = malloc(16 * 3);

        if (pVersion != NULL) {
#if (1) //APP�汾
            char *p = (char *)TR9_SOFT_VERSION_APP;
            p = "12345678876543211234567887654321";  //32//����
            p = "12345678876543211234567887654321*"; //33//����
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

            //��������չЭ��汾����ʾ
            lcd_dis_one_page(16, len, (unsigned char *)pVersion);
#endif //APP�汾

#if (0)
            //RK�汾
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

            lcd_dis_one_line(1, 0, buff, false); //��������׼Э��汾����ʾ
            lcd_area_dis_set(4, 7, 0, 131, 0x00);

            //	lcd_area_dis_set(0, 0, 6, 131, 0x00);
            //i = _strlen((u8 *)(query.host.ver_date2));
            //lcd_dis_one_page(32, i, query.host.ver_date2);  //��������չЭ��汾����ʾ
#endif
        }
#endif  //�汾
#if (1) //��ʾ����ҳ��ͷ��
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
#endif
#pragma endregion APP�汾
        break;
    }
    default:
        loge("err ln %d", cnt);
#if (1)
        //��IP���˿ں�
        cnt = 0; //��ֵ�ĵ���ȷ�ķ�Χ
        lcd_dis_one_line(0, 0, (*(host_state_lib_dis + 0)), false);

        lcd_area_dis_set(4, 7, 0, 131, 0x00);
        lcd_dis_one_page(16, query.host.ip_lgth, query.host.ip);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
#endif
        break;
    }

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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
                cnt = 0x00; //�˳�ʱ���õ���һҳ
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
    *(buff + (n++)) = ';'; //����
		logd("------test222---����,ic_card.name:[%s]-----", ic_card.name);

    i = _strlen((u8 *)(*(ic_card_dis + 1)));
    _memcpy((buff + n), (*(ic_card_dis + 1)), (i));
    n += i;
    _memcpy((buff + n), (ic_card.work_num), (ic_card.work_num_lgth));
    n += (ic_card.work_num_lgth);
    *(buff + (n++)) = ';'; //����
		logd("------test222---����,ic_card.work_num:[%s]-----", ic_card.work_num);

    i = _strlen((u8 *)(*(ic_card_dis + 2)));
    _memcpy((buff + n), (*(ic_card_dis + 2)), (i));
    n += i;
    _memcpy((buff + n), (ic_card.id_card), (ic_card.id_card_lgth));
    n += (ic_card.id_card_lgth);
    *(buff + (n++)) = ';'; //���֤	
		logd("------test222---��ʻ֤,ic_card.id_card:[%s]-----", ic_card.id_card);
#if (0) //��ʻ֤�ţ�
    i = _strlen((u8 *)(*(ic_card_dis + 3)));
    _memcpy((buff + n), (*(ic_card_dis + 3)), (i));
    n += i;
#endif
		
    _memcpy((buff + n), (ic_card.driv_lic), (ic_card.driv_lic_lgth));
    n += (ic_card.driv_lic_lgth);
    // *(buff + (n++)) = ' ';
		logd("------test222---��ic_card.driv_lic:[%s]-----", ic_card.driv_lic);

#if (0) //��Ч�ڣ�
    i = _strlen((u8 *)(*(ic_card_dis + 6)));
    _memcpy((buff + n), (*(ic_card_dis + 6)), (i));
    n += i;

    data_hex_to_ascii((buff + n), (ic_card.driv_lic_validity), 3);
    n += 6;

    *(buff + (n++)) = ';'; //��ʻ֤
#endif

    i = _strlen((u8 *)(*(ic_card_dis + 4)));
    _memcpy((buff + n), (*(ic_card_dis + 4)), (i));
    n += i;
    _memcpy((buff + n), (ic_card.certificate), (ic_card.certificate_lgth));
    n += (ic_card.certificate_lgth);
    *(buff + (n++)) = ';'; //��ҵ�ʸ�֤
		logd("------test222---��ҵ�ʸ�֤�ţ�ic_card.certificate:[%s]-----", ic_card.certificate);

#if (0) //��֤������
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
    *(buff + (n++)) = ';'; //��֤����

    dis_multi_page(buff, n, false, NULL);
}

/*
static void mo_slave_ver_check_hdl(void)
{
	static u8  cnt;
	inflash_para_struct  para;

	stm32_flash_read(boot_para_address, ((u8*)(&para)), (sizeof(para)));
	lcd_area_dis_set(6, 7, 0, 131, 0x00);

	if (cnt == 0x00) //��һҳ
	{
		lcd_dis_one_line(0, 0, (*(slave_ver_dis+0)), false);
		lcd_dis_one_page(16, para.bl_name_lgth, para.bl_buff);

		lcd_dis_one_word(6, (8*15), (*(up_down_flag+1)), lcd_dis_type_8x16ascii, false);
	}
	else  //�ڶ�ҳ
	{
		lcd_dis_one_line(0, 0, (*(slave_ver_dis+1)), false);
		lcd_dis_one_page(16, para.ap_name_lgth, para.ap_buff);

		lcd_dis_one_word(6, (8*15), (*(up_down_flag+0)), lcd_dis_type_8x16ascii, false);
	}

	//--------------   ��������   --------------//
	if (key_data.active)
	{
		if (key_data.lg) //������������
		{
			;
		}
		else   //�����̰�����
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
				cnt = 0x00;  //�˳�ʱ���õ���һҳ

				menu_cnt.menu_other = mo_reset;
				menu_cnt.menu_flag = true;
			}
		}
	}
	//------------------------------------------//
}
*/

//��ѯCSQ�ź�ֵ
static void mo_csq_vol_hdl(void) {
    u8 tmp;
    u8 buff[3];

    tmp = (gps_data.csq_state & 0x7f);
    if (tmp > 99) //��ֹ������Χ
    {
        tmp = 99;
    }

    buff[0] = tmp / 10 + '0';
    buff[1] = tmp % 10 + '0';
    buff[2] = 0x00; //���ý�����־  CKP  2015-03-30

    lcd_dis_one_line(0, 0, "CSQ:", false);
    lcd_dis_one_line(0, 8 * 5, buff, false);

    //--------------   ��������   --------------//

    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
        {
            if (key_data.key == key_esc) {
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
}

//�Լ���Ϣ
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

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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
                cnt = 0x00; //�˳�ʱ���õ���һҳ
                DIS_SELF_CHECK;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
    //------------------------------------------//
}

//��ʻ��¼�ǲ�����ѯ
//������
#define query_rec_wait_time_max ((u16)5000) //�����ȴ�ʱ��
static void mo_query_rec_hdl(void) {
    u8 buff[20], *ptr;

    switch (query.rec.step) {
        //-----------    ��ѯ����      ---------//

    case 0x00:                                                          //��ʾ��ѯ���沢��ʼ��ѯ��һ������  //������ϵ��
        lcd_dis_one_line(1, (8 * 1), *(menu_other_lib_dis + 4), false); //���ڲ�ѯ�����Ժ�

        slave_send_msg((0x190000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "4", false);
        query.rec.step = 0x01; //�ȴ�Ӧ��
        break;

    case 0x01: //�ȴ�
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x02: //�ٶ�ģʽ
        slave_send_msg((0x00210000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "3", false);
        query.rec.step = 0x03; //�ȴ�Ӧ��
        break;

    case 0x03: //�ȴ�
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x04: //��ӡ��׼ѡ��
        slave_send_msg((0x6a0000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "2", false);
        query.rec.step = 0x05; //�ȴ�Ӧ��
        break;

    case 0x05: //�ȴ�
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x06: //��������
        slave_send_msg((0x660000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.rec.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "1", false);
        query.rec.step = 0x07; //�ȴ�Ӧ��
        break;

    case 0x07: //�ȴ�
        if (_pasti(query.rec.tim) >= query_rec_wait_time_max) {
            query.rec.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

        //----------    ��ʾ����       ---------//

    case 0x08: //��ʾ��һ����
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

    case 0x21: //��ʾ�ڶ�����
        lcd_dis_one_line(0, 0, (menu_1st_1st_dis[2] + 2), false);

        lcd_dis_one_line(1, 0, (menu_1st_1st_3rd_dis[query.rec.print_type - 1] + 2), false);

        lcd_dis_one_line(2, 0, (menu_1st_1st_dis[3] + 2), false);

        _memcpy(buff, (query.rec.car_type), rec_car_type_max_lgth);
        buff[rec_car_type_max_lgth] = 0x00;
        lcd_dis_one_line(3, 0, (buff), false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
        break;

        //-----------    ��ʱ����    ---------//

    case 0xf0: //��ʱ����
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[1][0]));
        mw_dis.bg_2ln_pixel = (8 * 4); //��Ϣ��ʾ��������

        menu_auto_exit_set(2000, true); //�˻���һ���˵�
        menu_cnt.menu_flag = true;
        break;

    default:
        query.rec.step = 0x00;
    }

    if (query.rec.step < 0x20) //��ʼ��ʾ����ÿ�ζ�ˢ��
    {
        menu_cnt.menu_flag = true;
    } else //�����Ĵ���
    {
        if ((key_data.active) && (!key_data.lg)) //���ж̰���Ч
        {
            if (key_data.key == key_up) //�Ϸ�ҳ
            {
                if (query.rec.step == 0x21) {
                    query.rec.step = 0x20;
                }
            } else if (key_data.key == key_down) //�·�ҳ
            {
                if (query.rec.step == 0x20) {
                    query.rec.step = 0x21;
                }
            } else if (key_data.key == key_esc) //�˳�����
            {
                menu_cnt.menu_other = mo_reset;
            }

            menu_cnt.menu_flag = true;
        }
    }
}

//ע�������ѯ
//������
#define query_regist_wait_time_max ((u16)5000) //�����ȴ�ʱ��
static void mo_query_reg_hdl(void) {
    u8 buff[40];

    //    char* ptm_1;
    //    u8 len1;

    switch (query.regist.step) {
        //-----------	 ��ѯ����	 ---------//

    case 0x00:                                                          //��ʾ��ѯ���沢��ʼ��ѯ��һ������  //ʡ��ID
        lcd_dis_one_line(1, (8 * 1), *(menu_other_lib_dis + 4), false); //���ڲ�ѯ�����Ժ�

        slave_send_msg((0x600000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "8", false);
        query.regist.step = 0x01; //�ȴ�Ӧ��
        break;

    case 0x01: //�ȴ�
        if (sub_u16(jiffies, (query.regist.tim)) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x02: //������ID
        slave_send_msg((0x610000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "7", false);
        query.regist.step = 0x03; //�ȴ�Ӧ��
        break;

    case 0x03: //�ȴ�
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x04: //������ID
        slave_send_msg((0x270000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "6", false);
        query.regist.step = 0x05; //�ȴ�Ӧ��
        break;

    case 0x05: //�ȴ�
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x06: //�ն��ͺ�
        slave_send_msg((0x620000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "5", false);
        query.regist.step = 0x07; //�ȴ�Ӧ��
        break;

    case 0x07: //�ȴ�
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x08: //�ն�ID
        slave_send_msg((0x630000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "4", false);
        query.regist.step = 0x09; //�ȴ�Ӧ��
        break;

    case 0x09: //�ȴ�
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x0a: //������ɫ
        slave_send_msg((0x260000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "3", false);
        query.regist.step = 0x0b; //�ȴ�Ӧ��
        break;

    case 0x0b: //�ȴ�
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x0c: //���ƺ���
        slave_send_msg((0x1d0000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "2", false);
        query.regist.step = 0x0d; //�ȴ�Ӧ��
        break;

    case 0x0d: //�ȴ�
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x0e: //����VIN
        slave_send_msg((0x1c0000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.regist.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "1", false);
        query.regist.step = 0x0f; //�ȴ�Ӧ��
        break;

    case 0x0f: //�ȴ�
        if (_pasti(query.regist.tim) >= query_regist_wait_time_max) {
            query.regist.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

        //----------    ��ʾ����    ---------//

    case 0x10: //��һ��ʾ����  ʡ��ID��������ID
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

    case 0x21: //�ڶ���ʾ����  ������ID���ն�ID
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

    case 0x22: //������ʾ����  ������ɫ�����ƺ���
        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[5] + 2), false);

        lcd_dis_one_line(1, 0, (menu_1st_5th_9th_6th_dis[query.regist.car_plate_color - 1] + 2), false);

        lcd_dis_one_line(2, 0, (menu_1st_5th_9th_dis[6] + 2), false);

        _memcpy(buff, (query.regist.car_plate), rec_car_plate_max_lgth); //���ƺ���
        buff[rec_car_plate_max_lgth] = 0x00;
        lcd_dis_one_line(3, 0, buff, false);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 0x23: //������ʾ����  �ն��ͺ�
        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[3] + 2), false);
        lcd_area_dis_set(2, 7, 0, 131, 0x00);

        _memcpy(buff, (query.regist.terminal_type), rec_terminal_type_max_lgth);
        buff[rec_terminal_type_max_lgth] = 0x00;
        lcd_dis_one_page(16, rec_terminal_type_max_lgth, buff);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
        break;

    case 0x24: //������ʾ����  ����VIN��
        lcd_dis_one_line(0, 0, (menu_1st_5th_9th_dis[7] + 2), false);
        lcd_area_dis_set(2, 7, 0, 131, 0x00);

        _memcpy(buff, (query.regist.car_vin), rec_car_vin_max_lgth);
        buff[rec_car_vin_max_lgth] = 0x00;
        lcd_dis_one_page(16, rec_car_vin_max_lgth, buff);

        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
        break;

        //-----------    ��ʱ����    ---------//

    case 0xf0: //��ʱ����
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[1][0]));
        mw_dis.bg_2ln_pixel = (8 * 4); //��Ϣ��ʾ��������

        menu_auto_exit_set(2000, true); //�˻���һ���˵�
        menu_cnt.menu_flag = true;
        break;

    default:
        query.rec.step = 0x00;
    }

    if (query.regist.step < 0x20) //��ʼ��ʾ����ÿ�ζ�ˢ��
    {
        menu_cnt.menu_flag = true;
    } else //�����Ĵ���
    {
        if ((key_data.active) && (!key_data.lg)) //���ж̰���Ч
        {
            if (key_data.key == key_up) //�Ϸ�ҳ
            {
                if (query.regist.step > 0x20) {
                    query.regist.step -= 1;
                }
            } else if (key_data.key == key_down) //�·�ҳ
            {
                if (query.regist.step < 0x24) {
                    query.regist.step += 1;
                }
            } else if (key_data.key == key_esc) //�˳�����
            {
                menu_cnt.menu_other = mo_reset;
            }

            menu_cnt.menu_flag = true;
        }
    }
}

//��ʻԱ������ѯ
//������
#define query_driver_wait_time_max ((u16)5000) //�����ȴ�ʱ��
static void mo_query_driver_hdl(void) {
    u8 i, n;
    u8 buff[65];

    switch (query.driver.step) {
        //-----------    ��ѯ����    ---------//

    case 0x00:                             //��ʾ��ѯ���沢��ʼ��ѯ��һ������  //��ʻԱ�����ѯ
        if (state_data.state.flag.ic_flag) //��IC���Ѿ�����
        {
            query.driver.step = 0x20;
            menu_cnt.menu_flag = true;
        } else //��δ����IC��������Ҫ��������ѯ
        {
            lcd_dis_one_line(1, (8 * 1), *(menu_other_lib_dis + 4), false); //���ڲ�ѯ�����Ժ�

            slave_send_msg((0x1f0000), 0x00, 0x00, false, spi1_up_comm_team_max);
            query.driver.tim = jiffies; //ʱ��ͬ��

            lcd_dis_one_line(3, 0, "2", false);
            query.driver.step = 0x01; //�ȴ�Ӧ��
        }
        break;

    case 0x01: //�ȴ�
        if (sub_u16(jiffies, (query.driver.tim)) >= query_driver_wait_time_max) {
            query.driver.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x02: //��ʻ֤�����ѯ
        slave_send_msg((0x200000), 0x00, 0x00, false, spi1_up_comm_team_max);
        query.driver.tim = jiffies; //ʱ��ͬ��

        lcd_dis_one_line(3, 0, "1", false);
        query.driver.step = 0x03; //�ȴ�Ӧ��
        break;

    case 0x03: //�ȴ�
        if (_pasti(query.driver.tim) >= query_driver_wait_time_max) {
            query.driver.step = 0xf0; //��ʱ����
            menu_cnt.menu_flag = true;
        }
        break;

        //----------    ��ʾ����    ---------//

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
        *(buff + (n++)) = ';'; //��ʻԱ����

        i = str_len((u8 *)(*(menu_1st_1st_5th_dis + 1)), 16);
        i -= 2;
        _memcpy((buff + n), ((*(menu_1st_1st_5th_dis + 1)) + 2), (i));
        n += i;
        *(buff + (n++)) = ':';
        _memcpy((buff + n), (ic_card.driv_lic), (ic_card.driv_lic_lgth));
        n += (ic_card.driv_lic_lgth);
        *(buff + (n++)) = ';'; //��ʻԱ����

        lcd_area_dis_set(4, 7, 0, 131, 0x00); //����������
        lcd_dis_one_page(0x00, n, buff);
        break;

        //-----------    ��ʱ����    ---------//

    case 0xf0: //��ʱ����
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[1][0]));
        mw_dis.bg_2ln_pixel = (8 * 4); //��Ϣ��ʾ��������

        menu_auto_exit_set(2000, true); //�˻���һ���˵�
        menu_cnt.menu_flag = true;
        break;

    default:
        query.driver.step = 0x00;
    }

    if (query.driver.step < 0x20) //��ʼ��ʾ����ÿ�ζ�ˢ��
    {
        menu_cnt.menu_flag = true;
    } else //�����Ĵ���  //�����˳�����Ч
    {
        if ((key_data.active) && (!key_data.lg)) //���ж̰���Ч
        {
            if (key_data.key == key_esc) //�˳���
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
/* �Զ��������ú��� 2017-12-6*/
static void mo_cfg_parameter_hdl(void) {
}

static void mo_read_rcd_data_hal(void) {
}

static void mo_auto_init_hal(void) {
}

//��ʱ��¼
static void mo_timeout_driving_hal(void) {
    static u8 cnt;
    u8 buff[32] = {0};
    //u16 i, len;
    u16 ret;
    bool flag = false;

    switch (cnt) {
    case 0: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
        if (fatigue_print.cell[0].write_ed != 0) {
            ret = _verify_time(&fatigue_print.cell[0].start);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "�޳�ʱ��¼", false);
                flag = true;
                break;
            }
            ret = _verify_time(&fatigue_print.cell[0].end);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "�޳�ʱ��¼", false);
                flag = true;
                break;
            }
            if (ret) {
                lcd_dis_one_line(0, 1, "��ʱ��¼һ:", false);
                lcd_dis_one_line(1, 1, "��ʻ֤��:", false);
                lcd_dis_one_line(2, 1, fatigue_print.cell[0].driver_lse, false);
                lcd_dis_one_line(3, 1, "��ʼ��ʻʱ��:", false);
            }
            lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 1)), lcd_dis_type_8x16ascii, false);
        } else {
            lcd_dis_one_line(0, 1, "�޳�ʱ��¼", false);
            flag = true;
        }
    } break;
    case 1: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00);
        _memset(buff, 0, 32);
        _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[0].start.year, fatigue_print.cell[0].start.month,
                     fatigue_print.cell[0].start.date, fatigue_print.cell[0].start.hour, fatigue_print.cell[0].start.min, fatigue_print.cell[0].start.sec);

        lcd_dis_one_line(0, 1, buff, false);
        lcd_dis_one_line(1, 1, "������ʻʱ��:", false);

        _memset(buff, 0x00, 32);
        _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[0].end.year, fatigue_print.cell[0].end.month,
                     fatigue_print.cell[0].end.date, fatigue_print.cell[0].end.hour, fatigue_print.cell[0].end.min, fatigue_print.cell[0].end.sec);
        lcd_dis_one_line(2, 1, buff, false);
        lcd_dis_one_line(3, 1, "��ʱ��¼��", false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);

    } break;
    case 2: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00);
        if (fatigue_print.cell[1].write_ed != 0) {
            ret = _verify_time(&fatigue_print.cell[1].start);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "��ʱ��¼��:�޼�¼", false);
                flag = true;
                break;
            }
            ret = _verify_time(&fatigue_print.cell[1].end);
            if (ret == false) {
                lcd_dis_one_line(0, 1, "��ʱ��¼��:�޼�¼", false);
                flag = true;
                break;
            }
            if (ret) {
                lcd_dis_one_line(0, 1, "��ʻ֤��:", false);
                lcd_dis_one_line(1, 1, fatigue_print.cell[1].driver_lse, false);
                lcd_dis_one_line(2, 1, "��ʼ��ʻʱ��:", false);

                _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[1].start.year, fatigue_print.cell[1].start.month,
                             fatigue_print.cell[1].start.date, fatigue_print.cell[1].start.hour, fatigue_print.cell[1].start.min, fatigue_print.cell[1].start.sec);
                lcd_dis_one_line(3, 1, buff, false);
                lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 2)), lcd_dis_type_8x16ascii, false);
            }
        } else {
            lcd_dis_one_line(0, 1, "��ʱ��¼��:�޼�¼", false);
            flag = true;
            break;
        }

    } break;
    case 3: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00);
        lcd_dis_one_line(1, 1, "������ʻʱ��:", false);

        _sprintf_len((char *)buff, "20%02d-%02d-%02d_%02d:%02d:%02d", fatigue_print.cell[1].end.year, fatigue_print.cell[1].end.month,
                     fatigue_print.cell[1].end.date, fatigue_print.cell[1].end.hour, fatigue_print.cell[1].end.min, fatigue_print.cell[1].end.sec);
        lcd_dis_one_line(2, 1, buff, false);
        lcd_dis_one_word(6, (8 * 15), (*(up_down_flag + 0)), lcd_dis_type_8x16ascii, false);
    } break;
    default:
        break;
    }
    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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
                cnt = 0x00; //�˳�ʱ���õ���һҳ
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
}
//�ٶȼ�¼
static void mo_speed_record_hal(void) {
    static u8 cnt;
    u8 buff[70] = {0};
    u16 i, len = 0;
    //u16 ret;
    bool flag = false;

    switch (cnt) {
    case 0: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
        for (i = 0; i < 4; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_�ٶ�:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);
        lcd_dis_one_line(2, 1, buff + 32, false);
        lcd_dis_one_line(3, 1, buff + 48, false);
    } break;
    case 1: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
        for (i = 4; i < 8; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_�ٶ�:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);
        lcd_dis_one_line(2, 1, buff + 32, false);
        lcd_dis_one_line(3, 1, buff + 48, false);

    } break;
    case 2: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
        for (i = 8; i < 12; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_�ٶ�:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);
        lcd_dis_one_line(2, 1, buff + 32, false);
        lcd_dis_one_line(3, 1, buff + 48, false);
    } break;
    case 3: {
        lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
        for (i = 12; i < 15; i++) {
            _sprintf_len((char *)(buff + len), "%02d_%02d:%02d_�ٶ�:%02d", i, speed_print.HMt[i].hour, speed_print.HMt[i].minute, speed_print.HMt[i].speed);
            len += 16;
        }

        lcd_dis_one_line(0, 1, buff + 00, false);
        lcd_dis_one_line(1, 1, buff + 16, false);

    } break;
    default: break;
    }
    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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
                cnt = 0x00; //�˳�ʱ���õ���һҳ
                menu_cnt.menu_3rd = 0;
                menu_cnt.menu_other = mo_reset;
                menu_cnt.menu_flag = true;
            }
        }
    }
}

#if (0)
static void mo_sms_phone(void) {
    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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

//���������ʾ ������ʾ
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

    //--------------   ��������   --------------//
    if (key_data.active) {
        if (key_data.lg) //������������
        {
            ;
        } else //�����̰�����
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
    case call_free: //����״̬���������ص��˵�����
        menu_cnt.menu_other = mo_reset;
        menu_cnt.menu_flag = true;
        break;

    case call_dial: //�ֶ����Ŵ���
        if (input_method_hdl()) {
            if (!(input_method.cnt)) //���Ȳ�Ϊ0
                return;              //���ֶ�����ĺ���Ϊ0ʱ����

            if (check_num_ascii_error(input_method.buff, input_method.cnt)) //�����ʽ����
            {
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 30)));
                mw_dis.bg_2ln_pixel = (8 * 4); //��Ϣ��ʾ��������

                menu_auto_exit_set(1500, false); //�˻ش�������
            } else                               //��������
            {
                spi_flash_read(((u8 *)(&phbk_cnt)), phbk_start_addr_count, (sizeof(phbk_count_struct)));
                if ((phbk_cnt.total == 0x00) || (memcmp(&input_method.buff[0], "10086", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "10000", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "110", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "119", input_method.cnt) == 0x00) || (memcmp(&input_method.buff[0], "120", input_method.cnt) == 0x00)) {
                    /*
                        *(((u16 *)(input_method.ptr_dest)) - 1) = input_method.cnt; //���泤��
                        _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt);  //��������
                        */

                    *(input_method.ptr_dest) = input_method.cnt;                                 //���泤��
                    _memcpy((input_method.ptr_dest + 2), (input_method.buff), input_method.cnt); //��������

                    lcd_area_dis_set(6, 7, 0, 131, 0x00);
                    lcd_dis_one_line(0, 0, (*(dial_lib_dis + 1)), false);
                    phone_data.step = call_out;
                    phbk_call_in_data.num.lgth = phone_data.buff[0];
                    _memcpy(phbk_call_in_data.num.buff, (&phone_data.buff[2]), phbk_call_in_data.num.lgth);
                    write_data_to_call_record_flash(flash_call_out_record_addr, phbk_call_in_data);
                    // slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //֪ͨ��������
                    logd("������� %s", input_method.buff);
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

        //--------------   ��������   --------------//
        if (key_data.active) {
            if (!(key_data.lg)) //�����̰�
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

        //--------------   ��������   --------------//
        if (key_data.active) {
            if (!(key_data.lg)) //�����̰�
            {
                if (key_data.key == key_esc) {
                    phone_data.step = call_halt;
                    menu_cnt.menu_flag = true;
                } else if (key_data.key == key_ok) {
                    phone_data.dtmf_ptr = 0x00;
                    phone_data.dtmf_cnt = 0x00; //���������
                    phone_data.tim = jiffies;   //ʱ��ͬ��

                    phone_data.step = call_connect;
                    menu_cnt.menu_flag = true;

                    slave_send_msg((0x0e0000), 0x00, 0x00, false, spi1_up_comm_team_max); //֪ͨ�����ӵ绰
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

        //--------------   ��������   --------------//
        if (key_data.active) {
            if (!(key_data.lg)) //�����̰�
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
                    slave_send_msg((0x090000), 0x00, 0x00, false, spi1_up_comm_team_max); //����DTMF
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
        slave_send_msg((0x080000), 0x00, 0x00, false, spi1_up_comm_team_max); //֪ͨ�����һ�
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
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if (input_method.buff[0] == '1') //ֻ������1����Ч
        {
            slave_send_msg((0xee0301), 0x00, 0x00, false, spi1_up_comm_team_max); //�ϴ��¼�

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 6)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 4), rpt_average, true); //һ������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        } else //�������ݴ���
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        }
    }
}

static void mo_sms_vod_input_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if ((input_method.buff[0] == '1') || (input_method.buff[0] == '0')) //ֻ������1��0����Ч
        {
            slave_send_msg((0xee0303), 0x00, 0x00, false, spi1_up_comm_team_max); //��Ϣ�㲥

            if (input_method.buff[0] == '1') {
                sms_up_center.vod_status = 0x01; //ȷ�ϵ㲥
            } else {
                sms_up_center.vod_status = 0x00; //ȡ���㲥
            }

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 7)));
            mw_dis.bg_2ln_pixel = (8 * 0); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 4), rpt_average, true); //һ������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        } else //�������ݴ���
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;
        }
    }
}

static void mo_sms_ask_input_hdl(void) {
    if (input_method_hdl()) {
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
            return;

        if ((input_method.buff[0] >= '0') && (input_method.buff[0] < (0x30 + sms_union_data.ask.ans_cnt))) //ֻ�������������ڷ�Χ֮�ڲ���Ч
        {
            slave_send_msg((0xee0302), 0x00, 0x00, false, spi1_up_comm_team_max); //��Ϣ�㲥

            sms_up_center.ask_id_point = (input_method.buff[0] - 0x30); //�ύ�����

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 9)));
            mw_dis.bg_2ln_pixel = (8 * 0); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 4), rpt_average, true); //һ������  ����ɹ���ʧ��
            menu_cnt.menu_flag = true;
            menu_report.ok_flag = true;
        } else //�������ݴ���
        {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 3)));
            mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

            menu_auto_exit_set(1500, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;
        }
    }
}

static void mo_sms_display_hdl(void) {
    u8 buff[1200];
    u8 n, i;
    u16 lgth;

    switch (sms_up_center.sms_type) {
    case sms_type_fix: //�¼���Ϣ
        n = _strlen((u8 *)(*(sms_key_op_dis + 0)));
        _memcpy((sms_union_data.fix.dat + sms_union_data.fix.lgth), (*(sms_key_op_dis + 0)), n);

        dis_multi_page((sms_union_data.fix.dat), (sms_union_data.fix.lgth + n), true, sms_fix_input_set);
        break;

    case sms_type_ask:                                                        //������Ϣ
        _memcpy((buff), (sms_union_data.ask.dat), (sms_union_data.ask.lgth)); //��������
        lgth = (sms_union_data.ask.lgth);

        *(buff + (lgth++)) = '('; //������
        for (i = 0x00; i < (sms_union_data.ask.ans_cnt); i++) {
            *(sms_up_center.ask_ans + i) = ((sms_union_data.ask.answer + i)->ans_id); //װ�ش�ID��

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

    case sms_type_vod: //�㲥��Ϣ
        n = _strlen((u8 *)(*(sms_key_op_dis + 2)));
        _memcpy((sms_union_data.vod.dat + sms_union_data.vod.lgth), (*(sms_key_op_dis + 2)), n);

        dis_multi_page((sms_union_data.vod.dat), (sms_union_data.vod.lgth + n), true, sms_vod_input_set);
        break;

    case sms_type_serve: //������Ϣ
        dis_multi_page((sms_union_data.serve.dat), (sms_union_data.serve.lgth), false, NULL);
        break;

    case sms_type_center: //�ı���Ϣ
        dis_multi_page((sms_union_data.center.dat), (sms_union_data.center.lgth), false, NULL);
        break;

    case sms_type_phone: //�ֻ���Ϣ

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

    //--------------   ��������   --------------//
    if (key_data.lg) //������������
    {
        if (key_data.key == key_ok) //������ȷ����������
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
            slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //֪ͨ��������
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
    //��չ��·����uart3:Ĭ��Ϊ��6.���ص�����
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
//-------------------------------      �˵�������     -----------------------------------------//

//�˵���ʾ�绰��
static void menu_dis_phone_book(void) {
    u16 i, n;
    u8 buff[10];
    u8 index_buff[3][18];
    u8 *index_ptr[3];

    phbk_count_struct phbk_cnt;

    lcd_dis_one_line(0, (8 * 0), ((menu_1st_dis[3]) + 2), false);
    for (i = 0x00; i < 3; i++) //Ŀ¼��ָ�븳ֵ
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

//�˵���ʾ��Ϣ
static void menu_dis_sms(void) {
    u8 i, n;
    u8 buff[10];
    u8 cnt; //��һ�е����

    u8 index_buff[3][18];
    u8 *index_ptr[3];

    //    sms_index_struct  sms_index;

    lcd_dis_one_line(0, (8 * 0), ((menu_1st_2nd_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
    for (i = 0x00; i < 3; i++) //Ŀ¼��ָ�븳ֵ
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

//�˵���ʾͨ����¼
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

    for (i = 0x00; i < 3; i++) //Ŀ¼��ָ�븳ֵ
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

//menu��key����ͬ���ģ����Ǳ������ü�����ܽ����ü�����ȥ����
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
    case 0x01: //��¼������
        menu_move_display((menu_1st_1st_dis + i), i, menu_cnt.menu_2nd, menu_1st_1st_length);
        break;

    case 0x02: //��Ϣ
        menu_move_display((menu_1st_2nd_dis + i), i, menu_cnt.menu_2nd, menu_1st_2nd_length);
        break;

    case 0x03: //ͨ����¼
        menu_move_display((menu_1st_3rd_dis + i), i, menu_cnt.menu_2nd, menu_1st_3rd_length);
        break;

    case 0x04: //�绰��
               //menu_move_display((menu_1st_4rd_dis + i), i, menu_cnt.menu_2nd, menu_1st_4rd_length);
        menu_dis_phone_book();
        break;

    case 0x05: //��������
        menu_move_display((menu_1st_5th_dis + i), i, menu_cnt.menu_2nd, menu_1st_5th_length);
        break;

    case 0x06: //��ѯ
        menu_move_display((menu_1st_6th_dis + i), i, menu_cnt.menu_2nd, menu_1st_6th_length);
        break;

    case 0x07: //�߼�ѡ��
               //COMT:��ʾ
        menu_move_display((menu_1st_7th_dis + i), i, menu_cnt.menu_2nd, menu_1st_7th_length);
        break;

    case 0x08: //�Աȶ�����
        menu_move_display((menu_1st_8th_dis + i), i, menu_cnt.menu_2nd, menu_1st_8th_length);
        break;
    case 0x09: //����绰
        menu_move_display((menu_1st_9th_dis + i), i, menu_cnt.menu_2nd, menu_1st_9th_length);
        break;
    default:
        menu_cnt.menu_1st = 0x01; //ǿ���޸ĵ���һ��˵�
    }
}

static void menu_3rd_hdl(void) {
    u16 i;
    //    u16 da;
    //    u8 temp[8];
    switch (menu_cnt.menu_1st) {
    case 0x01: //��¼������
        i = _strlen((u8 *)(menu_1st_1st_dis[(menu_cnt.menu_2nd - 1)]));
        lcd_dis_one_line(0, (8 * 0), ((menu_1st_1st_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
        lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

        i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);

        switch (menu_cnt.menu_2nd) {
        case 0x01: //������ϵ��
            menu_move_display((menu_1st_1st_1nd_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_1nd_length);
            break;
        case 0x02: //220828�ٶ�����://�ٶ�ģʽ����ʾ���������ٶȡ�CAN�ٶȡ�GPS�ٶȡ��޸�����
            menu_move_display((menu_1st_1st_2nd_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_2nd_length);
            break;

        case 0x03: //��ӡ��׼����
            menu_move_display((menu_1st_1st_3rd_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_3rd_length);
            break;

        case 0x04: //�������Ʒ���
            menu_move_display((menu_1st_1st_4th_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_4th_length);
            break;

        case 0x05: //��ʻԱ����
            menu_move_display((menu_1st_1st_5th_dis + i), i, menu_cnt.menu_3rd, menu_1st_1st_5th_length);
            break;

        default:;
        }
        break;

    case 0x02: //��Ϣ
        menu_dis_sms();
        break;

    case 0x03: //ͨ����¼
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

    case 0x05: //��������
        i = _strlen((u8 *)(menu_1st_5th_dis[(menu_cnt.menu_2nd - 1)]));
        lcd_dis_one_line(0, (8 * 0), ((menu_1st_5th_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
        lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

        i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);

        switch (menu_cnt.menu_2nd) {
        case 0x01: //ͨ�Ų�������
            menu_move_display((menu_1st_5th_1st_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_1st_length);
            break;

        case 0x02: //����ѡ��
            menu_move_display((menu_1st_5th_2nd_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_2nd_length);
            break;

        case 0x03: //��������
            menu_move_display((menu_1st_5th_3rd_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_3rd_length);
            break;

        case 0x04: //��·����
            menu_move_display((menu_1st_5th_4th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_4th_length);
            break;

        case 0x05: //�̼�ѡ��
            menu_move_display((menu_1st_5th_5th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_5th_length);
            break;

        case 0x06: //��������
            menu_move_display((menu_1st_5th_6th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_6th_length);
            break;

        case 0x07: //��λģʽ����
            menu_move_display((menu_1st_5th_7th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_7th_length);
            break;

        case 0x09: //ע���������
            menu_move_display((menu_1st_5th_9th_dis + i), i, menu_cnt.menu_3rd, menu_1st_5th_9th_length);
            break;
        case 0x0A: //IC��������2018-1-8
            menu_move_display((const u8 **)(menu_driver_name_dis + i), i, menu_cnt.menu_3rd, menu_driver_info_cnt);
            break;

        default:;
        }
        break;
    case 0x06: //������ѯ

        break;
    case 0x07: //�߼�ѡ��//RST-RK-REF:
        i = _strlen((u8 *)(menu_1st_7th_dis[(menu_cnt.menu_2nd - 1)]));
        lcd_dis_one_line(0, (8 * 0), ((menu_1st_7th_dis[(menu_cnt.menu_2nd - 1)]) + 2), false);
        lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

        i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_3rd);

        switch (menu_cnt.menu_2nd) {
        case 0x01: //�ն˿���
            menu_move_display((menu_1st_7th_1st_dis_item + i), i, menu_cnt.menu_3rd, menu_count(enum_terminal_ctrl_cnt));
            break;

        case 0x03: //WIFI�ȵ�
            menu_move_display((menu_1st_7th_6th_1th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_6th_1th_length);
            break;
            /*
				case 0x03:   //WIFI�ȵ�
					   //���������� 
                    menu_move_display((menu_1st_7th_3th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_3th_length);
				    da = (gps_data.oil_data[0] >> 4) * 1000 + (gps_data.oil_data[0] & 0x0f) * 100 ;
				    da += (gps_data.oil_data[1] >> 4) * 10 + (gps_data.oil_data[1] & 0x0f);

				    data_mask_ascii(temp, da, 4);
					_memcpy( (u8*)&temp[5], "mV", 2);
				    lcd_dis_one_line(4, (3 * 8), data_check_valid(temp), false);		
				*/

        case 0x06: //��������
            menu_move_display((menu_1st_7th_6th_2th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_6th_2th_length);
            break;
        case 0x07: //��������//RST-RK-REF:
            menu_move_display((menu_1st_7th_6th_3th_dis + i), i, menu_cnt.menu_3rd, menu_1st_7th_6th_3th_length);
            break;
        case 0x08: //��������
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
    case 0x05: //��������
        switch (menu_cnt.menu_2nd) {
        case 0x05: //�̼�ѡ��
            switch (menu_cnt.menu_3rd) {
            case 0x03: //��չ��·����usart0:
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

        case 0x09: //ע���������
            switch (menu_cnt.menu_3rd) {
            case 0x06: //������ɫ
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
    case 07: //�߼�ѡ��
        switch (menu_cnt.menu_2nd) {
        case 0x01: //�ն˿���
            switch (menu_cnt.menu_3rd) {
            case 0x07: //ϵͳ����
                       //RST-RK:ϵͳ�����˵���ʾ
#if (0)                //ϵͳ����
                i = _strlen((u8 *)(menu_1st_5th_9th_dis[5]));
                lcd_dis_one_line(0, (8 * 0), ((menu_1st_5th_9th_dis[5]) + 2), false);
                lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);

                i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_4th);
                menu_move_display((menu_1st_7th_1st_dis_val + i), i, menu_cnt.menu_4th, menu_1st_7th_1st_val_length);
#elif (1)
                if (1) { //��ʾ:ϵͳ����->
                    i = _strlen((u8 *)(menu_1st_7th_1st_dis_item[menu_line(enum_terminal_ctrl_foster_care)]));
                    lcd_dis_one_line(0, (8 * 0), ((menu_1st_7th_1st_dis_item[menu_line(enum_terminal_ctrl_foster_care)]) + 2), false); //ϵͳ����
                    lcd_dis_one_line(0, (8 * (i - 2)), menu_str, false);                                                               //->
                }

                //logd("move_first_line %d, %d", menu_cnt.last_start, menu_cnt.menu_4th);
                i = menu_move_first_line(menu_cnt.last_start, menu_cnt.menu_4th);
                menu_move_display((menu_1st_7th_1st_dis_val + i), i, menu_cnt.menu_4th, menu_1st_7th_1st_val_length);
#elif (1) //�ο�����չ��·���ڣ�����
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

    switch (menu_cnt.menu_1st) //��һ���˵�
    {
    case 0x05:                     //��������
        switch (menu_cnt.menu_2nd) //�ڶ����˵�
        {
        case 0x05:                     //�ڶ����˵�������: �̼�ѡ��
            switch (menu_cnt.menu_3rd) //�������˵�
            {
            case 0x03:                     //�������˵�������://��չ��·����usart0:
                switch (menu_cnt.menu_4th) //���ļ��˵�
                {
                case 0x01: //���ļ��˵���һ��: usart1
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

                case 0x02: //���ļ��˵��ڶ���:usart2
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
                if (1) { //ѡ��ͼ��'*'
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

//�Զ��˳�����Ĵ���
static void menu_auto_exit_hdl(void) {
    if (menu_auto_exit.auto_flag) {
        if (_pastn(menu_auto_exit.cnt) >= menu_auto_exit.lgth) {
            menu_auto_exit.auto_flag = false; //�Զ��˳������������

            if (menu_auto_exit.dir_flag) //�˳�����һ���˵�
            {
                menu_cnt.menu_other = mo_reset;
            } else //�˳�����������
            {
                dis_goto_standby();
            }
            _memset((u8 *)(&menu_auto_exit), 0x00, (sizeof(menu_auto_exit)));
            menu_cnt.menu_flag = true; //�����˵�����
        }
    }
}

//���ر��洦��
static void menu_report_hdl(void) {
    u8 flag;
    u16 n;

    if (menu_report.en_flag) {
        flag = 0x00;

        if (menu_report.ok_flag) //����ɹ�����
        {
            if (menu_report.rpt) {
                flag = 2;
            }
        } else if (_pastn(menu_report.cnt) >= menu_report.lgth) //��ʱ����ʧ�ܽ���
        {
            flag = 1;
        }

        if (flag) {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));

            mw_dis.p_dis_2ln = (u8 *)(menu_report.ptr[flag - 1]); //��ʾ������
            n = _strlen((u8 *)(menu_report.ptr[flag - 1]));
            mw_dis.bg_2ln_pixel = (((16 - n) / 2) * 8); //��Ϣ��ʾ��������

            menu_auto_exit_set(1000, true); //�˻���һ���˵�

            menu_cnt.menu_flag = true;   //�����˵�����
            menu_report.en_flag = false; //������洦�����
        }
    }
}

//***********************************************************************************************//
//***********************************************************************************************//

//***********************************************************************************************//
//***********************************************************************************************//
//--------------------------------         ����������        ----------------------------------//

//�绰����������
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
        if (!(input_method.cnt)) //���Ȳ�Ϊ0
                                 //return;

            if (input_method.ptr_dest == phone_number) {
                _memcpy((input_method.ptr_dest), (input_method.buff), input_method.cnt); //��������

                call_record.call_record_cnt++;
                call_record.call_record_buff[i].name.lgth = 6;
                //		call_record.call_record_buff[i].name.buff = "δ֪";
                _memcpy_len(call_record.call_record_buff[i].name.buff, "δ֪", 4);
                call_record.call_record_buff[i].num.lgth = input_method.cnt;
                _memcpy_len(call_record.call_record_buff[i].num.buff, input_method.buff, input_method.cnt);
                call_record.call_record_buff[i].type = 1;

                flash25_program_auto_save(flash_call_out_record_addr, (u8 *)&call_record, 1);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 5), rpt_average, true); //��������  ����ɹ���ʧ��
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

//��Ϣ��������
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

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //������ȡ��Ϣ������־����Ϊ�Ѷ�״̬
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

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //������ȡ��Ϣ������־����Ϊ�Ѷ�״̬
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

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //������ȡ��Ϣ������־����Ϊ�Ѷ�״̬
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

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //������ȡ��Ϣ������־����Ϊ�Ѷ�״̬
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

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //������ȡ��Ϣ������־����Ϊ�Ѷ�״̬
            {
                ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) = false;
                flash25_program_auto_save(flash_sms_center_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));
            }

            dis_multi_page_cnt = 0x00;
            menu_cnt.menu_other = mo_sms_display;
            menu_cnt.menu_flag = true;
        }
        break;

    case 0x06: //�ֻ�����

        spi_flash_read(((u8 *)(&sms_index)), flash_sms_phone_addr, (sizeof(sms_index)));
        if (sms_index.total > sms_total_max)
            sms_index.total = sms_total_max;
        if (function_key_hdl(3, (sms_index.total)) == 0x00) {
            sms_up_center.sms_type = sms_type_phone;
            spi_flash_read(((u8 *)(&(sms_union_data.phone))), ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->addr), (sizeof(sms_phone_content_struct)));

            if ((sms_index.use_infor + menu_cnt.menu_3rd - 1)->read_flag) //������ȡ��Ϣ������־����Ϊ�Ѷ�״̬
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

//ͨ����¼��������
static void key_3rd_phone_records_hdl(void) {
    switch (menu_cnt.menu_2nd) {
    case 0x01: //�Ѳ��绰

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
            slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //֪ͨ��������
        }
        break;
    case 0x02: //�ѽӵ绰
    case 0x03: //δ�ӵ绰
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
                slave_send_msg((0x070000), 0x00, 0x00, false, spi1_up_comm_team_max); //֪ͨ��������
            }
        }

        break;

    default:
        break;
    }
}

//key������Ҫ�ǶԸü������еİ����������д����Լ��ǹ���˵�����ʾ����menu���ǶԸü�����Ĳ˵�������ʾ
//��һ���˵�
static void key_1st_hdl(void) {
    menu_cnt.next_regular = true;
    function_key_hdl(1, menu_1st_length);
}

//�ڶ����˵�
static void key_2nd_hdl(void) { //�����˵�������,��Ҫ�Ǵ�����
    //u8 i;
    //    u8 ret;
    switch (menu_cnt.menu_1st) {
    case 0x01: //��¼������
        if ((menu_cnt.menu_2nd == 3) || (menu_cnt.menu_2nd == 4)
            || (menu_cnt.menu_2nd == 5)) //����˵�
        {
            menu_cnt.next_regular = true;
        } else {
            menu_cnt.next_regular = false;
        }

        if (function_key_hdl(2, menu_1st_1st_length) == 0x00) {
            if (menu_cnt.menu_2nd == 1) { //������ϵ��
                menu_cnt.menu_other = mo_1st_1st_1st_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.sensor + 2), 0, password_sensor_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 2) { //�ٶ�ģʽ
                menu_cnt.menu_other = mo_1st_1st_2nd_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.speed + 2), 0, password_speed_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 6) { //������ʷ����
                ;                                //�������Σ��ݲ�ʵ��  CKP  20140219
            }
        }
        break;

    case 0x02: //��Ϣ
        menu_cnt.next_regular = true;
        function_key_hdl(2, menu_1st_2nd_length);
        break;

    case 0x03: //ͨ����¼
        menu_cnt.next_regular = true;
        if (menu_cnt.menu_2nd == 0x04) //�����ɾ�����м�¼ѡ������һ������Ϊ���������
        {
            menu_cnt.next_regular = false;
        }
        if (function_key_hdl(2, menu_1st_3rd_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x04:
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(menu_report_dis[4][1]));
                mw_dis.bg_2ln_pixel = (8 * 2); //��Ϣ��ʾ��������

                call_record_data_flash_init();  //ͨ����¼��ʼ��
                menu_auto_exit_set(2000, true); //�˻���һ���˵�
                menu_cnt.menu_flag = true;
                break;
            }
        }
        _memset((void *)&call_record, 0x00, sizeof(call_record_struct));
        break;

    case 0x04: //�绰��
        menu_cnt.next_regular = false;
        key_2nd_phone_book_hdl();
        break;

    case 0x05: //��������
        if ((menu_cnt.menu_2nd == 2) || (menu_cnt.menu_2nd == 3) || (menu_cnt.menu_2nd == 5)
            || (menu_cnt.menu_2nd == 7) || menu_cnt.menu_2nd == 10 /*IC����*/) //����˵�
        {
            menu_cnt.next_regular = true;
        } else {
            menu_cnt.next_regular = false;
        }

        if (function_key_hdl(2, menu_1st_5th_length) == 0x00) //��ǰ��������еİ������������ɴ˺�������
        {
            if (menu_cnt.menu_2nd == 1) //ͨ�Ų�������
            {
                menu_cnt.menu_other = mo_1st_5th_1st_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 4) //��·����
            {
                menu_cnt.menu_other = mo_1st_5th_4th_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.oil + 2), 0, password_oil_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 6) //��������
            {
                menu_cnt.menu_other = mo_1st_5th_6th_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.car + 2), 0, password_car_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 8) //����������ʼ��
            {
                if (sms_phone_save) //���ý�������������ʼ��
                    return;
                menu_cnt.menu_other = mo_1st_5th_8th_password;
                menu_cnt.menu_flag = true;

                //input_method_init(menu_password_dis, (password_manage.lcd_para_rst+2), 0, password_lcd_para_rst_max_lgth, true);
                input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
            } else if (menu_cnt.menu_2nd == 9) //ע���������
            {
                menu_cnt.menu_other = mo_1st_5th_1st_password;
                menu_cnt.menu_flag = true;

                input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
            }
        }
        break;

    case 0x06: //��ѯ

        menu_cnt.next_regular = false;

        if (function_key_hdl(2, menu_1st_6th_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x01: //ͨ�Ų�����ѯ
                       /*
                        menu_cnt.menu_other = mo_1st_6th_1st_password;
                        menu_cnt.menu_flag = true;
                        
                        input_method_init(menu_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, true);
			*/

                slave_send_msg((0x110000), 0x00, 0x00, false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 4)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 1), rpt_com, false); //��������  ������ʧ��
                menu_cnt.menu_flag = true;

                break;

            case 0x02:                                //ע�������ѯ
                lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
                menu_cnt.menu_other = mo_query_reg;

                query.regist.step = 0x00; //������ѯ����
                menu_cnt.menu_flag = true;
                break;

            case 0x03:                                //��¼�ǲ�����ѯ
                lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
                menu_cnt.menu_other = mo_query_rec;

                query.rec.step = 0x00; //������ѯ����
                menu_cnt.menu_flag = true;
                break;

            case 0x04:                                //��ʻԱ��Ϣ
                lcd_area_dis_set(0, 7, 0, 131, 0x00); //����
                menu_cnt.menu_other = mo_query_driver;

                query.driver.step = 0x00; //������ѯ��ʻԱ����
                menu_cnt.menu_flag = true;
                break;

            case 0x05: //IC����Ϣ
                if (state_data.state.flag.ic_flag) {
                    menu_cnt.menu_other = mo_ic_card_dis;
                    dis_multi_page_cnt = 0x00;
                } else //δ����IC��
                {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 31)));
                    mw_dis.bg_2ln_pixel = (8 * 0); //��Ϣ��ʾ��������

                    menu_auto_exit_set(1500, true); //�˻���һ���˵�
                }

                menu_cnt.menu_flag = true;
                break;

            case 0x06: //ƽ�����ټ�¼
                menu_cnt.menu_other = mo_speed_record;
                menu_cnt.menu_flag = true;
                break;

            case 0x07: //ƣ�ͼ�ʻ��¼
            {
                menu_cnt.menu_other = mo_timeout_driving;
                menu_cnt.menu_flag = true;
            } break;

                /*
                    case 0x08:  //����汾��ѯ
                    	menu_cnt.menu_other = mo_slave_ver_check;
                    	menu_cnt.menu_flag = true;
                    	break;
                    */

            case 0x08: //��ѯCSQ�ź�ֵ
                menu_cnt.menu_other = mo_csq_vol;
                menu_cnt.menu_flag = true;
                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                break;
            }
        }
        break;

    case 0x07:                                                                                                                                        //�߼�ѡ��
        if ((menu_cnt.menu_2nd == 1) || (menu_cnt.menu_2nd == 3) || (menu_cnt.menu_2nd == 6) || (menu_cnt.menu_2nd == 7) || (menu_cnt.menu_2nd == 8)) //����˵�
        {
            menu_cnt.next_regular = true;
        } else {
            menu_cnt.next_regular = false;
        }

        if (function_key_hdl(2, menu_1st_7th_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x02: //�����˵�
                slave_send_msg((0xee0701), 0x00, 0x00, false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 8)));
                mw_dis.bg_2ln_pixel = (8 * 0); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 4), rpt_average, false); //һ������  ������ʧ��
                menu_cnt.menu_flag = true;
                break;

            case 0x04: //�ն��Լ�
                slave_send_msg((0x0c0000), 0x00, 0x00, false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 5)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 4), rpt_com, false); //��������  ������ʧ��
                menu_cnt.menu_flag = true;
                break;

            case 0x05: //��������
                menu_cnt.menu_other = mo_oil_test;
                menu_cnt.menu_flag = true;

                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                lcd_dis_one_line(1, 0, oil_data_dis, false);
                break;
            default:;
            }
        }
        break;

    case 0x08:                         //�Աȶȵ���
        menu_cnt.next_regular = false; //�������˵�Ϊ������˵�����ʾ���óɹ�

        if (function_key_hdl(2, menu_1st_8th_length) == 0x00) {
            //�������õĶԱȶȲ���
            set_para.LCD_Contrast = lcd_contrast_table[menu_cnt.menu_2nd - 1];
            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

            //��ʾ���óɹ�
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
            mw_dis.bg_2ln_pixel = (8 * 3); //��Ϣ��ʾ��������

            menu_auto_exit_set(1000, true); //�˻���һ���˵�
            menu_cnt.menu_flag = true;

            //����LCD�Աȶ�
            //lcd_init(true);  /* ??????????? */
        }

        break;
    case 0x09: //��绰
        menu_cnt.next_regular = false;
        if (function_key_hdl(2, menu_1st_1st_5th_length) == 0x00) {
            switch (menu_cnt.menu_2nd) {
            case 0x01:
                phone_data.step = call_halt; //�绰�Ҷ�
                menu_cnt.menu_other = mo_telephone;
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0) + 1, rpt_average, true); //��������  ����ɹ���ʧ��
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
                break;
            case 0x02:
                phone_data.step = call_dial; //�ֶ�����
                menu_cnt.menu_other = mo_telephone;
                input_method_init(phoneDial_dis, (u8 *)phone_data.buff, 0, 11, false);
                break;
            }
        }
        break;
    default:
        menu_cnt.menu_1st = 0x01; //ǿ���޸ĵ���һ��˵�
    }
}

static void key_3rd_hdl(void) { //�����˵�����������
    u8 cun = 0;
    switch (menu_cnt.menu_1st) {
    case 0x01: //��¼������
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

        case 0x02: //220828�ٶ�����://�ٶ�ģʽ//ִ�ж���0x00210000
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_2nd_length) == 0x00) {
                enum_speed_mode_typedef enum_speed_mode = (enum_speed_mode_typedef)menu_cnt.menu_3rd;

                if (enum_speed_mode > enum_speed_mode_null && cun < enum_speed_mode_cnt) {
                    if (enum_speed_mode == enum_speed_mode_set_password) { //220828�ٶ�����://�޸�����//������������:
                        menu_cnt.menu_other = mo_password_change;
                        input_method_init(menu_new_password_dis, (password_manage.speed + 2), 0, password_speed_max_lgth, false);
                    } else {
                        slave_send_msg((0x00210000), 0x01, (menu_cnt.menu_3rd), false, spi1_up_comm_team_max);

                        //��Ϣ��ʾ��������
                        /*
                        menu_cnt.menu_other = mo_message_window;
                        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                        mw_dis.bg_2ln_pixel = (8 * 1);    //��Ϣ��ʾ��������

                        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                        menu_cnt.menu_flag = true;
                        */

                        sys_cfg.speed_mode = enum_speed_mode;
                    }
                }
            }
            break;

        case 0x03: //��ӡ��׼����
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
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��

                if ((menu_cnt.menu_3rd == 0x03) || (menu_cnt.menu_3rd == 0x04)) {
                    menu_report.ok_flag = true;
                    flash25_program_auto_save((flash_parameter_addr + flash_recorder_para_addr), ((u8 *)(&recorder_para)), (sizeof(recorder_para)));
                }

                menu_cnt.menu_flag = true;
            }
            break;

        case 0x04: //�������Ʒ���
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_4th_length) == 0x00) {
                slave_send_msg((0x660000), 0x01, (menu_cnt.menu_3rd), false, spi1_up_comm_team_max);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                menu_cnt.menu_flag = true;
            }
            break;

        case 0x05: //��ʻԱ����
            menu_cnt.next_regular = false;
            if (function_key_hdl(3, menu_1st_1st_5th_length) == 0x00) {
                if (state_data.state.flag.ic_flag) //IC���������޷��ֶ�����
                {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(menu_other_lib_dis[33]));
                    mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                    menu_auto_exit_set(2000, true); //�˻���һ���˵�
                } else {
                    menu_cnt.menu_other = mo_1st_1st_5th_1st;

                    if (menu_cnt.menu_3rd == 0x01) //��ʻԱ���뼴��������
                    {
                        input_method_init(((*(menu_1st_1st_5th_dis + 0)) + 2), (ic_card.work_num), (ic_card.work_num_lgth), 7, false);
                    } else //��ʻ֤��������
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

    case 0x02: //��Ϣ
        menu_cnt.next_regular = false;
        key_3rd_sms_hdl();
        break;

        //----------------------------------//

    case 0x03: //ͨ����¼
        menu_cnt.next_regular = false;
        key_3rd_phone_records_hdl();
        break;

        //----------------------------------//

    case 0x05: //��������
        menu_cnt.next_regular = false;
        switch (menu_cnt.menu_2nd) {
        case 0x01: //ͨ�Ų�������
            if (function_key_hdl(3, menu_1st_5th_1st_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //��IP����
                    menu_cnt.menu_other = mo_1st_5th_1st_1st;
                    input_method_init(((*(menu_1st_5th_1st_dis + 0)) + 2), (set_para.m_ip + 2), (*((u16 *)(set_para.m_ip))), para_main_ip_max_lgth, false);
                    break;

                case 0x02: //����IP����
                    menu_cnt.menu_other = mo_1st_5th_1st_1st;
                    input_method_init(((*(menu_1st_5th_1st_dis + 1)) + 2), (set_para.b_ip + 2), (*((u16 *)(set_para.b_ip))), para_backup_ip_max_lgth, false);
                    break;

                case 0x03: //����ID����
                    menu_cnt.menu_other = mo_1st_5th_1st_3rd;
                    input_method_init(((*(menu_1st_5th_1st_dis + 2)) + 2), (set_para.id + 2), (*((u16 *)(set_para.id))), para_id_max_lgth, false);
                    break;

                case 0x04: //APN����
                    menu_cnt.menu_other = mo_1st_5th_1st_4th;
                    input_method_init(((*(menu_1st_5th_1st_dis + 3)) + 2), (set_para.apn + 2), (*((u16 *)(set_para.apn))), para_apn_max_lgth, false);
                    break;

                case 0x05: //�����������
                    menu_cnt.menu_other = mo_1st_5th_1st_3rd;
                    input_method_init(((*(menu_1st_5th_1st_dis + 4)) + 2), (set_para.num_s + 2), (*((u16 *)(set_para.num_s))), para_num_server_max_lgth, false);
                    break;

                case 0x06: //������ʼ��
                    slave_send_msg((0x150000), 0x00, 0x00, false, spi1_up_comm_team_max);

                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 5)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //��������  ����ɹ���ʧ��
                    break;

                case 0x07: //�޸���������
                    menu_cnt.menu_other = mo_password_change;
                    input_method_init(menu_new_password_dis, (password_manage.para + 2), 0, password_para_max_lgth, false);
                    break;
                }

                menu_cnt.menu_flag = true;
            }
            break;

        case 0x02: //����ѡ��
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
                mw_dis.bg_2ln_pixel = (8 * 3); //��Ϣ��ʾ��������

                menu_auto_exit_set(1000, true); //�˻���һ���˵�
                menu_cnt.menu_flag = true;
            }
            break;

        case 0x03: //��������
            if (function_key_hdl(3, menu_1st_5th_3rd_length) == 0x00) {
                set_para.bl_type = lcd_bl_time_table[menu_cnt.menu_3rd - 1];

                flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 0) + 1)));
                mw_dis.bg_2ln_pixel = (8 * 3); //��Ϣ��ʾ��������

                menu_auto_exit_set(1000, true); //�˻���һ���˵�
                menu_cnt.menu_flag = true;
            }
            break;

        case 0x04: //��·����
            if (function_key_hdl(3, menu_1st_5th_4th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //�������Σ��ݲ�ʵ��  CKP  20140219
                    slave_send_msg((0x380000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x02: //�������Σ��ݲ�ʵ��  CKP  20140219
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
                    mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                    menu_cnt.menu_flag = true;
                }
            }
            break;

        case 0x05: //�̼�ѡ��
            if (function_key_hdl(3, menu_1st_5th_5th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //�ֿ����

#if 0
                                menu_cnt.menu_other = mo_update_font_lib;
                                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                                _memset(((u8 *)(&font_lib_update)), '\x0', (sizeof(font_lib_update)));

                                lcd_dis_one_line(1, (8 * 0), ((u8 *)(*(menu_other_lib_dis + 14))), false);
                                lcd_dis_at_once();  //������ʾ����
#endif
                    us.u1.reg = 0;
                    us.u1.b.tr9 = 1;
                    uart1_update_func(us.u1.reg, BY_SETUP);

                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                    menu_cnt.menu_flag = true;
                    menu_report.ok_flag = true;

                    //     menu_auto_exit_set(1000, true);   //�˻���һ���˵�

                    break;

                case 0x02: //�̼�����
#if 0
                                menu_cnt.menu_other = mo_update_tf_main;
                                lcd_area_dis_set(0, 7, 0, 131, 0x00);
                                _memset(((u8 *)(&update_tf_data)), '\x0', (sizeof(update_tf_data)));

                                lcd_dis_one_line(1, (8 * 0), ((u8 *)(*(menu_other_lib_dis + 14))), false);
                                lcd_dis_at_once();  //������ʾ����
                                menu_cnt.menu_flag = true;
#endif
                    menu_auto_exit_set(1000, true); //�˻���һ���˵�
                    menu_cnt.menu_flag = true;
                    break;

                case 0x03: //��չ��·����usart0:
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 4)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                    menu_report_set(6000, *(menu_report_dis + 1), rpt_com, false); //��������  ������ʧ��
                    menu_cnt.menu_flag = true;
                    slave_send_msg((0x250000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    _memset((u8 *)(&mult_usart_set), 0x00, sizeof(mult_usart_set));
                    break;
                }
            }
            break;

        case 0x06: //��������
            if (function_key_hdl(3, menu_1st_5th_6th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //�������Σ��ݲ�ʵ��  CKP  20140219
                    slave_send_msg((0x470000), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x02: //�������Σ��ݲ�ʵ��  CKP  20140219
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
                        mw_dis.bg_2ln_pixel = (8 * 1);		//��Ϣ��ʾ��������

                        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                        menu_cnt.menu_flag = true;
			*/
            }
            break;

        case 0x07: //��λģ��ģʽѡ��
            if (function_key_hdl(3, menu_1st_5th_7th_length) == 0x00) {
                host_no_save_para.set_gps_module_type = menu_cnt.menu_3rd;

                menu_cnt.menu_other = mo_message_window;

                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
            break;

        case 0x09: //ע���������
            if (menu_cnt.menu_3rd == 6) {
                menu_cnt.next_regular = true;
            } else {
                menu_cnt.next_regular = false;
            }

            if (function_key_hdl(3, menu_1st_5th_9th_length) == 0x00) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: //ʡ��ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 0)) + 2), (recorder_para.province_id + 2), (*((u16 *)(recorder_para.province_id))), rec_province_id_max_lgth, false);
                    break;

                case 0x02: //������ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 1)) + 2), (recorder_para.city_id + 2), (*((u16 *)(recorder_para.city_id))), rec_city_id_max_lgth, false);
                    break;

                case 0x03: //������ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 2)) + 2), (recorder_para.manufacturer_id + 2), (*((u16 *)(recorder_para.manufacturer_id))), rec_manufacturer_id_max_lgth, false);
                    break;

                case 0x04: //�ն��ͺ�
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 3)) + 2), (recorder_para.terminal_type + 2), (*((u16 *)(recorder_para.terminal_type))), rec_terminal_type_max_lgth, false);
                    break;

                case 0x05: //�ն�ID
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 4)) + 2), (recorder_para.terminal_id + 2), (*((u16 *)(recorder_para.terminal_id))), rec_terminal_id_max_lgth, false);
                    break;

                case 0x07: //���ƺ���
                    menu_cnt.menu_other = mo_1st_5th_9th_7th;
                    input_method_init(((*(menu_1st_5th_9th_dis + 6)) + 2), (recorder_para.car_plate + 2), (*((u16 *)(recorder_para.car_plate))), rec_car_plate_max_lgth, false);

                    input_method.input_lib.crisis = 2;
                    input_method.input_lib.front_crisis_size = 2; //���뷨�ᷢ���л�
                    break;

                case 0x08: //����VIN��
                    menu_cnt.menu_other = mo_1st_5th_9th_1st;
                    input_method_init(((*(menu_1st_5th_9th_dis + 7)) + 2), (recorder_para.car_vin + 2), (*((u16 *)(recorder_para.car_vin))), rec_car_vin_max_lgth, false);
                    break;

                    /*
                                case 0x09:  //�޸�����
                                	menu_cnt.menu_other = mo_password_change;
                                	input_method_init(menu_new_password_dis, (password_manage.regist+2), 0, password_regist_max_lgth, false);
                                	break;
                                */
                }

                menu_cnt.menu_flag = true; //��ʾҳ��ˢ��
            }
            break;
        case 0x0A: //IC����2018-1-8
            if (function_key_hdl(3, menu_driver_info_cnt) == 0x00) {
                if ((menu_driver_info_cnt > 0)) {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(*(menu_report_dis + 5) + 0)));
                    mw_dis.bg_2ln_pixel = (8 * 3); //��Ϣ��ʾ��������
                }
                menu_auto_exit_set(1000, true); //�˻���һ���˵�
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
        case 0x01: //�ն˿���
            slave_send_msg((0x110000), 0x00, 0x00, false, spi1_up_comm_team_max);

            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 4)));
            mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

            menu_report_set(6000, *(menu_report_dis + 1), rpt_com, false); //��������  ������ʧ��
            menu_cnt.menu_flag = true;
            break;

        case 0x09: {
            logd("���й���");
            break;
        }
        default:
            break;
        }
        break;
    case 0x07: //�߼�ѡ��
        menu_cnt.next_regular = false;
        switch (menu_cnt.menu_2nd) {
        case 0x01: //�ն˿���
#if (1)            //RST-RK:�ն˸�λ//û����Σ������˲˵���7.ϵͳ������
            if (menu_cnt.menu_3rd == 7) {
                menu_cnt.next_regular = true;
            } else {
                menu_cnt.next_regular = false;
            }
#endif
            if (function_key_hdl(3, menu_count(enum_terminal_ctrl_cnt)) == 0) {
                switch (menu_cnt.menu_3rd) {
                case 0x01: { //RST-RK:�ն˸�λ
#if (DEBUG_EN_WATCH_DOG)

                    log_write(event_mcu_boot_lcd_menu);

                    while (1) { ; }
#else
                    logf("ret mcu, undef");
                    break;
#endif
                }
                case 0x02: //ע��
                    slave_send_msg((0xee0100), 0x00, 0x00, false, spi1_up_comm_team_max);
                    break;

                case 0x03: //��Ȩ
                    slave_send_msg((0xee0102), 0x00, 0x00, false, spi1_up_comm_team_max);
                    menu_report.ok_flag = true;
                    break;

                case 0x04: //ע��
                    slave_send_msg((0xee0003), 0x00, 0x00, false, spi1_up_comm_team_max);
                    menu_report.ok_flag = true;
                    break;

                case 0x05: //����  //�������Σ��ݲ�ʵ��  CKP  20140219
                    break;

                case 0x06: //����  //�������Σ��ݲ�ʵ��  CKP  20140219
                    break;

                case 0x07: //RST-RK:ϵͳ����
                    break;

                default:;
                }

#if (1) //������:�����У����Ժ�//�ĸ���־��������ɹ���
                if ((menu_cnt.menu_3rd == 1) || (menu_cnt.menu_3rd == 2) || (menu_cnt.menu_3rd == 3) || (menu_cnt.menu_3rd == 4)) {
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                    menu_cnt.menu_flag = true;
                }
#endif
            }
            break;

        case 0x03: //�ȵ㹦��
            if (function_key_hdl(3, menu_1st_7th_6th_1th_length) == 0x00) {
                cun = (u8)menu_cnt.menu_3rd - 1;
                tr9_frame_pack2rk(0x6049, &cun, 0x01);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
            break;

        case 0x06: //��������
        {
            if (function_key_hdl(3, menu_1st_7th_6th_2th_length) == 0x00) {
                cun = (u8)menu_cnt.menu_3rd - 1;
                tr9_frame_pack2rk(0x6042, &cun, 0x01);

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
        } break;
        case 0x07: //�����ػ�״̬
        {
            if (function_key_hdl(3, menu_1st_7th_6th_3th_length) == 0x00) {
                car_state.bit.carriage = menu_cnt.menu_3rd;

                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
        } break;
        case 0x08: //��������
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
                mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                menu_cnt.menu_flag = true;
                menu_report.ok_flag = true;
            }
        }
            /*
						 if (function_key_hdl(3, menu_1st_7th_6th_length) == 0x00)
	                    {
	                  						
							 //������
							host_no_save_para.set_load_status = menu_cnt.menu_3rd;  //����״̬
	                        slave_send_msg((0x450000), 0x00, 0x00, false, spi1_up_comm_team_max);
	                  		
	                        menu_cnt.menu_other = mo_message_window;
	                        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
	                        mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
	                        mw_dis.bg_2ln_pixel = (8 * 1);		//��Ϣ��ʾ��������

	                        menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
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
    case 0x05: //��������
        switch (menu_cnt.menu_2nd) {
        case 0x05: //�̼�ѡ��
            switch (menu_cnt.menu_3rd) {
            case 0x03: //��չ��·����usart0:
                menu_cnt.next_regular = true;
                if (function_key_hdl(4, extend_uart_cnt) == 0x00) {
                    ;
                }
                break;
            }
            break;

        case 0x09: //ע���������
            switch (menu_cnt.menu_3rd) {
            case 0x06: //������ɫѡ��
                menu_cnt.next_regular = false;
                if (function_key_hdl(4, menu_1st_5th_9th_6th_length) == 0x00) {
                    if (menu_cnt.menu_4th <= 4)
                        temp = (menu_cnt.menu_4th);
                    else if (menu_cnt.menu_4th == 5)
                        temp = 9;
                    else
                        temp = 0x00;

                    slave_send_msg((0x260000), 0x01, temp, false, spi1_up_comm_team_max); //���ó�����ɫ

                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1);                                    //��Ϣ��ʾ��������
                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                    menu_cnt.menu_flag = true;
                }
                break;
            }
            break;
        }
        break;
    case 0x07: //�߼�ѡ��
        switch (menu_cnt.menu_2nd) {
        case 0x01: //�ն˿���
            switch (menu_cnt.menu_3rd) {
            case 0x07: //ϵͳ����
#if (1)                //����//RST-RK:ϵ�y����--�ſ��ԡ��ϡ����¡���ת���Լ�ִ�С����ء���ȷ�ϡ�
                menu_cnt.next_regular = false;
                if (function_key_hdl(4, menu_1st_7th_1st_val_length) == 0x00 /*���ڡ�ȷ�ϡ�����*/) {
                    //RST-RK:ϵ�y����//��������Ч
                    sys_cfg.foster_care = menu_cnt.menu_4th - 1; //���á�ϵͳ��������ϵͳ������ֵ��

                    Rk_Task_Manage.resetCount = 0;
                    s_Hi3520_Monitor.reset_tmr = jiffies;
                    s_Hi3520_Monitor.reset = false;
                    //logd("menu_cnt.menu_4th = %d, foster_care = %d", menu_cnt.menu_4th, sys_cfg.foster_care);

#if (1) //������:�����У����Ժ�//�ĸ���־��������ɹ���
                    menu_cnt.menu_other = mo_message_window;
                    _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                    mw_dis.p_dis_2ln = ((u8 *)(*(menu_other_lib_dis + 2)));
                    mw_dis.bg_2ln_pixel = (8 * 1);                                    //��Ϣ��ʾ��������
                    menu_report_set(6000, *(menu_report_dis + 0), rpt_average, true); //һ������  ����ɹ���ʧ��
                    menu_cnt.menu_flag = true;

                    //���óɹ�
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
    case 0x05: //��������
        switch (menu_cnt.menu_2nd) {
        case 0x05: //�̼�ѡ��
            switch (menu_cnt.menu_3rd) {
            case 0x03: //��չ��·����usart0:
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
                        mw_dis.bg_2ln_pixel = (8 * 1); //��Ϣ��ʾ��������

                        //COMT:����ʲô����������ȥ
                        //�����������ɹ�����ʧ�ܣ��ᱨ�浽��Ļ������
                        menu_report_set(6000, *(menu_report_dis /*���������*/), rpt_average, true); //һ������  ����ɹ���ʧ��
                        menu_cnt.menu_flag = true;
                        //�ֱ����ݣ�
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

    menu_cnt.menu_flag = true; //�����˵�����
}

//��ʱʹ�÷�������һ��
//2022��05��06�պ���Ҫ�����
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
//-----------               ����������               -----------------//
//�ú������������˵��µİ�������
//������˵�(�����˵�)��ز��ֵİ�������������Ӧ�Ĳ˵���������
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

    beeper(0);                //COMT:�ض���
    if (!(key_data.active)) { //��ֵû�и���
        return;
    }

    lcd_bl_ctrl(true);

#if (DEBUG_EN_BEEP)
    // buzzer_init(s_key); //����ʱ���η�����
    beeper(1); //COMT:������
#endif

    if ((key_data.lg) && (key_data.key == key_esc)) //���κν����£�����ESC�����᷵�ص���������
    {
        dis_goto_standby(); //�ص���������
    }

    key_data.active = false; //����������±�־

    if (menu_cnt.menu_other) {      //������˵�����
        key_data.active = true;     //�ָ�������־
        return;                     //������˵��Ĵ�����������״̬���ȴ���ز˵���������
    } else if (menu_cnt.menu_5th) { //���弶�˵���������
        menuFlag = true;
        countTime = tick;
        key_5th_hdl();
    } else if (menu_cnt.menu_4th) { //���ļ��˵���������
        menuFlag = true;
        countTime = tick;
        key_4th_hdl();
    } else if (menu_cnt.menu_3rd) { //�������˵���������
        menuFlag = true;
        countTime = tick;
        key_3rd_hdl();
    } else if (menu_cnt.menu_2nd) { //�ڶ����˵���������
        menuFlag = true;
        countTime = tick;
        key_2nd_hdl();
    } else if (menu_cnt.menu_1st) { //��һ���˵���������
        menuFlag = true;
        countTime = tick;
        key_1st_hdl();
    } else {
        menuFlag = false;
    }
}

/**************************************************************************************************
* �� �� ��:  analysis_IC_platform_info
* ��������:  �ж�ƽ̨�·���͸���������Ƿ��п�����Ϣ  dr: ��ͷ   *�ǺŽ�β
* ��ڲ���:  �洢ƽ̨����͸����ԭʼ����
* �� �� ֵ:  0,��ƽ̨������Ϣ   1����ƽ̨������Ϣ
* ��д����:  2018-3-29  ����ƽ̨��������
***************************************************************************************************/
u8 analysis_IC_platform_info(u8 *buf) {
    u8 *ptr1;
    u8 *ptr2;
    ptr1 = _strstr(buf, "dr:"); //����dr:
    ptr2 = _strstr(buf, "*");   //���� *

    //������ͬʱ����dr:  ��   * ,����Ϊ��ƽ̨��������
    if ((ptr1 != NULL) && (ptr2 != NULL)) {
        return 1; //�п�����Ϣ
    }
    return 0; //�޿�����Ϣ
}

/**************************************************************************************************
* �� �� ��:  Platform_set_card_message
* ��������:  ��ƽ̨�·��Ŀ�����Ϣ�Զ���ɿ�������
* ��ڲ���:  ��
* �� �� ֵ:  ��
* ��д����:  2018-3-29
***************************************************************************************************/
void Platform_set_card_message(void) {
    if ((menu_driver_info_cnt > 0)) //�п�����Ϣ
    {
    }
    usb_upgrade.ic_platform_flag = false;
    menu_auto_exit_set(2000, false); //�˻ش�������
}

/*************************************************
* �� �� ��:analyse_driver_name
* ��������:������ʻԱ������Ϣ,���ڲ˵���ʾ
* ��ڲ���:��
* �� �� ֵ:��
* ��д����:2017-12-25
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
        ptr += 3; //ȥ��ͷ" dr: "
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
* �� �� ��:read_ic_info
* ��������:��ȡIC��������Ϣ
* ��ڲ���:��
* �� �� ֵ:��
* ��д����:2017-12-25
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
            _memset(IC_info_tmp, 0x00, sizeof(IC_info_tmp)); //��ջ���
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
        case 0x02: //��ʼ��ȡ��ʻԱ������Ϣ
            if (res == 0) //��Ϣ��������0
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
* �� �� ��:my_itoa()
* ��������:��һ��10��������ת��Ϊһ���ַ���
* ��ڲ���:��Ҫת����������
* �� �� ֵ:��
* ��д����:2018-4-3
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
* �� �� ��:mo_id_auto_cfg_hal
* ��������:��ȡID������Ϣ
* ��ڲ���:��
* �� �� ֵ:��
* ��д����:2018-4-3
* ���ݸ�ʽ:id,����,
*  ����: 13800000001,100,
*  ��ʾ��1380000001��ʼ������һ����д100��id,�ڶ���
*  idΪ1380000002,���һ��IDΪ1380000100
*
**************************************************/

//static void mo_id_auto_cfg_hal(void)
//{
//    FIL file;
//    UINT btr = 32; //һ�ζ�ȡ���32���ֽ�����
//    UINT br;
//	static u8 res;
//    static enum
//    {
//        STEP_INIT = 0,   //��ʼ�����ָ��,�򿪲���ȡ�ļ�
//        STEP_ANALYZE ,   //����ID,��ID����
//        STEP_DISPLAY_ID, //��ʾID
//        STEP_WRITE_ID,   //��IDд���ն��豸
//        STEP_UPDATE_ID,  //����ID
//        STEP_WRITE_U_DISK, //�����º������д��u��
//        STEP_FINISH,       //���
//        STEP_ERR,          //������

//    } cfg_step = STEP_INIT;

//    switch (cfg_step)
//    {
//        case STEP_INIT: //���ļ�
//            _memset(id_cfg_buf, 0x00, sizeof(id_cfg_buf)); //��ջ���
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
//        lcd_dis_one_line(1, (8 * 0), ((uchar *)id_buf_tmp), false); //��ʾ��ǰ���õ�ID
//        /*����������ʾ�հ�,��Ҫ��Ϊ������Զ�����ʱ�������л�����ʾ�ַ�,��ʹ�ý��治��������*/
//        lcd_dis_one_line(2, (8 * 0), ((uchar *)(*(menu_other_lib_dis + 35))), false);
//        lcd_dis_one_line(3, (8 * 0), ((uchar *)(*(menu_other_lib_dis + 35))), false);
//          cfg_step = STEP_WRITE_ID;
//
//            break;
//
//       case STEP_WRITE_ID:
//            set_para.id[0] = _strlen(id_buf_tmp);                //�����ն�ID����
//            _memcpy((set_para.id + 2), id_buf_tmp, _strlen(id_buf_tmp)); //��������
//            flash25_program_auto_save((flash_parameter_addr + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));
//            slave_send_msg((0x040000), 0x00, 0x00, false, spi1_up_comm_team_max); //���ñ���ID
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

    if (Ack_Export == true) { //ȷ�˵�������
        menu_cnt.menu_other = mo_message_window;
        _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
        mw_dis.p_dis_2ln = ((u8 *)(USB_ack_export));
        mw_dis.bg_2ln_pixel = 16; //��Ϣ��ʾ��������
        Usb_Export = true;
    }

    if (Usb_Export == true) {
        if (1 == Export_finish) {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(USB_Export_finish));
            mw_dis.bg_2ln_pixel = 16 * 2; //��ʾλ��
            Usb_Export = false;
            exit = tick;
        } else if (2 == Export_finish) {
            menu_cnt.menu_other = mo_message_window;
            _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
            mw_dis.p_dis_2ln = ((u8 *)(USB_Export_failure));
            mw_dis.bg_2ln_pixel = 16 * 2; //��ʾλ��
            Usb_Export = false;
            exit = tick;
        }
    }

    if (0 != Export_finish) {
        if (_coveri(exit) >= 120) {
            dis_goto_standby(); //�ص���������
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
    mo_null_fun_hdl, //�պ���
#endif
    mo_standby_hdl,        //��������
    mo_gps_infor1_hdl,     //GPSȫ��Ϣ����1
    mo_gps_infor2_hdl,     //GPSȫ��Ϣ����2
    mo_recorder_state_hdl, //��ʻ��¼��״̬����
    mo_check_state_hdl,    //״̬������

    mo_print_hdl,        //��ӡ����
    mo_lcd_para_rst_hdl, //Һ����ز�����ʼ����ʾ����

    mo_message_window_hdl, //��Ϣ���ڽ���

    mo_1st_5th_1st_password_hdl, //���뷨����
    mo_1st_5th_4th_password_hdl,
    mo_1st_5th_6th_password_hdl,
    mo_1st_5th_8th_password_hdl,
    mo_1st_6th_1th_password_hdl,

    mo_1st_1st_1st_password_hdl,
    mo_1st_1st_2nd_password_hdl, //�ٶ�ģʽ����//220828�ٶ�����:

    mo_password_change_hdl, //�����޸Ľ���

    mo_1st_5th_1st_1st_hdl, //������IP����������
    mo_1st_5th_1st_3rd_hdl, //����ID����ŷ����������
    mo_1st_5th_1st_4th_hdl, //APN����

    mo_1st_5th_9th_1st_hdl, //ע������ز�����������
    mo_1st_5th_9th_7th_hdl, //ע������ز������ƺ�������

    mo_1st_1st_1st_1st_hdl, //������ϵ������
    mo_1st_1st_5th_1st_hdl, //��ʻԱ��������

    mo_host_para_hdl,   //������������
    mo_ic_card_dis_hdl, //IC����Ϣ
    //mo_slave_ver_check_hdl,   //�ӻ��汾�Ų�ѯ
    mo_csq_vol_hdl, //��ѯCSQֵ

    mo_auto_check_hdl,   //�Լ���Ϣ
    mo_query_rec_hdl,    //��ʻ��¼�ǲ�����ѯ
    mo_query_reg_hdl,    //ע�������ѯ
    mo_query_driver_hdl, //��ʻԱ������ѯ

    mo_update_tf_main_hdl,  //TF����������
    mo_update_font_lib_hdl, //�ֿ�����

    mo_oil_test_hdl,  //��������
    mo_telephone_hdl, //ͨ��

    mo_sms_fix_input_hdl, //�ϴ��¼�
    mo_sms_vod_input_hdl, //��Ϣ�㲥
    mo_sms_ask_input_hdl, //����Ӧ��
    mo_sms_display_hdl,   //��Ϣ������ʾ

    mo_phbk_display_hdl,      //�绰��������ʾ
    mo_expand_mult_usart_hdl, //��չ��·������ʾ
    mo_cfg_parameter_hdl,     //�Զ���������
    mo_read_rcd_data_hal,     //��ȡ��ʻ��¼������
    mo_auto_init_hal,         //�Զ���ʼ��������
    mo_timeout_driving_hal,
    mo_speed_record_hal,
    //mo_id_auto_cfg_hal,     //ID�Զ����ý���
    //key_2nd_phone_dial_hdl,		//��绰

};

//**********************************************************************//
//-----------               �˵�������               -----------------//
//�ú������������˵��Ͳ�����˵��µ���ʾ
//
//**********************************************************************//
//COMT:�˵�������//��ʾ
void menu_processing(void) {
    //    static u8 mm = 1;
    static u32 state_jif = 0;
    static u32 mainInterface = 0;

    if ((menu_cnt.menu_flag) || ((menu_cnt.menu_other) && (key_data.active))) { //�˵���������
        menu_cnt.menu_flag = false;                                             //����˵����²�����־

        if (menu_cnt.menu_other) { //������˵�����
#if (MO_ZERO_IS_NULL_FUN == 1)
            menu_other_point[(menu_cnt.menu_other)]();
#else
            menu_other_point[(menu_cnt.menu_other - 1)]();
#endif
            key_data.active = false;    //����������±�־���򲻹���˵���Ҫ������־���̱�־�ڴ˴�����
        } else if (menu_cnt.menu_5th) { //���弶�˵����洦��
            mainInterface = tick;
            menu_5th_hdl();
        } else if (menu_cnt.menu_4th) { //���ļ��˵����洦��
            mainInterface = tick;
            menu_4th_hdl();
        } else if (menu_cnt.menu_3rd) { //�������˵����洦��
            mainInterface = tick;
            menu_3rd_hdl();
        } else if (menu_cnt.menu_2nd) { //�ڶ����˵����洦��
            mainInterface = tick;
            menu_2nd_hdl();
        } else if (menu_cnt.menu_1st) { //��һ���˵����洦��
            mainInterface = tick;
            menu_1st_hdl();
        } else { //���ش�������
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

    USB_data_extern(); //���ݵ�����ʾ
}

//**********************************************************************//
//**********************************************************************//
//-----------             �͹��Ĵ�����               -----------------//
//
//**********************************************************************//
void low_power_hdl(void) {
    //----Һ�����⴦��----//
    if (gps_data.acc_state && (set_para.bl_type == 0)) //���ACC���ұ�������Ϊ����2018-3-29
    {
        lcd_bl_ctrl(true);

        return;
    } else {
        if (!gps_data.acc_state) //ACC�ص������,Ĭ��30��رձ���
        {
            if (sub_u32(jiffies, user_time.bl) > 30000) {
                lcd_bl_ctrl(false);
                return;
            }
        } else //���ACC��,�ұ������Ͳ�Ϊ��������
        {
            if (set_para.bl_type != 0) //������ǳ�������
            {
                if (set_para.bl_type != 255) //��������
                {
                    if (sub_u32(jiffies, user_time.bl) > (set_para.bl_type * 1000)) {
                        lcd_bl_ctrl(false);
                    }
                } else //ʡ��ģʽ
                {
                    set_para.bl_type = 5; //ʡ��ģʽ�ݰ��ձ�����5����ִ��  CKP  2014-10-08  //////////
                }
            }
        }
    }
}
