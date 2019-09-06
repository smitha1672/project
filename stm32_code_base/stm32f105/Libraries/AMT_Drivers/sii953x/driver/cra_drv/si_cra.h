//***************************************************************************
//!file     si_cra.h
//!brief    Silicon Image Device register I/O support.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2008-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_CRA_H__
#define __SI_CRA_H__

typedef uint16_t    SiiReg_t;


bool_t              SiiCraInitialize( void );
SiiResultCodes_t    SiiCraGetLastResult( void );
bool_t              SiiRegInstanceSet( SiiReg_t virtualAddress, prefuint_t newInstance );

void    SiiRegReadBlock ( SiiReg_t virtualAddr, uint8_t *pBuffer, uint16_t count );
uint8_t SiiRegRead ( SiiReg_t virtualAddr );
void    SiiRegWriteBlock ( SiiReg_t virtualAddr, const uint8_t *pBuffer, uint16_t count );
void    SiiRegWrite ( SiiReg_t virtualAddr, uint8_t value );
void    SiiRegModify ( SiiReg_t virtualAddr, uint8_t mask, uint8_t value);
void    SiiRegBitsSet ( SiiReg_t virtualAddr, uint8_t bitMask, bool_t setBits );
void    SiiRegBitsSetNew ( SiiReg_t virtualAddr, uint8_t bitMask, bool_t setBits );

// Special purpose
void    SiiRegEdidReadBlock ( SiiReg_t segmentAddr, SiiReg_t virtualAddr, uint8_t *pBuffer, uint16_t count );

#endif  // __SI_CRA_H__
