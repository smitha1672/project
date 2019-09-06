//***************************************************************************
//!file     si_thx_component.h
//!brief    Silicon Image THX Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_THX_COMPONENT_H__
#define __SI_THX_COMPONENT_H__
#include "si_device_config.h"

#define THX_PKT_HDR_LEN				4
#define NO_PKTS						4
#define	THX_PKT_LEN					32
#define AIF_HDR_FIRST_BYTE			0x84
#define VSIF_HDR_FIRST_BYTE			0x81

#define THX_INSTANCE				1

typedef enum _ThxAppPktType
{
    PKT_NON_THX,
    PKT_THX_VSIF
} ThxAppPktType_t;

//------------------------------------------------------------------------------
//  THX Component Instance Data
//------------------------------------------------------------------------------

typedef struct
{
	int					lastResultCode;
	uint8_t				pktNo;
	uint8_t				thxPktCnt;
	uint8_t				thxPktHdrs[NO_PKTS][THX_PKT_HDR_LEN];
	uint8_t				pkts[NO_PKTS][THX_PKT_LEN];
	uint8_t				nextPkt;
	uint8_t				curPkt;
	uint8_t				curThxPkt;
}	ThxInstanceData_t;

bool_t SiiThxInitialize ( const uint8_t* pPktHdr, uint8_t pktCnt );

//------------------------------------------------------------------------------
// Function:    SiiThxAnalyzePkt
// Description: Analyzes new packet to see if there is any new AIF/VSIF packet has arrived
//------------------------------------------------------------------------------
void SiiThxAnalyzePkt ( void );


#endif // __SI_THX_COMPONENT_H__
