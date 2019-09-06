//******************************************************************************
// !file    si_drv_arc.h
// !brief   Silicon Image ARC driver functions for the Sii9535
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_ARC_H__
#define __SI_DRV_ARC_H__
#include "string.h"
#include "si_datatypes.h"

#define SII_NUM_ARC             1       // Number of ARC driver instances

#define ARC_TX_MODE   			0		// ARC Configuration Modes.
#define ARC_RX_MODE   			1
#define ARC_DISABLE_MODE		2

//------------------------------------------------------------------------------
// Function Parameters
//------------------------------------------------------------------------------

int     SiiDrvArcInstanceGet( void );
bool_t  SiiDrvArcInstanceSet( int_t instanceIndex );
bool_t  SiiDrvArcStandby( void );
bool_t  SiiDrvArcResume( void );
bool_t  SiiDrvArcInitialize(  );
bool_t  SiiDrvArcConfigureArcPin( int_t arcPin, int_t spdifSelPin, int_t mode );


#endif  // __SI_DRV_ARC_H__
