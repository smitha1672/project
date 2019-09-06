//***************************************************************************
//! @file   si_cec_switch_api.h
//! @brief  Top level CEC / Switch
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/


#ifndef __SI_CEC_SWITCH_API_H__
#define __SI_CEC_SWITCH_API_H__

#include "si_common.h"
#include "si_cec_timer.h"
#include "si_cec_switch_enums.h"

//-------------------------------------------------------------------------------------------------
// Constant definitions
//-------------------------------------------------------------------------------------------------

//! @defgroup SWITCH_TASKS     Switch Tasks
//! @{
#define CEC_SWITCH_TASK_BASE_SERVICE        0x0001  //!< responds to Switch commands and requests
#define CEC_SWITCH_TASK_SEND_ROUTE_INFO     0x0002  //!< Performs route information broadcasting
//
//! @}

//! @defgroup SWITCH_FB_MSG    Switch Feedback Messages
//! @{
#define CEC_SWITCH_FB_MSG_NONE              0x0000  //!< default value: no messages
#define CEC_SWITCH_FB_MSG_SEND_ROUTE_DONE   0x0001  //!< Send routing info task has finished
#define CEC_SWITCH_FB_MSG_STATUS_CHANGED    0x00A0  //!< Switch status has changed
#define CEC_SWITCH_FB_MSG_ERROR             0x00E0  //!< General error occurred
#define CEC_SWITCH_FB_MSG_ERR_NONSWITCH_CMD 0x00E1  //!< Non CEC/Switch command received

//
//! @}

//! @defgroup SWITCH_TASK_MASK     Switch Task mask
//! @{
#define CEC_SWITCH_TASK_MASK_ALL_UNMASKED   0xFFFF  //!< default value of the Task mask allowing any tasks to start
#define CEC_SWITCH_TASK_MASK_ALL_BUT_BASE   0xFFFE  //!< mask for all tasks except the Base service task
//
//! @}


#define CEC_SWITCH_ROUTE_INFO_DELAY_MS      2000    //!< Minimum delay before sending routing information

//-------------------------------------------------------------------------------------------------
// Data type definitions
//-------------------------------------------------------------------------------------------------

//! CEC/SAC Instance Record

typedef struct
{

    // -------------------- Private area -----------------------
    CecTimeCounter_t                recallTimeCount;    //!< Estimated time point of the task handler next recall

    // Task states
    uint16_t                        taskFlags;          //!< Bit fields indicating active tasks
    uint16_t                        taskMask;           //!< Bitmask selecting currently active task
    CecTimeCounter_t                taskTimeCount;      //!< Time counter for waiting within tasks
    CecSwitchTaskState_t            taskState;          //!< task current state (combined)

    uint16_t                        sysTimerCountCurMs; //!< current reading of the system timer
    uint16_t                        sysTimerCountPrvMs; //!< previous reading of the system timer

    uint8_t                         activeInputPortIndex;  //!< currently active (selected) input port

} CecSwitchInstanceRecord_t;


//-------------------------------------------------------------------------------------------------
// Top level function prototypes
//-------------------------------------------------------------------------------------------------

void     SiiCecSwitchInstanceSet(uint16_t deviceIndex);
CecSwitchInstanceRecord_t *SiiCecSwitchDeviceInstanceGet(void);
void     SiiCecSwitchConfig();
    
void     SiiCecSwitchTaskInit(void);
bool_t   SiiCecSwitchSrvStart(void);
void     SiiCecSwitchSrvStop(void);

bool_t  SiiCecSwitchTaskProcess(SiiCpiData_t *pCecMessage);
uint16_t SiiCecSwitchNextInvocationTimeGet(uint16_t sysTimerCountMs);

bool_t   SiiCecSwitchSendRouteInfo(void);

void     SiiCecRoutingInformationSend(void);
void     SiiCecRoutingChangeSend(uint8_t newInputPortIndex);

//-------------------------------------------------------------------------------------------------
// Callback function prototypes
//-------------------------------------------------------------------------------------------------

uint8_t  SiiCbCecActiveInputGet(void);
void     SiiCbCecStandby(bool_t standby);
bool_t   SiiCbCecSwitchRxHpdGet(void);
bool_t   SiiCbCecSwitchMessageHandler(uint8_t fbMsg);


#endif // __SI_CEC_SWITCH_API_H__

