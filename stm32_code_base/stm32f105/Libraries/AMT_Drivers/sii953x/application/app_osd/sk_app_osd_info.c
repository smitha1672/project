//***************************************************************************
//!file     sk_app_osd_info.c
//!brief    OSD INFO display implementation
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "sk_app_osd.h"
#include "si_osal_timer.h"
#include "si_timer.h"

//-------------------------------------------------------------------------------------------------
// Enumeration and defines
//-------------------------------------------------------------------------------------------------

char *pSourceNames [] =
{
    "HDMI1",
    "HDMI2",
    "HDMI3",
    "HDMI4",
    "             ",
};
//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------

SiiInfoWinStruct_t infoWin;


//-------------------------------------------------------------------------------------------------
//! @brief      Display the system OSD clock
//! @param[in]  winRef      Window reference value for Info Display Window
//-------------------------------------------------------------------------------------------------
static void DisplayClockInfo( int winRef, int row  )
{
    char            clockString [40];
    int             clockHour, hours, minutes, seconds;
    clock_time_t    diffMs, msCount;

    // Clock set: convert requested hours/minutes/ampm to ms since the previous midnight
    //            subtract the calculated value from the current ms and save to start clock value

    msCount = SiiPlatformTimerSysTicksGet();
    diffMs = SkTimeDiffMs( infoWin.clockSetMs, msCount );

    seconds     = ( diffMs / 1000 );
    seconds     = seconds % ( 60 * 60 * 24 );
    hours       = seconds / 3600;
    minutes     = (seconds % 3600) / 60;
    clockHour   = ( infoWin.twentyFourHourClock ) ? hours : (hours > 12) ? (hours - 12) : hours;

    SiiSprintf(
        clockString, "%2d:%02d %s",
        clockHour, minutes,
        ( infoWin.twentyFourHourClock ) ? "  " : (hours >= 12) ? "PM" : "AM" );
    SiiOsdWriteText( winRef, row, 1, clockString, OPT_CLEAR_TO_END );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Determine current info values and display on passed window
//! @param[in]  winRef      Window reference value for Info Display Window
//-------------------------------------------------------------------------------------------------
static void DisplayZoneInfo( int winRef, int row, int zone )
{
    int     hRes, vRes;
    bool_t  isInterlaced;
    char outString [40];

#if ( configSII_DEV_9535 == 1 )
    if ( app.currentSource[zone] <= SiiSwitchSource3 )
#else
    if ( app.currentSource[zone] <= SiiSwitchSource2 )
#endif
    {
        SiiDrvSwitchPortInfoGet( app.currentSource[zone], true, NULL, &hRes, &vRes, &isInterlaced, NULL );
    }
    else
    {
        SiiDrvSwitchVideoFmtParse( app.currentVideoFormat[zone], &hRes, &vRes, &isInterlaced );
    }

    SiiSprintf(
        outString,
        " %-9s          %4d%c",
        pSourceNames[ app.currentSource[zone]],
        vRes,
        isInterlaced ? 'i' : 'p'
        );
    SiiOsdWriteText( winRef, row, 1, outString, OPT_CLEAR_TO_END );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Create the text for the info display
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
bool_t UserFuncInfoDisplay ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t  result  = true;
    int     row     = 0;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:

            if ( app.zoneEnabled[0] )
            {
                DisplayZoneInfo( winRef, row++, 0 );
            }
            else
            {
                SiiOsdWriteText( winRef, row++, 1, "Zone 1: Disabled", OPT_CLEAR_TO_END );
            }
#if SII_NUM_TX > 1
            if ( app.zoneEnabled[1] )//zone 2 is not required for melbourne..
            {
                DisplayZoneInfo( winRef, row++, 1 );
            }
            else
            {
                SiiOsdWriteText( winRef, row++, 1, "Zone 2: Disabled", OPT_CLEAR_TO_END );
            }
#endif
            DisplayClockInfo( winRef, ++row );
            break;
        case UFA_CLOSE:
        case UFA_SELECT:
        case UFA_CANCEL:

            // If showing this window as part of the menu demo, restore
            // the original window configuration.
            if ( app.isOsdMenuEnabled )
            {
                SkAppOsdRestoreOsdDemoMenu( winRef );
                result = false;
            }
            infoWin.winRef = -1;
            break;
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief  Display the current system information on-screen
//-------------------------------------------------------------------------------------------------
void SkAppOsdShowInfo ( void )
{
    if ( !app.isOsdMenuEnabled )
    {
    	if( ( infoWin.winRef != -1)&&(app.isUserInfoEnabled == 1))
    	{
    		  SiiOsdCloseWindow( infoWin.winRef, OPT_CANCEL|OPT_FORCE_CLOSE );
    		  app.isUserInfoEnabled = 0;
    	}

        if (( infoWin.winRef == -1 ) && (( app.currentSource[0] != 0xFF ) || ( app.currentSource[1] != 0xFF )))
        {
            // Open window at top left on display
            infoWin.winRef = SiiOsdCreateWindow(
                RID32_WINDOW21_SI_UF_INFO,
                OSD_DEFAULT_FONT_WIDTH * 3,
                OSD_DEFAULT_FONT_HEIGHT * 2,
                0, 0, NULL, OPT_SHOW | OPT_TOP );
            if ( infoWin.winRef >= 0 )
            {
                SiiOsdSetFocus( portWin.winRef, OPT_SEND_UFA_FOCUS );
                SiiOsdEnableOsdDisplay( true );
            }
        }
    }
}

