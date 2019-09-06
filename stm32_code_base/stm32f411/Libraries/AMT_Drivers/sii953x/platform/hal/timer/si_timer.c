//***************************************************************************
//!file     si_timer.c
//!brief    Platform timer system, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_common.h"
#include "si_platform.h"
#include "si_timer.h"

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
#include "si_arm.h"
#endif 


#if ( configRTOS_TIMER == 1 )
#define MIN_BLOCK_TIME(x)		(x/portTICK_PERIOD_MS)

#endif 

volatile unsigned long g_ulTickCounter = 0;

//------------------------------------------------------------------------------
// Array of timer values
//------------------------------------------------------------------------------

static volatile uint16_t gTimerCounters[ TIMER_COUNT ];

static volatile uint16_t gTimerElapsed;
static volatile uint16_t gElapsedTick;
static volatile uint16_t gTimerElapsedGranularity;

static volatile uint16_t gTimerElapsed1;
static volatile uint16_t gElapsedTick1;
static volatile uint16_t gTimerElapsedGranularity1;

#if ( configRTOS_TIMER == 1 )
xTimerHandle xSiTimer = NULL;
#endif 


//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************

#if ( configRTOS_TIMER == 1 )
void SiiPlatformTimerSysTickIntHandler ( TimerHandle_t pxTimer )

#else
void SiiPlatformTimerSysTickIntHandler (void)
#endif 
{
    int             i;
    unsigned long   ulInt;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
    ulInt = ROM_IntMasterDisable(); // Temporarily turn off interrupts.
#endif 

    g_ulTickCounter++;              // Increment the system tick count.

    // Decrement all active timers in array

    for ( i = 0; i < TIMER_COUNT; i++ )
    {
        if ( gTimerCounters[ i ] > 0 )
        {
            gTimerCounters[ i ]--;
        }
    }
    gElapsedTick++;
    if ( gElapsedTick == gTimerElapsedGranularity )
    {
        gTimerElapsed++;
        gElapsedTick = 0;
    }
    gElapsedTick1++;
    if ( gElapsedTick1 == gTimerElapsedGranularity1 )
    {
        gTimerElapsed1++;
        gElapsedTick1 = 0;
    }

    if ( !ulInt )   // Re-enable interrupts if needed
    {
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956 )
        ROM_IntMasterEnable();
#endif         
    }

}

//------------------------------------------------------------------------------
// Function:    SiiPlatformTimerSysTicksGet
// Description: Returns the current number of system ticks since we started
// Parameters:
// Returns:
//------------------------------------------------------------------------------

clock_time_t SiiPlatformTimerSysTicksGet (void)
{
    return((clock_time_t)g_ulTickCounter);
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformTimerSet
// Description:
//------------------------------------------------------------------------------

void SiiPlatformTimerSet ( uint8_t index, uint16_t m_sec )
{
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_TIMER )
    ROM_SysTickIntDisable();
#endif

    switch ( index )
    {
    case TIMER_0:
    case TIMER_1:
    case TIMER_2:
    case TIMER_3:
    case TIMER_4:
        gTimerCounters[ index ] = m_sec;
        break;

    case ELAPSED_TIMER:
        gTimerElapsedGranularity = m_sec;
        gTimerElapsed = 0;
        gElapsedTick = 0;
        break;
    case ELAPSED_TIMER1:
        gTimerElapsedGranularity1 = m_sec;
        gTimerElapsed1 = 0;
        gElapsedTick1 = 0;
        break;
    }
    
#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_TIMER )
    ROM_SysTickIntEnable();
#endif

}

//------------------------------------------------------------------------------
// Function:    SiiPlatformTimerExpired
// Description: Returns > 0 if specified timer has expired.
//------------------------------------------------------------------------------

uint8_t SiiPlatformTimerExpired ( uint8_t timer )
{
    if ( timer < TIMER_COUNT )
    {
        return( gTimerCounters[ timer ] == 0);
    }

    return( 0 );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformTimerElapsed
// Description: Returns current timer tick.  Rollover depends on the
//              granularity specified in the SetTimer() call.
//------------------------------------------------------------------------------

uint16_t SiiPlatformTimerElapsed ( uint8_t index )
{
    uint16_t elapsedTime;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_TIMER )
    ROM_SysTickIntDisable();
#endif     
    if ( index == ELAPSED_TIMER )
        elapsedTime = gTimerElapsed;
    else
        elapsedTime = gTimerElapsed1;

#if defined ( SII_DEV_953x_PORTING_PLATFORM_LM3S5956_TIMER )
    ROM_SysTickIntEnable();
#endif 

    return( elapsedTime );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformTimerWait
// Description: Waits for the specified number of milliseconds, using timer 0.
//------------------------------------------------------------------------------

void SiiPlatformTimerWait ( uint16_t m_sec )
{
#if (configRTOS_TIMER == 0)
    SiiPlatformTimerSet( 0, m_sec );
    while ( !SiiPlatformTimerExpired( 0 ) )
    {
        ;  //empty wait loop
    }
#else
	vTaskDelay( TASK_MSEC2TICKS(m_sec) );	
#endif 
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformTimerInit
// Description:
// Parameters:
// Returns:
//------------------------------------------------------------------------------
 

void SiiPlatformTimerInit (void)
{
    int i;

    //initializer timer counters in array

    for ( i = 0; i < TIMER_COUNT; i++ )
    {
        gTimerCounters[ i ] = 0;
    }
    gTimerElapsed  = 0;
    gTimerElapsed1 = 0;
    gElapsedTick   = 0;
    gElapsedTick1  = 0;
    gTimerElapsedGranularity   = 0;
    gTimerElapsedGranularity1  = 0;

    // Configure SysTick for a 1ms periodic interrupt.
#if ( configSII_DEV_953x_PORTING == 1 ) && ( configRTOS_TIMER == 1 )
	xSiTimer = xTimerCreate( ( const signed char * ) "si_timer", MIN_BLOCK_TIME(1), pdTRUE, NULL, SiiPlatformTimerSysTickIntHandler );
	if ( xSiTimer != NULL )
	{
		xTimerStart( xSiTimer, ( portTickType ) 0 );
	}
#else
	ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKHZ);
	ROM_SysTickEnable();
	ROM_SysTickIntEnable();
#endif 

}

