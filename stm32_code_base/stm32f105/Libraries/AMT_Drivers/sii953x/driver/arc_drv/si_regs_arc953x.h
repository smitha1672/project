//***************************************************************************
//!file     si_regs_arc953x.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_9535REGS_ARC_H__
#define __SI_9535REGS_ARC_H__

#include "si_cra_cfg.h"

//------------------------------------------------------------------------------
// Registers in Page 15   (0xF8)
//------------------------------------------------------------------------------

#define REG_ARC_SPDIF_CTRL          (PP_PAGE_AUDIO | 0xEF)
#define BIT_SPDIF_OUT_MODE_SEL_0        BIT0
#define BIT_ARC_DISABLE                 BIT5

#endif  // __SI_9535REGS_ARC_H__

