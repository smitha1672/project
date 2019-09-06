/**
  ******************************************************************************
  * @file    STM32F0xx_IAP/src/menu.c 
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
#include "common.h"
#include "flash_if.h"
#include "menu.h"
#include "ymodem.h"
#include "Ext_flash_driver.h"
/** @addtogroup STM32F0xx_IAP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifdef EXT_FLASH_UPDATE_MCU
#define MCU_APP_SIZE             (uint32_t)0x3d000
#define VERSION_SIZE             (uint32_t)0x06
#define EXT_FLASH_SIZE           (uint32_t)0x400000
#define EXT_FLASH_MCU_VER_START  (uint32_t)0x400000-0x40000 /* give a 4 sector size*/
#define EXT_FLASH_MCU_START      EXT_FLASH_MCU_VER_START + VERSION_SIZE 
#define EXT_FLASH_MCU_VER_END    (uint32_t)EXT_FLASH_SIZE /*include dummy*/
#define EXT_FLASH_PAGE_SIZE      4096

#define FLASH_STARTADDRESS ((uint32_t)0x08000000) /* Flash Start Address */
#define IAP_SIZE	((uint32_t)0x3000)   /* 12Kbytes as IAP size */
#define FLASH_APPLICATION_STARTADDRESS (FLASH_STARTADDRESS + IAP_SIZE)
uint8_t AMT_RAM_Buf[EXT_FLASH_PAGE_SIZE] ={ 0x00 };
#else
uint8_t tab_1024[1024] ={0};
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;
uint32_t JumpAddress;
extern uint32_t UserMemoryMask;
uint32_t FlashProtection = 0;

uint8_t FileName[FILE_NAME_LENGTH];

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);
void SerialUpload(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Download a file via serial port
  * @param  None
  * @retval None
  */
void SerialDownload(void)
{
 #ifndef EXT_FLASH_UPDATE_MCU
    uint8_t Number[10] = {0};
    int32_t Size = 0;

    SerialPutString("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
    Size = Ymodem_Receive(&tab_1024[0]);
    if (Size > 0)
    {
        SerialPutString("\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
        SerialPutString(FileName);
        Int2Str(Number, Size);
        SerialPutString("\n\r Size: ");
        SerialPutString(Number);
        SerialPutString(" Bytes\r\n");
        SerialPutString("-------------------\n");
    }
    else if (Size == -1)
    {
        SerialPutString("\n\n\rThe image size is higher than the allowed space memory!\n\r");
    }
    else if (Size == -2)
    {
        SerialPutString("\n\n\rVerification failed!\n\r");
    }
    else if (Size == -3)
    {
        SerialPutString("\r\n\nAborted by user.\n\r");
    }
    else
    {
        SerialPutString("\n\rFailed to receive the file!\n\r");
    }
#endif
}

/**
  * @brief  Upload a file via serial port.
  * @param  None
  * @retval None
  */
void SerialUpload(void)
{
    uint8_t status = 0 ; 

    SerialPutString("\n\n\rSelect Receive File\n\r");

    if (GetKey() == CRC16)
    {
        /* Transmit the flash image through ymodem protocol */
        status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS, (const uint8_t*)"UploadedFlashImage.bin", USER_FLASH_SIZE);

        if (status != 0) 
        {
            SerialPutString("\n\rError Occurred while Transmitting File\n\r");
        }
        else
        {
            SerialPutString("\n\rFile uploaded successfully \n\r");
        }
    }
}

#if ( config_EXT_SPI_FLASH_FW_UPDATE == 1 ) 
void UpdateMCUFromSPIFlash(void)
{
    uint32_t flashdestination = FLASH_APPLICATION_STARTADDRESS;    
    uint32_t read_addr = EXT_FLASH_MCU_START;

    //uint32_t value = 0x00;

    uint32_t page_num = MCU_APP_SIZE/EXT_FLASH_PAGE_SIZE;
    uint32_t i = 0;

    /* Unlock the Flash Program Erase controller */
    FLASH_If_Init();

    FLASH_If_Erase(APPLICATION_ADDRESS);

	if ( page_num != 0 )
	{
		for ( i = 0; i <= page_num; i++ )
		{
            sFLASH_ReadBuffer(AMT_RAM_Buf, read_addr, EXT_FLASH_PAGE_SIZE);

            FLASH_If_Write(&flashdestination, (uint32_t*)AMT_RAM_Buf, (uint16_t) EXT_FLASH_PAGE_SIZE/4);

            read_addr = read_addr + EXT_FLASH_PAGE_SIZE; 
		}  

        /* Enable PWR and BKP clocks */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR , ENABLE); 

        /* Enable write access to Backup domain */
        PWR_BackupAccessCmd(ENABLE);

        //value = RTC_ReadBackupRegister(RTC_BKP_DR0);

        RTC_WriteBackupRegister(RTC_BKP_DR0,0x00);
        PWR_BackupAccessCmd(DISABLE);

        SerialPutString("\n\rUpdated!!!! \n\r");
        NVIC_SystemReset();
	}
}
#endif

/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(void)
{
    /* Test if any sector of Flash memory where user application will be loaded is write protected */
    if (FLASH_If_GetWriteProtectionStatus() != 0)  
    {
        FlashProtection = 1;
    }
    else
    {
        FlashProtection = 0;
    }
    while (1)
    {
        SerialPutString("\r\nUpdate Flash from SPI FLASH\r");

#if ( config_EXT_SPI_FLASH_FW_UPDATE == 1 ) 
        UpdateMCUFromSPIFlash();
#endif
    }
}
/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
