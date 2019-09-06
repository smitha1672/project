//***************************************************************************
//!file     si_rtpi_internal.h
//!brief    Silicon Image CBUS Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_RTPI_INTERNAL_H__
#define __SI_RTPI_INTERNAL_H__
#include "si_device_config.h"
#include "si_rtpi_component.h"
#include "si_i2c.h"

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//  RTPI Component Instance Data
//------------------------------------------------------------------------------

typedef struct _SiiRtpiInstanceData_t
{
	uint8_t		lastErrorCode;
	uint8_t 	rtpiInstance;
	uint8_t		needReset;
	uint8_t		debugPrintMode;
	//uint8_t 	rtpi_regs[RP_L__TOTAL_REGS];

} SiiRtpiInstanceData_t;


typedef struct _SiiRtpiBusSetup_t
{
	uint8_t pr;
	uint8_t risingEdge;
	uint8_t busWidth;
	uint8_t iclk; // Changed from Rsvd to to ICLK in 9022A/24A
} SiiRtpiBusSetup_t;


typedef struct _SiiRtpiSystemControl_t
{
	uint8_t setHDMI;
	uint8_t ddcStat;
	uint8_t ddcReq;
	uint8_t avMuteEn;
	uint8_t powerDown;
	uint8_t rsvd1;
	uint8_t integrityMode;
	uint8_t rsvd2;
} SiiRtpiSystemControl_t;


extern SiiRtpiInstanceData_t    rtpiInstance[SII_NUM_RTPI];
extern SiiRtpiInstanceData_t    *pRtpi;

extern SiiRtpiBusSetup_t        siiTpiInputBusSetupVal;
extern SiiRtpiSystemControl_t   siiTpiSystemControlVal;

extern uint8_t rtpiRegs[RTPI_MAX_REGISTERS];   // RTPI Registers.
extern uint8_t rtpiWriteBuf[RTPI_CMD_BUFFER_SIZE][2];
extern uint8_t rtpiWrPtr;
extern uint8_t rtpiRdPtr;

//------------------------------------------------------------------------------
//  Component Specific functions
//------------------------------------------------------------------------------

void SiiRtpiLoadEepromInit( void );
void SiiRpReadBlock(uint8_t address, uint8_t *p_data, uint8_t length);
void SiiRpWriteBlock(uint8_t address, uint8_t *p_data, uint8_t length);

void SiiRpForceWriteByte(uint8_t address, uint8_t d);

uint8_t RtpiRegs0x18( uint8_t val );
uint8_t RtpiRegs0x1C( uint8_t val );
uint8_t RtpiRegs0x20( uint8_t val );
uint8_t RtpiRegs0x21( uint8_t val );
uint8_t RtpiRegs0x23( uint8_t val );
uint8_t RtpiRegs0x24( uint8_t val );
uint8_t RtpiRegs0x28( uint8_t val );
uint8_t RtpiRegs0x2B( uint8_t val );
uint8_t RtpiRegs0x2C( uint8_t val );
uint8_t RtpiRegs0x2D( uint8_t val );
uint8_t RtpiRegs0x30( uint8_t val );
uint8_t RtpiRegs0x33( uint8_t val );
uint8_t RtpiRegs0x34( uint8_t val );
uint8_t RtpiRegs0x35( uint8_t val );
uint8_t RtpiRegs0x36( uint8_t val );
uint8_t RtpiRegs0x37( uint8_t val );
uint8_t RtpiRegs0x38( uint8_t val );
uint8_t RtpiRegs0x6A( uint8_t val );
uint8_t RtpiRegs0x6B( uint8_t val );
uint8_t RtpiRegs0x6C( uint8_t val );
uint8_t RtpiRegs0x6D( uint8_t val );
uint8_t RtpiRegs0x6E( uint8_t val );
uint8_t RtpiRegs0x6F( uint8_t val );

uint8_t RtpiRegsDummy( uint8_t val );

bool_t  RtpiExecuteCbusCmd( uint8_t cmdType );
bool_t  RtpiExecuteCecCmd( uint8_t cmdType );


#endif // __SI_RTPI_INTERNAL_H__
