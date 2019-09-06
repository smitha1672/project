//***************************************************************************
//!file     sk_app_cec.c
//!brief    Wraps board and device functions for the CEC component
//!         and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_app_cec.h"
#if INC_CBUS
#include "sk_app_cbus.h"
#endif
#include "si_edid_tx_component.h"

#define ALLOW_VIRTUAL_DEVICES 1

static uint8_t  l_sinkCecTypeList[] = { 0, CEC_LOGADDR_UNREGORBC };
static uint8_t  l_allCecTypeList[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, CEC_LOGADDR_UNREGORBC };
#if ( configSII_DEV_953x_PORTING == 1 )
uint8_t  appCecSourceLaList[] = { CEC_LOGADDR_AUDSYS, CEC_LOGADDR_UNREGORBC };
#else
uint8_t  appCecSourceLaList[] = { CEC_LOGADDR_AUDSYS,
#if ALLOW_VIRTUAL_DEVICES
		CEC_LOGADDR_PLAYBACK1, CEC_LOGADDR_PLAYBACK2,CEC_LOGADDR_PLAYBACK3,
#else
		CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC,	CEC_LOGADDR_UNREGORBC,
#endif
		CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC,
		CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC };
#endif
static char equalSymbolLine [] = "**============================**\n";

static char l_cecAvrOsdNameString [] = "VIZIO SB4051";
//static char l_cecAvrOsdNameString [] = "SII AVR";
//static char l_cecAuxOsdNameString [] = "SII AUX";

SiiAppCecInstanceData_t appCec[SII_NUM_CPI];
SiiAppCecInstanceData_t *pAppCec = &appCec[0];

#if ( configSII_DEV_953x_PORTING == 1 )
uint8_t SYS_CEC_TASK = 0x00;

extern CecSacInstanceRecord_t *pSac;
extern CecInstanceData_t *pCec;
extern void AmTArcAppTaskAssign( AmTArcTaskEvent event);
#endif

//------------------------------------------------------------------------------
//! @brief      Set the correct instance for each of the CEC-related components
//!             and drivers associated with the passed CEC instance.
//! @param[in]  newInstance - new instnce index.
//------------------------------------------------------------------------------
void SkAppCecInstanceSet ( int_t newInstance )
{
    // In this case, the instances are one-to-one, but it is
    // possible for a mismatch if, for example, you have one
    // CDC instance that is associated with CPI instance 1
    // instead of instance 0.

    if ( newInstance < SII_NUM_CPI )
    {
        pAppCec = &appCec[newInstance];
        pAppCec->instanceIndex = newInstance;
        SiiCecInstanceSet( newInstance );
#if INC_CDC
        SiiCdcInstanceSet( CDC_INSTANCE_RX );
#endif
        SiiDrvCpiInstanceSet( newInstance );
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppCecStandby
//! @brief      Places all CEC components/drivers into standby
// Returns:     true if the CEC component entered standby mode successfully,
//              or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SkAppCecStandby ( void )
{
    // If CEC is not enabled by the user, we're done.

    if ( !app.cecEnable )
    {
        return( true );
    }

    // Tell CEC that system power is going off
    SkAppCecInstanceSet( CEC_INSTANCE_AVR );
    SiiCecSetPowerState( (SiiCecPowerstatus_t)APP_POWERSTATUS_ON_TO_STANDBY );

    // If we're a TV, tell all attached CEC devices to power off
    if ( SiiCecGetDeviceType() == CEC_DT_TV )
    {
        SiiCecSendMessage( CECOP_STANDBY, CEC_LOGADDR_UNREGORBC );
    }
    SiiCecStandby();

    return( true );
}

//------------------------------------------------------------------------------
//! @brief      Restore operation to all enabled CEC components/drivers.
//! @param[in]  powerIsOn   if true, set CEC power state to ON, otherwise the
//!                         component is operational but does not tell any
//!                         other CEC devices that it system power is on.
//! @return     true if the CEC component successfully exited standby mode,
//!             or false if some failure occurred.
//------------------------------------------------------------------------------
bool_t SkAppCecResume ( bool_t powerIsOn )
{
    // If CEC is not enabled by the user, we're done.
    if ( !app.cecEnable )
    {
        return( true );
    }

    SkAppCecInstanceSet( CEC_INSTANCE_AVR );
    SiiCecResume();
    SiiCecEnumerateDevices( appCecSourceLaList );
    SiiCecEnumerateDeviceLa( appCecSourceLaList );
    if ( powerIsOn )
    {
        SiiCecSetPowerState( (SiiCecPowerstatus_t)APP_POWERSTATUS_STANDBY_TO_ON );
    }

    // Queue-up the enumerate tasks
    if (SiiCecGetDeviceType() == CEC_DT_TV)
    {
        SiiCecEnumerateDevices( l_allCecTypeList );
        SiiCecEnumerateDeviceLa( l_sinkCecTypeList );
    }

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SkAppCecConfigure
// Description:
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppCecConfigure ( bool_t enable )
{
    // If CEC enable state has changed, tell the component.

    if ( app.oldCecEnable != enable )
    {
        app.cecEnable     = enable;
        app.oldCecEnable  = enable;
        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
        SiiCecConfigure( enable );
       // PrintStringOnOff( "CEC:", app.cecEnable );
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppCecRxMsgHandler
// Description: Parse received messages and execute response as necessary
//              Only handle the messages needed at the top level to interact
//              with the Port Switch hardware.  The SiiAPI message handler
//              handles all messages not handled here.
// Parameters:  none
// Returns:     Returns true if message was processed by this handler
//------------------------------------------------------------------------------

bool_t SkAppCecRxMsgHandler ( SiiCpiData_t *pMsg )
{
    bool_t  processedMsg, isDirectAddressed;

    isDirectAddressed = (bool_t)!((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );

	TRACE_DEBUG((0, "SkAppCecRxMsgHandler pMsg->opcode = 0x%X", pMsg->opcode ));

    processedMsg = false;
    switch ( pMsg->opcode )
    {
        case CECOP_IMAGE_VIEW_ON:       // In our case, respond the same to both these messages
        case CECOP_TEXT_VIEW_ON:
            if ( !isDirectAddressed )   break;          // Ignore as broadcast message

            // The CEC Component "First Message Handler" has already seen this message
            // and turned on the system power status (as necessary).  Now, the application
            // must do whatever else needs to be done to turn on the display.

            //TODO:OEM - Do whatever else is required to turn on the TV display.

            DEBUG_PRINT( MSG_STAT, equalSymbolLine );
            DEBUG_PRINT( MSG_STAT, "**     IMAGE/TEXT VIEW ON     **\n");
            DEBUG_PRINT( MSG_STAT, equalSymbolLine );
            processedMsg = true;
            break;

        case CECOP_USER_CONTROL_PRESSED:
        case CECOP_USER_CONTROL_RELEASED:
            if ( !isDirectAddressed )   break;          // Ignore as broadcast message

#if INC_CBUS
//            SkAppCbusReceiveCec( pMsg );
#endif
            processedMsg = true;
            break;

        case CECOP_INACTIVE_SOURCE:
            if ( !isDirectAddressed )   break;          // Ignore as broadcast message

            DEBUG_PRINT( MSG_STAT, equalSymbolLine );
            DEBUG_PRINT( MSG_STAT, "** SWITCH TO INTERNAL SOURCE  **\n");
            DEBUG_PRINT( MSG_STAT, equalSymbolLine );
            processedMsg = true;
            break;
        default:
            break;
    }

    return( processedMsg );
}

//------------------------------------------------------------------------------
// Function:    CecTimerSet
// Description: Elapsed timers for CEC use
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

static void CecTimerSet ( clock_time_t msDelay  )
{
    app.msCecDelay = msDelay;
    app.msCecStart 	= SiiOsTimerTotalElapsed();
}
static bool_t CecTimerElapsed ( void )
{
    clock_time_t thisTime = SiiOsTimerTotalElapsed();

    return((bool_t)(( thisTime - app.msCecStart) >= app.msCecDelay));
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskCec
// Description: CEC control task.  Emulate a task with a blocking semaphore
//              waiting for a device interrupt or a timer-based poll request
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskCec ( void )
{
    bool_t      	cecProcessed = false;
    uint16_t    	cecComponentStatus;

    if ( app.cecEnable == false )
    {
        return;
    }

    // If a CEC interrupt occurred, call the CEC handler.

    if ( app.cecInterruptRx || app.cecInterruptTx ||CecTimerElapsed())
    {
        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
        SiiCecHandler();
        cecProcessed = true;

        // This instance is the Switch side, so check for a port change.

        cecComponentStatus = SiiCecStatus();    // Only get one chance at these
        if ( cecComponentStatus & SiiCEC_PORT_CHANGE )
        {
            app.newSource[app.currentZone ] = (SiiSwitchSource_t)SiiCecPortSelectGet();
        }

        if ( cecComponentStatus & SiiCEC_POWERSTATE_CHANGE )
        {
            app.powerState = (SiiAppPowerStatus_t)SiiCecGetPowerState();
        }
        app.cecInterruptRx = false;
        app.cecInterruptTx = false;
    }

#if ( configSII_DEV_953x_PORTING == 1 )
    AmTCecTask();
#endif

/*    if ( SYS_CEC_TASK | !(SYS_CEC_TASK_INIT_SAC) )
    {
	xHMISystemParams SysPrams;
	
	SysPrams = AmTCecGetSyspramsFromHMI();

	if ( pSac->status.isSystemAudioModeEnabled != SysPrams.cec_sac )
	{	
		pSac->taskFlags |= CEC_SAC_TASK_POWER_ON_TV;
		SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, true);
		SYS_CEC_TASK &= !(SYS_CEC_TASK_INIT_SAC);
	}
    }
*/

    if ( cecProcessed )
    {
        CecTimerSet( 100 );     // 100ms before next poll
    }
}

//------------------------------------------------------------------------------
//! @brief  Initializes CPI and CEC hardware for both Rx and Tx.
//!         The hardware is initialized and message handlers registered,
//!         but the hardware is left in a disabled state.  The hardware (and
//!         thus CEC support) is enabled/disabled via the SkAppCecConfigure
//!         function.
//
//! @note   TX Instance CPI initialization MUST be done after the TPI driver
//!         has been initialized.
//------------------------------------------------------------------------------
bool_t SkAppDeviceInitCec ( void )
{
    uint16_t    cecPa;
    bool_t      success = false;

    do
    {
        //**************************************************************************************
        // Configure/Initialize the RX CEC/CPI as an AUDIO System with SWITCH
        //**************************************************************************************

        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
        if ( !SiiDrvCpiInitialize())    break;
#if INC_CEC_LOGGER
        if ( !SiiCpiRegisterLogger( SkAppCecMessageLogger ))
        {
            DEBUG_PRINT( MSG_ERR, "Unable to register RX CPI logger\n");
            break;
        }
#endif

        SiiEdidTxInstanceSet(0);
        SkAppTxInstanceSet(0);
        // Can't initialize if valid physical address not assigned
        cecPa = SiiEdidTxCecPhysAddrGet();
        if (cecPa == 0xFFFF)                                                            break;
        if ( !SiiCecInitialize( cecPa, CEC_DT_AUDIO_SYSTEM ))                           break;
        SiiCecSetOsdName( l_cecAvrOsdNameString );

#if INC_CEC_SWITCH
        if(app.isCecAsSwitch)
        {
        	// Initialize CEC Switch component and enable its base service
        	SiiCecSwitchConfig();
        	if ( !SiiCecCallbackRegisterParser( SiiCecSwitchTaskProcess, false ))            break;
        	if ( SiiCecSwitchSrvStart())                                                    break;
        }
#else
        if(!SiiCecCallbackRegisterParser( CecRxMsgHandler, false )) break;
#endif

#if INC_CEC_SAC
        // Initialize CEC System Audio Control component and enable its base service
        // Can't initialize SAC if valid physical address not assigned
#if ( configSII_DEV_953x_PORTING == 1 )
        SiiCecSacConfig(0); // take default format list
#else
        SiiCecSacConfig(0, NULL, 0); // take default format list
#endif
        if ( !SiiCecCallbackRegisterParser( SiiCecSacTaskProcess, false ))               break;
        if ( SiiCecSacSrvStart())                                                       break;
#endif

#if INC_CBUS
        // Add in a CEC message parser for CBUS to filter User Key Press and Release
        // We add this extra parser AFTER the SAC parser to allow the SAC to use
        // some of the CECOP_USER_CONTROL_PRESSED and CECOP_USER_CONTROL_RELEASED commands
        // if it needs them. It does not have to be called every time by the CEC handler
        // because it runs no tasks.
//        if ( !SiiCecCallbackRegisterParser( SkAppCbusCecRxMsgFilter, false ))           break;
#endif

        success = true;
    }   while (0);  // Do the above only once

    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Informs CEC system about the input port change by user action
//!
//!             This function shall be called by the system if user changes
//!             input port selection
//!
//! @param[in]  newInputPortIndex - zero based input port index
//-------------------------------------------------------------------------------------------------

void SiiCecAppInputPortChangeHandler(uint8_t newInputPortIndex)
{
    SiiCecSwitchSources( newInputPortIndex );
#if (INC_CEC_SWITCH == ENABLE)
    SiiCecRoutingChangeSend(newInputPortIndex);
#endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      CEC Callback reporting ACK/NOACK from SiiCecSendMessageTask() function.
//!
//! @param[in]  opCode          - Opcode of message being reported
//! @param[in]  messageAcked    - true if message was ACKed
//-------------------------------------------------------------------------------------------------
void SiiCecCbSendMessage( uint8_t opCode, bool_t messageAcked )
{

  //  DEBUG_PRINT( MSG_DBG, "CEC Message %02X was %s\n", opCode, messageAcked ? "ACKed" : "Not ACKed" );
}

//-------------------------------------------------------------------------------------------------
//! @brief      CEC Callback requesting to switch input port.
//!
//! @param[in]  inputPortIndex - input port to switch to
//! @return     success flag.
//-------------------------------------------------------------------------------------------------

bool_t SiiCecCbInputPortSet(uint8_t inputPortIndex)
{
    app.newSource[app.currentZone] = (SiiSwitchSource_t)inputPortIndex;
    SiiCecPortSelectSet(inputPortIndex);
    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      CEC Callback requesting standby mode of this device.
//!
//! @param[in]  inputPortIndex - input port to switch to
//! @return     sucess flag.
//-------------------------------------------------------------------------------------------------

void SiiCbCecStandby(bool_t standby)
{
    if ( standby && (app.powerState == APP_POWERSTATUS_ON ))
    {
        app.powerState = APP_POWERSTATUS_ON_TO_STANDBY;
    }
    else if (!standby && (app.powerState != APP_POWERSTATUS_ON))
    {
        app.powerState = APP_POWERSTATUS_STANDBY_TO_ON;
    }
}

#if (INC_CEC_SWITCH == ENABLE)
//-------------------------------------------------------------------------------------------------
//! @brief      CEC Callback requesting HPD status of selected input port.
//!
//! @return     HPD On status.
//-------------------------------------------------------------------------------------------------

bool_t SiiCbCecSwitchRxHpdGet(void)
{

    return( (bool_t)SiiDrvSwitchStateGet(SiiSwitch_SELECTED_INPUT_CONNECT_STATE));
}

//-------------------------------------------------------------------------------------------------
//! @brief      CEC Callback processing messages from CEC/Switch sub-system.
//!
//! @param[in]  fbMsg - message code
//!
//! @retval     true  - if the message was recognized and processed
//! @retval     false - if the message is irrelevant and wasn't processed
//-------------------------------------------------------------------------------------------------
bool_t SiiCbCecSwitchMessageHandler(uint8_t fbMsg)
{
    bool_t   usedMessage = true;

    /* Process feedback messages.       */
    switch (fbMsg)
    {
        default:
        case CEC_SWITCH_FB_MSG_NONE:
            // nothing to do
            break;

        case CEC_SWITCH_FB_MSG_STATUS_CHANGED:

            // This message occurs when the switch processes a SET_STREAM_PATH
            // or ACTIVE_SOURCE message, which must also be passed to subsequent
            // handlers, so mark message as unused.  They will not feature abort
            // if nothing else uses it, because they are broadcast messages.
            usedMessage = false;
            break;

        case CEC_SWITCH_FB_MSG_SEND_ROUTE_DONE:
           // DEBUG_PRINT(MSG_STAT, ("CEC Switch send route task finished.\n"));
            break;

        case CEC_SWITCH_FB_MSG_ERR_NONSWITCH_CMD:
             usedMessage = false;
             break;
    }

    return( usedMessage );
}
#endif
#if INC_CEC
//-------------------------------------------------------------------------------------------------
//! @brief      CEC Callback updating enumeration task status
//!
//! @param[in]  isComplete  - true if enumeration task has completed
//-------------------------------------------------------------------------------------------------
void SiiCecCbEnumerateComplete( bool_t isComplete )
{

    pAppCec->isEnumerated = isComplete;

#if INC_CBUS
    // May need to assign an available LA to a CBUS device
    SkAppCbusAssignCecDeviceAddress( isComplete );
#endif
}
#endif

#if ( configSII_DEV_953x_PORTING == 1 )
//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
void AmTCecTask( void )
{
    xHMISystemParams SysPrams;

    SysPrams = AmTCecGetSyspramsFromHMI();

    if ( SYS_CEC_TASK & SYS_CEC_TASK_PA_CHANGE )
    {
	SiiCecSendReportPhysicalAddress( pCec->logicalAddr, pCec->physicalAddr );
	
	SYS_CEC_TASK &= !(SYS_CEC_TASK_PA_CHANGE);
    }
#if INC_ARC

    if ( SysPrams.input_src == AUDIO_SOURCE_HDMI_ARC )
    {
	AmTCecArcTask();
	AmTCecSacTask();
    }
    else if ( SysPrams.input_src == AUDIO_SOURCE_HDMI_IN )
    {
	AmTCecSacTask();
    }
#endif


}

//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
void AmTCecTaskAssign( AmTCecTaskEvent event )
{

	switch ( event )
	{
		case Cec_TASK_CHANGE_PA:
		{
			if ( pCec->physicalAddr != 0x0000 )	// PA change, reinitial ARC
			{
				SYS_CEC_TASK |= SYS_CEC_TASK_PA_CHANGE;
#if INC_ARC
				AmTArcAppTaskAssign(ARC_TASK_INIT_PRIVATELY);
#endif
			}
			
			
		}
			break;
	}
}

#endif

