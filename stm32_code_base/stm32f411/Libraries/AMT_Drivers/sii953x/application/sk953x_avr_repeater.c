//***************************************************************************
//!file     sk9535_avr_repeater.c
//!brief    SiI9535 Starter Kit firmware main module.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#if INC_CBUS
#include "sk_app_cbus.h"
#endif
#include "si_debugger_hdmigear.h"
#include "si_eeprom.h"
#include "si_gpio.h"
#if INC_RTPI
#include "si_rtpi_component.h"
#endif

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#include "GPIOMiddleLevel.h"
#include "HdmiDeviceManager.h"

#if INC_ARC
#include "sk_app_arc.h"
#endif

#if INC_CEC_SAC
#include "si_sac_main.h"
#endif

#endif

//------------------------------------------------------------------------------
#define HDMI_REPEATER_TIME_TICK TASK_MSEC2TICKS(50) /*Smith todo: it could be 100 msec*/
#define HDMI_RST_TIME_TICK TASK_MSEC2TICKS(5000)
//------------------------------------------------------------------------------

typedef struct HDMI_MANAGER_VOL_PARAMETERS 
{
    xTaskHandle hdmi_avr_repeater_task_handle;
    TaskHandleState hdmi_avr_repeater_state;
	xTaskHandle hdmi_service_handle;
	TaskHandleState ServiceState;
	xQueueParameters QParams;
    xOS_TaskErrIndicator xOS_ErrId;
} xOS_HDMI_Params;


//------------------------------------------------------------------------------
//  Application Data
//------------------------------------------------------------------------------
#if ( configSII_DEV_9535 == 1 )
const char lSignonMsg [] = "SK9535 Firmware v";
#else
const char lSignonMsg [] = "SK9533 Firmware v";
#endif

const char lCopyrightMsg [] = "Copyright Silicon Image Inc, 2010-2013";
extern char *buildTime;     // From buildtime.c
extern char *buildVersion;  // From buildtime.c

AppInstanceData_t app;
AppInstanceData_t *pApp = &app;

xOS_HDMI_Params xOS_HDMI_Parms;

#if ( configSII_DEV_953x_PORTING == 1 )
uint8_t SYS_CEC_ARC_APP_TASK = 0x00;
uint16_t SYS_CEC_SAC_APP_TASK = 0x00;
#endif

extern uint16_t SiiDrvDeviceIdGet(void);
extern uint8_t SiiDrvDeviceRevGet(void);

//------------------------------------------------------------------------------
static void HdmiDeviceManager_CreateTask( void );

static void HdmiManager_DeviceEnable( bool enable );

static bool HdmiDeviceManager_InstrSender( void *params ) ;

static bool HdmiDeviceManager_GetMultiChannel(void);

xHMI_SiiAppPowerStatus_t HdmiDeviceManager_GetHDMIPowerStatus(void);

void AmTArcAppTaskAssign( AmTArcTaskEvent event);

void AmTSacAppTaskAssign(AmTSacTaskEvent sac_event);

const HDMI_DEVICE_MANAGER_OBJECT HdmiDeviceManager =
{
    HdmiDeviceManager_CreateTask,
    HdmiDeviceManager_InstrSender,
    HdmiDeviceManager_GetMultiChannel,
    HdmiDeviceManager_GetHDMIPowerStatus,
};
const HDMI_DEVICE_MANAGER_OBJECT *pHDMI_DM_ObjCtrl = &HdmiDeviceManager;


//------------------------------------------------------------------------------
// Function:    SkAppSignon
// Description: Display signon and configuration information
//-----------------------------------------------------------------------------c
void SkAppSignon ( void )
{

#if ( configSII_DEV_953x_PORTING == 0 )
    DEBUG_PRINT( MSG_ALWAYS, "\n\n%s%s\n%s\n", lSignonMsg, buildVersion, lCopyrightMsg );
    PrintAlways( buildTime );   // From buildtime.c
    DEBUG_PRINT( MSG_ALWAYS, "\nChipdID:0x%x\tChip Rev:0x%x\n", SiiDrvDeviceIdGet(), SiiDrvDeviceRevGet() );
#else
    TRACE_DEBUG(( 0, "%s %s %s", lSignonMsg, buildVersion, lCopyrightMsg ));
    TRACE_DEBUG((0, " 0x%X, 0x%X", SiiDrvDeviceIdGet(), SiiDrvDeviceRevGet() ));
#endif 

}

//------------------------------------------------------------------------------
// Function:    main
// Description: Main entry point for SK9535 Starter Kit firmware
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
#if ( configSII_DEV_953x_PORTING == 0 )
int main(void)
{
    if ( !SiiPlatformInitialize() )
    {
        SkAppBlinkTilReset( ERRLED_BAD_INIT );
    }
    SkAppInitialize();
    SkAppSignon();

#if ( SII_POWER_TYPE == APP_POWERTYPE_FULL )
    // In this power type, power is always on to the entire chip
    SiiPlatformGpioSetPower(SII_GPIO_POWER_ALL, ON );
#endif

#if ( INC_CBUS_WAKEUP )
    SkAppDeviceInitCbusWakeup();
#endif

#if ( SII_POWER_TYPE != APP_POWERTYPE_MAIN )
    SkAppDeviceInitialize();    // Initialize anything that can be done with PWD and/or AON power on
#ifndef POWER_UP_ONLY    
    SkAppLowPowerStandby();     // Place most of the chip into low power standby mode
#if INC_CEC
    SkAppCecResume(false);      // Resume the CEC component so it can handle wake up messages
#endif
#endif // #ifndef POWER_UP_ONLY
#endif

    for ( ;; )  // Main loop
	{
        // Only check the debugger if power is on.
#if ( SII_POWER_TYPE != APP_POWERTYPE_FULL )
        if ( app.powerState == APP_POWERSTATUS_ON)
#endif
        {
            if ( SkRemoteRequestHandler() )
            {
                continue;
            }
        }

        //---------------------------------------------------------------------
        // The code in the following tasks can modify the power state of the
        // system, so they must be run in any power state.  They do not
        // access the SiI9535 I2C registers in case the system is designed
        // to not provide standby power to the SiI9535.
        //---------------------------------------------------------------------
        SkAppTaskCbusStandByMonitoring();
        SkAppDevicePower();
#ifndef POWER_UP_ONLY
        SkAppTaskIrRemote( app.powerState == APP_POWERSTATUS_ON );
#if INC_CBUS && INC_CBUS_WAKEUP

        SkAppCbusWakeupSequence(app.powerState == APP_POWERSTATUS_ON);
#endif
        //---------------------------------------------------------------------
        // From this point on it is assumed that the SiI9535 Always On power
        // domain has power, but not necessarily the power down domain.
        //
        // Service the Silicon Image device interrupts.  If the system is in
        // standby, a reduced interrupt check is performed only on those
        // systems that are capable of waking up the system.
        //---------------------------------------------------------------------
        SkAppTaskSiiDevice();
#if INC_CEC
        SkAppTaskCec();
#endif
        if ( app.powerState != APP_POWERSTATUS_ON )
        {
            continue;
    	}

        //---------------------------------------------------------------------
        // From this point on it is assumed that the SiI9535 is at full power.
        //---------------------------------------------------------------------
#if INC_CBUS
       	SkAppTaskCbus();
#endif
      	SkAppTaskSwitch();
        SkAppTaskRepeater();
        SkAppTaskTx();
#if INC_IPV
        SkAppTaskIpv();
#endif

#if INC_OSD
		SkAppTaskOsd();
#endif

		SkAppTaskAudio();


#if INC_RTPI
		SiiRtpiProcessRtpi();
#endif

        // Run the appropriate UI
        switch ( app.modeSelect )
        {
            case SK_MODE_TV:
                SkAppTv();
                break;
            case SK_MODE_DEMO:
                SkAppDemo();
                break;
            case SK_MODE_SETUP:
                SkAppSetup();
                break;
#if INC_BENCH_TEST
            case SK_MODE_BTST:
                SkAppBenchTest();
                break;
#endif

            default:
                break;
        }

        if( pApp->isThxDemo )
        {
        	SkAppTaskThx();
        }

        SkAppSwitchPortUpdate();    // Check for port updates from any of the tasks.
#endif // #ifndef POWER_UP_ONLY
    }


    return( 0 );
}
#endif /*( configSII_DEV_953x_PORTING == 0 )*/ 


static void sii953x_repeater_task( void )
{
    //---------------------------------------------------------------------
    // The code in the following tasks can modify the power state of the
    // system, so they must be run in any power state.	They do not
    // access the SiI9535 I2C registers in case the system is designed
    // to not provide standby power to the SiI9535.
    //---------------------------------------------------------------------

#if ( configSII_DEV_953x_PORTING_CBUS == 1 )	 
    SkAppTaskCbusStandByMonitoring();
#endif
    SkAppDevicePower();

#if ( configSII_DEV_953x_PORTING_CBUS == 1 )	 
#if INC_CBUS && INC_CBUS_WAKEUP
    SkAppCbusWakeupSequence(app.powerState == APP_POWERSTATUS_ON);
#endif
#endif

    //---------------------------------------------------------------------
    // From this point on it is assumed that the SiI9535 Always On power
    // domain has power, but not necessarily the power down domain.
    //
    // Service the Silicon Image device interrupts.  If the system is in
    // standby, a reduced interrupt check is performed only on those
    // systems that are capable of waking up the system.
    //---------------------------------------------------------------------
    SkAppTaskSiiDevice();

#if INC_CEC
    SkAppTaskCec();
#endif

	if ( app.powerState == APP_POWERSTATUS_ON )
	{
	
#if ( configSII_DEV_953x_PORTING == 1 )	 
    AmTArcTaskAssign(SYS_CEC_ARC_APP_TASK);	//Assign task event to ARC after powered on sil953x
    SYS_CEC_ARC_APP_TASK = 0;	//Reset app task event
	
    AmTSacTaskAssign(SYS_CEC_SAC_APP_TASK);	//Assign task event to SAC after powered on sil953x
    SYS_CEC_SAC_APP_TASK = 0;	//Reset app task event
#endif
		//---------------------------------------------------------------------
		// From this point on it is assumed that the SiI9535 is at full power.
		//---------------------------------------------------------------------
#if ( configSII_DEV_953x_PORTING_CBUS == 1 )	 
#if INC_CBUS
		SkAppTaskCbus();
#endif
#endif
        SkAppTaskSwitch();
        SkAppTaskRepeater();
        SkAppTaskTx();
        
#if INC_IPV
		SkAppTaskIpv();
#endif

#if INC_OSD
		SkAppTaskOsd();
#endif
		SkAppTaskAudio();
		
#if INC_RTPI
		SiiRtpiProcessRtpi();
#endif

		switch ( app.modeSelect )
		{
			case SK_MODE_TV:
				SkAppTv();
				break;
			case SK_MODE_DEMO:
				SkAppDemo();
				break;
			case SK_MODE_SETUP:
				SkAppSetup();
				break;
#if INC_BENCH_TEST
			case SK_MODE_BTST:
				SkAppBenchTest();
				break;
#endif

			default:
				break;
		}

		if( pApp->isThxDemo )
		{
			SkAppTaskThx();
		}

		SkAppSwitchPortUpdate();	// Check for port updates from any of the tasks.

	}

}

static void HdmiDeviceManager_RstDevice( void )
{
	GPIOMiddleLevel_Clr(__O_RST_HDMI_SIL953x );
	vTaskDelay(1);
	GPIOMiddleLevel_Set(__O_RST_HDMI_SIL953x );
	vTaskDelay(1);
}

static bool_t HdmiDeviceManager_Initializie( void )
{
	bool_t ret = true;
	
    ret = SkAppInitialize();
    SkAppSignon();

#if ( configSII_DEV_953x_PORTING_CBUS == 1 )
#if ( INC_CBUS_WAKEUP )
    ret = SkAppDeviceInitCbusWakeup();
#endif
#endif

	ret= SkAppDeviceInitialize(); // Initialize anything that can be done with PWD and/or AON power on
    SkAppLowPowerStandby();     // Place most of the chip into low power standby mode
    
#if INC_CEC
    SkAppCecResume(false);      // Resume the CEC component so it can handle wake up messages
#endif

	return ( ret );

}

//_______________________________AMTRAN IMPLEMENTED_____________________________________________
static void HdmiManager_DeviceEnable( bool enable )
{
	if ( enable == TRUE )
	{
		if ( app.powerState == APP_POWERSTATUS_STANDBY )
		{
			app.powerState = APP_POWERSTATUS_STANDBY_TO_ON;
			TRACE_DEBUG((0, "sil9533 powered up !! "));
		}
	}
	else if ( enable == FALSE )
	{
		if ( app.powerState == APP_POWERSTATUS_ON )
		{
			app.powerState = APP_POWERSTATUS_ON_TO_STANDBY;
			TRACE_DEBUG((0, "sil9533 powered down !! "));
		}	
	}
}

static void HdmiDeviceManager_handleCommand( xHMISystemParams params  )
{
	switch( params.sys_event )
	{
            case SYS_EVENT_SYSTEM_UP:
            case SYS_EVENT_CEC_SYSTEM_UP:
            case SYS_EVENT_SAC_SYSTEM_UP:
            case SYS_EVENT_ARC_SYSTEM_UP:
		{
			if ( params.input_src == AUDIO_SOURCE_HDMI_ARC )
			{
                                HdmiManager_DeviceEnable( TRUE );
								
                                #if INC_ARC
				AmTArcAppTaskAssign(ARC_TASK_INIT_OUTRIGHT);
                                #endif	
								
                                #if INC_CEC_SAC
		                AmTSacAppTaskAssign(SAC_TASK_INITIATE_DIRECTLY);
                                #endif	
			}
			else if ( params.input_src == AUDIO_SOURCE_HDMI_IN )
			{
                                HdmiManager_DeviceEnable( TRUE );
								
                                #if INC_ARC
				AmTArcAppTaskAssign(ARC_TASK_TERM_PRIVATELY);	//terminate ARC but keep flag enable		
                                #endif	
								
                                #if INC_CEC_SAC
		                AmTSacAppTaskAssign(SAC_TASK_INITIATE_POWER_ON);
                                #endif	
			}
			/*else
			{
#if INC_ARC
                        AmTArcTaskAssign(ARC_TASK_TERM_PRIVATELY);	//terminate ARC but keep flag enable
#endif								
			}*/
                }
			break;

            case SYS_EVENT_SYSTEM_DOWN:
		{
                    //HdmiManager_DeviceEnable( FALSE );
					
#if INC_CEC_SAC
                    //Need use this api to pass event when HDMI is off
                    AmTSacTaskAssign(CEC_SAC_TASK_TERMINATE);
#endif

#if INC_ARC
                    //Need use this api to pass event when HDMI is off
                    AmTArcTaskAssign(SYS_CEC_TASK_ARC_TERMINATE);	//terminate ARC but keep flag enable		
#endif
                    vTaskDelay(200);
                    HdmiManager_DeviceEnable( FALSE );

                }
			break;

		case SYS_EVENT_SEL_SRC:
		{
			TRACE_DEBUG((0, "port select = %d", params.input_src ));

			if ( params.input_src == AUDIO_SOURCE_HDMI_ARC )
			{
                             HdmiManager_DeviceEnable( TRUE );
#if INC_ARC
                            AmTArcAppTaskAssign(ARC_TASK_INIT_OUTRIGHT);
#endif
#if INC_CEC_SAC
 	                    AmTSacAppTaskAssign(SAC_TASK_INITIATE_DIRECTLY);
#endif	
			}
			else if ( params.input_src == AUDIO_SOURCE_HDMI_IN )
			{
                            HdmiManager_DeviceEnable( TRUE );
#if INC_ARC
                            AmTArcAppTaskAssign(ARC_TASK_TERM_PRIVATELY);	//terminate ARC but keep flag enable		
#endif
#if INC_CEC_SAC
		            AmTSacAppTaskAssign(SAC_TASK_INITIATE_POWER_ON);
#endif	
			}
			else
			{
                            HdmiManager_DeviceEnable( FALSE );
#if INC_ARC
                            //Need use this api to pass event when HDMI is off
                            AmTArcTaskAssign(SYS_CEC_TASK_ARC_TERMINATE);	//terminate ARC but keep flag enable
#endif

#if INC_CEC_SAC
                            //Need use this api to pass event when HDMI is off
                            AmTSacTaskAssign(CEC_SAC_TASK_TERMINATE);
#endif
			}
		}
			break;

	}
}

static bool HdmiDeviceManager_InstrSender( void *params ) 
{
   xHMISystemParams* pInstr = ( xHMISystemParams *)params;

    if ( pInstr == NULL )
    {
        TRACE_ERROR((0, "HdmiDeviceManager_InstrSender parameters error !! "));
        return FALSE;
    }
    
    if (xOS_HDMI_Parms.QParams.xQueue == NULL )
    {
        TRACE_ERROR((0, "HdmiDeviceManager_InstrSender xqueue is null !! "));
        return FALSE;
    }
    
    if ( xQueueSend( xOS_HDMI_Parms.QParams.xQueue, pInstr, xOS_HDMI_Parms.QParams.xBlockTime ) != pdPASS )
	{
		TRACE_ERROR((0, "HdmiDeviceManager_InstrSender sends queue failure "));
		return FALSE;
	}
    return TRUE;

}

static uint8 HdmiDeviceManager_GetQueueNumber( void )
{
	return (uint8)uxQueueMessagesWaiting(xOS_HDMI_Parms.QParams.xQueue);
}

static bool HdmiDeviceManager_InstrReceiver( xHMISystemParams *pInstr ) 
{
    if ( pInstr == NULL )
    {
        TRACE_ERROR((0, " HdmiDeviceManager_InstrReceiver instruction recevier error !! "));
    }
    
	if ( xOS_HDMI_Parms.QParams.xQueue == NULL )
	{
        TRACE_ERROR((0, "HdmiDeviceManager_InstrReceiver queue is null !! "));
        return FALSE;
	}

	if ( xQueueReceive( xOS_HDMI_Parms.QParams.xQueue, pInstr, xOS_HDMI_Parms.QParams.xBlockTime ) != pdPASS )
	{
		TRACE_ERROR((0, " HdmiDeviceManager_InstrReceiver receiver queue is failure "));
		return FALSE;
	}

	return TRUE;
}

static void HdmiManager_RepeaterTask( void *pvParameters )
{
    for ( ;; )
    {
		switch( xOS_HDMI_Parms.hdmi_avr_repeater_state )
		{
			case TASK_SUSPENDED:
				break;
			
			case TASK_READY:
			{
				HdmiDeviceManager_RstDevice( );
				if( HdmiDeviceManager_Initializie( ) == true )
				{
					xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_RUNING;
				}
				else
				{
					TRACE_ERROR((0, "SIL9533 cannot be connected !! "));
					xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_BLOCKED;
				}
			}
				break;

			case TASK_RUNING:
			{
				sii953x_repeater_task( );
			}
				break;

			case TASK_BLOCKED:
			{

			}
				break;

		}
		
        vTaskDelay( HDMI_REPEATER_TIME_TICK );
    }
}


static void HdmiDeviceManager_ServiceHandle( void *pvParameters )
{
	xHMISystemParams systemParams;
	
	for( ;; )
	{
		switch( xOS_HDMI_Parms.ServiceState )
		{
			case TASK_SUSPENDED:
			{
                if( ( HdmiDeviceManager_GetQueueNumber() > 0 )  && (xOS_HDMI_Parms.hdmi_avr_repeater_state == TASK_RUNING) )
                {
                    xOS_HDMI_Parms.ServiceState = TASK_READY;
                }
			}
				break;
		
			case TASK_READY:
			{
                if ( HdmiDeviceManager_InstrReceiver( (xHMISystemParams*)&systemParams ) == TRUE )
				{
					xOS_HDMI_Parms.ServiceState = TASK_RUNING;
				}
				else
				{
					xOS_HDMI_Parms.ServiceState = TASK_SUSPENDED;
				}
				
			}
				break;

			case TASK_RUNING:
			{
				HdmiDeviceManager_handleCommand( systemParams );
				xOS_HDMI_Parms.ServiceState = TASK_BLOCKED;
			}
				break;

			case TASK_BLOCKED:
			{
                xOS_HDMI_Parms.ServiceState = TASK_SUSPENDED;
			}
				break;	
		}

		vTaskDelay( TASK_MSEC2TICKS(1) );
	}

}

static void HdmiDeviceManager_CreateTask( void )
{
	xOS_HDMI_Parms.QParams.xQueue = xQueueCreate( 5, FRTOS_SIZE(xHMISystemParams) );
	xOS_HDMI_Parms.QParams.xBlockTime = BLOCK_TIME(0);
	if( xOS_HDMI_Parms.QParams.xQueue == NULL )
	{
	}

    if ( xTaskCreate( HdmiManager_RepeaterTask, 
            ( portCHAR * ) "SII9535_ENTRY", 
            (STACK_SIZE*2), NULL, tskSII9535_PRIORITY,&xOS_HDMI_Parms.hdmi_avr_repeater_task_handle) != pdPASS )
    {
        vTaskDelete( xOS_HDMI_Parms.hdmi_avr_repeater_task_handle );
        xOS_HDMI_Parms.xOS_ErrId = xOS_TASK_CREATE_FAIL;
		xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_SUSPENDED;
    }
    else
    {
		SiiPlatformInitialize();
        //vTaskSuspend( xOS_HDMI_Parms.hdmi_avr_repeater_task_handle );
        xOS_HDMI_Parms.xOS_ErrId = xOS_TASK_ERR_NULL;
		xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_READY;
    }

	if ( xTaskCreate( 
		HdmiDeviceManager_ServiceHandle, 
		( portCHAR * ) "HDMI_SRV", 
		(STACK_SIZE), 
		NULL, 
		tskHDMI_SERVICE_PRIORITY, 
		&xOS_HDMI_Parms.hdmi_service_handle ) != pdPASS )
	{
		xOS_HDMI_Parms.xOS_ErrId = xOS_TASK_CREATE_FAIL;
		xOS_HDMI_Parms.ServiceState = TASK_SUSPENDED;
		vTaskDelete( xOS_HDMI_Parms.hdmi_service_handle );
	}
    
}

bool HdmiDeviceManager_GetMultiChannel(void)
{
    return AmTAudiogetMultiChannel();
}

xHMI_SiiAppPowerStatus_t HdmiDeviceManager_GetHDMIPowerStatus(void)
{
    return (xHMI_SiiAppPowerStatus_t)app.powerState;
}


//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
void AmTArcAppTaskAssign( AmTArcTaskEvent event)
{
	xHMISystemParams SysPrams;

	SysPrams = AmTCecGetSyspramsFromHMI();
	
	switch (event)
	{
		case ARC_TASK_INIT_OUTRIGHT:
		{			
	                AmTCecEventPassToHMI(USER_EVENT_CEC_SET_ARC_ON);
	                SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_INITIONAL;
		}
			break;
			
		case ARC_TASK_INIT_PRIVATELY:
		{
			if ( SysPrams.cec_arc == TRUE )
			{
	                        SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_INITIONAL;
			}
		}
			break;

		case ARC_TASK_INIT_REQUESTED:
		{			
/*			if ( !AmTIPowerOnSystem() )	//if need power on system, then no need to set task here
			{
	                        AmTCecEventPassToHMI(USER_EVENT_CEC_SET_ARC_ON);
	                        SYS_CEC_TASK |= SYS_CEC_TASK_ARC_INITIONAL;
			}
			if ( SysPrams.input_src != AUDIO_SOURCE_HDMI_ARC )
			    AmTCecEventPassToHMI(USER_EVENT_SEL_HDMI_ARC);
*/
			AmTCecEventPassToHMI(USER_EVENT_CEC_SET_ARC_ON);
			AmTCecEventPassToHMI(USER_EVENT_ARC_POWER_ON);
	                SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_INITIONAL;
		}
			break;

		case ARC_TASK_TERM_REQUESTED:
		{
			if ( SysPrams.cec_arc == TRUE )
			{
	                        AmTCecEventPassToHMI(USER_EVENT_CEC_SET_ARC_OFF);
	                        SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_TERMINATE;
			}
		}
			break;

		case ARC_TASK_TERM_PRIVATELY:
		{
			if ( SysPrams.cec_arc == TRUE )
			{
	                        SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_TERMINATE;
			}
		}
			break;

		case ARC_TASK_REPORT_INIT:
		{
	                SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_REPORT_INITIONAL;
		}
			break;

		case ARC_TASK_REPORT_TERM:
		{
	                SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_REPORT_TERMINATE;
		}
			break;

		default:
			break;
	}
    //TRACE_DEBUG((0, "SYS_CEC_ARC_APP_TASK %d", SYS_CEC_ARC_APP_TASK ));
}


//-------------------------------------------------------------------------------------------------
//! @brief    
//-------------------------------------------------------------------------------------------------
void AmTSacAppTaskAssign(AmTSacTaskEvent sac_event)
{		
        switch ( sac_event )
	{
	        case SAC_TASK_TERMINATE_POWER_OFF:
		{
		    SYS_CEC_SAC_APP_TASK |= CEC_SAC_TASK_TERMINATE;
	        }
			break;

		case SAC_TASK_INITIATE_POWER_ON:
		{
		    SYS_CEC_SAC_APP_TASK |= CEC_SAC_TASK_INITIATE;
	        }
		        break;

		case SAC_TASK_INITIATE_WAIT_TV_RESPONSE:
		{
                    SYS_CEC_SAC_APP_TASK |= CEC_SAC_TASK_WAIT_TV_RESPONSE;
	        }
		        break;

		case SAC_TASK_INITIATE_DIRECTLY:
		{
                    SYS_CEC_SAC_APP_TASK |= CEC_SAC_TASK_INITIATE_DIRECTLY;
		}
		        break;

		default:
		{
		        break;
		}
        }
}

