//***************************************************************************
//!file     si_cec_config.h
//!brief    System-specific configuration information for CEC
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_CEC_H__
#define __SI_CONFIG_CEC_H__

//------------------------------------------------------------------------------
//  CEC Component SiI953x-specific
//------------------------------------------------------------------------------

#define SII_NUM_CEC                 1   // Number of CEC instances
#define SII_NUM_CEC_SWITCH          1   // Number of CEC Switch component instances
#define SII_NUM_CEC_SAC             1   // Number of CEC System Audio Control component instances

/* Control for amount of text printed by CEC commands.  */

#define CEC_NO_TEXT         0
#define CEC_NO_NAMES        1
#define CEC_ALL_TEXT        2
#define INCLUDE_CEC_NAMES   CEC_NO_NAMES

typedef enum _SiiCecInstanceTypes_t
{
    CEC_INSTANCE_AVR    = 0,
#if 0
    CEC_INSTANCE_AUXTX  = 1
#endif
} SiiCecInstanceTypes_t;

typedef enum _SiiCpiInstanceTypes_t
{
    CPI_INSTANCE_AVR    = 0,
#if 0
    CPI_INSTANCE_AUXTX  = 1
#endif
} SiiCpiInstanceTypes_t;

#endif  //__SI_CONFIG_CEC_H__
