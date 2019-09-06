//************************************************************************************************
//! @file     si_cdc_main.c
//! @brief    Top level CDC API
//
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#include "si_common.h"
#include "si_cdc_component.h"
#include "si_cdc_internal.h"

#ifndef CDC_PC_SIMULATION
#include <string.h>    
#else
#include "stdio.h"
#include "memory.h"
#include "debug_print.h"
#endif


//-------------------------------------------------------------------------------------------------
// Global Data
//-------------------------------------------------------------------------------------------------

#ifdef CDC_PC_SIMULATION
uint8_t  g_halMsgLevel = CDC_MSG_DBG;
#endif

//-------------------------------------------------------------------------------------------------
// Local Service Function Prototypes
//-------------------------------------------------------------------------------------------------

static void CdcAliveTimerSet(uint8_t minTimeSec, uint8_t maxTimeSec);
static void CdcLostDeviceDisconnect(uint16_t physAddr);


//-------------------------------------------------------------------------------------------------
// Top Level API Functions
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
//! @brief      Initialize the Instance Record and fills its fields related to
//!             device capabilities and non-volatile configuration.
//!
//!             Alternatively, the Instance Record can be addressed and set
//!             directly.
//!
//! @param[in]  physAddr - physical address of this device,
//! @param[in]  pIoPorts - pointer to I/O configuration structure,
//! @param[in]  hecCapability  - true if the device supports HEC functionality
//! @param[in]  hostCapability - true if the device can be a special service provider (host)
//! @param[in]  extNetCapability - true if the device has external network connection
//-------------------------------------------------------------------------------------------------

void SiiCdcConfig(uint16_t physAddr, CdcIoConfig_t *pIoPorts,  
               bool_t hecCapability, bool_t hostCapability, bool_t extNetCapability)
{
    CdcHecInstanceRecordReset(); // Initialize all Instance Record fields and free all allocated memory

    pCdc->physAddr = physAddr;                             // set PA of this device
    memcpy(&pCdc->ports, pIoPorts, sizeof(CdcIoConfig_t)); // set device I/O configuration

    pCdc->deviceStatus.hecFuncState = hecCapability ? CDC_HEC_STATE_INACTIVE : CDC_HEC_FUNC_NOT_SUPPORTED;
    pCdc->deviceStatus.hostFuncState= hostCapability ? CDC_HOST_STATE_INACTIVE : CDC_HOST_NOT_SUPPORTED;
    pCdc->deviceStatus.encFuncState = extNetCapability ? CDC_ENC_STATE_INACTIVE : CDC_ENC_NOT_SUPPORTED;

    // Broadcast "Capability Notification" message
    CdcHecReportStateSend(0xFFFF, CDC_REPORT_STATE_SHORT_MODE);
    CdcDiscoveryDataArrange(); // update discovered and reachable device list
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enables or Disables HEC Support function of this Device
//!
//! @param[in]  isEnabled - true enables HEC support capability
//-------------------------------------------------------------------------------------------------

void SiiCdcDeviceHecSupEnable(const bool_t isEnabled)
{
    uint8_t oldState = pCdc->deviceStatus.hecFuncState;

    if (isEnabled && (oldState == CDC_HEC_FUNC_NOT_SUPPORTED))
    {
        pCdc->deviceStatus.hecFuncState = CDC_HEC_STATE_INACTIVE;       
        // don't change HEC state if already enabled (either Inactive or Active)
    }
    else if (!isEnabled)
    {
        SiiCdcDeviceDisconnect(true,0,0,0); // disconnect all active HECs
        pCdc->deviceStatus.hecFuncState = CDC_HEC_FUNC_NOT_SUPPORTED;        
    }

    if (pCdc->deviceStatus.hecFuncState != oldState)
    {
        // Broadcast "Capability Notification" message
        // only if the state has been changed
        CdcHecReportStateSend(0xFFFF, 0);
        CdcDiscoveryDataArrange(); // update discovered and reachable device list        
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sets new Host Capability state of this Device
//!
//! @param[in]  state - new Host capability state
//-------------------------------------------------------------------------------------------------

void SiiCdcDeviceHostStateSet(const CdcHostFuncState_t state)
{

    if (pCdc->deviceStatus.hostFuncState != state)
    {
        pCdc->deviceStatus.hostFuncState = state;
        // Broadcast "Capability Notification" message
        // only if the state has been changed
        CdcHecReportStateSend(0xFFFF, 0);           
        CdcDiscoveryDataArrange(); // update discovered and reachable device list
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sets new External Network Capability state of this Device
//!
//! @param[in]  state - new External Network capability state
//-------------------------------------------------------------------------------------------------

void SiiCdcDeviceEncStateSet(const CdcEncFuncState_t state)
{

    if (pCdc->deviceStatus.encFuncState != state)
    {
        pCdc->deviceStatus.encFuncState = state;
        // Broadcast "Capability Notification" message
        // only if the state has been changed
        CdcHecReportStateSend(0xFFFF, 0); 
        CdcDiscoveryDataArrange(); // update discovered and reachable device list
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enables or Disables HEC Support function on I/O ports
//!
//! @param[in]  isOutput       - true, if output port is addressed
//! @param[in]  inputPortIndex - input port index (zero-based)
//! @param[in]  isEnabled      - boolean flag
//-------------------------------------------------------------------------------------------------

void SiiCdcPortHecSupEnable(const bool_t isOutput, const uint8_t inputPortIndex, const bool_t isEnabled)
{
    bool_t    isChanged = false;
    uint16_t  physAddr;

    if ((isOutput == true) && (pCdc->ports.numberOfOutputs != 0))
    {
        if (pCdc->ports.output.hecSupport != isEnabled)
        {
            // before disabling output, deactivate affected AHECs
            if (!isEnabled) 
            {
                physAddr = CdcAdjacentPhysAddrGet(pCdc->physAddr, true, 0);
                if (physAddr != 0xFFFF)
                {
                    CdcLostDeviceDisconnect(physAddr);
                }
            }
            
            pCdc->ports.output.hecSupport = isEnabled;
            isChanged = true;
        }
    }
    else if ((isOutput == false) && (pCdc->ports.numberOfInputs > inputPortIndex)) // inputs
    {
        if (pCdc->ports.input[inputPortIndex].hecSupport != isEnabled)
        {
            // before disabling input, deactivate affected AHECs
            if (!isEnabled) 
            {
                physAddr = CdcAdjacentPhysAddrGet(pCdc->physAddr, false, inputPortIndex);
                if (physAddr != 0xFFFF)
                {
                    CdcLostDeviceDisconnect(physAddr);
                }
            }
            
            pCdc->ports.input[inputPortIndex].hecSupport = isEnabled;
            isChanged = true;
        }
    }
    
    if (isChanged)
    {
        // Broadcast "Capability Notification" message
        // only if the state has been changed
        CdcHecReportStateSend(0xFFFF, 0);          
        CdcDiscoveryDataArrange(); // update discovered and reachable device list
    }
}
  
    
//-------------------------------------------------------------------------------------------------
//! @brief      Resets all tasks
//-------------------------------------------------------------------------------------------------

void SiiCdcTaskInit(void)
{        
    pCdc->cdcTaskFlags = 0;
    pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // all tasks are unmasked by default

    // Reset time counters
    CdcTimeCounterReset(&pCdc->cdcTaskTimeCount);
    CdcTimeCounterReset(&pCdc->aliveNtfDelayCount);
    CdcTimeCounterReset(&pCdc->cdcDiscoveryTimeCount);
    

    // Reset task states
    pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE;     

    Mrand(pCdc->physAddr); // seed the random numbers generator

    pCdc->sysTimerCountCurMs = 0;
    pCdc->sysTimerCountPrvMs = 0;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sets random initial delay time
//!
//! @param[in]  minTimeSec - lower boundary of a time (in seconds)
//! @param[in]  maxTimeSec - upper boundary of a time (in seconds)
//-------------------------------------------------------------------------------------------------

static void CdcAliveTimerSet(uint8_t minTimeSec, uint8_t maxTimeSec)
{
    if (minTimeSec <= maxTimeSec)
    {
        // Initial delay of AliveNotification messages (Tmin...Tmax randomly chosen)
        CdcTimeCounterSet(CDC_TIME_SEC2TCK(minTimeSec + Mrand(0) % (maxTimeSec - minTimeSec + 1)), &pCdc->aliveNtfDelayCount);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      If no HEC devices, disable HEC hardware.
//-------------------------------------------------------------------------------------------------

static void CdcHecDisableCheck(void)
{
    if (pCdc->numberOfHecs == 0) // not participating in any HEC
    {
         // Deactivate HEC function
         if (CdcEhdmiHwEnable(false))
         {
            if (pCdc->deviceStatus.hecFuncState != CDC_HEC_NOT_SUPPORTED)  
            {
                pCdc->deviceStatus.hecFuncState = CDC_HEC_STATE_INACTIVE; 
            }
         }
    } 
}


//-------------------------------------------------------------------------------------------------
//! @brief      Auxiliary function for Base Service
//!
//!             Adds new device into Found Host List in response to CDCOP_HEC_REPORT_STATE message
//!
//! @param[in]  pCdcMessage         - pointer to CDC message structure
//! @param[in]  initiatorPhysAddr   - physical address of the new device
//-------------------------------------------------------------------------------------------------

static void CdcHecAddNewDeviceToHostList(SiiCpiData_t *pCdcMessage, uint16_t initiatorPhysAddr)
{

    if ((pCdc->numberOfAllHosts < CDC_NUMBER_OF_FOUND_HOSTS_MAX) &&        // There is space in the device list
        (((pCdcMessage->args[5] >> 6) & 0x03) != CDC_HEC_FUNC_NOT_SUPPORTED) && // HEC function is available  
        ((pCdcMessage->args[5] & 0x03) == CDC_ERRCODE_NO_ERROR)) // the ReportState has no indication of a error
    {
        // add new device to the list
        pCdc->cdcFoundHostsList[pCdc->numberOfAllHosts].physAddr = initiatorPhysAddr;
        pCdc->cdcFoundHostsList[pCdc->numberOfAllHosts].hostInfo.portHecCapBits = 0x7FFF & ((pCdcMessage->args[6] << 8) | pCdcMessage->args[7]);
        pCdc->cdcFoundHostsList[pCdc->numberOfAllHosts].devStat.hecFuncState  = ((pCdcMessage->args[5] >> 6) & 0x03);                                      
        pCdc->cdcFoundHostsList[pCdc->numberOfAllHosts].devStat.hostFuncState = ((pCdcMessage->args[5] >> 4) & 0x03); 
        pCdc->cdcFoundHostsList[pCdc->numberOfAllHosts].devStat.encFuncState  = ((pCdcMessage->args[5] >> 2) & 0x03);                                  
        pCdc->cdcFoundHostsList[pCdc->numberOfAllHosts].devStat.cdcErrorState =  (pCdcMessage->args[5] & 0x03);                                      

        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x added to found Hosts list\n", initiatorPhysAddr);
        
        pCdc->numberOfAllHosts++;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Launch discovery process
//!
//!             When done, create a list of found devices
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CdcTaskDiscover(void)
{
    uint8_t cdcFeedbackMsg = CDC_FB_MSG_NONE;
    
    
    switch(pCdc->cdcTaskState)
    {
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pCdc->cdcTaskMask = CDC_TASK_HOSTS_SEARCH; // mask out other tasks until this one is done
                pCdc->cdcTaskState = CDC_TASK_FIND_STATE_DISCOVERY_START;
             
                // set Discovery repetition timer
                CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MIN_DISCOVERY_REP_TIME_SEC), &pCdc->cdcDiscoveryTimeCount);
                // don't break here to save time and go ahead to the fist step
                
        case CDC_TASK_FIND_STATE_DISCOVERY_START: // First stage: Sending out a discovery message
                CdcHecDiscoverSend();
                pCdc->numberOfFoundHosts = 0; // reset counter of found devices (ENC or Host capable)
                pCdc->numberOfAllHosts = 0;   // reset counter of all found devices
                pCdc->cdcTaskState = CDC_TASK_FIND_STATE_GATHER_RESPONSES; // next stage
                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_FIND]: Started gathering responses\n");
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);
                CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MAX_MONITORING_TIME_SEC), &pCdc->cdcTotalMonTimeCount);
                break;
        
        case CDC_TASK_FIND_STATE_GATHER_RESPONSES: // Stage: collect responses from all devices
                // wait until all devices respond
                // criteria: timeout since the last response
    
                // Set recall timer to maximum in order to allow the task counters to set it to a new value
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
                
                CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);
                CdcTimeCounterUpdate(&pCdc->cdcTotalMonTimeCount);
                if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount) || CdcIsTimeCounterExpired(&pCdc->cdcTotalMonTimeCount))
                {   // if timer expired
                    pCdc->cdcTaskState = CDC_TASK_FIND_STATE_REPORT_RESULTS;  // next stage
                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_FIND]: Stopped gathering responses\n");
                }
                break;
    
        case CDC_TASK_FIND_STATE_REPORT_RESULTS: // Last stage
                // Identify all found HOST or ENC capable devices that aren't reachable from this device
                // then cleanup and rearrange the list to leave only capable & reachable devices
                CdcDiscoveryDataArrange();
    
                // event message to the caller
                cdcFeedbackMsg = CDC_FB_MSG_HST_SEARCH_DONE;
                pCdc->cdcTaskFlags &= ~CDC_TASK_HOSTS_SEARCH; // stop the task
                pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE;  // reset stage                    
                pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED;  // reset task mask as this task is done
                break;
    
        default:
				 break;
    }

    return  cdcFeedbackMsg;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Activate one or more HECs
//!
//!             Sends InquireState message to all PHECs and
//!             checks if the PHECs have responded, then update PHECs status
//!             to VHEC. Initiate activation of all VHECs and check if the
//!             VHECs have responded, then update VHECs status to AHEC
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CdcTaskConnect(void)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;
    uint16_t    firstPhysAddr;
    uint16_t    lastPhysAddr;
    bool_t      tmpCond;
    uint8_t     activatorExcludeFactor;
    uint8_t     i;

    switch(pCdc->cdcTaskState)
    {
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pCdc->cdcTaskMask = CDC_TASK_CONNECT; // mask out other tasks until this one is done
                pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INIT;
                // don't break here to save time and go ahead to the fist step
        
        case CDC_TASK_CONNECT_STATE_INIT:
                pCdc->currentHecIdx = 0;   // initialize HEC index
                pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INQUIRE_START; // next stage
                pCdc->connectSettings.hecActDelayed = 0;   // reset number of delayed multi-HEC SetState messages
                pCdc->connectSettings.isEndOfList = false; // reset End of VHEC List flag
                //break;
                
        case CDC_TASK_CONNECT_STATE_INQUIRE_START: // First stage: Sending out InquireState messages for PHECs
                // find first PHEC in the HEC records and start activation of it
                while (pCdc->currentHecIdx < CDC_NUMBER_OF_HECS_MAX)
                {
                    if ((pCdc->aHec[pCdc->currentHecIdx].status.isInUse == true) && 
                        (pCdc->aHec[pCdc->currentHecIdx].status.channelState == CDC_PHEC))
                    {
                        // Found a PHEC; begin activation process. Increment the current HEC index when done
                        CdcHecInquireStateSend(pCdc->currentHecIdx);
                        CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);                            
                        CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MAX_MONITORING_TIME_SEC), &pCdc->cdcTotalMonTimeCount);
                        pCdc->verifiedDevNum[0] = 0; // reset counter of verified devices
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_GATHER_INQ_REP; // next stage
                        break; 
                    }
                    pCdc->currentHecIdx++;
                }
         
                if (pCdc->cdcTaskState == CDC_TASK_CONNECT_STATE_GATHER_INQ_REP) // if found new PHEC
                {                        
                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Started gathering responses for PHEC %d\n", (int) pCdc->currentHecIdx);
                }
                else // no new PHEC was found
                {

                    if (pCdc->connectSettings.isMultHecMode && (pCdc->connectSettings.hecActDelayed > 0))
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: End of PHEC list found\n");
                        pCdc->connectSettings.isEndOfList = true;
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_ACTIVATE_START; // send last activation command                            
                    }
                    else // single HEC activation mode
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: No new PHEC found. Task finished\n");
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_REPORT_RESULTS; // stop the task                            
                    }
                }
                break;

        case CDC_TASK_CONNECT_STATE_GATHER_INQ_REP:
                // wait until all devices respond
                // criteria: timeout since the last response or arrival of the last verified report 

                // Set recall timer to maximum in order to allow the task counters to set it to a new value
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);

                CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);
                CdcTimeCounterUpdate(&pCdc->cdcTotalMonTimeCount);
                
                firstPhysAddr = pCdc->aHec[pCdc->currentHecIdx].firstDevPhysAddr;
                lastPhysAddr  = pCdc->aHec[pCdc->currentHecIdx].lastDevPhysAddr; 
                
                // Exclude activator (this device) from the HEC lengh formula, if it's a part of the HEC
                activatorExcludeFactor = CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, pCdc->physAddr) ? 1 : 0;

                tmpCond = (pCdc->verifiedDevNum[0] == (CdcHecLengthGet(firstPhysAddr, lastPhysAddr) - activatorExcludeFactor)); // "-1" means activator exclusion
                if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount) || CdcIsTimeCounterExpired(&pCdc->cdcTotalMonTimeCount) || tmpCond)
                {   // if timer expired or all device responses have been collected
                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Stopped gathering InquireState responses\n");
                    
                    if (tmpCond)
                    {
                        pCdc->aHec[pCdc->currentHecIdx].status.channelState = CDC_VHEC;
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: HEC %d status has changed to VHEC.\n", (int) pCdc->currentHecIdx);
                     
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_ACTIVATE_START; // next stage
                    }
                    else
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: PHEC %d wasn't verified. Removed.\n", (int) pCdc->currentHecIdx);
                        // Remove the failed PHEC
                        CdcHecDescriptorRemove(pCdc->currentHecIdx);
                        pCdc->currentHecIdx++; // step to next HEC
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INQUIRE_START; // move to another PHEC search                        
                    }

                    // minimize the time of the next call
                    CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MIN_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
                }
                break;

        case CDC_TASK_CONNECT_STATE_ACTIVATE_START:
                if (pCdc->connectSettings.isMultHecMode)
                {
                    // Issue multi-HEC activation command, if there are 4 HECs to activate or the End of HEC list is reached
                    if ((pCdc->connectSettings.hecActDelayed >= 3) || (pCdc->connectSettings.isEndOfList))
                    {
                        if (!pCdc->connectSettings.isEndOfList) // new HEC has to be added
                        {
                            pCdc->delayedHecIdx[pCdc->connectSettings.hecActDelayed++] = pCdc->currentHecIdx; // put current HEC into the list
                        }

                        if (pCdc->connectSettings.hecActDelayed > 0)
                        {
                            CdcHecSetStateSend(CDC_HEC_ACTIVE, pCdc->connectSettings.hecActDelayed, pCdc->delayedHecIdx);
                            DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Started gathering responses for VHECs in multi-HEC mode\n");
                        }
                    }
                    else // delay the activation, memorize delayed HEC index
                    {
                        pCdc->delayedHecIdx[pCdc->connectSettings.hecActDelayed] = pCdc->currentHecIdx++;
                        pCdc->connectSettings.hecActDelayed++;
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INQUIRE_START; // move to another PHEC search 
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Activation delayed for %d VHECs\n", (int) pCdc->connectSettings.hecActDelayed);
                        break;
                    }
                }
                else
                {
                    // Issue single-HEC activation command
                    CdcHecSetStateSend(CDC_HEC_ACTIVE, 1, &pCdc->currentHecIdx);
                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Started gathering responses for VHEC %d\n", (int) pCdc->currentHecIdx);
                }
                
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);                   
                CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MAX_MONITORING_TIME_SEC), &pCdc->cdcTotalMonTimeCount);
                
                for (i = 0; i < 4; i++)
                {
                    pCdc->verifiedDevNum[i] = 0; // reset counters of verified devices
                }
                pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_GATHER_ACT_REP; // next stage
                break;
                
        case CDC_TASK_CONNECT_STATE_GATHER_ACT_REP:
                // wait until all devices respond
                // criteria: timeout since the last response or arrival of the last verified report 

                // Set recall timer to maximum in order to allow the task counters to set it to a new value
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
                
                CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);                    
                CdcTimeCounterUpdate(&pCdc->cdcTotalMonTimeCount);

                if (pCdc->connectSettings.isMultHecMode)
                {
                    tmpCond = false;
                    for (i = 0; i < pCdc->connectSettings.hecActDelayed; i++)
                    {
                        // Exclude activator (this device) from the HEC length formula, if it's a part of the HEC
                        activatorExcludeFactor = CdcIsDeviceBelongsToHec(pCdc->aHec[pCdc->delayedHecIdx[i]].firstDevPhysAddr, 
                                                                         pCdc->aHec[pCdc->delayedHecIdx[i]].lastDevPhysAddr, 
                                                                         pCdc->physAddr) ? 1 : 0;                        

                        tmpCond &= (pCdc->verifiedDevNum[i] == 
                                    (CdcHecLengthGet(pCdc->aHec[pCdc->delayedHecIdx[i]].firstDevPhysAddr, 
                                                     pCdc->aHec[pCdc->delayedHecIdx[i]].lastDevPhysAddr) - activatorExcludeFactor));// "-1" means activator exclusion
                    }

                    if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount) || CdcIsTimeCounterExpired(&pCdc->cdcTotalMonTimeCount) || tmpCond)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Stopped gathering Activation responses in Multi-HEC mode\n");
                        
                        // Change current HEC status to AHEC if all devices in the HEC are verified to be active
                        // do it for all HECs in the list
                        for (i = 0; i < pCdc->connectSettings.hecActDelayed; i++)
                        {
                            // Exclude activator (this device) from the HEC lengh formula, if it's a part of the HEC
                            activatorExcludeFactor = CdcIsDeviceBelongsToHec(pCdc->aHec[pCdc->delayedHecIdx[i]].firstDevPhysAddr, 
                                                                             pCdc->aHec[pCdc->delayedHecIdx[i]].lastDevPhysAddr, 
                                                                             pCdc->physAddr) ? 1 : 0;                        
                            
                            tmpCond = (pCdc->verifiedDevNum[i] == 
                                       (CdcHecLengthGet(pCdc->aHec[pCdc->delayedHecIdx[i]].firstDevPhysAddr, 
                                                        pCdc->aHec[pCdc->delayedHecIdx[i]].lastDevPhysAddr) - activatorExcludeFactor));// "-1" means activator exclusion
                            if (tmpCond) // all devices in this HEC have responded positively
                            {
                                bool_t isHwEnabledOk = true;
                                
                                // Activate eHDMI function
                                if (pCdc->deviceStatus.hecFuncState < CDC_HEC_STATE_ACTIVE)
                                {
                                    isHwEnabledOk = CdcEhdmiHwEnable(true);
                                    if (isHwEnabledOk)
                                    {
                                        pCdc->deviceStatus.hecFuncState = CDC_HEC_STATE_ACTIVE;
                                        CdcAliveTimerSet(10, 50); // start first AliveNotification message in 10-50 seconds
                                    }
                                }
                                
                                if (isHwEnabledOk)
                                {
                                    CdcAliveDeviceListAdd(pCdc->delayedHecIdx[i]); // add HEC devices (but Activator) to the Alive List
                                    pCdc->aHec[pCdc->delayedHecIdx[i]].status.channelState = CDC_AHEC;
                                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: HEC %d status has changed to AHEC.\n", (int) pCdc->delayedHecIdx[i]);
                                    pCdc->delayedHecIdx[i] = 0xFF; // mark as empty
                                }
                                
                            }
                            else
                            {
                                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: VHEC %d wasn't activated. Will be deactivated.\n", (int) pCdc->delayedHecIdx[i]);
                                // Leave values pCdc->delayedHecIdx[i] of failed HECs intact
                                // to be used on clean-up stage
                            }
                        }

                        // clean up failed HECs, if any
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_REMOVE_FAILED;

                        // minimize the time of the next call
                        CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MIN_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);                            
                     }
                    
                }
                else // single HEC connection mode
                {
                    firstPhysAddr = pCdc->aHec[pCdc->currentHecIdx].firstDevPhysAddr;
                    lastPhysAddr  = pCdc->aHec[pCdc->currentHecIdx].lastDevPhysAddr; 

                    // Exclude activator (this device) from the HEC lengh formula, if it's a part of the HEC
                    activatorExcludeFactor = CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, pCdc->physAddr) ? 1 : 0;                    
                    
                    tmpCond = (pCdc->verifiedDevNum[0] == (CdcHecLengthGet(firstPhysAddr, lastPhysAddr) - activatorExcludeFactor)); // "-1" means activator exclusion
                    if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount) || tmpCond)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Stopped gathering Activation responses\n");
                        // Change current HEC status to AHEC if all devices in the HEC are verified to be active
                         if (tmpCond) 
                         {
                            bool_t isHwEnabledOk = true;
                            
                            // Activate eHDMI function
                            if (pCdc->deviceStatus.hecFuncState < CDC_HEC_STATE_ACTIVE)
                            {
                                isHwEnabledOk = CdcEhdmiHwEnable(true);
                                if (isHwEnabledOk)
                                {
                                    pCdc->deviceStatus.hecFuncState = CDC_HEC_STATE_ACTIVE;
                                    CdcAliveTimerSet(10, 50); // start first AliveNotification message in 10-50 seconds
                                }                                
                            }

                            if (isHwEnabledOk)
                            {
                                CdcAliveDeviceListAdd(pCdc->currentHecIdx); // add HEC devices (but Activator) to the Alive List
                                pCdc->aHec[pCdc->currentHecIdx].status.channelState = CDC_AHEC;
                                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: HEC %d status has changed to AHEC.\n", (int) pCdc->currentHecIdx);
                            }
                            
                         }
                         else
                         {
                             DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: VHEC %d wasn't activated. Will be deactivated.\n", (int) pCdc->currentHecIdx);
                             
                             CdcHecSetStateSend(CDC_HEC_INACTIVE, 1, &pCdc->currentHecIdx); // deactivation of failed VHEC    
                             CdcHecDescriptorRemove(pCdc->currentHecIdx); // removal of failed VHEC
                             pCdc->currentHecIdx++; // step to next HEC
                         }
                         pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INQUIRE_START; // move to another PHEC search                    
                    }
                }
                break;

        case CDC_TASK_CONNECT_STATE_REMOVE_FAILED:
                // find all HECs from the list that failed to activate
                tmpCond = false;
                for (i = 0; i < pCdc->connectSettings.hecActDelayed; i++)
                {
                    if (pCdc->delayedHecIdx[i] != 0xFF)
                    {
                        CdcHecSetStateSend(CDC_HEC_INACTIVE, 1, &pCdc->delayedHecIdx[i]); // deactivation of failed VHEC    
                        CdcHecDescriptorRemove(pCdc->delayedHecIdx[i]); // removal of failed VHEC
                        pCdc->delayedHecIdx[i] = 0xFF;
                        tmpCond = true; // indication of a found failed HEC
                        break;
                    }
                }

                if (!tmpCond) // if all failed HECs have been sanitized
                {
                    pCdc->connectSettings.hecActDelayed = 0; // reset number of delayed HECs (all have been processed)
                    if (pCdc->connectSettings.isEndOfList)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT]: Task finished (Multi-HEC mode)\n");
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_REPORT_RESULTS; // end the task                               
                    }
                    else
                    {
                        pCdc->currentHecIdx++; // step to next HEC
                        pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INQUIRE_START; // move to another PHEC search  
                    }
                }
                else
                {
                    pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_REMOVE_FAILED; // come back to continue  
                }
                break;

        case CDC_TASK_CONNECT_STATE_REPORT_RESULTS:
                CdcIoPortsStatusUpdate(); // activate ports involved in the new AHECs 
                pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE; // reset stage
                pCdc->cdcTaskFlags &= ~CDC_TASK_CONNECT; // stop the task
                pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done     
                cdcFeedbackMsg = CDC_FB_MSG_CONNECT_DONE;                    
                break;
        default:
				 break;
    }

    return cdcFeedbackMsg;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Establishes connection to adjacent device using
//!             simplified activation method: control to adjacent device
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CdcTaskConnectAdjacent(void)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;
    bool_t      tmpCond;

    switch(pCdc->cdcTaskState)
    {
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pCdc->cdcTaskMask = CDC_TASK_CONNECT_ADJACENT; // mask out other tasks until this one is done
                pCdc->cdcTaskState = CDC_TASK_CONNECT_ADJ_STATE_INIT;
                // don't break here to save time and go ahead to the fist step
        
        case CDC_TASK_CONNECT_ADJ_STATE_INIT:
                pCdc->cdcTaskState = CDC_TASK_CONNECT_ADJ_STATE_ACTIVATE_START; // next stage
                pCdc->verifiedDevNum[0] = 0;
                //break;
                
        case CDC_TASK_CONNECT_ADJ_STATE_ACTIVATE_START:
                // Issue activation command
                CdcHecSetStateAdjacentSend(CDC_HEC_ACTIVE, pCdc->currentHecIdx);
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);
                CdcTimeCounterSet(CDC_TIME_SEC2TCK(CDC_MAX_MONITORING_TIME_SEC), &pCdc->cdcTotalMonTimeCount);
                pCdc->cdcTaskState = CDC_TASK_CONNECT_ADJ_STATE_WAIT_ACT_REP; // next stage
                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT_ADJACENT]: Waiting for response for HEC %d\n", (int) pCdc->currentHecIdx);
                break;
                
        case CDC_TASK_CONNECT_ADJ_STATE_WAIT_ACT_REP:
                // wait until adjacent device respond

                // Set recall timer to maximum in order to allow the task counters to set it to a new value
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);

                CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);                    
                CdcTimeCounterUpdate(&pCdc->cdcTotalMonTimeCount);

                tmpCond = (pCdc->verifiedDevNum[0] == 1);
                
                if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount) || CdcIsTimeCounterExpired(&pCdc->cdcTotalMonTimeCount) || tmpCond)
                {
                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT_ADJACENT]: Stopped waiting for Activation response\n");
                     // Change curent HEC status to AHEC if adjacent devices is active
                     if (tmpCond) 
                     {
                         // Activate eHDMI function
                         if (CdcEhdmiHwEnable(true))
                         {
                             pCdc->aHec[pCdc->currentHecIdx].status.channelState = CDC_AHEC;
                             pCdc->deviceStatus.hecFuncState = CDC_HEC_STATE_ACTIVE;
                             DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT_ADJACENT]: HEC %d status has changed to AHEC.\n", (int) pCdc->currentHecIdx);
                         }
                     }
                     else
                     {
                         DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_CONNECT_ADJACENT]: HEC %d wasn't activated. Removed.\n", (int) pCdc->currentHecIdx);
                         CdcHecDescriptorRemove(pCdc->currentHecIdx); // removal of failed VHEC
                     }
                     pCdc->cdcTaskState = CDC_TASK_CONNECT_ADJ_STATE_REPORT_RESULTS; // move to another PHEC search                    

                     // minimize the time of the next call
                     CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MIN_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);  
                }
                break;

        case CDC_TASK_CONNECT_ADJ_STATE_REPORT_RESULTS:
                CdcIoPortsStatusUpdate(); // activate ports involved in the new AHECs 
                pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE; // reset stage
                pCdc->cdcTaskFlags &= ~CDC_TASK_CONNECT_ADJACENT; // stop the task
                pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done     
                cdcFeedbackMsg = CDC_FB_MSG_CONNECT_ADJ_DONE;                    
                break;
        default:
				 break;
    }

    return cdcFeedbackMsg;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Deactivation of an AHEC by Activator request is done by sending
//!             out SetState message (no response gathering)
//!
//!             Deactivation of an AHEC by Passive device is done by sending
//!             RequestDeactivation message
//!             Task approach is needed if more than one AHEC has to be deactivated
//!
//! @return      Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CdcTaskDisconnect(void)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;

    switch(pCdc->cdcTaskState)
    {    
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pCdc->cdcTaskMask = CDC_TASK_DISCONNECT; // mask out other tasks until this one is done
                pCdc->cdcTaskState = CDC_TASK_DISCONNECT_STATE_DEACTIVATE;
                // don't break here to save time and go ahead to the fist step
        
        case CDC_TASK_DISCONNECT_STATE_DEACTIVATE:
            
                if (pCdc->aHec[pCdc->currentHecIdx].status.isActivator == true)
                {
                    // Activator deactivates AHEC by sending SetState command
                    CdcHecSetStateSend(CDC_HEC_INACTIVE, 1, &pCdc->currentHecIdx); // one HEC at a time
                    CdcAliveDeviceListRemove(pCdc->currentHecIdx); // remove devices from Alive List (if possible)                        
                    CdcHecDescriptorRemove(pCdc->currentHecIdx); // release resources for HEC
                    CdcHecDisableCheck(); // disable HEC hardware if no active HECs
                }
                else
                {
                    // Passive device must request deactivation from the Activator
                    CdcHecDeactivationRequestSend(pCdc->currentHecIdx);
                }
                
                pCdc->cdcTaskState = CDC_TASK_DISCONNECT_STATE_REPORT_RESULTS; // next stage
                break;
                
        case CDC_TASK_DISCONNECT_STATE_REPORT_RESULTS:                
                CdcIoPortsStatusUpdate(); // deactivate ports involved in the deactivated HEC
                pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE; // reset stage
                pCdc->cdcTaskFlags &= ~CDC_TASK_DISCONNECT; // stop the task
                pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done    
                cdcFeedbackMsg = CDC_FB_MSG_DISCONNECT_DONE;                    
                break;
        default:
				 break;
    }
    
    return cdcFeedbackMsg;    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Deactivates all currently open AHECs
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CdcTaskDisconnectAll(void)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;

    switch(pCdc->cdcTaskState)
    {
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pCdc->cdcTaskMask = CDC_TASK_DISCONNECT_ALL; // mask out other tasks until this one is done
                pCdc->cdcTaskState = CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE_INIT;
                // don't break here to save time and go ahead to the fist step
        
        case CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE_INIT:
                pCdc->currentHecIdx = 0;   // initialize HEC index
                pCdc->cdcTaskState = CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE; // next stage
                //break;

        case CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE:

                while (pCdc->currentHecIdx < CDC_NUMBER_OF_HECS_MAX)
                {
                    if ((pCdc->aHec[pCdc->currentHecIdx].status.isInUse == true) && 
                        (pCdc->aHec[pCdc->currentHecIdx].status.channelState > CDC_PHEC))
                    {
                        // Found AHEC or VHEC; begin deactivation process. Increment the current HEC index when done                          
                        if (pCdc->aHec[pCdc->currentHecIdx].status.isActivator == true)
                        {
                            // Activator deactivates AHEC by sending SetState command
                            // Must check if CEC write FIFO isn't full before sending out a new message
                            CdcHecSetStateSend(CDC_HEC_INACTIVE, 1, &pCdc->currentHecIdx); // one HEC at a time
                            CdcAliveDeviceListRemove(pCdc->currentHecIdx); // remove devices from Alive List (if possible)                                
                            CdcHecDescriptorRemove(pCdc->currentHecIdx); // release resources for the HEC                                
                            CdcHecDisableCheck(); // disable HEC hardware if no active HECs
                            
                            // Then it's necessary to wait for some time to let the Activator's message to get out (?)
                            CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MSG_OUT_TIME_MS), &pCdc->cdcTaskTimeCount);
                            pCdc->cdcTaskState = CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE_WAIT;                                
                        }
                        else
                        {
                            // Passive device must request deactivation from the Activator
                            CdcHecDeactivationRequestSend(pCdc->currentHecIdx);  
                            // Then it's necessary to wait for MRT to let the Activator respond with Deactivation message
                            CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);
                            pCdc->cdcTaskState = CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE_WAIT;
                        }

                        pCdc->currentHecIdx++;
                        break; // do one message sending at a time
                    }
                    pCdc->currentHecIdx++;
                }                

                if (pCdc->currentHecIdx == CDC_NUMBER_OF_HECS_MAX) // if all HECs have been processed
                {
                    pCdc->cdcTaskState = CDC_TASK_DISCONNECT_ALL_STATE_REPORT_RESULTS; // next stage
                }
                break;

        case CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE_WAIT:
                // wait until the Activator respond
                // criteria: timeout since the DeactivationRequest message sending 

                // Set recall timer to maximum in order to allow the task counters to set it to a new value
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
                
                CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);
                
                if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount))
                {
                    
                    pCdc->cdcTaskState = CDC_TASK_DISCONNECT_ALL_STATE_DEACTIVATE; // return from waiting
                }
                break;

        case CDC_TASK_DISCONNECT_ALL_STATE_REPORT_RESULTS:
                CdcIoPortsStatusUpdate(); // deactivate ports involved in the deactivated HEC
                pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE; // reset stage
                pCdc->cdcTaskFlags &= ~CDC_TASK_DISCONNECT_ALL; // stop the task
                pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done                  
                cdcFeedbackMsg = CDC_FB_MSG_DISCONNECT_ALL_DONE;
                break;    
        default:
				 break;
    }
    
    return cdcFeedbackMsg;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Deactivate all AHECs including a failed device
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CdcTaskDisconnectLost(void)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;

    switch(pCdc->cdcTaskState)
    {
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pCdc->cdcTaskMask = CDC_TASK_DISCONNECT_LOST; // mask out other tasks until this one is done
                pCdc->cdcTaskState = CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE_INIT;
                // don't break here to save time and go ahead to the fist step
        
        case CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE_INIT:
                 pCdc->currentHecIdx = 0;   // initialize HEC index
                 pCdc->cdcTaskState = CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE; // next stage
                 //break;
        
         case CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE:
        
                 while (pCdc->currentHecIdx < CDC_NUMBER_OF_HECS_MAX)
                 {
                     if ((pCdc->aHec[pCdc->currentHecIdx].status.isInUse == true) && 
                         (pCdc->aHec[pCdc->currentHecIdx].status.channelState > CDC_PHEC))
                     {
                        if (((pCdc->aHec[pCdc->currentHecIdx].status.isActivator == true) &&
                             CdcIsDeviceBelongsToHec(pCdc->aHec[pCdc->currentHecIdx].firstDevPhysAddr,
                                                     pCdc->aHec[pCdc->currentHecIdx].lastDevPhysAddr,
                                                     pCdc->aliveDevPhysAddr)) || 
                            (pCdc->aHec[pCdc->currentHecIdx].activatorPhysAddr == pCdc->aliveDevPhysAddr))
                         {
                             // Found AHEC or VHEC that contain the lost device; begin deactivation process. 
                             if (pCdc->aHec[pCdc->currentHecIdx].status.isActivator == true)
                             {
                                 // Activator deactivates AHEC by sending SetState command
                                 // Must check if CEC write FIFO isn't full before sending out a new message
                                 CdcHecSetStateSend(CDC_HEC_INACTIVE, 1, &pCdc->currentHecIdx); // one HEC at a time
                                 // Then it's necessary to wait for some time to let the Activator's message to get out (?)
                                 CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MSG_OUT_TIME_MS), &pCdc->cdcTaskTimeCount);
                                 pCdc->cdcTaskState = CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE_WAIT;                                
                             }
                             else
                             {
                                 // Passive device remove HEC containing the lost device without any signalling                           
                                 // No need for waiting, schedule the next cycle to continue immediatelly 
                                 pCdc->cdcTaskState = CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE;
                             }
                             CdcAliveDeviceListRemove(pCdc->currentHecIdx); // remove devices from Alive List (if possible)                                
                             CdcHecDescriptorRemove(pCdc->currentHecIdx); // release resources for the HEC                                
                             CdcHecDisableCheck(); // disable HEC hardware if no active HECs
                             
                             pCdc->currentHecIdx++;
                             break; // do one message sending at a time
                         }
                     }
                     pCdc->currentHecIdx++;
                 }                
        
                 if (pCdc->currentHecIdx == CDC_NUMBER_OF_HECS_MAX) // if all HECs have been processed
                 {  
                    pCdc->cdcTaskState = CDC_TASK_DISCONNECT_LOST_STATE_REPORT_RESULTS; // next stage
                 }
                 break;
        
         case CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE_WAIT:
                 // wait until activator's deactivation message is processed
                 // criteria: timeout since the starting of deactivation 

                 // Set recall timer to maximum in order to allow the task counters to set it to a new value
                 CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
                 
                 CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);
                 
                 if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount))
                 {
                     pCdc->cdcTaskState = CDC_TASK_DISCONNECT_LOST_STATE_DEACTIVATE; // return from waiting
                 }
                 break;
                 
         case CDC_TASK_DISCONNECT_LOST_STATE_REPORT_RESULTS:
                 CdcIoPortsStatusUpdate(); // deactivate ports involved in the deactivated HEC
                 pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE; // reset stage
                 pCdc->cdcTaskFlags &= ~CDC_TASK_DISCONNECT_LOST; // stop the task
                 pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done       
                 cdcFeedbackMsg = CDC_FB_MSG_DISCONNECT_LOST_DONE;                     
                 break;
         default:
				 break;
    }

    return cdcFeedbackMsg;
}


#if (IS_RX == ENABLE)
//-------------------------------------------------------------------------------------------------
//! @brief      Send HPD SetState message and wait until the HDMI Source responds
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

uint8_t CdcTaskHpdSignal(void)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;

    // Send HPD SetState message and wait until the HDMI Source responds 
    switch(pCdc->cdcTaskState)
    {
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                pCdc->cdcTaskMask = CDC_TASK_HPD_SIGNAL; // mask out other tasks until this one is done
                pCdc->cdcTaskState = CDC_TASK_HPD_SIGNAL_STATE_START;
                // don't break here to save time and go ahead to the first step
        
        case CDC_TASK_HPD_SIGNAL_STATE_START: // First stage: Sending an HPD SetState message                  
                CdcHpdSetStateSend(pCdc->currentPortIdx); 
                pCdc->cdcTaskState = CDC_TASK_HPD_SIGNAL_STATE_RESPONSE_WAIT; // next stage
                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_HPD_SIGNAL]: Waiting for HPD ReportState\n");
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);
                break;
        
        case CDC_TASK_HPD_SIGNAL_STATE_RESPONSE_WAIT:
                // wait until Source device responds
                // criteria: MRT timeout 

                // Set recall timer to maximum in order to allow the task counters to set it to a new value
                CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
                
                CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);

                if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount))
                {
                    // stop the task and provide error feedback
                    pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE;  // reset stage
                    pCdc->cdcTaskFlags &= ~CDC_TASK_HPD_SIGNAL;  // stop the task
                    pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done     
                    cdcFeedbackMsg = CDC_FB_MSG_ERR_HPD_SIGNAL;
                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_HPD_SIGNAL]: HDMI Source haven't responded\n");
                }
                break;

        case CDC_TASK_HPD_SIGNAL_STATE_REPORT_RESULTS: 
                pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE;  // reset stage
                pCdc->cdcTaskFlags &= ~CDC_TASK_HPD_SIGNAL; // stop the task
                pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done     
                cdcFeedbackMsg = CDC_FB_MSG_HPD_SIGNAL_DONE; // successful end of the HPD Signalling task                   
                break;

        default:
				 break;
    }

    return cdcFeedbackMsg;
}
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      HPD Capability verification task; sends InqureState Message to
//!             an adjacent device and checks the response
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

uint8_t CdcTaskHpdCapabilityVerification(void)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;

    // HPD Capability verification task
    // Sends InqureState Message to an adjacent device and checks the response
    switch(pCdc->cdcTaskState)
    {
        case CDC_TASK_EMPTY_STATE_NONE: // all tasks shall start from empty state
                 pCdc->cdcTaskMask = CDC_TASK_HPD_CAPAB_VERIF; // mask out other tasks until this one is done
                 pCdc->cdcTaskState = CDC_TASK_HPD_CAPAB_STATE_START;
                 // don't break here to save time and go ahead to the fist step
         
         case CDC_TASK_HPD_CAPAB_STATE_START: // First stage: Sending an InquireState message to adjacent source device                  
                 if (CdcHecInquireStateOfAdjacentSend(false, pCdc->currentPortIdx)) // input port mode
                 { // if error
                     DEBUG_PRINT(CDC_MSG_ALWAYS, "[CDC_TASK_HPD_CAPAB_VERIF]: The adjacent device doesn't exist\n");
                 }
                 pCdc->cdcTaskState = CDC_TASK_HPD_CAPAB_STATE_RESPONSE_WAIT; // next stage
                 DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_HPD_CAPAB_VERIF]: Waiting for HPD capability verification ReportState\n");
                 CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);
                 break;
         
         case CDC_TASK_HPD_CAPAB_STATE_RESPONSE_WAIT:
                 // wait until Source device responds
                 // criteria: MRT timeout 

                 // Set recall timer to maximum in order to allow the task counters to set it to a new value
                 CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
                 
                 CdcTimeCounterUpdate(&pCdc->cdcTaskTimeCount);
        
                 if (CdcIsTimeCounterExpired(&pCdc->cdcTaskTimeCount))
                 {
                     // stop the task and provide error feedback
                     pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE;   // reset stage
                     pCdc->cdcTaskFlags &= ~CDC_TASK_HPD_CAPAB_VERIF;  // stop the task
                     pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED;  // reset task mask as this task is done     
                     cdcFeedbackMsg = CDC_FB_MSG_ERR_HPD_CAP_FAILED;
                     DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_HPD_SIGNAL]: HDMI Source haven't responded to HPD capability verification request\n");
                 }
                 break;
        
         case CDC_TASK_HPD_CAPAB_STATE_REPORT_RESULTS: 
                 pCdc->cdcTaskState = CDC_TASK_EMPTY_STATE_NONE;  // reset stage
                 pCdc->cdcTaskFlags &= ~CDC_TASK_HPD_CAPAB_VERIF; // stop the task
                 pCdc->cdcTaskMask = CDC_TASK_MASK_ALL_UNMASKED; // reset task mask as this task is done     
                 cdcFeedbackMsg = CDC_FB_MSG_HPD_CAPABILITY_CONF; // HPD capability of the adjacent source is confirmed                   
                 break;
         default:
				 break;
    }
        
    return cdcFeedbackMsg;    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Performs CDC message parsing and support basic CDC functionality
//!             of passive devices
//!
//!             This task can't be masked and will run in background of the
//!             other active task, if an incoming CDC message is pending
//!
//! @param[in]  pCdcMessage - pointer to a pending CDC message or 0, if there is no message awaiting
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

static uint8_t CdcTaskBase(SiiCpiData_t *pCdcMessage)
{
    uint8_t     cdcFeedbackMsg = CDC_FB_MSG_NONE;
    uint16_t    initiatorPhysAddr;
    uint16_t    targetPhysAddr;
    uint16_t    firstPhysAddr;
    uint16_t    lastPhysAddr;
    uint8_t     inputPortIndex;
    uint8_t     aInputPortRtIdx[2];
    uint8_t     outputPortRtFlg;
    uint8_t     hecIdx;    
    uint8_t     hecCount;
    uint8_t     numberOfHecs;
    bool_t      tmpCond;
    uint8_t     errCode = CDC_ERRCODE_NO_ERROR;
    uint8_t     i;
    uint8_t     j;


    if (pCdcMessage != 0) //decode a message, update status
    {
        initiatorPhysAddr = (pCdcMessage->args[0] << 8) + pCdcMessage->args[1];

        // check if the message belongs to CDC group and has broadcast destination
        if ((pCdcMessage->opcode == CDC_CEC_OPCODE) && ((pCdcMessage->srcDestAddr & 0xF) == 0xF))
        {
            switch (pCdcMessage->args[2]) // CDC operation code
            {
                case CDCOP_HEC_INQUIRE_STATE:
                    
                    if (pCdcMessage->argCount != 7)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: ERROR in InquireState message: wrong length!\n");
                        break;
                    }
                    
                    firstPhysAddr = (pCdcMessage->args[3] << 8) + pCdcMessage->args[4];
                    lastPhysAddr  = (pCdcMessage->args[5] << 8) + pCdcMessage->args[6];
                    
                    // reply with ReportState message only if the device belongs to the inquired HEC
                    if (CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, pCdc->physAddr))
                    {
                        // report without error if all relevant ports are HEC-capable
                        if (CdcCapablePortGet(firstPhysAddr, lastPhysAddr, aInputPortRtIdx, &outputPortRtFlg))
                        {                            
                            DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received InquireState message from PA = %04x\n", initiatorPhysAddr);
                            if (outputPortRtFlg != 0)
                            {
                                DEBUG_PRINT(CDC_MSG_DBG, "* HEC capable Output port is involved\n");
                            }

                            for (i = 0; i < 2; i++)
                            {
                                if (aInputPortRtIdx[i] != 0xFF)
                                {
                                    DEBUG_PRINT(CDC_MSG_DBG, "* HEC capable Input port %d is involved\n", (int) aInputPortRtIdx[i]);
                                }
                            }
                        }
                        else
                        {
                            // report an error when required ports are HEC incapable
                            pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_NOT_SUPPORTED_CAPABILITY; 
                        }
                        
                        CdcHecReportStateSend(initiatorPhysAddr, CDC_REPORT_STATE_SHORT_MODE);                        
                    }
                    break;
                    
                case CDCOP_HEC_REPORT_STATE: 
                    
                    errCode = pCdcMessage->args[5] & 0x03;
                    
                    if ((pCdcMessage->argCount != 6) && (pCdcMessage->argCount != 8))
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: ERROR in ReportState message: wrong length!\n");
                        break;
                    }
                    else if (errCode != CDC_ERRCODE_NO_ERROR)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x reported an error: ", initiatorPhysAddr);
                        switch (errCode)
                        {
                            case CDC_ERRCODE_NOT_SUPPORTED_CAPABILITY:
                                DEBUG_PRINT(CDC_MSG_DBG, "capability not supported.\n");
                                break;
                            case CDC_ERRCODE_INCOMPATIBLE_STATE:
                                DEBUG_PRINT(CDC_MSG_DBG, "incompatible state.\n");
                                break;
                            case CDC_ERRCODE_OTHER_ERROR:
                                DEBUG_PRINT(CDC_MSG_DBG, "other error.\n");
                                break;
                        }
                        
                        CdcTimeCounterReset(&pCdc->cdcTaskTimeCount); // stop waiting to react immediatelly
                        break;
                    }
                    
                     
                    targetPhysAddr = (pCdcMessage->args[3] << 8) + pCdcMessage->args[4];

                    if ((targetPhysAddr == 0xFFFF) && (pCdcMessage->argCount == 8)) // Capability notification broadcast
                    {
                        DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received Capability Notification message from PA = %04x\n", initiatorPhysAddr);
                        // Check if some of AHECs have to be deactivated if the initiating device reports 
                        // its HEC capability change and takes part in those AHECs

                        // -- Normally, all devices shall disconnect all their open HECs
                        //    before changing HEC support state to not supported

                        // Update found device list taking into account the new capability information
                        tmpCond = false; // not found by default
                        for (i = 0; i < pCdc->numberOfAllHosts; i++)
                        {
                            if (pCdc->cdcFoundHostsList[i].physAddr == initiatorPhysAddr)
                            {
                                if (((pCdcMessage->args[5] >> 6) & 0x03) != CDC_HEC_FUNC_NOT_SUPPORTED)
                                {  // update port capability bits and device status
                                    pCdc->cdcFoundHostsList[i].hostInfo.portHecCapBits = 0x7FFF & ((pCdcMessage->args[6] << 8) | pCdcMessage->args[7]);
                                    pCdc->cdcFoundHostsList[i].devStat.hecFuncState  = ((pCdcMessage->args[5] >> 6) & 0x03);                                      
                                    pCdc->cdcFoundHostsList[i].devStat.hostFuncState = ((pCdcMessage->args[5] >> 4) & 0x03); 
                                    pCdc->cdcFoundHostsList[i].devStat.encFuncState  = ((pCdcMessage->args[5] >> 2) & 0x03);                                  
                                    pCdc->cdcFoundHostsList[i].devStat.cdcErrorState =  (pCdcMessage->args[5] & 0x03);                                                                              
                                }
                                else
                                {
                                    // remove incapable device from the list
                                    for (j = i; j < (pCdc->numberOfAllHosts - 1); j++)
                                    {
                                        pCdc->cdcFoundHostsList[j] = pCdc->cdcFoundHostsList[j+1];
                                    }
                                    pCdc->numberOfAllHosts--;
                                }
                                tmpCond = true; // set found flag
                                break;
                            }
                        }

                        if (!tmpCond) // if initiator haven't been found in the list
                        {
                            CdcHecAddNewDeviceToHostList(pCdcMessage, initiatorPhysAddr);
                        }

                        CdcDiscoveryDataArrange();  

                        // check if initiating device has been excluded from the list of
                        // potentially reachable hosts 
                        tmpCond = true; // will flip to false, if the device have been found in the list 
                        for (i = 0; i < pCdc->numberOfFoundHosts; i++)
                        {
                            if (pCdc->cdcFoundHostsList[i].physAddr == initiatorPhysAddr)
                            {
                                tmpCond = false; // device is still in the list 
                                break;
                            }
                        }
                        // if so, disconnect all HECs that include the failed device
                        // Make sure other tasks are inactive, leave the affected HECs to be 
                        // deactivated by timeout otherwise
                        if (tmpCond)
                        {
                            CdcLostDeviceDisconnect(initiatorPhysAddr);
                        }
                            
                        cdcFeedbackMsg = CDC_FB_MSG_CAPABILITY_CHANGED;
                        break;
                    }
                    else if (targetPhysAddr != pCdc->physAddr)
                    {
                        break; // don't process message addressed to another device
                    }
                    
                    DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received ReportState message from PA = %04x\n", initiatorPhysAddr);
                    
                    // Process ReportState messages for response gathering stage of ENC search task 
                    if (pCdc->cdcTaskState == CDC_TASK_FIND_STATE_GATHER_RESPONSES)
                    {
                        if (pCdc->cdcTaskFlags & CDC_TASK_HOSTS_SEARCH)
                        {
                            // reset the time counter if new ReportState message arrived
                            CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);
                           
                            // Collect HEC & Ext NW capable device PAs if ...
                            if (pCdcMessage->argCount == 8) // Discovery mode of ReportState 
                            {
                                CdcHecAddNewDeviceToHostList(pCdcMessage, initiatorPhysAddr);
                            }
                        }
                    }
                    // Process ReportState messages for InquireState response gathering stage of Connect Task 
                    else if (pCdc->cdcTaskState == CDC_TASK_CONNECT_STATE_GATHER_INQ_REP)
                    {
                        // reset the time counter if new ReportState message arrived
                        CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);

                        // Make sure all devices in tested PHEC report HEC support
                        // if not, the HEC can't obtain VHEC status
                        if (((pCdcMessage->args[5] >> 6) & 0x03) == CDC_HEC_FUNC_NOT_SUPPORTED) // HEC function isn't available
                        {
                            DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x doesn't support HEC function. PHEC %d can't be verified.\n",
                                        initiatorPhysAddr, (int) pCdc->currentHecIdx);
                            CdcHecDescriptorRemove(pCdc->currentHecIdx); // Remove failed PHEC                                
                            pCdc->currentHecIdx++; // step to next HEC
                            pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INQUIRE_START; // move to another PHEC search                            
                            CdcTimeCounterReset(&pCdc->cdcTaskTimeCount); // stop waiting to react immediately
                        }
                        else if (errCode == CDC_ERRCODE_NO_ERROR)
                        {   // add to the number of verified devices, if the device also belongs to HEC
                            firstPhysAddr = pCdc->aHec[pCdc->currentHecIdx].firstDevPhysAddr;
                            lastPhysAddr  = pCdc->aHec[pCdc->currentHecIdx].lastDevPhysAddr;
                            if (CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, initiatorPhysAddr))
                            {
                                pCdc->verifiedDevNum[0]++;                                  
                                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x supports HEC function.\n", initiatorPhysAddr);
                            }
                        }
                    }
                    else if (pCdc->cdcTaskState == CDC_TASK_CONNECT_STATE_GATHER_ACT_REP)
                    {
                        // reset the time counter if new ReportState message arrived
                        CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RESPONSE_TIME_MS), &pCdc->cdcTaskTimeCount);
                        
                        if ((!pCdc->connectSettings.isMultHecMode) && (pCdcMessage->argCount == 6)) // short mode of ReportState
                        {
                            // Make sure all devices in tested VHEC report activated status
                            // if not, the HEC can't obtain AHEC status
                            if (((pCdcMessage->args[5] >> 6) & 0x03) != CDC_HEC_STATE_ACTIVE) // HEC isn't active
                            {
                                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x isn't active. AHEC %d can't be verified.\n",
                                            initiatorPhysAddr, (int) pCdc->currentHecIdx);
                                CdcHecDescriptorRemove(pCdc->currentHecIdx); // Remove failed PHEC
                                pCdc->currentHecIdx++; // step to next HEC
                                pCdc->cdcTaskState = CDC_TASK_CONNECT_STATE_INQUIRE_START; // move to another PHEC search                            
                                CdcTimeCounterReset(&pCdc->cdcTaskTimeCount); // stop waiting to react immediately
                            }
                            else if (errCode == CDC_ERRCODE_NO_ERROR)
                            {   // add to the number of verified devices, if the device also belongs to HEC
                                firstPhysAddr = pCdc->aHec[pCdc->currentHecIdx].firstDevPhysAddr;
                                lastPhysAddr  = pCdc->aHec[pCdc->currentHecIdx].lastDevPhysAddr;
                                if (CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, initiatorPhysAddr))
                                {
                                    pCdc->verifiedDevNum[0]++;                                  
                                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x is HEC-active.\n", initiatorPhysAddr);
                                }
                            }
                            
                        }
                        else if (pCdc->connectSettings.isMultHecMode) // Multi-HEC activation mode
                        {
                            uint8_t hecFuncState = (pCdcMessage->args[5] >> 6) & 0x03;
                            
                            // collect positive responses for every VHEC in multi-VHEC activation command
                            if (((hecFuncState == CDC_HEC_STATE_ACTIVE) && (pCdcMessage->argCount == 6)) ||
                                ((hecFuncState == CDC_HEC_ACTIVATION_FLD) && (pCdcMessage->argCount == 8)))
                            {   // add to the number of verified device
                                for (i = 0; i < pCdc->connectSettings.hecActDelayed; i++)
                                {
                                    if (CdcIsDeviceBelongsToHec(pCdc->aHec[pCdc->delayedHecIdx[i]].firstDevPhysAddr, 
                                                                pCdc->aHec[pCdc->delayedHecIdx[i]].lastDevPhysAddr, 
                                                                initiatorPhysAddr))
                                    {
                                        if (pCdcMessage->argCount == 8) // multi-HEC mode of ReportState (device is part of more than one HECs)
                                        {   // check if ports are active to support this HEC
                                            if (!CdcIsHecEnabledInDev(pCdc->delayedHecIdx[i], initiatorPhysAddr, 
                                                                 (pCdcMessage->args[6] << 8) | pCdcMessage->args[7]))
                                            {
                                                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x ports can't support HEC %d.\n", initiatorPhysAddr, (int) i);
                                                continue; // jump to the next HEC, if failed to confirm port activation
                                            }
                                        }

                                        pCdc->verifiedDevNum[i]++;
                                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x is active for HEC %d.\n", initiatorPhysAddr, (int) i);
                                    }
                                }
                            }
                            else
                            {
                                DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Device PA = %04x isn't active. AHECs can't be verified.\n", initiatorPhysAddr);
                                CdcTimeCounterReset(&pCdc->cdcTaskTimeCount); // stop waiting to react immediately
                            }
                        }
                            
                    }
                    else if (pCdc->cdcTaskState == CDC_TASK_CONNECT_ADJ_STATE_WAIT_ACT_REP)
                    {
                        
                        if (pCdcMessage->argCount == 6) // short mode of ReportState
                        {
                            // Make sure adjacent device reports activated status
                            // if not, the HEC can't obtain AHEC status
                            if (((pCdcMessage->args[5] >> 6) & 0x03) == CDC_HEC_STATE_ACTIVE) // HEC is active
                            {
                                firstPhysAddr = pCdc->aHec[pCdc->currentHecIdx].firstDevPhysAddr;
                                lastPhysAddr  = pCdc->aHec[pCdc->currentHecIdx].lastDevPhysAddr;
                                if (CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, initiatorPhysAddr))
                                {
                                    pCdc->verifiedDevNum[0] = 1;                                  
                                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Adjacent device PA = %04x is HEC-active.\n", initiatorPhysAddr);
                                }
                            }
                        }
                    }
                    else if (pCdc->cdcTaskState == CDC_TASK_HPD_CAPAB_STATE_RESPONSE_WAIT)
                    {
                        // Make sure the response comes from an adjacent HDMI source
                        // and the device doesn't report an error status
                        if (CdcIsLinkedDirectly(pCdc->physAddr, initiatorPhysAddr, pCdc->currentPortIdx) &&
                            ((pCdcMessage->args[5] & 0x03) == CDC_ERRCODE_NO_ERROR))
                        {           
                            DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: PA = %04x confirms HPD-over-CDC capability\n", initiatorPhysAddr);
                            pCdc->cdcTaskState = CDC_TASK_HPD_CAPAB_STATE_REPORT_RESULTS; // break waiting
                        }
                    }
                    break;
                    
                case CDCOP_HEC_SET_STATE_ADJACENT:
                    
                    if (pCdcMessage->argCount != 6)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: ERROR in SetStateAdjacent message: wrong length!\n");
                        break;
                    }

                    lastPhysAddr  = (pCdcMessage->args[3] << 8) + pCdcMessage->args[4]; // shall be equal to PA of this device ???
                    if (lastPhysAddr == pCdc->physAddr)
                    {
                        uint8_t     adjacentFlg;

                        // Make sure the request comes from an adjacent device, either HDMI Sink or Source,
                        // and receiving (this) device has capable input or output port that connects it to the initiator
                        adjacentFlg = 0; // adjacency flag
                        if (CdcIsCapableToAdjacent(initiatorPhysAddr, lastPhysAddr, &adjacentFlg) && (pCdc->deviceStatus.hecFuncState != CDC_HEC_FUNC_NOT_SUPPORTED))
                        {
                            // Immediately enable the AHEC without verification of the channel and Alive Notifications
                            // Activation command
                            if (pCdcMessage->args[5] == CDC_HEC_ACTIVE)
                            {
                                DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received SetStateAdjacent::Activate message from PA = %04x\n", initiatorPhysAddr);

                                // Add new AHEC to the Instance Record
                                hecIdx = CdcHecDescriptorAdd(false);
                                if (hecIdx == 0xFF)
                                {
                                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Can't add new HEC descriptor.\n");
                                    pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_OTHER_ERROR; // memory limitation error
                                }
                                else
                                {
                                    pCdc->aHec[hecIdx].activatorPhysAddr = initiatorPhysAddr;
                                    pCdc->aHec[hecIdx].firstDevPhysAddr = initiatorPhysAddr;
                                    pCdc->aHec[hecIdx].lastDevPhysAddr = lastPhysAddr;
                                    pCdc->aHec[hecIdx].status.channelState = CDC_AHEC; // Passive device registers AHECs only
                                    pCdc->aHec[hecIdx].status.isAdjacent = true;

                                    // Activate eHDMI function
                                    if (CdcEhdmiHwEnable(true))
                                    {
                                        pCdc->deviceStatus.hecFuncState = CDC_HEC_STATE_ACTIVE;
                                        CdcIoPortsStatusUpdate();// activate involved ports
                                    }
                                }

                            }
                            else // Deactivation command
                            if (pCdcMessage->args[5] == CDC_HEC_INACTIVE)
                            {
                                DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received SetStateAdjacent::Deactivate message from PA = %04x\n", initiatorPhysAddr);

                                // Remove the AHEC from the Instance Record
                                hecIdx = CdcHecIndexGet(initiatorPhysAddr, initiatorPhysAddr, lastPhysAddr);
                                if (hecIdx == 0xFF)
                                {
                                    DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Can't find existing HEC descriptor to delete.\n");
                                }
                                else
                                {
                                    CdcHecDescriptorRemove(hecIdx);
                                    CdcHecDisableCheck(); // disable HEC hardware if no active HECs

                                    // Update I/O ports status
                                    // Deactivate ports that no longer participate in active HECs
                                    CdcIoPortsStatusUpdate();
                                }

                            }
                            
                            CdcHecReportStateSend(initiatorPhysAddr, CDC_REPORT_STATE_SHORT_MODE);
                        }
                        else if (adjacentFlg == 1) // if two devices are adjacent but not capable for connection
                        {
                            // report an error when required ports are HEC incapable
                            pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_NOT_SUPPORTED_CAPABILITY;
                            CdcHecReportStateSend(initiatorPhysAddr, CDC_REPORT_STATE_SHORT_MODE);
                        }
                    }                  
                    break;
                    
                case CDCOP_HEC_SET_STATE:
                    numberOfHecs = (pCdcMessage->argCount - 6) / 2; // total number of HECs in the SetState command
                    if ((numberOfHecs < 1) || (numberOfHecs > 4))
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, ("[CDC_TASK_BASE_SERVICE]: ERROR in SetState message format!\n"));
                        break;
                    }
                    
                    hecCount = 0; // number of HECs where this device participate (=0 by default)
                    firstPhysAddr = (pCdcMessage->args[3] << 8) + pCdcMessage->args[4];
                    lastPhysAddr  = (pCdcMessage->args[5] << 8) + pCdcMessage->args[6];

                    // Activation command
                    if (pCdcMessage->args[7] == CDC_HEC_ACTIVE) 
                    {
                        bool_t isAtLeastOneDeviceBelongsToHec = false;
                        
                        // It can be from 1 to 4 HECs that need to be activated or omited
                        for (i = 0; i < numberOfHecs; i++) // can be up to 4 terminator's PAs
                        {
                            // reply with ReportState message only if the device belongs to the inquired HEC
                            if (CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, pCdc->physAddr))
                            {
                                isAtLeastOneDeviceBelongsToHec = true;
                                
                                // report no error if all relevant ports are HEC capable
                                if (CdcCapablePortGet(firstPhysAddr, lastPhysAddr, aInputPortRtIdx, &outputPortRtFlg))
                                {
                                        DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received SetState::Activate message from PA = %04x, (HEC %d)\n", initiatorPhysAddr, (int) i);

                                        // Make sure the HEC is new and hasn't been activated already
                                        hecIdx = CdcHecIndexGet(initiatorPhysAddr, firstPhysAddr, lastPhysAddr);
                                        if (hecIdx == 0xFF) // existing HEC not found
                                        {
                                            hecCount++; // count this HEC, if this device is a part of it and capable to handle the HEC
                                        
                                            // Add new AHEC to the Instance Record
                                            hecIdx = CdcHecDescriptorAdd(false); 
                                            if (hecIdx == 0xFF)
                                            {                                    
                                                DEBUG_PRINT(CDC_MSG_DBG, ("[CDC_TASK_BASE_SERVICE]: Can't add new HEC descriptor.\n"));
                                                pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_OTHER_ERROR; // memory limitation error
                                            }
                                            else
                                            {
                                                pCdc->aHec[hecIdx].activatorPhysAddr = initiatorPhysAddr;
                                                pCdc->aHec[hecIdx].firstDevPhysAddr = firstPhysAddr;
                                                pCdc->aHec[hecIdx].lastDevPhysAddr = lastPhysAddr;
                                                pCdc->aHec[hecIdx].status.channelState = CDC_AHEC; // Passive device registers AHECs only
                                                
                                                CdcAliveDeviceListAdd(hecIdx); // add HEC's activator to Alive List     
                                                CdcIoPortsActivate(aInputPortRtIdx, outputPortRtFlg); // activate involved ports                                       
                                            }
                                        }
                                        else if ((pCdc->aHec[hecIdx].status.channelState == CDC_AHEC) && (pCdc->deviceStatus.cdcErrorState == CDC_ERRCODE_NO_ERROR))
                                        {
                                            hecCount++; // count this compatible AHEC without deactivation
                                        }
                                        else
                                        {
                                            DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: HEC already exists and it is incompatible state. The HEC must be removed first.\n");
                                            pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_INCOMPATIBLE_STATE;
                                        }
                                                                    
                                }
                                else
                                {
                                    // report an error when required ports are HEC incapable
                                    pCdc->deviceStatus.cdcErrorState = CDC_ERRCODE_NOT_SUPPORTED_CAPABILITY;
                                }
                            }
                            
                            lastPhysAddr  = (pCdcMessage->args[8 + 2 * i] << 8) + pCdcMessage->args[9 + 2 * i];
                        }
                        
                    
                        // Report activation state
                        // Only one report is sent for any number of HECs in one SetState command
                        if (isAtLeastOneDeviceBelongsToHec) // don't report, if not in declared HECs
                        {
                            // Activate eHDMI function
                            if (hecCount > 0)
                            {
                                if (pCdc->deviceStatus.hecFuncState < CDC_HEC_STATE_ACTIVE)
                                {
                                    if (CdcEhdmiHwEnable(true))
                                    {   
                                        pCdc->deviceStatus.hecFuncState = CDC_HEC_STATE_ACTIVE; 
                                        CdcAliveTimerSet(10, 50); // set random delay for NotifyAlive messages (10 - 50 sec)                                                                    
                                    }
                                }
                            }

                            if (numberOfHecs == 1)
                            {   
                                uint8_t hecFuncState = pCdc->deviceStatus.hecFuncState;
                                // simplified forms of report is used for single HEC activation command
                                if (pCdc->deviceStatus.cdcErrorState == CDC_ERRCODE_NOT_SUPPORTED_CAPABILITY)
                                {   // temporary change the device status in order to report "HEC not supported"
                                    // at one or two of the ports
                                    pCdc->deviceStatus.hecFuncState = CDC_HEC_NOT_SUPPORTED;
                                }
                                CdcHecReportStateSend(initiatorPhysAddr, CDC_REPORT_STATE_SHORT_MODE);
                                pCdc->deviceStatus.hecFuncState = hecFuncState; // restore the device status                               
                            }
                            else
                            {   // HEC activation fields must be sent for multi-HEC activation command
                                CdcHecReportStateSend(initiatorPhysAddr, CDC_REPORT_STATE_MULTIVHEC_MODE);                                     
                            }
                        }                        
                    
                    }

                    // Deactivation command
                    if (CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, pCdc->physAddr) &&
                        (pCdcMessage->args[7] == CDC_HEC_INACTIVE) && (numberOfHecs == 1)) 
                    {
                        DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received SetState::Deactivate message from PA = %04x\n", initiatorPhysAddr);
                    
                        // Remove the AHEC from the Instance Record
                        hecIdx = CdcHecIndexGet(initiatorPhysAddr, firstPhysAddr, lastPhysAddr);
                        if (hecIdx == 0xFF)
                        {                                    
                            DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: Can't find existing HEC descriptor to delete.\n");
                        }
                        else
                        {
                            CdcAliveDeviceListRemove(hecIdx); // remove devices from Alive List (if possible)                                    
                            CdcHecDescriptorRemove(hecIdx);
                            CdcHecDisableCheck(); // disable HEC hardware if no active HECs
                    
                            // Update I/O ports status
                            // Deactivate ports that no longer participate in active HECs
                            CdcIoPortsStatusUpdate();
                        }
                        // No report is needed for deactivation command                               
                    }
                    
                    break;
                    
                case CDCOP_HEC_REQUEST_DEACTIVATION:

                    if (pCdcMessage->argCount != 9)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: ERROR in DeactivationRequest message: wrong length!\n");
                        break;
                    }
                    
                    // Only Activator of deactivated AHEC must respond to the message
                    targetPhysAddr = (pCdcMessage->args[3] << 8) + pCdcMessage->args[4]; // activator's physical address
                    firstPhysAddr = (pCdcMessage->args[5] << 8) + pCdcMessage->args[6];
                    lastPhysAddr  = (pCdcMessage->args[7] << 8) + pCdcMessage->args[8]; 
                    
                    // reply with SetState message only if the device is activator of the registered AHEC
                    // and initiating device belongs to the HEC it requests to deactivate 
                    hecIdx = CdcHecIndexGet(targetPhysAddr, firstPhysAddr, lastPhysAddr);
                    if ((hecIdx != 0xFF) && (pCdc->aHec[hecIdx].status.channelState == CDC_AHEC) &&
                         CdcIsDeviceBelongsToHec(firstPhysAddr, lastPhysAddr, initiatorPhysAddr))
                    {
                        DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received RequestDeactivation message from PA = %04x\n", initiatorPhysAddr);
                        SiiCdcDeviceDisconnect(false, targetPhysAddr, firstPhysAddr, lastPhysAddr); 
                    }
                    break;
                    
                case CDCOP_HEC_NOTIFY_ALIVE:
                    // find out if the initiator is in the list of monitored devices
                    // reset Alive Time counter to 140 sec for the initiator
                    if (CdcAliveDeviceListTimeReset(initiatorPhysAddr) == true) // device has been found in AliveDevice List
                    {                            
                        DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received NotifyAlive message from PA = %04x\n", initiatorPhysAddr);
                    }
                    break;
                    
                case CDCOP_HEC_DISCOVER:
                    // reply with ReportState message
                    if (pCdc->deviceStatus.hecFuncState != CDC_HEC_FUNC_NOT_SUPPORTED)
                    {
                        DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received Discover message from PA = %04x\n", initiatorPhysAddr);
                        CdcHecReportStateSend(initiatorPhysAddr, CDC_REPORT_STATE_DISCOVER_MODE);
                    }
                    break;
                    
                case CDCOP_HPD_SET_STATE:

                    if (pCdcMessage->argCount != 4)
                    {
                        DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: ERROR in HpdSetState message: wrong length!\n");
                        break;
                    }
                    
                    // Source replies with HPD ReportState message if the message came from the port of adjacent Sink
                    // that is connected to this Source
                    inputPortIndex = (pCdcMessage->args[3] >> 4) & 0x0F; // index of an input port that sent the HPD SetState command
                    if (CdcIsLinkedDirectly(initiatorPhysAddr, pCdc->physAddr, inputPortIndex))
                    {
                        DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received HPD SetState message from PA = %04x\n", initiatorPhysAddr);
                        // set HPD state of the output port
                        pCdc->ports.output.hpdState = pCdcMessage->args[3] & 0x0F;
                        CdcHpdReportStateSend();                            
                        cdcFeedbackMsg = CDC_FB_MSG_HPD_STATE_CHANGED;
                    }
                    break;
                    
                case CDCOP_HPD_REPORT_STATE:
                    // Make sure the HPD ReportState comes from adjacent Source over currently selected input port
                    // during HPD Signalling Task execution on the waiting stage
                    if ((pCdc->cdcTaskFlags & CDC_TASK_HPD_SIGNAL) && (pCdc->cdcTaskState == CDC_TASK_HPD_SIGNAL_STATE_RESPONSE_WAIT))
                    {
                        if (CdcIsLinkedDirectly(pCdc->physAddr, initiatorPhysAddr, pCdc->currentPortIdx))
                        {           
                            DEBUG_PRINT(MSG_STAT, "[CDC_TASK_BASE_SERVICE]: received HPD ReportState message from PA = %04x\n", initiatorPhysAddr);
                            pCdc->cdcTaskState = CDC_TASK_HPD_SIGNAL_STATE_REPORT_RESULTS; // break waiting
                        }
                    }
                    break;
            }
        }
        else
        {
//            DEBUG_PRINT(CDC_MSG_DBG, "[CDC_TASK_BASE_SERVICE]: non-CDC CEC opcode or non-broadcast destination\n");
            cdcFeedbackMsg = CDC_FB_MSG_ERR_NONCDC_CMD;
        }

    }
    else
    {
        if ((pCdc->numberOfAliveDev > 0) && (pCdc->deviceStatus.hecFuncState >= CDC_HEC_STATE_ACTIVE))
        {                
            // if it's time to send alive notification, do it also
            // if participating in AHECs (either as activator or passive device),
            // and if Alive Device list has at least one entry     
            if (CdcIsTimeCounterExpired(&pCdc->aliveNtfDelayCount)) // if timer expired
            {
                CdcHecNotifyAliveSend();
                CdcAliveTimerSet(60, 62); // send consecutive notification in 60-65 seconds
                DEBUG_PRINT(MSG_STAT, ("[CDC_TASK_BASE_SERVICE]: sent NotifyAlive message.\n"));
            }
        }
        
    }

    return cdcFeedbackMsg;        
}


//-------------------------------------------------------------------------------------------------
//! @brief      Task dispatching function
//!
//!             Shall be called periodically.
//!             Using of SiiCdcNextInvocationTimeGet() helps to figure out
//!             exact recall time that can vary depending on current task status.
//!             Alternatively, simple periodic polling would work as well;
//!             in the case of polling, CDC_MIN_RECALL_TIME_MS is a suggested
//!             recall time.
//!
//! @param[in]  sysTimerCountMs - current value of the system time counter (in milliseconds)
//! @param[in]  pCdcMessage     - pointer to a pending CDC message or 0,
//!                               if there is no message awaiting
//!
//! @return     Feedback message to the caller (Task status or error reports)
//-------------------------------------------------------------------------------------------------

uint8_t SiiCdcTaskProcess(uint16_t sysTimerCountMs, SiiCpiData_t *pCdcMessage)
{
    uint8_t  cdcFeedbackMsg = CDC_FB_MSG_NONE;
    uint16_t cdcTaskFlagsMasked;
    uint16_t devIdx;    


    // Keep all tasks frozen if the Base task is off
    if (pCdc->cdcTaskFlags & CDC_TASK_BASE_SERVICE)
    {

        // Update time reading variables
        pCdc->sysTimerCountPrvMs = pCdc->sysTimerCountCurMs; // memorise previous timer reading
        pCdc->sysTimerCountCurMs = sysTimerCountMs; // set current timer reading  
        
        if (((pCdc->cdcTaskFlags & CDC_TASK_MASK_ALL_BUT_BASE) == 0) && (pCdcMessage == 0))
        {
            // Initialize the recall time counter by maximum possible value
            // in order to ensure rare recalls of the task handler when tasks are inactive
            CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MAX_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);
        }
        else
        {   // Ensure fast recall if there are active tasks or pending messages as they could 
            // require immediate subsequent action (e.g., launching of active tasks) 
            // Actual recall time will be corrected by task internal timer updates, if any
            CdcTimeCounterSet(CDC_TIME_MS2TCK(CDC_MIN_RECALL_TIME_MS), &pCdc->cdcRecallTimeCount);        
        }

        if ((pCdc->numberOfAliveDev > 0) && (pCdc->deviceStatus.hecFuncState >= CDC_HEC_STATE_ACTIVE))
        {                            
            CdcTimeCounterUpdate(&pCdc->aliveNtfDelayCount);
        }

        // Update AliveDevice List times and react on timeouts
        if ((devIdx = CdcAliveDeviceListTimeUpdate()) != 0xFFFF) // if timeout detected for some device
        {
            // Disconnect all HECs that include the failed device
            // Make sure other tasks are inactive, leave the timout pending otherwise
            CdcLostDeviceDisconnect(pCdc->aAliveDevice[devIdx].physAddr);
        }

        // Update the Discovery repetition timer
        CdcTimeCounterUpdate(&pCdc->cdcDiscoveryTimeCount);
        
        // Do one task at a time. Tasks shall never be running concurrently except the Base Service that
        // can run in background of another task, not corrupting its context.
        // Base Service has highest priority if a CDC message is pending and lowest priority otherwise
        
        cdcTaskFlagsMasked = (pCdc->cdcTaskFlags & pCdc->cdcTaskMask);

        if (pCdcMessage != 0) // CDC message is pending
        {   
            cdcFeedbackMsg = CdcTaskBase(pCdcMessage);      
        }
        else
        {
            if (cdcTaskFlagsMasked & CDC_TASK_HOSTS_SEARCH)
            {
                cdcFeedbackMsg = CdcTaskDiscover();
            }
            else if (cdcTaskFlagsMasked & CDC_TASK_CONNECT)
            {
                cdcFeedbackMsg = CdcTaskConnect();        
            }
            else if (cdcTaskFlagsMasked & CDC_TASK_DISCONNECT)
            {
                cdcFeedbackMsg = CdcTaskDisconnect();
            }

            else if (cdcTaskFlagsMasked & CDC_TASK_CONNECT_ADJACENT)
            {
                cdcFeedbackMsg = CdcTaskConnectAdjacent();        
            }                
            else if (cdcTaskFlagsMasked & CDC_TASK_DISCONNECT_ALL)
            {
                cdcFeedbackMsg = CdcTaskDisconnectAll();
            }
            else if (cdcTaskFlagsMasked & CDC_TASK_DISCONNECT_LOST)
            {
                cdcFeedbackMsg = CdcTaskDisconnectLost();
            }   
            else if (cdcTaskFlagsMasked & CDC_TASK_HPD_SIGNAL)
            {
#if (IS_RX == ENABLE)
                cdcFeedbackMsg = CdcTaskHpdSignal();
#else
                cdcFeedbackMsg = CDC_FB_MSG_ERROR;
#endif
            }
            else if (cdcTaskFlagsMasked & CDC_TASK_HPD_CAPAB_VERIF)
            {
                cdcFeedbackMsg = CdcTaskHpdCapabilityVerification();
            }
            // Base Task can't be masked and can run in background of the other active task if incoming CDC message is pending
            else
            {
                cdcFeedbackMsg = CdcTaskBase(0);
            }
        }
        
    }
    
    return cdcFeedbackMsg;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns longest possible time delay before the next invocation
//!             of the SiiCdcTaskProcess() function
//!
//!             This function can be used to set a countdown timer and call
//!             the SiiCdcTaskProcess() on the timer's expiration event unless
//!             new CDC message is pending
//!
//! @param[in]  sysTimerCountMs - current value of the system time counter (in milliseconds)
//!
//! @return     time in milliseconds
//-------------------------------------------------------------------------------------------------

uint16_t SiiCdcNextInvocationTimeGet(uint16_t sysTimerCountMs)
{
    uint32_t totalDelayMs;
    uint16_t invDelayMs;

    if (CdcIsTimeCounterExpired(&pCdc->cdcRecallTimeCount))
    {
        totalDelayMs = 0;
    }
    else
    {
        totalDelayMs =  (uint32_t) pCdc->cdcRecallTimeCount.tRov * 0x10000 /* 2^16 */ + 
                        (uint32_t) pCdc->cdcRecallTimeCount.tRes * CDC_TIME_TCK_MS - (uint32_t) sysTimerCountMs;
    }
    
    // Apply limits to recall delay
    if (totalDelayMs < CDC_MIN_RECALL_TIME_MS)
    {
        invDelayMs = CDC_MIN_RECALL_TIME_MS;
    }
    else if (totalDelayMs > CDC_MAX_RECALL_TIME_MS)
    {
        invDelayMs = CDC_MAX_RECALL_TIME_MS;        
    }
    else
    {
        invDelayMs = (uint16_t) totalDelayMs;
    }

    //DEBUG_PRINT(CDC_MSG_DBG, "SiiCdcNextInvocationTimeGet(): -- recall in -- %d ms\n", invDelayMs);
      
    return invDelayMs;   
}


//-------------------------------------------------------------------------------------------------
//! @brief      Launches CDC Active and Passive Services
//!
//!             Active CDC service maintains operation of all activated by this
//!             device HECs. Passive CDC Service maintains responsiveness to
//!             CDC requests and sending Alive messages (when participating in
//!             AHECs)
//!
//! @retval     true, if the task can't be scheduled,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcSrvStart(void)
{
    bool_t errStatus = false;
    
    // check if Passive or Active services can be started
    if (pCdc->deviceStatus.hecFuncState != CDC_HEC_FUNC_NOT_SUPPORTED)
    {
        pCdc->cdcTaskFlags |= CDC_TASK_BASE_SERVICE;        
    }
    else
    {       
        errStatus = true;        
        DEBUG_PRINT(CDC_MSG_ALWAYS, "WARNING in SiiCdcSrvStart(): Services can't be started.\n");
    }
    
    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Stops CDC Active and Passive Services
//-------------------------------------------------------------------------------------------------

void SiiCdcSrvStop(void)
{
    pCdc->cdcTaskFlags &= ~CDC_TASK_BASE_SERVICE;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Retrieves status of a task
//!
//! @retval     true if the task is active,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CdcTaskStatusGet(uint16_t cdcTaskId)
{
    return (pCdc->cdcTaskFlags & cdcTaskId);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Discovery of all devices having HEC capability and supports Host
//!             or External network function throughout entire HDMI network
//!
//!             Discovery process shall not be conducted more frequently than
//!             once in 3 minutes. This rule is enforced in this function; it
//!             will return non-zero, if called before 3 minutes since the last call
//!
//! @retval     0        - if the search task is scheduled successfully,
//! @retval     [1-0xFE] - the number of seconds before next permitted Discovery,
//! @retval     0xFF     - other error
//-------------------------------------------------------------------------------------------------

uint8_t SiiCdcSrvHostsFind(void)
{
    uint16_t timeLeftSec = 0;
        
    if (CdcIsTimeCounterExpired(&pCdc->cdcDiscoveryTimeCount))
    {        
        // schedule immediate start of the search
        pCdc->cdcTaskFlags |= CDC_TASK_HOSTS_SEARCH;
    }
    else
    {
        timeLeftSec = CdcTimeCounterLeftSecGet(&pCdc->cdcDiscoveryTimeCount);
        DEBUG_PRINT(CDC_MSG_ALWAYS, "WARNING: Discovery function will be available after %d seconds\n", timeLeftSec);
    }

    if (timeLeftSec < 0xFF)
    {
        return (uint8_t) timeLeftSec;
    }
    else
    {
        return 0xFF; // error code
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Returns physical address of a nearest (HEC of a minimum length)
//!             device from the found list that has external network capability (ENC)
//!
//!             This function can be used in order to find shortest path to
//!             the Internet access point
//!
//! @return     Physical Address of a nearest external network capable device,
//! @retval     0xFFFF - if ENC device wasn't found
//-------------------------------------------------------------------------------------------------

uint16_t SiiCdcNearestEncDeviceGet(void)
{
    uint8_t  i;
    uint8_t  iMin = 0;
    uint8_t  hecLen;
    uint8_t  hecLenMin = 0xFF;
    
    for (i = 0; i < pCdc->numberOfFoundHosts; i++)
    {
        if (pCdc->cdcFoundHostsList[i].devStat.encFuncState != CDC_ENC_NOT_SUPPORTED)
        {
            hecLen = CdcHecLengthGet(pCdc->physAddr, pCdc->cdcFoundHostsList[i].physAddr); 
            if (hecLen < hecLenMin)
            {
                hecLenMin = hecLen;
                iMin = i;
            }
        }
    }

    if (hecLenMin != 0xFF)   
    {
        return pCdc->cdcFoundHostsList[iMin].physAddr;
    }
    else
    {
        return 0xFFFF;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Establishes connection to a specified device or group of devices
//!
//!             This function should be used after discovery process, so that the
//!             cdcFoundHostsList[] list isn't empty. It must be used by Activator
//!             of the HEC
//!
//! @param[in]  isConnectEntireList - if true, will connect all devices from the
//!                                   cdcFoundHostsList[] that have selection bit
//!                                   (hostInfo.isSelected) asserted. The lastPhysAddr
//!                                   parameter is ignored in this case.
//!                                   if false, only one device having specified
//!                                   physical address will be connected.
//! @param[in]  firstPhysAddr       - physical address of a first HEC device to be
//!                                   connected to the last device or devices in
//!                                   the cdcFoundHostsList[]. Typically,
//!                                   the first device is an activator of the HEC
//! @param[in]  lastPhysAddr        - physical address of a second device to be
//!                                   connected to the first device
//!
//! @retval     true, if the connect task can't be scheduled,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcDeviceConnect(bool_t isConnectEntireList, uint16_t firstPhysAddr, uint16_t lastPhysAddr)
{
    uint8_t i;
    uint8_t idx;
    bool_t  errStatus = false;

    // check if this device is HEC capable
    if (pCdc->deviceStatus.hecFuncState == CDC_HEC_FUNC_NOT_SUPPORTED)
    {
        DEBUG_PRINT(CDC_MSG_ALWAYS, "ERROR in SiiCdcDeviceConnect(): Device is HEC incapable and can't initiate HEC communications\n");
        errStatus = true;
    }
    else
    {
        // check if the found device list has the device entry(ies)
        if ((isConnectEntireList && pCdc->numberOfFoundHosts > 0) || (!isConnectEntireList))
        {
            // create new HEC descriptor(s) and label the HEC as PHEC 
            if (isConnectEntireList)
            {
                pCdc->connectSettings.isMultHecMode = true;  // use multi-HEC activation mode for more than 1 device
                
                for (i = 0; i < pCdc->numberOfFoundHosts; i++)
                {   
                    // connect only selected devices having unequal PAs
                    if (pCdc->cdcFoundHostsList[i].hostInfo.isSelected && (firstPhysAddr != pCdc->cdcFoundHostsList[i].physAddr))
                    {                        
                        // check if activator itself can be a part of the new HEC, if it belongs to the HEC
                        if (CdcIsCapableForHec(pCdc->physAddr, pCdc->cdcFoundHostsList[i].physAddr, false))
                        {
                            if (CdcHecIndexGet(pCdc->physAddr, firstPhysAddr, pCdc->cdcFoundHostsList[i].physAddr) == 0xFF) // make sure that new HEC is unique
                            {
                                if ((idx = CdcHecDescriptorAdd(true)) != 0xFF)
                                {                    
                                    pCdc->aHec[idx].activatorPhysAddr   = pCdc->physAddr;
                                    pCdc->aHec[idx].firstDevPhysAddr    = firstPhysAddr; 
                                    pCdc->aHec[idx].lastDevPhysAddr     = pCdc->cdcFoundHostsList[i].physAddr;
                                    pCdc->aHec[idx].status.channelState = CDC_PHEC; 
                                }
                                else
                                {
                                    errStatus = true;
                                    break;
                                } 
                            }
                            // if HEC isn't unique, don't rise error flag because connection should be already established
                        }
                        else
                        {
                            DEBUG_PRINT(CDC_MSG_ALWAYS, "WARNING in SiiCdcDeviceConnect(): Device is incapable to be an activator for a HEC\n");
                        }
                    }
                }
            }
            else // one HEC
            {            
                pCdc->connectSettings.isMultHecMode = false;  // use single-HEC activation mode for 1 device case
                
                // check if activator itself can be a part of the new HEC, if it belongs to the HEC
                if (CdcIsCapableForHec(firstPhysAddr, lastPhysAddr, false))
                {
                    if (CdcHecIndexGet(pCdc->physAddr, firstPhysAddr, lastPhysAddr) == 0xFF) // make sure that new HEC is unique
                    {
                        if ((idx = CdcHecDescriptorAdd(true)) != 0xFF)
                        {                
                            pCdc->aHec[idx].activatorPhysAddr   = pCdc->physAddr;
                            pCdc->aHec[idx].firstDevPhysAddr    = firstPhysAddr;
                            pCdc->aHec[idx].lastDevPhysAddr     = lastPhysAddr;
                            pCdc->aHec[idx].status.channelState = CDC_PHEC; 
                        }
                        else
                        {
                            errStatus = true;
                        }
                    }
                }
                else
                {
                    DEBUG_PRINT(CDC_MSG_ALWAYS, "WARNING in SiiCdcDeviceConnect(): Device is incapable to be an activator for a HEC\n");
                }
                
            }

            // schedule immediate start of connection task
            if (errStatus == false) 
            {
                pCdc->cdcTaskFlags |= CDC_TASK_CONNECT;
            }
        }
        else
        {
            errStatus = true; // can't run the task
        }
    }
    
    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Establishes connection to a specified device using "HEC control
//!             to Adjacent Device" method.
//!
//! @param[in]  adjacentPhysAddr  - physical address of an adjacent device
//!
//! @retval     true, if the connect task can't be scheduled,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcDeviceAdjacentConnect(uint16_t adjacentPhysAddr)
{
    uint8_t idx;    
    bool_t  errStatus = false;
    
    // check if this device is HEC capable
    if (pCdc->deviceStatus.hecFuncState == CDC_HEC_FUNC_NOT_SUPPORTED)
    {
        DEBUG_PRINT(CDC_MSG_ALWAYS, "ERROR in SiiCdcDeviceAdjacentConnect(): Device is HEC incapable and can't initiate HEC communications\n");
        errStatus = true;
    }
    else
    {
        // check if activator itself can be a part of the new HEC (strict check)
        if (CdcIsCapableForHec(pCdc->physAddr, adjacentPhysAddr, true))
        {
            if (CdcHecIndexGet(pCdc->physAddr, pCdc->physAddr, adjacentPhysAddr) == 0xFF) // make sure that new HEC is unique
            {
                if ((idx = CdcHecDescriptorAdd(true)) != 0xFF)
                {                
                    pCdc->aHec[idx].activatorPhysAddr   = pCdc->physAddr;
                    pCdc->aHec[idx].firstDevPhysAddr    = pCdc->physAddr;
                    pCdc->aHec[idx].lastDevPhysAddr     = adjacentPhysAddr;
                    pCdc->aHec[idx].status.channelState = CDC_PHEC; 
                    pCdc->aHec[idx].status.isAdjacent   = true;

                    pCdc->currentHecIdx = idx;
                    pCdc->cdcTaskFlags |= CDC_TASK_CONNECT_ADJACENT; // schedule immediate start of connection task                   
                }
                else
                {
                    errStatus = true;
                }
            }
            else
            {
                DEBUG_PRINT(CDC_MSG_ALWAYS, ("WARNING in SiiCdcDeviceAdjacentConnect(): HEC already exist.\n"));
                
            }
        }
        else
        {
             DEBUG_PRINT(CDC_MSG_ALWAYS, "WARNING in SiiCdcDeviceAdjacentConnect(): Device is incapable to be an activator for a HEC\n");
        }
     
     }
    
    
    return errStatus;    
}


//-------------------------------------------------------------------------------------------------
//! @brief      Terminates connection to a specified device or all connected devices
//!
//!             This function can be used by Activator of the HEC
//!
//! @param[in]  isDisconnectAll    - if true, will close all active HECs. The second & third
//!                                  parameters are ignored in this case.
//!                                  If false, only one device having specified
//!                                  physical address will be disconnected.
//! @param[in]  actPhysAddr        - physical address of HEC's activator
//! @param[in]  firstPhysAddr      - physical address of a first HEC device
//! @param[in]  lastPhysAddr       - physical address of a second device in HEC
//!
//! @retval     true, if the disconnect task can't be scheduled,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcDeviceDisconnect(bool_t isDisconnectAll, uint16_t actPhysAddr, uint16_t firstPhysAddr, uint16_t lastPhysAddr)
{
    bool_t  errStatus = false;

    if (isDisconnectAll)
    {       
        // schedule immediate start of disconnection task 
        pCdc->cdcTaskFlags |= CDC_TASK_DISCONNECT_ALL;
    }
    else // disconnect only one device 
    {
        pCdc->currentHecIdx = CdcHecIndexGet(actPhysAddr, firstPhysAddr, lastPhysAddr);

        if (pCdc->currentHecIdx < 0xFF)
        {
            if (pCdc->aHec[pCdc->currentHecIdx].status.channelState > CDC_PHEC)
            {
                // schedule immediate start of disconnection task 
                pCdc->cdcTaskFlags |= CDC_TASK_DISCONNECT;
            }
            else
            {
                errStatus = true; // HEC isn't active            
            }
        }
        else
        {
            errStatus = true; // the AHEC wasn't found 
        }
    }

    return errStatus;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Terminates connection to a specified adjacent device
//!
//!             This function should be used by Activator of the HEC, however,
//!             passive device can reset it's connection to the adjacent
//!             device (activator) when it has been physically disconnected
//!
//! @param[in]  adjacentPhysAddr - physical address of an adjacent device
//!
//! @retval     true, if the disconnect task can't be scheduled,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcDeviceAdjacentDisconnect(uint16_t adjacentPhysAddr)
{
    bool_t  errStatus = false;

    pCdc->currentHecIdx = CdcHecIndexGet(pCdc->physAddr, pCdc->physAddr, adjacentPhysAddr);

    if (pCdc->currentHecIdx < 0xFF)
    {
        if ((pCdc->aHec[pCdc->currentHecIdx].status.isAdjacent == true) &&
            (pCdc->aHec[pCdc->currentHecIdx].status.channelState > CDC_PHEC))
        {
            // disconnection of adjacent device doesn't require state reporting,
            // therefore, non-task approach is relevant. Just send disconnection CDC command
            CdcHecSetStateAdjacentSend(CDC_HEC_INACTIVE, pCdc->currentHecIdx);
            
            CdcHecDescriptorRemove(pCdc->currentHecIdx); // release resources for HEC
            CdcHecDisableCheck(); // disable HEC hardware if no active HECs
            CdcIoPortsStatusUpdate(); // deactivate ports involved in the deactivated HEC            
        }
        else
        {
            errStatus = true; // HEC isn't active or activated by control to adjacent device           
        }
    }
    else
    {   // check if Adjacent Device is the activator of a HEC 
        pCdc->currentHecIdx = CdcHecIndexGet(adjacentPhysAddr, pCdc->physAddr, adjacentPhysAddr);
        
        if (pCdc->currentHecIdx < 0xFF)
        {
            if (pCdc->aHec[pCdc->currentHecIdx].status.isAdjacent == true)
            {
                // force the HEC disconnection
                CdcHecDescriptorRemove(pCdc->currentHecIdx); // release resources for HEC
                CdcHecDisableCheck(); // disable HEC hardware if no active HECs
                CdcIoPortsStatusUpdate(); // deactivate ports involved in the deactivated HEC 
            }
            else
            {
                errStatus = true; // AHEC isn't activated by control to adjacent device 
            }
        }
        else
        {
            errStatus = true; // the AHEC wasn't found 
        }
    }

    return errStatus;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Deactivates all AHECs that include a device having
//!             given physical address.
//!
//! @param[in]  physAddr - physical address of the failed device
//-------------------------------------------------------------------------------------------------

static void CdcLostDeviceDisconnect(uint16_t physAddr)
{
    if ((pCdc->cdcTaskFlags & CDC_TASK_MASK_ALL_BUT_BASE) == 0)
    {
            // disconnect only one device (other timeouts will be pending)
            pCdc->aliveDevPhysAddr = physAddr;
            // schedule immediate start of disconnection task
            pCdc->cdcTaskFlags |= CDC_TASK_DISCONNECT_LOST;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Determines if the specified port is an active CDC port
//!
//! @param[in]  isOutput  - true, if output port is addressed
//! @param[in]  portIndex - input port index (zero-based)
//!
//! @retval     true if HEC channel is attached to the specified HDMI port and is active,
//! @retval     false if HDMI port has no active HEC channels
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcPortIsActive(const bool_t isOutput, const uint8_t portIndex)
{
    bool_t    isActive = false;

    if (( isOutput == true) && (pCdc->ports.numberOfOutputs != 0))
    {
        isActive = (pCdc->ports.output.hecSupport == true ) &&
                   (pCdc->ports.output.hecActive == true );
    }
    else if ((isOutput == false) && (pCdc->ports.numberOfInputs > portIndex)) // inputs
    {
        isActive = (pCdc->ports.input[portIndex].hecSupport == true ) &&
                   (pCdc->ports.input[portIndex].hecActive == true );
    }

    return isActive;
}


#if (IS_RX == ENABLE)
//-------------------------------------------------------------------------------------------------
//! @brief      Sets new HPD state to selected port and sends an HPD State
//!             message to HDMI Source device
//!
//!             This function shall be used by an HDMI Sink device
//!
//! @param[in]  inputPortIndex - zero based input port number
//! @param[in]  hpdState       - HPD state of the input port
//!
//! @retval     true, if the HPD signaling task can't be scheduled,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcHpdSetState(uint8_t inputPortIndex, CdcHpdState_t hpdState)
{
    bool_t  errStatus = true;
    
    if (inputPortIndex < pCdc->ports.numberOfInputs)
    {
        if (pCdc->ports.input[inputPortIndex].hecSupport != CDC_HEC_NOT_SUPPORTED) 
        {
            pCdc->currentPortIdx = inputPortIndex;
            pCdc->ports.input[inputPortIndex].hpdState = hpdState;
            
            // schedule immediate start of HPD signaling task
            pCdc->cdcTaskFlags |= CDC_TASK_HPD_SIGNAL;            
            errStatus = false;
        }
    }
    
    return errStatus;    
}
#endif


#if (IS_TX == ENABLE)
//-------------------------------------------------------------------------------------------------
//! @brief      Gets current HPD state of a Source device
//!
//!             This function shall be used by an HDMI Source device
//!
//! @return     HPD state code or CDC_HPD_UNKNOWN_STATE, if output is in
//!             error state or doesn't exist (root device)
//-------------------------------------------------------------------------------------------------

CdcHpdState_t SiiCdcHpdStateGet(void)
{
    CdcHpdState_t hpdState = CDC_HPD_UNKNOWN_STATE;
    
    if ((pCdc->ports.numberOfOutputs == 1) && 
        ( (pCdc->ports.output.hpdErrorState == CDC_HPD_ERRCODE_NO_ERROR) || 
          (pCdc->ports.output.hpdErrorState == CDC_HPD_ERRCODE_NO_ERROR_NO_VIDEO)  ))
    {
        hpdState = pCdc->ports.output.hpdState;
    }

    return hpdState;
}
#endif


#if (IS_RX == ENABLE)
//-------------------------------------------------------------------------------------------------
//! @brief      Launches verification task that checks if selected adjacent
//!             device supports HPD-over-CDC messaging
//!
//!             This function shall be used by an HDMI Sink device
//!
//! @param[in]  inputPortIndex - zero based input port number
//!
//! @retval     true, if the HPD capability verification task can't be scheduled,
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t SiiCdcHpdCapabilityInquire(uint8_t inputPortIndex)
{
    bool_t  errStatus = true;
    
    if (inputPortIndex < pCdc->ports.numberOfInputs)
    {
        if (pCdc->ports.input[inputPortIndex].hecSupport != CDC_HEC_NOT_SUPPORTED) 
        {
            pCdc->currentPortIdx = inputPortIndex;
            
            // schedule immediate start of HPD capability verification task 
            pCdc->cdcTaskFlags |= CDC_TASK_HPD_CAPAB_VERIF;            
            errStatus = false;
        }
    }
    
    return errStatus;    
}

#endif
