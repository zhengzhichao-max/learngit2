#ifndef __TEST_H__
#define __TEST_H__

/*======================================================================================*/
/*=======================*/
//����
#define TEST_MONITOR 0
/*=======================*/
//����
#define TEST_MCU_FLASH_20220921 0
/*=======================*/
//����
#if (LOG_USING_LOGGER)
#define TEST_LOGGER_MODULE 0
#else
#define TEST_LOGGER_MODULE 0 //��Ҫ������
#endif
/*======================================================================================*/

void test(void);

#endif /* __TEST_H__ */