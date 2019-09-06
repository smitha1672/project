//***************************************************************************
//!file     si_ipv_data.c
//!brief    Silicon Image InstaPrevue Component theme data.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_ipv_component.h"

SiiWinSize_t    siiIpvWinSizes[SiiIpvSizeCount] =
{
    {SII_IPV_WIN_SIZE_640x480},
    {SII_IPV_WIN_SIZE_720x480},
    {SII_IPV_WIN_SIZE_1280x720},
    {SII_IPV_WIN_SIZE_1440x480},  // Bug 33345 - Melbourne SiI9535|when only two sources are connected to port 0 and port 1 PIP fails to work 
    {SII_IPV_WIN_SIZE_1920x1080},

//    //   wid16x9 wid4x3  height hspacer yspacer
//    {    64,     64,     48,    14,     32  },      // (640x480)    .750
//    {    72,     64,     48,    14,     32  },      // (720x480)    .66667
//    {   128,     96,     72,    26,     32  },      // (1280x720)   .5625
//    {   192,    144,    108,    40,     48  },      // (1920x1080)  .5625
};

SiiWinSize_t    siiIpvPipSizes[SiiIpvSizeCount] =
{
    {SII_IPV_PIP_WIN_SIZE_640x480},
    {SII_IPV_PIP_WIN_SIZE_720x480},
    {SII_IPV_PIP_WIN_SIZE_1280x720},
    {SII_IPV_PIP_WIN_SIZE_1440x480},    // Bug 33345 - Melbourne SiI9535|when only two sources are connected to port 0 and port 1 PIP fails to work 
    {SII_IPV_PIP_WIN_SIZE_1920x1080},

//    {   128,     96,     72,     0,      0  },      // (640x480)
//    {   144,    108,     82,     0,      0  },      // (720x480)
//    {   256,    192,    144,     0,      0  },      // (1280x720)
//    {   384,    288,    216,     0,      0  },      // (1920x1080)
};

// For best performance, Y coord should be a multiple of 8
SiiWinCoords_t  siiIpvPipCoords[SiiIpvSizeCount] =
{
    {SII_IPV_PIP_WIN_COORDS_640x480},
    {SII_IPV_PIP_WIN_COORDS_720x480},
    {SII_IPV_PIP_WIN_COORDS_1280x720},
    {SII_IPV_PIP_WIN_COORDS_1440x480},    // Bug 33345 - Melbourne SiI9535|when only two sources are connected to port 0 and port 1 PIP fails to work 
    {SII_IPV_PIP_WIN_COORDS_1920x1080},

//    {    640 - 128 - 24,  480 -  72 - 32    },
//    {    720 - 144 - 32,  480 -  80 - 32    },
//    {   1280 - 256 - 48,  720 - 144 - 40    },
//    {   1920 - 384 - 72, 1080 - 216 - 80    },
};

SiiWinCoords_t  siiMainWindowSizes[SiiIpvSizeCount] =
{
    {    640,  480 },
    {    720,  480 },
    {   1280,  720 },
    {   1440,  480 },      // Bug 33345 - Melbourne SiI9535|when only two sources are connected to port 0 and port 1 PIP fails to work 
    {   1920, 1080 },
};


uint8_t siiIpvBorderEnables[SiiIpvThemeCount] =   // Display boundary enables for each theme
{
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

uint8_t siiIpvBorderColorsCrCbY[SiiIpvBorderColorCount][YCBCR_LEN] =  // YCbCr color data array
{
    {SII_IPV_WIN_BORDER_COLOR}, // ALL mode enable & border color CrCbY values
    {0x45, 0x54, 0xB5},         // ACT mode enable & border color CrCbY values  (RGB:57E964 - Spring Green2 (brighter))
    {0xD1, 0x91, 0x66},         // CPB enable & border color CrCbY values       (RGB:F6358A - Violet Red)
    {0x00, 0x00, 0x00},         // USF enable & border color CrCbY values       (RGB:2B547E - Steel Blue4)
    {SII_IPV_WIN_BG_COLOR}      // USF background color CrCbY values
};

uint8_t siiIpvAlphaMax[IPV_WINDOW_COUNT];           // Maximum alpha blend level for each window

uint8_t siiIpvAlphaStep[IPV_WINDOW_COUNT];          // Alpha blend level step for each window

// Sine and Cosine factors of polar coordinates for "Pentagram" theme
const int32_t siiIpvCc[IPV_WINDOW_COUNT] = {SII_IPV_PENTA_THEME_CC_INIT};
const int32_t siiIpvCs[IPV_WINDOW_COUNT] = {SII_IPV_PENTA_THEME_CS_INIT};

// Relative radius {numerator, denominator} (has to be < 1)
const int32_t siiIpvRad[2] =  {SII_IPV_PENTA_THEME_RD_INIT};

