//***************************************************************************
//!file     sk_app_osd.c
//!brief    Wraps platform and device functions for the OSD component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "sk_app_osd.h"

//-------------------------------------------------------------------------------------------------
// Enumeration and defines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------
SiiOsdAppDataStruct_t osdApp;

//------------------------------------------------------------------------------
//! @brief  Display a simple dialog box in the center of the screen with the
//!         passed message and an 'Error' title.  The dialog does not go away
//!         until the enter key is pressed.
//! @param[in]  message     ASCII null terminated string.
//------------------------------------------------------------------------------
void SkAppOsdErrorDialog ( char *pMessage )
{
    int winRef;

    if (app.isOsdMenuEnabled)
    {
        SkAppOsdCloseMainMenu();
    }
    winRef = SiiOsdMessageBox( 0, "Error", pMessage, 0, 0, 0, OPT_CENTER_WINDOW | OPT_CENTER_TEXT );
    if ( winRef != -1 )
    {
        SiiOsdSetFocus( winRef, OPT_TOP );
        SiiOsdEnableOsdDisplay( true );
    }
}

//------------------------------------------------------------------------------
//! @brief  Display a simple dialog box in the center of the screen with the
//!         passed message and title.  The dialog does not go away
//!         until the enter key is pressed or the passed non-zero timeout has
//!         been reached.
//! @param[in]  pTitle      ASCII null terminated string.
//! @param[in]  pMessage    ASCII null terminated string.
//! @param[in]  msTimeout   timeout value.  If 0, window will not time out.
//------------------------------------------------------------------------------
void SkAppOsdmessage ( char *pTitle, char *pMessage, int msTimeout )
{
    int winRef;

    winRef = SiiOsdMessageBox( 0, pTitle, pMessage, 0, 0, msTimeout, OPT_CENTER_WINDOW | OPT_CENTER_TEXT );
    if ( winRef != -1 )
    {
        SiiOsdSetFocus( winRef, OPT_TOP );
        SiiOsdEnableOsdDisplay( true );
    }
}

//------------------------------------------------------------------------------
//! @brief      Update active OSD menu if required by resolution change
//------------------------------------------------------------------------------
void SkAppOsdMenuUpdate ( void )
{

    if ( app.isOsdMenuEnabled && (osdApp.winRefLeft != -1 ))
    {
        // Send the UPDATE message to the left menu window.
        // It will update the right window if necessary
        SiiOsdSendMessage( osdApp.winRefLeft, UFA_UPDATE, 0 );
    }
}

//------------------------------------------------------------------------------
//! @brief      Parse remote control keys for the OSD application.
//! @param[in]  key         Remote control key value
//! @param[in]  fullAccess  true if allowed to respond to all normal keys
//!                         false if only allowed to respond to power keys
//! @retval     true if key was used
//------------------------------------------------------------------------------
bool_t SkAppOsdIrRemote ( RC5Commands_t key, bool_t fullAccess )
{
    bool_t keyUsed = false;

    if ( fullAccess )
    {	
        switch ( key )
        {
         	case RC5_MENU:
        	    if (app.isOsdMenuEnabled)
        	    {
        	        SkAppOsdCloseMainMenu();
        	    }
        	    else
        	    {
        	        SkAppOsdMainMenu();
        	    }
                keyUsed = true;
				break;
            case RC5_ENTER:
            case RC5_CURSOR_UP:
            case RC5_CURSOR_DOWN:
            case RC5_CURSOR_LEFT:
            case RC5_CURSOR_RIGHT:
                keyUsed = (SiiOsdWindowManager( key, 0 ) != 0);
                break;
            default:
                break;
        }
    }

    return( keyUsed );
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskOsd
// Description:  Main task of the osd.
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
void SkAppTaskOsd ( void )
{

    // If zone 1 is displaying an HDMI input, check for input resolution changes
    if ( app.currentSource[0] < SiiSwitchSourceInternalVideo )
    {
        SiiDrvOsdHandleInterrupts();        // Check for resolution changes
    }

#if INC_OSD_BASE_HANDLER
        SiiOsdWindowManager( -1, 0 );   // Check window timers
#endif // INC_OSD_BASE_HANDLER

}

//------------------------------------------------------------------------------
// Function:    SkAppDeviceInitOsd
// Description:  Osd Init
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
bool_t SkAppDeviceInitOsd ( void )
{
    bool_t success = false;

	memset( &osdApp, 0, sizeof(SiiOsdAppDataStruct_t));
    memset( &infoWin, 0, sizeof(SiiInfoWinStruct_t));
    memset( &volInfo, 0, sizeof(SiiVolInfoStruct_t));
    infoWin.winRef      = -1;
    volInfo.winRef      = -1;
    portWin.winRef      = -1;
    optWin.winRef       = -1;
    osdApp.winRefLeft   = -1;       // Left (main) window
    osdApp.winRefTop    = -1;       // Top (menu title/path)
    osdApp.winRefRight  = -1;       // Right (sub-menu, if needed)
    osdApp.winRefBottom = -1;       // Bottom (context-sensitive help)

    SiiOsdEnableOsdDisplay( false );    // Force all off

    // Point the driver resource handler to our local (non-flash) resource array.
    SiiDrvOsdSetLocalResourceData( OSD_LOCAL_RESOURCE_COUNT, &osdResourceData[0] );
    // MUST initialize the desired font resource IDs prior to calling SiiOSDInitialize()
    SiiDrvOsdSetFontResourceId( VAL_OSD_FONT_12x16, RID35_FONT1_12x16 );
    SiiDrvOsdSetFontResourceId( VAL_OSD_FONT_16x24, RID34_FONT0_16x24 );

    // Use the 16x24 font for 1080p because the 24x32 font has no room for icons.
    SiiDrvOsdSetFontResourceId( VAL_OSD_FONT_24x32, RID34_FONT0_16x24 );    //, RID36_FONT3_24x32 );
    if ( SiiOsdInitialize( SI_UF_TOTAL_COUNT, (SiiOsdUserFunction_t *)&g_UserFuncs ))
    {
        SkAppOsdInitializeDemo();
        success = true;
    }

	return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief  OSD driver callback function to translate character value into font character index.
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdCbCharacterToIndex( int lookupChar )
{
    lookupChar = (lookupChar == 0) ? ' ' : (lookupChar < ' ') ? SII_OSD_CHAR_ILLEGAL_CHAR : lookupChar;
    return( lookupChar - ' ' );
}

//-------------------------------------------------------------------------------------------------
//! @brief  OSD driver callback function to translate font character index into character value.
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdCbIndexToCharacter( int lookupIndex )
{
    return( lookupIndex + ' ' );
}
