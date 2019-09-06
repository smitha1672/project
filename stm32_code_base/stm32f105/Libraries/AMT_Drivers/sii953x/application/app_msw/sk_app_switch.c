//***************************************************************************
//!file     sk_app_switch.c
//!brief    Wraps platform and device functions for the SWITCH component
//          and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_tx_component.h"
#include "si_repeater_component.h"
#include "si_tx_hdcp.h"
#include "si_drv_device.h"
#include "si_drv_tpi_system.h"
#include "si_drv_switch.h"
#include "si_drv_audio.h"
#include "si_drv_repeater.h"
#if INC_RTPI
#include "si_rtpi_component.h"
#endif

#if INC_CBUS
#include "sk_app_cbus.h"
#endif
#include "si_tx_video.h"
#include "si_connection_monitor.h"  // Bug 33551 - Snow/green video appears on TV while port switching
//-------------------------------------------------------------------------------------------------
//! @brief      Determine the next available HDMI/MHL input (circular) for display.
//!             Starts from the passed current port and skips any unconnected
//!             ports.
//!
//! @param[in]  currentPort - Rx port (0-3).
//! @param[in]  isForward   - Next port is given by increment (true), or decrement (false) of port's index
//!
//! @return     Next sequential active HDMI/MHL port.
//-------------------------------------------------------------------------------------------------
uint8_t SkAppSwitchGetNextPort ( uint8_t currentPort, bool_t isForward )
{
    uint8_t i, stateValue;

    stateValue = SiiDrvSwitchStateGet( SiiSwitch_INPUT_PORTS_CONNECT_STATE );

    for ( i = 0; i < SII_INPUT_PORT_COUNT; i++ )
    {
        if (isForward)
        {
            currentPort = (currentPort + 1) % SII_INPUT_PORT_COUNT;
        }
        else
        {
            currentPort = (currentPort == 0) ? (SII_INPUT_PORT_COUNT - 1) : (currentPort - 1);
        }

            /* Only choose port if it is plugged in.    */

        if ( stateValue & ( 0x01 << currentPort))
        {
            break;
        }
    }

    return( currentPort );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Submit routing settings to the switch hardware.
//-------------------------------------------------------------------------------------------------

static void UpdateFinalZoneConf( void )
{
    uint8_t zone;
    bool_t txSourcechange[] = {false, false};
    SiiSwitchSource_t rptRxSource[2];
	SiiMswInstanceCommonData_t switchZoneInfo;

	SiiDrvSwitchZoneConfigGet( &switchZoneInfo );

    for (zone = 0; zone < SII_NUM_TX; ++zone)
    {
        if (app.newSource[zone] < SiiSwitchSourceInternalVideo)
        {
            rptRxSource[zone] = app.newSource[zone];
        }
        else
        {
            // Enable repeater if audio comes from HDMI Rx
            // and if video source is of non-HDMI type
            if ((app.newAudioSource[zone] == SiiSwitchAudioSourceMainPipe) ||
                (app.newAudioSource[zone] == SiiSwitchAudioSourceSubPipe))
            {
                switchZoneInfo.pipeSel[zone] = (app.newAudioSource[zone] == SiiSwitchAudioSourceMainPipe) ?
                                                 SiiSwitchHdmiPipeMain : SiiSwitchHdmiPipeSub;
                rptRxSource[zone] = app.audioZoneSource[(app.newAudioSource[zone] == SiiSwitchAudioSourceSubPipe)? 1:0];
                txSourcechange[zone] = true;
            }
            else
            {
                // Disable repeater if audio is not from HDMI Rx
                switchZoneInfo.pipeSel[zone] = SiiSwitchHdmiPipeNone;
            }
        }

        // Deactivate Repeater on disabled zones
        if (!app.newZoneEnabled[zone])
        {
           switchZoneInfo.pipeSel[zone] = SiiSwitchHdmiPipeNone;
        }
    }

    // Tell repeater about the change
	SkAppRepeaterSourceConfig( rptRxSource, switchZoneInfo, txSourcechange );
	SiiDrvSwitchZoneConfigSet();
}
//------------------------------------------------------------------------------
// Function:    UpdateResInfo
// Description: Display the resolution info for the port we're switching to.
//              If we were an actual AVR, we could use this information to prepare
//              our receiver hardware for the new format.
//------------------------------------------------------------------------------

static void UpdateResInfo ( int_t zone, SiiSwitchSource_t source )
{
    int_t  horiz, vert, tmdsClock = 0;
    bool_t isInterlaced, isHdmi;
    int_t  isMainPipe = SiiDrvSwitchZoneTrackingGet() || (zone == 0);

    // Don't do it if in auto-switch mode, and can't handle internal video
    if (( app.newSource[0] ) >= SII_INPUT_PORT_COUNT )
    {
        SiiDrvTpiSourceTerminationEnable(false);
        DEBUG_PRINT( MSG_ALWAYS, "TPG : Source Term Disabled\n" );
        return;
    }

    if ( SiiDrvSwitchPortInfoGet( source, isMainPipe, &tmdsClock, &horiz, &vert, &isInterlaced, &isHdmi ))
    {
        if((app.isresolutionVaild == false)&&(app.isTxDisconnectedImmed==true))
        {
            if (!SiiDrvTpiIsPoweredUp())
            {
                SiiDrvTpiPowerUp();
            }
        }

        if(!app.autoSwitch)
        {
            DEBUG_PRINT( MSG_STAT, asterisks );
            if (( horiz | vert ) == 0 )
            {
                DEBUG_PRINT( MSG_STAT, "** linked to %s source %d (Not CEA-861D) **\n",
                    isHdmi ? "" : "DVI", source );
            }
            else
            {
                DEBUG_PRINT( MSG_STAT, "** linked to %s source %d (%4d x %4d%c)\n"
                                       "** Video Clock: %d\n",
                    isHdmi ? "" : "DVI",
                    source, horiz, vert,
                    isInterlaced ? 'i' : 'p', tmdsClock );
            }
            DEBUG_PRINT( MSG_STAT, asterisks );
            app.isresolutionVaild = true;
        }

        if( tmdsClock <= 16500 )
        {
            SiiDrvTpiSourceTerminationEnable(false);
            DEBUG_PRINT( MSG_ALWAYS, "Source Term Disabled\n" );
        }
        else
        {
            SiiDrvTpiSourceTerminationEnable(true);
            DEBUG_PRINT( MSG_ALWAYS, "Source Term Enabled\n" );
        }
    }
    else if(!app.autoSwitch)
    {
        DEBUG_PRINT( MSG_STAT, asterisks );
        DEBUG_PRINT( MSG_STAT, "** linked to source %d (unknown format)    **\n"
                               "** Video Clock: %d\n", source, tmdsClock );
        DEBUG_PRINT( MSG_STAT, asterisks );
        app.isresolutionVaild = false;
        SiiDrvTpiSourceTerminationEnable(false);
        DEBUG_PRINT( MSG_ALWAYS, "Source Term Disabled\n" );
    }

#if ((INC_BENCH_TEST == DISABLE) || (BENCH_TEST_STATIC_300MHZ == DISABLE))
        // Provide pixel clock frequency info to the TX to let it adjust its TMDS termination
        if (source < SiiSwitchSourceInternalVideo)
        {
            uint8_t aAvi[18];
            uint8_t aviLen;

            SiiDrvSwitchInfoFrameGet(isMainPipe, IF_AVI, aAvi, &aviLen);
            SkAppTxAdjustVideoSettings(zone, aAvi, aviLen);

            SkAppTxAdjustAnalogSettings(zone, tmdsClock);
        }

        //SkAppTxAdjustAudioClkDivider(zone, tmdsClock);
#endif

}

//-------------------------------------------------------------------------------------------------
//! @brief      Detects new connection request to a non HDMI source (IV or PV) and
//!             configures TX and/or TPG accordingly.
//-------------------------------------------------------------------------------------------------
void SkAppServiceNonHdmiVideo(bool_t forceResChange )
{
    uint8_t i;
    bool_t isZoneChange;
    bool_t isZoneEnable;
    bool_t isRepeaterModeChange = false;
    bool_t isEnterIv = ((app.newSource[0] == SiiSwitchSourceInternalVideo)) &&
                       ((app.currentSource[0] != SiiSwitchSourceInternalVideo));
    bool_t isQuitIv  = ((app.newSource[0] != SiiSwitchSourceInternalVideo)) &&
                       ((app.currentSource[0] == SiiSwitchSourceInternalVideo));

    isEnterIv = (forceResChange) ? true : isEnterIv;

    // TPG control

    if (isEnterIv || isQuitIv)
    {
    	SkAppTpgEnable(isEnterIv);
    }

    // IV, PV and Repeater/Bypass mode control
    for (i = 0; i < SII_NUM_TX; ++i)
    {
        // Set Repeater mode and Bypass mode
        // Repeater must be enabled for Non HDMI video when accompanied audio comes from HDMI Rx
    	if(app.repeaterEnable) // condition added to enable tx hdcp in non repeter mode, when isTxHdcpRequired is true.
    	{
    		isRepeaterModeChange = app.isTxRepeaterMode[i]; // store old repeater mode flag
    		app.isTxRepeaterMode[i] = ( app.newSource[i] < SiiSwitchSourceInternalVideo) ||
    				(app.newAudioSource[i] == SiiSwitchAudioSourceMainPipe) || (app.newAudioSource[i] == SiiSwitchAudioSourceSubPipe);
    		isRepeaterModeChange = (app.isTxRepeaterMode[i] != isRepeaterModeChange); // set the change flag
    	}
    	else
    	{
    		app.isTxRepeaterMode[i] = false;
    	}
    	app.isTxBypassMode[i] =  (app.newSource[i] < SiiSwitchSourceInternalVideo);

        // Service IV
        // Find out if Zone switches to or from the video source (change detection)
        isZoneChange = forceResChange || isRepeaterModeChange ||
                       ((app.currentSource[i] != SiiSwitchSourceInternalVideo) && (app.newSource[i] == SiiSwitchSourceInternalVideo)) ||
                       ((app.currentSource[i] == SiiSwitchSourceInternalVideo) && (app.newSource[i] != SiiSwitchSourceInternalVideo));
        // Find out if Zone is connected to the video source (status detection)
        isZoneEnable = (app.newSource[i] == SiiSwitchSourceInternalVideo);
        if (isZoneChange)
        {
            SkAppInternalVideoConfig(i, isZoneEnable);
        }
    }

}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable OSD Background video.
//!
//!             OSD BG Video is 480p blue screen using internal clock source
//!             and required no HDCP. Main use is for OSD menus to be visible
//!             when no active video source is connected.
//-------------------------------------------------------------------------------------------------

static void OsdBackgroundVideoEnable(bool_t isEnable)
{
    if (isEnable)
    {
		// Memorize current source selection and VPG settings
		TxHdcpStop();          // Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
		app.tpgClockSrc = SI_TPG_CLK_XCLK;
		app.tpgVideoPattern = SI_ETPG_PATTERN_BLUE;
		app.tpgVideoFormat = SI_TPG_FMT_VID_480_60;
		app.isTxHdcpRequired[0] = false;    // Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
		app.newSource[0]    = SiiSwitchSourceInternalVideo;
		
#if INC_CEC
	#if ( configSII_DEV_953x_PORTING == 0 )
		SiiCecSetSourceActive(true);
	#endif
#endif
    }
    else
    {
       TxHdcpStop();          //Bug 31574 - Snow appears on TV with Astro on the input port and Unplug and Plug input HDMI cable.
       app.tpgClockSrc = SI_TPG_CLK_MAIN_PIPE;
       app.isTxHdcpRequired[0] = true;
       app.newSource[0] = SiiDrvSwitchStateGet(SiiSwitch_SELECTED_PORT);
#if INC_CEC
       SiiCecSetSourceActive(false);
#endif
    }

    DEBUG_PRINT( MSG_DBG, "OSD BG Mode %s\n", isEnable ? "ON" : "OFF");
}

//------------------------------------------------------------------------------
// Function:    SkAppSwitchPortUpdate
// Description: Calls the appropriate Silicon Image functions for switching
//              to a new port.
//------------------------------------------------------------------------------

void SkAppSwitchPortUpdate ( void )
{
    static int ccnt = 0;

    uint8_t i, numZones = SII_NUM_SWITCH;
#if (SII_NUM_SWITCH > 1)
    bool_t  reenableOutput[SII_NUM_SWITCH] = {false, false};
    bool_t  isZoneSourceChanged[SII_NUM_SWITCH] = {false, false};
    bool_t  isQuitMatrix, isEnterMatrix, isZoneEnableChange;
    bool_t  isAudioOvlChange[SII_NUM_SWITCH] = {false, false};
#else
    bool_t  reenableOutput[SII_NUM_SWITCH] = {false};
    bool_t  isZoneSourceChanged[SII_NUM_SWITCH] = {false};
    bool_t  isZoneEnableChange;
    bool_t  isAudioOvlChange[SII_NUM_SWITCH] = {false};
#endif
    // Force common source selection if in zone tracking mode
    if (SiiDrvSwitchZoneTrackingGet())
    {
        app.newSource[1] = app.newSource[0];
    }

    // Prepare important conditions
    isAudioOvlChange[0] =( (app.currentAudioSource[0] != app.newAudioSource[0])||(app.currentaudExtSource != app.newaudExtSource));
#if (SII_NUM_SWITCH > 1)
    isAudioOvlChange[1] = (app.currentAudioSource[1] != app.newAudioSource[1]);
#endif
    isZoneSourceChanged[0] = (app.currentSource[0] != app.newSource[0]) || isAudioOvlChange[0];
#if (SII_NUM_SWITCH > 1)
#if INC_IPV
    if(!app.ipvPipEnabled)
    	isZoneSourceChanged[1] = (app.currentSource[1] != app.newSource[1]) || isAudioOvlChange[1];
    else
    {
    	isZoneSourceChanged[1] = false;
        numZones = 1;
    }
#else
    isZoneSourceChanged[1] = (app.currentSource[1] != app.newSource[1]) || isAudioOvlChange[1];
#endif
#endif
#if (SII_NUM_SWITCH > 1)
    isQuitMatrix = (app.currentSource[0] != app.currentSource[1]) && (app.newSource[0] == app.newSource[1]);
    isEnterMatrix = (app.currentSource[0] == app.currentSource[1]) && (app.newSource[0] != app.newSource[1]);
#endif

#if (SII_NUM_SWITCH > 1)
    isZoneEnableChange = (app.newZoneEnabled[0] != app.zoneEnabled[0]) || (app.newZoneEnabled[1] != app.zoneEnabled[1]) ;
#else
    isZoneEnableChange = (app.newZoneEnabled[0] != app.zoneEnabled[0]);
#endif

#if (SII_NUM_SWITCH > 1)
    // Re-initialize EDID Tx Component whenever topology changes
    if (isEnterMatrix)
    {
        // Into matrix mode
        SkAppDeviceInitEdidTx(app.isAudioByPass);
    }
    else if (isQuitMatrix)
    {
        // Out of matrix mode
        SkAppDeviceInitEdidTx(app.isAudioByPass);
    }
#endif
    // Check Zone 1 & 2 for change
    for (i = 0; i < numZones; ++i)
    {
        if (isZoneSourceChanged[i])
        {
            reenableOutput[i] = app.newZoneEnabled[i];
            SiiDrvSwitchInstanceSet( i );

            // The new Audio Zone Source is defined by transmitter zone source
            // selection unless the TX zone source is of non-TMDS type
            if (app.newSource[i] >= SiiSwitchSourceInternalVideo)       // Non-TMDS source
            {
                if ( !app.isOsdBackgroundMode )
                {
                    SiiDrvSwitchAudioZoneSourceSet(SiiDrvSwitchZoneTrackingGet() ? app.audioZoneSource[0] : app.audioZoneSource[i]);
                }
                else
                {
                    // Do not change audio zone source if in background mode because this may
                    // cause the video source to be switched to whatever source was associated
                    // with the audio source.
                }
            }
            else
            {
                // If a TMDS source, the audio source is the same as the video source.
                SiiDrvSwitchAudioZoneSourceSet(app.newSource[i]);
            }

            SiiDrvSwitchSourceSelect( app.newSource[i]);

            // Force audio change as well, if Video source has changed
            isAudioOvlChange[i] = true;
        }

        if(isAudioOvlChange[i])
        {
#if INC_IPV
            // In PIP mode make sure audio is inserted from sub pipe, not from main pipe
            if ((app.newAudioSource[i] == SiiSwitchAudioSourceMainPipe) && app.ipvPipEnabled)
            {
                app.newAudioSource[i] = SiiSwitchAudioSourceSubPipe;
            }
#endif
//            // In VPG & PV modes make sure audio is inserted from main pipe, not from sub pipe
//            if ((app.newSource[i] >= SiiSwitchSourceInternalVideo) && (app.newAudioSource[i] == SiiSwitchAudioSourceSubPipe))
//            {
//                app.newAudioSource[i] = SiiSwitchAudioSourceMainPipe;
//            }
        }
    }

    // Enable AV Mute during Internal Video to External Video transition
    {
        bool_t isQuitIv  = ((app.newSource[0] != SiiSwitchSourceInternalVideo)) &&
       ((app.currentSource[0] == SiiSwitchSourceInternalVideo));
        if(isQuitIv)
        {
            // Bug 33551 - Snow/green video appears on TV while port switching
            SiiRepeaterConInstanceSet(i);
            RepeaterConAvMuteSet(AV_MUTE_TX_IN_SWITCH, ON);
        }
    }

    // Do actual switching and inform repeater about the change
#if INC_IPV
    if ( !app.ipvPipEnabled )
#endif
    {
        if ((isZoneSourceChanged[0] ||
#if (SII_NUM_SWITCH > 1)
        		isZoneSourceChanged[1] ||
#endif
        		isZoneEnableChange))
        {
            SiiDrvSwitchConfigureMatrixSwitch( false, false );
            UpdateFinalZoneConf();
        }
    }


    // If either zone was changed there is more work to do
    if ( reenableOutput[0]
#if (SII_NUM_SWITCH > 1)
    		 || reenableOutput[1]
#endif
    		               )
    {
#if INC_IPV
        if ( app.ipvPipEnabled )
        {
            if ( SkAppIpvPipModeEnable( app.pipSource ))
            {
                app.ipvPipEnabled = true;
                app.ipvEnabled = false;
            }
        }
#endif
        if ( !app.autoSwitch )
        {
            // Zone 1 changes get reported via CEC and get displayed on OSD
            if ( reenableOutput[0] )
            {
#if INC_CEC
                if ( ( app.newSource[0] ) < SII_INPUT_PORT_COUNT )
                {
                    SkAppCecInstanceSet( CEC_INSTANCE_AVR );
                    SiiCecAppInputPortChangeHandler(app.newSource[0]);
                }
#endif
                DEBUG_PRINT( MSG_DBG | DBGF_TS, "Switch to source %d complete\n", app.newSource[0] );
            }
        }
        else
        {
            if (( ccnt % 30 ) == 0 )
            {
                DEBUG_PRINT( MSG_DBG, "\n" );
            }
            DEBUG_PRINT( MSG_DBG, "%d", app.newSource[0] );
            ccnt++;
        }

        // Control Internal Video Generator and TXs, if necessary
        SkAppServiceNonHdmiVideo(false);

        app.currentSource[0] = app.newSource[0];
#if (SII_NUM_SWITCH > 1)
        app.currentSource[1] = app.newSource[1];
#endif
        app.currentaudExtSource = app.newaudExtSource;
    }

    // Service Audio switchings
#if (SII_NUM_SWITCH > 1)
    if(isAudioOvlChange[0] || isAudioOvlChange[1])
#else
    if(isAudioOvlChange[0])
#endif
    {
        // Audio Overlay processing
        for (i = 0; i < numZones; ++i)
        {
            if (isAudioOvlChange[i])
            {
                bool_t isTmdsSource = app.currentSource[i] < SiiSwitchSourceInternalVideo;

                SkAppTxInstanceSet(i);
                SiiDrvRxAudioMixInstanceSet(i);

                switch(app.newAudioSource[i])
                {
                    case SiiSwitchAudioSourceMainPipe:
                        SiiDrvTpiAudioRxSource(true, true);
                        // Copy AIF to TX
                        SkAppTxCopyAif(i, true);
                        SiiDrvRxAudioInstanceSet(0);
                        SiiDrvRxAudioFifoRedirSet(true);
                        // Disable bypassing audio packet by audio mixer
                        SiiDrvRxAudioMixPassThroughConfig(false, isTmdsSource);
                        SiiTxInputAudioUnmute();
                        // Switch audio MUX
                        SiiDrvSwitchInstanceSet(i);
                        SiiDrvSwitchAudioSourceSelect(app.newAudioSource[i], app.audioZoneSource[i]);
                        break;

                    case SiiSwitchAudioSourceSubPipe:
                        SiiDrvTpiAudioRxSource(true, false);
                        // Copy AIF to TX
                        SkAppTxCopyAif(i, false);
                        SiiDrvRxAudioInstanceSet(1);
                        SiiDrvRxAudioFifoRedirSet(true);
                        // Disable bypassing audio packet by audio mixer
                        SiiDrvRxAudioMixPassThroughConfig(true, isTmdsSource);

                        SkAppExtAudioInsertConfig(i, false);
                        SiiTxInputAudioUnmute();
                        // Switch audio MUX
                        SiiDrvSwitchInstanceSet(i);
                        SiiDrvSwitchAudioSourceSelect(app.newAudioSource[i], app.audioZoneSource[i]);
                        break;

                    case SiiSwitchAudioSourceExternal:
                        SiiDrvTpiAudioRxSource(false, true);
                        SiiDrvRxAudioInstanceSet(app.currentAudioSource[i] == SiiSwitchAudioSourceSubPipe);
                        SiiDrvRxAudioFifoRedirSet(false);
                        if(isTmdsSource)
                        {
                            // Allow video pass through, but audio will be local
                            SiiDrvRxAudioMixPassThroughConfig(false, true);
                            SkAppExtAudioInsertConfig(i, true);
                        }
                        else
                        {
                            // Both audio and video are local
                            SiiDrvRxAudioMixPassThroughConfig(false, false);
                        }
                        break;

                    case SiiSwitchAudioSourceDefault:  // Bug 33568 - On Remote Control, press audioi mute button, mutes both audio and video.
                        SiiDrvTpiAudioRxSource(false, true);
                        SiiDrvTpiAudioInEnable();
                        SiiDrvRxAudioInstanceSet(app.currentAudioSource[i] == SiiSwitchAudioSourceSubPipe);
                        SiiDrvRxAudioFifoRedirSet(false);
                        if(isTmdsSource)
                        {
                            SiiDrvRxAudioMixPassThroughConfig(true, true);
                            SkAppExtAudioInsertConfig(i, false);
                        }
                        else
                        {
                            SiiDrvRxAudioMixPassThroughConfig(false, false);
                        }
                        // Switch audio MUX
                        SiiDrvSwitchInstanceSet(i);
                        SiiDrvSwitchAudioSourceSelect(app.newAudioSource[i], app.audioZoneSource[0]);
                        break;
                    // Bug 33568 - On Remote Control, press audio mute button, mutes both audio and video.
                    default:
                        SiiDrvTpiAudioRxSource(false, true);
                        SiiDrvTpiAudioInEnable();
                        SiiDrvRxAudioInstanceSet(app.currentAudioSource[i] == SiiSwitchAudioSourceSubPipe);
                        SiiDrvRxAudioFifoRedirSet(false);
                        if(isTmdsSource)
                        {
                            SiiDrvRxAudioMixPassThroughConfig(false, true);
                            SkAppExtAudioInsertConfig(i, false);
                        }
                        else
                        {
                            SiiDrvRxAudioMixPassThroughConfig(false, false);
                        }
                        // Switch audio MUX
                        SiiDrvSwitchInstanceSet(i);
                        SiiDrvSwitchAudioSourceSelect(app.newAudioSource[i], app.audioZoneSource[0]);
                        break;

                }
            }
        }

        app.currentAudioSource[0] = app.newAudioSource[0];
#if (SII_NUM_SWITCH > 1)
        app.currentAudioSource[1] = app.newAudioSource[1];
#endif
    }

    if ( isZoneEnableChange )
    {
        app.zoneEnabled[0] = app.newZoneEnabled[0];
#if (SII_NUM_SWITCH > 1)
        app.zoneEnabled[1] = app.newZoneEnabled[1];
#endif
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Initialize both instances of the Matrix Switch
//-------------------------------------------------------------------------------------------------

bool_t SkAppDeviceInitMsw ( void )
{

    SiiDrvSwitchInstanceSet( 0 );
    SiiDrvSwitchInitialize();
#if (SII_NUM_SWITCH > 1)
    SiiDrvSwitchInstanceSet( 1 );
    SiiDrvSwitchInitialize();
#endif
    return( true);
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskSwitch
// Description: Switch control task.  Emulate a task with a blocking semaphore
//              waiting for a device interrupt.
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskSwitch ( void )
{
    uint8_t     ifMpFlags;
    uint8_t     ifSpFlags;
    uint16_t    statusFlags;
    uint8_t     portConnFlags;
    uint8_t		selectedPort;

    // Check the application Switch interrupt flag to see if
    // anything happened that involves the switch
    if ( app.switchInterrupt )
    {
        statusFlags = SiiDrvSwitchStatus( SiiSWITCH_INTFLAGS );

    	if(statusFlags & SiiSWITCH_CABLE_CHANGE)
    	{
    		SiiDrvSwitchPortsProcessTermCtrl();
    		portConnFlags = SiiDrvSwitchStateGet(SiiSwitch_INPUT_PORTS_CONNECT_STATE);
    		DEBUG_PRINT( MSG_DBG, "Rx Port Cable: %d%d%d%d\n", (portConnFlags & BIT3) != 0, (portConnFlags & BIT2) != 0,
    		                                                   (portConnFlags & BIT1) != 0, (portConnFlags & BIT0) != 0);

			TRACE_DEBUG((0, "Rx Port Cable: %d%d%d%d", (portConnFlags & BIT3) != 0, (portConnFlags & BIT2) != 0,
														(portConnFlags & BIT1) != 0, (portConnFlags & BIT0) != 0));
#if INC_RTPI
    		SiiRtpiPortConnect(portConnFlags);
#endif

        if(!SiiTxIsHotplugDetected() || !SiiTxIsRxSensed())
        {
            // reset Tx HPD status to prevent unstable interupt if Tx disconnect and Rx connected, kaomin
            SkAppTxHpdConnection(ON);
            SkAppTxHpdConnection(OFF);
            SiiTxSetDsConnect(false); // to run Tx HPD and RxSense scnario, kaomin
        }

    	}

        // Info frame change checks
        if ( SiiDrvSwitchIfInterruptStatusGet( &ifMpFlags, &ifSpFlags ))
        {
            if ( ifMpFlags & (SiiSwitch_NO_AIF | SiiSwitch_NO_AVI | SiiSwitch_NO_VSI))
            {
                DEBUG_PRINT( MSG_DBG, "MP: Missing InfoFrames: %02X\n", ( ifMpFlags & (SiiSwitch_NO_AIF | SiiSwitch_NO_AVI | SiiSwitch_NO_VSI)));
				TRACE_DEBUG((0, "MP: Missing InfoFrames: 0x%X", ( ifMpFlags & (SiiSwitch_NO_AIF | SiiSwitch_NO_AVI | SiiSwitch_NO_VSI)) ));
            }
            if ( ifMpFlags & (SiiSwitch_NEW_AIF | SiiSwitch_NEW_AVI | SiiSwitch_NEW_VSI))
            {
                //DEBUG_PRINT( MSG_ALWAYS, "VSIF new Frames recognised\n");
                DEBUG_PRINT( MSG_DBG, "MP: Changed InfoFrames: %02X\n", ( ifMpFlags & (SiiSwitch_NEW_AIF | SiiSwitch_NEW_AVI | SiiSwitch_NEW_VSI)));
				TRACE_DEBUG((0, "MP: Changed InfoFrames: 0x%X", ( ifMpFlags & (SiiSwitch_NEW_AIF | SiiSwitch_NEW_AVI | SiiSwitch_NEW_VSI)) ));
                SkAppTxInfoFrameUpdate(true, ifMpFlags & SiiSwitch_NEW_AVI, ifMpFlags & SiiSwitch_NEW_AIF,  ifMpFlags & SiiSwitch_NEW_VSI);
            }
            if ( ifSpFlags & (SiiSwitch_NEW_AIF | SiiSwitch_NEW_AVI | SiiSwitch_NEW_VSI))
            {
                DEBUG_PRINT( MSG_DBG, "SP: Changed InfoFrames: %02X\n", ( ifSpFlags & (SiiSwitch_NEW_AIF | SiiSwitch_NEW_AVI | SiiSwitch_NEW_VSI)));
				TRACE_DEBUG((0, "SP: Changed InfoFrames: 0x%X", ( ifSpFlags & (SiiSwitch_NEW_AIF | SiiSwitch_NEW_AVI | SiiSwitch_NEW_VSI)) ));
#if INC_IPV
                // SWWA: 23593 - Reset IPV scaler when AVI in sub-pipe changes
                if (app.ipvPipEnabled)
                {
                    // Re-enabling restarts down-scaler
                    SiiIpvRefresh();
                }
#endif
                SkAppTxInfoFrameUpdate(false, ifSpFlags & SiiSwitch_NEW_AVI, ifSpFlags & SiiSwitch_NEW_AIF,  ifSpFlags & SiiSwitch_NEW_VSI);
            }
			if( (ifMpFlags & SiiSwitch_NO_AVI) || (ifMpFlags & SiiSwitch_NEW_AVI))
			{
				uint8_t avi[18], len;
				len = sizeof(avi);
				SiiDrvSwitchInfoFrameGet(1,IF_AVI,avi,&len);
				// avi[4] bit6, bit5 for Y1 and Y0 of color value
				//    2'b00 = RGB
				//    2'b01 = YCbCr422
				//    2'b10 = YCbCr444
				//    2'b11 = Reserved
				//
				// avi[8] is the VIC value
				//    Low frame rate VICs are
				//    60: 720p 24Hz
				//    61: 720p 25Hz
				//    32: 1080p 24Hz
				//    33: 1080p 25Hz
				// setup TPI R0 delay setting
			//	TxVideoBlankingLevelsSet( (((avi[4]>>5) & 0x03) != 0)? 1 : 0, 1); // always full range. because we have bug in blank register setting

				//SkAppHdcpR0DelaySetting( avi[7] );
				SiiDrvTpiHdcpR0CalcConfigSet(false);

			}
        }

        app.switchInterrupt = false;
    }

    if(app.resChangeIntr[0]) //MP
    {
        UpdateResInfo( 0, app.newSource[0] );
        //UpdateResInfo( 1, app.newSource[1] );
        app.resChangeIntr[0] = false;
    	app.isUserInfoEnabled = 1;
    	TxHdcpRequestReauthentication();  //Bug 31574 - Snow appears on TV with Astro on the input port and Unplug and Plug input HDMI cable.
#if INC_OSD
        SkAppOsdShowInfo();
        if ( app.isOsdMenuEnabled )
        {
            // Conditions may have changed for currently displayed menu
            // so check for updates.
            SkAppOsdMenuUpdate();
        }
#endif
    }

#if ( configSII_DEV_953x_PORTING == 1 ) /*Smith fixes bug: For USB playback is not smoothly output. */
	selectedPort = SiiPlatformReadRotary( true );
#else
	selectedPort = SiiDrvSwitchStateGet(SiiSwitch_SELECTED_PORT);
#endif 	

    // Check if OSD background is required in MP
    if(!SiiDrvSwitchStateGet(SiiSwitch_SELECTED_INPUT_CONNECT_STATE) && !SkAppCbusIsMhlCableConnected(selectedPort))
    {
        // Only apply OSD BG mode if video source selection is of TMDS type
        if (app.currentSource[0] < SiiSwitchSourceInternalVideo)
        {
#if 1 /*Smith comment: When RX cable is removed, blue internal blue pattern 480p will show on screen*/
			app.isOsdBackgroundMode = true; 
			OsdBackgroundVideoEnable( true ); 
#else/* Smith modify: it is always polling rx port, that will make USB playback is not smoothly*/
            app.isOsdBackgroundMode = false;	//Elvis modified: Closed OSD mode for support no OSD
            OsdBackgroundVideoEnable(false);	//Elvis modified: No OSD, so no need to be active source
#endif

        }
        // For IV mode, only switch to OSD BG if APLL1/720p mode is in use
        else if ((app.currentSource[0] == SiiSwitchSourceInternalVideo) && (app.tpgClockSrc == SI_TPG_CLK_P0))
        {
            app.isOsdBackgroundMode = true;
            OsdBackgroundVideoEnable(true);
            // Change source to TMDS port, this will trigger OSD BG mode update.
            // Also, the IV mode will be off when port clock returns
            app.newSource[0] = SkAppSwitchGetNextPort(1, false);
        }
    }
    else
    {
    	if((selectedPort == SI_MHL_PORT0) || (selectedPort == SI_MHL_PORT1))
		{
    		if(SkAppCbusIsMhlCableConnected(selectedPort))
    		{
				if(SkAppSourceIsConnectedCbusPort(selectedPort) && app.isOsdBackgroundMode)
				{
					// Quit OSD background mode
					app.isOsdBackgroundMode = false;
					OsdBackgroundVideoEnable(false);
				}
				if(!SkAppSourceIsConnectedCbusPort(selectedPort) && !app.isOsdBackgroundMode)
				{
					// Quit OSD background mode
					app.isOsdBackgroundMode = true;
					OsdBackgroundVideoEnable(true);
				}
    		}
    		else if(app.isOsdBackgroundMode)
			{
				// Quit OSD background mode
				app.isOsdBackgroundMode = false;
				OsdBackgroundVideoEnable(false);
			}
		}
    	else if(app.isOsdBackgroundMode)
        {
            // Quit OSD background mode
            app.isOsdBackgroundMode = false;
            OsdBackgroundVideoEnable(false);
        }
    }
}

void SkAppHdcpR0DelaySetting( uint8_t vic )
{

	//DEBUG_PRINT(MSG_DBG,"\n\n\r SkAppHdcpR0DelaySetting = vic:%x\n\n\r",vic);
	//	  60: 720p 24Hz
	//	  61: 720p 25Hz
	//	  32: 1080p 24Hz
	//	  33: 1080p 25Hz

	/*fix iar building issue: 
		Internal Error: [CoreUtil/General]: Size mismatch for "        MOVS     R0,#+0", inserted as 2 bytes, assembled as 4 bytes
		Error while running C/C++ Compiler	
	*/
#if ( configSII_DEV_953x_PORTING == 1 ) /*that is for fix building issue*/ 
	if ( vic == 0 )
	{
		SiiDrvTpiHdcpR0CalcConfigSet(false);
	}
	else if( ( vic > 0 ) && ( vic <= 31 ) )
	{
		SiiDrvTpiHdcpR0CalcConfigSet(true);
	}
	else
	{
		SiiDrvTpiHdcpR0CalcConfigSet(false);
	}
#else
	if( vic == 0 ){ // in case of DVI or HDMI 4k video, set to absolute time.
		SiiDrvTpiHdcpR0CalcConfigSet(false);
		return;
	}
	if( vic<=31 ){ // most 60Hz and 50Hz
		SiiDrvTpiHdcpR0CalcConfigSet(true);
		return;
	}
	if( vic<=34 ){ // 1080p 24, 25, 30 Hz
		SiiDrvTpiHdcpR0CalcConfigSet(false);
		return;
	}
	if( vic<=59 ){ // high frequencies
		SiiDrvTpiHdcpR0CalcConfigSet(false);
		return;
	}
	if( vic<=62 ){ // low frequency staff
		SiiDrvTpiHdcpR0CalcConfigSet(false);
		return;
	}
	if( vic<=64 ){ // high frequency staff
		SiiDrvTpiHdcpR0CalcConfigSet(false);
		return;
	}
	// others
	SiiDrvTpiHdcpR0CalcConfigSet(false);
	return;
#endif 	
}

void checkMhlRsense(void)
{
	if(SiiDrvSwitchSelectedPorttype() == SiiPortType_MHL )
	{
		if( (app.isresolutionVaild == true))
		{
			SiiDrvRepeaterInstanceSet(0);
			//DEBUG_PRINT(MSG_DBG,"\n\n\r @@@@@@@@@\n\n\r");
			if(SiiDrvRepeaterRxSyncIsOn())
			{
			//	DEBUG_PRINT(MSG_DBG,"!!!!");
				if (!SiiDrvTpiIsPoweredUp())
				{
					///DEBUG_PRINT(MSG_DBG,"######\n\n\r");
					SiiDrvTpiPowerUp();
					SiiDrvTpiInterruptsEnable(true);
					SiiDrvTpiTmdsOutputEnable(true);
				}
				else if(SiiDrvTmdsIsPoweredUp())
				{
					SiiDrvTpiInterruptsEnable(true);
					SiiDrvTpiTmdsOutputEnable(true);
				}
			}
		}
	}
}
