//***************************************************************************
//! @file     si_drv_audio_mix.c
//! @brief    Audio Mixer driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_drv_device.h"
#include "si_regs_audio_mix953x.h"
#include "si_drv_internal.h"
#include "si_drv_audio_mix.h"
#include "si_regs_tpi953x.h"

#define SII_NUM_TX  1

//-------------------------------------------------------------------------------------------------
//! @brief      Select Tx device instance in multi-transmitter system.
//!
//! @param[in]  device_idx - zero based index of an instance.
//-------------------------------------------------------------------------------------------------

void SiiDrvRxAudioMixInstanceSet(uint8_t deviceIndex)
{
#if SII_NUM_TX > 1
    if(deviceIndex < SII_NUM_TX)
    {
        SiiRegInstanceSet( PP_PAGE_AUDIO_MIXER, deviceIndex );
    }
#endif
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set pass through mode.
//!
//! @param[in]  isAudioPassEnabled     - Audio packets will pass through, if true
//! @param[in]  isNonAudioPassEnabled  - Non-audio packets will pass through, if true
//-------------------------------------------------------------------------------------------------

void SiiDrvRxAudioMixPassThroughConfig(bool_t isAudioPassEnabled, bool_t isNonAudioPassEnabled)
{
	//SiiRegModify( REG_TPI__AUDIO_CFG, (BIT_TPI__AUDIO_CFG__MUTE | VAL_TPI__AUDIO_CFG__SPDIF | VAL_TPI__AUDIO_CFG__I2S), BIT_TPI__AUDIO_CFG__MUTE);
    SiiRegModify( REG_TX_AUD_MIX_CTRL1, BIT_TX_AUD_MIX_PASS_AUD_PKT |
                  BIT_TX_AUD_MIX_PASS_NAUD_PKTS | BIT_TX_AUD_MIX_DROP_GEN1,
                    (isAudioPassEnabled ? BIT_TX_AUD_MIX_PASS_AUD_PKT : 0) |
                    (isNonAudioPassEnabled ? BIT_TX_AUD_MIX_PASS_NAUD_PKTS : 0) |
                    /* SWWA: 23624 Always drop NULL packets. They may get in the way of mixer operation */
                    BIT_TX_AUD_MIX_DROP_GEN1);

    SiiRegModify( REG_TX_AUD_MIX_CTRL0, BIT_TX_AUD_MIX_DROP_AIF | BIT_TX_AUD_MIX_DROP_SPDIF | BIT_TX_AUD_MIX_DROP_CTS,
                    (isAudioPassEnabled ? 0: (BIT_TX_AUD_MIX_DROP_AIF | BIT_TX_AUD_MIX_DROP_SPDIF | BIT_TX_AUD_MIX_DROP_CTS)) );

    SiiRegModify( REG_TX_AUD_MIX_CTRL3, BIT_TX_AUD_MIX_AUD_TYPE_OVR, (isAudioPassEnabled ? 0: BIT_TX_AUD_MIX_AUD_TYPE_OVR));
}

//-------------------------------------------------------------------------------------------------
//! @brief      drop GCP packet.
//!
//! @param[in]
//-------------------------------------------------------------------------------------------------

void SiiDrvRxAudioMixGcpPassThroughConfig(bool_t qOn)
{
    SiiRegModify( REG_TX_AUD_MIX_CTRL0, BIT_TX_AUD_MIX_DROP_GCP,
                    (qOn ? 0: BIT_TX_AUD_MIX_DROP_GCP) );
}

//-------------------------------------------------------------------------------------------------
//! @brief      drop VSIF packet.
//!
//! @param[in]
//-------------------------------------------------------------------------------------------------

void SiiDrvRxAudioMixVsifPassThroughConfig(bool_t qOn)
{
    SiiRegModify( REG_TX_AUD_MIX_CTRL1, BIT_TX_AUD_MIX_DROP_VSIF,
                    (qOn ? 0: BIT_TX_AUD_MIX_DROP_VSIF) );
}

