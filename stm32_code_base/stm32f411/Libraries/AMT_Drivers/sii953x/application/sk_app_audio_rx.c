//***************************************************************************
//!file     sk_app_audio_rx.c
//!brief    Wraps platform and device functions for the AUDIO_RX component
//!         and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "sk_application.h"

#include "si_audio_component.h"

#if defined ( SII_DEV_953x_PORTING_PLATFORM_DAC )
#include "si_audio_dac.h"
#endif

#include "si_drv_gpio.h"

#if defined ( SII_DEV_953x_PORTING_PLATFORM_PLL )
#include "si_idt_5v9885t.h"
#endif 

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
#include "si_gpio.h"
#endif 

#if INC_CEC_SAC
#include "si_sac_main.h"
#endif
#if INC_CEC_SWITCH
#include "si_cec_switch_api.h"
#endif

#include "cea931_keys.h"

#if INC_RTPI
#include "si_rtpi_component.h"
//#include "si_i2c.h"
#endif

SiiRxAudioInstanceData_t RxAudStatus;
SiiRxAudioInstanceData_t RxAudState[2];


enum
{
	SII_MATRIX_MUX_DISABLE,
	SII_MATRIX_MUX_ENABLE,
};


//------------------------------------------------------------------------------
// Function:    SkAppRxAudioConfigLayout
// Description:
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
static void RxAudioConfigLayout(uint8_t audLayout)
{
	
    if (audLayout == SII_AUD_MULTI_CHANNEL)
    {
		SiiDrvGpioPinType(SII_GPIO_PIN_1, SII_GPIO_ALT_SD1);
		SiiDrvGpioPinType(SII_GPIO_PIN_2, SII_GPIO_ALT_SD2);
		SiiDrvGpioPinType(SII_GPIO_PIN_3, SII_GPIO_ALT_SD3);
    }
    else
    {
    	SiiDrvGpioPinType(SII_GPIO_PIN_1, SII_GPIO_STANDARD);
    	SiiDrvGpioPinType(SII_GPIO_PIN_2, SII_GPIO_STANDARD);
    	SiiDrvGpioPinType(SII_GPIO_PIN_3, SII_GPIO_STANDARD);
    }

}

//------------------------------------------------------------------------------
// Function:    SkAppRxAudioMuteSet
// Description:
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
static void RxAudioMuteSet(uint8_t chn, bool_t mute )
{
    if (mute) //mute
    {
#if defined ( SII_DEV_953x_PORTING_PLATFORM_DAC )    
        SiiPlatformAudioDacControl(chn, SII_AUD_POWER_DOWN);
#endif         
        RxAudioConfigLayout(SII_AUD_TWO_CHANNEL);
    }

    // If SAC system is unmuted and we're not just passing through the audio,
    // then allow the unmute to occur
    else if ( !app.isAudioByPass
#if INC_CEC_SAC
    		&& !SiiCecSacIsMuted()
#endif
    		)
    {
		switch (RxAudState[chn].audMode)
       	{
       		case SII_AUD_HBRA_MODE:
#if defined ( SII_DEV_953x_PORTING_PLATFORM_DAC )    
       	     	//DEBUG_PRINT( MSG_DBG, "HBRA\n");
     	     	SiiPlatformAudioDacControl(chn, SII_AUD_POWER_DOWN);
#endif      	     	
     	     	RxAudioConfigLayout(SII_AUD_MULTI_CHANNEL);
      	     	break;
      	  	case SII_AUD_DSD_MODE:
#if defined ( SII_DEV_953x_PORTING_PLATFORM_DAC )    
       	     	//DEBUG_PRINT( MSG_DBG, "DSD\n");
      	  		SiiPlatformAudioDacControl(chn, SII_AUD_DSD_MODE);
#endif       	  		
      	  		RxAudioConfigLayout(SII_AUD_MULTI_CHANNEL);
     	  		break;
      	  	case SII_AUD_PCM_MODE:
       	     	//DEBUG_PRINT( MSG_DBG, "PCM\n");

       	     	//Program the external PLL for generate the MCLK.
       	     	if (chn == SII_AUD_RX_SUB)
       	     	{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_PLL )       	     	
       	     		//DEBUG_PRINT( MSG_ALWAYS, "Generate the MCLK for subpipe: %d\n", RxAudState[chn].audFs);
       	     		SiiPlatformExtnalAudioPllSetting( RxAudState[chn].audMclk, RxAudState[chn].audFs);
#endif        	     		
       	     	}

#if defined ( SII_DEV_953x_PORTING_PLATFORM_DAC )    
				SiiPlatformAudioDacControl(chn, SII_AUD_PCM_MODE);
#endif 				
				if (RxAudState[chn].audLayout)
				{
					//DEBUG_PRINT( MSG_DBG, "SII_AUD_MULTI_CHANNEL\n");
				    RxAudioConfigLayout(SII_AUD_MULTI_CHANNEL);
				}
				else
				{
					//DEBUG_PRINT( MSG_DBG, "SII_AUD_TWO_CHANNEL\n");
				    RxAudioConfigLayout(SII_AUD_TWO_CHANNEL);
				}
      	  		break;
      	  	default:
      	  	    break;
        }
#if INC_RTPI
	{
		uint8_t val = 0;

    	val |= RxAudState[chn].audLayout & RP_M__RX_AUDIO_FORMAT__LAYOUT1;
    	switch (RxAudState[chn].audMode)
		{
			case SII_AUD_HBRA_MODE:
				val |= RP_M__RX_AUDIO_FORMAT__HBR;
				break;
			case SII_AUD_DSD_MODE:
				val |= RP_M__RX_AUDIO_FORMAT__DSD;
				break;
			default:
				break;
		}
    	/*these two must be there for audio to be ready to unmute*/
    	val |= RP_M__RX_AUDIO_FORMAT__CHST_RDY | RP_M__RX_AUDIO_FORMAT__AUD_IF_RDY;

    	SiiRtpiOnAudioStatusChange(val);
    	SiiRtpiOnAudioInfoFrameChange();

	}
#endif
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppDeviceInitAudio
// Description:  Init the audio extraction for main channel only
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

bool_t SkAppDeviceInitAudio ( void )
{
	uint8_t chn;

	//Enable audio extraction 
	SiiPlatformGpioAudioInOut(true);
    SiiDrvGpioPinType(SII_GPIO_PIN_7, SII_GPIO_ALT_I2S_WS0_OUT);

	RxAudioConfigLayout(SII_AUD_TWO_CHANNEL);
    for ( chn = 0; chn < SII_NUM_RX_AUDIO; chn++ )
    {    	  
        SiiRxAudioInstanceSet(chn);
		SiiRxAudioInit();
		// Enable SPDIF in MP and disable it in SP (in favor of I2S stereo output)
		SiiDrvRxSpdifOutEnable(chn == 0);
    }

    //Only enable main pipe int mask
   	SiiDrvRxAudioInstanceSet(SII_AUD_RX_MAIN);
    SiiDrvRxAudioIntMaskEnable(ENABLE);
	return true;
}

//------------------------------------------------------------------------------
#if (INC_CEC_SAC == ENABLE)
//-------------------------------------------------------------------------------------------------
//! @brief      CEC Callback processing messages from CEC/SAC sub-system.
//!
//! @param[in]  fbMsg - message code
//!
//! @retval     true  - if the message was recognized and processed
//! @retval     false - if the message is irrelevant and wasn't processed
//-------------------------------------------------------------------------------------------------
bool_t SiiCbCecSacMessageHandler(uint8_t fbMsg)
{
    bool_t   usedMessage = true;

    /* Process feedback messages.       */

    switch (fbMsg)
    {
        default:
        case CEC_SAC_FB_MSG_NONE:
            // nothing to do
            usedMessage = false;
            break;

        case CEC_SAC_FB_MSG_SAC_INITIATE_DONE:
            DEBUG_PRINT(MSG_STAT, "SAC Initiation task finished\n");
            break;

        case CEC_SAC_FB_MSG_SAC_FMT_DISC_DONE:
            DEBUG_PRINT(MSG_STAT, "SAC Audio Format Discovery task finished\n");
            break;

        case CEC_SAC_FB_MSG_MUTE:
        case CEC_SAC_FB_MSG_UNMUTE:
        case CEC_SAC_FB_MSG_SAC_STATUS_CHANGED:
            // For us, this means that audio is either muted or unmuted
            RxAudioMuteSet( 0, SiiCecSacIsMuted());         // Unmute speakers
            break;

        case CEC_SAC_FB_MSG_SAC_VOLUME_CHANGED:
            DEBUG_PRINT(MSG_STAT, "SAC volume has changed\n");
#if INC_OSD
            SkAppOsdVolumeUpdate( SiiCecSacVolumeGet() );
#endif
            break;

        case CEC_SAC_FB_MSG_ERR_LA_NOT_ASSIGNED:
            DEBUG_PRINT(MSG_STAT, "Audio System Logical Address not assigned\n");
            SiiCecSacSrvStop(); // Stop SAC operation
            break;

        case CEC_SAC_FB_MSG_WAKEUP_UNMUTE:
            // Come out of standby mode
            DEBUG_PRINT(MSG_STAT, "Standby mode Off Request\n");
			
#if ( configSII_DEV_953x_PORTING == 0 )
            SiiCbCecStandby(false);
#endif

            RxAudioMuteSet( 0, false );      // Unmute speakers
            break;

        case CEC_SAC_FB_MSG_ERR_NONSAC_CMD:
            usedMessage = false;
            break;

    }

    return( usedMessage );
}
#endif

#if INC_CEC_SAC
//------------------------------------------------------------------------------
//! @brief      System Audio Control command processing
//! @param[in]  cmdData - CEC SAC key code
//! @retval     true  - if the cmd was recognized and processed
//! @retval     false - if the cmd wasn't processed
//------------------------------------------------------------------------------
bool_t  SkAppAudioProcessSacCommand ( uint8_t cmdData )
{
    uint8_t sacMsg      = CEC_SAC_FB_MSG_NONE;

    // If System Audio Mode is enabled, process volume controls locally
    if ( SiiCecSacIsSysAudioModeEnabled())
    {
        switch ( cmdData )
        {
            case CEA931_KEY_MUTE:
            case CEA931_KEY_MUTE_FUNC:
                PrintAlways( "CBUS MUTE received\n" );
                SiiCecSacMute( CEC_SAC_MUTE_ON );
                sacMsg = CEC_SAC_FB_MSG_MUTE;
                break;
            case CEA931_KEY_UN_MUTE_FUNC:
                PrintAlways( "CBUS UN-MUTE received\n" );
                SiiCecSacMute( CEC_SAC_MUTE_OFF );
                sacMsg = CEC_SAC_FB_MSG_UNMUTE;
                break;
            case CEA931_KEY_VOL_UP:
                PrintAlways( "CBUS VOL UP received\n" );
                SiiCecSacVolumeStep( true );
                sacMsg = CEC_SAC_FB_MSG_SAC_VOLUME_CHANGED;
                break;
            case CEA931_KEY_VOL_DOWN:
                PrintAlways( "CBUS VOL DOWN received\n" );
                SiiCecSacVolumeStep( false );
                sacMsg = CEC_SAC_FB_MSG_SAC_VOLUME_CHANGED;
                break;
            default:
                break;
        }
    }

    // If message was consumed, tell our local System Audio Controller about it
    return( SiiCbCecSacMessageHandler( sacMsg ));
}
#endif
//------------------------------------------------------------------------------
// Function:    SkAppTaskAudio
// Description:
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskAudio( void )
{
	uint8_t chn;

	//CheckStreamMuxEnable();     // Determine if one or two instances are enabled.

	for( chn = 0; chn < 1 ; chn++)
	{
	    SiiRxAudioInstanceSet(chn);
		if (SiiRxAudioHandler(&RxAudStatus))
		{
			if (memcmp(&RxAudState[chn], &RxAudStatus, sizeof(SiiRxAudioInstanceData_t)))
			{
				memcpy(&RxAudState[chn], &RxAudStatus, sizeof(SiiRxAudioInstanceData_t));

				switch (RxAudStatus.audState)
				{
					case SII_AUD_STATE_OFF:
					    RxAudioMuteSet( chn, ON );  // Configure audio layout as 2ch and turn off audio hardware
						break;
					case SII_AUD_STATE_ON:
					    RxAudioMuteSet( chn, OFF ); // Configure audio layout and hardware as appropriate for mode.
						break;
					default:
					    break;
				}

			    if (chn == 1) // sub pipe
			    {
			        SiiDrvRxSpdifOutEnable(app.isSubPipeAudioExtSpdif);
                }
			}
		}
		if ( RxAudState[chn].audState == SII_AUD_NO_ACP_CHECK )
		{
			SiiRxAudioNoAcpCheck();
		}
	}
}

#if INC_RTPI
//------------------------------------------------------------------------------
// Function:    RtpiCbAudioByPassSet
// Description:
// Parameters:  isByPass
//              -true: audio by pass repeater,
//              -false:audio played by repeater
// Returns:     none
//------------------------------------------------------------------------------
void SiiRtpiCbAudioByPassSet(bool_t isByPass )
{
	app.isAudioByPass = isByPass;
}
#endif //#if INC_RTPI

//_______________________________AMTRAN IMPLEMENTED_____________________________________________
#if ( configSII_DEV_953x_PORTING == 1 )
bool AmTAudiogetMultiChannel (void)
{
    if (RxAudState[0].audLayout == SII_AUD_MULTI_CHANNEL)
    {
        return TRUE;
    }
    return FALSE;
}
#endif
