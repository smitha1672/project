//************************************************************************************************
//! @file   si_edid_tx_tables.h
//! @brief  Default downstream EDID tables for substitution in case of invalid DS EDID structures
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#ifndef SII_EDID_TABLES_H
#define SII_EDID_TABLES_H

//#include "si_edid_parsing.h"

extern ROM const uint8_t defaultDsEdid[EDID_BLOCK_SIZE];
//extern ROM const byte DefaultDtd576p[EDID_LEN__DTD];

#define DEFAULT_EDID_BL0_ADR__DUMMY_STRING EDID_BL0_ADR__DTD2
#define DEFAULT_EDID_BL0_ADR__MONITOR_NAME EDID_BL0_ADR__DTD3
#define DEFAULT_EDID_BL0_ADR__RANGE_LIMITS EDID_BL0_ADR__DTD4

#ifdef DEFAULT_SYSTEM__PAL
// PAL
// Detailed Timing: 720x576p 50Hz 4:3
#  define DEFAULT_VIC	17
#else // DEFAULT_SYSTEM__PAL
// NTSC
// Detailed Timing: 720x480p 59.94Hz 4:3
#  define DEFAULT_VIC	2
#endif // DEFAULT_SYSTEM__PAL

#endif // SII_EDID_TABLES_H
