//***************************************************************************
//!file     sk_app_cbus.c
//!brief    Wraps board and device functions for the CBUS component
//          and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_app_cbus.h"
#include "si_gpio.h"
#include "si_cra.h"
#include "si_regs_pp953x.h"
#include "si_regs_gpio953x.h"
#if INC_RTPI
#include "si_rtpi_component.h"
#endif
#if INC_CEC_SAC
#include "si_sac_main.h"
#endif
#include "si_timer.h"
#include "si_drv_tpi_system.h"
#include "sk_application.h"

uint8_t mhlcabledetect=0;
//------------------------------------------------------------------------------
//  CBUS Component configuration data
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Module variables
//------------------------------------------------------------------------------

SiiAppCbusInstanceData_t appCbus[MHL_MAX_CHANNELS];
SiiAppCbusInstanceData_t *pAppCbus;

uint8_t portArray[2] = {SI_MHL_PORT0, SI_MHL_PORT1};

//------------------------------------------------------------------------------
// Description: Set the driver global data pointer to the requested instance.
// Parameters:  instanceIndex
// Returns:     true if legal index, false if index value is illegal.
//------------------------------------------------------------------------------

bool_t SkAppCbusInstanceSet ( uint_t instanceIndex )
{
    if ( instanceIndex < SII_NUM_CBUS )
    {
        SiiCbusInstanceSet( instanceIndex );
    	pAppCbus = &appCbus[instanceIndex];
    	pAppCbus->instanceIndex = instanceIndex;
        return( true );
    }
    return( false );
}

//------------------------------------------------------------------------------
//! @brief      If specified port is connected as MHL, return true.  As a side
//!             effect, the port instance is selected.
//! @param[in]  portIndex   - RX port index
//! @return     bool_t      - true if port is a connected MHL channel
//------------------------------------------------------------------------------
bool_t SkAppSourceIsConnectedCbusPort ( SiiSwitchSource_t source )
{
    int channel;

    // Must be a TMDS source
    if ( source < SiiSwitchSourceInternalVideo)
    {
        for ( channel = 0; channel < MHL_MAX_CHANNELS; channel++ )
        {
            if ( source == portArray[channel] )
            {
                SkAppCbusInstanceSet( channel );
                if ( SiiMhlRxCbusConnected())
                {
                    return( true );
                }
            }
        }
    }
    return( false );
}

//------------------------------------------------------------------------------
// Function:    SkAppCbusSendRapMessage
// Parameters:  actCode - Action code.
// Returns:     true if successful, false if not MHL port or other failure.
//------------------------------------------------------------------------------

bool_t SkAppCbusSendRapMessage ( uint8_t actCode )
{
    bool_t  success = false;

    switch ( actCode )
    {
        case MHL_RAP_CMD_POLL:
        case MHL_RAP_CONTENT_ON:
        case MHL_RAP_CONTENT_OFF:
            if((success = SiiMhlRxSendRAPCmd( actCode )) == false)
            {
                DEBUG_PRINT( MSG_DBG, "Unable to send command....\n" );
            }
            break;
        default:
            DEBUG_PRINT( MSG_DBG, "Action code not recognized!!\n" );
            break;
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    SkAppCbusSendUcpMessage
// Parameters:  ucpData - ucp ASCII character
//
// Returns:     true if successful, false if not MHL port or other failure.
//------------------------------------------------------------------------------

bool_t  SkAppCbusSendUcpMessage ( uint8_t ucpData )
{
    bool_t  success;

    if((success = SiiMhlRxSendUCPCmd( ucpData )) == false)
    {
        DEBUG_PRINT( MSG_DBG, "Unable to send ucp....\n" );
    }

    return( success );
}

//------------------------------------------------------------------------------
// Function:    SkAppCbusProcessRcpMessage
// Description: Process the passed RCP message.
// Returns:     The RCPK status code.
//------------------------------------------------------------------------------
static uint8_t SkAppCbusProcessRcpMessage ( uint8_t rcpData )
{
    uint8_t rcpkStatus  = MHL_MSC_MSG_RCP_NO_ERROR;

    DEBUG_PRINT( MSG_DBG, "RCP Key Code: 0x%02X\n", (int)rcpData );

#if INC_RTPI
    SiiRtpiSetInt(RP_M__INTERRUPT2__CBUS_MSG_RECEIVED << 8);
#endif

#if INC_CEC
    {
#if INC_CEC_SAC
        if ( !SkAppAudioProcessSacCommand( rcpData ))
#endif
        {
            // Message wasn't processed, send it to CEC bus
            SkAppCbusRcpToCec( rcpData );
        }
    }
#endif

	return( rcpkStatus );
}

//------------------------------------------------------------------------------
//! @brief  Part of the callback from the CBUS component used for RAPx messages
// Returns:     The RAPK status code.
//------------------------------------------------------------------------------
static uint8_t SkAppCbusProcessRapMessage ( uint8_t cmd, uint8_t rapData )
{
    uint8_t rapkStatus = MHL_MSC_MSG_RAP_NO_ERROR;

    DEBUG_PRINT( MSG_DBG, "RAP Action Code: 0x%02X\n", (int)rapData );

    if ( cmd == MHL_MSC_MSG_RAP )
    {
        switch ( rapData )
        {
            case MHL_RAP_CMD_POLL:
                break;

            case MHL_RAP_CONTENT_ON:
                // MHL device sent a CONTENT ON message, change our source
                // selection to match the MHL device. If CEC is enabled, send
                // appropriate CEC messages.
                app.newSource[app.currentZone] = (SiiSwitchSource_t)pAppCbus->port;
                app.newAudioSource[app.currentZone] = SiiSwitchAudioSourceDefault;
                SiiMhlRxPathEnable( true );

                SiiDrvTpiTmdsOutputEnable(true);

#if INC_CEC
                SkAppCbusRapToCec( rapData );
#endif
                break;
            case MHL_RAP_CONTENT_OFF:
                // MHL device sent a CONTENT OFF message.  We need do nothing
                // unless CEC is enabled, in which case send appropriate
                // CEC messages.

                SiiDrvTpiTmdsOutputEnable(false);

#if INC_CEC
                SkAppCbusRapToCec( rapData );
#endif
                break;
            default:
                rapkStatus = MHL_MSC_MSG_RAP_UNRECOGNIZED_ACT_CODE;
                break;
        }

        if (( rapkStatus == MHL_MSC_MSG_RAP_UNSUPPORTED_ACT_CODE ) || ( rapkStatus == MHL_MSC_MSG_RAP_UNRECOGNIZED_ACT_CODE ))
        {
            DEBUG_PRINT( MSG_DBG, "\nUnsupported or unrecognized MHL RAP Action Code!!\n" );
        }
    }
    else    // Process RAPK command.
    {
#if INC_CEC
        SkAppCbusRapkToCec( rapData );
#endif
    }

    return( rapkStatus );
}

//------------------------------------------------------------------------------
// Function:    SkAppCbusProcessUcpMessage
// Description: Process the passed UCP message.
// Returns:     The UCPK status code.
//------------------------------------------------------------------------------

static uint8_t SkAppCbusProcessUcpMessage ( uint8_t ucpData )
{
    uint8_t ucpkStatus = MHL_MSC_MSG_UCP_NO_ERROR;

    DEBUG_PRINT( MSG_DBG, "UCP ascii Code: 0x%02X\n", ucpData );

    if( ucpData > CBUS_UCP_ASCII_LIMIT )
    {
        ucpkStatus = MHL_MSC_MSG_UCP_INEFFECTIVE_KEY_CODE;
        DEBUG_PRINT( MSG_DBG, ("code not effective!!\n" ));
    }

    return( ucpkStatus );
}

//------------------------------------------------------------------------------
//! @brief  This is a callback API for Cbus connection change, prototype
//!         is defined in si_cbus_component.h
//! @param[in]  connected   true/false
//------------------------------------------------------------------------------
void SiiMhlCbRxConnectChange(bool_t connected)
{
    DEBUG_PRINT( MSG_DBG, "\n Notification to Application:: Cbus Connection Status :: %s\n", connected ? "Connected" : "Disconnected" );

#if INC_CEC
    SkAppCbusConnectChangeCecUpdate( connected );
#endif
}

//------------------------------------------------------------------------------
// Function:    SiiMhlCbRxScratchpadWritten
// Description: This is a callback API for scratchpad being written by peer, prototype
//				is defined in si_cbus_component.h
// Parameters:  none
// Returns:     void
//------------------------------------------------------------------------------
void SiiMhlCbRxScratchpadWritten()
{
	DEBUG_PRINT( MSG_DBG, "\nNotification to Application:: Scratchpad written!!\n" );
}

//------------------------------------------------------------------------------
// Function:    SiiMhlCbRxMscCmdResponseData
// Description: Response data received from peer in response to an MSC command
//				used only for MHL_READ_DEVCAP, prototype is defined in
//				si_cbus_component.h
// Parameters:  cmd - original MSC cmd sent from this device, response of which
//				      came back from peer the the 'data' below
//				data - response data from peer
// Returns:     void
//------------------------------------------------------------------------------
void SiiMhlCbRxMscCmdResponseData (uint8_t cmd, uint8_t data)
{
	if( cmd == MHL_READ_DEVCAP )
	{
		DEBUG_PRINT( MSG_ALWAYS, "\n Last read peer's device capability register is: %02X\n", data);
	}
}

//------------------------------------------------------------------------------
//! @brief  This function is called from the CBUS component when a
//!         MSC_MSG (RCP/RAP) is received.  The prototype is defined in
//!         si_cbus_component.h
//! @param[in]  cmd     - RCP/RCPK/RCPE, RAP/RAPK
//!				msgData - RCP/RAP data
//! @return     RCP/RAP error status
//------------------------------------------------------------------------------
uint8_t SiiMhlCbRxRcpRapReceived( uint8_t cmd, uint8_t msgData )
{
    uint8_t     status = MHL_MSC_MSG_RCP_NO_ERROR;

    DEBUG_PRINT( MSG_DBG, "\nApplication layer:: SiiMhlCbRxRcpRapReceived( %02X, %02X )\n", cmd, msgData );
    switch ( cmd )
    {
        case MHL_MSC_MSG_RCP:
            DEBUG_PRINT( MSG_DBG, "RCP received by app.\n" );
            status = SkAppCbusProcessRcpMessage( msgData );
            break;
        case MHL_MSC_MSG_RCPK:
            DEBUG_PRINT( MSG_DBG, "RCPK received by app.\n" );
#if INC_RTPI
            SiiRtpiSetInt(RP_M__INTERRUPT2__CBUS_MSG_RECEIVED << 8);
#endif
#if INC_CEC
            // If this RCPK follows an RCPE, and the initial RCP
            // originated from a CEC message, send a feature abort back
            // to the source of the CEC message.
            if ( pAppCbus->lastRcpFailed )
            {
                SkAppCbusCecFeatureAbort( msgData );
                pAppCbus->lastRcpFailed = false;
            }
            else
            {
            	switch(msgData)
            	{
            	case MHL_RCP_CMD_PLAY:
                	SiiCecSendActiveSource(pAppCbus->cecLa, pAppCbus->cecPa);
            		pAppCbus->deckStatus = CEC_DECKSTATUS_PLAY;
            		break;
            	case MHL_RCP_CMD_STOP:
            		pAppCbus->deckStatus = CEC_DECKSTATUS_STOP;
            		break;
            	case MHL_RCP_CMD_PAUSE:
            		pAppCbus->deckStatus = CEC_DECKSTATUS_STILL;
            		break;
            	default:
            		pAppCbus->deckStatus = CEC_DECKSTATUS_PLAY;
            		break;
            	}
            	SiiCecSendDeckStatus(pAppCbus->cecLa, pAppCbus->cecPa, pAppCbus->deckStatus);
            }
#endif
            break;
        case MHL_MSC_MSG_RCPE:
            DEBUG_PRINT( MSG_DBG, "RCPE received by app.\n" );
#if INC_RTPI
            SiiRtpiSetInt(RP_M__INTERRUPT2__CBUS_MSG_RECEIVED << 8);
#endif

#if INC_CEC
            pAppCbus->lastRcpFailed = true;
#endif
            break;
        case MHL_MSC_MSG_RAP:
            DEBUG_PRINT( MSG_DBG, "RAP received by app.\n" );
            status = SkAppCbusProcessRapMessage( cmd, msgData );
            break;
        case MHL_MSC_MSG_RAPK:
            DEBUG_PRINT( MSG_DBG, "RAPK received by app.\n" );
            status = SkAppCbusProcessRapMessage( cmd, msgData );
           break;
        case MHL_MSC_MSG_UCP:
            DEBUG_PRINT( MSG_DBG, "UCP received.\n" );
            status = SkAppCbusProcessUcpMessage( msgData );
            break;
        case MHL_MSC_MSG_UCPK:
            DEBUG_PRINT( MSG_DBG, "UCPK received.\n" );
            break;
        case MHL_MSC_MSG_UCPE:
            DEBUG_PRINT( MSG_DBG, "UCPE received.\n" );
           break;
        default:
            break;
    }

    return( status );
}

//------------------------------------------------------------------------------
// Function:    SkAppChnEn
// Description: Enable or disable the specified CBUS channel.
// param		isEnable - true to enable, false to disable
//------------------------------------------------------------------------------
void SkAppChnEn ( bool_t isEnable )
{
	pAppCbus->mhlCableIn = isEnable;
    if ( isEnable )
    {
    	DEBUG_PRINT( MSG_STAT | DBGF_TS, "MHL Cable IN!! port: %0X\n", pAppCbus->port );
    	SiiPlatformGpioVbusCtrl(pAppCbus->port, ON);
    	SiiPlatformGpioRpwr( pAppCbus->port, OFF);
    }
    else
    {
    	DEBUG_PRINT( MSG_STAT | DBGF_TS, "MHL Cable OUT!! port: %0X\n", pAppCbus->port );
    	SiiPlatformGpioVbusCtrl(pAppCbus->port, OFF);
    	SiiPlatformGpioRpwr( pAppCbus->port, OFF);
    }

    SiiMhlRxChannelEnable(isEnable);
}

//------------------------------------------------------------------------------
// Function:    SkAppCbusIsMhlCableConnected
// Description: see if MHL cable is connected to a particular port
// param		port - port#
//------------------------------------------------------------------------------
bool_t SkAppCbusIsMhlCableConnected ( uint8_t port )
{
	uint8_t	channel;
	for ( channel = 0; channel < MHL_MAX_CHANNELS; channel++ )
	{
		if ( port == portArray[channel] )
		{
			SkAppCbusInstanceSet( channel );
			if ( SiiRegRead( REG_CBUS_BUS_STATUS ) & BIT_BUS_CONNECTED)
			{
				return( true );
			}
		}
	}
	return false;
}


//------------------------------------------------------------------------------
//! @brief:     Perform any board-level initialization required at the same
//! @brief:     time as CBUS component initialization
//! @params[in] none
// Returns:     none
//------------------------------------------------------------------------------

bool_t SkAppDeviceInitCbus ( void )
{
	uint8_t		i;

	memset( &appCbus[0], 0, sizeof(SiiAppCbusInstanceData_t) * MHL_MAX_CHANNELS);
	pAppCbus = &appCbus[0];

	for( i = 0; i < MHL_MAX_CHANNELS; i++ )
	{
		appCbus[i].port = portArray[i];
		SiiCbusInstanceSet( i );
		SiiMhlRxInitialize();
		// We are not MHL_LD_VIDEO or MHL_LD_AUDIO because we do not actually
		// source the video or audio (at least from the TMDS perspective)
		appCbus[i].supportMask  = MHL_LD_DISPLAY | MHL_LD_SPEAKER | MHL_LD_GUI;
		SiiCbusConfigure( appCbus[i].port, appCbus[i].supportMask );
	}

	return( true );
}

//------------------------------------------------------------------------------
// Function:    SkAppCbusResume
//! @brief      Restore operation to all Cbus components/drivers.
//------------------------------------------------------------------------------
bool_t SkAppCbusResume ( void )
{
    SiiCbusResume();
    return( true );
}

//------------------------------------------------------------------------------
// Function:    SkAppCbusStandby
//! @brief      Places all CBUS components/drivers into standby
// Returns:     true if the CBUS component entered standby mode successfully,
//              or false if some failure occurred.
//------------------------------------------------------------------------------

bool_t SkAppCbusStandby ( void )
{
	uint8_t 	i;

	for( i=0; i<MHL_MAX_CHANNELS; i++)
	{
		SiiPlatformGpioVbusCtrl(appCbus[i].port, OFF);
		SiiPlatformGpioRpwr(appCbus[i].port, ON);
	}
	SiiCbusStandby();

	memset(&appCbus[0], 0, sizeof(SiiAppCbusInstanceData_t) * MHL_MAX_CHANNELS);

    return( true );
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskCbusStandByMonitoring
// Description: Wrapper for the CBUS Component at the application level
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SkAppTaskCbusStandByMonitoring (void)
{
    clock_time_t timeDiff = SkTimeDiffMs( app.cbusTimeLastMs, SiiOsTimerTotalElapsed());

    if (timeDiff > 20)
    {
        // Call SkAppTaskCbusCdsenseMonitoring with the number of milliseconds since the last call
    	SkAppTaskCbusCdsenseMonitoring();
        app.cbusTimeLastMs = SiiOsTimerTotalElapsed();
    }
}

//------------------------------------------------------------------------------
// Function:    SkAppTaskCbus
// Description: Wrapper for the CBUS Component at the application level
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
void SkAppTaskCbus ( void )
{
	uint8_t i;
	uint8_t status;
	
	for (i=0; i < MHL_MAX_CHANNELS; i++)
	{
		SkAppCbusInstanceSet(i);
		//DEBUG_PRINT( MSG_DBG, "SkAppCbusChannelEnable:%d %d\n", i, pAppCbus->port );

		SkAppTaskGpio(i);

		if (pAppCbus->cbusEnable != pAppCbus->oldCbusEnable)
		{
			//DEBUG_PRINT(MSG_DBG,"\nCdSense changed pApp->cbusEnable: %02X, pApp->oldCbusEnable: %02X. %x\n", pAppCbus->cbusEnable, pAppCbus->oldCbusEnable,i );
			SkAppChnEn(pAppCbus->cbusEnable);
			pAppCbus->oldCbusEnable = pAppCbus->cbusEnable;
		}
		// Is there anything to do?
	    if (!pAppCbus->cbusEnable)
	    {
	        continue;
	    }

	    // Should we be checking the CBUS?
	    if ( !pAppCbus->cbusInterrupt && !SiiOsTimerExpired(pAppCbus->cbusTimer) )
	    {
	       continue;
	    }

		if ( pAppCbus->cbusInterrupt )
		{
			//DEBUG_PRINT( MSG_DBG, "\n\n CBus Interrupt came !! Timer Elapsed:: %02X\n\n", CbusTimerElapsed() );
			pAppCbus->cbusInterrupt = false;
		}

		SiiOsTimerSet(&(pAppCbus->cbusTimer), 10); // Bug 33045 - Melbourne SiI9535/9533 MHL QD882 HDCP CTS 3C fails at four test items

#if ( configSII_DEV_9535 == 1 )
		if ( (status = SiiMhlRxHandler()) != CBUS_SUCCESS )	// Monitor CBUS interrupts.
#else
		if ( (status = SiiMhlRxHandler()) != SUCCESS )	// Monitor CBUS interrupts.
#endif 
		{
			DEBUG_PRINT( MSG_DBG, "SiiMhlRxHandler() failed with error :: %02X\n", status );
		}
	}
}
//------------------------------------------------------------------------------
// Function:    SkAppTaskCbusCdsenseMonitoring
// Description: Wrapper for the CBUS Component at the application level
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------
void SkAppTaskCbusCdsenseMonitoring ( void )
{
	uint8_t i;
	if((app.powerState == APP_POWERSTATUS_STANDBY) || (app.powerState == APP_POWERSTATUS_INITIAL))
	{
        for (i=0; i < MHL_MAX_CHANNELS; i++)
        {
            SkAppCbusInstanceSet(i);
            SkAppTaskCbusGpio(i);
            if (( (pAppCbus->cdSense == 1) && (pAppCbus->oldcdSense != pAppCbus->cdSense)) /*&& (i == app.currentSource[i])*/ )
            {
                pAppCbus->oldcdSense = pAppCbus->cdSense;
                app.mhlcabledetect = true;
                SiiPlatformTimerWait( 1000 );
            }
        }
	}
}
