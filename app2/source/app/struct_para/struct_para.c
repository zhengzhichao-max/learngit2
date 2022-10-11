/****

****/

#define MAX_GPS_SIZE		100	//�ṹ������32���ֽ�		
#define GPS_ADDR			(u32)(SECT_GPS_INFO_ADDR*0x1000*1ul)   //F_GPS_INFO_ADDR
#define _gps_addr(ptr)    		(GPS_ADDR+ptr *36*1uL)
u16 save_gps_pos = MAX_GPS_SIZE;

/****************************************************************************
* ����:    load_save_parameteror ()
* ���ܣ���ָ����SECTOR��Ѱ����Ч��ָ��
1.��flash������ֻ��10��Σ���Ҫѭ���洢
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
bool load_save_parameteror(void)
{
    s16 low;
    s16 mid;
    s16 high;

    GPS_BASE_INFOMATION ps;
    u16 cnt;

    cnt = 0;
    low = 0;
    mid = 0;
    high = MAX_GPS_SIZE;

    while (low <= high)
    {
search_gps_start:
        if (++cnt > 9)
            return false;

        mid = (low + high) / 2;

        spi_flash_read((u8 *)&ps, _gps_addr(mid), sizeof(ps));
        if (ps.flag == 0x55aa) 		//��ǰ��Ч����
        {
            //���У��
            save_gps_pos = mid;
            _memcpy((u8 *)&gps_base, (u8 *)&ps, sizeof(ps));
            return true;
        }
        else if (ps.flag == 0xffff) 	//δʹ������
        {
            high = mid - 1;
        }
        else 	//��ʹ�ù�������
        {
            low = mid + 1;
        }
        goto search_gps_start;
    }

    return false;
}

/****************************************************************************
* ����:    save_gps_parameterer ()
* ���ܣ���������
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void save_gps_parameterer(void)
{
    u8 buf[64];

    if (save_gps_pos >= MAX_GPS_SIZE - 1)
    {
        spi_flash_erase(GPS_ADDR);
        save_gps_pos = 0;
    }
    else
    {
        _memset(buf, 0x00, 64);
        spi_flash_write(buf, _gps_addr(save_gps_pos), sizeof(gps_base));
        save_gps_pos++;
    }

    add_struct_crc((u8 *)&gps_base, sizeof(gps_base));
    spi_flash_write((u8 *)&gps_base, _gps_addr(save_gps_pos), sizeof(gps_base));
}

/****************************************************************************
* ����:    init_gps_parameterer ()
* ���ܣ���ʼ��
* ��ڲ�������
* ���ڲ�������
****************************************************************************/
void init_gps_parameterer(void)
{
    sys_time.year = 17;
    sys_time.month = 1;
    sys_time.date = 1;
    sys_time.hour = 9;
    sys_time.min = 0;
    sys_time.sec = 0;
	
    save_gps_parameterer();
}



