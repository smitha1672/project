//***************************************************************************
//!file     si_audio_dac.c
//!brief    Platform GPIO driver, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"
#include "inc/hw_gpio.h"
#undef GPIO_LOCK_KEY

#include "si_common.h"
#include "si_platform.h"
#include "si_i2c.h"
#include "si_audio_dac.h"

#include "si_arm.h"
#include "si_gpio.h"
#include "si_drv_audio.h"


//------------------------------------------------------------------------------
// Function:    AudDacI2cWriteByte
// Description: 
// Parameters:
// Returns:     None
// Note:        
//------------------------------------------------------------------------------

static void AudDacI2cWriteByte (uint8_t chn, uint8_t regAddr, uint8_t wrData )
{
    uint8_t devId = chn ? SI_ADAC_SLAVE_ADDR_SP : SI_ADAC_SLAVE_ADDR_MP;
    SiiPlatformI2cWriteByte(0, devId, regAddr, wrData);
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformAudioDacMute 
// Description: Mute or Unmute the audio
// Parameters:  
// Returns:
//------------------------------------------------------------------------------

void SiiPlatformAudioDacMute(uint8_t chn, uint8_t ctrl)
{
	if (chn == 0)
	{
		ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, ctrl ? GPIO_PIN_0 : 0);
	}
	else
	{
		ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, ctrl ? GPIO_PIN_1 : 0);
	}
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformAudioDacPower 
// Description: Put the audio DAC in normal operation or reset mode
// Parameters:  
// Returns:
//------------------------------------------------------------------------------

void SiiPlatformAudioDacPower( uint8_t chn, uint8_t ctrl )
{
	SiiPlatformAudioDacMute(chn, ctrl ? 1 : 0);
	AudDacI2cWriteByte(chn, SI_ADAC_SPEED_PD_ADDR, ctrl ? SI_ADAC_NORMAL_OPERATION : SI_ADAC_POWER_DOWN);
}


//------------------------------------------------------------------------------
// Function:    SiiPlatformAudioDacControl
// Description: control the Audio DAC
// Parameters:  
//------------------------------------------------------------------------------

void SiiPlatformAudioDacControl(uint8_t chn,  uint8_t audType)
{

	switch (audType)
	{
		//Power down the Audio Dac if input is HBR
		case SII_AUD_POWER_DOWN:
			SiiPlatformAudioDacPower(chn, OFF);
			break;

    	case  SII_AUD_DSD_MODE:
	    	//Set Audio Dac to DSD
	    	//DEBUG_PRINT( MSG_DBG, "*****DSD  %d %d", chn, audType);

		    SiiPlatformAudioDacPower(chn, ON);
			AudDacI2cWriteByte(chn, SI_ADAC_CTRL3_ADDR, SI_ADAC_DSD_MODE);
	    	break;
	
    	case SII_AUD_PCM_MODE:
	    	//Set Audio Dac to LPCM
	    	//DEBUG_PRINT( MSG_DBG, "*****PCM: %d %d", chn, audType);

	    	SiiPlatformAudioDacPower(chn, ON);
	        AudDacI2cWriteByte(chn, SI_ADAC_CTRL3_ADDR, SI_ADAC_PCM_MODE);
			break;

		default:
			break;
    }
 }

