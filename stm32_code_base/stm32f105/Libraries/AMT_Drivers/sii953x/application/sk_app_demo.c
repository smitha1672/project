//***************************************************************************
//!file     sk_app_demo.c
//!brief    Built-in Demo manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_timer.h"

#include "sk_application.h"
#if INC_CBUS
#include "sk_app_cbus.h"
#endif
#if INC_CEC
#include "si_cec_component.h"
#endif
#include "si_tx_component.h"
#include "si_drv_tpi_system.h"
#include "si_drv_switch.h"
#if INC_AUDIO_RX
#include "si_drv_audio.h"
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Service function for IV & PV demo.
//-------------------------------------------------------------------------------------------------
static void DemoLocalVideoModeEnable ( bool_t isEnabled, SiiSwitchSource_t txSrc[2])
{
    uint8_t i;

    if (isEnabled)
    {
        for (i = 0; i < SII_NUM_TX; ++i)
        {
            // Save current state of TX source type configuration
            if(app.currentSource[i] != txSrc[i])
            {
                app.txSourcePipe[i] = app.currentSource[i];
            }
            // Select source for TX0 and TX1
            app.newSource[i] = txSrc[i];
        }
    }
    else
    {
        for (i = 0; i < SII_NUM_TX; ++i)
        {
            // Recover source for TX0 and TX1
            app.newSource[i] = app.txSourcePipe[i];
        }
    }

}

//-------------------------------------------------------------------------------------------------
//! @brief      TPG on/off demo.
//-------------------------------------------------------------------------------------------------
#if INC_IV
void DemoTpgModeEnable ( bool_t isEnabled )
{
    SiiSwitchSource_t txSrc[2] = {SiiSwitchSourceInternalVideo, SiiSwitchSourceInternalVideo};
    DemoLocalVideoModeEnable(isEnabled, txSrc);
}
#endif

#if INC_IV

static char *patternNames [] =
{
    "Red", "Green", "Blue", "Black", "White", "Ramp", "Chess board", "Color Bars"
};

static char * PatternNameGet(tpgVideoPattern_t patternCode)
{
    if ( patternCode < 8 )
    {
        return( patternNames[ patternCode]);
    }
    else
    {
        return( "-" );
    }
}

//------------------------------------------------------------------------------
// Function:    SkDemoTpg
// Description: Internal Video (TPG) demo command options
// Returns:
//------------------------------------------------------------------------------

static char *pTpgMenuNames [] =
{
    "\nInternal Video Demo",
    "--------",
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    "1 - IV display ON",
    "2 - IV display OFF",
    "3 - Change Video mode",
    "4 - Change Audio type",
    "5 - Change Audio Source",
    "6 - Change VCLK Source",
    "7 - Change Video Pattern",
    NULL
};

bool_t SkDemoTpg ( RC5Commands_t key )
{
    uint8_t i;
    uint8_t digit;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pTpgMenuNames );
            //PrintStringOnOff( "8 - HDCP", !app.isTxHdcpRequired[0] ); // Bug 33490 - Melbourne SiI9535 | FW Menu System|Internal Video Demo|HDCP fails to work

            break;

        case RC5_0:
            app.demoMenuLevel = 0;
            PrintAlways( demoMenuExit );
            break;

        case RC5_1:
            DemoTpgModeEnable(true);
            break;

        case RC5_2:
            DemoTpgModeEnable(false);
            break;

        case RC5_3:
            PrintAlways( tenDashes );
            PrintStringCheckMark( "0 - 480p60", (app.tpgVideoFormat == SI_TPG_FMT_VID_480_60) );
            PrintStringCheckMark( "1 - 576p50", (app.tpgVideoFormat == SI_TPG_FMT_VID_576_50) );
            PrintStringCheckMark( "2 - 720p60", (app.tpgVideoFormat == SI_TPG_FMT_VID_720_60) );
            PrintStringCheckMark( "3 - 720p50", (app.tpgVideoFormat == SI_TPG_FMT_VID_720_50) );
            PrintStringCheckMark( "4 - 1080p60-8b", (app.tpgVideoFormat == SI_TPG_FMT_VID_1080_60_8bpp) );
            PrintStringCheckMark( "5 - 1080p50-8b", (app.tpgVideoFormat == SI_TPG_FMT_VID_1080_50_8bpp) );
            PrintStringCheckMark( "6 - 1080p60-10b", (app.tpgVideoFormat == SI_TPG_FMT_VID_1080_60_10bpp) );
            PrintStringCheckMark( "7 - 1080p50-10b", (app.tpgVideoFormat == SI_TPG_FMT_VID_1080_50_10bpp) );
            PrintStringCheckMark( "8 - 1080p60-12b", (app.tpgVideoFormat == SI_TPG_FMT_VID_1080_60_12bpp) );
            PrintStringCheckMark( "9 - 1080p50-12b", (app.tpgVideoFormat == SI_TPG_FMT_VID_1080_50_12bpp) );

            digit = SkAppIrGetSingleDigit(10);
            switch (digit)
            {
                default:
                case 0:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_480_60;
                    PrintAlways( "TPG RES: 480p60, VCLK = 27.00 MHz\n" );
                    break;
                case 1:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_576_50;
                    PrintAlways( "TPG RES: 576p50, VCLK = 27.00 MHz\n" );
                    break;
                case 2:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_720_60;
                    PrintAlways( "TPG RES: 720p60, VCLK = 74.25 MHz\n" );
                    break;
                case 3:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_720_50;
                    PrintAlways( "TPG RES: 720p50, VCLK = 74.25 MHz\n" );
                    break;
                case 4:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_1080_60_8bpp;
                    PrintAlways( "TPG RES: 1080p60-8b, VCLK = 148.5 MHz\n" );
                    break;
                case 5:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_1080_50_8bpp;
                    PrintAlways( "TPG RES: 1080p50-8b, VCLK = 148.5 MHz\n" );
                    break;
                case 6:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_1080_60_10bpp;
                    PrintAlways( "TPG RES: 1080p60-10b, VCLK = 185.625 MHz\n" );
                    break;
                case 7:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_1080_50_10bpp;
                    PrintAlways( "TPG RES: 1080p50-10b, VCLK = 185.625 MHz\n" );
                    break;
                case 8:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_1080_60_12bpp;
                    PrintAlways( "TPG RES: 1080p60-12b, VCLK = 222.75 MHz\n" );
                    break;
                case 9:
                    app.tpgVideoFormat = SI_TPG_FMT_VID_1080_50_12bpp;
                    PrintAlways( "TPG RES: 1080p50-12b, VCLK = 222.75 MHz\n" );
                    break;
            }
            break;

        case RC5_4:
            PrintAlways( tenDashes );
            DEBUG_PRINT( MSG_ALWAYS, "0 - TX %s SPDIF\n", (app.currentAudioType[0] == SI_TX_AUDIO_SRC_SPDIF) ? "<--" : "   ");
            DEBUG_PRINT( MSG_ALWAYS, "1 - TX %s I2S 2ch\n", (app.currentAudioType[0] == SI_TX_AUDIO_SRC_I2S_L0) ? "<--" : "   ");

            digit = SkAppIrGetSingleDigit(1);
            switch (digit)
            {
                case 0:
                    app.currentAudioType[0] = SI_TX_AUDIO_SRC_SPDIF;
                    PrintAlways( "SPDIF >> TX\n" );
                    break;
                case 1:
                    app.currentAudioType[0] = SI_TX_AUDIO_SRC_I2S_L0;
                    PrintAlways( "I2S 2ch >> TX\n" );
                    break;
            }
            break;

        case RC5_5:
            PrintAlways( tenDashes );
            DEBUG_PRINT( MSG_ALWAYS, "0 - TX %s DEF\n", (app.currentAudioSource[0] == SiiSwitchAudioSourceDefault) ? "<--" : "   ");
            DEBUG_PRINT( MSG_ALWAYS, "1 - TX %s EXT\n", (app.currentAudioSource[0] == SiiSwitchAudioSourceExternal) ? "<--" : "   ");

            digit = SkAppIrGetSingleDigit(1);
            switch (digit)
            {
                case 0:
                    app.newAudioSource[0] = SiiSwitchAudioSourceDefault;
                    PrintAlways( "DEFAULT\n" );
                    break;

                case 1:
                    app.newAudioSource[0] = SiiSwitchAudioSourceExternal;
                    PrintAlways( "EXT\n" );
                    break;
            }
            break;

        case RC5_6:
            PrintAlways( tenDashes );
            PrintStringCheckMark( "0 - Main Pipe", (app.tpgClockSrc == SI_TPG_CLK_MAIN_PIPE) );
            PrintStringCheckMark( "1 - XCLK", (app.tpgClockSrc == SI_TPG_CLK_XCLK) );
            PrintStringCheckMark( "2 - Audio PLL 0", (app.tpgClockSrc == SI_TPG_CLK_P0) );
            digit = SkAppIrGetSingleDigit(2);
            switch (digit)
            {
                case 0:
                    app.tpgClockSrc = SI_TPG_CLK_MAIN_PIPE;
                    PrintAlways( "MP CLK\n" );
                    break;
                case 1:
                    app.tpgClockSrc = SI_TPG_CLK_XCLK;
                    PrintAlways( "XCLK\n" );
                    break;
                case 2:
                    app.tpgClockSrc = SI_TPG_CLK_P0;
                    PrintAlways( "AUD PLL 0\n" );
                    break;
            }
            break;

        case RC5_7:
            while(true)
            {
                PrintAlways( "Press 0 to Quit, (1-8) to Select Pattern:\n--------\n");
                for (i = 0; i < 8; ++i)
                {
                    DEBUG_PRINT( MSG_ALWAYS,"%s\t%d - %s\n", (i == app.tpgVideoPattern) ? " -->": "", i + 1, PatternNameGet(i));
                }
                digit = SkAppIrGetSingleDigit(8);
                if (digit == 0)
                {
                    break;
                }
                SiiDrvExtTpgPatternSet((tpgVideoPattern_t) (digit - 1));
                app.tpgVideoPattern = digit - 1;
            }
            break;

        case RC5_8:
            // Bug 33490 - Melbourne SiI9535 | FW Menu System|Internal Video Demo|HDCP fails to work
            //app.isTxHdcpRequired[0] = !app.isTxHdcpRequired[0]; // Toggle TX0 HDCP
            //app.isTxHdcpRequired[1] = !app.isTxHdcpRequired[1]; // Toggle TX1 HDCP
            //PrintStringOnOff( "TX0 HDCP", app.isTxHdcpRequired[0] );
            //PrintStringOnOff( "TX1 HDCP", app.isTxHdcpRequired[1] );
            break;

        default:
            break;
    }

    return( false );
}

#endif

static char * AudioTypeNameGet(txAudioSrc_t audioTypeCode)
{
    char *pName;

    switch(audioTypeCode)
    {
        default:
         case SI_TX_AUDIO_SRC_NONE:
             pName = "None";
             break;
         case SI_TX_AUDIO_SRC_I2S_L0:
         case SI_TX_AUDIO_SRC_I2S_L1:
             pName = "I2S";
             break;
         case SI_TX_AUDIO_SRC_HBRA:
             pName = "HBR";
             break;
         case SI_TX_AUDIO_SRC_DSD_L0:
         case SI_TX_AUDIO_SRC_DSD_L1:
             pName = "DSD";
             break;
         case SI_TX_AUDIO_SRC_SPDIF:
             pName = "SPDIF";
             break;
    }
    return pName;
}


//------------------------------------------------------------------------------
// Function:    SkAppAudioDemo
// Description: Internal and External Audio insertion demo
// Returns:
//------------------------------------------------------------------------------
static char *pAudioDemoMenu [] =
{
    "\nAudio Demo",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    NULL
};

bool_t SkAppAudioDemo ( RC5Commands_t key )
{
    uint8_t digit;

    switch (key)
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pAudioDemoMenu );
            DEBUG_PRINT( MSG_ALWAYS, "1 - External Audio Format (%s)\n", AudioTypeNameGet(app.currentAudioType[0]) );
            PrintStringOnOff( "2 - External Audio Insertion", (app.currentAudioSource[0] == SiiSwitchAudioSourceExternal) );
            PrintStringOnOff( "3 - Internal Audio Insertion", (app.currentAudioSource[0] == SiiSwitchAudioSourceSubPipe) );
            PrintStringOnOff( "4 - Multi-channel Audio extraction from MP", app.isAudioExtractionEnabled[0] );
            break;

        case RC5_0:
            app.demoMenuLevel = 0;
            PrintAlways( demoMenuExit );
            break;

        case RC5_1:
            PrintAlways( tenDashes );
            DEBUG_PRINT( MSG_ALWAYS, "0 - Tx %s SPDIF\n", (app.currentAudioType[0] == SI_TX_AUDIO_SRC_SPDIF) ? "<--" : "   ");
            DEBUG_PRINT( MSG_ALWAYS, "1 - Tx %s I2S 2ch\n", (app.currentAudioType[0] == SI_TX_AUDIO_SRC_I2S_L0) ? "<--" : "   ");

            digit = SkAppIrGetSingleDigit(3);
            switch (digit)
            {
                case 0:
                    app.currentAudioType[0] = SI_TX_AUDIO_SRC_SPDIF;
                    PrintAlways( "SPDIF >> Tx\n" );
                    break;
                case 1:
                    app.currentAudioType[0] = SI_TX_AUDIO_SRC_I2S_L0;
                    PrintAlways( "I2S 2ch >> Tx\n" );
                    break;
            }
            DEBUG_PRINT( MSG_ALWAYS, "!! Check Jumper(JP10) Settings \n");
            break;

        case RC5_2:
            if (app.currentAudioSource[0] == SiiSwitchAudioSourceExternal)
            {
                app.newAudioSource[0] = SiiSwitchAudioSourceDefault;
                PrintAlways( "Audio SRC: Default\n" );
            }
            else
            {
                app.newAudioSource[0] = SiiSwitchAudioSourceExternal;
                PrintAlways( "Audio SRC: External\n" );
                PrintAlways( "!! Check External Audio Format and Jumper (JP10) Settings \n" );
            }
            break;

        case RC5_3:
            if (app.currentAudioSource[0] == SiiSwitchAudioSourceSubPipe)
            {
                app.newAudioSource[0] = SiiSwitchAudioSourceDefault;
                PrintAlways( "Audio SRC: Default\n" );
            }
            else
            {
#if INC_IPV
            	if(app.ipvPipEnabled)
            	{
            		app.newAudioSource[0] = SiiSwitchAudioSourceSubPipe;
            		PrintAlways( "Audio SRC: Sub Pipe\n" );
            	}
            	else
            	{
            		PrintAlways( "\n\r!!!Internal Audio insertion will work only when PIP is Enabled!!!\n" );
            	}
#else
            	PrintAlways( "\n\rIPV is Disabled\n\r" );
#endif
            }
            break;

        case RC5_4:
            app.isAudioExtractionEnabled[0] = !app.isAudioExtractionEnabled[0];
            (void)SiiDrvRxAudioInstanceSet(0);
            SiiDrvRxAudioMute(!app.isAudioExtractionEnabled[0]);
            DEBUG_PRINT( MSG_ALWAYS, "MP Audio Extraction %s\n", app.isAudioExtractionEnabled[0] ? "Enabled" : "Disabled" );
            break;

        default:
            break;
    }

    return (false);
}


//------------------------------------------------------------------------------
// Function:    SkAppDemoIrRemote
// Description: Parse remote control keys for the Demo application
//------------------------------------------------------------------------------

bool_t SkAppDemoIrRemote ( RC5Commands_t key, bool_t fullAccess )
{
    bool_t  usedKey = false;

#if (PHILIPS_REMOTE == 1)
    if ( key == RC5_TOGGLE )
#else
        if ( key == RC5_GUIDE )
#endif
    {
        PrintAlways( "\n Demo Menu\n" );
        PrintAlways( " -----------\n" );
        PrintAlways( "0 - Return to TV Mode\n" );
        PrintAlways( "1 - Audio Demo\n" );
#if INC_ARC
        PrintAlways( "2 - ARC Demo\n" );
#endif
        PrintAlways( "3 - Internal Video Demo\n" );
#if INC_CBUS
      //  PrintAlways( "4 - CBUS Demo\n" );
#endif
#if INC_IPV
        PrintAlways( "4 - InstaPrevue Demo\n" );
#endif
#if INC_CEC
        PrintAlways( "5 - CEC Demo\n" );
#endif

#if INC_CEC
        PrintAlways( "\n Select Demo (0-5): " );
#else
        PrintAlways( "\n Select Demo (0-4): " );
#endif

       app.demoMenuLevel = SkAppIrGetSingleDigit( 6 ) + 1;
#if (PHILIPS_REMOTE == 1)
       key = RC5_CC_SUBTITLE;    // Cause new level menu to be displayed
#else
       key = RC5_HELP;    // Cause new level menu to be displayed
#endif
    }

    switch ( app.demoMenuLevel )
    {
        case 0:     // Menu IDLE mode
            break;
        case 1:
            SkAppIrRemoteReturnToTV();
            break;
        case 2:
            SkAppAudioDemo( key );
            break;
#if INC_ARC
        case 3:
            SkAppArcDemo( key );
            break;
#endif
#if INC_IV
        case 4:
            SkDemoTpg( key );
            break;
#endif
#if INC_CBUS
//        case 5:
//            SkAppCbusDemo( key );
//            break;
#endif
#if INC_IPV
        case 5:
            SkAppIpvDemo( key );
            break;
#endif
#if INC_CEC
        case 6:
            SkAppCecDemo( key );
            break;
#endif

        default:
            break;
    }

    return( usedKey );
}

//------------------------------------------------------------------------------
// Function:    SkAppDemo
// Description: Demo handler
//------------------------------------------------------------------------------

void SkAppDemo ( void )
{
    if ( !SiiPlatformTimerExpired( TIMER_1 ))
    {
        return;
    }
    SiiPlatformTimerSet( TIMER_1, DEM_POLLING_DELAY );     // Reset poll timer

    // Check the UI here rather than in the main loop because we don't need
    // to check it at such a high rate, and besides, if the user changed the
    // rotary switch in between polling delays, the next time it was checked
    // it would report unchanged - you have only one chance to get the changed value.

    SkAppUiUpdate();

}

