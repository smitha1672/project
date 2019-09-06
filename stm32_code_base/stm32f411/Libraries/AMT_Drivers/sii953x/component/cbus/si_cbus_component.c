//***************************************************************************
//!file     si_cbus_component.c
//!brief    Silicon Image CBUS Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/


#include "string.h"
#include "si_common.h"
#include "si_cbus_internal.h"
#include "si_drv_cbus.h"
#include "si_debug.h"
#include "si_regs_pp953x.h"
#include "si_cra.h"
#include "si_edid_tx_component.h"
#include "si_cbus_config.h"
#if (MHL_EXTRA == ENABLE)

// 2D SVDs and DTDs that are supported in 3D
extern  uint8_t g3DEdidTableVIC [ EDID_3D_VIC_TABLE_LEN ];
extern const uint8_t g3DEdidTableDTD [ EDID_3D_DTD_TABLE_LEN ];
#endif

#if (MHL_20 == ENABLE)
#include "si_edid_tx_config.h"
MHL3DSupportData_t gMHL3D_VIC[SII_NUM_EDID_TX];
MHL3DSupportData_t gMHL3D_DTD[SII_NUM_EDID_TX];
MHL3DSupportData_t  *pMHL3D_VIC = &gMHL3D_VIC[0];
MHL3DSupportData_t  *pMHL3D_DTD = &gMHL3D_DTD[0];
#endif

//  active index in cbus queue
#define CH_ACTIVE_INDEX     (pCbus->activeIndex)

//static functions
static void 	CBusProcessSubCommand (uint8_t* vsCmdData);
static void 	CBusResetToIdle ();
static int_t 	CBusSendNextInQueue ();
static void 	CBusDumpQueueContents ( void );
static void 	CBusClearQueueEntry ( uint8_t cmd );
static bool_t 	CBusWriteCommand ( cbus_req_t *pReq  );
static bool_t 	CBusWritePeersScratchpad();
static uint8_t 	CBusCheckInterruptStatus ();
static bool_t 	CBusSendDcapRdyMsg ();
static bool_t 	CBusSetInt ( uint8_t regOffset, uint8_t regBit );

/*static */bool_t 	CBusSendMscCommand(uint8_t cmd);
static bool_t 	CBusSendDscrChange ();
static bool_t 	CBusReqWrt ();
static bool_t 	CBusGrtWrt ();
static bool_t 	CBusWriteStatus ( uint8_t regOffset, uint8_t value );
static bool_t 	CBusSendDcapRdy ();
static bool_t 	CBusTimerExpired(clock_time_t *pTimer, uint16_t waitTime);
static void 	CBusChkTimers ();



#if (MHL_20 == ENABLE)
static void		CBus3DCheckInstance( uint_t instanceIndex );
static void 	CBus3DInstanceSet ( uint_t instanceIndex );
static void 	CBusSend3DInfo();
static int seq;
#endif

//------------------------------------------------------------------------------
//  CBUS Component Instance Data
//------------------------------------------------------------------------------

CbusInstanceData_t cbusInstance[MHL_MAX_CHANNELS];
CbusInstanceData_t *pCbus = &cbusInstance[0];

//------------------------------------------------------------------------------
//! @brief  Assign port and support mask to this CBUS instance (channel)
//! @param[in]  port - Receiver input port
//! @param[in]  supportMask - combination of SiiMhlLogicalDeviceTypes_t types
//!                           supported by this device.
//------------------------------------------------------------------------------
void SiiCbusConfigure ( uint8_t port, uint8_t supportMask )
{
    pCbus->port = port;
    pCbus->supportMask  = supportMask;
    SiiDrvCbusConfigure(port);
}

//------------------------------------------------------------------------------
// Description: Set the component global data pointer to the requested instance.
// Parameters:  instanceIndex
// Returns:     true if legal index, false if index value is illegal.
//------------------------------------------------------------------------------

bool_t SiiCbusInstanceSet ( uint_t instanceIndex )
{
    if ( instanceIndex < MHL_MAX_CHANNELS )
    {
        SiiDrvCbusInstanceSet( instanceIndex );
        pCbus = &cbusInstance[ instanceIndex];
        pCbus->instanceIndex = instanceIndex;
#if (MHL_20 == ENABLE)
        CBus3DCheckInstance ( instanceIndex );
#endif
        return( true );
    }

    return( false );
}

//------------------------------------------------------------------------------
// Description: Set the component global data pointer to the requested instance
//				based on Port
// Parameters:  portIndex
// Returns:     true if legal index, false if index value is illegal.
//------------------------------------------------------------------------------
void SiiCbusInstanceSetBasedOnPort ( uint_t portIndex )
{
	uint8_t i;

	for ( i=0; i < MHL_MAX_CHANNELS; i++ )
	{
		pCbus = &cbusInstance[ i ];
		if( pCbus->port == portIndex )
		{
			//DEBUG_PRINT( MSG_ALWAYS, "\n\n*******Setting Instance: %02X\n", i);
			SiiCbusInstanceSet(i);
			break;
		}
	}
}

//------------------------------------------------------------------------------
// Function:    SiiCbusStatus
// Description: Returns a status flag word containing CBUS-specific
//				information about the state of the device.
// Parameters:  none
// Returns:     Sixteen-bit misc flags word for the CBUS Component
//------------------------------------------------------------------------------

uint16_t SiiCbusStatus ( void )
{
	return( pCbus->miscFlags );
}

//------------------------------------------------------------------------------
// Function:    SiiCbusStandby
// Description: Places the CBUS component into standby mode if available
// Parameters:  none
// Returns:     true if the CBUS component entered standby mode successfully,
//				or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCbusStandby ( void )
{
	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCbusResume
// Description: Causes operation of the CBUS component to return to the state
//				it was prior to the previous call to SiiCbusStandby.
// Parameters:  none
// Returns:     true if the CBUS component successfully exited standby mode,
//				or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCbusResume ( void )
{
	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxCbusConnected
// Description: Return the CBUS channel connected status for this channel.
// Returns:     true if connected.
//              false if disconnected.
//------------------------------------------------------------------------------

bool_t SiiMhlRxCbusConnected ()
{
    return( pCbus->connected );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxChannelEnable
// Description: Enable or disable the specified CBUS channel.
// param		isEnable - true to enable, false to disable
//------------------------------------------------------------------------------
void SiiMhlRxChannelEnable ( bool_t isEnable )
{
	SiiDrvCbusChnEn( isEnable, pCbus->port );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxIsQueueFull
// Description: is cbus queue full ?
// Parameters:  none
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t SiiMhlRxIsQueueFull ( void )
{
    int_t   queueIndex;

	for ( queueIndex = 0; queueIndex < CBUS_MAX_COMMAND_QUEUE; queueIndex++ )
	{
		if ( pCbus->request[ queueIndex].reqStatus == CBUS_REQ_IDLE )
		{
			return false;
		}
	}

	return true;
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxIsQueueEmpty
// Description: is cbus queue empty ?
// Parameters:  none
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t SiiMhlRxIsQueueEmpty ( void )
{
    int_t   queueIndex;

    for ( queueIndex = 0; queueIndex < CBUS_MAX_COMMAND_QUEUE; queueIndex++ )
    {
        if ( pCbus->request[ queueIndex].reqStatus == CBUS_REQ_PENDING )
        {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendRAPCmd
// Description: Send MSC_MSG (RAP) message to the specified CBUS channel (port)
//
// Parameters:  actCode 	- RAP action code
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------
bool_t SiiMhlRxSendRAPCmd ( uint8_t actCode )
{
    return CBusSendMscMsgCmd(MHL_MSC_MSG_RAP, actCode);
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendACPCmd
// Description: Send MSC_MSG (ACP) message to the specified CBUS channel (port)
//
// Parameters:  asciiCode 	- ACP ASCII character
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------
bool_t SiiMhlRxSendUCPCmd ( uint8_t asciiCode )
{
    return CBusSendMscMsgCmd(MHL_MSC_MSG_UCP, asciiCode);
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxGetMscMsgCmd
// Description: Get MSC_MSG (RCP/RAP) message that came on CBUS channel
//
// Parameters:  cmd    		- MSC_MSG cmd (RCP/RAP/ACP)
//              data     	- keyCode
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------

bool_t SiiMhlRxGetMscMsgCmd ( uint8_t* cmd, uint8_t* data )
{
    *cmd = pCbus->lastCbusRcvdCmd;
    *data = pCbus->lastCbusRcvdData;
	return true;
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendRcpk
// Description: Send RCPK (ack) message
//
// Parameters:  keyCode
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------

bool_t SiiMhlRxSendRcpk ( uint8_t keyCode)
{
    return( CBusSendMscMsgCmd( MHL_MSC_MSG_RCPK, keyCode ));
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendAcpe
// Description: Send ACPE (error) message
//
// Parameters:  cmdStatus
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------
bool_t SiiMhlRxSendUcpe ( uint8_t cmdStatus )
{
    return( CBusSendMscMsgCmd( MHL_MSC_MSG_UCPE, cmdStatus ));
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendAcpk
// Description: Send ACPK (ack) message
//
// Parameters:  asciiCode
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------

bool_t SiiMhlRxSendUcpk ( uint8_t asciiCode)
{
    return( CBusSendMscMsgCmd( MHL_MSC_MSG_UCPK, asciiCode ));
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendRcpe
// Description: Send RCPE (error) message
//
// Parameters:  cmdStatus
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------
bool_t SiiMhlRxSendRcpe ( uint8_t cmdStatus )
{
    return( CBusSendMscMsgCmd( MHL_MSC_MSG_RCPE, cmdStatus ));
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendRapk
// Description: Send RAPK (acknowledge) message to the specified CBUS channel
//              and set the request status to idle.
//
// Parameters:  cmdStatus
// Returns:     true        - successful queue/write
//              false       - write and/or queue failed
//------------------------------------------------------------------------------

bool_t SiiMhlRxSendRapk ( uint8_t cmdStatus )
{
    return( CBusSendMscMsgCmd( MHL_MSC_MSG_RAPK, cmdStatus ));
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendMsge
// Description: Send MSGE msg back if the MSC command received is not recognized
//
// Returns:     true        - successful
//              false       - failed
//------------------------------------------------------------------------------

bool_t SiiMhlRxSendMsge ()
{
    return( CBusSendMscMsgCmd( MHL_MSC_MSG_E, MHL_MSC_INVALID_SUBCMD ));
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxHpdSet
// Description: Send MHL_SET_HPD to source
// parameters:	setHpd - self explanatory :)
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t SiiMhlRxHpdSet (bool_t setHpd)
{
	if( setHpd )
	{
		if( SiiMhlRxCbusConnected() )
		{
			if( !CBusSendMscCommand( MHL_SET_HPD ) )
			{
				DEBUG_PRINT(MSG_ALWAYS,"MHL SET HPD FAILED\n");
				return false;
			}
			//Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
//			if( !SiiMhlRxSendEdidChange() )
//				return false;
			if( !SiiMhlRxPathEnable(true) )
				return false;
			// Configure always MHL 1x mode
			SiiRegWrite(PA_MHL1X_CONF_REG_1, (BIT_ALWAYS_1X_MHL1X | BIT_MHL_OPMODE_3X_DEF) );
		}
    }
	else
	{
		if( SiiMhlRxCbusConnected() )
		{
			if( !SiiMhlRxPathEnable(false) )
				return false;
			if( !CBusSendMscCommand( MHL_CLR_HPD ) )
				return false;
		}
    }

	return true;
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxWritePeerScratchpad
// Description: sends MHL write burst cmd to peer
// Parameters:  startOffset - start offset of peer's scratchpad register
//				length - total# of scratchpad registers to be written
//				pMsgData - pointer to data needs to be written in peer's scratchpad
// Returns:     SUCCESS or one of CBUS_SOFTWARE_ERRORS_t
//------------------------------------------------------------------------------

bool_t SiiMhlRxWritePeerScratchpad(uint8_t startOffset, uint8_t length, uint8_t* pMsgData)
{
	if( pCbus->miscFlags & FLAGS_SCRATCHPAD_BUSY )
	{
		DEBUG_PRINT( MSG_ALWAYS, "Scratchpad busy!! Please come back later!!\n" );
		return false;
	}

	pCbus->wbLength = length;
	pCbus->wbStartOffset = startOffset;

	// write different values in different registers
	SiiDrvCbusWriteLocalScratchpad( startOffset, length, pMsgData );

	// send REQ_WRT interrupt to peer
	if( !(CBusReqWrt()) )
	{
		DEBUG_PRINT( MSG_ALWAYS, "Couldn't send REQ_WRT to peer\n" );
		return false;
	}

	pCbus->miscFlags |= FLAGS_SCRATCHPAD_BUSY;

	return true;
}


//------------------------------------------------------------------------------
// Function:    SiiMhlRxReadDevCapReg
// Description: Read device capability register
// Parameters:  regOffset - offset of peer's device capability register
// Returns:     true/false
//------------------------------------------------------------------------------

bool_t SiiMhlRxReadDevCapReg(uint8_t regOffset)
{
	cbus_req_t 	req;
	bool_t		success;

	pCbus->lastCbusSentCmd = req.command = MHL_READ_DEVCAP;
	req.offsetData = regOffset;

	if( !(success = CBusWriteCommand(&req)) )
	{
		DEBUG_PRINT( MSG_ALWAYS, "Couldn't send MHL_READ_DEVCAP to peer\n" );
	}

	return success;
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendEdidChange
// Description: set edid_chg interrupt
//------------------------------------------------------------------------------
bool_t SiiMhlRxSendEdidChange ( void )
{
	return ( CBusSetInt(0x01, BIT1) );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxSendDevCapChange
// Description: sends device capability change interrupt to peer
// Parameters:  none
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t SiiMhlRxSendDevCapChange ( void )
{
	return ( CBusSetInt(0x00, BIT0) );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxPathEnable
//------------------------------------------------------------------------------
bool_t SiiMhlRxPathEnable ( bool_t enable )
{
	bool_t success = true;

//	DEBUG_PRINT( MSG_DBG, "\nSiiMhlRxPathEnable:: pCbus->pathEnableSent = %X.\n", pCbus->pathEnableSent );
	if ( enable )
    {
		if ( !pCbus->pathEnableSent )
		{
			// enable PATH_EN bit on peer's appropriate status register (offset 0x31)
			DEBUG_PRINT( MSG_DBG, "\nSiiMhlRxPathEnable:: Setting bit 3 to peer's status register.\n" );
			if( (success = CBusWriteStatus(0x01, BIT3)) == true )
			{
				pCbus->pathEnableSent = true;
			}
		}
    }
    else
    {
    	if ( pCbus->pathEnableSent )
    	{
            // disable PATH_EN bit on peer's appropriate status register (offset 0x31)
//    		DEBUG_PRINT( MSG_DBG, "\nSiiMhlRxPathEnable:: Clearing bit 3 to peer's status register.\n" );
    		if( (success = CBusWriteStatus(0x01, 0)) == true )
    		{
    			pCbus->pathEnableSent = false;
    		}
    	}
    }

	return success;
}

#if INC_CEC
extern void SkAppCbusCecSendVendorId(uint8_t instanceIndex, uint8_t *vendorId );
#endif

//------------------------------------------------------------------------------
// Function:    SiiMhlRxHandler
// Description: Check the state of any current CBUS message on specified channel.
//              Handle responses or failures and send any pending message if
//              channel is IDLE.
// Parameters:  channel - CBUS channel to check, must be in range, NOT 0xFF
// Returns:     SUCCESS or one of CBUS_SOFTWARE_ERRORS_t
//------------------------------------------------------------------------------

uint8_t SiiMhlRxHandler ( void )
{
#if ( configSII_DEV_953x_PORTING == 1 )
    uint8_t result = CBUS_SUCCESS;
#else
   uint8_t result = SUCCESS; 
#endif 

    /* Check the channel interrupt status to see if anybody is  */
    /* talking to us. If they are, talk back.                   */

	if( !(pCbus->miscFlags & FLAGS_SENT_DCAP_RDY) && !pCbus->initTimer && SiiMhlRxCbusConnected())
	{
		// Send MHL_SET_HPD command
		//  Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
		// Clear queue
		memset(&pCbus->request[0], 0, (sizeof(cbus_req_t) * CBUS_MAX_COMMAND_QUEUE) );
		pCbus->queueDepth = 0;

		pCbus->sendMHLCommands = 1;

		SiiMhlRxHpdSet(false);
		SiiMhlRxHpdSet(true);

		// send DCAP_RDY and DCAP_CHG
		CBusSendDcapRdyMsg();
	}

	if(!pCbus->sendMHLCommands)
	{
		//  Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
		memset(&pCbus->request[0], 0, (sizeof(cbus_req_t) * CBUS_MAX_COMMAND_QUEUE) );
		pCbus->queueDepth = 0;
	}

	//checkMhlRsense();
	result = CBusCheckInterruptStatus();

    // check for all the f/w timers
    CBusChkTimers();

    /* Update the channel state machine as necessary.   */
	if ( pCbus->state == CBUS_IDLE )
	{
		result = CBusSendNextInQueue(); // No command in progress, write new command immediately.
	}

	if ( pCbus->state & CBUS_XFR_DONE )
	{
		pCbus->state = pCbus->state & ~CBUS_XFR_DONE;
		if ( pCbus->lastCbusSentCmd == MHL_READ_DEVCAP )
		{
			pCbus->lastReadDevCapReg = pCbus->request[ CH_ACTIVE_INDEX ].msgData[0];
			SiiMhlCbRxMscCmdResponseData( pCbus->lastCbusSentCmd, pCbus->lastReadDevCapReg );
		}
#if INC_CEC
		if ( pCbus->lastCbusSentCmd == MHL_GET_VENDOR_ID )
		{
			SkAppCbusCecSendVendorId( pCbus->instanceIndex,pCbus->request[ CH_ACTIVE_INDEX ].msgData );
		}
#endif

		if ( pCbus->lastCbusSentCmd == MHL_WRITE_BURST )
		{
			//DEBUG_PRINT( MSG_DBG, "Send DSCR Change!\n" );
			// send DSCR_CHG interrupt to peer
			if( !(CBusSendDscrChange()) )
			{
				DEBUG_PRINT( MSG_ALWAYS, "Couldn't send DSCR_CHG to peer\n" );
			}
			pCbus->miscFlags &= ~FLAGS_SCRATCHPAD_BUSY;

#if (MHL_EXTRA == ENABLE)
			if ( pCbus->serving3dVicReq || pCbus->serving3dDtdReq )
			{
				//CBusSend3DInfo();
			}
#endif
#if (MHL_20 == ENABLE)
			if ( pCbus->sending_3D_info)
			{
				CBusSend3DInfo();
			}
#endif
		}

		pCbus->waitTimer = 0;
		if( pCbus->queueDepth )
		{
			pCbus->queueDepth--;
		}
		DEBUG_PRINT( MSG_DBG, "\n-------Transfer Done!!------ "
								"Queue Depth: %02X active index(cbus): %02X\n", pCbus->queueDepth, CH_ACTIVE_INDEX);
		memset( &(pCbus->request[CH_ACTIVE_INDEX]), 0, sizeof( cbus_req_t ) );
	}

	if ( pCbus->state & CBUS_RECEIVED )
	{
	    // Process RCPx/RAPx messages
		pCbus->state = pCbus->state & ~CBUS_RECEIVED;
		if( (pCbus->lastCbusRcvdCmd == MHL_MSC_MSG_RCPK) || (pCbus->lastCbusRcvdCmd == MHL_MSC_MSG_RAPK) || ( pCbus->lastCbusRcvdCmd == MHL_MSC_MSG_UCPK))
		{
			pCbus->rcpRapAbortTimer = 0;
		}
//		DEBUG_PRINT( MSG_DBG, "\nMSG MSG received!! active index(cbus): %02X\n", CH_ACTIVE_INDEX);
		CbusProcessRcpRap( pCbus->lastCbusRcvdCmd, pCbus->lastCbusRcvdData);
	}

	if ( pCbus->state & CBUS_FAILED )
	{
		pCbus->state = pCbus->state & ~CBUS_FAILED;
		pCbus->request[ CH_ACTIVE_INDEX ].reqStatus = CBUS_REQ_IDLE;
		if( pCbus->queueDepth )
		{
			pCbus->queueDepth--;
		}
		DEBUG_PRINT( MSG_DBG, "\n-----Last Cbus cmd failed!!------"
						"Queue Depth: %02X cmd failed: %02X\n", pCbus->queueDepth, pCbus->request[ CH_ACTIVE_INDEX ].command);
		memset( &(pCbus->request[CH_ACTIVE_INDEX]), 0, sizeof( cbus_req_t ) );
	}

    return( result );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlRxInitialize
// Description: Attempts to initialize the CBUS. If register reads return 0xFF,
//              it declares error in initialization.
//              Initializes discovery enabling registers and anything needed in
//              config register, interrupt masks.
// Returns:     TRUE if no problem
//------------------------------------------------------------------------------

bool_t SiiMhlRxInitialize ( void )
{
    int_t instanceIndex;

    // Clear instance data
    instanceIndex = pCbus->instanceIndex;       // Remember our instance
    memset( pCbus, 0, sizeof( CbusInstanceData_t ));
    memset( &(pCbus->request), 0, sizeof( cbus_req_t ) * CBUS_MAX_COMMAND_QUEUE);
    pCbus->instanceIndex = instanceIndex;       // Restore our instance

#if (MHL_20 == ENABLE)
    memset(&gMHL3D_VIC[0], 0, sizeof(MHL3DSupportData_t)*SII_NUM_EDID_TX);
    memset(&gMHL3D_DTD[0], 0, sizeof(MHL3DSupportData_t)*SII_NUM_EDID_TX);
#endif
    return( SiiDrvCbusInitialize() );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlCbHpdSetState
// Description: This callback is used when the Receiver finishes with all its
//              initialization and ready for Transmitter to read
//              EDID/KSV FIFO/BCAPS ...
// Returns:     true: success
//              false: either port is not MHL or CBUS is not ready.
//------------------------------------------------------------------------------
bool_t SiiMhlCbHpdSetState ( int_t portIndex, SiiDrvHpdActions_t newState )
{
    bool_t success;

    success = false;

    // Translate request to CBUS command.
    switch ( newState )
    {
    case SiiHPD_INACTIVE:
    case SiiHPD_TOGGLE:             // Start with a toggle to OFF
        success = SiiMhlRxHpdSet(false);
        break;
    case SiiHPD_ACTIVE:
    	success = SiiMhlRxHpdSet(true);
        break;
    default:
        break;
    }

    // If a toggle, queue up the second command.
    if ( success && ( newState == SiiHPD_TOGGLE ))
    {
        success = SiiMhlRxHpdSet(true);
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    CBusSendNextInQueue
// Description: Starting at the current active index, send the next pending
//              entry, if any
//------------------------------------------------------------------------------

static int_t CBusSendNextInQueue ()
{
#if ( configSII_DEV_953x_PORTING == 1 )
    int_t   result = CBUS_SUCCESS;
#else
 int_t   result = SUCCESS;
#endif 
    int_t   nextIndex = 0;

    if ( ( !pCbus->hpdWaitTimer ) && ( !pCbus->abortTimer ) && ( !pCbus->waitTimer )  )
    {
		nextIndex = (CH_ACTIVE_INDEX == ( CBUS_MAX_COMMAND_QUEUE - 1 )) ? 0 : (CH_ACTIVE_INDEX + 1);

		while (	( pCbus->request[ nextIndex].reqStatus != CBUS_REQ_PENDING ) ||
				( pCbus->rcpeRcpkGapTimer && ( (pCbus->request[ nextIndex].msgData[0] == MHL_MSC_MSG_RCPK) || (pCbus->request[ nextIndex].msgData[0] == MHL_MSC_MSG_UCPK) )) ||
				( pCbus->rcpRapAbortTimer && ( pCbus->request[ nextIndex].msgData[0] == MHL_MSC_MSG )))
		{
			if ( nextIndex == CH_ACTIVE_INDEX )     // Searched whole queue, no pending
			{
				CH_ACTIVE_INDEX = CBUS_MAX_COMMAND_QUEUE - 1;

#if ( configSII_DEV_953x_PORTING == 1 )
				return( CBUS_SUCCESS );  // No pending messages is still success
#else
				return( SUCCESS );  // No pending messages is still success
#endif

			}
			nextIndex = ( nextIndex == (CBUS_MAX_COMMAND_QUEUE - 1)) ? 0 : (nextIndex + 1);
		}

//		DEBUG_PRINT( MSG_DBG, "Active Index: %0x\n", nextIndex);
		CBusDumpQueueContents();
        //  Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 

		if( pCbus->request[nextIndex].command == MHL_CLR_HPD )
		{
			SiiDrvRxHdcpDdcControl( pCbus->port, false );
			SiiDrvRxEdidDdcControl( pCbus->port, false );
		}
		else if( pCbus->request[nextIndex].command == MHL_SET_HPD )
		{
			SiiDrvRxHdcpDdcControl( pCbus->port, true );
			SiiDrvRxEdidDdcControl( pCbus->port, true );
		}

		// Found a pending message, send it out
		if ( SiiDrvInternalCBusWriteCommand( &pCbus->request[ nextIndex] ))
		{
			CH_ACTIVE_INDEX = nextIndex;
			pCbus->waitTimer = SiiOsTimerTotalElapsed();
			pCbus->state = CBUS_SENT;
			pCbus->lastCbusSentCmd = pCbus->request[CH_ACTIVE_INDEX].command;

			if( pCbus->request[CH_ACTIVE_INDEX].command == MHL_CLR_HPD )
			{
				pCbus->hpdWaitTimer = SiiOsTimerTotalElapsed();
			}
			if( (pCbus->request[CH_ACTIVE_INDEX].command == MHL_SET_INT) && (pCbus->request[CH_ACTIVE_INDEX].msgData[0] == BIT2) )
			{
				DEBUG_PRINT( MSG_DBG, "\n\nWaiting for peer to respond to reqWrt!!\n");
				pCbus->reqWrtTimer = SiiOsTimerTotalElapsed();
			}
			if( pCbus->request[ nextIndex].command == MHL_MSC_MSG )
			{
				if( (pCbus->request[ nextIndex].msgData[0] != MHL_MSC_MSG_RCPK) &&
					(pCbus->request[ nextIndex].msgData[0] != MHL_MSC_MSG_RAPK) &&
					(pCbus->request[ nextIndex].msgData[0] != MHL_MSC_MSG_UCPK) )
				{
					pCbus->rcpRapAbortTimer = SiiOsTimerTotalElapsed();
				}
				if( (pCbus->request[ nextIndex].msgData[0] == MHL_MSC_MSG_RCPE) || (pCbus->request[ nextIndex].msgData[0] == MHL_MSC_MSG_UCPE))
				{
					pCbus->rcpeRcpkGapTimer = SiiOsTimerTotalElapsed();
				}
			}
		}
		else
		{
            //  Bug 31921 - Melbourne on power up, Unconnected port to MHL connected port switch not working 
			if( pCbus->request[nextIndex].command == MHL_SET_HPD )
			{
				SiiDrvRxHdcpDdcControl( pCbus->port, false );
				SiiDrvRxEdidDdcControl( pCbus->port, false );
			}

			result = ERROR_WRITE_FAILED;
		}
    }

    return( result );
}

//------------------------------------------------------------------------------
// Function:    CBusProcessSubCommand
// Description: Process a sub-command
//------------------------------------------------------------------------------
static void CBusProcessSubCommand (uint8_t* vsCmdData)
{
    uint8_t vs_cmd, vs_data;

    vs_cmd = vsCmdData[0];
    vs_data = vsCmdData[1];

    // Save MSC_MSG data in the request structure to be returned
    // to the upper level.

    pCbus->lastCbusRcvdCmd = vs_cmd;
    pCbus->lastCbusRcvdData = vs_data;

//    DEBUG_PRINT( MSG_DBG, "CBUS:: MSG_MSC CMD:  0x%02X\n", vsCmdData[0] );
//    DEBUG_PRINT( MSG_DBG, "CBUS:: MSG_MSC Data: 0x%02X\n", vsCmdData[1] );
}

//------------------------------------------------------------------------------
// Function:    CBusResetToIdle
// Description: Set the specified channel state to IDLE. Clears any messages that
//              are in progress or queued.  Usually used if a channel connection
//              changed or the channel heartbeat has been lost.
//------------------------------------------------------------------------------

static void CBusResetToIdle ()
{
	uint8_t port, instanceIndex, supportMask;

	// we have to preserve the port and instance information
	port = pCbus->port;
	instanceIndex = pCbus->instanceIndex;
	supportMask = pCbus->supportMask;

	memset( pCbus, 0, sizeof( CbusInstanceData_t ));
    memset( &(pCbus->request), 0, sizeof( cbus_req_t ) * CBUS_MAX_COMMAND_QUEUE);

    // fill back the port and instance information
	pCbus->port = port;
	pCbus->instanceIndex = instanceIndex;
	pCbus->supportMask = supportMask;

}


//------------------------------------------------------------------------------
// Function:    CBusSendMscMsgCmd
// Description: Send MSC_MSG message to the specified CBUS channel (port)
//
// Parameters:  subCmd   - MSC_MSG cmd (RCP/RAP/ACP)
//              data     - MSC_MSG data
// Returns:     true     - successful queue/write
//              false    - write and/or queue failed
//------------------------------------------------------------------------------

bool_t CBusSendMscMsgCmd ( uint8_t subCmd, uint8_t data )
{
    cbus_req_t  req;

    // Send MSC_MSG command (Vendor Specific command)

    req.command     = MHL_MSC_MSG;
    req.msgData[0]  = subCmd;
    req.msgData[1]  = data;
	if( !(CBusWriteCommand(&req)) )
	{
		DEBUG_PRINT( MSG_ALWAYS, "Couldn't send MHL_MSC_MSG to peer\n");
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
// Function:    CBusDumpQueueContents
// Description: print the cbus queue contents
// Parameters:  none
// Returns:     void
//------------------------------------------------------------------------------
static void CBusDumpQueueContents ( void )
{
//    int_t   	queueIndex;
//    cbus_req_t 	temp;
//
//    DEBUG_PRINT( MSG_DBG, "SiiMhlRxDumpQueueContents:: ---- \n", pCbus->queueDepth );
//	for ( queueIndex = 0; queueIndex < CBUS_MAX_COMMAND_QUEUE; queueIndex++ )
//	{
//		temp = pCbus->request[ queueIndex];
//		if( temp.reqStatus != CBUS_REQ_IDLE )
//		{
//			DEBUG_PRINT( MSG_DBG, "index: %0x requestStatus: %0x, cmd: %0x, subcmd: %0x, data: %0x \n", queueIndex, temp.reqStatus, temp.command, temp.msgData[0], temp.msgData[1] );
//		}
//	}
}

//------------------------------------------------------------------------------
// Function:    CBusClearQueueEntry
// Description: clear a particular entry from the queue
// Parameters:  cmd - the one that needs to be removed
// Returns:     void
//------------------------------------------------------------------------------
static void CBusClearQueueEntry ( uint8_t cmd )
{
    int_t   	queueIndex;

//    DEBUG_PRINT( MSG_DBG, "CBusClearQueueEntry:: clearing cmd: %02X\n", cmd );
	for ( queueIndex = 0; queueIndex < CBUS_MAX_COMMAND_QUEUE; queueIndex++ )
	{
		if( pCbus->request[queueIndex].command == cmd )
		{
			memset( &(pCbus->request[queueIndex]), 0, sizeof( cbus_req_t ) );
			break;
		}
	}
}

//------------------------------------------------------------------------------
// Function:    CBusWriteCommand
// Description: Place a command in the CBUS message queue.
//
// Parameters:  pReq    - Pointer to a cbus_req_t structure containing the
//                        command to write
// Returns:     true    - successful queue/write
//              false   - write and/or queue failed
//------------------------------------------------------------------------------

static bool_t CBusWriteCommand ( cbus_req_t *pReq  )
{
    int_t   queueIndex, loop_count = 0;
    bool_t  success = false;

    /* Copy the request to the queue.   */

    if( SiiMhlRxCbusConnected() )
    {
    	queueIndex = (CH_ACTIVE_INDEX == ( CBUS_MAX_COMMAND_QUEUE - 1 )) ? 0 : (CH_ACTIVE_INDEX + 1);
		for ( loop_count = 0; loop_count < CBUS_MAX_COMMAND_QUEUE; loop_count++ )
		{
			if ( pCbus->request[ queueIndex].reqStatus == CBUS_REQ_IDLE )
			{
				// Found an idle queue entry, copy the request and set to pending.

				memcpy( &pCbus->request[ queueIndex], pReq, sizeof( cbus_req_t ));
				pCbus->request[ queueIndex].reqStatus = CBUS_REQ_PENDING;
				pCbus->queueDepth++;
				success = true;
//				DEBUG_PRINT( MSG_DBG, "\nCBUS:: CBusWriteCommand:: Queue Depth: %02X\n", pCbus->queueDepth );
				break;
			}
			queueIndex++;
			if( queueIndex == CBUS_MAX_COMMAND_QUEUE )
			{
				queueIndex = 0;
			}
		}

		if ( !success )
		{
			DEBUG_PRINT( MSG_DBG, "\nCBUS:: Queue Full\n" );
			for ( queueIndex = 0; queueIndex < CBUS_MAX_COMMAND_QUEUE; queueIndex++ )
			{
				DEBUG_PRINT( MSG_DBG, "\nCBUS:: CBusWriteCommand:: Queue Index: %02X, Cmd: %02X data: %02X\n",
						queueIndex, pCbus->request[ queueIndex].command, pCbus->request[ queueIndex].offsetData );
			}
		}
    }
    else
    {
    	DEBUG_PRINT( MSG_DBG, "CBus is not connected yet! MHL command could not be sent!\n" );
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    CBusWritePeersScratchpad
// Description: sends MHL write burst cmd
//------------------------------------------------------------------------------
static bool_t CBusWritePeersScratchpad()
{
	cbus_req_t req;

	req.command = MHL_WRITE_BURST;
	req.offsetData = pCbus->wbStartOffset;
	req.length = pCbus->wbLength;

	if( !(CBusWriteCommand(&req)) )
	{
		DEBUG_PRINT( MSG_ALWAYS, "Couldn't send Write Burst to peer\n" );
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
// Function:    CBusCheckInterruptStatus
// Description: If any interrupts on the specified channel are set, process them.
// Parameters:  none
// Returns:     success or error code
//------------------------------------------------------------------------------
static uint8_t CBusCheckInterruptStatus ( void )
{
    uint8_t result;
    uint8_t busStatus;
    uint8_t	temp;
    uint8_t data[2];
#if ( configSII_DEV_953x_PORTING == 1 )
    result = CBUS_SUCCESS;
#else
result = SUCCESS;
#endif 

    if ( SiiDrvCbusIntrFlagGet() )
    {
        if ( SiiDrvCbusVsDataGet( &data[0] ) )
        {
            CBusProcessSubCommand( &data[0] );
            pCbus->state |= CBUS_RECEIVED;
        }
        if ( SiiDrvCbusCmdRetDataGet( &pCbus->request[ CH_ACTIVE_INDEX ].msgData[0] ) )
        {
        	pCbus->state &= ~CBUS_SENT;

			/* A previous MSC sub-command has been acknowledged by the responder.   */
			pCbus->state |= CBUS_XFR_DONE;

        }
        else
        {
    		if( SiiDrvCbusDdcAbortReasonGet( &temp ) )
    		{
    			DEBUG_PRINT( MSG_DBG, "CBUS DDC ABORT happened, reason: %02X\n", temp );
    			pCbus->state &= ~CBUS_SENT;
    			pCbus->abortTimer = SiiOsTimerTotalElapsed();
    			result = ERROR_CBUS_ABORT;
    			pCbus->state |= CBUS_FAILED;
    		}


    		if ( SiiDrvCbusMscAbortTransReasonGet( &temp ) )
    		{
    			DEBUG_PRINT( MSG_DBG, "MSC CMD aborted (this device was a transmitter), reason: %02X\n", temp );
    			pCbus->state &= ~CBUS_SENT;
    			pCbus->abortTimer = SiiOsTimerTotalElapsed();
    			result = ERROR_CBUS_ABORT;
    			pCbus->state |= CBUS_FAILED;
    		}

    		if ( SiiDrvCbusMscAbortRcvrReasonGet( &temp ) )
    		{
    			DEBUG_PRINT( MSG_DBG, "MSC CMD aborted (this device was a receiver) , reason: %02X\n", temp );
    			pCbus->state &= ~CBUS_SENT;
    			pCbus->abortTimer = SiiOsTimerTotalElapsed();
    			result = ERROR_CBUS_ABORT;
    			pCbus->state |= CBUS_FAILED;
    		}
        }

        if ( SiiDrvCbusBusStatusGet( &busStatus ) )
        {
        	/* The connection change interrupt has been received.   */
			pCbus->connected = busStatus ? true : false;
        	DEBUG_PRINT( MSG_DBG, "\nCBUS:: ----Connection Change---- %s \n", pCbus->connected ? "Connected" : "Disconnected" );
			if( pCbus->connected )
			{
				pCbus->initTimer = SiiOsTimerTotalElapsed();
			}
			else
			{
				//set the cbus to idle
				CBusResetToIdle();
			}

			SiiMhlCbRxConnectChange(pCbus->connected);
        }

    	// request received from peer to write into scratchpad
    	if ( SiiDrvCbusReqWrtGet() )
    	{
    		if( !(pCbus->miscFlags & FLAGS_SCRATCHPAD_BUSY) )
			{
//				DEBUG_PRINT( MSG_DBG, "\n\ngranting peer's request to write scratchpad!!\n");
				CBusGrtWrt();
				pCbus->miscFlags |= FLAGS_SCRATCHPAD_BUSY;
			}
			else
			{
				DEBUG_PRINT( MSG_DBG, "\nGot request from peer for scratchpad write, "
						"couldn't grant request as the scartchpad is busy!!\n" );
			}
		}

    	// scratchpad write notification received from peer
    	if ( SiiDrvCbusScratchpadWrtnGet() )
		{
    		pCbus->miscFlags &= ~FLAGS_SCRATCHPAD_BUSY;
			// send it to app layer
			SiiMhlCbRxScratchpadWritten();
		}

    	// request to write into peer's scratchpad is granted
    	if ( SiiDrvCbusGrtWrtGet() )
    	{
//    		DEBUG_PRINT( MSG_DBG, "\n\npeer sent grtWrt!!\n");
    		if( pCbus->miscFlags & FLAGS_SCRATCHPAD_BUSY )
			{
				pCbus->reqWrtTimer = 0;
				CBusWritePeersScratchpad();
			}
        }

#if (MHL_EXTRA == ENABLE)
    	// request to send over 3D information
//    	if ( SiiDrvCbus3DReqGet() )
//    	{
//    		DEBUG_PRINT( MSG_DBG, "\n\npeer sent 3D data request!!\n");
//    		pCbus->serving3dVicReq = true;
//    		CBusSend3DInfo();
//        }
#endif
#if (MHL_20 == ENABLE)
    	// request to send over 3D information
    	if ( SiiDrvCbus3DReqGet() )
    	{
    		DEBUG_PRINT( MSG_DBG, "\n\n************* peer sent 3D data request!! ********************\n");
    		pCbus->sending_3D_info = true;
    		pCbus->cbus3Dstate = WB_3D_SENDING_VIC;
    		seq = 0;
    		CBusSend3DInfo();
        }
#endif

    }

    SiiDrvCbusIntrFlagSet();
    SiiDrvCbusInterruptStatusSet();
    return( result );
}

#if (MHL_EXTRA == ENABLE)
//------------------------------------------------------------------------------
// Function:    CalculateChecksum_3D
// Description: Calculate the checksum
// Parameters:  pData - pointer to data
//                     length - length of data
// Returns:     void
//------------------------------------------------------------------------------
static uint8_t CalculateChecksum_3D(uint8_t *pData, uint8_t length)
		{
    uint8_t i;
    uint8_t checksum = 0;

    for (i = 0; i < length; i++)
	{
        if (i==2) continue;
        checksum += pData[i];
	}

    return (0 - checksum);
}

//------------------------------------------------------------------------------
// Function:    setVICdata
// Description: set up VIC data for 3D info
// Parameters:  pReq - pointer to cbus_req_t
//                     seq - sequence number
// Returns:     void
//------------------------------------------------------------------------------
static void setVICdata( cbus_req_t *pReq, int seq )
				{
	int i;
	int num_ent = 0;
	uint8_t rest_ent;

	memset(&(pReq->msgData), 0, MHL_MAX_BUFFER_SIZE);

#if 0
	DEBUG_PRINT( MSG_DBG, "gMHL3D_VIC[0]\n");
	DEBUG_PRINT( MSG_DBG, "TOT : %02X\n", gMHL3D_VIC[0].tot);
	for (i=0; i<16; i++){
		DEBUG_PRINT( MSG_DBG, "VDI%02X_L : %02X\n", i, gMHL3D_VIC[0].vdi[i].vdi_l);
		DEBUG_PRINT( MSG_DBG, "VDI%02X_H : %02X\n", i, gMHL3D_VIC[0].vdi[i].vdi_h);
				}
	DEBUG_PRINT( MSG_DBG, "gMHL3D_VIC[1]\n");
	DEBUG_PRINT( MSG_DBG, "TOT : %02X\n", gMHL3D_VIC[1].tot);
	for (i=0; i<16; i++){
		DEBUG_PRINT( MSG_DBG, "VDI%02X_L : %02X\n", i, gMHL3D_VIC[1].vdi[i].vdi_l);
		DEBUG_PRINT( MSG_DBG, "VDI%02X_H : %02X\n", i, gMHL3D_VIC[1].vdi[i].vdi_h);
			}
#endif

#if ( MHL_20 == ENABLE )  /*Smith modifies: Fix building error when MHL20 == DISABLE*/
	pReq->msgData[0] = BURST_3D_VIC_H;
	pReq->msgData[1] = BURST_3D_VIC_L;

	pReq->msgData[3] = pMHL3D_VIC->tot;  // total entries of VICs
	if( pMHL3D_VIC->tot )
	{
		pReq->msgData[4] = seq + 1;
	}

	if (pReq->msgData[3] !=0){
		rest_ent = pMHL3D_VIC->tot - seq * 5;
		num_ent = (rest_ent > 5)? 5 : rest_ent;
		pReq->msgData[5] = num_ent;
		for (i=0; i<num_ent; i++){
			pReq->msgData[7 + i*2] = pMHL3D_VIC->vdi[seq * 5 + i].vdi_l;
		}
			}

	pReq->msgData[2] = CalculateChecksum_3D(pReq->msgData, num_ent*2 + 6);

	pReq->offsetData = 0x00;  // 0x40 will be added as offset 
	pReq->length = num_ent*2 + 6;

	for (i=0; i<num_ent*2+6; i++){
		DEBUG_PRINT( MSG_DBG, "CBUS 3D(VIC) WRITE_BURST - %02X : %02X\n", i, pReq->msgData[i] );
	}
#endif 	
}

//------------------------------------------------------------------------------
// Function:    setDTDdata
// Description: set up VIC data for 3D info
// Parameters:  pReq - pointer to cbus_req_t
//                     seq - sequence number
// Returns:     void
//------------------------------------------------------------------------------
static void setDTDdata( cbus_req_t *pReq, int seq )
				{
	int i;
	int num_ent = 0;
	uint8_t rest_ent;

	memset(&(pReq->msgData), 0, MHL_MAX_BUFFER_SIZE);

#if (MHL_20 == ENABLE)  /*Smith modifies*/
	pReq->msgData[0] = BURST_3D_DTD_H;
	pReq->msgData[1] = BURST_3D_DTD_L;

	pReq->msgData[3] = pMHL3D_DTD->tot;  // total entries of VICs
	if( pMHL3D_DTD->tot )
	{
		pReq->msgData[4] = seq + 1;
	}

	if (pReq->msgData[3] !=0){
		rest_ent = pMHL3D_DTD->tot - seq * 5;
		num_ent = (rest_ent > 5)? 5 : rest_ent;
		pReq->msgData[5] = num_ent;
		for (i=0; i<num_ent; i++){
			pReq->msgData[7 + i*2] = pMHL3D_DTD->vdi[seq * 5 + i].vdi_l;
				}
			}

	pReq->msgData[2] = CalculateChecksum_3D(pReq->msgData, num_ent*2 + 6);

	pReq->offsetData = 0x00;  // 0x40 will be added as offset 
	pReq->length = num_ent*2 + 6;

	for (i=0; i<num_ent*2+6; i++){
		DEBUG_PRINT( MSG_DBG, "CBUS 3D(DTD) WRITE_BURST - %02X : %02X\n", i, pReq->msgData[i] );
		}
#endif

}

	

//------------------------------------------------------------------------------
// Function:    CBusSend3DInfo
// Description: send over 3D write bursts
// Parameters:  none
// Returns:     void
//------------------------------------------------------------------------------
static void CBusSend3DInfo()
{
	cbus_req_t 	req;

#if (MHL_20 == ENABLE) /*Smith modifies*/

	switch (pCbus->cbus3Dstate){

		case WB_3D_IDLE:
			pCbus->sending_3D_info = false;
			break;

		case WB_3D_SENDING_VIC:  // granted, start sending VICs
			// if need to send more VICs, send REQ_WRT and go to STEP 2
			if (pMHL3D_VIC->tot == 0)
			{
				setVICdata(&req, seq);
				SiiMhlRxWritePeerScratchpad(req.offsetData, req.length, &(req.msgData[0]));
				pCbus->cbus3Dstate = WB_3D_SENDING_DTD;
			}
			else if ((pMHL3D_VIC->tot - seq * 5) > 0)
			{
				setVICdata(&req, seq);
				SiiMhlRxWritePeerScratchpad(req.offsetData, req.length, &(req.msgData[0]));
				seq++;
				if ((pMHL3D_VIC->tot - seq * 5) <= 0)
				{
					seq = 0;
					pCbus->cbus3Dstate = WB_3D_SENDING_DTD;
				}
				DEBUG_PRINT( MSG_DBG, "CBUS State: %02X seq:: %02X\n", pCbus->cbus3Dstate, seq );
			}

			break;

		case WB_3D_SENDING_DTD:  // granted, start sending DTDs
			if (pMHL3D_DTD->tot == 0)
			{
				setDTDdata(&req, seq);
				SiiMhlRxWritePeerScratchpad(req.offsetData, req.length, &(req.msgData[0]));
				pCbus->cbus3Dstate = WB_3D_IDLE;
			}
			else if ((pMHL3D_DTD->tot - seq * 5) > 0)
			{
				setDTDdata(&req, seq);
				SiiMhlRxWritePeerScratchpad(req.offsetData, req.length, &(req.msgData[0]));
				seq++;
				if ((pMHL3D_DTD->tot - seq*5) <= 0)
				{
					seq = 0;
					pCbus->cbus3Dstate = WB_3D_IDLE;
				}
			}
			DEBUG_PRINT( MSG_DBG, "CBUS State: %02X seq:: %02X\n", pCbus->cbus3Dstate, seq );
			break;
	}
#endif 

}
#endif

//------------------------------------------------------------------------------
// Function:    CBusSendDcapRdyMsg
// Description: Send a msg to peer informing the devive capability registers are
//				ready to be read.
// Returns:     TRUE    - success
//              FALSE   - failure
//------------------------------------------------------------------------------

static bool_t CBusSendDcapRdyMsg ()
{
	bool_t result = true;

	if( SiiMhlRxCbusConnected() )
	{
//		DEBUG_PRINT( MSG_DBG, ( "cbusSendDcapRdyMsg() Called!!\n"));
		pCbus->miscFlags |= FLAGS_SENT_DCAP_RDY;

		//send a msg to peer that the device capability registers are ready to be read.
		//set DCAP_RDY bit
		result = CBusSendDcapRdy();

		//set DCAP_CHG bit
		result = SiiMhlRxSendDevCapChange();
	}

	return result;
}

//------------------------------------------------------------------------------
// Function:    CBusSendMscCommand
// Description: sends general MHL commands
//------------------------------------------------------------------------------
/*static */bool_t CBusSendMscCommand(uint8_t cmd)
{
	cbus_req_t 	req;
	bool_t		success = true;

	req.command = cmd;

	switch(cmd)
	{
		case MHL_GET_STATE:
		case MHL_GET_VENDOR_ID:
		case MHL_SET_HPD:
		case MHL_CLR_HPD:
		case MHL_GET_SC1_ERRORCODE:
		case MHL_GET_DDC_ERRORCODE:
		case MHL_GET_MSC_ERRORCODE:
		case MHL_GET_SC3_ERRORCODE:

			if( !(success = CBusWriteCommand(&req)) )
			{
				DEBUG_PRINT( MSG_ALWAYS, "Couldn't send cmd: %02X to peer\n", cmd );
				return false;
			}
			break;

		default:
			DEBUG_PRINT( MSG_ALWAYS, "Invalid command send request!!\n", cmd );
			success = false;
	}
	return success;
}

//------------------------------------------------------------------------------
// Function:    CBusSetInt
// Description: write peer's status registers
//				regOffset - peer's register offset
//				regBit - bit to be set
//------------------------------------------------------------------------------
static bool_t CBusSetInt ( uint8_t regOffset, uint8_t regBit )
{
	cbus_req_t req;

	req.command = MHL_SET_INT;
	req.offsetData = regOffset;
	req.msgData[0] = regBit;

	if( SiiMhlRxCbusConnected() )
	{
		if( !(CBusWriteCommand(&req)) )
		{
			DEBUG_PRINT( MSG_ALWAYS, "Couldn't send MHL_SET_INT to peer\n" );
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------
// Function:    CBusSendDscrChange
//------------------------------------------------------------------------------
static bool_t CBusSendDscrChange ()
{
	return ( CBusSetInt(0x00, BIT1) );
}

//------------------------------------------------------------------------------
// Function:    CBusReqWrt
//------------------------------------------------------------------------------
static bool_t CBusReqWrt ()
{
	return ( CBusSetInt(0x00, BIT2) );
}

//------------------------------------------------------------------------------
// Function:    CBusGrtWrt
// Description:
//------------------------------------------------------------------------------
static bool_t CBusGrtWrt ()
{
	return ( CBusSetInt(0x00, BIT3) );
}

//------------------------------------------------------------------------------
// Function:    CBusWriteStatus
// Description: write peer's status registers
// Parameters:  regOffset - peer's register offset
//				value - value to be written
//------------------------------------------------------------------------------
static bool_t CBusWriteStatus ( uint8_t regOffset, uint8_t value )
{
	cbus_req_t req;

	req.command = MHL_WRITE_STAT;
	req.offsetData = regOffset;
	req.msgData[0] = value;

	if( !(CBusWriteCommand(&req)) )
	{
		DEBUG_PRINT( MSG_ALWAYS, "Couldn't send MHL_WRITE_STAT to peer\n" );
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
// Function:    CBusSendDcapRdy
//------------------------------------------------------------------------------
static bool_t CBusSendDcapRdy ()
{
	return ( CBusWriteStatus(0x00, BIT0) );

}

static bool_t CBusTimerExpired(clock_time_t *pTimer, uint16_t waitTime)
{
	if( (*pTimer > 0) && ((SiiOsTimerTotalElapsed() - *pTimer ) > waitTime) )
	{
//		DEBUG_PRINT( MSG_DBG, "\n*pTimer = %02X SiiOsTimerTotalElapsed(): %02X\n", *pTimer, SiiOsTimerTotalElapsed());
		*pTimer = 0;
		return true;
	}
	else
	{
		return false;
	}

}

// This is to check all the Timers
static void CBusChkTimers ()
{
	CBusTimerExpired(&(pCbus->hpdWaitTimer), CBUS_HPD_WAIT_TIMER);
	CBusTimerExpired(&(pCbus->abortTimer), CBUS_ABORT_TIMER);
	CBusTimerExpired(&(pCbus->initTimer), CBUS_INIT_TIMER);
	CBusTimerExpired(&(pCbus->rcpeRcpkGapTimer), CBUS_RCPE_RCPK_GAP_TIMER);

	if( CBusTimerExpired(&(pCbus->reqWrtTimer), CBUS_WB_REQ_TIMER) )
	{
		CBusClearQueueEntry( MHL_WRITE_BURST );
		pCbus->miscFlags &= ~FLAGS_SCRATCHPAD_BUSY;
		DEBUG_PRINT( MSG_DBG, "GrtWrt interrupt did not come from peer within the timeout limit!! \n");
	}

	if( CBusTimerExpired(&(pCbus->waitTimer), CBUS_MSG_RSP_TIMER) )
	{
		pCbus->state |= CBUS_FAILED;
		DEBUG_PRINT( MSG_DBG, "\npCbus->waitTimer = %02X \n", pCbus->waitTimer);
	}

	if( CBusTimerExpired(&(pCbus->rcpRapAbortTimer), CBUS_RCP_RCP_ABORT_TIMER) )
	{
		pCbus->state |= CBUS_FAILED;
		DEBUG_PRINT( MSG_DBG, "Ack didn't come back for MSC_MSG cmd for %02X milliseconds.\n", pCbus->rcpRapAbortTimer);
	}

}

#if (MHL_20 == ENABLE)
static void CBus3DInstanceSet( uint_t instanceIndex )
{
	pMHL3D_VIC = &gMHL3D_VIC[instanceIndex];
	pMHL3D_DTD = &gMHL3D_DTD[instanceIndex];
}

static void CBus3DCheckInstance ( uint_t instanceIndex )
{
#if INC_RTPI
	uint_t port;
	uint8_t tx_video_src;
#endif
	
	if (!SiiDrvSwitchMatrixModeEnableGet())
	{
		// InstaPort mode
		CBus3DInstanceSet( 0 );
		return;
	}
#if INC_RTPI
	// Matrix mode
	port = (instanceIndex == 0)? 0 : 5;
	tx_video_src = SiiDrvSwitchStateGet(SiiSwitch_TX_VIDEO_SRC);
	if (SiiDrvSwitchStateGet(SiiSwitch_SELECTED_PORT) == port){
		// port is main
		if ((tx_video_src & 0x0c) == 0x00) CBus3DInstanceSet(0);  // Tx0 is main
		else if ((tx_video_src & 0x30) == 0x00) CBus3DInstanceSet(1);  // Tx1 is main
	}
	else if (SiiDrvSwitchStateGet(SiiSwitch_SELECTED_PORT_SUB) == port){
		// port is sub
		if ((tx_video_src & 0x0c) == 0x04) CBus3DInstanceSet(0);  // Tx0 is sub
		else if ((tx_video_src & 0x30) == 0x10) CBus3DInstanceSet(1);  // Tx1 is sub
	}
#endif
}
#endif
