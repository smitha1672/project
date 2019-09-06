//***************************************************************************
//!file     si_cdc_component.c
//!brief    Silicon Image CDC Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_cdc_component.h"
#include "si_cdc_internal.h"

//------------------------------------------------------------------------------
//  Module Data
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  CDC Component Instance Data
//------------------------------------------------------------------------------

CdcInstanceData_t cdcInstance[SII_NUM_CDC] =
{
{
	0,							// structVersion
    SII_SUCCESS,             // lastResultCode Contains the result of the last API function called
    0,                          // statusFlags
}
};
CdcInstanceData_t *pCdc = &cdcInstance[0];

//------------------------------------------------------------------------------
// Function:    SiiCdcConfigure
// Description:
// Parameters:  none
// Returns:     true if the configuration was successful, or false if some
//				failure occurred
//------------------------------------------------------------------------------

bool_t SiiCdcConfigure ( void )
{

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCdcInitialize
// Description:
// Parameters:  none
// Returns:     It returns true if the initialization is successful, or false
//				if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCdcInitialize ( void )
{

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCdcStatus
// Description: Returns a status flag word containing CDC-specific
//				information about the state of the device.
// Parameters:  none
// Returns:     Sixteen-bit status flags word for the CDC Component
//------------------------------------------------------------------------------

bool_t SiiCdcStatus ( void )
{

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCdcStandby
// Description: Places the CDC component into standby mode if available
// Parameters:  none
// Returns:     true if the CDC component entered standby mode successfully,
//				or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCdcStandby ( void )
{

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCdcResume
// Description: Causes operation of the CDC component to return to the state
//				it was prior to the previous call to SiiCdcStandby.
// Parameters:  none
// Returns:     true if the CDC component successfully exited standby mode,
//				or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiCdcResume ( void )
{

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiCdcInstanceSet
// Description: Brings device instance record into current context. This function
//              is useful when multiple HDMI outputs are present
// Parameters:  componentIndex	- Index of a CDC component instance
// Returns:     none
//------------------------------------------------------------------------------

void SiiCdcInstanceSet ( uint16_t componentIndex )
{
    if ( componentIndex < NUMBER_OF_DEVICES)
    {
        pCdc = &cdcInstance[componentIndex];
    }
}


//------------------------------------------------------------------------------
// Function:    SiiCdcGetLastResult
// Description: Returns the result of the last SiiCdcxxx function called
// Parameters:  none
// Returns:     Result of last CDC component function.
//------------------------------------------------------------------------------

bool_t SiiCdcGetLastResult ( void )
{

	return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get current Physical Address of the device.
//-------------------------------------------------------------------------------------------------

uint16_t  SiiCdcPhysAddrGet(void)
{
    return pCdc->physAddr;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get I/O ports configuration and capability information.
//-------------------------------------------------------------------------------------------------

CdcIoConfig_t *SiiCdcIoPortsGet(void)
{
    return &pCdc->ports;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get HEC records.
//-------------------------------------------------------------------------------------------------

CdcHecDescriptor_t *SiiCdcHecRecordsGet(void)
{
    return pCdc->aHec;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get number of HECs in the record.
//-------------------------------------------------------------------------------------------------

uint8_t SiiCdcNumberOfHecsGet(void)
{
    return pCdc->numberOfHecs;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get status & capability of this device.
//-------------------------------------------------------------------------------------------------

CdcDeviceStatus_t *SiiCdcDeviceStatusGet(void)
{
    return &pCdc->deviceStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get List of Device Physical Addresses found as a result of search tasks.
//-------------------------------------------------------------------------------------------------

CdcFoundListEntry_t *SiiCdcFoundHostListGet(void)
{
    return pCdc->cdcFoundHostsList;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get number of Host and/or ENC capable devices.
//-------------------------------------------------------------------------------------------------

uint8_t  SiiCdcNumberOfFoundHostsGet(void)
{
    return pCdc->numberOfFoundHosts;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get list of devices Alive status.
//-------------------------------------------------------------------------------------------------

CdcPhysAddrTime_t *SiiCdcAliveDeviceListGet(void)
{
    return pCdc->aAliveDevice;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Get number of nonempty records in aAliveDevice[] list.
//-------------------------------------------------------------------------------------------------

uint8_t SiiCdcNumberOfAliveDevGet(void)
{
    return pCdc->numberOfAliveDev;
}

