//*************************************************************************************************
//! @file     si_sac_msg.h
//! @brief    Message level of CEC/System Audio Control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/
#ifndef __SI_SAC_MSG_H__
#define __SI_SAC_MSG_H__

#include "si_common.h"
#include "si_cec_timer.h"
#include "si_cec_enums.h"
#include "si_sac_enums.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#define MAX_NUMBER_OF_AUDIO_FORMATS   3  //!< Maximum number of supported audio formats
#else
#define MAX_NUMBER_OF_AUDIO_FORMATS   5  //!< Maximum number of supported audio formats
#endif

#define MAX_VOLUME_LEVEL            35  //!< Maximum volume value. Minimum is 0
#define VOLUME_STEP_SIZE             1  //!< Volume increment/decrement step

#define CEC_SAC_STATUS_DELAY_MS     500  //!< Audio Status Report repetition delay (>500ms)


//! @brief Short Audio Descriptor (defined in CEA-861-D)

typedef struct
{

    uint8_t maxChanNumber;  //!< Max number of channels (bits 2-0)
    uint8_t audioFmtCode;   //!< Audio Format Code (bits 6-3)
    uint8_t sampleRate;     //!< Sample Rate bits: [res 192kHz 176kHz 96kHz 88kHz 48kHz 44kHz 32kHz]
    uint8_t quality;        //!< Bitrates or Word Length: [res res res res res 24bit 20bit 16bit]
    
} CecSacShortAudioDescriptor_t;


//! System Audio status

typedef struct
{
    bit_fld_t isSystemAudioModeEnabled : 1;   //!< SAC enable status
    bit_fld_t isMuted : 1;                    //!< Mute audio output flag
    bit_fld_t reserved : 6;                   //!< Reserved

} CecSacStatus_t;


//! CEC/SAC Instance Record

typedef struct
{
    CecSacStatus_t                  status;                             //!< SAC status flags
    CecSacShortAudioDescriptor_t    aFmt[MAX_NUMBER_OF_AUDIO_FORMATS];  //!< List of supported audio formats
    uint8_t                         numberOfAudioFormats;               //!< Total number of supported audio formats
    uint8_t                         volume;                             //!< Audio volume level [0-100]

    // -------------------- Private area -----------------------    
    CecTimeCounter_t                recallTimeCount; //!< Estimated time point of the task handler next recall
    
    // Task states    
    uint16_t                        taskFlags;     //!< Bit fields indicating active tasks
    uint16_t                        taskMask;      //!< Bitmask selecting currently active task
    CecTimeCounter_t                taskTimeCount; //!< Time counter for waiting within tasks
    CecSacTaskState_t               taskState;     //!< task current state (combined)

    SiiCecUiCommand_t               lastRcCode;               //!< Most recent key code from remote control
    CecTimeCounter_t                reportDelayCnt;           //!< Time counter for reporting audio status
    bool_t                          cecUserKeyPressed;        //!< true - waiting for CEC SAC USER KEY RELEASED message

    CecSacAudioFmtCode_t            aFmtInquired[4];          //!< List of inquired audio formats (up to 4)
    uint8_t                         numberOfRequestedFormats; //!< Length of the aFmtInquired[]
    
    uint16_t                        sysTimerCountCurMs; //!< current reading of the system timer
    uint16_t                        sysTimerCountPrvMs; //!< previous reading of the system timer

    uint8_t  SacTargetLa; // to cover TV with LA!=0, kaomin
} CecSacInstanceRecord_t;

//-------------------------------------------------------------------------------------------------
// Low level function prototypes
//-------------------------------------------------------------------------------------------------

// Low level API functions
void SiiCecSacSystemAudioModeRequestSend (const uint16_t srcPhysAddr, const uint8_t destLogAddr);
void SiiCecSacGiveAudioStatusSend (const uint8_t destLogAddr);
void SiiCecSacSetSystemAudioModeSend (const bool_t enabled, const uint8_t destLogAddr);
void SiiCecSacReportAudioStatusSend (const uint8_t volume, const bool_t muted, const uint8_t destLogAddr);
void SiiCecSacGiveSystemAudioModeStatusSend (const uint8_t destLogAddr);
#if ( configSII_DEV_953x_PORTING == 1 )
void SiiCecSacSystemAudioModeStatusSend (const uint8_t destLogAddr, bool OnOff);
#else
void SiiCecSacSystemAudioModeStatusSend (const uint8_t destLogAddr);
#endif
void SiiCecSacReportAudioCapabilitySend (const uint8_t destLogAddr, uint8_t nFmt, CecSacAudioFmtCode_t aAudioFormats[4]);
void SiiCecSacRequestAudioCapabilitySend (const uint8_t destLogAddr, uint8_t nFmt, CecSacAudioFmtCode_t aAudioFormats[4]);
void SiiCecSacUserControlPressedSend (const SiiCecUiCommand_t uiCommand, const uint8_t destLogAddr);
void SiiCecSacUserControlReleasedSend (const uint8_t destLogAddr);





#endif // __SI_SAC_MSG_H__

