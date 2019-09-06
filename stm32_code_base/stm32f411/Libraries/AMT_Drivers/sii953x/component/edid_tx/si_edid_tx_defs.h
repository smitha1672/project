//************************************************************************************************
//! @file   si_edid_tx_defs.h
//! @brief  EDID Tx definitions
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#ifndef EDID_DEFS_H
#define EDID_DEFS_H

#define EDID_BLOCK_SIZE 128

// EDID structure definitions

#define EDID_BL0_ADR__HEADER						0x00
#define EDID_BL0_LEN__HEADER						8

#define EDID_BL0_ADR__MANUFACTURER_ID				0x08
#define EDID_BL0_LEN__MANUFACTURER_ID				2
#define EDID_BL0_ADR__PRODUCT_ID					0x0A
#define EDID_BL0_LEN__PRODUCT_ID					2
#define EDID_BL0_ADR__SHORT_SERIAL_NUMBER			0x0C
#define EDID_BL0_LEN__SHORT_SERIAL_NUMBER			4
#define EDID_BL0_ADR__MANUFACTURE_WEEK				0x10
#define EDID_BL0_ADR__MANUFACTURE_YEAR				0x11

#define EDID_BL0_ADR__VERSION						0x12
#define EDID_BL0_ADR__REVISION						0x13

#define EDID_BL0_ADR__BASIC_DISPLAY_PARAMS			0x14
#define EDID_BL0_LEN__BASIC_DISPLAY_PARAMS			5

#define EDID_BL0_ADR__CHROMATICITY					0x19
#define EDID_BL0_LEN__CHROMATICITY					10

#define EDID_BL0_ADR__ESTABLISHED_TIMING_1			0x23
#define EDID_BL0_ADR__ESTABLISHED_TIMING_2			0x24
#define EDID_BL0_ADR__MANUFACTURER_RESERVED_TIMING	0x25

#define EDID_BL0_ADR__STANDARD_TIMING				0x26
#define EDID_BL0_LEN__STANDARD_TIMING				16

#define EDID_BL0_ADR__DTD1							0x36
#define EDID_BL0_ADR__DTD2							0x48
#define EDID_BL0_ADR__DTD3							0x5A
#define EDID_BL0_ADR__DTD4							0x6C
#define EDID_LEN__DTD								18

#define EDID_BL0_ADR__EXTENSION_NMB					0x7E

#define EDID_ADR__CHECK_SUM							0x7F // common for any block


// Extension Tags
#define EDID_EXT_TAG__LCD_TIMINGS					0x01
#define EDID_EXT_TAG__ADDITIONAL_TIMINGS			0x02
#define EDID_EXT_TAG__EDID_2_0						0x20
#define EDID_EXT_TAG__COLOR_INFO_0					0x30
#define EDID_EXT_TAG__DVI_FEATURE					0x40
#define EDID_EXT_TAG__TOUCH_SCREEN					0x50
#define EDID_EXT_TAG__BLOCK_MAP						0xF0
#define EDID_EXT_TAG__BY_MANUFACTURER				0xFF


// types of alternative DTD structures
#define EDID_DTD_TAG__SN						0xFF // monitor serial number
#define EDID_DTD_TAG__STRING					0xFE // ASCII string
#define EDID_DTD_TAG__RANGE_LIMITS				0xFD // monitor range limits
#define EDID_DTD_TAG__NAME						0xFC // monitor name
#define EDID_DTD_TAG__COLOR_POINT				0xFB // color point
#define EDID_DTD_TAG__STI						0xFA // standard timing identifiers
#define EDID_DTD_ADR__DATA_TYPE_TAG				0x03 // offset in DTD structure where the tags are placed


// DTD structure when used for Timings
#define EDID_DTD_ADR__PIX_CLK_LOW				0x00
#define EDID_DTD_ADR__PIX_CLK_HIGH				0x01
#define EDID_DTD_ADR__H_ACTIVE_LOW				0x02
#define EDID_DTD_ADR__H_BLANK_LOW				0x03
#define EDID_DTD_ADR__H_HIGH					0x04
#define EDID_DTD_ADR__V_ACTIVE_LOW				0x05
#define EDID_DTD_ADR__V_BLANK_LOW				0x06
#define EDID_DTD_ADR__V_HIGH					0x07
#define EDID_DTD_ADR__H_SYNC_OFFSET_LOW			0x08
#define EDID_DTD_ADR__H_SYNC_WIDTH_LOW			0x09
#define EDID_DTD_ADR__V_SYNC_LOW				0x0A
#define EDID_DTD_ADR__H_V_SYNC_HIGH				0x0B
#define EDID_DTD_ADR__H_SIZE_LOW				0x0C
#define EDID_DTD_ADR__V_SIZE_LOW				0x0D
#define EDID_DTD_ADR__H_V_SIZE_HIGH				0x0E
#define EDID_DTD_ADR__H_BORDER					0x0F
#define EDID_DTD_ADR__V_BORDER					0x10
#define EDID_DTD_ADR__FLAGS						0x11

// Block 0 has alvays 4 DTDs, but extensions can have more. The end of DTD area
// is marked by 5 zeros.
/*
#define EDID_EXT_ADR__LAST_DTD_START (EDID_ADR__CHECK_SUM-5)
// This is a start address in extended block where last DTD at the block could be placed.
// Even a DTD size is 18 bytes, 5 bytes is the minimum length of a DTD structure.
// This shorten 5-bytes structure is used to mark the end of the DTD area.
// Note, if this type of DTD structure exist, all the bytes must be zeros.
*/

// Monitor Range Limits (MRL) structure - one of the alternative DTDs
#define EDID_MRL_ADR__V_MIN						0x05
#define EDID_MRL_ADR__V_MAX						0x06
#define EDID_MRL_ADR__H_MIN						0x07
#define EDID_MRL_ADR__H_MAX						0x08
#define EDID_MRL_ADR__PCLK_MAX					0x09


// extension block structure
#define EDID_EXT_ADR__TAG						0x00 // should be set to EDID_EXT_TAG__ADDITIONAL_TIMINGS
#define EDID_EXT_ADR__REVISION					0x01 // revision (at least 2)
#define EDID_EXT_ADR__D							0x02 // where DTD area starts
#define EDID_EXT_ADR__DTD_OPT					0x03 // total number of native DTDs & options
#define EDID_EXT_ADR__DBC						0x04 // Data Block Collection start address

// CEA data block collection tags
#define EDID_DBC_TAG__AUDIO						1
#define EDID_DBC_TAG__VIDEO						2
#define EDID_DBC_TAG__VENDOR_SPECIFIC			3
#define EDID_DBC_TAG__SPEAKER_ALLOCATION		4
#define EDID_DBC_TAG__VESA_DTC					5
#define EDID_DBC_TAG__USE_EXTENDED_TAG			7

// Extended tags for CEA data block collection
#define EDID_DBC_ETG__MISC_VIDEO_FIELDS			0
#define EDID_DBC_ETG__VENDOR_SPECIFIC_VIDEO		1
#define EDID_DBC_ETG__MISC_AUDIO_FIELDS			16
#define EDID_DBC_ETG__VENDOR_SPECIFIC_AUDIO		17

#define EDID_SAD__LENGTH						3 // size of a short audio descriptor block
#define EDID_SPK__LENGTH						3 // size of a speaker allocation block

//#define EDID_VSB__HDMI_SIGNATURE				0x000C03
#define EDID_VSB_DAT__HDMI_SIGNATURE			{0x03, 0x0C, 0x00}
#define EDID_VSB_LEN__HDMI_SIGNATURE			3
#define EDID_VSB_LEN__MIN_HDMI_VSDB				5
#define EDID_VSB_LEN__MAX_EXTRAS				7 // maximum number of extra fields in HDMI 1.3a
//#define EDID_VSB_LEN__MAX_HDMI_VSDB				(EDID_VSB_LEN__MIN_HDMI_VSDB + EDID_VSB_LEN__MAX_EXTRAS) // HDMI 1.3a
#define EDID_VSB_LEN__MAX_HDMI_VSDB				31 // HDMI 1.4: maximum that can fit into the data block

#define DTD_DESCRIPTOR_DATA_LENGTH	(18-5) // (18 is the DTD block size, -5 to exclude header)
#define DTD_DESCRIPTOR_DATA_START	5

#endif // EDID_DEFS_H
