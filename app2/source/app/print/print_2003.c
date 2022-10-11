/**
  ******************************************************************************
  * @file    print03.c 
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

static uc8   print03_data[2]={0x0D,0x0A};		      				//换行
static uc8   print03_data7[]="自停车时刻起前推15分钟内车辆每分钟的平均速度:\r\n";	//	45字节
static uc8   print03_data9[]="疲劳驾驶记录:\r\n";			  		//  13字节
static uc8   print03_data13[]="--------------------------------\r\n"; //  32字节

/****************************************************************************
* 名称:    print03_head ()
* 功能：按2003标准打印行驶记录仪信息
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void print03_head( u8 *buf )
{
	_memcpy_len( buf, (u8 *)print03_data13,34 ) ;
}

/****************************************************************************
* 名称:    print03_tail ()
* 功能：按2003标准打印行驶记录仪信息
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void print03_tail( u8 *buf )
{
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;

	_memcpy_len( buf, (u8 *)print03_data13,20 );
	buf += 20;

	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;	
	_memcpy_len( buf, (u8 *)print03_data,2 );
	buf += 2;	



}


/*********************************************************
名    称：print03_car_info
功    能：打印车辆信息
输入参数：s_CARPARA
输出参数：
返    回：无
注意事项：
*********************************************************/
static u16 print03_car_info(u8 *buf, CAR_CONFIG_STRUCT t)
{
	u8 msg[128]={0};
	u8 len;
	u16 plen;
	u8  src[20]={0};
	
	plen = 0;

	t.license[11] ='\0';
	len = _sprintf_len( (char *)msg, "车牌号码:%s\r\n", t.license);
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;

	t.type[15] = '\0';
	len = _sprintf_len( (char *)msg, "车牌分类:%s\r\n", t.type );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	

	_memcpy_len(src,t.vin,18);
	len = _sprintf_len( (char *)msg, "车辆识别码VIN:%s\r\n", src );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	

	return plen;
}

/*********************************************************
名    称：print03_driver
功    能：打印驾驶员信息
输入参数：s_Recorder
输出参数：
返    回：无
注意事项：
*********************************************************/
u16 print03_driver( u8 *buf, DRIVER_CONFIG_STRUCT t )
{
	u8 msg[250]={0};
	u8 len;
	u16 plen = 0;
	u8 tmp[9]={0};

	if( recorder_para.pr_driver == 0 )
	{
		if( t.name_len > 0  && t.name_len < 11 )
		{
			len = _sprintf_len( (char *)msg, "驾驶证姓名:%s\r\n", t.name);   //driver_cfg.qualification   driver_cfg.license
			_memcpy_len( buf, msg, len );
			buf += len;
			plen += len;	
		}
	}	
	else
	{
		_memcpy( tmp, t.driver_code, 6 );
		len = _sprintf_len( (char *)msg, "驾驶员代码:%s\r\n", tmp);
		_memcpy_len( buf, msg, len );
		buf += len;
		plen += len;	
	}
	
	len = _sprintf_len( (char *)msg, "驾驶证号码:%s\r\n", t.license);   //driver_cfg.qualification   driver_cfg.license
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	


	len = _sprintf_len( (char *)msg, "打印时间:20%02d年%02d月%02d日%02d时%02d分%02d秒\r\n", 
 		 sys_time.year, sys_time.month, sys_time.date, sys_time.hour,sys_time.min,sys_time.sec );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	

	len = _sprintf_len( (char *)msg, "停车时间:20%02d年%02d月%02d日%02d时%02d分%02d秒\r\n", 
		 speed_print.HMt[0].year,speed_print.HMt[0].month,speed_print.HMt[0].day,speed_print.HMt[0].hour, speed_print.HMt[0].minute );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	


	return plen;
}

/*********************************************************
名    称：print03_speed
功    能：打印停车前15分钟车速信息
输入参数：s_DOUBTSpeed	
输出参数：
返    回：无
注意事项：
*********************************************************/
u16 print03_speed( u8 *buf, AVERAGE_SPEED_STRUCT  t )
{
	u8 msg[500]={0};
	u8 len;
	u8  i;
	u16 plen = 0;

	_memcpy_len( buf, (u8 *)print03_data7, 47 );
	buf += 47;
	plen += 47;

	for(i=0; i<15; i++){
		if(t.HMt[i].hour>23||t.HMt[i].minute>59||t.HMt[i].speed>120){
			len = _sprintf_len( (char *)msg, "_ _ _\r\n");
			_memcpy_len( buf, msg, len );
			buf += len;
			plen += len;	
			init_printf_rcd_info();			
			return plen;
		}
	}
	
	for( i=0; i<15; i++ )
	{
		len = _sprintf_len( (char *)msg, "%02d年%02d月%02d日 %02d:%02d  %d km/h \r\n", t.HMt[i].year,t.HMt[i].month,t.HMt[i].day,t.HMt[i].hour, t.HMt[i].minute, t.HMt[i].speed );
		_memcpy_len( buf, msg, len );
		buf += len;
		plen += len;		
	}

	return plen;
}


/*********************************************************
名    称：print03_fatigue
功    能：打印疲劳驾驶信息
输入参数：ALL_FATIGUE_STRUCT
输出参数：
返    回：无
注意事项：
*********************************************************/
u16 print03_fatigue( u8 *buf, ALL_FATIGUE_STRUCT sFt )
{
	u8 msg[200]={0};
	u8 len, i,j=1;
	u16 plen = 0;
	bool ret;
	bool verify_err;
	
	_memcpy_len( buf, (u8 *)print03_data9, 15 );
	buf += 15;
	plen += 15;

	verify_err = false;
	for( i = 0; i< RCD_PRINT_FATCNTS_MAX_SIZE; i++ )
	{
		if( sFt.cell[i].write_ed!= 00 ){
			ret = _verify_time(&sFt.cell[i].start);
			if(ret == false){
				verify_err = true;
			}

			ret = _verify_time(&sFt.cell[i].end);
			if(ret == false){
				verify_err = true;
			}

			if(verify_err){
                logd("print03 fatigue err : verify");
                len = _sprintf_len( (char *)msg, "_ _ _\r\n");
				_memcpy_len( buf, msg, len );
				buf += len;
				plen += len;	
				init_printf_rcd_info();			
				return plen;
			}
		}
	}
	
	for( i = 0; i< RCD_PRINT_FATCNTS_MAX_SIZE; i++ )
	{
		if( sFt.cell[i].write_ed!= 00 )
		{
			len = _sprintf_len( (char *)msg, "记录 %d:  \r\n", j++ );
			_memcpy_len( buf, msg, len );
			buf += len;
			plen += len;

			len = _sprintf_len( (char *)msg, "开始时间20%02d/%02d/%02d %02d:%02d结束时间20%02d/%02d/%02d %02d:%02d\r\n", sFt.cell[i].start.year, \
			 sFt.cell[i].start.month,  sFt.cell[i].start.date,  sFt.cell[i].start.hour, sFt.cell[i].start.min, sFt.cell[i].end.year, sFt.cell[i].end.month, \
			 sFt.cell[i].end.date,sFt.cell[i].end.hour, sFt.cell[i].end.min);

			_memcpy_len( buf, msg, len );
			buf += len;
			plen += len;			
		}	
	}	
	return plen;
}


u8 HextoBin( u8 bByte )
{
	u8 ucRn=0;
	
	ucRn =  bByte >> 4 ;
	ucRn *= 10;
	ucRn += bByte & 0xF;

	return ucRn;
}

u8 BintoHex( u8 bByte )
{
	u8 ucRn=0;
	
	ucRn =  bByte / 10 ;
	ucRn *= 16;
	ucRn += bByte % 10;

	return ucRn;
}


/************************************************************
函数名称: Rdr_Cacl_Day_Hour
功    能: 计算1个小时前后的时间
输入参数: pyz 偏移值  0: 减  1: 加
输出参数:
注意事项: 根据年月日时 计算1个小时前后的时间
************************************************************/
time_t Rdr_Cacl_Day_Hour( time_t stm, u8 pyz )
{
	u8 hour, month, day, min;

	month = (stm.month);	 //HextoBin
	day = (stm.date);
	hour = (stm.hour);
	min = (stm.min);
	if( pyz == 0 )
	{
		if( min >= 1 )
			min--;
		else
		{
			if( hour > 1 )  //大于1号
			{
				hour--;
			}	
			else	//判断月份 
			{
				if( day > 1 )
					day--;
				hour = 23;
			}
			min = 59;			
		}
	}

	stm.hour= (hour);		  //BintoHex
	stm.date= (day);
	stm.month= (month);
	stm.min= (min);
	return stm;
}


//读取最后15分钟的平均车速
//2015-01-22 修正1 次 
/***********************************************************
函数名称: average_print_time
功    能: 停车前15分钟平均速度打印前 排序
输入参数:
输出参数:
注意事项: 打印前调用
***********************************************************/
void average_print_time( time_t t1 )
{
	signed char i;
	bool ret = false;
	time_t time, otime;	
	u8 buf_t[20];
	u16 index_t;
	S_RCD_360HOUR_INFOMATION	rcd360_inf;
	u8 cnt = 0;
	bool open = false;
	u8 page_err = 0;
	u8 tmr_r = 0;

	otime = sys_time;
	time = sys_time;
	index_t = rcd360_m.head.node;
	while( cnt < 15 )
	{
		spi_flash_read(buf_t,_rcd360_t_addr(index_t),sizeof(time_t));	
		_memcpy((u8 *)&time, buf_t,6);
		ret = _verify_time(&time);
		if( (ret == false) && (!open) ) 
		{
			index_t =  decp_rcd_p(T_RCD_360, index_t, 1 );	
	
			if( ++page_err >= 2 )
			{
				time = sys_time;
			print_ioop:
				for( i=0; i<15; i++ )
				{
					speed_print.HMt[i].year = time.year;
					speed_print.HMt[i].month = time.month;
					speed_print.HMt[i].day = time.date;
					speed_print.HMt[i].hour = time.hour;
					speed_print.HMt[i].minute = time.min;		
					time = Rdr_Cacl_Day_Hour(time, 0); 				
				}
				return;
			}	
			continue;
		}	

		for(i=59;i>=0;i--)
		{
			if( ret )
				spi_flash_read((u8*)&rcd360_inf,_rcd360_d_addr(index_t,i),sizeof(rcd360_inf));	
			else
			{
				time = Rdr_Cacl_Day_Hour(time, 0); 	
				rcd360_inf.speed = 0;
			}
			if( (rcd360_inf.speed < 0xf0)&&(rcd360_inf.speed>0) )    //修改为 = 0  ; 150122
			{
				if( !open )
					otime = time;
			write_time_15:
				speed_print.HMt[cnt].year = otime.year;
				speed_print.HMt[cnt].month = otime.month;
				speed_print.HMt[cnt].day = otime.date;
				speed_print.HMt[cnt].hour = otime.hour;
				speed_print.HMt[cnt].minute = i;
				if( rcd360_inf.speed >= 0xf0 )
					speed_print.HMt[cnt].speed = 0x00;
				else
					speed_print.HMt[cnt].speed = rcd360_inf.speed;
				cnt++;
				open = true;
				otime.min =  i;
				otime = Rdr_Cacl_Day_Hour(otime, 0); 	
			}
			else if( open )
			{
				rcd360_inf.speed = 0;
				goto write_time_15;	
			}
			if( cnt >= 15 )
				return;
		}		
		
		index_t =  decp_rcd_p(T_RCD_360, index_t, 1 );	

		if( ++tmr_r >= 3 )
		{
			goto print_ioop;
		}	
	}	
}

/*********************************************************
名    称：PrinterToLcd
功    能：打印到调度屏/打印机
输入参数：
输出参数：
返    回：打印到LCD长度
注意事项：
*********************************************************/
u16 printer03_MDT( u8 *buf )
{
	u16 len = 0;
	u16 tmp = 0;
		
	print03_head( buf );
	len += 34;
	buf += 34;
	tmp = print03_car_info( buf, car_cfg );

    logd("print01");
    len += tmp;
	buf += tmp;
    logd("print02");

    average_print_time(sys_time);
	
	tmp = print03_driver( buf, driver_cfg );   //driver_cfg.license
	len += tmp;
	buf += tmp;
    logd("print03");

    tmp = print03_speed( buf, speed_print);
	len += tmp;
	buf += tmp;
    logd("print04");
    tmp = print03_fatigue( buf, fatigue_print); 
	len += tmp;
	buf += tmp;
    logd("print05");
    print03_tail(buf);
	len += 38;
	
	return len;
}



/****************************************************************************
* 名称:    print03_rcd_infomation ()
* 功能：按2003标准打印行驶记录仪信息
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void print03_rcd_infomation(void)
{
#if 0
	print_test();

#else
	u8  print_buff[1150] = {0};	
	u16 p_len;

	_memset((u8*)&print_buff,0x00 ,1150);	
	p_len = printer03_MDT( print_buff );  	
	if( p_len > 1100 ){
        p_len = 1100;
    }

    logd("print num:%d", p_len);

    //handle_send_ext_msg( FROM_SLAVE, PRINT_TO_LCD, print_buff,  p_len ); //0x8402 
    load_print_data(print_buff, p_len);

    //logd("%s",print_buff);
#endif	
}

#endif


