//***************************************************************************
//!file     si_drv_edid_rx.c
//!brief    Silicon Image RX EDID driver.  Contains functions for controlling
//          the EDID CEC translation logic in Silicon Image devices.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_osal_timer.h"
#include "si_common.h"
#include "si_drv_nvram_sram.h"
#include "si_drv_edid_rx_internal.h"
#include "si_cra.h"
#include "si_device_config.h"
#include "si_regs_rxedid953x.h"

extern NvramDrvInstanceData_t *pDrvNvram;

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the index of the first byte of the CEC physical address within the
//!             extension block.
//! @param[in]  pData
//! @param[out] pEdidOffset
//!
//! @retval     - true: Found CEC physical address.
//! @retval     - false: Did not find address
//!
//! @remarks    Expects a data array of 128 bytes (EDID_BLOCKLEN)
//-------------------------------------------------------------------------------------------------
static bool_t FindHdmiSignature( uint8_t *pData, uint8_t *pEdidOffset )
{
    int_t   i;
    uint8_t edidOffset = 0xFF;  // signal 'not found'

    if (( pData[0] != 2 ) || ( pData[1] < 3 ))  // Looking for CEA extension block rev 3 or higher.
    {
        return( false );
    }

    i = 4;  // Offset of start of CEA data block collection.
    while ((i < EDID_BLOCK_LEN) && (edidOffset == 0xFF) && (i < pData[2]))
    {
        if (( pData[i] & 0xE0 ) == 0x60 )       // Found Vendor-specific tag
        {
            if ( pData[i + 1] == 0x03 )         // 0x000C03 belongs to HDMI Licensing, LLC
            {
                if ( pData[i + 2] == 0x0C )
                {
                    if ( pData[i + 3] == 0x00 )
                    {
                        edidOffset = i + 4;     // Point to CEC Phys Address field.
                    }
                }
            }
        }
        i += ((pData[i] & 0x1F) + 1);           // Point to next block.
    }

    if ( edidOffset != 0xFF )
    {
        *pEdidOffset = edidOffset;
        return( true );
    }
    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Accepts an EDID data block and updates the CEC physical address field and checksum
//! @brief      registers associated with the specified TX for all ports.
//!             If passed block is not the CEA extension block, no action is taken.
//! @param[in]  txSelect - 0(TX0)
//! @param[in]  pSrc - pointer to an EDID block data
//!
//! @retval     - true: successful.
//! @retval     - false: failure
//!
//! @remarks    Expects an EDID block data array of 128 bytes (EDID_BLOCK_LEN)
//! @remarks    If passed block is the CEA extension block, the physical address and checksum will
//!             be updated prior to writing.  The function replaces the left-most zero nibble in
//!             physical address already contained in the block with the correct value for each port.
//-------------------------------------------------------------------------------------------------
static bool_t UpdateHdmiCecTranslationRegisters ( int_t txSelect, uint8_t *pSrc )
{
    int_t       i;
    uint16_t    mask, increment, physicalAddress;
    uint8_t     zeroPaChecksum, checksum, paOffset;

    pDrvNvram->lastResultCode = SII_DRV_NVRAM_SUCCESS;

    // Obtain offset of physical address if this is a CEA extension
    if ( !FindHdmiSignature( pSrc, &paOffset ))
    {
        // Did not find signature, don't do anything but return success
        return( true );
    }

    // Obtain current physical address in CEA extension
    physicalAddress = (pSrc[paOffset] << 8) | pSrc[paOffset + 1];

    // Remove the current physical address values from the checksum.
    zeroPaChecksum = (0 - pSrc[EDID_BLOCK_LEN - 1]) - pSrc[paOffset] - pSrc[paOffset + 1];

    // Find port index mask of passed physical address. The port index
    // is the value of the first non-zero nibble in the physical address
    // starting from the LSB, plus 1.
    mask = 0x000F;
    increment = 0x0001;         // Used for incrementing the physical address port.
    for ( i = 0; i < 4; i++ )
    {
        if ( physicalAddress & mask )
        {
            if (i == 0)
            {
                // Too many levels. Fail
                pDrvNvram->lastResultCode = SII_DRV_NVRAM_ERR_FAIL;
                return false;
            }
            else
            {
                // move to left-most zero
                mask >>= 4;
                increment >>= 4;
                break;
            }
        }
        mask <<= 4;
        increment <<= 4;
    }

    // Update the translation registers for ALL ports.
    //physicalAddress = (physicalAddress & ~mask);
    SiiRegWrite( REG_CEC_PA_ADDR, paOffset + EDID_BLOCK_LEN );
    for ( i = 0; i < SII_INPUT_PORT_COUNT; i++ )
    {
        // Update physical address for this port.
        physicalAddress += increment;

        SiiRegWrite( REG_CEC_PA_L_CH0 + (i * 2), (uint8_t)(physicalAddress >> 8) );
        SiiRegWrite( REG_CEC_PA_H_CH0 + (i * 2), (uint8_t)physicalAddress);

        // Update checksum for this port.
        checksum = 0x100 - (zeroPaChecksum + (physicalAddress & 0xFF) + ((physicalAddress >> 8) & 0x00FF));
        SiiRegWrite( REG_CEC_CHECKSUM_CH0 + i, checksum );
    }

    // Transfer the new value to the appropriate internal translation registers.
    SiiRegWrite( REG_CECPA_CTRL,
                 BIT_CEC_PULSE | BIT_TX0_SRC_REG | (( txSelect ) ?  BIT_CEC_DEST_TX1 : 0) );

    return( pDrvNvram->lastResultCode == SII_DRV_NVRAM_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Accepts an EDID data block and updates the CEC physical address field and checksum.
//! @brief      If passed block is not the CEA extension block, no action is taken.
//! @param[in]  portIndex - input port selection,
//! @param[in]  paOffcet  - Byte offset to location of the Physical Address first byte
//! @param[in]  physicalAddress - original Physical Address assigned by downstream
//! @param[in]  pSrc - pointer to an EDID block data
//!
//! @retval     - true: successful.
//! @retval     - false: failure
//!
//! @remarks    Expects a data array of 128 bytes (EDID_BLOCK_LEN)
//! @remarks    If passed block is the CEA extension block, the physical address and checksum will
//!             be updated prior to writing.  The function replaces the left-most zero nibble in
//!             physical address already contained in the block with the correct value for each port.
//!
//! @warning    The source data must be writable because CEC physical address and checksum values
//! @warning    are updated before writing to the SRAM destination
//-------------------------------------------------------------------------------------------------
static bool_t UpdateHdmiCecData ( int_t portIndex, uint8_t paOffset, uint16_t physicalAddress, uint8_t *pSrc )
{
    int_t       i;
    uint16_t    mask;
    uint8_t     checksum;

    pDrvNvram->lastResultCode = SII_DRV_NVRAM_SUCCESS;

    // Remove the current physical address values from the checksum.
    checksum = (0 - pSrc[EDID_BLOCK_LEN - 1]) - pSrc[paOffset] - pSrc[paOffset + 1];

    // Find port index segment of passed physical address. The port index
    // is the value of the first non-zero nibble in the physical address
    // starting from the LSB, plus 1.
    mask = 0x000F;
    for ( i = 0; i < 4; i++ )
    {
        if ( physicalAddress & mask )
        {
            if (i == 0)
            {
                physicalAddress = 0xFFFF;       // Too many levels. Set to F.F.F.F
            }
            else
            {
                // Replace the port in the physical address with the passed port.
                physicalAddress = physicalAddress | ((portIndex + 1) << ((i-1)*4));
            }
            break;
        }
        mask <<= 4;
    }

    // Update source data with new AP and checksum
    pSrc[paOffset + 1]  = (uint8_t)physicalAddress;
    pSrc[paOffset ]     = (uint8_t)(physicalAddress >> 8);
    pSrc[EDID_BLOCK_LEN - 1] = 0x100 - (checksum + pSrc[paOffset] + pSrc[paOffset + 1]);

    return( pDrvNvram->lastResultCode == SII_DRV_NVRAM_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  pData
//!
//! @retval     - 128 byte checksum value
//!
//! @remarks    Expects a data array of 128 bytes (EDID_BLOCKLEN)
//-------------------------------------------------------------------------------------------------

int_t SiiDrvNvramEdidBlockChecksumCalc( uint8_t *pData  )
{
    int_t   i;
    uint8_t accum = 0;

    for ( i = 0; i < EDID_BLOCK_LEN; i++ )
    {
        accum += pData[i];
    }

    return( accum );    // Return value should be 0 for valid checksum
}

//-------------------------------------------------------------------------------------------------
//! @brief      Sets the EDID that the next call to SiiDrvNvramEdidBlockWrite() will write.
//! @param[in]  newMode - EDID mode (legacy or 2-edid)
//! @remarks    This is a SiI9535-specific function.
//-------------------------------------------------------------------------------------------------
void SiiDrvNvramEdidModeSet ( SiiNvramEdidMode_t newMode )
{
    switch (newMode)
    {
        default:
        case SII_NVRAM_LEGACY_EDID_MODE:
            SiiRegModify(REG_EDID_CTRL_XTRA, BIT_TWO_EDID_MODE, 0);
            break;

        case SII_NVRAM_TWO_EDID_MODE:
            SiiRegModify(REG_EDID_CTRL_XTRA, BIT_TWO_EDID_MODE, BIT_TWO_EDID_MODE);
            break;
    }

    pDrvNvram->edidMode = newMode;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Accepts EDID data and writes to the port SRAMs appropriate to the current EDID
//!             mode.  Typically used by a TX in repeater mode when updating the upstream EDID
//!             from the downstream EDID.  In legacy mode, all port SRAMS will be written with
//!             the passed data.  In TWO-EDID mode, either SRAM 0 or SRAM 1 is written, depending
//!             upon the ramIdx argument.
//! @param[in]  txSelect    SRAM selection (SRAM_P0 or SRAM_P1). Ignored in Legacy mode.
//! @param[in]  pSrc        Block of EDID data to be written (EDID_BLOCK_LEN bytes)
//! @param[in]  blockNum    0 - 1. Block number being written.
//!
//! @retval     - true: successful.
//! @retval     - false: failure
//!
//! @remarks    Expects a data array of 128 bytes (EDID_BLOCK_LEN)
//! @remarks    If passed block is the CEA extension block, the physical address and checksum will
//!             be updated prior to writing.  The function replaces the lowest non-zero nibble in
//!             physical address already contained in the block with the correct value for each port.
//!
//! @warning    The source data must be writable because CEC physical address and checksum values
//! @warning    are updated before writing to the SRAM destination
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvNvramEdidBlockWrite ( SiiSramType_t txSelect, uint8_t *pSrc, int_t blockNum )
{
    uint8_t  offset, ramSelect = 0;
    uint8_t  paOffset = 0;
    uint16_t dsPa = 0;
    bool_t   isCeaExt = false;

    pDrvNvram->lastResultCode = SII_DRV_NVRAM_SUCCESS;

    offset = blockNum * EDID_BLOCK_LEN;   // 0x00 or 0x80
    switch ( pDrvNvram->edidMode )
    {
        case SII_NVRAM_LEGACY_EDID_MODE:

            // Obtain offset of physical address if this is a CEA extension
            isCeaExt = FindHdmiSignature( pSrc, &paOffset );

            if (isCeaExt)
            {    // Obtain current physical address in CEA extension
                 dsPa = (pSrc[paOffset] << 8) | pSrc[paOffset + 1];
            }

            // Legacy mode writes each SRAM with its own EDID
            for ( ramSelect = 0; ramSelect < SII_INPUT_PORT_COUNT; ramSelect++ )
            {
                // Update CEC physical address and checksum in source data if CEA extension block.
                if (isCeaExt)
                {
                    UpdateHdmiCecData( ramSelect, paOffset, dsPa, pSrc );
                }

                // Point to offset into selected port SRAM.
                SiiRegModify( REG_EDID_FIFO_SEL, MSK_SEL_EDID_FIFO, ramSelect );
                SiiRegWrite( REG_EDID_FIFO_ADDR, offset );

                // Write data into the destination.
                SiiRegWriteBlock( REG_EDID_FIFO_DATA, pSrc, EDID_BLOCK_LEN );
            }
            break;

        case SII_NVRAM_TWO_EDID_MODE:

            // Tx0 goes into SRAM 0, Tx1 into SRAM 1
            ramSelect = (txSelect == SRAM_P0) ? SRAM_P0 : SRAM_P1;

            // Point to offset into selected port SRAM.
            SiiRegModify( REG_EDID_FIFO_SEL, MSK_SEL_EDID_FIFO, ramSelect );
            SiiRegWrite( REG_EDID_FIFO_ADDR, offset );

            // Write data into the destination (don't need to update CEC data).
            SiiRegWriteBlock( REG_EDID_FIFO_DATA, pSrc, EDID_BLOCK_LEN );

            // Update translation registers
            UpdateHdmiCecTranslationRegisters( txSelect, pSrc );
            break;

        default:
            pDrvNvram->lastResultCode = SII_DRV_NVRAM_ERR_INVALID_PARAMETER;
            break;
    }

    return( pDrvNvram->lastResultCode == SII_DRV_NVRAM_SUCCESS );
}

