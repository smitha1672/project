//***************************************************************************
//!file     si_drv_osd_config.h
//!brief    System-specific configuration information for OSD driver
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_DRV_OSD_H__
#define __SI_CONFIG_DRV_OSD_H__


//------------------------------------------------------------------------------
//  OSD Driver SiI9535-specific
//------------------------------------------------------------------------------


#define WIN_X_ALIGN_MASK            0x0007  // Align on 8 pixels
#define WIN_Y_ALIGN_MASK            0x0001  // Align on even rows


#define OSD_CI_WINSIZE_SIZE         64      // Number of OSD RAM Words for each window content index array.
                                            // multiply by 4 to get the number of characters for each window, which
                                            // of course must be divided into x number of rows * y number of columns.
#define OSD_CI_ICONSIZE_SIZE        4       // Number of OSD RAM Words for each icon resource. Multiply by 4
                                            // to get the maximum number of characters for an icon, which must be
                                            // divided into x number of rows * y number of columns.
#define OSD_CI_TEXTSIZE_SIZE        64      // Maximum number of OSD RAM Words for each text resource. Multiply by 4
                                            // to get the the number of characters for a text string.

#define OSD_BITMAP_OFFSET           0       // Location in OSD RAM of bitmap segment
#define OSD_CONTENT_INDEX_1_OFFSET  1536    // Location in OSD RAM of first char index segment
// Location in OSD RAM of second char index segment
#define OSD_CONTENT_INDEX_2_OFFSET  (OSD_CONTENT_INDEX_1_OFFSET + (OSD_CI_WINSIZE_SIZE * SII_OSD_WIN_COUNT))

#define OSD_WIN_ROW_COUNT_MAX_COL   8       // Maximum row count if using full OSD_WIN_COL_COUNT

#define OSD_RESOURCE_LIMIT          128     // Number of external (flash) resources allowed

#define OSD_RGB( r, g, b )          ((( r & 0x03) << 4 ) | (( g & 0x03) << 2 ) | ( b & 0x03 ))

// System message colors
#define SYSTEM_MSG_TITLE_FG         OSD_RGB( 0,0,3 )
#define SYSTEM_MSG_TITLE_BG         OSD_RGB( 3,3,3 )
#define SYSTEM_MSG_BORDER           OSD_RGB( 0,0,3 )


#define OSD_DEFAULT_BORDERWIDTH         1
#define OSD_DEFAULT_DIVIDERWIDTH        0
#define OSD_DEFAULT_FG_COLOR            OSD_RGB( 3, 3, 3 )  // foreColor;
#define OSD_DEFAULT_BG_COLOR            OSD_RGB( 0, 0, 0 )  // backColor;
#define OSD_DEFAULT_FG_HIGHLIGHT_COLOR  OSD_RGB( 0, 3, 3 )  // foreColorHighlight;
#define OSD_DEFAULT_BG_HIGHLIGHT_COLOR  OSD_RGB( 3, 3, 0 )  // backColorHighlight;
#define OSD_DEFAULT_FG_DISABLED_COLOR   OSD_RGB( 2, 2, 2 )  // foreColorDisabled;
#define OSD_DEFAULT_BG_DISABLED_COLOR   OSD_RGB( 0, 0, 0 )  // backColorDisabled;
#define OSD_DEFAULT_BORDER_COLOR        OSD_RGB( 3, 3, 3 )  // borderColor;
#define OSD_DEFAULT_DIVIDER_COLOR       OSD_RGB( 3, 3, 3 )  // dividerColor;
#define OSD_DEFAULT_ALPHA_BLEND         0                   // alphaBlendLevel;
#define OSD_DEFAULT_TIMEOUT             0                   // msTimeout;


//------------------------------------------------------------------------------
//  OSD Font - specific character values.
//------------------------------------------------------------------------------
typedef enum
    {
    SII_OSD_CHAR_ILLEGAL_CHAR       = 0x5F  + ' ',  // Add 0x20 so that font char translation works properly
    SII_OSD_CHAR_CRLF               = 0x60  + ' ',
    SII_OSD_CHAR_ELLIPSIS           = 0x61  + ' ',
    SII_OSD_CHAR_YES                = 0x62  + ' ',
    SII_OSD_CHAR_NO                 = 0x63  + ' ',
    SII_OSD_EDIT_OK_CHAR            = 0x64  + ' ',  // Edit OK character (finished edit)
    SII_OSD_EDIT_UC_CHAR            = 0x65  + ' ',  // Edit Upper case select character
    SII_OSD_EDIT_LC_CHAR            = 0x66  + ' ',  // Edit Lower case select character
    SII_OSD_EDIT_CLR_CHAR           = 0x67  + ' ',  // Edit CLR character (clear edit text string)
    SII_OSD_EDIT_SPC_CHAR           = 0x68  + ' ',  // Edit SPC character (space)
    SII_OSD_EDIT_DEL_CHAR           = 0x69  + ' ',  // Edit DEL character (backspace)
    SII_OSD_EDIT_INS_CHAR           = 0x6A  + ' ',  // Edit INS character (insert)
    SII_OSD_CHAR_DOT_MARKER         = 0x6B  + ' ',
    SII_OSD_CHAR_UP                 = 0x6C  + ' ',
    SII_OSD_CHAR_DOWN               = 0x6D  + ' ',
    SII_OSD_CHAR_LEFT               = 0x6E  + ' ',
    SII_OSD_CHAR_RIGHT              = 0x6F  + ' ',
    SII_OSD_CHAR_SLIDER_CAP_LEFT    = 0x70  + ' ',
    SII_OSD_CHAR_SLIDER_CAP_RIGHT   = 0x71  + ' ',
    SII_OSD_CHAR_SLIDER_BAR         = 0x72  + ' ',
    SII_OSD_CHAR_PROGRESS_1QUARTER  = 0x73  + ' ',
    SII_OSD_CHAR_PROGRESS_HALF      = 0x74  + ' ',
    SII_OSD_CHAR_PROGRESS_3QUARTER  = 0x75  + ' ',
    SII_OSD_CHAR_PROGRESS_FULL      = 0x76  + ' ',
    SII_OSD_CHAR_THUMB_1QUARTER     = 0x77  + ' ',
    SII_OSD_CHAR_THUMB_HALF         = 0x78  + ' ',
    SII_OSD_CHAR_THUMB_3QUARTER     = 0x79  + ' ',
    SII_OSD_CHAR_THUMB_4QUARTER     = 0x7A  + ' ',
    SII_OSD_CHAR_CHECKBOX_EMPTY     = 0x7B  + ' ',
    SII_OSD_CHAR_CHECKBOX_CHECKED   = 0x7C  + ' ',

    SII_OSD_CHAR_CANCEL             = 0xAD  + ' ',

    SII_OSD_INVERT_CHAR             = 0xBE  + ' ',  // Location in Bitmap segment used by invert character function
    SII_OSD_BOX_CHAR                = 0xBF  + ' ',  // Location in Bitmap segment used by box character function
} SiiOsdIconCharacters_t;


#endif  //__SI_CONFIG_DRV_CEC_H__
