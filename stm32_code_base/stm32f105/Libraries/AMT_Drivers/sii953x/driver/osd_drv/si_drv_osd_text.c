//***************************************************************************
//!file     si_drv_osd_write_window.c
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
#include "si_drv_osd_internal.h"
#include "si_drv_internal.h"
#include "si_osd_resource.h"
#include "si_regs_osd.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Added passed OSDRAM range to dirty list so that it will be written to OSDRAM
//!             from the local buffer the next time OsdWriteBackgroundSurface() is called
//!             Up to 32 independent ranges are available, but if the passed range overlaps an
//!             existing dirty range, that range will be expanded as necessary to include the
//!             new range.
//! @param[in]  pWin            Pointer to window structure
//! @param[in]  startWordIndex  Starting OSDRAM word index.
//! @param[in]  osdWordCount    number of OSD words in range.
//! @param[in]  byteIndex       starting byte within starting OSD word.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdUpdateDirtyList( SiiDrvOsdWindow_t *pWin, int startWordIndex, int osdWordCount, int byteIndex )
{
    int     i, dirtyStart, dirtyEnd, newEnd;
    bool_t  rangeAdded = false;

    for ( i = 0; i < 32; i++ )
    {
        // Don't look at empty ranges for this check.
        if (( pWin->dirtyList[i].dirtyBits & 0x03 ) == 0)
        {
            continue;
        }

        // Make it easier to read.
        dirtyStart  = pWin->dirtyList[i].startWord;
        dirtyEnd    = dirtyStart + pWin->dirtyList[i].wordCount - 1;
        newEnd      = startWordIndex + osdWordCount - 1;

        // If passed range starts or ends in an existing dirty range, expand the range as needed
        // instead of creating a new range in the list.
        if ((( startWordIndex >= dirtyStart) && ( startWordIndex <= dirtyEnd )) ||
            (( newEnd >= dirtyStart)         && ( newEnd <= dirtyEnd )))
        {
            // New range starts lower than previous range.
            if ( startWordIndex < dirtyStart)
            {
                pWin->dirtyList[i].startWord    = startWordIndex;
                pWin->dirtyList[i].startByte    = byteIndex;

                // Increase the word count by the number of words below the original start index.
                pWin->dirtyList[i].wordCount += (dirtyStart - startWordIndex);
            }
            // New range starts at the same location as previous range.
            else if ( startWordIndex == dirtyStart)
            {
                // Same starting word, check to see if the new starting byte is lower.
                if ( byteIndex < pWin->dirtyList[i].startByte )
                {
                    pWin->dirtyList[i].startByte = byteIndex;
                }
            }
            // If the new range ends beyond the original end, extend the word count.
            if ( newEnd > dirtyEnd )
            {
                // Increase the word count by the number of words beyond the original end index.
                pWin->dirtyList[i].wordCount += (newEnd - dirtyEnd);
            }
            pWin->dirtyList[i].dirtyBits    = 0x03;     // Update dirty mark on both sides
            rangeAdded = true;
            break;
        }
    }

    // If added to an existing range above, we're done.  Otherwise, find and empty
    // slot and add it.
    if ( !rangeAdded )
    {
        for ( i = 0; i < 32; i++ )
        {
            // Find first available entry that is not dirty on either side.
            if (( pWin->dirtyList[i].dirtyBits & 0x03 ) == 0)
            {
                pWin->dirtyList[i].startWord    = startWordIndex;
                pWin->dirtyList[i].wordCount    = osdWordCount;
                pWin->dirtyList[i].startByte    = byteIndex;
                pWin->dirtyList[i].dirtyBits    = 0x03;
                break;
            }
        }
        if ( i == 32 )
        {
            DEBUG_PRINT( 0, "Overflowed dirty list!! (win: %d)\n", pWin->contentIndexStartWord );
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Clear the dirty flags for the specified window
//! @param[in]  pWin
//-------------------------------------------------------------------------------------------------
void SiiOsdDrvClearDirtyList( SiiDrvOsdWindow_t *pWin )
{
    int i;

    for ( i = 0; i < 32; i++ )
    {
        pWin->dirtyList[i].dirtyBits = 0x00;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns true if passed location is dirty
//! @param[in]  winRef      Value returned by SiiDrvOsdWindowCreate function
//! @param[in]  row         row within window of character (0-15)
//! @param[in]  col         column within row character (0-31)
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdIsDirtyLocation( SiiDrvOsdWindow_t *pWin, int wordIndex )
{
    int     i;
    bool_t  isDirty = false;

    for ( i = 0; i < 32; i++ )
    {
        // Only check if entry is dirty
        if (( pWin->dirtyList[i].dirtyBits & 0x03 ) != 0)
        {
            if (( wordIndex >= pWin->dirtyList[i].startWord) && (( wordIndex < pWin->dirtyList[i].startWord + pWin->dirtyList[i].wordCount)))
            {
                isDirty = true;
                break;
            }
        }
    }

    return( isDirty );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Get an OSDRAM word from OSDRAM or local content buffer for read-modify-write.
//! @note       This function is necessary because we don't pre-load the entire window contents
//! @note       from SPI Flash into the local content buffer when the window is created from a
//! @note       window resource.  Although doing that requires reading from OSDRAM when we need
//! @note       part of an OSDRAM word, it is faster than loading the entire window at creation time
//! @note       since we rarely modify only part of an OSDRAM word.
//! @param[in]  pWin
//! @param[in]  wordIndex
//! @param[in]  segmentStart
//! @param[out] pByteData       - pointer to output buffer.
//-------------------------------------------------------------------------------------------------
static bool_t OsdRamGetOneWord( SiiDrvOsdWindow_t *pWin, int wordIndex, int segmentStart, uint8_t *pByteData )
{
    bool_t success = true;

    // If dirty, get the word from the local buffer.
    if ( SiiDrvOsdIsDirtyLocation( pWin, wordIndex - pWin->contentIndexStartWord ))
    {
        memcpy( pByteData, &pWin->contentIndexWordData[ (wordIndex - pWin->contentIndexStartWord) * OSDRAM_BYTES_PER_WORD ], OSDRAM_BYTES_PER_WORD );
    }
    else
    {
        success = SiiDrvOsdRamI2cRead( wordIndex + segmentStart, 1, pByteData );
    }
    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Write a text string to the specified window row and column.
//!             Write character index values corresponding to each character in the
//!             passed array to the inactive content index buffer.  If showOptions OPT_SHOW
//!             flag is set, the inactive and active content index buffers are swapped.
//! @param[in]  pWin
//! @param[in]  row         row within window of string start (0-15)
//! @param[in]  col         column within row of string start (0-31)
//! @param[in]  pStr        Pointer to ASCII string, null terminated
//! @param[in]  options     OPT_RESOURCEID, OPT_ELLIPSIS, OPT_CLEAR_TO_END, OPT_CENTER_TEXT, OPT_WRAP_TEXT, OPT_SHOW
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdWriteTextRow( SiiDrvOsdWindow_t *pWin, int row, int col, char *pStr, int length, SiiOsdOptions_t options )
{
    bool_t      success = false;
    bool_t      isOffset;
    int         i, indexValue, x, spaceValue, startPartial, endPartial;
    int         startWordIndex, osdWordCount, byteIndex, charCount, startCharIndex;
    int         foreSurface, backSurface;
    uint16_t    segmentStart;
    uint8_t     indexByte;
    uint8_t     tBuffer [ 100];     // buffer for one window line.

    if ( pWin->windowStatus & OSD_WSTAT_AVAILABLE )
    {
        return( false );
    }
    spaceValue = 0;

    // Determine background surface.
    foreSurface = ((SiiRegRead( REG_OSD_INDEX_CTRL) >> 4) & 0x03) - 1;  // Treat the surface bits as an index
    backSurface = (foreSurface + 1) % 2;
    segmentStart = (backSurface == 0) ? OSD_CONTENT_INDEX_1_OFFSET : OSD_CONTENT_INDEX_2_OFFSET;

    if ( options & OPT_CLEAR_TO_END )
    {
        spaceValue  = SiiDrvOsdCbCharacterToIndex( ' ' );
        charCount   = (pWin->colCount - col);    // Fill out the entire row.
    }
    else
    {
        charCount   = (length < ((pWin->colCount - col))) ? length : (pWin->colCount - col);
    }

    // Get OSDRAM boundaries for text destination.  If we don't have to read any OSDRAM first, we won't
    startCharIndex  = ((row * pWin->colCount) + col);
    startWordIndex  = startCharIndex / OSDRAM_INDEXES_PER_WORD;     // from start of window...
    startPartial    = startCharIndex % OSDRAM_INDEXES_PER_WORD;
    osdWordCount    = (charCount + startPartial) / OSDRAM_INDEXES_PER_WORD;
    endPartial      = (charCount + startPartial) % OSDRAM_INDEXES_PER_WORD;
    osdWordCount    += endPartial ? 1 : 0;

    // Read the starting OSDRAM word if it is partial.
    if ( startPartial )
    {
        if ( !OsdRamGetOneWord( pWin, startWordIndex + pWin->contentIndexStartWord, segmentStart, tBuffer ))
        {
            return( false );
        }
    }
    // Read the last word of OSDRAM if it is partial
    if ( endPartial && !OsdRamGetOneWord( pWin, startWordIndex + pWin->contentIndexStartWord + (osdWordCount - 1), segmentStart, &tBuffer[(osdWordCount - 1) * 6] ))
    {
        return( false );
    }

    // Convert the passed text into character index values and merge into the index array from the OSDRAM.
    byteIndex   = ((startCharIndex % OSDRAM_INDEXES_PER_WORD) * 12) / 8;
    isOffset    = !(( startCharIndex * 12 ) % 8) == 0;
    i = 0;
    for ( x = col; x < pWin->colCount; x++ )
    {
        // Determine correct character to place at this location
        if ( i >= length )
        {
            if ( options & OPT_CLEAR_TO_END )
            {
                indexValue = spaceValue;
            }
            else
            {
                break;
            }
        }
        else
        {
            // Perform character substitution as required by position/value/options.
            indexValue = (options & OPT_NO_TRANSLATE) ? pStr[i++] : SiiDrvOsdCbCharacterToIndex( pStr[i++] );
            indexValue = (indexValue < pDrvOsd->lastFontCharacterIndex ) ? indexValue : SiiDrvOsdCbCharacterToIndex( SII_OSD_CHAR_ILLEGAL_CHAR );
            indexValue = (options & OPT_REMOVE_ICON) ? SiiDrvOsdCbCharacterToIndex(' ') : indexValue;
            if ( (x == (pWin->colCount - 1)) && ( options & OPT_ELLIPSIS ) && (( options & OPT_WRAP_TEXT ) == 0 ))
            {
                indexValue = SiiDrvOsdCbCharacterToIndex( SII_OSD_CHAR_ELLIPSIS );
            }
        }
        if ( isOffset )
        {
            indexByte = tBuffer[ byteIndex ];
            tBuffer[ byteIndex] = ( indexByte & 0xF0 ) | ((indexValue >> 8) & 0x0F);    // axx  --> xa xx
            tBuffer[ byteIndex + 1] = (indexValue & 0xFF);                              // xbc  --> xa bc
            byteIndex += 2;
        }
        else
        {
            tBuffer[ byteIndex] = (indexValue >> 4) & 0xFF;                             // abx  --> ab xx
            indexByte = (tBuffer[ byteIndex + 1] & ~0xF0);
            tBuffer[ byteIndex + 1] = (indexByte | ((indexValue << 4) & 0xF0));         // xxc  --> ab cx
            byteIndex++;
        }
        isOffset = !isOffset;
    }

    // Copy the modified buffer to the local content index word buffer and update the dirty list
    // so that the next time show window is called, we will update the OSDRAM.
    memcpy( &pWin->contentIndexWordData[startWordIndex * OSDRAM_BYTES_PER_WORD], tBuffer, osdWordCount * OSDRAM_BYTES_PER_WORD );
    SiiDrvOsdUpdateDirtyList( pWin, startWordIndex, osdWordCount, startWordIndex * OSDRAM_BYTES_PER_WORD );

    return( success );
}

