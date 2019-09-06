//***************************************************************************
//!file     si_repeater_sha.c
//!brief    Silicon Image Repeater component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_repeater_component_internal.h"
#include "si_connection_monitor.h"
#include "si_drv_repeater.h"
#include "si_edid_tx_component.h"
#include "si_drv_repeater_sha.h"



repeaterShaRequest_t 	sha_request_q[SII_NUM_PIPE]; /*buffer to save Rx SHA request*/
uint8_t	req_index;
bool_t	first_sha_request;		/*indicates first request, if true don't need to check HDCP status*/

//-------------------------------------------------------------------------------------------------
//! @brief      put SHA request to the queue
//-------------------------------------------------------------------------------------------------
static void ShaRequestPutInQue( repeaterShaRequest_t sReq)
{
	uint_t i;
	/*only Rx request to be in the queue. Tx should request again if SHA busy*/
	if (sReq.isTx)
	{
		return;
	}
	/*ignore same request*/
	if (req_index != 0)
	{
		for (i = 0; i < SII_NUM_PIPE; i++)
		{
			if (sha_request_q[i].portNum == sReq.portNum)
				return;
		}
	}
	sha_request_q[req_index] = sReq;
	DEBUG_PRINT( RPT_MSG_DBG, "SHA req put in queue %s %d q# %d\n ", sha_request_q[0].isTx ? "Tx":"Rx", sha_request_q[0].portNum, req_index);
	req_index++;

}

//-------------------------------------------------------------------------------------------------
//! @brief      get the next SHA request from the queue
//-------------------------------------------------------------------------------------------------
static repeaterShaRequest_t ShaRequestGetFromQue(void)
{
	uint_t i;
	repeaterShaRequest_t 	sha_current_p;
	sha_current_p = sha_request_q[0];

	req_index--;

	/*all move one step forward*/
	for (i=0;i<req_index;i++)
	{
		sha_request_q[i] = sha_request_q[i+1];
	}
	DEBUG_PRINT( RPT_MSG_DBG, "SHA req get from queue %s %d q# %d\n ", sha_current_p.isTx ? "Tx":"Rx", sha_current_p.portNum, req_index);

	return sha_current_p;
}

//-------------------------------------------------------------------------------------------------
//! @brief      service the SHA request
//-------------------------------------------------------------------------------------------------
void RepeaterShaHandler( void )
{
	if ((req_index)&&(!SiiDrvShaInUse()))
	{
		DEBUG_PRINT( RPT_MSG_DBG, "queued SHA req\n ");
		SiiDrvShaRequestStart(ShaRequestGetFromQue());
	}
}

//-------------------------------------------------------------------------------------------------
//! @brief      init data structure
//-------------------------------------------------------------------------------------------------
bool_t RepeaterShaHandlerInitialize ( void )
{
     memset(&sha_request_q[0], 0x00, sizeof(repeaterShaRequest_t) * SII_NUM_PIPE);
     req_index = 0;
	 SiiDrvShaInit();
     return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      force SHA pointer to a TX in TX only mode
//! @param[in]  force  - force the SHA request regardless of its availability
//!             srq - the SHA request
//-------------------------------------------------------------------------------------------------
void SiiRepeaterShaRequest ( bool_t force, repeaterShaRequest_t srq )  //add switchConf to 
{
	if (force) //unconditionally, used when application is sure SHA is not used
	{
		SiiDrvShaRequestStart(srq);
	}
	else
	{
		if (!SiiDrvShaInUse())
		{
			SiiDrvShaRequestStart(srq);
		}
		else
		{
			ShaRequestPutInQue(srq);
		}
	}

}

//-------------------------------------------------------------------------------------------------
//! @brief      set the SHA length as a parameter for SHA calculation
//-------------------------------------------------------------------------------------------------
void RepeaterShaSetLength(uint16_t slength)
{
	SiiDrvSetShaLength(slength);
}






