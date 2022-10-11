/**
  ******************************************************************************
  * @file    jt808_manage.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   jt808 协议或功能管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"
LINK_KEEP_CONFIG_STRUCT link_cfg;      //link_cfg 链接设置结构体
IP_SERVER_CONFIG_STRUCT server_cfg;    //server_cfg IP设置结构体
DRIVER_CONFIG_STRUCT driver_cfg;       //驾驶员信息
SEND_MESS_INFOMATION_STRUCT send_info; //最新发送消息
VEHICLE_ALARM_UNION pwr_alm_chk;       //上电自检
VEHICLE_ALARM_UNION bypass_pwr_chk;    //上电自检
CAR_CONFIG_STRUCT car_cfg;             //车辆系数
#if (P_RCD == RCD_BASE)
FACTORY_PARAMETER_STRUCT factory_para; //行驶记录仪参数
SIGNAL_CONFIG_STRUCT sig_cfg;          //端口配置
UNIQUE_ID_STRUCT unique_id;            //唯一性编码
U_BD_RCD_STATUS bd_rcd_io;             //北斗格式的IO定义
#endif

GNSS_INF_STRUCT gnss_cfg; //GNSS 信息配置
CAN_FANC_INF_STRUCT can_cfg;
authority_configuration_struct authority_cfg;
bool quick_into_enable = true;  //快速注册或鉴权，不用等待时间
bool quick2_into_enable = true; //快速注册或鉴权，不用等待时间

bool imei_flag;

/*********************************************************
名	  称：Terminal_Login
功	  能：终端注册
输入参数：终端注册消息体
输出参数：
返	  回：
注意事项：0x0100 ，上线后立即进行鉴权操作，不得发送其他数据
		  如果有鉴权码，则发送终端鉴权信息 
		  如果不发送鉴权码，重新上电可以继续操作吗 08-30
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

        if (gs.gprs1.quick_first || _pastn(relogin_jiff) > 20000) { //注意心跳包
            relogin_jiff = jiffies;
            gs.gprs1.quick_first = false;

            if (gs.gprs1.server_type == SERVER_UPDATE) {
                gs.gprs1.login = true;
            }

            if (gs.gprs1.login)
                goto GPRS2_LOGIN;

            if (imei_flag) {
                if (authority_cfg.type == 0xcc) {
                    //已注册成功、发送鉴权码
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
						logd("多次鉴权失败,注销");
						pack_any_data(0x0003, buf, 0, PK_HEX, LINK_IP0 ); //添加注销
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
                } else { //未注册成功
                    if (0 == (u1_m.cur.b.tt) || 1) {
                        loge("logo1 ext");
                        loge("\t鉴权码<%s>IMEI码<%s>版本号<%s>", authority_cfg.code, mx.imei, versions);
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

        if (gs.gprs2.quick_first || _pastn(relogin_jiff) > 20000) //注意心跳包
        {
            relogin_jiff = jiffies;
            gs.gprs2.quick_first = false;

            if (gs.gprs2.login)
                goto GPRS3_LOGIN;

            if (imei_flag) {
                if (authority_cfg.type2 == 0xcc) {
                    //已注册成功、发送鉴权码
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
						pack_any_data(0x0003, buf, 0, PK_HEX, LINK_IP1 ); //添加注销
						authority_cfg.type2 = 0x00;	
						uGetCnt2 = 0;
						lm.sms_rst_en= true;
						lm.sms_rst_tick = tick; 	
					}
#endif
                } else { //未注册成功
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

        if (gs.gprs3.quick_first || _pastn(relogin_jiff) > 20000) //注意心跳包
        {
            relogin_jiff = jiffies;
            gs.gprs3.quick_first = false;

            if (gs.gprs3.login)
                return;

            logd("logo3 = 0x%02X", authority_cfg.type3);
            if (imei_flag) {
                if (authority_cfg.type3 == 0xcc) {
                    //已注册成功、发送鉴权码
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
                        pack_any_data(0x0003, buf, 0, PK_HEX, LINK_IP2); //添加注销
                        authority_cfg.type3 = 0x00;
                        uGetCnt3 = 0;
                        lm.sms_rst_en = true;
                        lm.sms_rst_tick = tick;
                    }
                } else { //未注册成功
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
