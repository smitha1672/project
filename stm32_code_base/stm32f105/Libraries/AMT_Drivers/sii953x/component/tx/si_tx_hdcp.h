//************************************************************************************************
//! @file   si_tx_hdcp.h
//! @brief  HDCP related methods
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_HDCP_H
#define SI_TX_HDCP_H

// Time between Part1 attempts in repeater mode
// (should be significantly less then TMO_MS_HDCP__MAX_AUTH_ATTEMPT)
#define TIM_MS_HDCP__RP_PART1_RETRY             100

// time between Part1 attempts in "Repeater" mode on TX Deep Color change
#define TIM_MS_HDCP__DS_PART1_RETRY             1000

// This time out starts when authentication begins after the request.
// If authentication cannot be finished within this timeout,
// the upstream device gets informed about the failure and
// downstream re-authentications stops until
//  a new request comes.
// It's OK to have it >5s. Downstream authentication attempts
// may be independent on upstream authentication.
// But if upstream pulses TMDS, downstream will restart the whole
// routine and it will be still OK.
#define TMO_MS_HDCP__MAX_AUTH_ATTEMPT           10000

// During this time the HDCP state machine waits for DDC ACK.
// If there is no ACK during this time, the authentication
// attempt is considered as failed.
#define TMO_MS_HDCP__MAX_DDC_NACK               1000

// Minimum time after enabling TMDS output before
// unmuting.
// During this time CTL3 pulses are not sent out and
// therefore there is no encryption even after
// the authentication has been successfully finished.
// This gating period improves compatibility with some incompliant
// sinks that may skip Ri updating within first few hundred milliseconds
// after sync detection.
// This delay applies for incompatible downstream devices,
// but may be omitted for the good ones.
#define TIM_MS_HDCP__MIN_MUTE_TIME              1000

// A delay of HDCP start after turning TMDS ON.
// Many receivers are not capable of HDCP authentication for some time
// after they started receiving TMDS signal. This is because they
// have to perform some operations before they are ready for
// te authentication.
// Such devices often do not disable HDCP DDC to indicate they are
// not ready (because the disabling is causing even more
// compatibility issues).
// Unfortunately, TIM_MS_HDCP__TMDS_ON_DELAY_XXX have to be small
// enough to pass the AKSV propagation test (100ms)
// with some test equipment that starts HDCP right after TMDS enabling.
#define TIM_MS_HDCP__TMDS_ON_DELAY_MIN          20
#define TIM_MS_HDCP__TMDS_ON_DELAY_MAX          40


// Maximum "HDCP Part 2 done" waiting time before timeout in 0-KSV case
#define TMO_MS_HDCP__ZERO_KSV_READY_MAX         25

// Default time for HDMI timer handler TxHdcpTimerHandler() calling
#define TIM_MS_HDCP__HANDLER_DEFAULT_INVOCATION 30

// TxHdcpTimerHandler() invocation time for critical phases.
#define TIM_MS_HDCP__HANDLER_URGENT_INVOCATION  10


//! @defgroup HDCP_ERRORS     HDCP Error Flags
//! @{
#define ERR_HDCP__INTERNAL      0x0040 //!< internal FW error
#define ERR_HDCP__V             0x0100 //!< Vs in part 2 mismatched
#define ERR_HDCP__DEV_LIMIT     0x0800 //!< firmware or hardware limitation: too many devices on DS side
#define ERR_HDCP__NACK          0x1000 //!< no ACK (NACK) on DDC bus (probably DS device does not have HDCP)
#define ERR_HDCP__P2_TO         0x2000 //!< Part 2 Time Out
#define ERR_HDCP__DS_EXCEED     0x4000 //!< DS BSTATUS indicates DEV_EXCEED or CAS_EXCEED
//
//! @}

//! HCDP Authentication States
typedef enum txHdcpState
{
    SI_TX_HDCP__OFF,
    SI_TX_HDCP__WAIT_FOR_START,
    SI_TX_HDCP__WAIT_FOR_RETRY,
    SI_TX_HDCP__WAIT_FOR_DONE,
    SI_TX_HDCP__WAIT_FIFO_READY,
    SI_TX_HDCP__KSV_FIFO_READ,
    SI_TX_HDCP__V_CHECK,
    SI_TX_HDCP__AUTHENTICATED,
    SI_TX_HDCP__SECURE,
    SI_TX_HDCP__FAILED,
    SI_TX_HDCP__INITIAL,

} txHdcpState_t;



//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

typedef struct txHdcpInstanceRecord
{
    bit_fld_t           isRepeaterMode              : 1; //!< set when TX is a part of repeater system
    bit_fld_t           dsDdcAckCheck               : 1; //!< set when US DDC is off due to DS DDC was with NACK
    bit_fld_t           isReauthRequested           : 1; //!< set on deep color change
    bit_fld_t           isAuthRequested             : 1; //!< set if downstream authentication is requested
    bit_fld_t           tmdsOnCountDownStart        : 1; //!< set on turning TMDS on
    bit_fld_t           upstreamDdcCtrl             : 1; //!< enable TX HDCP controlling upstream DDC (repeater mode)

    txHdcpState_t       authState;                  //!< authentication state
    txHdcpState_t       prevAuthState;              //!< previous value of authState

    uint32_t            dsKsvHashPrev;              //!< current KSV hash value
    uint32_t            dsKsvHash;                  //!< current KSV hash value
    uint32_t            hashWord;                   //!< internal storage for KSV hash words
    uint8_t             hashWordLen;                //!< number of bytes in hashWord storage

    time_ms_t           timeoutForDdcNack;          //!< timeout for DDC NACK
    time_ms_t           timeoutForPart1AndPart2;    //!< timeout for HDCP Part1 & Part2
    time_ms_t           timeToRetry;                //!< time left before new Part1 attempt happen
    time_ms_t           minimumUnmuteCountdown;     //!< timer to unmute after authentication
    time_ms_t           tmdsCountdown;              //!< timer to delay HDCP start after turning TMDS on
    time_ms_t           tmdsOnToUnmuteTime;         //!< Minimum time from turning TMDS ON to unmuting

    uint16_t            fifoByteCounter;            //!< how many FIFO byte left to download

    uint16_t            error;                      //!< error bitfield. @see HDCP_ERRORS
    uint16_t            prevError;                  //!< previous state of error
    txFeedbackMsg_t     fbMsg;                      //!< HDCP feedback message

} txHdcpInstanceRecord_t;


//-------------------------------------------------------------------------------------------------
//  TX AVI API functions
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void TxHdcpInstanceSet(uint8_t device_idx);
#endif

void            TxHdcpInit(void);
void            TxHdcpReset(void);
void            TxHdcpConfig(bool_t isRepeaterMode, time_ms_t tmdsOnToUnmuteTime, bool_t usDdcCtrl);
void            TxHdcpStop(void);
void            TxHdcpStart(void);
void            TxHdcpOnOutputEnable(void);
void            TxHdcpInterruptHandler(uint16_t interrupts);
void            TxHdcpRequestReauthentication(void);
time_ms_t       TxHdcpTimerHandler(time_ms_t timeDelta);
uint16_t        TxHdcpLastErrorGet(void);
txFeedbackMsg_t TxHdcpLastMessageGet(void);

#endif //        SI_TX_HDCP_H
