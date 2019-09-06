//***************************************************************************
//!file     si_repeater_avr_config.h
//!brief    System-specific configuration information for REPEATER AVR
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SI_CONFIG_REPEATER_AVR_H__
#define __SI_CONFIG_REPEATER_AVR_H__

//------------------------------------------------------------------------------
//  REPEATER AVR Component SiI953x-specific
//------------------------------------------------------------------------------

#define SII_NUM_PIPE                  2   // Number of repeater instances
#define CHECK_SAME_TX					  0   // don't check if DS is the same as previous
#endif  //__SI_CONFIG_REPEATER_AVR_H__
