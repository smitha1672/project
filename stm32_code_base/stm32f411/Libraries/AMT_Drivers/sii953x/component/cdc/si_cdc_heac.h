//***************************************************************************
//! @file     si_cdc_heac.h
//! @brief    Low level CDC operation API
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CDC_HEAC_H__
#define __SI_CDC_HEAC_H__

#include "si_cdc_enums.h"

//------------------------------------------------------------------------------
// Constant definitions
//------------------------------------------------------------------------------

#define CDC_NUMBER_OF_HECS_PAS_MAX      6   //!< Maximum number of HECs the device can participate passively
#define CDC_NUMBER_OF_HECS_ACT_MAX      3   //!< Maximum number of HECs where the device plays as an activator
#define CDC_NUMBER_OF_HECS_MAX          (CDC_NUMBER_OF_HECS_PAS_MAX + CDC_NUMBER_OF_HECS_ACT_MAX) //!< Maximum number of HECs (<256)

#define CDC_NUMBER_OF_NODES_MAX         3   //!< Maximum number of path nodes in an HEC not including terminators
#define CDC_EHDMI_INPUTS_MAX            5   //!< Maximum number of HDMI inputs per device

//! Maximum number of different physical addresses in all HECs
//! to be monitored by this device (as activator and passive node)
#define CDC_NUMBER_OF_DEVICES_MAX       (CDC_NUMBER_OF_HECS_PAS_MAX +\
                                         CDC_NUMBER_OF_HECS_ACT_MAX * (CDC_NUMBER_OF_NODES_MAX + 2))
                                         
#define CDC_NUMBER_OF_FOUND_HOSTS_MAX   CDC_NUMBER_OF_DEVICES_MAX   //!< Maximum number of devices to be found as a result of a search

#define CDC_CEC_OPCODE                  0xF8    //!< CEC opcode for all CDC messages
#define CDC_CEC_SRCDEST_ADDR_DEFAULT    0xFF    //!< Default (broadcast) CEC logical addresses used in CDC messages

// Time related definitions
#define CDC_TIME_TCK_MS                 4                               //!< number of milliseconds in one tick (definition of the "tick")
#define CDC_TIME_MS2TCK(t)              ((t) / CDC_TIME_TCK_MS)         //!< Converts time in Ms to number of counter ticks
#define CDC_TIME_SEC2TCK(t)             ((t) * (1000 / CDC_TIME_TCK_MS))//!< Converts time in seconds to number of counter ticks
#define CDC_TIME_TCK2SEC(t)             ((t) / (1000 / CDC_TIME_TCK_MS))//!< Converts time in number of counter ticks to seconds

#define CDC_MAX_NTF_ALIVE_TIME_SEC      140  //!< threshold time for a Watchdog (devices shall keep sending NotifyAlive messages within this time)


//------------------------------------------------------------------------------
// Data type definitions
//-------------------------------------------------------------------------------------------------
                        //! Time counter structure
typedef struct
{
    uint_t              tRov : 2;    //!< number system timer overflow events ( = log_2(CDC_TIME_TCK_MS))
    uint_t              tRes : 14;   //!< residual time count (in ticks)      ( = 16 - log_2(CDC_TIME_TCK_MS))
    
} CdcTimeCounter_t;

                        //! HEC record status
typedef struct
{
    uint_t              isInUse      : 1;      //!< true, if the HEC record is not vacant
    uint_t              isActivator  : 1;      //!< true, if the device is Activator of this HEC
    uint_t              isAdjacent   : 1;      //!< true, if AHEC is established by control to adjacent device
    uint_t              channelState : 2;      //!< channel state of the HEC (PHEC,AHEC,VHEC)   @see CdcHecChannelState_t
    
} CdcHecRecStatus_t;

                        //! HEC descriptor
typedef struct
{
    CdcHecRecStatus_t   status;               //!< HEC record status flags
    uint16_t            activatorPhysAddr;    //!< PA of the device activated this HEC
    uint16_t            firstDevPhysAddr;     //!< PA of first terminating device
    uint16_t            lastDevPhysAddr;      //!< PA of last terminating device
    
} CdcHecDescriptor_t;

                        //! Port (input/output) capability/status
typedef struct
{
    uint_t              hecSupport      : 1; //!< HEC support by the port flag                        @see CdcHecSupport_t
    uint_t              hecActive       : 1; //!< AHEC operation over the port flag                   @see CdcHecActivation_t
    uint_t              hpdState        : 3; //!< latest sent/received HPD state                      @see CdcHpdState_t
    uint_t              hpdErrorState   : 3; //!< CDC_HPD error code in case of failed HPD operation  @see CdcHpdErrCode_t
    
} CdcPort_t;

                        //! Descriptor of I/O ports configuration
typedef struct
{
    uint8_t             numberOfInputs;  //!< number of all HDMI inputs
    uint8_t             numberOfOutputs; //!< 0 (root) or 1
    CdcPort_t           input[CDC_EHDMI_INPUTS_MAX]; //!< Input ports capability
    CdcPort_t           output; //!< output port capability
    
} CdcIoConfig_t;

                        //! Bitfield reflecting states and capability of device
typedef struct
{
    uint_t              hecFuncState    : 2; //!< HEC functional capability/state                 @see CdcHecFuncState_t
    uint_t              hostFuncState   : 2; //!< host capability/state                           @see CdcHostFuncState_t
    uint_t              encFuncState    : 2; //!< external network connection capability/state    @see CdcEncFuncState_t
    uint_t              cdcErrorState   : 2; //!< CDC error code in case of failed operation      @see CdcErrCode_t
    
} CdcDeviceStatus_t;

                        //! Table entry for HEC Watchdog to monitor Alive status of devices
typedef struct
{
    uint16_t            physAddr;       //!< Physical Address of a Device
    CdcTimeCounter_t    TimeCount;      //!< Time counter to generate timeouts
    
} CdcPhysAddrTime_t;
    
                        //! Multi-HEC activation mode descriptor
typedef struct
{
    uint_t              isMultHecMode : 1; //!< Multi-HEC Activation mode flag
    uint_t              isEndOfList   : 1; //!< End of VHEC list to activate flag
    uint_t              hecActDelayed : 3; //!< number of delayed activation events (0-4)

} CdcMultHecActData_t;


                        //! cdcFoundHostsList[] entry for Discovery process
typedef struct
{
    uint16_t            physAddr;       //!< Physical Address of a Device
    CdcDeviceStatus_t   devStat;        //!< State and capabilities of the Device

    //! Device selection and HEC capability descriptor
    struct
    {
        uint_t          isSelected     :  1;//!< flag indicating that the Device has been selected (for sorting)
        uint_t          portHecCapBits : 15;//!< HEC capability flags of I/O ports
        
    } hostInfo;
    
} CdcFoundListEntry_t;

//------------------------------------------------------------------------------
// Low level function prototypes
//------------------------------------------------------------------------------

// Low level API functions

void     CdcHecDiscoverSend(void);
void     CdcHecInquireStateSend(const uint8_t hecIdx);
bool_t   CdcHecInquireStateOfAdjacentSend(const bool_t isAtOutput, const uint8_t inputPortIndex);
void     CdcHecReportStateSend(const uint16_t targetPhysAddr, const CdcReportStateMode_t reportStateMode);
void     CdcHecSetStateSend(const CdcHecActivation_t hecActivState, const uint8_t numberOfHecs, const uint8_t aHecIdx[]);
void     CdcHecSetStateAdjacentSend(const CdcHecActivation_t hecActivState, const uint8_t adjHecIdx);
void     CdcHecDeactivationRequestSend(const uint8_t hecIdx);
void     CdcHecNotifyAliveSend(void);

void     CdcHpdSetStateSend(const uint8_t inputPortIndex);
void     CdcHpdReportStateSend(void);


// Internal service functions

void     CdcHecInstanceRecordReset(void);
uint8_t  CdcHecDescriptorAdd(const bool_t isActivator);
bool_t   CdcHecDescriptorRemove(const uint8_t hecIdx);
uint8_t  CdcHecIndexGet(const uint16_t actPhysAddr, const uint16_t firstPhysAddr, const uint16_t lastPhysAddr);

bool_t   CdcAliveDeviceListAdd(const uint8_t hecIdx);
bool_t   CdcAliveDeviceListRemove(const uint8_t hecIdx);
bool_t   CdcAliveDeviceListTimeReset(const uint16_t physAddr);
uint16_t CdcAliveDeviceListTimeUpdate(void);

void     CdcTimeCounterReset(CdcTimeCounter_t *pTimeCounter);
void     CdcTimeCounterSet(const uint16_t delayTimeTck, CdcTimeCounter_t *pTimeCounter);
bool_t   CdcTimeCounterUpdate(CdcTimeCounter_t *pTimeCounter);
bool_t   CdcIsTimeCounterExpired(CdcTimeCounter_t *pTimeCounter);
uint16_t CdcTimeCounterLeftSecGet(CdcTimeCounter_t *pTimeCounter);
uint8_t  CdcTimeCounterCompare(CdcTimeCounter_t *pTimeCounterA, CdcTimeCounter_t *pTimeCounterB);


bool_t   CdcIsDeviceBelongsToHec(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, const uint16_t testPhysAddr);
uint16_t CdcHecNextDeviceGet(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, const bool_t restart);
uint8_t  CdcHecLengthGet(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr);
bool_t   CdcIsLinkedDirectly(const uint16_t sinkPhysAddr, const uint16_t sourcePhysAddr, const uint8_t inputPortIndex);
uint16_t CdcAdjacentPhysAddrGet(const uint16_t basePhysAddr, const bool_t isAtOutput, const uint8_t inputPortIndex);
bool_t   CdcIsCapableToAdjacent(const uint16_t initiatorPhysAddr, const uint16_t adjacentPhysAddr, bool_t *isAdjacent);

#if ( configSII_DEV_953x_PORTING == 1)
bool_t  CdcCapablePortGet(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, uint8_t aInPortIdx[2], uint8_t *pOutPortFlg);
#else
bool_t   CdcCapablePortGet(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, uint8_t aInPortIdx[2], bool_t *pOutPortFlg);
#endif 

bool_t   CdcIsCapableForHec(const uint16_t firstPhysAddr, const uint16_t lastPhysAddr, const bool_t isStrict);
void     CdcIoPortsActivate(const uint8_t aInPortIdx[2], const bool_t outPortFlg);
void     CdcIoPortsStatusUpdate(void);
bool_t   CdcIsHecEnabledInDev(const uint8_t hecIdx, const uint16_t devPhysAddr, const uint16_t portBitField); 
void     CdcDiscoveryDataArrange(void);

bool_t   CdcEhdmiHwEnable(const bool_t isEnabled);
bool_t   CdcEhdmiIoPortHwEnable(const bool_t isOutput, const uint8_t inPortIdx, const bool_t isEnabled);

uint32_t Mrand(uint32_t seed);



#endif //__SI_CDC_HEAC_H__
