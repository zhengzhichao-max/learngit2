
/********************************************************************************
  * @file    main.c
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  主程序
  1.程序采用时间调度的方式，管理每个任务。
  2.每个任务需要尽量缩短执行时间，提高系统响应时间。
  3.复杂任务可通过任务分解的方式实现
  4.避免在中断中处理大量数据，避免使用中断查询的方式。
  5.尽量保持程序风格一致，方便日后维护
  ******************************************************************************
  * @attention
  ******************************************************************************
*/

/*2022-03-23 修改如下:
  1. 为调试行驶记录仪数据,  把 #define MAX_CAN_BUF 80  暂时修改为 10 *** 注意恢复 03-26 已经恢复****
  2. 把 #define UART1_RECV_BUFF_MAX_LGTH        ((uint)512) 修改为
        #define UART1_RECV_BUFF_MAX_LGTH        ((uint)1024)
     把 #define UART1_SEND_BUFF_MAX_LGTH        ((uint)512) 修改为
        #define UART1_SEND_BUFF_MAX_LGTH        ((uint)1024)   
        ****** 注意 恢复 ********

二、 2022-03-25修改如下:
     1. 在 小秦 同步过来的 压缩包: HGXH-BASE-GD305_RK3568-2022-03-24-1245-秦  上面进行修改。
     2. 在 结构体 Rk_Task_Manage_t 增加部分变量
     3. rcd_assemble_body() 函数中 增加 对无时间段数据读取的判断.
     4. 更新 tr9_Auto_start() 该函数主要是为应对 启动
     5. 在 uart1_debug_proc() 函数中增加 串口启动 读行驶记录仪数据 方式。
     6. 在 gps_debonce() 函数中增加 对GPS 的初始速度赋值, 调试使用, 记住 发行时 注释掉** 已经恢复。
     7. 更新 tr9_Export_state() 函数。
     8. 更新 Rk_Driving_Record_Up() 函数
     

三、 2022-03-28修改如下:
     针对 行驶记录仪 cmd=0x08H - 0x15H 的 处理  在: Rk_Driving_Record_Up() 函数中,修改如下
           rcd_send_data(FROM_U4, RCD_C_OK, rsp.id, usb_rsp.rcdDataBuf, (u16)usb_rsp.len);  修改为：
           rcd_send_data(FROM_U4, RCD_C_OK, cmd, usb_rsp.rcdDataBuf, (u16)usb_rsp.len); 

三、 2022-03-30修改如下:
     1. 用小秦提供的 source_03_30的压缩包更新所有程序
     2. 并针对 CMD 11\12\14 无记录时, 行驶记录仪不应答进行了处理。行驶记录仪无记录时,也需要应答
        修改地方为: 08H ~ 15H 的相关部分, 详细见 时间戳 2022-03-30 注释。
     
四、 2022-03-31修改如下:     
     1.  在 Rk_Driving_Record_Up() 函数中 修改：
         1.1 针对 08-15H 的发送包长度 控制在512个字节内，调整参数 rk_6033_task.frame 的值
         1.2 rk_6033_task.u8_delay 这个延时时间 需要做成32位的,原来定义有问题。
             在 rk_6033_task_t结构体中修改
         1.3 发现 在08 读取的时候， 第一次开启时 cmd 本应该时 08H 在组包时，会变成 00H.  
             这里是代码中的笔误bug  故在重发时, 则需要 用 rk_6033_task.cmd 替换rsp.id
         1.4 在重发记录仪数据时逻辑上有错误,已修改
         
     2. 把 RCD_COM_MAX_SIZE 定义   由800byte 修改为1024 byte()
     3. 修改 T1_BUF_SIZE 有原来1000 修改为1600，  为TR9的发送显示。  
		 
五、2022-03-31  秦修改
		1.添加协议6056,6057,修改5008协议,
		2.在串口二中添加导航屏功能。


六 、2022-04-01  修改 如下
     1.  采用秦工合并的最新 source 文件夹中的所有文件。
     2.  暂时打开 pub.c 中的  s_Hi3520_Monitor.start = true;
		 3.  打开GPS 为 125 速度。用于测试。
		 4.  在 tr9_Auto_start添加 设置参数的打印。 *XCL#R,130,   *XCL#R,131,  *XCL#R,194, *XCL#R,195,  *XCL#R,196,

七 、2022-04-03  修改 如下
     1.  电源记录中 修改 判断条件: mix.power 为 car_alarm.bit.cut_volt
         rcd_power_manage() ; rcd_power_fill_in_infomation();

八 、2022-04-04  修改 如下         
     1. 把 search_rcd_speed_pointor() 函数里面的 save_rcd_power_p_pos 修改为： save_rcd_speed_p_pos
     2. 把 rcd_driver_manage() 函数里面的 ic_driver_lisence = false; 注释掉。
     
九 、2022-04-05  修改 如下      
     1. 增加 void rcd_ot_judge(void) 函数, 用于疲劳驾驶条件触发处理.
     2. 修改参数设置 触发条件： rcd_para_inf.en 及 rcd_para_inf.type   
		 
十 、2022-04-03	秦修改如下
		在字符屏添加GPS定位模式切换功能，此功能只对华大的定位模块起作用。
		
十一、2022-04-16 修改如下：
      行驶记录仪的导入 8-15H
      由于15H 需要至少保留600条数据, 现阶段提供的FLASH 存储只有3page, 最大才90条数据,故对spi_flash.h		
      进行调整. 详细见 15h的flash 参数值的修改。可能受影响的地方为: 打印和 AREA的flash 区域。
十二、2022-04-05  秦修改 如下
	根据何总电话要求把15H 中的rand_num = rand()%(10+1);改成rand_num = rand()%(3+1);

*/

/*
.vscode，settings.json
"todo-tree.general.tags": [
        "TODO",
        "BUG",
        "[x]"
    ]
*/
//TODO:标记待办
//HACK:标记可能需要更改
//NOTE:添加一些说明
//INFO：表达一些信息
//TAG:标记一下
//XXX：标记一些草率实现的地方
//BUG: 标记BUG~
//FIXME: 标记需要修复
//COMT:注释

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
    static u32 task_10ms = 0;  // 10ms 任务
    static u32 task_30ms = 0;  // 30ms 任务
    static u32 task_100ms = 0; // 100ms 任务
    static u32 task_125ms = 0; // 125ms 任务
    static u32 task_995ms = 0; // 1000ms 任务
    // u8 buff1[30] = {0};
    cli();
    init_drv_cfg();

    //BSP_USB_Init();

    //MMA8452Q_Init(); //三轴传感器
    init_mid_cfg(); //包括：串口
    init_app_cfg();
    init_alarm_cfg();
    systick_time_init(72); //滴答时钟初始化,为USB提供 us ms 延时
    lcd_init(_true_, _false_);
    //BSP_RX8025Init();

    selfChk_SetTicBoot();
    selfChk_RunLed_initMode(run_led_mode_on_forever, 1, 1);

    wdr();
    sei();

#if (DEBUG_EN_BEEP)
    // buzzer_init(s_key); //调试时屏蔽蜂鸣器
    //COMT:按键蜂鸣，逻辑如何，包括响多长，以及何时关闭蜂鸣，以及不影响其他任务性能
    beeper(1); //开动作
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
        gps_parse_task();    //GPS数据解析
        update_timer_task(); //自动刷时间
        uart_manage_task();
        public_manage_task();
        gprs_parse_task(); //GPRS数据解析
        lsm6ds3tr_c_task();

#if SLV_MCU_ENB
        key_scan();        //COMT:仅负责按键状态的扫描
        key_processing();  //COMT:根据按键状态，（仅）改变菜单状态变量
        menu_processing(); //COMT:根据菜单状态变量，刷新UI显示
        
        print_auto_process();
        slave_recive_data_proc();
        slave_send_data_proc();
#endif
        tr9_6037_task_CAN();
        gsm_handle_task();

        // 每隔10毫秒调度一次
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
						//logd("------test000---姓名,ic_card.name:[%s]-----", ic_card.name);
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
