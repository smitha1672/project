//***************************************************************************
//!file     sk_app_ipv.c
//!brief    Application InstaPrevue control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "sk_app_ipv.h"
#include "si_drv_ipv.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Move selection to the next or previous IPV window.
//!
//! @param[in]  isForward - select next input (true) or previous input (false)
//-------------------------------------------------------------------------------------------------

void SkAppIpvCycleInstaPrevueWindows ( bool_t isForward )
{
    if ((app.currentSource[0] >= SiiSwitchSourceInternalVideo) && (app.currentSource[1] >= SiiSwitchSourceInternalVideo))
    {
        // Only TMDS source can have IPV overlay
        return;
    }

    // If not enabled, enable InstaPrevue
    if ( !app.ipvEnabled )
    {
        app.ipvHighlightWindow = 0;
        if ( SkAppIpvEnable( false ))  // In case PIP was enabled
        {
            app.ipvPipEnabled = false;   // Bug 33246 - Melbourne SiI9535 PIP | selected video is NOT displayed
            SiiIpvModeSet( app.ipvMode );
            SiiIpvThemeSet( app.ipvTheme );
            (void)SkAppIpvEnable( true );   // Don't need to check return value here
            app.ipvEnabled = true;
            app.ipvHighlightWindow = SiiIpvHighlightWindow( app.ipvHighlightWindow, isForward );
        }
    }
    // Otherwise, cycle through the highlight window selection
    else
    {
        if (isForward)
        {
            app.ipvHighlightWindow++;
            if ( app.ipvHighlightWindow >= IPV_WINDOW_COUNT )
            {
                app.ipvHighlightWindow = 0;
            }
        }
        else
        {
            if ( app.ipvHighlightWindow == 0 )
            {
                app.ipvHighlightWindow = IPV_WINDOW_COUNT;
            }
            else
            {
                app.ipvHighlightWindow--;
            }
        }
        app.ipvHighlightWindow = SiiIpvHighlightWindow( app.ipvHighlightWindow, isForward );
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable IPV wrapper.
//-------------------------------------------------------------------------------------------------
bool_t SkAppIpvEnable ( bool_t isEnable )
{
    bool_t success;

    if (!isEnable)
    {
#if (SII_NUM_SWITCH > 1)
        if ( app.ipvPipEnabled )
        {
            // Restore original zone 1 video source
            SiiDrvSwitchInstanceSet( 1 );
            SiiDrvSwitchSourceSelect( app.pipSaveOrigSource );
        }
#endif
        SiiIpvModeSet( app.ipvMode );   // Go back to a roving mode (if we were in PIP mode)
    }
    success = SiiIpvEnable(isEnable);
    if ( isEnable )
    {
        // Need time for at least one preview window to activate, otherwise we will
        // start out with no windows highlighted.
        SiiOsTimerWait(500);
    }
    return success;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable PIP wrapper.
//-------------------------------------------------------------------------------------------------

bool_t SkAppIpvPipModeEnable ( SiiSwitchSource_t pipSource )
{
    bool_t originalZoneTrackingState;

    SiiDrvSwitchInstanceSet( 0 );
    if ( pipSource == SiiDrvSwitchSourceGet())
    {
        DEBUG_PRINT( MSG_ERR, " Cannot use the main pipe source!\n" );
        return( false );
    }
   // DEBUG_PRINT(MSG_ALWAYS,"\nSTAGE 2\n");

    originalZoneTrackingState = SiiDrvSwitchZoneTrackingGet();
    SiiDrvSwitchZoneTrackingSet( false );   // Allow zones to use different sources
    SiiDrvSwitchZoneEnableSet( true );      // Make sure zone 1 is enabled

#if (SII_NUM_SWITCH > 1)
    SiiDrvSwitchInstanceSet( 1 );
    if ( !app.ipvPipEnabled )
    {
        app.pipSaveOrigSource = SiiDrvSwitchSourceGet();
    }

    // Select the source configuration for the secondary pipe
    SiiDrvSwitchZoneEnableSet( true );      // Make sure zone 2 is enabled
    SiiDrvSwitchSourceSelect( pipSource );
#endif
    SiiDrvSwitchConfigureMatrixSwitch( true, false );

    SiiDrvIpvPIPSourceSet((uint8_t)pipSource);

    SiiIpvPipModeEnable();

    // Apply new port selection settings
    SiiDrvSwitchZoneConfigSet();
    SiiDrvSwitchZoneTrackingSet( originalZoneTrackingState );

    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Close all IPV windows including the PIP window.
//-------------------------------------------------------------------------------------------------

void SkAppIpvDisableInstaPrevue ( void )
{
    if (app.ipvEnabled)
    {
        app.ipvEnabled = false;
        if ( SkAppIpvEnable( false ))
        {
            // If PIP previously enabled, restore it
            if ( app.ipvPipEnabled )
            {
                // PIP source cannot be the same as main pipe.
                if ( app.pipSource == app.currentSource[0] )
                {
                    app.pipSource = SkAppSwitchGetNextPort( app.currentSource[0], true );
                }
                SkAppIpvPipModeEnable( app.pipSource );
                SkAppIpvEnable( true );
            }
        }
    }
    else
    {
        // If PIP previously enabled, close it
        if ( app.ipvPipEnabled )
        {
            SkAppIpvEnable( false );
            app.ipvPipEnabled = false;
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Initialize IPV Task
//-------------------------------------------------------------------------------------------------
bool_t SkAppTaskIpvInitialize ( void )
{
    SiiIpvInitialize();

    app.ipvTheme = SiiIpvLEFT_MIDDLE;

    // Initialize the component window data
    SiiIpvBorderColorsLoad( siiIpvBorderColorsCrCbY );
    SiiIpvAlphaLoad( siiIpvAlphaMax, siiIpvAlphaStep, SII_IPV_ALPHA_STEP_MS );

#if ( configSII_DEV_9535 == 0 )
    SiiDrvIpvEnableMaskSet( 0x03 );
#else
    SiiDrvIpvEnableMaskSet( 0x07 );
#endif
    // Set the default theme (window locations and size) into the hardware.
    app.ipvMode = SiiIpvModeALL;
    SiiIpvModeSet( SiiIpvModeALL );
    SiiIpvHighlightEnable( true );  // Default to enabled preview window highlighting.
    app.ipvHighlight = true;
    return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      IPV Task function. Services IPV component state machine.
//-------------------------------------------------------------------------------------------------
void SkAppTaskIpv ( void )
{
    clock_time_t timeDiff = SkTimeDiffMs( app.ipvTimeLastMs, SiiOsTimerTotalElapsed());
    if (timeDiff > 20)
    {
        // Call IPV handler with the number of milliseconds since the last call
        SiiIpvHandler(timeDiff);
        app.ipvTimeLastMs = SiiOsTimerTotalElapsed();
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      IPV Callback function. Retrieves current video resolution data.
//-------------------------------------------------------------------------------------------------

bool_t SiiIpvCbVideoInfoGet(int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced)
{
    return SiiDrvSwitchSelectedPortInfo( true, pHoriz, pVert, pIsInterlaced, NULL );
}
