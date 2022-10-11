/**
  ******************************************************************************
  * @file    usbh_usr.h
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   Header file for usbh_usr.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USH_USR_H__
#define __USH_USR_H__

/* Includes ------------------------------------------------------------------*/
#include "ff.h"
#include "usbh_core.h"
#include "usb_conf.h"
#include <stdio.h>
#include "usbh_msc_core.h"

/** @addtogroup USBH_USER
  * @{
  */

/** @addtogroup USBH_MSC_DEMO_USER_CALLBACKS
  * @{
  */

/** @defgroup USBH_USR
  * @brief This file is the Header file for usbh_usr.c
  * @{
  */

extern USB_OTG_CORE_HANDLE      USB_OTG_Core;
extern USBH_HOST                USB_Host;

extern  FRESULT Explore_Disk(char * path, uint8_t recu_level);

/** @defgroup USBH_USR_Exported_Types
  * @{
  */


extern  USBH_Usr_cb_TypeDef USR_cb;



/**
  * @}
  */



/** @defgroup USBH_USR_Exported_Defines
  * @{
  */
/* State Machine for the USBH_USR_ApplicationState */
#define USH_USR_FS_INIT       	0
#define USH_USR_FS_READLIST   	1
#define USH_USR_FS_WRITEFILE  	2
#define USH_USR_FS_READFILE   	3
#define USH_USR_FS_DRAW       	4
#define USH_USR_FS_IAP        	5
#define USH_USR_FS_FONT   	  	6     	//字库升级步骤
#define USH_USR_FS_APP   	  	7      	//应用程序升级步骤
#define USH_USR_FS_CFG        	8     	//参数配置步骤
#define USH_USR_FS_READ_RCD    	9    	//读取行驶记录仪数据步骤
#define USH_USR_FS_IC_CREATION 	10   	//IC卡开卡步骤
#define USH_AUTO_INIT           11      //自动初始参数步骤
#define USH_AUTO_ID_CFG         12      //自动ID配置 2018-4-2
#define USH_USR_FS_NULL   	   	13

/**
  * @}
  */

/** @defgroup USBH_USR_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_USR_Exported_Variables
  * @{
  */
extern  uint8_t USBH_USR_ApplicationState ;
/**
  * @}
  */

/** @defgroup USBH_USR_Exported_FunctionsPrototype
  * @{
  */
void USBH_USR_ApplicationSelected(void);
void USBH_USR_Init(void);
void USBH_USR_DeInit(void);
void USBH_USR_DeviceAttached(void);
void USBH_USR_ResetDevice(void);
void USBH_USR_DeviceDisconnected(void);
void USBH_USR_OverCurrentDetected(void);
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed);
void USBH_USR_Device_DescAvailable(void *);
void USBH_USR_DeviceAddressAssigned(void);
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
        USBH_InterfaceDesc_TypeDef * itfDesc,
        USBH_EpDesc_TypeDef * epDesc);
void USBH_USR_Manufacturer_String(void *);
void USBH_USR_Product_String(void *);
void USBH_USR_SerialNum_String(void *);
void USBH_USR_EnumerationDone(void);
USBH_USR_Status USBH_USR_UserInput(void);
void USBH_USR_DeInit(void);
void USBH_USR_DeviceNotSupported(void);
void USBH_USR_UnrecoveredError(void);
int USBH_USR_MSC_Application(void);


extern u8 USBH_UDISK_Read(u8 * buf, u32 sector, u32 cnt);
extern u8 USBH_UDISK_Write(u8 * buf, u32 sector, u32 cnt);

/**
  * @}
  */

#endif /*__USH_USR_H__*/

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

