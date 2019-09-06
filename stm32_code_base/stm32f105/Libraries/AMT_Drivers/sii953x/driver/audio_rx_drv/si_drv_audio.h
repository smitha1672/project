/******************************************************************************/
//!file     si_drv_audio.h
//!brief    Audio Driver functions
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2007-2012, Silicon Image, Inc.  All rights reserved.
/******************************************************************************/

#ifndef __SI_DRV_AUDIO_H__
#define __SI_DRV_AUDIO_H__
#include "si_common.h"


#define SII_AUD_RX_MAIN			0
#define SII_AUD_RX_SUB			1

//------------------------------------------------------------------------------
// Audio Output Configuration
//------------------------------------------------------------------------------
typedef enum
{
    SII_AUD_DSD_MODE,
	SII_AUD_HBRA_MODE,
    SII_AUD_PCM_MODE,
	SII_AUD_POWER_DOWN,
} SiiRxAudioModes_t;

typedef enum
{
    SII_AUD_TWO_CHANNEL,
    SII_AUD_MULTI_CHANNEL,
} SiiRxAudioLayouts_t;

typedef enum
{
	SII_AUD_MAIN_MULTI_CHANNEL,
	SII_AUD_SUB_MULTI_CHANNEL,
	SII_AUD_PIN_EXTRACTION,
	SII_AUD_PIN_INSERTION,
	SII_AUD_NORMAL,
	SII_AUD_SWAP, 
} SiiRxAudioControl_t;

typedef enum
{
	SII_AUD_CHST4_FS_22			= 0x04,
	SII_AUD_CHST4_FS_24			= 0x06,
	SII_AUD_CHST4_FS_32			= 0x03,
	SII_AUD_CHST4_FS_44			= 0x00,
	SII_AUD_CHST4_FS_48			= 0x02,
	SII_AUD_CHST4_FS_88			= 0x08,
	SII_AUD_CHST4_FS_96			= 0x0A,
	SII_AUD_CHST4_FS_176		= 0x0C,
	SII_AUD_CHST4_FS_192		= 0x0E,
	SII_AUD_CHST4_FS_768		= 0x09,
	SII_AUD_CHST4_FS_UNKNOWN    = 0x01,
}SiiRxAudioChst4FsValues_t;

typedef enum
{
	SII_AUD_FS_32,
	SII_AUD_FS_44_1,
	SII_AUD_FS_48,
	SII_AUD_FS_88_2,
	SII_AUD_FS_96,
	SII_AUD_FS_176_4,
	SII_AUD_FS_192,
	SII_AUD_FS_MAX,
} SiiRxAudioFsValues_t;

typedef enum
{
    SII_AUD_MCLK_128,       // Fm = 128*Fs
    SII_AUD_MCLK_256,       // Fm = 256*Fs
    SII_AUD_MCLK_384,       // Fm = 384*Fs
    SII_AUD_MCLK_512,       // Fm = 512*Fs
} SiiRxAudioMclk_t;

//------------------------------------------------------------------------------
// Global Status Structure
//------------------------------------------------------------------------------

typedef struct
{
	uint8_t		code_value; // corresponding audio status Fs code
	uint16_t	ref_Fs; // reference Fs frequency in 100 Hz units
	uint16_t	min_Fs; // minimum Fs frequency in 100 Hz units
	uint16_t	max_Fs; // maximum Fs frequency in 100 Hz units
}
audio_fs_search_t;

#define AUDIO_FS_LIST_LENGTH 9
static ROM const audio_fs_search_t audio_fs_list[AUDIO_FS_LIST_LENGTH+1] =
{
	{ SII_AUD_CHST4_FS_22,		220,	200,	230  },
	{ SII_AUD_CHST4_FS_24,		240,	230,	280  },
	{ SII_AUD_CHST4_FS_32,		320,	280,	380  },
	{ SII_AUD_CHST4_FS_44,		441,	380,	460  },
	{ SII_AUD_CHST4_FS_48,		480,	460,	540  },
	{ SII_AUD_CHST4_FS_88,		882,	820,	921  },
	{ SII_AUD_CHST4_FS_96,		960,	921,	1100 },
	{ SII_AUD_CHST4_FS_176,		1764,	1600,	1792 },
	{ SII_AUD_CHST4_FS_192,		1920,	1792,	2500 },
	{ SII_AUD_CHST4_FS_UNKNOWN,	0,    	0,		0 }

};

uint8_t SiiDrvRxAudioGetMclk();
void    SiiDrvRxAudioMute(bool_t isMuted);
void    SiiDrvRxAudioSoftMute(bool_t isMuted);
bool_t  SiiDrvRxAudioInstanceSet ( uint_t instanceIndex );
uint8_t SiiDrvRxAudioModeGet(void);
bool_t SiiDrvRxAudioHdmiModeGet();   //Bug 31751 - Audio extraction is disabled and not resumed after receiving glitch in the audio
uint8_t SiiDrvRxAudioLayoutGet();
void    SiiDrvRxAudioIntMaskEnable( bool_t isEnable );
bool_t  SiiDrvRxAudioStandby( void );
uint8_t SiiDrvRxAudioSampleRateGet();
void    SiiDrvRxAudioFifoRedirSet(bool_t isTxInsertion);
void    SiiDrvRxAudioPllConfig(bool_t isOvrd);
void    SiiDrvRxAudioChstGet(uint8_t *pChst);

// Sony configuration
void SiiDrvRxAudioInitMclk(void);
void SiiDrvRxAudioSetMclk(uint8_t audMode, uint8_t audFs);

void SiiDrvRxAudioEnableAAC(void);
void SiiDrvRxAudioDisableAAC(void);
void SiiDrvRxAudioEnableCHSTINT(void);
void SiiDrvRxAudioDisableCHSTINT(void);

uint8_t SiiDrvRxAudioCheckFsAndFix ( void );

// Unused
void    SiiDrvRxAudioReset(uint8_t);
void    SiiDrvRxAudioACRControl(uint8_t bitFields, uint8_t ctrl);
//void    SiiDrvRxAudioControl ( SiiRxAudioControl_t audSel );
void    SiiDrvRxSpdifOutEnable(bool_t isEnabled);
uint8_t SiiDrvRxAudioInstanceGet ( void );
void    SiiDrvRxAudioVsifCheckedByOthers(void);

// Called from main driver
void    SiiDrvRxAudioProcessInterrupts ( void );



#endif      // __SI_DRV_AUDIO_H__
