//***************************************************************************
//!file     si_audio_dac.c
//!brief    Platform GPIO driver, ARM (Stellaris) version.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#include "string.h"

#include "inc/hw_gpio.h"
#undef GPIO_LOCK_KEY

#include "si_common.h"
#include "si_platform.h"
#include "si_i2c.h"
#include "si_fpga.h"

#include "si_arm.h"

//-------------------------------------------------------------------------------------------------
//! @brief      Initialization of FPGA platform.
//!
//-------------------------------------------------------------------------------------------------

void SiiPlatformFpgaInit()
{
	//Power Up Ursula
	SiiPlatformI2cWriteByte(0, 0x64, 0x08, 0x01);
	SiiPlatformI2cWriteByte(0, 0x66, 0x08, 0x01);
	//SiiRegWrite(0x1078, 0x01);
}
