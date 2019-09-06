//***************************************************************************
//!file     si_sii9535_registers.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_REPEATER_H__
#define __SI_9535REGS_REPEATER_H__

#include "si_cra_cfg.h"



//------------------------------------------------------------------------------
// NOTE: Register addresses are 16 bit values with page and offset combined.
//
// Examples:  0x005 = page 0, offset 0x05
//            0x1B6 = page 1, offset 0xB6
//------------------------------------------------------------------------------

#define REG_EDID_FIFO_SEL           (PP_PAGE_EDID | 0x04)

#define BIT_KSV_TX1_FIRST               0x20
#define VAL_KSV_SELECT_MASK             0xC0
#define VAL_KSV_SELECT_US               0x00
#define VAL_KSV_SELECT_TX0              0x40
#define VAL_KSV_SELECT_TX1              0x80



#endif  // __SI_9535REGS_REPEATER_H__

