//***************************************************************************
//!file     si_osd_internal.h
//!brief    Silicon Image OSD Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_OSD_INTERNAL_H__
#define __SI_OSD_INTERNAL_H__
#include "si_device_config.h"
#include "si_osal_timer.h"

#include "si_osd_component.h"

//------------------------------------------------------------------------------
//  OSD Component internal data types
//------------------------------------------------------------------------------

typedef struct _SiiOsdWindow_t
{
    SiiOsdWindowStatusTypes_t   windowStatus;

    int         windowType;             // Type of window (OSD_WINTYPE_BASIC, OSD_WINTYPE_SLIDER, etc.)
    int         resourceId;
    int         options;                // Window options

    int         upperLeftX;
    int         upperLeftY;
    int         rowCount;
    int         colCount;

    int         highlightRow;
    bool_t      highlightEnabled;
    int         borderWidth;
    int         dividerWidth;

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
    SiiTimer_t  winTimer;

    uint8_t     rowChildWindowId [OSD_WIN_ROW_COUNT];

    int         childWinRef;            // winRef of child window if displayed, otherwise 0.
    int         parentWinRef;
    int         windowFunctionId;

    SiiOsdUserFunction_t    pFunction;
    // Spare data used by some window types.
    void        *pExternalData;
    int         param1;
    int         param2;
    int         param3;
    int         param4;
    int         editMode;

} SiiOsdWindow_t;

//------------------------------------------------------------------------------
//  OSD Component Instance Data
//------------------------------------------------------------------------------

typedef struct _OsdInstanceData_t
{
    int                 structVersion;
    int                 instanceIndex;
    SiiOsdResultCodes_t lastResultCode;     // Contains the result of the last API function called
    uint16_t            statusFlags;

    bool_t              isEnabled;
    int                 fontWidth;
    int                 fontHeight;

    SiiOsdWindow_t      winData[ SII_OSD_WIN_COUNT ];

    int                     functionCount;
    SiiOsdUserFunction_t    *pUserFuncs;

    int                 focusWinRef;        // winRef of window currently in focus

}   OsdInstanceData_t;

extern OsdInstanceData_t osdInstance[1];
extern OsdInstanceData_t *pOsd;

#define SII_OSD_UF_MSGBOX   0xC000      // Signal a builtin user function

//------------------------------------------------------------------------------
//  Local function definitions
//------------------------------------------------------------------------------

bool_t  OsdExecuteUserFunction ( int winRef, int action, int key );
int     OsdCreateBaseWindow( int windowId, int startX, int startY, int rowCount, int colCount );

#endif // __SI_OSD_INTERNAL_H__
