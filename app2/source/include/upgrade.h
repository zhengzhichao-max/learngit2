/**
  ******************************************************************************
  * @file    upgrade.h
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  Զ����������
  ******************************************************************************
  * @attention
  ******************************************************************************
*/
#ifndef __UPGRADE_H
#define __UPGRADE_H
///#include "define.h"

#include "ckp_mcu_file.h"

#define font_lib_max_lgth ((ulong)0x100000)     //�ֿ��ļ���󳤶�
#define font_lib_once_max_lgth ((uint)2080)     //�ֿ��ļ�����һ�ζ�ȡ����󳤶�
#define update_tf_once_max_lgth ((uint)2048)    //TF���ӻ�����һ�ζ�ȡ����󳤶�
#define parameter_cfg_once_max_lgth ((uint)512) //�����Զ�����һ�ζ�ȡ���ݵ���󳤶�

#define PAGE_SIZE (1024 * 2)

#define BootStartAddress ((u32)0x08000000)   //BOOT��ʼ��ַ
#define ApplicationAddress ((u32)0x08004000) //Ӧ�ó���ʼ��ַ

#if (TR_USING_IAP)
#define MCU_NORMAL_CFG_ADDR ((u32)0x0803D000) //��ͨ������ŵ�ַ
#define MCU_IMPORT_CFG_ADDR ((u32)0x0803D800)
#define BLParaAddress ((u32)0x0803E000) //������־0x0800F000
#else
/*
512K
STM32F105VC(256K)
IROM1:
0x8000000
0x7F400
*/
#define MCU_NORMAL_CFG_ADDR ((u32)0x08080000 - 1024 * 6) //��ͨ������ŵ�ַ
#define MCU_IMPORT_CFG_ADDR ((u32)0x08080000 - 1024 * 4)
#define BLParaAddress ((u32)0x08080000 - 1024 * 2) //������־0x0800F000
#endif

#define APP_MAX_DATA_LEN ((BLParaAddress - ApplicationAddress) * 1ul)
/*
IAP flash �ռ����256K
*/
//spi_flash.h//�Ǳ��ж��壬���ظ�����
//#define SECT_IAP_ADDR (SECT_IAP_TMP_ADDR + SECT_IAP_TMP_SIZE)
//#define SECT_IAP_SIZE 64

#define ADDR_IAP_TMP (SECT_IAP_TMP_ADDR * 0x1000 * 1ul)
#define ADDR_IAP_MIN (SECT_IAP_ADDR * 0x1000 * 1ul)
#define ADDR_IAP_MAX ((SECT_IAP_ADDR + SECT_IAP_SIZE + 1) * 0x1000 * 1ul)

///#define FLASH_TOTAL_ADDR			0x04000000-0x00002000

#define UPGRADE_AUTO 0
#define UPGRADE_BYPASS 1
#define UPGRADE_IGNORE_ALL 2

#define _upg_addr(ptr) ((ADDR_IAP_MIN + ptr) * 1ul)

typedef struct {
    u16 crc;
    u16 flag;
    u32 ApplicationFlag;      //��־λ
    u32 ProgramDataLen;       //���ߵȴ����µĳ����ļ�����
    u32 ProgramAddCheckValue; //���ߵȴ����µĳ����ļ��Ӻ�У��ֵ
    u8 ProgramXorValue;       //���ߵȴ����µĳ����ļ����У��ֵ
    u8 FilePageNum;           //�ȴ����µĳ����ļ���ҳ��
    u8 res[2];
} SPI_FLASH_SAVE;

extern SPI_FLASH_SAVE sp;

#define UPGRADE_SD 0
#define UPGRADE_OTA 1
#define UPGRADE_ERR 2

typedef struct {
    u16 crc;
    u16 flag;
    bool enable; //����ʹ��
    bool ack;    //���ݰ�Ӧ��
    bool newest; //���������°汾
    bool banben; //

    u8 mode;         //����ģʽ1: ����  0: SD��
    u8 err_cnt;      //���������������ʧ��
    u8 xor ;         //���������ֵ
    u8 xor_tmp;      //���������ֵ
    u8 filename_len; //�ļ�������
    u16 pack_num;    //�ܰ���
    u16 pack_no;     //�Ѷ�����
    u16 time_out;    //60�� ���ʱ�� 1������ʱ��
    u16 ack_tick;    //Ӧ��ʱ��
    u16 buf_len;     //����
    u32 enable_tick; //ʹ�ܵ�ʱ��
    u32 file_len;    //��ǰ�����ļ����ܳ���
    u32 read_len;    //��ǰ�Ѷ������ļ���С
    u32 d_len;       //�����ܳ���
    u32 sum;         //�������ۼӺ�
    u32 sum_tmp;     //�������ۼӺ�
    u8 filename[36]; //�����ļ����ļ�����
    bool slv_en;
} s_UPDATAECONTROL;

typedef struct
{
    bool start;
    bool start_m;
    bool boot;
    uchar step; //�ֿ���������
    uchar typ;  //�ֿ�汾����  0�ϰ汾  1�°汾131209 (��ʱδʹ��)
    u8 total_pack;
    u8 write_pack;
    u16 PackLen;
    ulong wr_lgth; //�������ֽ�����
    ulong cnt;     //��д���ֽ�������
    ulong tmr;
} font_lib_update_struct;

typedef struct
{
    uchar step; //��������

    uchar check_xor; //���У��
    ulong check_sum; //��У��

    ulong wr_lgth;       //�������ֽ�����
    ulong cnt;           //��д���ֽ�������
} update_tf_data_struct; //�������ݽṹ

typedef struct
{
    u8 step;            //�������ò���
    u32 wr_lgth;        //���������ֽ���
    u32 cnt;            //��д����ֽڼ���
} parameter_cfg_struct; //�������ýṹ�� 2017-12-6

typedef struct
{
    u8 step;            //��ʻ��¼�Ƕ�ȡ����
    u32 rd_lgth;        //����ȡ���ֽ���
    u32 cnt;            //�Զ�ȡ���ֽ���
} read_rcd_data_struct; //U�̶�ȡ��ʻ��¼�ǽṹ�� 2017-12-15

typedef struct //2017-12-6
{
    bool ic_platform_flag; //ƽ̨������־λ

} upgrade_status_flag;

extern upgrade_status_flag usb_upgrade; //usb������־�ṹ��

extern s_UPDATAECONTROL upgrade_m; //�������ƽṹ��
extern font_lib_update_struct font_lib_update;
extern update_tf_data_struct update_tf_data;
extern parameter_cfg_struct parameter_auto_cfg;
extern read_rcd_data_struct read_rcd_data_cfg; //��¼�����ݶ�ȡ�ṹ�� 2017-12-15
extern u8 update_buf[];                        //�������ݻ���
extern void auto_ack_tr9_data(void);

extern void init_upgrade(void);
extern u8 check_update_version(u8 type, u8 *buf, u16 len);
extern void update_from_sd_mess(u8 *buf, u16 len);
extern void upgrade_program_task(void);
extern void com1_down_font_lib(void);
extern void update_font_lib(u8 *lib, u16 len);
extern void update_tf_main(u8 *lib, u16 len);

#endif /* __UPGRADE_H */
