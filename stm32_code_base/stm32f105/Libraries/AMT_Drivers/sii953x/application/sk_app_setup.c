//***************************************************************************
//!file     sk_app_setup.c
//!brief    Built-in Setup manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "device_config.h"
#include "si_device_config.h"

#if INC_SETUP
#include "si_eeprom.h"
#include "sk_application.h"
#if INC_CEC
#include "si_cec_component.h"
#endif
#include "si_edid_tx_component.h"

// Included here only because this is a 'special' app layer module
#include "si_drv_device.h"
#include "si_drv_internal.h"
#include "si_drv_tpi_ddc_access.h"
#include "si_drv_tpi_system.h"
#include "si_edid_tx_analyzer.h"
#include "si_edid_tx_parsing.h"

uint8_t  dispData[ EDID_TABLE_LEN];


#if 0
//------------------------------------------------------------------------------
// Function:    SetupLevelTwo
// Description: Handles commands on the third level of command options
// Returns:     TRUE if exit from setup, false if return to previous level
//------------------------------------------------------------------------------
static char *pLevelTwoMenu0 [] =
{
    "\nSetup Menu Three",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToSetupMenuMsg,
    NULL
};
static bool_t SetupLevelTwo( RC5Commands_t key )
{
    uint16_t    markerOptionFlagsMp, markerOptionFlagsRp;
    uint8_t     marker3dGpioEnableMp, marker3dGpioEnableRp;
    bool_t      changeConfigure, isRp;

    markerOptionFlagsMp     = app.markerOptionFlagsMp;
    markerOptionFlagsRp     = app.markerOptionFlagsRp;
    marker3dGpioEnableMp    = app.marker3dGpioEnableMp ? 1 : 0;
    marker3dGpioEnableRp    = app.marker3dGpioEnableRp ? 1 : 0;
    changeConfigure         = false;
    isRp                    = false;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pLevelTwoMenu0 );
            DEBUG_PRINT( MSG_ALWAYS, "1 - Toggle MP 3D Marker Support (%s)\n", (markerOptionFlagsMp & ENABLE_3D_MARKER_SUPPORT) ? "enabled" : "disabled ");
            DEBUG_PRINT( MSG_ALWAYS, "2 - Toggle MP 3D VS insertion (%s)\n", (markerOptionFlagsMp & INSERT_3D_VS) ? "enabled" : "disabled");
            DEBUG_PRINT( MSG_ALWAYS, "3 - Toggle MP 3D GPIO(s) (%s)\n", marker3dGpioEnableMp ? "enabled" : "disabled " );
            DEBUG_PRINT( MSG_ALWAYS, "4 - Toggle RP 3D Marker Support (%s)\n", (markerOptionFlagsRp & ENABLE_3D_MARKER_SUPPORT) ? "enabled" : "disabled ");
            DEBUG_PRINT( MSG_ALWAYS, "5 - Toggle RP 3D VS insertion (%s)\n", (markerOptionFlagsRp & INSERT_3D_VS) ? "enabled" : "disabled");
            DEBUG_PRINT( MSG_ALWAYS, "6 - Toggle RP 3D GPIO(s) (%s)\n", marker3dGpioEnableRp ? "enabled" : "disabled " );
            DEBUG_PRINT( MSG_ALWAYS, "7 - Set 3D Marker 720p delay (%d lines)\n", app.markerDelay720p );
            DEBUG_PRINT( MSG_ALWAYS, "8 - Set 3D Marker 1080p delay (%d lines)\n", app.markerDelay1080p );
            break;

        case RC5_0:
            app.setupMenuLevel = 0;
            PrintAlways( setupSelectMenu );
            break;

        case RC5_1:
            markerOptionFlagsMp ^= ENABLE_3D_MARKER_SUPPORT;
            DEBUG_PRINT(
                MSG_ALWAYS,
                "\nMP 3D Marker Support (%s)\n", (markerOptionFlagsMp & ENABLE_3D_MARKER_SUPPORT) ? "enabled" : "disabled" );
            changeConfigure = true;
            break;
        case RC5_2:
            markerOptionFlagsMp ^= INSERT_3D_VS;
            DEBUG_PRINT(
                MSG_ALWAYS,
                "\nMP 3D VS insertion (%s)\n", (markerOptionFlagsMp & INSERT_3D_VS) ? "enabled" : "disabled" );
            changeConfigure = true;
            break;
        case RC5_3:
            marker3dGpioEnableMp = marker3dGpioEnableMp  ? 0 : 1;
            DEBUG_PRINT( MSG_ALWAYS, "MP 3D GPIO(s) %s\n", marker3dGpioEnableMp ? "enabled" : "disabled " );
            break;
        case RC5_4:
            markerOptionFlagsRp ^= ENABLE_3D_MARKER_SUPPORT;
            DEBUG_PRINT(
                MSG_ALWAYS, "\nRP 3D Marker Support (%s)\n",
                (markerOptionFlagsRp & ENABLE_3D_MARKER_SUPPORT) ? "enabled" : "disabled" );
            changeConfigure = true;
            isRp = true;
            break;
        case RC5_5:
            markerOptionFlagsRp ^= INSERT_3D_VS;
            DEBUG_PRINT(
                MSG_ALWAYS,
                "\nRP 3D VS insertion (%s)\n", (markerOptionFlagsRp & INSERT_3D_VS) ? "enabled" : "disabled" );
            changeConfigure = true;
            isRp = true;
            break;
        case RC5_6:
            marker3dGpioEnableRp = marker3dGpioEnableRp ? 0 : 1;
            DEBUG_PRINT( MSG_ALWAYS, "RP 3D GPIO(s) %s\n", marker3dGpioEnableRp ? "enabled" : "disabled " );
            isRp = true;
            break;
        case RC5_7:
            {
                uint16_t delayValue;
                DEBUG_PRINT( MSG_ALWAYS, "\n Enter 4 decimal digits (0000-4095): " );
                delayValue = SkAppIrGetSingleDigit( 4 );
                delayValue = (delayValue * 10) + SkAppIrGetSingleDigit( 9 );
                delayValue = (delayValue * 10) + SkAppIrGetSingleDigit( 9 );
                delayValue = (delayValue * 10) + SkAppIrGetSingleDigit( 9 );
                if ( delayValue < 4096 )
                {
                    app.markerDelay720p = delayValue;
                    changeConfigure = true;
                }
                else
                {
                    DEBUG_PRINT( MSG_ALWAYS, "\nMust be 4 decimal digits in the range of 0000-4095\n" );
                }
            }
            break;
        case RC5_8:
            {
                uint16_t delayValue;
                DEBUG_PRINT( MSG_ALWAYS, "\n Enter 4 decimal digits (0000-4095): " );
                delayValue = SkAppIrGetSingleDigit( 4 );
                delayValue = (delayValue * 10) + SkAppIrGetSingleDigit( 9 );
                delayValue = (delayValue * 10) + SkAppIrGetSingleDigit( 9 );
                delayValue = (delayValue * 10) + SkAppIrGetSingleDigit( 9 );
                if ( delayValue < 4096 )
                {
                    app.markerDelay1080p = delayValue;
                    changeConfigure = true;
                }
                else
                {
                    DEBUG_PRINT( MSG_ALWAYS, "\nMust be 4 decimal digits in the range of 0000-4095\n" );
                }
            }
            break;
        default:
            break;
    }

    if ( changeConfigure )
    {
        SiiDrvDevice3dMarkerConfigure(
            isRp ? (markerOptionFlagsRp | SET_SUBPIPE_OPTIONS) : markerOptionFlagsMp,
            app.markerDelay720p, app.markerDelay1080p );
        app.markerOptionFlagsMp = markerOptionFlagsMp;
        app.markerOptionFlagsRp = markerOptionFlagsRp;
    }
    app.marker3dGpioEnableMp = (marker3dGpioEnableMp != 0);
    SiiDrvDevice3dMarkerGpioEnable( marker3dGpioEnableMp );
    app.marker3dGpioEnableRp = (marker3dGpioEnableRp != 0);
    SiiDrvDevice3dMarkerGpioEnable( marker3dGpioEnableRp | SET_SUBPIPE_OPTIONS );

    return( false );
}
#endif
//------------------------------------------------------------------------------
// Function:    SetupLevelOne
// Description: Handles commands on the second level of command options
// Returns:     TRUE if exit from setup, false if return to previous level
//------------------------------------------------------------------------------
static char *pPixelClockRangeMenu [] =
{
    "\n Select Pixel Clock Range:",
    "\t 0 - Auto",
    "\t 1 - 0-165 MHz",
    "\t 2 - 165-225 MHz",
    "\t 3 - 225-300 MHz",
    NULL
};

static char *pLevelOneMenu0 [] =
{
    "\nSetup Menu Two",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToSetupMenuMsg,
    "1 - Show video modes supported by DS",
    NULL
};

static char *pLevelOneMenu1 [] =
{
    "2 - Display EEPROM EDID #",
    "3 - Display Port EDID SRAM",
    "4 - Set TX TMDS operation frequency range",
    //"5 - Set SP audio extraction output type (I2S/SPDIF)",
    NULL
};

static bool_t SetupLevelOne ( RC5Commands_t key )
{
    uint8_t     i;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pLevelOneMenu0 );
           // PrintStringOnOff( "2 - Toggle CEC debug message print mode", app.cecPrint );
            PrintMenu( pLevelOneMenu1 );
            break;

        case RC5_0:
            app.setupMenuLevel = 0;
            PrintAlways( setupSelectMenu );
            break;

        case RC5_1:

            PrintAlways( "4x3:\n" );
            PrintAlways( tenDashes );
            for ( i = 0; i < NMB_OF_VIDEO_MODES; i++ )
            {
                if ( VideoModeTable[i].Vic4x3 )
                {
                    if ( SiiEdidTxOutVideoModeRankGet( i, SI_EDID_TX_ASPECT_RATIO_4x3 ) > 0 )
                    {
                        DEBUG_PRINT(
                            MSG_ALWAYS, "  %dx%d%c\n",
                            VideoModeTable[i].Active.H,
                            VideoModeTable[i].Active.V,
                            VideoModeTable[i].Interlaced ? 'i' : 'p' );
                    }
                }
            }
            PrintAlways( "16x9:\n" );
            PrintAlways( tenDashes );
            for ( i = 0; i < NMB_OF_VIDEO_MODES; i++ )
            {
                if ( VideoModeTable[i].Vic16x9 )
                {
                    if ( SiiEdidTxOutVideoModeRankGet( i, SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9 ) > 0 )
                    {
                        DEBUG_PRINT(
                            MSG_ALWAYS, "  %dx%d%c\n",
                            VideoModeTable[i].Active.H,
                            VideoModeTable[i].Active.V,
                            VideoModeTable[i].Interlaced ? 'i' : 'p' );
                    }
                }
            }
            break;

//        case RC5_2:
//
//            app.cecPrint = !app.cecPrint;
//            flags = SiiPlatformEepromReadByte( EE_FW_CONTROL );
//            flags &= ~EEFLAG_CEC_PRINT;
//            flags |= app.cecPrint ? EEFLAG_CEC_PRINT : 0;
//            SiiPlatformEepromWriteByte( EE_FW_CONTROL, flags );
//            break;

        case RC5_2:
            //DEBUG_PRINT( MSG_ALWAYS, "\n EEPROM EDID (0-%d): ", SII_INPUT_PORT_COUNT - 1 );
            i = 0; //SkAppIrGetSingleDigit( SII_INPUT_PORT_COUNT - 1 );
            if ( SkAppEdidRead( EDID0_EEPROM + i, dispData ))
            {
                SkAppDisplayData( MSG_ALWAYS, dispData, EDID_TABLE_LEN );
            }
            else
            {
                DEBUG_PRINT( MSG_ERR, "Error reading EEPROM\n" );
            }
            break;

        case RC5_3:
            {
                uint8_t portIndex;

#if ( configSII_DEV_9535 == 1 )
                PrintAlways( "\n Display SRAM for Port 0 (0-3): " );
#else
                PrintAlways( "\n Display SRAM for Port 0 (0-2): " );
#endif
                portIndex = SkAppIrGetSingleDigit( 3 );
                if ( SiiDrvNvramSramRead( SRAM_P0 + portIndex, dispData, 0, EDID_TABLE_LEN ))
                {
                    SkAppDisplayData( MSG_ALWAYS, dispData, EDID_TABLE_LEN );
                }
                else
                {
                    DEBUG_PRINT( MSG_ERR, "Error reading SRAM for port %d\n", portIndex );
                }
            }
            break;

        case RC5_4:
                {
                    int_t txSel = 0;
                    int_t rangeSel = 0;

                    //PrintAlways( "\n Select TX (0-Tx0): " );
                    //txSel = SkAppIrGetSingleDigit( 0 );
                    PrintMenu( pPixelClockRangeMenu );
                    rangeSel = SkAppIrGetSingleDigit( 3 );

                    app.txTmdsFreqRange[txSel] = rangeSel;
                    if (rangeSel != 0)
                    {
                        SkAppTxAdjustAnalogSettings(txSel, 0);
                    }
                    PrintAlways( "\nDone.\n" );
                }
            break;

//        case RC5_5:
//            {
//                PrintAlways( "\n 0 - I2S, 1 - SPDIF: " );
//                app.isSubPipeAudioExtSpdif = SkAppIrGetSingleDigit( 1 );
//            }

        default:
            break;
    }

    return( false );
}

//------------------------------------------------------------------------------
// Function:    SetupLevelZero
// Description: Command Mode processor
//------------------------------------------------------------------------------

static char *pLevel0Menu0 [] =
{
    "\nSetup Menu One",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToSetupMenuMsg,
    NULL
};
static char *pLevel0Menu1 [] =
{
    "2 - Display NVRAM EDID",
    "3 - Display NVRAM Boot Data",
    "4 - Reset NVRAM to default",
    "5 - Firmware version",
    "6 - Reset EEPROM to default",
    NULL
};

static void SetupLevelZero ( RC5Commands_t key )
{
    uint16_t    rcKey;
    bool_t      done = false;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pLevel0Menu0 );
            ///PrintStringOnOff( "1 - Toggle CEC enable", app.cecEnable );
            DEBUG_PRINT( MSG_ALWAYS, "1 - Set message level (0x%02X)\n", app.dbgMsgLevel );
            //DEBUG_PRINT( MSG_ALWAYS, "3 - Enable message coloring (0x%02X)\n", app.dbgMsgColoring );
            PrintMenu( pLevel0Menu1 );
            break;

        case RC5_0:
            app.setupMenuLevel = 0;
            PrintAlways( setupSelectMenu );
            break;
//        case RC5_1:
//            app.cecEnable = !app.cecEnable;
//            flags = SiiPlatformEepromReadByte( EE_FW_CONTROL );
//            flags &= ~EEFLAG_CEC_ENABLE;
//            flags |= app.cecEnable ? EEFLAG_CEC_ENABLE : 0;
//            SiiPlatformEepromWriteByte( EE_FW_CONTROL, flags );
//#if INC_CEC
//            SkAppCecConfigure( app.cecEnable );
//#endif
//            break;
        case RC5_1:
            PrintAlways( " (0-4): " );
            app.dbgMsgLevel = SkAppIrGetSingleDigit( 4 );
            if ( app.dbgMsgLevel == 4 )
            {
                app.dbgMsgLevel = MSG_PRINT_ALL;
            }
            SiiPlatformEepromWriteByte( EE_UART_MSG_LEVEL, app.dbgMsgLevel );
            SiiPlatformLoggerMessageLevelSet( app.dbgMsgLevel );
            break;
//        case RC5_3:
//            PrintAlways( " (0-1): " );
//            app.dbgMsgColoring = SkAppIrGetSingleDigit( 1 );
//            flags = SiiPlatformEepromReadByte( EE_FW_CONTROL );
//            flags &= ~EEFLAG_DBG_MSG_COLORING;
//            flags |= app.dbgMsgColoring ? EEFLAG_DBG_MSG_COLORING : 0;
//            SiiPlatformEepromWriteByte( EE_FW_CONTROL, flags );
//            SiiPlatformLoggerColorEnable( app.dbgMsgColoring );
//            break;
        case RC5_2:
            if ( SkAppEdidRead( EDID_NVRAM, dispData ))
            {
                uint8_t chksum;

                SkAppDisplayData( MSG_ALWAYS, dispData, EDID_TABLE_LEN );
                chksum = SiiDrvNvramEdidBlockChecksumCalc( dispData );
                DEBUG_PRINT( MSG_ALWAYS, " Checksum block 0: %02X (%02X)\n", chksum, (dispData[EDID_BLOCK_LEN - 1] - chksum) & 0xFF );
                chksum = SiiDrvNvramEdidBlockChecksumCalc( &dispData[ EDID_BLOCK_LEN] );
                DEBUG_PRINT( MSG_ALWAYS, " Checksum block 1: %02X (%02X)\n", chksum, (dispData[2 * EDID_BLOCK_LEN - 1] - chksum) & 0xFF );
            }
            else
            {
                DEBUG_PRINT( MSG_ERR, "Error reading NVRAM\n" );
            }
            break;
        case RC5_3:
            if ( SkAppEdidRead( EDID_NVRAM_BOOT, dispData ))
            {
                SkAppDisplayData( MSG_ALWAYS, dispData, EDID_DEVBOOT_LEN );
            }
            else
            {
                DEBUG_PRINT( MSG_ERR, "Error reading NVRAM BOOT DATA\n" );
            }
            break;
        case RC5_4:
            PrintAlways( " Press Power to confirm NVRAM restore: \n" );
            done = false;
            while ( !done )
            {
                rcKey = SiiPlatformInfraRedGetKey();
                switch ( rcKey )
                {
                    case RC5_STANDBY:
                        PrintAlways( "\n" );
                        if ( !SiiDrvDeviceNonVolatileInitialize( true ))
                        {
                            DEBUG_PRINT( MSG_ERR, "---ERROR IN NVRAM UPDATE\n" );
                        }
                        else
                        {
                            PrintAlways( "---NVRAM restore complete\n" );
                        }
                        done = true;
                        break;
                    case RC5_BUFFER_EMPTY:
                        break;
                    default:
                        PrintAlways( "---Canceled\n" );
                        done = true;
                        break;
                }
            }
            break;
        case RC5_5:
            SkAppSignon();
            break;
        case RC5_6:
            PrintAlways( " Press Power to confirm EEPROM restore: " );
            done = false;
            while ( !done )
            {
                rcKey = SiiPlatformInfraRedGetKey();
                switch ( rcKey )
                {
                    case RC5_STANDBY:
                        SkAppEepromInitialize( true, true );
                        PrintAlways( "---EEPROM restore complete\n" );
                        done = true;
                        break;
                    case RC5_BUFFER_EMPTY:
                        break;
                    default:
                        PrintAlways( "---Canceled\n" );
                        done = true;
                        break;
                }
            }
            break;
        case RC5_9:
            break;

        default:
            break;
    }
}

static char *pSetupIrRemoteMenu [] =
{
    "\n Setup Menu",
      " -----------",
    "0 - Return to TV Mode",
    "1 - Setup Menu One",
    "2 - Setup Menu Two",
    "3 - Diagnostics Menu",
   // "4 - Setup Menu Three",
    NULL
};

//------------------------------------------------------------------------------
// Function:    SkAppSetupIrRemote
// Description: Parse remote control keys for the setup application
//------------------------------------------------------------------------------

bool_t SkAppSetupIrRemote ( RC5Commands_t key, bool_t fullAccess )
{
    bool_t  usedKey = false;

#if (PHILIPS_REMOTE == 1)
    if ( key == RC5_TOGGLE )
#else
        if ( key == RC5_GUIDE )
#endif
    {
       PrintMenu( pSetupIrRemoteMenu );
       DEBUG_PRINT( MSG_ALWAYS, "\n Select Setup Menu (0-3):" );

       app.setupMenuLevel = SkAppIrGetSingleDigit( 4 ) + 1;
#if (PHILIPS_REMOTE == 1)
       key = RC5_CC_SUBTITLE;    // Cause new level menu to be displayed
#else
       key = RC5_HELP;    // Cause new level menu to be displayed
#endif
       }
    switch ( app.setupMenuLevel )
    {
        case 0:         // Menu idle mode
            break;
        case 1:
            SkAppIrRemoteReturnToTV();
            break;
        case 2:
            SetupLevelZero( key );
            break;
        case 3:
            SetupLevelOne( key );
            break;
#if INC_DIAG
        case 4:
            SkAppDiagLevelZero( key );
            break;
#endif
//        case 5:
//            SetupLevelTwo( key );
//            break;
        default:
            break;
    }

    return( usedKey );
}

//------------------------------------------------------------------------------
// Function:    SkAppSetup
// Description: Setup handler
//------------------------------------------------------------------------------

void SkAppSetup ( void )
{
    if ( !SiiPlatformTimerExpired( TIMER_1 ))
    {
        return;
    }
    SiiPlatformTimerSet( TIMER_1, DEM_POLLING_DELAY );     // Reset poll timer

    // Check the UI here rather than in the main loop because we don't need
    // to check it at such a high rate, and besides, if the user changed the
    // rotary switch in-between polling delays, the next time it was checked
    // it would report unchanged - you have only one chance to get the changed value.

    SkAppUiUpdate();

}

#endif // #if INC_DIAG

