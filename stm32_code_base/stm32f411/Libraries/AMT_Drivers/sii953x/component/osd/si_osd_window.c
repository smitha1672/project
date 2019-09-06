//***************************************************************************
//!file     si_osd_window.c
//!brief    Silicon Image OSD Component window control.
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
#include "si_drv_switch.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Common utility function to check winref validity
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @return     true if valid winref, false if an error occurred.
//-------------------------------------------------------------------------------------------------
bool_t OsdIsValidWinRef ( int winRef )
{
    if (( winRef == -1 ) || ( winRef >= SII_OSD_WIN_COUNT ))
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_VALID;
    }
    else
    {
        pOsd->lastResultCode = SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Common utility function to check winref validity
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @return     true if valid winref, false if an error occurred.
//-------------------------------------------------------------------------------------------------
bool_t OsdIsValidActiveWinRef ( int winRef )
{
    if (( winRef == -1 ) || ( winRef >= SII_OSD_WIN_COUNT ))
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_VALID;
    }
    else if ( ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) != 0 )
    {
        pOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
    }
    else
    {
        pOsd->lastResultCode = SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Attach a pointer to a window user function to the window. If a window has a function
//!             pointer attached, it will supersede the windowFunctionId member of the window data
//!             structure associated with the window.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  pFunction   Pointer to window user function.
//! @return     true if successfully attached to window, false if an error occurred.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdAttachFunction( int winRef, SiiOsdUserFunction_t pFunction )
{
    if (!OsdIsValidActiveWinRef( winRef ))  // Ignore window if not enabled.
    {
        return( false );
    }

    pOsd->winData[ winRef].pFunction = pFunction;

    return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Attach a pointer to external data to the window.  The external data is available
//!             to the developer for their own use, although one or more of the complex window
//!             types require external data with a specific structure.
//! @param[in]  winRef          Return value from any SiiOsdCreate function.
//! @param[in]  pExternalData   Pointer to void.
//! @return     true if successfully attached to window, false if an error occurred.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdAttachExternalData( int winRef, void *pExternalData )
{
    if (!OsdIsValidActiveWinRef( winRef ))  // Ignore window if not enabled.
    {
        return( false );
    }

    pOsd->winData[ winRef].pExternalData = (void *)pExternalData;

    return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the private data members of the specified window structure.  This data can be
//!             accessed by the window user function via the SiiFuncArgs_t structure as param1,
//!             param2, param3, param4, and editMode.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  p1          Private data value 1.
//! @param[in]  p2          Private data value 2.
//! @param[in]  p3          Private data value 3.
//! @param[in]  p4          Private data value 4.
//! @return     true if success, false if an error occurred.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSetWindowPrivateData( int winRef, int p1, int p2, int p3, int p4, int editMode )
{
    if (!OsdIsValidActiveWinRef( winRef ))  // Ignore window if not enabled.
    {
        return( false );
    }

    pOsd->winData[ winRef].param1 = p1;
    pOsd->winData[ winRef].param2 = p2;
    pOsd->winData[ winRef].param3 = p3;
    pOsd->winData[ winRef].param4 = p4;
    pOsd->winData[ winRef].editMode = editMode;

    return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the child window reference ID for a window.
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @param[in]  childWinRef New value for child window reference ID.
//! @retval                 Previous value of child winRef
//-------------------------------------------------------------------------------------------------
int SiiOsdAssignChildWinRef( int winRef, int childWinRef )
{
    int oldChildWinRef = -1;

    if ((OsdIsValidWinRef( winRef )) && ( childWinRef != -1 ))
    {
        oldChildWinRef = pOsd->winData[ winRef].childWinRef;
        pOsd->winData[ winRef].childWinRef = childWinRef;
    }
    return( oldChildWinRef );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the color of a specific element of the specified window.
//!             Change takes effect immediately.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  element     OSD_FG, OSD_BG, OSD_FG_HIGHLIGHT, OSD_BG_HIGHLIGHT, OSD_FG_DISABLED,
//!                         OSD_BG_DISABLED, OSD_BORDER, OSD_ROW_DIVIDER
//! @param[in]  rgbVal      OSD_RGB() color value.
//! @retval                 true if successful, false if a failure.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSetColor( int winRef,  SiiOsdColorElement_t element, int rgbVal )
{
    while ( OsdIsValidWinRef( winRef ))
    {
        if ( element >= OSD_ELEMENT_COUNT )
        {
            pOsd->lastResultCode = SII_OSD_ERR_INVALID_PARAMETER;
            break;
        }

        SiiDrvOsdSetColor( winRef, element, rgbVal );
        break;  // Always break
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the enable state of the specified row
//!             Change takes effect immediately.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  row         Window row to mark enabled/disabled
//! @param[in]  isEnable    true == enable, false == disable.
//! @retval                 true if successful, false if a failure.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSetRowEnable( int winRef, int row, bool_t isEnable )
{
    while ( OsdIsValidWinRef( winRef ))
    {
        if ( row >= pOsd->winData[ winRef].rowCount)
        {
            pOsd->lastResultCode = SII_OSD_ERR_INVALID_PARAMETER;
            break;
        }

        SiiDrvOsdRowEnable( winRef, row, isEnable );
        break;  // Always break
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the enable state of the specified row
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  row         window row (0-15)
//! @retval     true if row is enabled, false if a disabled or invalid parameter.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdIsRowEnabled( int winRef, int row )
{
    if ( OsdIsValidWinRef( winRef ))
    {
        if ( row >= pOsd->winData[ winRef].rowCount)
        {
            pOsd->lastResultCode = SII_OSD_ERR_INVALID_PARAMETER;
        }
    }

    return(( pOsd->lastResultCode == SII_OSD_SUCCESS ) ? SiiDrvOsdRowIsEnabled( winRef, row ) : false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the winRef of the window that currently has focus (receives key events)
//! @retval     winRef of window with input focus if valid.
//! @retval     -1 if no window has focus
//-------------------------------------------------------------------------------------------------
int SiiOsdGetFocus( void )
{
    pOsd->lastResultCode = SII_OSD_SUCCESS;
    return( pOsd->focusWinRef );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the input focus to the specified window and bring it to the top if requested.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  options     OPT_TOP, OPT_SEND_UFA_FOCUS
//! @retval                 winRef of window with input focus if valid.
//! @retval                 -1 if no window has focus
//-------------------------------------------------------------------------------------------------
int SiiOsdSetFocus( int winRef, SiiOsdOptions_t options )
{
    bool_t  setFocus = true;

    pOsd->lastResultCode = SII_OSD_SUCCESS;

    // If winRef is not -1, we are setting focus for the specified window
    // instead of removing it from all windows.
    if ( winRef != -1 )
    {
        if ( OsdIsValidWinRef( winRef ))
        {
            // Maybe we need to ask the window if they want the focus
            if ( options & OPT_SEND_UFA_FOCUS )
            {
                if ( !OsdExecuteUserFunction( winRef, UFA_FOCUS, 0 ))
                {
                    setFocus = false;
                }
            }
            if ( setFocus )
            {
                // Focus being set to an enabled window, bring it to top if requested
                if ( setFocus && (options & OPT_TOP ))
                {
                    SiiDrvOsdSetWindowZaxis( winRef, 0 );
                }
                // Set the focus to the new window by setting its highlight colors to the
                // window's highlight color.
                SiiDrvOsdSetCursor( winRef, SiiDrvOsdGetCursor( winRef ), OPT_SHOW );
            }
        }
    }

    // Don't remove the old focus if the new window didn't want it
    if ( setFocus )
    {
        // In either case, remove the focus from current focus window by setting its highlight colors to the
        // same color as normal rows.
        if (( pOsd->focusWinRef != -1 ) && ( winRef != pOsd->focusWinRef ))
        {
            SiiDrvOsdSetCursor( pOsd->focusWinRef, SiiDrvOsdGetCursor( pOsd->focusWinRef), OPT_HIDE );
        }

        // Save the new focus window value.
        pOsd->focusWinRef = winRef;
    }

    return( pOsd->focusWinRef );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Sets the timeout value for the specified window.
//! @param[in]  winRef  - Value returned by any SiiOsdWindowCreate function
//! @param[in]  msTimeout - Window display time in milliseconds.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSetWindowTimeout( int winRef, int msTimeout )
{
    if ( OsdIsValidWinRef( winRef ))
    {
        pOsd->winData[ winRef].msTimeout = msTimeout;   // Set new value.
        return( true );
    }
    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Determines state of window timer
//! @param[in]  winRef  - Value returned by any SiiOsdWindowCreate function
//! @retval     true if timer is expired, false if a not expired or window is not present.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdIsWindowTimeoutExpired( int winRef )
{
    if ( OsdIsValidWinRef( winRef ))
    {
        return( SiiOsTimerExpired( pOsd->winData[ winRef].winTimer ) );
    }
    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Show or hide the OSD window display.
//! @param[in]  enableDisplay   true - show, false - hide
//-------------------------------------------------------------------------------------------------
void SiiOsdEnableOsdDisplay ( bool_t enableDisplay )
{
    bool_t  isInterlaced;
    int     vert, horiz;

    // If enable call and not already enabled, ensure that we have the correct
    // resolution fonts loaded.
    if ( enableDisplay && !pOsd->isEnabled )
    {
        if ( SiiOsdCbGetOutputRes( 0, &horiz, &vert, &isInterlaced )) // Use callback to get proper output resolution
        {
            SiiDrvOsdResolutionChange( SI_VIDEO_MODE_NON_STD, vert, isInterlaced );
        }
    }
    SiiDrvOsdEnable( enableDisplay );
    pOsd->isEnabled = enableDisplay;
}

//-------------------------------------------------------------------------------------------------
//! @brief      If options OPT_SHOW flag is set, the window is enabled, otherwise it is
//!             disabled. This is the only valid options bit.
//! @param[in]  winRef  - Value returned by any SiiOsdWindowCreate function
//! @param[in]  options - OPT_TOP, OPT_SHOW, OPT_CENTER_WINDOW
//! @retval                 true if successful, false if a failure.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdShowWindow( int winRef, SiiOsdOptions_t options )
{
    int             windowWidth, windowHeight, hRes, vRes;
    SiiOsdWindow_t  *pWin;

    if ( OsdIsValidWinRef( winRef ))
    {
        pWin = &pOsd->winData[ winRef];

        if ( options & OPT_CENTER_WINDOW )
        {
            // Only center if screen size is known
            if ( SiiDrvSwitchSelectedPortInfo( true, &hRes, &vRes, NULL, NULL ))
            {
                windowWidth         = (pWin->colCount * pOsd->fontWidth);
                windowHeight        = (pWin->rowCount * pOsd->fontHeight);
                pWin->upperLeftX    = ( hRes - windowWidth ) / 2;
                pWin->upperLeftY    = ( vRes - windowHeight ) / 2;
                SiiDrvOsdSetHorizVert( winRef, pWin->upperLeftX, pWin->upperLeftY );
            }
        }
        SiiDrvOsdShowWindow( winRef, options );    // Let driver do the work

        // Update our local window status.
        if ( options & OPT_SHOW )
        {
            pWin->windowStatus |= OSD_WSTAT_ENABLED;
            pWin->windowStatus &= ~OSD_WSTAT_INVISIBLE;
        }
        else
        {
            pWin->windowStatus |= OSD_WSTAT_INVISIBLE;
        }

        if ( options & OPT_TOP )
        {
            SiiDrvOsdSetWindowZaxis( winRef, 0 );
        }

        // If window has a timer, restart it upon show request
        if ( options & OPT_SHOW )
        {
            if ( pWin->msTimeout )
            {
                SiiOsTimerSet( &pWin->winTimer, pWin->msTimeout );
            }
        }
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a text string to the specified window row and column.
//!             If options OPT_SHOW flag is set, the window is updated immediately
//! @param[in]  winRef      Value returned by any SiiOsdWindowCreate function
//! @param[in]  row         row within window of string start (0-15)
//! @param[in]  col         column within row of string start (0-31)
//! @param[in]  pStr        Pointer to ASCII string, null terminated
//! @param[in]  options     OPT_RESOURCEID, OPT_ELLIPSIS, OPT_CLEAR_TO_END, OPT_CENTER_TEXT,
//!                         OPT_SHOW, OPT_WRAP_TEXT
//! @retval                 true if successful, false if a failure.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdWriteText( int winRef, int row, int col, char *pStr, SiiOsdOptions_t options )
{
    if ( OsdIsValidWinRef( winRef ))
    {
        return( SiiDrvOsdWriteText( winRef, row, col, pStr, options ) );
    }
    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a text string to the specified window row and column.
//!             The window is updated immediately
//! @param[in]  winRef      Value returned by any SiiOsdWindowCreate function
//! @param[in]  row         row within window of string start (0-(row-1))
//! @param[in]  col         column within row of string start (0-(column - 1))
//! @param[in]  numChars    Number of characters to display.  Must be less that the remaining
//!                         number of characters in the window.
//! @param[in]  pBuf        Pointer to ASCII string
//! @note       This function is a customer request
//-------------------------------------------------------------------------------------------------
void SiiOsdUpdateChars( uint8_t winRef, uint8_t row, uint8_t col, uint16_t numChars, uint8_t *pBuf )
{
    char    strBuffer[OSD_WIN_COL_COUNT * OSD_WIN_ROW_COUNT];
    int     writeLength;

    if ( OsdIsValidWinRef( winRef ))
    {
        writeLength = sizeof( strBuffer ) - 1;
        writeLength = (numChars <= writeLength) ? numChars : writeLength;
        memcpy( strBuffer, pBuf, writeLength );
        strBuffer[ writeLength] = 0;
        SiiDrvOsdWriteText( winRef, row, col, strBuffer, OPT_SHOW | OPT_WRAP_TEXT );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write an icon to the specified window row and column. The icon is character-based
//!             and can be any size up to the row/col size of the window. If the options OPT_SHOW
//!             flag is set, the icon is displayed immediately.
//! @note       SiiOsdWriteIcon does NOT translate index values passed via the pIcon parameter as
//!             the SiiWriteText function does, so the actual font character index values must be used.
//! @param[in]  winRef      Value returned by any SiiOsdWindowCreate function
//! @param[in]  row         row within window of string start (0-15)
//! @param[in]  col         column within row of string start (0-31)
//! @param[in]  rowCount    height of icon in character rows
//! @param[in]  colCount    width of icon in character columns
//! @param[in]  pIcon       Pointer to array of icon character indexes or resourceId if OPT_RESOURCEID set.
//! @param[in]  options     OPT_RESOURCEID, OPT_SHOW, OPT_REMOVE_ICON
//! @retval                 true if successful, false if a failure.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdWriteIcon( int winRef, int row, int col, int rowCount, int colCount, uint8_t *pIcon, SiiOsdOptions_t options )
{
    if ( OsdIsValidWinRef( winRef ))
    {
        SiiDrvOsdWriteIcon( winRef, row, col, rowCount, colCount, pIcon, options );
    }
    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Move the window to the specified location on the display screen. If the OPT_TOP
//!             option is set, the window is also brought to the top of the window stack on the display.
//!             Change takes effect immediately.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  startX      Upper left pixel horizontal coordinate of window
//! @param[in]  startY      Upper left pixel vertical coordinate of window
//! @param[in]  options     OPT_TOP, OPT_SHOW
//! @retval                 true if successful, false if a failure.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdMoveWindow( int winRef, int startX, int startY, SiiOsdOptions_t options )
{
    SiiOsdWindow_t  *pWin;

    if ( OsdIsValidWinRef( winRef ))
    {
        pWin = &pOsd->winData[ winRef];

        pWin->upperLeftX    = startX;
        pWin->upperLeftY    = startY;
        SiiDrvOsdSetHorizVert( winRef, startX, startY );
        if ( options & ( OPT_TOP | OPT_SHOW ))
        {
            SiiOsdShowWindow( winRef, options );
        }
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the highlight (cursor) row for the specified window.
//! @param[in]  winRef      Value returned by a SiiOsdWindowCreate function
//! @param[in]  row         new cursor row.
//! @param[in]  options     OPT_HIDE, OPT_SHOW, OPT_ENABLE_CURSOR
//! @return     bool        true: success, false: requested row is disabled or out of range.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSetCursor ( int winRef, int row, SiiOsdOptions_t options )
{
    if ( OsdIsValidWinRef( winRef ))
    {
        if (SiiDrvOsdSetCursor( winRef, row, options ))
        {
            pOsd->winData[ winRef].highlightRow = row;
        }
    }
    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}
//-------------------------------------------------------------------------------------------------
//! @brief      Returns the highlight row index of the specified window.
//! @param[in]  winRef      Return value from the SiiOsdWindowCreate function.
//! @retval     Highlight row index
//-------------------------------------------------------------------------------------------------
int SiiOsdGetCursor ( int winRef )
{
    if ( OsdIsValidWinRef( winRef ))
    {
        return( SiiDrvOsdGetCursor( winRef ) );
    }
    return( 0 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Deactivate a window
//! @param[in]  winRef      Return value from a CreateWindow function.
//! @param[in]  options     OPT_CANCEL, OPT_FORCE_CLOSE
//! @retval     true        Window has been closed.
//! @retval     false       User window function forced window to remain active.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdCloseWindow( int winRef, SiiOsdOptions_t options )
{
    bool_t          wasKilled = false;
    SiiOsdWindow_t  *pWin;

    if ( OsdIsValidWinRef( winRef ))
    {
        pWin = &pOsd->winData[ winRef];

        // Call the window function with the CLOSE action
#if INC_OSD_BASE_HANDLER
        if (OsdExecuteUserFunction( winRef, UFA_CLOSE, (options & OPT_CANCEL) ? SIIKEY_CANCEL : SIIKEY_SELECT ) || (options & OPT_FORCE_CLOSE))
#endif // INC_OSD_BASE_HANDLER
        {
            // Do any special window type-specific stuff
#if INC_OSD_BASE_HANDLER
            switch ( pWin->windowType )
            {
                case OSD_WINTYPE_BASIC:
                case OSD_WINTYPE_STATUS:
                case OSD_WINTYPE_SLIDER:
                case OSD_WINTYPE_LISTBOX:
                case OSD_WINTYPE_MENU:
                case OSD_WINTYPE_EDITBOX:
                    break;
            }
#endif // INC_OSD_BASE_HANDLER
            SiiDrvOsdCloseWindow( winRef );
            pWin->msTimeout = 0;
            pWin->windowStatus = OSD_WSTAT_AVAILABLE;
            if ( pOsd->focusWinRef == winRef )
            {
                pOsd->focusWinRef = -1;
            }
            wasKilled = true;
        }

        // Successful at component level, check the driver level.
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( wasKilled );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Close all active windows
//! @retval     true        All windows closed
//! @retval     false       User window function forced at least one window to remain active.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdCloseAll( void )
{
    int     winRef;
    bool_t  success = true;

    for ( winRef = 0; winRef < SII_OSD_WIN_COUNT; winRef++ )
    {
        if ( pOsd->winData[winRef].windowStatus & OSD_WSTAT_ENABLED )
        {
            // Close window, but save any changes
            if ( !SiiOsdCloseWindow( winRef, OPT_CANCEL | OPT_FORCE_CLOSE ))
            {
                success = false;
                break;
            }
        }
    }
    pOsd->focusWinRef = -1; // Remove focus also.
    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize a window structure to be used as a base for other window types
//! @param[in]  windowId    - ID of resource to provide window contents and properties. If 0, create
//!                           an empty window.
//! @param[in]  startX      - Upper left pixel horizontal coordinate of window
//! @param[in]  startY      - Upper left pixel vertical coordinate of window
//! @param[in]  rowCount    - Number of character rows in window (1-16)
//! @param[in]  colCount    - Number of character columns per row (1-32)
//! @retval                 - Window reference value if successful
//! @retval                 - -1 if creation failed
//-------------------------------------------------------------------------------------------------
int OsdCreateBaseWindow( int windowId, int startX, int startY, int rowCount, int colCount )
{
    SiiOsdWindow_t  *pWin;
    int             winRef;

    pOsd->lastResultCode = SII_OSD_ERR_FAIL;

    // Verify that the resource is valid and create the window.
    if ( windowId != 0 )
    {
        if ( !SiiDrvOsdIsResource( OSD_RESOURCE_WINDOW, windowId ))
        {
            pOsd->lastResultCode = SII_OSD_ERR_FAIL;    //TODO: Need a better error message
            return( -1 );
        }
    }

    // Attempt to create the window in hardware
    winRef = SiiDrvOsdCreateWindow( windowId, startX, startY, rowCount, colCount );

    // Get window info from driver
    SiiDrvOsdGetWindowProperties( winRef, (void*)&pOsd->winData[ winRef] );

    // Fill out the remainder of the window properties, if needed
    if ( winRef != -1 )
    {
        pWin = &pOsd->winData[ winRef];
        pWin->parentWinRef = -1;       // No parent that we are aware of yet.
        pWin->childWinRef = -1;        // No child that we are aware of yet. (We are all alone in this world!)
        pWin->pExternalData = NULL;
        pWin->pFunction = NULL;
        pOsd->lastResultCode = SII_OSD_SUCCESS;
    }

    return( winRef );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Use the content index data from the passed window resource to fill the specified window.
//! @note       Only the content index data from the resource is used.  It is assumed that there
//!             is enough data in this resource to fill the referenced window.
//! @param[in]  winRef      Return value from a SiiOsdWindowCreate function.
//! @param[in]  windowId    ID of resource to provide window contents and properties.
//! @retval     true        Fill was successful
//! @retval     false       Fill did not happen
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdFillWindow( int winRef, int resourceId )
{
    SiiOsdWindow_t  *pWin;

    if ( OsdIsValidWinRef( winRef ))
    {
        pWin = &pOsd->winData[ winRef];

        // Verify that the resource is valid and create the window.
        if ( resourceId != 0 )
        {
            if ( !SiiDrvOsdIsResource( OSD_RESOURCE_WINDOW, resourceId ))
            {
                pOsd->lastResultCode = SII_OSD_NOT_VALID_RESOURCE;
            }
            else
            {
                if (( resourceId != 0 ) && (( resourceId & OSD_RESOURCE_LOCAL_FLAG ) == 0 ))
                {
                    // Load only the window content data, using current window sizes.
                    if ( !SiiDrvOsdLoadWindowResource( winRef, resourceId, true ))
                    {
                        pOsd->lastResultCode = SII_OSD_RESOURCE_LOAD_ERROR;
                    }
                }
                else
                {
                    pOsd->lastResultCode = SII_OSD_NOT_VALID_RESOURCE;
                }

            }
        }
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Replace the window specs for the passed window with the passed resource ID data.
//! @note       The new window retains the same parent and child window references, but NOT the
//!             resourceId for the child.  Unless care is taken, any child windows of the original
//!             window should be closed prior to calling ReplaceWindow.
//! @param[in]  winRef      Return value from a SiiOsdWindowCreate function.
//! @param[in]  windowId    ID of resource to provide window contents and properties.
//! @retval     true        Replace was successful
//! @retval     false       Replace did not happen
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdReplaceWindow( int winRef, int windowId )
{
    SiiOsdWindow_t  *pWin;
    bool_t          success = false;
    int             param1, param2, param3, param4, editMode, selectedRow;

    if ( !OsdIsValidWinRef( winRef ))
    {
        return( false );
    }
    pWin = &pOsd->winData[ winRef];

    // If given window already contains the specified window resource, we're done.
    if ( pWin->resourceId == windowId )
    {
        return( true );
    }

    // Verify that the resource is valid and create the window.
    if ( windowId != 0 )
    {
        if ( !SiiDrvOsdIsResource( OSD_RESOURCE_WINDOW, windowId ))
        {
            pOsd->lastResultCode = SII_OSD_ERR_FAIL;    //TODO: Need a better error message
        }
        else
        {
            // For ReplaceWindow, we pass the current windows private data AND selected row.
            param1      = pWin->param1;
            param2      = pWin->param2;
            param3      = pWin->param3;
            param4      = pWin->param4;
            editMode    = pWin->editMode;
            selectedRow = pWin->highlightRow;

            // Attempt to create the window in hardware
            winRef = SiiDrvOsdCreateWindow( windowId, -1, winRef, 0, 0 );  // Create the window with special parameter values

            // Get window info from driver
            SiiDrvOsdGetWindowProperties( winRef, (void*)&pOsd->winData[ winRef] );
            pOsd->winData[ winRef].windowStatus &= ~OSD_WSTAT_INVISIBLE;        // It would defeat the purpose for it to be invisible
            success = true;
        }
    }

    // Fill out the remainder of the window properties, if needed
    if ( success )
    {
#if INC_OSD_BASE_HANDLER
        // Call the window function with the REPLACE_WINDOW action (almost the same as CREATE)
        // Note that winRef refers to the new window now, not the calling window. However, the private data
        // and the selected row from the original window is sent to the new window in the REPLACEWINDOW message
        pWin->param1 = param1;
        pWin->param2 = param2;
        pWin->param3 = param3;
        pWin->param4 = param4;
        pWin->editMode = editMode;
        pWin->highlightRow = selectedRow;
        success = OsdExecuteUserFunction( winRef, UFA_REPLACEWINDOW, -1 );
#endif // INC_OSD_BASE_HANDLER
    }

    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Create an OSD window
//! @param[in]  windowId        ID of resource to provide window contents and properties. If 0,
//!                             create an empty window.
//! @param[in]  startX          Upper left pixel horizontal coordinate of window
//! @param[in]  startY          Upper left pixel vertical coordinate of window
//! @param[in]  rowCount        Number of character rows in window (1-16)
//! @param[in]  colCount        Number of character columns per row (1-32)
//! @param[in]  pWinTypeData    Pointer to a SiiOsdWinTypeData_t further describing the window
//! @param[in]  options         OPT_SHOW, OPT_TOP
//! @retval                     Window reference value if successful
//! @retval                     -1 if creation failed
//-------------------------------------------------------------------------------------------------
int SiiOsdCreateWindow( int windowId, int startX, int startY, int rowCount, int colCount, SiiOsdWinTypeData_t *pWinTypeData, SiiOsdOptions_t options )
{
    int             winRef = -1;
    SiiOsdWindow_t  *pWin;

    // Create a base window with the supplied window specs.
    winRef = OsdCreateBaseWindow( windowId, startX, startY, rowCount, colCount );
    if ( winRef == -1 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_AVAIL;
        return( -1 );
    }
    pOsd->lastResultCode    = SII_OSD_SUCCESS;
    pWin                    = &pOsd->winData[ winRef];

    // If function parameters are non-zero, they override the resource values.
    if ( startX || startY )
    {
        pWin->upperLeftX    = (startX)? startX : pWin->upperLeftX;
        pWin->upperLeftY    = (startY)? startY : pWin->upperLeftY;
        SiiDrvOsdSetHorizVert( winRef, pWin->upperLeftX, pWin->upperLeftY );
    }

    // If not default window type, update here
    if ( pWinTypeData )
    {
        pWin->windowType = pWinTypeData->winType;  // Set to our type
        if ( pWinTypeData->useParamValues )
        {
            pWin->param1 = pWinTypeData->param1;
            pWin->param2 = pWinTypeData->param2;
            pWin->param3 = pWinTypeData->param3;
            pWin->param4 = pWinTypeData->param4;
            pWin->editMode = pWinTypeData->editMode;
        }
    }

    // Give the window function a chance to initialize the window before display
#if INC_OSD_BASE_HANDLER
    if ( !OsdExecuteUserFunction( winRef, UFA_CREATE, -1 ))
    {
        // User function said no, fail the window create.
        SiiOsdCloseWindow( winRef, OPT_CANCEL );
        pOsd->lastResultCode = SII_OSD_ERR_USER_FUNC_FAIL;
        winRef = -1;
    }
#endif // INC_OSD_BASE_HANDLER

    // Display if requested
    if ( options & OPT_SHOW )
    {
        SiiOsdShowWindow( winRef, options );
    }

    return( winRef );
}

