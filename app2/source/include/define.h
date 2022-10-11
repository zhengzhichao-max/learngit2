/**  74303540
  ******************************************************************************
  * @file    define.h
  * @author  TRWY_TEAM
  * @Email
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  �궨��
  ******************************************************************************
*/
/**
��Խƽ̨
0x52	1	����ת(0:δ֪��1����ת���ճ���2:��ת���س�����3��ͣת)
0x2B	4	��·������4�ֽڣ��ɱ�ʾ����ģ����
0x50	2	һ·������2�ֽڣ���ģ����
**/
#ifndef __DEFINE_H__
#define __DEFINE_H__

#define XH7_EC20_ZTC_V300
/*======================================================================================*/
/*��ƷPCBӲ����Դ����*/
//PRODUCT

#define M9C19A 770

//MAIN_BOARD
#define P8036_V100 200

//GSM
#define EC20 30   //C��ģ��
#define SLM790 20 //C��ģ��

//MCU
#define STM32F103VCT6 1

//DATA FALSH
#define SST25VF032B 1

//������
#define AREA_FUN_NO 1
#define AREA_BASE 2
#define AREA_EXT 3
#define AREA_CUSTOMIZE 4

//·�߼��
#define LINE_NO 1
#define LINE_BASE 2
#define LINE_EXT 3
#define LINE_CUSTOMIZE 4

//����ͷ
#define CMA_NO 1
#define CMA_BASE 2
#define CMA_CUSTOMIZE 3

//�������ݷ���
#define BATCH_NO 1
#define BATCH_BASE 2
#define BATCH_CUSTOMIZE 3

//��¼�ǹ���
#define RCD_NO 1
#define RCD_BASE 2
#define RCD_CUSTOMIZE 3

//˫IP ����
#define DOUBLEIP_NO 1
#define DOUBLEIP_BASE 2
#define DOUBLE_CUSTOMIZE 3

//CAN ����
#define _CAN_NO 1
#define _CAN_BASE 2
#define _CAN_CUSTOMIZE 3

//֧��Э��
#define NET_TR 1
#define NET_JTB 2  //��ͨ��Э��
#define NET_GB 3   //����Э��
#define NET_DB44 4 //�㶫ʡ��Э��

//�绰����
#define SOUND_NO 1
#define SOUND_BASE 2
#define SOUND_CUSTOMIZE 3

//һ����ӻ�ͨѶ
#define GNSS_NO 1
#define GNSS_BASE 2
#define GNSS_CUSTOMIZE 3

//��׼�ο���ѹ
#define VOLT_25 1
#define VOLT_33 2

/*======================================================================================
�ͻ���������
ԭ��
1.���������֮�����ѽ�DRV���MID�㾡���������иĶ�������APP���޸�
2.���ں���ĸĽ������߱����в�Ʒ���Եģ�ֱ�����������ظĶ�
3.���ڸ�������΢�Ķ������ҿͻ��Ķ���Ƶ���ģ�ֱ��ʹ�ú궨�巽ʽ�޸�
4.���ڿͻ�Ҫ���ܴ����Ķ�����Ķ��Ƚ�Ƶ���ģ����ø����ļ��Ķ�
5.����ͻ�����Ĺ��ܣ���ģ�黯�����޷�ʵ��ʱ�������̣���Ŀ���¿���
6.���޸Ĺ����У��޸Ĳ���Ӱ��������׼����Ʒ�Ĺ���
======================================================================================*/

/*-----------Ӳ���汾-----Ӳ���汾-----Ӳ���汾------Ӳ���汾-------------*/
#define PRODUCT_NAME "XH7_EC20"
#define PCB_VER "ZTC"
#define FW_VER "V300"
#define FW_TIME "211130"
//���������ʾ���ƣ�����24���ֽ�
#define FW_VERSION PRODUCT_NAME "_" PCB_VER "_" FW_VER "_" FW_TIME
/*-----------Ӳ���汾-----Ӳ���汾-----Ӳ���汾------Ӳ���汾-------------*/

#define C_TR9 1

//����+����
#pragma region                //#ifdef XH7_EC20_ZTC_V300 //V600,Сģ��
#ifdef XH7_EC20_ZTC_V300      //V600,Сģ��
#define TRWY_PRODUCT (M9C19A) //2018-1-5 �°汾��USB����
#define MAIN_BOARD (P8036_V100)
#define GSM (EC20)
#define MCU (STM32F103VCT6)
#define P_CAN (_CAN_BASE) //��ҪCAN����
#define P_DBIP (DOUBLEIP_NO)

#define P_AREA (AREA_FUN_NO)
#define P_LINE (LINE_NO) //��·���
//#define P_LINE				(LINE_BASE)         //��·���
#define P_CMA (CMA_NO)
#define P_BATCH (BATCH_NO)
#define P_RCD (RCD_BASE)
#define P_NET (NET_JTB)
#define P_SOUND (SOUND_BASE)
#define P_GNSS (GNSS_NO)
#define p_VOLT_BECH (VOLT_33)

#define JTT_808_2019 1

#define COMPANY (C_TR9)

#pragma region /*���԰汾/���԰汾/�����汾*/
//****************************************************
//M9C19A220905V0D1D4D
#define TR_DEBUG
// #undef TR_DEBUG //����ע�����:��ע�͡� = ��DEBUG�汾��

//****************************************************
#if !defined(TR_DEBUG)
//M9C19A220905V0D1D4T
#define TR_TEST
//#undef TR_TEST//����ע�����:��ע�͡� = ��TEST�汾��
#endif /*TR_DEBUG*/

//****************************************************
#if !defined(TR_DEBUG) && !defined(TR_TEST)
//M9C19A220905V0D1D4
#define TR_PROD
#endif //#if !defined(TR_DEBUG) && !defined(TR_TEST)

#define TR_USING_IAP 0 //����ע�����:���Ƿ�ʹ��IAP��
/*
512K
STM32F105VC(256K)
IROM1:
0x8000000
0x7F400
*/

#pragma endregion /*���԰汾/���԰汾/�����汾*/
/*���԰汾/���԰汾/�����汾*/

/*-----------����汾-----����汾-----����汾------����汾-------------*/
#if (1)                             //2022-09-23
#define TR9_SOFT_VERSION_MAX_LEN 30 //IAP & APP

#define TR9_APP_VERSION_V_XXX "V1.0.13" //����ע�����:

#define TR9_IAP_VERSION ((const char *)(0x08001000))                               //�޳�<=30
#define TR9_IAP_BUILD_TIME ((const char *)(0x08001000 + TR9_SOFT_VERSION_MAX_LEN)) //�޳�12(get_build_time_need_free)//"Sep 20 2022 08:45:26 D/T"

#pragma region //����汾����//EC20+STM ����汾
#if defined(TR_DEBUG)
//���԰汾
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_V_XXX
#elif defined(TR_TEST)
//���԰汾
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_V_XXX
#elif defined(TR_PROD)
//�����汾
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_V_XXX
#endif                              /*TR_DEBUG*/
#pragma endregion                   //����汾����//EC20+STM ����汾
#else                               //2022-08-15
#define TR9_SOFT_VERSION_MAX_LEN 30 //IAP & APP

#define TR9_APP_VERSION_NAME "M9C19A"
#define TR9_APP_VERSION_TIME "220922"  //����-ע�����:
#define TR9_APP_VERSION_V_XXX "V1.0.9" //����-ע�����:

#define TR9_IAP_VERSION ((const char *)(0x08001000))                               //�޳�<=30
#define TR9_IAP_BUILD_TIME ((const char *)(0x08001000 + TR9_SOFT_VERSION_MAX_LEN)) //�޳�12(get_build_time_need_free)//"Sep 20 2022 08:45:26 D/T"

#pragma region //����汾����//EC20+STM ����汾
#if defined(TR_DEBUG)
//���԰汾
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_NAME "" TR9_APP_VERSION_TIME "" TR9_APP_VERSION_V_XXX "D"
#elif defined(TR_TEST)
//���԰汾
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_NAME "" TR9_APP_VERSION_TIME "" TR9_APP_VERSION_V_XXX "T"
#elif defined(TR_PROD)
//�����汾
#define TR9_SOFT_VERSION_APP TR9_APP_VERSION_NAME "" TR9_APP_VERSION_TIME "" TR9_APP_VERSION_V_XXX
#endif            /*TR_DEBUG*/
#pragma endregion //����汾����//EC20+STM ����汾
#endif
/*-----------����汾-----����汾-----����汾------����汾-------------*/
#endif

//**********���ֵ��Խӿ�****************************************
#define _CLK72M_
#define _STM32F105_
//**************************************************************

#ifndef TRWY_PRODUCT
#error "PRODUCT == ?"
#endif
#pragma endregion //#ifdef XH7_EC20_ZTC_V300 //V600,Сģ��

#define DEBUG_EC20
#define BOOT_MODEm

/*======================================================================================*/

/*======================================================================================*/
//2022-08-20
//�����ϱ���RK�ġ������ٶȡ������á������ٶȡ�����
//220828�ٶ�����:�����ٶȣ�����GPS���ٶȣ��ϱ���RK��GPS�ٶȣ������ˣ������ٶȣ�
//�����ٶȣ�����GPS���ٶȣ��ϱ���RK��GPS�ٶȣ������ˣ������ٶȣ�
#define PULSE_SPEED_2_RK_USE_GPS_SPEED 0
/*======================================================================================*/

/*======================================================================================*/
//0:�˵���0��Ϊ�պ���//1���˵���0�Ϊ�����ģ�other�˵�����
//��Ҫ����Ϊ����0��Ϊ�պ�������ԭ���ǣ�other�˵�������ִ��������index��0
//menu_cnt.menu_other//menu_other_point
#define MO_ZERO_IS_NULL_FUN 0 //0����������ʾ//1��ʾҳ�治��ȷ(����һ������)
/*======================================================================================*/

/*======================================================================================*/
#define USE_SYSTERM_BITS1_UNION
//#undef USE_SYSTERM_BITS1_UNION
#define USE_MID_TEST
#undef USE_MID_TEST //������
/*======================================================================================*/

/*======================================================================================*/
//��������0��PA9 PA10����handset����
#define USING_HANDSET
//#undef USING_HANDSET
/*======================================================================================*/

/*======================================================================================*/
//>>>>>>>>>>>>> ��־ģ�� <<<<<<<<<<<<<<
/*======================================================================================*/
//������־ģ��
#define LOG_USING_LOGGER 1 //����ע�����:
//������־ģ����ɫ����

#pragma region //������־ģ��
//������־ģ����ɫ����
#define LOG_USING_COLOR 1 //����ע�����:
//������־ģ��ȼ�����//�������:1��= 0���޵ȼ��ַ��������еȼ���ɫ��2��= 1���еȼ��ַ�������ɫ/����ɫ��
#define LOG_USING_LEVEL 1 //����ע�����:
//������־ģ��ʱ�书��
#define LOG_USING_TIME_INFO 0 //����ע�����:
/*=======*/
//���ļ������������������кš�
#define LOG_USING_FILE_INFO 0 //����ע�����:
#define LOG_USING_FUNC_INFO 1 //����ע�����:
#define LOG_USING_LINE_INFO 1 //����ע�����:
/*=======*/
//���+��ģ������֤
//#elif (LOG_USING_COLOR && LOG_USING_TIME_INFO && !LOG_USING_LEVEL && !LOG_USING_FILE_INFO && LOG_USING_FUNC_INFO && LOG_USING_LINE_INFO)
#pragma endregion //������־ģ��
/*======================================================================================*/

typedef enum {
    u8_bit0 = 0x01,
    u8_bit1 = 0x02,
    u8_bit2 = 0x04,
    u8_bit3 = 0x08,
    u8_bit4 = 0x10,
    u8_bit5 = 0x20,
    u8_bit6 = 0x40,
    u8_bit7 = 0x80,
} u8_bit_enum;

typedef enum {
    u16_bit0 = 0x0001,
    u16_bit1 = 0x0002,
    u16_bit2 = 0x0004,
    u16_bit3 = 0x0008,
    u16_bit4 = 0x0010,
    u16_bit5 = 0x0020,
    u16_bit6 = 0x0040,
    u16_bit7 = 0x0080,

    u16_bit8 = 0x0100,
    u16_bit9 = 0x0200,
    u16_bitA = 0x0400,
    u16_bitB = 0x0800,
    u16_bitC = 0x1000,
    u16_bitD = 0x2000,
    u16_bitE = 0x4000,
    u16_bitF = 0x8000,
} u16_bit_enum;

typedef enum {
    false = 0,
    true = !false,
} bool;

#ifndef rt_uint8_t
#define rt_uint8_t u8
#endif

#ifndef uint8_t
#define uint8_t u8
#endif

typedef enum {
    RT_EOK = 0,
    RT_ERROR,
    RT_ETIMEOUT,
    RT_EFULL,
    RT_EEMPTY,
    RT_ENOMEM,
    RT_ENOSYS,
    RT_EBUSY,
    RT_EIO,
    RT_EINTR,
    RT_EINVAL,
} rt_err_t;

#pragma region //���Կ��غ궨��
/*======================================================================================*/
//2022-08-22
//�����ڼ䣺���÷�����
#ifdef TR_DEBUG
//���԰汾
#define DEBUG_EN_BEEP 0
#else
//TR_TEST//���԰汾
//TR_PROD//�����汾
#define DEBUG_EN_BEEP 1
#endif
/*==========================*/
//�����ڼ䣺���ÿ��Ź�
#ifdef TR_DEBUG
//���԰汾
#define DEBUG_EN_GPS_UN_dingWei_printf 0 //����ע�����:
#else
//TR_TEST//���԰汾
//TR_PROD//�����汾
#define DEBUG_EN_GPS_UN_dingWei_printf 0
#endif
/*==========================*/
//�����ڼ䣺���ÿ��Ź�
#ifdef TR_DEBUG
//���԰汾
#define DEBUG_EN_WATCH_DOG 0
#else
//TR_TEST//���԰汾
//TR_PROD//�����汾
#define DEBUG_EN_WATCH_DOG 1
#endif
/*==========================*/
//rcd_3c�ǹ����ʱ��ʹ�õġ�
//u1_m.cur.b.rcd_3c
#if defined(TR_DEBUG) || defined(TR_TEST)
#define DEBUG_EN_RCD_3C 0
#else
//TR_PROD//�����汾
#define DEBUG_EN_RCD_3C 1
#endif
/*======================================================================================*/
#pragma endregion //���Կ��غ궨��

/*======================================================================================*/
//>>>>>>>>>>>>> ����ģ�鿪�� bg<<<<<<<<<<<<<<
/*======================================================================================*/
//����ģ�飺���Ų����⣺��λRK���ϵ�RK������IO������־��
#define MODULE_USING_pwr_rst_rk_logSave 1 //����ע�����://����/���ñ�ģ��
#define MODULE_DEBUG_pwr_rst_rk_logSave 0 //����ע�����://����/���ò��ֵ��Թ���//�����������Թ���
//>>>>>>>>>>
//����ģ�飺���Ų����⣺��
#define MODULE_USING_self_chk 1 //����ע�����://����/���ñ�ģ��
#define MODULE_DEBUG_self_chk 1 //����ע�����://����/���ò��ֵ��Թ���//�����������Թ���
//>>>>>>>>>>
//����ģ�飺���Ų����⣺��
#define MODULE_USING_lsm6ts3trc 0 //����ע�����://����/���ñ�ģ��
#define MODULE_DEBUG_lsm6ts3trc 0 //����ע�����://����/���ò��ֵ��Թ���//�����������Թ���
//>>>>>>>>>>
//����ģ�飺���Ų����⣺��
#define MODULE_DEBUG_ic_card 0 //����ע�����://����/���ò��ֵ��Թ���//�����������Թ���
//>>>>>>>>>>
//����ģ�飺���Ų����⣺��
#define MODULE_DEBUG_ota 1 //����ע�����://����/���ò��ֵ��Թ���//�����������Թ���
//>>>>>>>>>>

#ifdef TR_DEBUG
#define USE_DEBUG_FUNCTION
//#undef USE_DEBUG_FUNCTION
#define TIC_USE_DEBUG_FUNCTION 5
#endif
/*======================================================================================*/
//>>>>>>>>>>>>> ����ģ�鿪�� end<<<<<<<<<<<<<<
/*======================================================================================*/

// typedef enum {
//     DISABLE = 0,
//     ENABLE = !DISABLE
// } FunctionalState;

#endif /* __DEFINE_H__ */
