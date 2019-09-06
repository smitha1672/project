//***************************************************************************
//!file     si_cdc_component.h
//!brief    Silicon Image CDC Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CDC_COMPONENT_H__
#define __SI_CDC_COMPONENT_H__

#include "si_cdc_config.h"
#include "si_drv_cpi.h"
#include "si_cec_component.h"
#include "si_cdc_enums.h"
#include "si_cdc_heac.h"

//------------------------------------------------------------------------------
// CDC Component Manifest Constants
//------------------------------------------------------------------------------

//! @defgroup CDC_FB_MSG    Feedback Messages
//! @{
#define CDC_FB_MSG_NONE                 0x0000  //!< default value: no messages
#define CDC_FB_MSG_HST_SEARCH_DONE      0x0001  //!< Hosts search task finished
#define CDC_FB_MSG_CONNECT_DONE         0x0002  //!< Connect task finished
#define CDC_FB_MSG_CONNECT_ADJ_DONE     0x0003  //!< Connect to Adjacent device task finished
#define CDC_FB_MSG_DISCONNECT_DONE      0x0004  //!< Disconnect task finished
#define CDC_FB_MSG_DISCONNECT_ALL_DONE  0x0005  //!< Disconnect All task finished
#define CDC_FB_MSG_DISCONNECT_LOST_DONE 0x0006  //!< Disconnect Lost Device task finished
#define CDC_FB_MSG_HPD_SIGNAL_DONE      0x0007  //!< HPD signaling task finished
#define CDC_FB_MSG_HPD_STATE_CHANGED    0x0008  //!< HPD state of a Source has been changed by Sink device
#define CDC_FB_MSG_HPD_CAPABILITY_CONF  0x0009  //!< HPD capability has been confirmed
#define CDC_FB_MSG_CAPABILITY_CHANGED   0x000a  //!< One of Devices in the HDMI network has changed its capability state
#define CDC_FB_MSG_ERROR                0x00E0  //!< General error occurred
#define CDC_FB_MSG_ERR_NONCDC_CMD       0x00E1  //!< Non CDC command received
#define CDC_FB_MSG_ERR_HPD_SIGNAL       0x00E2  //!< HDMI Source not responded
#define CDC_FB_MSG_ERR_HPD_CAP_FAILED   0x00E3  //!< Failed to verify HPD-over-CDC capability
//
//! @}


//------------------------------------------------------------------------------
//  Standard component functions
//------------------------------------------------------------------------------

bool_t    SiiCdcConfigure ( void );
bool_t    SiiCdcInitialize ( void );
bool_t    SiiCdcStatus ( void );
bool_t    SiiCdcStandby ( void );
bool_t    SiiCdcResume ( void );
void      SiiCdcInstanceSet ( uint16_t deviceIndex );
bool_t    SiiCdcGetLastResult ( void );

//------------------------------------------------------------------------------
//  Component Specific functions
//------------------------------------------------------------------------------

void     SiiCdcConfig(uint16_t physAddr, CdcIoConfig_t *pIoPorts,
                      bool_t hecCapability, bool_t hostCapability, bool_t extNetCapability);
void     SiiCdcDeviceHecSupEnable(const bool_t isEnabled);
void     SiiCdcDeviceHostStateSet(const CdcHostFuncState_t state);
void     SiiCdcDeviceEncStateSet(const CdcEncFuncState_t state);
void     SiiCdcPortHecSupEnable(const bool_t isOutput, const uint8_t inputPortIndex, const bool_t isEnabled);

uint8_t  SiiCdcSrvHostsFind(void);
uint16_t SI_CdcNearestEncDeviceGet(void);
bool_t   SiiCdcDeviceConnect(bool_t isConnectEntireList, uint16_t firstPhysAddr, uint16_t lastPhysAddr);
bool_t   SiiCdcDeviceAdjacentConnect(uint16_t adjacentPhysAddr);
bool_t   SiiCdcDeviceDisconnect(bool_t isDisconnectAll, uint16_t actPhysAddr, uint16_t firstPhysAddr, uint16_t lastPhysAddr);
bool_t   SiiCdcDeviceAdjacentDisconnect(uint16_t adjacentPhysAddr);

bool_t   SiiCdcSrvStart(void);
void     SiiCdcSrvStop(void);

bool_t   SiiCdcPortIsActive(const bool_t isOutput, const uint8_t portIndex);
bool_t   SiiCdcHpdSetState(uint8_t inputPortIndex, CdcHpdState_t hpdState);
bool_t   SiiCdcHpdCapabilityInquire(uint8_t inputPortIndex);
CdcHpdState_t SiiCdcHpdStateGet(void);

void     SiiCdcTaskInit();
uint8_t  SiiCdcTaskProcess(uint16_t sysTimerCountMs, SiiCpiData_t *pCdcMessage);
uint16_t SiiCdcNextInvocationTimeGet(uint16_t sysTimerCountMs);

// CDC data getters
uint16_t              SiiCdcPhysAddrGet(void);
CdcIoConfig_t *       SiiCdcIoPortsGet(void);
CdcHecDescriptor_t *  SiiCdcHecRecordsGet(void);
uint8_t               SiiCdcNumberOfHecsGet(void);
CdcDeviceStatus_t *   SiiCdcDeviceStatusGet(void);
CdcFoundListEntry_t * SiiCdcFoundHostListGet(void);
uint8_t               SiiCdcNumberOfFoundHostsGet(void);
CdcPhysAddrTime_t *   SiiCdcAliveDeviceListGet(void);
uint8_t               SiiCdcNumberOfAliveDevGet(void);


#endif // __SI_CDC_COMPONENT_H__
