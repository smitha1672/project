//***************************************************************************
//!file     si_cec_internal.h
//!brief    Silicon Image CEC Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CEC_INTERNAL_H__
#define __SI_CEC_INTERNAL_H__
#include "si_common.h"
#include "si_cec_component.h"
//#include "si_device_config.h"
#include "si_drv_cpi.h"
#include "si_cec_enums.h"
#include "si_cec_timer.h"
#include "si_osal_timer.h"

#if INC_CEC_SWITCH
#   include "si_cec_switch_api.h"
#endif
#if INC_CEC_SAC
#   include "si_sac_main.h"
#endif

//-------------------------------------------------------------------------------
// CPI Enums, typedefs, and manifest constants
//-------------------------------------------------------------------------------

#define CEC_MSG_DBG                     (MSG_DBG | DBGF_TS | DBGF_CN), DBG_CEC
#define CEC_MSG_STAT                    (MSG_STAT | DBGF_CN), DBG_CEC
#define CEC_MSG_ALWAYS                  (MSG_ALWAYS | DBGF_CN), DBG_CEC

#define TASK_QUEUE_LENGTH               6   // Number of task queue entries

#define CHIP_MAX_VIRTUAL_CEC_DEVS     2   //with 2 MHL ports

#define ACTIVE_TASK                     pCec->taskQueue[ pCec->taskQueueOut]


// CDC Tasks related time constants
#define CEC_MAX_RESPONSE_TIME_MS         1500   //!< milliseconds to wait before ending of each response gathering (required maximum = 1s, exception may apply)
#define CEC_MIN_RECALL_TIME_MS           100    //!< shortest CEC task handler recall time in milliseconds
#define CEC_MAX_RECALL_TIME_MS           20000  //!< longest CEC task handler recall time in milliseconds
#define CEC_UC_REPETITION_TIME_MS        350    //!< user control repetition time (200-500ms)
#define CEC_PING_ACK_WAITING_TIME_MS     2000   //!< maximum waiting time for acknowledge of the CEC ping

typedef enum _SiiCecQueueStates_t
{
    SiiCecTaskQueueStateIdle     = 0,
    SiiCecTaskQueueStateQueued,
    SiiCecTaskQueueStateRunning,
} SiiCecQueueStates_t;

    /* Task CPI states. */

typedef enum _SiiCpiStates_t
{
    CPI_IDLE            = 0,
    CPI_WAIT_ACK,
    CPI_WAIT_RESPONSE,
    CPI_RESPONSE
} SiiCpiStates_t;

typedef enum _SiiCecTasks_t
{
    SiiCECTASK_IDLE                 = 0,
    SiiCECTASK_CANCEL,
    SiiCECTASK_ENUMERATE,
    SiiCECTASK_SETLA,
    SiiCECTASK_ONETOUCH,
    SiiCECTASK_SENDMSG,

    SiiCECTASK_COUNT
} SiiCecTasks_t;

typedef struct _SiiCecTaskState_t
{
    SiiCecTasks_t       task;           // This CEC task #
    SiiCecQueueStates_t queueState;     // Task running or waiting to be run
    uint8_t             taskState;      // Internal task state
    SiiCpiStates_t      cpiState;       // State of CPI transactions
    uint8_t             destLA;         // Logical address of target device
    uint8_t             taskData1;      // BYTE Data unique to task.
    uint16_t            taskData2;      // WORD Data unique to task.
    uint8_t             *pTaskData3;    // Pointer to BYTE Data array unique to task.
    uint16_t            msgId;          // Helps serialize CPI transactions
    SiiTimer_t          taskTimer;

} SiiCecTaskState_t;

typedef struct _CecDevice_t
{
    uint8_t deviceType;     // 0 - Device is a TV.
                            // 1 - Device is a Recording device
                            // 2 - Device is a reserved device
                            // 3 - Device is a Tuner
                            // 4 - Device is a Playback device
                            // 5 - Device is an Audio System
    uint8_t  cecLA;         // CEC Logical address of the device.
    uint16_t cecPA;         // CEC Physical address of the device.
} CecDevice_t;

typedef struct _CecLogicalDevice_t
{
    int_t   deviceType;     // 0 - Device is a TV.
                            // 1 - Device is a Recording device
                            // 2 - Device is a reserved device
                            // 3 - Device is a Tuner
                            // 4 - Device is a Playback device
                            // 5 - Device is an Audio System
    int_t       port;       // HDMI port index for this device.
    int_t       selected;   // This logical address was selected on this port
    uint16_t    cecPA;      // CEC Physical address of the device.
    bool_t      isVirtual;  // device is a local virtual device
} CecLogicalDevice_t;

typedef struct _SiiCecMsgHandlers_t
{
    bool_t      callAlways;
    bool_t      (*pMsgHandler)( SiiCpiData_t * );
} SiiCecMsgHandlers_t;

//------------------------------------------------------------------------------
//  CEC Component Instance Data
//------------------------------------------------------------------------------

typedef struct _CecInstanceData_t
{
    int         structVersion;
    int         instanceIndex;
    int         lastResultCode;         // Contains the result of the last API function called
    uint16_t    statusFlags;

    SiiCecDeviceTypes_t deviceType;     // type of CEC device

    uint_t      debugDisplayLevel;

    bool_t                      enable;
    uint8_t                     numVirtualDevices;
    SiiCecLogicalAddresses_t    virtualDevLA[CHIP_MAX_VIRTUAL_CEC_DEVS];
    SiiCecLogicalAddresses_t    devLogicalAddr;
    SiiCecLogicalAddresses_t    logicalAddr;
    uint8_t                     portSelect;
    SiiCecPowerstatus_t         powerState;

    uint8_t     paShift;
    uint16_t    paChildMask;
    uint16_t    physicalAddr;

    uint8_t     osdName[14];
    int         osdNameLen;

    uint8_t     lastUserControlPressedSourceLa;     // For User Control Released
    uint8_t     lastUserControlPressedTargetLa;     // For User Control Released

    // RX-only data

    SiiCecPowerstatus_t         sourcePowerStatus;
    SiiCecLogicalAddresses_t    activeSrcLogical;
    uint16_t                    activeSrcPhysical;
    CecLogicalDevice_t          logicalDeviceInfo [16];

    // TX-only data

    bool_t      isActiveSource;

    // Task data

    int_t               taskQueueIn;
    int_t               taskQueueOut;
    SiiCecTasks_t       currentTask;
    SiiCecTaskState_t   taskQueue[TASK_QUEUE_LENGTH];
    bool_t              enumerateComplete;

    // External message handlers

    SiiCecMsgHandlers_t messageHandlers[MAX_CEC_PARSERS];

    // External Task Handlers

    bool_t      (*pTasks[MAX_CEC_TASKS])( SiiCpiStatus_t * );

}	CecInstanceData_t;

extern CecInstanceData_t cecInstance[SII_NUM_CEC];
extern CecInstanceData_t *pCec;


//------------------------------------------------------------------------------
//  Internal data
//------------------------------------------------------------------------------

extern uint8_t l_devTypes [16];

//------------------------------------------------------------------------------
//  Internal functions
//------------------------------------------------------------------------------

bool_t  CecTxMsgHandler( SiiCpiData_t *pMsg );
uint8_t CecTaskOneTouchPlay( SiiCpiStatus_t *pCecStatus );

bool_t  CecRxMsgHandler( SiiCpiData_t *pMsg );

void    CecSendSetStreamPath( uint16_t destPhysAddr );
void    CecSendTextViewOn ( uint8_t destLogicalAddr );
void    CecSendUserControlPressed ( uint8_t destLogicalAddr, SiiCecUiCommand_t rcCode );
void    CecSendUserControlReleased ( uint8_t destLogicalAddr, SiiCecUiCommand_t rcCode );
void    CecPrintLogAddr ( uint8_t bLogAddr );

bool_t  CecIsRoomInTaskQueue( void );
bool_t  CecAddTask( SiiCecTaskState_t *pNewTask );
void    CecTaskServer( SiiCpiStatus_t *pCecStatus );


#endif // __SI_CEC_INTERNAL_H__
