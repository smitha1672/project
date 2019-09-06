//***************************************************************************
//!file     si_api cec_tx_handler.c
//!brief    Built-in tasks and handlers for CEC PLAYBACKx devices
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include <string.h>
#include "si_cec_component.h"
#include "si_cec_internal.h"
#include "si_timer.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#endif 

/* One Touch Play Task internal states    */
enum
{
    SiiCecTaskOtpIdle           = 0,
    SiiCecTaskOtpStart,
    SiiCecTaskOtpSendImageViewOn,
    SiiCecTaskOtpSentActiveSource,
};

//------------------------------------------------------------------------------
//! @brief  Send an ACTIVE SOURCE message.  Does not wait for a reply.
//! @param[in]  - logicalAddr   Logical device going active
//! @param[in]  - physicalAddr  Physical address of device
//------------------------------------------------------------------------------
void SiiCecSendActiveSource ( uint8_t logicalAddr, uint16_t physicalAddr )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_ACTIVE_SOURCE;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( logicalAddr, CEC_LOGADDR_UNREGORBC );
    cecFrame.args[0]        = physicalAddr >> 8;        // [Physical Address] High
    cecFrame.args[1]        = physicalAddr & 0xFF;      // [Physical Address] Low
    cecFrame.argCount       = 2;

    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
//! @brief  Send an INACTIVE SOURCE message.  Does not wait for a reply.
//! @param[in]  - logicalAddr   Logical device going inactive
//! @param[in]  - physicalAddr  Physical address of device
//------------------------------------------------------------------------------
void SiiCecSendInactiveSource ( uint8_t logicalAddr, uint16_t physicalAddr )
{
    SiiCpiData_t cecFrame;

    cecFrame.opcode         = CECOP_INACTIVE_SOURCE;
    cecFrame.srcDestAddr    = MAKE_SRCDEST( logicalAddr, CEC_LOGADDR_UNREGORBC );
    cecFrame.args[0]        = physicalAddr >> 8;        // [Physical Address] High
    cecFrame.args[1]        = physicalAddr & 0xFF;      // [Physical Address] Low
    cecFrame.argCount       = 2;

    SiiDrvCpiWrite( &cecFrame );
}

//------------------------------------------------------------------------------
// Function:    CecTaskCpiWaitAck
// Description: Waits for an ACK from the last command sent.
//------------------------------------------------------------------------------

static uint8_t CecTaskCpiWaitAck ( SiiCpiStatus_t *pCecStatus )
{
    uint8_t         newTask = ACTIVE_TASK.task;

    if ( pCecStatus->txState == SiiTX_SENDFAILED )
    {
        DEBUG_PRINT( CEC_MSG_DBG,( "Task:: NoAck received\n" ));

            /* Abort task */

        ACTIVE_TASK.cpiState    = CPI_IDLE;
        newTask                         = SiiCECTASK_IDLE;
    }
    else if ( pCecStatus->txState == SiiTX_SENDACKED )
    {
        DEBUG_PRINT( CEC_MSG_DBG,( "Task:: ACK received\n" ));
    }
    return( newTask );
}

//------------------------------------------------------------------------------
// Function:    CecTaskOneTouchPlay
// Description: Implement One Touch Play message sequence.
//
// cecTaskState.taskData1 ==
// cecTaskState.taskData1 ==
// cecTaskState.taskData2 ==
//------------------------------------------------------------------------------

uint8_t CecTaskOneTouchPlay ( SiiCpiStatus_t *pCecStatus )
{
    SiiCpiData_t    cecFrame;
    uint8_t         newTask = ACTIVE_TASK.task;

    switch ( ACTIVE_TASK.cpiState )
    {
        case CPI_IDLE:

            switch ( ACTIVE_TASK.taskState )
            {
                case SiiCecTaskOtpStart:
                    // Send an Image View On command to wake up the TV

                    cecFrame.opcode             = CECOP_IMAGE_VIEW_ON;
                    cecFrame.srcDestAddr        = MAKE_SRCDEST( pCec->logicalAddr, CEC_LOGADDR_TV );
                    cecFrame.argCount           = 0;

                    SiiDrvCpiWrite( &cecFrame );
                    ACTIVE_TASK.taskState       = SiiCecTaskOtpSendImageViewOn;
                    ACTIVE_TASK.cpiState        = CPI_WAIT_ACK;
                    break;

                case SiiCecTaskOtpSendImageViewOn:
                    // Now send an ACTIVE SOURCE message to tell them who we are

                    SiiCecSendActiveSource( pCec->logicalAddr, pCec->physicalAddr );
                    DEBUG_PRINT( CEC_MSG_DBG, ("TX: Sent Active Source\n" ));
                    ACTIVE_TASK.taskState       = SiiCecTaskOtpSentActiveSource;
                    break;

                case SiiCecTaskOtpSentActiveSource:
                    newTask = SiiCECTASK_IDLE;
                    pCec->isActiveSource = true;
                    break;
            }

            break;

        case CPI_WAIT_ACK:
            ACTIVE_TASK.cpiState = CPI_IDLE;
            newTask = CecTaskCpiWaitAck( pCecStatus );  // Will go to idle task if no ack received
            break;
        default:
            break;
    }

    return( newTask );
}

//------------------------------------------------------------------------------
// Function:    SI_CecOneTouchPlay
// Description: Send the appropriate CEC commands to the specified logical
//              device on the CEC Tx bus.
//------------------------------------------------------------------------------

bool_t SiiCecOneTouchPlay ( void )
{
    SiiCecTaskState_t  newTask;

    newTask.task        = SiiCECTASK_ONETOUCH;
    newTask.taskState   = SiiCecTaskOtpStart;
    newTask.destLA      = 0;
    newTask.cpiState    = CPI_IDLE;
    newTask.taskData1   = 0;
    newTask.taskData2   = 0;
    return( CecAddTask( &newTask ));
}

//------------------------------------------------------------------------------
// Function:    si_CecTxMsgHandler
// Description: This is the first TX message handler called in the chain. It
//              parses messages that it needs to keep the internal component
//              status up to date.
//------------------------------------------------------------------------------

bool_t CecTxMsgHandler ( SiiCpiData_t *pMsg )
{
    bool_t  isDirectAddressed, usedMessage;

    isDirectAddressed   = !((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );
    usedMessage         = false;

    switch ( pMsg->opcode )
    {
        case CECOP_ACTIVE_SOURCE:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
	            switch ((pMsg->srcDestAddr & 0xF0) >> 4)
	            {
		            case CEC_LOGADDR_PLAYBACK1:
		            case CEC_LOGADDR_PLAYBACK2:
		            case CEC_LOGADDR_PLAYBACK3:
					//case CEC_LOGADDR_TV:
					pCec->isActiveSource = false;
					//DEBUG_PRINT(MSG_ALWAYS,"used Message in ACTIVE SOURCE\n");
		           	break;
		            default:break;
	            }
            }
            else
	            usedMessage = true;
            break;

        case CECOP_REQUEST_ACTIVE_SOURCE:
            if ( !isDirectAddressed )                   // Ignore as direct message
            {
                if ( pCec->isActiveSource )
                {
                    SiiCecSendActiveSource( pCec->logicalAddr, pCec->physicalAddr );
                }
            }
            usedMessage = true;
            break;        

	case CECOP_USER_CONTROL_PRESSED:
            if ( isDirectAddressed )                   // Ignore as broadcast message
            {
                if ( pMsg->args[0] == CEC_RC_POWER )
                {
#if ( configSII_DEV_953x_PORTING == 0 )
	                pCec->powerState    = CEC_POWERSTATUS_STANDBY_TO_ON;
	                pCec->statusFlags   |= SiiCEC_POWERSTATE_CHANGE;            // Signal upper layer
#endif	                
			AmTCecEventPassToHMI(USER_EVENT_CEC_POWER_ON);	//Power On sound bar
					
		        usedMessage = true;
                }
            }
            break;        

        default:
            break;
    }

    return( usedMessage );
}
