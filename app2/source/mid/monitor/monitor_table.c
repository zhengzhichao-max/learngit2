
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include_all.h"

#if (0)
#include "monitor_table.h"
#include "monitor.h"
#include "clib.h"
#include "logger.h"
#endif

static void print_space(log_level_enum log_level, unsigned char cnt) {
    unsigned char i;
    for (i = 0; i < cnt; i++) {
        logb_empty(log_level, " ");
    }
}

#if (TEST_MONITOR) //for test
static int cmd_set_parm(argv_list_st *argv_list);
static int cmd_get_parm(argv_list_st *argv_list);
#endif //#if (TEST_MONITOR) //for test

// static int cmd_sensor(argv_list_st *argv_list);
// static int cmd_task(argv_list_st *argv_list);

//当有多个命令时，打印多个命令看看//cmd_name == NULL时，打印所有命令信息
void cmd_help_promt(char *cmd_name, log_level_enum log_level, const char *who, int line) {
    const cmd_list_st *cmd;

    logb_empty(log_level, "\r\n");

    logb(log_level, "    %s    %s    %s                  %s", COMMAND_NAME, SHORT_NAME, DESCRIPTION, USAGE);
    logb(log_level, "--------------------------[%s, %d][%s]------------------------------", who, line, cmd_name == NULL ? "null" : cmd_name);
    uart1_tran_task_nms(5);

    for (cmd = g_cmd_table; cmd != NULL && cmd->name != NULL; ++cmd) {
        if (cmd_name == NULL
            || _strnicmp(cmd_name, cmd->name, strlen((const char *)cmd_name)) == 0
            || _strnicmp(cmd_name, cmd->short_name, strlen((const char *)cmd_name)) == 0) {
            logb_NoNewLn(log_level, "    ");
            logb_empty(log_level, "%s", cmd->name);
            print_space(log_level, SIZE_COMMAND_NAME - strlen(cmd->name));

            logb_empty(log_level, "%s", cmd->short_name);
            print_space(log_level, SIZE_SHORT_NAME - strlen(cmd->short_name));

            logb_empty(log_level, "%s", cmd->description);
            print_space(log_level, SIZE_DESCRIPTION - strlen(cmd->description));

            logb_empty(log_level, "%s\r\n", cmd->usage);

            uart1_tran_task_nms(5);
        }
    }

    logb_endColor_newLn(log_level, );
    uart1_tran_task_nms(5);
}

//*---------------------------------命令函数区----------------------------------
//*#############################################################################
//*函 数 名 ：cmd_help
//*功    能 ：help命令处理
//*说    明 ：
//*参    数 ：
//*返 回 值 ：
//*历史记录 ：
//*状    态 ：
//*#############################################################################
static int cmd_help(argv_list_st *argv_list) {
    cmd_help_promt(NULL, log_level_debug, __func__, __LINE__);
    return 0;
}

//*#############################################################################
//*函 数 名 ：dev_reboot
//*功    能 ：系统重启
//*说    明 ：
//*参    数 ：
//*返 回 值 ：
//*历史记录 ：
//*状    态 ：
//*#############################################################################
static int mcu_reboot(argv_list_st *argv_list) {
    //OSSemPost(sys_reboot_sem);
#if (DEBUG_EN_WATCH_DOG)

    log_write(event_mcu_boot_monitor);

    while (1) { ; }
#else
    logf("ret mcu, undef");
    return 0;
#endif
}

//*#############################################################################
//*函 数 名 ：dev_reboot
//*功    能 ：系统重启
//*说    明 ：
//*参    数 ：
//*返 回 值 ：
//*历史记录 ：
//*状    态 ：
//*#############################################################################
static int _6062_reboot(argv_list_st *argv_list) {
    //tr9_cmd_6062//0x6062//6.79 MCU复位【RK->MCU】
    logd("_6062_reboot do");
    return 0;
}

#if (TEST_MONITOR) //for test
//*#############################################################################
//*函 数 名 ：cmd_set_parm
//*功    能 ：设置指令解析
//*说    明 ：
//*参    数 ：
//*返 回 值 ：
//*历史记录 ：
//*状    态 ：
//*#############################################################################
static int cmd_set_parm(argv_list_st *argv_list) {
    logf("IMPORTANT_BUT_NEED_DESIGN, undef");
#if (IMPORTANT_BUT_NEED_DESIGN)
    char *p;
    char *pbuf = NULL;
    char tbuf[25] = {0};
    u32 u32SetData;
    u32 mode;
    u32 opentime;
    u32 cycletime;

    p = &argv_list->argv[0][4];
    //help
    pbuf = strstr(p, "help");
    if (pbuf != NULL) {
        logd("set main,cmnet,183.62.55.236,9001\r\n");
        logd("set backup,cmnet,183.62.55.236,9002\r\n");
        logd("set main,cmnet,fengyehu888.wicp.io,47044\r\n");
        logd("set backup,cmnet,fengyehu888.wicp.io,47044\r\n");
        logd("set tid,08806002\r\n");
        logd("set aim,08806002\r\n");
        logd("set bdpdu,0 (0:BDV2.1; 1:BDV3.0; 2:CSHGBD3; )\r\n");
        logd("set footmode,1,50,60(0:实时(RD常开); 1:低功耗(RD发送时开，平时关)\r\n");
        logd("set sosmode,1,50,60.(0:紧急报警; 1:自定义)\r\n");
        logd("set rdbsi,1\r\n");
        logd("set lte_printf,1\r\n");
        logd("set bt_printf,1\r\n");
        logd("set gnss_printf,1\r\n");
        logd("set rdss_printf,1\r\n");
        logd("set rola_printf,1\r\n");
        logd("set bt_name,1234567\r\n");
        logd("set lbstoken,8mR000K45g5y59L8\r\n");
        logd("set netmode,2(0:NONE; 1:MOBLE NET 2:WIFI 3:Bluetooth)\r\n");
        logd("set xhserver\r\n");
        logd("set wifissid,harmonyos\r\n");
        logd("set wifipwd,abc123456\r\n");
        logd("set wifiserverport,9001\r\n");
        logd("set cshgupdate,1\r\n");
        logd("set runmode,0,0,0(0:Common,1:Timing report,2:data transmission; 0:Report No Sleep,1:Sleep; 0:Sos No Sleep,1:Sleep)\r\n");
        logd("set quiet,1\r\n");
        logd("set bt_poweron,1\r\n");
        logd("set bsisend,0\r\n");
        logd("set configmodetime,300\r\n");
        logd("set rdssversion,0(0:联参 1:兵器)\r\n");

        return CMD_SUCCESS;
    }
    //set main,cmnet,183.62.55.236,9001		//30byte
    //set main,cmnet,183.62.55.236,9002		//16byte
    //set main,cmnet,183.62.55.235,8888		//30byte,生产测试平台
    pbuf = strstr(p, "main");
    if (pbuf != NULL) {
        memset(terminal_cfg_module.param.lte_parame.main_server_ip, 0, sizeof(terminal_cfg_module.param.lte_parame.main_server_ip));
        memset(tbuf, 0, sizeof(tbuf));

        sscanf(p, "%*[^,],%[^,],%[^,],%s", terminal_cfg_module.param.lte_parame.main_server_ip, terminal_cfg_module.param.lte_parame.main_server_ip, tbuf);
        logd("lte_parame.main_server_ip:%s\r\n", terminal_cfg_module.param.lte_parame.main_server_ip);
        logd("lte_parame.main_server_ip: %s\r\n", terminal_cfg_module.param.lte_parame.main_server_ip);
        terminal_cfg_module.param.lte_parame.main_server_port = atoi(tbuf);
        logd("main_lte_parame.main_server_port:%d\r\n", terminal_cfg_module.param.lte_parame.main_server_port);
        terminal_cfg_module.param_save();
        logd("Set main server Success.\r\n");
        return CMD_SUCCESS;
    }
#endif /* IMPORTANT_BUT_NEED_DESIGN */

#if 0
    //set backup,cmnet,183.62.55.236,9001
	pbuf = strstr(p, "backup");
	if(pbuf != NULL)
	{	
		memset(terminal_cfg_module.param.lte_parame.backup_server_ip,0,sizeof(terminal_cfg_module.param.lte_parame.backup_server_ip));
		memset(tbuf,0,sizeof(tbuf));
		
		sscanf(p,"%*[^,],%[^,],%[^,],%s",terminal_cfg_module.param.lte.backup_server_apn,terminal_cfg_module.param.lte_parame.backup_server_ip,tbuf);
		logd("lte.backup_server_apn:%s\r\n",terminal_cfg_module.param.lte.backup_server_apn);
		logd("lte_parame.backup_server_ip: %s\r\n",terminal_cfg_module.param.lte_parame.backup_server_ip);
		terminal_cfg_module.param.lte_parame.main_server_port=atoi(tbuf);
		logd("lte_parame.main_server_port:%d\r\n",terminal_cfg_module.param.lte_parame.main_server_port);
		terminal_cfg_module.param_save();
		logd("Set backup server Success.\r\n");
		return CMD_SUCCESS;
	}
	//set id,08806002
	pbuf = strstr(p, "tid,");			//加个t，以防与ssid冲突
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%lX",&u32SetData);
		logd("u32ID:%lX\r\n",u32SetData);

		if(terminal_cfg_module.param.terminal_id!=u32SetData)
		{
			terminal_cfg_module.param.terminal_id=u32SetData;
			sprintf(Device_SN,"%08X",terminal_cfg_module.param.terminal_id);
			Device_SN[8]=0;
			logd("terminal_id:%08X\r\n",terminal_cfg_module.param.terminal_id);
			terminal_cfg_module.param_save();
			logd("Set terminal_id Success.\r\n");
#ifdef CONFIG_BLUETOOTH_FUN
			if(terminal_cfg_module.param.Net_Mode==DEFAULT_PARAM_NET_MODE_BLUETOOTH)
			{
				OSSemPost(sBluetooth_UseSNName_Sem);
			}
#endif
		}
		else
		{
			logd("Set terminal_id Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set report,15
	pbuf = strstr(p, "report,");
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%d",&u32SetData);
		logd("Report interval Value:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.default_report_interval!=u32SetData)
		{
			terminal_cfg_module.param.default_report_interval=u32SetData;
			terminal_cfg_module.param_save();
			logd("set Report interval Value Success.\r\n");
		}
		else
		{
			logd("set Report interval Value Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
#endif

#if (IMPORTANT_BUT_NEED_DESIGN)
    //set workmode,0
    pbuf = strstr(p, "workmode,");
    if (pbuf != NULL) {
        sscanf(p, "%*[^,],%s", tbuf);
        u32SetData = atoi(tbuf);
        logd("Workmode:%d\r\n", u32SetData);
        if (terminal_cfg_module.param.work_mode.poweron_mode != u32SetData) {
            terminal_cfg_module.param.work_mode.poweron_mode = u32SetData;
            terminal_cfg_module.param_save();
            logd("set workmode Success.\r\n");
        } else {
            logd("set workmode Success.(Already)\r\n");
        }
        return CMD_SUCCESS;
    }
#endif /* #if (IMPORTANT_BUT_NEED_DESIGN) */

#if 0
	//set aim,sim
	pbuf = strstr(p, "aim,sim");
	if(pbuf != NULL)
	{	
		logd("IC_Card_ASCII:%s\r\n",IC_Card_ASCII);
		if(BD_CARD_NUM_LEN==7)
		{
			sprintf(AIM_Card_ASCII,"%07lX",terminal_cfg_module.param.aim_id);
		}
		else
		{
			sprintf(AIM_Card_ASCII,"%08lX",terminal_cfg_module.param.aim_id);
		}
		if(memcmp(IC_Card_ASCII,AIM_Card_ASCII,BD_CARD_NUM_LEN)!=0)
		{
			memcpy(AIM_Card_ASCII,IC_Card_ASCII,10);
			sscanf(AIM_Card_ASCII,"%lX",&u32SetData);
			terminal_cfg_module.param.aim_id=u32SetData;
			terminal_cfg_module.param_save();
			logd("set aim_id Success.\r\n");
		}
		else
		{
			logd("set aim_id Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set aim,08806002
	pbuf = strstr(p, "aim,");
	if(pbuf != NULL)
	{	
		sscanf(p,"%*[^,],%lX",&u32SetData);
		logd("tbuf:%lX\r\n",u32SetData);
		if(terminal_cfg_module.param.aim_id!=u32SetData)
		{
			terminal_cfg_module.param.aim_id=u32SetData;
			if(BD_CARD_NUM_LEN==7)
			{
				logd("aim_id:%07lX\r\n",terminal_cfg_module.param.aim_id);
				memset(AIM_Card_ASCII,0,sizeof(AIM_Card_ASCII));
				sprintf(AIM_Card_ASCII,"%07lX",terminal_cfg_module.param.aim_id);
			}
			else
			{
				logd("aim_id:%08lX\r\n",terminal_cfg_module.param.aim_id);
				memset(AIM_Card_ASCII,0,sizeof(AIM_Card_ASCII));
				sprintf(AIM_Card_ASCII,"%08lX",terminal_cfg_module.param.aim_id);
			}
			terminal_cfg_module.param_save();
			logd("set aim_id Success.\r\n");
		}
		else
		{
			logd("set aim_id Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set bdpdu,0
	pbuf = strstr(p, "bdpdu,");
	if(pbuf != NULL)
	{	
		sscanf(p,"%*[^,],%d",&u32SetData);
		logd("bdpdu:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.bdpud_ver!=u32SetData)
		{
			if(terminal_cfg_module.param.bdpud_ver>PROTOCOL_CSHGBD3)
			{
				logd("set bdpud_ver Error.Data exceed.\r\n");
				return CMD_SUCCESS;
			}
			terminal_cfg_module.param.bdpud_ver=u32SetData;
			terminal_cfg_module.param_save();
			logd("set bdpud_ver Success.\r\n");
		}
		else
		{
			logd("set bdpud_ver Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
#endif

#if (IMPORTANT_BUT_NEED_DESIGN)
    //set footmode,1,50,60
    pbuf = strstr(p, "footmode,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d,%d,%d", &mode, &opentime, &cycletime);
        if ((mode != terminal_cfg_module.param.work_mode.footprint_working_mode) || (opentime != terminal_cfg_module.param.work_mode.footprint_opentime) || (cycletime != terminal_cfg_module.param.work_mode.footprint_cycletime)) {
            terminal_cfg_module.param.work_mode.footprint_working_mode = mode;
            terminal_cfg_module.param.work_mode.footprint_opentime = opentime;
            terminal_cfg_module.param.work_mode.footprint_cycletime = cycletime;
            logd("work_mode.footprint_working_mode: %d\r\n", terminal_cfg_module.param.work_mode.sos_mode);
            logd("work_mode.footprint_opentime: %ds\r\n", terminal_cfg_module.param.work_mode.footprint_opentime);
            logd("work_mode.footprint_cycletime:%ds\r\n", terminal_cfg_module.param.work_mode.footprint_cycletime);
            terminal_cfg_module.param_save();
            logd("Set footmode Success.\r\n");
        } else {
            logd("Set footmode Success.(Already)\r\n");
        }
        return CMD_SUCCESS;
    }
    //set sosmode,1,50,60
    pbuf = strstr(p, "sosmode,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d,%d,%d", &mode, &opentime, &cycletime);
        if ((mode != terminal_cfg_module.param.work_mode.sos_mode) || (opentime != terminal_cfg_module.param.work_mode.footprint_opentime) || (cycletime != terminal_cfg_module.param.work_mode.footprint_cycletime)) {
            terminal_cfg_module.param.work_mode.sos_mode = mode;
            terminal_cfg_module.param.work_mode.sos_opentime = opentime;
            terminal_cfg_module.param.work_mode.sos_cycletime = cycletime;
            logd("work_mode.sos_mode: %d\r\n", terminal_cfg_module.param.work_mode.sos_mode);
            logd("work_mode.sos_opentime: %ds\r\n", terminal_cfg_module.param.work_mode.sos_opentime);
            logd("work_mode.sos_cycletime:%ds\r\n", terminal_cfg_module.param.work_mode.sos_cycletime);
            terminal_cfg_module.param_save();
            logd("Set sosmode Success.\r\n");
        } else {
            logd("Set sosmode Success.(Already)\r\n");
        }
        return CMD_SUCCESS;
    }
#endif /* #if (IMPORTANT_BUT_NEED_DESIGN) */
    //set rdbsi,1
#if 0
	pbuf = strstr(p, "rdbsi,");
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%d",&u32SetData);
		logd("rdbsi:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.rdbsi!=u32SetData)
		{
			terminal_cfg_module.param.rdbsi=u32SetData;
			terminal_cfg_module.param_save();
			logd("set rdbsi Success.\r\n");
		}
		else
		{
			logd("set rdbsi Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set lbstoken,8mR000K45g5y59L8
	pbuf = strstr(p, "lbstoken,");
	if(pbuf != NULL)
	{	
		memset(terminal_cfg_module.param.Lbs_Token,0,sizeof(terminal_cfg_module.param.Lbs_Token));	
		sscanf(p,"%*[^,],%s",terminal_cfg_module.param.Lbs_Token);
		logd("Lbs_Token:%s\r\n",terminal_cfg_module.param.Lbs_Token);
		terminal_cfg_module.param_save();
		logd("Set Lbs Token Success.\r\n");
		return CMD_SUCCESS;
	}
	//set xhserver
	pbuf = strstr(p, "xhserver");
	if(pbuf != NULL)
	{	
		strcpy((char *)terminal_cfg_module.param.lte_parame.main_server_ip, DEFAULT_PARAM_MAIN_SERVER_APN);
		strcpy((char *)terminal_cfg_module.param.lte_parame.main_server_ip, DEFAULT_PARAM_MAIN_SERVER_IP);
		terminal_cfg_module.param.lte_parame.main_server_port = DEFAULT_PARAM_SERVER_PORT;
		logd("lte_parame.main_server_ip:%s\r\n",terminal_cfg_module.param.lte_parame.main_server_ip);
		logd("lte_parame.main_server_ip: %s\r\n",terminal_cfg_module.param.lte_parame.main_server_ip);
		logd("main_lte_parame.main_server_port:%d\r\n",terminal_cfg_module.param.lte_parame.main_server_port);
		terminal_cfg_module.param_save();
		logd("Set main server Success.\r\n");
		return CMD_SUCCESS;
	}
	//set fengserver
	pbuf = strstr(p, "fengserver");
	if(pbuf != NULL)
	{	
		strcpy((char *)terminal_cfg_module.param.lte_parame.main_server_ip, "cmnet");
		strcpy((char *)terminal_cfg_module.param.lte_parame.main_server_ip, "fengyehu888.wicp.io");
		terminal_cfg_module.param.lte_parame.main_server_port = 47044;
		logd("lte_parame.main_server_ip:%s\r\n",terminal_cfg_module.param.lte_parame.main_server_ip);
		logd("lte_parame.main_server_ip: %s\r\n",terminal_cfg_module.param.lte_parame.main_server_ip);
		logd("main_lte_parame.main_server_port:%d\r\n",terminal_cfg_module.param.lte_parame.main_server_port);
		terminal_cfg_module.param_save();
		logd("Set main server Success.\r\n");
		return CMD_SUCCESS;
	}
	//set netmode,1
	pbuf = strstr(p, "netmode,");
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%d",&u32SetData);
		logd("Net Mode Value:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.Net_Mode!=u32SetData)
		{
			if(u32SetData>DEFAULT_PARAM_NET_MODE_BLUETOOTH)
			{
				logd("Set Net Mode Value Error.(Data Exceed)\r\n");
				return CMD_SUCCESS;
			}
			terminal_cfg_module.param.Net_Mode=u32SetData;
			terminal_cfg_module.param_save();
			switch(terminal_cfg_module.param.Net_Mode)
			{
				case DEFAULT_PARAM_NET_MODE_MOBILE:
					logd("Net_Mode:MOBILE NET.(%d)\r\n",terminal_cfg_module.param.Net_Mode);
					break;

				case DEFAULT_PARAM_NET_MODE_WIFI:
					logd("Net_Mode:WIFI.(%d)\r\n",terminal_cfg_module.param.Net_Mode);
					break;

				case DEFAULT_PARAM_NET_MODE_BLUETOOTH:
					logd("Net_Mode:Bluetooth.(%d)\r\n",terminal_cfg_module.param.Net_Mode);
					break;

				case DEFAULT_PARAM_NET_MODE_NONE:
					logd("Net_Mode:NONE.(%d)\r\n",terminal_cfg_module.param.Net_Mode);
					break;
			}		
		}
		else
		{
			logd("Set Net Mode Value Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set wifissid,harmonyos
	pbuf = strstr(p, "wifissid,");
	if(pbuf != NULL)
	{	
		sscanf(p,"%*[^,],%s",tbuf);
		if((strcmp(terminal_cfg_module.param.WIFI_ssid,tbuf)!=0))
		{
			memset(terminal_cfg_module.param.WIFI_ssid,0,sizeof(terminal_cfg_module.param.WIFI_ssid));	
			memcpy(terminal_cfg_module.param.WIFI_ssid,tbuf,strlen(tbuf));	
			logd("WIFI ssid:%s\r\n",terminal_cfg_module.param.WIFI_ssid);
			terminal_cfg_module.param_save();
			logd("Set WIFI ssid Success.\r\n");
		}
		else
		{
			logd("Set WIFI ssid Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set wifipwd,abc123456
	pbuf = strstr(p, "wifipwd,");
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%s",tbuf);
		if((strcmp(terminal_cfg_module.param.WIFI_pwd,tbuf)!=0))
		{
			memset(terminal_cfg_module.param.WIFI_pwd,0,sizeof(terminal_cfg_module.param.WIFI_pwd));	
			memcpy(terminal_cfg_module.param.WIFI_pwd,tbuf,strlen(tbuf));	
			sscanf(p,"%*[^,],%s",terminal_cfg_module.param.WIFI_pwd);
			logd("WIFI pwd:%s\r\n",terminal_cfg_module.param.WIFI_pwd);
			terminal_cfg_module.param_save();
			logd("Set WIFI passwd Success.\r\n");
		}
		else
		{
			logd("Set WIFI passwd Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set wifiserverport,9001
	pbuf = strstr(p, "wifiserverport");
	if(pbuf != NULL)
	{			
		sscanf(p,"%*[^,],%d",&u32SetData);
		logd("WIFI_lte_parame.main_server_port:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.WIFI_lte_parame.main_server_port!=u32SetData)
		{
			terminal_cfg_module.param.WIFI_lte_parame.main_server_port=u32SetData;
			terminal_cfg_module.param_save();
			logd("Set WIFI server port Success.\r\n");
		}
		else
		{
			logd("Set WIFI server port Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
#endif
#ifdef CONFIG_EXTENDCOM_FUN
    //set cshgupdate,1
    pbuf = strstr(p, "cshgupdate,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d", &u32SetData);
        logd("cshgupdate:%d\r\n", u32SetData);
        if (u32SetData == 1) {
            g_bExtendCom_CSHG_RDSS_Update_Sign = true;
            logd("set cshgupdate open.\r\n");
        } else {
            g_bExtendCom_CSHG_RDSS_Update_Sign = false;
            logd("set cshgupdate close.\r\n");
        }
        return CMD_SUCCESS;
    }
#endif

#if (IMPORTANT_BUT_NEED_DESIGN)
    //set runmode,0,0,0
    pbuf = strstr(p, "runmode,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d,%d,%d", (int *)&terminal_cfg_module.param.work_mode.poweron_mode,
               (int *)&terminal_cfg_module.param.work_mode.footprint_working_mode,
               (int *)&terminal_cfg_module.param.work_mode.sos_mode);

        logd("poweron_mode:%s.(%d)\r\n", (terminal_cfg_module.param.work_mode.poweron_mode == POWERON_COMMON) ? "Common" : ((terminal_cfg_module.param.work_mode.poweron_mode == POWERON_FOOTPRINT) ? "Timing report" : "Data transmission"), terminal_cfg_module.param.work_mode.poweron_mode);
        logd("timingreport_mode:Idle %s.(%d)\r\n", (terminal_cfg_module.param.work_mode.footprint_working_mode == IDLE_NOSLEEP) ? "No Sleep" : "Sleep", terminal_cfg_module.param.work_mode.footprint_idle_mode);
        logd("sos_mode:Idle %s.(%d)\r\n", (terminal_cfg_module.param.work_mode.sos_mode == SOS_EMERGENY) ? "emergeny" : "custom", terminal_cfg_module.param.work_mode.sos_mode);
        terminal_cfg_module.param_save();
        logd("Set runmode Success.\r\n");
        return CMD_SUCCESS;
    }
#endif /* #if (IMPORTANT_BUT_NEED_DESIGN) */

    //set quiet,1
#if 0
	pbuf = strstr(p, "quiet,");
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%d",&u32SetData);
		logd("quiet able:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.quiet_able!=u32SetData)
		{
			terminal_cfg_module.param.quiet_able=u32SetData;
			terminal_cfg_module.param_save();
			logd("Set quiet able Success.\r\n");
		}
		else
		{
			logd("Set quiet able Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
#endif

#if (IMPORTANT_BUT_NEED_DESIGN)
    //set btpoweron,1
    pbuf = strstr(p, "bt_poweron,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d", &u32SetData);
        logd("bt poweron:%d\r\n", u32SetData);
        if (terminal_cfg_module.param.bt_parame.PowerOnEnable != u32SetData) {
            terminal_cfg_module.param.bt_parame.PowerOnEnable = u32SetData;
            terminal_cfg_module.param_save();
            logd("Set bluetooth poweron Success.\r\n");
        } else {
            logd("Set bluetooth poweron Success.(Already)\r\n");
        }
        return CMD_SUCCESS;
    }
    //set lte,1
    pbuf = strstr(p, "lte_printf,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d", &u32SetData);
        logd("lte logd:%d\r\n", u32SetData);
        if (u32SetData == 1) {
            SETING_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, LTE_PRINTF);
        } else {
            RESET_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, LTE_PRINTF);
        }
        return CMD_SUCCESS;
    }
    pbuf = strstr(p, "bt_printf,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d", &u32SetData);
        logd("bt logd:%d\r\n", u32SetData);
        if (u32SetData == 1) {
            SETING_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, BT_PRINTF);
        } else {
            RESET_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, BT_PRINTF);
        }
        return CMD_SUCCESS;
    }
    pbuf = strstr(p, "gnss_printf,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d", &u32SetData);
        logd("gnss logd:%d\r\n", u32SetData);
        if (u32SetData == 1) {
            SETING_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, GNSS_PRINTF);
        } else {
            RESET_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, GNSS_PRINTF);
        }
        return CMD_SUCCESS;
    }
    pbuf = strstr(p, "rdss_printf,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d", &u32SetData);
        logd("rdss logd:%d\r\n", u32SetData);
        if (u32SetData == 1) {
            SETING_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, RDSS_PRINTF);
        } else {
            RESET_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, RDSS_PRINTF);
        }
        return CMD_SUCCESS;
    }
    pbuf = strstr(p, "rola_printf,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%d", &u32SetData);
        logd("rola logd:%d\r\n", u32SetData);
        if (u32SetData == 1) {
            SETING_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, ROLAWAN_PRINTF);
        } else {
            RESET_BIT(terminal_cfg_module.param.console_parame.printf_ctrl, ROLAWAN_PRINTF);
        }
        return CMD_SUCCESS;
    }
    pbuf = strstr(p, "bt_name,");
    if (pbuf != NULL) {
        sscanf(pbuf, "%*[^,],%s", tbuf);
        logd("bt name:%s\r\n", tbuf);
        bluetooth_module.set_name(tbuf);
        return CMD_SUCCESS;
    }
#endif /* #if (IMPORTANT_BUT_NEED_DESIGN) */

#if 0
    //set bsisend,0
	pbuf = strstr(p, "bsisend,");
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%d",&u32SetData);
		logd("bsi send:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.bsi_send!=u32SetData)
		{
			terminal_cfg_module.param.bsi_send=u32SetData;
			terminal_cfg_module.param_save();
			logd("Set bsi send Success.\r\n");
		}
		else
		{
			logd("Set bsi send Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set configmodetime,300
	pbuf = strstr(p, "configmodetime,");
	if(pbuf != NULL)
	{	
		sscanf(pbuf,"%*[^,],%d",&u32SetData);
		logd("config mode time:%d\r\n",u32SetData);
		if(terminal_cfg_module.param.ConfigMode_Time!=u32SetData)
		{
			terminal_cfg_module.param.ConfigMode_Time=u32SetData;
			terminal_cfg_module.param_save();
			logd("Set config mode time Success.\r\n");
		}
		else
		{
			logd("Set config mode time Success.(Already)\r\n");
		}
		return CMD_SUCCESS;
	}
	//set rdssversion,0
	pbuf = strstr(p, "rdssversion,0");
	if(pbuf != NULL)
	{	
		logd("RDSS Wait for Switch 联参.(waitfor 10 seconds).\r\n");
		ExtendCom_SwitchRdssVer_Handle(0);
		return CMD_SUCCESS;
	}
	//set rdssversion,1
	pbuf = strstr(p, "rdssversion,1");
	if(pbuf != NULL)
	{	
		logd("RDSS Wait for Switch 兵器.(waitfor in 10 seconds).\r\n");
		ExtendCom_SwitchRdssVer_Handle(1);		
		return CMD_SUCCESS;
	}

	//set factory
	pbuf = strstr(p, "factory");	
	if(pbuf != NULL)
	{	
		TerminalParam_RestoreFactory_ExceptID();
		logd("set,factory\r\n");
		return CMD_SUCCESS;
	}
	logd("$set,parm,error.........\r\n");
#endif

    return 0;
}

//*#############################################################################
//*函 数 名 ：cmd_get_parm
//*功    能 ：设置指令解析
//*说    明 ：
//*参    数 ：
//*返 回 值 ：
//*历史记录 ：
//*状    态 ：
//*#############################################################################
static int cmd_get_parm(argv_list_st *argv_list) {
    logf("IMPORTANT_BUT_NEED_DESIGN, undef");

#if (IMPORTANT_BUT_NEED_DESIGN)
    char *p, *str[3];

    p = &argv_list->argv[0][4];
    //help
    if (((*p == 'H') && (*(p + 1) == 'E') && (*(p + 2) == 'L') && (*(p + 3) == 'P'))
        || ((*p == 'h') && (*(p + 1) == 'e') && (*(p + 2) == 'l') && (*(p + 3) == 'p'))) {
        logd("get 3		log report interval Value.\r\n");
        logd("get 4		log terminal_id.\r\n");
        logd("get 5		log server para.\r\n");
        logd("get 6		log WIFI para.\r\n");
        logd("get 8		log Work mode para.\r\n");

        logd("get all		log all para.\r\n");
    } else if (((*p == 'A') && (*(p + 1) == 'L') && (*(p + 2) == 'L'))
               || ((*p == 'a') && (*(p + 1) == 'l') && (*(p + 2) == 'l'))) {
        terminal_cfg_module.param_read();
    } else {
        switch (*p) {
            //			case '3':
            //				logd("Report_interval:%d second.\r\n",terminal_cfg_module.param.default_report_interval);
            //				break;
            //
            //			case '4':
            //				logd("Terminal_id:%08X\r\n",terminal_cfg_module.param.terminal_id);
            //				break;

        case '5':
            logd("lte_parame.main_server_ip:%s\r\n", terminal_cfg_module.param.lte_parame.main_server_ip);
            logd("lte_parame.main_server_ip:%s\r\n", terminal_cfg_module.param.lte_parame.main_server_ip);
            logd("lte_parame.main_server_port:%d\r\n", terminal_cfg_module.param.lte_parame.main_server_port);

            logd("lte.backup_server_apn:%s\r\n", terminal_cfg_module.param.lte_parame.backup_server_apn);
            logd("lte_parame.backup_server_ip:%s\r\n", terminal_cfg_module.param.lte_parame.backup_server_ip);
            logd("lte_parame.main_server_port:%d\r\n", terminal_cfg_module.param.lte_parame.main_server_port);
            break;

        case '7':
            break;

        case '8':
            sscanf("Common Timing report Sos", "%s %s %s", str[0], str[1], str[2]);
            logd("poweron_mode:%s.(%d)\r\n", str[terminal_cfg_module.param.work_mode.poweron_mode], terminal_cfg_module.param.work_mode.poweron_mode);
            logd("timingreport_mode:Idle %s.(%d)\r\n", (terminal_cfg_module.param.work_mode.footprint_idle_mode == IDLE_NOSLEEP) ? "No Sleep" : "Sleep", terminal_cfg_module.param.work_mode.footprint_idle_mode);
            logd("sos_mode:Idle %s.(%d)\r\n", (terminal_cfg_module.param.work_mode.sos_mode == SOS_EMERGENY) ? "Emergeny" : "Custom", terminal_cfg_module.param.work_mode.sos_mode);
            break;

        case '9':

            break;

        default:
            break;
        }
    }
#endif /* #if (IMPORTANT_BUT_NEED_DESIGN) */

    return 0;
}

#if (0)
//*#############################################################################
//*函 数 名 :cmd_sensor
//*功    能 ：printf sensor
//*说    明 ：
//*参    数 ：
//*返 回 值 ：
//*历史记录 ：
//*状    态 ：
//*#############################################################################
static int
cmd_sensor(argv_list_st *argv_list) {
    char *p;

    p = argv_list->argv[1];
    //    sensor_printf = (*p == '0') ? 0 : 1;
    return 0;
}
#endif
#endif //#if (TEST_MONITOR) //for test

#if (0)
//*#############################################################################
//*函 数 名 :cmd_sensor
//*功    能 ：printf sensor
//*说    明 ：
//*参    数 ：
//*返 回 值 ：
//*历史记录 ：
//*状    态 ：
//*#############################################################################
static int cmd_task(argv_list_st *argv_list) {
    u8 err, prio;
    char *name;
    OS_STK_DATA StackBytes;

    printf("CPU usage:%d%c\r\n", OSCPUUsage, '%');
    printf("task name             priority      use stack      free stack \n\r"); //6,6,6
    printf("---------------------------------------------------------------\r\n");
    for (prio = 0; prio < OS_LOWEST_PRIO + 1; prio++) {
        if (OSTCBPrioTbl[prio] != OS_NULL) {
            OSTaskNameGet(prio, (u8 **)&name, &err);
            OSTaskStkChk(prio, &StackBytes);
            printf("%s", name);
            print_space(SIZE_COMMAND_NAME - strlen(name));
            print_space(6);
            printf("%d", prio);
            print_space(strlen("priority") - (prio > 10 ? 2 : 1));
            print_space(6);
            printf("%d", StackBytes.OSUsed);
            print_space(strlen("use stack") - (StackBytes.OSUsed > 100 ? 3 : (StackBytes.OSUsed > 10 ? 2 : 1)));
            print_space(6);
            printf("%d", StackBytes.OSFree);
            printf("\r\n");
        }
    }
    return 0;
}
#endif

static int nln(argv_list_st *argv_list) {
    logd("-----------------------");
    return 0;
}

// #include"monitor_mcu_flash.h"

//* 命令列表（每一行占用20个字节） */
const cmd_list_st g_cmd_table[] = {
    {"help", "hp", "print help message", "help", cmd_help},
    {"newln", "nln", "--", "nln", nln},

#if MODULE_USING_pwr_rst_rk_logSave && 1
    {"mcuflash", "mcuf", "mcu flash do", "mcuf", cmd_mcuflash},
#endif //#if MODULE_USING_pwr_rst_rk_logSave

#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && MODULE_DEBUG_self_chk&&1
    {"selfchk", "sc", "", "sc 0-1 0-2", cmd_self_chk},
#endif //#if defined(TR_DEBUG) && defined(USE_DEBUG_FUNCTION) && MODULE_DEBUG_self_chk&&1

    {"util", "util", "", "", cmd_util},

//  {	"version",  "ver",   "print version message",	"version",
//  &CMD_Version  },
//  {	"date",     "dt",   "print date time",			"date",
//  &CMD_DataTime }, {	"free",     "free", "print memory message",
//  "free",     	&CMD_Free	},
#ifdef CMD_TEMP
    {"temp", "temp", "print temp data", "temp x(0~255)", &CMD_TEMP},
#endif
#ifdef CMD_GNSS
    {"gnss", "gnss", "print gnss info", "gnss x(x:1~9)", &CMD_GNSS},
#endif
#ifdef RDSS_GNSS
    {"rdss", "rdss", "print rdss info", "rdss x(same gnss)", &CMD_RDSS},
#endif
#ifdef CMD_RDSS_CMD
    {"rdcmd", "rdcmd", "send rdss cmd", "rdcmd xx x(x:cmd)", &CMD_RDSS_CMD},
#endif
#ifdef CMD_Bat
    {"bat", "bat", "print battery voltage", "bat", &CMD_Bat},
#endif
#ifdef CMD_SOS
    {"sos", "sos", "set sos work state", "sos 0/1", &CMD_SOS},
#endif
#ifdef CMD_SWMode
    {"swmode", "swmode", "switch work mode", "swmode 0~4", &CMD_SWMode},
#endif
#ifdef CONFIG_EXTENDCOM_FUN
    {"extend", "extend", "set extend com para", "extend enable/disable/buad x",
     &CMD_Extend},
#endif

//	{ "lte",   "lte","logd LTE info",	  		"LTE x(same gnss
//x)",cmd_lte},

#if CONFIG_WIFI_FUN
    {"wifi", "wifi", "logd wifi info", "wifi x(same gnss x)", &CMD_WIFI},
#endif
#ifdef CONFIG_DB44_PROTOCOL
    {"hewp", "hewp", "print hewp info", "hewp x(same gnss x)", &CMD_HEWP},
#endif
#ifdef CONFIG_BLUETOOTH_FUN
    {"bluetooth", "bt", "print bluetooth info", "bt x(same gps x)",
     &CMD_Bluetooth},
#endif
//  { "clear",	"clr",	"clear screen",				"clear",
//  &CMD_CLEAR},
#ifdef CMD_FACTORY
    {"factory", "factory", "restore factory", "factory x", &CMD_FACTORY},
#endif

    //0x6062//6.79 MCU复位【RK->MCU】
    {"6062", "6062", "mcu rst", "6062", _6062_reboot},
    {"reboot", "rst", "mcu rst", "reboot", mcu_reboot},

#if (TEST_MONITOR) //for test
    {"reboot", "res", "restart system", "reboot", mcu_reboot},
    {"reboot", "re", "restart system", "reboot", mcu_reboot},
    {"set", "set", "set cmd", "set parm", cmd_set_parm},
    {"set2", "set2", "set cmd", "set parm", cmd_set_parm},
    {"gets", "get", "get cmd", "get x(x:1~9)", cmd_get_parm},
    {"gea", "get", "get cmd", "get x(x:1~9)", cmd_get_parm},
    {"ena", "en", "get cmd", "get x(x:1~9)", cmd_get_parm},
#endif //for test

    //{"sensor", "sensor", "sensor logd", "sensor x(x:0/1)", cmd_sensor},
    // {"task", "task", "task stack", "task", cmd_task},
    // {"exit", "exit", "reselect work mode", "exit", cmd_exit},
    {NULL}};
