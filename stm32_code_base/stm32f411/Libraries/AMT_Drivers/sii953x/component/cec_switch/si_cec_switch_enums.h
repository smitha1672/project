//***************************************************************************
//! @file     si_cec_switch_enums.h
//! @brief    CEC/Switch command and operand codes
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2002-2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/
#ifndef __SI_CEC_SWITCH_ENUMS_H__
#define __SI_CEC_SWITCH_ENUMS_H__

                                    //! CEC/Switch tasks FSM states
typedef enum _CecSwitchTaskState_t
{
    CEC_SWITCH_TASK_EMPTY_STATE_NONE                      = 0x00,
    CEC_SWITCH_TASK_SEND_ROUTE_INFO_STATE_CHECK_HPD       = 0x01,
    CEC_SWITCH_TASK_SEND_ROUTE_INFO_STATE_PAUSE           = 0x02,
    CEC_SWITCH_TASK_SEND_ROUTE_INFO_STATE_REPORT_RESULTS  = 0x03,
    
} CecSwitchTaskState_t;

#endif // #define __SI_CEC_SWITCH_ENUMS_H__
