//***************************************************************************
//!file     si_gpio.c
//!brief    Platform GPIO driver, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
#include "inc/hw_gpio.h"
#include "si_arm.h"
#endif 

#undef GPIO_LOCK_KEY
#include "si_platform.h"
#include "si_i2c.h"
#include "si_gpio.h"
#include "si_timer.h"

//*****************************************************************************
//
// The base addresses of all the GPIO modules.  Both the APB and AHB apertures
// are provided.
//
//*****************************************************************************

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
static const unsigned long g_pulGPIOBaseAddrs[] =
{
    GPIO_PORTA_BASE, GPIO_PORTA_AHB_BASE,
    GPIO_PORTB_BASE, GPIO_PORTB_AHB_BASE,
    GPIO_PORTC_BASE, GPIO_PORTC_AHB_BASE,
    GPIO_PORTD_BASE, GPIO_PORTD_AHB_BASE,
    GPIO_PORTE_BASE, GPIO_PORTE_AHB_BASE,
    GPIO_PORTF_BASE, GPIO_PORTF_AHB_BASE,
    GPIO_PORTG_BASE, GPIO_PORTG_AHB_BASE,
    GPIO_PORTH_BASE, GPIO_PORTH_AHB_BASE,
    GPIO_PORTJ_BASE, GPIO_PORTJ_AHB_BASE,
};
#endif 

//*****************************************************************************
//
//! Configures the alternate function of a GPIO pin.
//!
//! \param ulPinConfig is the pin configuration value, specified as one of the
//! \b GPIO_P??_??? values.
//!
//! This function configures the pin mux that selects the peripheral function
//! associated with a particular GPIO pin.  Only one peripheral function at a
//! time can be associated with a GPIO pin, and each peripheral function should
//! only be associated with a single GPIO pin at a time (despite the fact that
//! many of them can be associated with more than one GPIO pin).
//!
//! \note This function is only valid on Tempest-class devices.
//!
//! \return None.
//
//*****************************************************************************

void SiiPlatformGPIOPinConfigure ( unsigned long ulPinConfig )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

    unsigned long ulBase, ulShift;

    // Extract the base address index from the input value.

    ulBase = (ulPinConfig >> 16) & 0xff;

    // Get the base address of the GPIO module, selecting either the APB or the
    // AHB aperture as appropriate.

    if (HWREG(SYSCTL_GPIOHSCTL) & (1 << ulBase))
    {
        ulBase = g_pulGPIOBaseAddrs[(ulBase << 1) + 1];
    }
    else
    {
        ulBase = g_pulGPIOBaseAddrs[ulBase << 1];
    }

    // Extract the shift from the input value.

    ulShift = (ulPinConfig >> 8) & 0xff;

    // Write the requested pin muxing value for this GPIO pin.

    HWREG(ulBase + GPIO_O_PCTL) = ((HWREG(ulBase + GPIO_O_PCTL) &
                                    ~(0xfUL << ulShift)) |
                                   ((ulPinConfig & 0xf) << ulShift));
#endif                                    

}

//*****************************************************************************
//
//! Configures pin(s) for use as GPIO inputs.
//!
//! \param ulPort is the base address of the GPIO port.
//! \param ucPins is the bit-packed representation of the pin(s).
//!
//! The GPIO pins must be properly configured in order to function correctly as
//! GPIO inputs; this is especially true of Fury-class devices where the
//! digital input enable is turned off by default.  This function provides the
//! proper configuration for those pin(s).
//!
//! The pin(s) are specified using a bit-packed byte, where each bit that is
//! set identifies the pin to be accessed, and where bit 0 of the byte
//! represents GPIO port pin 0, bit 1 represents GPIO port pin 1, and so on.
//!
//! \return None.
//
//*****************************************************************************

static void SkPlatformGPIOPinTypeGPIOInput( unsigned long ulPort, unsigned char ucPins, unsigned long ulPinType )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

    //
    // Make the pin(s) be inputs.
    //
    ROM_GPIODirModeSet(ulPort, ucPins, GPIO_DIR_MODE_IN);
  
    
    //
    // Set the pad(s) for standard push-pull operation.
    //
    ROM_GPIOPadConfigSet(ulPort, ucPins, GPIO_STRENGTH_2MA, ulPinType );
#endif     
}

//------------------------------------------------------------------------------
// Function:    Pca9557Read
// Description: Read the PCA9557PW device at the passed I2C device ID.
//              Mask the result with bitMask and return the result.
// Parameters:  devId - I2C address of port extender to read.
//              bitMask - Value to AND with port extender data before returning.
// Returns:     Value of selected PCA9557PW, masked with bitMask
//------------------------------------------------------------------------------

static uint8_t  Pca9557Read ( uint8_t devId, uint8_t bitMask )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

    uint8_t value;

    value = SiiPlatformI2cReadByte(0, devId, PCA_INPUT );

    return( value & bitMask );
#else
	return 0;
#endif 
}

#if (FPGA_BUILD == DISABLE)
//------------------------------------------------------------------------------
// Function:    Pca9557ReadModifyWrite
// Description: Read/Modify/Write the PCA9557PW device at the passed I2C device
//              ID.
// Parameters:  devId - I2C address of port extender to read.
//              bitMask - Value to NAND with port extender data before writeback.
//              bitValue- Value to OR with port extender data before writeback.
// Returns:     none
//------------------------------------------------------------------------------

static void Pca9557ReadModifyWrite ( uint8_t devId, uint8_t bitMask, uint8_t bitValue  )
{
    uint8_t value;


    value = SiiPlatformI2cReadByte(0, devId, PCA_INPUT );
    value &= ~bitMask;
    value |= (bitValue & bitMask);

    SiiPlatformI2cWriteByte(0, devId, PCA_OUTPUT, value );
}
#endif

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioCecSelect
// Description: 
// Parameters:  portBits
// Returns:     none
//------------------------------------------------------------------------------

void SiiPlatformGpioCecSelect (uint8_t cecSel, uint8_t ctrl )
{
#if (FPGA_BUILD == DISABLE)
    cecSel = 1 << cecSel;
    Pca9557ReadModifyWrite( I2C_SADDR_34, cecSel, ctrl ? SET_BITS : CLEAR_BITS);
#endif
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioAudioInOut
// Description: Enable Audio Input (Inerstion) or Extratcion
// Parameters:  audType: false: Insertion, true:extraction
// Returns:     none
//------------------------------------------------------------------------------

void SiiPlatformGpioAudioInOut ( uint8_t audType )
{
#if (FPGA_BUILD == DISABLE)
    Pca9557ReadModifyWrite( I2C_SADDR_38, BIT7, audType ? SET_BITS : CLEAR_BITS);
#endif
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioLedPorts
// Description: Turn port LEDs on or off
// Parameters:  portBits
// Returns:     none
//------------------------------------------------------------------------------

void SiiPlatformGpioLedPorts ( uint8_t portBits )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
#if (FPGA_BUILD == DISABLE)

    // Bit permutations due to board design
    portBits = ((portBits & BIT0) ? BIT1 : 0) |
               ((portBits & BIT1) ? BIT0 : 0) |
               ((portBits & BIT2) ? BIT5 : 0) |
               ((portBits & BIT3) ? BIT4 : 0);

    Pca9557ReadModifyWrite( I2C_SADDR_38, BIT3|BIT2|BIT1|BIT0, ~portBits & LED_STATUS_MASK );
#endif
#endif 
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioLedStatus1
// Description: Turn Status LED 1 on or off
// Parameters:  ledOn   - true: turn LED on
// Returns:     none
//------------------------------------------------------------------------------

void SiiPlatformGpioLedStatus1 ( bool_t ledOn )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

#if (FPGA_BUILD == DISABLE)
    Pca9557ReadModifyWrite( I2C_SADDR_3A, BIT0, (ledOn) ? LED_ON : LED_OFF );
#else
    ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_0, (ledOn ) ? 0 : GPIO_PIN_0 );
#endif

#endif 

}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioLedStatus2
// Description: Turn Status LED 2 on or off
// Parameters:  ledOn   - true: turn LED on
// Returns:     none
//------------------------------------------------------------------------------
void SiiPlatformGpioLedStatus2 ( bool_t ledOn )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

#if (FPGA_BUILD == DISABLE)
    Pca9557ReadModifyWrite( I2C_SADDR_3A, BIT1, ledOn ? LED_ON : LED_OFF );
#else
    ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_1, (ledOn ) ? 0 : GPIO_PIN_1 );
#endif
#endif

}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioLedArc
// Description: Turn ARC LED on or off
// Parameters:  ledOn   - true: turn LED on
// Returns:     none
//------------------------------------------------------------------------------

void SiiPlatformGpioLedArc ( bool_t ledOn )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

#if (FPGA_BUILD == DISABLE)
    Pca9557ReadModifyWrite( I2C_SADDR_38, BIT5, ledOn ? LED_ON : LED_OFF );
#endif
#endif

}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioLedHec
// Description: Turn HEC LED on or off
// Parameters:  ledOn   - true: turn LED on
// Returns:     none
//------------------------------------------------------------------------------
void SiiPlatformGpioLedHec ( bool_t ledOn )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
#if (FPGA_BUILD == DISABLE)
    Pca9557ReadModifyWrite( I2C_SADDR_38, BIT6, ledOn ? LED_ON : LED_OFF );
#endif
#endif 
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioReadSwitch
// Description: Read the state of a switch on the Starter kit board
// Parameters:  gpioSw: Upper nibble is the read function switch case, which
//                      chooses the GPIO bank
//                      Lower nibble is the bit position in the selected GPIO bank
// Returns:     true if Switch position closed, false if open
//------------------------------------------------------------------------------

bool_t SiiPlatformGpioReadSwitch ( GpioSw_t gpioSw )
{
    bool_t result = false;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

    switch ( (gpioSw >> 8) & 0xFF )
    {
        case 0:     // PORT A Bank
            result = (ROM_GPIOPinRead( GPIO_PORTA_BASE, gpioSw & 0xFF ) == 0);
            break;
        case 1:     // PORT B Bank
            result = (ROM_GPIOPinRead( GPIO_PORTB_BASE, gpioSw & 0xFF ) == 0);
            break;
        case 2:     // PORT C Bank
            result = (ROM_GPIOPinRead( GPIO_PORTC_BASE, gpioSw & 0xFF ) == 0);
            break;
        case 3:     // PORT D Bank
            result = (ROM_GPIOPinRead( GPIO_PORTD_BASE, gpioSw & 0xFF ) == 0);
            break;

        case 0x10:  // These switch positions are on an I/O extender
#if (FPGA_BUILD == DISABLE)
            result = (Pca9557Read( I2C_SADDR_3F, gpioSw & 0x7F ) != 0 );
#else
            result = (Pca9557Read( I2C_SADDR_3A, gpioSw & 0xFF ) != 0 );
#endif
            break;

        default:
            break;
    }
#endif

    return( result );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformEthernetSwitchEnable
// Description: Put Ethernet switch in reset (disable) or release from
//              reset (enable)
// Parameters:  enableEthernet    true or false
// Returns:     none
//------------------------------------------------------------------------------

void SiiPlatformEthernetSwitchEnable ( bool_t enableEthernet )
{
#if (FPGA_BUILD == DISABLE)
    Pca9557ReadModifyWrite( I2C_SADDR_38, BIT7, enableEthernet ? BIT7 : 0x00 );
#endif
}

//------------------------------------------------------------------------------
// Function:    SkPlatformGpioRpwrOff
// Description: Turn off RPWR for port specified
// Parameters:  rpwrPort    - Port to turn off
// Returns:     true if success, false if invalid port number
//------------------------------------------------------------------------------

bool_t SiiPlatformGpioVbusCtrl ( uint8_t vbusPort, uint8_t ctrl )
{
    bool_t success = true;

#if (FPGA_BUILD == DISABLE)
    if (vbusPort == MHL_RPWR_EN0)
    {
        Pca9557ReadModifyWrite( I2C_SADDR_3A, BIT5, ctrl ? BIT5 : 0x00 );  // '1' disable
    }
    else if (vbusPort == MHL_RPWR_EN1)
    {
        Pca9557ReadModifyWrite( I2C_SADDR_3A, BIT7 , ctrl ? BIT7 : 0x00  );  // '1' disable
    }
#endif

    return( success );
}

//------------------------------------------------------------------------------
// Function:    SkPlatformGpioRpwrOff
// Description: Turn off RPWR for port specified
// Parameters:  rpwrPort    - Port to turn off
// Returns:     true if success, false if invalid port number
//------------------------------------------------------------------------------

bool_t SiiPlatformGpioRpwr ( uint8_t rpwrPort, uint8_t ctrl )
{
    bool_t success = true;

#if (FPGA_BUILD == DISABLE)
    if (rpwrPort == MHL_RPWR_EN0)
    {
        Pca9557ReadModifyWrite( I2C_SADDR_3A, BIT4, ctrl ? 0x00 : BIT4);  // '1' disable
    }
    else if (rpwrPort == MHL_RPWR_EN1)
    {
        Pca9557ReadModifyWrite( I2C_SADDR_3A, BIT6 , ctrl ? 0x00 : BIT6 );  // '1' disable
    }
#endif

    return( success );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioSetPower
// Description: Power on or off the chip
// Parameters: ON - power up the chip. Off - power down the chip
// Returns:
//------------------------------------------------------------------------------

void SiiPlatformGpioSetPower (uint8_t pwrType, uint8_t pwrCtrl )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

#if (FPGA_BUILD == DISABLE)
    if (pwrType == SII_GPIO_PDN_POWER)
    {
        pwrCtrl ? ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_3,  GPIO_PIN_3) : ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_3,  0);
    }
    else if (pwrType == SII_GPIO_AON_POWER)
    {
        pwrCtrl ? ROM_GPIOPinWrite( GPIO_PORTB_BASE, GPIO_PIN_0,  0) : ROM_GPIOPinWrite( GPIO_PORTB_BASE, GPIO_PIN_0,  GPIO_PIN_0);
    }
    else if (pwrType == SII_GPIO_POWER_ALL)
    {
        pwrCtrl ? ROM_GPIOPinWrite( GPIO_PORTB_BASE, GPIO_PIN_0,  0) : ROM_GPIOPinWrite( GPIO_PORTB_BASE, GPIO_PIN_0,  GPIO_PIN_0);
        pwrCtrl ? ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_3,  GPIO_PIN_3) : ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_3,  0);
    }
#endif
#endif
}

#if INC_CBUS_WAKEUP
//------------------------------------------------------------------------------
// Function:    SiiPlatformCbusWakeupPinGet
// Description: returns state of GPIO pin connected to CBUS line where wake up
//              sequence is expected to arrive
//------------------------------------------------------------------------------
bool_t SiiPlatformCbusWakeupPinGet( uint8_t mhlChnl )
{
    uint8_t val = false;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
#if (FPGA_BUILD == DISABLE)
    if ( mhlChnl == MHL_CHNL_0 )
    {
        val = ROM_GPIOPinRead( GPIO_PORTC_BASE, GPIO_PIN_6 );
    }

    if ( mhlChnl == MHL_CHNL_1 )
    {
        val = ROM_GPIOPinRead( GPIO_PORTC_BASE, GPIO_PIN_7 );
    }
#endif
#endif 
    return (val ? true : false);
}
#endif

//------------------------------------------------------------------------------
// Function:    SiiPlatformMHLCDSensePinGet
// Description: returns state of GPIO pin connected to CBUS line where wake up
//              sequence is expected to arrive
//------------------------------------------------------------------------------
bool_t SiiPlatformMHLCDSensePinGet( uint8_t mhlChnl )
{
    uint8_t val = true;
    
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

#if (FPGA_BUILD == DISABLE)
    if ( mhlChnl == MHL_CHNL_0 )
    {
        val = ROM_GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_2 );
    }

    if ( mhlChnl == MHL_CHNL_1 )
    {
        val = ROM_GPIOPinRead( GPIO_PORTE_BASE, GPIO_PIN_3 );
    }
#endif

#endif

    return (val ? true : false);
}
//------------------------------------------------------------------------------
// Function:    SiiBoardGpioControlMainVCC5
// Description: Detecting the switch s1 and power on or off the chip.
// Parameters:
// Returns: true if pin has been changed. false pin has not been changed.
//------------------------------------------------------------------------------

uint8_t SiiPlatformGpioPowerVCC5 (void )
{
    bool_t          changePin = false;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )    
    static bool_t   debouncing = false;
    static bool_t   oldPin = false;
    bool_t          newPin;
    static uint32_t    firstTime;
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
#endif     
    return changePin;
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformGpioInit
// Description:
// Parameters:
// Returns:
//------------------------------------------------------------------------------

void SiiPlatformGpioInit (void)
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )

	int i,j;
    // Enable the GPIO ports used for the board UI

    ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOA );
    ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOB );
    ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOC );
    ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOD );
    ROM_SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE );

    // Port B, bit 7 is special, because it CAN be an NMI pin, and TI didn't
    // want it to be programmed accidently.  So they 'locked' it and it takes a
    // special sequence to unlock so that we can actually change any of its features.
    HWREG( GPIO_PORTB_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY_DD; // LOCK code
    HWREG( GPIO_PORTB_BASE + GPIO_O_CR)   = 0x000000FF;       // Enable all 8 bits
    HWREG( GPIO_PORTB_BASE + GPIO_O_LOCK) = 0x00000000;       // re-lock

    // Configure the GPIO pins used for the Rotary Switch.
    SkPlatformGPIOPinTypeGPIOInput( GPIO_PORTB_BASE, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5, GPIO_PIN_TYPE_STD_WPU );

#if INC_CBUS_WAKEUP
    //Configure this GPIO to detect the MHL wake pulse.
    SkPlatformGPIOPinTypeGPIOInput( GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_TYPE_STD_WPU );
    ROM_GPIOIntTypeSet( GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7, GPIO_FALLING_EDGE | GPIO_RISING_EDGE);
    ROM_IntEnable(INT_GPIOC);
#else
    // Configure the GPIO pins used for the Siimon BUS Grant and Request.
    SkPlatformGPIOPinTypeGPIOInput( GPIO_PORTC_BASE, GPIO_PIN_7, GPIO_PIN_TYPE_STD_WPU );
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7 );
#endif

    // Configure the GPIO pin used external Interrupt from the chip
    SkPlatformGPIOPinTypeGPIOInput( GPIO_INT_PORT_BASE, GPIO_INT_PIN_IDX, GPIO_PIN_TYPE_STD_WPU );
    ROM_GPIOIntTypeSet( GPIO_INT_PORT_BASE, GPIO_INT_PIN_IDX, GPIO_FALLING_EDGE );
    ROM_IntEnable(GPIO_INT_PORT);

#if (FPGA_BUILD == DISABLE)
    // Configure the GPIO pins used for the Main power switch.
    // VCC5_MAIN_EN#_PB reads the push button state, so it is an input
    // VCC5_MAIN_EN is used to turn the power on, so it is an output. Immediately set it to 1
    SkPlatformGPIOPinTypeGPIOInput( GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_TYPE_STD_WPU );
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTB_BASE, GPIO_PIN_0 );
    ROM_GPIOPinWrite( GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0 );

    // Configure the GPIO to control the LPSTBY (Low Power Standby)
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTD_BASE, GPIO_PIN_3 );
    ROM_GPIOPadConfigSet( GPIO_PORTD_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );
    ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_PIN_3 ); 

    // Configure the GPIO pin used to reset the I/O extenders
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTD_BASE, GPIO_PIN_1 );
    ROM_GPIOPadConfigSet( GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );
    // Enable CP9535 Chip Hard Reset
    ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_1, 0 );
    j=0;
    for(i=0;i<0x7fff;i++)
    {
    	j++;
    }
    ROM_GPIOPinWrite( GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_PIN_1 );

    //Configure GPIO for Reset Audio Dac
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 );
    ROM_GPIOPadConfigSet( GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );

    //Configure GPIO for Port 0 and Port 1 Cable Detect
    ROM_GPIOPinTypeGPIOInput( GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3 );
    ROM_GPIOPadConfigSet( GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );

    // Configure the GPIO pins used external Interrupt from the chip
    SkPlatformGPIOPinTypeGPIOInput( GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_PIN_TYPE_STD_WPU );
    ROM_GPIOIntTypeSet( GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE );

    // Extended GPIO chip initialization

    SiiPlatformI2cWriteByte(0, I2C_SADDR_38, PCA_POLARITY, PCA_DEFAULT_POLARITY );
    SiiPlatformI2cWriteByte(0, I2C_SADDR_38, PCA_CONFIG, 0x00 );                      // 7:0 output
    SiiPlatformI2cWriteByte(0, I2C_SADDR_38, PCA_OUTPUT, PCA_DEFAULT_OUTPUT );

    SiiPlatformI2cWriteByte(0, I2C_SADDR_3A, PCA_POLARITY, BIT2 );   // Upper nibble inverted inputs
    SiiPlatformI2cWriteByte(0, I2C_SADDR_3A, PCA_CONFIG, BIT2 );     // 7:3 output, 2-input, 1:0 output
    SiiPlatformI2cWriteByte(0, I2C_SADDR_3A, PCA_OUTPUT, PCA_DEFAULT_OUTPUT );

    SiiPlatformI2cWriteByte(0, I2C_SADDR_3F, PCA_POLARITY, (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0) );
    SiiPlatformI2cWriteByte(0, I2C_SADDR_3F, PCA_CONFIG, (BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0) );    // 7 output, 6:0 input
    SiiPlatformI2cWriteByte(0, I2C_SADDR_3F, PCA_OUTPUT, PCA_DEFAULT_OUTPUT|BIT7 );

    SiiPlatformI2cWriteByte(0, I2C_SADDR_34, PCA_POLARITY, PCA_DEFAULT_POLARITY );
    SiiPlatformI2cWriteByte(0, I2C_SADDR_34, PCA_CONFIG, 0x00 );                      // 7:0 output
    SiiPlatformI2cWriteByte(0, I2C_SADDR_34, PCA_OUTPUT, PCA_DEFAULT_OUTPUT );

    Pca9557ReadModifyWrite( I2C_SADDR_3A, BIT4 | BIT5 | BIT6 | BIT7, 0x00 );  // '1' disable
#else

    // Configure GPIO PE4 to control the audio DAC reset 
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTE_BASE, GPIO_PIN_4 );
    ROM_GPIOPadConfigSet( GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );

    // SW-2 GPIOs
    SkPlatformGPIOPinTypeGPIOInput(    // Switches 1-6
        GPIO_PORTA_BASE,
        GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,
        GPIO_PIN_TYPE_STD_WPU );   // SW2-7
    SkPlatformGPIOPinTypeGPIOInput( GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_TYPE_STD_WPU );   // Switches 7-8

    // Status LED 3
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTD_BASE, GPIO_PIN_0 );
    ROM_GPIOPadConfigSet( GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );

    // Status LED 4
    ROM_GPIOPinTypeGPIOOutput( GPIO_PORTD_BASE, GPIO_PIN_1 );
    ROM_GPIOPadConfigSet( GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU );

    ROM_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0xFF); // Turn the LEDs off

#endif
#endif 

}


