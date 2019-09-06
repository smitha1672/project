/**
  ******************************************************************************
  * @file    usbh_usr.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date   22/07/2011
  * @brief   Header file for usbh_usr.c
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
#ifndef __USH_USR_H__
#define __USH_USR_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "efs.h"
//#include "stm3210c_eval_lcd.h"
#include "usbh_core.h"
//#include "stm32_eval.h"
#include <stdio.h>
#include "usbh_msc_core.h"
#include "ls.h"
//#include "stm3210c_eval_ioe.h"

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
  uint8_t filenameString[12];
  uint8_t file_index ;
}Download_FileTypeDef;
/* Exported constants --------------------------------------------------------*/
/* IAP defines */
#define IAP_COMMAND_SELECT               ((uint8_t)0xFF)
#define IAP_DOWNLOAD_SELECT              ((uint8_t)0xF0)
#define NEXT_DOWNLOAD                    ((uint8_t)0xAA)
#define NEXT_MENU                        ((uint8_t)0xBB)
#define DEVICE_DISCONNECT                ((uint8_t)0xCC) 


/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern  USBH_Usr_cb_TypeDef USR_USBH_MSC_cb;


//extern  USBH_Usr_cb_TypeDef USR_Callbacks;
extern  EmbeddedFile               fileR;
extern EmbeddedFileSystem          efs;
extern EmbeddedFile                file;
extern DirList                     list;
extern Download_FileTypeDef Download_File [9];
extern __IO uint8_t Seclect_FileCounter;
extern __IO uint8_t Seclect_CommandCounter;
extern uint8_t CMD_index;
/* Exported functions ------------------------------------------------------- */ 
void USBH_USR_IAPInit(void);
void USBH_USR_IAPMenu(void);
void USBH_USR_Init(void);
void USBH_USR_DeviceAttached(void);
void USBH_USR_ResetDevice(void);
void USBH_USR_DeviceDisconnected (void);
void USBH_USR_OverCurrentDetected (void);
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed); 
void USBH_USR_Device_DescAvailable(void *);
void USBH_USR_DeviceAddressAssigned(void);
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc);
void USBH_USR_Manufacturer_String(void *);
void USBH_USR_Product_String(void *);
void USBH_USR_SerialNum_String(void *);
USBH_USR_Status USBH_USR_UserInput(void);
int USBH_USR_MSC_Application(void);
void USBH_USR_DeInit(void);
void USBH_USR_DeviceNotSupported(void);
void USBH_USR_UnrecoveredError(void);
uint8_t USBH_USR_SelectControlMenu(void);
void USBH_USR_EnumerationDone(void);

uint32_t USBH_IsDeviceConnect( void ); /*Smith implemented !!*/
uint8_t USBH_isDeviceMount( void );
uint8_t USBH_UnMount(void);	//Angus add
uint8_t USBH_Mount(void);

void USBH_Set_USB_Ever_UnMount_State();
void USBH_ReSet_USB_Ever_UnMount_State();

int USBH_Get_USB_Ever_UnMount_State(void);



//____________________________________________________________________________________
//extern int COMMAND_IAPExecuteApplication(void);
//extern int COMMAND_WavePlayStateMachine(void);

#ifdef __cplusplus
}
#endif

#endif /*__USH_USR_H__*/

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/




