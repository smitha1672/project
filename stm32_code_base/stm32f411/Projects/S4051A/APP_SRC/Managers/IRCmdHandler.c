/* Scheduler includes. */
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"

#include "device_config.h"
#include "command.h"
#include "Debug.h"

#include "HMI_Service.h" 
#include "IRKeyCode.h"
#include "IRLowLevel.h"
#include "IRCmdHandler.h"
#include "PowerHandler.h"
#include "StorageDeviceManager.h"
#include "UIDeviceManager.h"
//____________________________________________________________________________________________________________
#define IR_BUILD_DEBUG 0

//____________________________________________________________________________________________________________
typedef struct IR_CMD_PARAMETERS
{
    xTaskHandle IrCmdTaskHandle;
    xOS_TaskErrIndicator xOS_ErrId;
    xQueueParameters queue;
} xOS_IR_CMD_HanlderParams;

typedef enum{
	IR_MODE_NULL = 0,
	IR_MODE_RELOAD_PRG_KEY,	
	IR_MODE_PROGRAM,
	IR_MODE_OPERATION
}IR_MODE;

typedef enum {
	IR_PRG_NULL = 0,
	IR_PRG_FAILURE,
	IR_PRG_SUCCESS,
}eIR_PRG_REMOTE_STATE;

typedef struct _IR_PRG_REMOTE_STATE
{
	IR_MODE ir_mode;
	eIR_PRG_REMOTE_STATE ir_prg_state;
	uint8 index;
}IR_PRG_REMOTE_STATE;


//____________________________________________________________________________________________________________
xOS_IR_CMD_HanlderParams xOS_IrCmd;

//____________________________________________________________________________________________________________
//static api header
static void IRCmdHandler_Initialize( void );

static void IRCmdHandler_ServiceHandle(void *pvParameters);

static void IRCmdHandler_CreateTask(void);

static uint16 IRCmdHandler_getQueueNumber( void );

static void IRCmdHandler_getQueue( IR_PROTOCOL_PAYLOAD *pData );

static void IRCmdHandler_InsertQueue( IR_PROTOCOL_PAYLOAD ir_payload );

static void IRCmdHandler_ProgramRemoteEvent( xHMIUserEvents event );

static void IRCmdHandler_Initialize( void );

static void IRCmdHandler_ReloadProgramRemoteEvent( void );


//____________________________________________________________________________________________________________
/* extern variable */

#if ( configSTM32F411_PORTING == 0 )
extern HMI_SERVICE_OBJECT *pHS_ObjCtrl; /*HMI service*/
//extern STORAGE_DEVICE_MANAGER_OBJECT *pSDM_ObjCtrl;
#endif


const IR_CMD_HANDLER_OBJECT   IRCmdHandle =
{
    IRCmdHandler_CreateTask,
    IRCmdHandler_getQueueNumber,
    IRCmdHandler_getQueue,
    IRCmdHandler_InsertQueue
};

const IR_CMD_PROGRAM_OBJECT IRCMDPrgHandle = 
{
	IRCmdHandler_Initialize,	
	IRCmdHandler_ProgramRemoteEvent,
};

const IR_CMD_HANDLER_OBJECT *pIRCmdHandle_ObjCtrl = &IRCmdHandle;
const IR_CMD_PROGRAM_OBJECT *pIR_PrgRemote_ObjCtrl = &IRCMDPrgHandle;

//____________________________________________________________________________________________________________
/*static variable*/
static const uint8 SIZE_AMTRAN_IR_CMD_KEY_MAP = sizeof(IR_CMD_KEY_MAP)/sizeof(IR_CMD_KEY_MAP[0]);
static const uint8 SIZE_VIZ_FAC_IR_CMD_KEY_MAP = sizeof(VIZ_FAC_IR_CMD_KEY_MAP)/sizeof(VIZ_FAC_IR_CMD_KEY_MAP[0]);

IR_PRG_REMOTE_STATE mIrState = { IR_MODE_RELOAD_PRG_KEY, IR_PRG_NULL, 0 };
IR_PRG_REMOTE_USER_EVENT mIrPrgRemoteUserEvent[3] = 
{
	{IR_PROTOCOL_NULL, IR_PROTOCOL_REPEAT_NULL, 0xffffffff, USER_EVENT_VOL_UP },
	{IR_PROTOCOL_NULL, IR_PROTOCOL_REPEAT_NULL, 0xffffffff, USER_EVENT_VOL_DN },
	{IR_PROTOCOL_NULL, IR_PROTOCOL_REPEAT_NULL, 0xffffffff, USER_EVENT_MUTE }
};

//____________________________________________________________________________________________________________
static xHMISrvEventParams IRCmdHandler_TriggerFactoryModeEventHandle( NEC_PROTOCOL_PAYLOAD nec_ir_payload )
{
	uint16 ir_address = 0;
	static portTickType xTimeTick = 0;
	static uint8 check_state = 0;
	xHMISrvEventParams srv_parms = {xHMI_EVENT_NULL, USER_EVENT_UNDEFINED };

	ir_address = ( ( nec_ir_payload.address << 8 ) | nec_ir_payload.int_address );

	if ( ir_address == VIZ_FAC_RMT_ADDRESS )
	{

		switch( check_state )
		{
			case 0:
			{
				if ( ENTER_FACTORY_KEY_MAP[0] == nec_ir_payload.command )
				{
					xTimeTick = xTaskGetTickCount( );
					check_state++;
				}
			}
				break;

			case 1:
			{
				if ( ENTER_FACTORY_KEY_MAP[1] == nec_ir_payload.command )
				{
					if ( (xTaskGetTickCount( ) - xTimeTick) < 2000 )
					{
						check_state++;
						xTimeTick = xTaskGetTickCount( );
					}
				}
				else
				{
					check_state = 0;
				}
			}
				break;

			case 2:
			{
				if ( ENTER_FACTORY_KEY_MAP[2] == nec_ir_payload.command )
				{
					if ( (xTaskGetTickCount( ) - xTimeTick) < 2000 )
					{
						check_state++;
						xTimeTick = xTaskGetTickCount( );
					}
				}
				else
				{
					check_state = 0;
				}
			}
				break;

			case 3:
			{
				if ( ENTER_FACTORY_KEY_MAP[3] == nec_ir_payload.command )
				{
					if ( (xTaskGetTickCount( ) - xTimeTick) < 2000 )
					{
						check_state++;
						xTimeTick = xTaskGetTickCount( );
					}
				}
				else
				{
					check_state = 0;
				}
			}
				break;

			case 4:
			{
				if ( ENTER_FACTORY_KEY_MAP[4] == nec_ir_payload.command )
				{
					if ( (xTaskGetTickCount( ) - xTimeTick) < 2000 )
					{
						srv_parms.event_id = xHMI_EVENT_XRS351_IR;
						srv_parms.event = USER_EVENT_MODE_FACTORY;
						check_state = 0;
						xTimeTick = xTaskGetTickCount( );
					}
				}
				else
				{
					check_state = 0;
				}
			}
				break;

		}
		
	}


	return srv_parms;
	
}

static xHMISrvEventParams IRCmdHandler_UserEventHandle( NEC_PROTOCOL_PAYLOAD nec_ir_payload )
{
	uint16 ir_address = 0;
	xHMISrvEventParams srv_parms = {xHMI_EVENT_NULL, USER_EVENT_UNDEFINED };
	uint8 i = 0;
	
	ir_address = ( ( nec_ir_payload.address << 8 ) | nec_ir_payload.int_address );
	
	if ( ir_address == XRS351_ADDRESS )
	{

#if 0
		TRACE_DEBUG((0, "sb ir command = 0x%X", nec_ir_payload.command ));
#endif 	

		for( i=0; i<SIZE_AMTRAN_IR_CMD_KEY_MAP; i++ )
		{
			if(IR_CMD_KEY_MAP[i].keyCode == nec_ir_payload.command )
			{
				srv_parms.event_id = xHMI_EVENT_XRS351_IR;
				srv_parms.event = IR_CMD_KEY_MAP[i].UserEvent;
				break;
			}
		}
	}
	else if ( ir_address == VIZ_FAC_RMT_ADDRESS )
	{
		for( i=0; i<SIZE_VIZ_FAC_IR_CMD_KEY_MAP; i++ )
	    {
	        if( VIZ_FAC_IR_CMD_KEY_MAP[i].keyCode == nec_ir_payload.command )
	        {
				srv_parms.event_id = xHMI_EVENT_VIZ_FAC_IR;
	            srv_parms.event = VIZ_FAC_IR_CMD_KEY_MAP[i].UserEvent;
				break;
	        }
	    }
	}
	
    return srv_parms; 
}

static uint16 IRCmdHandler_getQueueNumber( void )
{
    if(  xOS_IrCmd.queue.xQueue != NULL )
    {
        return (uint16)uxQueueMessagesWaiting(  xOS_IrCmd.queue.xQueue );
    }

    return 0;
}

static void IRCmdHandler_getQueue( IR_PROTOCOL_PAYLOAD *pData )
{
    if ( pData == NULL )
        return;

    if ( xOS_IrCmd.queue.xQueue == NULL )
    {
        return;
    }

	if ( xQueueReceive( xOS_IrCmd.queue.xQueue, pData , xOS_IrCmd.queue.xBlockTime ) != pdPASS )
    {
        xOS_IrCmd.xOS_ErrId = xOS_TASK_QUEUE_GET_FAIL;
    }

	xQueueReset( xOS_IrCmd.queue.xQueue );
	return;
}

static void IRCmdHandler_InsertQueue( IR_PROTOCOL_PAYLOAD ir_payload )
{
	IR_PROTOCOL_PAYLOAD data = ir_payload;
	
	if( xOS_IrCmd.queue.xQueue != NULL )
	{
		xQueueReset( xOS_IrCmd.queue.xQueue );
		if (xQueueSend( xOS_IrCmd.queue.xQueue, &data, xOS_IrCmd.queue.xBlockTime)!= pdPASS )
        {
            xOS_IrCmd.xOS_ErrId = xOS_TASK_QUEUE_SET_FAIL;
        }
	}
}

static xHMIUserEvents IRCmdHandler_AdmitRepeatUserEvent( xHMIUserEvents event )
{
	xHMIUserEvents ret = USER_EVENT_NULL;
#if ( configSII_DEV_953x_PORTING == 1 )
    if( ( event == USER_EVENT_VOL_UP ) || \
        ( event == USER_EVENT_VOL_DN ) || \
        ( event == USER_EVENT_BASS_UP ) || \
        ( event == USER_EVENT_BASS_DN ) || \
        ( event == USER_EVENT_TREBLE_UP ) || \
        ( event == USER_EVENT_TREBLE_DN ) || \
        ( event == USER_EVENT_SUB_UP ) || \
        ( event == USER_EVENT_SUB_DN ) || \
        ( event == USER_EVENT_CENTER_UP ) || \
        ( event == USER_EVENT_CENTER_DN ) || \
        ( event == USER_EVENT_REAR_UP ) || \
        ( event == USER_EVENT_REAR_DN ) || \
        ( event == USER_EVENT_BALANCE_UP ) || \
        ( event == USER_EVENT_BALANCE_DN ) || \
        ( event == USER_EVENT_AV_DELAY_UP ) || \
        ( event == USER_EVENT_AV_DELAY_DN ) || \
        ( event == USER_EVENT_START_AUTO_SEL_SRC)
      )
    {
        ret = event;
    }
#else
	if( ( event == USER_EVENT_VOL_UP ) || \
		( event == USER_EVENT_VOL_DN ) || \
		( event == USER_EVENT_BASS_UP ) || \
		( event == USER_EVENT_BASS_DN ) || \
		( event == USER_EVENT_TREBLE_UP ) || \
		( event == USER_EVENT_TREBLE_DN ) || \
		( event == USER_EVENT_SUB_UP ) || \
		( event == USER_EVENT_SUB_DN ) || \
		( event == USER_EVENT_CENTER_UP ) || \
		( event == USER_EVENT_CENTER_DN ) || \
		( event == USER_EVENT_REAR_UP ) || \
		( event == USER_EVENT_REAR_DN ) || \
		( event == USER_EVENT_BALANCE_UP ) || \
		( event == USER_EVENT_BALANCE_DN ) || \
		( event == USER_EVENT_AV_DELAY_UP ) || \
        ( event == USER_EVENT_AV_DELAY_DN ) || \
        ( event == USER_EVENT_START_AUTO_SEL_SRC)
		)
	{
		ret = event;
	}
#endif   
    return ret;

}

static xHMIUserEvents IRCmdHandler_VizioSpecialUserEvent(bool repeatCode, xHMIUserEvents event)
{
 //Angus added for VIZIO Spec 2.1.2 which is defined long press input key will enter auto detect mode.
    static portTickType RepeatTick = 0;
    portTickType RepeatDuration;
    static bool startTimer = FALSE;
    xHMIUserEvents ret = event;  


    if(repeatCode)
    {
        switch(event)
        {
            case USER_EVENT_SEL_AUX1:
            case USER_EVENT_SEL_AUX2:
            case USER_EVENT_SEL_COAXIAL:
            case USER_EVENT_SEL_OPTICAL:
#if( configSII_DEV_953x_PORTING == 1 )
            case USER_EVENT_SEL_HDMI_IN:
            case USER_EVENT_SEL_HDMI_ARC:
#endif
            case USER_EVENT_SEL_BT:
            case USER_EVENT_SEL_USB_PLAYBACK:   
            {
                if(startTimer == FALSE)
                {
                    startTimer = TRUE;
                    RepeatDuration = xTaskGetTickCount( );
                    RepeatTick = RepeatDuration;
                }
                else
                {
                    RepeatDuration = ( xTaskGetTickCount( ) - RepeatTick);
                    if((RepeatDuration/portTICK_RATE_MS) > TASK_MSEC2TICKS(2000))
                    {
                        ret = USER_EVENT_START_AUTO_SEL_SRC;

                        RepeatDuration = 0;
                        RepeatTick = 0;
                        startTimer = FALSE;
                    }
                }
            }
            break;
    
            default:
            {
                RepeatDuration = 0;
                RepeatTick = 0;
                startTimer = FALSE;
            }
            break;           
        }
        
    }
    else
    {
        if(startTimer)
        {
            startTimer = FALSE;
            RepeatDuration = 0;
            RepeatTick = 0;
            startTimer = FALSE;
        }  
    }

	return ret;
}

static xHMISrvEventParams IRCmdHandler_ProgramCode_UserEventHandle( IR_PROTOCOL_PAYLOAD ir_payload )
{
	uint8 i = 0;
	xHMISrvEventParams event = { xHMI_EVENT_NULL, USER_EVENT_UNDEFINED };

	for( i = 0; i < 3; i++ )
	{
		if( mIrPrgRemoteUserEvent[i].ir_payload.payload == ir_payload.payload )
		{
			event.event_id = xHMI_EVENT_PROGRAM_IR;
			event.event = mIrPrgRemoteUserEvent[i].user_event;
		}
	}

	return event;
}

static xHMISrvEventParams IRCmdHandler_NEC_CmdEvent( IR_PROTOCOL_PAYLOAD ir_payload )
{
	NEC_PROTOCOL_PAYLOAD *pNecProtocol;
	IR_PROTOCOL_PAYLOAD IrPayload = ir_payload;
	xHMISrvEventParams srv_parms = { xHMI_EVENT_NULL, USER_EVENT_UNDEFINED };
	

	pNecProtocol = (NEC_PROTOCOL_PAYLOAD*)&IrPayload.payload;
	
	if( ir_payload.info.bits.repeat_type == IR_PROTOCOL_REPEAT_NULL ) 
	{
		srv_parms = IRCmdHandler_ProgramCode_UserEventHandle( ir_payload );
		if ( srv_parms.event_id == xHMI_EVENT_NULL )
		{
			srv_parms = IRCmdHandler_UserEventHandle( *pNecProtocol );
		}

		if ( srv_parms.event_id == xHMI_EVENT_NULL )
		{
			srv_parms = IRCmdHandler_TriggerFactoryModeEventHandle( *pNecProtocol );
		}

        srv_parms.event = IRCmdHandler_VizioSpecialUserEvent(FALSE,srv_parms.event);
	}
	else if ( ir_payload.info.bits.repeat_type == IR_PROTOCOL_38K_NEC_REPEAT )
	{
		srv_parms = IRCmdHandler_ProgramCode_UserEventHandle( ir_payload );
		if ( srv_parms.event_id == xHMI_EVENT_NULL )
		{
			srv_parms = IRCmdHandler_UserEventHandle( *pNecProtocol  );
		}
        srv_parms.event = IRCmdHandler_VizioSpecialUserEvent(TRUE,srv_parms.event);
		srv_parms.event = IRCmdHandler_AdmitRepeatUserEvent( srv_parms.event );
	}
	
	return (  srv_parms );
		
}

static xHMISrvEventParams IRCmdHandler_Program_CmdEvent( IR_PROTOCOL_PAYLOAD ir_payload )
{
	xHMISrvEventParams srv_parms = {xHMI_EVENT_NULL, USER_EVENT_UNDEFINED };

	if ( ir_payload.info.bits.repeat_type != IR_PROTOCOL_REPEAT_NULL )
	{
		srv_parms = IRCmdHandler_ProgramCode_UserEventHandle( ir_payload );
		srv_parms.event = IRCmdHandler_AdmitRepeatUserEvent( srv_parms.event );
	}
	else
	{
		srv_parms = IRCmdHandler_ProgramCode_UserEventHandle( ir_payload );
	}

	return (  srv_parms );
}

static void IRCmdHandler_ReloadProgramRemoteEvent( void )
{
	uint8 i = 0;
	IR_PRG_REMOTE_USER_EVENT event;

	for( i = 0; i < 3; i++ )
	{
		switch( i )
		{
			case 0:
			{
#if ( configSTM32F411_PORTING == 1 )
				pSDM_ObjCtrl->read( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_UP, &event );
#endif				
			}
				break;

			case 1:
			{
#if ( configSTM32F411_PORTING == 1 )
				pSDM_ObjCtrl->read( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_DN, &event );
#endif				
			}
				break;

			case 2:
			{
#if ( configSTM32F411_PORTING == 1 )
				pSDM_ObjCtrl->read( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_MUTE, &event );
#endif				
			}
				break;

		}

		if ( mIrPrgRemoteUserEvent[i].user_event == event.user_event )
		{
			mIrPrgRemoteUserEvent[i].ir_payload.info.byte = event.ir_payload.info.byte;
			mIrPrgRemoteUserEvent[i].ir_payload.payload = event.ir_payload.payload;
		}
		
	}
}

static void IRCmdHandler_ProgramRemoteEvent( xHMIUserEvents event )
{
	switch( event )
	{
		case USER_EVENT_PROGRAM_IR_VOL_UP:
		{
			mIrState.index = 0;
			mIrState.ir_mode = IR_MODE_PROGRAM;
			mIrState.ir_prg_state = IR_PRG_NULL;
            //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_PRG_REMOTE_LEARNING);

			TRACE_DEBUG((0, "programe USER_EVENT_PROGRAM_IR_VOL_UP "));
		}
			break;
			
		case USER_EVENT_PROGRAM_IR_VOL_DN:
		{
			mIrState.index = 1;
			mIrState.ir_mode = IR_MODE_PROGRAM;
			mIrState.ir_prg_state = IR_PRG_NULL;
            //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_PRG_REMOTE_LEARNING);

			TRACE_DEBUG((0, "programe USER_EVENT_PROGRAM_IR_VOL_DN "));
		}
			break;
			
		case USER_EVENT_PROGRAM_IR_MUTE:
		{
			mIrState.index = 2;
			mIrState.ir_mode = IR_MODE_PROGRAM;
			mIrState.ir_prg_state = IR_PRG_NULL;
            //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_PRG_REMOTE_LEARNING);

			TRACE_DEBUG((0, "programe USER_EVENT_PROGRAM_IR_MUTE "));
		}
			break;

		case USER_EVENT_DEVICE_RESET:
		{
			IRCmdHandler_Initialize( );
		}
			break;

		default:
		{
			mIrState.index = 0;
			mIrState.ir_mode = IR_MODE_OPERATION;
			mIrState.ir_prg_state = IR_PRG_NULL;
		}
			break;
	}
}

static void IRCmdHanlder_ProgramRemote_BuildingUserEvent( IR_PROTOCOL_PAYLOAD ir_payload )
{
	NEC_PROTOCOL_PAYLOAD *pNecProtocol;
	uint16 ir_address = 0;

#if IR_BUILD_DEBUG
	uint8 i = 0;
	uint8 *pBYTE;
#endif 
	
	if ( ir_payload.info.bits.repeat_type == IR_PROTOCOL_38K_NEC_REPEAT )
	{
		mIrState.ir_prg_state = IR_PRG_FAILURE;
		mIrState.ir_mode = IR_MODE_OPERATION;
		TRACE_DEBUG((0, "1 program ir is failure!!"));
        //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_NORMAL);
	}
	else if ( ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_NEC )
	{
		pNecProtocol = (NEC_PROTOCOL_PAYLOAD*)&ir_payload.payload;
		ir_address = ( ( pNecProtocol->address << 8 ) | pNecProtocol->int_address );

#if IR_BUILD_DEBUG
		TRACE_DEBUG((0, "ir address = 0x%X", ir_address ));
#endif 

		if ( ir_address == XRS351_ADDRESS )
		{
			mIrState.ir_prg_state = IR_PRG_FAILURE;
			mIrState.ir_mode = IR_MODE_OPERATION;
			TRACE_DEBUG((0, "2 program ir is failure!!"));
            //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_NORMAL);
			return;
		}
	}

	
	mIrPrgRemoteUserEvent[mIrState.index].ir_payload.info.byte = ir_payload.info.byte;
	mIrPrgRemoteUserEvent[mIrState.index].ir_payload.payload = ir_payload.payload;
	mIrState.ir_prg_state = IR_PRG_SUCCESS;

	switch( mIrPrgRemoteUserEvent[mIrState.index].user_event )
	{
		case USER_EVENT_VOL_UP:
		{
#if ( configSTM32F411_PORTING == 1 )
			pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_UP ,(uint8*)&mIrPrgRemoteUserEvent[mIrState.index] );
#endif			
			TRACE_DEBUG((0, "USER_EVENT_VOL_UP program success"));
            //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_PRG_REMOTE_LEARNED);
		}
			break;

		case USER_EVENT_VOL_DN:
		{
#if ( configSTM32F411_PORTING == 1 )
			pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_DN ,(uint8*)&mIrPrgRemoteUserEvent[mIrState.index] );
#endif			
			TRACE_DEBUG((0, "USER_EVENT_VOL_DN program success"));
            //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_PRG_REMOTE_LEARNED);
		}
			break;
			
		case USER_EVENT_MUTE:
		{
#if ( configSTM32F411_PORTING == 1 )
			pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_MUTE,(uint8*)&mIrPrgRemoteUserEvent[mIrState.index] );
#endif			
			TRACE_DEBUG((0, "USER_EVENT_MUTE program success"));
            //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_PRG_REMOTE_LEARNED);
		}
			break;
	}

#if IR_BUILD_DEBUG 
	pBYTE = (uint8*)&mIrPrgRemoteUserEvent[mIrState.index];

	for ( i = 0; i < (sizeof(mIrPrgRemoteUserEvent[mIrState.index])/sizeof ( uint8 )); i ++ )
	{
		TRACE_DEBUG((0, "data[%d] = 0x%X", i, *(pBYTE+i)));
	

	}
#endif 
}

static void IRCmdHandler_ServiceHandle(void *pvParameters)
{
	static IR_PROTOCOL_PAYLOAD ir_payload;
	xHMISrvEventParams srv_parms = { xHMI_EVENT_NULL, USER_EVENT_UNDEFINED };
	

	for( ;; )
	{
		if ( IRCmdHandler_getQueueNumber() > 0 )
		{
			IRCmdHandler_getQueue( &ir_payload );

			switch( mIrState.ir_mode )
			{
				case IR_MODE_PROGRAM:
				{
					IRCmdHanlder_ProgramRemote_BuildingUserEvent( ir_payload );
					mIrState.ir_mode = IR_MODE_OPERATION;
					mIrState.ir_prg_state = IR_PRG_NULL;
				}
					break;

				case IR_MODE_OPERATION:
				{
					if ( ir_payload.info.bits.protocol_type == IR_PROTOCOL_38K_NEC )
					{
						srv_parms = IRCmdHandler_NEC_CmdEvent( ir_payload );
					}
					else
					{
						srv_parms = IRCmdHandler_Program_CmdEvent( ir_payload );
					}

#if ( configSTM32F411_PORTING == 0 )
					pHS_ObjCtrl->SendEvent( &srv_parms ); 
#endif					
				}
					break;

			}
			
		}
		else
		{
			if ( mIrState.ir_mode == IR_MODE_RELOAD_PRG_KEY )
			{
				IRCmdHandler_ReloadProgramRemoteEvent( );
				mIrState.ir_mode = IR_MODE_OPERATION;
			}
		}

		vTaskDelay( TASK_MSEC2TICKS(10) );
	}

}

static void IRCmdHandler_Initialize( void )
{
	uint8 i = 0;
	
	for ( i = 0; i < 3; i++ )
	{
		mIrPrgRemoteUserEvent[i].ir_payload.info.byte = 0;
		mIrPrgRemoteUserEvent[i].ir_payload.payload = 0xffffffff;

		switch( mIrPrgRemoteUserEvent[i].user_event )
		{
			case USER_EVENT_VOL_UP:
			{
#if ( configSTM32F411_PORTING == 1 )
				pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_UP ,(uint8*)&mIrPrgRemoteUserEvent[i] );
#endif				
			}
				break;

			case USER_EVENT_VOL_DN:
			{
#if ( configSTM32F411_PORTING == 1 )
				pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_DN ,(uint8*)&mIrPrgRemoteUserEvent[i] );
#endif				
			}
				break;
				
			case USER_EVENT_MUTE:
			{
#if ( configSTM32F411_PORTING == 1 )
				pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_MUTE,(uint8*)&mIrPrgRemoteUserEvent[i] );
#endif				
			}
				break;
		}
		
	}

	mIrState.ir_mode = IR_MODE_RELOAD_PRG_KEY;
	mIrState.ir_prg_state = IR_PRG_NULL;
	mIrState.index = 0;
}

static void IRCmdHandler_CreateTask(void)
{
#if ( configAPP_IR_REMOTE == 1 )
    if ( xTaskCreate( IRCmdHandler_ServiceHandle,
        ( portCHAR * ) "IRCmdHandler",
         configMINIMAL_STACK_SIZE, NULL,
         tskHMI_EVENT_PRIORITY, &xOS_IrCmd.IrCmdTaskHandle ) != pdPASS)
    {
		vTaskDelete( xOS_IrCmd.IrCmdTaskHandle );
		xOS_IrCmd.xOS_ErrId = xOS_TASK_CREATE_FAIL;
    }

	xOS_IrCmd.queue.xQueue = xQueueCreate( 1, FRTOS_SIZE(IR_PROTOCOL_PAYLOAD) );
	xOS_IrCmd.queue.xBlockTime = BLOCK_TIME(0);
    if( xOS_IrCmd.queue.xQueue == NULL )
    {
       xOS_IrCmd.xOS_ErrId = xOS_TASK_QUEUE_CREATE_FAIL;
    }
#endif 	
}

