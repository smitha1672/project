#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"

#if ( configSTM32F411_PORTING == 1)
#include "usb_host_device.h"
#include "IRCmdHandler.h"
#include "UARTCmdDispatcher.h"
#include "FactoryCommandHandler.h"
#include "HMI_Service.h"
#endif

#if ( configSTM32F411_PORTING == 0)
#include "GPIOMiddleLevel.h" 
#include "usb_host_device.h"
#include "IRCmdHandler.h"
#include "UARTCmdDispatcher.h"
#include "HMI_Service.h"
xTaskHandle systemMonitorTaskHandle;

extern USB_HOST_DEVICE_HANDLE_OBJECT *pUSB_HD_ObjCtrl;

#endif 

//_____________________________________________________________________________
/* extern variable */
extern IR_CMD_HANDLER_OBJECT *pIRCmdHandle_ObjCtrl;
//_____________________________________________________________________________
extern void CardLowLevelPlatform_initializeCard(void);

extern void ButtonCmdDispatcher_initialize( void );

//_____________________________________________________________________________

/*enable MCU*/
static void prvSetupHardware(void)
{
    CardLowLevelPlatform_initializeCard();
}

static void prvSetupSystem( void )
{
#if ( configSTM32F411_PORTING == 1 )
    ButtonCmdDispatcher_initialize(  ); /*keypad register*/
    pIRCmdHandle_ObjCtrl->CreateTask(); /* IR */ 
    UARTCmdDispatcher_CreateTask( );
    HMI_Service_CreateTask( );
    FactoryCmdHandler_CreateTask( );
    Debug_initialize();
#else
    ButtonCmdDispatcher_initialize(  ); /*keypad register*/
    HMI_Service_CreateTask( );
    UARTCmdDispatcher_CreateTask( );
    pUSB_HD_ObjCtrl->CreateTask( );
    pIRCmdHandle_ObjCtrl->CreateTask(); /* IR */
    Debug_initialize();
#endif
}

#if ( configSTM32F411_PORTING == 0 )
void SystemCheck_Task( void *pvParameters )
{
	for( ;; )
	{
        TRACE_DEBUG((0, "STM32F411 - application is working ")); 
		vTaskDelay(1000);
	}
}
#endif

#if ( configSTM32F411_PORTING == 0 )
void SystemCheck_CreateTask( void )
{
    if ( xTaskCreate( 
            SystemCheck_Task, 
            ( portCHAR * ) "SYS_MON", 
            (STACK_SIZE), 
            NULL, 
            tskIDLE_PRIORITY, 
            &systemMonitorTaskHandle ) != pdPASS )
        {
            vTaskDelete( systemMonitorTaskHandle );
        }
}
#endif

/*_______________________________________________________________________________________________________*/
    int main(void)
{
    prvSetupHardware( ); /*initial CPU internal funcation*/
    prvSetupSystem( );

#if ( configSTM32F411_PORTING == 0 )
    SystemCheck_CreateTask();
#endif    

	/* Start the scheduler. */
	vTaskStartScheduler();
  
   return 0;
}

