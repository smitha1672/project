//***************************************************************************
//!file     si_osd_win_slider.c
//!brief    Silicon Image OSD slider/progress window.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_debug.h"
#include "si_osd_internal.h"
#include "si_osd_resource.h"

#if INC_OSD_SLIDER

#define SLIDER_MIN  pWin->param1
#define SLIDER_MAX  pWin->param2
#define SLIDER_STEP pWin->param3
#define SLIDER_VAL  pWin->param4

//-------------------------------------------------------------------------------------------------
//! @brief      Update the thumb position of the slider/progress bar window
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  thumbValue  new thumb position value.
//-------------------------------------------------------------------------------------------------
static void DrawSlider( int winRef, int thumbValue )
{
    SiiOsdWindow_t  *pWin;
    int             thumbChar, col, barLength, ticksPer, thumbCol, thumbTick, barRow;
    float           unitsPerTick;
    char            barBuffer[ OSD_WIN_COL_COUNT + 1];

    pWin = &pOsd->winData[ winRef];

    memset( barBuffer, SII_OSD_CHAR_SLIDER_BAR, sizeof( barBuffer));
    barBuffer[sizeof( barBuffer) - 1] = 0;

    // Check options
    if ( pWin->options & OPTW_SHOW_ENDCAPS )
    {
        col = 1;
        barLength = pWin->colCount - 2;
        barBuffer[ 0] = SII_OSD_CHAR_SLIDER_CAP_LEFT;
        barBuffer[ pWin->colCount - 1] = SII_OSD_CHAR_SLIDER_CAP_RIGHT;
    }
    else
    {
        col = 0;
        barLength = pWin->colCount;
    }

    // Make room for thumb value at end of bar if needed
    barLength = (pWin->options & OPTW_TEXT_RIGHT) ? barLength - 3 : barLength;

    ticksPer = 1;
    if ( pWin->options & ( OPTW_2_TICK | OPTW_4_TICK ))
    {
        ticksPer    = ( pWin->options & OPTW_2_TICK ) ? 2 : ( pWin->options & OPTW_4_TICK ) ? 4 : 1;
    }
    unitsPerTick    = ((pWin->param2 - pWin->param1) + 1) / (float)(barLength * ticksPer);
    thumbTick   = thumbValue / unitsPerTick;
    thumbCol    = thumbTick / ticksPer;
    thumbCol    = ( pWin->options & OPTW_SHOW_ENDCAPS ) ? thumbCol + 1 : thumbCol;
    thumbChar   = ( pWin->options & OPTW_PROGRESS_BAR ) ? SII_OSD_CHAR_PROGRESS_1QUARTER : SII_OSD_CHAR_THUMB_1QUARTER;
    thumbChar   += (thumbTick % ticksPer);
    if ( pWin->options & OPTW_PROGRESS_BAR )
    {
        for ( ; col <= barLength; col++ )
        {
            if ( col < thumbCol )
            {
                barBuffer[ col] = SII_OSD_CHAR_PROGRESS_FULL;
            }
            else if ( col == thumbCol )
            {
                barBuffer[ col] = thumbChar;
            }
            else
            {
                barBuffer[ col] = SII_OSD_CHAR_SLIDER_BAR;
            }
        }
    }
    else
    {
        for ( ; col <= barLength; col++ )
        {
            if ( col != thumbCol )
            {
                barBuffer[ col] = SII_OSD_CHAR_SLIDER_BAR;
            }
            else
            {
                barBuffer[ col] = thumbChar;
            }
        }
    }

    barRow = pWin->rowCount - 1;    // Defaults to bottom row
    barRow = (pWin->options & OPTW_TEXT_BOTTOM) ? (barRow - 1) : barRow;
    barBuffer[ pWin->colCount] = 0;     // Terminate string.
    SiiDrvOsdWriteText( winRef, barRow, 0, barBuffer, 0 );

    // Now print value text if specified
    if ( pWin->options & (OPTW_TEXT_TOP | OPTW_TEXT_BOTTOM | OPTW_TEXT_RIGHT))
    {
        memset( barBuffer, ' ', sizeof( barBuffer));
        barBuffer[sizeof( barBuffer) - 1] = 0;

        // Determine value row placement
        barRow = (pWin->options & OPTW_TEXT_TOP) ? barRow - 1 : ((pWin->options & OPTW_TEXT_BOTTOM) ? barRow + 1 : barRow);

        // Convert values and print. Assumes value is less than 999 (three decimal digits)
        SiiSprintf( barBuffer, "%-3d", thumbValue );
        SiiDrvOsdWriteText( winRef, barRow, (pWin->options & OPTW_TEXT_RIGHT) ? barLength : (barLength/2) - 1, barBuffer, 0 );

        // Only print min/max if above or below text is specified
        if ( pWin->options & (OPTW_TEXT_TOP | OPTW_TEXT_BOTTOM ))
        {
            SiiSprintf( barBuffer, "%d", pWin->param1 );
            SiiDrvOsdWriteText( winRef, barRow, 0, barBuffer, 0 );
            SiiSprintf( barBuffer, "%3d", pWin->param2 );
            SiiDrvOsdWriteText( winRef, barRow, barLength - 3, barBuffer, 0 );
        }
    }
    SiiDrvOsdSwapSurfaces();    // The equivalent of OPT_SHOW on the WriteText calls.
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the current position of the Slider thumb.
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @return     Value between the MIN_VALUE and MAX_VALUE specified at create time or -1 if window
//!             not enabled.
//-------------------------------------------------------------------------------------------------
int SiiOsdSliderValueGet( int winRef )
{
    // Ignore window if not enabled.
    if ((winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
        return( -1 );
    }
    pOsd->lastResultCode = SII_OSD_SUCCESS;
    return( pOsd->winData[ winRef].param4 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Update the thumb position of the slider/progress bar window
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  thumbValue  new thumb position value.
//! @param[in]  options     OPT_SHOW, OPT_HIDE
//! @return     Returns result of window user function
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSliderUpdate( int winRef, int thumbValue, SiiOsdOptions_t options )
{
    SiiOsdWindow_t  *pWin;
    bool_t          success;

    // Ignore window if not enabled.
    if ((winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
        return( false );
    }
    pOsd->lastResultCode = SII_OSD_SUCCESS;

    pWin = &pOsd->winData[ winRef];

    // Call the window function with the UPDATE action
    pWin->param4    = thumbValue;
    success = OsdExecuteUserFunction( winRef, UFA_UPDATE, -1 );
    if ( success )
    {
        DrawSlider( winRef, thumbValue );

        // Display if requested
        if ( options & OPT_SHOW )
        {
            SiiOsdShowWindow( winRef, options );
        }
    }
    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Manage a Slider window
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  key         SIIKEY_SELECT, SIIKEY_CANCEL, SIIKEY_CURSOR_xxx.
//! @param[in]  options     none
//! @return     true if key was used, false if not.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSliderHandler( int winRef, int key, SiiOsdOptions_t options )
{
    bool_t          userFuncResult = false;
    bool_t          keyUsed = true;
    SiiOsdWindow_t  *pWin;

    // Ignore window if not enabled.
    if (( winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
        return( false );
    }
    pOsd->lastResultCode    = SII_OSD_SUCCESS;
    pWin                    = &pOsd->winData[ winRef];

    if ( winRef == SiiOsdGetFocus())
    {
        userFuncResult = SiiOsdBaseWindowHandler( winRef, key, options );
        switch ( key )
        {
            case SIIKEY_CURSOR_UP:
            case SIIKEY_CURSOR_DOWN:
                break;
            case SIIKEY_CURSOR_LEFT:
            case SIIKEY_CURSOR_RIGHT:
                // Increment/decrement with limits
                SLIDER_VAL += (key == SIIKEY_CURSOR_RIGHT) ? SLIDER_STEP : -SLIDER_STEP;
                if ( SLIDER_VAL < SLIDER_MIN )
                {
                    SLIDER_VAL = SLIDER_MIN;
                }
                if ( SLIDER_VAL > SLIDER_MAX )
                {
                    SLIDER_VAL = SLIDER_MAX;
                }
                DrawSlider( winRef, SLIDER_VAL );
                OsdExecuteUserFunction( winRef, UFA_UPDATE, key );

                // If window has a timer, a key action will restart it.
                if ( pWin->msTimeout )
                {
                    SiiOsTimerSet( &pWin->winTimer, pWin->msTimeout );
                }
                break;

            case SIIKEY_CANCEL:
            case SIIKEY_SELECT:
                // Handled completely by the SiiOsdBaseWindowHandler function
                break;
            default:
                keyUsed = false;
                break;
        }
    }
    return( keyUsed );
}

#endif // if INC_OSD_SLIDER
