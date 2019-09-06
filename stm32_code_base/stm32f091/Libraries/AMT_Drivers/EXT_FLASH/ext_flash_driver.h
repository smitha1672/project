#ifndef __EXT_FLASH_DRIVER_H__
#define __EXT_FLASH_DRIVER_H__

#include "Defs.h"
#include "device_config.h"

#if ( configSPI_FLASH == 1 )
void sFLASH_EraseSector(uint32_t SectorAddr);

void sFLASH_EraseChip(void);

void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);

void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t *pWriteAddr, uint16_t NumByteToWrite);

void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);

void sFLASH_StartReadSequence(uint32_t ReadAddr);

uint8_t sFLASH_ReadByte(void);

uint8_t sFLASH_SendByte(uint8_t byte);

uint16_t sFLASH_SendHalfWord(uint16_t HalfWord);
#else

#if ( configAPP_INTERNAL_DSP_ULD == 0 )
#error " external flash has not been enable.That is set from device_config.h "
#endif 

#endif 

#endif /* ( configSPI_FLASH == 1 ) */
