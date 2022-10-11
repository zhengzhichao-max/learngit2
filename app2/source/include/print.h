/**
  ******************************************************************************
  * @file    print.h 
  * @author TRWY_TEAM
  * @Email    
  * @version V2.0.0
  * @date    2013-12-01
  * @brief   2012 行驶记录仪功能打印
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#ifndef _PRINT_H_
#define _PRINT_H_

#define PRINT_TO_LCD   		0x8402

#define RCD_PRINT_SPEED_MAX_SIZE	15
#define  RCD_PRINT_FATCNTS_MAX_SIZE	2


extern void print03_rcd_infomation(void);
extern void print12_rcd_infomation(void);

#endif

