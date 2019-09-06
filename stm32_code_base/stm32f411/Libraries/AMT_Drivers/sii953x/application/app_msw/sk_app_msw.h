//***************************************************************************
//!file     sk_app_msw.h
//!brief    Matrix Switch Application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SKAPP_MSW_H__
#define __SKAPP_MSW_H__
#include "si_infrared.h"

void    SkAppMswDemoZone1( int_t port );
void    SkAppMswDemoZone2( int_t port );
void    SkAppMswDemoSetSources( int_t portA, int_t portB );

bool_t  SkAppMswDemo ( RC5Commands_t key );
void 	SkAppHdcpR0DelaySetting( uint8_t vic );
#endif  // __SKAPP_MSW_H__
