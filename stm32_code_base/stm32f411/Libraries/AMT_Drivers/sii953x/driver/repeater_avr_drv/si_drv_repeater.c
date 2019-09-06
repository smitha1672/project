//***************************************************************************
//!file     si_drv_repeater.c
//!brief    Silicon Image Repeater Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_drv_repeater.h"
#include "si_drv_internal.h"
#include "si_gpio.h"
#include "si_regs_tpi953x.h"



//#define NONE_REPT_WITH_TX_ON


uint8_t mainPort = 0x00; //as default
uint8_t inputPort[SII_NUM_PIPE];
uint8_t	pipeNumber;
RepeaterTopoMode_t currentTopoMode;
bool_t  intStatus[SII_NUM_PIPE];
bool_t  resChngStatus[SII_NUM_PIPE];
bool_t  inputPortToChange;

// SWWA_FP1554
#if INC_OSD
#include "si_drv_osd_internal.h"
#endif
// SWWA_FP1554

//-------------------------------------------------------------------------------------------------
//! @brief      get the corresponding register name for the pipe
//!
//-------------------------------------------------------------------------------------------------

static uint16_t GetRegName(uint16_t regName)
{
	uint16_t resultName = 0xFFFF;

	switch(regName)
	{
		case REG_MP_STATE:
				resultName = (pipeNumber == 0) ? REG_MP_STATE : REG_RP_STATE;
				break;
		case REG_MP_GCP_STATUS:
				resultName = (pipeNumber == 0) ? REG_MP_GCP_STATUS : REG_RP_GCP_STATUS;
				break;

	}
	return resultName;
}

//-------------------------------------------------------------------------------------------------
//! @brief      init repeater driver
//!
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterInit()
{
	uint8_t i;
	for (i=0;i<SII_NUM_PIPE;i++)
	{
		inputPort[i]     = SiiPORT_0;  //legal when in Instrp mode.
		intStatus[i]     = false;
		resChngStatus[i] = false;
	}
	pipeNumber = 0;
	currentTopoMode = SI_RPT_MODE_INITIAL;
	inputPortToChange = false;
	SiiRegWrite(REG_HDCP_KSV_FIFO_CTRL, VAL_HDCP_FIFO_VALID);
}

//-------------------------------------------------------------------------------------------------
//! @brief      set repeater mode
//!
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterSetMode(RepeaterTopoMode_t cmode)
{
	if( currentTopoMode != cmode )
	{
		//to clear all the rep bits, application will set it on later with active Tx.
		currentTopoMode =  SI_RPT_MODE_SINGLE_REPEATER;
		SiiDrvRepeaterSetBcapsRepeaterBits(OFF);
		currentTopoMode = cmode;
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      set instance 
//!
//! @param[in]  selectedInstance - current instance
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterInstanceSet(uint8_t setP)
{
	pipeNumber = setP;
}

//-------------------------------------------------------------------------------------------------
//! @brief      if the rx SCDT is on for the inputs list, or the (main/sub)channel 
//! @param[in]  none
//! @return     true: ON
//!				false: OFF
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterRxSyncIsOn( void )
{
	return (SiiRegRead( GetRegName( REG_MP_STATE)) & BIT_SCDT);
}

//-------------------------------------------------------------------------------------------------
//! @brief      if the rx main port SCDT interrupt happened
//! @param[in]  none
//! @return     true: interrupt happened
//!				false: interrupt not happened
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterRxSyncIntr( void )
{
	uint8_t intSt = 0;

	intSt = SiiRegRead( REG_INT_STATUS_2 ) & ((pipeNumber == 0) ? BIT_SCDT_CHG : BIT_RP_SCDT_CHG);
	
	if (intSt)
	{
		SiiRegWrite( REG_INT_STATUS_2, (pipeNumber == 0) ? BIT_SCDT_CHG : BIT_RP_SCDT_CHG);
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
//! @brief      if the rx main port is authenticated
//! @param[in]  none
//! @return     true: authenticated
//!				false:not authenticated
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterMainPortAuthed( void )
{
	bool_t result = false;

	result = (SiiRegRead(GetRegName(REG_MP_STATE)) & BIT_DECRYPT_STATUS);

   	return result;
}


//-------------------------------------------------------------------------------------------------
//! @brief      toggle the Rx Hot Plug High or Low
//! @param[in]  true: High
//!				false: Low
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterSwitchRxHotPlug(bool_t switch_on)
{
// SWWA_FP1554
#if INC_OSD
    {
        if ( !switch_on )
        {
            // Turn off OSD_EN so that the loss of TCLK when the RX is shut off will not
            // affect OSDRAM transfer operations.
          //  DEBUG_PRINT( MSG_DBG, "\nSWWA_FP1554: OSD_EN = 0 (RPTR_RX_OFF)\n");
            DrvOsdDisableInternal();
        }
    }
#endif
// SWWA_FP1554

// Bug 33051 - [CP9533_FW] MHL1/2:Video color is incorrect after hotplug DS HDMI cable of 9533

//	if (currentTopoMode == SI_RPT_MODE_SINGLE_REPEATER)
//	{

		if ( switch_on )
		{
		    SiiDrvRxHpeStateSet( SiiPORT_ALL, SiiHPD_ACTIVE );
		}
		else
		{
		    SiiDrvRxHpeStateSet( SiiPORT_ALL, SiiHPD_INACTIVE );
		}

    /*}
	else if (currentTopoMode == SI_RPT_MODE_MULTIPLE_REPEATER)
	{
		if( switch_on )
		{
		    SiiDrvRxHpeStateSet( inputPort[pipeNumber], SiiHPD_ACTIVE );
		}
		else
		{
		    SiiDrvRxHpeStateSet( inputPort[pipeNumber], SiiHPD_INACTIVE );
		}
	}*/

    DEBUG_PRINT( MSG_DBG, "RX: HPD %s\n", switch_on ? "ON" : "OFF");
	TRACE_DEBUG((0, "[ HDMI MSG] RX: HPD %s", switch_on ? "ON" : "OFF" ));
/*   if(switch_on)
   {
//
//     SiiRegModify ( REG_SYS_RESET_1, BIT0, SET_BITS);
//     SiiRegModify ( REG_SYS_RESET_1, BIT0, CLEAR_BITS);
//     DEBUG_PRINT( MSG_DBG, "\n\n RX--SOFTRESET IS PERFORMED\n\n");
     SiiRegModify ( REG_TPI__RAR, BIT0, SET_BITS);
     SiiPlatformTimerWait( 500 );
     SiiRegModify ( REG_TPI__RAR, BIT0, CLEAR_BITS);
//     SiiDrvTpiIndirectWrite(REG_TX__TMDS_ZONE_CTRL2, 0x09);
////   SiiTxInitialize();
      DEBUG_PRINT( MSG_DBG, "\n\nTX--SOFTRESET IS PERFORMED\n\n");
//	    SiiDrvRxTermControl(0,false);
//	    SiiPlatformTimerWait( 500 );
//	    SiiDrvRxTermControl(0,true);
   }*/
}

//-------------------------------------------------------------------------------------------------
//! @brief      clear the bstatus of Rx
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterRxHdcpReset(void)
{
	uint8_t clearStatus[] = {0,0};
    // inform US side about the failure
	SiiDrvRepeaterSetUsBstatus(clearStatus);
}

//-------------------------------------------------------------------------------------------------
//! @brief      if the rx main port has HPD, or is MHD
//! @param[in]  none
//! @return     true: has HPD
//!				false:no HPD
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterIsRxHotPlugAsserted(void)
{

	if( (SiiRegRead(REG_RX_PORT_SEL) & MSK_MP_PORT_SEL) != SiiSwitchSource1)
	{
		return  (SiiRegRead( REG_MP_STATE ) & ( BIT_PWR5V | BIT_MHL ));
	}
	else
	{
		return (SiiPlatformMHLCDSensePinGet( SiiSwitchSource1 )? SiiPortType_HDMI:SiiPortType_MHL);
	}

//	return (SiiRegRead(GetRegName(REG_MP_STATE)) & ( BIT_PWR5V | BIT_MHL));
}

//-------------------------------------------------------------------------------------------------
//! @brief      assign rx bstatus bytes
//! @param[in]  us_bstatus
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterSetUsBstatus(uint8_t us_bstatus[2])
{

    SiiRegWriteBlock(REG_HDCP_SHD_BSTAT1, us_bstatus, 2);
}

//-------------------------------------------------------------------------------------------------
//! @brief      set the bacps repeater bits
//! @param[in]  true: ON
//!				false: OFF
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterSetBcapsRepeaterBits(bool_t switchOn)
{
#ifdef 	NONE_REPT_WITH_TX_ON
 	switchOn = false;
#endif

//IF INSTA PORT MODE, ALL CLEARED OR SET
//if matrix mode, individually set
    if (currentTopoMode == SI_RPT_MODE_MULTIPLE_REPEATER)
	{
		if (switchOn)
		{
			//Enable repeater bit for port 0-3
			if (inputPort[pipeNumber] <= SiiPORT_3)
			{
				SiiRegModify(REG_HDCP_BCAPS_SET, 0x02 << (inputPort[pipeNumber] * 2), SET_BITS);
			}



		}
		else
		{
			if (inputPort[pipeNumber] <= SiiPORT_3)
				SiiRegModify(REG_HDCP_BCAPS_SET, 0x03 << (inputPort[pipeNumber] * 2), CLEAR_BITS); //both repeater and FIFO rdy



		}


	}
	else //if(currentTopoMode == SI_RPT_MODE_SINGLE_REPEATER)
	{
		if (switchOn)
		{
			//Enable repeater bit for port 0-3
			SiiRegWrite(REG_HDCP_BCAPS_SET,
					VAL_P3_REPEATER | VAL_P2_REPEATER | VAL_P1_REPEATER | VAL_P0_REPEATER);


		}
		else
		{
			//Disable repeater bit for port 0-3
			SiiRegWrite(REG_HDCP_BCAPS_SET, CLEAR_BITS);


		}
	}

//	if switchOn
	SiiRegModify(REG_HDCP_KSV_FIFO_CTRL, VAL_HDCP_FIFO_VALID, SET_BITS);

}

//-------------------------------------------------------------------------------------------------
//! @brief      toggle the Rx Hot Plug High or Low
//
//! @param[in]  portIndex- the active port
//!				switchOn-
//!						true: High
//!						false: Low
//-------------------------------------------------------------------------------------------------
void SiiDrvRepeaterSetBcapsFifoReady(bool_t switchOn)
{
    uint8_t bcaps_msk = 0;
    uint8_t bcaps_val = 0;
    uint8_t portIndex = inputPort[pipeNumber];

    //HW sets ready bit itself when ready.
    //FW sets ready bit manually to ensure when SHA is assigned to Tx,
    //Ready bit still active
    //HW clears the ready bit when new authentication starts


    if(currentTopoMode != SI_RPT_MODE_MULTIPLE_REPEATER)
    {
    	portIndex = SiiPORT_ALL;
    }

	if ( portIndex == SiiPORT_ALL )
	{
		bcaps_val = (switchOn) ? SET_BITS : CLEAR_BITS;
		bcaps_msk = BIT_P0_FIFO_RDY|BIT_P1_FIFO_RDY|BIT_P2_FIFO_RDY|BIT_P3_FIFO_RDY;

		SiiRegModify(REG_HDCP_BCAPS_SET, bcaps_msk, bcaps_val);
		return;
	}

	switch(portIndex)
	{
		case SiiPORT_0:
				bcaps_msk = BIT_P0_FIFO_RDY;
				break;
		case SiiPORT_1:
				bcaps_msk = BIT_P1_FIFO_RDY;
				break;
		case SiiPORT_2:
				bcaps_msk = BIT_P2_FIFO_RDY;
				break;
		case SiiPORT_3:
				bcaps_msk = BIT_P3_FIFO_RDY;
				break;
	}

	bcaps_val = (switchOn) ? SET_BITS : CLEAR_BITS;
	SiiRegModify(REG_HDCP_BCAPS_SET, bcaps_msk, bcaps_val);
}

//-------------------------------------------------------------------------------------------------
//! @brief     reset the Ri' for US to notice the HDCP failure
//-------------------------------------------------------------------------------------------------

void SiiDrvRxHdcpResetRi( void )
{
	//uint8_t tempVal;
	//tempVal = SiiRegRead(REG_HDCP_STAT);

	SiiRegWrite(REG_HDCPCTRL, BIT_CLEAR_RI);
	//SiiRegWrite(REG_HDCP_STAT, tempVal);
}
//-------------------------------------------------------------------------------------------------
//! @brief      Initialize repeater for operation in the requested mode.
//!
//! @param[in]  repeaterMode    - true: initialize as a repeater, false: initialize as a sink
//! @return     true: success
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterEnable ( bool_t repeaterMode )
{
	uint8_t intrMask = 0;

	SiiDrvRepeaterSetBcapsRepeaterBits(repeaterMode);

	//find out the active input mask
	intrMask = 0x01<<(4 + inputPort[pipeNumber]);
	SiiRegModify(REG_INT_ENABLE_IP2, intrMask, repeaterMode? SET_BITS : CLEAR_BITS);

	//turn on SCDT mask here too
	if (pipeNumber == 0)
	{
		SiiRegModify(REG_INT_ENABLE_2, BIT_SCDT_CHG, repeaterMode? SET_BITS : CLEAR_BITS);
        SiiRegModify(REG_INT_ENABLE_P1, BIT_MP_AV_MUTE_MASK | BIT_MP_AV_UNMUTE_MASK, repeaterMode? SET_BITS : CLEAR_BITS);
	}
	else
	{
		SiiRegModify(REG_INT_ENABLE_2, BIT_RP_SCDT_CHG, repeaterMode? SET_BITS : CLEAR_BITS);
        SiiRegModify(REG_INT_ENABLE_P1, BIT_RP_AV_MUTE_MASK, repeaterMode? SET_BITS : CLEAR_BITS);
        SiiRegModify(REG_INT_ENABLE_P2, BIT_RP_AV_UNMUTE_MASK, repeaterMode? SET_BITS : CLEAR_BITS);
	}

	//none pipe related, shared for all the inputs
	SiiRegModify(REG_INT_ENABLE_IP2, BIT_DECRYPT, repeaterMode? SET_BITS : CLEAR_BITS);

    return( true );
}



//-------------------------------------------------------------------------------------------------
//! @brief      Fill the Ksv to Ksv FIFO.
//! @param[in]  p_data- KSV data
//!				length- KSV length
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterFillUsKsvFifo(const uint8_t *p_data, uint16_t length)
{
	SiiRegWriteBlock(REG_KSV_FIFO, p_data, length);
}



//-------------------------------------------------------------------------------------------------
//! @brief      Returns if the main port authentication starts
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterMainPortAuthChanged(void)
{
	bool_t changeIntr = false;
	bool_t result = false;
	uint8_t regVal;

	regVal=SiiRegRead( REG_INT_STATUS_IP2 )& MSK_AUTH_CHG_P0_P3;
	changeIntr = regVal & (0x01<<(4 + inputPort[pipeNumber]));
	if (changeIntr)
	{
		SiiRegWrite(REG_INT_STATUS_IP2, regVal);
	}
	if (changeIntr)
	{
		result = SiiRegRead(GetRegName(REG_MP_STATE)) & BIT_AUTHENED_STATUS;
	}

#ifdef NONE_REPT_WITH_TX_ON
	result = false;
#endif
	return result;

}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns if the Decryption state change interrupt happened
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterAnyInputDecryptionChanged(void)
{
	bool_t result = false;

	//all the input share one interrupt
	if ( SiiRegRead( REG_INT_STATUS_IP2 ) & BIT_DECRYPT )
	{
		result = true;
		SiiRegWrite( REG_INT_STATUS_IP2, BIT_DECRYPT );
	}
#ifdef NONE_REPT_WITH_TX_ON
	result = false;
#endif
	return result;

}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns if the main port AV MUTE status changed, and what it changes to
//
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterMainPortAvMuteChanged(bool_t *muteOn)
{
	bool_t result = false;
	uint8_t regVal;
	uint8_t regVal1;
	uint8_t regMuteBit = 0x01;
	uint8_t regUnMuteBit = 0x02;

	if (pipeNumber == 0)
	{
		regVal = SiiRegRead( REG_INT_STATUS_P1 )& (BIT_MP_AV_MUTE | BIT_MP_AV_UNMUTE);
		if (regVal)
		{
			SiiRegWrite(REG_INT_STATUS_P1, regVal);
		}
		regVal = regVal >> 4;
	}
	else
	{
		regVal1 = SiiRegRead( REG_INT_STATUS_P2 )& BIT_RP_AV_UNMUTE;
		if (regVal1)
		{
			SiiRegWrite(REG_INT_STATUS_P2, regVal1);
		}
		regVal = SiiRegRead( REG_INT_STATUS_P1 )& BIT_RP_AV_MUTE;
		if (regVal)
		{
			SiiRegWrite(REG_INT_STATUS_P1, regVal);
		}
		regVal = (regVal >> 7) | (regVal1 << 1);
	}

	if (regVal)
	{
		if ((regVal & regMuteBit)&&(regVal & regUnMuteBit)) //both set
		{
			if (SiiDrvRepeaterIsUsMuteClr())
			{
				*muteOn = OFF;
			}
			else
			{
				*muteOn = ON;
			}
		}
		else if (regVal & regMuteBit) //mute
		{
			*muteOn = ON;
		}
		else //unmute
		{
			*muteOn = OFF;
		}
		result = true;
	}

	return result;

}

//-------------------------------------------------------------------------------------------------
//! @brief      check if the US cleared the AVMUTE
//! @return  	true: clear
//!				false: set
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterIsUsMuteClr(void)
{
	uint8_t mute;

	mute = SiiRegRead(GetRegName(REG_MP_GCP_STATUS));
	if ((( mute & BIT_GCP_MUTE) == 0) || ( mute & BIT_GCP_UNMUTE) )
	{
		return true;
	}
	else
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      get the active input port
//! @return  	port number
//-------------------------------------------------------------------------------------------------

uint8_t SiiDrvRepeaterGetSelectedPort( void )
{
	return inputPort[pipeNumber];
}


//-------------------------------------------------------------------------------------------------
//! @brief      check if the US input is HMD mode
//! @return  	true: HMDI
//!				false: DVI
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterInputIsHdmi(void)
{
	return true; //not used, pass through
}

//-------------------------------------------------------------------------------------------------
//! @brief      check if the US input mode changed
//! @return  	true: changed
//!				false: no change
//-------------------------------------------------------------------------------------------------

bool_t SiiDrvRepeaterInputModeChange( void )
{
	return false;
}


//-------------------------------------------------------------------------------------------------
//! @brief      change the active input source
//! @param[in]  portIndex - active input index
//-------------------------------------------------------------------------------------------------

void SiiDrvRepeaterActiveInputSet(uint8_t portIndex)
{
	uint8_t intrMask = 0;

	//enable the HDCP change interrupt mask, will clear when Tx cable out
	//same for both mode. Only care the active input.
	//clear the old port

	intrMask = 0x01<<(4 + inputPort[pipeNumber]);
	SiiRegModify(REG_INT_ENABLE_IP2, intrMask, CLEAR_BITS);


	//set the new port
	intrMask = 0x01<<(4 + portIndex);
	SiiRegModify(REG_INT_ENABLE_IP2, intrMask, SET_BITS);

	if (currentTopoMode != SI_RPT_MODE_MULTIPLE_REPEATER)
	{
		inputPortToChange = true;
	}
	else
	{
		SiiDrvRepeaterSetBcapsRepeaterBits(false);//clear the old port rep bit
	}

	inputPort[pipeNumber] = portIndex;
}

//------------------------------------------------------------------------------
// Function:    SiiDrvRepeaterNewPortAuthed
// Description: In the instaPort mode, new input port is decrypted or not
//              if yes, need to start HDCP on the Tx
//------------------------------------------------------------------------------

bool_t SiiDrvRepeaterNewPortAuthed( void )
{
	if ( SiiDrvRepeaterMainPortAuthed() /*&& inputPortToChange*/ )
	{
		inputPortToChange = false;
		return true;
	}
	return false;

}
//------------------------------------------------------------------------------
// Function:    SiiDrvRepeaterProcessInterrupts
// Description: set the flag when interrupt comes
//              these interrupts are used in repeater only
//------------------------------------------------------------------------------

void SiiDrvRepeaterInterruptsSet(bool_t qOn)
{
	intStatus[pipeNumber] = qOn;
}
//------------------------------------------------------------------------------
// Function:    SiiDrvRepeaterResChngStatusSet
// Description: set the flag when resolution change interrupt comes
//              The interrupt is processed elsewhere too.
//------------------------------------------------------------------------------

void SiiDrvRepeaterResChngStatusSet(bool_t qOn)
{
#if (SII_NUM_PIPE == 1)
	if(pipeNumber == 0)
#endif
		resChngStatus[pipeNumber] = qOn;
}

//------------------------------------------------------------------------------
// Function:    SiiDrvRepeaterResChngStatusGet
// Description: get the status of resolution change interrupt flag read previously
//              The interrupt is processed elsewhere too.
//------------------------------------------------------------------------------

bool_t SiiDrvRepeaterResChngStatusGet(void)
{
    return (resChngStatus[pipeNumber]);
}
//------------------------------------------------------------------------------
// Function:    SiiDrvRepeaterProcessInterrupts
// Description: set the flag when interrupt comes
//------------------------------------------------------------------------------
bool_t SiiDrvRepeaterGetInterruptsOn(void)
{
	return intStatus[pipeNumber];
}


