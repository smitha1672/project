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

#include "sk_app_cec.h"
#include "si_edid_tx_component.h"


//------------------------------------------------------------------------------
// Function:    DemoCbus
// Description: CBUS demo command options
// Returns:
//------------------------------------------------------------------------------

static char *pCecDemoMenu [] =
{
    "\nCEC Demo",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    "1 - One Touch Play",
    NULL
};

void CecConfDeviceType (SiiCecDeviceTypes_t devicetype, SiiCecLogicalAddresses_t deviceLogicalAddr, bool_t isCecAsSwitch, bool_t isAudioSysWithSac)
{
    uint16_t 	cecPa;
    SkAppCecInstanceSet( CEC_INSTANCE_AVR );
    SiiEdidTxInstanceSet(0);
    SkAppTxInstanceSet(0);
    // Can't initialize if valid physical address not assigned
    cecPa = SiiEdidTxCecPhysAddrGet();
    if (cecPa == 0xFFFF || !SiiCecInitialize( cecPa, devicetype ))
    {
        PrintAlways( "\nERROR: CEC device type could not be set \n" );
        return;
    }
    SiiCecSetDeviceLA(deviceLogicalAddr);
#if INC_CEC_SWITCH
    if(isCecAsSwitch)
    {
       	// Initialize CEC Switch component and enable its base service
        SiiCecSwitchConfig();
        if ( !SiiCecCallbackRegisterParser( SiiCecSwitchTaskProcess, false ))
        {
            PrintAlways( "\nERROR: CEC switch task registering failed \n" );
            return;
        }
        if ( SiiCecSwitchSrvStart())
        {
            PrintAlways( "\nERROR: Launching CEC/Switch Active and Base Services \n" );
        }
    }
#endif
#if INC_CEC_SAC
    if(isAudioSysWithSac)
    {
        // Initialize CEC System Audio Control component and enable its base service
        // Can't initialize SAC if valid physical address not assigned
#if ( configSII_DEV_953x_PORTING == 1 )
        SiiCecSacConfig(0); // take default format list
#else
        SiiCecSacConfig(0, NULL, 0); // take default format list
#endif
        if ( !SiiCecCallbackRegisterParser( SiiCecSacTaskProcess, false ))
        {
            PrintAlways( "\nERROR: CEC SAC task registering failed \n" );
            return;
        }
        if ( SiiCecSacSrvStart())
        {
            PrintAlways( "\nERROR: Launching CEC/SAC Active and Base Services \n" );
        }
    }
#endif
}

bool_t SkAppCecDemo ( RC5Commands_t key )
{

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pCecDemoMenu );
            break;

        case RC5_0:
            app.demoMenuLevel = 0;
            PrintAlways( demoMenuExit );
            break;

        case RC5_1:
            PrintAlways( "\n\nSENDING ONETOUCH PLAY COMMAND \n" );
            app.txSendOneTouch = true;
            break;
        default:
            break;
    }

    return( false );
}

