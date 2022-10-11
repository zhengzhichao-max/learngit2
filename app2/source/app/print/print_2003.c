/**
  ******************************************************************************
  * @file    print03.c 
  * @author  
  * @Email    
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   2012 ��ʻ��¼�ǹ��ܴ�ӡ
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#include "include_all.h"

#if( P_RCD == RCD_BASE ) 

static uc8   print03_data[2]={0x0D,0x0A};		      				//����
static uc8   print03_data7[]="��ͣ��ʱ����ǰ��15�����ڳ���ÿ���ӵ�ƽ���ٶ�:\r\n";	//	45�ֽ�
static uc8   print03_data9[]="ƣ�ͼ�ʻ��¼:\r\n";			  		//  13�ֽ�
static uc8   print03_data13[]="--------------------------------\r\n"; //  32�ֽ�

/****************************************************************************
* ����:    print03_head ()
* ���ܣ���2003��׼��ӡ��ʻ��¼����Ϣ
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
void print03_head( u8 *buf )
{
	_memcpy_len( buf, (u8 *)print03_data13,34 ) ;
}

/****************************************************************************
* ����:    print03_tail ()
* ���ܣ���2003��׼��ӡ��ʻ��¼����Ϣ
* ��ڲ�������                         
* ���ڲ�������
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
��    �ƣ�print03_car_info
��    �ܣ���ӡ������Ϣ
���������s_CARPARA
���������
��    �أ���
ע�����
*********************************************************/
static u16 print03_car_info(u8 *buf, CAR_CONFIG_STRUCT t)
{
	u8 msg[128]={0};
	u8 len;
	u16 plen;
	u8  src[20]={0};
	
	plen = 0;

	t.license[11] ='\0';
	len = _sprintf_len( (char *)msg, "���ƺ���:%s\r\n", t.license);
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;

	t.type[15] = '\0';
	len = _sprintf_len( (char *)msg, "���Ʒ���:%s\r\n", t.type );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	

	_memcpy_len(src,t.vin,18);
	len = _sprintf_len( (char *)msg, "����ʶ����VIN:%s\r\n", src );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	

	return plen;
}

/*********************************************************
��    �ƣ�print03_driver
��    �ܣ���ӡ��ʻԱ��Ϣ
���������s_Recorder
���������
��    �أ���
ע�����
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
			len = _sprintf_len( (char *)msg, "��ʻ֤����:%s\r\n", t.name);   //driver_cfg.qualification   driver_cfg.license
			_memcpy_len( buf, msg, len );
			buf += len;
			plen += len;	
		}
	}	
	else
	{
		_memcpy( tmp, t.driver_code, 6 );
		len = _sprintf_len( (char *)msg, "��ʻԱ����:%s\r\n", tmp);
		_memcpy_len( buf, msg, len );
		buf += len;
		plen += len;	
	}
	
	len = _sprintf_len( (char *)msg, "��ʻ֤����:%s\r\n", t.license);   //driver_cfg.qualification   driver_cfg.license
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	


	len = _sprintf_len( (char *)msg, "��ӡʱ��:20%02d��%02d��%02d��%02dʱ%02d��%02d��\r\n", 
 		 sys_time.year, sys_time.month, sys_time.date, sys_time.hour,sys_time.min,sys_time.sec );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	

	len = _sprintf_len( (char *)msg, "ͣ��ʱ��:20%02d��%02d��%02d��%02dʱ%02d��%02d��\r\n", 
		 speed_print.HMt[0].year,speed_print.HMt[0].month,speed_print.HMt[0].day,speed_print.HMt[0].hour, speed_print.HMt[0].minute );
	_memcpy_len( buf, msg, len );
	buf += len;
	plen += len;	


	return plen;
}

/*********************************************************
��    �ƣ�print03_speed
��    �ܣ���ӡͣ��ǰ15���ӳ�����Ϣ
���������s_DOUBTSpeed	
���������
��    �أ���
ע�����
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
		len = _sprintf_len( (char *)msg, "%02d��%02d��%02d�� %02d:%02d  %d km/h \r\n", t.HMt[i].year,t.HMt[i].month,t.HMt[i].day,t.HMt[i].hour, t.HMt[i].minute, t.HMt[i].speed );
		_memcpy_len( buf, msg, len );
		buf += len;
		plen += len;		
	}

	return plen;
}


/*********************************************************
��    �ƣ�print03_fatigue
��    �ܣ���ӡƣ�ͼ�ʻ��Ϣ
���������ALL_FATIGUE_STRUCT
���������
��    �أ���
ע�����
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
			len = _sprintf_len( (char *)msg, "��¼ %d:  \r\n", j++ );
			_memcpy_len( buf, msg, len );
			buf += len;
			plen += len;

			len = _sprintf_len( (char *)msg, "��ʼʱ��20%02d/%02d/%02d %02d:%02d����ʱ��20%02d/%02d/%02d %02d:%02d\r\n", sFt.cell[i].start.year, \
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
��������: Rdr_Cacl_Day_Hour
��    ��: ����1��Сʱǰ���ʱ��
�������: pyz ƫ��ֵ  0: ��  1: ��
�������:
ע������: ����������ʱ ����1��Сʱǰ���ʱ��
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
			if( hour > 1 )  //����1��
			{
				hour--;
			}	
			else	//�ж��·� 
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


//��ȡ���15���ӵ�ƽ������
//2015-01-22 ����1 �� 
/***********************************************************
��������: average_print_time
��    ��: ͣ��ǰ15����ƽ���ٶȴ�ӡǰ ����
�������:
�������:
ע������: ��ӡǰ����
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
			if( (rcd360_inf.speed < 0xf0)&&(rcd360_inf.speed>0) )    //�޸�Ϊ = 0  ; 150122
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
��    �ƣ�PrinterToLcd
��    �ܣ���ӡ��������/��ӡ��
���������
���������
��    �أ���ӡ��LCD����
ע�����
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
* ����:    print03_rcd_infomation ()
* ���ܣ���2003��׼��ӡ��ʻ��¼����Ϣ
* ��ڲ�������                         
* ���ڲ�������
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


