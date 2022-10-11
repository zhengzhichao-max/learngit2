#include "include_all.h"

const u8 data_format_error[] = "IC�����ݸ�ʽ����";

//static state_struct  tf_card_state;  //TF��״̬���
static state_struct ic_card_state; //IC��״̬���

IC_Time_t IC_Time;

//��ʼ������
void card_scan_init(void) {
    ic_type = ic_none; //��δ����

    //	tf_card_state.last = tf_card_state_default;  //Ĭ��״̬
    //	tf_card_state.filt_tim = jiffies;
    //	tf_card_state.tim = jiffies;

    ic_card_state.last = ic_card_state_default; //Ĭ��״̬
    ic_card_state.filt_tim = jiffies;
    ic_card_state.tim = jiffies;
}

//IC������ת��
static void load_driver_data(ic_card_save_format_struct *src, ic_card_date_struct *dsc) {
    (dsc->work_num_lgth) = str_len((src->work_num), 6); //����  ��˾IC�������Ϊ6�ֽڣ���ʵ���豸֧���7�ֽ�  CKP  20140618
    _memcpy((dsc->work_num), (src->work_num), (dsc->work_num_lgth));

    //logd("<%s>", src->name);
    (dsc->name_lgth) = str_len(src->name, 12); //����
    _memcpy((dsc->name), (src->name), (dsc->name_lgth));

    (dsc->id_card_lgth) = str_len((src->id_card), 20); //���֤����
    _memcpy((dsc->id_card), (src->id_card), (dsc->id_card_lgth));

    (dsc->driv_lic_lgth) = str_len((src->driv_lic), 18); //��ʻ֤����
    _memcpy((dsc->driv_lic), (src->driv_lic), (dsc->driv_lic_lgth));
    _memcpy((dsc->driv_lic_validity), (src->driv_lic_validity), 3);

    (dsc->certificate_lgth) = str_len((src->certificate), 18); //��ҵ�ʸ�֤
    _memcpy((dsc->certificate), (src->certificate), (dsc->certificate_lgth));

    (dsc->license_organ_lgth) = str_len((src->license_organ), 40); //��ҵ�ʸ�֤��֤����
    _memcpy((dsc->license_organ), (src->license_organ), (dsc->license_organ_lgth));
    _memcpy((dsc->license_organ_validity), (src->license_organ_validity), 4);
}

//IC��ɨ�貢��ʼ��IC��
//�忨//�жϿ�����//����//���ݸ�ֵ�ṹ��//END
//Rk//�忨�ϱ�.��־λ
void ic_card_scan(void) {
    static u8 last = ic_card_state_default; //��һ�ε�״̬  0��ʾ�������п�����  1��ʾ������û��
    u8 temp;
    u32 dat;
    u8 i;
    //u8 xorr = 0;
    u8 ic_data[260];
    //ic_card_save_format_struct  ic_data;
		//static state_struct ic_card_state; //IC��״̬���
    temp = state_scan((&ic_card_state), ic_insert_read(), card_scan_time_max);
    if (temp == last) //����״̬�ޱ仯���˳�����
    {
        return;
    }

    last = temp;
    if (temp == ic_card_state_default) {     //�����γ�
        IC_Time.IC_exit = mix.time;          //��¼�ο�ʱ��
        if (state_data.state.flag.ic_flag) { //�ж��Ƿ��б�Ҫ��ʾ���γ�
            ic_type = ic_none;               //���γ�
            _memset(((u8 *)(&ic_card)), 0x00, (sizeof(ic_card)));

            state_data.state.flag.ic_flag = false;
            menu_cnt.menu_flag = true; //�˵����²���

            if ((gps_data.speed[0] == 0x00) && (gps_data.speed[1] == 0x00)) {         //�ж���ǩ����
                slave_send_msg((0xee0702), 0x00, 0x00, false, spi1_up_comm_team_max); //��ʻԱ��Ϣ�ϱ�
            }
            lcd_bl_ctrl(true);
        }

        mix.ic_login = false;
        ic_driver_lisence = true;
    } else {                                                                                            //�п�����
        dat = ic4442_init();                                                                            //��ȡ��λֵ
        IC_Time.IC_enter = mix.time;                                                                    //��¼�忨ʱ��
        if ((dat == 0xffffffff) || (dat == 0x00)) {                                                     //IC2402����������
            iic_bus_init();                                                                             //��ʼ��I2C����
            if (at24cxx_read_data(at24_i2c_slave_addr, eeprom_start_addr, ic_data, eeprom_data_lgth)) { //IC�����룬��ȡ����
                ic_type = ic_2402;                                                                      //24C02��
            } else {
                ic_type = ic_error; //δ֪��Ƭ
            }
        } else { //������Ϊ4442��
            ic4442_read_data(read_main_mem, 0x00, ic_data);
            ic_type = ic_4442; //4442��
        }

        if ((ic_type == ic_2402) || (ic_type == ic_4442)) {
            lcd_bl_ctrl(true);
            menu_cnt.menu_flag = true; //�˵����²���
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
                    //COMT:��ʻԱ��Ϣ//�ؼ�����
                    _memset((u8 *)&mix.ic_driver, 0, 18);
                    _memcpy((u8 *)&mix.ic_driver, (u8 *)&driver_cfg.license, 18); //2018-1-22,�¹����,�������˾��,ƣ�ͼ�ʻ��������������

                    slave_send_msg((0xee0702), 0x00, 0x00, false, spi1_up_comm_team_max); //��ʻԱ��Ϣ�ϱ�
                }
            } else {
                state_data.state.flag.ic_flag = false; //����ʧ�ܣ�����忨��־
                menu_cnt.menu_other = mo_message_window;
                _memset(((u8 *)(&mw_dis)), '\x0', (sizeof(mw_dis)));
                mw_dis.p_dis_2ln = ((u8 *)(data_format_error));
                mw_dis.bg_2ln_pixel = 0x00; //��Ϣ��ʾ��������
            }
        } else {
            state_data.state.flag.ic_flag = false; //����ʧ�ܣ�����忨��־
        }
        mix.ic_login = true;
        ic_driver_lisence = true;
    }
}
