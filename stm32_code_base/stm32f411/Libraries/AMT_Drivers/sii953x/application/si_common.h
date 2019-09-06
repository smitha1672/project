//***************************************************************************
//!file     si_common.h
//!brief    Silicon Image common definitions header.
//
// No part of this work may be reproduced, modified, distributed, 
// transmitted, transcribed, or translated into any language or computer 
// format, in any form or by any means without written permission of 
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_COMMON_H__
#define __SI_COMMON_H__

#include "device_config.h"
#include "si_datatypes.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "si_device_config.h"
#else
#include "si_platform.h"
#endif 

#include "si_debug.h"


// Standard result codes are in the range of 0 - 4095
typedef enum _SiiResultCodes_t
{
    SII_SUCCESS      = 0,           // Success.
    SII_ERR_FAIL,                   // General failure.
    SII_ERR_INVALID_PARAMETER,      //
    SII_ERR_IN_USE,                 // Module already initialized.
    SII_ERR_NOT_AVAIL,              // Allocation of resources failed.
} SiiResultCodes_t;

#define ELAPSED_TIMER               0xFF
#define ELAPSED_TIMER1              0xFE
#define TIMER_0                     0   // DO NOT USE - reserved for TimerWait()
#define TIMER_1                     1
#define TIMER_2                     2
#define TIMER_3                     3
#define TIMER_4                     4

#define YES                         1
#define NO                          0

#if ( configSII_DEV_953x_PORTING == 0 )
//------------------------------------------------------------------------------
//  Basic system functions
//------------------------------------------------------------------------------

uint8_t SiiTimerExpired( uint8_t timer );
long    SiiTimerElapsed( uint8_t index );
void    SiiTimerSet( uint8_t index, uint16_t m_sec );
void    SiiTimerInit( void );
#endif 

#endif  // __SI_COMMON_H__
