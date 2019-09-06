//***************************************************************************
//!file     sk_app_cbus_demo.c
//!brief
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_app_cbus.h"
#include "si_regs_mhl953x.h"    //TODO: direct register access is not allowed at the app layer

extern uint8_t portArray[2];

static void PrintSendErr( char *pStr )
{
    DEBUG_PRINT( MSG_ERR, "Couldn't send %s to peer\n", pStr );
}

// Selecting the MHL port that you want to sent the demo command on
static bool_t SelectMHLPort()
{
	uint8_t   	i, port;
	bool_t		isMHLPort;

	DEBUG_PRINT( MSG_ALWAYS, "Which MHL port you want to use for this demo: \n" );
	isMHLPort = false;
	port = SkAppIrGetSingleDigit( SII_INPUT_PORT_COUNT - 1 );
	for ( i = 0; i < MHL_MAX_CHANNELS; i++ )
	{
		if ( portArray[i] == port )
		{
			isMHLPort = true;
			break;
		}
	}

	if( isMHLPort )
	{
		SkAppCbusInstanceSet(i);
		if( !SiiMhlRxCbusConnected())
		{
			DEBUG_PRINT( MSG_ALWAYS, "This CBus channel is not connected!, Please select the one that is connected! \n" );
			return false;
		}
	}
	else
	{
		DEBUG_PRINT( MSG_ALWAYS, "You have chosen a Non-MHL port, please choose wisely!! \n" );
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
// Function:    DemoCbus
// Description: CBUS demo command options
// Returns:
//------------------------------------------------------------------------------

static char *pCbusDemoMenu [] =
{
    "\nCBUS Demo",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    "1 - Display this device's DEV CAPS",
    "2 - Display connected device's DEV CAPS",
    "3 - Send MHL_SET_HPD msg",
    "4 - Send SET_INT msg",
    "5 - Send WRITE_STAT msg",
    "6 - Send MHL_MSC_MSG -> ( RCP 'PLAY' cmd ) msg",
    "7 - Send MHL_WRITE_BURST msg",
    NULL
};

bool_t SkAppCbusDemo ( RC5Commands_t key )
{
    uint8_t   	i;
    uint8_t 	devCapValue;
    uint16_t 	regAddr;
    cbus_req_t 	req;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pCbusDemoMenu );
            break;

        case RC5_0:
            app.demoMenuLevel = 0;
            PrintAlways( demoMenuExit );
            break;

        case RC5_1:
         	 // Get our DEV CAPS
            PrintAlways( "\n\nMy Device Capability Regs are:: \n" );
         	for ( i = 0; i < 16; i++ )
         	{
         		regAddr = REG_CBUS_DEVICE_CAP_0 + i;
         		devCapValue = SiiDrvCbusRegisterGet( regAddr );
         		DEBUG_PRINT( MSG_ALWAYS, " Reg %02X :: %02X\n", REG_CBUS_DEVICE_CAP_0 + i, devCapValue );
         	}
             break;

        case RC5_2:
        	if( SelectMHLPort() )
        	{
        		DEBUG_PRINT( MSG_ALWAYS, "Which peer's dev cap register offset you want to read: 0 to 9 ?\n" );
        		i = SkAppIrGetSingleDigit( 9 );
        		DEBUG_PRINT( MSG_ALWAYS, "\n\n Reading peer's Dev Cap register %02X: \n", i);
				// reading register offset
				SiiMhlRxReadDevCapReg(i);
        	}
            break;

        case RC5_3:
        	if( SelectMHLPort() )
        	{
				 // Send MHL_CLR_HPD / and then send MHL_SET_HPD
				PrintAlways( "\n\nSending MHL_CLR_HPD to peer\n" );
				if( !(SiiMhlRxHpdSet( false )))
				{
					PrintSendErr( "MHL_CLR_HPD" );
				}

				PrintAlways( "\n\nSending MHL_SET_HPD to peer\n" );
				if( !(SiiMhlRxHpdSet( true )))
				{
					PrintSendErr( "MHL_SET_HPD" );
				}
        	}
            break;

        case RC5_4:
        	if( SelectMHLPort() )
        	{
        		PrintAlways( "\n\nSending MHL_SET_INT to peer, sending EDID_CHG interrupt" );
				if( !SiiMhlRxSendEdidChange())
				{
					PrintSendErr( "\nSiiMhlRxSendEdidChange() failed!!\n" );
				}
        	}
            break;

        case RC5_5:
        	if( SelectMHLPort() )
        	{
        		PrintAlways( "\n\nSending MHL_WRITE_STAT to peer, sending PATH_EN status to peer" );
				if( !SiiMhlRxPathEnable( true ) )
				{
					PrintSendErr( "SiiMhlRxPathEnable() failed!!\n" );
				}
        	}
			break;

        case RC5_6:
        	if( SelectMHLPort() )
        	{
				PrintAlways( "\n\nSending MHL_MSC_MSG -> ( RCP 'PLAY' cmd ) msg to peer\n" );
				if( !(SiiMhlRxSendRCPCmd(MHL_RCP_CMD_PLAY)) )
				{
					PrintSendErr( "MHL_MSC_MSG" );
				}
        	}
            break;
        case RC5_7:
        	if( SelectMHLPort() )
        	{
				PrintAlways( "\n\nSending MHL_WRITE_BURST to peer, check peer's scratchpad registers!!\n" );
				// write different values in different registers
				for ( i = 0; i < 16; i++ )
				{
					req.msgData[i] = i + 5;
				}

				if( !(SiiMhlRxWritePeerScratchpad(0, 16, &req.msgData[0])) )
				{
					PrintSendErr( "MHL_WRITE_BURST" );
				}
        	}
			break;
        default:
            break;
    }

    return( false );
}

