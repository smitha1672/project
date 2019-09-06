//***************************************************************************
//!file     si_osd_win_menu.c
//!brief    Silicon Image OSD Menu window.
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

#if INC_OSD_MENU

#define IS_CHECKBOX     pWin->param1
#define IS_CHECKED      pWin->param2
#define HAS_ICON        pWin->param3

static char l_checkboxEmpty []      = { SII_OSD_CHAR_CHECKBOX_EMPTY };
static char l_checkboxChecked []    = { SII_OSD_CHAR_CHECKBOX_CHECKED };

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  winRef      - Return value from the SiiOsdCreateWindow function.
//! @param[in]  itemIndex   -
//! @param[in]  options     - OPT_SET_CHECK, OPT_SHOW
//! @return
//-------------------------------------------------------------------------------------------------
void SiiOsdMenuSetItemCheck( int winRef, int itemIndex, SiiOsdOptions_t options )
{
    uint16_t        itemBit;
    SiiOsdWindow_t  *pWin;

    // Ignore window if not enabled.
    if (( winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
    }

    else if ( itemIndex < OSD_WIN_ROW_COUNT )
    {
        pOsd->lastResultCode    = SII_OSD_ERR_INVALID_PARAMETER;
        pWin                    = &pOsd->winData[ winRef];

        itemBit = (1 << itemIndex);
        if ( IS_CHECKBOX & itemBit )
        {
            pOsd->lastResultCode    = SII_OSD_SUCCESS;
            if ( options & OPT_SET_CHECK )
            {
                IS_CHECKED      |= itemBit;
            }
            else
            {
                IS_CHECKED      &= ~itemBit;
            }
            SiiOsdWriteText(
                winRef, itemIndex, pWin->colCount - 1,
                (IS_CHECKED & itemBit) ? l_checkboxChecked : l_checkboxEmpty,
                (options & OPT_SHOW) );
        }
    }

}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  winRef      - Return value from the SiiOsdCreateWindow function.
//! @param[in]  itemIndex   -
//! @return
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdMenuGetItemCheck( int winRef, int itemIndex )
{
    bool_t          itemChecked = false;
    uint16_t        itemBit;
    SiiOsdWindow_t  *pWin;

    // Ignore window if not enabled.
    if (( winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
    }

    else if ( itemIndex < OSD_WIN_ROW_COUNT )
    {
        pOsd->lastResultCode    = SII_OSD_ERR_INVALID_PARAMETER;
        pWin                    = &pOsd->winData[ winRef];

        itemBit = (1 << itemIndex);
        if ( IS_CHECKBOX & itemBit )
        {
            pOsd->lastResultCode    = SII_OSD_SUCCESS;
            itemChecked = (( IS_CHECKED & itemBit) != 0);
        }
    }

    return( itemChecked );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  winRef      - Return value from the SiiOsdCreateWindow function.
//! @param[in]  itemIndex   -
//! @param[in]  options     - OPT_ADD_CHECK, OPT_SET_CHECK, OPT_SHOW
//! @return
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdMenuAddItemCheck( int winRef, int itemIndex, SiiOsdOptions_t options )
{
    uint16_t        itemBit;
    SiiOsdWindow_t  *pWin;

    // Ignore window if not enabled.
    if (( winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
    }

    else if ( itemIndex < pOsd->winData[ winRef].rowCount )
    {
        pOsd->lastResultCode    = SII_OSD_SUCCESS;
        pWin                    = &pOsd->winData[ winRef];

        itemBit = (1 << itemIndex);
        if ( options & OPT_ADD_CHECK )
        {
            IS_CHECKBOX     |= itemBit;
            IS_CHECKED      &= ~itemBit;
        }
        if (( options & OPT_SET_CHECK ) && ( IS_CHECKBOX & itemBit ))
        {
            IS_CHECKED      |= itemBit;
        }

        // Show or hide checkbox icon
        if ( IS_CHECKBOX & itemBit )
        {
            SiiOsdWriteText(
                winRef, itemIndex, pWin->colCount - 1,
                (IS_CHECKED & itemBit) ? l_checkboxChecked : l_checkboxEmpty,
                (options & OPT_SHOW) );
        }
        else
        {
            SiiOsdWriteText( winRef, itemIndex, pWin->colCount, " ", (options & OPT_SHOW) );
        }
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  winRef      - Return value from the SiiOsdCreateWindow function.
//! @param[in]  itemIndex   -
//! @param[in]  options     -
//! @return
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdMenuSetItemIcon( int winRef, int itemIndex, uint8_t *pIcon, SiiOsdOptions_t options )
{
    pOsd->lastResultCode = SII_OSD_ERR_NOT_VALID;
    if ( winRef != -1 )
    {
        pOsd->lastResultCode = SII_OSD_SUCCESS;
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]  winRef      - Return value from the SiiOsdCreateWindow function.
//! @param[in]  itemIndex   -
//! @param[in]  options     -
//! @return
//-------------------------------------------------------------------------------------------------
void SiiOsdMenuItemEnable( int winRef, int itemIndex, SiiOsdOptions_t options )
{
    pOsd->lastResultCode = SII_OSD_ERR_NOT_VALID;
    if ( winRef != -1 )
    {
        pOsd->lastResultCode = SII_OSD_SUCCESS;
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the index of the menu item beneath the menu cursor.
//! @param[in]  winRef      - Return value from the SiiOsdCreateWindow function.
//! @return
//-------------------------------------------------------------------------------------------------
int SiiOsdMenuGetSelect( int winRef )
{
    pOsd->lastResultCode = SII_OSD_ERR_NOT_VALID;
    if ( winRef != -1 )
    {
        pOsd->lastResultCode = SII_OSD_SUCCESS;
        return( pOsd->winData[ winRef].highlightRow );
    }

    return( 0 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the highlight (cursor) row for the specified menu.
//! @param[in]  winRef      - Return value from the SiiOsdCreateWindow function.
//! @param[in]  itemIndex   -
//! @return     true: success, false: requested row is disabled or out of range.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdMenuSetSelect( int winRef, int itemIndex )
{
    pOsd->lastResultCode = SII_OSD_ERR_NOT_VALID;
    if ( winRef != -1 )
    {
        pOsd->lastResultCode = SII_OSD_SUCCESS;
    }

    // If successful at component level, check the driver level.
    if ( pOsd->lastResultCode == SII_OSD_SUCCESS )
    {
        SiiOsdSetCursor( winRef, itemIndex, OPT_SHOW );
        pOsd->lastResultCode = (SiiDrvOsdGetLastResult() != 0) ? SII_OSD_ERR_FAIL : SII_OSD_SUCCESS;
    }

    return( pOsd->lastResultCode == SII_OSD_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Manage a Menu window
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @param[in]  key         SIIKEY_SELECT, SIIKEY_CANCEL, OSD_CURSOR_xxx.
//! @param[in]  options     none
//! @return     true if key was used, false if not.
//-------------------------------------------------------------------------------------------------
bool_t SiiOsdMenuHandler( int winRef, int key, SiiOsdOptions_t options )
{
    int             row, previousRow;
    bool_t          userFuncResult = false;
    bool_t          keyUsed = true;
    SiiOsdWindow_t  *pWin;

    // Ignore window if not enabled.
    if (( winRef == -1) || ( pOsd->winData[ winRef].windowStatus & OSD_WSTAT_ENABLED ) == 0 )
    {
        pOsd->lastResultCode = SII_OSD_ERR_NOT_ENABLED;
        return( false );
    }
    pOsd->lastResultCode    = SII_OSD_SUCCESS;
    pWin                    = &pOsd->winData[ winRef];

    if ( winRef == SiiOsdGetFocus())
    {
        previousRow = pWin->highlightRow;
        userFuncResult = SiiOsdBaseWindowHandler( winRef, key, options );
        row = SiiDrvOsdGetCursor( winRef );
        switch ( key )
        {
            case SIIKEY_CURSOR_UP:
            case SIIKEY_CURSOR_DOWN:
                if ( userFuncResult )
                {
                    // If a child window is currently displayed, remove it; there should not be any changes to save..
                    SiiOsdCloseWindow( pWin->childWinRef, OPT_CANCEL );
                    pWin->childWinRef = -1;

                    // Create and display a new child window if one exists for this row.
                    if ( pWin->rowChildWindowId[ row] != 0)
                    {
                        pWin->childWinRef = SiiOsdCreateWindow( pWin->rowChildWindowId[ row], 0, 0, 0, 0, NULL, OPT_SHOW );
                    }
                }
                break;
            case SIIKEY_CURSOR_LEFT:
            case SIIKEY_CURSOR_RIGHT:
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
#endif // if INC_OSD_MENU
