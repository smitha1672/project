//***************************************************************************
//!file     sk_app_emu_aud.c
//!brief    Built-in diagnostics manager
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_application.h"
#include "si_drv_internal.h"
#include "si_regs_audio953x.h"
#include "si_regs_rxedid953x.h"
#include "si_gpio.h"

#include "inc/hw_gpio.h"
#include "si_platform.h"
#include "si_i2c.h"
#include "si_audio_dac.h"
#undef GPIO_LOCK_KEY
#include "si_arm.h"
#include "si_drv_audio.h"
#include "si_idt_5v9885t.h"


uint16_t ChInt1;
uint16_t ChInt2;
uint16_t ChInt3;
uint16_t nHVAL1;
uint16_t nHVAL2;
uint16_t nHVAL3;
uint16_t ctsHVAL1;
uint16_t ctsHVAL2;
uint16_t ctsHVAL3;
uint16_t audioFifo;
uint16_t chStatus1;
uint16_t chStatus4;
uint16_t chStatus5;

uint16_t i2sCtrl1;
uint16_t i2sCtrl2;
uint16_t audioCtrl;
uint16_t audioMute;

//------------------------------------------------------------------------------
// Function:    SkAudioPage0
// Description: 
// Returns:
//------------------------------------------------------------------------------

void SkAudioPage0()
{

	ChInt1 = REG_AUDIO_INTR11;
	ChInt2 = REG_AUDIOAUD_INTR12;
	ChInt3 = REG_AUDIOAUD_INTR13;

	nHVAL1 = REG_N_HVAL1_P0;
	nHVAL2 = REG_N_HVAL2_P0;
	nHVAL3 = REG_N_HVAL3_P0;
	ctsHVAL1 = REG_CTS_HVAL1_P0;
	ctsHVAL2 = REG_CTS_HVAL2_P0;
	ctsHVAL3 = REG_CTS_HVAL3_P0;
	audioFifo = REG_AUDP0_FIFO;

	chStatus1 = REG_CHST1_P0;
	chStatus4 = REG_AUDIO_FS;
	chStatus5 = REG_CHST5_P0;

	i2sCtrl1 = REG_AUDIO_I2S_CTRL1;     //0x26
	i2sCtrl2 = REG_AUDIO_I2S_CTRL2;     //0x27
	audioCtrl = REG_AUDIOAUD_CTRL;      //0x29
	audioMute = REG_AUDIOHDMI_MUTE;     //0x37

}

//------------------------------------------------------------------------------
// Function:    SkAudioPage1
// Description: 
// Returns:
//------------------------------------------------------------------------------
void SkAudioPage1()
{
	ChInt1 = REG_CH0_INTR14;
	ChInt2 = REG_CH0_INTR15;
	ChInt3 = REG_CH0_INTR16;

}

//------------------------------------------------------------------------------
// Function:    SkAudioPageSet
// Description: 
// Returns:
//------------------------------------------------------------------------------
void SkAudioPageSet(uint8_t chn)
{
		SkAudioPage0();
}

//------------------------------------------------------------------------------
// Function:    SkAudioDisplayStatus
// Description: 
// Returns:
//------------------------------------------------------------------------------
void SkAudioDisplayStatus(void)
{
	uint8_t val;
	uint8_t FIFOtemp;
	//uint8_t Status1Reg;

	SkAudioPage0();
	
  	DEBUG_PRINT(MSG_DBG, "\n************ Audio Status **********");

    // CTS HW 
    uint32_t CTStemp = SiiRegRead(ctsHVAL3);
    CTStemp <<= 8;
    CTStemp |= SiiRegRead(ctsHVAL2);
    CTStemp <<= 8;
    CTStemp |= SiiRegRead(ctsHVAL1);
    DEBUG_PRINT(MSG_DBG, "\nCTS Val: %d --", CTStemp);

    // N Value 
    uint32_t Nvaluetemp = SiiRegRead(nHVAL3);
    Nvaluetemp <<= 8;
    Nvaluetemp |= SiiRegRead(nHVAL2);
    Nvaluetemp <<= 8;
    Nvaluetemp |= SiiRegRead(nHVAL1);
    DEBUG_PRINT(MSG_DBG, "\nN Val: %d", Nvaluetemp);


    // Audio Out SPDIF Channel Status #4 Register (page 1)
    //
    // Sampling frequency:
    // "0  1  0  0" - Fs = 22.05 kHz
    // "0  0  0  0" - Fs = 44.1 kHz
    // "1  0  0  0" - Fs = 88.2 kHz
    // "1  1  0  0" - Fs = 176.4 kHz
    // "0  1  1  0" - Fs = 24 kHz
    // "0  0  1  0" - Fs = 48 kHz
    // "1  0  1  0" - Fs = 96 kHz
    // "1  1  1  0" - Fs = 192 kHz
    // "0  0  1  1" - Fs = 32 kHz
    // "1  0  0  1" - Fs = 768 kHz
    // "0  0  0  1" Sampling frequency not indicated

    val = SiiRegRead(chStatus4);
	DEBUG_PRINT(MSG_DBG, "\nFs: ");

    switch (val & 0x0F)
    {
        case 0x04: DEBUG_PRINT(MSG_DBG, "   22.05 kHz"); break;
        case 0x00: DEBUG_PRINT(MSG_DBG, "   44.1 kHz"); break;
        case 0x08: DEBUG_PRINT(MSG_DBG, "   88.2 kHz"); break;
        case 0x09: DEBUG_PRINT(MSG_DBG, "   768 kHz"); break;
        case 0x0C: DEBUG_PRINT(MSG_DBG, "   176.4 kHz"); break;
        case 0x06: DEBUG_PRINT(MSG_DBG, "   24 kHz"); break;
        case 0x02: DEBUG_PRINT(MSG_DBG, "   48 kHz"); break;
        case 0x0A: DEBUG_PRINT(MSG_DBG, "   96 kHz"); break;
        case 0x0E: DEBUG_PRINT(MSG_DBG, "   192 kHz"); break;
        case 0x03: DEBUG_PRINT(MSG_DBG, "   32 kHz"); break;
        case 0x01: DEBUG_PRINT(MSG_DBG, "  Sampling frequency not indicated"); break;
    }

#if 0
    if ((val & 0x30) == 0x00)
        DEBUG_PRINT(MSG_DBG, "   Level II");
    else if ((val & 0x30) == 0x02)
        DEBUG_PRINT(MSG_DBG, "   Level I");
    else if ((val & 0x30) == 0x01)
        DEBUG_PRINT(MSG_DBG, "   Level III");




    // Audio Out SPDIF Channel Status #5 Register (page 1)
    //          max 24                          max 20
    // "000"    not indicated (default)        not indicated (default)
    // "001"    20 bits                         16 bits
    // "010"    22 bits                         18 bits
    // "100"    23 bits                         19 bits
    // "101"    24 bits                         20 bits
    // "110"    21 bits                         17 bits

    val = SiiRegRead(chStatus5);
	DEBUG_PRINT(MSG_DBG, "\n** SPDIF Channel: ");

    if ((val & 0x01) == 0x01)
    {
    	DEBUG_PRINT(MSG_DBG, "   24 Bit Max -- ");
        switch (val >> 1)
        {
            case 0x00: DEBUG_PRINT(MSG_DBG, "   not indicated (default)"); break;
            case 0x01: DEBUG_PRINT(MSG_DBG, "   20 bits"); break;
            case 0x02: DEBUG_PRINT(MSG_DBG, "   22 bits"); break;
            case 0x04: DEBUG_PRINT(MSG_DBG, "   23 bits"); break;
            case 0x05: DEBUG_PRINT(MSG_DBG, "   24 bits"); break;
            case 0x06: DEBUG_PRINT(MSG_DBG, "   21 bits"); break;
        }
    }
    else
    {
        DEBUG_PRINT(MSG_DBG, "   20 Bit Max -- ");
        switch (val >> 1)
        {
            case 0x00: DEBUG_PRINT(MSG_DBG, "   not indicated (default)"); break;
            case 0x01: DEBUG_PRINT(MSG_DBG, "   16 bits"); break;
            case 0x02: DEBUG_PRINT(MSG_DBG, "   18 bits"); break;
            case 0x04: DEBUG_PRINT(MSG_DBG, "   19 bits"); break;
            case 0x05: DEBUG_PRINT(MSG_DBG, "   20 bits"); break;
            case 0x06: DEBUG_PRINT(MSG_DBG, "   17 bits"); break;
        }
    }


        // Audio Out SPDIF Channel Status #1 Register (page 1)
    Status1Reg = SiiRegRead(chStatus1);

    if (Status1Reg & 0x01)
        DEBUG_PRINT(MSG_DBG, "\n** Professional / ");
    else
       DEBUG_PRINT(MSG_DBG, "\n** Consumer / ");

    if (Status1Reg & 0x02)
       DEBUG_PRINT(MSG_DBG, " Used for other purposes\n");
    else
       DEBUG_PRINT(MSG_DBG, " Represent PCM samples\n");
#endif

    uint8_t    chn = SiiRegRead(REG_AUDIOAUDP_STAT);
    if ((chn >> 3) & 0x03)
        DEBUG_PRINT(MSG_DBG, "\nLayout 1 -- multi-channel");
	else
        DEBUG_PRINT(MSG_DBG, "\nLayout 0 -- 2 channel");
		
    if (chn & 0x20)
        DEBUG_PRINT(MSG_DBG, "\nDSD on");
    else if (chn & 0x40)
        DEBUG_PRINT(MSG_DBG, "\nHBR on");
    else 
        DEBUG_PRINT(MSG_DBG, "\nPCM on");
      // FIFO Diff Ptr 
    FIFOtemp = SiiRegRead(audioFifo) & (uint8_t)0x7F;
    DEBUG_PRINT(MSG_DBG, "\nAudio FIFO: %x", FIFOtemp);
  
}




//------------------------------------------------------------------------------
// Function:    SkEmuEdid
// Description: 
// Returns:
//------------------------------------------------------------------------------
void SkI2SAudioGet()
{

    uint8_t temp = SiiRegRead(audioMute);

    DEBUG_PRINT(MSG_DBG, "\n**Audio Mute: %s", (temp & 0x02) ? "Yes" : "No");

    temp = SiiRegRead(audioCtrl);
    //DEBUG_PRINT(MSG_DBG, "\n----- Audio Control (0xF48/0xFC8)-------");
    DEBUG_PRINT(MSG_DBG, "\n   SPDIF Enable: %d", (temp & 0x01) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   SPDIF Mode: %s", (temp & 0x02) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   I2S Mode: %d", (temp & 0x04) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   Pass Audio Error: %d", (temp & 0x08) ? 1 : 0);  
    DEBUG_PRINT(MSG_DBG, "\n   Pass Audio Error: %d", (temp & 0x08) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   Pass SPDIF Error: %d", (temp & 0x10) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   Smooth HW Mute En: %d", (temp & 0x10) ? 1 : 0);

    temp = SiiRegRead(i2sCtrl1);
    DEBUG_PRINT(MSG_DBG, "\n----- I2S CTRL 1 (0xF45/0xFC5)-------");
    DEBUG_PRINT(MSG_DBG, "\n   WS TO SD: 1st bit shift: %d", (temp & 0x01) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   0: MSB -- 1 LSB first: %d", (temp & 0x02) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   SD Left/Right Justify: %d", (temp & 0x04) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   WS Left/Right Polarity: %d", (temp & 0x08) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   MSB Extended: %d", (temp & 0x10) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   Word Size: %d", (temp & 0x20) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   Clock Edge: %d", (temp & 0x40) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   Invalid Enabled: %d", (temp & 0x80) ? 1 : 0);


    temp = SiiRegRead(i2sCtrl2);
    DEBUG_PRINT(MSG_DBG, "\n----- I2S CTRL 1 (0xF46/0xFC6)-------");
    DEBUG_PRINT(MSG_DBG, "\n   PCM Only: %d", (temp & 0x01) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   VUCP: %d", (temp & 0x02));
    DEBUG_PRINT(MSG_DBG, "\n   Master Clock Enable: %d", (temp & 0x08)  ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   SD0: %d", (temp & 0x10) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   SD1: %d", (temp & 0x20) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   SD2: %d", (temp & 0x40) ? 1 : 0);
    DEBUG_PRINT(MSG_DBG, "\n   SD3: %d", (temp & 0x80) ? 1 : 0);
}



//------------------------------------------------------------------------------
// Function:    SkEmuEdid
// Description: 
// Returns:
//------------------------------------------------------------------------------
void  SkEmuAudio()
{
	uint8_t key;
	uint8_t cnt, i;	
	uint8_t intStatus[3] = { 0xFF, 0xFF, 0xFF};

	while (1)
	{
		        
    	DEBUG_PRINT( MSG_ALWAYS, "\nAudio Menu\n" );
        DEBUG_PRINT( MSG_ALWAYS, "--------------\n" );
        DEBUG_PRINT( MSG_ALWAYS, ccDisplayThisMenuMsg );
        DEBUG_PRINT( MSG_ALWAYS, returnToMainDemoMsg );
        DEBUG_PRINT( MSG_ALWAYS, "1 - Display Main Port  Audio Configuration I2S-SPDIF \n" );
        DEBUG_PRINT( MSG_ALWAYS, "2 - Display Sub Audio Configuration I2S-SPDIF \n" );
        DEBUG_PRINT( MSG_ALWAYS, "3 - Set Audio DAC Hi\n" );
        DEBUG_PRINT( MSG_ALWAYS, "4 - Set Audio DAC Lo\n" );
        DEBUG_PRINT( MSG_ALWAYS, "5 - Configure Audio Sub Pipe \n" );
        DEBUG_PRINT( MSG_ALWAYS, "6 - Polling Interrupt Main Pipe & clear it.\n" );
        DEBUG_PRINT( MSG_ALWAYS, "7 - Polling Interrupt Sub Pipe & clear it.\n" );
        DEBUG_PRINT( MSG_ALWAYS, "8 - Program PLL for audio extraction sub pipe.\n" );

        
		key = SkAppIrGetSingleDigit( 9 );

	    switch ( key )
	    {
	        case RC5_0:
	            return;
	            
	        case RC5_1:
	        	SiiDrvRxAudioInstanceSet(0);
	        	SkAudioDisplayStatus();
	            break;
	        case RC5_2:
	        	SiiDrvRxAudioInstanceSet(1);
	        	SkAudioDisplayStatus();
	            break;
	        case RC5_3:
        		ROM_GPIOPinWrite( GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4);
	            break;
	        case RC5_4:
        		ROM_GPIOPinWrite( GPIO_PORTE_BASE, GPIO_PIN_4, 0);
	            break;
	        case RC5_5:
	            DEBUG_PRINT( MSG_ALWAYS, "\n Press key # [0-3]\n" );
	        	key = SkAppIrGetSingleDigit( 3 );
	        	app.newSource[1] = key; // Select port key into sub pipe (Zone2)
				SiiPlatformGpioAudioInOut(true);

				SiiDrvRxAudioInstanceSet(0);
				break;
	        case RC5_6:
	        	cnt = SiiRegRead(REG_SCRATCH0);
                SiiDrvRxAudioInstanceSet(0);
	        	for (i=0; i < cnt; i++)
	        	{
		        	SiiDrvRxAudioGetIntStatus(intStatus);
	        		DEBUG_PRINT( MSG_ALWAYS, "\nValue 0x%x", intStatus[0]);		        	
	        		DEBUG_PRINT( MSG_ALWAYS, "\nValue 0x%x", intStatus[0]);
	        		DEBUG_PRINT( MSG_ALWAYS, "\nValue 0x%x", intStatus[1]);
	        		DEBUG_PRINT( MSG_ALWAYS, "\nValue 0x%x", intStatus[2]);
		        	SiiDrvRxAudioClearIntStatus(intStatus);
		        	SiiPlatformTimerWait( 100 );
	        	}
				break;
	        case RC5_7:
	        	cnt = SiiRegRead(REG_SCRATCH0);
                SiiDrvRxAudioInstanceSet(1);
	        	for (i=0; i < cnt; i++)
	        	{
		        	SiiDrvRxAudioGetIntStatus(intStatus );
	        		DEBUG_PRINT( MSG_ALWAYS, "\nValue 0x%x", intStatus[0]);
	        		DEBUG_PRINT( MSG_ALWAYS, "\nValue 0x%x", intStatus[1]);
	        		DEBUG_PRINT( MSG_ALWAYS, "\nValue 0x%x", intStatus[2]);
		        	SiiDrvRxAudioClearIntStatus(intStatus );
		        	SiiPlatformTimerWait( 100 );
	        	}
				break;
	        case RC5_8:
	            DEBUG_PRINT( MSG_ALWAYS, "0: Fs 32  Khz\n" );	        	
	            DEBUG_PRINT( MSG_ALWAYS, "1: Fs 44.1 Khz\n" );	        	
	            DEBUG_PRINT( MSG_ALWAYS, "2: Fs 48  Khz\n" );	        	
	            DEBUG_PRINT( MSG_ALWAYS, "3: Fs 88.2 Khz\n" );	        	
	            DEBUG_PRINT( MSG_ALWAYS, "4: Fs 96  Khz\n" );	        	
	            DEBUG_PRINT( MSG_ALWAYS, "5: Fs 176.4 Khz\n" );	        	
	            DEBUG_PRINT( MSG_ALWAYS, "6: Fs 196  Khz\n" );	        	
	        	key = SkAppIrGetSingleDigit( 6 );
				SiiPlatformExtnalAudioPllSetting(SII_AUD_MCLK_256, key);
				break;
	        case RC5_9:
#if 0	        	
	        	while(1)
	        	{
		            DEBUG_PRINT( MSG_ALWAYS, "0: Exit\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "1: Port 0 Vbus Enable\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "2: Port 0 Vbus Disable\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "3: Port 0 Rpwr Enable\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "4: Port 0 Rpwr Disable\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "5: Port 5 Vbus Enable\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "6: Port 5 Vbus Disable\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "7: Port 5 Rpwr Enable\n" );	        	
		            DEBUG_PRINT( MSG_ALWAYS, "8: Port 5 Rpwr Disable\n" );	        	
		        	key = SkAppIrGetSingleDigit( 9 );
		        	switch (key)
		        	{
		        		case 0:
		        			return;
						case 1:
							SiiPlatformGpioVbusCtrl(MHL_RPWR_EN0, ON);
							break;
						case 2:
							SiiPlatformGpioVbusCtrl(MHL_RPWR_EN0, OFF);
							break;
						case 3:
							SiiPlatformGpioRpwr(MHL_RPWR_EN0, ON);
							break;
						case 4:
							SiiPlatformGpioRpwr(MHL_RPWR_EN0, OFF);
							break;
						case 5:
							SiiPlatformGpioVbusCtrl(MHL_RPWR_EN5, ON);
							break;
						case 6:
							SiiPlatformGpioVbusCtrl(MHL_RPWR_EN5, OFF);
							break;
						case 7:
							SiiPlatformGpioRpwr(MHL_RPWR_EN5, ON);
							break;
						case 8:
							SiiPlatformGpioRpwr(MHL_RPWR_EN5, OFF);
							break;
		        	}
	        	}
				break;
#endif				
	        default:
	            break;
	    }
	}	    
}
