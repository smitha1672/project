//************************************************************************************************
//! @file   si_scaler_tables.h
//! @brief  Define permitted scaling modes (input A/V format into output A/V format) by a table
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#ifndef SII_SCALER_TABLES_H
#define SII_SCALER_TABLES_H

#include "si_common.h"

typedef struct videoScalerMapping PACKED
{
    uint8_t	    InFmtId;
    uint8_t	    OutFmtId;
    bit_fld_t   InAspectRatio16x9   : 1;
    bit_fld_t   OutAspectRatio16x9  : 1;
    bit_fld_t   CapableOf3D         : 1;

} videoScalerMapping_t;

#ifndef SII_USE_SCALER_TABLE_IN_NVM // NOT
extern ROM const videoScalerMapping_t DefaultAllowedScalerModes[];
#endif // NOT SII_USE_SCALER_TABLE_IN_NVM

#endif // SII_SCALER_TABLES_H

