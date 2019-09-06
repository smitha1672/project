//***************************************************************************
//!file     si_device_edid.c
//!brief    SiI953x EDID stuff.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "device_config.h"
#include "si_drv_nvram_sram.h"
#include "si_edid_tx_component.h"

//------------------------------------------------------------------------------
//  Module Data
//------------------------------------------------------------------------------
/*
ROM const uint8_t gEdidFlashEdidVgaTable [ EDID_VGA_TABLE_LEN ] =
{
//  00    01    02   03     04    05    06    07   08     09    0A    0B    0C    0D    0E   0F
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x10, 0xAC, 0x12, 0x40, 0x51, 0x53, 0x44, 0x42,
    0x1C, 0x10, 0x01, 0x03, 0x0E, 0x22, 0x1B, 0x78, 0xEE, 0xE7, 0xE5, 0xA3, 0x59, 0x4A, 0x9E, 0x23,
    0x14, 0x50, 0x54, 0xA5, 0x4B, 0x00, 0x71, 0x4F, 0x81, 0x80, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x30, 0x2A, 0x00, 0x98, 0x51, 0x00, 0x2A, 0x40, 0x30, 0x70,
    0x13, 0x00, 0x52, 0x0E, 0x11, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x43, 0x43, 0x32,
    0x38, 0x30, 0x36, 0x37, 0x45, 0x42, 0x44, 0x53, 0x51, 0x0A, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x44,
    0x45, 0x4C, 0x4C, 0x20, 0x31, 0x37, 0x30, 0x37, 0x46, 0x50, 0x0A, 0x20, 0x00, 0x00, 0x00, 0xFD,
    0x00, 0x38, 0x4C, 0x1E, 0x51, 0x0E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x1D
};
*/

/* Basic Audio, with deep color.  */
const uint8_t gEdidFlashEdidTable [ EDID_TABLE_LEN ] =
{
#if ( configSII_DEV_953x_PORTING == 1 ) /*Smith fixes: EDID 8-3 fail: Removes VSDB 48 bits deep color*/

0x00,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0x00,  0x4D,  0x29,  0x33,  0x95,  0x01,  0x00,  0x00,  0x00,
0x20,  0x15,  0x01,  0x03,  0x80,  0x52,  0x2E,  0x78,  0x0A,  0x0D,  0xC9,  0xA0,  0x57,  0x47,  0x98,  0x27,
0x12,  0x48,  0x4C,  0x21,  0x09,  0x00,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,
0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x02,  0x3A,  0x80,  0x18,  0x71,  0x38,  0x2D,  0x40,  0x58,  0x2C,
0x45,  0x00,  0xC4,  0x8E,  0x21,  0x00,  0x00,  0x1A,  0x01,  0x1D,  0x00,  0x72,  0x51,  0xD0,  0x1E,  0x20,
0x6E,  0x28,  0x55,  0x00,  0xC4,  0x8E,  0x21,  0x00,  0x00,  0x1E,  0x00,  0x00,  0x00,  0xFC,  0x00,  0x43,
0x50,  0x39,  0x36,  0x38,  0x37,  0x0A,  0x20,  0x20,  0x20,  0x20,  0x20,  0x20,  0x00,  0x00,  0x00,  0xFD,
0x00,  0x17,  0x78,  0x0F,  0x85,  0x1E,  0x00,  0x0A,  0x20,  0x20,  0x20,  0x20,  0x20,  0x20,  0x01,  0x6E,
0x02 ,0x03 ,0x37 ,0x72 ,0x55 ,0x90 ,0x84 ,0x03 ,0x02 ,0x0E ,0x0F ,0x07 ,0x06 ,0x22 ,0x05 ,0x94,
0x13 ,0x12 ,0x11 ,0x1D ,0x1E ,0x16 ,0x15 ,0x20 ,0x01 ,0x1F ,0x26 ,0x09 ,0x7F ,0x07 ,0x15 ,0x07,
0x50,  0x83,  0x01,  0x00,  0x00,  0x6E,  0x03,  0x0C,  0x00,  0x10,  0x00,  0xB8,  0x3C,  0x21,  0x84,  0x80,
0x01,  0x02,  0x03,  0x04,  0xE2,  0x00,  0xFF,  0x8C,  0x0A,  0xD0,  0x90,  0x20,  0x40,  0x31,  0x20,  0x0C,
0x40,  0x55,  0x00,  0xC4,  0x8E,  0x21,  0x00,  0x00,  0x18,  0x01,  0x1D,  0x80,  0x18,  0x71,  0x1C,  0x16,
0x20,  0x58,  0x2C,  0x25,  0x00,  0xC4,  0x8E,  0x21,  0x00,  0x00,  0x00,  0x9E,  0x00,  0x00,  0x00,  0x00,
0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,
0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xE7

#else

// Bug 33154 - Melbourne SiI9533 EDID ID Product Code is NOT correct
#if ( configSII_DEV_9535 == 1)
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4D, 0x29, 0x35, 0x95, 0x01, 0x00, 0x00, 0x00,
#else
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x4D, 0x29, 0x33, 0x95, 0x01, 0x00, 0x00, 0x00,
#endif
	0x20, 0x15, 0x01, 0x03, 0x80, 0x52, 0x2E, 0x78, 0x0A, 0x0D, 0xC9, 0xA0, 0x57, 0x47, 0x98, 0x27,
	0x12, 0x48, 0x4C, 0x21, 0x09, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
	0x45, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20,
	0x6E, 0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x43,
	0x50, 0x39, 0x36, 0x38, 0x37, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFD,
// Bug 33154 - Melbourne SiI9533 EDID ID Product Code is NOT correct
#if ( configSII_DEV_9535 == 1)
	0x00, 0x17, 0x78, 0x0F, 0x85, 0x1E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x68,
#else
	0x00, 0x17, 0x78, 0x0F, 0x85, 0x1E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x6A,
#endif
    0x02, 0x03, 0x37, 0x72, 0x55, 0x90, 0x84, 0x03, 0x02, 0x0E, 0x0F, 0x07, 0x23, 0x24, 0x05, 0x94,
    0x13, 0x12, 0x11, 0x1D, 0x1E, 0x20, 0x21, 0x22, 0x01, 0x1F, 0x26, 0x09, 0x7F, 0x07, 0x15, 0x07,
    0x50, 0x83, 0x01, 0x00, 0x00, 0x6E, 0x03, 0x0C, 0x00, 0x10, 0x00, 0xF8, 0x3C, 0x21, 0x84, 0x80,
    0x01, 0x02, 0x03, 0x04, 0xE2, 0x00, 0xFF, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C,
    0x40, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x18, 0x01, 0x1D, 0x80, 0x18, 0x71, 0x1C, 0x16,
    0x20, 0x58, 0x2C, 0x25, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70
#endif

};

#if ( configSII_DEV_953x_PORTING == 1 ) /*Smith fixes: EDID 8-3 fail for VSDB 48 bits deep color*/
#define EDID_B1_CSUM        0xE7 // Last byte of EDID above ------------------------------------^
#else
#define EDID_B1_CSUM        0x70 // Last byte of EDID above ------------------------------------^
#endif

#define CEC_PA_ADDR         0xA9 // Offset of PA in EDID (... 03 0C 00 *10* 00 ...)

// EDID table is supposed to have PA = 1000
#define CSUM_PA_0     EDID_B1_CSUM                    // PA = 1000
#define CSUM_PA_1   ((EDID_B1_CSUM - 0x10) & 0xFF)    // PA = 2000
#define CSUM_PA_2   ((EDID_B1_CSUM - 0x20) & 0xFF)    // PA = 3000
#define CSUM_PA_3   ((EDID_B1_CSUM - 0x30) & 0xFF)    // PA = 4000

#define ENABLE_TWO_EDID_MODE  DISABLE

SiiDeviceBootData_t gEdidFlashDevBootData =
{
/*    0xAA,           // nvm_config;
    0x55,           // edid_valid;
    0x06,           // nvm_version;
#if (ENABLE_TWO_EDID_MODE == ENABLE)
    0x03,           // edid_copy_dest;  //two edid
#else
    0x3F,           // edid_copy_dest; 
#endif    
    0x3F,           // hpd_hw_ctrl;
    0x00,           // ddc_filter_sel;
    0x00,           // wakeup_source;
    0x40,           // edid_vga_copy_dest;
    { 0xFF, 0xFF }, // spare1[2];
    0xA6,           // cec_pa_addr;
    0xFF,           // spare2;
    0x10,           // cec_pad_h_ch0;
    0x00,           // cec_pad_l_ch0;
    0x20,           // cec_pad_h_ch1;
    0x00,           // cec_pad_l_ch1;   Offset 0x0F
    0x30,           // cec_pad_h_ch2;   Offset 0x10
    0x00,           // cec_pad_l_ch2;
    0x40,           // cec_pad_h_ch3;
    0x00,           // cec_pad_l_ch3;
    0x50,           // cec_pad_h_ch4;
    0x00,           // cec_pad_l_ch4;
    0x60,           // cec_pad_h_ch5;
    0x00,           // cec_pad_l_ch5;
    { 0xFF, 0xFF, 0xFF, 0xFF },// spare4[4];
    0x2C,           // checksum_ch0;    Offset 0x1C
    0x1C,           // checksum_ch1;
    0x0C,           // checksum_ch2;
    0xFC,           // checksum_ch3;
    0xEC,           // checksum_ch4;
    0xDC,           // checksum_ch5;    Offset 0x21
    { 0xFF, 0xFF }, // spare6[2];
    0x03,           // mhlPortSelect
    0xFF,           // spare
    0x84,           // miscControl0     RSEN and External Reset;    //0x26
    0xFF,           // Spare7                                       //0x27
    0x35,           // miscControl1                                 //0x28
    { 0 }           // spare8[24]

*/
    0xAA,
    0x55,
    0x06,
    0x0F, //00 extra Start
    0x0F,           //04: hpd_hw_ctrl
    0x00,           //05: i2c_filter_sel, sda_delay_en & ddc_filter_sel. CAUTION: non-zero value may lock up I2C permanently
    0x00,           //06: wakeup_source
    0x00,           //07: edid_vga_copy_dest
    {0xFF,0xFF},    //08,09: spare1[2]
    CEC_PA_ADDR,           //0A: cec_pa_addr
    0xFF, //08

#if 1 // smith fixes: EDID 8-3 failure item, FAIL->Physical address in HDMI VSDB is 0.0.3.0
    0x10,
    0x00,
    0x20,
    0x00, //0C
    0x30,
    0x00,
    0x40,
    0x00, //10
#else    
    0x00,
    0x10,
    0x00,
    0x20, //0C
    0x00,
    0x30,
    0x00,
    0x40, //10
#endif    
    {0xFF,0xFF,0x00,0x00,0xFF,0xFF}, //14-19: spare3[6]
    {0xFF,0xFF},    //1A-1B: spare4[2]
    CSUM_PA_0,           //1C: checksum_ch0
    CSUM_PA_1,           //1D: checksum_ch1
    CSUM_PA_2,           //1E: checksum_ch2
    CSUM_PA_3,           //1F: checksum_ch3
    {0xFF,0x00,0xFF}, //20,21,22: spare5[3]
    0xFF,           //23: spare6
    0x03,           //24: select MHL for port 0 and 1 (1st byte in this dword)
    0x00,           //25: mhl_version_supported
    0x04,           //26: RSEN and External Reset, mhl_1x_en, mhl_hpd_en, term_timer_en
    0x00,           //27: Spare7[1]
    0x35,           //28: MHl Control Register: (reserved), NVRAM_hpd_retry_sel,MHL_AUTO_TERM, EN_WAKEUP,
                    //                               PDN_IO_POWR, HW_CABLE_DETECT_EN, MHL_TRI_EN,NVM_STPG_EN
    0x01,           //29: MHL_DEV_CAP_03 -- MHL_ADOPTER_ID_HIGH
    0x42,           //2A: MHL_DEV_CAP_04 -- MHL_ADOPTER_ID_LOW
    0x95,           //2B: MHL_DEV_CAP_0B -- MHL_DEVICE_ID_HIGH
#if ( configSII_DEV_9535 == 1 )
    0x35,           //2C: MHL_DEV_CAP_0C -- MHL_DEVICE_ID_LOW
#else
    0x33,           //2C: MHL_DEV_CAP_0C -- MHL_DEVICE_ID_LOW
#endif
    { 0 }           // spare8[19]
};

#if (MHL_EXTRA == ENABLE)
// TODO:: TV Vendor needs to populate this
// These are the 2D Short Video Descriptors that are supported also in 3D
// These should be exactly in the same order as they are listed in EDID Block 1 (CEA-861)
const uint8_t g3DEdidTableVIC [ EDID_3D_VIC_TABLE_LEN ] =
{
	0x00, 	// burst_id_h
	0x10, 	// burst_id_l
	0xB6, 	// checksum:: for this particular WRITE_BURST of max 16 bytes, this includes all the entries
			//			  along with checksum itself, pls see MHL spec for more info
	0x15, 	// total # of MHL Video Descriptors in 3D_VIC Write_Burst sequence
	0x01,	// sequence of this particular WRITE_BURST among others
	0x05,	// number of 2 byte entries in this WRITE_BURST, anywhere between 0 to 5
	0x00, 0x06, 0x00, 0x07, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, // entries

	0x00, 	// burst_id_h , ,
	0x10, 	// burst_id_l
	0xB5, 	// checksum:: for this particular WRITE_BURST of max 16 bytes, this includes all the entries
			//			  along with checksum itself, pls see MHL spec for more info
	0x15, 	// total # of MHL Video Descriptors in 3D_VIC Write_Burst sequence
	0x02,	// sequence of this particular WRITE_BURST among others
	0x05,	// number of 2 byte entries in this WRITE_BURST, anywhere between 0 to 5
	0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x06, 0x00, 0x07, // entries

	0x00, 	// burst_id_h , ,
	0x10, 	// burst_id_l
	0xB0, 	// checksum:: for this particular WRITE_BURST of max 16 bytes, this includes all the entries
			//			  along with checksum itself, pls see MHL spec for more info
	0x15, 	// total # of MHL Video Descriptors in 3D_VIC Write_Burst sequence
	0x03,	// sequence of this particular WRITE_BURST among others
	0x05,	// number of 2 byte entries in this WRITE_BURST, anywhere between 0 to 5
	0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, 0x00, 0x07, // entries

	0x00, 	// burst_id_h , ,
	0x10, 	// burst_id_l
	0xB8, 	// checksum:: for this particular WRITE_BURST of max 16 bytes, this includes all the entries
			//			  along with checksum itself, pls see MHL spec for more info
	0x15, 	// total # of MHL Video Descriptors in 3D_VIC Write_Burst sequence
	0x04,	// sequence of this particular WRITE_BURST among others
	0x05,	// number of 2 byte entries in this WRITE_BURST, anywhere between 0 to 5
	0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, // entries

	0x00, 	// burst_id_h , ,
	0x10, 	// burst_id_l
	0xCF, 	// checksum:: for this particular WRITE_BURST of max 16 bytes, this includes all the entries
			//			  along with checksum itself, pls see MHL spec for more info
	0x15, 	// total # of MHL Video Descriptors in 3D_VIC Write_Burst sequence
	0x05,	// sequence of this particular WRITE_BURST among others
	0x01,	// number of 2 byte entries in this WRITE_BURST, anywhere between 0 to 5
	0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // entries
};

// TODO:: TV Vendor needs to populate this
// These are the 2D Detailed Timing Descriptors that are supported also in 3D
// These should be exactly in the same order as they are listed in EDID Block 0 and 1
// Block 1 (CEA-861) DTDs should follow after Block 0's
const uint8_t g3DEdidTableDTD [ EDID_3D_DTD_TABLE_LEN ] =
{
	0x00, 	// burst_id_h
	0x11, 	// burst_id_l
	0xDB, 	// checksum:: for this particular WRITE_BURST of max 16 bytes, this includes all the entries
			//			  along with checksum itself, pls see MHL spec for more info
	0x04, 	// total # of MHL Detailed Timing Descriptors in 3D_DTD Write_Burst sequence
	0x01,	// sequence of this particular WRITE_BURST among others
	0x02,	// number of 2 byte entries in this WRITE_BURST, anywhere between 0 to 5
	0x00, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // entries

	0x00, 	// burst_id_h , ,
	0x11, 	// burst_id_l
	0xDA, 	// checksum:: for this particular WRITE_BURST of max 16 bytes, this includes all the entries
			//			  along with checksum itself, pls see MHL spec for more info
	0x04, 	// total # of MHL Detailed Timing Descriptors in 3D_DTD Write_Burst sequence
	0x02,	// sequence of this particular WRITE_BURST among others
	0x02,	// number of 2 byte entries in this WRITE_BURST, anywhere between 0 to 5
	0x00, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // entries
};
#endif
