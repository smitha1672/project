//************************************************************************************************
//! @file   si_edid_tx_config.h
//! @brief  EDIT Tx configuration definitions
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2011, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/

#ifndef SII_EDID_CONFIG_H
#define SII_EDID_CONFIG_H

#include "si_common.h"

//------------------------------------------------------------------------------
//  EDID TX Component SiI953x-specific
//------------------------------------------------------------------------------

#define SII_NUM_EDID_TX             1   // Number of EDID_TX instance

//------------------------------------------------------------------------------
//  SII_PREFERRED_OUT_IDX   - user preferred repeater output video index
//                            or 0xFF if no preference
//------------------------------------------------------------------------------

#define SII_PREFERRED_OUT_IDX         0xFF

#define SET_MANUFACTURER_WEEK (18)
#define SET_MANUFACTURER_YEAR (13)


#define REPEATER__STANDBY   0x80
#define REPEATER__SUSPEND   0x40
#define REPEATER__LOW_POWER 0x20

//#define   REPEATER_POWER_FEATURES (REPEATER__STANDBY | REPEATER__SUSPEND | REPEATER__LOW_POWER)
#define REPEATER_POWER_FEATURES (0)

#define DEFAULT__V_SIZE     90 // 90mm (because it is easily divided by 9 and by 3)

// Audio
#if ( configSII_DEV_953x_PORTING == 1 ) /*Smith modifies for SAD*/
#define REPEATER_AUDIO__SPEAKERS  0x0F // 5.1 channels
#else
#define REPEATER_AUDIO__SPEAKERS    0x4F // 7.1 channels (5.1ch + RRC/RLC)
#endif 

#define REPEATER_AUDIO__AI_SUPPORT  1    // 1 - supported, 0 - not supported
#ifdef SII_ALLOW_ENCODED_AUDIO_IN_EDID_DEFAULT_SETTINGS

#if 0 /*Smith arranges*/
/* PCM 8ch 32-192K, AC-3 6ch 640Kbps, DTS 6ch 1536Kbps, 1BIT 2ch 32-192K */
#define REPEATER_AUDIO__SADS        {0x00077F0F, 0x00500415, 0x00C0143D, 0x00001F49, 0}
#define REPEATER_AUDIO__SADS_LENGTH 5
#endif

#if 0 /*Smith arranges*/
/* PCM 8ch 32-192K, AC-3 6ch (48K) 640Kbps, DTS 6ch (48 & 96K) 1536Kbps, 1BIT 6ch (44.1K), DD+ 8ch 32-192K, DTS-HD 8ch 32-192K*/
#define REPEATER_AUDIO__SADS        {0x00077F0F, 0x00500415, 0x00C0143D, 0x0000024D, 0x00007F57, 0x00007F5F, 0}
#define REPEATER_AUDIO__SADS_LENGTH 7
#endif

#if 0 /*Smith arranges*/
/* PCM 8ch 32-192K, AC-3 6ch (48K) 640Kbps, DTS 6ch (48 & 96K) 1536Kbps, 1BIT 6ch (44.1K), DD+ 8ch 32K,44K,48K,96K, DTS-HD 8ch 48K,96k*/
#define REPEATER_AUDIO__SADS        {0x00077F0F, 0x00500415, 0x00C0143D, 0x0000024D, 0x00001757, 0x0000145F, 0}
#define REPEATER_AUDIO__SADS_LENGTH 7
#endif 

#if 1 /*Smith arranges*/
/* PCM 8ch 32-192K, AC-3 6ch (32.44,48K) 640Kbps, DTS 6ch (32,44,48,88,96K) 1536Kbps, 1BIT 6ch (44.1K), DD+ 8ch 32K,44K,48K,96K, DTS-HD 8ch 48K,96k*/
#define REPEATER_AUDIO__SADS        {0x00077F0F, 0x00500715, 0x00C01F3D, 0x0000024D, 0x00001757, 0x0000145F, 0}
#define REPEATER_AUDIO__SADS_LENGTH 7
#endif 

// LAST WORD MUST BE ZERO! NO MORE THEN MAX_NMB_OF_SADS ELEMENTS!
#else // SII_ALLOW_ENCODED_AUDIO_IN_EDID_DEFAULT_SETTINGS

#if 0 /*Smith arranges*/
/* PCM 2ch 32-192K, PCM 6ch 32-48K LAST WORD MUST BE ZERO! NO MORE THEN MAX_NMB_OF_SADS ELEMENTS! */
#define REPEATER_AUDIO__SADS        {0x00077F09, 0x0007070D, 0}
#define REPEATER_AUDIO__SADS_LENGTH 3
#endif

#if ( configSII_DEV_953x_PORTING == 1 ) /*Tony modifies*/
#define REPEATER_AUDIO__SADS        { 	0x00071F0F, /* PCM 8ch 32k,44.1k,48k,96k*/\
                                        0x00500715, /* AC-3 6ch (48K) 640Kbps */\
                                        0x00C01F3D, /* DTS 6ch (48 & 96K) 1536Kbps*/ \
                                        0}
                                        //0x0000024D, /* 1BIT 6ch (44.1K)*/ \
                                        //0x00001757, /*DD+ 8ch 32K,44K,48K,96K,*/ 
                                        //0x0000145F, /*DTS-HD 8ch 48K,96k*/
										
#define REPEATER_AUDIO__SADS_LENGTH 4//7
#else
/* PCM 8ch 32-192K, 1BIT 6ch (44.1K) LAST WORD MUST BE ZERO! NO MORE THEN MAX_NMB_OF_SADS ELEMENTS!*/
#define REPEATER_AUDIO__SADS        {0x00077F0F, 0x0000024D, 0}
#define REPEATER_AUDIO__SADS_LENGTH 3
#endif

#endif // SII_ALLOW_ENCODED_AUDIO_IN_EDID_DEFAULT_SETTINGS


#ifdef SII_LIMIT_VIDEO_BUS_WITH_861B_50_60_HZ_RESOLUTIONS
#  define VIDEO__MIN_V_HZ           48 // PAL
#  define VIDEO__MAX_V_HZ           62 // NTSC
#  define VIDEO__MIN_H_KHZ          14 // 480i, 576i
#  define VIDEO__MAX_H_KHZ          69 // 1080p
#  define VIDEO__MAX_PIX_CLK_10MHZ  15 // 1080p
#else // SII_LIMIT_VIDEO_BUS_WITH_861B_50_60_HZ_RESOLUTIONS
//range limits to satisfy all 861C video modes
#  define VIDEO__MIN_V_HZ           22
#  define VIDEO__MAX_V_HZ           243
#  define VIDEO__MIN_H_KHZ          14
#  define VIDEO__MAX_H_KHZ          129

#  if (FPGA_BUILD == DISABLE)
#     define VIDEO__MAX_PIX_CLK_10MHZ   15 // 1080p,  17 - to support more PC modes
#  else
#     define VIDEO__MAX_PIX_CLK_10MHZ   8 //Only support up to 720p for FPGA requirement.
#  endif
#endif // SII_LIMIT_VIDEO_BUS_WITH_861B_50_60_HZ_RESOLUTIONS

#define MHL__MIN_V_HZ           22
#define MHL__MAX_V_HZ           62
#define MHL__MIN_H_KHZ          14
#define MHL__MAX_H_KHZ          69
#define MHL__MAX_PIX_CLK_10MHZ  8  // up to 720p60

#define DEFAULT__MIN_V_HZ           48 // Min. Vertical rate (for interlace this refers to field rate), in Hz.
#define DEFAULT__MAX_V_HZ           62 // Max. Vertical rate (for interlace this refers to field rate), in Hz.
#define DEFAULT__MIN_H_KHZ          23 // Min. Horizontal in kHz
#define DEFAULT__MAX_H_KHZ          47 // Max. Horizontal in kHz,
#define DEFAULT__MAX_PIX_CLK_10MHZ  8  // Max. Supported Pixel Clock, in MHz/10, rounded

#define DEFAULT__MAX_TMDS_CLK_5MHZ  60 // Max. Supported TMDS Clock, in MHz/5, rounded

#if ( configSII_DEV_953x_PORTING == 0 )
#define EDID__MANUFACTURER_ID   0x4D29 // SII
#else
#define EDID__MANUFACTURER_ID 0x593A /*VIZ*/
#endif 

// Bug 33154 - Melbourne SiI9533 EDID ID Product Code is NOT correct
#if( configSII_DEV_9535 == 1 )          
#define EDID__PRODUCT_ID        0x3595 // (CP9535)
#else
#define EDID__PRODUCT_ID        0x3395 // (CP9533)
#endif

#define EDID_LATENCY_VIDEO_LOCAL        0   // Video latency in (milliseconds/2 + 1) units
#define EDID_LATENCY_AUDIO_LOCAL        0   // Audio latency in (milliseconds/2 + 1) units
#define EDID_LATENCY_I_VIDEO_LOCAL      0   // Interlaced Video latency in (milliseconds/2 + 1) units
#define EDID_LATENCY_I_AUDIO_LOCAL      0   // Interlaced Video latency in (milliseconds/2 + 1) units


#define ALWAYS_PLAY_AUDIO_LOCALLY_WHEN_DS_DOES_NOT_SUPPORT_AUDIO

#define CHECK_AND_CORRECT_RANGE_LIMITS


#endif // SII_EDID_CONFIG_H

