/**
  ******************************************************************************
  * @file    spi_flash.c
  * @author  TRWY_TEAM
  * @Email     
  * @version V2.0.0
  * @date    2013-12-01
  * @brief  spi flash 数据管理
  ******************************************************************************
  * @attention
  ******************************************************************************
*/  
#include "include_all.h"

/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

/* Private typedef -----------------------------------------------------------*/
#define SPI_FLASH_PAGESIZE    256
#define    f25_sec_lgth        ((u16)4096)    //扇区大小
#define    f25_page_lgth       ((u16)256)     //页大小

#if 1
/****************************************************************************
* 名称:    SPI_FLASH_Init ()
* 功能：初始化
* 入口参数：无                         
* 出口参数：无
****************************************************************************/
void SPI_FLASH_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	//RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SPI1,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;          //澶嶇敤鍔熻兘锛堟帹鎸斤級杈撳嚭  SPI1
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

#if 0	
	GPIO_InitStructure.GPIO_Pin = SPI_FLASH_SCK | SPI_FLASH_MOSI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SPI_FLASH_GPIO, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =SPI_FLASH_MISO ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(SPI_FLASH_GPIO, &GPIO_InitStructure);
#endif 

	GPIO_InitStructure.GPIO_Pin = SPI_FLASH_CS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_FLASH_CS_GPIO, &GPIO_InitStructure);

	SPI_FLASH_CS_HIGH();

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	 
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;	 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	 
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	
	SPI_InitStructure.SPI_CRCPolynomial = 7;			


	SPI_Init(SPI1, &SPI_InitStructure);

#if 0
	GPIO_InitStructure.GPIO_Pin = SPI_FLASH_WP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_FLASH_WP_GPIO, &GPIO_InitStructure);
	SPI_FLASH_WP_HIGH();
#endif

	SPI_Cmd(SPI1, ENABLE);
	
}

/*******************************************************************************
* Function Name  : SPI_GetFlagStatus
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
FlagStatus SPI_GetFlagStatus(SPI_TypeDef* SPIx, u16 SPI_FLAG)
{
	FlagStatus bitstatus = RESET;

	assert(IS_SPI_GET_FLAG(SPI_FLAG));
	if ((SPIx->SR & SPI_FLAG) != (u16)RESET)
	{bitstatus = SET;}
	else
	{bitstatus = RESET;}

	return  bitstatus;
}

/*******************************************************************************
* Function Name  : SPI_SendData
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void SPI_SendData(SPI_TypeDef* SPIx, u16 Data)
{  
	SPIx->DR = Data;
}

/*******************************************************************************
* Function Name  : spi_flash_erase(4K)
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void spi_flash_erase(u32 SectorAddr)
{ 
	SPI_FLASH_WriteEnable();
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(CMD_EBMA);
	SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((SectorAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(SectorAddr & 0xFF);
	SPI_FLASH_CS_HIGH();
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : spi_flash_erase
* Description    : Erases the specified FLASH sector.
* Input          : SectorAddr: address of the sector to erase.
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_BulkErase(void)
{
	/* Send write enable instruction */
	SPI_FLASH_WriteEnable();

	/* Bulk Erase */
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send Bulk Erase instruction  */
	SPI_FLASH_SendByte(CMD_BE);
	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_PageWrite
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{  
	//static u8 uWrData = 0;

	SPI_FLASH_WriteEnable();
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(CMD_WRITE);
	SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((WriteAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte(WriteAddr & 0xFF);
	while(NumByteToWrite--) 
	{    
		//uWrData = *pBuffer;			
		SPI_FLASH_SendByte(*pBuffer);
		pBuffer++;
	}  
	SPI_FLASH_CS_HIGH();
	SPI_FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : SPI_ReceiveData
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 SPI_ReceiveData(SPI_TypeDef* SPIx)
{  
	return SPIx->DR;
}



/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte 
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
u8 SPI_FLASH_SendByte(u8 byte)
{

	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
	SPI_SendData(SPI1, byte);
	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET);
	return SPI_ReceiveData(SPI1);
//#else
//	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
//	SPI_SendData(SPI1, byte);
//	while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET);
//	return SPI_ReceiveData(SPI1);
//#endif	
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WriteEnable
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WriteEnable(void)
{
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(CMD_WREN);
	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
* Function Name  : SPI_FLASH_WaitForWriteEnd
* Description    : Polls the status of the Write In Progress (WIP) flag in the  
*                  FLASH's status  register  and  loop  until write  opertaion
*                  has completed.  
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
	u8 FLASH_Status = 0;
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte(CMD_RDSR);
	do
	{
		FLASH_Status = SPI_FLASH_SendByte(Dummy_Byte);
	} while((FLASH_Status & WIP_Flag) == SET);

	SPI_FLASH_CS_HIGH();
}

/*******************************************************************************
* Function Name  : spi_flash_read
* Description    : Reads a block of data from the FLASH.
* Input          : - pBuffer : pointer to the buffer that receives the data read 
*                    from the FLASH.
*                  - ReadAddr : FLASH's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void spi_flash_read(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{ 
//	static u8 uRdData = 0;
	if(pBuffer == NULL ||ReadAddr>ADD_MAX_LIMIT || NumByteToRead>4096)
	{
		return;
	}

	wdr();
	SPI_FLASH_CS_LOW();
	SPI_FLASH_SendByte( CMD_READ);
	SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	SPI_FLASH_SendByte((ReadAddr & 0xFF00) >> 8);
	SPI_FLASH_SendByte( ReadAddr & 0xFF);

	while(NumByteToRead--) 
	{    
		*pBuffer = SPI_FLASH_SendByte(Dummy_Byte);
		//uRdData = *pBuffer;
		pBuffer++;
	}  
	SPI_FLASH_CS_HIGH();
}


/*******************************************************************************
* Function Name  : spi_flash_write
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be 
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void spi_flash_write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	static u8 NumOfPage = 0, NumOfSingle = 0;
	u8 Addr = 0, count = 0, temp = 0;

	if(pBuffer == NULL ||WriteAddr>ADD_MAX_LIMIT || NumByteToWrite>4096)
	{
		return;
	}

	wdr();					//喂狗
	Addr = WriteAddr % SPI_FLASH_PAGESIZE;
	count = SPI_FLASH_PAGESIZE - Addr;
	NumOfPage =  NumByteToWrite / SPI_FLASH_PAGESIZE;
	NumOfSingle = NumByteToWrite % SPI_FLASH_PAGESIZE;

	if (Addr == 0) /* WriteAddr is SPI_FLASH_PAGESIZE aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PAGESIZE */
		{
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
		}
		else /* NumByteToWrite > SPI_FLASH_PAGESIZE */
		{
			while (NumOfPage--)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PAGESIZE);
				WriteAddr +=  SPI_FLASH_PAGESIZE;
				pBuffer += SPI_FLASH_PAGESIZE;
			}
			SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
		}
	}
	else /* WriteAddr is not SPI_FLASH_PAGESIZE aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PAGESIZE */
		{
			if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PAGESIZE */
			{
				temp = NumOfSingle - count;

				SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
				WriteAddr +=  count;
				pBuffer += count;

				SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
			}
			else
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
			}
		}
		else /* NumByteToWrite > SPI_FLASH_PAGESIZE */
		{
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PAGESIZE;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PAGESIZE;

			SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
			WriteAddr +=	count;
			pBuffer += count;

			while (NumOfPage--)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PAGESIZE);
				WriteAddr +=  SPI_FLASH_PAGESIZE;
				pBuffer += SPI_FLASH_PAGESIZE;
			}

			if (NumOfSingle != 0)
			{
				SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
			}
		}
	}
}
#endif


/*****************************************************************************
**	  25系列芯片直接写操作，里面考虑了擦除扇区自动保存数据，跨扇区写操作，直接调用该函数即可
**	  此处应注意看门狗复位
**	  注意:调用该函数时，程序的栈会多至少一个扇区大小的空间!!!
*****************************************************************************/
void flash25_program_auto_save(u32 startaddr, u8 *buffer, u32 length)
{
	u32 temp_addr;
	u16  temp_count, i, n;
	u8 tp_save_buffer[f25_sec_lgth];

	if(buffer == NULL ||startaddr>=ADD_MAX_LIMIT)
	{
		return;
	}

	temp_addr = (startaddr - (startaddr%f25_sec_lgth)); 	 //计算该扇区的首地址
	temp_count = (f25_sec_lgth - (startaddr%f25_sec_lgth));   //计算在当前扇区下，可以写入的字节数

	while (length)
	{
		if (temp_count > length)
		{
			n = length;
		}
		else
		{
			n = temp_count;
		}

		if (n != f25_sec_lgth)	  //非整扇区操作
		{
			spi_flash_read(tp_save_buffer, temp_addr, f25_sec_lgth);	//读出该扇区的数据，做备份
		}
		spi_flash_erase(temp_addr);   //扇区擦除

		if (n != f25_sec_lgth)	  //非整扇区操作
		{
			for (i=0; i<n; i++)
			{
				tp_save_buffer[(f25_sec_lgth-temp_count)+i] = *(buffer++);		//修改数据
			}

			spi_flash_write(tp_save_buffer, temp_addr, f25_sec_lgth);	//将数据写入到扇区中
		}
		else
		{
			spi_flash_write(buffer, temp_addr, f25_sec_lgth);	//将数据写入到扇区中
			buffer += f25_sec_lgth;    //修改写入目标指针
		}

		temp_count = f25_sec_lgth;
		temp_addr += f25_sec_lgth;
		length -= n;
	}
}



