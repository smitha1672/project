//***************************************************************************
//!file     si_infrared.c
//!brief    Platform IR driver, ARM (Stellaris) version.
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
#include "si_infrared.h"
#include "si_osal_timer.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_timer.h"
#include "si_gpio.h"

#if (SI_USE_SCHEDULER_IN_HAL)
#include "si_scheduler.h"
#endif

/*
 * 16-Bit Timer With Prescaler Configurations
 * Prescale #Clock(Tc)  Max Time    Units
 * 00000000     1       0.8192      mS
 * 00000001     2       1.6385      mS
 * 00000010     3       2.4576      mS
 * ------------ --      --          --
 * 11111100     254     208.0768    mS
 * 11111110     255     208.896     mS
 * 11111111     256     209.7152    mS
 *
 * Tc is the clock period.
 * All values assume an 80-MHz clock with Tc=12.5 ns (clock period).
 *
 * 16-Bit Timer With Prescaler Configurations
 * Prescale #Clock(Tc)  Max Time    Units
 * 00000000     1       4.096       mS
 * 00000001     2       8.192       mS
 * 00000010     3       16.384      mS
 * ------------ --      --          --
 * 00001111     16      65.536      ms      -- 1us period
 * ------------ --      --          --
 * 11111100     254     1040.384    mS
 * 11111110     255     1044.480    mS
 * 11111111     256     1048.576    mS
 *
 * Tc is the clock period.
 * All values assume an 16-MHz clock with Tc=62.5 ns (clock period).
 *
 */

//-------------------------------------------------------------------------------
// Module data
//-------------------------------------------------------------------------------

InfraredInstanceData_t irData =
{
    false,                          // allowAutoRepeat

    // Key ring buffer

    { 0 },                          // buffer[IR_RING_BUFFER_SIZE];
    0,                              // first;
    0,                              // last;
    0,                              // overflow;
    0xFFFF,                         // lastKey;

    // Interrupt handler data

    IRSTATE_WAIT_FIRST_EDGE,        // decodeState;
    0,                              // bitHalfCount;
    false,                          // firstHalfIsHigh;
    0,                              // rcCode;
    0                               // bitWidthMeasured
};

// System clock in Hertz
static unsigned long gSystemClockHz;
static uint_t gBulkBitWidth;

//------------------------------------------------------------------------------
// Function:    PutRc5Key
// Description: Store the passed command unless the ring buffer is full, in which
//              case the overflow bit is set.
// Parameters:  rcCommand
// Returns:     None
//------------------------------------------------------------------------------

static void PutRc5Key ( uint16_t rcKey )
{
    // If the current buffer entry is valid, the ring buffer is full.

    if ( irData.buffer[ irData.last ] & RC5_VALID_ENTRY )
    {
        return;
    }

    // There is at least one slot in the buffer. If a new
    // key (toggle bit has changed) or we are allowing duplicated
    // keys, stick it in the buffer.
    // To avoid first time uncertainty, RC5 toggle bit check is waived for the very first code

    if ((( rcKey ^ irData.lastKey ) & RC5_TOGGLE_FIELD ) || ( irData.allowAutoRepeat ) ||
         (irData.lastKey == 0xFFFF))
    {
        irData.lastKey              = rcKey;
        irData.buffer[ irData.last] = rcKey |= RC5_VALID_ENTRY;

        // Update entry pointer for next time.

        irData.last = (( irData.last + 1) % IR_RING_BUFFER_SIZE );

#if (SI_USE_SCHEDULER_IN_HAL)
        SiiSchedSendSignal(SI_EVENT_ID_PLATFORM_IR_INT, 0);
#endif
    }
}

//------------------------------------------------------------------------------
// Function:    ReturnToIdle
// Description:
//------------------------------------------------------------------------------

static void ReturnToIdle ( void )
{
    irData.decodeState     = IRSTATE_WAIT_FIRST_EDGE;
    irData.bitHalfCount    = 0;

    ROM_TimerDisable( IR_TIMER_BASE, TIMER_B );

    // Configure the pin as a timer input and restart timer A

    ROM_GPIOPinTypeTimer( GPIO_PORTD_BASE, IR_GPIO_PIN );   // CCP0
    ROM_TimerLoadSet( IR_TIMER_BASE, TIMER_A, 0xFFFF );     // default value
    ROM_TimerIntClear( IR_TIMER_BASE, TIMER_CAPA_EVENT );   // Clear the timer interrupt.
    ROM_TimerEnable( IR_TIMER_BASE, TIMER_A );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInfraRedEdgeTimerIntHandler
// Description: Infrared edge timer interrupt  (TIMER0A)
//------------------------------------------------------------------------------

void SiiPlatformInfraRedEdgeTimerIntHandler ( void )
{
    static uint_t   secondEdge;
    unsigned long   ulInt;

    ulInt = ROM_IntMasterDisable(); // Temporarily turn off interrupts.
    ROM_TimerIntClear( IR_TIMER_BASE, TIMER_CAPA_EVENT );   // Clear the timer interrupt.

    switch ( irData.decodeState )
    {
        case IRSTATE_WAIT_FIRST_EDGE:

            // When we get the first falling edge (remember, the sensor is inverted),
            // restart the timer and wait for the second falling edge.

            ROM_TimerLoadSet( IR_TIMER_BASE, TIMER_A, 0xFFFF );
            irData.decodeState = IRSTATE_MEASURE_START_BIT;
            break;

        case IRSTATE_MEASURE_START_BIT:

            // Second edge has arrived, the timer holds the width of the 'bit'

            secondEdge = (0xFFFF - ROM_TimerValueGet( IR_TIMER_BASE, TIMER_A )) & 0xFFFF;
            if (( secondEdge > IR_MIN_BITPERIOD ) && ( secondEdge < IR_MAX_BITPERIOD ))
            {
                irData.decodeState  = IRSTATE_SAMPLE_BITS;  // Start bit detected, change to sampling mode.
                irData.bitHalfCount = 0;
                irData.rcCode       = 0;
                irData.bitWidthMeasured = gBulkBitWidth + secondEdge / 2;

                // Disable the edge time timer (Timer A) and enable the one-shot
                // timer (Timer B) to start looking at bit states.
                // Since we are now half-way through the second start bit, we want to
                // sample the sensor half-way through the first half of the next bit
                // Also, we need to change the pin type to be able to sample it

                ROM_TimerDisable( IR_TIMER_BASE, TIMER_A );
                ROM_GPIOPinTypeGPIOInput( GPIO_PORTD_BASE, IR_GPIO_PIN );
                ROM_TimerLoadSet(
                        IR_TIMER_BASE, TIMER_B,
                        irData.bitWidthMeasured / 2 + irData.bitWidthMeasured / 4 // 3/4 bit time
                        );
                ROM_IntEnable( IR_INT_TIMERB );
                ROM_TimerEnable( IR_TIMER_BASE, TIMER_B );
            }
            else
            {
                irData.decodeState  = IRSTATE_WAIT_FIRST_EDGE;
                irData.bitHalfCount = 0;
            }
            break;
        case IRSTATE_SAMPLE_BITS:
            break;
        default:
            break;
    }

    if ( !ulInt )   // Re-enable interrupts if needed
    {
        ROM_IntMasterEnable();
    }
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInfraRedSamplingTimerIntHandler
// Description: Infrared bit sampling interrupt (TIMER0B)
//------------------------------------------------------------------------------

void SiiPlatformInfraRedSamplingTimerIntHandler ( void )
{
    uint_t          sensorPin;
    bool_t          secondHalfIsHigh;
    unsigned long   ulInt;

    ulInt = ROM_IntMasterDisable(); // Temporarily turn off interrupts.
    ROM_TimerIntClear( IR_TIMER_BASE, TIMER_TIMB_TIMEOUT ); // Clear the timer interrupt.

    // Read the current state of the pin.
    // From now on, sample every half-bit time, which should put us
    // in the middle of the half-bit in question.

    sensorPin = ROM_GPIOPinRead( GPIO_PORTD_BASE, IR_GPIO_PIN );

    // Kick the timer off for the next bit.

    ROM_TimerLoadSet( IR_TIMER_BASE, TIMER_B, irData.bitWidthMeasured / 2 );   // 1/2 bit time

    switch ( irData.decodeState )
    {
        case IRSTATE_WAIT_FIRST_EDGE:
        case IRSTATE_MEASURE_START_BIT:
            break;

        case IRSTATE_SAMPLE_BITS:

            // Sample the state

            if (( irData.bitHalfCount % 2) == 0 )
            {
                irData.firstHalfIsHigh = (sensorPin == 0);
            }

            // Second half of the bit, validate it and determine if it is 1 or 0

            else
            {
                secondHalfIsHigh = ( sensorPin == 0);

                // Can't both be the same state.  If not, determine the bit state

                if ( irData.firstHalfIsHigh != secondHalfIsHigh )
                {
                    irData.rcCode <<= 1;
                    if ( !irData.firstHalfIsHigh )
                    {
                        irData.rcCode |= 0x01;
                    }
                }
                else
                {
                    ReturnToIdle();
                    break;
                }
            }
            irData.bitHalfCount++;

            // If we've reached the end of the RC code, push the command onto
            // the command stack and go back to waiting for a new start bit.

            if ( irData.bitHalfCount >= IR_BIT_HALF_TOTAL )
            {
                // Put the key into the ring and return to waiting for start bit edge

                ReturnToIdle();
                PutRc5Key( irData.rcCode );
            }
            break;

        default:
            break;
    }

    if ( !ulInt )   // Re-enable interrupts if needed
    {
        ROM_IntMasterEnable();
    }
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInfraRedGetKey
// Description: Return oldest RC5 key value in ring buffer.
// Parameters:  none
// Returns:
//------------------------------------------------------------------------------

uint_t SiiPlatformInfraRedGetKey (void)
{
    uint16_t retKey = RC5_BUFFER_EMPTY;

    // If current buffer data is valid, return it.

    if (( irData.buffer[ irData.first] & RC5_VALID_ENTRY ))
    {
        // Get the key, mark it invalid in the buffer, and update the
        // index for next time.

        retKey = irData.buffer[ irData.first ] & RC5_KEY_MASK;
        irData.buffer[ irData.first ] &= ~RC5_VALID_ENTRY;
        irData.first = (( irData.first + 1) % IR_RING_BUFFER_SIZE );
    }

    return( retKey );
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformInfraRedInit
// Description:
// Parameters:
// Returns:
//------------------------------------------------------------------------------

void SiiPlatformInfraRedInit (void)
{
    // initialize ring buffer
    irData.first        = 0;
    irData.last         = 0;
    irData.overflow     = 0;
    irData.decodeState  = IRSTATE_WAIT_FIRST_EDGE;
    irData.bitHalfCount = 0;
    irData.rcCode       = 0;

    // Determine timings depending on system clock
    gSystemClockHz = ROM_SysCtlClockGet();

    // Pre-calculate fixed part of bit time corresponding to a number of whole cycles that
    // timer A completes before second edge of the measured bit arrives
    gBulkBitWidth = 32768 * (IR_BIT_CLOCKS / 32768);

    // Make sure the tolerance interval has valid boundaries
    // It may happen that the interval falls outside the linear segment of timer A (includes overflow point)
    ASSERT(IR_MAX_BITPERIOD > IR_MIN_BITPERIOD);

    // Initialize the peripherals we'll be using.

    ROM_SysCtlPeripheralEnable( IR_PERIPH_TIMER );
    ROM_SysCtlPeripheralReset( IR_PERIPH_TIMER );
    ROM_GPIOPinTypeTimer( GPIO_PORTD_BASE, IR_GPIO_PIN );
    SiiPlatformGPIOPinConfigure( IR_GPIO_CCP );

    /* Setup TimerA counter in edge-time-capture mode.  */
    /* Setup TimerB counter in periodic mode.           */

    ROM_TimerDisable( IR_TIMER_BASE, TIMER_BOTH );

    // Prescaler reduces timer B clock in half to support system clocks up to 70 MHz
    ROM_TimerPrescaleSet(IR_TIMER_BASE, TIMER_B, 1);

    ROM_TimerConfigure(
            IR_TIMER_BASE,
            TIMER_CFG_16_BIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_PERIODIC );
    ROM_TimerIntEnable( IR_TIMER_BASE, TIMER_CAPA_EVENT | TIMER_TIMB_TIMEOUT );

    // Timer A is the edge time capture timer used to find a start bit.

    MAP_TimerControlEvent( IR_TIMER_BASE, TIMER_A, TIMER_EVENT_NEG_EDGE );
    ROM_TimerLoadSet( IR_TIMER_BASE, TIMER_A, 0xFFFF );         // default value

    // Timer B is the periodic timer used to check bit states after the start bit is found

    ROM_TimerLoadSet( IR_TIMER_BASE, TIMER_B, IR_BIT_CLOCKS );      // Set periodic timeout
    ROM_IntDisable( IR_INT_TIMERB );                                // Start disabled

    ROM_TimerIntClear( IR_TIMER_BASE, TIMER_TIMB_TIMEOUT ); // Clear the timer interrupt.

    // Activate the edge time capture timer to wait for an edge from the sensor

    ROM_IntEnable( IR_INT_TIMERA );
    ROM_TimerEnable( IR_TIMER_BASE, TIMER_A );
}
