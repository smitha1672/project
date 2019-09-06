//************************************************************************************************
//! @file   si_edid_tx_decode_l.c
//! @brief  EDID parsing and decoding. Low-level API
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#include <string.h>
#include "si_common.h"
#include "si_drv_tpi_ddc_access.h"
#include "si_edid_tx_component.h"
#include "si_edid_tx_internal.h"
#include "si_debug.h"

#define CHECK_POLARITY_IN_DTD
#define FIX_INCORRECT_ASPECT_RATIOS

//-------------------------------------------------------------------------------------------------
//! @brief      Verify EDID block checksum.
//!
//! @retval     true  - checksum is correct,
//! @retval     false - checksum mismatch
//-------------------------------------------------------------------------------------------------


bool_t EdidTxCheckBlockCheckSum(uint8_t aBlock[EDID_BLOCK_SIZE])
{
    uint8_t check_sum = 0;
    int i;
    for (i = 0; i < (EDID_BLOCK_SIZE-1); i++)
    {
        check_sum += aBlock[i];
    }
    check_sum = 0x100 - check_sum;

    if ( aBlock[EDID_BLOCK_SIZE-1] != check_sum )
    {
        return false;
    }
    return true;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return video mode ID corresponding to provided DTD timings.
//!
//-------------------------------------------------------------------------------------------------

static int GetVideoModeIndex (dtd_t *pTiming)
{
    int i;
    for ( i=0; VideoModeTable[i].Vic4x3 || VideoModeTable[i].Vic16x9 ; i++)
    {
        uint8_t interlaced = (VideoModeTable[i].Interlaced == INTL) ? 1 : 0;
        //if ( (VideoModeTable[i].Interlaced == INTL) !=
        if ( interlaced != (pTiming->Interlaced) )
            continue;
        if ( ABS_DIFF(VideoModeTable[i].PixFreq, (pTiming->PixClk_10kHz/100)) > 1 ) // +-1MHz
            continue;
        if ( VideoModeTable[i].Active.H != pTiming->HAct_Pix )
            continue;
        if ( VideoModeTable[i].Active.V != (pTiming->VAct_Pix << interlaced) )
            continue;
        if ( VideoModeTable[i].Blank.H != pTiming->HBlank_Pix )
            continue;
        if ( VideoModeTable[i].Blank.V != pTiming->VBlank_Pix )
            continue;
        if ( VideoModeTable[i].SyncOffset.H != pTiming->HSyncOffset )
            continue;
        if ( VideoModeTable[i].SyncOffset.V != pTiming->VSyncOffset )
            continue;
        if ( VideoModeTable[i].SyncWidth.H != pTiming->HSyncPulseWith )
            continue;
        if ( VideoModeTable[i].SyncWidth.V != pTiming->VSyncPulseWith )
            continue;
#ifdef CHECK_POLARITY_IN_DTD
        {
            uint8_t interf = pTiming->InterfaceType;
            if ((interf & 0x0C) == 0x0C)
            {
                // digital interface
                if ( (VideoModeTable[i].HPol == POS) != ((pTiming->InterfaceType & 0x01) != 0) )
                    continue;
                if ( (VideoModeTable[i].VPol == POS) != ((pTiming->InterfaceType & 0x02) != 0) )
                    continue;
            }
            else
            {
                // analog interface
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DTD_VIOLATION;
#define FORBID_ANALOG_TVS
#ifdef FORBID_ANALOG_TVS
                continue;
#endif
            }
        }
#endif // CHECK_POLARITY_IN_DTD

        // border must be 0 according to the HDMI standard
        if ( (0 != pTiming->HBorder) || (0 != pTiming->VBorder) )
        {
            // Let's allow using DTD with Border field which does not match
            // the HDMI spec., but at least mark that there is a violation.
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DTD_VIOLATION;
            //continue; // uncomment it to forbid using such DTDs
        }

        // all tests passed
        return i; // return found index
    }
    return -1; // not found
}



//-------------------------------------------------------------------------------------------------
//! @brief      Parse and decode DTD block.
//!
//-------------------------------------------------------------------------------------------------

void EdidTxDecodeDtd (uint8_t *pDtd)
{
    if ( (0 == pDtd[EDID_DTD_ADR__PIX_CLK_LOW]) &&
        (0 == pDtd[EDID_DTD_ADR__PIX_CLK_HIGH]) )
    {
        // Alternative using of DTD block
        switch( pDtd[EDID_DTD_ADR__DATA_TYPE_TAG] )
        {
        case EDID_DTD_TAG__STRING:          // ASCII String
#ifdef COLLECT_EXCESSIVE_INFO
            if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.String)
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
            else
            {
                pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.String = true;
                memcpy(pEdidTx->pEdidDecodData->edidInfo.AsciiString,
                    &pDtd[DTD_DESCRIPTOR_DATA_START], DTD_DESCRIPTOR_DATA_LENGTH);
            }
#endif // COLLECT_EXCESSIVE_INFO
            break;
        case EDID_DTD_TAG__SN:              // Serial Number
#ifdef COLLECT_EXCESSIVE_INFO
            // must be only one
            if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.SerialNumber)
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DTD_VIOLATION;
            else
            {
                pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.SerialNumber = true;
                memcpy(pEdidTx->pEdidDecodData->edidInfo.AsciiSerialNumber,
                    &pDtd[DTD_DESCRIPTOR_DATA_START], DTD_DESCRIPTOR_DATA_LENGTH);
            }
#endif // COLLECT_EXCESSIVE_INFO
            break;
        case EDID_DTD_TAG__RANGE_LIMITS:    // Range Limits
            // must be only one
            if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.RangeLimits)
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DTD_VIOLATION;
            else
            {
                pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.RangeLimits = true;
                pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinV_Hz = pDtd[5];
                pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxV_Hz = pDtd[6];
                pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinH_kHz = pDtd[7];
                pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxH_kHz = pDtd[8];
                pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxPixClk_10MHz = pDtd[9];
                memcpy(pEdidTx->pEdidDecodData->edidInfo.RangeLimits.SecondaryTimingFormula,
                    &pDtd[10], sizeof(pEdidTx->pEdidDecodData->edidInfo.RangeLimits.SecondaryTimingFormula)); // 8 bytes
            }
            break;
        case EDID_DTD_TAG__NAME:            // Name
#ifdef COLLECT_EXCESSIVE_INFO
            // must be only one
            if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.Name)
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DTD_VIOLATION;
            else
            {
                pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.Name = true;
                memcpy(pEdidTx->pEdidDecodData->edidInfo.AsciiName,
                    &pDtd[DTD_DESCRIPTOR_DATA_START], DTD_DESCRIPTOR_DATA_LENGTH);
            }
#endif // COLLECT_EXCESSIVE_INFO
            break;
        case EDID_DTD_TAG__COLOR_POINT:     // Color Point
#ifdef COLLECT_EXCESSIVE_INFO
            // must be only one (???)
            if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.ColorPoint)
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DTD_VIOLATION;
            else
            {
                pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.ColorPoint = true;
                memcpy(pEdidTx->pEdidDecodData->edidInfo.ColorPoint,
                    &pDtd[DTD_DESCRIPTOR_DATA_START], DTD_DESCRIPTOR_DATA_LENGTH);
            }
#endif // COLLECT_EXCESSIVE_INFO
            break;
        case EDID_DTD_TAG__STI:             // Standard Timing Identifiers
#ifdef COLLECT_EXCESSIVE_INFO
            if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.StandardTiming)
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
            else
            {
                pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.StandardTiming = true;
                memcpy(pEdidTx->pEdidDecodData->edidInfo.StandardTimingInDtd,
                    &pDtd[DTD_DESCRIPTOR_DATA_START], DTD_DESCRIPTOR_DATA_LENGTH);
            }
#endif // COLLECT_EXCESSIVE_INFO
            break;
        case 0:
            // it seems like padding area, not like a DTD
            break;
        default:
            // if we came here, it could means that came accross
            // some unsupported code (newer than described in EDID 1.3)
            // or EDID sructure is inconsistent.
#ifdef COLLECT_EXCESSIVE_INFO
            if (pEdidTx->pEdidDecodData->edidInfo.Revision <= 3)
#else // COLLECT_EXCESSIVE_INFO
            if (pEdidTx->pEdidDecodData->Revision <= 3)
#endif // COLLECT_EXCESSIVE_INFO
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__DTD_VIOLATION;
            else
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__FOUND_UNSUPPORTED_DTD;
            break;
        }
    }
    else
    {
        // DTD is used as the Detail Timing structure
        int video_idx = -1; // index in VideoModeTable, -1 means "not found"
        dtd_t timing;
        timing.PixClk_10kHz = pDtd[EDID_DTD_ADR__PIX_CLK_LOW] |
                        (pDtd[EDID_DTD_ADR__PIX_CLK_HIGH] << 8);
        timing.HAct_Pix     = pDtd[EDID_DTD_ADR__H_ACTIVE_LOW] |
                        ((pDtd[EDID_DTD_ADR__H_HIGH] & 0xF0) << 4);
        timing.HBlank_Pix   = pDtd[EDID_DTD_ADR__H_BLANK_LOW] |
                        ((pDtd[EDID_DTD_ADR__H_HIGH] & 0x0F) << 8);
        timing.VAct_Pix     = pDtd[EDID_DTD_ADR__V_ACTIVE_LOW] |
                        ((pDtd[EDID_DTD_ADR__V_HIGH] & 0xF0) << 4);
        timing.VBlank_Pix   = pDtd[EDID_DTD_ADR__V_BLANK_LOW] |
                        ((pDtd[EDID_DTD_ADR__V_HIGH] & 0x0F) << 8);
        timing.HSyncOffset  = pDtd[EDID_DTD_ADR__H_SYNC_OFFSET_LOW] |
                        ((pDtd[EDID_DTD_ADR__H_V_SYNC_HIGH] & 0xC0) << 2);
        timing.HSyncPulseWith= pDtd[EDID_DTD_ADR__H_SYNC_WIDTH_LOW] |
                        ((pDtd[EDID_DTD_ADR__H_V_SYNC_HIGH] & 0x30) << 4);
        timing.VSyncOffset  = ((pDtd[EDID_DTD_ADR__V_SYNC_LOW] & 0xF0) >> 4) |
                        ((pDtd[EDID_DTD_ADR__H_V_SYNC_HIGH] & 0x0C) << 2);
        timing.VSyncPulseWith= (pDtd[EDID_DTD_ADR__V_SYNC_LOW] & 0x0F) |
                        ((pDtd[EDID_DTD_ADR__H_V_SYNC_HIGH] & 0x03) << 4);
        timing.HImageSize   = pDtd[EDID_DTD_ADR__H_SIZE_LOW] |
                        ((pDtd[EDID_DTD_ADR__H_V_SIZE_HIGH] & 0xF0) << 4);
        timing.VImageSize   = pDtd[EDID_DTD_ADR__V_SIZE_LOW] |
                        ((pDtd[EDID_DTD_ADR__H_V_SIZE_HIGH] & 0x0F) << 8);
        timing.HBorder      = pDtd[EDID_DTD_ADR__H_BORDER];
        timing.VBorder      = pDtd[EDID_DTD_ADR__V_BORDER];

        // Flag's bit 7
        timing.Interlaced   = (pDtd[EDID_DTD_ADR__FLAGS] & 0x80) >> 7;
        // Flag's bits 0, 5, 6
        timing.DisplayType  = (pDtd[EDID_DTD_ADR__FLAGS] & 0x01) |
                        ((pDtd[EDID_DTD_ADR__FLAGS] & 0x60) >> 4);
        // Flag's bits 1, 2, 3, 4
        timing.InterfaceType    = (pDtd[EDID_DTD_ADR__FLAGS] & 0x1E) >> 1;

        video_idx = GetVideoModeIndex(&timing);
        if (-1 != video_idx)
        {
            // video mode found

            // 4/3=12/9=1.(3), 16/9= 1.(7)
            // 14/9=1.(5) (the middle)
            uint32_t aspect_ratio = ((timing.HImageSize * 9) / timing.VImageSize);
            bool_t ar16x9 = ( aspect_ratio > 14);
            // aspect ratio ar16x9: 4:3 if false, 16:9 if true

/*
CTSpec 1.3:
If DTD matches any 861-D format, search SVDs for that same video format at
same aspect ratio.
- If no matching SVD -> FAIL
*/
            uint8_t vic = 0; // CEA video code
            uint8_t hdmi_vic = VideoModeTable[video_idx].HdmiVic; // HDMI VIC

#ifdef FIX_INCORRECT_ASPECT_RATIOS
            if (ar16x9)
            {
                vic = VideoModeTable[video_idx].Vic16x9; // according to CEA-861B/C/D
                if (0 == vic) // attempt to correct wrong aspect ratio
                {
                    ar16x9 = false;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__INCORRECT_AR_IN_DTD_FIX;
                }
            }
            else
            {
                vic = VideoModeTable[video_idx].Vic4x3; // according to CEA-861B/C/D
                if (0 == vic) // attempt to correct wrong aspect ratio
                {
                    ar16x9 = true;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__INCORRECT_AR_IN_DTD_FIX;
                }
            }

            if (0 == vic)
            {
                // first attempt was unsuccessful
                if (ar16x9)
                    vic = VideoModeTable[video_idx].Vic16x9; // preferred
                else
                    vic = VideoModeTable[video_idx].Vic4x3; // preferred
            }
            else
            {
                // the DTD is fully compliant to CEA-861C
                if (ar16x9)
                {
                    if (0 == pEdidTx->pEdidDecodData->edidInfo.VSize16x9)
                        pEdidTx->pEdidDecodData->edidInfo.VSize16x9 = timing.VImageSize;
                    pEdidTx->pEdidDecodData->edidInfo.SupportAspect16_9 = true;
                }
                else
                {
                    if (0 == pEdidTx->pEdidDecodData->edidInfo.VSize4x3)
                        pEdidTx->pEdidDecodData->edidInfo.VSize4x3 = timing.VImageSize;
                    pEdidTx->pEdidDecodData->edidInfo.SupportAspect4_3 = true;
                }
            }
#else // FIX_INCORRECT_ASPECT_RATIOS
            if (ar16x9)
            {
                vic = VideoModeTable[video_idx].Vic16x9; // preferred
                if (0 == vic)
                    vic = VideoModeTable[video_idx].Vic4x3;
                if (0 == pEdidTx->pEdidDecodData->edidInfo.VSize16x9)
                    pEdidTx->pEdidDecodData->edidInfo.VSize16x9 = timing.VImageSize;
                pEdidTx->pEdidDecodData->edidInfo.SupportAspect16_9 = true;
            }
            else
            {
                vic = VideoModeTable[video_idx].Vic4x3; // preferred
                if (0 == vic)
                    vic = VideoModeTable[video_idx].Vic16x9;
                if (0 == pEdidTx->pEdidDecodData->edidInfo.VSize4x3)
                    pEdidTx->pEdidDecodData->edidInfo.VSize4x3 = timing.VImageSize;
                pEdidTx->pEdidDecodData->edidInfo.SupportAspect4_3 = true;
            }
#endif // FIX_INCORRECT_ASPECT_RATIOS

            if (VideoModeTable[video_idx].NtscPal == PAL)
                pEdidTx->pEdidDecodData->edidInfo.SupportPal = true;
            if (VideoModeTable[video_idx].NtscPal == NTSC)
                pEdidTx->pEdidDecodData->edidInfo.SupportNtsc = true;

            // Even video mode is known, there could be some variations.
            // To avoid such an ambiguous situation, several
            // parameters are to be stored.
            {
                int i;
                for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
                {
                    if (!pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid) // if the slot is empty
                    {
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid = true; // mark the slot as occupied
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].VideoIdx = video_idx;
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Vic = vic;
#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].HdmiVic = hdmi_vic;
#endif // SI_HDMI_1_4_EDID_PROCESSING
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Ar16x9 = ar16x9;
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].DisplayType = timing.DisplayType;
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].PreferredTiming = false;
                        if (pEdidTx->pEdidDecodData->edidInfo.BasicDispayParameters[4] & 0x02) // Preferred Timing
                            if (!pEdidTx->pEdidDecodData->isNotFirstDtd)
                                pEdidTx->pEdidDecodData->edidInfo.SDTD[i].PreferredTiming = true;
                        break;
                    }
                } // for ()
                if (MAX_NMB_OF_KNOWN_DTDS == i) // no empty slots case
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
            }
        }
        else
        {
            // Video mode is not found, the DTD is for PC mode, store it as is
            int i;
            for (i=0; i<MAX_NMB_OF_UNKNOWN_DTDS; i++)
            {
                // search for an empty slot
                if (!pEdidTx->pEdidDecodData->edidInfo.DTD[i].Valid)
                {
                    timing.Valid = true; // mark the slot as occupied

                    timing.PreferredTiming = false;
                    if (pEdidTx->pEdidDecodData->edidInfo.BasicDispayParameters[4] & 0x02) // Preferred Timing
                        if (!pEdidTx->pEdidDecodData->isNotFirstDtd)
                            timing.PreferredTiming = true;

                    // copy to the output (resulting) structure
                    memcpy(&pEdidTx->pEdidDecodData->edidInfo.DTD[i], &timing, sizeof(dtd_t));
                    break;
                }
            } // for ()
            if (MAX_NMB_OF_UNKNOWN_DTDS == i) // no empty slots case
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
        } // if (video_mode)
    } // if (descriptor) else timing
    pEdidTx->pEdidDecodData->isNotFirstDtd = true; // first DTD is already processed
}


//-------------------------------------------------------------------------------------------------
//! @brief      Decode the DTD_OPT field in the first extension block.
//!
//!             @note: For an extension block version 1 EDID_EXT_ADR__DTD_OPT byte is not
//!                    defined, but anyhow it must be equal 0 for any of the extension blocks.
//!
//! @param[in]  aBlock - pointer to EDID block array
//-------------------------------------------------------------------------------------------------


static void EdidDecodeDtdOpt ( uint8_t aBlock[EDID_BLOCK_SIZE] )
{

    // It is NOT the first decoded extension block

    if ( pEdidTx->pEdidDecodData->nmbOfDecodedExtBlocks)
    {
        if ( pEdidTx->pEdidDecodData->dtdOpt != aBlock[EDID_EXT_ADR__DTD_OPT])
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__EXT_BLOCK_VIOLATION;
        }
    }

    // It IS the first decoded extension block
    else
    {
        uint8_t dtd_opt = aBlock[EDID_EXT_ADR__DTD_OPT];

        // if it is the first decoding extension block,
        // store it for future comparing

        pEdidTx->pEdidDecodData->dtdOpt = dtd_opt;

        if ( aBlock[EDID_EXT_ADR__REVISION] >= 2)
        {
            pEdidTx->pEdidDecodData->edidInfo.NativeFormatsNumber = dtd_opt & 0x0F;
            pEdidTx->pEdidDecodData->edidInfo.SupportYCbCr = false;
            if ( dtd_opt & 0x30) // if YCbCr444 or YCbCr422 is indicated
            {
                if (( dtd_opt & 0x30) == 0x30) // if both of them are indicated
                {
                    pEdidTx->pEdidDecodData->edidInfo.SupportYCbCr = true;
                }
                else
                {
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__EXT_BLOCK_VIOLATION;
                }
                    // 861Cv22 (see page 81) allows to use just one of the bits,
                    // but it is said on pages 77 & 78 of 861Cv22 that both
                    // of the bits must be the same
            }
            pEdidTx->pEdidDecodData->edidInfo.SupportAudio = (0 != (dtd_opt & 0x40));
            pEdidTx->pEdidDecodData->edidInfo.SupportUnderscan = (0 != (dtd_opt & 0x80));
        }
        else
        {
            pEdidTx->pEdidDecodData->edidInfo.NativeFormatsNumber = 1;
            pEdidTx->pEdidDecodData->edidInfo.SupportYCbCr = false;
            pEdidTx->pEdidDecodData->edidInfo.SupportAudio = false;
            pEdidTx->pEdidDecodData->edidInfo.SupportUnderscan = false;
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      If passed data block belongs to HDMI, LLC., parse it.  Otherwise
//!             it is ignored.
//!
//! @param[in]  pData  - pointer to data block
//! @param[in]  length - length of data block
//-------------------------------------------------------------------------------------------------

static void ParseHdmiLlcDataBlock ( uint8_t *pData, uint8_t length )
{
    bool_t hdmi_video_present = false;
    uint8_t *p_current_position = &pData[8]; // pointer to default HDMI video address
    uint8_t k;

    hdmiVsdb_t *p_vsdb = &pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB;

    // From all vendor specific data blocks (VSDB) we are interested only
    // in HDMI one. Others will not be stored or copied to US EDID.

    // If HDMI VSDB has already found, no reason for checking others,
    // especially because only one block of this type is allowed by HDMI spec.
    if ( pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.HdmiMode)
    {
        return;
    }

    //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length1 = %d  \n", length);

    // filter out VSDB which length is less then minimum allowed for HDMI VSDB
    if (length < EDID_VSB_LEN__MIN_HDMI_VSDB)
    {
        return;
    }

    // limit the length in order to not consider bytes not supported by current spec.
    if (length > EDID_VSB_LEN__MAX_HDMI_VSDB)
        length = EDID_VSB_LEN__MAX_HDMI_VSDB;

    // check for the signature
    {
        ROM const uint8_t signature_reference[EDID_VSB_LEN__HDMI_SIGNATURE] = EDID_VSB_DAT__HDMI_SIGNATURE;
        if ( memcmp(signature_reference, pData, EDID_VSB_LEN__HDMI_SIGNATURE) )
        {
            return; // signature was not found
        }
    }

    p_vsdb->HdmiMode = true;

#if ( configSII_DEV_953x_PORTING == 0 )
    // get CEC address
    p_vsdb->CEC.a = pData[3] >> 4;
    p_vsdb->CEC.b = pData[3] & 0x0F;
    p_vsdb->CEC.c = pData[4] >> 4;
    p_vsdb->CEC.d = pData[4] & 0x0F;
#else
	// get CEC address
    p_vsdb->cec.a = pData[3] >> 4;
    p_vsdb->cec.b = pData[3] & 0x0F;
    p_vsdb->cec.c = pData[4] >> 4;
    p_vsdb->cec.d = pData[4] & 0x0F;
#endif 

    // reusing VSDB length as VSDB_extras length
    length -= EDID_VSB_LEN__MIN_HDMI_VSDB;

    //clear out the HDMI VICs so that the composer doesn't see any old data
    for (k = 0; k < MAX_NMB_OF_HDMI_VIC_FORMATS; k++)
    {
        pEdidTx->pEdidDecodData->hdmi_vic[k] = 0;
    }

    // Save HDMI 1.3a VSDB extras if they are present,
    // otherwise 0s will be used as defaults
    if (length >= 3)
    {
        uint8_t extras_length = 3; // minimum HDMI 1.3a extra field length
        length -= 3;

        if (pData[7] & 0x20) // HDMI_Video_present
        {
            // HDMI_Video_present is detected.
            hdmi_video_present = true;
        }
        if ((pData[7] & 0x80) && (length >= 2)) // Latency_Fields_Present
        {
            extras_length += 2;
            p_current_position += 2;
            length -= 2;
        }
        if ((pData[7] & 0x40) && (length >= 2)) // I_Latency_Fields_Present
        {
            extras_length += 2;
            p_current_position += 2;
            length -= 2;
        }

        if (extras_length > EDID_VSB_LEN__MAX_EXTRAS)
        {
            extras_length = EDID_VSB_LEN__MAX_EXTRAS;
        }
        if (extras_length)
        {
            memcpy(p_vsdb->Extras, &pData[5], extras_length);
        }
    }
    else
    {
        memcpy(p_vsdb->Extras, &pData[5], length);
        length = 0;
    }
    //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length2 = %d : \n", length);

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    if (length && hdmi_video_present)
    {
        bool_t hdmi_3d_present = false;
        uint8_t hdmi_3d_multi_present = 0;

        // p_current_position points to the beginning of HDMI Video area

        uint8_t hdmi_video_flags = *p_current_position;

        //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length3 = %d hdmi_video_flags: %02X \n", length, hdmi_video_flags);

        pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.image_size = (hdmi_video_flags >> 3) & 0x03;

        // 3D_present
        hdmi_3d_present =
            (0 != (hdmi_video_flags & 0x80));

        pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.hdmi_3d_present = hdmi_3d_present;

        // 3D_Multi_present
        hdmi_3d_multi_present =
            (hdmi_video_flags & 0x60) >> 5;

        p_current_position++;
        length--;

        //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length4 = %d hdmi_3d_present: %d  hdmi_3d_multi_present: %d\n", length, hdmi_3d_present, hdmi_3d_multi_present);

        if (length)
        {
            uint8_t i;
            uint8_t hdmi_vic_len = ((*p_current_position) >> 5) & 0x7; // HDMI_VIC_LEN field
            uint8_t hdmi_3d_len = (*p_current_position) & 0x1F; // HDMI_3D_LEN field

            p_current_position++;
            length--;

            //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length5 = %d hdmi_vic_len: %d  hdmi_3d_len: %d\n", length, hdmi_vic_len, hdmi_3d_len);

            // store HDMI_VIC fields
            for (i = 0; i < hdmi_vic_len; i++)
            {
                if (length)
                {
                    pEdidTx->pEdidDecodData->hdmi_vic[i] = (*p_current_position);
                    //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length6 = %d hdmi_vic[%d]: %d  hdmi_3d_len: %d\n", i, pEdidTx->pEdidDecodData->hdmi_vic[i]);
                    length--;
                    p_current_position++;
                }
            }


            if (length && hdmi_3d_present)
            {
                // p_current_position points to the beginning of 3D area

                if (length < hdmi_3d_len)
                {
                    hdmi_3d_len = length;
                    length = 0;
                }
                else
                {
                    length -= hdmi_3d_len;
                }

                if (1 == hdmi_3d_multi_present)
                {
                    pEdidTx->pEdidDecodData->isAll3dFormats = true;
                }

                if ((1 == hdmi_3d_multi_present) || (2 == hdmi_3d_multi_present))
                {
                    if (hdmi_3d_len >= 2)
                    {
                        // 3D_Structure_ALL present
                        p_vsdb->hdmi_3d_structure_all = (*p_current_position) << 8;
                        p_current_position++;
                        p_vsdb->hdmi_3d_structure_all |= (*p_current_position);
                        p_current_position++;
                        hdmi_3d_len -= 2;
                    }
                    else
                    {
                        hdmi_3d_len = 0;
                    }
                    //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length5 = %d hdmi_3d_structure_all: %x  \n", length, p_vsdb->hdmi_3d_structure_all);

                }

                if (2 == hdmi_3d_multi_present)
                {
                    if (hdmi_3d_len >= 2)
                    {
                        // 3D_MASK present
                        pEdidTx->pEdidDecodData->hdmi3dMask = (*p_current_position) << 8;
                        p_current_position++;
                        pEdidTx->pEdidDecodData->hdmi3dMask |= (*p_current_position);
                        p_current_position++;
                        hdmi_3d_len -= 2;
                    }
                    else
                    {
                        hdmi_3d_len = 0;
                    }
                    //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length5 = %d hdmi3dMask: %x  \n", length, pEdidTx->pEdidDecodData->hdmi3dMask);
                }

                for (i=0; (hdmi_3d_len) && (i < HDMI_3D_SVD_STRUCTURE_LENGTH); i++)
                {
                    pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].vic_order = (*p_current_position) >> 4; // 2D_VIC_order
                    pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].struct_3d = (*p_current_position) & 0x0F; // 3D_Structure
                    pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].valid = true;
                    //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length7 = %d vic_order: %d struct_3d: %02X valid: %d \n", length, pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].vic_order,pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].struct_3d, pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].valid);
                    p_current_position++;
                    hdmi_3d_len--;
                    if (0 == hdmi_3d_len)
                    {
                        break;
                    }

                    if ( pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].struct_3d >= 8)
                    {
                        pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].detail_3d = (*p_current_position) >> 4; // 3D_Detail
                        //DEBUG_PRINT( MSG_DBG, "ParseHdmiLlcDataBlock: Length7 = %d detail_3d: %x \n", length, pEdidTx->pEdidDecodData->hdmi3dSvdStructure[i].detail_3d);
                        p_current_position++;
                        hdmi_3d_len--;
                    }

                }
            }
        }
    }
#endif // SI_HDMI_1_4_EDID_PROCESSING
}


//-------------------------------------------------------------------------------------------------
//! @brief      Decode data from the data block collection.
//!
//! @param[in]  pDataBlock       - pointer to data block collection
//! @param[in]  collectionLength - length of data block collection
//-------------------------------------------------------------------------------------------------

static void DecodeDataBlockCollection (uint8_t *pDataBlock, int collectionLength)
{
    while(collectionLength > 0)
    {
        uint8_t tag = pDataBlock[0] >> 5;
        int dataBlockLength = pDataBlock[0] & 0x1F;
        pDataBlock++; // move to the data area
        collectionLength -= dataBlockLength + 1; // +1 is for tag
        if (collectionLength < 0) // protection
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DATA_COLLECT_VIOLATION;
            return; // too serious error
        }

        // Decode the current data block.  We may want ONLY the audio information,
        // or we may want ONLY the non-audio information, or we may want everything.

        if (( pEdidTx->isDecodeAudio      &&  pEdidTx->isDecodeNonAudio  ) ||
            ( pEdidTx->isDecodeAudio      &&  (( tag == EDID_DBC_TAG__AUDIO ) || ( tag == EDID_DBC_TAG__SPEAKER_ALLOCATION)))  ||
            ( pEdidTx->isDecodeNonAudio   && !(( tag == EDID_DBC_TAG__AUDIO ) || ( tag == EDID_DBC_TAG__SPEAKER_ALLOCATION)))
            )
        {
            switch(tag)
            {
            case 0:
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DATA_COLLECT_VIOLATION;
                return; // serious error
            case EDID_DBC_TAG__AUDIO:
                if (dataBlockLength % EDID_SAD__LENGTH) // check length
                {
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DATA_COLLECT_VIOLATION;
                    return; // serious error
                }
                while(dataBlockLength)
                {
                    if ( pEdidTx->pEdidDecodData->edidInfo.SadCount < MAX_NMB_OF_SADS )
                    {
                        pEdidTx->pEdidDecodData->edidInfo.SAD[ pEdidTx->pEdidDecodData->edidInfo.SadCount].ByteData.Byte1 = *(pDataBlock++);
                        pEdidTx->pEdidDecodData->edidInfo.SAD[ pEdidTx->pEdidDecodData->edidInfo.SadCount].ByteData.Byte2 = *(pDataBlock++);
                        pEdidTx->pEdidDecodData->edidInfo.SAD[ pEdidTx->pEdidDecodData->edidInfo.SadCount].ByteData.Byte3 = *(pDataBlock++);
                        pEdidTx->pEdidDecodData->edidInfo.SadCount++;
                    }
                    else
                    {
                        pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
                    }
                    dataBlockLength -= EDID_SAD__LENGTH;
                }
                break;
            case EDID_DBC_TAG__VIDEO:
                {
                    int i;
                    for (i=0; i<dataBlockLength; i++) // copy all descriptors (while space exsists)
                    {
                        if ( pEdidTx->pEdidDecodData->SVD_idx < MAX_NMB_OF_SVDS )
                        {
                            uint8_t vic_f = pDataBlock[i]; // video code with a flag
                            uint8_t vic = vic_f & 0x7F;
                            if (vic <= LAST_KNOWN_CEA_VIC)
                            {
                                uint8_t idx = ceaVicToVideoTableIndex[vic];
                                pEdidTx->pEdidDecodData->edidInfo.SupportPal = (VideoModeTable[idx].NtscPal == PAL);
                                pEdidTx->pEdidDecodData->edidInfo.SupportNtsc = (VideoModeTable[idx].NtscPal == NTSC);
                            }
                            pEdidTx->pEdidDecodData->SVD[pEdidTx->pEdidDecodData->SVD_idx++].Data = vic_f;
                        }
                        else
                        {
                            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
                            break;
                        }
                    }
                }
                break;
            case EDID_DBC_TAG__VENDOR_SPECIFIC:
                ParseHdmiLlcDataBlock( pDataBlock, dataBlockLength );   // Looking for HDMI LLC.
                break;
            case EDID_DBC_TAG__SPEAKER_ALLOCATION:
                if ( 0 == pEdidTx->pEdidDecodData->edidInfo.SPK.Data)
                {
                    // If SPK block is not filled yet, fill it if it has correct length.
                    if ( EDID_SPK__LENGTH == dataBlockLength)
                    {
                        pEdidTx->pEdidDecodData->edidInfo.SPK.Data = pDataBlock[0];
                    }
                    else
                    {
                        pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DATA_COLLECT_VIOLATION;
                        return;
                    }
                }
                else
                {
                    // Otherwise do not replace it, but mark as an error.
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__DATA_COLLECT_VIOLATION;
                    //return;
                }
                break;
            case EDID_DBC_TAG__VESA_DTC:
                // I did not find any description of this field
                // as well as requirement to use it or copy to
                // US EDID.
                break;
            case EDID_DBC_TAG__USE_EXTENDED_TAG: // first byte of data area is an another tag
                // 861C requires these field to be copied to US EDID under following
                // conditions:
                // -Video related fields must be copied from DS EDID to US EDID
                // if repeater does not modify video.
                // -Audio related fields must be copied from DS EDID to US EDID
                // if repeater does not modify audio.
                // -If repeater modifies audio or video, corresponded fields should
                // be copied only in case if it is known that they will be correct
                // for US device. The fields could be modified accordingly or at least
                // skipped from coping.
                // Since the exact meaning of them is unknown, what we can do is
                // to copy video related fields if video is not modified and not to copy
                // if there is any video processing. The same thing is for audio:
                // copy audio related fields in case if no audio is altered,
                // otherwise skip copying.

                if ( 0x05 == pDataBlock[0])
                {
                    // extended code 0x05- Colorimetry Data Block

                    // 0xE3 // tag (7) + length (3)
                    // 0x05 // extended tag code
                    // 0x01 for xvYCC601 or 0x02 xvYCC709 or 0x03 for both
                    // 0x01 // profile 0

                    if ( pDataBlock[2] & 0x01)
                    {
                        // if profile P0 is supported
                        pEdidTx->pEdidDecodData->edidInfo.xv601 = (0 != (pDataBlock[1] & 0x01));
                        pEdidTx->pEdidDecodData->edidInfo.xv709 = (0 != (pDataBlock[1] & 0x02));
                    }
                }
    #ifdef COLLECT_EXTENDED_DATA_BLOCKS
                else // !!!
                {
                    if ( pDataBlock[0] < 16)
                    {
                        // video blocks
                        if ((pEdidTx->pEdidDecodData->edidInfo.UnknownVideoExt_Idx + dataBlockLength + 1)
                            < SIZE_OF_UNKNOWN_VIDEO_EXTENDED_BLOCKS_BUFFER)
                        {
                            // save the length
                            pEdidTx->pEdidDecodData->edidInfo.UnknownVideoExtBlocks[pEdidTx->pEdidDecodData->edidInfo.UnknownVideoExt_Idx++] =
                                dataBlockLength;
                            memcpy(&pEdidTx->pEdidDecodData->edidInfo.UnknownVideoExtBlocks[pEdidTx->pEdidDecodData->edidInfo.UnknownVideoExt_Idx],
                                pDataBlock, dataBlockLength);
                            pEdidTx->pEdidDecodData->edidInfo.UnknownVideoExt_Idx += dataBlockLength;
                        }
                        else
                        {
                            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
                        }
                    }
                    else if ( pDataBlock[0] < 32)
                    {
                        // audio blocks
                        if ((pEdidTx->pEdidDecodData->edidInfo.UnknownAudioExt_Idx + dataBlockLength + 1)
                            < SIZE_OF_UNKNOWN_AUDIO_EXTENDED_BLOCKS_BUFFER)
                        {
                            // save the length
                            pEdidTx->pEdidDecodData->edidInfo.UnknownAudioExtBlocks[pEdidTx->pEdidDecodData->edidInfo.UnknownAudioExt_Idx++] =
                                dataBlockLength;
                            memcpy(&pEdidTx->pEdidDecodData->edidInfo.UnknownAudioExtBlocks[pEdidTx->pEdidDecodData->edidInfo.UnknownAudioExt_Idx],
                                pDataBlock, dataBlockLength);
                            pEdidTx->pEdidDecodData->edidInfo.UnknownAudioExt_Idx += dataBlockLength;
                        }
                        else
                        {
                            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
                        }
                    }
                    else
                    {
                        // skip, i.e. do not copy others
                    }
                }
    #endif // COLLECT_EXTENDED_DATA_BLOCKS
                break;
            default:
                // do nothing with the reserved tag
                break;
            }
        } //End 'if decodeAudioOnly...'

        pDataBlock += dataBlockLength;
    }
}



#ifdef CHECK_AND_CORRECT_RANGE_LIMITS

//-------------------------------------------------------------------------------------------------
//! @brief      This function is to check and recover Range Limits structure (if required).
//!             It works by looking through the DTD and the SDTD arrays for the minimum
//!             and maximum values.
//-------------------------------------------------------------------------------------------------

static void CheckAndCorrectRangeLimits(void)
{
    int i;
    uint8_t min_v_hz = DEFAULT__MIN_V_HZ; // Min. Vertical rate (for interlace this refers to field rate), in Hz.
    uint8_t max_v_hz = DEFAULT__MAX_V_HZ; // Max. Vertical rate (for interlace this refers to field rate), in Hz.
    uint8_t min_h_khz = DEFAULT__MIN_H_KHZ; // Min. Horizontal in kHz
    uint8_t max_h_khz = DEFAULT__MAX_H_KHZ; // Max. Horizontal in kHz,
    uint8_t max_pix_clk_10mhz = DEFAULT__MAX_PIX_CLK_10MHZ; // Max. Supported Pixel Clock, in MHz/10, rounded

    if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.RangeLimits)
    {
        // if the Range Limits structure exists
        min_v_hz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinV_Hz;
        max_v_hz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxV_Hz;
        min_h_khz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinH_kHz;
        max_h_khz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxH_kHz;
        max_pix_clk_10mhz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxPixClk_10MHz;
    }

    // Look through SDTDs
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
        {
            uint8_t idx = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].VideoIdx;
            if (VideoModeTable[idx].PixFreq > ((uint16_t) max_pix_clk_10mhz * 10))
            {
                max_pix_clk_10mhz = (VideoModeTable[idx].PixFreq + 9) / 10;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
            }
            if (VideoModeTable[idx].VFreq > max_v_hz)
            {
                max_v_hz = VideoModeTable[idx].VFreq;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
            }
            if (VideoModeTable[idx].VFreq < min_v_hz)
            {
                min_v_hz = VideoModeTable[idx].VFreq;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
            }
            if (VideoModeTable[idx].HFreq > max_h_khz)
            {
                max_h_khz = VideoModeTable[idx].HFreq;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
            }
            if (VideoModeTable[idx].HFreq < min_h_khz)
            {
                min_h_khz = VideoModeTable[idx].HFreq;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
            }
        }
    } // for () SDTDs

    // Look through DTDs
    for (i=0; i<MAX_NMB_OF_UNKNOWN_DTDS; i++)
    {
        dtd_t *p_dtd = &pEdidTx->pEdidDecodData->edidInfo.DTD[i];
        if (p_dtd->Valid)
        {
            if (p_dtd->PixClk_10kHz > ((uint16_t) max_pix_clk_10mhz * 1000))
            {
                max_pix_clk_10mhz = (p_dtd->PixClk_10kHz + 999) / 1000;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
            }
            {
                uint16_t total_h_pix = p_dtd->HAct_Pix + p_dtd->HBlank_Pix;
                uint16_t total_v_pix = p_dtd->VAct_Pix + p_dtd->VBlank_Pix;
                // check the variable width carefully when porting to small micros!
                uint8_t h_khz = (uint8_t) (((uint32_t) p_dtd->PixClk_10kHz) * 10 / total_h_pix);
                uint8_t v_hz = (uint8_t) (((uint32_t) h_khz) * 1000 / total_v_pix);
                //if (p_dtd->Interlaced)
                //  v_hz *= 2;
                if (v_hz > max_v_hz)
                {
                    max_v_hz = v_hz;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
                }
                if (v_hz < min_v_hz)
                {
                    min_v_hz = v_hz;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
                }
                if (h_khz > max_h_khz)
                {
                    max_h_khz = h_khz;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
                }
                if (h_khz < min_h_khz)
                {
                    min_h_khz = h_khz;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITS_CORRECTED;
                }
            }
        }
    } // for () DTDs

    {
        // store recovered data
        pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinV_Hz = min_v_hz;
        pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxV_Hz = max_v_hz;
        pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinH_kHz = min_h_khz;
        pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxH_kHz = max_h_khz;
        pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxPixClk_10MHz = max_pix_clk_10mhz;
        pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.RangeLimits = true;
    }
}
#endif // CHECK_AND_CORRECT_RANGE_LIMITS


//-------------------------------------------------------------------------------------------------
//! @brief      Add video mode.
//-------------------------------------------------------------------------------------------------

static void AddVideoMode(uint8_t vic)
{
    // Note: there are no SVDs in DVI monitor case
    // (even no EDID extension blocks (almost in 100% cases))
    if ( pEdidTx->pEdidDecodData->SVD_idx < MAX_NMB_OF_SVDS)
    {
        // no need to set "native" flag
        pEdidTx->pEdidDecodData->SVD[pEdidTx->pEdidDecodData->SVD_idx++].Data = vic;
    }
}


#ifdef TRUST_RANGE_LIMITS

//-------------------------------------------------------------------------------------------------
//! @brief      This function is to filter out any "out of range" timing
//!             (in the DTD and the SDTD arrays).
//-------------------------------------------------------------------------------------------------

static void FilterOutOfRangeVideo (void)
{
    int i;
    uint8_t min_v_hz = DEFAULT__MIN_V_HZ; // Min. Vertical rate (for interlace this refers to field rate), in Hz.
    uint8_t max_v_hz = DEFAULT__MAX_V_HZ; // Max. Vertical rate (for interlace this refers to field rate), in Hz.
    uint8_t min_h_khz = DEFAULT__MIN_H_KHZ; // Min. Horizontal in kHz
    uint8_t max_h_khz = DEFAULT__MAX_H_KHZ; // Max. Horizontal in kHz,
    uint8_t max_pix_clk_10mhz = DEFAULT__MAX_PIX_CLK_10MHZ; // Max. Supported Pixel Clock, in MHz/10, rounded

    if (pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.RangeLimits)
    {
        // if the Range Limits structure exists
        min_v_hz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinV_Hz;
        max_v_hz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxV_Hz;
        min_h_khz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinH_kHz;
        max_h_khz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxH_kHz;
        max_pix_clk_10mhz = pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxPixClk_10MHz;
    }
    else
    {
        // if the Range Limits structure was not declared in the EDID,
        // store the default values
    //  pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinV_Hz = min_v_hz;
    //  pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxV_Hz = max_v_hz;
    //  pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinH_kHz = min_h_khz;
    //  pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxH_kHz = max_h_khz;
    //  pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxPixClk_10MHz = max_pix_clk_10mhz;
    //  // ??? pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.RangeLimits = true; ???
    }

    // Look through SDTDs
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
        {
            uint8_t idx = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].VideoIdx;
            if (VideoModeTable[idx].PixFreq > ((uint16_t) max_pix_clk_10mhz * 10))
            {
                pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid = false;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITATION;
                continue;
            }
            if ( (VideoModeTable[idx].VFreq > max_v_hz) ||
                (VideoModeTable[idx].VFreq < min_v_hz) )
            {
                pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid = false;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITATION;
                continue;
            }
            if ( (VideoModeTable[idx].HFreq > max_h_khz) ||
                (VideoModeTable[idx].HFreq < min_h_khz) )
            {
                pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid = false;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITATION;
                continue;
            }
        }
    } // for () SDTDs

    // Look through DTDs
    for (i=0; i<MAX_NMB_OF_UNKNOWN_DTDS; i++)
    {
        dtd_t *p_dtd = &pEdidTx->pEdidDecodData->edidInfo.DTD[i];
        if (p_dtd->Valid)
        {
            if (p_dtd->PixClk_10kHz > ((uint16_t) max_pix_clk_10mhz * 1000))
            {
                p_dtd->Valid = false;
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITATION;
                continue;
            }
            {
                uint16_t total_h_pix = p_dtd->HAct_Pix + p_dtd->HBlank_Pix;
                uint16_t total_v_pix = p_dtd->VAct_Pix + p_dtd->VBlank_Pix;
                // check the variable width carefully when porting to small micros!
                uint8_t h_khz = ((uint32_t) p_dtd->PixClk_10kHz) * 10 / total_h_pix;
                uint8_t v_hz = ((uint32_t) h_khz) * 1000 / total_v_pix;
                //if (p_dtd->Interlaced)
                //  v_hz *= 2;
                if ( (v_hz > max_v_hz) ||
                    (v_hz < min_v_hz) )
                {
                    p_dtd->Valid = false;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITATION;
                    continue;
                }
                if ( (h_khz > max_h_khz) ||
                    (h_khz < min_h_khz) )
                {
                    p_dtd->Valid = false;
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__RANGE_LIMITATION;
                    continue;
                }
            }
        }
    } // for () DTDs
}
#endif // TRUST_RANGE_LIMITS


//-------------------------------------------------------------------------------------------------
//! @brief      Add DVI video modes.
//!
//!             This function is called in 1 block EDID case.
//!             It means that it is a DVI monitor which does not have
//!             enough room in its EDID and therefore some video modes
//!             are not declared.
//!             This function tries to discover skipped modes.
//-------------------------------------------------------------------------------------------------

void EdidTxAddDviVideoModes(void)
{
    bool_t add_ntsc_modes = false;
    bool_t add_pal_modes = false;
    bool_t ar4_3 = pEdidTx->pEdidDecodData->edidInfo.SupportAspect4_3; // true if aspect ratio 4:3 is supported
    bool_t ar16_9 = pEdidTx->pEdidDecodData->edidInfo.SupportAspect16_9; // true if aspect ratio 16:9 is supported

    if ( pEdidTx->pEdidDecodData->edidInfo.SupportNtsc)
    {
        add_ntsc_modes = true;
    }
    if ( pEdidTx->pEdidDecodData->edidInfo.SupportPal )
    {
        add_pal_modes = true;
    }
    if (( pEdidTx->pEdidDecodData->edidInfo.SupportNtsc) && ( !pEdidTx->pEdidDecodData->edidInfo.SupportPal))
    {
        // if support neither for NTSC neither for PAL was found
#ifdef DEFAULT_SYSTEM__PAL
        add_pal_modes = true;
#else // DEFAULT_SYSTEM__PAL
        add_ntsc_modes = true;
#endif // DEFAULT_SYSTEM__PAL
    }

    if (add_pal_modes)
    {
        if (ar4_3 || ((!ar4_3) && (!ar16_9))) // if 4:3 is supported or no information about the aspect ratio
        {
            //add_video_mode(21); // 576i @ 50Hz
            AddVideoMode(17); // 576p @ 50Hz
        }
        if (ar16_9)
        {
            //add_video_mode(22); // 576i @ 50Hz
            AddVideoMode(18); // 576p @ 50Hz
        }
        AddVideoMode(19); // 720p @ 50Hz
        AddVideoMode(20); // 1080i @ 50Hz
    }
    if (add_ntsc_modes)
    {
        if (ar4_3 || ((!ar4_3) && (!ar16_9))) // if 4:3 is supported or no information about the aspect ratio
        {
            //add_video_mode(6); // 480i @ 60Hz
            AddVideoMode(2); // 480p @ 60Hz
        }
        if (ar16_9)
        {
            //add_video_mode(7); // 480i @ 60Hz
            AddVideoMode(3); // 480p @ 60Hz
        }
        AddVideoMode(4); // 720p @ 60Hz
        AddVideoMode(5); // 1080i @ 60Hz
    }
    pEdidTx->pEdidDecodData->edidInfo.ExtraVicAdded = true;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Actions after decoding.
//!
//-------------------------------------------------------------------------------------------------

void EdidTxPostDecode(void)
{
    int i, j;

    if (pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming1 & 0x20) // 640 x 480 @ 60Hz
    {
        // be sure that VGA mode does exist
        // (it can be skipped in DVI EDIDs or incompliant HDMI ones)
        bool_t found = false;
        uint8_t vic = 1; // VGA 460x480 (vm1_640x480p)

        // be sure that an SVD with the same VIC does not exist
        for (i=0; i<pEdidTx->pEdidDecodData->SVD_idx; i++)
        {
            // no need to set "native" flag
            if (vic == pEdidTx->pEdidDecodData->SVD[i].Fields.Vic)
            {
                found = true;
                break;
            }
        }
        // Actually, the searching for existed VIC is not necessary-
        // doubled VICs could be filtered out at the stage when
        // SDTDs are formed from SVDs.

        if (!found)
            AddVideoMode(vic);
    }


////////////////////////////////////////////////////////////
    // Delete duplicated video modes in SDTD[]
    // (Some TVs have a problem: dublicated DTDs, i.e. 2 identical
    // DTD detail timings are present in one EDID)
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        uint8_t vic = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Vic;
        for (j=i+1; j<MAX_NMB_OF_KNOWN_DTDS; j++)
        {
            if (pEdidTx->pEdidDecodData->edidInfo.SDTD[j].Valid)
            {
                // Check for the same VIC in SDTDs
                if (vic == pEdidTx->pEdidDecodData->edidInfo.SDTD[j].Vic)
                {
                    pEdidTx->pEdidDecodData->edidInfo.SDTD[j].Valid = false;
                    break;
                }
            }
        } // for (i) SDTDs
    } // for (j) SDTDs
////////////////////////////////////////////////////////////


//#define COMPOSE_ABSENT_SVDS
#ifdef COMPOSE_ABSENT_SVDS
    // Compose SVDs if corresponded DTDs are not present.

    // Look through SDTDs
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
        {
            // Check for the same VIC in SVDs
            uint8_t vic = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Vic;
            uint8_t j_max = pEdidTx->pEdidDecodData->edidInfo.SVD_idx;
            if (j_max > MAX_NMB_OF_SVDS)
            {
                pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__INTERNAL_ERROR;
                j_max = pEdidTx->pEdidDecodData->edidInfo.SVD_idx = MAX_NMB_OF_SVDS;
            }
            for (j=0; j<j_max; j++)
            {
                if (vic == pEdidTx->pEdidDecodData->edidInfo.SVD[j].Fields.Vic)
                    break;
            } // for () SVDs
            if (j_max == j)
            {
                // found a DTD Detail Timing described in CEA861C which does not have
                // corresponded SVD (HDMI spec violation)
                if (pEdidTx->pEdidDecodData->edidInfo.HDMI_VSDB.HdmiMode)
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__DATA_COLLECT_VIOLATION;
                //else
                //  pEdidTx->pEdidDecodData->edidInfo.ExtraVicAdded = true;
                AddVideoMode(vic); // correct the mistake
            }
        }
    } // for () SDTDs
#endif // COMPOSE_ABSENT_SVDS

#define COMPOSE_ABSENT_DTDS
#ifdef COMPOSE_ABSENT_DTDS
    // Recover the DTDs which have corresponding SVDs
    // (i.e. which were not placed into the EDID because of the limited size of it)

    // Look through SVDs
    {
        uint8_t j_max = pEdidTx->pEdidDecodData->SVD_idx;
        if (j_max > MAX_NMB_OF_SVDS)
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__INTERNAL_ERROR;
            j_max = pEdidTx->pEdidDecodData->SVD_idx = MAX_NMB_OF_SVDS;
        }
        for (j=0; j<j_max; j++)
        {
            uint8_t vic = pEdidTx->pEdidDecodData->SVD[j].Fields.Vic;
            bool_t native = pEdidTx->pEdidDecodData->SVD[j].Fields.Native;

            if (vic > LAST_KNOWN_CEA_VIC)
                vic = 0;
            for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
            {
                if (pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
                {
                    // Check for the same VIC in SDTDs
                    if (vic == pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Vic)
                    {
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Native = native;
                        break;
                    }
                }
            } // for () SDTDs
            if (MAX_NMB_OF_KNOWN_DTDS == i)
            {
                // No Short DTD was found
                // Compose it
////////////////////////////////////////////////////////////////////////////////////

                uint8_t video_idx = ceaVicToVideoTableIndex[vic];
                //090615 int i;
                for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
                {
                    if (!pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid) // if the slot is empty
                    {
                //      uint16_t v, h; // V and H picture sizes in mm
                        // determine picture aspect ratio
                        bool_t ar16x9 = (vic == VideoModeTable[video_idx].Vic16x9);

                //      // get V size and calculate H size
                //      v = ar16x9 ? pEdidTx->pEdidDecodData->edidInfo.VSize16x9 :
                //          pEdidTx->pEdidDecodData->edidInfo.VSize4x3;
                //      if (0 == v) // if the size for given aspect ratio is not saved, try the size for another aspect ratio
                //          v = ar16x9 ? pEdidTx->pEdidDecodData->edidInfo.VSize4x3 :
                //              pEdidTx->pEdidDecodData->edidInfo.VSize16x9;
                //      if (0 == v) // if the size is stil unknown, make it
                //          v = 90; // because it is easily divided by 9
                //      h = v * (ar16x9 ? 16 : 12) / 9;

                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid = true; // mark the slot as occupied
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].VideoIdx = video_idx;
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Vic = vic;
                //      pEdidTx->pEdidDecodData->edidInfo.SDTD[i].HImageSize = h;
                //      pEdidTx->pEdidDecodData->edidInfo.SDTD[i].VImageSize = v;
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Ar16x9 = ar16x9;
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].DisplayType = 0; // Normal, Not Stereo
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].PreferredTiming = false;
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Native = native;

                        if (ar16x9)
                            pEdidTx->pEdidDecodData->edidInfo.SupportAspect16_9 = true;
                        else
                            pEdidTx->pEdidDecodData->edidInfo.SupportAspect4_3 = true;
                        break;
                    }
                } // for ()
                if (MAX_NMB_OF_KNOWN_DTDS == i) // no empty slots case
                    pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;

////////////////////////////////////////////////////////////////////////////////////
            }
        } // for () SVDs
    }
#endif // COMPOSE_ABSENT_DTDS

#ifdef CHECK_AND_CORRECT_RANGE_LIMITS
    CheckAndCorrectRangeLimits();
#endif // CHECK_AND_CORRECT_RANGE_LIMITS

#ifdef TRUST_RANGE_LIMITS
    FilterOutOfRangeVideo();
#endif // TRUST_RANGE_LIMITS

    // Check H and V sizes precision.
    // If the precision is not enough, replace the values with the default ones.
    {
        uint16_t v, h;
        uint16_t ar;

        // Check 4:3 sizes
        EdidTxImageSizeGet(&(pEdidTx->pEdidDecodData->edidInfo), false, &v, &h);
        ar = (uint16_t) (((uint32_t) h) * 1000 / ((uint32_t) v));
        // should be within (1.333 * 1000) +- 5%
        if ((ar < 1270) || (ar > 1390))
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__INCORRECT_PICTURE_SIZE;
            pEdidTx->pEdidDecodData->edidInfo.VSize4x3 = DEFAULT__V_SIZE;
        }

        // Check 16:9 sizes
        EdidTxImageSizeGet(&(pEdidTx->pEdidDecodData->edidInfo), true, &v, &h);
        ar = (uint16_t) (((uint32_t) h) * 1000 / ((uint32_t) v));
        // should be within (1.778 * 1000) +- 5%
        if ((ar < 1690) || (ar > 1860))
        {
            pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__INCORRECT_PICTURE_SIZE;
            pEdidTx->pEdidDecodData->edidInfo.VSize16x9 = DEFAULT__V_SIZE;
        }
    }


#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    if (pEdidTx->pEdidDecodData->isAll3dFormats)
    {
        // Mark all SDTDs as 3D capable.
        // Look through all SDTDs
        for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
        {
            // If SDTD is filled with valid data
            if (pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
            {
                // If VIC from SVD matches SDTD's VIC
                pEdidTx->pEdidDecodData->edidInfo.SDTD[i].masked_3d = true;
            }
        } // for () SDTDs
    }


    // Look through SVDs and mark those that support 3D
    // Limitation: Only 1 uncommon 3D mode per resolution is currently supported

    // First 16 SVDs can be marked with 3D capabilities,
    // look through them then:
    // a) Find SDTD structure using found VIC.
    // b) If found, mark it as 3D capable and copy the 3D format info.
#if MAX_NMB_OF_SVDS < 16
#  error incorrect MAX_NMB_OF_SVDS
#endif // MAX_NMB_OF_SVDS
    for (j=0; j<16; j++) // "j" is a vic index as it appears in the EDID
    {
        uint8_t vic = pEdidTx->pEdidDecodData->SVD[j].Fields.Vic;

        // Look through all SDTDs
        for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++) // "i" is SDTD index
        {
        	shortDtd_t *pSdtd = &pEdidTx->pEdidDecodData->edidInfo.SDTD[i];
            // Check if SDTD is filled with valid data
            if (pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
            {
                // Check if VIC from SVD matches SDTD's VIC
                if (vic == pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Vic)
                {
                    // Found SDTD with VIC that appears in EDID at "j" index
                    uint8_t k;
                    // If the 3D Common Mode is supported for this VIC
                    if (pEdidTx->pEdidDecodData->hdmi3dMask & (1 << j))
                    {
                        pEdidTx->pEdidDecodData->edidInfo.SDTD[i].masked_3d = true;
                    }

                    // Search if there is any 3D_Structure format
                    // Referring to the VIC.
                    for (k=0; k < HDMI_3D_SVD_STRUCTURE_LENGTH; k++)
                    {
                        if (pEdidTx->pEdidDecodData->hdmi3dSvdStructure[k].valid)
                        {
                            if (j == pEdidTx->pEdidDecodData->hdmi3dSvdStructure[k].vic_order)
                            {
                            	uint8_t indiv3Dindex = pSdtd->indiv3dModeLength;
                            	if(indiv3Dindex < MAX_INDIVIDUAL_3D_MODES_PER_VIC)
                            	{
                                    uint8_t struct_3d =
                                        pEdidTx->pEdidDecodData->hdmi3dSvdStructure[k].struct_3d;
                                    // mark that 3D is present
                                    pSdtd->indiv3dModes[indiv3Dindex].struct_3d_present = true;
                                    pSdtd->indiv3dModes[indiv3Dindex].struct_3d = struct_3d;
                                    if (struct_3d >= 8)
                                    {
                                    	pSdtd->indiv3dModes[indiv3Dindex].detail_3d =
                                            pEdidTx->pEdidDecodData->hdmi3dSvdStructure[k].detail_3d;
                                    }

                                    pSdtd->indiv3dModeLength++;
                            	}
                            }
                        }
                    }
                }
            }
        } // for () SDTDs
    } // for () SVDs
#endif // SI_HDMI_1_4_EDID_PROCESSING

}


//-------------------------------------------------------------------------------------------------
//! @brief      Decode "additional timings" blocks.  Ignore all others.
//!
//! @param[in]  aBlock     - pointer to EDID block array,
//! @param[in]  blockIndex - zero based EDID block number
//-------------------------------------------------------------------------------------------------

void EdidTxParseBlock ( uint8_t aBlock[EDID_BLOCK_SIZE], int blockIndex )
{
    // Blocks greater than 0 that are Additional Timing blocks are all
    // treated the same.  All other block types are ignored.

    if ( EDID_EXT_TAG__ADDITIONAL_TIMINGS != aBlock[EDID_EXT_ADR__TAG] )
    {
        return;
    }

    // collect DTDs
    if ( pEdidTx->isDecodeNonAudio )
    {
        uint8_t dtd_offset = aBlock[EDID_EXT_ADR__D];
        uint8_t last_dtd_offset = EDID_ADR__CHECK_SUM - EDID_LEN__DTD;

        EdidDecodeDtdOpt( aBlock );     // If first extension, decode the DTD_OPT

        for ( ; dtd_offset <= last_dtd_offset; dtd_offset += EDID_LEN__DTD)
        {
            if ((aBlock[dtd_offset+0])  | (aBlock[dtd_offset+1]) |
                (aBlock[dtd_offset+2])  | (aBlock[dtd_offset+3]) )
            {
                // if at least one byte in the header differs from 0
                EdidTxDecodeDtd(&aBlock[dtd_offset]);
            }
            else
                break; // stop DTD decoding after meeting zero header
        }
    }

    if ( aBlock[EDID_EXT_ADR__REVISION] >= 3)   // this feature is supported starting from version 3
    {
        // process data from the data block collection
        uint8_t data_block_collection_length = aBlock[EDID_EXT_ADR__D] - EDID_EXT_ADR__DBC;
        DecodeDataBlockCollection( &aBlock[EDID_EXT_ADR__DBC], data_block_collection_length);
    }

    pEdidTx->pEdidDecodData->nmbOfDecodedExtBlocks++;
}


//---------------------------------------------------------------------
// NOTES
/*
CEA-861C page 70
If, for some reason, an indication is received that conflicts with the video format being received (e.g., the
source device indicates 4:3 but sends the 1920x1080i format), then the DTV Monitor shall use the picture
aspect ratio that is associated with the format being sent.

BUT! on page 75
Data Byte 4 contains a Video Identification Code. In most cases, the video format can be uniquely
determined from the video format timing itself. However, if the source box is sending one of the video
formats defined in this document, then it shall set this field to the proper code. If this field is used and if it
is inconsistent with the video format being received, then it shall be ignored by the DTV Monitor. If the
picture aspect ratio implied by this field does not agree with the picture aspect ratio communicated in
Data Byte 2, then Data Byte 2 shall take precedence.

HDMI CTS 1.1 Final page 129:
[AVI M1, M0 bits (picture aspect ratio) must match transmitted video format.]
If AVI M0-M1 fields indicates an aspect ratio not permitted for the transmitted video
format timing -> FAIL
If AVI M1, M0 fields are 0, 1 (4:3) or 1, 0 (16:9) and do not match aspect ratio
corresponding to transmitted VIC field -> FAIL
page 184:
If DTD matches any 861B format, search SVDs for that same video format at same
aspect ratio.
If no matching SVD -> FAIL
*/

/*
HDMI CTS 1.1 Final page 185:
["The required 861/861A formats shall be advertised using the EDID 18-byte Detailed
Timing Descriptors (for backward compatibility)."]
For each SVD in EDID:
- If SVD matches one of the following
 720x480p @ 59.94/60Hz
 1280x720p @ 59.94/60Hz
 1920x1080i @ 59.94/60Hz
 720x576p @ 50Hz
 1280x720p @ 50Hz
 1920x1080i @ 50Hz
- then search for DTD matching (per algorithm above) the format at the same aspect
ratio.
- If no corresponding DTD found -> FAIL
*/
