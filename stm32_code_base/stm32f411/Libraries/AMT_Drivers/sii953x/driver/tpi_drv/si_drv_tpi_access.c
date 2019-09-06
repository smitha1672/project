//***************************************************************************
//! @file     si_drv_tpi_access.c
//! @brief    TPI and Legacy registers access
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_tx_component_internal.h"
#include "si_drv_tpi_access.h"

//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

txAccessInstanceRecord_t txAccessInstRec[SII_NUM_TX];
txAccessInstanceRecord_t *pTxRegAccess = &txAccessInstRec[0];

//-------------------------------------------------------------------------------------------------
//  Local Service Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Set a page to access legacy registers through an indirect access mechanism.
//-------------------------------------------------------------------------------------------------

static void SetIndirectLegacyPage(uint8_t currentPage)
{
    if (pTxRegAccess->previouslyAccessedLegacyPage != currentPage)
    {
        SiiRegWrite(REG_TPI__REG_SEL, currentPage & MSK_TPI__REG_SEL__PAGE);
        pTxRegAccess->previouslyAccessedLegacyPage = currentPage;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set an offset to access legacy registers through an indirect access mechanism.
//-------------------------------------------------------------------------------------------------

static void SetIndirectLegacyOffset(uint8_t offset)
{
    if (pTxRegAccess->previouslyAccessedLegacyOffset != offset)
    {
        SiiRegWrite(REG_TPI__REG_OFFSET, offset);
        pTxRegAccess->previouslyAccessedLegacyOffset = offset;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Extract legacy page local address to be used for indirect accessing.
//!
//! @param[in]  fullAddr - 16-bit full address containing a page code and offset.
//-------------------------------------------------------------------------------------------------

static uint8_t ExtractLegacyPage(uint16_t fullAddr)
{
    uint8_t page;

    switch ( fullAddr & 0xFF00 )
    {
        default:
        case TX_PAGE_L0:
            page = VAL_TPI__REG_SEL__PAGE_L0;
            break;

        case TX_PAGE_L1:
            page = VAL_TPI__REG_SEL__PAGE_L1;
            break;

        case TX_PAGE_2:
            page = VAL_TPI__REG_SEL__PAGE_L2;
            break;
    }

    return page;
}

//-------------------------------------------------------------------------------------------------
//  Module API Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Select Tx device instance in multi-transmitter system.
//!
//! @param[in]  device_idx - zero based index of an instance.
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void SiiDrvTpiAccessInstanceSet(uint8_t device_idx)
{
    if(device_idx < SII_NUM_TX)
    {
        pTxRegAccess = &txAccessInstRec[device_idx];
    }
}
#endif // SII_NUM_TX > 1


//-------------------------------------------------------------------------------------------------
//! @brief      Bring TPI access to transmitter into initial operational state.
//!
//!             All initial settings, including the software workarounds
//!             that must be implemented through the legacy registers,
//!             shall be done in this function.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAccessInit(void)
{
    // Set non-existing offset and page to never miss the very first setting
    pTxRegAccess->previouslyAccessedLegacyOffset = 0xFFFF;
    pTxRegAccess->previouslyAccessedLegacyPage   = 0xFF;
    // Legacy TPI mode is set by power on
    pTxRegAccess->isTpiMode = false;

    // Enable TPI access mode for normal operation
    SiiDrvTpiAccessEnable(true);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Switch between TPI and Legacy access modes.
//!
//!             Underlying the TPI register set is a broader and more complex internal
//!             register set that is normally hidden from direct access.  This register set
//!             includes the “legacy” registers that are available for implementation
//!             of software workarounds and debugging.
//!
//! @param[in]  isTpi - true, if TPI mode is selected.
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiAccessEnable(bool_t isTpi)
{
    if (isTpi)
    {
        if (!pTxRegAccess->isTpiMode)
        {
            SiiRegWrite(REG_TX_TPI__ENABLE, BIT_TX_TPI__ENABLE);
            pTxRegAccess->isTpiMode = true;
        }
    }
    else
    {
        if (pTxRegAccess->isTpiMode)
        {
            SiiRegWrite(REG_TPI__REG_SEL, BIT_TPI__REG_SEL__LEGACY_ENABLE);
            pTxRegAccess->isTpiMode = false;
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Write byte to the Legacy registers through a window in TPI page.
//!
//!             Uses indirect access method to write a byte to the Legacy registers
//!             without disabling the TPI mode.
//!
//! @param[in]  regAddr     - Legacy register full address including page and offset,
//! @param[in]  data        - byte to write,
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiIndirectWrite(uint16_t regAddr, uint8_t data)
{
    SiiDrvTpiIndirectBlockWrite(regAddr, &data, 1);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read byte from the Legacy registers through a window in TPI page.
//!
//!             Uses indirect access method to read a byte to the Legacy registers
//!             without disabling the TPI mode.
//!
//! @param[in]  regAddr     - Legacy register full address including page and offset,
//! @return     content of the register
//-------------------------------------------------------------------------------------------------

uint8_t SiiDrvTpiIndirectRead(uint16_t regAddr)
{
    uint8_t data;

    SiiDrvTpiIndirectBlockRead(regAddr, &data, 1);
    return data;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a block of data to the Legacy registers through a window in TPI page.
//!
//!             Uses indirect access method to write data array to the Legacy registers
//!             without disabling the TPI mode.
//!
//! @param[in]  regAddr     - Legacy register full address including page and offset,
//! @param[in]  pData       - Pointer to an input buffer of bytes,
//! @param[in]  length      - Length of the input buffer
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiIndirectBlockWrite(uint16_t regAddr, uint8_t *pData, uint8_t length)
{
    uint8_t page    = ExtractLegacyPage(regAddr);
    uint8_t offset  = regAddr & 0xFF;

    SetIndirectLegacyPage(page);
    for (; length; length--)
    {
        SetIndirectLegacyOffset(offset);
        SiiRegWrite(REG_TPI__REG_DATA, *pData);
        pData++;
        offset++;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read a block of data to the Legacy registers through a window in TPI page.
//!
//!             Uses indirect access method to read data array from the Legacy registers
//!             without disabling the TPI mode.
//!
//! @param[in]  regAddr     - Legacy register full address including page and offset,
//! @param[in]  pData       - Pointer to an input buffer of bytes,
//! @param[in]  length      - Length of the input buffer
//-------------------------------------------------------------------------------------------------

void SiiDrvTpiIndirectBlockRead(uint16_t regAddr, uint8_t *pData, uint8_t length)
{
    uint8_t page    = ExtractLegacyPage(regAddr);
    uint8_t offset  = regAddr & 0xFF;

    SetIndirectLegacyPage(page);
    for (; length; length--)
    {
        SetIndirectLegacyOffset(offset);
        *pData = SiiRegRead(REG_TPI__REG_DATA);
        pData++;
        regAddr++;
    }
}

