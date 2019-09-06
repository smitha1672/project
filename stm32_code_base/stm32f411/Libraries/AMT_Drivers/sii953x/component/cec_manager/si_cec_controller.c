//***************************************************************************
//!file     si_api cec_controller.c
//!brief    Silicon Image mid-level CEC handler
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include <string.h>
#include "si_cec_internal.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#endif 


//------------------------------------------------------------------------------
// Component data
//------------------------------------------------------------------------------

uint8_t l_devTypes [16] =
{
    CEC_LOGADDR_TV,
    CEC_LOGADDR_RECDEV1,
    CEC_LOGADDR_RECDEV1,
    CEC_LOGADDR_TUNER1,
    CEC_LOGADDR_PLAYBACK1,
    CEC_LOGADDR_AUDSYS,
    CEC_LOGADDR_PURE_SWITCH,
    CEC_LOGADDR_VIDEO_PROCESSOR,
    CEC_LOGADDR_PLAYBACK1,
    CEC_LOGADDR_RECDEV1,
    CEC_LOGADDR_TUNER1,
    CEC_LOGADDR_PLAYBACK1,
    CEC_LOGADDR_RECDEV2,
    CEC_LOGADDR_RECDEV2,
    CEC_LOGADDR_TV,
    CEC_LOGADDR_TV
};

//------------------------------------------------------------------------------
// Module data
//------------------------------------------------------------------------------

uint8_t CecVendorID[3] = {0x00,0x01,0x03}; //should be SIMG vendor ID

static uint8_t cecValidate [128] =
{
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1,

    //0x0E - 0x0F Reserved
    0, 0,

    1, 1,

    //0x12 - 0x1C Reserved
    0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0,

    1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1,

    //0x2D - 0x2E Reserved
    0, 0,

    1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1,

    //0x39 - 0x3F Reserved
    1, 1, 1,
    1, 0, 0, 0,

    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1,

    //0x4D - 0x4F Reserved
    0, 0, 0,

    1, 1, 1, 1,
    1, 1, 1, 1,

    //0x58 - 0x5F Reserved
    0, 0, 0, 0,
    0, 0, 0, 0,

    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1, 1, 1,
    1, 1,

    //0x6E - 0x70 Reserved
    0, 0,
    0,

    1, 1, 1,
    1, 1, 1,

    // 0x77 - 0x7F Reserved
    0,
    0, 0, 0, 0,
    0, 0, 0, 0
};


//------------------------------------------------------------------------------
//! @brief  Check for valid CEC key code.
//! @param[in]  CEC RC key
//------------------------------------------------------------------------------
bool_t SiiCecValidateKeyCode ( uint8_t keyData )
 {
    bool_t  validKey = false;

    // All keys 0x80 - 0xFF are invalid, use the table for the rest
    if (( cecValidate[ keyData & ~BIT7]) != 0 )
    {
        validKey = true;
    }

    return( validKey );
 }

//------------------------------------------------------------------------------
// Function:
// Description:
//------------------------------------------------------------------------------

void CecPrintLogAddr ( uint8_t bLogAddr )
{

    //    DEBUG_PRINT( MSG_DBG, " [%X] %s\n", (int)bLogAddr, CpCecTranslateLA( bLogAddr ) );
    DEBUG_PRINT( MSG_DBG, " [%X] ", (int)bLogAddr );
}

//------------------------------------------------------------------------------
//! @brief      Broadcast a REPORT PHYSICAL ADDRESS message.
//!             Does not wait for a reply.
//! @param[in]  srcLa     - source CEC logical address
//! @param[in]  srcPa     - source CEC physical address
//! @return     uint16_t    16 bit message ID or 0 if CEC not enabled
//------------------------------------------------------------------------------
uint16_t SiiCecSendReportPhysicalAddress ( SiiCecLogicalAddresses_t srcLa, uint16_t srcPa )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_REPORT_PHYSICAL_ADDRESS;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( srcLa, CEC_LOGADDR_UNREGORBC );
    cecFrame.args[0]        = srcPa >> 8;           // [Physical Address] High
    cecFrame.args[1]        = srcPa & 0xFF;         // [Physical Address] Low
    cecFrame.args[2]        = l_devTypes[srcLa];    // [Device Type]
    cecFrame.argCount       = 3;

    return( SiiDrvCpiWrite( &cecFrame ));
}

uint16_t SiiCecSendMenuStatus(SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa, uint8_t menuStatus )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode        = CECOP_MENU_STATUS;
    cecFrame.srcDestAddr   = MAKE_SRCDEST( srcLa, destLa );
    cecFrame.args[0]       = menuStatus;
    cecFrame.argCount      = 1;

    return( SiiDrvCpiWrite( &cecFrame ));

}


uint16_t SiiCecSendDeckStatus( SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa, uint8_t deckStatus  )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode        = CECOP_DECK_STATUS;
    cecFrame.srcDestAddr   = MAKE_SRCDEST( srcLa, destLa );
    cecFrame.args[0]       = deckStatus;
    cecFrame.argCount      = 1;

    return( SiiDrvCpiWrite( &cecFrame ));

}

//------------------------------------------------------------------------------
//! @brief      Broadcast a REPORT PHYSICAL ADDRESS message.
//!             Does not wait for a reply.
//! @param[in]  srcLa     - source CEC logical address
//! @param[in]  srcPa     - source CEC physical address
//! @return     uint16_t    16 bit message ID or 0 if CEC not enabled
//------------------------------------------------------------------------------
uint16_t SiiCecSendReportPowerStatus ( SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa, uint8_t powerState )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode        = CECOP_REPORT_POWER_STATUS;
    cecFrame.srcDestAddr   = MAKE_SRCDEST( srcLa, destLa );
    cecFrame.args[0]       = powerState;
    cecFrame.argCount      = 1;

    return( SiiDrvCpiWrite( &cecFrame ));
}

uint16_t SiiCecSendVendorId(  SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa,uint8_t *vendorId )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode        = CECOP_DEVICE_VENDOR_ID;
    cecFrame.srcDestAddr   = MAKE_SRCDEST( srcLa, destLa );
    cecFrame.args[0]       = vendorId[0];
    cecFrame.args[1]       = vendorId[1];
    cecFrame.args[2]       = vendorId[2];
    cecFrame.argCount      = 3;

    return( SiiDrvCpiWrite( &cecFrame ));

}

//------------------------------------------------------------------------------
// Function:    CecSendSetStreamPath
// Description: Broadcast a SET STREAM PATH message.  Does not wait for a reply.
//------------------------------------------------------------------------------

void CecSendSetStreamPath ( uint16_t destPhysAddr )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_SET_STREAM_PATH;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( pCec->logicalAddr, CEC_LOGADDR_UNREGORBC );
    cecFrame.args[0]        = (uint8_t)(destPhysAddr >> 8);
    cecFrame.args[1]        = (uint8_t)destPhysAddr;
    cecFrame.argCount       = 2;

    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  Send the device name string as the OSD name
//------------------------------------------------------------------------------
static void CecSendSetOsdName ( uint8_t destLogicalAddr )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_SET_OSD_NAME;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( pCec->logicalAddr, destLogicalAddr );
    memcpy( &cecFrame.args, pCec->osdName, pCec->osdNameLen );
    cecFrame.argCount       = pCec->osdNameLen;

    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  
//------------------------------------------------------------------------------
void CecSendTextViewOn ( uint8_t destLogicalAddr )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_TEXT_VIEW_ON;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( pCec->logicalAddr, destLogicalAddr );
    cecFrame.argCount      = 0;

    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  
//------------------------------------------------------------------------------
void CecSendUserControlPressed ( uint8_t destLogicalAddr, SiiCecUiCommand_t rcCode )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_USER_CONTROL_PRESSED;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( pCec->logicalAddr, destLogicalAddr );
    cecFrame.args[0] = rcCode;
    cecFrame.argCount      = 1;

    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  
//------------------------------------------------------------------------------
void CecSendUserControlReleased ( uint8_t destLogicalAddr, SiiCecUiCommand_t rcCode )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_USER_CONTROL_RELEASED;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( pCec->logicalAddr, destLogicalAddr );
    cecFrame.argCount      = 0;

    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
// Function:    CecHandleFeatureAbort
// Description: Received a failure response to a previous message.  Print a
//              message and notify the rest of the system
//TODO: Enhancement - This function should also keep track of the opcode and the
//              follower that does not recognize the opcode so that the opcode
//              is not sent to that follower again.
//------------------------------------------------------------------------------

static void CecHandleFeatureAbort ( SiiCpiData_t *pMsg )
{
    DEBUG_PRINT(
         CEC_MSG_STAT,
        "\nCEC Message %02X was Feature Aborted (%02X) by logical address %d\n",
        (int)pMsg->args[0],
        pMsg->args[1],
        (int)(pMsg->srcDestAddr >> 4)
        );
}

//------------------------------------------------------------------------------
// Function:    CecValidateMessage
// Description: Validate parameter count of passed CEC message
//              Returns FALSE if not enough operands for the message
//              Returns TRUE if the correct amount or more of operands (if more
//              the message processor will just ignore them).
//------------------------------------------------------------------------------

static bool_t CecValidateMessage ( SiiCpiData_t *pMsg )
{
    uint8_t parameterCount = 0;
    bool_t  countOK = true;
    bool_t  isFromUnregistered = false;

    // If message is from Broadcast address, we ignore it except for
    // some specific cases.

    if (( pMsg->srcDestAddr & 0xF0 ) == 0xF0 )
    {
        switch ( pMsg->opcode )
        {
            case CECOP_STANDBY:
            case CECOP_SYSTEM_AUDIO_MODE_REQUEST:
            case CECOP_ROUTING_CHANGE:
            case CECOP_ROUTING_INFORMATION:
            case CECOP_ACTIVE_SOURCE:
            case CECOP_GIVE_PHYSICAL_ADDRESS:
            case CECOP_REPORT_PHYSICAL_ADDRESS:
            case CECOP_REQUEST_ACTIVE_SOURCE:
            case CECOP_GET_MENU_LANGUAGE:
            case CECOP_SET_STREAM_PATH:
            case CDCOP_HEADER:
                break;
            default:
                isFromUnregistered = true;          // All others should be ignored
                break;
        }
    }

    /* Determine required parameter count   */

    switch ( pMsg->opcode )
    {
        case CECOP_IMAGE_VIEW_ON:
        case CECOP_TEXT_VIEW_ON:
        case CECOP_STANDBY:
        case CECOP_GIVE_PHYSICAL_ADDRESS:
        case CECOP_GIVE_DEVICE_VENDOR_ID:
        case CECOP_GIVE_DEVICE_POWER_STATUS:
        case CECOP_GET_MENU_LANGUAGE:
        case CECOP_GET_CEC_VERSION:
        case CECOP_INITIATE_ARC:
        case CECOP_REPORT_ARC_INITIATED:
        case CECOP_REPORT_ARC_TERMINATED:
        case CECOP_REQUEST_ARC_INITIATION:
        case CECOP_REQUEST_ARC_TERMINATION:
        case CECOP_TERMINATE_ARC:
        case CECOP_ABORT:
	case CECOP_GIVE_AUDIO_STATUS:
            parameterCount = 0;
            break;
        case CECOP_REPORT_POWER_STATUS:         // power status
        case CECOP_CEC_VERSION:                 // cec version
            parameterCount = 1;
            break;
	case	CECOP_SET_STREAM_PATH:
        case CECOP_INACTIVE_SOURCE:             // physical address
        case CECOP_FEATURE_ABORT:               // feature opcode / abort reason
        case CECOP_ACTIVE_SOURCE:               // physical address
            parameterCount = 2;
            break;
        case CECOP_REPORT_PHYSICAL_ADDRESS:     // physical address / device type
        case CECOP_DEVICE_VENDOR_ID:            // vendor id
            parameterCount = 3;
            break;
        case CECOP_USER_CONTROL_PRESSED:        // UI command
        case CECOP_SET_OSD_NAME:                // osd name (1-14 bytes)
        case CECOP_SET_OSD_STRING:              // 1 + x   display control / osd string (1-13 bytes)
            parameterCount = 1;                 // must have a minimum of 1 operands
            break;

        default:
            break;
    }

    /* Test for correct parameter count.    */

    if (( pMsg->argCount < parameterCount ) || isFromUnregistered )
    {
        countOK = false;
    }

    return( countOK );
}

//------------------------------------------------------------------------------
// Function:    UpdateTaskQueueState
// Description: If the current task is idle, clear the queue state and bump
//              the task out index to the next task.
// NOTE:        This function is to be called ONLY after the task server has
//              run a task function.
//------------------------------------------------------------------------------

static void UpdateTaskQueueState ( void )
{
    if ( pCec->currentTask == SiiCECTASK_IDLE )
    {
        ACTIVE_TASK.queueState = SiiCecTaskQueueStateIdle;
        pCec->taskQueueOut = (pCec->taskQueueOut + 1) % TASK_QUEUE_LENGTH;
    }
}

//------------------------------------------------------------------------------
//! @brief  Return state of current enumeration task.
//------------------------------------------------------------------------------
bool_t SiiCecEnumerateIsComplete ( void )
{
    return( pCec->enumerateComplete );
}

//------------------------------------------------------------------------------
//! @brief  Return availability of passed logical address.
//------------------------------------------------------------------------------
bool_t SiiCecDeviceLaIsAvailable ( uint8_t deviceLa )
{
    return( pCec->logicalDeviceInfo[ deviceLa].deviceType == CEC_DT_COUNT );
}

//------------------------------------------------------------------------------
//! @brief      Ping every logical address on the CEC bus and log the attached
//!             devices
// cecTaskState.taskData1 == Not Used.
// cecTaskState.taskData2 == Not used
//------------------------------------------------------------------------------
static uint8_t CecTaskEnumerateDevices ( SiiCpiStatus_t *pCecStatus )
{
    uint8_t newTask = ACTIVE_TASK.task;

    switch ( ACTIVE_TASK.cpiState )
    {
    case CPI_IDLE:

        // We're done if we've reached the unregistered address
        if ( ACTIVE_TASK.pTaskData3[ ACTIVE_TASK.taskData1] >= CEC_LOGADDR_UNREGORBC )
        {
            pCec->enumerateComplete = true;
            SiiCecCbEnumerateComplete( true );          // Let the app layer know.
            //DEBUG_PRINT( CEC_MSG_DBG, "ENUM DONE\n" );
            ACTIVE_TASK.cpiState = CPI_IDLE;
            newTask = SiiCECTASK_IDLE;
            break;
        }
        ACTIVE_TASK.destLA = ACTIVE_TASK.pTaskData3[ ACTIVE_TASK.taskData1++];
        ACTIVE_TASK.msgId = SiiDrvCpiSendPing( ACTIVE_TASK.destLA );
        ACTIVE_TASK.cpiState = CPI_WAIT_ACK;
        break;

    case CPI_WAIT_ACK:
        // Make sure this message status is associated with the message we sent.
        if ( pCecStatus->msgId != ACTIVE_TASK.msgId )
        {
            break;
        }
        if ( pCecStatus->txState == SiiTX_SENDFAILED )
        {
            //DEBUG_PRINT( MSG_DBG, "Enum Task NoAck" );
            CecPrintLogAddr( ACTIVE_TASK.destLA );
           // DEBUG_PRINT( MSG_DBG, "\n" );

            // Remove LA from active list unless it is a virtual device,
            // which apparently do not respond to our ping even though
            // they are enabled in the CAPTURE_ID register.
            if ( !pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].isVirtual )
            {
                //DEBUG_PRINT( CEC_MSG_DBG, "Remove LA %X from CEC list\n", ACTIVE_TASK.destLA );
                pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].cecPA = 0xFFFF;
                pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].deviceType = CEC_DT_COUNT;
                pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].selected = false;
            }

            // Restore Tx State to IDLE to try next address.
            ACTIVE_TASK.cpiState = CPI_IDLE;
        }
        else if ( pCecStatus->txState == SiiTX_SENDACKED )
        {
            //DEBUG_PRINT( MSG_DBG, "Rx Enum Ack" );
            CecPrintLogAddr( ACTIVE_TASK.destLA );
           // DEBUG_PRINT( MSG_DBG, "\n" ); Remove LA

            // Get the physical address from this source and add it to our
            // list if it responds within 2 seconds, otherwise, ignore it.
            //DEBUG_PRINT(MSG_ALWAYS,"PHYSICAL ADDRESS IS BEING REPORTED WITH DST LA:%d\n",ACTIVE_TASK.destLA);
            ACTIVE_TASK.msgId = SiiCecSendMessage( CECOP_GIVE_PHYSICAL_ADDRESS, ACTIVE_TASK.destLA );
            SiiOsTimerSet( &ACTIVE_TASK.taskTimer, 2000 );
            ACTIVE_TASK.cpiState = CPI_WAIT_RESPONSE;
        }
        break;

    case CPI_WAIT_RESPONSE:
        if ( SiiOsTimerExpired( ACTIVE_TASK.taskTimer ))
        {
            DEBUG_PRINT( CEC_MSG_DBG, "RX Enumerate: Timed out waiting for response\n" );

            // Ignore this LA and move on to the next.
            ACTIVE_TASK.cpiState = CPI_IDLE;
        }
        break;

    case CPI_RESPONSE:
        // The CEC Rx Message Handler has updated the child port list,
        // restore Tx State to IDLE to try next address.
        ACTIVE_TASK.cpiState = CPI_IDLE;
        break;
    }

    return( newTask );
}

//------------------------------------------------------------------------------
//! @brief      Send a CEC message and wait for ACK.  Perform a callback
//!             when ACK is received or a timeout occurs.
//! @note       This task STARTS in CPI_WAIT_ACK state.
// cecTaskState.taskData1 == Not Used.
// cecTaskState.taskData2 == Not used
//------------------------------------------------------------------------------
static uint8_t CecTaskSendMsg ( SiiCpiStatus_t *pCecStatus )
{
    uint8_t newTask = ACTIVE_TASK.task;

    switch ( ACTIVE_TASK.cpiState )
    {
        case CPI_IDLE:

            ACTIVE_TASK.cpiState = CPI_IDLE;
            newTask = SiiCECTASK_IDLE;
            break;

        case CPI_WAIT_ACK:
            // Make sure this message status is associated with the message we sent.
            if ( pCecStatus->msgId != ACTIVE_TASK.msgId )
            {
                break;
            }
            if ( pCecStatus->txState == SiiTX_SENDFAILED )
            {
               // DEBUG_PRINT( MSG_DBG, "CEC Message Send NoAck" );
                CecPrintLogAddr( ACTIVE_TASK.destLA );
                DEBUG_PRINT( MSG_DBG, "\n" );

                // Remove LA from active list
                //DEBUG_PRINT( CEC_MSG_DBG, "Remove LA %X from CEC list\n", ACTIVE_TASK.destLA );
                pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].cecPA      = 0xFFFF;
                pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].deviceType = CEC_DT_COUNT;
                pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].selected   = false;
                pCec->logicalDeviceInfo[ ACTIVE_TASK.destLA].isVirtual  = false;
                SiiCecCbSendMessage( ACTIVE_TASK.taskData1, false );
            }
            else if ( pCecStatus->txState == SiiTX_SENDACKED )
            {
                SiiCecCbSendMessage( ACTIVE_TASK.taskData1, true );
            }
            ACTIVE_TASK.cpiState = CPI_IDLE;
            break;
        default:
            ACTIVE_TASK.cpiState = CPI_IDLE;
            break;
    }

    return( newTask );
}

//------------------------------------------------------------------------------
//! @brief      Returns an available logical address from the passed list of
//!             devices.
//! @param[in]  pDeviceList - list of logical addresses to check
//! @return     Available logical address or CEC_LOGADDR_UNREGORBC.
//------------------------------------------------------------------------------
uint8_t SiiCecGetAvailableLa ( uint8_t *pDeviceLaList )
{
    int     i;
    bool_t  laFound = false;

    // We must have already enumerated
    if ( pCec->enumerateComplete )
    {
        for ( i = 0; pDeviceLaList[i] < CEC_LOGADDR_UNREGORBC; i++ )
        {
            CecPrintLogAddr( pDeviceLaList[i] );
            if ( SiiCecDeviceLaIsAvailable( pDeviceLaList[i] ))
            {
                laFound = true;
                break;
            }
        }

        if ( !laFound )
        {
            DEBUG_PRINT( MSG_DBG, "No LA available from list\n" );
        }
    }
    else
    {
        DEBUG_PRINT( MSG_DBG, "Must be enumerated to set LA\n" );
    }

    return( laFound ? pDeviceLaList[i] : CEC_LOGADDR_UNREGORBC );
}

//------------------------------------------------------------------------------
//! @brief  Send the device name string as the OSD name
//------------------------------------------------------------------------------
bool_t SiiCecSetOsdName ( char *pOsdName )
{
    int nameLen = strlen( pOsdName );

    pCec->lastResultCode = RESULT_CEC_INVALID_PARAMETER;
    if ( nameLen < 14 )
    {
        pCec->osdNameLen = nameLen;
        memcpy( pCec->osdName, pOsdName, pCec->osdNameLen );
        pCec->lastResultCode = RESULT_CEC_SUCCESS;
    }

    return( pCec->lastResultCode == RESULT_CEC_SUCCESS );
}

//------------------------------------------------------------------------------
//! @brief  Set the device logical address based on the first available entry
//!         in the passed list of device addresses.  Expects that an enumerate
//!         function has been run.
//------------------------------------------------------------------------------
bool_t SiiCecEnumerateDeviceLa ( uint8_t *pDeviceList )
{
    SiiCecTaskState_t  newTask;

    newTask.task        = SiiCECTASK_SETLA;
    newTask.taskState   = 0;
    newTask.destLA      = 0;
    newTask.cpiState    = CPI_IDLE;
    newTask.taskData1   = 0;
    newTask.taskData2   = 0;
    newTask.pTaskData3  = pDeviceList;
    return( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
//! @brief  Cancel enumeration task.
//------------------------------------------------------------------------------
void SiiCecEnumerateCancel ( void )
{
    if ( pCec->currentTask == SiiCECTASK_ENUMERATE )
    {
        pCec->currentTask = SiiCECTASK_CANCEL;
        CecTaskServer( NULL );
        pCec->enumerateComplete = false;
    }
}

//------------------------------------------------------------------------------
//! @brief  Send the appropriate CEC commands to enumerate the passed list of
//!         logical devices on the CEC bus.
//! @return true if added to task queue, false if not
//------------------------------------------------------------------------------
bool_t SiiCecEnumerateDevices ( uint8_t *pDeviceList )
{
    SiiCecTaskState_t  newTask;

    pCec->enumerateComplete = false;
    SiiCecCbEnumerateComplete( false );     // Let the app layer know.

    newTask.task        = SiiCECTASK_ENUMERATE;
    newTask.taskState   = 0;
    newTask.destLA      = 0;
    newTask.cpiState    = CPI_IDLE;
    newTask.taskData1   = 0;
    newTask.taskData2   = 0;
    newTask.pTaskData3  = pDeviceList;
    return( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
//! @brief      Send the appropriate CEC commands to enumerate the passed list
//!             of logical devices on the CEC bus.
//! @param[in]  opCode  - CEC opcode to send
//! @param[in]  src     - source CEC logical address
//! @param[in]  dest    - destination CEC logical address
//! @return     bool_t  true if added to task queue, false if not
//------------------------------------------------------------------------------
bool_t SiiCecSendMessageTaskEx ( uint8_t opCode, uint8_t src, uint8_t dest )
{
    SiiCecTaskState_t  newTask;

    if ( !CecIsRoomInTaskQueue())
    {
        return( false );
    }

    // Add the message to the message queue, then the task will
    // wait for an ACK/NACK
    newTask.msgId       = SiiCecSendMessageEx( opCode, src, dest );
    if (( newTask.msgId == 0 ) || (SiiDrvCpiGetLastResult() != 0))
    {
        return( false );
    }

    // Message sent successfully, add it to the task queue
    newTask.task        = SiiCECTASK_SENDMSG;
    newTask.taskState   = 0;
    newTask.destLA      = 0;
    newTask.cpiState    = CPI_WAIT_ACK; // Start already looking for ACK/NOACK
    newTask.taskData1   = opCode;
    newTask.taskData2   = 0;
    newTask.pTaskData3  = 0;
    return( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
//! @brief      Send the appropriate CEC commands to enumerate the passed list
//!             of logical devices on the CEC bus.
//! @param[in]  opCode  - CEC opcode to send
//! @param[in]  dest    - destination CEC logical address
//! @return     bool_t  true if added to task queue, false if not
//!
//! @note       This function is deprecated; please use SiiCecSendMessageTaskEx
//------------------------------------------------------------------------------
bool_t SiiCecSendMessageTask ( uint8_t opCode, uint8_t dest )
{
    return( SiiCecSendMessageTaskEx ( opCode, pCec->logicalAddr, dest ));
}

//------------------------------------------------------------------------------
// Function:    CecTaskServer
// Description: Calls the current task handler.  A task is used to handle cases
//              where we must send and receive a specific set of CEC commands.
//
// NOTE:        If the contents of the SiiCpiStatus_t structure are not new,
//              they should be set to 0 before calling.
//------------------------------------------------------------------------------

void CecTaskServer ( SiiCpiStatus_t *pCecStatus )
{
    uint8_t logicalAddr;
    int_t   regTaskIndex;

    switch ( pCec->currentTask )
    {
        case SiiCECTASK_CANCEL:

            // If a task is in progress, cancel it.
            if ( pCec->taskQueue[ pCec->taskQueueOut].queueState == SiiCecTaskQueueStateRunning )
            {
                pCec->taskQueue[ pCec->taskQueueOut].queueState = SiiCecTaskQueueStateIdle;
                pCec->currentTask = SiiCECTASK_IDLE;
            }
            break;

        case SiiCECTASK_IDLE:
            if ( pCec->taskQueue[ pCec->taskQueueOut].queueState == SiiCecTaskQueueStateQueued )
            {
                pCec->taskQueue[ pCec->taskQueueOut].queueState = SiiCecTaskQueueStateRunning;
                pCec->currentTask = pCec->taskQueue[ pCec->taskQueueOut].task;
            }
            break;

        case SiiCECTASK_ENUMERATE:
            pCec->currentTask = CecTaskEnumerateDevices( pCecStatus );
            UpdateTaskQueueState();
            break;

        case SiiCECTASK_SETLA:
            logicalAddr = SiiCecGetAvailableLa( ACTIVE_TASK.pTaskData3 );
            if ( logicalAddr < CEC_LOGADDR_UNREGORBC )
            {
                SiiCecSetDeviceLA( logicalAddr );
            }
            pCec->currentTask = SiiCECTASK_IDLE;
            UpdateTaskQueueState();
            break;

        case SiiCECTASK_ONETOUCH:
            pCec->currentTask = CecTaskOneTouchPlay( pCecStatus );
            UpdateTaskQueueState();
            break;

        case SiiCECTASK_SENDMSG:
            pCec->currentTask = CecTaskSendMsg( pCecStatus );
            UpdateTaskQueueState();
            break;

        default:

            // Check the registered task list.
            regTaskIndex = pCec->currentTask - SiiCECTASK_COUNT;
            if (( regTaskIndex < MAX_CEC_TASKS) && ( pCec->pTasks[ regTaskIndex ] != 0))
            {
                pCec->currentTask = (*pCec->pTasks[ regTaskIndex])( pCecStatus );
            }
            break;
    }
}

#if INC_CBUS
extern bool_t SkAppCbusCecRxMsgFilter ( SiiCpiData_t *pMsg,  SiiCecLogicalAddresses_t virtualDevLA );
#endif
//------------------------------------------------------------------------------
//! @brief      This is the first message handler called in the chain. It parses
//!             messages that are handled the same way for every system type and
//!             those that it needs to keep the internal component status
//!             up to date.  Some message are left marked as unused so that the
//!             application layer can also see them.
//! @param[in]  pMsg - Pointer to CEC message received
//! @return     true if message consumed, false if not used
//------------------------------------------------------------------------------
static bool_t CecMsgHandlerFirst ( SiiCpiData_t *pMsg )
{
    bool_t  usedMessage         = true;
    bool_t  isDirectAddressed   = !((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );

    // Don't process unless it is intended for the local logical address (we must check
    // in case another device has been added for virtual device support).
    switch ( pMsg->opcode )
    {
        case CECOP_STANDBY:                                             // Direct and Broadcast
#if ( configSII_DEV_953x_PORTING == 0 )
            if ( pCec->powerState != CEC_POWERSTATUS_STANDBY )
            {
                // Next time through the main loop, power will be cycled off

                pCec->powerState    = CEC_POWERSTATUS_ON_TO_STANDBY;
                pCec->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;        // Signal upper layer
                pCec->isActiveSource = false;                           // Only impacts TX
                
		AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_OFF);
            }
#endif
            AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_OFF);
            break;

        case CECOP_GIVE_PHYSICAL_ADDRESS:
            if ( isDirectAddressed)                    // Ignore as broadcast message
            {
            	DEBUG_PRINT(MSG_ALWAYS, "PHYSICAL ADDRESS BEING GIVE FROM FIRST HANFLER\n");
                SiiCecSendReportPhysicalAddress( pCec->logicalAddr, pCec->physicalAddr );
            }
            else
            	usedMessage = false;
            break;

        case CECOP_REPORT_PHYSICAL_ADDRESS:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
            	if(pMsg->args[2] != 0)
            		pMsg->srcDestAddr = (pMsg->srcDestAddr & 0x0F) | (pMsg->args[2] << 4) ;
                SiiCecUpdateLogicalDeviceInfo((pMsg->srcDestAddr >> 4) & 0x0F,         // broadcast logical address
                    (((uint16_t)pMsg->args[0]) << 8) | pMsg->args[1],   // broadcast physical address
                    false
                    );

                // Let Enumerate task know about it.
                if ( ACTIVE_TASK.task == SiiCECTASK_ENUMERATE )
                {
                    ACTIVE_TASK.cpiState = CPI_RESPONSE;
                }
            }
            break;

        case CECOP_GIVE_DEVICE_POWER_STATUS:
            if ( isDirectAddressed )                // Ignore as broadcast message
            {
		POWER_STATE power_state;

		power_state = AmTCecGetSystemPowerStatus();
                SiiCecSendReportPowerStatus( pCec->logicalAddr, pMsg->srcDestAddr >> 4, (uint8_t)power_state );
            }
            else
            	usedMessage = false;
            break;

        case CECOP_REPORT_POWER_STATUS:                 // Someone sent us their power state.
            if ( isDirectAddressed )                    // Ignore as broadcast message
            {
                pCec->sourcePowerStatus = pMsg->args[0];
            }
            break;

        case CECOP_USER_CONTROL_PRESSED:
            if ( !isDirectAddressed )   break;  // Ignore as broadcast message

            // If not a VALID CEC key, feature abort
            if ( !SiiCecValidateKeyCode( pMsg->args[0] ))
            {
                SiiCecFeatureAbortSendEx( pMsg->opcode, CECAR_INVALID_OPERAND, pCec->logicalAddr, GET_CEC_SRC( pMsg->srcDestAddr ));
                break;              // Used the message...
            }
            usedMessage = false;    // Didn't use the message, we just validated the key ID
            break;

        case CECOP_GIVE_OSD_NAME:
        	SiiCecSetOsdName(pCec->osdName);
        	usedMessage = false;
        	break;

        case CECOP_GIVE_DEVICE_VENDOR_ID:
        	SiiCecSendVendorId(pCec->logicalAddr, CEC_LOGADDR_UNREGORBC, CecVendorID);
        	break;

        default:
            usedMessage = false;                        // Didn't use the message
            break;
    }

    return( usedMessage );
}

//------------------------------------------------------------------------------
// Function:    CecMsgHandlerLast
// Description: This is the last message handler called in the chain, and
//              parses any messages left untouched by the previous handlers.
//
// NOTE:        Messages handled here should be common to all system types.
//
//------------------------------------------------------------------------------

static void CecMsgHandlerLast ( SiiCpiData_t *pMsg )
{
    uint8_t         srcAddr;
    bool_t          isDirectAddressed;
    SiiCpiData_t    cecFrame;

    isDirectAddressed   = !((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );
    srcAddr             = GET_CEC_SRC( pMsg->srcDestAddr );

    switch ( pMsg->opcode )
    {
        case CECOP_FEATURE_ABORT:
            if ( isDirectAddressed )                // Ignore as broadcast message
            {
                CecHandleFeatureAbort( pMsg );
            }
            break;

            // These messages have already been handled for internal purposes
            // by CecRxMsgHandler and passed to the application level
            // and/or were ignored but not consumed by the application level.
            // Ignore them here.

        case CECOP_IMAGE_VIEW_ON:
        case CECOP_TEXT_VIEW_ON:
        case CECOP_GET_MENU_LANGUAGE:
        case CECOP_USER_CONTROL_PRESSED:
        case CECOP_USER_CONTROL_RELEASED:
#if ( configSII_DEV_953x_PORTING == 1 )            
        case CECOP_GIVE_AUDIO_STATUS:
        case CECOP_SYSTEM_AUDIO_MODE_REQUEST:
#endif            
            break;

            // Handle this here because the app level may have upgraded the version
            // and handled it before it gets here.

        case CECOP_GET_CEC_VERSION:
            if ( isDirectAddressed )                    // Ignore as broadcast message
            {
                // Respond to this request with the CEC version support.
                cecFrame.srcDestAddr   = MAKE_SRCDEST( pCec->logicalAddr, pMsg->srcDestAddr >> 4 );
                cecFrame.opcode        = CECOP_CEC_VERSION;
                cecFrame.args[0]       = 0x05;       // Report CEC1.4
                cecFrame.argCount      = 1;
                SiiDrvCpiWrite( &cecFrame );
            }
            break;

        case CECOP_GIVE_OSD_NAME:
            if ( isDirectAddressed )
            {
                CecSendSetOsdName( srcAddr );
            }
            break;

            // Ignore these messages if unrecognized AND broadcast
            // but feature abort them if directly addressed
        case CECOP_GIVE_DEVICE_VENDOR_ID:
        case CDCOP_HEADER:
            if ( isDirectAddressed )
            {
                SiiCecFeatureAbortSendEx( pMsg->opcode, CECAR_UNRECOG_OPCODE, GET_CEC_DEST( pMsg->srcDestAddr), srcAddr );
            }
            break;

            // Any directly addressed message that gets here is not supported by this
            // device, so feature abort it with unrecognized opcode.
            // This means that the app layer must be sure to properly handle any messages
            // that it should be able to handle.

        case CECOP_ABORT:
        default:
            if((pMsg->opcode != CECOP_ABORT) &&
            		((pMsg->opcode == CECOP_ROUTING_CHANGE) || (pMsg->opcode == CECOP_ROUTING_INFORMATION) || (pMsg->opcode == CECOP_SET_STREAM_PATH)))
                break;
            if ( isDirectAddressed )                    // Ignore as broadcast message
            {
                SiiCecFeatureAbortSendEx( pMsg->opcode, CECAR_UNRECOG_OPCODE, GET_CEC_DEST( pMsg->srcDestAddr), srcAddr );
            }
            break;
    }
}

//------------------------------------------------------------------------------
// Function:    SiiCecHandler
// Description: Polls the send/receive state of the CPI hardware and runs the
//              current task, if any.
//
//              A task is used to handle cases where we must send and receive
//              a specific set of CEC commands.
//------------------------------------------------------------------------------

void SiiCecHandler ( void )
{
    bool_t          processedMsg;
    int_t           callbackIndex, frameCount;
    SiiCpiStatus_t  cecStatus;
    SiiCpiData_t    cecFrame;
    uint8_t			cecDevice;
	bool_t isVirtualDev = false;

    do
    {
        if ( !pCec->enable ) break;

        SiiDrvCpiServiceWriteQueue();               // Send any pending messages
        SiiDrvCpiHwStatusGet( &cecStatus );         // Get the CEC transceiver status

        // Now look to see if any CEC commands were received.
        if ( cecStatus.rxState )
        {
            // Get CEC frames until no more are present.
            cecStatus.rxState = 0;      // Clear activity flag
            for ( ;; )
            {
                frameCount = SiiDrvCpiFrameCount();
                if ( frameCount == 0 )
                {
                    break;
                }
                if ( !SiiDrvCpiRead( &cecFrame ))
                {
                    DEBUG_PRINT( CEC_MSG_DBG, "Error in Rx Fifo\n" );
                    break;
                }

                processedMsg = false;
                if ( !CecValidateMessage( &cecFrame ))   // If invalid message, ignore it, but treat it as handled
                {
                    processedMsg = true;
                }

		//TRACE_DEBUG((0, "SiiCecHandler pMsg->opcode = 0x%X", cecFrame.opcode ));
		//TRACE_DEBUG((0, "Source and Destination = 0x%X", cecFrame.srcDestAddr));
		
#if INC_CBUS
                for(cecDevice = 0; cecDevice < (CHIP_MAX_VIRTUAL_CEC_DEVS + 1); cecDevice++)//should handle for all cec devices our device is handling
                {
                	switch(cecDevice)
                	{
                	case 0:
                		pCec->logicalAddr = pCec->devLogicalAddr; //get back the actual device logical address
                	    if ( (GET_CEC_DEST( cecFrame.srcDestAddr) != CEC_LOGADDR_UNREGORBC ) && (GET_CEC_DEST( cecFrame.srcDestAddr) != pCec->logicalAddr ))
                	    {
                	    	processedMsg = true;; // Don't handle the message
                	    }
                 		break;
                	case 1:
                	case 2:
                		if(pCec->virtualDevLA[cecDevice-1] != CEC_LOGADDR_TV)
                		{
                			processedMsg = false;
                			pCec->logicalAddr = pCec->virtualDevLA[cecDevice-1]; // assign the virtual logical address with which we need to respond
                			isVirtualDev = true;
                			if((GET_CEC_DEST( cecFrame.srcDestAddr) == CEC_LOGADDR_UNREGORBC) || (GET_CEC_DEST( cecFrame.srcDestAddr) == pCec->logicalAddr ))
                			{
                				if(SkAppCbusCecRxMsgFilter(&cecFrame,  pCec->virtualDevLA[cecDevice-1]))
                				{
                					processedMsg = true;
                				}
                				else
                				{
                					processedMsg = false;
                				}
                			}
                			else
                			{
                				processedMsg = true;
                			}
                		}
                		else
                		{
                			processedMsg = true;
                		}
                		break;
                	default:
                		processedMsg = true;
                		break;
                	}
#endif
                	if ( !processedMsg && CecMsgHandlerFirst( &cecFrame ))   // Handle the common system messages
                	{
                		processedMsg = true;
                	}

                	// Iterate through registered message handlers.
                	for ( callbackIndex = 0; callbackIndex < MAX_CEC_PARSERS; callbackIndex++ )
                	{
                		if ( pCec->messageHandlers[ callbackIndex].pMsgHandler != 0)
                		{
                			// Some handlers must be called every time through the handler.
                			if ( pCec->messageHandlers[ callbackIndex].callAlways )
                			{
                				if ( (*pCec->messageHandlers[ callbackIndex].pMsgHandler)( &cecFrame ) )
                				{
                					processedMsg = true;
							break;
                				}
                			}

                			// Other handlers are called only if the message has not been used.
                			else if ( !processedMsg )
                			{
                				if ( (*pCec->messageHandlers[ callbackIndex].pMsgHandler)( &cecFrame ) )
                				{
                					processedMsg = true;
							break;
                				}
                			}
                		}

                	}


                	if ( !processedMsg )
                	{
                		CecMsgHandlerLast( &cecFrame );   // Let the built-in handler take care of leftovers.
                	}
#if INC_CBUS
                }
#endif
            }
        }
        else
        {
            // No message, but iterate through registered message handlers in case
            // one or more has an 'always call' tag.  In this case, the handler
            // is called with a null pointer instead of a pointer to a CEC message
            for ( callbackIndex = 0; callbackIndex < MAX_CEC_PARSERS; callbackIndex++ )
            {
                if ( pCec->messageHandlers[ callbackIndex].pMsgHandler != 0)
                {
                    if ( pCec->messageHandlers[ callbackIndex].callAlways )
                    {
                        (*pCec->messageHandlers[ callbackIndex].pMsgHandler)( NULL );
                    }
                }
            }
        }

        // After the current message (if any) has been processed, cecStatus.msgId is
        // still valid, so we pass that information to the task server in case the message
        // was one that a task was waiting for.
        CecTaskServer( &cecStatus );                // Process task even if no new status

    } while ( 0 );
}

//------------------------------------------------------------------------------
//! @brief  Determine if there is a slot available in the CEC task queue
//------------------------------------------------------------------------------
bool_t CecIsRoomInTaskQueue ( void )
{

    if ( pCec->taskQueue[ pCec->taskQueueIn ].queueState == SiiCecTaskQueueStateIdle )
    {
        return( true );
    }

    return( false );
}

//------------------------------------------------------------------------------
//! @brief  Add the passed task to the CEC task queue if room.
//------------------------------------------------------------------------------
bool_t CecAddTask ( SiiCecTaskState_t *pNewTask )
{
    bool_t      success = true;

    // Store the message in the task queue
    if ( pCec->taskQueue[ pCec->taskQueueIn ].queueState == SiiCecTaskQueueStateIdle )
    {
        memcpy( &pCec->taskQueue[ pCec->taskQueueIn ], pNewTask, sizeof( SiiCecTaskState_t ));
        pCec->taskQueue[ pCec->taskQueueIn ].queueState = SiiCecTaskQueueStateQueued;

        pCec->taskQueueIn = (pCec->taskQueueIn + 1) % TASK_QUEUE_LENGTH;
    }
    else
    {
        //DEBUG_PRINT( MSG_DBG, "CEC Task Queue full\n" );
        success = false;
    }

    pCec->lastResultCode = (success) ? RESULT_CEC_SUCCESS : RESULT_CEC_TASK_QUEUE_FULL;
    return( success );
}

