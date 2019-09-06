//***************************************************************************
//!file     si_gpio.h
//!brief    Platform GPIO driver, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_GPIO_H__
#define __SI_GPIO_H__

    /* Define the settling time, in 100ms intervals, */
    /* of each position on the rotary switch.       */

#define DEBOUNCE_0      (200)
#define DEBOUNCE_1      (200)
#define DEBOUNCE_2      (200)
#define DEBOUNCE_3      (200)
#define DEBOUNCE_4      (500)
#define DEBOUNCE_5      (500)
#define DEBOUNCE_6      (500)
#define DEBOUNCE_7      (500)
#define DEBOUNCE_8      (500)
#define DEBOUNCE_9      (500)
#define DEBOUNCE_10     (500)
#define DEBOUNCE_11     (500)
#define DEBOUNCE_12     (500)
#define DEBOUNCE_13     (500)
#define DEBOUNCE_14     (500)
#define DEBOUNCE_15     (500)

    /* Rotary switch values.    */

#define RSW_POS_0           0x00
#define RSW_POS_1           0x01
#define RSW_POS_2           0x02
#define RSW_POS_3           0x03
#define RSW_POS_4           0x04
#define RSW_POS_5           0x05
#define RSW_POS_6           0x06
#define RSW_POS_7           0x07
#define RSW_NO_CHANGE       0xFF    // Rotary switch has not changed

#define LED_STATUS_MASK    0x3F    // led mask status
#define GPIO_ROTARY_SWITCH_MASK         0xE0
#define GPIO_ROTARY_SWITCH_XOR          0x00    // No invert
#define GPIO_ROTARY_SWITCH_SHIFT        5
#define GPIO_ROTARY_SWITCH_BASE         GPIO_PORTB_BASE

// GPIO Port and Pin hooked to External H/W interrupt line
#if (FPGA_BUILD == DISABLE)
#   define GPIO_INT_PORT_BASE   GPIO_PORTD_BASE
#   define GPIO_INT_PIN_IDX     GPIO_PIN_0
#   define GPIO_INT_PORT        INT_GPIOD
#else
#   define GPIO_INT_PORT_BASE   GPIO_PORTE_BASE
#   define GPIO_INT_PIN_IDX     GPIO_PIN_2
#   define GPIO_INT_PORT        INT_GPIOE
#endif

// SkPlatformGpioRpwrOn
// SkPlatformGpioRpwrOff
enum
{
    MHL_RPWR_EN0,   // RPWR on port 0
    MHL_RPWR_EN1,   // RPWR on port 1
    MHL_RPWR_EN2,   // RPWR on port 2
    MHL_RPWR_EN3,   // RPWR on port 3
};

enum
{
    MHL_CHNL_0 = 0,
    MHL_CHNL_1,
};

enum
{
    CEC_A_MAIN,
    CEC_A_SUB,
};

enum
{
    CEC_PORT_0=0x01,
    CEC_PORT_1=0x02,
    CEC_PORT_2=0x04,
    CEC_PORT_3=0x08,
};

typedef enum _ledState_t
{
    LED_ON      = 0x00,
    LED_OFF     = 0xFF
} LedState_t;

#if (FPGA_BUILD == DISABLE)
typedef enum _GpioSw_t
{
    // Upper nibble is the read function switch case, which chooses the GPIO bank
    // Lower nibble is the bit position in the selected GPIO bank
    SW2_1   = 0x1001,   // U52-0
    SW2_2   = 0x1002,   // U52-1
    SW2_3   = 0x1004,   // U52-2
    SW2_4   = 0x1008,   // U52-3
    SW2_5   = 0x1010,   // U52-4
    SW2_6   = 0x1020,   // U52-5
    SW2_7   = 0x1040,   // U52-6
} GpioSw_t;
#else
typedef enum _GpioSw_t
{
    // Upper nibble is the read function switch case, which chooses the GPIO bank
    // Lower nibble is the bit position in the selected GPIO bank

    SW2_1   = 0x0004,     // PA_2
    SW2_2   = 0x0008,     // PA_3
    SW2_3   = 0x0010,     // PA_4
    SW2_4   = 0x0020,     // PA_5
    SW2_5   = 0x0040,     // PA_6
    SW2_6   = 0x0080,     // PA_7
    SW2_7   = 0x0210,     // PC_4
    SW2_8   = 0x0220,     // PC_5
} GpioSw_t;
#endif

typedef enum _Pca5597_devId_t
{
        I2C_SADDR_38   = 0x38,     // U55 device I2C ID
        I2C_SADDR_3A   = 0x3A,     // U53 device I2C ID
        I2C_SADDR_3F   = 0x3F,     // U52 device I2C ID
        I2C_SADDR_34   = 0x34,     // U79 device I2C ID
} Pca5597_devId_t;

#define PCA_DEFAULT_POLARITY    0x00    // All GPIO inputs non-inverted
#define PCA_DEFAULT_OUTPUT      0x00    // All GPIO outputs low by default

typedef enum _Pca5597_regs_t
{
    PCA_INPUT       = 0,
    PCA_OUTPUT,
    PCA_POLARITY,
    PCA_CONFIG
} Pca5597_regs_t;


enum
{
    SII_AUD_IN,
    SII_AUD_OUT,
};

enum
{
    SII_GPIO_AON_POWER,
    SII_GPIO_PDN_POWER,
    SII_GPIO_POWER_ALL,
};

enum
{
    SII_GPIO_CEC_SELECT_0,
    SII_GPIO_CEC_SELECT_1,
    SII_GPIO_CEC_SELECT_2,
    SII_GPIO_CEC_SELECT_3,
    SII_GPIO_CEC_SELECT_4,
    SII_GPIO_CEC_SELECT_5,
};
enum
{
    SII_GPIO_CEC_AM,
    SII_GPIO_CEC_AS,
};


//------------------------------------------------------------------------------
//  GPIO functions
//------------------------------------------------------------------------------

void    SiiPlatformGpioInit(void);
bool_t  SiiPlatformGpioVbusCtrl ( uint8_t vbusPort, uint8_t ctrl );
bool_t  SiiPlatformGpioRpwr ( uint8_t rpwrPort, uint8_t ctrl );

void    SiiPlatformGpioLedPorts ( uint8_t portBits );
void    SiiPlatformGpioLedStatus1( bool_t ledOn );
void    SiiPlatformGpioLedStatus2( bool_t ledOn );
void    SiiPlatformGpioLedArc( bool_t ledOn );
void    SiiPlatformGpioLedHec( bool_t ledOn );

bool_t  SiiPlatformGpioReadSwitch( GpioSw_t gpioSw );

void    SiiPlatformGpioSetPower (uint8_t pwrType, uint8_t pwrCtrl );

void    SiiPlatformGPIOPinConfigure ( unsigned long ulPinConfig );
void    SiiPlatformGpioAudioInOut ( uint8_t audType );
uint8_t SiiPlatformGpioPowerVCC5 (void );
void    SiiPlatformGpioCecSelect (uint8_t cecSel, uint8_t ctrl );
bool_t  SiiPlatformMHLCDSensePinGet( uint8_t mhlChnl );

#endif	// __SI_GPIO_H__
