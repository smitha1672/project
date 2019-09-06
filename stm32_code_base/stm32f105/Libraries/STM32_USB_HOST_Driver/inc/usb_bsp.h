/**
  ******************************************************************************
  * @file    usb_bsp.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/29/2010
  * @brief   specific api's relative to the used hardware platform
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_BSP__H__
#define __USB_BSP__H__

/* Includes ------------------------------------------------------------------*/
#include "usb_defines.h"

/** @addtogroup USB_OTG_DRIVER
  * @{
  */

  
/** @defgroup USB_BSP
  * @brief This file is the 
  * @{
  */ 


/** @defgroup USB_BSP_Exported_Defines
  * @{
  */ 
#define USE_ACCURATE_TIME
#define TIM_MSEC_DELAY                     0x01
#define TIM_USEC_DELAY                     0x02
#define HOST_OVRCURR_PORT                  GPIOE
#define HOST_OVRCURR_LINE                  GPIO_Pin_1
#define HOST_OVRCURR_PORT_SOURCE           GPIO_PortSourceGPIOE
#define HOST_OVRCURR_PIN_SOURCE            GPIO_PinSource1
#define HOST_OVRCURR_PORT_RCC              RCC_APB2Periph_GPIOE
#define HOST_OVRCURR_EXTI_LINE             EXTI_Line1
#define HOST_OVRCURR_IRQn                  EXTI1_IRQn 

#define HOST_POWERSW_PORT_RCC              RCC_APB2Periph_GPIOC
#define HOST_POWERSW_PORT                  GPIOC
#define HOST_POWERSW_VBUS                  GPIO_Pin_9

#define HOST_SOF_OUTPUT_RCC                RCC_APB2Periph_GPIOA
#define HOST_SOF_PORT                      GPIOA
#define HOST_SOF_SIGNAL                    GPIO_Pin_8
/**
  * @}
  */ 


/** @defgroup USB_BSP_Exported_Types
  * @{
  */ 
typedef enum 
{
  NoTimeout = 1,
  Timeout,  
}BSP_TimeoutStatus; 
/**
  * @}
  */ 


/** @defgroup USB_BSP_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_BSP_Exported_Variables
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USB_BSP_Exported_FunctionsPrototype
  * @{
  */ 
void BSP_Init(void);
void USB_OTG_BSP_Init(void);
void USB_OTG_BSP_TimeInit (void);
void USB_OTG_BSP_uDelay (const uint32_t usec);
void USB_OTG_BSP_mDelay (const uint32_t msec);
void USB_OTG_BSP_EnableInterrupt (void);
void USB_OTG_BSP_ConfigVBUS(void);
void USB_OTG_BSP_DriveVBUS(uint8_t state);
void USB_OTG_BSP_TimerIRQ (void);
BSP_TimeoutStatus BSP_secTimeout(void);
void BSP_secTimeoutStart(uint32_t);
void BSP_msecTimeoutStart(uint32_t);
BSP_TimeoutStatus BSP_msecTimeout(void);
/**
  * @}
  */ 

#endif //__USB_BSP__H__

/**
  * @}
  */ 

/**
  * @}
  */ 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

