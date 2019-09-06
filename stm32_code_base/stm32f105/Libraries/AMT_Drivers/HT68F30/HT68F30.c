#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"
#include "HT68F30.h"
#include "HT68F30_i2c.h"

/* LED_effect*/

typedef struct HT68F30_PARAMETERS
{
    xTaskHandle HT68F30_TaskHandle;
    TaskHandleState HT68F30_state;
    uint8 volume;
    bool mute_state;
} xHT68F30_Params;

static xHT68F30_Params mHT68F30Parms;
static xSemaphoreHandle mHT68F30Mutex = NULL;


static void HT68F30_LightBar_CreateTask( void );

void HT68F30_setStatus(const void *parms);

void HT68F30_LightConverter(uint8 StartPos, uint8 EndPos);

void HT68F30_Set(uint8 StartPos, uint8 EndPos);

void HT68F30_BlinkFadeToDarkConverter(uint8 StartPos, uint8 EndPos,uint16 Holdtime,uint16 duration, uint16 times);

void HT68F30_FadeToDarkConverter(uint8 StartPos, uint8 EndPos,uint16 Holdtime,bool Foreground,bool Background,bool Clean);

void HT68F30_BreathConverter(uint8 StartPos, uint8 EndPos);

void HT68F30_BreathClear(uint8 StartPos, uint8 EndPos);

void HT68F30_ScanningConverter(uint8 StartPos, uint8 EndPos);

void HT68F30_StepsConverter(uint8 StartPos, uint8 EndPos);

void HT68F30_StepsClear(uint8 StartPos, uint8 EndPos);

void HT68F30_DarkConverter(uint8 StartPos, uint8 EndPos);

void HT68F30_Flashing(uint8 StartPos, uint8 EndPos,uint16 duration, uint16 times,bool StandbyLED);

void HT68F30_Version(void);

void HT68F30_CleanALL(uint8 StartPos, uint8 EndPos,uint16 Holdtime, bool background);

void HT68F30_CleanLED(uint8 StartPos, uint8 EndPos,uint16 Holdtime, bool background);

bool HT68F30_IsControllingLED(void);    //Angus added in order to know whether leds are contoled or not for UI 4.3 Spec 7.1.1.

void HT68F30_UpdateDemoTimeOutState(bool state);    //Angus adder for demo mode timeout behavior.

void HT68F30_UpdateDemoShowVolState(bool state); //Tony add for demo first vol

const LED_OBJECT HT68F30_ObjCtrl =
{
    HT68F30_LightBar_CreateTask,
    HT68F30_setStatus,
    HT68F30_LightConverter,
    HT68F30_Set,
    HT68F30_BlinkFadeToDarkConverter,
    HT68F30_FadeToDarkConverter,
    HT68F30_BreathConverter,
    HT68F30_BreathClear,
    HT68F30_ScanningConverter,
    HT68F30_StepsConverter,
    HT68F30_StepsClear,
    HT68F30_DarkConverter,
    HT68F30_Flashing,
    HT68F30_Version,
    HT68F30_CleanALL,
    HT68F30_CleanLED,
    HT68F30_IsControllingLED,
    HT68F30_UpdateDemoTimeOutState,
    HT68F30_UpdateDemoShowVolState,
};

const LED_OBJECT *pLED_ObjCtrl = &HT68F30_ObjCtrl;

/* LED position define */
typedef struct
{
    const uint8 light_addr;
    uint8 duty;
    xHT68F30EFFECT effect;
    uint16 time_stamp;
    bool changed;
}LightBar;

/* LED duty define */
#define LIGHT_BAR_DUTY_MAX   0x78
#define DUTY_HIGH            50
#define DUTY_MID             26
#define DUTY_LOW             2

#ifdef LIGHT_BAR_INVERT
#define LIGHT_BAR_DUTY_HIGH (0x78 - ((LIGHT_BAR_DUTY_MAX*DUTY_HIGH) /100))
#define LIGHT_BAR_DUTY_MID  (0x78 - ((LIGHT_BAR_DUTY_MAX*DUTY_MID) /100))
#define LIGHT_BAR_DUTY_LOW  (0x78 - ((LIGHT_BAR_DUTY_MAX*DUTY_LOW) /100))
#define LIGHT_BAR_DUTY(X)   (0x78 - ((LIGHT_BAR_DUTY_MAX * X) /100))
#define LIGHT_BAR_DUTY_DARK (LIGHT_BAR_DUTY_MAX)
#else
#define LIGHT_BAR_DUTY_HIGH (0x00 + ((LIGHT_BAR_DUTY_MAX*DUTY_HIGH) /100))
#define LIGHT_BAR_DUTY_MID  (0x00 + ((LIGHT_BAR_DUTY_MAX*DUTY_MID) /100))
#define LIGHT_BAR_DUTY_LOW  (0x00 + ((LIGHT_BAR_DUTY_MAX*DUTY_LOW) /100))
#define LIGHT_BAR_DUTY(X)   (0x00 + ((LIGHT_BAR_DUTY_MAX * X) /100))
#define LIGHT_BAR_DUTY_DARK (0x00)
#endif /*LIGHT_BAR_INVERT*/

/* Light Bar */
const static uint8 mLightBar_Duty1[]={LIGHT_BAR_DUTY_LOW,LIGHT_BAR_DUTY_MID,LIGHT_BAR_DUTY_HIGH};

static LightBar mLightBarParams[] =
{
    { 0x0C, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x0B, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x0A, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x09, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x08, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x07, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x06, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x05, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x04, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x03, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x02, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x01, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x0D, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE},
    { 0x0E, LIGHT_BAR_DUTY_DARK, HT68F30_NULL, 0, TRUE}
};

/*  Sound Bar LED S3851
level:0~                               35  36 ~41
    _____________________________________________
   |  *  *  *  *  *  *  *  *  *  *  *  *   *  *  |
      C  B  A  9  8  7  6  5  4  3  2  1 | D  E
               LED_VOL_NUMBER              LED_FORMAT_NUMBER
*/
#define DUTY1_NUM                 (sizeof(mLightBar_Duty1)/sizeof(uint8))
#define MAX_DUTY1_INDEX           DUTY1_NUM-1
#define MIN_DUTY1_INDEX           0

#define LED_LEVEL_NUM             (sizeof(mLightBarParams)/sizeof(LightBar))
#define LED_VOL_LEVEL_NUM         LED_LEVEL_NUM -2

#define MAX_LED_LEVEL_INDEX       LED_LEVEL_NUM - 1
#define MID_LED_LEVEL_INDEX       (LED_LEVEL_NUM/2)
#define MIN_LED_LEVEL_INDEX       0

#define LED_FORMAT_NUMBER         2
#define LED_VOL_NUMBER            LED_LEVEL_NUM - LED_FORMAT_NUMBER

#define MIN_VOL_POS               0
#define POW_VOL_POS               2
#define MAX_VOL_POS               35
#define MIN_FORMAT_POS            36
#define MAX_FORMAT_POS            41

static LightBar mNewLightBarParams[] =
{
    { 0x0C, NULL, HT68F30_NULL, 0, TRUE},
    { 0x0B, NULL, HT68F30_NULL, 0, TRUE},
    { 0x0A, NULL, HT68F30_NULL, 0, TRUE},
    { 0x09, NULL, HT68F30_NULL, 0, TRUE},
    { 0x08, NULL, HT68F30_NULL, 0, TRUE},
    { 0x07, NULL, HT68F30_NULL, 0, TRUE},
    { 0x06, NULL, HT68F30_NULL, 0, TRUE},
    { 0x05, NULL, HT68F30_NULL, 0, TRUE},
    { 0x04, NULL, HT68F30_NULL, 0, TRUE},
    { 0x03, NULL, HT68F30_NULL, 0, TRUE},
    { 0x02, NULL, HT68F30_NULL, 0, TRUE},
    { 0x01, NULL, HT68F30_NULL, 0, TRUE},
    { 0x0D, NULL, HT68F30_NULL, 0, TRUE},
    { 0x0E, NULL, HT68F30_NULL, 0, TRUE}
};

static uint8 nFlashCount;
static uint8 nFlashSpeedTarget;
static uint8 nFlashSpeed;
static bool bFlasgingVector = TRUE;
static bool bStandby = TRUE;

static uint8 nStepsEndNum = 0;
static uint8 nScanNum;
static uint8 nStepsNum;
static uint8 nScanCount;
static uint8 nStepsCount;

static bool LightBar_DemoTimeOut = FALSE;    //Angus added , for auto setting timeout led after diaplay another led behavior.(2014/11/5)
static bool LightBar_DemoShowVol = FALSE;
//______________________________________________________________________________

static void ht68f30_lock(void)
{
    if(mHT68F30Mutex==NULL)
        return;

    ASSERT(xSemaphoreTake( mHT68F30Mutex, portMAX_DELAY ) == pdPASS);
}

static void ht68f30_unlock()
{
    if(mHT68F30Mutex==NULL)
        return;

    ASSERT(xSemaphoreGive( mHT68F30Mutex ) == pdPASS);
}

void HT68F30_setStatus(const void *parms)
{
    xHMISystemParams* pSystemParms = (xHMISystemParams*)parms;

    ht68f30_lock();
    mHT68F30Parms.mute_state = pSystemParms->mute;
    mHT68F30Parms.volume = pSystemParms->master_gain;
    ht68f30_unlock();
}

void HT68F30_LightBarControl( uint8 position, uint8 duty )
{
    uint8 valDuty, valAddr;
    
    ht68f30_lock();
    mLightBarParams[position].duty = duty;
    valDuty = mLightBarParams[position].duty;
    valAddr = mLightBarParams[position].light_addr;
    ht68f30_unlock();
    
    HT68F30_WriteI2C_Byte( valAddr, valDuty );
}

static void _CleanALL(uint8 StartPos, uint8 EndPos,uint16 Holdtime, bool background)
{
    uint8 position;
    uint8 StartIndex;
    uint8 EndIndex;
    uint8 i;
    /* If start and end are in volume section, clean all volume section */
    /* otherwise start and end are in format section, clean all format section */

    if(( StartPos >= MIN_FORMAT_POS)&&( EndPos <= MAX_FORMAT_POS))
    {
        StartIndex = MIN_FORMAT_POS;
        EndIndex = MAX_FORMAT_POS;
    }
    else
    {
        StartIndex = MIN_VOL_POS;
        EndIndex = MAX_VOL_POS;
    }

    for (i = StartIndex; i <= EndIndex; i = i + DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        if(background == FALSE)
        {
            mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_DARK;
            mNewLightBarParams[position].effect = HT68F30_DARK;
        }
        else
        {
            mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_LOW;
            mNewLightBarParams[position].effect = HT68F30_FADE_TO_DARK;
        }

        mNewLightBarParams[position].time_stamp = Holdtime + 60;
        mNewLightBarParams[position].changed= TRUE;
    }
}

void HT68F30_CleanALL(uint8 StartPos, uint8 EndPos,uint16 Holdtime, bool background)
{
    ht68f30_lock();
    _CleanALL(StartPos, EndPos, Holdtime, background);
    ht68f30_unlock();
}

void HT68F30_CleanLED(uint8 StartPos, uint8 EndPos,uint16 Holdtime, bool background)
{
    uint8 position;
    uint8 i;

    ht68f30_lock();
    for (i = StartPos; i <= EndPos; i = i + DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        if(background == FALSE)
        {
            mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_DARK;
            mNewLightBarParams[position].effect = HT68F30_DARK;
        }
        else
        {
            mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_LOW;
            mNewLightBarParams[position].effect = HT68F30_FADE_TO_DARK;
        }

        mNewLightBarParams[position].time_stamp = Holdtime + 60;
        mNewLightBarParams[position].changed= TRUE;
    }
    ht68f30_unlock();
}

void HT68F30_LightConverter(uint8 StartPos, uint8 EndPos)
{
    uint8 LightSum;
    uint8 LightRemainder;
    uint8 position;
    uint8 LastPos;
    uint8 i;
    LightSum = EndPos - StartPos;
    LightRemainder = LightSum%DUTY1_NUM;
    LastPos = (EndPos + LightRemainder)/DUTY1_NUM;

    ht68f30_lock();
    _CleanALL(StartPos,EndPos,0,FALSE);

    for (i = StartPos; i<=EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        if (position == LastPos)
        {
            mNewLightBarParams[position].duty = mLightBar_Duty1[LightRemainder];
        }
        else
        {
            mNewLightBarParams[position].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        }
        mNewLightBarParams[position].effect = HT68F30_NORMAL;
        mNewLightBarParams[position].time_stamp = 0;
        mNewLightBarParams[position].changed= TRUE;
    }
    ht68f30_unlock();
}

void HT68F30_Set(uint8 StartPos, uint8 EndPos)
{
    uint8 LightSum;
    uint8 LightRemainder;
    uint8 position;
    uint8 LastPos;
    uint8 i;
    LightSum = EndPos - StartPos;
    LightRemainder = LightSum%DUTY1_NUM;
    LastPos = (EndPos + LightRemainder)/DUTY1_NUM;

    ht68f30_lock();
    for (i = StartPos; i<=EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        if (position == LastPos)
        {
            mNewLightBarParams[position].duty = mLightBar_Duty1[LightRemainder];
        }
        else
        {
            mNewLightBarParams[position].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        }
        mNewLightBarParams[position].effect = HT68F30_NORMAL;
        mNewLightBarParams[position].time_stamp = 0;
        mNewLightBarParams[position].changed= TRUE;
    }
    ht68f30_unlock();
}

void HT68F30_BlinkFadeToDarkConverter(uint8 StartPos, uint8 EndPos,uint16 Holdtime,uint16 duration, uint16 times)
{
    uint8 position;
    uint8 i;

    ht68f30_lock();
    _CleanALL(StartPos,EndPos,0,FALSE);

    nFlashSpeedTarget = duration;
    nFlashSpeed = 0;
    nFlashCount = times;
    bFlasgingVector = FALSE;

    /* Light the LED */
    for (i = StartPos; i<=EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_DARK;
        mNewLightBarParams[position].effect = HT68F30_BLINK_FADE_TO_DARK;
        mNewLightBarParams[position].time_stamp = Holdtime + 60;
        mNewLightBarParams[position].changed= TRUE;
    }
    ht68f30_unlock();
}

void _FadeToDarkConverter(uint8 StartPos, uint8 EndPos,uint16 Holdtime,bool Foreground,bool Background,bool Clean)
{
    uint8 LightSum;
    uint8 LightRemainder;
    uint8 position;
    uint8 LastPos;
    uint8 i;
    LightSum = EndPos - StartPos;
    LightRemainder = LightSum % DUTY1_NUM;
    LastPos = EndPos/DUTY1_NUM;

    if (Clean)
    {
        _CleanALL(StartPos,EndPos,Holdtime,Background);
    }
    /* Light the LED */
    for (i = StartPos; i<=EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;
        if(Foreground == TRUE)
        {
        if (position == LastPos)
        {
            mNewLightBarParams[position].duty = mLightBar_Duty1[LightRemainder];
        }
        else
        {
            mNewLightBarParams[position].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        }

        mNewLightBarParams[position].effect = HT68F30_FADE_TO_DARK;
        }
        else
        {
            mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_DARK;
            mNewLightBarParams[position].effect = HT68F30_DARK;
        }

        mNewLightBarParams[position].time_stamp = Holdtime + 60;
        mNewLightBarParams[position].changed= TRUE;
    }
}

void HT68F30_FadeToDarkConverter(uint8 StartPos, uint8 EndPos,uint16 Holdtime,bool Foreground,bool Background,bool Clean)
{
    ht68f30_lock();
    _FadeToDarkConverter(StartPos, EndPos, Holdtime, Foreground, Background, Clean);
    ht68f30_unlock();
}

static void _BreathConverter(uint8 StartPos, uint8 EndPos)
{
    uint8 position;
    uint8 i;

    _CleanALL(StartPos,EndPos,0,FALSE);

    for (i = StartPos; i<=EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;
        mNewLightBarParams[position].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        mNewLightBarParams[position].effect = HT68F30_BREATH;
        mNewLightBarParams[position].time_stamp = 0;
        mNewLightBarParams[position].changed= TRUE;
    }
}

void HT68F30_BreathConverter(uint8 StartPos, uint8 EndPos)
{
    ht68f30_lock();
    _BreathConverter(StartPos, EndPos);
    ht68f30_unlock();
}

void HT68F30_BreathClear(uint8 StartPos, uint8 EndPos)
{
    uint8 i;

    i = EndPos % DUTY1_NUM;
    if ( i == 0 )
    {
	EndPos = EndPos +3;
    }
    else if ( i == 1 )
    {
	EndPos = EndPos +2;
    }
    else if ( i == 2)
    {
	EndPos = EndPos +1;
    }

    HT68F30_CleanLED(EndPos,MAX_VOL_POS,0,FALSE);
}

void HT68F30_ScanningConverter(uint8 StartPos, uint8 EndPos)
{
    uint8 position;
    uint8 i;

    ht68f30_lock();
    _CleanALL(StartPos,EndPos,0,FALSE);

    for (i = StartPos; i<EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_DARK;
        mNewLightBarParams[position].effect = HT68F30_SCANNING;
        mNewLightBarParams[position].time_stamp = 0;
        mNewLightBarParams[position].changed= TRUE;
    }
    ht68f30_unlock();
}

static void _StepsConverter(uint8 StartPos, uint8 EndPos)
{
    uint8 LightSum;
    uint8 LightRemainder;
    uint8 position;
    uint8 i;
    LightSum = EndPos - StartPos;
    LightRemainder = LightSum % DUTY1_NUM;

    _CleanALL(StartPos,EndPos,0,FALSE);
    nStepsEndNum = ((EndPos-LightRemainder)/DUTY1_NUM)+1;
    nStepsNum = ((StartPos < LightRemainder) ? 1 : ((EndPos-LightRemainder)/DUTY1_NUM)+1);
    nStepsCount = 0;


    for (i = StartPos; i<=EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_DARK;
        mNewLightBarParams[position].effect = HT68F30_STEPS;
        mNewLightBarParams[position].time_stamp = 0;
        mNewLightBarParams[position].changed= TRUE;
    }
}

void HT68F30_StepsConverter(uint8 StartPos, uint8 EndPos)
{
    ht68f30_lock();
    _StepsConverter(StartPos, EndPos);
    ht68f30_unlock();
}

void HT68F30_StepsClear(uint8 StartPos, uint8 EndPos)
{
    uint8 LightSum;
    uint8 LightRemainder;
    LightSum = EndPos - StartPos;
    LightRemainder = LightSum % DUTY1_NUM;

    ht68f30_lock();
    nStepsEndNum = ((EndPos-LightRemainder)/DUTY1_NUM)+1;
    ht68f30_unlock();
}

void HT68F30_DarkConverter(uint8 StartPos, uint8 EndPos)
{
    uint8 position;
    uint8 i;

    ht68f30_lock();
    for (i = StartPos; i<EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;
        mNewLightBarParams[position].duty = LIGHT_BAR_DUTY_DARK;

        mNewLightBarParams[position].effect = HT68F30_DARK;
        mNewLightBarParams[position].time_stamp = 0;
        mNewLightBarParams[position].changed= TRUE;
    }
    ht68f30_unlock();
}

void HT68F30_Flashing(uint8 StartPos, uint8 EndPos,uint16 duration, uint16 times,bool StandbyLED)
{
    uint8 position;
    uint8 i;

    ht68f30_lock();
    _CleanALL(StartPos,MAX_VOL_POS,0,FALSE);

    nFlashSpeedTarget = duration;
    nFlashSpeed = duration;
    nFlashCount = times;
    bFlasgingVector = TRUE;
    bStandby = StandbyLED;
    if (StartPos!=MIN_VOL_POS)
    {
        mLightBarParams[0].time_stamp = 0;
        mLightBarParams[0].duty = LIGHT_BAR_DUTY_HIGH;
        mLightBarParams[0].effect = HT68F30_NORMAL;
        mLightBarParams[0].changed= TRUE;
    }
    /* Light the LED */
    for (i = StartPos; i<=EndPos; i = i+DUTY1_NUM)
    {
        position = i/DUTY1_NUM;

        mNewLightBarParams[position].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        mNewLightBarParams[position].effect = HT68F30_FLASHING;
        mNewLightBarParams[position].time_stamp = 0;
        mNewLightBarParams[position].changed= TRUE;
    }
    ht68f30_unlock();
}

void HT68F30_Version(void)
{
    uint8 i = 0;
    uint8 tmp,tmp2;

    ht68f30_lock();
    _CleanALL(0,MAX_FORMAT_POS,0,FALSE);
    tmp = (uint8)(VERSION>>8) & 0xFF;
    tmp2 = tmp/16*10;
    tmp2 += tmp%16;
    tmp2 <<= 4;

    for (i=0; i < 4; i++)
    { // first 4 LED
        if ((tmp2<<i) & 0x80)
        {
            mNewLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        }
        else
        {
            mNewLightBarParams[i].duty = mLightBar_Duty1[MIN_DUTY1_INDEX];
        }
        mNewLightBarParams[i].effect = HT68F30_NORMAL;
        mNewLightBarParams[i].time_stamp = 0;
        mNewLightBarParams[i].changed= TRUE;
    }

    tmp = (uint8)(VERSION&0x00F0);
    for (; i < 8; i++)
    { // min 4 LED
        if ((tmp<<(i-4)) & 0x80)
        {
            mNewLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        }
        else
        {
            mNewLightBarParams[i].duty = mLightBar_Duty1[MIN_DUTY1_INDEX];
        }
        mNewLightBarParams[i].effect = HT68F30_NORMAL;
        mNewLightBarParams[i].time_stamp = 0;
        mNewLightBarParams[i].changed= TRUE;
    }

    tmp = (uint8)(VERSION&0X000F);
    tmp <<= 4;
    for ( ; i < LED_VOL_LEVEL_NUM; i++ ) // last 4 LED
    {
        //TRACE_DEBUG((0,"LED_VOL_LEVEL_NUM=%X",LED_VOL_LEVEL_NUM));
        if ((tmp<<(i-8)) & 0x80)
        {
            mNewLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
        }
        else
        {
            mNewLightBarParams[i].duty = mLightBar_Duty1[MIN_DUTY1_INDEX];
        }
        mNewLightBarParams[i].effect = HT68F30_NORMAL;
        mNewLightBarParams[i].time_stamp = 0;
        mNewLightBarParams[i].changed= TRUE;
    }
    ht68f30_unlock();
}

/*Enter demo mode show vol*/
void HT68F30_UpdateDemoShowVolState(bool state)
{
    ht68f30_lock();
    LightBar_DemoShowVol = state;
    ht68f30_unlock();
}

void _DemoShowVol(void)
{
    if((!mHT68F30Parms.mute_state)&& LightBar_DemoShowVol)
    {
        _FadeToDarkConverter(MIN_VOL_POS, mHT68F30Parms.volume,10,TRUE,FALSE,TRUE);
        LightBar_DemoShowVol = FALSE;
    }
}
/*Resume mute status*/
static void _MuteLED(void)
{
    if(mHT68F30Parms.mute_state)
    {
        _BreathConverter(MIN_VOL_POS, mHT68F30Parms.volume);
    }
}

/*Demo mode time out*/
void HT68F30_UpdateDemoTimeOutState(bool state)
{
    ht68f30_lock();
    LightBar_DemoTimeOut = state;
    ht68f30_unlock();
}

static void _DemoTimeOutLED(void)
{
    if(LightBar_DemoTimeOut)
    {
        _StepsConverter(MIN_VOL_POS,MAX_VOL_POS);
    }
}

bool HT68F30_IsControllingLED(void)
{
    ht68f30_lock();
    if(mLightBarParams[0].effect == HT68F30_NORMAL &&
        mLightBarParams[0].changed == FALSE)
    {
        ht68f30_unlock();
        return FALSE;
    }
    else
    {
        ht68f30_unlock();
    }
    return TRUE;
}

void HT68F30_LightBar_task( void *pvParameters )
{
    bool bChange;
    uint8 changeAddr, changeDuty;
    
    for(;;)
    {
        static bool bBreatheVector = TRUE;/* conunt down */
        static bool bScanningVector = TRUE;/* left to right */
        static bool bStepsVector = TRUE;/* left to right */
        uint8 i;
        uint8 nDuty;

        for(i=0 ; i <= MAX_LED_LEVEL_INDEX; i++)
        {
            ht68f30_lock();
            if (i == 0)
            {
                /* Breath */
                if (bBreatheVector == FALSE)
                {
                    nDuty = nDuty - 5;
                    if (nDuty < 15)
                    {
                        bBreatheVector = TRUE;
                    }
                }
                else if (bBreatheVector == TRUE)
                {
                    nDuty = nDuty + 5;
                    if (nDuty >=100)
                    {
                        bBreatheVector = FALSE;
                    }
                }

                /* Scaning*/
                if (bScanningVector == FALSE) /*right to left*/
                {
                    if (nScanCount%1==0)
                    {   /*0.1 sec move one step*/
                        nScanNum = nScanNum - 1;
                        if (nScanNum == 0)
                        {
                            bScanningVector = TRUE;
                        }
                    }
                    nScanCount++;

                }
                else if (bScanningVector == TRUE) /*left to right*/
                {
                    if (nScanCount%1==0)
                    {
                        nScanNum = nScanNum + 1;
                        if (nScanNum == LED_VOL_NUMBER-1)
                        {
                            bScanningVector = FALSE;
                        }
                    }
                    nScanCount++;
                }

                /* Steps*/
                if (bStepsVector == FALSE) /*right to left*/
                {
                    if (nStepsCount%1==0)
                    {   /*0.3 sec move one step*/
                        nStepsNum = nStepsNum - 1;
                        if (nStepsNum == 0)
                        {
                            bStepsVector = TRUE;
                        }
                    }
                    nStepsCount++;
                }
                else if (bStepsVector == TRUE) /*left to right*/
                {
                    if (nStepsCount%1==0)
                    {
                        nStepsNum = nStepsNum + 1;
                        if (nStepsNum >nStepsEndNum)
                        {
                            bStepsVector = FALSE;
                            nStepsNum = nStepsNum - 1;
                        }
                    }
                    nStepsCount++;
                }

                /* Flashing*/
                if (bFlasgingVector == TRUE)
                {
                    nFlashSpeed --;
                    if (nFlashSpeed == 0)
                    {
                        bFlasgingVector = FALSE;
                    }
                }
                else if (bFlasgingVector == FALSE)
                {
                    nFlashSpeed ++;
                    if (nFlashSpeed == nFlashSpeedTarget)
                    {
                        if (nFlashCount!=0xFF)
                        {
                            nFlashCount--;
                        }
                        bFlasgingVector = TRUE;
                    }
                }
            }

            if (mNewLightBarParams[i].changed)
            {
                mLightBarParams[i].duty = mNewLightBarParams[i].duty;
                mLightBarParams[i].effect = mNewLightBarParams[i].effect;
                mLightBarParams[i].time_stamp= mNewLightBarParams[i].time_stamp;
                mLightBarParams[i].changed= mNewLightBarParams[i].changed;
                mNewLightBarParams[i].changed = FALSE;
            }

            switch(mLightBarParams[i].effect)
            {
                case HT68F30_BLINK_FADE_TO_DARK:
                {
                    if (bFlasgingVector ==TRUE)
                    {
                        mLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
                    }
                    else
                    {
                        mLightBarParams[i].duty = LIGHT_BAR_DUTY_DARK;
                    }

                    if(nFlashCount == 0)
                    {
                        mLightBarParams[i].time_stamp = 60;
                        mLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
                        mLightBarParams[i].effect = HT68F30_FADE_TO_DARK;
                    }

                    mLightBarParams[i].changed= TRUE;
                }

                case HT68F30_FADE_TO_DARK:
                {
                    mLightBarParams[i].time_stamp--;

                    /* Stay at full brightness for 1.5 ,3,15 seconds */
                    if (mLightBarParams[i].time_stamp > 6 && mLightBarParams[i].time_stamp <= 60)
                    {
                        /*Wait for half duty, only reduce the full brightness*/
                        if ( mLightBarParams[i].duty > LIGHT_BAR_DUTY_LOW )
                        {
                            mLightBarParams[i].duty = mLightBarParams[i].duty - 1;
                        }
                    }
                    else if (mLightBarParams[i].time_stamp > 0 && mLightBarParams[i].time_stamp <= 6)
                    {
                    if ( mLightBarParams[i].duty != LIGHT_BAR_DUTY_DARK )
                    {
                            mLightBarParams[i].duty = mLightBarParams[i].duty - 1;
                        }
                    }
                    /* LIGHT_BAR_DUTY_HIGH is 0d60, so after 3 sec it become bark */
                    else if ( mLightBarParams[i].time_stamp == 0 )
                    {
                        mLightBarParams[i].time_stamp = 0;
                        mLightBarParams[i].duty = LIGHT_BAR_DUTY_DARK;
                        mLightBarParams[i].effect = HT68F30_DARK;

                        if (i<=MAX_LED_LEVEL_INDEX)/* Only the volume part,no status part*/
                        {
                            /* After FTD, turn on Power light(First one) */
                            mLightBarParams[0].time_stamp = 0;
                            mLightBarParams[0].duty = mLightBar_Duty1[MIN_DUTY1_INDEX];
                            mLightBarParams[0].effect = HT68F30_NORMAL;
                            mLightBarParams[0].changed= TRUE;
                            _MuteLED();/*resume mute status*/
                            _DemoTimeOutLED();  
                        }
                    }
                    mLightBarParams[i].changed= TRUE;
                }
                    break;

                case HT68F30_BREATH:
                {
                    mLightBarParams[i].duty = LIGHT_BAR_DUTY(nDuty);
                    mLightBarParams[i].changed= TRUE;
                }
                    break;

                case HT68F30_SCANNING:
                {
                    if (i == nScanNum)
                    {
                        mLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
                    }
                    else
                    {
                        mLightBarParams[i].duty = LIGHT_BAR_DUTY_DARK;
                    }
                    mLightBarParams[i].changed= TRUE;
                }
                    break;

                case HT68F30_STEPS:
                {
                    if (i < nStepsNum)
                    {
                        mLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
                    }
                    else
                    {
                        mLightBarParams[i].duty = LIGHT_BAR_DUTY_DARK;
                    }
                    mLightBarParams[i].changed= TRUE;
                }
                    break;

                case HT68F30_FLASHING:
                {
                    if (bFlasgingVector ==TRUE)
                    {
                        mLightBarParams[i].duty = mLightBar_Duty1[MAX_DUTY1_INDEX];
                    }
                    else
                    {
                        mLightBarParams[i].duty = LIGHT_BAR_DUTY_DARK;
                    }

                    if(nFlashCount == 0)
                    {
                        mLightBarParams[i].time_stamp = 0;
                        mLightBarParams[i].duty = LIGHT_BAR_DUTY_DARK;
                        mLightBarParams[i].effect = HT68F30_DARK;

                        if (i<=MAX_LED_LEVEL_INDEX)/* Only the volume part,no status part*/
                        {
                            if (bStandby)
                            {
                                /* After FTD, turn on Power light(First one) */
                                mLightBarParams[0].time_stamp = 0;
                                mLightBarParams[0].duty = mLightBar_Duty1[MIN_DUTY1_INDEX];
                                mLightBarParams[0].effect = HT68F30_NORMAL;
                                mLightBarParams[0].changed= TRUE;
                                _MuteLED();/*resume mute status*/
                                _DemoTimeOutLED();
                                _DemoShowVol();
                            }
                        }
                    }

                    mLightBarParams[i].changed= TRUE;
                }
                    break;

                case HT68F30_NORMAL:
                case HT68F30_DARK:
                default:
                   /*Don't need to change duty, but clean the time stamp*/

                    break;
            }

            bChange = mLightBarParams[i].changed;
            mLightBarParams[i].changed = FALSE;
            changeAddr = mLightBarParams[i].light_addr;
            changeDuty = mLightBarParams[i].duty;
            
            ht68f30_unlock();

            //Angus modify , It can improve led state at hdmi src when writing sil95ee error with I2C bus.
            if (bChange)
            {
                if(HT68F30_WriteI2C_Byte(changeAddr, changeDuty) == FALSE)
                {
                    ht68f30_lock();
                    mLightBarParams[i].changed = TRUE;
                    ht68f30_unlock();
                }
            }
        }
        vTaskDelay(TASK_MSEC2TICKS(50));
    }
}

static void HT68F30_LightBar_CreateTask( void )
{
    if(mHT68F30Mutex == NULL)
        mHT68F30Mutex  = xSemaphoreCreateMutex();

    if ( xTaskCreate( HT68F30_LightBar_task,
            ( portCHAR * ) "HT68F30_LightBar_task",
            (STACK_SIZE), NULL, tskH68F30_PRIORITY,&mHT68F30Parms.HT68F30_TaskHandle) != pdPASS )
    {
        TRACE_ERROR((0, "HT68F30_LightBar_task task create failure " ));
    }
}


