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
#include "PowerHandler.h"
#include "AudioDeviceManager.h"

extern AUDIO_DEVICE_MANAGER_OBJECT *pADM_ObjCtrl;
extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
#endif 

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
#define SII_HDMI_PORT	3
static void CecHandleActiveSource ( SiiCpiData_t *pMsg )
{
    uint16_t devicePA;

    // Extract the logical and physical addresses of the new active source.

    pCec->activeSrcLogical  = (pMsg->srcDestAddr >> 4) & 0x0F;
    pCec->activeSrcPhysical = ((uint16_t)pMsg->args[0] << 8 ) | pMsg->args[1];
    pCec->isActiveSource = FALSE;

    SiiCecUpdateLogicalDeviceInfo( pCec->activeSrcLogical, pCec->activeSrcPhysical, true );

    // Determine the index of the HDMI port that is handling this physical address.

#if ( configSII_DEV_953x_PORTING == 1 )
    devicePA = pMsg->args[0] << 8;
    devicePA |= pMsg->args[1];
		
    if ( (CecIfChild( devicePA ) == TRUE) && ( pCec->activeSrcPhysical != 0) )
    {
    // check player PA consist with SB HPD status, kaomin
    if(((pCec->activeSrcPhysical>>pCec->paShift) & 0xF) == SII_HDMI_PORT) 
    {
        pCec->portSelect = 2;//((pCec->activeSrcPhysical >> pCec->paShift) & 0x000F) - 1;
     }
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
            if ( !isDirectAddressed )	break;

#if ( configSII_DEV_953x_PORTING == 0 )
            // Make sure the system is powered on.  The application layer
            // must handle any other procedures needed to turn on the display
            if ( pCec->powerState != CEC_POWERSTATUS_ON )
            {
                // Next time through the main loop, power will be cycled on
                pCec->powerState    = CEC_POWERSTATUS_STANDBY_TO_ON;
                pCec->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;            // Signal upper layer
            }
#else
//TRACE_DEBUG((0, "CECOP_IMAGE/TEXT_VIEW_ON"));
	    uint16_t devicePA;

            devicePA = pMsg->srcDestAddr>>4;
TRACE_DEBUG((0, "***CECOP_IMAGE/TEXT_VIEW_ON: deviceLA = %X", devicePA));
	    devicePA = pCec->logicalDeviceInfo[devicePA].cecPA;
		
TRACE_DEBUG((0, "***CECOP_IMAGE/TEXT_VIEW_ON: devicePA = %X", devicePA));

            if ( CecIfChild( devicePA ) == TRUE )
            {
            	//if ( (pCec->powerState == CEC_POWERSTATUS_ON) || (pCec->powerState == CEC_POWERSTATUS_STANDBY_TO_ON)  )
            	//{
                     //AmTCecEventPassToHMI(USER_EVENT_CEC_SEL_HDMI_IN);	//Change to HDMI_IN
//TRACE_DEBUG((0, "CECOP_IMAGE/TEXT_VIEW_ON: USER_EVENT_CEC_SEL_HDMI_IN"));
            	//}
		//else
		//{
                     AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_ON_HDMI_IN);	//Power SB on and change to HDMI_IN
//TRACE_DEBUG((0, "CECOP_IMAGE/TEXT_VIEW_ON: USER_EVENT_CEC_POWER_ON_HDMI_IN"));
		//}
            }
#endif
            // don't tell anyone we looked at this message.  The app layer
            // likely needs to see it also.
            usedMessage = false;
            break;

        case CECOP_ACTIVE_SOURCE:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
#if ( configSII_DEV_953x_PORTING == 1 )
TRACE_DEBUG((0, "CECOP_ACTIVE_SOURCE"));
	        uint16_t devicePA;

	        devicePA = pMsg->args[0] << 8;
	        devicePA |= pMsg->args[1];
		
                if ( CecIfChild( devicePA ) == TRUE )
                {
                    //if ( pMsg->args[0] != 0x00 )	//if Active is child but not TV => Go to HDMI IN
                    if ( devicePA != 0x00 )	//if Active is child but not TV => Go to HDMI IN
                    {
//TRACE_DEBUG((0, "CECOP_ACTIVE_SOURCE: Non TV"));
            	        //if ( (pCec->powerState == CEC_POWERSTATUS_ON) || (pCec->powerState == CEC_POWERSTATUS_STANDBY_TO_ON)  )
            	        //{
                        //    AmTCecEventPassToHMI(USER_EVENT_CEC_SEL_HDMI_IN);	//Change to HDMI_IN
//TRACE_DEBUG((0, "CECOP_ACTIVE_SOURCE: USER_EVENT_CEC_SEL_HDMI_IN"));
            	        //}
		        //else
		        //{
		        //TRACE_DEBUG((0, "USER_EVENT_CEC_POWER_ON_HDMI_IN ------- aaaa "));
                            if (((devicePA & 0xF000) != (pCec->physicalAddr & 0xF000)))
                            {
                                //TRACE_DEBUG((0, "From TV other port 0x%X ",devicePA));
                            }
                            else
                            {
                                if( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON ) 
                                {
                                    //TRACE_DEBUG((0, "***CECOP_IMAGE/TEXT_VIEW_ON: CHANGE source to HDMI 5"));
                                    xAudDeviceParms adm_params;
                                
                                    adm_params = pADM_ObjCtrl->GetAudioParams();
                                
                                    if ((adm_params.input_src != AUDIO_SOURCE_HDMI_ARC) && (adm_params.input_src != AUDIO_SOURCE_HDMI_IN))
                                    {
                                        AmTCecEventPassToHMI(USER_EVENT_CEC_SEL_HDMI_IN);
                                    }
                                }
                            }
                            //AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_ON_HDMI_IN);	//Power SB on and change to HDMI_IN
//TRACE_DEBUG((0, "CECOP_ACTIVE_SOURCE: USER_EVENT_CEC_POWER_ON_HDMI_IN"));
		        //}
                    }
#if 0
		    else	//Active is child and TV. Go to HDMI ARC
		    {
//TRACE_DEBUG((0, "CECOP_ACTIVE_SOURCE: TV"));
            	        //if ( (pCec->powerState == CEC_POWERSTATUS_ON) || (pCec->powerState == CEC_POWERSTATUS_STANDBY_TO_ON)  )
            	        //{
                            //AmTCecEventPassToHMI(USER_EVENT_CEC_SEL_HDMI_ARC);	//Change to HDMI_ARC
//TRACE_DEBUG((0, "CECOP_ACTIVE_SOURCE: USER_EVENT_CEC_SEL_HDMI_ARC"));
            	        //}
		        //else
		        //{
                            AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_ON);	//Power SB on and change to HDMI_ARC
//TRACE_DEBUG((0, "CECOP_ACTIVE_SOURCE: USER_EVENT_CEC_POWER_ON_HDMI_ARC"));
		        //}
		    }
#endif            
                }
#endif
				
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
            pCec->portSelect = 2;//portIndex;

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

bool_t CecIfChild ( uint16_t devicePA )
{
	uint16_t SoundBarPA, ChildMask;
	uint8_t paShift = 0;

	//ChildMask = 0xFFFF << pCec->paShift;
	//ChildMask <<= 4;

	ChildMask = 0x000F;
	for ( paShift = 0; paShift < 4; paShift++ )
	{
            ChildMask <<= (paShift*4);
		
//TRACE_DEBUG((0, "CecIfChild: ChildMask = %X", ChildMask));
	    SoundBarPA = pCec->physicalAddr & ChildMask;
	    if ( SoundBarPA )
	    {
		break;
	    }
	}
TRACE_DEBUG((0, "CecIfChild: PA Masked = %X", ChildMask));
	
//TRACE_DEBUG((0, "CecIfChild: Active Source PA = %X", devicePA));
//TRACE_DEBUG((0, "CecIfChild: Our PA = %X", pCec->physicalAddr));
//ERR     >>  sil9533 writing retry 0 times; Error device id 0xC0, regAddr 0x9F
        if ( (devicePA == 0x0000) && (ChildMask == 0xF000) )
        {
		return TRUE;
        }
	else if ( (devicePA  & ChildMask) == SoundBarPA )
		return TRUE;
	else
	{
		TRACE_DEBUG((0, "CEC: PA for this device is not our child."));
                TRACE_DEBUG((0, "CecIfChild: PA Masked = %X", ChildMask));
                TRACE_DEBUG((0, "CecIfChild: devicePA = %X", devicePA));
		return FALSE;
	}
}
