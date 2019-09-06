//***************************************************************************
//!file     si_cec_component.c
//!brief    Silicon Image CEC Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_cec_component.h"
#include "si_cec_internal.h"
#include "si_drv_switch_config.h"


//------------------------------------------------------------------------------
//  CEC Component Instance Data
//------------------------------------------------------------------------------

CecInstanceData_t cecInstance[SII_NUM_CEC];
CecInstanceData_t *pCec = &cecInstance[0];

//------------------------------------------------------------------------------
// Function:    SiiCecInitialize
// Description: Initialize the current instance of the CEC component.
// Parameters:  none
// Returns:     It returns true if the initialization is successful, or false
//				if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCecInitialize ( uint16_t physicalAddress, SiiCecDeviceTypes_t deviceType )
{
    int_t   i;
    bool_t  error = false;

    // Initialize the internal data structures used by CEC
    memset( pCec, 0, sizeof( CecInstanceData_t ));

    pCec->enable            = false;
    pCec->logicalAddr       = CEC_LOGADDR_UNREGORBC;
    pCec->sourcePowerStatus = CEC_POWERSTATUS_STANDBY;
    pCec->activeSrcLogical  = CEC_LOGADDR_UNREGORBC;
    pCec->deviceType        = deviceType;

    pCec->lastUserControlPressedSourceLa = CEC_LOGADDR_UNREGORBC;
    pCec->lastUserControlPressedTargetLa = CEC_LOGADDR_UNREGORBC;

    SiiCecSetDevicePA( physicalAddress );

    // Mark all devices as not found
    for ( i = 0; i <= CEC_LOGADDR_UNREGORBC; i++ )
    {
        pCec->logicalDeviceInfo[ i].cecPA = 0xFFFF;
        pCec->logicalDeviceInfo[ i].deviceType = CEC_DT_COUNT;
    }

    for (i = 0 ; i< CHIP_MAX_VIRTUAL_CEC_DEVS; i++)
    {
    	pCec->virtualDevLA[i] = CEC_LOGADDR_TV;
    }

#if ( configSII_DEV_953x_PORTING == 1 )
    pCec->portSelect = 2;
#endif

    // Register internal message handlers defined for certain device types
    switch ( pCec->deviceType )
    {
        case CEC_DT_TV:
            error = !SiiCecCallbackRegisterParser( CecRxMsgHandler, false );
            break;
        case CEC_DT_VIDEO_PROCESSOR:
            error = !SiiCecCallbackRegisterParser( CecTxMsgHandler, false );
            SiiCecSetDeviceLA( CEC_LOGADDR_VIDEO_PROCESSOR );
            break;
        case CEC_DT_AUDIO_SYSTEM:
            error = !SiiCecCallbackRegisterParser( CecRxMsgHandler, false );
            error = !SiiCecCallbackRegisterParser( CecTxMsgHandler, false );
    		pCec->devLogicalAddr = CEC_LOGADDR_AUDSYS; //store the actaul device logical address

            SiiCecSetDeviceLA( CEC_LOGADDR_AUDSYS );
            break;
        case CEC_DT_PLAYBACK:
//            error = !SiiCecCallbackRegisterParser( CecRxMsgHandler, false );
            error = !SiiCecCallbackRegisterParser( CecTxMsgHandler, false );
            SiiCecSetDeviceLA( CEC_LOGADDR_PLAYBACK1 );
            break;
        default:
            break;
    }

	return( !error );
}

//------------------------------------------------------------------------------
// Function:    SiiCecConfigure
// Description:
// Parameters:  none
// Returns:     true if the configuration was successful, or false if some
//              failure occurred
//------------------------------------------------------------------------------

bool_t SiiCecConfigure ( bool_t enable )
{

    pCec->enable   = enable;
    pCec->enable ? SiiDrvCpiEnable() : SiiDrvCpiDisable();

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCecStatus
// Description: Returns a status flag word containing CEC-specific
//				information about the state of the device.
//              NOTE: Some flags are cleared as soon as they are read.
// Parameters:  none
// Returns:     Sixteen-bit status flags word for the CEC Component
//------------------------------------------------------------------------------

uint16_t SiiCecStatus ( void )
{
    uint16_t statusFlags = pCec->statusFlags;

    pCec->statusFlags &= ~(SiiCEC_POWERSTATE_CHANGE | SiiCEC_PORT_CHANGE);
    return( statusFlags );
}

//------------------------------------------------------------------------------
// Function:    SiiCecStandby
//! @brief      Prepare for device standby mode.
//              Make any configuration changes required for standby mode and
//              save any settings that must be restored after a power resume.
// Returns:     true if the CEC component entered standby mode successfully,
//				or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCecStandby ( void )
{
    return( SiiDrvCpiStandby());
}

//------------------------------------------------------------------------------
// Function:    SiiCecResume
//! @brief      Recover from power down standby mode
//				Restore settings saved by SiiCecStandby and configure the
//              component as required.
// Parameters:  none
// Returns:     true if the CEC component successfully exited standby mode,
//				or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCecResume ( void )
{
    return( SiiDrvCpiResume());
}

//------------------------------------------------------------------------------
// Function:    SiiCecInstanceGet
// Description: returns the index of the current CEC instance
// Parameters:  none
// Returns:     Index of the current CEC instance
//------------------------------------------------------------------------------
int SiiCecInstanceGet ( void )
{

    return( pCec->instanceIndex );
}

//------------------------------------------------------------------------------
// Function:    SiiCecInstanceSet
// Description: Set the component global data pointer to the requested instance.
// Parameters:  instanceIndex
// Returns:     true if legal index, false if index value is illegal.
//------------------------------------------------------------------------------
bool_t SiiCecInstanceSet ( int instanceIndex )
{
    if ( instanceIndex < SII_NUM_CPI )
    {
        pCec = &cecInstance[ instanceIndex];
        return( true );
    }

    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiCecGetLastResult
// Description: Returns the result of the last SiiCecxxx function called
// Parameters:  none
// Returns:     Result of last CEC component function.
//------------------------------------------------------------------------------

bool_t SiiCecGetLastResult ( void )
{

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCecCallbackRegisterParser
// Description: Register a message handler with the specified CEC instance
// Parameters:  Callback function pointer
// Returns:     true if successful
//------------------------------------------------------------------------------

bool_t SiiCecCallbackRegisterParser ( bool_t (*pCallBack)(), bool_t callAlways )
{
    bool_t  success = false;
    int_t   callBackIndex;

    for ( callBackIndex = 0; callBackIndex < MAX_CEC_PARSERS; callBackIndex++ )
    {
        if ( pCec->messageHandlers[ callBackIndex ].pMsgHandler == 0)
        {
            pCec->messageHandlers[ callBackIndex ].pMsgHandler  = pCallBack;
            pCec->messageHandlers[ callBackIndex ].callAlways   = callAlways;
            success = true;
            break;
        }
    }

    return( success );
}

bool_t SiiCecCallbackRegisterSet ( bool_t (*pCallBack)(), bool_t callAlways )
{
    bool_t  success = false;
    int_t   callBackIndex;

    for ( callBackIndex = 0; callBackIndex < MAX_CEC_PARSERS; callBackIndex++ )
    {
        if ( pCec->messageHandlers[ callBackIndex ].pMsgHandler != 0)
        {
            if ( pCec->messageHandlers[ callBackIndex ].pMsgHandler  == pCallBack )
            {
	            pCec->messageHandlers[ callBackIndex ].callAlways   = callAlways;
        	    success = true;
            	    break;
            }
        }
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    SiiCecPortSelectGet
// Description: Returns the last port selected via an ACTIVE_SOURCE or other
//              CEC command that may change the selected port.  Note that it
//              is only valid immediately after the SiiCEC_PORT_CHANGE flag is
//              set.
// Parameters:  none
// Returns:     Input Processor port selected via CEC command.
//------------------------------------------------------------------------------

int_t SiiCecPortSelectGet ( void )
{

    return( pCec->portSelect );
}

//------------------------------------------------------------------------------
// Function:    SiiCecPortSelectSet
// Description: Set internal port selection. Used for communication with
//              other CEC components.
// Parameters:  none
// Returns:
//------------------------------------------------------------------------------

void SiiCecPortSelectSet ( uint8_t inputPort)
{
    pCec->portSelect = 2;//inputPort;
}

//------------------------------------------------------------------------------
// Function:    SiiCecCallbackRegisterTask
// Description: Register a CEC task with the current CEC instance
// Parameters:  Callback function pointer
// Returns:     Index of registered task, or -1 if no room
//------------------------------------------------------------------------------

int_t SiiCecCallbackRegisterTask ( bool_t (*pCallBack)() )
{
    int_t callBackIndex;

    for ( callBackIndex = 0; callBackIndex < MAX_CEC_TASKS; callBackIndex++ )
    {
        if ( pCec->pTasks[ callBackIndex ] == 0)
        {
            pCec->pTasks[ callBackIndex ] = pCallBack;
            break;
        }
    }

    return( ( callBackIndex == MAX_CEC_TASKS) ? -1 : (callBackIndex + SiiCECTASK_COUNT) );
}

//------------------------------------------------------------------------------
//! @brief      Send a single byte (no parameter) message on the CEC bus.  Does
//!             not wait for a reply.
//! @param[in]  opCode  - CEC opcode to send
//! @param[in]  src     - source CEC logical address
//! @param[in]  dest    - destination CEC logical address
//! @return     uint16_t    16 bit message ID or 0 if CEC not enabled
//------------------------------------------------------------------------------
uint16_t SiiCecSendMessageEx ( uint8_t opCode, uint8_t src, uint8_t dest )
{
    SiiCpiData_t cecFrame;

    if ( !pCec->enable )
    {
        pCec->lastResultCode = RESULT_CEC_FAIL;
        return( 0 );
    }
    cecFrame.opcode        = opCode;
    cecFrame.srcDestAddr   = MAKE_SRCDEST( src, dest );
    cecFrame.argCount      = 0;

    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    return( SiiDrvCpiWrite( &cecFrame ));
}

//------------------------------------------------------------------------------
//! @brief      Send a single byte (no parameter) message on the CEC bus.  Does
//!             not wait for a reply. The local device CEC logical address is
//!             used as the source device in the CEC message.
//! @param[in]  opCode  - CEC opcode to send
//! @param[in]  dest    - destination CEC logical address
//! @return     uint16_t    16 bit message ID
//!
//! @note       This function is deprecated; please use SiiCecSendMessageEx
//------------------------------------------------------------------------------
uint16_t SiiCecSendMessage ( uint8_t opCode, uint8_t dest )
{

    return( SiiCecSendMessageEx( opCode, pCec->logicalAddr, dest ));
}

//------------------------------------------------------------------------------
//! @brief  Send a remote control key pressed command to the specified logical
//!         device.
//! @param[in]  keyCode - UI Command from CEC enum lists
//------------------------------------------------------------------------------
void SiiCecSendUserControlPressed ( SiiCecUiCommand_t keyCode, SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa )
{
    SiiCpiData_t cecFrame;

    if ( !pCec->enable )
    {
        return;
    }

    pCec->lastUserControlPressedSourceLa = srcLa;
    pCec->lastUserControlPressedTargetLa = destLa;

    cecFrame.opcode        = CECOP_USER_CONTROL_PRESSED;
    cecFrame.srcDestAddr   = MAKE_SRCDEST( srcLa, destLa );
    cecFrame.args[0]       = keyCode;
    cecFrame.argCount      = 1;
    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  Send a remote control key released command to the logical device
//!         specified in the last SiiCecSendUserControlPressed() call.
//------------------------------------------------------------------------------
void SiiCecSendUserControlReleased ( void )
{
    if ( !pCec->enable )
    {
        return;
    }

    if ( pCec->lastUserControlPressedTargetLa != CEC_LOGADDR_UNREGORBC )
    {
        SiiCecSendMessageEx(
            CECOP_USER_CONTROL_RELEASED,
            pCec->lastUserControlPressedSourceLa,
            pCec->lastUserControlPressedTargetLa );
        pCec->lastUserControlPressedSourceLa = CEC_LOGADDR_UNREGORBC;
        pCec->lastUserControlPressedTargetLa = CEC_LOGADDR_UNREGORBC;
    }
}

//------------------------------------------------------------------------------
// Function:    SiiCecGetPowerState
// Description: Returns the CEC local power state.
//              Should be called after every call to SiiCecHandler since
//              a CEC device may have issued a standby or view message.
//
// Valid values:    CEC_POWERSTATUS_ON
//                  CEC_POWERSTATUS_STANDBY
//                  CEC_POWERSTATUS_STANDBY_TO_ON
//                  CEC_POWERSTATUS_ON_TO_STANDBY
//------------------------------------------------------------------------------

uint8_t SiiCecGetPowerState ( void )
{

    return( pCec->powerState );
}

//------------------------------------------------------------------------------
// Function:    SiiCecSetPowerState
// Description: Set the CEC local power state.
//
// Valid values:    CEC_POWERSTATUS_ON
//                  CEC_POWERSTATUS_STANDBY
//                  CEC_POWERSTATUS_STANDBY_TO_ON
//                  CEC_POWERSTATUS_ON_TO_STANDBY
//------------------------------------------------------------------------------

void SiiCecSetPowerState ( SiiCecPowerstatus_t newPowerState )
{
    if ( !pCec->enable )
    {
        return;
    }

    if ( pCec->powerState != newPowerState )
    {
        switch ( newPowerState )
        {
            case CEC_POWERSTATUS_STANDBY_TO_ON:
            case CEC_POWERSTATUS_ON:
                newPowerState = CEC_POWERSTATUS_ON;
                break;

            case CEC_POWERSTATUS_ON_TO_STANDBY:
            case CEC_POWERSTATUS_STANDBY:
                newPowerState = CEC_POWERSTATUS_STANDBY;
                break;

            default:
                break;
        }

    pCec->powerState = newPowerState;
    }
}

//------------------------------------------------------------------------------
// Function:    SiiCecPortToPA
// Description: Return the CEC physical address of the last selected logical
//              device on the CEC path for this port.  If no logical devices
//              on this path, return 0xFFFF
// Parameters:  portIndex:  HDMI RX, 0-3
//                          HDMI TX, 0
// Returns:     CEC physical address for the port or 0xFFFF if not found
//------------------------------------------------------------------------------

uint16_t SiiCecPortToPA ( uint8_t portIndex )
{
    int_t i;
    uint16_t    physicalAddress = 0xFFFF;   // failure return

    pCec->lastResultCode = RESULT_CEC_SUCCESS;

    for ( i = 0; i <= CEC_LOGADDR_UNREGORBC; i++ )
    {
        // Does requested port has the same index as the current logical address?
        if ( pCec->logicalDeviceInfo[i].port == portIndex )
        {
            // Save the physical address.
            physicalAddress = pCec->logicalDeviceInfo[ i].cecPA;

            // There may be more than one device in the physical address chain
            // for a particular port, check to see if this is the last device
            // that was selected (ACTIVE SOURCE) on this particular port.
            // If NO logical device was the last active source on this port,
            // we'll take the first logical device on this port.
            if ( pCec->logicalDeviceInfo[i].selected )
            {
                break;
            }
        }
    }

    if ( physicalAddress == 0xFFFF )
    {
        pCec->lastResultCode = RESULT_CEC_NO_PA_FOUND;
    }
    return( physicalAddress );
}

//------------------------------------------------------------------------------
//! @brief  Return the CEC logical address of the passed port
//! @param[in]  portIndex
//! @return     CEC logical address for the port or CEC_LOGADDR_UNREGORBC
//!             if port has not been assigned a logical address
//------------------------------------------------------------------------------
uint8_t SiiCecPortToLA ( uint8_t portIndex )
{
    uint8_t logicalAddress;

    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    for ( logicalAddress = 0; logicalAddress <= CEC_LOGADDR_UNREGORBC; logicalAddress++ )
    {
        // Does requested port has the same index as the current logical address?
        if ( pCec->logicalDeviceInfo[ logicalAddress].port == portIndex )
        {
            break;
        }
    }

    if ( logicalAddress == CEC_LOGADDR_UNREGORBC )
    {
        pCec->lastResultCode = RESULT_CEC_NO_LA_FOUND;
    }
    return( logicalAddress );
}

//------------------------------------------------------------------------------
// Function:    SiiCecLaToPort
// Description: Translate the passed logical address into a physical HDMI port
// Parameters:  logicalAddress  - The CEC logical address to translate
// Returns:     HDMI physical port index or oxFF if not found.
//------------------------------------------------------------------------------

int_t SiiCecLaToPort ( uint8_t logicalAddr )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;

    if ( logicalAddr > CEC_LOGADDR_UNREGORBC )
    {
        pCec->lastResultCode = RESULT_CEC_INVALID_LOGICAL_ADDRESS;
        return( 0xFF );
    }
    return( pCec->logicalDeviceInfo[ logicalAddr].port );
}

//------------------------------------------------------------------------------
// Function:    SiiCecGetDeviceLA
// Description: Return the CEC logical address for this Host device
//------------------------------------------------------------------------------

uint8_t SiiCecGetDeviceLA ( void )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    return( pCec->logicalAddr );
}

bool_t SiiCecIsVirtualDevicePresent(void)
{
	return pCec->numVirtualDevices? true:false;
}

//------------------------------------------------------------------------------
// Function:    SiiIsVirtualDeviceLA
// Description: returns if the given cec LA is of virtual device
//------------------------------------------------------------------------------
bool_t SiiCecIsVirtualDeviceLA(SiiCecLogicalAddresses_t cecLA)
{
	uint8_t i;
	for(i = 0; i < CHIP_MAX_VIRTUAL_CEC_DEVS; i++)
	{
		if(pCec->virtualDevLA[i] == cecLA)
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
// Function:    SiiSetCecVirtualDevLA
// Description: Set the CEC logical address for virtual device
//------------------------------------------------------------------------------
void SiiCecSetVirtualDevLA(uint8_t index, SiiCecLogicalAddresses_t virtualLA)
{
	pCec->virtualDevLA[index] = virtualLA;
	pCec->numVirtualDevices++;
}

//------------------------------------------------------------------------------
// Function:    SiiRemoveVirtualDevice
// Description: remove virtual device
//------------------------------------------------------------------------------
bool_t SiiCecRemoveVirtualDevice( SiiCecLogicalAddresses_t logicalAddr )
{
	uint8_t i;
	bool_t success = false;
	for(i = 0; i < CHIP_MAX_VIRTUAL_CEC_DEVS; i++)
	{
		if(pCec->virtualDevLA[i] == logicalAddr)
		{
			pCec->virtualDevLA[i] = CEC_LOGADDR_TV;
			pCec->numVirtualDevices--;
			success = true;
			break;
		}
	}
	if(pCec->numVirtualDevices == 0)
	{
		SiiDrvCpiClearVirtualDevBit();
	}
	return success;
}

//------------------------------------------------------------------------------
// Function:    SiiCecSetDeviceLA
// Description: Set the CEC logical address for this Host device
//------------------------------------------------------------------------------

void SiiCecSetDeviceLA ( SiiCecLogicalAddresses_t logicalAddr )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    pCec->logicalAddr = logicalAddr;
    if(!SiiCecIsVirtualDevicePresent())
    	SiiDrvCpiSetLogicalAddr( logicalAddr );
    else
    	SiiDrvCpiAddLogicalAddr(true,logicalAddr);
}

//------------------------------------------------------------------------------
// Function:    SiiCecGetDevicePA
// Description: Return the physical address for this Host device
//------------------------------------------------------------------------------

uint16_t SiiCecGetDevicePA ( void )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;

    return( pCec->physicalAddr );
}


//------------------------------------------------------------------------------
// Function:    SiiCecSetDevicePA
// Description: Set the host device physical address (initiator physical address)
//------------------------------------------------------------------------------

void SiiCecSetDevicePA ( uint16_t devPa )
{
    uint8_t     index;
    uint16_t    mask;

    pCec->physicalAddr = devPa;

    /* We were given the Host, or parent, PA, so we determine   */
    /* the direct child Physical Address field (A.B.C.D).       */

    mask = 0x00F0;
    for ( index = 1; index < 4; index++ )
    {
        if (( devPa & mask ) != 0)
        {
            break;
        }
        mask <<= 4;
    }

    pCec->paShift = (index - 1) * 4;
    pCec->paChildMask = 0x000F << pCec->paShift;

    //DEBUG_PRINT( MSG_STAT, "Device PA:[%04X] Mask:[%04X] Shift:[%d]\n", (int)devPa, (int)pCec->paChildMask, (int)pCec->paShift );

#if (INC_CEC_SWITCH == ENABLE)
    // PA change makes the switch to respond by sending new routing information after a short
    // pause, which is required to let the other HDMI devices to settle down after
    // HDP and following reconfiguration
    SiiCecSwitchSendRouteInfo();
    // TODO: Indicate for CEC dispatcher that new task is pending and has to be processed ASAP
    //SiiCecForceFastHandling();
#endif

}

//------------------------------------------------------------------------------
// Function:    SiiCecGetDeviceType
// Description: Return the CEC device type
//------------------------------------------------------------------------------

SiiCecDeviceTypes_t SiiCecGetDeviceType ( void )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    return( pCec->deviceType );
}
//------------------------------------------------------------------------------
// Function:    SiiCecSetSourceActive
// Description: Set the current source as Active in TPG case
//------------------------------------------------------------------------------

void SiiCecSetSourceActive( bool_t isActiveSource )
{
    pCec->isActiveSource = isActiveSource;
    if(pCec->isActiveSource)
    {
    	SiiCecSendActiveSource(pCec->logicalAddr, pCec->physicalAddr);
    }
}

//------------------------------------------------------------------------------
// Function:    SiiCecSetActiveSource
// Description: Set the CEC logical and physical address for active source device
//------------------------------------------------------------------------------

void SiiCecSetActiveSource( SiiCecLogicalAddresses_t activeSrcLA, uint16_t activeSrcPA )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;

    pCec->activeSrcLogical = activeSrcLA;
    pCec->activeSrcPhysical = activeSrcPA;
}

//------------------------------------------------------------------------------
// Function:    SiiCecSetActiveSourceLA
// Description: Set the CEC logical address for active source device
//------------------------------------------------------------------------------

void SiiCecSetActiveSourceLA( SiiCecLogicalAddresses_t activeSrcLA)
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    pCec->activeSrcLogical = activeSrcLA;
}

//------------------------------------------------------------------------------
//! @brief  Set the CEC physical address for active source device
//------------------------------------------------------------------------------
void SiiCecSetActiveSourcePA ( uint16_t activeSrcPa )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    pCec->activeSrcPhysical = activeSrcPa;
}


//------------------------------------------------------------------------------
// Function:    SiiCecGetActiveSource
// Description: Get the CEC logical address for active source device
//------------------------------------------------------------------------------

SiiCecLogicalAddresses_t SiiCecGetActiveSourceLA( void )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    return (pCec->activeSrcLogical);
}

//------------------------------------------------------------------------------
//! @brief  Return the CEC physical address for active source device
//------------------------------------------------------------------------------
uint16_t SiiCecGetActiveSourcePA ( void )
{
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    return( pCec->activeSrcPhysical );
}

//------------------------------------------------------------------------------
// Function:    SiiCecAdjacentPA
// Description: Returns true if passed physical devices are adjacent
//
//  Examples:
//
//  Upstream                        Downstream
//  --------                        ----------
//  0.0.0.0     is adjacent     to  2.0.0.0
//  0.0.0.0     is NOT adjacent to  2.1.0.0
//  2.1.0.0     is adjacent     to  2.0.0.0
//  2.1.0.0     is NOT adjacent to  2.2.0.0
//  2.1.0.0     is adjacent     to  2.1.1.0
//------------------------------------------------------------------------------

bool_t SiiCecAdjacentPA ( uint16_t paOne, uint16_t paTwo )
{
    uint8_t     i;
    uint16_t    one, two, paMask;
    bool_t      isChild = false;

    /* Compare nodes from A to D.  If they don't match and one of them  */
    /* is 0, they are adjacent.                                         */

    isChild = false;
    paMask = 0xF000;
    for ( i = 0; i < 4; i++ )
    {
        one = paOne & paMask;
        two = paTwo & paMask;

        /* If a previous node mismatched, but this node is not  */
        /* both 0, these two are not adjacent.                  */

        if ( isChild && ( one != two ))
        {
            isChild = false;
            break;
        }

        /* If the two nodes are NOT the same, and one of them is 0, */
        /* they MIGHT be adjacent, but we have to check for the     */
        /* remainder of the nodes to be the same on both.           */

        if ( one != two )
        {
            /* If one node is zero we have adjacent nodes.  */

            if (( one == 0 ) || ( two == 0 ))
            {
                isChild = true;
                break;
            }
        }
        paMask >>= 4;
    }
   // DEBUG_PRINT( CEC_MSG_DBG, "one: %04X  two: %04X  paMask: %04X\n", one, two, paMask );
    return( isChild );
}

//------------------------------------------------------------------------------
// Function:    SiiCecIsAdjacentLA
// Description: Returns true if passed logical device is in the adjacent rank
//              of devices.
//------------------------------------------------------------------------------

bool_t SiiCecIsAdjacentLA ( uint8_t logicalAddr )
{

    if ( logicalAddr > CEC_LOGADDR_UNREGORBC)
    {
        pCec->lastResultCode = RESULT_CEC_INVALID_LOGICAL_ADDRESS;
        return( false );
    }

    if ( !SiiCecAdjacentPA( pCec->physicalAddr, pCec->logicalDeviceInfo[ logicalAddr].cecPA ))
        {
            DEBUG_PRINT(
                CEC_MSG_DBG,
                "LA=%02X (%04X) is NOT adjacent to %02X (%04X)\n",
                logicalAddr, pCec->logicalDeviceInfo[ logicalAddr].cecPA, pCec->logicalAddr, pCec->physicalAddr
                );
            pCec->lastResultCode = RESULT_CEC_NOT_ADJACENT;
            return( false );
        }
    pCec->lastResultCode = RESULT_CEC_SUCCESS;
    return( true );
    }

//------------------------------------------------------------------------------
// Function:    SiiCecPortToAdjacentLA
// Description: Return the CEC logical address for a device that is adjacent
//              to the specified device port
// Parameters:  portIndex:  HDMI RX port number, 0-based
// Returns:     CEC Logical address for the port, or 0xFF if no LA found.
//------------------------------------------------------------------------------

uint8_t SiiCecPortToAdjacentLA ( int_t portIndex )
{
    int_t       thisLa;

    pCec->lastResultCode = RESULT_CEC_SUCCESS;

    for ( thisLa = 0; thisLa <= CEC_LOGADDR_UNREGORBC; thisLa++ )
    {
        // Does requested port has the same index as the current logical address?
        if ( pCec->logicalDeviceInfo[ thisLa].port == portIndex )
        {
            // Is the physical address adjacent to the requested port on our device?
            if ( SiiCecAdjacentPA( pCec->logicalDeviceInfo[ thisLa].cecPA, pCec->physicalAddr ))
            {
                break;
            }
        }
    }

    if ( thisLa >= CEC_LOGADDR_UNREGORBC )
    {
        thisLa = 0xFF;
    }

    return( thisLa );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Figures out whether two devices have relationship
//!             of a Sink and a Source.
//!
//!             If so, returns index of the Sink's
//!             input port that is connected to the Source output port
//!
//! @param[in]  sinkPhysAddr      - PA of the possible Sink device,
//! @param[in]  sourcePhysAddr    - PA of the possible Source device,
//! @param[out] pInputPortIndex   - (return value) pointer to an index of an input  port of the Sink
//!                                 0xFF - in the case when Sink-Source relationship hasn't been verified
//!
//! @retval     true, if devices are linked directly over some of the Sink's inputs
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

static bool_t CecIsLinkedDirectlyInPortGet(const uint16_t sinkPhysAddr, const uint16_t sourcePhysAddr, uint8_t *pInputPortIndex)
{
    // criteria: 1. Sink must be one level above the Source
    //           2. The input port number (= inputPortIdx+1) must be equal to
    //              the Source's physical address last nonzero digit (p. 8.7.2. of HDMI 1.4 spec)
    uint8_t     i;
    uint8_t     n = 0;
    uint16_t    mask = 0x000F;
    uint16_t    sourceMasked;
    uint16_t    sinkMasked;
    bool_t      isLinked = false;

    // crit.1 meets if and only if sink and source have one of the following patterns:
    // {0000, n000}, {x000, xn00}, {xy00, xyn0}, {xyz0, xyzn}
    for (i = 0; i < 4; i++)
    {
        sourceMasked = sourcePhysAddr & mask;
        sinkMasked   = sinkPhysAddr & mask;

        if (n == 0) // n point not found yet
        {
             // skip zero-zero pairs
             if ((sinkMasked == 0) && (sourceMasked !=0)) // first zero-nonzero pair found
             {
                 n = sourceMasked >> (4 * i); // input port number (nonzero)
                 *pInputPortIndex = n - 1;      // input port index to return
                 isLinked = true;             // potential detection
             }
             else if ((sinkMasked != 0) && (sourceMasked !=0))
             {
                break;
             }
             else if ((sinkMasked != 0) && (sourceMasked ==0))
             {
                break;
             }
        }
        else // all other digit pairs must be non-zero & equal
        {
            if ((sinkMasked == 0) || (sinkMasked != sourceMasked))
            {
                isLinked = false; // reset linked flag if wrong MSB pattern found
                *pInputPortIndex = 0xFF;
                break;
            }
        }

        mask <<= 4; // scan all 4 digits
    }


    return isLinked;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Selects an input port or ARC output port adjacent to the active audio source
//!
//! @retval     true  - success,
//! @retval     false - active source device isn't adjacent or can't be selected
//-------------------------------------------------------------------------------------------------

bool_t SiiCecPortToActSrcSelect(void)
{
    uint8_t inputPortIndex;
    bool_t  isLinkedToSink = false;
    bool_t  isLinkedToSource = false;
    bool_t  isSuccess = true;

    // Check if active source is adjacent to this device 
    isLinkedToSource = CecIsLinkedDirectlyInPortGet(pCec->physicalAddr, pCec->activeSrcPhysical, &inputPortIndex);//check if active source is child
    if (!isLinkedToSource)
    {
        isLinkedToSink = CecIsLinkedDirectlyInPortGet(pCec->activeSrcPhysical, pCec->physicalAddr, &inputPortIndex);//check if active source is parent
        if (!isLinkedToSink)
        {
            DEBUG_PRINT( CEC_MSG_DBG, "SiiCecPortToActSrcSelect(): Active source isn't adjacent to this device.\n");
            isSuccess = false;
        }
        else
        {   // Use ARC to receive audio
            DEBUG_PRINT( CEC_MSG_DBG, "Audio comes over Audio Return Channel.\n");
            //isSuccess = false; // If ARC isn't available
            isSuccess = true;
        }
    }
    else
    {   // Switch to Active Source
        SiiCecCbInputPortSet(inputPortIndex);
    }

    if (isSuccess)
    {
        DEBUG_PRINT(CEC_MSG_DBG, "\nActive Source: LA[%02X], PA[%04X], Port[%02X]\n", (int)pCec->activeSrcLogical, pCec->activeSrcPhysical, (int)pCec->portSelect);
    }

    return isSuccess;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Calculates physical address of a device, potential or real, that
//!             is adjacent to the reference device.
//!
//! @param[in]  basePhysAddr   - PA of the reference device,
//! @param[in]  isAtOutput     - if true, returns a device's PA adjacent to output port,
//!                              if false, returns a device's PA adjacent PA to
//!                              specified input port
//! @param[in]  inputPortIndex - zero-based index of the input port of the adjacent device
//!                             (ignored, if isAtOutput == true)
//!
//! @return     Physical Address of the Adjacent device,
//! @retval     0xFFFF - if the Adjacent device PA doesn't exist
//!                       (sink of a root or source of 5th level)
//-------------------------------------------------------------------------------------------------

uint16_t SiiCecAdjacentPhysAddrGet(const uint16_t basePhysAddr, const bool_t isAtOutput, const uint8_t inputPortIndex)
{
    uint16_t mask = 0xFFF0;
    uint16_t adjPhysAddr = 0xFFFF;
    uint8_t  n = 0;

    if (inputPortIndex > (SII_INPUT_PORT_COUNT -1 )) // illegal port index
    {
        return 0xFFFF;
    }

    // find the last non-zero digit in basePhysAddr
    while ((n < 4) && ((basePhysAddr & mask) == basePhysAddr))
    {
        n++;
        mask <<= 4;
    }

    if (isAtOutput)
    {
        if (basePhysAddr != 0x0000)
        {
            adjPhysAddr = basePhysAddr & mask;
        }
    }
    else // adjacent to input
    {
        if (n != 0) // if last digit is 0
        {
           adjPhysAddr = basePhysAddr | ((inputPortIndex + 1) << (4 * (n - 1)));
        }
    }

    return adjPhysAddr;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns longest possible time delay before the next invocation
//!             of the SiiCecXXXTaskProcess() function.
//!
//!             This function can be used to set a countdown timer and call
//!             the SiiCecXXXTaskProcess() on the timer's expiration event unless
//!             new CEC message is pending
//!
//! @param[in]  sysTimerCountMs  - current value of the system time counter (in milliseconds)
//! @param[in]  recallTimerCount - pointer to the recall time counter
//!
//! @return     time in milliseconds
//-------------------------------------------------------------------------------------------------

uint16_t SiiCecNextInvocationTimeGet(uint16_t sysTimerCountMs, CecTimeCounter_t recallTimerCount)
{
    uint32_t totalDelayMs;
    uint16_t invDelayMs;

    if (CecIsTimeCounterExpired(&recallTimerCount))
    {
        totalDelayMs = 0;
    }
    else
    {
        totalDelayMs =  (uint32_t) recallTimerCount.tRov * 0x10000 /* 2^16 */ +
                        (uint32_t) recallTimerCount.tRes * CEC_TIME_TCK_MS - (uint32_t) sysTimerCountMs;
    }

    // Apply limits to recall delay
    if (totalDelayMs < CEC_MIN_RECALL_TIME_MS)
    {
        invDelayMs = CEC_MIN_RECALL_TIME_MS;
    }
    else if (totalDelayMs > CEC_MAX_RECALL_TIME_MS)
    {
        invDelayMs = CEC_MAX_RECALL_TIME_MS;
    }
    else
    {
        invDelayMs = (uint16_t) totalDelayMs;
    }

    //DEBUG_PRINT(MSG_DBG_CEC, ("SiiCecNextInvocationTimeGet(): -- recall in -- %d ms\n", invDelayMs));

    return invDelayMs;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends a CEC message over the CEC bus using CPI
//!
//! @param[in]  opCode      - CEC operation code
//! @param[in]  argCount    - number of byte-size arguments in the CEC message
//! @param[in]  srcLa       - logical address of source device
//! @param[in]  destLa      - logical address of destination device
//! @param[in]  pCecMsg     - pointer to the CEC message frame (structure)
//-------------------------------------------------------------------------------------------------
void SiiCecCpiWriteEx(
    uint8_t opCode, uint8_t argCount,
    SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa,
    SiiCpiData_t *pCecMsg)
{
    pCecMsg->opcode          = opCode;
    pCecMsg->srcDestAddr     = MAKE_SRCDEST( srcLa, destLa );
    pCecMsg->argCount        = argCount;

    SiiDrvCpiWrite( pCecMsg );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Sends a CEC message over the CEC bus using CPI
//!
//! @param[in]  opCode      - CEC operation code
//! @param[in]  argCount    - number of byte-size arguments in the CEC message
//! @param[in]  destLogAddr - logical address of destination device
//! @param[in]  pCecMsg     - pointer to the CEC message frame (structure)
//-------------------------------------------------------------------------------------------------
uint16_t SiiCecCpiWrite(uint8_t opCode, uint8_t argCount, uint8_t destLogAddr, SiiCpiData_t *pCecMsg)
{
    pCecMsg->opcode          = opCode;
    pCecMsg->srcDestAddr     = MAKE_SRCDEST(SiiCecGetDeviceLA(), destLogAddr);
    pCecMsg->argCount        = argCount;

    return SiiDrvCpiWrite(pCecMsg);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Send a feature abort as a response to an inappropriate message
//!
//! @param[in]  opCode  - CEC operation code
//! @param[in]  reason  - abort reason code @see CEC_ABORT_REASON_t
//! @param[in]  srcLa   - logical address of the source device
//! @param[in]  destLa  - logical address of the destination device
//-------------------------------------------------------------------------------------------------
void SiiCecFeatureAbortSendEx (uint8_t opCode, uint8_t reason, SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa )
{
    SiiCpiData_t cecFrame;

    cecFrame.args[0]        = opCode;
    cecFrame.args[1]        = reason;
    cecFrame.opcode         = CECOP_FEATURE_ABORT;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( srcLa, destLa );
    cecFrame.argCount       = 2;

    SiiDrvCpiWrite( &cecFrame );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Send a feature abort as a response to an inappropriate message
//!
//! @param[in]  opCode      - CEC operation code
//! @param[in]  reason      - abort reason code @see CEC_ABORT_REASON_t
//! @param[in]  destLogAddr - logical address of the destination device
//-------------------------------------------------------------------------------------------------

void SiiCecFeatureAbortSend(uint8_t opCode, uint8_t reason, uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;

    cecFrame.args[0]       = opCode;
    cecFrame.args[1]       = reason;

    SiiCecCpiWrite(CECOP_FEATURE_ABORT, 2, destLogAddr, &cecFrame);
}


#if ( configSII_DEV_953x_PORTING == 1 )
bool_t AmTCecPortToActSrcSelect(void)
{
    uint8_t inputPortIndex;
    bool_t  isLinkedToSource = false;
    bool_t  isSuccess = true;
    xHMISystemParams SysParms;

    // Check if active source is adjacent to this device
    isLinkedToSource = CecIsLinkedDirectlyInPortGet(pCec->physicalAddr, pCec->activeSrcPhysical, &inputPortIndex);
    if (!isLinkedToSource)
    {
        SysParms = AmTCecGetSyspramsFromHMI();
        if (!SysParms.cec_arc)	//Check ARC settings
        {
            DEBUG_PRINT( CEC_MSG_DBG, "CEC ARC is Off and active source isn't adjacent to this device.\n");
            isSuccess = false;
        }
        else
        {  
            DEBUG_PRINT( CEC_MSG_DBG, "Audio comes over Audio Return Channel.\n");
            isSuccess = true;
        }
    }
    else
    {
    	SiiCecCbInputPortSet(inputPortIndex);
        isSuccess = true;
    }

    return isSuccess;
}


//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
void AmTCecEventPassToHMI(uint8_t event)
{
	xHMISrvEventParams srv_parms = { xHMI_EVENT_CEC, USER_EVENT_UNDEFINED, 0 };
	
   	srv_parms.event = event;
    	pHS_ObjCtrl->SendEvent(&srv_parms);
}

void AmTCecEventVolumeCtrl_HMI(  xHMISrvEventParams event ) 
{
	xHMISrvEventParams srv_parms = { xHMI_EVENT_CEC, USER_EVENT_UNDEFINED, 0 };
	
   	srv_parms.event = event.event;
    	srv_parms.params = event.params;
    	pHS_ObjCtrl->SendEvent(&srv_parms);
}



//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
xHMISystemParams AmTCecGetSyspramsFromHMI(void)
{
	return (pHS_ObjCtrl->GetSystemParams());
}


//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
POWER_STATE AmTCecGetSystemPowerStatus(void)
{
	return (pPowerHandle_ObjCtrl->get_power_state());
}
#endif

