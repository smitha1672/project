//***************************************************************************
//!file     sk_app_cec_logger.c
//!brief
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_cec_component.h"
#include "si_cec_enums.h"
#include "string.h"

#if INC_CEC_LOGGER

static char *deckControlMsgs [] =
    {
    "SKIP_FWD",
    "SKIP_BACK",
    "STOP",
    "EJECT"
    };

static char *l_deviceTypeNames [] =
{
    "TV",
    "RecDev",
    "Reserved",
    "Tuner",
    "Playback",
    "Audio System",
    "Pure CEC Switch",
    "Video Processor"
};

static char l_descBuffer [40];

//------------------------------------------------------------------------------
// Function:    CpCecTranslateDeviceType
// Description: Returns device type string from passed device type index
//------------------------------------------------------------------------------

char *CpCecTranslateDeviceType ( uint8_t deviceType )
{
    char *pStr = "Invalid";

    if ( deviceType < CEC_DT_COUNT )
    {
        pStr = l_deviceTypeNames[ deviceType];
    }
    return( pStr );
}

//------------------------------------------------------------------------------
// Function:    CpCecTranslateLA
// Description:
//
// Returns a string of up to 19 bytes (including null terminator)
//------------------------------------------------------------------------------

char *CpCecTranslateLA ( uint8_t bLogAddr )
{
    char *pStr;

    switch ( bLogAddr & CEC_LOGADDR_UNREGORBC )
    {
        case CEC_LOGADDR_TV:        pStr = "TV";                   break;
        case CEC_LOGADDR_RECDEV1:   pStr = "RecDev 1";             break;
        case CEC_LOGADDR_RECDEV2:   pStr = "RecDev 2";             break;
        case CEC_LOGADDR_TUNER1:    pStr = "Tuner 1";              break;
        case CEC_LOGADDR_PLAYBACK1: pStr = "Playback 1";           break;
        case CEC_LOGADDR_AUDSYS:    pStr = "Audio System";         break;
        case CEC_LOGADDR_PURE_SWITCH:    pStr = "Pure Switch";              break;
        case CEC_LOGADDR_VIDEO_PROCESSOR:    pStr = "Video Processor";              break;
        case CEC_LOGADDR_PLAYBACK2: pStr = "Playback 2";           break;
        case CEC_LOGADDR_RECDEV3:   pStr = "RecDev 3";             break;
        case CEC_LOGADDR_TUNER4:    pStr = "Tuner 4";              break;
        case CEC_LOGADDR_PLAYBACK3: pStr = "Playback 3";           break;
        case CEC_LOGADDR_RES3:      pStr = "Res3";                 break;
        case CEC_LOGADDR_RES4:      pStr = "Res4";                 break;
        case CEC_LOGADDR_FREEUSE:   pStr = "FreeUse";              break;
        case CEC_LOGADDR_UNREGORBC: pStr = "Unreg or Broadcast";   break;
        default:
                                    pStr = "UNKNOWN LA!";
    }

    return( pStr );
}

//------------------------------------------------------------------------------
// Function:    CpCecTranslateOpcodeName
// Description:
//------------------------------------------------------------------------------

char *CpCecTranslateOpcodeName ( SiiCpiData_t *pMsg )
    {
    char *pName, *pOperand;
    int         i, outIndex;

    switch ( pMsg->opcode )
        {
        case CECOP_SII_SENDPING:                pName = "PING";                         break;
        case CECOP_FEATURE_ABORT:               pName = "FEATURE_ABORT";                break;
        case CECOP_IMAGE_VIEW_ON:               pName = "IMAGE_VIEW_ON";                break;
        case CECOP_TUNER_STEP_INCREMENT:        pName = "TUNER_STEP_INCREMENT";         break;
        case CECOP_TUNER_STEP_DECREMENT:        pName = "TUNER_STEP_DECREMENT";         break;
        case CECOP_TUNER_DEVICE_STATUS:         pName = "TUNER_DEVICE_STATUS";          break;
        case CECOP_GIVE_TUNER_DEVICE_STATUS:    pName = "GIVE_TUNER_DEVICE_STATUS";     break;
        case CECOP_RECORD_ON:                   pName = "RECORD_ON";                    break;
        case CECOP_RECORD_STATUS:               pName = "RECORD_STATUS";                break;
        case CECOP_RECORD_OFF:                  pName = "RECORD_OFF";                   break;
        case CECOP_TEXT_VIEW_ON:                pName = "TEXT_VIEW_ON";                 break;
        case CECOP_RECORD_TV_SCREEN:            pName = "RECORD_TV_SCREEN";             break;
        case CECOP_GIVE_DECK_STATUS:            pName = "GIVE_DECK_STATUS";             break;
        case CECOP_DECK_STATUS:                 pName = "DECK_STATUS";                  break;
        case CECOP_SET_MENU_LANGUAGE:           pName = "SET_MENU_LANGUAGE";            break;
        case CECOP_CLEAR_ANALOGUE_TIMER:        pName = "CLEAR_ANALOGUE_TIMER";         break;
        case CECOP_SET_ANALOGUE_TIMER:          pName = "SET_ANALOGUE_TIMER";           break;
        case CECOP_TIMER_STATUS:                pName = "TIMER_STATUS";                 break;
        case CECOP_STANDBY:                     pName = "STANDBY";                      break;
        case CECOP_PLAY:
            pName = "PLAY";
            switch ( pMsg->args[0] )
                {
                case CEC_PLAY_SCAN_FWD_MIN: pOperand = "SCAN_FWD_MIN";  break;
                case CEC_PLAY_SCAN_FWD_MED: pOperand = "SCAN_FWD_MED";  break;
                case CEC_PLAY_SCAN_FWD_MAX: pOperand = "SCAN_FWD_MAX";  break;
                case CEC_PLAY_SCAN_REV_MIN: pOperand = "SCAN_REV_MIN";  break;
                case CEC_PLAY_SCAN_REV_MED: pOperand = "SCAN_REV_MED";  break;
                case CEC_PLAY_SCAN_REV_MAX: pOperand = "SCAN_REV_MAX";  break;
                case CEC_PLAY_SLOW_FWD_MIN: pOperand = "SLOW_FWD_MIN";  break;
                case CEC_PLAY_SLOW_FWD_MED: pOperand = "SLOW_FWD_MED";  break;
                case CEC_PLAY_SLOW_FWD_MAX: pOperand = "SLOW_FWD_MAX";  break;
                case CEC_PLAY_SLOW_REV_MIN: pOperand = "SLOW_REV_MIN";  break;
                case CEC_PLAY_SLOW_REV_MED: pOperand = "SLOW_REV_MED";  break;
                case CEC_PLAY_SLOW_REV_MAX: pOperand = "SLOW_REV_MAX";  break;
                case CEC_PLAY_REVERSE:      pOperand = "REVERSE";       break;
                case CEC_PLAY_FORWARD:      pOperand = "FORWARD";       break;
                case CEC_PLAY_STILL:        pOperand = "STILL";         break;
                default:                    pOperand = "UNKNOWN";       break;
                };
            SiiSprintf( l_descBuffer, "%s (%s)", pName, pOperand );
            pName = l_descBuffer;
            break;
        case CECOP_DECK_CONTROL:
            pName = "DECK_CONTROL";
            SiiSprintf( l_descBuffer, "%s (%s)", pName, deckControlMsgs[ pMsg->args[0] - 1] );
            pName = l_descBuffer;
            break;
        case CECOP_TIMER_CLEARED_STATUS:        pName = "TIMER_CLEARED_STATUS";         break;
        case CECOP_USER_CONTROL_PRESSED:        pName = "USER_CONTROL_PRESSED";         break;
        case CECOP_USER_CONTROL_RELEASED:       pName = "USER_CONTROL_RELEASED";        break;
        case CECOP_GIVE_OSD_NAME:               pName = "GIVE_OSD_NAME";                break;
        case CECOP_SET_OSD_NAME:
        {
            pName = "SET_OSD_NAME";
            outIndex = SiiSprintf( l_descBuffer, "%s (", pName );
            for ( i = 0; i < pMsg->argCount; i++ )
            {
                l_descBuffer[ outIndex++] = pMsg->args[i];
            }
            l_descBuffer[ outIndex] = 0;

            strcat( l_descBuffer, ")" );
            pName = l_descBuffer;
            break;
        }
        case CECOP_SET_OSD_STRING:
        {
            pName = "SET_OSD_STRING";
            outIndex = SiiSprintf( l_descBuffer, "%s (", pName );
            for ( i = 1; i < pMsg->argCount; i++ )
                l_descBuffer[ outIndex++] = pMsg->args[i];
            l_descBuffer[ outIndex] = 0;

            strcat( l_descBuffer, ")" );
            pName = l_descBuffer;
            break;
        }
        case CECOP_SET_TIMER_PROGRAM_TITLE:     pName = "SET_TIMER_PROGRAM_TITLE";      break;
        case CECOP_SYSTEM_AUDIO_MODE_REQUEST:   pName = "SYSTEM_AUDIO_MODE_REQUEST";    break;
        case CECOP_GIVE_AUDIO_STATUS:           pName = "GIVE_AUDIO_STATUS";            break;
        case CECOP_SET_SYSTEM_AUDIO_MODE:       pName = "SET_SYSTEM_AUDIO_MODE";        break;
        case CECOP_REPORT_AUDIO_STATUS:         pName = "REPORT_AUDIO_STATUS";          break;
        case CECOP_GIVE_SYSTEM_AUDIO_MODE_STATUS: pName = "GIVE_SYSTEM_AUDIO_MODE_STATUS";  break;
        case CECOP_SYSTEM_AUDIO_MODE_STATUS:    pName = "SYSTEM_AUDIO_MODE_STATUS";     break;
        case CECOP_ROUTING_CHANGE:              pName = "ROUTING_CHANGE";               break;
        case CECOP_ROUTING_INFORMATION:         pName = "ROUTING_INFORMATION";          break;
        case CECOP_ACTIVE_SOURCE:               pName = "ACTIVE_SOURCE";                break;
        case CECOP_GIVE_PHYSICAL_ADDRESS:       pName = "GIVE_PHYSICAL_ADDRESS";        break;
        case CECOP_REPORT_PHYSICAL_ADDRESS:     pName = "REPORT_PHYSICAL_ADDRESS";      break;
        case CECOP_REQUEST_ACTIVE_SOURCE:       pName = "REQUEST_ACTIVE_SOURCE";        break;
        case CECOP_SET_STREAM_PATH:             pName = "SET_STREAM_PATH";              break;
        case CECOP_DEVICE_VENDOR_ID:            pName = "DEVICE_VENDOR_ID";             break;
        case CECOP_VENDOR_COMMAND:              pName = "VENDOR_COMMAND";               break;
        case CECOP_VENDOR_REMOTE_BUTTON_DOWN:   pName = "VENDOR_REMOTE_BUTTON_DOWN";    break;
        case CECOP_VENDOR_REMOTE_BUTTON_UP:     pName = "VENDOR_REMOTE_BUTTON_UP";      break;
        case CECOP_GIVE_DEVICE_VENDOR_ID:       pName = "GIVE_DEVICE_VENDOR_ID";        break;
        case CECOP_MENU_REQUEST:                pName = "MENU_REQUEST";                 break;
        case CECOP_MENU_STATUS:                 pName = "MENU_STATUS";                  break;
        case CECOP_GIVE_DEVICE_POWER_STATUS:    pName = "GIVE_DEVICE_POWER_STATUS";     break;
        case CECOP_REPORT_POWER_STATUS:         pName = "REPORT_POWER_STATUS";          break;
        case CECOP_GET_MENU_LANGUAGE:           pName = "GET_MENU_LANGUAGE";            break;
        case CECOP_SELECT_ANALOGUE_SERVICE:     pName = "SELECT_ANALOGUE_SERVICE";      break;
        case CECOP_SELECT_DIGITAL_SERVICE:      pName = "SELECT_DIGITAL_SERVICE";       break;
        case CECOP_SET_DIGITAL_TIMER:           pName = "SET_DIGITAL_TIMER";            break;
        case CECOP_CLEAR_DIGITAL_TIMER:         pName = "CLEAR_DIGITAL_TIMER";          break;
        case CECOP_SET_AUDIO_RATE:              pName = "SET_AUDIO_RATE";               break;
        case CECOP_INACTIVE_SOURCE:             pName = "INACTIVE_SOURCE";              break;
        case CECOP_CEC_VERSION:                 pName = "CEC_VERSION";                  break;
        case CECOP_GET_CEC_VERSION:             pName = "GET_CEC_VERSION";              break;
        case CECOP_VENDOR_COMMAND_WITH_ID:      pName = "VENDOR_COMMAND_WITH_ID";       break;
        case CECOP_CLEAR_EXTERNAL_TIMER:        pName = "CLEAR_EXTERNAL_TIMER";         break;
        case CECOP_SET_EXTERNAL_TIMER:          pName = "SET_EXTERNAL_TIMER";           break;
        case CECOP_REPORT_SHORT_AUDIO:          pName = "REPORT_SHORT_AUDIO";           break;
        case CECOP_REQUEST_SHORT_AUDIO:         pName = "REQUEST_SHORT_AUDIO";          break;

        case CECOP_INITIATE_ARC:                pName = "INITIATE_ARC";                 break;
        case CECOP_REPORT_ARC_INITIATED:        pName = "REPORT_ARC_INITIATED";         break;
        case CECOP_REPORT_ARC_TERMINATED:       pName = "REPORT_ARC_TERMINATED";        break;
        case CECOP_REQUEST_ARC_INITIATION:      pName = "REQUEST_ARC_INITIATION";       break;
        case CECOP_REQUEST_ARC_TERMINATION:     pName = "REQUEST_ARC_TERMINATION";      break;
        case CECOP_TERMINATE_ARC:               pName = "TERMINATE_ARC";                break;

        case CDCOP_HEADER:
        {
            pName = "CDC Header opcode";
#if IS_CDC == 1
            switch ( pMsg->args[2] )    // CDC opcode is parameter byte 2
            {
                case CDCOP_HEC_INQUIRE_STATE:       pName = "CDC::HEC_INQUIRE_STATE";       break;
                case CDCOP_HEC_REPORT_STATE:        pName = "CDC::HEC_REPORT_STATE";        break;
                case CDCOP_HEC_SET_STATE_ADJACENT:  pName = "CDC::HEC_SET_STATE_ADJACENT";  break;
                case CDCOP_HEC_SET_STATE:           pName = "CDC::HEC_SET_STATE";           break;
                case CDCOP_HEC_NOTIFY_ALIVE:        pName = "CDC::HEC_NOTIFY_ALIVE";        break;
                case CDCOP_HEC_DISCOVER:            pName = "CDC::HEC_DISCOVER";            break;
                case CDCOP_HPD_SET_STATE:           pName = "CDC::HPD_SET_STATE";           break;
                case CDCOP_HPD_REPORT_STATE:        pName = "CDC::HPD_REPORT_STATE";        break;
            };
#endif
            break;
        }
        case CECOP_ABORT:                       pName = "ABORT";                        break;
        default:                                pName = "UNKNOWN";                      break;
    }

    return( pName );
}

//------------------------------------------------------------------------------
// Function:    CpCecPrintCommand
// Description: Translate CEC transaction into human-readable form.
//------------------------------------------------------------------------------

void SkAppCecMessageLogger ( SiiCpiData_t *pMsg, int systemType, bool_t isTx )
{
    char        logStr [120];
    uint16_t    writeCnt;
    int         i;

    writeCnt = SiiSprintf(
        logStr, "CEC%s[%c%02X][%02X]: ",
        (systemType == 0) ? "RX" : "TX",
        isTx ? 'W' : 'R', pMsg->srcDestAddr, pMsg->opcode
        );
    if ( pMsg->argCount > 14 )
    {
        pMsg->argCount = 14;
    }
    for ( i = 0; i < pMsg->argCount; i++ )
    {
        writeCnt = SiiSprintf( logStr, "%s%02X ", logStr, pMsg->args[ i] );
    }

    /* Add human readable command name  */

    SiiSprintf(
        logStr, "%s%*s%s",
        logStr,
        52 - writeCnt, "; ",
        CpCecTranslateOpcodeName( pMsg )
        );
    SiiSprintf( logStr, "%s\n", logStr );

    DEBUG_PRINT( MSG_STAT, logStr );
    }

#endif  //#if INC_CEC_LOGGER
