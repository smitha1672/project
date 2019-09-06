//***************************************************************************
//!file     si_debug.h
//!brief    Silicon Image Debug header.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#ifndef __SI_DEBUG_H__
#define __SI_DEBUG_H__
#include <stdio.h>

#include "device_config.h"
#include "si_datatypes.h"

//------------------------------------------------------------------------------
// Debug Flags
//------------------------------------------------------------------------------

#define RCP_DEBUG           0       // 1 - Enable RCP debug mode

#if ( configSII_DEV_953x_PORTING == 0 )
#define API_DEBUG_CODE      0       // Set to 0 to eliminate debug print messages from code
                                    // Set to 1 to allow debug print messages in code
#else
#define API_DEBUG_CODE      0
#endif 
//------------------------------------------------------------------------------
// Debug Macros
//------------------------------------------------------------------------------

    // Debug print message level
#define MSG_ALWAYS              0x00

#ifndef MSG_ERR
#define MSG_ERR                 0x01
#endif 

#define MSG_STAT                0x02
#define MSG_DBG                 0x03

#if (API_DEBUG_CODE==1)

    #ifndef DEBUG_PRINT
    #define TDEBUG_PRINT(x,y)   SiiPlatformLoggerPrintf( x, y )
#if ( configSII_DEV_953x_PORTING == 0 )
    #define DEBUG_PRINT(x,...)  SiiPlatformLoggerPrintf(x,__VA_ARGS__)
#else
    #define DEBUG_PRINT(x,y,...) TRACE_DEBUG((x,y,##__VA_ARGS__))
#endif
    #endif
    #define _DEBUG_(x)              x

#else
    #ifndef DEBUG_PRINT
        #define DEBUG_PRINT(...)
    #endif
    #define _DEBUG_(x)
#endif

//------------------------------------------------------------------------------
// Debug Status Codes
//------------------------------------------------------------------------------

#if ( configSII_DEV_953x_PORTING == 0 )
enum
{
    I2C_SUCCESS,
    I2C_READ_FAIL,
    I2C_WRITE_FAIL,
};
#endif 

//------------------------------------------------------------------------------
// Debug Trace
//------------------------------------------------------------------------------

#if (API_DEBUG_CODE==1)
    #define DF1_SW_HDCPGOOD         0x01
    #define DF1_HW_HDCPGOOD         0x02
    #define DF1_SCDT_INT            0x04
    #define DF1_SCDT_HI             0x08
    #define DF1_SCDT_STABLE         0x10
    #define DF1_HDCP_STABLE         0x20
    #define DF1_NON_HDCP_STABLE     0x40
    #define DF1_RI_CLEARED          0x80

    #define DF2_MP_AUTH             0x01
    #define DF2_MP_DECRYPT          0x02
    #define DF2_HPD                 0x04
    #define DF2_HDMI_MODE           0x08
    #define DF2_MUTE_ON             0x10
    #define DF2_PORT_SWITCH_REQ     0x20
    #define DF2_PIPE_PWR            0x40
    #define DF2_PORT_PWR_CHG        0x80

#endif

//------------------------------------------------------------------------------
//  From si_platform.h
//------------------------------------------------------------------------------
#if !defined __PLATFORM_STATUS_CODES__
#define __PLATFORM_STATUS_CODES__

typedef enum _SkDebugPrintModule_t
{
    DBG_BRD,
    DBG_DEV,
    DBG_CBUS,
    DBG_CDC ,
    DBG_CEC ,
    DBG_CPI ,
    DBG_EDID,
    DBG_HEAC,
    DBG_RPTR,
    DBG_SWCH,
    DBG_VSIF,
    DBG_TX,
    DBG_EDID_TX,

    DBG_MAX_COMPONENTS
} SkDebugPrintModule_t;

typedef enum _SkDebugPrintFlags_t
{
    DBGF_TS     = 0x0100,       // Add Time Stamp to debug message output
    DBGF_CN     = 0x0200,       // Add Component Name to debug message output
                                // (must include component name index as first
                                // parameter after format string)
    DBGF_CP     = 0x0400,       // Add Component Name to debug message output
                                // (must include pointer to component instance
                                // data as first parameter after format string)
    DBGF_CS     = 0x0800,       // Add Component Name to debug message output
                                // (must include pointer to display string
                                // as first parameter after format string)
} SkDebugPrintFlags_t;

#endif // __PLATFORM_STATUS_CODES__

//------------------------------------------------------------------------------
//  Debug Print Function.  This function declaration is actually from the
//                         Board component, but will always have the same
//                         interface, so we will not have to include the
//                         si_platform.h file, which would be ugly.
//------------------------------------------------------------------------------

//void    SiiPlatformLoggerPrintf( uint_t printFlags, ... );
//void    SiiPlatformLoggerMessageLevelSet( uint_t msgLevel );
//void    SiiPlatformLoggerColorEnable( bool_t enable );
//uint8_t SiiPlatformLoggerCharGet(void);
//uint8_t SiiPlatformLoggerDigitGet(uint8_t rangeEnd);

#endif // __SI_DEBUG_H__
