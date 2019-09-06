//***************************************************************************
//!file     si_ipv_config.h
//!brief    System-specific configuration information for IPV
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_IPV_H__
#define __SI_CONFIG_IPV_H__

#include "device_config.h"
//------------------------------------------------------------------------------
//  IPV Component SiI953x-specific
//------------------------------------------------------------------------------

#define SII_NUM_IPV                 1

#if ( configSII_DEV_9535 == 1 )
#define SII_IPV_PENTA_THEME_CC_INIT  0, -8660, 8660      // 0, -10^4 * cos(pi/6), +10^4 * cos(pi/6)
#define SII_IPV_PENTA_THEME_CS_INIT -10000, 5000, 5000   // 1, -10^4 * sin(pi/6), -10^4 * sin(pi/6)
#else
#define SII_IPV_PENTA_THEME_CC_INIT  0, -8660      // 0, -10^4 * cos(pi/6), +10^4 * cos(pi/6)
#define SII_IPV_PENTA_THEME_CS_INIT -10000, 5000   // 1, -10^4 * sin(pi/6), -10^4 * sin(pi/6)
#endif
#define SII_IPV_PENTA_THEME_RD_INIT  2, 3 // Relative radius {numerator, denominator} (has to be < 1)
// Bug 33345 - Melbourne SiI9535|when only two sources are connected to port 0 and port 1 PIP fails to work 
// InstaPrevue windows sizes for select MP resolutions
// Note: The InstaPrevue scaler has a maximum scale limit of 26:1, which prevents
// achieving correct aspect ratios for 640x480 and 720x480 displays (for 1920x1080
// or larger preview inputs) while maintaining the same approximate window size for
// each of the four main display resolutions.
//                                     wid16x9 wid4x3  height hspacer yspacer
#define SII_IPV_WIN_SIZE_640x480          80,     80,     48,    14,     32
#define SII_IPV_WIN_SIZE_720x480          80,     80,     48,    14,     32
#define SII_IPV_WIN_SIZE_1280x720        128,     96,     72,    26,     32
#define SII_IPV_WIN_SIZE_1440x480        144,    108,     48,    26,     32
#define SII_IPV_WIN_SIZE_1920x1080       192,    144,    108,    40,     48

// PIP window sizes for select MP resolutions
//                                       wid16x9 wid4x3  height hspacer yspacer
#define SII_IPV_PIP_WIN_SIZE_640x480     128,     96,     72,     0,      0
#define SII_IPV_PIP_WIN_SIZE_720x480     144,    108,     82,     0,      0
#define SII_IPV_PIP_WIN_SIZE_1280x720    256,    192,    144,     0,      0
#define SII_IPV_PIP_WIN_SIZE_1440x480    256,    192,     82,     0,      0
#define SII_IPV_PIP_WIN_SIZE_1920x1080   256,    192,    144,     0,      0

// PIP window coordinates for select MP resolutions
// For best performance, Y coord should be a multiple of 8
//                                       X                Y
#define SII_IPV_PIP_WIN_COORDS_640x480    640 - 128 - 24,  480 -  72 - 32
#define SII_IPV_PIP_WIN_COORDS_720x480    720 - 144 - 32,  480 -  80 - 32
#define SII_IPV_PIP_WIN_COORDS_1280x720  1280 - 256 - 48,  720 - 144 - 40
#define SII_IPV_PIP_WIN_COORDS_1440x480  1440 - 256 - 48,  480 -  82 - 32
#define SII_IPV_PIP_WIN_COORDS_1920x1080 1920 - 256 - 72, 1080 - 144 - 80

// Window background color (Cr, Cb, Y)
#define SII_IPV_WIN_BG_COLOR        0x7F, 0x7F, 0x40  //  Dark Gray
// Window border color (Cr, Cb, Y)
#define SII_IPV_WIN_BORDER_COLOR    0x7F, 0x7F, 0x82 // Light Gray

// Bouncing effect amplitude (pixels)
#define SII_IPV_BOUNCE_AMPL     32

#endif  //__SI_CONFIG_IPV_H__
