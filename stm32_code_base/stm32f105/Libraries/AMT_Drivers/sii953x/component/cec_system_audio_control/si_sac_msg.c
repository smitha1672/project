//*************************************************************************************************
//! @file     si_sac_msg.c
//! @brief    Message level of CEC/System Audio Control
//
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//************************************************************************************************/

#include "si_common.h"
#include "si_cec_internal.h"
#include "si_sac_msg.h"

#ifndef CEC_PC_SIMULATION
#include <string.h>    
#else
#include "stdio.h"
#include "memory.h"
#include "debug_print.h"
#endif

//-------------------------------------------------------------------------------------------------
// Global Data
//-------------------------------------------------------------------------------------------------

extern CecSacInstanceRecord_t   *pSac; //!< global pointer to CEC/SAC Instance Record


//-------------------------------------------------------------------------------------------------
// Service Functions
//-------------------------------------------------------------------------------------------------

                                            
//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC System Audio Mode Request" message to initiate
//!             the SAC feature from TV
//!
//! @param[in]  srcPhysAddr - physical address of audio content source device.
//!                           If the value is 0xFFFF, command is sent without parameter
//! @param[in]  destLogAddr - logical address of the destination device (AMP)
//-------------------------------------------------------------------------------------------------

void SiiCecSacSystemAudioModeRequestSend (const uint16_t srcPhysAddr, const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    uint8_t argCount;

    if (srcPhysAddr != 0xFFFF)
    {
        cecFrame.args[0] = (srcPhysAddr >> 8) & 0xFF;   // Audio Source Physical Address (MSB)
        cecFrame.args[1] = srcPhysAddr & 0xFF;          // Audio Source Physical Address (LSB)
        argCount = 2;
    }
    else
    {
        argCount = 0;
    }

    SiiCecCpiWrite(CECOP_SYSTEM_AUDIO_MODE_REQUEST, argCount, destLogAddr, &cecFrame);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Give Audio Status" message to request an Amplifier to
//!             send its volume and mute status
//!
//! @param[in]  destLogAddr - logical address of the destination device (AMP)
//-------------------------------------------------------------------------------------------------

void SiiCecSacGiveAudioStatusSend (const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    SiiCecCpiWrite(CECOP_GIVE_AUDIO_STATUS, 0, destLogAddr, &cecFrame);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Set System Audio Mode" message to set
//!             the SAC mode on or off
//!
//! @param[in]  enabled     - SAC mode status,
//! @param[in]  destLogAddr - logical address of the destination device (TV,STB)
//-------------------------------------------------------------------------------------------------

void SiiCecSacSetSystemAudioModeSend (const bool_t enabled, const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    cecFrame.args[0] = enabled & 0x01;   // SAC mode status
    if(SiiCecCpiWrite(CECOP_SET_SYSTEM_AUDIO_MODE, 1, destLogAddr, &cecFrame)==0)
    {
        SiiDrvCpiRemove1stInQ();
        SiiCecCpiWrite(CECOP_SET_SYSTEM_AUDIO_MODE, 1, destLogAddr, &cecFrame);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Report Audio Status" message to report
//!             Amplifier's volume and mute status
//!
//! @param[in]  volume      - master volume value [0-100],
//! @param[in]  muted       - mute status (on/off),
//! @param[in]  destLogAddr - logical address of the destination device (TV,STB)
//-------------------------------------------------------------------------------------------------

void SiiCecSacReportAudioStatusSend (const uint8_t volume, const bool_t muted, const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    cecFrame.args[0] = (muted ? 0x80 : 0) | (volume & 0x7F);   // Volume & mute status
    SiiDrvCpiReportAudioStatusClear(MAKE_SRCDEST(SiiCecGetDeviceLA(), destLogAddr));
    SiiCecCpiWrite(CECOP_REPORT_AUDIO_STATUS, 1, destLogAddr, &cecFrame);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Give System Audio Mode Status" message to request an
//!             Amplifier to send its system audio mode status (on/off)
//!
//! @param[in]  destLogAddr - logical address of the destination device (AMP)
//-------------------------------------------------------------------------------------------------

void SiiCecSacGiveSystemAudioModeStatusSend (const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    SiiCecCpiWrite(CECOP_GIVE_SYSTEM_AUDIO_MODE_STATUS, 0, destLogAddr, &cecFrame);
}


#if ( configSII_DEV_953x_PORTING == 1 )
//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Set System Audio Mode" message to report
//!             the SAC mode (on/off)
//!
//! @param[in]  destLogAddr - logical address of the destination device (TV,STB), OnOff - send SAC on or off
//-------------------------------------------------------------------------------------------------

void SiiCecSacSystemAudioModeStatusSend (const uint8_t destLogAddr, bool OnOff)
{
    SiiCpiData_t cecFrame;
    cecFrame.args[0] = (uint8_t)OnOff;
    if(SiiCecCpiWrite(CECOP_SYSTEM_AUDIO_MODE_STATUS, 1, destLogAddr, &cecFrame)==0)
    {
        SiiDrvCpiRemove1stInQ();
        SiiCecCpiWrite(CECOP_SYSTEM_AUDIO_MODE_STATUS, 1, destLogAddr, &cecFrame);
    }
}

#else
//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Set System Audio Mode" message to report
//!             the SAC mode (on/off)
//!
//! @param[in]  destLogAddr - logical address of the destination device (TV,STB)
//-------------------------------------------------------------------------------------------------

void SiiCecSacSystemAudioModeStatusSend (const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    cecFrame.args[0] = pSac->status.isSystemAudioModeEnabled & 0x01;   // SAC mode status
    SiiCecCpiWrite(CECOP_SYSTEM_AUDIO_MODE_STATUS, 1, destLogAddr, &cecFrame);
}

#endif
//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Report Short Audio Descriptor" message to report
//!             audio capabilities (supported formats) of the Amplifier
//!
//! @param[in]  destLogAddr   - logical address of the destination device (TV,STB)
//! @param[in]  nFmt          - number of requested audio formats [1-4],
//! @param[in]  aAudioFormats - list of requested audio format codes
//-------------------------------------------------------------------------------------------------

void SiiCecSacReportAudioCapabilitySend (const uint8_t destLogAddr, uint8_t nFmt, CecSacAudioFmtCode_t aAudioFormats[4])
{
    SiiCpiData_t cecFrame;
    uint8_t i;
    uint8_t j;
    uint8_t argCount = 0;

    for (j = 0; j < nFmt; j++)
    {
        for (i = 0; i < pSac->numberOfAudioFormats; i++)
        {
            if (aAudioFormats[j] == pSac->aFmt[i].audioFmtCode)
            {
                cecFrame.args[argCount++] = (((pSac->aFmt[i].audioFmtCode) & 0x0F) << 3) | (pSac->aFmt[i].maxChanNumber & 0x07);
                cecFrame.args[argCount++] = pSac->aFmt[i].sampleRate;
                cecFrame.args[argCount++] = pSac->aFmt[i].quality;
            }
        }
    }

    if (argCount > 0)
    {
        SiiCecCpiWrite(CECOP_REPORT_SHORT_AUDIO, argCount, destLogAddr, &cecFrame);
    }
    else
    {
        DEBUG_PRINT(MSG_ALWAYS, ("SI_CecSacReportAudioCapabilitySend(): None of the requested formats found. Replied with FeatureAbort.\n"));
        SiiCecFeatureAbortSend(CECOP_REQUEST_SHORT_AUDIO, CECAR_INVALID_OPERAND, destLogAddr);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC Request Short Audio Descriptor" message to request
//!             audio capabilities (supported formats) of the Amplifier
//!
//! @param[in]  destLogAddr   - logical address of the destination device (AMP),
//! @param[in]  nFmt          - number of requested audio formats [1-4],
//! @param[in]  aAudioFormats - list of requested audio format codes
//-------------------------------------------------------------------------------------------------

void SiiCecSacRequestAudioCapabilitySend (const uint8_t destLogAddr, uint8_t nFmt, CecSacAudioFmtCode_t aAudioFormats[4])
{
    SiiCpiData_t cecFrame;
    uint8_t i;
    uint8_t AudioFormatID = 0; // Indicates that [Audio Format Code] and [Short Audio Descriptor] are as defined in CEA-861-D

    if ((nFmt > 0) && (nFmt <= 4))
    {
        for (i = 0; i < nFmt; i++)
        {
            cecFrame.args[i] = (AudioFormatID << 6) | aAudioFormats[i]; // Audio Format ID [7-6] + Audio Format Code [5-0]
        }
        SiiCecCpiWrite(CECOP_REQUEST_SHORT_AUDIO, nFmt, destLogAddr, &cecFrame);
    }
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC User Control Pressed" message to indicate that the
//!             user pressed a remote control button or switched from one
//!             remote control button to another
//!
//! @param[in]  uiCommand   - UI Command code from the following set:
//!                           CEC_RC_VOLUME_UP, CEC_RC_VOLUME_DOWN, CEC_RC_MUTE
//!                           CEC_RC_MUTE_FUNCTION, CEC_RC_RESTORE_VOLUME_FUNCTION
//! @param[in]  destLogAddr - logical address of the destination device (TV,STB)
//-------------------------------------------------------------------------------------------------

void SiiCecSacUserControlPressedSend (const SiiCecUiCommand_t uiCommand, const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    cecFrame.args[0] = (uint8_t) uiCommand;   // UI Command
    SiiCecCpiWrite(CECOP_USER_CONTROL_PRESSED, 1, destLogAddr, &cecFrame);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Sends "CEC User Control Released" message to indicate that the
//!             user released the remote control button that had been pressed
//!             and accompanied by the last "CEC User Control Pressed" message
//!
//! @param[in]  destLogAddr - logical address of the destination device (TV,STB)
//-------------------------------------------------------------------------------------------------

void SiiCecSacUserControlReleasedSend (const uint8_t destLogAddr)
{
    SiiCpiData_t cecFrame;
    SiiCecCpiWrite(CECOP_USER_CONTROL_RELEASED, 0, destLogAddr, &cecFrame);
}


//-------------------------------------------------------------------------------------------------
//! @brief      Verifies if the CEC messaged belongs to the SAC group of messages
//!
//! @param[in]  pCecMsg  - pointer to CEC message structure
//! @retval     true  - if the message belongs to CEC/SAC group
//! @retval     false - otherwise
//-------------------------------------------------------------------------------------------------

bool_t CecSacIsMessageValid (SiiCpiData_t *pCecMsg)
{
    bool_t isValid = false;

    if ((pCecMsg->srcDestAddr & 0xF) == SiiCecGetDeviceLA())
    {
        switch(pCecMsg->opcode)
        {
            case CECOP_SYSTEM_AUDIO_MODE_REQUEST:
            case CECOP_GIVE_AUDIO_STATUS:
            case CECOP_SET_SYSTEM_AUDIO_MODE:
            case CECOP_REPORT_AUDIO_STATUS:
            case CECOP_GIVE_SYSTEM_AUDIO_MODE_STATUS:
            case CECOP_SYSTEM_AUDIO_MODE_STATUS:
            case CECOP_REPORT_SHORT_AUDIO:
            case CECOP_REQUEST_SHORT_AUDIO:
            case CECOP_USER_CONTROL_PRESSED:
            case CECOP_USER_CONTROL_RELEASED:
                isValid = true;
                break;
            default:break;
        }
    }

    return isValid;
}
