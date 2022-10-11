/**
  ******************************************************************************
  * @file    print_2012.c 
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
static uc8   print12_data[2]={0x0D, 0x0A};
static uc8   print12_data5[]="ǩ��:\r\n";
static uc8   print12_data6[]="��\r\n";
static uc8   print12_data8[]="2���������ڳ�ʱ��ʻ��¼:\r\n";
static uc8   print12_data10[]="                    ------------\r\n";
static uc8   print12_data13[]="--------------------------------\r\n";

/****************************************************************************
* ����:    print12_head ()
* ���ܣ���ӡ��Ϣͷ
* ��ڲ�������                         
* ���ڲ�������
****************************************************************************/
void print12_head(u8 *buf)
{
	_memcpy(buf, (u8 *)print12_data13, 34) ;
}

/****************************************************************************
* ����:    print12_tail ()
* ���ܣ���ӡ��Ϣβ
* ��ڲ�������                         
* ���ڲ�������
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
��    �ƣ�print12_car_info
��    �ܣ���ӡ������Ϣ
���������car_cfg
���������
��    �أ���
ע�����
*********************************************************/
u16 print12_car_info(u8 *buf, CAR_CONFIG_STRUCT t)
{
	u8 pbuf[128];
	u8 len;
	u16 plen = 0;

	t.license[11] ='\0';
	len = _sprintf_len((char *)pbuf, "���������ƺ���:%s\r\n", t.license);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	t.type[15] = '\0';
	len = _sprintf_len((char *)pbuf, "���������Ʒ���:%s\r\n", t.type);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	t.vin[17] ='\0';
	len = _sprintf_len((char *)pbuf, "����ʶ����VIN:%s\r\n", t.vin);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	return plen;
}

/*********************************************************
��    �ƣ�print12_driver
��    �ܣ���ӡ��ʻԱ��Ϣ
���������s_Recorder
���������
��    �أ���
ע�����
*********************************************************/
u16 print12_driver(u8 *buf, DRIVER_CONFIG_STRUCT t)
{
	u8 pbuf[250];
	u8 len;
	u16 plen = 0;
	u8 tmp[24]={0};

	
	if( recorder_para.pr_driver == 0 )
	{
		len = _sprintf_len( (char *)tmp, "��ʻ֤����:%s\r\n", t.name);	 //driver_cfg.qualification   driver_cfg.license
		_memcpy_len( buf, tmp, len );
		buf += len;
		plen += len;	
	}
	_memset( tmp, 0, 24 );
	
	_memcpy_len( tmp, t.license, 20 );
	len = _sprintf_len((char *)pbuf, "��ʻ֤����:%s\r\n", tmp);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;

	_memcpy(pbuf, "�ٶ�״̬: ����\r\n", 16);
	_memcpy(buf, pbuf, 16);
	buf += 16;
	plen += 16;

	len = _sprintf_len((char *)pbuf, "��ӡʱ��:20%02d��%02d��%02d��%02dʱ%02d��%02d��\r\n", 
 		 sys_time.year, sys_time.month, sys_time.date, sys_time.hour, sys_time.min, sys_time.sec);
	_memcpy(buf, pbuf, len);
	buf += len;
	plen += len;
	
	return plen;
}

/*********************************************************
��    �ƣ�print12_fatigue
��    �ܣ���ӡƣ�ͼ�ʻ��Ϣ
���������ALL_FATIGUE_STRUCT
���������
��    �أ���
ע�����
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
			len = _sprintf_len((char *)pbuf, "��¼ %d: \r\n", j++);
			_memcpy(buf, pbuf, len);
			buf += len;
			plen += len;
			
			_memcpy(buf, "��������ʻ֤����: \r\n", 20);
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
			len = _sprintf_len((char *)pbuf, "��ʼʱ��20%02d/%02d/%02d %02d:%02d\r\n", t.cell[i].start.year, 
								t.cell[i].start.month, t.cell[i].start.date, t.cell[i].start.hour, t.cell[i].start.min);
			_memcpy(buf, pbuf, len);
			buf += len;
			plen += len;

			_memset(pbuf, 0, 100);
			len = _sprintf_len((char *)pbuf, "����ʱ��20%02d/%02d/%02d %02d:%02d\r\n", t.cell[i].end.year, t.cell[i].end.month, 
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
��    �ƣ�PrinterToLcd
��    �ܣ���ӡ��������/��ӡ��
���������
���������
��    �أ���ӡ��LCD����
ע�����
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
* ����:    print12_rcd_2012 ()
* ���ܣ���2012��׼��ӡ��ʻ��¼����Ϣ
* ��ڲ�������                         
* ���ڲ�������
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


