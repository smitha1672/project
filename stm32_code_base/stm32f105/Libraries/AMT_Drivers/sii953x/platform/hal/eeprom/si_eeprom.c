//***************************************************************************
//!file     si_eeprom.c
//!brief    Silicon Image Starter Kit EEPROM interface
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_platform.h"
#include "si_eeprom.h"
#include "si_i2c.h"

//------------------------------------------------------------------------------
// Function:    SiiPlatformEepromWriteBlock
// Description:	Write a block of data to the EEPROM
// Parameters:  addr
// Returns:     true if successful, false if an error occurred
//
// Note:		The Microchip 24LC128 uses an internal 64-byte
//              page buffer, and block writes must be aligned.
// Note:		A write is not complete until the EE_PROM begins ACKing
//				again.
// Note:		PollForAck cannot be done on Stellaris I2C controller
// Note:		The SiiPlatformI2cWriteBlock() function updates lastResultCode and i2CError
//------------------------------------------------------------------------------

bool_t SiiPlatformEepromWriteBlock ( uint16_t addr, const uint8_t *pWrData, uint16_t nBytes )
{
    bool_t        success = true;
    
#if defined ( SII_DEV_953x_PORTING_PLATFORM_EEPROM )
    uint16_t    i, packetLen;

    if ( addr & (64-1))
    {
        packetLen = addr & (64-1);
        if ( packetLen > nBytes )
            packetLen = nBytes;

        // Write individual bytes until we get to a 64 byte boundary
        // or run out of bytes.

        for ( i = 0; (( i < packetLen ) && success ); i++ )
        {
            success = SiiPlatformI2cWriteBlock16( 0, EEPROM_I2C_ADDRESS, addr, pWrData, 1 );
            if ( success )
            {
                success = SiiPlatformI2cPollForAck ( 0, EEPROM_I2C_ADDRESS );
            }
            addr++;
            pWrData++;
        }
        nBytes -= packetLen;
    }

    // Write the remaining aligned packets.

    for ( i = 0; (( i < nBytes ) && success ); i++ )
    {
        packetLen = 64;
        if ( packetLen > nBytes )
            packetLen = nBytes;
        success = SiiPlatformI2cWriteBlock16( 0, EEPROM_I2C_ADDRESS, addr, pWrData, packetLen );
        if ( success )
        {
            success = SiiPlatformI2cPollForAck ( 0, EEPROM_I2C_ADDRESS );
        }
        addr    += packetLen;
        pWrData += packetLen;
        nBytes  -= packetLen;
    }
#endif 

    return( success );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformEepromWriteByte
// Description:	Write a single byte of data to the on-board EEPROM
// Parameters:  addr
//				value
// Returns:
// Note:		The SiiPlatformEepromWriteBlock() function updates lastResultCode and i2CError
//------------------------------------------------------------------------------

void SiiPlatformEepromWriteByte ( uint16_t addr, uint8_t value )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_EEPROM )
	SiiPlatformEepromWriteBlock( addr, &value, 1 );
#endif 	
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformEepromReadBlock
// Description:	Read a block of data from the on-board EEPROM
// Parameters:  addr
//				pRdData
//				nBytes
// Returns:
//------------------------------------------------------------------------------

bool_t SiiPlatformEepromReadBlock ( uint16_t addr, uint8_t *pRdData, uint16_t nBytes )
{
    return( SiiPlatformI2cReadBlock16( 0, EEPROM_I2C_ADDRESS, addr, pRdData, nBytes ));
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformEepromReadByte
// Description:	Read a byte of data from the on-board EEPROM
// Parameters:  addr
// Returns:     Data byte read from EEPROM
//------------------------------------------------------------------------------

uint8_t SiiPlatformEepromReadByte ( uint16_t addr )
{
    uint8_t value;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_EEPROM )
    SiiPlatformI2cReadBlock16( 0, EEPROM_I2C_ADDRESS, addr, &value, 1 );
#else
	value = 0xff;
#endif 

    return( value );
}
