//***************************************************************************
//!file     si_drv_cpi.h
//!brief    Silicon Image CPI driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CPI_DRV_H__
#define __SI_CPI_DRV_H__
#include "si_common.h"
#include "si_drv_cec_config.h"
#include "si_cec_enums.h"

#define SII_NUM_CPI                 1

//-------------------------------------------------------------------------------
// CPI Enums and manifest constants
//-------------------------------------------------------------------------------

#define CPI_MSG_DBG             (MSG_DBG | DBGF_TS | DBGF_CN), DBG_CPI
#define CPI_MSG_STAT            (MSG_STAT | DBGF_CN), DBG_CPI
#define CPI_MSG_ALWAYS          (MSG_ALWAYS | DBGF_CN), DBG_CPI

#define SII_MAX_CMD_SIZE 16

typedef enum _SiiDrvCpiError_t
{
    RESULT_CPI_SUCCESS,             // Success result code
    RESULT_CPI_FAIL,                // General Failure result code
    RESULT_CPI_WRITE_QUEUE_FULL,    // CPI Write Queue was full
} SiiDrvCpiError_t;


typedef enum _SiiCpiTxState_t
{
    SiiTX_IDLE          = 0,
    SiiTX_WAITCMD,
    SiiTX_SENDING,
    SiiTX_SENDACKED,
    SiiTX_SENDFAILED,
    SiiTX_TIMEOUT
} SiiCpiTxState_t;

typedef enum _SiiDrvCpiStatus_t
{
    SiiCPI_CEC_INT          = 0x0001,
    SiiCPI_CEC_STATUS_VALID = 0x0002,
    SiiCPI_CEC_DISABLED     = 0x0004,
} SiiDrvCpiStatus_t;

//-------------------------------------------------------------------------------
// CPI data structures
//-------------------------------------------------------------------------------

typedef struct _SiiCpiData_t
{
    uint8_t         srcDestAddr;            // Source in upper nibble, dest in lower nibble
    SiiCecOpcodes_t opcode;
    uint8_t         args[ SII_MAX_CMD_SIZE ];
    uint8_t         argCount;
    SiiCpiTxState_t txState;
    uint16_t        msgId;
} SiiCpiData_t;

//-------------------------------------------------------------------------------
// CPI data structures
//-------------------------------------------------------------------------------

typedef struct _SiiCpiStatus_t
{
    uint8_t         rxState;
    SiiCpiTxState_t txState;
    uint8_t         cecError;
    uint16_t        msgId;

} SiiCpiStatus_t;

//------------------------------------------------------------------------------
//  Standard driver functions
//------------------------------------------------------------------------------

bool_t      SiiDrvCpiInitialize( void );
uint16_t    SiiDrvCpiStatus( void );
bool_t      SiiDrvCpiStandby( void );
bool_t      SiiDrvCpiResume( void );
uint_t      SiiDrvCpiInstanceGet( void );
bool_t      SiiDrvCpiInstanceSet( uint_t instanceIndex );
uint_t      SiiDrvCpiGetLastResult( void );

bool_t      SiiCpiRegisterLogger ( void (*pLogger)(SiiCpiData_t *, int, bool_t ) );

void        SiiDrvCpiEnable( void );
void        SiiDrvCpiDisable( void );

//------------------------------------------------------------------------------
//  Driver Specific functions
//------------------------------------------------------------------------------

bool_t      SiiDrvCpiRead( SiiCpiData_t *pMsg );
uint16_t    SiiDrvCpiWrite( SiiCpiData_t *pMsg );
void        SiiDrvCpiServiceWriteQueue( void );
bool_t      SiiDrvCpiHwStatusGet( SiiCpiStatus_t *pCpiStat );
uint_t      SiiDrvCpiFrameCount( void );

bool_t      SiiDrvCpiSetLogicalAddr( uint8_t logicalAddress );
bool_t      SiiDrvCpiAddLogicalAddr( bool_t addLa, uint8_t logicalAddress );
void SiiDrvCpiClearVirtualDevBit(void);
uint8_t	    SiiDrvCpiGetLogicalAddr( void );
uint16_t    SiiDrvCpiSendPing( uint8_t destLa );

#endif // __SI_CPI_DRV_H__
