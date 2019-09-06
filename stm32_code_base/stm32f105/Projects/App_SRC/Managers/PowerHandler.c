#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "PowerHandler.h"
#include "GPIOMiddleLevel.h"

#include "BTHandler.h"
#include "BackupAccessLowLevel.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "HdmiDeviceManager.h"
#include "AudioSystemHandler.h"
#endif

//____________________________________________________________________________________________________________
#define POWER_HANDLER_TICK TASK_MSEC2TICKS(100)
#define POWER_QUEUE_LENGTH 5
#define POWER_STATE_SUBSYS_MAX 4
#define POWER_HANDLER_TIMEOUT 160

#if ( configSII_DEV_953x_PORTING == 1 )        
#define HPD_FROM_TV_IO __I_HPD_FROM_TV
#define HPD_TO_953X_IO __O_HPD_TO_SII953X
#define HPD_TO_DVD_IO __O_HPD_TO_DVD
#endif
//____________________________________________________________________________________________________________
/*typedef enum{
    POWER_HANDLE_IDLE,
    POWER_HANDLE_UP,
    POWER_HANDLE_DOWN
}xPowerHandleState;
*/
typedef struct POWER_HANDLE_PARAMETERS
{
    xTaskHandle TaskHandle;
    xQueueParameters serviceQueue;
 } xPowerHandleParams;

typedef struct POWER_HANDLE_SUBSYSTEM
{
	uint8 handle;
	xPowerStateSubSystem pwr_state;
} xPowerStateSubsystem;

#if ( configSII_DEV_953x_PORTING == 1 )
extern HDMI_DEVICE_MANAGER_OBJECT *pHDMI_DM_ObjCtrl;
extern AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl;
extern const HMI_SERVICE_OBJECT *pHS_ObjCtrl;
#endif

//____________________________________________________________________________________________________________
//static api header
static void PowerHandler_Task( void *pvParameters );

static void PowerHandler_PowerToggle(void);

//static void PowerHandler_GoBackPowerUp( void );

static POWER_STATE PowerHandler_getPowerState(void);

static xPowerHandleState PowerHandler_getPowerHandleState(void);

static uint8 PowerHandler_RegisterHandle( void );

static void PowerHandler_TurnOffDone( uint8 handle );

static void PowerHandler_TurnOnDone( uint8 handle );

static void PowerHandler_ResetPowerHandle( void );

static void PowerHandler_Initialize( void );

static void PowerHandler_PowerCtrl( xPowerHandle_Queue val );

static uint8 PowerHandler_getSystemResetType( void );

static void PowerManager_clrSystemResetType( void );

static void PowerHandler_Power5VOn(bool TurnOn);

static uint8 PowerHandler_getBackupSystemPowerState( void );

#if ( configSII_DEV_953x_PORTING == 1 )        
static bool PowerHandler_Is_HPD_FROM_TV_PlugIn(void);

static void PowerHandler_HPD_TO_953X_CTRL(bool Enable);

static void PowerHandler_HPD_TO_DVD_CTRL(bool Enable);

static bool PowerHandler_CHECK_5V_FROM_DVD(void);
#endif

//____________________________________________________________________________________________________________
const POWER_HANDLE_OBJECT PowerHandle =
{
    PowerHandler_Initialize,
    PowerHandler_RegisterHandle,
    PowerHandler_PowerToggle,
    //PowerHandler_GoBackPowerUp,
    PowerHandler_PowerCtrl,
    PowerHandler_getPowerState,
    PowerHandler_getPowerHandleState,
    PowerHandler_TurnOffDone,
    PowerHandler_TurnOnDone,
    PowerHandler_getSystemResetType,
    PowerManager_clrSystemResetType,
    PowerHandler_Power5VOn,
    PowerHandler_getBackupSystemPowerState,
    PowerHandler_getSystemResetType,
#if ( configSII_DEV_953x_PORTING == 1 )        
    PowerHandler_Is_HPD_FROM_TV_PlugIn,
    PowerHandler_HPD_TO_953X_CTRL,
    PowerHandler_HPD_TO_DVD_CTRL,
    PowerHandler_CHECK_5V_FROM_DVD,
#endif
};
const POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl = &PowerHandle;

//____________________________________________________________________________________________________________
/*static variable*/
static xPowerHandleState mPowerHandleState = POWER_HANDLE_IDLE;
static xPowerHandleParams mPowerHandleParams;
static uint8 mSubsysHandle = 0;
static POWER_STATE mPowerState = POWER_OFF;
static xPowerStateSubsystem pwr_state_subsys[POWER_STATE_SUBSYS_MAX];

#if ( configSII_DEV_953x_PORTING == 1 )      
static bool Is_5V_exist = FALSE;
#endif

//____________________________________________________________________________________________________________
static void PowerHandler_setBackupSystemPowerState( POWER_STATE power_state )
{
    uint16_t value = (0xFF00 | (uint16_t)power_state);

    BackupAccessLowLevel_WriteBackupRegister(PWR_SYSTEM_POWER_STATE, value );
}

static uint8 PowerHandler_getBackupSystemPowerState( void )
{
    uint16 ret = 0;

    ret = BackupAccessLowLevel_ReadBackupRegister(PWR_SYSTEM_POWER_STATE);

    return((uint8)(0x00FF&ret));
}

static uint8 PowerHandler_getSystemResetType( void )
{
    uint16 ret = 0;
    
    ret = BackupAccessLowLevel_ReadBackupRegister(PWR_SYSTEM_RESET_TYPE_INDICATOR);

    return((uint8)(0x00FF&ret));
}

static void PowerManager_clrSystemResetType( void )
{
    BackupAccessLowLevel_WriteBackupRegister(PWR_SYSTEM_RESET_TYPE_INDICATOR, 0xFFFF);
}

static bool PowerHandler_EventSender( void *params ) 
{
    xPowerHandle_Queue* pParams = ( xPowerHandle_Queue *)params;

    if ( pParams == NULL )
    {
        return FALSE;
    }

    if (mPowerHandleParams.serviceQueue.xQueue == NULL )
    {
        TRACE_ERROR((0, "PowerHandler_EventSender error !! "));
        return FALSE;
    }
    
    if ( xQueueSend( mPowerHandleParams.serviceQueue.xQueue, pParams, mPowerHandleParams.serviceQueue.xBlockTime) != pdPASS )
    {
        TRACE_ERROR((0, " PowerHandler_EventSender queue is failure "));
        return FALSE;
    }
    return TRUE;

}

static bool PowerHandler_EventReceiver( xPowerHandle_Queue *pEventParams ) 
{
    if ( pEventParams == NULL )
    {
        TRACE_ERROR((0, " PowerHandler_EventReceiver error !! "));
        return FALSE;
    }
    
    if ( mPowerHandleParams.serviceQueue.xQueue == NULL )
    {
        return FALSE;
    }

    if ( xQueueReceive( mPowerHandleParams.serviceQueue.xQueue, pEventParams, mPowerHandleParams.serviceQueue.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR((0, " PowerHandler_EventReceiver queue is failure "));
        return FALSE;
    }

    return TRUE;
}

static void PowerHandler_Initialize( void )
{
    //Create Q

    mPowerHandleParams.serviceQueue.xQueue = xQueueCreate( POWER_QUEUE_LENGTH,(unsigned portBASE_TYPE )(sizeof(xPowerHandle_Queue)/sizeof(uint8)) );

    mPowerHandleParams.serviceQueue.xBlockTime = portMAX_DELAY;

	if ( xTaskCreate( 
			PowerHandler_Task, 
			( portCHAR * ) "Power_Task", 
			(STACK_SIZE), 
			NULL, 
			tskPOWER_HANDLER_PRIORITY, 
			&mPowerHandleParams.TaskHandle ) != pdPASS )
	{
		vTaskDelete( mPowerHandleParams.TaskHandle );
	}
	else
	{
		memset( pwr_state_subsys, 0, (sizeof(pwr_state_subsys)/sizeof(uint8)) );
	}	
}

static uint8 PowerHandler_RegisterHandle( void )
{
	uint8 i = 0;

	for( i = 0; i < POWER_STATE_SUBSYS_MAX; i++ )
	{
		if ( pwr_state_subsys[i].handle == 0 )
		{
			mSubsysHandle ++;
			pwr_state_subsys[i].handle = mSubsysHandle;
			return pwr_state_subsys[i].handle;
		}
	}

	return 0;
}

static void PowerHandler_TurnOffDone( uint8 handle )
{
	uint8 i = 0;

	if ( handle == 0 )
		return;

	for ( i = 0; i < POWER_STATE_SUBSYS_MAX; i++ )
	{
		if ( pwr_state_subsys[i].handle == handle )
		{
			pwr_state_subsys[i].pwr_state = PWR_STATE_SUBSYS_TURN_OFF_DONE;
		}
	}
		
}

static void PowerHandler_TurnOnDone( uint8 handle )
{
	uint8 i = 0;

	if ( handle == 0 )
		return;

	for ( i = 0; i < POWER_STATE_SUBSYS_MAX; i++ )
	{
		if ( pwr_state_subsys[i].handle == handle )
		{
			pwr_state_subsys[i].pwr_state = PWR_STATE_SUBSYS_TURN_ON_DONE;
		}
	}
}

static void PowerHandler_ResetPowerHandle( void )
{
	uint8 i = 0;

	for ( i = 0; i < POWER_STATE_SUBSYS_MAX; i++ )
	{
		pwr_state_subsys[i].pwr_state = PWR_STATE_SUBSYS_NULL;
	}
}

static bool PowerHandler_IsTurnOffReady( void )
{
	uint8 i = 0;

	for( i = 0; i <POWER_STATE_SUBSYS_MAX; i++ )
	{
		if( pwr_state_subsys[i].handle != 0 )
		{
			 if ( pwr_state_subsys[i].pwr_state != PWR_STATE_SUBSYS_TURN_OFF_DONE )
			 {
                TRACE_DEBUG((0, "Turn off not ready ----%d",i));
                return FALSE;
			 }

#if 0
			 if ( pwr_state_subsys[i].pwr_state == PWR_STATE_SUBSYS_TURN_OFF_DONE )
			 {
				TRACE_DEBUG((0, " power handle %d is ready off", pwr_state_subsys[i].handle ));
			 }
#endif 			 
		}
	}
	PowerHandler_ResetPowerHandle();

	return TRUE;
}

static void PowerHandler_WhichHandleIsNotReadyOff( void )
{
	uint8 i = 0;

	for( i = 0; i <POWER_STATE_SUBSYS_MAX; i++ )
	{
		if( pwr_state_subsys[i].handle != 0 )
		{
			 if ( pwr_state_subsys[i].pwr_state != PWR_STATE_SUBSYS_TURN_OFF_DONE )
			 {
				TRACE_ERROR((0, "TIMEOUT:power handle %d is not ready off", pwr_state_subsys[i].handle ));
			 }
		}
	}
}

static POWER_STATE PowerHandler_getPowerState(void)
{
    return mPowerState;
}

static xPowerHandleState PowerHandler_getPowerHandleState(void)
{
    return mPowerHandleState;
}

static void PowerHandler_PowerToggle(void)
{
    xPowerHandle_Queue tmpPowerHandleQ;

    tmpPowerHandleQ.pwr_handle_id = POWER_HANDLE_OTHER;
    tmpPowerHandleQ.pwr_handle_5v_ctrl = POWER_HANDLE_5V_UNKNOW;
    tmpPowerHandleQ.b_pwr_handle_restart = FALSE;
        
	if ( mPowerState == POWER_OFF )
	{
        TRACE_DEBUG((0, "Power handle ready to power on----"));
        //mPowerHandleState = POWER_HANDLE_UP;
        tmpPowerHandleQ.pwr_handle_state = POWER_HANDLE_UP;
        PowerHandler_EventSender(&tmpPowerHandleQ);    
        
	}
	else if( mPowerState == POWER_ON )
	{
        TRACE_DEBUG((0, "Power handle ready to power off----"));
        //mPowerHandleState = POWER_HANDLE_DOWN;  
        tmpPowerHandleQ.pwr_handle_state = POWER_HANDLE_DOWN;
        PowerHandler_EventSender(&tmpPowerHandleQ);    
		
	}
}

#if 0
static void PowerHandler_GoBackPowerUp( void )
{
    //mPowerHandleState = POWER_HANDLE_UP;
    xPowerHandleState tmpPowerHandleState;

    tmpPowerHandleState = POWER_HANDLE_UP;
    PowerHandler_EventSender(&tmpPowerHandleState);    
}
#endif

static void PowerHandler_Power5VOn(bool TurnOn)
{
    if (TurnOn == TRUE)
    {
        GPIOMiddleLevel_Set(__O_DE_5V);
    }
    else if (TurnOn == FALSE)
    {
        GPIOMiddleLevel_Clr(__O_DE_5V);
    }
}

#if ( configSII_DEV_953x_PORTING == 1 )        
static bool PowerHandler_Is_HPD_FROM_TV_PlugIn(void)
{
    if ( GPIOMiddleLevel_Read_InBit(HPD_FROM_TV_IO) )
        return TRUE;

    return FALSE;
}

static void PowerHandler_HPD_TO_953X_CTRL(bool Enable)
{
    if (Enable)
    {
        GPIOMiddleLevel_Set(HPD_TO_953X_IO);
    }
    else
    {
        GPIOMiddleLevel_Clr(HPD_TO_953X_IO);
    }
}

static void PowerHandler_HPD_TO_DVD_CTRL(bool Enable)
{
    if (Enable)
    {
        GPIOMiddleLevel_Set(HPD_TO_DVD_IO);
    }
    else
    {
        GPIOMiddleLevel_Clr(HPD_TO_DVD_IO);
    }
}

static bool PowerHandler_CHECK_5V_FROM_DVD(void)
{
    if ( GPIOMiddleLevel_Read_InBit(__I_HDMI_WAKE) )
        return TRUE;

    return FALSE;
}

#endif

static void PowerHandler_PowerCtrl( xPowerHandle_Queue val )
{
    xPowerHandle_Queue tmpPowerHandleQ;

    memcpy(&tmpPowerHandleQ, &val, sizeof(xPowerHandle_Queue));

    PowerHandler_EventSender(&tmpPowerHandleQ);
}

static void PowerHandler_Task( void *pvParameters )
{
    //static uint8 timeout = 0;
    //xHMISystemParams SystemParams;
    xPowerHandle_Queue EventParams;

    for(;;)
    {
        if(PowerHandler_EventReceiver(&EventParams) == TRUE)
        {            
            if ((EventParams.b_pwr_handle_restart == TRUE)
                && (EventParams.pwr_handle_id == POWER_HANDLE_SII953X)
                && (EventParams.pwr_handle_state == POWER_HANDLE_DOWN))
            {
                //record the 5v status
                if (EventParams.pwr_handle_5v_ctrl == POWER_HANDLE_5V_ON)
                {
#if ( configSII_DEV_953x_PORTING == 1 )                  
                    Is_5V_exist = TRUE;
#endif                    
                }
                else if (EventParams.pwr_handle_5v_ctrl == POWER_HANDLE_5V_OFF)
                {
#if ( configSII_DEV_953x_PORTING == 1 )                  
                    Is_5V_exist = FALSE;
#endif
                }
            }

            mPowerHandleState = EventParams.pwr_handle_state;
                        
            switch( mPowerHandleState )
            {
                case POWER_HANDLE_IDLE:
                {
                    //timeout = 0;
                }
                    break;
                case POWER_HANDLE_UP:
                {
                    mPowerState = POWER_UP;
                    PowerHandler_setBackupSystemPowerState( mPowerState );

                    TRACE_DEBUG((0, "Turn on 24V and 5V----"));
                    
                    GPIOMiddleLevel_Set(__O_EN_24V);
                    GPIOMiddleLevel_Set(__O_DE_5V);
                    
                    mPowerState = POWER_ON;
                    PowerHandler_setBackupSystemPowerState( mPowerState );
                    
                    mPowerHandleState = POWER_HANDLE_IDLE;
                }
                    break;
    
                case POWER_HANDLE_DOWN:
                {
                    if ( PowerHandler_IsTurnOffReady() == TRUE )
                    {
                        GPIOMiddleLevel_Clr(__O_EN_24V);
#if (configSII_DEV_953x_PORTING == 1)
                        if ( Is_5V_exist == FALSE )
                        {                        
                            //no hdmi cable pluged
                            GPIOMiddleLevel_Clr(__O_DE_5V);
                        }
#else
                        GPIOMiddleLevel_Clr(__O_DE_5V);
#endif
                        //vTaskDelay(TASK_MSEC2TICKS(100));
                        TRACE_DEBUG((0, "system shut down"));
                        
                        mPowerState = POWER_OFF;
                        PowerHandler_setBackupSystemPowerState( mPowerState );
                        
                        mPowerHandleState = POWER_HANDLE_IDLE;
                    }
                    else
                    {
                        // need to check all task suspend complete
                        // like as: BT, ASH, USB, 9533, CC
                        
#if 0 // disable timeout shut down mechanism
                        timeout ++;
    
                        if ( timeout > POWER_HANDLER_TIMEOUT )
                        {
                            PowerHandler_WhichHandleIsNotReadyOff();
                        
                            timeout = 0;
                            GPIOMiddleLevel_Clr(__O_EN_24V);
#if (configSII_DEV_953x_PORTING == 1)
                            if ( GPIOMiddleLevel_Read_InBit(__I_HDMI_WAKE) == 0 )
                            {
                                SystemParams.sys_event = SYS_EVENT_HDMI_AUTO_PWR_OFF;
                                TRACE_DEBUG((0,"Time out PWR_OFF and need to turn off 9533 "));
                                pHDMI_DM_ObjCtrl->SendEvent(&SystemParams );
                                
                                while(!pASH_ObjCtrl->isHDMIPowerOff())
                                {
                                    TRACE_DEBUG((0, " HDMI still work, please wait ..."));
                                    vTaskDelay(1);
                                }

                                //no hdmi cable pluged
                                GPIOMiddleLevel_Clr(__O_DE_5V);
                            }
#else
                            GPIOMiddleLevel_Clr(__O_DE_5V);
#endif
                            vTaskDelay(TASK_MSEC2TICKS(100));
                            mPowerHandleState = POWER_HANDLE_IDLE;
                            
                            mPowerState = POWER_OFF;
                            PowerHandler_setBackupSystemPowerState( mPowerState );
                            break;
                            
                        }
                        else
                        {
                            mPowerState = POWER_DOWN;
                            PowerHandler_setBackupSystemPowerState( mPowerState );
                        }
                        break;
#endif                        
                    }
                }
                    break;
            
            }
        }
        //vTaskDelay( POWER_HANDLER_TICK );
    }
}

