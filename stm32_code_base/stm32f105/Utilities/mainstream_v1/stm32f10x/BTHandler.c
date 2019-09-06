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
#include "AudioDeviceManager.h"


#define BT_AVRCP_CTRL_TIME  TASK_MSEC2TICKS(80)
#define BT_AUTO_PAIRING_TIME TASK_MSEC2TICKS(50000) //50 SEC
#define BT_ENERGY_SAVING_TIME TASK_MSEC2TICKS(900000) //15 MIN

#define BT_START_UP_TIMER 22    // Spec requires 2Sec , Angus added 10% tolerant
#define BT_PAIRING_UP_TIMER 5
#define BT_CLEAN_DEV_UP_TIMER 50
#define BT_ERROR_UP_TIMER 3

#define BT_POWER_TOOGLE_TIME 85
#define BT_PREVENT_AUTO_CONNECT_SOLUTION 1

#define BTHANDLER_QUEUE_LENGTH	10
#define BT_MUTE_POLLING_PERIOD		100
//____________________________________________________________________________________________________________
/* extern variable */
extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
extern UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl;
extern HMI_SERVICE_OBJECT *pHS_ObjCtrl; /*HMI service*/
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;
//____________________________________________________________________________________________________________
//static api header
static void BTHandler_Init(void);
static void BTHandler_CreateTask(void);

static void BTHandler_ServiceHandle( void );

static xBTHandleModuleStatus BTHanlder_reportBTStatus( void );

static bool BTHandler_ToogleBTPower(bool OnOff);

static void BTHandler_handleCommand( void *params  );

static bool BT_GPIO_sema_mutex_take( void );

static bool BT_GPIO_sema_mutex_give( void );

static void BT_GPIO_BT_PAIRING_CTTL(void);

static void BT_GPIO_BT_PLAY_CTTL(void);

static void BT_GPIO_BT_PLAY_NEXT(void);

static void BT_GPIO_BT_PLAY_PRE(void);

static bool BTHandler_TaskCtrl( bool val );

static bool BTHandler_ManagerCommand(xBTManagerState cmd);

static void BTHandler_ServiceManager( void );   //Angus added for improving lock of part of BT

static void BTManager_Service( void *pvParameters );

static void BTMute_Handler(void);

static bool BTHandler_MuteStatus(void);

//____________________________________________________________________________________________________________
const BT_HANDLE_OBJECT   BTHandle =
{
    BTHandler_CreateTask,
    BTHandler_handleCommand,
    BTHanlder_reportBTStatus,
    BTHandler_MuteStatus
};
const BT_HANDLE_OBJECT *pBTHandle_ObjCtrl = &BTHandle;

typedef struct BT_HANDLE_PARAMETERS
{
    xTaskHandle TaskHandle;
    uint8 power_handle;
    xSemaphoreParameters SParams;
} xBTHandleParams;

//____________________________________________________________________________________________________________
/*static variable*/
xBTHandleParams mBTHandleParams;

xBTHandleModuleStatus BTStatus = BT_POWER_OFF;
xBTHandleParams mBTManagerParams;
xBTManagerState BTManagerState = BT_MG_IDLE;
static uint8 BTManagerStep = 0;
static xHMISystemParams BTSystemParams;
static int BTRetryCnt = 0;
static int BTErrorCnt = 0;
static bool BTPaired = FALSE;
static bool BTPairing = FALSE;
static uint8 BTUICnt = 0;   //It can prevent leds show scanning before leds show input bt source.

static bool BTStartTimer = FALSE;   //To start timer of calculation of auto pairing and system shut down.
static portTickType startAutoPairingTime = 0;
static portTickType startSystemShutDownTime = 0;
static bool preventAutoConnect = FALSE;
static int BTMute = -1;

//for mutex mechanism when read/write gpio
xSemaphoreParameters BT_GPIO_sema = NULL;
xQueueHandle bthandler_queue = NULL;

/*******************************************************************************
 * Mutex
 ******************************************************************************/
static bool BT_GPIO_sema_mutex_take( void )
{
    if ( BT_GPIO_sema.xSemaphore == NULL )
    {
        return FALSE;
    }

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

static bool bt_msg_receive(xHMISystemParams *pParm)
{
    if(pdPASS != xQueueReceive(bthandler_queue, pParm, TASK_MSEC2TICKS(BT_MUTE_POLLING_PERIOD)))
        return FALSE;
    else
        return TRUE;
}

static bool bt_msg_send(xHMISystemParams *pParm)
{
    if(NULL==pParm)
    {
        return FALSE;
    }
    
    if(pdPASS==xQueueSend( bthandler_queue, pParm, TASK_MSEC2TICKS(100)))
    {
        return TRUE;
    }
    else
    {
        xHMISystemParams tmp;
        // queue full, give up the oldest one, kaomin
        if(pdPASS != xQueueReceive(bthandler_queue, &tmp, 0))
            return FALSE;
            
        if(pdPASS==xQueueSend( bthandler_queue, pParm, 0 ))
            return TRUE;
        else
            return FALSE;
    }
}


//____________________________________________________________________________________________________________

static void BTHandler_Init(void)
{
    BT_GPIO_sema_mutex_take();

    GPIOMiddleLevel_Clr(__O_BT_REW);
    GPIOMiddleLevel_Clr(__O_BT_FWD);
    GPIOMiddleLevel_Clr(__O_BT_REST);
    GPIOMiddleLevel_Clr(__O_BT_MFB);

    BT_GPIO_sema_mutex_give();
}

static void BTHandler_CreateTask(void)
{
    BT_GPIO_sema.xSemaphore = xSemaphoreCreateMutex();
    BT_GPIO_sema.xBlockTime = portMAX_DELAY;

    bthandler_queue = xQueueCreate( BTHANDLER_QUEUE_LENGTH, FRTOS_SIZE(xHMISystemParams) );

    BTHandler_Init();

    mBTHandleParams.power_handle = pPowerHandle_ObjCtrl->register_handle();

    if ( xTaskCreate(
        BTManager_Service,
         ( portCHAR * ) "BT_Manager",
         (STACK_SIZE),
         NULL,
         tskBT_HANDLER_PRIORITY,
         &mBTManagerParams.TaskHandle ) != pdPASS )
     {
         TRACE_ERROR((0, "BT_Handle task create failure " ));
     }
}

static bool BTHandler_ToogleBTPower(bool OnOff)
{
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {        
        if ( OnOff == TRUE )
        {
            if (BTStatus == BT_POWER_OFF)
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
        }
        BT_GPIO_sema_mutex_give();
        return TRUE;
    }

    return FALSE;
}


static xBTHandleModuleStatus BTHanlder_reportBTStatus( void )
{
    return BTStatus;
}

static void BTHandler_GetBTStatus( xBTHandleModuleStatus *update_status )
{
    uint8 TempStatus0 = 0;
    uint8 TempStatus1 = 0;

    if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
    {
        *update_status = BT_POWER_OFF;
        
        //TRACE_DEBUG((0, "update_status power off---- "));
    }
    else
    {
        ASSERT(BT_GPIO_sema_mutex_take() == TRUE);
        TempStatus0 = GPIOMiddleLevel_Read_InBit(__I_BT_DET2);
        TempStatus1 = GPIOMiddleLevel_Read_InBit(__I_BT_DET1);
        BT_GPIO_sema_mutex_give();

        if (TempStatus0 == 0 && TempStatus1 == 0)
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
    }
}

static void BT_GPIO_BT_PAIRING_CTTL(void)
{
    if (BT_GPIO_sema_mutex_take() == TRUE)
    {   
        TRACE_DEBUG((0, "BT do pair !!"));
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

static void BTHandler_handleCommand( void *params  )
{
    xHMISystemParams* pSystemParms = (xHMISystemParams*)params;

    if ( params == NULL )
        return;

    bt_msg_send(pSystemParms);
        }

static void BTHandler_ServiceHandle( void ) //process 90sec auto pairing and 15mins system shut down
{
    xHMISrvEventParams srv_parms = {xHMI_EVENT_BT, USER_EVENT_UNDEFINED };
    xHMISystemParams HMI_parms;
    if( (pPowerHandle_ObjCtrl->get_power_state() == POWER_ON) &&
         (BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH))
    {
        if(BTStatus == BT_LINKED)
        {
            if(BTPaired == FALSE)
            {
#if BT_PREVENT_AUTO_CONNECT_SOLUTION
                if(preventAutoConnect)
                {
                    //Don't show paired led !!
                    //If we send bt pairing command , SB isn't BT source.
                    //it will from others source switch to BT source , so doing power on first.
                    //BT Module will auto connect devices so we needs waitting over 1.8s  that is stated by Big tommy
                    TRACE_DEBUG((0, "Don't show UI_EVENT_BT_PAIRED !!!!"));
                }
                else
#endif
                {
                    TRACE_DEBUG((0, "UI_EVENT_BT_PAIRED !!!!"));
                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRED);
                    BTPaired = TRUE;
                    BTPairing = FALSE;
                }
            }

            BTStartTimer = FALSE;
        }
        else if(BTStatus == BT_PAIRING || BTStatus == BT_POWER_ON)
        {
            if(BTStatus == BT_PAIRING)
            {
                if(BTPairing == FALSE)
                {
                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
                    BTPaired = FALSE;
                    BTPairing = TRUE;
                }
            }

            if(BTStartTimer == FALSE)
            {
                startSystemShutDownTime = xTaskGetTickCount();
                startAutoPairingTime = xTaskGetTickCount();
                BTStartTimer =  TRUE;
            }

            if(BTStartTimer)
            {
                if(((xTaskGetTickCount() - startSystemShutDownTime)/portTICK_RATE_MS) > BT_ENERGY_SAVING_TIME)
                {
                    startSystemShutDownTime = xTaskGetTickCount();

                    HMI_parms = pHS_ObjCtrl->GetSystemParams();

                    if(HMI_parms.apd)
                    {
                        //send event to HMI
                        srv_parms.event = USER_EVENT_POWER;
                        pHS_ObjCtrl->SendEvent( &srv_parms );
                    }
                    else
                        TRACE_DEBUG((0, "Can't be shut down !!!!!t..."));

                }
                else if(((xTaskGetTickCount() - startAutoPairingTime)/portTICK_RATE_MS) > BT_AUTO_PAIRING_TIME)
                {
                    if (BTStatus != BT_PAIRING)
                    {
                        BTHandler_ManagerCommand(BT_MG_PAIRING);
                    }
                   startAutoPairingTime = xTaskGetTickCount();
                   TRACE_DEBUG((0, "BT auto pairing start..."));
                }
            }
        }
    }
}

static bool BTHandler_TaskCtrl( bool val )
{
    BTPaired = FALSE;
    BTPairing = FALSE;

    if ( val == BTASK_SUSPENDED )
    {
        TRACE_DEBUG((0, "SUSPEND: BTHandler_ServiceHandle"));
        return FALSE;
    }
    else if ( val == BTASK_RESUME )
    {
        BTStartTimer = FALSE;   //To start timer of calculation of auto pairing and system shut down.
        startAutoPairingTime = 0;
        startSystemShutDownTime = 0;
        TRACE_DEBUG((0, "RESUME: BTHandler_ServiceHandle"));
        return TRUE;
    }
    else
        return FALSE;
}

static bool BTHandler_ManagerCommand(xBTManagerState cmd)
{
    BTErrorCnt = 0;
    BTRetryCnt = 0;
    BTManagerStep = 0;
    BTUICnt = 0;
    BTManagerState = cmd;
#if BT_PREVENT_AUTO_CONNECT_SOLUTION
    /* FIX BT ISSUE */
    if(cmd == BT_MG_PAIRING)
    {
        preventAutoConnect = TRUE;
    }
    else
    {
        preventAutoConnect = FALSE;
    }
#endif
    if(cmd == BT_MG_POWER_ON || cmd == BT_MG_SEL_SRC || cmd == BT_MG_PAIRING)
    {
        if(BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH)
        {
            return BTHandler_TaskCtrl(BTASK_RESUME);  
        }
        else
        {
            return BTHandler_TaskCtrl(BTASK_SUSPENDED);  
        }
    }
    else
    {
        return BTHandler_TaskCtrl(BTASK_SUSPENDED);  
    }
}

static void BTHandler_ServiceManager( void )
{
    xPowerHandle_Queue EventParams;
    
    switch(BTManagerState)
    {
        case BT_MG_POWER_ON:
        {
            if(BTManagerStep == 0)
            {
                TRACE_ERROR((0, "BT BT_MG_POWER_ON !!!" ));
                if(BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH)
                {
                    TRACE_ERROR((0, "BT start up ..1111.. " ));
                    BTHandler_ToogleBTPower( TRUE );
                    TRACE_ERROR((0, "BT start up ..2222.. " ));
                    BTManagerStep = 1;
                }
                else
                {
                    BTHandler_ToogleBTPower( FALSE );
                    BTManagerState = BT_MG_IDLE;

                    TRACE_DEBUG((0, "BT SYS_EVENT_SYSTEM_UP - Disable "));
                    pPowerHandle_ObjCtrl->turn_on_done( mBTHandleParams.power_handle );
                }
            }
            else if(BTManagerStep == 1)
            {
                if(BTStatus  != BT_POWER_OFF)   //BT Module has been power on!!
                {
                    TRACE_DEBUG((0, "BT ON , BTRetryCnt = %d",BTRetryCnt));
                    BTRetryCnt = 0;
                    BTManagerStep = 2;
                    BTErrorCnt = 0;
                }
                else
                {
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_START_UP_TIMER)
                    {   
                        TRACE_DEBUG((0, "BT still off , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 0;

                        // BT Module out of work that no response after send command.
                        //Adding this architecture prevent BT Handler is locked
                        BTErrorCnt++;   //After send command, BT Module no reponse.
                        if(BTErrorCnt > BT_ERROR_UP_TIMER)
                        {
                            BTManagerStep = 2;
                            BTErrorCnt = 0;
                        }
                    }
                }
            }
            else
            {
                BTManagerState = BT_MG_IDLE;

                pPowerHandle_ObjCtrl->turn_on_done( mBTHandleParams.power_handle );
            }
        }
        break;

        case BT_MG_POWER_OFF:
        {
            /* Disconnect immediately */
           if(BTManagerStep == 0)
           {
                if(BTStatus == BT_LINKED)
                {
                    BT_GPIO_BT_PAIRING_CTTL();
                    BTManagerStep = 1;
                }
                else
                {
                    BTManagerStep = 2;
                }
            }
           else if(BTManagerStep == 1)
           {
               if(BTStatus  == BT_PAIRING)   //BT Module has been paring mode!!
               {
                   TRACE_DEBUG((0, "BT PAIRING , BTRetryCnt = %d",BTRetryCnt));
                   BTRetryCnt = 0;
                   BTManagerStep = 2;
                   BTErrorCnt = 0;
               }
               else
               {
                   BTRetryCnt++;
                   if(BTRetryCnt > BT_PAIRING_UP_TIMER)
                   {   TRACE_DEBUG((0, "BT Still isn't PAIRING , BTRetryCnt = %d",BTRetryCnt));
                       BTRetryCnt = 0;
                       BTManagerStep = 0;
                       // BT Module out of work that no response after send command.
                       //Adding this architecture prevent BT Handler is locked
                       BTErrorCnt++;   //After send command, BT Module no reponse.
                       if(BTErrorCnt > BT_ERROR_UP_TIMER)
                       {
                           BTManagerStep = 2;
                           BTErrorCnt = 0;
                       }
                   }
               }
           }
            else if(BTManagerStep == 2)
            {
                TRACE_ERROR((0, "BT BT_MG_POWER_OFF !!!" ));
                BTHandler_ToogleBTPower( FALSE );
                BTManagerStep = 3;
            }
            else if(BTManagerStep == 3)
            {
                if(BTStatus  == BT_POWER_OFF)   //BT Module is power off!!
                {
                    TRACE_DEBUG((0, "BT OFF , BTRetryCnt = %d",BTRetryCnt));
                    BTRetryCnt = 0;
                    BTManagerStep = 4;
                    BTErrorCnt = 0;
                }
                else
                {
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_START_UP_TIMER)
                    {   TRACE_DEBUG((0, "BT Still ON , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 2;

                        // BT Module out of work that no response after send command.
                        //Adding this architecture prevent BT Handler is locked
                        BTErrorCnt++;   //After send command, BT Module no reponse.
                        if(BTErrorCnt > BT_ERROR_UP_TIMER)
                        {
                            BTManagerStep = 4;
                            BTErrorCnt = 0;
                        }
                    }
                }
            }
            else
            {
                BTManagerState = BT_MG_IDLE;
                pPowerHandle_ObjCtrl->turn_off_done( mBTHandleParams.power_handle );
                TRACE_DEBUG((0, " BT SYS_EVENT_SYSTEM_DOWN  %d ",mBTHandleParams.power_handle));

                EventParams.pwr_handle_id = POWER_HANDLE_OTHER;
                EventParams.b_pwr_handle_restart = FALSE;
                EventParams.pwr_handle_state = POWER_HANDLE_DOWN;
                EventParams.pwr_handle_5v_ctrl = POWER_HANDLE_5V_UNKNOW;
                pPowerHandle_ObjCtrl->power_ctrl(EventParams);
            }
        }
        break;

        case BT_MG_SEL_SRC: //BT Module Power on when SB source is BT.
        {
            //TRACE_ERROR((0, "BT BT_MG_SEL_SRC !!!" ));
            if(BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH)
            {
#if 1   //Angus added , prevent no show input source  instead of show pairing led directly.
                BTUICnt++;
                if(BTUICnt < 20)    //waiting 2 sec that is reference leds behavior of switching input source
                {
                    return;
                }
#endif
                if(BTManagerStep == 0)
                {
                    BTHandler_ToogleBTPower( TRUE );
                    BTManagerStep = 1;
                }
                else if(BTManagerStep == 1)
                {
                    if(BTStatus  != BT_POWER_OFF)   //BT Module has been power on!!
                    {
                        TRACE_DEBUG((0, "BT ON , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 2;
                        BTErrorCnt = 0;
                    }
                    else
                    {
                        BTRetryCnt++;
                        if(BTRetryCnt > BT_START_UP_TIMER)
                        {   TRACE_DEBUG((0, "BT Still OFF , BTRetryCnt = %d",BTRetryCnt));
                            BTRetryCnt = 0;
                            BTManagerStep = 0;
                            // BT Module out of work that no response after send command.
                            //Adding this architecture prevent BT Handler is locked
                            BTErrorCnt++;   //After send command, BT Module no reponse.
                            if(BTErrorCnt > BT_ERROR_UP_TIMER)
                            {
                                BTManagerStep = 2;
                                BTErrorCnt = 0;
                            }
                        }
                    }
                }
                else
                {
                    BTManagerState = BT_MG_IDLE;
                }
            }
            else
            {
                if(BTManagerStep == 0)
                {
                    BTHandler_ToogleBTPower( FALSE );
                    BTManagerStep = 1;
                    BTHandler_GetBTStatus(&BTStatus);

                    if(BTStatus  == BT_POWER_OFF)
                    {
                        BTManagerStep = 0;
                        BTManagerState = BT_MG_IDLE; //BT_MS_IDLE; // David, fix warning.
                    }
                }
                else if(BTManagerStep == 1)
                {
                    if(BTStatus  == BT_POWER_OFF)   //BT Module hasn't been power off!!
                    {
                        TRACE_DEBUG((0, "BT OFF , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 2;
                        BTErrorCnt = 0;
                    }
                    else
                    {
                        BTRetryCnt++;
                        if(BTRetryCnt > BT_START_UP_TIMER)
                        {   TRACE_DEBUG((0, "BT Still ON , BTRetryCnt = %d",BTRetryCnt));
                            BTRetryCnt = 0;
                            BTManagerStep = 0;
                            // BT Module out of work that no response after send command.
                            //Adding this architecture prevent BT Handler is locked
                            BTErrorCnt++;   //After send command, BT Module no reponse.
                            if(BTErrorCnt > BT_ERROR_UP_TIMER)
                            {
                                BTManagerStep = 2;
                                BTErrorCnt = 0;
                            }
                        }
                    }
                }
                else
                {
                    BTManagerState = BT_MG_IDLE; //BT_MS_IDLE; // David, fix warning.
                }
            }
        }
        break;

        case BT_MG_PAIRING:
        {
            if(BTManagerStep == 0)
            {
#if BT_PREVENT_AUTO_CONNECT_SOLUTION//Angus added , prevention power on BT module auto connecting device.
                if(BTStatus == BT_POWER_OFF)
                {
                    preventAutoConnect = TRUE;
                    BTPaired = TRUE;   //avoid led show paired.
                }
#endif
                BTHandler_ToogleBTPower(TRUE);
                BTManagerStep = 1;
            }
            else if(BTManagerStep == 1)
            {
                if(BTStatus  != BT_POWER_OFF)   //BT Module has been power on!!
                {
                    TRACE_DEBUG((0, "BT ON , BTStatus = %d , BTRetryCnt = %d !!!!!",BTStatus,BTRetryCnt));
                    BTRetryCnt = 0;
                    BTManagerStep = 9;
                    BTErrorCnt = 0;
                }
                else
                {
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_START_UP_TIMER)
                    {   TRACE_DEBUG((0, "BT Still OFF , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 0;
                        // BT Module out of work that no response after send command.
                        //Adding this architecture prevent BT Handler is locked
                        BTErrorCnt++;   //After send command, BT Module no reponse.
                        if(BTErrorCnt > BT_ERROR_UP_TIMER)
                        {
                            BTManagerStep = 9;
                            BTErrorCnt = 0;
                        }
                    }
                }
            }
            else if((BTManagerStep == 9))
            {
#if BT_PREVENT_AUTO_CONNECT_SOLUTION  //Angus added , prevention power on BT module auto connecting device even if we send pairing command.
                if(preventAutoConnect)
                {
                    BT_GPIO_BT_PAIRING_CTTL();
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_START_UP_TIMER)  //Big Tommy stated BT power on to pairing that needs 1.8s least.
                    {
                        BTRetryCnt = 0;
                        BTPaired = FALSE;
                        preventAutoConnect = FALSE;
                        BTManagerStep = 2;
                    }
                }
                else
#endif
                {
                    BTManagerStep = 2;
                }
            }
            else  if(BTManagerStep == 2)
            {
                BT_GPIO_BT_PAIRING_CTTL();
                BTManagerStep = 3;
            }
            else if(BTManagerStep == 3)
            {
                if(BTStatus  == BT_PAIRING)   //BT Module has been paring mode!!
                {
                    TRACE_DEBUG((0, "BT PAIRING , BTRetryCnt = %d",BTRetryCnt));
                    BTRetryCnt = 0;
                    BTManagerStep = 4;
                    BTErrorCnt = 0;
                }
                else
                {
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_PAIRING_UP_TIMER)
                    {   TRACE_DEBUG((0, "BT Still isn't PAIRING , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 2;
                        // BT Module out of work that no response after send command.
                        //Adding this architecture prevent BT Handler is locked
                        BTErrorCnt++;   //After send command, BT Module no reponse.
                        if(BTErrorCnt > BT_ERROR_UP_TIMER)
                        {
                            BTManagerStep = 4;
                            BTErrorCnt = 0;
                        }
                    }
                }
            }
            else
            {
                BTPaired = FALSE;
                BTManagerState = BT_MG_IDLE; //BT_MS_IDLE; // David, fix warning.
            }
        }
        break;

        case BT_MG_CLEAN_DEVICE:
        {
            if(BTManagerStep == 0)
            {
                BTHandler_ToogleBTPower( TRUE );
                BTManagerStep = 1;
            }
            else if(BTManagerStep == 1)
            {
                if(BTStatus  != BT_POWER_OFF)   //BT Module has been power on!!
                {
                    TRACE_DEBUG((0, "BT ON , BTRetryCnt = %d",BTRetryCnt));
                    BTRetryCnt = 0;
                    BTManagerStep = 2;
                    BTErrorCnt = 0;
                }
                else
                {
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_START_UP_TIMER)
                    {   TRACE_DEBUG((0, "BT Still OFF , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 0;
                        // BT Module out of work that no response after send command.
                        //Adding this architecture prevent BT Handler is locked
                        BTErrorCnt++;   //After send command, BT Module no reponse.
                        if(BTErrorCnt > BT_ERROR_UP_TIMER)
                        {
                            BTManagerStep = 2;
                            BTErrorCnt = 0;
                        }
                    }
                }
            }
            /* Disconnect immediately */
            else  if(BTManagerStep == 2)
            {
                if(BTSystemParams.op_mode != MODE_FACTORY)
                {
                    BT_GPIO_BT_PAIRING_CTTL();  //It will effect factory production but it must be existed in user mode. 
                }
                
                BTManagerStep = 3;
            }
            else if(BTManagerStep == 3)
            {
                if(BTStatus  == BT_PAIRING)   //BT Module has been paring mode!!
                {
                    TRACE_DEBUG((0, "BT PAIRING , BTRetryCnt = %d",BTRetryCnt));
                    BTRetryCnt = 0;
                    BTManagerStep = 4;
                    BTErrorCnt = 0;
                }
                else
                {
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_PAIRING_UP_TIMER)
                    {   TRACE_DEBUG((0, "BT Still isn't PAIRING , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 2;
                        // BT Module out of work that no response after send command.
                        //Adding this architecture prevent BT Handler is locked
                        BTErrorCnt++;   //After send command, BT Module no reponse.
                        if(BTErrorCnt > BT_ERROR_UP_TIMER)
                        {
                            BTManagerStep = 4;
                            BTErrorCnt = 0;
                        }
                    }
                }
            }
            else if(BTManagerStep == 4)
            {
                BT_GPIO_sema_mutex_take();
                GPIOMiddleLevel_Set(__O_BT_REW);
                GPIOMiddleLevel_Set(__O_BT_FWD);
                BT_GPIO_sema_mutex_give();
                BTManagerStep = 5;
            }
            else if(BTManagerStep == 5)
            {
                BTRetryCnt++;
                if(BTRetryCnt > BT_CLEAN_DEV_UP_TIMER)
                {   
                    TRACE_DEBUG((0, "BT Clear , BTRetryCnt = %d",BTRetryCnt));
                    BT_GPIO_sema_mutex_take();
                    GPIOMiddleLevel_Clr(__O_BT_REW);
                    GPIOMiddleLevel_Clr(__O_BT_FWD);
                    BT_GPIO_sema_mutex_give();
                    BTRetryCnt = 0;
                    BTManagerStep = 6;
                }
            }
            else if(BTManagerStep == 6)
            {
                //delay least over 100ms... then doing power off
                TRACE_DEBUG((0, "BT BT_MG_CLEAN_DEVICE !!!" ));
                BTManagerStep = 7;
            }
            else if(BTManagerStep == 7)
            {
                BTHandler_ToogleBTPower( FALSE );
                BTManagerStep = 8;
            }
            else if(BTManagerStep == 8)
            {
                if(BTStatus  == BT_POWER_OFF)   //BT Module hasn't been power off!!
                {
                    TRACE_DEBUG((0, "BT OFF , BTRetryCnt = %d",BTRetryCnt));
                    BTRetryCnt = 0;
                    BTManagerStep = 9;
                    BTErrorCnt = 0;
                }
                else
                {
                    BTRetryCnt++;
                    if(BTRetryCnt > BT_START_UP_TIMER)
                    {   TRACE_DEBUG((0, "BT Still ON , BTRetryCnt = %d",BTRetryCnt));
                        BTRetryCnt = 0;
                        BTManagerStep = 7;
                        // BT Module out of work that no response after send command.
                        //Adding this architecture prevent BT Handler is locked
                        BTErrorCnt++;   //After send command, BT Module no reponse.
                        if(BTErrorCnt > BT_ERROR_UP_TIMER)
                        {
                            BTManagerStep = 9;
                            BTErrorCnt = 0;
                        }
                    }
                }
            }
            else
            {
                BTManagerState = BT_MG_IDLE;
                pPowerHandle_ObjCtrl->turn_off_done( mBTHandleParams.power_handle );
                TRACE_DEBUG((0, " BT SYS_EVENT_SYSTEM_DOWN "));
                
                EventParams.pwr_handle_id = POWER_HANDLE_OTHER;
                EventParams.b_pwr_handle_restart = FALSE;
                EventParams.pwr_handle_state = POWER_HANDLE_DOWN;
                EventParams.pwr_handle_5v_ctrl = POWER_HANDLE_5V_UNKNOW;
                pPowerHandle_ObjCtrl->power_ctrl(EventParams);                
            }
        }
        break;
    }
}

static void BTManager_Service( void *pvParameters )
{
    xHMISystemParams tmpParms;
    static bool bServiceHandle = FALSE;;

    for ( ;; )
    {
        BTHandler_GetBTStatus(&BTStatus);
        
        if(bt_msg_receive(&tmpParms)) // timeout per 100ms
        {
            if( tmpParms.sys_event == SYS_EVENT_SEL_SRC && 
                tmpParms.input_src == AUDIO_SOURCE_BLUETOOTH &&
                BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH)
            {
                if(BTStatus == BT_PAIRING)
                {
                    BTPairing = FALSE;
                }
                continue;
            }
            else
            {
                BTSystemParams = tmpParms;
            }

            switch( BTSystemParams.sys_event )
            {
                case SYS_EVENT_SYSTEM_UP:
                {
                    bServiceHandle = BTHandler_ManagerCommand(BT_MG_POWER_ON);
                }
                break;
        
                case SYS_EVENT_SYSTEM_DOWN:
                {
                    bServiceHandle = BTHandler_ManagerCommand(BT_MG_POWER_OFF);
                }
                break;
        
                case SYS_EVENT_SEL_SRC:
                {
                    bServiceHandle = BTHandler_ManagerCommand(BT_MG_SEL_SRC);
                }
                break;
        
                case SYS_EVENT_BT_PAIRING:
                {
                    if ( BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH )
                    {
                        bServiceHandle = BTHandler_ManagerCommand(BT_MG_PAIRING);
                        TRACE_DEBUG((0, "BT is pairing..."));
                    }
                }
                break;
        
                case SYS_EVENT_BT_AVRCP_PLAY_PAUSE:
                {
                    if( ( BTStatus == BT_LINKED ) && ( BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH ) )
                    {
                        BT_GPIO_BT_PLAY_CTTL();
                        TRACE_DEBUG((0, "BT AVRCP PLAY or PAUSE "));
                    }
                }
                break;
        
                case SYS_EVENT_BT_AVRCP_NEXT:
                {
                    if( ( BTStatus == BT_LINKED ) && ( BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH ) )
                    {
                        BT_GPIO_BT_PLAY_NEXT();
                        TRACE_DEBUG((0, "BT AVRCP NEXT"));
                    }
                }
                break;
        
                case SYS_EVENT_BT_AVRCP_PREVIOUS:
                {
                    if( ( BTStatus == BT_LINKED ) && ( BTSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH ) )
                    {
                        BT_GPIO_BT_PLAY_PRE();
                        TRACE_DEBUG((0, "BT AVRCP PREVIOUS"));
                    }
                }
                break;
        
                case SYS_EVENT_SYSTEM_RESET:
                case SYS_EVENT_BT_CLEAN_DEVICE:
                {
                    bServiceHandle = BTHandler_ManagerCommand(BT_MG_CLEAN_DEVICE);
                }
                break;
        
                default:
                    continue;
            }    
        }

        if(BTManagerState != BT_MG_IDLE)
            BTHandler_ServiceManager();

        if(bServiceHandle)
            BTHandler_ServiceHandle();

        BTMute_Handler();
    }
}

void BTMute_Handler(void)
{
    xHMISystemParams HMI_parms;
    bool BTMuteGpiostatus = FALSE;

    if(pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)
    {
        HMI_parms = pHS_ObjCtrl->GetSystemParams();
        if(HMI_parms.input_src == AUDIO_SOURCE_BLUETOOTH)
        {
            BT_GPIO_sema_mutex_take();
            BTMuteGpiostatus = GPIOMiddleLevel_Read_InBit(__I_BT_INF);
            BT_GPIO_sema_mutex_give();

            if(BTMuteGpiostatus)    //unmute
            {
                if(BTMute != 0)
                {
                    TRACE_DEBUG((0,"BT_INF is High !!!!  set Unmute"));
                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_SET_DEMUTE);
                    BTMute = 0;
                }
            }
            else
            {
                if(BTMute != 1)
                {
                    TRACE_DEBUG((0,"BT_INF is Low !!!!! set mute"));
                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_SET_MUTE);
                    BTMute = 1;
                }
            }
        }
        else
        {
            BTMute = -1;
        }
    }
}

static bool BTHandler_MuteStatus(void)
{
    if(BTMute == 1)
        return TRUE;
    else
        return FALSE;
}

