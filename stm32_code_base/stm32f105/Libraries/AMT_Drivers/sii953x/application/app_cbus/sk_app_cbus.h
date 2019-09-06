//***************************************************************************
//!file     sk_app_cbus.h
//!brief    Wraps board and device functions for the CBUS component
//          and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SKAPP_CBUS_H__
#define __SKAPP_CBUS_H__

#include "sk_application.h"
#include "si_cbus_component.h"
#include "si_drv_cbus.h"

//------------------------------------------------------------------------------
// Module variables
//------------------------------------------------------------------------------

#define SI_MHL_PORT0        0   // MHL Channel 0 port number on system board
#define SI_MHL_PORT1        1   // MHL Channel 1 port number on system board

#define CBUS_UCP_ASCII_LIMIT                0x7F


#define MHL_DEV1_OSD_NAME "SII_MHLDEV1"
#define MHL_DEV2_OSD_NAME "SII_MHLDEV2"


typedef struct _SiiAppCbusInstanceData_t
{
	int         		instanceIndex;
    bool_t              cbusInterrupt;              // CBUS interrupt flag
    bool_t              cbusEnable;                 // true == CBusEnable
    bool_t              oldCbusEnable;              // true == CBusEnable
    bool_t              cdSense;                 // true == CBusEnable
    bool_t              oldcdSense;              // true == CBusEnable
    bool_t              cbusCableSense;             // true == cbus cable detected.
    uint8_t             busConnected;
    bool_t				mhlCableIn;
    uint8_t             port;
    uint8_t             supportMask;                // LD_xx support bits

    uint8_t             lastRcpCecMessageOpcode;
    uint8_t             lastRcpCecMessageData;
    uint8_t             lastRcpCecSourceLa;
    bool_t              lastRcpFailed;

    // CEC stuff for CBUS device on this instance
    uint8_t             cecLa;
    uint16_t            cecPa;
    bool_t              cecWaitingForEnum;
    bool_t              cecIsActiveSource;

    SiiTimer_t        	cbusTimer;
    uint8_t             deckStatus;
    int8_t              osdName[14];
}   SiiAppCbusInstanceData_t;

extern SiiAppCbusInstanceData_t appCbus[MHL_MAX_CHANNELS];
extern SiiAppCbusInstanceData_t *pAppCbus;

bool_t  SkAppCbusInstanceSet( uint_t instanceIndex );

bool_t  SkAppDeviceInitCbus( void );
void    SkAppTaskCbus( void );
bool_t  SkAppCbusSendRapMessage ( uint8_t actCode );
bool_t  SkAppCbusSendUcpMessage ( uint8_t ucpData );
void    SkAppCbusChannelEnable( bool_t isEnable );
bool_t  SkAppCbusResume( void );
bool_t  SkAppCbusStandby ( void );

bool_t  SkAppCbusDemo( RC5Commands_t key );

void    SkAppCbusRapToCec( uint8_t rcpData );
void    SkAppCbusRapkToCec( uint8_t rapData );
void    SkAppCbusRcpToCec( uint8_t rcpData );
void    SkAppCbusSendRcKey( RC5Commands_t key );
#if INC_CEC
bool_t  SkAppCbusCecRxMsgFilter( SiiCpiData_t *pMsg,  SiiCecLogicalAddresses_t virtualDevLA );
void SiiCbusCecSendVendorId(uint8_t instanceIndex, uint8_t *vendorId );
#endif
void    SkAppCbusCecFeatureAbort( uint8_t msgData );
bool_t  SkAppSourceIsConnectedCbusPort( SiiSwitchSource_t source );
void    SkAppCbusAssignCecDeviceAddress( bool_t isComplete );
void    SkAppCbusConnectChangeCecUpdate( bool_t connected );

//-------------------------------------------------------------------------------
//  Cbus Wake Sequence start from here
//-------------------------------------------------------------------------------
bool_t  SkAppCbusWakeupSequence(uint8_t pwrState);
bool_t  SkAppDeviceInitCbusWakeup( void );
bool_t 	SkAppCbusIsMhlCableConnected ( uint8_t port );

#endif  //__SKAPP_CBUS_H__
