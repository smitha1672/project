/**
  ******************************************************************************
  * @file    flash_layer.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22/07/2011
  * @brief   This file provides all the flash_layer functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "flash_layer.h"

uint32_t mdebugdata = 0;
uint32_t mdebugaddr = 0;

#if defined ( STM32F40_41xxx )
uint32_t FLASH_LAYER_GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  
#if defined (USE_STM324xG_EVAL)
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }

#else /* USE_STM324x7I_EVAL or USE_STM324x9I_EVAL */  
  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_Sector_11;  
  }

  else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_Sector_12;  
  }
  else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_Sector_13;  
  }
  else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_Sector_14;  
  }
  else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_Sector_15;  
  }
  else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_Sector_16;  
  }
  else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_Sector_17;  
  }
  else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_Sector_18;  
  }
  else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
  {
    sector = FLASH_Sector_19;  
  }
  else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
  {
    sector = FLASH_Sector_20;  
  } 
  else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
  {
    sector = FLASH_Sector_21;  
  }
  else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
  {
    sector = FLASH_Sector_22;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
  {
    sector = FLASH_Sector_23;  
  }
#endif /* USE_STM324x7I_EVAL or USE_STM324x9I_EVAL */
  return sector;
}
#endif

void FLASH_LAYER_FlashUnlock(void)
{
    FLASH_Unlock();
}

void FLASH_LAYER_Flashlock(void)
{
    FLASH_Lock( );
}

FlagStatus FLASH_LAYER_ReadOutProtectionStatus(void)
{
  FlagStatus readoutstatus = RESET;

#if defined ( STM32F10X_CL )  
  if (FLASH_GetReadOutProtectionStatus() == SET)
  {
    readoutstatus = SET;
  }
  else
  {
    readoutstatus = RESET;
  }
#endif
  
  return readoutstatus;
}

FLASH_Status FLASH_LAYER_ErasePage(uint32_t Page_Address)
{
  FLASH_Status status = FLASH_COMPLETE;

#if defined ( STM32F10X_CL )
  status = FLASH_ErasePage(Page_Address);
#endif 

  return status;
}

FLASH_Status FLASH_LAYER_EraseSector( uint32_t FlashSector )
{
    FLASH_Status status = FLASH_COMPLETE;

#if defined ( STM32F40_41xxx )
    status = FLASH_EraseSector(FlashSector, VoltageRange_3);
#endif    

    return status;
}

/**
  * @brief  Programs a word at a specified address.
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *   FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_LAYER_ProgramWord(uint32_t Address, uint32_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;

  status = FLASH_ProgramWord(Address, Data);
  return status;
}

FLASH_Status FLASH_LAYER_write_n_word( uint32_t *pflash_addr, uint8_t *pByte, uint32_t len )
{
	uint32_t *pWord;
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t block_num = 0;
	uint32_t insufficient_block = 0;
	uint32_t addr = *pflash_addr;
	uint32_t i = 0;

	pWord = (uint32_t*)pByte;

	if ( *pflash_addr > FLASH_APPLICATION_ENDADDRESS )
		return FLASH_ERROR_WRP;

	block_num = len/4;
	if ( block_num != 0)
	{
		for ( i=0; i<block_num; i++ )
		{
			mdebugaddr = addr;
			mdebugdata = *(pWord+i);

			if ( addr > FLASH_APPLICATION_ENDADDRESS )
				return FLASH_ERROR_WRP;

			status = FLASH_ProgramWord(addr, *(pWord+i));
			
			*pflash_addr = (*pflash_addr + 4);
			addr = *pflash_addr;
			if ( status != FLASH_COMPLETE )
			{
				mdebugaddr = addr;
				return status;
			}

			
		}
		
	}

	insufficient_block = len%4; /*If data length is insufficient a 4 bytes, */
	if ( insufficient_block != 0 )		
	{
		if ( block_num != 0 )
		{
			*pflash_addr = (*pflash_addr+4);
		}
		
		addr = *pflash_addr;

		mdebugaddr = addr;
		mdebugdata = *(pWord+i);

		if ( addr > FLASH_APPLICATION_ENDADDRESS )
				return FLASH_ERROR_WRP;

		status = FLASH_ProgramWord(addr, *(pWord+i));
		if ( status != FLASH_COMPLETE )
		{
			mdebugaddr = addr;
			return status;
		}
	}
	
	return status;
}


/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
