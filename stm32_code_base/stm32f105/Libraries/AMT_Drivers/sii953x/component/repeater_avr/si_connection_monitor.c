//***************************************************************************
//!file     si_connection_monitor.c
//!brief    Silicon Image Repeater component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include <stdio.h>
#include <string.h>
#include "si_common.h"
#include "si_repeater_component_internal.h"
#include "si_connection_monitor.h"
#include "si_drv_repeater.h"
#include "si_tx_component.h"
#include "si_edid_tx_component.h"
#include "si_drv_tpi_system.h"

typedef struct
{
	bit_fld_t output_tmdson_new 			:1;
	bit_fld_t output_tmdson_old 			:1;
	bit_fld_t output_is_avmute				:1; //previous request status

	uint8_t	  overiden_hold_time;
	uint8_t	  avmute_mask;
	uint16_t  unplugTimeout;

} RepeaterConnectionState_t;

RepeaterConnectionState_t ConState[SII_NUM_TX];
RepeaterConnectionState_t *pCon = &ConState[0];

#if 0
uint8_t SiiRepeaterConInstanceGet ()
{
#if (SII_NUM_TX > 1)
	uint8_t t = (pCon - &ConState[0]);
	return t;
#endif //(SII_NUM_TX > 1)
}
#endif
//-------------------------------------------------------------------------------------------------
//! @brief      Enable the Rx input.
//-------------------------------------------------------------------------------------------------

static void EnableRxInput ( void )
{
    DEBUG_PRINT( RPT_MSG_DBG, "enable input\n" );
    SiiDrvRepeaterSwitchRxHotPlug(ON);
}

//-------------------------------------------------------------------------------------------------
//! @brief      request to turn the TMDS on or off.
//!				The actual TMDS turn on or off is done in a handler
//! @param[in]  qOn
//				- true:  TMDS on
//				- false: TMDS off
//-------------------------------------------------------------------------------------------------

static void ConnectionMonitorTmdsRequestSet(bool_t qOn)
{
	pCon->output_tmdson_new = qOn;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enables or disables Tx TMDS as appropriate
//!             Watches output disable requests and SCDT state to decide when to enable or disable
//!             TMDS output.  Also restores FakeVideo if it was present before turning TMDS off.
//!             state changes.
//-------------------------------------------------------------------------------------------------

static void MonitorTmds( void )
{
    if( pCon->output_tmdson_new != pCon->output_tmdson_old )
    {
        if (pCon->output_tmdson_new == ON)
        {
            if ( SiiDrvRepeaterRxSyncIsOn() )      //scdt on
            {
                SiiRepeaterCbTxResume();   //Tx TMDS on
            	pCon->output_tmdson_old = ON;
            }
        }
        else
        {
        	pCon->output_tmdson_old = OFF;
        	SiiRepeaterCbTxStandby();     //to turn off Tx here.
        	SiiTxHdcpDisable(); //if auto termination en, RX TMDS no interruption to turn off the Tx
            DEBUG_PRINT( RPT_MSG_DBG, "Tx TMDS turn OFF \n");
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Monitors the toggle of Rx Hot plug line is complete or not.
//!				After Rx is enabled again, request to turn of Tx TMDS
//! @param[in]  msDiff: time in ms from last time this function is called
//-------------------------------------------------------------------------------------------------

static void DsConnectionMonitor ( clock_time_t   msDiff )
{
   	MonitorTmds();

    if ( pCon->unplugTimeout == 0 )
    {
    	return;
    }

    if ( pCon->unplugTimeout > msDiff )
    {
    	pCon->unplugTimeout -= msDiff;
        /*DEBUG_PRINT( RPT_MSG_DBG, "Rx HPD time left %d\n",pCon->unplugTimeout);*/

    }
    else if ( pCon->unplugTimeout <= msDiff )
    {
    	pCon->unplugTimeout = 0;
        EnableRxInput();	//time out over, toggle the Rx HPD high
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Brings an instance referred by given index into current context.
//!
//! @param[in]  instance - index of an instance record
//-------------------------------------------------------------------------------------------------
void SiiRepeaterConInstanceSet ( uint8_t instance )
{
#if (SII_NUM_TX > 1)
	pCon = &ConState[instance];
	SiiRepeaterCbTxSelect(instance);
	SiiDrvRxAudioMixInstanceSet(instance);
#endif //(SII_NUM_TX > 1)
}


//-------------------------------------------------------------------------------------------------
//! @brief     init data stucture
//!
//-------------------------------------------------------------------------------------------------
void SiiRepeaterConnectionMonitorInit ( void )
{
	uint8_t i;
	for (i=0; i<SII_NUM_TX; i++)
	{
		ConState[i].output_tmdson_new 	= OFF;
		ConState[i].output_tmdson_old 	= OFF;
		ConState[i].overiden_hold_time  = 0;
		ConState[i].unplugTimeout 		= 0;
		ConState[i].output_is_avmute    = 0;
		ConState[i].avmute_mask         = 0;
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      disable the Rx input
//!
//! @param[in]  disable_hot_plug_also
//				- true: toggles the HPD too.
//				- false: not toggle HPD
//-------------------------------------------------------------------------------------------------
void RepeaterConDisableInput ( bool_t disable_hot_plug_also )
{
    DEBUG_PRINT( RPT_MSG_DBG, "disable input\n");

    if ( disable_hot_plug_also )
    {   //  emulates disconnection on RX side, disable the HPD, termination, HDCP DDC
    	SiiDrvRepeaterSwitchRxHotPlug( OFF );
    }
    //RDY bit will be cleared when new authentication starts
    //HW SHA is disabled in SDCT event
    SiiDrvRepeaterRxHdcpReset();
}

//-------------------------------------------------------------------------------------------------
//! @brief      Rx side handling when downstream HPD status change
//!
//! @param[in]  qOn
//				- true: enable the Rx side when there is active downstream.
//				- false: do nothing since we also have Tx0
//
//-------------------------------------------------------------------------------------------------
void RepeaterConOnTxHpdTurnOn( bool_t qOn , bool_t sameTx)
{
    if ( !qOn )
    {
    	ConnectionMonitorTmdsRequestSet( OFF );
    }
    else
    {
        DEBUG_PRINT( RPT_MSG_DBG, "DS new connection\n");
		ConnectionMonitorTmdsRequestSet( ON );
    }

    if (!sameTx) ////disable input both when On and Off
    {
    	RepeaterConDisableInput( true );
    	switch(SiiDrvSwitchSelectedPorttype())
    	{
    	case SiiPortType_HDMI:
            pCon->unplugTimeout = UNPLUG_TIMEOUT_MS;
    		break;
    	case SiiPortType_MHL:
            pCon->unplugTimeout = UNPLUG_TIMEOUT_MHL_MS;
    		break;
    	default:
    		DEBUG_PRINT( RPT_MSG_DBG, "Error: HPD Width: Wrong Port Type\n");
    		break;
    	}
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Monitor the state of Rx and Tx connections.
//!
//! @param[in]  msThisDiff: time_period from last call
//-------------------------------------------------------------------------------------------------
clock_time_t RepeaterConnectionMonitor( clock_time_t  msThisDiff )
{
	DsConnectionMonitor( msThisDiff );
    return  msThisDiff;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Perform the Tx AVMUTE SET or Clear based on the mute sources status.
//!				To CLEAR: if all the source status clear; To SET: if any one status set
//-------------------------------------------------------------------------------------------------
static void UpdateTxAvMute()
{
	if ( pCon->avmute_mask == 0 ) //clear mute
	{
		if ( pCon->output_is_avmute ) //if current status is different
		{
			SiiDrvTpiAvMuteSourceSet( true );
			SiiDrvTpiAvMuteSendingEnable(OFF);
			pCon->output_is_avmute = false;

		}
	}
	else //set mute
	{
		if ( !pCon->output_is_avmute ) //if current status is different
		{
			SiiDrvTpiAvMuteSourceSet( true );
			SiiDrvTpiAvMuteSendingEnable(ON);
			pCon->output_is_avmute = true;
		}
	}
}
//-------------------------------------------------------------------------------------------------
//! @brief      set the AVMUTE state with cause
//!
//! @param[in]  qOn
//				- true:  AVMUTE set
//				- false: AVMUTE clear
//				cause_mask
//				the initiator of the AVMUTE
//-------------------------------------------------------------------------------------------------
void RepeaterConAvMuteSet( uint8_t cause_mask, bool_t qOn )
{
	if ( cause_mask )
	{
		if ( qOn )
		{
			pCon->avmute_mask |= cause_mask;
		}
		else
		{
			pCon->avmute_mask &= ~cause_mask;
		}
	}
	else
	{
		pCon->avmute_mask = 0;
	}

	UpdateTxAvMute();
}




