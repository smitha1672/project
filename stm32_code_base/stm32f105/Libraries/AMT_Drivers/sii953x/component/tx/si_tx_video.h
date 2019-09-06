//************************************************************************************************
//! @file   si_tx_video.h
//! @brief  Video Conversion Control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_VIDEO_H
#define SI_TX_VIDEO_H

#if (SI_TX_VIDEO_CONVERSION == ENABLE)
//-------------------------------------------------------------------------------------------------
// Enums
//-------------------------------------------------------------------------------------------------

typedef enum txDataRangeConversion
{
    SI_TX_DATA_RANGE_NO_CHANGE     = 0x0,
    SI_TX_DATA_RANGE_EXPAND        = 0x1,
    SI_TX_DATA_RANGE_CONTRACT      = 0x2

} txDataRangeConversion_t;


//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

typedef struct txVideoConversionInstanceRecord
{
    txVbusColDepth_t prevColorDepth;     //!< Previous color depth value
    txVbusColSpace_t outColorSpace;      //!< Output color space (if enforced)

    bit_fld_t isDeepColorMode       : 1; //!< True if deep color mode has been set at least once
    bit_fld_t isDcResetNeeded       : 1;
    bit_fld_t isPrevColorSpaceYCbCr : 1; //!< Indicates if previous color space was YCbCr
    bit_fld_t isDitheringEnabled    : 1; //!< Color dithering mode enable flag
    bit_fld_t isForceOutColorSpace  : 1; //!< Enable explicit setting of output color space

} txVideoConversionInstanceRecord_t;


//-------------------------------------------------------------------------------------------------
//  TX Video Path API functions
//-------------------------------------------------------------------------------------------------
#if SII_NUM_TX > 1
void TxVideoConversionInstanceSet(uint8_t deviceIndex);
#endif

void TxVideoConversionInit(void);
void TxVideoConversionReset(void);
void TxVideoConversionSet(bool_t isTxOutInHdmiMode, txVideoBusMode_t *pBusMode, bool_t isUpdateAvi);
void TxInputClockEdgeConfig(bool_t latch_on_rising_edge);
void TxVideoConversionDitheringEnable(bool_t isEnabled);
void TxVideoConversionOutColorSpaceSet(txVbusColSpace_t colorSpace);
void TxVideoBlankingLevelsSet(bool_t isYCbCr, bool_t isFullRange);

#endif // (SI_TX_VIDEO_CONVERSION == ENABLE)

#endif // SI_TX_VIDEO_H
