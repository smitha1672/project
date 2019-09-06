//***************************************************************************
//!file     si_drv_internal.h
//!brief    Sii9535 driver internal header.
//!brief    Driver layer internal file
//
//          The Sii9535 driver contains all functions data, and constants
//          common to all modules at the next higher, component driver level.
//
//          This file is to be included in each driver-level module.  The
//          information contained in the file is typically used only at the
//          component driver level.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_IDRV_SII9535_H__
#define __SI_IDRV_SII9535_H__
#include "si_common.h"
#include "si_cra.h"
#include "si_device_config.h"
#include "si_drv_nvram_sram.h"
#include "si_drv_msw_internal.h"

#include "si_timer.h"

//------------------------------------------------------------------------------
//  Device Component Instance Data
//------------------------------------------------------------------------------

typedef struct
{
    int         structVersion;
    int         lastResultCode;         // Contains the result of the last API function called
    uint16_t    statusFlags;

    uint16_t    devType;
    uint8_t     devRev;

    bool_t      hardwareInt;

}   DeviceDrvInstanceData_t;

extern DeviceDrvInstanceData_t deviceDrvInstance[1];
extern DeviceDrvInstanceData_t *pDrvDevice;

//------------------------------------------------------------------------------
//  Indirect function callback type definitions
//------------------------------------------------------------------------------

typedef bool_t (*BoolTwoUInts_t)(uint_t, uint_t);

//------------------------------------------------------------------------------
//  Device Component SiI9535-specific
//------------------------------------------------------------------------------

/* SiiDEV_BOOT_STATE_MACHINE bit definitions.   */

#define BSM_BOOT_DONE   0x04
#define BSM_BOOT_ERROR  0x03

//------------------------------------------------------------------------------
//  Device Driver external peripheral control functions
//------------------------------------------------------------------------------

uint8_t SiiRegRead( uint16_t regAddr );
void    SiiRegWrite( uint16_t regAddr, uint8_t wrData );
void    SiiRegModify( uint16_t regAddr, uint8_t mask, uint8_t value );
void    SiiRegReadBlock( uint16_t regAddr, uint8_t *pData, uint16_t count );
void    SiiRegWriteBlock( uint16_t regAddr, const uint8_t *pData, uint16_t count );
void    SiiRegSetNewBits( uint16_t regAddr, uint8_t bits, bool_t setBits );
bool_t  SiiDeviceEdidReadBlock( uint16_t segmentAddr, uint16_t offsetAddr, uint8_t *pBuffer, uint16_t count );

uint_t  SiiDeviceAccessStatus( void );
void    SiiRegSetBase( uint16_t regAddr, uint8_t newId );


#endif // __ISI_DRV_SII9535_H__
