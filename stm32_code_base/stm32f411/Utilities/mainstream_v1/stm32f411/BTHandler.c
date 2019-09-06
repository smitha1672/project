#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "GPIOMiddleLevel.h"
#include "GPIOLowLevel.h"

#include "api_typedef.h"
#include "BTHandler.h"
#include "PowerHandler.h"
#include "UIDeviceManager.h"
#include "HMI_Service.h"


#define BT_QUEUE_LENGTH 3
#define BT_AVRCP_CTRL_TIME  TASK_MSEC2TICKS(80)

#define BT_AUTO_PAIRING_TIME TASK_MSEC2TICKS(50000) //50 SEC
#define BT_ENERGY_SAVING_TIME TASK_MSEC2TICKS(900000) //15 MIN

#define BT_CLEAN_DEVICE_TIME TASK_MSEC2TICKS(28000) //28 SEC

#define BT_START_UP_TIMER 10

#define BT_POWER_TOOGLE_TIME 85

//____________________________________________________________________________________________________________
/* extern variable */
extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
extern UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl;
extern HMI_SERVICE_OBJECT *pHS_ObjCtrl; /*HMI service*/

//____________________________________________________________________________________________________________
//static api header
static void BTHandler_Init(void);
static void BTHandler_CreateTask(void);
static bool BTHandler_InstrSender(void * cmds);
static uint8 BTHandler_GetQueueNumber( void );
static void BTHandler_ServiceHandle( void );
static void BTHandler_PowerCtl(void);
//static xBTHandleModuleStatus BTHandler_GetBTStatus(void);
static void BTHandler_GetBTStatus( xBTHandleModuleStatus *update_status );

static xBTHandleModuleStatus BTHanlder_reportBTStatus( void );

static bool BTHandler_ToogleBTPower(bool OnOff);

#if 0
static void BTHandler_handleCommand(void *cmds);
#endif
static void BTHandler_handleCommand( void *params  );

static void BTHandler_Service( void *pvParameters );

static bool BT_GPIO_sema_mutex_take( void );

static bool BT_GPIO_sema_mutex_give( void );

static void BT_GPIO_BT_PAIRING_CTTL(void);

static void BT_GPIO_BT_PLAY_CTTL(void);

static void BT_GPIO_BT_PLAY_NEXT(void);

static void BT_GPIO_BT_PLAY_PRE(void);

static void BT_GPIO_BT_CLEAN_DEVICE(void);

static void BTHandler_TaskCtrl( bool val );

//____________________________________________________________________________________________________________
const BT_HANDLE_OBJECT   BTHandle =
{
    BTHandler_CreateTask,
    BTHandler_handleCommand,
    BTHandler_InstrSender,
    BTHandler_PowerCtl,
    BTHanlder_reportBTStatus
};
const BT_HANDLE_OBJECT *pBTHandle_ObjCtrl = &BTHandle;

typedef struct BT_HANDLE_PARAMETERS
{
    xTaskHandle TaskHandle;
    uint8 power_handle;
    TaskHandleState taskState;
    xSemaphoreParameters SParams;
    xQueueParameters QParams;
} xBTHandleParams;

//____________________________________________________________________________________________________________
/*static variable*/
/*static xBTHandleParams mBTHandleParams;
static xBTHandleState BTHandlerState = BT_MS_IDLE;
static uint16 mBTHandlerStamp = 0;
static xBTHandleCommand mBTHandleComd;
static bool NeedPairing = FALSE;
static xBTHandleModuleStatus BTStatus = BT_STATUS_ERROR;
static xBTHandleModuleStatus LastBTStatus = BT_STATUS_ERROR;
static uint8 CleanDeviceCmdSource = 3;
static uint16 mBTHandlerPowerOnStamp = 0;
*/
xBTHandleParams mBTHandleParams;
xBTHandleState BTHandlerState = BT_MS_IDLE;
uint16 mBTHandlerStamp = 0;
xBTHandleCommand mBTHandleComd;
bool NeedPairing = FALSE;
xBTHandleModuleStatus BTStatus = BT_STATUS_ERROR;
xBTHandleModuleStatus LastBTStatus = BT_STATUS_ERROR;
uint8 CleanDeviceCmdSource = 3;
uint16 mBTHandlerPowerOnStamp = 0;

static bool Is_Start_Counter = FALSE;
static portTickType duration_time = 0;
static portTickType start_time = 0;
xBTTimeStatus BT_time_status = BT_TIME_INIT;

static int retry_timer = 0;
//for mutex mechanism when read/write gpio
xSemaphoreParameters BT_GPIO_sema;


/*******************************************************************************
 * Mutex
 ******************************************************************************/
static bool BT_GPIO_sema_mutex_noWait_take( void )
{
#if 1
    if ( BT_GPIO_sema.xSemaphore == NULL )
    {
        return FALSE;
    }

    //BT_GPIO_sema.xBlockTime = portMAX_DELAY;
    if ( xSemaphoreTake( BT_GPIO_sema.xSemaphore, 0 ) != pdTRUE )
    {
        return FALSE;
    }
 #endif   
    return TRUE;
}

static bool BT_GPIO_sema_mutex_take( void )
{
    if ( BT_GPIO_sema.xSemaphore == NULL )
    {
        return FALSE;
    }

    //BT_GPIO_sema.xBlockTime = portMAX_DELAY;
    if ( xSemaphoreTake( BT_GPIO_sema.xSemaphore, BT_GPIO_sema.xBlockTime ) != pdTRUE )
    {
        return FALSE;
    }
    
    return TRUE;
}

static bool BT_GPIO_sema_mutex_give( void )
{
    if ( BT_GPIO_sema.xSemaphore == NULL )
        return FALSE;

    if ( xSemaphoreGive( BT_GPIO_sema.xSemaphore ) != pdTRUE )
    {
        return FALSE;    
    }
    return TRUE;
}


//____________________________________________________________________________________________________________

static void BTHandler_Init(void)
{
    BTHandlerState = BT_MS_IDLE; /* Go to Init state when SB power on */

    GPIOMiddleLevel_Clr(__O_BT_REW);
    GPIOMiddleLevel_Clr(__O_BT_FWD);
    GPIOMiddleLevel_Clr(__O_BT_REST);
    GPIOMiddleLevel_Clr(__O_BT_MFB);
}

static void BTHandler_CreateTask(void)
{
    BTHandler_Init();
    
    if ( xTaskCreate( 
         BTHandler_Service,
             ( portCHAR * ) "BT_Handle", 
             (STACK_SIZE), 
             NULL, 
             tskBT_HANDLER_PRIORITY, 
             &mBTHandleParams.TaskHandle ) != pdPASS )
     {
         TRACE_ERROR((0, "BT_Handle task create failure " ));
     }
    else
     {
        vTaskSuspend( mBTHandleParams.TaskHandle );
     }

    mBTHandleParams.QParams.xQueue = xQueueCreate( BT_QUEUE_LENGTH, FRTOS_SIZE(xBTHandleCommand) );
    mBTHandleParams.QParams.xBlockTime = BLOCK_TIME(0);

    if(mBTHandleParams.QParams.xQueue == NULL)
    {
        TRACE_ERROR((0, "Create BTHandle queue creates failure " ));
    }

    xQueueReset(mBTHandleParams.QParams.xQueue);

    mBTHandleParams.power_handle = pPowerHandle_ObjCtrl->register_handle();

    BT_GPIO_sema.xSemaphore = xSemaphoreCreateMutex();
    BT_GPIO_sema.xBlockTime = portMAX_DELAY;

    //TRACE_ERROR((0, "BT start up .... " ));
    //BTHandler_ToogleBTPower( FALSE );
}


static bool BTHandler_InstrSender(void * cmds) 
{
#if 0 // smith removes
    xBTHandleCommand* pCmd = ( xBTHandleCommand *)cmds;
    
    if(mBTHandleParams.QParams.xQueue == NULL)
    {
        TRACE_ERROR((0, "BTHandler_InstrSender error !! "));
        return FALSE;
    }
    else if(xQueueSend(mBTHandleParams.QParams.xQueue,pCmd,mBTHandleParams.QParams.xBlockTime) != pdPASS)
    {
        TRACE_ERROR((0, " BTHandler_InstrSender send queue is failure "));
        return FALSE;
    }
#endif    
    return TRUE;
}

static bool BTHandler_InstrReceiver( xBTHandleCommand *pCmd ) 
{
    if(pCmd == NULL)
    {
        TRACE_ERROR((0, " BTHandler_InstrReceiver recevier error !! "));
        return FALSE;
    } 
    else if(mBTHandleParams.QParams.xQueue == NULL)
    {
        TRACE_ERROR((0, "BTHandler_InstrReceiver error !! "));
        return FALSE;
    }    
    else if(xQueueReceive(mBTHandleParams.QParams.xQueue,pCmd,mBTHandleParams.QParams.xBlockTime) != pdPASS)
    {
        TRACE_ERROR((0, " BTHandler_InstrReceiver receiver queue is failure "));
        return FALSE;
    }

    return TRUE;   
}


static void BTHandler_Service( void *pvParameters )
{
    xBTHandleCommand EventParams;

    for ( ;; )
    {
        BTHandler_GetBTStatus(&BTStatus);
    switch( mBTHandleParams.taskState )
    {
        case TASK_SUSPENDED:
        {
        }
        break;

        case TASK_READY:
        {
            if ( BTHandler_GetQueueNumber() != 0 )
            {
                mBTHandleParams.taskState = TASK_RUNING;
            }
            else
            {
                //if ( BTHandlerState == BT_MS_IDLE )
                    BTHandler_ServiceHandle();
            }
        }
        break;

        case TASK_RUNING:
        {
            if ( BTHandlerState == BT_MS_IDLE )
            {
                if (BTHandler_InstrReceiver( &EventParams ) == TRUE )
                {
                    //BTHandler_handleCommand( &EventParams );
                }
                else
                    mBTHandleParams.taskState = TASK_READY;
            }
            
            BTHandler_ServiceHandle();
        }
        break;    
    }
    
        vTaskDelay( TASK_MSEC2TICKS(100) );
    }
}

static void BTHandler_PowerCtl(void)
{
    //xBTHandleCommand mBTHandleCommand;

    if(pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF)
    {
        mBTHandleParams.taskState = TASK_READY;

        mBTHandleComd = BT_HANDLE_INIT;
        pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleComd);
    }
    else
    {
        mBTHandleComd = BT_HANDLE_DINIT;
        pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleComd);
    }
}

static bool BTHandler_ToogleBTPower(bool OnOff)
{
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {
        if ( OnOff == TRUE )
        {
            //if ( GPIOMiddleLevel_Read_OutBit(__O_BT_ON) == 0 )
            //TRACE_DEBUG((0, "BTStatus VREG - 1 %d",BTStatus)); 
            if (BTStatus == BT_POWER_OFF || BTStatus == BT_STATUS_ERROR)
            {
                GPIOMiddleLevel_Clr(__O_BT_ON);
                vTaskDelay( BT_POWER_TOOGLE_TIME );
                GPIOMiddleLevel_Set(__O_BT_ON);
                vTaskDelay( BT_POWER_TOOGLE_TIME );
                GPIOMiddleLevel_Clr(__O_BT_ON);
                vTaskDelay( BT_POWER_TOOGLE_TIME );
                TRACE_DEBUG((0, "BT VREG - 1 ")); 
            }    
            
        }
        else if ( OnOff == FALSE )
        {
            //if ( GPIOMiddleLevel_Read_OutBit(__O_BT_ON) == 1 )        
            if ( BTStatus  != BT_POWER_OFF ) 
            {
                GPIOMiddleLevel_Clr(__O_BT_ON);
                vTaskDelay( BT_POWER_TOOGLE_TIME );
                GPIOMiddleLevel_Set(__O_BT_ON);
                vTaskDelay( BT_POWER_TOOGLE_TIME );
                GPIOMiddleLevel_Clr(__O_BT_ON);
                vTaskDelay( BT_POWER_TOOGLE_TIME );
                TRACE_DEBUG((0, "BT VREG - 0 "));    
                
            }
            //return TRUE;
        }
        BT_GPIO_sema_mutex_give();
        return TRUE;
    }

    return FALSE;

}

static uint8 BTHandler_GetQueueNumber( void )
{
    return (uint8)uxQueueMessagesWaiting(mBTHandleParams.QParams.xQueue);
}

static xBTHandleModuleStatus BTHanlder_reportBTStatus( void )
{
    return BTStatus;
}

static void BTHandler_GetBTStatus( xBTHandleModuleStatus *update_status )
{   
#if 0
    uint8 TempStatus = 5;
    
    if ( GPIOMiddleLevel_Read_InBit(__I_BT_DET2) ) //LED 0
        TempStatus = 1;
    else
        TempStatus = 0;

    if ( GPIOMiddleLevel_Read_InBit(__I_BT_DET1) ) //LED 1
        TempStatus = TempStatus+2;
    else
        TempStatus = TempStatus+0;

    return (xBTHandleModuleStatus)TempStatus;
#else
    uint8 TempStatus0 = 0;
    uint8 TempStatus1 = 0;
    xHMISystemParams tmpSystemParams;
    //xBTHandleModuleStatus ret_val = BT_STATUS_ERROR;
    
    if (BT_GPIO_sema_mutex_noWait_take() == TRUE)
    {
        TempStatus0 = GPIOMiddleLevel_Read_InBit(__I_BT_DET2);
        TempStatus1 = GPIOMiddleLevel_Read_InBit(__I_BT_DET1);

        if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
        {
            *update_status = BT_POWER_OFF;
            tmpSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
            //pUDM_ObjCtrl->SendEvent( &tmpSystemParams );
            
            //TRACE_DEBUG((0, "update_status power off---- "));
        }
        else if (TempStatus0 == 0 && TempStatus1 == 0)
        {
            //connectable
            *update_status = BT_POWER_ON;
            //TRACE_DEBUG((0, "update_status connectable ---- "));
        }
        else if (TempStatus0 == 1 && TempStatus1 == 1)
        {
            //standby mode
            *update_status = BT_POWER_OFF;
            //TRACE_DEBUG((0, "update_status standby ---- "));
        }
        else if (TempStatus0 == 0 && TempStatus1 == 1)
        {
            //linked
            *update_status = BT_LINKED;
            //TRACE_DEBUG((0, "update_status linked ---- "));
        }
        else if (TempStatus0 == 1 && TempStatus1 == 0)
        {
            //connectable or discoverable
            *update_status = BT_PAIRING;
            //TRACE_DEBUG((0, "update_status discoverable ---- "));
        }
        else
        {
            *update_status = BT_STATUS_ERROR;
            //TRACE_DEBUG((0, "update_status error ---- "));
        }

        BT_GPIO_sema_mutex_give();
    }
    //return ret_val;
#endif
}

static void BT_GPIO_BT_PAIRING_CTTL(void)
{
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {
        GPIOMiddleLevel_Clr(__O_BT_PAIRING);
        vTaskDelay( TASK_MSEC2TICKS(100) );
        GPIOMiddleLevel_Set(__O_BT_PAIRING);
        vTaskDelay( TASK_MSEC2TICKS(100) );
        GPIOMiddleLevel_Clr(__O_BT_PAIRING);
        BT_GPIO_sema_mutex_give();
    }
}

static void BT_GPIO_BT_PLAY_CTTL(void)
{
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {
        GPIOMiddleLevel_Set(__O_BT_MFB);
        vTaskDelay( BT_AVRCP_CTRL_TIME );
        GPIOMiddleLevel_Clr(__O_BT_MFB);
        BT_GPIO_sema_mutex_give();
    }
}

static void BT_GPIO_BT_PLAY_NEXT(void)
{
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {
        GPIOMiddleLevel_Set(__O_BT_FWD);
        vTaskDelay( BT_AVRCP_CTRL_TIME );
        GPIOMiddleLevel_Clr(__O_BT_FWD);
        BT_GPIO_sema_mutex_give();
    }
}

static void BT_GPIO_BT_PLAY_PRE(void)
{
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {
        GPIOMiddleLevel_Set(__O_BT_REW);
        vTaskDelay( BT_AVRCP_CTRL_TIME );
        GPIOMiddleLevel_Clr(__O_BT_REW);
        BT_GPIO_sema_mutex_give();
    }
}

static void BT_GPIO_BT_CLEAN_DEVICE(void)
{
    portTickType curr_time = 0;
    //portTickType total_clean_time = 0;
    
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {
        GPIOMiddleLevel_Set(__O_BT_PAIRING);
        GPIOMiddleLevel_Set(__O_BT_REW);
        GPIOMiddleLevel_Set(__O_BT_FWD);
        //vTaskDelay( TASK_MSEC2TICKS(5000) );
        vTaskDelay( TASK_MSEC2TICKS(3000) );
#if 0        
        curr_time = xTaskGetTickCount();
        while (( ((xTaskGetTickCount() - curr_time)/portTICK_RATE_MS) <= BT_CLEAN_DEVICE_TIME))
        {
            curr_time = xTaskGetTickCount();
            vTaskDelay( TASK_MSEC2TICKS(100) );
        }
#endif        
        GPIOMiddleLevel_Clr(__O_BT_REW);
        GPIOMiddleLevel_Clr(__O_BT_FWD);
        vTaskDelay( TASK_MSEC2TICKS(100) );

#if 0        
        GPIOMiddleLevel_Clr(__O_BT_PAIRING);
        vTaskDelay( TASK_MSEC2TICKS(100) );
        GPIOMiddleLevel_Set(__O_BT_PAIRING);
        vTaskDelay( TASK_MSEC2TICKS(100) );
        GPIOMiddleLevel_Clr(__O_BT_PAIRING);
#endif        
        //vTaskDelay( TASK_MSEC2TICKS(5000) );
        vTaskDelay( TASK_MSEC2TICKS(100) );
        BT_GPIO_sema_mutex_give();
    }
}

static void BTHandler_handleCommand( void *params  )
{
    xHMISystemParams* pSystemParms = (xHMISystemParams*)params;

    if ( params == NULL )
        return;

    switch( pSystemParms->sys_event )
    {
        case SYS_EVENT_SYSTEM_UP:
        {
            mBTHandleParams.taskState = TASK_READY;
            Is_Start_Counter = FALSE;
            BT_time_status = BT_TIME_INIT;
            
            if ( pSystemParms->input_src == AUDIO_SOURCE_BLUETOOTH )
            {
                //TRACE_DEBUG((0, "111 BT SYS_EVENT_SYSTEM_UP - enable %d",BTHandler_GetBTStatus()));
                //TRACE_ERROR((0, "BT start up ..1111.. " ));    
                BTHandler_ToogleBTPower( TRUE );

                while (BTStatus == BT_POWER_OFF || BTStatus == BT_STATUS_ERROR)
                {
                    BTHandler_GetBTStatus(&BTStatus);
                    //TRACE_ERROR((0, "BT start status ..5555.. %d", BTStatus));
                    if (retry_timer >= BT_START_UP_TIMER)
                    {
                        BTHandler_ToogleBTPower( TRUE );
                        retry_timer = 0;
                    }
                    else
                    {
                        retry_timer++;
                    }
                }
                
                BTHandlerState = BT_MS_CHECKING_PAIRING_STATUS;
                //TRACE_DEBUG((0, "BT SYS_EVENT_SYSTEM_UP - enable %d",BTHandler_GetBTStatus()));
                TRACE_DEBUG((0, "BT SYS_EVENT_SYSTEM_UP - Enable "));

                BTHandler_TaskCtrl(BTASK_RESUME);
            }
            else
            {
                //TRACE_ERROR((0, "BT start down 2222 .... " ));
                BTHandler_ToogleBTPower( FALSE );
                BTHandlerState = BT_MS_IDLE;
                BTHandler_TaskCtrl(BTASK_SUSPENDED);
                TRACE_DEBUG((0, "BT SYS_EVENT_SYSTEM_UP - Disable "));
            }

            pPowerHandle_ObjCtrl->turn_on_done( mBTHandleParams.power_handle );
        }
            break;

        case SYS_EVENT_SYSTEM_DOWN:
        {
            //TRACE_ERROR((0, "BT start down 3333 .... " ));
            BTHandler_ToogleBTPower( FALSE );
            BTHandler_TaskCtrl(BTASK_SUSPENDED);
            Is_Start_Counter = FALSE;
            BT_time_status = BT_TIME_INIT;
            pPowerHandle_ObjCtrl->turn_off_done( mBTHandleParams.power_handle );
            TRACE_DEBUG((0, " BT SYS_EVENT_SYSTEM_DOWN "));
        }
            break;

        case SYS_EVENT_SEL_SRC:
        {
            if ( pSystemParms->input_src == AUDIO_SOURCE_BLUETOOTH )
            {
                //TRACE_ERROR((0, "BT start up 4444.... " ));
                BTHandler_ToogleBTPower( TRUE );
                //BTHandlerState = BT_MS_CHECKING_PAIRING_STATUS;
                BTHandler_TaskCtrl(BTASK_RESUME);
            }
            else
            {
                //TRACE_ERROR((0, "BT start down 5555.... " ));
                BTHandler_ToogleBTPower( FALSE );
                BTHandlerState = BT_MS_IDLE;
                Is_Start_Counter = FALSE;
                BT_time_status = BT_TIME_INIT;
                BTHandler_TaskCtrl(BTASK_SUSPENDED);
                
                TRACE_DEBUG((0, "BT SYS_EVENT_SEL_SRC - Disable "));
            }
        }
            break;

        case SYS_EVENT_BT_PAIRING:
        {
            if ( pSystemParms->input_src == AUDIO_SOURCE_BLUETOOTH )
            {
#if 0
                GPIOMiddleLevel_Clr(__O_BT_PAIRING);
                vTaskDelay( TASK_MSEC2TICKS(100) );
                GPIOMiddleLevel_Set(__O_BT_PAIRING);
                vTaskDelay( TASK_MSEC2TICKS(100) );
#else
                BT_GPIO_BT_PAIRING_CTTL();
#endif
                BTHandler_TaskCtrl(BTASK_RESUME);
                BTHandlerState = BT_MS_CHECKING_PAIRING_STATUS;
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
                //TRACE_DEBUG((0, "BT is pairing..."));
            }
        }
            break;

        case SYS_EVENT_BT_AVRCP_PLAY_PAUSE:
        {
            if( ( BTStatus == BT_LINKED ) && ( pSystemParms->input_src == AUDIO_SOURCE_BLUETOOTH ) )
            {
#if 0
                GPIOMiddleLevel_Set(__O_BT_MFB);
                vTaskDelay( BT_AVRCP_CTRL_TIME );
                GPIOMiddleLevel_Clr(__O_BT_MFB);
#else
                BT_GPIO_BT_PLAY_CTTL();
#endif
                TRACE_DEBUG((0, "BT AVRCP PLAY or PAUSE "));
            }

        }
            break;

        case SYS_EVENT_BT_AVRCP_NEXT:
        {
            if( ( BTStatus == BT_LINKED ) && ( pSystemParms->input_src == AUDIO_SOURCE_BLUETOOTH ) )
            {
#if 0
                GPIOMiddleLevel_Set(__O_BT_FWD);
                vTaskDelay( BT_AVRCP_CTRL_TIME );
                GPIOMiddleLevel_Clr(__O_BT_FWD);
#else
                BT_GPIO_BT_PLAY_NEXT();
#endif
                TRACE_DEBUG((0, "BT AVRCP NEXT"));
            }
        }
            break;

        case SYS_EVENT_BT_AVRCP_PREVIOUS:
        {
            if( ( BTStatus == BT_LINKED ) && ( pSystemParms->input_src == AUDIO_SOURCE_BLUETOOTH ) )
            {
#if 0
                GPIOMiddleLevel_Set(__O_BT_REW);
                vTaskDelay( BT_AVRCP_CTRL_TIME );
                GPIOMiddleLevel_Clr(__O_BT_REW);
#else
                BT_GPIO_BT_PLAY_PRE();
#endif
                TRACE_DEBUG((0, "BT AVRCP PREVIOUS"));
            }
        }
            break;

        case SYS_EVENT_SYSTEM_RESET:
        case SYS_EVENT_BT_CLEAN_DEVICE:
        {
            BTHandlerState = BT_MS_CLEAN_DEVICE;
            
            BTHandler_ToogleBTPower( TRUE );
            
            BTHandler_TaskCtrl(BTASK_RESUME);
            
            //TRACE_DEBUG((0, "BT clean device start  %d",xTaskGetTickCount()));
            
           //TRACE_DEBUG((0, "BT clean device end  %d",xTaskGetTickCount()));
#if 1            
            while (BTHandlerState != BT_MS_IDLE)
            {
                TRACE_DEBUG((0, "BT clean device ..."));
            }
#endif
            BTHandler_ToogleBTPower( FALSE );
            BTHandler_TaskCtrl(BTASK_SUSPENDED);
        }
            break;

#if 0
            case SYS_EVENT_BT_AVRCP_PREVIOUS:
                {
                    static bool invert = FALSE;

                    invert = ~invert;

                    if ( invert )
                        {
                    GPIOMiddleLevel_Set(__O_BT_REW);
                    GPIOMiddleLevel_Set(__O_BT_FWD);
                        }
                    else
                        {
                    GPIOMiddleLevel_Clr(__O_BT_REW);
                    GPIOMiddleLevel_Clr(__O_BT_FWD);
                        }
                }
                break;
#endif
        default:
            break;

    }        
    
}

#if 0
static void BTHandler_handleCommand(void *cmds)
{
    xBTHandleCommand* BTHandleCmd = (xBTHandleCommand*)cmds;

    switch(*BTHandleCmd)
    {

        case BT_NOTIFY_POW_SEQ_DONE:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_NOTIFY_POW_SEQ_DONE !! "));
            
            if(pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)
            {
                pPowerHandle_ObjCtrl->turn_on_done( mBTHandleParams.power_handle );
            }
            else// if(pPowerHandle_ObjCtrl->get_power_state() == POWER_DOWN)
            {
                pPowerHandle_ObjCtrl->turn_off_done( mBTHandleParams.power_handle );
                mBTHandleParams.taskState = TASK_SUSPENDED;
            }
        }
        break;
        
        case BT_HANDLE_ON:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_ON !! "));
            BTHandlerState = BT_MS_POWER_ON;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_OFF:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_OFF !! "));
            BTHandlerState = BT_MS_POWER_OFF;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_INIT:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_INIT !! "));
            BTHandlerState = BT_MS_INIT;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_DINIT:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_DINIT !! "));
            BTHandlerState = BT_MS_DEINIT;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_NEXT:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_NEXT !! "));    
            BTHandlerState = BT_MS_NEXT;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_PREVIOUS:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_PREVIOUS !! "));
            BTHandlerState = BT_MS_PREVIOUS;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_PLAY_PAUSE:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_PLAY_PAUSE !! "));
            BTHandlerState = BT_MS_PLAY_PAUSE;
            mBTHandlerStamp = 0;
        }
        break;
        
        case BT_HANDLE_PAIRING:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_PAIRING !! "));
            BTHandlerState = BT_MS_PAIRING;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_CLEAN_DEVICE:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_CLEAN_DEVICE !! "));
            BTHandlerState = BT_MS_CLEAN_DEVICE;
            mBTHandlerStamp = 0;
        }
        break;

        case BT_HANDLE_RESET_BT:
        {
            TRACE_INFO((0, " BTHandler_handleCommand :  BT_HANDLE_RESET_BT !! "));
            BTHandlerState = BT_MS_RESET_BT;
            mBTHandlerStamp = 0;
        }
        break;

    default:
        break;    
    }
}
#endif

static void BTHandler_ServiceHandle( void )
{
    xHMISrvEventParams srv_parms = {xHMI_EVENT_BT, USER_EVENT_UNDEFINED };

    switch(BTHandlerState)
    {
        case BT_MS_IDLE:
        {
            //TRACE_ERROR((0, "BT_MS_IDLE.... %d", BTStatus));
            if ((BTStatus == BT_POWER_ON) ||
                (BTStatus == BT_PAIRING))
            {
                BTHandlerState = BT_MS_CHECKING_PAIRING_STATUS;
            }
        }
            break;

        case BT_MS_CLEAN_DEVICE:
        {
            //TRACE_ERROR((0, "BT start up 6666.... " ));
#if 1
            BTHandler_ToogleBTPower( TRUE );
            vTaskDelay( TASK_MSEC2TICKS(100) );

#if 0
            GPIOMiddleLevel_Set(__O_BT_REW);
            GPIOMiddleLevel_Set(__O_BT_FWD);
            //vTaskDelay( TASK_MSEC2TICKS(5000) );
            vTaskDelay( TASK_MSEC2TICKS(2900) );
            GPIOMiddleLevel_Clr(__O_BT_REW);
            GPIOMiddleLevel_Clr(__O_BT_FWD);
#else
            BT_GPIO_BT_CLEAN_DEVICE();
#endif
            //TRACE_ERROR((0, "BT start down 7777 .... " ));
            BTHandler_ToogleBTPower( FALSE );
            pPowerHandle_ObjCtrl->turn_off_done( mBTHandleParams.power_handle );
            TRACE_DEBUG((0, "BT clean device finished"));
#endif        
            BTHandlerState = BT_MS_IDLE;
        }
            break;

        case BT_MS_CHECKING_PAIRING_STATUS:
        {
            //TRACE_DEBUG((0, " BT_MS_CHECKING_PAIRING_STATUS BT status ... %d ",BTStatus));
            if ( BTStatus == BT_LINKED )
            {
                /*LED indicates bt pairing success*/
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRED);

                TRACE_DEBUG((0, "BT pairing is done "));
                BTHandlerState = BT_MS_IDLE;
                Is_Start_Counter = FALSE;
                BT_time_status = BT_TIME_INIT;
                start_time = 0;
            }
            else if ( BTStatus == BT_PAIRING  || BTStatus == BT_POWER_ON)
            {
                /*LED indicates bt pairing success*/
                if (Is_Start_Counter == FALSE)
                {
                    //if (BTStatus == BT_PAIRING)
                    {
                        pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
                        TRACE_DEBUG((0, " show pairing led light ...  "));
                    }
                    start_time = xTaskGetTickCount( );
                    Is_Start_Counter = TRUE;
                    
                }

                //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
                
                if (( ((xTaskGetTickCount() - start_time)/portTICK_RATE_MS) > BT_AUTO_PAIRING_TIME) && (BT_time_status == BT_TIME_INIT))
                {
                    //Trigger auto pairing mechanism
                    if (BTStatus != BT_PAIRING)
                    {
                        BT_GPIO_BT_PAIRING_CTTL();
                    }
                    start_time = xTaskGetTickCount();
                    BT_time_status = BT_TIME_AUTO_PAIRING;
                    TRACE_DEBUG((0, "BT auto pairing start..."));
                }
                else if ( (((xTaskGetTickCount() - start_time)/portTICK_RATE_MS) > BT_ENERGY_SAVING_TIME) && (BT_time_status == BT_TIME_AUTO_PAIRING))
                {
                    //Auto power off
                    //TRACE_ERROR((0, "BT start down 8888 .... " ));
                    start_time = 0;
                    Is_Start_Counter = FALSE;
                    BTHandlerState = BT_MS_IDLE;
                    BT_time_status = BT_TIME_INIT;

                    
                    BTHandler_ToogleBTPower(FALSE);
                    //send event to hmi
                    
                    srv_parms.event = USER_EVENT_POWER;
                    pHS_ObjCtrl->SendEvent( &srv_parms );
                }
                
                //TRACE_DEBUG((0, "Display BT pairing ... "));
                //BTHandlerState = BT_MS_IDLE;
            }
            else if ( BTStatus == BT_POWER_OFF )
            {
                //TRACE_DEBUG((0, "Display BT Power off ... "));
                start_time = 0;
                Is_Start_Counter = FALSE;
                BTHandlerState = BT_MS_IDLE;
                BT_time_status = BT_TIME_INIT;
            }
        }
            break;


    }

}

static void BTHandler_TaskCtrl( bool val )
{
    if ( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( mBTHandleParams.TaskHandle ) != pdPASS ) /*task is working*/
        {            
            start_time = 0;
            Is_Start_Counter = FALSE;
            BTHandlerState = BT_MS_IDLE;
            BT_time_status = BT_TIME_INIT;
            BTStatus = BT_POWER_OFF;
            vTaskSuspend( mBTHandleParams.TaskHandle );
            //TRACE_DEBUG((0, "SUSPEND: AuidoDeviceManager_lowlevel_Exception_Task"));
        }
    }
    else if ( val == BTASK_RESUME )
    {
        if ( xTaskIsTaskSuspended( mBTHandleParams.TaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( mBTHandleParams.TaskHandle );
            //TRACE_DEBUG((0, "RESUME: AuidoDeviceManager_lowlevel_Exception_Task"));
        }
    }
}

#if 0
static void BTHandler_ServiceHandle( void )
{ 

    //for(;;)
    //{       
        switch(BTHandlerState)
        {
            case BT_MS_IDLE:
            {    
                LastBTStatus = BTStatus;
                BTStatus = BTHandler_GetBTStatus();
                
		if ( BTStatus != BT_POWER_OFF )
		{
		    if ( mBTHandlerPowerOnStamp != 0xff )
		        mBTHandlerPowerOnStamp++;
			
		    if ( mBTHandlerPowerOnStamp > 11 );
			mBTHandlerPowerOnStamp = 0xff;	//had powered on more than 1s
		
		}
		else
			mBTHandlerPowerOnStamp = 0;	//No need to count for power on when power off or more than 1s

                /*if( (BTStatus == BT_LINKED) && ( NeedPairing == TRUE) )
                {                     
                    xHMISystemParams srv_parms;

                    NeedPairing = FALSE;
            
                    srv_parms.sys_event = SYS_EVENT_SYSTEM_UP;    // Set LED
                    pUDM_ObjCtrl->SendEvent( &srv_parms ); 
                }*/
				
                if ( (BTStatus == BT_LINKED) && (LastBTStatus != BT_LINKED) )
                {
                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRED);
                }
                
/*              if( BTStatus == BT_LINKED )
                {                     
                    mBTHandlerStamp = 0;
                }
                else
                {                   
                    mBTHandlerStamp++;
                    
                    if(mBTHandlerStamp > 600)
                    {
                        BTHandlerState = BT_MS_PAIRING;
                        mBTHandlerStamp = 0;
                    }
                }
*/
            }
            break;

            case BT_MS_INIT:
            {
/*              mBTHandlerStamp++;

                if ( mBTHandlerStamp == 1 )
                {
                    GPIOMiddleLevel_Clr(__O_BT_INF);
                }
                if ( mBTHandlerStamp == 10 )
                {
                    GPIOMiddleLevel_Set(__O_BT_INF);
                }
                else if ( mBTHandlerStamp >= 31 )
                {
                    GPIOMiddleLevel_Clr(__O_BT_INF);
                   BTHandlerState = BT_MS_IDLE;
                        BTHandler_handleCommand(BT_NOTIFY_POW_SEQ_DONE);
                }
*/
                GPIOMiddleLevel_Clr(__O_BT_REW);
                GPIOMiddleLevel_Clr(__O_BT_FWD);
                GPIOMiddleLevel_Clr(__O_BT_MFB);
                GPIOMiddleLevel_Set(__O_BT_REST);
            
                //mBTHandleComd = BT_NOTIFY_POW_SEQ_DONE;
                //pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleComd);
            
                BTHandlerState = BT_MS_IDLE;
            }
            break;

            case BT_MS_POWER_ON:
            {                
                mBTHandlerStamp++;

                if ( BTStatus == BT_CLEAN_DONE )
                    NeedPairing = TRUE;

                LastBTStatus = BT_POWER_OFF;
                BTStatus = BTHandler_GetBTStatus();

                if(mBTHandlerStamp == 1)
                {
                    GPIOMiddleLevel_Set(__O_BT_REST);                    

                    if ( BTStatus == BT_POWER_OFF )    //Power of BT if BT is not in power off mode
                    {
                        if ( BTHandler_ToogleBTPower(TRUE) )
	    		    mBTHandlerStamp = 0;
                    }
                }
                else
                {
                    if ( BTStatus != BT_POWER_OFF )    //Power of BT if BT is not in power off mode
                    {
                        mBTHandlerStamp = 0;
			mBTHandlerPowerOnStamp = 1;

			if ( BTStatus != BT_LINKED )
                            pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
			
                        if ( NeedPairing == TRUE )
                        {
                            BTHandlerState = BT_MS_PAIRING;
			    NeedPairing = FALSE;
                        }
                        else
                        {
                            BTHandlerState = BT_MS_IDLE;
                        }

                    }
                    else if ( mBTHandlerStamp >= 31 )
                    {
                        TRACE_INFO((0, " BTHandler_handleCommand :  Wait for BT Power ON timeout. "));

                        mBTHandlerStamp = 0;
                        //BTHandlerState = BT_MS_IDLE;
                    }
                }
            }
            break;

            case BT_MS_RESET_BT:
	    {
                mBTHandlerStamp++;
                BTHandlerState = BT_MS_CLEAN_DEVICE;
            }
	    break;
		
            case BT_MS_DEINIT:
            case BT_MS_POWER_OFF:
            {
            //xBTHandleModuleStatus BTStatus = BT_STATUS_ERROR

		if ( mBTHandlerPowerOnStamp != 0xff )	// The time of BT in power on stage is less 1 second 
		{
		        if ( mBTHandlerPowerOnStamp )
		        {
			    mBTHandlerPowerOnStamp++;

			    if ( mBTHandlerPowerOnStamp > 11 )	// The time of BT in power on stage is more than 1 second 
			    {
				mBTHandlerPowerOnStamp = 0xff;
			    }
		        }
			else 
			{
			    if ( BTHandlerState == BT_MS_DEINIT )
			    {
                                mBTHandleComd = BT_NOTIFY_POW_SEQ_DONE;
                                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleComd);
			    }
				
			    BTHandlerState = BT_MS_IDLE;
			}
		}

                //if ( (BTHandlerState == BT_MS_RESET_BT) && (mBTHandlerStamp == 0) ) // Need do clean device before powering off
                //{
                //    mBTHandlerStamp++;
                //    BTHandlerState = BT_MS_CLEAN_DEVICE;
                //}
                else	//power off BT process
                {
                    mBTHandlerStamp++;
                    LastBTStatus = BTStatus;
                    BTStatus = BTHandler_GetBTStatus();
            
                    if ( mBTHandlerStamp == 2 )
                    {
                        GPIOMiddleLevel_Clr(__O_BT_MFB);
                        GPIOMiddleLevel_Clr(__O_BT_REW);
                        GPIOMiddleLevel_Clr(__O_BT_FWD);

                        if ( BTHandlerState != BT_MS_POWER_OFF )
                            GPIOMiddleLevel_Clr(__O_BT_REST);                    

                        //if( BTStatus == BT_PAIRING )                // Need exit Pairing mode before power off BT
                        //    GPIOMiddleLevel_Set(__O_BT_PAIRING);
                    }
                    else if ( mBTHandlerStamp == 3 )
                    {
                        if( BTStatus == BT_PAIRING )
                            GPIOMiddleLevel_Clr(__O_BT_PAIRING);
                    }
                    else if ( mBTHandlerStamp == 4 )
                    {
                        if ( BTStatus != BT_POWER_OFF )    //Power off BT if BT is not in power off mode
                            if ( BTHandler_ToogleBTPower(FALSE) )
	    			mBTHandlerStamp = 3;
                    }
                    else if ( mBTHandlerStamp >= 6 )
                    {
                        if ( BTStatus == BT_POWER_OFF )    //Power off BT if BT is not in power off mode
                        {
                            if ( BTHandlerState == BT_MS_DEINIT)        //Need send notify message to system when powered off while system is powering off
                            {
                                mBTHandleComd = BT_NOTIFY_POW_SEQ_DONE;
                                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleComd);
                            }

                            mBTHandlerStamp = 0;
                            BTHandlerState = BT_MS_IDLE;
			    mBTHandlerPowerOnStamp = 0;

                        }
                        else if ( mBTHandlerStamp >= 36 )
                        {
                            //TRACE_INFO((0, " BTHandler_handleCommand :  BT deinit time out!Can power off BT in 8 seconds"));
                            
                            mBTHandlerStamp = 3;
                            //BTHandlerState = BT_MS_IDLE;
                        }
                    }
                }
            }
            break;
            
            case BT_MS_AUTO_LINK:
            {
		if ( mBTHandlerPowerOnStamp != 0xff )
			mBTHandlerPowerOnStamp++;

		if ( mBTHandlerPowerOnStamp > 11 )
			mBTHandlerPowerOnStamp = 0xff;

                mBTHandlerStamp++;

                if ( mBTHandlerStamp == 1 )
                    GPIOMiddleLevel_Set(__O_BT_MFB);
                else if ( mBTHandlerStamp == 13 )
                {
                    GPIOMiddleLevel_Clr(__O_BT_MFB);
                    mBTHandlerStamp = 0;
                    BTHandlerState = BT_MS_IDLE;
                }
            }
            break;     
            case BT_MS_PLAY_PAUSE:
            {
		if ( mBTHandlerPowerOnStamp != 0 )
			mBTHandlerPowerOnStamp++;

                mBTHandlerStamp++;
                if(mBTHandlerStamp == 1)
                {
                    GPIOMiddleLevel_Set(__O_BT_MFB);
                }
                else if(mBTHandlerStamp == 2)
                {
                    GPIOMiddleLevel_Clr(__O_BT_MFB);
                    mBTHandlerStamp = 0;
                    BTHandlerState = BT_MS_IDLE;
                }
            }
            break;

            case BT_MS_NEXT:
            {
		if ( mBTHandlerPowerOnStamp != 0 )
			mBTHandlerPowerOnStamp++;

                mBTHandlerStamp++;
                if(mBTHandlerStamp == 1)
                {
                    GPIOMiddleLevel_Set(__O_BT_FWD);
                }
                else if(mBTHandlerStamp == 2)
                {
                    GPIOMiddleLevel_Clr(__O_BT_FWD);
                    mBTHandlerStamp = 0;
                    BTHandlerState = BT_MS_IDLE;
                }
            }
            break;

            case BT_MS_PREVIOUS:
            {
		if ( mBTHandlerPowerOnStamp != 0 )
			mBTHandlerPowerOnStamp++;

                mBTHandlerStamp++;
                if(mBTHandlerStamp == 1)
                {
                    GPIOMiddleLevel_Set(__O_BT_REW);
                }
                else if(mBTHandlerStamp == 2)
                {
                    GPIOMiddleLevel_Clr(__O_BT_REW);
                    mBTHandlerStamp = 0;
                    BTHandlerState = BT_MS_IDLE;
                }
            }
            break;

            case BT_MS_PAIRING:
            {
		if ( mBTHandlerPowerOnStamp != 0xff )
			mBTHandlerPowerOnStamp++;

		if ( mBTHandlerPowerOnStamp > 11 )
			mBTHandlerPowerOnStamp = 0xff;

                LastBTStatus = BTStatus;
                BTStatus = BTHandler_GetBTStatus();

                if ( BTStatus == BT_POWER_OFF )
                {
                    TRACE_INFO((0, " BTHandler_handleCommand :  Fail to do BT pairing. BT is in power off mode"));
                    mBTHandlerStamp = 0;
                    BTHandlerState = BT_MS_IDLE;
                    return;
                }
                
                mBTHandlerStamp++;
        
                if ( BTStatus == BT_PAIRING )
                {
                    mBTHandlerStamp = 0;
                    BTHandlerState = BT_MS_IDLE;
                }
                else if(mBTHandlerStamp == 1)
                {
                    GPIOMiddleLevel_Clr(__O_BT_PAIRING);
                }
                else if(mBTHandlerStamp == 2)
                {
                    GPIOMiddleLevel_Set(__O_BT_PAIRING);
                }
                else if(mBTHandlerStamp >= 10)
                {
                    TRACE_INFO((0, " BTHandler_handleCommand :  BT is fail to go into pairing mode in 1s."));

                    mBTHandlerStamp = 0;
                    BTHandlerState = BT_MS_IDLE;
                }
            }
            break;
            
            case BT_MS_CLEAN_DEVICE:
            {
		if ( mBTHandlerPowerOnStamp )	// 0 means power off, non 0 means power on
		{
		    if ( mBTHandlerPowerOnStamp != 0xff )
		    {
			mBTHandlerPowerOnStamp++;

		        if ( mBTHandlerPowerOnStamp > 11 )
			    mBTHandlerPowerOnStamp = 0xff;
		    }
		}

                LastBTStatus = BTStatus;
                BTStatus = BTHandler_GetBTStatus();

                if ( mBTHandlerStamp <= 1 )        // 0 means from factory command, 1 means from BT_MS_RESET_BT (device reset)
                {
                    if ( BTStatus == BT_POWER_OFF )
                        BTHandler_ToogleBTPower(TRUE);

                    if ( mBTHandlerStamp == 0 )    
                    {
                        CleanDeviceCmdSource = 0    ;    // command request from factory command
                    }
                    else
                    {
                        CleanDeviceCmdSource = 1;    // command request from BT_MS_RESET_BT (device reset)
                    }

                    mBTHandlerStamp = 2;
                }
                else 
                {
                    if ( BTStatus != BT_POWER_OFF )
                        mBTHandlerStamp++;
		    else
			break;

                    if( mBTHandlerStamp == 3 )
                    {
                        GPIOMiddleLevel_Set(__O_BT_REW);
                        GPIOMiddleLevel_Set(__O_BT_FWD);
                    }
                    else if( mBTHandlerStamp == 54 )
                    {
                        GPIOMiddleLevel_Clr(__O_BT_REW);
                        GPIOMiddleLevel_Clr(__O_BT_FWD);
                    }
                    else if ( mBTHandlerStamp > 54 ) //wait BT clean time 2.5s (8 device * 300ms = 2.4 s)
                    {
                        TRACE_INFO((0, "BT clean Device is finish !! "));
                       
                        BTStatus = BT_CLEAN_DONE;
                
                        if ( CleanDeviceCmdSource == 0 )
                        {
                            mBTHandlerStamp = 0;
                            BTHandlerState = BT_MS_IDLE;
                        }
                        else if ( CleanDeviceCmdSource == 1 )
                        {
                            mBTHandlerStamp = 1;
                            BTHandlerState = BT_MS_DEINIT;
                        }
                        else
                        {
                            TRACE_INFO((0, "ERROR: Don't who ask for BT Clean Devic! "));

                            mBTHandlerStamp = 0;
                            BTHandlerState = BT_MS_IDLE;
                        }
                        
                    }
                }
            }
            break;

            case BT_MS_END:
            {
		if ( mBTHandlerPowerOnStamp != 0 )
			mBTHandlerPowerOnStamp++;

                mBTHandlerStamp = 0;
            }
            break;
        }

     //   vTaskDelay( TASK_MSEC2TICKS(100) );
    //}
}
#endif


