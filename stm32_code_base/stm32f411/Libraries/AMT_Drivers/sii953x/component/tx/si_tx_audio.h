//************************************************************************************************
//! @file   si_tx_audio.h
//! @brief  Audio control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_AUDIO_H
#define SI_TX_AUDIO_H

//-------------------------------------------------------------------------------------------------
// TX Enums and other typedefs
//-------------------------------------------------------------------------------------------------

                                //! Audio Source Group type (related to I2S bus)
typedef enum txAudioSrcGrp
{
    TX_AUDIO_SRC_GRP_I2S     = 0x0,  //!< I2S group
    TX_AUDIO_SRC_GRP_DSD     = 0x1,  //!< DSD group
    TX_AUDIO_SRC_GRP_HBR     = 0x2,  //!< HBR audio group
    TX_AUDIO_SRC_GRP_SIZE

} txAudioSrcGrp_t;


//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

typedef struct TxAudioInstanceRecord
{
    bit_fld_t           isAudioContentProtected: 1;     //!< Audio Content Protection flag (ACP)
    bit_fld_t           isAudioEnabled: 1;              //!< Indicates active transmission of audio
    bit_fld_t           isDownSampled: 1;               //!< Stereo LPCM downsampling required

    txAudioSrc_t        audioSrcMode;                   //!< Audio Source type
    audioFormat_t       audioFormat;                    //!< Format of an audio content

    //!< Configurations for I2S-type interface variations
    txAudioSrcConfig_t  audioInterface[TX_AUDIO_SRC_GRP_SIZE];

} TxAudioInstanceRecord_t;



//-------------------------------------------------------------------------------------------------
//  TX Audio API functions
//-------------------------------------------------------------------------------------------------
#if SII_NUM_TX > 1
void        TxAudioInstanceSet(uint8_t deviceIndex);
#endif

void        TxAudioInit(void);
void        TxAudioI2SConfigSet(txAudioSrc_t source, txAudioSrcConfig_t *pInterfaceSettings);
void        TxAudioModeSet(txAudioSrc_t audioSourceMode, audioFormat_t *pFmt);
void        TxAudioMute(bool_t isMute);
void        TxAudioOnAcpPacketUpdate(bool_t isAudioProtected);
void        TxAudioOnHdmiModeChange(void);
bool_t      TxAudioIsTransmitting(void);
void        TxAudioUpdateFromInfoFrame(uint8_t *pAif);



#endif // SI_TX_AUDIO_H
