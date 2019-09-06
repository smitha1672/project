//***************************************************************************
//!file     sk_app_osd_portsel.c
//!brief    OSD Port Select Window implementation
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
#include "si_drv_switch.h"

//-------------------------------------------------------------------------------------------------
// Enumeration and defines
//-------------------------------------------------------------------------------------------------

extern char *pSourceNames [];

//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------

SiiPortWinStruct_t portWin;

//-------------------------------------------------------------------------------------------------
//! @brief      Display the menu entries
//! @param[in]  winRef      Window reference value for Info Display Window
//-------------------------------------------------------------------------------------------------
static void DisplayInputMenu ( int winRef )
{
    uint16_t    activeMask;
    int         row;

    activeMask = SiiDrvSwitchStateGet( SiiSwitch_INPUT_PORTS_CONNECT_STATE );
    for ( row = 0; row < SII_INPUT_PORT_COUNT; row++ )
    {
        SiiOsdSetRowEnable( winRef, row, ( activeMask & (1 << row )) != 0 );
        SiiOsdWriteText( winRef, row, 1, pSourceNames[ row], OPT_CENTER_TEXT );
    }

    SiiOsdSetRowEnable( winRef, 4, 1 );
    SiiOsdWriteText( winRef, 4, 1, pSourceNames[ 4], OPT_CENTER_TEXT );
    SiiOsdSetRowEnable( winRef, 4, 0 );
    SiiOsdSetRowEnable( winRef, 5, 1 );
    SiiOsdWriteText( winRef, 5, 1, pSourceNames[ 4], OPT_CENTER_TEXT );
    SiiOsdSetRowEnable( winRef, 5, 0 );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
bool_t UserFuncPortSelect ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t      result  = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            DisplayInputMenu( winRef );
            SiiOsdSetCursor( winRef, SiiDrvSwitchStateGet( SiiSwitch_SELECTED_PORT ), OPT_SHOW );
            break;

        case UFA_FOCUS:
            DisplayInputMenu( winRef );
            break;

        case UFA_SELECT:
            if(pWinInfo->selectedRow < SII_INPUT_PORT_COUNT)
                app.newSource[0] = pWinInfo->selectedRow;   // Change to selected port
            // Fall-through to UFA_CLOSE
        case UFA_CLOSE:
        case UFA_CANCEL:

            portWin.winRef = -1;
            break;
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//-------------------------------------------------------------------------------------------------
void SkAppOsdPortSelect ( void )
{
    if ( !app.isOsdMenuEnabled )
    {
        if ( portWin.winRef == -1 )
        {
            portWin.winRef = SiiOsdCreateWindow( LRID1_PORT_SEL, 0, 0, 0, 0, NULL, OPT_TOP );
            if ( portWin.winRef >= 0 )
            {
                SiiOsdSetFocus( portWin.winRef, OPT_SEND_UFA_FOCUS );
                SiiOsdShowWindow( portWin.winRef, OPT_SHOW );
                SiiOsdEnableOsdDisplay( true );
            }
        }
        else
        {
            // Cycle through port selections
            SiiOsdWindowManager( SIIKEY_CURSOR_DOWN, 0 );
        }
    }
}

