#include "include_all.h"

const u8 password_para_rst[] = {'\x8', '\x0', '2', '0', '0', '5', '0', '2', '1', '5'};
const u8 password_oil_rst[] = {'\x6', '\x0', '0', '0', '0', '0', '0', '0'};
const u8 password_car_rst[] = {'\x6', '\x0', '8', '8', '8', '8', '8', '8'};

const u8 password_sensor_rst[] = {'\x6', '\x0', '8', '8', '8', '8', '8', '8'};
const u8 password_speed_rst[] = {'\x6', '\x0', '8', '8', '8', '8', '8', '8'};
const u8 password_lcd_para_rst[] = {'\x6', '\x0', '8', '8', '8', '8', '8', '8'};

//"061.154.039.040" 112.051.008.008
const u8 host_set_main_ip_rst[] = {22, 0, '1', ',', '0', '6', '1', '.', '1', '5', '4', '.', '0', '3', '9', '.', '0', '4', '0', ',', '7', '6', '1', '3'};   //1,203.086.008.005,9999
const u8 host_set_backup_ip_rst[] = {22, 0, '1', ',', '2', '0', '3', '.', '0', '8', '6', '.', '0', '0', '8', '.', '0', '6', '6', ',', '9', '9', '9', '9'}; //1,203.086.008.005,9999
const u8 host_set_id_rst[] = {12, 0, '0', '6', '4', '1', '9', '4', '2', '8', '3', '6', '6', '6'};                                                          //13012345678
const u8 host_set_apn_rst[] = {5, 0, 'C', 'M', 'N', 'E', 'T'};                                                                                             //CMNET
const u8 host_set_server_rst[] = {11, 0, '1', '3', '8', '2', '7', '4', '0', '3', '5', '5', '9'};                                                           //13827403559
const u8 host_set_user_rst[] = {11, 0, '1', '3', '8', '2', '7', '4', '0', '3', '5', '5', '9'};                                                             //13827403559

const u8 rec_ratio_rst[] = "\x4\x0"
                           "1000";             //1000
const u8 rec_car_vin_rst[] = "\x0\x0";         //
const u8 rec_car_plate_rst[] = "\x0\x0";       //
const u8 rec_province_id_rst[] = "\x0\x0";     //
const u8 rec_city_id_rst[] = "\x0\x0";         //
const u8 rec_manufacturer_id_rst[] = "\x0\x0"; //
const u8 rec_terminal_type_rst[] = "\x0\x0";   //
const u8 rec_terminal_id_rst[] = "\x0\x0";     //

//-------------------------------------
static void memory_init(void) {
    jiffies = 0x00;

    _memset(((u8 *)(&user_time)), 0x00, (sizeof(user_time)));
    _memset(((u8 *)(&host_no_save_para)), 0x00, (sizeof(host_no_save_para)));
    host_no_save_para.set_gps_module_type = 3; //Ĭ��˫ģģʽ
}

//-------------------------------------
//�ⲿ�洢FLASHоƬ��ʼ��
void ex_flash_init(bool rst) {
    u8 buff[flash_parameter_max_lgth];

    _memset(((u8 *)(&password_manage)), '\x0', (sizeof(password_manage)));
    _memset(((u8 *)(&set_para)), '\x0', (sizeof(set_para)));
    _memset(((u8 *)(&recorder_para)), '\x0', (sizeof(recorder_para)));

#if 1
    spi_flash_read(buff, flash_parameter_addr, flash_parameter_max_lgth); //��ȡ����

    if (((*(buff + flash_flag_1_addr)) != spi_25flash_flag_1) || ((*(buff + flash_flag_2_addr)) != spi_25flash_flag_2)
        || ((*(buff + flash_flag_3_addr)) != spi_25flash_flag_3) || (rst)) //�ж��Ƿ���Ҫ��ʼ������
    {
        //--------//
        sms_data_flash_init(); //��Ϣ
        IWDG_ReloadCounter();  //ι������
        //--------//

        //--------//
        phbk_data_flash_init(); //�绰��
        IWDG_ReloadCounter();   //ι������
                                //--------//

        //--------//
        call_record_data_flash_init(); //ͨ����¼
        IWDG_ReloadCounter();          //ι������
        //--------//

        //--------//
        _memset(buff, 0x00, flash_parameter_max_lgth); //���㻺����

        _memset(((u8 *)(&password_manage)), '\x0', (sizeof(password_manage)));
        _memcpy(password_manage.para, ((u8 *)(password_para_rst)), (sizeof(password_para_rst)));
        _memcpy(password_manage.oil, ((u8 *)(password_oil_rst)), (sizeof(password_oil_rst)));
        _memcpy(password_manage.car, ((u8 *)(password_car_rst)), (sizeof(password_car_rst)));
        _memcpy(password_manage.sensor, ((u8 *)(password_sensor_rst)), (sizeof(password_sensor_rst)));
        _memcpy(password_manage.speed, ((u8 *)(password_speed_rst)), (sizeof(password_speed_rst)));
        _memcpy(password_manage.lcd_para_rst, ((u8 *)(password_lcd_para_rst)), (sizeof(password_lcd_para_rst)));
        //_memcpy(password_manage.regist, ((u8 *)(password_regist_rst)), (sizeof(password_regist_rst)));
        _memcpy((buff + flash_password_addr), ((u8 *)(&password_manage)), (sizeof(password_manage)));

        _memset(((u8 *)(&set_para)), '\x0', (sizeof(set_para)));
        _memcpy(set_para.m_ip, ((u8 *)(host_set_main_ip_rst)), (sizeof(host_set_main_ip_rst)));
        _memcpy(set_para.b_ip, ((u8 *)(host_set_backup_ip_rst)), (sizeof(host_set_backup_ip_rst)));
        _memcpy(set_para.id, ((u8 *)(host_set_id_rst)), (sizeof(host_set_id_rst)));
        _memcpy(set_para.apn, ((u8 *)(host_set_apn_rst)), (sizeof(host_set_apn_rst)));
        _memcpy(set_para.num_s, ((u8 *)(host_set_server_rst)), (sizeof(host_set_server_rst)));
        _memcpy(set_para.num_u, ((u8 *)(host_set_user_rst)), (sizeof(host_set_user_rst)));

        set_para.bl_type = 30; //��ʼ����LCD�������ֵΪ30,����ACC����ʱ��,30,��ACC�ص�ʱ����30��,�Ҳ��������� 2018-3-29 LUJING
        set_para.buzzer_type = 0;

        set_para.LCD_Contrast = 0x28; //������ʼ�����LCD�Աȶ�
        //lcd_init(_true_);  /* ??????????? */

        _memcpy((buff + flash_set_para_addr), ((u8 *)(&set_para)), (sizeof(set_para)));

        _memset(((u8 *)(&recorder_para)), '\x0', (sizeof(recorder_para)));
        _memcpy(recorder_para.province_id, ((u8 *)(rec_province_id_rst)), (sizeof(rec_province_id_rst) - 1));
        _memcpy(recorder_para.city_id, ((u8 *)(rec_city_id_rst)), (sizeof(rec_city_id_rst) - 1));
        _memcpy(recorder_para.manufacturer_id, ((u8 *)(rec_manufacturer_id_rst)), (sizeof(rec_manufacturer_id_rst) - 1));
        _memcpy(recorder_para.terminal_type, ((u8 *)(rec_terminal_type_rst)), (sizeof(rec_terminal_type_rst) - 1));
        _memcpy(recorder_para.terminal_id, ((u8 *)(rec_terminal_id_rst)), (sizeof(rec_terminal_id_rst) - 1));
        _memcpy(recorder_para.car_plate, ((u8 *)(rec_car_plate_rst)), (sizeof(rec_car_plate_rst) - 1));
        _memcpy(recorder_para.car_vin, ((u8 *)(rec_car_vin_rst)), (sizeof(rec_car_vin_rst) - 1));
        _memcpy(recorder_para.ratio, ((u8 *)(rec_ratio_rst)), (sizeof(rec_ratio_rst) - 1));
        _memcpy((buff + flash_recorder_para_addr), ((u8 *)(&recorder_para)), (sizeof(recorder_para)));

        (*(buff + flash_flag_1_addr)) = spi_25flash_flag_1;
        (*(buff + flash_flag_2_addr)) = spi_25flash_flag_2;
        (*(buff + flash_flag_3_addr)) = spi_25flash_flag_3;

        flash25_program_auto_save(flash_parameter_addr, buff, flash_parameter_max_lgth); //��FLASH��д������
                                                                                         //--------//
    }

    spi_flash_read(buff, flash_parameter_addr, flash_parameter_max_lgth); //������ȡ����
    _memcpy(((u8 *)(&password_manage)), (buff + flash_password_addr), (sizeof(password_manage)));
    _memcpy(((u8 *)(&set_para)), (buff + flash_set_para_addr), (sizeof(set_para)));
    _memcpy(((u8 *)(&recorder_para)), (buff + flash_recorder_para_addr), (sizeof(recorder_para)));
#endif
}

//*****************************************************************************************
//ϵͳ����
//
//*****************************************************************************************
void system_config(void) {
    memory_init();
    ///hard_config();
    ex_flash_init(false);
}

//�������
//����en    TRUE��ʾ��    FALSE��ʾ�ر�
void lcd_bl_ctrl(bool en) {
    if (en) {
        lcd_bl_on;
        user_time.bl = jiffies;
    } else {
        lcd_bl_off;
    }
}

//*****************************************************************************************
//�ú����Ĺ����Ǹ����ṩ�ĵ�ַ(����)��ȡ����Ӧ�ĵ�����Ϣ��ָ���Ļ�����
//
//����addrָ������ַ��ĵ�ַ(����)
//����*code_buff��ָ�����ҵ�������Ϣ�󽫽���ŵ��û�������
//����flagָ�����ַ��ı��뷽ʽ     FALSE��ʾASCII      TRUE��ʾGB2312
//
//*****************************************************************************************
void read_flash_fonts(u16 addr, u8 *code_buff) {
    u32 temp_u32;
    u16 temp_uint;
    u8 th, tl;
    u8 i;

    i = 0x00;

    if (addr >= 0x100) {
        temp_uint = (addr - 0xa1a1);
        th = (temp_uint / 0x100);
        tl = (temp_uint % 0x100);

        temp_u32 = (((th * 94) + tl) * 32 + font_gb2312_start_addr);
        i = 32;
    } else {
        i = 16;

        if ((addr > 0x1f) && (addr < 0x7f)) //Ӣ�Ŀɴ�ӡ�ַ����ֿⷶΧС��0x7f������0x1f
        {
            temp_u32 = ((addr - 0x20) * 16 + font_ascii_start_addr);
        } else //��������ʾ��Χ������ʾ�հ�����
        {
            temp_u32 = font_ascii_start_addr;
        }
    }

    if (i) {
        spi_flash_read(code_buff, temp_u32, i); //����Ѱַ
    }
}
//********************************************************************//
//********************************************************************//

//�豸���Լ�����
//����srcΪ��д��Ŀ�������
//�������ؿ���ִ�н��
bit_enum ic_card_write_self(ic_card_date_struct *src) {
    ic_card_save_format_struct card_dat;
    uchar n;
    bit_enum ret;

    ret = _false_;
    memset(((uchar *)(&card_dat)), 0x00, (sizeof(ic_card_save_format_struct)));

    if (src->work_num_lgth > 6) {
        n = 6;
    } else {
        n = src->work_num_lgth;
    }
    memcpy(card_dat.work_num, src->work_num, n);

    if (src->name_lgth > 12) {
        n = 12;
    } else {
        n = src->name_lgth;
    }
    memcpy(card_dat.name, src->name, n);
    logd("<%s>", card_dat.name);

    if (src->id_card_lgth > 20) {
        n = 20;
    }
    else {
        n = src->id_card_lgth;
    }
    memcpy(card_dat.id_card, src->id_card, n);

    if (src->driv_lic_lgth > 18) {
        n = 18;
    } else {
        n = src->driv_lic_lgth;
    }
    memcpy(card_dat.driv_lic, src->driv_lic, n);
    memcpy(card_dat.driv_lic_validity, src->driv_lic_validity, 3); //��ʻ֤��Ч��

    if (src->certificate_lgth > 18) {
        n = 18;
    } else {
        n = src->certificate_lgth;
    }
    memcpy(card_dat.certificate, src->certificate, n);
    memcpy(card_dat.license_organ_validity, src->license_organ_validity, 4); //��ҵ�ʸ�֤��Ч��

    if (src->license_organ_lgth > 40) {
        n = 40;
    } else {
        n = src->license_organ_lgth;
    }
    memcpy(card_dat.license_organ, src->license_organ, n);

    card_dat.xor = _get_xor(((uchar *)(&card_dat)), ((sizeof(ic_card_save_format_struct)) - 1));

    if (ic_type == ic_2402) { //IC2402
        //ic4442д��:
        if (at24cxx_write_data(at24_i2c_slave_addr, eeprom_start_addr, ((uchar *)(&card_dat)), eeprom_data_lgth)) {
            ret = _true_;
        }
    } else if (ic_type == ic_4442) { //IC4442
        //ic4442д��:
        if (ic4442_write_data(ic4442_default_password, write_main_mem, 0x00, ((uchar *)(&card_dat)), eeprom_data_lgth)) {
            ret = _true_;
        }
    }

    return ret;
}
