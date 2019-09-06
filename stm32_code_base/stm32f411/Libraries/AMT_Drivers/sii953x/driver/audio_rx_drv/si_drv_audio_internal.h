//***************************************************************************
//!file     si_drv_cbus_internal.h
//!brief    Silicon Image CBUS Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_AUDIO_EX_INTERNAL_H__
#define __SI_DRV_AUDIO_EX_INTERNAL_H__
#include "si_device_config.h"
#include "si_drv_audio.h"

// Internal only
void    SiiDrvRxAudioGetIntStatus(uint8_t *pStatus);
void    SiiDrvRxAudioClearIntStatus(uint8_t *pStatus);
void    SiiDrvRxAudioInit ( void );
uint8_t SiiDrvRxAudioIsInterruptAssert();
void    SiiDrvRxAudioInterruptClearAssert();
uint8_t SiiDrvRxAudioGetMclk();
void    SiiDrvRxAudioUnmuteReady(bool_t qOn);
bool_t  SiiDrvRxAudioGetNewAcpInt(void);
void    SiiDrvRxAudioOnNoAcp();
bool_t  SiiDrvRxAudioGetNoAcpInt(void);
void    SiiDrvRxAudioOnNewAcp(bool_t qOn);
void    SiiDrvRxAudioNoAcpPacketcheckStart(bool_t qOn);
bool_t  SiiDrvRxAudioNoAcpPacketcheckConfirm(void);
bool_t  SiiDrvRxAudioAcpCheckDisturbance(void);

void    SiiDrvRxAudioInternalMute(bool_t qOn);

#endif // __SI_DRV_AUDIO_EX_INTERNAL_H__
