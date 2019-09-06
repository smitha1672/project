//************************************************************************************************
//! @file   si_edid_tx_component.h
//! @brief  Silicon Image EDID Tx Component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/

#ifndef SI_EDIDTX_COMPONENT_H
#define SI_EDIDTX_COMPONENT_H

#include "string.h"
#include "si_common.h"

#define MHL_EXTRA           ENABLE     // extra MHL features
#define EDID_MHL_3D_SUPPORT			ENABLE
#define EDID_MHL_WRITE_BURST_LENGTH	16
#define EDID_3D_VIC_TOT_ENT			5		// Total # of WRITE BURSTS needed for sending VIC blocks
											// 1 WRITE BURST can carry 10 data bytes
#define EDID_3D_VIC_TABLE_LEN       (EDID_MHL_WRITE_BURST_LENGTH * EDID_3D_VIC_TOT_ENT)
#define EDID_3D_DTD_TOT_ENT			2		// Total # of WRITE BURSTS needed for sending DTD blocks
											// 1 WRITE BURST can carry 10 data bytes
#define EDID_3D_DTD_TABLE_LEN       (EDID_MHL_WRITE_BURST_LENGTH * EDID_3D_DTD_TOT_ENT)

//------------------------------------------------------------------------------
//  Manifest Constants
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//  SII_EDID_TX_OPTIONS - Specifies the options desired for the EDID component
//------------------------------------------------------------------------------

//! @defgroup TX_EDID_VIDEO_CFG_MASKS  EDID Video Configuration masks
//! @{
#define SI_TX_EDID_VIDEO_CFG_BYPASS_SCALER                  0x0001 //!< Set when video scaler is not used
#define SI_TX_EDID_VIDEO_CFG_ALLOW_PC_MODES_NO_SCALER       0x0002 //!< Set when PC video modes are allowed without scaler
#define SI_TX_EDID_VIDEO_CFG_ALLOW_BAD_CSUM_FOR_BLOCK0      0x0004 //!< Set to ignore invalid checksum (some TVs may have bad check sum in block 0)
#define SI_TX_EDID_VIDEO_CFG_ENABLE_REMEDIES                0x0008 //!< Set to enable detection of legacy/incompliant devices
#define SI_TX_EDID_VIDEO_CFG_ENABLE_XVYCC                   0x0010 //!< Set to enable extended gamut color space
#define SI_TX_EDID_VIDEO_CFG_ENABLE_3D                      0x0020 //!< Set to allow 3D video formats
#define SI_TX_EDID_VIDEO_CFG_ENABLE_MHL_LIMITS              0x0040 //!< MHL limitations will apply
#define SI_TX_EDID_VIDEO_CFG_DC30_36                        0x0080 //!< Set if repeater is able to support Deep Color 30 and 36 bit
#define SI_TX_EDID_VIDEO_CFG_DC_OVRD                        0x0100 //!< Set if repeater overrides DC settings of DS
//
//! @}

//! @defgroup TX_EDID_REMEDY_MASKS       EDID Remedy masks
//! @{
#define SI_TX_EDID_REMEDY_DO_NOT_USE_YCBCR      0x0001  //!< For TVs having problem with YCbCr processing
#define SI_TX_EDID_REMEDY_DO_NOT_USE_CP_MUTE    0x0002  //!< Do not use HDMI mute packets
#define SI_TX_EDID_REMEDY_DENY_SUPPORT          0x0004  //!< To block a device
#define SI_TX_EDID_REMEDY_DVI_PC_RANGE          0x0008  //!< For DVI TVs which prefer 0...255 range (instead of 16...235) for non-861 video
#define SI_TX_EDID_REMEDY_DELAY_DS_AUTH         0x0010  //!< For devices which have a problem with authentication started right after video clock starts coming.
#define SI_TX_EDID_REMEDY_SP_TEST               0x0020  //!< SP test equipment detected
#define SI_TX_EDID_REMEDY_TEST                  0x8000  //!< For test purposes
//
//! @}


//! @defgroup TX_EDID_AUDIO_CFG_MASKS    EDID Audio Configuration masks
//! @{
#define SI_TX_EDID_AUDIO_CFG_BYPASS                         0x0001 //!< Set to bypass audio to downstream (repeater don't play audio)
#define SI_TX_EDID_AUDIO_CFG_SPDIF_INPUT                    0x0002 //!< Set to disable audio formats that can't be carried over SPDIF
//
//! @}

//! @defgroup TX_EDID_MISC_CFG_MASKS    EDID Miscellaneous Configuration masks
//! @{
#define SI_TX_EDID_CFG_OPTIONS_FORCE_DVI_INPUT              0x0001 //!< Set to force DVI mode in generated EDID
#define SI_TX_EDID_CFG_OPTIONS_SOUNDBAR_MODE                0x0002 //!< Combine TX audio modes and TV video modes (experimental)
//
//! @}


//! TX EDID error codes for lastErrorCode
typedef enum txEdidErrCode
{
    SI_TX_EDID_ERROR_CODE_NO_ERROR                 = 0x00, //!< Success
    SI_TX_EDID_ERROR_CODE_INSTANCE_NOT_EXIST       = 0x01, //!< Non-existing instance record index provided
    SI_TX_EDID_ERROR_CODE_BAD_INPUT_INDEX          = 0x02, //!< Non-existing EDID TX input is selected
    SI_TX_EDID_ERROR_CODE_NO_US_EDID_CREATED       = 0x03, //!< No upstream EDID created because all inputs are inactive

} txEdidErrCode_t;


//------------------------------------------------------------------------------
//  EdidTx type definitions
//------------------------------------------------------------------------------

                                            //! Supported features
typedef enum edidFeatureSupport
{
    SI_EDID_TX_SUPPORT_HDMI,                //!< HDMI (not DVI)
    SI_EDID_TX_SUPPORT_AI,                  //!< Audio content protection features
    SI_EDID_TX_SUPPORT_AUDIO,               //!< Audio playback capability
    SI_EDID_TX_SUPPORT_YCbCr,
    SI_EDID_TX_SUPPORT_NTSC,
    SI_EDID_TX_SUPPORT_PAL,
    SI_EDID_TX_SUPPORT_AR_16x9,             //!< Aspect ratio is 16x9
    SI_EDID_TX_SUPPORT_AR_4x3,              //!< aspect ratio is 4x3
    SI_EDID_TX_SUPPORT_ENCODED_AUDIO,       //!< Any audio except PCM
    SI_EDID_TX_SUPPORT_1BIT_AUDIO,          //!< DSD Audio
    SI_EDID_TX_SUPPORT_HBR_AUDIO,           //!< HBR Audio
    SI_EDID_TX_SUPPORT_MC_PCM_AUDIO,        //!< PCM Multichannel Audio
    SI_EDID_TX_SUPPORT_PCM_FS_88_KHZ,
    SI_EDID_TX_SUPPORT_PCM_FS_96_KHZ,
    SI_EDID_TX_SUPPORT_PCM_FS_176_KHZ,
    SI_EDID_TX_SUPPORT_PCM_FS_192_KHZ,
    SI_EDID_TX_SUPPORT_DEEP_COLOR_30_BIT,
    SI_EDID_TX_SUPPORT_DEEP_COLOR_36_BIT,
    SI_EDID_TX_SUPPORT_DEEP_COLOR_48_BIT,
    SI_EDID_TX_SUPPORT_DEEP_COLOR_YCbCr_444,
    SI_EDID_TX_SUPPORT_XVYCC_P0,            //!< xvYCC profile P0
    SI_EDID_TX_SUPPORT_3D,

} edidFeature_t;

                                            //! Aspect Ratios
typedef enum aspectRatio PACKED
{
    SI_EDID_TX_ASPECT_RATIO_UNKNOWN = 0,
    SI_EDID_TX_ASPECT_RATIO_4x3,
    SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9

} aspectRatio_t;


//------------------------------------------------------------------------------
//  EDIT TX component functions
//------------------------------------------------------------------------------

uint8_t         SiiEdidTxInstanceGet(void);
void            SiiEdidTxInstanceSet(uint8_t instance);
bool_t          SiiEdidTxConfigure ( uint8_t  numOfTx, uint16_t videoFeatureMask, uint16_t audioFeatureMask, uint16_t miscOptionsMask);
bool_t          SiiEdidTxInitialize(void);
uint16_t        SiiEdidTxStatusGet(void);
bool_t          SiiEdidTxStandby(void);
bool_t          SiiEdidTxResume(void);
txEdidErrCode_t SiiEdidTxGetLastResult(void);
uint16_t        SiiEdidTxCecPhysAddrGet(void);

void            SiiEdidTxProcessEdid(uint8_t input);
void            SiiEdidTxDismissEdid(uint8_t input);
bool_t          SiiEdidTxInputSet(uint8_t input);

bool_t          SiiEdidTxDoesDsSupport(edidFeature_t feature);
uint16_t        SiiEdidTxDsMaxTmdsClkGet(void);
uint16_t        SiiEdidTxDsDeviceRemedyMaskGet(void);
uint8_t         SiiEdidTxOutVideoModeRankGet(uint8_t idx, aspectRatio_t aspRatio);
bool_t          SiiEdidTxBestDsVideoModeGet(uint8_t inIdx, aspectRatio_t inAspRatio, uint8_t *pOutIdx, aspectRatio_t *pOutAspRatio);
aspectRatio_t   SiiEdidTxScalerOutAspectRatioGet(uint8_t inIdx, aspectRatio_t inAspRatio, uint8_t outIdx);
bool_t          SiiEdidTxScalerIsOutVideoModeSupported(uint8_t idx, aspectRatio_t aspRatio);

//------------------------------------------------------------------------------
//  Callback functions to the Application level
//------------------------------------------------------------------------------

bool_t          SiiTxEdidCbBlockWrite(uint_t blockIndex, uint8_t* const pEdidBlock, uint8_t cecPhysAddrLocation);


#endif // SI_EDIDTX_COMPONENT_H
