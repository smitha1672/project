//***************************************************************************
//!file     sk_app_osd_resource_data.c
//!brief    Local (code space) OSD resources.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "sk_app_osd.h"


uint8_t  SPEAKER_ICON [] = "\x89\x8A\x8B\x8C";
uint8_t  GEARS_ICON []   = "\x91\x92\x93\x94";
uint8_t  BOX_AND_GEAR_ICON [] = "\x8D\x8E\x8F\x90";

uint8_t  SII_LOGO [] = "\x7D\x7E\x81\x82\x85\x86\x7F\x80\x83\x84\x87\x88";

uint8_t  SPK_CENTER_SM []    = "\xA6\xa7";
uint8_t  SPK_CENTER_LG []    = "\xAB\xac";
uint8_t  SPK_WOOFER []       = "\xAA";
uint8_t  SPK_FRONT_SM []     = "\x00\x00\xA8\xA9";
uint8_t  SPK_FRONT_LG []     = "\x9F\xA0\xA1\xA2";
uint8_t  SPK_LEFT_SM []      = "\x99\x9B";
uint8_t  SPK_LEFT_LG []      = "\x99\x9A";
uint8_t  SPK_RIGHT_SM []     = "\x9C\x9E";
uint8_t  SPK_RIGHT_LG []     = "\x9C\x9D";
uint8_t  SPK_REAR_SM []      = "\xA3\xA5";
uint8_t  SPK_REAR_LG []      = "\xA3\xA4";

// g_userFuncs order must correspond to the Sii_UserFunctionId_t enums
const SiiOsdUserFunction_t    g_UserFuncs[SI_UF_TOTAL_COUNT] =
{
    NULL,
    &UserFuncEditBoxDemo,               // SI_UF_EDITBOX
    &UserFuncLeftMain,                  // SI_UF_MAINMENU
    &UserFuncBlankSub,                  // SI_UF_BLANKSUBMENU
    &UserFuncRightCrossover,            // SI_UF_CROSSOVER
    &UserFuncLeftSpeakerSetup,          // SI_UF_SPEAKERSETUP
    &UserFuncRightSpeakerConfig,        // SI_UF_SPEAKERCONFIG
    &UserFuncLeftSpeakerConfig,         // SI_UF_SPEAKERCONFIG_SUB
    &UserFuncVideoSetup,                // SI_UF_VIDEOSETUP
    &UserFuncVideoSetupIv,              // SI_UF_VIDEOSETUP_IV
    &UserFuncVolumeControl,             // SI_UF_VOLUME_CTRL
    &UserFuncProgressDemo,              // SI_UF_PROGRESS_DEMO
    &UserFuncListBoxDemo,               // SI_UF_LISTBOX_DEMO
    &UserFuncVideoSetupIvPatternSelect, // SI_UF_VIDEOSETUP_IV_PATTERN
    &UserFuncAudioSetup,                // SI_UF_AUDIOSETUP,
    &UserFuncOsdDemo,                   // SI_UF_OSDDEMO
    &UserFuncOsdAttributesDemo,         // SI_UF_OSDDEMO_ATTR
    &UserFuncDemoMessageBox,            // SI_UF_DEMO_MSGBOX
    &UserFuncInfoDisplay,               // SI_UF_INFO
    &UserFuncAudioControls,             // SI_UF_AUDIOSETUP_CONTROLS,

    // Local function index (as opposed to defined in si_osd_resource.h
    &UserFuncPortSelect,                // SI_UF_PORT_SEL,
    &UserFuncOptionSelect,              // SI_UF_PORT_SEL,
};


const SiiDrvOsdWindowResource_t osdResourceData[] =
{
    {       // Window LRID0_BLANK_WINDOW
        {OSD_RESOURCE_WINDOW, OSD_WINTYPE_BASIC, BYTEWORD(sizeof(SiiDrvOsdWindowResource_t))},
        BYTEWORD(LRID0_BLANK_WINDOW),   // wIdLo, wIdHi;
        BYTEWORD(0),                // windowStatusLo, windowStatusHi;
        BYTEWORD(0),                // optionsLo, optionsHi;
        BYTEWORD(140),              // horizStart;
        BYTEWORD(388),              // vertStart;
        16,                         // rowCount;
        16,                         // colCount;
        0,                          // dividerWidth;
        0,                          // borderWidth;
        0,                          // highlightRow;
        0,                          // disabledRowMapLo;
        0,                          // disabledRowMapHi;
        OSD_RGB(3,3,3),             // foreColor;
        OSD_RGB(0,0,0),             // backColor;
        OSD_RGB(0,0,0),             // foreColorHighlight;
        OSD_RGB(0,3,3),             // backColorHighlight;
        OSD_RGB(3,3,3),             // foreColorDisabled;
        OSD_RGB(0,0,0),             // backColorDisabled;
        OSD_RGB(3,3,3),             // borderColor;
        OSD_RGB(3,3,3),             // dividerColor;
        24,                         // alphaBlendLevel;
        BYTEWORD(0),                // msDisplayTimeLo, msDisplayTimeHi;
        SI_UF_BLANKSUBMENU,         // windowFunctionId;
        0,                          // param1; (min value)
        0,                          // param2; (max value)
        0,                          // param3; (step value)
        0,                          // param4;
        0,                          // editMode;
        {0},                        // rowChildWindowId [OSD_WIN_ROW_COUNT];
        {0},                        // contentIndexWordData[OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD];
    },
    {       // Window LRID1_PORT_SEL
        {OSD_RESOURCE_WINDOW, OSD_WINTYPE_MENU, BYTEWORD(sizeof(SiiDrvOsdWindowResource_t))},
        BYTEWORD(LRID1_PORT_SEL),   // wIdLo, wIdHi;
        BYTEWORD(0),                // windowStatusLo, windowStatusHi;
        BYTEWORD((OPTW_OSD_CURSOR | OPTW_WRAPCURSOR)),  // optionsLo, optionsHi;
        BYTEWORD(100),              // horizStart;
        BYTEWORD(100),              // vertStart;
        6,                          // rowCount;
        11,                         // colCount;
        0,                          // dividerWidth;
        1,                          // borderWidth;
        0,                          // highlightRow;
        0,                          // disabledRowMapLo;
        0,                          // disabledRowMapHi;
        OSD_RGB(3,3,3),             // foreColor;
        OSD_RGB(0,0,0),             // backColor;
        OSD_RGB(0,0,0),             // foreColorHighlight;
        OSD_RGB(0,3,3),             // backColorHighlight;
        OSD_RGB(1,1,1),             // foreColorDisabled;
        OSD_RGB(0,0,0),             // backColorDisabled;
        OSD_RGB(3,3,3),             // borderColor;
        OSD_RGB(3,3,3),             // dividerColor;
        24,                         // alphaBlendLevel;
        BYTEWORD(10000),            // msDisplayTimeLo, msDisplayTimeHi;
        SI_UF_PORT_SEL,             // windowFunctionId;
        0,                          // param1; (min value)
        0,                          // param2; (max value)
        0,                          // param3; (step value)
        0,                          // param4;
        0,                          // editMode;
        {0},                        // rowChildWindowId [OSD_WIN_ROW_COUNT];
        {0},                        // contentIndexWordData[OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD];
    },

    {
        {OSD_RESOURCE_WINDOW, OSD_WINTYPE_MENU, BYTEWORD(sizeof(SiiDrvOsdWindowResource_t))},
        BYTEWORD(LRID2_OPT_SEL),    // wIdLo, wIdHi;
        BYTEWORD(0),                // windowStatusLo, windowStatusHi;
        BYTEWORD(OPTW_OSD_CURSOR),  // optionsLo, optionsHi;
        0x8C,       // Horizontal Start Low
        0x00,       // Horizontal Start High
        0xC4,       // Vertical Start Low
        0x00,       // Vertical Start High
        0x08,       // Number of Rows.
        0x12,       // Number of Columns
        0x00,       // Row Divider
        0x00,       // Border Width
        0x00,       // High Lighted Row
        BYTEWORD(0xFFFC),   // Disabled Rows
        OSD_RGB(3,3,3),             // foreColor;
        OSD_RGB(0,0,0),             // backColor;
        OSD_RGB(0,0,0),             // foreColorHighlight;
        OSD_RGB(0,3,3),             // backColorHighlight;
        OSD_RGB(1,1,1),             // foreColorDisabled;
        OSD_RGB(0,0,0),             // backColorDisabled;
        OSD_RGB(3,3,3),             // borderColor;
        OSD_RGB(3,3,3),             // dividerColor;
        24,                         // alphaBlendLevel;
        BYTEWORD(0),                // msDisplayTimeLo, msDisplayTimeHi;
        SI_UF_OPT_SEL,              // windowFunctionId;
        0,                          // param1; (min value)
        0,                          // param2; (max value)
        0,                          // param3; (step value)
        0,                          // param4;
        0,                          // editMode;
        {0},                        // rowChildWindowId [OSD_WIN_ROW_COUNT];
        {0},                        // contentIndexWordData[OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD];
    },

    // Volume Control replacement for SPI Flash version
    {
        {OSD_RESOURCE_WINDOW, OSD_WINTYPE_SLIDER, BYTEWORD(sizeof(SiiDrvOsdWindowResource_t))},
        BYTEWORD(LRID3_VOL_CTRL),                   // wIdLo, wIdHi;
        BYTEWORD(0),                                // windowStatusLo, windowStatusHi;
        BYTEWORD((OPTW_4_TICK | OPTW_TEXT_RIGHT)),  // optionsLo, optionsHi;
        BYTEWORD(OSD_DEFAULT_FONT_WIDTH * 3),       // horizStart;
        BYTEWORD(OSD_DEFAULT_FONT_HEIGHT * 2),      // vertStart;
        4,                                          // Number of Rows.
        18,                                         // Number of Columns
        0,                                          // Row Divider
        1,                                          // Border Width
        0,                                          // High Lighted Row
        BYTEWORD(0xFFF0),                           // Disabled Rows
        OSD_RGB(3,3,3),             // foreColor;
        OSD_RGB(0,0,3),             // backColor;
        OSD_RGB(3,3,3),             // foreColorHighlight;
        OSD_RGB(0,0,3),             // backColorHighlight;
        OSD_RGB(3,3,3),             // foreColorDisabled;
        OSD_RGB(0,0,0),             // backColorDisabled;
        OSD_RGB(3,3,3),             // borderColor;
        OSD_RGB(3,3,3),             // dividerColor;
        28,                         // alphaBlendLevel;
        BYTEWORD(5000),             // msDisplayTimeLo, msDisplayTimeHi;
        SI_UF_VOLUME_CTRL,          // Window Function Id.
        0,                          // param1; (min value)
        100,                        // param2; (max value)
        10,                         // param3; (step value)
        0,                          // param4;
        0,                          // editMode;
        {0},                        // rowChildWindowId [OSD_WIN_ROW_COUNT];
        {0},                        // contentIndexWordData[OSD_CI_WINSIZE_SIZE * OSDRAM_BYTES_PER_WORD];
    },
};





