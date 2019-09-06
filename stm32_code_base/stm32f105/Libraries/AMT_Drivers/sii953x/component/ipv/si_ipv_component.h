//***************************************************************************
//!file     si_ipv_component.h
//!brief    Silicon Image InstaPrevue Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_IPV_COMPONENT_H__
#define __SI_IPV_COMPONENT_H__
#include "si_common.h"
#include "si_ipv_config.h"
#include "si_drv_ipv.h"
#include "si_drv_switch.h"


//------------------------------------------------------------------------------
//  Constants
//------------------------------------------------------------------------------

typedef enum _SiiIpvResultCodes_t
{
    SII_IPV_SUCCESS      = 0,           // Success.
    SII_IPV_ERR_FAIL,                   // General failure.
    SII_IPV_ERR_INVALID_PARAMETER,      //
    SII_IPV_ERR_IN_USE,                 // Cannot perform requested function
                                        // while IPV is in use
    SII_IPV_ERR_NOT_AVAIL,              // Allocation of resources failed.

    SII_IPV_ERR_SCREEN_RES_TOO_SMALL,   // Screen resolution too small for all windows.

} SiiIpvResultCodes_t;

typedef enum _SiiIpvTheme_t
{
    SiiIpvTOP_CENTER,
    SiiIpvLEFT_MIDDLE,
    SiiIpvBOTTOM_CENTER,
    SiiIpvRIGHT_MIDDLE,
    SiiIpvPentagram,

    SiiIpvPIP,
    SiiIpvThemeCount,
} SiiIpvTheme_t;

typedef enum _SiiIpvSizeSet_t
{
    SiiIpvSize640x480,
    SiiIpvSize720x480,
    SiiIpvSize1280x720,
    SiiIpvSize1440x480,     // Bug 33345 - Melbourne SiI9535|when only two sources are connected to port 0 and port 1 PIP fails to work 
    SiiIpvSize1920x1080,

    SiiIpvSizeCount,
    SiiIpvSizePIP,          // Special size value
} SiiIpvSizeSet_t;


#define YCBCR_LEN       3

//------------------------------------------------------------------------------
//  Data Structures
//------------------------------------------------------------------------------

typedef struct
{
    int_t    x;
    int_t    y;
} SiiWinCoords_t;

typedef struct _SiiWinSize_t
{
    int_t       width16x9;
    int_t       width4x3;
    int_t       height;
    int_t       horizSpacer;
    int_t       vertSpacer;
} SiiWinSize_t;

typedef struct _SiiIpvWindowSpecs_t
{
    SiiWinCoords_t  coords[IPV_WINDOW_COUNT];
    uint16_t        width16x9[IPV_WINDOW_COUNT];    // 16:9 width when 1 to IPV_WINDOW_COUNT frames active
    uint16_t        width4x3[IPV_WINDOW_COUNT];     // 4:3 width when 1 to IPV_WINDOW_COUNT frames active
    uint8_t         height[IPV_WINDOW_COUNT];       // Height when 1 to IPV_WINDOW_COUNT frames active
    uint8_t         alphaMax[IPV_WINDOW_COUNT];     // Maximum alpha blend level for each window
    uint8_t         alphaStep[IPV_WINDOW_COUNT];    // Alpha blend level step for each window
    int_t           alphaStepSpeedMs;               // step speed in milliseconds
    bool_t          heightIsInterlaced;             // Height values are modified for interlace mode.
} SiiIpvWindowSpecs_t;

typedef struct _SiiIpvSingleWindowSpecs_t
{
    SiiWinCoords_t  coords;
    uint16_t        width16x9;
    uint16_t        width4x3;
    uint8_t         height;
} SiiIpvSingleWindowSpecs_t;

//------------------------------------------------------------------------------
//  Data
//------------------------------------------------------------------------------

extern SiiWinSize_t     siiIpvWinSizes[SiiIpvSizeCount];
extern uint8_t          siiIpvBorderEnables[SiiIpvThemeCount];                      // Display boundary enables for each theme
extern uint8_t          siiIpvBorderColorsCrCbY[SiiIpvBorderColorCount][YCBCR_LEN]; // YCbCr color data array
extern uint8_t          siiIpvAlphaMax[IPV_WINDOW_COUNT];                           // Maximum alpha blend level for each window
extern uint8_t          siiIpvAlphaStep[IPV_WINDOW_COUNT];                          // Alpha blend level step for each window

// Picture-In-Picture info
extern SiiWinSize_t     siiIpvPipSizes[SiiIpvSizeCount];
extern SiiWinCoords_t   siiIpvPipCoords[SiiIpvSizeCount];

extern SiiWinCoords_t   siiMainWindowSizes[SiiIpvSizeCount];

extern const int32_t    siiIpvCc[IPV_WINDOW_COUNT];
extern const int32_t    siiIpvCs[IPV_WINDOW_COUNT];
extern const int32_t    siiIpvRad[2];

//------------------------------------------------------------------------------
//  Function templates
//------------------------------------------------------------------------------

bool_t  SiiIpvInitialize( void );
SiiIpvResultCodes_t SiiIpvGetLastResult( void );

bool_t  SiiIpvWindowSpecsGet( int window, SiiIpvSingleWindowSpecs_t *pSpecs );
bool_t  SiiIpvHighlightEnable( bool_t isEnable );
void    SiiIpvBorderColorsLoad( uint8_t newColors[][YCBCR_LEN] );
void    SiiIpvAlphaLoad( uint8_t *pNewAlphaMaxTable, uint8_t *pNewAlphaStepTable, int_t stepMs );
bool_t  SiiIpvAlphaStep( void );
void    SiiIpvHighlightSet( int_t windowIndex );

void    SiiIpvAlphaBlendSet( int_t windowIndex, int_t newLevel );
void    SiiIpvModeSet( SiiIpvMode_t mode );
bool_t  SiiIpvEnable( bool_t isEnable );
void    SiiIpvRefresh(void);
void    SiiIpvThemeSet( SiiIpvTheme_t newTheme );
bool_t  SiiIpvPipModeEnable ( void );
int_t   SiiIpvHighlightWindow( int_t highlightIndex,  bool_t isForward );
int_t   SiiIpvHighlightPortGet ( int_t currentPortIndex, int_t highlightIndex );

bool_t  SiiIpvHandler( clock_time_t msDiff );

bool_t  SiiIpvCbVideoInfoGet(int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced);

#endif // __SI_IPV_COMPONENT_H__
