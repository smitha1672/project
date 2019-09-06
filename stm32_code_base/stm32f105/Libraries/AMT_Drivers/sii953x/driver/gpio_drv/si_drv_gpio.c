//***************************************************************************
//!file     si_drv_gpio.c
//!brief    Silicon Image CPI Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_drv_gpio_internal.h"
#include "si_drv_internal.h"

//------------------------------------------------------------------------------
//  Driver Data
//------------------------------------------------------------------------------

// Available GPIO alternate pin functions

static uint16_t l_pinTypeMask =
    {
    SII_GPIO_ALT_MHL_CABLE_CONN0    |
    SII_GPIO_ALT_EXT_CLK            |
    SII_GPIO_ALT_SPI_SS             |

    SII_GPIO_ALT_SPI_SCLK           |
    SII_GPIO_ALT_SPI_SDO            |
    SII_GPIO_ALT_SPI_SDI            |
    SII_GPIO_ALT_SD1                |
    SII_GPIO_ALT_SD2                |
    SII_GPIO_ALT_SD3                |
    SII_GPIO_ALT_MUTEOUT            |
    SII_GPIO_ALT_I2S_SCK0           |

    SII_GPIO_ALT_I2S_WS0            |

    SII_GPIO_ALT_3D_RP_RIGHT        |
    SII_GPIO_ALT_3D_RP_ACTIVE       |
    SII_GPIO_ALT_3D_RP_LEFT         |
    SII_GPIO_ALT_3D_MP_RIGHT        |
    SII_GPIO_ALT_3D_MP_ACTIVE       |
    SII_GPIO_ALT_3D_MP_LEFT         |
    SII_GPIO_ALT_MP_MUTE            |
    SII_GPIO_ALT_RP_MUTE            |

    0
    };

//------------------------------------------------------------------------------
//  GPIO Driver Instance Data
//------------------------------------------------------------------------------

uint16_t    l_statusFlags = 0;

//------------------------------------------------------------------------------
// Function:    SiiDrvGpioInitialize
// Description:
//
// Returns:     TRUE if no problem
//------------------------------------------------------------------------------

bool_t SiiDrvGpioInitialize ( void )
{

    l_statusFlags = 0;

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvGpioStatus
// Description: Returns a status flag word containing GPIO driver-specific
//              information about the state of the device.
// Parameters:  none
// Returns:     Sixteen-bit status flags word for the GPIO Driver
//------------------------------------------------------------------------------

uint16_t SiiDrvGpioStatus ( void )
{
    uint16_t statusFlags = l_statusFlags;

    l_statusFlags = 0;          // INT flags only valid first time it is read.
    return( statusFlags );
}
//------------------------------------------------------------------------------
// Function:    SiiDrvGpioPinType
// ! @brief     Enable/disable the passed pin alternative function
// Parameters:  pins -- GPIO pins to be configured, GPIO0 == 0x0001,
//                      GPIO1 == 0x0002, etc.
//              type -- one of SiiGpioTypes_t
// Returns:     true if success, false if requested type not available for
//              one or more of the specified pin(s).
//------------------------------------------------------------------------------

bool_t SiiDrvGpioPinType ( uint16_t pins, SiiGpioPinTypes_t pinType )
{
    bool_t  success = true;
    uint8_t pinsPwdHi, pinsPwdLo, pinsAon, pinsPwdAud;

    pinsPwdHi   = (uint8_t)((pins >> 10) & MSK_ALTEN_12_10);
    pinsPwdLo   = (uint8_t)(pins >> 2);
    pinsAon     = (uint8_t)(pins) & MSK_ALTEN_0;
    pinsPwdAud  = (uint8_t)(((pins >> 1) & MSK_ALTEN_0) << 6);

    // Set the requested pin(s) to GPIO mode.
    if ( pinType == SII_GPIO_STANDARD )
    {
        // Clear all specified pins to GPIO functionality
        SiiRegBitsSet( REG_GPIO_AON_CTRL0, pinsAon, false );
        SiiRegBitsSet( REG_GPIO_ALTEN, pinsPwdLo, false );
        SiiRegBitsSet( REG_GPIO_ALTEN_DIR, pinsPwdHi, false );
        SiiRegBitsSet( REG_GPIO_DIR, pinsPwdAud, false );

    }
    else
    {
        // Determine if pins requested are valid against the pin type requested
        if (( pinType & pins & l_pinTypeMask ) != pins )
        {
            success = false;
        }
        else
        {
            // Enable the alternate pin function for the requested pin(s)
            SiiRegBitsSet( REG_GPIO_AON_CTRL0, pinsAon, true );
            SiiRegBitsSet( REG_GPIO_ALTEN, pinsPwdLo, true );
            SiiRegBitsSet( REG_GPIO_ALTEN_DIR, pinsPwdHi, true );
            SiiRegBitsSet( REG_GPIO_DIR, pinsPwdAud, true );
            // Choose primary or secondary alternate function.
            if (( pinType & SII_GPIO_ALTFUNCTION ) == SII_GPIO_ALTFUNCTION )
            {
                pinsPwdHi =  (pins & (SII_GPIO_PIN_9| SII_GPIO_PIN_10 | SII_GPIO_PIN_11 )) ? BIT_GPIO_SEC_SEL0 : 0;
                pinsPwdHi |= (pins & (SII_GPIO_PIN_1 | SII_GPIO_PIN_2 | SII_GPIO_PIN_3)) ? BIT_GPIO_SEC_SEL1 : 0;
                pinsPwdHi |= (pins & (SII_GPIO_PIN_4 | SII_GPIO_PIN_5 )) ? BIT_GPIO_SEC_SEL2 : 0;
                SiiRegBitsSet( REG_GPIO_PRI_SEC_SEL, pinsPwdHi, true );
            }
        }
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvGpioPinConfigure
// ! @brief     Configure the passed pins as specified by the mode.
// Parameters:  pins -- GPIO pins to be configured, GPIO0 == 0x0001,
//                      GPIO1 == 0x0002, etc.
//              mode -- logical 'OR' of SiiGpioModes_t values
//------------------------------------------------------------------------------

void SiiDrvGpioPinConfigure ( uint16_t pins, SiiGpioModes_t pinMode )
{
    uint8_t pinsAon, pinsEdge0, pinsEdge1, pinsEdge2;
    uint8_t pins92, pins1210, pins62, pins127;

    // Some precalculated bit positioning
    pins92      = (pins >> 2)   & MSK_GPIO_9_2;
    pins1210    = (pins >> 10)  & MSK_GPIO_12_10;
    pins62      = (pins << 1)   & MSK_GPIO_6_2;
    pins127     = (pins >> 7)   & MSK_GPIO_12_7;

    // Set the pull up/down mode requested
    if ( pinMode & ( SII_GPIO_PU | SII_GPIO_PD ))
    {
        // PU = up/down
        SiiRegBitsSet( REG_GPIO_AON_CTRL1,  (pins << 4) & MSK_PAD_PU_1_0, ((pinMode & SII_GPIO_PU) != 0 ));
        SiiRegBitsSet( REG_GPIO_PU_SEL,     pins92,   ((pinMode & SII_GPIO_PU) != 0 ));
        SiiRegBitsSet( REG_GPIO_PU_SEL_EN,  pins1210, ((pinMode & SII_GPIO_PU) != 0 ));

        // PE = true
        SiiRegBitsSet( REG_GPIO_AON_CTRL1,  (pins << 6) & MSK_PAD_PE_1_0, true );
        SiiRegBitsSet( REG_GPIO_PU_SEL_EN,  pins62, true );
        SiiRegBitsSet( REG_GPIO_PU_EN,      pins127, true );
    }
    else
    {
        // PE = false
        SiiRegBitsSet( REG_GPIO_AON_CTRL1,  (pins << 6) & MSK_PAD_PE_1_0, false );
        SiiRegBitsSet( REG_GPIO_PU_SEL_EN,  pins62, false );
        SiiRegBitsSet( REG_GPIO_PU_EN,      pins127, false );
    }

    // Pin direction INPUT
    if ( pinMode & SII_GPIO_INPUT )
    {
        // Set direction to INPUT
        SiiRegBitsSet( REG_GPIO_AON_CTRL0,  (pins << 2) & MSK_DIREN_1_0, true );
        SiiRegBitsSet( REG_GPIO_ALTEN_DIR,  pins62, true );
        SiiRegBitsSet( REG_GPIO_DIR,        pins127, true );

        // Inputs can also be interrupts
        if ( pinMode & ( SII_GPIO_INT_RISING_EDGE | SII_GPIO_INT_FALLING_EDGE ))
        {
            pinsAon     = 0;
            pinsEdge0   = 0;
            pinsEdge1   = 0;
            pinsEdge2   = 0;

            // Either edge, both, or none...
            if ( pinMode & SII_GPIO_INT_RISING_EDGE )
            {
                pinsAon     |=  (pins << 4)  & MSK_INT_EDGE_RISE_1_0;
                pinsEdge0   =   pins92;
                pinsEdge1   |=  pins1210;
            }
            if ( pinMode & SII_GPIO_INT_FALLING_EDGE )
            {
                pinsAon     |=  (pins << 6) & MSK_INT_EDGE_FALL_1_0;
                pinsEdge1   |=  pins62;
                pinsEdge2   =   pins127;
            }

            // Now perform the actual interrupt mode modifications
            SiiRegModify( REG_GPIO_AON_CTRL0, ((pins << 4) & MSK_INT_EDGE_RISE_1_0) | ((pins << 6) & MSK_INT_EDGE_FALL_1_0), pinsAon );
            SiiRegModify( REG_GPIO_INT_RE_EN,       pins92, pinsEdge0 );
            SiiRegModify( REG_GPIO_INT_RE_FE_EN,    pins1210 | pins62, pinsEdge1 );
            SiiRegModify( REG_GPIO_INT_FE_EN,       pins127, pinsEdge2 );
        }
    }

    // Pin direction OUTPUT
    else
    {
        // Set direction to INPUT
        SiiRegBitsSet( REG_GPIO_AON_CTRL0, (pins << 2) & MSK_DIREN_1_0, false );
        SiiRegBitsSet( REG_GPIO_ALTEN_DIR, pins62, false );
        SiiRegBitsSet( REG_GPIO_DIR, pins127, false );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable interrupts for the passed GPIO pins.  The pins must also be configured
//!             as interrupt pins using SiiDrvGpioPinConfigure()
//! @param[in]  pins    one or more GPIO pins to be interrupt enabled or disabled
//! @param[in]  enMask  bits corresponding to the pins parameter that are to be enabled.  To
//!                     disable interrupts for a pin, specify it in the pins parameter but
//!                     set the corresponding bit in this mask to 0.
//-------------------------------------------------------------------------------------------------
void SiiDrvGpioIntEnable ( uint16_t pins, uint16_t enMask )
{
    uint8_t pins92, pins1210, mask92, mask1210;

    // Some precalculated bit positioning
    pins92      = (pins >> 2)       & MSK_GPIO_9_2;
    pins1210    = (pins >> 10)      & MSK_GPIO_12_10;

    mask92      = (enMask >> 2)     & MSK_GPIO_9_2;
    mask1210    = (enMask >> 10)    & MSK_GPIO_12_10;

    // Set the appropriate interrupt mask state for these pins.
    SiiRegModify( REG_INT_ENABLE_9,  pins & MSK_GPIO_INT_1_0, enMask & MSK_GPIO_INT_1_0 );
    SiiRegModify( REG_INT_ENABLE_GPIO_1, pins92, mask92 );
    SiiRegModify( REG_INT_ENABLE_GPIO_2, pins1210, mask1210 );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvGpioPinRead
//! @brief      Read the GPIO pins specified
// Parameters:  pins -- GPIO pins to be read, GPIO0 == 0x0001,
//                      GPIO1 == 0x0002, etc.
//------------------------------------------------------------------------------

uint16_t    SiiDrvGpioPinRead ( SiiGpioPins_t pins )
{
    uint8_t     pinsPwdHi, pinsPwdLo, pinsAon;
    uint16_t    pinValue;

    pinsPwdHi   = (uint8_t)((pins >> 7) & 0x3F);
    pinsPwdLo   = (uint8_t)((pins >> 2) & 0x1F);
    pinsAon     = (uint8_t)(pins) & 0x01;

    pinValue = 0;
    if ( pinsAon )
    {
        pinValue = ((SiiRegRead( REG_GPIO_AON_CTRL1 ) & MSK_READ_0) >> 2) & pinsAon;
    }
    if ( pinsPwdLo )
    {
        pinValue |= (((SiiRegRead( REG_GPIO_WR_RD ) & MSK_READ_6_2) >> 3) & pinsPwdLo );
    }
    if ( pinsPwdHi )
    {
        pinValue |= ((SiiRegRead( REG_GPIO_RD ) & MSK_READ_12_7) & pinsPwdHi );
    }

    return( pinValue );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvGpioPinWrite
//! @brief      Read the GPIO pins specified
// Parameters:  pins -- GPIO pins to be read, GPIO0 == 0x0001,
//                      GPIO1 == 0x0002, etc.
//------------------------------------------------------------------------------

void SiiDrvGpioPinWrite ( SiiGpioPins_t pins, uint16_t pinValue )
{
    uint8_t     pinsPwdHi, pinsPwdLo, pinsAon;

    pinsPwdHi   = (uint8_t)(pins >> 10);
    pinsPwdLo   = (uint8_t)(pins >> 2);
    pinsAon     = (uint8_t)(pins) & 0x03;

    if ( pinsAon )
    {
        SiiRegModify( REG_GPIO_AON_CTRL1, pinsAon, (uint8_t)pinValue );
    }
    if ( pinsPwdLo )
    {
        SiiRegModify( REG_GPIO_WR, pinsPwdLo, (uint8_t)(pinValue >> 2) );
    }
    if ( pinsPwdHi )
    {
        SiiRegModify( REG_GPIO_WR_RD, pinsPwdHi, (uint8_t)(pinValue >> 10) );
    }
}

//------------------------------------------------------------------------------
// Function:    SiiDrvGpioPinIntStatus
//! @brief      Read the interrupt status of the specified GPIO pins
// Parameters:  pins -- GPIO pins to be read, GPIO0 == 0x0001,
//                      GPIO1 == 0x0002, etc.
//------------------------------------------------------------------------------

uint16_t    SiiDrvGpioPinIntStatus ( SiiGpioPins_t pins )
{
    uint8_t     pinsPwd456789, pinsPwd123, pinsAon, pinsPwd10_11;
    uint16_t    pinValue;


    pinsAon     = (uint8_t)(pins) & BIT_GPIO_INT_0;

    pinsPwd123   = (uint8_t)((pins >> 1) << 4) ;

    pinsPwd456789   = (uint8_t)((pins >> 4) << 2);  // GPIOs

    pinsPwd10_11   = (uint8_t)((pins >> 10) & 0x03); // GPIOs

    pinValue = 0;

    if ( pinsAon )
    {
        pinValue = SiiRegRead( REG_INT_STATUS_9 ) & pinsAon;
    }
    if ( pinsPwd123 )
    {
    	pinValue |= (((uint16_t)(SiiRegRead( REG_INT_STATUS_GPIO_2 ) & pinsPwd123)) >> 3);
    }
    if ( pinsPwd456789 )
    {
        pinValue |= (((uint16_t)(SiiRegRead( REG_INT_STATUS_GPIO_1 ) & pinsPwd456789)) << 2);
    }
    if ( pinsPwd10_11 )
    {
        pinValue |= (((uint16_t)(SiiRegRead( REG_INT_STATUS_GPIO_2 ) & pinsPwd10_11)) << 10);
    }

    return( pinValue );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvGpioPinIntClear
//! @brief      Clear the interrupt status of the specified GPIO pins
// Parameters:  pins -- GPIO pins to be read, GPIO0 == 0x0001,
//                      GPIO1 == 0x0002, etc.
//------------------------------------------------------------------------------

void SiiDrvGpioPinIntClear ( SiiGpioPins_t pins )
{
    uint8_t     pinsPwd456789, pinsPwd123, pinsAon, pinsPwd10_11;

    pinsAon     = (uint8_t)(pins) & BIT_GPIO_INT_0;
    pinsPwd123   = (uint8_t)((pins >> 1) << 4) ;
    pinsPwd456789   = (uint8_t)((pins >> 4) << 2);  // GPIOs
    pinsPwd10_11   = (uint8_t)((pins >> 10) & 0x03); // GPIOs

    if ( pinsAon )
    {
        SiiRegModify( REG_INT_STATUS_9, pinsAon, pinsAon );
    }
    if ( pinsPwd123 )
    {
    	SiiRegModify( REG_INT_STATUS_GPIO_2, pinsPwd123, pinsPwd123 );
    }
    if ( pinsPwd456789 )
    {
    	SiiRegModify( REG_INT_STATUS_GPIO_1, pinsPwd456789, pinsPwd456789 );
    }
    if ( pinsPwd10_11 )
    {
    	SiiRegModify( REG_INT_STATUS_GPIO_2, pinsPwd10_11, pinsPwd10_11 );
    }
}

//------------------------------------------------------------------------------
// Function:    DrvGpioProcessInterrupts
// Description:
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void DrvGpioProcessInterrupts ( void )
{
    uint16_t    intStatus = 0;

    // Check GPIO interrupts.
    intStatus = SiiDrvGpioPinIntStatus(
    		            SII_GPIO_PIN_0  |   SII_GPIO_PIN_1  |
    		            SII_GPIO_PIN_2  |   SII_GPIO_PIN_3  |
    		            SII_GPIO_PIN_4  |   SII_GPIO_PIN_5  |
    		            SII_GPIO_PIN_6  |   SII_GPIO_PIN_7  |
    		            SII_GPIO_PIN_8  |   SII_GPIO_PIN_9  |
    		            SII_GPIO_PIN_10 |   SII_GPIO_PIN_11
            );

    // Update a change in GPIO status.
    if ( intStatus )
    {
        SiiDrvGpioPinIntClear( intStatus );
    }

    if ( intStatus )
    {
        l_statusFlags |= intStatus;
    }

}



