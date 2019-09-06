//***************************************************************************
//!file     si_cbus_rcp_parse.c
//!brief    CBUS RCP parse interface
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"
#include "si_common.h"
#include "si_cbus_internal.h"
#include "si_edid_tx_component.h"

//------------------------------------------------------------------------------
// Module constants
//------------------------------------------------------------------------------

#define MHL_LD_INVALID  (0)

//------------------------------------------------------------------------------
// Module data
//------------------------------------------------------------------------------

static uint8_t rcpValidate [128] =
{
    LD_MHL_RCP_CMD_SELECT,
    LD_MHL_RCP_CMD_UP,
    LD_MHL_RCP_CMD_DOWN,
    LD_MHL_RCP_CMD_LEFT,
    LD_MHL_RCP_CMD_RIGHT,
    LD_MHL_RCP_CMD_RIGHT_UP,
    LD_MHL_RCP_CMD_RIGHT_DOWN,
    LD_MHL_RCP_CMD_LEFT_UP,
    LD_MHL_RCP_CMD_LEFT_DOWN,
    LD_MHL_RCP_CMD_ROOT_MENU,
    LD_MHL_RCP_CMD_SETUP_MENU,
    LD_MHL_RCP_CMD_CONTENTS_MENU,
    LD_MHL_RCP_CMD_FAVORITE_MENU,
    LD_MHL_RCP_CMD_EXIT,

    //0x0E - 0x1F Reserved
    MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,

    LD_MHL_RCP_CMD_NUM_0,
    LD_MHL_RCP_CMD_NUM_1,
    LD_MHL_RCP_CMD_NUM_2,
    LD_MHL_RCP_CMD_NUM_3,
    LD_MHL_RCP_CMD_NUM_4,
    LD_MHL_RCP_CMD_NUM_5,
    LD_MHL_RCP_CMD_NUM_6,
    LD_MHL_RCP_CMD_NUM_7,
    LD_MHL_RCP_CMD_NUM_8,
    LD_MHL_RCP_CMD_NUM_9,
    LD_MHL_RCP_CMD_DOT,
    LD_MHL_RCP_CMD_ENTER,
    LD_MHL_RCP_CMD_CLEAR,

    //0x2D - 0x2F Reserved
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,

    LD_MHL_RCP_CMD_CH_UP,
    LD_MHL_RCP_CMD_CH_DOWN,
    LD_MHL_RCP_CMD_PRE_CH,
    LD_MHL_RCP_CMD_SOUND_SELECT,
    LD_MHL_RCP_CMD_INPUT_SELECT,
    LD_MHL_RCP_CMD_SHOW_INFO,
    LD_MHL_RCP_CMD_HELP,
    LD_MHL_RCP_CMD_PAGE_UP,
    LD_MHL_RCP_CMD_PAGE_DOWN,

    //0x39 - 0x3F Reserved
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,

    LD_MHL_RAP_CONTENT_ON,
    LD_MHL_RCP_CMD_VOL_UP,
    LD_MHL_RCP_CMD_VOL_DOWN,
    LD_MHL_RCP_CMD_MUTE,
    LD_MHL_RCP_CMD_PLAY,
    LD_MHL_RCP_CMD_STOP,
    LD_MHL_RCP_CMD_PAUSE,
    LD_MHL_RCP_CMD_RECORD,
    LD_MHL_RCP_CMD_REWIND,
    LD_MHL_RCP_CMD_FAST_FWD,
    LD_MHL_RCP_CMD_EJECT,
    LD_MHL_RCP_CMD_FWD,
    LD_MHL_RCP_CMD_BKWD,

    //0x4D - 0x4F Reserved
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,

    LD_MHL_RCP_CMD_ANGLE,
    LD_MHL_RCP_CMD_SUBPICTURE,

    //0x52 - 0x5F Reserved
    MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,

    LD_MHL_RCP_CMD_PLAY_FUNC,
    LD_MHL_RCP_CMD_PAUSE_PLAY_FUNC,
    LD_MHL_RCP_CMD_RECORD_FUNC,
    LD_MHL_RCP_CMD_PAUSE_REC_FUNC,
    LD_MHL_RCP_CMD_STOP_FUNC,
    LD_MHL_RCP_CMD_MUTE_FUNC,
    LD_MHL_RCP_CMD_UN_MUTE_FUNC,
    LD_MHL_RCP_CMD_TUNE_FUNC,
    LD_MHL_RCP_CMD_MEDIA_FUNC,

    //0x69 - 0x70 Reserved
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID,

    LD_MHL_RCP_CMD_F1,
    LD_MHL_RCP_CMD_F2,
    LD_MHL_RCP_CMD_F3,
    LD_MHL_RCP_CMD_F4,
    LD_MHL_RCP_CMD_F5,

    // 0x76 - 0x7F Reserved
    MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
    MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID, MHL_LD_INVALID,
};


//------------------------------------------------------------------------------
//! @brief  Check for valid CBUS RCP key code.  Although there is a one-to-one
//!         correspondence for most of the key codes, some codes supported by
//!         CEC are not supported by RCP and should be ignored.
//! @param[in]  CEC RC key
//------------------------------------------------------------------------------
bool_t SiiMhlValidateRcpKeyCode ( uint8_t keyData, uint8_t supportMask )
 {
    bool_t  validKey = true;

    // Is it valid for the specified logical device types
    if (( rcpValidate[ keyData & ~BIT7] & supportMask ) == 0 )
    {
        validKey = false;
    }

    return( validKey );
 }

//------------------------------------------------------------------------------
//! @brief  Send MSC_MSG (RCP) message to the CBUS channel.  Does not validate
//!         key code because we don't want to assume anything about the
//!         attached device support capabilities
//! @param[in]  keyId   RCP key ID value
//! @retval     true        - successful queue/write
//! @retval     false       - write and/or queue failed
//------------------------------------------------------------------------------
bool_t SiiMhlRxSendRCPCmd ( uint8_t keyCode )
{
    return( CBusSendMscMsgCmd( MHL_MSC_MSG_RCP, keyCode ));
}

//------------------------------------------------------------------------------
//! @brief  Transfer a CBUS message to CEC
//! @param[in]
//------------------------------------------------------------------------------
void CbusProcessRcpRap ( uint8_t cmd, uint8_t msgData )
{
    uint8_t     status;
    switch ( cmd )
    {
        case MHL_MSC_MSG_RCP:
            DEBUG_PRINT( MSG_DBG, "RCP received. Key Code: %02X\n", msgData );

            // Don't bother application with basic key code validation, do it here.
            if ( !SiiMhlValidateRcpKeyCode( msgData, MHL_LD_ALL ))
            {
                SiiMhlRxSendRcpe( MHL_MSC_MSG_RCP_INEFFECTIVE_KEY_CODE );
            }
            else
            {
                // Allow application to process key.
                status = SiiMhlCbRxRcpRapReceived( cmd, msgData );
                if ( status != MHL_MSC_MSG_RCP_NO_ERROR )
                {
                    SiiMhlRxSendRcpe( status );
                }
            }

            // Acknowledge command whether there was an error or not.
            SiiMhlRxSendRcpk( msgData );
            break;
        case MHL_MSC_MSG_RAP:
        	DEBUG_PRINT( MSG_DBG, "RAP received. Action Code: %02X\n", msgData );
            status = SiiMhlCbRxRcpRapReceived( cmd, msgData );
            SiiMhlRxSendRapk( status );
           break;
#if ((MHL_20 == ENABLE) || (MHL_EXTRA == ENABLE))
        case MHL_MSC_MSG_UCP:
            DEBUG_PRINT( MSG_DBG, "UCP received. Key Code: %02X\n", msgData );
            // Allow application to process key.
            status = SiiMhlCbRxRcpRapReceived( cmd, msgData );
            if ( status != MHL_MSC_MSG_UCP_NO_ERROR )
            {
                SiiMhlRxSendUcpe( status );
            }
            SiiMhlRxSendUcpk( msgData );
            break;
#endif

        case MHL_MSC_MSG_RCPK:
        case MHL_MSC_MSG_RCPE:
        case MHL_MSC_MSG_RAPK:
        case MHL_MSC_MSG_UCPK:
        case MHL_MSC_MSG_UCPE:
            status = SiiMhlCbRxRcpRapReceived( cmd, msgData );
            break;

        default:
            DEBUG_PRINT( MSG_DBG, "MSC_MSG sub-command not recognized!! Sending back MSGE code !!\n" );
            SiiMhlRxSendMsge();
            break;
    }
}
