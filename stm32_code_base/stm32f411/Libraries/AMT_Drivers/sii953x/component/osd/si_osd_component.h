//***************************************************************************
//!file     si_osd_component.h
//!brief    Silicon Image OSD Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_OSD_COMPONENT_H__
#define __SI_OSD_COMPONENT_H__
#include "si_device_config.h"
#include "si_osd_config.h"
#include "si_video_tables.h"
#include "si_drv_osd.h"

//------------------------------------------------------------------------------
//  OSD Component Constants and Enums
//------------------------------------------------------------------------------

typedef enum _SiiOsdResultCodes_t
{
    SII_OSD_SUCCESS      = 0,           // Success.
    SII_OSD_ERR_FAIL,                   // General failure.
    SII_OSD_ERR_INVALID_PARAMETER,      //
    SII_OSD_ERR_IN_USE,                 // Cannot perform requested function
                                        // while OSD is in use
    SII_OSD_ERR_NOT_AVAIL,              // Allocation of resources failed.

    SII_OSD_ERR_NOT_VALID,              // Requested window is not allocated
    SII_OSD_ERR_NOT_ENABLED,            // Requested window is not enabled
    SII_OSD_ERR_WINTYPE_MISMATCH,       // Window not of correct type for operation requested
    SII_OSD_ERR_USER_FUNC_FAIL,         // User function returned failure
    SII_OSD_NOT_VALID_RESOURCE,         // Referenced value is not a valid resource.
    SII_OSD_RESOURCE_LOAD_ERROR,        // Error loading referenced resource.

} SiiOsdResultCodes_t;

typedef enum _SiiOsdWindowTypes_t
{
    OSD_WINTYPE_BASIC       = 1,
    OSD_WINTYPE_SLIDER,
    OSD_WINTYPE_STATUS,
    OSD_WINTYPE_LISTBOX,
    OSD_WINTYPE_MENU,
    OSD_WINTYPE_EDITBOX,

} SiiOsdWindowTypes_t;

typedef struct _SiiOsdWinTypeData_t
{
    SiiOsdWindowTypes_t winType;
    int                 param1;
    int                 param2;
    int                 param3;
    int                 param4;
    int                 editMode;
    bool_t              useParamValues;
} SiiOsdWinTypeData_t;

#include "si_infrared.h"    //TODO: THIS IS TEMPORARY!!!

typedef enum _RcKeys_t
{
    SIIKEY_0         = RC5_0,
    SIIKEY_1         = RC5_1,
    SIIKEY_2         = RC5_2,
    SIIKEY_3         = RC5_3,
    SIIKEY_4         = RC5_4,
    SIIKEY_5         = RC5_5,
    SIIKEY_6         = RC5_6,
    SIIKEY_7         = RC5_7,
    SIIKEY_8         = RC5_8,
    SIIKEY_9         = RC5_9,
    SIIKEY_VOL_UP    = RC5_VOL_UP,
    SIIKEY_VOL_DOWN  = RC5_VOL_DOWN,
    SIIKEY_CH_UP     = RC5_CH_PLUS,
    SIIKEY_CH_DOWN   = RC5_CH_MINUS,
    SIIKEY_MUTE      = RC5_MUTE,
    SIIKEY_POWER     = RC5_STANDBY,

    SIIKEY_CURSOR_UP    = RC5_CURSOR_UP,
    SIIKEY_CURSOR_DOWN  = RC5_CURSOR_DOWN,
    SIIKEY_CURSOR_LEFT  = RC5_CURSOR_LEFT,
    SIIKEY_CURSOR_RIGHT = RC5_CURSOR_RIGHT,
    SIIKEY_SCROLL_UP    = RC5_CH_PLUS,
    SIIKEY_SCROLL_DOWN  = RC5_CH_MINUS,
    SIIKEY_ENTER        = RC5_ENTER,
    SIIKEY_SELECT       = RC5_ENTER,
    SIIKEY_CANCEL       = RC5_QUIT,

    SIIKEY_GUIDE        = RC5_GUIDE,
    SIIKEY_MENU         = RC5_MENU,
    SIIKEY_INFO         = RC5_0,
    SIIKEY_QUIT         = RC5_QUIT,

    SIIKEY_INPUT        = RC5_INPUT,
    SIIKEY_PIP          = RC5_PIP,
#if (PHILIPS_REMOTE == 1)
    SIIKEY_TOGGLE       = RC5_TOGGLE,
#else
    SIIKEY_TOGGLE       = RC5_GUIDE,
#endif

    SIIKEY_UNKNOWN      = 0xFF

} RcKeys_t;

typedef enum
    {
    UFA_CANCEL,
    UFA_CREATE,
    UFA_CLOSE,
    UFA_ROW_ENTER,          // CURSOR UP/DOWN
    UFA_SCROLL,             // SCROLL UP/DOWN
    UFA_BACK,               // CURSOR LEFT
    UFA_FOCUS,              // CURSOR RIGHT, child window received focus
    UFA_SELECT,             // ENTER/SELECT
    UFA_REPLACEWINDOW,
    UFA_UPDATE,             // SLIDER/PROGRESS WINDOW ONLY

    } SiiOsdUserFunctionAction_t;

//------------------------------------------------------------------------------
//  OSD Component data typedefs
//------------------------------------------------------------------------------

typedef struct _SiiFuncArgs_t
{
    int     resourceId;
    int     options;
    int     selectedRow;
    int     rowChildWinId;
    int     childWinRef;
    int     parentWinRef;
    int     actionKey;
    int     param1;
    int     param2;
    int     param3;
    int     param4;
    int     editMode;
    void    *pExternalData;
} SiiFuncArgs_t;


typedef bool_t (*SiiOsdUserFunction_t)( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );

//------------------------------------------------------------------------------
//  Public data
//------------------------------------------------------------------------------

//extern const SiiOsdUserFunction_t   g_UserFuncs[];

//------------------------------------------------------------------------------
//  Public function definitions
//------------------------------------------------------------------------------

// si_osd_component.c

SiiOsdResultCodes_t SiiOsdGetLastResult( void );
bool_t  SiiOsdInitialize( int functionCount, SiiOsdUserFunction_t *pUserFuncs );
bool_t  SiiOsdWindowManager( int key, SiiOsdOptions_t options );

// si_osd_win_listbox.c

int     SiiOsdListBoxGetSelect( int winRef );
bool_t  SiiOsdListBoxSetSelect( int winRef, int itemIndex, SiiOsdOptions_t options );
bool_t  SiiOsdListBoxAddItem( int winRef, char *pItemText, int index, SiiOsdOptions_t options );
bool_t  SiiOsdListBoxDeleteItem( int winRef, int index, SiiOsdOptions_t options );
bool_t  SiiOsdListBoxGetItem( int winRef, int index, char *pDestStr, int length );
bool_t  SiiOsdListBoxHandler( int winRef, int key, SiiOsdOptions_t options );

// si_osd_win_menu.c

void    SiiOsdMenuSetItemCheck( int winRef, int itemIndex, SiiOsdOptions_t options );
bool_t  SiiOsdMenuGetItemCheck( int winRef, int itemIndex );
bool_t  SiiOsdMenuAddItemCheck( int winRef, int itemIndex, SiiOsdOptions_t options );
bool_t  SiiOsdMenuSetItemIcon( int winRef, int itemIndex, uint8_t *pIcon, SiiOsdOptions_t options );
void    SiiOsdMenuItemEnable( int winRef, int itemIndex, SiiOsdOptions_t options );
int     SiiOsdMenuGetSelect( int winRef );
bool_t  SiiOsdMenuSetSelect( int winRef, int itemIndex );
bool_t  SiiOsdMenuHandler( int winRef, int key, SiiOsdOptions_t options );

// si_osd_win_slider.c

bool_t  SiiOsdSliderUpdate( int winRef, int thumbValue, SiiOsdOptions_t options );
bool_t  SiiOsdSliderHandler( int winRef, int key, SiiOsdOptions_t options );
int     SiiOsdSliderValueGet( int winRef );

// si_osd_win_edit.c

int     SiiOsdCreateEditBox( int winRef, int windowId, int startX, int startY, char *pTextBuffer, int bufferLen, SiiOsdOptions_t options );
bool_t  SiiOsdEditBoxHandler( int winRef, int key, SiiOsdOptions_t options );

// si_osd_win_msgbox.c

int     SiiOsdMessageBox( int windowId, char *pTitleStr, char *pMsgStr, int startX, int startY, int msTimeout, SiiOsdOptions_t options );
int     SiiOsdResourceMessageBox( int resourceId, int startX, int startY, int msTimeout, SiiOsdOptions_t options );

// si_osd_window.c

bool_t  SiiOsdAttachFunction( int winRef, SiiOsdUserFunction_t pFunction );
bool_t  SiiOsdAttachExternalData( int winRef, void *pExternalData );
bool_t  SiiOsdSetWindowPrivateData( int winRef, int p1, int p2, int p3, int p4, int editMode );
bool_t  SiiOsdSetColor( int winRef, SiiOsdColorElement_t element, int rgbVal );
bool_t  SiiOsdSetRowEnable( int winRef, int row, bool_t isEnable );
bool_t  SiiOsdIsRowEnabled( int winRef, int row );
int     SiiOsdGetFocus( void );
int     SiiOsdSetFocus( int winRef, SiiOsdOptions_t options );
bool_t  SiiOsdSetWindowTimeout( int winRef, int msTimeout );
bool_t  SiiOsdIsWindowTimeoutExpired( int winRef );
void    SiiOsdEnableOsdDisplay( bool_t enableDisplay );
bool_t  SiiOsdShowWindow( int winRef, SiiOsdOptions_t options );
bool_t  SiiOsdWriteText( int winRef, int row, int col, char *pStr, SiiOsdOptions_t options );
void    SiiOsdUpdateChars( uint8_t winRef, uint8_t row, uint8_t col, uint16_t numChars, uint8_t *pBuf );
bool_t  SiiOsdWriteIcon( int winRef, int row, int col, int rowCount, int colCount, uint8_t *pIcon, SiiOsdOptions_t options );
bool_t  SiiOsdMoveWindow( int winRef, int startX, int startY, SiiOsdOptions_t options );
bool_t  SiiOsdSetCursor( int winRef, int row, SiiOsdOptions_t options );
bool_t  SiiOsdCloseWindow( int winRef, SiiOsdOptions_t options );
bool_t  SiiOsdCloseAll( void );
bool_t  SiiOsdReplaceWindow( int winRef, int windowId );
int     SiiOsdCreateWindow( int windowId, int startX, int startY, int rowCount, int colCount, SiiOsdWinTypeData_t *pWinTypeData, SiiOsdOptions_t options );
int     SiiOsdAssignChildWinRef( int winRef, int childWinRef );

bool_t  SiiOsdFillWindow( int winRef, int resourceId );
int     SiiOsdGetCursor( int winRef );

// si_osd_handler.c

int     SiiOsdMoveCursor( int winRef, bool_t moveDown, SiiOsdOptions_t options );
bool_t  SiiOsdBaseWindowHandler( int winRef, int key, SiiOsdOptions_t options );
bool_t  SiiOsdSendMessage( int winRef, SiiOsdUserFunctionAction_t action, int key );
void    SiiOsdSetEditMode( int winRef, bool_t isEditMode );
int     SiiOsdIsEditMode( int winRef );

// Callback function prototypes
// Definition of the callback functions shall be done on the application level
bool_t  SiiOsdCbGetOutputRes( int zone, int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced );



#endif // __SI_OSD_COMPONENT_H__
