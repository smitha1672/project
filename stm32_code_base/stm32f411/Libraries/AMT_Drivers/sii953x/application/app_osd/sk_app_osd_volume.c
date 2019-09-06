//***************************************************************************
//!file     sk_app_osd_volume.c
//!brief    OSD Volume Control implementation
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "sk_app_osd.h"
#if (INC_CEC_SAC == ENABLE)
#include "si_sac_msg.h"
#else
#define MAX_VOLUME_LEVEL            24  //!< Maximum volume value. Minimum is 0
#define VOLUME_STEP_SIZE             1  //!< Volume increment/decrement step
#endif

//-------------------------------------------------------------------------------------------------
// Enumeration and defines
//-------------------------------------------------------------------------------------------------
SiiVolInfoStruct_t volInfo;

//-------------------------------------------------------------------------------------------------
//! @brief      Volume control user window function.
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  action
//! @param[in]  pArgs
//! @return
//-------------------------------------------------------------------------------------------------
bool_t UserFuncVolumeControl ( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs )
{
    bool_t          result = true;
    SiiFuncArgs_t   *pWinInfo = (SiiFuncArgs_t *)pArgs;

    switch ( action )
    {
        case UFA_REPLACEWINDOW:
        case UFA_CREATE:
            SiiOsdWriteIcon( winRef, 1, 1, 2, 2, SPEAKER_ICON, 0 );
            SiiOsdWriteText( winRef, 2, 4, "Volume", 0 );
            SiiOsdSliderUpdate( winRef, volInfo.speakerVolume, 0 );
            result = true;      // Replace or Create was successful
            break;
        case UFA_CLOSE:
            if ( app.isOsdMenuEnabled )
            {
                // If the OSD menu is enabled, it is likely we are being displayed as part of a demo.
                SkAppOsdRestoreOsdDemoMenu( winRef );
                result = false;     // Don't close the window
            }
            volInfo.winRef = -1;
            break;
        case UFA_ROW_ENTER:
        case UFA_SELECT:
        case UFA_CANCEL:
        default:
            break;
        case UFA_UPDATE:        // Slider/progress only.
            volInfo.speakerVolume = pWinInfo->param4;     // Save updated value.
            break;
    }

    return( result );
}

//-------------------------------------------------------------------------------------------------
//! @brief  Volume Control.  This is the main application volume control.  It uses the same
//!         window that is used in demonstrating a slider window.
//-------------------------------------------------------------------------------------------------
void SkAppOsdVolumeControl ( int key )
{
    if ( !app.isOsdMenuEnabled )
    {
        if ( volInfo.winRef == -1 )
        {
            volInfo.winRef = SiiOsdCreateWindow(
                LRID3_VOL_CTRL,                 // Replace SPI flash resource RID24_WINDOW13_SI_UF_VOLUME_CTRL,
                OSD_DEFAULT_FONT_WIDTH * 3,
                OSD_DEFAULT_FONT_HEIGHT * 2,
                0, 0, NULL, 0 );
            if ( volInfo.winRef >= 0 )
            {
                // Set current volume information
                SiiOsdSetWindowPrivateData(
                    volInfo.winRef,
                    0, MAX_VOLUME_LEVEL, VOLUME_STEP_SIZE, volInfo.speakerVolume,
                    false );
                SiiOsdSetFocus( volInfo.winRef, OPT_TOP ); // Window has to have focus to let window manager handle it for key input.
                SiiOsdShowWindow( volInfo.winRef, OPT_SHOW );
                SiiOsdEnableOsdDisplay( true );
            }
        }
        else if ( key != 0 )
        {
            // Let Window Manager handle volume changes.
            SiiOsdWindowManager( (key == RC5_VOL_UP) ? SIIKEY_CURSOR_RIGHT : SIIKEY_CURSOR_LEFT, 0 );
        }
        else
        {
            // Update the displayed volume if needed
            SiiOsdSliderUpdate( volInfo.winRef, volInfo.speakerVolume, OPT_SHOW );
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief  Volume Control update from CEC System Audio Control.
//-------------------------------------------------------------------------------------------------
void SkAppOsdVolumeUpdate ( int newVolume )
{
    volInfo.speakerVolume = newVolume;
    SkAppOsdVolumeControl( 0 );         // Show volume control
}

