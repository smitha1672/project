//***************************************************************************
//!file     si_drv_gpio_internal.h
//!brief    Silicon Image CPI internal driver functions.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_GPIO_DRV_INTERNAL_H__
#define __SI_GPIO_DRV_INTERNAL_H__
#include "si_common.h"
#include "si_regs_gpio953x.h"
#include "si_drv_gpio.h"
#include "si_drv_device.h"

void        DrvGpioProcessInterrupts ( void );

#endif // __SI_GPIO_DRV_INTERNAL_H__
