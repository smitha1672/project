//***************************************************************************
//!file     si_drv_cbus_internal.h
//!brief    Silicon Image CBUS Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_CBUS_INTERNAL_H__
#define __SI_DRV_CBUS_INTERNAL_H__
#include "si_device_config.h"
#include "si_drv_cbus.h"
#include "si_regs_mhl953x.h"

//------------------------------------------------------------------------------
// CBUS Component Manifest Constants
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// API typedefs
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// MHL Specs defined registers in device capability set
//
//
typedef struct {
	unsigned char	mhl_devcap_version;					// 0x00
	unsigned char	mhl_devcap_cbus_version;			// 0x01
	unsigned char	mhl_devcap_device_category;			// 0x02
	unsigned char	mhl_devcap_power_supply_capacity;	// 0x03
   	unsigned char	mhl_devcap_power_supply_provided;	// 0x04
   	unsigned char	mhl_devcap_video_link_mode_support;	// 0x05
   	unsigned char	mhl_devcap_audio_link_mode_support;	// 0x06
   	unsigned char	mhl_devcap_hdcp_status;				// 0x07
   	unsigned char	mhl_devcap_logical_device_map;		// 0x08
   	unsigned char	mhl_devcap_link_bandwidth_limit;	// 0x09
   	unsigned char	mhl_devcap_reserved_1;				// 0x0a
   	unsigned char	mhl_devcap_reserved_2;				// 0x0b
   	unsigned char	mhl_devcap_reserved_3;				// 0x0c
   	unsigned char	mhl_devcap_scratchpad_size;			// 0x0d
   	unsigned char	mhl_devcap_interrupt_size;			// 0x0e
   	unsigned char	mhl_devcap_devcap_size;				// 0x0f

} mhl_devcap_t;
//------------------------------------------------------------------------------
//
// MHL Specs defined registers for interrupts
//
//
typedef struct {

	unsigned char	mhl_intr_0;		// 0x00
	unsigned char	mhl_intr_1;		// 0x01
	unsigned char	mhl_intr_2;		// 0x02
	unsigned char	mhl_intr_3;		// 0x03

} mhl_interrupt_t;
//------------------------------------------------------------------------------
//
// MHL Specs defined registers for status
//
//
typedef struct {

	unsigned char	mhl_status_0;	// 0x00
	unsigned char	mhl_status_1;	// 0x01
	unsigned char	mhl_status_2;	// 0x02
	unsigned char	mhl_status_3;	// 0x03

} mhl_status_t;
//------------------------------------------------------------------------------
//
// MHL Specs defined registers for local scratchpad registers
//
//
typedef struct {

	unsigned char	mhl_scratchpad[16];

} mhl_scratchpad_t;

//------------------------------------------------------------------------------
//  CBUS Component Data
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  CBUS Driver Instance Data
//------------------------------------------------------------------------------

typedef struct
{
    int         structVersion;
    int         instanceIndex;
    int         lastResultCode;              // Contains the result of the last API function called
    uint16_t    statusFlags;
    uint16_t    statusWakeFlags;

    // CBUS transfer values read at last interrupt for each specific channel
    uint8_t     port;
    uint8_t     interruptStatus0;
    uint8_t     interruptStatus1;
    uint8_t     intWakeStatus;
    uint8_t     busConnected;
    uint8_t     vsCmd;
    uint8_t     vsData;
    uint8_t     msgData0;
    uint8_t     msgData1;
    uint8_t     cecAbortReason;
    uint8_t     ddcAbortReason;
    uint8_t     mscAbortFmPeerReason;
    uint8_t     mscAbortReason;
}	CbusDrvInstanceData_t;

extern CbusDrvInstanceData_t cbusDrvInstance[SII_NUM_CBUS];
extern CbusDrvInstanceData_t *pDrvCbus;

//------------------------------------------------------------------------------
//  Driver internal functions
//------------------------------------------------------------------------------

void DrvCbusProcesssWakeInterrupts();

#endif // __SI_DRV_CBUS_INTERNAL_H__
