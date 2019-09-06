//*************************************************************************************************
//! @file     si_cec_timer.h
//! @brief    Internal CEC timer's service routines
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/
#ifndef __SI_CEC_TIMER_H__
#define __SI_CEC_TIMER_H__

#ifdef CEC_PC_SIMULATION
#include "si_datatypes.h"
#else
#include "si_common.h"
#endif

// Time related definitions
#define CEC_TIME_TCK_MS                 4                               //!< number of milliseconds in one tick (definition of the "tick")
#define CEC_TIME_MS2TCK(t)              ((t) / CEC_TIME_TCK_MS)         //!< Converts time in ms to number of counter ticks
#define CEC_TIME_SEC2TCK(t)             ((t) * (1000 / CEC_TIME_TCK_MS))//!< Converts time in seconds to number of counter ticks
#define CEC_TIME_TCK2SEC(t)             ((t) / (1000 / CEC_TIME_TCK_MS))//!< Converts time in number of counter ticks to seconds


//! Time counter structure

typedef struct
{
    bit_fld16_t  tRov : 2;    //!< number system timer overflow events ( = log_2(CEC_TIME_TCK_MS))
    bit_fld16_t  tRes : 14;   //!< residual time count (in ticks)      ( = 16 - log_2(CEC_TIME_TCK_MS))

} CecTimeCounter_t;


//-------------------------------------------------------------------------------------------------
// Timer's function prototypes
//-------------------------------------------------------------------------------------------------

void     CecSysTimeMsSet(uint16_t sysTimeCountMs, uint16_t *pSysTimeCntCurMs, uint16_t *pSysTimeCntPrvMs);
void     CecTimeCounterSet(const uint16_t delayTimeTck, CecTimeCounter_t *pTimeCounter);
bool_t   CecTimeCounterUpdate(CecTimeCounter_t *pTimeCounter, CecTimeCounter_t *pRecallTimer);
bool_t   CecIsTimeCounterExpired(CecTimeCounter_t *pTimeCounter);
void     CecTimeCounterReset(CecTimeCounter_t *pTimeCounter);
uint8_t  CecTimeCounterCompare(CecTimeCounter_t *pTimeCounterA, CecTimeCounter_t *pTimeCounterB);
uint16_t CecTimeCounterLeftSecGet(CecTimeCounter_t *pTimeCounter);

#endif // __SI_CEC_TIMER_H__
