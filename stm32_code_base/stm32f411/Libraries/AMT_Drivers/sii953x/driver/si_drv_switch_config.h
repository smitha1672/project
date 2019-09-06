//***************************************************************************
//!file     si_drv_switch_config.h
//!brief    System-specific configuration information for SWITCH driver
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_DRV_SWITCH_H__
#define __SI_CONFIG_DRV_SWITCH_H__

#include "device_config.h"
//------------------------------------------------------------------------------
//  SWITCH Driver SiI9535-specific
//------------------------------------------------------------------------------

#define SII_NUM_SWITCH          1   // Number of SWITCH instances

#if ( configSII_DEV_9535 == 1 )
#define SII_INPUT_PORT_COUNT    4
#else
#define SII_INPUT_PORT_COUNT    3
#endif
#define SII_OUTPUT_PORT_COUNT   1
#define SII_ARC_PORT_PATH       1

#endif  //__SI_CONFIG_DRV_SWITCH_H__
