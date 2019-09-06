//***************************************************************************
//!file     sk_app_emu_spi.c
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
#include "si_drv_spi.h"
#include "si_drv_msw_internal.h"
#include "si_platform.h"
#include "si_osd_component.h"
#include "si_gpio.h"


extern uint8_t FontCharsTable[];
extern OsdSpiMem_t OsdSpiData;

//------------------------------------------------------------------------------
// Function:    SkEmuOsd
// Description: 
// Returns:
//------------------------------------------------------------------------------
void  SkEmuSpi()
{
	uint8_t key;
	//uint8_t buf[5];
	while (1)
	{

    	DEBUG_PRINT( MSG_ALWAYS, "\nSetup SPI Menu \n" );
        DEBUG_PRINT( MSG_ALWAYS, "--------------\n" );
        DEBUG_PRINT( MSG_ALWAYS, ccDisplayThisMenuMsg );
        DEBUG_PRINT( MSG_ALWAYS, returnToSetupMenuMsg );
        DEBUG_PRINT( MSG_ALWAYS, "1 - SPI Erase All Sectors\n" );
        DEBUG_PRINT( MSG_ALWAYS, "2 - SPI Program\n" );
        DEBUG_PRINT( MSG_ALWAYS, "3 - CEC Select 0\n" );
        DEBUG_PRINT( MSG_ALWAYS, "4 - CEC Select 1\n" );
        DEBUG_PRINT( MSG_ALWAYS, "5 - CEC Select 2\n" );
        DEBUG_PRINT( MSG_ALWAYS, "6 - CEC Select 3\n" );
        DEBUG_PRINT( MSG_ALWAYS, "7 - CEC Select 4\n" );
        DEBUG_PRINT( MSG_ALWAYS, "8 - CEC Select 5\n" );
        
		key = SkAppIrGetSingleDigit( 9 );

	    switch ( key )
	    {
	        case RC5_0:
	            return;
	        case RC5_1:
	        	SiiDrvSpiErase (SPI_SECTOR_ERASE, 0x00);
	        	break;
	        case RC5_2:
				SiiDrvSpiWrite(OsdSpiData.SpiFontTable12x16StartAddr, FontCharsTable, OsdSpiData.SpiFontTable16x24StartAddr);
	        	break;
	        case RC5_3:
        		DEBUG_PRINT( MSG_ALWAYS, "CEC Select 0: IR Key 0 for CEC_AM, IR Key 1 for  CEC_AS\n" );
        		key = SkAppIrGetSingleDigit(1);
        		SiiPlatformGpioCecSelect(SII_GPIO_CEC_SELECT_0, key);
	        	break;
	        case RC5_4:
        		DEBUG_PRINT( MSG_ALWAYS, "CEC Select 1: IR Key 0 for CEC_AM, IR Key 1 for  CEC_AS\n" );
        		key = SkAppIrGetSingleDigit(1);
        		SiiPlatformGpioCecSelect(SII_GPIO_CEC_SELECT_1, key);
	        	break;
	        case RC5_5:
        		DEBUG_PRINT( MSG_ALWAYS, "CEC Select 2: IR Key 0 for CEC_AM, IR Key 1 for  CEC_AS\n" );
        		key = SkAppIrGetSingleDigit(1);
        		SiiPlatformGpioCecSelect(SII_GPIO_CEC_SELECT_2, key);
	        	break;
	        case RC5_6:
        		DEBUG_PRINT( MSG_ALWAYS, "CEC Select 3: IR Key 0 for CEC_AM, IR Key 1 for  CEC_AS\n" );
        		key = SkAppIrGetSingleDigit(1);
        		SiiPlatformGpioCecSelect(SII_GPIO_CEC_SELECT_3, key);
	        	break;
	        case RC5_7:
        		DEBUG_PRINT( MSG_ALWAYS, "CEC Select 4: IR Key 0 for CEC_AM, IR Key 1 for  CEC_AS\n" );
        		key = SkAppIrGetSingleDigit(1);
        		SiiPlatformGpioCecSelect(SII_GPIO_CEC_SELECT_4, key);
	        	break;
	        case RC5_8:
        		DEBUG_PRINT( MSG_ALWAYS, "CEC Select 5: IR Key 0 for CEC_AM, IR Key 1 for  CEC_AS\n" );
        		key = SkAppIrGetSingleDigit(1);
        		SiiPlatformGpioCecSelect(SII_GPIO_CEC_SELECT_5, key);
	        	break;
	        default:
	            break;
	    }

	}
}




