/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22/07/2011
  * @brief   IAP thru USB host main file
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
#if defined( STM32F10X_CL )
#include "stm32f10x_conf.h"
#endif

#if defined ( USE_STM3210C_EVAL )
#include "stm3210c_eval.h"
#endif

#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "flash_layer.h"
#include "usb_bsp.h"
#include "command.h"

//! Smith implemets @{
#include "HT68F30.h"
#include "GPIOMiddleLevel.h"
//@}


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE      USB_OTG_Core __ALIGN_END;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST                USB_Host __ALIGN_END;

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
//#define DEBUG_BOOT_LOADER 

#define VBUS GPIO_Pin_9
#define USB_VIN GPIO_Pin_6
#define DE_24V GPIO_Pin_7
#define USB_OE GPIO_Pin_5
#define OTI_OE GPIO_Pin_4
#define DE_5V GPIO_Pin_5 // Brendan - on AC, pull low
#define BKP_RESET_STATE_INDICATOR BKP_DR42

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;	
uint32_t JumpAddress;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void GPIO_Configuration(void);
/**
  * @brief  main
  *         Main routine for IAP application
  * @param  None
  * @retval int
  */
int main(void)
{
#if defined ( STM32F10X_CL )
    uint16_t bkp_data = 0;
#endif    
    
	BSP_Init();

	USBH_USR_IAPInit();

	FLASH_LAYER_FlashUnlock();

#if 1//!defined (DEBUG_BOOT_LOADER)
	if( ( RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) /*|| ( STM_EVAL_PBGetState(BUTTON_WAKEUP)  == RESET )*/ )/*Last time is AC OFF*/
	{
#else
    if ( 1 )
    {
#endif 

#if defined ( STM32F10X_CL )        
        bkp_data = ( 0xFF00 | RCC_FLAG_PORRST );
        BKP_WriteBackupRegister(BKP_RESET_STATE_INDICATOR, bkp_data);
#endif        
        
		/* Clear reset flags */
		RCC_ClearFlag();
		
        USBH_Init(&USB_OTG_Core, 
#ifdef USE_USB_OTG_FS  
        USB_OTG_FS_CORE_ID,
#else 
        USB_OTG_HS_CORE_ID,
#endif 
        &USB_Host,
        &USBH_MSC_cb, 
        &USR_cb);

        //GPIOMiddleLevel_Clr( __O_USB_VBUS ); 
        GPIOMiddleLevel_Set( __O_USB_VIN );  
        
        while (1)
        {
            /* Host Task handler */
            if (USBH_Process(&USB_OTG_Core, &USB_Host) == HOST_ERROR_STATE )
            {
                break;
            }    
        }
        
	}
#if defined ( STM32F10X_CL )/*Smith debug on 8/26*/
    else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST) != RESET)
    {
#if defined ( STM32F10X_CL )    
        bkp_data = ( 0xFF00 |  RCC_FLAG_SFTRST );
        BKP_WriteBackupRegister(BKP_RESET_STATE_INDICATOR, bkp_data);
#endif        

        /* Clear reset flags */
        RCC_ClearFlag();
    }
    else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
#if defined ( STM32F10X_CL )    
        bkp_data = ( 0xFF00 | RCC_FLAG_IWDGRST );
        BKP_WriteBackupRegister(BKP_RESET_STATE_INDICATOR, bkp_data);
#endif        
        /* Clear reset flags */
        RCC_ClearFlag();
    }
#endif

    FLASH_LAYER_Flashlock( );

	/* Test if user code is programmed starting from address "ApplicationAddress" */
	if (((*(__IO uint32_t*)APPLICATIONADDRESS) & 0x2FFE0000 ) == 0x20000000 )
	{
		/* Jump to user application */
		JumpAddress = *(__IO uint32_t*) (APPLICATIONADDRESS + 4);
	
		Jump_To_Application = (pFunction) JumpAddress;
	
		/* Initialize user application's Stack Pointer */
		__set_MSP(*(__IO uint32_t*) APPLICATIONADDRESS);
	
		Jump_To_Application();
	}
	
	while (1);
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  assert_failed
  *         Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  File: pointer to the source file name
  * @param  Line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif


/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
