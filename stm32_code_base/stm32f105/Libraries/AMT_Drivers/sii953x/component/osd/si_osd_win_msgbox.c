//***************************************************************************
//!file     si_osd_win_msgbox.c
//!brief    Silicon Image OSD Message Box window.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_debug.h"
#include "si_osd_internal.h"
#if INC_OSD_MESSAGEBOX

//-------------------------------------------------------------------------------------------------
//! @brief      Built-in message box user function.
//! @param[in]  winRef      Return value from the SiiOsdCreateWindow function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------

bool_t SiiOsdUserFuncBuiltIn ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            break;
        case UFA_CLOSE:
            break;
        case UFA_ROW_ENTER:
        case UFA_SELECT:
        case UFA_CANCEL:
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Display a message dialog box.
//! @param[in]  windowId    ID of resource to provide window contents and properties. If 0, use
//!                         standard SYSTEM_MSG properties.
//! @param[in]  pTitleStr   Title text string (resource ID not allowed) or null if no title
//! @param[in]  pMsgStr     Message text string or text resourceID
//! @param[in]  startX      Upper left pixel horizontal coordinate of window
//! @param[in]  startY      Upper left pixel vertical coordinate of window
//! @param[in]  msTimeOut   Window close timeout in milliseconds after last command input to handler
//! @param[in]  options     OPT_CENTER_WINDOW, OPT_CENTER_TEXT
//!                         OPTW_ALLOW_ICON
//! @retval                 Window reference value if successful
//! @retval                 -1 if creation failed.
//-------------------------------------------------------------------------------------------------
int SiiOsdMessageBox( int windowId, char *pTitleStr, char *pMsgStr, int startX, int startY, int msTimeout, SiiOsdOptions_t options )
{
    int             i, thisRowCount, thisColCount, winRef = -1;
    int             crlfCount, longestLine, lineLength, stringLength, displayRow, stringOffset, titleLength;
    SiiOsdWindow_t  *pWin;

    options &= ~OPT_SHOW;       // Hold off any show requests, we'll force a show at the end.

    thisColCount    = 0;
    thisRowCount    = 0;
    stringLength    = 0;
    titleLength = (pTitleStr) ? strlen( pTitleStr ) : 0;
    if (( windowId == 0 ) || (pMsgStr != NULL ))
    {
        // Determine required window size, build the window, and display the text string.
        crlfCount = 1;
        longestLine = 0;
        lineLength = 0;
        stringLength = strlen( pMsgStr );
        for ( i = 0; i < stringLength; i++ )
        {
            if ( pMsgStr[ i] == '\x0A' )
            {
                if ( lineLength > longestLine )
                {
                    longestLine = lineLength;
                    lineLength = 0;
                }
                crlfCount++;
            }
            else
            {
                lineLength++;
            }
        }
        if ( longestLine == 0 )
        {
            longestLine = lineLength;
        }
        thisColCount = ( longestLine > titleLength ) ? longestLine : titleLength;
        thisColCount = ( thisColCount > OSD_WIN_COL_COUNT ) ? OSD_WIN_COL_COUNT : thisColCount;
        thisRowCount = titleLength ? ( crlfCount + 1 ) : crlfCount;
        thisRowCount = ( thisRowCount > OSD_WIN_ROW_COUNT_MAX_COL ) ? OSD_WIN_ROW_COUNT_MAX_COL : thisRowCount;
    }

    winRef = OsdCreateBaseWindow( windowId, startX, startY, thisRowCount, thisColCount );
    if ( winRef == -1 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_AVAIL;
        return( -1 );
    }
    pOsd->lastResultCode = SII_OSD_SUCCESS;
    pWin = &pOsd->winData[ winRef];
    pWin->windowType    = OSD_WINTYPE_STATUS;   // Set to our type

    // If function parameters are non-zero, they override the resource values.
    pWin->msTimeout     = (msTimeout) ? msTimeout : pWin->msTimeout;

    pWin->upperLeftX    = (startX)? startX : pWin->upperLeftX;
    pWin->upperLeftY    = (startY)? startY : pWin->upperLeftY;
    SiiDrvOsdSetHorizVert( winRef, pWin->upperLeftX, pWin->upperLeftY );

    // If non-resource message, display it now.
    if (( windowId == 0 ) || (pMsgStr != NULL ))
    {
        displayRow = (titleLength | (pWin->options & OPTW_TITLE)) ? 1 : 0;
        stringOffset = 0;
        for ( i = 0; i < stringLength; i++ )
        {
            if ( pMsgStr[ i] == '\x0A' )
            {
                pMsgStr[i] = 0;    // Terminate string up to this point.
                SiiOsdWriteText( winRef, displayRow, 0, &pMsgStr[stringOffset], options );
                stringOffset = i + 1;
                displayRow++;
            }
        }
        if ( stringOffset < stringLength )
        {
            SiiOsdWriteText( winRef, displayRow, 0, &pMsgStr[stringOffset], options );
        }
        if ( titleLength )
        {
            // Assumes that the default selected (highlight) row is 0.
            SiiDrvOsdSetColor( winRef, OSD_FG_HIGHLIGHT, SYSTEM_MSG_TITLE_FG );
            SiiDrvOsdSetColor( winRef, OSD_BG_HIGHLIGHT, SYSTEM_MSG_TITLE_BG );
            SiiOsdWriteText( winRef, 0, 0, pTitleStr, OPT_CENTER_TEXT );
        }

        // Set system message box border color
        SiiDrvOsdSetColor( winRef, OSD_BORDER, SYSTEM_MSG_BORDER );
    }

    // If nobody specified a user function, use our internal UF
    if ( pWin->windowFunctionId == 0 )
    {
        pWin->windowFunctionId = SII_OSD_UF_MSGBOX;     //
    }
    pWin->parentWinRef = SiiOsdGetFocus();      // Return focus when this window is closed.

    // Always show the window, always on top
    SiiOsdShowWindow( winRef, OPT_SHOW | OPT_TOP | options );
    return( winRef );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Display a message dialog box from a resource.
//! @param[in]  resourceId  ID of resource to provide window contents and properties.
//! @retval                 Window reference value if successful
//! @retval                 -1 if creation failed.
//-------------------------------------------------------------------------------------------------
int SiiOsdResourceMessageBox ( int resourceId, int startX, int startY, int msTimeout, SiiOsdOptions_t options )
{
    return( SiiOsdMessageBox( resourceId, "System Message", 0, startX, startY, msTimeout, options ));
}

#endif // if INC_OSD_MESSAGEBOX
