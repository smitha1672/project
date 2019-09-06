//***************************************************************************
//!file     si_idt_5v9885t.h
//!brief    Audio master clock PLL driver
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_IDT_5V9885T_H__
#define __SI_IDT_5V9885T_H__

//I2C slave address
#define IDT_PLL_I2C_SADDR       0xD4
#define SII_AUD_FS_SIZE		    110 //sizeof(Freq256[0][0])

void SiiPlatformExtnalAudioPllSetting (uint8_t fsType, uint8_t freq);

#endif	// __SI_IDT_5V9885T_H__
