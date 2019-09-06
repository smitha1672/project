//************************************************************************************************
//! @file   si_edid_tx_scaler.c
//! @brief  Part of the EDID conversion considering A/V stream modifications (scaling) in repeater
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
#include "si_scaler_tables.h"

#ifndef SII_USE_SCALER_TABLE_IN_NVM // NOT
extern ROM const videoScalerMapping_t DefaultAllowedScalerModes[];
const videoScalerMapping_t *pAllowedScalerModes = DefaultAllowedScalerModes;
#endif // NOT SII_USE_SCALER_TABLE_IN_NVM


//-------------------------------------------------------------------------------------------------
//! @brief      Convert EDID taking into account the video scaler conversion.
//!
//-------------------------------------------------------------------------------------------------

void EdidTxScalerConvert( void )
{
    int i, j, k;
    bool_t add_vga = false;
    shortDtd_t us_sdtd[MAX_NMB_OF_KNOWN_DTDS];
    int nmb_of_sdtds = 0;

#ifdef SII_USE_SCALER_TABLE_IN_NVM
    videoScalerMapping_t   conversion_element;
    if (!SI_HalNvmGetElement(NV__SCALER_CONVERSION, &conversion_element, 0, sizeof(conversion_element)))
#else // SII_USE_SCALER_TABLE_IN_NVM
    if (!pAllowedScalerModes) // protection
#endif // SII_USE_SCALER_TABLE_IN_NVM
    {
        pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__INTERNAL_ERROR;
        //return;
        goto label_exit;
    }

    if ( pEdidTx->parseOptions.BypassScaler)
    {
        if ( pEdidTx->parseOptions.AllowPcModesOnScalerBypass)
        {
            goto label_exit;
        }
        else
        {
            if ( pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming1 & 0x20) // VGA 640 x 480 @ 60Hz
            {
                add_vga = true;
            }
            goto label_disable_pc_modes;
        }
    }

    // Clear new short DTD structure
    memset( us_sdtd, 0, sizeof(us_sdtd));

    // move preferred and native modes to the beginning of pEdidTx->pEdidDecodData->edidInfo.SDTD[i]
    // (it is needed to be sure that native/preferred modes are not cut off
    // due to the limitation of US SDTD array)
    // put Native and Preferred SDTDs to the beginning of the list

    for ( i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if ( pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
        {
            if ( pEdidTx->pEdidDecodData->edidInfo.SDTD[i].PreferredTiming || pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Native)
            {
                memcpy( &us_sdtd[nmb_of_sdtds++], &pEdidTx->pEdidDecodData->edidInfo.SDTD[i], sizeof(shortDtd_t));
            }
        }
    }
    // then place the rest
    for ( i=0; i<MAX_NMB_OF_KNOWN_DTDS; i++)
    {
        if ( pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid )
        {
            if ( !(pEdidTx->pEdidDecodData->edidInfo.SDTD[i].PreferredTiming || pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Native))
            {
                memcpy(&us_sdtd[nmb_of_sdtds++], &pEdidTx->pEdidDecodData->edidInfo.SDTD[i], sizeof(shortDtd_t));
            }
        }
    }
    if ( nmb_of_sdtds > MAX_NMB_OF_KNOWN_DTDS) // protection- this must not happened
    {
        nmb_of_sdtds = MAX_NMB_OF_KNOWN_DTDS;
        pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__INTERNAL_ERROR;
    }
    // Finally copy to the DS array
    memcpy( pEdidTx->pEdidDecodData->edidInfo.SDTD, us_sdtd, sizeof( pEdidTx->pEdidDecodData->edidInfo.SDTD));

    // Clear new short DTD structure
    memset(us_sdtd, 0, sizeof(us_sdtd));

    // compose a list of allowed input modes based on given output modes and
    // a mode conversion table

    for ( i=0; i < nmb_of_sdtds; i++)
    {
        if ( pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
        {
            // a valid DS SDTD
            uint8_t out_idx = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].VideoIdx;
            bool_t out_ar16x9 = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Ar16x9;

#ifdef SII_USE_SCALER_TABLE_IN_NVM
            for (j=0; SI_HalNvmGetElement(NV__SCALER_CONVERSION, &conversion_element, j, sizeof(conversion_element)) && (0xFF != conversion_element.OutFmtId); j++)
            {
                if ((out_idx == conversion_element.OutFmtId) &&
                    (out_ar16x9 == conversion_element.OutAspectRatio16x9))
                {
                    // output mode found
                    uint8_t in_idx = conversion_element.InFmtId;
                    bool_t in_ar16x9 = conversion_element.InAspectRatio16x9;
#  if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
                    bool_t capable_3d = conversion_element.CapableOf3D;
#  endif // SI_HDMI_1_4_EDID_PROCESSING
#else // SII_USE_SCALER_TABLE_IN_NVM
            for (j=0; 0xFF != pAllowedScalerModes[j].OutFmtId; j++)
            {
                if ((out_idx == pAllowedScalerModes[j].OutFmtId) &&
                    (out_ar16x9 == pAllowedScalerModes[j].OutAspectRatio16x9))
                {
                    // output mode found
                    uint8_t in_idx = pAllowedScalerModes[j].InFmtId;
                    bool_t in_ar16x9 = pAllowedScalerModes[j].InAspectRatio16x9;
#  if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
                    bool_t capable_3d = pAllowedScalerModes[j].CapableOf3D;
#  endif // SI_HDMI_1_4_EDID_PROCESSING
#endif // SII_USE_SCALER_TABLE_IN_NVM
                    uint8_t in_vic = in_ar16x9 ? VideoModeTable[in_idx].Vic16x9 : VideoModeTable[in_idx].Vic4x3;
                    if (0 == in_vic)
                    {
                        // pAllowedScalerModes[] has an incorrect record (i.e. internal error)
                        in_ar16x9 = !in_ar16x9;
                        in_vic = in_ar16x9 ? VideoModeTable[in_idx].Vic16x9 : VideoModeTable[in_idx].Vic4x3;
                        pEdidTx->pEdidDecodData->edidInfo.ErrorCode = EDID_ERROR__INTERNAL_ERROR;
                    }

                    if (vm1_640x480p == in_idx)
                    {
                        // VGA 640x480
                        add_vga = true;
                        //continue;
                    }

                    { // adding
                        bool_t already_added = false;
                        // Check whether a SDTD with the same VIC exists.
                        // If it does, do not allow it to be added again.
                        for (k=0; k<MAX_NMB_OF_KNOWN_DTDS; k++) // find an empty slot in us_sdtd[]
                        {
                            if (us_sdtd[k].Valid)
                            {
                                if (in_vic == us_sdtd[k].Vic)
                                {
                                    already_added = true;
                                    break;
                                }
                            }
                        } // for (k) 1st

                        if ( !already_added)
                        {
                            // find an empty slot in us_sdtd[] to add a new element
                            for (k=0; k<MAX_NMB_OF_KNOWN_DTDS; k++)
                            {
                                if (!us_sdtd[k].Valid)
                                {
                                    // found an empty slot in the us_sdtd table
                                    us_sdtd[k].Valid = true; // occupate the slot
                                    us_sdtd[k].VideoIdx = in_idx;
                                    us_sdtd[k].Vic = in_vic;
                                    us_sdtd[k].Ar16x9 = in_ar16x9;
                                    us_sdtd[k].DisplayType = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].DisplayType;
#if (SI_HDMI_1_4_EDID_PROCESSING == ENABLE)
                                    if (capable_3d)
                                    {
                                        us_sdtd[k].masked_3d = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].masked_3d;
                                        us_sdtd[k].struct_3d_present = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].struct_3d_present;
                                        us_sdtd[k].struct_3d = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].struct_3d;
                                        us_sdtd[k].detail_3d = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].detail_3d;
 memcpy(us_sdtd[k].indiv3dModes,
                                        		pEdidTx->edidInfo.SDTD[i].indiv3dModes,
                                        		sizeof(Indiv3dMode_t) * MAX_INDIVIDUAL_3D_MODES_PER_VIC);
                                        us_sdtd[k].indiv3dModeLength = pEdidTx->edidInfo.SDTD[i].indiv3dModeLength;										
                                    }
#endif // SI_HDMI_1_4_EDID_PROCESSING
                                    break;
                                }  // if (US SDTD valid)
                            } // for (k) 2nd
                            if (MAX_NMB_OF_KNOWN_DTDS == k)
                            {
                                // no more empty slots
                                pEdidTx->pEdidDecodData->edidInfo.ErrorCode |= EDID_ERROR__NOT_ENOUGH_ROOM;
                                goto label_leave_all_cycles;
                            }
                        } // if (!already_added)
                    } // adding
                }
            } // for (j)
        } // if (DS SDTD valid)
    } // for (i)
label_leave_all_cycles:

    // Copy Preferred and Native flags to those input formats
    // that match output formats with the flags
    for ( i=0; i<nmb_of_sdtds; i++)
    {
        if ( pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Valid)
        {
            // a valid DS SDTD
            uint8_t out_idx = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].VideoIdx;
            uint8_t out_vic = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Vic;
            for ( k=0; k<MAX_NMB_OF_KNOWN_DTDS; k++) // look through us_sdtd[]
            {
                if ( (us_sdtd[k].Valid) &&
                    (out_idx == us_sdtd[k].VideoIdx) &&
                    (out_vic == us_sdtd[k].Vic) )
                {
                    // found the same input video mode as the given output one
                    us_sdtd[k].Native = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].Native;
                    us_sdtd[k].PreferredTiming = pEdidTx->pEdidDecodData->edidInfo.SDTD[i].PreferredTiming;
                    break;
                }
            } // for (k)
        } // if (DS SDTD valid)
    } // for (i)



    // Copy allowed scaler input modes to the input/result buffer

    memcpy( pEdidTx->pEdidDecodData->edidInfo.SDTD, us_sdtd, sizeof( pEdidTx->pEdidDecodData->edidInfo.SDTD));

    pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinV_Hz   = pEdidTx->edidConfig.minVertHz;
    pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxV_Hz   = pEdidTx->edidConfig.maxVertHz;
    pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MinH_kHz  = pEdidTx->edidConfig.minHorizKhz;
    pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxH_kHz  = pEdidTx->edidConfig.maxHorizKhz;
    pEdidTx->pEdidDecodData->edidInfo.RangeLimits.MaxPixClk_10MHz = pEdidTx->edidConfig.maxPixClock10Mhz;
    pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.RangeLimits = true;

    // Suppose our scaler does not support PC modes except VGA.
    // If it is not true, modify the code below.
label_disable_pc_modes:
    // Disable PC video modes
//  if (pEdidTx->parseOptions.VgaSupportedByScaler) // a user option
//      pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming1 = 0x20; // 640 x 480 @ 60Hz
//  else
    pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming1 = 0;
    if (add_vga)
    {
        pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming1 |= 0x20; // 640 x 480 @ 60Hz
    }
    pEdidTx->pEdidDecodData->edidInfo.EstablishedTiming2 = 0x00;
    pEdidTx->pEdidDecodData->edidInfo.ManufacturerReservedTiming = 0x00;
    memset( pEdidTx->pEdidDecodData->edidInfo.StandardTiming, 0x01, sizeof( pEdidTx->pEdidDecodData->edidInfo.StandardTiming));
#ifdef COLLECT_EXCESSIVE_INFO
    pEdidTx->pEdidDecodData->edidInfo.DescriptorValid.StandardTiming = false;
#endif // COLLECT_EXCESSIVE_INFO
#ifdef COLLECT_EXTENDED_DATA_BLOCKS
    pEdidTx->pEdidDecodData->edidInfo.UnknownVideoExt_Idx = 0;
#endif // COLLECT_EXTENDED_DATA_BLOCKS
    memset( pEdidTx->pEdidDecodData->edidInfo.DTD, 0, sizeof( pEdidTx->pEdidDecodData->edidInfo.DTD));

label_exit:
;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Check if scaler can do given conversion (from one video mode to another).
//!
//!             More detailed description.
//!
//! @param[in]  inIndex        - input video mode ID,
//! @param[in]  inAspectRatio  - input aspect ratio,
//! @param[in]  outIndex       - output video mode ID,
//! @param[in]  outAspectRatio - output aspect ratio.
//!
//! @retval     true  - conversion supported,
//! @retval     false - conversion not supported.
//-------------------------------------------------------------------------------------------------

static bool_t DoesScalerSupport(uint8_t inIndex, aspectRatio_t inAspectRatio, uint8_t outIndex, aspectRatio_t outAspectRatio)
{
    int i;
    bool_t ret = false;
    bool_t in_ar16x9 = (inAspectRatio == SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9);
    bool_t out_ar16x9 = (outAspectRatio == SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9);
    bool_t in_ar_unknown = (SI_EDID_TX_ASPECT_RATIO_UNKNOWN == inAspectRatio);
    bool_t out_ar_unknown = (SI_EDID_TX_ASPECT_RATIO_UNKNOWN == outAspectRatio);

#ifdef SII_USE_SCALER_TABLE_IN_NVM
    videoScalerMapping_t   conversion_element;
    for (i=0; SI_HalNvmGetElement(NV__SCALER_CONVERSION, &conversion_element, i, sizeof(conversion_element)) && (0xFF != conversion_element.OutFmtId); i++)
    {
        if ( (inIndex == conversion_element.InFmtId) &&
            (outIndex == conversion_element.OutFmtId) &&
            (in_ar_unknown || (in_ar16x9 == conversion_element.InAspectRatio16x9)) &&
            (out_ar_unknown || (out_ar16x9 == conversion_element.OutAspectRatio16x9)) )
        {
            ret = true;
            break;
        }
    }
#else // SII_USE_SCALER_TABLE_IN_NVM
    for (i=0; 0xFF != pAllowedScalerModes[i].OutFmtId; i++)
    {
        if ( (inIndex == pAllowedScalerModes[i].InFmtId) &&
            (outIndex == pAllowedScalerModes[i].OutFmtId) &&
            (in_ar_unknown || (in_ar16x9 == pAllowedScalerModes[i].InAspectRatio16x9)) &&
            (out_ar_unknown || (out_ar16x9 == pAllowedScalerModes[i].OutAspectRatio16x9)) )
        {
            ret = true;
            break;
        }
    }
#endif // SII_USE_SCALER_TABLE_IN_NVM
    return ret;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return the downstream best fit output video mode for the given input mode.
//!
//!             This function is intended to be called when user selected automatic selection
//!             of output resolution, input video mode is determined on RX side and
//!             it is time to make decision on it to configure repeater's scaler.
//!
//! @param[in]  inIdx        - input video format ID,
//! @param[in]  inAspRatio   - input video aspect ratio,
//! @param[out] pOutIdx      - pointer to output video format ID,
//! @param[out] pOutAspRatio - pointer to output video aspect ratio,
//!
//! @return     true - if a scaler supported mode is found, false - otherwise.
//-------------------------------------------------------------------------------------------------

bool_t SiiEdidTxBestDsVideoModeGet(uint8_t inIdx, aspectRatio_t inAspRatio, uint8_t *pOutIdx, aspectRatio_t *pOutAspRatio)
{
    bool_t scaler_supported = true; // true if a scaler supported mode is found
    uint8_t best_fit_rate = 0; // tells how good the output mode fits to the DS device
    uint8_t best_out_idx = vm1_640x480p; // VGA
    aspectRatio_t best_out_ar = SI_EDID_TX_ASPECT_RATIO_4x3; // default aspect ratio for VGA

    {
        int i;
        bool_t in_ar16x9 = (SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9 == inAspRatio);
        bool_t in_ar_unknown = (SI_EDID_TX_ASPECT_RATIO_UNKNOWN == inAspRatio);

#ifdef SII_USE_SCALER_TABLE_IN_NVM
        videoScalerMapping_t   conversion_element;
        for (i=0; SI_HalNvmGetElement(NV__SCALER_CONVERSION, &conversion_element, i, sizeof(conversion_element)) && (0xFF != conversion_element.OutFmtId); i++)
        {
            if ((inIdx == conversion_element.InFmtId) &&
                (in_ar_unknown || (in_ar16x9 == conversion_element.InAspectRatio16x9)))
            {
                uint8_t out_idx = conversion_element.OutFmtId;
                uint8_t out_ar16x9 = conversion_element.OutAspectRatio16x9;
                uint8_t fit_rate = SiiEdidTxOutVideoModeRankGet(out_idx, inAspRatio);
                if (fit_rate > best_fit_rate)
                {
                    best_fit_rate = fit_rate;
                    best_out_idx = out_idx;
                    best_out_ar = out_ar16x9 ? SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9 : SI_EDID_TX_ASPECT_RATIO_4x3;
                }
            }
        }
#else // SII_USE_SCALER_TABLE_IN_NVM
        for (i=0; 0xFF != pAllowedScalerModes[i].OutFmtId; i++)
        {
            if ((inIdx == pAllowedScalerModes[i].InFmtId) &&
                (in_ar_unknown || (in_ar16x9 == pAllowedScalerModes[i].InAspectRatio16x9)))
            {
                uint8_t out_idx = pAllowedScalerModes[i].OutFmtId;
                uint8_t out_ar16x9 = pAllowedScalerModes[i].OutAspectRatio16x9;
                uint8_t fit_rate = SiiEdidTxOutVideoModeRankGet(out_idx, inAspRatio);
                if (fit_rate > best_fit_rate)
                {
                    best_fit_rate = fit_rate;
                    best_out_idx = out_idx;
                    best_out_ar = out_ar16x9 ? SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9 : SI_EDID_TX_ASPECT_RATIO_4x3;
                }
            }
        }
#endif // SII_USE_SCALER_TABLE_IN_NVM
    }

    if (0 == best_fit_rate)
    {
        // no supported modes were found
#ifdef DEFAULT_SYSTEM__PAL
#  define FIRST_SYSTEM SI_EDID_TX_SUPPORT_PAL
#  define FIRST_MODE vm17_18_720x576p
#  define SECOND_SYSTEM SI_EDID_TX_SUPPORT_NTSC
#  define SECOND_MODE vm2_3_720x480p
#else // DEFAULT_SYSTEM__PAL
#  define FIRST_SYSTEM SI_EDID_TX_SUPPORT_NTSC
#  define FIRST_MODE vm2_3_720x480p
#  define SECOND_SYSTEM SI_EDID_TX_SUPPORT_PAL
#  define SECOND_MODE vm17_18_720x576p
#endif // DEFAULT_SYSTEM__PAL

        bool_t only16x9 = ((!SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AR_4x3)) &&
                SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AR_16x9));

        // if the scaler supports that transformation !!!

        if (SiiEdidTxDoesDsSupport(FIRST_SYSTEM))
        {
            if (only16x9)
            {
                if ( DoesScalerSupport(inIdx, inAspRatio, FIRST_MODE, SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9) )
                {
                    best_out_idx = FIRST_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9;
                }
                else if ( DoesScalerSupport(inIdx, inAspRatio, FIRST_MODE, SI_EDID_TX_ASPECT_RATIO_4x3) )
                {
                    best_out_idx = FIRST_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_4x3;
                }
            }
            else
            {
                if ( DoesScalerSupport(inIdx, inAspRatio, FIRST_MODE, SI_EDID_TX_ASPECT_RATIO_4x3) )
                {
                    best_out_idx = FIRST_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_4x3;
                }
                else if ( DoesScalerSupport(inIdx, inAspRatio, FIRST_MODE, SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9) )
                {
                    best_out_idx = FIRST_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9;
                }
            }
        }
        else if (SiiEdidTxDoesDsSupport(SECOND_SYSTEM))
        {
            if (only16x9)
            {
                if ( DoesScalerSupport(inIdx, inAspRatio, SECOND_MODE, SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9) )
                {
                    best_out_idx = SECOND_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9;
                }
                else if ( DoesScalerSupport(inIdx, inAspRatio, SECOND_MODE, SI_EDID_TX_ASPECT_RATIO_4x3) )
                {
                    best_out_idx = SECOND_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_4x3;
                }
            }
            else
            {
                if ( DoesScalerSupport(inIdx, inAspRatio, SECOND_MODE, SI_EDID_TX_ASPECT_RATIO_4x3) )
                {
                    best_out_idx = SECOND_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_4x3;
                }
                else if ( DoesScalerSupport(inIdx, inAspRatio, FIRST_MODE, SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9) )
                {
                    best_out_idx = SECOND_MODE;
                    best_out_ar = SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9;
                }
            }
        }
        else if ( DoesScalerSupport(inIdx, inAspRatio, vm1_640x480p, SI_EDID_TX_ASPECT_RATIO_4x3) )
        {
            best_out_idx = vm1_640x480p;
            best_out_ar = SI_EDID_TX_ASPECT_RATIO_4x3;
        }
        else if ( DoesScalerSupport(inIdx, inAspRatio, inIdx, inAspRatio) )
        {
            best_out_idx = inIdx;
            best_out_ar = inAspRatio;
        }
        else
        {
            // switch off scaler
            scaler_supported = false;
            best_out_idx = inIdx;
            best_out_ar = inAspRatio;
        }
    }
    *pOutIdx = best_out_idx;
    *pOutAspRatio = (uint8_t) best_out_ar;

    return scaler_supported;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Return aspect ratio of scaled video.
//!
//!             This function is intended to be called when user selected one
//!             of the output resolutions, input video mode is determined on RX and
//!             it is time to determine whether scaler can do that conversion and if
//!             it can- what is the output aspect ratio.
//!
//! @param[in]  in_idx  - input video mode ID,
//! @param[in]  out_idx - output video mode ID,
//! @param[in]  in_ar   - aspect ratio of input video,
//!
//! @return     the aspect ratio if the conversion is possible.
//! @retval     SI_EDID_TX_ASPECT_RATIO_UNKNOWN - if out_idx is not available.
//-------------------------------------------------------------------------------------------------

aspectRatio_t SiiEdidTxScalerOutAspectRatioGet(uint8_t inIdx, aspectRatio_t inAspRatio, uint8_t outIdx)
{
    aspectRatio_t out_ar = SI_EDID_TX_ASPECT_RATIO_UNKNOWN;
    bool_t ar4x3 = false;
    bool_t ar16x9 = false;
    if ( DoesScalerSupport(inIdx, inAspRatio, outIdx, SI_EDID_TX_ASPECT_RATIO_4x3) )
        ar4x3 = true;
    if ( DoesScalerSupport(inIdx, inAspRatio, outIdx, SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9) )
        ar16x9 = true;
    if (ar4x3 && (SI_EDID_TX_ASPECT_RATIO_4x3 == inAspRatio))
        out_ar = SI_EDID_TX_ASPECT_RATIO_4x3;
    else if (ar16x9 && (SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9 == inAspRatio))
        out_ar = SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9;
    else if (ar4x3 && (!ar16x9))
        out_ar = SI_EDID_TX_ASPECT_RATIO_4x3;
    else if ((!ar4x3) && ar16x9)
        out_ar = SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9;
    else if (ar4x3 && ar16x9)
    {
        bool_t only16x9 = ((!SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AR_4x3)) &&
                SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AR_16x9));
        out_ar = only16x9 ? SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9 : SI_EDID_TX_ASPECT_RATIO_4x3;
    }


    return out_ar;
}


//-------------------------------------------------------------------------------------------------
//! @brief      This function determines if the scaler supports the given video mode.
//!
//!             The function could be used in conjunction with SiiEdidTxOutVideoModeRankGet()
//!             to determine whether or not to dim given resolution in a repeater output mode menu.
//!
//! @param[in]  idx - video mode ID,
//! @param[in]  ar  - aspect ratio.
//!
//! @return     If it returns true the mode is supported, false otherwise.
//-------------------------------------------------------------------------------------------------

bool_t SiiEdidTxScalerIsOutVideoModeSupported(uint8_t idx, aspectRatio_t aspRatio)
{
    bool_t support = false;
    int i;
    bool_t ar16x9 = (SI_EDID_TX_ASPECT_RATIO_UNKNOWN_16x9 == aspRatio);
    bool_t ar_unknown = (SI_EDID_TX_ASPECT_RATIO_UNKNOWN == aspRatio);

#ifdef SII_USE_SCALER_TABLE_IN_NVM
    videoScalerMapping_t   conversion_element;
    for (i=0; SI_HalNvmGetElement(NV__SCALER_CONVERSION, &conversion_element, i, sizeof(conversion_element)) && (0xFF != conversion_element.OutFmtId); i++)
    {
        if ( (ar_unknown || (ar16x9 == conversion_element.OutAspectRatio16x9)) &&
            (idx == conversion_element.OutFmtId) )
        {
            support = true;
            break;
        }
    }
#else // SII_USE_SCALER_TABLE_IN_NVM
    for (i=0; 0xFF != pAllowedScalerModes[i].OutFmtId; i++)
    {
        if ( (ar_unknown || (ar16x9 == pAllowedScalerModes[i].OutAspectRatio16x9)) &&
            (idx == pAllowedScalerModes[i].OutFmtId) )
        {
            support = true;
            break;
        }
    }
#endif // SII_USE_SCALER_TABLE_IN_NVM
    return support;
}
