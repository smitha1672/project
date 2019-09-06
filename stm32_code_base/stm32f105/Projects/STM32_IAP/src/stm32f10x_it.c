/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file includes the interrupt handlers for the application
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_bsp.h"
#include "usb_hcd_int.h"
#include "usbh_core.h"
#include "stm32f10x_it.h"

/** @addtogroup USBH_USER
  * @{
  */

/** @defgroup IAP_OVER_USB_INTERRUPT
  * @brief
  * @{
  */


/* External variables --------------------------------------------------------*/
extern uint8_t joystick_use;
extern uint8_t lcdLineNo;
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t CMD_index = 0x01;
__IO uint8_t Seclect_CommandCounter = 40;
__IO uint8_t Seclect_FileCounter = 130;
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
extern USBH_HOST                    USB_Host;
 
/* Private function prototypes -----------------------------------------------*/
extern void USB_OTG_BSP_TimerIRQ (void);
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
/**
  * @brief  NMI_Handler
  *         This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{}

/**
  * @brief  HardFault_Handler
  *         This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  MemManage_Handler
  *         This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {}
}

/**
  * @brief  BusFault_Handler
  *         This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  UsageFault_Handler
  *         This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {}
}

/**
  * @brief  SVC_Handler
  *         This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{}

/**
  * @brief  DebugMon_Handler
  *         This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{}


/**
  * @brief  PendSV_Handler
  *         This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{}

/**
  * @brief  EXTI1_IRQHandler
  *         This function handles External line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line1) != RESET)
  {
      USB_Host.usr_cb->OverCurrentDetected();
      EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
/**
  * @brief  TIM2_IRQHandler
  *         This function handles Timer2 Handler.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
  USB_OTG_BSP_TimerIRQ();
}


/**
  * @brief  OTG_FS_IRQHandler
  *          This function handles USB-On-The-Go FS global interrupt request.
  *          requests.
  * @param  None
  * @retval None
  */
#ifdef USE_USB_OTG_FS  
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  USBH_OTG_ISR_Handler(&USB_OTG_Core);
}
/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
#if 0 /*smith mark; it is un-used*/    
  static JOYState_TypeDef JoyState = JOY_NONE;

  /* Control joystick interrupt */
  if (EXTI_GetITStatus(IOE_IT_EXTI_LINE) != RESET)
  {
    if (IOE_GetGITStatus(IOE_2_ADDR, IOE_GIT_GPIO))
    {

      /* Get the Joytick State */
      JoyState = IOE_JoyStickGetState();

      switch (JoyState)
      {
        case JOY_UP:
          /* Joystick is used to select IAP command */
          if (joystick_use == IAP_COMMAND_SELECT)
          {
            if (Seclect_CommandCounter == 40)
            {
              /* maximum up position */
              break;
            }
            else
            {
              //LCD_SetBackColor(Black);
              //LCD_DrawFullRect(Seclect_CommandCounter, 200, 10, 8);
              Seclect_CommandCounter = Seclect_CommandCounter - 10;
              //LCD_SetBackColor(Blue);
              //LCD_DrawFullRect(Seclect_CommandCounter, 200, 10, 8);
              CMD_index--;
              //LCD_SetBackColor(Black);
              //LCD_SetTextColor(White);
            }
          }
          /* Joystick is used to select the file to be downloaded */
          else if (joystick_use == IAP_DOWNLOAD_SELECT )
          {
            if (Seclect_FileCounter == 130)
            {
              /* maximum up position */
              break;
            }
            else
            {
              //LCD_SetBackColor(Black);
              //LCD_DrawFullRect(Seclect_FileCounter, 200, 10, 8);
              Seclect_FileCounter = Seclect_FileCounter - 10;
              //LCD_SetBackColor(Blue);
              //LCD_DrawFullRect(Seclect_FileCounter, 200, 10, 8);
              //LCD_SetBackColor(Black);
              //LCD_SetTextColor(White);
            }
          }
          break;
        case JOY_DOWN:
          /* Joystick is used to select IAP command */
          if (joystick_use == IAP_COMMAND_SELECT)
          {
            if (Seclect_CommandCounter == 60)
            {
              /* maximum down position  */
              break;
            }
            else
            {
              //LCD_SetBackColor(Black);
              //LCD_DrawFullRect(Seclect_CommandCounter, 200, 10, 8);
              Seclect_CommandCounter = Seclect_CommandCounter + 10;
              //LCD_SetBackColor(Blue);
              //LCD_DrawFullRect(Seclect_CommandCounter, 200, 10, 8);
              CMD_index++;
              //LCD_SetBackColor(Black);
              //LCD_SetTextColor(White);
            }
          }
          /* Joystick is used to select the file to be downloaded */
          else if (joystick_use == IAP_DOWNLOAD_SELECT )
          {
            if ((Seclect_FileCounter == lcdLineNo) || (Seclect_FileCounter == 210))
            {
              /* maximum down position */
              break;
            }
            else
            {
              //LCD_SetBackColor(Black);
              //LCD_DrawFullRect(Seclect_FileCounter, 200, 10, 8);
              Seclect_FileCounter = Seclect_FileCounter + 10;
              //LCD_SetBackColor(Blue);
              //LCD_DrawFullRect(Seclect_FileCounter, 200, 10, 8);
              //LCD_SetBackColor(Black);
              //LCD_SetTextColor(White);
            }
          }
          break;
        default:
          break;
      }
      /* Clear the interrupt pending bits */
      IOE_ClearGITPending(IOE_2_ADDR, IOE_GIT_GPIO);
      IOE_ClearIOITPending(IOE_2_ADDR, IOE_JOY_IT);
    }
    /* Clear EXTI pending bit */
    EXTI_ClearITPendingBit(IOE_IT_EXTI_LINE);
  }
#endif 

  
}
/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
