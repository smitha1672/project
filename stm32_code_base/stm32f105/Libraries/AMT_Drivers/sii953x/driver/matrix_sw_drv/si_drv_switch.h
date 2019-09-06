//***************************************************************************
//!file     si_drv_switch.h
//!brief    SiI9535 Port Switch driver functions.
//!brief    Maintains the same filename as the base switch driver header
//          so that it is backwards compatible with components that may include
//          the switch driver header file.
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRVSWITCH_H__
#define __SI_DRVSWITCH_H__
#include "si_common.h"
#include "si_video_tables.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------

typedef enum _SiiDrvSwitchResults_t
{
    SII_DRV_SWITCH_SUCCESS      = 0,            // Success.
    SII_DRV_SWITCH_ERR_FAIL,                    // General failure.
    SII_DRV_SWITCH_ERR_INVALID_PARAMETER,       //
    SII_DRV_SWITCH_ERR_IN_USE,                  // Module already initialized.
    SII_DRV_SWITCH_ERR_NOT_AVAIL,               // Allocation of resources failed.

    SII_DRV_SWITCH_WRONG_ZONE_MODE  = 0x1000,   // Operation not valid for current zone mode
} SiiDrvSwitchResults_t;

enum
{
    SiiPORT_0   = 0,
    SiiPORT_1   = 1,
    SiiPORT_2   = 2,
    SiiPORT_3   = 3,

    SiiPORT_Pipe0   = 0x80,
    SiiPORT_Pipe1   = 0x81,
    SiiPORT_ALL = 0xFF
};
// SiiDrvSwitchSelectSource() parameter values
typedef enum _SiiSwitchSource_t
{
    SiiSwitchSource0                = 0,
    SiiSwitchSource1,
    SiiSwitchSource2,
#if ( configSII_DEV_9535 == 1 )
    SiiSwitchSource3,
#endif
    SiiSwitchSourceInternalVideo,

    SiiSwitchSourceCount
} SiiSwitchSource_t;

typedef enum _SiiSwitchAudioSource_t
{
    SiiSwitchAudioSourceDefault    = 0, // Default for the Video Source type (ex.:HDMI for HDMI src, SPDIF/I2S for IV/PV)
    SiiSwitchAudioSourceMainPipe,       // Audio overlay comes from main pipe
    SiiSwitchAudioSourceSubPipe,        // Audio overlay comes from sub pipe
    SiiSwitchAudioSourceExternal,       // Audio overlay comes from external source (SPDIF, I2S,...)

    SiiSwitchAudioSourceCount
} SiiSwitchAudioSource_t;

typedef enum _SiiPortType_t
{
    SiiPortType_HDMI,               // Port is in HDMI mode
    SiiPortType_MHL,                // Port is in MHL mode
    SiiPortType_HEC,                // Port is in HEC mode
} SiiPortType_t;

typedef enum _SiiSwitchZoneEnables_t
{
    SiiSwitchZoneNone   = 0x00,
    SiiSwitchZone1Enabled      = 0x01,     // Zone 1 output enabled
    SiiSwitchZone2Enabled      = 0x02,     // Zone 2 output enabled
    SiiSwitchZoneBoth   = ( SiiSwitchZone1Enabled | SiiSwitchZone2Enabled ),
} SiiSwitchZoneEnables_t;

typedef enum _SiiSwitchHdmiPipe_t
{
    SiiSwitchHdmiPipeMain                ,//= VAL_MUX_SEL_HDMI_MP, //Tx inputs from HDMI main pipe
    SiiSwitchHdmiPipeSub                 ,//= VAL_MUX_SEL_HDMI_RP,     //Tx inputs from HDMI sub pipe
    SiiSwitchHdmiPipeNone                //= VAL_MUX_SEL_PARALLEL     //Tx inputs from non-HDMI sources, or disabled
} SiiSwitchHdmiPipe_t;

typedef struct _SiiMswInstanceCommonData_t
{
	SiiSwitchHdmiPipe_t       pipeSel[2];             // zone input pipe selection
}   SiiMswInstanceCommonData_t;

// DEBUG_PRINT flags macros (NOTE: represents TWO parameters)

#define SW_ERR  (MSG_ERR | DBGF_TS | DBGF_CN), DBG_SWCH
#define SW_DBG  (MSG_DBG | DBGF_TS | DBGF_CN), DBG_SWCH

//-------------------------------------------------------------------------------
// InfoFrame PACKET data structures
//-------------------------------------------------------------------------------

#define PACKET_MAX_LENGTH           28 //27 + 1 checksum byte

typedef struct
{
    bool_t      packetNew;          // Last status indicated a new packet change
    bool_t      packetMissing;      // Last status indicated packets are missing
    uint8_t     packetType;
    uint8_t     packetVersion;
    uint8_t     packetLength;
    uint8_t     packetContent[ PACKET_MAX_LENGTH ];
} SiiPacketData_t;

enum   // HW extracted color depth value to the register
{
    COLORDEPTH_8  = 4,
    COLORDEPTH_10 = 5,
    COLORDEPTH_12 = 6,
    COLORDEPTH_16 = 7
};

typedef enum  _SiiSwitchInfoFrame_t // HW extracted info frame types
{
    IF_AVI  = 0,
    IF_AIF  = 1,
    IF_VSI  = 2
} SiiSwitchInfoFrame_t;



//-------------------------------------------------------------------------------
//  Driver Function Prototypes
//-------------------------------------------------------------------------------

bool_t  SiiDrvSwitchInitialize( void );
bool_t  SiiDrvSwitchInstanceSet( uint_t instanceIndex );
int_t   SiiDrvSwitchInstanceGet( void );

typedef enum _SwitchStatusFlags_t
{
    SiiSWITCH_PEEK_STATUS           = 0x0000,       // Returns all flags without clearing any
    SiiSWITCH_INT                   = 0x0001,       // At least one of SiiSWITCH_INTFLAGS is set
    SiiSWITCH_FIRST_PASS            = 0x0002,
    SiiSWITCH_PORT_CHANGE           = 0x0004,       // One or more port status changed
    SiiSWITCH_CABLE_CHANGE          = 0x0008,       // Main port cable plug in/out state changed
    SiiSWITCH_HPE_COMPLETE          = 0x0010,
    SiiSWITCH_MP_HPE_TRIGGER        = 0x0020,
    SiiSWITCH_RP_HPE_TRIGGER        = 0x0040,
    SiiSWITCH_MP_MUTED              = 0x0080,

    SiiSWITCH_MP_INFOFRAME_INTR_VALID   = 0x0100,   // A new (updated) Infoframe has been detected OR
                                                    // an infoframe has NOT been sent for 4 periods.
    SiiSWITCH_SP_INFOFRAME_INTR_VALID   = 0x0200,   // A new (updated) Infoframe has been detected OR
    SiiSWITCH_INTFLAGS                  = SiiSWITCH_PORT_CHANGE | SiiSWITCH_CABLE_CHANGE |
                                          SiiSWITCH_HPE_COMPLETE | SiiSWITCH_MP_HPE_TRIGGER |
                                          SiiSWITCH_RP_HPE_TRIGGER,

    // Currently used by Repeater only, not included by above
    SiiSWITCH_SCDT_CHANGE               = 0x0400,   // SCDT changed state, see SiiSWITCH_SCDT_ACTIVE
    SiiSWITCH_SCDT_ACTIVE               = 0x0800,   // State of SiiSWITCH_SCDT_ACTIVE after SiiSWITCH_SCDT_CHANGE

} SwitchStatusFlags_t;

SwitchStatusFlags_t SiiDrvSwitchStatus( SwitchStatusFlags_t mask );
bool_t              SiiDrvSwitchStandby( void );
bool_t              SiiDrvSwitchResume( void );

typedef enum _ifInterruptStatusFlags
{
    SiiSwitch_NO_AIF    = 0x01,
    SiiSwitch_NO_AVI    = 0x02,
    SiiSwitch_NO_VSI    = 0x04,
    SiiSwitch_NEW_AIF   = 0x08,
    SiiSwitch_NEW_AVI   = 0x10,
    SiiSwitch_NEW_VSI   = 0x20
} SiiInfoframeInterruptStatusFlags_t;

bool_t  SiiDrvSwitchIfInterruptStatusGet ( uint8_t *pMpIntStatus, uint8_t *pSpIntStatus );

typedef enum _SiiDrvSwitchStateTypes_t
{
    SiiSwitch_SELECTED_INPUT_CONNECT_STATE,
    SiiSwitch_INPUT_PORTS_CONNECT_STATE,
    SiiSwitch_SELECTED_PORT,
    SiiSwitch_SELECTED_PORT_SUB,
#if INC_RTPI
    SiiSwitch_TX_VIDEO_SRC,
#endif
} SiiDrvSwitchStateTypes_t;

uint8_t SiiDrvSwitchStateGet( SiiDrvSwitchStateTypes_t stateType );
bool_t  SiiDrvSwitchPortInfoGet( uint8_t port, bool_t isMainPipe, int_t *pTmdsClk, int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced, bool_t *isHdmi );
bool_t  SiiDrvSwitchSelectedPortInfo( bool_t isMainPipe, int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced, bool_t *isHdmi );
void    SiiDrvSwitchVideoFmtParse( videoFormatId_t format, int_t *pHoriz, int_t *pVert, bool_t *pIsInterlaced );

SiiDrvSwitchResults_t   SiiDrvSwitchSourceSelect( SiiSwitchSource_t newSource);
SiiDrvSwitchResults_t   SiiDrvSwitchAudioZoneSourceSet ( SiiSwitchSource_t newAudioZoneSource );
SiiSwitchSource_t       SiiDrvSwitchSourceGet( void );
SiiSwitchSource_t       SiiDrvSwitchAudioZoneSourceGet ( void );
void                    SiiDrvSwitchOutputEnable( bool_t isEnable );
void                    SiiDrvSwitchZoneEnableSet( bool_t isEnable );
void                    SiiDrvSwitchConfigureMatrixSwitch ( bool_t forceNonMatrix, bool_t portSelectOnly );
void                    SiiDrvSwitchZoneConfigGet( SiiMswInstanceCommonData_t *pZoneInfo );
void                    SiiDrvSwitchZoneConfigSet ( void );
bool_t                  SiiDrvSwitchZoneEnableGet( void );
void                    SiiDrvSwitchZoneTrackingSet( bool_t isEnabled );
bool_t                  SiiDrvSwitchZoneTrackingGet( void );

SiiDrvSwitchResults_t   SiiDrvSwitchAudioSourceSelect ( SiiSwitchAudioSource_t newSource, SiiSwitchSource_t newPort);
bool_t                  SiiDrvSwitchMatrixModeEnableGet( void );
int_t                   SiiDrvSwitchTmdsClockGet( bool_t isMainPipe );

// Note that these enums match the values of the corresponding CdcHpdState_t enums
typedef enum _SiiDrvHpdActions_t
{
    SiiHPD_INACTIVE,        // HPD HI, HDCP, EDID, RX Term disabled
    SiiHPD_ACTIVE,          // HPD HI, HDCP, EDID, RX Term enabled
    SiiHPD_TOGGLE,          // All Toggle Off then On
    SiiHPD_INACTIVE_EDID,   // EDID disabled
    SiiHPD_ACTIVE_EDID,     // EDID enabled
    SiiHPD_TOGGLE_EDID,     // EDID Toggle Off then On
    SiiHPD_UNUSED,          // Unused to help match CDC values
} SiiDrvHpdActions_t;

bool_t SiiDrvRxHpeStateSet ( int_t portIndex, SiiDrvHpdActions_t newState );

void SiiDrvRxPipeHpeTrigger( int_t pipe, bool_t includeEdid );
void SiiDrvSwitchDeviceRXTermControl( uint8_t portIndex, uint8_t enableVal );
void SiiDrvSwitchDeviceHpdControl( uint8_t portIndex, bool_t enableHPD, uint8_t mode);

void SiiDrvRxEdidDdcControl( uint8_t portIndex, bool_t setEnable );
void SiiDrvRxHdcpDdcControl( uint8_t portIndex, bool_t setEnable );
void SiiDrvRxTermControl( int_t portIndex, bool_t setEnable );
void SiiDrvRxHpdControl( uint8_t portIndex, bool_t setActive );
bool_t SiiDrvRxHpdStateGet ( uint8_t portIndex );

void SiiDrvRxAutoHpeConfigure( int_t portIndex, SiiPortType_t portType );

//------------------------------------------------------------------------------
//  InfoFrame-Specific functions
//------------------------------------------------------------------------------

uint8_t SiiVsifPacketHasNewData(  uint8_t packetChn, uint8_t packetType );
void    SiiVsifPacketContentRead( uint8_t packetChn, uint8_t packetType, SiiPacketData_t *pData );
bool_t  SiiDrvSwitchInfoFrameGet(bool_t isMainPipe, SiiSwitchInfoFrame_t ifType, uint8_t *pInfoFrame, uint8_t *pLen);
#if INC_RTPI
uint8_t SiiDrvSwitchInfoFrameValidGet(bool_t isMainPipe);
#endif // INC_RTPI
void SiiDrvSwitchPortsProcessTermCtrl();
SiiPortType_t SiiDrvSwitchSelectedPorttype(void);

//------------------------------------------------------------------------------
// Function:    SiiMhlCbHpdSetState
// Description: This callback is used when the Receiver finishes with all its
//              initialization and ready for Transmitter to read
//              EDID/KSV FIFO/BCAPS ...
// Returns:     true: success
//              false: either port is not MHL or CBUS is not ready.
//------------------------------------------------------------------------------
bool_t SiiMhlCbHpdSetState ( int_t portIndex, SiiDrvHpdActions_t newState );
bool_t SiiDrvSwitchPictAspectRatioGet(void);
#endif  // __SI_DRVSWITCH_H__



