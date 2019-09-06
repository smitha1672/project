/**
  ******************************************************************************
  * @file    IAP/inc/command.h 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22/07/2011
  * @brief   Header file for command.c 
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
#ifndef _COMMAND_H
#define _COMMAND_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "flash_layer.h"
#include "usbh_usr.h"
#include "ff.h"       /* FATFS */


/* Exported types ------------------------------------------------------------*/
#define PES_HEADER_LEN	20
#define PES_START_CODE_LEN	2
#define PES_PID_LEN	6
#define PES_PAYLOAD_LEN 3
#define PES_RESERVE_LEN 8

typedef struct _PES_ {
	uint8_t	start_code[PES_START_CODE_LEN];
	uint8_t	pid[PES_PID_LEN];	/*product ID*/
	uint8_t	payload_size[PES_PAYLOAD_LEN];
	uint8_t	payload_checksum;
	uint8_t reserve[PES_RESERVE_LEN];
} PES;

/* Exported constants --------------------------------------------------------*/
   
#define CMD_UPLOAD                         ((uint8_t)0x01) /* Reads memory starting from an
                                                                  address specified by the application */
#define CMD_DOWNLOAD                       ((uint8_t)0x02) /* Writes RAM or Flash memory starting 
                                                            from an address specified by the application */
#define CMD_JUMP                           ((uint8_t)0x03) /* Jumps to user application code located
                                                                 in the internal Flash memory or in SRAM */
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
int COMMAND_IAPExecuteApplication(void);
void COMMAND_UPLOAD(void);
void COMMAND_JUMP(void);
void COMMAND_DOWNLOAD(void);
FRESULT COMMAND_ProgramFlashMemory(void);

bool COMMAND_isCorrectCheckSum(uint8_t CheckSum); 

#ifdef __cplusplus
}
#endif

#endif  /* _COMMAND_H */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
