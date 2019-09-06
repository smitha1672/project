//***************************************************************************
//!file     si_osd_win_edit.c
//!brief    Silicon Image OSD text edit window.
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

#if INC_OSD_EDITBOX

#define PICK_ROW        pWin->param1
#define PICK_COL        pWin->param2
#define EDIT_COL        pWin->param3
#define EDIT_BUFFER_LEN pWin->param4
#define EDIT_BUFFER     ((char *)(pWin->pExternalData))

#define EDITBOX_PICK_TOP_ROW        2
#define EDITBOX_PICK_SPACER_ROW     6
#define EDITBOX_PICK_BOTTOM_ROW     7
#define EDITBOX_PICK_LEFT_COL       3
#define EDITBOX_PICK_RIGHT_COL      27
#define EDITBOX_PICK_CMD_ROW        EDITBOX_PICK_BOTTOM_ROW
#define EDITBOX_PICK_CMD_LAST_COL   17

#define EDIT_TEXT_ROW               0
#define EDIT_FIRST_TEXT_COL         0
#define EDIT_LAST_TEXT_COL          31

//-------------------------------------------------------------------------------------------------
//! @brief      Create or replace an Edit box window.
//! @param[in]  winRef      if -1, create a new Edit Box with the specified resource ID.
//!                         if > -1, return value from a SiiOsdEditBoxCreate for replacement.
//! @param[in]  windowId    ID of resource to provide replacement Edit box window (if winRef != -1).
//!                         ID of resource to provide original Edit box window (if winRef == -1).
//! @param[in]  startX      Upper left pixel horizontal coordinate of window.
//! @param[in]  startY      Upper left pixel vertical coordinate of window.
//! @param[in]  pTextStr    Character string to be edited.
//! @param[in]  options     OPT_SHOW
//! @retval                 Window reference value if successful
//! @retval                 -1 if creation failed.
//-------------------------------------------------------------------------------------------------
int SiiOsdCreateEditBox( int winRef, int windowId, int startX, int startY, char *pTextBuffer, int bufferLen, SiiOsdOptions_t options )
{
    SiiOsdWindow_t  *pWin;
    int             param1, param2, param3, param4;

    pOsd->lastResultCode = SII_OSD_ERR_NOT_AVAIL;
    if ( winRef == -1 )
    {
        // Create a base window using passed resource ID.
        winRef = OsdCreateBaseWindow( windowId, 0, 0, 0, 0 );
        if ( winRef == -1 )
        {
            return( -1 );
        }
        pWin = &pOsd->winData[ winRef];
        EDIT_COL        = 0;
        PICK_ROW        = EDITBOX_PICK_TOP_ROW;
        PICK_COL        = EDITBOX_PICK_LEFT_COL;
        EDIT_BUFFER_LEN = bufferLen;
    }
    else
    {
        // Save current edit box cursors, etc.
        pWin    = &pOsd->winData[ winRef];
        param1  = PICK_ROW;
        param2  = PICK_COL;
        param3  = EDIT_COL;
        param4  = EDIT_BUFFER_LEN;

        // Replace existing window
        if ( !SiiOsdReplaceWindow( winRef, windowId ))
        {
            return( -1 );
        }
        // Restore current edit box cursors, etc.
        PICK_ROW        = param1;
        PICK_COL        = param2;
        EDIT_COL        = param3;
        EDIT_BUFFER_LEN = param4;
    }
    pWin->pExternalData     = pTextBuffer;

    pOsd->lastResultCode    = SII_OSD_SUCCESS;
    pWin->windowType        = OSD_WINTYPE_EDITBOX;  // Set to our type

    // If function parameters are non-zero, they override the resource values.
    pWin->upperLeftX    = (startX)? startX : pWin->upperLeftX;
    pWin->upperLeftY    = (startY)? startY : pWin->upperLeftY;
    SiiDrvOsdSetHorizVert( winRef, pWin->upperLeftX, pWin->upperLeftY );

    // Update the text edit string row.
    SiiOsdWriteText( winRef, EDIT_TEXT_ROW, EDIT_FIRST_TEXT_COL, EDIT_BUFFER, OPT_ELLIPSIS | OPT_CLEAR_TO_END );
    SiiDrvOsdCursorChar( winRef, CURSOR_INVERT, false, EDIT_TEXT_ROW, EDIT_COL );   // Invert the first edit character
    SiiDrvOsdCursorChar( winRef, CURSOR_BOX, false, PICK_ROW, PICK_COL );           // Box the default pick character

    // Call the window function with the POST_CREATE action
    if ( !OsdExecuteUserFunction( winRef, UFA_CREATE, -1 ))
    {
        // Post create failed, fail the window create.
        SiiOsdCloseWindow( winRef, OPT_CANCEL );
        pOsd->lastResultCode = SII_OSD_ERR_USER_FUNC_FAIL;
        winRef = -1;
    }

    // Display if requested
    if ( options & OPT_SHOW )
    {
        SiiOsdShowWindow( winRef, options );
    }

    return( winRef );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Manage a text edit window. Sends only UFA_CREATE, UFA_CLOSE, UFA_SELECT,
//!             and UFA_CANCEL actions to the window user function.
//! @param[in]  winRef      Return value from the SiiOsdCreateEditBox function.
//! @param[in]  key         SIIKEY_SELECT, SIIKEY_CANCEL, SIIKEY_CURSOR_xxx.
//! @param[in]  options     none
//! @return     true if key was used, false if not.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdEditBoxHandler( int winRef, int key, SiiOsdOptions_t options )
{
    bool_t          cursorMove, editCursorMove;
    bool_t          keyUsed = true;
    int             pickChar;
    SiiOsdWindow_t  *pWin;

    // Ignore window if not enabled.
    if (( winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
        return( false );
    }
    pOsd->lastResultCode    = SII_OSD_SUCCESS;
    pWin                    = &pOsd->winData[ winRef];

    cursorMove      = false;
    editCursorMove  = false;
    if ( winRef == SiiOsdGetFocus())
    {
        // Don't use the base window handler for this window
        switch ( key )
        {
            case SIIKEY_CURSOR_UP:
                if ( PICK_ROW > EDITBOX_PICK_TOP_ROW )
                {
                    PICK_ROW--;
                    if ( PICK_ROW == EDITBOX_PICK_SPACER_ROW )
                    {
                        PICK_ROW--;
                    }
                }
                else
                {
                    PICK_ROW = EDITBOX_PICK_BOTTOM_ROW;
                }
                cursorMove = true;
                break;
            case SIIKEY_CURSOR_DOWN:
                if ( PICK_ROW < EDITBOX_PICK_BOTTOM_ROW )
                {
                    PICK_ROW++;
                    if ( PICK_ROW == EDITBOX_PICK_SPACER_ROW )
                    {
                        PICK_ROW++;
                    }
                }
                else
                {
                    PICK_ROW = EDITBOX_PICK_TOP_ROW;
                }
                cursorMove = true;
                break;
            case SIIKEY_CURSOR_LEFT:
                if ( PICK_COL > EDITBOX_PICK_LEFT_COL )
                {
                    PICK_COL -= 2;  // Skip spaces
                }
                else
                {
                    PICK_COL = EDITBOX_PICK_RIGHT_COL;
                }
                cursorMove = true;
                break;
            case SIIKEY_CURSOR_RIGHT:
                if ( PICK_COL < EDITBOX_PICK_RIGHT_COL )
                {
                    PICK_COL += 2;  // Skip spaces;
                }
                else
                {
                    PICK_COL = EDITBOX_PICK_LEFT_COL;
                }
                cursorMove = true;
                break;

            case SIIKEY_CANCEL:
                SiiDrvOsdCursorChar( winRef, CURSOR_INVERT, true, 0, 0 );   // Remove last inverted character
                SiiDrvOsdCursorChar( winRef, CURSOR_BOX, true, 0, 0 );      // Remove last boxed character

                // Execute user function with UFA_CANCEL action
                if ( OsdExecuteUserFunction( winRef, UFA_CANCEL, key ))
                {
                    // If this is a child window, the Select and Cancel actions both
                    // close this window and set focus to the parent window, if any.
                    SiiOsdSetFocus( pWin->parentWinRef, OPT_TOP );
                    SiiOsdCloseWindow( winRef, OPT_CANCEL );
                }
                break;
            case SIIKEY_SELECT:

                pickChar = SiiDrvOsdGetCharIndex( winRef, PICK_ROW, PICK_COL );
                switch ( pickChar )
                {
                    case SII_OSD_EDIT_UC_CHAR:          // Edit Upper case select character
                        // It is assumed that since the UC character is displayed, we have the LC window resource,
                        // and the LC window resource MUST immediately follow the UC window resource, so that
                        // means the the UC window we want to display is our resource value - 1.
                        SiiOsdCreateEditBox( winRef, pWin->resourceId - 1, 0, 0, EDIT_BUFFER, EDIT_BUFFER_LEN, 0 );
                        SiiOsdShowWindow( winRef, OPT_SHOW );
                        break;
                    case SII_OSD_EDIT_LC_CHAR:          // Edit Lower case select character
                        // It is assumed that since the LC character is displayed, we have the UC window resource,
                        // and the LC window resource MUST immediately follow the UC window resource, so that
                        // means the the LC window we want to display is our resource value + 1.
                        SiiOsdCreateEditBox( winRef, pWin->resourceId + 1, 0, 0, EDIT_BUFFER, EDIT_BUFFER_LEN, 0 );
                        SiiOsdShowWindow( winRef, OPT_SHOW );
                        break;
                    case SII_OSD_EDIT_CLR_CHAR:         // Edit CLR character (clear edit text string)
                        memset( EDIT_BUFFER, ' ', EDIT_BUFFER_LEN );
                        EDIT_BUFFER[ EDIT_BUFFER_LEN - 1] = 0;
                        SiiOsdWriteText( winRef, 0, 0, " ", OPT_SHOW | OPT_CLEAR_TO_END );
                        EDIT_COL = 0;
                        editCursorMove = true;
                        break;
                    case SII_OSD_EDIT_DEL_CHAR:         // Edit DEL character (backspace)
                        break;
                    case SII_OSD_EDIT_INS_CHAR:         // Edit INS character (insert)
                        break;
                    case SII_OSD_EDIT_OK_CHAR:          // Edit OK character (finished edit)
                        SiiDrvOsdCursorChar( winRef, CURSOR_INVERT, true, 0, 0 );   // Remove last inverted character
                        SiiDrvOsdCursorChar( winRef, CURSOR_BOX, true, 0, 0 );      // Remove last boxed character

                        // Execute user function with SELECT action
                        if ( OsdExecuteUserFunction( winRef, UFA_SELECT, -1 ))
                        {
                            SiiOsdSetFocus( pWin->parentWinRef, OPT_TOP );
                            SiiOsdCloseWindow( winRef, 0 ); // Save results
                        }
                        break;
                    case SII_OSD_CHAR_LEFT:
                        EDIT_COL--;
                        if ( EDIT_COL < EDIT_FIRST_TEXT_COL )
                        {
                            EDIT_COL = EDIT_LAST_TEXT_COL;
                        }
                        editCursorMove = true;
                        break;
                    case SII_OSD_CHAR_RIGHT:
                        EDIT_COL++;
                        if ( EDIT_COL > EDIT_LAST_TEXT_COL )
                        {
                            EDIT_COL = EDIT_FIRST_TEXT_COL;
                        }
                        editCursorMove = true;
                        break;

                    case SII_OSD_EDIT_SPC_CHAR:         // Edit SPC character (space)
                        pickChar = ' ';
                        // Fall through to character entry.
                    default:

                        // Write selected character to current text string edit position
                        EDIT_BUFFER[ EDIT_COL] = pickChar;
                        SiiOsdWriteText( winRef, EDIT_TEXT_ROW, EDIT_COL, &EDIT_BUFFER[ EDIT_COL], OPT_SHOW );
                        EDIT_COL++;
                        if ( EDIT_COL > EDIT_LAST_TEXT_COL )
                        {
                            EDIT_COL = EDIT_LAST_TEXT_COL;
                        }
                        editCursorMove = true;
                        break;
                }
                break;
            default:
                keyUsed = false;
                break;
        }

        if ( editCursorMove )
        {
            SiiDrvOsdCursorChar( winRef, CURSOR_INVERT, false, EDIT_TEXT_ROW, EDIT_COL );   // Box the new character
        }
        if ( cursorMove )
        {
            // Bottom line is shorter
            if (( PICK_ROW == EDITBOX_PICK_CMD_ROW ) && ( PICK_COL > EDITBOX_PICK_CMD_LAST_COL ))
            {
                PICK_COL = EDITBOX_PICK_CMD_LAST_COL;
            }
            SiiDrvOsdCursorChar( winRef, CURSOR_BOX, false, PICK_ROW, PICK_COL );   // Box the new character
        }
    }

    return( keyUsed );
}
#endif  // if INC_OSD_EDITBOX
