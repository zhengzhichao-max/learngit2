/**
  ******************************************************************************
  * @file    tts.c 
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   ����������Ϣ����
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#include "include_all.h"

TTS_MANAGE_STRUCT tts_m; //������������
tts_alarm_manage  tts_c;
static u32 delay_ring = 15; 		//delay_tick
bool   _b_gghypt = false;


/*********************************************************
��    �ƣ�write_tts
��    �ܣ�TTS�ӿں�������TTS �豸д����Ҫ��������Ϣ
�����������
��    ������
��д���ڣ�2013-12-01
��������:
1.��ʱ������ͳһʹ��tts_alarm_task ����
2.�¼�������ֱ�ӵ���
**********************************************************/
bool write_tts(u8 *str , u16 len)
{
#if 0
	if( run.voice_sw == 0x55 )
	{
		CLOSE_PHONE;
		return true;
	}		
	
	if(str == NULL )
	{
		return false;
	}
	if( len > 500 )
		len = 500;

	if( pub_io.b.dvr_open==false )
		play_tts_by_gsm(str , len);
#endif
	return true;
}


/*********************************************************
��    �ƣ�load_tts_rpt_alarm
��    �ܣ����ر�������
�����������
��    ������
��д���ڣ�2013-12-01
**********************************************************/
static void load_tts_rpt_alarm1(VEHICLE_ALARM_UNION* f)
{
	u8 rpt_buf[256];
	u16 rpt_len;

	rpt_len  = 0;

	if((f->bit.speed_over)||(f->bit.speed_near))
	{
		if(f->bit.speed_over)
		{

			rpt_len += _memcpy_len(&rpt_buf[rpt_len], "���ѳ��������", 14);
		}	
		else
		{
			rpt_len += _memcpy_len(&rpt_buf[rpt_len], "����������", 10);
		}

		//�˴��������·�ߵȱ���
	
		write_tts(rpt_buf, rpt_len);
	}

}


static void load_tts_rpt_alarm2(VEHICLE_ALARM_UNION* f)
{
	u8 rpt_buf[256];
	u16 rpt_len;

	rpt_len  = 0;

	if( tts_c.ring )
	{
		if(f->bit.stop_over)
		{
			rpt_len += _memcpy_len(&rpt_buf[rpt_len], "�����ѳ�ʱͣ��", 14);
		}
		if( (f->bit.fatigue)&&(mix.moving) )
		{
			if( ( gps_base.speed > 7 ) || ( mix.complex_speed01 > 4 ) ) 
				rpt_len += _memcpy_len(&rpt_buf[rpt_len], "����ƣ�ͼ�ʻ����ע����Ϣ��", 26);
			else
				return;
		}
		else if((f->bit.fatigue_near)&&(mix.moving))
		{
			rpt_len += _memcpy_len(&rpt_buf[rpt_len], "������ƣ�ͼ�ʻ����ͣ����Ϣ�������ʻԱ��", 40);
		}

		delay_ring = tick;
		//�˴��������·�ߵȱ���
		
		write_tts(rpt_buf, rpt_len);
	}
}


void tts_accopen_voice( void )
{
	
}




void tts_ICCard_voice( void )
{
	u8 rpt_buf[50];

	
	_memset( rpt_buf, 0, 50 );
	_memcpy( rpt_buf, "�����ɹ����������ˣ�ף��һ·ƽ��!", 33);

	
	write_tts(rpt_buf, 33);
}

/*********************************************************
��    �ƣ�tts_alarm_task
��    �ܣ�TTS ��������
�����������
��    ������
��д���ڣ�2013-12-01
**********************************************************/
void tts_alarm_task(void)
{
	static VEHICLE_ALARM_UNION bak_tts_alm; //bakeuP_SOUND_alarm
	static VEHICLE_ALARM_UNION idle_tts_alm; //idle_tts_alarm
	static bool new_tts1 = false;
	static bool new_tts2 = false;
	static bool init_para = true; //initialize_parameter	
	static u32 delay_t1 = 15; 		//delay_tick
	static u32 delay_t2 = 15; 		//delay_tick
	u32 tmp_reg = 0;
	static u16  tmr1 = 20;
	static u16  tmr2 = 20;


	
	if(init_para)
	{
		init_para = false;
		bak_tts_alm.reg = 0;
		idle_tts_alm.reg = 0;
		tts_c.tick = tick;
	}

	tmp_reg = car_alarm.reg&TTS_ALM_MSK;		
	if(bak_tts_alm.reg != tmp_reg)
	{
		bak_tts_alm.reg = car_alarm.reg&TTS_ALM_MSK;

		if(bak_tts_alm.reg!=idle_tts_alm.reg)
		{
			
			if( tts_c.open == false )
			{
				if(( (bak_tts_alm.reg&0x00000004) == 0x00000004 )||( (bak_tts_alm.reg&0x00004000) == 0x00004000 )||( (bak_tts_alm.reg&0x00080000) == 0x00080000 ))				//0x0008000e
				{
					tts_c.cnt = 3;
					tts_c.open = true;
					tts_c.tick = tick;
					tts_c.ring = true;
					new_tts2 = true;
				}		
			#if 1
				if( (bak_tts_alm.reg&0x00008000) != 0x00008000 )	//IC��δ���벻����
					new_tts1 = true;
			#endif
			}
			else
				new_tts1 = true;	
		}
		
		if(( bak_tts_alm.reg == 0x00000004 )||( bak_tts_alm.reg == 0x00004000 )||( bak_tts_alm.reg == 0x00008000 ))			
			tmr2 = 300;
		else if( bak_tts_alm.reg == 0x00080000 )   //ͣ������
			tmr2 = 300;
		else 
			tmr1 = 30;
	}

	tmp_reg = bak_tts_alm.reg&TTS_ALM_MSK;
	if(tmp_reg)
	{
		if( new_tts1 || _covern(delay_t1)>=tmr1  )
		{
			new_tts1 = false;
			delay_t1 = tick;		
			//��ȡ���в���λ�����ڿ�����Ӧ
			idle_tts_alm.reg = bak_tts_alm.reg;  //(~bak_tts_alm.reg)&TTS_ALM_MSK;
			load_tts_rpt_alarm1(&bak_tts_alm);
		}	
		if( new_tts2 || _covern(delay_t2)>=tmr2  )
		{
			new_tts2 = false;
			delay_t2 = tick;		
			//��ȡ���в���λ�����ڿ�����Ӧ
			idle_tts_alm.reg = bak_tts_alm.reg;  //(~bak_tts_alm.reg)&TTS_ALM_MSK;
			load_tts_rpt_alarm2(&bak_tts_alm);
			if( tts_c.ring )
			{
				tts_c.cnt = 2;
			}
		}	
		else if( tts_c.ring && tts_c.cnt > 0 )
		{
			if(_covern(delay_ring)>=15)
			{
				load_tts_rpt_alarm2(&bak_tts_alm);
				if( tts_c.cnt > 0 )
					tts_c.cnt--;				
			}
		}
	}
	else
	{
		new_tts1 = false;
		new_tts2 = false;
		delay_t1 = tick;
		delay_t2 = tick;
		idle_tts_alm.reg = 0;
		_memset( (u8*)&tts_c, 0, sizeof(tts_c) );
	}
	if( tts_c.ring )
	{
		if( _covern(tts_c.tick)>=1800 )
		{
			tts_c.ring = false;

		}
	}	
	
}






