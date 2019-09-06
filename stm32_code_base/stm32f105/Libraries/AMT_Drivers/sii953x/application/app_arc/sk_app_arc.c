//***************************************************************************
//!file     sk_app_arc.c
//!brief    Wraps board and device functions for the ARC component
//!         and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_drv_arc.h"
#include "si_drv_switch.h"
#include "si_gpio.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "Debug.h"
#endif


#if ( configSII_DEV_953x_PORTING == 1 )
uint16_t SYS_CEC_TASK_ARC;
extern HMI_SERVICE_OBJECT *pHS_ObjCtrl; /*HMI service*/

static uint8_t CEC_ARC_TxLA = 0x00;	//CEC_LOGADDR_TV
static uint8_t ArcTaskCounter = 0xFF;

extern void AmTArcAppTaskAssign( AmTArcTaskEvent event);
#endif
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#if INC_CEC
static char unableToRegisterArcMsgHandler [] = "Unable to register ARC CEC msg handler\n";

//------------------------------------------------------------------------------
// Function:    ArcSendResponse
// Description: Send the passed ARC message if the target is adjacent. If the
//              target was NOT adjacent, an ABORT-INVALID message is sent instead.
//
// Returns:     TRUE if message was sent, FALSE if not adjacent
//------------------------------------------------------------------------------
static bool_t ArcSendResponse ( uint8_t opCode, int portIndex, int destLA, bool_t isArcTx )
{
    bool_t  success;
    xHMISystemParams SysPrams;

    for ( ;; )
    {
        success = false;
        if (!SiiCecIsAdjacentLA( destLA ))
        {
            SiiCecFeatureAbortSend( opCode, CECAR_INVALID_OPERAND, destLA );
            DEBUG_PRINT( MSG_STAT, "ARC Request from non-adjacent device\n" );
            break;
        }

#if 0//( configSII_DEV_953x_PORTING == 1 )
	if ( opCode == CECOP_REQUEST_ARC_INITIATION )
	{
		AmTUpdateARCTxLA(destLA);
		AmTArcAppTaskAssign(ARC_TASK_INIT_REQUESTED);
	}
	else
	{
		AmTUpdateARCTxLA(destLA);
		AmTArcAppTaskAssign(ARC_TASK_TERM_REQUESTED);
	}
#else
		//All is well, send the response
        if(SiiCecSendMessageTask( opCode, destLA ))
        {
            // if add to task fail, just force to send, kaomin
            SiiCecSendMessageEx(opCode, pCec->logicalAddr, destLA);
        }
#endif
        success = true;
        break;
    }
    return( success );
}

#if (1)//( configSII_DEV_953x_PORTING == 0 )
//------------------------------------------------------------------------------
//!  @brief     ARC message Handler for a HDMI RX device (ARC TX)
//!  @return    True if message was processed by this handler
//------------------------------------------------------------------------------
static bool_t SkAppArcTxCecMsgHandler ( SiiCpiData_t *pMsg )
{
    int     sourcePortIndex;
    bool_t  processedMsg, isDirectAddressed;
    uint8_t sourceLA;

    isDirectAddressed   = (bool_t)!((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );
    sourceLA            = (pMsg->srcDestAddr >> 4) & 0xF;
    sourcePortIndex     = SiiCecLaToPort( sourceLA);

    processedMsg = true;
    switch ( pMsg->opcode )
    {
        case CECOP_INITIATE_ARC:                                        // HDMI RX (ARC TX) Only
            if ( !isDirectAddressed )   break;                          // Ignore as broadcast message

            // An ARC RX device (HDMI TX) asked us to activate our ARC TX device (HDMI RX).
            DEBUG_PRINT( MSG_STAT, "ARC Initiate from HDMI RX\n" );
            if ( ArcSendResponse( CECOP_REPORT_ARC_INITIATED, sourcePortIndex, sourceLA, true ) )
            {
                // Assumptions for the SiI9535 Starter Kit:
                // RX port 1 ARC pin is connected to 9535 ARCRX0_TX0 and S/PDIF 0 is the 9535 input pin.
                SiiDrvArcConfigureArcPin( sourcePortIndex - 1, sourcePortIndex - 1, ARC_TX_MODE );
                SiiPlatformGpioLedArc( true );
                DEBUG_PRINT( MSG_STAT, "ARC Initiate report sent\n" );
            }
            break;

        case CECOP_TERMINATE_ARC:                                       // HDMI RX (ARC TX) Only
            if ( !isDirectAddressed )   break;                          // Ignore as broadcast message

            // An ARC RX device (HDMI TX) asked us to deactivate our ARC TX device (HDMI RX).
            DEBUG_PRINT( MSG_STAT, "ARC Terminate from RX\n" );
            if ( ArcSendResponse( CECOP_REPORT_ARC_TERMINATED, sourcePortIndex, sourceLA, true ))
            {
                // Assumptions for the SiI9535 Starter Kit:
                // RX port 1 ARC pin is connected to 9535 ARCRX0_TX0 and S/PDIF 0 is the 9535 input pin.
                SiiDrvArcConfigureArcPin( sourcePortIndex - 1, sourcePortIndex - 1, ARC_DISABLE_MODE );
                SiiPlatformGpioLedArc( false );
                DEBUG_PRINT( MSG_STAT, "ARC Terminate report sent\n" );
            }
            break;


        default:
            processedMsg = false;
            break;
    }

    return( processedMsg );
}
#endif

//------------------------------------------------------------------------------
//!  @brief     ARC message Handler for a HDMI TX device (ARC RX)
//!  @return    True if message was processed by this handler
//------------------------------------------------------------------------------
static bool_t SkAppArcRxCecMsgHandler ( SiiCpiData_t *pMsg )
{
    int     sourcePortIndex;
    bool_t  processedMsg, isDirectAddressed;
    uint8_t sourceLA;
#if ( configSII_DEV_953x_PORTING == 1 )
    xHMISystemParams SystemInfo;
#endif

    isDirectAddressed   = (bool_t)!((pMsg->srcDestAddr & 0x0F ) == CEC_LOGADDR_UNREGORBC );
    sourceLA            = (pMsg->srcDestAddr >> 4) & 0xF;
    sourcePortIndex     = SiiCecLaToPort( sourceLA);

    processedMsg = true;
    switch ( pMsg->opcode )
    {
    	case CECOP_REPORT_ARC_INITIATED:                                        // HDMI TX (ARC RX) Only
    		if ( !isDirectAddressed )  break;

            if ( AmTCecGetSystemPowerStatus() == POWER_OFF )
            {
                AmTSacAppTaskAssign ( SAC_TASK_TERMINATE_POWER_OFF );
                AmTArcAppTaskAssign(ARC_TASK_TERM_PRIVATELY); // CECT 11.2.17-2
            }
            else
            {
                // An ARC TX device (HDMI RX) has indicated that its ARC TX functionality
                // has been activated, so enable our ARC receiver.  The port index we are referring to
                // here is the local ARC port, in this case one of the TX ports.
                // Assumptions for the SiI9535 Starter Kit:
        		// TX0 ARC pin is connected to 9535 ARCRX0_TX0 and S/PDIF 0 is the 9535 output pin.
    #if (0)//( configSII_DEV_953x_PORTING == 1 )
    	    AmTArcAppTaskAssign(ARC_TASK_REPORT_INIT);
    #else
        	    //SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_RX_MODE );
                SiiPlatformGpioLedArc( true );
#if 0
                SystemInfo = pHS_ObjCtrl->GetSystemParams();

                if ((SystemInfo.input_src == AUDIO_SOURCE_HDMI_ARC)) 
                {
                    //TRACE_DEBUG(( 0, ( "CECOP_REPORT_ARC_INITIATED" )));
                    SiiDrvArcConfigureArcPin( sourcePortIndex - 1, sourcePortIndex - 1, ARC_RX_MODE );
                }
#endif                
    #endif
                // to sync SAC status with TV in time, kaomin
                CecSacSetStatus(TRUE);
                CecSacGetReportArcInitiate(); // info sac polling task, kaomin
                
                AmTSacSetDestLa(sourceLA);
                DEBUG_PRINT( MSG_STAT, ( "ARC initiated by sink device\n" ));
            }
            break;

    	case CECOP_REPORT_ARC_TERMINATED:                                       // HDMI TX (ARC RX) Only
    		if ( !isDirectAddressed )  break;

            // An ARC TX device (HDMI RX) has indicated that its ARC TX functionality
            // has been deactivated, so disable our receiver.  The port index we are referring to
            // here is the local ARC port, in this case one of the TX ports.
            // Assumptions for the SiI9535 Starter Kit:
            // TX0 ARC pin is connected to 9535 ARCRX0_TX0 and S/PDIF 0 is the 9535 output pin.
#if (0)//( configSII_DEV_953x_PORTING == 1 )
	    AmTArcAppTaskAssign(ARC_TASK_REPORT_TERM);
#else
            //SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_DISABLE_MODE );
            SiiPlatformGpioLedArc( false );
            // RX port 1 ARC pin is connected to 9535 ARCRX0_TX0 and S/PDIF 0 is the 9535 input pin.
            //SiiDrvArcConfigureArcPin( sourcePortIndex - 1, sourcePortIndex - 1, ARC_DISABLE_MODE );
#endif
            // to sync SAC status with TV, kaomin
            CecSacSetStatus(FALSE);
            DEBUG_PRINT( MSG_STAT, ( "ARC terminated by sink device\n" ));
            break;

    	case CECOP_REQUEST_ARC_INITIATION:                                      // HDMI TX (ARC RX) Only
            if ( !isDirectAddressed )  break;

            // An ARC TX device (HDMI RX) has requested that we send
            // an INITIATE_ARC message back to it.  The port index we are referring to
            // here is the local ARC port, in this case one of the TX ARC ports.
            //if ( ArcSendResponse( CECOP_REQUEST_ARC_INITIATION, SiiCecInstanceGet(), sourceLA, false ))
#if ( configSII_DEV_953x_PORTING == 0 )
            SystemInfo = pHS_ObjCtrl->GetSystemParams();
            //TRACE_DEBUG((0,"Init ARC input source ------%d",SystemInfo.input_src));
            if (SystemInfo.input_src != AUDIO_SOURCE_HDMI_ARC)
            {
                //ArcSendResponse( CECOP_FEATURE_ABORT, SiiCecInstanceGet(), sourceLA, false );
                //ArcSendResponse( CECOP_TERMINATE_ARC, SiiCecInstanceGet(), sourceLA, false );
                SiiCecFeatureAbortSend(CECOP_REQUEST_ARC_INITIATION, CECAR_REFUSED, sourceLA);
            }
            else 
            {
#endif
            if ( ArcSendResponse( CECOP_INITIATE_ARC, SiiCecInstanceGet(), sourceLA, false ))
            {
#if ( configSII_DEV_953x_PORTING == 1 )
                SystemInfo = pHS_ObjCtrl->GetSystemParams();
                //TRACE_DEBUG((0,"Init ARC input source ------%d",SystemInfo.input_src));
                if ((SystemInfo.input_src == AUDIO_SOURCE_HDMI_ARC) && (AmTCecGetSystemPowerStatus() == POWER_ON))
		    break;
#endif
                // just power on the device, kaomin
                if(AmTCecGetSystemPowerStatus() == POWER_OFF)
                {
                    AmTCecEventPassToHMI(USER_EVENT_SAC_WAKE_UP);	//Power SB on and change to HDMI_ARC
                    //TRACE_DEBUG((0,"Sent Initiate ARC request to source device --------------"));
                }
                //SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_RX_MODE );
                DEBUG_PRINT( MSG_STAT, ( "Sent Initiate ARC request to source device\n" ));
            }
#if ( configSII_DEV_953x_PORTING == 0 )            
            }            
#endif            
    	    break;
    	case CECOP_REQUEST_ARC_TERMINATION:                                     // HDMI TX (ARC RX) Only
            if ( !isDirectAddressed )  break;

            // An ARC TX device (HDMI RX) has requested that we send a TERMINATE_ARC
            // message back to it.  The port index we are referring to here is
            // the local ARC port, in this case one of the TX ARC ports.
            //if ( ArcSendResponse( CECOP_REQUEST_ARC_TERMINATION, SiiCecInstanceGet(), sourceLA, false ))
#if ( configSII_DEV_953x_PORTING == 0 )
            SystemInfo = pHS_ObjCtrl->GetSystemParams();
            TRACE_DEBUG((0,"termination ARC input source ------%d",SystemInfo.input_src));
            if (SystemInfo.input_src == AUDIO_SOURCE_HDMI_IN)
            {
                //ArcSendResponse( CECOP_FEATURE_ABORT, SiiCecInstanceGet(), sourceLA, false );
                SiiCecFeatureAbortSend(CECOP_REQUEST_ARC_TERMINATION, CECAR_REFUSED, sourceLA);
            }
            else 
            {
#endif
            if ( ArcSendResponse( CECOP_TERMINATE_ARC, SiiCecInstanceGet(), sourceLA, false ))
            {
#if ( configSII_DEV_953x_PORTING == 0 )
                // smith add
                SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_DISABLE_MODE );
#endif
                DEBUG_PRINT( MSG_STAT, ( "Sent Terminate ARC request to source device\n" ));
            }
#if ( configSII_DEV_953x_PORTING == 0 )            
            }            
#endif            
            break;

	case CECOP_FEATURE_ABORT:
	    {
                if ( !isDirectAddressed )  break;
				
		if ( (pMsg->args[0] == CECOP_INITIATE_ARC) && ((pMsg->srcDestAddr & 0x0F ) == pCec->logicalAddr) ) 
		{
		        AmTUpdateARCTxLA(pMsg->srcDestAddr >> 4);
		        AmTArcAppTaskAssign(ARC_TASK_TERM_REQUESTED);
		}
		else
            		processedMsg = false;
	    }
	    break;
       default:
            processedMsg = false;
            break;
    }

    return( processedMsg );
}
#endif

//------------------------------------------------------------------------------
// Function:    SkAppDeviceInitArc
// Description: Perform any board-level initialization required at the same
//              time as ARC component initialization
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

bool_t SkAppDeviceInitArc ( void )
{
    bool_t  success = true;
	
    do {
        if ( !SiiDrvArcInitialize())
        {
            success = false;
            break;
        }
	
#if INC_CEC
        // If CEC is enabled (and it should be to use ARC), register our RX parser
        // with the CEC message handler for the CEC RX instance.  Note that
        // the HDMI RX instance is an ARC Tx.
        if ( app.cecEnable )
        {
            SkAppCecInstanceSet( CEC_INSTANCE_AVR );
#if (1)//( configSII_DEV_953x_PORTING == 0 )
            if ( !SiiCecCallbackRegisterParser( SkAppArcTxCecMsgHandler, false ) ||
                 !SiiCecCallbackRegisterParser( SkAppArcRxCecMsgHandler, false ) )
#else
            if ( !SiiCecCallbackRegisterParser( SkAppArcRxCecMsgHandler, false ) )	// Only support ARC Rx for our 2015 main stream project
#endif
            {
                DEBUG_PRINT( MSG_ERR, unableToRegisterArcMsgHandler );
                success = false;
                break;
            }
			
#if (SII_NUM_ARC > 1)
            /******************* NEED TO Register CEC TX       *************************/
            /******************* if ARC_RX is configured in arcCfg *************************/
            SkAppCecInstanceSet( CEC_INSTANCE_AUXTX );
            if ( !SiiCecCallbackRegisterParser( SkAppArcRxCecMsgHandler, false ))
            {
                DEBUG_PRINT( MSG_ERR, unableToRegisterArcMsgHandler );
                success = false;
                break;
            }
#endif
        }
#endif
    } while (0);  // Do the above only once.

    return( success );
}

#if ( configSII_DEV_953x_PORTING == 1 )
//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
void AmTCecArcTask( void )
{
    xHMISystemParams SysPrams;

    SysPrams = AmTCecGetSyspramsFromHMI();

    // Ignore CEC message under demo mode
    if ( (SysPrams.op_mode != MODE_USER) && (SysPrams.op_mode != MODE_FACTORY) )
    {
        return;
    }

#if 0
    // block <IARC> for 10 sec to prevent TV LA loss, kaomin
    if ( SYS_CEC_TASK_ARC & SYS_CEC_TASK_ARC_INITIONAL_LATE )
    {
        static uint16_t late_cnt = 0; 
        
        late_cnt++;
        if(late_cnt >= 100) // 2.5 sec late after hpd assert, kaomin
        {
            SYS_CEC_TASK_ARC &= ~SYS_CEC_TASK_ARC_INITIONAL_LATE;
            SYS_CEC_TASK_ARC |= SYS_CEC_TASK_ARC_INITIONAL;
            late_cnt = 0;
        }
        else
        {
            SYS_CEC_TASK_ARC &= ~SYS_CEC_TASK_ARC_INITIONAL;
        }
    }
#endif
    
     if ( SYS_CEC_TASK_ARC & SYS_CEC_TASK_ARC_INITIONAL )
    {	
	//CecInitialARCSend();
	AmTARCMessageSend(TRUE);
	ArcTaskCounter = 1;
	
	SYS_CEC_TASK_ARC &= ~SYS_CEC_TASK_ARC_INITIONAL;
    }
    else if ( SYS_CEC_TASK_ARC & SYS_CEC_TASK_ARC_TERMINATE )
    {
	//CecTerminalARCSend();
	AmTARCMessageSend(FALSE);
	ArcTaskCounter = 1;
		
	SYS_CEC_TASK_ARC &= ~SYS_CEC_TASK_ARC_TERMINATE;
    }
    else if ( SYS_CEC_TASK_ARC & SYS_CEC_TASK_ARC_REPORT_INITIONAL )
    {
	SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_RX_MODE );
	ArcTaskCounter = 0;
	
	SYS_CEC_TASK_ARC &= ~SYS_CEC_TASK_ARC_REPORT_INITIONAL;
    }
    else if ( SYS_CEC_TASK_ARC & SYS_CEC_TASK_ARC_REPORT_TERMINATE )
    {
	SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_DISABLE_MODE );
	ArcTaskCounter = -1;
	
	SYS_CEC_TASK_ARC &= ~SYS_CEC_TASK_ARC_REPORT_TERMINATE;
    }
#if 0
    else if ( ArcTaskCounter >= 13 )	//timeout for receive report message 
    {
            xHMISystemParams SysPrams;

            SysPrams = AmTCecGetSyspramsFromHMI();
	
            if ( SysPrams.cec_arc )
            {
	            AmTCecEventPassToHMI(USER_EVENT_CEC_SET_ARC_OFF);
            }
	    ArcTaskCounter = -1;
    }

    if ( ArcTaskCounter > 0 )
	ArcTaskCounter++;
#endif
}

//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
void AmTArcTaskAssign( uint16_t SysArcAppTask)
{
	SYS_CEC_TASK_ARC |= SysArcAppTask;	
	/*if ( SysArcAppTask )
	{
            TRACE_DEBUG((0, "SYS_CEC_TASK_ARC %X", SYS_CEC_TASK_ARC ));
	}*/
}

//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
#if 0
void AmTArcRxModeCtrl( bool_t ArcOn )
{
    if (ArcOn == TRUE)
    {
        SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_RX_MODE );
    }
    else if (ArcOn == FALSE)
    {
        SiiDrvArcConfigureArcPin( SiiCecInstanceGet(), SiiCecInstanceGet(), ARC_DISABLE_MODE );
        TRACE_DEBUG(( 0, ( "AmTArcRxModeCtrl\n" )));
    }       
}
#endif
//-------------------------------------------------------------------------------------------------
//! @brief      
//-------------------------------------------------------------------------------------------------
bool_t AmTIPowerOnSystem( void )
{
	POWER_STATE SysPower;
	bool_t NeedSetArcTask = FALSE;

	SysPower = AmTCecGetSystemPowerStatus();
	if ( SysPower != POWER_ON && SysPower != POWER_UP )
	{
		AmTCecEventPassToHMI(USER_EVENT_POWER);		//use this event for power on hdmi when powering system up
		NeedSetArcTask = TRUE;	//return false if need power on system
	}

	return NeedSetArcTask;
}

static void AmTARCMessageSend( bool_t MsgType )
{
    SiiCpiData_t cecFrame;
	
    if ( MsgType )
    {
        SiiCecCpiWrite(CECOP_INITIATE_ARC, 0, CEC_ARC_TxLA, &cecFrame);
    }
    else
    {
        SiiCecCpiWrite(CECOP_TERMINATE_ARC, 0, CEC_ARC_TxLA, &cecFrame);
    }
}

static void AmTUpdateARCTxLA ( uint16 ARCTxLA)
{
	xHMISystemParams SysPrams;

        //Update ARC Tx LA to HMI
	SysPrams = AmTCecGetSyspramsFromHMI();
        SysPrams.cec_arc_tx = ARCTxLA;
	pHS_ObjCtrl->SetSystemParams(&SysPrams);

        //Update ARC Tx LA to global patams
        CEC_ARC_TxLA = ARCTxLA;
}


#endif
