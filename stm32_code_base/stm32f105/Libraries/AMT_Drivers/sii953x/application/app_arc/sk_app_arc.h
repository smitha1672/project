//***************************************************************************
//!file     sk_app_arc.h
//!brief    Wraps board and device functions for the ARC component/driver
//          and the application
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2009-2011, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#if !defined __SKAPP_ARC_H__
#define __SKAPP_ARC_H__


#if INC_ARC
#include "sk_application.h"
#include "si_drv_arc.h"
#include "si_drv_arc_config.h"

#if ( configSII_DEV_953x_PORTING )
#define SYS_CEC_TASK_ARC_INITIONAL           0x01  //
#define SYS_CEC_TASK_ARC_TERMINATE          0x02  //
#define SYS_CEC_TASK_ARC_REPORT_INITIONAL           0x04  //
#define SYS_CEC_TASK_ARC_REPORT_TERMINATE          0x08  //
#define SYS_CEC_TASK_ARC_INITIONAL_LATE           0x10  //
#endif

//------------------------------------------------------------------------------
//  ARC Component typedefs
//------------------------------------------------------------------------------

typedef struct SiiArcConfigure_t
{
    int_t       chPort[SII_NUM_ARC];   // ARC channel port assignments.
} SiiArcConfigure_t;


#if ( configSII_DEV_953x_PORTING == 1 )
typedef enum
{
    //ARC_TASK_INIT_OUTRIGHT,
    //ARC_TASK_INIT_PRIVATELY,
    //ARC_TASK_INIT_REQUESTED,
    ARC_TASK_REPORT_INIT,
    ARC_TASK_TERM_REQUESTED,
    ARC_TASK_TERM_PRIVATELY,
    ARC_TASK_REPORT_TERM
}AmTArcTaskEvent; 
#endif

//------------------------------------------------------------------------------
// Module variables
//------------------------------------------------------------------------------

extern char arcRequestToNonArcPort [];

void    SkAppUpdateHeacState( void );
bool_t  SkAppDeviceInitArc( void );
#if ( configSII_DEV_953x_PORTING == 1 )
void AmTCecArcTask ( void );
void AmTArcTaskAssign( uint16_t SysArcAppTask );
//void AmTArcRxModeCtrl( bool_t ArcOn );
bool_t AmTIPowerOnSystem( void );
static void AmTARCMessageSend( bool_t MsgType );
static void AmTUpdateARCTxLA ( uint16 ARCTxLA);
#endif

bool_t  SkAppArcDemo( RC5Commands_t key );

#endif
#endif  //__SKAPP_ARC_H__
