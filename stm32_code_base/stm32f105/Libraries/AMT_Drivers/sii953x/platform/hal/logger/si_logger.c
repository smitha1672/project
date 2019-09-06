//***************************************************************************
//!file     si_logger.c
//!brief    Silicon Image Starter Kit debug print message interface
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "stdarg.h"
#include "string.h"
#include "si_common.h"
#include "si_platform.h"
#include "si_timer.h"

#include "si_uartstdio.h"

#define MAX_OUTPUT_STR_LEN  400

static const char *dbgMsgComponentNames [] =
{
    "BRD ",
    "DEV ",

    "CBUS",
    "CDC ",
    "CEC ",
    "CPI ",
    "EDID",
    "HEAC",
    "RPTR",
    "SWCH",
    "VSIF",
    "TX",
    "EDID TX"
};

static const char *const g_pcHex        = "0123456789abcdef";
static const char *const g_pcHexCapital = "0123456789ABCDEF";
static bool_t colorEnable = false;
static char outString[MAX_OUTPUT_STR_LEN];     // SkPrintf internal buffer


void SiiPlatformLoggerColorEnable ( bool_t enable )
{
    colorEnable = enable;
}


//------------------------------------------------------------------------------
// Function:    ___sprintf
// Description: non-library version of sprintf that is passed a pointer to the
//              first entry of the caller's variable argument list.
// Parameters:
// Returns:     length of string copied to the passed buffer, not including
//              the terminating 0
//------------------------------------------------------------------------------

unsigned long ___sprintf ( char *pOutString, const char *pcString, void *pArgs )
{
    unsigned long   outIndex, ulIdx, ulValue, ulPos, ulCount, ulBase, ulNeg;
    char            *pcStr, pcBuf[16], cFill, switchChar;
    bool_t          leftJustified;
    va_list         args = (va_list)pArgs;

    // Loop while there are more characters in the string.
    outIndex = 0;
    while(*pcString)
    {
        // Find the first non-% character, or the end of the string.
        for (ulIdx = 0; (pcString[ulIdx] != '%') && (pcString[ulIdx] != '\0');
            ulIdx++)
        {
        }

        // Write this portion of the string.
        strncpy( &pOutString[outIndex], pcString, ulIdx );
        outIndex += ulIdx;
        pcString += ulIdx;

        if (*pcString == '%')
        {
            pcString++;     // Skip the %.

            // Set the digit count to zero, and the fill character to space
            // (i.e. to the defaults).
            leftJustified = false;
            ulCount = 0;
            cFill = ' ';

            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.

again:
            // Determine how to handle the next character.
            switchChar = *pcString++;
            switch ( switchChar )
            {
                //
                // Handle the digit characters.
                //
                case '-':                                   // Left justified
                    leftJustified = true;
                    goto again;                             // Get the next format character.
                    break;

                case '*':                                   // Get fill count from parameter list
                    ulCount = va_arg(args, unsigned long);  // Get the value from the varargs.
                    goto again;                             // Get the next format character.
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.

                    if ((pcString[-1] == '0') && (ulCount == 0))
                    {
                        cFill = '0';
                    }

                    // Update the digit count.

                    ulCount *= 10;
                    ulCount += pcString[-1] - '0';

                    goto again;                             // Get the next format character.
                }

                case 'c':                                       // Handle the %c command.
                {
                    ulValue = va_arg(args, unsigned long);      // Get the value from the varargs.
                    pOutString[ outIndex++] = (char)ulValue;    // Print out the character.
                    break;                                      // This command has been handled.
                }

                case 'd':                                       // Handle the %d command.
                {
                    ulValue = va_arg(args, unsigned long);      // Get the value from the varargs.
                    ulPos = 0;                                  // Reset the buffer position.

                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.

                    if ((long)ulValue < 0)
                    {
                        ulValue = -(long)ulValue;               // Make the value positive.
                        ulNeg = 1;                              // Indicate that the value is negative.
                    }
                    else
                    {
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.

                        ulNeg = 0;
                    }

                    ulBase = 10;        // Set the base to 10.
                    goto convert;       // Convert the value to ASCII.
                }

                case 's':                                       // Handle the %s command.
                {
                    pcStr = va_arg(args, char *);               // Get the string pointer from the varargs.

                    // Determine the length of the string.

                    for (ulIdx = 0; pcStr[ulIdx] != '\0'; ulIdx++)
                    {
                    }

                    // Write any required padding spaces (right justified)

                    if ( !leftJustified && (ulCount > ulIdx))
                    {
                        ulCount -= ulIdx;
                        while (ulCount--)
                        {
                            pOutString[ outIndex++] = ' ';
                        }
                    }

                    // Write the string.

                    strncpy( &pOutString[ outIndex], pcStr, ulIdx);
                    outIndex += ulIdx;

                    // Write any required padding spaces (left justified)

                    if ( leftJustified && (ulCount > ulIdx))
                    {
                        ulCount -= ulIdx;
                        while (ulCount--)
                        {
                            pOutString[ outIndex++] = ' ';
                        }
                    }
                    break;                                      // This command has been handled.
                }

                case 'u':                                       // Handle the %u command.
                {
                    ulValue = va_arg(args, unsigned long);      // Get the value from the varargs.
                    ulPos = 0;                                  // Reset the buffer position.
                    ulBase = 10;                                // Set the base to 10.

                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.

                    ulNeg = 0;

                    goto convert;           // Convert the value to ASCII.
                }

                // Handle the %x and %X commands.  Note that we also
                // alias %p to %X.

                case 'x':
                case 'X':
                case 'p':
                {
                    ulValue = va_arg(args, unsigned long);      // Get the value from the varargs.
                    ulPos = 0;                                  // Reset the buffer position.
                    ulBase = 16;                                // Set the base to 16.

                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.

                    ulNeg = 0;

                    // Determine the number of digits in the string version of
                    // the value.
convert:
                    for (ulIdx = 1;
                        (((ulIdx * ulBase) <= ulValue) &&
                         (((ulIdx * ulBase) / ulBase) == ulIdx));
                        ulIdx *= ulBase, ulCount--)
                    {
                    }

                    // If the value is negative, reduce the count of padding
                    // characters needed.

                    if (ulNeg)
                    {
                        ulCount--;
                    }

                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.

                    if (ulNeg && (cFill == '0'))
                    {
                        // Place the minus sign in the output buffer.

                        pcBuf[ulPos++] = '-';

                        // The minus sign has been placed, so turn off the
                        // negative flag.

                        ulNeg = 0;
                    }

                    // Provide additional padding at the beginning of the
                    // string conversion if needed.

                    if ((ulCount > 1) && (ulCount < 16))
                    {
                        for (ulCount--; ulCount; ulCount--)
                        {
                            pcBuf[ulPos++] = cFill;
                        }
                    }

                    // If the value is negative, then place the minus sign
                    // before the number.

                    if (ulNeg)
                    {
                        // Place the minus sign in the output buffer.

                        pcBuf[ulPos++] = '-';
                    }

                    // Convert the value into a string.

                    for (; ulIdx; ulIdx /= ulBase)
                    {
                        if ( switchChar != 'x' )
                        {
                            pcBuf[ulPos++] = g_pcHexCapital[(ulValue / ulIdx) % ulBase];
                        }
                        else
                        {
                            pcBuf[ulPos++] = g_pcHex[(ulValue / ulIdx) % ulBase];
                        }
                    }

                    // Write the string.

                    strncpy( &pOutString[ outIndex], pcBuf, ulPos);
                    outIndex += ulPos;
                    break;                      // This command has been handled.
                }

                case '%':                                       // Handle the %% command.
                {
                    pOutString[ outIndex++] = *(pcString - 1);  // Simply write a single %.
                    break;                                      // This command has been handled.
                }

                default:                                        // Handle all other commands.
                {
                    strcpy( &pOutString[ outIndex], "<ERROR>" );    // Indicate an error.
                    outIndex += 7;
                    break;                                      // This command has been handled.
                }
            }
        }
    }

    // End the varargs processing.

    va_end(vaArgP);

    // Terminate and return character count

    pOutString[ outIndex] = 0;
    return( outIndex );
}

//------------------------------------------------------------------------------
// Function:    SiiSprintf
// Description: non-library version of sprintf
// Parameters:
// Returns:     length of string copied to the passed buffer, not including
//              the terminating 0
//------------------------------------------------------------------------------

unsigned long SiiSprintf ( char *pOutString, const char *pcString, ... )
{
    va_list         args;

    // Start the varargs processing.

    va_start( args, pcString );
    return( ___sprintf( pOutString, pcString, args ));
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformLoggerPrintf
// Description: Debug print function based on SiiPlatformUartPrintf
// Parameters:
// Returns:
//------------------------------------------------------------------------------

void SiiPlatformLoggerPrintf ( uint_t printFlags, ... )
{
    unsigned long   componentNumber, ulCount;
    const char      *pcString;
    va_list         args;

    if (( printFlags & 0xFF ) > platform.dbgMsgLevel )
    {
        return;
    }
    // Start the varargs processing.

    va_start(args, printFlags);

    if ( colorEnable )
    {
        // Set color based on message type
        switch ( printFlags & 0xFF )
        {
            case MSG_ALWAYS:
                SiiPlatformUartWrite( "\x1b[39m", 5 );   // Default text color
                break;
            case MSG_ERR:
                SiiPlatformUartWrite( "\x1b[31m", 5 );   // RED
                break;
            case MSG_STAT:
                SiiPlatformUartWrite( "\x1b[32m", 5 );   // GREEN
                break;
            case MSG_DBG:
                SiiPlatformUartWrite( "\x1b[33m", 5 );   // YELLOW
                break;
        }
    }

    // Write message prefix as necessary...

    ulCount = 0;
    if ( printFlags & DBGF_CN )
    {
        componentNumber = va_arg( args, unsigned long);
        if ( componentNumber < DBG_MAX_COMPONENTS )
        {
            ulCount += SiiSprintf( &outString[ulCount], "%s: ", dbgMsgComponentNames[ componentNumber ] );
        }
    }
    if ( printFlags & DBGF_TS )
    {
        ulCount += SiiSprintf( &outString[ulCount], "[%010d] ", SiiPlatformTimerSysTicksGet() );
    }

    pcString = va_arg( args, const char *);
    ulCount += ___sprintf( &outString[ulCount], pcString, args );
    SiiPlatformUartCrlfFilterSet( true );
    SiiPlatformUartWrite( outString, ulCount );

    // return text color to default
    if ( colorEnable )
    {
        SiiPlatformUartWrite( "\x1b[39m", 5 );   // Default text color
    }

    // End the varargs processing.

    va_end(vaArgP);
//    SiiPlatformUartFlushTx(false);
}

//------------------------------------------------------------------------------
// Function:    SiiPlatformLoggerMessageLevelSet
// Description: Set platform debug print level
// Parameters:
// Returns:
//------------------------------------------------------------------------------

void SiiPlatformLoggerMessageLevelSet ( uint_t msgLevel )
{
    platform.dbgMsgLevel = (uint8_t)msgLevel;
}


uint8_t SiiPlatformLoggerCharGet(void)
{
    uint8_t ch;
    SiiPlatformUartFlushRx();
    ch = SiiPlatformUartGetc();
    return ch;
}

uint8_t SiiPlatformLoggerDigitGet( uint8_t rangeEnd )
{
    uint8_t digit = 0;
    char ch;

    do
    {
        ch = SiiPlatformLoggerCharGet();
        digit = ch - '0';
    } while (digit > rangeEnd);

    return digit;
}

//------------------------------------------------------------------------------
// Debug function called by ASSERT() macro in DEBUG mode
//------------------------------------------------------------------------------

void __error__(char *pcFilename, unsigned long ulLine)
{
}
