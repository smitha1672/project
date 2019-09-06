//***************************************************************************
//!file     sk_app_osd_speaker.c
//!brief    OSD example Speaker Setup Menu
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "sk_application.h"
#include "sk_app_osd.h"

//-------------------------------------------------------------------------------------------------
// Local and global data and variables
//-------------------------------------------------------------------------------------------------

static int      crossoverSelect = 0;            // Speaker Setup
static int      currentEditRow = 0;             // Current Speaker Config left menu row (a cheat needed to do the composite window)
static bool_t   ssConfigSubActive = false;      // true if Speaker Config left menu is the focus.

    // Speaker set window rows.
static int scfgCursorRows1[ 5] = { 1,   0,  3, 4, 6 };
static int scfgCursorRows2[ 5] = { 2,  -1, -1, 5, 7 };  // Set to -1 if the speaker set uses only one row.

// Left windows chosen by the main left window. (Press Enter in the main window.)
static const int leftSpeakerWindowIds [] =
{
    RID5_WINDOW6_SI_UF_BLANKSUBMENU,
    RID8_WINDOW9_SI_UF_SPEAKERCONFIG,
    RID5_WINDOW6_SI_UF_BLANKSUBMENU,
    RID5_WINDOW6_SI_UF_BLANKSUBMENU,
    RID5_WINDOW6_SI_UF_BLANKSUBMENU,
    RID6_WINDOW7_SI_UF_CROSSOVER,
};

static int speakerSize [] =
{
    0,      // Front speaker, default large
    0,      // Center speaker, default Large
    3,      // Subwoofer, default Yes
    0,      // Surround, default Large
    0,      // Rear speaker, default Large
};

char *speakerConfigHelpText [] =
{
    "Front speakers",
    "Center speaker",
    "Sub woofer",
    "Surround speakers",
    "Rear speakers",
};
static char *pSpeakerSizeStrings [] = {
    "Large",
    "Small",
    "No   ",
    "Yes  ",
};

char *speakerSetupHelpText [] =
{
    "Assign amplifiers",
    "Configure speakers",
    "Set bass level",
    "Set distance",
    "Set channel level",
    "Select crossover frequency from which the sub-woofer handles low range signals",
};

//-------------------------------------------------------------------------------------------------
//! @brief      Display the correct speaker icon and color for the specified row.
//! @note       This function does NOT update the display.  The SiiOsdShowWindow function must be
//!             used to display changes made by this function. This requirement is so that the
//!             function can be called multiple times before updating the display, resulting in a
//!             smoother and faster update.
//! @param[in]  winRef      Speaker config window reference.
//! @param[in]  speakerSet  Speaker set index
//-------------------------------------------------------------------------------------------------
static void DisplaySpeakerRow( int winRef, int row, bool_t isSelected )
{
    int     options;

    switch ( row )
    {
        case 0:             // Front speakers
            SiiOsdWriteIcon( winRef, 1, 3, 2, 2, ( speakerSize[row] == 0 ) ? SPK_FRONT_LG : SPK_FRONT_SM, 0 );
            SiiOsdWriteIcon( winRef, 1, 9, 2, 2, ( speakerSize[row] == 0 ) ? SPK_FRONT_LG : SPK_FRONT_SM, 0 );
            break;
        case 1:             // Center speaker
            options = ( speakerSize[row] == 2 ) ? OPT_REMOVE_ICON : 0;
            SiiOsdWriteIcon( winRef, 0, 6, 1, 2, ( speakerSize[row] == 0 ) ? SPK_CENTER_LG : SPK_CENTER_SM, options );
            break;
        case 2:             // Subwoofer
            options = ( speakerSize[row] == 2 ) ? OPT_REMOVE_ICON : 0;
            SiiOsdWriteIcon( winRef, 3, 5, 1, 1, SPK_WOOFER, options );
            break;
        case 3:             // Surround speakers
            options = ( speakerSize[row] == 2 ) ? OPT_REMOVE_ICON : 0;
            SiiOsdWriteIcon( winRef, 4, 2, 2, 1, ( speakerSize[row] == 0 ) ? SPK_LEFT_LG : SPK_LEFT_SM, options );
            SiiOsdWriteIcon( winRef, 4, 11, 2, 1, ( speakerSize[row] == 0 ) ? SPK_RIGHT_LG : SPK_RIGHT_SM, options );
            break;
        case 4:             // Rear speakers
            options = ( speakerSize[row] == 2 ) ? OPT_REMOVE_ICON : 0;
            SiiOsdWriteIcon( winRef, 6, 5, 2, 1, ( speakerSize[row] == 0 ) ? SPK_REAR_LG : SPK_REAR_SM, options );
            SiiOsdWriteIcon( winRef, 6, 8, 2, 1, ( speakerSize[row] == 0 ) ? SPK_REAR_LG : SPK_REAR_SM, options );
            break;
    }

    // Enable the speaker cursor coloring by setting the window 'disabled' rows to be the row(s) that
    // the speaker set is displayed on.  Disabled row colors are what we are using for speaker coloring.
    SiiOsdSetRowEnable( winRef, scfgCursorRows1[ row], !isSelected );
    if ( scfgCursorRows2[ row] != -1 )
    {
        SiiOsdSetRowEnable( winRef, scfgCursorRows2[ row], !isSelected );
    }

    // Make sure the hardware window selected row is not the same as our cursor row, since
    // we use disabled row color values as our row coloring and selected row color has precedence
    // over the disabled row color.
    SiiOsdSetCursor( winRef, (scfgCursorRows1[ row] == 0) ? 1 : 0, OPT_SHOW );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Display the speaker configuration text (x.x).
//! @note       This function does NOT update the display.  The SiiOsdShowWindow function must be
//!             used to display changes made by this function. This requirement is so that the
//!             function can be called multiple times before updating the display, resulting in a
//!             smoother and faster update.
//! @param[in]  winRef      Speaker config window reference.
//! @param[in]  speakerSet  Speaker set index
//-------------------------------------------------------------------------------------------------
static void UpdateSpeakerConfigText( int winRef )
{
    int     speakerCount;
    char    tempStr[20];

    // Display the current speaker configuration at the bottom of the menu

    speakerCount = 2;   // Assume front speakers.
    speakerCount += (speakerSize[1] != 2) ? 1 : 0;     // Only one center speaker
    speakerCount += (speakerSize[3] != 2) ? (speakerSize[4] != 2) ? 2 : 10 : 0;   // Mark if surround but not rear
    speakerCount += (speakerSize[4] != 2) ? 2 : 0;     // Rear

    strcpy( tempStr, " Speaker Cfg: 7.1 ");
    if ( speakerCount < 8)
    {
        tempStr[14] = speakerCount + '0';
    }
    else
    {
        // Surround speakers, but not rear speakers
        tempStr[14] = (speakerCount - 8) + '0';
        tempStr[17] = '*';
    }
    tempStr[16] = (speakerSize[2] != 2) ? '1' : '0';
    tempStr[18] = 0;
    SiiOsdWriteText( winRef, 7, 0, tempStr, 0 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      The edit cursor is displayed if isSelected is true, otherwise it is removed.
//! @note       This function does NOT update the display.  The SiiOsdShowWindow function must be
//!             used to display changes made by this function. This requirement is so that the
//!             function can be called multiple times before updating the display, resulting in a
//!             smoother and faster update.
//! @param[in]  winRef      Speaker config window reference.
//! @param[in]  speakerSet  Speaker set index
//-------------------------------------------------------------------------------------------------
static void UpdateSpeakerConfigSizeText( int winRef, int row )
{
    char    tempStr[20];
    bool_t  isEditMode = SiiOsdIsEditMode( winRef ) != 0;

    strcpy( &tempStr[1], pSpeakerSizeStrings[ speakerSize[row]] );
    tempStr[0] = (isEditMode) ? SII_OSD_CHAR_LEFT : ' ';
    tempStr[6] = (isEditMode) ? SII_OSD_CHAR_RIGHT : ' ';
    tempStr[7] = 0;
    SiiOsdWriteText( winRef, row, 11, tempStr, 0 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Process a SIIKEY_CURSOR_RIGHT or SIIKEY_CURSOR_LEFT key received as a speaker size change,
//!             but only if in speaker change mode.
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
static void EditSpeakerConfigSize ( int winRef, int speakerIconWinRef, SiiOsdUserFunctionAction_t action, int row )
{
    int spkrSize;
    int increment = (action == UFA_FOCUS ) ? 1 : -1;    // Right cursor or left cursor

    // If in change mode (user pressed enter in this row), change the selected speaker set size
    if ( SiiOsdIsEditMode( winRef ) )
    {
        spkrSize = (speakerSize[ row ] + increment);
        switch ( row )
        {
            case 0: // Front        Large, Small
                spkrSize = (spkrSize > 1) ? 0 : (spkrSize < 0) ? 1 : spkrSize;
                break;
            case 1: // Center       Large, Small, No
            case 3: // Surround     Large, Small, No
            case 4: // Rear         Large, Small, No
                spkrSize = (spkrSize > 2) ? 0 : (spkrSize < 0) ? 2 : spkrSize;
                break;
            case 2: // Subwoofer    No, Yes
                spkrSize = (spkrSize > 3) ? 2 : (spkrSize < 2) ? 3 : spkrSize;
                break;
        }
        speakerSize[ row ] = spkrSize;

        UpdateSpeakerConfigText( winRef );                      // Update left side config text
        UpdateSpeakerConfigSizeText( winRef, row );             // Speaker size string and cursor
        DisplaySpeakerRow( speakerIconWinRef, row, true );      // Update right side speaker drawing
        SiiOsdShowWindow( winRef, OPT_SHOW );                   // Force updates to be displayed
        SiiOsdShowWindow( speakerIconWinRef, OPT_SHOW );        // Force updates to be displayed
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Speaker configuration sub menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncRightSpeakerConfig( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    int             i;
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            // Display speakers without selected speaker coloring
            for ( i = 0; i < 5; i++ )
            {
                DisplaySpeakerRow( winRef, i, false );  // Just display the speaker icons
            }
            result = true;      // Replace or Create was successful
            osdApp.speakerConfigActive = false;
            break;
        case UFA_CLOSE:
            break;
        case UFA_ROW_ENTER:
            result = false;     // A composite sub-menu has no children, so don't bother checking.
            break;
        case UFA_SELECT:
            result = false;     // Stay in sub-menu until the SIIKEY_CURSOR_LEFT is pressed.
            break;
        case UFA_FOCUS:         // Window has received focus via the SIIKEY_CURSOR_RIGHT key.

            // For this menu, we really don't want focus, we just want to replace the left
            // menu with a special control menu for this window.
            if ( !osdApp.speakerConfigActive )
            {
                SiiOsdReplaceWindow( pWinInfo->parentWinRef, RID9_WINDOW10_SI_UF_SPEAKERCONFIG_SUB ); // Replace left menu with our control menu
                SiiOsdShowWindow( pWinInfo->parentWinRef, OPT_SHOW );   // ReplaceWindow does not update display
                osdApp.speakerConfigActive = true;
            }
            SiiOsdSetFocus( pWinInfo->parentWinRef, 0 );    // Return focus to the left control menu
            result = false;     // Don't do any menu closing
            break;
        case UFA_BACK:
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
//! @brief      Speaker Config menu. Instead of giving focus to the right-side Speaker Config
//!             menu, replace the Speaker Setup menu on the left with this menu.  The Speaker Setup
//!             menu will return when the left arrow (BACK) key is pressed.
//!             in focus user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 Varies by action. See documentation
//-------------------------------------------------------------------------------------------------
bool_t UserFuncLeftSpeakerConfig( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    int             i;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
            SiiOsdSetCursor( winRef, 0, OPT_SHOW );    // Set cursor to row 0, since the current value is from the former window
        case UFA_CREATE:
            // Window text.
            SiiOsdSetEditMode( winRef, false );
            for ( i = 0; i < 5; i++ )
            {
                UpdateSpeakerConfigSizeText( winRef, i);
            }

            currentEditRow = 0;
            UpdateSpeakerConfigText( winRef );
            DisplaySpeakerRow( pWinInfo->childWinRef, currentEditRow, true );  // Add selected speaker coloring
            SiiOsdShowWindow( winRef, OPT_SHOW );
            UpdateMenuHelp( speakerConfigHelpText, KH_MOD1_KEYS, 0 );
            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            break;
        case UFA_ROW_ENTER:
            // For this custom composite menu, we handle row entry differently from normal.
            // Instead of closing an existing child window for this menu, we want to exit edit mode (if on),
            // and remove the 'speaker cursor' in the right side menu.
            if ( SiiOsdIsEditMode( winRef ))
            {
                // No longer editing.  Remove edit cursor from previous row.
                SiiOsdSetEditMode( winRef, false );
                UpdateSpeakerConfigSizeText( winRef, currentEditRow );               // Remove edit cursor
                UpdateMenuHelp( speakerConfigHelpText, KH_MOD1_KEYS, pWinInfo->selectedRow );
            }
            DisplaySpeakerRow( pWinInfo->childWinRef, currentEditRow, false );   // Remove speaker color
            currentEditRow = pWinInfo->selectedRow;
            DisplaySpeakerRow( pWinInfo->childWinRef, currentEditRow, true );    // Add speaker color

            SiiOsdShowWindow( winRef, OPT_SHOW );                                // Force updates to be displayed
            SiiOsdShowWindow( pWinInfo->childWinRef, OPT_SHOW );                 // Force updates to be displayed
            result = false; // Don't close any windows upon return.
            break;
        case UFA_BACK:
        case UFA_FOCUS:

            if ( SiiOsdIsEditMode( winRef ))
            {
                EditSpeakerConfigSize( winRef, pWinInfo->childWinRef, action, currentEditRow );
                result = false;     // Stay in sub-menu until the SIIKEY_CURSOR_LEFT is pressed.
            }
            else if ( action == UFA_BACK )
            {
                // SIIKEY_CURSOR_LEFT and SIIKEY_CANCEL have the same effect for this window, which is to return to the
                // Speaker Setup Window.
                SiiOsdReplaceWindow( pWinInfo->childWinRef, RID5_WINDOW6_SI_UF_BLANKSUBMENU );  // Remove child window.
                SiiOsdReplaceWindow( winRef, RID7_WINDOW8_SI_UF_SPEAKERSETUP );                 // Replace left window with previous left window
                SiiOsdShowWindow( winRef, OPT_SHOW );                   // ReplaceWindow does not update display
                SiiOsdSetFocus( winRef, 0 );                            // Restore select cursor
                ssConfigSubActive       = false;
            }
            break;

        case UFA_SELECT:

            // If we were editing, get out of edit mode.
            if ( SiiOsdIsEditMode( winRef ))
            {
                SiiOsdSetEditMode( winRef, false );                     // No longer editing.
                UpdateMenuHelp( NULL, KH_MOD1_KEYS, -1 );
            }
            else
            {
                SiiOsdSetEditMode( winRef, true );                      // Turn on the edit mode
                UpdateMenuHelp( NULL, KH_EDIT_KEYS, -1 );
            }
            UpdateSpeakerConfigSizeText( winRef, currentEditRow );      // Add or remove edit cursor
            SiiOsdShowWindow( winRef, OPT_SHOW );                       // Force updates to be displayed
            result = false;     // Don't allow the SIIKEY_SELECT key to kill this window.
            break;
        case UFA_CANCEL:
        case UFA_UPDATE:        // Slider/progress only.
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Speaker Setup menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 Varies by action. See documentation
//-------------------------------------------------------------------------------------------------
bool_t UserFuncLeftSpeakerSetup( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    int             i;
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
            // Use saved former cursor row.
            SiiOsdSetCursor( winRef, osdApp.cursorLeftSpeakerSetup, OPT_SHOW );
            // Fall through...
        case UFA_CREATE:
            // Update bread crumb location
            SiiOsdWriteText( osdApp.winRefTop, 2, 4, "SPEAKER SETUP", OPT_CLEAR_TO_END );
            SiiOsdWriteIcon( osdApp.winRefTop, 1, 1, 2, 2, SPEAKER_ICON, 0 );

            SiiOsdWriteText( winRef, 2, 1, "               ", OPT_SHOW); /* Masking the "Bass Setting" */
            SiiOsdWriteText( winRef, 3, 1, "               ", OPT_SHOW); /* Masking the "Distance" */
            SiiOsdWriteText( winRef, 4, 1, "               ", OPT_SHOW); /* Masking the "Channel Level" */

            // Make sure we start out on an enabled row
            for( i = osdApp.cursorLeftSpeakerSetup; i < 8; i++ )
            {
                if ( SiiOsdIsRowEnabled( winRef, i ))
                {
                    osdApp.cursorLeftSpeakerSetup = i;
                    SiiOsdSetCursor( winRef, i, OPT_SHOW );
                    break;
                }
            }
            pWinInfo->selectedRow = osdApp.cursorLeftSpeakerSetup;  // Set selected row for UFA_ROW_ENTER fall through

            result = true;      // Replace or Create was successful
            // Fall through to display right side window if needed...
        case UFA_ROW_ENTER:
            UpdateMenuHelp( speakerSetupHelpText, KH_MOD_KEYS, pWinInfo->selectedRow );
            osdApp.cursorLeftSpeakerSetup = pWinInfo->selectedRow;

            // For this custom composite menu, we handle row entry differently from normal.
            // Instead of closing an existing child window for this menu, replace it with the
            // child window for the passed row.  This will avoid background flashes during the removal of the
            // existing window and the creation of the new window. Returning false prevents the handler from
            // closing the existing window.
            SiiOsdReplaceWindow( pWinInfo->childWinRef, leftSpeakerWindowIds[ pWinInfo->selectedRow] );
            SiiOsdShowWindow( pWinInfo->childWinRef, OPT_SHOW );   // ReplaceWindow does not update display
            if ( action == UFA_ROW_ENTER)   // UFA_REPLACEWINDOW and UFA_CREATE return true, UFA_ROW_ENTER does not
            {
                result = false;
            }
            break;
        case UFA_CLOSE:
            break;
        case UFA_BACK:
        case UFA_CANCEL:
            // SIIKEY_CURSOR_LEFT and SIIKEY_CANCEL have the same effect for this window, which is to return to the
            // main window.
            SkAppOsdRestoreMainMenu( winRef, pWinInfo->childWinRef );
            break;

        case UFA_SELECT:
            result = false;     // Don't allow the SIIKEY_SELECT key to kill this window.
            break;
        case UFA_UPDATE:        // Slider/progress only.
        default:
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Crossover sub menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncRightCrossover( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            SkAppOsdMoveDotMarker( winRef, -1, crossoverSelect );   // Display 'selected' marker
            UpdateMenuHelp( speakerSetupHelpText, KH_SUB_KEYS, 5 );
            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            break;
        case UFA_ROW_ENTER:
            result = false;     // A composite sub-menu has no children, so don't bother checking.
            break;
        case UFA_SELECT:
            SkAppOsdMoveDotMarker( winRef, crossoverSelect, pWinInfo->selectedRow );
            crossoverSelect = pWinInfo->selectedRow;
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
//! @brief      Initialize demo system variables.
//-------------------------------------------------------------------------------------------------
void SkAppOsdInitializeSpeakerSetupDemo ( void )
{
    crossoverSelect = 0;                // Speaker Setup
    currentEditRow = 0;         // Current Speaker Config left menu row (a cheat needed to do the composite window)
    ssConfigSubActive = false;          // true if Speaker Config left menu is the focus.
}
