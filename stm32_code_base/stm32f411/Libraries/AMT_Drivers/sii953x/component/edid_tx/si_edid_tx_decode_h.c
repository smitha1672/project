//************************************************************************************************
//! @file   si_edid_tx_decode_h.c
//! @brief  EDID parsing and decoding. High-level API
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#include <string.h>
#include <stdio.h>
#include "si_edid_tx_component.h"
#include "si_edid_tx_internal.h"
#include "si_edid_black_list.h"
#include "si_drv_tpi_ddc_access.h"
#include "si_drv_nvram_sram.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Check correctness of the block 0 header.
//!
//! @param[in]  aBlock - pointer to EDID block 0 array
//!
//! @retval     true  - success
//! @retval     false - header is broken
//-------------------------------------------------------------------------------------------------

static bool_t CheckEdidBlock0Header(uint8_t aBlock[EDID_BLOCK_SIZE])
{
    // Check EDID header
    if ( 0 != memcmp(&aBlock[EDID_BL0_ADR__HEADER], &defaultDsEdid[EDID_BL0_ADR__HEADER], EDID_BL0_LEN__HEADER) )
        return false;
    return true;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Check correctness of the block 0 version field.
//!
//! @param[in]  aBlock - pointer to EDID block 0 array
//!
//! @retval     true  - success
//! @retval     false - header is broken
//-------------------------------------------------------------------------------------------------

static bool_t CheckEdidBlock0Version(uint8_t aBlock[EDID_BLOCK_SIZE])
{
    // Check EDID version
    if (aBlock[EDID_BL0_ADR__VERSION] != 1) // only 1.x versions are allowed (not 2.0)
        return false;
    return true;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Decode the passed EDID block 0.
//!
//! @param[in]  aBlock - pointer to EDID block 0 array
//-------------------------------------------------------------------------------------------------

void EdidTxParseBlock0 ( uint8_t aBlock[EDID_BLOCK_SIZE] )
{

#ifdef COLLECT_EXCESSIVE_INFO
    pEdidTx->pEdidDecodData->Revision = aBlock[EDID_BL0_ADR__REVISION];
#else // COLLECT_EXCESSIVE_INFO
    pEdidTx->pEdidDecodData->Revision = aBlock[EDID_BL0_ADR__REVISION];
#endif // COLLECT_EXCESSIVE_INFO

    pEdidTx->pEdidDecodData->edidInfo.ManufacturerId =
        (aBlock[EDID_BL0_ADR__MANUFACTURER_ID] << 8) |
        aBlock[EDID_BL0_ADR__MANUFACTURER_ID+1];
    pEdidTx->pEdidDecodData->edidInfo.ProductId =
        aBlock[EDID_BL0_ADR__PRODUCT_ID] |
        (aBlock[EDID_BL0_ADR__PRODUCT_ID+1] << 8);

#ifdef COLLECT_EXCESSIVE_INFO
    pEdidTx->pEdidDecodData->edidInfo.ShortSerialNumber =
        aBlock[EDID_BL0_ADR__SHORT_SERIAL_NUMBER] |
        (aBlock[EDID_BL0_ADR__SHORT_SERIAL_NUMBER+1] << 8) |
        (aBlock[EDID_BL0_ADR__SHORT_SERIAL_NUMBER+2] << 16) |
        (aBlock[EDID_BL0_ADR__SHORT_SERIAL_NUMBER+3] << 24);
#endif // COLLECT_EXCESSIVE_INFO

    pEdidTx->pEdidDecodData->edidInfo.ManufactureWeek = aBlock[EDID_BL0_ADR__MANUFACTURE_WEEK];
    pEdidTx->pEdidDecodData->edidInfo.ManufactureYear = aBlock[EDID_BL0_ADR__MANUFACTURE_YEAR];
    pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming1 = aBlock[EDID_BL0_ADR__ESTABLISHED_TIMING_1];
    pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming2 = aBlock[EDID_BL0_ADR__ESTABLISHED_TIMING_2];
    pEdidTx->pEdidDecodData->edidInfo.ManufacturerReservedTiming = aBlock[EDID_BL0_ADR__MANUFACTURER_RESERVED_TIMING];

    memcpy( pEdidTx->pEdidDecodData->edidInfo.BasicDispayParameters,
        &aBlock[EDID_BL0_ADR__BASIC_DISPLAY_PARAMS],
        EDID_BL0_LEN__BASIC_DISPLAY_PARAMS );
    memcpy( pEdidTx->pEdidDecodData->edidInfo.Chromaticity,
        &aBlock[EDID_BL0_ADR__CHROMATICITY],
        EDID_BL0_LEN__CHROMATICITY );
    memcpy( pEdidTx->pEdidDecodData->edidInfo.StandardTiming,
        &aBlock[EDID_BL0_ADR__STANDARD_TIMING],
        EDID_BL0_LEN__STANDARD_TIMING );

    if ( 0 == ( pEdidTx->pEdidDecodData->edidInfo.BasicDispayParameters[0] & 0x80) )
    {
        // EDID is marked as for analog TV
        // (some of DVI TVs have this bug)
        pEdidTx->pEdidDecodData->edidInfo.BasicDispayParameters[0] = 0x80;
        pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__BLOCK_0_VIOLATION;
    }

    EdidTxDecodeDtd(&aBlock[EDID_BL0_ADR__DTD1]);
    EdidTxDecodeDtd(&aBlock[EDID_BL0_ADR__DTD2]);
    EdidTxDecodeDtd(&aBlock[EDID_BL0_ADR__DTD3]);
    EdidTxDecodeDtd(&aBlock[EDID_BL0_ADR__DTD4]);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Load block 0 of the selected EDID and verify that it is at least
//!             nominally a valid block 0 EDID.  If not, replace it with the
//!             default.  Decode the result.
//!
//! @param[in]  pEdid - pointer to EDID block 0.
//-------------------------------------------------------------------------------------------------

static void EdidDecodeBlock0 ( uint8_t *pEdid )
{
//    bool_t replaceEdidwithDefault = false;

    for ( ;; )
    {
        // Read the EDID. Give it two chances.

        if ( !EdidTxReadBlock( 0, pEdid ))
        {
            if ( !EdidTxReadBlock( 0, pEdid))
            {
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__CANNOT_READ_BL0 | EDID_ERROR__DEFAULT_EDID_USED;
                break;
            }
        }

        /* Verify that block 0 of the EDID is nominally a real block 0 EDID.    */

        if ( pEdidTx->pEdidDecodData->edidInfo.ErrorCode & EDID_ERROR__CHECK_SUM_MISMATCH )
        {
            if ( !pEdidTx->parseOptions.AllowCheckSumMismatchForBlock0)
            {
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DEFAULT_EDID_USED;
                break;
            }
        }
        if ( !CheckEdidBlock0Header(pEdid))
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__INCORRECT_HEADER | EDID_ERROR__DEFAULT_EDID_USED;
        }
        if ( !CheckEdidBlock0Version(pEdid))
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__INCORRECT_VERSION | EDID_ERROR__DEFAULT_EDID_USED;
        }
        break;
    }

    // If a checksum error occurred but is not fatal (parseOptions.AllowCheckSumMismatchForBlock0 == true),
    // AND the EDID has more than one block, we can ignore the checksum if the checksum for block 1 is OK.
    // This chunk of code reads in the second block and verifies the checksum.  If it cannot read the second
    // block, or the second block checksum is bad, force use of the default block 0 EDID.

    if (( pEdidTx->pEdidDecodData->edidInfo.ErrorCode & EDID_ERROR__CHECK_SUM_MISMATCH ) &&
       !( pEdidTx->pEdidDecodData->edidInfo.ErrorCode & EDID_ERROR__DEFAULT_EDID_USED ))
    {
        // Multiple extensions were specified.

        if ( 0 != pEdid[EDID_BL0_ADR__EXTENSION_NMB]) 
        {
            // If block 0 had mismatched CS, ensure that at least one other block has correct CS
            if ( !EdidTxReadBlock( 1, pEdid ))
            {
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__CANNOT_READ_EXT | EDID_ERROR__DEFAULT_EDID_USED;
            } 

            // If there was a checksum error this time, it's fatal.

            else if ( pEdidTx->pEdidDecodData->edidInfo.ErrorCode & EDID_ERROR__CHECK_SUM_MISMATCH )
            {
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DEFAULT_EDID_USED;
            }
            else
            {
                EdidTxReadBlock( 0, pEdid );     // No problems with block 1, re-read block 0 into edid array.
            }
        }

        // No extensions, use basic DVI default EDID.

        else
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DEFAULT_EDID_USED;
        }
    }

    // Replace read EDID with default one if suggested.  We assume the default has none
    // of the problems listed above.

    if ( pEdidTx->pEdidDecodData->edidInfo.ErrorCode & EDID_ERROR__DEFAULT_EDID_USED )
    {
        memcpy( pEdid, defaultDsEdid, EDID_BLOCK_SIZE);
    }

    // Decode the resulting block 0 EDID

    EdidTxParseBlock0( pEdid );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Load and decode an extension EDID block.
//!
//! @param[in]  blockIdx - EDID block index,
//! @param[in]  pEdid    - pointer to EDID block
//-------------------------------------------------------------------------------------------------

static void EdidDecodeBlockExtension ( uint8_t blockIdx, uint8_t *pEdid )
{
    if ( !EdidTxReadBlock( blockIdx, pEdid))
    {
        pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__CANNOT_READ_EXT;
        DEBUG_PRINT( MSG_ERR, "\nCannot read EDID Block %d", (int)blockIdx);
    }
    else
    {
        EdidTxParseBlock( pEdid, blockIdx );
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Parse downstream EDID into an internal structure.
//!             Also performs some basic error checking on the EDID.
//-------------------------------------------------------------------------------------------------
//extern const uint8_t gEdidFlashEdidTable [ EDID_TABLE_LEN ];

void EdidTxDecodeEdid ( void )
{
    uint8_t edid[EDID_BLOCK_SIZE];
	int    extensionCount = 0;

    //memcpy(edid, gEdidFlashEdidTable, EDID_BLOCK_SIZE);

    // Configuration may force the DVI mode in the output
    pEdidTx->pEdidDecodData->edidInfo.isHdmiMode = !pEdidTx->parseOptions.DviInput;

    // In Sound Bar mode, we take the basic properties, including video, from
    // the local RX EDID and the Audio properties from the downstream (Sound Bar) EDID
	// when we would normally take everything from the downstream EDID

    pEdidTx->isDecodeAudio        = true;
    pEdidTx->isDecodeNonAudio     = true;
    if ( pEdidTx->isSoundBarMode )
    {
        pEdidTx->isDecodeAudio = false;
    }

    EdidDecodeBlock0( edid );
    extensionCount = edid[EDID_BL0_ADR__EXTENSION_NMB];

    DEBUG_PRINT( EDID_TX_MSG_DBG, "DS blocks read: %d \n", extensionCount + 1 );

    //memcpy(edid, &gEdidFlashEdidTable[EDID_BLOCK_SIZE], EDID_BLOCK_SIZE);

    if ( pEdidTx->isSoundBarMode )
    {
        // If there is at least 1 extension block, decode NON-audio from NVRAM EDID block 1

        if ( extensionCount > 0 )
        {
            EdidDecodeBlockExtension( 1, edid );
        }

        // The rest of the decoding is audio only, from the DS EDID
        pEdidTx->isDecodeAudio        = true;
        pEdidTx->isDecodeNonAudio     = false;
    }

    // One extension is simple, just decode it.

    if ( extensionCount == 1)
    {
        EdidDecodeBlockExtension( 1, edid );
    }

    // If more than 1 extension, the first extension must be a map block,
    // so read it first, then decode the rest of the extensions.

    else if ( extensionCount > 1 )
    {
        uint8_t map[EDID_BLOCK_SIZE];
        if ( !EdidTxReadBlock( 1, map ))
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__CANNOT_READ_EXT;
        }
        else
        {
            if ( EDID_EXT_TAG__BLOCK_MAP != map[0])
            {
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__INCORRECT_MAP_TAG;
            }
            else
            {
                int blockIndex;
                if (extensionCount > 127) // 126 timings extensions + one map extension
                {
                    extensionCount = 127;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__TOO_MANY_EXTENSIONS;
                }
                // block 0 is the main block
                // block 1 is a map
                // block 2 ... nmb_of_extension+1  are timings extensions
                for (blockIndex = 2; blockIndex <= extensionCount; blockIndex++)
                {
                    if (EDID_EXT_TAG__ADDITIONAL_TIMINGS == map[blockIndex-1])
                    {
                        EdidDecodeBlockExtension( blockIndex, edid );
                    }
                }
            }
        }
    }

    // if no extension, it must be a DVI EDID

    else 
    {
        // DVI monitor case.
        // Very often DVI monitors support more modes then declared in their EDIDs.
        // Probably it is due to they use just one EDID block and there is not enough
        // room to list all the modes.
        // Many existing players know that and allow playing
        // some additional modes even though they are not indicated by the monitor EDID.

        DEBUG_PRINT( EDID_TX_MSG_DBG, ( "DS DVI EDID Detected, adding DVI video modes\n" ));

        pEdidTx->pEdidDecodData->edidInfo.isHdmiMode = false;

        EdidTxAddDviVideoModes();
    }

    EdidTxPostDecode();

    // Check for remedies
    if ( pEdidTx->parseOptions.enableRemedies)
    {
        pEdidTx->pEdidDecodData->edidInfo.RemedyMask = EdidTxRemedyMaskBuild( &pEdidTx->pEdidDecodData->edidInfo );
    }

    // make a correction if required
    if ( pEdidTx->pEdidDecodData->edidInfo.RemedyMask & SI_TX_EDID_REMEDY_DO_NOT_USE_YCBCR)
    {
        pEdidTx->pEdidDecodData->edidInfo.SupportYCbCr = false;
    }

}
