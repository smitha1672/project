/******************************************************************************/
//!file     si_drv_cbus.c
//!brief    SiI9687 CBUS Driver.
//
// NOTE: This driver has a dependency on the Switch driver for the 9687
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2011, Silicon Image, Inc.  All rights reserved.
/******************************************************************************/

#include "string.h"
#include "si_drv_cbus_internal.h"
#include "si_cbus_enums.h"
#include "si_drv_internal.h"
#include "si_drv_switch.h"
#include "si_drv_gpio.h"
#include "si_cbus_component.h"
#include "si_drv_cbus.h"

//------------------------------------------------------------------------------
//  CBUS Driver Instance Data
//------------------------------------------------------------------------------

CbusDrvInstanceData_t cbusDrvInstance[SII_NUM_CBUS];
CbusDrvInstanceData_t *pDrvCbus = &cbusDrvInstance[0];

//------------------------------------------------------------------------------
//  CBUS Driver local Data
//------------------------------------------------------------------------------

static uint16_t cbusInitCbusRegsList [32] =
{
    REG_CBUS_DEVICE_CAP_1, 		MHL_VERSION,
    REG_CBUS_DEVICE_CAP_2,		MHL_DEV_CAT_POW,			// MHL PLIM0/PLIM1 needs to be set according to VBUS power it can supply
    REG_CBUS_DEVICE_CAP_3,      MHL_DEV_CAT_ADOPTER_ID_H,
    REG_CBUS_DEVICE_CAP_4,      MHL_DEV_CAT_ADOPTER_ID_L,
    REG_CBUS_DEVICE_CAP_5,      MHL_VID_LINK_MODE,
    REG_CBUS_DEVICE_CAP_6,      MHL_AUDIO_LINK_MODE,    // Bug 33183 - Melbourne SiI9535 | UTS800 SL850 MHL CBUS CTS test | 6.3.1.1 fails.
    REG_CBUS_DEVICE_CAP_7,      MHL_VIDEO_TYPE,
    REG_CBUS_DEVICE_CAP_8,      MHL_LOG_DEV_MAP,
    REG_CBUS_DEVICE_CAP_9,      MHL_BANDWIDTH_LIMIT,
    REG_CBUS_DEVICE_CAP_A,      MHL_FEATURE_SUPPORT,
    REG_CBUS_DEVICE_CAP_B,      MHL_DEVICE_ID_H,
    REG_CBUS_DEVICE_CAP_C,      MHL_DEVICE_ID_L,
    REG_CBUS_DEVICE_CAP_D,      MHL_SCRATCHPAD_SIZE,
    REG_CBUS_DEVICE_CAP_E,      MHL_INT_STAT_SIZE,
    REG_CBUS_DEVICE_CAP_F,      0x00
};


//------------------------------------------------------------------------------
// Description: Set the active cbus port
// Parameters:  instanceIndex
// Returns:     true if legal index, false if index value is illegal.
//------------------------------------------------------------------------------
void SiiDrvCbusConfigure ( uint8_t port )
{
    pDrvCbus->port = port;
}

//------------------------------------------------------------------------------
// Description: Set the driver global data pointer to the requested instance.
// Parameters:  instanceIndex
// Returns:     true if legal index, false if index value is illegal.
//------------------------------------------------------------------------------

bool_t SiiDrvCbusInstanceSet ( uint_t instanceIndex )
{
    if ( instanceIndex < SII_NUM_CBUS )
    {
        SiiRegInstanceSet( CBUS_PAGE, instanceIndex );
        pDrvCbus = &cbusDrvInstance[ instanceIndex];
        pDrvCbus->instanceIndex = instanceIndex;
        return( true );
    }

    return( false );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusInstanceGet
// Description: Return the current instance index.
// Parameters:  none
// Returns:     instance index.
//------------------------------------------------------------------------------

int_t SiiDrvCbusInstanceGet ( void )
{
    return( pDrvCbus->instanceIndex );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCbusStatus
// Description: Returns a status flag word containing CBUS driver-specific
//              information about the state of the device.
// Parameters:  none
// Returns:     Sixteen-bit status flags word for the CBUS Driver
//------------------------------------------------------------------------------

uint16_t SiiDrvCbusStatus ( void )
{
    uint16_t statusFlags;

    statusFlags = pDrvCbus->statusFlags;
//    pDrvCbus->statusFlags &= ~SiiCBUS_INT;  // INT flag only valid first time it is read.

    return( statusFlags );
}

//------------------------------------------------------------------------------
// Description: Initialize the CBUS hardware for the current instance.
// Returns:     TRUE if no problem
//
// Note:        Requires that SiiDrvCbusInstanceSet() is called prior to this call
//------------------------------------------------------------------------------
bool_t SiiDrvCbusInitialize ( void )
{
    uint_t  instanceIndex, index;
    uint8_t mhlPortBits;

    // Clear instance data
    instanceIndex = pDrvCbus->instanceIndex;        // Remember our instance
    memset( pDrvCbus, 0, sizeof( CbusDrvInstanceData_t ));
    pDrvCbus->instanceIndex = instanceIndex;        // Restore our instance

/*
    //Set Min and Max discover pulse width
    //This gives us ~153.5us if OSC=20MHz and ~139.6us if OSC=22MHz (20MHz + 10%).
    SiiRegWrite( REG_CBUS_DISC_PWIDTH_MIN, 0x4E );
    SiiRegWrite( REG_CBUS_DISC_PWIDTH_MAX, 0xC0 );

    // Set default CBUS bit offset
    SiiRegModify(REG_CBUS_LINK_CTRL11, MSK_CBUS_LINK_BIT_OFFSET, 0x4 << SFT_CBUS_LINK_BIT_OFFSET);
    */

    // Setup local DEVCAP registers for read by the peer per rc7
    for ( index = 0; index < (sizeof( cbusInitCbusRegsList) / 2); index += 2 )
    {
        SiiRegWrite( cbusInitCbusRegsList[ index], cbusInitCbusRegsList[ index + 1] );
    }

    // Enable the VS commands, all interrupts, and clear legacy
    SiiRegWrite( REG_CBUS_INTR_0_MASK, 0xFF );      // Enable desired interrupts
    SiiRegWrite( REG_CBUS_INTR_1_MASK, 0x4C );      // Enable desired interrupts

    //SiiRegWrite( REG_CBUS_SET_INT_0_MASK, 0xFF );   // Enable desired interrupts for SET_INT
    //SiiRegWrite( REG_CBUS_SET_INT_1_MASK, 0xFF );   // Enable desired interrupts for SET_INT


    //DEBUG_PRINT( MSG_ALWAYS, "\nINITIALIZING MHL\n" );
    mhlPortBits = SiiDrvCbusPortSelectBitsGet();      // Get the MHL selected port bit-field.

   // DEBUG_PRINT( MSG_ERR, "\nMHL: Ports Selected from NVRAM: %d \n",mhlPortBits  );

    SiiRegWrite( REG_MHL_1X2X_PORT_REG2, 0x00 );

	//Enable Cbus Cable sense
    SiiRegWrite( REG_CH0_INTR4_MASK, mhlPortBits );

    // Set up both MHL GPIOs for cable sense.
//  TODO: GPIO used for MHL Cable Detect
    SiiDrvGpioPinType( SII_GPIO_PIN_0, SII_GPIO_ALT_MHL_CABLE_CONN0 );
   
    SiiDrvGpioPinConfigure(
            SII_GPIO_PIN_0,
            SII_GPIO_INPUT | SII_GPIO_PD | SII_GPIO_INT_RISING_EDGE | SII_GPIO_INT_FALLING_EDGE );

    SiiDrvGpioIntEnable( SII_GPIO_PIN_0, SII_GPIO_PIN_0 );

    // Override CBUS port select over NVRAM selection. Select Port 0 and Port 1
    SiiRegWrite( REG_HDMIM_HPD_OVRD_CTRL, (BIT_MHL_PORT_SEL_FORCE | BIT_MHL_SEL_PORT0 | BIT_MHL_SEL_PORT1) );

    // use 1/3x clock input in MHL mode0: 1x input (default)1:1/3x input clock in MHL
   // SiiRegWrite( REG_TMDS0_CTRL2, REG_MHL_TEST_SOFT_CTRL_1X);

    // PLL configuration
   // SiiRegWrite( REG_TMDS0_CTRL3, 0x00);

    // Packed Pixed mode
    //SiiRegWrite( REG_PAUTH_MHL_CFG2, 0xC1);

    // Packed Pixed mode
    //SiiRegWrite( REG_PAUTH_MHL_CFG3, 0x01);




    return( true );
}
