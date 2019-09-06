//***************************************************************************
//!file     sk_app_osd_demo.c
//!brief    OSD attributes demo
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
#include "si_osd_config.h"


//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------

#if INC_OSD_SLIDER
static int  progress        = 0;
#endif // INC_OSD_SLIDER

static char *demoHelpText [] =
{
    "Port Info window",
    "Message Box",
    "Progress Bar",
    "Volume Control",
    "List Box",
    "Edit Box",
    "Window Attributes",
};
//-------------------------------------------------------------------------------------------------
//! @brief      Special user function for message boxes used in this demo.
//! @param[in]  winRef      Return value from the SiiOsdCreateWindow function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
bool_t UserFuncDemoMessageBox ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
//    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            break;
        case UFA_CLOSE:
            SkAppOsdRestoreOsdDemoMenu( winRef );
            result = false;
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
//! @brief      Restore the OSD Demo (left) menu window
//-------------------------------------------------------------------------------------------------
void SkAppOsdRestoreOsdDemoMenu( int winRef )
{
    if ( !osdApp.contextHelpOpen )
    {
        // Recreate context help window
        SiiOsdReplaceWindow( osdApp.winRefBottom, RID4_WINDOW5 );   // Remove child window.
        SiiOsdSetColor( osdApp.winRefBottom, OSD_FG_HIGHLIGHT, OSD_RGB( 0, 3, 3 ));
        SiiOsdShowWindow( osdApp.winRefBottom, OPT_SHOW );          // ReplaceWindow does not update display
        osdApp.contextHelpOpen = true;
    }
    SiiOsdSetFocus( osdApp.winRefLeft, OPT_TOP );           // Restore focus to main menu
}

//-------------------------------------------------------------------------------------------------
//! @brief      Progress bar demo user window function.
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
#if INC_OSD_SLIDER
bool_t UserFuncProgressDemo ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t  result  = true;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:

            progress    = 0;

            SiiOsdSetWindowTimeout( winRef, 75 );                   // Currently not set by IForm.
            SiiOsdSetWindowPrivateData( winRef, 0, 99, 1, 0, 0 );   // Currently not set by IForm.
            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            // For this demo, will will consider the close message to be a progress update
            // unless progress is complete, at which point we will delay one more timer
            // period and then close.
            if ( progress < 100 )
            {
                progress += 1; // Ignore volume control step value.
                SiiOsdSliderUpdate( winRef, progress, OPT_SHOW | OPT_TOP );
                if ( progress > 85 )
                {
                    SiiOsdSetWindowTimeout( winRef, 400 );  // Slow it down
                }
            }
            else
            {
                SkAppOsdRestoreOsdDemoMenu( winRef );
            }
            result = false;     // Don't close the window
            break;
        case UFA_ROW_ENTER:
            result = false;     // No children, so don't bother checking.
            break;
        default:
            break;
    }

    return( result );
}
#endif // INC_OSD_SLIDER

//-------------------------------------------------------------------------------------------------
//! @brief      Edit Box user window function.
//!             The SiiOsdEditBoxHandler sends only UFA_CREATE, UFA_CLOSE, UFA_SELECT,
//!             and UFA_CANCEL.
//!             action messages to this function.
//! @param[in]  winRef      Return value from the SiiOsdCreateEditBox function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
#if INC_OSD_EDITBOX
bool_t UserFuncEditBoxDemo ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t  result  = true;

    switch ( action )
    {
        case UFA_CREATE:
            break;
        case UFA_CLOSE:
        case UFA_SELECT:
        case UFA_CANCEL:

            SkAppOsdRestoreOsdDemoMenu( winRef );
            result = false;     // Don't close the window
            break;
        default:
            break;
    }

    return( result );
}
#endif // INC_OSD_EDITBOX

//-------------------------------------------------------------------------------------------------
//! @brief      OSD Attributes Demo menu
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncOsdAttributesDemo ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:

            result = true;      // Replace or Create was successful
            break;
        case UFA_ROW_ENTER:
            result = false;
            break;

        case UFA_SELECT:

            result = false;                 // Don't kill the main menu window
            break;

        case UFA_BACK:
        case UFA_CANCEL:
        case UFA_CLOSE:
            SkAppOsdRestoreOsdDemoMenu( winRef );
            result = false;     // Don't close the window
            break;
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      OSD Demo menu
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncOsdDemo ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;
    char            *pMsg;
    bool_t          interlaced;
    int_t           horiz, vert;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
            // Use saved former cursor row.
            SiiOsdSetCursor( winRef, osdApp.cursorOsdDemo, OPT_SHOW );
            // Fall through...
        case UFA_CREATE:
            // Update bread crumb location
            SiiOsdWriteText( osdApp.winRefTop, 2, 4, "OSD DEMO", OPT_CLEAR_TO_END );
            SiiOsdWriteIcon( osdApp.winRefTop, 1, 1, 2, 2, GEARS_ICON, 0 );

            SiiOsdWriteText( winRef, 4, 1, "          ", OPT_SHOW); /* Masking the OSD DEMO "List Box" */
            SiiOsdSetRowEnable( winRef,4,false );    /* Disableing the OSD DEMO "List Box" */

            SiiOsdWriteText( winRef, 6, 1, "                  ", OPT_SHOW); /* Masking the OSD DEMO "Window Attributes"  */


            UpdateMenuHelp( demoHelpText, KH_SUB1_KEYS, osdApp.cursorOsdDemo );

            result = true;      // Replace or Create was successful
            break;
        case UFA_ROW_ENTER:
            UpdateMenuHelp( demoHelpText, KH_SUB1_KEYS, pWinInfo->selectedRow );
            osdApp.cursorOsdDemo = pWinInfo->selectedRow;

            // For this custom composite menu, we handle row entry differently from normal.
            // Instead of closing an existing child window for this menu, replace it with the
            // child window for the passed row.  This will avoid background flashes during the removal of the
            // existing window and the creation of the new window. Returning false prevents the handler from
            // closing the existing window.
            SiiOsdReplaceWindow( pWinInfo->childWinRef, (pWinInfo->selectedRow == 6) ? RID30_WINDOW19_SI_UF_OSDATTR : RID5_WINDOW6_SI_UF_BLANKSUBMENU );
            SiiOsdShowWindow( pWinInfo->childWinRef, OPT_SHOW );   // ReplaceWindow does not update display
            result = false;
            break;

        case UFA_BACK:
        case UFA_CANCEL:
            // SIIKEY_CURSOR_LEFT and SIIKEY_CANCEL have the same effect for this window, which is to return to the
            // main window.
            if ( !osdApp.contextHelpOpen )
            {

                // Force any open demo to close.
                if ( osdApp.winRefBottom != -1 )
                {
                    SiiOsdCloseWindow( osdApp.winRefBottom, OPT_CANCEL );
                    result = false;                 // Don't kill the main menu window
                    break;
                }

                osdApp.contextHelpOpen = true;
            }
            SkAppOsdRestoreMainMenu( winRef, pWinInfo->childWinRef );
            break;

        case UFA_SELECT:

            // To do these tests, we need an available window, so close the help window
            if ( osdApp.contextHelpOpen )
            {
                SiiOsdCloseWindow( osdApp.winRefBottom, OPT_CANCEL );
                osdApp.winRefBottom     = -1;
                osdApp.contextHelpOpen  = false;
            }

            // Force any open demo to close.
            if ( osdApp.winRefBottom != -1 )
            {
                SiiOsdCloseWindow( osdApp.winRefBottom, OPT_CANCEL );
                result = false;                 // Don't kill the main menu window
                break;
            }
            switch ( pWinInfo->selectedRow )
            {
                case 0:             // Port Info Window
                    // Need main pipe info to determine where to display window
                    // Use default font width for placement because the value will be scaled
                    // to the current font width.
                    if ( SiiOsdCbGetOutputRes( 0, &horiz, &vert, &interlaced ))
                    {
                        osdApp.winRefBottom = SiiOsdCreateWindow(
                            RID32_WINDOW21_SI_UF_INFO,
                            horiz - ((OSD_INFO_COLS + 4) * OSD_DEFAULT_FONT_WIDTH),     // Place at right top of screen
                            OSD_DEFAULT_FONT_HEIGHT,                                    // Place at right top of screen
                            0, 0, NULL, OPT_SHOW | OPT_TOP );
                        SiiOsdShowWindow( osdApp.winRefBottom, OPT_SHOW );
                        SiiOsdSetFocus( osdApp.winRefBottom, OPT_SHOW );                // Don't let main menu continue to get user input
                    }
                    break;
                case 1:             // Message Box
                    if ( strlen( osdApp.itemString) != 0 )
                    {
                        pMsg = osdApp.itemString;
                    }
                    else
                    {
                        pMsg = "Test Message Box";
                    }
                    osdApp.winRefBottom = SiiOsdMessageBox(
                        RID31_WINDOW20_SI_UF_DEMO_MSGBOX,
                        NULL, pMsg,
                        100, 100, 2000, OPT_TOP | OPT_CENTER_WINDOW );
                    SiiOsdSetFocus( osdApp.winRefBottom, OPT_TOP ); // Give the window focus to take it away from the main window
                    break;
                case 2:             // Progress Bar
                    osdApp.winRefBottom = SiiOsdCreateWindow( RID25_WINDOW14_SI_UF_PROGRESS_DEMO, 300, 350, 0, 0, NULL, 0 );
                    SiiOsdShowWindow( osdApp.winRefBottom, OPT_SHOW | OPT_CENTER_WINDOW );
                    SiiOsdSetFocus( osdApp.winRefBottom, OPT_TOP ); // Give the window focus to take it away from the main window
                    break;
                case 3:             // Slider Control
                    osdApp.winRefBottom = SiiOsdCreateWindow(
                        LRID3_VOL_CTRL,                 // Replace SPI flash resource RID24_WINDOW13_SI_UF_VOLUME_CTRL,
                        OSD_DEFAULT_FONT_WIDTH * 3,
                        OSD_DEFAULT_FONT_HEIGHT * 2,
                        0, 0, NULL, 0 );
                    SiiOsdShowWindow( osdApp.winRefBottom, OPT_SHOW | OPT_CENTER_WINDOW);
                    SiiOsdSetFocus( osdApp.winRefBottom, OPT_TOP ); // Window has to have focus to let window manager handle it for key input.
                    break;
                case 4:             // List Box
                    osdApp.winRefBottom = SiiOsdCreateWindow( RID26_WINDOW15_SI_UF_LISTBOX_DEMO, 0, 0, 0, 0, NULL, 0 );
                    SiiOsdShowWindow( osdApp.winRefBottom, OPT_SHOW | OPT_CENTER_WINDOW );
                    SiiOsdSetFocus( osdApp.winRefBottom, OPT_TOP );
                    break;
                case 5:             // Edit Box
                    memset( osdApp.itemString, 0, sizeof( osdApp.itemString));
                    osdApp.winRefBottom = SiiOsdCreateEditBox( -1, RID0_WINDOW1_SI_UF_EDITBOX, 0, 0, osdApp.itemString, sizeof( osdApp.itemString), 0 );
                    SiiOsdShowWindow( osdApp.winRefBottom, OPT_SHOW );
                    SiiOsdSetFocus( osdApp.winRefBottom, OPT_TOP );
                    break;
                case 6:             // Attributes
                    break;
            }
            result = false;                 // Don't kill the main menu window
            break;
        default:
            break;
    }

    return( result );
}

