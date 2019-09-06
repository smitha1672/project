//***************************************************************************
//!file     si_flash_update.c
//!brief    Silicon Image flash update interface.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_platform.h"
#include "si_osal_timer.h"
#include "si_gpio.h"

#include "si_arm.h"
#include "si_uartstdio.h"
#include "si_flash_update.h"


//*****************************************************************************
//
// Configuration of a bootloader type.
//
//*****************************************************************************


//*****************************************************************************
//
// Initialize UART0 and set the appropriate communication parameters.
//
//*****************************************************************************

static void JumpToBootLoader(unsigned long baudrate, uint8_t loaderType)
{
    // We must make sure we turn off SysTick and its interrupt before entering
    // the boot loader!
    //
    ROM_SysTickIntDisable();
    ROM_SysTickDisable();

    //
    // Disable all processor interrupts.  Instead of disabling them
    // one at a time, a direct write to NVIC is done to disable all
    // peripheral interrupts.
    //
    HWREG(NVIC_DIS0_R) = 0xffffffff;
    HWREG(NVIC_DIS1_R) = 0xffffffff;


	if (loaderType == SI_ROM_UART_BOOT_LOADER)
	{
	    // Configure the UART for 115200, n, 8, 1
		ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), baudrate,
	                        (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE |
	                         UART_CONFIG_WLEN_8));

	    // Enable the UART operation.
		ROM_UARTEnable(UART0_BASE);

		//Jump to rom boot loader
	    ROM_UpdateUART();
	}
	else if (loaderType == SI_FLASH_UART_BOOT_LOADER)
	{
		SiiPlatformUartExpClkInit( 0, baudrate );  // UART 0, 115KBaud
	}
	//SI_FLASH_USB_DFU_BOOT_LOADER or SI_FLASH_UART_BOOT_LOADER
    (*((void (*)(void))(*(unsigned long *)0x2c)))();
}


//------------------------------------------------------------------------------
// Function:    SiiPlatformFirmwareUpdate
// Description: Using ROM/Flash based UART or USB DFU to program the flash.
// Parameters:  none
// Returns:     none
//------------------------------------------------------------------------------

void SiiPlatformFirmwareUpdate(uint8_t loaderType)
{
	if (loaderType == SI_ROM_UART_BOOT_LOADER)
	{
	    SiiPlatformLoggerPrintf( 0, "\nROM-Based Serial UART Bootloader.\n" );
	}
	else if (loaderType == SI_FLASH_UART_BOOT_LOADER)
	{
	    SiiPlatformLoggerPrintf( 0, "\nFLASH-Based Serial UART Bootloader.\n" );
	}
	else 
	{
	    SiiPlatformLoggerPrintf( 0, "\nFLASH-Based USB DFU Bootloader.\n" );
	}

    SiiOsTimerWait(100);
    JumpToBootLoader(115200, loaderType);
}

