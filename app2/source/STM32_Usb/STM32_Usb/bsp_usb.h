

#ifndef __BSP_USB_H__
#define	__BSP_USB_H__

#include "stm32f10x.h"
#include "jt808_type.h"

typedef enum __tag_BSP_USB_INT_E
{
	USB_INT_NONE,
	USB_GINTSTS_outepintr,
	USB_GINTSTS_inepint,
	USB_GINTSTS_modemismatch,
	USB_GINTSTS_wkupintr,
	USB_GINTSTS_usbsuspend,
	USB_GINTSTS_sofintr,
	USB_GINTSTS_rxstsqlvl,
	USB_GINTSTS_usbreset,
	USB_GINTSTS_enumdone,
	USB_GINTSTS_incomplisoin,
	USB_GINTSTS_incomplisoout,

	USB_APP_FOPS_INIT,
	USB_APP_FOPS_DEINIT,
	USB_APP_FOPS_CTRL_1,
	USB_APP_FOPS_CTRL_2,
	USB_APP_FOPS_CTRL_3,
	USB_APP_FOPS_RX,
	USB_APP_FOPS_TX,
	USB_APP_FOPS_SOF_TX,

	USB_INT_MAX
}BSP_USB_INT_E;




extern uint32_t			s_TIMER_Delay_ms;
extern u32  usb_send_interrupt_cnt;

void BSP_USB_Init(void);

void BSP_USB_DeInit(void);

void BSP_USB_IRQHandle(void);

uint32_t BSP_USB_DataWrite(uint8_t *pTxData, uint16_t dataLen);

uint32_t BSP_USB_DataRead(uint8_t *pRxData, uint16_t dataLen);

void BSP_USB_SAVE_INT(BSP_USB_INT_E CurInt);


/*检测USB底层 <接收> 缓存情况.
  Return: 0---未接收到新数据
          1---有接收到新数据
 */
uint8_t BSP_USB_CheckRXBuf(void);

/*检测USB底层 <发送> 缓存情况.
  Return: 0---未发送数据
          1---有发送新数据
 */
uint8_t BSP_USB_CheckTXBuf(void);

/*清楚USB的 <接收> 和 <发送> 缓存*/
void BSP_USB_DataReset(void);

void BSP_TIMER_Delay(__IO uint32_t nDelay_ms);

u8 Get_Usb_Send_Len(void);

u32 Get_Occupy_Len(void);
u32 Get_UsbSend_Len(void);
void clear_usb_busy(void);
#endif

