//------------------------------------------------------------------------------
// Project: HDMI Repeater
// Copyright (C) 2002-2009, Silicon Image, Inc.  All rights reserved.
//
// No part of this work may be reproduced, modified, distributed, transmitted,
// transcribed, or translated into any language or computer format, in any form
// or by any means without written permission of: Silicon Image, Inc.,
// 1060 East Arques Avenue, Sunnyvale, California 94085
//------------------------------------------------------------------------------

#ifndef SIICFGEEPROM_H
#define SIICFGEEPROM_H


// Legend:
// NV_  never changes from within the same product (places into ROM or Internal EEPROM)
// EE_  changes on every device sample (any EEPROM)
// RP_  changes during run-time execution (preset from any EEPROM (i.e. EE_) Slave Register)


#define EE_A__ID		0x00
#define EE_L__ID		5
#define EE_A__CFG		EE_A__ID


// ------------------------------------------------------------------------------

#define NV_A__VIDEO_MIN_V_HZ			0x28
#define NV_A__VIDEO_MAX_V_HZ			0x29
#define NV_A__VIDEO_MIN_H_KHZ			0x2A
#define NV_A__VIDEO_MAX_H_KHZ			0x2B
#define NV_A__VIDEO_MAX_PIX_CLK_10MHZ	0x2C
#define NV_A__MAX_TMDS_CLK_5MHZ			0x2D
#define NV_A__SPK_ALLOC					0x2E

#define NV_A__VIDEO_LATENCY_LOC			0x30
#define NV_A__AUDIO_LATENCY_LOC			0x31
#define NV_A__I_VIDEO_LATENCY_LOC		0x32
#define NV_A__I_AUDIO_LATENCY_LOC		0x33
#define NV_A__VIDEO_LATENCY_BYP			0x34
#define NV_A__AUDIO_LATENCY_BYP			0x35
#define NV_A__I_VIDEO_LATENCY_BYP		0x36
#define NV_A__I_AUDIO_LATENCY_BYP		0x37

// SPD packet to be sent out
#define NV_A__SPD_VENDOR1				0x38
#define NV_A__SPD_VENDOR2				0x39
#define NV_A__SPD_VENDOR3				0x3A
#define NV_A__SPD_VENDOR4				0x3B
#define NV_A__SPD_VENDOR5				0x3C
#define NV_A__SPD_VENDOR6				0x3D
#define NV_A__SPD_VENDOR7				0x3E
#define NV_A__SPD_VENDOR8				0x3F
#define NV_A__SPD_PRODUCT1				0x40
#define NV_A__SPD_PRODUCT2				0x41
#define NV_A__SPD_PRODUCT3				0x42
#define NV_A__SPD_PRODUCT4				0x43
#define NV_A__SPD_PRODUCT5				0x44
#define NV_A__SPD_PRODUCT6				0x45
#define NV_A__SPD_PRODUCT7				0x46
#define NV_A__SPD_PRODUCT8				0x47
#define NV_A__SPD_PRODUCT9				0x48
#define NV_A__SPD_PRODUCT10				0x49
#define NV_A__SPD_PRODUCT11				0x4A
#define NV_A__SPD_PRODUCT12				0x4B
#define NV_A__SPD_PRODUCT13				0x4C
#define NV_A__SPD_PRODUCT14				0x4D
#define NV_A__SPD_PRODUCT15				0x4E
#define NV_A__SPD_PRODUCT16				0x4F
#define NV_A__SPD_SOURCE				0x50

// Register for intermal use only
#define EE_A__SPECIAL					0x51
#define EE_M__SPECIAL__NVM_PROGRAMMED		0x01 // set when RX Chip's NV ROM is valid

#define NV_A__PTRS_START				0x60
// starting from that address, each 6 bytes means following:
// bytes 0,1: pointer to a table
// bytes 2,3: size of the table
// bytes 4,5: size of the table element
// if bytes 0 and 1 are filled with 0 or 0xFF, the slot is empty

#define NV_A__DEFAULT_SAD_START				0x80 // Short Audio Descriptors
#define NV_A__DEFAULT_BLACK_LIST_START		0x100 // Black List Pointer
#define NV_A__DEFAULT_SCALER_TABLE_START	0x200 // Scaler Table

// ADC calibration Data
#define EE_A__VIDEO_ADC_CALIBR			0x300
#define EE_C_CALIBRATION_ID_1					0xAA
#define EE_C_CALIBRATION_ID_2					0x55
#define EE_L__VIDEO_ADC_CALIBRATION				12

// Phase data
#define EE_A__VIDEO_PHASE				0x340
#define EE_L__VIDEO_PHASE						3

#endif // SIICFGEEPROM_H
