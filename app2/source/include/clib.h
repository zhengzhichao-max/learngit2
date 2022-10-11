/**
  ******************************************************************************
  * @file    clib.h
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  库函数
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef __CLIB_H__
#define __CLIB_H__

extern int _sprintf_len(char *buf, const char *fmt, ...);
extern int _printf2(const char *fmt, ...);

//在标准库文件的基础上添加'\0' 判断
char _tolower(char ch);
extern u16 _strlen(u8 *str);
extern u8 *_strcpy(u8 *str1, uc8 *str2);
extern u8 _strcpy_len(u8 *str1, u8 *str2);
extern u8 *_strcat(u8 *str1, uc8 *str2);
extern u8 *_strncat(u8 *str1, uc8 *str2, u8 n);
extern u8 *_strncpy(u8 *dst, uc8 *sur, u8 n);
extern s16 _strcmp(u8 *str1, u8 *str2);
extern s16 _strncmp(u8 *str1, u8 *str2, u8 n);
//功能：比较字符串s1和s2，但不区分字母的大小写。
int stricmp(const char *str1, const char *str2);
//功 能： 比较字符串str1和str2的前n个字符串字典序的大小，但是不区分字母大小写。
int _strnicmp(const char *str1, const char *str2, int size);
extern u8 *_strstr(u8 *str1, u8 *str2);
extern s16 _strspn(uc8 *str1, uc8 *str2);
extern s16 _strcspn(uc8 *str1, uc8 *str2);
extern u8 *_strtok(u8 *str1, uc8 *str2);

//内存操作
extern void _memset(u8 *str1, u8 c, u16 len);
extern void _memcpy(u8 *cp1, uc8 *cp2, u16 n);
extern u16 _memcpy_len(u8 *cp1, u8 *cp2, u16 n);
extern u8 _sw_endian(u8 *cp1, u8 *cp2, u8 n);

//格式转换如xxx to yyy ，_bcdtobin , bcd 格式转换到bin格式
extern void _htoa(u8 *str, u8 hex);
extern u16 _str2tobcd(u8 *bByte, u8 *str);
extern u8 _bcdtobin(u8 bByte);
extern u8 _bintobcd(u8 bByte);
extern u8 _nbcdtobin(u8 *p, u8 n_t);
extern u8 _nbintobcd(u8 *p, u8 n_t);
extern u8 _chartohex(u8 uChar_H, u8 uChar_L);
extern u8 AsctoBCD(u16 const *pStr, u8 *bBcd, u8 len, u8 ifz);
extern u8 bintohex(u8 bin);
extern u16 _HexToBCD(u16 bcd);

//转义和反转义
extern u16 _del_filt_char(u16 len, u8 *chr);
extern u16 _add_filt_char(u32 len, u8 *chr);

//公用函数
extern bool _verify_time(time_t *t);
extern u8 _get_sum(u8 *p, u16 len);
extern u8 _get_xor(u8 *p, u16 len);
//crc16
unsigned short crc16Modbus_Get2(unsigned char *p, unsigned short len, unsigned short crc16Init);
//crc16
unsigned short crc16Modbus_Get(unsigned char *p, unsigned short len);
extern s32 _timetotick(u8 *timep);
extern float _cal_distance(s32 lati_1, s32 lati_2, s32 lngi_1, s32 lngi_2);
extern u32 _jtb_to_gb(u32 jtb);
extern u8 get_flag_cnt(u8 *buf_t, u16 b_len_t, u8 flag);

extern u16 _memcpyCH(u8 *dst, u8 ch, u8 *buf, u16 len);
extern u8 _n_BintoBCD(u8 *p, u8 n_t);
extern u8 DriverCodeBcd(u32 const *pStr, u8 *bBcd, u8 len, u8 ifz);
#if (P_DES == DES_BASE)
extern u8 _crol_(u8 c, u8 b);
extern u8 _cror_(u8 c, u8 b);
#endif
//ckp lib
extern u32 verify_sum(u8 *src, u16 cnt);
extern u8 *data_check_valid(u8 *src);
extern void data_mask_ascii(u8 *dest, u32 s_data, u8 len);
extern u32 data_comm_hex(u8 *src, u8 len);
extern void data_ascii_to_hex(u8 *dest, u8 *src, u16 len);
extern void data_hex_to_ascii(u8 *dest, u8 *src, u16 len);
extern u16 find_the_symbol(u8 *src, u16 len, u8 tmp_str, u16 n);
extern u16 str_len(void *s_buff, u16 max_lgth);
extern void time_to_ascii(u8 *dest, u8 *src, u16 len);
extern void ascii_to_hex(u8 *dst, u8 *buf, u16 len);
extern int _printf_tmp(const char *fmt, ...);
extern int _printf_tr9(const char *fmt, ...);
void trim(char *str);
unsigned char dec2bcd(unsigned char dec);
int str_visible(const char *str, int len_max);

unsigned short reverse_u16(unsigned short val);

#pragma region //putHexs
//type:0,hex only;1,string;2 hex & string
void putHexs(const char *func, int line, u8 *p, int len, int type, int bg, int end);
#define putHexs_hex(p, len) putHexs(__func__, __LINE__, p, len, 0, -1, -1)
#define putHexs_string(p, len) putHexs(__func__, __LINE__, p, len, 1, -1, -1)
#define putHexs_hex_and_string(p, len) putHexs(__func__, __LINE__, p, len, 2, -1, -1)
#pragma endregion //putHexs
#pragma region    //rk_mcu_RxTx
void rk_mcu_RxTx(const char *dir, u16 cmd, const char *func, int line, const char *comment, u8 *p, int len, int type, int bg, int end, unsigned short len_lim_promt, log_level_enum lv);
//-----
#define rk_mcu_comment_hex(dir /*(const char *)*/, cmd, comment, p, len, bg, end, lim, lv) rk_mcu_RxTx(dir, cmd, __func__, __LINE__, comment, p, len, 0, bg, end, lim, lv)
//-----
#define rk2mcu_hex(cmd, p, len, bg, end) rk_mcu_RxTx("rk2mcu", cmd, __func__, __LINE__, NULL, p, len, 0, bg, end, 16, log_level_debug)
#define rk2mcu_string(cmd, p, len, bg, end) rk_mcu_RxTx("rk2mcu", cmd, __func__, __LINE__, NULL, p, len, 1, bg, end, 16, log_level_debug)
#define rk2mcu_hex_and_string(cmd, p, len, bg, end) rk_mcu_RxTx("rk2mcu", cmd, __func__, __LINE__, NULL, p, len, 2, bg, end, 16, log_level_debug)
//-----
#define rk2mcu_comment_hex(cmd, comment, p, len, bg, end, lim, lv) rk_mcu_RxTx("rk2mcu", cmd, __func__, __LINE__, comment, p, len, 0, bg, end, lim, lv)
#define rk2mcu_comment_string(cmd, comment, p, len, bg, end) rk_mcu_RxTx("rk2mcu", cmd, __func__, __LINE__, comment, p, len, 1, bg, end, 16, log_level_debug)
#define rk2mcu_comment_hex_and_string(cmd, comment, p, len, bg, end) rk_mcu_RxTx("rk2mcu", cmd, __func__, __LINE__, comment, p, len, 2, bg, end, 16, log_level_debug)
//-----
#define mcu2rk_hex(cmd, p, len, bg, end) rk_mcu_RxTx("mcu2rk", cmd, __func__, __LINE__, NULL, p, len, 0, bg, end, 16, log_level_debug)
#define mcu2rk_string(cmd, p, len, bg, end) rk_mcu_RxTx("mcu2rk", cmd, __func__, __LINE__, NULL, p, len, 1, bg, end, 16, log_level_debug)
#define mcu2rk_hex_and_string(cmd, p, len, bg, end) rk_mcu_RxTx("mcu2rk", cmd, __func__, __LINE__, NULL, p, len, 2, bg, end, 16, log_level_debug)
//-----
#define mcu2rk_comment_hex(cmd, comment, p, len, bg, end) rk_mcu_RxTx("mcu2rk", cmd, __func__, __LINE__, comment, p, len, 0, bg, end, 16, log_level_debug)
#define mcu2rk_comment_string(cmd, comment, p, len, bg, end) rk_mcu_RxTx("mcu2rk", cmd, __func__, __LINE__, comment, p, len, 1, bg, end, 16, log_level_debug)
#define mcu2rk_comment_hex_and_string(cmd, comment, p, len, bg, end) rk_mcu_RxTx("mcu2rk", cmd, __func__, __LINE__, comment, p, len, 2, bg, end, 16, log_level_debug)
//-----
#pragma endregion //rk_mcu_RxTx

#endif /* __CLIB_H__ */
