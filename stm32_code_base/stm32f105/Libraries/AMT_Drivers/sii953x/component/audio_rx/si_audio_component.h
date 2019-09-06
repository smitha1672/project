//***************************************************************************
//!file     si_audio_component.h
//!brief    Silicon Image Rx Audio Component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_EX_AUDIO__COMPONENT_H__
#define __SI_EX_AUDIO__COMPONENT_H__
#include "si_audio_rx_config.h"
#include "si_drv_audio.h"

//------------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------------

#define SII_NOACP_TIMEOUT       600 //ms

// Audio states
typedef enum
{
	SII_AUD_STATE_IDLE,
	SII_AUD_STATE_OFF,
	SII_AUD_STATE_ON,
	SII_AUD_NO_ACP_CHECK,
} SiiRxAudioStates_t;

enum
{
	SII_AUD_RX_FSCHG = BIT0,
	SII_AUD_RX_AUDIO_READY = BIT2,
	SII_AUD_RX_AACDONE = BIT6,	
	SII_AUD_RX_FNCHG = BIT7,
	SII_AUD_CHST_RDY = BIT5,
};

typedef struct
{
    SiiRxAudioStates_t      audState;
    SiiRxAudioModes_t       audMode;
    SiiRxAudioLayouts_t     audLayout;
    SiiRxAudioFsValues_t    audFs;
    SiiRxAudioMclk_t        audMclk;
} SiiRxAudioInstanceData_t;

void    SiiRxAudioInit(void);
bool_t  SiiRxAudioInstanceSet(int instanceIndex);
bool_t  SiiRxAudioStandby( void );
uint8_t SiiRxAudioHandler(SiiRxAudioInstanceData_t *pAudRx);
void    SiiRxAudioNoAcpCheck(void);

#endif // __SI_EX_AUDIO_COMPONENT_H__
