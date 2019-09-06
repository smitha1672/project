//***************************************************************************
//!file     si_audio_component.c
//!brief    Silicon Image AUDIO Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_audio_component.h"
#include "si_debug.h"
#include "si_drv_audio_internal.h"
#include "si_timer.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#endif

SiiRxAudioInstanceData_t RxAudInstance[SII_NUM_RX_AUDIO];
SiiRxAudioInstanceData_t *pAudExtraction = &RxAudInstance[0];


//-------------------------------------------------------------------------------------------------
//! @brief      Set the component global data pointer to the requested instance.
//! @param[in]  instanceIndex   Instance of RX Audio component.
//! @returns    true if legal index, false if index value is illegal.
//-------------------------------------------------------------------------------------------------
bool_t SiiRxAudioInstanceSet( int instanceIndex )
{
    if ( instanceIndex < SII_NUM_RX_AUDIO )
    {
        SiiDrvRxAudioInstanceSet( instanceIndex );
        pAudExtraction = &RxAudInstance[ instanceIndex ];
        return( true );
    }

    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiRxAudioInit
// Description:  Initialize the audio state machine and audio hardware start up
//              for both main and sub channel
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
void SiiRxAudioInit( void )
{
    pAudExtraction->audState    = SII_AUD_STATE_OFF;
    pAudExtraction->audMode     = SII_AUD_PCM_MODE;
    pAudExtraction->audLayout   = SII_AUD_TWO_CHANNEL;

    SiiDrvRxAudioInit();
    SiiDrvRxSpdifOutEnable(false); // SPDIF digital outputs are disabled by default
}

//------------------------------------------------------------------------------
//! @brief  Place audio component in standby
//------------------------------------------------------------------------------
bool_t SiiRxAudioStandby( void )
{
    int i;

    for ( i = 0; i < SII_NUM_RX_AUDIO; i++ )
    {
        SiiRxAudioInstanceSet( i );
        SiiDrvRxSpdifOutEnable( false );
    }

    return( SiiDrvRxAudioStandby() );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @return
//-------------------------------------------------------------------------------------------------
uint8_t SiiRxAudioHandler(SiiRxAudioInstanceData_t *pAudRx)
{
    uint8_t IntrStatus[3];
	uint8_t intTrigger=false;

	if (SiiDrvRxAudioIsInterruptAssert())
	{
	    //read all interrupt registers
	    SiiDrvRxAudioGetIntStatus(IntrStatus);
	    //SiiDrvRxAudioUnmuteReady(false);
	    if (IntrStatus[1] & SII_AUD_RX_AACDONE)    //if soft mute done
	    {
			//DEBUG_PRINT( MSG_ALWAYS, "=== AAC_DONE ===\n");
	        SiiDrvRxAudioInternalMute(true);
	        if( (SiiDrvRxAudioModeGet()== SII_AUD_DSD_MODE) ||(SiiDrvRxAudioHdmiModeGet()))  //Bug 31751 - Audio extraction is disabled and not resumed after receiving glitch in the audio
	        {
	        	DEBUG_PRINT( MSG_ALWAYS, "AAC_DONE: DSD mode, enable AAC_ALL\n");
	        	SiiDrvRxAudioEnableAAC();
	        }
            if( pAudExtraction->audState != SII_AUD_STATE_OFF ){ // from Audio ON condition.
		        pAudExtraction->audState = SII_AUD_STATE_OFF;
                //SiiDrvRxAudioEnableCHSTINT();
				SiiDrvRxAudioInitMclk();

			}
	    }
	    else 
	    {
			//if( IntrStatus[2] & SII_AUD_RX_FSCHG){
	    	if((pAudExtraction->audState == SII_AUD_STATE_OFF)&&
	    			((IntrStatus[1] & SII_AUD_RX_FNCHG) ||
	    			(IntrStatus[2] & SII_AUD_RX_FSCHG))
	    			)
	    	{
			//	DEBUG_PRINT( MSG_ALWAYS, "=== FNCHG ===\n");
	    		//if audFs is unknown, don't clear FNCHG, then check again
				pAudExtraction->audMode = (SiiRxAudioModes_t)SiiDrvRxAudioModeGet();
				pAudExtraction->audFs = SiiDrvRxAudioCheckFsAndFix();
				if ( (pAudExtraction->audMode == SII_AUD_DSD_MODE) ||
					( ((pAudExtraction->audMode == SII_AUD_HBRA_MODE) || (pAudExtraction->audMode == SII_AUD_PCM_MODE) ) &&
					    (pAudExtraction->audFs != (SiiRxAudioFsValues_t)SII_AUD_CHST4_FS_UNKNOWN)	) )

				{
					SiiDrvRxAudioSetMclk( pAudExtraction->audMode, pAudExtraction->audFs );
					SiiDrvRxAudioEnableAAC();
				}

			}
			if (IntrStatus[0] & SII_AUD_RX_AUDIO_READY)    //audio ready
			{
				DEBUG_PRINT( MSG_ALWAYS, "=== UNMUTE ===\n");
				//clear all pending interrupts
				IntrStatus[0] = SII_AUD_RX_AUDIO_READY;
				IntrStatus[1] = 0;
				IntrStatus[2] = 0;
				SiiDrvRxAudioClearIntStatus(IntrStatus);
				pAudExtraction->audState = SII_AUD_STATE_ON;
				SiiDrvRxAudioUnmuteReady(true); //for ACP chk
				//SiiDrvRxAudioDisableCHSTINT();
				//SiiDrvRxAudioDisableAAC();  //Bug 31751 - Audio extraction is disabled and not resumed after receiving glitch in the audio

			}
	    }

        //check ACP packet
	    if (SiiDrvRxAudioGetNewAcpInt())    //new ACP change
	    {
	    	//DEBUG_PRINT( MSG_ALWAYS, "\n\n\nAudio:New Acp\n");
	    	SiiDrvRxAudioOnNewAcp(true);
	    	//if it is in No ACP check already, exit it
	    	if(pAudExtraction->audState == SII_AUD_NO_ACP_CHECK)
	    	{
	    		SiiDrvRxAudioNoAcpPacketcheckStart(OFF);
	    		pAudExtraction->audState = SII_AUD_STATE_IDLE;
	    	}

	    }
	    // If audio is ready, and we were in ACP type 2 or above but
	    // have not received an ACP packet, start the 600ms to wait for one.
	    else if (SiiDrvRxAudioGetNoAcpInt())    //no ACP change
	    {
	    	pAudExtraction->audState = SII_AUD_NO_ACP_CHECK;
	    	SiiTimerSet( TIMER_4, SII_NOACP_TIMEOUT ); //start the timer
	    	SiiDrvRxAudioNoAcpPacketcheckStart(ON);
	    }

		pAudExtraction->audMode = (SiiRxAudioModes_t)SiiDrvRxAudioModeGet();
		pAudExtraction->audLayout = (SiiRxAudioLayouts_t)SiiDrvRxAudioLayoutGet();
		pAudExtraction->audFs = (SiiRxAudioFsValues_t)SiiDrvRxAudioSampleRateGet();
		pAudExtraction->audMclk = (SiiRxAudioMclk_t)SiiDrvRxAudioGetMclk();
		memcpy(pAudRx, pAudExtraction, sizeof(SiiRxAudioInstanceData_t));
		
		IntrStatus[0] = 0;
		IntrStatus[1] = SII_AUD_RX_AACDONE;
		IntrStatus[2] = 0;
	    //clear all pending interrupts
	    SiiDrvRxAudioClearIntStatus(IntrStatus);

		SiiDrvRxAudioInterruptClearAssert();
		intTrigger = true;
	}

	return intTrigger;
}


void SiiRxAudioNoAcpCheck(void)
{
	if ( SiiTimerExpired( TIMER_4 ))
	{
		pAudExtraction->audState = SII_AUD_STATE_ON;
		//time out,
		if(SiiDrvRxAudioNoAcpPacketcheckConfirm())
		{
			//revert to no ACP action
			DEBUG_PRINT( MSG_DBG, "No Acp Packet\n");

			SiiDrvRxAudioOnNoAcp();
		}
		else
		{
			SiiDrvRxAudioOnNewAcp(ON);
		}

		SiiDrvRxAudioNoAcpPacketcheckStart(OFF);
	}
}

