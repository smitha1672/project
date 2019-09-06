//***************************************************************************
//!file     sk_app_osd_video.c
//!brief    OSD Video control window functions
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
#include "si_drv_tpi_system.h"

//-------------------------------------------------------------------------------------------------
// Enumeration and defines
//-------------------------------------------------------------------------------------------------

#define IV_FORMAT_ROW_480P      0
#define IV_FORMAT_ROW_576P      1
#define IV_FORMAT_ROW_720P      2

#define VIDEO_MODE_EXTERNAL     0
#define VIDEO_MODE_INTERNAL     1
#define PARALLEL_VIDEO          2
#define VIDEO_MODE_PATTERNS     3   // Not technically a video mode
#define VIDEO_MODE_FAKE         4
#define AV_MUTE                 5

// Sub menus for the Video Setup Main window
static const int videoSetupSubMenuIds [] =
{
    RID5_WINDOW6_SI_UF_BLANKSUBMENU,
    RID23_WINDOW12_SI_UF_VIDEOSETUP_IV,
    RID5_WINDOW6_SI_UF_BLANKSUBMENU,
    RID27_WINDOW16_SI_UF_VIDEOSETUP_IV_PATTERN,
    RID5_WINDOW6_SI_UF_BLANKSUBMENU,
    RID5_WINDOW6_SI_UF_BLANKSUBMENU
};

static char *videoHelpText [] =
{
    "Enable External Video",
    "Enable Internal Video",
    "Select background color",
    "Select video resolution",
    "Select Fake video",
    "Select A/V mute",
};

//-------------------------------------------------------------------------------------------------
//! @brief      Video Setup Internal Video pattern select sub menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncVideoSetupIvPatternSelect( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            // Display 'selected' marker
            SkAppOsdMoveDotMarker( winRef, -1, osdApp.ivPatternSelect );
            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            SiiDrvExtTpgPatternSet( osdApp.ivPatternSelect );   // Close with the 'selected' pattern
            break;
        case UFA_ROW_ENTER:

            // Show the pattern for this row, but don't officially select it.
           // SiiDrvExtTpgPatternSet( pWinInfo->selectedRow );
            result = false;     // A composite sub-menu has no children, so don't bother checking.
            break;

        case UFA_FOCUS:
           // Display help text for this menu only when it gets focus.
          //  pWinInfo->selectedRow = SiiOsdMoveCursor( winRef, false, OPT_SHOW );
        	pWinInfo->selectedRow = SiiOsdGetCursor(winRef);
        	SiiOsdSetCursor( winRef, pWinInfo->selectedRow, OPT_SHOW );
            result = false;
            break;


        case UFA_SELECT:
            // Update and display 'selected' marker
            SkAppOsdMoveDotMarker( winRef, osdApp.ivPatternSelect, pWinInfo->selectedRow );
            SiiDrvExtTpgPatternSet( pWinInfo->selectedRow );
            osdApp.ivPatternSelect = pWinInfo->selectedRow;

            // Perform requested menu command
            app.tpgVideoPattern = osdApp.ivPatternSelect;
            SiiDrvExtTpgPatternSet( osdApp.ivPatternSelect );

            result = false;     // Stay in sub-menu until the SIIKEY_CURSOR_LEFT is pressed.
            break;
        case UFA_CANCEL:
            result = false;     // Stay in sub-menu until the SIIKEY_CURSOR_LEFT is pressed.
            break;
        case UFA_UPDATE:        // Slider/progress only.
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Video Setup Internal Video sub menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncVideoSetupIv( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
        case UFA_UPDATE:
           //  Enable 720p option only if a TMDS source is present
            SiiOsdSetRowEnable(
                winRef, IV_FORMAT_ROW_720P,
                (( SiiDrvSwitchStateGet( SiiSwitch_INPUT_PORTS_CONNECT_STATE ) != 0 ) ? true : false ));

            // If formerly selected row is not valid, move to next valid selection.
            if ( !SiiOsdIsRowEnabled( winRef, osdApp.ivFormatSelect ))
            {
                osdApp.ivFormatSelect = SiiOsdMoveCursor( winRef, true, OPT_SHOW );
            }
            // Display 'selected' marker
            SkAppOsdMoveDotMarker( winRef, -1, osdApp.ivFormatSelect );

            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            break;

        case UFA_ROW_ENTER:
            result = false;     // A composite sub-menu has no children, so don't bother checking.
            break;
        case UFA_FOCUS:
            // Display help text for this menu only when it gets focus.
            //UpdateMenuHelp( videoHelpText, KH_SUB1_KEYS, 4 );
            break;
        case UFA_SELECT:
            // Update and display 'seleSiiDrvSwitchStateGetcted' marker
            SkAppOsdMoveDotMarker( winRef, osdApp.ivFormatSelect, pWinInfo->selectedRow );
            osdApp.ivFormatSelect = pWinInfo->selectedRow;

            // Perform requested menu command
            switch ( pWinInfo->selectedRow )
            {
                case IV_FORMAT_ROW_480P:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_480_60;
                    app.tpgClockSrc = SI_TPG_CLK_XCLK;
                    break;
                case IV_FORMAT_ROW_576P:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_576_50;
                    app.tpgClockSrc = SI_TPG_CLK_XCLK;
                    break;
                case IV_FORMAT_ROW_720P:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_720_60;
                    // Use APLL1 (SWWA:22565)
                    app.tpgClockSrc = SI_TPG_CLK_P0;
                    break;
            }
           SkAppServiceNonHdmiVideo( true );

            result = false;     // Stay in sub-menu until the SIIKEY_CURSOR_LEFT is pressed.
            break;
        case UFA_CANCEL:
            result = false;     // Stay in sub-menu until the SIIKEY_CURSOR_LEFT is pressed.
            break;
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Video Setup menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 Varies by action. See documentation
//-------------------------------------------------------------------------------------------------
bool_t UserFuncVideoSetup( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            SiiOsdWriteText( osdApp.winRefTop, 2, 4, "VIDEO SETUP", OPT_CLEAR_TO_END );
            SiiOsdWriteIcon( osdApp.winRefTop, 1, 1, 2, 2, BOX_AND_GEAR_ICON, 0 );

            DEBUG_PRINT(MSG_ALWAYS, "\n\n OsdApp.videoTypeSelect: %d \n",osdApp.videoTypeSelect);

            // If formerly selected row is not valid, move to next valid selection.
            if ( !SiiOsdIsRowEnabled( winRef, osdApp.videoTypeSelect ))
            {
                if(osdApp.videoTypeSelect != AV_MUTE)
                    osdApp.videoTypeSelect = SiiOsdMoveCursor( winRef, true, OPT_SHOW );
            }

            // Disable Parallel video pattern select command as needed for melbourne...
            SiiOsdWriteText( winRef, 2, 1, "               ", OPT_SHOW);
            SiiOsdSetRowEnable( winRef,PARALLEL_VIDEO,false );
            //Enable fake vedio pattern select command as needed for melbourne
            SiiOsdWriteText( winRef, 4, 2, "Fake Video", OPT_SHOW);
          // SiiOsdSetRowEnable( winRef,VIDEO_MODE_FAKE,true );
            SiiOsdWriteText( winRef, 5, 2, "A/V Mute", OPT_SHOW);

            // Display 'selected' video type marker and enable/disable pattern select command as needed
            switch ( osdApp.videoTypeSelect )
            {
                case VIDEO_MODE_EXTERNAL:
                    SiiOsdSetRowEnable( winRef,AV_MUTE, true );
                	break;
                case VIDEO_MODE_INTERNAL:
                    SiiOsdSetRowEnable( winRef, VIDEO_MODE_PATTERNS, true );
                   	SiiOsdSetRowEnable( winRef, VIDEO_MODE_FAKE, true );
                    SiiOsdSetRowEnable( winRef,AV_MUTE, false );
                	break;
                case AV_MUTE :
                    SiiOsdSetRowEnable( winRef,AV_MUTE, true );
                    SiiOsdSetRowEnable( winRef, VIDEO_MODE_PATTERNS, false );
                   	SiiOsdSetRowEnable( winRef, VIDEO_MODE_FAKE, false );
                	break;
          }

            SkAppOsdMoveDotMarker( winRef, -1, osdApp.videoTypeSelect );
            UpdateMenuHelp( videoHelpText, KH_SUB1_KEYS, 0 );

            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            break;
        case UFA_UPDATE:

            // Called when display resolution has changed while menu is displayed.
            // Update internal video mode choices in right side window.
            if ( pWinInfo->selectedRow == VIDEO_MODE_INTERNAL)
            {
                SiiOsdSendMessage( osdApp.winRefRight, UFA_UPDATE, 0 );
            }

            // If formerly selected row is not valid, move to next valid selection.
            if ( !SiiOsdIsRowEnabled( winRef, osdApp.videoTypeSelect ))
            {
                if(osdApp.videoTypeSelect != AV_MUTE)
                    osdApp.videoTypeSelect = SiiOsdMoveCursor( winRef, true, OPT_SHOW );
            }
            break;
        case UFA_ROW_ENTER:
        case UFA_FOCUS:
            UpdateMenuHelp(
                videoHelpText,
                ((pWinInfo->selectedRow == VIDEO_MODE_INTERNAL) || (pWinInfo->selectedRow == VIDEO_MODE_PATTERNS)) ? KH_SUB_KEYS : KH_SUB1_KEYS,
                pWinInfo->selectedRow );

            if ( action == UFA_FOCUS)
            {
                break;  // Only need to update the help text when receiving focus.
            }
            // For this custom composite menu, we handle row entry differently from normal.
            // Instead of closing an existing child window for this menu, replace it with the
            // child window for the passed row.  This will avoid background flashes during the removal of the
            // existing window and the creation of the new window. Returning false prevents the handler from
            // closing the existing window.
            SiiOsdReplaceWindow( pWinInfo->childWinRef, videoSetupSubMenuIds[ pWinInfo->selectedRow] );
            SiiOsdShowWindow( pWinInfo->childWinRef, OPT_SHOW );   // ReplaceWindow does not update display
            result = false;
            break;
        case UFA_BACK:
        case UFA_CANCEL:
            // SIIKEY_CURSOR_LEFT and SIIKEY_CANCEL have the same effect for this window, which is to return to the
            // main window.
            SkAppOsdRestoreMainMenu( winRef, pWinInfo->childWinRef );
            break;

        case UFA_SELECT:
            // If entries 0 - 2, we have a video type select, so place the marker appropriately
            // and switch to the selected source
            if ( (pWinInfo->selectedRow <= VIDEO_MODE_INTERNAL )||(pWinInfo->selectedRow == AV_MUTE))
            {
                // Update and display 'selected' marker
                SkAppOsdMoveDotMarker( winRef, osdApp.videoTypeSelect, pWinInfo->selectedRow );
                osdApp.videoTypeSelect = pWinInfo->selectedRow;
                app.isExtTpg=false;
                SkAppTpgEnable( false );
                app.tpgVideoPattern = SI_ETPG_PATTERN_RED;
                // Enable/disable video pattern select command based on video mode enable
                SiiOsdSetRowEnable( winRef, VIDEO_MODE_PATTERNS, (pWinInfo->selectedRow == VIDEO_MODE_INTERNAL) ? true : false );
                SiiOsdSetRowEnable( winRef, VIDEO_MODE_FAKE, (pWinInfo->selectedRow == VIDEO_MODE_INTERNAL) ? true : false );
                SiiOsdSetRowEnable( winRef, AV_MUTE, (pWinInfo->selectedRow == VIDEO_MODE_INTERNAL) ? false : true );
                // Perform requested menu command
                switch ( pWinInfo->selectedRow )
                {
                    case VIDEO_MODE_EXTERNAL:
                        DEBUG_PRINT(MSG_ALWAYS, "\n\nEntered VIDEO_MODE_EXTERNAL case \n");
                        if(app.isTPGEnabled)
                        {
                            DemoTpgModeEnable(false);
                            app.isTPGEnabled = false;
                            if(app.isExtTpg == false)
                            {
                                app.isExtTpg = true;
                            }
                        }
                        break;
                    case VIDEO_MODE_INTERNAL:
                        DEBUG_PRINT(MSG_ALWAYS, "\n\nEntered VIDEO_MODE_INTERNAL case \n");
                        // Set the TPG format previously selected.
                        switch ( osdApp.ivFormatSelect )
                        {
                            case IV_FORMAT_ROW_480P:
                                app.tpgVideoFormat = SI_TPG_FMT_VID_480_60;
                                app.tpgClockSrc = SI_TPG_CLK_XCLK;
                                break;
                            case IV_FORMAT_ROW_576P:
                                app.tpgVideoFormat = SI_TPG_FMT_VID_576_50;
                                app.tpgClockSrc = SI_TPG_CLK_XCLK;
                                break;
                            case IV_FORMAT_ROW_720P:
                                app.tpgVideoFormat = SI_TPG_FMT_VID_720_60;
                                // Use APLL1 (SWWA:22565)
                                app.tpgClockSrc = SI_TPG_CLK_P0;
                                break;
                        }
                        app.isExtTpg=true;
                        app.isTPGEnabled = true;
                        DemoTpgModeEnable(true);
                        app.tpgVideoPattern = osdApp.ivPatternSelect;
                        SiiDrvExtTpgPatternSet( osdApp.ivPatternSelect );
                        break;
                    case AV_MUTE :
                        DEBUG_PRINT(MSG_ALWAYS, "\n\nEntered AV_MUTE case \n");
                        DemoTpgModeEnable(true);
                        app.isExtTpg=true;
                        app.isTPGEnabled = true;
                        app.tpgVideoFormat = SI_TPG_FMT_VID_480_60;
                        app.tpgClockSrc = SI_TPG_CLK_XCLK;
                        app.tpgVideoPattern = SI_ETPG_PATTERN_BLUE;
                        SiiDrvExtTpgPatternSet( app.tpgVideoPattern );
                        break;
                }
            }
            if ( pWinInfo->selectedRow == VIDEO_MODE_FAKE )
            {

                SkAppOsdMoveDotMarker( winRef, osdApp.videoTypeSelect, pWinInfo->selectedRow );
                osdApp.videoTypeSelect = pWinInfo->selectedRow;
                app.isExtTpg=false;
                SkAppTpgEnable( true );
                // Enable/disable fake video  select command based on video mode enable
               SiiOsdSetRowEnable( winRef, VIDEO_MODE_FAKE, true);         	                // Perform requested menu command
               SiiOsdSetRowEnable( winRef, VIDEO_MODE_PATTERNS, false );

              // SiiDrvVideoMute(false);
            }
//            else
//            {
//            	SkAppOsdMoveDotMarker( winRef, osdApp.videoTypeSelect, pWinInfo->selectedRow );
//            	osdApp.videoTypeSelect = pWinInfo->selectedRow;
//            }
            result = false;     // Don't allow the SIIKEY_SELECT key to kill this window.
            break;
        default:
            break;
    }

    return( result );
}

