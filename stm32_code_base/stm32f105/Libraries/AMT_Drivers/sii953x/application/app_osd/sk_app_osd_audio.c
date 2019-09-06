//***************************************************************************
//!file     sk_app_osd_audio.c
//!brief    OSD Audio Demo window functions
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
#include "si_osd_component.h"
#include "sk_app_osd.h"
#include "si_drv_audio.h"

#define SII_EXT_AUD_SPDIF            1
#define SII_EXT_AUD_I2S              2
#define SII_INT_AUDIO              	 3

char *pAudioSetup[] =
{
    " Audio Insertion ",
    "                 ",
    " Audio Extration ",
    "                 "
};

static int  currentEditRow = 0;
static int  controlValues [4] =
{
    0,      // Zone 1 insertion source
    0,      // Zone 2 insertion source
    0,      // Main Port extraction on/off
    0       // Sub Port extraction on/off
};
#if 0
static SiiSwitchAudioSource_t zone1sources [] =
{
    SiiSwitchAudioSourceDefault,
    SiiSwitchAudioSourceExternal,
    SiiSwitchAudioSourceSubPipe
};
#endif
static SiiSwitchAudioSource_t zone1AudSources [] =
{
    SiiSwitchAudioSourceDefault,
    SiiSwitchAudioSourceExternal,
    SiiSwitchAudioSourceExternal,
    SiiSwitchAudioSourceSubPipe
};
static SiiSwitchAudioSource_t zone2sources [] =
{
    SiiSwitchAudioSourceDefault,
    SiiSwitchAudioSourceExternal,
    SiiSwitchAudioSourceMainPipe
};

static char *pAudioInsertModeStrings [] =
{
    "Off      ",
    "SPDIF_EXT",
    "I2S_EXT  ",
   // "Internal ",
};

static char *pAudioLocalModeStrings [] =
{
    "Off",
    "On ",
};

char *audioSetupHelpText [] =
{
    "Select Audio Insertion Mode   ",
    "Select Audio Insertion Mode",
    "Enable/disable Audio extraction",
    "Enable/disable Audio extraction",
};

//-------------------------------------------------------------------------------------------------
//! @brief      The actual zone hardware control is here.  Yes, all that code just for
//!             the interface, and just a little code doing the real work.
//! @param[in]  row     Menu row
//-------------------------------------------------------------------------------------------------
static void UpdateAudioHardware ( int row )
{
    int pipe;

    switch ( row )
    {
	    case 0:
	    	if(controlValues[row] == SII_INT_AUDIO)
	    	{
	    		if(app.ipvPipEnabled)
	    		{
	                app.newAudioSource[0] = zone1AudSources[ controlValues[row]];
	    		}
	    		else
	    		{
	    			DEBUG_PRINT(MSG_ALWAYS,"\n\r UpdateAudioHardware SiiSwitchAudioSourceSubPipe");
	    		}
	    	}
	    	else
	    	{
                app.newAudioSource[0] = zone1AudSources[ controlValues[row]];
	    	}

            break;
        case 1:
            app.newAudioSource[1] = zone2sources[ controlValues[row]];
            break;
        case 2:
        case 3:
            pipe = row - 2;
            app.isAudioExtractionEnabled[ pipe] = controlValues[row];
            SiiDrvRxAudioInstanceSet( pipe );
            SiiDrvRxAudioMute( !controlValues[row] );
            break;
    }
    if(app.newAudioSource[0]==SiiSwitchAudioSourceExternal)
    {
    	 app.currentAudioType[0] = SI_TX_AUDIO_SRC_SPDIF;
    	if(controlValues[ row ] == SII_EXT_AUD_SPDIF)
    	{
    		app.currentAudioType[0] = SI_TX_AUDIO_SRC_SPDIF;
    		app.newaudExtSource=SI_TX_AUDIO_SRC_SPDIF;
    	}
    	else if(controlValues[ row ] == SII_EXT_AUD_I2S)
    	{
    		app.currentAudioType[0] = SI_TX_AUDIO_SRC_I2S_L0;
    		app.newaudExtSource=SI_TX_AUDIO_SRC_I2S_L0;
    	}
    	else
    	{
    		app.currentAudioType[0] = SI_TX_AUDIO_SRC_SPDIF;

    	}
    	// app.currentAudioType[0] = SI_TX_AUDIO_SRC_SPDIF;
    }

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
static void UpdateAudioControl( int winRef, int row )
{
    int     endCol = 0;
    char    tempStr[20];
    bool_t  isEditMode = SiiOsdIsEditMode( winRef ) != 0;

    memset( tempStr, 0, sizeof( tempStr) );
    switch ( row )
    {
        case 0:
        case 1:
            strcpy( &tempStr[1], pAudioInsertModeStrings[ controlValues[row]] );
            endCol = 10;
            break;
        case 2:
        case 3:
            strcpy( &tempStr[1], pAudioLocalModeStrings[ controlValues[row]] );
            endCol = 4;
            break;
    }
    tempStr[0] = (isEditMode) ? SII_OSD_CHAR_LEFT : ' ';
    tempStr[endCol] = (isEditMode) ? SII_OSD_CHAR_RIGHT : ' ';
    SiiOsdWriteText( winRef, row, 1, tempStr, 0 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Process a SIIKEY_CURSOR_RIGHT or SIIKEY_CURSOR_LEFT key received as an audio insert
//!             mode change, but only if in audio insert mode change mode.
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
static void EditAudioControl ( int winRef, SiiOsdUserFunctionAction_t action, int row )
{
    int control;
    int increment = (action == UFA_FOCUS ) ? 1 : -1;    // Right cursor or left cursor

    control = controlValues[ row ] + increment;
    switch ( row )
    {
        case 0:
        case 1:
            // Change the selected zone insertion mode
            control = (control < 0) ? 2 : (control > 2) ? 0 : control;
            break;
        case 2:
        case 3:
            // Change the selected port audio extraction mode
            control = (control < 0) ? 1 : (control > 1) ? 0 : control;
            break;
    }

    controlValues[ row ] = control;

    UpdateAudioControl( winRef, row );
    SiiOsdShowWindow( winRef, OPT_SHOW );           // Force updates to be displayed
}

//-------------------------------------------------------------------------------------------------
//! @brief      Audio Setup Zone audio source selection (Insertion)
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 true if success, false if failure.
//-------------------------------------------------------------------------------------------------
bool_t UserFuncAudioControls( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    //int             i;
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            SiiOsdSetEditMode( winRef, false );

            // Initialize control values
            controlValues[0] = app.newAudioSource[0];
            controlValues[1] = app.newAudioSource[1];
            controlValues[2] = app.isAudioExtractionEnabled[0];
            controlValues[3] = app.isAudioExtractionEnabled[1];
            SiiOsdSetRowEnable( winRef,1,false );
            SiiOsdSetRowEnable( winRef,3,false );
            SiiOsdSetRowEnable( winRef,4,false );
            SiiOsdSetRowEnable( winRef,5,false );
            SiiOsdSetRowEnable( winRef,6,false );
            SiiOsdSetRowEnable( winRef,7,false );


          //  for ( i = 0; i < 4; i++ )
           // {
                UpdateAudioControl( winRef, 0 );
                UpdateAudioControl( winRef, 2 );
           // }
            UpdateMenuHelp( audioSetupHelpText, KH_MOD1_KEYS, 0 );

            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            break;
        case UFA_ROW_ENTER:

            // If we were editing, tell the audio hardware what we've done.
          /*  if ( SiiOsdIsEditMode( winRef ))
            {
            	// No longer editing.  Remove edit cursor from previous row.
                SiiOsdSetEditMode( winRef, false );
                UpdateAudioControl( winRef, currentEditRow );
                SiiOsdShowWindow( winRef, OPT_SHOW );       // Force updates to be displayed
                UpdateAudioHardware( currentEditRow );      // Actual hardware control
            }

            currentEditRow = pWinInfo->selectedRow;
            SiiOsdSetCursor( pWinInfo->parentWinRef, currentEditRow, OPT_HIDE );    // Parent cursor should track our cursor.
            UpdateMenuHelp( audioSetupHelpText, KH_MOD1_KEYS, currentEditRow ); */

            SiiOsdSetCursor( winRef, currentEditRow,OPT_SHOW );

            result = false;
            break;
        case UFA_BACK:
        case UFA_FOCUS:

            if ( SiiOsdIsEditMode( winRef ))
            {
                EditAudioControl( winRef, action, currentEditRow );
                result = false;     // Stay in sub-menu until the SIIKEY_CURSOR_LEFT is pressed.
            }
            else if ( action == UFA_FOCUS )
            {
                // Start out on the same row as our parent.
                currentEditRow = SiiOsdGetCursor( pWinInfo->parentWinRef );
                SiiOsdSetCursor( winRef, currentEditRow, OPT_SHOW );
                UpdateMenuHelp( audioSetupHelpText, KH_EDIT_KEYS, currentEditRow );
            }
            break;
        case UFA_SELECT:
            // If we were editing, tell the audio hardware what we've done.
            if ( SiiOsdIsEditMode( winRef ))
            {
                SiiOsdSetEditMode( winRef, false );     // No longer editing.
                UpdateMenuHelp( audioSetupHelpText, KH_SUB_KEYS, currentEditRow );
                UpdateAudioHardware( currentEditRow );  // Actual hardware control
            }
            else
            {
                SiiOsdSetEditMode( winRef, true );      // Turn on the edit mode
                UpdateMenuHelp( audioSetupHelpText, KH_EDIT_KEYS, currentEditRow );
            }

            UpdateAudioControl( winRef, currentEditRow );
            SiiOsdShowWindow( winRef, OPT_SHOW );       // Force updates to be displayed
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
//! @brief      Audio Setup menu user function
//! @param[in]  winRef      Return value from the SiiOsdCreateMenu function.
//! @param[in]  extra
//! @param[in]  isCancel
//! @param[in]  pArgs
//! @return                 Varies by action. See documentation
//-------------------------------------------------------------------------------------------------
bool_t UserFuncAudioSetup( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
            // Use saved former cursor row.
            SiiOsdSetCursor( winRef, osdApp.cursorAudioSetup, OPT_SHOW );
            // Fall through...
        case UFA_CREATE:
            SiiOsdWriteText( osdApp.winRefTop, 2, 4, "AUDIO SETUP", OPT_CLEAR_TO_END );
            SiiOsdWriteIcon( osdApp.winRefTop, 1, 1, 2, 2, SPEAKER_ICON, 0 );

            // Window text.
            SiiOsdWriteText( winRef, 0, 1, pAudioSetup[0], OPT_CENTER_TEXT );

            SiiOsdWriteText( winRef, 1, 1, pAudioSetup[1], OPT_CENTER_TEXT );
            SiiOsdSetRowEnable( winRef,1,false );

            SiiOsdWriteText( winRef, 2, 1, pAudioSetup[2], OPT_CENTER_TEXT );

            SiiOsdWriteText( winRef, 3, 1, pAudioSetup[3], OPT_CENTER_TEXT );
            SiiOsdSetRowEnable( winRef,3,false );

            SiiOsdSetRowEnable( winRef,4,false );
            SiiOsdSetRowEnable( winRef,5,false );
            SiiOsdSetRowEnable( winRef,6,false );
            SiiOsdSetRowEnable( winRef,7,false );
            UpdateMenuHelp( audioSetupHelpText, KH_MOD_KEYS, osdApp.cursorAudioSetup );
            SiiOsdReplaceWindow( pWinInfo->childWinRef, RID33_WINDOW22_SI_UF_AUDIOSETUP_CONTROLS );
            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            break;
        case UFA_ROW_ENTER:
            UpdateMenuHelp( audioSetupHelpText, KH_MOD_KEYS, pWinInfo->selectedRow );
            osdApp.cursorAudioSetup = pWinInfo->selectedRow;

            // For this custom composite menu, we handle row entry differently from normal.
            // Instead of closing an existing child window for this menu, use the same one for
            // every row. Returning false prevents the handler from closing the existing window.
            result = false;
            break;
        case UFA_FOCUS:
            UpdateMenuHelp( audioSetupHelpText, KH_MOD_KEYS, SiiOsdGetCursor( winRef ));// winInfo->selectedRow value may not be accurate in our case.
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

