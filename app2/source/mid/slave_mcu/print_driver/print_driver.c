//*****************************************************************************//
//*****************************************************************************//
//
//  CKP  2015-07-15  �Ż��ײ�����
//
//*****************************************************************************//
//*****************************************************************************//
#include "string.h"
#include "ckp_mcu_file.h"
#include "print_driver.h"

static moto_driv_para_struct moto_driv_para;   //�����������
static print_task_para_struct print_task_para; //��ӡ�������
static pt487fb_driver_need_struct pt487fb_driver;

static uchar print_speed; //��ӡ����ӡ�ٶ�

//*****************************************************//
//*************      �رյ������         *************//
//
//*****************************************************//
static void print_moto_stop(void) {
    pt487fb_driver.prt_ma_set(_false_);
    pt487fb_driver.prt_mb_set(_false_);
    pt487fb_driver.prt_mna_set(_false_);
    pt487fb_driver.prt_mnb_set(_false_);
}

//*****************************************************//
//*************        ������ʼ��         *************//
//
//*****************************************************//
void print_data_init(pt487fb_driver_need_struct src) {
    pt487fb_driver = src;
}

//*****************************************************//
//*************        ������ʼ��         *************//
//����SPΪ��ӡ�ٶ�����    ȡֵ��Χ(50-1200)����ò�Ҫ���ٽ�ֵ
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
//*************        ��ȡ��ӡ״̬       *************//
//��������ֵ      TRUE��ʾæ״̬      FALSE��ʾ����״̬
//����ӡ������æ���򲻻���Ӧ��������
//*****************************************************//
bit_enum print_read_state(void) {
    if ((print_task_para.cnt == 0) && (print_task_para.paper_move_flag == _false_))
        return _false_; //����״̬
    else
        return _true_; //æ״̬
}

//*****************************************************//
//*************        ���ص�������       *************//
//����cntΪ����λ�ã���ΧΪ(0-15)
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

    pt487fb_driver.prt_lat_set(_false_); //�����ź�
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

    pt487fb_driver.prt_lat_set(_false_); //�����ź�
}

#endif

//*****************************************************//
//*************     ��ӡ���ֿ����ת��    *************//
//����code_buff��Ϊ����ҲΪ���������
//����flagΪ��������    TRUEΪ����������  FALSEΪASCII������
//----//
//�����ֿ��������:����ȡģ���ֽڵ���(���ֽڸ�λ���£�����������ң���������)
//����ֿ��������:����ȡģ���ֽ�˳��(���ֽڸ�λ��������������ң���������)
//----//
//*****************************************************//
static void print_font_convert(uchar *code_buff, bit_enum flag) {
    uchar buff[32];
    uchar i, j;

    memset(buff, 0x00, sizeof(buff)); //������ʱ������

    if (flag == _false_) //ASCII���ʽ
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

    else //�������ʽ
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
//*************      ���õ������         *************//
//����m_cycleΪ��������������е�������
//����m_speedΪ�������ȣ�һ�����ȵ�λ
//
//*****************************************************//
static void print_moto_config(ulong m_cycle, uint m_speed) {
    moto_driv_para.step_cycle = m_cycle;
    moto_driv_para.step_tim = m_speed;

    moto_driv_para.cnt = 0x00;
}

//*****************************************************//
//*************      �����������         *************//
//��������ֵ     TRUE��ʾ��������    FALSE��ʾ�ô��������ڽ���
//
//*****************************************************//
static bit_enum print_moto_driver(void) {
    bit_enum ret;

    if (moto_driv_para.step_cycle == 0x00) //���н���
    {
        return _false_;
    }

    moto_driv_para.cnt++;
    if (moto_driv_para.cnt < moto_driv_para.step_tim) //������ת��һ��
    {
        return _true_;
    }

    moto_driv_para.cnt = 0x00;
    ret = _true_;

    switch (moto_driv_para.step) {
    case 0x00: //��һ��
        pt487fb_driver.prt_ma_set(_false_);
        pt487fb_driver.prt_mna_set(_true_);
        pt487fb_driver.prt_mb_set(_false_);
        pt487fb_driver.prt_mnb_set(_true_);

        moto_driv_para.step = 1;
        moto_driv_para.step_cycle--;
        break;

    case 0x01: //�ڶ���
        pt487fb_driver.prt_ma_set(_false_);
        pt487fb_driver.prt_mna_set(_true_);
        pt487fb_driver.prt_mb_set(_true_);
        pt487fb_driver.prt_mnb_set(_false_);

        moto_driv_para.step = 2;
        moto_driv_para.step_cycle--;
        break;

    case 0x02: //������
        pt487fb_driver.prt_ma_set(_true_);
        pt487fb_driver.prt_mna_set(_false_);
        pt487fb_driver.prt_mb_set(_true_);
        pt487fb_driver.prt_mnb_set(_false_);

        moto_driv_para.step = 3;
        moto_driv_para.step_cycle--;
        break;

    case 0x03: //���Ĳ�
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
//*************     ��ӡ������ȴ���      *************//
//��ʼ��ӡʱ�����øú������ú����ĵ���Ƶ��Ϊ5KHz������Ϊ200΢��
//�ö�ʱ���ж���������ȷ���Ʋ������
//
//��������ֵ    TRUE��ʾ��������    FALSE��ʾ�ô��������ڽ���
//*****************************************************//
bit_enum print_base_task_hdl(void) {
    bit_enum ret;

    ret = _true_;

    if (print_task_para.paper_move_flag) //��ֽ�������ȴ���
    {
        if (!print_moto_driver()) {
            print_task_para.paper_move_flag = _false_; //��ֽ���������
            ret = _false_;
        }
    }

    else if (!print_moto_driver()) {
        pt487fb_driver.prt_heat_set(_false_); //ֹͣ����

        if (print_task_para.cnt == 0x00) {
            ret = _false_;
        } else {
            print_latch_data(16 - print_task_para.cnt);
            print_moto_config(moto_print_step_cyc_volue, print_speed);

            pt487fb_driver.prt_heat_set(_true_); //��ʼ����
            print_task_para.cnt--;
        }
    }

    return ret;
}

//*****************************************************//
//*************       ���ô�ӡ����ֽ      *************//
//��������ֵ����������ֽ�����Ƿ�ɹ�  TRUE���óɳɹ�  FALSE����ʧ��(æ״̬���)
//*****************************************************//
bit_enum print_move_paper_config(void) //û��ʹ�õ�
{
    if (print_read_state()) //æ״̬����������ֽ������Ч
    {
        return _false_;
    }

    print_moto_config(moto_noload_step_cyc_volue, print_speed);
    print_task_para.paper_move_flag = _true_; //������ֽ����

    return _true_;
}

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

//*****************************************************//
//*************        ���ֵ��󽻻�       *************//
//����xΪ�������꣬ȡֵ��ΧΪ0-47
//����nΪһ�μ��ص�����������Ϊ���У�ASCIIΪһ�У�����n��ȡֵ��ΧΪ1����2
//����s_buffΪԴ���ݻ�����
//����d_buffΪĿ�����ݻ�����
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
//*************        ��ӡһ���ַ�       *************//
//����xΪ�д�ӡ����ʼ��ַ��ȡֵ��ΧΪ0-47���ܴ�ӡ�ķ�Χ����Ϊ48���ַ���24������
//����lgthΪ����ӡ�ַ�����
//����s_buffΪ����ӡ���ַ�����
//����cnt���ػ�ʣ�´���ӡ���ֽڸ���
//��������ֵ���ر��β��������Ƿ�ִ��  TRUE�ɹ�  FALSEʧ��(æ״̬���)
//
//ע��:����ʵ�ʴ�ӡ���ֽ�����������Ϊ�س����з������ǳ����˴�ӡ��Χ
//ע��:���б���ͬʱ���ֻس����з���������Ϊ�ַ�����
//*****************************************************//
bit_enum print_one_line(uchar x, uint lgth, uchar *s_buff, uint *cnt) {
    uchar curr, f;
    uchar i, j, n;
    uchar temp_buff[32];
    uint word_addr;

    if (print_read_state()) //æ״̬�������ô�ӡ������Ч
    {
        return _false_;
    }

    if ((x > (print_line_range_max - 1)) || ((x == (print_line_range_max - 1)) && (*s_buff >= 0xa1))) //��ʼ��ַΥ�����������ӡ
    {
        *cnt = lgth;
    }

    if ((x + lgth) > print_line_range_max) //��������ֵ��������Ч��ӡ������
    {
        curr = (print_line_range_max - x);

        if (data_cnt_gb(s_buff, curr) % 2) //�ž���������ʾһ��������ͬʱҲ�ž�����Խ��
        {
            curr -= 1;
        }
    } else {
        curr = lgth;
    }

    f = 0x00;
    for (i = 0x00; i < curr; i++) {
        if ((s_buff[i] == '\x0d') && (s_buff[i + 1] == '\x0a')) {
            f = 0x01; //���б�־
            break;
        }
    }

    curr = i; //���¸�ֵ���δ�ӡ��ʵ����Ч�ֽ���

    for (i = 0x00; i < x; i++) //����δ��ӡ������ǰ
    {
        for (j = 0x00; j < 16; j++) {
            print_task_para.pixel_buff[j][i] = 0x00;
        }
    }

    n = (print_line_range_max - (x + curr)); //����δ��ӡ�������
    for (i = 0x00; i < n; i++) {
        for (j = 0x00; j < 16; j++) {
            print_task_para.pixel_buff[j][i + x + curr] = 0x00;
        }
    }

    i = 0x00;
    while (i < curr) {
        if (*(s_buff + i) >= 0xa1) {
            word_addr = (*(s_buff + i)) * 0x100 + (*(s_buff + i + 1));
            pt487fb_driver.prt_font(word_addr, temp_buff); //������
            print_font_convert(temp_buff, _true_);
            pixel_swap((x + i), 2, print_task_para.pixel_buff, temp_buff); //����������Ϣ

            i += 2; //�޸���ʾָ��
        } else {
            word_addr = *(s_buff + i);
            pt487fb_driver.prt_font(word_addr, temp_buff); //ASCII��
            print_font_convert(temp_buff, _false_);
            pixel_swap((x + i), 1, print_task_para.pixel_buff, temp_buff); //����������Ϣ

            i += 1; //�޸���ʾָ��
        }
    }

    print_task_para.cnt = print_task_cnt_cycle; //������ӡ

    if (f) //���з���Ҫ��2
    {
        curr += 2;
    }

    *cnt = (lgth - curr);

    return _true_;
}
