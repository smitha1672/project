//************************************************************************************************
//! @file   si_drv_tpi_ddc_access.c
//! @brief  Access to downstream device over DDC line
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#include "si_drv_tpi_ddc_access.h"
#include "si_drv_tpi_access.h"

#define WAIT_FOR_BUS_GRANT      ENABLE

#define SI_DDC_DIRECT_LINK      0
#define SI_DDC_MASTER_TPI       1

#define SI_DDC_CTRL_MODE        SI_DDC_MASTER_TPI


static ddcComErr_t dsDdcError = SI_TX_DDC_ERROR_CODE_NO_ERROR;

//-------------------------------------------------------------------------------------------------
//! @defgroup MDDC_ADDR Downstream DDC device addresses
//! @brief DDC device addresses
//!
//! DDC device addresses to use in
//! SiiDrvTpiDdcReadBlock() and SiiDrvTpiDdcWriteBlock()
//! functions.
//!
//! @see SI_TxDdcReadExtendedBlock(), SI_TxDdcWriteBlock()
//!------------------------------------------------------------------------------------------------
//! @{
#define DDC_PAGE__EDID_DATA       (0x13 << 8)  //!< main EDID DDC page
#define DDC_PAGE__EDID_SEGMENT    (0x14 << 8)  //!< EDID segment DDC page
//
//! @}


//-------------------------------------------------------------------------------------------------
//  Local service functions
//-------------------------------------------------------------------------------------------------

static uint8_t sys_ctrl_reg_shadow = 0;

//-------------------------------------------------------------------------------------------------
//! @brief      Request and accept grant for DDC bus I2C direct access.
//!
//!             Limitation: If WAIT_FOR_BUS_GRANT is not defined,
//!             this function works only with HDCP turned off.
//!
//! @param[in]  isI2cBypassEnable - true:  if local I2C bus must be directly connected to the
//!                                        DDC bus after receiving the grant.
//!                                 false: avoid I2C direct access.
//! @return     true for successfully obtained DDC bus grant.
//-------------------------------------------------------------------------------------------------

static bool_t GrabDsDdcLine(bool_t isI2cBypassEnable)
{
    bool_t isSuccess = true;
    uint8_t sys_ctrl_reg = SiiRegRead(REG_TPI__SYSTEM_CONTROL);
    sys_ctrl_reg_shadow = sys_ctrl_reg;
    sys_ctrl_reg |= BIT_TPI__SYSTEM_CONTROL__DDC_REQUEST;

#if (WAIT_FOR_BUS_GRANT == ENABLE)
    SiiRegWrite (REG_TPI__SYSTEM_CONTROL, sys_ctrl_reg);
    isSuccess = false;

    {
        int i;
        sys_ctrl_reg |= BIT_TPI__SYSTEM_CONTROL__DDC_GRANT;
        for(i=0; i<300; i++)
        {
            uint8_t reg_val = SiiRegRead(REG_TPI__SYSTEM_CONTROL);
            if(reg_val & BIT_TPI__SYSTEM_CONTROL__DDC_GRANT)
            {
                if (isI2cBypassEnable)
                {
                    SiiRegWrite(REG_TPI__SYSTEM_CONTROL, sys_ctrl_reg);
                }
                isSuccess = true;
                break;
            }
        }
    }

#else // WAIT_FOR_BUS_GRANT

    if (isI2cBypassEnable)
    {
        SiiRegWrite(REG_TPI__SYSTEM_CONTROL, sys_ctrl_reg);
        SiiRegRead(REG_TPI__SYSTEM_CONTROL);

        sys_ctrl_reg |= BIT_TPI__SYSTEM_CONTROL__DDC_GRANT;
        SiiRegWrite(REG_TPI__SYSTEM_CONTROL, sys_ctrl_reg);
    }
#endif // WAIT_FOR_BUS_GRANT

    return isSuccess;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Release previously received DDC bus grant.
//-------------------------------------------------------------------------------------------------

static void ReleaseDsDdcLine(void)
{
    uint8_t i;
    uint8_t regVal = 0;

    sys_ctrl_reg_shadow &=
        ~(BIT_TPI__SYSTEM_CONTROL__DDC_REQUEST | BIT_TPI__SYSTEM_CONTROL__DDC_GRANT);

    // Do up to 4 tries to clear bus grant and bus request bits
    for (i = 0; i < 4; ++i)
    {
        SiiRegWrite(REG_TPI__SYSTEM_CONTROL, sys_ctrl_reg_shadow);

        // Read TPI System Control Register to make sure bus request and bus grant
        // bits are cleared. It also triggers I2C access to switch back to TPI.
        regVal = SiiRegRead(REG_TPI__SYSTEM_CONTROL);
        regVal &= BIT_TPI__SYSTEM_CONTROL__DDC_REQUEST | BIT_TPI__SYSTEM_CONTROL__DDC_GRANT;

        if (!regVal)
        {
            // Quit if the bits have been cleared
            break;
        }
    }
}

#if (SI_DDC_CTRL_MODE == SI_DDC_MASTER_TPI)
//-------------------------------------------------------------------------------------------------
//! @brief      Wait for DS DDC operation to finish.
//-------------------------------------------------------------------------------------------------

static bool_t WaitForDdcBus(void)
{
    uint8_t time_out = LEN_TPI__DDC_FIFO_SIZE + 1;
    // time_out is time in ms, which is proportional to the FIFO size.
    // Since the time required to transmit one byte is 100 uS, the time_out
    // is as much as 10 times longer. +1 is additional time due to SI_Sleep() function precision
    // allows error +0-1 ms.
    do
        if(0 == (SiiRegRead(REG_TPI__DDC_STATUS) & MSK_TPI__DDC_STATUS__DDC_I2C_IN_PROG))
            return true;
        else
            SiiOsTimerWait(1);
    while(--time_out);

    return false;
}
#endif

//-------------------------------------------------------------------------------------------------
//  TX DDC Access API functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Initialization of DDC access module.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiDdcInit(void)
{
    dsDdcError = SI_TX_DDC_ERROR_CODE_NO_ERROR;

#if (FPGA_BUILD == ENABLE)
    {
        // Fixes an issue when enabled the DDC stalled on every clock (in FPGA).
        SiiDrvTpiIndirectWrite(REG_TPI__DDC_DOUT_CNT, 0);
    }
#endif

#if (SI_DDC_CTRL_MODE == SI_DDC_MASTER_TPI)
    // DDC speed formula: F_ddc = 1/(200ns * (4 * (delay count + 1)  + 6))
    SiiRegWrite(REG_TPI__DDC_DELAY_CNT, 0x0E); // Set DDC Speed ~ 76 kHz
#endif
}


//-------------------------------------------------------------------------------------------------
//! @brief      Reset DDC master if it got stuck (SWWA:18705).
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiDdcMasterReset(void)
{
    SiiOsTimerWait(1); //SWWA:20334
    GrabDsDdcLine(true);
    ReleaseDsDdcLine();
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return last error code.
//-------------------------------------------------------------------------------------------------

ddcComErr_t SiiDrvTpiDdcGetLastError(void)
{
    return dsDdcError;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Print last DDC access error info.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiDdcErrorsPrint(void)
{
    if (dsDdcError)
    {
        DEBUG_PRINT(MSG_ERR, "DS DDC Errors: ");

        switch (dsDdcError)
        {
            case SI_TX_DDC_ERROR_CODE_TIMEOUT:
                DEBUG_PRINT(MSG_ERR, "TIMEOUT\n");
                break;
            case SI_TX_DDC_ERROR_CODE_NO_ACK:
                DEBUG_PRINT(MSG_ERR, "NO ACK\n");
                break;
            default:
                break;
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Read from downstream DDC device.
//!
//!             The function can be used for downstream EDID or HDCP DDC reading.
//!
//! @param[in]  segmentIndex - EDID segment number; 0 for HDCP DDC
//! @param[in]  regAddr      - HDCP register offset or EDID data offset
//! @param[out] pBuf         - pointer to the buffer
//! @param[in]  length       - number of bytes to read
//!
//! @return     Error code, @see ddcComErr_t
//-------------------------------------------------------------------------------------------------

ddcComErr_t SiiDrvTpiDdcReadBlock(uint8_t segmentIndex, uint8_t regAddr, uint8_t *pBuf, uint16_t length)
{
#if (SI_DDC_CTRL_MODE == SI_DDC_DIRECT_LINK)
    uint8_t hdcpReg = SiiRegRead(REG_TPI__HDCP_CTRL);

    dsDdcError = SI_TX_DDC_ERROR_CODE_NO_ERROR;

    // temporary disable HDCP if it was on
    if((hdcpReg & BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE) == 0)
    {
        SiiRegWrite(REG_TPI__HDCP_CTRL, hdcpReg | BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE);
    }

    // If speed < 100k is necessary for stable DDC communication (e.g., due to long cables),
    // switch to slower i2c data rate
    //I2cSetSlowSpeed(true);

    if(GrabDsDdcLine(true))
    {
        if (length != 0)
        {
            SiiRegEdidReadBlock(TX_PAGE_DDC_SEGM | segmentIndex,
                                TX_PAGE_DDC_EDID | regAddr,
                                pBuf, length );

            if ( SiiCraGetLastResult() != SII_SUCCESS)
            {
                dsDdcError = SI_TX_DDC_ERROR_CODE_NO_ACK;
            }
        }
    }
    else
    {
        dsDdcError = SI_TX_DDC_ERROR_CODE_TIMEOUT;
    }

    ReleaseDsDdcLine();


    // Switch back to full speed i2c communication
    //I2cSetSlowSpeed(false);


    // recover HDCP state that was before this function execution
    if((hdcpReg & BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE) == 0)
    {
        SiiRegWrite(REG_TPI__HDCP_CTRL, hdcpReg);
    }

#else //   SI_DDC_CTRL_MODE == SI_DDC_MASTER_TPI
    uint16_t fifoSize;
    time_ms_t timeOutMs;

    dsDdcError = SI_TX_DDC_ERROR_CODE_NO_ERROR;

    if (GrabDsDdcLine(false))
    {
        BEGIN_TEST
                {

                    FAIL_IF(length == 0);
                    FAIL_IF(!pBuf);

                    if (!WaitForDdcBus())
                    {
                        dsDdcError = SI_TX_DDC_ERROR_CODE_BUSY;
                        FAIL_IF(true);
                    }

                    SiiRegWrite(REG_TPI__DDC_ADDR, 0xA0);
                    SiiRegWrite(REG_TPI__DDC_SEGM, segmentIndex);
                    SiiRegWrite(REG_TPI__DDC_OFFSET, regAddr);
                    SiiRegWrite(REG_TPI__DDC_DIN_CNT2, 0);
                    SiiRegWrite(REG_TPI__DDC_DIN_CNT1, length);
                    SiiRegWrite(REG_TPI__DDC_CMD, CMD_TPI__DDC_CLEAR_FIFO | MSK_TPI__DDC_CMD__ENABLE);
                    SiiRegWrite(REG_TPI__DDC_CMD, (segmentIndex ?
                            CMD_TPI__DDC_ENHANCED_DDC_READ_WITH_NO_ACK_ON_LAST_BYTE :
                            CMD_TPI__DDC_SEQUENTIAL_READ_WITH_NO_ACK_ON_LAST_BYTE) |
                            MSK_TPI__DDC_CMD__ENABLE);

                    timeOutMs = length + 3; // timeout is proportional to length

                    // wait until the FIFO is filled with several bytes
                    SiiOsTimerWait(2); // also makes time aligning

                    do
                    {
                        fifoSize = SiiRegRead(REG_TPI__DDC_DOUT_CNT) & 0x1F;
                        //          test_status = SiiRegRead(REG_TPI__DDC_STATUS);
                        if (fifoSize)
                        {
                            // if the FIFO has some bytes
                            if (fifoSize > length)
                            {
                                dsDdcError = SI_TX_DDC_ERROR_CODE_TX_HW;
                                FAIL_IF(true);
                            }
                            else if (fifoSize > LEN_TPI__DDC_FIFO_SIZE)
                            {
                                dsDdcError = SI_TX_DDC_ERROR_CODE_LIM_EXCEED;
                                FAIL_IF(true);
                            }
                            else
                            {
                                // read fifo_size bytes
                                SiiRegReadBlock(REG_TPI__DDC_DATA, pBuf, fifoSize);
                                length -= fifoSize;
                                pBuf += fifoSize;
                            }
                        }
                        else
                        {
                            SiiOsTimerWait(1); // note, the time is aligned
                            timeOutMs--;
                        }
                    } while (length && timeOutMs);

                    FAIL_IF(dsDdcError);

                    if (0 == timeOutMs)
                    {
                        dsDdcError = SI_TX_DDC_ERROR_CODE_TIMEOUT;
                    }

                }END_TEST

        if (dsDdcError)
        {
            SiiRegWrite(REG_TPI__DDC_CMD, CMD_TPI__DDC_ABORT_TRANSACTION | MSK_TPI__DDC_CMD__ENABLE);
        }

    }
    else
    {
        dsDdcError = SI_TX_DDC_ERROR_CODE_TIMEOUT;
    }

    ReleaseDsDdcLine();

    // Disable DDC Master
    SiiRegWrite(REG_TPI__DDC_CMD, 0);


#endif
    return dsDdcError;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Write into the downstream DDC device.
//!
//!             The function can be used for HDCP DDC writing.
//!
//! @param[in]  regAddr - HDCP register offset,
//! @param[in]  pBuf    - pointer to the buffer,
//! @param[in]  length  - number of bytes to write,
//!
//! @return     Error code, @see ddcComErr_t
//-------------------------------------------------------------------------------------------------

ddcComErr_t SiiDrvTpiDdcWriteBlock(uint8_t regAddr, const uint8_t *pBuf, uint16_t length)
{
    uint8_t hdcpReg = SiiRegRead(REG_TPI__HDCP_CTRL);

    dsDdcError = SI_TX_DDC_ERROR_CODE_NO_ERROR;

    // temporary disable HDCP if it was on
    if((hdcpReg & BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE) == 0)
    {
        SiiRegWrite(REG_TPI__HDCP_CTRL, hdcpReg | BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE);
    }


    // I2cSetSlowSpeed(true);


    if(GrabDsDdcLine(true))
    {
        SiiRegWriteBlock(DDC_PAGE__EDID_DATA | regAddr, pBuf, length);
    }
    else
    {
        dsDdcError = SI_TX_DDC_ERROR_CODE_TIMEOUT;
    }

    ReleaseDsDdcLine();


    //I2cSetSlowSpeed(false);


    // recover HDCP state that was before this function execution
    if((hdcpReg & BIT_TPI__HDCP_CTRL__ENCRYPT_DISABLE) == 0)
    {
        SiiRegWrite(REG_TPI__HDCP_CTRL, hdcpReg);
    }

    return dsDdcError;
}

