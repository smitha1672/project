#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"

#include "usb_host_device.h"
#include "IRCmdHandler.h"
#include "UARTCmdDispatcher.h"

#include "FactoryCommandHandler.h"
#include "HMI_Service.h"

//____________________________________________________________________________________________________________
/* extern variable */
extern IR_CMD_HANDLER_OBJECT *pIRCmdHandle_ObjCtrl;
xTaskHandle hTaskWatchdog = NULL;
//_____________________________________________________________________________
extern void CardLowLevelPlatform_initializeCard(void);
extern void ButtonCmdDispatcher_initialize( void );
extern void CPU_resetMicroController( void );

//_____________________________________________________________________________
#define MAX_WATCHDOG_HANDLE	2
static int16 watchdog_cnt = 10;
static xSemaphoreHandle semaTaskWatch[MAX_WATCHDOG_HANDLE] = {NULL};
static void watchdog_task(void *pParm)
{
    uint8 idx;
    while(1)
    {
        for(idx=0;idx<MAX_WATCHDOG_HANDLE;idx++)
        {
            if(semaTaskWatch[idx] != NULL)
                xSemaphoreTake(semaTaskWatch[idx], portMAX_DELAY);
        }
        watchdog_cnt = 10;
        vTaskDelay(1000);
    }
}

void Watchdog_check(void)
{
    if(watchdog_cnt-- <= 0)
    {
        TRACE_DEBUG((0, "reset"));
        CPU_resetMicroController();
    }
}


void Watchdog_Go(void)
{
    uint8 idx;
    for(idx=0;idx<MAX_WATCHDOG_HANDLE;idx++)
    {
        semaTaskWatch[idx] = NULL;
    }
    xTaskCreate(watchdog_task, "watchdog", STACK_SIZE, NULL, tskWatchdog_PRIORITY, &hTaskWatchdog);
}

int8 Watchdog_register(void)
{
    uint8 idx;
    for(idx=0;idx<MAX_WATCHDOG_HANDLE;idx++)
    {
        if(semaTaskWatch[idx] == NULL)
        {
            semaTaskWatch[idx] = xSemaphoreCreateBinary();

            return idx;
        }
    }
    
    return -1;
}

void Watchdog_taskAlive(int8 idx)
{
    if(idx < MAX_WATCHDOG_HANDLE && semaTaskWatch[idx]!= NULL)
    {
        xSemaphoreGive(semaTaskWatch[idx]);
    }
}

/*enable MCU*/
static void prvSetupHardware(void)
{
    CardLowLevelPlatform_initializeCard();
}

static void prvSetupSystem( void )
{
    ButtonCmdDispatcher_initialize(  ); /*keypad register*/
    pIRCmdHandle_ObjCtrl->CreateTask(); /* IR */ 
    UARTCmdDispatcher_CreateTask( );

    HMI_Service_CreateTask( );
    FactoryCmdHandler_CreateTask( );

    Debug_initialize();

    Watchdog_Go();
}


/*_______________________________________________________________________________________________________*/
int main(void)
{
    prvSetupHardware( ); /*initial CPU internal funcation*/
    prvSetupSystem( );

	/* Start the scheduler. */
	vTaskStartScheduler();

   return 0;
}

