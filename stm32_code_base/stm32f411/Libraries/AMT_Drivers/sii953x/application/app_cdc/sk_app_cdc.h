//***************************************************************************
//!file     sk_app_cdc.h
//!brief    Application and demo functions for the CDC component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SKAPP_CDC_H__
#define __SKAPP_CDC_H__

#include "si_cdc_component.h"

bool_t  SkAppInitCdc( void );
bool_t  SkAppCdcMsgHandler( SiiCpiData_t *pMsg );
void    SkAppCdcInfoPrint ( void );
void    SkAppCdcInquireStateTest ( void );
void    SkAppCdcSingleDeviceConnectTest ( uint8_t devIndex );

bool_t  SkAppCdcDemo( RC5Commands_t key );

#endif  //__SKAPP_CDC_H__
