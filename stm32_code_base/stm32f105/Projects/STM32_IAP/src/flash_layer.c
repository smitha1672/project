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


/** @addtogroup USBH_USER
  * @{
  */

/** @defgroup IAP_OVER_USB_FLASH_LAYER
  * @brief
  * @{
  */

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Unlocks the FLASH Program Erase Controller.
  * @note   This function can be used for all STM32F10x devices.
  *         - For STM32F10X_XL devices this function unlocks Bank1 and Bank2.
  *         - For all other devices it unlocks Bank1 and it is equivalent
  *           to FLASH_UnlockBank1 function..
  * @param  None
  * @retval None
  */
void FLASH_LAYER_FlashUnlock(void)
{
  FLASH_Unlock();
}

/**
  * @brief Get readout protection status
  * @param  None
  * @retval FLASH ReadOut Protection Status(SET or RESET)
  */
FlagStatus FLASH_LAYER_ReadOutProtectionStatus(void)
{
  FlagStatus readoutstatus = RESET;
  if (FLASH_GetReadOutProtectionStatus() == SET)
  {
    readoutstatus = SET;
  }
  else
  {
    readoutstatus = RESET;
  }
  return readoutstatus;
}

/**
  * @brief  Erases a specified FLASH page.
  * @param  Page_Address: The page address to be erased.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PG,
  *   FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
FLASH_Status FLASH_LAYER_ErasePage(uint32_t Page_Address)
{
  FLASH_Status status = FLASH_COMPLETE;

  status = FLASH_ErasePage(Page_Address);
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


/**
  * @}
  */

/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
