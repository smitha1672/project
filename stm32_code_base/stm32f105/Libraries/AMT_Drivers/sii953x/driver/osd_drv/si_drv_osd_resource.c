//***************************************************************************
//!file     si_drv_osd_resource.c
//!brief    Silicon Image OSD Driver resource functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_drv_osd_internal.h"
#include "si_drv_internal.h"
#include "si_regs_osd.h"
#if INC_SPI
#include "si_drv_spi.h"
#endif
#include "si_osal_timer.h"

//------------------------------------------------------------------------------
//  OSD Driver module Data
//------------------------------------------------------------------------------

static char resourceString [256];

static int fontTypeCharWordCount [] =
{
    OSDRAM_WORDS_PER_12X16,
    OSDRAM_WORDS_PER_16X24,
    OSDRAM_WORDS_PER_24X32,
};

// Font Info for three basic font types supported by SiI9535
int l_fontWidths[]  = { OSD_PIXELS_CHAR_12X16, OSD_PIXELS_CHAR_16X24, OSD_PIXELS_CHAR_24X32 };
int l_fontHeights[] = { OSD_LINES_CHAR_12X16, OSD_LINES_CHAR_16X24, OSD_LINES_CHAR_24X32 };
int l_fontVres[]    = { 480, 720, 1080 };
int l_fontHres[]    = { 720, 1280, 1920 };

//-------------------------------------------------------------------------------------------------
//! @brief      Copies the specified resource header into the passed buffer.
//! @param[in]      resourceId  - Index of resource within resource data
//! @param[in]      pHeader     - Pointer to destination buffer
//! @retval         true if valid resource of specified type.
//! @retval         false if valid resource is not in range or not of specified type.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdGetResourceHeader ( uint16_t resourceId, SiiOsdResourceHeader_t *pHeader )
{
    bool_t      success = false;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    if ( resourceId & OSD_RESOURCE_LOCAL_FLAG)
    {
        resourceId -= OSD_RESOURCE_LOCAL_FLAG;      // Remove local resource flag
        if (( pDrvOsd->pLocalResourceData ) && ( resourceId < pDrvOsd->localResourceCount ))
        {
            memcpy( pHeader, &pDrvOsd->pLocalResourceData[ resourceId], sizeof( SiiOsdResourceHeader_t ));
            return( true );
        }
        return( false );
    }

    do
    {
        // Zero is illegal as a resource ID - we subtract 1 from every value
        if ( resourceId == 0 )
        {
            pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
            break;
        }
        resourceId--;   // Get resourceMap index

        // Are resources loaded?
        if ( pDrvOsd->externalResourceMap[0] == 0 ) // No resource map entry will ever be 0.
        {
            pDrvOsd->lastResultCode = SII_OSDDRV_ERR_RESOURCE_NOT_LOADED;
            break;
        }

        // Verify resource ID
        if ( resourceId >= pDrvOsd->externalResourceCount)
        {
            pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
            break;
        }

        // Read just the resource header.
        if ( SiiDrvSpiRead( pDrvOsd->externalResourceMap[ resourceId], sizeof( SiiOsdResourceHeader_t ), (uint8_t *)pHeader ))
        {
            success = true;
        }
    } while (0);

    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Determines if passed resourceId is valid and of the specified type.
//! @param[in]      resourceType    - Type of resource
//! @param[in]      resourceId      - Index of resource within resource data
//! @retval         true if valid resource of specified type.
//! @retval         false if valid resource is not in range or not of specified type.
//-------------------------------------------------------------------------------------------------
bool_t  SiiDrvOsdIsResource( SiiOsdResourceType_t resourceType, uint16_t resourceId )
{
    SiiOsdResourceHeader_t lHeader;

    if ( SiiDrvOsdGetResourceHeader( resourceId, &lHeader ))
    {
        if ( lHeader.type == resourceType )
        {
            return( true );
        }
    }

    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Load a flash resource into local memory.
//! @param[in]      resourceType    Type of resource
//! @param[in]      resourceId      Index of resource within resource data
//! @param[in][out] pSize           Requested size in bytes of data read. If larger than the
//!                                 actual resource data size, the amount read is returned.
//! @param[in]      pDestData       Destination buffer for data read from flash.
//! @retval         true if successful.
//! @retval         false failure occurred.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdGetResource ( SiiOsdResourceType_t resourceType, uint16_t resourceId, uint32_t *pSize, uint8_t *pDestData )
{
//    SiiTimer_t                  timerInfo;
    SiiOsdResourceHeader_t      lHeader;
    bool_t                      success = false;
    uint32_t                    spiByteAddr, spiByteLength;

//    SiiOsTimerSet( &timerInfo, 0 );
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    do
    {
        if (( resourceId & OSD_RESOURCE_LOCAL_FLAG ) == 0 )
        {
            if ( !SiiDrvOsdGetResourceHeader( resourceId, &lHeader ))
            {
                break;
            }
            if ( lHeader.type != resourceType )
            {
                break;  // LastError value supplied by SiiDrvOsdGetResourceHeader
            }

            // Determine resource index
            resourceId--;   // Get resourceMap index
            spiByteAddr     = pDrvOsd->externalResourceMap[ resourceId];
            spiByteLength   = lHeader.sizeLo + ((uint32_t)lHeader.sizeHi << 8 );

            // Text resource does not use the resource info part.
            if ( resourceType == OSD_RESOURCE_TEXT )
            {
                // Get past the window information part to the text buffer.
                spiByteAddr += sizeof(SiiDrvOsdWindowResource_t) - (OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD);
                spiByteLength = (OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD);
            }
            // For a local buffer window implementation, we would normally load the local window structure info AND
            // load the local window surface buffer from the SPI flash.  That way, updates to the OSDRAM are
            // write-only.  However, since most of the window surface contents are static, we load ONLY the
            // info part of the window resource.  The window contents are loaded directly into OSDRAM via SPI, which
            // is much faster.  This means that when we update a window, we may have to read bits of OSDRAM at the
            // start and end of a string, but we will accept that.
            else if ( resourceType == OSD_RESOURCE_WINDOW )
            {
                spiByteLength = sizeof(SiiDrvOsdWindowResource_t) - (OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD);
            }

            // Read the actual resource data from flash
            success = SiiDrvSpiRead( spiByteAddr, spiByteLength, pDestData );
            *pSize = spiByteLength;
        }
        else
        {
            // If local resource data is present, use it.
            resourceId -= OSD_RESOURCE_LOCAL_FLAG;      // Remove local resource flag
            if (( pDrvOsd->pLocalResourceData ) && ( resourceId < pDrvOsd->localResourceCount ))
            {
                memcpy( pDestData, &pDrvOsd->pLocalResourceData[ resourceId ], sizeof( SiiDrvOsdWindowResource_t ));
                success = true;
            }
        }
    } while (0);

//    DEBUG_PRINT( MSG_DBG, "SiiDrvOsdGetResource: %d ms\n", SiiOsTimerElapsed( timerInfo ));
    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set a pointer to the application local resource data (not flash memory)
//! @param[in]  resourceCount   Number of window resources in local resource data array.
//! @param[in]  pResourceData   Pointer to application local resource data array.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdSetLocalResourceData ( int resourceCount, SiiDrvOsdWindowResource_t const *pResourceData )
{
    pDrvOsd->pLocalResourceData     = pResourceData;
    pDrvOsd->localResourceCount     = resourceCount;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Check the passed pointer to see if it is a valid text resource, and if so,
//!             load the text string from the resource store and return a pointer to it. Otherwise,
//!             return the passed pointer as a text string pointer.
//! @note       If a resource is loaded, it is only valid until the next call to this function.
//! @param[in]  pStr    Resource ID of type OSD_RESOURCE_TEXT, or pointer to a null-terminated
//!                     ASCII string.
//-------------------------------------------------------------------------------------------------
char *SiiDrvOsdGetTextResource ( char *pStr )
{
    char        *pTextStr = 0;
    uint32_t    loadSize;

    // Determine if passed value is a text string resource
    if ( !SiiDrvOsdIsResource( OSD_RESOURCE_TEXT, (int)pStr ))
    {
        // Not a string resource, must be a text string, so return the pointer.
        pTextStr = pStr;
    }
    else
    {
        // Load window data from the specified resource ID.
        loadSize = 256; //TODO: temporary until we get separate resource type structures.
        if ( !SiiDrvOsdGetResource( OSD_RESOURCE_TEXT, (int)pStr, &loadSize, (uint8_t *)resourceString ))
        {
            pDrvOsd->lastResultCode = SII_OSDDRV_RESOURCE_READ_ERR;
        }
        else
        {
            pTextStr = resourceString;
        }
    }

    return( pTextStr );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Font info providers
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdGetFontWidth( void )
{
    return( l_fontWidths[ pDrvOsd->fontSize] );
}
int SiiDrvOsdGetFontHeight( void )
{
    return( l_fontHeights[ pDrvOsd->fontSize] );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Loads the specified window resource into the OSD RAM font location.
//! @param[in]  winRef  Return value from any SiiOsdCreate function.
//! @param[in]  pStr    Resource ID of type OSD_RESOURCE_FONT.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdLoadWindowResource ( int winRef, int winResourceId, bool_t useCurrentWindowSize )
{
//    SiiTimer_t                  timerInfo;
    SiiOsdResourceHeader_t      lHeader;
    bool_t                      success = false;
    uint32_t                    spiByteAddr, spiByteLength;

    // Determine if passed value is a Window resource
//    SiiOsTimerSet( &timerInfo, 0 );
    if (( winResourceId < pDrvOsd->externalResourceCount ) && SiiDrvOsdIsResource( OSD_RESOURCE_WINDOW, winResourceId ))
    {
        winResourceId--;    // Adjust for actual resourceMap index

        // Load the window character indexes into OSD RAM
        // Read resource header from flash into local memory
        spiByteAddr = pDrvOsd->externalResourceMap[ winResourceId];
        if ( SiiDrvSpiRead( spiByteAddr, sizeof( lHeader ), (uint8_t *)&lHeader ))
        {
            // Read the window resource data into OSD RAM
            spiByteLength = (lHeader.sizeLo + ((uint32_t)lHeader.sizeHi << 8 )) - 54;       // TODO: Lee specify this as an expression, not a hardcoded constant
            if ( useCurrentWindowSize )
            {
                spiByteLength = ((pDrvOsd->winData[ winRef].rowCount * pDrvOsd->winData[ winRef].colCount) / OSDRAM_INDEXES_PER_WORD) * OSDRAM_BYTES_PER_WORD;
            }
            if (SiiDrvOsdSpiOsdRamLoad( spiByteAddr + 54, OSD_CONTENT_INDEX_1_OFFSET + pDrvOsd->winData[ winRef].contentIndexStartWord, spiByteLength ))
            {
                if (SiiDrvOsdSpiOsdRamLoad( spiByteAddr + 54, OSD_CONTENT_INDEX_2_OFFSET + pDrvOsd->winData[ winRef].contentIndexStartWord, spiByteLength ))
                {
                    SiiOsdDrvClearDirtyList( &pDrvOsd->winData[ winRef] );
                    success = true;
                }
            }
        }
    }
//    DEBUG_PRINT( MSG_DBG, "SiiDrvOsdLoadWindowResource: %d ms\n", SiiOsTimerElapsed( timerInfo ));
    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Loads the specified font resource into the OSD RAM font location.
//!             load the text string from the resource store and return a pointer to it. Otherwise,
//!             return the passed pointer as a text string pointer.
//! @param[in]  pStr    Resource ID of type OSD_RESOURCE_FONT.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdLoadFontResource ( int fontResourceId )
{
    SiiTimer_t                  timerInfo;
    SiiOsdResourceHeader_t      lHeader;
    bool_t                      success = false;
    uint32_t                    spiByteAddr, spiByteLength;

    // Determine if passed value is a FONT resource
    SiiOsTimerSet( &timerInfo, 0 );
    // Read resource header from flash into local memory
    if ( SiiDrvOsdGetResourceHeader( fontResourceId, &lHeader ))
    {
        if ( lHeader.type == OSD_RESOURCE_FONT)
        {
            fontResourceId--;   // Adjust for actual resourceMap index

            // Read the font bitmap data into OSD RAM
            spiByteLength = lHeader.sizeLo + ((uint32_t)lHeader.sizeHi << 8 );
            spiByteAddr = pDrvOsd->externalResourceMap[ fontResourceId];
            if (SiiDrvOsdSpiOsdRamLoad( spiByteAddr + sizeof(SiiOsdResourceHeader_t),  pDrvOsd->bitmapSegmentStart, spiByteLength - sizeof(SiiOsdResourceHeader_t )))
            {
                // Set font type and determine last character index in loaded font.
                pDrvOsd->lastFontCharacterIndex = (spiByteLength / ( fontTypeCharWordCount[lHeader.winType] * OSDRAM_BYTES_PER_WORD)) - 1;
                pDrvOsd->fontResourceId         = fontResourceId + 1;   // unadjust
                pDrvOsd->fontSize               = (SiiOsdFontSize_t)lHeader.winType;
                pDrvOsd->fontHeight             = l_fontHeights[ pDrvOsd->fontSize ];
                pDrvOsd->fontWidth              = l_fontWidths[ pDrvOsd->fontSize ];
                SiiRegModify( REG_OSD_WIN_CTRL, VAL_OSD_FONT_SIZE_MASK, lHeader.winType << VAL_OSD_FONT_SIZE_SHIFT);
                success = true;
            }

//                if(fontResourceId==35)
//                {
//                SiiRegModify( REG_OSD_WIN_CTRL, VAL_OSD_FONT_SIZE_MASK, (2 << VAL_OSD_FONT_SIZE_SHIFT) );
//                success = true;
//                }
//                else
//                {
//                	SiiRegModify( REG_OSD_WIN_CTRL, VAL_OSD_FONT_SIZE_MASK, lHeader.winType << VAL_OSD_FONT_SIZE_SHIFT);
//                	                success = true;
//                }
           //  }
            //TODO: The first SPI to local memory read seems to fail the first 3 bytes when performed after
            //TODO: a SPI to OSDRAM transfer of more than 2412 bytes.
            SiiDrvSpiRead( spiByteAddr, 6, (uint8_t *)&lHeader );
    //        DEBUG_PRINT( 0, "Header: %02X %02X %02X %02X\n", lHeader.type, lHeader.winType, lHeader.sizeLo, lHeader.sizeHi );
    //        SiiDrvSpiRead( spiByteAddr, sizeof( lHeader ), (uint8_t *)&lHeader );
    //        DEBUG_PRINT( 0, "Header: %02X %02X %02X %02X\n", lHeader.type, lHeader.winType, lHeader.sizeLo, lHeader.sizeHi );
        }
    }
  //  DEBUG_PRINT( MSG_DBG, "SiiDrvOsdLoadFontResource: %d ms\n", SiiOsTimerElapsed( timerInfo ));
    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Read the resource map from the beginning of external flash memory.  This resource
//!             ALWAYS the first resource, and it is NOT in the resource map itself.
//! @param[in]  pStr    Resource ID of type OSD_RESOURCE_TEXT, or pointer to a null-terminated
//!                     ASCII string.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdGetMapResource ( void )
{
//    SiiTimer_t                  timerInfo;
    uint32_t                    spiByteLength;
    bool_t                      success = false;
    uint8_t                     buffer[ sizeof( SiiOsdResourceMapHeader_t)];
    SiiOsdResourceMapHeader_t   *pHeader;
    uint32_t                    firstFontOffset;

    // Unconditionally read the resource header at the beginning of the flash memory
    // (skipping the OSD SPI header)
//    SiiOsTimerSet( &timerInfo, 0 );
    if ( SiiDrvSpiRead( sizeof( SiiOsdSpiHeader_t), sizeof(SiiOsdResourceMapHeader_t), buffer ))
    {
        pHeader = (SiiOsdResourceMapHeader_t *)buffer;
        if ( pHeader->type == OSD_RESOURCE_MAP )
        {
            // Read the resource map into our OSD driver data structure.
            spiByteLength   = pHeader->sizeLo + ((uint32_t)pHeader->sizeHi << 8 );
            spiByteLength   = (spiByteLength < (OSD_RESOURCE_LIMIT * sizeof(uint32_t))) ? spiByteLength : (OSD_RESOURCE_LIMIT * sizeof(uint32_t));
            success = SiiDrvSpiRead( sizeof( SiiOsdSpiHeader_t) + sizeof(SiiOsdResourceMapHeader_t), spiByteLength, (uint8_t *)&pDrvOsd->externalResourceMap );

            // Store actual number of resource IDs read from flash.
            pDrvOsd->externalResourceCount = (spiByteLength - sizeof(SiiOsdResourceMapHeader_t)) / sizeof(uint32_t);

            // Get resource ID of first FONT resource and bump it by 1 because the resource functions expect a non-zero based number.
            firstFontOffset = MAKEUINT32( pHeader->firstFontOffset0, pHeader->firstFontOffset1, pHeader->firstFontOffset2, pHeader->firstFontOffset3 );
            pDrvOsd->defaultFontId = 1 + ((firstFontOffset - (sizeof( SiiOsdSpiHeader_t) + sizeof(SiiOsdResourceMapHeader_t))) / sizeof( uint32_t));
        }
    }

//    DEBUG_PRINT( MSG_DBG, "SiiDrvOsdGetMapResource: %d ms\n", SiiOsTimerElapsed( timerInfo ));
    return( success );
}
