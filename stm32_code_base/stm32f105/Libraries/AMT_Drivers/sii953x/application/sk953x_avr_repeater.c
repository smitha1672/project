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
#include "PowerHandler.h"
#include "VirtualTimer.h"
#if INC_ARC
#include "sk_app_arc.h"
#endif

#if INC_CEC_SAC
#include "si_sac_main.h"
#endif

#endif

//------------------------------------------------------------------------------
#define HDMI_REPEATER_TIME_TICK TASK_MSEC2TICKS(25) /*Smith modifies: To avoid system crash*/
#define HDMI_RST_TIME_TICK TASK_MSEC2TICKS(5000)
//------------------------------------------------------------------------------

typedef struct HDMI_MANAGER_VOL_PARAMETERS
{
    xTaskHandle hdmi_avr_repeater_task_handle;
    TaskHandleState hdmi_avr_repeater_state;
    //xTaskHandle hdmi_service_handle;
    //TaskHandleState ServiceState;
    xQueueParameters QParams;
    xOS_TaskErrIndicator xOS_ErrId;
    uint8 power_handle;
    int8 watchdog_idx;
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
//static xSemaphoreHandle mSk953xMutex = NULL;

#if ( configSII_DEV_953x_PORTING == 1 )
uint16_t SYS_CEC_ARC_APP_TASK = 0x0000;
uint16_t SYS_CEC_SAC_APP_TASK = 0x0000;
uint16_t SYS_CEC_APP_TASK = 0x0000;

#if 0
typedef enum
{
    HDMI_POWER_INIT = 0,
    HDMI_POWER_TURN_ON = 1,
    HDMI_POWER_TURN_OFF = 2

} HDMI_POWER_STATUS;
#endif

typedef struct
{
    bool b_change_status;
    SiiAppPowerStatus_t new_pwr_status;
}HDMI_STANDBY_STATUS;

static HDMI_STANDBY_STATUS standby_status;

//static bool IS_HDMI_PWR_ON = FALSE;
//static HDMI_POWER_STATUS _hdmi_pwr_status = HDMI_POWER_INIT;
//xSemaphoreParameters HdmiDevMNGsema;

#endif

extern uint16_t SiiDrvDeviceIdGet ( void );
extern uint8_t SiiDrvDeviceRevGet ( void );

//------------------------------------------------------------------------------
static void HdmiDeviceManager_CreateTask ( void );

static bool HdmiManager_DeviceEnable ( bool enable );

static bool HdmiDeviceManager_InstrSender ( void *params ) ;

static bool HdmiDeviceManager_GetMultiChannel ( void );

xHMI_SiiAppPowerStatus_t HdmiDeviceManager_GetHDMIPowerStatus ( void );

#if ( configSII_DEV_953x_PORTING == 1 )
//static void HdmiDeviceManager_RestartTask ( bool Enable );

//static bool HdmiDeviceManager_Is_HDMI_Task_Ready ( void );

static void HdmiDeviceManager_RstDevice ( void );

#if 0
static bool HdmiDeviceManager_mutex_take ( void );

static bool HdmiDeviceManager_mutex_give ( void );
#endif

#endif

bool HdmiDeviceManager_IsHDMIPowerOff ( void );

void AmTArcAppTaskAssign ( AmTArcTaskEvent arc_event );

void AmTSacAppTaskAssign ( AmTSacTaskEvent sac_event );

void AmTCecAppTaskAssign ( AmTCecTaskEvent cec_event );

const HDMI_DEVICE_MANAGER_OBJECT HdmiDeviceManager =
{
    HdmiDeviceManager_CreateTask,
    HdmiDeviceManager_InstrSender,
    HdmiDeviceManager_GetMultiChannel,
#if ( configSII_DEV_953x_PORTING == 1 )
    HdmiDeviceManager_GetHDMIPowerStatus,
    HdmiDeviceManager_IsHDMIPowerOff,
    HdmiManager_DeviceEnable,

    //HdmiDeviceManager_Is_HDMI_Task_Ready,
    //HdmiDeviceManager_RestartTask,
    HdmiDeviceManager_RstDevice,
#endif
};
const HDMI_DEVICE_MANAGER_OBJECT *pHDMI_DM_ObjCtrl = &HdmiDeviceManager;

extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;


//------------------------------------------------------------------------------
// Function:    SkAppSignon
// Description: Display signon and configuration information
//-----------------------------------------------------------------------------c
void SkAppSignon ( void )
{

#if ( configSII_DEV_953x_PORTING == 0 )
    DEBUG_PRINT ( MSG_ALWAYS, "\n\n%s%s\n%s\n", lSignonMsg, buildVersion, lCopyrightMsg );
    PrintAlways ( buildTime );  // From buildtime.c
    DEBUG_PRINT ( MSG_ALWAYS, "\nChipdID:0x%x\tChip Rev:0x%x\n", SiiDrvDeviceIdGet(), SiiDrvDeviceRevGet() );
#else
    TRACE_DEBUG ( ( 0, "%s %s %s", lSignonMsg, buildVersion, lCopyrightMsg ) );
    TRACE_DEBUG ( ( 0, " 0x%X, 0x%X", SiiDrvDeviceIdGet(), SiiDrvDeviceRevGet() ) );
#endif

}

//------------------------------------------------------------------------------
// Function:    main
// Description: Main entry point for SK9535 Starter Kit firmware
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
#if ( configSII_DEV_953x_PORTING == 0 )
int main ( void )
{
    if ( !SiiPlatformInitialize() )
    {
        SkAppBlinkTilReset ( ERRLED_BAD_INIT );
    }

    SkAppInitialize();
    SkAppSignon();

#if ( SII_POWER_TYPE == APP_POWERTYPE_FULL )
    // In this power type, power is always on to the entire chip
    SiiPlatformGpioSetPower ( SII_GPIO_POWER_ALL, ON );
#endif

#if ( INC_CBUS_WAKEUP )
    SkAppDeviceInitCbusWakeup();
#endif

#if ( SII_POWER_TYPE != APP_POWERTYPE_MAIN )
    SkAppDeviceInitialize();    // Initialize anything that can be done with PWD and/or AON power on
#ifndef POWER_UP_ONLY
    SkAppLowPowerStandby();     // Place most of the chip into low power standby mode
#if INC_CEC
    SkAppCecResume ( false );   // Resume the CEC component so it can handle wake up messages
#endif
#endif // #ifndef POWER_UP_ONLY
#endif

    for ( ;; )  // Main loop
    {
        // Only check the debugger if power is on.
#if ( SII_POWER_TYPE != APP_POWERTYPE_FULL )
        if ( app.powerState == APP_POWERSTATUS_ON )
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
        SkAppTaskIrRemote ( app.powerState == APP_POWERSTATUS_ON );
#if INC_CBUS && INC_CBUS_WAKEUP

        SkAppCbusWakeupSequence ( app.powerState == APP_POWERSTATUS_ON );
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

        if ( pApp->isThxDemo )
        {
            SkAppTaskThx();
        }

        SkAppSwitchPortUpdate();    // Check for port updates from any of the tasks.
#endif // #ifndef POWER_UP_ONLY
    }


    return ( 0 );
}
#endif /*( configSII_DEV_953x_PORTING == 0 )*/


static void sii953x_repeater_task ( void )
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

    if ( standby_status.b_change_status == TRUE )
    {
        app.powerState = standby_status.new_pwr_status;
        standby_status.b_change_status = FALSE;
    }

    SkAppDevicePower();

#if ( configSII_DEV_953x_PORTING_CBUS == 1 )
#if INC_CBUS && INC_CBUS_WAKEUP
    SkAppCbusWakeupSequence ( app.powerState == APP_POWERSTATUS_ON );
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

        if ( pApp->isThxDemo )
        {
            SkAppTaskThx();
        }

        SkAppSwitchPortUpdate();	// Check for port updates from any of the tasks.
    }

// CEC, ARC, SAC mechanism have to be active on both power on and standby mode, kaomin
#if ( configSII_DEV_953x_PORTING == 1 )
        AmTCecTaskAssign ( SYS_CEC_APP_TASK );	//Assign task event to CEC after powered on sil953x
        SYS_CEC_APP_TASK = 0;	//Reset app task event

        AmTArcTaskAssign ( SYS_CEC_ARC_APP_TASK );	//Assign task event to ARC after powered on sil953x
        SYS_CEC_ARC_APP_TASK = 0;	//Reset app task event

        AmTSacTaskAssign ( SYS_CEC_SAC_APP_TASK );	//Assign task event to SAC after powered on sil953x
        SYS_CEC_SAC_APP_TASK = 0;	//Reset app task event
#endif

}

static void HdmiDeviceManager_RstDevice ( void )
{
    GPIOMiddleLevel_Clr ( __O_RST_HDMI_SIL953x );
    VirtualTimer_sleep ( 1 );
    GPIOMiddleLevel_Set ( __O_RST_HDMI_SIL953x );
    VirtualTimer_sleep ( 1 );
}

static bool_t HdmiDeviceManager_Initializie ( void )
{
    bool_t ret = true;

    ret = SkAppInitialize();
    SkAppSignon();

#if ( configSII_DEV_953x_PORTING_CBUS == 1 )
#if ( INC_CBUS_WAKEUP )
    ret = SkAppDeviceInitCbusWakeup();
#endif
#endif

    ret = SkAppDeviceInitialize(); // Initialize anything that can be done with PWD and/or AON power on
    SkAppLowPowerStandby();     // Place most of the chip into low power standby mode

#if INC_CEC
    SkAppCecResume ( false );   // Resume the CEC component so it can handle wake up messages
#endif

    return ( ret );

}

//_______________________________AMTRAN IMPLEMENTED_____________________________________________
#if ( configSII_DEV_953x_PORTING == 1 )
#if 0
static bool HdmiDeviceManager_Is_HDMI_Task_Ready ( void )
{
    return IS_HDMI_PWR_ON;
}

static void HdmiDeviceManager_RestartTask ( bool Enable )
{
    if ( Enable )
    {
        xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_READY;
        //TRACE_DEBUG((0,"Restart HDMI Task-----------------"));
    }
    else
    {
        xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_SUSPENDED;
        //TRACE_DEBUG((0,"Suspend HDMI Task-----------------"));
    }
}
#endif
#endif

static bool HdmiManager_DeviceEnable ( bool enable )
{
    if ( enable == TRUE )
    {
        if ( app.powerState == app.oldPowerState )
        {
            if ( app.powerState == APP_POWERSTATUS_STANDBY )
            {
                //app.powerState = APP_POWERSTATUS_STANDBY_TO_ON;
                standby_status.b_change_status = TRUE;
                standby_status.new_pwr_status = APP_POWERSTATUS_STANDBY_TO_ON;
                TRACE_DEBUG ( ( 0, "sil9533 powering up !! " ) );
                return TRUE;
            }
            else if ( app.powerState == APP_POWERSTATUS_ON )
            {
                TRACE_DEBUG ( ( 0, "sil9533 is ALREADY powered up !! " ) );
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else if ( app.powerState == APP_POWERSTATUS_STANDBY_TO_ON )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        if ( app.powerState == app.oldPowerState )
        {
            if ( app.powerState == APP_POWERSTATUS_ON )
            {
                //app.powerState = APP_POWERSTATUS_ON_TO_STANDBY;
                standby_status.b_change_status = TRUE;
                standby_status.new_pwr_status = APP_POWERSTATUS_ON_TO_STANDBY;

                TRACE_DEBUG ( ( 0, "sil9533 powering down !! " ) );
                return TRUE;
            }
            else if ( app.powerState == APP_POWERSTATUS_STANDBY )
            {
                TRACE_DEBUG ( ( 0, "sil9533 is ALREADY powered down !! " ) );
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else if ( app.powerState == APP_POWERSTATUS_ON_TO_STANDBY )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    /*
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
    */
}

static void setPowerDown(xPowerHandle_5V_Ctrl state5V)
{
    xPowerHandle_Queue EventParams;
    memset ( &EventParams, 0, sizeof ( xPowerHandle_Queue ) );
    EventParams.pwr_handle_id = POWER_HANDLE_SII953X;
    EventParams.b_pwr_handle_restart = TRUE;
    EventParams.pwr_handle_state = POWER_HANDLE_DOWN;
    EventParams.pwr_handle_5v_ctrl = state5V;
    pPowerHandle_ObjCtrl->turn_off_done ( xOS_HDMI_Parms.power_handle );
    pPowerHandle_ObjCtrl->power_ctrl ( EventParams );
}

static void HdmiDeviceManager_handleCommand ( xHMISystemParams params )
{
    switch ( params.sys_event )
    {
    case SYS_EVENT_SYSTEM_UP:
    {
        SYS_CEC_APP_TASK = 0;
        SYS_CEC_ARC_APP_TASK = 0;
        SYS_CEC_SAC_APP_TASK = 0;

#if 0
        while ( HdmiManager_DeviceEnable ( TRUE ) == FALSE )
        {
            TRACE_DEBUG ( ( 0, "hdmi wait sil9533 power on event send !! " ) );
            vTaskDelay ( 1 );
        }

        while ( HdmiDeviceManager_IsHDMIPowerOff() == TRUE )
        {
            TRACE_DEBUG ( ( 0, "hdmi arc wait sil9533 power up !! " ) );
            vTaskDelay ( 1 );
        }
#else
        HdmiManager_DeviceEnable ( TRUE );
#endif
        pPowerHandle_ObjCtrl->turn_on_done ( xOS_HDMI_Parms.power_handle );

        //_hdmi_pwr_status = HDMI_POWER_TURN_ON;

        if ( params.input_src == AUDIO_SOURCE_HDMI_ARC )
        {

#if INC_ARC
            AmTCecAppTaskAssign ( HDMI_TASK_ARC_RX_MODE );
            //TRACE_DEBUG((0, "SYS_EVENT_SYSTEM_UP: HDMI_TASK_ARC_RX_MODE"));
            //AmTArcAppTaskAssign(ARC_TASK_INIT_OUTRIGHT);
#endif

#if INC_CEC_SAC
            //if ( params.cec_sac == 1 )
            //    AmTSacAppTaskAssign ( SAC_TASK_INITIATE_POWER_ON );
            //AmTSacAppTaskAssign(SAC_TASK_INITIATE_DIRECTLY);
#endif
        }
        else if ( params.input_src == AUDIO_SOURCE_HDMI_IN )
        {
#if INC_ARC
            AmTCecAppTaskAssign ( HDMI_TASK_ARC_DISABLE_MODE );
            //TRACE_DEBUG((0, "SYS_EVENT_SYSTEM_UP: HDMI_TASK_ARC_DISABLE_MODE"));
            //AmTArcAppTaskAssign(ARC_TASK_TERM_PRIVATELY);	//terminate ARC but keep flag enable
#endif

#if INC_CEC_SAC
            //AmTSacAppTaskAssign(SAC_TASK_INITIATE_POWER_ON);
#endif
        }

	//Initial SAC under all input, kaomin
        {
#if INC_CEC_SAC
            //if ( params.cec_sac == 1 )
            {
                AmTSacAppTaskAssign(SAC_TASK_INITIATE_POWER_ON);
            }
#endif
        }
    }
    break;

    case SYS_EVENT_SYSTEM_DOWN:
    {
        //HdmiManager_DeviceEnable( FALSE );

#if INC_CEC_SAC
        //Need use this api to pass event when HDMI is off
        AmTSacAppTaskAssign ( SAC_TASK_TERMINATE_POWER_OFF );
        AmTArcAppTaskAssign(ARC_TASK_TERM_PRIVATELY); // CECT 11.2.17-2
        //AmTCecEventPassToHMI(USER_EVENT_CEC_SET_SAC_OFF);
#endif

        // repeater loop should still active because of CEC, kaomin
        //check 5v from dvd
        if ( pPowerHandle_ObjCtrl->power_check_5v_from_dvd() == TRUE )
        {
            setPowerDown(POWER_HANDLE_5V_ON);
        }
        else
        {
            //CecSacTaskTerminate();
            HdmiManager_DeviceEnable( FALSE );
        }
        //unregister to power handle
    }
    break;

    case SYS_EVENT_SEL_SRC:
    {
        TRACE_DEBUG ( ( 0, "port select = %d", params.input_src ) );

        if ( params.input_src == AUDIO_SOURCE_HDMI_ARC )
        {
            SYS_CEC_APP_TASK = 0;
            SYS_CEC_ARC_APP_TASK = 0;
            SYS_CEC_SAC_APP_TASK = 0;

#if 0
            while ( HdmiManager_DeviceEnable ( TRUE ) == FALSE )
            {
                TRACE_DEBUG ( ( 0, "hdmi wait sil9533 power on event send !! " ) );
                vTaskDelay ( 1 );
            }

            while ( HdmiDeviceManager_IsHDMIPowerOff() == TRUE )
            {
                TRACE_DEBUG ( ( 0, "sel src hdmi arc  wait sil9533 power up !! " ) );
                vTaskDelay ( 1 );
            }
#else
            if ( HdmiDeviceManager_IsHDMIPowerOff() == TRUE )
            {
                HdmiManager_DeviceEnable ( TRUE );
            }
#endif
#if INC_ARC
            AmTCecAppTaskAssign ( HDMI_TASK_ARC_RX_MODE );
            //TRACE_DEBUG((0, "SYS_EVENT_SEL_SRC: HDMI_TASK_ARC_RX_MODE"));
            //AmTArcAppTaskAssign(ARC_TASK_INIT_OUTRIGHT);
#endif
#if INC_CEC_SAC
            if ( params.cec_sac == 1 )
                AmTSacAppTaskAssign ( SAC_TASK_INITIATE_POWER_ON );
            //AmTSacAppTaskAssign(SAC_TASK_INITIATE_DIRECTLY);
#endif
        }
        else if ( params.input_src == AUDIO_SOURCE_HDMI_IN )
        {
            SYS_CEC_APP_TASK = 0;
            SYS_CEC_ARC_APP_TASK = 0;
            SYS_CEC_SAC_APP_TASK = 0;

#if 0
            while ( HdmiManager_DeviceEnable ( TRUE ) == FALSE )
            {
                TRACE_DEBUG ( ( 0, "hdmi wait sil9533 power on event send !! " ) );
                vTaskDelay ( 1 );
            }

            while ( HdmiDeviceManager_IsHDMIPowerOff() == TRUE )
            {
                TRACE_DEBUG ( ( 0, "sel src hdmi in  wait sil9533 power up !! " ) );
                vTaskDelay ( 1 );
            }
#else
            if ( HdmiDeviceManager_IsHDMIPowerOff() == TRUE )
            {
                HdmiManager_DeviceEnable ( TRUE );
            }
#endif

#if INC_ARC
            AmTCecAppTaskAssign ( HDMI_TASK_ARC_DISABLE_MODE );
            //TRACE_DEBUG((0, "SYS_EVENT_SEL_SRC: HDMI_TASK_ARC_DISABLE_MODE"));
            // AmTArcAppTaskAssign(ARC_TASK_TERM_PRIVATELY);	//terminate ARC but keep flag enable
#endif
#if INC_CEC_SAC
            // AmTSacAppTaskAssign(SAC_TASK_INITIATE_POWER_ON);
#endif	
                    }
#if 0 //Switch to non HDMI input won't terminated SAC link
                    else if ( (params.last_input_src == AUDIO_SOURCE_HDMI_IN) || (params.last_input_src == AUDIO_SOURCE_HDMI_ARC) )
                    {
#if INC_ARC
            //Need use this api to pass event when HDMI is off
            AmTArcAppTaskAssign ( ARC_TASK_TERM_REQUESTED );	//terminate ARC but keep flag enable
#endif

#if INC_CEC_SAC
            //Need use this api to pass event when HDMI is off
            AmTSacAppTaskAssign ( SAC_TASK_TERMINATE_POWER_OFF );
            AmTCecEventPassToHMI ( USER_EVENT_CEC_SET_SAC_OFF );//Fix SB4051_C0-299, SB4051_C0-315(SB3851_C0M-115 )
#endif
            //AmTCecAppTaskAssign(HDMI_TASK_HDMI_OFF);
            //TRACE_DEBUG((0, "SYS_EVENT_SEL_SRC: HDMI_TASK_HDMI_OFF"));
            //TRACE_DEBUG((0, "Current Input: %d",  params.input_src));
                    }
#endif
    }
    break;

    case SYS_EVENT_HDMI_AUTO_PWR_ON:
    {
        SYS_CEC_APP_TASK = 0;
        SYS_CEC_ARC_APP_TASK = 0;
        SYS_CEC_SAC_APP_TASK = 0;

#if 0
        while ( HdmiManager_DeviceEnable ( TRUE ) == FALSE )
        {
            TRACE_DEBUG ( ( 0, "hdmi wait sil9533 power on event send !! " ) );
            vTaskDelay ( 1 );
        }

        while ( HdmiDeviceManager_IsHDMIPowerOff() == TRUE )
        {
            TRACE_DEBUG ( ( 0, "auto hdmi wait sil9533 power up !! " ) );
            vTaskDelay ( 1 );
        }
#else
        if ( HdmiDeviceManager_IsHDMIPowerOff() == TRUE )
        {
            TRACE_DEBUG ( ( 0, "hdmi task turn on !! " ) );
            HdmiManager_DeviceEnable ( TRUE );
        }
#endif

#if(0)
#if INC_ARC
        AmTCecAppTaskAssign ( HDMI_TASK_ARC_DISABLE_MODE );
        AmTArcAppTaskAssign ( ARC_TASK_TERM_PRIVATELY );  //terminate ARC but keep flag enable
#endif
#if INC_CEC_SAC
        AmTSacAppTaskAssign ( SAC_TASK_INITIATE_POWER_ON );
#endif
#endif
    }
    break;

    case SYS_EVENT_HDMI_AUTO_PWR_OFF:
    {
#if 0
        while ( HdmiManager_DeviceEnable ( FALSE ) == FALSE )
        {
            TRACE_DEBUG ( ( 0, "hdmi wait sil9533 power off event send !! " ) );
            vTaskDelay ( 1 );
        }

        while ( HdmiDeviceManager_IsHDMIPowerOff() == FALSE )
        {
            TRACE_DEBUG ( ( 0, "auto hdmi wait sil9533 power down !! " ) );
            vTaskDelay ( 1 );
        }
#else
        //if (HdmiDeviceManager_IsHDMIPowerOff() == FALSE)
        {
            TRACE_DEBUG ( ( 0, "hdmi task turn off !! " ) );
            HdmiManager_DeviceEnable ( FALSE );
        }
#endif

#if 0// INC_CEC_SAC
        //Need use this api to pass event when HDMI is off
        AmTSacAppTaskAssign ( SAC_TASK_TERMINATE_POWER_OFF );
        //AmTCecEventPassToHMI(USER_EVENT_CEC_SET_SAC_OFF);
#endif

#if 0//INC_ARC
        //Need use this api to pass event when HDMI is off
        AmTArcAppTaskAssign ( ARC_TASK_TERM_REQUESTED );	//terminate ARC but keep flag enable
#endif
        //AmTCecAppTaskAssign(HDMI_TASK_HDMI_OFF);
    }
    break;

#if INC_CEC_SAC
    case SYS_EVENT_SAC_REPORT_AUDIO_STATUS:
    {
        AmTCecSacSetVolumeStatus(params.cec_sac, params.master_gain, params.mute);
        AmTSacAppTaskAssign ( SAC_TASK_REPORT_AUDIO_STATUS );
        //audio_status_synced();
    }
    break;

    case SYS_EVENT_VOLUME_SET:
    {
        AmTCecSacSetVolumeStatus(params.cec_sac, params.master_gain, params.mute);
        AmTSacAppTaskAssign ( SAC_TASK_REPORT_AUDIO_STATUS );
        //audio_status_synced();
    }
    break;
#endif
    }
}

static bool HdmiDeviceManager_InstrSender ( void *params )
{
    xHMISystemParams* pInstr = ( xHMISystemParams * ) params;

    if ( pInstr == NULL )
    {
        TRACE_ERROR ( ( 0, "HdmiDeviceManager_InstrSender parameters error !! " ) );
        return FALSE;
    }

    if ( xOS_HDMI_Parms.QParams.xQueue == NULL )
    {
        TRACE_ERROR ( ( 0, "HdmiDeviceManager_InstrSender xqueue is null !! " ) );
        return FALSE;
    }

    if ( xQueueSend ( xOS_HDMI_Parms.QParams.xQueue, pInstr, xOS_HDMI_Parms.QParams.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR ( ( 0, "HdmiDeviceManager_InstrSender sends queue failure " ) );
        return FALSE;
    }
    return TRUE;

}

#if 0
static uint8 HdmiDeviceManager_GetQueueNumber ( void )
{
    return ( uint8 ) uxQueueMessagesWaiting ( xOS_HDMI_Parms.QParams.xQueue );
}
#endif

static bool HdmiDeviceManager_InstrReceiver ( xHMISystemParams *pInstr )
{
    if ( pInstr == NULL )
    {
        TRACE_ERROR ( ( 0, " HdmiDeviceManager_InstrReceiver instruction recevier error !! " ) );
    }

    if ( xOS_HDMI_Parms.QParams.xQueue == NULL )
    {
        TRACE_ERROR ( ( 0, "HdmiDeviceManager_InstrReceiver queue is null !! " ) );
        return FALSE;
    }

    if ( xQueueReceive ( xOS_HDMI_Parms.QParams.xQueue, pInstr, xOS_HDMI_Parms.QParams.xBlockTime ) != pdPASS )
    {
        // just timeout
        //TRACE_ERROR ( ( 0, " HdmiDeviceManager_InstrReceiver receiver queue is failure " ) );
        return FALSE;
    }

    return TRUE;
}

static void HdmiManager_RepeaterTask ( void *pvParameters )
{
    xHMISystemParams systemParams;
    uint8 watchAliveCnt = 0;
    //xPowerHandle_Queue EventParams;

    // initialization of sii9533 and repeater task, kaomin
    SiiPlatformInitialize();
    xOS_HDMI_Parms.xOS_ErrId = xOS_TASK_ERR_NULL;
    xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_SUSPENDED;
    xOS_HDMI_Parms.power_handle = pPowerHandle_ObjCtrl->register_handle();

    HdmiDeviceManager_RstDevice();

    while ( HdmiDeviceManager_Initializie() != true )
    {
        TRACE_ERROR ( ( 0, "SIL9533 cannot be connected !! " ) );
        //xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_READY;
        HdmiManager_DeviceEnable ( FALSE );
        vTaskDelay(1);
    }
    vTaskDelay(25); // wait a while for initial complete, kaomin
   // end of initialization of sii9533 and repeater task, kaomin

    xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_READY;
    SkAppCecConfigure(true); // cec is always on even in the beginning, kaomin

    xOS_HDMI_Parms.watchdog_idx = Watchdog_register();
        
    for ( ;; )
    {
        if(watchAliveCnt++ >= 10)
        {
            Watchdog_taskAlive(xOS_HDMI_Parms.watchdog_idx);
            watchAliveCnt = 0;
        }
        if ( HdmiDeviceManager_InstrReceiver ( ( xHMISystemParams* ) &systemParams ) == TRUE )
        {
            HdmiDeviceManager_handleCommand ( systemParams );
        }

        switch ( xOS_HDMI_Parms.hdmi_avr_repeater_state )
        {
        case TASK_SUSPENDED:
            // actually, suspend mode should be meaningless, just keep for future use, kaomin
            break;

        case TASK_READY:
        {
            xHMISystemParams pdCmd;
                    if(SiiPlatformInterruptWait( 0 ))
                    {
                        // do nothing
                    }
                    
                    sii953x_repeater_task();
            // to fix continous interupt issue if AC on with Tx no HPD, kaomin
                    SkAppTxHpdConnection(ON);
                    SkAppTxHpdConnection(OFF);
                    xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_RUNING;
            // start in the power down status, kaomin
            pdCmd.sys_event = SYS_EVENT_SYSTEM_DOWN;
            HdmiDeviceManager_handleCommand(pdCmd);
                   break;
            }

        case TASK_RUNING:
        {
            if(SiiPlatformInterruptWait( 0 ))
            {
                // do nothing
            }

            sii953x_repeater_task();

            // once standby ready, send command to power down 5V, kaomin
            if(app.powerState == APP_POWERSTATUS_STANDBY)
            {
                if(pPowerHandle_ObjCtrl->get_power_handle_state() == POWER_HANDLE_DOWN)
                {
                    setPowerDown(POWER_HANDLE_5V_OFF);
                }
            }
        }
        break;

        case TASK_BLOCKED:
        {
            TRACE_ERROR ( ( 0, "SIL9533 cannot be connected !! " ) );
        }
        break;

        }

        //vTaskDelay( HDMI_REPEATER_TIME_TICK );
    }
}

#if 0
static void HdmiDeviceManager_ServiceHandle ( void *pvParameters )
{
    xHMISystemParams systemParams;

    for ( ;; )
    {
        if ( HdmiDeviceManager_InstrReceiver ( ( xHMISystemParams* ) &systemParams ) == TRUE )
        {
            HdmiDeviceManager_handleCommand ( systemParams );
        }
    }

}
#endif

static void HdmiDeviceManager_CreateTask ( void )
{
    xOS_HDMI_Parms.QParams.xQueue = xQueueCreate ( 10, FRTOS_SIZE ( xHMISystemParams ) );
    xOS_HDMI_Parms.QParams.xBlockTime = BLOCK_TIME ( 25 ); // both send and receive wait 25ms for queue ready, kaomin
    ASSERT ( xOS_HDMI_Parms.QParams.xQueue != NULL );

    /*Smith Fixes bug: stack size is too close high water that cause system hard fault, 6/3/2014*/
    if ( xTaskCreate ( HdmiManager_RepeaterTask,
                       ( portCHAR * ) "SII9535_ENTRY",
                       ( STACK_SIZE*3 ), NULL, tskSII9535_PRIORITY, &xOS_HDMI_Parms.hdmi_avr_repeater_task_handle ) != pdPASS )
    {
        vTaskDelete ( xOS_HDMI_Parms.hdmi_avr_repeater_task_handle );
        xOS_HDMI_Parms.xOS_ErrId = xOS_TASK_CREATE_FAIL;
        xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_SUSPENDED;
    }
#if 0 // move to task inside, kaomin
    else
    {
        SiiPlatformInitialize();
        //vTaskSuspend( xOS_HDMI_Parms.hdmi_avr_repeater_task_handle );
        xOS_HDMI_Parms.xOS_ErrId = xOS_TASK_ERR_NULL;
        xOS_HDMI_Parms.hdmi_avr_repeater_state = TASK_READY;
    }

    if ( xTaskCreate (
                HdmiDeviceManager_ServiceHandle,
                ( portCHAR * ) "HDMI_SRV",
                ( STACK_SIZE ),
                NULL,
                tskHDMI_SERVICE_PRIORITY,
                &xOS_HDMI_Parms.hdmi_service_handle ) != pdPASS )
    {
        xOS_HDMI_Parms.xOS_ErrId = xOS_TASK_CREATE_FAIL;
        xOS_HDMI_Parms.ServiceState = TASK_SUSPENDED;
        vTaskDelete ( xOS_HDMI_Parms.hdmi_service_handle );
    }
    else
    {
        //register to power handle
        xOS_HDMI_Parms.power_handle = pPowerHandle_ObjCtrl->register_handle();
        //TRACE_DEBUG((0,"hdmi handle register --- %d",xOS_HDMI_Parms.power_handle));
    }

#endif
}

bool HdmiDeviceManager_GetMultiChannel ( void )
{
    return AmTAudiogetMultiChannel();
}

xHMI_SiiAppPowerStatus_t HdmiDeviceManager_GetHDMIPowerStatus ( void )
{
    return ( xHMI_SiiAppPowerStatus_t ) app.powerState;
}

bool HdmiDeviceManager_IsHDMIPowerOff ( void )
{
    bool ret_val = FALSE;

    if ( app.powerState == app.oldPowerState )
    {
        if ( app.powerState == APP_POWERSTATUS_STANDBY )
        {
            ret_val = TRUE;
        }
        else
        {
            ret_val = FALSE;
        }
    }
    else
    {
        ret_val = FALSE;
    }
    return ret_val;
}


//-------------------------------------------------------------------------------------------------
//! @brief
//-------------------------------------------------------------------------------------------------
void AmTArcAppTaskAssign ( AmTArcTaskEvent arc_event )
{
    xHMISystemParams SysPrams;

    SysPrams = AmTCecGetSyspramsFromHMI();

    // Ignore CEC message under demo mode
    if ( (SysPrams.op_mode != MODE_USER) && (SysPrams.op_mode != MODE_FACTORY) )
    {
        return;
    }

    switch ( arc_event )
    {
#if 0 // useless, kaomin
    case ARC_TASK_INIT_OUTRIGHT:
    {
        AmTCecEventPassToHMI ( USER_EVENT_CEC_SET_ARC_ON );
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
        AmTCecEventPassToHMI ( USER_EVENT_CEC_SET_ARC_ON );
        AmTCecEventPassToHMI ( USER_EVENT_CEC_POWER_ON_HDMI_ARC );
        SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_INITIONAL;
    }
    break;
#endif
    case ARC_TASK_TERM_REQUESTED:
    {
        //if ( SysPrams.cec_arc == TRUE )
        {
            AmTCecEventPassToHMI ( USER_EVENT_CEC_SET_ARC_OFF );
            SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_TERMINATE;
            SYS_CEC_ARC_APP_TASK &= ~SYS_CEC_TASK_ARC_INITIONAL;
        }
    }
    break;

    case ARC_TASK_TERM_PRIVATELY:
    {
        //if ( SysPrams.cec_arc == TRUE )
        {
            SYS_CEC_ARC_APP_TASK |= SYS_CEC_TASK_ARC_TERMINATE;
            SYS_CEC_ARC_APP_TASK &= ~SYS_CEC_TASK_ARC_INITIONAL;
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
void AmTSacAppTaskAssign ( AmTSacTaskEvent sac_event )
{
    xHMISystemParams SysPrams;

    SysPrams = AmTCecGetSyspramsFromHMI();

    // Ignore CEC message under demo mode
    if ( (SysPrams.op_mode != MODE_USER) && (SysPrams.op_mode != MODE_FACTORY) )
    {
        return;
    }

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

    case SAC_TASK_REPORT_AUDIO_STATUS:
    {
        SYS_CEC_SAC_APP_TASK |= CEC_SAC_TASK_REPORT_AUDIO_STATUS;
    }
    break;

    case SAC_TASK_POLLING_ARC:
    {
        SYS_CEC_SAC_APP_TASK |= CEC_SAC_TASK_POLLING_ACTIVE;
    }
    break;
    
    default:
    {
        break;
    }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief
//-------------------------------------------------------------------------------------------------
void AmTCecAppTaskAssign ( AmTCecTaskEvent cec_event )
{
    xHMISystemParams SysPrams;

    SysPrams = AmTCecGetSyspramsFromHMI();

    // Ignore CEC message under demo mode
    if ( (SysPrams.op_mode != MODE_USER) && (SysPrams.op_mode != MODE_FACTORY) )
    {
        return;
    }

    switch ( cec_event )
    {
    case HDMI_TASK_ARC_RX_MODE:
    {
        SYS_CEC_APP_TASK |= CEC_HDMI_TASK_ARC_RX_MODE;
    }
    break;

    case HDMI_TASK_ARC_DISABLE_MODE:
    {
        SYS_CEC_APP_TASK |= CEC_HDMI_TASK_ARC_DISABLE_MODE;
    }
    break;

    case HDMI_TASK_HDMI_OFF:
    {
        SYS_CEC_APP_TASK |= CEC_HDMI_TASK_HDMI_OFF;
    }
    break;

    default:
    {
        break;
    }
    }
}
#if ( configSII_DEV_953x_PORTING == 1 )

/*******************************************************************************
 * Mutex
 ******************************************************************************/
#if 0
static bool HdmiDeviceManager_mutex_take ( void )
{
#if 0
    //if ( bIsI2C2_enable!= TRUE )
    //{
    //    TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 1111 !! "));
    //    return FALSE;
    //}
    //return TRUE;
    if ( HdmiDevMNGsema.xSemaphore == NULL )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 2222 !! "));
        return FALSE;
    }

    HdmiDevMNGsema.xBlockTime = portMAX_DELAY;
    if ( xSemaphoreTake ( HdmiDevMNGsema.xSemaphore, HdmiDevMNGsema.xBlockTime ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 3333 !! "));
        return FALSE;
    }
    //TRACE_DEBUG((0, "ADM_I2C2_sema_mutex_take "));
    return TRUE;
#else
    return TRUE;
#endif
}

static bool HdmiDeviceManager_mutex_give ( void )
{
#if 0
    //if ( bIsI2C2_enable!= TRUE )
    //    return FALSE;
    //return TRUE;
    if ( HdmiDevMNGsema.xSemaphore == NULL )
        return FALSE;

    if ( xSemaphoreGive ( HdmiDevMNGsema.xSemaphore ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 GIVE BUSY !! "));
        return FALSE;
    }
    //TRACE_DEBUG((0, "ADM_I2C2_sema_mutex_give "));
    return TRUE;
#else
    return TRUE;
#endif
}
#endif
#endif
