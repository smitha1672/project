//***************************************************************************
//!file     si_logger.h
//!brief    Silicon Image Starter Kit debug print message interface
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_LOGGER_H__
#define __SI_LOGGER_H__

//------------------------------------------------------------------------------
//  Logger UI API functions
//------------------------------------------------------------------------------

void    SiiPlatformLoggerPrintf( uint_t printFlags, ... );
void    SiiPlatformLoggerMessageLevelSet( uint_t msgLevel );
void    SiiPlatformLoggerColorEnable( bool_t enable );

uint8_t SiiPlatformLoggerCharGet(void);
uint8_t SiiPlatformLoggerDigitGet(uint8_t rangeEnd);

//------------------------------------------------------------------------------
//  API for formatted printing to a string
//------------------------------------------------------------------------------

unsigned long SiiSprintf ( char *pOutString, const char *pcString, ... );
unsigned long ___sprintf ( char *pOutString, const char *pcString, void *pArgs );

//------------------------------------------------------------------------------
//  API for ASSERT reporting
//------------------------------------------------------------------------------

void __error__(char *pcFilename, unsigned long ulLine);

#endif
