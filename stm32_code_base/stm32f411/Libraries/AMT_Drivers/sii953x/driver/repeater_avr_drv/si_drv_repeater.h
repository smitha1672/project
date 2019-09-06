//***************************************************************************
//!file     si_drv_repeater.h
//!brief    Silicon Image Repeater Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_REPEATER_H__
#define __SI_DRV_REPEATER_H__
#include "si_common.h"
#include "si_drv_switch.h"
#include "si_repeater_component_internal.h"

//------------------------------------------------------------------------------
// Driver enums
//------------------------------------------------------------------------------


typedef enum repeaterRxHdmiMode
{
    SI_RPT_INPUT_MODE_DVI       = 0x0,
    SI_RPT_INPUT_MODE_HDMI      = 0x1

} repeaterRxHdmiMode_t;



//------------------------------------------------------------------------------
// conf
//------------------------------------------------------------------------------

bool_t SiiDrvRepeaterEnable( bool_t repeaterMode );
void SiiDrvRepeaterInit();
void SiiDrvRepeaterSetMode(RepeaterTopoMode_t cmode);
void SiiDrvRepeaterInstanceSet(uint8_t p);
//------------------------------------------------------------------------------
// HDCP
//------------------------------------------------------------------------------

bool_t SiiDrvRepeaterMainPortAuthed();
bool_t SiiDrvRepeaterRxSyncIsOn();
void SiiDrvRepeaterSwitchRxHotPlug(bool_t switch_on);
void SiiDrvRepeaterRxHdcpReset(void);
void SiiDrvRepeaterSetUsBstatus(uint8_t us_bstatus[2]);
void SiiDrvRxHdcpResetRi( void );
void SiiDrvRepeaterSetBcapsFifoReady(bool_t switchOn);
void SiiDrvRepeaterSetBcapsRepeaterBits(bool_t switchOn);
bool_t SiiDrvRepeaterIsRxHotPlugAsserted(void);
uint8_t SiiDrvRepeaterGetSelectedPort(void);
bool_t SiiDrvRepeaterMainPortAuthChanged(void);
bool_t SiiDrvRepeaterAnyInputDecryptionChanged(void);
bool_t SiiDrvRepeaterNewPortAuthed( void );

//------------------------------------------------------------------------------
// isr
//------------------------------------------------------------------------------

bool_t SiiDrvRepeaterIsUsMuteClr(void);
bool_t SiiDrvRepeaterInputIsHdmi(void);
bool_t SiiDrvRepeaterInputModeChange( void );
bool_t SiiDrvRepeaterRxSyncIntr( void );
void SiiDrvRepeaterInterruptsSet(bool_t qOn);
bool_t SiiDrvRepeaterGetInterruptsOn();
bool_t SiiDrvRepeaterMainPortAvMuteChanged(bool_t *muteOn);
void SiiDrvRepeaterResChngStatusSet(bool_t qOn);
bool_t SiiDrvRepeaterResChngStatusGet(void);

//------------------------------------------------------------------------------
// switch
//------------------------------------------------------------------------------
void SiiDrvRepeaterActiveInputSet(uint8_t portIndex);


#endif // __SI_DRVREPEATER_H__


