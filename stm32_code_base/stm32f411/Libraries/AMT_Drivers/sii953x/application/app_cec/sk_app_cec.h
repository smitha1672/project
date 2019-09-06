//***************************************************************************
//!file     sk_app_cec.h
//!brief    Application and demo functions for the CEC component
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2011-2012, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SKAPP_CEC_H__
#define __SKAPP_CEC_H__

#include "sk_application.h"
#include "si_cec_component.h"
#if INC_CEC_SWITCH
#include "si_cec_switch_api.h"
#if ( configSII_DEV_953x_PORTING == 1 )
#include "si_cec_internal.h"
#endif
#else
#if ( configSII_DEV_953x_PORTING == 1 )
#include "si_cec_internal.h"
#endif
#endif 

#if INC_CEC_SAC
#include "si_sac_main.h"
#endif

#if ( configSII_DEV_953x_PORTING == 1 )
#if INC_ARC
#include "sk_app_arc.h"
#endif
#endif

extern  uint8_t  appCecSourceLaList[];

typedef struct _SiiAppCecInstanceData_t
{
    int     instanceIndex;

    bool_t  isEnumerated;
    bool_t  setStreamPathReceived;  // Used by wakeup for CBUS support
    uint8_t setStreamPathArg0;      // Used by wakeup for CBUS support
    uint8_t setStreamPathArg1;      // Used by wakeup for CBUS support

}   SiiAppCecInstanceData_t;

#if ( configSII_DEV_953x_PORTING == 1 )
typedef enum
{
    Cec_TASK_CHANGE_PA
}AmTCecTaskEvent; 
#endif


extern SiiAppCecInstanceData_t appCec[SII_NUM_CPI];
extern SiiAppCecInstanceData_t *pAppCec;


void    SkAppCecInstanceSet( int_t newInstance );
bool_t  SkAppDeviceInitCec( void );
void    SkAppCecConfigure( bool_t enable );
void    SkAppTaskCec( void );
bool_t  SkAppCecStandby( void );
bool_t  SkAppCecResume( bool_t powerIsOn );

void    SiiCecAppInputPortChangeHandler(uint8_t newInputPortIndex);

void    SkAppCecMessageLogger( SiiCpiData_t *pMsg, int systemType, bool_t isTx );
bool_t  SkAppCecDemo( RC5Commands_t key );

#if ( configSII_DEV_953x_PORTING == 1 )
void AmTCecTask( void );
void AmTCecTaskAssign( AmTCecTaskEvent event );
#endif

#endif  //__SKAPP_CEC_H__
