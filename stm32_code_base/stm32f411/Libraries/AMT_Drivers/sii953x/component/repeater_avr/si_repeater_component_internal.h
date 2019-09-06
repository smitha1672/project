//************************************************************************************************
//! @file   si_repeater_component_internal.h
//! @brief  Repeater software component (internal definitions)
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/


#ifndef SI_REPEATER_COMPONENT_INTERNAL_H_
#define SI_REPEATER_COMPONENT_INTERNAL_H_
#include "si_debug.h"
#include "si_repeater_component.h"

#define RPT_MSG_DBG                  (MSG_DBG | DBGF_TS | DBGF_CN), DBG_RPTR
#define RPT_MSG_ALWAYS                (MSG_ALWAYS | DBGF_CN), DBG_RPTR

#define RPT_ALL_TX						0xFF

//------------------------------------------------------------------------------
//  Repeater Component Instance Data
//------------------------------------------------------------------------------

void RepeaterIsrHandler( clock_time_t msDiff, RepeaterInstanceData_t *currentPipe);
void RepeaterConOnTxAvMuteTurnOn( clock_time_t msDiff );
void RepeaterConOnTxHpdTurnOn( bool_t qOn , bool_t sameTx);
void RepeaterResetTxHdcpStatusData(uint8_t txN);
bool_t RepeaterCurrentInputPortIsSetToNoHdcp();


#endif /* SI_REPEATER_COMPONENT_INTERNAL_H_ */
