#include "include_all.h"

static key_proc_struct key_proc;
static key_struct key;

/*****************************************************************************
**    按键扫描初始化
*****************************************************************************/
void key_state_scan_init(void) {
    _memset(((u8 *)(&key_proc)), (0x00), (sizeof(key_proc_struct)));
    _memset(((u8 *)(&key)), (0x00), (sizeof(key_struct)));
}

/*****************************************************************************
**    按键扫描
**
**  该函数仅支持单个按键检测，不支持同时按下两个或两个以上的按键检测
*****************************************************************************/
key_struct key_scan(void) {
    if (_pastc(key_proc.t) < key_filt_time) {
        return key;
    }

    key_proc.t = jiffies; //赋值

    switch (key_proc.step) {
    case 0x00:
        if (key_read() != key_free_data) {
            key_proc.last = key_read();
            key_proc.step = 1;
        }
        break;

    case 0x01:
        if (key_read() == key_proc.last) {
            key_proc.step = 2;
            key_proc.long_t = jiffies; //长按计时器时间同步
        } else {
            key_proc.step = 0x00;
        }
        break;

    case 0x02:
        if (key_read() == key_proc.last) {
            if (key_proc.press == false) {
                if (_pasti(key_proc.long_t) >= key_long_time) {
                    key.active = true;
                    key.lg = true;
                    key.dat = key_proc.last; //记录键值
                    key_proc.press = true;
                }
            }
        } else if (key_read() == key_free_data) {
            key_proc.step = 3;
        }
        break;

    case 0x03:
        if (key_read() == key_free_data) {
            if (key_proc.press == false) //长按状态没确定，则判定为短按
            {
                key.active = true;
                key.lg = false;
                key.dat = key_proc.last; //记录键值
            }

            key_proc.step = 0;
            key_proc.press = false; //按键扫描过程结束
        } else {
            key_proc.step = 2;
        }
        break;

    default:
        key_proc.step = 0x00;
    }

    //////////////////////////////
    key_data.active = key.active;
    key_data.lg = key.lg;
    key_data.key = key.dat;

    key.active = false;
    //////////////////////////////

    return key;
}

/*****************************************************************************
**    状态扫描
**
**  该函数仅支持单一量状态检测
**
**参数sta
**参数curr表示当前的状态
**参数tim_max为状态稳定最长时间值
**返回最近一次有效状态
*****************************************************************************/
u8 state_scan(state_struct *sta, u8 curr, u16 tim_max) {
    if (_pastc(sta->filt_tim) >= state_filt_time) {
        (sta->filt_tim) = jiffies;

        if (curr == (sta->last)) //状态一致，同步计时器
        {
            (sta->tim) = jiffies;
        } else {
            if (_pasti(sta->tim) >= tim_max) {
                (sta->last) = curr;
                (sta->tim) = jiffies;
            }
        }
    }

    return (sta->last);
}
