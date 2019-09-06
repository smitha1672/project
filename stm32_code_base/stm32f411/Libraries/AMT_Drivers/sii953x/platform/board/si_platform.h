//***************************************************************************
//!file     si_platform.h
//!brief    Silicon Image SiI9535 Starter Kit board Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_PLATFORM_H__
#define __SI_PLATFORM_H__

#include "si_common.h"
#if ( configSII_DEV_953x_PORTING == 0 )
#include "si_logger.h"
#endif

typedef enum _SkSwitchNumbers_t
{
    SW_1,
    SW_2,
    SW_3,
    SW_4,
    SW_5,
    SW_6,
    SW_7,
} SkSwitchNumbers_t;

//------------------------------------------------------------------------------
//  Board function result status codes
//------------------------------------------------------------------------------

#define MSG_PLATFORM       (MSG_DBG | DBGF_TS | DBGF_CN), DBG_BRD

typedef enum _SiiPlatformStatus_t
{
    PLATFORM_SUCCESS,
    PLATFORM_FAIL,              // General fail
    PLATFORM_INVALID_PARAMETER,
    PLATFORM_I2C_READ_FAIL,
    PLATFORM_I2C_WRITE_FAIL,
} SiiPlatformStatus_t;

//-----------------------------------------------------------------------------
//  I2C Instance defines
//-----------------------------------------------------------------------------
#define I2C_INDEX0		0
#define I2C_INDEX1		1
#define I2C_MASTER		1
#define I2C_SLAVE		0

//------------------------------------------------------------------------------
//  Platform Instance Data
//------------------------------------------------------------------------------

typedef struct BoardInstanceData
{
    int                 structVersion;
    SiiPlatformStatus_t lastResultCode; // Contains the result of the last API function called
    uint16_t            statusFlags;


    bool_t      i2CError;				// I2C error pass/fail
    uint32_t    i2cErrorCode;           // Sticky flags for I2C I/O failure
    uint8_t     dbgMsgLevel;

    bool_t      i2c16bitAddr;           // 16 bit address flag for I2C transactions
    bool_t      i2cNoInterimStops;      // True to avoid stop condition followed by start condition immediately

    bool_t      hardwareInt;            // Set by interrupt handler from H/W INT pin
}	BoardInstanceData_t;

extern BoardInstanceData_t platform;

//------------------------------------------------------------------------------
//  UI functions
//------------------------------------------------------------------------------

bool_t  SiiPlatformInitialize( void );
uint32_t SiiPlatformLastI2cStatus( void );
int_t   SiiPlatformGetLastResult( void );

bool_t  SiiPlatformPowerSwitchGet( void );
uint8_t SiiPlatformReadRotary( uint8_t iWantItNow );
bool_t  SiiPlatformSwitchGet( SkSwitchNumbers_t switchNum );
bool_t  SiiPlatformSiimonBusRequest ( void );
void    SiiPlatformSiimonBusGrant ( bool_t grant );

void    SiiPlatformAllInterruptsEnable ( void );
void    SiiPlatformAllInterruptsDisable ( void );

void    SiiPlatformInterruptEnable( void );
void    SiiPlatformInterruptDisable( void );
void    SiiPlatformInterruptHandler( void );
bool_t  SiiPlatformInterruptPinStateGet ( void );
bool_t  SiiPlatformInterruptStateGet( void );
void    SiiPlatformInterruptClear ( void );


#endif // __SI_PLATFORM_COMPONENT_H__
