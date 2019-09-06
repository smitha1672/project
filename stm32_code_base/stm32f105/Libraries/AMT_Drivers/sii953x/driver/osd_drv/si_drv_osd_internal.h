//***************************************************************************
//!file     si_drv_osd_internal.h
//!brief    Silicon Image OSD internal driver functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_OSD_DRV_INTERNAL_H__
#define __SI_OSD_DRV_INTERNAL_H__
#include "si_common.h"
#include "si_osal_timer.h"
#include "si_osd_config.h"
#include "si_drv_osd.h"

#define MAX_VERT_PERIOD_MS      42  // 42ms is ~1 (rounded up) frame @24FPS

//------------------------------------------------------------------------------
//  OSD Driver internal type definitions
//------------------------------------------------------------------------------

typedef struct _SiiOsdDirtyList_t
{
    int     startWord;              // OSDRAM index
    int     wordCount;              // OSDRAM word count
    int     startByte;              // byte index into contentWordIndexData[]
    int     dirtyBits;              // bit 0 == surface 0 dirty, bit 1 == surface 1 dirty
} SiiOsdDirtyList_t;

typedef struct _SiiDrvOsdWindow_t
{
    SiiOsdWindowStatusTypes_t   windowStatus;

    int         winType;
    int         resourceId;
    int         options;
    int         upperLeftX;
    int         upperLeftY;
    int         rowCount;
    int         colCount;

    int         highlightRow;
    bool_t      highlightEnabled;
    int         borderWidth;
    int         dividerWidth;
    uint16_t    disabledRowMap;

    uint8_t     foreColor;              // Unselected rows
    uint8_t     backColor;
    uint8_t     foreColorHighlight;
    uint8_t     backColorHighlight;
    uint8_t     foreColorDisabled;
    uint8_t     backColorDisabled;
    uint8_t     borderColor;
    uint8_t     dividerColor;

    uint8_t     alphaBlendLevel;
    int         msTimeout;
    uint8_t     windowFunctionId;
    uint8_t     param1;
    uint8_t     param2;
    uint8_t     param3;
    uint8_t     param4;
    uint8_t     editMode;

    uint8_t     rowChildWindowId [OSD_WIN_ROW_COUNT];
    uint16_t    contentIndexStartWord;
    uint8_t     contentIndexWordData[OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD ];

    SiiOsdDirtyList_t   dirtyList[32];
} SiiDrvOsdWindow_t;

typedef enum
{
    OSD_STATE_DISABLED,         // Explicitly Disabled
    OSD_STATE_PRE_ENABLE,       // Enable requested, clock/resolution not stable
    OSD_STATE_ENABLED,          // OSD is enabled and displaying

} SiiDrvOsdEnableState_t;

//------------------------------------------------------------------------------
//  OSD Driver Instance Data
//------------------------------------------------------------------------------

typedef struct  _SiiOsdDrvInstanceData_t
{
    int             structVersion;
    uint_t          instanceIndex;
    int             lastResultCode;                 // Contains the result of the last API function called
    uint16_t        statusFlags;
    bool_t          interruptPending;

    SiiDrvOsdEnableState_t  enableState;
    int                     bitmapSegmentStart;     // OSD RAM WORD bitmap segment offset

    SiiDrvOsdWindow_t       winData[4];

    int                         localResourceCount;
    SiiDrvOsdWindowResource_t   const *pLocalResourceData;
    int                         externalResourceCount;
    uint32_t                    externalResourceMap[OSD_RESOURCE_LIMIT];    // Resource index array

    // Font knowledge
    SiiOsdFontSize_t    fontSize;                   // Size of current font.
    int                 fontResourceId;             // Resource ID of current font.
    int                 fontHeight;
    int                 fontWidth;
    int                 lastFontCharacterIndex;     // Index of last font character in current font
    int                 defaultFontId;              // Resource ID of default font.
    SiiOsdFontSize_t    defaultFontSize;
    int                 defaultFontHeight;
    int                 defaultFontWidth;
    int                 defaultDisplayHeight;       // Height of display associated with default font.
    int                 defaultDisplayWidth;        // Width of display associated with default font.
    bool_t              useDefaultFont;
    int                 fontResourceId12x16;        // Resource ID of default 12x16 font.
    int                 fontResourceId16x24;        // Resource ID of default 16x24 font.
    int                 fontResourceId24x32;        // Resource ID of default 24x32 font.

    bool_t              outputIsInterlace;
    int                 outputVres;

}   SiiOsdDrvInstanceData_t;

extern SiiOsdDrvInstanceData_t  drvOsdInstance[1];
extern SiiOsdDrvInstanceData_t *pDrvOsd;

extern SiiDrvOsdWindow_t defaultStyle;

extern int l_fontWidths[];
extern int l_fontHeights[];
extern int l_fontVres[];
extern int l_fontHres[];

//------------------------------------------------------------------------------
//  OSD Driver internal functions
//------------------------------------------------------------------------------

bool_t  SiiDrvOsdGetResourceHeader ( uint16_t resourceId, SiiOsdResourceHeader_t *pHeader );
bool_t  SiiDrvOsdIsResource( SiiOsdResourceType_t resourceType, uint16_t resourceId );
bool_t  SiiDrvOsdGetResource( SiiOsdResourceType_t resourceType, uint16_t resourceId, uint32_t *pSize, uint8_t *pDestData );

bool_t  SiiDrvOsdSpiOsdRamLoad ( uint32_t spiByteAddr,  uint32_t osdWordAddr, uint16_t spiByteLength);
void    DrvOsdDisableInternal( void );

bool_t  OsdWaitForWriteIntDone(void);
void    OsdSetIndexValue( uint8_t *pBuffer, int charIndex, int indexValue );

void    SiiDrvOsdUpdateDirtyList( SiiDrvOsdWindow_t *pWin, int startWordIndex, int osdWordCount, int byteIndex );
void    SiiOsdDrvClearDirtyList( SiiDrvOsdWindow_t *pWin );
bool_t  SiiDrvOsdIsDirtyLocation( SiiDrvOsdWindow_t *pWin, int wordIndex );
bool_t  SiiDrvOsdWriteTextRow( SiiDrvOsdWindow_t *pWin, int row, int col, char *pStr, int length, SiiOsdOptions_t options );

void    SiiDrvOsdProcessInterrupts ( void );

#endif // __SI_OSD_DRV_INTERNAL_H__
