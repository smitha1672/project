//***************************************************************************
//!file     sk_app_cdc_demo.c
//!brief    Built-in Demo manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if INC_CDC
#include "si_timer.h"
#include "sk_application.h"

#include "si_cdc_heac.h"
#include "si_cdc_component.h"
#include "si_cec_component.h"

//------------------------------------------------------------------------------
// Function:    DemoCdcInfoPrint
// Description: Show current CDC status
//------------------------------------------------------------------------------

static void DemoCdcInfoPrint ( void )
{
    int i;
    char *pStateStr;
    CdcHecDescriptor_t  *pHec            = SiiCdcHecRecordsGet();
    CdcPhysAddrTime_t   *pAliveDevice    = SiiCdcAliveDeviceListGet();
    CdcFoundListEntry_t *pFoundHostsList = SiiCdcFoundHostListGet();

    // show HEC list
    DEBUG_PRINT( MSG_STAT, "\n-------- HEC List --------\n" );
    for (i = 0; i < CDC_NUMBER_OF_HECS_MAX; i++)
    {
        if (pHec[i].status.isInUse == true)
        {
            switch (pHec[i].status.channelState)
            {
                default:
                case CDC_PHEC:
                    pStateStr = "PHEC";
                    break;
                case CDC_VHEC:
                    pStateStr = "VHEC";
                    break;
                case CDC_AHEC:
                    pStateStr = "AHEC";
                    break;
            }

            DEBUG_PRINT(
                MSG_STAT,
                "%04x >> %04x, act: %04x, typ: %s, state: %s\n",
                pHec[i].firstDevPhysAddr,
                pHec[i].lastDevPhysAddr,
                pHec[i].activatorPhysAddr,
                pHec[i].status.isAdjacent ? "adjacent" : "normal",
                pStateStr );
        }
    }

    // show alive list
    DEBUG_PRINT( MSG_STAT, "-------- Alive List --------\n" );
    for (i = 0; i < CDC_NUMBER_OF_DEVICES_MAX; i++)
    {
        if (pAliveDevice[i].physAddr != 0xFFFF)
        {
            DEBUG_PRINT(
                MSG_STAT,
                "%04x, left: %d s\n",
                pAliveDevice[i].physAddr,
                CdcTimeCounterLeftSecGet(&pAliveDevice[i].TimeCount)
                );
        }

    }

    // show found devices list
    DEBUG_PRINT( MSG_STAT, "-------- Found Devices List --------\n" );

    for (i = 0; i < SiiCdcNumberOfFoundHostsGet(); i++)
    {
        DEBUG_PRINT( MSG_STAT, "%04x [h = %d, e = %d, sel = %d]\n",
            pFoundHostsList[i].physAddr,
            (int) pFoundHostsList[i].devStat.hostFuncState,
            (int) pFoundHostsList[i].devStat.encFuncState,
            (int) pFoundHostsList[i].hostInfo.isSelected );
    }

}

//------------------------------------------------------------------------------
// Function:    DemoCdcInquireStateTest
// Description: Sends InquireState CDC message
//------------------------------------------------------------------------------

static void DemoCdcInquireStateTest ( void )
{
    uint8_t hecIdx;
    CdcHecDescriptor_t  *pHec     = SiiCdcHecRecordsGet();
    uint16_t             physAddr = SiiCdcPhysAddrGet();

    hecIdx = CdcHecDescriptorAdd(true);

    pHec[hecIdx].activatorPhysAddr = physAddr;
    pHec[hecIdx].firstDevPhysAddr  = physAddr;
    pHec[hecIdx].lastDevPhysAddr  = 0x2000; // TE ID1 (HEACT Table 7-1)

    CdcHecInquireStateSend(hecIdx);

    DEBUG_PRINT(
        MSG_STAT,
        "InquireState sent to HEC %d [%04x,%04x]\n",
        (int)hecIdx,
        pHec[hecIdx].firstDevPhysAddr,
        pHec[hecIdx].lastDevPhysAddr );

    CdcHecDescriptorRemove(hecIdx);
}


//------------------------------------------------------------------------------
// Function:    DemoCdcSingleDeviceConnectTest
// Description: Activates and Deactivates HECs to the devices from
//              the device found list
//------------------------------------------------------------------------------

static void DemoCdcSingleDeviceConnectTest ( uint8_t devIndex )
{
    CdcFoundListEntry_t *pFoundHostsList = SiiCdcFoundHostListGet();
    uint16_t             physAddr        = SiiCdcPhysAddrGet();

    if (CdcHecIndexGet(physAddr, physAddr, pFoundHostsList[devIndex].physAddr) == 0xFF)
    {
        SiiCdcDeviceConnect( false, physAddr, pFoundHostsList[devIndex].physAddr);
        DEBUG_PRINT(
            MSG_STAT,
            "TEST: Activation of a HEC %d [%04x,%04x]\n",
            (int)devIndex,
            physAddr,
            pFoundHostsList[devIndex].physAddr );
    }
    else
    {
        SiiCdcDeviceDisconnect( false, physAddr, physAddr, pFoundHostsList[devIndex].physAddr);
        DEBUG_PRINT(
            MSG_STAT,
            "TEST: Deactivation of a HEC %d [%04x,%04x]\n",
            (int) devIndex,
            physAddr,
            pFoundHostsList[devIndex].physAddr );
    }
}

//------------------------------------------------------------------------------
// Function:    DemoCdc
// Description: CDC demo command options
// Returns:
//------------------------------------------------------------------------------

bool_t SkAppCdcDemo ( RC5Commands_t key )
{
    uint8_t digit;

    switch ( key )
    {
#if (PHILIPS_REMOTE == 1)
        case RC5_CC_SUBTITLE:
#else
        case RC5_HELP:
#endif
            DEBUG_PRINT( MSG_ALWAYS, "\nCDC Demo\n" );
            DEBUG_PRINT( MSG_ALWAYS, "--------\n" );
            DEBUG_PRINT( MSG_ALWAYS, ccDisplayThisMenuMsg );
            DEBUG_PRINT( MSG_ALWAYS, returnToMainDemoMsg );
            DEBUG_PRINT( MSG_ALWAYS, "1 - CDC Information display\n" );
            DEBUG_PRINT( MSG_ALWAYS, "2 - CDC Discovery\n" );
            DEBUG_PRINT( MSG_ALWAYS, "3 - CDC Inquire State Test\n" );
            DEBUG_PRINT( MSG_ALWAYS, "4 - CDC Connect to all found devices\n" );
            DEBUG_PRINT( MSG_ALWAYS, "5 - CDC Disconnect from all devices\n" );
            DEBUG_PRINT( MSG_ALWAYS, "6 - CDC Connect to a single device\n" );
            DEBUG_PRINT( MSG_ALWAYS, "7 - CDC Connect to adjacent device at PA 2000\n" );
            DEBUG_PRINT( MSG_ALWAYS, "8 - CDC Disconnect from adjacent device at PA 2000\n" );
            DEBUG_PRINT( MSG_ALWAYS, "9 - CDC HPD Enable/Disable\n" );
            break;

        case 0:
            app.demoMenuLevel = 0;
            DEBUG_PRINT( MSG_ALWAYS, demoMenuExit );
            break;
        case RC5_1:
            DemoCdcInfoPrint();
            break;
        case RC5_2:
            SiiCdcSrvHostsFind();
            break;
        case RC5_3:
            DemoCdcInquireStateTest();
            break;
        case RC5_4:
            SiiCdcDeviceConnect(true, 0, 0);
            break;
        case RC5_5:
            SiiCdcDeviceDisconnect(true, 0, 0, 0);
            break;
        case RC5_6:
            DEBUG_PRINT( MSG_ALWAYS, " Device number (0-9): " );
            digit = SkAppIrGetSingleDigit( 9 );
            DemoCdcSingleDeviceConnectTest( digit );
            break;
        case RC5_7:
            SiiCdcDeviceAdjacentConnect(0x2000);
            break;
        case RC5_8:
            SiiCdcDeviceAdjacentDisconnect(0x2000);
            break;
        case RC5_9:
            DEBUG_PRINT( MSG_ALWAYS, " Enable/Disable(1/0): " );
            digit = SkAppIrGetSingleDigit( 1 );
            SiiCdcHpdSetState( SiiPORT_0, digit ? CDC_HPD_CP_AND_EDID_ENABLE : CDC_HPD_CP_AND_EDID_DISABLE);
            break;
        default:
            break;
    }

    return( false );
}

#else
char cdcDemo;   // Avoids compiler warning of empty translation unit
#endif          // #if INC_APP_CDC == 1
