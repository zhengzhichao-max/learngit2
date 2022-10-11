#include "include_all.h"

const u8 hand_standard_ack[] = {0x24, 0x0D, 0x02, 0x2B, 0xFF}; //�ֱ�Э����׼Ӧ��

//�����˵�����
const u8 gb_bill_ack[14] =
    {
        0x00, 0x00, 0x00, 0x0a,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const u8 driver_infor[] =
    {
        "\x4����44010119790320123X\x0\x0\xc�㶫ʡ��ͨ��\x20\x20\x02\x14"};

const u8 dial_in_dis[] = "�绰����:";

#if 1

//�޷����ַ�����������һ��Ԫ��
void array_uchar_move_left(void *s_buff, u16 cnt) {
    *(((u8 *)(s_buff)) + (cnt - 1)) = *(((u8 *)(s_buff)) + cnt);
}

//�޷��Ŷ�������������һ��Ԫ��
void array_uint_move_left(void *s_buff, u16 cnt) {
    *(((u16 *)(s_buff)) + (cnt - 1)) = *(((u16 *)(s_buff)) + cnt);
}

//�޷��Ŷ�������������һ��Ԫ��
void array_uint_move_right(void *s_buff, u16 cnt) {
    *(((u16 *)(s_buff)) + (cnt + 1)) = *(((u16 *)(s_buff)) + cnt);
}

//�޷��ų�������������һ��Ԫ��
void array_ulong_move_left(void *s_buff, u16 cnt) {
    *(((u32 *)(s_buff)) + (cnt - 1)) = *(((u32 *)(s_buff)) + cnt);
}

//��Ϣʹ�õ�ַ������������һ��Ԫ��
void array_sms_use_move_left(void *s_buff, u16 cnt) {
    use_infor_struct *ptr;

    ptr = ((use_infor_struct *)(s_buff));

    (ptr + (cnt - 1))->read_flag = (ptr + cnt)->read_flag;
    (ptr + (cnt - 1))->type_id_serial_num = (ptr + cnt)->type_id_serial_num;
    (ptr + (cnt - 1))->addr = (ptr + cnt)->addr;
}

//��Ϣʹ�õ�ַ������������һ��Ԫ��
void array_sms_use_move_right(void *s_buff, u16 cnt) {
    use_infor_struct *ptr;

    ptr = ((use_infor_struct *)(s_buff));

    (ptr + (cnt + 1))->read_flag = (ptr + cnt)->read_flag;
    (ptr + (cnt + 1))->type_id_serial_num = (ptr + cnt)->type_id_serial_num;
    (ptr + (cnt + 1))->addr = (ptr + cnt)->addr;
}

//�Ƚ��ж�ָ��λ�õ�ֵ�ǲ���Ҫ���ҵ�ֵ
//����addrָ����λ��
//����d_dataҪ���ҵ�ֵ
//��������ֵ����
//0��ʾĿ��ֵ��������λ��ֵ
//1��ʾĿ��ֵ��������λ��ֵ
//2��ʾĿ��ֵС������λ��ֵ
static u8 cmp_spi_flash_unigb(u32 addr, void *d_data) {
    u8 ret;
    u8 buff[2];
    u16 s, d;

    spi_flash_read(buff, addr, 2);
    s = (buff[1] * 0x100 + buff[0]);
    d = *((u16 *)d_data);

    if (d >= s) {
        if (d == s) {
            ret = 0; //Ŀ��ֵ��������λ��ֵ
        } else {
            ret = 1; //Ŀ��ֵ��������λ��ֵ
        }
    } else {
        ret = 2; //Ŀ��ֵС������λ��ֵ
    }

    return ret;
}

//�۰����(���ַ�)
//����dirΪ���Ա����������    TRUEΪ����   FALSEΪ�ݼ�
//����addrΪ���Ա��ַ(������RAM��Ƭ��FLASH�еĻ�����Ҳ������SPI_FLASH�е�����)
//����lgthΪ���ҷ�Χ��ע���������з��ŵĳ�����
//����sizΪ���Ա���ֵ
//����d_dataΪҪ���ҵ�Ŀ��ֵָ��
//����ret_posiΪ���ҳɹ��󷵻�λ��
//����cmpΪ�Ƚϵײ㺯��
//��������ֵ    TRUE��ʾ���ҳɹ�    FALSE��ʾ����ʧ��
bool search_half(bool dir, u32 addr, signed long int lgth, u8 siz, void *d_data, u32 *ret_posi, gbk_unicode_compare_base cmp) {
    signed long int low, mid, high;
    u8 res;

    if (lgth <= siz)
        return false;

    low = 0x00;
    high = (lgth - siz);

    while (low <= high) {
        mid = ((high + low) / 2); //���м�ֵ
        mid -= (mid % siz);       //���ݶ���

        res = cmp((addr + mid), d_data); //����ȽϽ��
        if (res == 0) {
            *ret_posi = (addr + mid); //װ��λ��
            return true;              //�����ɹ�
        } else if (res == 1)          //Ŀ��ֵ��������λ��ֵ
        {
            if (dir == true) //��������
            {
                low = (mid + siz);
            } else {
                high = (mid - siz);
            }
        } else //Ŀ��ֵС������λ��ֵ
        {
            if (dir == true) //��������
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
    u32 posi; //���ַ����ҷ���Ҫ���ҵ����ݵ�λ��
    u16 ret;

    if (search_half(true, base_addr, font_gb_uni_table_lgth, 4, (&d_dat), (&posi), cmp_spi_flash_unigb)) {
        spi_flash_read(buff, (posi + 2), 2);
        ret = (buff[1] * 0x100 + buff[0]);
    } else //����ʧ��
    {
        ret = 0x00;
    }

    return ret;
}

//GBK������UNICODE��ת������
//ע��1:GBK�������ASCII��������룬��UNICODE����
//ע��2:���������ԴΪ���ģʽ
//����typ��ʾ  TRUE��ʾGBKת��UNI  FALSE��ʾUNIת��GBK
//��������ֵ��ʾת��֮������ݳ���
u16 gbk_unicode(bool typ, u8 *d_buff, u8 *s_buff, u16 lgth) {
    u16 ret, dat;

    ret = 0x00;
    while (lgth) {
        if (typ == true) //����ԴΪ������
        {
            if (*s_buff <= 0x7f) //ASCII��
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
        } else //����ԴΪUNICODE��
        {
            if ((*s_buff == 0x00) && (*(s_buff + 1) <= 0x7f)) //��ת��ASCII��
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

//�Ƚ���������������
//��������ֵ  0��ʾ�������  1��ʾBUFF1����BUFF2  2��ʾBUFF2����BUFF1
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

//�����ƶ�����
//����s_buff���ƶ�������
//����cntΪ���ƶ���Ԫ�ظ���
//����dirΪ����    TRUE����    FALSE����
//����function_baseΪ�ײ��������ָ��
void array_move(void *s_buff, u16 cnt, bool dir, array_move_base function_base) {
    u16 i;

    if (!cnt)
        return;

    if (dir) //�����ƶ�
    {
        while (cnt) {
            cnt--;
            function_base(s_buff, cnt);
        }
    } else //�����ƶ�
    {
        for (i = 0x00; i < cnt; i++) {
            function_base(s_buff, i);
        }
    }
}

//******************************************************************************************************//
//----------------------------            Э��������ݴ���               -------------------------------//
//******************************************************************************************************//
//��������
//ͨ��Ӧ��
static void down_0x0d(void) {
    if (menu_report.dat_type == rpt_average) {
        menu_report.ok_flag = true; //�յ����óɹ�Ӧ���־
    }
}

//��������
//������ϵ��
static void down_0x19(void) {
    u16 dat;

    if (query.rec.step == 1) {
        query.rec.step = 2;

        dat = *(slv_rev_data.buf + spi_rec_valid_start) * 0x100;
        dat += *(slv_rev_data.buf + spi_rec_valid_start + 1);

        data_mask_ascii((query.rec.ratio), dat, rec_para_ratio_max_lgth); //ת��ASCII��
    }
}

//��������
//����VIN��ѯ
static void down_0x1c(void) {
    if (query.regist.step == 0x0f) {
        query.regist.step = 0x10;

        _memset((query.regist.car_vin), 0x00, (rec_car_vin_max_lgth));
        _memcpy((query.regist.car_vin), (slv_rev_data.buf + spi_rec_valid_start), (rec_car_vin_max_lgth));
    }
}

//��������
//���ƺ����ѯ
static void down_0x1d(void) {
    if (query.regist.step == 0x0d) {
        query.regist.step = 0x0e;

        _memset((query.regist.car_plate), 0x00, (rec_car_plate_max_lgth));
        _memcpy((query.regist.car_plate), (slv_rev_data.buf + spi_rec_valid_start), (rec_car_plate_max_lgth));
    }
}

//��������
//��ʻԱ�����ѯ
static void down_0x1f(void) {
    if (query.driver.step == 0x01) {
        query.driver.step = 0x02;

        ic_card.work_num_lgth = (*(slv_rev_data.buf + 2) - 2); //��ȡ������Ϣ
        if (ic_card.work_num_lgth > 7)                         //��ʻԱ���뼴������󳤶�Ϊ7�ֽ�
        {
            ic_card.work_num_lgth = 7;
        }

        _memcpy((ic_card.work_num), (slv_rev_data.buf + spi_rec_valid_start), (ic_card.work_num_lgth));
    }
}

//��������
//��ʻ֤�����ѯ
static void down_0x20(void) {
    if (query.driver.step == 0x03) {
        query.driver.step = 0x04;

        ic_card.driv_lic_lgth = (*(slv_rev_data.buf + 2) - 2); //��ȡ������Ϣ
        if (ic_card.driv_lic_lgth > 18)                        //��ʻ֤������󳤶�Ϊ18�ֽ�
        {
            ic_card.driv_lic_lgth = 18;
        }

        _memcpy((ic_card.driv_lic), (slv_rev_data.buf + spi_rec_valid_start), (ic_card.driv_lic_lgth));
    }
}

//��������
//�ٶ�ģʽ����
//220828�ٶ�����:���á�ѯ�ʡ����صġ���¼ֵ��
static void down_0x21(void) {
    if (query.rec.step == 3) {
        query.rec.step = 4;
        query.rec.speed_type = *(slv_rev_data.buf + spi_rec_valid_start);
        logd("query.rec.step == 3, is %d, set step = 4, set speed_type = %d", query.rec.step, query.rec.speed_type);
    } else {
        logd("query.rec.step != 3, is %d", query.rec.step);
    }
}

//��������
//��չ��·����usart0:
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
        menu_report.ok_flag = true; //�յ����óɹ�Ӧ���־
    }

    menu_cnt.menu_other = mo_reset;
    menu_cnt.next_regular = true;
    *(((uint *)(&menu_cnt)) + 3) = 0x01;

    menu_cnt.menu_flag = true; //���²˵�
}

//��������
//������ɫ
static void down_0x26(void) {
    if (query.regist.step == 0x0b) {
        query.regist.step = 0x0c;

        ; //Э���д�����  CKP  20140306
        query.regist.car_plate_color = *(slv_rev_data.buf + spi_rec_valid_start);
    }
}

//��������
//����ID����
static void down_0x27(void) {
    u8 n;

    if (query.regist.step == 0x05) {
        query.regist.step = 0x06;

        n = *(slv_rev_data.buf + spi_rec_valid_start - 1) - 2;
        _memset((query.regist.manufacturer_id), 0x00, (rec_manufacturer_id_max_lgth));
        _memcpy((query.regist.manufacturer_id), (slv_rev_data.buf + spi_rec_valid_start), n);
    }
}

//��������
//ʡ��ID
static void down_0x60(void) {
    if (query.regist.step == 0x01) {
        query.regist.step = 0x02;

        query.regist.province_id[0] = *(slv_rev_data.buf + spi_rec_valid_start);
        query.regist.province_id[1] = *(slv_rev_data.buf + spi_rec_valid_start + 1);
    }
}

//��������
//������ID
static void down_0x61(void) {
    if (query.regist.step == 0x03) {
        query.regist.step = 0x04;

        query.regist.city_id[0] = *(slv_rev_data.buf + spi_rec_valid_start);
        query.regist.city_id[1] = *(slv_rev_data.buf + spi_rec_valid_start + 1);
        query.regist.city_id[2] = *(slv_rev_data.buf + spi_rec_valid_start + 2);
        query.regist.city_id[3] = *(slv_rev_data.buf + spi_rec_valid_start + 3);
    }
}

//��������
//�ն��ͺ�
static void down_0x62(void) {
    u8 n;

    if (query.regist.step == 0x07) {
        query.regist.step = 0x08;

        n = *(slv_rev_data.buf + spi_rec_valid_start - 1) - 2;
        _memset((query.regist.terminal_type), 0x00, (rec_terminal_type_max_lgth));
        _memcpy((query.regist.terminal_type), (slv_rev_data.buf + spi_rec_valid_start), n);
    }
}

//��������
//�ն�ID
static void down_0x63(void) {
    if (query.regist.step == 0x09) {
        query.regist.step = 0x0a;

        _memcpy((query.regist.terminal_id), (slv_rev_data.buf + spi_rec_valid_start), (rec_terminal_id_max_lgth));
    }
}

//��������
//�������Ʒ���
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

//��������
//��ӡ��׼����
static void down_0x6a(void) {
    if (query.rec.step == 5) {
        query.rec.step = 6;

        query.rec.print_type = *(slv_rev_data.buf + spi_rec_valid_start);
    }
}

//��������
//��λģʽ
static void down_0x6b(void) {
    u8 dat;

    dat = *(slv_rev_data.buf + spi_rec_valid_start); //��ȡ״ֵ̬

    if ((testbit(dat, 0)) && (testbit(dat, 1))) {
        host_no_save_para.set_gps_module_type = 3; //˫ģģʽ
    } else if (testbit(dat, 1)) {
        host_no_save_para.set_gps_module_type = 2; //������ģʽ
    } else {
        host_no_save_para.set_gps_module_type = 1; //��GPSģʽ
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
}

//��������
//���óɹ�
static void down_0xb1(void) {
    menu_report.ok_flag = true; //�յ����óɹ�Ӧ���־
    menu_cnt.menu_flag = true;
}

//��������
//λ������
static void down_0xa0(void) {
    _memcpy(((u8 *)(&gps_data)), (slv_rev_data.buf + spi_rec_valid_start), (sizeof(gps_data)));
    menu_cnt.menu_flag = true; //���²˵�

#ifdef sla_ack_0xa0_comm_en
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
#endif
}

//��������
//���յ�����Ϣ
static void down_0xa1(void) {
}

//��������
//����������Ϣ
static void down_0xa2(void) {
    ;
}

//��������
//������Ϣ����ʧ��
static void down_0xa3(void) {
    ;
}

//��������
//������Ϣ�����ɹ�
static void down_0xa4(void) {
    ;
}

//��������
//�绰����
static void down_0xa5(void) {
    u8 tmp_flag;
    u16 t1, t2, t3; //t1Ϊ������Ǹ������λ�ã�t2Ϊ���ұ��Ǹ������λ�ã�t3Ϊ�м��Ǹ������λ��
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
        slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
    } else {
        (*((u16 *)(&(phone_data.buff)))) = (slv_rev_data.len - 5);
        _memcpy((phone_data.buff + 2), (slv_rev_data.buf + spi_rec_valid_start), (slv_rev_data.len - 5));

        lcd_dis_one_line(0, 0, dial_in_dis, false);

        phone_data.step = call_in;
        menu_cnt.menu_other = mo_telephone;
        menu_cnt.menu_flag = true;
        //�ӵ绰������ȥѰ�Ҹ���������֣��ö��ַ�����
        spi_flash_read(((u8 *)(&phbk_count)), phbk_start_addr_count, (sizeof(phbk_count_struct))); //��ȡ����
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

                if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth - 1), ((phone_data.buff) + 2), (phone_data.buff[0])) == 1) //��ǰ�벿��
                {
                    t2 = t3;
                } else if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth - 1), ((phone_data.buff) + 2), (phone_data.buff[0])) == 2) //�ں�벿��
                {
                    t1 = t3;
                }

                else if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth - 1), ((phone_data.buff) + 2), (phone_data.buff[0])) == 0) //�������м������
                {
                    tmp_flag = 0x01;
                }
                cnt++;
                if (cnt > phbk_count.total) {
                    break;
                }
            }
            if (tmp_flag == 0x01) //�ҵ��绰�����Ӧ������
            {
                if ((phbk_data.type == phbk_type_in_only) || (phbk_data.type == phbk_type_in_out)) //�ɺ���
                {
                    lcd_dis_one_page(16, (*((u16 *)(&(phbk_data.name.lgth)))), (phbk_data.name.buff));
                    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
                    write_data_to_call_record_flash(flash_missed_call_record_addr, phbk_data);
                    _memcpy((u8 *)(&phbk_call_in_data), (u8 *)&phbk_data, sizeof(phbk_data));
                } else //���ɺ��������ɺ���
                {
                    phone_data.step = call_halt;                                             //֪ͨ�����һ�
                    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
                }
            } else //û���ҵ��绰�����Ӧ������
            {
                phone_data.step = call_halt;                                             //֪ͨ�����һ�
                slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
            }
        } else {
            lcd_dis_one_page(16, (*((u16 *)(&(phone_data.buff)))), (phone_data.buff + 2));
            slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
            phbk_call_in_data.num.lgth = phone_data.buff[0];
            _memcpy(phbk_call_in_data.num.buff, (&phone_data.buff[2]), phbk_call_in_data.num.lgth);
            write_data_to_call_record_flash(flash_missed_call_record_addr, phbk_call_in_data);
        }
        lcd_area_dis_set(4, 7, 0, 131, 0x00);
    }
}

//��������
//�Է��һ�
static void down_0xa6(void) {
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������

    phone_data.step = call_end;
    menu_cnt.menu_other = mo_telephone;
    menu_cnt.menu_flag = true;
}

//��������
//�绰��ͨ
static void down_0xa7(void) {
    static u8 tmp_call_in_cnt; //��������ÿ�ν����绰���ᷢ����0xa7������ӻ������Ա����ô˼���������һ�ν��յ��ĸ��������ε�

    if (phone_data.step == call_out) {
        slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������

        phone_data.dtmf_ptr = 0x00;
        phone_data.dtmf_cnt = 0x00; //���������
        phone_data.tim = jiffies;   //ʱ��ͬ��

        phone_data.step = call_connect;
        menu_cnt.menu_other = mo_telephone;
        menu_cnt.menu_flag = true;

    } else if (phone_data.step == call_in) //����Ǻ����Ϊ�ѽӵ绰
    {
        tmp_call_in_cnt++;
        slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
        if (tmp_call_in_cnt == 0x02) {
            tmp_call_in_cnt = 0x00;
            //ɾ���ո��Լӵ�δ�ӵ绰��¼
            spi_flash_read(&call_record.call_record_cnt, flash_missed_call_record_addr, sizeof(call_record.call_record_cnt));
            call_record.call_record_cnt--;
            flash25_program_auto_save(flash_missed_call_record_addr, &call_record.call_record_cnt, sizeof(call_record.call_record_cnt));
            //�洢�ѽӺ���

            write_data_to_call_record_flash(flash_received_call_record_addr, phbk_call_in_data);
            _memset((void *)(&phbk_call_in_data), 0x00, sizeof(phbk_call_in_data));

            spi_flash_read(((u8 *)(&call_record)), flash_received_call_record_addr, (sizeof(call_record_struct)));
            if (call_record.call_record_cnt > call_record_num)
                call_record.call_record_cnt = call_record_num;
        }
    }
}

//��������
//�Լ���Ϣ�·�
static void down_0xa8(void) {
    if (menu_report.dat_type == rpt_com) {
        menu_report.ok_flag = true; //�յ����óɹ�Ӧ���־
    }

    _memcpy(((u8 *)(&query.auto_check)), (slv_rev_data.buf + spi_rec_valid_start), (sizeof(query.auto_check))); //�̶���ȡ19���ֽڵ�����
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max);                                    //װ��Ӧ������

    menu_cnt.menu_other = mo_auto_check;
    menu_cnt.menu_flag = true; //���²˵�
}

//��������
//����״̬����
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
        //����ID
        for (j = 0x00; j < 19; j++) {
            if (*(ptr + i + j) == 'C') {
                break;
            }

            query.host.id_num[j + 1] = *(ptr + i + j);
        }
        query.host.id_num[0] = j;
        query.host.id_num[j + 1] = '\x0';

        i = i + j + 1;
        //���ŷ������
        for (j = 0x00; j < 19; j++) {
            if (*(ptr + i + j) == 'I') {
                break;
            }

            query.host.sms_num[j + 1] = *(ptr + i + j);
        }
        query.host.sms_num[0] = j;
        query.host.sms_num[j + 1] = '\x0';

        i = i + j + 1;
        //IP��ַ
        for (j = 0x00; j < 32; j++) {
            if (*(ptr + i + j) == 'U')
                break;

            if (*(ptr + i + j) == '"') {
                m++;
                if (m == 1)
                    query.host.ip[j] = ',';
                //����һ�����Ÿ�Ϊ����
                else
                    n = 1;
                //�����ڶ�������
            } else {
                query.host.ip[j - n] = *(ptr + i + j);
            }
        }
        query.host.ip_lgth = (j - 1);

        i = i + j + 1;
        //�û�����
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
        //���ͷ�ʽ
        query.host.transmit_time[0] = *ptr++;
        query.host.transmit_time[1] = *ptr++;
        //����ʱ��
        query.host.speed = *ptr++;
        //���ٱ���
        query.host.outage = *ptr++;
        //�ϵ籨��
        query.host.stop = *ptr++;
        //ͣ������

        query.host.call_time[0] = *ptr++;
        query.host.call_time[1] = *ptr++;
        //����ʱ��
        query.host.answer_time[0] = *ptr++;
        query.host.answer_time[1] = *ptr++;
        //����ʱ��

        //�����汾������
        for (j = 0x00; j < 6; j++)
            query.host.ver_date[j] = *ptr++;

        //���Զ��������ѡ��ÿ���ͽ�ȫ����ʾ  CMNET
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
            //����IP��ַ
            for (j = 0x00; j < 32; j++) {
                if (*ptr == 'K')
                    break;

                if (*ptr == '"') {
                    m++;
                    if (m == 1)
                        query.host.Backup_ip[j] = ',';
                    //����һ�����Ÿ�Ϊ����
                    else
                        n = 1;
                    //�����ڶ�������
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
            query.host.ver_date2[24] = '\x0'; //��ֹ�������͹���������û�н�����־

            m = 0x00;
            n = 0x00;

            //IP��ַ
            for (j = 0x00; j < 32; j++) {
                if (*(ptr + j) == 'U')
                    break;

                if (*(ptr + j) == '"') {
                    m++;
                    if (m == 1)
                        query.host.ip3[j] = ',';
                    //����һ�����Ÿ�Ϊ����
                    else
                        n = 1;
                    //�����ڶ�������
                } else {
                    query.host.ip3[j - n] = *(ptr + j);
                }
            }
            query.host.ip3_lgth = (j - 1);
        } else {
            query.host.Backup_ip_lgth = 0x00; //����չЭ�鲻��������IP
            query.host.ver_date2[0] = '\x0';  //����չЭ�鲻�����汾���ַ���
        }

        if (menu_report.dat_type == rpt_com) {
            menu_report.ok_flag = true; //�յ����óɹ�Ӧ���־
        }

        slave_send_msg((0x0d0000), (0x00), 0x00, true, spi1_up_comm_team_max); //װ��Ӧ������

        menu_cnt.menu_other = mo_host_para;
        menu_cnt.menu_flag = true; //���²˵�
    }
}

//��������
//�ֻ�����pdu��ʽ
static void down_0xaf(void) {
    u8 *ptr;
    sms_index_struct sms_index;
    u8 gb2312_buff[sms_phone_single_piece_lgth];
    u16 n, addr;
    u16 gb2312_cnt;

    n = *(slv_rev_data.buf + 2) - 2; //��Э�����賤�������ж�
    ptr = (slv_rev_data.buf + spi_rec_valid_start);

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_phone_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max) {
        sms_index.total = sms_total_max;
    }

    addr = find_the_symbol(ptr, n, '"', 5); //�ҵ���������ţ�Ϊ���ݵ���ʼ��
    gb2312_cnt = gbk_unicode(false, gb2312_buff, (ptr + addr + 1), (n - addr - 2));
    _memcpy((ptr + addr), gb2312_buff, gb2312_cnt); //��������

    n = (addr + gb2312_cnt);               //����ʾ��������Ϣ���ݵĳ���
    *(slv_rev_data.buf + 1) = (n % 0x100); //���û������洢����
    *(slv_rev_data.buf + 2) = (n / 0x100);
    n += 2;

    sms_operate_hdl((&sms_index), sms_op_add, 0x00, n, (slv_rev_data.buf + 1));
    flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //ֻ���ڷ���Ϣ��ʾ�����²Ż��Զ�������Ϣ��ʾ
    {
        sms_up_center.sms_type = sms_type_phone;
        spi_flash_read(((u8 *)(&(sms_union_data.phone))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_phone_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������

    //----------------------//  �����ʶ�����
    _memcpy(sound_play_buff, (slv_rev_data.buf + 1), n);
    slave_send_msg((0xee0000), 0x00, 0x00, false, spi1_up_comm_team_max);
    //----------------------//
}

//��������
//�ֻ������ı���ʽ
static void down_0xb0(void) {
    u8 *ptr, buff[300]; //��Э��300�ֽڳ�������  ǰ�����ֽڴ�ų���  ���ģʽ
    u16 n, addr;
    sms_index_struct sms_index;

    n = *(slv_rev_data.buf + 2) - 2; //��Э�����賤�������ж�
    ptr = (slv_rev_data.buf + spi_rec_valid_start);

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_phone_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max) {
        sms_index.total = sms_total_max;
    }

    addr = find_the_symbol(ptr, n, '"', 5); //�ҵ���������ţ�Ϊ���ݵ���ʼ��
    n -= 2;                                 //ȥ���������˵�����
    _memcpy((buff + 2), ptr, addr);
    _memcpy((buff + 2 + addr), (ptr + addr + 1), (n - addr));

    *(buff + 0) = (n % 0x100);
    *(buff + 1) = (n / 0x100);
    n += 2;

    sms_operate_hdl((&sms_index), sms_op_add, 0x00, n, (buff));
    flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //ֻ���ڷ���Ϣ��ʾ�����²Ż��Զ�������Ϣ��ʾ
    {
        sms_up_center.sms_type = sms_type_phone;
        spi_flash_read(((u8 *)(&(sms_union_data.phone))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_phone_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_phone_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������

    //----------------------//  �����ʶ�����
    _memcpy(sound_play_buff, buff, n);
    slave_send_msg((0xee0000), 0x00, 0x00, false, spi1_up_comm_team_max);
    //----------------------//
}

//��������
//IC����������
static void down_0xc2(void) {
    u8 ok_flag; //����ִ�гɹ���־  0ִ��ʧ��  1ִ�гɹ�
    u8 *ptr;    //����ָ��

    /*
    if(state_data.state.flag.ic_flag == false)  //����IC��������ֱ���˳�
    {
    	return;
    }
    */
    //���ж�״̬λ����Ϊ״̬λ���п�����������У��ͨ�������λ  CKP  2014-06-20

    ptr = (slv_rev_data.buf + spi_rec_valid_start);
    ok_flag = 0x00;

    if (ic_type == ic_2402) { //IC2402
        logd("IC����:%c", *(ptr + 7));
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

//��������
//�ı���Ϣ
static void down_0xee_0x8300(void) {
    u8 type;
    u16 n;
    sms_index_struct sms_index;

    n = ((*(slv_rev_data.buf + 2)) * 0x100) + (*(slv_rev_data.buf + 3)) - 5;
    if (n > sms_center_single_piece_lgth) //��������
    {
        n = sms_center_single_piece_lgth;
    }
    type = (*(slv_rev_data.buf + spi_rec_expand_valid_start));

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_center_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    *(slv_rev_data.buf + spi_rec_expand_valid_start - 1) = (n % 0x100);
    *(slv_rev_data.buf + spi_rec_expand_valid_start) = (n / 0x100);
    n += 2; //���û������洢����

    sms_operate_hdl((&sms_index), sms_op_add, type, n, (slv_rev_data.buf + spi_rec_expand_valid_start - 1));
    flash25_program_auto_save(flash_sms_center_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //ֻ���ڷ���Ϣ��ʾ�����²Ż��Զ�������Ϣ��ʾ
    {
        sms_up_center.sms_type = sms_type_center;
        spi_flash_read(((u8 *)(&(sms_union_data.center))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_center_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_center_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������

    if (type & (bit_move(3))) //���ʶ���Ϣ������
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

//��������
//�¼���Ϣ
static void down_0xee_0x8301(void) {
    u8 type;
    u8 flag, i, n;
    u8 *ptr;

    u8 id;
    u16 lgth, cnt;

    sms_index_struct sms_index;
    sms_op_enum sms_op;

    flag = 0x00; //ɾ����־����
    sms_op = sms_op_add;
    type = (*(slv_rev_data.buf + spi_rec_expand_valid_start));  //��������
    n = (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //������
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start + 2);  //��ָ�븳ֵ

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_fix_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    switch (type) {
    case 0x00: //ɾ��ȫ��
        flag = 1;
        sms_index.total = 0x00;
        break;

    case 0x01: //����
        sms_index.total = 0x00;
        sms_op = sms_op_add;
        break;

    case 0x02: //׷��
        sms_op = sms_op_add;
        break;

    case 0x03: //�޸�
        sms_op = sms_op_change;
        break;

    case 0x04: //ɾ��
        sms_op = sms_op_del;
        break;

    default: //�����쳣
        return;
    }

    if (flag == 0x00) //��ȫ��ɾ������
    {
        for (i = 0x00; i < n; i++) {
            id = *(ptr + 0);
            cnt = *(ptr + 1);

            if (cnt > (sms_fix_single_piece_lgth - 40)) //��������
            {
                lgth = (sms_fix_single_piece_lgth - 40);
            } else {
                lgth = cnt;
            }

            *(ptr + 0) = lgth;
            *(ptr + 1) = 0x00; //���û������洢����
            lgth += 2;

            sms_operate_hdl((&sms_index), sms_op, id, lgth, ptr);

            ptr += (cnt + 2);
        }
    }

    flash25_program_auto_save(flash_sms_fix_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if ((menu_cnt.menu_other != mo_sms_display) && (sms_op == sms_op_add)) //ֻ���ڷ���Ϣ��ʾ�����²Ż��Զ�������Ϣ��ʾ
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

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
}

//��������
//�����·�
static void down_0xee_0x8302(void) {
    sms_index_struct sms_index;
    sms_ask_content_struct sms_ask;

    u8 *ptr;
    u16 i, cnt;
    u16 n;
    u16 ser_num;

    cnt = ((*(slv_rev_data.buf + 2)) * 0x100) + (*(slv_rev_data.buf + 3)) - 4; //���㱾�����ݵ���Ч���ݳ���
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start);
    ser_num = ((*(ptr + 0)) * 0x100) + (*(ptr + 1)); //��¼��ˮ��

    n = (*(ptr + 3)); //���ⳤ��
    i = 4;            //�޸�ָ�룬ָ����������

    if (n > (sms_ask_content_lgth - 40)) //�жϳ�������
    {
        sms_ask.lgth = (sms_ask_content_lgth - 40);
    } else {
        sms_ask.lgth = n;
    }

    _memcpy(sms_ask.dat, (ptr + i), sms_ask.lgth); //��ֵ����
    i += n;                                        //�޸�ָ�룬ָ�������

    sms_ask.ans_cnt = 0x00;
    while (i < cnt) {
        if (sms_ask.ans_cnt >= sms_ask_answer_total_max) //����Ŀ������Χ
            break;

        (sms_ask.answer + sms_ask.ans_cnt)->ans_id = *(ptr + i); //��ID
        n = ((*(ptr + i + 1)) * 0x100) + (*(ptr + i + 2));       //�𰸳���

        if (n > sms_ask_answer_content_lgth) //�жϳ�������
        {
            (sms_ask.answer + sms_ask.ans_cnt)->ans_lgth = sms_ask_answer_content_lgth;
        } else {
            (sms_ask.answer + sms_ask.ans_cnt)->ans_lgth = n;
        }

        _memcpy(((sms_ask.answer + sms_ask.ans_cnt)->ans_dat), (ptr + i + 3), ((sms_ask.answer + sms_ask.ans_cnt)->ans_lgth)); //��ֵ����

        i = (i + 3 + n);
        sms_ask.ans_cnt++;
    }

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_ask_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    sms_operate_hdl((&sms_index), sms_op_add, ser_num, (sizeof(sms_ask)), ((u8 *)(&sms_ask)));
    flash25_program_auto_save(flash_sms_ask_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //ֻ���ڷ���Ϣ��ʾ�����²Ż��Զ�������Ϣ��ʾ
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

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
}

//��������
//��Ϣ�㲥
static void down_0xee_0x8303(void) {
    u8 type;
    u8 flag, i, n;
    u8 *ptr;

    u8 id;
    u16 lgth, cnt;

    sms_index_struct sms_index;
    sms_op_enum sms_op;

    flag = 0x00; //ɾ����־����
    sms_op = sms_op_add;
    type = (*(slv_rev_data.buf + spi_rec_expand_valid_start));  //��������
    n = (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //������
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start + 2);  //��ָ�븳ֵ

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_vod_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    switch (type) {
    case 0x00: //ɾ��ȫ��
        flag = 1;
        sms_index.total = 0x00;
        break;

    case 0x01: //����
        sms_index.total = 0x00;
        sms_op = sms_op_add;
        break;

    case 0x02: //׷��
        sms_op = sms_op_add;
        break;

    case 0x03: //�޸�
        sms_op = sms_op_change;
        break;

    default: //�����쳣
        return;
    }

    if (flag == 0x00) //��ȫ��ɾ������
    {
        for (i = 0x00; i < n; i++) {
            id = *(ptr + 0);
            cnt = ((*(ptr + 1)) * 0x100) + (*(ptr + 2));

            if (cnt > (sms_vod_single_piece_lgth - 40)) //��������
            {
                lgth = (sms_vod_single_piece_lgth - 40);
            } else {
                lgth = cnt;
            }

            *(ptr + 1) = (lgth % 0x100);
            *(ptr + 2) = (lgth / 0x100); //���û������洢����
            lgth += 2;

            sms_operate_hdl((&sms_index), sms_op, id, lgth, (ptr + 1));

            ptr = (ptr + cnt + 3);
        }
    }

    flash25_program_auto_save(flash_sms_vod_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if ((menu_cnt.menu_other != mo_sms_display) && (sms_op == sms_op_add)) //ֻ���ڷ���Ϣ��ʾ�����²Ż��Զ�������Ϣ��ʾ
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

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
}

//��������
//������Ϣ
static void down_0xee_0x8304(void) {
    u16 n;
    sms_index_struct sms_index;

    n = ((*(slv_rev_data.buf + 2)) * 0x100) + (*(slv_rev_data.buf + 3)) - 7;
    if (n > sms_serve_single_piece_lgth) //��������
    {
        n = sms_serve_single_piece_lgth;
    }

    spi_flash_read(((u8 *)(&sms_index)), flash_sms_serve_addr, (sizeof(sms_index)));
    if (sms_index.total > sms_total_max)
        sms_index.total = sms_total_max;

    *(slv_rev_data.buf + spi_rec_expand_valid_start + 1) = (n % 0x100);
    *(slv_rev_data.buf + spi_rec_expand_valid_start + 2) = (n / 0x100); //���û������洢����
    n += 2;

    sms_operate_hdl((&sms_index), sms_op_add, (*(slv_rev_data.buf + spi_rec_expand_valid_start)), n, (slv_rev_data.buf + spi_rec_expand_valid_start + 1));
    flash25_program_auto_save(flash_sms_serve_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

    if (menu_cnt.menu_other != mo_sms_display) //ֻ���ڷ���Ϣ��ʾ�����²Ż��Զ�������Ϣ��ʾ
    {
        sms_up_center.sms_type = sms_type_serve;
        spi_flash_read(((u8 *)(&(sms_union_data.serve))), ((sms_index.use_infor + 0)->addr), (sizeof(sms_serve_content_struct)));

        ((sms_index.use_infor + 0)->read_flag) = false;
        flash25_program_auto_save(flash_sms_serve_addr, ((u8 *)(&sms_index)), (sizeof(sms_index)));

        dis_multi_page_cnt = 0x00;
        menu_cnt.menu_other = mo_sms_display;
        menu_cnt.menu_flag = true;
    }

    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
}

//��������
//�绰��
static void down_0xee_0x8401(void) {
    u8 *ptr;
    u8 op, cnt, lgth;
    u16 i, n;

    phbk_count_struct phbk_count;
    phbk_data_struct phbk_data;

    op = (*(slv_rev_data.buf + spi_rec_expand_valid_start));    //��������
    n = (*(slv_rev_data.buf + spi_rec_expand_valid_start + 1)); //������
    ptr = (slv_rev_data.buf + spi_rec_expand_valid_start + 2);  //��ָ�븳ֵ

    if (op == 0x00) {
        phbk_data_flash_init();
    } else if (op != 0x00) //��ȫ��ɾ������
    {
        //------------// ////////////////
        if (op >= 0x03) //�������޸Ĳ�������������
            goto ret_0x8401;
        //------------//

        spi_flash_read(((u8 *)(&phbk_count)), phbk_start_addr_count, (sizeof(phbk_count_struct))); //��ȡ����
        (phbk_count.posi) = (phbk_count.posi) % phone_book_total_max;
        if ((phbk_count.total) > phone_book_total_max) {
            (phbk_count.total) = phone_book_total_max;
        }

        if (op == 0x01) //���²���
        {
            phbk_count.total = 0x00;
            phbk_count.posi = 0x00;
        }

        for (i = 0x00; i < n; i++) {
            //------------// ////////////////
            if (phbk_count.total >= phone_book_total_max) //������Χ����������������
                break;
            //--------------------//

            phbk_data.type = (phbk_type_enum)(*(ptr++) - 1); //����

            cnt = *(ptr++); //���볤��
            if (cnt > phone_book_num_max_lgth) {
                lgth = phone_book_num_max_lgth;
            } else {
                lgth = cnt;
            }
            phbk_data.num.lgth = lgth;
            _memcpy((phbk_data.num.buff), (ptr), lgth);
            ptr += cnt;

            cnt = *(ptr++); //��������
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

        flash25_program_auto_save(phbk_start_addr_count, ((uchar *)(&phbk_count)), (sizeof(phbk_count_struct))); //��������
    }

ret_0x8401:
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
}

//��������
//������ȡ��ʻԱ��Ϣ
static void down_0xee_0x8702(void) {
    slave_send_msg((0x0d0000), (0x00), (0x00), true, spi1_up_comm_team_max); //װ��Ӧ������
    slave_send_msg((0xee0702), 0x00, 0x00, false, spi1_up_comm_team_max);
}

//��������ɢת��(����FLASH��)
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
��    �ƣ�slave_recive_msg
��    �ܣ��ӻ���������
�����������
��    ������
**********************************************************/
u8 slave_recive_msg(u8 *str_t, u16 len_t) {
    if (len_t > HANDSET_MAX_SIZE || len_t < HANDSET_MIN_SIZE || *str_t != 0x24)
        return CMD_ERROR;

    _memset((u8 *)&slv_rev_data.buf, 0x00, HANDSET_MIN_SIZE);
    slv_rev_data.len = _memcpy_len((u8 *)&slv_rev_data.buf, str_t, len_t);

    return CMD_ACCEPT;
}

/*********************************************************
��    �ƣ�slave_send_data_proc
��    �ܣ�����ӻ����ݵķ��ͣ����ǵ�Ƕ�׺�����������
���������ݹ���ʽ��������
�����������
��    ������
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

//���а���װ
//lgth���������ݳ���
void up_packeg(u32 main_comm, u8 *buff, u16 lgth) {
    u16 n;
    u16 m;
    u8 temp[single_packet_max_lgth];

    n = 0x00;
    *(temp + (n++)) = 0x24;  //��ͷ��־
    m = main_comm / 0x10000; //�����
    *(temp + (n++)) = m;

    if (m == expand_num)  { //��չ����
        *(temp + (n++)) = ((lgth + 4) / 0x100);
        *(temp + (n++)) = ((lgth + 4) % 0x100); //����

        *(temp + (n++)) = ((main_comm % 0x10000) / 0x100);
        *(temp + (n++)) = (main_comm % 0x100); //������
    } else {                                   //��׼����
        *(temp + (n++)) = (lgth + 2);
    }

    _memcpy((temp + n), buff, lgth);
    n += lgth;
    *(temp + n) = _get_xor(temp, n); //����У��
    n++;

    *(temp + (n++)) = 0xff; //������

    if (n >= HANDSET_MAX_SIZE) {
        return;
    }

    _memset((u8 *)&slv_send_data.buf, 0x00, HANDSET_MAX_SIZE);
    slv_send_data.len = _memcpy_len((u8 *)&slv_send_data.buf, temp, n);
}

//��������
//����ƽ̨������Ϣ
static void up_0x01(void) {
    ;
}

//��������
//�����ֻ���Ϣ
static void up_0x02(void) {
    ;
}

//��������
//�����ն�IP��ַ
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

//��������
//�����ն�ID��
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

//��������
//���ö��ŷ������
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

//��������
//����
static void up_0x07(void) {
    up_packeg((0x070000), (phone_data.buff + 2), (*((u16 *)(&(phone_data.buff)))));
}

//��������
//�һ�
static void up_0x08(void) {
    u8 temp;

    up_packeg((0x080000), (&temp), 0);
}

//��������
//����DTMF�ź�
static void up_0x09(void) {
    up_packeg((0x090000), (phone_data.dtmf_buff + (phone_data.dtmf_cnt - 1)), 1);
}

//��������
//�ն��Լ�
static void up_0x0c(void) {
    u8 temp;

    up_packeg((0x0c0000), (&temp), 0);
}

//��������
//ͨ��Ӧ��
static void up_0x0d(void) {
    u8 temp;

    up_packeg((0x0d0000), (&temp), 0);
}

//��������
//�����绰
static void up_0x0e(void) {
    u8 temp;

    up_packeg((0x0e0000), (&temp), 0);
}

//��������
//����״̬��ѯ
static void up_0x11(void) {
    u8 temp;

    up_packeg((0x110000), (&temp), 0);
}

//��������
//����������ʼ��
static void up_0x15(void) {
    u8 temp;

    up_packeg((0x150000), (&temp), 0);
}

//��������
//������ϵ������
static void up_0x19(void) {
    u8 temp[10];
    u16 i;
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
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

//��������
//����VIN����
static void up_0x1c(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        n = (*((u16 *)(recorder_para.car_vin)));
    }

    up_packeg((0x1c0000), (recorder_para.car_vin + 2), n);
}

//��������
//���ƺ�������
static void up_0x1d(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        n = (*((u16 *)(recorder_para.car_plate)));
    }

    up_packeg((0x1d0000), (recorder_para.car_plate + 2), n);
}

//��������
//��ʻԱ��������
static void up_0x1f(void) {
    //u8  n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        //	n = 0x00;
    } else {
        //	n = ic_card.work_num_lgth;
    }

    //up_packeg((0x1f0000), (ic_card.work_num), n);
}

//��������
//��ʻ֤��������
static void up_0x20(void) {
    //u8  n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        //	n = 0x00;
    } else {
        //	n = ic_card.driv_lic_lgth;
    }

    //up_packeg((0x200000), (ic_card.driv_lic), n);
}

//��������
//�ٶȷ�ʽѡ��
static void up_0x21(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) { //��ѯģʽ
        n = 0;
    } else {
        n = 1;
    }

    up_packeg((0x00210000), (&(slv_send_flag.status)), n);
}

//��������
//������ӡ
static void up_0x22(void) {
    u8 temp;

    up_packeg((0x220000), (&temp), 0);
}

//��������
//�Զ���APN����
static void up_0x24(void) {
    u8 temp[100];

    *(temp + 0) = 0x01;
    _memcpy((temp + 1), (set_para.apn + 2), (*((u16 *)(set_para.apn))));
    up_packeg((0x240000), temp, (*((u16 *)(set_para.apn)) + 1));
}

//��������
//��չ��·����usart0:��������
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

//��������
//������ɫ����
static void up_0x26(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        n = 1;
    }

    up_packeg((0x260000), (&(slv_send_flag.status)), n);
}

//��������
//��ѯ/����������ID
static void up_0x27(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        n = *((u16 *)(&(recorder_para.manufacturer_id)));
    }

    up_packeg((0x270000), (recorder_para.manufacturer_id + 2), n);
}

//��������
//У׼��
static void up_0x38(void) {
    up_packeg((0x380000), (&(host_no_save_para.set_load_status)), 0);
}

//��������
//У׼�ر�
static void up_0x39(void) {
    up_packeg((0x390000), (&(host_no_save_para.set_load_status)), 0);
}

//��������
//���ñ���IP��ַ
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

//��������
//������
static void up_0x45(void) {
    up_packeg((0x450000), (&(host_no_save_para.set_load_status)), 1);
}

//��������
//������
static void up_0x47(void) {
    up_packeg((0x470000), (&(host_no_save_para.set_load_status)), 0);
}

//��������
//�����ر�
static void up_0x48(void) {
    up_packeg((0x480000), (&(host_no_save_para.set_load_status)), 0);
}

//��������
//ʡ��ID����
static void up_0x60(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        n = *((u16 *)(&(recorder_para.province_id)));
    }

    up_packeg((0x600000), (recorder_para.province_id + 2), n);
}

//��������
//������ID����
static void up_0x61(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        n = *((u16 *)(&(recorder_para.city_id)));
    }

    up_packeg((0x610000), (recorder_para.city_id + 2), n);
}

//��������
//�ն��ͺ�����
static void up_0x62(void) {
    u8 i, n;
    u8 temp[rec_terminal_type_max_lgth];

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
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

//��������
//�ն�ID����
static void up_0x63(void) {
    u8 i, n;
    u8 temp[rec_terminal_id_max_lgth];

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
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

//��������
//���ƺ����������
static void up_0x66(void) {
    u8 n = 0;
    u8 *ptr = 0;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        ptr = (u8 *)(menu_1st_1st_4th_dis[slv_send_flag.status - 1]);

        if (slv_send_flag.status <= 9) //ȥ��λ��
        {
            ptr += 2;
        } else {
            ptr += 3;
        }

        n = _strlen((u8 *)ptr);
    }

    up_packeg((0x660000), ptr, n);
}

//��������
//��ӡѡ��
static void up_0x6a(void) {
    u8 n;

    if (slv_send_flag.sla_comm == 0x00) //��ѯģʽ
    {
        n = 0x00;
    } else {
        n = 1;
    }

    up_packeg((0x6a0000), (&(slv_send_flag.status)), n);
}

//��������
//�����ʶ���Ϣ
static void up_0xee_0x0000(void) {
    if ((*((u16 *)sound_play_buff)) > sound_play_buff_max_lgth)
        (*((u16 *)sound_play_buff)) = sound_play_buff_max_lgth;

    up_packeg((0xee0000), (sound_play_buff + 2), (*((u16 *)sound_play_buff)));
}

//��������
//�ն�ע��
static void up_0xee_0x0003(void) {
    u8 temp;

    up_packeg((0xee0003), (&temp), 0);
}

//��������
//�ն�ע��
static void up_0xee_0x0100(void) {
    u8 temp;

    up_packeg((0xee0100), (&temp), 0);
}

//��������
//�ն˼�Ȩ
static void up_0xee_0x0102(void) {
    u8 temp;

    up_packeg((0xee0102), (&temp), 0);
}

//��������
//�¼�����
static void up_0xee_0x0301(void) {
    up_packeg((0xee0301), (&sms_up_center.fix_id), 1);
}

//��������
//����Ӧ��
static void up_0xee_0x0302(void) {
    u8 temp[3];

    *(temp + 0) = (sms_up_center.ask_num) / 0x100;
    *(temp + 1) = (sms_up_center.ask_num) % 0x100;
    *(temp + 2) = *((sms_up_center.ask_ans) + (sms_up_center.ask_id_point));

    up_packeg((0xee0302), (temp), 3);
}

//��������
//��Ϣ�㲥
static void up_0xee_0x0303(void) {
    u8 temp[2];

    *(temp + 0) = sms_up_center.vod_type;
    *(temp + 1) = sms_up_center.vod_status;

    up_packeg((0xee0303), (temp), 2);
}

//��������
//�����˵�
static void up_0xee_0x0701(void) {
    up_packeg((0xee0701), ((u8 *)(&gb_bill_ack)), (sizeof(gb_bill_ack)));
}

//��������
//��ʻԱ��Ϣ�ϱ�
void up_0xee_0x0702(void) {
    u8 temp[200];
    u16 i;

#ifdef jtbb_test_ver //���������
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

#else //����������
    i = 0x00;
    if (state_data.state.flag.ic_flag) //ic������
    {
        *(temp + (i++)) = 0x01;
        _memcpy((temp + i), gps_data.date, 3);
        _memcpy((temp + i + 3), gps_data.time, 3);
        i += 6;

        *(temp + (i++)) = 0x00; //�����ɹ�

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

        *(temp + (i++)) = 0x00; //�����ɹ�

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

//��������ɢת��(����FLASH��)
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
��    �ƣ�slave_send_msg
��    �ܣ��ӻ���������
�����������
��    ������
**********************************************************/
//COMT:�ֱ�Э�����
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
��    �ƣ�slave_send_data_proc
��    �ܣ�����ӻ����ݵķ��ͣ����ǵ�Ƕ�׺�����������
���������ݹ���ʽ��������
�����������
��    ������
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
        if (slv_send_data.buf[1] != 0x0d) //������ȷ�������ݵ�Ӧ��ָ��ֱ�
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
