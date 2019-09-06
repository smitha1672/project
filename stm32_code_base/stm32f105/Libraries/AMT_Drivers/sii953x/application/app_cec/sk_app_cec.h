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

#if ( configSII_DEV_953x_PORTING == 1 )
#define CEC_HDMI_TASK_CHANGE_PA 0x0001
#define CEC_HDMI_TASK_HDMI_OFF 0x0002
#define CEC_HDMI_TASK_ARC_RX_MODE 0x0004
#define CEC_HDMI_TASK_ARC_DISABLE_MODE 0x0008
#endif

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
    HDMI_TASK_CHANGE_PA,
    HDMI_TASK_HDMI_OFF,
    HDMI_TASK_ARC_RX_MODE,
    HDMI_TASK_ARC_DISABLE_MODE
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
void AmTCecTaskAssign( uint16_t SysCECAppTASK );
#endif

#endif  //__SKAPP_CEC_H__
