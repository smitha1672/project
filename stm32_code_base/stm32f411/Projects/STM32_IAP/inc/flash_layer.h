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
#include "Defs.h"   
   
/* Exported types ------------------------------------------------------------*/
typedef  void (*pFunction)(void);
/* Exported constants --------------------------------------------------------*/
#define FLASH_SIZE	((uint32_t)0x40000)  /* 256 KBytes */
#if defined ( STM32F10X_CL )
#define IAP_SIZE	((uint32_t)0x6000)   /* 24Kbytes as IAP size */
#elif defined ( STM32F40_41xxx )
#define IAP_SIZE	((uint32_t)0x8000)   /* 32Kbytes as IAP size */
#endif

#define PAGE_SIZE ((uint16_t)0x800)    /* 2 Kbytes */
#define FLASH_STARTADDRESS ((uint32_t)0x08000000) /* Flash Start Address */

#define FLASH_APPLICATION_STARTADDRESS (FLASH_STARTADDRESS + IAP_SIZE)
#define FLASH_APPLICATION_ENDADDRESS (FLASH_STARTADDRESS + FLASH_SIZE)

#define APPLICATIONADDRESS	FLASH_APPLICATION_STARTADDRESS /* User start code space */
#define FLASH_ENDADDRESS  	FLASH_APPLICATION_ENDADDRESS 

#if defined ( EXT_FLASH )
#define EXT_FLASH_SIZE ((uint32_t)0x100000) /*1024 KBytes*/ /*K2 U14*/
#define EXT_FLASH_STARTADDRESS ((uint32_t)0x00000000)
#define EXT_FLASH_ENDADDRESS ( EXT_FLASH_STARTADDRESS + EXT_FLASH_SIZE )
#endif

#if defined ( STM32F40_41xxx )
/* Base address of the Flash sectors */ 
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes   */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 Kbytes */

#if defined (USE_STM324x7I_EVAL) || defined (USE_STM324x9I_EVAL) 
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base address of Sector 12, 16 Kbytes  */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base address of Sector 13, 16 Kbytes  */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base address of Sector 14, 16 Kbytes  */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base address of Sector 15, 16 Kbytes  */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base address of Sector 16, 64 Kbytes  */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base address of Sector 17, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base address of Sector 18, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base address of Sector 19, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base address of Sector 20, 128 Kbytes */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base address of Sector 21, 128 Kbytes */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base address of Sector 22, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base address of Sector 23, 128 Kbytes */
#endif /* USE_STM324x7I_EVAL or USE_STM324x9I_EVAL */
#endif


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
void FLASH_LAYER_FlashUnlock(void);

void FLASH_LAYER_Flashlock(void);

uint32_t FLASH_LAYER_GetSector(uint32_t Address);

FlagStatus FLASH_LAYER_ReadOutProtectionStatus(void);

FLASH_Status FLASH_LAYER_ErasePage(uint32_t Page_Address);

FLASH_Status FLASH_LAYER_ProgramWord(uint32_t Address, uint32_t Data);

FLASH_Status FLASH_LAYER_write_n_word( uint32_t *pflash_addr, uint8_t *pByte, uint32_t len );

#ifdef __cplusplus
}
#endif

#endif  /* _FLASH_LAYER_H */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/


