//****************************************************************************
//! @file   si_sac_main.c
//! @brief  Top level CEC / System Audio Control API
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_cec_internal.h"
#include "si_sac_msg.h"
#include "si_sac_main.h"
#include "si_cec_sac_config.h"

#ifndef CEC_PC_SIMULATION
#include <string.h>    
#else
#include "stdio.h"
#include "memory.h"
#include "debug_print.h"
#endif

#include "si_cec_component.h"
#include "PowerHandler.h"
#include "sk_app_arc.h"
//-------------------------------------------------------------------------------------------------
// Global Data
//-------------------------------------------------------------------------------------------------
CecSacInstanceRecord_t gSacInstRec[SII_NUM_CEC_SAC];       //!< global CEC/SAC instance record
CecSacInstanceRecord_t *pSac = &gSacInstRec[0];    //!< global pointer to CEC/SAC Instance Record

extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;

extern void AmTSacAppTaskAssign(AmTSacTaskEvent sac_event);
//-------------------------------------------------------------------------------------------------
// Local Functions
//-------------------------------------------------------------------------------------------------
static uint8_t SacTaskCounter = 0xFF;

static bool_t bGotReportARCInitiate = false;


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Request Active Source" broadcast message to
//!             identify physical address of the source device
//-------------------------------------------------------------------------------------------------

static void CecActiveSourceRequestSend(void)
{
    SiiCpiData_t cecFrame;
    SiiCecCpiWrite(CECOP_REQUEST_ACTIVE_SOURCE, 0, CEC_LOGADDR_UNREGORBC, &cecFrame);
}


//-------------------------------------------------------------------------------------------------
// Top Level API Functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Brings device instance record into current context. This function
//!             is useful when multiple HDMI outputs are present
//! @param[in]  deviceIndex - index of the device in case of multiple instantiation
//-------------------------------------------------------------------------------------------------*/

void SI_CecSacInstanceSet(uint16_t deviceIndex)
{
    if( deviceIndex < SII_NUM_CEC_SAC)
    {
        pSac = &gSacInstRec[deviceIndex];
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns pointer to current device CEC/SAC instance record
//-------------------------------------------------------------------------------------------------

CecSacInstanceRecord_t *SiiCecSacDeviceInstanceGet(void)
{
    return pSac;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Initializes current CEC/SAC Instance Record
//-------------------------------------------------------------------------------------------------

static void CecSacInstanceRecordReset(void)
{    
    memset(pSac, 0, sizeof(CecSacInstanceRecord_t)); // erase all data   
}

#if ( configSII_DEV_953x_PORTING == 1 )
//-------------------------------------------------------------------------------------------------
//! @brief      Initializes the CEC/SAC Instance Record and fills its fields related to
//!             device capabilities and non-volatile configuration.
//! @param[in]  volume - initial master volume setting,
//! @param[in]  aFmt  - pointer to array of nFmt audio format descriptors
//! @param[in]  nFmt  - number of audio format descriptors in aFmt[]
//-------------------------------------------------------------------------------------------------
void SiiCecSacConfig(uint8_t volume)
{
    uint8_t i;
    CecSacShortAudioDescriptor_t aFormats[NUMBER_OF_AUDIO_FORMATS] = SII_CEC_SAC_DEFAULT_AUDIO_FORMAT_LIST;

    CecSacInstanceRecordReset();    // Initialize all Instance Record fields
    pSac->volume = volume;  // Set initial volume
    
    pSac->numberOfAudioFormats = NUMBER_OF_AUDIO_FORMATS;
    for (i = 0; i < NUMBER_OF_AUDIO_FORMATS; i++)
    {
        pSac->aFmt[i] = aFormats[i];
    }
        
    SiiCecSacTaskInit();

    // Immediately initiate allocation of Logical Address for Audio System
    pSac->taskFlags |= CEC_SAC_TASK_ALLOC_LOG_ADDR;
}

#else
//-------------------------------------------------------------------------------------------------
//! @brief      Initializes the CEC/SAC Instance Record and fills its fields related to
//!             device capabilities and non-volatile configuration.
//! @param[in]  volume - initial master volume setting,
//! @param[in]  aFmt  - pointer to array of nFmt audio format descriptors
//! @param[in]  nFmt  - number of audio format descriptors in aFmt[]
//-------------------------------------------------------------------------------------------------
void SiiCecSacConfig(uint8_t volume, CecSacShortAudioDescriptor_t aFmt[], uint8_t nFmt)
{
    uint8_t i;
    
    if (!aFmt)
    {
        // Device audio format support configuration list
        CecSacShortAudioDescriptor_t aFormats[NUMBER_OF_AUDIO_FORMATS] = SII_CEC_SAC_DEFAULT_AUDIO_FORMAT_LIST;

        aFmt = aFormats;
        nFmt = NUMBER_OF_AUDIO_FORMATS;
    }

    CecSacInstanceRecordReset();    // Initialize all Instance Record fields
    pSac->volume = volume;  // Set initial volume
    
    if (nFmt <= MAX_NUMBER_OF_AUDIO_FORMATS)
    {
        pSac->numberOfAudioFormats = nFmt;
        for (i = 0; i < nFmt; i++)
        {
            pSac->aFmt[i] = aFmt[i];
        }
        
    }

    SiiCecSacTaskInit();

    // Immediately initiate allocation of Logical Address for Audio System
    pSac->taskFlags |= CEC_SAC_TASK_ALLOC_LOG_ADDR;
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Get the audio format details for the specified format if it is supported.
//! @param[in]  volume - initial master volume setting,
//! @param[in]  aFmt  - pointer to array of nFmt audio format descriptors
//! @param[in]  nFmt  - number of audio format descriptors in aFmt[]
//! @retval     true    - Requested Audio format supported
//! @retval     false   - Requested Audio format not supported
//-------------------------------------------------------------------------------------------------
bool_t SiiCecSacAudioFormatDetailsGet ( uint8_t audioFormatCode, uint8_t *pSad0, uint8_t *pSad1, uint8_t *pSad2 )
{
    uint8_t i;

    for ( i = 0; i < pSac->numberOfAudioFormats; i++ )
    {
        if ( pSac->aFmt[i].audioFmtCode == audioFormatCode )
        {
            *pSad0 = pSac->aFmt[i].maxChanNumber | (audioFormatCode << 3);
            *pSad1 = pSac->aFmt[i].sampleRate;
            *pSad2 = pSac->aFmt[i].quality;
            return( true );
        }
    }

    return( false );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the System Audio Mode status
//
//! @retval     true    - SA mode enabled
//! @retval     false   - SA mode disabled
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacIsSysAudioModeEnabled(void)
{
    return pSac->status.isSystemAudioModeEnabled;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns mute status
//              
//! @retval     true    - muted
//! @retval     false   - unmuted
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacIsMuted(void)
{
    return pSac->status.isMuted;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sets new volume level
//! @param[in]  volume - volume level [0..100]
//-------------------------------------------------------------------------------------------------

void SiiCecSacVolumeSet(uint8_t volume)
{
    
    pSac->volume = (volume > MAX_VOLUME_LEVEL) ? MAX_VOLUME_LEVEL : volume;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns volume
//
//! @return     volume level [0..100]
//-------------------------------------------------------------------------------------------------

uint8_t SiiCecSacVolumeGet(void)
{
    return pSac->volume;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Increments or decrements volume level with saturation
//! @param[in]  isUp - if true, the volume is incremented (decremented otherwise)
//-------------------------------------------------------------------------------------------------
#if 1
static bool_t bAudioStatusSyncStart = false;
//static CecTimeCounter_t AudioStatusSyncTimer = {0,0};
static uint32 rasTimer = 0;


void audio_status_sync_start(void)
{
    bAudioStatusSyncStart = true;
    //CecTimeCounterSet(CEC_TIME_MS2TCK(300), &AudioStatusSyncTimer);
    rasTimer = VirtualTimer_nowMs();
}

bool_t audio_status_is_sync(void)
{
    //CecTimeCounter_t timeNow;
    uint32_t timenow, duration;
    
    if(!bAudioStatusSyncStart)
        return true;

    timenow = VirtualTimer_nowMs();

    if(timenow >= rasTimer)
    {
        duration = timenow-rasTimer;
    }
    else
    {
        duration = 0xffffffff-rasTimer + timenow;
    }
    
    
    //if(CecTimeCounterUpdate( &AudioStatusSyncTimer, 0 ))
    if(duration >= 300)
    {
        bAudioStatusSyncStart = false;
        return true;
    }

    return false;
}

bool_t audio_status_synced(void)
{
    bAudioStatusSyncStart = false;
}
#endif

void SiiCecSacVolumeStep(bool_t isUp)
{
    xHMISrvEventParams event = { xHMI_EVENT_CEC, USER_EVENT_UNDEFINED, 0 };

    //AmTCecSacSyncVolumeStatus();
    if(!audio_status_is_sync())
        return;

    if (isUp)
    {
	//AmTCecEventPassToHMI(USER_EVENT_VOL_UP);
	#if 0 // volume should be sync after HMI process, kaomin
        if (pSac->volume >= MAX_VOLUME_LEVEL)
        {
            pSac->volume = MAX_VOLUME_LEVEL;
        }
        else
        {
            pSac->volume += VOLUME_STEP_SIZE;
        }
       #endif
        //AmTCecEventPassToHMI(USER_EVENT_VOL_UP);
         #if 0
        event.event = USER_EVENT_CEC_SET_VOLUME_UP;
        event.event_id = xHMI_EVENT_CEC;
        event.params = pSac->volume;
        AmTCecEventVolumeCtrl_HMI( event );
        #else
        HMI_CecVolCtrlDirect(USER_EVENT_CEC_SET_VOLUME_UP);
        AmTCecSacSyncVolumeStatus(false);
        pSac->taskFlags |= CEC_SAC_TASK_REPORT_AUDIO_STATUS;
        #endif
    }
    else // Down
    {
    #if 0 // volume should be sync after HMI process, kaomin
        if (pSac->volume <= 0)
        {
            pSac->volume = 0;
        }
        else
        {
            pSac->volume -= VOLUME_STEP_SIZE;
        }
      #endif
        //TRACE_DEBUG((0, "DN pSac->volume= %d", pSac->volume));    
        //AmTCecEventPassToHMI(USER_EVENT_VOL_DN);
        #if 0
        event.event = USER_EVENT_CEC_SET_VOLUME_DOWN;
        event.event_id = xHMI_EVENT_CEC;
        event.params = pSac->volume;
        AmTCecEventVolumeCtrl_HMI( event );
        #else
        HMI_CecVolCtrlDirect(USER_EVENT_CEC_SET_VOLUME_DOWN);
        AmTCecSacSyncVolumeStatus(false);
        pSac->taskFlags |= CEC_SAC_TASK_REPORT_AUDIO_STATUS;
        #endif
    }
    audio_status_sync_start();
}


//-------------------------------------------------------------------------------------------------
//! @brief      Mutes, unmutes or toggles mute status
//! @param[in]  muteOp - mute on, mute off or mute toggle operation
//-------------------------------------------------------------------------------------------------

void SiiCecSacMute(CecSacMuteCmd_t muteOp)
{
    
    xHMISrvEventParams event = { xHMI_EVENT_CEC, USER_EVENT_UNDEFINED, 0 };

    switch (muteOp)
    {   
        default:
        case CEC_SAC_MUTE_TOGGLE:
            pSac->status.isMuted = pSac->status.isMuted ? false : true;
	    if ( pSac->status.isMuted )
	    {
		    //AmTCecEventPassToHMI(USER_EVENT_CEC_SET_MUTE);
                    event.event = USER_EVENT_CEC_SET_VOLUME_MUTE;
                    event.event_id = xHMI_EVENT_CEC;
                    event.params = 1;
                    AmTCecEventVolumeCtrl_HMI( event );
	    }
	    else
	    {
		    //AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE);
                    event.event = USER_EVENT_CEC_SET_VOLUME_MUTE;
                    event.event_id = xHMI_EVENT_CEC;
                    event.params = 0;
                    AmTCecEventVolumeCtrl_HMI( event );
	    }
            break;
        case CEC_SAC_MUTE_ON:
            pSac->status.isMuted = true;

            event.event = USER_EVENT_CEC_SET_VOLUME_MUTE;
            event.event_id = xHMI_EVENT_CEC;
            event.params = 1;
            AmTCecEventVolumeCtrl_HMI( event );
            
	    //AmTCecEventPassToHMI(USER_EVENT_CEC_SET_MUTE);
            break;
        case CEC_SAC_MUTE_OFF:
            pSac->status.isMuted = false;

            event.event = USER_EVENT_CEC_SET_VOLUME_MUTE;
            event.event_id = xHMI_EVENT_CEC;
            event.params = 0;
            AmTCecEventVolumeCtrl_HMI( event );
	    //AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE);
            break;            
    }
    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Resets all tasks
//-------------------------------------------------------------------------------------------------

void SiiCecSacTaskInit(void)
{        
    pSac->taskFlags = 0;
    pSac->taskMask  = CEC_SAC_TASK_MASK_ALL_UNMASKED; // all tasks are unmasked by default

    // Reset time counters
    CecTimeCounterReset(&pSac->taskTimeCount);
    CecTimeCounterReset(&pSac->reportDelayCnt);    

    // Reset task states
    pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE;     

    pSac->sysTimerCountCurMs = 0;
    pSac->sysTimerCountPrvMs = 0;
    pSac->SacTargetLa = 0;

}


//-------------------------------------------------------------------------------------------------
//! @brief      Stops given task
//!             For internal use in the last stage of a task
//-------------------------------------------------------------------------------------------------

static void StopTask(uint16_t task)
{
    pSac->taskFlags &= ~task; // clear the task activity flag
    pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE;  // reset stage                    
    pSac->taskMask = CEC_SAC_TASK_MASK_ALL_UNMASKED;  // reset task mask as this task is done
}


//-------------------------------------------------------------------------------------------------
//! @brief      Standard check for the Task Timer event
//!             Including the Recall Timer update
//-------------------------------------------------------------------------------------------------

static bool_t IsUpdatedTaskTimerExpired(void)
{
    bool_t isExpired;
    // Set recall timer to maximum in order to allow the task counters to set it to a new value
    CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RECALL_TIME_MS), &pSac->recallTimeCount);                
    isExpired = CecTimeCounterUpdate(&pSac->taskTimeCount, &pSac->recallTimeCount);

    return isExpired;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Internal error reporting service for CecSacTaskBase()
//-------------------------------------------------------------------------------------------------

static void CecSacBadCmdReport(char *str)
{
    DEBUG_PRINT(CEC_MSG_DBG, "[CEC_SAC]: ERROR in <%s> message: wrong length!\n", str);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Allocates Logical Address for Audio System
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskLogAddrAllocate(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;
    SiiCpiStatus_t cecStatus;


    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pSac->taskMask = CEC_SAC_TASK_ALLOC_LOG_ADDR; // mask out other tasks until this one is done
                pSac->taskState = CEC_SAC_TASK_ALLOC_LA_STATE_PING;
                // don't break here to save time and go ahead to the fist step

        case CEC_SAC_TASK_ALLOC_LA_STATE_PING: // Send user RC code to Amp
               SiiDrvCpiSendPing(CEC_LOGADDR_AUDSYS);
                DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_ALLOC_LA]: Ping for Audio System address sent\n"));
                CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_PING_ACK_WAITING_TIME_MS), &pSac->taskTimeCount);
                pSac->taskState = CEC_SAC_TASK_ALLOC_LA_STATE_WAIT_FOR_ACK; // next stage
                break;

        case CEC_SAC_TASK_ALLOC_LA_STATE_WAIT_FOR_ACK: // Wait for ping acknowledge
                if (IsUpdatedTaskTimerExpired())
                {   // if timer expired, LA can be assigned
                    SiiDrvCpiHwStatusGet(&cecStatus); // read current CPI status
                    // and ping was not acknowledged
                    if (cecStatus.txState != SiiTX_SENDACKED)
                    {
                        // Keep the address
                        SiiCecSetDeviceLA(CEC_LOGADDR_AUDSYS);
                        sacFeedbackMsg = CEC_SAC_FB_MSG_LA_ASSIGNED;
                        DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_ALLOC_LA]: Logical Address assigned for Audio System\n"));
                    }
                    else
                    {
                        // report assignment error, if ping was acknowledged
                        sacFeedbackMsg = CEC_SAC_FB_MSG_ERR_LA_NOT_ASSIGNED;
                    }
                    pSac->taskState = CEC_SAC_TASK_ALLOC_LA_STATE_REPORT_RESULTS;
                }
                break;

        case CEC_SAC_TASK_ALLOC_LA_STATE_REPORT_RESULTS: // Last stage
                // event message to the caller
                sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_ALLOC_LA_DONE;
                StopTask(CEC_SAC_TASK_ALLOC_LOG_ADDR);
                break;

    default: 
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if initial interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }

    return  sacFeedbackMsg;
}



#if ( configSII_DEV_953x_PORTING == 1 )

//-------------------------------------------------------------------------------------------------
//! @brief      Initiates System Audio Control feature. Steps sequence depends
//!             on type of the device that initiates the SAC.
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskInitiate(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;
    
    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pSac->taskMask= CEC_SAC_TASK_INITIATE; // mask out other tasks until this one is done
                pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REQUEST_SOURCE;
        pSac->SacTargetLa = 0;
        AmTCecSacSyncVolumeStatus(true);

                break;

        case CEC_SAC_TASK_INITIATE_STATE_REQUEST_SOURCE:
                // if Active Source is unknown,
                if (SiiCecGetActiveSourceLA() == CEC_LOGADDR_UNREGORBC)
                {
                    // try to find active source
                    CecActiveSourceRequestSend();
                    // set timer for the following waiting stage
                    CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);
                    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_WAIT_SRC_REPLY;
                }
                else
                {
                    // if Active Source is already known,
                    // don't wait for response and proceed with instruction to initiate SAC on TV
                    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE;
                }
                break;

        case CEC_SAC_TASK_INITIATE_STATE_WAIT_SRC_REPLY:
                // wait for reply from Active Source.
                // If not replied, quit the task
                if (SiiCecGetActiveSourceLA() != CEC_LOGADDR_UNREGORBC)
                {
		    CecTimeCounterReset(&pSac->taskTimeCount);
			
                    //if ( pSac->status.isSystemAudioModeEnabled == 0 )	//
                    	pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE;
		    //else
                    //    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS; // quit the task
                }
                else if (IsUpdatedTaskTimerExpired())
                {   // if timer expired
                    if (SiiCecGetActiveSourceLA() == CEC_LOGADDR_UNREGORBC) // Active Source is still unknown
                    {
                // send <SSAM> even if no <AS> received, kaomin
                pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE;//CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS; // quit the task
                    }
                }
                break;
                
        case CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE:
                // Directly send "Set System Audio Mode" message to TV
                // to verify if the TV supports SAC feature.
                // If not, the TV will reply with "FeatureAbort" message
                SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_TV);
                // set timer for MRT delay
                CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);
                pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT;
                break;

        case CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT:
                // If no Feature Abort within MRT, turn the SAC on, broadcast <Send System Audio mode> and quit.
                // In case of FeatureAbort arrival (serviced by CecSacTaskBase()) within MRT, this task will be stopped
                // by Base task action
                if (IsUpdatedTaskTimerExpired())
                {
                    bool_t isEnabled;

            //isEnabled = SiiCecPortToActSrcSelect();         // Switch to the port towards activeSrcPhysAddr
                    //if (isEnabled && (pSac->status.isSystemAudioModeEnabled == false))
            //if( isEnabled )
                    {
                CecSacSetStatus(true);
			
                        pSac->status.isMuted = false;                           // Unmute audio output
		        AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE);
	
                        SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_UNREGORBC);   // Broadcast "Set System Audio Mode" message
                        DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_TASK_INITIATE]: System Audio initiated by AMP"));
                        sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_STATUS_CHANGED;             // Indicate SA state change for application
                    }
                    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS; // quit the task
                }
                break;

        case CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS: // Last stage
    
                // event message to the caller
                sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_INITIATE_DONE;
        SiiCecSacSystemAudioModeStatusSend( pSac->SacTargetLa, pSac->status.isSystemAudioModeEnabled );
		SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
                StopTask(CEC_SAC_TASK_INITIATE);
                break;

    default: 
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if initial interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }

    return  sacFeedbackMsg;
}

static uint8_t CecSacTaskInitiateDirectly(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;
    

    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
        {
           pSac->taskMask= CEC_SAC_TASK_INITIATE_DIRECTLY; // mask out other tasks until this one is done
           pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE;
        AmTCecSacSyncVolumeStatus(true);
    	}
               break;
			   
        case CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE:
	{
            // Directly send "Set System Audio Mode" message to TV
           // to verify if the TV supports SAC feature.
           // If not, the TV will reply with "FeatureAbort" message
           SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_TV);
           // set timer for MRT delay
           CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);
           pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT;
    	}
               break;

        case CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT:
        {
           // If no Feature Abort within MRT, turn the SAC on, broadcast <Send System Audio mode> and quit.
           // In case of FeatureAbort arrival (serviced by CecSacTaskBase()) within MRT, this task will be stopped
           // by Base task action
           if (IsUpdatedTaskTimerExpired())
           {
              bool_t isEnabled;

            //isEnabled = SiiCecPortToActSrcSelect();         // Switch to the port towards activeSrcPhysAddr
              //if (isEnabled && (pSac->status.isSystemAudioModeEnabled == false))
            //if( isEnabled )
              {
                CecSacSetStatus(true);
			
                  pSac->status.isMuted = false;                           // Unmute audio output
                  AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE);
			
                  SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_UNREGORBC);   // Broadcast "Set System Audio Mode" message
                  DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_TASK_INITIATE]: InitiateDirectly: System Audio initiated by AMP"));
                  sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_STATUS_CHANGED;             // Indicate SA state change for application
              }
              pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS; // quit the task
           }
        }
              break;

        case CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS: // Last stage
    	{
            // event message to the caller
            sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_INITIATE_DONE;
        SiiCecSacSystemAudioModeStatusSend( pSac->SacTargetLa, pSac->status.isSystemAudioModeEnabled );
            SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
            StopTask(CEC_SAC_TASK_INITIATE_DIRECTLY);
        }
            break;

    default:
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if initial interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Terminates System Audio Control feature
//!             Steps sequence depends on type of the device that terminates the SAC
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskTerminate(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;
    
    SiiCecSacSetSystemAudioModeSend(FALSE, CEC_LOGADDR_UNREGORBC);   // Broadcast with Set System Audio Mode message
    //AmTCecEventPassToHMI(USER_EVENT_CEC_SET_SAC_OFF);
    CecSacSetStatus(false);

    SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
	
    StopTask(CEC_SAC_TASK_TERMINATE);
    sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_TERMINATE_DONE;
    return  sacFeedbackMsg;
}


static uint8_t CecSacTaskWaitTvResponse(void)
{
    uint8_t  sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
        {
            CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);
	    pSac->taskState = CEC_SAC_TASK_INITIATE_WAIT_TV_RESPONSE;
        }
	    break;

	case CEC_SAC_TASK_INITIATE_WAIT_TV_RESPONSE:
	{
            if (IsUpdatedTaskTimerExpired())
            {
            CecSacSetStatus(true);
	
                   pSac->status.isMuted = false;                           // Unmute audio output
                   AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE);
	
                   SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_UNREGORBC);   // Broadcast "Set System Audio Mode" message
                   DEBUG_PRINT(CEC_MSG_DBG, ("[SAC_TASK_INITIATE_WAIT_TV_RESPONSE]"));
	           pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS;
                   sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_STATUS_CHANGED;             // Indicate SA state change for application
            }
	}
	    break;
			
	case CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS:
	{
        SiiCecSacSystemAudioModeStatusSend( pSac->SacTargetLa, pSac->status.isSystemAudioModeEnabled );
		SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
                StopTask(CEC_SAC_TASK_WAIT_TV_RESPONSE);
                sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_INITIATE_DONE;
	}
	    break;

    default:
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if initial interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }


    return sacFeedbackMsg;
}

static uint8_t CecSacTaskPowerOnTv(void)
{
    uint8_t  sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    CecSendUserControlPressed(CEC_LOGADDR_TV, CEC_RC_POWER);
    CecSendUserControlReleased(CEC_LOGADDR_TV, CEC_RC_POWER);

    StopTask(CEC_SAC_TASK_POWER_ON_TV);

    SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
	
    pSac->taskFlags |= CEC_SAC_TASK_INITIATE;

    sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    return sacFeedbackMsg;
}

static uint8_t CecSacTaskReportAudioStatus(void)
{
    uint8_t  sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    switch ( pSac->taskState )
    {
    case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
    {
        //CecTimeCounterSet( CEC_TIME_MS2TCK( CEC_REPORT_AUDIO_STATUS_MIN_MS), &pSac->taskTimeCount );
        pSac->taskState = CEC_SAC_TASK_SEND_REPORT_AUDIO_STATUS;
    }
    //break; // to save time, kaomin

    case CEC_SAC_TASK_SEND_REPORT_AUDIO_STATUS:
    {
        //if ( IsUpdatedTaskTimerExpired() )
        {
            //AmTCecSacSyncVolumeStatus();
    SiiCecSacReportAudioStatusSend( pSac->volume, pSac->status.isMuted, pSac->SacTargetLa);
            pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS;
        }
    }
    break;

    case CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS:
    {
    StopTask(CEC_SAC_TASK_REPORT_AUDIO_STATUS);
        SiiCecCallbackRegisterSet( SiiCecSacTaskProcess, false );
    }
    break;
    
    default:
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if initial interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }

    return sacFeedbackMsg;
}

static uint8_t CecSacTaskSysAudModeStatus( void )
{
    uint8_t  sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    SiiCecSacSystemAudioModeStatusSend( pSac->SacTargetLa, pSac->status.isSystemAudioModeEnabled );
    
    StopTask( CEC_SAC_TASK_SYS_AUD_MODE_STATUS );

    SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);

    sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    return sacFeedbackMsg;
}

static uint8_t CecSacTaskPollingActive( void )
{
    uint8_t  sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    switch ( pSac->taskState )
    {
    case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
    {
        pSac->taskState = CEC_SAC_TASK_POLLING_STATE_SEND_IARC;
    }
    break;

    case CEC_SAC_TASK_POLLING_STATE_SEND_IARC:
    {
        AmTArcTaskAssign(SYS_CEC_TASK_ARC_INITIONAL);
        CecTimeCounterSet( CEC_TIME_MS2TCK( CEC_POLLING_WAITING_TIME_MS ), &pSac->taskTimeCount );
        pSac->taskState = CEC_SAC_TASK_POLLING_STATE_WAIT_RESPONSE;
        bGotReportARCInitiate = false;
    }
    break;

    case CEC_SAC_TASK_POLLING_STATE_WAIT_RESPONSE:
    {
        if(bGotReportARCInitiate)
        {
            CecSacSetStatus(true);
            pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS;            
        }
        else if ( IsUpdatedTaskTimerExpired() )
        { // no <RARCI> received, kaomin
            CecSacSetStatus(false);
            pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS;
        }
    }
    break;

    case CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS:
    {
        // if <RARCI> received, it will set state to this one, kaomin
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE;
        SiiCecCallbackRegisterSet( SiiCecSacTaskProcess, false );
        StopTask( CEC_SAC_TASK_POLLING_ACTIVE);
    }
    break;
    
    default:
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if initial interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }

    return sacFeedbackMsg;
}

#else

//-------------------------------------------------------------------------------------------------
//! @brief      Initiates System Audio Control feature. Steps sequence depends
//!             on type of the device that initiates the SAC.
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskInitiate(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;
    
    
    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pSac->taskMask= CEC_SAC_TASK_INITIATE; // mask out other tasks until this one is done

                switch(SiiCecGetDeviceLA())
                {
                    default:
                    case CEC_LOGADDR_TV:    
                            pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REQUEST_SYS_AUD_MODE;
                            break;
                    case CEC_LOGADDR_AUDSYS:
                            pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REQUEST_SOURCE;
                            break;   
                }
                break;

        case CEC_SAC_TASK_INITIATE_STATE_REQUEST_SOURCE:
                // if Active Source is unknown,
                if (SiiCecGetActiveSourceLA() == CEC_LOGADDR_UNREGORBC)
                {
                    // try to find active source
                    CecActiveSourceRequestSend();
                    // set timer for the following waiting stage
                    CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);
                    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_WAIT_SRC_REPLY;
                }
                else
                {
                    // if Active Source is already known,
                    // don't wait for response and proceed with instruction to initiate SAC on TV
                    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE;
                }
                break;

        case CEC_SAC_TASK_INITIATE_STATE_WAIT_SRC_REPLY:
                // wait for reply from Active Source.
                // If not replied, quit the task
                if (SiiCecGetActiveSourceLA() != CEC_LOGADDR_UNREGORBC)
                {
		    CecTimeCounterReset(&pSac->taskTimeCount);
			
                    if ( AmTCecPortToActSrcSelect() )	//Check if need shpport SAC
                    	pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE;
		    else
                        pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS; // quit the task
                }
                else if (IsUpdatedTaskTimerExpired())
                {   // if timer expired
                    if (SiiCecGetActiveSourceLA() == CEC_LOGADDR_UNREGORBC) // Active Source is still unknown
                    {
                        pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS; // quit the task
                    }
                }
                break;
                
        case CEC_SAC_TASK_INITIATE_STATE_SET_SYS_AUD_MODE:
                // Directly send "Set System Audio Mode" message to TV
                // to verify if the TV supports SAC feature.
                // If not, the TV will reply with "FeatureAbort" message
                SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_TV);
                // set timer for MRT delay
                CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);
                pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT;
                break;

        case CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT:
                // If no Feature Abort within MRT, turn the SAC on, broadcast <Send System Audio mode> and quit.
                // In case of FeatureAbort arrival (serviced by CecSacTaskBase()) within MRT, this task will be stopped
                // by Base task action
                if (IsUpdatedTaskTimerExpired())
                {
                    bool_t isEnabled;

                    isEnabled = SiiCecPortToActSrcSelect();         // Switch to the port towards activeSrcPhysAddr
                    if (isEnabled && (pSac->status.isSystemAudioModeEnabled == false))
                    {
                        pSac->status.isSystemAudioModeEnabled = true;           // Turn SA mode On
		        AmTCecEventPassToHMI(USER_EVENT_CEC_SET_SAC_ON);
			
                        pSac->status.isMuted = false;                           // Unmute audio output
		        AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE);
			
                        SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_UNREGORBC);   // Broadcast "Set System Audio Mode" message
                        DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_TASK_INITIATE]: System Audio initiated by AMP"));
                        sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_STATUS_CHANGED;             // Indicate SA state change for application
                    }
                    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS; // quit the task
                }
                break;


        case CEC_SAC_TASK_INITIATE_STATE_REQUEST_SYS_AUD_MODE: // System Audio Mode request sending
                SiiCecSacSystemAudioModeRequestSend(SiiCecGetActiveSourceLA(), CEC_LOGADDR_AUDSYS);
                DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_INITIATE]: Requested audio mode\n"));
                CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);                
                pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_WAIT_SYS_AUD_MODE_CONFIRM; // next stage
                break;

        
        case CEC_SAC_TASK_INITIATE_STATE_WAIT_SYS_AUD_MODE_CONFIRM: // Wait for confirmation of SA mode enabling
                // wait until the Amp respond or timeout    
                if (IsUpdatedTaskTimerExpired())
                {   // if timer expired
                    pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS;  // next stage
                    DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_INITIATE]: Stopped waiting for response\n"));
                    sacFeedbackMsg = CEC_SAC_FB_MSG_ERR_TIMEOUT; 
                }
                break;

        case CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS: // Last stage
    
                // event message to the caller
                sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_INITIATE_DONE;
		SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
                StopTask(CEC_SAC_TASK_INITIATE);
                break;

        default:break;
    }

    return  sacFeedbackMsg;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Terminates System Audio Control feature
//!             Steps sequence depends on type of the device that terminates the SAC
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskTerminate(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;
    
    
    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pSac->taskMask= CEC_SAC_TASK_TERMINATE; // mask out other tasks until this one is done

                switch(SiiCecGetDeviceLA())
                {
                    default:
                    case CEC_LOGADDR_TV:
                            pSac->taskState = CEC_SAC_TASK_TERMINATE_STATE_REQUEST_SYS_AUD_MODE;
                            break;
                    case CEC_LOGADDR_AUDSYS:
                            pSac->status.isSystemAudioModeEnabled = false;  // Turn SA mode Off
                            pSac->status.isMuted = true;                    // Mute audio output			
                            SiiCecSacSetSystemAudioModeSend(false, 0xF);    // Broadcast with Set System Audio Mode message
                            pSac->taskState = CEC_SAC_TASK_TERMINATE_STATE_REPORT_RESULTS;
                            CecSacSetStatus(false);
                            break;   
                }
                break;
                
        case CEC_SAC_TASK_TERMINATE_STATE_REQUEST_SYS_AUD_MODE: // System Audio Mode request sending
                SiiCecSacSystemAudioModeRequestSend(0xFFFF, CEC_LOGADDR_AUDSYS); // request to terminate
                DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_TERMINATE]: Requested audio mode termination\n"));
                CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);                
                pSac->taskState = CEC_SAC_TASK_TERMINATE_STATE_WAIT_SYS_AUD_MODE_CONF; // next stage
                break;

        case CEC_SAC_TASK_TERMINATE_STATE_WAIT_SYS_AUD_MODE_CONF: // Wait for confirmation of SA mode enabling
                // wait until the Amp respond or timeout    
                if (IsUpdatedTaskTimerExpired())
                {   // if timer expired
                    pSac->taskState = CEC_SAC_TASK_TERMINATE_STATE_REPORT_RESULTS;  // next stage
                    DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_TERMINATE]: Stopped waiting for response\n"));
                    sacFeedbackMsg = CEC_SAC_FB_MSG_ERR_TIMEOUT; 
                }
                break;
    
        case CEC_SAC_TASK_TERMINATE_STATE_REPORT_RESULTS: // Last stage
    
                // event message to the caller
                sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_TERMINATE_DONE;
                StopTask(CEC_SAC_TASK_TERMINATE);
                break;

        default:break;
    }

    return  sacFeedbackMsg;
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Implements CEC operation of Volume Control feature
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskVolume(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;
    
    
    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pSac->taskMask = CEC_SAC_TASK_VOLUME; // mask out other tasks until this one is done
                pSac->taskState = CEC_SAC_TASK_VOLUME_STATE_CTRL_PRESSED_SEND;
                // don't break here to save time and go ahead to the fist step
                
        case CEC_SAC_TASK_VOLUME_STATE_CTRL_PRESSED_SEND: // Send user RC code to Amp
                SiiCecSacUserControlPressedSend(pSac->lastRcCode, CEC_LOGADDR_AUDSYS);
                DEBUG_PRINT(CEC_MSG_DBG, "[CEC_SAC_TASK_VOLUME]: User control pressed code [%02X] sent\n", pSac->lastRcCode);
                CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_UC_REPETITION_TIME_MS), &pSac->taskTimeCount);                
                pSac->taskState = CEC_SAC_TASK_VOLUME_STATE_WAIT_FOR_STATUS; // next stage
                break;

        case CEC_SAC_TASK_VOLUME_STATE_WAIT_FOR_STATUS: // Wait 200-500 ms before repetition of the UC command
                if (IsUpdatedTaskTimerExpired())
                {   // if timer expired
                    pSac->taskState = CEC_SAC_TASK_VOLUME_STATE_CTRL_PRESSED_SEND;  // loop
                    // The loop shall be broken by external switching to the following release step
                }
                break;

        case CEC_SAC_TASK_VOLUME_STATE_CTRL_RELEASED_SEND:
                SiiCecSacUserControlReleasedSend(CEC_LOGADDR_AUDSYS);
                DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_VOLUME]: User control released sent\n"));
                // don't break here to save time and go ahead to the last step
     
        case CEC_SAC_TASK_VOLUME_STATE_REPORT_RESULTS: // Last stage
                // event message to the caller
                sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_VOLUME_DONE;
                StopTask(CEC_SAC_TASK_VOLUME);
                break;
    default: 
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if this task interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }

    return  sacFeedbackMsg;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Implements Audio Format capability discovery feature
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskDiscoverFmt(void)
{
    uint8_t sacFeedbackMsg = CEC_SAC_FB_MSG_NONE;

    
    switch(pSac->taskState)
    {
        case CEC_SAC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pSac->taskMask = CEC_SAC_TASK_DISCOVER_FMT; // mask out other tasks until this one is done
                pSac->taskState = CEC_SAC_TASK_DISCOVER_STATE_REQUEST_SAD;
                // don't break here to save time and go ahead to the fist step
                
        case CEC_SAC_TASK_DISCOVER_STATE_REQUEST_SAD: // Request Amp to provide audio format support information
                SiiCecSacRequestAudioCapabilitySend(CEC_LOGADDR_AUDSYS, pSac->numberOfRequestedFormats, pSac->aFmtInquired);
               // DEBUG_PRINT(CEC_MSG_DBG, "[CEC_SAC_TASK_DISCOVER_FMT]: Request for [%d] audio formats support sent\n", (uint16_t) pSac->numberOfRequestedFormats);
                CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RESPONSE_TIME_MS), &pSac->taskTimeCount);                
                pSac->taskState = CEC_SAC_TASK_DISCOVER_STATE_WAIT_FOR_REPLY; // next stage
                break;

        case CEC_SAC_TASK_DISCOVER_STATE_WAIT_FOR_REPLY: 
                if (IsUpdatedTaskTimerExpired())
                {   // if timer expired
                   // DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC_TASK_DISCOVER_FMT]: Audio System not responded with Short Audio Descriptor.\n"));
                    sacFeedbackMsg = CEC_SAC_FB_MSG_ERR_TIMEOUT; 
                    pSac->taskState = CEC_SAC_TASK_DISCOVER_STATE_REPORT_RESULTS;
                }
                break;
     
        case CEC_SAC_TASK_DISCOVER_STATE_REPORT_RESULTS: // Last stage
                // event message to the caller
                sacFeedbackMsg = CEC_SAC_FB_MSG_SAC_FMT_DISC_DONE;
                StopTask(CEC_SAC_TASK_DISCOVER_FMT);
                break;
    default: 
        pSac->taskState = CEC_SAC_TASK_EMPTY_STATE_NONE; // if initial interupt other task, the state may be invalid in the task, so start from 1st, kaomin
        break;
    }

    return  sacFeedbackMsg;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Add the passed Short Audio Descriptor info to our format list.
//!
//! @param[in]
//! @retval     true    - Format added
//! @retval     false   - Unable to add format
//-------------------------------------------------------------------------------------------------
bool_t SiiCecSacAddFormat ( int index, uint8_t audFmtCode, uint8_t maxChanNumber, uint8_t sampleRate, uint8_t quality )
{
    int     curFmtIndex, fmtIndex;
    bool_t  isEmptyFound;

    fmtIndex        = -1;
    curFmtIndex     = -1;
    isEmptyFound    = false;
    if ( index == -1 )
    {
        // Find empty, update existing or add new record to the format list
        fmtIndex        = -1;       // Assume unable to find space
        curFmtIndex     = 0;
        while ( curFmtIndex < pSac->numberOfAudioFormats)
        {
            if ( pSac->aFmt[ curFmtIndex].audioFmtCode == CEC_AUD_FMT_RES0 ||
                    pSac->aFmt[ curFmtIndex].audioFmtCode == audFmtCode)
            {
                // empty or existing record found
                isEmptyFound = true;
                break;
            }
            curFmtIndex++;
        }
    }
    else
    {
        if ( index < pSac->numberOfAudioFormats)
        {
            curFmtIndex = index;
            isEmptyFound = true;    // Not really empty, but this is where we'll put it.
        }
    }
    if ( isEmptyFound )
    {
        fmtIndex = curFmtIndex;     // use empty cell
    }
    else
    {   // occupy new cell if possible
        if ( pSac->numberOfAudioFormats < MAX_NUMBER_OF_AUDIO_FORMATS)
        {
            fmtIndex = pSac->numberOfAudioFormats;
            pSac->numberOfAudioFormats++;
        }
        else
        {
            DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC]: ERROR: Not enough room for audio format records!\n"));
        }
    }

    if ( fmtIndex != -1 )
    {
        // Add received format record
        pSac->aFmt[fmtIndex].maxChanNumber  = maxChanNumber & 0x7;
        pSac->aFmt[fmtIndex].audioFmtCode   = audFmtCode;
        pSac->aFmt[fmtIndex].sampleRate     = sampleRate;
        pSac->aFmt[fmtIndex].quality        = quality;
    }

    return( fmtIndex != -1 );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Performs CEC/SAC message parsing and support basic SAC functionality
//!             This task can't be masked and will run in background of the
//!             other active tasks, if an incoming CEC message is pending.
//
//! @param[in]  pCecMessage - pointer to a pending CEC message or 0, if there is no message awaiting
//
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CecSacTaskBase(SiiCpiData_t *pCecMessage)
{
    uint8_t     feedbackMsg = CEC_SAC_FB_MSG_NONE;
    uint8_t     senderLogAddr;
    uint8_t     destLogAddr;
    bool_t  isDirectAddressed;
		    
    isDirectAddressed   = (bool_t)!((pCecMessage->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );
	
    // Update timers    
    CecTimeCounterUpdate(&pSac->reportDelayCnt, 0); // don't affect recall timer because operation is tied to UC events

    if (pCecMessage != 0) //decode a message, update status
    {
        senderLogAddr = (pCecMessage->srcDestAddr & 0xF0) >> 4; // sender's logical address
        destLogAddr   =  pCecMessage->srcDestAddr & 0x0F; // destination logical address

        if ((destLogAddr == SiiCecGetDeviceLA()) || (destLogAddr == 0xF)) // ignore messages addressed to other devices
        {
            switch (pCecMessage->opcode) // CEC operation code
            {
                case CECOP_SYSTEM_AUDIO_MODE_REQUEST:
                    
                    if ((pCecMessage->argCount != 2) && (pCecMessage->argCount != 0))
                    {
                    	feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                        CecSacBadCmdReport("System Audio Mode Request");
                        break;
                    }
                    
                    if (SiiCecGetDeviceLA() == CEC_LOGADDR_AUDSYS) // Device must be an Amp to accept this command
                    {
                        if (pCecMessage->argCount == 2)	// Initial SAC
                        {
                            bool_t isEnabled;
#if ( configSII_DEV_953x_PORTING == 1 )
			    uint16_t activeSrcPa;

			    activeSrcPa = (pCecMessage->args[0] << 8) | pCecMessage->args[1];
                            SiiCecSetActiveSourcePA(activeSrcPa);
#else
                            SiiCecSetActiveSourcePA((pCecMessage->args[0] << 8) | pCecMessage->args[1]);
#endif
                            DEBUG_PRINT(CEC_MSG_DBG, "[CEC_SAC]: received <System Audio Mode Request> message with source PA = %04x\n", SiiCecGetActiveSourcePA());
			    
                            isEnabled = SiiCecPortToActSrcSelect();         // Switch to the port towards activeSrcPhysAddr	
#if ( configSII_DEV_953x_PORTING == 1 )
                            //AAAAA: LA = tuner, 接input. 但isEnabled卻為fail 導致11.2.15-1
                            if (isEnabled )
                            {
				//if (!SiiCecIsAdjacentLA(senderLogAddr))	// if requested by non-child device
				/*if ( activeSrcPa == 0x0000 )	// if active source is TV
				{
                                        SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_TV);   //
					AmTSacAppTaskAssign(SAC_TASK_INITIATE_WAIT_TV_RESPONSE);
                                        //feedbackMsg = wait for feature abort;     //
                                        break;
				}*/
                            CecSacSetStatus(true);
                            AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE); // CECT 11.2.15-16
                                feedbackMsg = CEC_SAC_FB_MSG_WAKEUP_UNMUTE;     // Amp goes out of standby
                                //SiiCecPortToActSrcSelect();
                                //pSac->status.isMuted = false;                   // Unmute audio output
                                
                            if(senderLogAddr == CEC_LOGADDR_TV) // for CECT 11.2.15-19
                            {
                                SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_UNREGORBC);   // Broadcast "Set System Audio Mode" message
                                SiiCecSacSystemAudioModeStatusSend( pSac->SacTargetLa, pSac->status.isSystemAudioModeEnabled );
                                AmTCecSacSyncVolumeStatus(true);
                            }

                            // power on device when receiving System Audio Mode Request, kaomin
			        //AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_ON_HDMI_IN);	//Send event to HMI to power on system if need
                            }
                        else // Active Source is not TV nor child device
			    {
		                //AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_ON_HDMI_ARC);
                                SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_TV);   //
				AmTSacAppTaskAssign(SAC_TASK_INITIATE_WAIT_TV_RESPONSE);
                                //feedbackMsg = wait for feature abort;     //
                                //SiiCecFeatureAbortSend(pCecMessage->opcode, CECAR_REFUSED, senderLogAddr);				
			    }
                        
                        if ( AmTCecGetSystemPowerStatus() == POWER_OFF )
                        {
                            AmTCecEventPassToHMI(USER_EVENT_SAC_WAKE_UP);
                        }
#else
                            if (isEnabled && (pSac->status.isSystemAudioModeEnabled == false))
                            {
                                feedbackMsg = CEC_SAC_FB_MSG_WAKEUP_UNMUTE;     // Amp goes out of standby
                                //SiiCecPortToActSrcSelect();
                                pSac->status.isSystemAudioModeEnabled = true;   // Turn SA mode On					
                                pSac->status.isMuted = false;                   // Unmute audio output
                                
                                SiiCecSacSetSystemAudioModeSend(true, CEC_LOGADDR_UNREGORBC);   // Broadcast "Set System Audio Mode" message
                            }
#endif
                        }
                        else // message without a parameter => Terminate SAC
                        {
                            DEBUG_PRINT(CEC_MSG_DBG, "[CEC_SAC]: received <System Audio Mode Request> message without a parameter\n");
#if ( configSII_DEV_953x_PORTING == 1 )
                            CecSacSetStatus(false);
                            AmTCecEventPassToHMI(USER_EVENT_CEC_SET_MUTE); // CECT 11.2.15-17
                            SiiCecSacSetSystemAudioModeSend(false, CEC_LOGADDR_UNREGORBC);  // Reply with Set System Audio Mode message
#else
                            pSac->status.isSystemAudioModeEnabled = false;  // Turn SA mode Off
                            pSac->status.isMuted = true;                    // Mute audio output
                            SiiCecSacSetSystemAudioModeSend(false, senderLogAddr);  // Reply with Set System Audio Mode message
#endif
			    //SiiCecFeatureAbortSend(pCecMessage->opcode, CECAR_INVALID_OPERAND, destLogAddr);
                            feedbackMsg = CEC_SAC_FB_MSG_SAC_STATUS_CHANGED;                            
                        }
                    }
                    else
                    {
                        DEBUG_PRINT(MSG_ALWAYS, "[CEC_SAC]: Device must be Audio System. Replied with FeatureAbort.\n");
                        SiiCecFeatureAbortSend(pCecMessage->opcode, CECAR_REFUSED, senderLogAddr);
                    }                    
                    break;

                case CECOP_GIVE_AUDIO_STATUS:
                    if (pCecMessage->argCount != 0)
                    {
                    	feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                        CecSacBadCmdReport("Give Audio Status");
                    }
                    else if ( isDirectAddressed )
                    {
#if ( configSII_DEV_953x_PORTING == 0 )
                        if ( AmTCecGetSystemPowerStatus() != POWER_ON )
                        {
                            SiiCecFeatureAbortSend(pCecMessage->opcode, CECAR_NOT_CORRECT_MODE, senderLogAddr);
			    break;
                        }
#endif
                    //if(audio_status_is_sync())
                    {
                         // Respond with "Report Audio Status" to sender
                         //AmTCecSacSyncVolumeStatus();
                         DEBUG_PRINT((0, "<RAS> response %d %d", pSac->volume, pSac->status.isMuted));
                         SiiCecSacReportAudioStatusSend( pSac->volume, pSac->status.isMuted, senderLogAddr );
                    }
                    }
                    break;
                    
                case CECOP_SET_SYSTEM_AUDIO_MODE:
                    if (pCecMessage->argCount != 1)
                    {
                    	feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                        CecSacBadCmdReport("Set System Audio Mode");
                        break;
                    }
                    
                    if (pSac->taskState == CEC_SAC_TASK_INITIATE_STATE_WAIT_SYS_AUD_MODE_CONFIRM ||
                        pSac->taskState == CEC_SAC_TASK_TERMINATE_STATE_WAIT_SYS_AUD_MODE_CONF)
                    {   // if TV is waiting for SA mode reply from the AMP
                        bool_t isSAModeEnabledPrv = pSac->status.isSystemAudioModeEnabled;
                        
                        DEBUG_PRINT(MSG_ALWAYS, ("[CEC_SAC]: System Audio Mode "));
                        if (pCecMessage->args[0] != 0) // SA mode enabled
                        {
                        CecSacSetStatus(true);
				
			    //SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, true);

                            pSac->status.isMuted = true;
		            AmTCecEventPassToHMI(USER_EVENT_CEC_SET_MUTE);

                            DEBUG_PRINT(MSG_ALWAYS, ("enabled.\n"));
                        }
                        else  // SA mode disabled
                        {
                        CecSacSetStatus(false);
				
			    //SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
				
                            pSac->status.isMuted = false;
		            AmTCecEventPassToHMI(USER_EVENT_CEC_SET_DEMUTE);
				
                            DEBUG_PRINT(MSG_ALWAYS, ("disabled.\n"));                            
                        }

                        if (isSAModeEnabledPrv != pSac->status.isSystemAudioModeEnabled)
                        {
                            feedbackMsg = CEC_SAC_FB_MSG_SAC_STATUS_CHANGED;
                        }
                        CecTimeCounterReset(&pSac->taskTimeCount); // stop waiting to finish the Initiate task immediately
                    }
                    break;
                    
                case CECOP_REPORT_AUDIO_STATUS:
                    if (pCecMessage->argCount != 1)
                    {
                    	feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                        CecSacBadCmdReport("Report Audio Status");
                        break;
                    }
                     
                    if (pSac->status.isSystemAudioModeEnabled)
                    {
                    xHMISrvEventParams event;
                        // Set local volume equal to the volume of the Amp
                    //pSac->volume = pCecMessage->args[0] & 0x7F;
                    event.event = USER_EVENT_CEC_SET_VOLUME_MUTE;
                    event.event_id = xHMI_EVENT_CEC;
                    event.params = pCecMessage->args[0] & 0x7F;
                    AmTCecEventVolumeCtrl_HMI( event );

                        feedbackMsg = CEC_SAC_FB_MSG_SAC_VOLUME_CHANGED;                        
                    }
                    break;
                    
                case CECOP_GIVE_SYSTEM_AUDIO_MODE_STATUS:					
                    if (pCecMessage->argCount != 0)
                     {
                    	feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                        CecSacBadCmdReport("Give System Audio Mode Status");
                     }
                     else if ( isDirectAddressed )
                     {
                    SiiCecSacSystemAudioModeStatusSend( senderLogAddr, pSac->status.isSystemAudioModeEnabled );
                }
                break;

                case CECOP_SYSTEM_AUDIO_MODE_STATUS:
                if ( pCecMessage->argCount != 1 )
                {
                    feedbackMsg = CEC_SAC_FB_MSG_ERROR;
                    CecSacBadCmdReport( "System Audio Mode Status" );
                }
                else if ( isDirectAddressed )
                {
                    uint8 status = pCecMessage->args[0]?1:0;
                    if(pSac->status.isSystemAudioModeEnabled != status)
                    {
                        // to sync sac status softly, kaomin
                        SiiCecSacSystemAudioModeStatusSend( senderLogAddr, pSac->status.isSystemAudioModeEnabled );
                    }
                }
                    break;
                    
                case CECOP_REPORT_SHORT_AUDIO: 
                    if (pCecMessage->argCount % 3 != 0)
                    {
                    	feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                        CecSacBadCmdReport("Report Short Audio Descriptor");
                        break;                        
                    }
                    
                    if ((senderLogAddr == CEC_LOGADDR_AUDSYS) && /* Shall come from Amp*/
                        (pSac->taskState == CEC_SAC_TASK_DISCOVER_STATE_WAIT_FOR_REPLY)) // Format Discovery task is running
                    { 
                        uint8_t i, nFmt;

                        nFmt = pCecMessage->argCount / 3;

                        // Update local information about supported audio formats
                        for (i = 0; i < nFmt; i++)
                        {
                            // Add received format record
                            SiiCecSacAddFormat(
                                -1, // Add to empty slot if available
                                (pCecMessage->args[3 * i] >> 3) & 0xF,
                                pCecMessage->args[3 * i] & 0x7,
                                pCecMessage->args[3 * i + 1],
                                pCecMessage->args[3 * i + 2]
                                );
                        }
                        pSac->taskState = CEC_SAC_TASK_DISCOVER_STATE_REPORT_RESULTS; // skip timeout check
                    }
                    break;

                case CECOP_REQUEST_SHORT_AUDIO:
                    if (SiiCecGetDeviceLA() == CEC_LOGADDR_AUDSYS) // Device must be an Amp to accept this command
                    {
                        uint8_t i;
                        
                        if ((pCecMessage->argCount > 0) && (pCecMessage->argCount <= 4))
                        {
                            pSac->numberOfRequestedFormats = pCecMessage->argCount;
                            for (i = 0; i < pSac->numberOfRequestedFormats; i++)
                            {
                                if (((pCecMessage->args[i] >> 6) & 0x3) == 0)
                                {
                                    pSac->aFmtInquired[i] = pCecMessage->args[i] & 0x1F;
                                }
                                else
                                {
                                    DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC]: Unknown Audio Format ID!\n"));
                                    pSac->aFmtInquired[i] = CEC_AUD_FMT_RES0;
                                }
                            }
                            SiiCecSacReportAudioCapabilitySend(senderLogAddr, pSac->numberOfRequestedFormats, pSac->aFmtInquired);
                        }
			else
                    		feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                    }
		    else
                    	feedbackMsg = CEC_SAC_FB_MSG_ERR_NONSAC_CMD;                    
                    break;
                    
                case CECOP_USER_CONTROL_PRESSED:
                        xHMISystemParams SysPrams;
                    if (pCecMessage->argCount != 1)
                    {
                        CecSacBadCmdReport("User Control Pressed");
                    	feedbackMsg = CEC_SAC_FB_MSG_ERROR;                    
                        break;
                    }

#if ( configSII_DEV_953x_PORTING == 1 )

                    SysPrams = AmTCecGetSyspramsFromHMI();
                    if ( AmTCecGetSystemPowerStatus() != POWER_ON )
                    {
                    if(pCecMessage->args[0] != CEC_RC_POWER_ON_FUNCTION)
                        SiiCecFeatureAbortSend(pCecMessage->opcode, CECAR_NOT_CORRECT_MODE, senderLogAddr);
			break;
                    }
#if 0 //Accept User Control Press message under any input
		    else if ( (SysPrams.input_src != AUDIO_SOURCE_HDMI_ARC) && (SysPrams.input_src != AUDIO_SOURCE_HDMI_IN) )
		    {
                        SiiCecFeatureAbortSend(pCecMessage->opcode, CECAR_NOT_CORRECT_MODE, senderLogAddr);
			break;
		    }
#endif
#endif
                    // Only process the User Control pressed commands if SAC mode is enabled
                    feedbackMsg = CEC_SAC_FB_MSG_ERR_NONSAC_CMD;
                    if ( pSac->status.isSystemAudioModeEnabled )
                    {
                        feedbackMsg = CEC_SAC_FB_MSG_NONE;
                        switch (pCecMessage->args[0])
                        {
                            case CEC_RC_VOLUME_UP:
                                SiiCecSacVolumeStep(true);
                        //SiiCecSacMute( CEC_SAC_MUTE_OFF );
                                feedbackMsg = CEC_SAC_FB_MSG_SAC_VOLUME_CHANGED;
                                break;
                            case CEC_RC_VOLUME_DOWN:
                                SiiCecSacVolumeStep(false);
                                feedbackMsg = CEC_SAC_FB_MSG_SAC_VOLUME_CHANGED;
                                break;
                            case CEC_RC_MUTE:
                                SiiCecSacMute(CEC_SAC_MUTE_TOGGLE);
                                feedbackMsg = CEC_SAC_FB_MSG_SAC_STATUS_CHANGED;								
                                break;
                            case CEC_RC_MUTE_FUNCTION:
                                SiiCecSacMute(CEC_SAC_MUTE_ON);
                                feedbackMsg = CEC_SAC_FB_MSG_MUTE;
                                break;
                            case CEC_RC_RESTORE_VOLUME_FUNCTION:
                                SiiCecSacMute(CEC_SAC_MUTE_OFF);
                                feedbackMsg = CEC_SAC_FB_MSG_UNMUTE;
                                break;
                            default:
                                // Let other keys pass through
                                feedbackMsg = CEC_SAC_FB_MSG_ERR_NONSAC_CMD;
                                DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC]: User Control Pressed message not used by SAC\n"));
                                break;
                        }
                    }
#if 0
                    // If key was used, continue processing...
                    if ( feedbackMsg != CEC_SAC_FB_MSG_ERR_NONSAC_CMD)
                    {
                        pSac->cecUserKeyPressed = true;

                        // Report Audio Status to control device
                        // Shall be sent no more frequently than once in 500ms
                        if (CecIsTimeCounterExpired(&pSac->reportDelayCnt))
                        {
			    // Report status
                            SiiCecSacReportAudioStatusSend(pSac->volume, pSac->status.isMuted, senderLogAddr);
                            DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC]: Report Audio Status sent\n"));
                            // set new delay timer
                            CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_SAC_STATUS_DELAY_MS), &pSac->reportDelayCnt);
                        }
                    }
		    else
                    	feedbackMsg = CEC_SAC_FB_MSG_ERR_NONSAC_CMD;      
#endif
                    break;
                    
                case CECOP_USER_CONTROL_RELEASED:
                    if ( pSac->cecUserKeyPressed )
                        pSac->cecUserKeyPressed = false;
		    else
                    	feedbackMsg = CEC_SAC_FB_MSG_ERR_NONSAC_CMD;                    
                    break;
                    
                case CECOP_FEATURE_ABORT:
                    // "Feature Abort" message shall break Initiate Task if it waits for "No Feature Abort" condition
#if ( configSII_DEV_953x_PORTING == 1 )
                    if ( (pCecMessage->args[0] == CECOP_SET_SYSTEM_AUDIO_MODE) 
						&& ((pSac->taskState == CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT)
						||(pSac->taskState == CEC_SAC_TASK_INITIATE_WAIT_TV_RESPONSE )))
#else
                    if (pSac->taskState == CEC_SAC_TASK_INITIATE_STATE_WAIT_NO_FEATURE_ABORT)
#endif
                    {
                        // Terminate Initiate Task
		    	CecTimeCounterReset(&pSac->taskTimeCount);
                    CecSacSetStatus(false);
                        pSac->taskState = CEC_SAC_TASK_INITIATE_STATE_REPORT_RESULTS;
                    }
		    else
                    	feedbackMsg = CEC_SAC_FB_MSG_ERR_NONSAC_CMD;                    

                    break;

                default:  // the message doesn't belong to CEC/SAC group
                    //DEBUG_PRINT(CEC_MSG_DBG, ("[CEC_SAC]: Non System Audio Control CEC message\n"));
                    feedbackMsg = CEC_SAC_FB_MSG_ERR_NONSAC_CMD;                    
            }
        }
        
    }

    return feedbackMsg;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Task dispatching function.
//
//!             Shall be called periodically.
//!             Using of SiiCecSacNextInvocationTimeGet() helps to figure out
//!             exact recall time that can vary depending on current task status.
//!             Alternatively, simple periodic polling would work as well;
//!             in the case of polling, CEC_MIN_RECALL_TIME_MS is a suggested
//!             recall time.
//!
//! @param[in]  pCecMessage   - pointer to a pending CEC message or 0, if there is no message awaiting
//!
//! @return     true if CEC message has been consumed
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacTaskProcess(SiiCpiData_t *pCecMessage)
{
    uint8_t  feedbackMsg = CEC_SAC_FB_MSG_NONE;
    uint16_t taskFlagsMasked;   
    uint16_t sysTimerCountMs = SiiOsTimerTotalElapsed();

    // Keep all tasks frozen if the Base task is off
    if (pSac->taskFlags & CEC_SAC_TASK_BASE_SERVICE)
    {
        // Update time reading variables
        CecSysTimeMsSet(sysTimerCountMs, &pSac->sysTimerCountCurMs, &pSac->sysTimerCountPrvMs);
        
        if (((pSac->taskFlags & CEC_SAC_TASK_MASK_ALL_BUT_BASE) == 0) && (pCecMessage == 0))
        {
            // Initialize the recall time counter by maximum possible value
            // in order to ensure rare recalls of the task handler when tasks are inactive
            CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MAX_RECALL_TIME_MS), &pSac->recallTimeCount);
        }
        else
        {   // Ensure fast recall if there are active tasks or pending messages as they could 
            // require immediate subsequent action (e.g., launching of active tasks) 
            // Actual recall time will be corrected by task internal timer updates, if any
            CecTimeCounterSet(CEC_TIME_MS2TCK(CEC_MIN_RECALL_TIME_MS), &pSac->recallTimeCount);        
        }
        
        // Do one task at a time. Tasks shall never be running concurrently except the Base Service that
        // can run in background of another task, not corrupting its context.
        // Base Service has highest priority if a CEC message is pending and lowest priority otherwise
        
        taskFlagsMasked = (pSac->taskFlags & pSac->taskMask);

        if (pCecMessage != 0) // CEC message is pending
        {   
            feedbackMsg = CecSacTaskBase(pCecMessage);      
        }
        else
        {
            if (taskFlagsMasked & CEC_SAC_TASK_INITIATE)
            {
                feedbackMsg = CecSacTaskInitiate();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
            else if (taskFlagsMasked & CEC_SAC_TASK_TERMINATE)
            {
                feedbackMsg = CecSacTaskTerminate();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
            else if (taskFlagsMasked & CEC_SAC_TASK_VOLUME)
            {
                feedbackMsg = CecSacTaskVolume();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
            else if (taskFlagsMasked & CEC_SAC_TASK_DISCOVER_FMT)
            {
                feedbackMsg = CecSacTaskDiscoverFmt();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
            else if (taskFlagsMasked & CEC_SAC_TASK_ALLOC_LOG_ADDR)
            {
                feedbackMsg = CecSacTaskLogAddrAllocate();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
#if ( configSII_DEV_953x_PORTING == 1 )

            else if (taskFlagsMasked & CEC_SAC_TASK_WAIT_TV_RESPONSE)
            {
                feedbackMsg = CecSacTaskWaitTvResponse();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
            else if (taskFlagsMasked & CEC_SAC_TASK_POWER_ON_TV)
            {
                feedbackMsg = CecSacTaskPowerOnTv();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
	    else if ( taskFlagsMasked & CEC_SAC_TASK_INITIATE_DIRECTLY)
	    {
                feedbackMsg = CecSacTaskInitiateDirectly();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
	    }
            else if( taskFlagsMasked & CEC_SAC_TASK_SYS_AUD_MODE_STATUS )
            {
               feedbackMsg = CecSacTaskSysAudModeStatus();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
            }
	    else if ( taskFlagsMasked & CEC_SAC_TASK_REPORT_AUDIO_STATUS)
	    {
                feedbackMsg = CecSacTaskReportAudioStatus();
                pSac->taskFlags &= ~CEC_SAC_TASK_POLLING_ACTIVE;
	    }
            else if( taskFlagsMasked & CEC_SAC_TASK_POLLING_ACTIVE ) 
            {
                feedbackMsg = CecSacTaskPollingActive();
            }
#endif
            // Base Task can't be masked and can run in background of the other active task if incoming CEC message is pending
            else
            {
                feedbackMsg = CecSacTaskBase(0);
            }
        }
        
    }
    
    return SiiCbCecSacMessageHandler(feedbackMsg);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns longest possible time delay before the next invocation
//!             of the SiiCecSacTaskProcess() function.
//
//!             This function can be used to set a countdown timer and call
//!             the SiiCecSacTaskProcess() on the timer's expiration event unless
//!             new CEC message is pending
//  
//! @param[in]  sysTimerCountMs - current value of the system time counter (in milliseconds)
//
//! @return     time in milliseconds
//-------------------------------------------------------------------------------------------------

uint16_t SiiCecSacNextInvocationTimeGet(uint16_t sysTimerCountMs)
{
    return SiiCecNextInvocationTimeGet(sysTimerCountMs, pSac->recallTimeCount);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Launches CEC/SAC Active and Base Services.
//
//! @return     true, if the task can't be scheduled, false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacSrvStart(void)
{
    bool_t errStatus = false;
    xHMISystemParams SysParms;
			    
    pSac->taskFlags |= CEC_SAC_TASK_BASE_SERVICE;

    /*SysParms = AmTCecGetSyspramsFromHMI();
    if ( (SysParms.cec_sac == TRUE) && (SysParms.input_src == AUDIO_SOURCE_HDMI_IN) )
    {
        pSac->taskMask |= CEC_SAC_TASK_POWER_ON_TV;
	
	SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, true);
    }*/
	
    AmTCecSacSyncVolumeStatus(true);
		
    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Stops Active and Base Services
//-------------------------------------------------------------------------------------------------

void SiiCecSacSrvStop(void)
{
    pSac->taskFlags &= ~CEC_SAC_TASK_BASE_SERVICE;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Initiate System Audio Control Feature
//              
//! @retval     true     - if the task is scheduled successfully,
//! @retval     false    - error
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacInitiate(void)
{
    xHMISystemParams SysPrams;

    SysPrams = AmTCecGetSyspramsFromHMI();
TRACE_DEBUG((0, "SiiCecSacInitiate" ));
    if ( (AmTCecGetSystemPowerStatus() == POWER_ON) && (SysPrams.input_src == AUDIO_SOURCE_HDMI_IN) )
    {
        // schedule immediate start of the task
        pSac->taskFlags |= CEC_SAC_TASK_INITIATE;
    }

    return true;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Terminate System Audio Control Feature
//              
//! @retval     true     - if the task is scheduled successfully,
//! @retval     false    - error
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacTerminate(void)
{
    // schedule immediate start of the task
    pSac->taskFlags |= CEC_SAC_TASK_TERMINATE;

    return true;
}

bool_t SiiCecSacSendSysAudModeStatus( void )
{
    pSac->taskFlags |= CEC_SAC_TASK_SYS_AUD_MODE_STATUS;
    return true;
}

//-------------------------------------------------------------------------------------------------
//! @brief Process Audio User Control pressed.
//              
//! @retval     true     - if the task is scheduled successfully,
//! @retval     false    - error
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacUserControlPressed( SiiCecUiCommand_t rcCode)
{
    bool_t isSuccess = true;
    
    // Audio Mode must be enabled to adjust controls remotely
    if (pSac->status.isSystemAudioModeEnabled && (SiiCecGetDeviceLA() != CEC_LOGADDR_AUDSYS))
    {        
        switch (rcCode)
        {
            case CEC_RC_VOLUME_UP:
            case CEC_RC_VOLUME_DOWN:
                pSac->lastRcCode = rcCode;
                pSac->taskFlags |= CEC_SAC_TASK_VOLUME; // schedule immediate start of the task                             
                break;

            case CEC_RC_MUTE:                
                SiiCecSacUserControlPressedSend(rcCode, CEC_LOGADDR_AUDSYS);
                break;
                
            default:
                DEBUG_PRINT(MSG_STAT, "SI_CecSacUserControlPressed(): RC code [%02X] not supported\n", rcCode);
                isSuccess = false; // error
                break;
        }
    }
    else
    {
        isSuccess = false; // error
    }
    
    return isSuccess;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Process Audio User Control released.
//              
//! @retval     true     - if the task is scheduled successfully,
//! @retval     false    - error
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacUserControlReleased(void)
{
    bool_t isSuccess = true;
    
    // Audio Mode must be enabled to adjust controls remotely
    if (pSac->status.isSystemAudioModeEnabled && (SiiCecGetDeviceLA() != CEC_LOGADDR_AUDSYS))
    {        
        if (pSac->taskFlags & CEC_SAC_TASK_VOLUME)
        {
            // break the press & hold loop in the Volume Task
            // and send UC release message
            pSac->taskState = CEC_SAC_TASK_VOLUME_STATE_CTRL_RELEASED_SEND;
        }
    }
    else
    {
        isSuccess = false; // error
    }
    
    return isSuccess;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Determines if audio system supports given audio formats.
//
//!             The result will be reflected in updated aFmt[] list in the CEC/SAC
//!             instance record upon the Discover task completion
//! @param[in]  nFmt        - number of requested formats
//! @param[in]  aReqFmt     - pointer to array of audio format codes
//              
//! @retval     true    - if the task is scheduled successfully,
//! @retval     false   - error
//-------------------------------------------------------------------------------------------------

bool_t SiiCecSacDiscoverAudioFormats(uint8_t nFmt, CecSacAudioFmtCode_t aReqFmt[])
{
    uint8_t i;
    uint8_t k;
    bool_t isSuccess = true;
    bool_t  isMatchFound;

    // Format Discovery can't be initiated by Amp
    if (pSac->status.isSystemAudioModeEnabled && SiiCecGetDeviceLA() != CEC_LOGADDR_AUDSYS)
    {
    
        // Invalidate the status of requested formats in order to update it
        // upon receiving response from the Amp
        for (i = 0; i < pSac->numberOfAudioFormats; i++)
        {
            isMatchFound = false;
            for (k = 0; k < nFmt; k++)
            {
                if (pSac->aFmt[i].audioFmtCode == aReqFmt[k])
                {
                    isMatchFound = true;
                    break;
                }
            }
            
            if (isMatchFound)
            {   
                // Invalidate by resetting the format code
                pSac->aFmt[i].audioFmtCode = CEC_AUD_FMT_RES0;
            }
        }

        pSac->numberOfRequestedFormats = nFmt;

        for (i = 0; i < nFmt; i++)
        {
            pSac->aFmtInquired[i] = aReqFmt[i];
        }
        
        // schedule immediate start of the task
        pSac->taskFlags |= CEC_SAC_TASK_DISCOVER_FMT;
    }
    else
    {
        isSuccess = false;
    }

    return isSuccess;
}

#if ( configSII_DEV_953x_PORTING == 1 )

//-------------------------------------------------------------------------------------------------
//! @brief    
//-------------------------------------------------------------------------------------------------

void AmTCecSacSyncVolumeStatus( bool_t bModeOnly )
{
#if 1
	xHMISystemParams SysParms;
	
	SysParms = AmTCecGetSyspramsFromHMI();
    pSac->status.isSystemAudioModeEnabled = SysParms.cec_sac;
    if(!bModeOnly)
    {
        pSac->volume = SysParms.master_gain;
        pSac->status.isMuted = SysParms.mute;
    }
#endif
}

void AmTCecSacSetVolumeStatus( bool_t sac_on, uint8_t vol,  bool_t mute)
{
    pSac->volume = vol;
    pSac->status.isMuted = mute;
    pSac->status.isSystemAudioModeEnabled = sac_on;
}

void AmTCecSacEnable( bool bEnable )
{
    pSac->status.isSystemAudioModeEnabled = bEnable;
}


//-------------------------------------------------------------------------------------------------
//! @brief    
//-------------------------------------------------------------------------------------------------
void AmTCecSacTask( void )
{	
    xHMISystemParams SysPrams;

    SysPrams = AmTCecGetSyspramsFromHMI();

    // Ignore CEC message under demo mode
    if ( (SysPrams.op_mode != MODE_USER) && (SysPrams.op_mode != MODE_FACTORY) )
    {
        return;
    }

    if ( pSac->taskFlags & CEC_SAC_TASK_MASK_ALL_BUT_BASE )
    {
	SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, true);
    }
    else
    {
	SiiCecCallbackRegisterSet(SiiCecSacTaskProcess, false);
	//SacTaskCounter
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief    
//-------------------------------------------------------------------------------------------------
void AmTSacTaskAssign(uint16_t SysCECSacAppTASK)
{
    pSac->taskFlags |= SysCECSacAppTASK;
    /*if ( SysCECSacAppTASK )
    {
            TRACE_DEBUG((0, "pSac->taskFlags %X", pSac->taskFlags ));
    }*/
}

#endif

void AmTSacSetDestLa(uint8_t la)
{
    pSac->SacTargetLa = la;

}

void CecSacGetReportArcInitiate(void)
{
    bGotReportARCInitiate = true;
}

void CecSacSetStatus(bool bSacOn)
{
    //xHMISystemParams SysParms;   
    //SysParms = AmTCecGetSyspramsFromHMI();

    if(bSacOn)
    {
        //if(!SysParms.cec_sac)
            AmTCecEventPassToHMI( USER_EVENT_CEC_SET_SAC_ON );
        //if(!SiiCecSacIsSysAudioModeEnabled())
            AmTCecSacEnable(TRUE);
    }
    else
    {
        //if(SysParms.cec_sac)
            AmTCecEventPassToHMI( USER_EVENT_CEC_SET_SAC_OFF );
        //if(SiiCecSacIsSysAudioModeEnabled())
            AmTCecSacEnable(FALSE);
    }
}

