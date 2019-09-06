//***************************************************************************
//!file     si_cra.c
//!brief    Silicon Image Device register I/O support.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_common.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#include "I2C1LowLevel.h"
#endif

#if ( configSII_DEV_953x_PORTING == 0 )
#include "si_memsegsupport.h"
#endif
#include "si_platform.h"
#include "si_cra.h"
#include "si_cra_internal.h"
#include "si_cra_cfg.h"
#include "si_i2c.h"
#include "HT68F30.h"
#include "si_osal_timer.h"

#define INTERVAL_WITH_PREV_I2C		1 // ms


#if ( configSII_DEV_953x_PORTING == 1 )
#define I2C_RETRY_TIME 3 /*if I2C retry over this time, MCU will reset it-self*/
#endif

static prefuint_t   l_pageInstance[SII_CRA_DEVICE_PAGE_COUNT] = {0};
extern pageConfig_t g_addrDescriptor[SII_CRA_MAX_DEVICE_INSTANCES][SII_CRA_DEVICE_PAGE_COUNT];
extern SiiReg_t     g_siiRegPageBaseReassign [];
extern SiiReg_t     g_siiRegPageBaseRegs[SII_CRA_DEVICE_PAGE_COUNT];

CraInstanceData_t craInstance =
{
    0,                          // structVersion
    0,                          // instanceIndex
    SII_SUCCESS,                // lastResultCode
    0,                          // statusFlags
};

extern LED_OBJECT *pLED_ObjCtrl;

#if (INTERVAL_WITH_PREV_I2C!=0)
//static volatile clock_time_t i2c_rw_time = 0;
static volatile uint32_t i2c_rw_time = 0;
#endif

static void CraI2CErrorIndicator( void )
{
    pLED_ObjCtrl->Flashing( 39, 41, 5, 10, FALSE );
}

static uint32_t time_diff(uint32_t t)
{
    uint32_t timenow = VirtualTimer_nowMs();
    if(timenow >= t)
    {
        return  timenow-t;
    }
    else
    {
        return 0xffffffff-t + timenow;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read a byte or bytes from the specified bus and i2c device.
//! @param[in]  busIndex
//! @param[in]  deviceId    - I2C device address, eight-bit format (addr + r/w bit)
//! @param[in]  regAddr     - Eight or sixteen bit starting register address
//! @param[in]  pBuffer     - destination buffer.
//! @param[in]  count       - number of registers (bytes) to read.
//! @return     SiiResultCodes_t status of operation (0 == success).
//-------------------------------------------------------------------------------------------------
static SiiResultCodes_t CraReadBlockI2c ( prefuint_t busIndex, uint8_t deviceId, uint16_t regAddr, uint8_t *pBuffer, uint16_t count )
{
    SiiResultCodes_t    status = SII_SUCCESS;
    SiiI2cMsg_t         msgs[2];
    uint8_t             addrData[2];
    int                 addrLen;

    static uint16 read_reboot_cnt = 0;

    if ( pBuffer == NULL )
    {
        status = SII_ERR_FAIL;

        TRACE_ERROR( ( 0, "CraReadBlockI2c pBuffer is null !! " ) );
        return( ( SiiResultCodes_t )status );
    }

    if ( busIndex != DEV_I2C_ADDR16 )
    {
        // Only one byte register address
        addrData[0] = ( uint8_t )regAddr;
        addrLen = 1;
    }
    else
    {
        TRACE_DEBUG( ( 0, "CraReadBlockI2c  DEV_I2C_ADDR16 " ) );
        // Two byte, big endian register address
        addrData[0] = ( uint8_t )( regAddr >> 8 );
        addrData[1] = ( uint8_t )( regAddr );
        addrLen = 2;
    }

    // Split into register address write and register data read messages.
    msgs[0].addr        = deviceId;
    msgs[0].cmdFlags    = SII_MI2C_WR;
    msgs[0].len         = addrLen;
    msgs[0].pBuf        = &addrData[0];
    msgs[1].addr        = deviceId;
    msgs[1].cmdFlags    = SII_MI2C_RD;
    msgs[1].len         = count;
    msgs[1].pBuf        = pBuffer;

    // Send the messages
#if ( configSII_DEV_953x_PORTING == 1 )
    //for( retry_cnt = 0; retry_cnt < I2C_RETRY_TIME; retry_cnt ++  )
    for ( ;; )
    {
#if (INTERVAL_WITH_PREV_I2C!=0)
        if(i2c_rw_time != 0)
        {
            //if(SkTimeDiffMs(i2c_rw_time, SiiOsTimerTotalElapsed())<INTERVAL_WITH_PREV_I2C)
            if(time_diff(i2c_rw_time)<INTERVAL_WITH_PREV_I2C)/*Tony150211:virtualtime is better */
            {
                vTaskDelay(1);
                continue;
            }
        }
#endif
        if ( I2C_readSlave( deviceId, regAddr, ( byte* )pBuffer, count, FALSE ) == TRUE )
        {
#if (INTERVAL_WITH_PREV_I2C!=0)
            i2c_rw_time = VirtualTimer_nowMs();//SiiOsTimerTotalElapsed();
#endif
            status = SII_SUCCESS;
            read_reboot_cnt = 0;
            break;
        }
        else
        {
#if (INTERVAL_WITH_PREV_I2C!=0)
            i2c_rw_time = VirtualTimer_nowMs();//SiiOsTimerTotalElapsed();
#endif
            status = SII_ERR_FAIL;
            if(read_reboot_cnt > 16)
            {
                //if(deviceId!=0xE0 || regAddr!=0xFF) // exception: it should be dummy read after HARD_RESET set, kaomin
                {
                    CraI2CErrorIndicator( );
                }
            }
            TRACE_ERROR( ( 0, "sil9533 reading retry; Error device id 0x%X, regAddr 0x%X", deviceId, regAddr ) );

            read_reboot_cnt++;
#if 1//( configSYS_RESET == 1 )
            if ( read_reboot_cnt >= 666 )
            {
                read_reboot_cnt = 0;
                CPU_resetMicroController( );
            }
#endif
        }
    }
#else
    status = ( SiiResultCodes_t )SiiPlatformI2cTransfer( busIndex, &msgs[0], sizeof( msgs ) / sizeof( SiiI2cMsg_t ) );
#endif

    return( ( SiiResultCodes_t )status );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a byte or bytes to the specified bus and i2c device.
//! @param[in]  busIndex
//! @param[in]  deviceId    - I2C device address, eight-bit format (addr + r/w bit)
//! @param[in]  regAddr     - Eight or sixteen bit starting register address
//! @param[in]  pBuffer     - source data buffer.
//! @param[in]  count       - number of registers (bytes) to write (maximum 256).
//! @return     SiiResultCodes_t status of operation (0 == success).
//-------------------------------------------------------------------------------------------------
static SiiResultCodes_t CraWriteBlockI2c ( prefuint_t busIndex, uint8_t deviceId, uint16_t regAddr, const uint8_t *pBuffer, uint16_t count )
{
    SiiResultCodes_t    status = SII_SUCCESS;
    SiiI2cMsg_t         msg;
    uint8_t             srcData [ 2 + 256]; // Room for 256 register data bytes and one or two register address bytes
    uint16_t            msgLen;
    static uint16 write_reboot_cnt = 0;

    if ( pBuffer == NULL )
    {
        TRACE_ERROR( ( 0, "i2c pBuffer is null " ) );

        status = SII_ERR_FAIL;
        return ( ( SiiResultCodes_t )status );
    }

    if ( busIndex != DEV_I2C_ADDR16 )
    {
        // Only one byte register address
        srcData[0] = ( uint8_t )regAddr;
        msgLen = 1;
    }
    else
    {
        TRACE_DEBUG( ( 0, "dev i2c addr16 = 0x%X", regAddr ) );

        // Two byte, big endian register address
        srcData[0] = ( uint8_t )( regAddr >> 8 );
        srcData[1] = ( uint8_t )( regAddr );
        msgLen = 2;
    }

    // Copy payload data to src buffer following the register address.
    memcpy( &srcData[msgLen], pBuffer, count );
    msgLen += count;

    // Create message
    msg.addr            = deviceId;
    msg.cmdFlags        = SII_MI2C_WR;
    msg.len             = msgLen;
    msg.pBuf            = &srcData[0];

    // Send the messages
#if ( configSII_DEV_953x_PORTING == 1 )
    //for ( retry_cnt = 0; retry_cnt < I2C_RETRY_TIME; retry_cnt ++  )
    for ( ;; )
    {
#if (INTERVAL_WITH_PREV_I2C!=0)
        if(i2c_rw_time != 0)
        {
            //if(SkTimeDiffMs(i2c_rw_time, SiiOsTimerTotalElapsed())<INTERVAL_WITH_PREV_I2C)
            if(time_diff(i2c_rw_time)<INTERVAL_WITH_PREV_I2C)/*Tony150211:virtualtime is better */
            {
                vTaskDelay(1);
                continue;
            }
        }
#endif
        if ( I2C_writeSlave( deviceId, regAddr, ( byte* )pBuffer, count, FALSE, FALSE ) == TRUE )
        {
#if (INTERVAL_WITH_PREV_I2C!=0)
            i2c_rw_time = VirtualTimer_nowMs();//SiiOsTimerTotalElapsed();
#endif
            status = SII_SUCCESS;
            write_reboot_cnt = 0;
            break;
        }
        else
        {
#if (INTERVAL_WITH_PREV_I2C!=0)
            i2c_rw_time = VirtualTimer_nowMs();//SiiOsTimerTotalElapsed();
#endif
            status = SII_ERR_FAIL;

            if(write_reboot_cnt > 16)
            {
                CraI2CErrorIndicator( );
            }
            TRACE_ERROR( ( 0, "sil9533 writing retry; Error device id 0x%X, regAddr 0x%X", deviceId, regAddr ) );

            write_reboot_cnt ++;

#if 1//( configSYS_RESET == 1 )
            if ( write_reboot_cnt >= 666 )
            {
                write_reboot_cnt = 0;
                CPU_resetMicroController( );

            }
#endif
        }
    }
#else
    status = ( SiiResultCodes_t )SiiPlatformI2cTransfer( busIndex, &msg, 1 );
#endif

    return( ( SiiResultCodes_t )status );
}

#if ( configSII_DEV_953x_PORTING == 1 )
static void _AmtranRegReadBlock ( SiiReg_t virtualAddr, uint8_t *pBuffer, uint16_t count )
{
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[ l_pageInstance[ virtualAddr]][ virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset, pBuffer, count );
        break;
    case DEV_I2C_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), pBuffer, count );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }
}

uint8_t _AmtranRegRead ( SiiReg_t virtualAddr )
{
    uint8_t             value = 0;
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[ l_pageInstance[ virtualAddr]][ virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset, &value, 1 );
        break;
    case DEV_I2C_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), &value, 1 );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }

    return( value );
}

static void _AmtranRegWriteBlock ( SiiReg_t virtualAddr, const uint8_t *pBuffer, uint16_t count )
{
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset, pBuffer, count );
        break;
    case DEV_I2C_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), pBuffer, count );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }
}

static void _AmtranRegWrite ( SiiReg_t virtualAddr, uint8_t value )
{
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;
    uint8_t buffer = value;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
    case DEV_I2C_1:
    case DEV_I2C_2:
    case DEV_I2C_3:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( pPage->busType, ( uint8_t )pPage->address, regOffset, &buffer, 1 );
        break;
    case DEV_I2C_OFFSET:
    case DEV_I2C_1_OFFSET:
    case DEV_I2C_2_OFFSET:
    case DEV_I2C_3_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( pPage->busType - DEV_I2C_OFFSET , ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), &buffer, 1 );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }

}
#endif

//------------------------------------------------------------------------------
// Function:    SiiCraInitialize
// Description: Initialize the CRA page instance array and perform any register
//              page base address reassignments required.
// Parameters:  none
// Returns:     None
//------------------------------------------------------------------------------
bool_t SiiCraInitialize ( void )
{
    prefuint_t i, index;
    craInstance.lastResultCode = RESULT_CRA_SUCCESS;

    for ( i = 0; i < SII_CRA_DEVICE_PAGE_COUNT; i++ )
    {
        l_pageInstance[i] = 0;
    }

    // Perform any register page base address reassignments
    i = 0;
    while ( g_siiRegPageBaseReassign[ i] != 0xFFFF )
    {
        index = g_siiRegPageBaseReassign[ i] >> 8;
        if ( ( index < SII_CRA_DEVICE_PAGE_COUNT ) && ( g_siiRegPageBaseRegs[ index] != 0xFF ) )
        {
            // The page base registers allow reassignment of the
            // I2C device ID for almost all device register pages.
            SiiRegWrite( g_siiRegPageBaseRegs[ index], g_siiRegPageBaseReassign[ index] & 0x00FF );
        }
        else
        {
            craInstance.lastResultCode = SII_ERR_INVALID_PARAMETER;
            break;
        }
        i++;
    }

    return( craInstance.lastResultCode == RESULT_CRA_SUCCESS );
}

//------------------------------------------------------------------------------
// Function:    SiiCraGetLastResult
// Description: Returns the result of the last call to a CRA driver function.
// Parameters:  none.
// Returns:     Returns the result of the last call to a CRA driver function
//------------------------------------------------------------------------------
SiiResultCodes_t SiiCraGetLastResult ( void )
{
    return( craInstance.lastResultCode );
}

//------------------------------------------------------------------------------
// Function:    SiiRegInstanceSet
// Description: Sets the instance for subsequent register accesses.  The register
//              access functions use this value as an instance index of the multi-
//              dimensional virtual address lookup table.
// Parameters:  newInstance - new value for instance axis of virtual address table.
// Returns:     None
//------------------------------------------------------------------------------
bool_t SiiRegInstanceSet ( SiiReg_t virtualAddress, prefuint_t newInstance )
{
    prefuint_t va = virtualAddress >> 8;

    craInstance.lastResultCode = RESULT_CRA_SUCCESS;
    if ( ( va < SII_CRA_DEVICE_PAGE_COUNT ) && ( newInstance < SII_CRA_MAX_DEVICE_INSTANCES ) )
    {
        l_pageInstance[ va ] = newInstance;
        return( true );
    }

    craInstance.lastResultCode = SII_ERR_INVALID_PARAMETER;
    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Reads a block of data from sequential registers.
//! @param[in]  virtualAddress  - Sixteen bit virtual register address, including device page.
//! @param[in]  pBuffer     - destination buffer.
//! @param[in]  count       - number of registers (bytes) to read.
//! @return     none
//! @note       This function relies on the auto-increment model used by
//! @note       Silicon Image devices.  Because of this, if a FIFO register
//! @note       is encountered before the end of the requested count, the
//! @note       data remaining from the count is read from the FIFO, NOT
//! @note       from subsequent registers.
//-------------------------------------------------------------------------------------------------
void SiiRegReadBlock ( SiiReg_t virtualAddr, uint8_t *pBuffer, uint16_t count )
{
#if ( configSII_DEV_953x_PORTING == 1 )
    _AmtranRegReadBlock( virtualAddr, pBuffer, count );
#else
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[ l_pageInstance[ virtualAddr]][ virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset, pBuffer, count );
        break;
    case DEV_I2C_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), pBuffer, count );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }
#endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read a one byte register.
//! @param[in]  virtualAddress  - Sixteen bit virtual register address, including device page.
//! @return     eight bit register data.
//-------------------------------------------------------------------------------------------------
uint8_t SiiRegRead ( SiiReg_t virtualAddr )
{
#if ( configSII_DEV_953x_PORTING == 1 )
    uint8_t ret = 0;

    ret = _AmtranRegRead( virtualAddr );

    return ret;
#else

    uint8_t             value = 0;
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[ l_pageInstance[ virtualAddr]][ virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset, &value, 1 );
        break;
    case DEV_I2C_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraReadBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), &value, 1 );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }

    return( value );
#endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      Writes a block of data to sequential registers.
//! @param[in]  virtualAddress  - Sixteen bit virtual register address, including device page.
//! @param[in]  pBuffer     - source data buffer.
//! @param[in]  count       - number of registers (bytes) to write.
//! @return     none
//! @note       This function relies on the auto-increment model used by
//! @note       Silicon Image devices.  Because of this, if a FIFO register
//! @note       is encountered before the end of the requested count, the
//! @note       data remaining from the count is written to the FIFO, NOT
//! @note       to subsequent registers.
//-------------------------------------------------------------------------------------------------
void SiiRegWriteBlock ( SiiReg_t virtualAddr, const uint8_t *pBuffer, uint16_t count )
{
#if ( configSII_DEV_953x_PORTING == 1 )
    _AmtranRegWriteBlock( virtualAddr, pBuffer, count );
#else
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset, pBuffer, count );
        break;
    case DEV_I2C_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( DEV_I2C_0, ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), pBuffer, count );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }
#endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a one byte register.
//! @param[in]  virtualAddress  - Sixteen bit virtual register address, including device page.
//! @param[in]  value           - eight bit data to write to register.
//! @return     None
//-------------------------------------------------------------------------------------------------
void SiiRegWrite ( SiiReg_t virtualAddr, uint8_t value )
{
#if ( configSII_DEV_953x_PORTING == 1 )
    _AmtranRegWrite( virtualAddr, value );
#else
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;
    uint8_t buffer = value;

    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];

    switch ( pPage->busType )
    {
    case DEV_I2C_0:
    case DEV_I2C_1:
    case DEV_I2C_2:
    case DEV_I2C_3:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( pPage->busType, ( uint8_t )pPage->address, regOffset, &buffer, 1 );
        break;
    case DEV_I2C_OFFSET:
    case DEV_I2C_1_OFFSET:
    case DEV_I2C_2_OFFSET:
    case DEV_I2C_3_OFFSET:
        craInstance.lastResultCode = ( SiiDrvCraError_t )CraWriteBlockI2c( pPage->busType - DEV_I2C_OFFSET , ( uint8_t )pPage->address, regOffset + ( uint8_t )( pPage->address >> 8 ), &buffer, 1 );
        break;
    default:
        craInstance.lastResultCode = RESULT_CRA_INVALID_PARAMETER;
        break;
    }
#endif
}

//------------------------------------------------------------------------------
// Function:    SiiRegModify
// Description: Reads the register, performs an AND function on the data using
//              the mask parameter, and an OR function on the data using the
//              value ANDed with the mask. The result is then written to the
//              device register specified in the regAddr parameter.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              mask    - Eight bit mask
//              value   - Eight bit data to be written, combined with mask.
// Returns:     None
//------------------------------------------------------------------------------
void SiiRegModify ( SiiReg_t virtualAddr, uint8_t mask, uint8_t value )
{
    uint8_t aByte;

    aByte = SiiRegRead( virtualAddr );
    aByte &= ( ~mask );                     // first clear all bits in mask
    aByte |= ( mask & value );              // then set bits from value
    SiiRegWrite( virtualAddr, aByte );
}

//------------------------------------------------------------------------------
// Function:    SiiRegBitsSet
// Description: Reads the register, sets the passed bits, and writes the
//              result back to the register.  All other bits are left untouched
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              bits   - bit data to be written
// Returns:     None
//------------------------------------------------------------------------------
void SiiRegBitsSet ( SiiReg_t virtualAddr, uint8_t bitMask, bool_t setBits )
{
    uint8_t aByte;

    aByte = SiiRegRead( virtualAddr );
    aByte = ( setBits ) ? ( aByte | bitMask ) : ( aByte & ~bitMask );
    SiiRegWrite( virtualAddr, aByte );
}

//------------------------------------------------------------------------------
// Function:    SiiRegBitsSetNew
// Description: Reads the register, sets or clears the specified bits, and
//              writes the result back to the register ONLY if it would change
//              the current register contents.
// Parameters:  regAddr - Sixteen bit register address, including device page.
//              bits   - bit data to be written
//              setBits- true == set, false == clear
// Returns:     None
//------------------------------------------------------------------------------
void SiiRegBitsSetNew ( SiiReg_t virtualAddr, uint8_t bitMask, bool_t setBits )
{
    uint8_t newByte, oldByte;

    oldByte = SiiRegRead( virtualAddr );
    newByte = ( setBits ) ? ( oldByte | bitMask ) : ( oldByte & ~bitMask );
    if ( oldByte != newByte )
    {
        SiiRegWrite( virtualAddr, newByte );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Reads a block of data from EDID record over DDC link.
//! @param[in]  segmentAddress  - Sixteen bit virtual EDID segment address, including device page.
//! @param[in]  virtualAddress  - Sixteen bit virtual EDID address, including device page.
//! @param[in]  pBuffer         - source data buffer.
//! @param[in]  count           - number of registers (bytes) to read.
//! @return     none
//-------------------------------------------------------------------------------------------------
void SiiRegEdidReadBlock ( SiiReg_t segmentAddr, SiiReg_t virtualAddr, uint8_t *pBuffer, uint16_t count )
{
    uint8_t             regOffset = ( uint8_t )virtualAddr;
    pageConfig_t        *pPage;
    SiiI2cMsg_t         msgs[3];
    int                 msgIndex = 0;

    // Split into register address write and register data read messages.
    if ( ( segmentAddr & 0xFF ) != 0 )  // Default segment #0 index should not be sent explicitly
    {
        regOffset = ( uint8_t )segmentAddr;
        segmentAddr >>= 8;
        pPage = &g_addrDescriptor[l_pageInstance[segmentAddr]][segmentAddr];

        // Write non-zero EDID segment address
        msgs[0].addr        = pPage->address;
        msgs[0].cmdFlags    = SII_MI2C_WR;
        msgs[0].len         = 1;
        msgs[0].pBuf        = &regOffset;
        msgIndex = 1;
    }

    // Read the actual EDID data
    regOffset = ( uint8_t )virtualAddr;
    virtualAddr >>= 8;
    pPage = &g_addrDescriptor[l_pageInstance[virtualAddr]][virtualAddr];
    msgs[msgIndex].addr        = pPage->address;
    msgs[msgIndex].cmdFlags    = SII_MI2C_WR;
    msgs[msgIndex].len         = 1;
    msgs[msgIndex].pBuf        = &regOffset;
    msgIndex++;
    msgs[msgIndex].addr        = pPage->address;
    msgs[msgIndex].cmdFlags    = SII_MI2C_RD;
    msgs[msgIndex].len         = count;
    msgs[msgIndex].pBuf        = pBuffer;

    // Send the messages
    craInstance.lastResultCode = ( SiiDrvCraError_t )SiiPlatformI2cTransfer( pPage->busType, &msgs[0], sizeof( msgs ) / sizeof( SiiI2cMsg_t ) );
}
