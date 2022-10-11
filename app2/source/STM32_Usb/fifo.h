#ifndef __FIFO_H
#define __FIFO_H

#include "stm32f10x.h"

typedef struct
{
	uint16_t length;
	uint16_t occupy;		//操作的数据长度
	uint8_t *pArray;
	uint8_t *pLimit;		
	uint8_t *pWr;
	uint8_t *pRd;
} FIFO_TypeDef;

void FIFO_Init(FIFO_TypeDef *pFifo, uint8_t *pBaseAddr, uint16_t len);

void FIFO_Reset(FIFO_TypeDef *pFifo);

uint16_t FIFO_GetFree(FIFO_TypeDef *pFifo);

uint16_t FIFO_GetAlloc(FIFO_TypeDef *pFifo);

uint8_t FIFO_CheckEmpty(FIFO_TypeDef *pFifo);

uint8_t FIFO_CheckBaseNull(FIFO_TypeDef *pFifo);

uint8_t FIFO_CheckFull(FIFO_TypeDef *pFifo);

uint8_t FIFO_Get(FIFO_TypeDef *pFifo, uint8_t *pData);

uint16_t FIFO_Read(FIFO_TypeDef *pFifo, uint8_t *pData, uint16_t len);

uint8_t FIFO_Put(FIFO_TypeDef *pFifo, uint8_t data);

uint16_t FIFO_Write(FIFO_TypeDef *pFifo, const uint8_t *pData, uint16_t len);

#endif


