//***************************************************************************
//!file     si_drv_msw_internal.h
//!brief    SiI9535 Matrix Switch driver functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRVMSW_INTERNAL_H__
#define __SI_DRVMSW_INTERNAL_H__

#include "si_device_config.h"
#include "si_regs_pp953x.h"
#include "si_drv_switch.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// Video Input information Enums and manifest constants
//-------------------------------------------------------------------------------

enum
{
    SiiPACKET_HEADER_VSIF          = 0x81,
    SiiPACKET_HEADER_AVI           = 0x82,
    SiiPACKET_HEADER_SPDI          = 0x83,
    SiiPACKET_HEADER_AUD           = 0x84,
    SiiPACKET_HEADER_MPEG          = 0x85
};


typedef enum  //Packet for main or roving port
{
    SiiPACKET_MAIN = 0,   //AVI Main port
    SiiPACKET_ROVING,     //AVI Roving port
}SiiPacketChannel_t;


typedef enum  //Packets extracted, with headers defined specifically in related registers
{
    SiiPACKET_TYPE_AIF = 0,     //header 0x82
    SiiPACKET_TYPE_AVI,         //header defined in 0x273
    SiiPACKET_TYPE_VSIF         //header defined in 0x272
}SiiPacketType_t;

enum   //HDMI_Video_Format
{
    PACKET_FORMAT_NONE      = 0x00,
    PACKET_FORMAT_EXT_RES   = 0x01,
    PACKET_FORMAT_3D        = 0x02
};

enum  //3D_Structure
{
    PACKET_3D_FRAME_PACK    = 0x00,
    PACKET_3D_FIELD_ALT     = 0x01,
    PACKET_3D_LINE_ALT      = 0x02,
    PACKET_3D_SIDE_FULL     = 0x03,
    PACKET_3D_L_DEPTH       = 0x04,
    PACKET_3D_L_DEPTH_GFX   = 0x05,
    PACKET_3D_SIDE_HALF     = 0x08

};

enum   //3D_Ext_Data
{
    PACKET_3D_HORIZONTAL    = 0x00,
    PACKET_3D_QUINCUNX      = 0x04
};

enum
{
    PACKET_3D_OL_OR     = 0x00,
    PACKET_3D_OL_ER     = 0x01,
    PACKET_3D_EL_OR     = 0x10,
    PACKET_3D_EL_ER     = 0x11
};

#define PIPE_COUNT                  2   // MP & RP
#define PACKET_TYPE_COUNT           3   // AVI, AIF, VSI

//------------------------------------------------------------------------------
//  Switch Driver Instance Data
//------------------------------------------------------------------------------

typedef struct  _SwitchDrvInstanceData_t
{
    int             structVersion;
    uint_t          instanceIndex;
    int             lastResultCode;             // Contains the result of the last API function called
    SwitchStatusFlags_t statusFlags;

    int_t           hecPortIndex;
    int_t           mhdPortIndex;
    SiiPortType_t   portType[SII_INPUT_PORT_COUNT]; // Port type info for each port

    uint8_t         mpInfoFrameMask;                // Determines which infoframe types will be
                                                    // monitored for change on the main pipe.
    uint8_t         mpInfoFrameInterruptStatus;
    uint8_t         spInfoFrameInterruptStatus;

    bool_t      (*pHpdSetStateMhd)( uint_t, uint_t );   // MHD callback function pointer.
    bool_t      (*pHpdSetStateCbus)( uint_t, uint_t );  // CBUS callback function pointer.
    void        (*pCallbackStatusChange)( uint16_t );   // Status Change callback function pointer


    // Matrix Switch Instance-specific data.
    SiiSwitchSource_t   mswVideoSource;
    SiiSwitchSource_t   mswAudioZoneSource;
    bool_t              mswZoneEnabled;
    bool_t              zoneTracking;
}   SwitchDrvInstanceData_t;

//------------------------------------------------------------------------------
//  Switch Driver Common Data
//------------------------------------------------------------------------------

typedef struct  _SwitchDrvCommonData_t
{
    uint8_t         userPortSelect;
    bool_t          mpCableIn;
    uint8_t         rPwrIntrPorts;

} SwitchDrvCommonData_t;

typedef struct _SiiMswRegisterConfData_t
{
    uint8_t     tmdsPortSelect;         // Register 0xB0:0A
    uint8_t     sourceControl;          // Register 0xB0:50
}   SiiMswRegisterConfData_t;

extern SwitchDrvInstanceData_t drvSwitchInstance[SII_NUM_SWITCH];
extern SwitchDrvInstanceData_t *pDrvSwitch;

extern SiiMswRegisterConfData_t   drvSwitchRegisterConf;
//-------------------------------------------------------------------------------
//  Driver Function Prototypes
//-------------------------------------------------------------------------------

bool_t  SiiDrvSwitchHpdSetState( uint_t portIndex, uint_t newState );
void    SiiDrvSwitchProcessInterrupts( uint8_t globalIntStatus );

//------------------------------------------------------------------------------
//  InfoFrame-Specific functions
//------------------------------------------------------------------------------

void    SwitchInfoframeProcessInterrupts( void );

#endif  // __SI_DRVMSW_INTERNAL_H__



