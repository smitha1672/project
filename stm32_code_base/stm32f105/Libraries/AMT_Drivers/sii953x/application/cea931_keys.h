//***************************************************************************
//!file     cea931_keys.h
//!brief    CEA931 key codes (actually the MHL RCP subset at the moment)
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __CEA931_KEYS_H__
#define __CEA931_KEYS_H__

typedef enum _SiiCea931KeyIds_t
{
    CEA931_KEY_SELECT          = 0x00,
    CEA931_KEY_UP              = 0x01,
    CEA931_KEY_DOWN            = 0x02,
    CEA931_KEY_LEFT            = 0x03,
    CEA931_KEY_RIGHT           = 0x04,
    CEA931_KEY_RIGHT_UP        = 0x05,
    CEA931_KEY_RIGHT_DOWN      = 0x06,
    CEA931_KEY_LEFT_UP         = 0x07,
    CEA931_KEY_LEFT_DOWN       = 0x08,
    CEA931_KEY_ROOT_MENU       = 0x09,
    CEA931_KEY_SETUP_MENU      = 0x0A,
    CEA931_KEY_CONTENTS_MENU   = 0x0B,
    CEA931_KEY_FAVORITE_MENU   = 0x0C,
    CEA931_KEY_EXIT            = 0x0D,

    //0x0E - 0x1F are reserved

    CEA931_KEY_NUM_0           = 0x20,
    CEA931_KEY_NUM_1           = 0x21,
    CEA931_KEY_NUM_2           = 0x22,
    CEA931_KEY_NUM_3           = 0x23,
    CEA931_KEY_NUM_4           = 0x24,
    CEA931_KEY_NUM_5           = 0x25,
    CEA931_KEY_NUM_6           = 0x26,
    CEA931_KEY_NUM_7           = 0x27,
    CEA931_KEY_NUM_8           = 0x28,
    CEA931_KEY_NUM_9           = 0x29,

    CEA931_KEY_DOT             = 0x2A,
    CEA931_KEY_ENTER           = 0x2B,
    CEA931_KEY_CLEAR           = 0x2C,

    //0x2D - 0x2F are reserved

    CEA931_KEY_CH_UP           = 0x30,
    CEA931_KEY_CH_DOWN         = 0x31,
    CEA931_KEY_PRE_CH          = 0x32,
    CEA931_KEY_SOUND_SELECT    = 0x33,
    CEA931_KEY_INPUT_SELECT    = 0x34,
    CEA931_KEY_SHOW_INFO       = 0x35,
    CEA931_KEY_HELP            = 0x36,
    CEA931_KEY_PAGE_UP         = 0x37,
    CEA931_KEY_PAGE_DOWN       = 0x38,

    //0x39 - 0x40 are reserved

    CEA931_KEY_VOL_UP          = 0x41,
    CEA931_KEY_VOL_DOWN        = 0x42,
    CEA931_KEY_MUTE            = 0x43,
    CEA931_KEY_PLAY            = 0x44,
    CEA931_KEY_STOP            = 0x45,
    CEA931_KEY_PAUSE           = 0x46,
    CEA931_KEY_RECORD          = 0x47,
    CEA931_KEY_REWIND          = 0x48,
    CEA931_KEY_FAST_FWD        = 0x49,
    CEA931_KEY_EJECT           = 0x4A,
    CEA931_KEY_FWD             = 0x4B,
    CEA931_KEY_BKWD            = 0x4C,

    //0x4D - 0x4F are reserved

    CEA931_KEY_ANGLE            = 0x50,
    CEA931_KEY_SUBPICTURE       = 0x51,

    //0x52 - 0x5F are reserved

    CEA931_KEY_PLAY_FUNC       = 0x60,
    CEA931_KEY_PAUSE_PLAY_FUNC = 0x61,
    CEA931_KEY_RECORD_FUNC     = 0x62,
    CEA931_KEY_PAUSE_REC_FUNC  = 0x63,
    CEA931_KEY_STOP_FUNC       = 0x64,
    CEA931_KEY_MUTE_FUNC       = 0x65,
    CEA931_KEY_UN_MUTE_FUNC    = 0x66,
    CEA931_KEY_TUNE_FUNC       = 0x67,
    CEA931_KEY_MEDIA_FUNC      = 0x68,

    CEA931_KEY_AV_IN_SELECT_FUNC    = 0x69,
    CEA931_KEY_AUDIO_IN_SELECT_FUNC = 0x6A,
    CEA931_KEY_POWER_TOGGLE_FUNC    = 0x6B,
    CEA931_KEY_POWER_OFF_FUNC       = 0x6C,
    CEA931_KEY_POWER_ON_FUNC        = 0x6D,

    //0x69 - 0x70 are reserved

    CEA931_KEY_F1              = 0x71,
    CEA931_KEY_F2              = 0x72,
    CEA931_KEY_F3              = 0x73,
    CEA931_KEY_F4              = 0x74,
    CEA931_KEY_F5              = 0x75,

    //0x76 - 0x7D are reserved

    CEA931_KEY_VS              = 0x7E,
    CEA931_KEY_RSVD            = 0x7F,

} SiiCea931KeyIds_t;

#endif // __CEA931_KEYS_H__
