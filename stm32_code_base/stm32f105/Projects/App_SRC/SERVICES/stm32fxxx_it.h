/**
  ******************************************************************************
  * @file    stm32fxxx_it.h 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32Fxxx_IT_H
#define __STM32Fxxx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "Defs.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);

void HardFault_Handler(void);

void MemManage_Handler(void);

void BusFault_Handler(void);

void UsageFault_Handler(void);

void SVC_Handler(void);

void DebugMon_Handler(void);

void PendSV_Handler(void);

void SysTick_Handler(void);

void WWDG_IRQHandler( void );

void PVD_IRQHandler(void);

void RTC_IRQHandler(void);

void TAMPER_IRQHandler(void);

void FLASH_IRQHandler( void );

void RCC_IRQHandler( void );

void EXTI0_IRQHandler( void );

void EXTI1_IRQHandler(void);

void EXTI2_IRQHandler(void);

void EXTI3_IRQHandler(void);

void EXTI4_IRQHandler(void);


void DMA1_Channel1_IRQHandler( void ); //DMA1 Channel 1
void DMA1_Channel2_IRQHandler( void ); //DMA1 Channel 2
void DMA1_Channel3_IRQHandler( void ); //DMA1 Channel 3
void DMA1_Channel4_IRQHandler( void ); //DMA1 Channel 4
void DMA1_Channel5_IRQHandler( void ); //DMA1 Channel 5
void DMA1_Channel6_IRQHandler( void ); //DMA1 Channel 6
void DMA1_Channel7_IRQHandler( void ); //DMA1 Channel 7
void ADC1_2_IRQHandler( void ); //ADC1 and ADC2

void OTG_HS_EP1_OUT_IRQHandler( void );
void OTG_HS_EP1_IN_IRQHandler( void );

void CAN1_TX_IRQHandler( void ); //CAN1 TX
void CAN1_RX0_IRQHandler( void ); //CAN1 RX0
void CAN1_RX1_IRQHandler( void ); //CAN1 RX1
void CAN1_SCE_IRQHandler( void ); //CAN1 SCE
void EXTI9_5_IRQHandler( void ); //EXTI Line 9..5
void TIM1_BRK_IRQHandler( void ); //TIM1 Break
void TIM1_UP_IRQHandler( void ); //TIM1 Update
void TIM1_TRG_COM_IRQHandler( void ); //TIM1 Trigger and Commutation
void TIM1_CC_IRQHandler( void ); //TIM1 Capture Compare
void TIM2_IRQHandler( void ); //TIM2
void TIM3_IRQHandler( void ); //TIM3
void TIM4_IRQHandler( void ); //TIM4
void I2C1_EV_IRQHandler( void ); //I2C1 Event
void I2C1_ER_IRQHandler( void ); //I2C1 Error
void I2C2_EV_IRQHandler( void ); //I2C2 Event
void I2C2_ER_IRQHandler( void ); //I2C1 Error
void SPI1_IRQHandler( void ); //SPI1
void SPI2_IRQHandler ( void ); //SPI2
void USART1_IRQHandler( void ); //USART1
void USART2_IRQHandler( void ); //USART2
void USART3_IRQHandler( void ); //USART3
void EXTI15_10_IRQHandler( void ) ; //EXTI Line 15..10
void RTCAlarm_IRQHandler(void ); //RTC alarm through EXTI line
void OTG_FS_WKUP_IRQHandler(void); //USB OTG FS Wakeup through EXTI line
void TIM5_IRQHandler(void);
void SPI3_IRQHandler( void );  //SPI3
void UART4_IRQHandler( void ); //UART4
void UART5_IRQHandler( void ); //UART5
void TIM6_IRQHandler( void );  //TIM6
void TIM7_IRQHandler( void );  //TIM7
void DMA2_Channel1_IRQHandler( void ); //DMA2 Channel1
void DMA2_Channel2_IRQHandler( void ); //DMA2 Channel2
void DMA2_Channel3_IRQHandler( void ); //DMA2 Channel3
void DMA2_Channel4_IRQHandler( void ); //DMA2 Channel4
void DMA2_Channel5_IRQHandler( void ); //DMA2 Channel5
void ETH_IRQHandler( void );           //Ethernet
void ETH_WKUP_IRQHandler( void );      //Ethernet Wakeup through EXTI line
void CAN2_TX_IRQHandler( void );       //CAN2 TX
void CAN2_RX0_IRQHandler( void );      //CAN2 RX0
void CAN2_RX1_IRQHandler( void );      //CAN2 RX1
void CAN2_SCE_IRQHandler( void );      //CAN2 SCE
void OTG_FS_IRQHandler( void );         //USB OTG FS







#ifdef __cplusplus
}
#endif

#endif /* __STM32Fxxx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
