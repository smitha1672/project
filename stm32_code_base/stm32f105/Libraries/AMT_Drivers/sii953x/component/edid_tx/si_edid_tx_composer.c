//************************************************************************************************
//! @file   si_edid_tx_composer.c
//! @brief  Generation of the repeater EDID structure
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***********************************************************************************************/


#include <string.h>
#include "si_edid_tx_component.h"
#include "si_edid_tx_internal.h"

#define COMPOSE_SAD_SEPARATELY


/*
This Version Limitations:
- If DS EDID has native DTDs which does not have corresponding Short Video Descriptor,
  in rare cases, the Native attribute for them can be lost.
- The Data Collection could be placed into the first extension block.
*/

// Following definitions are to be sure that the EDID buffer margin is not exceeded
#define BEGIN_BOUNDARY_CHECK do{
#define CHECK_BOUNDARY(last_idx) if ((last_idx) >= EDID_ADR__CHECK_SUM) {pEdidTx->edidInfo.ErrorCode = EDID_ERROR__NO_SPACE_FOR_DBC; break;}
#define END_BOUNDARY_CHECK }while(false);


//-------------------------------------------------------------------------------------------------
//! @brief      Calculate and return EDID checksum.
//!
//! @param[in]  aEdidBlock - array containing one EDID block
//!
//! @return     EDID checksum.
//-------------------------------------------------------------------------------------------------

static uint8_t EdidCheckSumCalc(uint8_t aEdidBlock[EDID_BLOCK_SIZE])
{
    uint8_t checkSum = 0;
    int i;
    for (i = 0; i < (EDID_BLOCK_SIZE-1); i++)
        checkSum += aEdidBlock[i];
    checkSum = 0x100 - checkSum;
    return checkSum;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Create Short DTD EDID section based on parsed/decoded input DTD.
//!
//! @param[out] pTiming - pointer to a DTD section of EDID,
//! @param[in]  pSdtd   - pointer to a SDTD
//-------------------------------------------------------------------------------------------------

static void MakeDtdFromSdtd( uint8_t *pTiming, shortDtd_t *pSdtd)
{
    uint8_t video_idx = pSdtd->VideoIdx;
    const videoMode_t *pVideoTable;
    uint16_t v_size, h_size;
    uint16_t active_v;
    uint8_t flags = 0x18; // means "Digital Interface"

    if ( video_idx > NMB_OF_VIDEO_MODES) // protection
    {
        video_idx = vm1_640x480p;
    }
    pVideoTable = &VideoModeTable[video_idx];

    EdidTxImageSizeGet(&(pEdidTx->edidInfo), pSdtd->Ar16x9, &v_size, &h_size);

    active_v = (pVideoTable->Active.V >> pVideoTable->Interlaced);
/*
CEA-861C:
7.2.1 Use of EDID Detailed Timing Descriptors
For the purposes of this standard, a DTV Monitor intended for 60 Hz countries shall support both the
60 Hz and 59.94 Hz version of any format it supports. The 60 Hz version shall be described in the EDID
structure for HDTV formats, the 59.94Hz version shall be described for all 480-line formats.
*/
//  {
//      uint16_t v_rate = 6000;
//      uint32_t pix_clk;
//      if (ABS_DIFF(p_video_table->VFreq, 60) <= 1) // 59/60 Hz modes
//          if (480 == p_video_table->Active.V) // VGA, 480i, 480p
//              v_rate = 5994;
//      pix_clk = v_rate * p_video_table->Total.H * p_video_table->Total.V;
//      if (p_video_table->Interlaced)
//          pix_clk /= 2;
//      pixel_clock = (uint16_t) ((pix_clk + 5000) / 10000); // +5000 is for better rounding
//  }

    pTiming[EDID_DTD_ADR__PIX_CLK_LOW] = (uint8_t) pVideoTable->PixClk;
    pTiming[EDID_DTD_ADR__PIX_CLK_HIGH] = pVideoTable->PixClk >> 8;
    pTiming[EDID_DTD_ADR__H_ACTIVE_LOW] = (uint8_t) pVideoTable->Active.H;
    pTiming[EDID_DTD_ADR__H_BLANK_LOW] = (uint8_t) pVideoTable->Blank.H;
    pTiming[EDID_DTD_ADR__H_HIGH] =
        ((pVideoTable->Active.H >> 4) & 0xF0) |
        ((pVideoTable->Blank.H  >> 8) & 0x0F);
    pTiming[EDID_DTD_ADR__V_ACTIVE_LOW] = (uint8_t) active_v;
    pTiming[EDID_DTD_ADR__V_BLANK_LOW] = (uint8_t) pVideoTable->Blank.V;
    pTiming[EDID_DTD_ADR__V_HIGH] =
        ((active_v >> 4) & 0xF0) |
        ((pVideoTable->Blank.V  >> 8) & 0x0F);
    pTiming[EDID_DTD_ADR__H_SYNC_OFFSET_LOW] = (uint8_t) pVideoTable->SyncOffset.H;
    pTiming[EDID_DTD_ADR__H_SYNC_WIDTH_LOW] = (uint8_t) pVideoTable->SyncWidth.H;
    pTiming[EDID_DTD_ADR__V_SYNC_LOW] =
        ((pVideoTable->SyncOffset.V << 4) & 0xF0) |
        ((pVideoTable->SyncWidth.V) & 0x0F);
    pTiming[EDID_DTD_ADR__H_V_SYNC_HIGH] =
        ((pVideoTable->SyncOffset.H >> 2) & 0xC0) |
        ((pVideoTable->SyncWidth.H  >> 4) & 0x30) |
        ((pVideoTable->SyncOffset.V >> 2) & 0x0C) |
        ((pVideoTable->SyncWidth.V  >> 4) & 0x03);
    pTiming[EDID_DTD_ADR__H_SIZE_LOW] = (uint8_t) h_size;
    pTiming[EDID_DTD_ADR__V_SIZE_LOW] = (uint8_t) v_size;
    pTiming[EDID_DTD_ADR__H_V_SIZE_HIGH] =
        ((h_size >> 4) & 0xF0) |
        ((v_size >> 8) & 0x0F);
    pTiming[EDID_DTD_ADR__H_BORDER] = 0;
    pTiming[EDID_DTD_ADR__V_BORDER] = 0;
    if (pVideoTable->Interlaced)
        flags |= 0x80;
    if (POS == pVideoTable->HPol)
    {
        flags |= 0x02;
    }
    if (POS == pVideoTable->VPol)
    {
        flags |= 0x04;
    }
    if (pSdtd->DisplayType & 0x01)
    {
        flags |= 0x01;
    }
    if (pSdtd->DisplayType & 0x02)
    {
        flags |= 0x20;
    }
    if (pSdtd->DisplayType & 0x04)
    {
        flags |= 0x40;
    }
    pTiming[EDID_DTD_ADR__FLAGS] = flags;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Create DTD EDID section based on parsed/decoded input DTD.
//!
//! @param[out] pTiming - pointer to a DTD section of EDID,
//! @param[in]  pSdtd   - pointer to a SDTD
//-------------------------------------------------------------------------------------------------

static void MakeDtdFromDtd(uint8_t *pTiming, dtd_t *pDtd)
{
    pTiming[EDID_DTD_ADR__PIX_CLK_LOW] = (uint8_t) pDtd->PixClk_10kHz;
    pTiming[EDID_DTD_ADR__PIX_CLK_HIGH] = pDtd->PixClk_10kHz >> 8;
    pTiming[EDID_DTD_ADR__H_ACTIVE_LOW] = (uint8_t) pDtd->HAct_Pix;
    pTiming[EDID_DTD_ADR__H_BLANK_LOW] = (uint8_t) pDtd->HBlank_Pix;
    pTiming[EDID_DTD_ADR__H_HIGH] =
        ((pDtd->HAct_Pix   >> 4) & 0xF0) |
        ((pDtd->HBlank_Pix >> 8) & 0x0F);
    pTiming[EDID_DTD_ADR__V_ACTIVE_LOW] = (uint8_t) pDtd->VAct_Pix;
    pTiming[EDID_DTD_ADR__V_BLANK_LOW] = (uint8_t) pDtd->VBlank_Pix;
    pTiming[EDID_DTD_ADR__V_HIGH] =
        ((pDtd->VAct_Pix   >> 4) & 0xF0) |
        ((pDtd->VBlank_Pix >> 8) & 0x0F);
    pTiming[EDID_DTD_ADR__H_SYNC_OFFSET_LOW] = (uint8_t) pDtd->HSyncOffset;
    pTiming[EDID_DTD_ADR__H_SYNC_WIDTH_LOW] = (uint8_t) pDtd->HSyncPulseWith;
    pTiming[EDID_DTD_ADR__V_SYNC_LOW] =
        ((pDtd->VSyncOffset << 4) & 0xF0) |
        ((pDtd->VSyncPulseWith) & 0x0F);
    pTiming[EDID_DTD_ADR__H_V_SYNC_HIGH] =
        ((pDtd->HSyncOffset    >> 2) & 0xC0) |
        ((pDtd->HSyncPulseWith >> 4) & 0x30) |
        ((pDtd->VSyncOffset    >> 2) & 0x0C) |
        ((pDtd->VSyncPulseWith >> 4) & 0x03);
    pTiming[EDID_DTD_ADR__H_SIZE_LOW] = (uint8_t) pDtd->HImageSize;
    pTiming[EDID_DTD_ADR__V_SIZE_LOW] = (uint8_t) pDtd->VImageSize;
    pTiming[EDID_DTD_ADR__H_V_SIZE_HIGH] =
        ((pDtd->HImageSize >> 4) & 0xF0) |
        ((pDtd->VImageSize >> 8) & 0x0F);
    pTiming[EDID_DTD_ADR__H_BORDER] = pDtd->HBorder;
    pTiming[EDID_DTD_ADR__V_BORDER] = pDtd->VBorder;
    {
        uint8_t flags = (pDtd->InterfaceType << 1) & 0x1E;
        if (pDtd->Interlaced)
        {
            flags |= 0x80;
        }
        if (pDtd->DisplayType & 0x01)
        {
            flags |= 0x01;
        }
        if (pDtd->DisplayType & 0x02)
        {
            flags |= 0x20;
        }
        if (pDtd->DisplayType & 0x04)
        {
            flags |= 0x40;
        }
        pTiming[EDID_DTD_ADR__FLAGS] = flags;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Calculates pixel clock multiplication factor for 3D modes.
//!
//!             This function evaluates 3D_Structure_All structure
//!             that represents common 3D capabilities of formats marked as referring to this structure
//!             and decided on pixel clock multiplication factor.
//!
//! @return     clock multiplication factor.
//-------------------------------------------------------------------------------------------------

static uint8_t Comm3DPixFreqMultGet(void)
{
    decodedEdid_t *pEdidInfo = &(pEdidTx->edidInfo);
    uint8_t pix_clk_mult_3d = 1;
#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    uint16_t struct_3d_all = pEdidInfo->HDMI_VSDB.hdmi_3d_structure_all;
    if(struct_3d_all & BIT5)
    {
        // L + depth + graphics + graphics-depth: progressive only

        // multiply clock x4
        pix_clk_mult_3d = 4;
    }
    else if(struct_3d_all & (BIT0 | BIT1 | BIT2 | BIT3 | BIT4))
    {
        // Frame packing: progressive/interlaced
        // Field alternative: interlaced
        // Line alternative: progressive only
        // Side-by-Side (Full): progressive/interlaced
        // L + depth: progressive only

        // multiply clock x2
        pix_clk_mult_3d = 2;
    }
#else // SI_HDMI_1_4_EDID_PROCESSING
    pEdidInfo = pEdidInfo;
#endif // SI_HDMI_1_4_EDID_PROCESSING
    return pix_clk_mult_3d;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Protect the repeater from SDTDs with clock higher then the device can handle.
//!
//! @param[in]  aSdtds - pointer to an SDTD array to check.
//-------------------------------------------------------------------------------------------------

static void InvalidateOverclockedSdtds(shortDtd_t aSdtds[MAX_NMB_OF_KNOWN_DTDS])
{
    int i;
    uint8_t f_pix_max;
    uint8_t pix_freq_mult_3d;

    f_pix_max = pEdidTx->edidConfig.maxPixClock10Mhz * 10; // in MHz
    pix_freq_mult_3d = Comm3DPixFreqMultGet();

    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (aSdtds[i].Valid)
        {
            uint8_t idx = aSdtds[i].VideoIdx;
            uint16_t format_pix_freq = VideoModeTable[idx].PixFreq;

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
            if(aSdtds[i].struct_3d_present)
            {
                uint16_t format_pix_freq_3d_individual = format_pix_freq;
                switch(aSdtds[i].struct_3d)
                {
                case 0: // Frame packing: progressive/interlaced
                case 1: // Field alternative: interlaced
                case 2: // Line alternative: progressive only
                case 3: // Side-by-Side (Full): progressive/interlaced
                case 4: // L + depth: progressive only
                    // multiply clock x2
                    format_pix_freq_3d_individual *= 2;
                    break;

                case 5: // L + depth + graphics + graphics-depth: progressive only
                    // multiply clock x4
                    format_pix_freq_3d_individual *= 4;
                    break;

                //default: // 2D timing compatible: progressive/interlaced
                }

                if(format_pix_freq_3d_individual > f_pix_max)
                {
                    // Disable 3D capabilities for given format
                    // if the frequency condition is not met.
                    aSdtds[i].struct_3d_present = false;
                }
            }

            if(aSdtds[i].masked_3d)
            {
                uint16_t format_pix_freq_3d_common = format_pix_freq * pix_freq_mult_3d;

                // Check if 3D version of the format is within
                // allowed frequency range.
                if(format_pix_freq_3d_common > f_pix_max)
                {
                    // Disable 3D capabilities for given format
                    // if the frequency condition is not met.
                    aSdtds[i].masked_3d = false;
                }
            }
#else  // SI_HDMI_1_4_EDID_PROCESSING
            pix_freq_mult_3d = pix_freq_mult_3d;
#endif // SI_HDMI_1_4_EDID_PROCESSING

            if(format_pix_freq > f_pix_max)
            {
                // Disable video format if it requires higher
                // pixel frequency than the device can support.
                aSdtds[i].Valid = false;
            }

        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Protect the repeater from DTDs with clock higher then the device can handle.
//!
//! @param[in]  aDtds - pointer to an SDTD array to check.
//-------------------------------------------------------------------------------------------------

static void InvalidateOverclockedDtds(dtd_t aDtds[MAX_NMB_OF_UNKNOWN_DTDS])
{
    int i;
    uint16_t f_pix_max = pEdidTx->edidConfig.maxPixClock10Mhz * 1000; // in 10 kHz units

    for (i=0; i<MAX_NMB_OF_UNKNOWN_DTDS; i++)
    {
        if (aDtds[i].Valid)
            if (aDtds[i].PixClk_10kHz > f_pix_max)
                aDtds[i].Valid = false;
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Arrange SDTDs with preferred timing first and native formats next.
//!             Anything else is placed after the first two categories
//!
//! @param[in]  aSdtds   - table of SDTDs
//!
//! @return     Number of valid SDTDs
//-------------------------------------------------------------------------------------------------

static int RearrangeSdtds (shortDtd_t aSdtds[MAX_NMB_OF_KNOWN_DTDS])
{
    shortDtd_t tempSdtds[MAX_NMB_OF_KNOWN_DTDS];
    int nmbOfSdtds = 0;
    int i;

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    // HDMI 1.4a spec does not support 3D for HDMI VIC formats.
    // Although 3D flags should not be set in SDTDs that indicate
    // HDMI VIC, it is better to look through the data and clear
    // the erroneous flags. Otherwise, in case of incorrect
    // SDTD data, some logic in EDID composer module may fail.
    for (i = 0; i < MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        shortDtd_t *pSdtd = &aSdtds[i];
        if (pSdtd->HdmiVic)
        {
            // BZ#2591
//        	memset(pSdtd->indiv3dModes, 0, sizeof(pSdtd->indiv3dModes));
        	pSdtd->indiv3dModeLength = 0;
            pSdtd->masked_3d = false;
        }
    }
#endif // SI_HDMI_1_4_EDID_PROCESSING

    // Filter-out video modes having pixel clock higher then the repeater can handle
    InvalidateOverclockedSdtds(aSdtds);

    memset(tempSdtds, 0, sizeof(tempSdtds)); // clean it

    // copy an SDTD with Preferred Mode bit set
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (aSdtds[i].Valid)
            if ( aSdtds[i].PreferredTiming )
            {
                memcpy(&tempSdtds[nmbOfSdtds++], &aSdtds[i], sizeof(shortDtd_t));
                break; // because only one SDTD with "Preferred" attribute set is possible
            }
    }

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
    // then copy all Native formats that are not 3D and not Preferred
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (aSdtds[i].Valid)
            if ( (!aSdtds[i].PreferredTiming) && aSdtds[i].Native )
            {
                memcpy(&tempSdtds[nmbOfSdtds++], &aSdtds[i], sizeof(shortDtd_t));
            }
    }

    // then place 3D formats as they can only take first 16 positions (HDMI 1.4 limitation)
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (aSdtds[i].Valid)
            if ( (!aSdtds[i].PreferredTiming) && (!aSdtds[i].Native)
                && (aSdtds[i].masked_3d || (0 != pEdidTx->edidInfo.SDTD[i].indiv3dModeLength)))
                memcpy(&tempSdtds[nmbOfSdtds++], &aSdtds[i], sizeof(shortDtd_t));
    }

    // then place the rest
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (aSdtds[i].Valid)
            if ( !(aSdtds[i].PreferredTiming || aSdtds[i].Native
                || aSdtds[i].masked_3d || (0 != pEdidTx->edidInfo.SDTD[i].indiv3dModeLength)) )
            {
                memcpy(&tempSdtds[nmbOfSdtds++], &aSdtds[i], sizeof(shortDtd_t));
            }
    }
#else // SI_HDMI_1_4_EDID_PROCESSING
    // copy all Native except Native && Preferred
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (aSdtds[i].Valid)
            if ( (!aSdtds[i].PreferredTiming) && aSdtds[i].Native )
                memcpy(&tempSdtds[nmbOfSdtds++], &aSdtds[i], sizeof(shortDtd_t));
    }

    // then place the rest
    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (aSdtds[i].Valid)
            if ( !(aSdtds[i].PreferredTiming || aSdtds[i].Native) )
                memcpy(&tempSdtds[nmbOfSdtds++], &aSdtds[i], sizeof(shortDtd_t));
    }
#endif // SI_HDMI_1_4_EDID_PROCESSING

    // Finally copy to the main array
    memcpy(aSdtds, tempSdtds, sizeof(tempSdtds));

    return nmbOfSdtds;
}



//-------------------------------------------------------------------------------------------------
//! @brief      Arrange valid DTDs with preferred timing first.
//!
//! @param[in]  aDtds   - table of DTDs
//!
//! @return     Number of valid DTDs
//-------------------------------------------------------------------------------------------------

static int RearrangeDtds (dtd_t aDtds[MAX_NMB_OF_UNKNOWN_DTDS])
{
    dtd_t temp_dtds[MAX_NMB_OF_UNKNOWN_DTDS];
    int nmb_of_dtds = 0;
    int i;

    InvalidateOverclockedDtds(aDtds); // filter-out video modes with pixel clock higher then our repeater can handle

    memset(temp_dtds, 0, sizeof(temp_dtds)); // clean it

    // copy an SDTD with Preferred Mode bit set
    for (i=0; i<MAX_NMB_OF_UNKNOWN_DTDS; i++)
    {
        if (aDtds[i].Valid)
            if (aDtds[i].PreferredTiming)
            {
                memcpy(&temp_dtds[nmb_of_dtds++], &aDtds[i], sizeof(dtd_t));
                break; // because only one SDTD with "Preferred" attribute set is posible
            }
    }

    // then place the rest
    for (i=0; i<MAX_NMB_OF_UNKNOWN_DTDS; i++)
    {
        if (aDtds[i].Valid)
            if (!aDtds[i].PreferredTiming)
                memcpy(&temp_dtds[nmb_of_dtds++], &aDtds[i], sizeof(dtd_t));
    }

    // protection- this must not happened
    if (nmb_of_dtds > MAX_NMB_OF_UNKNOWN_DTDS)
        nmb_of_dtds = MAX_NMB_OF_UNKNOWN_DTDS;

    // Finally copy to the main array
    memcpy(aDtds, temp_dtds, sizeof(temp_dtds));
    return nmb_of_dtds;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Find next index in array of SDTDs structures that points to SDTD suitable for
//!             composing EDID's DTD block.
//!
//! @param[in]  pEdidInfo   - pointer to the input data structure
//! @param[in]  pSdtdIndex  - pointer to current SDTD index
//! @param[in]  nmbOfSdtds  - total number of SDTDs
//!
//! @return     true, if the next SDTD index is within valid range.
//-------------------------------------------------------------------------------------------------

static bool_t FindNextSdtdConvertibleToDtd(decodedEdid_t *pEdidInfo, uint8_t *pSdtdIndex, uint8_t nmbOfSdtds)
{
    uint8_t sdtdIndex = *pSdtdIndex;

    for(; sdtdIndex < nmbOfSdtds; sdtdIndex++)
    {
        if(pEdidInfo->SDTD[sdtdIndex].Valid)
        {
            uint8_t videoIdx = pEdidInfo->SDTD[sdtdIndex].VideoIdx;
            if(videoIdx <= NMB_OF_VIDEO_MODES)
            {
                const videoMode_t *pVideoMode = &VideoModeTable[videoIdx];
                // Check if DTD structure (block) can be made out of SDTD.
                // It is possible that number of bits allocated in the EDID 1.3's DTD block
                // is not enough. For example, HDMI VIC (4K2K) formats 2,3, and 4
                // cannot fit into the DTD structure.
                if(pVideoMode->SyncOffset.H >= 0x200)
                {
                    continue;
                }
                if(pVideoMode->Active.H >= 0x1000)
                {
                    continue;
                }
                // Note: Other fields are not currently checked as
                // it is known that the video table does not have
                // resolutions that do not fit into the DTD fields.
                break;
            }
        }
    }

    *pSdtdIndex = sdtdIndex; // return new index

    // return true if the index is valid
    return (sdtdIndex < nmbOfSdtds);
}


#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)

static uint16_t Get3dMask( void )
{
    uint8_t  i = 0;
    uint8_t  validVicCount = 0;
    uint16_t mask3d = 0;
    uint16_t slider = 0x0001;
    bool_t   isAllVicSupportMask3d = true;

    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (validVicCount > 16)
        {
            // only first 16 VICs can have 3D mask
            break;
        }
        if ( pEdidTx->edidInfo.SDTD[i].Valid)
        {
            if ( pEdidTx->edidInfo.SDTD[i].masked_3d)
            {
                mask3d |= slider;
            }
            else
            {
                isAllVicSupportMask3d = false;
            }
            validVicCount++;
            slider <<= 1;
        }
    }
    return isAllVicSupportMask3d ? 0 : mask3d;
}


static uint8_t Calculate3dLength ( bool_t isSupport3dForAnyVic )
{
    uint8_t i = 0;
    uint8_t valid_vic_count = 0;
    uint8_t length = 0;
    uint8_t validVicCount = 0;
    if ( pEdidTx->edidInfo.HDMI_VSDB.hdmi_3d_structure_all)
    {
        length += 2; // 2 bytes for 3D_structure_All

        if (!isSupport3dForAnyVic)
        {
            length += 2; // 2 bytes for 3D_MASK
        }
    }

    for (i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (valid_vic_count >= 16)
        {
            // only first 16 VICs can have 3D structure
            break;
        }
        if ( pEdidTx->edidInfo.SDTD[i].Valid)
        {
        	uint8_t j;
			Indiv3dMode_t *pIndiv3d = pEdidTx->edidInfo.SDTD[i].indiv3dModes;
			uint8_t indivLength = pEdidTx->edidInfo.SDTD[i].indiv3dModeLength;
        	for(j = 0; j < indivLength; j++)
        	{
				if (pIndiv3d[j].struct_3d_present)
				{
					// BZ#2591
					length++; // at least one byte per 3D format
					if (pIndiv3d[j].struct_3d >= 8) //HDMI VSDB 3D Details present
					{
						length++; // additional byte for 3D_Struct 8...15
					}
				}
        	}
            validVicCount++;
        }
    }

    return length;
}


static void FillIndividual3dModes ( uint8_t aEdid[EDID_BLOCK_SIZE], uint8_t *pByteIndex )
{
    uint8_t i = 0;
    uint8_t validVicCount = 0;

    for (i = 0; i < MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if (validVicCount >= 16)
        {
            // only first 16 VICs can have 3D mask
            break;
        }
        if (pEdidTx->edidInfo.SDTD[i].Valid)
        {
        	uint8_t j;
			Indiv3dMode_t *pIndiv3d = pEdidTx->edidInfo.SDTD[i].indiv3dModes;
			uint8_t indivLength = pEdidTx->edidInfo.SDTD[i].indiv3dModeLength;
        	for(j = 0; j < indivLength; j++)
        	{
				if (pIndiv3d[j].struct_3d_present)
				{
	                // at least one byte per 3D format
	                aEdid[*pByteIndex] = validVicCount << 4;
	                aEdid[*pByteIndex] |= pIndiv3d[j].struct_3d;
	                (*pByteIndex)++;
	                if (pIndiv3d[j].struct_3d >= 8) //HDMI VSDB 3D Details present
	                {
	                    // additional byte for 3D_Struct 8...15
	                    aEdid[*pByteIndex] = pIndiv3d[j].detail_3d << 4;
	                    (*pByteIndex)++;
	                }
				}
        	}
        	validVicCount++;
        }
    }
}
#endif  //SI_HDMI_1_4_EDID_PROCESSING


static ROM const uint8_t t_sec_formula[8] = {01, 11, 19, 65, 13, 03, 19, 67};

//-------------------------------------------------------------------------------------------------
//! @brief      Compose block 0 of a new EDID based on the Upstream EDID and
//!             repeater configuration.
//!
//!             Re-arranges the EDID into proper format if necessary, adds
//!             missing fields, replaces manufacturer ID, etc., with new values, and
//!             possibly other stuff.
//!
//! @param[out] pEdid               - pointer to EDID block,
//! @param[in]  pDtdIndex           - pointer to DTD structures,
//! @param[in]  pSdtdIndex          - pointer to SDTD structures,
//! @param[in]  nmbOfNativeFormats  - a number of native DTDs to be displayed.
//!
//! @return     Success flag.
//-------------------------------------------------------------------------------------------------


static bool_t ComposeEdidBlock0( uint8_t *pEdid, uint8_t *pDtdIndex, uint8_t *pSdtdIndex,
                                 uint8_t nmbOfSdtds, uint8_t *nmbOfNativeFormats  )
{
    uint8_t dtdIndex = *pDtdIndex;
    uint8_t sdtdIndex = *pSdtdIndex;
    bool_t  isSdtdIndexValid;

    // Copy header, version number, manufacturer ID...
    memcpy( pEdid, defaultDsEdid, EDID_BL0_ADR__BASIC_DISPLAY_PARAMS);

    // Replace certain default values from customer configuration

    pEdid[ EDID_BL0_ADR__MANUFACTURER_ID ]      = (uint8_t)(pEdidTx->edidConfig.mfgId >> 8);
    pEdid[ EDID_BL0_ADR__MANUFACTURER_ID + 1]   = (uint8_t)(pEdidTx->edidConfig.mfgId);

    pEdid[ EDID_BL0_ADR__PRODUCT_ID ]           = (uint8_t)(pEdidTx->edidConfig.productId >> 8);
    pEdid[ EDID_BL0_ADR__PRODUCT_ID + 1 ]       = (uint8_t)(pEdidTx->edidConfig.productId);

    pEdid[EDID_BL0_ADR__SHORT_SERIAL_NUMBER + 0]    = (uint8_t)(pEdidTx->edidConfig.serialNumber >> 24);
    pEdid[EDID_BL0_ADR__SHORT_SERIAL_NUMBER + 1]    = (uint8_t)(pEdidTx->edidConfig.serialNumber >> 16);
    pEdid[EDID_BL0_ADR__SHORT_SERIAL_NUMBER + 2]    = (uint8_t)(pEdidTx->edidConfig.serialNumber >>  8);
    pEdid[EDID_BL0_ADR__SHORT_SERIAL_NUMBER + 3]    = (uint8_t)(pEdidTx->edidConfig.serialNumber >>  0);
    pEdid[EDID_BL0_ADR__MANUFACTURE_WEEK]           = (uint8_t)(pEdidTx->edidConfig.mfgWeek);
    pEdid[EDID_BL0_ADR__MANUFACTURE_YEAR]           = (uint8_t)(pEdidTx->edidConfig.mfgYear);

    // Replace other defaults with the decoded info from SiiEdidTxDecode().

    memcpy( &pEdid[EDID_BL0_ADR__BASIC_DISPLAY_PARAMS], pEdidTx->edidInfo.BasicDispayParameters, EDID_BL0_LEN__BASIC_DISPLAY_PARAMS);
    memcpy( &pEdid[EDID_BL0_ADR__CHROMATICITY], pEdidTx->edidInfo.Chromaticity, EDID_BL0_LEN__CHROMATICITY);
    pEdid[EDID_BL0_ADR__ESTABLISHED_TIMING_1] = pEdidTx->edidInfo.EstablishedTiming1;
    pEdid[EDID_BL0_ADR__ESTABLISHED_TIMING_2] = pEdidTx->edidInfo.EstablishedTiming2;
    pEdid[EDID_BL0_ADR__MANUFACTURER_RESERVED_TIMING] = pEdidTx->edidInfo.ManufacturerReservedTiming;
    memcpy( &pEdid[EDID_BL0_ADR__STANDARD_TIMING], pEdidTx->edidInfo.StandardTiming, EDID_BL0_LEN__STANDARD_TIMING);

    pEdid[EDID_BL0_ADR__BASIC_DISPLAY_PARAMS + 4] |= 0x02; // set the Preferred Bit
    pEdid[EDID_BL0_ADR__BASIC_DISPLAY_PARAMS + 4] &= 0x1F;
    pEdid[EDID_BL0_ADR__BASIC_DISPLAY_PARAMS + 4] |= REPEATER_POWER_FEATURES;
    pEdid[EDID_BL0_ADR__BASIC_DISPLAY_PARAMS + 0] |= 0x80; // Digital Interface
    pEdid[EDID_BL0_ADR__BASIC_DISPLAY_PARAMS + 0] &= 0x81; // clear reserved bits

    // Confirm currently processing SDTD can be used to build EDID's DTD block.
    // If it is not usable for this purpose, find the next usable.
    isSdtdIndexValid = FindNextSdtdConvertibleToDtd(&(pEdidTx->edidInfo), &sdtdIndex, nmbOfSdtds);

    // use DTD1 place for the Preferred Detail Timing
    if ( pEdidTx->edidInfo.DTD[ dtdIndex].Valid && pEdidTx->edidInfo.DTD[ dtdIndex].PreferredTiming)
    {
        // if DTD[0] is the Preferred one
        MakeDtdFromDtd( &pEdid[EDID_BL0_ADR__DTD1], &pEdidTx->edidInfo.DTD[ dtdIndex++]);
    }

    else if ( isSdtdIndexValid && pEdidTx->edidInfo.SDTD[ sdtdIndex].Valid &&
              (pEdidTx->edidInfo.SDTD[ sdtdIndex].PreferredTiming || pEdidTx->edidInfo.SDTD[ sdtdIndex].Native))
    {
        // if SDTD[0] is the Preferred one or at least Native
        MakeDtdFromSdtd( &pEdid[EDID_BL0_ADR__DTD1], &pEdidTx->edidInfo.SDTD[sdtdIndex++]);
    }
    else if ( pEdidTx->edidInfo.DTD[ dtdIndex].Valid)
    {
        // if DTD[0] is existing at least
        MakeDtdFromDtd( &pEdid[EDID_BL0_ADR__DTD1], &pEdidTx->edidInfo.DTD[dtdIndex++]);
    }
    else if ( isSdtdIndexValid && pEdidTx->edidInfo.SDTD[ sdtdIndex].Valid)
    {
        // if SDTD[0] is existing at least
        pEdidTx->edidInfo.SDTD[ sdtdIndex].PreferredTiming = true; // will be used during SVD forming in Data Block Collection
        MakeDtdFromSdtd( &pEdid[EDID_BL0_ADR__DTD1], &pEdidTx->edidInfo.SDTD[ sdtdIndex++]);
    }
    else
    {
        // noting found- use default one
        memcpy( &pEdid[EDID_BL0_ADR__DTD1], &defaultDsEdid[EDID_BL0_ADR__DTD1], EDID_LEN__DTD);
    }

    // Confirm currently processing SDTD can be used to build EDID's DTD block.
    // If it is not usable for this purpose, find the next usable.
    isSdtdIndexValid = FindNextSdtdConvertibleToDtd(&(pEdidTx->edidInfo), &sdtdIndex, nmbOfSdtds);

    // use DTD2 place for another Detail Timing
    if ( isSdtdIndexValid && pEdidTx->edidInfo.SDTD[ sdtdIndex].Valid && pEdidTx->edidInfo.SDTD[ sdtdIndex].Native)
    {
        // if SDTD[sdtd_idx] is native
        MakeDtdFromSdtd( &pEdid[EDID_BL0_ADR__DTD2], &pEdidTx->edidInfo.SDTD[ sdtdIndex++]);
        (*nmbOfNativeFormats)++;
    }
    else if ( pEdidTx->edidInfo.DTD[ dtdIndex].Valid)
    {
        // if DTD[dtd_idx] is existing
        MakeDtdFromDtd( &pEdid[EDID_BL0_ADR__DTD2], &pEdidTx->edidInfo.DTD[ dtdIndex++]);
    }
    else if ( isSdtdIndexValid && pEdidTx->edidInfo.SDTD[ sdtdIndex].Valid)
    {
        // if SDTD[sdtd_idx] is existing
        MakeDtdFromSdtd( &pEdid[EDID_BL0_ADR__DTD2], &pEdidTx->edidInfo.SDTD[ sdtdIndex++]);
    }
    else
    {
        // nothing found - fill with a dummy string descriptor
        memcpy( &pEdid[EDID_BL0_ADR__DTD2], &defaultDsEdid[DEFAULT_EDID_BL0_ADR__DUMMY_STRING], EDID_LEN__DTD);
    }

    // use DTD3 place for a Monitor Name
    memcpy( &pEdid[EDID_BL0_ADR__DTD3], &defaultDsEdid[DEFAULT_EDID_BL0_ADR__MONITOR_NAME], EDID_LEN__DTD);

    // use DTD4 place for a Range Limits structure
    memcpy( &pEdid[EDID_BL0_ADR__DTD4], &defaultDsEdid[DEFAULT_EDID_BL0_ADR__RANGE_LIMITS], EDID_LEN__DTD);
    if ( pEdidTx->edidInfo.DescriptorValid.RangeLimits)
    {
        uint8_t f_pix_max = pEdidTx->edidConfig.maxPixClock10Mhz; // in 10 MHz units

        if ( pEdidTx->edidInfo.RangeLimits.MaxPixClk_10MHz > f_pix_max)
        {
            pEdidTx->edidInfo.RangeLimits.MaxPixClk_10MHz = f_pix_max;
        }
        pEdid[EDID_BL0_ADR__DTD4 + 5] = pEdidTx->edidInfo.RangeLimits.MinV_Hz;
        pEdid[EDID_BL0_ADR__DTD4 + 6] = pEdidTx->edidInfo.RangeLimits.MaxV_Hz;
        pEdid[EDID_BL0_ADR__DTD4 + 7] = pEdidTx->edidInfo.RangeLimits.MinH_kHz;
        pEdid[EDID_BL0_ADR__DTD4 + 8] = pEdidTx->edidInfo.RangeLimits.MaxH_kHz;
        pEdid[EDID_BL0_ADR__DTD4 + 9] = pEdidTx->edidInfo.RangeLimits.MaxPixClk_10MHz;
        memcpy( &pEdid[EDID_BL0_ADR__DTD4 + 10], pEdidTx->edidInfo.RangeLimits.SecondaryTimingFormula, 8);
        if ( !memcmp( &pEdid[EDID_BL0_ADR__DTD4 + 10], t_sec_formula, sizeof(t_sec_formula)))
        {
            pEdid[0x08] = 0x4D;
            pEdid[0x09] = 0x29;
        }
    }

    if ( pEdidTx->usEdidBlockCount > 128) // current version limitation
    {
        pEdidTx->edidInfo.ErrorCode |= EDID_ERROR__TOO_MANY_EXTENSIONS;
        pEdidTx->usEdidBlockCount = 128;
    }

    // write number of extensions
    pEdid[ EDID_BL0_ADR__EXTENSION_NMB] = pEdidTx->usEdidBlockCount ? (pEdidTx->usEdidBlockCount - 1) : 0;

    // calculate check sum
    pEdid[EDID_ADR__CHECK_SUM] = EdidCheckSumCalc( pEdid );

    // Return index values
    *pDtdIndex = dtdIndex;
    *pSdtdIndex = sdtdIndex;

    // save block 0
    if ( !EdidTxWriteBlockToRx(0, pEdid, 0))
    {
        pEdidTx->edidInfo.ErrorCode = EDID_ERROR__CANNOT_WRITE;
        return( false );
    }

    return( true );
}


//-------------------------------------------------------------------------------------------------
//! @brief      Compose (encode) an HDMI EDID block based on the Downstream EDID
//!
//! @param[out] pEdid - pointer to the output EDID buffer
//! @param[in]  byte_idx_in
//-------------------------------------------------------------------------------------------------

static void ComposeHdmiEdidBlock ( uint8_t *pEdid, uint8_t *pByteIndexIn, uint8_t *nmbOfSdtds, uint8_t *pCecPhysAddrLocation )
{
    uint8_t byteIndex = *pByteIndexIn;
    uint8_t header_idx;
    uint8_t cntr = 0;
    uint8_t errorPosition = 0;

    BEGIN_BOUNDARY_CHECK

    // Make data collection

    // add a Video Data Block
    header_idx = byteIndex++;
    if ( *nmbOfSdtds > 31)
    {
        *nmbOfSdtds = 31;
    }
    errorPosition = 1;
    CHECK_BOUNDARY(byteIndex + 1 + *nmbOfSdtds);
#if (MHL_20 == ENABLE)
    pEdidTx->edidInfo.numOfVic = *nmbOfSdtds;
#endif
    if ( *nmbOfSdtds )
    {
        uint8_t i;
        for ( i=0; i < *nmbOfSdtds; i++)
        {
            uint8_t vic = pEdidTx->edidInfo.SDTD[i].Vic;
            if(vic) // condition to exclude HDMI VICs
            {
                if ( pEdidTx->edidInfo.SDTD[i].Native || pEdidTx->edidInfo.SDTD[i].PreferredTiming )
                {
                    vic |= 0x80;
                }
                pEdid[byteIndex++] = vic;
                cntr++;
            }
        }
    }
    else
    {
        pEdid[byteIndex++] = DEFAULT_VIC | 0x80; // (must be native/preferred)
        cntr = 1;
    }
    pEdid[header_idx] = (EDID_DBC_TAG__VIDEO << 5) | cntr;

    // add a Short Audio Data Block
#ifdef COMPOSE_SAD_SEPARATELY
    if ( pEdidTx->edidInfo.SupportAudio && (0 != pEdidTx->edidInfo.SadCount))
    {
        uint8_t nmbOfSads = pEdidTx->edidInfo.SadCount;
        errorPosition = 2;
        CHECK_BOUNDARY(byteIndex + nmbOfSads * 4);
        for (cntr=0; cntr<nmbOfSads; cntr++)
        {
            if ( pEdidTx->edidInfo.SAD[cntr].Common.Coding)
            {
                pEdid[byteIndex++] = (EDID_DBC_TAG__AUDIO << 5) | 3;
                pEdid[byteIndex++] = pEdidTx->edidInfo.SAD[cntr].ByteData.Byte1;
                pEdid[byteIndex++] = pEdidTx->edidInfo.SAD[cntr].ByteData.Byte2;
                pEdid[byteIndex++] = pEdidTx->edidInfo.SAD[cntr].ByteData.Byte3;
            }
        }
    }
#else // COMPOSE_SAD_SEPARATELY
    if ( pEdidTx->edidInfo.SupportAudio && (0 != pEdidTx->edidInfo.SadCount))
    {
        uint8_t nmb_of_sads = pEdidTx->edidInfo.SadCount;
        errorPosition = 3;
        CHECK_BOUNDARY(byteIndex + nmb_of_sads * EDID_SAD__LENGTH + 1);
        if (nmb_of_sads > MAX_NMB_OF_SADS) // could be true only in case of internal error
            nmb_of_sads = MAX_NMB_OF_SADS;
#if MAX_NMB_OF_SADS > 10
#  error MAX_NMB_OF_SADS * EDID_SAD__LENGTH > 2^5
#endif
        pEdid[byteIndex++] = (EDID_DBC_TAG__AUDIO << 5) | (nmb_of_sads * EDID_SAD__LENGTH);
        for (cntr=0; cntr<nmb_of_sads; cntr++)
        {
            if ( pEdidTx->edidInfo.SAD[cntr].Common.Coding)
            {
                pEdid[byteIndex++] = pEdidTx->edidInfo.SAD[cntr].ByteData.Byte1;
                pEdid[byteIndex++] = pEdidTx->edidInfo.SAD[cntr].ByteData.Byte2;
                pEdid[byteIndex++] = pEdidTx->edidInfo.SAD[cntr].ByteData.Byte3;
            }
        }
    }
#endif // COMPOSE_SAD_SEPARATELY

    // add a Speaker Allocation Data Block
    if ( pEdidTx->edidInfo.SupportAudio && (0 != pEdidTx->edidInfo.SPK.Data) && (0 != pEdidTx->edidInfo.SadCount))
    {
        errorPosition = 4;
        CHECK_BOUNDARY(byteIndex + 4);
        pEdid[byteIndex++] = (EDID_DBC_TAG__SPEAKER_ALLOCATION << 5) | 3; // header
        pEdid[byteIndex++] = pEdidTx->edidInfo.SPK.Data;
        pEdid[byteIndex++] = 0;
        pEdid[byteIndex++] = 0;
    }

    // add an HDMI Vendor Specific Data Block
    {
        hdmiVsdb_t *pVsdb = &pEdidTx->edidInfo.HDMI_VSDB;
        bool_t add_hdmi_1_4_fields = false;
        uint8_t h3d_len = 0; // 3D data length
        uint8_t hdmi_vic_len = 0; // Number of HDMI VICs (for 4K2K)
#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
        uint8_t hdmi_video_flags = pEdidTx->edidInfo.HDMI_VSDB.image_size << 3;

        // Note: hdmi_video_flags bit 7 is set if
        // DS device support 3D. Only in this case
        // US EDID is allowed to have any 3D mode.
        // Bits 5...6 will be overridden in this function.
        // Bits 2...4 will be kept as is.
        // Bits 0...1 will be cleared as they are reserved
        // in HDMI 1.4 and have to be 0.
        uint16_t mask_3d = 0;
#endif // SI_HDMI_1_4_EDID_PROCESSING
        uint8_t extra_len = 3; // HDMI 1.3a extras

        if (pVsdb->Extras[2] & 0x80)
        {
            extra_len +=2;
            if (pVsdb->Extras[2] & 0x40)
                extra_len +=2;
        }

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)

        // Count number of HDMI VIC formats
        for(cntr = 0; cntr < MAX_NMB_OF_HDMI_VIC_FORMATS; cntr++)
        {
            if(pEdidTx->pEdidDecodData->hdmi_vic[cntr])
            {
                hdmi_vic_len++;
                // The number of VICs is limited to 7 by HDMI 1.4a spec.
                if(hdmi_vic_len >= MAX_NMB_OF_HDMI_VIC_FORMATS)
                    break;
            }
        }

        if (pEdidTx->isHdmi3DEnabled && (pEdidTx->edidConfig.maxPixClock10Mhz > 14))
        {
            // Declare upstream 3D capabilities if downstream device
            // supports 3D.
            // Note: at list 148 MHz pixel clock support is required for
            // some of the mandatory 3D formats.
            if(pEdidTx->edidInfo.HDMI_VSDB.hdmi_3d_present)
            {
                hdmi_video_flags |= 0x80;
            }
        }

        // If there is a 3D or Image_Size data,
        // add HDMI Video Flags plus the length fields.
        if(hdmi_video_flags)
        {
            add_hdmi_1_4_fields = true;
            // Check if 3D is allowed at the input
            mask_3d = Get3dMask();
#if (MHL_20 ==ENABLE)
            pEdidTx->edidInfo.HDMI_VSDB.mask_3d_support = mask_3d;
#endif
            // Calculate what would be 3D data size
            h3d_len = Calculate3dLength( 0 == mask_3d);        }

        // Do not add extra 3D info and/or HDMI VICs
        // if there is not enough space for it.
        if(
            (hdmi_vic_len || h3d_len) &&
            (((byteIndex + h3d_len + hdmi_vic_len) >= (EDID_ADR__CHECK_SUM - 2)) ||
            ((h3d_len + extra_len + hdmi_vic_len) >
                (30 - 2 - EDID_VSB_LEN__MIN_HDMI_VSDB)))
            )
        {

            // Note: 2 here reserves space for additional 2 bytes
            // needed as for 3Dm as for HDMI VIC.
            h3d_len = 0;

            // Check if it is fits now. If it does not, try cutting off HDMI VICs.
            if((byteIndex + h3d_len + hdmi_vic_len) >= (EDID_ADR__CHECK_SUM - 2))
            {
                // Do not add HDMI VICs.
                hdmi_vic_len = 0;
            }
        }

        if(h3d_len || hdmi_vic_len)
        {
            add_hdmi_1_4_fields = true;
        }

#endif // SI_HDMI_1_4_EDID_PROCESSING

        errorPosition = 5;
        CHECK_BOUNDARY(byteIndex + (1 + EDID_VSB_LEN__MIN_HDMI_VSDB + extra_len + h3d_len +
                       hdmi_vic_len + (add_hdmi_1_4_fields ? 2 : 0) ));

        pEdid[byteIndex++] = // header
            (EDID_DBC_TAG__VENDOR_SPECIFIC << 5) |
            (EDID_VSB_LEN__MIN_HDMI_VSDB + extra_len + h3d_len +
                                           hdmi_vic_len + (add_hdmi_1_4_fields ? 2 : 0));
        pEdid[byteIndex++] = 0x03; // HDMI signature byte 1
        pEdid[byteIndex++] = 0x0C; // HDMI signature byte 2
        pEdid[byteIndex++] = 0x00; // HDMI signature byte 3
        *pCecPhysAddrLocation = byteIndex;

#if ( configSII_DEV_953x_PORTING == 0 )        
        pEdid[byteIndex++] = (pVsdb->CEC.a << 4) | pVsdb->CEC.b; // CEC address a,b
        pEdid[byteIndex++] = (pVsdb->CEC.c << 4) | pVsdb->CEC.d; // CEC address c,d
#else
        pEdid[byteIndex++] = (pVsdb->cec.a << 4) | pVsdb->cec.b; // CEC address a,b
        pEdid[byteIndex++] = (pVsdb->cec.c << 4) | pVsdb->cec.d; // CEC address c,d
#endif 

        memcpy( &pEdid[byteIndex], pVsdb->Extras, extra_len);
        if ( add_hdmi_1_4_fields )
        {
            // Add HDMI_Video_present flag
            pEdid[byteIndex+2] |= 0x20;
        }
        else
        {
            // Remove HDMI_Video_present flag
            pEdid[byteIndex+2] &= ~0x20;
        }
        // Note: extra_len is at least 3
        byteIndex += extra_len;

#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
        if (add_hdmi_1_4_fields)
        {
            /*
            3D_Structure_All
            bit 0:  Frame packing: progressive/interlaced
            bit 1:  Field alternative: interlaced
            bit 2:  Line alternative: progressive only
            bit 3:  Side-by-Side (Full): progressive/interlaced
            bit 4:  L + depth: progressive only
            bit 5:  L + depth + graphics + graphics-depth: progressive only
            bit 6:  Top-and-Bottom
            bit 8:  Side-by-Side (Half) with horizontal subsampling
            bit 15: Side-by-Side (Half) with quincunx subsampling
            */

            uint16_t hdmi_3d_structure_all = pVsdb->hdmi_3d_structure_all;

            if ( hdmi_3d_structure_all )
            {
                // Note: 3D_Multi_present = 0 at this point.
                if ( mask_3d )
                {
                    // Set 3D_Multi_present = 2
                    hdmi_video_flags |= 0x40;
                }
                else
                {
                    // Set 3D_Multi_present = 1
                    hdmi_video_flags |= 0x20;
                }
            }
            pEdid[byteIndex++] = hdmi_video_flags;

            // Add the 3D length field.
            pEdid[byteIndex++] = h3d_len + (hdmi_vic_len << 5);

            if(hdmi_vic_len)
            {
                for(cntr = 0; cntr < MAX_NMB_OF_HDMI_VIC_FORMATS; cntr++)
                {
                    uint8_t hdmi_vic = pEdidTx->pEdidDecodData->hdmi_vic[cntr];
                    if(hdmi_vic)
                    {
                        pEdid[byteIndex++] = hdmi_vic;
                        hdmi_vic_len--;
                        if(0 == hdmi_vic_len)
                            break;
                    }
                }
            }

            if(h3d_len)
            {
                if ( hdmi_3d_structure_all )
                {
                    pEdid[byteIndex++] = (uint8_t) (hdmi_3d_structure_all >> 8);
                    pEdid[byteIndex++] = (uint8_t) (hdmi_3d_structure_all);
                    if ( mask_3d )
                    {
                        pEdid[byteIndex++] = (uint8_t) (mask_3d >> 8);
                        pEdid[byteIndex++] = (uint8_t) (mask_3d);
                    }
                }

                FillIndividual3dModes( pEdid, &byteIndex);
            }
        }
#endif // SI_HDMI_1_4_EDID_PROCESSING
    }

    // add a Colorimetry Data Block
    if ( pEdidTx->edidInfo.SupportYCbCr && ( pEdidTx->edidInfo.xv601 || pEdidTx->edidInfo.xv709))
    {
        errorPosition = 6;
        CHECK_BOUNDARY(byteIndex + 4);

        // 0xE3 // tag (7) + length (3)
        // 0x05 // extended tag code
        // 0x01 for xvYCC601 or 0x02 xvYCC709 or 0x03 for both
        // 0x01 // profile 0

        pEdid[byteIndex++] = (EDID_DBC_TAG__USE_EXTENDED_TAG << 5) | 3; // header
        pEdid[byteIndex++] = 0x05;
        pEdid[byteIndex++] = ( pEdidTx->edidInfo.xv601 ? 0x01 : 0) | (pEdidTx->edidInfo.xv709 ? 0x02 : 0);
        pEdid[byteIndex++] = 0x01; // P0 profile
    }

#ifdef COLLECT_EXTENDED_DATA_BLOCKS
    // add an Extended Audio Data Block
    if ( pEdidTx->edidInfo.UnknownAudioExt_Idx)
    {
        uint8_t n = 0;
        uint8_t buf_size = pEdidTx->edidInfo.UnknownAudioExt_Idx;
        if (buf_size > SIZE_OF_UNKNOWN_AUDIO_EXTENDED_BLOCKS_BUFFER)
            buf_size = SIZE_OF_UNKNOWN_AUDIO_EXTENDED_BLOCKS_BUFFER;
        errorPosition = 7;
        CHECK_BOUNDARY(byteIndex + buf_size);
        while(n < buf_size)
        {
            uint8_t length = pEdidTx->edidInfo.UnknownAudioExtBlocks[n++];
            if ( (n + length + 1) > buf_size )
                break; // protection
            pEdid[byteIndex++] = (EDID_DBC_TAG__USE_EXTENDED_TAG << 5) | length;
            memcpy(&pEdid[byteIndex], &pEdidTx->edidInfo.UnknownAudioExtBlocks[n], length);
            n += length;
        }
    }

    // add an Extended Video Data Block
    if ( pEdidTx->edidInfo.UnknownVideoExt_Idx)
    {
        uint8_t n = 0;
        uint8_t buf_size = pEdidTx->edidInfo.UnknownVideoExt_Idx;
        if ( buf_size > SIZE_OF_UNKNOWN_VIDEO_EXTENDED_BLOCKS_BUFFER)
        {
            buf_size = SIZE_OF_UNKNOWN_VIDEO_EXTENDED_BLOCKS_BUFFER;
        }
        errorPosition = 8;
        CHECK_BOUNDARY(byteIndex + buf_size);
        while ( n < buf_size )
        {
            uint8_t length = pEdidTx->edidInfo.UnknownVideoExtBlocks[n++];
            if ( (n + length + 1) > buf_size )
                break; // protection
            pEdid[byteIndex++] = (EDID_DBC_TAG__USE_EXTENDED_TAG << 5) | length;
            memcpy( &pEdid[byteIndex], &pEdidTx->edidInfo.UnknownVideoExtBlocks[n], length);
            n += length;
        }
    }
#endif // COLLECT_EXTENDED_DATA_BLOCKS

    END_BOUNDARY_CHECK

    *pByteIndexIn = byteIndex;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Calculate dtdOpt which will be used for all extension blocks
//!             without changing.
//!
//!             edid[3] (i.e. dtdOpt) has to be filled with number of
//!             native formats. But this value is unknown until all
//!             blocks are filled with DTDs and number of Native
//!             DTDs is counted. The problem to count before putting
//!             all the DTDs into all EDID blocks is in EDID size
//!             limitation- it is not clear how many slots are available.
//!             To find out this value, the algorithm is trying to predict it.
//-------------------------------------------------------------------------------------------------

static void ComposeDtdOptEdid ( uint8_t *pEdid, uint8_t extensionIndex, uint8_t *pDtdOptIn,
                                uint8_t *nmbOfSdtds, uint8_t *nmbOfNativeFormats )
{
    int     i;
    uint8_t dtdOpt = *pDtdOptIn;

    // predict how many DTDs could be allocated
    int totalNmbOfTimingDts =
        (pEdidTx->usEdidBlockCount - 1 - extensionIndex) * 6 +          // DTD slots in the rest of extensions
        (EDID_ADR__CHECK_SUM - pEdid[EDID_EXT_ADR__D]) / EDID_LEN__DTD  // DTD slots in the extension block (i.e current block)
        + 2; // Timing DTDs in block 0

    // count how many Natives we have
    int required_nmb_of_natives = 0;
    for ( i=0; i< *nmbOfSdtds; i++)
    {
        if ( pEdidTx->edidInfo.SDTD[i].Native || pEdidTx->edidInfo.SDTD[i].PreferredTiming)
        {
            required_nmb_of_natives++;
        }
    }
    // Consider the case when the Preferred is placed into pEdidTx->edidInfo.DTD[0]

    if ( pEdidTx->edidInfo.DTD[0].Valid && pEdidTx->edidInfo.DTD[0].PreferredTiming)
    {
        required_nmb_of_natives++;
    }

    // If number of Natives is more then the predicted value,
    // nmb_of_native_formats = the predicted value.
    if ( required_nmb_of_natives > totalNmbOfTimingDts)
    {
        *nmbOfNativeFormats = totalNmbOfTimingDts;
    }
    // If some of the Natives has to be placed into extension blocks,
    // use the value we just predicted.
    else if ( *nmbOfNativeFormats > 2)
    {
        *nmbOfNativeFormats = required_nmb_of_natives;
    }
    // If the number as small that could be placed in block 0 only,
    // nmb_of_native_formats is already the searching value.
    // The predicted value is not good in this case because
    // it could be not exact and could be even 0.

    // make dtdOpt byte which is the same for all extensions
    if ( *nmbOfNativeFormats > 15) // only 4 bits are allocated for it
    {
        *nmbOfNativeFormats = 0; // "information is not provided"
    }
    dtdOpt = *nmbOfNativeFormats;
    if ( pEdidTx->edidInfo.HDMI_VSDB.HdmiMode)
    {
        // HDMI input
        // suppose our repeater always supports audio and YCbCr
        // on HDMI inputs (but not on DVI ones)
        if ( pEdidTx->edidInfo.SupportYCbCr )
        {
            dtdOpt |= 0x30; // Support: YCbCr444, YCbCr422
        }
        if ( pEdidTx->edidInfo.SupportAudio)
        {
            dtdOpt |= 0x40; // Support: Basic Audio
        }
    }
    if ( pEdidTx->edidInfo.SupportUnderscan)
    {
        dtdOpt |= 0x80;
    }

    *pDtdOptIn = dtdOpt;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Create an EDID map page (used for EDIDs longer than 2 blocks).
//!             longer
//!
//! @return     true: success, false: error occurred writing EDID
//-------------------------------------------------------------------------------------------------

static bool_t CreateMapPage ( void )
{
    uint8_t aEdid[EDID_BLOCK_SIZE];
    int i;
    int i_max = pEdidTx->usEdidBlockCount - 1; // NmbOfUsEdidBlocks is already limited to 128 max

    // Clear EDID and add one byte of EDID_EXT_TAG__ADDITIONAL_TIMINGS for each
    // extended block.

    memset(aEdid, 0, sizeof(aEdid));
    aEdid[0] = EDID_EXT_TAG__BLOCK_MAP;
    for (i = 1; i < i_max; i++)
    {
        aEdid[i] = EDID_EXT_TAG__ADDITIONAL_TIMINGS;
    }

    // Finish up by calculating checksum and writing the EDID

    aEdid[EDID_ADR__CHECK_SUM] = EdidCheckSumCalc(aEdid);
    if (!EdidTxWriteBlockToRx(1, aEdid, 0))
    {
        pEdidTx->edidInfo.ErrorCode = EDID_ERROR__CANNOT_WRITE;
        return (false);
    }

    return (true);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Create a new EDID based on the Downstream EDID and the repeater configuration.
//!
//!             Re-arranges the EDID into proper format if necessary, adds
//!             missing fields, replaces manufacture ID, etc., with new values, and
//!             possibly other stuff.
//!
//-------------------------------------------------------------------------------------------------

void EdidTxCompose ( void )
{
    uint8_t aEdid[EDID_BLOCK_SIZE]; // !!! make sure stack has enough room for 2*EDID_BLOCK_SIZE=256 byte
    uint8_t extensionIndex = 1;
    uint8_t dtdOpt = 0;
    uint8_t nmbOfNativeFormats = 1; // Shows how many native DTDs are displayed. At least one preferred (i.e. native) DTD should exist.
    uint8_t dtdIndex = 0;
    uint8_t sdtdIndex = 0;

    uint8_t nmbOfSdtds = RearrangeSdtds(pEdidTx->edidInfo.SDTD);
    uint8_t nmbOfDtds = RearrangeDtds(pEdidTx->edidInfo.DTD);

    DEBUG_PRINT( EDID_TX_MSG_DBG, "US blocks wrote: %d\n", (int)pEdidTx->usEdidBlockCount );

    pEdidTx->edidInfo.ErrorCode = EDID_ERROR__OK;

    if (!ComposeEdidBlock0(aEdid, &dtdIndex, &sdtdIndex, nmbOfSdtds, &nmbOfNativeFormats))
    {
        return;
    }

    // Create a map page as page 1 if more than 2 EDID blocks

    if (pEdidTx->usEdidBlockCount > 2)
    {
        if (!CreateMapPage())
        {
            return;
        }
        extensionIndex = 2;
    }

    // Create extension blocks if needed

    if (pEdidTx->usEdidBlockCount > 1)
    {
    	
#if ( configSII_DEV_953x_PORTING == 1 ) /*Smith fixed EDID CTS fail for DVI_MinFormats.EDD*/           
        if (pEdidTx->edidInfo.HDMI_VSDB.HdmiMode)
        {
#endif
			int i_max = pEdidTx->usEdidBlockCount; // Already limited to 128 max
			bool_t isSdtdIndexValid;
			bool_t isFirstExtension = true;
			uint8_t cecPhysAddrPosition = 0;

	        for (; extensionIndex < i_max; extensionIndex++)
	        {
	            uint8_t byteIdx = EDID_EXT_ADR__DBC; // body start position at all extension blocks

	            // Set up fixed header stuff

	            memset(aEdid, 0, sizeof(aEdid));
	            aEdid[EDID_EXT_ADR__TAG] = EDID_EXT_TAG__ADDITIONAL_TIMINGS;
	            aEdid[EDID_EXT_ADR__REVISION] = 3;

	            // If an HDMI EDID, create the special HDMI first extension block
	            if (pEdidTx->edidInfo.HDMI_VSDB.HdmiMode)
	            {
	                if (isFirstExtension)
	                {
	                    ComposeHdmiEdidBlock(aEdid, &byteIdx, &nmbOfSdtds, &cecPhysAddrPosition);
	                }
	            }

	            // Create DTDs extension blocks
	            aEdid[EDID_EXT_ADR__D] = byteIdx;
	            for (; byteIdx <= (EDID_ADR__CHECK_SUM - EDID_LEN__DTD); byteIdx += EDID_LEN__DTD)
	            {
	                // Find SDTD that can be used to build EDID's DTD block
	                isSdtdIndexValid = FindNextSdtdConvertibleToDtd(&(pEdidTx->edidInfo), &sdtdIndex, nmbOfSdtds);

	                // while there is free space for DTDs
	                if (isSdtdIndexValid && pEdidTx->edidInfo.SDTD[sdtdIndex].Native)
	                {
	                    // if SDTD[sdtd_idx] is native
	                    MakeDtdFromSdtd(&aEdid[byteIdx], &pEdidTx->edidInfo.SDTD[sdtdIndex++]);
	                    if (isFirstExtension)
	                    {
	                        nmbOfNativeFormats++;
	                    }
	                }
	                else if (dtdIndex < nmbOfDtds)
	                {
	                    MakeDtdFromDtd(&aEdid[byteIdx], &pEdidTx->edidInfo.DTD[dtdIndex++]);
	                }
	                else if (isSdtdIndexValid)
	                {
	                    MakeDtdFromSdtd(&aEdid[byteIdx], &pEdidTx->edidInfo.SDTD[sdtdIndex++]);
	                }
	                else
	                    break;
	            }

	            // Calculate dtdOpt which will be used for all extension blocks
	            // without changing.
	            if (isFirstExtension)
	            {
	                ComposeDtdOptEdid(aEdid, extensionIndex, &dtdOpt, &nmbOfSdtds, &nmbOfNativeFormats);
	            }
	            aEdid[EDID_EXT_ADR__DTD_OPT] = dtdOpt;

	            // Finish up by calculating checksum and writing the EDID

	            aEdid[EDID_ADR__CHECK_SUM] = EdidCheckSumCalc(aEdid);
	            if (!EdidTxWriteBlockToRx(extensionIndex, aEdid, cecPhysAddrPosition))
	            {
	                pEdidTx->edidInfo.ErrorCode = EDID_ERROR__CANNOT_WRITE;
	                return;
	            }

	            isFirstExtension = false;

	        } // for (all extensions)
#if ( configSII_DEV_953x_PORTING == 1 ) /*Smith fixed EDID CTS fail for DVI_MinFormats.EDD*/
		}
		else
		{
			bool_t isFirstExtension = true;
			uint8_t cecPhysAddrPosition = 0;
			memset(aEdid, 0, sizeof(aEdid));
			if ( !EdidTxReadBlock( 1, aEdid))
			{
				pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__CANNOT_READ_EXT;
				DEBUG_PRINT( MSG_ERR, "\nCannot read EDID Block %d", (int)1);
			}
			if (!EdidTxWriteBlockToRx(extensionIndex, aEdid, cecPhysAddrPosition))
			{
				pEdidTx->edidInfo.ErrorCode = EDID_ERROR__CANNOT_WRITE;
				return;
			}
			isFirstExtension = false;
        }
#endif

	}
}


