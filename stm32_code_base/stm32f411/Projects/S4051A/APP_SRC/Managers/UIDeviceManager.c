#include "Defs.h"
#include "Debug.h"
#include "api_typedef.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "UIDeviceManager.h"
#include "AudioSystemHandler.h"
#include "HT68F30.h"
//______________________________________________________________________________
//______________________________________________________________________________

/*  Sound Bar LED S3851
level:0~                               35  36 ~41
    _____________________________________________
   |  *  *  *  *  *  *  *  *  *  *  *  *   *  *  |   
      C  B  A  9  8  7  6  5  4  3  2  1 | D  E   
               LED_VOL_NUMBER              LED_FORMAT_NUMBER
*/
#define UI_LED_START   0
#define UI_LED_END     41
#define UI_LEDC_START  0
#define UI_LEDC_END    2
#define UI_LEDB_START  3
#define UI_LEDB_END    5
#define UI_LEDA_START  6
#define UI_LEDA_END    8
#define UI_LED9_START  9
#define UI_LED9_END    11
#define UI_LED8_START  12
#define UI_LED8_END    14
#define UI_LED7_START  15
#define UI_LED7_END    17
#define UI_LED6_START  18
#define UI_LED6_END    20
#define UI_LED5_START  21
#define UI_LED5_END    23
#define UI_LED4_START  24
#define UI_LED4_END    26
#define UI_LED3_START  27
#define UI_LED3_END    29
#define UI_LED2_START  30
#define UI_LED2_END    32
#define UI_LED1_START  33
#define UI_LED1_END    35

#define UI_VOL_START   0
#define UI_VOL_END     35
#define UI_FOM_START   36
#define UI_FOM_END     41
#define UI_DD_START    36
#define UI_DD_END      38
#define UI_DTS_START   39
#define UI_DTS_END     41
#define UI_PRO_START   12  /*Process mode*/
#define UI_PRO_END     23

#define UI_CENTER_GAIN 6

#define Hold1_5sec 10/*fine tuned*/
#define Hold3sec   60
#define UIBounceTime TASK_MSEC2TICKS(1500)
//______________________________________________________________________________
extern LED_OBJECT *pLED_ObjCtrl;
extern AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl;
//______________________________________________________________________________
/*static variable*/
static bool adjustTrebleBass = FALSE;
const static uint8 avdelay[]={0,2,3,5,6,8,9,11,12,14,15,17,18,20,21,23,24,26,27,29,30,32,33,35};
static bool UIFirstPowerVolramp;
//static variable
static xUIDeviceParms mUIDevParms = 
{
    //TRUE, //RF_Paired; 
    //TRUE,//BT_Paired; 
    DEFAULT_MUTE_STATE, //mute;
    DEFAULT_MASTER_GAIN //master_gain;
};
/*read only for this parameters*/
const xUIDeviceParms *pUIDevParms = &mUIDevParms;
//______________________________________________________________________________
//static API header

static void UIDeviceManager_handleCommand(const void *parms );
void UIDeviceManager_CreateTask( void );
static void UIDeviceManager_Exception_handleCommand( xUIExceptionEvents UIExcept );

const UI_DEV_MANAGER_OBJECT UDM_ObjCtrl =
{
    UIDeviceManager_CreateTask,
    UIDeviceManager_handleCommand,
    UIDeviceManager_Exception_handleCommand
};

const UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl = &UDM_ObjCtrl;

void UIDeviceManager_SetStatus( const void *parms )
{
    xHMISystemParams* pSystemParms = (xHMISystemParams*)parms;

    switch(pSystemParms->sys_event)
    {
        case SYS_EVENT_VOLUME_SET:
        {
            mUIDevParms.master_gain = pSystemParms->master_gain;
        }
            break;
    
        case SYS_EVENT_VOLUME_MUTE_SET:
        {
            mUIDevParms.mute = pSystemParms->mute;
        }
            break;

        default:
            break;
    }
}

void UIDeviceManager_Exception_SetStatus( xUIExceptionEvents UIExcept )
{
    switch(UIExcept)
    {            
        default:
            break;
    }
}

bool UIDeviceManager_Debounce(const void *parms)
{
    xHMISystemParams* pSystemParms = (xHMISystemParams*)parms;

    static portTickType xUI_BounceTick = 0;
    portTickType UI_BounceDuration;
    static xHMIAudioSysEvents LastEvent;

    UI_BounceDuration = ( xTaskGetTickCount( ) - xUI_BounceTick );
    xUI_BounceTick = xTaskGetTickCount( );
    
    bool ret = FALSE;
    
    if ( (UI_BounceDuration/portTICK_RATE_MS) < UIBounceTime )    // 1.5 sec
    {
        switch( pSystemParms->sys_event )
        {
            case SYS_EVENT_VOLUME_SET:    
            {
                if (LastEvent == SYS_EVENT_VERSION)
                {
                    ret = TRUE;
                }
            }
                break;
                
            default:
            {
                ret = FALSE;
            }
                break;
        }  
    }
    
    LastEvent = pSystemParms->sys_event;

    return ret;
}

static void UIDeviceManager_handleCommand( const void *parms )
{
    uint8 holdtime;
    
    xHMISystemParams* pSystemParms = (xHMISystemParams*)parms;
    
    if ( parms == NULL )
        return;        
    
    if (UIDeviceManager_Debounce(pSystemParms))
    {
        return;
    }
    UIDeviceManager_SetStatus(pSystemParms);
    pLED_ObjCtrl->setStatus(pSystemParms);/*Update mute status to the driver*/

    UIFirstPowerVolramp = FALSE;

#if 1   //Angus added it following UI 4.3 Spec 7.1.1 for bug fixing SB4051_C0-93 
    if(pSystemParms->sys_event == SYS_EVENT_BASS_SET ||
       pSystemParms->sys_event == SYS_EVENT_TREBLE_SET)
    {
        adjustTrebleBass = TRUE;
    }
    else if(pSystemParms->sys_event == SYS_EVENT_VOLUME_SET)
    {
        if(adjustTrebleBass && pLED_ObjCtrl->IsControllingLED())
        {
            return;
        }
        else
        {
            adjustTrebleBass = FALSE;
        }
            
    }
    else
    {
        adjustTrebleBass = FALSE;
    }
#endif

    switch( pSystemParms->sys_event )
    {
        case SYS_EVENT_SYSTEM_UP:
#if ( configSII_DEV_953x_PORTING == 1)
        case SYS_EVENT_CEC_SYSTEM_UP:
        case SYS_EVENT_SAC_SYSTEM_UP:
        case SYS_EVENT_ARC_SYSTEM_UP:
#endif
        {
            pLED_ObjCtrl->LightConverter(UI_LED_START, UI_LEDC_END);
            if (pSystemParms->auto_input_detect == FALSE)
            {
                UIFirstPowerVolramp = TRUE;
            }
        }
            break;

        case SYS_EVENT_SYSTEM_DOWN:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
        }
            break;    

        case SYS_EVENT_SEL_SRC:
        case SYS_EVENT_AUTO_SEL_SRC:
        {
            if (pSystemParms->sys_event==SYS_EVENT_SEL_SRC)
            {
                holdtime = Hold1_5sec;    
            }
            else
            {
                holdtime = Hold3sec;
            }
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);/* Clean formate led */
            if (pSystemParms->input_src<=5)
            {
                /*Smith fixes bug: when AUX1 is enum 0, and (0-1) is negative value, it makes system crash*/
                pLED_ObjCtrl->BlinkFadeToDarkConverter((pSystemParms->input_src-AUDIO_SOURCE_AUX1)*6,(pSystemParms->input_src-AUDIO_SOURCE_AUX1)*6+5,holdtime,3,2); 
            }
            else if (pSystemParms->input_src>5 && pSystemParms->input_src<=10)
            {
                pLED_ObjCtrl->BlinkFadeToDarkConverter(UI_LED_START,(pSystemParms->input_src-AUDIO_SOURCE_AUX1-5)*6+11,holdtime,3,2);
                pLED_ObjCtrl->CleanLED(6,(pSystemParms->input_src-5)*6+5,0,FALSE);
            }    
        }
            break;

        case SYS_EVENT_VOLUME_SET:
        {
            if( pSystemParms->master_gain==0)
            {
                pLED_ObjCtrl->BreathConverter( UI_LED_START, UI_LEDC_END);
            }
            else
            {
                if (pSystemParms->mute ==FALSE)
                {
                    pLED_ObjCtrl->FadeToDarkConverter( UI_LED_START, pSystemParms->master_gain,Hold1_5sec,TRUE,FALSE,TRUE);
                }
                else
                {
                    pLED_ObjCtrl->BreathClear(UI_LED_START,pSystemParms->master_gain);
                }
            }
        }
            break;

        case SYS_EVENT_VOLUME_MUTE_SET:
        {
            if (pSystemParms->mute == TRUE)
            {
                pLED_ObjCtrl->BreathConverter( UI_LED_START, pSystemParms->master_gain); 
            }
            else
            {
                if( pSystemParms->master_gain==0)
                {
                    pLED_ObjCtrl->BreathConverter( UI_LED_START, UI_LEDC_END);
                }
                else
                {
                    pLED_ObjCtrl->FadeToDarkConverter( UI_LED_START , pSystemParms->master_gain,Hold1_5sec,TRUE,FALSE,TRUE); 
                }
            }
        }
            break;

        case SYS_EVENT_BASS_SET:
        {
            if(pSystemParms->bass_gain>UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 18+ 3*(pSystemParms->bass_gain-7),20 + 3*(pSystemParms->bass_gain-7) ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else if (pSystemParms->bass_gain<UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 3*(pSystemParms->bass_gain) ,2+ 3*(pSystemParms->bass_gain),Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else
            {
                pLED_ObjCtrl->FadeToDarkConverter( 15 , 20 ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            
        }
            break;

        case SYS_EVENT_TREBLE_SET:
        {
            if(pSystemParms->treble_gain>UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 18+ 3*(pSystemParms->treble_gain-7),20 + 3*(pSystemParms->treble_gain-7) ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else if (pSystemParms->treble_gain<UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 3*(pSystemParms->treble_gain) ,2+ 3*(pSystemParms->treble_gain),Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else
            {
                pLED_ObjCtrl->FadeToDarkConverter( 15 , 20 ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            
        }
            break;
            
        case SYS_EVENT_SUB_SET:
        {
            if(pSystemParms->sub_gain>UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 18+ 3*(pSystemParms->sub_gain-7),20 + 3*(pSystemParms->sub_gain-7) ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else if (pSystemParms->sub_gain<UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 3*(pSystemParms->sub_gain) ,2+ 3*(pSystemParms->sub_gain),Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else
            {
                pLED_ObjCtrl->FadeToDarkConverter( 15 , 20 ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            
        }
            break;

        case SYS_EVENT_CENTER_SET:
        {
            if(pSystemParms->center_gain>UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 18+ 3*(pSystemParms->center_gain-7),20 + 3*(pSystemParms->center_gain-7) ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else if (pSystemParms->center_gain<UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 3*(pSystemParms->center_gain) ,2+ 3*(pSystemParms->center_gain),Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else
            {
                pLED_ObjCtrl->FadeToDarkConverter( 15 , 20 ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            
        }
            break;
            
        case SYS_EVENT_LSRS_SET:
        {
            if(pSystemParms->LsRs_gain>UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 18+ 3*(pSystemParms->LsRs_gain-7),20 + 3*(pSystemParms->LsRs_gain-7) ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else if (pSystemParms->LsRs_gain<UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 3*(pSystemParms->LsRs_gain) ,2+ 3*(pSystemParms->LsRs_gain),Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else
            {
                pLED_ObjCtrl->FadeToDarkConverter( 15 , 20 ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            
        }
            break;
                
        case SYS_EVENT_BALANCE_SET:
        {
            if(pSystemParms->balance>UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 18+ 3*(pSystemParms->balance-7),20 + 3*(pSystemParms->balance-7) ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else if (pSystemParms->balance<UI_CENTER_GAIN)
            {
                pLED_ObjCtrl->FadeToDarkConverter( 3*(pSystemParms->balance) ,2+ 3*(pSystemParms->balance),Hold1_5sec,TRUE,TRUE,TRUE);
            }
            else
            {
                pLED_ObjCtrl->FadeToDarkConverter( 15 , 20 ,Hold1_5sec,TRUE,TRUE,TRUE);
            }
            
        }
            break;
            
        case SYS_EVENT_SRS_TRUVOL_SET:
        {
            pLED_ObjCtrl->FadeToDarkConverter( UI_PRO_START,UI_PRO_END,30,pSystemParms->srs_truvol,TRUE,TRUE);
        }
            break;
            
        case SYS_EVENT_SRS_TSHD_SET:
        {
            pLED_ObjCtrl->FadeToDarkConverter( UI_PRO_START,UI_PRO_END,30,pSystemParms->srs_tshd,TRUE,TRUE);
        }
            break;

        case SYS_EVENT_VIZ_RMT_SET:
        {
            pLED_ObjCtrl->FadeToDarkConverter( UI_PRO_START,UI_PRO_END,30,pSystemParms->vizio_rmt,TRUE,TRUE);
        }
            break;

        case SYS_EVENT_APD_SET:
        {
            pLED_ObjCtrl->FadeToDarkConverter( UI_PRO_START,UI_PRO_END,30,pSystemParms->apd,TRUE,TRUE);
        }
            break;

        case SYS_EVENT_NIGHT_MODE_SET:
        {
            pLED_ObjCtrl->FadeToDarkConverter( UI_PRO_START,UI_PRO_END,Hold1_5sec,pSystemParms->night_mode,TRUE,TRUE);
        }
            break;

        case SYS_EVENT_AV_DELAY_SET:
        {
            pLED_ObjCtrl->FadeToDarkConverter( UI_VOL_START ,avdelay[ pSystemParms->av_delay],Hold1_5sec,TRUE,FALSE,TRUE);
        }
            break;
            
        case SYS_EVENT_SYSTEM_RESET:
        {
            #if 1   //Angus added , for HT68F30 auto setting timeout led by timeout state .
            pLED_ObjCtrl->UpdateDemoTimeOutState(FALSE);
            #endif
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,12, 3,FALSE);   
        }
            break;
            
        case SYS_EVENT_VERSION:
        {
            pLED_ObjCtrl->Version();
        }
            break;
            
        case SYS_EVENT_FAC_AQ_CTRL_ON:
        {
            TRACE_DEBUG((0,"SYS_EVENT_FAC_AQ_CTRL_ON"));
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LED4_START, UI_LED3_END); 
        }
            break;

        case SYS_EVENT_FAC_AQ_CTRL_OFF:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
    }
            break;  
            
        case SYS_EVENT_FAC_ROUTE_CTRL_INITIAL:
{
            //pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            //pLED_ObjCtrl->LightConverter(UI_VOL_START, UI_VOL_END,5, 4,FALSE); 
        }
            break;
            
        case SYS_EVENT_FAC_ROUTE_CTRL_FL_FR:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LED8_START, UI_LED8_END); /*FR LED5*/
            pLED_ObjCtrl->Set(UI_LEDB_START, UI_LEDB_END); /*FL LED2*/
        }
            break;
    
        case SYS_EVENT_FAC_ROUTE_CTRL_RR_RL:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LED7_START, UI_LED7_END);/*RR LED6*/ 
            pLED_ObjCtrl->Set(UI_LEDC_START, UI_LEDC_END);/*RL LED1*/
        }
            break;
            
        case SYS_EVENT_FAC_ROUTE_CTRL_CENTER:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LEDA_START, UI_LED9_END);
        }
            break;
            
        case SYS_EVENT_FAC_ROUTE_CTRL_LFE0:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LEDC_START, UI_LED7_END);/*RR LED6*/ 
        }
            break;

        case SYS_EVENT_FAC_ROUTE_CTRL_FL:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LED8_START, UI_LED8_END); /*FL LED5*/
        }
            break;
            
        case SYS_EVENT_FAC_ROUTE_CTRL_FR:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LEDB_START, UI_LEDB_END); /*FR LED2*/
        }
            break;

        case SYS_EVENT_FAC_ROUTE_CTRL_RR: 
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LEDC_START, UI_LEDC_END); /*RR LED1*/
        }
            break;
            
        case SYS_EVENT_FAC_ROUTE_CTRL_RL:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LED7_START, UI_LED7_END); /*RL LED6*/
        }
            break;
            
        case SYS_EVENT_FAC_ROUTE_CTRL_FL_FR_CENTER:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LEDA_START, UI_LED9_END); /*LRF3 LED4*/
            pLED_ObjCtrl->Set(UI_LED8_START, UI_LED8_END); /*FR LED5*/
            pLED_ObjCtrl->Set(UI_LEDB_START, UI_LEDB_END); /*FL LED2*/
        }
            break;
    
        case SYS_EVENT_FAC_ROUTE_CTRL_RR_RL_WOOFER:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->Set(UI_LEDC_START, UI_LEDC_END);
            pLED_ObjCtrl->Set(UI_LED7_START, UI_LED7_END);        
            pLED_ObjCtrl->Set(UI_LEDA_START, UI_LED9_END); 
        }
            break;    

        case SYS_EVENT_FAC_ROUTE_CTRL_DISABLE:    
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_VOL_START, 0);
        }
            break;

        case SYS_EVENT_FAC_ALL_LED:
        {
            pLED_ObjCtrl->LightConverter(UI_LED_START, UI_LED_END);
        }
            break;

        case SYS_EVENT_FAC_ALL_LED_DD:
        {
            pLED_ObjCtrl->LightConverter(UI_LED_START, UI_VOL_END);
            pLED_ObjCtrl->LightConverter(UI_DD_START, UI_DD_END);
        }
            break;

        case SYS_EVENT_FAC_ALL_LED_DTS:
        {
            pLED_ObjCtrl->LightConverter(UI_LED_START, UI_VOL_END);
            pLED_ObjCtrl->LightConverter(UI_DTS_START, UI_DTS_END);
        }
            break;    

        case SYS_EVENT_FAC_LED1:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LEDC_START, UI_LEDC_END);
        }
            break;  

        case SYS_EVENT_FAC_LED2:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LEDB_START, UI_LEDB_END);
        }
            break; 

        case SYS_EVENT_FAC_LED3:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LEDA_START, UI_LEDA_END);
        }
            break; 
            
        case SYS_EVENT_FAC_LED4:
        {   
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED9_START, UI_LED9_END);
        }
            break;       

        case SYS_EVENT_FAC_LED5:
        {   
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED8_START, UI_LED8_END);
        }
            break;  

        case SYS_EVENT_FAC_LED6:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED7_START, UI_LED7_END);
        }
            break;  

        case SYS_EVENT_FAC_LED7:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED6_START, UI_LED6_END);
        }
            break;  

        case SYS_EVENT_FAC_LED8:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED5_START, UI_LED5_END);
        }
            break;  

        case SYS_EVENT_FAC_LED9:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED4_START, UI_LED4_END);
        }
            break;  

        case SYS_EVENT_FAC_LED10:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED3_START, UI_LED3_END);
        }
            break;            

        case SYS_EVENT_FAC_LED11:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED2_START, UI_LED2_END);
        }
            break;  

        case SYS_EVENT_FAC_LED12:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_LED1_START, UI_LED1_END);
        }
            break;  

        case SYS_EVENT_FAC_LED13:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_DD_START, UI_DD_END);
        }
            break;  

        case SYS_EVENT_FAC_LED14:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
            pLED_ObjCtrl->LightConverter(UI_DTS_START, UI_DTS_END);
        }
            break;  

        case SYS_EVENT_FAC_NO_LED:
        {
            pLED_ObjCtrl->DarkConverter(UI_LED_START,UI_LED_END);
        }
            break;

        default:
            break;
    }
}
/* TODO: change to Q */
static void UIDeviceManager_Exception_handleCommand( xUIExceptionEvents UIExcept)
{
    static xUIExceptionEvents Last_UIExcept = UI_EVENT_NONE;

#if 1   //Angus added , for HT68F30 auto setting timeout led by timeout state .
    if(UIExcept == UI_EVENT_TIMEOUT_DEMO)
    {
        pLED_ObjCtrl->UpdateDemoTimeOutState(TRUE);
    }
    else if(UIExcept == UI_EVENT_EXIT_TIMEOUT_DEMO || UIExcept == UI_EVENT_EXIT_DEMO ||
               UIExcept == UI_EVENT_ENTER_DEMO1 || UIExcept == UI_EVENT_ENTER_DEMO2 || 
               UIExcept == UI_EVENT_ENTER_DEMO3)
    {
        pLED_ObjCtrl->UpdateDemoTimeOutState(FALSE);
    }
#endif
    
    switch( UIExcept )
    {
        case UI_Event_VolRAMP: 
        {
            if (UIFirstPowerVolramp)
            {
                pLED_ObjCtrl->FadeToDarkConverter( UI_LED_START, pASH_ObjCtrl->getRampVolvalue(),Hold1_5sec,TRUE,FALSE,TRUE);
            }            
        }
            break;    

        case UI_Event_VolRAMP_TARGET: 
        {
            if (UIFirstPowerVolramp)
            {
                pLED_ObjCtrl->FadeToDarkConverter( UI_LED_START, pASH_ObjCtrl->getTargetVol(),Hold1_5sec,TRUE,FALSE,TRUE);
                UIFirstPowerVolramp = FALSE;
            }
        }
            break;   

        case UI_EVENT_EXIT_TIMEOUT_DEMO:
        case UI_EVENT_NORMAL:
        {
            if (pUIDevParms->mute == FALSE)
            {
                pLED_ObjCtrl->LightConverter(UI_VOL_START, 0);
            }
        }
            break;

        case UI_EVENT_PCM:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
        }
            break;
            
        case UI_EVENT_DOLBY:
        {
            pLED_ObjCtrl->DarkConverter(UI_DTS_START,UI_DTS_END);
            pLED_ObjCtrl->FadeToDarkConverter( UI_DD_START,UI_DD_END,Hold3sec,TRUE,FALSE,FALSE);/*UI4.2*/
        }
            break;
            
        case UI_EVENT_FAC_DOLBY:
        {
            pLED_ObjCtrl->DarkConverter(UI_DTS_START,UI_DTS_END);   
            pLED_ObjCtrl->Set( UI_DD_START,UI_DD_END);
        }
            break;

        case UI_EVENT_DTS:
        {
            pLED_ObjCtrl->DarkConverter(UI_DD_START,UI_DD_END);
            pLED_ObjCtrl->FadeToDarkConverter( UI_DTS_START,UI_DTS_END,Hold3sec,TRUE,FALSE,FALSE);/*UI4.2*/
        }
            break;

        case UI_EVENT_FAC_DTS:
        {
            pLED_ObjCtrl->DarkConverter( UI_DD_START,UI_DD_END);
            pLED_ObjCtrl->Set( UI_DTS_START,UI_DTS_START);
        }
            break;

        case UI_EVENT_PRG_REMOTE_LEARNING:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->ScanningConverter(UI_VOL_START, UI_VOL_END);  
        }
            break;
        case UI_EVENT_BT_PAIRING:
        { 
            if(UIExcept!=Last_UIExcept)
            {
                pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
                pLED_ObjCtrl->ScanningConverter(UI_VOL_START, UI_VOL_END);  
            }
        }
            break;
        case UI_EVENT_TIMEOUT_DEMO:
        {
            if(UIExcept!=Last_UIExcept)
            {
                pLED_ObjCtrl->StepsConverter(UI_VOL_START, UI_VOL_END);
            }

        }
        break;
        case UI_EVENT_RF_PAIRING:
        {
            if(UIExcept!=Last_UIExcept)
            {
                pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
                pLED_ObjCtrl->ScanningConverter(UI_VOL_START, UI_VOL_END);
            }
        }
            break;
           
        case UI_EVENT_BT_CLEAN_DEVICE:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 3,TRUE);
        }
            break;
           
        case UI_EVENT_BT_PAIRED:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 2,TRUE); 
        }
            break;

        case UI_EVENT_RF_PAIRED:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 3,TRUE); 
        }
            break;
 
        case UI_EVENT_PRG_REMOTE_LEARNED:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 2,TRUE); 
        }
            break;

        case UI_EVENT_ENTER_DEMO1:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 1,TRUE); 
        }
            break;

        case UI_EVENT_ENTER_DEMO2:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 2,TRUE); 
        }
            break;

        case UI_EVENT_ENTER_DEMO3:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 3,TRUE); 
        }
            break;

        case UI_EVENT_EXIT_DEMO:
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,5, 4,FALSE); 
        }
            break;
            
        case UI_EVENT_ULD_ERROR:    
        {
            pLED_ObjCtrl->DarkConverter(UI_FOM_START,UI_FOM_END);
            pLED_ObjCtrl->Flashing(UI_VOL_START, UI_VOL_END,3, 99,TRUE); 
        }
            break;     
            
        default:
            break;     
    }
    Last_UIExcept == UIExcept;
}

void UIDeviceManager_CreateTask( void )
{    
    pLED_ObjCtrl->CreateTask();
}

