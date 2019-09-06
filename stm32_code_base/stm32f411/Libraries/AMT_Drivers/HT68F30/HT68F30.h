#ifndef __HT68F30_H__
#define __HT68F30_H__

#include "Defs.h"

typedef enum
{
    HT68F30_NULL = 0,
    HT68F30_NORMAL,
    HT68F30_BLINK_FADE_TO_DARK,
    HT68F30_FADE_TO_DARK,
    HT68F30_BREATH,
    HT68F30_SCANNING,
    HT68F30_STEPS,
    HT68F30_DARK,
    HT68F30_FLASHING
}xHT68F30EFFECT;

typedef struct HT68F30_PARMS
{
    uint8 start;
    uint8 end;
    xHT68F30EFFECT effect;
}xHT68F30Parms;


typedef struct 
{
  void (*CreateTask)(void);  
    void (*setStatus)(const void *parms);
    void (*LightConverter)(uint8 StartPos, uint8 EndPos);
    void (*Set)(uint8 StartPos, uint8 EndPos);
    void (*BlinkFadeToDarkConverter)(uint8 StartPos, uint8 EndPos,uint16 Holdtime,uint16 duration, uint16 times);
    void (*FadeToDarkConverter)(uint8 StartPos, uint8 EndPos,uint16 Holdtime,bool Foreground,bool Background,bool Clean);
    void (*BreathConverter)(uint8 StartPos, uint8 EndPos);
    void (*BreathClear)(uint8 StartPos, uint8 EndPos);
    void (*ScanningConverter)(uint8 StartPos, uint8 EndPos);
    void (*StepsConverter)(uint8 StartPos, uint8 EndPos);
    void (*StepsClear)(uint8 StartPos, uint8 EndPos);
    void (*DarkConverter)(uint8 StartPos, uint8 EndPos);
    void (*Flashing)(uint8 StartPos, uint8 EndPos,uint16 duration, uint16 times,bool StandbyLED);
    void (*Version)(void);    
    void (*CleanALL)(uint8 StartPos, uint8 EndPos,uint16 Holdtime, bool background);
    void (*CleanLED)(uint8 StartPos, uint8 EndPos,uint16 Holdtime, bool background);
    bool (*IsControllingLED)(void);
    void (*UpdateDemoTimeOutState)(bool state);
}LED_OBJECT;


#endif /*__HT68F30_H__*/
