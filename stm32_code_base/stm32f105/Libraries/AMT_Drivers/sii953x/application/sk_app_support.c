//***************************************************************************
//!file     sk_app_support.c
//!brief    Silicon Image SiI953x Starter Kit Application support functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "device_config.h"
#include "si_i2c.h"
#include "si_timer.h"

#if ( configSII_DEV_953x_PORTING == 0 )
#include "si_flash_update.h"
#endif 

#include "si_eeprom.h"
#include "si_platform.h"
#include "si_debugger_hdmigear.h"
#include "si_gpio.h"

#include "sk_application.h"
#include "sk_app_edid.h"
#if INC_CBUS
#include "sk_app_cbus.h"
#endif
#if INC_CEC
#include "sk_app_cec.h"
#endif
#include "si_drv_nvram_sram.h"
#include "si_drv_device.h"

#if INC_RTPI
#include "si_rtpi_component.h"
#endif

char tenDashes [] = "----------------\n";
char asterisks [] = "****************************************************\n";

char demoMenuExit [] = "\n Demo Menu -- Press GUIDE key to exit Demo mode\n";
char setupSelectMenu [] = "\n Setup Level -- Press GUIDE key to select Setup Menu\n";
char ccDisplayThisMenuMsg [] = "[HELP] - Display this menu";
char returnToSetupMenuMsg [] = "0 - Return to main Setup Menu";
char returnToMainDemoMsg [] = "0 - Return to main Demo Menu";

//-------------------------------------------------------------------------------------------------
//! @brief      DEBUG_PRINT( MSG_ALWAYS ) helpers
//-------------------------------------------------------------------------------------------------
void PrintMenu ( char *pMenu[] )
{
    int i = 0;

    while ( pMenu[i])
    {
        DEBUG_PRINT( MSG_ALWAYS, "%s\n", pMenu[i] );
        i++;
    }
}

void PrintStringCheckMark ( char *pString, bool_t isChecked )
{
    DEBUG_PRINT( MSG_ALWAYS, "%s %c\n", pString, (isChecked) ? '*' : ' ');
}

void PrintStringOnOff ( char *pString, bool_t isOn )
{
    DEBUG_PRINT( MSG_ALWAYS, "%s (%s)\n", pString, (isOn) ? "ON" : "OFF" );
}

void PrintStringYesNo ( char *pString, bool_t isYes )
{
    DEBUG_PRINT( MSG_ALWAYS, "%s %s\n", pString, (isYes) ? "YES" : "NO" );
}

void PrintAlways ( char *pStr )
{
    DEBUG_PRINT( MSG_ALWAYS, pStr );
}

//------------------------------------------------------------------------------
// Function:    SkAppDisplayData
// Description: Display the passed buffer in ASCII-HEX block format
//------------------------------------------------------------------------------

void SkAppDisplayData ( uint8_t msgLevel, uint8_t *pData, uint16_t length )
{
    uint16_t    i = 0;
    uint8_t     x, y;

    for ( y = 0; i < length; y++ )
    {
        DEBUG_PRINT( msgLevel, "\n%02X: ", i );
        for ( x = 0; (i < length) && (x < 16); x++ )
        {
            DEBUG_PRINT( msgLevel, " %02X", (uint16_t)pData[ i++] );
        }
    }

    DEBUG_PRINT( msgLevel, "\n" );
}

//------------------------------------------------------------------------------
//! @brief  Initialize any uninitialized areas of the EEPROM
//-----------------------------------------------------------------------------c
bool_t SkAppEepromInitialize( bool_t useDefaults, bool_t forceInit )
{
    bool_t  updated = false;

#if ( configSII_DEV_953x_PORTING == 0 ) /*Smith marks, we do not need to store this config in our system.*/
    uint8_t controlByte;
    uint8_t initBuffer [256];

    // Re-initialize the EEPROM the database is wrong or corrupt, or we're being forced.
    if ( forceInit ||
        ( SiiPlatformEepromReadByte( EE_CONFIG_VALID ) != CONFIG_VALID)  ||
        ( SiiPlatformEepromReadByte( EE_DB_VERSION ) != EEPROM_DB_VERSION))
    {
        if ( useDefaults )
        {
            SiiPlatformEepromWriteByte( EE_CONFIG_VALID, 0 );    // In case we get interrupted before we are finished
            memset( initBuffer, 0xFF, 256 );
            SiiPlatformEepromWriteBlock( 0, initBuffer, 256 );
            SiiPlatformEepromWriteByte( EE_FW_CONTROL, (EEFLAG_ROTARY_ENABLE | EEFLAG_IR_ENABLE | EEFLAG_CEC_ENABLE | EEFLAG_CEC_PRINT ));
            SiiPlatformEepromWriteByte( EEPROM_EDID_LOADSEL, 0 );
            SiiPlatformEepromWriteByte( EE_DB_VERSION, EEPROM_DB_VERSION );
            SiiPlatformEepromWriteByte( EE_UART_MSG_LEVEL, MSG_DBG );
        }
        else
        {
            controlByte =
                app.irEnable        ? EEFLAG_IR_ENABLE          : 0  |
                app.rotaryDisabled  ? 0                         : EEFLAG_ROTARY_ENABLE  |
                app.cecEnum         ? EEFLAG_CEC_ENUM           : 0  |
                app.cecPrint        ? EEFLAG_CEC_PRINT          : 0  |
                app.dbgMsgColoring  ? EEFLAG_DBG_MSG_COLORING   : 0;

            SiiPlatformEepromWriteByte( EE_FW_CONTROL, controlByte );
            SiiPlatformEepromWriteByte( EEPROM_EDID_LOADSEL, app.edidLoad );
            SiiPlatformEepromWriteByte( EE_DB_VERSION, EEPROM_DB_VERSION );
            SiiPlatformEepromWriteByte( EE_UART_MSG_LEVEL, app.dbgMsgLevel );
        }
        SiiPlatformEepromWriteByte( EE_CONFIG_VALID, CONFIG_VALID );
        updated = true;
    }
#endif 	

#if ( configSII_DEV_953x_PORTING == 0 )  /*Smith marks, we could not need to store BOOT data  in system EEPROM.*/
    if (( SiiPlatformEepromReadByte( EE_DEVBOOTDATA_VALID ) != CONFIG_VALID ) || forceInit )
    {
        SkAppEdidWrite( EDID_EEPROM_BOOT, (uint8_t *)&gEdidFlashDevBootData );
        updated = true;
    }
#endif	

#if ( configSII_DEV_953x_PORTING == 0 ) /*Smith marks, we could not need to store EDID in system EEPROM.*/
    if (( SiiPlatformEepromReadByte( EE_EDID_VALID ) != CONFIG_VALID ) || forceInit )
    {
        SkAppEdidWrite( EDID_EEPROM, gEdidFlashEdidTable );
        updated = true;
    }
#endif 
	
	/*
	if (( SiiPlatformEepromReadByte( EE_EDIDVGA_VALID ) != CONFIG_VALID ) || forceInit )
	{
		SkAppEdidWrite( EDID_EEPROM_VGA, gEdidFlashEdidVgaTable );
		updated = true;
	}
	*/
	
    return( updated );
}

//------------------------------------------------------------------------------
//! @brief  Update EEPROM control flags byte
//------------------------------------------------------------------------------
void SkAppEepromUpdateControl ( uint8_t mask, uint8_t value )
{
    uint8_t controlByte;

    controlByte = SiiPlatformEepromReadByte( EE_FW_CONTROL );
    SiiPlatformEepromWriteByte( EE_FW_CONTROL, (controlByte & mask) | value );

}

//------------------------------------------------------------------------------
//! @brief  Update EEPROM control flags byte
//! @note   Currently handles only EEFLAG_ROTARY_ENABLE flag
//------------------------------------------------------------------------------
bool_t SiiRtpiCbUpdateControl ( uint8_t mask, uint8_t value )
{
    uint8_t controlByte, oldControlByte;

    oldControlByte = SiiPlatformEepromReadByte( EE_FW_CONTROL );
    controlByte = oldControlByte;
    if ( mask & EEFLAG_ROTARY_ENABLE )
    {
        app.rotaryDisabled = (( value & EEFLAG_ROTARY_ENABLE ) == 0 );
        controlByte = (oldControlByte & ~EEFLAG_ROTARY_ENABLE) | ( value & EEFLAG_ROTARY_ENABLE );
    }
    if ( controlByte != oldControlByte )
    {
        SiiPlatformEepromWriteByte( EE_FW_CONTROL, (controlByte & mask) | value );
    }

    return( true );
}

#if INC_RTPI
//------------------------------------------------------------------------------
//! @brief  Support RRTPI register INPUT_NMB (0x24)
//------------------------------------------------------------------------------
uint8_t SiiRtpiCbUpdateSource( uint8_t functionSelect, uint8_t val)
{
    switch ( functionSelect )
    {
        case 0:                                     // Read current source value
            val = app.newSource[app.currentZone];
            break;
        case 1:                                     // Write source value
            app.newSource[app.currentZone] = val;
            break;
        case 2:                                     // Read immediate value of RSW (not debounced)
            val = SiiPlatformReadRotary( true );
            break;
    }

	return( val );
}
#endif // INC_RTPI

//------------------------------------------------------------------------------
// Function:    SkAppUiUpdate
// Description:
//------------------------------------------------------------------------------

void SkAppUiUpdate ( void )
{
    bool_t isEnabledBySwitch;

    app.rotarySwitch = SiiPlatformReadRotary(false);
#if INC_RTPI
    if (app.rotarySwitch != RSW_NO_CHANGE)
        SiiRtpiPort(app.rotarySwitch, 1);
#endif

#if ( configSII_DEV_953x_PORTING == 0 )
    isEnabledBySwitch = SiiPlatformSwitchGet(UI_FLASH_UPDATE);
    if (app.fwUpdEnabled != isEnabledBySwitch)
    {
        app.fwUpdEnabled = isEnabledBySwitch;
        DEBUG_PRINT(MSG_ALWAYS,"%s\n", app.fwUpdEnabled  ? "RESET MCU to enter F/W update mode." : "Canceled." );
    }
#endif 	

    isEnabledBySwitch = SiiPlatformSwitchGet(UI_AUTO_SWITCH);
    if (app.autoSwitch != isEnabledBySwitch)
    {   // Auto port switching enable change has detected
        app.autoSwitch = isEnabledBySwitch;
        DEBUG_PRINT(MSG_ALWAYS,"Rx Port Switching: %s\n", app.autoSwitch  ? "AUTO" : "MANUAL" );
		TRACE_DEBUG((0, "Rx Port Switching::0x%X ", app.autoSwitch ));
    }

#if ( configSII_DEV_953x_PORTING == 0 )
    isEnabledBySwitch = !SiiPlatformSwitchGet(UI_RC_ENABLE);
    if (app.irEnable != isEnabledBySwitch)
    {   // RC enable switch change has detected
        app.irEnable = isEnabledBySwitch;
        PrintStringOnOff( "IR Remote Control:", app.irEnable );
    }
#endif 

    isEnabledBySwitch = SiiPlatformSwitchGet(UI_MHL_ENABLE);
    if (app.isEdidMhlLimits != isEnabledBySwitch)
    {   // MHL switch change has detected
        app.isEdidMhlLimits = isEnabledBySwitch;
        // The change will be applied when EdidTx gets re-initialized
    }

    if(app.initialPowerOn == 0)
    {
		isEnabledBySwitch = !SiiPlatformSwitchGet(UI_REPEATER_ENABLE);
		// Repeater enable can also be toggled by firmware, so
		// only change the real repeater enable if the switch
		// position has changed.
		
		if ((app.wasRepEnabledBySwitch != isEnabledBySwitch))
		{
			app.wasRepEnabledBySwitch = isEnabledBySwitch;
			app.repeaterEnable = isEnabledBySwitch;
			SkAppRepeaterConfigure(app.repeaterEnable);
		}
		app.initialPowerOn = 1;
		DEBUG_PRINT(MSG_ALWAYS,"\n\r!!!!!  repeater Switch get:%x \n\r",isEnabledBySwitch);
		TRACE_DEBUG((0, "repeater Switch get:0x%X ", isEnabledBySwitch ));
    }

    // see if THX demo is enabled
    pApp->isThxDemo = SiiPlatformSwitchGet(UI_THX_ENABLE);

    // Several switch state changes need to call into the device
    // drivers, and as such should not be called when in standby.
    if ( app.powerState == APP_POWERSTATUS_ON )
    {
#if INC_CEC
        SkAppCecConfigure(!SiiPlatformSwitchGet(UI_CEC_ENABLE));
#endif
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppInstaPortLedUpdate
// Description:
//------------------------------------------------------------------------------

void SkAppInstaPortLedUpdate ( uint8_t portBits )
{
	SiiPlatformGpioLedPorts(portBits);
}

//------------------------------------------------------------------------------
// Function:    SkAppBlinkTilReset
// Description:
//-----------------------------------------------------------------------------c

void SkAppBlinkTilReset ( uint8_t errorLedPattern )
{
#if ( configSII_DEV_953x_PORTING == 0 )
    SiiPlatformGpioLedStatus1( true );      // Signal that port LEDs signify an error

    for ( ;; )
    {
        SkRemoteRequestHandler();
        SiiPlatformGpioLedPorts( errorLedPattern );
        SiiPlatformTimerWait( 300 );
        errorLedPattern ^= 0xFF;
        SkAppTaskIrRemote( true );
    }
#endif     

}

//------------------------------------------------------------------------------
// Function:    SkAppInitialize
// Description:
//-----------------------------------------------------------------------------c

void SkAppInitInstanceData ( void )
{
	int i;
    memset( &app, 0, sizeof( AppInstanceData_t ));

    app.powerState              = APP_POWERSTATUS_STANDBY;
    app.oldPowerState           = APP_POWERSTATUS_STANDBY;
#if INC_BENCH_TEST
    app.modeSelect              = SK_MODE_BTST;
#else
    app.modeSelect              = SK_MODE_TV;
#endif

    app.irEnable                = false;
	app.irUartKey				= 0xFF;
#if ( configSII_DEV_953x_PORTING == 1 )	
    app.cecEnable               = true;
#else
	app.cecEnable               = false;
#endif 
    app.cecPrint                = false;
    app.cecEnum                 = false;
    app.edidLoad                = 0;
    app.dbgMsgLevel             = MSG_PRINT_ALL;
    app.dbgMsgColoring          = false;

    app.arcDemoEnabled          = false;
    app.arcDemoSelect           = 0;

    for (i = 0 ; i < SII_NUM_TX; i++)
    {
    	app.isTxRepeaterMode[i]     = true;
        app.isTxBypassMode[i]       = true;
        app.isTxHdcpRequired[i]     = true;
    }


    app.v3dMode                 = SI_TX_VIDEO_3D_NONE;
    app.vSyncMode               = SI_TX_VIDEO_HVSYNC_DE;

#if INC_IV
    app.isExtTpg                = true;
    app.tpgVideoPattern         = SI_ETPG_PATTERN_COLORBAR;
    app.tpgVideoFormat          = SI_TPG_FMT_VID_480_60;
    app.tpgClockSrc             = SI_TPG_CLK_XCLK;
#endif

    for (i = 0 ; i < SII_NUM_TX; i++)
    {
		app.currentVideoSrc[i]      = SI_TX_VIDEO_SOURCE_HDMI;
	    app.currentVideoFormat[i]   = vm1_640x480p;
	    app.currentAudioType[i]      = SI_TX_AUDIO_SRC_SPDIF;
    }
    app.isEdidMhlLimits         = false;
    app.numOfEdidTxInst         = 1;
    app.rotarySwitch            = RSW_NO_CHANGE; // Smith implementes: HDMI port 2 is working when hdmi repeater task run.
    app.lastRotarySwitch        = RSW_NO_CHANGE;
    app.zoneEnabled[0]          = true;
    app.zoneEnabled[1]          = true;
    app.newZoneEnabled[0]       = true;
    app.newZoneEnabled[1]       = true;
    app.currentSource[0]        = 0xFF;
    app.currentSource[1]        = 0xFF;
    app.currentZone             = 0;
    app.audioZoneSource[0]      = SiiSwitchSource0;
    app.audioZoneSource[1]      = SiiSwitchSource1;
    app.newAudioSource[0]       = SiiSwitchAudioSourceDefault;
    app.newAudioSource[1]       = SiiSwitchAudioSourceDefault;
    app.currentAudioSource[0]   = SiiSwitchAudioSourceDefault;
    app.currentAudioSource[1]   = SiiSwitchAudioSourceDefault;
    app.audioSampleRate[0]      = SI_TX_AUDIO_SMP_RATE_48_KHZ;
    app.audioSampleRate[1]      = SI_TX_AUDIO_SMP_RATE_48_KHZ;
    app.isAudioExtractionEnabled[0]= true;
    app.isAudioExtractionEnabled[1]= true;
#if INC_IPV
    app.ipvAllowed              = true;
    app.ipvEnabled              = false;
    app.ipvPipEnabled           = false;
#endif
    app.isThxDemo				= false;
    app.msThxDelay				= 0;
    app.msThxStart				= 0;

    app.markerOptionFlagsMp     = ENABLE_3D_MARKER_SUPPORT;
    app.markerOptionFlagsRp     = ENABLE_3D_MARKER_SUPPORT;

    app.isCecAsSwitch = true;
    app.txTmdsFreqRange[0] = 3; //< 300MHz
    app.initialPowerOn = false;
    app.mhlcabledetect = false;
}
//------------------------------------------------------------------------------
//! @brief  Initialize the application layer
//-----------------------------------------------------------------------------c
bool_t SkAppInitialize ( void )
{
    uint8_t flags;

    SkAppInitInstanceData();
    SiiOsTimerSet( &app.appTimer, DEM_POLLING_DELAY );      // Reset poll timer

    if ( SkAppEepromInitialize( true, false ))
    {
        PrintAlways( "Settings EEPROM initialized to defaults.\n");
        TRACE_DEBUG((0, "Settings EEPROM initialized to defaults. "));
    }


#if ( configSII_DEV_953x_PORTING == 1 )
	app.autoSwitch      = SiiPlatformSwitchGet(UI_AUTO_SWITCH);
    app.newSource[app.currentZone] = SiiPlatformReadRotary( true ); // Read immediate value of RSW (not debounced)

#else
    // Load control flags from EEPROM.
    flags = SiiPlatformEepromReadByte( EE_FW_CONTROL );
    app.irEnable        = ((flags & EEFLAG_IR_ENABLE) == EEFLAG_IR_ENABLE );
    app.rotaryDisabled  = ((flags & EEFLAG_ROTARY_ENABLE) != EEFLAG_ROTARY_ENABLE );
    app.cecPrint        = ((flags & EEFLAG_CEC_PRINT) == EEFLAG_CEC_PRINT );
    app.cecEnum         = ((flags & EEFLAG_CEC_ENUM) == EEFLAG_CEC_ENUM );
    app.dbgMsgColoring  = ((flags & EEFLAG_DBG_MSG_COLORING) == EEFLAG_DBG_MSG_COLORING );
    app.edidLoad        = SiiPlatformEepromReadByte( EEPROM_EDID_LOADSEL );
    app.dbgMsgLevel     = SiiPlatformEepromReadByte( EE_UART_MSG_LEVEL );

 // CEC switch closed (CEC OFF) setting overrides an EEPROM CEC ON setting.
    app.cecEnable       = !SiiPlatformSwitchGet(UI_CEC_ENABLE);
#endif

#if ( configSII_DEV_953x_PORTING == 0 )
    SiiPlatformLoggerColorEnable( app.dbgMsgColoring );
    SiiPlatformLoggerMessageLevelSet( app.dbgMsgLevel );
#endif     

#if ( configSII_DEV_953x_PORTING == 0 )
    app.autoSwitch      = SiiPlatformSwitchGet(UI_AUTO_SWITCH);
    app.newSource[app.currentZone] = SiiPlatformReadRotary( true ); // Read immediate value of RSW (not debounced)

	// PrintStringYesNo( "\nCEC Enabled:  ", app.cecEnable );
	PrintStringYesNo( "IR Enabled:   ", app.irEnable );
	PrintAlways( "\n" );
#endif

    return( true );
}
