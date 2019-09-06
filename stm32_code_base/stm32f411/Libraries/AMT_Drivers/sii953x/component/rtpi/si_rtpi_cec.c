//***************************************************************************
//!file     si_rtpi_cec.c
//!brief    Silicon Image RTPI Component CEC command processing.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************

#include "string.h"
#include "si_rtpi_internal.h"
#if INC_CEC
#include "si_cec_component.h"
#endif
#if INC_CEC_SAC
#include "si_sac_main.h"
#include "si_sac_msg.h"
#endif
//-------------------------------------------------------------------------------------------------
//! @brief      Execute CEC commands written to the 0x6x register set
//-------------------------------------------------------------------------------------------------
bool_t RtpiExecuteCecCmd ( uint8_t cmdType )
{
    bool_t  executedCmd = true;
    uint8_t cmdBuf0In, resultCode;
#if INC_CEC_SAC
    uint8_t dummy;
    uint8_t cmdBuf0Out, cmdBuf1Out, cmdBuf2Out, cmdBuf3Out;
    int i;
#endif
    uint8_t nameBuf[16];

    resultCode = RP_STS__NO_ERROR;
    cmdBuf0In = SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_BUF );
    switch ( cmdType )
    {
#if INC_CEC_SAC
        case RP_C__CAT_CMD__SAC_INITIATE:
            if ( !SiiCecSacInitiate())
            {
                resultCode = RP_STS__ERROR_FAILED;
            }
            break;

        case RP_C__CAT_CMD__SAC_TERMINATE:
            if ( !SiiCecSacTerminate())
            {
                resultCode = RP_STS__ERROR_FAILED;
            }
            break;

        case RP_C__CAT_CMD__SAC_STATUS_GET:
            cmdBuf0Out = SiiCecSacIsSysAudioModeEnabled() ? 0x01 : 0x00;
            cmdBuf0Out |= SiiCecSacIsMuted() ? 0x02 : 0x00;
            cmdBuf1Out = SiiCecSacVolumeGet();
            SiiRpForceWriteByte(RP_A__CMD_BUF, cmdBuf0Out );
            SiiRpForceWriteByte(RP_A__CMD_BUF1, cmdBuf1Out );

            // Create supported formats bit field 1
            cmdBuf2Out = 0;
            for ( i = 1; i < 9; i++ )
            {
                if ( SiiCecSacAudioFormatDetailsGet( i, &dummy, &dummy, &dummy ))
                    cmdBuf2Out |= (1 << (i - 1));
            }
            // Create supported formats bit field 2
            cmdBuf3Out = 0;
            for ( i = 9; i < 16; i++ )
            {
                if ( SiiCecSacAudioFormatDetailsGet( i, &dummy, &dummy, &dummy ))
                    cmdBuf3Out |= (1 << (i - 9));
            }

            SiiRpForceWriteByte( RP_A__CMD_BUF2, cmdBuf2Out );
            SiiRpForceWriteByte( RP_A__CMD_BUF3, cmdBuf3Out );
            break;

        case RP_C__CAT_CMD__SAC_SHORT_AUDIO_DESCRIPTOR_GET:
            if ( SiiCecSacAudioFormatDetailsGet( cmdBuf0In, &cmdBuf0Out, &cmdBuf1Out, &cmdBuf2Out ))
            {
                SiiRpForceWriteByte( RP_A__CMD_BUF, cmdBuf0Out );
                SiiRpForceWriteByte( RP_A__CMD_BUF1, cmdBuf1Out );
                SiiRpForceWriteByte( RP_A__CMD_BUF2, cmdBuf2Out );
            }
            else
            {
                resultCode = RP_STS__ERROR_FAILED;
            }
            break;

        case RP_C__CAT_CMD__SAC_STATUS_SET:
            SiiCecSacMute(( cmdBuf0In & BIT1) ? CEC_SAC_MUTE_ON : CEC_SAC_MUTE_OFF );
            SiiCecSacVolumeSet( SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_BUF1 ));
            break;

        case RP_C__CAT_CMD__SAC_SHORT_AUDIO_DESCRIPTOR_SET:
        {
            uint8_t cmdBuf1In       = SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_BUF1 );
            uint8_t audioFormatCode = (cmdBuf1In >> 3) & 0x0F;
            uint8_t maxChanNumber   = cmdBuf1In & 0x07;
            uint8_t sampleRate      = SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_BUF2 );
            uint8_t quality         = SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_BUF3 );

            // Add to our list, overwriting former entry if necessary
            if ( !SiiCecSacAddFormat( (int)cmdBuf0In, audioFormatCode, maxChanNumber, sampleRate, quality ))
            {
                resultCode = RP_STS__ERROR_FAILED;
            }
            break;
        }
#endif
        case RP_C__CAT_CMD__CEC_SET_OSD_NAME:
            if (( cmdBuf0In > 1) && ( cmdBuf0In < 15))
            {
                SiiRpReadBlock( RP_A__CMD_BUF1, nameBuf, cmdBuf0In );
                nameBuf[ cmdBuf0In] = 0;
                SiiCecSetOsdName( (char *)nameBuf );
            }
            else
            {
                resultCode = RP_STS__ERROR_INVALID_OPERAND;
            }
            break;

        default:
            executedCmd = false;        // command does not belong to us
            break;
    }

    if ( executedCmd )
    {
        SiiRpForceWriteByte( RP_A__CMD_STATUS, resultCode );
    }
    return( executedCmd );
}



