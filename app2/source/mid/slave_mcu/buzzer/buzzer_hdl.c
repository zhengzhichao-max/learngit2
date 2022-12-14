#include "include_all.h"

const u16  voice_freq_table[4][7] =
{
	{262,  294,  330,  349,  392,  440,  494},
	{523,  587,  659,  698,  784,  880,  988},
	{1046, 1175, 1318, 1397, 1568, 1760, 1967},
	{2092, 2350, 2636, 2794, 3136, 3520, 3934}
};

const voice_source_struct  sound1[] =
{
	{0x35, 0x02},

	{0x00, 0x00}
};

const voice_source_struct  sound2[] =
{
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},

	{0x00, 0x00}
};

const voice_source_struct  sound3[] =
{
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},

	{0x00, 0x00}
};

const voice_source_struct  sound4[] =
{
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},
	{0x33, 0x02},
	{0x31, 0x02},

	{0x00, 0x00}
};

const voice_source_struct  sound5[] =
{
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},
	{0x31, 0x03},
	{0x27, 0x03},

	{0x00, 0x00}
};

const voice_source_struct  sound6[] =
{
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},
	{0x27, 0x02},
	{0x25, 0x02},

	{0x00, 0x00}
};



static bool buzzer_load(const voice_source_struct *s_ptr)
{
	bool  ret;
	u16  temp;

	ret = true;
	if ((s_ptr->voice) == buzzer_end_flag)
	{
		buzzer_ctrl(false);
		buz_voice.end_flag = true;  //播放结束

		ret =  false;
	}
	else
	{
		if ((s_ptr->voice) == buzzer_mute_flag)
		{
			temp = 1;  //静音是播放1Hz频率的声音
		}
		else
		{
			if (((s_ptr->voice) >= 0x31) && ((s_ptr->voice) <= 0x37))
				temp = voice_freq_table[3][(s_ptr->voice)-0x31];
			else if (((s_ptr->voice) >= 0x21) && ((s_ptr->voice) <= 0x27))
				temp = voice_freq_table[2][(s_ptr->voice)-0x21];
			else if (((s_ptr->voice) >= 0x11) && ((s_ptr->voice) <= 0x17))
				temp = voice_freq_table[1][(s_ptr->voice)-0x11];
			else if (((s_ptr->voice) >= 0x01) && ((s_ptr->voice) <= 0x07))
				temp = voice_freq_table[0][(s_ptr->voice)-0x01];
			else
				temp = voice_freq_table[3][6];  //超过范围
		}

		tim3_config(temp);
		buz_voice.cnt = (s_ptr->tim);
	}

	return  ret;
}




void buzzer_init(buzzer_voice_type_eunm type)
{
	switch (type)
	{
	case s_short:
		buz_voice.ptr = sound2;
		break;
	case s_long:
		buz_voice.ptr = sound3;
		break;
	case s_sound_1:
		buz_voice.ptr = sound4;
		break;
	case s_sound_2:
		buz_voice.ptr = sound5;
		break;
	case s_sound_3:
		buz_voice.ptr = sound6;
		break;

    case s_sound_cus:  //添加自定义数据
		buz_voice.ptr = buz_cus;
		break;
        
	default:
		buz_voice.ptr = sound1;
	}

	if (buzzer_load(buz_voice.ptr))
	{
		buz_voice.end_flag = false;

		buz_voice.cnt --;
		buz_voice.ptr ++;

		buzzer_ctrl(true);
	}
}




void buzzer_int_hdl(void)
{
	if (buz_voice.cnt == 0x00)
	{
		if (buzzer_load(buz_voice.ptr))
		{
			buz_voice.ptr ++;
		}
	}
	else
	{
		buz_voice.cnt --;
	}
}




void buzzer_cus_hdl(void)
{
    if(buz_cus_time.flag == _false_)
    {
    	return;
    }

	if(sub_u32(jiffies, buz_cus_time.all_cnt) < buz_cus_time.all_lgth)
    {
    	if(sub_u32(jiffies, buz_cus_time.cycle_cnt) < buz_cus_time.cycle_lgth)
    	{
    		if(buz_cus_time.sta == 0x00)
    		{
    			buz_cus_time.sta = 0x01;

				buzzer_init(s_sound_cus);
    		}
    	}
		else
		{
			buz_cus_time.cycle_cnt = jiffies;  //保持时间同步
		    buz_cus_time.sta = 0x00;  //准备下周期鸣叫
		}
    }
	else
	{
		buz_cus_time.flag = false;
	}
}






