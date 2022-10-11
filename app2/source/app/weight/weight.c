#include "include_all.h"

#if (0)
u16 weight_filter[10];

//æ ‡å®šåŠè?æ•°è‡ªåŠ¨æŽ§åˆ?
//è‡?Š¨1.5ç§’è?ä¸€æ¬¡æ•°æ?
//$0.GD#  $0.S0#  $0.S1#  $0.S2#
void auto_cali_weight_task(void) {
    static u32 tmr_c = 0;
    static bool cut_ext_pwr = false;

    if (u4_m.cur.b.weight == false)
        return;

    if (_pastn(auto_weight.tmr) > 1500) {
        auto_weight.tmr = jiffies;
        write_uart4("$0.GD#", 6);

        //if( auto_weight.read )
        car_weight = auto_weight.value;
        //else
        //	car_weight = GravityValue;
    }

    if (auto_weight.cali_empty) //ç©ºè½¦æ ‡å®š, é»˜è?å‡ºåŽ‚éƒ½æ˜¯æœ€å¤§æ”¾å¤§å€æ•°
    {
        if (_pastn(tmr_c) > 2000) //value
        {
            tmr_c = jiffies;
            if (auto_weight.value > 1000)
                write_uart4("$0.S1#", 6);
            else if (auto_weight.value > 2000)
                write_uart4("$0.S0#", 6);
        }
    }

    //å¤–æŽ¥è®¾å?å¼‚å¸¸ï¼›é‡æ–°ä¸Šç”?
    if (_covern(auto_weight.exisTmr) >= 60) {
        auto_weight.exisTmr = tick;
        beep_off();
        cut_ext_pwr = true;
        auto_weight.read = false;
    }

    if (cut_ext_pwr) {
        if (_covern(auto_weight.exisTmr) >= 3) {
            cut_ext_pwr = false;
            beep_on();
        }
    }
}

/****************************************************************************
* Ãû³Æ:    get_uart_is_camera ()
* ¹¦ÄÜ£ºÅÐ¶Ïµ±Ç°´®¿ÚÊÇ·ñÊÇÉãÏñÍ·¹¦ÄÜ
* Èë¿Ú²ÎÊý£ºÎÞ                         
* ³ö¿Ú²ÎÊý£ºÎÞ
****************************************************************************/
static bool get_uart_is_weight(u8 from_t) {
    if (u4_m.cur.b.weight == true && from_t == FROM_U4) {
        return true;
    } else {
        return false;
    }
}

/****************************************************************************
* Ãû³Æ:    camera_parse_proc ()
* ¹¦ÄÜ£º½ÓÊÕÉãÏñÍ·Êý¾Ý
* Èë¿Ú²ÎÊý£ºÎÞ                         
* ³ö¿Ú²ÎÊý£ºÎÞ
****************************************************************************/
void weight_parse_proc(void) {
    typedef enum {
        E_WEIGHT_IDLE,
        E_WEIGHT_RECV,
        E_WEIGHT_EXIT
    } E_WEIGHT_STEP;

    static E_WEIGHT_STEP step = E_WEIGHT_IDLE;
    bool ret;
    u8 buf[30];
    u8 r_buf[WEIGHT_BUF_LEN];
    u16 r_len = 0;

    static u8 from_id = 0;
    static UART_MANAGE_STRUCT present;

    switch (step) {
    case E_WEIGHT_IDLE:
        from_id = get_uart4_news(&present);
        ret = get_uart_is_weight(from_id);
        if (ret)
            step = E_WEIGHT_RECV;
        break;
    case E_WEIGHT_RECV:
        _memset(r_buf, 0x00, WEIGHT_BUF_LEN);
        r_len = copy_uart_news(from_id, r_buf, present.cur_p, present.c_len);
        if (r_len < 5 || r_len > WEIGHT_BUF_LEN) {
            step = E_WEIGHT_EXIT;
            break;
        }

        if (_strncmp(r_buf, "$0.+D", 5) == 0) {
            _memset(buf, 0, 20);
            _memcpy_len(buf, r_buf + 5, 4);
            auto_weight.value = atoi((const char *)buf);
            auto_weight.read = true;
            auto_weight.zf = false;
        } else if (_strncmp(r_buf, "$0.-D", 5) == 0) {
            _memset(buf, 0, 20);
            _memcpy_len(buf, r_buf + 5, 4);
            r_len = atoi((const char *)buf);
            auto_weight.value = r_len;
            auto_weight.read = true;
            auto_weight.zf = true;
        }

        step = E_WEIGHT_EXIT;
        break;
    case E_WEIGHT_EXIT:
        set_uart_ack(from_id);
        step = E_WEIGHT_IDLE;
        break;
    default:
        step = E_WEIGHT_IDLE;
        break;
    }
}
#endif
