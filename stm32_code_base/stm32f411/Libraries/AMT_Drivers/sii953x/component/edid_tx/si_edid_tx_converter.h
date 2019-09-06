//************************************************************************************************
//! @file   si_edid_tx_converter.h
//! @brief  Modify DS EDID parsed data for the Repeater mode
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#ifndef SII_EDID_CONVERTER_H
#define SII_EDID_CONVERTER_H

#include "si_common.h"

void EdidTxConvert( void );
void EdidTxOwnSadAndSpkSet( void );
void EdidTxUsCecPhysAddrDiscovery(cecPhysAddr_t *p_cec, uint8_t input_nmb);

#endif // SII_EDID_CONVERTER_H
