//***************************************************************************
//!file     sk_app_tv.c
//!brief    SK 9535 Starter Kit firmware TV emulation
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_platform.h"
#include "si_gpio.h"

#include "si_drv_device.h"
#include "si_tx_component.h"  // Bug 33568 - On Remote Control, press audioi mute button, mutes both audio and video.

#if INC_CEC
#include "si_cec_component.h"
#endif

#if INC_CBUS
#include "sk_app_cbus.h"
#endif

#if INC_IPV
#include "si_ipv_component.h"
#endif

extern SiiTimer_t testTimer;       	// used in wake-up detection by switching between instances

//------------------------------------------------------------------------------
// Module data
//------------------------------------------------------------------------------

static uint16_t l_autoSwitchDelaysMs [8] =
{
    5000, 10000, 15000, 20000, 30000, 45000, 60000, 3000
};

static char *pCcHelp [] =
{
    tenDashes,
#if ( configSII_DEV_9535 == 1 )
    "1-4   - Select HDMI/MHL Input 1 - 4",
#else
    "1-3   - Select HDMI/MHL Input 1 - 3",
#endif
    "8     - Select Internal Video Source / Change pattern",
    "MUTE  - Blank Video and Audio",
    "GUIDE - Open Text based Menu",
    "MENU   - Open / Close OSD menu",
    "QUIT   - Cancel mode",
    "INPUT  - Select Input via InstaPrevue menu",
    "ENTER  - Change to selected Input",
    "PIP    - PIP with selected source ON/OFF",
    "CHAN+  - InstaPrevue ALL/ACTIVE",
    "CHAN-  - InstaPrevue layout change",
    "LT/RT  - Change Zone source / InstaPrevue selection / PIP source",
    "UP/DWN - Change Zone source / InstaPrevue selection / PIP source",
    NULL
};

#if 0
//------------------------------------------------------------------------------
//! @brief      Handles Remote Control keys for System Audio Control
//! @param[in]  key - RC5 key code
//! @return     true if key was consumed
//------------------------------------------------------------------------------
static bool_t AvrRcSacKeyHandler ( RC5Commands_t key )
{
    uint8_t ceaKey;
    bool_t  keyUsed = false;

    switch ( key )
    {
        case RC5_MUTE:
        case RC5_VOL_UP:
        case RC5_VOL_DOWN:
            ceaKey = SkAppIrConvertToCea931( key );

            // Convert the key to CEA931 and process it locally.
#if INC_CEC_SAC
            if ( SkAppAudioProcessSacCommand( ceaKey ))
            {
#if INC_OSD
                // Display the results locally
                SkAppOsdVolumeControl( key );
#endif
            }
            else
#endif
            {
#if INC_CEC
                // Send to the DTV
                SkAppCecInstanceSet( CEC_INSTANCE_AVR );
                SiiCecSendUserControlPressed( ceaKey, SiiCecGetDeviceLA(), CEC_LOGADDR_TV );
#endif
            }
            keyUsed = true;
            break;
        default:
            break;
    }

    return( keyUsed );
}
#endif
//------------------------------------------------------------------------------
// Function:    SkAppTvIrRemote
// Description: Parse remote control keys for the TV application.
// Parameters:  fullAccess - true if allowed to respond to all normal keys
//                           false if only allowed to respond to power keys
// Returns:     none
//------------------------------------------------------------------------------
void SkAppTvIrRemote ( RC5Commands_t key, bool_t fullAccess )
{
    bool_t  isForward = true;
    bool_t  isZoneTrackingEnabled = false;
#if INC_OSD
    char    tempMsg[40];
#endif

    while ( fullAccess )
    {
        // Handle System Audio Control keys
//        if ( AvrRcSacKeyHandler( key ))
//        {
//            break;
//        }

#if INC_OSD
        // If the OSD uses the key, we're done
        if ( SkAppOsdIrRemote( key, fullAccess ))
        {
            break;
        }
#endif

        // Now handle TV action keys
        switch ( key )
        {
            case RC5_0:
                app.currentZone = 0;//(app.currentZone == 0) ? 1 : 0;
                //SiiSprintf( tempMsg, "Select Zone %d\n", app.currentZone + 1 );
                //DEBUG_PRINT( MSG_ALWAYS, tempMsg );
#if INC_OSD
                //SkAppOsdmessage( "Info", tempMsg, 3000 );
#endif
                break;

            case RC5_1:
            case RC5_2:
            case RC5_3:
#if ( configSII_DEV_9535 == 1 )
            case RC5_4:
#endif
#if INC_IPV
                if ( !app.ipvEnabled && !app.ipvPipEnabled )
#endif
                {
                    app.newSource[app.currentZone] = key - RC5_1;
                    app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceDefault;
                    SiiOsTimerSet( &testTimer, 10000);
                }
                break;
//            case RC5_7:
//                // Toggle Zone 1 Enable status
//                app.newZoneEnabled[0] = !app.zoneEnabled[0];
//#if INC_OSD
//                SiiSprintf( tempMsg, "Zone 1 %s", app.newZoneEnabled[0] ? "Enabled" : "Disabled ");
//                SkAppOsdmessage( "Info", tempMsg, 3000 );
//#endif
//                break;

            case RC5_8:
                // Select internal video source
                if (app.currentSource[app.currentZone] != SiiSwitchSourceInternalVideo)
                {
                    //app.currentZone = 1;
                    app.newSource[app.currentZone] = SiiSwitchSourceInternalVideo;
                    app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceDefault;
                    app.currentAudioType[app.currentZone] = SI_TX_AUDIO_SRC_SPDIF;
                    app.tpgVideoPattern = SI_ETPG_PATTERN_COLORBAR;
                    app.tpgVideoFormat = SI_TPG_FMT_VID_480_60;
                    app.tpgClockSrc = SI_TPG_CLK_XCLK;
                    app.isExtTpg=true;

#if INC_IPV
                    app.ipvPipEnabled = false;
                    SkAppIpvDisableInstaPrevue();
#endif
                    PrintAlways( "IV On\n" );
                }
                else
                {
                    // Repeated TPG selection results in pattern change
                    if (app.tpgVideoPattern < SI_ETPG_PATTERN_COLORBAR)
                    {
                        app.tpgVideoPattern++;
                    }
                    else
                    {
                        app.tpgVideoPattern = SI_ETPG_PATTERN_RED;
                    }
                    SiiDrvExtTpgPatternSet(app.tpgVideoPattern);
                    PrintAlways( "IV Pattern changed.\n" );
                }
                break;

            case RC5_GUIDE:
                // Toggle zone tracking mode
               // SiiDrvSwitchZoneTrackingSet(isZoneTrackingEnabled = !SiiDrvSwitchZoneTrackingGet());
             //   SiiSprintf( tempMsg, "Zone Source: %s\n", isZoneTrackingEnabled ? "SINGLE" : "DUAL");
                //DEBUG_PRINT( MSG_ALWAYS, "\n\nGUIDE PRESSED\n" );
#if INC_OSD
                strcpy(tempMsg, "GUIDE Key Pressed");
                SkAppOsdmessage( "Info", tempMsg, 3000 );
#endif
                break;

            case RC5_STOP:
#if INC_IPV
                if (!app.ipvPipEnabled && !app.ipvEnabled)
#endif
                {
                    // Cancel everything
                    app.currentZone = 0;
                    // Switch to input 0 or any available, if port 0 is not connected
                    app.newSource[0] = app.newSource[1] = SkAppSwitchGetNextPort( 1, false );
                    app.newAudioSource[1] = app.newAudioSource[0] = SiiSwitchAudioSourceDefault;
                    SiiDrvSwitchZoneTrackingSet(isZoneTrackingEnabled = true);
                    PrintAlways( "Reset UI\n" );
#if INC_OSD
                    SkAppOsdmessage( "Info", "Reset UI", 3000 );
#endif
                }
                break;

            case RC5_INFO:
#if INC_OSD
                SkAppOsdShowInfo();
#endif
                break;

            case RC5_QUIT:
#if INC_IPV
                if (app.ipvPipEnabled)
                {
                    SkAppIpvEnable(false);
                    app.ipvPipEnabled = false;
                }

                if (app.ipvEnabled)
                {
                    SkAppIpvDisableInstaPrevue();
                }
#endif
                break;

            case RC5_CURSOR_LEFT:
                isForward = false;
            case RC5_CURSOR_RIGHT:
#if INC_IPV
                if ( app.ipvEnabled )
                {
                    // Change selection
                    SkAppIpvCycleInstaPrevueWindows(isForward);
                }
                else if (app.ipvPipEnabled)
                {
                    // Switch to PIP source
                    app.newSource[0] = app.pipSource;
                    SkAppIpvDisableInstaPrevue();
                }
                else
#endif
                {
                    app.newSource[0] = SkAppSwitchGetNextPort( app.currentSource[0], isForward );
                    app.newAudioSource[0] = SiiSwitchAudioSourceDefault;
                    DEBUG_PRINT( MSG_ALWAYS, "ZONE SRC: %d\n", app.newSource[0]);
                    SiiOsTimerSet( &testTimer, 10000);
                }
                break;

            case RC5_CURSOR_DOWN:
                isForward = false;
            case RC5_CURSOR_UP:
#if INC_IPV
                if ( app.ipvEnabled )
                {
                    // Change selection
                    SkAppIpvCycleInstaPrevueWindows(!isForward);
                }
                else if (app.ipvPipEnabled)
                {
                    // Change PIP source
                    app.pipSource = SkAppSwitchGetNextPort( app.pipSource, isForward );
                    // PIP source cannot be the same as main pipe.
                    if ( app.pipSource == app.currentSource[0] )
                    {
                        app.pipSource = SkAppSwitchGetNextPort( app.pipSource, isForward );
                    }
                    //SkAppIpvEnable( false );
                    SkAppIpvPipModeEnable( app.pipSource );
                }
                else
#endif
                {
                    app.newSource[0] = SkAppSwitchGetNextPort( app.currentSource[0], isForward );
                    app.newAudioSource[0] = SiiSwitchAudioSourceDefault;
                    DEBUG_PRINT( MSG_ALWAYS, "ZONE SRC: %d\n", app.newSource[0]);
                    SiiOsTimerSet( &testTimer, 10000);
                }
                break;

            case RC5_PIP:
#if INC_IPV
                if ( app.ipvPipEnabled )
                {
                    SkAppIpvEnable( false );
                    app.ipvPipEnabled = false;
                    app.ipvEnabled = false;
                }
                else
                {
                    if ( app.ipvEnabled )
                    {
                        // Switch to port highlighted in InstaPrevue
                        app.pipSource = SiiIpvHighlightPortGet( app.currentSource[0], app.ipvHighlightWindow );
                    }
                    else
                    {
                    	app.pipSource = SkAppSwitchGetNextPort( SiiDrvSwitchStateGet(SiiSwitch_SELECTED_PORT), true );
                    }

                    //SkAppIpvEnable( false );

                    if ( SkAppIpvPipModeEnable( app.pipSource ))
                    {
                        app.ipvPipEnabled = true;
                        app.ipvEnabled = false;
                    }
                }
                break;

            case RC5_CH_PLUS:
                if ( app.ipvEnabled && !app.ipvPipEnabled )
                {
                    app.ipvMode = (app.ipvMode == SiiIpvModeACTIVE) ? SiiIpvModeALL : SiiIpvModeACTIVE;
                    if ( SkAppIpvEnable( false ))
                    {
                        SiiIpvModeSet( app.ipvMode );
                        SkAppIpvEnable( true );
                    }
                }
                break;

            case RC5_CH_MINUS:
                if ( app.ipvEnabled && !app.ipvPipEnabled )
                {
                    if ( SkAppIpvEnable( false ))
                    {
                        app.ipvTheme++;
                        if ( app.ipvTheme >= SiiIpvPIP )
                        {
                            app.ipvTheme = 0;
                        }
                        SiiIpvThemeSet( app.ipvTheme );
                        SkAppIpvEnable( true );
                        app.ipvHighlightWindow = SiiIpvHighlightWindow( app.ipvHighlightWindow, true );
                    }
                }
                break;
#endif//#if INC_IPV

            case RC5_INPUT:
                // When we allow InstaPrevue AND we are displaying an HDMI source,
                // the INPUT key shows the preview windows
#if INC_IPV
                if ((app.ipvAllowed) && ( app.currentSource[0] < SiiSwitchSourceInternalVideo))
                {

                    SkAppIpvCycleInstaPrevueWindows(true);
                }
                // Otherwise, use OSD
                else
#endif
                {
#if INC_OSD
                    SkAppOsdPortSelect();
#endif
                }
                break;

#if INC_IPV
            case RC5_ENTER:
                if ( app.ipvPipEnabled )
                {
                    app.newSource[app.currentZone] = app.pipSource;
                    app.pipSource = app.currentSource[app.currentZone];
                    app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceDefault;

                    SkAppIpvEnable( false );
                    app.ipvPipEnabled = false;
                    app.ipvEnabled = false;
                }
                else if ( app.ipvEnabled )
                {
                    // Switch to port highlighted in InstaPrevue.  If no port highlighted,
                    // ignore the request.  The user will have to move to an active window
                    // or cancel (quit).
                    if ( app.ipvHighlightWindow != -1 )
                    {
                        app.newSource[0] = app.newSource[1] = SiiIpvHighlightPortGet(app.currentSource[0], app.ipvHighlightWindow );
                        app.newAudioSource[0] = app.newAudioSource[1] = SiiSwitchAudioSourceDefault;
                        SkAppIpvDisableInstaPrevue();
                    }
                }
                break;
#endif//#if INC_IPV
            case RC5_MUTE:
                // Bug 33568 - On Remote Control, press audioi mute button, mutes both audio and video.
            	if(app.muteON)
            	{
            		app.muteON=false;
            		//DemoTpgModeEnable( false );
            		SiiTxInputAudioUnmute();
                    app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceDefault;
            	}
            	else
            	{
            		app.muteON=true;
                    SiiTxInputAudioMute();
                    app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceCount;
                    //app.isExtTpg=true;
                    //app.tpgVideoPattern= SI_ETPG_PATTERN_BLACK;

                    //DemoTpgModeEnable( true );
            	}
                // Select Internal video source
                //app.newSource[app.currentZone] = SiiSwitchSourceInternalVideo;
                //app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceDefault;
                DEBUG_PRINT( MSG_ALWAYS, "Audio  %s\n", (app.muteON ? "Muted":"Unmuted"));
                break;

#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
                SkAppIrRemoteReturnToTV();
                PrintMenu( pCcHelp );
                break;

            default:
            {
#if INC_CEC
                uint8_t ceaKey = SkAppIrConvertToCea931( key );
                if ( ceaKey != 0xFF )
                {
                    // If the specified port is HDMI, send the CEC key to it
                    if ( app.currentSource[app.currentZone] < SiiSwitchSourceInternalVideo)
                    {
                        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
                        SiiCecSendUserControlPressed( ceaKey, SiiCecGetDeviceLA(), CEC_LOGADDR_TV );
                    }
                }
#endif

#if INC_CEC && INC_CBUS
                SkAppCbusSendRcKey( key );
#endif
                break;
            }
        }
        break;  // Always break;
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppTv
// Description: CP 953x TV emulation control
//------------------------------------------------------------------------------
void SkAppTv ( void )
{
    uint16_t counter = 0;

    if (!SiiOsTimerExpired(app.appTimer))
    {
        return;
    }
    SiiOsTimerSet(&app.appTimer, DEM_POLLING_DELAY); // Reset poll timer
    counter++;

    // Check the UI here rather than in the main loop because we don't need
    // to check it at such a high rate, and besides, if the user changed the
    // rotary switch between polling delays, the next time it was checked
    // it would report unchanged - you have only one chance to get the changed value.
    SkAppUiUpdate();

#if ( configSII_DEV_953x_PORTING == 0 )
    /****************************************************************************************/
    /* Display Port Processor State                                                         */
    /****************************************************************************************/
    SkAppInstaPortLedUpdate(SiiDrvDeviceInfo(SiiDEV_SELECTED_PORT_BITFIELD));
#endif

    /****************************************************************************************/
    /* Rotary Switch handling                                                               */
    /****************************************************************************************/

    if ( app.autoSwitch )
    {
        uint8_t tempPort = SiiPlatformReadRotary(true);

        if (SiiOsTimerExpired(app.autoSwitchTimer))
        {
            SiiOsTimerSet(&app.autoSwitchTimer, l_autoSwitchDelaysMs[tempPort]);

            // Auto-switch zone 1 only
            app.newSource[0] = SkAppSwitchGetNextPort(app.newSource[0], true);
        }
    }
    else if ((app.rotarySwitch != RSW_NO_CHANGE) && !app.rotaryDisabled )
    {
        app.lastRotarySwitch = app.rotarySwitch;
        //  Switch positions 0-3, select new active port on SiI9535

        if (app.rotarySwitch < SII_INPUT_PORT_COUNT)
        {
#if INC_IPV
            if ( app.ipvEnabled )
            {
                // Change selection
                SkAppIpvCycleInstaPrevueWindows(true);
            }
            else
#endif
            {
                // The rotary switch always controls zone 2 only in matrix mode
                // RC controls zone 1 by default
                app.newSource[SiiDrvSwitchZoneTrackingGet() ? 0 : 1] = app.rotarySwitch;
            }
        }
    }
}

//___________________________AMTRAN IMPLEMENTED__________________________________________
void SkAppTv_AMT_RxPortSelect( uint8_t input_path )
{
#if ( configSII_DEV_953x_PORTING == 1 )
	if ( input_path < SII_INPUT_PORT_COUNT )
	{
		app.rotarySwitch = input_path;
	}
#endif 	
}


