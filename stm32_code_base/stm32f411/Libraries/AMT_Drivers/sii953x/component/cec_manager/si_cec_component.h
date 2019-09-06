//***************************************************************************
//!file     si_cec_component.h
//!brief    Silicon Image CEC Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CEC_COMPONENT_H__
#define __SI_CEC_COMPONENT_H__
#include "si_common.h"
#include "si_cec_config.h"
#include "si_cec_timer.h"
#include "si_cec_enums.h"
#include "si_drv_cpi.h"

#include "Api_Typedef.h"
#include "HMI_Service.h"
#include "PowerHandler.h"


extern HMI_SERVICE_OBJECT *pHS_ObjCtrl; /*HMI service*/
extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
//-------------------------------------------------------------------------------
// CPI Enums, typedefs, and manifest constants
//-------------------------------------------------------------------------------

typedef enum _SiiDrvCecError_t
{
    RESULT_CEC_SUCCESS,             // Success result code
    RESULT_CEC_FAIL,                // General Failure result code

    RESULT_CEC_INVALID_PARAMETER,

    RESULT_CEC_INVALID_LOGICAL_ADDRESS,
    RESULT_CEC_INVALID_PHYSICAL_ADDRESS,
    RESULT_CEC_INVALID_PORT_INDEX,
    RESULT_CEC_NOT_ADJACENT,
    RESULT_CEC_NO_PA_FOUND,
    RESULT_CEC_TASK_QUEUE_FULL,
    RESULT_CEC_NO_LA_FOUND,

} SiiDrvCecError_t;


typedef enum
{
    SiiCEC_PORT_CHANGE          = 0x0001,
    SiiCEC_POWERSTATE_CHANGE    = 0x0002,
    SiiCEC_SOURCE_LOST          = 0x0004,
} SiiCecStatus_t;

#define MAKE_SRCDEST( src, dest )   ((( (src) << 4) & 0xF0) | ((dest) & 0x0F))
#define GET_CEC_SRC( srcDest )      (( srcDest >> 4) & 0x0F)
#define GET_CEC_DEST( srcDest )     (( srcDest >> 0) & 0x0F)

#define MAX_CEC_PARSERS             8   // Number of CEC message handlers that can
                                        // be registered with this instance.
#define MAX_CEC_TASKS               3   // Number of CEC Task handlers that can
                                        // be registered with this instance.
//------------------------------------------------------------------------------
//  Component functions
//------------------------------------------------------------------------------

int         SiiCecInstanceGet( void );
bool_t      SiiCecInstanceSet( int instanceIndex );

bool_t      SiiCecInitialize( uint16_t physicalAddress, SiiCecDeviceTypes_t deviceType );
bool_t      SiiCecConfigure( bool_t enable );

uint16_t    SiiCecStatus( void );
bool_t      SiiCecStandby( void );
bool_t      SiiCecResume( void );
bool_t      SiiCecGetLastResult( void );

int_t       SiiCecPortSelectGet( void );
void        SiiCecPortSelectSet( uint8_t inputPort);

uint16_t    SiiCecSendMessage( uint8_t opCode, uint8_t dest );                      // Deprecated
uint16_t    SiiCecSendMessageEx( uint8_t opCode, uint8_t src, uint8_t dest );
bool_t      SiiCecSendMessageTask( uint8_t opCode, uint8_t dest );                  // Deprecated
bool_t      SiiCecSendMessageTaskEx( uint8_t opCode, uint8_t src, uint8_t dest );
void        SiiCecSendUserControlPressed( SiiCecUiCommand_t keyCode, SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa );
void        SiiCecSendUserControlReleased( void );
uint16_t    SiiCecSendReportPhysicalAddress( SiiCecLogicalAddresses_t srcLa, uint16_t srcPa );
uint16_t SiiCecSendMenuStatus(SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa, uint8_t menuStatus );
uint16_t SiiCecSendDeckStatus( SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa, uint8_t deckStatus  );
uint16_t    SiiCecSendReportPowerStatus( SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa, uint8_t powerState );
uint16_t SiiCecSendVendorId(  SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa,uint8_t *vendorId );
bool_t      SiiCecValidateKeyCode( uint8_t keyData );

uint8_t     SiiCecGetAvailableLa( uint8_t *pDeviceLaList );
bool_t      SiiCecSetOsdName( char *pOsdName );

void        SiiCecHandler( void );
uint8_t     SiiCecGetPowerState( void );
void        SiiCecSetPowerState( SiiCecPowerstatus_t newPowerState );

uint8_t     SiiCecGetDeviceLA( void );
void        SiiCecSetDeviceLA ( SiiCecLogicalAddresses_t logicalAddr );
uint16_t    SiiCecGetDevicePA( void );
void        SiiCecSetDevicePA( uint16_t devPa );
SiiCecDeviceTypes_t SiiCecGetDeviceType ( void );
void        SiiCecSetActiveSource( SiiCecLogicalAddresses_t activeSrcLA, uint16_t activeSrcPA );
void        SiiCecSetSourceActive(bool_t);
void        SiiCecSetActiveSourceLA( SiiCecLogicalAddresses_t activeSrcLA);
void        SiiCecSetActiveSourcePA( uint16_t activeSrcPa );
SiiCecLogicalAddresses_t SiiCecGetActiveSourceLA( void );
uint16_t    SiiCecGetActiveSourcePA( void );

bool_t      SiiCecCallbackRegisterParser( bool_t (*pCallBack)(), bool_t callAlways );
bool_t SiiCecCallbackRegisterSet ( bool_t (*pCallBack)(), bool_t callAlways );
int_t       SiiCecCallbackRegisterTask( bool_t (*pCallBack)() );

void        SiiCecFeatureAbortSend(uint8_t opCode, uint8_t reason, uint8_t destLogAddr);
void        SiiCecCpiWrite(uint8_t opCode, uint8_t argCount, uint8_t destLogAddr, SiiCpiData_t *pCecMsg);
void        SiiCecFeatureAbortSendEx(
                uint8_t opCode, uint8_t reason,
                SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa );
void        SiiCecCpiWriteEx(
                uint8_t opCode, uint8_t argCount,
                SiiCecLogicalAddresses_t srcLa, SiiCecLogicalAddresses_t destLa,
                SiiCpiData_t *pCecMsg );

void SiiCecSetVirtualDevLA(uint8_t index, SiiCecLogicalAddresses_t virtualLA);
bool_t SiiCecRemoveVirtualDevice(SiiCecLogicalAddresses_t virtualLA);
bool_t SiiCecIsVirtualDeviceLA(SiiCecLogicalAddresses_t cecLA);

#if ( configSII_DEV_953x_PORTING == 1 )
void AmTCecEventPassToHMI(uint8_t event);
xHMISystemParams AmTCecGetSyspramsFromHMI(void);
POWER_STATE AmTCecGetSystemPowerStatus(void);
void AmTCecEventVolumeCtrl_HMI(  xHMISrvEventParams event ) ;

#endif

//------------------------------------------------------------------------------
// Enumeration Functions
//------------------------------------------------------------------------------

bool_t      SiiCecEnumerateDevices( uint8_t *pDeviceList );
bool_t      SiiCecEnumerateDeviceLa( uint8_t *pDeviceList );
void        SiiCecEnumerateCancel( void );
bool_t      SiiCecEnumerateIsComplete( void );
void        SiiCecUpdateLogicalDeviceInfo( uint8_t newLA, uint16_t newPA, bool_t isActive );

//------------------------------------------------------------------------------
// HDMI RX-specific API Function Templates
//------------------------------------------------------------------------------

bool_t      SiiCecSwitchSources( uint8_t portIndex );
uint16_t    SiiCecPortToPA( uint8_t portIndex );
uint8_t     SiiCecPortToLA( uint8_t portIndex );
int_t       SiiCecLaToPort( uint8_t logicalAddr );
bool_t      SiiCecIsAdjacentLA( uint8_t logicalAddr );
uint8_t     SiiCecPortToAdjacentLA( int_t portIndex );
bool_t      SiiCecPortToActSrcSelect(void);
#if ( configSII_DEV_953x_PORTING == 1 )
bool_t      AmTCecPortToActSrcSelect(void);
#endif
bool_t      SiiCecDeviceLaIsAvailable( uint8_t deviceLa );

//------------------------------------------------------------------------------
// HDMI TX-specific API Function Templates
//------------------------------------------------------------------------------

bool_t      SiiCecOneTouchPlay( void );
void        SiiCecSendActiveSource( uint8_t logicalAddr, uint16_t physicalAddr );
void        SiiCecSendInactiveSource( uint8_t logicalAddr, uint16_t physicalAddr );

//------------------------------------------------------------------------------
// HDMI Repeater-specific CEC API Function Templates
//------------------------------------------------------------------------------

uint16_t    SiiCecNextInvocationTimeGet(uint16_t sysTimerCountMs, CecTimeCounter_t recallTimerCount);
uint16_t    SiiCecAdjacentPhysAddrGet(const uint16_t basePhysAddr, const bool_t isAtOutput, const uint8_t inputPortIndex);

//------------------------------------------------------------------------------
// HDMI CEC Callback API Functions
//------------------------------------------------------------------------------

bool_t      SiiCecCbInputPortSet( uint8_t inputPortIndex);
void        SiiCecCbSendMessage( uint8_t opCode, bool_t messageAcked );
void        SiiCecCbEnumerateComplete( bool_t isComplete );

#endif // __SI_CEC_COMPONENT_H__
