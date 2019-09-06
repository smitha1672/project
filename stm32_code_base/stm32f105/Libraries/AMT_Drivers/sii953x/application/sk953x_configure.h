//***************************************************************************
//!file     sk9535_configure.h
//!brief    Configure the Application and drivers for a specific system design.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_SK9535_CONFIGURE_H__
#define __SI_SK9535_CONFIGURE_H__

#define APP_POWERTYPE_FULL      0   // AON and PWD domain always have power.
                                    // (Enter low power mode when standby functions called.)
#define APP_POWERTYPE_STANDBY   1   // AON domain always has power
#define APP_POWERTYPE_MAIN      2   // AON domain has power only when PWD domain has power

#define SII_POWER_TYPE          APP_POWERTYPE_STANDBY //smith implemented

//------------------------------------------------------------------------------
//  Board-specific constants
//
//  These values are used by the application layer or driver layer, NOT by the
//  component layer, to specify physical board assignments
//------------------------------------------------------------------------------

#define SK_HEC_PORT         1   // HDMI port on board that is connected to HEC

//------------------------------------------------------------------------------
//  CEC
//------------------------------------------------------------------------------

/* Control for amount of text printed by CEC commands.  */

#define CEC_NO_TEXT         0
#define CEC_NO_NAMES        1
#define CEC_ALL_TEXT        2
#define INCLUDE_CEC_NAMES   CEC_NO_NAMES


#endif  // __SI_SK9535_CONFIGURE_H__
