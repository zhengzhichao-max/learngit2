//VCP_DataTx         -- 发送中断函数
//s_USB_SndTotalLen  -- 是否有有效发送数据  =0；出问题；USB中断也停止了 2020-03
//occupy;		//操作的数据长度  FIFO_Write 失败了有调用   VCP_DataRx 没有调用
//问题1；经常数据发送完321个字节进入DCD_HandleSof_ISR；
//问题2：重新复位USB不管用；不能进入中断
//问题3：重新初始化是否需要模拟热拔插
#include <string.h>

#include "usbd_core.h"
#include "usbd_cdc_core.h"
#include "usbd_conf.h"
#include "usbd_desc.h"
#include "usb_dcd_int.h"

#include "bsp_usb.h"
#include "fifo.h"
#include "jt808_type.h"
//#include "pub.h"

USB_OTG_CORE_HANDLE    USB_OTG_dev;


u32  usb_send_interrupt_cnt = 0;
static FIFO_TypeDef		s_USB_FIFO_Rx;
static FIFO_TypeDef		s_USB_FIFO_Tx;

static uint8_t			s_USB_RX_BUF[USB_RX_BUF_MAX_LEN] = {0};
static uint8_t			s_USB_TX_BUF[USB_TX_BUF_MAX_LEN] = {0};

static volatile BSP_USB_INT_E	s_BSP_USB_Int[BSP_USB_INT_MAX_NUM];
static volatile uint32_t		s_BSP_USB_IntCnt = 0;

static uint32_t			s_USB_RcvTotalLen = 0;
static uint32_t			s_USB_SndTotalLen = 0;

uint32_t		s_TIMER_Delay_ms = 0;



void BSP_TIMER_Delay(__IO uint32_t nDelay_ms)
{
	uint32_t	nTmpVal = 0;

	uint32_t	nLapseCnt = 0;
	
	s_TIMER_Delay_ms = nDelay_ms;
	
	while (s_TIMER_Delay_ms != 0)
	{
		if (s_TIMER_Delay_ms != nTmpVal)
		{
			s_TIMER_Delay_ms = nTmpVal;

			nLapseCnt++;

			if (nLapseCnt > 20)
			{
				nLapseCnt = 0;

				//BSP_WDT_ForceWDG();
			}
		}
	}
}



static uint16_t VCP_Init(void)
{
	return USBD_OK;
}


void BSP_NVIC_USB_IRQ(FunctionalState state)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = state;
	NVIC_Init(&NVIC_InitStructure);
}

static uint16_t VCP_DeInit(void)
{
	return USBD_OK;
}

static uint16_t VCP_Ctrl(uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
	return USBD_OK;
}

static uint16_t VCP_DataRx(uint8_t *pRxData, uint32_t dataLen)
{
	if ((dataLen > 0) && (pRxData != NULL))
	{
		FIFO_Write(&s_USB_FIFO_Rx, pRxData, dataLen);

		s_USB_RcvTotalLen += dataLen;
	}
	
	return 0;
}
//xh step 2
static uint16_t VCP_DataTx(uint8_t *pTxData, uint32_t dataLen)
{
	uint16_t	nTxLen = 0;
	
	if ((dataLen > 0) && (pTxData != NULL))
	{
		nTxLen = FIFO_Read(&s_USB_FIFO_Tx, pTxData, dataLen);
		usb_send_interrupt_cnt++;
		s_USB_SndTotalLen += nTxLen;
	}
	
	return nTxLen;
}


CDC_IF_Prop_TypeDef VCP_fops = 
{
	VCP_Init,
	VCP_DeInit,
	VCP_Ctrl,
	VCP_DataTx,
	VCP_DataRx
};

static void USB_UsrCb_Init(void)
{
	
}

static void USB_UsrCb_DeviceReset(uint8_t speed)
{
	
}

static void USB_UsrCb_DeviceConfigured(void)
{
	
}

static void USB_UsrCb_DeviceSuspended(void)
{
	
}

static void USB_UsrCb_DeviceResumed(void)
{
	
}


static USBD_Usr_cb_TypeDef s_USB_Usr_cb = 
{
	USB_UsrCb_Init,
	USB_UsrCb_DeviceReset,
	USB_UsrCb_DeviceConfigured,
	USB_UsrCb_DeviceSuspended,
	USB_UsrCb_DeviceResumed,
};

static void USB_GpioInit(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/*USB_DM: PA11, USB_DP: PA12*/
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void USB_GpioDeInit(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/*USB_DM: PA11, USB_DP: PA12*/
	GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void BSP_USB_Init(void)
{
	USB_GpioInit();

	RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
	
	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &s_USB_Usr_cb);
	
	//BSP_WDT_FeedWDG();
	
	BSP_NVIC_USB_IRQ(ENABLE);
	
	FIFO_Init(&s_USB_FIFO_Rx, s_USB_RX_BUF, USB_RX_BUF_MAX_LEN);

	FIFO_Init(&s_USB_FIFO_Tx, s_USB_TX_BUF, USB_TX_BUF_MAX_LEN);

	s_USB_RcvTotalLen = 0;
	s_USB_SndTotalLen = 0;
}

void BSP_USB_DeInit(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, DISABLE);

	BSP_NVIC_USB_IRQ(DISABLE);

	USB_GpioDeInit();

	FIFO_Reset(&s_USB_FIFO_Rx);

	FIFO_Reset(&s_USB_FIFO_Tx);
}

void BSP_USB_IRQHandle(void)
{
	USBD_OTG_ISR_Handler(&USB_OTG_dev);
}





//UserToPMABufferCopy--->SetEPTxCount--->SetEPTxValid
uint32_t BSP_USB_DataWrite(uint8_t *pTxData, uint16_t dataLen)
{
	
	if ((dataLen > 0) && (pTxData != NULL))
	{
		FIFO_Write(&s_USB_FIFO_Tx, pTxData, dataLen);
	}
	
	return 0;
}

uint32_t BSP_USB_DataRead(uint8_t *pRxData, uint16_t dataLen)
{
	uint32_t	nReadLen = 0;

	if ((dataLen > 0) && (pRxData != NULL))
	{
		nReadLen = (uint32_t)FIFO_Read(&s_USB_FIFO_Rx, pRxData, dataLen);
	}

	return nReadLen;
}

void BSP_USB_SAVE_INT(BSP_USB_INT_E CurInt)
{
	if (s_BSP_USB_IntCnt < (BSP_USB_INT_MAX_NUM - 1))   //USB 调试 进不了
	{
		s_BSP_USB_Int[s_BSP_USB_IntCnt] = CurInt;

		s_BSP_USB_IntCnt++;
	}
}

/*检测USB底层 <接收> 缓存情况.
  Return: 0---未接收到新数据
          1---有接收到新数据
 */
uint8_t BSP_USB_CheckRXBuf(void)
{
	uint8_t		nReturn = 0x0;

	static uint32_t		s_USB_LastRcvLen = 0;

	if (s_USB_LastRcvLen != s_USB_RcvTotalLen)
	{
		s_USB_LastRcvLen = s_USB_RcvTotalLen;

		nReturn = 0x1;
	}

	return nReturn;
}

/*检测USB底层 <发送> 缓存情况.
  Return: 0---未发送数据
          1---有发送新数据
 */
uint8_t BSP_USB_CheckTXBuf(void)
{
	uint8_t		nReturn = 0x0;

	static uint32_t		s_USB_LastSndLen = 0;

	if (s_USB_LastSndLen != s_USB_SndTotalLen)
	{
		s_USB_LastSndLen = s_USB_SndTotalLen;

		nReturn = 0x1;
	}
	
	return nReturn;
}

/*清楚USB的 <接收> 和 <发送> 缓存*/
void BSP_USB_DataReset(void)
{
	FIFO_Reset(&s_USB_FIFO_Rx);
	
	FIFO_Reset(&s_USB_FIFO_Tx);
}


u8 Get_Usb_Send_Len(void)
{
	if( s_USB_FIFO_Tx.occupy == 0 )
		return 1;
	else
		return 0;
}

u32 Get_Occupy_Len(void)
{
	return s_USB_FIFO_Tx.occupy;
}

u32 Get_UsbSend_Len(void)
{
	return s_USB_SndTotalLen;
}


void clear_usb_busy(void)
{
}	


