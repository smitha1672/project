//***************************************************************************
//!file     sk_app_osd_options.c
//!brief    OSD Option Select Window implementation
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "sk_app_osd.h"

//-------------------------------------------------------------------------------------------------
// Enumeration and defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------

SiiOptWinStruct_t optWin;

char *optionsHelpText [] =
{
    "Enable InstaPrevue port selection",
    "Enable InstaPrevue highlighting",
};


//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
bool_t UserFuncOptionSelect ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t      result  = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
            // Use saved former cursor row.
            SiiOsdSetCursor( winRef, osdApp.cursorOptionsMenu, OPT_SHOW );
            // Fall through...
        case UFA_CREATE:

            SiiOsdWriteText( osdApp.winRefTop, 2, 4, "SYSTEM SETUP", OPT_CLEAR_TO_END );
            SiiOsdWriteIcon( osdApp.winRefTop, 1, 1, 2, 2, GEARS_ICON, 0 );

            optWin.ipvAllowed = app.ipvAllowed;
            SiiOsdWriteText( winRef, 0, 1, "Enable IPV", 0 );
            SiiOsdMenuAddItemCheck( winRef, 0, OPT_ADD_CHECK | (app.ipvAllowed ? OPT_SET_CHECK : 0 ));

            optWin.ipvHighlight = app.ipvHighlight;
            SiiOsdWriteText( winRef, 1, 1, "IPV highlight", 0 );
            SiiOsdMenuAddItemCheck( winRef, 1, OPT_ADD_CHECK | (app.ipvHighlight ? OPT_SET_CHECK : 0 ));

            UpdateMenuHelp( optionsHelpText, KH_TOP_KEYS, pWinInfo->selectedRow );
            break;

        case UFA_ROW_ENTER:
            UpdateMenuHelp( optionsHelpText, KH_TOP_KEYS, pWinInfo->selectedRow );
            osdApp.cursorOptionsMenu = pWinInfo->selectedRow;

            result = false;
            break;

        case UFA_FOCUS:
            break;

        case UFA_SELECT:

            result = false;     // Don't allow the SIIKEY_SELECT key to kill this window.
            switch ( pWinInfo->selectedRow )
            {
                case 0:
                    optWin.ipvAllowed = !optWin.ipvAllowed;
                    SiiOsdMenuSetItemCheck( winRef, 0, OPT_SHOW | (optWin.ipvAllowed ? OPT_SET_CHECK : 0) );
                    break;
                case 1:
                    optWin.ipvHighlight = !optWin.ipvHighlight;
                    SiiOsdMenuSetItemCheck( winRef, 1, OPT_SHOW | (optWin.ipvHighlight ? OPT_SET_CHECK : 0) );
                    break;
                default:
                    break;
            }
            break;

        case UFA_CLOSE:
        case UFA_BACK:
            // Save results when normal exit from window
            app.ipvAllowed      = optWin.ipvAllowed;
            app.ipvHighlight    = optWin.ipvHighlight;
            SiiIpvHighlightEnable( app.ipvHighlight );
        case UFA_CANCEL:

            // SIIKEY_CURSOR_LEFT and SIIKEY_CANCEL have the same effect for this window, which is to return to the
            // main window.
            SkAppOsdRestoreMainMenu( winRef, pWinInfo->childWinRef );
            break;
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//-------------------------------------------------------------------------------------------------
void SkAppOsdOptionSelect ( void )
{
    if ( optWin.winRef == -1 )
    {
        SkAppOsdCloseMainMenu();

        optWin.winRef = SiiOsdCreateWindow( LRID2_OPT_SEL, 0, 0, 0, 0, NULL, OPT_TOP );
        if ( optWin.winRef >= 0 )
        {
            SiiOsdSetFocus( optWin.winRef, OPT_SEND_UFA_FOCUS );
            SiiOsdShowWindow( optWin.winRef, OPT_SHOW );
            SiiOsdEnableOsdDisplay( true );
        }
    }
}

