//***************************************************************************
//! @file     si_drv_audio_mix.h
//! @brief    Audio Mixer driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef SI_DRV_AUD_MIX_H
#define SI_DRV_AUD_MIX_H


//-------------------------------------------------------------------------------------------------
// Type definitions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// API functions
//-------------------------------------------------------------------------------------------------

void SiiDrvRxAudioMixInstanceSet(uint8_t device_idx);
void SiiDrvRxAudioMixPassThroughConfig(bool_t isAudioPassEnabled, bool_t isNonAudioPassEnabled);
void SiiDrvRxAudioMixGcpPassThroughConfig(bool_t qOn);
void SiiDrvRxAudioMixVsifPassThroughConfig(bool_t qOn);
#endif // SI_DRV_AUD_MIX_H
