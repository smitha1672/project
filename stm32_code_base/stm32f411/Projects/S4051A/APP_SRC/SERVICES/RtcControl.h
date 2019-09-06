#ifndef __RTCCONTROL_H__
#define __RTCCONTROL_H__

#include "Defs.h"

typedef enum
{
    TIME_EVENT_0,
    TIME_EVENT_30minute,
    TIME_EVENT_4Hour,
    TIME_EVENT_NULL
} TIME_EVENT_ASSIGN;

void RTC_Control(bool RtcEn);   //Jerry add for BT use
void RTCTime_Show(void);
void Time_Control_Poll(void);
void RTC_Configuration(void);

//!_____________________________________Smith implemented @{
uint32 RealTimeCounter_reportCurrentTime(void);

//!@}
#endif /*__RTCCONTROL_H__*/

