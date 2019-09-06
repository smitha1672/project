//***************************************************************************
//!file     si_rtpi_cbus.c
//!brief    Silicon Image RTPI Component CBUS command processing.
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
#if INC_CBUS
#include "si_cbus_internal.h"
#include "si_cbus_component.h"
#endif

//-------------------------------------------------------------------------------------------------
//! @brief      Execute CBUS commands written to the 0x6x register set
//-------------------------------------------------------------------------------------------------
bool_t RtpiExecuteCbusCmd ( uint8_t cmdType )
{
    bool_t  executedCmd = true;

    switch ( cmdType )
    {
        case RP_C__CAT_CMD__MHL_WRITE_MSG:
            {
                bool_t result = 0;

                // For MHL Execute commands, register RP_A__CMD_DEVICE_ID contains the port number
                SiiCbusInstanceSetBasedOnPort(SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_DEVICE_ID));
                switch (SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_BUF))//sub command
                {
                    case MHL_MSC_MSG_RCP:
                        result = SiiMhlRxSendRCPCmd(SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_BUF1)); //data in 0x71
                        break;
                    case MHL_MSC_MSG_RCPK:
                        result = SiiMhlRxSendRapk(SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_BUF1)); //data in 0x71
                        break;
                    case MHL_MSC_MSG_RCPE:
                        result = SiiMhlRxSendRcpe(SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_BUF1)); //data in 0x71
                        break;
                }
                SiiRpForceWriteByte(RP_A__CMD_BUF2, result);
            }
            break;
        case RP_C__CAT_CMD__MHL_READ_MSG:
            {
                bool_t result;
                uint8_t msgCmd;
                uint8_t msgData;
                SiiCbusInstanceSetBasedOnPort(SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_DEVICE_ID));
                result = SiiMhlRxGetMscMsgCmd(&msgCmd, &msgData);
                SiiRpForceWriteByte(RP_A__CMD_BUF, msgCmd);
                SiiRpForceWriteByte(RP_A__CMD_BUF1, msgData);
                SiiRpForceWriteByte(RP_A__CMD_BUF2, result);
            }
            break;
        case RP_C__CAT_CMD__MHL_WRITE_BURST:
            {
                uint8_t buf[16];
                uint16_t offset, length;
                bool_t result;
                SiiCbusInstanceSetBasedOnPort(SiiPlatformI2cCbSlaveReadByte(1, RP_A__CMD_DEVICE_ID));
                SiiRpReadBlock(RP_A__CMD_BUF, buf, 16);
                offset = SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_ADDR_L );
                length = SiiPlatformI2cCbSlaveReadByte( 1, RP_A__CMD_LENGTH );
                if( length > 16 )
                {
                    length = 16;
                }
                result = SiiMhlRxWritePeerScratchpad(offset, length, &buf[0]);
                SiiRpForceWriteByte(RP_A__CMD_BUF2, result);
            }
            break;
        default:
            executedCmd = false;        // command does not belong to us
            break;
    }

    return( executedCmd );
}

