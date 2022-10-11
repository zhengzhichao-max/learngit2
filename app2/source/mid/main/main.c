
/********************************************************************************
  * @file    main.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  ������
  1.�������ʱ����ȵķ�ʽ������ÿ������
  2.ÿ��������Ҫ��������ִ��ʱ�䣬���ϵͳ��Ӧʱ�䡣
  3.���������ͨ������ֽ�ķ�ʽʵ��
  4.�������ж��д���������ݣ�����ʹ���жϲ�ѯ�ķ�ʽ��
  5.�������ֳ�����һ�£������պ�ά��
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

/*2022-03-23 �޸�����:
  1. Ϊ������ʻ��¼������,  �� #define MAX_CAN_BUF 80  ��ʱ�޸�Ϊ 10 *** ע��ָ� 03-26 �Ѿ��ָ�****
  2. �� #define UART1_RECV_BUFF_MAX_LGTH        ((uint)512) �޸�Ϊ
        #define UART1_RECV_BUFF_MAX_LGTH        ((uint)1024)
     �� #define UART1_SEND_BUFF_MAX_LGTH        ((uint)512) �޸�Ϊ
        #define UART1_SEND_BUFF_MAX_LGTH        ((uint)1024)   
        ****** ע�� �ָ� ********

���� 2022-03-25�޸�����:
     1. �� С�� ͬ�������� ѹ����: HGXH-BASE-GD305_RK3568-2022-03-24-1245-��  ��������޸ġ�
     2. �� �ṹ�� Rk_Task_Manage_t ���Ӳ��ֱ���
     3. rcd_assemble_body() ������ ���� ����ʱ������ݶ�ȡ���ж�.
     4. ���� tr9_Auto_start() �ú�����Ҫ��ΪӦ�� ����
     5. �� uart1_debug_proc() ���������� �������� ����ʻ��¼������ ��ʽ��
     6. �� gps_debonce() ���������� ��GPS �ĳ�ʼ�ٶȸ�ֵ, ����ʹ��, ��ס ����ʱ ע�͵�** �Ѿ��ָ���
     7. ���� tr9_Export_state() ������
     8. ���� Rk_Driving_Record_Up() ����
     

���� 2022-03-28�޸�����:
     ��� ��ʻ��¼�� cmd=0x08H - 0x15H �� ����  ��: Rk_Driving_Record_Up() ������,�޸�����
           rcd_send_data(FROM_U4, RCD_C_OK, rsp.id, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);  �޸�Ϊ��
           rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len); 

���� 2022-03-30�޸�����:
     1. ��С���ṩ�� source_03_30��ѹ�����������г���
     2. ����� CMD 11\12\14 �޼�¼ʱ, ��ʻ��¼�ǲ�Ӧ������˴�����ʻ��¼���޼�¼ʱ,Ҳ��ҪӦ��
        �޸ĵط�Ϊ: 08H ~ 15H ����ز���, ��ϸ�� ʱ��� 2022-03-30 ע�͡�
     
�ġ� 2022-03-31�޸�����:     
     1.  �� Rk_Driving_Record_Up() ������ �޸ģ�
         1.1 ��� 08-15H �ķ��Ͱ����� ������512���ֽ��ڣ��������� rk_6033_task.frame ��ֵ
         1.2 rk_6033_task.u8_delay �����ʱʱ�� ��Ҫ����32λ��,ԭ�����������⡣
             �� rk_6033_task_t�ṹ�����޸�
         1.3 ���� ��08 ��ȡ��ʱ�� ��һ�ο���ʱ cmd ��Ӧ��ʱ 08H �����ʱ������ 00H.  
             �����Ǵ����еı���bug  �����ط�ʱ, ����Ҫ �� rk_6033_task.cmd �滻rsp.id
         1.4 ���ط���¼������ʱ�߼����д���,���޸�
         
     2. �� RCD_COM_MAX_SIZE ����   ��800byte �޸�Ϊ1024 byte()
     3. �޸� T1_BUF_SIZE ��ԭ��1000 �޸�Ϊ1600��  ΪTR9�ķ�����ʾ��  
		 
�塢2022-03-31  ���޸�
		1.���Э��6056,6057,�޸�5008Э��,
		2.�ڴ��ڶ�����ӵ��������ܡ�


�� ��2022-04-01  �޸� ����
     1.  �����ع��ϲ������� source �ļ����е������ļ���
     2.  ��ʱ�� pub.c �е�  s_Hi3520_Monitor.start = true;
		 3.  ��GPS Ϊ 125 �ٶȡ����ڲ��ԡ�
		 4.  �� tr9_Auto_start��� ���ò����Ĵ�ӡ�� *XCL#R,130,   *XCL#R,131,  *XCL#R,194, *XCL#R,195,  *XCL#R,196,

�� ��2022-04-03  �޸� ����
     1.  ��Դ��¼�� �޸� �ж�����: mix.power Ϊ car_alarm.bit.cut_volt
         rcd_power_manage() ; rcd_power_fill_in_infomation();

�� ��2022-04-04  �޸� ����         
     1. �� search_rcd_speed_pointor() ��������� save_rcd_power_p_pos �޸�Ϊ�� save_rcd_speed_p_pos
     2. �� rcd_driver_manage() ��������� ic_driver_lisence = false; ע�͵���
     
�� ��2022-04-05  �޸� ����      
     1. ���� void rcd_ot_judge(void) ����, ����ƣ�ͼ�ʻ������������.
     2. �޸Ĳ������� ���������� rcd_para_inf.en �� rcd_para_inf.type   
		 
ʮ ��2022-04-03	���޸�����
		���ַ������GPS��λģʽ�л����ܣ��˹���ֻ�Ի���Ķ�λģ�������á�
		
ʮһ��2022-04-16 �޸����£�
      ��ʻ��¼�ǵĵ��� 8-15H
      ����15H ��Ҫ���ٱ���600������, �ֽ׶��ṩ��FLASH �洢ֻ��3page, ����90������,�ʶ�spi_flash.h		
      ���е���. ��ϸ�� 15h��flash ����ֵ���޸ġ�������Ӱ��ĵط�Ϊ: ��ӡ�� AREA��flash ����
ʮ����2022-04-05  ���޸� ����
	���ݺ��ܵ绰Ҫ���15H �е�rand_num = rand()%(10+1);�ĳ�rand_num = rand()%(3+1);

*/

/*
.vscode��settings.json
"todo-tree.general.tags": [
        "TODO",
        "BUG",
        "[x]"
    ]
*/
//TODO:��Ǵ���
//HACK:��ǿ�����Ҫ����
//NOTE:���һЩ˵��
//INFO�����һЩ��Ϣ
//TAG:���һ��
//XXX�����һЩ����ʵ�ֵĵط�
//BUG: ���BUG~
//FIXME: �����Ҫ�޸�
//COMT:ע��

#include "include_all.h"

#define SLV_MCU_ENB 1

extern rk_6033_task_t rk_6033_task; //2022-03-17 add by hj
extern RCD_DATA_INPORT_STRUCT data_inport_para;
extern bool ExInport;

void Reset_Hi3520_Pin(void) {
    static bool reset = false;
    static u32 tmr = 0;

    if (reset == false) {
        if (_covern(tmr) > 2) {
            reset = true;
            DIS_RESET_HI35XX;
            logd("rst");
            tmr = 0;
        }
    }
}

int main(void) {
    static u32 task_10ms = 0;  // 10ms ����
    static u32 task_30ms = 0;  // 30ms ����
    static u32 task_100ms = 0; // 100ms ����
    static u32 task_125ms = 0; // 125ms ����
    static u32 task_995ms = 0; // 1000ms ����
    // u8 buff1[30] = {0};
    cli();
    init_drv_cfg();

    //BSP_USB_Init();

    //MMA8452Q_Init(); //���ᴫ����
    init_mid_cfg(); //����������
    init_app_cfg();
    init_alarm_cfg();
    systick_time_init(72); //�δ�ʱ�ӳ�ʼ��,ΪUSB�ṩ us ms ��ʱ
    lcd_init(_true_, _false_);
    //BSP_RX8025Init();

    selfChk_SetTicBoot();
    selfChk_RunLed_initMode(run_led_mode_on_forever, 1, 1);

    wdr();
    sei();

#if (DEBUG_EN_BEEP)
    // buzzer_init(s_key); //����ʱ���η�����
    //COMT:�����������߼���Σ�������೤���Լ���ʱ�رշ������Լ���Ӱ��������������
    beeper(1); //������
#endif

#if (TR_USING_IAP)
    logi("APP<M9C19A_%s><%s> start!!! BOOT<%s>", TR9_SOFT_VERSION_APP, str_app_build_time, TR9_IAP_VERSION);
#else
    logi("APP<M9C19A_%s><%s> start!!!", TR9_SOFT_VERSION_APP, str_app_build_time);
#endif

#if (MODULE_USING_pwr_rst_rk_logSave)
    init_par_log_write();
    log_write(event_mcu_boot);
#endif //#if (MODULE_USING_pwr_rst_rk_logSave)

    chk_err();

    server_cfg.dbip = 0x55;
    register_cfg.all_polar = 0x55;

    test();

    while (1) {
        //food_dog();
        gps_parse_task();    //GPS���ݽ���
        update_timer_task(); //�Զ�ˢʱ��
        uart_manage_task();
        public_manage_task();
        gprs_parse_task(); //GPRS���ݽ���
        lsm6ds3tr_c_task();

#if SLV_MCU_ENB
        key_scan();        //COMT:�����𰴼�״̬��ɨ��
        key_processing();  //COMT:���ݰ���״̬���������ı�˵�״̬����
        menu_processing(); //COMT:���ݲ˵�״̬������ˢ��UI��ʾ
        
        print_auto_process();
        slave_recive_data_proc();
        slave_send_data_proc();
#endif
        tr9_6037_task_CAN();
        gsm_handle_task();

        // ÿ��10�������һ��
        if (_pastn(task_10ms) >= 10) {
            task_10ms = jiffies;
            report_manage_task();
            //led_manage_task();
            cut_oil_ctrl_task();

            if (ExInport == false) {
                rcd_manage_task();
            }

            rcd_data_send();
            stop_rept();
        }

        if (_pastn(task_30ms) >= 35) {
            task_30ms = jiffies;
            adc_power_task();
            upgrade_program_task();
            report_create_task();
#if SLV_MCU_ENB
            low_power_hdl();
            lcd_dis_all();
#endif
            Monitor_Hi3520_Status();
            ic_card_scan();
        }

        if (_pastn(task_100ms) >= 100) {
            task_100ms = jiffies;
            link_manage_task();
            calculation_sum_mile();

            adc_power_task();
            //Mma8452Q_Read();
            auto_ack_tr9_data();

            selfChk_do();
        }
        //2022-03-17 add by hj
        if (_pastn(task_125ms) >= 125) {
            task_125ms = jiffies;
            Rk_Driving_Record_Up(rk_6033_task.cmd);
        }

        if (_pastn(task_995ms) >= 995) {
            task_995ms = jiffies;
#if (P_CAN == _CAN_BASE)
            test_can_speed();
//cali_can_speed_task();
#endif
						//logd("------test000---����,ic_card.name:[%s]-----", ic_card.name);
            debug_task();
            mid_test_task();
            selfChk();

            Reset_Hi3520_Pin();
            terminal_login_task();
            power_manage_task();
            report_key_status_to_tr9();
            ic_card_to_tr9();
            synchronize_flash_spi_para();
            mcu_ask_tr9_www_status();
            if (!car_state.bit.acc) {
                AwakenSet();
            }
        }
        wdr();
        if (ExInport == true) {
            rcd_data_inport();
        }
    }

    //return 0;
}
