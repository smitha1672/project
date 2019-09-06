//***************************************************************************
//!file     si_cbus_config.h
//!brief    System-specific configuration information for MHL/CBUS
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_CBUS_H__
#define __SI_CONFIG_CBUS_H__

#include "device_config.h"

//------------------------------------------------------------------------------
//  CBUS Component 953x-specific
//------------------------------------------------------------------------------

#define MHL_MAX_CHANNELS    2           // Number of Mhl channels

#if ( configSII_DEV_953x_PORTING == 0 )
#define MHL_20           ENABLE     // MHL 2.0 features
#else
#define MHL_20           DISABLE     // MHL 2.0 features
#endif 



//------------------------------------------------------------------------------
//  The following constants define the CBUS Logical Device ID support
//  for each of the valid RCP Key IDs.  To add additional support, or (using |)
//  the desired logical ID code with the existing support.  Do NOT remove any
//  existing codes, as that will break MHL spec compliance.
//------------------------------------------------------------------------------

#define MHL_LD_NONE     (0x00)
#define MHL_LD_ALL      (0xFF)
#define MHL_LD_NUMBER   (MHL_LD_VIDEO | MHL_LD_AUDIO | MHL_LD_MEDIA | MHL_LD_TUNER)
#define MHL_LD_AV       (MHL_LD_VIDEO | MHL_LD_AUDIO)
#define MHL_LD_AV_REC   (MHL_LD_VIDEO | MHL_LD_AUDIO | MHL_LD_RECORD)
#define MHL_LD_AV_MEDIA (MHL_LD_VIDEO | MHL_LD_AUDIO | MHL_LD_MEDIA )
#define MHL_LD_NUMBER   (MHL_LD_VIDEO | MHL_LD_AUDIO | MHL_LD_MEDIA | MHL_LD_TUNER)


#define  LD_MHL_RCP_CMD_SELECT          MHL_LD_GUI
#define  LD_MHL_RCP_CMD_UP              MHL_LD_GUI
#define  LD_MHL_RCP_CMD_DOWN            MHL_LD_GUI
#define  LD_MHL_RCP_CMD_LEFT            MHL_LD_GUI
#define  LD_MHL_RCP_CMD_RIGHT           MHL_LD_GUI
#define  LD_MHL_RCP_CMD_RIGHT_UP        MHL_LD_NONE
#define  LD_MHL_RCP_CMD_RIGHT_DOWN      MHL_LD_NONE
#define  LD_MHL_RCP_CMD_LEFT_UP         MHL_LD_NONE
#define  LD_MHL_RCP_CMD_LEFT_DOWN       MHL_LD_NONE
#define  LD_MHL_RCP_CMD_ROOT_MENU       MHL_LD_GUI
#define  LD_MHL_RCP_CMD_SETUP_MENU      MHL_LD_NONE
#define  LD_MHL_RCP_CMD_CONTENTS_MENU   MHL_LD_NONE
#define  LD_MHL_RCP_CMD_FAVORITE_MENU   MHL_LD_NONE
#define  LD_MHL_RCP_CMD_EXIT            MHL_LD_GUI
#define  LD_MHL_RCP_CMD_NUM_0           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_1           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_2           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_3           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_4           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_5           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_6           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_7           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_8           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_NUM_9           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_DOT             MHL_LD_NONE
#define  LD_MHL_RCP_CMD_ENTER           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_CLEAR           MHL_LD_NUMBER
#define  LD_MHL_RCP_CMD_CH_UP           MHL_LD_TUNER
#define  LD_MHL_RCP_CMD_CH_DOWN         MHL_LD_TUNER
#define  LD_MHL_RCP_CMD_PRE_CH          MHL_LD_TUNER
#define  LD_MHL_RCP_CMD_SOUND_SELECT    MHL_LD_AUDIO
#define  LD_MHL_RCP_CMD_INPUT_SELECT    MHL_LD_NONE
#define  LD_MHL_RCP_CMD_SHOW_INFO       MHL_LD_NONE
#define  LD_MHL_RCP_CMD_HELP            MHL_LD_NONE
#define  LD_MHL_RCP_CMD_PAGE_UP         MHL_LD_NONE
#define  LD_MHL_RCP_CMD_PAGE_DOWN       MHL_LD_NONE
#define  LD_MHL_RAP_CONTENT_ON          MHL_LD_NONE
#define  LD_MHL_RCP_CMD_VOL_UP          MHL_LD_SPEAKER
#define  LD_MHL_RCP_CMD_VOL_DOWN        MHL_LD_SPEAKER
#define  LD_MHL_RCP_CMD_MUTE            MHL_LD_SPEAKER
#define  LD_MHL_RCP_CMD_PLAY            MHL_LD_AV
#define  LD_MHL_RCP_CMD_STOP            MHL_LD_AV_REC
#define  LD_MHL_RCP_CMD_PAUSE           MHL_LD_AV_REC
#define  LD_MHL_RCP_CMD_RECORD          MHL_LD_RECORD
#define  LD_MHL_RCP_CMD_REWIND          MHL_LD_AV
#define  LD_MHL_RCP_CMD_FAST_FWD        MHL_LD_AV
#define  LD_MHL_RCP_CMD_EJECT           MHL_LD_MEDIA
#define  LD_MHL_RCP_CMD_FWD             MHL_LD_AV_MEDIA
#define  LD_MHL_RCP_CMD_BKWD            MHL_LD_AV_MEDIA
#define  LD_MHL_RCP_CMD_ANGLE           MHL_LD_NONE
#define  LD_MHL_RCP_CMD_SUBPICTURE      MHL_LD_NONE
#define  LD_MHL_RCP_CMD_PLAY_FUNC       MHL_LD_AV
#define  LD_MHL_RCP_CMD_PAUSE_PLAY_FUNC MHL_LD_AV
#define  LD_MHL_RCP_CMD_RECORD_FUNC     MHL_LD_RECORD
#define  LD_MHL_RCP_CMD_PAUSE_REC_FUNC  MHL_LD_RECORD
#define  LD_MHL_RCP_CMD_STOP_FUNC       MHL_LD_AV_REC
#define  LD_MHL_RCP_CMD_MUTE_FUNC       MHL_LD_SPEAKER
#define  LD_MHL_RCP_CMD_UN_MUTE_FUNC    MHL_LD_SPEAKER
#define  LD_MHL_RCP_CMD_TUNE_FUNC       MHL_LD_NONE
#define  LD_MHL_RCP_CMD_MEDIA_FUNC      MHL_LD_NONE
#define  LD_MHL_RCP_CMD_F1              MHL_LD_NONE
#define  LD_MHL_RCP_CMD_F2              MHL_LD_NONE
#define  LD_MHL_RCP_CMD_F3              MHL_LD_NONE
#define  LD_MHL_RCP_CMD_F4              MHL_LD_NONE
#define  LD_MHL_RCP_CMD_F5              MHL_LD_NONE

#endif  //__SI_CONFIG_CBUS_H__
