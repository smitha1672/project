#include "ext_flash_driver.h"

#if ( configSPI_FLASH == 1 )
//___________________________________________________________________________________________

#define sFLASH_SPI SPI1
#define sFLASH_CMD_WRITE          0x02  /*!< Write to Memory instruction */
#define sFLASH_CMD_WRSR           0x01  /*!< Write Status Register instruction */
#define sFLASH_CMD_WREN           0x06  /*!< Write enable instruction */
#define sFLASH_CMD_READ           0x03  /*!< Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /*!< Read Status Register instruction  */
#define sFLASH_CMD_RDID           0x9F  /*!< Read identification */
#define sFLASH_CMD_SE             0xD8  /*!< Sector Erase instruction */
#define sFLASH_CMD_CE             0xC7  /*!< Bulk Erase instruction */ /*chip erase*/

#define sFLASH_WIP_FLAG           0x01  /*!< Write In Progress (WIP) flag */

#define sFLASH_DUMMY_BYTE         0x00
#define sFLASH_SPI_PAGESIZE       0x100

#define sFLASH_M25P128_ID         0x202018
#define sFLASH_M25P64_ID          0x202017


#if defined ( FREE_RTOS )
#define sFLASH_MUTEX_LOCK        SPI_mutex_lock();
#define sFLASH_MUTEX_UNLOCK      SPI_mutex_unlock();
#endif 

#define sFLASH_CS_LOW()       SPI_selectChip( 1 )
#define sFLASH_CS_HIGH()      SPI_unselectChip( 1 )

#define W25Q80BV_PAGE_LEN  256U
#define W25Q80BV_NUM_PAGES 4096U
#define W25Q80BV_NUM_BYTES 1048576U
#define W25Q80BV_WRITE_ENABLE 0x06
#define W25Q80BV_CHIP_ERASE 0xC7
#define W25Q80BV_READ_STATUS1 0x05
#define W25Q80BV_PAGE_PROGRAM 0x02
#define W25Q80BV_DEVICE_ID 0xAB
#define W25Q80BV_UNIQUE_ID 0x4B
#define W25Q80BV_STATUS_BUSY 0x01
#define W25Q80BV_DEVICE_ID_RES 0x13 /* Expected device_id for W25Q80BV */


//___________________________________________________________________________________________
uint8_t device_id;
//___________________________________________________________________________________________

static void sFLASH_WriteEnable(void);

static void sFLASH_WaitForWriteEnd(void);

//static
    uint32_t sFLASH_ReadID(void);


//___________________________________________________________________________________
void sFLASH_EraseSector(uint32_t SectorAddr)
{

#if defined ( FREE_RTOS )
  if (SPI_mutex_lock() == FALSE)
  {
    return;
  }
#endif   

  /*!< Send write enable instruction */
  sFLASH_WriteEnable();

  /*!< Sector Erase */
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send Sector Erase instruction */
  sFLASH_SendByte(sFLASH_CMD_SE);
  /*!< Send SectorAddr high nibble address byte */
  sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /*!< Send SectorAddr medium nibble address byte */
  sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /*!< Send SectorAddr low nibble address byte */
  sFLASH_SendByte(SectorAddr & 0xFF);
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();

#if defined  ( FREE_RTOS )  
  SPI_mutex_unlock();
#endif 

}

/**
  * @brief  Erases the entire FLASH.
  * @param  None
  * @retval None
  */
void sFLASH_EraseChip(void)
{

#if defined ( FREE_RTOS )
    if (SPI_mutex_lock() == FALSE)
    {
      return;
    }
#endif 	

  /*!< Send write enable instruction */
  sFLASH_WriteEnable();

  /*!< Bulk Erase */
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send Bulk Erase instruction  */
  sFLASH_SendByte(sFLASH_CMD_CE);
  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();

#if defined ( FREE_RTOS )  
  SPI_mutex_unlock();
#endif 

}

/**
  * @brief  Writes more than one byte to the FLASH with a single WRITE cycle 
  *         (Page WRITE sequence).
  * @note   The number of byte can't exceed the FLASH page size.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be equal
  *         or less than "sFLASH_PAGESIZE" value.
  * @retval None
  */
void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
#if defined ( FREE_RTOS )  
    if (SPI_mutex_lock() == FALSE)
    {
      return;
    }
#endif 	

  /*!< Enable the write access to the FLASH */
  sFLASH_WriteEnable();

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send "Write to Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /*!< Send WriteAddr high nibble address byte to write to */
  sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*!< Send WriteAddr medium nibble address byte to write to */
  sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*!< Send WriteAddr low nibble address byte to write to */
  sFLASH_SendByte(WriteAddr & 0xFF);

  /*!< while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /*!< Send the current byte */
    sFLASH_SendByte(*pBuffer);
    /*!< Point on the next byte to be written */
    pBuffer++;
  }

	/*!< Deselect the FLASH: Chip Select high */
	sFLASH_CS_HIGH();

	/*!< Wait the end of Flash writing */
	sFLASH_WaitForWriteEnd();

#if defined ( FREE_RTOS )  
	SPI_mutex_unlock();
#endif 
}

/**
  * @brief  Writes block of data to the FLASH. In this function, the number of
  *         WRITE cycles are reduced, using Page WRITE sequence.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH.
  * @retval None
  */
void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t *pWriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    uint32_t WriteAddr = 0;

    if ( pBuffer == NULL )
        return;

    if ( pWriteAddr == NULL )
        return;

    if ( NumByteToWrite == 0 )
        return;        


    WriteAddr = *pWriteAddr;
    Addr = WriteAddr % sFLASH_SPI_PAGESIZE;
    count = sFLASH_SPI_PAGESIZE - Addr;
    NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
    NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

    if (Addr == 0) /*!< WriteAddr is sFLASH_PAGESIZE aligned  */
    {
        if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
        {
            sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
        }
        else /*!< NumByteToWrite > sFLASH_PAGESIZE */
        {
            while (NumOfPage--)
            {
                sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
                WriteAddr +=  sFLASH_SPI_PAGESIZE;
                pBuffer += sFLASH_SPI_PAGESIZE;
            }

            sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    else /*!< WriteAddr is not sFLASH_PAGESIZE aligned  */
    {
        if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
        {
            if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > sFLASH_PAGESIZE */
            {
                temp = NumOfSingle - count;

                sFLASH_WritePage(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;

                sFLASH_WritePage(pBuffer, WriteAddr, temp);
            }
            else
            {
                sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        else /*!< NumByteToWrite > sFLASH_PAGESIZE */
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
            NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

            sFLASH_WritePage(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--)
            {
                sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
                WriteAddr +=  sFLASH_SPI_PAGESIZE;
                pBuffer += sFLASH_SPI_PAGESIZE;
            }

            if (NumOfSingle != 0)
            {
                sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }

  *pWriteAddr = WriteAddr;

  
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @retval None
  */
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
#if defined ( FREE_RTOS )	
	if (SPI_mutex_lock() == FALSE)
	{
		return;
	}
#endif 	


	/*!< Select the FLASH: Chip Select low */
	sFLASH_CS_LOW();

	/*!< Send "Read from Memory " instruction */
	sFLASH_SendByte(sFLASH_CMD_READ);

	/*!< Send ReadAddr high nibble address byte to read from */
	sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
	/*!< Send ReadAddr medium nibble address byte to read from */
	sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
	/*!< Send ReadAddr low nibble address byte to read from */
	sFLASH_SendByte(ReadAddr & 0xFF);

	while (NumByteToRead--) /*!< while there is data to be read */
	{
		/*!< Read a byte from the FLASH */
		*pBuffer = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
		/*!< Point to the next location where the byte read will be saved */
		pBuffer++;
	}

	/*!< Deselect the FLASH: Chip Select high */
	sFLASH_CS_HIGH();

#if defined ( FREE_RTOS )
	SPI_mutex_unlock();
#endif 

}

/**
  * @brief  Reads FLASH identification.
  * @param  None
  * @retval FLASH identification
  */
uint32_t sFLASH_ReadID(void)
{
  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

#if defined ( FREE_RTOS )
  if (SPI_mutex_lock() == FALSE)
  {
    return 0;
  }
#endif   

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "RDID " instruction */
  sFLASH_SendByte(0x9F);

  /*!< Read a byte from the FLASH */
  Temp0 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp1 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Read a byte from the FLASH */
  Temp2 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

#if defined ( FREE_RTOS )
  SPI_mutex_unlock();
#endif 

  return Temp;
}

/**
  * @brief  Initiates a read data byte (READ) sequence from the Flash.
  *   This is done by driving the /CS line low to select the device, then the READ
  *   instruction is transmitted followed by 3 bytes address. This function exit
  *   and keep the /CS line low, so the Flash still being selected. With this
  *   technique the whole content of the Flash is read with a single READ instruction.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @retval None
  */
void sFLASH_StartReadSequence(uint32_t ReadAddr)
{
#if defined ( FREE_RTOS )
    if (SPI_mutex_lock() == FALSE)
    {
      return ;
    }
#endif 

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_READ);

  /*!< Send the 24-bit address of the address to read from -------------------*/
  /*!< Send ReadAddr high nibble address byte */
  sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*!< Send ReadAddr medium nibble address byte */
  sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*!< Send ReadAddr low nibble address byte */
  sFLASH_SendByte(ReadAddr & 0xFF);

  sFLASH_CS_HIGH();

#if defined ( FREE_RTOS )
  SPI_mutex_unlock();
#endif 

}

/**
  * @brief  Reads a byte from the SPI Flash.
  * @note   This function must be used only if the Start_Read_Sequence function
  *         has been previously called.
  * @param  None
  * @retval Byte Read from the SPI Flash.
  */
uint8_t sFLASH_ReadByte(void)
{
  return (sFLASH_SendByte(sFLASH_DUMMY_BYTE));
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t sFLASH_SendByte(uint8_t byte)
{

  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(sFLASH_SPI, byte);

  /*!< Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(sFLASH_SPI);

}

/**
  * @brief  Sends a Half Word through the SPI interface and return the Half Word
  *         received from the SPI bus.
  * @param  HalfWord: Half Word to send.
  * @retval The value of the received Half Word.
  */
uint16_t sFLASH_SendHalfWord(uint16_t HalfWord)
{
  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

  /*!< Send Half Word through the sFLASH peripheral */
  SPI_I2S_SendData(sFLASH_SPI, HalfWord);

  /*!< Wait to receive a Half Word */
  while (SPI_I2S_GetFlagStatus(sFLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

  /*!< Return the Half Word read from the SPI bus */
  return SPI_I2S_ReceiveData(sFLASH_SPI);
}

/**
  * @brief  Enables the write access to the FLASH.
  * @param  None
  * @retval None
  */
void sFLASH_WriteEnable(void)
{
#if defined ( FREE_RTOS )	
  /*!< Select the FLASH: Chip Select low */
  if (SPI_mutex_lock() == FALSE)
  {
    return ;
  }
#endif 

  sFLASH_CS_LOW();

  /*!< Send "Write Enable" instruction */
  sFLASH_SendByte(sFLASH_CMD_WREN);

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

#if defined ( FREE_RTOS )
  SPI_mutex_unlock();
#endif 


}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
void sFLASH_WaitForWriteEnd(void)
{
  uint8_t flashstatus = 0;

#if defined ( FREE_RTOS )

  if (SPI_mutex_lock() == FALSE)
  {
    return ;
  }
#endif   

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read Status Register" instruction */
  sFLASH_SendByte(sFLASH_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus = sFLASH_SendByte(sFLASH_DUMMY_BYTE);

  }
  while ((flashstatus & sFLASH_WIP_FLAG) == SET); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

#if defined ( FREE_RTOS )
  SPI_mutex_unlock();
#endif 

}

#endif /*EXT_FLASH*/
