/**
  ******************************************************************************
  * @file    jt808_manage.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   jt808 Э����ܹ���
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"
LINK_KEEP_CONFIG_STRUCT link_cfg;      //link_cfg �������ýṹ��
IP_SERVER_CONFIG_STRUCT server_cfg;    //server_cfg IP���ýṹ��
DRIVER_CONFIG_STRUCT driver_cfg;       //��ʻԱ��Ϣ
SEND_MESS_INFOMATION_STRUCT send_info; //���·�����Ϣ
VEHICLE_ALARM_UNION pwr_alm_chk;       //�ϵ��Լ�
VEHICLE_ALARM_UNION bypass_pwr_chk;    //�ϵ��Լ�
CAR_CONFIG_STRUCT car_cfg;             //����ϵ��
#if (P_RCD == RCD_BASE)
FACTORY_PARAMETER_STRUCT factory_para; //��ʻ��¼�ǲ���
SIGNAL_CONFIG_STRUCT sig_cfg;          //�˿�����
UNIQUE_ID_STRUCT unique_id;            //Ψһ�Ա���
U_BD_RCD_STATUS bd_rcd_io;             //������ʽ��IO����
#endif

GNSS_INF_STRUCT gnss_cfg; //GNSS ��Ϣ����
CAN_FANC_INF_STRUCT can_cfg;
authority_configuration_struct authority_cfg;
bool quick_into_enable = true;  //����ע����Ȩ�����õȴ�ʱ��
bool quick2_into_enable = true; //����ע����Ȩ�����õȴ�ʱ��

bool imei_flag;

/*********************************************************
��	  �ƣ�Terminal_Login
��	  �ܣ��ն�ע��
����������ն�ע����Ϣ��
���������
��	  �أ�
ע�����0x0100 �����ߺ��������м�Ȩ���������÷�����������
		  ����м�Ȩ�룬�����ն˼�Ȩ��Ϣ 
		  ��������ͼ�Ȩ�룬�����ϵ���Լ��������� 08-30
*********************************************************/
void terminal_login_task(void) {
    u8 buf[100] = {0x00};
    u8 *pa = NULL;
    u8 code_len = 0;
    //    static u8 con = 0;

    static u8 uGetCnt = 0;
    static u8 uGetCnt2 = 0;
    static u8 uGetCnt3 = 0;
    static u32 relogin_jiff = 0;
    /*
	if(lm.sms_rst_en||lm.reset)
		return;
*/

#if defined(JTT_808_2019)
    imei_flag = imei_flag;
#else
    imei_flag = true;
#endif

    if (!gs.gprs1.login) {
        if (gs.gprs1.stat <= NET_LOST) {
            goto GPRS2_LOGIN;
        }

        if (gs.gprs1.quick_first || _pastn(relogin_jiff) > 20000) { //ע��������
            relogin_jiff = jiffies;
            gs.gprs1.quick_first = false;

            if (gs.gprs1.server_type == SERVER_UPDATE) {
                gs.gprs1.login = true;
            }

            if (gs.gprs1.login)
                goto GPRS2_LOGIN;

            if (imei_flag) {
                if (authority_cfg.type == 0xcc) {
                    //��ע��ɹ������ͼ�Ȩ��
                    _memset(buf, 0x00, sizeof(buf));
                    pa = buf;
#if defined(JTT_808_2019)

                    *pa = _strlen(authority_cfg.code);
                    code_len = 1;
                    code_len += _memcpy_len((pa + code_len), authority_cfg.code, *pa);
                    code_len += _memcpy_len(pa + code_len, mx.imei, 15);
                    code_len += _memcpy_len(pa + code_len, versions, 20);

                    pack_any_data(CMD_UP_RELOGIN, buf, code_len, PK_HEX, LINK_IP0);

#else //jt808_2011

                    //code_len = (authority_cfg.code[20]>=20) ? 0 : authority_cfg.code[20];
                    code_len = _strlen(authority_cfg.code);
                    _memcpy(buf, (u8 *)&authority_cfg.code, code_len);
                    pack_any_data(CMD_UP_RELOGIN, buf, code_len, PK_HEX, LINK_IP0);

#endif
#if 0
					if(++uGetCnt >= 10)
					{
						logd("��μ�Ȩʧ��,ע��");
						pack_any_data(0x0003, buf, 0, PK_HEX, LINK_IP0 ); //���ע��
						authority_cfg.type = 0x00;	
						
						uGetCnt = 0;
						lm.sms_rst_en= true;
						lm.sms_rst_tick = tick; 	
					}
#else
                    if (++uGetCnt >= 4) {
                        self_test.uGsmU = '4';
                    }
#endif
                } else { //δע��ɹ�
                    if (0 == (u1_m.cur.b.tt) || 1) {
                        loge("logo1 ext");
                        loge("\t��Ȩ��<%s>IMEI��<%s>�汾��<%s>", authority_cfg.code, mx.imei, versions);
                        self_test.uGsmU = '3';
                        //if()
                        //	goto GPRS2_LOGIN;
                    }

                    //if(++uGetCnt >= 3)
                    //{
                    //pack_register_info(LINK_IP0);
                    //authority_cfg.type = 0xcc;
                    //uGetCnt = 0;
                    //}
                }
            }
        }
    }

GPRS2_LOGIN:
    if (!gs.gprs2.login) {
        if (gs.gprs2.stat <= NET_LOST)
            goto GPRS3_LOGIN;

        if (gs.gprs2.quick_first || _pastn(relogin_jiff) > 20000) //ע��������
        {
            relogin_jiff = jiffies;
            gs.gprs2.quick_first = false;

            if (gs.gprs2.login)
                goto GPRS3_LOGIN;

            if (imei_flag) {
                if (authority_cfg.type2 == 0xcc) {
                    //��ע��ɹ������ͼ�Ȩ��
                    logd("logo2 into");
                    _memset(buf, 0x00, sizeof(buf));
                    pa = buf;
#if defined(JTT_808_2019)

                    *pa = _strlen(authority_cfg.code2);
                    code_len = 1;
                    code_len += _memcpy_len((pa + code_len), authority_cfg.code2, *pa);
                    code_len += _memcpy_len(pa + code_len, mx.imei, 15);
                    code_len += _memcpy_len(pa + code_len, versions, 20);

                    pack_any_data(CMD_UP_RELOGIN, buf, code_len, PK_HEX, LINK_IP1);

#else

                    //code_len = (authority_cfg.code2[20]>=20) ? 0 : authority_cfg.code2[20];
                    code_len = _strlen(authority_cfg.code2);
                    _memcpy(pa, (u8 *)&authority_cfg.code2, code_len);
                    pack_any_data(CMD_UP_RELOGIN, buf, code_len, PK_HEX, LINK_IP1);

#endif
#if 0
					if(++uGetCnt2 >= 3)
					{
						pack_any_data(0x0003, buf, 0, PK_HEX, LINK_IP1 ); //���ע��
						authority_cfg.type2 = 0x00;	
						uGetCnt2 = 0;
						lm.sms_rst_en= true;
						lm.sms_rst_tick = tick; 	
					}
#endif
                } else { //δע��ɹ�
                    loge("logo2 ext");
                    //pack_register_info(LINK_IP1);
                    if (++uGetCnt2 >= 3) {
                        //authority_cfg.type2 = 0xcc;
                        uGetCnt2 = 0;
                    }
                }
            }
        }
    }

GPRS3_LOGIN:
    if (!gs.gprs3.login) {
        if (gs.gprs3.stat <= NET_LOST)
            return;

        if (gs.gprs3.quick_first || _pastn(relogin_jiff) > 20000) //ע��������
        {
            relogin_jiff = jiffies;
            gs.gprs3.quick_first = false;

            if (gs.gprs3.login)
                return;

            logd("logo3 = 0x%02X", authority_cfg.type3);
            if (imei_flag) {
                if (authority_cfg.type3 == 0xcc) {
                    //��ע��ɹ������ͼ�Ȩ��
                    logd("logo3 into");

                    _memset(buf, 0x00, 100);
                    pa = buf;
#if defined(JTT_808_2019)

                    *pa = _strlen(authority_cfg.code3);
                    code_len = 1;
                    code_len += _memcpy_len((pa + code_len), authority_cfg.code3, *pa);
                    code_len += _memcpy_len(pa + code_len, mx.imei, 15);
                    code_len += _memcpy_len(pa + code_len, versions, 20);

                    pack_any_data(CMD_UP_RELOGIN, buf, code_len, PK_HEX, LINK_IP2);
#else

                    //code_len = (authority_cfg.code3[20]>=20) ? 0 : authority_cfg.code3[20];
                    code_len = _strlen(authority_cfg.code3);
                    _memcpy(buf, (u8 *)&authority_cfg.code3, code_len);
                    pack_any_data(CMD_UP_RELOGIN, buf, code_len, PK_HEX, LINK_IP2);

#endif

                    if (++uGetCnt3 >= 3) {
                        pack_any_data(0x0003, buf, 0, PK_HEX, LINK_IP2); //���ע��
                        authority_cfg.type3 = 0x00;
                        uGetCnt3 = 0;
                        lm.sms_rst_en = true;
                        lm.sms_rst_tick = tick;
                    }
                } else { //δע��ɹ�
                    loge("logo3 ext");
                    //pack_register_info(LINK_IP2);
                    //if(++uGetCnt3 >= 3)
                    //{
                    //	authority_cfg.type3 = 0xcc;
                    //	uGetCnt3 = 0;
                    //}
                }
            }
        }
    }
}
