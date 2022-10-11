#include "include_all.h"

//*****************************************************//
//*************     记录仪相关初始化      *************//
//
//*****************************************************//
void recorder_init(void) {
    _memset(((u8 *)(&ic_card)), 0x00, (sizeof(ic_card)));
}

void load_print_data(u8 *src, u16 lgth) {
    if (print_auto_data.req_flag){
        return;
    }

    logd("flag of print busy num:%d", lgth);

    if (lgth > print_auto_buff_lgth_max) {
        lgth = print_auto_buff_lgth_max;
    }
    print_auto_data.lgth = lgth;
    print_auto_data.cnt = 0x00;
    memcpy(print_auto_data.buff, src, print_auto_data.lgth);

    print_auto_data.req_flag = _true_;
    print_start_ctrl(_true_);
}

void print_auto_process(void) {
    uint n = 0;
//    static uint tmr = 0;
    if (!print_auto_data.req_flag)
        return;

    if (!print_read_state()) {
        if (print_auto_data.lgth) {
            print_one_line(0, print_auto_data.lgth, (print_auto_data.buff + print_auto_data.cnt), (&n)); //

            print_auto_data.cnt += (print_auto_data.lgth - n);
            print_auto_data.lgth = n;

            logd("print left num:%d", n);
        } else {
            print_start_ctrl(_false_); //1??1?7?1?7??1?7?4?1?1?7?1?7?
            print_driver_init(print_speed_set);
            print_auto_data.lgth = 0;
            print_auto_data.req_flag = _false_;
        }
    }

#if 0
    if (!print_read_state())
    {
    	if (print_auto_data.lgth)
    	{
    		print_one_line(0, print_auto_data.lgth, (print_auto_data.buff+print_auto_data.cnt), (&n));	//′òó?
    
    		print_auto_data.cnt += (print_auto_data.lgth-n);
    		print_auto_data.lgth = n;
			//#if 1
			//if( _pasti(tmr)>500)
			{
				lcd_init(_false_, _false_); /* ??????????? */
				tmr = jiffies;
			}
			//#endif
			tmr = jiffies;
    	}
    	else
    	{
    		if( _pasti(tmr)<1000)
    		{
	    		print_start_ctrl(_false_);		//1?±?′òó?
	    		print_driver_init(print_speed_set);
	    		
	    		print_auto_data.lgth = 0x00;
	    		print_auto_data.req_flag = _false_;
				
				lcd_init(_false_, _false_); /* ??????????? */				
			}
    	}
    }
#endif
}
