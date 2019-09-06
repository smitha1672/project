//***************************************************************************
//!file     si_device_config.h
//!brief    Sii9535 Device configuration file.
//!brief    Driver layer internal file
//          This file supplies the configuration of resources on the
//          Sii9535 device, including how many of each port, MHD channel,
//          ARC, HEC, etc.
//
//          This file is to be included in each driver-level module.  The
//          information contained in the file is propagated up to the
//          component level via driver API functions.
//
//          This file obtains some specific board implementation information
//          that may affect the configuration and usage of the actual device
//          resources via board API functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#ifndef __SI_DEVICECONFIG_H__
#define __SI_DEVICECONFIG_H__
#include "si_common.h"
#include "si_drv_msw_cfg.h"

//------------------------------------------------------------------------------
//  Compile-time build options
//------------------------------------------------------------------------------

#define USE_INTERNAL_MUTE       0       // Set to 1 to use SiI9535 muting
#define DO_EDID_INIT            1       // Set to 1 to perform EDID initialization during boot process
#define MHL_WORKAROUND          0       // Set to 1 to enable MHL workaround

#define IS_RX                   1       // Set to 1 to if code is for Sink device
#define IS_TX                   0       // Set to 1 to if code is for Source device


//------------------------------------------------------------------------------
//  Matrix Switch Component SiI9535-specific
//------------------------------------------------------------------------------

#define SWITCH_LINK_CALLBACK        ENABLE

#if ( configSII_DEV_953x_PORTING == 1 )
#if ( configSII_DEV_9535 == 1 )
#define INC_TX 1
#define INC_AUDIO_RX 1
#define INC_IV 1
#define INC_CBUS 1
#define INC_RTPI 1
#define INC_CDC 1
#define INC_CEC 0
#define INC_SETUP 1 
#define INC_IPV 1 
#define INC_OSD 0
#define INC_CBUS_WAKEUP 1
#define INC_ARC 1
#define INC_DIAG 1
#define INC_MSW 1
#define INC_THX 1
#else
#define INC_TX 1
#define INC_AUDIO_RX 1
#define INC_IV 1
#define INC_CBUS 1
#define INC_RTPI 1
#define INC_CDC 0
#define INC_CEC 1
#define INC_SETUP 1 
#define INC_IPV 1 
#define INC_OSD 0
#define INC_CBUS_WAKEUP 1
#define INC_ARC 1
#define INC_DIAG 1
#define INC_MSW 1
#define INC_THX 1
#define INC_CEC_SAC 1
#define INC_CEC_SWITCH 1


#endif 
#endif /*( configSII_DEV_953x_PORTING == 1 )*/




#endif // __SI_DEVICECONFIG_H__
