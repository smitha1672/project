//*************************************************************************************************
//! @file    si_cec_common.c
//! @brief   Internal CEC timer's service routines
//
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#include "si_cec_timer.h"

static uint16_t *pSysTimeCountCurMs;        //!< pointer to current time
static uint16_t *pSysTimeCountPrvMs;        //!< pointer to previous time reading

//-------------------------------------------------------------------------------------------------
//! @brief      Updates the system time related variables.
//!
//!             Shall be called from TaskProcess() functions before any manipulations with CEC timers
//!
//! @param[in]  sysTimeCountMs    - delay time to set (in ticks)
//! @param[in]  pSysTimeCntCurMs  - pointer to current time value (public member of an instance record)
//! @param[in]  pSysTimeCntPrvMs  - pointer to previously recorded time value
//!                                 (public member of an instance record)
//-------------------------------------------------------------------------------------------------

void CecSysTimeMsSet(uint16_t sysTimeCountMs, uint16_t *pSysTimeCntCurMs, uint16_t *pSysTimeCntPrvMs)
{
    pSysTimeCountCurMs = pSysTimeCntCurMs;
    pSysTimeCountPrvMs = pSysTimeCntPrvMs;

    *pSysTimeCountPrvMs = *pSysTimeCountCurMs;    // memorize previous timer reading
    *pSysTimeCountCurMs = sysTimeCountMs;         // set current timer reading
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sets the time counter to given time in the future relatively
//!             to provided system timer reading.
//!
//! @note       CAUTION! This function shall never be followed by the
//!             CecTimeCounterUpdate() for the same timer in one
//!             TaskProcess invocation
//!
//! @param[in]  delayTimeTck   - delay time to set (in ticks)
//! @param[in]  pTimeCounter   - pointer to the time counter to set
//-------------------------------------------------------------------------------------------------

void CecTimeCounterSet(const uint16_t delayTimeTck, CecTimeCounter_t *pTimeCounter)
{
    uint32_t totalTimeMs;

    totalTimeMs = (uint32_t) (*pSysTimeCountCurMs) + CEC_TIME_TCK_MS * ((uint32_t) delayTimeTck);
    pTimeCounter->tRov = (uint16_t)((totalTimeMs / 0x10000) & 0x3)+1;  // 0x10000 = 2^16	//Add "+1" by Elvis
    pTimeCounter->tRes = (uint16_t)((totalTimeMs / CEC_TIME_TCK_MS) & 0x3FFF); // mod (2^16)
}


//-------------------------------------------------------------------------------------------------
//! @brief      Checks if the time counter has expired and updates the timer counter.
//!             Resets the time counter if it is expired
//!             CAUTION: this function shall never be called more than once per
//!             TaskProcess invocation for the same timer
//!
//! @param[in]  pTimeCounter   - pointer to the time counter to check
//! @param[in]  pRecallTimer   - pointer to the next call timer; if zero, recall timer will not be updated
//!
//! @retval     true - if timer is expired
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CecTimeCounterUpdate(CecTimeCounter_t *pTimeCounter, CecTimeCounter_t *pRecallTimer)
{
    bool_t isExpired = false;

    if ((pTimeCounter->tRov != 0) || (pTimeCounter->tRes != 0)) // if not expired already
    {
        if (*pSysTimeCountPrvMs > *pSysTimeCountCurMs) // if timer overflow detected
        {
            if (pTimeCounter->tRov > 0)
            {
                pTimeCounter->tRov--;
            }
	    if ( pTimeCounter->tRov == 0 )	// Add by Elvis
	    {
                isExpired = true;
                pTimeCounter->tRes = 0;
	    }
        }

        if ((pTimeCounter->tRov == 1) && (*pSysTimeCountCurMs > (CEC_TIME_TCK_MS * pTimeCounter->tRes)))	// Modified by Elvis (change 0 to 1)
        {
            isExpired = true;

            // reset the counter
            pTimeCounter->tRov = 0;
            pTimeCounter->tRes = 0;
        }
    }
    else
    {
        isExpired = true;
    }

    // Update minimum time counter
    if (pRecallTimer != 0)
    {
        if (CecTimeCounterCompare(pTimeCounter, pRecallTimer) == 0)
        {   // if (*pTimeCounter < *pRecallTimer)
            *pRecallTimer = *pTimeCounter;
        }
    }

    return isExpired;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Checks if the time counter has expired
//!
//! @param[in]  pTimeCounter - pointer to the time counter to check
//!
//! @retval     true  - if timer is expired
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CecIsTimeCounterExpired(CecTimeCounter_t *pTimeCounter)
{
    return ((pTimeCounter->tRov == 0) && (pTimeCounter->tRes == 0));
}


//-------------------------------------------------------------------------------------------------
//! @brief      Resets the time counter to zero
//!
//! @param[in]  pTimeCounter - pointer to the time counter to check
//-------------------------------------------------------------------------------------------------

void CecTimeCounterReset(CecTimeCounter_t *pTimeCounter)
{
    pTimeCounter->tRov = 0;
    pTimeCounter->tRes = 0;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Compares two time counters
//!
//!             CAUTION: time counters shall be updated before comparison
//!
//! @param[in]  pTimeCounterA - pointer to the time counter A to compare
//! @param[in]  pTimeCounterB - pointer to the time counter B to compare
//!
//! @retval     0 - if time A < B,
//! @retval     1 - if time A == B
//! @retval     2 - if time A > B
//-------------------------------------------------------------------------------------------------

uint8_t CecTimeCounterCompare(CecTimeCounter_t *pTimeCounterA, CecTimeCounter_t *pTimeCounterB)
{
    uint8_t cmpRet;

    if (pTimeCounterA->tRov > pTimeCounterB->tRov)
    {
        cmpRet = 2;
    }
    else if (pTimeCounterA->tRov < pTimeCounterB->tRov)
    {
        cmpRet = 0;
    }
    else if (pTimeCounterA->tRes > pTimeCounterB->tRes)
    {
        cmpRet = 2;
    }
    else if (pTimeCounterA->tRes < pTimeCounterB->tRes)
    {
        cmpRet = 0;
    }
    else
    {
        cmpRet = 1;
    }

    return cmpRet;

}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns time left until counter expires in seconds
//!
//! @param[in]  pTimeCounter   - pointer to the time counter
//!
//! @return     time in seconds
//-------------------------------------------------------------------------------------------------

uint16_t CecTimeCounterLeftSecGet(CecTimeCounter_t *pTimeCounter)
{
    return ((pTimeCounter->tRov * (0x10000 / CEC_TIME_TCK_MS) + pTimeCounter->tRes -
            (*pSysTimeCountCurMs / CEC_TIME_TCK_MS)) / (1000 / CEC_TIME_TCK_MS));
}
