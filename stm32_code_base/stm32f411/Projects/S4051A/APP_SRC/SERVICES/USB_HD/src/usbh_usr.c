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
#include "ff.h"       /* FATFS */
#include "usbh_msc_core.h"
#include "usbh_msc_scsi.h"
#include "usbh_msc_bot.h"


#include "USBFileSearch.h"

//#include "MusicPlaybackDispatcher.h"
//#include "NoiseGeneratorDispatcher.h" /* Angus remove for pass compiler */


#include "USBMusicManager.h"
#include "USBMediaManager.h"


extern USB_FILE_SEARCH_OBJECT *pUSBFSE_Object;
extern USB_MEDIA_MANAGE_OBJECT *pUSBMedia_ObjCtrl;
extern USB_MUSIC_MANAGE_OBJECT *pUSB_MMP_ObjCtrl;


//extern void USB_Media_StateDispatcher(void);
//extern bool IsNoiseGeneratorMode(void);


//Mike
uint8_t bUsbMount = 0;
//Angus
extern void MSCPB_Set_getTime(void);
static int USB_Ever_UnMount = FALSE;

/** @addtogroup USBH_USER
  * @{
  */

/** @defgroup IAP_OVER_USB_USER
  * @brief
  * @{
  */

typedef enum
{
	USH_USR_FS_INIT = 0,
	USH_USR_FS_FileSearch,
	USH_USR_FS_PLAY,
	USH_USR_FS_WAIT
 
} USBH_USER_APP_STATE;

FATFS fatfs;

extern USB_OTG_CORE_HANDLE          USB_OTG_Core;   

/*  The purpose of this register is to speed up the execution */
USBH_USER_APP_STATE USBH_USR_ApplicationState = USH_USR_FS_INIT;

USBH_Usr_cb_TypeDef USR_USBH_MSC_cb =
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



/**
  * @brief  USBH_USR_IAPInit
  *         Displays the message on LCD for IAP initialization
  * @param  None
  * @retval None
  */
void USBH_USR_IAPInit(void)
{
  
}

/**
  * @brief  USBH_USR_Init
  *         Displays the message on LCD for host lib initialization
  * @param  None
  * @retval None
  */
void USBH_USR_Init(void)
{
    bUsbMount = 0;
}

/**
  * @brief  USBH_USR_DeviceAttached
  *         Displays the message on LCD on device attached
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAttached(void)
{
}


/**
  * @brief  USBH_USR_UnrecoveredError
  * @param  None
  * @retval None
  */
void USBH_USR_UnrecoveredError (void)
{
}

/**
  * @brief  USBH_DisconnectEvent
  *         Device disconnect event
  * @param  None
  * @retval Staus
  */
void USBH_USR_DeviceDisconnected (void)
{
    /*Smith Implements: 2013/5/30 */
    f_mount(0, NULL);
    bUsbMount = 0;
    USBH_Set_USB_Ever_UnMount_State();

    //	MSCPB_Set_Play_State(); /* Angus remove for pass compiler */	//Angus add,SB need auto play music when USB be pause and re-plug USB again.
    //	USB_Reset_PlayIndex();  /* Angus remove for pass compiler */  

    pUSBFSE_Object->init();

    //20130625 Angus add. if USB unmount that need calulate time of timeout. 
    //First time into demo2 it will time out at 30seconds, so regardless of USB unmount, otherwise it will over 30 seconds.
#if 0   /* Angus remove for pass compiler */
    if(MSCPB_Get_Demo2_Play_State()!=FALSE)
    MSCPB_Set_getTime();
#endif
    USBH_USR_ApplicationState = USH_USR_FS_INIT;
    pUSB_MMP_ObjCtrl->init();
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
} 



/**
  * @brief  USBH_USR_DeviceNotSupported
  *         Device is not supported
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceNotSupported(void)
{

}


/**
  * @brief  USBH_USR_UserInput
  *         User Action for application state entry
  * @param  None
  * @retval USBH_USR_Status : User response for key button
  */
USBH_USR_Status USBH_USR_UserInput(void)
{
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
}

/**
  * @brief  USBH_USR_MSC_Application
  *         Demo application for IAP thru USB mass storage
  * @param  None
  * @retval Staus
  */
int USBH_USR_MSC_Application(void)
{	
    #if 0   /* Angus remove for pass compiler */
        if(IsNoiseGeneratorMode())
        {
            USBH_USR_ApplicationState = USH_USR_FS_INIT;
            return(USBH_OK);
        }
    #endif


    #if 1 /* Angus debug 2014/2/21 */
	switch(USBH_USR_ApplicationState)
	{
		case USH_USR_FS_INIT:
		{	

                    pUSBFSE_Object->init();
            
			/* Initialises the File System*/
			if ( f_mount( 0, &fatfs ) != FR_OK ) 
			{
				bUsbMount = 0;

				return(USBH_APPLY_DEINIT);
			}
			else
			{
				bUsbMount = 1;

				USBH_USR_ApplicationState = USH_USR_FS_FileSearch;
			}
		}

		break;
		case USH_USR_FS_FileSearch:
		{
			pUSBFSE_Object->fileSearchTask();
        
                    if(pUSBFSE_Object->isSearchDone() == TRUE)
                    {
                        USBH_USR_ApplicationState = USH_USR_FS_PLAY;
                    }
		}
		break;
		case USH_USR_FS_PLAY:
		{	
		      pUSBMedia_ObjCtrl->mediaManagerTask();
		}
			break;
	}
    #else
        pUSBMedia_ObjCtrl->mediaManagerTask();
    #endif
	return(USBH_OK);

	
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
    bUsbMount = 0;
    
    pUSBFSE_Object->init();
    pUSB_MMP_ObjCtrl->init();
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

uint32_t USBH_IsDeviceConnect( void )
{
	return HCD_IsDeviceConnected(&USB_OTG_Core);
}

uint8_t USBH_isDeviceMount( void )
{
	return bUsbMount;

}

uint8_t USBH_UnMount(void){
	if(f_mount( 0, NULL ) == FR_OK ){
		bUsbMount = 0;
	}
	return bUsbMount;
}
uint8_t USBH_Mount(void){
	if(f_mount( 0, &fatfs ) == FR_OK ){
		bUsbMount = 1;
	} 
	return bUsbMount;
}

void USBH_Set_USB_Ever_UnMount_State(void){
	USB_Ever_UnMount = 1;
}

void USBH_ReSet_USB_Ever_UnMount_State(void){
	USB_Ever_UnMount = 0;
}

int USBH_Get_USB_Ever_UnMount_State(void){
	return USB_Ever_UnMount;
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

