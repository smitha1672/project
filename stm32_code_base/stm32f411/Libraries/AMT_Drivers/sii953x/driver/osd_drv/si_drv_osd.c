//***************************************************************************
//!file     si_drv_osd.c
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
#include "si_regs_osd.h"
#if INC_SPI
#include "si_drv_spi.h"
#endif
#include "si_drv_switch.h"  // SWWAFP1609_1743

//------------------------------------------------------------------------------
//  OSD Driver Instance Data
//------------------------------------------------------------------------------

SiiOsdDrvInstanceData_t  drvOsdInstance[1];
SiiOsdDrvInstanceData_t *pDrvOsd = &drvOsdInstance[0];

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the result of the last call to an OSD component function.
//! @return     SiiDrvOsdResultCodes_t result code.
//! @retval     SII_OSDDRV_SUCCESS: Success
//! @retval     SII_OSDDRV_ERR_FAIL: General failure
//! @retval     others
//-------------------------------------------------------------------------------------------------
SiiDrvOsdResultCodes_t SiiDrvOsdGetLastResult( void )
{
    return( pDrvOsd->lastResultCode );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Make OSD resolution change to font size, horiz, and vertical position as necessary
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdResolutionChange( videoFormatId_t newVidMode, int newVres, bool_t newInterlace )
{
    int_t               winRef, vRes, newFontResourceId;
    bool_t              isInterlaced, success, test;

   // DEBUG_PRINT( MSG_DBG, "========= OSD Resolution Change Handler ============\n");
    //DEBUG_PRINT( MSG_DBG, "OLD - Vres: %4d  FontHeight: %d  OSD_EN: %d\n", pDrvOsd->outputVres, pDrvOsd->fontHeight, SiiRegRead( REG_OSD_WIN_CTRL ) & BIT_OSD_ENABLE );
    if ( newVidMode == SI_VIDEO_MODE_NON_STD )
    {
        vRes = newVres;
        isInterlaced = newInterlace;
    }
    else
    {
        SiiDrvSwitchVideoFmtParse( newVidMode, NULL, &vRes, &isInterlaced );
    }

    // Determine appropriate font resource
    test = false;
    switch (vRes)
    {
        case VIDEO_VRES_720:
            newFontResourceId   = pDrvOsd->fontResourceId16x24;
            break;
        case VIDEO_VRES_1080:
            newFontResourceId   = pDrvOsd->fontResourceId24x32;
            break;
        case VIDEO_VRES_480:
        case VIDEO_VRES_576:
        default:
            newFontResourceId   = pDrvOsd->fontResourceId12x16;
            break;
    }

    // If different than existing font, load it now.
    success = true;
    if (( pDrvOsd->fontResourceId != newFontResourceId ) || (isInterlaced != pDrvOsd->outputIsInterlace) || pDrvOsd->useDefaultFont )
    {
        pDrvOsd->useDefaultFont = false;  // This was set only for startup case when font cannot be determined but has not been loaded

        // Don't change resolution with OSD enabled - we want to use the
        // internal X clk for the SPI-OSD transfer.  OSD will be re-enabled as necessary externally to this function.
        if ( SiiRegRead( REG_OSD_WIN_CTRL ) & BIT_OSD_ENABLE )
        {
           // DEBUG_PRINT( MSG_DBG, "---------------------> RES_CHG EN <-(A)-------------- %d\n", false );
            DrvOsdDisableInternal();
        }
        //DEBUG_PRINT( MSG_DBG, "Window update...\n" );
        success = ( pDrvOsd->fontResourceId != newFontResourceId ) ? SiiDrvOsdLoadFontResource( newFontResourceId ) : true;
        if ( success )
        {
           // DEBUG_PRINT( MSG_DBG, "...Done\n" );
            pDrvOsd->outputIsInterlace  = isInterlaced;     // Update global
            pDrvOsd->outputVres         = vRes;             // Update global

            // Update the display position of enabled windows
            for ( winRef = 0; winRef < SII_OSD_WIN_COUNT; winRef++ )
            {
                if ( pDrvOsd->winData[winRef].windowStatus & OSD_WSTAT_ENABLED)
                {
                    // Update screen position
                    SiiDrvOsdMoveWindow( winRef, pDrvOsd->winData[winRef].upperLeftX, pDrvOsd->winData[winRef].upperLeftY, OPT_SHOW );
                }
            }
        }
        else
        {
          //  DEBUG_PRINT( MSG_DBG, "...NOT Done\n" );
        }
    }
   // DEBUG_PRINT( MSG_DBG, "NEW - Vres: %4d  FontHeight: %d\n", pDrvOsd->outputVres, pDrvOsd->fontHeight );
   // DEBUG_PRINT( MSG_DBG, "====================================================\n");

    return( success );
}

//-------------------------------------------------------------------------------------------------
//! @brief      OSD driver initialization
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdInitialize( void )
{
    SiiOsdResourceHeader_t      lHeader;
    bool_t  isInterlaced, success = false;
    int i, vRes, fontId12x16, fontId16x24, fontId24x32, resourceCount;
    SiiDrvOsdWindowResource_t   const *pResourceData;

    // Get the hardware into a more-or-less known state.
    SiiRegWrite( REG_INT_STATUS_OSD, 0xFF);
    SiiRegBitsSet( REG_OSD_ACCESS_CTRL, REG_BIT_OSD_FORCE_TO_IDLE, true );

    // Caller is expected to set the font resource IDs prior to calling this function,
    // so save their value before clearing instance data.
    fontId12x16     = pDrvOsd->fontResourceId12x16;
    fontId16x24     = pDrvOsd->fontResourceId16x24;
    fontId24x32     = pDrvOsd->fontResourceId24x32;
    pResourceData   = pDrvOsd->pLocalResourceData;
    resourceCount   = pDrvOsd->localResourceCount;
    memset( pDrvOsd, 0, sizeof( SiiOsdDrvInstanceData_t ));
    pDrvOsd->fontResourceId12x16    = fontId12x16;
    pDrvOsd->fontResourceId16x24    = fontId16x24;
    pDrvOsd->fontResourceId24x32    = fontId24x32;
    pDrvOsd->pLocalResourceData     = pResourceData;
    pDrvOsd->localResourceCount     = resourceCount;

    for ( i = 0; i < SII_OSD_WIN_COUNT; i++ )
    {
        pDrvOsd->winData[i].windowStatus = OSD_WSTAT_INVISIBLE | OSD_WSTAT_AVAILABLE;
    }

    // Get current selected port info to determine initial font size.
    if ( SiiDrvSwitchSelectedPortInfo( true, NULL, &vRes, &isInterlaced, NULL ))
    {
        pDrvOsd->outputIsInterlace = isInterlaced;
    }
    else
    {
        // Unable to determine resolution; use default font and flag the condition
        pDrvOsd->useDefaultFont = true;
    }
    SiiDrvSpiInitialize();  // Configure for SPI control.

    // First order of business is to get the resource map from the SPI flash
    // Unconditionally read the resource header at the beginning of the flash memory
    if ( SiiDrvOsdGetMapResource())
    {
        // OK, got a map resource, get the default font info.
        if ( !SiiDrvOsdGetResourceHeader ( pDrvOsd->defaultFontId, &lHeader ))
        {
            success = false;
        }
        else
        {
            // Got a resource map, now get the default font stuff.
            pDrvOsd->defaultFontSize        = (SiiOsdFontSize_t)lHeader.winType;
            pDrvOsd->defaultFontHeight      = l_fontHeights[ pDrvOsd->defaultFontSize ];
            pDrvOsd->defaultFontWidth       = l_fontWidths[ pDrvOsd->defaultFontSize ];
            pDrvOsd->defaultDisplayHeight   = l_fontVres[ pDrvOsd->defaultFontSize ];
            pDrvOsd->defaultDisplayWidth    = l_fontHres[ pDrvOsd->defaultFontSize ];

            if ( pDrvOsd->useDefaultFont )
            {
                vRes = l_fontVres[ pDrvOsd->defaultFontSize];
                pDrvOsd->outputIsInterlace = false;
            }

           // Now load the FONT for the current resolution into OSD RAM
            pDrvOsd->bitmapSegmentStart = OSD_BITMAP_OFFSET;
            success = SiiDrvOsdResolutionChange( SI_VIDEO_MODE_NON_STD, vRes, pDrvOsd->outputIsInterlace );
        }
    }
    return( success );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Test for pending interrupts and handle as necessary.
//!
//! @param[in]
//-------------------------------------------------------------------------------------------------
bool_t SiiDrvOsdHandleInterrupts( void )
{
    bool_t  isInterlaced, interruptHandled;
    uint8_t u8Data;
    int     vRes;

    interruptHandled    = false;
    if ( pDrvOsd->interruptPending )
    {
        // Determine new main port resolution stability state.
        u8Data = SiiRegRead( REG_MP_GCP_STATUS ) & BIT_MP_RES_STABLE;

        if ( u8Data == BIT_MP_RES_STABLE )
        {
            // If OSD was ENABLED or PRE_ENABLE, update the OSD resolution if necessary before
            // enabling the OSD display.
            if (( pDrvOsd->enableState == OSD_STATE_PRE_ENABLE ) || ( pDrvOsd->enableState == OSD_STATE_ENABLED ))
            {
                SiiDrvSwitchSelectedPortInfo( true, NULL, &vRes, &isInterlaced, NULL ); // Get current port info
                SiiDrvOsdResolutionChange( SI_VIDEO_MODE_NON_STD, vRes, isInterlaced );
                SiiDrvOsdEnable( true );
            }
        }
        else
        {
            DrvOsdDisableInternal();
        }

        interruptHandled            = true;
        pDrvOsd->interruptPending   = false;
    }
    return( interruptHandled );
}

//-------------------------------------------------------------------------------------------------
//! @brief  Initialize the font resources used for each font size.
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdSetFontResourceId ( SiiOsdFontSize_t fontSize, int resourceId )
{
    switch ( fontSize )
    {
        case VAL_OSD_FONT_12x16:
            pDrvOsd->fontResourceId12x16 = resourceId;
            break;
        case VAL_OSD_FONT_16x24:
            pDrvOsd->fontResourceId16x24 = resourceId;
            break;
        case VAL_OSD_FONT_24x32:
            pDrvOsd->fontResourceId24x32 = resourceId;
            break;
    }
    pDrvOsd->interruptPending = true;
}

//-------------------------------------------------------------------------------------------------
//! @brief  Set default font resource ID for specified font size
//-------------------------------------------------------------------------------------------------
void SiiDrvOsdProcessInterrupts ( void )
{
    pDrvOsd->interruptPending = true;
}



