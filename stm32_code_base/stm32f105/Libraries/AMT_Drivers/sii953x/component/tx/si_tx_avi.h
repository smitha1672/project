//************************************************************************************************
//! @file   si_tx_avi.h
//! @brief  Auxiliary Video Info frame control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_AVI_H
#define SI_TX_AVI_H

//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

//! CEA861D AVI InfoFrame partial data structure
typedef struct txAviInstRec
{
    uint8_t dataByte1;
    uint8_t dataByte2;
    uint8_t dataByte3;
    uint8_t dataByte4;
    uint8_t dataByte5;

} txAviInstRec_t;

//-------------------------------------------------------------------------------------------------
//  TX AVI API functions
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void                TxAviInstanceSet(uint8_t deviceIndex);
#endif

void                TxAviInit(void);
void                TxAviPostAviPacket(void);
void                TxAviColorSpaceSet(txVbusColSpace_t colorSpace);
txVbusColSpace_t    TxAviColorSpaceGet(void);
uint8_t             TxAviRepetitionGet(uint8_t tx_video_index);
txVbusColorimetry_t TxAviColorimetryGet(void);
void                TxAviFrameCreate(videoFormatId_t videoFmtIndex, txVideoBusMode_t *pBusMode);

#endif // SI_TX_AVI_H
