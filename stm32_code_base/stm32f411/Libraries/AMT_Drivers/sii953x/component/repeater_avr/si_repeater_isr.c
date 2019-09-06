
//************************************************************************************************
//! @file   si_repeater_isr.c
//! @brief  Transmitter software component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#include <string.h>
#include <stdio.h>
#include "si_repeater_component_internal.h"
#include "si_tx_component.h"
#include "si_connection_monitor.h"
#include "si_drv_repeater.h"
#include "si_drv_repeater_sha.h"


//-------------------------------------------------------------------------------------------------
//! @brief      Stop the downstream HDCP when upstream has SCDT event. If the SCDT is active
//!			already during the check, and the port is already encrypting, the port is pre-authenticated
//!			start TPI HDCP process
//!
//-------------------------------------------------------------------------------------------------
static void RepeaterOnMainPortScdtChange( RepeaterInstanceData_t *localPipe )
{
	uint8_t i;

	for (i=0;i<SII_NUM_TX;i++)
	{
	    if (localPipe->txOnThePipe[i].txStat.isRptTxHpdOn)
		{
			DEBUG_PRINT( RPT_MSG_DBG, "SCDT, stop Tx: %d \n", i);
			SiiRepeaterCbTxSelect(i);
			SiiRepeaterCbTxStandby();
			if (localPipe->txOnThePipe[i].txStat.isHdcpRequested)
			{
				SiiTxHdcpDisable();
			}
			SiiRepeaterConInstanceSet(i);
			RepeaterConAvMuteSet(AV_MUTE_ALL_CLEAR, OFF); //clear all the mute mask
		}
	}

	RepeaterResetTxHdcpStatusData(RPT_ALL_TX);

	if ( SiiDrvRepeaterRxSyncIsOn() )
	{
		for (i=0;i<SII_NUM_TX;i++)
		{
		    if (localPipe->txOnThePipe[i].txStat.isRptTxHpdOn)
			{
			    SiiRepeaterCbTxSelect(i);
			    SiiRepeaterCbTxResume();   //Tx TMDS on
			}
		}

		if ( SiiDrvRepeaterNewPortAuthed()
			|| RepeaterCurrentInputPortIsSetToNoHdcp() )
		{

			for (i=0;i<SII_NUM_TX;i++)
			{
			    if (localPipe->txOnThePipe[i].txStat.isRptTxHpdOn)
				{
					SiiRepeaterConInstanceSet(i);
					RepeaterConAvMuteSet(AV_MUTE_TX_IN_HDCP, ON);
					SiiTxHdcpEnable();
					localPipe->txOnThePipe[i].txStat.isHdcpRequested = true;
					DEBUG_PRINT( RPT_MSG_DBG, "new port auth on, Tx %d SI_TxHdcpStart()\n ",i);

				}
			}
		}
	}

}


//-------------------------------------------------------------------------------------------------
//! @brief      Interrupt service routine handler
//!
//! @param[in]  msDiff  	- time difference between last call and this call.
//!				currentPipe - the instance data of this repeater pipe
//! @return     none
//-------------------------------------------------------------------------------------------------
//pass in the instance data, instead of instance Tx data, to use repeaterActive and numberOfTx
//to determine the delayed repeater mode change
void RepeaterIsrHandler( clock_time_t msDiff, RepeaterInstanceData_t *currentPipe)
{
	uint8_t i;
	bool_t muteOn;
	bool_t hdmiOn;

	if ((!SiiDrvRepeaterGetInterruptsOn())
	     &&(!SiiDrvRepeaterResChngStatusGet()))
	{
		return; //no interrupts pending
	}

	if (SiiDrvRepeaterMainPortAvMuteChanged(&muteOn))
	{
		for (i=0;i<SII_NUM_TX;i++)
		{
            if (currentPipe->txOnThePipe[i].txStat.isRptTxHpdOn)
			{
				SiiRepeaterConInstanceSet(i);
				RepeaterConAvMuteSet(AV_MUTE_RX_REQUEST, muteOn);
			}
		}
	}

   	if ( SiiDrvRepeaterRxSyncIntr() || SiiDrvRepeaterResChngStatusGet() )
	{
   		SiiDrvHwAutoShaDisable();
   		if((currentPipe->repeaterActive == true)
   		    &&(currentPipe->numberOfTx == 0)) //not a repeater mode any more
   		{
   		     SiiDrvRepeaterEnable(OFF); //delay the repeater mode change to prevent audio disturb
   		     currentPipe->repeaterActive = false;
   		}

	    if ( !SiiDrvRepeaterIsRxHotPlugAsserted()) 
	    {
	    	SiiDrvRepeaterRxHdcpReset();
	    }

		RepeaterOnMainPortScdtChange(currentPipe);
	}

   	if (SiiDrvRepeaterMainPortAuthChanged())
	{
		for (i=0;i<SII_NUM_TX;i++)
		{
            if (currentPipe->txOnThePipe[i].txStat.isRptTxHpdOn
                    && (!currentPipe->txOnThePipe[i].txStat.isRptDsNoHdcp))
			{
				SiiRepeaterConInstanceSet(i);
				RepeaterConAvMuteSet(AV_MUTE_TX_IN_HDCP, ON);
				SiiTxHdcpEnable();
				currentPipe->txOnThePipe[i].txStat.isHdcpRequested = true;
			}
		}
	}

   	if (SiiDrvRepeaterAnyInputDecryptionChanged())
   	{//the Tx doesn't start the HDCP, but the Rx has started Decryption
   		if (SiiDrvRepeaterMainPortAuthed())
   		{
   			for (i=0;i<SII_NUM_TX;i++)
   			{
   	            if (currentPipe->txOnThePipe[i].txStat.isRptTxHpdOn
   	                    && (!currentPipe->txOnThePipe[i].txStat.isHdcpRequested))
   				{
   					SiiRepeaterConInstanceSet(i);
   					RepeaterConAvMuteSet(AV_MUTE_TX_IN_HDCP, ON);
   					SiiTxHdcpEnable();
   	                currentPipe->txOnThePipe[i].txStat.isHdcpRequested = true;
   				}
   			}
   		}

   	}
	if (SiiDrvRepeaterInputModeChange())
	{
		DEBUG_PRINT( RPT_MSG_DBG, ("~~ HDMI mode change to "));
		hdmiOn = SiiDrvRepeaterInputIsHdmi();
		for (i=0;i<SII_NUM_TX;i++)
		{
            if (currentPipe->txOnThePipe[i].txStat.isRptTxHpdOn)
 		    {
				SiiRepeaterCbTxSelect(i);
				SiiTxOutputInterfaceSet(hdmiOn?
						SI_TX_OUTPUT_MODE_HDMI : SI_TX_OUTPUT_MODE_DVI);
			}
		}
		DEBUG_PRINT( MSG_DBG, hdmiOn? "HDMI\n" : "DVI\n");
	}

	SiiDrvRepeaterResChngStatusSet(false);
	SiiDrvRepeaterInterruptsSet(false); /*the pipe interrupt serviced,clear the flag*/

}
