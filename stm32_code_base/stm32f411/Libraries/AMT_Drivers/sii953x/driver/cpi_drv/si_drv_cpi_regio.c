//***************************************************************************
//!file     si_drv_cpi_regio.c
//!brief    Silicon Image CPI Driver register access.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_drv_internal.h"
#include "si_i2c.h"

//------------------------------------------------------------------------------
// Function:    SiiDrvCpiRegisterRead
// Description: Read a one byte register from a CEC page.
//              The register address parameter is translated into an I2C slave
//              address and offset. The I2C slave address and offset are used
//              to perform an I2C read operation.
//------------------------------------------------------------------------------

uint8_t SiiDrvCpiRegisterRead ( uint8_t devicePage, uint16_t regAddr )
{

    return( SiiPlatformI2cReadByte( devicePage, (uint8_t)regAddr ));
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCpiRegisterWrite
// Description: Performs a register write on the SiI9535 device.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              wrData  - Eight bit data to be written
// Returns:     None
//------------------------------------------------------------------------------

void SiiDrvCpiRegisterWrite ( uint8_t devicePage, uint16_t regAddr, uint8_t value )
{

    SiiPlatformI2cWriteByte( devicePage, (uint8_t)regAddr, value );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCpiRegisterReadBlock
// Description: Reads a block of data from sequential registers in a CEC page.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              mask    - Eight bit mask
//              value   - Eight bit data to be written, combined with mask.
// Returns:     None
//
// NOTE:        This function relies on the auto-increment model used by
//              Silicon Image devices.  Because of this, if a FIFO register
//              is encountered before the end of the requested count, the
//              data remaining from the count is read from the FIFO, NOT
//              from subsequent registers.
//------------------------------------------------------------------------------

void SiiDrvCpiRegisterReadBlock ( uint8_t devicePage, uint16_t regAddr, uint8_t* buffer, uint_t count )
{

    SiiPlatformI2cReadBlock( devicePage, (uint8_t)regAddr, buffer, count);
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCpiRegisterWriteBlock
// Description: Writes a block of data to sequential registers in a CEC page.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              mask    - Eight bit mask
//              value   - Eight bit data to be written, combined with mask.
// Returns:     None
//
// NOTE:        This function relies on the auto-increment model used by
//              Silicon Image devices.  Because of this, if a FIFO register
//              is encountered before the end of the requested count, the
//              data remaining from the count is written to the FIFO, NOT
//              to subsequent registers.
//------------------------------------------------------------------------------

void SiiDrvCpiRegisterWriteBlock ( uint8_t devicePage, uint16_t regAddr, uint8_t *pBuffer, uint_t count )
{

    SiiPlatformI2cWriteBlock( devicePage, (uint8_t)regAddr, pBuffer, count );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCpiRegisterModify
// Description: Reads the register, performs an AND function on the data using
//              the mask parameter, and an OR function on the data using the
//              value ANDed with the mask. The result is then written to the
//              device register specified in the regAddr parameter.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              mask    - Eight bit mask
//              value   - Eight bit data to be written, combined with mask.
// Returns:     None
//------------------------------------------------------------------------------

void SiiDrvCpiRegisterModify ( uint8_t devicePage, uint16_t regAddr, uint8_t mask, uint8_t value)
{
    uint8_t abyte;

    abyte = SiiPlatformI2cReadByte( devicePage, (uint8_t)regAddr );
    abyte &= (~mask);                                   // first clear all bits in mask
    abyte |= (mask & value);                            // then set bits from value
    SiiPlatformI2cWriteByte( devicePage, (uint8_t)regAddr, abyte);
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCpiRegisterBitsSet
// Description: Reads the register, sets the passed bits, and writes the
//              result back to the register.  All other bits are left untouched
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              bits   - bit data to be written
// Returns:     None
//------------------------------------------------------------------------------

void SiiDrvCpiRegisterBitsSet ( uint8_t devicePage, uint16_t regAddr, uint8_t bits )
{
    uint8_t abyte;

    abyte = SiiPlatformI2cReadByte( devicePage, (uint8_t)regAddr );
    SiiPlatformI2cWriteByte( devicePage, (uint8_t)regAddr, abyte | bits );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvCpiRegisterBitsClear
// Description: Reads the register, clears the passed bits, and writes the
//              result back to the register.  All other bits are left untouched
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              bits   - bit data to be written
// Returns:     None
//------------------------------------------------------------------------------

void SiiDrvCpiRegisterBitsClear ( uint8_t devicePage, uint16_t regAddr, uint8_t bits )
{
    uint8_t abyte;

    abyte = SiiPlatformI2cReadByte( devicePage, (uint8_t)regAddr );
    SiiPlatformI2cWriteByte( devicePage, (uint8_t)regAddr, abyte & ~bits );
}
