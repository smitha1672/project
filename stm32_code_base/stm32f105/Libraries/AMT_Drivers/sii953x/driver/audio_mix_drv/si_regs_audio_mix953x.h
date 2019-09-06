//***************************************************************************
//!file     si_regs_audio_mix953x.h
//!brief    SiI9535 Device Register Manifest Constants.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef SI_9535REGS_AUDIO_MIX_H
#define SI_9535REGS_AUDIO_MIX_H

#include "si_cra_cfg.h"

#define REG_TX_AUD_MIX_CTRL0          (PP_PAGE_AUDIO_MIXER | 0xF0)
#define BIT_TX_AUD_MIX_DROP_GCP                 (BIT0)
#define BIT_TX_AUD_MIX_DROP_CTS                 (BIT1)
#define BIT_TX_AUD_MIX_DROP_AVI                 (BIT2)
#define BIT_TX_AUD_MIX_DROP_AIF                 (BIT3)
#define BIT_TX_AUD_MIX_DROP_SPDIF               (BIT4)
#define BIT_TX_AUD_MIX_DROP_MPEG                (BIT5)
#define BIT_TX_AUD_MIX_DROP_CEA_CP              (BIT6)
#define BIT_TX_AUD_MIX_DROP_CEA_GAMUT           (BIT7)

#define REG_TX_AUD_MIX_CTRL1          (PP_PAGE_AUDIO_MIXER | 0xF1)
#define BIT_TX_AUD_MIX_DROP_VSIF                (BIT0)
#define BIT_TX_AUD_MIX_DROP_GEN1                (BIT1)
#define BIT_TX_AUD_MIX_DROP_GEN2                (BIT2)
#define BIT_TX_AUD_MIX_PASS_NAUD_PKTS           (BIT3)
#define BIT_TX_AUD_MIX_PASS_AUD_PKT             (BIT4)
#define BIT_TX_AUD_MIX_PQ_PRIORITY              (BIT6)  // Packet Queue over audio input priority bit

#define REG_TX_AUD_MIX_CTRL3          (PP_PAGE_AUDIO_MIXER | 0xF2)
#define BIT_TX_AUD_MIX_DI_ADJUST                (BIT0)
#define BIT_TX_AUD_MIX_AUD_TYPE_OVR             (BIT1)
#define BIT_TX_AUD_MIX_HDMI_MODE_OVR            (BIT2)
#define BIT_TX_AUD_MIX_DEEP_COL_OVR             (BIT3)


#endif  // SI_9535REGS_AUDIO_MIX_H
