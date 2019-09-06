/**
  ******************************************************************************
  * @file    usbh_usr.c
  * @author  MCD Application Team
  * @version V2.1.0
  * @date    19-March-2012
  * @brief   This file includes the usb host library user callbacks
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
#include <string.h>
#include "usbh_usr.h"
//#include "lcd_log.h"
#include "ff.h"       /* FATFS */
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"
#include "HT68F30.h"
#include "command.h"
/** @addtogroup USBH_USER
  * @{
  */

/** @defgroup IAP_OVER_USB_USER
  * @brief
  * @{
  */
FATFS fatfs;
//FIL file;
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;   //Jerry add
/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;

USBH_Usr_cb_TypeDef USR_cb =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_MSC_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
    
};

EmbeddedFileSystem          efs;
//EmbeddedFile                file;
EmbeddedFile                fileR;
DirList                     list;
uint8_t joystick_use = 0x00;
uint8_t lcdLineNo = 0x00;

//static const uint8_t MSG_NOT_DIRECTORY[]   = "CAN NOT OPEN THE SELECTED DIRECTORY     ";


/**
  * @brief  USBH_USR_IAPInit
  *         Displays the message on LCD for IAP initialization
  * @param  None
  * @retval None
  */
void USBH_USR_IAPInit(void)
{

#if 0	/*smith mark*/
	/* Set default screen color*/
	LCD_Clear(Black);

	/* Display the application header */
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);

	/* Display LCD message: how to enter IAP mode */
	LCD_DisplayStringLine(Line0, (uint8_t *)MSG_START_IAP);
	LCD_DisplayStringLine(Line5, (uint8_t *)MSG_IAP_ENTER);
	LCD_DisplayStringLine(Line6, (uint8_t *)MSG_CLEAR);
	LCD_DisplayStringLine(Line7, (uint8_t *)MSG_IAP_PRSS);
	LCD_DisplayStringLine(Line8, (uint8_t *)MSG_CLEAR);
	LCD_DisplayStringLine(Line9, (uint8_t *)MSG_IAP_KEY);
#endif 

	HT68F30_LightBar_PowerIndicator( FALSE); /*smith initiates LED bar:  2013/3/22 implemented*/

  
}

/**
  * @brief  USBH_USR_Init
  *         Displays the message on LCD for host lib initialization
  * @param  None
  * @retval None
  */
void USBH_USR_Init(void)
{
#if 0
	/* Set default screen color*/
	LCD_Clear(Black);

	/* Display LCD message: how to enter IAP mode */
	LCD_DisplayStringLine(0, (uint8_t *)MSG_START_IAP);

	/* Set back color to black */
	LCD_SetBackColor(Black);
#endif 	
    HT68F30_LightBar_USB_DeviceAttached( FALSE );
}

/**
  * @brief  USBH_USR_DeviceAttached
  *         Displays the message on LCD on device attached
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAttached(void)
{
	/* LCD message device attached */
	//LCD_DisplayStringLine(80, (uint8_t *)MSG_DEV_ATTACHED);

	HT68F30_LightBar_USB_DeviceAttached( TRUE );
}


/**
  * @brief  USBH_USR_UnrecoveredError
  * @param  None
  * @retval None
  */
void USBH_USR_UnrecoveredError (void)
{

  /* Set default screen color*/
  //LCD_Clear(Black);

#if 0	/*smith mark*/
  /* Display the application header */
  LCD_SetBackColor(Blue);
  LCD_SetTextColor(White);
  LCD_DisplayStringLine(0, (uint8_t *)MSG_START_IAP);
#endif   

  /* LCD message device error  */
  //LCD_SetBackColor(Red);
  //LCD_DisplayStringLine(20,  (uint8_t *)MSG_UNREC_ERROR);
}

/**
  * @brief  USBH_DisconnectEvent
  *         Device disconnect event
  * @param  None
  * @retval Staus
  */
void USBH_USR_DeviceDisconnected (void)
{

  /* Set default screen color*/
  //LCD_Clear(Black);

#if 0	/*smith mark*/
  /* Display the application header */
  LCD_SetBackColor(Blue);
  LCD_SetTextColor(White);
  LCD_DisplayStringLine(0, (uint8_t *)MSG_START_IAP);
#endif   

  /* LCD message device disconnecting */
  //LCD_SetBackColor(Black);
  //LCD_DisplayStringLine(80, (uint8_t *)MSG_DEV_DISCONNECTED);

	HT68F30_LightBar_USB_DeviceAttached( FALSE );

}

/**
  * @brief  USBH_USR_ResetUSBDevice
  * @param  None
  * @retval None
  */
void USBH_USR_ResetDevice(void)
{
  /* callback for USB-Reset */
}


/**
  * @brief  USBH_USR_DeviceSpeedDetected
  *         Displays the message on LCD for device speed
  * @param  Device speed:
  * @retval None
  */
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
	//!	Smith mark	@{
#if 0
	if (DeviceSpeed == HPRT0_PRTSPD_FULL_SPEED)
	{
		LCD_DisplayStringLine(100, (uint8_t *)MSG_DEV_FULLSPEED);
	}
	else if (DeviceSpeed == HPRT0_PRTSPD_LOW_SPEED)
	{
		LCD_DisplayStringLine(100, (uint8_t *)MSG_DEV_LOWSPEED);
	}
	else
	{
		LCD_DisplayStringLine(100, (uint8_t *)MSG_DEV_ERROR);
	}
#endif	
	//!	 @}

}

/**
  * @brief  USBH_USR_Device_DescAvailable
  * @param  device descriptor
  * @retval None
  */
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
  /* callback for device descriptor */
}

/**
  * @brief  USBH_USR_DeviceAddressAssigned
  *         USB device is successfully assigned the Address
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAddressAssigned(void)
{
  /* callback for device successfully assigned the Address */
}


/**
  * @brief  USBH_USR_Conf_Desc
  * @param  Configuration descriptor
  * @retval None
  */
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
    USBH_InterfaceDesc_TypeDef *itfDesc,
    USBH_EpDesc_TypeDef *epDesc)
{
  /* callback for configuration descriptor */
}

/**
  * @brief  USBH_USR_Manufacturer_String
  * @param  Manufacturer String
  * @retval None
  */
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  /* callback for  Manufacturer String */
}

/**
  * @brief  USBH_USR_Product_String
  * @param  Product String
  * @retval None
  */
void USBH_USR_Product_String(void *ProductString)
{
  /* callback for Product String */
}

/**
  * @brief  USBH_USR_SerialNum_String
  * @param  SerialNum_String
  * @retval None
  */
void USBH_USR_SerialNum_String(void *SerialNumString)
{
  /* callback for SerialNum_String */
}

/**
* @brief  EnumerationDone 
*         User response request is displayed to ask application jump to class
* @param  None
* @retval None
*/
void USBH_USR_EnumerationDone(void)
{
#if 0  
  /* Enumeration complete */
  LCD_UsrLog((void *)MSG_DEV_ENUMERATED);
  
  LCD_SetTextColor(Green);
  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "To see the root content of the disk : " );
  LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "Press Key...                       ");
  LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR); 
#endif  
} 



/**
  * @brief  USBH_USR_DeviceNotSupported
  *         Device is not supported
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceNotSupported(void)
{
	//! Smith mark	@{
#if 0	

	/* Set default screen color */
	LCD_Clear(Black);

	/* Display the application header */
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	LCD_DisplayStringLine(0, (uint8_t*)MSG_START_IAP);

	/* Device not supported */
	LCD_SetBackColor(Black);
	LCD_DisplayStringLine(40, (uint8_t*)MSG_DEV_NSUPPORTED);

	/* 2 seconds delay */
	USB_OTG_BSP_mDelay(2000);
	
#endif 	
	//! @}

}


/**
  * @brief  USBH_USR_UserInput
  *         User Action for application state entry
  * @param  None
  * @retval USBH_USR_Status : User response for key button
  */
USBH_USR_Status USBH_USR_UserInput(void)
{
#if 0
  USBH_USR_Status usbh_usr_status;
  
  usbh_usr_status = USBH_USR_NO_RESP;  
  
  /*Key B3 is in polling mode to detect user action */
  if(STM_EVAL_PBGetState(Button_WAKEUP) == RESET) 
  {
    
    usbh_usr_status = USBH_USR_RESP_OK;
    
  } 
  return usbh_usr_status;
  #endif
  return USBH_USR_RESP_OK;
}  

/**
  * @brief  USBH_USR_OverCurrentDetected
  *         Over Current Detected on VBUS
  * @param  None
  * @retval None
  */
void USBH_USR_OverCurrentDetected (void)
{
  //LCD_ErrLog ("Overcurrent detected.");
}

/**
  * @brief  USBH_USR_MSC_Application
  *         Demo application for IAP thru USB mass storage
  * @param  None
  * @retval Staus
  */
int USBH_USR_MSC_Application(void)
{

    if ( f_mount( 0, &fatfs ) != FR_OK ) 
    {
        /* efs initialisation fails*/
        return(-1);
	}

    COMMAND_IAPExecuteApplication();

    return(0);
#if 1	/*Smith modify */
#if 0
	/* Initialises the EFS lib*/
	if ( f_mount( 0, &fatfs ) != FR_OK ) 
	{
		/* efs initialisation fails*/
		return(-1);
	}
	
	/* Control the select of commands */
	if (USBH_USR_SelectControlMenu() == DEVICE_DISCONNECT )
		return (-1);

	/* Go to IAP menu */
	COMMAND_IAPExecuteApplication();
#endif
#else
  switch(USBH_USR_ApplicationState)
  {
  case USH_USR_FS_INIT: 
    
    /* Initialises the File System*/
    if ( f_mount( 0, &fatfs ) != FR_OK ) 
    {
      /* efs initialisation fails*/
      //LCD_ErrLog("> Cannot initialize File System.\n");
      return(-1);
    }
    //LCD_UsrLog("> File System initialized.\n");
    //LCD_UsrLog("> Disk capacity : %d Bytes\n", USBH_MSC_Param.MSCapacity * \
      USBH_MSC_Param.MSPageLength); 
    
    if(USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
    {
      //LCD_ErrLog((void *)MSG_WR_PROTECT);
    }
    
    USBH_USR_ApplicationState = USH_USR_FS_READLIST;
    break;
    
  case USH_USR_FS_READLIST:
    
    //LCD_UsrLog((void *)MSG_ROOT_CONT);
    Explore_Disk("0:/", 1);
    line_idx = 0;   
    USBH_USR_ApplicationState = USH_USR_FS_WRITEFILE;
    
    break;
    
  case USH_USR_FS_WRITEFILE:
    
    //LCD_SetTextColor(Green);
    //LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "                                              ");
    //LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "Press Key to write file");
    //LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR); 
    USB_OTG_BSP_mDelay(100);
    
    /*Key B3 in polling*/
    while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
      (STM_EVAL_PBGetState (BUTTON_WAKEUP) == SET))          
    {
      //Toggle_Leds();
    }
    /* Writes a text file, STM32.TXT in the disk*/
    //LCD_UsrLog("> Writing File to disk flash ...\n");
    if(USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
    {
      
      //LCD_ErrLog ( "> Disk flash is write protected \n");
      USBH_USR_ApplicationState = USH_USR_FS_DRAW;
      break;
    }
    
    /* Register work area for logical drives */
    f_mount(0, &fatfs);
    
    if(f_open(&file, "0:upgrade.bin",FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
    { 
      /* Write buffer to file */
      bytesToWrite = sizeof(writeTextBuff); 
      res= f_write (&file, writeTextBuff, bytesToWrite, (void *)&bytesWritten);   
      
      if((bytesWritten == 0) || (res != FR_OK)) /*EOF or Error*/
      {
        //LCD_ErrLog("> STM32.TXT CANNOT be writen.\n");
      }
      else
      {
        //LCD_UsrLog("> 'STM32.TXT' file created\n");
      }
      
      /*close file and filesystem*/
      f_close(&file);
      f_mount(0, NULL); 
    }
    
    else
    {
      LCD_UsrLog ("> STM32.TXT created in the disk\n");
    }

    USBH_USR_ApplicationState = USH_USR_FS_DRAW; 
    
    LCD_SetTextColor(Green);
    LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "                                              ");
    LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "To start Image slide show Press Key.");
    LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR); 
  
    break;
    
  case USH_USR_FS_DRAW:
    
    /*Key B3 in polling*/
    while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
      (STM_EVAL_PBGetState (BUTTON_KEY) == SET))
    {
      Toggle_Leds();
    }
  
    while(HCD_IsDeviceConnected(&USB_OTG_Core))
    {
      if ( f_mount( 0, &fatfs ) != FR_OK ) 
      {
        /* fat_fs initialisation fails*/
        return(-1);
      }
      return Image_Browser("0:/");
    }
    break;
  default: break;
  }
	return(0);
#endif 

	
}

/**
* @brief  Explore_Disk 
*         Displays disk content
* @param  path: pointer to root path
* @retval None
*/
#if 0
static uint8_t Explore_Disk (char* path , uint8_t recu_level)
{

  FRESULT res;
  FILINFO fno;
  DIR dir;
  char *fn;
  char tmp[14];
  
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    while(HCD_IsDeviceConnected(&USB_OTG_Core)) 
    {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) 
      {
        break;
      }
      if (fno.fname[0] == '.')
      {
        continue;
      }

      fn = fno.fname;
      strcpy(tmp, fn); 

      line_idx++;
      if(line_idx > 9)
      {
        line_idx = 0;
        LCD_SetTextColor(Green);
        LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 42, "                                              ");
        LCD_DisplayStringLine( LCD_PIXEL_HEIGHT - 30, "Press Key to continue...");
        LCD_SetTextColor(LCD_LOG_DEFAULT_COLOR); 
        
        /*Key B3 in polling*/
        while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
          (STM_EVAL_PBGetState (BUTTON_KEY) == SET))
        {
          Toggle_Leds();
          
        }
      } 
      
      if(recu_level == 1)
      {
        LCD_DbgLog("   |__");
      }
      else if(recu_level == 2)
      {
        LCD_DbgLog("   |   |__");
      }
      if((fno.fattrib & AM_MASK) == AM_DIR)
      {
        strcat(tmp, "\n"); 
        LCD_UsrLog((void *)tmp);
      }
      else
      {
        strcat(tmp, "\n"); 
        LCD_DbgLog((void *)tmp);
      }

      if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recu_level == 1))
      {
        Explore_Disk(fn, 2);
      }
    }
  }
  return res;
}

static uint8_t Image_Browser (char* path)
{
  FRESULT res;
  uint8_t ret = 1;
  FILINFO fno;
  DIR dir;
  char *fn;
  
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0) break;
      if (fno.fname[0] == '.') continue;

      fn = fno.fname;
 
      if (fno.fattrib & AM_DIR) 
      {
        continue;
      } 
      else 
      {
        if((strstr(fn, "bmp")) || (strstr(fn, "BMP")))
        {
          res = f_open(&file, fn, FA_OPEN_EXISTING | FA_READ);
          Show_Image();
          USB_OTG_BSP_mDelay(100);
          ret = 0;
          while((HCD_IsDeviceConnected(&USB_OTG_Core)) && \
            (STM_EVAL_PBGetState (BUTTON_KEY) == SET))
          {
            Toggle_Leds();
          }
          f_close(&file);
          
        }
      }
    }  
  }
  
  #ifdef USE_USB_OTG_HS 
  LCD_LOG_SetHeader(" USB OTG HS MSC Host");
#else
  LCD_LOG_SetHeader(" USB OTG FS MSC Host");
#endif
  LCD_LOG_SetFooter ("     USB Host Library v2.1.0" );
  LCD_UsrLog("> Disk capacity : %d Bytes\n", USBH_MSC_Param.MSCapacity * \
      USBH_MSC_Param.MSPageLength); 
  USBH_USR_ApplicationState = USH_USR_FS_READLIST;
  return ret;
}

/**
* @brief  Show_Image 
*         Displays BMP image
* @param  None
* @retval None
*/
static void Show_Image(void)
{
  
  uint16_t i = 0;
  uint16_t numOfReadBytes = 0;
  FRESULT res; 
  
  LCD_SetDisplayWindow(239, 319, 240, 320);
  LCD_WriteReg(R3, 0x1008);
  LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  
  /* Bypass Bitmap header */ 
  f_lseek (&file, 54);
  
  while (HCD_IsDeviceConnected(&USB_OTG_Core))
  {
    res = f_read(&file, Image_Buf, IMAGE_BUFFER_SIZE, (void *)&numOfReadBytes);
    if((numOfReadBytes == 0) || (res != FR_OK)) /*EOF or Error*/
    {
      break; 
    }
    for(i = 0 ; i < IMAGE_BUFFER_SIZE; i+= 2)
    {
      LCD_WriteRAM(Image_Buf[i+1] << 8 | Image_Buf[i]); 
    } 
  }
  
}

/**
* @brief  Toggle_Leds
*         Toggle leds to shows user input state
* @param  None
* @retval None
*/
static void Toggle_Leds(void)
{
  static uint32_t i;
  if (i++ == 0x10000)
  {
    STM_EVAL_LEDToggle(LED1);
    STM_EVAL_LEDToggle(LED2);
    STM_EVAL_LEDToggle(LED3);
    STM_EVAL_LEDToggle(LED4);
    i = 0;
  }  
}
#endif
/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/
void USBH_USR_DeInit(void)
{
  USBH_USR_ApplicationState = USH_USR_FS_INIT;
}

uint8_t USBH_USR_SelectControlMenu(void)
{
	uint8_t status = DEVICE_DISCONNECT;

	if (HCD_IsDeviceConnected(&USB_OTG_Core) == 0)
	{
		status = DEVICE_DISCONNECT;
		return status;
	}

	status = NEXT_DOWNLOAD;

	/* 0.8 seconds delay */
	USB_OTG_BSP_mDelay(800);

	return status;
}

/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

