//***************************************************************************
//!file     si_datatypes.h
//!brief    Silicon Image data type header (tries to conform to C99).
//
// No part of this work may be reproduced, modified, distributed, 
// transmitted, transcribed, or translated into any language or computer 
// format, in any form or by any means without written permission of 
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DATATYPES_H__
#define __SI_DATATYPES_H__

#include "device_config.h"

#if ( configSII_DEV_953x_PORTING == 0 )
#include "limits.h"
#endif 

#if ( configSII_DEV_953x_PORTING == 1 )

#if defined (__ARM_CORTEX_MX__)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined ( STM32F10X_CL )
#include "stm32f10x.h"
#include "core_cm3.h"
#elif defined ( STM32F40_41xxx )
#include "stm32f4xx.h"
#endif

#endif	//! @}

#define CLOCK_TIME_MAX  0xFFFFFFFF

typedef uint32_t clock_time_t;   // The clock type used for returning system ticks (1ms).
typedef uint16_t time_ms_t;      // Time in milliseconds
typedef int int_t;
typedef unsigned int uint_t;
typedef int prefuint_t; // Platform-specific efficient integers

typedef unsigned char   bit_fld_t_t;    // bit field type used in structures
typedef unsigned short  bit_fld16_t_t;
typedef bit_fld_t_t bit_fld_t;
typedef bit_fld16_t_t	bit_fld16_t;

#endif 


#if ( configSII_DEV_953x_PORTING == 0 )
// Use these two types most of the time, such as for index variables,
// etc. when you want the native integer type and don't
// care what size it is (within reason; most int types are 16
// or 32 bit, not 8 bit -- even on the 8051)
// On 8051 compilers, these types can be defined as 8-bit to
// use the most efficient data size for that processor.  When you
// absolutely need an integer of at least 16 bits, use the native type int

typedef signed int      int_t;
typedef unsigned int    uint_t;
//typedef char            int_t;    // Eight-bit processor only
//typedef unsigned char   uint_t;   // Eight-bit processor only
typedef int             prefuint_t; // Platform-specific efficient integers

    /* C99 defined data types.  */

typedef unsigned char  uint8_t;		// Don't use this unless you truly need an unsigned 8 bit variable
typedef unsigned short uint16_t;	// Don't use this unless you truly need an unsigned 16 bit variable
typedef unsigned long  uint32_t;    // Don't use this unless you truly need an unsigned 32 bit variable

typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed long    int32_t;

typedef unsigned long   clock_time_t;   // The clock type used for returning system ticks (1ms).
typedef unsigned short  time_ms_t;      // Time in milliseconds

#define CLOCK_TIME_MAX  ULONG_MAX       // Maximum value of time type

// gcc -pedantic does not allow bit field operation, so to avoid the warning, we need the following trick
typedef unsigned char   bit_fld_t_t;    // bit field type used in structures
typedef unsigned short  bit_fld16_t_t;
#define bit_fld_t       __extension__ bit_fld_t_t
#define bit_fld16_t     __extension__ bit_fld16_t_t
#endif 


#if !defined ON
#define ON  true
#define OFF false
#endif

#if ( configSII_DEV_953x_PORTING == 1 )

#if !defined( true )
#define true 1
#endif 

#if !defined ( false )
#define false 0
#endif 

typedef uint8_t bool_t;

#else
#if !defined TRUE
#define TRUE  true
#define FALSE false
#endif
/* Emulate C99/C++ bool type    */
#ifdef __cplusplus
typedef bool bool_t;
#else
typedef enum
{
	false   = 0,
	true    = !(false)
} bool_t;
#endif // __cplusplus
#endif 

#define SET_BITS    0xFF
#define CLEAR_BITS  0x00

#define ROM            // 8051 type of ROM memory
#define XDATA          // 8051 type of external memory

#define PACKED

#define ABS_DIFF(A, B) ((A>B) ? (A-B) : (B-A))

//------------------------------------------------------------------------------
// Configuration defines used by hal_config.h
//------------------------------------------------------------------------------

#define ENABLE      (1)
#define DISABLE     (0)

#define BIT0 		0x01
#define BIT1 		0x02
#define BIT2 		0x04
#define BIT3 		0x08
#define BIT4 		0x10
#define BIT5 		0x20
#define BIT6 		0x40
#define BIT7 		0x80
#define BIT8        0x0100
#define BIT9        0x0200
#define BIT10       0x0400
#define BIT11       0x0800
#define BIT12       0x1000
#define BIT13       0x2000
#define BIT14       0x4000
#define BIT15       0x8000

//------------------------------------------------------------------------------
// Language extending macros
//------------------------------------------------------------------------------

// Test chain macro
// Note: "switch" operator must be used with caution as PASS_IF includes "break"
#define BEGIN_TEST do{
#define PASS_IF(a) {if(!(a)) break;}
#define FAIL_IF(a) {if(a) break;}
#define END_TEST }while(false);


#endif  // __SI_DATATYPES_H__
