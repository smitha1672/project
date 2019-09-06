//************************************************************************************************
//! @file   si_edid_tx_converter.c
//! @brief  Modify DS EDID parsed data for the Repeater mode
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

ROM const uint32_t RepeaterSADs[REPEATER_AUDIO__SADS_LENGTH] = REPEATER_AUDIO__SADS;

//-------------------------------------------------------------------------------------------------
//! @brief      CEC discovery algorithm
//!             longer
//!
//! @param[in]  pCecPhysAddr       - pointer to CEC physical address structure
//! @param[in]  inputPortIndex     - zero based input port number
//-------------------------------------------------------------------------------------------------

void EdidTxUsCecPhysAddrDiscovery(cecPhysAddr_t *pCecPhysAddr, uint8_t inputPortIndex)
{
    uint8_t a = pCecPhysAddr->a;
    uint8_t b = pCecPhysAddr->b;
    uint8_t c = pCecPhysAddr->c;
    uint8_t d = pCecPhysAddr->d;

    uint8_t *pAddrPos = NULL;

    if (0 == a)
    {
        // A valid HDMI sink must not have 'a' address equal to 0,
        // but in case of DVI sink it is possible.
        // Make "root" address in DVI case.
        pAddrPos = &a;
        b = c = d = 0;
    }
    else if (0 == b)
    {
        pAddrPos = &b;
        c = d = 0;
    }
    else if (0 == c)
    {
        pAddrPos = &c;
        d = 0;
    }
    else if (0 == d)
    {
        pAddrPos = &d;
    }
    else
    {
        // nesting limitation
        a = b = c = d = 0xF;
    }

    if (pAddrPos)
        *pAddrPos = inputPortIndex + 1;

    pCecPhysAddr->a = a;
    pCecPhysAddr->b = b;
    pCecPhysAddr->c = c;
    pCecPhysAddr->d = d;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Set short audio descriptors and speaker configuration
//-------------------------------------------------------------------------------------------------
void EdidTxOwnSadAndSpkSet( void )
{
    // replace SADs
    const uint32_t *p_ref_sads = RepeaterSADs;
    pEdidTx->edidInfo.SadCount = 0;
    while(*p_ref_sads)
    {
        uint32_t sad = *(p_ref_sads++);
        pEdidTx->edidInfo.SAD[pEdidTx->edidInfo.SadCount].ByteData.Byte1 = (uint8_t) sad;
        pEdidTx->edidInfo.SAD[pEdidTx->edidInfo.SadCount].ByteData.Byte2 = (uint8_t) (sad >> 8);
        pEdidTx->edidInfo.SAD[pEdidTx->edidInfo.SadCount].ByteData.Byte3 = (uint8_t) (sad >> 16);
        pEdidTx->edidInfo.SadCount++;
        if (pEdidTx->edidInfo.SadCount > MAX_NMB_OF_SADS) // just a protection
        {
            break;
        }
    }
    pEdidTx->edidInfo.SupportAudio = true;

    pEdidTx->edidInfo.SPK.Data = REPEATER_AUDIO__SPEAKERS;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Check if the input audio is to be played back by this device (repeater)
//-------------------------------------------------------------------------------------------------

static bool_t IsAudioToBePlayedLocally(void)
{
    bool_t play_locally = true; // means replace the audio block, i.e. "Play Locally"

#ifdef ALWAYS_PLAY_AUDIO_LOCALLY_WHEN_DS_DOES_NOT_SUPPORT_AUDIO
    if (SiiEdidTxDoesDsSupport(SI_EDID_TX_SUPPORT_AUDIO)) // But if DS device supports audio
#endif // ALWAYS_PLAY_AUDIO_LOCALLY_WHEN_DS_DOES_NOT_SUPPORT_AUDIO
    {
        if (pEdidTx->parseOptions.AudioBypass)  // and a user preferred to play it on the DS device,
        {
            play_locally = false;               // then do not replace the audio block with our own one
        }
    }
    return play_locally;
}


//-------------------------------------------------------------------------------------------------
//! @brief      Convert audio modes
//-------------------------------------------------------------------------------------------------

static void ConvertAudioModes ( void )
{
    if (IsAudioToBePlayedLocally())
    {
        EdidTxOwnSadAndSpkSet();

#if REPEATER_AUDIO__AI_SUPPORT
        pEdidTx->edidInfo.HDMI_VSDB.Extras[0] |= 0x80;
        //pEdidTx->edidInfo.HDMI_VSDB.SupportsAi = REPEATER_AUDIO__AI_SUPPORT;
#endif // REPEATER_AUDIO__AI_SUPPORT

#ifdef COLLECT_EXTENDED_DATA_BLOCKS
        pEdidTx->edidInfo.UnknownAudioExt_Idx = 0;
#endif // COLLECT_EXTENDED_DATA_BLOCKS
    }
    else
    {
        // use DS audio block
        if ( pEdidTx->edidInfo.SupportAudio )
        {
            // the DS audio block exists
            if ( pEdidTx->edidInfo.SadCount && !pEdidTx->edidInfo.SPK.Data )
            {
                // if SAD exists, but no SPK block
                pEdidTx->edidInfo.SPK.Fields.FL_FR = true; // use stereo speakers
            }
        //  else if ((!pEdidTx->edidInfo.SadCount) && pEdidTx->edidInfo.SPK.Data)
        //  {
        //      // if SPK block exists without SADs
        //      pEdidTx->edidInfo.SPK.Data = 0; // use only "Basic Audio"
        //  }
            {
                // Filter out unsupported audio modes.
                uint8_t nmb_of_sads = pEdidTx->edidInfo.SadCount;
                int i;
                bool_t at_least_one_sad_remain = false;
                for (i=0; i<nmb_of_sads; i++)
                {
                    uint8_t coding = pEdidTx->edidInfo.SAD[i].Common.Coding;
                    switch(coding)
                    {
                    case AUDIO_CODING__LPCM: // Linear PCM (e.g., IEC60958)
                        // Limit PCM audio with stereo up to 96 kHz.
                        if (pEdidTx->isSpdifOnly)
                        {
                            pEdidTx->edidInfo.SAD[i].Common.MaxChannel = 1;
                            pEdidTx->edidInfo.SAD[i].Common.Fs192k = 0;
                            pEdidTx->edidInfo.SAD[i].Common.Fs176k = 0;
                        }
                        at_least_one_sad_remain = true;
                        break;
                    case AUDIO_CODING__AC3: // AC-3
                    case AUDIO_CODING__DTS: // DTS
                        at_least_one_sad_remain = true;
                        break;
                    case AUDIO_CODING__1BIT:    // One Bit Audio
                    case AUDIO_CODING__DDP: // Dolby Digital +
                    case AUDIO_CODING__DTSHD:   // DTS-HD
                    case AUDIO_CODING__MLP: // MLP
                        if (pEdidTx->isSpdifOnly)
                        {
                            // SPDIF cannot carry the audio stream.
                            pEdidTx->edidInfo.SAD[i].Common.Coding = 0;
                        }
                        else
                        {
                            // I2S/DSD interface does support it.
                            at_least_one_sad_remain = true;
                        }
                        break;
                    default:
                        // Other audio formats are not supported.
                        pEdidTx->edidInfo.SAD[i].Common.Coding = 0;
                        break;
                    }
                }
                if ( !at_least_one_sad_remain )
                {
                    // All SAD were filtered out.
                    // Mark as for Basic Audio only:
                    // remove all SAD (clear the index). SPK block will be deleted also on the Composing stage.
                    pEdidTx->edidInfo.SadCount = 0; // notify EDID composer to not create SAD blocks.
            //      pEdidTx->edidInfo.SPK.Data = 0; // delete SPK block
                }
            }
        }
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Convert EDID configuration from Tx to Rx.
//!
//-------------------------------------------------------------------------------------------------

void EdidTxConvert ( void )
{
    uint8_t latency[4];

    // clear the error mask to start count composing (not decoding) errors
    pEdidTx->edidInfo.ErrorCode = EDID_ERROR__OK;

    // Convert video modes.

    // If user selected an output resolution,
    // check whether that resolution exists
    // and if it does, delete all other modes in pEdidTx->edidInfo.SDTD[]
    if ( !pEdidTx->parseOptions.BypassScaler )
    {
        int i;
        uint8_t outIdx = pEdidTx->parseOptions.OutIdx;
        if ( ((uint8_t) (-1)) != outIdx ) // -1 means "any supported output mode"
        {
            // if user selected an output video resolution

            // check if chosen mode is allowed by DS device
            if ( SiiEdidTxOutVideoModeRankGet( outIdx, SI_EDID_TX_ASPECT_RATIO_UNKNOWN ))
            {
                // if the given mode is presented in the DS EDID,
                // delete all the others
                for ( i=0; i < MAX_NMB_OF_KNOWN_DTDS; i++)
                {
                    if ( pEdidTx->edidInfo.SDTD[i].Valid )
                    {
                        // a valid DS SDTD
                        if ( outIdx != pEdidTx->edidInfo.SDTD[i].VideoIdx )
                        {
                            pEdidTx->edidInfo.SDTD[i].Valid = false;
                        }
                    } // if (DS SDTD valid)
                } // for (i)
            }
        }
    }

    // Find allowed input video modes.

    EdidTxScalerConvert();
    if ( !pEdidTx->parseOptions.BypassScaler )
    {
        if ( ((uint8_t) (-1)) != pEdidTx->parseOptions.OutIdx ) // -1 means "any supported output mode"
        {
            // If a user specified an output resolution (not Auto Select mode).
            if ( !pEdidTx->edidInfo.SDTD[0].Valid ) // after Edid_ScalerConvert() all SDTDs are placed at the beginning
            {
                // If no input modes were discovered,
                // try to repeat the procedure for any output format.
                EdidTxDsSdtdsGet( pEdidTx->edidInfo.SDTD ); // Recover all DS video modes.
                EdidTxScalerConvert(); // The second attempt.
            }
        }
    }

    // Convert Audio modes
    ConvertAudioModes();
    if ( !pEdidTx->pEdidDecodData->edidAnalysis.HdmiMode )       // No audio for DVI
    {
        pEdidTx->edidInfo.HDMI_VSDB.HdmiMode = false;
        DEBUG_PRINT( EDID_TX_MSG_DBG, "EDID_CONVERT: DVI Mode selected\n" );
    }
    else
    {
        // RX with HDMI support

        pEdidTx->edidInfo.HDMI_VSDB.HdmiMode = true;

        if (pEdidTx->isDeepColorOvrd)
        {
            pEdidTx->edidInfo.HDMI_VSDB.Extras[0] &= 0x80; // clear all fields except Support_AI
            if ( pEdidTx->isDeepColor3036 )
            {
                pEdidTx->edidInfo.HDMI_VSDB.Extras[0] |= 0x38; // DC_Y444 | DC_36bit | DC_30bit
                pEdidTx->edidInfo.HDMI_VSDB.Extras[1] = pEdidTx->edidConfig.maxTmdsClock5Mhz; // MAX TMDS Clock

                // Note. Sometimes it may be reasonable to copy DS Max TMDS clock value to US.
                // Modify this part if it is required.
            }
        }
        else
        {
            // Leave DC settings as they are from DS
            // and limit max TMDS clock frequency according to repeater capabilities
            if (pEdidTx->edidInfo.HDMI_VSDB.Extras[1] > pEdidTx->edidConfig.maxTmdsClock5Mhz)
            {
                pEdidTx->edidInfo.HDMI_VSDB.Extras[1] = pEdidTx->edidConfig.maxTmdsClock5Mhz; // MAX TMDS Clock
            }
        }

        // HDMI 1.3a's latency fields support

        if (IsAudioToBePlayedLocally())
        {
            latency[0] = EDID_LATENCY_VIDEO_LOCAL;  // Video latency
            latency[1] = EDID_LATENCY_AUDIO_LOCAL;  // Audio latency
            latency[2] = EDID_LATENCY_I_VIDEO_LOCAL;// Interlaced Video latency
            latency[3] = EDID_LATENCY_I_AUDIO_LOCAL;// Interlaced Audio latency
        }
        else
        {
            latency[0] = 0;// Video latency
            latency[1] = 0;// Audio latency
            latency[2] = 0;// Interlaced Video latency
            latency[3] = 0;// Interlaced Audio latency
        }

        if ( (0xFF == latency[0]) && (0xFF == latency[1]) && (0xFF == latency[2]) && (0xFF == latency[3]) )
        {
            // no latency to be shown to an US device
            pEdidTx->edidInfo.HDMI_VSDB.Extras[2] = 0;
        }
        else
        {
            // Calculate latencies to be shown to an US device.

            if ( pEdidTx->edidInfo.HDMI_VSDB.Extras[2] & 0x80)
            {
                // if DS latency field are found

                latency[0] += pEdidTx->edidInfo.HDMI_VSDB.Extras[3];
                latency[1] += pEdidTx->edidInfo.HDMI_VSDB.Extras[4];

                if ( pEdidTx->edidInfo.HDMI_VSDB.Extras[2] & 0x40)
                {
                    // if DS interlaced latency fields are found
                    latency[2] += pEdidTx->edidInfo.HDMI_VSDB.Extras[5];
                    latency[3] += pEdidTx->edidInfo.HDMI_VSDB.Extras[6];
                }
                else
                {
                    // if DS interlaced latency fields are NOT found
                    latency[2] += pEdidTx->edidInfo.HDMI_VSDB.Extras[3];
                    latency[3] += pEdidTx->edidInfo.HDMI_VSDB.Extras[4];
                }
            }

            // clear all latency fields
            memset( &pEdidTx->edidInfo.HDMI_VSDB.Extras[2], 0, EDID_VSB_LEN__MAX_EXTRAS - 2);

            if (latency[0] || latency[1] || latency[2] || latency[3])
            {
                // if any latency field differs from 0
                pEdidTx->edidInfo.HDMI_VSDB.Extras[3] = latency[0];
                pEdidTx->edidInfo.HDMI_VSDB.Extras[4] = latency[1];
                pEdidTx->edidInfo.HDMI_VSDB.Extras[2] |= 0x80;

                if ((latency[0] != latency[2]) ||(latency[1] != latency[3]))
                {
                    // if interlaced formats have different latency
                    pEdidTx->edidInfo.HDMI_VSDB.Extras[5] = latency[2];
                    pEdidTx->edidInfo.HDMI_VSDB.Extras[6] = latency[3];
                    pEdidTx->edidInfo.HDMI_VSDB.Extras[2] |= 0x40;
                }
            }

        }

        // xvYCC support
        if ( !pEdidTx->isXvYccProfile0 )
        {
            // Disable xvYCC if xvYCC is blocked by the component xvYCC option
            // or if any color space conversion is allowed.
            pEdidTx->edidInfo.xv601 = false;
            pEdidTx->edidInfo.xv709 = false;
        }

    }
}
