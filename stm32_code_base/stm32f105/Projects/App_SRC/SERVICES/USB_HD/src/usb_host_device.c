/**
  ******************************************************************************
  * @file    app.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file provides all the Application firmware functions.
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
#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"


#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usb_conf.h"
#include "usbd_desc.h"

#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "ff.h"  

#include "USBMusicManager.h"
#include "usb_host_device.h"
#include "GPIOMiddleLevel.h"

typedef struct USB_MUSIC_HANDLE_PARAMETERS
{
    xTaskHandle TaskHandle;
    xTaskHandle TaskMSCErrHandle;
    xOS_TaskErrIndicator xOS_ErrId;
    xSemaphoreHandle TaskSema;
} xUSBHostDeviceHandleParams;

typedef enum {
  USB_HD_IDLE   = 0,
  USB_HD_WAIT,  
  USB_HD_DEVICE,
  USB_HD_HOST,
}USB_HD_STATE;

typedef enum {
  USB_DEVICE_IDLE   = 0,
  USB_DEVICE_WAIT,  
}USB_DEVICE_STATE;

typedef enum {
  USB_HOST_IDLE   = 0,
  USB_HOST_WAIT,  
}USB_HOST_STATE;

typedef struct _USB_HD_StateMachine
{
    USB_HD_STATE	state;
    USB_HOST_STATE	HostState;
    USB_DEVICE_STATE	DeviceState;
    USB_MODE mode;
    uint8_t             select;
  
}USB_HD_STATE_MACHINE;


//#define USBHostDeviceHandler_StateMonitor
#ifdef USBHostDeviceHandler_StateMonitor   /* for monitor */
portBASE_TYPE USBHostDeviceHandlerPriority = 0xff;
portBASE_TYPE USBHostDeviceHandlerStack = 0;
#endif

//____________________________________________________________________________________________________________
/* extern variable */

HOST_State usb_state;
int MSC_ErrCnt = 0;
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
   
__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_Core __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST USB_Host __ALIGN_END ;


//____________________________________________________________________________________________________________
/*static variable*/
static USB_HD_STATE_MACHINE mUSB_HD_State = {USB_HD_DEVICE, USB_HOST_IDLE, USB_DEVICE_IDLE, USB_DEVICE, USB_HOST_DEVICE_CMD_VCP_CONNECT}; /*Default is device mode.*/
static xUSBHostDeviceHandleParams mUSBHostDeviceHandleParams;

static uint8_t prev_select_mode = USB_DEVICE;

//____________________________________________________________________________________________________________
//static api header
static void USBHostDeviceHandler_CreateTask(void);

static void USBHostDeviceHandle_Switch(uint8 usb_mode_cmd);

static void USBHostDeviceHandler_ServiceHandle( void *pvParameters );

static USB_MODE USBHostDeviceHandler_get_mode( void );

//____________________________________________________________________________________________________________
extern CDC_IF_Prop_TypeDef  APP_FOPS;
extern USB_MUSIC_MANAGE_OBJECT *pUSB_MMP_ObjCtrl;

const USB_HOST_DEVICE_HANDLE_OBJECT   USBHostDeviceHandle =
{
    USBHostDeviceHandler_CreateTask,
    USBHostDeviceHandle_Switch,
    USBHostDeviceHandler_get_mode,
};
const USB_HOST_DEVICE_HANDLE_OBJECT *pUSB_HD_ObjCtrl = &USBHostDeviceHandle;


//____________________________________________________________________________________________________________ 
void usart_putchar( const char c )
{
	uint8_t buf = (uint8_t)c;

	APP_FOPS.pIf_DataTx(&buf, 1);
}
#if 0
static void USBHostDeviceHandler_MSC_ErrHandle( void *pvParameters )
{
    for( ;; )
    {
        USBHostDeviceHandle_Switch( USB_HOST_DEVICE_CMD_VCP_CONNECT );
        vTaskDelay( TASK_MSEC2TICKS(500));
        USBHostDeviceHandle_Switch(USB_HOST_DEVICE_CMD_MSC_PROCESS);
        vTaskSuspend( mUSBHostDeviceHandleParams.TaskMSCErrHandle );
    }
}
#endif

static void USBHostDeviceHandle_Switch_to_Host(bool b_Set_Mode)
{
    //return;
    __disable_irq();
    __enable_irq();

    USBH_Init(&USB_OTG_Core, 
#ifdef USE_USB_OTG_FS  
    	USB_OTG_FS_CORE_ID,
#else 
    	USB_OTG_HS_CORE_ID,
#endif 
    	&USB_Host,
    	&USBH_MSC_cb, 
    	&USR_USBH_MSC_cb);

    USB_OTG_DisableGlobalInt(&USB_OTG_Core);
    USB_OTG_EnableGlobalInt(&USB_OTG_Core);

    //VirtualTimer_sleep(10);
    
    GPIOMiddleLevel_Set(__O_USB_VIN);
    
    if (b_Set_Mode)
    {
        mUSB_HD_State.mode = USB_HOST;
    }
}

static void USBHostDeviceHandle_Switch_to_Device(bool b_Set_Mode)
{
    //return;
    __disable_irq();
    __enable_irq();

    GPIOMiddleLevel_Clr( __O_USB_VIN);
    
    USBD_Init(&USB_OTG_Core,
#ifdef USE_USB_OTG_HS 
        USB_OTG_HS_CORE_ID,
#else            
        USB_OTG_FS_CORE_ID,
#endif  
        &USR_desc, 
        &USBD_CDC_cb,    
        &USR_cb);

    __disable_irq();
    __enable_irq();

    if (b_Set_Mode)
    {
        mUSB_HD_State.mode = USB_DEVICE;
    }
}

static void USBHostDeviceHandler_Init(void)
{
    //Default setting : Device mode    
    prev_select_mode = mUSB_HD_State.mode;
    USBHostDeviceHandle_Switch_to_Device(TRUE);
    //USBHostDeviceHandle_Switch_to_Host();
}

static void USBHostDeviceHandler_CreateTask(void)
{
    USBHostDeviceHandler_Init();

    vSemaphoreCreateBinary(mUSBHostDeviceHandleParams.TaskSema);

    if (mUSBHostDeviceHandleParams.TaskSema == NULL)
    {
        TRACE_DEBUG((0,"error create usb sema"));
    }
   
    if ( xTaskCreate( 
         USBHostDeviceHandler_ServiceHandle, 
         ( portCHAR * ) "USB_HostDevice", 
         (configMINIMAL_STACK_SIZE*2), 
         NULL, 
         tskUSB_DEVICE_PRIORITY, 
         &mUSBHostDeviceHandleParams.TaskHandle ) != pdPASS )
    {
        mUSBHostDeviceHandleParams.xOS_ErrId = xOS_TASK_CREATE_FAIL;
        vTaskDelete( mUSBHostDeviceHandleParams.TaskHandle );
    }


#if 0
    if ( xTaskCreate( 
         USBHostDeviceHandler_MSC_ErrHandle, 
         ( portCHAR * ) "USB_MSCErrHandle", 
         (configMINIMAL_STACK_SIZE), 
         NULL, 
         tskUSB_DEVICE_PRIORITY, 
         &mUSBHostDeviceHandleParams.TaskMSCErrHandle ) != pdPASS )
    {
        mUSBHostDeviceHandleParams.xOS_ErrId = xOS_TASK_CREATE_FAIL;
        vTaskDelete( mUSBHostDeviceHandleParams.TaskMSCErrHandle );
    }
    else
    {
        vTaskSuspend( mUSBHostDeviceHandleParams.TaskMSCErrHandle );
    }
#endif
}

static USB_MODE USBHostDeviceHandler_get_mode( void )
{
    return mUSB_HD_State.mode;
}

static void USBHostDeviceHandle_Switch( uint8 usb_mode_cmd )
{
    mUSB_HD_State.state = USB_HD_IDLE;
    //vTaskSuspend( mUSBHostDeviceHandleParams.TaskHandle );
    
    if( ( mUSB_HD_State.select == USB_HOST_DEVICE_CMD_MSC_PROCESS ) && (usb_mode_cmd == USB_HOST_DEVICE_CMD_VCP_CONNECT) )
    {
        //unmount
        f_mount(0, NULL);
        USB_OTG_StopHost(&USB_OTG_Core);
        USBH_DeInit(&USB_OTG_Core, &USB_Host);
        USB_Host.usr_cb->DeInit();
        USB_Host.class_cb->DeInit(&USB_OTG_Core, &USB_Host.device_prop);

        GPIOMiddleLevel_Clr( __O_USB_VIN);

        /*TODO: for usb */
        //mUSB_HD_State.select = USB_HOST_DEVICE_CMD_MSC_UNMOUNT;
        //vTaskDelay( TASK_MSEC2TICKS(500));

        mUSB_HD_State.select = USB_HOST_DEVICE_CMD_VCP_CONNECT;
        //vTaskDelay( TASK_MSEC2TICKS(500));
        
        USBHostDeviceHandle_Switch_to_Device(TRUE);
        
        vTaskPrioritySet(mUSBHostDeviceHandleParams.TaskHandle,tskUSB_DEVICE_PRIORITY);
    }
    else if( ( mUSB_HD_State.select == USB_HOST_DEVICE_CMD_VCP_CONNECT ) && (usb_mode_cmd == USB_HOST_DEVICE_CMD_MSC_PROCESS) )
    {
        /*TODO: for usb */
        //mUSB_HD_State.select = USB_HOST_DEVICE_CMD_VCP_DISCONNECT;
        //vTaskDelay( TASK_MSEC2TICKS(500));
        //Disconnect        
        DCD_DevDisconnect (&USB_OTG_Core);
        USB_OTG_StopDevice(&USB_OTG_Core);  

        mUSB_HD_State.select = USB_HOST_DEVICE_CMD_MSC_PROCESS;
        //vTaskDelay( TASK_MSEC2TICKS(500));
        
        USBHostDeviceHandle_Switch_to_Host(TRUE);
        
        vTaskPrioritySet(mUSBHostDeviceHandleParams.TaskHandle,tskUSB_HOST_PRIORITY);
    }
    //vTaskResume( mUSBHostDeviceHandleParams.TaskHandle );
    mUSB_HD_State.state = USB_HD_WAIT;
    xSemaphoreGive(mUSBHostDeviceHandleParams.TaskSema);
}

static void USBHostDeviceHandle_Recovery()
{
    //unmount
#if 0    
    f_mount(0, NULL);
    USB_OTG_StopHost(&USB_OTG_Core);
    USBH_DeInit(&USB_OTG_Core, &USB_Host);
    USB_Host.usr_cb->DeInit();
    USB_Host.class_cb->DeInit(&USB_OTG_Core, &USB_Host.device_prop);
#endif
    USBH_DeInit(&USB_OTG_Core, &USB_Host);
    USB_Host.usr_cb->DeInit();
    USB_Host.class_cb->DeInit(&USB_OTG_Core, &USB_Host.device_prop);

    USB_Host.gState = HOST_ERROR_STATE; 
    USB_Host.usr_cb->Init();

    USBHostDeviceHandle_Switch_to_Device(FALSE);
    //VirtualTimer_sleep(100);
    USBHostDeviceHandle_Switch_to_Host(FALSE);
}

#if 0
static void USBHostDeviceHandler_ServiceHandle( void *pvParameters )
{
    static uint8_t prev_select = 0;
    uint8_t select_value = 0;
    
    for(;;)
    {
    	switch( mUSB_HD_State.state )
    	{
    		case USB_HD_IDLE:
    		{
    			__disable_irq();
    			__enable_irq();
    			mUSB_HD_State.state = USB_HD_WAIT;
    			mUSB_HD_State.select = 0;
    		}
    			break;

    		case USB_HD_WAIT:
    		{	
    			if( mUSB_HD_State.select != prev_select )
    			{
    				prev_select = mUSB_HD_State.select;
    			
    				if( mUSB_HD_State.select & 0x80 )
    				{
    					select_value = (mUSB_HD_State.select & 0x01);

    					switch( select_value )
    					{
    						case 0:
    						{
    							mUSB_HD_State.state = USB_HD_HOST;  
    							mUSB_HD_State.HostState = USB_HOST_IDLE;	
    						}
    							break;

    						case 1:
    						{
    							mUSB_HD_State.state = USB_HD_DEVICE;  
    							mUSB_HD_State.DeviceState = USB_DEVICE_IDLE;	
    						}
    							break;

    						default:
    						{	
    							mUSB_HD_State.state = USB_HD_IDLE;
    						}
    						
    						break;
    					}
    				}
    			}
    		}
    		
    			break;

    		case USB_HD_HOST:
    		{
    			switch( mUSB_HD_State.HostState )
    			{
    				case USB_HOST_IDLE:
    				{
    					USBH_Init(&USB_OTG_Core, 
#ifdef USE_USB_OTG_FS  
    						USB_OTG_FS_CORE_ID,
#else 
    						USB_OTG_HS_CORE_ID,
#endif 
    						&USB_Host,
    						&USBH_MSC_cb, 
    						&USR_USBH_MSC_cb);

    					mUSB_HD_State.HostState = USB_HOST_WAIT;

    					USB_OTG_DisableGlobalInt(&USB_OTG_Core);
    					USB_OTG_EnableGlobalInt(&USB_OTG_Core);
    				}
    					break;

    				case USB_HOST_WAIT:
    				{
#if 0	/*fix: file list cannot be claified to ZERO when USB flash driver was be re-plugged*/					
    					if (!HCD_IsDeviceConnected(&USB_OTG_Core))
    					{
    						/* Re-Initilaize Host for new Enumeration */
    						USBH_DeInit(&USB_OTG_Core, &USB_Host);
    					}
#endif					
    					
    					GPIOMiddleLevel_Set(__O_USB_VIN);	
    					/* Handle select item */
    					if(mUSB_HD_State.select & 0x80)
    					{
    						select_value = (mUSB_HD_State.select & 0x0E ) >> 1;
    		
    						switch ( select_value )
    						{
    							case 0:
    							{
    								/*NO PROCESS !!*/
    							}
    								break;
    						
    							case  1: /* USB_HOST_DEVICE_CMD_MSC_PROCESS */
    							{
    								usb_state = USBH_Process(&USB_OTG_Core, &USB_Host);
    								mUSB_HD_State.mode = USB_HOST;

    								if ( usb_state == HOST_CTRL_XFER )
    								{
                                        MSC_ErrCnt ++;

                                        if ( MSC_ErrCnt > 100 )
                                        {
                                            if ( xTaskIsTaskSuspended( mUSBHostDeviceHandleParams.TaskMSCErrHandle ) == pdPASS ) /*task is working*/
                                            {
                                                vTaskResume( mUSBHostDeviceHandleParams.TaskMSCErrHandle );
                                            }
                                            
                                            MSC_ErrCnt = 0 ;
                                        }
    								}
    								else
    								{
                                        MSC_ErrCnt = 0;
    								}
    							}
    								break;

    							case 2:	/* USB_HOST_DEVICE_CMD_MSC_UNMOUNT */
    							{
    								mUSB_HD_State.state = USB_HD_IDLE;
    								f_mount(0, NULL);
    								USB_OTG_StopHost(&USB_OTG_Core);
    								
    								/* Re-Initilaize Host for new Enumeration */
    								USBH_DeInit(&USB_OTG_Core, &USB_Host);
    								USB_Host.usr_cb->DeInit();
    								USB_Host.class_cb->DeInit(&USB_OTG_Core, &USB_Host.device_prop);
    							}
    								break;

    							default:
    							{
    								mUSB_HD_State.state = USB_HD_WAIT;
    								mUSB_HD_State.HostState = USB_HOST_IDLE;	
    								f_mount(0, NULL);
    								USB_OTG_StopHost(&USB_OTG_Core);
    								
    								/* Re-Initilaize Host for new Enumeration */
    								USBH_DeInit(&USB_OTG_Core, &USB_Host);
    								USB_Host.usr_cb->DeInit();
    								USB_Host.class_cb->DeInit(&USB_OTG_Core, &USB_Host.device_prop);

    							}
    								break;

    						}
    					}
    				}
    					break;
    			}
    		}
    			break;

    		case USB_HD_DEVICE:
    		{
    			switch( mUSB_HD_State.DeviceState )
    			{
    				case USB_DEVICE_IDLE:
    				{	GPIOMiddleLevel_Clr( __O_USB_VIN);
    					USBD_Init(&USB_OTG_Core,
#ifdef USE_USB_OTG_HS 
    						USB_OTG_HS_CORE_ID,
#else            
    						USB_OTG_FS_CORE_ID,
#endif  
    						&USR_desc, 
    						&USBD_CDC_cb,	 
    						&USR_cb);

    					mUSB_HD_State.DeviceState = USB_DEVICE_WAIT;

    					__disable_irq();
    					__enable_irq();
    				}
    				break;

    				case USB_DEVICE_WAIT:
    				{
    					/* Handle select item */
    					if(mUSB_HD_State.select & 0x80)
    					{
    						select_value = (mUSB_HD_State.select & 0x0E ) >> 1;
    						
    						switch ( select_value )
    						{
    							case 3: /*USB_HOST_DEVICE_CMD_VCP_CONNECT*/
    							{
    								/*NO PROCESS*/
    								mUSB_HD_State.mode = USB_DEVICE;
    							}
    								break;

    							case 4: /* USB_HOST_DEVICE_CMD_VCP_DISCONNECT */
    							{
    								__disable_irq();
    								mUSB_HD_State.state = USB_HD_IDLE;
    								__enable_irq();
    								DCD_DevDisconnect (&USB_OTG_Core);
    								USB_OTG_StopDevice(&USB_OTG_Core);  
    							}
    								break;
    						
    							default:
    							{
    								__disable_irq();
    								mUSB_HD_State.state = USB_HD_WAIT;
    								mUSB_HD_State.DeviceState = USB_DEVICE_IDLE;
    								__enable_irq();
    								DCD_DevDisconnect (&USB_OTG_Core);
    								USB_OTG_StopDevice(&USB_OTG_Core);     
    							}
    								break;
    						}
    			        }
      				}
    				break;
    			}
    				
    		}
    			break;

    	}


#if 0    
        USB_HOST_DEVICE_Application();
        
#ifdef USBHostDeviceHandler_StateMonitor     /* for monitor */
        USBHostDeviceHandlerPriority = uxTaskPriorityGet( mUSBHostDeviceHandleParams.TaskHandle );
        USBHostDeviceHandlerStack = uxTaskGetStackHighWaterMark(NULL);
#endif
#endif        
        vTaskDelay( TASK_MSEC2TICKS(10));
    }
}
#else
static void USBHostDeviceHandler_ServiceHandle( void *pvParameters )
{
    static uint8_t prev_select = 0;
    uint8_t select_value = 0;
    //static bool _is_restart_host_play = TRUE;
    xUSBMusicManagerState recState;
    
    xSemaphoreTake(mUSBHostDeviceHandleParams.TaskSema, BLOCK_TIME(0));

    for(;;)
    {
        xSemaphoreTake(mUSBHostDeviceHandleParams.TaskSema, BLOCK_TIME(10));
    	switch( mUSB_HD_State.state )
    	{
    		case USB_HD_IDLE:
    		{
    		}
    			break;

    		case USB_HD_WAIT:
    		{	
                if (mUSB_HD_State.mode == USB_HOST)
                {
                    usb_state = USBH_Process(&USB_OTG_Core, &USB_Host);
                }
                else if (mUSB_HD_State.mode == USB_DEVICE)
                {
                    //_is_restart_host_play = TRUE;
                }
    		}
    			break;
    	}

        //vTaskDelay( TASK_MSEC2TICKS(10));
    }
}

#endif

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



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
