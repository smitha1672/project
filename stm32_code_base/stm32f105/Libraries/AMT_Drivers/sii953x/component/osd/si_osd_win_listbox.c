//***************************************************************************
//!file     si_osd_win_listbox.c
//!brief    Silicon Image OSD List Box window.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_debug.h"
#include "si_osd_internal.h"

#if INC_OSD_LISTBOX

#define ITEM_LENGTH pWin->param1
#define ITEM_MAX    pWin->param2
#define ITEM_COUNT  pWin->param3
#define TOP_ITEM    pWin->param4

//-------------------------------------------------------------------------------------------------
//! @brief      Validate specified window as a listbox.
//! @param[in]  winRef      - Return value from the SiiOsdCreateListBox function.
//! @return     true - winRef is a valid list box, false if not
//-------------------------------------------------------------------------------------------------
static bool_t OsdListBoxValidate( int winRef )
{
    SiiOsdWindow_t  *pWin;

    pOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
    while ( winRef != -1 )
    {
        pWin = &pOsd->winData[ winRef];

        // Ignore window if not enabled.
        if (( pWin->windowStatus & OSD_WSTAT_ENABLED ) == 0 )
        {
            pOsd->lastResultCode = SII_OSDDRV_ERR_NOT_ACTIVE;
            break;
        }
        // If not a list box, can't do this.
        if ( pWin->windowType != OSD_WINTYPE_LISTBOX )
        {
            pOsd->lastResultCode = SII_OSD_ERR_WINTYPE_MISMATCH;
            break;
        }
        // No list data info attached
        if ( pWin->pExternalData == NULL )
        {
            pOsd->lastResultCode = SII_OSD_ERR_NOT_AVAIL;
            break;
        }

        pOsd->lastResultCode = SII_OSD_SUCCESS;
        break;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Update the listbox display starting at the specified row.
//! @param[in]  winRef      - Return value from the SiiOsdCreateListBox function.
//! @param[in]  firstRow    First row to be re-displayed, or -1 if all rows to be re-displayed
//-------------------------------------------------------------------------------------------------
static void UpdateListBox( int winRef, int firstRow )
{
    int             row;
    SiiOsdWindow_t  *pWin = &pOsd->winData[ winRef];
    char            *pStr = (char*)pWin->pExternalData;

    // -1 is the same as 0 so that this function can be used directly by the DeleteItem function.
    if ( firstRow == -1 )
    {
        firstRow = 0;
    }
    for ( row = firstRow; row < pWin->rowCount; row++ )
    {
        if ( row < ( ITEM_COUNT - TOP_ITEM))
        {
            SiiDrvOsdWriteText( winRef, row, 0, &pStr[ (TOP_ITEM + row) * ITEM_LENGTH], OPT_CLEAR_TO_END | OPT_ELLIPSIS );
        }
        else
        {
            SiiDrvOsdClearTextRow( winRef, row, 0, 0 );
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Scroll list box up or down as requested
//! @param[in]  winRef      - Return value from the SiiOsdCreateListBox function.
//! @return     true - winRef is a valid list box, false if not
//-------------------------------------------------------------------------------------------------
static void ScrollListBox ( int winRef, int key )
{
    SiiOsdWindow_t  *pWin = &pOsd->winData[ winRef];

    if ( key == SIIKEY_SCROLL_UP)
    {
        // Room to scroll up?
        if (( ITEM_COUNT - TOP_ITEM ) > pWin->rowCount )
        {
            TOP_ITEM++; // Increment top item index
        }
    }
    else if ( key == SIIKEY_SCROLL_DOWN)
    {
        // Room to scroll down?
        if ( TOP_ITEM > 0 )
        {
            TOP_ITEM--; // Decrement top item index
        }
    }
    else
    {
        return;
    }

    UpdateListBox( winRef, 0 );
    SiiOsdShowWindow( winRef, OPT_SHOW );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the index of the list item beneath the current list box cursor.
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @return     Index of selected item or -1 if nothing selected or other error
//-------------------------------------------------------------------------------------------------
int SiiOsdListBoxGetSelect( int winRef )
{
    SiiOsdWindow_t  *pWin;

    if ( !OsdListBoxValidate( winRef ))
    {
        return( -1 );
    }
    pWin = &pOsd->winData[ winRef];

    return( pWin->highlightRow + TOP_ITEM );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize a window data structure
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @param[in]  itemIndex
//! @param[in]  options     OPT_SHOW: Scroll row into view if not currently in display area
//! @return     Pointer to valid SiiDrvOsdWindow_t window structure or NULL if no window is available.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdListBoxSetSelect( int winRef, int itemIndex, SiiOsdOptions_t options  )
{
    if ( !OsdListBoxValidate( winRef ))
    {
        return( false );
    }

    return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Add a list box item string
//! @param[in]  winRef      - Return value from the SiiOsdCreateListBox function.
//! @param[in]  pItemText   - ASCII character string.
//! @param[in]  index       - Position in list to add item.  -1 adds to the end of the list.
//! @param[in]  options     - OPT_INSERT, OPT_SHOW
//! @return     true if successful, false if failed.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdListBoxAddItem( int winRef, char *pItemText, int index, SiiOsdOptions_t options )
{
    int             row;
    SiiOsdWindow_t  *pWin;
    char            *pStr;

    if ( !OsdListBoxValidate( winRef ))
    {
        return( false );
    }
    pWin    = &pOsd->winData[ winRef];
    pStr    = (char*)pWin->pExternalData;

    // If an add, must have at least one slot available.
    // index == -1 (add to end)
    // option insert
    // index >= itemCount (treated like -1)
    index = ( index > ITEM_COUNT ) ? -1 : index;
    index = ( index < 0 ) ? ITEM_COUNT : index;

    // Update item count only if NOT overwriting an existing entry.
    if (( index == ITEM_COUNT) || ( options & OPT_INSERT ))
    {
        if (( ITEM_COUNT + 1 ) >= ITEM_MAX )
        {
            pOsd->lastResultCode = SII_OSD_ERR_NOT_AVAIL;
            return( false );
        }
        ITEM_COUNT++;
    }

    if ( options & OPT_INSERT )
    {
        // Move all strings at index and below down one.
        for ( row = ITEM_COUNT; row >= index; row-- )
        {
            memcpy( &pStr[ row * ITEM_LENGTH ], &pStr[ (row - 1) * ITEM_LENGTH ], ITEM_LENGTH );
        }
        ITEM_COUNT++;
    }

    // Now insert the string into the empty spot.
    strncpy( &pStr[ index * ITEM_LENGTH ], pItemText, ITEM_LENGTH - 1 );
    pStr[ (index * ITEM_LENGTH) + ITEM_LENGTH - 1 ] = 0;    // Make sure the item string is terminated

    if (( index - TOP_ITEM ) >= 0 )
    {
        UpdateListBox( winRef, index - TOP_ITEM ); // Update the background surface
    }

    // Display if requested
    if ( options & OPT_SHOW )
    {
        SiiOsdShowWindow( winRef, options );
    }
    return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Delete a list box item string
//! @param[in]  winRef      - Return value from the SiiOsdCreateListBox function.
//! @param[in]  index       - Position in list to add item.  -1 for the entire list.
//! @param[in]  options     - OPT_SHOW
//! @return     true if successful, false if failed.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdListBoxDeleteItem( int winRef, int index, SiiOsdOptions_t options )
{
    int row, itemLength;
    SiiOsdWindow_t  *pWin;
    char            *pStr;

    if ( !OsdListBoxValidate( winRef ))
    {
        return( false );
    }
    pWin    = &pOsd->winData[ winRef];
    pStr    = (char*)pWin->pExternalData;

    // Is item to be deleted in range?
    if ( index >= ITEM_COUNT )
    {
        pOsd->lastResultCode = SII_OSD_ERR_INVALID_PARAMETER;
        return( false );
    }

    itemLength = pWin->colCount + 1;        // Include the null terminator byte

    // Caller wants entire list deleted?
    if ( index == -1 )
    {
        ITEM_COUNT = 0;
    }
    else
    {
        // Move all strings below index up one.
        for ( row = index; row < (ITEM_COUNT - 1); row++ )
        {
            memcpy( &pStr[ row * ITEM_LENGTH ], &pStr[ (row + 1) * ITEM_LENGTH ], ITEM_LENGTH );
        }
        ITEM_COUNT--;
    }

    UpdateListBox( winRef, index ); // Update the background surface

    // Display if requested
    if ( options & OPT_SHOW )
    {
        SiiOsdShowWindow( winRef, options );
    }

    return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Copies the text of the item at the specified index to the destination string buffer.
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @param[in]  index       Index of item to be returned.
//! @param[in]  pDestStr    Pointer to destination string buffer.
//! @param[in]  length      Length of destination string buffer.
//! @return     true if successful, false if error occurred.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdListBoxGetItem( int winRef, int index, char *pDestStr, int length )
{
    SiiOsdWindow_t  *pWin;

    if ( OsdListBoxValidate( winRef ))
    {
        pWin = &pOsd->winData[ winRef];

        // Make sure it's a valid index and copy the data if so.
        if ( index < ITEM_COUNT)
        {
            char *pString = pWin->pExternalData;
            strncpy( pDestStr, &pString[ index * ITEM_LENGTH ], length );
        }
        else
        {
            pOsd->lastResultCode = SII_OSD_ERR_INVALID_PARAMETER;
        }
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Manage a list box window
//! @param[in]  winRef      Return value from the SiiOsdCreateSlider function.
//! @param[in]  key         SIIKEY_SELECT, SIIKEY_CANCEL, SIIKEY_CURSOR_xxx, SIIKEY_SCROLL_xxx.
//! @param[in]  options     none
//! @return     true if key was used, false if not.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdListBoxHandler( int winRef, int key, SiiOsdOptions_t options )
{
    int             prevRow, moveIncrement, newRow;
    bool_t          userFuncResult = false;
    bool_t          keyUsed = true;
    char            *pString;
    SiiOsdWindow_t  *pWin;

    if ( !OsdListBoxValidate( winRef ))
    {
        return( false );
    }
    pWin = &pOsd->winData[ winRef];

    if ( winRef == SiiOsdGetFocus())
    {
        prevRow     = SiiDrvOsdGetCursor( winRef );
        userFuncResult = SiiOsdBaseWindowHandler( winRef, key, options );
        switch ( key )
        {
            case SIIKEY_CURSOR_UP:
            case SIIKEY_CURSOR_DOWN:
                // Check to see if we need to scroll up or down
                moveIncrement = (key == SIIKEY_CURSOR_DOWN) ? 1 : -1;

                // Determine new row (with optional wrap)
                newRow = ( prevRow + moveIncrement );
                if ( newRow >= pWin->rowCount )
                {
                    // Scroll if more items in list below bottom row
                    if (( TOP_ITEM + pWin->rowCount ) < ITEM_COUNT )
                    {
                        ScrollListBox( winRef, SIIKEY_SCROLL_UP ); // Need to scroll up a line.
                    }
                }
                else if ( newRow < 0 )
                {
                    if ( TOP_ITEM > 0 )
                    {
                        ScrollListBox( winRef, SIIKEY_SCROLL_DOWN ); // Need to scroll down a line.
                    }
                }
                break;

            case SIIKEY_SCROLL_UP:
            case SIIKEY_SCROLL_DOWN:
                ScrollListBox( winRef, key );
                break;

            case SIIKEY_CURSOR_LEFT:
                // If the editMode window member is non-zero, that means the current row item text is longer
                // than the display area and has been shifted to the right, so shift it one character to the left.
                if ( pWin->editMode > 0 )
                {
                    pWin->editMode--;
                    pString = &((char *)pWin->pExternalData)[((prevRow + TOP_ITEM) * ITEM_LENGTH) + pWin->editMode ];
                    SiiDrvOsdWriteText( winRef, prevRow, 0, pString, OPT_SHOW | OPT_CLEAR_TO_END | OPT_ELLIPSIS );
                }
                break;
            case SIIKEY_CURSOR_RIGHT:
                // If the current row item text is longer than the display area, shift the text
                // one character to the right.
                pString = &((char *)pWin->pExternalData)[((prevRow + TOP_ITEM) * ITEM_LENGTH) + pWin->editMode ];
                if ( strlen( pString ) > (size_t)pWin->colCount )
                {
                    pWin->editMode++;
                    pString++;
                    SiiDrvOsdWriteText( winRef, prevRow, 0, pString, OPT_SHOW | OPT_CLEAR_TO_END | OPT_ELLIPSIS );
                }
                break;
            case SIIKEY_CANCEL:
            case SIIKEY_SELECT:
                // Handled completely by the SiiOsdBaseWindowHandler function
                break;
            default:
                keyUsed = false;
                break;
        }
    }

    return( keyUsed );
}

#endif // if INC_OSD_LISTBOX
