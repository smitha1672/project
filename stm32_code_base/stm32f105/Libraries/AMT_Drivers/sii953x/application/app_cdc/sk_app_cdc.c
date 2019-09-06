//***************************************************************************
//!file     sk_app_cdc.c
//!brief    Application and demo functions for the CDC component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "device_config.h"
#include "si_device_config.h"

#if INC_CDC
#include "si_timer.h"
#include "sk_application.h"

#include "si_cdc_component.h"

//------------------------------------------------------------------------------
// Module variables
//------------------------------------------------------------------------------

static CdcIoConfig_t cdcIo;

static int l_devPhysAddr    = 0x0000;

static uint8_t lastFbMsg    = CDC_FB_MSG_NONE;

//------------------------------------------------------------------------------
// Function:    SkAppCdcMsgHandler
// Description: Monitor CDC sub-system.  Called from the CEC handler
//------------------------------------------------------------------------------

bool_t SkAppCdcMsgHandler ( SiiCpiData_t *pMsg )
{
    bool_t  usedMessage;
    uint8_t fbMsg;
    int i;
    uint16_t timeTickMs;
    CdcFoundListEntry_t *foundDevList;

    // Bring RX CEC into current CPI context. HEC is available on SiI9489 RX ports only.
    // Note: Not using of SiiCdcInstanceSet() here is essential: CEC component has bigger set of
    // options (CEC RX & CEC TX), so the CDC can use either of them still being instantiated
    // only once.
    SkAppCecInstanceSet( CEC_INSTANCE_AVR );

    // Call the main CDC handler. We pass it the system tick timer's value
    // so that it can update its internal timers.

    timeTickMs = SiiPlatformTimerElapsed( ELAPSED_TIMER1 );
    fbMsg = SiiCdcTaskProcess( timeTickMs, pMsg );

    /* Process feedback messages.       */

    usedMessage = true;
    switch ( fbMsg )
    {
        default:
        case CDC_FB_MSG_NONE:
            // nothing to do
            break;
        case CDC_FB_MSG_HST_SEARCH_DONE:    // Host search done, print list of found devices.
            foundDevList =  SiiCdcFoundHostListGet();
            for (i = 0; i < SiiCdcNumberOfFoundHostsGet(); i++)
            {
                DEBUG_PRINT( MSG_STAT, "%04x", foundDevList[i].physAddr );
                if ( foundDevList[i].hostInfo.isSelected )
                {
                    DEBUG_PRINT( MSG_STAT, " +" );
                }
                DEBUG_PRINT( MSG_STAT, "\n" );
            }
            lastFbMsg = fbMsg;
            break;
        case CDC_FB_MSG_CONNECT_DONE:       // Update lists
            DEBUG_PRINT( MSG_STAT, "Connection task finished\n" );
            lastFbMsg = fbMsg;
            break;
        case CDC_FB_MSG_DISCONNECT_DONE:
        case CDC_FB_MSG_DISCONNECT_ALL_DONE:
        case CDC_FB_MSG_DISCONNECT_LOST_DONE:
            DEBUG_PRINT( MSG_STAT, "Disconnect task finished\n" );
            break;
        case CDC_FB_MSG_HPD_SIGNAL_DONE:
            DEBUG_PRINT( MSG_STAT, "HPD Signal task finished\n" );
            break;
        case CDC_FB_MSG_ERR_HPD_SIGNAL:
            DEBUG_PRINT( MSG_STAT, "HDMI Source not responded to HPD command!\n" );
            break;
        case CDC_FB_MSG_HPD_STATE_CHANGED: // message from Sink to Source
            break;
        case CDC_FB_MSG_ERR_NONCDC_CMD:
            usedMessage = false;
            break;
    }

    return( usedMessage );
}


//------------------------------------------------------------------------------
// Function:    SkAppInitCdc
// Description: Perform any board-level initialization required at the same
//              time as CDC component initialization
// Parameters:  none
// Returns:     success or failure
//
// API NOTE:    CEC MUST be initialized prior to calling this function.
//------------------------------------------------------------------------------

bool_t SkAppInitCdc ( void )
{
    int i;

    if ( app.cecEnable == false )
    {
        return( true ); // Don't fail, fake success
    }

    // Register the CDC message handler with the CEC component

    SkAppCecInstanceSet( CDC_INSTANCE_RX );
    if ( !SiiCecCallbackRegisterParser( SkAppCdcMsgHandler, true ))
    {
        DEBUG_PRINT( MSG_ERR, "Unable to register CDC message handler\n");
    }

#if (IS_TX == 1)
    l_devPhysAddr = 0x3000; // manual setting of PA is for CTS test purposes only
#else
    l_devPhysAddr = SiiCecGetDevicePA();
#endif

    cdcIo.numberOfInputs = SII_INPUT_PORT_COUNT;
    for (i = 0; i < cdcIo.numberOfInputs; i++)
    {
        cdcIo.input[i].hecSupport = CDC_HEC_NOT_SUPPORTED;
        cdcIo.input[i].hecActive = CDC_HEC_INACTIVE;
        cdcIo.input[i].hpdState = CDC_HPD_CP_AND_EDID_ENABLE;
    }

    /* The starter kit uses port 1 for HEC. */

    cdcIo.input[SK_HEC_PORT].hecSupport = CDC_HEC_SUPPORTED;

    cdcIo.numberOfOutputs = (l_devPhysAddr == 0x0000) ? 0 : 1;
    cdcIo.output.hecSupport = CDC_HEC_SUPPORTED;                // If TX has HEC: CDC_HEC_SUPPORTED;
    cdcIo.output.hecActive = CDC_HEC_INACTIVE;
    cdcIo.output.hpdState = CDC_HPD_CP_AND_EDID_ENABLE;

    SiiCdcConfig( l_devPhysAddr, &cdcIo, true, true, false);    // Device is HEC, Host capable and External network incapable

    SiiCdcTaskInit();
    SiiCdcSrvStart();

    SiiPlatformTimerSet( ELAPSED_TIMER1, 1 ); // sets CDC timer's granularity and kicks the timer off to free running

    return( true );
}
#else
char cdcApp;    // Avoids compiler warning of empty translation unit
#endif          // #if INC_APP_CDC == 1
