//***************************************************************************
//!file     sk_app_cbus_wake_sequence.c
//!brief
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "sk_app_cbus.h"
#include "si_timer.h"
#include "si_gpio.h"

//------------------------------------------------------------------------------
// Local constants and type definitions
//------------------------------------------------------------------------------

#define CBUS_SRC_POLLING_W1               20 //
#define CBUS_SRC_WP_WIDTH1                CBUS_SRC_POLLING_W1
#define CBUS_SRC_WP_WIDTH1_MIN          ((CBUS_SRC_WP_WIDTH1 - (CBUS_SRC_WP_WIDTH1 / 20)) - 2) // 5% tolerance
#define CBUS_SRC_WP_WIDTH1_MAX          ((CBUS_SRC_WP_WIDTH1 + (CBUS_SRC_WP_WIDTH1 / 20)) + 2) // 5% tolerance

#define CBUS_SRC_POLLING_W2               60
#define CBUS_SRC_WP_WIDTH2                CBUS_SRC_POLLING_W2
#define CBUS_SRC_WP_WIDTH2_MIN          ((CBUS_SRC_WP_WIDTH2 - (CBUS_SRC_WP_WIDTH2 / 20)) - 2) // 5% tolerance
#define CBUS_SRC_WP_WIDTH2_MAX          ((CBUS_SRC_WP_WIDTH2 + (CBUS_SRC_WP_WIDTH2 / 20)) + 2) // 5% tolerance

#define CBUS_SRC_WP_DISCOVER_MIN            100
#define CBUS_SRC_WP_DISCOVER_MAX            1000

#define CBUS_SRC_WP_POLARITY                1
#define CBUS_SRC_PULSE_MAX                  6

#define CBUS_INST_SWAP_TIMER    1000

enum
{
    CBUS_SRC_WAKE_PULSE,
    CBUS_SRC_DISCOVERY_PULSE,
};

typedef struct _SiiCbusWakeUpData_t
{
    uint8_t cbusPin;
    uint16_t cbusTimeCount;
    uint16_t cbusMax;
    uint16_t cbusMin;
    uint8_t cbusPulse;
    uint8_t cbusState;
} SiiCbusWakeUpData_t;

typedef struct _SiiAppCbusWakeupInstanceData_t
{
    int                 instanceIndex;
    bool_t              wakeDetect;
    SiiCbusWakeUpData_t cbusWakeup;
}   SiiAppCbusWakeupInstanceData_t;


static SiiAppCbusWakeupInstanceData_t appCbusWake[MHL_MAX_CHANNELS];
static SiiAppCbusWakeupInstanceData_t *pAppCbusWake = &appCbusWake[0];

static uint8_t cbusInst = 0;
static SiiTimer_t cbusInstSwitchTimer;       	// used in wake-up detection by switching between instances
static bool_t wake_up_sensed = false;

//------------------------------------------------------------------------------
//! @brief  Reset all the Cbus Wake up Status
//------------------------------------------------------------------------------
static void SkCbusPulseReset()
{
	memset( &pAppCbusWake->cbusWakeup, 0, sizeof(SiiCbusWakeUpData_t));
}

//------------------------------------------------------------------------------
//! @brief  Check the CBUS wakeup pulse status
//------------------------------------------------------------------------------
static bool_t SkCbusWakeupPulseCheck( uint8_t channel )
{
	//cBusPin has changed the state
	if ( pAppCbusWake->cbusWakeup.cbusPin )
	{
		//ok the pulse is in range
		if ((pAppCbusWake->cbusWakeup.cbusTimeCount >= pAppCbusWake->cbusWakeup.cbusMin) &&
		    (pAppCbusWake->cbusWakeup.cbusTimeCount <= pAppCbusWake->cbusWakeup.cbusMax))
		{
			//DEBUG_PRINT(MSG_DBG, "\n**Inst: %0x Pulse : %02X width : %02X\n", channel, pAppCbus->cbusWakeup.cbusPulse, pAppCbus->cbusWakeup.cbusTimeCount);
		    pAppCbusWake->cbusWakeup.cbusPulse++;
		    pAppCbusWake->cbusWakeup.cbusTimeCount=0;

		    if(!wake_up_sensed)
			{
				wake_up_sensed = true;
				//DEBUG_PRINT(MSG_DBG, "\n wake up pulse sensed on cbus channel %0x\n", channel);
			}
		}
		else
		{
			//DEBUG_PRINT(MSG_DBG, "\n**Inst: %0x Pusle Not in Range!! Pulse : %02X width : %02X\n", channel, pAppCbus->cbusWakeup.cbusPulse, pAppCbus->cbusWakeup.cbusTimeCount);
			return false;
		}
	}
	else
	{
		//hi for too long
		//DEBUG_PRINT(MSG_DBG, "\n** Inst: %02X Pulse : %02X width : %02X\n", channel, pAppCbus->cbusWakeup.cbusPulse, pAppCbus->cbusWakeup.cbusTimeCount);
		if (pAppCbusWake->cbusWakeup.cbusTimeCount > pAppCbusWake->cbusWakeup.cbusMax)
		{
		    pAppCbusWake->cbusWakeup.cbusTimeCount=0;
			return false;
		}
		pAppCbusWake->cbusWakeup.cbusTimeCount++;
	}
	return true;
}

//------------------------------------------------------------------------------
//! @brief  If system power is off, check for a wakeup request from the CBUS
//! @note   This function relies on being called at least once per millisecond
//------------------------------------------------------------------------------
#if ( configSII_DEV_953x_PORTING == 1 ) /*fix IAR building issue: Error[Pe147]: declaration is incompatible with "__interwork __softfp bool_t SkAppCbusWakeupSequence(uint8_t)" (declared <SRCREF line=95 file="D:\FreeRTOS_SB\Libraries\AMT_Drivers\sii953x\application\app_cbus\sk_app_cbus.h">at line 95 of "D:\FreeRTOS_SB\Libraries\AMT_Drivers\sii953x\application\app_cbus\sk_app_cbus.h"</SRCREF>)
Error while running C/C++ Compiler*/
bool_t SkAppCbusWakeupSequence( uint8_t powerIsOn )
#else
bool_t SkAppCbusWakeupSequence( bool_t powerIsOn )
#endif 
{
	bool_t cbusPin  = false;
	bool_t results  = false;
	
	if ( powerIsOn )
	{
		wake_up_sensed = false;
		return true;
	}

	if ( SiiOsTimerExpired( cbusInstSwitchTimer ))
	{
		if ( !wake_up_sensed )
		{
			SiiOsTimerSet( &cbusInstSwitchTimer, CBUS_INST_SWAP_TIMER);
			cbusInst++;
			if ( cbusInst == MHL_MAX_CHANNELS )
			{
				cbusInst = 0;
			}
			//DEBUG_PRINT(MSG_DBG, "\n Current Inst:: %0x\n", cbusInst);
		}
	}

	// Can't be called more often than 1ms.
	pAppCbusWake = &appCbusWake[cbusInst];
    if (!SiiPlatformTimerExpired( TIMER_3 ))
    {
        return results;
    }
    SiiPlatformTimerSet( TIMER_3, 1);
//TODO: OEM: Here is where the system GPIO connected to the wakeup pin is read.
//    cbusPin = SiiPlatformCbusWakeupPinGet( cbusInst );

    if ((cbusPin == CBUS_SRC_WP_POLARITY) && !pAppCbusWake->wakeDetect)
    {
        SkCbusPulseReset();
        pAppCbusWake->cbusWakeup.cbusState = CBUS_SRC_WAKE_PULSE;
        return false;
    }

    //DEBUG_PRINT(MSG_DBG, "cbusPin: %0x, pAppCbus->wakeDetect: %0x\n", cbusPin, pAppCbus->wakeDetect);
    pAppCbusWake->wakeDetect = true;
    switch (pAppCbusWake->cbusWakeup.cbusState)
    {
        case CBUS_SRC_WAKE_PULSE:
            if (pAppCbusWake->cbusWakeup.cbusPulse == 3)
            {
                pAppCbusWake->cbusWakeup.cbusMin = CBUS_SRC_WP_WIDTH2_MIN;
                pAppCbusWake->cbusWakeup.cbusMax = CBUS_SRC_WP_WIDTH2_MAX;
            }
            else
            {
                pAppCbusWake->cbusWakeup.cbusMin = CBUS_SRC_WP_WIDTH1_MIN;
                pAppCbusWake->cbusWakeup.cbusMax = CBUS_SRC_WP_WIDTH1_MAX;
            }

            //complement the cbusPin for Odd pulse
            if (pAppCbusWake->cbusWakeup.cbusPulse % 2)
            {
                pAppCbusWake->cbusWakeup.cbusPin = !cbusPin;
            }
            else
            {
                pAppCbusWake->cbusWakeup.cbusPin = cbusPin;
            }

            results = SkCbusWakeupPulseCheck( cbusInst );

            //Number of pulse has been reached.
            if (pAppCbusWake->cbusWakeup.cbusPulse > CBUS_SRC_PULSE_MAX)
            {
                pAppCbusWake->cbusWakeup.cbusState = CBUS_SRC_DISCOVERY_PULSE;
            }
            else
            {
                break;
            }
        case CBUS_SRC_DISCOVERY_PULSE:
            pAppCbusWake->cbusWakeup.cbusMin = CBUS_SRC_WP_DISCOVER_MIN;
            pAppCbusWake->cbusWakeup.cbusMin = CBUS_SRC_WP_DISCOVER_MAX;
            pAppCbusWake->cbusWakeup.cbusState = CBUS_SRC_WAKE_PULSE;
            pAppCbusWake->wakeDetect = false;

            DEBUG_PRINT(MSG_DBG, "\n" );
            DEBUG_PRINT(MSG_DBG, asterisks );
            DEBUG_PRINT(MSG_DBG, "CBUS Power Up by detecting wake up pulse sequence\n");
            DEBUG_PRINT(MSG_DBG, asterisks );

            if ( app.powerState != APP_POWERSTATUS_ON )
            {
                app.powerState = APP_POWERSTATUS_STANDBY_TO_ON;
            }
            SkCbusPulseReset();
            return true;
    }

    if (!results)
    {
        pAppCbusWake->cbusWakeup.cbusState = CBUS_SRC_WAKE_PULSE;
        pAppCbusWake->wakeDetect = false;
        SkCbusPulseReset();
    }
	return false;
}

//------------------------------------------------------------------------------
//! @brief:     Initialize the CBUS wakeup code
//------------------------------------------------------------------------------
bool_t SkAppDeviceInitCbusWakeup ( void )
{

    memset( &appCbusWake[0], 0, sizeof(SiiAppCbusWakeupInstanceData_t) * MHL_MAX_CHANNELS);
    pAppCbusWake = &appCbusWake[0];
    SiiOsTimerSet(&cbusInstSwitchTimer, CBUS_INST_SWAP_TIMER);

    return( true );
}



