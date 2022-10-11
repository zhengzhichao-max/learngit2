/**
  ******************************************************************************
  * @file    print_2012.c 
  * @author  
  * @Email    
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   2012 行驶记录仪功能打印
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#include "include_all.h"
#if( P_RCD == RCD_BASE ) 
static uc8   print12_data[2]={0x0D, 0x0A};
static uc8   print12_data5[]="签名:\r\n";
static uc8   print12_data6[]="无\r\n";
static uc8   print12_data8[]="2个日历天内超时驾驶记录:\r\n";
static uc8   print12_data10[]="                    ------------\r\n";
static uc8   print12_data13[]="--------------------------------\r\n";

/****************************************************************************
* 名称:    print12_head ()
* 功能：打印信息头
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void print12_head(u8 *buf)
{
	_memcpy(buf, (u8 *)print12_data13, 34) ;
}

/****************************************************************************
* 名称:    print12_tail ()
* 功能：打印信息尾
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void print12_tail(u8 *buf)
{
	_memcpy(buf, (u8 *)print12_data, 2);
	buf += 2;

	_memcpy(buf, (u8 *)print12_data5, 7);
	buf += 7;

	_memcpy(buf, (u8 *)print12_data, 2);
	buf += 2;
	_memcpy(buf, (u8 *)print12_data, 2);
	buf += 2;
	_memcpy(buf, (u8 *)print12_data, 2);
	buf += 2;
	_memcpy(buf, (u8 *)print12_data, 2);
	buf += 2;
	_memcpy(buf, (u8 *)print12_data10, 20);
	buf += 20;

	_memcpy_len( buf, (u8 *)print12_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print12_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print12_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print12_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print12_data,2 );
	buf += 2;		
}


/*********************************************************
名    称：print12_car_info
功    能：打印车辆信息
输入参数：car_cfg
输出参数：
返    回：无
注意事项：
*********************************************************/
u16 print12_car_info(u8 *buf, CAR_CONFIG_STRUCT t)
{
	u8 pbuf[128];
	u8 len;
	u16 plen = 0;

	t.license[11] ='\0';
	len = _sprintf_len((char *)pbuf, "机动车号牌号码:%s\r\n", t.license);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	t.type[15] = '\0';
	len = _sprintf_len((char *)pbuf, "机动车号牌分类:%s\r\n", t.type);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	t.vin[17] ='\0';
	len = _sprintf_len((char *)pbuf, "车辆识别码VIN:%s\r\n", t.vin);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	return plen;
}

/*********************************************************
名    称：print12_driver
功    能：打印驾驶员信息
输入参数：s_Recorder
输出参数：
返    回：无
注意事项：
*********************************************************/
u16 print12_driver(u8 *buf, DRIVER_CONFIG_STRUCT t)
{
	u8 pbuf[250];
	u8 len;
	u16 plen = 0;
	u8 tmp[24]={0};

	
	if( recorder_para.pr_driver == 0 )
	{
		len = _sprintf_len( (char *)tmp, "驾驶证姓名:%s\r\n", t.name);	 //driver_cfg.qualification   driver_cfg.license
		_memcpy_len( buf, tmp, len );
		buf += len;
		plen += len;	
	}
	_memset( tmp, 0, 24 );
	
	_memcpy_len( tmp, t.license, 20 );
	len = _sprintf_len((char *)pbuf, "驾驶证号码:%s\r\n", tmp);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	_memcpy(pbuf, "速度状态: 正常\r\n", 16);
	_memcpy(buf, pbuf, 16);
	buf += 16;
	plen += 16;

	len = _sprintf_len((char *)pbuf, "打印时间:20%02d年%02d月%02d日%02d时%02d分%02d秒\r\n", 
 		 sys_time.year, sys_time.month, sys_time.date, sys_time.hour, sys_time.min, sys_time.sec);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;
	
	return plen;
}

/*********************************************************
名    称：print12_fatigue
功    能：打印疲劳驾驶信息
输入参数：ALL_FATIGUE_STRUCT
输出参数：
返    回：无
注意事项：
*********************************************************/
u16 print12_fatigue(u8 *buf, ALL_FATIGUE_STRUCT t)
{
	u8 pbuf[500];
	u8 i, j=1;
	u16 len;
	u16 plen = 0;
	bool ret;
	bool bull= false;
	_memcpy(buf, (u8 *)print12_data8, 28);
	buf += 26;
	plen += 26;
	for(i = 0;i< RCD_PRINT_FATCNTS_MAX_SIZE;i++)
	{
		if(t.cell[i].write_ed!= 00)
		{
			ret = _verify_time(&t.cell[i].start);
			if(ret == false)
				continue;
			
			ret = _verify_time(&t.cell[i].end);
			if(ret == false)
				continue;

			_memset(pbuf, 0, 100);   
			len = _sprintf_len((char *)pbuf, "记录 %d: \r\n", j++);
			_memcpy(buf, pbuf, len);
			buf += len;
			plen += len;
			
			_memcpy(buf, "机动车驾驶证号码: \r\n", 20);
			buf += 20;
			plen += 20;
			_memset(pbuf, 0, 100);

			t.cell[i].driver_lse[18] = '\0';
			len = _strlen((u8*)&t.cell[i].driver_lse);
			_memcpy(buf, (u8*)&t.cell[i].driver_lse, len);
			buf += len;
			plen += len;

			_memcpy(buf, (u8 *)print12_data, 2);
			buf += 2;
			
			_memset(pbuf, 0, 100);
			len = _sprintf_len((char *)pbuf, "开始时间20%02d/%02d/%02d %02d:%02d\r\n", t.cell[i].start.year, 
								t.cell[i].start.month, t.cell[i].start.date, t.cell[i].start.hour, t.cell[i].start.min);
			_memcpy(buf, pbuf, len);
			buf += len;
			plen += len;

			_memset(pbuf, 0, 100);
			len = _sprintf_len((char *)pbuf, "结束时间20%02d/%02d/%02d %02d:%02d\r\n", t.cell[i].end.year, t.cell[i].end.month, 
								t.cell[i].end.date, t.cell[i].end.hour, t.cell[i].end.min);
			_memcpy(buf, pbuf, len);
			buf += len;
			plen += len;	

			bull = true;
		}	
	}	

	if(bull == false)
	{
		_memcpy(buf, (u8 *)print12_data6, 2);
		buf += 2;
		plen += 2;
	}
	return plen;
}


/*********************************************************
名    称：PrinterToLcd
功    能：打印到调度屏/打印机
输入参数：
输出参数：
返    回：打印到LCD长度
注意事项：
*********************************************************/
u16 print12_mdt(u8 *buf)
{
	u16 len = 0;
	u16 tmp = 0;
		
	print12_head(buf);
	len += 34;
	buf += 34;

	tmp = print12_car_info(buf, car_cfg);
	len += tmp;
	buf += tmp;
		
	tmp = print12_driver(buf, driver_cfg);
	len += tmp;
	buf += tmp;
	
	tmp = print12_fatigue(buf, fatigue_print);
	len += tmp;
	buf += tmp;
	
	print12_tail(buf);
	len += 30;

	len += 10;
	return len;
}

/****************************************************************************
* 名称:    print12_rcd_2012 ()
* 功能：按2012标准打印行驶记录仪信息
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void print12_rcd_infomation(void)
{
	u16 print12_len;
	u8  print12_buf[1150] = {0};

	_memset((u8*)&print12_buf, 0x00 , 1150);
	print12_len = print12_mdt(print12_buf);		
	if( print12_len > 1100 )
		print12_len = 1100;
	
	//handle_send_ext_msg(FROM_SLAVE, PRINT_TO_LCD, print12_buf, print12_len);
	load_print_data(print12_buf, print12_len);
}

#endif


