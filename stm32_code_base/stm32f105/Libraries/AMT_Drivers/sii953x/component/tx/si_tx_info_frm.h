//************************************************************************************************
//! @file   si_tx_info_frm.h
//! @brief  Info frame and HDMI packet control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#ifndef SI_TX_INFO_FRM_H
#define SI_TX_INFO_FRM_H

//-------------------------------------------------------------------------------------------------
//  Module Instance Data
//-------------------------------------------------------------------------------------------------

typedef struct txInfoRecord
{
    bit_fld_t isAvMuted     :1;

    // Flags indicating that IF has been loaded in a chip
    bit_fld_t isLoadedAvi   :1;
    bit_fld_t isLoadedAudio :1;
    bit_fld_t isLoadedSpd   :1;
    bit_fld_t isLoadedMpeg  :1;
    bit_fld_t isLoadedIsrc1 :1;
    bit_fld_t isLoadedIsrc2 :1;
    bit_fld_t isLoadedAcp   :1;
    bit_fld_t isLoadedGbd   :1;
    bit_fld_t isLoadedVsif  :1;

    // Flags indicating that IF auto composition is blocked by explicit setting of the IF
    bit_fld_t isLockedAvi   :1;
    bit_fld_t isLockedAudio :1;
    bit_fld_t isLockedSpd   :1;
    bit_fld_t isLockedMpeg  :1;
    bit_fld_t isLockedIsrc1 :1;
    bit_fld_t isLockedIsrc2 :1;
    bit_fld_t isLockedAcp   :1;
    bit_fld_t isLockedGbd   :1;
    bit_fld_t isLockedVsif  :1;

} txInfoRecord_t;

//-------------------------------------------------------------------------------------------------
//  TX AVI API functions
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void    TxInfoInstanceSet(uint8_t device_idx);
#endif

void    TxInfoInit(void);
void    TxInfoPacketTransmissionEnable(infoFrameType_t ifType, bool_t isEnabled);
void    TxInfoAllPacketsStop(void);
bool_t  TxInfoPacketSet(infoFrameType_t title, uint8_t *p_data);
void    TxInfoAvMute(bool_t isMuted);
void    TxAllInfoFramesUpdate(void);
void    TxAviInfoFrameUpdate(void);
bool_t  TxIsInfoFrameLocked(infoFrameType_t ifType);
void    TxInfoFrameLockSet(infoFrameType_t ifType, bool_t isLocked);



#endif // SI_TX_INFO_FRM_H
