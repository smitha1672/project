#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "GPIOMiddleLevel.h"

#include "RFHandler.h"

#include "PowerHandler.h"
#include "UIDeviceManager.h"

#define RF_QUEUE_LENGTH 1

//____________________________________________________________________________________________________________
/* extern variable */
extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
extern UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl; 
//____________________________________________________________________________________________________________
//static api header
static void RFHandler_CreateTask(void);
static bool RFHandler_InstrSender(void * cmds);
static void RFHandler_ServiceHandle( void *pvParameters );
uint8 RFHandler_Paired_Status(void);
//____________________________________________________________________________________________________________
const RF_HANDLE_OBJECT   RFHandle =
{
    RFHandler_CreateTask,
    RFHandler_InstrSender,
};
const RF_HANDLE_OBJECT *pRFHandle_ObjCtrl = &RFHandle;

typedef struct RF_HANDLE_PARAMETERS
{
    xTaskHandle TaskHandle;
    uint8 power_handle;
    TaskHandleState taskState;
    xSemaphoreParameters SParams;
    xQueueParameters QParams;
} xRFHandleParams;

//____________________________________________________________________________________________________________
/*static variable*/
static bool mRF_isPairing = FALSE;
static bool mRF_PairingProcess = FALSE;
static bool mRF_CheckPaired = FALSE;
static bool mRF_isPaired = TRUE;
static xRFHandleParams mRFHandleParams;
static uint16 RFConnectionFail_counter = 0;

//____________________________________________________________________________________________________________
static void RFHandler_CreateTask(void)
{
    if ( xTaskCreate( 
             RFHandler_ServiceHandle, 
             ( portCHAR * ) "RF_Handle", 
             (STACK_SIZE), 
             NULL, 
             tskRF_HANDLER_PRIORITY, 
             &mRFHandleParams.TaskHandle ) != pdPASS )
     {
         TRACE_ERROR((0, "RF_Handle task create failure " ));
     }

    mRFHandleParams.QParams.xQueue = xQueueCreate( RF_QUEUE_LENGTH, FRTOS_SIZE(xRFHandleCommand) );
    mRFHandleParams.QParams.xBlockTime = BLOCK_TIME(0);

    if(mRFHandleParams.QParams.xQueue == NULL)
    {
        TRACE_ERROR((0, "Create RFHandle queue creates failure " ));
    }

    xQueueReset(mRFHandleParams.QParams.xQueue);

    //mRFHandleParams.power_handle = pPowerHandle_ObjCtrl->register_handle();
}


static bool RFHandler_InstrSender(void * cmds) 
{
    xRFHandleCommand* pCmd = ( xRFHandleCommand *)cmds;
    
    if(mRFHandleParams.QParams.xQueue == NULL)
    {
        TRACE_ERROR((0, "RFHandler_InstrSender error !! "));
        return FALSE;
    }
    else if(xQueueSend(mRFHandleParams.QParams.xQueue,pCmd,mRFHandleParams.QParams.xBlockTime) != pdPASS)
    {
        TRACE_ERROR((0, " RFHandler_InstrSender send queue is failure "));
        return FALSE;
    }
    
    return TRUE;
}

static bool RFHandler_InstrReceiver( xRFHandleCommand *pCmd ) 
{
    if(pCmd == NULL)
    {
        TRACE_ERROR((0, " RFHandler_InstrReceiver recevier error !! "));
        return FALSE;
    } 
    else if(mRFHandleParams.QParams.xQueue == NULL)
    {
        TRACE_ERROR((0, "RFHandler_InstrReceiver error !! "));
        return FALSE;
    }    
    else if(xQueueReceive(mRFHandleParams.QParams.xQueue,pCmd,mRFHandleParams.QParams.xBlockTime) != pdPASS)
    {
        TRACE_ERROR((0, " RFHandler_InstrReceiver receiver queue is failure "));
        return FALSE;
    }

    return TRUE;   
}

static uint8 RFHandler_GetQueueNumber( void )
{
    return (uint8)uxQueueMessagesWaiting(mRFHandleParams.QParams.xQueue);
}

uint8 RFHandler_Paired_Status(void)
{
#if 0
    if (mRF_isPaired == TRUE)
    {   
        return 1;
    }
    return 0;
#else
    if ( GPIOMiddleLevel_Read_InBit(__O_PAIRING_LED) )
    {
        return 1;
    }
    else
    {
        //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_NORMAL);
        TRACE_DEBUG((0,"RF Module Pairing Fail connection."));

        return 0;
    }
#endif
}

static void RFHandler_handleCommand(void *cmds)
{
    xRFHandleCommand* RFHandleCmd = (xRFHandleCommand*)cmds;

    switch(*RFHandleCmd)
    {        
        case RF_HANDLE_PAIRING_PHASE1:
        {
            TRACE_DEBUG((0, " RFHandler_handleCommand :  RF_HANDLE_PAIRING_PHASE1 !! "));
            GPIOMiddleLevel_Clr(__O_RF_PAIRING);
            mRF_isPairing = TRUE;
            mRF_isPaired = FALSE;

            mRF_PairingProcess = TRUE;
            mRF_CheckPaired = FALSE;
            RFConnectionFail_counter = 0;
        }
        break;

        case RF_HANDLE_PAIRING_PHASE2:
        {
            TRACE_DEBUG((0, " RFHandler_handleCommand :  RF_HANDLE_PAIRING_PHASE2 !! "));

            GPIOMiddleLevel_Set(__O_RF_PAIRING);
            mRF_PairingProcess = FALSE;
        }
        break;

        case RF_HANDLE_CHECK:
        {
            //TRACE_DEBUG((0, " RFHandler_handleCommand :  RF_HANDLE_CHECK !! "));
            
            if ( GPIOMiddleLevel_Read_InBit(__O_PAIRING_LED) )
            {
                //TRACE_DEBUG((0,"RF Module RX is connection."));
                if (mRF_isPaired == FALSE)
                {
                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_RF_PAIRED);
                    mRF_isPaired = TRUE;
                }
                RFConnectionFail_counter = 0;

                mRF_isPairing = FALSE;
                mRF_CheckPaired = FALSE;
            }
            else
            {
                RFConnectionFail_counter++;
                
                //TRACE_DEBUG((0,"RF Module Pairing waiting."));
                
                if (RFConnectionFail_counter >= 300 )  //60s
                {
                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_NORMAL);
                    RFConnectionFail_counter = 0;

                    mRF_isPairing = FALSE;
                    mRF_CheckPaired = FALSE;
                    TRACE_DEBUG((0,"RF Module Pairing Fail connection."));
                }
            }
        }
        break;
    }
    
    mRFHandleParams.taskState =  TASK_SUSPENDED;                
}


static void RFHandler_ServiceHandle( void *pvParameters )
{
    xRFHandleCommand RFHandleCmd;
    uint32 RFCheckingCounter = 0;
    uint32 RFPairingCounter = 0;
    xRFHandleCommand mRFHandleCommand;
    
    for(;;)
    {
        switch(mRFHandleParams.taskState)
        {
            case TASK_SUSPENDED:
            {
                if( mRF_isPairing == TRUE )
                {
                    if(mRF_CheckPaired == TRUE )
                    {  
                        RFCheckingCounter++;
                        
                        if ( RFCheckingCounter >= 20 )    /*every 220 msec: Suspended 200ms, Ready 10ms and Running 10ms */
                        {
                            mRFHandleCommand = RF_HANDLE_CHECK;
                            pRFHandle_ObjCtrl->rf_instr_sender(&mRFHandleCommand);
                        
                            RFCheckingCounter = 0;
                        }
                    }
                    else
                    {
                        RFPairingCounter++;             
                        if ( RFPairingCounter >= 100 )
                        {
                            mRFHandleCommand = RF_HANDLE_PAIRING_PHASE2;
                            pRFHandle_ObjCtrl->rf_instr_sender(&mRFHandleCommand);

                            RFPairingCounter = 0;
                            RFCheckingCounter = 0;
                            mRF_CheckPaired = TRUE;
                        }
                    }
                }
                 
                if( RFHandler_GetQueueNumber() != 0 )
                {
                    mRFHandleParams.taskState = TASK_READY;
                } 
            }
            break;

            case TASK_READY:
            {
                if( xTaskIsTaskSuspended( mRFHandleParams.TaskHandle ) == pdPASS ) /*task is not working*/
                {
                    mRFHandleParams.taskState =  TASK_SUSPENDED;                
                }
                else
                {
                    if( RFHandler_GetQueueNumber() != 0 )
                    {
                        if ( RFHandler_InstrReceiver( &RFHandleCmd ) == TRUE )
                        {
                            /* Ignore all event if system is NOT in power on stage or RF pairing process does NOT complete. */
                            if(pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)
                            {
                                if(mRF_PairingProcess)
                                {
                                    if(RFHandleCmd == RF_HANDLE_PAIRING_PHASE2)
                                    {
                                        mRFHandleParams.taskState = TASK_RUNING;
                                    }
                                    else
                                    {
                                        mRFHandleParams.taskState = TASK_SUSPENDED;
                                    }             
                                }
                                else
                                {
                                    mRFHandleParams.taskState = TASK_RUNING;
                                }
                            }
                            else
                            {
                                mRFHandleParams.taskState = TASK_SUSPENDED;
                                //TRACE_DEBUG((0, "RFHandler: Back to SUSPENDEDS. Cause system are not be powered up or RF pairing is still processing."));
                            }
                        }
                        else
                        {
                            mRFHandleParams.taskState = TASK_SUSPENDED;
                        }
                    }
                    else
                    {
                        mRFHandleParams.taskState =  TASK_SUSPENDED;      
                    }
                }
            }
            break;

            case TASK_RUNING:
            {
                if( xTaskIsTaskSuspended( mRFHandleParams.TaskHandle ) == pdPASS ) /*task is not working*/
                {
                    mRFHandleParams.taskState =  TASK_SUSPENDED;                
                }
                else
                {
                    RFHandler_handleCommand((xRFHandleCommand*)&RFHandleCmd);
                }
            }
            break;
        }
        vTaskDelay( TASK_MSEC2TICKS(10) );
    }
}

