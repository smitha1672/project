/**
  ******************************************************************************
  * @file    flash_layer.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22/07/2011
  * @brief   Header file for flash_layer.c
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FLASH_ACCESS_LAYER_H
#define _FLASH_ACCESS_LAYER_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
   
/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);
/* Exported constants --------------------------------------------------------*/

#if defined ( STM32F10X_CL )
#define FLASH_SIZE	((uint32_t)0x40000)  /* 256 KBytes */
#define IAP_SIZE	((uint32_t)0x6000)   /* 24Kbytes as IAP size */
#define PAGE_SIZE ((uint16_t)0x800)    /* 2 Kbytes */
#define FLASH_STARTADDRESS ((uint32_t)0x08000000) /* Flash Start Address */

#define FLASH_APPLICATION_STARTADDRESS (FLASH_STARTADDRESS + IAP_SIZE)
#define FLASH_APPLICATION_ENDADDRESS (FLASH_STARTADDRESS + FLASH_SIZE)

#define APPLICATIONADDRESS	FLASH_APPLICATION_STARTADDRESS /* User start code space */
#define FLASH_ENDADDRESS  	FLASH_APPLICATION_ENDADDRESS 
#endif  

#if defined ( EXT_FLASH )
#define EXT_FLASH_SIZE ((uint32_t)0x200000) /*2048 KBytes*/
#define EXT_FLASH_STARTADDRESS ((uint32_t)0x00000000)
#define EXT_FLASH_ENDADDRESS ( EXT_FLASH_STARTADDRESS + EXT_FLASH_SIZE )
#endif 


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
void FLASH_LAYER_FlashUnlock(void);
FlagStatus FLASH_LAYER_ReadOutProtectionStatus(void);
FLASH_Status FLASH_LAYER_ErasePage(uint32_t Page_Address);
FLASH_Status FLASH_LAYER_ProgramWord(uint32_t Address, uint32_t Data);

FLASH_Status FLASH_LAYER_write_n_word( uint32_t *pflash_addr, uint8_t *pByte, uint32_t len );


#ifdef __cplusplus
}
#endif

#endif  /* _FLASH_LAYER_H */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/


