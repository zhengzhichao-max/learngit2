////////////////////////////////////////////////////////////////
//   LED - 9600 
//   注意事项:  1、低电平为空重载检测线
////////////////////////////////////////////////////////////////
#include "include_all.h"

#if (0)
#define LED_CHK_TIME  0x54494D45	//
#define LED_ASK_TIME   0x50574F4E	//LED请求标定
#define LED_ALA_CMD   0x414C4152
#define LED_UALA_CMD   0x55414C4D
#define LED_DRIV_CMD  0x44524956    //行车命令、GPS发给LED、：0x00-空车；0x01-载客；0x02-电召；0x03-停运；0x04-报警；0x05-其他； 0xFD-全黑；0xFE全亮

//0x00-空车；0x01-载客；0x02-电召；0x03-停运；0x04-报警；0x05-其他； 0xFD-全黑；0xFE全亮
//状态发生变化时发送数据到LED 屏
//空车
uc8 led_null_car[] = {0x7E, 0x44, 0x00, 0x0C, 0x00, 0x00, 0x44, 0x52, 0x49, 0x56, 0x00, 0x00, 0x00, 0x3B, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
//载客
uc8 led_full_car[] = {0x7E, 0x44, 0x00, 0x0C, 0x00, 0x00, 0x44, 0x52, 0x49, 0x56, 0x01, 0x00, 0x00, 0x0C, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
//电招
uc8 led_eleccall_car[] = {0x7E, 0x44, 0x00, 0x0C, 0x00, 0x00, 0x44, 0x52, 0x49, 0x56, 0x02, 0x00, 0x00, 0x55, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
//停运
uc8 led_stop_car[] = {0x7E, 0x44, 0x00, 0x0C, 0x00, 0x00, 0x44, 0x52, 0x49, 0x56, 0x03, 0x00, 0x00, 0x62, 0xF2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};
//报警
uc8 led_ala_car[] = {0x7E, 0x44, 0x00, 0x0C, 0x00, 0x00, 0x44, 0x52, 0x49, 0x56, 0x04, 0x00, 0x00, 0xE7, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7E};


uc8 led_alarm_open[]  = {0x7E, 0x43, 0x00, 0x0B, 0x00, 0x00, 0x41, 0x4C, 0x41, 0x52, 0x00, 0x05, 0xF5, 0x44, 0x7E};
uc8 led_alarm_close[] = {0x7E, 0x43, 0x00, 0x09, 0x00, 0x00, 0x55, 0x41, 0x4C, 0x4D, 0x6B, 0x7A, 0x7E};

uc16 crc_table[256]=
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,  
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,  
  0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,  
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,  
  0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,  
  0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,  
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,  
  0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,  
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,  
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,  
  0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,  
  0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,  
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,  
  0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,  
  0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,  
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,  
  0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,  
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,  
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,  
  0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,  
  0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,  
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,  
  0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,  
  0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,  
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,  
  0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,  
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,  
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,  
  0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,  
  0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,  
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,  
  0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0  
};

u8 iled_func = 0;   //清零后不再发送；否则10秒1次
bool bohai_led_time_send = false;
u16  volt_cut_delay_time = 0;

/*********************************************************
名    称：GetCrc16
功    能：查表法计算16位CRC校验码
参    数：u8 *pData, u16 nLength
输    出：无
编写日期：2011-03-24
*********************************************************/
u16 GetCrc16(u8 *pData, u16 nLength)
{
/*
    u16 Result = 0;
    while(nLength>0)
    {
		Result = (u16)(crc_table[(Result ^ *pData) & 0xff] ^ (Result >> 8));
		nLength--;
		pData++;
	}
    return Result;
*/
	unsigned long i;
	unsigned short nAccum = 0;
	for ( i = 0; i < nLength; i++ )
		nAccum = ( nAccum << 8 ) ^ ( unsigned short )crc_table[( nAccum >> 8) ^ *pData++];
	return nAccum;
}

static void send_data_to_led( u8 *msg, u16 mlen )
{
	if(u1_m.cur.b.tt){
		send_any_uart(FROM_U1, msg, mlen);
	}

#if (MCU == STM32F103VCT6)
	 if(u2_m.cur.b.tt){
		send_any_uart(FROM_U2, msg, mlen);
	}

	 if(u3_m.cur.b.tt){
		send_any_uart(FROM_U3, msg, mlen);
	}
#endif	

}

/****************************************************************************
* 名称:    get_uart_is_led ()
* 功能：判断串口是否是LED 或透传功能
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
static bool get_uart_is_led(u8 from_t)
{
	if(u1_m.cur.b.tt == true && from_t == FROM_U1)
	{
		return true;
	}
#if (MCU == STM32F103VCT6)
	else if(u2_m.cur.b.tt == true && from_t == FROM_U2)
	{
		return true;
	}
	else if(u3_m.cur.b.tt == true && from_t == FROM_U3)
	{
		return true;
	}
#endif
	else
	{
		return false;
	}
}

void LED_Function_Proc( u8 func )
{
	u8 buf[30]={0};
	
	if( func == 0x01 )
	{
		_memcpy( buf, led_null_car, 22);	
	}	
	else if( func == 0x02 )
	{
		_memcpy( buf, led_full_car, 22);	
	}
	else if( func == 0x03 )
	{
		_memcpy( buf, led_eleccall_car, 22);	
	}
	else if( func == 0x04 )
	{
		_memcpy( buf, led_stop_car, 22);	
	}
	else if( func == 0x05 )
	{
		_memcpy( buf, led_ala_car, 22);	
	}
	else 
	{
		return;	
	}	
	send_data_to_led( buf, 22 );
}

void gps_alarm_to_led( bool ala )
{
	if(ala)
	{
		iled_func = 0x02;
		send_data_to_led( (u8*)&led_alarm_open, 15 );
	}
	else
	{
		iled_func = 0x03;
		send_data_to_led( (u8*)&led_alarm_close, 13 );
	}
}

/****************************************************************************
* 名称:    get_week ()
* 功能：计算星期
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
u8 get_week(u8 year, u8 month, u8 date)
{
	u8 y, m, r;

	if (month >= 3) {
		m = month - 2;
		y = year;
	} else {
		m = month + 10;
		y = year == 0x00 ? 99 : year - 1;
	}
	y = y/4 + y;
	r = m * 13;
	(year == 0x00 && month < 3) ? (r += 4) : (r -= 1);

	return ((y + r/5 + date) % 7);
}

/************************************************
	发送紧急报警信息到LED 屏
************************************************/
void LedSendAlaCmd( void )
{
	u8 msg[64]={0};
 //  	u8  i, xor;
	u8 mlen = 0;
	//u8 week_t = 0;
	u16 crc16 = 0;

	u32 idc;

	
	msg[mlen++] = 0x7e;
	msg[mlen++] = 0x43;
	msg[mlen++] = 0x00;
	msg[mlen++] = 0x0B;	//长度
	msg[mlen++] = 0x00;
	msg[mlen++] = 0x00;

	idc = LED_ALA_CMD;
	_sw_endian(msg+mlen, (u8*)&idc, 4);

	mlen += 4;
	
	msg[mlen++] = 0x00;
	msg[mlen++] = 30;

	crc16 = GetCrc16( msg+1, mlen-1 );

	msg[mlen++] = crc16>>8;
	msg[mlen++] = crc16;
	//转义处理 

	mlen += _add_filt_char(mlen-1, &msg[1]);
	msg[mlen++] = 0x7e;		

	send_data_to_led( msg, mlen );
}

/****************************************************************************
* 名称:    led_parse_proc ()
* 功能：LED或透传数据解析
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void led_parse_proc(void) {
    typedef enum {
        E_LED_IDLE,
        E_LED_RECV,
        E_LED_EXIT
    } E_LED_STEP;

    static E_LED_STEP step = E_LED_IDLE;
    bool ret;

    u8 type = 0;
    u8 r_buf[U1_BUF_SIZE];
    u8 pack[U1_BUF_SIZE];
    u16 r_len = 0;
    u16 pack_len = 0;

    static u8 from_id = 0;
    static UART_MANAGE_STRUCT present;

    switch (step) {
    case E_LED_IDLE:
        from_id = get_uart_news(&present);
        ret = get_uart_is_led(from_id);
        if (ret)
            step = E_LED_RECV;
        break;
    case E_LED_RECV:
        _memset(r_buf, 0x00, U1_BUF_SIZE);
        r_len = copy_uart_news(from_id, r_buf, present.cur_p, present.c_len);
        if (r_len == 0 || r_len > U1_BUF_SIZE) {
            step = E_LED_EXIT;
            break;
        }

        pack_len = 0;
        if (from_id == FROM_U1)
            type = 0x07;
        else if (from_id == FROM_U2)
            type = 0x07;
        else
            type = 0x07;

        pack[pack_len++] = type;
        pack_len += _memcpy_len(&pack[pack_len], r_buf, r_len);

        if (car_login)
            pack_any_data(CMD_UP_DATA_PASS, pack, pack_len, PK_HEX, LINK_IP0 | LINK_IP1 | LINK_IP2);
        step = E_LED_EXIT;
        break;
    case E_LED_EXIT:
        set_uart_ack(from_id);
        step = E_LED_IDLE;
        break;
    default:
        step = E_LED_IDLE;
        break;
    }
}

/****************************************************************************
* 名称:    acc_timing_led ()
* 功能：LED 广告屏时间校准
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void acc_timing_led(void) {
    u8 msg[64] = {0};
    u8 i, xor;
    u8 mlen = 0;
    u8 week_t = 0;
    //u16 crc16 = 0;

    //u32 idc;

    week_t = get_week(sys_time.year, sys_time.month, sys_time.date);

    if (week_t > 7)
        return;

    _memset(msg, 0x00, 64);
    mlen = 0;
    msg[mlen++] = 0x24;
    msg[mlen++] = 0x24;
    msg[mlen++] = 0x54;
    msg[mlen++] = 0x13; //长度
    msg[mlen++] = (sys_time.year / 10) + 0x30;
    msg[mlen++] = (sys_time.year % 10) + 0x30;
    msg[mlen++] = (sys_time.month / 10) + 0x30;
    msg[mlen++] = (sys_time.month % 10) + 0x30;
    msg[mlen++] = (sys_time.date / 10) + 0x30;
    msg[mlen++] = (sys_time.date % 10) + 0x30;
    msg[mlen++] = (sys_time.hour / 10) + 0x30;
    msg[mlen++] = (sys_time.hour % 10) + 0x30;
    msg[mlen++] = (sys_time.min / 10) + 0x30;
    msg[mlen++] = (sys_time.min % 10) + 0x30;
    msg[mlen++] = (sys_time.sec / 10) + 0x30;
    msg[mlen++] = (sys_time.sec % 10) + 0x30;
    msg[mlen++] = (week_t % 10) + 0x30;

    xor = 0;
    for (i = 0; i < mlen; i++) {
        xor ^= msg[i];
    }
    msg[mlen++] = xor;
    msg[mlen++] = 0x26;
    send_data_to_led(msg, mlen);
}

/************************************************
	发送取消紧急报警信息到LED 屏
************************************************/
void LedSendUnAlaCmd(void) {
    u8 msg[64] = {0};
    //	u8  i;
    // u8 xor;
    u8 mlen = 0;
    //	u8 week_t = 0;
    u16 crc16 = 0;

    u32 idc;

    msg[mlen++] = 0x7e;
    msg[mlen++] = 0x43;
    msg[mlen++] = 0x00;
    msg[mlen++] = 0x09; //长度
    msg[mlen++] = 0x00;
    msg[mlen++] = 0x00;

    idc = LED_UALA_CMD;
    _sw_endian(msg + mlen, (u8 *)&idc, 4);

    mlen += 4;

    msg[mlen++] = 0x00;
    msg[mlen++] = 30;

    crc16 = GetCrc16(msg + 1, mlen - 1);

    msg[mlen++] = crc16 >> 8;
    msg[mlen++] = crc16;
    //转义处理

    mlen += _add_filt_char(mlen - 1, &msg[1]);
    msg[mlen++] = 0x7e;

    send_data_to_led(msg, mlen);
}

#endif


