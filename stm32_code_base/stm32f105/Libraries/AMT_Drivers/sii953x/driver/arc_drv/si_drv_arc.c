//******************************************************************************
// !file    si_drv_arc.c
// !brief   Silicon Image ARC driver functions for the Sii9535
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_drv_arc.h"
#include "si_drv_arc_internal.h"
#include "si_drv_switch.h"


//------------------------------------------------------------------------------
//  ARC Driver Instance Data
//------------------------------------------------------------------------------

ArcDrvInstanceData_t arcDrvInstance[SII_NUM_ARC];
ArcDrvInstanceData_t *pDrvArc = &arcDrvInstance[0];

//------------------------------------------------------------------------------
//! @brief  Return the current instance index.
//------------------------------------------------------------------------------
int SiiDrvArcInstanceGet ( void )
{
    return( pDrvArc->instanceIndex );
}

//------------------------------------------------------------------------------
//! @brief  Set the component global data pointer to the requested instance.
//! @param[in]  instanceIndex
//! @return     true if legal index, false if index value is illegal.
//------------------------------------------------------------------------------
bool_t SiiDrvArcInstanceSet ( int_t instanceIndex )
{
    if ( instanceIndex < SII_NUM_ARC )
    {
        pDrvArc = &arcDrvInstance[instanceIndex];
        pDrvArc->instanceIndex = instanceIndex;
        return( true );
    }

    return( false );
}

//------------------------------------------------------------------------------
//! @brief  Places the ARC driver into standby mode if available
//! @return     true if the ARC driver entered standby mode successfully,
//!             or false if some failure occurred.
//------------------------------------------------------------------------------
bool_t SiiDrvArcStandby ( void )
{
    SiiRegModify( REG_ARC_SPDIF_CTRL, BIT_SPDIF_OUT_MODE_SEL_0, CLEAR_BITS );
    return( true );
}

//------------------------------------------------------------------------------
//! @brief  Causes operation of the ARC driver to return to the state
//!         it was prior to the previous call to SiiDrvArcStandby.
//! @return     true if the ARC driver successfully exited standby mode,
//!             or false if some failure occurred.
//------------------------------------------------------------------------------
bool_t SiiDrvArcResume ( void )
{
    SiiRegModify( REG_ARC_SPDIF_CTRL, BIT_SPDIF_OUT_MODE_SEL_0, SET_BITS );
    return( true );
}

//------------------------------------------------------------------------------
//! @brief  Initialize ARC driver
//------------------------------------------------------------------------------
bool_t SiiDrvArcInitialize ( void )
{
    int instanceIndex;

    // Clear instance data
    instanceIndex = pDrvArc->instanceIndex;         // Remember our instance
    memset( pDrvArc, 0, sizeof( ArcDrvInstanceData_t ));
    pDrvArc->instanceIndex = instanceIndex;         // Restore our instance

    // Start off with ARC drivers disabled
    return( SiiDrvArcStandby() );
}

//------------------------------------------------------------------------------
//! @brief  Configure an ARC pin's type and source or outputs.
//!         For normal mode SPDIF (HDMI out) call with (x,x,ARC_DISABLE_MODE)
//! @param[in]: arcPin       - 0,1: ARC pin in focus
//! @param[in]: spdifSelPin  - 0,1: SPDIF In/Out port selection
//! @param[in]: mode         - ARC_TX_MODE: enables ARC-TX using pins specified
//!                            ARC_RX_MODE: enables ARC-RX using pins specified
//!                            ARC_DISABLE_MODE: disables ARC associated with the pins
//------------------------------------------------------------------------------
bool_t SiiDrvArcConfigureArcPin( int_t arcPin, int_t spdifSelPin, int_t mode )
{
    switch ( mode )
    {
    	case ARC_TX_MODE:
            //TRACE_DEBUG((0, "SiiDrvArcConfigureArcPin - ARC_TX_MODE"));
			// ARC Transmitter.
    		SiiDrvArcResume ();
    		SiiRegWrite( REG_ARC_CTRL7, BIT_ARC_TX_SELECT );
			break;

    	case ARC_RX_MODE:
            //TRACE_DEBUG((0, "SiiDrvArcConfigureArcPin - ARC_RX_MODE"));
			// ARC Receiver.
    		SiiDrvArcResume ();
			// Enable specified S/PDIF pin as output from ARC source
    		SiiRegWrite( REG_ARC_CTRL7, BIT_ARC_RX_SELECT );
			break;

    	case ARC_DISABLE_MODE:
            //TRACE_DEBUG((0, "SiiDrvArcConfigureArcPin - ARC_DISABLE_MODE"));
			// RX Normal mode. Disable ARC for normal audio to work.
            // Select Audio Extraction as source for specified S/PDIF pin output
    		SiiDrvArcStandby ();
    		SiiRegWrite( REG_ARC_CTRL7, BIT_ARC_RX_TX_CLEAR );
			break;
    }

    return( true );
}

