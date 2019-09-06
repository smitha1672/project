//***************************************************************************
//!file     si_cbus_component.h
//!brief    Silicon Image CBUS Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CBUS_COMPONENT_H__
#define __SI_CBUS_COMPONENT_H__

#include "si_common.h"
#include "si_cbus_enums.h"
#include "si_osal_timer.h"
#include "si_drv_cbus.h"
#include "si_cbus_config.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------

#define 	CBUS_MAX_COMMAND_QUEUE      12


typedef struct
{
    uint8_t rcpKeyCode;         // RCP CBUS KeyID Code
    uint8_t supportMask;        // Bitmap of logical devices required to support this key
} SiiRcpValidate_t;

//------------------------------------------------------------------------------
//  Basic Component functions
//------------------------------------------------------------------------------

bool_t  SiiMhlRxInitialize( void );
void    SiiMhlRxChannelEnable( bool_t isEnable );
uint8_t SiiMhlRxHandler( void );
bool_t  SiiMhlRxWritePeerScratchpad( uint8_t startOffset, uint8_t length, uint8_t* pMsgData );
bool_t  SiiMhlRxReadDevCapReg( uint8_t regOffset );
bool_t  SiiMhlRxPathEnable( bool_t enable );
bool_t  SiiMhlRxSendDevCapChange( void );
bool_t  SiiMhlRxSendEdidChange( void );
bool_t  SiiMhlRxHpdSet( bool_t setHpd );
bool_t  SiiMhlRxSendRCPCmd ( uint8_t keyCode );


bool_t  SiiMhlRxSendRAPCmd( uint8_t actCode );
bool_t  SiiMhlRxSendRapk( uint8_t cmdStatus );
bool_t  SiiMhlRxSendMsge( void );

bool_t  SiiMhlRxCbusConnected( void );
bool_t  SiiMhlRxIsQueueFull( void );

//------------------------------------------------------------------------------
//  Extended Component functions
//------------------------------------------------------------------------------

void    SiiCbusConfigure ( uint8_t port, uint8_t supportMask );
bool_t  SiiCbusInstanceSet ( uint_t instanceIndex );
void    SiiCbusInstanceSetBasedOnPort ( uint_t portIndex );
uint16_t SiiCbusStatus ( void );
bool_t  SiiCbusStandby ( void );
bool_t  SiiCbusResume ( void );

bool_t  SiiMhlRxSendUCPCmd ( uint8_t asciiCode );
bool_t  SiiMhlRxSendUcpk ( uint8_t asciiCode);
bool_t  SiiMhlRxSendUcpe ( uint8_t cmdStatus );
bool_t  SiiMhlRxIsQueueEmpty ( void );
bool_t  SiiMhlRxGetMscMsgCmd( uint8_t* cmd, uint8_t* data );

bool_t  SiiMhlValidateRcpKeyCode( uint8_t keyData, uint8_t supportMask );

//------------------------------------------------------------------------------
// Callbacks
//------------------------------------------------------------------------------

void    SiiMhlCbRxConnectChange (bool_t connected);
void    SiiMhlCbRxScratchpadWritten();
uint8_t SiiMhlCbRxRcpRapReceived( uint8_t cmd, uint8_t keyCode);
void    SiiMhlCbRxMscCmdResponseData  (uint8_t cmd, uint8_t data);

#endif // __SI_CBUS_COMPONENT_H__
