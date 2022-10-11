#ifndef __TEST_H__
#define __TEST_H__

/*======================================================================================*/
/*=======================*/
//测试
#define TEST_MONITOR 0
/*=======================*/
//测试
#define TEST_MCU_FLASH_20220921 0
/*=======================*/
//测试
#if (LOG_USING_LOGGER)
#define TEST_LOGGER_MODULE 0
#else
#define TEST_LOGGER_MODULE 0 //不要动这里
#endif
/*======================================================================================*/

void test(void);

#endif /* __TEST_H__ */
