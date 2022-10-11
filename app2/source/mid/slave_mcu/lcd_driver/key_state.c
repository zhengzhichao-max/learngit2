#include "include_all.h"

static key_proc_struct key_proc;
static key_struct key;

/*****************************************************************************
**    ����ɨ���ʼ��
*****************************************************************************/
void key_state_scan_init(void) {
    _memset(((u8 *)(&key_proc)), (0x00), (sizeof(key_proc_struct)));
    _memset(((u8 *)(&key)), (0x00), (sizeof(key_struct)));
}

/*****************************************************************************
**    ����ɨ��
**
**  �ú�����֧�ֵ���������⣬��֧��ͬʱ�����������������ϵİ������
*****************************************************************************/
key_struct key_scan(void) {
    if (_pastc(key_proc.t) < key_filt_time) {
        return key;
    }

    key_proc.t = jiffies; //��ֵ

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
            key_proc.long_t = jiffies; //������ʱ��ʱ��ͬ��
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
                    key.dat = key_proc.last; //��¼��ֵ
                    key_proc.press = true;
                }
            }
        } else if (key_read() == key_free_data) {
            key_proc.step = 3;
        }
        break;

    case 0x03:
        if (key_read() == key_free_data) {
            if (key_proc.press == false) //����״̬ûȷ�������ж�Ϊ�̰�
            {
                key.active = true;
                key.lg = false;
                key.dat = key_proc.last; //��¼��ֵ
            }

            key_proc.step = 0;
            key_proc.press = false; //����ɨ����̽���
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
**    ״̬ɨ��
**
**  �ú�����֧�ֵ�һ��״̬���
**
**����sta
**����curr��ʾ��ǰ��״̬
**����tim_maxΪ״̬�ȶ��ʱ��ֵ
**�������һ����Ч״̬
*****************************************************************************/
u8 state_scan(state_struct *sta, u8 curr, u16 tim_max) {
    if (_pastc(sta->filt_tim) >= state_filt_time) {
        (sta->filt_tim) = jiffies;

        if (curr == (sta->last)) //״̬һ�£�ͬ����ʱ��
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
