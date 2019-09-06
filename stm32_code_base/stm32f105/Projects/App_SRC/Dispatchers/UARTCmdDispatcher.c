#include "Defs.h"
#include "Debug.h"
#include "device_config.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "api_typedef.h"
#include "UART_LowLevel.h"
#include "FactoryCommandHandler.h"
#include "HMI_Service.h" 
#include "CommandsManager.h"


#include "UARTCmdDispatcher.h"

//__________________________________________________________________________________________
extern FACTORY_CMD_HANDLE_OBJECT *pFMD_ObjCtrl;
extern HMI_SERVICE_OBJECT *pHS_ObjCtrl;
extern CTRL_CMD_HANDLE_OBJECT *pCtrlCmd_ObjCtrl;


//__________________________________________________________________________________________
#define WAIT_TIMEOUT VIRTUAL_TIMER_MSEC2TICKS(50)
#define UART_TIME_TICK TASK_MSEC2TICKS(1)

#define FAC_HEADER 0xC5
#define CTRL_HEADER 0xCA
#define PACKAGE_MAX 136 /*header+ command type + data[128] + checksum*/

//__________________________________________________________________________________________
const static xHMIUserEvents IRNumberTable[] =
{
    USER_EVENT_NULL,    		/* 0 */
    USER_EVENT_NULL,   	/* 1 */
    USER_EVENT_NULL,    	/* 2 */
    USER_EVENT_NULL,    	/* 3 */
    USER_EVENT_NULL,    /* 4 */
    USER_EVENT_NULL,    		/* 5 */
    USER_EVENT_NULL,    		/* 6 */
    USER_EVENT_NULL,    		/* 7 */
    USER_EVENT_NULL,    		/* 8 */
    USER_EVENT_NULL     		/* 9 */
};

const static xHMIUserEvents IRAlphaTable[] =
{
    USER_EVENT_NULL,    		/* a */
    USER_EVENT_NULL,    		/* b */
    USER_EVENT_NULL,    /* c */
    USER_EVENT_NULL,    		/* d */
    USER_EVENT_NULL,           /* e */
    USER_EVENT_NULL,    		/* f */
    USER_EVENT_NULL,    		/* g */
    USER_EVENT_NULL,    		/* h */
    USER_EVENT_NULL,    		/* i */
    USER_EVENT_NULL,    		/* j */
    USER_EVENT_NULL,    		/* k */
    USER_EVENT_NULL,    		/* l */
    USER_EVENT_MUTE,    		/* m */
    USER_EVENT_NULL,    		/* n */
    USER_EVENT_NULL,    		/* o */
    USER_EVENT_NULL,    		/* p */
    USER_EVENT_NULL,    		/* q */
    USER_EVENT_NULL,          /* r */
    USER_EVENT_NULL,  		/* s */
    USER_EVENT_NULL,    		/* t */
    USER_EVENT_NULL,    		/* u */
    USER_EVENT_NULL,    	/* v */
    USER_EVENT_NULL,  		/* w */
    USER_EVENT_NULL,    	/* x */
    USER_EVENT_NULL,    		/* y */
    USER_EVENT_NULL     	/* z */
};

static uint8 mBuffer[PACKAGE_MAX]={0xff};
static uint8 *pPackage = mBuffer;


//___________________________________________________________________________________
xHMIUserEvents UartCmdDispatcher_UserEventHandle(char UartCode)
{
    if (UartCode >= 48 && UartCode <= 57) /*ascii 0~9*/
    {
        return IRNumberTable[UartCode-48];
    }
    else if (UartCode >= 97 && UartCode <= 122) /*ascii a~z*/
    {
        return IRAlphaTable[UartCode-97];
    }
    else
    {
        return USER_EVENT_NULL;
    }
}

void UARTCmdDispatcher_poll( void *data )
{
    xHMISrvEventParams srv_parms = {xHMI_EVENT_UART, USER_EVENT_UNDEFINED };
    byte data_len = 0;
    
    for( ;; )
    {
        data_len = UARTLowLevel_getDataLength( );
                
        if ( data_len > 0 )
        {
            if ( UARTLowLevel_readData(VIRTUAL_PORT, pPackage, data_len ) == data_len )
            {
                if(*( pPackage + 0 ) == FAC_HEADER ) /*it could be factroy command header*/
                {
                    if (*( pPackage + 2 ) == data_len )
                    {
                        pFMD_ObjCtrl->Transfer2Task( pPackage );
                    }
                }
				else if (*( pPackage + 0 ) == CTRL_HEADER )
				{
					pCtrlCmd_ObjCtrl->control_command( *( pPackage + 1 ),  ( pPackage + 2 ) );
				}
                else
                {
#if ( configAPP_UART_CTRL == 1)
                    srv_parms.event = UartCmdDispatcher_UserEventHandle(*pPackage);
                    pHS_ObjCtrl->SendEvent( &srv_parms );
#endif
                }
            }
            
        }
        vTaskDelay( UART_TIME_TICK );
    }
}

void UARTCmdDispatcher_CreateTask( void )
{
    if ( xTaskCreate( UARTCmdDispatcher_poll, 
                      ( portCHAR * ) "UARTCmdDispatcher_poll", 
                      configMINIMAL_STACK_SIZE, NULL, 
                      tskHMI_EVENT_PRIORITY, NULL ) != pdPASS)
    {
        TRACE_ERROR((0, " UARTCmdDispatcher_poll task create error !! "));
    }
}


