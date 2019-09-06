//***************************************************************************
//!file     sk_app_arc_demo.c
//!brief    Built-in Demo manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_timer.h"

#include "sk_application.h"
#include "si_drv_arc.h"

#if INC_ARC

//------------------------------------------------------------------------------
// Function:    DemoArc
// Description: ARC/HEC demo command options
// Returns:
//------------------------------------------------------------------------------
static char *pArcDemoMenu0 [] =
{
    "\n ARC Demo",
    " --------",
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    NULL
};

static char *pArcDemoMenu1 [] =
{
    "3 - Disable ARC",
//    "4 - Send ARC_REQUEST_INITIATE CEC msg to ARC port (HDMI source)",
//    "5 - Send ARC_REQUEST_TERMINATE CEC msg to ARC port (HDMI source)",
//    "6 - Send ARC_INITIATE CEC msg to ARC port (HDMI sink)",
//    "7 - Send ARC_TERMINATE CEC msg to ARC port (HDMI sink)",
    NULL
};
#endif
bool_t SkAppArcDemo ( RC5Commands_t key )
{
    // Start with ARC disabled for some of these menu selections
    if (( key >= RC5_1 ) && ( key <= RC5_5 ))
    {
        if ( app.arcDemoEnabled )
        {
            SiiDrvArcConfigureArcPin( 0, 0, ARC_DISABLE_MODE);
            app.arcDemoEnabled = false;
        }
    }

#if INC_ARC
    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
        	PrintMenu( pArcDemoMenu0 );
            PrintStringOnOff( "1 - ARC TX ", (bool_t)(app.arcDemoEnabled && (app.arcDemoSelect == 0)));
            PrintStringOnOff( "2 - ARC RX ", (bool_t)(app.arcDemoEnabled && (app.arcDemoSelect == 1)));
            PrintMenu( pArcDemoMenu1 );
            break;

        case RC5_0:
            app.demoMenuLevel = 0;
            PrintAlways( demoMenuExit );
            break;

        case RC5_1:
            app.arcDemoSelect = 0;
            app.arcDemoEnabled = true;
            SiiDrvArcConfigureArcPin(0, 0, ARC_TX_MODE);
            PrintAlways( "SPDIF In -> ARC TX\n");
            break;

        case RC5_2:
            app.arcDemoSelect = 1;
            app.arcDemoEnabled = true;
            SiiDrvArcConfigureArcPin(0, 0, ARC_RX_MODE);
            PrintAlways( "ARC RX -> SPDIF Out\n");
            break;

        case RC5_3: // Disable actually happens before switch statement...
            PrintAlways( "ARC Disabled\n");
            break;

//        case RC5_4:
//            //DEBUG_PRINT( MSG_ALWAYS, " ARC TX Port #(0-1): \n" );
//            //digit = SkAppIrGetSingleDigit(1);
//        	digit = 0;
//
//            SkAppCecInstanceSet( CEC_INSTANCE_AVR );
//            digit = SiiCecPortToAdjacentLA( l_arcTxPortAssignments[digit] );
//            if (digit <= CEC_LOGADDR_UNREGORBC)
//            {
//                SiiCecSendMessageTask(CECOP_REQUEST_ARC_INITIATION, digit );
//                DEBUG_PRINT( MSG_STAT, "ARC Request Initiation msg sent\n" );
//            }
//            else
//            {
//                DEBUG_PRINT( MSG_STAT, adjDeviceNotFoundStr );
//            }
//            break;
//
//        case RC5_5:
////            DEBUG_PRINT( MSG_ALWAYS, " ARC TX Port number (0-1): \n" );
////            digit = SkAppIrGetSingleDigit( 1 );
//        	digit = 0;
//
//            SkAppCecInstanceSet(CEC_INSTANCE_AVR);
//            digit = SiiCecPortToAdjacentLA( l_arcTxPortAssignments[digit] );
//            if (digit <= CEC_LOGADDR_UNREGORBC)
//            {
//                SiiCecSendMessageTask(CECOP_REQUEST_ARC_TERMINATION, digit);
//                DEBUG_PRINT( MSG_STAT, "ARC Request Termination msg sent\n" );
//            }
//            else
//            {
//                DEBUG_PRINT( MSG_STAT, adjDeviceNotFoundStr );
//            }
//            break;
//
//        case RC5_6:
////            DEBUG_PRINT( MSG_ALWAYS, " ARC RX Port number (0-1): \n" );
////            digit = SkAppIrGetSingleDigit( 1 );
//        	digit = 0;
//
//            SkAppCecInstanceSet( l_arcRxInstanceAssignments[digit] );
//            // This assumes that the ARC RX is connected to a TV with ARC support.
//            SiiCecSendMessageTask( CECOP_INITIATE_ARC, CEC_LOGADDR_TV );
//            DEBUG_PRINT( MSG_STAT, "ARC Initiate command sent\n" );
//            break;
//
//        case RC5_7:
////            DEBUG_PRINT( MSG_ALWAYS, " ARC RX Port number (0-1): \n" );
////            digit = SkAppIrGetSingleDigit( 1 );
//        	digit = 0;
//
//            SkAppCecInstanceSet( l_arcRxInstanceAssignments[digit] );
//            // This assumes that the ARC RX is connected to a TV with ARC support.
//            SiiCecSendMessageTask( CECOP_TERMINATE_ARC, CEC_LOGADDR_TV );
//            DEBUG_PRINT( MSG_STAT, "ARC Terminate command sent\n" );
//            break;

        default:
            break;
    }
#endif
    return( false );
}

