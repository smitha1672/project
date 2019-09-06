//***************************************************************************
//!file     si_audio_dac.h
//!brief    Audio DAC driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SI_AUDIO_H__
#define __SI_AUDIO_H__

//------------------------------------------------------------------------------
// Local Defines for Audio DAC control
//------------------------------------------------------------------------------

//I2C slave address
#define SI_ADAC_SLAVE_ADDR_MP     0x22
#define SI_ADAC_SLAVE_ADDR_SP     0x24

//register offsets
#define SI_ADAC_CTRL1_ADDR     0x00
#define SI_ADAC_CTRL2_ADDR     0x01
#define SI_ADAC_SPEED_PD_ADDR  0x02
#define SI_ADAC_CTRL3_ADDR     0x0A

//power down control bits
#define SI_ADAC_POWER_DOWN        0x00
#define SI_ADAC_NORMAL_OPERATION  0x4F

//dac operating modes
#define SI_ADAC_PCM_MODE       0x00
#define SI_ADAC_DSD_MODE       0x18


//------------------------------------------------------------------------------
// Audio DAC control API
//------------------------------------------------------------------------------

void SiiPlatformAudioDacMute(uint8_t chn, uint8_t ctrl);
void SiiPlatformAudioDacPower( uint8_t chn, uint8_t ctrl );
void SiiPlatformAudioDacControl(uint8_t chn,  uint8_t audType);
#endif	// __SI_AUDIO_H__
