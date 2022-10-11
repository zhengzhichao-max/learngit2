
#include <string.h>
#include "fifo.h"

void FIFO_Init(FIFO_TypeDef *pFifo, uint8_t *pBaseAddr, uint16_t len)
{
	pFifo->length = len;
	pFifo->occupy = 0;
	pFifo->pArray  = pBaseAddr;
	pFifo->pLimit  = pBaseAddr + len;
	pFifo->pWr     = pFifo->pArray;
	pFifo->pRd     = pFifo->pArray;
}

void FIFO_Reset(FIFO_TypeDef *pFifo)
{
	pFifo->occupy = 0;
	pFifo->pRd = pFifo->pArray;
	pFifo->pWr = pFifo->pArray;
}

uint8_t FIFO_CheckEmpty(FIFO_TypeDef *pFifo)
{
	return (0 == pFifo->occupy) ? 1 : 0;
}

uint8_t FIFO_CheckBaseNull(FIFO_TypeDef *pFifo)
{
	return (NULL == pFifo->pArray) ? 1 : 0;
}

uint8_t FIFO_CheckFull(FIFO_TypeDef *pFifo)
{
	return (pFifo->occupy == pFifo->length) ? 1 : 0;
}

uint16_t FIFO_GetFree(FIFO_TypeDef *pFifo)
{
	return (pFifo->length - pFifo->occupy);
}

uint16_t FIFO_GetAlloc(FIFO_TypeDef *pFifo)
{
	return pFifo->occupy;
}

uint8_t FIFO_Get(FIFO_TypeDef *pFifo, uint8_t *pData)
{
	if ((NULL == pFifo) || (NULL == pData) || (0 == pFifo->occupy)) 
	{
		return 0;
	}
	
	*pData = *pFifo->pRd++;
	if (pFifo->pRd >= pFifo->pLimit)
	{
		pFifo->pRd = pFifo->pArray;
	}
	
	__disable_irq();
	pFifo->occupy--;
	__enable_irq();
	
	return 1;
}

uint16_t FIFO_Read(FIFO_TypeDef *pFifo, uint8_t *pData, uint16_t len)
{
	uint16_t i = 0;
	
	if ((0 == pFifo->occupy) || (0 == len) || (NULL == pData) || (NULL == pFifo))
	{
		return 0;
	}
	
	for (i = 0; i < len; i++)
	{
		if (pFifo->occupy > 0)
		{
			*pData++ = *pFifo->pRd++;
			if (pFifo->pRd >= pFifo->pLimit)
			{
				pFifo->pRd = pFifo->pArray;
			}
			__disable_irq();
			pFifo->occupy--;
			__enable_irq();
		}
		else
		{
			break;
		}
	}
	
	return i;
}

uint8_t FIFO_Put(FIFO_TypeDef *pFifo, uint8_t data)
{
	if ((NULL == pFifo) || (pFifo->occupy >= pFifo->length))
	{
		return 0;
	}
	
	*pFifo->pWr++ = data;
	if (pFifo->pWr >= pFifo->pLimit) 
	{
		pFifo->pWr = pFifo->pArray;
	}
	
	__disable_irq();
	pFifo->occupy++;
	__enable_irq();
	
	return 1;
}


//UserToPMABufferCopy--->SetEPTxCount--->SetEPTxValid
uint16_t FIFO_Write(FIFO_TypeDef *pFifo, const uint8_t *pData, uint16_t len)
{
	uint16_t i = 0;
	
	if ((0 == len) || (NULL == pData) || (NULL == pFifo))
	{
		return 0;
	}
	
	for (i = 0; i < len; i++)
	{
		if ((pFifo->length - pFifo->occupy) > 0)
		{
			*pFifo->pWr++ = *pData++;
			if (pFifo->pWr >= pFifo->pLimit) 
			{
				pFifo->pWr = pFifo->pArray;
			}
			
			__disable_irq();
			pFifo->occupy++;
			__enable_irq();
		}
		else
		{
			break;
		}
	}
	
	return i;
}


