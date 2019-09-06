//***************************************************************************
//!file     sk_app_emu_osd.c
//!brief    Built-in diagnostics manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "sk_application.h"
#include "si_drv_osd.h"
#include "si_osd_component.h"
#include "si_drv_msw_internal.h"
//------------------------------------------------------------------------------
// Function:    OsdRAMCompare
// Description: 
// Returns:
//------------------------------------------------------------------------------
void OsdRAMCompare(uint16_t length, uint8_t *pData, uint8_t val)
{
	uint8_t i;
	uint8_t result=false;

	while (length)
	{
		for (i=0; i < OSD_RAM_BURST_SIZE; i++)
		{
	    	if (pData[i] != val)
	    	{
	 			DEBUG_PRINT( MSG_ALWAYS, "\n%04d: %02x %02x\n", i,  pData[i], val);
	 			result = true;
	    	}
		}
		length--;
	}

	if (result)
		DEBUG_PRINT( MSG_ALWAYS, "\n******* OSD RAM fail ********\n");
	else
		DEBUG_PRINT( MSG_ALWAYS, "\n******* OSD RAM pass ********\n");
	
}

//------------------------------------------------------------------------------
// Function:    OsdRAMInit
// Description: 
// Returns:
//------------------------------------------------------------------------------
void OsdRAMInit(uint8_t *pData, uint8_t val)
{
	uint8_t i;

	for (i=0; i < OSD_RAM_BURST_SIZE; i++)
		pData[i] = val;

}

#define OSD_RAM_MAX	(2048*6)
extern uint8_t FontCharsTable[]; 
//------------------------------------------------------------------------------
// Function:    SkEmuOsd
// Description: 
// Returns:
//------------------------------------------------------------------------------
void  SkEmuOsd()
{
	uint8_t key;
	uint8_t OsdRAM[OSD_RAM_MAX];
	uint16_t i;
#ifdef OSD_TIME_RELOAD        

	uint16_t temp ;
#endif
	while (1)
	{

    	DEBUG_PRINT( MSG_ALWAYS, "\nSetup OSD Menu \n" );
        DEBUG_PRINT( MSG_ALWAYS, "--------------\n" );
        DEBUG_PRINT( MSG_ALWAYS, ccDisplayThisMenuMsg );
        DEBUG_PRINT( MSG_ALWAYS, returnToSetupMenuMsg );
        DEBUG_PRINT( MSG_ALWAYS, "1 - Burst Write 2048 RAM Word (One word is 6 bytes)\n" );
        DEBUG_PRINT( MSG_ALWAYS, "2 - Burst Write 256 RAM Word (8 Burst Write)\n" );
        DEBUG_PRINT( MSG_ALWAYS, "3 - Burst Write 1 OSD RAM Word (2048 Burst Write)\n" );
        DEBUG_PRINT( MSG_ALWAYS, "4 - Burst Read 2048 RAM Word (One word is 6 bytes)\n" );
#ifdef OSD_TIME_RELOAD        
        DEBUG_PRINT( MSG_ALWAYS, "5 - OSDRAM I2C Reload\n" );
        DEBUG_PRINT( MSG_ALWAYS, "6 - OSDRAM SPI Reload\n" );
#else
        DEBUG_PRINT( MSG_ALWAYS, "5 - Compare pattern 0xAA\n" );
        DEBUG_PRINT( MSG_ALWAYS, "6 - Compare pattern 0x55\n" );
#endif
        DEBUG_PRINT( MSG_ALWAYS, "7 - Compare pattern 0x00\n" );
        DEBUG_PRINT( MSG_ALWAYS, "8 - Load data from SPI to OSD Ram\n" );
        DEBUG_PRINT( MSG_ALWAYS, "9 - Write to OSD RAM from a ROM FONT Table\n" );
        
		key = SkAppIrGetSingleDigit( 9 );

	    switch ( key )
	    {
	        case RC5_0:
	            return;
	        case RC5_1:
	        	for (i=0; i < OSD_RAM_MAX; i++)
	        	{
	        		OsdRAM[i] = 0xAA;
	        	}
	        	SiiDrvOsdRamI2CInit(2048, OsdRAM);
	        	break;
	        case RC5_2:
	        	for (i=0; i < OSD_RAM_MAX; i++)
	        		OsdRAM[i] = 0x55;
	        	for (i=0; i < 8; i++)	//2048/256
	        	{
	        		SiiDrvOsdRamI2CInit(256, OsdRAM);
	        	}
	        	break;
	        case RC5_3:
	        	for (i=0; i < OSD_RAM_MAX; i++)
	        		OsdRAM[i] = 0x00;
	        	for (i=0; i < 4; i++)
	        	{
	        		SiiDrvOsdRamI2CInit(1, OsdRAM);
	        	}
	        	break;

	        case RC5_4:
	        	SiiDrvOsdRamI2CRead(0, 2048, OsdRAM);
	        	for (i=0; i < OSD_RAM_MAX; i++)
	        	{
	        		if (( i % 6) == 0)
	        		{
	        			DEBUG_PRINT(MSG_DBG, "\n%04d:", i);
	        		}
					DEBUG_PRINT(MSG_DBG, "0x%02x " , OsdRAM[i]);
	        	}
	        	break;
#ifdef OSD_TIME_RELOAD
	        case RC5_5:
	        	temp = SiiRegRead(REG_SCRATCH0);
	        	temp = (SiiRegRead(REG_SCRATCH1) << 8) | temp;
	        	if (temp == 0)
	        	{
	        		temp = 2048;
	        	}	
				SiiDrvOsdRamI2CWrite(0x00, temp, OsdRAM);
				break;

	        case RC5_6:
	        	temp = SiiRegRead(REG_SCRATCH0);
	        	temp = (SiiRegRead(REG_SCRATCH1) << 8) | temp;

	        	if (temp == 0)
	        	{
	        		temp = 2048;
	        	}	
				SiiDrvOsdSpiReload(0x00, 0x00, temp * 6);
				break;
#else
	        case RC5_5:
	        	SiiDrvOsdRamI2CRead(0, 2048, OsdRAM);
		        OsdRAMCompare(2048, OsdRAM, 0xAA);
				break;
	        case RC5_6:
	        	SiiDrvOsdRamI2CRead(0, 2048, OsdRAM);
		        OsdRAMCompare(2048, OsdRAM, 0x55);
				break;
	        case RC5_7:
	        	SiiDrvOsdRamI2CRead(0, 2048, OsdRAM);
		        OsdRAMCompare(2048, OsdRAM, 0x00);
				break;
#endif				
	        case RC5_8:
//TODO: Lee				SiiDrvOsdSpiReload(pOsdSpi->SpiFontTable12x16StartAddr,
//TODO: Lee						pOsdSpi->SpiFontTable12x16StartAddr, pOsdSpi->SpiFontTable16x24StartAddr);
				break;
	        case RC5_9:
//TODO: Lee				SiiDrvOsdRamI2CWrite(pOsdSpi->SpiFontTable12x16StartAddr,
//TODO: Lee					pOsdSpi->SpiFontTable16x24StartAddr/6,
//TODO: Lee							FontCharsTable);
				break;        	
				
	        default:
	            break;
	    }

	}
}

//------------------------------------------------------------------------------
// Function:    SkEmuArc
// Description: 
// Returns:
//------------------------------------------------------------------------------
void  SkEmuArc()
{
	uint8_t key;

	while (1)
	{
		        
		DEBUG_PRINT( MSG_ALWAYS, "\n ARC Main Menu\n" );
	    DEBUG_PRINT( MSG_ALWAYS, "-----------\n" );
	    DEBUG_PRINT( MSG_ALWAYS, ccDisplayThisMenuMsg );
	    DEBUG_PRINT( MSG_ALWAYS, "0 - Return to main Emulation Menu\n" );
        
		DEBUG_PRINT( MSG_ALWAYS, "1 - ARC TX,S/PDIF \n" );
		DEBUG_PRINT( MSG_ALWAYS, "2 - ARC RX,S/PDIF \n" );

		key = SkAppIrGetSingleDigit( 4 );

	    switch ( key )
	    {

	        case RC5_0:
	            return;

	        case RC5_1:
	        	SkAppCecInstanceSet( CEC_INSTANCE_AVR );
                SiiDrvArcConfigureArcPin( 0, 0, ARC_TX_MODE );
				break;
	        case RC5_2:
	        	SkAppCecInstanceSet( CEC_INSTANCE_AUXTX );
                SiiDrvArcConfigureArcPin( 0, 0, ARC_RX_MODE );
				break;
	        default:
	            break;
	    }
	}	
}


