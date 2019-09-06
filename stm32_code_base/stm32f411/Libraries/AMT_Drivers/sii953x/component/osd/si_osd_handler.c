//***************************************************************************
//!file     si_osd_handler.c
//!brief    Silicon Image OSD Component optional window management.
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
#if INC_OSD_BASE_HANDLER
//-------------------------------------------------------------------------------------------------
//! @brief      Move cursor one active row position up or down. Skips disabled rows.  If OPT_WRAP
//!             is set for the window, wraps from top to bottom and vice-versa. OPT_SHOW must be
//!             specified or cursor will be hidden, not moved.
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @param[in]  moveDown    true: Move cursor down, false: move cursor up.
//! @param[in]  options     OPT_HIDE, OPT_SHOW
//! @retval                 New row location
//-------------------------------------------------------------------------------------------------
int SiiOsdMoveCursor( int winRef, bool_t moveDown, SiiOsdOptions_t options )
{
    SiiOsdWindow_t  *pWin;
    int             row, newRow, startRow, moveIncrement;

    pOsd->lastResultCode = SII_OSD_SUCCESS;
    pWin = &pOsd->winData[ winRef];

    row = SiiDrvOsdGetCursor( winRef );
    if (( options & OPT_SHOW ) == 0 )
    {
        SiiDrvOsdSetCursor( winRef, row, OPT_HIDE );
        return( row );
    }

    startRow = row;
    moveIncrement = (moveDown) ? 1 : -1;
    for (;;)
    {
        // Determine new row (with optional wrap)
        newRow = ( row + moveIncrement );
        if ( newRow >= pWin->rowCount )
        {
            if ( pWin->options & OPTW_WRAPCURSOR  )
            {
                newRow = 0;     // Wrap to first.
            }
            else
            {
                // We have reached the end of the list.  If it is a disabled row,
                // we can't go any further than we started, so return the start row.
                newRow = pWin->rowCount - 1;
                if ( !SiiDrvOsdRowIsEnabled( winRef, newRow ))
                {
                    newRow = startRow;
                    break;
                }
            }
        }
        else if ( newRow < 0 )
        {
            if ( pWin->options & OPTW_WRAPCURSOR  )
            {
                newRow = pWin->rowCount - 1; // Wrap to last
            }
            else
            {
                // We have reached the beginning of the list.  If it is a disabled row,
                // we can't go any further than we started, so return the start row.
                newRow = 0;
                if ( !SiiDrvOsdRowIsEnabled( winRef, newRow ))
                {
                    newRow = startRow;
                    break;
                }
            }
        }

        // If we haven't landed on a disabled row, we're done.
        if ( SiiDrvOsdRowIsEnabled( winRef, newRow ))
        {
            SiiDrvOsdSetCursor( winRef, newRow, OPT_SHOW );
            break;
        }
        else
        {
            row = newRow;

            // If we have wrapped around to our starting position
            // there are no other enabled rows, so get out.
            if ( newRow == startRow )
            {
                break;
            }
        }
    }
    pWin->highlightRow = newRow;
    return( newRow );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Execute a window callback function
//! @param[in]  winRef      Return value from a CreateWindow function.
//! @return     function result; depends upon action.  See documentation.
//-------------------------------------------------------------------------------------------------
bool_t OsdExecuteUserFunction ( int winRef, int action, int key )
{
    bool_t                  result = true;  // Assume success if no function ID
    SiiFuncArgs_t           winInfo;
    SiiOsdWindow_t          *pWin;
    SiiOsdUserFunction_t    pUserFunc = NULL;

    pWin = &pOsd->winData[ winRef];

    // Could be function index or attached function pointer.
    if ((pWin->windowFunctionId != 0) && ( pWin->windowFunctionId < pOsd->functionCount ))
    {
        pUserFunc = pOsd->pUserFuncs[ pWin->windowFunctionId];
    }
    else if ( pWin->pFunction)
    {
        pUserFunc = pWin->pFunction;
    }

    // If either exists, do it.
    if ( pUserFunc != NULL)
    {
        winInfo.childWinRef     = pWin->childWinRef;
        winInfo.parentWinRef    = pWin->parentWinRef;
        winInfo.resourceId      = pWin->resourceId;
        winInfo.options         = pWin->options;
        winInfo.selectedRow     = pWin->highlightRow;
        winInfo.rowChildWinId   = pWin->rowChildWindowId[ pWin->highlightRow];
        winInfo.actionKey       = key;
        winInfo.param1          = pWin->param1;
        winInfo.param2          = pWin->param2;
        winInfo.param3          = pWin->param3;
        winInfo.param4          = pWin->param4;
        winInfo.editMode        = pWin->editMode;
        winInfo.pExternalData   = pWin->pExternalData;

        result = pUserFunc( winRef, action, (void *)&winInfo );
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Send a message to the specified window
//! @param[in]  winRef      Return value from a CreateWindow function.
//! @return     function result; depends upon action.  See documentation.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdSendMessage ( int winRef, SiiOsdUserFunctionAction_t action, int key )
{
    // Error if window not enabled.
    if ((winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
        return( false );
    }
    pOsd->lastResultCode = SII_OSD_SUCCESS;

    return( OsdExecuteUserFunction( winRef, action, key ));
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set or clear Edit Mode flag.  The flag is used by cursor edit modes (shows < and >
//!             on either end of a choice set.
//! @param[in]  winRef      Return value from a CreateWindow function.
//! @param[in]  key         SIIKEY_SELECT, SIIKEY_CANCEL.
//! @param[in]  options     OPT_HIDE, OPT_SHOW
//! @return                 Result of the Window User function call, if any, otherwise false..
//-------------------------------------------------------------------------------------------------
void SiiOsdSetEditMode( int winRef, bool_t isEditMode )
{
    if ((winRef == -1) || ( winRef >= SII_OSD_WIN_COUNT ))
    {
        return;
    }
    pOsd->winData[ winRef].editMode = isEditMode;
}

int SiiOsdIsEditMode( int winRef )
{
    if ((winRef == -1) || ( winRef >= SII_OSD_WIN_COUNT ))
    {
        return( 0 );
    }

    return( pOsd->winData[ winRef].editMode );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Manage the basic window functions
//! @param[in]  winRef      Return value from a CreateWindow function.
//! @param[in]  key         SIIKEY_SELECT, SIIKEY_CANCEL.
//! @param[in]  options     OPT_HIDE, OPT_SHOW
//! @return                 Result of the Window User function call, if any, otherwise false..
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdBaseWindowHandler( int winRef, int key, SiiOsdOptions_t options )
{
    int             activeRow, row;
    bool_t          result = false;
    bool_t          performedAction = false;
    SiiOsdWindow_t  *pWin;

    // Ignore window if not enabled.
    if (( winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
        return( false );
    }
    pOsd->lastResultCode = SII_OSD_SUCCESS;
    pWin = &pOsd->winData[ winRef];

    if ( winRef == SiiOsdGetFocus())
    {
        activeRow   = pWin->highlightRow;

        switch ( key )
        {
            case SIIKEY_CURSOR_UP:
            case SIIKEY_CURSOR_DOWN:
                // Move the cursor.
                row = SiiOsdMoveCursor( winRef, ( key == SIIKEY_CURSOR_DOWN) ? true : false, OPT_SHOW );
                if (( row != -1 ) && ( row != activeRow ))
                {
                    // Execute user function with ROW_ENTER action
                    result = OsdExecuteUserFunction( winRef, UFA_ROW_ENTER, key );
                    pWin->editMode = 0;   //extendCol
                }
                performedAction = true;
                break;

            case SIIKEY_CURSOR_LEFT:
                // Change window focus to parent window if present and not in
                // edit mode, otherwise send event to current window function
                if (( pWin->parentWinRef != -1 ) && ( pWin->editMode == 0 ))  //extendCol
                {
                    SiiOsdSetFocus( pWin->parentWinRef, OPT_TOP );

                    // Send a UFA_FOCUS to the parent.
                    result = OsdExecuteUserFunction( pWin->parentWinRef, UFA_FOCUS, key );
                }
                else
                {
                    // Pass the UFA_BACK message to the current window to indicate CURSOR_LEFT key received.
                    result = OsdExecuteUserFunction( winRef, UFA_BACK, key );
                }
                performedAction = true;
                break;

            case SIIKEY_CURSOR_RIGHT:
                // Change window focus to child window if present and not in edit mode, otherwise
                // pass cursor key to user function.
                if (( pWin->childWinRef != -1 ) && !pWin->editMode)
                {
                    pOsd->winData[ pWin->childWinRef].parentWinRef = winRef;  // Remember our parent
                    SiiOsdSetFocus( pWin->childWinRef, OPT_TOP );

                    // Send a UFA_FOCUS to the child.
                    result = OsdExecuteUserFunction( pWin->childWinRef, UFA_FOCUS, key );
                }
                else
                {
                    // Pass the UFA_FOCUS message to the current window to indicate CURSOR_RIGHT key received.
                    result = OsdExecuteUserFunction( winRef, UFA_FOCUS, key );
                }
                performedAction = true;
                break;

            case SIIKEY_CANCEL:
            case SIIKEY_SELECT:

                // Execute user function with SELECT action
                result = OsdExecuteUserFunction( winRef, (key == SIIKEY_SELECT) ? UFA_SELECT : UFA_CANCEL, key );
                if ( result )
                {
                    // If this is a child window, the Select and Cancel actions both
                    // close this window and set focus to the parent window, if any.
                    SiiOsdSetFocus( pWin->parentWinRef, OPT_TOP );
                    SiiOsdCloseWindow( winRef, (key == SIIKEY_SELECT) ? false : true );
                }
                break;

            default:
                break;
        }
    }

    // If window has a timer and a key was used, restart the timer now..
    if ( performedAction && pWin->msTimeout )
    {
        if (( key != SIIKEY_CANCEL ) && ( key != SIIKEY_SELECT ))
        {
            SiiOsTimerSet( &pWin->winTimer, pWin->msTimeout  );
        }
    }
    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Manage all windows created using the SiiOsdCreate window functions.
//! @param[in]  key         Key to send to the window that currently has focus, or -1 if only check timers.
//! @param[in]  options     none
//! @retval                 Window action value if successful
//! @return     true if key was used, false if not.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdWindowManager( int key, SiiOsdOptions_t options )
{
    SiiOsdWindow_t  *pWin;
    bool_t          keyUsed = false;
    int             winRef;

#if INC_OSD_BASE_HANDLER
    // Manage window timers
    for ( winRef = 0; winRef < SII_OSD_WIN_COUNT; winRef++ )
    {
        pWin = &pOsd->winData[ winRef];
        if (( pWin->windowStatus & OSD_WSTAT_ENABLED ) && ( pWin->msTimeout ))
        {
            // Close window if timer expired.
            if ( SiiOsTimerExpired( pWin->winTimer ))
            {
                // Close the window, but don't save any changes.
                SiiOsdCloseWindow( winRef, OPT_CANCEL );

                // If this is a child window and it had focus, set focus to the parent window.
                if (( pWin->parentWinRef != -1 ) && ( pOsd->focusWinRef == winRef ))
                {
                    SiiOsdSetFocus( pWin->parentWinRef, OPT_TOP );
                }
            }
        }
    }

    if ( pOsd->focusWinRef != -1 )
    {
        // If key event, pass the key to the window with focus.
        pWin = &pOsd->winData[ pOsd->focusWinRef];
        if (( pWin->windowStatus & OSD_WSTAT_ENABLED ) && ( key != -1 ))
        {
            switch ( pWin->windowType )
            {
                case OSD_WINTYPE_BASIC:
                case OSD_WINTYPE_STATUS:
                default:
                    SiiOsdBaseWindowHandler( pOsd->focusWinRef, key, options );
                    keyUsed = true;
                    break;

#if INC_OSD_SLIDER
                case OSD_WINTYPE_SLIDER:
                    keyUsed = SiiOsdSliderHandler( pOsd->focusWinRef, key, options );
                    break;
#endif // INC_OSD_SLIDER
#if INC_OSD_LISTBOX
                case OSD_WINTYPE_LISTBOX:
                    keyUsed = SiiOsdListBoxHandler( pOsd->focusWinRef, key, options );
                    break;
#endif // INC_OSD_LISTBOX
#if INC_OSD_SLIDER
                case OSD_WINTYPE_MENU:
                    keyUsed = SiiOsdMenuHandler( pOsd->focusWinRef, key, options );
                    break;
#endif // INC_OSD_LISTBOX
#if INC_OSD_EDITBOX
                case OSD_WINTYPE_EDITBOX:
                    keyUsed = SiiOsdEditBoxHandler( pOsd->focusWinRef, key, options );
                    break;
#endif // INC_OSD_EDITBOX
            }
        }
#endif // INC_OSD_BASE_HANDLER
    }

    return( keyUsed );
}

#endif // INC_OSD_BASE_HANDLER
