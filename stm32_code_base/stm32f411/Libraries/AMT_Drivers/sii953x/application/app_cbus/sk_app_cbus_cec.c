//***************************************************************************
//!file     sk_app_cbus_cec.c
//!brief    CBUS-CEC interface
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_app_cbus.h"
#include <string.h>

#if INC_CEC && INC_CBUS

#include "si_cec_component.h"

//------------------------------------------------------------------------------
//! @brief  If the last CEC message received was a USER_CONTROL_PRESSED and
//!         the key received matches the CBUS RCP key ID (msgData), we must
//!         send a feature abort back to the CEC device that sent the key.
//! @param[in]  msgData - RCP keyID value
//------------------------------------------------------------------------------
void SkAppCbusCecFeatureAbort ( uint8_t msgData )
{
    // Compare the last CEC message sent to the message being denied.
    if (( pAppCbus->lastRcpCecMessageOpcode == CECOP_USER_CONTROL_PRESSED) &&
        ( pAppCbus->lastRcpCecMessageData == msgData ))
    {
        pAppCbus->lastRcpCecMessageOpcode = 0;  // Kill the last message sent

        // The last CEC message received was not accepted by the target,
        // so we must feature abort it.
        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
        SiiCecFeatureAbortSendEx(
            pAppCbus->lastRcpCecMessageOpcode, CECAR_INVALID_OPERAND,
            pAppCbus->cecLa, pAppCbus->lastRcpCecSourceLa );
    }
}

//------------------------------------------------------------------------------
//! @brief      Called from the CBUS RAP message parser to forward as an
//!             appropriate CEC command.
//! @param[in]  rapData - RAP action value
//------------------------------------------------------------------------------
void SkAppCbusRapToCec ( uint8_t rapData )
{
    // We have received a CBUS RAP message from the currently selected
    // CBUS instance, so pass it on to the CEC world

    SkAppCecInstanceSet( CEC_INSTANCE_AVR );    // All CBUS devices are on this CEC instance
    switch ( rapData )
    {
        case MHL_RAP_CONTENT_ON:
            // Tell the CEC world that we are going active
            SiiCecSendActiveSource( pAppCbus->cecLa, pAppCbus->cecPa );
            pAppCbus->cecIsActiveSource = true;
            break;

        case MHL_RAP_CONTENT_OFF:
            // Tell the CEC world that we are going inactive
            SiiCecSendInactiveSource( pAppCbus->cecLa, pAppCbus->cecPa );
            pAppCbus->cecIsActiveSource = false;
            break;
        default:
            break;
    }
}

//------------------------------------------------------------------------------
//! @brief      Called from the CBUS RAPK message parser to handle the result
//!             of sending a RAP message to the device.
//! @param[in]  rapData - RAPK status
//------------------------------------------------------------------------------
void SkAppCbusRapkToCec ( uint8_t rapData )
{
    uint8_t srcAddr, opCode;

    // We have received the result of a CBUS RAP message from the currently selected
    // CBUS instance, so pass it on to the CEC world

    SkAppCecInstanceSet( CEC_INSTANCE_AVR );    // All CBUS devices are on this CEC instance
    srcAddr = pAppCbus->lastRcpCecSourceLa;
    opCode  = pAppCbus->lastRcpCecMessageOpcode;

    switch ( rapData )
    {
        case MHL_MSC_MSG_RAP_NO_ERROR:
            // RAP command successful, send appropriate CEC response.
            if ( pAppCbus->lastRcpCecMessageOpcode == CECOP_SET_STREAM_PATH )
            {
                // OK, this must be a RAPK for a CONTENT_ON request message.
                // We can send an ACTIVE_SOURCE to the CEC bus.
                SiiCecSendActiveSource( pAppCbus->cecLa, pAppCbus->cecPa );
                pAppCbus->cecIsActiveSource = true;
            }
            break;

        case MHL_MSC_MSG_RAP_UNRECOGNIZED_ACT_CODE:
            SiiCecFeatureAbortSendEx( opCode, CECAR_UNRECOG_OPCODE, pAppCbus->cecLa, srcAddr );
            break;
        case MHL_MSC_MSG_RAP_UNSUPPORTED_ACT_CODE:
            SiiCecFeatureAbortSendEx( opCode, CECAR_CANT_PROVIDE_SOURCE, pAppCbus->cecLa, srcAddr );
            break;
        case MHL_MSC_MSG_RAP_RESPONDER_BUSY:
            SiiCecFeatureAbortSendEx( opCode, CECAR_NOT_CORRECT_MODE, pAppCbus->cecLa, srcAddr );
            break;
        case MHL_MSC_MSG_RAP_OTHER_ERROR:           // NOT RAP code: Hardware or Queue problem caused error
        default:
            SiiCecFeatureAbortSendEx( opCode, CECAR_UNABLE_TO_DETERMINE, pAppCbus->cecLa, srcAddr );
            break;
    }
}

//------------------------------------------------------------------------------
//! @brief      Called from the CBUS RCP message parser to transfer a CBUS RCP
//!             keyID to CEC as a USER_CONTROL_PRESSED or USER_CONTROL_RELEASED
//!             key message.
//! @param[in]  rcpData - RCP keyID value
//------------------------------------------------------------------------------
void SkAppCbusRcpToCec ( uint8_t rcpData )
{
    // The rcpData has already been validated by the CBUS component before
    // passing to us, so just send it to the CEC bus.

    SkAppCecInstanceSet( CEC_INSTANCE_AVR );    // All CBUS devices are on this CEC instance
    pAppCbus->lastRcpCecSourceLa = SiiCecPortToLA( pAppCbus->port );
    if ( rcpData & BIT7 )
    {
        SiiCecSendUserControlReleased();
    }
    else
    {
        SiiCecSendUserControlPressed( rcpData & ~BIT7, pAppCbus->cecLa, SiiCecPortToLA( pAppCbus->port ));
    }
}

//------------------------------------------------------------------------------
//! @brief      If specified logical address is a connected MHL port, return
//!             true.  As a side effect, the port instance is selected.
//! @param[in]  logicalAddr - CEC logical address
//! @return     bool_t      - true if LA represents a connected MHL channel
//------------------------------------------------------------------------------
bool_t SkAppCbusSetLogicalAddressInstance ( int logicalAddr )
{
    int instanceIndex;

    for ( instanceIndex = 0; instanceIndex < MHL_MAX_CHANNELS; instanceIndex++ )
    {
        if ( appCbus[ instanceIndex].cecLa == logicalAddr )
        {
            SkAppCbusInstanceSet( instanceIndex );
            if ( SiiMhlRxCbusConnected())
            {
                return( true );
            }
        }
    }

    return( false );
}

//------------------------------------------------------------------------------
//! @brief  Called from the CEC message parser to transfer a CBUS CONTENT_ON
//!         or MHL_RAP_CONTENT_OFF RAP action message to CBUS.
//! @param[in]  pMsg    CEC message data
//------------------------------------------------------------------------------
static bool_t CbusCecToCbusRap( SiiCpiData_t *pMsg )
{
    SiiMhlMscMsgRapErrorCodes_t sendResult  = MHL_MSC_MSG_RAP_OTHER_ERROR;

    // Both messages accepted by this function are broadcast, so we can't
    // determine if the message was for us that way.
    // If the opcode is ACTIVE_SOURCE, then if the connected source is
    // MHL, we need to shut it off.
    if ( pMsg->opcode == CECOP_ACTIVE_SOURCE )
    {
        if ( SkAppSourceIsConnectedCbusPort( app.currentSource[app.currentZone] ))
        {
            sendResult = SiiMhlRxSendRAPCmd( MHL_RAP_CONTENT_OFF );
        }
    }
    // If the opcode is SET_STREAM_PATH, then we send the message to
    // any connected CBUS device that has the matching physical address
    else if ( pMsg->opcode == CECOP_SET_STREAM_PATH )
    {
        int         instanceIndex;
        uint16_t    physicalAddr;

        physicalAddr = pMsg->args[0];
        physicalAddr = (physicalAddr << 8) | pMsg->args[1];

        // Check both possible MHL channels.
        for ( instanceIndex = 0; instanceIndex < MHL_MAX_CHANNELS; instanceIndex++ )
        {
            if ( appCbus[ instanceIndex].cecPa == physicalAddr )
            {
                // This CBUS device has an assigned CEC physical address that
                // matches the stream path, so if it is connected, send it
                // a CONTENT ON command.  If an RAPK is received, the RAPK
                // handler will issue a CEC ACTIVE_SOURCE message.
                SkAppCbusInstanceSet( instanceIndex );
                if ( SiiMhlRxCbusConnected())
                {
                    sendResult = SiiMhlRxSendRAPCmd( MHL_RAP_CONTENT_ON );

                    // Save some info for the RAPK handler.
                    pAppCbus->lastRcpCecSourceLa        = CEC_LOGADDR_UNREGORBC;
                    pAppCbus->lastRcpCecMessageOpcode   = pMsg->opcode;
                    pAppCbus->lastRcpCecMessageData     = pMsg->args[0];
                }
            }
        }
    }

    // Cannot Feature Abort error responses for broadcast CEC messages

    return( sendResult == MHL_MSC_MSG_RAP_NO_ERROR );
}

//------------------------------------------------------------------------------
//! @brief  Called from the CEC message parser to transfer a
//!         USER_CONTROL_PRESSED or USER_CONTROL_RELEASED key message to
//!         CBUS as an RCP keyID.
//! @param[in]  pMsg    CEC message data
//------------------------------------------------------------------------------
static void CbusCecToCbusRcp( SiiCpiData_t *pMsg )
{
    bool_t  sendSuccess = true;
    uint8_t srcAddr     = GET_CEC_SRC( pMsg->srcDestAddr );
    uint8_t destAddr    = GET_CEC_DEST( pMsg->srcDestAddr );

    // If CEC dest is a valid, connected CBUS device, process the message
    if ( SkAppCbusSetLogicalAddressInstance( destAddr ))
    {
    	switch(pMsg->opcode)
    	{
    	case CECOP_USER_CONTROL_RELEASED:
    		break;
    	case CECOP_USER_CONTROL_PRESSED:
    		pAppCbus->lastRcpCecMessageData = pMsg->args[0];
    		sendSuccess = false;
    		break;
    	case CECOP_PLAY:
           	switch(pMsg->args[0])
            	{
            	case CEC_PLAY_FORWARD:
            		pAppCbus->lastRcpCecMessageData     = MHL_RCP_CMD_PLAY;
            		break;
            	case CEC_PLAY_STILL:
            		pAppCbus->lastRcpCecMessageData     = MHL_RCP_CMD_PAUSE;
            		break;
            	}
    		break;
    	case CECOP_DECK_CONTROL:
    		pAppCbus->lastRcpCecMessageData     = MHL_RCP_CMD_STOP;
    		break;
    	case CECOP_VENDOR_REMOTE_BUTTON_DOWN:
    		pAppCbus->lastRcpCecMessageData     = MHL_RCP_CMD_EXIT;
    		break;
/*
        case CECOP_GIVE_DEVICE_VENDOR_ID:
        	DEBUG_PRINT(MSG_ALWAYS,"\n TRYING TO GET VENDOR ID\n");
        	pAppCbus->lastRcpCecMessageData     = MHL_GET_VENDOR_ID;
        	break;
*/
    	default:
    		sendSuccess = false;
    		break;
    	}

        /*if ( pMsg->opcode == CECOP_USER_CONTROL_RELEASED )
        {
            // CEC does not include the key ID in the CECOP_USER_CONTROL_RELEASED message,
            // so we use the key ID from the last CEC message sent, which should have been
            // the CECOP_USER_CONTROL_PRESSED message.  If not, don't bother to send it
            if ( pAppCbus->lastRcpCecMessageOpcode == CECOP_USER_CONTROL_PRESSED )
            {
                pMsg->args[0] = pAppCbus->lastRcpCecMessageData | BIT7; // Add the key release bit
            }
            else
            {
                sendSuccess = false;
            }
        }
        pAppCbus->lastRcpCecSourceLa        = srcAddr;
        pAppCbus->lastRcpCecMessageOpcode   = pMsg->opcode;
        pAppCbus->lastRcpCecMessageData     = pMsg->args[0];

        	*/
        // Send the RCP key
        if ( sendSuccess )
        {
            sendSuccess = SiiMhlRxSendRCPCmd( pAppCbus->lastRcpCecMessageData );
        }
    }
    else
    {
        sendSuccess = false;
    }

    // Tell CEC device if the CBUS device didn't like the message, was not connected,
    // or was not a CBUS device at all.
    if ( !sendSuccess )
    {
        // Tell the CEC requester it didn't work
        SiiCecFeatureAbortSendEx( pMsg->opcode, CECAR_REFUSED, destAddr, srcAddr );
    }
}

//------------------------------------------------------------------------------
//! @brief  Send a Remote Control key to the selected port if it is MHL.
//!         Does not validate key, since we don't know what we're attached to.
//!         The MHL device will perform its own validation.
//! @param[in]  key     Remote Control (RC5 protocol) key
//------------------------------------------------------------------------------
void SkAppCbusSendRcKey( RC5Commands_t key )
{
    uint8_t ceaKey;

    ceaKey = SkAppIrConvertToCea931( key );
    if ( ceaKey != 0xFF )
    {
        // If the specified port is MHL, the correct instance will also be selected
        if ( SkAppSourceIsConnectedCbusPort( app.currentSource[app.currentZone] ))
        {
            SiiMhlRxSendRCPCmd( ceaKey );
        }
    }
}

void SkAppCbusCecSendVendorId(uint8_t instanceIndex, uint8_t *vendorId )
{
	SiiCecSendVendorId(pAppCbus[instanceIndex].cecLa,CEC_LOGADDR_UNREGORBC,vendorId);
}

extern bool_t CBusSendMscCommand(uint8_t cmd);

//------------------------------------------------------------------------------
//! @brief      Filter CEC messages and pass on to the MHL device if appropriate.
//! @param[in]  pMsg    - CEC message data structure
//! @return     true if message was processed by this handler
//------------------------------------------------------------------------------
bool_t SkAppCbusCecRxMsgFilter ( SiiCpiData_t *pMsg,  SiiCecLogicalAddresses_t virtualDevLA)
{
    bool_t  processedMsg, isDirectAddressed;

    isDirectAddressed = (bool_t)!(GET_CEC_DEST(pMsg->srcDestAddr) == CEC_LOGADDR_UNREGORBC );

    processedMsg = true;
    DEBUG_PRINT(MSG_ALWAYS,"CBUS CEC MSG HANDLER opcode = 0x%x, with LA = %d\n", pMsg->opcode,virtualDevLA);
    SkAppCbusInstanceSet( 0 );
    if(pAppCbus->cecLa != virtualDevLA)
    {
        SkAppCbusInstanceSet( 1 );
        if(pAppCbus->cecLa != virtualDevLA)
        {
        	DEBUG_PRINT(MSG_ALWAYS,"DID NOT FIND VIRTUAL DEVICE CBUS INSTANCE TO ADDRESS OPCODE\n");
        	return false;
        }
    }
    pAppCbus->cecPa = SiiCecAdjacentPhysAddrGet( SiiCecGetDevicePA(), false, pAppCbus->port );
    SiiCecUpdateLogicalDeviceInfo( pAppCbus->cecLa | BIT7, pAppCbus->cecPa, false );

    switch ( pMsg->opcode )
    {
        case CECOP_ACTIVE_SOURCE:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
                // Someone else wants to be the active source, send CONTENT_OFF to the
                // CBUS device if it is currently the active source.
                if ( pAppCbus->cecIsActiveSource )
                {
                    CbusCecToCbusRap( pMsg );
                    pAppCbus->cecIsActiveSource = false;
                }
            }
            break;

        case CECOP_SET_STREAM_PATH:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
                pAppCec->setStreamPathReceived = true;
                pAppCec->setStreamPathArg0 = pMsg->args[0];
                pAppCec->setStreamPathArg1 = pMsg->args[1];

                // The DTV wants to set the active source.  If it is one of
                // the CBUS devices, this function will turn it on.
                if ( CbusCecToCbusRap( pMsg ))
                {
                    pAppCbus->cecIsActiveSource = true;
                    pAppCec->setStreamPathReceived = false;
                }
            }
            break;

        case CECOP_REQUEST_ACTIVE_SOURCE:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
                // If we are the active source, tell the world (our CEC world, anyway).
                // No need to notify the CBUS device of anything
                if ( pAppCbus->cecIsActiveSource )
                {
                    SiiCecSendActiveSource( pAppCbus->cecLa, pAppCbus->cecPa );
                }
            }
            break;

        case CECOP_USER_CONTROL_PRESSED:
        case CECOP_USER_CONTROL_RELEASED:
        case CECOP_PLAY:
        case CECOP_DECK_CONTROL:
            if ( !isDirectAddressed )       break;  // Ignore as broadcast message

            CbusCecToCbusRcp( pMsg );
            processedMsg = true;                   // Let others parse this message also
            break;

        case CECOP_FEATURE_ABORT:
            break;

        // The following messages are normally handled by the CEC FirstMessageHandler,
        // but only for the local device, so we have to handle them for our virtual devices here.
        case CECOP_GIVE_PHYSICAL_ADDRESS:
            if ( isDirectAddressed )                // Ignore as broadcast message
            {
                SiiCecSendReportPhysicalAddress( pAppCbus->cecLa, pAppCbus->cecPa );
            }
            else    processedMsg = false;           // Let others parse this message also
            break;

        case CECOP_STANDBY:                         // Direct and Broadcast
            // Don't need to do anything here, when the DTV goes into standby, the
            // CBUS connection will dropped causing the desired result
            break;
        case CECOP_GIVE_DEVICE_POWER_STATUS:
            if ( isDirectAddressed )                // Ignore as broadcast message
            {
                //If we're even receiving this message, the MHL device power is on
                SiiCecSendReportPowerStatus( pAppCbus->cecLa, GET_CEC_SRC(pMsg->srcDestAddr), CEC_POWERSTATUS_ON );
            }
            else    processedMsg = false;           // Let others parse this message also
            break;
        case CECOP_REPORT_POWER_STATUS:     // Someone sent us their power state.
            break;
        case CECOP_GIVE_DECK_STATUS:
        	if(isDirectAddressed)
        	{
        		SiiCecSendDeckStatus(pAppCbus->cecLa, GET_CEC_SRC(pMsg->srcDestAddr), pAppCbus->deckStatus );
        	}
        	break;
        case CECOP_MENU_REQUEST:
        	if(isDirectAddressed)
        	{
        		SiiCecSendMenuStatus(pAppCbus->cecLa, GET_CEC_SRC(pMsg->srcDestAddr), CEC_MENUSTATE_ACTIVATED );
        	}
        	break;
        case CECOP_GIVE_OSD_NAME:
        	SiiCecSetOsdName(pAppCbus->osdName);
        	processedMsg = false;
        	break;

        case CECOP_GIVE_DEVICE_VENDOR_ID:
        	if ( !isDirectAddressed )       break;  // Ignore as broadcast message

        	CBusSendMscCommand(MHL_GET_VENDOR_ID);


        	processedMsg = true;
        	break;


        default:
			processedMsg = false;           // Let others parse this message also
            break;
    }

    return( processedMsg );
}

//------------------------------------------------------------------------------
//! @brief      Check for an available CEC LA from the CEC component and if
//!             found, assign it and a CEC physical address to the requesting
//!             CBUS device.
//! @param[in]  isComplete  - true - check for available LA, false, do nothing.
//------------------------------------------------------------------------------
void SkAppCbusAssignCecDeviceAddress( bool_t isComplete )
{
    int     cbusInstance;
    uint8_t cbusCecLa;

    // Don't care about not complete notification, and
    // the CBUS devices are attached only to CEC instance 0
    if (( !isComplete ) || ( pAppCec->instanceIndex != 0 ))
    {
        return;
    }

    // We may or may not be in the same CBUS instance as the
    // CEC enumeration request, so just check all CBUS instances,
    // since only one request can be run at a time.
    for ( cbusInstance = 0; cbusInstance < MHL_MAX_CHANNELS; cbusInstance++ )
    {
        SkAppCbusInstanceSet( cbusInstance );
        if ( pAppCbus->cecWaitingForEnum )
        {
            cbusCecLa = SiiCecGetAvailableLa( appCecSourceLaList+1 );
            if ( cbusCecLa < CEC_LOGADDR_UNREGORBC )
            {
            	DEBUG_PRINT(MSG_ALWAYS,"\n\n\n\nEnumerating CBUS os virtual CEC device with logical address :%d for cbusinstance %d\n\n",cbusCecLa,cbusInstance);
                // Set the logical and physical device addresses for this port
                pAppCbus->cecLa = cbusCecLa;
                pAppCbus->cecPa = SiiCecAdjacentPhysAddrGet( SiiCecGetDevicePA(), false, pAppCbus->port );
                pAppCbus->cecWaitingForEnum = false;

                SiiCecSetVirtualDevLA( cbusInstance, cbusCecLa );
                // Tell the CEC hardware to respond to this logical address
                SiiDrvCpiAddLogicalAddr( true, cbusCecLa );

                // Tell the CEC component about the device (adding BIT7 to the logical
                // address tells SiiCecUpdateLogicalDeviceInfo() this is a virtual device).
                SiiCecUpdateLogicalDeviceInfo( pAppCbus->cecLa | BIT7, pAppCbus->cecPa, false );

                memcpy(pAppCbus->osdName, cbusInstance?MHL_DEV2_OSD_NAME:MHL_DEV1_OSD_NAME, strlen(cbusInstance?MHL_DEV2_OSD_NAME:MHL_DEV1_OSD_NAME));

                // If SET_STREAM_PATH was received but not handled, it probably came from
                // the command that woke us up.  If the physical address attached to that
                // command matches our physical address, follow through with the
                // SET_STREAM_PATH CBUS action.
                if ( pAppCec->setStreamPathReceived )
                {
                    SiiCpiData_t cecFrame;

                    // Set up fake CEC message with parameters needed for SET_STREAM_PATH
                    cecFrame.opcode = CECOP_SET_STREAM_PATH;
                    cecFrame.args[0] = pAppCec->setStreamPathArg0;
                    cecFrame.args[1] = pAppCec->setStreamPathArg1;
                    if ( CbusCecToCbusRap( &cecFrame ))
                    {
                        pAppCbus->cecIsActiveSource = true;
                        pAppCec->setStreamPathReceived = false;
                    }
                }
            }
        }
    }

}

//------------------------------------------------------------------------------
//! @brief      Handle the CEC aspects of a CBUS connect change notification.
//!             If connected, enumerate available CEC playback devices for
//!             later assignment to this device.  If not connected, remove the
//!             previously assigned CEC device from the CEC watch list
//! @param[in]  connected  - true/false
//------------------------------------------------------------------------------
void SkAppCbusConnectChangeCecUpdate( bool_t connected )
{
    // If connected, we need to assign a CEC logical address, so ping for
    // available addresses
    if ( connected )
    {
        // Start an enumeration task to ensure that all playback devices are enumerated
        // so that we know what's already there. When the enumeration is complete,
        // another callback will assign the CEC logical device address
        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
        SiiCecEnumerateDevices( appCecSourceLaList );
        pAppCbus->cecWaitingForEnum = true;
    }
    // If disconnected, de-assign the CEC logical address.
    else
    {
        // Stop the CEC component from recognizing the CEC LA associated with this device.
        SkAppCecInstanceSet( CEC_INSTANCE_AVR );
        if(SiiCecRemoveVirtualDevice(pAppCbus->cecLa))
        {
            SiiDrvCpiAddLogicalAddr( false, pAppCbus->cecLa );
        }

        // Passing a bad physical address will take this logical address out of the
        // internal assigned LA table.
        SiiCecUpdateLogicalDeviceInfo( pAppCbus->cecLa, 0xFFFF, false );
        pAppCbus->cecLa = CEC_LOGADDR_UNREGORBC;
    }
}

#endif  // #if INC_CEC && INC_CBUS
