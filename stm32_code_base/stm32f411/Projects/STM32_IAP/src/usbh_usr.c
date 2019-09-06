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
#include "command.h"

#include "HT68F30.h"

FATFS fatfs;
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;   //Jerry add
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
EmbeddedFile                fileR;
DirList                     list;
uint8_t joystick_use = 0x00;
uint8_t lcdLineNo = 0x00;


void USBH_USR_IAPInit(void)
{
	HT68F30_LightBar_PowerIndicator( FALSE); /*smith initiates LED bar:  2013/3/22 implemented*/
}

void USBH_USR_Init(void)
{
    HT68F30_LightBar_USB_DeviceAttached( FALSE );
}

void USBH_USR_DeviceAttached(void)
{
	HT68F30_LightBar_USB_DeviceAttached( TRUE );
}

void USBH_USR_UnrecoveredError (void)
{
}

void USBH_USR_DeviceDisconnected (void)
{
	HT68F30_LightBar_USB_DeviceAttached( FALSE );
}

void USBH_USR_ResetDevice(void)
{
}

void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{

}

void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
}

void USBH_USR_DeviceAddressAssigned(void)
{
}

void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
    USBH_InterfaceDesc_TypeDef *itfDesc,
    USBH_EpDesc_TypeDef *epDesc)
{
}

void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
}

void USBH_USR_Product_String(void *ProductString)
{
}

void USBH_USR_SerialNum_String(void *SerialNumString)
{
}

void USBH_USR_EnumerationDone(void)
{
} 

void USBH_USR_DeviceNotSupported(void)
{
}

USBH_USR_Status USBH_USR_UserInput(void)
{
  return USBH_USR_RESP_OK;
}  

void USBH_USR_OverCurrentDetected (void)
{
}

int USBH_USR_MSC_Application(void)
{
    if ( f_mount( 0, &fatfs ) != FR_OK ) 
    {
        /* efs initialisation fails*/
        return(-1);
	}

    COMMAND_IAPExecuteApplication();

    return(0);
	
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

