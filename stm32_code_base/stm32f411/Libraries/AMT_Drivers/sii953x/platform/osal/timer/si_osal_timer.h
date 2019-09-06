//***************************************************************************
//!file     si_osal_timer.h
//!brief    Starter Kit Operating System Adaptation Layer Timer functions.
//
// No part of this work may be reproduced, modified, distributed, 
// transmitted, transcribed, or translated into any language or computer 
// format, in any form or by any means without written permission of 
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_OSAL_TIMER_H__
#define __SI_OSAL_TIMER_H__
#include "si_common.h"

typedef struct _SiiTimer_t
{
    bool_t          enabled;
    bool_t          oneShot;
    clock_time_t    msDelay;
    clock_time_t    msStart;
} SiiTimer_t;

//-------------------------------------------------------------------------------------------------
//  inline functions
//-------------------------------------------------------------------------------------------------

inline clock_time_t SkTimeDiffMs ( clock_time_t t1, clock_time_t t2)
{
    if (t2 >= t1)
    {
        return (t2 - t1);
    }

    // overflow situation (t2 < t1)
    return( t1 + ((CLOCK_TIME_MAX - t2) + 1));
}

//------------------------------------------------------------------------------
// OS Timer functions
//------------------------------------------------------------------------------

void            SiiOsTimerSet( SiiTimer_t *pTimerInfo, clock_time_t delay  );
bool_t          SiiOsTimerExpired( SiiTimer_t timerInfo );
clock_time_t    SiiOsTimerElapsed( SiiTimer_t timerInfo );
void            SiiOsTimerWait( clock_time_t delay );
clock_time_t    SiiOsTimerTotalElapsed( void );

//------------------------------------------------------------------------------
// Legacy timer functions
//------------------------------------------------------------------------------

uint8_t SiiTimerExpired ( uint8_t timer );
long SiiTimerElapsed( uint8_t index );
void SiiTimerSet ( uint8_t index, uint16_t mSec );


#endif  // __SI_OSAL_TIMER_H__
