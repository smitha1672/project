//***************************************************************************
//! @file   si_sac_main.h
//! @brief  Top level CEC / System Audio Control API
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/


#ifndef __SI_SAC_API_H__
#define __SI_SAC_API_H__

#include "si_common.h"
#include "si_cec_timer.h"
#include "si_cec_switch_enums.h"
#include "si_sac_msg.h"

//-------------------------------------------------------------------------------------------------
// Constant definitions
//-------------------------------------------------------------------------------------------------

//! @defgroup SAC_TASKS     SAC Tasks
//! @{
#define CEC_SAC_TASK_BASE_SERVICE           0x0001  //!< responds to SAC commands and requests
#define CEC_SAC_TASK_INITIATE               0x0002  //!< Initiates SAC
#define CEC_SAC_TASK_TERMINATE              0x0004  //!< Terminates SAC
#define CEC_SAC_TASK_VOLUME                 0x0008  //!< Remote volume control
#define CEC_SAC_TASK_DISCOVER_FMT           0x0010  //!< Discover audio formats supported by Amp
#define CEC_SAC_TASK_ALLOC_LOG_ADDR         0x0020  //!< Allocate logical address for Amp
#define CEC_SAC_TASK_POWER_ON_TV         0x0040  //!< Allocate logical address for Amp
#define CEC_SAC_TASK_WAIT_TV_RESPONSE         0x0080  //
#define CEC_SAC_TASK_INITIATE_DIRECTLY         0x0100  //
#define CEC_SAC_TASK_REPORT_AUDIO_STATUS         0x0200  //
#define CEC_SAC_TASK_POLLING_ACTIVE         0x0400  //
#define CEC_SAC_TASK_SYS_AUD_MODE_STATUS         0x0800  //
//
//! @}

//! @defgroup SAC_FB_MSG    SAC Feedback Messages
//! @{
#define CEC_SAC_FB_MSG_NONE                 0x0000  //!< default value: no messages
#define CEC_SAC_FB_MSG_SAC_INITIATE_DONE    0x0001  //!< SAC has been initiated
#define CEC_SAC_FB_MSG_SAC_TERMINATE_DONE   0x0002  //!< SAC has been terminated
#define CEC_SAC_FB_MSG_SAC_VOLUME_DONE      0x0003  //!< Volume task finished
#define CEC_SAC_FB_MSG_SAC_FMT_DISC_DONE    0x0004  //!< Audio Format Discover task finished
#define CEC_SAC_FB_MSG_SAC_ALLOC_LA_DONE    0x0005  //!< Logical Address allocation task finished
#define CEC_SAC_FB_MSG_WAKEUP_UNMUTE        0x0010  //!< request to quit Standby mode and unmute speakers
#define CEC_SAC_FB_MSG_MUTE                 0x0011  //!< request to mute speakers
#define CEC_SAC_FB_MSG_UNMUTE               0x0012  //!< request to unmute speakers
#define CEC_SAC_FB_MSG_SAC_STATUS_CHANGED   0x00A0  //!< SAC status has changed
#define CEC_SAC_FB_MSG_SAC_VOLUME_CHANGED   0x00A1  //!< SAC volume has changed
#define CEC_SAC_FB_MSG_LA_ASSIGNED          0x00A2  //!< CEC logical address assigned successfully
#define CEC_SAC_FB_MSG_ERROR                0x00E0  //!< General error occurred
#define CEC_SAC_FB_MSG_ERR_NONSAC_CMD       0x00E1  //!< Non CEC/SAC command received
#define CEC_SAC_FB_MSG_ERR_TIMEOUT          0x00E2  //!< Expected message not received within MRT
#define CEC_SAC_FB_MSG_ERR_LA_NOT_ASSIGNED  0x00E3  //!< CEC ping acknowledged (another AMP found in the network)
//
//! @}

//! @defgroup SAC_TASK_MASK     SAC Task mask
//! @{
#define CEC_SAC_TASK_MASK_ALL_UNMASKED      0xFFFF  //!< default value of the Task mask allowing any tasks to start
#define CEC_SAC_TASK_MASK_ALL_BUT_BASE      0xFFFE  //!< mask for all tasks except the Base service task
//
//! @}

//-------------------------------------------------------------------------------------------------
// Data type definitions
//-------------------------------------------------------------------------------------------------
#if ( configSII_DEV_953x_PORTING == 1 )
typedef enum
{
    SAC_TASK_TERMINATE_POWER_OFF,
    SAC_TASK_INITIATE_POWER_ON,
    SAC_TASK_INITIATE_WAIT_TV_RESPONSE,
    SAC_TASK_INITIATE_DIRECTLY,
    SAC_TASK_REPORT_AUDIO_STATUS,
    SAC_TASK_POLLING_ARC,
}AmTSacTaskEvent; 
#endif


//-------------------------------------------------------------------------------------------------
// Top level function prototypes
//-------------------------------------------------------------------------------------------------

void     SiiCecSacDeviceSelect(uint16_t deviceIndex);
CecSacInstanceRecord_t *SiiCecSacDeviceInstanceGet(void);
#if ( configSII_DEV_953x_PORTING == 1 )
void SiiCecSacConfig(uint8_t volume);
#else
void     SiiCecSacConfig(uint8_t volume, CecSacShortAudioDescriptor_t aFmt[], uint8_t nFmt);
#endif
bool_t   SiiCecSacAudioFormatDetailsGet( uint8_t audioFormatCode, uint8_t *pSad0, uint8_t *pSad1, uint8_t *pSad2 );
bool_t   SiiCecSacAddFormat( int index, uint8_t audFmtCode, uint8_t maxChanNumber, uint8_t sampleRate, uint8_t quality );
bool_t   SiiCecSacIsSysAudioModeEnabled(void);
bool_t   SiiCecSacIsMuted(void);
void     SiiCecSacVolumeSet(uint8_t volume);
uint8_t  SiiCecSacVolumeGet(void);
void     SiiCecSacVolumeStep(bool_t isUp);
void     SiiCecSacMute(CecSacMuteCmd_t muteOp);

    
void     SiiCecSacTaskInit(void);
bool_t   SiiCecSacSrvStart(void);
void     SiiCecSacSrvStop(void);

bool_t   SiiCecSacTaskProcess(SiiCpiData_t *pCecMessage);
uint16_t SiiCecSacNextInvocationTimeGet(uint16_t sysTimerCountMs);
bool_t   SiiCecSacInitiate(void);
bool_t   SiiCecSacTerminate(void);
bool_t SiiCecSacSendSysAudModeStatus( void );
bool_t   SiiCecSacUserControlPressed( SiiCecUiCommand_t rcCode);
bool_t   SiiCecSacUserControlReleased(void);
bool_t   SiiCecSacDiscoverAudioFormats(uint8_t nFmt, CecSacAudioFmtCode_t aReqFmt[]);
#if ( configSII_DEV_953x_PORTING == 1 )
void   AmTCecSacSyncVolumeStatus(bool_t bModeOnly);
void AmTCecSacTask( void );
void AmTSacTaskAssign(uint16_t SysCECSacAppTASK);
void AmTCecSacEnable( bool bEnable );
#endif

bool_t   SiiCbCecSacMessageHandler(uint8_t fbMsg);

void CecSacGetReportArcInitiate(void);
void CecSacSetStatus(bool bSacOn);

#endif // __SI_SAC_API_H__

