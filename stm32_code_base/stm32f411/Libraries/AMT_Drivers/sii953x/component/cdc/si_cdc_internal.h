//***************************************************************************
//!file     si_cdc_internal.h
//!brief    Silicon Image CDC Component internal header.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CDC_INTERNAL_H__
#define __SI_CDC_INTERNAL_H__

#include "si_device_config.h"
#include "si_drv_cpi.h"
#include "si_cec_component.h"
#include "si_cdc_enums.h"
#include "si_cdc_heac.h"

//------------------------------------------------------------------------------
// CDC Component Manifest Constants
//------------------------------------------------------------------------------

#define NUMBER_OF_DEVICES               1  //!< Can be more than 1 for HDMI repeater with multiple HDMI outputs

#define CDC_MSG_DBG                    (MSG_DBG | DBGF_TS | DBGF_CN), DBG_CDC
#define CDC_MSG_ALWAYS                 (MSG_ALWAYS | DBGF_TS | DBGF_CN), DBG_CDC


//! @defgroup CDC_TASKS     CDC Tasks
//! @{
#define CDC_TASK_BASE_SERVICE           0x0001  //!< sends/checks Alive messages, responds to commands and requests
#define CDC_TASK_HOSTS_SEARCH           0x0002  //!< searches for all HEC capable devices
#define CDC_TASK_CONNECT                0x0004  //!< establishes a connection to HEC capable devices
#define CDC_TASK_CONNECT_ADJACENT       0x0008  //!< establishes a connection to an adjacent HEC capable devices
#define CDC_TASK_DISCONNECT             0x0010  //!< dismisses connections to HEC capable devices
#define CDC_TASK_DISCONNECT_ALL         0x0020  //!< dismisses all open HECs
#define CDC_TASK_DISCONNECT_LOST        0x0040  //!< dismisses all HECs that include a lost device
#define CDC_TASK_HPD_SIGNAL             0x0080  //!< delivers HPD status from Sink to Source device
#define CDC_TASK_HPD_CAPAB_VERIF        0x0100  //!< verifies HPD-over-CDC capability
//
//! @}

//! @defgroup CDC_TASK_CON  CDC Tasks related time constants
//! @{
#define CDC_MAX_RESPONSE_TIME_MS          1500  //!< milliseconds to wait before ending of each response gathering (required maximum = 1s, exception may apply)
#define CDC_MAX_MONITORING_TIME_SEC          5  //!< seconds to timeout event while collecting all responses (total maximum waiting time)
#define CDC_MIN_DISCOVERY_REP_TIME_SEC     180  //!< seconds (3 minutes) before repetitive discovery command can be issued
#define CDC_MSG_OUT_TIME_MS                300  //!< milliseconds to wait until outgoing message presumably is sent
#define CDC_MIN_RECALL_TIME_MS             100  //!< shortest CDC task handler recall time in milliseconds
#define CDC_MAX_RECALL_TIME_MS           20000  //!< longest CDC task handler recall time in milliseconds
//
//! @}

//! @defgroup CDC_TASK_MSK  CDC Task masks
//! @{
#define CDC_TASK_MASK_ALL_UNMASKED      0xFFFF  //!< default value of the Task mask allowing any tasks to start
#define CDC_TASK_MASK_ALL_BUT_BASE      0xFFFE  //!< mask for all tasks except the Base service task
//
//! @}

//------------------------------------------------------------------------------
//  CDC Component Instance Data
//------------------------------------------------------------------------------
                                        //! CDC Instance Record
typedef struct CdcInstanceData
{
    int                 structVersion;
    int                 lastResultCode; //!< Contains the result of the last API function called
    uint16_t            statusFlags;


    // -------------------- Public area -----------------------
    uint16_t            physAddr;       //!< current Physical Address of the device
    CdcIoConfig_t       ports;          //!< IO configuration and capability
    CdcHecDescriptor_t  aHec[CDC_NUMBER_OF_HECS_MAX]; //!< HEC records
    uint8_t             numberOfHecs;   //!< number of HECs in the record
    CdcDeviceStatus_t   deviceStatus;   //!< status & capability of this device

    //! List of Device Physical Addresses found as a result of search tasks
    CdcFoundListEntry_t cdcFoundHostsList[CDC_NUMBER_OF_FOUND_HOSTS_MAX];
    uint8_t             numberOfFoundHosts; //!< number of Host and/or ENC capable devices

    //!< List of devices Alive status of that is monitored
    CdcPhysAddrTime_t   aAliveDevice[CDC_NUMBER_OF_DEVICES_MAX];
    uint8_t             numberOfAliveDev;   //!< number of nonempty records in aAliveDevice[] list


    // -------------------- Private area -----------------------
    // Storage for time since last Alive message from participating
    // devices has been received:
    // Activators monitor all members of their AHECs except themselves.
    // Passive devices monitor Activators alive status.
    // Special PA value 0xFFFF indicates vacant cells
    uint8_t             numberOfAllHosts;     //!< number of all found devices including ENC or Host incapable

    uint16_t            sysTimerCountCurMs;   //!< current reading of the system timer
    uint16_t            sysTimerCountPrvMs;   //!< previous reading of the system timer
    CdcTimeCounter_t    cdcTotalMonTimeCount; //!< Time counter for total monitoring time metering
    CdcTimeCounter_t    cdcDiscoveryTimeCount;//!< Time counter for minimal Discovery repetition time rule enforcement
    CdcTimeCounter_t    cdcRecallTimeCount;   //!< Estimated time point of the task handler next recall

    uint16_t            aliveDevPhysAddr;   //!< PA of a current Device from AliveDevice List the task works with
    CdcTimeCounter_t    aliveNtfDelayCount; //!< Time delay before sending NotifyAlive message

    // CDC Task states
    uint16_t            cdcTaskFlags;     //!< Bit fields indicating active CDC tasks
    uint16_t            cdcTaskMask;      //!< Bitmask selecting currently active CDC task
    CdcTimeCounter_t    cdcTaskTimeCount; //!< Time counter for waiting within tasks
    CdcTaskState_t      cdcTaskState;     //!< task current state (combined)

    // HEC status assignment states
    uint8_t             currentHecIdx;    //!< index of a current HEC the task works with
    uint8_t             verifiedDevNum[4];//!< number of verified devices for HEC status change
    uint8_t             currentPortIdx;   //!< index of a current input port for HPD signaling task
    CdcMultHecActData_t connectSettings;  //!< Multi-HEC activation mode settings
    uint8_t             delayedHecIdx[4]; //!< Up to 3 delayed and 1 current HEC (for Multi-HEC activation)

}   CdcInstanceData_t;

extern CdcInstanceData_t   cdcInstance[SII_NUM_CDC];
extern CdcInstanceData_t   *pCdc;

#endif // __SI_CDC_INTERNAL_H__
