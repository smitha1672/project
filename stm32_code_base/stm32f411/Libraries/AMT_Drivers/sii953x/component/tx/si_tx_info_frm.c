//************************************************************************************************
//! @file   si_tx_info_frm.c
//! @brief  Info frame and HDMI packet control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/


#include "si_tx_component_internal.h"
#include "si_tx_info_frm.h"
#include "si_drv_tpi_system.h"
#include "si_tx_audio.h"
#include "si_edid_tx_internal.h"


//-------------------------------------------------------------------------------------------------
// Module Instance Data
//-------------------------------------------------------------------------------------------------

txInfoRecord_t InfoInstRec[SII_NUM_TX];
txInfoRecord_t *pInfoFrm = &InfoInstRec[0];

//-------------------------------------------------------------------------------------------------
//  Local service functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Check if an InfoFrame has been loaded into chip.
//!
//! @param[in]  ifType - Info Frame type.
//-------------------------------------------------------------------------------------------------

static bool_t IsInfoFrameLoaded(infoFrameType_t ifType)
{
    switch (ifType)
    {
        case SI_INFO_FRAME_AVI:
            return pInfoFrm->isLoadedAvi;
        case SI_INFO_FRAME_AUDIO:
            return pInfoFrm->isLoadedAudio;
        case SI_INFO_FRAME_SPD:
            return pInfoFrm->isLoadedSpd;
        case SI_INFO_FRAME_MPEG:
            return pInfoFrm->isLoadedMpeg;
        case SI_INFO_FRAME_ISRC1:
            return pInfoFrm->isLoadedIsrc1;
        case SI_INFO_FRAME_ISRC2:
            return pInfoFrm->isLoadedIsrc2;
        case SI_INFO_FRAME_ACP:
            return pInfoFrm->isLoadedAcp;
        case SI_INFO_FRAME_GBD:
            return pInfoFrm->isLoadedGbd;
        case SI_INFO_FRAME_VSIF:
            return pInfoFrm->isLoadedVsif;
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set the InfoFrame loaded flag.
//!
//! @param[in]  ifType - Info Frame type.
//-------------------------------------------------------------------------------------------------

static void InfoFrameLoadedFlagSet(infoFrameType_t ifType, bool_t isLoaded)
{
    switch (ifType)
    {
        case SI_INFO_FRAME_AVI:
            pInfoFrm->isLoadedAvi = isLoaded;
            break;
        case SI_INFO_FRAME_AUDIO:
            pInfoFrm->isLoadedAudio = isLoaded;
            break;
        case SI_INFO_FRAME_SPD:
            pInfoFrm->isLoadedSpd = isLoaded;
            break;
        case SI_INFO_FRAME_MPEG:
            pInfoFrm->isLoadedMpeg = isLoaded;
            break;
        case SI_INFO_FRAME_ISRC1:
            pInfoFrm->isLoadedIsrc1 = isLoaded;
            break;
        case SI_INFO_FRAME_ISRC2:
            pInfoFrm->isLoadedIsrc2 = isLoaded;
            break;
        case SI_INFO_FRAME_ACP:
            pInfoFrm->isLoadedAcp = isLoaded;
            break;
        case SI_INFO_FRAME_GBD:
            pInfoFrm->isLoadedGbd = isLoaded;
            break;
        case SI_INFO_FRAME_VSIF:
            pInfoFrm->isLoadedVsif = isLoaded;
            break;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Identify Info Frames that require checksum to be sent as a part of a packet.
//!
//! @param[in]  ifType - Info Frame type.
//!
//! @return     indication of whether or not the checksum is required.
//-------------------------------------------------------------------------------------------------

static bool_t IsChecksumRequired(infoFrameType_t ifType)
{
    bool_t hasChecksum;

    switch (ifType)
    {
        case SI_INFO_FRAME_AVI:
        case SI_INFO_FRAME_SPD:
        case SI_INFO_FRAME_AUDIO:
        case SI_INFO_FRAME_VSIF:
            hasChecksum = true;
            break;
        default:
            hasChecksum = false;
    }
    return hasChecksum;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Calculate checksum of a packet fields.
//!
//! @param[in]  pFrame  - pointer to an InfoFrame packet.
//! @param[in]  version - InfoFrame version as per HDMI specification.
//! @param[in]  length  - number of bytes in the InfoFrame body (excluding header).
//!
//! @return     checksum byte.
//-------------------------------------------------------------------------------------------------

static uint8_t CalculateChecksum(uint8_t *pFrame, uint8_t version, uint8_t length)
{
    uint8_t i;
    uint8_t checksum = pFrame[IF_TITLE_INDEX] + version + length;

    for (i = 0; i < length; i++)
    {
        checksum += pFrame[IF_HEADER_LENGTH + i];
    }

    return (0 - checksum);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return true, if audio is being transmitted to a device ensuring
//!             audio content protection.
//-------------------------------------------------------------------------------------------------

static bool_t IsProtectedAudioTransmitting(void)
{
    return (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AI) && TxAudioIsTransmitting());
}


//-------------------------------------------------------------------------------------------------
//! @brief      Create SPD InfoFrame locally based on the data from NVRAM.
//-------------------------------------------------------------------------------------------------

static void ComposeDefaultSpdInfoFrame(uint8_t *pFrame)
{
    pFrame[IF_TITLE_INDEX] = SI_INFO_FRAME_SPD;
    pFrame[IF_VERSION_INDEX] = 1;
    pFrame[IF_LENGTH_INDEX] = IF_MIN_SPD_LENGTH;
    pFrame[IF_CHECKSUM_INDEX] = 0; // to be replaced later

    // TODO:OEM - Fill up vendor specific source product description info frame
    //pFrame[IF_DATA_INDEX] = 0;
    //pFrame[IF_DATA_INDEX + 1] = 0;
    // ...

    DEBUG_PRINT(TX_MSG_DBG, "TxInfo: SPD Created\n");
}


//-------------------------------------------------------------------------------------------------
//  TX Info Frame manipulation functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Select Tx device instance in multi-transmitter system.
//!
//! @param[in]  deviceIndex - zero based index of an instance.
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void TxInfoInstanceSet(uint8_t deviceIndex)
{
    if(deviceIndex < SII_NUM_TX)
    {
        pInfoFrm = &InfoInstRec[deviceIndex];
    }
}
#endif // SII_NUM_TX > 1


//-------------------------------------------------------------------------------------------------
//! @brief      Initialization of Info Frame module.
//-------------------------------------------------------------------------------------------------

void TxInfoInit(void)
{
    memset(pInfoFrm, 0, sizeof(txInfoRecord_t));
    // Make sure all info packets aren't being sent
    TxInfoAllPacketsStop();
}


//-------------------------------------------------------------------------------------------------
//! @brief      Enable/Disable transmission of Info frames that must be already loaded to the chip.
//!
//! @param[in]  ifType    - InfoFrame type,
//! @param[in]  isEnabled - true, if the InfoFrame has to be enabled,
//!                         false, otherwise.
//-------------------------------------------------------------------------------------------------

void TxInfoPacketTransmissionEnable(infoFrameType_t ifType, bool_t isEnabled)
{
    // Enabling requires that frame has been loaded,
    // disabling does not.
    if (!isEnabled || IsInfoFrameLoaded(ifType))
    {
        SiiDrvTpiInfoFrameEnable(isEnabled, false, ifType);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Terminate sending all InfoFrames/Packets.
//-------------------------------------------------------------------------------------------------

void TxInfoAllPacketsStop(void)
{
    SiiDrvTpiInfoFrameEnable(false, true, 0); // disable all
}


//-------------------------------------------------------------------------------------------------
//! @brief      Load InfoFrame/Packet data.The frame transmission state remains intact
//!             (enabled or disabled).
//!
//!             Exception is the AVI InfoFrame that is enabled all the time.
//!             For Info Frame types, pFrame should be a pointer to
//!             a structure starting with 4 byte header.
//!
//! @param[in]  ifType - InfoFrame type,
//! @param[in]  pFrame - pointer to the InfoFrame data.
//!
//! @retval     true   - loaded successfully,
//! @retval     false  - failed to load the frame.
//-------------------------------------------------------------------------------------------------

bool_t TxInfoPacketSet(infoFrameType_t ifType, uint8_t *pFrame)
{
    bool_t  isSuccess = true;
    uint8_t checksum;
    uint8_t frameLength;
    uint8_t aSpdFrameBuf[IF_HEADER_LENGTH + IF_MIN_SPD_LENGTH];

    // Only re-enable locked IFs and quit
    if (TxIsInfoFrameLocked(ifType))
    {
        SiiDrvTpiInfoFrameEnable(true, false, ifType);
        return true;
    }

    // Check pointer validity and try to create default IF,
    // when the pointer is NULL, if possible
    if (!pFrame)
    {
        if (ifType == SI_INFO_FRAME_SPD)
        {
            // (pFrame == NULL) means a request of using default SPD packet
            ComposeDefaultSpdInfoFrame(aSpdFrameBuf);
            pFrame = aSpdFrameBuf;
        }
        else
        {
            return false; // failure to provide valid pointer
        }
    }

    // Verify if downstream device supports the InfoFrame,
    // do specific actions for a particular type of InfoFrames
    switch (ifType)
    {
        case SI_INFO_FRAME_AVI:
            // AVI is a specially treated case
            checksum = CalculateChecksum(pFrame, 2, IF_MIN_AVI_LENGTH);
            SiiDrvTpiAviFrameStart(&pFrame[IF_DATA_INDEX], IF_MIN_AVI_LENGTH, checksum);
            pInfoFrm->isLoadedAvi = true;
            pFrame = NULL; // avoid further processing
            isSuccess = true;
            break;

        case SI_INFO_FRAME_ACP:
            if (!IsProtectedAudioTransmitting())
            {
                pFrame = NULL;
                isSuccess = false;
            }
            break;

        case SI_INFO_FRAME_GBD:
            if (!SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_XVYCC_P0))
            {
                pFrame = NULL;
                isSuccess = false;
            }
            break;

        case SI_INFO_FRAME_VSIF:
            if (!SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_3D))
            {
                pFrame = NULL;
                isSuccess = false;
            }
            break;

        case SI_INFO_FRAME_ISRC1:
        case SI_INFO_FRAME_ISRC2:
            if (!IsProtectedAudioTransmitting())
            {
                // Disable transmission as ISRC frames shall be stopped
                // if Audio protection is not supported by DS device
                SiiDrvTpiInfoFrameEnable(false, false, ifType);
            }
            break;
        default:
            break;
    }

    // Load InfoFrame to the chip
    if (pFrame)
    {
        if (IsChecksumRequired(ifType))
        {
            // checksum field is present in the packet
            frameLength = pFrame[IF_LENGTH_INDEX];
            checksum = CalculateChecksum(pFrame, pFrame[IF_VERSION_INDEX], frameLength);
            pFrame[IF_CHECKSUM_INDEX] = checksum;
            SiiDrvTpiInfoFrameLoad(ifType, pFrame, frameLength, true);
        }
        else
        {
            // If checksum is not used, assume that provided frame has
            // complete set of bytes (including stuffing bytes) up to the maximum size
            frameLength = IF_MAX_LENGTH;
            SiiDrvTpiInfoFrameLoad(ifType, pFrame, frameLength, false);
        }

        InfoFrameLoadedFlagSet(ifType, true); // Mark this InfoFrame as loaded
    }

    return isSuccess;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Mute AV content display on sink device.
//!
//! @param[in]  isMuted - true, if AV shall be muted.
//-------------------------------------------------------------------------------------------------

void TxInfoAvMute(bool_t isMuted)
{
	SiiDrvTpiAvMuteSourceSet( true );
    SiiDrvTpiAvMuteSendingEnable(isMuted);
    pInfoFrm->isAvMuted = isMuted;
    //DEBUG_PRINT(TX_MSG_DBG, "AV %s\n", isMuted ? "MUTED" : "UNMUTED");
}


//-------------------------------------------------------------------------------------------------
//! @brief      Update all loaded InfoFrames as they could have been filled
//!             while the chip was in power down.
//!
//!             AVI packet is not updated by this function;
//!             AVI update occurs in TxAviInfoFrameUpdate().
//!             Updating of the audio buffer is not needed as it is done
//!             in TxAudioOnHdmiModeChange().
//-------------------------------------------------------------------------------------------------

void TxAllInfoFramesUpdate(void)
{
    uint8_t i = 0;
    uint8_t framesToUpdate[] =
    {
            SI_INFO_FRAME_AVI,
            SI_INFO_FRAME_SPD,
            SI_INFO_FRAME_AUDIO,
            SI_INFO_FRAME_MPEG,
            SI_INFO_FRAME_ISRC1,
            SI_INFO_FRAME_ISRC2,
            SI_INFO_FRAME_ACP,
            SI_INFO_FRAME_GBD,
            SI_INFO_FRAME_VSIF,
            0                       // marker of list end
    };

    while (framesToUpdate[i])
    {
        if (IsInfoFrameLoaded(framesToUpdate[i])) // skip unloaded frames
        {
            SiiDrvTpiInfoFrameEnable(true, false, framesToUpdate[i]);
        }
        i++;
    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Update AVI info frame in the chip to apply any changes made in AVI structure.
//-------------------------------------------------------------------------------------------------

void TxAviInfoFrameUpdate(void)
{
    SiiDrvTpiAviFrameUpdate();
}


//-------------------------------------------------------------------------------------------------
//! @brief      Check if an InfoFrame has been locked (no implicit modification allowed)
//!
//! @param[in]  ifType - Info Frame type.
//-------------------------------------------------------------------------------------------------

bool_t TxIsInfoFrameLocked(infoFrameType_t ifType)
{
    switch (ifType)
    {
        case SI_INFO_FRAME_AVI:
            return pInfoFrm->isLockedAvi;
        case SI_INFO_FRAME_AUDIO:
            return pInfoFrm->isLockedAudio;
        case SI_INFO_FRAME_SPD:
            return pInfoFrm->isLockedSpd;
        case SI_INFO_FRAME_MPEG:
            return pInfoFrm->isLockedMpeg;
        case SI_INFO_FRAME_ISRC1:
            return pInfoFrm->isLockedIsrc1;
        case SI_INFO_FRAME_ISRC2:
            return pInfoFrm->isLockedIsrc2;
        case SI_INFO_FRAME_ACP:
            return pInfoFrm->isLockedAcp;
        case SI_INFO_FRAME_GBD:
            return pInfoFrm->isLockedGbd;
        case SI_INFO_FRAME_VSIF:
            return pInfoFrm->isLockedVsif;
    }

    return false;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set the InfoFrame locking flag.
//!
//! @param[in]  ifType - Info Frame type.
//-------------------------------------------------------------------------------------------------

void TxInfoFrameLockSet(infoFrameType_t ifType, bool_t isLocked)
{
    switch (ifType)
    {
        case SI_INFO_FRAME_AVI:
            pInfoFrm->isLockedAvi = isLocked;
            break;
        case SI_INFO_FRAME_AUDIO:
            pInfoFrm->isLockedAudio = isLocked;
            break;
        case SI_INFO_FRAME_SPD:
            pInfoFrm->isLockedSpd = isLocked;
            break;
        case SI_INFO_FRAME_MPEG:
            pInfoFrm->isLockedMpeg = isLocked;
            break;
        case SI_INFO_FRAME_ISRC1:
            pInfoFrm->isLockedIsrc1 = isLocked;
            break;
        case SI_INFO_FRAME_ISRC2:
            pInfoFrm->isLockedIsrc2 = isLocked;
            break;
        case SI_INFO_FRAME_ACP:
            pInfoFrm->isLockedAcp = isLocked;
            break;
        case SI_INFO_FRAME_GBD:
            pInfoFrm->isLockedGbd = isLocked;
            break;
        case SI_INFO_FRAME_VSIF:
            pInfoFrm->isLockedVsif = isLocked;
            break;
    }
}
