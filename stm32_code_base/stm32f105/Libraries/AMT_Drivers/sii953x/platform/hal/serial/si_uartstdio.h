//*****************************************************************************
//
// uartstdio.h - Prototypes for the UART console functions.
//
// Copyright (c) 2007-2010 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 5570 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

#ifndef __SI_UARTSTDIO_H__
#define __SI_UARTSTDIO_H__
#include "inc/hw_types.h"

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#define UART_BUFFERED

//*****************************************************************************
//
// If built for buffered operation, the following labels define the sizes of
// the transmit and receive buffers respectively.
//
//*****************************************************************************
#ifdef UART_BUFFERED
#ifndef UART_RX_BUFFER_SIZE
#define UART_RX_BUFFER_SIZE     128
#endif
#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE     4096
#endif
#endif

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
void            SiiPlatformUartInit(unsigned long ulPortNum, unsigned long ulBaud);
void            SiiPlatformUartExpClkInit(unsigned long ulPort, unsigned long ulBaud);
int             SiiPlatformUartGets(char *pcBuf, unsigned long ulLen);
void            SiiPlatformUartPutc( char outChar );
unsigned char   SiiPlatformUartGetc(void);
void            SiiPlatformUartPrintf(const char *pcString, ...);
int             SiiPlatformUartWrite(const char *pcBuf, unsigned long ulLen);
void            SiiPlatformUartCrlfFilterSet ( bool_t enable );
#ifdef UART_BUFFERED
void            SiiPlatformUartFlushTx(tBoolean bDiscard);
void            SiiPlatformUartFlushRx(void);
int             SiiPlatformUartRxBytesAvail(void);
int             SiiPlatformUartTxBytesFreeGet(void);
void            SiiPlatformUartPutcUnfiltered ( char outChar );
#endif

//------------------------------------------------------------------------------
//  UART h/w interrupt handler
//------------------------------------------------------------------------------

void            SiiPlatformUartIntHandler(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __SI_UARTSTDIO_H__
