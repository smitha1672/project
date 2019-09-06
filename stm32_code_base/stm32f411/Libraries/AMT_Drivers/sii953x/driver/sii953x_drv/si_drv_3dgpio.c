//***************************************************************************
//!file     si_drv_3dgpio.c
//!brief    Sii9535 driver 3D API.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_drv_internal.h"
#include "si_gpio.h"
#include "si_drv_gpio_internal.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Enable/disable the GPIOs for 3D marker operation.  The marker parameters must
//!             be configured prior to enabling the markers.  This function only controls the
//!             GPIO pins, it does not affect 3D detection or VS insertion into data stream.
//!             If the VS insertion feature is configured, the VS GPIO is enabled.  For disable,
//!             all three GPIOs are disabled.
//! @param[in]  enableFlags - bit 0: enable/disable the 3D GPIOs,
//!                           bit 6: 0 - operate on MP
//!                           bit 6: 1 - operate on RP
//-------------------------------------------------------------------------------------------------
void SiiDrvDevice3dMarkerGpioEnable ( uint8_t enableFlags )
{
    uint16_t    sii3dGpios, siiPinModes;

    sii3dGpios  = (enableFlags & SET_SUBPIPE_OPTIONS) ?
        (SII_GPIO_PIN_9 | SII_GPIO_PIN_11) : (SII_GPIO_PIN_1 | SII_GPIO_PIN_3);
    siiPinModes = (enableFlags & SET_SUBPIPE_OPTIONS) ?
        (SII_GPIO_ALT_3D_RP_LEFT | SII_GPIO_ALT_3D_RP_RIGHT) :
        (SII_GPIO_ALT_3D_MP_LEFT | SII_GPIO_ALT_3D_MP_RIGHT);
    if ( enableFlags & 0x01 )
    {
        // Add in VS insertion if it is enabled
        if ((( enableFlags & SET_SUBPIPE_OPTIONS) == 0) &&
            ( SiiRegRead( REG_3D_MARKER_CTRL ) & BIT_EN_3D_VS_INSERT ))
        {
            sii3dGpios  |= SII_GPIO_PIN_2;
            siiPinModes |= SII_GPIO_ALT_3D_MP_ACTIVE;
        }
        if (( enableFlags & SET_SUBPIPE_OPTIONS) &&
            ( SiiRegRead( REG_3D_MARKER_CTRL_RP ) & BIT_EN_3D_VS_INSERT ))
        {
            sii3dGpios  |= SII_GPIO_PIN_10;
            siiPinModes |= SII_GPIO_ALT_3D_RP_ACTIVE;
        }

        // Enable GPIOs as alternate outputs
        SiiDrvGpioPinConfigure( sii3dGpios, SII_GPIO_OUTPUT );
        SiiDrvGpioPinType( sii3dGpios, siiPinModes );
    }
    else
    {
        // Always disable the VS GPIO
    	sii3dGpios  |= (( enableFlags & SET_SUBPIPE_OPTIONS) ? SII_GPIO_PIN_10 : SII_GPIO_PIN_2 );

        // Disable 3D GPIOs.  Set them up as inputs so that they do not drain power.
        SiiDrvGpioPinConfigure( sii3dGpios, SII_GPIO_INPUT );
        SiiDrvGpioPinType( sii3dGpios, SII_GPIO_STANDARD );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Configure the 3D marker operation parameters.
//! @note       Only updates settings if ENABLE_3D_MARKER_SUPPORT is specified.
//-------------------------------------------------------------------------------------------------
void SiiDrvDevice3dMarkerConfigure( uint16_t optionFlags, uint16_t markerDelay720p, uint16_t markerDelay1080p )
{
    uint8_t     detectBits, delay, saveOppositeState;
    SiiReg_t    markerCtrlReg, oppositeReg;

    markerCtrlReg   = (optionFlags & SET_SUBPIPE_OPTIONS) ? REG_3D_MARKER_CTRL_RP : REG_3D_MARKER_CTRL;
    oppositeReg     = (optionFlags & SET_SUBPIPE_OPTIONS) ? REG_3D_MARKER_CTRL : REG_3D_MARKER_CTRL_RP;

    // Disable while updating delay values.
    SiiRegBitsSet( markerCtrlReg, BIT_EN_3D_MARKER_SUPPORT, false );
    saveOppositeState = SiiRegRead( oppositeReg );
    SiiRegBitsSet( oppositeReg, BIT_EN_3D_MARKER_SUPPORT, false );

        // Update delay values
        SiiRegWrite( REG_3D_720P_MARKER_DLY_LO, (uint8_t)markerDelay720p );
        delay = 0xA0 | ((markerDelay720p >> 8) & 0x0F);     // Add in the mandated reserved bits in [7:4]
        SiiRegWrite( REG_3D_720P_MARKER_DLY_HI, delay );
        SiiRegWrite( REG_3D_1080P_MARKER_DLY_LO, (uint8_t)markerDelay1080p );

    // 1080p HI byte also includes a 3D detect flag...
    delay = (optionFlags & SET_SUBPIPE_OPTIONS) ? BIT_SKIP_3D_STRUCT_CHK_RP : BIT_SKIP_3D_STRUCT_CHK_MP;
    delay = BIT_ENABLE_300MHZ_3D_SUPPORT | ((optionFlags & DISABLE_PACKED_FORMAT_CHK) ? delay : 0);
    delay |= ((markerDelay1080p >> 8) & 0x0F);
    SiiRegWrite( REG_3D_1080P_MARKER_DLY_HI, delay );

    // Configure the packet recognition logic
    detectBits  =   (optionFlags & DISABLE_VSIF_PACKET_CHK) ? BIT_SKIP_VSI_HEADER_CHK : 0;
    detectBits  |=  (optionFlags & DISABLE_IEEE_PACKET_CHK) ? BIT_SKIP_IEEE_ID_CHK : 0;
    detectBits  |=  (optionFlags & DISABLE_3D_FORMAT_CHK) ? BIT_SKIP_HDMI_3D_FORMAT_CHK : 0;
    detectBits  |=  (optionFlags & INSERT_3D_VS) ? BIT_EN_3D_VS_INSERT : 0;
    SiiRegModify(
        markerCtrlReg,
        BIT_SKIP_VSI_HEADER_CHK | BIT_SKIP_IEEE_ID_CHK | BIT_SKIP_HDMI_3D_FORMAT_CHK | BIT_EN_3D_VS_INSERT,
        detectBits
        );

    // Ensure that the main 3D detection logic for 3D IEEE packets is enabled.  Although this
    // does not directly enable the 3D markers, it is required for the port processor to
    // capture 3D packets of any type, so that the marker logic can filter frame-packed
    // packets from the 3D packets. Also enable the VSI buffer to automatically clear after
    // 2 or 4 frames are received without a 3D packet.
    if ( optionFlags & ENABLE_3D_MARKER_SUPPORT )
    {
        SiiRegBitsSet( REG_CLR_PACKET_BUFFER, BIT_VSI_ID_CHK_EN | BIT_VSI_CLR_EN, true );
    }

    // Enable/disable markers as requested
    SiiRegBitsSet( markerCtrlReg, BIT_EN_3D_MARKER_SUPPORT, (( optionFlags & ENABLE_3D_MARKER_SUPPORT ) != 0 ));

    // Restore state of opposite set of markers
    SiiRegWrite( oppositeReg, saveOppositeState );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return current state of hardware 3D marker controls.
//-------------------------------------------------------------------------------------------------
void SiiDrvDeviceGet3dMarkerConfiguration( uint16_t *pOptionFlags, uint16_t *pMarkerDelay720p, uint16_t *pMarkerDelay1080p )
{
    uint8_t     detectBits;
    uint16_t    flags, delay;
    SiiReg_t    markerCtrlReg;

    markerCtrlReg   = (*pOptionFlags & SET_SUBPIPE_OPTIONS) ? REG_3D_MARKER_CTRL_RP : REG_3D_MARKER_CTRL;

    detectBits = SiiRegRead( markerCtrlReg );
    flags = (detectBits & BIT_SKIP_VSI_HEADER_CHK) ? DISABLE_VSIF_PACKET_CHK : 0;
    flags |= (detectBits & BIT_SKIP_IEEE_ID_CHK) ? DISABLE_IEEE_PACKET_CHK : 0;
    flags |= (detectBits & BIT_SKIP_HDMI_3D_FORMAT_CHK) ? DISABLE_3D_FORMAT_CHK : 0;
    flags |= (detectBits & BIT_EN_3D_MARKER_SUPPORT) ? ENABLE_3D_MARKER_SUPPORT : 0;
    flags |= (detectBits & BIT_EN_3D_VS_INSERT) ? INSERT_3D_VS : 0;

    // Packed format check bit for both pipes is in the same register
    if (*pOptionFlags & SET_SUBPIPE_OPTIONS)
    {
        flags |= (SiiRegRead( REG_3D_1080P_MARKER_DLY_HI ) & BIT_SKIP_3D_STRUCT_CHK_RP) ? DISABLE_PACKED_FORMAT_CHK : 0;
    }
    else
    {
        flags |= (SiiRegRead( REG_3D_1080P_MARKER_DLY_HI ) & BIT_SKIP_3D_STRUCT_CHK_MP) ? DISABLE_PACKED_FORMAT_CHK : 0;
    }
    *pOptionFlags = flags;

    delay = SiiRegRead( REG_3D_720P_MARKER_DLY_HI ) & 0x0F;
    delay = (delay << 8) | SiiRegRead( REG_3D_720P_MARKER_DLY_LO );
    *pMarkerDelay720p = delay;

    delay = SiiRegRead( REG_3D_1080P_MARKER_DLY_HI ) & 0x0F;
    delay = (delay << 8) | SiiRegRead( REG_3D_1080P_MARKER_DLY_LO );
    *pMarkerDelay1080p = delay;
}

