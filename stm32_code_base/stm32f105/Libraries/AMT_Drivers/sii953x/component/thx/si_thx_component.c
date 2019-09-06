//***************************************************************************
//!file     si_thx_component.c
//!brief    Silicon Image THX Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_drv_internal.h"
#include "si_debug.h"
#include "si_thx_component.h"

ThxInstanceData_t		thxInst[THX_INSTANCE];
ThxInstanceData_t* 		pThx  = &thxInst[0];

//------------------------------------------------------------------------------
// Function:    SiiThxInitialize
// Description:
// Returns:     TRUE if no problem
//------------------------------------------------------------------------------

bool_t SiiThxInitialize ( const uint8_t* pPktHdr, uint8_t thxPktCnt)
{
    uint8_t i, j;
    memset(pThx, 0, sizeof(ThxInstanceData_t));

    pThx->pktNo = 0;
    pThx->thxPktCnt = thxPktCnt;

    for (i = 0; i < thxPktCnt; i++)
    {
        DEBUG_PRINT( MSG_ALWAYS, "\nTHX Packet Header[%X]:: ", i );
        for (j = 0; j < THX_PKT_HDR_LEN; j++)
        {
            pThx->thxPktHdrs[i][j] = *pPktHdr;
            pPktHdr++;
            DEBUG_PRINT( MSG_ALWAYS, "%02X ", pThx->thxPktHdrs[i][j] );
        }
    }
    DEBUG_PRINT( MSG_ALWAYS, "\n" );

    return true;
}

// figures out if two packets are identical
static bool_t ThxPktsIdentical ( uint8_t* pPkt1, uint8_t* pPkt2 )
{
    uint8_t i;

    for (i = 0; i < THX_PKT_LEN - 1; i++)
    {
        if (*pPkt1 != *pPkt2)
            return false;
        pPkt1++;
        pPkt2++;
    }

    return true;
}

// figures out if the packet is already there in the queue
static bool_t ThxPktExistsInQueue ( uint8_t* pPkt )
{
    uint8_t i;

    for (i = 0; i < NO_PKTS; i++)
    {
        if (ThxPktsIdentical(pPkt, pThx->pkts[i]))
        {
            return true;
        }
    }
    return false;
}

// inserts the packet in the queue
static void ThxInsertInQueue ( uint8_t* pPkt )
{
    uint8_t i, j;
    bool_t found;

    found = false;

    for (i = 0; i < NO_PKTS; i++)
    {
        // check the last unused byte to signal if its an empty packet
        if (pThx->pkts[i][THX_PKT_LEN - 1] == 0x00)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        for (j = 0; j < THX_PKT_LEN - 1; j++)
        {
            pThx->pkts[i][j] = *pPkt;
            pPkt++;
        }

        pThx->pkts[i][THX_PKT_LEN - 1] = 0x01;
    }
    else
    {
        DEBUG_PRINT( MSG_ALWAYS, "\nTHX:: Queue Full:: \n");
    }
}

// prints the whole queue
static void ThxPrintQueue()
{
    uint8_t i, j;

    DEBUG_PRINT( MSG_ALWAYS, "\nPrinting Queue:: \n");
    for (i = 0; i < NO_PKTS; i++)
    {
        // print only if the packet is non-empty
        if (pThx->pkts[i][THX_PKT_LEN - 1] == 0x01)
        {
            DEBUG_PRINT( MSG_ALWAYS, "\nPacket[%x][%04s]:: ", i, (pThx->pkts[i][0] == AIF_HDR_FIRST_BYTE) ? "AIF" : "VSIF" );
            for (j = 0; j < THX_PKT_LEN - 1; j++)
            {
                DEBUG_PRINT( MSG_ALWAYS, "%02X ", pThx->pkts[i][j] );
            }
        }
    }
    DEBUG_PRINT( MSG_ALWAYS, "\n\n" );
}

// print a packet including what kind of packet it is
static void ThxPrintPkt( uint8_t* pPkt )
{
    uint8_t i;

    DEBUG_PRINT( MSG_ALWAYS, "\nPrinting New Packet:: \n");
    for (i = 0; i < THX_PKT_LEN - 1; i++)
    {
        DEBUG_PRINT( MSG_ALWAYS, " %02X", *pPkt );
        pPkt++;
    }
    DEBUG_PRINT( MSG_ALWAYS, "\n\n" );
}

// figure out if the next packet search is going to be for THX or NON-THX
static uint8_t NextPkt()
{
    if (pThx->curPkt == PKT_NON_THX)
    {
        pThx->nextPkt = PKT_THX_VSIF;
        pThx->curThxPkt++;
    }
    else
    {
        if (pThx->curThxPkt == pThx->thxPktCnt)
        {
            pThx->nextPkt = PKT_NON_THX;
            pThx->curThxPkt = 0;
        }
        else
        {
            pThx->nextPkt = PKT_THX_VSIF;
            pThx->curThxPkt++;
        }
    }

    return pThx->nextPkt;
}


//------------------------------------------------------------------------------
// Function:    SiiThxAnalyzePkt
// Description: Analyzes new packet to see if there is any new AIF/VSIF packet has arrived
//------------------------------------------------------------------------------
void SiiThxAnalyzePkt ( void )
{
    uint16_t reg, i;
    uint8_t pkt[THX_PKT_LEN];

    //DEBUG_PRINT( MSG_ALWAYS, "SiiThxAnalyzePkt() called!! :%02X\n", pThx->curPkt );

    // Read VSIF infoframe
	for (i = 0; i < THX_PKT_LEN - 1; i++)
	{
		reg = REG_RP_VISF_CONTENT_HEADER + i;
		pkt[i] = SiiRegRead(reg);
		//DEBUG_PRINT( MSG_ALWAYS, "reg [%02X]:: %02x \n", reg, pkt[i] );
	}

	// If VSIF, check to see if it is in our queue
	if (pkt[0] == VSIF_HDR_FIRST_BYTE)
	{
		if (!ThxPktExistsInQueue(&pkt[0]))
		{
			ThxInsertInQueue(&pkt[0]);
			ThxPrintPkt(&pkt[0]);
			DEBUG_PRINT( MSG_ALWAYS, "This packet is a NEW VSIF packet!!\n" );
			ThxPrintQueue();
		}
	}

	// Now read the AIF buffer infoframe
    for (i = 0; i < THX_PKT_LEN - 1; i++)
    {
        reg = REG_MP_AIF_CONTENT_HEADER + i;
        pkt[i] = SiiRegRead(reg);
        //DEBUG_PRINT( MSG_ALWAYS, "reg [%02X]:: %02x \n", reg, pkt[i] );
    }

    // Check if AIF aor VSIF
    if ((pkt[0] == VSIF_HDR_FIRST_BYTE) || (pkt[0] == AIF_HDR_FIRST_BYTE))
    {
        if (!ThxPktExistsInQueue(&pkt[0]))
        {
        	ThxInsertInQueue(&pkt[0]);
			ThxPrintPkt(&pkt[0]);
			if (pThx->curPkt == PKT_NON_THX)
			{
				DEBUG_PRINT( MSG_ALWAYS, "This packet is a NEW AIF packet!!\n" );
			}
			else
			{
				DEBUG_PRINT( MSG_ALWAYS, "This packet is a NEW THX VSIF packet!!\n" );
			}
			ThxPrintQueue();
        }
    }

    // Alternate AIF buffer between checking for AIF packets and for specific VSIF packets
	if (NextPkt() == PKT_THX_VSIF)
	{
	    // Set up for capturing a specific VSIF packet
		for (i = 0; i < THX_PKT_HDR_LEN; i++)
		{
			SiiRegWrite(REG_VISF_PB1 + i, pThx->thxPktHdrs[pThx->curThxPkt - 1][i]);
			//DEBUG_PRINT( MSG_ALWAYS, "reg [%02X]:: %02x \n", REG_VISF_PB1+i, SiiDeviceRegisterRead(REG_VISF_PB1+i) );
		}

		// Enable capture of the specific VSIF loaded above
		SiiRegWrite(REG_CLR_PACKET_BUFFER, BIT7 | BIT6 | BIT5 | BIT3);
		//DEBUG_PRINT( MSG_ALWAYS, "\nWriting reg [%02X]:: %02x as the next pkt for search is THX packet!!\n", REG_CLR_PACKET_BUFFER, SiiDeviceRegisterRead(REG_CLR_PACKET_BUFFER) );
	}
	else
	{
	    // Return to normal capture of AIF
		SiiRegWrite(REG_CLR_PACKET_BUFFER, BIT0);
		//DEBUG_PRINT( MSG_ALWAYS, "\nWriting reg [%02X]:: %02x as the next pkt for search is a NON-THX packet!!\n", REG_CLR_PACKET_BUFFER, SiiDeviceRegisterRead(REG_CLR_PACKET_BUFFER) );
	}

    // set the current equal to next packet as when this routine will be called next the then current pkt would be current next packet
    pThx->curPkt = pThx->nextPkt;
}



