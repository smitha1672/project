//***************************************************************************
//!file     si_i2c.c
//!brief    Board Component i2c driver, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_platform.h"
#include "si_timer.h"
#include "si_i2c.h"
#include "si_gpio.h"

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 ) 
#include "si_arm.h"

#endif 

//*****************************************************************************
//
// The I2C read transaction structure definition.
// ENABLE:  <S> [DEVID][W] [OFFSET] <P> <S> [DEVID][R] {[DATAn]} <P>
// DISABLE: <S> [DEVID][W] [OFFSET] <S> [DEVID][R] {[DATAn]} <P>
//
//*****************************************************************************

#define SI_I2C_WRITE_TO_READ_STOP DISABLE

//*****************************************************************************
//
// The I2C pins that are used by this application.
//
//*****************************************************************************

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
static unsigned long l_i2cPeripheralGpio[]  = { SYSCTL_PERIPH_GPIOB, SYSCTL_PERIPH_GPIOA };
static unsigned long l_i2cPeripheralI2c[]   = { SYSCTL_PERIPH_I2C0, SYSCTL_PERIPH_I2C1 };
static unsigned long l_i2cPortBase[]        = { GPIO_PORTB_BASE, GPIO_PORTA_BASE };
static unsigned long l_i2cPins[]            = { GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_6 | GPIO_PIN_7 };
static unsigned long l_i2cBase[]            = { I2C0_MASTER_BASE, I2C1_MASTER_BASE };
static unsigned long l_i2cInt[]             = { INT_I2C0, INT_I2C1 };
#define I2C_SLAVE_BASE_OFFSET    0x00000800
#endif 



//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cPollForAck
// Description:	Write the device address and check for ACK.  Repeat until an
//				ACK is received.  Useful for waiting an EEPROM write to complete
// Parameters:
// Returns:
//------------------------------------------------------------------------------

bool_t SiiPlatformI2cPollForAck ( int busIndex, uint8_t devId )
{
    bool_t success = true;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_TIMER )    
    clock_time_t start = SiiPlatformTimerSysTicksGet();

    // Temporary wait until I can figure out how to actually
    // check whether or not the target is ACKing the device ID byte

    while (( SiiPlatformTimerSysTicksGet() - start ) < 10 )
    {

    }
//    for ( ;; )
//    {
//        /* Write the register address byte.	*/
//
//        ROM_I2CMasterSlaveAddrSet( I2C0_MASTER_BASE, devId >> 1, false);
//        ROM_I2CMasterDataPut( I2C0_MASTER_BASE, 0 );
//        ROM_I2CMasterControl( I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND );
//        while ( !ROM_I2CMasterBusy( I2C0_MASTER_BASE ))
//        {
//        }
//        if ( ROM_I2CMasterErr(I2C0_MASTER_BASE) != I2C_MASTER_ERR_NONE)
//        {
//            ROM_I2CMasterIntClear(I2C0_MASTER_BASE);
//            break;
//        }
//
//        success = true;
//        break;
//    }
//
//    if ( !success )
//    {
//        ROM_I2CMasterControl( I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP );
//    }
#endif 
    return( success );
}

//------------------------------------------------------------------------------
// Function:    ExecuteI2cTransaction
// Description: Send a transaction command (SEND/RECEIVE/CONT/FINISH/STOP) to
//              the I2C hardware and wait for the transaction to complete.
// Parameters:  command - transaction command
// Returns:     true - success, false - failure.
//------------------------------------------------------------------------------

static bool_t ExecuteI2cTransaction (int busIndex, uint_t command )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint_t      i2cError;
    SiiTimer_t  timeouter;

    // Start the transaction and wait for controller to go
    // busy.  Mask interrupts so that we don't get interrupted between
    // sending the command and testing the controller for 'not-busy'

    ROM_IntMasterDisable();
    ROM_I2CMasterControl( l_i2cBase[busIndex], command  );
    while ( !ROM_I2CMasterBusy( l_i2cBase[busIndex] ))
    {
    }
    ROM_IntMasterEnable();

    // Total transaction needs to be shorter than 10ms.
    SiiOsTimerSet(&timeouter, 10);

    // Now wait for the controller to finish
    while ( ROM_I2CMasterBusy( l_i2cBase[busIndex] ) && !SiiOsTimerExpired(timeouter))
    {
    }

    // Return result of error check.

    i2cError = ROM_I2CMasterErr( l_i2cBase[busIndex] );
    return( i2cError == I2C_MASTER_ERR_NONE );
#else
	return true;
#endif 
}

//------------------------------------------------------------------------------
// Function:    I2cWriteBlock
// Description: Local 8/16 bit address function provides base function for all I2C writes
// Parameters:
// Returns:
//------------------------------------------------------------------------------

static bool_t I2cWriteBlock ( int busIndex, uint8_t devId, uint16_t regAddr, const uint8_t *pWrData, uint16_t nBytes )
{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint16_t writeCount;
    SiiPlatformStatus_t status = PLATFORM_I2C_WRITE_FAIL;

    platform.i2CError = false;
    for ( ;; )
    {
        ROM_I2CMasterSlaveAddrSet( l_i2cBase[busIndex], devId >> 1, false);

        // Write the register address byte(s).

        if ( platform.i2c16bitAddr )
        {
            ROM_I2CMasterDataPut( l_i2cBase[busIndex], (regAddr >> 8) );
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_START))
            {
                break;
            }
        }
        ROM_I2CMasterDataPut( l_i2cBase[busIndex], (uint8_t)regAddr );
        if ( !ExecuteI2cTransaction( busIndex, ( platform.i2c16bitAddr ) ? I2C_MASTER_CMD_BURST_SEND_CONT : I2C_MASTER_CMD_BURST_SEND_START ))
        {
            break;
        }


        if (nBytes == 0)
        {
            // Skip data and stop condition sending, if requested
            if (!platform.i2cNoInterimStops)
            {
                ROM_I2CMasterControl( l_i2cBase[busIndex], I2C_MASTER_CMD_BURST_SEND_FINISH );
            }

            status = PLATFORM_SUCCESS;
            break;
        }


        /* Write the data bytes. */

        for ( writeCount = 0; writeCount < (nBytes - 1); writeCount++ )
        {

             ROM_I2CMasterDataPut( l_i2cBase[busIndex], *pWrData ); // Write the data byte to the controller.
            pWrData++;
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_CONT ))
            {
                break;
            }
        }

        /* One byte left in transfer, send it with a FINISH.    */

        ROM_I2CMasterDataPut( l_i2cBase[busIndex], *pWrData ); // Write the data byte to the controller.
        if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_FINISH ))
        {
            break;
        }

        status = PLATFORM_SUCCESS;
        break;
    }

    if ( status != PLATFORM_SUCCESS )
    {
        ROM_I2CMasterControl( l_i2cBase[busIndex], I2C_MASTER_CMD_BURST_SEND_ERROR_STOP );
        platform.i2CError = true;
        platform.i2cErrorCode   = ROM_I2CMasterErr(l_i2cBase[busIndex]);
        ROM_I2CMasterIntClear(l_i2cBase[busIndex]);
    }

    platform.lastResultCode  = status;
    return( !platform.i2CError );
#else
	return false;
#endif 
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cWriteBlock
// Description:
// Parameters:
// Returns:
//------------------------------------------------------------------------------

bool_t SiiPlatformI2cWriteBlock16 ( int busIndex, uint8_t devId, uint16_t regAddr, const uint8_t *pWrData, uint16_t nBytes )
{
    platform.i2c16bitAddr = true;
    return( I2cWriteBlock( busIndex, devId, regAddr, pWrData, nBytes ));
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cWriteByte
// Description: Write a single byte to the I2C controller.
// Parameters:
// Returns:		None
// Note:		The SiiPlatformI2cWriteBlock() function updates lastResultCode and i2CError
//------------------------------------------------------------------------------

void SiiPlatformI2cWriteByte16 ( int busIndex, uint8_t devId, uint16_t regAddr, uint8_t wrData )
{
    platform.i2c16bitAddr = true;
    I2cWriteBlock( busIndex, devId, regAddr, &wrData, 1 );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cWriteBlock
// Description:
// Parameters:
// Returns:
//------------------------------------------------------------------------------

bool_t SiiPlatformI2cWriteBlock ( int busIndex, uint8_t devId, uint8_t regAddr, const uint8_t *pWrData, uint16_t nBytes )
{
    platform.i2c16bitAddr = false;
    return( I2cWriteBlock( busIndex, devId, regAddr, pWrData, nBytes ));
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cWriteByte
// Description: Write a single byte to the I2C controller.
// Parameters:
// Returns:     None
// Note:        The SiiPlatformI2cWriteBlock() function updates lastResultCode and i2CError
//------------------------------------------------------------------------------

void SiiPlatformI2cWriteByte ( int busIndex, uint8_t devId, uint8_t regAddr, uint8_t wrData )
{
    platform.i2c16bitAddr = false;
    I2cWriteBlock( busIndex, devId, regAddr, &wrData, 1 );
}

//------------------------------------------------------------------------------
// Function:    I2cReadBlock
// Description: Local 8/16 bit address function provides base function for all I2C reads
// Parameters:
// Returns:
//------------------------------------------------------------------------------

static bool_t I2cReadBlock ( int busIndex, uint8_t devId, uint16_t regAddr, uint8_t *pRdData, uint16_t nBytes )
{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint16_t            command, exitCommand;
	uint16_t            readCount;
    SiiPlatformStatus_t status = PLATFORM_I2C_READ_FAIL;

    platform.i2CError = false;
    for ( ;; )
    {
        exitCommand = I2C_MASTER_CMD_BURST_SEND_ERROR_STOP;

        ROM_I2CMasterSlaveAddrSet( l_i2cBase[busIndex], devId >> 1, false);

        /* Write the register address byte(s).	*/

        if ( platform.i2c16bitAddr )
        {
            ROM_I2CMasterDataPut( l_i2cBase[busIndex], (regAddr >> 8) );
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_START))
            {
                break;
            }
        }
        ROM_I2CMasterDataPut( l_i2cBase[busIndex], (uint8_t)regAddr );

#if (SI_I2C_WRITE_TO_READ_STOP == ENABLE)
        if ( !ExecuteI2cTransaction( busIndex, ( platform.i2c16bitAddr ) ? I2C_MASTER_CMD_BURST_SEND_FINISH : I2C_MASTER_CMD_SINGLE_SEND) )
#else
        if ( !ExecuteI2cTransaction( busIndex, ( platform.i2c16bitAddr ) ? I2C_MASTER_CMD_BURST_SEND_FINISH : I2C_MASTER_CMD_BURST_SEND_START) )
#endif
        {
            break;
        }

        /* Start the transfer.	*/

        ROM_I2CMasterSlaveAddrSet( l_i2cBase[busIndex], devId >> 1, true);

        /* If only one byte to read, handle it specially.	*/

        exitCommand = I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP;
        if ( nBytes == 1 )
        {
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_SINGLE_RECEIVE ))
            {
                break;
            }
			*pRdData = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );	// Read the data byte from the controller.
			status = PLATFORM_SUCCESS;
			break;
        }

        /* More than one byte to read, start with START and loop here with CONT.	*/

        command = I2C_MASTER_CMD_BURST_RECEIVE_START;
        for ( readCount = 0; readCount < ( nBytes - 1 ); readCount++ )
        {
            if ( !ExecuteI2cTransaction( busIndex, command ))
            {
                break;
            }
            command = I2C_MASTER_CMD_BURST_RECEIVE_CONT;    // Use CONT for remaining bytes - 1

			// Read the data byte from the controller.

			*pRdData = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );
			pRdData++;
        }

        /* One byte left in transfer, get it with a FINISH. */

        if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_RECEIVE_FINISH ))
        {
            break;
        }
		*pRdData = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );

        status = PLATFORM_SUCCESS;
        break;
    }

    if ( status != PLATFORM_SUCCESS )
    {
        platform.i2CError       = true;
        platform.i2cErrorCode   = ROM_I2CMasterErr(l_i2cBase[busIndex]);
        ROM_IntMasterEnable();
    }

    platform.lastResultCode  = status;
    return( !platform.i2CError );
#else
	return ( false );
#endif 
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cReadBlock
// Description:
// Parameters:
// Returns:
//------------------------------------------------------------------------------

bool_t SiiPlatformI2cReadBlock16 ( int busIndex, uint8_t devId, uint16_t regAddr, uint8_t *pRdData, uint16_t nBytes )
{

    platform.i2c16bitAddr = true;
    return( I2cReadBlock( busIndex, devId, regAddr, pRdData, nBytes ));
}

//------------------------------------------------------------------------------
// Function:    SkI2cRead
// Description: Read a single byte from the I2C controller
// Parameters:
// Returns:
// Note:		The SkI2cWriteRead() function updates lastResultCode and i2CError
//------------------------------------------------------------------------------

uint8_t SiiPlatformI2cReadByte16 ( int busIndex, uint8_t devId, uint16_t regAddr )
{
	uint8_t returnData = 0;

    platform.i2c16bitAddr = true;
    I2cReadBlock( busIndex, devId, regAddr, &returnData, 1 );
    return( returnData );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cReadBlock
// Description:
// Parameters:
// Returns:
//------------------------------------------------------------------------------

bool_t SiiPlatformI2cReadBlock ( int busIndex, uint8_t devId, uint8_t regAddr, uint8_t *pRdData, uint16_t nBytes )
{

    platform.i2c16bitAddr = false;
    return( I2cReadBlock( busIndex, devId, regAddr, pRdData, nBytes ));
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cReadEdidBlock
// Description: Read an EDID block over DDC link
// Parameters:
// Returns:
//------------------------------------------------------------------------------

bool_t SiiPlatformI2cReadEdidBlock(int busIndex, uint8_t segmentDevId, uint8_t offsetDevId, uint8_t segment, uint8_t offset, uint8_t *pRdData, uint16_t nBytes)
{
    bool_t isSuccess = false;

    platform.i2c16bitAddr = false;
    platform.i2cNoInterimStops = true;

    // write EDID segment index
    isSuccess = I2cWriteBlock(busIndex, segmentDevId, segment, 0, 0);
    // write EDID offset and read nBytes of data
    isSuccess = isSuccess && I2cReadBlock(busIndex, offsetDevId, offset, pRdData, nBytes);

    platform.i2cNoInterimStops = false;

    return( isSuccess );
}


//------------------------------------------------------------------------------
// Function:    SkI2cRead
// Description: Read a single byte from the I2C controller
// Parameters:
// Returns:
// Note:        The SkI2cWriteRead() function updates lastResultCode and i2CError
//------------------------------------------------------------------------------

uint8_t SiiPlatformI2cReadByte ( int busIndex, uint8_t devId, uint8_t regAddr )
{
    uint8_t returnData = 0;

    platform.i2c16bitAddr = false;
    I2cReadBlock( busIndex, devId, regAddr, &returnData, 1 );
    return( returnData );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SiiPlatformStatus_t SiiPlatformI2cSendStart ( int busIndex, uint8_t deviceId, const uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop)
{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint16_t            i, command, writeCount;
    SiiPlatformStatus_t status = PLATFORM_I2C_WRITE_FAIL;

    platform.i2CError = false;
    for ( ;; )
    {
        if (nBytes == 0)
        {
            status = PLATFORM_INVALID_PARAMETER;
            break;
        }
        ROM_I2CMasterSlaveAddrSet( l_i2cBase[busIndex], deviceId >> 1, false);

        // Send the first byte.  If more than one byte, start a burst.
        // If only one byte, use send single mode unless no stop is requested
        command = I2C_MASTER_CMD_BURST_SEND_START;
        if ( sendStop && ( nBytes == 1 ))
        {
            command = I2C_MASTER_CMD_SINGLE_SEND;
        }
        ROM_I2CMasterDataPut( l_i2cBase[busIndex], *pBuffer++ );
        if ( !ExecuteI2cTransaction( busIndex, command ))
        {
            break;
        }
        // Sending one byte == we're done
        if ( nBytes == 1 )
        {
            status = PLATFORM_SUCCESS;
            break;
        }

        // More than one byte to write, continue the write using CONT.
        // If sending STOP, count is nBytes - 1, otherwise send all bytes here
        writeCount = (sendStop) ? (nBytes - 2) : (nBytes - 1);
        for ( i = 0; i < writeCount; i++ )
        {
            ROM_I2CMasterDataPut( l_i2cBase[busIndex], *pBuffer ); // Write the data byte to the controller.
            pBuffer++;
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_CONT ))
            {
                break;
            }
        }

        // If sending stop, there is one byte left in transfer, send it with a FINISH
        if ( sendStop )
        {
            ROM_I2CMasterDataPut( l_i2cBase[busIndex], *pBuffer ); // Write the data byte to the controller.
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_FINISH ))
            {
                break;
            }
        }

        status = PLATFORM_SUCCESS;
        break;
    }

    if ( status != PLATFORM_SUCCESS )
    {
        ROM_I2CMasterControl( l_i2cBase[busIndex], I2C_MASTER_CMD_BURST_SEND_ERROR_STOP );
        platform.i2CError       = true;
        platform.i2cErrorCode   = ROM_I2CMasterErr(l_i2cBase[busIndex]);
        ROM_I2CMasterIntClear(l_i2cBase[busIndex]);
    }

    platform.lastResultCode = status;
    return( status );
#else
	return (PLATFORM_FAIL);
#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

SiiPlatformStatus_t SiiPlatformI2cSendContinue ( int busIndex, const uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint16_t            i, writeCount;
    SiiPlatformStatus_t status = PLATFORM_I2C_WRITE_FAIL;

    platform.i2CError = false;
    for ( ;; )
    {
        if (nBytes == 0)
        {
            status = PLATFORM_INVALID_PARAMETER;
            break;
        }

        // Send the first byte. Since this is a continue function, we assume
        // that the last call already wrote the device address but did NOT send the
        // stop bit, so we are still good for that.  Then, if more than one data byte
        // to write OR they did NOT ask for stop, send data without a stop at
        // the end.
        if ( !sendStop || (nBytes > 1))
        {
            // More than one byte to write, continue the write using CONT.
            // If sending STOP, count is nBytes - 1, otherwise send all bytes here
            writeCount = (sendStop) ? (nBytes - 1) : nBytes;
            for ( i = 0; i < writeCount; i++ )
            {
                ROM_I2CMasterDataPut( l_i2cBase[busIndex], *pBuffer ); // Write the data byte to the controller.
                pBuffer++;
                if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_CONT ))
                {
                    break;
                }
            }
        }

        // If sending stop, there is one byte left in transfer, send it with a FINISH
        if ( sendStop )
        {
            ROM_I2CMasterDataPut( l_i2cBase[busIndex], *pBuffer ); // Write the data byte to the controller.
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_SEND_FINISH ))
            {
                break;
            }
        }

        status = PLATFORM_SUCCESS;
        break;
    }

    if ( status != PLATFORM_SUCCESS )
    {
        ROM_I2CMasterControl( l_i2cBase[busIndex], I2C_MASTER_CMD_BURST_SEND_ERROR_STOP );
        platform.i2CError       = true;
        platform.i2cErrorCode   = ROM_I2CMasterErr(l_i2cBase[busIndex]);
        ROM_I2CMasterIntClear(l_i2cBase[busIndex]);
    }

    platform.lastResultCode = status;
    return( status );
#else
	return ( PLATFORM_SUCCESS );
#endif 
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

SiiPlatformStatus_t SiiPlatformI2cReceiveStart ( int busIndex, uint8_t deviceId, uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop )
{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint16_t            command, exitCommand;
    uint16_t            i, readCount;
    SiiPlatformStatus_t status = PLATFORM_I2C_READ_FAIL;

    platform.i2CError = false;
    for ( ;; )
    {
        exitCommand = I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP;

        // Set up the device ID.
        ROM_I2CMasterSlaveAddrSet( l_i2cBase[busIndex], deviceId >> 1, true);

        // If only one byte to read, and we're asked to send STOP when finished, handle it specially.
        if ( sendStop && ( nBytes == 1 ))
        {
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_SINGLE_RECEIVE ))
            {
                break;
            }
            *pBuffer = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );    // Read the data byte from the controller.
            status = PLATFORM_SUCCESS;
            break;
        }

        // One byte with nostop or more than one byte to read, start with START
        // and loop here with CONT.
        // If sending STOP, count is nBytes - 1, otherwise send all bytes here
        readCount = (sendStop) ? (nBytes - 1) : nBytes;
        command = I2C_MASTER_CMD_BURST_RECEIVE_START;
        for ( i = 0; i < readCount; i++ )
        {
            if ( !ExecuteI2cTransaction( busIndex, command ))
            {
                break;
            }
            command = I2C_MASTER_CMD_BURST_RECEIVE_CONT;    // Use CONT for remaining bytes - 1

            // Read the data byte from the controller.

            *pBuffer = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );
            pBuffer++;
        }

        // If sending stop, there is one byte left in transfer, get it with a FINISH
        if ( sendStop )
        {
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_RECEIVE_FINISH ))
            {
                break;
            }
            *pBuffer = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );
        }

        status = PLATFORM_SUCCESS;
        break;
    }

    if ( status != PLATFORM_SUCCESS )
    {
        platform.i2CError       = true;
        platform.i2cErrorCode   = ROM_I2CMasterErr(l_i2cBase[busIndex]);
        ROM_I2CMasterIntClear(l_i2cBase[busIndex]);
    }

    platform.lastResultCode = status;
    return( status );
#else
	return PLATFORM_SUCCESS;
#endif 
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

SiiPlatformStatus_t SiiPlatformI2cReceiveContinue ( int busIndex, uint8_t *pBuffer, uint16_t nBytes, bool_t sendStop )
{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint16_t            exitCommand;
    uint16_t            i, readCount;
    SiiPlatformStatus_t status = PLATFORM_I2C_READ_FAIL;

    platform.i2CError = false;
    for ( ;; )
    {
        exitCommand = I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP;

        // We were already started (or should have been) by a previous call to
        // SiiPlatformI2cReceiveStart, so we just continue on here.  If the caller asked for a
        // stop at the end, this loop should handle all but the last byte
        readCount = (sendStop) ? (nBytes - 1) : nBytes;
        for ( i = 0; i < readCount; i++ )
        {
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_RECEIVE_CONT ))
            {
                break;
            }

            // Read the data byte from the controller.
            *pBuffer = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );
            pBuffer++;
        }

        // If sending stop, there is one byte left in transfer, get it with a FINISH
        if ( sendStop )
        {
            if ( !ExecuteI2cTransaction( busIndex, I2C_MASTER_CMD_BURST_RECEIVE_FINISH ))
            {
                break;
            }
            *pBuffer = ROM_I2CMasterDataGet( l_i2cBase[busIndex] );
        }

        status = PLATFORM_SUCCESS;
        break;
    }

    if ( status != PLATFORM_SUCCESS )
    {
        platform.i2CError       = true;
        platform.i2cErrorCode   = ROM_I2CMasterErr(l_i2cBase[busIndex]);
        ROM_I2CMasterIntClear(l_i2cBase[busIndex]);
    }

    platform.lastResultCode  = status;
    return( status );
#else
	return ( PLATFORM_SUCCESS );
#endif 
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends one or more msgNum messages (I2C sub-transactions).
//! @return
//-------------------------------------------------------------------------------------------------
SiiPlatformStatus_t SiiPlatformI2cTransfer ( int busIndex, SiiI2cMsg_t *pMsgs, uint8_t msgNum)
{
    SiiPlatformStatus_t status;
    int                 msgIndex;

    for ( msgIndex = 0; msgIndex < msgNum; msgIndex++ )
            {
        SiiI2cMsg_t *pMsg = &pMsgs[msgIndex];

        if ( pMsgs[msgIndex].cmdFlags & SII_MI2C_RD )
        {
            // Every message gets a start bit.
            status = SiiPlatformI2cReceiveStart( busIndex, pMsg->addr, pMsg->pBuf, pMsg->len, (msgIndex == (msgNum - 1)) ? true : false );
        }
        else
        {
            // Every message gets a start bit.
            status = SiiPlatformI2cSendStart( busIndex, pMsg->addr, pMsg->pBuf, pMsg->len, (msgIndex == (msgNum - 1)) ? true : false );
        }
        if ( status != PLATFORM_SUCCESS )
            {
                break;
            }
        }

    return( status );
    }


//------------------------------------------------------------------------------
// Function:    SiiPlatformI2cInit
// Description: Initialization of I2C hardware
// Parameters:  index        - 0 for I2C0, 1 for I2C1
//              isMasterMode - true for I2C master mode, false for I2C slave mode
//              slaveAddress - I2C slave address (applicable to slave mode only)
//              clockSpeedKHz - I2C clock speed (applicable to master mode only)
//------------------------------------------------------------------------------
void SiiPlatformI2cInit (uint8_t index, bool_t isMasterMode, uint8_t slaveAddress, uint_t clockSpeedKhz )
{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint_t ulSCLFreq, ulTPR;

    // Enable the ARM peripherals needed.
    ROM_SysCtlPeripheralEnable( l_i2cPeripheralGpio[ index] );
    ROM_SysCtlPeripheralEnable( l_i2cPeripheralI2c[ index] );
    ROM_SysCtlPeripheralReset( l_i2cPeripheralI2c[ index] );

    // Put the pins into HW control mode for the I2C module.
    ROM_GPIODirModeSet( l_i2cPortBase[ index], l_i2cPins[index], GPIO_DIR_MODE_HW);
    ROM_GPIOPinTypeI2C( l_i2cPortBase[ index], l_i2cPins[index] );

    if (isMasterMode)
    {
        ROM_I2CMasterEnable( l_i2cBase[ index] );

        // Clock speed needs to be set only on the master
        ulSCLFreq = clockSpeedKhz * 1000;
        ulTPR = (ROM_SysCtlClockGet() / (2 * 10 * ulSCLFreq)) - 1;
        HWREG( l_i2cBase[ index] + I2C_O_MTPR) = ulTPR;

    }
    else // Slave mode
    {
        unsigned long slaveBase = l_i2cBase[ index] + I2C_SLAVE_BASE_OFFSET;

        ROM_I2CSlaveEnable( slaveBase );
        ROM_I2CSlaveInit( slaveBase, slaveAddress );

        // Using interrupts only on the slave
        ROM_IntEnable( l_i2cInt[ index] );
        ROM_I2CSlaveIntClear( slaveBase );
        ROM_I2CSlaveIntEnable( slaveBase );
    }

    //I2C1 workaround, API could not set this register; so we set directly
    if ( index == 0 )
    {
        //GPIO_PORTB_PCTL_R = 0x11111111;
    }
    else
    {
        GPIO_PORTA_PCTL_R = 0x11111111;
    }

#endif 

}


//------------------------------------------------------------------------------
// Function:    SiiPlatformI2C0IntHandler
// Description: I2C interrupt handler
//------------------------------------------------------------------------------
void SiiPlatformI2C0IntHandler(void)
{
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformI2C0IntHandler
// Description: I2C interrupt handler
//------------------------------------------------------------------------------
void SiiPlatformI2C1IntHandler(void)
{

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_I2C )
    uint_t  ulInt;
    int_t status;
#if INC_RTPI
    int_t data = 0;
#endif
    static uint8_t addr = 0;

    ulInt = ROM_IntMasterDisable(); // Temporarily turn off interrupts.

    ROM_I2CSlaveIntClear(I2C1_SLAVE_BASE);

    //DEBUG_PRINT(MSG_ALWAYS, "I2C1 IRQ hit \n");

    // Test
    //SiiPlatformGpioLedPorts ( 0xFF );


    status = ROM_I2CSlaveStatus(I2C1_SLAVE_BASE);
    if (status != I2C_SLAVE_ACT_NONE)
    {
        //DEBUG_PRINT(MSG_ALWAYS,"I2C1 STATUS: %d\n", status);
        switch (status)
              {
                  case I2C_SLAVE_ACT_RREQ:
#if INC_RTPI
                      // Read request
                      data = ROM_I2CSlaveDataGet(I2C1_SLAVE_BASE);
                      SiiPlatformI2cCbSlaveWriteByte(1, addr, data );
                      addr++;
#endif
                      //DEBUG_PRINT(MSG_ALWAYS,"RRQ: %x\n", data);
                      break;
                  case I2C_SLAVE_ACT_TREQ:
#if INC_RTPI
                      // Write Request
                      data = (uint_t)SiiPlatformI2cCbSlaveReadByte(1, addr);
                      addr++;
                      ROM_I2CSlaveDataPut(I2C1_SLAVE_BASE, data );
                      //DEBUG_PRINT(MSG_ALWAYS,"TRQ\n");
#endif
                      break;
                  case I2C_SLAVE_ACT_RREQ_FBR:
                      // Read request
                      addr = ROM_I2CSlaveDataGet(I2C1_SLAVE_BASE);

                      //DEBUG_PRINT(MSG_ALWAYS,"FBR: %x\n", data);
                      break;
              }
}

    if ( !ulInt )   // Re-enable interrupts if needed
    {
        ROM_IntMasterEnable();
    }

#endif     
}
