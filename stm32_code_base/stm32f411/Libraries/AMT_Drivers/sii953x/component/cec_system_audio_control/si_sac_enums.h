//***************************************************************************
//! @file     si_sac_enums.h
//! @brief    HDMI/CEC Spec v1.4 System Audio Control command and operand codes
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#ifndef __SI_SAC_ENUMS_H__
#define __SI_SAC_ENUMS_H__

                                    //! Audio format codes for System Audio Control (defined in CEA-861-D)
typedef enum
{
    CEC_AUD_FMT_RES0    = 0, // Reserved
    CEC_AUD_FMT_LPCM    = 1, // Linear PCM (e.g., IEC 60958)
    CEC_AUD_FMT_AC3     = 2, // AC-3
    CEC_AUD_FMT_MPEG1   = 3, // MPEG1 (Layers 1 & 2)
    CEC_AUD_FMT_MP3     = 4, // MP3 (MPEG1 Layer 3)
    CEC_AUD_FMT_MPEG2   = 5, // MPEG2 (multichannel)
    CEC_AUD_FMT_AAC     = 6, // AAC
    CEC_AUD_FMT_DTS     = 7, // DTS
    CEC_AUD_FMT_ATRAC   = 8, // ATRAC
    CEC_AUD_FMT_ONEBIT  = 9, // One Bit Audio
    CEC_AUD_FMT_DDPLUS  = 10,// Dolby Digital +
    CEC_AUD_FMT_DTSHD   = 11,// DTS-HD
    CEC_AUD_FMT_MLP     = 12,// MAT (MLP)
    CEC_AUD_FMT_DST     = 13,// DST
    CEC_AUD_FMT_WMA     = 14,// WMA Pro
    CEC_AUD_FMT_RES15   = 15 // Reserved for audio format 15

} CecSacAudioFmtCode_t;


                                    //! CEC/SAC tasks FSM states
typedef enum
{
    CEC_SAC_TASK_EMPTY_STATE_NONE                         = 0x00,

    CEC_SAC_TASK_INITIATE_STATE_REQUEST_SOURCE            = 0x01,
    CEC_SAC_TASK_INITIATE_STATE_WAIT_SRC_REPLY            = 0x02,
#if ( configSII_DEV_953x_PORTING == 1 )
    CEC_SAC_TASK_INITIATE_STATE_REQUEST_SYS_AUD_MODE      = 0x03,
#else
    CEC_SAC_TASK_INITIATE_STATE_REQUEST_SYS_AUD_MODE      = 0x03,
#endif
    CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE          = 0x04,
    CEC_SAC_TASK_INITIATE_STATE_WAIT_SYS_AUD_MODE_CONFIRM = 0x05,           
    CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT     = 0x06,
    CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS            = 0x07,

#if ( configSII_DEV_953x_PORTING == 1 )
    CEC_SAC_TASK_INITIATE_WAIT_TV_RESPONSE      = 0x010,
#else
    CEC_SAC_TASK_TERMINATE_STATE_REQUEST_SYS_AUD_MODE     = 0x10,
#endif
    CEC_SAC_TASK_TERMINATE_STATE_WAIT_SYS_AUD_MODE_CONF   = 0x11,
    CEC_SAC_TASK_TERMINATE_STATE_REPORT_RESULTS           = 0x12,

    CEC_SAC_TASK_VOLUME_STATE_CTRL_PRESSED_SEND           = 0x20,
    CEC_SAC_TASK_VOLUME_STATE_WAIT_FOR_STATUS             = 0x21,
    CEC_SAC_TASK_VOLUME_STATE_CTRL_RELEASED_SEND          = 0x22,
    CEC_SAC_TASK_VOLUME_STATE_REPORT_RESULTS              = 0x23,

    CEC_SAC_TASK_DISCOVER_STATE_REQUEST_SAD               = 0x30,
    CEC_SAC_TASK_DISCOVER_STATE_WAIT_FOR_REPLY            = 0x31,
    CEC_SAC_TASK_DISCOVER_STATE_REPORT_RESULTS            = 0x32,

    CEC_SAC_TASK_ALLOC_LA_STATE_PING                      = 0x40,
    CEC_SAC_TASK_ALLOC_LA_STATE_WAIT_FOR_ACK              = 0x41,
    CEC_SAC_TASK_ALLOC_LA_STATE_REPORT_RESULTS            = 0x42,
    
} CecSacTaskState_t;

                                    //! mute command type
typedef enum
{
    CEC_SAC_MUTE_OFF    = 0,
    CEC_SAC_MUTE_ON     = 1,
    CEC_SAC_MUTE_TOGGLE = 2
    
} CecSacMuteCmd_t;
#endif // #define __SI_SAC_ENUMS_H__
