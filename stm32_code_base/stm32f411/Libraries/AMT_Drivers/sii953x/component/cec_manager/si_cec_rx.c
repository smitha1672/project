//***************************************************************************
//!file     si_api_cec_tasks.c
//!brief    Silicon Image mid-level CEC handler
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include <string.h>
#include "si_common.h"
#include "si_drv_switch_config.h"
#include "si_cec_component.h"
#include "si_cec_internal.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#endif 

static bool_t CecIfChild ( SiiCpiData_t *pMsg );

//------------------------------------------------------------------------------
//! @brief      Store the LA, PA, and type of the specified device.
//! @param[in]  newLA       - CEC logical address of device to add
//!                         - If bit 7 is set, this is a virtual device
//! @param[in]  newPA       - CEC physical address of device to add
//! @param[in]  isActive    - true if this device is the active source
//------------------------------------------------------------------------------
void SiiCecUpdateLogicalDeviceInfo ( uint8_t newLA, uint16_t newPA, bool_t isActive )
{
    int_t   i, portIndex;
    bool_t  isVirtual;

  //  DEBUG_PRINT( MSG_STAT, "UpdateLogicalDeviceInfo: newla %x portIndex %x %x %x\n",newLA,
    		//((newPA & pCec->paChildMask ) >> pCec->paShift) - 1,
    	//	pCec->paChildMask, pCec->paShift );

    isVirtual = ((newLA & BIT7) != 0);
    newLA &= ~BIT7;
    if ( newLA > CEC_LOGADDR_UNREGORBC )
    {
        pCec->lastResultCode = RESULT_CEC_INVALID_LOGICAL_ADDRESS;
        return;
    }

    portIndex = ((newPA & pCec->paChildMask ) >> pCec->paShift) - 1;  // Determine actual child port.
    if (portIndex < 0)
    {
    	portIndex = 0;
    }

    if ( portIndex < SII_INPUT_PORT_COUNT )
    {
        pCec->logicalDeviceInfo[ newLA ].cecPA      = newPA;
        pCec->logicalDeviceInfo[ newLA ].port       = portIndex;
        pCec->logicalDeviceInfo[ newLA ].selected   = false;
        pCec->logicalDeviceInfo[ newLA ].isVirtual  = isVirtual;
        pCec->logicalDeviceInfo[ newLA ].deviceType = l_devTypes[ newLA ];
//        pCec->logicalAddr = newLA;

        // If called from an ACTIVE SOURCE handler, mark this device as the
        // selected device for this port.

        if ( isActive )
        {
            for ( i = 0; i <= CEC_LOGADDR_UNREGORBC; i++ )
            {
                // Update all logical addresses on this port.

                if ( pCec->logicalDeviceInfo[ i ].port == portIndex )
                {
                    if ( i == newLA )
                    {
                        // The requested port is selected
                        pCec->logicalDeviceInfo[ newLA ].selected = true;
                    }
                    else
                    {
                        pCec->logicalDeviceInfo[ newLA ].selected = false;
                    }
                }
            }
        }
        //DEBUG_PRINT( CEC_MSG_DBG, "UpdateLogicalDeviceInfo- LA:%02X PA:%04X on port %d\n",
          //      (int)newLA, newPA, (int)portIndex );
        pCec->lastResultCode = RESULT_CEC_SUCCESS;
    }
    else
    {
        pCec->logicalDeviceInfo[ newLA ].cecPA      = 0xFFFF;
        pCec->logicalDeviceInfo[ newLA ].port       = SII_INPUT_PORT_COUNT;
        pCec->logicalDeviceInfo[ newLA ].selected   = false;
        pCec->logicalDeviceInfo[ newLA ].isVirtual  = false;
        pCec->logicalDeviceInfo[ newLA ].deviceType = CEC_DT_COUNT;

        pCec->lastResultCode = RESULT_CEC_INVALID_PHYSICAL_ADDRESS;
      //  DEBUG_PRINT( CEC_MSG_DBG, "UpdateLogicalDeviceInfo: Bad Physical Address: %04X\n", newPA );
    }
}

//------------------------------------------------------------------------------
// Function:    CecHandleActiveSource
// Description: Process the CEC Active Source command by switching to the
//              broadcast port.
//------------------------------------------------------------------------------

static void CecHandleActiveSource ( SiiCpiData_t *pMsg )
{
    // Extract the logical and physical addresses of the new active source.

    pCec->activeSrcLogical  = (pMsg->srcDestAddr >> 4) & 0x0F;
    pCec->activeSrcPhysical = ((uint16_t)pMsg->args[0] << 8 ) | pMsg->args[1];

    SiiCecUpdateLogicalDeviceInfo( pCec->activeSrcLogical, pCec->activeSrcPhysical, true );

    // Determine the index of the HDMI port that is handling this physical address.

#if ( configSII_DEV_953x_PORTING == 1 )
    if ( (CecIfChild( pMsg ) == TRUE) && ( pCec->activeSrcPhysical != 0) )
    {
        pCec->portSelect = ((pCec->activeSrcPhysical >> pCec->paShift) & 0x000F) - 1;
    }
#endif
    DEBUG_PRINT( CEC_MSG_DBG, "\nACTIVE_SOURCE: %02X (%04X) (port %02X)\n",
            (int)pCec->activeSrcLogical, pCec->activeSrcPhysical, (int)pCec->portSelect );

    // Signal App layer that a port select change has occurred.
    // The App layer will perform the physical port switch.

    pCec->statusFlags |= SiiCEC_PORT_CHANGE;
}

//------------------------------------------------------------------------------
// Function:    CecHandleInactiveSource
// Description: Process the CEC Inactive Source command
//------------------------------------------------------------------------------

static void CecHandleInactiveSource ( SiiCpiData_t *pMsg )
{
    uint8_t la;

    la = (pMsg->srcDestAddr >> 4) & 0x0F;
    if ( la == pCec->activeSrcLogical )    // The active source has deserted us!
    {
        pCec->activeSrcLogical  = CEC_LOGADDR_TV;
        pCec->activeSrcPhysical = 0x0000;
    }
    // Signal upper layer that the active source has been lost

    pCec->statusFlags |= SiiCEC_SOURCE_LOST;
}

//------------------------------------------------------------------------------
// Function:    CecHandleReportPhysicalAddress
// Description: Store the PA and LA of the subsystem.
//              This routine is called when a physical address was broadcast.
//              usually this routine is used for a system which configure as TV or Repeater.
//------------------------------------------------------------------------------

static void CecHandleReportPhysicalAddress ( SiiCpiData_t *pMsg )
{
	if(pMsg->args[2] != 0)
		pMsg->srcDestAddr = (pMsg->srcDestAddr & 0x0F) | (pMsg->args[2] << 4) ;
    SiiCecUpdateLogicalDeviceInfo((pMsg->srcDestAddr >> 4) & 0x0F,         // broadcast logical address
        (((uint16_t)pMsg->args[0]) << 8) | pMsg->args[1],   // broadcast physical address
        false
        );
}

//------------------------------------------------------------------------------
// Function:    CecRxMsgHandler
// Description: This is the first RX message handler called in the chain. It parses
//              messages that it needs to keep the internal component status
//              up to date.
//------------------------------------------------------------------------------
bool_t CecRxMsgHandler ( SiiCpiData_t *pMsg )
{
    bool_t  usedMessage         = true;
    bool_t  isDirectAddressed   = !((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );

    switch ( pMsg->opcode )
    {
        case CECOP_IMAGE_VIEW_ON:       // In our case, respond the same to both these messages
        case CECOP_TEXT_VIEW_ON:
#if ( configSII_DEV_953x_PORTING == 0 )
            if ( !isDirectAddressed )
            {
                if ( CecIfChild( pMsg ) == FALSE)
                {
			break;
                }
            }

            // Make sure the system is powered on.  The application layer
            // must handle any other procedures needed to turn on the display
            if ( pCec->powerState != CEC_POWERSTATUS_ON )
            {
		xHMISrvEventParams srv_parms = { xHMI_EVENT_CEC, USER_EVENT_UNDEFINED };
		
                // Next time through the main loop, power will be cycled on
                pCec->powerState    = CEC_POWERSTATUS_STANDBY_TO_ON;
                pCec->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;            // Signal upper layer
                
                srv_parms.event = USER_EVENT_CEC_POWER_ON;
                pHS_ObjCtrl->SendEvent(&srv_parms);
            }
            // don't tell anyone we looked at this message.  The app layer
            // likely needs to see it also.
            usedMessage = false;
#endif
            if ( !isDirectAddressed )	break;

            // Make sure the system is powered on.  The application layer
            // must handle any other procedures needed to turn on the display
            if ( pCec->powerState != CEC_POWERSTATUS_ON )
            {
                // Next time through the main loop, power will be cycled on
                pCec->powerState    = CEC_POWERSTATUS_STANDBY_TO_ON;
                pCec->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;            // Signal upper layer
            }
            // don't tell anyone we looked at this message.  The app layer
            // likely needs to see it also.
            usedMessage = false;
            break;

        case CECOP_ACTIVE_SOURCE:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
                if ( (CecIfChild( pMsg ) == TRUE) && (pCec->powerState != CEC_POWERSTATUS_ON) )
                {

#if ( configSII_DEV_953x_PORTING == 0 )
	                pCec->powerState    = CEC_POWERSTATUS_STANDBY_TO_ON;
	                pCec->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;            // Signal upper layer
#endif	                
	                
                        if ( pMsg->args[0] != 0x00 )
			{
			    AmTCecEventPassToHMI(USER_EVENT_SAC_POWER_ON);	//Power On sound bar
			AmTCecEventPassToHMI(USER_EVENT_SEL_HDMI_IN);	//Change to HDMI if need
                }
			else
			{
			    AmTCecEventPassToHMI(USER_EVENT_ARC_POWER_ON);	//Power On sound bar
			}
                }
				
                CecHandleActiveSource( pMsg );
            }
            break;

        case CECOP_REPORT_PHYSICAL_ADDRESS:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
                CecHandleReportPhysicalAddress( pMsg );

                // Let Enumerate task know about it.
                if ( ACTIVE_TASK.task == SiiCECTASK_ENUMERATE )
                {
                    ACTIVE_TASK.cpiState = CPI_RESPONSE;
                }
            }
            break;

        case CECOP_INACTIVE_SOURCE:
            if ( isDirectAddressed )                    // Ignore as broadcast message
            {
                CecHandleInactiveSource( pMsg );
            }
            break;

#if !INC_CEC_SWITCH
        case CECOP_SET_STREAM_PATH:
             if( !isDirectAddressed && (pMsg->argCount == 2) )
            {
	        uint16_t    newPhysAddr=0;
			
                newPhysAddr = (pMsg->args[0] << 8) | pMsg->args[1];
                DEBUG_PRINT(MSG_STAT, ("Physical Address : 0x%x.\n"),newPhysAddr);

                if( newPhysAddr == pCec->physicalAddr )
                {

                	DEBUG_PRINT(MSG_STAT, ("[CEC_SW]: received Set Stream Path msg.\n"));
                	DEBUG_PRINT(MSG_STAT, ("[CEC_SW]: Standby mode Off Request\n"));

	                pCec->powerState    = CEC_POWERSTATUS_STANDBY_TO_ON;
	                pCec->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;            // Signal upper layer
	                
			AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_ON);	//Power On sound bar
					
			AmTCecEventPassToHMI(USER_EVENT_SEL_HDMI_IN);	//Change to HDMI if need

			Broadcast Active Source and we are active source
                }
            }
            else
            {
    		DEBUG_PRINT(CEC_MSG_DBG, "[CEC_SW]: ERROR in <%s> message: wrong length!\n", str);
            }
            break;
#endif

        default:
            usedMessage = false;                        // Didn't use the message
            break;
    }

    return( usedMessage );
}

//------------------------------------------------------------------------------
//! @brief  Send the appropriate CEC commands for switching from the current
//!         active source to the requested source.
//!         This is called after the main process has ALREADY performed the
//!         actual port switch on the RX, so the physical address is changed
//!         even if the input is NOT in the CEC list, either because it has
//!         not reported itself as an active source, or it is not CEC
//!         capable.
//! @param[in]  portIndex - new HDMI port index.  May be CEC or not.
//! @return     bool_t    - true if successful, false if could not queue CEC task.
//------------------------------------------------------------------------------
bool_t SiiCecSwitchSources ( uint8_t portIndex )
{
    uint16_t    portPA;

    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    if ( pCec->enable )
    {
        // If bad port index or not yet enumerated, get out.
        if (( portIndex >= SII_INPUT_PORT_COUNT ) || ( SiiCecGetDeviceLA() == CEC_LOGADDR_UNREGORBC ))
        {
            pCec->lastResultCode = RESULT_CEC_INVALID_PORT_INDEX;
            return( false );
        }

        // If the CEC portSelect value is NOT already set to the requested port,
        // chances are the new port was requested via the Sink UI as opposed to
        // receiving an Active Source message from a source device.  If the
        // request came from the Sink UI, send a SET_STREAM_PATH to point
        // everybody to our new source (and wake it up, if necessary).
        if ( portIndex != pCec->portSelect )
        {
            pCec->portSelect = portIndex;

            // Only send this message if we are a SINK device
            if ( pCec->deviceType == CEC_DT_TV )
            {
                // If port has an associated CEC physical address, send the message
                portPA = SiiCecPortToPA( portIndex );
                if ( portPA != 0xFFFF )
                {
                    CecSendSetStreamPath( portPA );
                }
            }
        }
    }

    return( true );
}

static bool_t CecIfChild ( SiiCpiData_t *pMsg )
{
	uint8_t devicePA, ChildMask;

	ChildMask = 0xFFFF << pCec->paShift;
	ChildMask <<= 4;
	
	devicePA = pMsg->srcDestAddr & 0x0F;

	if ( (pCec->physicalAddr & ChildMask) == (devicePA & ChildMask ) )
		return TRUE;
	else
	{
		TRACE_DEBUG((0, "CEC: PA for this device is not our child."));
		return FALSE;
	}
}
