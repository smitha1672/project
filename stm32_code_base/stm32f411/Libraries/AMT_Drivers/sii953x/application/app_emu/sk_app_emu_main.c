//***************************************************************************
//!file     sk_app_emu_main.c
//!brief    Built-in diagnostics manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_drv_internal.h"
#include "si_drv_device.h"
#include "si_drv_tpi_ddc_access.h"


//------------------------------------------------------------------------------
// Function:    SkAppSetupIrRemote
// Description: Parse remote control keys for the setup application
//------------------------------------------------------------------------------

bool_t SkAppEmulationIrRemote (RC5Commands_t key, bool_t fullAccess)
{
    bool_t usedKey = false;
    uint8_t digit;

#if (PHILIPS_REMOTE == 1)
    if ( key == RC5_TOGGLE )
#else
        if ( key == RC5_GUIDE )
#endif
    {
	  	DEBUG_PRINT( MSG_ALWAYS, "\n Emulation Menu\n" );
	    DEBUG_PRINT( MSG_ALWAYS,   " -----------\n" );
	    DEBUG_PRINT( MSG_ALWAYS, "0 - Return to Tv menu\n" );
	    DEBUG_PRINT( MSG_ALWAYS, "1 - ARC Emulation\n" );
	    DEBUG_PRINT( MSG_ALWAYS, "2 - \n" );
	    DEBUG_PRINT( MSG_ALWAYS, "3 - Audio Emulation\n" );
	    DEBUG_PRINT( MSG_ALWAYS, "4 - Osd Emulation\n" );
	    DEBUG_PRINT( MSG_ALWAYS, "5 - Spi Emulation\n" );

	    digit = SkAppIrGetSingleDigit( 5 );
	   	
	    switch (digit)
	    {
	        case 0:         // Menu idle mode
	        	SkAppIrRemoteReturnToTV();
	            break;
	        case 1:
	        	SkEmuArc();
	            break;
	        case 3:
	            SkEmuAudio();
	            break;
	        case 4:
#if INC_OSD
	            SkEmuOsd();
#endif
	            break;
	        case 5:
#if INC_SPI
	            SkEmuSpi();
#endif
	            break;

	            
	        default:
	            break;
	    }
	}

    return( usedKey );
}
