//***************************************************************************
//! @file     sk_app_osd.h
//! @brief
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************
#if !defined __SK_APP_OSD_H__
#define __SK_APP_OSD_H__

#include "sk_application.h"
#include "si_osd_component.h"
#include "si_drv_osd_config.h"
#include "si_drv_osd.h"
#include "si_osd_resource.h"

//------------------------------------------------------------------------------
// Window constants
//------------------------------------------------------------------------------

#define BYTEWORD(n)         (uint8_t)(n & 0x00FF),(uint8_t)((n & 0xFF00) >> 8)
#define MAKEINT( lo, hi )   (((int)(hi << 8) & 0xFF00) | lo )

#define OSD_INFO_COLS           32
#define OSD_INFO_ROWS           4

#define KH_NONE                 0
#define KH_TOP_KEYS             1
#define KH_SUB_KEYS             2
#define KH_SUB1_KEYS            3
#define KH_MOD_KEYS             4
#define KH_MOD1_KEYS            5
#define KH_EDIT_KEYS            6

#define HLP_NONE                -1

//------------------------------------------------------------------------------
// OSD Application data structure
//------------------------------------------------------------------------------

typedef struct _SiiosdAppDataStruct_t
{
    // Composite Menu Window (composed of four separate windows)
    int             winRefLeft;             // Left (main) window
    int             winRefTop;              // Top (menu title/path)
    int             winRefRight;            // Right (sub-menu, if needed)
    int             winRefBottom;           // Bottom (context-sensitive help)
    bool_t          contextHelpOpen;        // true if context help window is open

    char            itemString[256];

    bool_t          speakerConfigActive;

    // Video Setup window
    int             videoTypeSelect;        // External/Internal
    int             ivFormatSelect;         // Internal video format selection
    int             ivPatternSelect;        // Internal video pattern selection
    int             videoMute;

    // Remember cursor locations
    int             cursorLeftMain;
    int             cursorOsdDemo;
    int             cursorAudioSetup;
    int             cursorLeftSpeakerSetup;
    int             cursorOptionsMenu;
} SiiOsdAppDataStruct_t;
extern SiiOsdAppDataStruct_t osdApp;

typedef struct _SiiInfoWinStruct_t
{
    int             winRef;
    clock_time_t    clockSetMs;             // ms value of midnight on the day the clock was set.
    bool_t          twentyFourHourClock;    // Clock display mode
} SiiInfoWinStruct_t;
extern SiiInfoWinStruct_t infoWin;

typedef struct _SiiVolInfoStruct_t
{
    int             winRef;
    int             speakerVolume;
} SiiVolInfoStruct_t;
extern SiiVolInfoStruct_t volInfo;

typedef struct _SiiPortWinStruct_t
{
    int             winRef;
    int             winRef1;
    int             winRef2;
    int             port1Row;
} SiiPortWinStruct_t;
extern SiiPortWinStruct_t portWin;

typedef struct _SiiOptWinStruct_t
{
    int             winRef;
    bool_t          ipvAllowed;
    bool_t          ipvHighlight;
} SiiOptWinStruct_t;
extern SiiOptWinStruct_t optWin;

//------------------------------------------------------------------------------
// Resource Data
//------------------------------------------------------------------------------

typedef enum
{
    LRID0_BLANK_WINDOW      = OSD_RESOURCE_LOCAL_FLAG,  // Start of internal resource IDs
    LRID1_PORT_SEL,
    LRID2_OPT_SEL,
    LRID3_VOL_CTRL,

    OSD_RSRC_COUNT
} SiiLocalResource_t;

#define OSD_LOCAL_RESOURCE_COUNT    (OSD_RSRC_COUNT - OSD_RESOURCE_LOCAL_FLAG) // Actual count of local resources

typedef enum
{
    SI_UF_PORT_SEL          = SI_UF_EXTERNAL_COUNT, // Start where the internal list left off
    SI_UF_OPT_SEL,

    SI_UF_TOTAL_COUNT
} Sii_LocalUserFunctionId_t;

extern uint8_t  SPEAKER_ICON [];
extern uint8_t  GEARS_ICON [];
extern uint8_t  BOX_AND_GEAR_ICON [];

extern uint8_t  SII_LOGO [];

extern uint8_t  SPK_CENTER_SM [];
extern uint8_t  SPK_CENTER_LG [];
extern uint8_t  SPK_WOOFER [];
extern uint8_t  SPK_FRONT_SM [];
extern uint8_t  SPK_FRONT_LG [];
extern uint8_t  SPK_LEFT_SM [];
extern uint8_t  SPK_LEFT_LG [];
extern uint8_t  SPK_RIGHT_SM [];
extern uint8_t  SPK_RIGHT_LG [];
extern uint8_t  SPK_REAR_SM [];
extern uint8_t  SPK_REAR_LG [];

extern char *editHelpText [];

extern const SiiDrvOsdWindowResource_t osdResourceData[];
extern const SiiOsdUserFunction_t    g_UserFuncs[SI_UF_TOTAL_COUNT];

//------------------------------------------------------------------------------
// External Data
//------------------------------------------------------------------------------

// sk_app_osd_avr.c
void    UpdateMenuHelp( char *pHelpText[], int keyHelpIndex, int helpIndex );

void    SkAppOsdCloseMainMenu( void );
void    SkAppOsdMainMenu( void );
void    DoOsdProgress( void );
void    DoOsdListBoxDemo( void );
void    DoOsdEditDemo( void );
void    SkAppOsdMessage( char *pMessage );
void    SkAppOsdInitializeDemo( void );

bool_t  SkAppOsdRestoreMainMenu( int winRef, int childWinRef );
void    SkAppOsdMoveDotMarker( int winRef, int oldRow, int newRow );

// sk_app_osd.c
void    SkAppOsdErrorDialog( char *pMessage );
void    SkAppOsdmessage( char *pTitle, char *pMessage, int msTimeout );
void    SkAppOsdMenuUpdate( void );
bool_t  SkAppOsdIrRemote( RC5Commands_t key, bool_t fullAccess );
void    SkAppTaskOsd( void );
bool_t  SkAppDeviceInitOsd( void );

// sk_app_osd_info.c
bool_t  UserFuncInfoDisplay( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
void    SkAppOsdShowInfo( void );

// sk_app_osd_video.c
bool_t UserFuncVideoSetup( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t UserFuncVideoSetupIv( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t UserFuncVideoSetupIvPatternSelect( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );

// sk_app_osd_demo.c
bool_t  UserFuncDemoMessageBox( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncOsdDemo( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncProgressDemo( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncEditBoxDemo( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncOsdAttributesDemo( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
void    SkAppOsdRestoreOsdDemoMenu( int winRef );

// sk_app_osd_volume.c
bool_t  UserFuncVolumeControl( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
void    SkAppOsdVolumeControl( int key );
void    SkAppOsdVolumeUpdate( int newVolume );

// sk_app_osd_portsel.c
bool_t  UserFuncPortSelect( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
void    SkAppOsdPortSelect( void );
void    SkAppOsdPortId( void );

// sk_app_osd_options.c
bool_t  UserFuncOptionSelect( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
void    SkAppOsdOptionSelect( void );

//------------------------------------------------------------------------------
// sk_app_osd_lb_demo.c
//------------------------------------------------------------------------------
#define LISTBOX_H   200
#define LISTBOX_V   100

#define LISTBOX1_ROWS           6
#define LISTBOX1_COLS           18
#define LISTBOX1_ITEM_MAX       10
#define LISTBOX1_ITEM_LENGTH    25  // Item string can be longer than column count

extern  int winRefList;

bool_t  UserFuncBlankSub( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncLeftMain( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncListBoxDemo( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
void    DoOsdListBoxDemo( void );

//------------------------------------------------------------------------------
// sk_app_osd_speaker.c
//------------------------------------------------------------------------------

bool_t  UserFuncRightSpeakerConfig( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncLeftSpeakerConfig( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncLeftSpeakerSetup( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncRightCrossover( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
void    SkAppOsdInitializeSpeakerSetupDemo( void );
void    SkAppOsdResetSpeakerSetupDemo ( void );


//------------------------------------------------------------------------------
// sk_app_osd_audio.c
//------------------------------------------------------------------------------

bool_t  UserFuncAudioControls( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );
bool_t  UserFuncAudioSetup( int winRef, SiiOsdUserFunctionAction_t action, uint8_t *pArgs );

#endif
