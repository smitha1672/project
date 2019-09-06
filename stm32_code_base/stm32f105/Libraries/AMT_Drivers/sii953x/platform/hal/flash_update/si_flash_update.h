//***************************************************************************
//!file     si_flash_update.h
//!brief    Silicon Image flash update interface.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_FLASH_UPDATE_H__
#define __SI_FLASH_UPDATE_H__

#define SI_ROM_UART_BOOT_LOADER         0
#define SI_FLASH_UART_BOOT_LOADER       1
#define SI_FLASH_USB_DFU_BOOT_LOADER    2

//------------------------------------------------------------------------------
// Flash Update functions
//------------------------------------------------------------------------------

void SiiPlatformFirmwareUpdate(uint8_t loaderType);

#endif  // __SI_FLASH_UPDATE_H__
