//***************************************************************************
//!file     si_drv_osd_window.c
//!brief    Silicon Image OSD Driver window control.
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
#include "si_regs_osd.h"
#include "si_drv_tpg.h"
#include "si_regs_tpg953x.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Set the row/col count size of the specified window.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  rowCount    Number of rows in window.
//! @param[in]  colCount    Number of columns in window
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdSetWindowSize( int winRef, int rowCount, int colCount )
{
    uint16_t    regOffset;
    uint8_t     mask;

    if ( winRef == -1 )
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
        return;
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    //Set row count for specified window
    regOffset = (uint16_t)REG_OSD_W0_W1_ROW_COUNT;
    regOffset = regOffset + ((winRef & 0x2) ? 1 : 0);
    mask = (winRef  & 0x1) ?  MSK_ROW_MASK_HI :  MSK_ROW_MASK_LO;
    rowCount = rowCount << ((winRef & 0x1) ? VAL_ROW_SHIFT_HI : VAL_ROW_SHIFT_LO );
    SiiRegModify( regOffset, mask, rowCount - 1 );

    // Set column count for specified window
    regOffset = (uint16_t)REG_OSD_W0_ROW_LEN + winRef;
    SiiRegWrite( regOffset, colCount - 1 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Translate the passed x position to the equivalent x position for the current font size.
//! @param[in]  virtX       - Default X position
//! @return     virtX translated to position for current font size.
//-------------------------------------------------------------------------------------------------
static int DrvOsdHorizPosition ( int virtX )
{
    if ( pDrvOsd->defaultFontWidth != pDrvOsd->fontWidth )
    {
        virtX   = virtX / pDrvOsd->defaultFontWidth;    // Reduce pixel position to character position
        virtX   = virtX * pDrvOsd->fontWidth;           // Convert back to pixel position with current font size.
    }

    return( virtX );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Translate the passed y position to the equivalent y position for the current font size.
//! @param[in]  virtY       - Default Y position
//! @return     virtY translated to position for current font size.
//-------------------------------------------------------------------------------------------------
static int DrvOsdVertPosition ( int virtY )
{
    if ( pDrvOsd->defaultFontHeight != pDrvOsd->fontHeight )
    {
        virtY   = virtY / pDrvOsd->defaultFontHeight;   // Reduce pixel position to character position
        virtY   = virtY * pDrvOsd->fontHeight;          // Convert back to pixel position with current font size.
    }
    if ( pDrvOsd->outputIsInterlace )
    {
        virtY   = virtY / 2;
    }

    return( virtY );
}

//------------------------------------------------------------------------------
// Function:    SiiDrvOsdSetHorizVert
//! @brief      Adjust the OSD Window vertically and horizontally.
// Parameters:  wId - Window Id 0, 1, 2 or 3
//				horzVal - adjust OSD Window horizontally
//              vertVal - adjust OSD window vertically
// Returns:   	horzVal - return horizontal position of a window
//              vertVal - return vertical position  of a window
//------------------------------------------------------------------------------
void SiiDrvOsdSetHorizVert ( int winRef, uint16_t horizVal, uint16_t vertVal)
{
    SiiDrvOsdWindow_t   *pDrvWin;
	uint16_t            regSetOffset, regVal;

    if ( winRef == -1 )
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
        return;
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    pDrvWin = &pDrvOsd->winData[ winRef];

    pDrvWin->upperLeftX = horizVal;
    pDrvWin->upperLeftY = vertVal;

    regSetOffset = (winRef * OSD_REG_BLOCK_OFFSET);

    // Horizontal position
    regVal    = DrvOsdHorizPosition( horizVal );        // Scale position
    SiiRegWrite( REG_OSD_W0_X_L + regSetOffset, regVal & 0xFF );
    SiiRegWrite( REG_OSD_W0_X_H + regSetOffset, (regVal >> 8) & 0x0F );

    // Vertical position
    regVal  = DrvOsdVertPosition( vertVal );                        // Scale position
    SiiRegWrite( REG_OSD_W0_Y_L + regSetOffset, regVal & 0xFF );
    SiiRegWrite( REG_OSD_W0_Y_H + regSetOffset, (regVal >> 8) & 0x0F );
}

//------------------------------------------------------------------------------
// Function: SiiDrvOsdGetHorizVert   
// Description: return the current vertical and horizontal position of a window
// Parameters:  wId - Window Id 0, 1, 2 or 3
//				horzVal - adjust OSD Window horizontally
//              vertVal - adjust OSD Window vertically
// Return: none
//------------------------------------------------------------------------------
void SiiDrvOsdGetHorizVert( int winRef, uint16_t *pHorz, uint16_t *pVert)
{
	uint16_t regVal;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

	regVal = REG_OSD_W0_X_L + winRef * OSD_WINDOW_ID_OFFSET_HORZ;
	
	*pHorz = SiiRegRead(regVal);
	*pHorz = *pHorz | (SiiRegRead(regVal + 1) << 8);

	regVal = REG_OSD_W0_Y_L + winRef * OSD_WINDOW_ID_OFFSET_VERT;
	*pVert = SiiRegRead(regVal);
	*pVert = *pVert | (SiiRegRead(regVal + 1) << 8);
}

//------------------------------------------------------------------------------
// Function: SiiDrvOsdSetBlending   
// Description:  set the alpha blending for the osd window
// Parameters:  SiiOsdPatch_t structure
//			   wId -- Window id of the chars that going to be patched/
//			   value -- value to be blending to the window (0-15) 
//			   ctrl -- enable or disable the alpha blending.
// Return: none
//------------------------------------------------------------------------------
void SiiDrvOsdSetBlending( int winRef, uint8_t value, uint8_t ctrl)
{
	uint16_t regVal;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
	
	if (ctrl)
	{
		//Enable Alpha Blending
		value |= BIT_OSD_WIN_ALPHA_EN;
	}
	else
	{
		//Disable Alpha Blending
		value &= ~BIT_OSD_WIN_ALPHA_EN;
	}
	regVal = REG_OSD_W0_ALPHA + winRef;
	SiiRegWrite(regVal, value);
}

//------------------------------------------------------------------------------
// Function: SiiDrvOsdGetBlending
// Description:  Get the alpha blending for the osd window
// Parameters:  SiiOsdPatch_t structure
//			   wId -- Window id of the chars that going to be patched/
//			   value -- value of the current blending to the window (0-15)
// Return: none
//------------------------------------------------------------------------------
void SiiDrvOsdGetBlending( int winRef, uint8_t *value)
{
	uint16_t regVal;

	pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

	regVal = REG_OSD_W0_ALPHA + winRef;
	*value = SiiRegRead(regVal) & MSK_W0_ALPHA_BLENDING;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the color value for the specified window element
//! @param[in]  winRef      Return value from SiiDrvOsdCreateWindow function.
//! @param[in]  color       window color element to modify.
//! @param[in]  colorVal    new window color val (use OSD_RGB() macro).
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdSetColor( int winRef, SiiOsdColorElement_t element, uint8_t colorVal )
{
    SiiDrvOsdWindow_t   *pDrvWin;
    uint16_t elementReg = 0;
    if ( winRef == -1 )
    {
        pDrvOsd->lastResultCode = SII_OSDDRV_ERR_INVALID_PARAMETER;
        return;
    }
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    pDrvWin = &pDrvOsd->winData[ winRef];
	switch (element)
	{
        case OSD_FG:
            elementReg = REG_OSD_W0_UNSEL_FG;
            pDrvWin->foreColor = colorVal;
            break;
        case OSD_BG:
            elementReg = REG_OSD_W0_UNSEL_BG;
            pDrvWin->backColor = colorVal;
            break;
		case OSD_FG_HIGHLIGHT:
		    elementReg = REG_OSD_W0_SELECTED_FG;
		    pDrvWin->foreColorHighlight = colorVal;
			break;
		case OSD_BG_HIGHLIGHT:
		    elementReg = REG_OSD_W0_SELECTED_BG;
		    pDrvWin->backColorHighlight = colorVal;
			break;
		case OSD_FG_DISABLED:
		    elementReg = REG_OSD_W0_DISABLED_FG;
		    pDrvWin->foreColorDisabled = colorVal;
			break;
		case OSD_BG_DISABLED:
		    elementReg = REG_OSD_W0_DISABLED_BG;
		    pDrvWin->backColorDisabled = colorVal;
			break;
        case OSD_BORDER:
            elementReg = REG_OSD_W0_BORDER;
            pDrvWin->borderColor = colorVal;
            break;
        case OSD_ROW_DIVIDER:
            elementReg = REG_OSD_W0_DIVIDER;
            pDrvWin->dividerColor = colorVal;
            break;
        default:
            break;
	}

	// Offset to correct color register
	elementReg += ( winRef * OSD_REG_BLOCK_OFFSET);
	SiiRegWrite( elementReg, colorVal );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Get the color value for the specified window element
//! @param[in]  winRef      Return value from SiiDrvOsdCreateWindow function.
//! @param[in]  element     window color element
//! @return     Window color value.
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdGetColor( int winRef, SiiOsdColorElement_t element )
{
	uint16_t elementReg = 0;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

	switch ( element )
	{
        case OSD_FG:
            elementReg = REG_OSD_W0_UNSEL_FG;
            break;
        case OSD_BG:
            elementReg = REG_OSD_W0_UNSEL_BG;
            break;
        case OSD_FG_HIGHLIGHT:
            elementReg = REG_OSD_W0_SELECTED_FG;
            break;
        case OSD_BG_HIGHLIGHT:
            elementReg = REG_OSD_W0_SELECTED_BG;
            break;
        case OSD_FG_DISABLED:
            elementReg = REG_OSD_W0_DISABLED_FG;
            break;
        case OSD_BG_DISABLED:
            elementReg = REG_OSD_W0_DISABLED_BG;
            break;
        case OSD_BORDER:
            elementReg = REG_OSD_W0_BORDER;
            break;
        case OSD_ROW_DIVIDER:
            elementReg = REG_OSD_W0_DIVIDER;
            break;
        default:
            break;
	}

	// Offset to correct register
	elementReg += ( winRef * OSD_REG_BLOCK_OFFSET );
	return( SiiRegRead( elementReg ));
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the highlight row index of the specified window.
//! @param[in]  winRef      Return value from the SiiDrvOsdWindowCreate function.
//! @retval     Highlight row index
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdGetCursor ( int winRef )
{
	uint16_t regAddr;
	uint8_t cursor;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

	regAddr = REG_OSD_W0_ROW_CTRL + winRef * OSD_REG_BLOCK_OFFSET;
	cursor = SiiRegRead( regAddr ) & MSK_CURSOR_SELECTED;
	return( (int)cursor );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns true if the passed row is enabled
//! @param[in]  winIndex    - index of target window.
//! @param[in]  row         - window row (0-15)
//! @retval     true if enabled
//! @retval     false if disabled
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdRowIsEnabled( int winRef, int row )
{
    uint16_t rowReg, disabledRowMap;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    rowReg = REG_OSD_W0_DISABLED_ROW_L + (winRef  * OSD_REG_BLOCK_OFFSET);
    disabledRowMap = SiiRegRead( rowReg );
    disabledRowMap |= (SiiRegRead( rowReg + 1) << 8);
    return( (disabledRowMap & (1 << (row))) == 0 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the enable state of the specified row
//!             Change takes effect immediately.
//! @param[in]  winRef      Return value from any SiiOsdCreate function.
//! @param[in]  row         Window row to mark enabled/disabled
//! @param[in]  isEnable    true == enable, false == disable.
//! @retval                 true if successful, false if a failure.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdRowEnable ( int winRef, int row, bool_t isEnable )
{
	uint16_t rowReg;
	uint16_t disabledRowBit;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

	rowReg = REG_OSD_W0_DISABLED_ROW_L + (winRef * OSD_REG_BLOCK_OFFSET);
	disabledRowBit = 1 << row;
	SiiRegModify( rowReg , disabledRowBit & 0xFF, isEnable ? CLEAR_BITS : SET_BITS);
	SiiRegModify( rowReg + 1, (disabledRowBit >> 8) & 0xFF, isEnable ? CLEAR_BITS : SET_BITS);

    if ( isEnable )
    {
        pDrvOsd->winData[ winRef].disabledRowMap &= ~(1 << row );
    }
    else
    {
        pDrvOsd->winData[ winRef].disabledRowMap |= (1 << row );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set the Z-axis position of the passed window to the specified level, with 0 being
//!             the top-most level.
//! @note       For the SiI9535, only the top-most level can be specified for any one window.
//!             The remaining windows follow in descending levels.
//! @param[in]  winRef      Return value from the SiiOsdCreateListBox function.
//! @param[in]  level       Z-axis position for window.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdSetWindowZaxis( int winRef, int level )
{
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
    if ( winRef != -1 )
    {
        // Set the active (top) window and the active window enable bit
        SiiRegWrite( REG_OSD_SET_ACTIVE_WIN, winRef );
        SiiRegBitsSet( REG_OSD_INDEX_CTRL, REG_BIT_ACTIVE_WND, true );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable/disable an individual OSD window for display.  To actually display any windows,
//!             the OSD window display system must also be enabled (SiiDrvOsdEnable)
//! @param[in]  enableIt    - true: Enable window system  false: disable window system.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdWndEnable( int winRef, bool_t isEnable )
{
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    SiiRegModify( REG_OSD_WIN_CTRL, (1 << winRef) << BIT_OSD_ENABLE_WINDOW_SHIFT, isEnable ? SET_BITS : CLEAR_BITS);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable/disable an individual OSD window for display.  To actually display any windows,
//!             the OSD window display system must also be enabled (SiiDrvOsdEnable)
//! @param[in]  enableIt    - true: Enable window system  false: disable window system.
//-------------------------------------------------------------------------------------------------
//void SiiDrvOsdWndEnable( int winRef, bool_t isEnable )
//{
//    uint8_t     winCtrl;
//
//    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
//
//    winCtrl = SiiRegRead( REG_OSD_WIN_CTRL );
//    winCtrl &= ~((1 << winRef) << BIT_OSD_ENABLE_WINDOW_SHIFT);
//    winCtrl |= (isEnable ? ((1 << winRef) << BIT_OSD_ENABLE_WINDOW_SHIFT) : 0);
//    SiiRegWrite( REG_OSD_WIN_CTRL, winCtrl );
//
//    // If no windows are displayed, disable OSD completely
//    if (( winCtrl & ( BIT_OSD_ENABLE_WINDOW_0  | BIT_OSD_ENABLE_WINDOW_1 | BIT_OSD_ENABLE_WINDOW_2 | BIT_OSD_ENABLE_WINDOW_3)) == 0)
//    {
//        SiiDrvOsdEnable( false );
//    }
//}

//-------------------------------------------------------------------------------------------------
//! @brief      Disable the OSD window display.
//-------------------------------------------------------------------------------------------------
static void OsdDisable ( void )
{
    uint8_t srcCtrl, vpgCtrl;

//    DEBUG_PRINT( MSG_DBG, "OSD: Forced Disable\n");
    // Change TCLK source to internal XCLK.
    vpgCtrl = SiiRegRead( REG_VPG_CTRL_3 );
    SiiRegModify( REG_VPG_CTRL_3, MSK_VPG_FORMAT | MSK_VPG_CLK_SEL, (SI_TPG_CLK_XCLK << 1) | (SI_TPG_FMT_VID_480_60 << VAL_VPG_FORMAT_SHIFT));
    srcCtrl = SiiRegRead( REG_TX_VIDEO_SRC );
    SiiRegModify(
        REG_TX_VIDEO_SRC,
        BIT_TX0_PIPE_EN | (VAL_MUX_SEL_MASK << VAL_MUX_SEL_TX0_SHIFT),
        BIT_TX0_PIPE_EN | (VAL_MUX_SEL_INTERNAL << VAL_MUX_SEL_TX0_SHIFT));

    // Now we are guaranteed that the OSD_EN bit will propagate through to the OSDRAM clock source mux by the end of the next frame.
    SiiRegBitsSet( REG_OSD_WIN_CTRL, BIT_OSD_ENABLE, false );
    SiiOsTimerWait( MAX_VERT_PERIOD_MS );

    // Reset OSD engine to bring back to known state
    SiiRegBitsSet( REG_OSD_INDEX_CTRL, REG_BIT_OSD_RESET, true );
    SiiRegBitsSet( REG_OSD_INDEX_CTRL, REG_BIT_OSD_RESET, false );

    // Restore the original TCLK source.
    SiiRegWrite( REG_TX_VIDEO_SRC, srcCtrl );
    SiiRegWrite( REG_VPG_CTRL_3, vpgCtrl );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable the OSD window display.  To actually display any windows, the appropriate
//!             window(s) must also be enabled.
//! @param[in]  enableIt    - true: Enable window system  false: disable window system.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdEnable( bool_t enableIt )
{
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

//    DEBUG_PRINT( MSG_DBG, "OSD: %s\n", enableIt ? "ENABLE" : "DISABLE" );

    // If a valid TCLK is present, just do the operation directly.
    if (( SiiRegRead( REG_MP_GCP_STATUS ) & BIT_MP_RES_STABLE ) ||                          // TMDS Clock source is stable
       (( SiiRegRead( REG_VPG_CTRL_3 ) & MSK_VPG_CLK_SEL) == (SI_TPG_CLK_XCLK << 1)) )      // TPG Clock source is present
    {
//        DEBUG_PRINT( MSG_DBG, "OSD: Valid clock\n");
        pDrvOsd->enableState = enableIt ? OSD_STATE_ENABLED : OSD_STATE_DISABLED;
        SiiRegBitsSet( REG_OSD_WIN_CTRL, BIT_OSD_ENABLE, enableIt );
        SiiOsTimerWait( MAX_VERT_PERIOD_MS );
    }
    else
    {
//        DEBUG_PRINT( MSG_DBG, "OSD: No TCLK\n");
        // We have no TCLK.  This should mean we have no display, so it is OK to move to
        // the Internal Video source to provide TCLK.
        if ( enableIt )
        {
            // If enabling, we can only PRE_ENABLE and leave the hardware alone (disabled)
            pDrvOsd->enableState = OSD_STATE_PRE_ENABLE;
        }
        else
        {
            pDrvOsd->enableState = OSD_STATE_DISABLED;
            OsdDisable();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      If OSD state is PRE_ENABLE, attempt to fully enable it.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdStateUpdate ( void )
{
//    DEBUG_PRINT( MSG_DBG, "OSD: State Update\n");
    if (( pDrvOsd->enableState == OSD_STATE_PRE_ENABLE ) || ( pDrvOsd->enableState == OSD_STATE_ENABLED ))
    {
        SiiDrvOsdEnable( true );
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Disable the OSD window display, but mark it as PRE_ENABLE if it is currently in
//!             the ENABLED or PRE_ENABLED state.  The resolution stable interrupt handler uses
//!             this info to determine if the OSD should be re-enabled.
//-------------------------------------------------------------------------------------------------
void DrvOsdDisableInternal ( void )
{
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;
//    DEBUG_PRINT( MSG_DBG, "OSD: Disable Internal\n");

    // If currently enabled, set state to PRE_ENABLE and disable the hardware
    if (( pDrvOsd->enableState == OSD_STATE_ENABLED ) && ( SiiRegRead( REG_OSD_WIN_CTRL ) & BIT_OSD_ENABLE ))
    {
        pDrvOsd->enableState = OSD_STATE_PRE_ENABLE;
        OsdDisable();
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Patch the character at the passed window, row, and col with the passed character.
//! @param[in]  winIndex - Hardware window containing character to be patched (0-3).
//! @param[in]  isEnable - Enable or disable the match
//! @param[in]  row      - Window row of character to be patched (0-15).
//! @param[in]  col      - Window column of character to be patched (0-31).
//! @param[in]  newChar  - ASCII index of character to replace existing character.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdSetCharPatch( int winIndex, bool_t isEnable, int row, int col, int newChar, int patchIndex )
{
	uint16_t    offset, patchCharIndex;

    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

	offset = patchIndex * REG_INDEX_PATCH_OFFSET;
//    patchCharIndex = OsdCharToIndex(VAL_OSD_FONT_12x16, newChar );
    patchCharIndex = newChar;

	if ( isEnable )
	{
        //Set the row/col and index char to be patched
        SiiRegWrite( REG_OSD_C0_PATCH_IDX_L + offset, patchCharIndex & 0xFF );
        SiiRegWrite( REG_OSD_C0_PATCH_COL + offset, ((patchCharIndex >> 8) & 0x01) | (col << REG_INDEX_PATCH_COL_SHIFT) );
        SiiRegWrite( REG_OSD_C0_PATCH_ROW + offset, row | (winIndex << REG_INDEX_PATCH_WND_SHIFT));
	}

	// Enable the patch char and global patch control.
	SiiRegModify( REG_OSD_CHAR_PATCH_EN, 1 << patchIndex, isEnable ? SET_BITS : CLEAR_BITS );
    SiiRegModify(REG_OSD_ACCESS_CTRL, REG_BIT_INDEX_PATCH_EN, REG_BIT_INDEX_PATCH_EN);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Clear all character patches.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdClearCharPatch( int patchIndex )
{
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    SiiRegModify( REG_OSD_CHAR_PATCH_EN, 1 << patchIndex, CLEAR_BITS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Clear all character patches.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdClearAllCharPatch( void )
{
    pDrvOsd->lastResultCode = SII_OSDDRV_SUCCESS;

    //global register patch enable/disable
    SiiRegModify(REG_OSD_ACCESS_CTRL, REG_BIT_INDEX_PATCH_EN, CLEAR_BITS );
    //clear all patch
    SiiRegWrite( REG_OSD_CHAR_PATCH_EN, CLEAR_BITS);
}


