//***************************************************************************
//!file     sk_app_osd_avr.c
//!brief    OSD example AVR implementation
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "sk_application.h"
#include "si_osd_component.h"
#include "sk_app_osd.h"

#include "si_osal_timer.h"
#include "si_timer.h"

#include "si_drv_msw_internal.h"
#include "si_platform.h"
#include "si_drv_switch.h"
#if INC_SPI
#include "si_drv_spi.h"
#endif
//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------

#if INC_OSD_EDITBOX
static int  winRefEdit      = -1;
#endif // INC_OSD_EDITBOX

#if INC_OSD_SLIDER
static int  progress        = 0;
#endif // INC_OSD_SLIDER

// Left windows chosen by the main left window. (Press Enter in the main window.)
static const int leftMainWindowIds [] =
{
    RID22_WINDOW11_SI_UF_VIDEOSETUP,
    RID28_WINDOW17_SI_UF_AUDIOSETUP,
    RID29_WINDOW18_SI_UF_OSDDEMO,
    RID7_WINDOW8_SI_UF_SPEAKERSETUP,
    0,
    LRID2_OPT_SEL,
    0,
    0,
};

static char *mainHelpText [] =
{
    "Video Setup",
    "Audio Setup",
    "OSD Demo",
    "Speaker Setup",
    "         ",
    "Set Options",
};

char *keyHelpText [] =
{
    "",
    "\x8E Exit \x80 Select \x8C\x8D Move",              // KH_TOP_KEYS
    "\x8E Back \x80 Select \x8F\x8C\x8D Move",          // KH_SUB_KEYS
    "\x8E Back \x80 Select \x8C\x8D Move",              // KH_SUB1_KEYS
    "\x8E Back \x8F Modify \x8C\x8D Move",              // KH_MOD_KEYS
    "\x8E Back \x80 Modify \x8C\x8D Move",              // KH_MOD1_KEYS
    "\x8E\x8F Change value, \x80\x8C\x8D Exit edit",    // KH_EDIT_KEYS
};

//-------------------------------------------------------------------------------------------------
//! @brief      Update the context sensitive help display
//! @param[in]  resourceId  Specifies which window to get help for.
//! @param[in]  selectedRow Specifies the row within the window to get help for.
//-------------------------------------------------------------------------------------------------
void UpdateMenuHelp( char *pHelpText[], int keyHelpIndex, int helpIndex )
{
    int helpTextRow;

    if ( osdApp.winRefBottom == -1 )
    {
        return;
    }

    helpTextRow = 2;
    if ( helpIndex != HLP_NONE )
    {
        SiiOsdFillWindow( osdApp.winRefBottom, RID4_WINDOW5 );
    }
    if ( keyHelpIndex == KH_NONE)
        {
            helpTextRow = 1;
        }
    else
    {
        SiiOsdWriteText( osdApp.winRefBottom, 1, 0, keyHelpText[ keyHelpIndex], OPT_SHOW | OPT_CLEAR_TO_END );
    }

    if (( helpIndex != HLP_NONE ) && pHelpText )
    {
        SiiOsdWriteText( osdApp.winRefBottom, helpTextRow, 0, pHelpText[ helpIndex], OPT_WRAP_TEXT | OPT_SHOW );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Blank window control.  This is used to replace an existing window with the
//!             blank window so that the window itself is never closed, which would ruin the
//!             look of the compound setup window.
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
bool_t UserFuncBlankSub( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            SiiOsdSetColor( winRef,  OSD_FG, OSD_RGB( 0, 3, 3 ));
            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            result = false;     // Don't close the window
            break;
        case UFA_ROW_ENTER:
            result = false;     // A composite sub-menu has no children, so don't bother checking.
            break;
        case UFA_FOCUS:
            SiiOsdSetFocus( pWinInfo->parentWinRef, OPT_SHOW ); // Send focus back where it came from
            break;
        case UFA_SELECT:
        case UFA_CANCEL:
        case UFA_UPDATE:        // Slider/progress only.
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Main menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncLeftMain ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
            // Use saved former cursor row.
            SiiOsdSetCursor( winRef, osdApp.cursorLeftMain, OPT_SHOW );
            // Fall through...
        case UFA_CREATE:
            // Update bread crumb location
            SiiOsdWriteText( osdApp.winRefTop, 2, 4, "SYSTEM SETUP", OPT_CLEAR_TO_END );
            SiiOsdWriteIcon( osdApp.winRefTop, 1, 1, 2, 2, GEARS_ICON, 0 );

            SiiOsdWriteText( winRef, 4, 1, "               ", 0 );

            // Adding a new menu entry NOT in the external flash resource
            SiiOsdWriteText( winRef, 5, 1, "Options", 0 );
            SiiOsdSetRowEnable( winRef, 5, true );

            UpdateMenuHelp( mainHelpText, KH_TOP_KEYS, osdApp.cursorLeftMain );

            result = true;      // Replace or Create was successful
            break;
        case UFA_ROW_ENTER:
            UpdateMenuHelp( mainHelpText, KH_TOP_KEYS, pWinInfo->selectedRow );
            osdApp.cursorLeftMain = pWinInfo->selectedRow;

            // For this custom composite menu, we handle row entry differently from normal.
            // Normally, any child window from the previous menu row would be closed. Because the main menu
            // submenus are all the same (blank), don't close the previous child window.
            result = false;
            break;

        case UFA_SELECT:
            // Normally, the calling function would close this window upon successful selection, but for this
            // custom composite menu, we actually want to replace the main menu (left) window with a new menu
            // window (the selection). This avoids the background flashes that would occur when this window
            // is closed and the selected window is opened.
            if ( leftMainWindowIds[ pWinInfo->selectedRow] != 0)
            {
                result = !SiiOsdReplaceWindow( winRef, leftMainWindowIds[ pWinInfo->selectedRow] );

                // Note that at this point, winRef represents the NEW window, not us.

                SiiOsdShowWindow( winRef, OPT_SHOW );   // ReplaceWindow does not update display
            }
            SiiOsdSetFocus( winRef, 0 );    // Restore select cursor
            result = false;                 // Don't kill the main menu window, because it is now the new menu
            break;

        case UFA_BACK:
        case UFA_CANCEL:
            SkAppOsdCloseMainMenu();        // Close the window, but don't save any changes.
            break;
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//-------------------------------------------------------------------------------------------------
void SkAppOsdCloseMainMenu ( void )
{
    if ( app.isOsdMenuEnabled )
    {
        SiiOsdEnableOsdDisplay( false );    // Hide all windows
        SiiOsdCloseWindow( osdApp.winRefLeft, OPT_CANCEL | OPT_FORCE_CLOSE );
        SiiOsdCloseWindow( osdApp.winRefTop, OPT_CANCEL | OPT_FORCE_CLOSE );
        SiiOsdCloseWindow( osdApp.winRefBottom, OPT_CANCEL | OPT_FORCE_CLOSE );
        SiiOsdCloseWindow( osdApp.winRefRight, OPT_CANCEL | OPT_FORCE_CLOSE );
        osdApp.winRefLeft   = -1;
        osdApp.winRefTop    = -1;
        osdApp.winRefRight  = -1;
        osdApp.winRefBottom = -1;
        osdApp.contextHelpOpen = false;
        app.isOsdMenuEnabled = false;
        SkAppIrRemoteReturnToTV();  // Return to parse keys as direct DTV control keys
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief
//-------------------------------------------------------------------------------------------------
void SkAppOsdMainMenu ( void )
{
    SiiTimer_t  timerInfo;
    bool_t      success = false;

    SiiOsTimerSet( &timerInfo, 0 );
    if ( osdApp.winRefLeft == -1 )
    {
        // Create the composite menu system, which consists of all 4 OSD windows
        do {
            SiiOsdEnableOsdDisplay( false );    // Hide all windows
            SiiOsdCloseAll();                   // Close all windows that may be open.


            // Top window, contains logo and breadcrumb menu path
            osdApp.winRefTop = SiiOsdCreateWindow( RID3_WINDOW4, 0, 0, 0, 0, NULL, 0 );
            if ( osdApp.winRefTop == -1 )
            {
                break;
            }
            SiiOsdSetColor( osdApp.winRefTop, OSD_FG, OSD_RGB( 0, 3, 3 ));
            SiiOsdSetColor( osdApp.winRefTop, OSD_FG_HIGHLIGHT, OSD_RGB( 0, 3, 3 ));

            // Bottom window, displays mini-help for current main menu selection
            osdApp.winRefBottom = SiiOsdCreateWindow( RID4_WINDOW5, 0, 0, 0, 0, NULL, 0 );
            if ( osdApp.winRefBottom == -1 )
            {
                break;
            }
            osdApp.contextHelpOpen = true;

            // Main Menu window.  Create it here because it writes to top and bottom windows
            // during its CREATE function.
            osdApp.winRefLeft = SiiOsdCreateWindow( RID2_WINDOW3_SI_UF_MAINMENU, 0, 0, 0, 0, NULL, 0 );
            if ( osdApp.winRefLeft == -1 )
            {
                break;
            }

            // Right menu, displays the sub-menu of current left menu selection, if any.
            // Start with it blank, because the Main menu does not have sub-menus.
            osdApp.winRefRight = SiiOsdCreateWindow( RID5_WINDOW6_SI_UF_BLANKSUBMENU, 0, 0, 0, 0, NULL, 0 );
            if ( osdApp.winRefRight == -1 )
            {
                break;
            }
            // Attach the sub-menu window as a child of the Main Menu window.
            SiiOsdAssignChildWinRef( osdApp.winRefLeft, osdApp.winRefRight );

            // Color the bar in the context help window (no user function)
            SiiOsdSetColor( osdApp.winRefBottom, OSD_FG_HIGHLIGHT, OSD_RGB( 0, 3, 3 ));
            success = true;
        } while (0);
    }
    if ( !success )
    {
        app.isOsdMenuEnabled = true;    // Mark as enabled so that all opened windows will be closed
        SkAppOsdCloseMainMenu();
        SkAppOsdErrorDialog( "Unable to open Main Menu" );
    }
    else
    {
        // Show the total creation
        osdApp.winRefLeft = SiiOsdSetFocus( osdApp.winRefLeft, OPT_TOP );
        SiiOsdShowWindow( osdApp.winRefLeft, OPT_SHOW );
        SiiOsdShowWindow( osdApp.winRefTop, OPT_SHOW );
        SiiOsdShowWindow( osdApp.winRefBottom, OPT_SHOW );
        SiiOsdShowWindow( osdApp.winRefRight, OPT_SHOW );
        SiiOsdEnableOsdDisplay( true );
        app.isOsdMenuEnabled = true;
    }
    DEBUG_PRINT( MSG_DBG, "DoOsdMenu: %d ms\n", SiiOsTimerElapsed( timerInfo ));

}

//-------------------------------------------------------------------------------------------------
//! @brief      Restore the main (left) menu window
//-------------------------------------------------------------------------------------------------
bool_t SkAppOsdRestoreMainMenu( int winRef, int childWinRef )
{
    bool_t result;

    SiiOsdReplaceWindow( childWinRef, RID5_WINDOW6_SI_UF_BLANKSUBMENU );  // Remove child window.
    result = !SiiOsdReplaceWindow( winRef, RID2_WINDOW3_SI_UF_MAINMENU );   // Restore main menu
    SiiOsdShowWindow( winRef, OPT_SHOW );                   // ReplaceWindow does not update display
    SiiOsdSetFocus( winRef, 0 );                            // Restore select cursor

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Move a dot select marker
//-------------------------------------------------------------------------------------------------
void SkAppOsdMoveDotMarker( int winRef, int oldRow, int newRow )
{
    if ( oldRow != -1 )
    {
        SiiOsdWriteText( winRef, oldRow, 1, " ", 0 );               // Remove former select marker
    }
    if ( newRow != -1 )
    {
        SiiOsdWriteText( winRef, newRow, 1, "\x8B", OPT_SHOW );     // SII_OSD_CHAR_DOT_MARKER
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize demo system variables.
//-------------------------------------------------------------------------------------------------
void SkAppOsdInitializeDemo ( void )
{
    osdApp.winRefLeft   = -1;
    osdApp.winRefTop    = -1;
    osdApp.winRefRight  = -1;
    osdApp.winRefBottom = -1;

#if INC_OSD_LISTBOX
    winRefList      = -1;
#endif // INC_OSD_LISTBOX
#if INC_OSD_EDITBOX
    winRefEdit      = -1;
#endif // INC_OSD_EDITBOX

#if INC_OSD_SLIDER
    progress        = 0;
#endif // INC_OSD_SLIDER

    SkAppOsdInitializeSpeakerSetupDemo();
}
