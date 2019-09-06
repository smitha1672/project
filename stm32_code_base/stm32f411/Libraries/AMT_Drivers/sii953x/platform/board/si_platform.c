//***************************************************************************
//!file     si_platform.c
//!brief    Silicon Image SiI9535 Starter Kit board platform.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "device_config.h"
#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105== 1 )||( configSII_DEV953x_PORTING_PLATFORM_STM32F411== 1 )
#include "freertos_conf.h"
#endif 

#include "si_common.h"
#include "si_platform.h"
#include "si_eeprom.h"
#include "si_gpio.h"
#include "si_timer.h"

#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
#include "si_uartstdio.h"
#include "si_usb_serial.h"
#include "si_flash_update.h"
#include "si_arm.h"
#endif 

#include "si_i2c.h"
#include "si_infrared.h"

//------------------------------------------------------------------------------
//  Board Component Instance Data
//------------------------------------------------------------------------------

BoardInstanceData_t boardInstance =
{
    0,                          // structVersion
    SII_SUCCESS,                // lastResultCode
    0,                          // statusFlags
    false,                      // i2CError;
    false,                      // i2cErrorCode
    MSG_PRINT_ALL,              // dbgMsgLevel

    false,                      // i2c16bitAddr
    false,                      // i2cNoInterimStops
    false,                      // hardwareInt
};

BoardInstanceData_t platform;

#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
static const uint32_t l_debounceTimes [ 16 ] =
{
    DEBOUNCE_0, DEBOUNCE_1, DEBOUNCE_2, DEBOUNCE_3,
    DEBOUNCE_4, DEBOUNCE_5, DEBOUNCE_6, DEBOUNCE_7,
    DEBOUNCE_8, DEBOUNCE_9, DEBOUNCE_10, DEBOUNCE_11,
    DEBOUNCE_12, DEBOUNCE_13, DEBOUNCE_14, DEBOUNCE_15
};
#endif 

//------------------------------------------------------------------------------
// Function:    SiiPlatformPowerSwitchGet
// Description: Detect the state of the power pushbutton switch.
// Parameters:
// Returns:     true if state has changed. false if state has not been changed.
//------------------------------------------------------------------------------

bool_t SiiPlatformPowerSwitchGet (void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    static bool_t   debouncing = false;
    static bool_t   oldPin = false;
    bool_t      newPin;
    bool_t  changePin = false;
    static uint32_t   firstTime;
    uint32_t         thisTime;

    // Detecting the s1 switch debouncing
    newPin = (ROM_GPIOPinRead( GPIO_PORTB_BASE, GPIO_PIN_1) & GPIO_PIN_1) == 0;
    if (oldPin != newPin)
    {
        while (1)
        {
            newPin = (ROM_GPIOPinRead( GPIO_PORTB_BASE, GPIO_PIN_1) & GPIO_PIN_1) == 0;
            //Button just has been pressed.
            if (newPin && !debouncing)
            {
                firstTime = SiiPlatformTimerSysTicksGet();
                debouncing = true;
            }
            else if (debouncing)
            {
                thisTime = SiiPlatformTimerSysTicksGet();
                if (( thisTime - firstTime ) >= 10 )
                {
                    if (newPin)
                    {
                        debouncing = true;
                        firstTime = SiiPlatformTimerSysTicksGet();
                    }
                    else
                    {
                        debouncing = false;
                        changePin = true;       //!changePin;
                        break;
                    }
                }
            }
        }
    }
    oldPin = newPin;
    return changePin;
#else
	return false;
#endif 
    
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformReadRotary
// Description: Read the current value of the rotary switch, with debounce
// Parameters:  iWantItNow  - true: return GPIO value without debounce
// Returns:     Debounced position or 0xFF if no change
//------------------------------------------------------------------------------

uint8_t SiiPlatformReadRotary ( uint8_t iWantItNow )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    static uint32_t firstTime = 0;
    uint32_t        thisTime;
    static bool_t   debouncing = false;
    static uint8_t  oldPins = RSW_NO_CHANGE;
    uint8_t         newPins;

        // Get Rotary switch GPIO pins and determine value.

    newPins = ~ROM_GPIOPinRead( GPIO_ROTARY_SWITCH_BASE, GPIO_ROTARY_SWITCH_MASK ) & GPIO_ROTARY_SWITCH_MASK;
    newPins >>= GPIO_ROTARY_SWITCH_SHIFT;
    if ( iWantItNow )
    {
        return( newPins );
    }

    if ( newPins != oldPins ) // port change detected
    {
        oldPins = newPins;
        firstTime = SiiPlatformTimerSysTicksGet();
        debouncing = true;
    }
    else if ( debouncing )
    {
        thisTime = SiiPlatformTimerSysTicksGet();
        //if (( thisTime - firstTime ) >= l_debounceTimes[ newPins ] )
        {
            debouncing = false;
            return( newPins );
        }
    }
	
    return( RSW_NO_CHANGE );                // indicate no change
#else 
	return (RSW_POS_2);

#endif
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformSwitchGet
// Description: This function virtualizes the platform DIP slide switches for
//              the application.  The application only know SW1, SW_2, etc.
// Parameters:  DIP slide switch to access.
// Returns:     State of the specified switch: true - closed, false - open.
//------------------------------------------------------------------------------

bool_t SiiPlatformSwitchGet ( SkSwitchNumbers_t switchNum )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    bool_t closed;

    switch ( switchNum )
    {
        case SW_1:  closed = SiiPlatformGpioReadSwitch( SW2_1 ); break;
        case SW_2:  closed = SiiPlatformGpioReadSwitch( SW2_2 ); break;
        case SW_3:  closed = SiiPlatformGpioReadSwitch( SW2_3 ); break;
        case SW_4:  closed = SiiPlatformGpioReadSwitch( SW2_4 ); break;
        case SW_5:  closed = SiiPlatformGpioReadSwitch( SW2_5 ); break;
        case SW_6:  closed = SiiPlatformGpioReadSwitch( SW2_6 ); break;
        case SW_7:  closed = SiiPlatformGpioReadSwitch( SW2_7 ); break;
        default:
            closed = false;
            break;
    }
#else
	bool_t closed;
/*
		UI_AUTO_SWITCH		= SW_1,
		UI_REPEATER_ENABLE	= SW_2,
		UI_RC_ENABLE		= SW_3,
		UI_CEC_ENABLE		= SW_4,
		UI_THX_ENABLE		= SW_5,
		UI_FLASH_UPDATE 	= SW_6,
		UI_MHL_ENABLE		= SW_7,
*/
	
	switch ( switchNum )
    {
        case SW_1:  closed = true /*SiiPlatformGpioReadSwitch( SW2_1 )*/; break;
        case SW_2:  closed = false  /*SiiPlatformGpioReadSwitch( SW2_2 )*/; break;
        case SW_3:  closed = false /*SiiPlatformGpioReadSwitch( SW2_3 )*/; break;
        case SW_4:  closed = false /*SiiPlatformGpioReadSwitch( SW2_4 )*/; break;
        case SW_5:  closed = false /*SiiPlatformGpioReadSwitch( SW2_5 )*/; break;
        case SW_6:  closed = false /*SiiPlatformGpioReadSwitch( SW2_6 )*/; break;
        case SW_7:  closed = false /*SiiPlatformGpioReadSwitch( SW2_7 )*/; break;
        default:
            closed = false;
            break;
    }
#endif     

	return( closed );
}

//------------------------------------------------------------------------------
//! @brief  Return state of Siimon bus request pin
//! @return true - bus requested. false - bus NOT requested
//------------------------------------------------------------------------------
bool_t SiiPlatformSiimonBusRequest ( void )
{
#if INC_CBUS_WAKEUP
    return( false );
#else
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    return( ROM_GPIOPinRead( GPIO_PORTC_BASE, GPIO_PIN_7 ) == 0 );
#else
	return ( false );
#endif
#endif
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformSiimonBusGrant
// Description: Grant the bus to SiMon
// Parameters:
// Returns:     none
//------------------------------------------------------------------------------
void SiiPlatformSiimonBusGrant ( bool_t grant )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )

#if INC_CBUS_WAKEUP
    grant = false;
#else
    if ( !grant )
    {
        ROM_GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6,  GPIO_PIN_6);
        ROM_IntMasterEnable();            // Unfreeze IRQ handlers
    }
    else
    {
        ROM_IntMasterDisable();           // Freeze IR & GPIO interrupt handlers
        ROM_GPIOPinWrite( GPIO_PORTC_BASE, GPIO_PIN_6, 0 );
    }
#endif
#endif 
}


//------------------------------------------------------------------------------
// Function:    SiiPlatformInitialize
// Description: Initialize the platform hardware
// Parameters:  none
// Returns:     It returns true if the initialization is successful, or false
//              if some failure occurred.
//------------------------------------------------------------------------------

bool_t SiiPlatformInitialize ( void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    // Set the clocking to run from the PLL at 50MHz
    ROM_SysCtlClockSet( SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SiiPlatformUartInit(0, 19200);
    SiiPlatformI2cInit( I2C_INDEX0, I2C_MASTER, 0, 100 );
    SiiPlatformI2cInit( I2C_INDEX1, I2C_SLAVE, 0x40, 0 );
    SiiPlatformGpioInit();
    SiiPlatformTimerInit();
    SiiPlatformInfraRedInit();

    if (SiiPlatformGpioReadSwitch( SW2_6 ))
    {
        // Update program flash if needed.
        SiiPlatformFirmwareUpdate(SI_FLASH_USB_DFU_BOOT_LOADER);
        // This is the dead end.
    }
    else
    {
        SiiPlatformUsbInit(); // Initialize USB->UART module
    }

    SiiPlatformInterruptEnable();
    ROM_IntMasterEnable();


#if (FPGA_BUILD == ENABLE)
    {
    int_t   i;

    /* FPGA Initialization. */

    for ( i = 6; i >= 0; i-- )
    {
        DEBUG_PRINT( MSG_ALWAYS, "%d...", i );
        SiiPlatformTimerWait( 1000 );
    }
    }
#endif /* (FPGA_BUILD == ENABLE) */

#else
     SiiPlatformTimerInit();        
#endif 


    return( true );
}


//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptEnable
// Description: Interrupt enable for all sources
// Parameters:  none
//------------------------------------------------------------------------------

void SiiPlatformAllInterruptsEnable ( void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    ROM_IntMasterEnable();
#endif     
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptDisable
// Description: Interrupt disable for all sources
// Parameters:  none
//------------------------------------------------------------------------------

void SiiPlatformAllInterruptsDisable ( void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    ROM_IntMasterDisable();
#endif    
}


//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptEnable
// Description: Interrupt enable for Device H/W INT pin
// Parameters:  none
//------------------------------------------------------------------------------

void SiiPlatformInterruptEnable ( void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    ROM_GPIOPinIntEnable( GPIO_INT_PORT_BASE, GPIO_INT_PIN_IDX );
#endif     
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptDisable
// Description: Interrupt disable for Device H/W INT pin
// Parameters:  none
//------------------------------------------------------------------------------

void SiiPlatformInterruptDisable ( void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    ROM_GPIOPinIntDisable( GPIO_INT_PORT_BASE, GPIO_INT_PIN_IDX );
#endif     
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptHandler
// Description: Interrupt handler for device H/W INT pin
// Parameters:  none
//------------------------------------------------------------------------------

void SiiPlatformInterruptHandler ( void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    ROM_IntDisable(GPIO_INT_PORT);
    if (( ROM_GPIOPinIntStatus( GPIO_INT_PORT_BASE, true ) & GPIO_INT_PIN_IDX ))
    {
        ROM_GPIOPinIntClear( GPIO_INT_PORT_BASE, GPIO_INT_PIN_IDX );
        platform.hardwareInt = true;
    }
    ROM_IntEnable(GPIO_INT_PORT);
#endif

#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) ||( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 1 ) 
		platform.hardwareInt = true;

#endif 

}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptPinStateGet
// Description: Return false if h/w interrupt line is low (active),
//              or true otherwise (no pending events)
// Parameters:  none
//------------------------------------------------------------------------------

bool_t SiiPlatformInterruptPinStateGet ( void )
{
#if ( configSII_DEV_953x_PORTING_PLATFORM_LM3S5956 == 1 )
    return( ROM_GPIOPinRead( GPIO_INT_PORT_BASE, GPIO_INT_PIN_IDX ) != 0 );
#endif

#if ( configSII_DEV953x_PORTING_PLATFORM_STM32F105 == 1 ) && ( configSIL953x_ISR == 1 )
	bool_t ret;

	if (GPIO_ReadInputDataBit( GPIOE, GPIO_Pin_3 ) == Bit_SET )
	{
		ret = true;
	}
	else
	{
		ret = false;
	}
	
	return ret;
#elif ( configSII_DEV953x_PORTING_PLATFORM_STM32F411 == 1 ) && ( configSIL953x_ISR == 1 )
	bool_t ret;

	if (GPIO_ReadInputDataBit( GPIOE, GPIO_Pin_1 ) == Bit_SET )
	{
		ret = true;
	}
	else
	{
		ret = false;
	}
	
	return ret;
#endif
}


//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptStateGet
// Description: Return true if h/w interrupt has fired
// Parameters:  none
//------------------------------------------------------------------------------

bool_t SiiPlatformInterruptStateGet ( void )
{
    bool_t isIntPending = platform.hardwareInt;
    return( isIntPending );
}


//------------------------------------------------------------------------------
// Function:    SiiPlatformInterruptClear
// Description: Clears interrupt flag. Call when all events got processed.
// Parameters:  none
//------------------------------------------------------------------------------

void SiiPlatformInterruptClear ( void )
{
    platform.hardwareInt = false;
}

//-------------------------------------------------------------------------------------------------
//! @brief      Return result of last I2C operation
//  Parameters:
//!
//! @retval     - true: successful.
//! @retval     - false: failure
//-------------------------------------------------------------------------------------------------

uint32_t SiiPlatformLastI2cStatus ( void )
{

    return( platform.i2cErrorCode );
}

//-------------------------------------------------------------------------------------------------
//! @brief      Returns the result of the last platform function called
//!
//! @retval     - true: successful.
//! @retval     - false: failure
//-------------------------------------------------------------------------------------------------

int_t SiiPlatformGetLastResult ( void )
{

    return( platform.lastResultCode );
}

