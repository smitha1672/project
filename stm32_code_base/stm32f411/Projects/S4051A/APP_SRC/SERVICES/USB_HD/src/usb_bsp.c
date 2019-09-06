/**
  ******************************************************************************
  * @file    usb_bsp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file is responsible to offer board support package and is 
  *          configurable by user.
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
#include "usbd_conf.h"
#include "config.h"
#include "FreeRTOSConfig.h"
#include "freertos_conf.h"

#if !defined ( FREE_RTOS )
#include "VirtualTimer.h"
#endif 

#define configUSB_VBUS 0

//_____________________________________________________________________________________
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
#if defined ( STM32F10X_CL )    
    RCC_OTGFSCLKConfig(RCC_OTGFSCLKSource_PLLVCO_Div3);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE) ;
#elif defined ( STM32F40_41xxx )

    /* Note: On STM32F4-Discovery board only USB OTG FS core is supported. */
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);  
#if 1
    /* Tony 141109: Need to ask STM. F411 have pull up resistor on DP */
    /* Configure SOF VBUS ID DM DP Pins */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

#endif
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ; 
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;

    /* this for ID line debug */
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_OTG1_FS) ;   


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE); 
#endif    
}
/**
* @brief  USB_OTG_BSP_EnableInterrupt
*         Enabele USB Global interrupt
* @param  None
* @retval None
*/
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
#ifdef USE_USB_OTG_HS   
	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
#else
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;  
#endif


#if defined (FREE_RTOS)
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_VCP_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
#else
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = VCP_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = VCP_SUBPRIO;
#endif 	
	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);  
	
#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_OUT_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);  
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_IN_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);   
#endif
}

void USB_OTG_BSP_Host_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
#ifdef USE_USB_OTG_HS   
	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
#else
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;  
#endif

#if defined ( FREE_RTOS )
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_MSC_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
#else 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MSC_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = MSC_SUBPRIO;
#endif 	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);  
	
#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_OUT_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);  
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_IN_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);   
#endif
}


/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  state : VBUS states
  * @retval None
  */

void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{
  /*
  On-chip 5 V VBUS generation is not supported. For this reason, a charge pump 
  or, if 5 V are available on the application board, a basic power switch, must 
  be added externally to drive the 5 V VBUS line. The external charge pump can 
  be driven by any GPIO output. When the application decides to power on VBUS 
  using the chosen GPIO, it must also set the port power bit in the host port 
  control and status register (PPWR bit in OTG_FS_HPRT).
  
  Bit 12 PPWR: Port power
  The application uses this field to control power to this port, and the core 
  clears this bit on an overcurrent condition.
  */

#if( configUSB_VBUS == 1 )
#ifndef USE_USB_OTG_HS   
  if (0 == state)
  { 
    /* DISABLE is needed on output of the Power Switch */
    GPIO_SetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
  }
  else
  {
    /*ENABLE the Power Switch by driving the Enable LOW */
    GPIO_ResetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
  }
#endif
#endif  
}


/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  None
  * @retval None
  */

void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
#if( configUSB_VBUS == 1 )
#ifndef USE_USB_OTG_HS 
  GPIO_InitTypeDef GPIO_InitStructure; 

#ifdef USE_STM3210C_EVAL
  RCC_APB2PeriphClockCmd(HOST_POWERSW_PORT_RCC, ENABLE);
  
  
  /* Configure Power Switch Vbus Pin */
  GPIO_InitStructure.GPIO_Pin = HOST_POWERSW_VBUS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(HOST_POWERSW_PORT, &GPIO_InitStructure);
#else
  #ifdef USE_USB_OTG_FS  
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOH , ENABLE);  
  
  GPIO_InitStructure.GPIO_Pin = HOST_POWERSW_VBUS;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(HOST_POWERSW_PORT,&GPIO_InitStructure);
  #endif  
#endif
#endif

  /* By Default, DISABLE is needed on output of the Power Switch */
  GPIO_SetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
  
  USB_OTG_BSP_mDelay(200);   /* Delay is need for stabilising the Vbus Low 
  in Reset Condition, when Vbus=1 and Reset-button is pressed by user */
#endif  
}


/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay (const uint32_t usec)
{
  uint32_t count = 0;
  const uint32_t utime = (120 * usec / 7);
  do
  {
    if ( ++count > utime )
    {
      return ;
    }
  }
  while (1);
}


/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
	USB_OTG_BSP_uDelay(msec * 1000);   
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
