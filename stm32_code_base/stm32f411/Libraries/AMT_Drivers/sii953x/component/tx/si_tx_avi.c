//************************************************************************************************
//! @file   si_tx_avi.c
//! @brief  Auxiliary Video Info frame control
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#include <string.h>
#include "si_tx_component_internal.h"
#include "si_tx_info_frm.h"
#include "si_tx_avi.h"


//-------------------------------------------------------------------------------------------------
// Module Instance Data
//-------------------------------------------------------------------------------------------------

txAviInstRec_t AviInstRec[SII_NUM_TX];
txAviInstRec_t *pAvi = &AviInstRec[0];


//-------------------------------------------------------------------------------------------------
//  Local service functions
//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//  AVI API functions
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//! @brief      Select Tx device instance in multi-transmitter system.
//!
//! @param[in]  device_idx - zero based index of an instance.
//-------------------------------------------------------------------------------------------------

#if SII_NUM_TX > 1
void TxAviInstanceSet(uint8_t deviceIndex)
{
	if(deviceIndex < SII_NUM_TX)
	{
		pAvi = &AviInstRec[deviceIndex];
	}
}
#endif // SII_NUM_TX > 1


//-------------------------------------------------------------------------------------------------
//! @brief      Initialization of TX AVI module.
//-------------------------------------------------------------------------------------------------

void TxAviInit(void)
{
    memset(pAvi, 0, sizeof(AviInstRec[0]));
}


//-------------------------------------------------------------------------------------------------
//! @brief      Submit AVI packet (header + info frame) into TX chip.
//-------------------------------------------------------------------------------------------------

void TxAviPostAviPacket(void)
{
	uint8_t aviPacket[IF_MAX_AVI_LENGTH + IF_HEADER_LENGTH];

	memset(aviPacket, 0, sizeof(aviPacket));

	aviPacket[IF_TITLE_INDEX] = SI_INFO_FRAME_AVI;
	aviPacket[IF_VERSION_INDEX] = 2;
	aviPacket[IF_LENGTH_INDEX] = IF_MIN_AVI_LENGTH;
	aviPacket[IF_CHECKSUM_INDEX] = 0; // to be filled inside TxInfoPacketSet() function

	aviPacket[IF_DATA_INDEX + 0] = pAvi->dataByte1 & (~0x0C);// clear Bar Info field
	aviPacket[IF_DATA_INDEX + 1] = pAvi->dataByte2;
	aviPacket[IF_DATA_INDEX + 2] = pAvi->dataByte3;
	aviPacket[IF_DATA_INDEX + 3] = pAvi->dataByte4;
	aviPacket[IF_DATA_INDEX + 4] = pAvi->dataByte5;

	TxInfoPacketSet(SI_INFO_FRAME_AVI, aviPacket);
}

//-------------------------------------------------------------------------------------------------
//! @brief      Set color space in the AVI info frame.
//-------------------------------------------------------------------------------------------------

void TxAviColorSpaceSet(txVbusColSpace_t colorSpace)
{
	// Clear color space info
	pAvi->dataByte1 &= ~0x60;

    // Replace with the new information
    switch (colorSpace)
    {
        case SI_TX_VBUS_COLOR_SPACE_YCBCR422:
            pAvi->dataByte1 |= 0x20; // color space- YCbCr 4:2:2
            break;

        case SI_TX_VBUS_COLOR_SPACE_YCBCR444:
            pAvi->dataByte1 |= 0x40; // color space- YCbCr 4:4:4
            break;

        case SI_TX_VBUS_COLOR_SPACE_RGB:
            pAvi->dataByte1 |= 0; // RGB color space
            break;
    }

}


//-------------------------------------------------------------------------------------------------
//! @brief      Return color space id extracted from the AVI info frame.
//-------------------------------------------------------------------------------------------------

txVbusColSpace_t TxAviColorSpaceGet(void)
{
    txVbusColSpace_t colSpace;

    switch ((pAvi->dataByte1 >> 5) & 0x03)
    {
        default:
        case 0:
            colSpace = SI_TX_VBUS_COLOR_SPACE_RGB;
            break;
        case 1:
            colSpace = SI_TX_VBUS_COLOR_SPACE_YCBCR422;
            break;
        case 2:
            colSpace = SI_TX_VBUS_COLOR_SPACE_YCBCR444;
            break;
    }

	return colSpace;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return repetition factor extracted from the AVI info frame.
//-------------------------------------------------------------------------------------------------

uint8_t TxAviRepetitionGet(uint8_t videoIndex)
{
	return
		((pAvi->dataByte4) & 0x7F) ? // if VIC != 0
		((pAvi->dataByte5) & 0x0F) : // use AVI repetition field
		SiiVideoTableRepetitionGet(videoIndex); // get repetition from the table
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return colorimetry type extracted from the AVI info frame.
//-------------------------------------------------------------------------------------------------

txVbusColorimetry_t TxAviColorimetryGet(void)
{
    txVbusColorimetry_t colorimetry = (pAvi->dataByte2 >> 6) & 0x03;
	if(colorimetry == SI_TX_VIDEO_COLORIMETRY_EXTENDED)
	{
		switch((pAvi->dataByte3 >> 4) & 0x07)
		{
		case 0:
			colorimetry = SI_TX_VIDEO_COLORIMETRY_XV601;
			break;
		case 1:
			colorimetry = SI_TX_VIDEO_COLORIMETRY_XV709;
			break;
		default:
			colorimetry = SI_TX_VIDEO_COLORIMETRY_NO_INFO;
		}
	}
	return colorimetry;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Create AVI info frame based on current video format and video bus mode.
//!
//! @param[in]  videoFmtIndex - current video format Id,
//! @param[in]  vBusMode      - input video bus mode.
//-------------------------------------------------------------------------------------------------

void TxAviFrameCreate(videoFormatId_t videoFmtIndex, txVideoBusMode_t *pBusMode)
{
    TxAviInit();

    if (videoFmtIndex != SI_VIDEO_MODE_NON_STD)
    {
        const videoMode_t *pVideoMode = &(VideoModeTable[videoFmtIndex]);
        uint8_t vic_4x3 = pVideoMode->Vic4x3;
        uint8_t vic_16x9 = pVideoMode->Vic16x9;

        // VIC and Aspect Ratio
        // VIC for 3D modes corresponds to one of 2D pictures (see p.136 of HDMI spec)
        if (vic_4x3)
        {
            pAvi->dataByte4 = vic_4x3;  // video code
            pAvi->dataByte2 = 0x18;     // aspect ratio- 4:3
        }
        else if (vic_16x9)
        {
            pAvi->dataByte4 = vic_16x9; // video code
            pAvi->dataByte2 = 0x28;     // aspect ratio- 16:9
        }
        else
        {
            pAvi->dataByte2 = 0x08;     // aspect ratio- No Data
        }

        // Colorimetry
        switch (VideoModeTable[videoFmtIndex].Active.V)
        {
            case 480:
            case 576:
            case 240:
            case 288:
                pAvi->dataByte2 |= 0x40; // colorimetry ITU601
                break;
            default:
                pAvi->dataByte2 |= 0x80; // colorimetry ITU709
        }

        // Pixel repetition factor
        pAvi->dataByte5 = SiiVideoTableRepetitionGet(videoFmtIndex);
    }
    else  // The case of non-standard video
    {
        // VIC must be 0 for non-standard video modes
        // Aspect Ratio - no data
        pAvi->dataByte2 = 0x08;

        // Colorimetry
        switch (pBusMode->colorimetry)
        {
            case SI_TX_VIDEO_COLORIMETRY_ITU601:
            case SI_TX_VIDEO_COLORIMETRY_XV601:
                pAvi->dataByte2 |= 0x40;
                break;

            case SI_TX_VIDEO_COLORIMETRY_ITU709:
            case SI_TX_VIDEO_COLORIMETRY_XV709:
                pAvi->dataByte2 |= 0x80;
                break;

            case SI_TX_VIDEO_COLORIMETRY_EXTENDED:
                pAvi->dataByte2 |= 0xc0;
                break;

            default:
                pAvi->dataByte2 |= 0x0;
                break;
        }

        // Pixel Repetition factor
        pAvi->dataByte5 = pBusMode->pixelRepetition;
    }

    // Color space indication
    TxAviColorSpaceSet(pBusMode->colorSpace);
}


