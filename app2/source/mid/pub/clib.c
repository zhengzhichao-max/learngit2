/**
  ******************************************************************************
  * @file    clib.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  �⺯��
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#include "include_all.h"

/****************************************************************************
* ����:    _sprintf_len ()
* ���ܣ��ַ�����ӡ
* ��ڲ�������                         
* ���ڲ��������ش�ӡ���ַ�������
****************************************************************************/
int _sprintf_len(char *buf, const char *fmt, ...) {
    int n;
    __va_list args;

    va_start(args, fmt);
    n = vsprintf(buf, fmt, args);
    va_end(args);

    return n;
}

/****************************************************************************
* ����:    _printf2 ()
* ���ܣ�������Ϣ��ӡ
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
//���Թ���
int _printf2(const char *fmt, ...) {
    int n = 0;
    char buf[500];
    va_list args;
    //if ( u1_m.cur.b.debug == true )
    {
        va_start(args, fmt);
        n = vsprintf(buf, fmt, args);

        if (n > 0 && n < 500) {
            write_uart1((u8 *)&buf, n);
        }
        va_end(args);
    }
    return n;
}

//͸������
int _printf_tr9(const char *fmt, ...) {
    int n = 0;
    char buf[3000];
    va_list args;
    if (!u1_m.cur.b.tt) {
        va_start(args, fmt);
        n = vsprintf(buf, fmt, args);

        if (n > 0 && n < 3000) {
            write_uart1((u8 *)&buf, n);
        }
        va_end(args);
    }
    return n;
}

int _printf_tmp(const char *fmt, ...) {
#if DEBUG_CAN

    int n = 0;

    char buf[500];
    va_list args;
    if (u1_m.cur.b.debug == true) {
        va_start(args, fmt);
        n = vsprintf(buf, fmt, args);

        if (n > 0 && n < 500) {
            write_uart4((u8 *)&buf, n);
            nop();
        }
    }

    return n;

#endif
    return 0;
}

static uc8 ascTable[17] = {"0123456789ABCDEF"};

static uc32 CCcd[6] = {100000, 10000, 1000, 100, 10, 1};
/****************************************************************************
* ���ƣ�AsctoBCD
* ���ܣ����ݴ�ӡ��ASCII
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u8 DriverCodeBcd(u32 const *pStr, u8 *bBcd, u8 len, u8 ifz) {
    u8 tmp = 0;
    u8 i = 0;
    u32 vk = *pStr;

    if (len > 6)
        return 0;

    while (len) {
        tmp = ascTable[vk / CCcd[6 - len]];
        if (ifz == 1) //��Ҫȥ��ǰ��0
        {
            if (tmp != 0x30)
                ifz = 0;
        }
        if (ifz == 0) {
            *bBcd++ = tmp;
            i++;
        }
        vk %= CCcd[6 - len];
        len--;
    }
    return i;
}

/****************************************************************************
* ����:    _strlen ()
* ���ܣ���ȡ�ַ�������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u16 _strlen(u8 *str) {
    u16 n;

    for (n = 0; *str++ != '\0'; n++)
        ;
    return n;
}

/****************************************************************************
* ����:    _strcpy ()
* ���ܣ��ַ�������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 *_strcpy(u8 *str1, uc8 *str2) {
    u8 *ret = str1;

    while ((*str1++ = *str2++) != '\0')
        ;
    return ret;
}

/****************************************************************************
* ����:    _strcpy ()
* ���ܣ��ַ�����ȡ������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 *_strcat(u8 *str1, uc8 *str2) {
    u8 *ret = str1;

    while (*str1 != '\0') str1++;
    while ((*str1++ = *str2++) != '\0')
        ;
    return ret;
}

/****************************************************************************
* ����:    _strncat ()
* ���ܣ��ַ�����ȡ������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 *_strncat(u8 *str1, uc8 *str2, u8 n) {
    u8 *ret = str1;

    while (*str1 != '\0') str1++;
    while (n && ((*str1++ = *str2++) != '\0')) n--;
    if (n == 0) *str1 = '\0';
    return ret;
}

/****************************************************************************
* ����:    _strncpy ()
* ���ܣ��ַ�����
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 *_strncpy(u8 *dst, uc8 *sur, u8 n) {
    *(dst) = '\0';
    return _strncat(dst, sur, n);
}

/****************************************************************************
* ����:    _strncpy ()
* ���ܣ��ַ����Ա�
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
s16 _strcmp(u8 *str1, u8 *str2) {
    while (*str1 == *str2) {
        if ((*str1 == '\0') || (*str2 == '\0')) {
            break;
        }
        str1++;
        str2++;
    }
    if (*str1 == '\0')
        return 1;
    if (*str1 != *str2) //�ַ�������ȫ��ͬ
        return 0;
    return 1;
}

char _tolower(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return (ch + ('a' - 'A'));
    }

    return ch;
}

/****************************************************************************
* ����:    _strncpy ()
* ���ܣ������ȣ��ַ��Ա�
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
s16 _strncmp(u8 *str1, u8 *str2, u8 n) {
    while (n && *str1 == *str2) {
        if (*str1 == '\0')
            return 0;
        n--;
        str1++;
        str2++;
    }
    return n ? (u8)*str1 - (u8)*str2 : 0;
}

//���ܣ��Ƚ��ַ���s1��s2������������ĸ�Ĵ�Сд��
int stricmp(const char *str1, const char *str2) {
    int i, ret;

    for (i = 0; *str1 != '\0'; i++) {
        ret = _tolower(*str1++) - _tolower(*str2++);

        if (ret) {
            return ret;
        }
    }

    ret = _tolower(*str1) - _tolower(*str2);

    if (ret) {
        return ret;
    }

    return 0;
}

//�� �ܣ� �Ƚ��ַ���str1��str2��ǰn���ַ����ֵ���Ĵ�С�����ǲ�������ĸ��Сд��
int _strnicmp(const char *str1, const char *str2, int size) {
    int i, ret;

    for (i = 0; *str1 != '\0' && i < size; i++) {
        ret = _tolower(*str1++) - _tolower(*str2++);

        if (ret) {
            return ret;
        }
    }

    if (i != size) {
        return -1;
    }

    return 0;
}

/****************************************************************************
* ����:    _strncpy ()
* ���ܣ��ַ�������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 *_strstr(u8 *str1, u8 *str2) {
    u16 i;

    if (*str2 == '\0') return str1;
    while (*str1) {
        for (i = 0; *(str1 + i) == *(str2 + i); i++)
            if (*(str2 + i + 1) == '\0') return str1;
        str1++;
    }
    return NULL;
}

/****************************************************************************
* ����:    _strspn ()
* ���ܣ��ַ�������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
s16 _strspn(uc8 *str1, uc8 *str2) {
    u16 n = 0;
    u8 *pt;

    while (*str1) {
        pt = (u8 *)str2;
        while (*pt)
            if (*pt++ == *str1) {
                n++;
                goto cmp_ok;
            }
        break;
    cmp_ok:
        str1++;
    }
    return n;
}

/****************************************************************************
* ����:    _strcspn ()
* ���ܣ��ַ�������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
s16 _strcspn(uc8 *str1, uc8 *str2) {
    u16 n = 0;
    u8 *pt;

    while (*str1) {
        pt = (u8 *)str2;
        while (*pt) {
            if (*pt++ == *str1)
                return n;
        }
        str1++;
        n++;
    }
    return n;
}

/****************************************************************************
* ����:    _memcpy ()
* ���ܣ����ݿ���
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
void _memcpy(u8 *cp1, uc8 *cp2, u16 n) {
    if (n == 0)
        return;

    while (n--)
        *cp1++ = *cp2++;
}

/****************************************************************************
* ����:    _memcpy ()
* ���ܣ������ȣ����ݿ���
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u16 _memcpy_len(u8 *cp1, u8 *cp2, u16 n) {
    u16 ret_len = 0;

    if (n == 0)
        return 0;

    ret_len = n;
    while (n--)
        *cp1++ = *cp2++;

    return ret_len;
}

/****************************************************************************
* ����:    _strtok ()
* ���ܣ��ַ�������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 *_strtok(u8 *str1, uc8 *str2) {
    u8 *begin;
    u8 *end;
    static u8 *strbak;

    begin = str1 ? str1 : strbak;
    if (*(begin += _strspn(begin, str2)) == '\0') {
        strbak = ""; // "" '\0'
        return NULL;
    }
    end = begin + _strcspn(begin, str2);
    if (*end != '\0')
        *end++ = '\0';
    strbak = end;
    return begin;
}

/****************************************************************************
* ����:    _strtok ()
* ���ܣ����С������������ַ������������Ȱ�����������������
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 _strcpy_len(u8 *str1, u8 *str2) {
    u8 i = 0;

    while (*str2 != '\0') {
        *str1++ = *str2++;
        i++;
    }
    *str1 = '\0';
    i++;
    return i;
}

/****************************************************************************
* ����:    _memset ()
* ���ܣ�ָ���������򣬹̶���ֵд
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
void _memset(u8 *str1, u8 c, u16 len) {
    if (str1 == 0)
        return;

    while (len--) {
        *str1++ = c;
    }
}

/****************************************************************************
* ����:    _htoa ()
* ���ܣ�HEX��ʽת��ΪASCII��ʽ
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
void _htoa(u8 *str, u8 hex) {
    u8 v;

    v = hex >> 4;
    str[0] = (v <= 9) ? 0x30 + v : 0x41 + v - 0x0A;
    v = hex & 0x0f;
    str[1] = (v <= 9) ? 0x30 + v : 0x41 + v - 0x0A;
    str[2] = '\0';
}

/****************************************************************************
* ����:    _bcdtobin ()
* ���ܣ�BCD�뵽�����Ƶ�ת����
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 _bcdtobin(u8 bByte) {
    u8 ucRn = 0;

    ucRn = bByte >> 4;
    ucRn *= 10;
    ucRn += bByte & 0xF;

    return ucRn;
}

u8 bintohex(u8 bin) {
    u8 ucRn = 0;

    ucRn = (bin / 10) << 4;
    //ucRn = ucRn*16;
    ucRn += bin % 10;

    return ucRn;
}

/****************************************************************************
* ����:    _bcdtobin ()
* ���ܣ������ȣ�BCD�뵽�����Ƶ�ת����
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 _nbcdtobin(u8 *p, u8 n_t) {
    u8 ucRn = 0;
    u8 n;

    n = n_t;
    while (n) {
        ucRn = *p >> 4;
        ucRn *= 10;
        ucRn += *p & 0x0f;
        *p++ = ucRn;
        n--;
    }

    return n_t;
}

/****************************************************************************
* ����:    _bcdtobin ()
* ���ܣ������Ƶ�BCD��ת��
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 _bintobcd(u8 bByte) {
    u8 ucRn = 0;

    ucRn = (bByte / 10) << 4;
    ucRn += bByte % 10;

    return ucRn;
}

/****************************************************************************
* ����:    _nbintobcd ()
* ���ܣ������ȣ������Ƶ�BCD��ת��
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/
u8 _nbintobcd(u8 *p, u8 n_t) {
    u8 ucRn = 0;
    u8 n;

    n = n_t;
    while (n) {
        ucRn = (*p / 10) << 4;
        ucRn += *p % 10;
        *p++ = ucRn;
        n--;
    }

    return n_t;
}

/****************************************************************************
* ����:    _HexToBCD ()
* ���ܣ�ʮ������תBCD��
* ��ڲ�������                         
* ���ڲ�������      
****************************************************************************/

u16 _HexToBCD(u16 bcd) {
    u8 u1, u2;

    u2 = bcd / 100;
    u1 = bcd % 100;

    return (u2 + (u2 / 10) * 6) << 8 | (u1 + (u1 / 10) * 6);
};

/****************************************************************************
* ���ƣ�_del_filt_char
* ���ܣ��ַ�ת��
* ��ڲ��������ݳ��ȣ���ת������
* ���ڲ�����ת��������ʵ�ʳ���
****************************************************************************/
u16 _del_filt_char(u16 len, u8 *chr) {
    u16 i, j, k;
    u8 *move;

    j = 0;
    for (i = 0; i < len; i++) {
        if (*chr++ == 0x7d) {
            j++;
            if (*chr == 0x02) *(chr - 1) = 0x7e;

            move = chr;
            for (k = 0; k < (len - i - 2); k++) {
                *move = *(move + 1);
                move++;
            }
            *move = '\0';
        }
    }
    return (j);
}

/****************************************************************************
* ���ƣ�_add_filt_char
* ���ܣ��ַ�ת��
* ��ڲ��������ݳ��ȣ���ת������
* ���ڲ�����ת��������ʵ�ʳ���
****************************************************************************/
u16 _add_filt_char(u32 len, u8 *chr) {
    u16 i, j, k;
    u8 *move;

    j = 0;
    for (i = 0; i < len; i++) {
        if (*chr == 0x7e || *chr == 0x7d) {
            j++;
            move = chr + (len - i);
            *(move + 1) = '\0';
            for (k = 0; k < (len - i); k++) {
                *move = *(move - 1);
                move--;
            }
            move++;
            if (*chr == 0x7d)
                *move = 0x01;
            else
                *move = 0x02;

            *chr = 0x7d;
            chr++;
        }
        chr++;
    }
    return (j);
}

/****************************************************************************
* ���ƣ�_sw_endian
* ���ܣ���С��ģʽת��	
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u8 _sw_endian(u8 *cp1, u8 *cp2, u8 n) {
    u8 ret_len = 0;

    ret_len = n;

    if ((cp1 == NULL) || (cp2 == NULL))
        return ret_len;
    cp2 += n - 1;
    while (n--) {
        *cp1 = *cp2;
        cp1++;
        cp2--;
    }
    return ret_len;
}

//����ָ���ַ�֮ǰ���ַ���
u16 _memcpyCH(u8 *dst, u8 ch, u8 *buf, u16 len) {
    u16 tl = 0;

    while (tl < len) {
        if (*buf == 0)
            break;
        if (*buf == ch) {
            tl++;
            break;
        }
        *dst++ = *buf++;
        tl++;
    }
    return tl;
}

//ret:0,success;other,fail//len_max,�����жϳ��ȷ�Χ��
int str_visible(const char *str, int len_max) {
    int len = strlen(str);

    if (len < 1 || len > len_max) {
        return -1;
    }

    for (; *str != '\0'; str++) {
        if (*str >= 0x20 && *str <= 0x7E) {
            continue;
        }

        return -1;
    }

    return 0;
}

/***************************************************************************
* ���ƣ�_str2tobcd
* ���ܣ�2���ַ�ת����BCD ��ʽ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u16 _str2tobcd(u8 *bByte, u8 *str) {
    *bByte = (*str++ - 0x30) << 4;
    *bByte |= (*str++ - 0x30) & 0x0f;

    return 0;
}

/****************************************************************************
* ���ƣ�_chartohex
* ���ܣ����ֽڵ��ַ�ת����HEX��ʽ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u8 _chartohex(u8 uChar_H, u8 uChar_L) {
    if ((uChar_H >= '0') && (uChar_H <= '9')) {
        uChar_H -= 0x30;
    } else {
        if ((uChar_H >= 'A') && (uChar_H <= 'F')) {
            uChar_H -= 0x37;
        } else {
            if ((uChar_H >= 'a') && (uChar_H <= 'f'))
                uChar_H -= 0x57;
            else
                return (0);
        }
    }

    if ((uChar_L >= '0') && (uChar_L <= '9')) {
        uChar_L -= 0x30;
    } else {
        if ((uChar_L >= 'A') && (uChar_L <= 'F')) {
            uChar_L -= 0x37;
        } else {
            if ((uChar_L >= 'a') && (uChar_L <= 'f'))
                uChar_L -= 0x57;
            else
                return (0);
        }
    }

    return (16 * uChar_H + uChar_L);
}

static uc16 BBcd[5] = {10000, 1000, 100, 10, 1};

/****************************************************************************
* ���ƣ�AsctoBCD
* ���ܣ����ݴ�ӡ��ASCII
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u8 AsctoBCD(u16 const *pStr, u8 *bBcd, u8 len, u8 ifz) {
    u8 tmp = 0;
    u8 i = 0;
    u16 vk = *pStr;

    if (len > 5)
        return 0;

    while (len) {
        tmp = ascTable[vk / BBcd[5 - len]];
        if (ifz == 1) //��Ҫȥ��ǰ��0
        {
            if (tmp != 0x30)
                ifz = 0;
        }
        if (ifz == 0) {
            *bBcd++ = tmp;
            i++;
        }
        vk %= BBcd[5 - len];
        len--;
    }
    return i;
}

/****************************************************************************
* ���ƣ�_get_sum
* ���ܣ������ۼӺ�
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u8 _get_sum(u8 *p, u16 len) {
    u32 sum;
    u8 sum_b;
    u16 i;
    u8 *pt;

    if (len > 550)
        return 0;

    pt = p;
    sum = 0;
    for (i = 0; i < len; i++)
        sum += *pt++;

    sum_b = (u8)sum;
    return sum_b;
}

/****************************************************************************
* ���ƣ�_verify_time
* ���ܣ�ʱ��У��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
bool _verify_time(time_t *t) {
    if (t->year < 13 || t->year > 50)
        return false;

    if (t->month == 0 || t->month > 12)
        return false;

    if (t->date == 0 || t->date > 31)
        return false;

    if (t->hour > 23)
        return false;

    if (t->min > 59)
        return false;

    if (t->sec > 59)
        return false;

    return true;
}

/****************************************************************************
* ���ƣ�_timetotick
* ���ܣ�ʱ����ת��Ϊ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
s32 _timetotick(u8 *timep) {
    u8 buf[3];
    u16 hour;
    u16 minute;
    u16 second;

    _strncpy(buf, timep, 2);
    buf[2] = '\0';
    hour = atoi((const char *)&buf);
    if (hour >= 24)
        return -1;

    _strncpy(buf, timep + 2, 2);
    buf[2] = '\0';
    minute = atoi((const char *)&buf);
    if (minute >= 60)
        return -1;

    _strncpy(buf, timep + 4, 2);
    buf[2] = '\0';
    second = atoi((const char *)&buf);
    if (second >= 60)
        return -1;

    return ((s32)3600 * hour + 60 * minute + second);
}

/****************************************************************************
* ���ƣ�_cal_distance
* ���ܣ����Ƽ��������ľ���
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
float _cal_distance(s32 lati_1, s32 lati_2, s32 lngi_1, s32 lngi_2) {
    float x, y;

    x = 60 * 1851.5 * labs(lati_1 - lati_2);
    x /= (float)1000000;

    y = 60 * 1422.2 * labs(lngi_1 - lngi_2);
    y /= (float)1000000;

    return (x * x + y * y);
}

/****************************************************************************
* ���ƣ�_jtb_to_gb
* ���ܣ���γ�ȸ�ʽת������ͨ����ʽתGB 2012 ��ʻ��¼�Ǹ�ʽ
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
u32 _jtb_to_gb(u32 jtb) {
    u32 ret;

    u32 dgr;
    u32 min;

    dgr = (jtb & 0x7fffffff) / 1000000;
    min = (jtb & 0x7fffffff) % 1000000;
    min = (min * 60) / 100;

    ret = (dgr * 600000 * 1uL) + min;

    return ret;
}

/****************************************************************************
* ����:    get_flag_7e_cnt ()
* ���ܣ���ȡ��־λ����
* ��ڲ�������                         
* ���ڲ�������
***************************************************************************/
u8 get_flag_cnt(u8 *buf_t, u16 b_len_t, u8 flag) {
    u8 n;
    u16 len_t;

    n = 0;
    if (buf_t == NULL || b_len_t == 0) {
        return 0;
    }

    len_t = 0;
    while (len_t <= b_len_t) {
        if (*buf_t == flag) {
            n++;
        }

        buf_t++;
        len_t++;
    }

    return n;
}

#if (P_DES == DES_BASE)
/*
//�ֽڵĶ��ѭ������   
//C���Ƶ��ַ� 
//b���Ƶ�λ�� 
*/
u8 _crol_(u8 c, u8 b) {
    u8 i;
    u8 tmp = 0;

    for (i = 0; i < b; i++) {
        if (c & 0x80)
            tmp = 0x01;
        else
            tmp = 0x00;
        c <<= 1;
        c |= tmp;
    }
    return c;
}

/*
//�ֽڵĶ��ѭ������   
//C���Ƶ��ַ� 
//b���Ƶ�λ�� 
*/
u8 _cror_(u8 c, u8 b) {
    u8 i;
    u8 tmp = 0;

    for (i = 0; i < b; i++) {
        if (c & 0x01)
            tmp = 0x80;
        else
            tmp = 0x00;

        c >>= 1;
        c |= tmp;
    }
    return c;
}
#endif

//ckp lib
/*********************************************************
��    �ƣ�_get_xor
��    �ܣ��������У�飬�������Ե��ֽ���ʽ����
��    ����
��    ����
��д���ڣ�2012-08-16
*********************************************************/
u8 _get_xor(u8 *src, u16 cnt) {
    u8 ret = 0x00;
    while (cnt--) {
        ret ^= *(src++);
    }

    return ret;
}

//crc16
unsigned short crc16Modbus_Get2(unsigned char *p, unsigned short len, unsigned short crc16Init) {
    unsigned short crc16 = crc16Init;

    unsigned short i;
    for (i = 0; i < len; i++) {
        crc16 = p[i] ^ crc16;

        unsigned char j;
        for (j = 0; j < 8; j++) {
            if (crc16 & 0x0001) {
                crc16 >>= 1;
                crc16 ^= 0xA001; // POLY
            } else {
                crc16 >>= 1;
            }
        }
    }

    return crc16;
}

//crc16
unsigned short crc16Modbus_Get(unsigned char *p, unsigned short len) {
    return crc16Modbus_Get2(p, len, 0xFFFF);
}

/*********************************************************
��    �ƣ�verify_sum
��    �ܣ��������У�飬�����������ֽ���ʽ����
��    ����
��    ����
��д���ڣ�2012-08-16
*********************************************************/
u32 verify_sum(u8 *src, u16 cnt) {
    u32 ret;

    ret = 0x00;
    while (cnt--) {
        ret += *(src++);
    }

    return ret;
}

/*********************************************************
��    �ƣ�data_check_valid
��    �ܣ�Ѱ���ṩ���ַ�������Ч��ʼλ��������Чλ��ָ��
��    ����
��    ����
��д���ڣ�2012-08-16
*********************************************************/
u8 *data_check_valid(u8 *src) {
    u8 i, cnt;

    cnt = _strlen((u8 *)src);

    for (i = 0x00; i < cnt; i++) {
        if (*(src + i) != '0') {
            return (src + i);
        } else if (*(src + i + 1) == '.') //��������а���С���㣬������������λ��0
        {
            return (src + i);
        }
    }

    return ((src + cnt) - 1);
}

/*********************************************************
��    �ƣ�data_mask_ascii
��    �ܣ���һ��16��������ת����һ��ASCII����
��    ����
��    ����
��д���ڣ�2012-08-16
*********************************************************/
void data_mask_ascii(u8 *dest, u32 s_data, u8 len) {
    while (len--) {
        dest[len] = ((s_data % 10) + '0');
        s_data /= 10;
    }
}

/*********************************************************
��    �ƣ�data_comm_hex
��    �ܣ���һ��ASCII����ת����һ��16��������
��    ����
��    ����
��д���ڣ�2012-08-16
*********************************************************/
u32 data_comm_hex(u8 *src, u8 len) {
    u32 temp = 0x00;

    while (len--) {
        temp *= 10;
        temp += (*(src++) - '0');
    }

    return temp;
}

/*********************************************************
��    �ƣ�data_ascii_to_hex
��    �ܣ���ASCII��ת����HEX
��    ����
��    ������
��д���ڣ�2012-08-16
*********************************************************/
void data_ascii_to_hex(u8 *dest, u8 *src, u16 len) {
    u8 dh, dl;

    while (len) {
        if (*src <= '9')
            dh = (*(src++) - '0');
        else
            dh = (*(src++) - 0x37);

        if (*src <= '9')
            dl = (*(src++) - '0');
        else
            dl = (*(src++) - 0x37);

        *(dest++) = ((dh << 4) + dl);
        len -= 2;
    }
}

/*********************************************************
��    �ƣ�data_hex_to_ascii
��    �ܣ���HEXת����ASCII��
��    ����
��    ������
��д���ڣ�2012-08-16
*********************************************************/
void data_hex_to_ascii(u8 *dest, u8 *src, u16 len) {
    u8 dh, dl;

    while (len) {
        dh = (*src >> 4);
        dl = (*src & 0x0f);

        src++;
        len--;

        if (dh > 0x09)
            *(dest++) = (dh + 0x37);
        else
            *(dest++) = (dh + '0');

        if (dl > 0x09)
            *(dest++) = (dl + 0x37);
        else
            *(dest++) = (dl + '0');
    }
}

/*********************************************************
��    �ƣ�find_the_symbol
��    �ܣ����ַ����в��ҵڼ���ĳ���ַ���
��    ����
    src:     ��Ҫ�����ַ����ַ���
    len:     �ô��ַ������ܳ���
    tmp_str: ��Ҫ��ѯ��ĳ���ַ�
    n:       �ڼ����ַ�
��    �������Ϊԭ�ַ����д�����ַ���λ�����ƫ����
��д���ڣ�2013-05-28
����:���
*********************************************************/
u16 find_the_symbol(u8 *src, u16 len, u8 tmp_str, u16 n) {
    u16 i, cnt;

    cnt = 0x00;
    for (i = 0x00; i < len; i++) {
        if (*src == tmp_str) {
            cnt++;

            if (cnt == n) {
                break;
            }
        }

        src++;
    }

    return i;
}

//��һ���ķ�Χ�ڼ���һ���ַ����ĳ���
u16 str_len(void *s_buff, u16 max_lgth) {
    u16 n;

    for (n = 0x00; n < max_lgth; n++) {
        if (*(((u8 *)(s_buff)) + n) == '\x0') {
            break;
        }
    }

    return n;
}

//ckp lib
/*********************************************************
��    �ƣ�data_hex_to_ascii
��    �ܣ���HEXת����ASCII��
��    ����
��    ������
��д���ڣ�2012-08-16
*********************************************************/
void time_to_ascii(u8 *dest, u8 *src, u16 len) {
    u8 dh, dl;

    while (len) {
        dh = (*src / 10);
        dl = (*src % 10);

        src++;
        len--;

        if (dh > 0x09)
            *(dest++) = (dh + 0x37);
        else
            *(dest++) = (dh + '0');

        if (dl > 0x09)
            *(dest++) = (dl + 0x37);
        else
            *(dest++) = (dl + '0');
    }
}

void ascii_to_hex(u8 *dst, u8 *buf, u16 len) {
    u8 dh, dl;
    u16 i, j = 0;

    for (i = 0; i < len; i++) {
        if (*buf < 0x3a)
            dh = *buf++ - 0x30;
        else
            dh = *buf++ - 0x37;

        if (*buf < 0x3a)
            dl = *buf++ - 0x30;
        else
            dl = *buf++ - 0x37;

        dst[j++] = ((dh << 4) + dl);
    }
}

//type:0,hex only;1,string;2 hex & string
static void putHexs_sub(u8 *p, int len, int type, int bg, int end, unsigned short len_lim_promt, log_level_enum lv) {
    if (p == NULL || len <= 0 || type < 0 || type > 2) {
        loge("\r\nerr:%d%d%d%d", (p == NULL), (len <= 0), (type < 0), (type > 2));
        return;
    }

    if (len_lim_promt < 2) len_lim_promt = 2;
    if (len_lim_promt > 300) len_lim_promt = 300;

    int i = 0;

    if (type == 0 || type == 2) {
        logb_empty(lv, "{");
        for (i = 0; i < len - 1; i++) {
            if (i != 0 && i % 200 == 0) {
                uart1_tran_task_nms(5);
            }

            if (len > (len_lim_promt + 10) && (i >= (len_lim_promt / 2)) && (i <= (len - (len_lim_promt / 2)))) {
                if (i == (len_lim_promt / 2)) {
                    logb_empty(lv, "... ");
                }

                continue;
            }

            if (i == end && bg == end + 1) {
                logb_empty(lv, "%02X [] ", p[i]);
            } else {
                if (i == bg && bg <= end) {
                    logb_empty(lv, "[");
                }

                if (i == end && bg <= end) {
                    logb_empty(lv, "%02X] ", p[i]);
                } else {
                    logb_empty(lv, "%02X ", p[i]);
                }
            }
        }
        logb_empty(lv, "%02X}.len is %d", p[i], len);
        if (type == 2) {
            logb_empty(lv, ", ");
        }
    }

    if (type == 1 || type == 2) {
        int lenStr = 0;
        for (i = 0; i < len; i++) {
            if (p[i] == '\0') {
                break;
            }
            lenStr++;
        }

        if (lenStr < len) {
            logb_empty(lv, "<%s>.len is %d < %d", (char *)p, lenStr, len);
        } else {
            logb_empty(lv, "<");
            for (i = 0; i < len; i++) {
                if (i != 0 && i % 200 == 0) {
                    uart1_tran_task_nms(5);
                }
                logb_empty(lv, "%c", p[i]);
            }
            logb_empty(lv, ">\0.len is %d", len);
        }
    }

    logb_endColor_newLn(lv, );
}

//type:0,hex only;1,string;2 hex & string
void putHexs(const char *func, int line, u8 *p, int len, int type, int bg, int end) {
    if (func == NULL) {
        loge("err");
        return;
    }

    logd_NoNewLn("(%s, %d) = ", func, line);

    putHexs_sub(p, len, type, bg, end, 16,log_level_debug);
}

//type:0,hex only;1,string;2 hex & string
void rk_mcu_RxTx(const char *dir, u16 cmd, const char *func, int line, const char *comment, u8 *p, int len, int type, int bg, int end, unsigned short len_lim_promt, log_level_enum lv) {
    if (func == NULL || dir == NULL) {
        loge("err");
        return;
    }

    if (comment == NULL) {
        logb_NoNewLn(lv, "[%s.%04X](%s, %d) = ", dir, cmd, func, line);
    } else {
        logb_NoNewLn(lv, "[%s.%04X](%s, %d, %s) = ", dir, cmd, func, line, comment);
    }

    putHexs_sub(p, len, type, bg, end, len_lim_promt, lv);
}

#if (0)
void trim(char *str) {
    //#define DEBUG_HERE_202209032025
    if (str == NULL) {
        return;
    }

    int len = strlen(str);
    if (len <= 0) {
        return;
    }

#ifdef DEBUG_HERE_202209032025
    logd("len %d", len);
    putHexs((u8 *)str, len);
    logd("str 0x%p", str);
#endif

    int start = 0;
    int end = len - 1;

#ifdef DEBUG_HERE_202209032025
    logd("s");
#endif
    while (1) {
        char c = *(str + start);
        if (/*!isspace(c)*/ c != ' ') { //isspace//�쳣�����ĳ�����
#ifdef DEBUG_HERE_202209032025
            logd("%02X, s 0x%p, %d", c, (str + start), start);
#endif
            break;
        }

        if (++start > end) {
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
#endif
            str[0] = '\0';
            return;
        } else {
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
#endif
        }
    }

#ifdef DEBUG_HERE_202209032025
    logd("e");
#endif
    while (1) {
        char c = *(str + end);
        if (/*!isspace(c)*/ c != ' ') { //isspace//�쳣�����ĳ�����
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
            logd("%02X, e 0x%p, %d, %d", c, (str + end), end, end - start);
#endif
            break;
        }

        if (--end < start) {
            str[0] = '\0';
            return;
        } else {
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
#endif
        }
    }

    len = end - start + 1;
    memmove(str, str + start, len);
    str[len] = '\0';

#ifdef DEBUG_HERE_202209032025
    logd("len %d", len);
#endif

    len = strlen(str);

#ifdef DEBUG_HERE_202209032025
    logd("len %d", len);
    putHexs((u8 *)str, len);
#endif
}
#else
void trim(char *str) {
    //#define DEBUG_HERE_202209032025

    if (str == NULL) {
        return;
    }

    char *start;
    char *end;
    int len = strlen(str);

#ifdef DEBUG_HERE_202209032025
    logd("len %d", len);
    putHexs((u8 *)str, len);
    logd("str 0x%p", str);
#endif

    start = str;
    end = str + len - 1;

    while (1) {
        char c = *start;
        if (/*!isspace(c)*/ c != ' ') { //isspace//�쳣�����ĳ�����
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
            logd("start 0x%p, %d", start, start - str);
#endif
            break;
        }

        if (++start > end) {
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
#endif
            str[0] = '\0';
            return;
        } else {
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
#endif
        }
    }

    while (1) {
        char c = *end;
        if (/*!isspace(c)*/ c != ' ') { //isspace//�쳣�����ĳ�����
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
            logd("end 0x%p, %d, %d", end, end - str, end - start);
#endif
            break;
        }

        if (--end < start) {
            str[0] = '\0';
            return;
        } else {
#ifdef DEBUG_HERE_202209032025
            logd("%02X", c);
#endif
        }
    }

    len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\0';

#ifdef DEBUG_HERE_202209032025
    logd("len %d", len);
#endif

    len = strlen(str);

#ifdef DEBUG_HERE_202209032025
    logd("len %d", len);
    putHexs((u8 *)str, len);
#endif
}
#endif

//ʮ����ת��ΪBCD//00~99
unsigned char dec2bcd(unsigned char dec) {
    return (((dec % 100) / 10) << 4) + ((dec % 10) & 0x0F);
}

unsigned short reverse_u16(unsigned short val) {
    union {
        unsigned char arr[2];
        unsigned short val;
    } tmp = {.val = val};

    unsigned char swap = tmp.arr[0];
    tmp.arr[0] = tmp.arr[1];
    tmp.arr[1] = swap;

    return tmp.val;
}
