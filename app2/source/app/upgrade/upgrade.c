/**
  ******************************************************************************
  * @file    upgrade.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  Զ����������
  ******************************************************************************
  * fat_read_file
  *
  ******************************************************************************
*/
#include "include_all.h"

SPI_FLASH_SAVE sp;          //�ļ���Ϣ
s_UPDATAECONTROL upgrade_m; //�������ƽṹ��

font_lib_update_struct font_lib_update;  //�ֿ������������
update_tf_data_struct update_tf_data;    //TF����������
parameter_cfg_struct parameter_auto_cfg; //�����Զ�����2017-12-6
read_rcd_data_struct read_rcd_data_cfg;  //��¼�����ݶ�ȡ�ṹ�� 2017-12-15
u8 update_buf[2100];                     //�������ݻ���

upgrade_status_flag usb_upgrade; //usb������־�ṹ��

const u8 black_display[16] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

u32 ApplicationFlag; //Զ��������־λ

void init_upgrade(void) {
    _memset((u8 *)&upgrade_m, 0x00, sizeof(upgrade_m));
}

/*********************************************************
��	  �ƣ�check_update_version
��	  �ܣ������汾�ż��
�����������������ַ��
���������
��	  �أ�
ע�����
�޸ļ�¼:22/07/25
*********************************************************/
u8 check_update_version(u8 type, u8 *buf, u16 len) {
    u8 buf1[16];
    u8 buf2[16];
    u32 ver1;
    u32 ver2;

    if (len < 24 || len > 30)
        return 0;

    upgrade_m.filename_len = _memcpy_len(upgrade_m.filename, buf, len); //ֻ�ж�24λ//6

    if (_strncmp((u8 *)&upgrade_m.filename[0], PRODUCT_NAME, 8) != 0)
        return 0;

    if (_strncmp(&upgrade_m.filename[9], PCB_VER, 3) != 0) {
        return 0;
    }

    if (_strncmp(&upgrade_m.filename[13], FW_VER, 4) != 0) {
        return 0;
    }

    if (type == UPGRADE_BYPASS)
        return 1;

    _memcpy(buf1, FW_TIME, 6);
    buf1[6] = '\0';
    ver1 = atol((const char *)buf1);
    _memcpy(buf2, &upgrade_m.filename[18], 6);
    buf2[6] = '\0';
    ver2 = atol((const char *)buf2);

    if (ver2 == ver1) {
        loge("OTA ERR: not newest ver");
        return 0;
    }

    return 1;
}

/****************************************************************************
* ����:    Flash_Write_BLParameters ()
* ���ܣ�д��Զ��������־λ
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
static bool Flash_Write_BLParameters(void) {
    if (upgrade_m.file_len > APP_MAX_DATA_LEN)
        return false;

    sp.ApplicationFlag = 0x87654321;
    sp.ProgramDataLen = upgrade_m.file_len;
    sp.ProgramXorValue = upgrade_m.xor ;
    sp.ProgramAddCheckValue = upgrade_m.sum;

    add_struct_crc((u8 *)&sp, sizeof(sp));
    spi_flash_erase(ADDR_IAP_TMP);

    spi_flash_write((u8 *)&sp, ADDR_IAP_TMP, sizeof(sp));

    logd("BL write :l:%ld, or:%d, sum:%ld", sp.ProgramDataLen, sp.ProgramXorValue, sp.ProgramAddCheckValue);
    return true;
}

typedef enum {
    UPDATE_DATA_RECEIVE_OTA_REQUST = 0,
    UPDATE_DATA_RECEIVE_OTA_SUCCESS = 1,
    UPDATE_DATA_RECEIVE_OTA_FAIL = 2,
    UPDATE_DATA_RECEIVE_OTA_LEN_ERR = 3,
} UPDATE_DATA_RECEIVE_enum;
/*********************************************************
��	  �ƣ�Update_Data_Receive
��	  �ܣ���������д�뻺����
�����������������ַ��
���������
��	  �أ�
ע�����
*********************************************************/
static UPDATE_DATA_RECEIVE_enum Update_Data_Receive(u8 *buf, u16 len) {
    u16 uNo;
    u16 i;
    u8 *p;
    u32 iflen = 0;
    u32 sector = 0;
    static u32 old_sector = 0;
    u32 addr = 0;

    if (upgrade_m.read_len > APP_MAX_DATA_LEN || upgrade_m.file_len > APP_MAX_DATA_LEN) {
        return UPDATE_DATA_RECEIVE_OTA_FAIL;
    }

    len -= 2;
    if (len > 512) {
        return UPDATE_DATA_RECEIVE_OTA_LEN_ERR;
    }

    _sw_endian((u8 *)&uNo, buf, 2); //�ڼ���
    buf += 2;
    if (uNo == upgrade_m.pack_no) {
        iflen = upgrade_m.file_len - upgrade_m.read_len;
        if (iflen > 512) {
            if (len != 512)
                return UPDATE_DATA_RECEIVE_OTA_LEN_ERR; //����
        } else {
            if (len != iflen)
                return UPDATE_DATA_RECEIVE_OTA_LEN_ERR;
        }

        p = buf;
        for (i = 0; i < len; i++) {
            upgrade_m.sum_tmp += *p;
            upgrade_m.xor_tmp ^= *p;
            p++;
        }

        sector = (upgrade_m.read_len + ADDR_IAP_MIN) / 0x1000;
        if (sector != old_sector) {
            old_sector = sector;
            spi_flash_erase((sector * 0x1000));
        }

        spi_flash_write(buf, _upg_addr(upgrade_m.read_len), len);
        upgrade_m.read_len += len;

        //��Ҫ�ж��Ƿ��Ѿ��������
        if (++upgrade_m.pack_no >= upgrade_m.pack_num) //�ļ��������
        {
            if (upgrade_m.sum == upgrade_m.sum_tmp) {
                return UPDATE_DATA_RECEIVE_OTA_SUCCESS; //�ɹ�
            } else {
#if 1 //�˴�������������
                write_uart1("Update File Start\r\n", 14);
                addr = 0;
                while (t1_tail != t1_head)
                    ;
                while (addr < upgrade_m.read_len) {
                    sector = (addr + ADDR_IAP_MIN) / 0x1000;
                    iflen = upgrade_m.read_len - addr;
                    if (iflen > 512) {
                        len = 512;
                    } else {
                        len = iflen;
                    }
                    addr += len;
                    _memset((u8 *)&update_buf, 0, 530);
                    spi_flash_read((u8 *)&update_buf, _upg_addr(addr), len);
                    write_uart1((u8 *)&update_buf, len);
                }
#endif
                write_uart1("Update File End!\r\n", 14);
                return UPDATE_DATA_RECEIVE_OTA_FAIL;
            }
        }
    }

    return UPDATE_DATA_RECEIVE_OTA_REQUST;
}

/*********************************************************
��	  �ƣ�update_from_sd_mess
��	  �ܣ�ͨ��SD����Ϣ�ж�
���������
��	  �أ�
ע�����
*********************************************************/
void update_from_sd_mess(u8 *buf, u16 len) {
    u8 ret;
    u8 buf1[32];

    _memset((u8 *)&upgrade_m, 0, sizeof(upgrade_m));
    ret = check_update_version(UPGRADE_BYPASS, buf, 24); //len
    if (ret == 0 || upgrade_m.enable == true) {
        buf1[0] = 0x01;
        handle_send_base_msg(FROM_INNER, UPDATE_FROM_SD, buf1, 1);
        logi("��������!");
    } else {
        upgrade_m.enable = true;
        upgrade_m.mode = UPGRADE_SD;
        buf += 40;
        _sw_endian((u8 *)&upgrade_m.pack_num, buf, 2);
        buf += 2;
        _sw_endian((u8 *)&upgrade_m.file_len, buf, 4);
        buf += 4;
        upgrade_m.xor = *buf++;
        _sw_endian((u8 *)&upgrade_m.sum, buf, 4);
        logi("��ʼ�������ȴ��������ݰ�...");
        logd("BL down : l:%ld, or:%d, sum:%ld", upgrade_m.file_len, upgrade_m.xor, upgrade_m.sum);
    }
}

/**************************************************************************
* ����:    verify_spi_flash ()
* ���ܣ�Զ������������У��
* ��ڲ�������
* ���ڲ�����
***************************************************************************/
//ret: int, 0,success; -1,fail
static int verify_spi_flash(void) {
    u32 address_t = 0;
    u32 add_t = 0;
    u8 xor_t = 0;
    u8 buf[1050];
    u16 len = 0;
    u32 sector = 0;
    u32 i = 0;
    u16 j = 0;

    if (sp.ProgramDataLen > APP_MAX_DATA_LEN) {
        loge("sp.ProgramDataLen > %d", APP_MAX_DATA_LEN);
        return -1;
    }

    address_t = 0;
    xor_t = 0;
    add_t = 0;
    //if( sp.ProgramDataLen%0x0400 != 0 )
    sector = (sp.ProgramDataLen / 0x0400) + 1;
    //else
    //	sector = (sp.ProgramDataLen / 0x0400) ;

    for (i = 0; i < sector; i++) {
        if (address_t >= sp.ProgramDataLen)
            break;

        if ((sp.ProgramDataLen - address_t) >= 0x0400)
            len = 0x0400;
        else
            len = sp.ProgramDataLen - ((sp.ProgramDataLen / 0x0400) * 0x0400);

        spi_flash_read((u8 *)&buf, (address_t + ADDR_IAP_MIN), len);
        for (j = 0; j < len; j++) {
            add_t = add_t + buf[j];
            xor_t = xor_t ^ buf[j];
        }
        address_t += len;
    }

    if ((sp.ProgramXorValue == xor_t) && (sp.ProgramAddCheckValue == add_t)) { //���У�����ȷ
        return 0;
    } else {
        loge("chk sum err");
        return -1;
    }
}

//ret: int, 0,success; -1,fail//������������ڴ���֤
static int check_flash_update_from_tr9(void) {
    int ret;

    ret = verify_spi_flash();
    if (ret == 0) {
        Flash_Write_BLParameters();
        logd("SPI FLASH У����ȷ���ȴ�����");
    } else {
        loge("SPI FLASH У�����");
    }

    return ret;
}

/****************************************************************************
* ����:    upgrade_program_task ()
* ���ܣ�Զ�������Ĺ�������
* ��ڲ�������
* ���ڲ�������
***************************************************************************/
void upgrade_program_task(void) {
    static enum E_UPDATE_MANAGE {
        UPDATE_IDLE,
        UPDATE_OTA_GET_VER, //����Զ������������
        UPDATE_GACK,        //Զ�̷���������
        UPDATE_OTA_LINK,    //����Զ������������
        UPDATE_LACK,        //Զ�̷���������
        UPDATE_OTA_REQUST,
        UPDATE_OTA_RACK,
        UPDATE_SPI_VERIFY,
        UPDATE_FINISH,
        UPDATE_ERR
    } step = UPDATE_IDLE;

    enum E_UPDATE_MANAGE old_step = UPDATE_ERR;

    static bool new_update_step = false;
    u8 buf[512] = {0x00};
    u16 blen = 0;
    static u32 delay = 0;
    u8 len = 0;

    if (!upgrade_m.enable) {
        step = UPDATE_IDLE;
        return;
    }
    old_step = step;

    switch (step) {
    case UPDATE_IDLE:
        if ((upgrade_m.mode == UPGRADE_OTA) && (lm.reset || gc.gsm.reset || lm.sms_rst_en))
            break;

        if (upgrade_m.enable) {
            upgrade_m.enable_tick = tick;
            upgrade_m.err_cnt = 0;
            upgrade_m.pack_no = 0;
            upgrade_m.ack = false;
            read_sms_tick = tick;
            if (upgrade_m.mode == UPGRADE_OTA) {
                //���²���
                gs.gprs1.stat = NET_LOST;
                lm.reset = true;
                step = UPDATE_OTA_GET_VER;
            } else
                step = UPDATE_ERR;
        }
        break;
    case UPDATE_OTA_GET_VER:
        if (new_update_step) {
            delay = tick;
            lm.sms_rst_en = true;
            lm.sms_rst_tick = tick;
            lm.cur_ip = LINK_IP2;
            lm.IP2_status = LINK_DIAL;
        }

        if (_covern(delay) > 180) {
            step = UPDATE_ERR;
            return;
        }

        if (lm.reset || gc.gsm.reset || lm.sms_rst_en)
            break;

        if (gs.gprs1.stat < NET_ONLINE)
            break;

        blen = 0;
        if (upgrade_m.newest) //���°汾
        {
            buf[blen++] = 0x01;
            buf[blen++] = 0x00;
            buf[blen++] = 24;
            blen += _memcpy_len(&buf[blen], (u8 *)FW_VERSION, 24);
        } else //ָ���汾
        {
            buf[blen++] = 0x01;
            buf[blen++] = 0x01;
            buf[blen++] = upgrade_m.filename_len;
            blen += _memcpy_len(&buf[blen], upgrade_m.filename, upgrade_m.filename_len);
        }

        pack_any_data(CMD_UP_UPDATE, buf, blen, PK_NET, LINK_IP0);
        upgrade_m.ack = false;
        upgrade_m.ack_tick = tick;
        upgrade_m.buf_len = 0;
        _memset((u8 *)&update_buf, 0x00, 530);
        step = UPDATE_GACK;
        break;
    case UPDATE_GACK:
        if (new_update_step)
            delay = tick;

        if (_covern(delay) > 180) {
            step = UPDATE_ERR;
            break;
        }

        if (upgrade_m.ack) {
            upgrade_m.ack = false;
            step = UPDATE_OTA_LINK;
            break;
        }

        if (_coveri(upgrade_m.ack_tick) > 20) {
            upgrade_m.ack_tick = tick;
            if (++upgrade_m.err_cnt > 5) {
                step = UPDATE_ERR;
                break;
            } else {
                step = UPDATE_OTA_GET_VER;
                break;
            }
        }
        break;
    case UPDATE_OTA_LINK:
        if (new_update_step)
            delay = tick;

        if (_covern(delay) > 180) {
            step = UPDATE_ERR;
            break;
        }

        if (lm.reset || gc.gsm.reset || lm.sms_rst_en)
            break;

        if (gs.gprs1.stat < NET_ONLINE)
            break;

        len = _strlen(upgrade_m.filename);
        if (len < 24 || len != upgrade_m.filename_len) {
            step = UPDATE_ERR;
            break;
        }

        buf[0] = 0x03;
        buf[1] = 0x03;
        buf[2] = upgrade_m.filename_len;
        _memcpy(&buf[3], (u8 *)&upgrade_m.filename, upgrade_m.filename_len);
        pack_any_data(CMD_UP_UPDATE, buf, upgrade_m.filename_len + 3, PK_NET, LINK_IP0);

        upgrade_m.ack = false;
        upgrade_m.banben = false;
        upgrade_m.ack_tick = tick;
        upgrade_m.buf_len = 0;
        _memset((u8 *)&update_buf, 0x00, 530);
        step = UPDATE_LACK;
        break;
    case UPDATE_LACK:
        if (new_update_step){
            delay = tick;
        }

        if (_covern(delay) > 180) {
            step = UPDATE_ERR;
            loge("delay over 180");
            break;
        }

        if (upgrade_m.banben) {
            upgrade_m.banben = false;
            step = UPDATE_OTA_REQUST;
            break;
        }

        if (_coveri(upgrade_m.ack_tick) > 10) {
            upgrade_m.ack_tick = tick;
            if (++upgrade_m.err_cnt > 5) {
                step = UPDATE_ERR;
                break;
            } else {
                step = UPDATE_OTA_LINK; //UPDATE_OTA_REQUST;
                break;
            }
        }
        break;
    case UPDATE_OTA_REQUST:{
        if (new_update_step) {
            delay = tick;
            read_sms_tick = tick;
        }

        if (_covern(delay) > 180) {
            step = UPDATE_ERR;
            loge("delay over 300");
            break;
        }

        if (gs.gprs1.stat < NET_ONLINE)
            break;

        len = _strlen(upgrade_m.filename);
        if (len < 24 || len > 33) {
            step = UPDATE_ERR;
            break;
        }

        buf[0] = 0x02;
        buf[1] = upgrade_m.filename_len;
        _memcpy(&buf[2], (u8 *)&upgrade_m.filename, upgrade_m.filename_len);
        buf[upgrade_m.filename_len + 2] = upgrade_m.pack_no >> 8;
        buf[upgrade_m.filename_len + 3] = upgrade_m.pack_no & 0x00ff;
        pack_any_data(CMD_UP_UPDATE, buf, upgrade_m.filename_len + 4, PK_HEX, LINK_IP0);
        loge("OTA��������:%d, err_cnt:%d", upgrade_m.pack_no, upgrade_m.err_cnt);
        upgrade_m.ack = false;
        upgrade_m.ack_tick = tick;
        upgrade_m.buf_len = 0;
        _memset((u8 *)&update_buf, 0x00, 530);
        step = UPDATE_OTA_RACK;
        break;
    }
    case UPDATE_OTA_RACK: {
        if (new_update_step)
            delay = tick;

        if ((_covern(delay) > 600)) {
            step = UPDATE_ERR;
            loge("delay over 600");
            break;
        }

        if (upgrade_m.ack) {
            upgrade_m.ack = false;
            upgrade_m.err_cnt = 0;

            UPDATE_DATA_RECEIVE_enum udr = Update_Data_Receive((u8 *)&update_buf, upgrade_m.buf_len);
            if (udr == UPDATE_DATA_RECEIVE_OTA_REQUST) {
                upgrade_m.err_cnt = 0;
                step = UPDATE_OTA_REQUST;
                break;
            } else if (udr == UPDATE_DATA_RECEIVE_OTA_SUCCESS) { //�����ɹ�
                upgrade_m.err_cnt = 0;
                buf[0] = 0x03;
                buf[1] = 0x01;
                pack_any_data(CMD_UP_UPDATE, buf, 2, PK_NET, LINK_IP0);
                logi("�����ɹ�");
                step = UPDATE_SPI_VERIFY;
                break;
            } else if (udr == UPDATE_DATA_RECEIVE_OTA_FAIL) {
                buf[0] = 0x03;
                buf[1] = 0x02;
                pack_any_data(CMD_UP_UPDATE, buf, 2, PK_NET, LINK_IP0);
                loge("����ʧ��");
                step = UPDATE_ERR;
            } else {
                logd("other, OTA_ack = %d, %d", upgrade_m.pack_no, (int)udr);
                goto err_ota_overtime;
            }
        }

        if (_coveri(upgrade_m.ack_tick) > 30) { //30
        err_ota_overtime:
            upgrade_m.ack_tick = tick;
            if (++upgrade_m.err_cnt > 3) {
                logi("upgrade_m.err_cnt 550");
                gsm_power_down();
                break;
            } else {
                step = UPDATE_OTA_REQUST;
                break;
            }
        }
        break;
    }
    case UPDATE_SPI_VERIFY: {
        int ret = check_flash_update_from_tr9();
        if (ret == 0) {
            step = UPDATE_FINISH;
        } else {
            step = UPDATE_ERR;
        }
        break;
    }
    case UPDATE_FINISH:
        logi("���뿴�Ź���λ");

        log_write(event_mcu_boot_ota);

        while (1) {
            nop();
        }
    case UPDATE_ERR:
        //if (upgrade_m.mode == UPGRADE_OTA)
        {
            lm.reset = true;
            lm.IP2_status = LINK_VOID;
            lm.cur_ip = LINK_IP0;
        }

        _memset((u8 *)&upgrade_m, 0x00, sizeof(upgrade_m));
        step = UPDATE_IDLE;
        break;
    default:
        step = UPDATE_IDLE;
        break;
    }

    new_update_step = (step == old_step) ? false : true;
}

void auto_ack_tr9_data(void) {
    if (font_lib_update.start_m == false)
        return;

    if (_pastn(font_lib_update.tmr) >= 600) {
        tr9_frame_pack2rk(tr9_cmd_4041, &font_lib_update.write_pack, 1);
        font_lib_update.tmr = jiffies;
    }
}

//*************************************************//
//��˼�����SD���з�����masterXXXXXX.bin  (XXXXXXΪ���������գ���Ϊ��??�ļ�??
//����Ϊ�ǵ�Ƭ�������ļ���������Э���͸���Ƭ����Ҫ��Ƭ��ÿ����ҪӦ��
//1�ֽ��ܰ� + 1�ֽڵ�ǰ���� + 2048�ֽ�����
//*************************************************//
void update_tf_main(u8 *lib, u16 len) {
    u8 *pf = NULL;
    static u8 pack = 1;
    u16 wl;
    u16 i;
    static u8 ecnt = 0;

    if (font_lib_update.start_m == false)
        return;

    pf = lib;
    wl = 1024;

    font_lib_update.tmr = jiffies;
    font_lib_update.total_pack = *pf++; //�ܰ���
    pack = *pf++;                       //��ǰ���

    if (pack == 0x01) {
        font_lib_update.cnt = 0;
        update_tf_data.check_sum = 0;
        update_tf_data.check_xor = 0;
        font_lib_update.start_m = true;

        spi_flash_erase(software_iap_addr + font_lib_update.cnt);
        spi_flash_write(pf, (software_iap_addr + font_lib_update.cnt), wl);

        update_tf_data.wr_lgth = font_lib_update.total_pack * 1024;
        for (i = 0x00; i < wl; i++) {
            update_tf_data.check_sum = update_tf_data.check_sum + pf[i];
            update_tf_data.check_xor = update_tf_data.check_xor ^ pf[i];
        }
        font_lib_update.write_pack = 2;
        tr9_frame_pack2rk(tr9_cmd_4041, &font_lib_update.write_pack, 1);

        font_lib_update.cnt += wl;
        _printf_tr9("####TR9 update start####\r\n");
    } else //��ʼ����
    {
        if ((font_lib_update.write_pack) != pack) {
            if (++ecnt > 20) {
                _memset((u8 *)&font_lib_update, 0, sizeof(font_lib_update));
                ecnt = 0;
                pack = 0;

                tr9_frame_pack2rk(tr9_cmd_4041, &font_lib_update.write_pack, 1);
                return;
            };
            tr9_frame_pack2rk(tr9_cmd_4041, &font_lib_update.write_pack, 1);
            _printf_tr9("####TR9 update pack no same=%d####\r\n", font_lib_update.write_pack);
            return;
        }
        _printf_tr9("####TR9 update pack=%d ####\r\n", pack);
        ecnt = 0;

        if ((font_lib_update.write_pack) % 4 == 1) //1234  5678  9101112
        {
            spi_flash_erase(software_iap_addr + font_lib_update.cnt);
        }
        spi_flash_write(pf, (software_iap_addr + font_lib_update.cnt), wl);
        for (i = 0x00; i < wl; i++) {
            update_tf_data.check_sum = update_tf_data.check_sum + pf[i];
            update_tf_data.check_xor = update_tf_data.check_xor ^ pf[i];
        }
        font_lib_update.cnt += wl;
        //д����ɣ�
        if (font_lib_update.write_pack >= font_lib_update.total_pack) {
            sp.ApplicationFlag = 0x87654321;
            upgrade_m.file_len = font_lib_update.cnt;
            sp.ProgramDataLen = font_lib_update.cnt;
            upgrade_m.xor = update_tf_data.check_xor;
            sp.ProgramXorValue = update_tf_data.check_xor;

            sp.ProgramAddCheckValue = update_tf_data.check_sum;
            upgrade_m.sum = update_tf_data.check_sum;
            font_lib_update.start_m = false;

            pack = 0;
            tr9_frame_pack2rk(tr9_cmd_4041, &pack, 1);
            tr9_frame_pack2rk(tr9_cmd_1818, 0, 0); //请求关闭KR电源

            if (check_flash_update_from_tr9() == 0) {
                t_rst.rst = true;
                t_rst.res = true;
                t_rst.jiff = jiffies;
                font_lib_update.start_m = false;
                font_lib_update.start_m = false;
            }
            return;
        }
        font_lib_update.write_pack++;
        tr9_frame_pack2rk(tr9_cmd_4041, &font_lib_update.write_pack, 1);
    }
}

#if 1

/*[23:53:11.614]�ա���#write boot len=161792, add_t=16934940,xor_t=208
##У��ʧ��##
BL write :l:163840, or:141, sum:17158373 */

///////////////////////////����д���ֿ⺯��///////////////////////////////

static bool get_uart_is_tr9(u8 from_t) {
    if (u1_m.cur.b.tr9 == true && from_t == FROM_U1) {
        return true;
    } else {
        return false;
    }
}

static void SendAckToPc(void) {
    u8 AckToPC[10];

    AckToPC[0] = 0x08;
    AckToPC[1] = 0x24;
    AckToPC[2] = 0x24;
    AckToPC[3] = 0x02;
    AckToPC[4] = 0x00;
    AckToPC[5] = 0x02;
    AckToPC[6] = 0x00; //
    AckToPC[7] = 0x0d; //

    write_uart1(AckToPC + 1, 7);
}

void SendAskNumToPc(void) {
    u8 AckToPC[10];

    AckToPC[0] = 0x09;
    AckToPC[1] = 0x24;
    AckToPC[2] = 0x24;
    AckToPC[3] = 0x03;
    AckToPC[4] = 0x00;
    AckToPC[5] = 0x03;
    AckToPC[6] = font_lib_update.write_pack; //
    AckToPC[7] = font_lib_update.write_pack; //
    AckToPC[8] = 0x0d;

    write_uart1(AckToPC + 1, 8);
}

/************************************************
* �� �� ��: WriteData_To_SpiFlash
* ��������: �����ڽ��յ�������д��SPI_Flash
* ��ڲ���: ��
* �� �� ֵ: �ɹ����� 0,ʧ�ܷ��� 1
*************************************************/
static u8 update_font_write_spiflash(void) {
    u32 addr = 0;
    u16 DataLen = 0;

    addr = (font_lib_update.write_pack - 1) * PAGE_SIZE; //ÿ��д��2Kbyte
    if (font_lib_update.write_pack % 2 != 0) {
        spi_flash_erase(font_gb2312_start_addr + addr);
    }
    DataLen = update_buf[1] * 0x100 + update_buf[2]; //��ȡÿ�����ݳ���
    if (DataLen > PAGE_SIZE)                         //������ݳ��ȳ���
        return 1;
    spi_flash_write(&update_buf[3], (addr + (u32)font_gb2312_start_addr), DataLen);
    return 0;
}

//��1���Զ������������ݰ�
static void auto_ack_font_data(void) {
    if (font_lib_update.start == false)
        return;

    if (new_r1())
        return;

    if (_pastn(font_lib_update.tmr) >= 500) {
        SendAskNumToPc();
        font_lib_update.tmr = jiffies;
    }
}

//����1 ���ֿ����أ�Э��0x24��ͷ
void com1_down_font_lib(void) {
    typedef enum {
        E_FONT_IDLE = 0,
        E_FONT_RECV,
        E_FONT_EXIT
    } E_FONT_STEP;

    //E_FONT_STEP old_step = E_FONT_EXIT;
    static E_FONT_STEP step = E_FONT_IDLE;

    //static bool new_step = false;

    bool ret = false;
    u16 j;
    u8 *pf;
    u8 r_buf[2100];
    u16 r_len = 0;

    static u8 from_id = 0;
    static UART_MANAGE_STRUCT present;

    switch (step) {
    case E_FONT_IDLE:
        from_id = get_uart_news(&present);
        ret = get_uart_is_tr9(from_id);
        if (u1_m.cur.b.tr9 == true) {
            //logd("u1_m.cur.b.tr9 == true");
            auto_ack_font_data();
        } else {
            //logd("u1_m.cur.b.tr9 == 0");
        }
        if (ret) {
            //logd("step = E_FONT_RECV");
            step = E_FONT_RECV;
        } else {
            //logd("E_FONT_IDLE");
        }
        break;
    case E_FONT_RECV:
        _memset(r_buf, 0x00, 2100);
        r_len = copy_uart_news(from_id, r_buf, present.cur_p, present.c_len);
        if (r_len < 5 || r_len > 2100) {
            step = E_FONT_EXIT;
            break;
        }
        pf = r_buf + 2;
        switch (r_buf[2]) //���Э���
        {
        case 0x01: //��λ�����Ҫ������
            sp.FilePageNum = pf[3];
            sp.ProgramDataLen = pf[4] * 0x01000000 + pf[5] * 0x010000 + pf[6] * 0x0100 + pf[7];
            sp.ProgramAddCheckValue = pf[8] * 0x01000000 + pf[9] * 0x010000 + pf[10] * 0x0100 + pf[11];
            sp.ProgramXorValue = pf[12];
            font_lib_update.write_pack = 1; //PageNumber=1; //��һ������

            if ((sp.ProgramDataLen < 0x4000) || (sp.ProgramDataLen > 0x35000))
                SendAckToPc();
            else
                break;
            font_lib_update.tmr = jiffies;
            font_lib_update.start = true;

            if (sp.ProgramDataLen < 0x4000) //ֱ��дBOOT
                font_lib_update.boot = true;
            else
                font_lib_update.boot = false;
            break;
        case 0x04:                                                //PC�·���ҳ��������
            font_lib_update.PackLen = pf[1] * 0x0100 + pf[2] - 3; // //�������
            update_buf[0] = pf[3];
            update_buf[1] = font_lib_update.PackLen / 0x0100;
            update_buf[2] = font_lib_update.PackLen % 0x0100;
            for (j = 0; j < font_lib_update.PackLen; j++) {
                update_buf[j + 3] = pf[j + 4];
            }
            //д������
            update_font_write_spiflash();

            font_lib_update.tmr = jiffies;
            if (sp.FilePageNum > font_lib_update.write_pack) //������ݰ�û��ȫ��������ɣ�������λ������ָ����PageNumber������
            {
                font_lib_update.write_pack++;
            } else //���ݽ������,��ʼУ��
            {
                _memset((u8 *)&font_lib_update, 0, sizeof(font_lib_update_struct));
            }
            break;
        case 0x55: //24 03 55 09 ff
            //s_Hi3520_Monitor.bS = true;
            //s_Hi3520_Monitor.tmr = tick;
            logd("pc ask reset NVP6158c!!!");
            break;
        default:
            break;
        }

        step = E_FONT_EXIT;
        break;
    case E_FONT_EXIT:
        set_uart_ack(from_id);
        step = E_FONT_IDLE;
        break;
    default:
        step = E_FONT_IDLE;
        break;
    }

    //new_step = (old_step != step) ? true : false;
}
#endif
