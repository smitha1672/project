/******************************************************************************/
//!file     si_drv_cbus.c
//!brief    SiI9687 CBUS Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
/******************************************************************************/

#include "string.h"
#include "si_osal_timer.h"
//#include "si_scheduler.h"
#include "si_drv_cbus_internal.h"
#include "si_cbus_enums.h"
#include "si_drv_internal.h"

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusRegisterGet
// Description: Returns the CBus register value
// Parameters:  regAddr - register address, channel
// Returns:     register value
//------------------------------------------------------------------------------

uint8_t  SiiDrvCbusRegisterGet ( uint16_t regAddr )
{
	uint8_t value;

	value = SiiRegRead( regAddr );
	return ( value );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusRegisterSet
// Description: Sets the CBus register value
// Parameters:  regAddr - register address
//------------------------------------------------------------------------------
void  SiiDrvCbusRegisterSet ( uint8_t regAddr , uint8_t value )
{
	SiiRegWrite( regAddr, value );
}


//------------------------------------------------------------------------------
// Function:    SiiDrvCbusInterruptStatusGet
// Description: Returns the last Interrupt Status data retrieved by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer1 (2 byte).
// Returns:     void
//------------------------------------------------------------------------------
void  SiiDrvCbusInterruptStatusGet ( uint8_t *pData )
{
    *pData++ = pDrvCbus->interruptStatus0;
    *pData = pDrvCbus->interruptStatus1;
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusInterruptStatusSet
// Description: Clears the interrupt variable
//------------------------------------------------------------------------------
void  SiiDrvCbusInterruptStatusSet ()
{
    pDrvCbus->interruptStatus0 = 0;
    pDrvCbus->interruptStatus1 = 0;
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusIntrFlagGet
// Description: Returns interrupt flag
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusIntrFlagGet()
{
	if ( pDrvCbus->statusFlags & SiiCBUS_INT )
	{
		pDrvCbus->statusFlags &= ~SiiCBUS_INT;
		return( true );
	}
	return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusIntrFlagSet
// Description: reset interrupt flags
//------------------------------------------------------------------------------
void  SiiDrvCbusIntrFlagSet()
{
	if ( pDrvCbus->statusFlags )
	{
		pDrvCbus->statusFlags = 0;
	}
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusNackFromPeerGet
// Description: Returns the last MSC Abort reason received by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer (1 byte).
// Returns:     true if a new MSC Abort reason data was available, false if not.
//              pData - Destination for MSC Abort reason data.
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusNackFromPeerGet ()
{
    if ( pDrvCbus->statusFlags & SiiCBUS_NACK_RECEIVED_FM_PEER )
    {
        pDrvCbus->statusFlags &= ~SiiCBUS_NACK_RECEIVED_FM_PEER;
        return( true );
    }
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusDevCapChangedGet
// Description: Returns if the peer's device capability values are changed
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusDevCapChangedGet ()
{
    if ( pDrvCbus->statusFlags & SiiCBUS_DCAP_CHG_RECEIVED_FM_PEER )
    {
        pDrvCbus->statusFlags &= ~SiiCBUS_DCAP_CHG_RECEIVED_FM_PEER;
        return( true );
    }
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusScratchpadWrtnGet
// Description: Returns if the peer has written the scratchpad
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusScratchpadWrtnGet ()
{
    if (pDrvCbus->statusFlags & SiiCBUS_SCRATCHPAD_WRITTEN_BY_PEER)
    {
        pDrvCbus->statusFlags &= ~SiiCBUS_SCRATCHPAD_WRITTEN_BY_PEER;
        return (true);
    }
    return (false);
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusReqWrtGet
// Description: Returns if the peer is requesting for scratchpad write permission
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusReqWrtGet ()
{
    if ( pDrvCbus->statusFlags & SiiCBUS_REQ_WRT_RECEIVED_FM_PEER )
    {
        pDrvCbus->statusFlags &= ~SiiCBUS_REQ_WRT_RECEIVED_FM_PEER;
        return( true );
    }
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusGrtWrtGet
// Description: Returns if the peer is requesting for scratchpad write permission
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusGrtWrtGet ()
{
    if ( pDrvCbus->statusFlags & SiiCBUS_GRT_WRT_RECEIVED_FM_PEER )
    {
        pDrvCbus->statusFlags &= ~SiiCBUS_GRT_WRT_RECEIVED_FM_PEER;
        return( true );
    }
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbus3DReqGet
// Description: Returns if the peer is requesting for 3D information
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t  SiiDrvCbus3DReqGet ()
{
    if ( pDrvCbus->statusFlags & SiiCBUS_3D_REQ_RECEIVED_FM_PEER )
    {
        pDrvCbus->statusFlags &= ~SiiCBUS_3D_REQ_RECEIVED_FM_PEER;
        return( true );
    }
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusVsDataGet
// Description: Returns the last VS cmd and data bytes retrieved by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer (2 bytes).
// Returns:     pData[0] - VS_CMD value
//              pData[1] - VS_DATA value
//------------------------------------------------------------------------------
bool_t SiiDrvCbusVsDataGet ( uint8_t *pData )
{
    if ( pDrvCbus->statusFlags & SiiCBUS_MSC_MSG_RCVD )
	{
        *pData++ = pDrvCbus->vsCmd;
        *pData = pDrvCbus->vsData;
		pDrvCbus->statusFlags &= ~SiiCBUS_MSC_MSG_RCVD;
		return( true );
	}
    return ( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusDevCapReadyGet
// Description: Returns if the peer's device capability values are ready
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusDevCapReadyGet ()
{
    if ( pDrvCbus->statusFlags & SiiCBUS_DCAP_RDY_RECEIVED_FM_PEER )
	{
		pDrvCbus->statusFlags &= ~SiiCBUS_DCAP_RDY_RECEIVED_FM_PEER;
		return( true );
	}
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvPathEnableGet
// Description: Returns if the peer has sent PATH_EN
// Returns:     true/false
//------------------------------------------------------------------------------
bool_t  SiiDrvPathEnableGet ()
{
    if ( pDrvCbus->statusFlags & SiiCBUS_PATH_EN_RECEIVED_FM_PEER )
	{
		pDrvCbus->statusFlags &= ~SiiCBUS_PATH_EN_RECEIVED_FM_PEER;
		return( true );
	}
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusCmdRetDataGet
// Description: return response from peer
// Parameters:  pData - pointer to return data buffer (2 bytes).
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusCmdRetDataGet ( uint8_t *pData )
{
    if ( pDrvCbus->statusFlags & SiiCBUS_MSC_CMD_DONE )
	{
		*pData++ = pDrvCbus->msgData0;
		*pData = pDrvCbus->msgData1;
		pDrvCbus->statusFlags &= ~SiiCBUS_MSC_CMD_DONE;
		return( true );
	}
     return ( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusBusStatusGet
// Description: Returns the last Bus Status data retrieved by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer (1 byte).
// Returns:     pData - Destination for bus status data.
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusBusStatusGet ( uint8_t *pData )
{
	DEBUG_PRINT(MSG_ALWAYS,"CBUS STATUS FLAG :%d, CBUS DRV CONNECTION :%d\n",pDrvCbus->statusFlags,pDrvCbus->busConnected);
    if ( pDrvCbus->statusFlags & SiiCBUS_CBUS_CONNECTION_CHG )
	{
    	*pData = pDrvCbus->busConnected;
		pDrvCbus->statusFlags &= ~SiiCBUS_CBUS_CONNECTION_CHG;
		return( true );
	}
    return ( false );
}


//------------------------------------------------------------------------------
// Function:    SiiDrvCbusCecAbortReasonGet
// Description: Returns the last DDC Abort reason received by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer (1 byte).
// Returns:     pData - Destination for DDC Abort reason data.
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusCecAbortReasonGet ( uint8_t *pData )
{
    if ( pDrvCbus->statusFlags & SiiCBUS_CEC_ABORT )
	{
    	*pData = pDrvCbus->cecAbortReason;
		pDrvCbus->statusFlags &= ~SiiCBUS_CEC_ABORT;
		return( true );
	}
    return ( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusDdcAbortReasonGet
// Description: Returns the last DDC Abort reason received by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer (1 byte).
// Returns:     pData - Destination for DDC Abort reason data.
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusDdcAbortReasonGet ( uint8_t *pData )
{
    if ( pDrvCbus->statusFlags & SiiCBUS_DDC_ABORT )
	{
    	*pData = pDrvCbus->ddcAbortReason;
		pDrvCbus->statusFlags &= ~SiiCBUS_DDC_ABORT;
		return( true );
	}
    return ( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusMscAbortTransReasonGet
// Description: Returns the last MSC Abort reason received by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer (1 byte).
// Returns:     pData - Destination for MSC Abort reason data.
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusMscAbortTransReasonGet ( uint8_t *pData )
{
    if ( pDrvCbus->statusFlags & SiiCBUS_XFR_ABORT_T )
	{
    	*pData = pDrvCbus->mscAbortReason;
		pDrvCbus->statusFlags &= ~SiiCBUS_XFR_ABORT_T;
		return( true );
	}
    return ( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusMscAbortRcvrReasonGet
// Description: Returns the last MSC Abort reason received by the CBUS ISR.
// Parameters:  pData - pointer to return data buffer (1 byte).
// Returns:     pData - Destination for MSC Abort reason data.
//------------------------------------------------------------------------------
bool_t  SiiDrvCbusMscAbortRcvrReasonGet ( uint8_t *pData )
{
    if ( pDrvCbus->statusFlags & SiiCBUS_XFR_ABORT_R )
	{
    	*pData = pDrvCbus->mscAbortFmPeerReason;
		//pDrvCbus->statusFlags &= ~SiiCBUS_XFR_ABORT_R;
		return( true );
	}
    return ( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusWriteLocalScratchpad
// Description: Returns if the peer is requesting for scratchpad write permission
// Returns:     true/false
//------------------------------------------------------------------------------
void  SiiDrvCbusWriteLocalScratchpad ( uint8_t startOffset, uint8_t length, uint8_t* pData )
{
	uint8_t i;
	uint16_t regAddr;

	for ( i = startOffset; i < (startOffset+length); i++ )
	{
		regAddr = REG_CBUS_SCRATCHPAD_0 + i;
		SiiRegWrite( regAddr, *(pData + i) );
	}
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusPortSelectBitsGet
// Description: Reads the MHL selected port(s) bit-field.  This is the value
//              downloaded from NVRAM at boot-up time.
// Parameters:  None
// Returns:     MHL selected port(s) bit-field.
//
//------------------------------------------------------------------------------
uint8_t SiiDrvCbusPortSelectBitsGet ( void )
{
    return( SiiRegRead( REG_CBUS_PORT_SEL_AUTO ));
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusHpdControl
// Description: Wrapper to call SiiDrvSwitchDeviceHpdControl()
//------------------------------------------------------------------------------
void SiiDrvCbusHpdControl ( bool_t enableHPD, uint8_t mode )
{
	// Modified for Melbourne
	//SiiDrvSwitchDeviceHpdControl(pDrvCbus->port, enableHPD, mode);
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusChnEn
// Description: Enable or disable the specified CBUS channel.
// param		isEnable - true to enable, false to disable
//------------------------------------------------------------------------------
void SiiDrvCbusChnEn ( bool_t isEnable, uint8_t port )
{

	//Bug 31744 - With samsung S3 and S4 as MHL source, video out is not consistent when plug/unplug cable at source side

//	if(port == 1)
//	{
//		SiiDrvCbusInstanceSet(1);
//		//pDrvCbus->busConnected = isEnable;
//		if(isEnable)
//		{
//			pDrvCbus->statusFlags |= SiiCBUS_INT ;
//			pDrvCbus->statusFlags |= SiiCBUS_CBUS_CONNECTION_CHG;
//		}
//		//DEBUG_PRINT(MSG_DBG, "CBus Driver:: SiiDrvCbusChnEn() called on port: %02X, with enable: %02X\n", port, isEnable);
//	}

    if ( isEnable )
    {
    	SiiDrvRxAutoHpeConfigure( port, SiiPortType_MHL );
    	SiiDrvRxHpdControl( port, true );
    }
    else
    {
    // Bug 33552 - Video not appearing on TV when MHL cable is exchanged with HDMI on Port 0 or Port 1
    	SiiDrvRxAutoHpeConfigure( port, SiiPortType_HDMI );
    	SiiDrvRxHpdControl( port, true );
    }

}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusTermCtrl
// Description: Wrapper to call SiiDrvSwitchDeviceRXTermControl()
//------------------------------------------------------------------------------
void  SiiDrvCbusTermCtrl ( bool_t terminate )
{
	// MOdified for Melbourne
    //SiiDrvSwitchDeviceRXTermControl(pDrvCbus->port, terminate ? SI_TERM_MHL : SI_TERM_HDMI );
	;
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusAutoHpeCbusEnable
// Description: Wrapper to call SiiDrvSwitchAutoHpeCbusEnable()
//------------------------------------------------------------------------------
void SiiDrvCbusAutoHpeCbusEnable ( bool_t enable )
{
//	SiiDrvSwitchAutoHpeCbusEnable(enable);
}

//------------------------------------------------------------------------------
// Function:    SiiInternalCBusWriteCommand
// Description: Write the specified Sideband Channel command to the CBUS.
//              Command can be a MSC_MSG command (RCP/MCW/RAP), or another command 
//              such as READ_DEVCAP, GET_VENDOR_ID, SET_HPD, CLR_HPD, etc.
//
// Parameters:  pReq    - Pointer to a cbus_req_t structure containing the
//                        command to write
// Returns:     TRUE    - successful write
//              FALSE   - write failed
//------------------------------------------------------------------------------
bool_t SiiDrvInternalCBusWriteCommand ( cbus_req_t *pReq  )
{
    uint8_t startbit;
    bool_t  success = true;

    DEBUG_PRINT(
        MSG_DBG, "CBUS:: Sending MSC command: %02X, offsetData: %02X, msgData[0]: %02X, PortIndex: %02x\n",
        (int)pReq->command, (int)pReq->offsetData, (int)pReq->msgData[0], pDrvCbus->port
        );

    /****************************************************************************************/
    /* Setup for the command - write appropriate registers and determine the correct        */
    /*                         start bit.                                                   */
    /****************************************************************************************/

    if(pDrvCbus->statusFlags & SiiCBUS_XFR_ABORT_R)
    {
    	SiiPlatformTimerWait( 2000 ); // 2 sec delay
    	pDrvCbus->statusFlags &= ~SiiCBUS_XFR_ABORT_R;
    }

    // Set the offset and outgoing data byte right away
    SiiRegWrite( REG_CBUS_PRI_ADDR_CMD, pReq->offsetData);   // set offset
    SiiRegWrite( REG_CBUS_PRI_WR_DATA_1ST, pReq->msgData[0] );

    startbit = 0x00;
    switch ( pReq->command )
    {
        case MHL_SET_INT:   // Set one interrupt register = 0x60
            SiiRegWrite( REG_CBUS_PRI_ADDR_CMD, pReq->offsetData + 0x20 );   // set offset
            startbit = MSC_START_BIT_SET_INT_WRITE_STAT;
            break;

        case MHL_WRITE_STAT:    // Write one status register = 0x60 | 0x80
            SiiRegWrite( REG_CBUS_PRI_ADDR_CMD, pReq->offsetData + 0x30 );   // set offset
            startbit = MSC_START_BIT_SET_INT_WRITE_STAT;
            break;

        case MHL_READ_DEVCAP:
            startbit = MSC_START_BIT_READ_DEV_CAP_REG;
            break;

        case MHL_GET_STATE:
        case MHL_GET_VENDOR_ID:
        case MHL_SET_HPD:
        case MHL_CLR_HPD:
        case MHL_GET_SC1_ERRORCODE:      // 0x69 - Get channel 1 command error code
        case MHL_GET_DDC_ERRORCODE:      // 0x6A - Get DDC channel command error code.
        case MHL_GET_MSC_ERRORCODE:      // 0x6B - Get MSC command error code.
        case MHL_GET_SC3_ERRORCODE:      // 0x6D - Get channel 3 command error code.
            SiiRegWrite( REG_CBUS_PRI_ADDR_CMD, pReq->command );
            startbit = MSC_START_BIT_MSC_CMD;
            break;

        case MHL_MSC_MSG:
            SiiRegWrite( REG_CBUS_PRI_WR_DATA_2ND, pReq->msgData[1] );
            SiiRegWrite( REG_CBUS_PRI_ADDR_CMD, pReq->command );
            DEBUG_PRINT( MSG_DBG, "CBUS:: MSG_MSC CMD:    0x%02X\n", (int)pReq->command );
            DEBUG_PRINT( MSG_DBG, "CBUS:: MSG_MSC Data 0: 0x%02X\n", (int)pReq->msgData[0] );
            DEBUG_PRINT( MSG_DBG, "CBUS:: MSG_MSC Data 1: 0x%02X\n", (int)pReq->msgData[1] );
            startbit = MSC_START_BIT_MSC_MSG_CMD;
            break;

        case MHL_WRITE_BURST:
            SiiRegWrite( REG_CBUS_PRI_ADDR_CMD, pReq->offsetData + 0x40);
            SiiRegWrite( REG_MSC_WRITE_BURST_LEN, pReq->length - 1);
            startbit = MSC_START_BIT_WRITE_BURST;
            break;

        default:
            success = false;
            break;
    }

    /****************************************************************************************/
    /* Trigger the CBUS command transfer using the determined start bit.                    */
    /****************************************************************************************/

    if ( success )
    {
        SiiRegWrite( REG_CBUS_PRI_START, startbit );
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    CbusDrvChannelProcessInterrupts
// Description: Check CBUS registers for a CBUS event on the specified channel
//------------------------------------------------------------------------------
void CbusDrvChannelProcessInterrupts ( void )
{
    uint8_t intStatus0, intStatus1, intStatusTemp;

    // Read CBUS interrupt status. Return if nothing happening on the interrupt front
    intStatus0 = SiiRegRead( REG_CBUS_INTR_0 );
    intStatus1 = SiiRegRead( REG_CBUS_INTR_1 );

    // clear the interrupts
    SiiRegWrite( REG_CBUS_INTR_0, intStatus0 );
    SiiRegWrite( REG_CBUS_INTR_1, intStatus1 );

    // mask out the interrupts that we don't care about
    intStatus1 &= ~(BIT_HEARTBEAT_TIMEOUT | BIT_SET_CAP_ID_RSVD | BIT_CBUS_PKT_RCVD);

    // Return if nothing happening on the interrupt front
    if ( !intStatus0 && !intStatus1 )
    {
        return;
    }

 //   DEBUG_PRINT( MSG_DBG, "*********** CBUS:: REG_CBUS_INTR_0:: %02X REG_CBUS_INTR_1:: %02X ***********\n", intStatus0, intStatus1);

    // An interrupt occurred, save the status.
    pDrvCbus->interruptStatus0 = intStatus0;
    pDrvCbus->interruptStatus1 = intStatus1;
    pDrvCbus->statusFlags |= SiiCBUS_INT ;

    if( intStatus0 )
    {
    	if ( intStatus0 & BIT_MSC_CMD_DONE_WITH_NACK )
		{
			pDrvCbus->statusFlags |= SiiCBUS_NACK_RECEIVED_FM_PEER;
		}

    	if ( intStatus0 & BIT_MSC_SET_INT_RCVD )
		{
			intStatusTemp = SiiRegRead( REG_CBUS_SET_INT_0 );
			if( intStatusTemp & BIT0 )
			{
				pDrvCbus->statusFlags |= SiiCBUS_DCAP_CHG_RECEIVED_FM_PEER;
			}
			if ( intStatusTemp & BIT1 )
			{
				pDrvCbus->statusFlags |= SiiCBUS_SCRATCHPAD_WRITTEN_BY_PEER;
			}
			if( intStatusTemp & BIT2 )
			{
				pDrvCbus->statusFlags |= SiiCBUS_REQ_WRT_RECEIVED_FM_PEER;
			}
			if( intStatusTemp & BIT3 )
			{
				pDrvCbus->statusFlags |= SiiCBUS_GRT_WRT_RECEIVED_FM_PEER;
			}
			if( intStatusTemp & BIT4 )
			{
				pDrvCbus->statusFlags |= SiiCBUS_3D_REQ_RECEIVED_FM_PEER;
			}

			SiiRegWrite( REG_CBUS_SET_INT_0, intStatusTemp );   // Clear received interrupts
		}

    	// This step is redundant as we do get DSCR_CHG interrupt up there
    	if ( intStatus0 & BIT_MSC_WRITE_BURST_RCVD )
		{
    		pDrvCbus->statusFlags |= SiiCBUS_SCRATCHPAD_WRITTEN_BY_PEER;
		}

		// Get any VS or MSC data received
		if ( intStatus0 & BIT_MSC_MSG_RCVD )
		{
			pDrvCbus->statusFlags |= SiiCBUS_MSC_MSG_RCVD;
			pDrvCbus->vsCmd  = SiiRegRead( REG_CBUS_PRI_VS_CMD );
			pDrvCbus->vsData = SiiRegRead( REG_CBUS_PRI_VS_DATA );
		}

		if ( intStatus0 & BIT_MSC_WRITE_STAT_RCVD )
		{
			// see if device capability values are changed
			intStatusTemp = SiiRegRead( REG_CBUS_WRITE_STAT_0 );
			if( intStatusTemp & BIT0 )
			{
				pDrvCbus->statusFlags |= SiiCBUS_DCAP_RDY_RECEIVED_FM_PEER;
				SiiRegWrite( REG_CBUS_WRITE_STAT_0, intStatusTemp );
			}

			intStatusTemp = SiiRegRead( REG_CBUS_WRITE_STAT_1 );
			if( intStatusTemp & BIT3 )
			{
				pDrvCbus->statusFlags |= SiiCBUS_PATH_EN_RECEIVED_FM_PEER;
				SiiRegWrite( REG_CBUS_WRITE_STAT_1, intStatusTemp );
			}
		}

		if ( intStatus0 & BIT_MSC_CMD_DONE )
		{
			pDrvCbus->statusFlags |= SiiCBUS_MSC_CMD_DONE;
			pDrvCbus->msgData0  = SiiRegRead( REG_CBUS_PRI_RD_DATA_1ST );
			pDrvCbus->msgData1  = SiiRegRead( REG_CBUS_PRI_RD_DATA_2ND );
		}

	    // Bus status changed?
	    if ( intStatus0 & BIT_CONNECT_CHG )
	    {
	    	pDrvCbus->statusFlags |= SiiCBUS_CBUS_CONNECTION_CHG;
	        pDrvCbus->busConnected = SiiRegRead( REG_CBUS_BUS_STATUS ) & BIT_BUS_CONNECTED;
	        //DEBUG_PRINT( MSG_DBG, "*********** CBUS:: BIT_CONNECT_CHG Interrupt came pDrvCbus->busConnected  :: %d ***********\n", pDrvCbus->busConnected );
	        //SiiRegWrite( REG_CBUS_BUS_STATUS, pDrvCbus->busConnected );  // Bus Status is read/clear
	    }
    }

    if(intStatus1)
    {
        if ( intStatus1 & BIT_CEC_ABORT )
        {
            pDrvCbus->cecAbortReason = SiiRegRead( REG_CEC_ABORT_REASON );
            pDrvCbus->statusFlags |= SiiCBUS_CEC_ABORT;
            SiiRegWrite( REG_CEC_ABORT_REASON, pDrvCbus->cecAbortReason );
        }

        if ( intStatus1 & BIT_DDC_ABORT )
        {
            pDrvCbus->ddcAbortReason = SiiRegRead( REG_DDC_ABORT_REASON );
            pDrvCbus->statusFlags |= SiiCBUS_DDC_ABORT;
            SiiRegWrite( REG_DDC_ABORT_REASON, pDrvCbus->ddcAbortReason );
        }

        // MSC_ABORT received from peer
        if ( intStatus1 & BIT_MSC_XFR_ABORT_R )
        {
            pDrvCbus->mscAbortFmPeerReason = SiiRegRead( REG_MSC_XFR_ABORT_REASON_R );
            pDrvCbus->statusFlags |= SiiCBUS_XFR_ABORT_R;
            SiiRegWrite( REG_MSC_XFR_ABORT_REASON_R, pDrvCbus->mscAbortFmPeerReason );
        }

        // MSC_ABORT happened at this device itself
        if ( intStatus1 & BIT_MSC_XFR_ABORT_T )
        {
            pDrvCbus->mscAbortReason = SiiRegRead( REG_MSC_XFR_ABORT_REASON_T );
            pDrvCbus->statusFlags |= SiiCBUS_XFR_ABORT_T;
            SiiRegWrite( REG_MSC_XFR_ABORT_REASON_T, pDrvCbus->mscAbortReason );
        }
    }
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusProcessInterrupts
// Description: Check CBUS registers for a CBUS event
//------------------------------------------------------------------------------

void SiiDrvCbusProcessInterrupts ( void )
{
    CbusDrvChannelProcessInterrupts();
    //SiiSchedSendSignal(SI_EVENT_ID_CBUS_INT, 0);
}

