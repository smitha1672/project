//***************************************************************************
//!file     sk_app_diag.c
//!brief    Built-in diagnostics manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if INC_DIAG

#include "sk_application.h"
#include "si_drv_internal.h"
#include "si_regs_rxedid953x.h"

static char *termTypeMsgs [] =
{
    "50 ohm (default)",
    "50 ohm Data, 100 ohm Clock",
    "MHL mode termination",
    "---",
};


//-------------------------------------------------------------------------------------------------
//! @brief      Display major state indications of the Switch
//!
//-------------------------------------------------------------------------------------------------

static void DumpSwitchState ( void )
{
    int_t       i;
    bool_t      first = true;
    uint8_t     uData, uData1, mpPortBitMask, portIndex;
    uint16_t    uData16;
    uint8_t     pwr5vStatus, hDdc, eDdc, ckdtStatus;
    uint16_t    termCtrl, bcaps;
    // Overall state

    PrintAlways( "\n---- Switch (Rx) State ----\n" );
    uData = SiiRegRead( REG_MP_STATE );
    DEBUG_PRINT( MSG_ALWAYS, " PWD Power: %s\n", (uData & BIT_PWRON_STANDBY) ? "ON" : "STANDBY" );
    uData = SiiRegRead( REG_TMDST_CTRL1 );
    DEBUG_PRINT( MSG_ALWAYS, "       Tx0: %s (%02X)\n",
        ((uData & (MSK_TMDS_OE_ALL | MSK_TMDS_EN)) == (MSK_TMDS_OE_ALL | MSK_TMDS_EN)) ? "Enabled" : "---", uData );
    uData = SiiRegRead( REG_TMDST_CTRL3 );
    DEBUG_PRINT( MSG_ALWAYS, "  Tx0 Term: %s (%02X)\n",
        ((uData & MSK_TMDS_TERM_EN_ALL) == MSK_TMDS_TERM_EN_ALL) ? "Enabled" : "---", uData );
    uData = SiiRegRead( REG_MISC_CTRL1 );
    DEBUG_PRINT( MSG_ALWAYS, "  Port Change: %s (%02X)\n", (uData & BIT_PORT_CHG_ENABLE) ? "Enabled" : "Disabled", uData );
    PrintAlways( "\n" );
    uData = (SiiRegRead( REG_PHYS_HPD_DISABLE ) >> 2) & 0x1F;
    PrintAlways( "HEC/MHL ports: " );
    for ( i = 0; i < SII_INPUT_PORT_COUNT; i++ )
    {
        if ( uData & ( 1 << i))
        {
            if ( !first )
            {
                PrintAlways( ", " );
            }
            DEBUG_PRINT( MSG_ALWAYS, "%d", i );
            first = false;
        }
    }
    DEBUG_PRINT( MSG_ALWAYS, " (Bits: 0x%02X)\n\n", uData );

    // Main Pipe State

    PrintAlways( "----     Main Pipe     ----\n" );
    uData = SiiRegRead( REG_SELECTED_PORT_0 ) & MAIN_PIPE_MASK;
    mpPortBitMask = uData;
    uData = SiiRegRead( REG_RX_PORT_SEL );
    portIndex = uData & MSK_MP_PORT_SEL;
    DEBUG_PRINT( MSG_ALWAYS, "      Port: %d (Bit: %02X)\n", portIndex, mpPortBitMask );
    if ( mpPortBitMask != (1 << portIndex))
    {
        PrintAlways( "** WARNING -- Index and port bits do not match!\n" );
    }

    uData = SiiRegRead( REG_MP_STATE );
    DEBUG_PRINT( MSG_ALWAYS, "HDMI Cable: %s\n", (uData & BIT_PWR5V) ? "Connected" : "---" );
    DEBUG_PRINT( MSG_ALWAYS, " MHL Cable: %s\n", (uData & BIT_MHL) ? "Connected" : "---" );
    uData  = SiiRegRead( REG_BSTATUS2 );
    DEBUG_PRINT( MSG_ALWAYS, "      MODE: %s\n", (uData & BIT_HDMI_MODE) ? "HDMI" : "DVI" );
    uData = SiiRegRead( REG_MP_STATE );
    PrintStringYesNo( "      CKDT:", (uData & BIT_CKDT) );
    PrintStringYesNo( "      SCDT:", (uData & BIT_SCDT) );
    uData16 = SiiRegRead( REG_ECC_ERRCOUNT_HI_MP );
    uData16 = (uData16 << 8) | SiiRegRead( REG_ECC_ERRCOUNT_LO_MP );
    DEBUG_PRINT( MSG_ALWAYS, "ECC Errors: %d\n", uData16 );


    DEBUG_PRINT( MSG_ALWAYS, "\n----   Selected Port (%d)   ----\n", portIndex );
    pwr5vStatus = SiiRegRead( REG_PWR5V_STATUS ) & MSK_PWR5V_ALL;
    DEBUG_PRINT( MSG_ALWAYS, "    Cable: %s\n", (pwr5vStatus & mpPortBitMask ) ? "Connected" : "---" );
    ckdtStatus = SiiRegRead( REG_CLKDETECT_STATUS ) & MSK_CKDT_ALL;
    PrintStringYesNo( "     CKDT:", (ckdtStatus & mpPortBitMask ) );

    uData = SiiRegRead( REG_RX_HDCP_DDC_EN );
    uData1 = uData >> 4;
    hDdc = uData1;
    DEBUG_PRINT( MSG_ALWAYS, " HDCP DDC: %s\n", (hDdc & mpPortBitMask ) ? "Enabled" : "---" );

    eDdc = SiiRegRead( RX_EDID_DDC_EN );
    DEBUG_PRINT( MSG_ALWAYS, " EDID DDC: %s\n", (eDdc & mpPortBitMask) ? "Enabled" : "---" );

    uData16  = (((uint16_t)SiiRegRead( REG_RX_TMDS_TERM_2 )) << 8);
    termCtrl = uData16 | SiiRegRead( REG_RX_TMDS_TERM_0 );
    DEBUG_PRINT( MSG_ALWAYS, "     TERM: %s\n", termTypeMsgs[ (termCtrl >> (portIndex * 2)) & 0x03]);


    uData16 = 0;
    bcaps   = uData16 | SiiRegRead( REG_HDCP_BCAPS_SET );
    PrintStringYesNo( " Repeater:", ((bcaps >> (portIndex * 2)) & 0x02) );
    if ( !app.repeaterEnable && ((bcaps >> (portIndex * 2)) & 0x02))
    {
        PrintAlways( "** WARNING -- RX repeater BCAPS bit set in non-repeater mode\n" );
    }
    PrintStringYesNo( " FIFO Rdy:", ((bcaps >> (portIndex * 2)) & 0x01) );

    PrintAlways( "\n----     All Ports     ----\n" );
    DEBUG_PRINT( MSG_ALWAYS, "    Cable: %02X\n", pwr5vStatus );
    DEBUG_PRINT( MSG_ALWAYS, "     CKDT: %02X\n", ckdtStatus );
    if ( ckdtStatus & ~pwr5vStatus )
    {
        PrintAlways( "** WARNING -- CKDT Activity on unconnected ports\n" );
    }
    DEBUG_PRINT( MSG_ALWAYS, " HDCP DDC: %02X\n", hDdc );
    DEBUG_PRINT( MSG_ALWAYS, " EDID DDC: %02X\n", eDdc );
    DEBUG_PRINT( MSG_ALWAYS, "     TERM: %03X\n", termCtrl );
    DEBUG_PRINT( MSG_ALWAYS, "    BCAPS: %03X\n", bcaps );

}


//------------------------------------------------------------------------------
// Function:    SkAppDiagLevelZero
// Description: Handles diagnostics command options
// Returns:
//------------------------------------------------------------------------------

static char *pDiagLevel0 [] =
{
    "\nDiagnostics",
    tenDashes,
    ccDisplayThisMenuMsg,
    returnToMainDemoMsg,
    "1 - Dump current Switch (RX) State",
    NULL
};

bool_t SkAppDiagLevelZero ( RC5Commands_t key )
{

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            PrintMenu( pDiagLevel0 );
            break;

        case RC5_0:
            app.setupMenuLevel = 0;
            PrintAlways( setupSelectMenu );
            break;

        case RC5_1:
            DumpSwitchState();
            break;

        default:
            break;
    }

    return( false );
}

#endif

