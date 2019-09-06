//***************************************************************************
//!file     si_drv_osd_tools.c
//!brief    Silicon Image OSD Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_osd_internal.h"
#include "si_drv_osd_internal.h"
#include "si_drv_internal.h"
#include "si_osd_resource.h"
#include "si_regs_osd.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Return the index of the first available hardware window.
//! @param[in]  windowTypeMask - type of windows to look for, AVAILABLE and/or STATUS.
//! @return     Index of first available hardware window
//! @retval     0 - 3 if a hardware window is available.
//! @retval     -1 if no hardware window is available.
//-------------------------------------------------------------------------------------------------
static int FindAvailableWindow ( SiiOsdWindowStatusTypes_t windowTypeMask )
{
    int winIndex;

    for ( winIndex = 0; winIndex < SII_OSD_WIN_COUNT; winIndex++ )
    {
        if ( pDrvOsd->winData[ winIndex].windowStatus & windowTypeMask & (OSD_WSTAT_AVAILABLE | OSD_WSTAT_STATUS_WINDOW))
        {
            if ( pDrvOsd->winData[ winIndex].windowStatus & OSD_WSTAT_STATUS_WINDOW )
            {
                // If an open status window, close it first.
                SiiDrvOsdShowWindow( winIndex, OPT_HIDE );
                SiiDrvOsdCloseWindow( winIndex );
            }
            break;
        }
    }

    if ( winIndex >= SII_OSD_WIN_COUNT )
    {
        winIndex = -1;
    }

    return( winIndex );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Load a 12-bit index value into the correct bytes of the OSD
//!             RAM word within the passed byte array.
//! @param[in]  pBuffer     - destination index array.
//! @param[in]  charIndex   - OSD RAM Bitmap character index
//! @param[in]  indexValue  - 12-bit index value to write.
//-------------------------------------------------------------------------------------------------
void OsdSetIndexValue ( uint8_t *pBuffer, int charIndex, int indexValue )
{
    int     wordIndex, wordBitIndex, wordByteIndex, arrayByteIndex;
    uint8_t indexByte;

    wordIndex       = charIndex / OSDRAM_INDEXES_PER_WORD;          // OSD RAM word index
    wordBitIndex    = (charIndex % OSDRAM_INDEXES_PER_WORD) * 12;   // Number of bits into selected word using remainder char index
    wordByteIndex   = wordBitIndex / 8;                             // Byte within word of remainder bits.

    arrayByteIndex  = (wordIndex * OSDRAM_BYTES_PER_WORD) + wordByteIndex;
    if ( (wordBitIndex % 8) != 0)
    {
        indexByte = pBuffer[ arrayByteIndex ];
        pBuffer[ arrayByteIndex] = ( indexByte & 0xF0 ) | ((indexValue >> 8) & 0x0F);       // axx  --> xa xx
        pBuffer[ arrayByteIndex + 1] = (indexValue & 0xFF);                                 // xbc  --> xa bc
    }
    else
    {
        pBuffer[ arrayByteIndex] = (indexValue >> 4) & 0xFF;                                // abx  --> ab xx
        indexByte = (pBuffer[ arrayByteIndex + 1] & ~0xF0);
        pBuffer[ arrayByteIndex + 1] = (indexByte | ((indexValue << 4) & 0xF0));            // xxc  --> ab cx
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write the dirty parts of the content index array of the target window to the
//!             current background surface in OSDRAM (Content Index segment).
//! @param[in]  winRef      - Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  backSurface - Current background surface index.
//! @retval     false if window not available or there was nothing to write.
//-------------------------------------------------------------------------------------------------
static bool_t OsdWriteBackgroundSurface ( int winRef, int backSurface )
{
    SiiDrvOsdWindow_t   *pWin = &pDrvOsd->winData[ winRef];
    int                 i;
    uint16_t            segmentStart, surfaceBit;
    bool_t              cleaned = false;

    if ( pWin->windowStatus & OSD_WSTAT_AVAILABLE )
    {
        return( false );
    }

    // Determine surface information.
    segmentStart    = (backSurface == 0) ? OSD_CONTENT_INDEX_1_OFFSET : OSD_CONTENT_INDEX_2_OFFSET;
    surfaceBit      = 1 << backSurface;

    // Write to OSDRAM from this surface's dirty list.
    for ( i = 0; i < 32; i++ )
    {
        // If entry is dirty, write to OSDRAM and mark as clean.
        if ( pWin->dirtyList[i].dirtyBits & surfaceBit )
        {
            SiiDrvOsdRamI2cWrite(
                pWin->dirtyList[i].startWord + pWin->contentIndexStartWord + segmentStart,
                pWin->dirtyList[i].wordCount,
                &pWin->contentIndexWordData[ pWin->dirtyList[i].startByte] );
            pWin->dirtyList[i].dirtyBits &= ~surfaceBit;    // no longer dirty
            cleaned = true;
        }
    }
    return( cleaned );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Swap the ping-pong content index buffers and update the instance data accordingly.
//! @note       If TPI is at D2 state, a swap may not take place even if there was stuff to
//! @note       update.  This is OK, because the display is not on.  The information is still
//! @note       written to the background surface and will be updated when the next swap is
//! @note       successful
//! @retval     true if the background surface was written and swapped to the foreground
//! @retval     false if there was nothing to write (surface not swapped in this case).
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdSwapSurfaces ( void )
{
    int     winIndex, i, foreSurface, backSurface;
    bool_t  surfaceWasWritten = false;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    // Determine current background surface
    foreSurface     = ((SiiRegRead( REG_OSD_INDEX_CTRL) >> 4) & 0x03) - 1;  // Treat the surface bits as an index
    backSurface     = (foreSurface + 1) % 2;

    // Update the background surface with any unwritten changes from all windows.
    for ( winIndex = 0; winIndex < SII_OSD_WIN_COUNT; winIndex++ )
    {
        if ( OsdWriteBackgroundSurface( winIndex, backSurface ))
        {
            surfaceWasWritten = true;
        }
    }

    if ( surfaceWasWritten )
    {
        // Let the viewer see the result by starting the surface swap.
        SiiRegBitsSet( REG_OSD_INDEX_CTRL, REG_BIT_INDEX_SWAP, true );

        // Must wait for swap to complete at next vertical interval.
        i = 0;
        winIndex = (backSurface + 1 ) << 4;
        while (( SiiRegRead( REG_OSD_INDEX_CTRL) & VAL_INDEX_MASK ) != winIndex )
        {
            if ( i > MAX_VERT_PERIOD_MS )
            {
                break;
            }
            SiiOsTimerWait( 2 );
            i++;
        }
    }

    pDrvOsd->lastResultCode = (surfaceWasWritten) ? SII_OSDDRV_SUCCESS : SII_OSDDRV_OSDRAM_NOTHING_WRITTEN;
    return( surfaceWasWritten );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return the font index of the character under the passed row/col cursor.
//! @param[in]  winRef      Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  row         row within window of character (0-15)
//! @param[in]  col         column within row character (0-31)
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdGetCharIndex( int winRef, int row, int col )
{
    SiiDrvOsdWindow_t   *pWin = &pDrvOsd->winData[ winRef];
    bool_t      isOffset;
    int         indexValue, startWordIndex, byteIndex, startCharIndex;
    int         foreSurface, backSurface;
    uint16_t    segmentStart;
    uint8_t     tBuffer [10];     // buffer for one OSDRAM word.

    // Determine background surface.
    foreSurface = ((SiiRegRead( REG_OSD_INDEX_CTRL) >> 4) & 0x03) - 1;  // Treat the surface bits as an index
    backSurface = (foreSurface + 1) % 2;
    segmentStart = (backSurface == 0) ? OSD_CONTENT_INDEX_1_OFFSET : OSD_CONTENT_INDEX_2_OFFSET;

    // Find OSD RAM word containing character in question.
    startCharIndex  = ((row * pWin->colCount) + col);
    startWordIndex  = startCharIndex / OSDRAM_INDEXES_PER_WORD;     // from start of window...

    // Get the OSD RAM word, then pull out the specific character index.
    indexValue = 0;
    if ( SiiDrvOsdRamI2cRead( startWordIndex + pWin->contentIndexStartWord + segmentStart, 1, tBuffer ))
    {
        byteIndex   = ((startCharIndex % OSDRAM_INDEXES_PER_WORD) * 12) / 8;
        isOffset    = !(( startCharIndex * 12 ) % 8) == 0;

        if ( isOffset )
        {
            indexValue = (uint16_t)(tBuffer[ byteIndex ] & 0x0F ) << 8;         // xa   --> xa xx
            indexValue = indexValue | tBuffer[ byteIndex + 1];                  // bc   --> xa bc
        }
        else
        {
            indexValue = (uint16_t)(tBuffer[ byteIndex ]) << 4;                 // ab   --> xa bx
            indexValue = indexValue | ((tBuffer[ byteIndex + 1] & 0xF0) >> 4);  // cx   --> xa bc
        }
    }
    return( SiiDrvOsdCbIndexToCharacter( indexValue ));
}

//-------------------------------------------------------------------------------------------------
//! @brief      Create or remove a cursor character of the specified type.  The CURSOR_INVERT type
//!             inverts the character beneath the cursor position, the CURSOR_BOX type places a box
//!             around the character beneath the cursor position. There can only be one of each
//!             type of cursor active at any time across all windows.
//! @note       This function uses patch character locations 0 (CURSOR_INVERT) and 1 (CURSOR_BOX)
//! @param[in]  winRef          Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  cursorType      CURSOR_BOX, CURSOR_INVERT
//! @param[in]  removeCursor    true - remove an existing cursor of the specified type.
//! @param[in]  row             row within window of string start (0-15)
//! @param[in]  col             column within row of string start (0-31)
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdCursorChar ( int winRef, int cursorType, bool_t removeCursor, int row, int col )
{
    int     i, charIndex;
    uint8_t wordByteData [OSDRAM_WORDS_PER_16X24 * OSDRAM_BYTES_PER_WORD];

    // Remove any previous cursor character
    SiiDrvOsdClearCharPatch(( cursorType == CURSOR_BOX ) ? 1 : 0 );
    if ( removeCursor )
    {
        // If only removing the cursor, we're done.
        return;
    }

    // Read the font bitmap of the character in the specified location from OSD RAM
    charIndex = SiiDrvOsdCbCharacterToIndex( SiiDrvOsdGetCharIndex( winRef, row, col ));
    if ( SiiDrvOsdRamI2cRead( pDrvOsd->bitmapSegmentStart + ( charIndex * OSDRAM_WORDS_PER_16X24), OSDRAM_WORDS_PER_16X24, wordByteData ))
    {
        if ( cursorType == CURSOR_INVERT )
        {
            // Invert the pixels in the bitmap
            for ( i = 0; i < (OSDRAM_WORDS_PER_16X24 * OSDRAM_BYTES_PER_WORD); i++ )
            {
                wordByteData[i] = ~wordByteData[i];
            }
        }
        else
        {
            // Write a box around the edge of the bitmap
            wordByteData[0] = 0x3F;
            wordByteData[1] = 0xFC;
            wordByteData[2] = 0x40;
            wordByteData[3] = 0x02;

            for ( i = 4; i < OSDRAM_WORDS_PER_16X24 * OSDRAM_BYTES_PER_WORD - 4; i += 2)
            {
                wordByteData[i]     |= 0x80;
                wordByteData[i + 1] |= 0x01;
            }
            wordByteData[OSDRAM_WORDS_PER_16X24 * OSDRAM_BYTES_PER_WORD - 4] = 0x40;
            wordByteData[OSDRAM_WORDS_PER_16X24 * OSDRAM_BYTES_PER_WORD - 3] = 0x02;
            wordByteData[OSDRAM_WORDS_PER_16X24 * OSDRAM_BYTES_PER_WORD - 2] = 0x3F;
            wordByteData[OSDRAM_WORDS_PER_16X24 * OSDRAM_BYTES_PER_WORD - 1] = 0xFC;
        }

        // Write it out to invert or box character font bitmap location in OSDRAM
        charIndex = ( cursorType == CURSOR_BOX ) ? SII_OSD_BOX_CHAR : SII_OSD_INVERT_CHAR ;
        if ( SiiDrvOsdRamI2cWrite( pDrvOsd->bitmapSegmentStart + (charIndex * OSDRAM_WORDS_PER_16X24), OSDRAM_WORDS_PER_16X24, wordByteData ))
        {
            // Patch the character into the specified row/col
            SiiDrvOsdSetCharPatch( winRef, true, row, col, charIndex, ( cursorType == CURSOR_BOX ) ? 1 : 0 );
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write an icon to the specified window row and column. The icon is character-based
//!             and can be any size up to the row/col size of the window. The function is passed an
//!             array of index values representing the bitmap characters in the icon and writes
//!             them to one or more locations in the inactive content index buffer.  If the
//!             options OPT_SHOW flag is set, the inactive and active content index buffers are swapped.
//! @note       SiiDrvOsdWriteIcon does NOT translate index values passed via the pIcon parameter as
//!             the SiiDrvWriteText function does, so the actual font character index values must be used.
//! @param[in]  winRef      Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  row         row within window of string start (0-15)
//! @param[in]  col         column within row of string start (0-31)
//! @param[in]  rowCount    height of icon in character rows
//! @param[in]  colCount    width of icon in character columns
//! @param[in]  pIcon       Pointer to array of icon character indexes or resourceId if OPT_RESOURCEID set.
//! @param[in]  options     OPT_RESOURCEID, OPT_SHOW, OPT_REMOVE_ICON
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdWriteIcon( int winRef, int row, int col, int rowCount, int colCount, uint8_t *pIcon, SiiOsdOptions_t options )
{
    SiiDrvOsdWindow_t   *pWin = &pDrvOsd->winData[ winRef];
    int                 i, y;
    uint8_t             *pContent;
    uint32_t            loadSize;
    uint8_t             resourceData[ sizeof( SiiDrvOsdWindowResource_t )];
    SiiDrvOsdWindowResource_t   *pResource;

    if ( pWin->windowStatus & OSD_WSTAT_AVAILABLE)
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
        return;
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    pContent = 0;

    // If OPT_RESOURCEID flag set, pIcon is a resourceId, not a pointer.
    if (( options & OPT_RESOURCEID ) != 0 )
    {
        // Load window data from the specified resource ID.
        loadSize = sizeof(SiiDrvOsdWindowResource_t);
        if ( !SiiDrvOsdGetResource( OSD_RESOURCE_ICON, (int)pIcon, &loadSize, resourceData ))
        {
            pDrvOsd->lastResultCode = SII_OSDDRV_RESOURCE_READ_ERR;
        }
        else
        {
            pResource = (SiiDrvOsdWindowResource_t *)&resourceData[0];

            // Transfer resource data to our window data structure.
            rowCount    = pResource->rowCount;
            colCount    = pResource->colCount;
            pContent    = pResource->contentIndexWordData;
        }
    }
    else
    {
        pContent = pIcon;
    }

    // Update the content index array
    i = 0;
    for ( y = row; y < ( row + rowCount); y++ )
    {
        SiiDrvOsdWriteTextRow( pWin, y, col, (char *)pContent, colCount, options | OPT_NO_TRANSLATE );
        pContent += colCount;
    }

    // If OPT_SHOW requested, flip the active and inactive buffers.
    if ( options & OPT_SHOW )
    {
        SiiDrvOsdSwapSurfaces();
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a text string to the specified window row and column.
//!             Write character index values corresponding to each character in the
//!             passed array to the inactive content index buffer.  If showOptions OPT_SHOW
//!             flag is set, the inactive and active content index buffers are swapped.
//! @param[in]  winRef      Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  row         row within window of string start (0-15)
//! @param[in]  col         column within row of string start (0-31)
//! @param[in]  pStr        Pointer to ASCII string, null terminated
//! @param[in]  options     OPT_RESOURCEID, OPT_ELLIPSIS, OPT_CLEAR_TO_END, OPT_CENTER_TEXT, OPT_WRAP_TEXT, OPT_SHOW
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdWriteText( int winRef, int row, int col, char *pStr, SiiOsdOptions_t options )
{
    SiiDrvOsdWindow_t   *pWin = &pDrvOsd->winData[ winRef];
    int                 i, len, offset, writeLength, y, rowCount;
    bool_t              success, addEllipsis;
    uint32_t            loadSize;
    uint8_t             resourceData[ sizeof( SiiDrvOsdWindowResource_t )];
    SiiDrvOsdWindowResource_t   *pResource;
    char                *pContent = NULL;

    if ( pWin->windowStatus & OSD_WSTAT_AVAILABLE)
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
        return( false );
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    success                 = false;
    addEllipsis             = false;

    // Is pStr a resourceId, or a pointer?
    if (( options & OPT_RESOURCEID ) != 0 )
    {
        // Load window data from the specified resource ID.
        loadSize = sizeof(SiiDrvOsdWindowResource_t);
        if ( !SiiDrvOsdGetResource( OSD_RESOURCE_TEXT, (int)pStr, &loadSize, resourceData ))
        {
            pDrvOsd->lastResultCode = SII_OSDDRV_RESOURCE_READ_ERR;
        }
        else
        {
            pResource   = (SiiDrvOsdWindowResource_t *)&resourceData[0];
            pContent    = (char *)pResource->contentIndexWordData;
        }
    }
    else
    {
        pContent = pStr;
    }

    // If text string available, write it to window background surface
    if ( pContent )
    {
        writeLength = strlen( pContent );
        if (( options & OPT_WRAP_TEXT ) == 0 )
        {
            // If string is longer than remaining space in row, either cut it to size or
            // cut it to size with an ellipsis character depending upon the settings.
            if ( writeLength > (pWin->colCount - col ))
            {
                writeLength = (pWin->colCount - col );
                if ( options & OPT_ELLIPSIS )
                {
                    writeLength--;
                    addEllipsis = true;
                }
            }

            // Center text if requested and possible
            if ( options & OPT_CENTER_TEXT )
            {
                if ( writeLength < ( pWin->colCount - 2))
                {
                    col = (pWin->colCount / 2) - (writeLength / 2);
                }
            }
            else if ( options & OPT_RIGHT_TEXT )
            {
                if ( writeLength < ( pWin->colCount - 2))
                {
                    col = pWin->colCount - writeLength;
                }
            }
        }
        // Update the content index array
        i = 0;
        rowCount = ( options & OPT_WRAP_TEXT ) ? pWin->rowCount : row + 1;
        for ( y = row; y < rowCount; y++ )
        {
            len = ( writeLength > (pWin->colCount - col)) ? (pWin->colCount - col) : writeLength;
            if ( len <= 0 )
            {
                break;
            }
            SiiDrvOsdWriteTextRow( pWin, y, col, pContent, len, options );
            pContent += len;
            writeLength -= len;
        }

        // Add ellipsis character if needed.
        if (( options & OPT_WRAP_TEXT ) == 0 )
        {
            if ( addEllipsis )
            {
                offset = (row * pWin->colCount) + (pWin->colCount - 1);
                OsdSetIndexValue( pWin->contentIndexWordData, offset, SiiDrvOsdCbCharacterToIndex( SII_OSD_CHAR_ELLIPSIS ));
            }
        }

        // If OPT_SHOW requested, flip the active and inactive buffers.
        if ( options & OPT_SHOW )
        {
            SiiDrvOsdSwapSurfaces();
        }
    }

    return( pDrvOsd->lastResultCode == SII_OSDDRV_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a blank text string to the specified window from the specified row and column
//!             to the end of the row. If showOptions OPT_SHOW flag is set, the inactive and
//!             active content index buffers are swapped.
//! @param[in]  winRef      Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  row         row within window of string start (0-15)
//! @param[in]  col         column within row of string start (0-31)
//! @param[in]  options     OPT_SHOW
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdClearTextRow ( int winRef, int row, int col, SiiOsdOptions_t options )
{
    SiiDrvOsdWriteText( winRef, row, col, " ", (options & ~OPT_ELLIPSIS) | OPT_CLEAR_TO_END );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Move the highlight row for the specified window. If OPTW_OSD_CURSOR flag
//!             is set, the row is enabled; otherwise the current highlight state applies.
//! @param[in]  winRef      Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  row         row within window of string start (0-15)
//! @param[in]  options     OPT_HIDE, OPT_SHOW, OPT_ENABLE_CURSOR
//! @return     bool        true: success, false: requested row is disabled or out of range.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdSetCursor ( int winRef, int row, SiiOsdOptions_t options )
{
    SiiDrvOsdWindow_t   *pWin;
    int                 regSetOffset;

    if ( winRef == -1 )
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
        return( false );
    }
    pWin = &pDrvOsd->winData[ winRef];

    // Return fail if not legal window row.
    if ( pWin->windowStatus & OSD_WSTAT_AVAILABLE)
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
        return( false );
    }
  
    if ((( row > pWin->rowCount ) || (((1 << row) & pWin->disabledRowMap))) && !(row == 5))
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
        return( false );
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    regSetOffset = (winRef * OSD_REG_BLOCK_OFFSET);

    // Enable the cursor if requested
    if ( OPT_ENABLE_CURSOR )
    {
        pWin->options |= OPTW_OSD_CURSOR;
    }

    // Actual hardware cursor row gets changed no matter what.
    SiiRegModify( REG_OSD_W0_ROW_CTRL + regSetOffset, MSK_CURSOR_SELECTED, row );
    if (( pWin->options & OPTW_OSD_CURSOR ) && ( options & OPT_SHOW ))
    {
        // To show the cursor, set the selected row colors.
        SiiRegWrite( REG_OSD_W0_SELECTED_FG + regSetOffset, pWin->foreColorHighlight );
        SiiRegWrite( REG_OSD_W0_SELECTED_BG + regSetOffset, pWin->backColorHighlight );
        pWin->highlightEnabled  = true;
        pWin->highlightRow      = row;
    }
    else
    {
        // To hide the cursor, match the unselected row colors.
        SiiRegWrite( REG_OSD_W0_SELECTED_FG + regSetOffset, pWin->foreColor );
        SiiRegWrite( REG_OSD_W0_SELECTED_BG + regSetOffset, pWin->backColor );
        pWin->highlightEnabled = false;
    }

    return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Move the specified window to the specified x-y location on the screen.
//! @Note       startX and startY are treated as virtual values with a 1:1 equivalence to the
//! @Note       default font.  If the current font is not the default font, the x and y
//! @Note       locations are scaled relative to the default.  This allows this function to be
//! @Note       used for resolution change window updates.
//! @param[in]  winRef      - Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  startX      - Upper left pixel horizontal coordinate of window
//! @param[in]  startY      - Upper left pixel vertical coordinate of window
//! @param[in]  options
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdMoveWindow( int winRef, int startX, int startY, SiiOsdOptions_t options )
{
    SiiDrvOsdWindow_t   *pWin = &pDrvOsd->winData[ winRef];

    if ( pWin->windowStatus & OSD_WSTAT_AVAILABLE)
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
        return;
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    // Update window data
    pWin->upperLeftX      = startX & ~WIN_X_ALIGN_MASK;
    pWin->upperLeftY      = startY & ~WIN_Y_ALIGN_MASK;

    // Update hardware if requested
    if ( options & OPT_SHOW)
    {
        SiiDrvOsdSetHorizVert( winRef, pWin->upperLeftX, pWin->upperLeftY );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      If options OPT_SHOW flag is set, the window is enabled, otherwise it is
//!             disabled. This is the only valid options bit.  If the OSD_WSTAT_INITIALIZE_HW
//!             flag is set in the windowStatus flags for the target window, the window
//!             parameters are loaded from the window data structure into the device hardware.
//! @param[in]  winRef    - Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  options
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdShowWindow ( int winRef, SiiOsdOptions_t options )
{
    SiiDrvOsdWindow_t   *pWin = &pDrvOsd->winData[ winRef];

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    // If show requested, enable the window.
    if ( options & OPT_SHOW )
    {
        // Swap to background surface before enabling the window.
        SiiDrvOsdSwapSurfaces();
        SiiDrvOsdWndEnable( winRef, true );
        pWin->windowStatus &= ~OSD_WSTAT_INVISIBLE;
    }
    else
    {
        SiiDrvOsdWndEnable( winRef, false );
        pWin->windowStatus |= OSD_WSTAT_INVISIBLE;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Fill out the component window structure of the specified hardware window.
//! @param[in]  winRef      - Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  pOsdWin     - Pointer to OSD component window data structure
//! @return                 - true if successful, false if failure occurred.
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdGetWindowProperties( int winRef, void *pWin )
{
    bool_t              success = false;
    SiiDrvOsdWindow_t   *pDrvWin = NULL;
    SiiOsdWindow_t      *pOsdWin    = (SiiOsdWindow_t *)pWin;

    pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
    if (( winRef != -1) && ( winRef < SII_OSD_WIN_COUNT ))
    {
        pDrvWin = &pDrvOsd->winData[ winRef];

        // Transfer driver data to component data structure.
        pOsdWin->windowType         = pDrvWin->winType;
        pOsdWin->windowStatus       = pDrvWin->windowStatus;
        pOsdWin->resourceId         = pDrvWin->resourceId;
        pOsdWin->options            = pDrvWin->options;
        pOsdWin->upperLeftX         = pDrvWin->upperLeftX;
        pOsdWin->upperLeftY         = pDrvWin->upperLeftY;
        pOsdWin->rowCount           = pDrvWin->rowCount;
        pOsdWin->colCount           = pDrvWin->colCount;

        pOsdWin->highlightRow       = pDrvWin->highlightRow;
        pOsdWin->borderWidth        = pDrvWin->borderWidth;
        pOsdWin->dividerWidth       = pDrvWin->dividerWidth;

        pOsdWin->foreColor          = pDrvWin->foreColor;
        pOsdWin->backColor          = pDrvWin->backColor;
        pOsdWin->foreColorHighlight = pDrvWin->foreColorHighlight;
        pOsdWin->backColorHighlight = pDrvWin->backColorHighlight;
        pOsdWin->foreColorDisabled  = pDrvWin->foreColorDisabled;
        pOsdWin->backColorDisabled  = pDrvWin->backColorDisabled;
        pOsdWin->borderColor        = pDrvWin->borderColor;
        pOsdWin->dividerColor       = pDrvWin->dividerColor;

        pOsdWin->alphaBlendLevel    = pDrvWin->alphaBlendLevel;
        pOsdWin->msTimeout          = pDrvWin->msTimeout;

        memcpy( pOsdWin->rowChildWindowId, pDrvWin->rowChildWindowId, sizeof( pOsdWin->rowChildWindowId ));

        pOsdWin->windowFunctionId       = pDrvWin->windowFunctionId;
        pOsdWin->param1                 = pDrvWin->param1;
        pOsdWin->param2                 = pDrvWin->param2;
        pOsdWin->param3                 = pDrvWin->param3;
        pOsdWin->param4                 = pDrvWin->param4;
        pOsdWin->editMode               = pDrvWin->editMode;

        pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
        success = true;
    }

    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize a window data structure
//! @param[in]  windowId    - ID of resource to provide window contents and properties. If 0, create
//!                           an empty window.
//! @param[in]  startX      - Upper left pixel horizontal coordinate of window
//! @param[in]  startY      - Upper left pixel vertical coordinate of window
//! @param[in]  rowCount    - Number of character rows in window (1-16)
//! @param[in]  colCount    - Number of character columns per row (1-32)
//! @retval                 - Window reference value if successful
//! @retval                 - -1 if creation failed.
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdCreateWindow( int windowId, int startX, int startY, int rowCount, int colCount )
{
    int                 i, winRef, regSetOffset;
    uint16_t            indexStartWord;
    uint32_t            loadSize;
    uint8_t             resourceData[ sizeof( SiiDrvOsdWindowResource_t )];
    SiiDrvOsdWindow_t   *pWin = NULL;
    SiiDrvOsdWindowResource_t   *pResource;

    // Special flag to force use of a specific window, in use or not.
    // Only used with resource ID window (windowID != 0).  Intended to
    // assist the SiiOsdReplaceWindow() function
    if ( startX == -1 )
    {
        winRef = startY;
    }
    else
    {
        // If there is no available hardware windows, return fail
        winRef    = FindAvailableWindow( OSD_WSTAT_AVAILABLE | OSD_WSTAT_STATUS_WINDOW );
        if ( winRef == -1 )
        {
            pDrvOsd->lastResultCode = SII_OSDDRV_ERR_NOT_AVAIL;
            return( -1 );
        }
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    pWin = &pDrvOsd->winData[ winRef];

    // Got a window, now fill out our copy of the window data.

    // If no window ID specified, use the default window style.
    if ( windowId == 0 )
    {
        pWin->winType               = OSD_WINTYPE_BASIC;
        pWin->upperLeftX            = startX        & ~WIN_X_ALIGN_MASK;
        pWin->upperLeftY            = startY        & ~WIN_Y_ALIGN_MASK;
        pWin->rowCount              = rowCount;
        pWin->colCount              = colCount;

        pWin->highlightRow          = 0;
        pWin->highlightEnabled      = false;

        pWin->borderWidth           = OSD_DEFAULT_BORDERWIDTH;
        pWin->dividerWidth          = OSD_DEFAULT_DIVIDERWIDTH;

        pWin->disabledRowMap        = 0;

        pWin->foreColor             = OSD_DEFAULT_FG_COLOR;
        pWin->backColor             = OSD_DEFAULT_BG_COLOR;
        pWin->foreColorHighlight    = OSD_DEFAULT_FG_HIGHLIGHT_COLOR;
        pWin->backColorHighlight    = OSD_DEFAULT_BG_HIGHLIGHT_COLOR;
        pWin->foreColorDisabled     = OSD_DEFAULT_FG_DISABLED_COLOR;
        pWin->backColorDisabled     = OSD_DEFAULT_BG_DISABLED_COLOR;
        pWin->borderColor           = OSD_DEFAULT_BORDER_COLOR;
        pWin->dividerColor          = OSD_DEFAULT_DIVIDER_COLOR;

        pWin->alphaBlendLevel       = OSD_DEFAULT_ALPHA_BLEND;
        pWin->msTimeout             = OSD_DEFAULT_TIMEOUT;

        memset( pWin->rowChildWindowId, 0, sizeof( pWin->rowChildWindowId ));
        pWin->windowFunctionId      = 0;
        pWin->param1                = 0;
        pWin->param2                = 0;
        pWin->param3                = 0;
        pWin->param4                = 0;
        pWin->editMode              = 0;

        // Initialize content index array to all character 0.
        memset( pWin->contentIndexWordData,
            0,
            OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD );

        // Force data to OSD RAM by setting entire buffer dirty.
        SiiOsdDrvClearDirtyList( pWin );
        SiiDrvOsdUpdateDirtyList( pWin, 0, OSD_CI_WINSIZE_SIZE, 0 );
    }
    else
    {
        // Load window data from the specified resource ID.
        loadSize = sizeof(SiiDrvOsdWindowResource_t);
        if ( !SiiDrvOsdGetResource( OSD_RESOURCE_WINDOW, windowId, &loadSize, resourceData ))
        {
            winRef = -1;
            pDrvOsd->lastResultCode = SII_OSDDRV_RESOURCE_READ_ERR;
        }
        else
        {
            pResource = (SiiDrvOsdWindowResource_t *)&resourceData[0];

            memset( (uint8_t *)pWin, 0, sizeof( SiiDrvOsdWindow_t ));

            // Transfer resource data to our window data structure.
            pWin->winType           = pResource->resHeader.winType;
            pWin->resourceId        = pResource->wIdLo | ((uint16_t)pResource->wIdHi << 8);
            pWin->options           = pResource->optionsLo | ((uint16_t)pResource->optionsHi << 8);
            pWin->upperLeftX        = pResource->horizStartLo | ((uint16_t)pResource->horizStartHi << 8);
            pWin->upperLeftY        = pResource->vertStartLo | ((uint16_t)pResource->vertStartHi << 8);
            pWin->rowCount          = pResource->rowCount;
            pWin->colCount          = pResource->colCount;

            pWin->highlightRow      = pResource->highlightRow;

            pWin->borderWidth       = pResource->borderWidth;
            pWin->dividerWidth      = pResource->dividerWidth;
            pWin->disabledRowMap    = pResource->disabledRowMapLo | ((uint16_t)pResource->disabledRowMapHi << 8);

            pWin->foreColor             = pResource->foreColor;
            pWin->backColor             = pResource->backColor;
            pWin->foreColorHighlight    = pResource->foreColorHighlight;
            pWin->backColorHighlight    = pResource->backColorHighlight;
            pWin->foreColorDisabled     = pResource->foreColorDisabled;
            pWin->backColorDisabled     = pResource->backColorDisabled;
            pWin->borderColor           = pResource->borderColor;
            pWin->dividerColor          = pResource->dividerColor;

            pWin->alphaBlendLevel       = pResource->alphaBlendLevel;
            pWin->msTimeout             = pResource->msDisplayTimeLo | ((uint16_t)pResource->msDisplayTimeHi << 8);

            memcpy( pWin->rowChildWindowId, pResource->rowChildWindowId, sizeof( pWin->rowChildWindowId ));
            memcpy( pWin->contentIndexWordData, pResource->contentIndexWordData, sizeof(pWin->contentIndexWordData )); //TODO: This is actually not useful

            pWin->windowFunctionId      = pResource->windowFunctionId;
            pWin->param1                = pResource->param1;
            pWin->param2                = pResource->param2;
            pWin->param3                = pResource->param3;
            pWin->param4                = pResource->param4;
            pWin->editMode              = pResource->editMode;
        }
    }

    pWin->contentIndexStartWord     = winRef * OSD_CI_WINSIZE_SIZE;

    // Disable any rows not used by this window.
    for ( i = pWin->rowCount; i < OSD_WIN_ROW_COUNT; i++ )
    {
        pWin->disabledRowMap |= (1 << i);
    }

    // Window always starts out invisible.
    pWin->windowStatus |= OSD_WSTAT_INVISIBLE | OSD_WSTAT_ENABLED;
    pWin->windowStatus &= ~OSD_WSTAT_AVAILABLE;

    // Load the hardware
    regSetOffset = (winRef * OSD_REG_BLOCK_OFFSET);

    // Location and size
    SiiDrvOsdMoveWindow( winRef, pWin->upperLeftX, pWin->upperLeftY, OPT_SHOW );
    SiiDrvOsdSetWindowSize( winRef, pWin->rowCount, pWin->colCount );

    // Disabled row mask
    SiiRegWrite( REG_OSD_W0_DISABLED_ROW_L + regSetOffset, pWin->disabledRowMap & 0xFF );
    SiiRegWrite( REG_OSD_W0_DISABLED_ROW_H + regSetOffset, (pWin->disabledRowMap >> 8) & 0xFF );

    // Colors
    SiiRegWrite( REG_OSD_W0_BORDER + regSetOffset, pWin->borderColor );
    SiiRegWrite( REG_OSD_W0_DIVIDER + regSetOffset, pWin->dividerColor );

    // Hardware cursor is always off at startup
    SiiRegWrite( REG_OSD_W0_SELECTED_FG + regSetOffset, pWin->foreColor );
    SiiRegWrite( REG_OSD_W0_SELECTED_BG + regSetOffset, pWin->backColor );

    SiiRegWrite( REG_OSD_W0_UNSEL_FG + regSetOffset, pWin->foreColor );
    SiiRegWrite( REG_OSD_W0_UNSEL_BG + regSetOffset, pWin->backColor );
    SiiRegWrite( REG_OSD_W0_DISABLED_FG + regSetOffset, pWin->foreColorDisabled );
    SiiRegWrite( REG_OSD_W0_DISABLED_BG + regSetOffset, pWin->backColorDisabled );

    // Content index arrays
    indexStartWord = OSD_CONTENT_INDEX_1_OFFSET + pWin->contentIndexStartWord;
    SiiRegWrite( REG_OSD_W0_CI0_L + (winRef * 2), indexStartWord & 0x0FF );
    SiiRegWrite( REG_OSD_W0_CI0_H + (winRef * 2), (indexStartWord >> 8) & 0x00F );
    indexStartWord = OSD_CONTENT_INDEX_2_OFFSET + pWin->contentIndexStartWord;
    SiiRegWrite( REG_OSD_W0_CI1_L + (winRef * 2), indexStartWord & 0x0FF );
    SiiRegWrite( REG_OSD_W0_CI1_H + (winRef * 2), (indexStartWord >> 8) & 0x00F );

    SiiRegWrite( REG_OSD_W0_ALPHA + winRef, (pWin->alphaBlendLevel != 0) ? pWin->alphaBlendLevel | 0x80 : 0 );

    SiiRegWrite( REG_OSD_W0_ROW_CTRL + regSetOffset,
        ((pWin->borderWidth << 4) & MSK_BORDER_WIDTH)   |
        ((pWin->dividerWidth << 6) & MSK_DIVIDER_WIDTH) |
        (pWin->highlightRow & MSK_CURSOR_SELECTED) );

    // If a flash resource, send the OSDRAM index data directly from flash to OSDRAM
    if (( windowId != 0 ) && (( windowId & OSD_RESOURCE_LOCAL_FLAG ) == 0 ))
    {
        if ( !SiiDrvOsdLoadWindowResource ( winRef, windowId, false ))
        {
        	//DEBUG_PRINT(MSG_DBG,"\n\nUNABLE TO LOAD OSD RESOURCE WINDOW\n\n");
        	 winRef = -1;
            pWin->windowStatus = OSD_WSTAT_INVISIBLE|OSD_WSTAT_AVAILABLE ;
        }
    }
    // If a local resource, let the window system firmware know it must be written
    if ( windowId & OSD_RESOURCE_LOCAL_FLAG )
    {
        pWin->dirtyList[0].startByte    = 0;
        pWin->dirtyList[0].startWord    = 0;
        pWin->dirtyList[0].wordCount    = (pWin->rowCount * pWin->colCount) / OSDRAM_INDEXES_PER_WORD;
        pWin->dirtyList[0].dirtyBits    = 0x03;
    }

    return( winRef );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Close a window
//! @param[in]  winRef    - Value returned by SiiDrvOsdWindowCreate function
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdCloseWindow( int winRef )
{

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    if ( winRef != -1 )
    {
        SiiDrvOsdShowWindow( winRef, 0 );
        pDrvOsd->winData[ winRef].windowStatus  = OSD_WSTAT_AVAILABLE;
    }
}

