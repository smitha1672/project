//***************************************************************************
//!file     si_repeater_sha.h
//!brief    Silicon Image Repeater component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_REPEATER_SHA_H__
#define __SI_REPEATER_SHA_H__

#include "si_debug.h"
#include "si_repeater_component.h"
#include "si_drv_repeater_sha.h"


bool_t RepeaterShaHandlerInitialize ( void );
void RepeaterShaHandler( void );
void RepeaterShaSetLength(uint16_t slength);
void SiiRepeaterShaRequest ( bool_t force, repeaterShaRequest_t srq );  //add switchConf to



#endif//__SI_REPEATER_SHA_H__

