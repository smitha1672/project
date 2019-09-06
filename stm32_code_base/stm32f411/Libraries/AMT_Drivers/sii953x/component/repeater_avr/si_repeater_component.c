//***************************************************************************
//!file     si_repeater_component.c
//!brief    Silicon Image Repeater component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include <string.h>
#include <stdio.h>
#include "si_repeater_component_internal.h"
#include "si_tx_component.h"
#include "si_connection_monitor.h"
#include "si_drv_repeater.h"
#include "si_hdcp_ddc.h"
#include "si_drv_repeater_sha.h"
#include "si_repeater_sha.h"

// Bug 33044 - Melbourne SiI9535/9533 MHL QD882 HDCP CTS 3AB fails at test item 3B-01a-RKSV-127 
// Bug 33042 - Melbourne SiI9535/9533 HDMI QD882 HDCP CTS 3AB fails at test item 3B-01a-RKSV-127 
#define SII_MAX_KSV_NMB	63

//------------------------------------------------------------------------------
//  Repeater Component Instance Data
//------------------------------------------------------------------------------

RepeaterInstanceData_t repeaterIns[SII_NUM_PIPE];
RepeaterInstanceData_t *pRpt = &repeaterIns[0];
uint8_t InputNoHdcp;



//-------------------------------------------------------------------------------------------------
//! @brief      Update last result structure and print an error message.
//!
//!             This function shall be used to report any error or warning in the component.
//!
//! @param[in]  code  - error or warning message code
//-------------------------------------------------------------------------------------------------
static void ReportIssue(rptErrCode_t err_code)
{
	switch(err_code)
	{
		case SI_RPT_ERROR_CODE_INSTANCE_NOT_EXIST:
			DEBUG_PRINT( RPT_MSG_DBG, ("repeater instance more than 2!!\n "));
			break;
		default:
			break;

	}
}

//-------------------------------------------------------------------------------------------------
//! @brief	   clear the HDCP status releated to the Tx
//!
//! @param[in]  txN   - the index of the Tx
//-------------------------------------------------------------------------------------------------
void RepeaterResetTxHdcpStatusData(uint8_t txN)
{
	uint8_t i;
	uint8_t numberOfTx = txN + 1;
	uint8_t startingTx = txN;

	if(txN == RPT_ALL_TX)
	{
		numberOfTx = SII_NUM_TX;
		startingTx = 0;
	}

	for(i = startingTx; i<numberOfTx; i++)
	{
		pRpt->txOnThePipe[i].txStat.isRptTxHdcpDone = false;
		pRpt->txOnThePipe[i].txStat.isHdcpRequested = false;
		pRpt->txOnThePipe[i].txStat.isBstatRdy = false;
		pRpt->txOnThePipe[i].txStat.isRptTxKsvRdy = false;
		pRpt->txOnThePipe[i].txStat.isRptTxShaDone = false;
		pRpt->txOnThePipe[i].txStat.isRptDsNoHdcp = false;
	}

}

//-------------------------------------------------------------------------------------------------
//! @brief	  selects the active repeater instance
//!
//! @param[in]  instance   - the index of the instance
//!             this is a cross instance function
//-------------------------------------------------------------------------------------------------
void SiiRepeaterInstanceSet(uint8_t instance)
{
#if (SII_NUM_PIPE > 1)
    if (instance < SII_NUM_PIPE)
    {
        pRpt = &repeaterIns[instance];
    	SiiDrvRepeaterInstanceSet(instance);
    }
    else
    {
        ReportIssue(SI_RPT_ERROR_CODE_INSTANCE_NOT_EXIST);
    }
#endif //(SII_NUM_PIPE > 1)
}



//-------------------------------------------------------------------------------------------------
//! @brief	   Called to set up the SHA for tx from application, when in TPG mode without repeater
//!			
//! @param[in]  txNum   - enable SHA for which tx output DS
//! @param[in]  isStart - start or stop the SHA resource
//-------------------------------------------------------------------------------------------------
void SiiRepeaterForceShaSet ( uint8_t txNum, bool_t isStart  )  //add switchConf to
{
	repeaterShaRequest_t srq;
	srq.isTx = true;
	srq.portNum = txNum;
	srq.manualTxStart = false;

	if (isStart)
	{
	    SiiDrvShaRequestStart(srq);
	}
	else
	{
	    SiiDrvShaRequestStop(srq.isTx, srq.portNum);
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief	   set up current sytem switch mode
//!
//! @param[in]  repeaterMode   - current switch mode
//-------------------------------------------------------------------------------------------------
void SiiRepeaterSwitchModeSet( RepeaterTopoMode_t repeaterMode )
{
	SiiDrvRepeaterSetMode(repeaterMode);
	RepeaterShaHandlerInitialize();
	SiiDrvShaSetSwitchMode(repeaterMode);
}
//-------------------------------------------------------------------------------------------------
//! @brief      Initialize repeater for operation in the requested mode.
//!             If Repeater option is enabled, init 2 repeater instances
//!
//! @param[in]  enable  - true enables repeater
//! @return     true: success
//!             this is a cross instance function
//-------------------------------------------------------------------------------------------------
bool_t SiiRepeaterInitialize (bool_t enable, bool_t isPowerUp)
{
    uint8_t i;
    if (enable)
    {
        for (i = 0; i < SII_NUM_PIPE; i++)
        {
            memset(&repeaterIns[i], 0x0, sizeof(RepeaterInstanceData_t));
        }
        SiiRepeaterConnectionMonitorInit();
        SiiDrvRepeaterInit();
        SiiRepeaterSwitchModeSet(SI_RPT_MODE_INITIAL);
        InputNoHdcp = 0; //clear the input port non-HDCP status
        //no Rx HPD here. Will do when Tx connection is active
        //re-init every time when mode change.
        RepeaterShaHandlerInitialize();
    }
    else
    {
        //when first power up as init, don't need to clear all status
        if(!isPowerUp)
        {
            SiiRepeaterSwitchModeSet (SI_RPT_MODE_SINGLE_REPEATER);//to clear all the inputs
            SiiDrvRepeaterRxHdcpReset(); //reset Rx
            SiiDrvRepeaterEnable(false);
            for (i = 0; i < SII_NUM_TX; i++)
            {
                SiiRepeaterConInstanceSet(i); //use any tx.
                RepeaterConOnTxHpdTurnOn(OFF, false);
            }
            for (i = 0; i < SII_NUM_PIPE; i++)
            {
                repeaterIns[i].repeaterActive = false;
            }
            //re-init every time when mode change.
            RepeaterShaHandlerInitialize();
        }

    }


    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief      configure the repeater instance
//! @param[in]  txId  - the tx index
//!             qOn   - true: the tx is add to this repeater pipe
//!                     false: the tx is removed from the repeater pipe
//! @return     
//-------------------------------------------------------------------------------------------------
void SiiRepeaterPipeTxConnectionSet(uint8_t txId, bool_t qOn)
{
	pRpt->txOnThePipe[txId].txStat.isRptTxOn = qOn;
	if( pRpt->txOnThePipe[txId].txStat.isRptTxHpdOn )//it was an active Tx
	{
		pRpt->txOnThePipe[txId].txStat.isRptTxHpdOn = false;
		pRpt->numberOfTx --;
        //if numberOfTx becomes 0, will change repeater mode later
	}

}

//-------------------------------------------------------------------------------------------------
//! @brief      configure the repeater instance
//! @param[in]  RepeaterInstanceData_t  - detailed instance configuration data
//! @return
//-------------------------------------------------------------------------------------------------
void SiiRepeaterPipeConfigureSet(RepeaterInstanceData_t  *reptr)
{
    uint8_t i;
    bool_t rptActive = false;
	*pRpt = *reptr;

	RepeaterResetTxHdcpStatusData(RPT_ALL_TX); //reset all the Tx HDCP status of the pipe

	pRpt->repeaterActive = false;
	pRpt->numberOfTx = 0;
	pRpt->numberOfTxAuthed = 0;
	pRpt->numberOfTxHasBstat = 0;

	for(i = 0; i<SII_NUM_TX; i++)
	{
		if(pRpt->txOnThePipe[i].txStat.isRptTxOn)//the Tx on the pipe
		{
			pRpt->txOnThePipe[i].txStat.isRptTxHpdOn = false;
			rptActive = true;
		}
	}
	if( !rptActive )
	{
		SiiDrvHwAutoShaDisable();
		SiiDrvRepeaterEnable(OFF); //in case the Tx is connected but changed to TPG/Parl
	}

}

//-------------------------------------------------------------------------------------------------
//! @brief      Manages repeater instance operations
//!
//! @param[in]  msDiff -  the time between last call and this
//-------------------------------------------------------------------------------------------------
void SiiRepeaterHandler ( clock_time_t msDiff )
{
	uint8_t i;

	if(pRpt->repeaterActive)
	{
	    RepeaterIsrHandler( msDiff, pRpt);
	    RepeaterShaHandler();
	}
	/*for each of the TX on the repeater instance*/
	/*need to full fill the Rx HPLUG pulse even when after repeater mode disabled*/
	for (i = 0; i<SII_NUM_TX; i++)
	{
		if (pRpt->txOnThePipe[i].txStat.isRptTxOn)
		{
			SiiRepeaterConInstanceSet(i);
			RepeaterConnectionMonitor( msDiff );
		}
	}

}



//-------------------------------------------------------------------------------------------------
//! @brief      Upstream authentication only, not change the downstream HDCP
//!
//! @param[in]  tx_number -  the tx that reported the message
//-------------------------------------------------------------------------------------------------
void SiiRepeaterHdcpUsOnlyAuthentication( uint8_t tx_number )
{
	repeaterShaRequest_t srq;
	srq.isTx = false;
	srq.portNum = pRpt->inputPort;
	srq.manualTxStart = false;
	uint8_t i;
	bool_t needReFreshHdcpData = false;

	DEBUG_PRINT( RPT_MSG_DBG, ("US only auth\n "));
	SiiRepeaterConInstanceSet(tx_number);
	RepeaterConAvMuteSet(AV_MUTE_TX_IN_HDCP, OFF);

	if((pRpt->rxBstatus[1]& MSK_HDCP_DDC__BSTATUS2__DEV_DEPTH) != 0)
	{
		RepeaterShaSetLength(pRpt->total_ksv_length); /*restore the SHA length*/
		SiiDrvRepeaterSetUsBstatus(pRpt->rxBstatus);
		SiiRepeaterShaRequest(false, srq);
		SiiDrvRepeaterSetBcapsFifoReady(ON); //DDC still shows not ready, if the HW is not done yet.
	}
	else
	{
		for (i = 0; i<SII_NUM_TX; i++)
		{
			if (pRpt->txOnThePipe[i].txStat.isRptTxOn
				&& pRpt->txOnThePipe[i].txStat.isHdcpRequested
				&& pRpt->txOnThePipe[i].txStat.isRptDsNoHdcp)
			{
				needReFreshHdcpData = true;
			}
		}
		if (needReFreshHdcpData)//DsNoHdcp status ready by now
		{
			for (i = 0; i<SII_NUM_TX; i++)
			{
				if (pRpt->txOnThePipe[i].txStat.isRptTxOn
					&& pRpt->txOnThePipe[i].txStat.isHdcpRequested
					&& !pRpt->txOnThePipe[i].txStat.isRptDsNoHdcp)
				{
					//do not change the instance
					SiiRepeaterHdcpProcessDsBstatus(pRpt->txOnThePipe[i].txBstatus, i);
					SiiRepeaterHdcpFinishUsPart2(pRpt->txOnThePipe[i].txsBksv, i);
				}
			}
		}
	}
}




//-------------------------------------------------------------------------------------------------
//! @brief      process the downstream Bstatus and update the upstream Bstatus accordingly. 
//!			if KSV number exceeded the limit, return false
//!
//! @param[in]  dsBstatus - downstream Bstatus
//! @param[in]  tx_number -  the tx that reported the message
//! @return     -true: KSV is over limit
//!			    -false: KSV within limit
//-------------------------------------------------------------------------------------------------
bool_t SiiRepeaterHdcpProcessDsBstatus( uint8_t dsBstatus[2],uint8_t tx_number )
{
	uint8_t ds_count = dsBstatus[0] & MSK_HDCP_DDC__BSTATUS1__DEV_COUNT;
	uint8_t ds_depth = dsBstatus[1] & MSK_HDCP_DDC__BSTATUS2__DEV_DEPTH;
	uint8_t our_bstatus[]={0,0}; /*compose repeater own BSTATUS and save it*/

	uint8_t numberOfTxHasBstat =0;
	uint8_t numberOfHdcpTx=0;
	bool_t overLimit = false;
	uint8_t i;

	DEBUG_PRINT( RPT_MSG_DBG, "$$$Tx bstatus =  %x  %x \n ",(int)dsBstatus[1], dsBstatus[0]);
	pRpt->txOnThePipe[tx_number].txStat.isBstatRdy= true;
 
	/* check for HW limitation*/
	if ( ds_count >= SII_MAX_KSV_NMB )
	{
		ds_count = SII_MAX_KSV_NMB;
		overLimit = true;
		DEBUG_PRINT( RPT_MSG_DBG, ("max KSV number exceeded\n "));
		/* indicate that the limit is exceeded include only as much devices as could be placed into the FIFO*/
		pRpt->txOnThePipe[tx_number].txBstatus[0] = BIT_HDCP_DDC__BSTATUS1__DEV_EXC;

	}
	else
	{
		pRpt->txOnThePipe[tx_number].txBstatus[0] = ds_count;
	}


	if ( dsBstatus[0] & BIT_HDCP_DDC__BSTATUS1__DEV_EXC )
	{
		pRpt->txOnThePipe[tx_number].txBstatus[0] |= BIT_HDCP_DDC__BSTATUS1__DEV_EXC;
	}


	if (ds_depth < 7 )
	{
		/*adding self level is done at last Tx*/
		pRpt->txOnThePipe[tx_number].txBstatus[1] = ds_depth;
	}
	else/* no more room to indicate bigger depth 0x07 is the maximum depth*/
	{
		pRpt->txOnThePipe[tx_number].txBstatus[1] = BIT_HDCP_DDC__BSTATUS2__CAS_EXC | 0x07;
	}

	if ( dsBstatus[1] & BIT_HDCP_DDC__BSTATUS2__CAS_EXC )
	{
		pRpt->txOnThePipe[tx_number].txBstatus[1] |= BIT_HDCP_DDC__BSTATUS2__CAS_EXC;
	}

	pRpt->txOnThePipe[tx_number].ksv_length = ds_count * LEN_HDCP_DDC__BKSV;

	for ( i = 0; i < SII_NUM_TX; i++)/*start over from Tx HPD beginning*/
	{
		if (pRpt->txOnThePipe[i].txStat.isBstatRdy)
		{
			numberOfTxHasBstat++;
		}
		if ((pRpt->txOnThePipe[i].txStat.isHdcpRequested)
			&&(!pRpt->txOnThePipe[i].txStat.isRptDsNoHdcp))
		{
			numberOfHdcpTx++;
		}
	}

	if (numberOfTxHasBstat == numberOfHdcpTx)//last tx in the pipe		lastTx = true;
	{
		for (i=0; i<SII_NUM_TX; i++)
		{
			if (pRpt->txOnThePipe[i].txStat.isBstatRdy)
			{
				our_bstatus[0] += pRpt->txOnThePipe[i].txBstatus[0];
				if (pRpt->txOnThePipe[i].txBstatus[1] > our_bstatus[1])

					our_bstatus[1] = pRpt->txOnThePipe[i].txBstatus[1];
			}
		}

		/*BIT_HDCP_DDC__BSTATUS2__CAS_EXC should have been carried over*/
		if ( our_bstatus[1] < 7 )
		{
			our_bstatus[1]++;		/*add our own*/
		}
		else
		{
			our_bstatus[1] = BIT_HDCP_DDC__BSTATUS2__CAS_EXC | 0x07;
		}

		if ( our_bstatus[0]<=( SII_MAX_KSV_NMB - numberOfHdcpTx))
		{
			our_bstatus[0] += numberOfHdcpTx;  /* add our own device into the list*/
		}
		else
		{
			overLimit = true;
		}

		if (overLimit)
		{
			/* indicate that the limit is exceeded*/
			/* include only as much devices as could be placed into the FIFO*/
			our_bstatus[0] = BIT_HDCP_DDC__BSTATUS1__DEV_EXC;
		}
	}
	DEBUG_PRINT( RPT_MSG_DBG, "$$$ our bstatus =  %x  %x \n ",(int)our_bstatus[1],
			our_bstatus[0]);

	SiiDrvRepeaterSetUsBstatus( our_bstatus );
	pRpt->rxBstatus[0] = our_bstatus[0]; /*save for future use*/
	pRpt->rxBstatus[1] = our_bstatus[1];
	return overLimit;

}

//-------------------------------------------------------------------------------------------------
//! @brief      finish the upstream part 2 of HDCP authentication
//!
//! @param[in]  aDsBksv - downstream BKSV
//! @param[in]  tx_number -  the tx that reported the message
//-------------------------------------------------------------------------------------------------
void SiiRepeaterHdcpFinishUsPart2( uint8_t aDsBksv[LEN_HDCP_DDC__BKSV], uint8_t tx_number )
{
	uint8_t i,j,n;
	repeaterShaRequest_t srq;
	uint8_t numberOfTxReq;

#if (CHECK_SAME_TX == 1)
	//if the DS is different, have to restart the HDCP from beginning
	if((pRpt->txOnThePipe[tx_number].wasAuthened == true)&& //skipped the HPD relay
	   (!SiiTxStatusGet().isSameDsConnected)) //status ready by now
	{
		pRpt->txOnThePipe[tx_number].wasAuthened = false;
		RepeaterResetTxHdcpStatusData(tx_number);
		RepeaterConOnTxHpdTurnOn( true, false);  //turn on, not the same Tx
		DEBUG_PRINT( RPT_MSG_DBG, "$$$Tx %d is not the same as before, re-authentication starts\n ",(int)tx_number);
	}
	else
#endif //#if (CHECK_SAME_TX == 1)
	{
		pRpt->numberOfTxAuthed = 0; //calculate authened number of tx
		pRpt->total_ksv_length = 0;
		numberOfTxReq = 0;

		pRpt->txOnThePipe[tx_number].txStat.isRptTxShaDone = true;
		/*save the BKSV from the tx*/
		for (i=0; i<LEN_HDCP_DDC__BKSV; i++)
		{
			   pRpt->txOnThePipe[tx_number].txsBksv[i] = aDsBksv[i];
		}
		/*add the tx BKSV to the repeater BKSV*/
		n = 0;
		for (i=0;i<SII_NUM_TX;i++)
		{
			if (pRpt->txOnThePipe[i].txStat.isRptTxShaDone)
			{
				pRpt->numberOfTxAuthed++;
				pRpt->total_ksv_length += pRpt->txOnThePipe[i].ksv_length;
				for (j=0; j<LEN_HDCP_DDC__BKSV; j++)
				{
					pRpt->txsBksv[n++]=pRpt->txOnThePipe[i].txsBksv[j];
				}
			}
			if ((pRpt->txOnThePipe[i].txStat.isHdcpRequested)
				&&(!pRpt->txOnThePipe[i].txStat.isRptDsNoHdcp))
				numberOfTxReq++;
		}
		/*for the last tx
		1. adjust pointer and add BKSV for the txs
		2. calculate the sha length
		3. sha point to the US*/
		if (pRpt->numberOfTxAuthed == numberOfTxReq) /*last one*/
		{
			SiiDrvRepeaterFillUsKsvInfo(
					pRpt->total_ksv_length,
					pRpt->txsBksv,
					pRpt->numberOfTxAuthed,
					tx_number);

			pRpt->total_ksv_length += LEN_HDCP_DDC__BKSV * pRpt->numberOfTxAuthed;  //add last one
			RepeaterShaSetLength(pRpt->total_ksv_length);
			srq.isTx = false;
			srq.portNum = pRpt->inputPort;
			srq.manualTxStart = false;

			/*clear the rdy bit of the input*/
			SiiDrvRepeaterSetBcapsFifoReady(OFF);
			SiiRepeaterShaRequest(false, srq);
			SiiDrvRepeaterSetBcapsFifoReady(ON);
			DEBUG_PRINT( RPT_MSG_DBG, "$$$finish part2\n ");
		}
	}
}


//-------------------------------------------------------------------------------------------------
//! @brief      trun on / off the rx side Fifo ready bits
//-------------------------------------------------------------------------------------------------
void SiiRepeaterHdcpSetBcapsFifoRdyBits( bool_t switchOn )
{
	SiiDrvRepeaterSetBcapsFifoReady( switchOn );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Called when the Tx DDC availability is set to true or false
//! @param[in]  isAcknowledged - DDC available or not
//!             tx_number -  the tx that reported the message

//-------------------------------------------------------------------------------------------------
void SiiRepeaterHdcpDsDdcAck(bool_t isAcknowledged, uint8_t tx_number)
{
	bool_t wasAcknowledged;
	//not update if the tx is not in any repeater instance. if added later, the Rx will be notified
	//and re-read EDID.
	//and this Tx will need to re-authenticated any way.

	wasAcknowledged = !pRpt->txOnThePipe[tx_number].txStat.isRptDsNoHdcp;
	if(pRpt->txOnThePipe[tx_number].txStat.isRptTxOn && pRpt->txOnThePipe[tx_number].txStat.isRptTxHpdOn)
	{
		//update now for SiiRepeaterHdcpInformUsDeviceAboutFailure use
		pRpt->txOnThePipe[tx_number].txStat.isRptDsNoHdcp = !isAcknowledged;
		//if becomes available in the middle
		if(isAcknowledged && (!wasAcknowledged))
		{
			if (pRpt->txOnThePipe[tx_number].txStat.isHdcpRequested)
			{
				SiiTxHdcpDisable();
				pRpt->txOnThePipe[tx_number].txStat.isHdcpRequested = false;
			}
			SiiRepeaterHdcpInformUsDeviceAboutFailure(true, tx_number);

		}
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief     called from TPI component when DS is a repeater, but KSV number is 0.
//!			   SHA is still needed in this case, compared to the DS is a none repeater case
//! @param[in]  tx_number -  the tx that reported the message
//! @return     -true: SHA is available for KSV processing
//!			    -false: SHA is NOT available for KSV processing
//-------------------------------------------------------------------------------------------------
bool_t SiiRepeaterHdcpPrepareFor0KsvListProcessing( uint8_t tx_number )
{
	repeaterShaRequest_t srq;
	pRpt->txOnThePipe[tx_number].txStat.isRptTxKsvRdy = false;/*clear first*/
	if (SiiDrvShaInUse())
	{
		srq = SiiDrvGetCurrentShaReq();
		if ( srq.isTx && srq.portNum != tx_number) //not this request
		{
			return false;
		}
	}

	srq.isTx = true;
	srq.portNum = tx_number;
	srq.manualTxStart = true; /*manual is a must for KSV0 case*/
	pRpt->txOnThePipe[tx_number].txStat.isRptTxKsvRdy = true;
	/*SHA is check already. Force request, since this is manual SHA, additional bit enable is needed every time*/
	SiiRepeaterShaRequest(true, srq); /*force request, since this is manual SHA, additional enable is needed every time*/

	DEBUG_PRINT( RPT_MSG_DBG, "$$$tx %d had 0 KSV \n ",(int)tx_number);

	return true;

}
//-------------------------------------------------------------------------------------------------
//! @brief     called from TPI component when KSV is ready.
//! @param[in]  tx_number -  the tx that reported the message
//@ Return
//				- true:  KSV list processing is ready, SHA is allocated
//				- false: KSV list processing is not ready, SHA is used at present
//-------------------------------------------------------------------------------------------------
bool_t SiiRepeaterHdcpPrepareForKsvListProcessing( uint8_t tx_number )
{
	uint8_t i;
	repeaterShaRequest_t srq;
	uint16_t	preKsvLength = 0;

	/*clear the status first, to avoid add the Tx itself*/
	pRpt->txOnThePipe[tx_number].txStat.isRptTxKsvRdy = false;

	/*check if Tx needs SHA or not*/
	if (!(pRpt->txOnThePipe[tx_number].txBstatus[0]& MSK_HDCP_DDC__BSTATUS1__DEV_COUNT))
	{
		return true;
	}

	/*Tx needs SHA, prepare to request*/
	if (SiiDrvShaInUse())
	{
		srq = SiiDrvGetCurrentShaReq();
		if ( srq.isTx && srq.portNum == tx_number)
		{
			return true;
		}
		else/*not this request*/
		{
			return false;
		}
	}
	else/*SHA is not busy at present*/
	{

		for (i=0;i<SII_NUM_TX;i++)
		{
			if (pRpt->txOnThePipe[i].txStat.isRptTxKsvRdy)
			{
				preKsvLength += pRpt->txOnThePipe[i].ksv_length;
			}
		}
		/*preKsvLength is 0 if it is the first Tx has KSV ready for process*/
		SiiDrvShaAdjustKsvStartAddr( preKsvLength );
		pRpt->txOnThePipe[tx_number].txStat.isRptTxKsvRdy = true;

		srq.isTx = true;
		srq.portNum = tx_number;
		srq.manualTxStart = false;

		SiiRepeaterShaRequest(false, srq);
		DEBUG_PRINT( RPT_MSG_DBG, "$$$set SHA to Tx %d, SHA start %04X\n ",(int)tx_number,preKsvLength);

		return true;
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief       called from TPI component when downstream Hdcp failed.
//!			 Rx side will reset the ri to inform upstream about the failure
//! @param[in]  tx_number -  the tx that reported the message
//-------------------------------------------------------------------------------------------------
void SiiRepeaterHdcpInformUsDeviceAboutFailure( bool_t isFatal,uint8_t tx_number)
{
	uint8_t clearStatus[] = {0,0};
    SiiRepeaterConInstanceSet(tx_number);
    /* Mute DS until HDCP is succeeded*/
	RepeaterConAvMuteSet(AV_MUTE_TX_IN_HDCP, ON);

	//do not reset the Rx side if it is DS HDCP not available.
    if ((isFatal)&&(!pRpt->txOnThePipe[tx_number].txStat.isRptDsNoHdcp))
    {
    	//do not reset Tx HDCP status here, will do after SCDT interrupt
        /* inform US side about the failure*/
    	SiiDrvRepeaterSetBcapsFifoReady( OFF );
       	SiiDrvRepeaterSetUsBstatus( clearStatus );
		DEBUG_PRINT( RPT_MSG_DBG, "$$$tx %d  inform us fatal failure\n ",(int)tx_number);
		/*disable HW auto SHA, will re-enable when 2 part of US auth*/
		SiiDrvHwAutoShaDisable();
        /* toggle HPD since the number of the downstream may changed*/
		RepeaterConOnTxHpdTurnOn( ON, false);
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief       Called when application has detected valid Hpd change from downstream. repeater module then starts tmds/Hdcp process accordingly:
//!              if it is the first Tx has HPD active, repeater enables the repeater mode
//!              if it is the last active Tx has HPD inactive, repeater enables the receiver mode
//!              if it is a HPD toggle event, repeater relays the HPD toggle to the upstream

//! @param[in]  tx_number -  the tx that reported the message
//!             newHpd - DS HPD event type
//-------------------------------------------------------------------------------------------------
void SiiRepeaterOnTxHpdChange( rptHpdTxEvent_t newHpd, uint8_t tx_number)
{
	bool_t sameDs = false;

	SiiRepeaterConInstanceSet(tx_number);
	RepeaterResetTxHdcpStatusData(tx_number);

	if (pRpt->txOnThePipe[tx_number].txStat.isRptTxOn)
	{
		if ((newHpd == SI_RPT_TX_HPD_ON) &&
			(pRpt->txOnThePipe[tx_number].txStat.isRptTxHpdOn == false))
		{
			pRpt->numberOfTx ++;
			pRpt->txOnThePipe[tx_number].txStat.isRptTxHpdOn = true;
			if (pRpt->numberOfTx == 1)/*first downstream*/
			{
				/* Since we have a downstream connection, we must operate in Repeater mode.*/
				SiiDrvRepeaterEnable(ON);
				pRpt->repeaterActive = true;
			}
		}
		 /*SI_RPT_TX_HPD_OFF */
		else if ((newHpd == SI_RPT_TX_HPD_OFF) &&
			(pRpt->txOnThePipe[tx_number].txStat.isRptTxHpdOn == true))
		{
			pRpt->numberOfTx--;
			pRpt->txOnThePipe[tx_number].txStat.isRptTxHpdOn = false;
			if(pRpt->numberOfTx == 0)
			{
				SiiDrvRepeaterEnable(OFF);
				pRpt->repeaterActive = false;
			}
		}

#if (CHECK_SAME_TX == 1)
		if(pRpt->txOnThePipe[tx_number].wasAuthened == true)
		{
			//assume it is the same at this time. Check again after KSV
			sameDs =  true; //SiiTxStatusGet().isSameDsConnected;
		}
#endif  //#if (CHECK_SAME_TX == 1)

		if (newHpd == SI_RPT_TX_HPD_ON)
		{
			RepeaterConOnTxHpdTurnOn( ON, sameDs);
			if ( sameDs && SiiDrvRepeaterMainPortAuthed())
			{
				RepeaterConAvMuteSet(AV_MUTE_TX_IN_HDCP, ON);
				SiiTxHdcpEnable();
				pRpt->txOnThePipe[tx_number].txStat.isHdcpRequested = true;
			}
		}
		else
		{   /*either TOGGLE or OFF, turn off the RX*/
			RepeaterConOnTxHpdTurnOn( OFF, false);
			SiiDrvShaRequestStop(true, tx_number); //stop the current Tx SHA request
		}
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief   called when Tpi has Hdcp authentication successfully done.
//!			 repeater module then unmutes tx
//! @param[in]  tx_number -  the tx that reported the message
//-------------------------------------------------------------------------------------------------
void SiiRepeaterOnTxHdcpDone( uint8_t tx_number )
{
	DEBUG_PRINT( RPT_MSG_DBG, "Tx %d auth done\n ",(int)tx_number);

	pRpt->txOnThePipe[tx_number].txStat.isRptTxHdcpDone = true;
	pRpt->txOnThePipe[tx_number].wasAuthened = true;

	SiiRepeaterConInstanceSet(tx_number);
	RepeaterConAvMuteSet(AV_MUTE_TX_IN_HDCP, OFF);

}

//-------------------------------------------------------------------------------------------------
//! @brief      change the active input source
//! @param[in]  portIndex - active input index
//-------------------------------------------------------------------------------------------------
void SiiRepeaterSetActiveSource( uint8_t portIndex)
{
	pRpt->inputPort = portIndex;
	SiiDrvRepeaterActiveInputSet( portIndex );
}


//-------------------------------------------------------------------------------------------------
//! @brief      retrieve the repeater interrupt status
//! @param[in]  none
//-------------------------------------------------------------------------------------------------
bool_t SiiRepeaterInterruptPending( void )
{
	return (SiiDrvRepeaterGetInterruptsOn()
	        ||SiiDrvRepeaterResChngStatusGet());
}

//-------------------------------------------------------------------------------------------------
//! @brief      Mark the input as Non-HDCP input, and expect the repeater to start the HDCP
//!             at the Downstream when this input is active
//! @param[in]  portIndex- the input port index
//!             qOn-       true:  set to non-HDCP input
//!                        false: clear the non-HDCP input status
//-------------------------------------------------------------------------------------------------
void SiiRepeaterHdcpDsOnlyAuthentication( uint8_t portIndex, bool_t qOn )
{
	if( qOn )
	{
		InputNoHdcp |=  0x01 << portIndex;
	}
	else
	{
		InputNoHdcp &=~(0x01 << portIndex);
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      return the query that the input port is set up as non-HDCP port or not
//! @param[in]  none
//@ Return
//				- true:  input port is set up as non-HDCP port
//				- false: input port is not set up as non-HDCP port
//-------------------------------------------------------------------------------------------------

bool_t RepeaterCurrentInputPortIsSetToNoHdcp( void )
{
	return (( InputNoHdcp & (0x01 << pRpt->inputPort)) == (0x01 << pRpt->inputPort));
	//return (( (0x01 << pRpt->inputPort)) == (0x01 << pRpt->inputPort));//quickfix
}
