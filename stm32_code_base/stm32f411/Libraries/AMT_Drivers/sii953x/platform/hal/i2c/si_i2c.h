//***************************************************************************
//!file     si_i2c.h
//!brief    Platform i2c driver, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_I2C_H__
#define __SI_I2C_H__
#include "si_common.h"
#include "si_platform.h"


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

typedef struct
{
    uint16_t addr;      // slave address
    uint16_t cmdFlags;  // flags defining message actions
    uint16_t len;       // buffer length
    uint8_t *pBuf;      // pointer to input (for write operations) or
                        // pointer to output (for read operations) buffer
} SiiI2cMsg_t;

// cmdFlags values
#define SII_MI2C_TEN        0x0010  // set for ten bit chip address; cleared otherwise
#define SII_MI2C_RD         0x0001  // set for read data op; cleared for write op
#define SII_MI2C_WR         0x0000  // (the absence of read == write)

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

void SiiPlatformI2C0IntHandler(void);
void SiiPlatformI2C1IntHandler(void);

SiiPlatformStatus_t SiiPlatformI2cTransfer( int busIndex, SiiI2cMsg_t *pMsgs, uint8_t msgNum);


void    SiiPlatformI2cInit( uint8_t index, bool_t isMasterMode, uint8_t slaveAddress, uint_t clockSpeedKhz );

bool_t  SiiPlatformI2cWriteBlock( int busIndex, uint8_t devId, uint8_t regAddr, const uint8_t *pWrData, uint16_t nBytes );
void    SiiPlatformI2cWriteByte( int busIndex, uint8_t devId, uint8_t regAddr, uint8_t wrData );
bool_t  SiiPlatformI2cReadBlock( int busIndex, uint8_t devId, uint8_t regAddr, uint8_t *pRdData, uint16_t nBytes );
uint8_t SiiPlatformI2cReadByte( int busIndex, uint8_t devId, uint8_t);

bool_t  SiiPlatformI2cWriteBlock16( int busIndex, uint8_t devId, uint16_t regAddr, const uint8_t *pWrData, uint16_t nBytes );
void    SiiPlatformI2cWriteByte16( int busIndex, uint8_t devId, uint16_t regAddr, uint8_t wrData );
bool_t  SiiPlatformI2cReadBlock16( int busIndex, uint8_t devId, uint16_t regAddr, uint8_t *pRdData, uint16_t nBytes );
uint8_t SiiPlatformI2cReadByte16( int busIndex, uint8_t devId, uint16_t regAddr );

bool_t  SiiPlatformI2cPollForAck ( int busIndex, uint8_t devId );

bool_t  SiiPlatformI2cReadEdidBlock(int busIndex, uint8_t segmentDevId, uint8_t offsetDevId, uint8_t segment, uint8_t offset, uint8_t *pRdData, uint16_t nBytes);

// Callback functions for I2C Slave port read/write
uint8_t SiiPlatformI2cCbSlaveReadByte( int busIndex, uint8_t regAddr );
void    SiiPlatformI2cCbSlaveWriteByte( int busIndex, uint8_t regAddr, uint8_t regData );

// Low level I2C methods
SiiPlatformStatus_t SiiPlatformI2cSendStart( int busIndex, uint8_t deviceId, const uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop);
SiiPlatformStatus_t SiiPlatformI2cSendContinue( int busIndex, const uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop );
SiiPlatformStatus_t SiiPlatformI2cReceiveStart( int busIndex, uint8_t deviceId, uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop );
SiiPlatformStatus_t SiiPlatformI2cReceiveContinue( int busIndex, uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop );

// I2C h/w interrupt handlers
void    SiiPlatformI2C0IntHandler(void);
void    SiiPlatformI2C1IntHandler(void);


#endif // __SI_I2C_H__
