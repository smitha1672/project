//***************************************************************************
//!file     si_timer.h
//!brief    Platform timer system, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_osal_timer.h"

//------------------------------------------------------------------------------
// Defines for setting up the system clock.
//------------------------------------------------------------------------------
//
// Define how many clock ticks in one second.
// Note:  This should match the value of SYSTICKHZ in the main program.
//
#define CLOCK_CONF_SECOND       1000         // 100 == 10ms

#define SYSTICKHZ               CLOCK_CONF_SECOND
#define SYSTICKMS               (1000 / SYSTICKHZ)      // Number of ms per SYSTICK
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)


#define ELAPSED_TIMER               0xFF
#define ELAPSED_TIMER1              0xFE
#define TIMER_0                     0   // DO NOT USE - reserved for TimerWait()
#define TIMER_1                     1
#define TIMER_2                     2
#define TIMER_3                     3
#define TIMER_4                     4
#define TIMER_COUNT                 5

uint8_t      SiiPlatformTimerExpired ( uint8_t timer );
uint16_t     SiiPlatformTimerElapsed ( uint8_t index );
void         SiiPlatformTimerWait ( uint16_t m_sec );
void         SiiPlatformTimerSet ( uint8_t index, uint16_t m_sec );
void         SiiPlatformTimerInit (void);
clock_time_t SiiPlatformTimerSysTicksGet (void);

//------------------------------------------------------------------------------
// Timer h/w interrupt handler.
//------------------------------------------------------------------------------
#if ( configRTOS_TIMER == 0 )
void SiiPlatformTimerSysTickIntHandler (void);
#endif 

