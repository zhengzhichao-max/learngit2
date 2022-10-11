/**
  ******************************************************************************
  * @file    report.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V1.0.0
  * @date    2012-07-03
  * @brief   ??????????????
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

REPORT_CONFIG_STRUCT report_cfg;       //report_cfg ��??????????????????
ALARM_CONFIG_STRUCT alarm_cfg;         //????????????????
TMP_TRACE_CONFIG_STRUCT tmp_trace_cfg; //??????????????
PUBLIC_IO_UNION pub_io;                //????????IO????????��????????????????????????0x2A
//u8  acc_simulate=0;                     //ACC =0; =1:ACC ǿ�ƹ�  =2 ǿ�ƿ�
u32 bak_time_tick = 0; //??????????????????????��????????????????
DVRTimeSet s_dvr_time;

u16 Delay_detection = 0; //��ʱ����ѹ

/****************************************************************************
* ????	????: send_base_report ()
* ????	????????????????????��??????????
* ????????????????????                         
* ??????????????????????
****************************************************************************/
void send_base_report(void) {
    u16 len = 0;
    u8 sbuf[120] = {0x00};
    len = 0;

    bak_time_tick = tick;

    if (!u1_m.cur.b.tt) {
        logd("��???0200,%d", Rk_Task_Manage.RK_send_0200_flag);
    }

    if (!Rk_Task_Manage.RK_send_0200_flag) {
        if (upgrade_m.enable == true) {
            return;
        }
    }

    heart_beat_tick = tick;
    _memset(sbuf, 0, 100);
    len = pack_gps_base_info(&sbuf[0]);
    pack_any_data(CMD_UP_AUTO_REPORT, sbuf, len, PK_HEX, LINK_IP0 | LINK_IP1 | LINK_IP2);
    Rk_Task_Manage.RK_send_0200_flag = false;
}
/****************************************************************************
* ????    ????: time_rept ()
* ????    ????????????????????
* ????????????????????                         
* ??????????????????????
****************************************************************************/

static void time_rept(void) {
    //    u32 ftime = 60;

    static u32 tmmk = 0;

    if (0 == u1_m.cur.b.tt) {
        if (_covern(tmmk) > 10) {
            tmmk = tick;
            //    logd("123 tick=%d, heart_beat_tick = %d  123", tick, heart_beat_tick );
        }
        if (_covern(heart_beat_tick) >= report_cfg.heartbeat) //30�뷢һ����
        {
            heart_beat_tick = tick;
            //   logd("0200 tick=%d",tick);
            pack_any_data(CMD_UP_HEART_BEAT, (u8 *)&tmmk, 0, PK_HEX, LINK_IP0 | LINK_IP1 | LINK_IP2);
        }

        if (pub_io.b.dvr_open && (s_Hi3520_Monitor.start)) //���TR9δ������������ִ��
        {
            return;
        }
    }

    unsigned int tic_diff = _covern(bak_time_tick);

    if (tic_diff >= report_cfg.sleep_time) { //�ɺ�������
        bak_time_tick = tick;                //����λ���ϱ�:
        if (login_net_123() == true) {
            mix.placeIostate = 2;
            Rk_Task_Manage.RK_send_0200_flag = false;
            send_base_report();
            logi("send_base_report"); //����λ���ϱ�:
            bak_time_tick = tick;
        } else {
            loge("all.un.login"); //����λ���ϱ�:
        }
    }
#if (1)
    else {
        if (((tic_diff % 10) == 0) && tic_diff != 0) {
            static unsigned int tic_diff_tmp = 0;
            if (tic_diff_tmp != tic_diff) {
                tic_diff_tmp = tic_diff;
                logi("tic_diff %d, cfg sleep t rpt", tic_diff); //����λ���ϱ�:
            }
        }
    }
#endif
}

#if (0)
/****************************************************************************
* ????????:    pwr_cut_rept ()
* ??????????????�d????
* ????????????????????                         
* ??????????????????????
****************************************************************************/

/************************************************************
* �������ƣ�pwr_cut_rept
* �������ܣ�������
* ����������

*************************************************************/
static void pwr_cut_rept(void) {
    static u16 db_tick1 = 0;

    if (mix.pwr_type == PWR_UNKNOWN) {
        car_alarm.bit.cut_volt = false;
        db_tick1 = tick;
        return;
    }

    if (car_alarm.bit.cut_volt) {
        if (mix.pwr_per > 50) {
            if (_coveri(db_tick1) > 3) {
                db_tick1 = tick;
                car_alarm.bit.cut_volt = false;
                mix.power = false;
            }
        } else
            db_tick1 = tick;
    } else if (/*mix.pwr_per < 43*/ mix.pwr_vol < 860) {
        if (_coveri(db_tick1) > 3) {
            db_tick1 = tick;

            DIS_TURN_OFF;
#if 0	
			car_alarm.bit.cut_volt = true;		
			if( alarm_cfg.bypass_sw.bit.cut_volt == false)
			{
				mix.power = true;
				send_base_report();

			}
#endif
        }
    } else
        db_tick1 = tick;
}

/****************************************************************************
* ????????:    pwr_low_rept ()
* ??????????????�d????
* ????????????????????                         
* ??????????????????????
****************************************************************************/

static void pwr_low_rept(void) {
    static u16 db_tick = 0;

    if (mix.pwr_per == 0 || mix.pwr_type == PWR_UNKNOWN || alarm_cfg.bypass_sw.bit.low_volt) {
        car_alarm.bit.low_volt = false;
        db_tick = tick;
        return;
    }

    if (car_alarm.bit.low_volt) {
        if (mix.pwr_per > 60) {
            if (_coveri(db_tick) > 3) {
                db_tick = tick;
                car_alarm.bit.low_volt = false;
            }
        } else
            db_tick = tick;
    } else if (mix.pwr_per < 50) {
        if (_coveri(db_tick) > 3) {
            db_tick = tick;
            //car_alarm.bit.low_volt = true;
        }
    } else
        db_tick = tick;
}
#endif

/****************************************************************************
�̵�������
****************************************************************************/
void init_relay_gpio(void) {
    relay_output_init();
    relay_disable();
    pub_io.reg = 0;
}

/****************************************************************************
* ????????:    check_acc ()
*�޸ļ�¼��ACC���
****************************************************************************/
static void check_acc(void) {
    static unsigned char cnt4 = 0;
    if (car_state.bit.acc == false) {
        if (PIN_ACC == LOW) //_HIGH
        {
            if (++cnt4 >= 30) {
                car_state.bit.acc = true;
                //send_base_report();
            } else {
            }
        } else {
            cnt4 = 0;
        }
    } else {
        if (PIN_ACC == HIGH) {
            if (++cnt4 >= 50) {
                car_state.bit.acc = false; //false
                                           //send_base_report();
            }
        } else {
            cnt4 = 0;
        }
    }
    if (cnt4 != 0) {
        logi("acc.chk.%d.%d", cnt4, (int)PIN_ACC);
    }
}

#if 1

/*
*������check_input_H5
*���ܣ������ƹ���
*����: PC15
*��ע
**/
static void check_input_H5(void) {
    static unsigned char cnt3 = 0;
    //if (bd_rcd_io.b.res05 == false)
    if (tr9_car_status.bit.res1 == false) {
        if (PIN_I5_FOGLIGHT == _HIGH) {
            if (++cnt3 >= 10) {
                bd_rcd_io.b.res05 = true;
                tr9_car_status.bit.res1 = true;
            }
        } else
            cnt3 = 0;
    } else {
        if (PIN_I5_FOGLIGHT == _LOW) {
            if (++cnt3 >= 10) {
                //tr9_car_status.bit.high6 = false;
                //tr9_car_status.bit.res1 = false;
                bd_rcd_io.b.res05 = false;
                tr9_car_status.bit.res1 = false;
                //logd("��ƹر� \n");
            }
        } else
            cnt3 = 0;
    }
}

static void check_input_H6(void) {
    static unsigned char cnt2 = 0;

    //if (tr9_car_status.bit.back == false)
    if (tr9_car_status.bit.res2 == false) {
        if (PIN_H6_BACK == _HIGH) {
            if (++cnt2 >= 10) {
                //tr9_car_status.bit.high6 = true;
                //tr9_car_status.bit.back = true;
                rcd_io.b.backing = true;
                tr9_car_status.bit.res2 = true;
                //logd("������ \n");
            }
        } else
            cnt2 = 0;
    } else {
        if (PIN_H6_BACK == _LOW) {
            if (++cnt2 >= 10) {
                //tr9_car_status.bit.high6 = false;
                //tr9_car_status.bit.back = false;
                rcd_io.b.backing = false;
                tr9_car_status.bit.res2 = false;
                //logd("������� \n");
            }
        } else
            cnt2 = 0;
    }
}
#endif

/****************************************************************************
* ????????:    check_input_a ()
3�����Ҹǹ��źţ���1�������ߵ�ƽ����ʱ�䳬��10���϶���������������10�룬ϵͳ��ʾ���ӹء�ʱ��Ҫ���Ը��ĵģ�
4�����Ҹǿ��źţ���2�������ߵ�ƽ����ʱ�䳬��2���϶���������������2�룬ϵͳ��ʾ���ӿ���ʱ��Ҫ���Ը��ĵģ�
****************************************************************************/
static void check_input_h2(void) {
    static u32 cnt1 = 0;

#if 1

    //if (pub_io.b.high2 == false)
    if (tr9_car_status.bit.res3 == false) {
        //flags = PIN_I7_HIGH; //��ȫ��
        if (PIN_I7_HIGH == _HIGH) //��ȫ��
        {
            if (++cnt1 >= 10) {
                pub_io.b.high2 = true;
                tr9_car_status.bit.res3 = true;
                cnt1 = jiffies;
                //logd("��ȫ�� \n");
            }
        } else
            cnt1 = 0;
    } else {
        if (PIN_I7_HIGH == _LOW) {
            if (++cnt1 >= 10) {
                pub_io.b.high2 = false;
                tr9_car_status.bit.res3 = false;
                cnt1 = jiffies;
                //logd("��ȫ���ɿ�\n");
            }
        }

        else
            cnt1 = 0;
    }
#endif
}

static void check_input_h1(void) {
    static unsigned char cnt = 0;
    //static u8 stat = 0;

    //static = PIN_I8_HIGH ;

    //if (pub_io.b.high1 == false)
    if (tr9_car_status.bit.res4 == false) //���񣺳���
    {
        if (PIN_I8_HIGH == _HIGH) {
            if (++cnt >= 10) {
                pub_io.b.high1 = true;
                tr9_car_status.bit.res4 = true;
            }
        } else
            cnt = 0;
    } else {
        if (PIN_I8_HIGH == _LOW) {
            if (++cnt >= 10) {
                pub_io.b.high1 = false;
                tr9_car_status.bit.res4 = false;
            }
        } else
            cnt = 0;
    }
}

//Ӧ������
void check_sos_report(void) {
    //static unsigned char cnt = 0;
    static bool press = false;

    static unsigned char cnt_5 = 0;
    if (car_alarm.bit.cut_volt || alarm_cfg.bypass_sw.bit.sos) {
        cnt_5 = 0;
        press = false;
        car_alarm.bit.sos = false;
        return;
    }

    if (tick >= 15) {
        if (press == false) {
            //if (PIN_INPUT_SOS== HIGH)
            if (PIN_INPUT_SOS == LOW) {
                if (++cnt_5 >= 15) {
                    press = true;
                    car_alarm.bit.sos = true;
                    if (car_alarm.bit.sos) {
                        //send_base_report();
                    }
                    //logd("sos �� press = %d",press);
                }
            } else
                cnt_5 = 0;
        } else {
            if (car_alarm.bit.sos == false) {
                if (PIN_INPUT_SOS == LOW)
                    press = false;
            }
        }
    }
}

/****************************************************************************
* ????????:    check_input_l2 ()
//��ȫ����
��ȫ�����źţ���2������⵽�ӵؾ�����ȫ��������⵽�Ͽ�����ƽ�ţ�					
****************************************************************************/
static void check_input_l2(void) {
    static u32 cnt = 0;

    if (pub_io.b.left_cover == false) {
        if (PIN_COVER_LOW == LOW) {
            if (_pastn(cnt) >= 500) {
                pub_io.b.left_cover = true;
                pub_io.b.right_cover = true;
                tr9_car_status.bit.covering = true;
                cnt = jiffies;
            }
        } else
            cnt = jiffies;
    } else {
        if (PIN_COVER_LOW == HIGH) {
            if (_pastn(cnt) >= 50) {
                //all_open_monitor.all_flag = false;
                pub_io.b.left_cover = false;
                pub_io.b.right_cover = false;
                tr9_car_status.bit.covering = false;
                cnt = jiffies;
            }
        } else
            cnt = jiffies;
    }
}

/****************************************************************************
* ????????:    check_input_l1 ()
��������     0 �ر�  
�����źţ���1������⵽�нӵؾ���ƽ�ţ���⵽�Ͽ��;����������Ͽ�3�����ʾ������	
****************************************************************************/
static void check_input_l1(void) {
    static u32 cnt = 0;

    if (pub_io.b.car_box == false) {
        if (PIN_LIFT_LOW == LOW) {
            if (_pastn(cnt) >= 500) {
                pub_io.b.car_box = true;
                tr9_car_status.bit.rising = true;
                cnt = jiffies;
            }

        } else
            cnt = jiffies;
    } else {
        if (PIN_LIFT_LOW == HIGH) //�պ�
        {
            if (_pastn(cnt) >= 50) {
                tr9_car_status.bit.rising = false;

                pub_io.b.car_box = false;
                cnt = jiffies;
            }
        } else
            cnt = jiffies;
    }
}

/****************************************************************************
* ????????:    check_lock_status ()
�����״̬:	
*
****************************************************************************/
static void check_lock_status(void) {
    static unsigned char cnt = 0;
    if (pub_io.b.key_s == false) //����
    {
        if (PIN_LOCK_CHECK == HIGH) {
            if (++cnt >= 30) //HIGH
            {
                pub_io.b.key_s = true;
            }
        } else
            cnt = 0;
    } else {
        if (PIN_LOCK_CHECK == LOW) {
            if (++cnt >= 30) {
                pub_io.b.key_s = false;
            }
        } else
            cnt = 0;
    }
}

void stop_rept(void) {
    static u32 dbnc = 0;
    //    u32 time = 0;

    if (mix.moving) {
        if ((mix.complex_speed < 1) || (car_state.bit.acc == false)) {
            //if (_covern(dbnc) >= 3)
            if (_covern(dbnc) >= 10) {
                mix.moving = false;
                dbnc = tick;
            }
        } else
            dbnc = tick;
    } else {
        if (((mix.complex_speed >= 1) && (car_state.bit.acc == true))) //
        {
            if (_covern(dbnc) >= 10) {
                mix.moving = true;
                dbnc = tick;
            }
        } else
            dbnc = tick;
    }
}

/****************************************************************************
 * ����:	  power_Down_Electrical_Testing ()
 * ���ܣ������⹦��
 * ��ڲ�������
 * ���ڲ�������
 ****************************************************************************/

void power_Down_Electrical_Testing(void) {
    bool close_Flag;
    static u8 colse_pwr = 0;
    //    static u8 close_low_volt = 0;

    if (tick > 10) //����10���ʼ����ѹ
    {
#if 0
		if (mix.pwr_type == 1)					//6~12V
		{
			if(mix.pwr_vol < 840)					//��ѹֵ
			{
				car_alarm.bit.cut_volt = true;
				Delay_detection = tick ;
				DIS_CAM12V_PWR;					//��Ƭ����Դ//����ԴPD12	
			}
			if(car_alarm.bit.cut_volt == false)		//????
			{
				if( mix.pwr_vol < 860)  			//����
				{	
					close_Flag = true ;
					colse_pwr = 0;
					car_alarm.bit.cut_volt = true;
					Delay_detection = tick ;
					return ;
				}
				/*
				else if((mix.pwr_vol >= 1580 )&& (mix.pwr_vol < 1760)) // 16~18��
				{
					close_Flag = true ;
					DIS_CAM12V_PWR;
					//car_alarm.bit.low_volt = true;	//����
				}
				*/
			}
			else 	//?????????			
			{
				if((mix.pwr_vol > 860) && (mix.pwr_vol < 1580))			//�ָ�����״̬
				{	
					colse_pwr = 0;
					close_Flag = false ;	
					car_alarm.bit.cut_volt = false;
				}
				else if((mix.pwr_vol > 1760))
				{
					colse_pwr = 0;
					close_Flag = false ;
					car_alarm.bit.cut_volt = false;	
				}
			}		
		}
		else
		{
			if((mix.pwr_vol > 1760))
			{
				colse_pwr = 0;
				close_Flag = false ;
				car_alarm.bit.cut_volt = false;
			}
		}
#else

        if (mix.pwr_vol < 840) //��ѹֵ
        {
            car_alarm.bit.cut_volt = true;
            Delay_detection = tick;
            DIS_CAM12V_PWR; //��Ƭ����Դ//����ԴPD12
        }
        if (car_alarm.bit.cut_volt == false) //????
        {
            if (mix.pwr_vol < 860) //����
            {
                close_Flag = false;
                colse_pwr = 0;
                car_alarm.bit.cut_volt = true;
                //Delay_detection = tick ;
                return;
            } else if ((mix.pwr_vol >= 1700) && (mix.pwr_vol < 2040)) {
                close_Flag = true;
            }
        } else //??????
        {
            if ((mix.pwr_vol > 860) && (mix.pwr_vol < 1500)) //�ָ�����״̬
            {
                colse_pwr = 0;
                close_Flag = false;
                car_alarm.bit.cut_volt = false;
            } else if (mix.pwr_vol > 2040) {
                colse_pwr = 0;
                close_Flag = false;
                car_alarm.bit.cut_volt = false;
            }
        }

#endif

        /*************************************************/
        if (close_Flag == true) //���籣��
        {
            if (colse_pwr >= 3) {
                if (car_alarm.bit.cut_volt == false) {
                    //Delay_detection = tick ;
                    car_alarm.bit.cut_volt = true;
                    logd("????");
                    DIS_CAM12V_PWR;
                    //Close_Extern_Pwr();
                }
                colse_pwr = 0;
            } else {
                colse_pwr++;
            }
        } else {
            /*
			if( )
			{
				if( close_low_volt >= 3)
				{
					close_low_volt = 0;
					car_alarm.bit.cut_volt = false ;
				}
				else
					close_low_volt++;
			}
			*/
        }
    }
}

/****************************************************************************
* ����:	  Low_electrical_testing ()
* ���ܣ�Ƿѹ��⹦��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void Low_electrical_testing(void) {
    bool pwr_low_Flag = false;
    bool normal_vol_Flag = false;

    static u8 pwr_low_true = 0;
    static u8 pwr_low_false = 0;

    if (_coveri(Delay_detection) > 13) //��ʱ����ѹ
    {
#if 0
		if (mix.pwr_type == 1)					//6~12V
		{		
			if(mix.pwr_vol < 680 )				//���粻�ñ�Ƿѹ
			{
				pwr_low_true  = 0 ;
				pwr_low_false = 0 ;
				car_alarm.bit.low_volt = false ;
				return ;
			}
			if(car_alarm.bit.low_volt == false)
			{	
				if( mix.pwr_vol < 1020)  	//10.5 	
				{	
					pwr_low_Flag = true ;
					normal_vol_Flag = false ;
				}
				else if((mix.pwr_vol > 1580)&& (mix.pwr_vol < 2005))	//20.5Ƿѹ
				{
					pwr_low_Flag =  true ;	
					normal_vol_Flag = false ;
				}
			}
			else 
			{
				if((mix.pwr_vol > 1020) && ( mix.pwr_vol < 1580) )		
				{	
					pwr_low_Flag =  false ;			//Ƿѹ
					normal_vol_Flag = true ;		//������ѹ
				}		
			}		
		}
		else if(mix.pwr_type == 2)		
		{
			if(car_alarm.bit.low_volt == false)
			{
				if(mix.pwr_vol < 2005 )  			// 20.5��
				{
					pwr_low_Flag = true ;			//Ƿѹ
					normal_vol_Flag = false ;		//������ѹ
				}
					
			}
			else 
			{
				if(mix.pwr_vol > 2005)
				{
					pwr_low_Flag = false ;			//Ƿѹ
					normal_vol_Flag = true ;  		//������ѹ
				}	
			}
				
		}
#else

        if (mix.pwr_vol < 680) //���粻�ñ�Ƿѹ
        {
            pwr_low_true = 0;
            pwr_low_false = 0;
            car_alarm.bit.low_volt = false;
            return;
        }
        if (car_alarm.bit.low_volt == false) {
            if (mix.pwr_vol < 1020) //10.5
            {
                pwr_low_Flag = true;
                normal_vol_Flag = false;
            } else if ((mix.pwr_vol > 1700) && (mix.pwr_vol <= 2095)) //21.6Ƿѹ
            {
                pwr_low_Flag = true;
                normal_vol_Flag = false;
            }
        } else {
            if ((mix.pwr_vol > 1020) && (mix.pwr_vol < 1580)) {
                pwr_low_Flag = false;   //Ƿѹ
                normal_vol_Flag = true; //������ѹ
            } else if (mix.pwr_vol > 2095) {
                pwr_low_Flag = false;   //Ƿѹ
                normal_vol_Flag = true; //������ѹ
            }
        }

#endif
        /*********************************************************************************/

        if (pwr_low_Flag == true) //Ƿѹ
        {
            if (pwr_low_true >= 15) {
                pwr_low_true = 0;
                car_alarm.bit.low_volt = true; //Ƿѹ
            } else {
                pwr_low_true++;
                pwr_low_false = 0;
            }
        } else if (normal_vol_Flag == true) {
            if (pwr_low_false > 2) {
                pwr_low_false = 0;
                car_alarm.bit.low_volt = false; //����
            } else {
                pwr_low_true = 0;
                pwr_low_false++;
            }
        }
    }
}

//����Ҫ����
//�Ҳ�ǰ��ܱ�  �ҳ��ٴ���5����ʾ�س���
//�س���ɿճ��Ǵ�����ȫ�������ɿճ�
void weight_time_mibi(void) {
}

//
void CX_False_Task(void) {
}

static funcp const ReptTaskTab[] = {
    time_rept, //??????????????
    NULL};

//static void tired_rept(void)
void tired_rept(void) {
    static u32 dbnc1 = 0;        //��ʱ����1
    static u32 dbnc2 = 0;        //��Ϣʱ���ʱ��
    static u32 dbnc3 = 0;        //
    static bool tired = false;   //ƣ�ͼ�ʻ��־
    static bool rest = true;     //��Ϣ��־
    static bool running = false; //���б�־

    static bool fat_run = false; //
    static u16 fat_tmr = 0;      //

    u32 s_time = 0;  //ͣ����Ϣʱ��
    u32 f_time1 = 0; //ƣ�ͼ�ʻ�涨Ԥ������ʱ�� = 12600 ��ǰ30����
    u32 f_time2 = 0; //ƣ�ͼ�ʻ�涨����ʱ�� = 14400 4Сʱʱ��

#if (P_RCD == RCD_BASE) //��ʻ��¼��
    static u32 rcd_start_tick;
    static bool rcd_update = false;

    if ((tired == false && rcd_start_tick != dbnc2) || (rcd_update == true)) {
        rcd_start_tick = dbnc2;
        rcd_update = false;
        _memcpy((u8 *)&rcd_ot_inf.start, (u8 *)&sys_time, 6);
        rcd_ot_inf.s_point.lati = _jtb_to_gb(mix.locate.lati);
        rcd_ot_inf.s_point.lngi = _jtb_to_gb(mix.locate.lngi);
        rcd_ot_inf.s_high = mix.heigh;
    }
#endif

    //��� ��ʻʱ��δ���� �� ͣ����Ϣʱ�� δ�����򷵻�
    if (alarm_cfg.fatigure_time < 10 || alarm_cfg.stop_rest_time == 0) { //���δ����ƣ�ͼ�ʻʱ��,�򷵻ء�
        return;
    }

    if (alarm_cfg.bypass_sw.bit.fatigue) {
        tired = false;
        dbnc1 = tick;
        dbnc2 = tick;
        dbnc3 = tick;
        return;
    }

    if (mix.moving) {
        if (!fat_run) {
            if ((_coveri(fat_tmr) > 30)) {
                fat_tmr = tick;
                fat_run = true;
            } else {
            }
        } else {
            fat_tmr = tick;
        }
    } else {
        if (fat_run) {
            if ((_coveri(fat_tmr) > 3)) {
                fat_tmr = tick;
                fat_run = false;
            } else {
            }
        } else {
            fat_tmr = tick;
        }
    }

    /*�жϼ�ʻ�������Ƿ���й���ʻԱIC����*/
    if (mix.ic_exchange) { //�Ƿ������ʻԱ, ���������,���¼���ƣ�ͼ�ʻʱ�䡣
        mix.ic_exchange = false;

        car_alarm.bit.fatigue = false;
        car_alarm.bit.fatigue_near = false;
        dbnc1 = tick;
        dbnc2 = tick;
        dbnc3 = tick;
        if (tired)
            rest = true;
        tired = false;
        return;
    }

    //logd("4...... i am here ......mix.moving=%x..",mix.moving);

    if (!alarm_cfg.fatigure_time || !alarm_cfg.stop_rest_time) {
        car_alarm.bit.fatigue = false;
        tired = false;
        dbnc1 = tick;
        dbnc2 = tick;
        dbnc3 = tick;
        return;
    }

#if 0
	if( Night_Speed_Task() == 1 )  //???2��???????
	{
		f_time2 = 7200;
		f_time1 = 6300;	
	}
	else

	{
		f_time2 = (alarm_cfg.fatigure_time< 300||alarm_cfg.fatigure_time>86400) ? 14400: alarm_cfg.fatigure_time;
		f_time1 = (alarm_cfg.fatigure_time>1800)?(f_time2 - 1800):(1800);
	}
	s_time =  (alarm_cfg.stop_rest_time< 180||alarm_cfg.stop_rest_time>86400) ? 1200: alarm_cfg.stop_rest_time;
	
	if( f_time2 < 7200 )
		f_time2 = 7200;
#endif

    f_time2 = alarm_cfg.fatigure_time;
    f_time1 = f_time2 - 120;
    s_time = 10;

    if (tired) {
        dbnc2 = tick;
        dbnc3 = tick;

        if (!fat_run) {
            if (_covern(dbnc1) >= s_time) {
                dbnc1 = tick;
                tired = false;
                car_alarm.bit.fatigue = false;
                running = false;
                car_alarm.bit.fatigue_near = false;
                //send_base_report();

#if (P_RCD == RCD_BASE)
                rcd_update = true;
                _memset((u8 *)&rcd_ot_inf.drive, 0, 18);
#endif
            }
        } else { //
            dbnc1 = tick;
            if (rest) {
                tired = false;
            }
        }
    } else {
        dbnc1 = tick;
        if (fat_run) {
            rest = false;
            dbnc3 = tick;
            if (running == false) {
                dbnc2 = tick;
                running = true;
            } else if (_covern(dbnc2) >= f_time2) {
                tired = true;
                dbnc1 = tick;
                car_alarm.bit.fatigue = true;
                car_alarm.bit.fatigue_near = false;

#if (P_SOUND == SOUND_BASE)
                tts_c.open = false;
#endif
                send_base_report();

#if 0
				if(alarm_cfg.sms_sw.bit.fatigue)
					send_sms_alarm(A_FATIGUE);
#endif

#if (P_CMA == CMA_BASE)
                if (alarm_cfg.pic_sw.bit.fatigue)
                    alarm_take_photo(ALARM_PHOTO_EVENT, T_PIC_TRIGER, alarm_cfg.pic_save_sw.bit.fatigue);
#endif

#if (P_RCD == RCD_BASE)
                _memcpy((u8 *)&rcd_ot_inf.drive, (u8 *)&mix.ic_driver, 18);
                _memcpy((u8 *)&rcd_ot_inf.end, (u8 *)&sys_time, 6);

                rcd_ot_inf.e_point.lati = _jtb_to_gb(mix.locate.lati);
                rcd_ot_inf.e_point.lngi = _jtb_to_gb(mix.locate.lngi);
                rcd_ot_inf.e_high = gps_base.heigh;
                rcd_ot_inf.new_en = true;
#endif

            } else if (_covern(dbnc2) >= f_time1) {
                if ((car_alarm.bit.fatigue_near == false) && (!alarm_cfg.bypass_sw.bit.fatigue_near)) {
                    car_alarm.bit.fatigue_near = true;

#if (P_CMA == CMA_BASE)
                    tts_c.open = false;
#endif
                }
            } else {
            }
        } else {
            if (_covern(dbnc3) >= s_time) {
                dbnc2 = tick; //
                running = false;
                car_alarm.bit.fatigue = false;
                car_alarm.bit.fatigue_near = false;

#if (P_RCD == RCD_BASE)
                _memset((u8 *)&rcd_ot_inf.drive, 0, 18);
#endif
            }
        }
    }
}

static u16 task_en = 0;
/****************************************************************************
* ????????:    report_create_task ()
* ????????????????????????????????
* ????????????????????                         
* ??????????????????????
****************************************************************************/
void report_create_task(void) {
    static u16 n = 0;
    static u32 check_100ms_jiff;

    check_acc();

    //if(jiffies<5000)
    //	return;

    task_en |= 0x0fff;
    if (task_en & (1 << n)) {
        (*ReptTaskTab[n])();
        task_en &= ~(1 << n);
    }

    if (ReptTaskTab[++n] == NULL)
        n = 0;

    check_sos_report();
    //if( car_state.bit.acc )
    {
        //if( cx_false_tmr.close == 0 )
        {
            check_input_h1();
            check_input_h2();
        }
        check_input_H6();
        check_input_H5();

        //if( cx_false_tmr.all == 0 )
        check_input_l2();
        //if( cx_false_tmr.lift == 0 )
        check_input_l1();
    }
    check_lock_status();

    if (_pasti(check_100ms_jiff) >= 400) {
        check_100ms_jiff = jiffies;
        //stop_rept();
        //pwr_cut_rept() ;
        //pwr_low_rept() ;
        power_Down_Electrical_Testing(); //���紦��
        Low_electrical_testing();        //Ƿѹ����
        weight_time_mibi();

        //tired_rept();

        CX_False_Task();
    }
}
