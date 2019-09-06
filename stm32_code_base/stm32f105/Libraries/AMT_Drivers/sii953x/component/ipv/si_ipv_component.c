//***************************************************************************
//!file     si_ipv_component.c
//!brief    Silicon Image InstaPrevue Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "string.h"
#include "si_common.h"
#include "si_ipv_internal.h"
#include "si_drv_ipv_internal.h"

IpvInstanceData_t ipvInstance[SII_NUM_IPV];
IpvInstanceData_t *pIpv = &ipvInstance[0];

//-------------------------------------------------------------------------------------------------
//! @brief      Initialize the IPV component.
//!
//! @return     SiiResultCodes_t
//-------------------------------------------------------------------------------------------------
bool_t SiiIpvInitialize ( void )
{
    int_t i;

    // Initialize the internal data structures
    memset( pIpv, 0, sizeof( IpvInstanceData_t ));

    pIpv->currentTheme          = SiiIpvLEFT_MIDDLE;    // Set default theme
    pIpv->lastTheme             = SiiIpvThemeCount;     // Force default theme to be loaded
    pIpv->alphaSteppingEnabled  = true;

    pIpv->allowBounce           = true;
    pIpv->bounceStep            = SII_IPV_BOUNCE_AMPL;

    for(i = 0; i < IPV_WINDOW_COUNT; i++)
    {
        siiIpvAlphaMax[i] = 0;
        siiIpvAlphaStep[i] = 4;
    }

    return( SiiDrvIpvInitialize());
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the result of the last call to a IPV component function.
//!
//! @return     true - success  false - some failure occurred
//-------------------------------------------------------------------------------------------------
SiiIpvResultCodes_t SiiIpvGetLastResult( void )
{
    return( pIpv->lastResultCode );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Select the basic operating mode for InstaPrevue.
//!
//! @param[in]  mode -  SiiIpvModeALL:      Always display all 5 ports
//!                     SiiIpvModeACTIVE:   Display only active ports
//!                     SiiIpvModeSELECT:   Display only RP selected port (PIP mode)
//
//! @comment    IPV must be disabled when this function is called.
//-------------------------------------------------------------------------------------------------
void SiiIpvModeSet ( SiiIpvMode_t mode )
{
    // For now, everything is done in the driver.
    SiiDrvIpvModeSet( mode );
    pIpv->lastResultCode = (SiiIpvResultCodes_t)SiiDrvIpvGetLastResult();
}

//-------------------------------------------------------------------------------------------------
//! @brief      Start or stop the IPV engine and display.
//!
//! @param[in]  enable
//-------------------------------------------------------------------------------------------------
bool_t SiiIpvEnable ( bool_t isEnable )
{
    if (( isEnable == true ) && pIpv->alphaSteppingEnabled )
    {
        pIpv->isAlphaStepping = true;
        pIpv->alphaStepRemainingDelay = pIpv->windowSpecs.alphaStepSpeedMs;
        memset( &pIpv->alphaCurrentValue[0], 31, sizeof( pIpv->alphaCurrentValue));
        SiiDrvIpvWindowAlphaSet( -1, 31 );
    }
    SiiDrvIpvEnable( isEnable );
    pIpv->lastResultCode = (SiiIpvResultCodes_t)SiiDrvIpvGetLastResult();
    if ( pIpv->lastResultCode == SII_IPV_SUCCESS )
    {
        pIpv->isEnabled = isEnable;
    }

    return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Refreshes IPV windows when AVI data changes without affecting input video clock.
//-------------------------------------------------------------------------------------------------
void SiiIpvRefresh(void)
{
    if (pIpv->isEnabled)
    {
        SiiDrvIpvDownsamplerReset();
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Move highlight to requested InstaPrevue window.  If not active, increment until an
//!             active input is found.  Returns the actual highlighted window index.
//! @param[in]  highlightIndex - index of a window to highlight,
//! @param[in]  isForward      - direction of a search for an active window.
//-------------------------------------------------------------------------------------------------
int_t SiiIpvHighlightWindow ( int_t highlightIndex, bool_t isForward )
{
    int_t   i;
    uint_t  activeMask;

    // Get active window mask.  If none are active, we'll end up leaving
    // without a highlighted window
    activeMask = SiiDrvIpvActiveWindowsGet();
//    if ( activeMask == 0 )
//    {
//        highlightIndex = -1;
//    }

    // Only look for a highlighted window if asked (highlightIndex != -1).
    if ( highlightIndex != -1 )
    {
        if (( activeMask & ( 1 << highlightIndex)) == 0 )
        {
            for ( i = 0; i < IPV_WINDOW_COUNT; i++ )
            {
                if ( activeMask & ( 1 << highlightIndex))
                {
                    break;
                }

                if(isForward)
                {
                    highlightIndex++;
                    if ( highlightIndex >= IPV_WINDOW_COUNT )
                    {
                        highlightIndex = 0;
                    }
                }
                else
                {
                    if ( highlightIndex == 0 )
                    {
                        highlightIndex = IPV_WINDOW_COUNT;
                    }
                    else
                    {
                        highlightIndex--;
                    }
                }
            }
        }
    }
    SiiIpvHighlightSet( highlightIndex );
    return( highlightIndex );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the port number of the highlighted window (not
//!             necessarily corresponding to the prevue window index).
//! @param[in]
//-------------------------------------------------------------------------------------------------
int_t SiiIpvHighlightPortGet ( int_t currentPortIndex, int_t highlightIndex )
{
    // If nothing is highlighted, don't change port selection
    if(highlightIndex == -1)
    {
        return currentPortIndex;
    }

    // The selected port is skipped in the prevue window index.
    if ( highlightIndex >= currentPortIndex )
    {
        // The highlight actually is pointing to the next highest port.
        highlightIndex++;
    }

    return( highlightIndex );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Determine the X/Y Coordinates and sizes of the preview windows based on the
//!             current resolution.
//-------------------------------------------------------------------------------------------------
static bool_t UpdateWindowSpecs ( void )
{
    SiiIpvSizeSet_t     sizeIndex;
    SiiWinCoords_t      mpResolution, upperLeft, increment = {0,0};
    int_t               i, totalWidth, totalHeight;
    bool_t              isInterlaced;
#if DETECT_ASPECT_RATIO
    bool_t 				pictureAspectRatio;
#endif
    pIpv->lastResultCode = SII_IPV_SUCCESS;
    if ( !SiiIpvCbVideoInfoGet( &mpResolution.x, &mpResolution.y, &isInterlaced))
    {
        DEBUG_PRINT( MSG_ERR, "IPV could not obtain res info for selected port\n");
        mpResolution.x = 640;
        mpResolution.y = 480;
    }

    // Cap resolution @ 1920x1080
    if (( mpResolution.x > 1920  ) && ( mpResolution.y > 1080  ))
    {
        DEBUG_PRINT( MSG_ERR, "IPV Detected > 1920x1080 res: %dx%d, forcing to 1920x1080\n", mpResolution.x, mpResolution.y );
        mpResolution.x = 1920;
        mpResolution.y = 1080;
    }

    // If nothing's changed, get out now.
    if (( pIpv->currentResolution.x == mpResolution.x   ) &&
        ( pIpv->currentResolution.y == mpResolution.y   ) &&
        ( pIpv->currentIsInterlaced == isInterlaced     ) &&
        ( pIpv->currentTheme        == pIpv->lastTheme ))
    {
        return( false );    // No need to change
    }
    pIpv->currentResolution.x   = mpResolution.x;
    pIpv->currentResolution.y   = mpResolution.y;
    pIpv->currentIsInterlaced   = isInterlaced;
    pIpv->lastTheme             = pIpv->currentTheme;

    DEBUG_PRINT( MSG_DBG, "Windows placed for %dx%d%c resolution\n", mpResolution.x, mpResolution.y, isInterlaced ? 'i' : 'p' );

    // Convert the current main window resolution into an index
    for ( sizeIndex = 0; sizeIndex < SiiIpvSizeCount; sizeIndex++ )
    {
        if ( mpResolution.x <= siiMainWindowSizes[ sizeIndex].x )
        {
            break;
        }
    }

    // Protection
    if (sizeIndex >= SiiIpvSizeCount)
    {
        sizeIndex = SiiIpvSize1920x1080;
    }

    // Pre-calculate some info using the widest aspect ratio
    totalWidth =    ((IPV_WINDOW_COUNT      * siiIpvWinSizes[ sizeIndex].width16x9) +
                    ((IPV_WINDOW_COUNT - 1) * siiIpvWinSizes[ sizeIndex].horizSpacer));
    totalHeight =   ((IPV_WINDOW_COUNT      * siiIpvWinSizes[ sizeIndex].height) +
                    ((IPV_WINDOW_COUNT - 1) * siiIpvWinSizes[ sizeIndex].vertSpacer));
    DEBUG_PRINT( MSG_DBG, "Total Width: %d  Total Height: %d SizeIndex: %d\n", totalWidth, totalHeight, sizeIndex );
    if (( totalWidth > mpResolution.x) || ( totalHeight > mpResolution.y ))
    {
        pIpv->lastResultCode = SII_IPV_ERR_SCREEN_RES_TOO_SMALL;
        DEBUG_PRINT( MSG_ERR, "**** SII_IPV_ERR_SCREEN_RES_TOO_SMALL ****\n" );
    }

    // Re-calculate the window positioning for the current theme
    switch ( pIpv->currentTheme  )
    {
        case SiiIpvTOP_CENTER:
            upperLeft.x = (mpResolution.x - totalWidth) / 2;
            upperLeft.y = (siiIpvWinSizes[ sizeIndex].vertSpacer) & ~0x07;  // Multiple of 8 for best performance
            increment.x = siiIpvWinSizes[ sizeIndex].width16x9 + siiIpvWinSizes[ sizeIndex].horizSpacer;
            increment.y = 0;
            pIpv->highLightHoriz = 0;
            pIpv->highLightVert = siiIpvWinSizes[ sizeIndex].height;
            break;

        case SiiIpvBOTTOM_CENTER:
            upperLeft.x = (mpResolution.x - totalWidth) / 2;
            upperLeft.y = mpResolution.y - (siiIpvWinSizes[ sizeIndex].height + (siiIpvWinSizes[ sizeIndex].vertSpacer)) - 1;// * 2)) - 1;
            increment.x = siiIpvWinSizes[ sizeIndex].width16x9 + siiIpvWinSizes[ sizeIndex].horizSpacer;
            increment.y = 0;
            pIpv->highLightHoriz = 0;
            pIpv->highLightVert = -siiIpvWinSizes[ sizeIndex].height;
            break;

        case SiiIpvLEFT_MIDDLE:
            upperLeft.x = siiIpvWinSizes[ sizeIndex].horizSpacer * 2;
            upperLeft.y = ((mpResolution.y - totalHeight) / 2) & ~0x07; // Multiple of 8 for best performance
            increment.x = 0;
            increment.y = siiIpvWinSizes[ sizeIndex].height + siiIpvWinSizes[ sizeIndex].vertSpacer;
            pIpv->highLightHoriz = siiIpvWinSizes[ sizeIndex].width16x9;
            pIpv->highLightVert = 0;
            break;

        case SiiIpvRIGHT_MIDDLE:
            upperLeft.x = mpResolution.x - (siiIpvWinSizes[ sizeIndex].width16x9 + (siiIpvWinSizes[ sizeIndex].horizSpacer * 2));
            upperLeft.y = (mpResolution.y - totalHeight) / 2;
            increment.x = 0;
            increment.y = siiIpvWinSizes[ sizeIndex].height + siiIpvWinSizes[ sizeIndex].vertSpacer;
            pIpv->highLightHoriz = -siiIpvWinSizes[ sizeIndex].width16x9;
            pIpv->highLightVert = 0;
            break;

        case SiiIpvPentagram:
            upperLeft.x = siiIpvWinSizes[ sizeIndex].width16x9 / 2;
            upperLeft.y = siiIpvWinSizes[ sizeIndex].height / 2;
            pIpv->highLightVert = 0;
            pIpv->highLightHoriz = 0;
            break;

        case SiiIpvPIP:
            // Use the position specified by the PIP window data.
            upperLeft.x = siiIpvPipCoords[ sizeIndex].x;
            upperLeft.y = siiIpvPipCoords[ sizeIndex].y;

            // All windows must be programmed to the same location.
            increment.x = 0;
            increment.y = 0;

            // Set size independent of the normal method (only needs info from one window size)
            pIpv->windowSpecs.width16x9[0]  = siiIpvPipSizes[ sizeIndex].width16x9;
            pIpv->windowSpecs.width4x3[0]   = siiIpvPipSizes[ sizeIndex].width4x3;
            pIpv->windowSpecs.height[0]     = siiIpvPipSizes[ sizeIndex].height;
            sizeIndex = SiiIpvSizePIP;
            pIpv->highLightHoriz = 0;
            pIpv->highLightVert = 0;
            break;

        default:
            upperLeft.x = 0;
            upperLeft.y = 0;
            increment.x = 0;
            increment.y = 0;
            break;
    }


    // Coordinates for all of the windows
    if (pIpv->currentTheme != SiiIpvPentagram)
    {
        if ( isInterlaced )     // Must consider interlace format when calculating Y coordinate
        {
            upperLeft.y /= 2;
            increment.y /= 2;
        }
#if DETECT_ASPECT_RATIO
        pictureAspectRatio =  SiiDrvSwitchPictAspectRatioGet();
#endif
        for (i = 0; i < IPV_WINDOW_COUNT; i++)
        {
            pIpv->windowSpecs.coords[i].x = upperLeft.x & ~1; // Ensure even X coord in case it is YCbCr 4:2:2
            pIpv->windowSpecs.coords[i].y = upperLeft.y;

            if (sizeIndex != SiiIpvSizePIP)
            {
                pIpv->windowSpecs.width16x9[i] = siiIpvWinSizes[sizeIndex].width16x9;
#if DETECT_ASPECT_RATIO
                if(pictureAspectRatio == PICTURE_ASPECT16_9)
                {
                    pIpv->windowSpecs.width4x3[i] = siiIpvWinSizes[sizeIndex].width16x9;
                }
                else
#endif
                {
                    pIpv->windowSpecs.width4x3[i] = siiIpvWinSizes[sizeIndex].width16x9;     //Bug 31705 - Huawei Mobile is delivering 1080p/24Hz video but Melbourne is detecting as 480p resolution
                }

                pIpv->windowSpecs.height[i] = siiIpvWinSizes[sizeIndex].height;
                DEBUG_PRINT(
                        MSG_DBG, "%d) %4d/%dx%d @ %dx%d\n",
                        i,
                        pIpv->windowSpecs.width16x9[i], pIpv->windowSpecs.width4x3[i], pIpv->windowSpecs.height[i],
                        pIpv->windowSpecs.coords[i].x, pIpv->windowSpecs.coords[i].y
                );
            }
            else if (i == 0) // Only print PIP location once
            {
                DEBUG_PRINT( MSG_DBG, "PIP) %4dx%4d\n", pIpv->windowSpecs.coords[0].x, pIpv->windowSpecs.coords[0].y );
            }

            upperLeft.x += increment.x;
            upperLeft.y += increment.y;
        }
    }
    else
    {
        int32_t radius = (siiIpvRad[0] * mpResolution.y) / siiIpvRad[1] / 2;

#if DETECT_ASPECT_RATIO
        pictureAspectRatio =  SiiDrvSwitchPictAspectRatioGet();
#endif
        for (i = 0; i < IPV_WINDOW_COUNT; i++)
        {
            pIpv->windowSpecs.coords[i].x = siiIpvCc[i] * radius / 10000 + mpResolution.x / 2 - upperLeft.x;
            pIpv->windowSpecs.coords[i].y = siiIpvCs[i] * radius / 10000 + mpResolution.y / 2 - upperLeft.y;
            // Compensate for interlaced
            if (isInterlaced)
            {
                pIpv->windowSpecs.coords[i].y /= 2;
            }

            pIpv->windowSpecs.width16x9[i] = siiIpvWinSizes[sizeIndex].width16x9;
#if DETECT_ASPECT_RATIO
            if(pictureAspectRatio == PICTURE_ASPECT16_9)
            {
                pIpv->windowSpecs.width4x3[i] = siiIpvWinSizes[sizeIndex].width16x9;
            }
            else
#endif
            {
                pIpv->windowSpecs.width4x3[i] = siiIpvWinSizes[sizeIndex].width4x3;
            }
            pIpv->windowSpecs.height[i] = siiIpvWinSizes[sizeIndex].height;
        }
    }

    pIpv->windowSpecs.heightIsInterlaced = isInterlaced;
    return( true ); // Window specs have been updated
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable highlighting for passed preview window.  Removes previous highlight if any.
//! @param[in]  windowIndex:  0 - IPV_WINDOW_COUNT - Window to highlight
//!                          -1 - Remove all highlighting.
//-------------------------------------------------------------------------------------------------
void SiiIpvHighlightSet( int_t windowIndex )
{
    SiiWinCoords_t hl;
    int_t i, j, k;
    int_t step;
    int_t x[IPV_WINDOW_COUNT];
    int_t y[IPV_WINDOW_COUNT];

    if(pIpv->currentTheme == SiiIpvPentagram)
    {
        if ( pIpv->highLightIndex != -1 && (pIpv->highLightIndex != windowIndex))
        {
            SiiDrvIpvAnimationEnable(true);
            // Store current coordinates
            for (k = 0; k < IPV_WINDOW_COUNT; k++)
            {
                x[k] = pIpv->windowSpecs.coords[k].x;
                y[k] = pIpv->windowSpecs.coords[k].y;
            }

            // Shift coordinates
            i = windowIndex;
            j = pIpv->highLightIndex;

            step = ((i - j) > 0) ? 1 : -1;

            for (k = 0; k < IPV_WINDOW_COUNT; k++)
            {

                pIpv->windowSpecs.coords[i].x = x[j];
                pIpv->windowSpecs.coords[i].y = y[j];

                j += step;
                // modulo logic for j
                if (j < 0)
                    j += IPV_WINDOW_COUNT;
                else if (j >= IPV_WINDOW_COUNT)
                    j -= IPV_WINDOW_COUNT;

                i += step;
                // modulo logic for i
                if (i < 0)
                    i += IPV_WINDOW_COUNT;
                else if (i >= IPV_WINDOW_COUNT)
                    i -= IPV_WINDOW_COUNT;
            }

            pIpv->highLightIndex = windowIndex;

            // Do move
            for (i = 0; i < IPV_WINDOW_COUNT; i++)
            {
                hl.x = pIpv->windowSpecs.coords[i].x;
                hl.y = pIpv->windowSpecs.coords[i].y;
                SiiDrvIpvWindowCoordSet( i, hl.x, hl.y);
            }
        }
    }
    else
    {
        // Remove old highlight, if any...
        if ( pIpv->highLightIndex != -1 )
        {
            // Animation will work without glitches if there is no horizontal overlapping only,
            // so disable animation for top and bottom horizontal arrangements
            SiiDrvIpvAnimationEnable( (pIpv->currentTheme == SiiIpvLEFT_MIDDLE) || (pIpv->currentTheme == SiiIpvRIGHT_MIDDLE));
            SiiDrvIpvWindowCoordSet(
                pIpv->highLightIndex,
                pIpv->windowSpecs.coords[pIpv->highLightIndex].x,
                pIpv->windowSpecs.coords[pIpv->highLightIndex].y);
        }
        // Apply new highlight, if any...
        pIpv->highLightIndex = windowIndex;
        if (( pIpv->highLightIndex != -1 ) && pIpv->highLightEnabled )
        {
            hl.x = pIpv->windowSpecs.coords[pIpv->highLightIndex].x + pIpv->highLightHoriz;
            hl.y = pIpv->windowSpecs.coords[pIpv->highLightIndex].y;

            // Compensate for interlaced.
            if ( pIpv->currentIsInterlaced )
            {
                hl.y = pIpv->windowSpecs.coords[pIpv->highLightIndex].y + (pIpv->highLightVert / 2);
            }
            else
            {
                hl.y = pIpv->windowSpecs.coords[pIpv->highLightIndex].y + pIpv->highLightVert;
            }
            SiiDrvIpvWindowCoordSet( pIpv->highLightIndex, hl.x, hl.y);
            // Trigger bouncing effect start
            if (pIpv->currentTheme != SiiIpvPIP)
            {
                pIpv->isBounceEffect =
                    (((pIpv->currentTheme == SiiIpvLEFT_MIDDLE) ||
                    (pIpv->currentTheme == SiiIpvRIGHT_MIDDLE)) &&
                    pIpv->allowBounce );
                // Set bounce step depending on initial bouncing direction
                if (pIpv->isBounceEffect)
                {
                    switch(pIpv->currentTheme)
                    {
                        case SiiIpvRIGHT_MIDDLE:
                            pIpv->bounceStep =  SII_IPV_BOUNCE_AMPL;
                            break;
                        case SiiIpvLEFT_MIDDLE:
                            pIpv->bounceStep =  -SII_IPV_BOUNCE_AMPL;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        else
        {
            // disable bouncing if selection is unavailable
            pIpv->isBounceEffect = false;
        }
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Load the passed border color arrays into instance (but not hardware).
//! @param[in]  pNewColors
//-------------------------------------------------------------------------------------------------
bool_t SiiIpvWindowSpecsGet ( int window, SiiIpvSingleWindowSpecs_t *pSpecs )
{
    SiiIpvSingleWindowSpecs_t specs;

    if ( window < IPV_WINDOW_COUNT )
    {
        pIpv->lastResultCode = SII_IPV_SUCCESS;

        specs.coords.x  = pIpv->windowSpecs.coords[ window].x;
        specs.coords.y  = (pIpv->windowSpecs.heightIsInterlaced) ?
            pIpv->windowSpecs.coords[ window].y * 2 :
            pIpv->windowSpecs.coords[ window].y;

        specs.height    = pIpv->windowSpecs.height[ window];
        specs.width16x9 = pIpv->windowSpecs.width16x9[ window];
        specs.width4x3  = pIpv->windowSpecs.width4x3[ window];

        memcpy( pSpecs, &specs, sizeof(SiiIpvSingleWindowSpecs_t));
    }
    else
    {
        pIpv->lastResultCode = SII_IPV_ERR_INVALID_PARAMETER;

    }

    return( pIpv->lastResultCode == SII_IPV_SUCCESS );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Enable or disable the preview window highlighting
//! @param[in]  isEnable
//! @return     previous enable state
//-------------------------------------------------------------------------------------------------
bool_t SiiIpvHighlightEnable ( bool_t isEnable )
{
    bool_t  oldEnable = pIpv->highLightEnabled;

    pIpv->lastResultCode    = SII_IPV_SUCCESS;
    pIpv->highLightEnabled  = isEnable;

    return( oldEnable );
}
//-------------------------------------------------------------------------------------------------
//! @brief      Load the passed border color arrays into instance (but not hardware).
//! @param[in]  pNewColors
//-------------------------------------------------------------------------------------------------
void SiiIpvBorderColorsLoad ( uint8_t newColors[][YCBCR_LEN] )
{
    if ( pIpv->isEnabled )
    {
        pIpv->lastResultCode = SII_IPV_ERR_IN_USE;
    }
    else
    {
        memcpy( pIpv->borderColors, newColors, sizeof( pIpv->borderColors ));
        pIpv->lastResultCode = SII_IPV_SUCCESS;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief
//! @param[in]
//-------------------------------------------------------------------------------------------------
void SiiIpvAlphaLoad ( uint8_t *pNewAlphaMaxTable, uint8_t *pNewAlphaStepTable, int_t stepMs )
{
    if ( pIpv->isEnabled )
    {
        pIpv->lastResultCode = SII_IPV_ERR_IN_USE;
    }
    else
    {
        memcpy( pIpv->windowSpecs.alphaMax, pNewAlphaMaxTable, sizeof( pIpv->windowSpecs.alphaMax ));
        memcpy( pIpv->windowSpecs.alphaStep, pNewAlphaStepTable, sizeof( pIpv->windowSpecs.alphaStep ));
        pIpv->windowSpecs.alphaStepSpeedMs = stepMs;
        pIpv->lastResultCode = SII_IPV_SUCCESS;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Sets the alpha blending level for the specified window if enabled.
//!
//! @param[in]  windowIndex
//! @param[in]  newLevel
//-------------------------------------------------------------------------------------------------
void SiiIpvAlphaBlendSet ( int_t windowIndex, int_t newLevel )
{
    SiiDrvIpvWindowAlphaSet( windowIndex, newLevel );
    pIpv->lastResultCode = SII_IPV_SUCCESS;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Perform alpha stepping for all windows
//!
//! @returns    true if all windows at maximum alpha.
//-------------------------------------------------------------------------------------------------
bool_t SiiIpvAlphaStep( void )
{
    int_t   i;
    bool_t  atMax = true;

    for ( i = 0; i < IPV_WINDOW_COUNT; i++ )
    {
        if ( pIpv->alphaCurrentValue[ i] > pIpv->windowSpecs.alphaMax[i])
        {
            if ( pIpv->windowSpecs.alphaStep[i] >= pIpv->alphaCurrentValue[ i] )
            {
                pIpv->alphaCurrentValue[ i] = 0;
            }
            else
            {
                pIpv->alphaCurrentValue[ i] -= pIpv->windowSpecs.alphaStep[i];
                atMax = false;  // At least one window is not at max
            }
            SiiDrvIpvWindowAlphaSet( i, pIpv->alphaCurrentValue[ i] );
        }
    }

    return( atMax );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Select one of the built-in themes stored in firmware and load it into firmware.
//!             The theme is automatically adjusted for screen resolution,
//!             aspect ratio and preview window size.
//! @param[in]  newTheme
//-------------------------------------------------------------------------------------------------
void SiiIpvThemeSet ( SiiIpvTheme_t newTheme )
{
    int_t   i;

    pIpv->lastResultCode = SII_IPV_SUCCESS;
    do
    {
        if ( pIpv->isEnabled )
        {
            pIpv->lastResultCode = SII_IPV_ERR_IN_USE;
            break;
        }
        if (( newTheme >= SiiIpvThemeCount ) && ( newTheme != SiiIpvPIP))
        {
            pIpv->lastResultCode = SII_IPV_ERR_INVALID_PARAMETER;
            break;
        }

        // Calculate window specs based on new theme.
        pIpv->currentTheme  = newTheme;
        pIpv->highLightIndex = 0;
        UpdateWindowSpecs();    // Ignore res change response; we'll always reload window specs.

        // Load it all into the hardware.
        for ( i = 0; i < SiiIpvBorderColorCount; i++ )
        {
            SiiDrvIpvBoundaryColorTableSet( i, pIpv->borderColors[i] );
        }
        SiiDrvIpvWindowAlphaTableSet( 0, pIpv->windowSpecs.alphaMax, pIpv->windowSpecs.alphaStep, IPV_WINDOW_COUNT );
        SiiDrvIpvBorderEnablesSet( siiIpvBorderEnables[ pIpv->currentTheme]);

        SiiDrvIpvAnimationEnable(false);
        for ( i = 0; i < IPV_WINDOW_COUNT; i++ )
        {
            SiiDrvIpvWindowCoordSet( i, pIpv->windowSpecs.coords[i].x, pIpv->windowSpecs.coords[i].y);
        }
        SiiDrvIpvSizeSet( pIpv->windowSpecs.width16x9, pIpv->windowSpecs.width4x3, pIpv->windowSpecs.height);
    } while (0);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Displays the single window InstaPrevue mode known as PIP using the passed value
//!             as the PIP source.  To turn off PIP, call SiiIpvEnable( false );
//! @param[in]  pipSource - Source port for PIP window.
//-------------------------------------------------------------------------------------------------
bool_t SiiIpvPipModeEnable ( void )
{
    SiiIpvEnable( false );
    SiiIpvThemeSet( SiiIpvPIP );

    SiiIpvModeSet( SiiIpvModeSELECT );
    SiiIpvEnable( true );

    return( true );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Check the state of the InstaPrevue windows.
//!
//! @return     true if alpha stepping is not complete yet
//-------------------------------------------------------------------------------------------------
bool_t SiiIpvHandler ( clock_time_t msDiff )
{
    int_t   i;

    // Handle any pending interrupts
    if ( SiiDrvIpvHandleInterrupts())
    {
        // There was a change from unstable to stable resolution, recalculate as necessary...
        if ( UpdateWindowSpecs())
        {
            SiiDrvIpvAnimationEnable(false);
            // ...reload...
            for ( i = 0; i < IPV_WINDOW_COUNT; i++ )
            {
                SiiDrvIpvWindowCoordSet( i, pIpv->windowSpecs.coords[i].x, pIpv->windowSpecs.coords[i].y);
            }
            SiiDrvIpvSizeSet( pIpv->windowSpecs.width16x9, pIpv->windowSpecs.width4x3, pIpv->windowSpecs.height );
            SiiIpvHighlightSet( -1 );
        }
        SiiDrvIpvEnable( true );    // ...and turn IPV back on
    }

    // Perform window alpha stepping if needed
    if ( pIpv->isAlphaStepping )
    {
        if ( msDiff >= (clock_time_t)pIpv->alphaStepRemainingDelay )
        {
            pIpv->alphaStepRemainingDelay = pIpv->windowSpecs.alphaStepSpeedMs;
            pIpv->isAlphaStepping = !SiiIpvAlphaStep();
        }
        else
        {
            pIpv->alphaStepRemainingDelay -= msDiff;
        }
    }

    // Perform selected window bouncing effect if needed
    if (pIpv->isBounceEffect)
    {
        if ((pIpv->highLightIndex != -1) && SiiDrvIpvAnimationIsComplete() && pIpv->highLightEnabled )
        {
            //SiiDrvIpvAnimationEnable(true);
            i = pIpv->highLightIndex;
            SiiDrvIpvWindowCoordSet( i, pIpv->windowSpecs.coords[i].x + pIpv->highLightHoriz + pIpv->bounceStep,
                                        pIpv->windowSpecs.coords[i].y);
            pIpv->bounceStep = -pIpv->bounceStep;
            pIpv->bounceStep /= 2;

            if (pIpv->bounceStep == 0)
            {
                pIpv->isBounceEffect = false;
                pIpv->bounceStep = SII_IPV_BOUNCE_AMPL;
                SiiDrvIpvAnimationEnable(false);
            }
        }
    }

    return (pIpv->isAlphaStepping || pIpv->isBounceEffect);
}



