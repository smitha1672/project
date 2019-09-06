//************************************************************************************************
//! @file   si_edid_tx_analyze.h
//! @brief  Downstream EDID analysis
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/

#ifndef SII_EDID_ANALYZER_H
#define SII_EDID_ANALYZER_H

#include "si_common.h"
#include "si_edid_tx_component.h"
#include "si_edid_tx_parsing.h"

#define EDID_LATENCY_LENGTH 4 // number of latency parameters


typedef struct
{
    bit_fld_t           HdmiMode : 1;
    bit_fld_t           SupportAudio : 1;
    bit_fld_t           SupportPcmMcAudio : 1; // Multichannel PCM support
    bit_fld_t           SupportPcmFs88k : 1;
    bit_fld_t           SupportPcmFs96k : 1;
    bit_fld_t           SupportPcmFs176k : 1;
    bit_fld_t           SupportPcmFs192k : 1;
    bit_fld_t           SupportYCbCr : 1;
    bit_fld_t           SupportNtsc : 1;
    bit_fld_t           SupportPal : 1;
    bit_fld_t           SupportEncodedAudio : 1;
    bit_fld_t           Support16x9 : 1;
    bit_fld_t           Support4x3 : 1;
    bit_fld_t           Support3D : 1;
    uint16_t            hdmi_3d_structure_all;
    uint8_t             Extra0;
    uint16_t            RemedyMask;
    uint8_t             MaxTmdsClk;
    bit_fld_t           SupportDtd : 1;
    bit_fld_t           SupportHbra : 1;
    bit_fld_t           xvYCC_p0 : 1; // xvYCC profile p0
    uint8_t             latency[EDID_LATENCY_LENGTH];
    shortDtd_t          SDTD[MAX_NMB_OF_KNOWN_DTDS];

} edidAnalysis_t;


#endif // SII_EDID_ANALYZER_H
