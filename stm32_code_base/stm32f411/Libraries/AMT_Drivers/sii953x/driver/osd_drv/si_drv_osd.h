//***************************************************************************
//!file     si_drv_osd.h
//!brief    Silicon Image OSD driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_OSD_DRV_H__
#define __SI_OSD_DRV_H__
#include "si_common.h"
#include "si_drv_osd_config.h"
#include "si_video_tables.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

typedef enum _SiiDrvOsdResultCodes_t
{
    SII_OSDDRV_SUCCESS      = 0,            // Success.
    SII_OSDDRV_ERR_FAIL,                    // General failure.
    SII_OSDDRV_ERR_INVALID_PARAMETER,       //
    SII_OSDDRV_ERR_IN_USE,                  // Module already initialized.
    SII_OSDDRV_ERR_NOT_AVAIL,               // Allocation of resources failed.

    SII_OSDDRV_ERR_NOT_ACTIVE,              // Attempted operation on an inactive window
    SII_OSDDRV_ERR_RESOURCE_ID_MISMATCH,    // Requested resource type does not match type of indexed resource.
    SII_OSDDRV_ERR_RESOURCE_NOT_LOADED,     // OSD resources have not been loaded.
    SII_OSDDRV_RESOURCE_READ_ERR,
    SII_OSDDRV_OSDRAM_ACCESS_ERR,           // Read or Write OSDRAM access by I2C failed
    SII_OSDDRV_OSDRAM_NOTHING_WRITTEN,      //

} SiiDrvOsdResultCodes_t;

typedef enum _SiiOsdResourceType_t
{
    OSD_RESOURCE_MAP        = (uint8_t)1,
    OSD_RESOURCE_FONT,
    OSD_RESOURCE_WINDOW,
    OSD_RESOURCE_ICON,
    OSD_RESOURCE_TEXT,

} SiiOsdResourceType_t;

typedef enum _SiiCursorStyle_t
{
    CURSOR_INVERT,
    CURSOR_BOX,
} SiiCursorStyle_t;


typedef enum _SiiOsdFontSize_t
{
    VAL_OSD_FONT_12x16 = 0,
    VAL_OSD_FONT_16x24,
    VAL_OSD_FONT_24x32,
} SiiOsdFontSize_t;

typedef enum
{
    OSD_WSTAT_ENABLED           = 0x0001,   // Window is enabled for use (visible or invisible)
    OSD_WSTAT_AVAILABLE         = 0x0002,   // Window is not used
    OSD_WSTAT_INVISIBLE         = 0x0004,   // Window is not visible
    OSD_WSTAT_STATUS_WINDOW     = 0x0008    // Window is a status window, possibly timed.
} SiiOsdWindowStatusTypes_t;

// Enums used in the window structure 'options' member
typedef enum
{
    OPTW_OSD_CURSOR     = 0x0002,       // Use OSD hardware cursor in window
    OPTW_TITLE          = 0x0004,       // Row 0 is title row
    OPTW_WRAPCURSOR     = 0x0008,       // Cursor wraps from top to bottom and vice-versa
    OPTW_PROGRESS_BAR   = 0x0010,       // Show slider as progress bar
    OPTW_ALLOW_ICON     = 0x0020,       // Make room for single character icon at beginning of each row

    // Slider window
    OPTW_SHOW_ENDCAPS   = 0x0100,       // Add rounded ends to bar
    OPTW_TEXT_TOP       = 0x0200,       // Place slider value text above slider bar
    OPTW_TEXT_BOTTOM    = 0x0400,       // Place slider value text above slider bar
    OPTW_TEXT_RIGHT     = 0x2000,       // Place slider thumb value text at end of slider bar
    OPTW_2_TICK         = 0x0800,       // 2 ticks per character (default is 1 tick)
    OPTW_4_TICK         = 0x1000,       // 4 ticks per character
} SiiOsdWindowOptions_t;

typedef enum
{
    VIDEO_VRES_480              = 480,
    VIDEO_VRES_576              = 576,
    VIDEO_VRES_720              = 720,
    VIDEO_VRES_1080             = 1080,
} SiiVerticalResList_t;

typedef enum _SiiOsdColorElement_t
{
    OSD_FG,
    OSD_BG,
    OSD_FG_HIGHLIGHT,
    OSD_BG_HIGHLIGHT,
    OSD_FG_DISABLED,
    OSD_BG_DISABLED,
    OSD_BORDER,
    OSD_ROW_DIVIDER,

    OSD_ELEMENT_COUNT
} SiiOsdColorElement_t;

// Enums used in the window function 'options' parameter
typedef enum
{
    OPT_HIDE            = 0x0000,       // Hide window (essentially just "not the OSD_SHOW bit")
    OPT_SHOW            = 0x0001,       // Show window

    OPT_ENABLE_CURSOR   = 0x0002,       // SetCursor only    Enable cursor display

    OPT_CANCEL          = 0x0001,       // CloseWindow only
    OPT_FORCE_CLOSE     = 0x0002,       // CloseWindow only

    OPT_SEND_UFA_FOCUS      = 0x0004,   // SetFocus() only
    OPT_PASS_PRIVATE_DATA   = 0x0008,   // ReplaceWindow only

    OPT_TOP             = 0x0002,       // Move window to top of z axis.
    OPT_RESOURCEID      = 0x0004,       // source data pointer is a resource ID
    OPT_ELLIPSIS        = 0x0008,       // Use ellipsis char if needed
    OPT_CENTER_WINDOW   = 0x0010,       // Center the window on the screen

    // Write Text
    OPT_CENTER_TEXT     = 0x0100,       // If less than window width, center text in window horizontally
    OPT_RIGHT_TEXT      = 0x0200,       // If less than window width, justify to the right side horizontally
    OPT_WRAP_TEXT       = 0x0400,       // Wrap text to next line if needed, starting at same column
    OPT_CLEAR_TO_END    = 0x0800,       // Fill to end of current row, or rows if OPT_WRAP_TEXT

    // Write Icon
    OPT_REMOVE_ICON     = 0x1000,       // Cause the icon to be removed by writing space characters instead of icon characters
    OPT_NO_TRANSLATE    = 0x2000,       // Do not run the source character through the translator callback.

    //Checkbox
    OPT_SET_CHECK       = 0x0100,       // Row options only - set the check box for a row
    OPT_ADD_CHECK       = 0x0200,       // Row options only - Add a checkbox to a row.

    //ListBox
    OPT_INSERT          = 0x1000,       // ListBox only - Insert listBox item instead of overwriting
} SiiOsdOptions_t;

#define OSD_RESOURCE_LOCAL_FLAG     0x8000      // Indicates that the resource is local

#define MAKEUINT32( p0, p1, p2, p3 )        ( (((uint32_t)p3) << 24) + (((uint32_t)p2) << 16) + (((uint32_t)p1) << 8) + ((uint32_t)p0) )

//------------------------------------------------------------------------------
// Hardware characteristics for the device controlled by this driver
//------------------------------------------------------------------------------

#define SII_OSD_WIN_COUNT           4
#define OSD_WIN_ROW_COUNT           16
#define OSD_WIN_COL_COUNT           32

#define OSD_REG_BLOCK_OFFSET        16
#define OSD_REG_BLOCK_SIZE          16

#define REG_INDEX_PATCH_OFFSET      3
#define OSD_WINDOW_ID_OFFSET_HORZ   2
#define OSD_WINDOW_ID_OFFSET_VERT   2
#define OSD_RAM_BURST_SIZE          6
#define OSDRAM_BYTES_PER_WORD       6
#define OSDRAM_INDEXES_PER_WORD     4

#define OSDRAM_WORDS_PER_12X16      4
#define OSDRAM_WORDS_PER_16X24      8
#define OSDRAM_WORDS_PER_24X32      16

// Font character statistics
#define OSD_PIXELS_CHAR_12X16       12
#define OSD_LINES_CHAR_12X16        16
#define OSD_PIXELS_CHAR_16X24       16
#define OSD_LINES_CHAR_16X24        24
#define OSD_PIXELS_CHAR_24X32       24
#define OSD_LINES_CHAR_24X32        32

#define ASCII_SPACE_CHAR            32

//------------------------------------------------------------------------------
// Structure definitions
//------------------------------------------------------------------------------
typedef struct _SiiOsdPatch_t
{
    uint8_t wId;            //Window id of the chars that going to be patched/
    uint8_t chNum;          //which patch char going to used
    uint16_t index;         //Index of location of the char in the font table
    uint8_t col;            //which col of the character to be patched
    uint8_t row;            //which row of the character to be patched
    uint8_t ctrl;           //enable or disable the patch
} SiiOsdPatch_t;


typedef struct _SiiFontInfo_t
{
    int xPixels;
    int yPixels;
    int osdWordCount;
} SiiFontInfo_t;

//
// Header data at front of the SPI data file.
//
typedef struct _SiiOsdSpiHeader_t
{
    uint8_t version;
    uint8_t month;      // in format "mm.dd.yyyy"
    uint8_t day;
    uint8_t yearHi;
    uint8_t yearLo;
    uint8_t minute;
    uint8_t milsecHi;
    uint8_t milsecLo;
    uint8_t pad1;
    uint8_t pad2;
    uint8_t pad3;
    uint8_t pad4;
} SiiOsdSpiHeader_t;

typedef struct  _SiiOsdResourceHeader_t
{
    uint8_t type;               // 0, index, 1 = font, 2 = window,
    uint8_t winType;            // higher level window type
    uint8_t sizeLo;             // Size of resource Lo value,
    uint8_t sizeHi;             // Size of resource Hi value,
} SiiOsdResourceHeader_t;

typedef struct  _SiiOsdResourceMapHeader_t
{
    uint8_t type;               // 0, index, 1 = font, 2 = window,
    uint8_t winType;            // higher level window type
    uint8_t sizeLo;             // Size of resource Lo value,
    uint8_t sizeHi;             // Size of resource Hi value,
    uint8_t fontCount;          // Number of Font resources
    uint8_t ciCount;            // Number of Window/Icon/Text resources
    uint8_t firstCiOffsetLo;    // Offset from the beginning of flash to the first content resource pointer
    uint8_t firstCiOffsetHi;    // This offset is a 16 bit value (little-endian)
    uint8_t firstFontOffset0;   // Offset from the beginning of flash to the first Font resource pointer
    uint8_t firstFontOffset1;   // This offset is a 32 bit value (little-endian)
    uint8_t firstFontOffset2;   // This offset is a 32 bit value (little-endian)
    uint8_t firstFontOffset3;   // This offset is a 32 bit value (little-endian)
} SiiOsdResourceMapHeader_t;

typedef struct _SiiDrvOsdWindowResource_t
{
    SiiOsdResourceHeader_t      resHeader;
    uint8_t     wIdLo;
    uint8_t     wIdHi;
    uint8_t     windowStatusLo;
    uint8_t     windowStatusHi;
    uint8_t     optionsLo;
    uint8_t     optionsHi;
    uint8_t     horizStartLo;
    uint8_t     horizStartHi;
    uint8_t     vertStartLo;
    uint8_t     vertStartHi;
    uint8_t     rowCount;
    uint8_t     colCount;
    uint8_t     dividerWidth;
    uint8_t     borderWidth;
    uint8_t     highlightRow;
    uint8_t     disabledRowMapLo;
    uint8_t     disabledRowMapHi;
    uint8_t     foreColor;
    uint8_t     backColor;
    uint8_t     foreColorHighlight;
    uint8_t     backColorHighlight;
    uint8_t     foreColorDisabled;
    uint8_t     backColorDisabled;
    uint8_t     borderColor;
    uint8_t     dividerColor;
    uint8_t     alphaBlendLevel;
    uint8_t     msDisplayTimeLo;
    uint8_t     msDisplayTimeHi;
    uint8_t     windowFunctionId;
    uint8_t     param1;
    uint8_t     param2;
    uint8_t     param3;
    uint8_t     param4;
    uint8_t     editMode;

    uint8_t     rowChildWindowId [OSD_WIN_ROW_COUNT];
    uint8_t     contentIndexWordData[OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD];

} SiiDrvOsdWindowResource_t;

typedef struct _SiiDrvOsdIconResource_t
{
    SiiOsdResourceHeader_t      resHeader;
    uint8_t     wIdLo;
    uint8_t     wIdHi;
    uint8_t     optionsLo;
    uint8_t     optionsHi;
    uint8_t     rowCount;
    uint8_t     colCount;
    uint8_t     foreColor;
    uint8_t     backColor;
    uint8_t     alphaBlendLevel;
    uint8_t     contentIndexWordData[OSD_CI_ICONSIZE_SIZE * OSDRAM_BYTES_PER_WORD];

} SiiDrvOsdIconResource_t;

typedef struct _SiiDrvOsdTextResource_t
{
    SiiOsdResourceHeader_t      resHeader;
    uint8_t     wIdLo;
    uint8_t     wIdHi;
    uint8_t     optionsLo;
    uint8_t     optionsHi;
    uint8_t     foreColor;
    uint8_t     backColor;
    uint8_t     alphaBlendLevel;
    uint8_t     contentIndexWordData[OSD_CI_TEXTSIZE_SIZE * OSDRAM_BYTES_PER_WORD];

} SiiDrvOsdTextResource_t;

//------------------------------------------------------------------------------
// Function templates
//------------------------------------------------------------------------------

// si_drv_osd.c
SiiDrvOsdResultCodes_t SiiDrvOsdGetLastResult( void );
bool_t  SiiDrvOsdInitialize( void );
void    SiiDrvOsdSetFontResourceId( SiiOsdFontSize_t fontSize, int resourceId );
bool_t  SiiDrvOsdResolutionChange( videoFormatId_t newVidMode, int newVres, bool_t newInterlace );
bool_t  SiiDrvOsdHandleInterrupts( void );

// si_drv_osd_resource.c
bool_t  SiiDrvOsdIsResource( SiiOsdResourceType_t resourceType, uint16_t resourceId );
char    *SiiDrvOsdGetTextResource( char *pStr );
bool_t  SiiDrvOsdGetMapResource( void );
bool_t  SiiDrvOsdLoadWindowResource( int winRef, int winResourceId, bool_t useCurrentWindowSize );
bool_t  SiiDrvOsdLoadFontResource( int fontResourceId );
void    SiiDrvOsdSetLocalResourceData( int resourceCount, SiiDrvOsdWindowResource_t const *pResourceData );

int SiiDrvOsdGetFontWidth( void );
int SiiDrvOsdGetFontHeight( void );

// si_drv_osd_window.c

void    SiiDrvOsdSetWindowSize( int winRef, int rowCount, int colCount );
void    SiiDrvOsdSetHorizVert( int winRef, uint16_t horizVal, uint16_t vertVal);
void    SiiDrvOsdGetHorizVert( int winRef, uint16_t *pHorz, uint16_t *pVert);
void    SiiDrvOsdSetBlending( int winRef, uint8_t value, uint8_t ctrl);
void    SiiDrvOsdGetBlending( int winRef, uint8_t *value);
void    SiiDrvOsdSetColor( int winRef, SiiOsdColorElement_t element, uint8_t colorVal );
int     SiiDrvOsdGetColor( int winRef, SiiOsdColorElement_t element );
int     SiiDrvOsdGetCursor ( int winRef );
bool_t  SiiDrvOsdRowIsEnabled( int winRef, int row );
void    SiiDrvOsdRowEnable( int winRef, int row, bool_t isEnable );
void    SiiDrvOsdSetWindowZaxis( int winRef, int level );
void    SiiDrvOsdWndEnable( int winRef, bool_t isEnable );
void    SiiDrvOsdEnable( bool_t enableIt );
void    SiiDrvOsdSetCharPatch( int winIndex, bool_t isEnable, int row, int col, int newChar, int patchIndex );
void    SiiDrvOsdClearCharPatch( int patchIndex );
void    SiiDrvOsdClearAllCharPatch( void );
void    SiiDrvOsdSetFontType(uint8_t fontType);
void    SiiDrvOsdStateUpdate ( void );

// si_drv_osd_tools.c

bool_t  SiiDrvOsdSwapSurfaces( void );
int     SiiDrvOsdGetCharIndex( int winRef, int row, int col );
void    SiiDrvOsdCursorChar( int winRef, int cursorType, bool_t removeCursor, int row, int col );
void    SiiDrvOsdWriteIcon( int winRef, int row, int col, int rowCount, int colCount, uint8_t *pIcon, SiiOsdOptions_t options );
bool_t  SiiDrvOsdWriteText( int winRef, int row, int col, char *pStr, SiiOsdOptions_t options );
void    SiiDrvOsdClearTextRow( int winRef, int row, int col, SiiOsdOptions_t options );
bool_t  SiiDrvOsdSetCursor( int winRef, int row, SiiOsdOptions_t options );
void    SiiDrvOsdShowWindow( int winRef, SiiOsdOptions_t options );
bool_t  SiiDrvOsdGetWindowProperties( int winRef, void *pWin );
int     SiiDrvOsdCreateWindow( int windowId, int startX, int startY, int rowCount, int colCount );
void    SiiDrvOsdCloseWindow( int winRef );
void    SiiDrvOsdMoveWindow( int winRef, int startX, int startY, SiiOsdOptions_t options );

// si_drv_osd_osd_ram.c

bool_t  SiiDrvOsdRamI2cWrite( int osdWordAddr,  int osdWordCount, uint8_t *pByteData );
bool_t  SiiDrvOsdRamI2cRead( int osdWordAddr,  int osdWordCount, uint8_t *pByteData );

// si_drv_osd_osd_ram.c

bool_t  SiiDrvOsdSpiOsdRamLoad ( uint32_t spiByteAddr,  uint32_t osdWordAddr, uint16_t spiByteLength);


//-------------------------------------------------------------------------------------------------
//! @brief  OSD driver application callback function to translate character value into a
//!         font character index and vice-versa.
//-------------------------------------------------------------------------------------------------
int SiiDrvOsdCbCharacterToIndex( int lookupChar );
int SiiDrvOsdCbIndexToCharacter( int lookupIndex );


#endif // __SI_OSD_DRV_H__
