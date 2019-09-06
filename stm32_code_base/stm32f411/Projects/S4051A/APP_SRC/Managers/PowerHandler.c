#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "PowerHandler.h"
#include "GPIOMiddleLevel.h"

#include "BTHandler.h"
#if ( configSTM32F411_PORTING == 1 )
#include "BackupAccessLowLevel.h"
#endif
//____________________________________________________________________________________________________________
#define POWER_HANDLER_TICK TASK_MSEC2TICKS(100)
#define POWER_QUEUE_LENGTH 5
#define POWER_STATE_SUBSYS_MAX 3
#define POWER_HANDLER_TIMEOUT 160

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
 } xPowerHandleParams;

typedef struct POWER_HANDLE_SUBSYSTEM
{
	uint8 handle;
	xPowerStateSubSystem pwr_state;
} xPowerStateSubsystem;

//____________________________________________________________________________________________________________
//static api header
static void PowerHandler_Task( void *pvParameters );

static void PowerHandler_PowerToggle(void);

static POWER_STATE PowerHandler_getPowerState(void);

static xPowerHandleState PowerHandler_getPowerHandleState(void);

static uint8 PowerHandler_RegisterHandle( void );

static void PowerHandler_TurnOffDone( uint8 handle );

static void PowerHandler_TurnOnDone( uint8 handle );

static void PowerHandler_ResetPowerHandle( void );

static void PowerHandler_Initialize( void );

static void PowerHandler_PowerCtrl( uint8 val );

static uint8 PowerHandler_getSystemResetType( void );

static void PowerManager_clrSystemResetType( void );
//____________________________________________________________________________________________________________
const POWER_HANDLE_OBJECT PowerHandle =
{
    PowerHandler_Initialize,
    PowerHandler_RegisterHandle,
    PowerHandler_PowerToggle,
    PowerHandler_PowerCtrl,
    PowerHandler_getPowerState,
    PowerHandler_getPowerHandleState,
    PowerHandler_TurnOffDone,
    PowerHandler_TurnOnDone,
    PowerHandler_getSystemResetType,
    PowerManager_clrSystemResetType,
};
const POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl = &PowerHandle;

//____________________________________________________________________________________________________________
/*static variable*/
static xPowerHandleState mPowerHandleState = POWER_HANDLE_IDLE;
static xPowerHandleParams mPowerHandleParams;
static uint8 mSubsysHandle = 0;
static POWER_STATE mPowerState = POWER_OFF;
static xPowerStateSubsystem pwr_state_subsys[POWER_STATE_SUBSYS_MAX];

//____________________________________________________________________________________________________________
static void PowerHandler_Initialize( void )
{
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
	if ( mPowerState == POWER_OFF )
	{
		mPowerHandleState = POWER_HANDLE_UP; 
	}
	else if( mPowerState == POWER_ON )
	{
		mPowerHandleState = POWER_HANDLE_DOWN;  
	}
}

static void PowerHandler_PowerCtrl( uint8 val )
{
	if ( val == POWER_UP )
	{
		mPowerHandleState = POWER_HANDLE_UP;
	}
	else if ( val == POWER_DOWN )
	{
		mPowerHandleState = POWER_HANDLE_DOWN;
	}
}

static void PowerHandler_Task( void *pvParameters )
{
	static uint8 timeout = 0;

    for(;;)
    {
		switch( mPowerHandleState )
		{
			case POWER_HANDLE_IDLE:
			{
				timeout = 0;
			}
				break;
			case POWER_HANDLE_UP:
			{
				mPowerState = POWER_UP;
				
				GPIOMiddleLevel_Set(__O_EN_24V);
				GPIOMiddleLevel_Set(__O_DE_5V);
				GPIOMiddleLevel_Set(__O_EN_1V);
				mPowerState = POWER_ON;
				mPowerHandleState = POWER_HANDLE_IDLE;
			}
				break;

			case POWER_HANDLE_DOWN:
			{
				if ( PowerHandler_IsTurnOffReady() == TRUE )
                {
					GPIOMiddleLevel_Clr(__O_EN_24V);
					GPIOMiddleLevel_Clr(__O_DE_5V);
					GPIOMiddleLevel_Clr(__O_EN_1V);
					vTaskDelay(TASK_MSEC2TICKS(100));
                    TRACE_DEBUG((0, "system shut down"));
					mPowerState = POWER_OFF;
					mPowerHandleState = POWER_HANDLE_IDLE;
                }
                else
                {
					timeout ++;

					if ( timeout > POWER_HANDLER_TIMEOUT )
					{
						PowerHandler_WhichHandleIsNotReadyOff();
					
						timeout = 0;
						GPIOMiddleLevel_Clr(__O_EN_24V);
						GPIOMiddleLevel_Clr(__O_DE_5V);
						GPIOMiddleLevel_Clr(__O_EN_1V);
						vTaskDelay(TASK_MSEC2TICKS(100));
						mPowerHandleState = POWER_HANDLE_IDLE;
						mPowerState = POWER_OFF;
						break;
						
					}
					else
					{
                		mPowerState = POWER_DOWN;
                	}
					break;
                }
			}
				break;
		
		}

        vTaskDelay( POWER_HANDLER_TICK );
    }
}

static uint8 PowerHandler_getSystemResetType( void )
{
    uint16 ret = 0;
#if ( configSTM32F411_PORTING == 1 )
    ret = BackupAccessLowLevel_ReadBackupRegister(PWR_SYSTEM_RESET_TYPE_INDICATOR);
#endif
    return((uint8)(0x00FF&ret));
}

static void PowerManager_clrSystemResetType( void )
{
#if ( configSTM32F411_PORTING == 1 )  
    BackupAccessLowLevel_WriteBackupRegister(PWR_SYSTEM_RESET_TYPE_INDICATOR, 0xFFFF);
#endif
}
