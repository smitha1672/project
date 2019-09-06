//***************************************************************************
//!file     si_drv_repeater_sha.h
//!brief    Silicon Image Repeater component.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_DRV_REPEATER_SHA_H__
#define __SI_DRV_REPEATER_SHA_H__
#include "si_repeater_component_internal.h"


typedef struct
{
	bool_t 	isTx;          /*true for tx req, false for rx req*/
	uint8_t	portNum;	   /*Rx or Tx port index*/
	bool_t  manualTxStart; /*needed for Tx 0 KSV use*/
	bool_t	isValid;       /*true for valid req, false for invalid/removed req*/
}repeaterShaRequest_t;



bool_t SiiDrvIsShaRequestDone(repeaterShaRequest_t srq);
void SiiDrvShaRequestStart(repeaterShaRequest_t srq);
bool_t SiiDrvShaInUse(void);
void SiiDrvSetShaLength(uint16_t slength);
void SiiDrvShaAdjustKsvStartAddr(uint16_t startPtr );
void SiiDrvRepeaterFillUsKsvInfo(uint16_t ksv_length, uint8_t txsBksv[], uint8_t num_of_tx, uint8_t txIndx);
void SiiDrvShaSetSwitchMode(RepeaterTopoMode_t mode);
void SiiDrvRepeaterKsvPointTo(uint8_t tx_number);
void SiiDrvHwAutoShaDisable(void);
void SiiDrvShaInit(void);

repeaterShaRequest_t SiiDrvGetCurrentShaReq(void);
void SiiDrvShaRequestStop(bool_t isTx, uint8_t portNumber);
#endif //_SI_DRV_REPEATER_SHA_H__
