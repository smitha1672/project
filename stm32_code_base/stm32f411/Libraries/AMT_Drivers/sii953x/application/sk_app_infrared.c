//***************************************************************************
//!file     sk_app_infrared.c
//!brief    Processes RC commands.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_timer.h" // for SiiPlatformTimerSysTicksGet();
#if INC_CEC
#include "si_cec_component.h"
#endif
#include "si_osal_timer.h"
#include "si_drv_switch.h"
#include "cea931_keys.h"

//------------------------------------------------------------------------------
// RC5 remote key to CEA931 key mapping
// Commented out commands below are not required for this product
//------------------------------------------------------------------------------
SiiCea931Translate_t cea931Translate[43] =
{
    {  CEA931_KEY_SELECT,          RC5_INFO,                   "Select"},
    {  CEA931_KEY_UP,              RC5_CURSOR_STEP_UP,         "Up"},
    {  CEA931_KEY_DOWN,            RC5_CURSOR_STEP_DOWN,       "Down"},
    {  CEA931_KEY_LEFT,            RC5_CURSOR_STEP_LEFT,       "Left"},
    {  CEA931_KEY_RIGHT,           RC5_CURSOR_STEP_RIGHT,      "Right"},
    /*
    {  CEA931_KEY_RIGHT_UP,        RC5_UNKNOWN,                 "Right Up"},
    {  CEA931_KEY_RIGHT_DOWN,      RC5_UNKNOWN,                 "Right Down"},
    {  CEA931_KEY_LEFT_UP,         RC5_UNKNOWN,                 "Left Up"},
    {  CEA931_KEY_LEFT_DOWN,       RC5_UNKNOWN,                 "Left Down"},
    */
    {  CEA931_KEY_ROOT_MENU,       RC5_MENU,                   "Root Menu"},
    /*
    {  CEA931_KEY_SETUP_MENU,      RC5_UNKNOWN,                 "Setup Menu"},
    {  CEA931_KEY_CONTENTS_MENU,   RC5_UNKNOWN,                 "Contents Menu"},
    {  CEA931_KEY_FAVORITE_MENU,   RC5_UNKNOWN,                 "Favorite Menu"},
    */
    {  CEA931_KEY_EXIT,            RC5_UNKNOWN,                "Exit"},
       //0x0E - 0x1F Reserved
    {  CEA931_KEY_NUM_0,           RC5_0,                      "Num 0"},
    {  CEA931_KEY_NUM_1,           RC5_1,                      "Num 1"},
    {  CEA931_KEY_NUM_2,           RC5_2,                      "Num 2"},
    {  CEA931_KEY_NUM_3,           RC5_3,                      "Num 3"},
    {  CEA931_KEY_NUM_4,           RC5_4,                      "Num 4"},
    {  CEA931_KEY_NUM_5,           RC5_5,                      "Num 5"},
    {  CEA931_KEY_NUM_6,           RC5_6,                      "Num 6"},
    {  CEA931_KEY_NUM_7,           RC5_7,                      "Num 7"},
    {  CEA931_KEY_NUM_8,           RC5_8,                      "Num 8"},
    {  CEA931_KEY_NUM_9,           RC5_9,                      "Num 9"},
    /*
    {  CEA931_KEY_DOT,             RC5_UNKNOWN,                "Dot"},
    */
    {  CEA931_KEY_ENTER,           RC5_UNKNOWN,                "Enter"},
    {  CEA931_KEY_CLEAR,           RC5_UNKNOWN,                "Clear"},
       //0x2D - 0x2F Reserved
    {  CEA931_KEY_CH_UP,           RC5_PRG_UP,                 "Channel Up"},
    {  CEA931_KEY_CH_DOWN,         RC5_PRG_DOWN,               "Channel Down"},
    {  CEA931_KEY_PRE_CH,          RC5_ALT_CHANNEL,            "Previous Channel"},
    {  CEA931_KEY_SOUND_SELECT,    RC5_UNKNOWN,                "Sound Select"},
    /*
    {  CEA931_KEY_INPUT_SELECT,    RC5_INPUT,                  "Input Select"},
    {  CEA931_KEY_SHOW_INFO,       RC5_UNKNOWN,                "Show Info"},
    {  CEA931_KEY_HELP,            RC5_UNKNOWN,                "Help"},
    {  CEA931_KEY_PAGE_UP,         RC5_UNKNOWN,                "Page Up"},
    {  CEA931_KEY_PAGE_DOWN,       RC5_UNKNOWN,                "Page Down"},
    */
       //0x39 - 0x3F Reserved
    {  CEA931_KEY_POWER_TOGGLE_FUNC,    RC5_STANDBY,           "Power"},
    {  CEA931_KEY_VOL_UP,               RC5_VOL_UP,            "Volume Up"},
    {  CEA931_KEY_VOL_DOWN,             RC5_VOL_DOWN,          "Volume Down"},
    {  CEA931_KEY_MUTE,                 RC5_MUTE,           "Mute"},
    {  CEA931_KEY_PLAY,                 RC5_PLAY,              "Play"},
    {  CEA931_KEY_STOP,                 RC5_STOP,              "Stop"},
    {  CEA931_KEY_PAUSE,                RC5_PAUSE,             "Pause"},
#if (PHILIPS_REMOTE == 1)
    {  CEA931_KEY_RECORD,               RC5_TOGGLE,            "Record"},
#else
    {  CEA931_KEY_RECORD,               RC5_GUIDE,            "Record"},
#endif
    {  CEA931_KEY_REWIND,               RC5_FAST_REW,          "Rewind"},
    {  CEA931_KEY_FAST_FWD,             RC5_FAST_FWD,          "Fast Fwd"},
    {  CEA931_KEY_EJECT,                RC5_UNKNOWN,           "Eject"},
    {  CEA931_KEY_FWD,                  RC5_UNKNOWN,           "Forward"},
    {  CEA931_KEY_BKWD,                 RC5_UNKNOWN,           "Backward"},
       //0X4D - 0x4F Reserved
    /*
    {  CEA931_KEY_ANGLE,           RC5_UNKNOWN,                "Angle"},
    {  CEA931_KEY_SUBPICTURE,      RC5_UNKNOWN,                "Subpicture"},
    */
       //0x52 - 0x5F Reserved
    {  CEA931_KEY_PLAY_FUNC,       RC5_UNKNOWN,                "Play Function"},
    {  CEA931_KEY_PAUSE_PLAY_FUNC, RC5_UNKNOWN,                "Pause Play Function"},
    {  CEA931_KEY_RECORD_FUNC,     RC5_UNKNOWN,                "Record Function"},
    {  CEA931_KEY_PAUSE_REC_FUNC,  RC5_UNKNOWN,                "Pause Record Function"},
    {  CEA931_KEY_STOP_FUNC,       RC5_UNKNOWN,                "Stop Function"},
    {  CEA931_KEY_MUTE_FUNC,       RC5_UNKNOWN,                "Mute Function"},
    {  CEA931_KEY_UN_MUTE_FUNC,    RC5_UNKNOWN,                "Un-Mute Function"},
    /*
    {  CEA931_KEY_TUNE_FUNC,       RC5_UNKNOWN,                "Tune Function"},
    {  CEA931_KEY_MEDIA_FUNC,      RC5_UNKNOWN,                "Media Function"},
       //0x69 - 0x70 Reserved
    {  CEA931_KEY_F1,              RC5_UNKNOWN,                "F1"},
    {  CEA931_KEY_F2,              RC5_UNKNOWN,                "F2"},
    {  CEA931_KEY_F3,              RC5_UNKNOWN,                "F3"},
    {  CEA931_KEY_F4,              RC5_UNKNOWN,                "F4"},
    {  CEA931_KEY_F5,              RC5_UNKNOWN,                "F5"}
    {  //0x76
    */
       // 0x77 - 0x7F Reserved
};

//------------------------------------------------------------------------------
//! @brief  Convert the passed RC5 key code to CEA-931 spec key code.
//! @param[in]  keyData - RC5 key code
//! @return     uint8_t - CEA931 key code
//------------------------------------------------------------------------------
uint8_t SkAppIrConvertToCea931 ( uint8_t keyData )
{
    uint8_t i, keyCompare;
    uint8_t retVal = 0xFF;
    uint8_t length = sizeof(cea931Translate)/sizeof(SiiCea931Translate_t);

    // Translate RC5 key to CEA931 key
    for ( i = 0; i < length ; i++ )
    {
        keyCompare = cea931Translate[i].rc5KeyCode;
        if ( keyData == keyCompare )
        {
            retVal = cea931Translate[i].rcpKeyCode;
            DEBUG_PRINT( MSG_DBG, "RCP:: Key ID: %s\n", cea931Translate[i].rcpName );
            break;
        }
    }

    /* Return the new code or 0xFF if not found.    */

    return( ( i == length ) ? 0xFF : retVal );
}

//------------------------------------------------------------------------------
// Module data
//------------------------------------------------------------------------------

extern bool_t SkRemoteRequestHandler ( void );

char *zoneMsgs [] =
{
    "Zone 1",
    "Zone 2"
};

//------------------------------------------------------------------------------
// Function:    SkAppIrGetSingleDigit
// Description: Waits for user to press digit key within passed range.
// Parameters:  rangeEnd - maximum value (0 is minimum)
// Returns:     Valid digit pressed by user.
//------------------------------------------------------------------------------

uint8_t SkAppIrGetSingleDigit ( uint8_t rangeEnd )
{
    uint16_t        rcKey;
    uint8_t         digitValue = 0;
    bool_t          done = false;
    static uint16_t oldKey = 0xFF;
    int_t           rc5Address;
    uint8_t         irOldUartKey=0xFF;
    
	app.irUartKey = 0xFF;


    while ( !done )
    {
    	SkRemoteRequestHandler();
    	    	
        rcKey = SiiPlatformInfraRedGetKey();

        //DEBUG_PRINT( MSG_ALWAYS, "\nSkAppIrGetSingleDigit:1 Key code: %x\n",rcKey );

	    if (( rcKey != RC5_BUFFER_EMPTY) && (rcKey != oldKey ))
	    {
	        oldKey = rcKey;
	        rc5Address = (rcKey & RC5_ADDRESS_FIELD) >> 6;
	        rcKey &= RC5_COMMAND_FIELD;     // We don't care if it's a TV or VCR

	        //DEBUG_PRINT( MSG_ALWAYS, "\nSkAppIrGetSingleDigit:2 Key code: %x\n",rcKey );
	    }

       	if (app.irUartKey != irOldUartKey)
        {
			rcKey = app.irUartKey;
			irOldUartKey = 0xFF;
			app.irUartKey = 0xFF;
        }

        switch ( rcKey )
        {
            case RC5_QUIT:
                // Cancel mode
                done = true;
                break;

            case RC5_0:
            case RC5_1:
            case RC5_2:
            case RC5_3:
            case RC5_4:
            case RC5_5:
            case RC5_6:
            case RC5_7:
            case RC5_8:
            case RC5_9:
                digitValue = rcKey;
                if ( digitValue > rangeEnd )
                    break;

                DEBUG_PRINT( MSG_ALWAYS, "%d \n", (int)digitValue );
                done = true;
                break;
            case RC5_BUFFER_EMPTY:
                break;
        }
    }

    return( digitValue );
}

// Function:    SkAppIrRemoteReturnToTV
// Description: 
//------------------------------------------------------------------------------

void SkAppIrRemoteReturnToTV ( void )
{
    app.modeSelect = SK_MODE_TV;
    PrintAlways( "\nTV Mode -- Press GUIDE key to exit Demo mode\n" );
}

//------------------------------------------------------------------------------
// Function:    IrRemoteKeyParser
// Description: Parse special 'superuser' keys for the IR task.
//------------------------------------------------------------------------------
static char *pAppModeMenu [] =
{
    "\n Application Mode",
      " ----------------",
    "0 - TV",
    "1 - Demo",
    "2 - Setup",
    NULL
};

static bool_t IrRemoteKeyParser ( RC5Commands_t key )
{
    bool_t  usedKey = false;
   uint8_t digit;

    switch ( key )
    {
        case RC5_STANDBY:
            if ( app.powerState == APP_POWERSTATUS_ON )
            {
                app.powerState = APP_POWERSTATUS_ON_TO_STANDBY;
            }
            else if (( app.powerState == APP_POWERSTATUS_STANDBY ) || ( app.powerState == APP_POWERSTATUS_INITIAL ))
            {
                app.powerState = APP_POWERSTATUS_STANDBY_TO_ON;
            }
            usedKey = true;
            break;

#if (PHILIPS_REMOTE == 1)
        case RC5_TOGGLE:
#else
        case RC5_GUIDE:
#endif
           // Application Mode select only available in TV Mode (Main App Mode)
#if SII_MARKETING_DEMO
           usedKey = true;
		   app.modeSelect = SK_MODE_TV;
		   return( usedKey );
#endif
           if ( app.modeSelect == SK_MODE_TV )
           {
               PrintMenu( pAppModeMenu );
               //DEBUG_PRINT( MSG_ALWAYS, "3 - Select Remote Control Zone (%d, tracking: %s)\n", app.currentZone + 1, SiiDrvSwitchZoneTrackingGet() ? "ON" : "OFF" );
               //DEBUG_PRINT( MSG_ALWAYS, "3 - Enable/Disable Zone %d (%s)\n", app.currentZone + 1, app.newZoneEnabled[app.currentZone] ? "Enabled" : "Disabled" );
               DEBUG_PRINT( MSG_ALWAYS, "\n Mode (0-%d): ", 2 );

               digit = SkAppIrGetSingleDigit( 2 );
               usedKey = false;
               app.modeSelect = digit;
               usedKey = false; // Leave the key unused so that it gets passed to the new mode to display its menu

           }
           break;

       default:
           break;
    }

    return( usedKey );
}


//------------------------------------------------------------------------------
// Function:    SkAppParseKey
// Description: Parse the key from IR or UART.
// Parameters:  fullAccess - true if allowed to respond to all normal keys
//                           false if only allowed to respond to power keys
// Returns:     none
//------------------------------------------------------------------------------
void SkAppParseKey ( uint16_t irKey, bool_t fullAccess  )
{
    RC5Commands_t   key;

    if ( irKey != RC5_BUFFER_EMPTY)
    {
        key = (RC5Commands_t)irKey;
        //DEBUG_PRINT( MSG_ALWAYS, "\nSkAppParseKey:1 Key code: %x\n",key );

        for ( ;; )
        {
            if ( IrRemoteKeyParser( key ))
            {
                break;
            }

            // Run the appropriate key parser

            switch ( app.modeSelect )
            {
                case SK_MODE_TV:
                    SkAppTvIrRemote( key, fullAccess );
                    break;

                case SK_MODE_DEMO:
                    SkAppDemoIrRemote( key, fullAccess );
                    break;

                case SK_MODE_SETUP:
                    SkAppSetupIrRemote( key, fullAccess );
                    break;

#if INC_BENCH_TEST
                case SK_MODE_BTST:
                    SkAppBenchTestIrRemote( key, fullAccess );
                    break;
#endif

                default:
                    break;
            }

            break;  // Always break if you get this far...
        }
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskIrRemote
// Description: IR Remote Control task.
// Parameters:  fullAccess - true if allowed to respond to all normal keys
//                           false if only allowed to respond to power keys
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskIrRemote ( bool_t fullAccess )
{
    uint16_t        irKey;
    static uint16_t oldKey = 0xFF;

    if ( !app.irEnable )
    {
        return;
    }

    irKey = SiiPlatformInfraRedGetKey();

//    if(irKey != RC5_BUFFER_EMPTY)
//    {
//        DEBUG_PRINT( MSG_ALWAYS, "\nSkAppTaskIrRemote:1 Key code: %x\n",irKey );
//    }

    if ( irKey != RC5_BUFFER_EMPTY )
    {
        oldKey = irKey;

        irKey &= RC5_COMMAND_FIELD;     // We don't care if it's a TV or VCR

//        DEBUG_PRINT( MSG_ALWAYS, "\nSkAppTaskIrRemote:2 Key code: %x\n",irKey );
    }

    SkAppParseKey( irKey, fullAccess );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Callback from SiiMon debugger.
//!
//!             Brings simulated IR codes from SiIMon tool.
//!
//! @param[in]  irKey  - IR Key code
//! @param[in]  irAddr - RC5 Address
//-------------------------------------------------------------------------------------------------

void SiiMonCbIrCmdSet(uint8_t irKey, uint8_t irAddr)
{
    app.irUartKey = irKey;
    app.irUartAddr = irAddr ;
}
