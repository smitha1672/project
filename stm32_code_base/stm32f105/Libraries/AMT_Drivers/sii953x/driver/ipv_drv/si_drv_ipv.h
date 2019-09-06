//***************************************************************************
//!file     si_drv_ipv.h
//!brief    SiI9535 InstaPrevue driver functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_IPV_H__
#define __SI_DRV_IPV_H__
#include "si_common.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------
#define IPV_WINDOW_COUNT            3
#define PICTURE_ASPECT16_9          1
#define PICTURE_ASPECT4_3           0



typedef enum _SiiDrvIpvResultCodes_t
{
    SII_DRV_IPV_SUCCESS      = 0,            // Success.
    SII_DRV_IPV_ERR_FAIL,                    // General failure.
    SII_DRV_IPV_ERR_INVALID_PARAMETER,       //
    SII_DRV_IPV_ERR_IN_USE,                  // Module already initialized.
    SII_DRV_IPV_ERR_NOT_AVAIL,               // Allocation of resources failed.

} SiiDrvIpvResultCodes_t;

typedef enum _SiiIpvBorderColors_t
{
    SiiIpvBorderColorALL,
    SiiIpvBorderColorACTIVE,
    SiiIpvBorderColorNONAuth,
    SiiIpvBorderColorUNSUPPORTED,
    SiiIpvBorderColorUNSUPPORTEDBackground,

    SiiIpvBorderColorCount
} SiiIpvBorderColors_t;

typedef enum _SiiIpvMode_t
{
    SiiIpvModeALL           = 0x00,
    SiiIpvModeACTIVE        = 0x01,
    SiiIpvModeSELECT        = 0x03,
} SiiIpvMode_t;

//------------------------------------------------------------------------------
//  Data Structures
//------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
//  Driver Function Prototypes
//-------------------------------------------------------------------------------

bool_t SiiDrvIpvInitialize( void );
bool_t SiiDrvIpvStandby( void );
SiiDrvIpvResultCodes_t SiiDrvIpvGetLastResult( void );

void SiiDrvIpvModeSet( SiiIpvMode_t mode );
void SiiDrvIpvEnable( bool_t isEnable );
void SiiDrvIpvPIPSourceSet( uint8_t spPort );
void SiiDrvIpvDownsamplerReset();
void SiiDrvIpvEnableMaskSet( uint_t enableMask );
void SiiDrvIpvBorderEnablesSet( uint8_t borderEnableMask );
void SiiDrvIpvAnimationEnable(bool_t isEnabled);
bool_t  SiiDrvIpvAnimationIsComplete(void);
void SiiDrvIpvAnimationConfig(int_t xSpeed, int_t xAccel, int_t ySpeed, int_t yAccel);
void SiiDrvIpvWindowCoordSet( int_t winIndex, uint16_t x, uint16_t y );
void SiiDrvIpvWindowAlphaSet( int_t winIndex, uint8_t newAlpha );
void SiiDrvIpvBoundaryColorTableSet( SiiIpvBorderColors_t tableIndex, uint8_t *pCrCbY );
void SiiDrvIpvWindowAlphaTableSet( int_t firstWinIndex, uint8_t *pAlphaMax, uint8_t *pAlphaStep, int_t winCount );
void SiiDrvIpvWindowAlphaFade( bool_t isEnable );
void SiiDrvIpvSizeSet( uint16_t *p16x9, uint16_t *p4x3, uint8_t *pHeight );

bool_t SiiDrvIpvHandleInterrupts( void );

#endif  // __SI_DRV_IPV_H__



