/**
  ******************************************************************************
  * @file    STM32F0xx_IAP/src/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    29-May-2012
  * @brief   Main program body
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
#include "main.h"
#include "STM32F0xx_board.h"
#include "common.h"
/** @addtogroup STM32F0xx_IAP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SHOWBOOTLED
#define UPDATE_REGISTER 0x01
#define MCU_VER_POS           0x0803FFFC
#define MCU_SUB_VER_POS       0x0803FFFE
#define SPI_FLASH_START_POS   0x3FD002
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;

/* Private function prototypes -----------------------------------------------*/
static void IAP_Init(void);

/* Private functions ---------------------------------------------------------*/
void IAP_Init(void)
{
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR , ENABLE); 

    ConfigBoot_USART();
    GPIO_Initialization();
    SPI2_LowLevel_initialize();
}

void IAP_Jump_To_APP(void)
{
    /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000 ) == 0x20000000)
    { 
        /* Jump to user application */
        JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
        Jump_To_Application = (pFunction) JumpAddress;

        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

        /* Jump to application */
        Jump_To_Application();
    }
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */

int main(void)
{
    /*!< At this stage the microcontroller clock setting is already configured, 
    this is done through SystemInit() function which is called from startup
    file (startup_stm32f0xx.s) before to branch to application main.
    To reconfigure the default setting of SystemInit() function, refer to
    system_stm32f0xx.c file
    */ 
    uint32 backregister = 0x00;

    IAP_Init();
            
    backregister = RTC_ReadBackupRegister(RTC_BKP_DR0);

    if ( (backregister & UPDATE_REGISTER) == UPDATE_REGISTER ) 
    { 
        FLASH_If_Init(); /* Unlock the Flash Program Erase controller */  
        Main_Menu (); /* grab fw from SPI flash */
    }  
    else
    {   
        IAP_Jump_To_APP();
    }

    while( 1 );
}

/**
  * @brief  Initialize the IAP.
  * @param  None
  * @retval None
  */


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
