//************************************************************************************************
//! @file   si_edid_tx_tables.c
//! @brief  Default downstream EDID tables for substitution in case of invalid DS EDID structures
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#include "si_edid_tx_internal.h"

// This table is to be used as DS EDID if our device is unable to read real DS EDID.
ROM const uint8_t defaultDsEdid[EDID_BLOCK_SIZE] =
{
// (00H)      Header
    0x00,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0x00,

// (08H-09H)  ID Manufacturer Name _________________________    = Unknown
// (0AH-0BH)  Product ID Code ______________________________    = Unknown
// (0CH-0FH)  Last 4 Digits of Serial Number _______________    = NOT USED
// (10H)      Week of Manufacture __________________________    = 0
// (11H)      Year of Manufacture __________________________    = 1990
    0x01, 0x01, // EDID__MANUFACTURER_ID,
    0x01, 0x01, //EDID__PRODUCT_ID,
    0x01,  0x01,  0x01,  0x01,
    0x00,
    0x00,

// (12H)      EDID version 1.3
    0x01,  0x03,

//           Basic Display Parameters
//(14H)      VIDEO INPUT DEFINITION: Digital Signal
//(15H)      Maximum Horizontal Image Size ________________    =   undefined
//(16H)      Maximum Vertical Image Size __________________    =   undefined
//(17H)      Display Gamma ________________________________    =   2.2 (BT.709)
//(18H)      DPMS and Supported Feature(s):
//           Display Type = RGB Color, Preferred Timing Mode
    0x80,  0x00,  0x00,  0x78,  0x0A,

//(19H-22H)  CHROMA INFO:

//         Red x - 0.640 Green x - 0.300 Blue x - 0.150 White x - 0.313
//         Red y - 0.330 Green y - 0.600 Blue y - 0.060 White y - 0.329
//         sRGB
    0xEE,  0x91,  0xA3,  0x54,  0x4C,  0x99,  0x26,  0x0F,  0x50,  0x54,


//(23H)      ESTABLISHED TIMING I:
    //0x20, // 640 x 480 @ 60Hz (IBM,VGA)
    0x00, // None Specified

//(24H)      ESTABLISHED TIMING II:
    0x00, // None Specified

//(25H)      Manufacturer's Reserved Timing:
    0x00, // None Specified

//(26H-35H)  Standard Timing Identification:
//           Not Used
    0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,
    0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,

// Since at least one preferred timing structure is required in EDID 1.3

//(36H-47H) Detailed Timing / Descriptor Block 1:
#ifdef DEFAULT_SYSTEM__PAL
// PAL
// Detailed Timing: 720x576p 50Hz 4:3
#  if 17 != DEFAULT_VIC
#  error Default EDID tables mismatch
#  endif
    0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55, 0x00,
    0x13, 0x8E, 0x21, 0x00, 0x00, 0x18,
#else // DEFAULT_SYSTEM__PAL
// NTSC
// Detailed Timing: 720x480p 59.94Hz 4:3
#  if 2 != DEFAULT_VIC
#  error Default EDID tables mismatch
#  endif
    0x8C, 0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00,
    0x13, 0x8E, 0x21, 0x00, 0x00, 0x18,
#endif // DEFAULT_SYSTEM__PAL


//(48H-59H) Detailed Timing / Descriptor Block 2:
// Descriptor: ASCII string with dummy data (just to fill out the DTD space)
    0x00, 0x00, 0x00, 0xFE, 0x00, // header
    'D', 'U', 'M', 'M', 'Y', 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,

//(5AH-6BH) Detailed Timing / Descriptor Block 3:
// Descriptor: Monitor Name string
    0x00,  0x00,  0x00,  0xFC,  0x00, // header
    'S', 'I', 'I', ' ', 'M', 'L', 'B', 'N', ' ', 'R', 'P', 'T', 0x0A,

//(6CH-7DH) Detailed Timing / Descriptor Block 4:
//         Monitor Range Limits:
    0x00,  0x00,  0x00,  0xFD,  0x00, // header
    DEFAULT__MIN_V_HZ,          // Min Vertical Freq
    DEFAULT__MAX_V_HZ,          // Max Vertical Freq
    DEFAULT__MIN_H_KHZ,         // Min Horiz. Freq
    DEFAULT__MAX_H_KHZ,         // Max Horiz. Freq
    DEFAULT__MAX_PIX_CLK_10MHZ, // Pixel Clock in 10MHz
    0x00,       // GTF - Not Used
    0x0A,  0x20,  0x20,  0x20,  0x20,  0x20,  0x20, // padding

//(7EH)    Number Of Extension Blocks
//         No Extension EDID Block(s)
    0x00,

//(7FH)    Check Sum
    0x00   // not correct, but it is not important here
};

#if 0
// Detailed Timing: 720x576p 50Hz 4:3
ROM const byte DefaultDtd576p[EDID_LEN__DTD] =
{
    0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55, 0x00,
    0x13, 0x8E, 0x21, 0x00, 0x00, 0x18,
};
#endif //0



// TODO:OEM - This table is an example. Fill up by results of QA testing.
ROM const remedyListElement_t EdidRemedyList[] =
{
    //ManufacturerId, ProductId/date, RemedyMask, Type

    // Examples
//  {0x1111,    0,      SI_TX_EDID_REMEDY_DO_NOT_USE_YCBCR, tManufacturerLimitation},
//  {0x2222,    0,      SI_TX_EDID_REMEDY_DVI_PC_RANGE, tManufacturerLimitation},
//  {0x3333,    0x0123, 0,      tNoLimitation},
//  {0x4444,    0x9999, SI_TX_EDID_REMEDY_DO_NOT_USE_CP_MUTE, tProductLimitation},
//  {0x5555,    0x8888, SI_TX_EDID_REMEDY_DO_NOT_USE_CP_MUTE, tProductLimitation},
//  {0x3333,    0x0F10, SI_TX_EDID_REMEDY_TEST, tLimitationOnTheseDays}, // 2005-2006
//  {0x4444,    0x0D0E, SI_TX_EDID_REMEDY_TEST, tLimitationOnTheseDays}, // 2003-2004

    {0x0000,    0x000F, SI_TX_EDID_REMEDY_DELAY_DS_AUTH, tLimitationOnTheseDays}, // 1990-2005, any manufacturer
    // This remedy makes additional delay between the moment when TX starts output but
    // before downstream authentication starts.
    // Without this some older incompatible sinks may fail during instaport switching.

    {0,         0,      0,      tEndOfList}, // End Of The List (the last element)
};


// TODO:OEM - The following example is a "bypass" scaler table.
// However, unlike the "Scaler Bypass" mode, it also allows for
// elimination of unsupported video modes.
ROM const videoScalerMapping_t DefaultAllowedScalerModes[] =
{
    {vm1_640x480p,          vm1_640x480p,           false,  false,  false},

    {vm2_3_720x480p,        vm2_3_720x480p,         false,  false,  false},
    {vm2_3_720x480p,        vm2_3_720x480p,         true,   true,   false},
    {vm17_18_720x576p,      vm17_18_720x576p,       false,  false,  false},
    {vm17_18_720x576p,      vm17_18_720x576p,       true,   true,   false},

    {vm4_1280x720p,         vm4_1280x720p,          true,   true,   true},
    {vm19_1280x720p,        vm19_1280x720p,         true,   true,   true},

    {vm5_1920x1080i,        vm5_1920x1080i,         true,   true,   true},
    {vm20_1920x1080i,       vm20_1920x1080i,        true,   true,   true},

    {vm16_1920x1080p,       vm16_1920x1080p,        true,   true,   true},
    {vm31_1920x1080p,       vm31_1920x1080p,        true,   true,   true},

    {vm6_7_720_1440x480i,   vm6_7_720_1440x480i,    false,  false,  false},
    {vm6_7_720_1440x480i,   vm6_7_720_1440x480i,    true,   true,   false},
    {vm21_22_720_1440x576i, vm6_7_720_1440x480i,    false,  false,  false},
    {vm21_22_720_1440x576i, vm6_7_720_1440x480i,    true,   true,   false},

    {0xFF,  0xFF,   false,  false, false }, // "EndOfTable" mark
};
