//***************************************************************************
//!file     sk_app_repeater.c
//!brief    Application level control of the repeater component and driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include <string.h>
#include "sk_application.h"
#include "si_repeater_component.h"
#include "si_tx_component.h"



typedef struct
{
	RepeaterTopoMode_t				switchMode;                     /*restart rep if mode change*/
	uint8_t							numberOfRepeater;               /*number of active repeater*/
	RepeaterInstanceData_t			repeaterPipe[SII_NUM_PIPE]; /*pipe 0 is main, 1 is sub*/

}RepeaterConfigureData_t;

//------------------------------------------------------------------------------
// Module variables
//------------------------------------------------------------------------------
RepeaterConfigureData_t 	initSwitchSetting;
RepeaterInstanceData_t     *pSwitchSetting = &initSwitchSetting.repeaterPipe[0];

//------------------------------------------------------------------------------
// Function:    RepeaterTimerSet
// Description: starts timer for Repeater use
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

static void RepeaterTimerSet ( clock_time_t msDelay  )
{
    app.msRepeaterDelay = msDelay;
    app.msRepeaterStart = SiiOsTimerTotalElapsed();
}

//------------------------------------------------------------------------------
// Function:    RepeaterTimerElapsed
// Description: Elapsed timers for Repeater use
// Parameters:  none
// Returns:     -true: timer time out
//
//------------------------------------------------------------------------------
static bool_t RepeaterTimerElapsed ( clock_time_t *msDiff )
{
    *msDiff = SkTimeDiffMs(app.msRepeaterStart, SiiOsTimerTotalElapsed());

    return(*msDiff >= app.msRepeaterDelay);
}


static uint8_t RepeaterInstanceGetFromTx( uint8_t tx_number)
{
	uint8_t i;

	for (i = 0; i<SII_NUM_TX; i++ )  //single repeater may on subpipe

	{
		if(initSwitchSetting.repeaterPipe[i].txOnThePipe[tx_number].txStat.isRptTxOn)
		{
			SiiRepeaterInstanceSet(i);
			break;
		}
	}
	return i;
}

//------------------------------------------------------------------------------
// Function:    RepeaterOnInputPortSwitch
// Description: Perform the repeater action when Tx#1 has detected Hpd ON or OFF
// Parameters:  [in] qOn
//				- true: TPI active downstream detected
//				- false: TPI inactive downstream detected
// Returns:     none
//------------------------------------------------------------------------------

static void	RepeaterOnInputPortSwitch (uint8_t ins, uint8_t rx)
{
	initSwitchSetting.repeaterPipe[ins].inputPort = rx;
	SiiRepeaterInstanceSet(ins);
	SiiRepeaterSetActiveSource(rx);
}


//------------------------------------------------------------------------------
// Function:    RepeaterSourceToOutputSel
// Description: Update the Tx to the right repeater pipe
// Parameters:  -muxSel: Tx input from main/sub/TPG
///             -txIndx: the Tx number
// Returns:     none
//------------------------------------------------------------------------------
static void	RepeaterSourceToOutputSel (SiiSwitchHdmiPipe_t muxSel, uint8_t txIndx)
{
	uint8_t repeaterIns;
	repeaterIns = RepeaterInstanceGetFromTx(txIndx);

	initSwitchSetting.repeaterPipe[repeaterIns].txOnThePipe[txIndx].txStat.isRptTxOn=false;

	if ((muxSel > SiiSwitchHdmiPipeSub) ) //disabled, or not a HDMI input
	{
		SkAppTxInstanceSet(txIndx);
		if (SiiTxStatusGet().isDsConnected)
		{
		    SiiRepeaterCbTxStandby();
			SiiTxHdcpDisable();
			// TX needs to be in standalone mode
			SiiTxHdcpConfigure(OFF, 0, OFF);
		}
	}
	else //main or sub, add the tx to new pipe
	{
		initSwitchSetting.repeaterPipe[muxSel].txOnThePipe[txIndx].txStat.isRptTxOn=true;
	}

}

//------------------------------------------------------------------------------
// Function:    SkAppRepeaterSourceConfig
// Description: Configure the repeater instances when the system topology changes
//              disable the HDCP and TMDS of the changed Tx before re-conf,
//              re-activate afterwards.
// Parameters:  - sourceSel: the HDMI input port of the zone(tx)
//              - PipeConf: the input source (mian/sub/TPG) of the zone, and Matrix en
//              - txHdcpRestart: if the Tx TPG with audio insertion change needs Rx EDID and HDCP restart
// Returns:		- none
//------------------------------------------------------------------------------

void SkAppRepeaterSourceConfig( SiiSwitchSource_t sourceSel[],
                                SiiMswInstanceCommonData_t PipeConf,
                                bool_t txHdcpRestart[])
{
	bool_t txChange[2] = {false, false};
	bool_t pipeChange[2] = {false, false};

	bool_t modeChange = false;
	txStatus_t tst;
	uint8_t i, j;

	if( !app.repeaterEnable )
	{
		return;
	}
	if (initSwitchSetting.switchMode != SI_RPT_MODE_SINGLE_REPEATER)
	{
		initSwitchSetting.switchMode = SI_RPT_MODE_SINGLE_REPEATER;
		initSwitchSetting.numberOfRepeater = 1;
		modeChange = true;
	}

	for( i = 0; i < SII_NUM_TX; i++ )
	{
		 //if the tx input pipe-source is changed:
		 //(1)main <-> sub
         //(2)TPG -> Main/Sub (if TPG <- Main/Sub, do nothing)
		 //or Matrix / InstaPort mode change
	     if ( modeChange
	    	//previously not on this pipe
	    	||((!initSwitchSetting.repeaterPipe[PipeConf.pipeSel[i]].txOnThePipe[i].txStat.isRptTxOn )
	    	//and now is HDMI source that valid to this pipe
	    	  && (PipeConf.pipeSel[i] < SiiSwitchHdmiPipeNone )))
		 {
			 RepeaterSourceToOutputSel(PipeConf.pipeSel[i], i); //re-conf the tx  pipe
			 txChange[i] = true;
			 if (PipeConf.pipeSel[i] < SiiSwitchHdmiPipeNone )
			 {
				 pipeChange[PipeConf.pipeSel[i]] = true;
			 }
		 }
	    	//previously on this pipe
		 if (PipeConf.pipeSel[i] >= SiiSwitchHdmiPipeNone )
		 {
			 uint8_t ins = RepeaterInstanceGetFromTx(i);//get the previous pipe instance for the Tx
			 if( ins < SII_NUM_PIPE )//the Tx was active in repeater pipe before
			 {
				 RepeaterSourceToOutputSel(PipeConf.pipeSel[i], i); //set new configuration
				 SiiRepeaterPipeTxConnectionSet(i, OFF);
			 }

		 }
         //re-conf the input HDMI to the pipe,
		 else if (initSwitchSetting.repeaterPipe[PipeConf.pipeSel[i]].inputPort != sourceSel[i])
		 {
	    	 RepeaterOnInputPortSwitch (PipeConf.pipeSel[i], sourceSel[i]);
		 }
	}

	//configure each repeater instance
	for( j = 0; j<SII_NUM_TX; j++ )
	{
		if( modeChange || pipeChange[j])
		{
			for( i = 0; i<SII_NUM_TX; i++ )
			{
				if(initSwitchSetting.repeaterPipe[j].txOnThePipe[i].txStat.isRptTxOn )
				{
					SkAppTxInstanceSet(i);
					SiiTxHdcpConfigure(ON, 0, ON);
					tst = SiiTxStatusGet();
					if (tst.isSinkReady)
					{
					    SiiRepeaterCbTxStandby();
						SiiTxHdcpDisable();
					}

				}
			}
		}
	}

	if( modeChange )
	{
		SiiRepeaterSwitchModeSet( initSwitchSetting.switchMode );

		for( j = 0; j<SII_NUM_TX; j++ )
		{
			SiiRepeaterInstanceSet(j);
			pSwitchSetting = &initSwitchSetting.repeaterPipe[j];
			SiiRepeaterPipeConfigureSet(pSwitchSetting);
		}
	}
	else for( i = 0; i<SII_NUM_TX; i++ )
	{
		if ( txChange[i] && (PipeConf.pipeSel[i] < SiiSwitchHdmiPipeNone))
		{
			SiiRepeaterInstanceSet(PipeConf.pipeSel[i]);
			pSwitchSetting = &initSwitchSetting.repeaterPipe[PipeConf.pipeSel[i]];
			SiiRepeaterPipeConfigureSet(pSwitchSetting);
		}
	}

	//activate the instance if the Tx is already on.
	//after the Tx has been turned off to avoid snow noise
	for( j = 0; j<SII_NUM_TX; j++ )
	{
		if( modeChange || pipeChange[j])
		{
			for( i = 0; i<SII_NUM_TX; i++ )
            {
                if(initSwitchSetting.repeaterPipe[j].txOnThePipe[i].txStat.isRptTxOn )
                {
                    SkAppTxInstanceSet(i);
                    tst = SiiTxStatusGet();
                    //Bug 31744 - With samsung S3 and S4 as MHL source, video out is not consistent when plug/unplug cable at source side
                    if (tst.isDsConnected)
                    {
                        //SkAppProcessTxEdid(i);
                    	SiiOsTimerWait(200);
                    }
                    if (tst.isSinkReady)
                    {
                        SkAppTxHpdConnection(SI_RPT_TX_HPD_ON);
                    }
                    else
                    {
                        SkAppTxHpdConnection(SI_RPT_TX_HPD_OFF);
                    }
                }
            }
		}
		else
		{
            for( i = 0; i<SII_NUM_TX; i++ )
          {
                if ((txHdcpRestart[i])&&
                    (initSwitchSetting.repeaterPipe[j].txOnThePipe[i].txStat.isRptTxOn ))
            {
                SkAppTxInstanceSet(i);
                tst = SiiTxStatusGet();
                if (tst.isSinkReady)
                {
                    SkAppTxHpdConnection(SI_RPT_TX_HPD_ON);
                }
            }
          }
		}

	}

}
//------------------------------------------------------------------------------
// Function:    SkAppDeviceInitRepeater
// Description: Perform any board-level initialization required at the same
//              time as Repeater component initialization
// Parameters:  isPowerUp
//              - true:  called when power up
//				- false: called to change the mode on fly

// Returns:
//				- true:  success
//				- false: fail
//------------------------------------------------------------------------------
bool_t SkAppDeviceInitRepeater( bool_t isPowerUp )
{
	memset(&initSwitchSetting, 0, sizeof(RepeaterConfigureData_t));

	initSwitchSetting.switchMode = SI_RPT_MODE_INITIAL;

	initSwitchSetting.numberOfRepeater = 0;

    SiiRepeaterInitialize(app.repeaterEnable, isPowerUp);

    return true;
}

//------------------------------------------------------------------------------
// Function:    SkAppRepeaterConfigure
// Description: Enable/disable repeater function
//              Must be followed by configuration of the
//              correct topo settings
// Parameters:  enable
//				- true:  enable repeater
//				- false: disable repeater
// Returns:
//				- true:  success
//				- false: fail
//------------------------------------------------------------------------------
bool_t SkAppRepeaterConfigure ( bool_t enable )
{
	uint8_t i;
    // If Repeater enable state has changed, tell the component.
	// app.oldRepeaterEnable by default is false
	if( app.oldRepeaterEnable != enable )
	{
		app.oldRepeaterEnable = enable;
    //	app.repeaterEnable default is determined by the SW1-2 on starter board
		app.repeaterEnable = enable;
	//  only call init as repeater when SW1-2 is on
	//  if SW1-2 is off, SkAppDeviceInitRepeater()is called by AppDeviceInitialize
	//  or later change on fly
		SkAppDeviceInitRepeater( false );
	}
	if(!enable)
	{
		for( i = 0; i<SII_NUM_TX; i++ )
		{
			SkAppTxInstanceSet(i);
			SiiTxHdcpConfigure(OFF, 0, OFF);
		}
	}

	PrintStringOnOff( "Repeater ", app.repeaterEnable );

    return( true );
}



//------------------------------------------------------------------------------
// Function:    SkAppTxHpdConnection
// Description: Perform the repeater action when Tx has detected Hpd ON or OFF
// Parameters:  [in] qOn
//				- true: TPI active downstream detected
//				- false: TPI inactive downstream detected
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTxHpdConnection (rptHpdTxEvent_t qOn)
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

    if ( app.repeaterEnable )
    {
    	(void)RepeaterInstanceGetFromTx(tx_number);
        SiiRepeaterOnTxHpdChange(qOn, tx_number);
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppTxHdcpAuthDone
// Description: Perform the repeater action when Tx has complete HDCP DS auth
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTxHdcpAuthDone ( void )
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

    if ( app.repeaterEnable )
    {
    	(void)RepeaterInstanceGetFromTx(tx_number);
        SiiRepeaterOnTxHdcpDone(tx_number);
    }

}

//------------------------------------------------------------------------------
// Function:    SkAppTaskRepeater
// Description: Repeater tasks run at the back ground
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskRepeater ( void )
{
    clock_time_t msDiff = 0;
    uint8_t instance;
    bool_t timeOut = false;

    if(RepeaterTimerElapsed( &msDiff ))
    {
    	timeOut = true;
    	RepeaterTimerSet( 40 );
    }
	for (instance = 0; instance < SII_NUM_TX; instance++)
	{
		SiiRepeaterInstanceSet(instance);
		if ( timeOut || (app.repeaterEnable && SiiRepeaterInterruptPending()) )
		{
			SiiRepeaterHandler( msDiff );
		}
	}

    timeOut = false;

}

//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater that downstream HDCP authentication
//!             is already done. Only upstream has to be authenticated.
//-------------------------------------------------------------------------------------------------

void SkAppRepeaterHdcpUsOnlyAuthentication(void)
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
    SiiRepeaterHdcpUsOnlyAuthentication(tx_number);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater if downstream HDCP is available.
//!
//! @param[in]  isAcknowledged - true:  DS HDCP responded, false: no DDC acknowledge at DS
//-------------------------------------------------------------------------------------------------

void SkAppRepeaterHdcpDsDdcAck(bool_t isAcknowledged)
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
	SiiRepeaterHdcpDsDdcAck(isAcknowledged, tx_number);

}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Send BSTATUS from TX to App/Repeater for processing.
//!
//! @param[in]  dsBstatus - 2-byte array carrying BSTATUS data
//-------------------------------------------------------------------------------------------------

bool_t SkAppRepeaterHdcpProcessDsBstatus(uint8_t dsBstatus[2])
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
    return SiiRepeaterHdcpProcessDsBstatus(dsBstatus, tx_number);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Send 20-bit BKSV from TX to App/Repeater in order to
//!             finalize 2nd part of repeater HDCP authentication.
//!
//! @param[in]  aDsBksv - 5-byte array carrying downstream BKSV data
//-------------------------------------------------------------------------------------------------

void SkAppRepeaterHdcpFinishUsPart2(uint8_t aDsBksv[5])
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
    SiiRepeaterHdcpFinishUsPart2(aDsBksv, tx_number);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater if BCAPS FIFO is ready.
//!
//! @param[in]  isFifoReady - BCAPS FIFO ready flag.
//-------------------------------------------------------------------------------------------------

void SkAppRepeaterHdcpSetBcapsFifoRdyBits(bool_t isFifoReady)
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
    SiiRepeaterHdcpSetBcapsFifoRdyBits(isFifoReady);
}


//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater that TX is about to receive KSV List.
//-------------------------------------------------------------------------------------------------

bool_t SkAppRepeaterHdcpPrepareForKsvListProcessing(void)
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
    return (SiiRepeaterHdcpPrepareForKsvListProcessing(tx_number));
}

//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater that TX is about to receive KSV List.
//-------------------------------------------------------------------------------------------------

bool_t SkAppRepeaterHdcpPrepareFor0KsvListProcessing(void)
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
    return(SiiRepeaterHdcpPrepareFor0KsvListProcessing(tx_number));
}

//-------------------------------------------------------------------------------------------------
//! @brief      TX callback function. Informs App/Repeater about HDCP failure.
//!
//! @param[in]  isFatal - true:  fatal HDCP issue, TX won't retry,
//!                       false: minor issue, TX will retry to authenticate
//-------------------------------------------------------------------------------------------------

void SkAppRepeaterHdcpInformUsDeviceAboutFailure(bool_t isFatal)
{
	uint8_t tx_number;
	tx_number = SiiTxInstanceGet();

	(void)RepeaterInstanceGetFromTx(tx_number);
    SiiRepeaterHdcpInformUsDeviceAboutFailure(isFatal, tx_number);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Repeater callback function setting transmitter instance
//-------------------------------------------------------------------------------------------------
void SiiRepeaterCbTxSelect(uint8_t instance)
{
    SkAppTxInstanceSet(instance);
}
