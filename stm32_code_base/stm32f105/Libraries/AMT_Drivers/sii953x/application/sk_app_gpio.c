//***************************************************************************
//!file     sk_app_gpio.c
//!brief
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "si_device_config.h"

#if INC_CBUS
#include "sk_application.h"
#include "sk_app_cbus.h"
#include "si_drv_gpio.h"
#include "si_gpio.h"
#include "si_regs_gpio953x.h"
#include "si_drv_internal.h"
//------------------------------------------------------------------------------
// Function:    SkAppDeviceInitGpio
// Description: Perform any board-level initialization required at the same
//              time as GPIO component initialization
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

bool_t SkAppDeviceInitGpio ( void )
{
	return( SiiDrvGpioInitialize() );
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskGpio
// Description: Wrapper for the GPIO Component at the application level
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskGpio ( uint8_t instance )
{
    uint8_t     pinState[2];
    uint16_t    status;
    
    status = SiiDrvGpioStatus();    // Determine who triggered the interrupt
    if ( (status & SII_GPIO_PIN_0) || (instance == 1) )
    {
		/*If it is marked, USB playback is able to working*/	
        pinState[0] = SiiDrvGpioPinRead( SII_GPIO_PIN_0 );
        pinState[1] = (SiiPlatformMHLCDSensePinGet( 1 )? SiiPortType_HDMI : SiiPortType_MHL);
        //pinState[1] = SiiDrvGpioPinRead( SII_GPIO_PIN_1 );
        if ( pinState[instance] )
		{
			//DEBUG_PRINT( MSG_DBG, "SkAppTaskGpio:: MHL channel 0 cable connect %s.\n", ( status & SII_GPIO_PIN_0 ) ? "true" : "false" );
			pAppCbus->cbusEnable = true;   //Bug 31744 - With samsung S3 and S4 as MHL source, video out is not consistent when plug/unplug cable at source side

		}
		else
		{
			//DEBUG_PRINT( MSG_DBG, "SkAppTaskGpio:: MHL channel 1 cable connect %s.\n", ( status & SII_GPIO_PIN_1 ) ? "true" : "false" );
			pAppCbus->cbusEnable = false;   //Bug 31744 - With samsung S3 and S4 as MHL source, video out is not consistent when plug/unplug cable at source side
		}
	}
}
//------------------------------------------------------------------------------
// Function:    SkAppTaskCbusGpio
// Description: Wrapper for the GPIO Component at the application level
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
void SkAppTaskCbusGpio (uint8_t instance )
{
    uint8_t     pinState[2];
    //uint16_t    status;

   	//status = SiiDrvGpioStatus();    // Determine who triggered the interrupt
	if ((instance == 1) )
	{
        pinState[1] = (SiiPlatformMHLCDSensePinGet( 1 )? SiiPortType_HDMI : SiiPortType_MHL);
	}
	else
	{
        pinState[0] = ((SiiRegRead( REG_GPIO_AON_CTRL1 ) & MSK_READ_0) >> 2) & SII_GPIO_PIN_0;
        //DEBUG_PRINT(MSG_DBG,"\n\r %x %x\n\r",pinState[0],((SiiRegRead( REG_GPIO_AON_CTRL1 ) & MSK_READ_0) >> 2));
	}
	
    if ( pinState[instance] )
	{
		pAppCbus->cdSense = true;
	}
    else
	{
		pAppCbus->cdSense = false;
	}
}


#else
char appGpio;   // Avoids compiler warning of empty translation unit
#endif          // #if INC_CBUS == 1
