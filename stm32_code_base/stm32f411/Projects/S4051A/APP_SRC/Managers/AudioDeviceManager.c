#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "device_config.h"
#include "audio_gain_parameters.h"
#include "GPIOMiddleLevel.h"
#include "AmplifierDeviceManager.h"
#include "AudioDeviceManager.h"
#include "UIDeviceManager.h"
#include "HdmiDeviceManager.h"
#include "BTHandler.h"

//______________________________________________________________________________
#define ADM_SIGNAL_DETECTOR_TIME_TICK TASK_MSEC2TICKS(500)
#define ADM_DIGITAL_DETECTOR_TIME_TICK TASK_MSEC2TICKS(50)
#define ADC_PATH_AUX1 4
#define ADC_PATH_AUX2 5
#define ADC_PATH_BT 0
#define ADC_PATH_USB 2
#define SRC_PATH_DITIGITAL1 1
#define SRC_PATH_DITIGITAL2 2
#define SRC_PATH_DITIGITAL3 3

#define MAX_VOL 35

#define AUDIO_DEV_BACKCTL_QUEUE_LENGTH 8
#define AUDIO_DEV_EXCEPTION_QUEUE_LENGTH 2
//______________________________________________________________________________
typedef enum 
{
    VRE_RAMP_IDLE = 0,
    VRE_SET_VOL,
    VRE_NON_RAMP_MUTE_LOCK,  //mute a little time for fixing channel check pop sound
    VRE_RAMP_WAIT_SIGNAL,
    VRE_RAMP_WAIT_BT_CONNECT,
    VRE_RAMP_MUTE_LOCK,
    VRE_RAMP_INITIAL,
    VRE_RAMP_START,
    VRE_RAMP_HOLD,
    VRE_RAMP_END,
}xVolumeRampEvents;

#if ( configSII_DEV_953x_PORTING == 1 )
typedef enum 
{
    HDMI_NON_PCM = 0,
    HDMI_PCM_STEREO,
    HDMI_PCM_MULTI_48K,
    HDMI_PCM_MULTI_96K,
}xHDMIAudiotype;
#endif

typedef struct AUDIO_MANAGER_PARAMETERS 
{
    xTaskHandle VolTaskHandle;
    xTaskHandle signal_detector_handle;
    xTaskHandle Digital_detector_handle; 
    xTaskHandle Audio_Exception_handle;
    xTaskHandle BackCtrlTaskHandle;
    xSemaphoreParameters ADM_I2C2_sema;
    xSemaphoreParameters ADM_Call_Back_sema;
    TaskHandleState Vol_state;
} xOS_ADM_Params;

/* For Back control*/
typedef struct AUDIO_DEV_BACKCTL_HANDLE_PARAMETERS
{
    TaskHandleState taskState;
    xQueueParameters QParams;
} xAudDevBackCtlHandleParams;

static xAudDevBackCtlHandleParams AudioDevBackCtlParams;

/* For Exception*/
typedef struct AUDIO_DEV_EXCEPTION_HANDLE_PARAMETERS
{
    TaskHandleState taskState;
    xQueueParameters QParams;
} xAudExceptionHandleParams;

static xAudExceptionHandleParams AudioDevExceptParams;
//______________________________________________________________________________
//static variable
static xAudDeviceParms mAudDevParms = 
{
    DEFAULT_INPUT_SRC,
    DEFAULT_MUTE_STATE, /*mute*/
    DEFAULT_MASTER_GAIN, /*master gain*/
    DEFAULT_BASS_GAIN,    /*bass gain*/
    DEFAULT_TREBLE_GAIN,    /*treble gain*/
    DEFAULT_SUB_GAIN,    /*sub gain*/
    DEFAULT_CENTER_GAIN,    /*center gain*/
    DEFAULT_BALANCE,    /*balance*/
    DEFAULT_SRS_TRUVOL,    /*srs truvol*/
    DEFAULT_SRS_TSHD,    /*srs tshd*/
    DEFAULT_NIGHT_MODE,    /*night mode*/
    DEFAULT_AV_DELAY,    /*av delay*/
};
/*read only for this parameters*/
const xAudDeviceParms *pAudioDevParms = &mAudDevParms;

static xVolumeRampEvents mVolumeRampEvent = VRE_RAMP_IDLE;
static bool mIsPCM96Khz = FALSE;
static bool mIsPCM = FALSE;
static xOS_ADM_Params xOS_ADM_Parms;
static uint8 SignalAvailable = 0;
static uint8 ramp_delay = 0;
static uint16 vre_ramp_count = 0;
static bool format_indicator = 0;
static CS49844LoadAudioStreamType CurrentAudioStream;
static uint8 volume_inc = 0;
static uint8 target_volume = 0;
//_________________________________________________________________________________
//static api header
static void AudioDeviceManager_CreateTask( void );

static void AudioDeviceManager_Initialize( void *parms );

static void AudioDeviceManager_DeviceConfig( void *parms );

static void AudioDeviceManager_DeInitialize( void );

static xAudDeviceParms AudioDeviceManager_getAudParams( void );

/* Volume control*/
void AuidoDeviceManager_VolController_task( void *pvParameters );

static void AuidoDeviceManager_VolController_CreateTask( void );

static void AuidoDeviceManager_VolController_setEvent(void *parms );

static void AuidoDeviceManager_VolController_TaskCtrl( bool val );

/* Set paramaters */
static void AudioDeviceManager_setInputPath( AUDIO_SOURCE idx );

static void AuidoDeviceManager_setBass( uint8 idx );

static void AuidoDeviceManager_setTreble( uint8 idx );

static void AuidoDeviceManager_setSub( uint8 idx );

static void AuidoDeviceManager_setCenter( uint8 idx );

static void AuidoDeviceManager_setLsRs( uint8 idx );

static void AuidoDeviceManager_setBalance( uint8 idx );

static void AuidoDeviceManager_setSRSTruVolTSHD( void *parms );

static void AuidoDeviceManager_setNightMode( bool idx );

static void AuidoDeviceManager_setAVDelay( uint8 idx );

static void AuidoDeviceManager_Factory_AQBypass( bool EQ );

static void AuidoDeviceManager_Route_Ctrl(AudioRouteCtrl router_idx);
/* Digital Detect */
static void AuidoDeviceManager_DigitalDetector_CreateTask( void );

static void AudioDeviceManager_DigitalDetector_TaskCtrl( bool val );

bool AudioStream_get_PCM96Khz( void );

void AudioStream_set_PCM96Khz( bool bValue );

bool AudioStream_get_PCM( void );

void AudioStream_set_PCM( bool bValue );

bool AudioSystem_isPCM_96Khz( bool bReboot );

void AudioSystem_Cover_PCM96Khz( void );

/* Signal Detect*/
static void AuidoDeviceManager_SignalDetector_CreateTask( void );

static void AudioDeviceManager_SignalDetector_TaskCtrl( bool val );

static uint8 AudioDeviceManager_getSignalAvailable( void );

//______________________________________________________________________________
/*LOW LEVEL*/
static void AudioDeviceManager_lowlevel_setMute( bool idx );

static void AuidoDeviceManager_lowlevel_setMasterGain( uint8 idx );

static void AuidoDeviceManager_lowlevel_setBassGain( uint8 idx );

static void AudioDeviceManager_lowlevel_AudParmsConfig( void *parms );

static void AudioDeviceManager_lowlevel_rstSRC( void );

static void AuidoDeviceManager_lowlevel_VolController_setEvent( xADMVolEvents parms );

static bool AuidoDeviceManager_VolController_RampStauts(void);

static uint8 AuidoDeviceManager_VolController_VolStauts(void);

static uint8 AuidoDeviceManager_VolController_TargetVol(void);

static void AudioDeviceManager_lowlevel_setInputPath( AUDIO_SOURCE idx );

static void AuidoDeviceManager_lowlevel_setLsRs( uint8 idx );

static uint8 AuidoDeviceManager_lowlevel_getAudioStreamType( void );

/* Mutex */
static bool ADM_I2C2_sema_mutex_take( void );

static bool ADM_I2C2_sema_mutex_give( void );

static bool ADM_Call_Back_sema_mutex_take( void );

static bool ADM_Call_Back_sema_mutex_give( void );

/* Exception */
static void AuidoDeviceManager_lowlevel_Exception_CreateTask( void );

void AuidoDeviceManager_lowlevel_Exception_Task( void *pvParameters );

static void AuidoDeviceManager_lowlevel_Exception_TaskCtrl( bool val);

static bool AuidoDeviceManager_lowlevel_Exception_InstrSender( const void *params );

/* Function Back */
void AuidoDeviceManager_lowlevel_Function_Back_CreateTask( void );

void AuidoDeviceManager_lowlevel_BackCtl_task( void *pvParameters );

void AuidoDeviceManager_lowlevel_Function_Back_TaskCtrl( bool val);

bool AuidoDeviceManager_lowlevel_Function_Back_InstrSender( const void *params );

//______________________________________________________________________________
extern DSP_CTRL_OBJECT *pDSP_ObjCtrl;
extern SRC_CTRL_OBJECT *pSRC_ObjCtrl;
extern ADC_CTRL_OBJECT *pADC_ObjCtrl;
extern AMP_DEV_MANAGER_OBJECT *pAMP_DEV_ObjCtrl;
extern UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl; 
extern HDMI_DEVICE_MANAGER_OBJECT *pHDMI_DM_ObjCtrl;
extern BT_HANDLE_OBJECT *pBTHandle_ObjCtrl;

const AUDIO_DEVICE_MANAGER_OBJECT AudioDeviceManager =
{
    AudioDeviceManager_CreateTask,
    AudioDeviceManager_Initialize,
    AudioDeviceManager_DeInitialize,
    AudioDeviceManager_DeviceConfig,
    AudioDeviceManager_getAudParams,
    AudioDeviceManager_setInputPath,
    AuidoDeviceManager_VolController_CreateTask,
    AuidoDeviceManager_VolController_setEvent,
    AuidoDeviceManager_VolController_RampStauts,
    AuidoDeviceManager_VolController_VolStauts,
    AuidoDeviceManager_VolController_TargetVol,
    AuidoDeviceManager_VolController_TaskCtrl,
    AuidoDeviceManager_setBass,
    AuidoDeviceManager_setTreble,
    AuidoDeviceManager_setSub,
    AuidoDeviceManager_setCenter,
    AuidoDeviceManager_setLsRs,
    AuidoDeviceManager_setBalance,
    AuidoDeviceManager_setSRSTruVolTSHD,
    AuidoDeviceManager_setNightMode,
    AuidoDeviceManager_setAVDelay,
    AuidoDeviceManager_Factory_AQBypass,
    AudioDeviceManager_getSignalAvailable,    
    AuidoDeviceManager_Route_Ctrl
};
const AUDIO_DEVICE_MANAGER_OBJECT *pADM_ObjCtrl = &AudioDeviceManager;

/*for cs8422 and cs495314 function call*/
const AUDIO_LOWLEVEL_DRIVER_OBJECT AudioLowLeveDriver =
{
    AudioDeviceManager_lowlevel_AudParmsConfig,
    AudioDeviceManager_lowlevel_rstSRC,
    AuidoDeviceManager_lowlevel_VolController_setEvent,
    AuidoDeviceManager_lowlevel_setLsRs,
    AuidoDeviceManager_lowlevel_getAudioStreamType,
    /* for exception control */
    AuidoDeviceManager_lowlevel_Exception_CreateTask,
    AuidoDeviceManager_lowlevel_Exception_TaskCtrl,
    AuidoDeviceManager_lowlevel_Exception_InstrSender,
    /* For function back*/
    AuidoDeviceManager_lowlevel_Function_Back_CreateTask,
    AuidoDeviceManager_lowlevel_Function_Back_TaskCtrl,
    AuidoDeviceManager_lowlevel_Function_Back_InstrSender
};
const AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl = &AudioLowLeveDriver;

//______________________________________________________________________________
static void AudioDeviceManager_CreateTask( void )
{
    //pDSP_ObjCtrl->loader_create_task();
    pDSP_ObjCtrl->fch_create_task();
    //pDSP_ObjCtrl->lowlevel_create_task();
    pADM_ObjCtrl->CreateVolTask();
    pAudLowLevel_ObjCtrl->CreateBackCtrlTask();
    pAudLowLevel_ObjCtrl->CreateExceptionTask();
    //AuidoDeviceManager_SignalDetector_CreateTask();
    //AuidoDeviceManager_DigitalDetector_CreateTask();

    xOS_ADM_Parms.ADM_I2C2_sema.xSemaphore = xSemaphoreCreateMutex(); 
    xOS_ADM_Parms.ADM_I2C2_sema.xBlockTime = portMAX_DELAY;

    xOS_ADM_Parms.ADM_Call_Back_sema.xSemaphore = xSemaphoreCreateMutex();
    xOS_ADM_Parms.ADM_Call_Back_sema.xBlockTime = portMAX_DELAY;

}

static xAudDeviceParms AudioDeviceManager_getAudParams( void )
{
    return *pAudioDevParms;
}

static void AudioDeviceManager_setInputPath( AUDIO_SOURCE idx )
{
    CS49844_QUEUE_TYPE QUEUE_TYPE;

    QUEUE_TYPE.audio_type = CS49844_LOAD_PCM;
  
    //TRACE_DEBUG((0,"AudioDeviceManager_setInputPath"));
    mAudDevParms.input_src = idx;

#if ( configSTM32F411_PORTING == 0 )

        switch( idx )
        {
            case AUDIO_SOURCE_AUX1:
            {
                GPIOMiddleLevel_Set(__O_CLK_SEL);
                pADC_ObjCtrl->input_path( ADC_PATH_AUX1 );
                pADC_ObjCtrl->input_gain( 0x3D, 0x3D ); // -1.5db
                pSRC_ObjCtrl->set_serial_audio_in();        
            }
                break;

            case AUDIO_SOURCE_AUX2:
            {
                GPIOMiddleLevel_Set(__O_CLK_SEL);
                pADC_ObjCtrl->input_path( ADC_PATH_AUX2 );
                pADC_ObjCtrl->input_gain( 0x3D, 0x3D ); // -1.5db
                pSRC_ObjCtrl->set_serial_audio_in();
            }
                break;

            case AUDIO_SOURCE_COAXIAL:
            {
                GPIOMiddleLevel_Set(__O_CLK_SEL);
                pSRC_ObjCtrl->set_rx_path( SRC_PATH_DITIGITAL1, 0 );
            }
                break;

            case AUDIO_SOURCE_OPTICAL:
            {
               GPIOMiddleLevel_Set(__O_CLK_SEL); 
               pSRC_ObjCtrl->set_rx_path( SRC_PATH_DITIGITAL2, 0 );
            }
                break;

            case AUDIO_SOURCE_HDMI_IN:
            case AUDIO_SOURCE_HDMI_ARC:
            {
                GPIOMiddleLevel_Set(__O_CLK_SEL);
                pSRC_ObjCtrl->set_rx_path( SRC_PATH_DITIGITAL3, 0 );
            }
                break;    

            case AUDIO_SOURCE_USB_PLAYBACK:
            {
                GPIOMiddleLevel_Clr(__O_CLK_SEL);
                pADC_ObjCtrl->input_path( ADC_PATH_USB );
                pADC_ObjCtrl->input_gain( 0x3a, 0x3a ); // -3db
                pSRC_ObjCtrl->set_serial_audio_in();
            }
            break;
            
            case AUDIO_SOURCE_BLUETOOTH:
            {
                GPIOMiddleLevel_Set(__O_CLK_SEL);
                pADC_ObjCtrl->input_path( ADC_PATH_BT );
                pADC_ObjCtrl->input_gain( 0x10, 0x10 ); // 8db
                pSRC_ObjCtrl->set_serial_audio_in();
            }
            break;
        }
#endif

    
#if ( configSTM32F411_PORTING == 1 )   
    AuidoDeviceManager_lowlevel_VolController_setEvent(AUD_VOL_EVENT_SET_MUTE);
  
    if (ADM_I2C2_sema_mutex_take() == TRUE )
    {    
        pDSP_ObjCtrl->fch_task_ctrl( xOS_setTASK_SUSPENDED );
        pDSP_ObjCtrl->lowlevel_task_ctrl( xOS_setTASK_SUSPENDED );
        AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_SUSPENDED );

        pSRC_ObjCtrl->isr_ctrl( FUNC_DISABLE );
        switch( idx )
        {
            case AUDIO_SOURCE_AUX1:
            {
                pADC_ObjCtrl->input_path( ADC_PATH_AUX1 );
                pADC_ObjCtrl->input_gain( 0x3D, 0x3D ); // -1.5db
                pAudLowLevel_ObjCtrl->RstSRC();  
                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_RUNING );
                /*initiate cs495xx dsp*/
                QUEUE_TYPE.audio_type = CS49844_LOAD_PCM;
                QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;
                
                if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
                {
                    pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
                }                
            }
                break;

            case AUDIO_SOURCE_AUX2:
            {
                pADC_ObjCtrl->input_path( ADC_PATH_AUX2 );
                pADC_ObjCtrl->input_gain( 0x3D, 0x3D ); // -1.5db
                
                pAudLowLevel_ObjCtrl->RstSRC();

                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_RUNING );
                QUEUE_TYPE.audio_type = CS49844_LOAD_PCM;
                QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;

                if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
                {
                    pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
                }
            }
                break;

            case AUDIO_SOURCE_COAXIAL:
            {
                AudioDeviceManager_DigitalDetector_TaskCtrl( BTASK_SUSPENDED );
                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_SUSPENDED);

                QUEUE_TYPE.audio_type = CS49844_LOAD_USER_CTRL;
                QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;

                if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
                {
                    pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
                }
                
            }
                break;

            case AUDIO_SOURCE_OPTICAL:
            {
                AudioDeviceManager_DigitalDetector_TaskCtrl( BTASK_SUSPENDED );
                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_SUSPENDED);
                pAudLowLevel_ObjCtrl->RstSRC();

                QUEUE_TYPE.audio_type = CS49844_LOAD_USER_CTRL;
                QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;

                if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
                {
                    pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
                }
            }
                break;

            case AUDIO_SOURCE_HDMI_IN:
            case AUDIO_SOURCE_HDMI_ARC:
            {
                 AudioDeviceManager_DigitalDetector_TaskCtrl( BTASK_SUSPENDED );
                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_SUSPENDED);
                pAudLowLevel_ObjCtrl->RstSRC();

                QUEUE_TYPE.audio_type = CS49844_LOAD_USER_CTRL;
                QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;

                if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
                {
                    pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
                }
            }
                break;    

            case AUDIO_SOURCE_USB_PLAYBACK:
            {
                pADC_ObjCtrl->input_path( ADC_PATH_USB );
                pADC_ObjCtrl->input_gain( 0x3a, 0x3a ); // -3db
                pAudLowLevel_ObjCtrl->RstSRC();

                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_RUNING );    
                /*initiate cs495xx dsp*/
                QUEUE_TYPE.audio_type = CS49844_LOAD_PCM;
                QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;
                
                if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
                {
                    pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
                }
            }
            break;
            
            case AUDIO_SOURCE_BLUETOOTH:
            {
                pADC_ObjCtrl->input_path( ADC_PATH_BT );
                pADC_ObjCtrl->input_gain( 0x10, 0x10 ); // 8db
                pAudLowLevel_ObjCtrl->RstSRC();

                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_RUNING );
                /*initiate cs495xx dsp*/
                QUEUE_TYPE.audio_type = CS49844_LOAD_PCM;
                QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;

                if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
                {
                    pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
                }
            }
            break;
        }
        ADM_I2C2_sema_mutex_give();
    }
#endif  
    TRACE_DEBUG((0, "input path = %d", idx ));
}

void AuidoDeviceManager_setBass( uint8 idx )
{
    pDSP_ObjCtrl->setBassGain(Bass_table[idx]);
    mAudDevParms.bass_gain = idx;
    TRACE_DEBUG((0, "BASS Gain = %d",  mAudDevParms.bass_gain));
}

void AuidoDeviceManager_setTreble( uint8 idx )
{
    pDSP_ObjCtrl->setTrebleGain(Treb_table[idx]);
    mAudDevParms.treble_gain = idx;
    TRACE_DEBUG((0, "TREBLE Gain = %d",  mAudDevParms.treble_gain));
}

void AuidoDeviceManager_setSub( uint8 idx )
{
    uint8 ASTIndex;

    ASTIndex = pDSP_ObjCtrl->GetAudioStreamType();
    
    switch ( ASTIndex )
    {
    case CS49844_LOAD_PCM:
    {
        if ( pAudioDevParms->srs_tshd == 1 )    //Surround on
        {
            pDSP_ObjCtrl->setSubGain(SUB_PCM51_GAIN[idx]);
        }
        else    //Surround off
        {
            pDSP_ObjCtrl->setSubGain(SUB_PCM21_GAIN[idx]);
        }
    }
        break;

    case CS49844_LOAD_AC3:
    {
        if ( pAudioDevParms->srs_tshd == 1 )    //Surround on
        {
            pDSP_ObjCtrl->setSubGain(SUB_AC351_GAIN[idx]);
        }
        else    //Surround off
        {
            pDSP_ObjCtrl->setSubGain(SUB_AC321_GAIN[idx]);
        }
    }
        break;

    case CS49844_LOAD_DTS:
    {
        if ( pAudioDevParms->srs_tshd == 1 )    //Surround on
        {
            pDSP_ObjCtrl->setSubGain(SUB_DTS51_GAIN[idx]);
        }
        else    //Surround off
        {
            pDSP_ObjCtrl->setSubGain(SUB_DTS21_GAIN[idx]);
        }
    }
        break;

    default:
    {
        pDSP_ObjCtrl->setSubGain(SUB_PCM51_GAIN[idx]);
    }
        break;

    }
    mAudDevParms.sub_gain= idx;
    TRACE_DEBUG((0, "Sub Gain = %d",  mAudDevParms.sub_gain));
}

void AuidoDeviceManager_setCenter( uint8 idx )
{
    pDSP_ObjCtrl->setCenterGain(CENTER_GAIN[idx]);
    mAudDevParms.center_gain= idx;
    TRACE_DEBUG((0, "Center Gain = %d",  mAudDevParms.center_gain));
}

void AuidoDeviceManager_setLsRs( uint8 idx )
{
    const uint32 *pLsRs;
    uint8 ASTIndex;

    ASTIndex = pDSP_ObjCtrl->GetAudioStreamType();

    switch ( ASTIndex )
    {
        case CS49844_LOAD_PCM:
        {
            pLsRs = LS_RS_PCM51_GAIN;
        }
            break;
        
        case CS49844_LOAD_AC3:
        {
            pLsRs = LS_RS_AC3_GAIN;
        }
            break;
        
        case CS49844_LOAD_DTS:
        {
            pLsRs = LS_RS_DTS_GAIN;
        }
            break;
        
        default:
        {
            pLsRs = LS_RS_PCM51_GAIN;
        }
            break;
        
    }
    pDSP_ObjCtrl->setLsRsGain( *(pLsRs + idx) );
    mAudDevParms.LsRs_gain = idx;
    TRACE_DEBUG((0, "LsRs Gain = %d",  mAudDevParms.LsRs_gain));
}

void AuidoDeviceManager_setBalance( uint8 idx )
{
    const uint32 *pBalance;
    uint8 ASTIndex;

    ASTIndex = pDSP_ObjCtrl->GetAudioStreamType();

    switch ( ASTIndex )
    {
    case CS49844_LOAD_PCM:
    {
        pBalance = BALANCE_PCM_GAIN;
    }
        break;

    case CS49844_LOAD_AC3:
    {
        pBalance = BALANCE_AC3_GAIN;
    }
        break;

    case CS49844_LOAD_DTS:
    {
        pBalance = BALANCE_DTS_GAIN;
    }
        break;

    default:
    {
        pBalance = BALANCE_PCM_GAIN;
    }
        break;

    }
    if ( idx> 6 )
    {
        pDSP_ObjCtrl->setBalanceLs( *(pBalance + idx) );
    }
    else if ( idx < 6 )
    {
        pDSP_ObjCtrl->setBalanceRs( *(pBalance + idx) );
    }
    else
    {
        pDSP_ObjCtrl->setBalanceLs( *(pBalance + 6) );
       pDSP_ObjCtrl->setBalanceRs( *(pBalance + 6) );
    }   
    mAudDevParms.balance= idx;
    TRACE_DEBUG((0, "BALANCE = %d",  mAudDevParms.balance));
}

void AuidoDeviceManager_setSRSTruVolTSHD( void *parms )
{
    xAudDeviceParms* pParms = (xAudDeviceParms*)parms;
    CS49844_QUEUE_TYPE QUEUE_TYPE;

    mAudDevParms.srs_truvol= pParms->srs_truvol;
    mAudDevParms.srs_tshd= pParms->srs_tshd;

    if( pDSP_ObjCtrl->GetLoadrState() != 0 )    /*LOADER_IDLE*/
    {
        return;
    }

    AudioDeviceManager_lowlevel_setMute( SOUND_MUTE ); 

    pDSP_ObjCtrl->fch_task_ctrl( xOS_setTASK_SUSPENDED );
    pDSP_ObjCtrl->lowlevel_task_ctrl( xOS_setTASK_SUSPENDED );

    /*initiate cs495xx dsp*/
    QUEUE_TYPE.audio_type = CS49844_LOAD_USER_CTRL;
    QUEUE_TYPE.source_ctrl = CS49844_SOURCE_PROCESS_MODE;
    QUEUE_TYPE.EQ = pAudioDevParms->EQ;
    QUEUE_TYPE.srs_tshd = pAudioDevParms->srs_tshd;
    QUEUE_TYPE.srs_truvol = pAudioDevParms->srs_truvol;

    if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
    {
        pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
    }
    return;
}

void AuidoDeviceManager_setNightMode( uint8 idx )
{
    if ( idx == TRUE)
    {
        pDSP_ObjCtrl->setNightMode(SubWoofer_GAIN_14dB);
    }
    else
    {
        pDSP_ObjCtrl->setNightMode(SubWoofer_GAIN_24dB);
    }
    
    mAudDevParms.night_mode= idx;
    TRACE_DEBUG((0, "Night Mode = %d",  mAudDevParms.night_mode));
}

void AuidoDeviceManager_setAVDelay( uint8 idx )
{
        pDSP_ObjCtrl->setAVDelay(idx);
        mAudDevParms.av_delay= idx;
        TRACE_DEBUG((0, "AV Delay = %d",  mAudDevParms.av_delay));
}

void AuidoDeviceManager_Factory_AQBypass( bool EQ )
{
    if (pAudioDevParms->op_mode == MODE_FACTORY)
    {
        if(EQ)
        {
            pDSP_ObjCtrl->SetTVHDMC_Ctrl((byte*)&PCM_51_DIS_BPS_PARMS, (sizeof(PCM_51_DIS_BPS_PARMS)/sizeof(uint8)) );
            TRACE_DEBUG((0,"PCM_51_DIS_BPS_PARMS"));
        }
        else
        {
            pDSP_ObjCtrl->SetTVHDMC_Ctrl((byte*)&PCM_51_BPS_PARMS, (sizeof(PCM_51_BPS_PARMS)/sizeof(uint8)) );
            TRACE_DEBUG((0,"PCM_51_BPS_PARMS"));
        }
    }
}

void AudioDeviceManager_lowlevel_AudioRouter( AudioRouteCtrl idx )
{
    switch( idx )
    {
        case ROUTE_CTRL_INITIAL:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_INITIAL );
            break;

        case ROUTE_CTRL_FL_FR:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_FL_FR );
            break;

        case ROUTE_CTRL_RR_RL:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_RR_RL );
            break;
        case ROUTE_CTRL_CENTER:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_CENTER );
            break;
        case ROUTE_CTRL_LFE0:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_LFE0 );
            break;
        case ROUTE_CTRL_FL:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_FL );
            break;
        case ROUTE_CTRL_FR:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_FR );
            break;
        case ROUTE_CTRL_RR:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_RR );
            break;
        case ROUTE_CTRL_RL:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_RL );
            break;
        case ROUTE_CTRL_FL_FR_CENTER:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_FL_FR_CENTER );
            break;
        case ROUTE_CTRL_RR_RL_WOOFER:
            pDSP_ObjCtrl->setAudioRoute( ROUTE_RR_RL_WOOFER );
            break;
        case ROUTE_CTRL_DISABLE:
        default:    
            pDSP_ObjCtrl->setAudioRoute( ROUTE_DISABLE );
            break;
    }

}

void AuidoDeviceManager_Route_Ctrl(AudioRouteCtrl router_idx)
{
    AudioDeviceManager_lowlevel_AudioRouter(router_idx);
}

void AudioDeviceManager_DeInitialize( void )
{
    //pDSP_ObjCtrl->loader_task_ctrl( xOS_setTASK_SUSPENDED );
    pDSP_ObjCtrl->fch_task_ctrl( xOS_setTASK_SUSPENDED );
    //pDSP_ObjCtrl->lowlevel_task_ctrl( xOS_setTASK_SUSPENDED );

    pSRC_ObjCtrl->isr_ctrl( FUNC_DISABLE );
    pADM_ObjCtrl->VolTaskCtrl( BTASK_SUSPENDED );
    pAudLowLevel_ObjCtrl->ExceptionTaskCtrl( BTASK_SUSPENDED );
    pAudLowLevel_ObjCtrl->BackCtrlTaskCtrl(BTASK_SUSPENDED);
    //AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_SUSPENDED );
    //AudioDeviceManager_DigitalDetector_TaskCtrl( BTASK_SUSPENDED );
    ADM_I2C2_sema_mutex_give();
}

static void AudioDeviceManager_InitializeParams( const xAudDeviceParms *pParms )
{
    /*initial audio parameters*/
    mAudDevParms.op_mode = pParms->op_mode;
    mAudDevParms.input_src = pParms->input_src;
    mAudDevParms.mute = pParms->mute;
    mAudDevParms.master_gain = pParms->master_gain;
    mAudDevParms.bass_gain = pParms->bass_gain;
    mAudDevParms.treble_gain = pParms->treble_gain;
    mAudDevParms.sub_gain = pParms->sub_gain;
    mAudDevParms.center_gain = pParms->center_gain;
    mAudDevParms.LsRs_gain = pParms->LsRs_gain;
    mAudDevParms.balance = pParms->balance;
    mAudDevParms.srs_truvol = pParms->srs_truvol;
    mAudDevParms.srs_tshd = pParms->srs_tshd;
    mAudDevParms.night_mode = pParms->night_mode;
    mAudDevParms.av_delay = pParms->av_delay;
    mAudDevParms.EQ = FALSE;
}

static void AudioDeviceManager_Initialize( void *parms )
{
    CS49844_QUEUE_TYPE QUEUE_TYPE;


    xAudDeviceParms* pParms = (xAudDeviceParms*)parms;

    AudioDeviceManager_InitializeParams( pParms );
#if ( configSTM32F411_PORTING == 1 )    
    pDSP_ObjCtrl->loader_task_ctrl( xOS_setTASK_RESUME ); 
#endif
    if (ADM_I2C2_sema_mutex_take() == TRUE )
    {    
        pSRC_ObjCtrl->initialize();
        pAMP_DEV_ObjCtrl->initialize( );
        pADC_ObjCtrl->initialize();
        ADM_I2C2_sema_mutex_give();
    }
    pADM_ObjCtrl->SetInputPath(pAudioDevParms->input_src);
#if ( configSTM32F411_PORTING == 1 )

    QUEUE_TYPE.audio_type = CS4953x_LOAD_PCM;
    QUEUE_TYPE.source_ctrl = CS4953x_SOURCE_DSP_INITIAL; /*Prevent two ramp when power on*/
    QUEUE_TYPE.op = pAudioDevParms->op_mode;
    QUEUE_TYPE.EQ = pAudioDevParms->EQ;
    QUEUE_TYPE.srs_tshd = pAudioDevParms->srs_tshd;
    QUEUE_TYPE.srs_truvol = pAudioDevParms->srs_truvol;

    if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
    {
        pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
    }

    pADM_ObjCtrl->SetInputPath(pAudioDevParms->input_src);
    
    pDSP_ObjCtrl->fch_task_ctrl( xOS_setTASK_SUSPENDED );
    pDSP_ObjCtrl->lowlevel_task_ctrl( xOS_setTASK_SUSPENDED );/*Tony140704: will be resume in user setting*/
    if (ADM_I2C2_sema_mutex_take() == TRUE )
    {   
        pADM_ObjCtrl->VolTaskCtrl(BTASK_RESUME);
        pAudLowLevel_ObjCtrl->ExceptionTaskCtrl(BTASK_RESUME);
        pAudLowLevel_ObjCtrl->BackCtrlTaskCtrl(BTASK_RESUME);

        AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_SUSPENDED); /*Tony140623: will be resume in user setting*/
        AudioDeviceManager_DigitalDetector_TaskCtrl( BTASK_SUSPENDED);
        ADM_I2C2_sema_mutex_give();
    }
#endif   
}


static void AudioDeviceManager_DeviceConfig( void *parms )
{
    CS49844_QUEUE_TYPE QUEUE_TYPE;
    xAudDeviceParms* pParms = (xAudDeviceParms*)parms;

    QUEUE_TYPE.audio_type = CS49844_LOAD_PCM;
    QUEUE_TYPE.source_ctrl = CS49844_SOURCE_USER;
    QUEUE_TYPE.EQ = pAudioDevParms->EQ;
    QUEUE_TYPE.srs_tshd = pAudioDevParms->srs_tshd;
    QUEUE_TYPE.srs_truvol = pAudioDevParms->srs_truvol;
    QUEUE_TYPE.sample_96k = FALSE;
    QUEUE_TYPE.multi_channel = FALSE;

    if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
    {
        pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );
    }
    
    AudioDeviceManager_InitializeParams( pParms );
    AudioDeviceManager_lowlevel_AudParmsConfig( NULL );
}

static void AudioDeviceManager_lowlevel_setMute( bool idx )
{
    if ( idx == SOUND_MUTE )
    {
        pAMP_DEV_ObjCtrl->set_rear_mute( idx );
        pAMP_DEV_ObjCtrl->set_front_mute( idx );
        pDSP_ObjCtrl->setMute( idx );
    }
    else if( ( pAudioDevParms->mute == SOUND_DEMUTE ) && ( idx == SOUND_DEMUTE ) )
    {
        pAMP_DEV_ObjCtrl->set_front_mute( idx );
        pDSP_ObjCtrl->setMute( idx );
        pAMP_DEV_ObjCtrl->set_rear_mute( idx );
    }
}

static void AuidoDeviceManager_lowlevel_lowlevel_setFACTORY(void *parms)
{
    if (pAudioDevParms->op_mode == MODE_FACTORY)
    {
        pDSP_ObjCtrl->SetTVHDMC_Ctrl((byte*)&PCM_51_BPS_PARMS, (sizeof(PCM_51_BPS_PARMS)/sizeof(uint8)) );
            }
        }

static void AuidoDeviceManager_lowlevel_setMasterGain( uint8 idx )
{
    pDSP_ObjCtrl->setMasterGain( MASTER_GAIN_TABLE[idx] );
}

static void AuidoDeviceManager_lowlevel_setBassGain( uint8 idx )
{

    pDSP_ObjCtrl->setBassGain(Bass_table[idx]);

}

static void AuidoDeviceManager_lowlevel_setSub( uint8 idx )
{
    switch ( pDSP_ObjCtrl->GetAudioStreamType() )
    {
        case CS49844_LOAD_PCM:
        {
            if ( pAudioDevParms->srs_tshd == 1 )    //Surround on
            {
                pDSP_ObjCtrl->setSubGain(SUB_PCM51_GAIN[idx]);
            }
            else    
            {
                pDSP_ObjCtrl->setSubGain(SUB_PCM21_GAIN[idx]);
            }
        }
            break;

        case CS49844_LOAD_AC3:
        {
            if ( pAudioDevParms->srs_tshd == 1 )    //Surround on
            {
                pDSP_ObjCtrl->setSubGain(SUB_AC351_GAIN[idx]);
            }
            else    
            {
                pDSP_ObjCtrl->setSubGain(SUB_AC321_GAIN[idx]);
            }
        }
            break;

        case CS49844_LOAD_DTS:
        {
            if ( pAudioDevParms->srs_tshd == 1 )    //Surround on
            {
                pDSP_ObjCtrl->setSubGain(SUB_DTS51_GAIN[idx]);
            }
            else    //Surround off
            {
                pDSP_ObjCtrl->setSubGain(SUB_DTS21_GAIN[idx]);
            }
        }
            break;

        default:
        {
            pDSP_ObjCtrl->setSubGain(SUB_PCM51_GAIN[idx]);
        }
            break;

    }
}

static void AuidoDeviceManager_lowlevel_setLsRs( uint8 idx )
{
    const uint32 *pLsRs;
    switch ( pDSP_ObjCtrl->GetAudioStreamType() )
    {
        case CS49844_LOAD_PCM:
        {
            pLsRs = LS_RS_PCM51_GAIN;
        }
            break;
        
        case CS49844_LOAD_AC3:
        {
            pLsRs = LS_RS_AC3_GAIN;
        }
            break;
        
        case CS49844_LOAD_DTS:
        {
            pLsRs = LS_RS_DTS_GAIN;
        }
            break;
        
        default:
        {
            pLsRs = LS_RS_PCM51_GAIN;
        }
            break;
        
    }
    pDSP_ObjCtrl->setLsRsGain( *(pLsRs + idx) );
}

static void AuidoDeviceManager_lowlevel_setBalance( uint8 idx )
{
    const uint32 *pBalance;
    
    switch ( pDSP_ObjCtrl->GetAudioStreamType() )
    {
        case CS49844_LOAD_PCM:
        {
            pBalance = BALANCE_PCM_GAIN;
        }
            break;

        case CS49844_LOAD_AC3:
        {
            pBalance = BALANCE_AC3_GAIN;
        }
            break;

        case CS49844_LOAD_DTS:
        {
            pBalance = BALANCE_DTS_GAIN;
        }
            break;

        default:
        {
            pBalance = BALANCE_PCM_GAIN;
        }
            break;

    }
    
    if ( idx> 6 )
    {
        pDSP_ObjCtrl->setBalanceLs( *(pBalance + idx) );
    }
    else if ( idx < 6 )
    {
        pDSP_ObjCtrl->setBalanceRs( *(pBalance + idx) );
    }
    else
    {
        pDSP_ObjCtrl->setBalanceLs( *(pBalance + 6) );
        pDSP_ObjCtrl->setBalanceRs( *(pBalance + 6) );
    }   
}

static void AuidoDeviceManager_lowlevel_setNightMode( uint8 idx )
{
    if ( idx == TRUE)
    {
        pDSP_ObjCtrl->setNightMode(SubWoofer_GAIN_14dB);
    }
    else
    {
        pDSP_ObjCtrl->setNightMode(SubWoofer_GAIN_24dB);
    }
}

static void AuidoDeviceManager_lowlevel_setCenter( uint8 idx )
{
    pDSP_ObjCtrl->setCenterGain(CENTER_GAIN[idx]);
}

static void AuidoDeviceManager_lowlevel_setTreble( uint8 idx )
{
    pDSP_ObjCtrl->setTrebleGain(Treb_table[idx]);
}

static void AuidoDeviceManager_lowlevel_setAVDelay( uint8 idx )
{
    pDSP_ObjCtrl->setAVDelay(idx);
}
uint8 AuidoDeviceManager_lowlevel_getAudioStreamType( void )
{
    return (uint8)pDSP_ObjCtrl->GetAudioStreamType();
}

void AudioDeviceManager_lowlevel_AudParmsConfig( void *parms )
{
    AuidoDeviceManager_lowlevel_lowlevel_setFACTORY(NULL);
    AuidoDeviceManager_lowlevel_setBassGain(pAudioDevParms->bass_gain);
    AuidoDeviceManager_lowlevel_setTreble(pAudioDevParms->treble_gain);
    AuidoDeviceManager_lowlevel_setSub( pAudioDevParms->sub_gain );
    AuidoDeviceManager_lowlevel_setCenter(pAudioDevParms->center_gain);
    AuidoDeviceManager_lowlevel_setLsRs( pAudioDevParms->LsRs_gain );
    AuidoDeviceManager_lowlevel_setBalance( pAudioDevParms->balance );
    AuidoDeviceManager_lowlevel_setNightMode( pAudioDevParms->night_mode );
    AuidoDeviceManager_lowlevel_setAVDelay( pAudioDevParms->av_delay );
    AuidoDeviceManager_lowlevel_setMasterGain(pAudioDevParms->master_gain);
    TRACE_DEBUG((0, "Configure lowlevel audio parameter"));
}

void AudioDeviceManager_lowlevel_rstSRC( void )
{
    GPIOMiddleLevel_Clr(__O_CS8422_RST);
    vTaskDelay(TASK_MSEC2TICKS(2));
    GPIOMiddleLevel_Set(__O_CS8422_RST);
    vTaskDelay(TASK_MSEC2TICKS(2));

    pSRC_ObjCtrl->initialize();
    
    switch( pAudioDevParms->input_src )
    {
        case AUDIO_SOURCE_AUX1:
        {
            pSRC_ObjCtrl->set_serial_audio_in();
        }
            break;

        case AUDIO_SOURCE_AUX2:
        {
            pSRC_ObjCtrl->set_serial_audio_in();
        }
            break;

        case AUDIO_SOURCE_COAXIAL:
        {
            pSRC_ObjCtrl->set_rx_path( SRC_PATH_DITIGITAL1, 0 );
        }
            break;

        case AUDIO_SOURCE_OPTICAL:
        {
            pSRC_ObjCtrl->set_rx_path( SRC_PATH_DITIGITAL2, 0 );
        }
            break;

#if ( configSII_DEV_953x_PORTING == 1 )
        case AUDIO_SOURCE_HDMI_IN:
        case AUDIO_SOURCE_HDMI_ARC:	
        {
            pSRC_ObjCtrl->set_rx_path( SRC_PATH_DITIGITAL3, 0 );
        }
            break;
#endif

        case AUDIO_SOURCE_USB_PLAYBACK:
        {
            pSRC_ObjCtrl->set_serial_audio_in();
        }
            break;
        
        case AUDIO_SOURCE_BLUETOOTH:
        {
            pSRC_ObjCtrl->set_serial_audio_in();
        }
            break;
    }
}

void AuidoDeviceManager_VolController_task( void *pvParameters )
{
    for(;;)
    {
        switch( mVolumeRampEvent )
        {
            case VRE_RAMP_IDLE:
            {
                if ( AudioDeviceManager_getSignalAvailable() == 0 )
                {
                    vre_ramp_count++;
                    
                    if ( vre_ramp_count >= 600 )    //60 sec
                    {
                        if (ADM_I2C2_sema_mutex_take() == TRUE )
                        {
                            AudioDeviceManager_lowlevel_setMute( SOUND_MUTE ); 
                            ADM_I2C2_sema_mutex_give();
                        } 
                        mVolumeRampEvent = VRE_RAMP_WAIT_SIGNAL;
                        vre_ramp_count = 0;
                    }
                }
                 else
                {
                    vre_ramp_count = 0;
                }
            }
                break;

            case VRE_RAMP_WAIT_SIGNAL:
            {
                vre_ramp_count++;

                if ( AudioDeviceManager_getSignalAvailable() == 1 )
                {
                    if (ADM_I2C2_sema_mutex_take() == TRUE )
                    {
                        AudioDeviceManager_lowlevel_setMute( SOUND_DEMUTE );  
                        ADM_I2C2_sema_mutex_give();
                    }
                    vre_ramp_count = 0;
                    mVolumeRampEvent = VRE_RAMP_IDLE;
                }
            }
                break;

            case VRE_RAMP_INITIAL:
            {
                TRACE_DEBUG((0,"VRE_RAMP_INITIAL"));
                vre_ramp_count = 0;
                target_volume = pAudioDevParms->master_gain;

                volume_inc = 0;

                if ( target_volume == 0 )
                {
                    if (ADM_I2C2_sema_mutex_take() == TRUE )
                    {
                        AudioDeviceManager_lowlevel_setMute( SOUND_DEMUTE ); 
                        ADM_I2C2_sema_mutex_give();
                    }
                    mVolumeRampEvent = VRE_RAMP_END;
                }
                else
                {
                    mVolumeRampEvent = VRE_RAMP_START;
                }
            }
            break;

            case VRE_RAMP_MUTE_LOCK:
            {
                target_volume = pAudioDevParms->master_gain; 
                volume_inc = 1;
                vre_ramp_count ++;
                if ( vre_ramp_count > ramp_delay )
                {
                    vre_ramp_count = 0;
                    mVolumeRampEvent = VRE_RAMP_START;
                }
            }
            break;

            case VRE_RAMP_WAIT_BT_CONNECT:
            {
                target_volume = pAudioDevParms->master_gain; 
                volume_inc = 1;
                mVolumeRampEvent = VRE_RAMP_START;
            }
            break;

            case VRE_RAMP_START:
            {
                if ( target_volume > MAX_VOL) // if target_volume out of range (0~35)
                {
                    target_volume=  pAudioDevParms->master_gain;
                }
                
                if ( volume_inc <= target_volume )
                {
                    if( volume_inc == 1 )
                    {
                        if (ADM_I2C2_sema_mutex_take() == TRUE )
                        {
                            AudioDeviceManager_lowlevel_setMute( SOUND_DEMUTE );
                            ADM_I2C2_sema_mutex_give();
                        }   
#if 1
                        /*UI 4.1 no volume ramp under Default gain*/
                        if (target_volume<=DEFAULT_MASTER_GAIN)
                        {
                            AuidoDeviceManager_lowlevel_setMasterGain( target_volume );
                            TRACE_DEBUG((0,"VOL = %d",target_volume));

                            pUDM_ObjCtrl->ExceptionSendEvent(UI_Event_VolRAMP_TARGET);

                            volume_inc = 0;
                            mVolumeRampEvent = VRE_RAMP_END;
                            break;
                        }
                        else
                        {
                            volume_inc = DEFAULT_MASTER_GAIN;
                        }
#endif
                    }
                    
                    AuidoDeviceManager_lowlevel_setMasterGain( volume_inc );
                    if (volume_inc == target_volume)
                    {
                        pUDM_ObjCtrl->ExceptionSendEvent(UI_Event_VolRAMP_TARGET);
                    }
                    else
                    {
                        pUDM_ObjCtrl->ExceptionSendEvent(UI_Event_VolRAMP);
                    }
                    TRACE_DEBUG((0,"VOL = %d",volume_inc));
                    volume_inc ++;
                }
                else
                {
                    if ( target_volume == 0 )
                    {
                        if (ADM_I2C2_sema_mutex_take() == TRUE )
                        {   
                            AudioDeviceManager_lowlevel_setMute( SOUND_DEMUTE );
                            ADM_I2C2_sema_mutex_give();
                        }
                    }
                    
                    mVolumeRampEvent = VRE_RAMP_END;
                }
            }
                break;

            case VRE_NON_RAMP_MUTE_LOCK:
            {
                target_volume = pAudioDevParms->master_gain; 
                volume_inc = 1;
                vre_ramp_count ++;
                if ( vre_ramp_count > ramp_delay )
                {
                    vre_ramp_count = 0;
                    mVolumeRampEvent = VRE_SET_VOL;
                }
            }
            break;
            
            case VRE_SET_VOL:
            {
                if ( (target_volume > MAX_VOL) || (target_volume < 0) )
                {
                    target_volume=  pAudioDevParms->master_gain;
                }
                
               if ( pAudioDevParms->master_gain >= target_volume )
                {
                    target_volume = pAudioDevParms->master_gain;
                    TRACE_DEBUG((0,"Change target volume to %d",target_volume));
                    mVolumeRampEvent = VRE_RAMP_START;
                }
                else
                {
                    if (ADM_I2C2_sema_mutex_take() == TRUE )
                    {               
                        AudioDeviceManager_lowlevel_setMute( SOUND_DEMUTE );
                        ADM_I2C2_sema_mutex_give();
                    }
                    mAudDevParms.master_gain = volume_inc;
                    mVolumeRampEvent = VRE_RAMP_IDLE;
                }
            }
                break;
                
            case VRE_RAMP_END:
            {
                if (mVolumeRampEvent == VRE_RAMP_END)
                {
                    //WE NEED TO ACTIVE THE LED SCANNING WHEN INPUT SOURCE IS BT
                    if( ( (pBTHandle_ObjCtrl->get_status() == BT_PAIRING) || (pBTHandle_ObjCtrl->get_status() == BT_POWER_ON)) && ( mAudDevParms.input_src == AUDIO_SOURCE_BLUETOOTH ) )
                    {
                        pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
                        TRACE_DEBUG((0, " 2222 show pairing led light ...  "));
                    }
                }
                
                target_volume = 0;
                volume_inc = 0;
                mVolumeRampEvent = VRE_RAMP_IDLE;
            }
                break;
        }
        vTaskDelay(TASK_MSEC2TICKS(100));    //Change to 100ms for VIZIO spec : ii.    Volume ramps at rate of 0.1 sec / step
    }
}

void AuidoDeviceManager_VolController_CreateTask( void )
{
    if ( xTaskCreate( AuidoDeviceManager_VolController_task, 
            ( portCHAR * ) "AuidoDeviceManager_VR", 
            (STACK_SIZE), NULL, tskAUD_VOLUMERAMP_PRIORITY,&xOS_ADM_Parms.VolTaskHandle) != pdPASS )
    {
        TRACE_ERROR((0, "AuidoDeviceManager_VR task create failure " ));
    }
    else
    {
        vTaskSuspend( xOS_ADM_Parms.VolTaskHandle);
        TRACE_ERROR((0, "AudioSystemHandler_VR task create OK?! " ));
    }
}

static void AuidoDeviceManager_lowlevel_VolController_setEvent( xADMVolEvents parms )
{
    if (ADM_I2C2_sema_mutex_take() == TRUE )
    {
        switch(parms)
        {
            /* When user mutes the system, vol ramp will be terminated */
            case AUD_VOL_EVENT_USER_MUTE: 
            {
                AudioDeviceManager_lowlevel_setMute( pAudioDevParms->mute );
                mVolumeRampEvent = VRE_RAMP_END;
            }
            break;
            
            /* When system mutes itself during source change, format change or error */
            case AUD_VOL_EVENT_SET_MUTE:
            {
                AudioDeviceManager_lowlevel_setMute( SOUND_MUTE);

                if ( (mVolumeRampEvent != VRE_RAMP_END) && (mVolumeRampEvent != VRE_RAMP_IDLE) )
                {
                    mVolumeRampEvent = VRE_RAMP_HOLD;
                }
            }
            break;
            
            /* When system demutes itself in audio back DSP and process mode*/
            case AUD_VOL_EVENT_SET_DEMUTE:
            {
                if ( (mVolumeRampEvent == VRE_RAMP_IDLE) || (mVolumeRampEvent == VRE_RAMP_END) )
                {
                    AuidoDeviceManager_lowlevel_setMasterGain( pAudioDevParms->master_gain );
                    AudioDeviceManager_lowlevel_setMute( SOUND_DEMUTE);
                }
                else if ( mVolumeRampEvent == VRE_RAMP_HOLD )
                {
                    mVolumeRampEvent = VRE_RAMP_INITIAL;
                }
            }
            break;
            
            case AUD_VOL_EVENT_SET_MASTER_GAIN:
            {
                if ( (mVolumeRampEvent == VRE_RAMP_IDLE) || (mVolumeRampEvent == VRE_RAMP_END) )
                {
                    AuidoDeviceManager_lowlevel_setMasterGain( pAudioDevParms->master_gain );
                    TRACE_DEBUG((0, "Master Gain = %d",  pAudioDevParms->master_gain ));
                }
                else
                {
                    mVolumeRampEvent = VRE_SET_VOL;
                }
            }
                break;
                
            case AUD_VOL_EVENT_VOLUME_DELAY_RELEASE:
            {
                vre_ramp_count = 0;
                AudioDeviceManager_lowlevel_setMute( SOUND_MUTE ); 

                if (  pAudioDevParms->input_src == AUDIO_SOURCE_BLUETOOTH )
                {
                    ramp_delay = 0;
                    mVolumeRampEvent = VRE_RAMP_WAIT_BT_CONNECT;
                }
                else
                {
                    if ( pAudioDevParms->input_src == AUDIO_SOURCE_USB_PLAYBACK )
                    {
                        ramp_delay = 1;
                    }
                    else
                    {
                        ramp_delay = 10;
                    }        
                    mVolumeRampEvent = VRE_RAMP_MUTE_LOCK;
                }
            }
                break;
	
            case AUD_VOL_EVENT_CHK_VOLUME_DELAY_RELEASE:    //for Fix issue for play channel check will cause pop sound
            {
                vre_ramp_count = 0;
                AudioDeviceManager_lowlevel_setMute( SOUND_MUTE ); 

                ramp_delay = 1;
                mVolumeRampEvent = VRE_NON_RAMP_MUTE_LOCK;
            }
            break;
		
            case AUD_VOL_EVENT_VOLUME_RAMP:
            {
                    mVolumeRampEvent = VRE_RAMP_INITIAL;
            }   
                break; 
        }
        ADM_I2C2_sema_mutex_give();
    }
}

static void AuidoDeviceManager_VolController_setEvent(void *parms )
    {
    xAudioVolParms* pParms = (xAudioVolParms*)parms;

    /* update mAudDevParms */
    mAudDevParms.mute = pParms->mute;
    mAudDevParms.master_gain= pParms->master_gain; 

    //TRACE_DEBUG((0, " >>>>> AuidoDeviceManager_VolController_setEvent = %d", pParms->adm_vol_event ));
    AuidoDeviceManager_lowlevel_VolController_setEvent(pParms->adm_vol_event);
}

static void AuidoDeviceManager_VolController_TaskCtrl( bool val )
{
    if ( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.VolTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_ADM_Parms.VolTaskHandle );
            TRACE_DEBUG((0, "SUSPEND: AuidoDeviceManager_VolController"));
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.VolTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_ADM_Parms.VolTaskHandle );
            TRACE_DEBUG((0, "RESUME: AuidoDeviceManager_VolController"));
        }
    }
}

static bool AuidoDeviceManager_VolController_RampStauts(void)
{
    if (mVolumeRampEvent == VRE_RAMP_START)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }  
}

static uint8 AuidoDeviceManager_VolController_VolStauts(void)
{
    return volume_inc;
}

static uint8 AuidoDeviceManager_VolController_TargetVol(void)
{
    return target_volume;
}

/*******************************************************************************
 * Digital Detector
 ******************************************************************************/
static void AudioDeviceManager_DigitalDetector_TaskCtrl( bool val )
{   
    if ( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.Digital_detector_handle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_ADM_Parms.Digital_detector_handle);
            ADM_I2C2_sema_mutex_give();/* prevent sema haven't give in the task */       
            //TRACE_DEBUG((0, "SUSPEND: DigitalDetector "));
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.Digital_detector_handle ) == pdPASS ) /*task is not working*/
        {
            vTaskDelay( TASK_MSEC2TICKS(50) ); /*wait to right uld for format led*/
            vTaskResume( xOS_ADM_Parms.Digital_detector_handle );
            //TRACE_DEBUG((0, "RESUME: DigitalDetector"));
        }
    }
}

void AudioDeviceManager_DigitalDetectorTask( void *pvParameters )
{
    bool ret;
#if ( configSII_DEV_953x_PORTING == 1 )    
    static xHDMIAudiotype CurrentHDMIAudiotype = HDMI_NON_PCM;
    static xHDMIAudiotype LastHDMIAudiotype = HDMI_NON_PCM;
    CS49844_QUEUE_TYPE QUEUE_TYPE;
#endif
    
    for( ;; )
    {
        switch( pAudioDevParms->input_src )
        {
            case AUDIO_SOURCE_OPTICAL:
            case AUDIO_SOURCE_COAXIAL:
#if ( configSII_DEV_953x_PORTING == 1 )
            case AUDIO_SOURCE_HDMI_IN:
            case AUDIO_SOURCE_HDMI_ARC:	
#endif
            { 
            /* Smith commanded 2013/4/05
            1. Confirm AES3 PCM 96Khz 
            2. Fix VIZIO TV short noise between DIGITAL AUDIO OUTPUT between PCM and bitstream.
            */
                if (ADM_I2C2_sema_mutex_take() == TRUE )
                { 
                    if ( AudioStream_get_PCM96Khz() )
                    {
                        ret = AudioSystem_isPCM_96Khz(FALSE);
                        AudioStream_set_PCM96Khz( ret );
                    }
                    else
                    {
                        ret = AudioSystem_isPCM_96Khz(TRUE);
                        AudioStream_set_PCM96Khz( ret );
                    }
                    AudioSystem_Cover_PCM96Khz();
                    ADM_I2C2_sema_mutex_give();
                }
                 /* for format LED*/
                    /* Tony need to change to Q*/
                if (AudioDeviceManager_getSignalAvailable() == 1)
                { 
                    if (format_indicator== TRUE)
                    {
                        format_indicator = FALSE;
                        switch(pDSP_ObjCtrl->GetAudioStreamType())
                        {
                            case CS49844_LOAD_AC3:
                            {
                                if (pAudioDevParms->op_mode==MODE_FACTORY)
                                {
                                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_FAC_DOLBY);
                                }
                                else
                                {
                                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_DOLBY);
                                }
                            }
                                break;

                            case CS49844_LOAD_DTS:
                            {
                                if (pAudioDevParms->op_mode==MODE_FACTORY)
                                {
                                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_FAC_DTS);
                                }
                                else
                                {
                                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_DTS);
                                }
                            }
                                break;
                                
                            case CS49844_LOAD_PCM:
                            {
                                 pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_PCM);
                            }
                                break;
                        }       
                    }
                }
                else
                {
                    format_indicator = TRUE;
                }
            }
                break;
        }
                
#if ( configSII_DEV_953x_PORTING == 1 )        
		/* MultiChannel PCM in HDMI*/
		switch( pAudioDevParms->input_src )
		{
			case AUDIO_SOURCE_HDMI_IN:
			case AUDIO_SOURCE_HDMI_ARC: 
			{
				if (AudioStream_get_PCM()==TRUE)
				{
					if (pHDMI_DM_ObjCtrl->GetMultiChannel()==TRUE)
					{
						if (AudioStream_get_PCM96Khz()==TRUE)
						{
							CurrentHDMIAudiotype = HDMI_PCM_MULTI_96K;
						}
						else
						{
							CurrentHDMIAudiotype = HDMI_PCM_MULTI_48K;
						}
					}
					else
					{
						CurrentHDMIAudiotype = HDMI_PCM_STEREO;
					}
				}
				else
				{
					CurrentHDMIAudiotype = HDMI_NON_PCM;
				}

				if (LastHDMIAudiotype!=CurrentHDMIAudiotype)
				{				 
					switch(CurrentHDMIAudiotype)
					{
						case HDMI_NON_PCM:
						case HDMI_PCM_STEREO:
						{
							QUEUE_TYPE.sample_96k = FALSE;
							QUEUE_TYPE.multi_channel = FALSE;
            }
                break;

						case HDMI_PCM_MULTI_48K:
						{
							QUEUE_TYPE.sample_96k = FALSE;
							QUEUE_TYPE.multi_channel = TRUE;
						}
							break;

						case HDMI_PCM_MULTI_96K:
						{
							QUEUE_TYPE.sample_96k = TRUE;
							QUEUE_TYPE.multi_channel = TRUE;
						}
							break;

						default:
							break;
					}
					QUEUE_TYPE.audio_type = CS49844_LOAD_HDMI_CTRL;
					QUEUE_TYPE.source_ctrl = CS49844_SOURCE_HDMI;
					
					if (pDSP_ObjCtrl->loader_load_fmt_mutex_take())
					{
						pDSP_ObjCtrl->loader_load_fmt_uld( &QUEUE_TYPE );	  
					}
					
					LastHDMIAudiotype = CurrentHDMIAudiotype;
				}
			}
				break;
        }
#endif        
        vTaskDelay( ADM_DIGITAL_DETECTOR_TIME_TICK );
    }
}

static void AuidoDeviceManager_DigitalDetector_CreateTask( void )
{
    if ( xTaskCreate( AudioDeviceManager_DigitalDetectorTask, 
            ( portCHAR * ) "ADM_SDT", 
            (STACK_SIZE), NULL, tskADM_DIGITAL_DETECTOR_PRIORITY,&xOS_ADM_Parms.Digital_detector_handle) != pdPASS )
    {
        vTaskDelete( xOS_ADM_Parms.Digital_detector_handle );
    }
    else
    {
        vTaskSuspend( xOS_ADM_Parms.Digital_detector_handle );
    }
}

bool AudioStream_get_PCM96Khz( void )
{
    return mIsPCM96Khz;
}

void AudioStream_set_PCM96Khz( bool bValue )
{
    mIsPCM96Khz = bValue;
}

bool AudioStream_get_PCM(void)
{
    return mIsPCM;
}

void AudioStream_set_PCM(bool bValue)
{
    mIsPCM = bValue;
}

bool AudioSystem_isPCM_96Khz( bool bReboot )
{
    if ( pSRC_ObjCtrl->Format_IsPCM() == TRUE )
    {
        AudioStream_set_PCM(TRUE);

        if ( pSRC_ObjCtrl->is_96Khz192Khz_AES3() == TRUE )
    {
        if (bReboot)
        {
            //TRACE_DEBUG((0, " ========================= CS8422_SoftReboot ============================"));
            pSRC_ObjCtrl->isr_ctrl( FUNC_DISABLE );
            pAudLowLevel_ObjCtrl->RstSRC();
            pSRC_ObjCtrl->isr_ctrl( FUNC_ENABLE );
            if( ( pSRC_ObjCtrl->is_96Khz192Khz_AES3() == TRUE ) )
                return TRUE;
            else 
                return FALSE;
        }
        else
            return TRUE;
    }
    }
    else
    {
        AudioStream_set_PCM(FALSE);
    }

    return FALSE;
}

/* Smith commanded: 
   2013/6/6 Here is only for AES3 PCM 96Khz and PCM 192Khz also is working 
   (optical or coxical) receiver input and 96Khz is dected by CS8422 GPO 0*/
void AudioSystem_Cover_PCM96Khz( void )
{
    uint8 ret;

    if ( AudioStream_get_PCM96Khz() == TRUE )    /*PCM96Khz only exist AES3 input*/
    {
        ret = pSRC_ObjCtrl->getForcesClockControl();
        if ( ret == 0 )
        {
            pSRC_ObjCtrl->ForcesClockControl( 1 );
            
            //TRACE_DEBUG((0, " PCM96 CS8422_ForcesClockControl( 1 ) "));
        }
    
        ret = pSRC_ObjCtrl->read_SDOUT1_DataSource();
        if ( ret == 1 ) /*if SDOUT1 is not equal to SRC*/ 
        {
            pSRC_ObjCtrl->SerialAudioOutput( 0 );    /*SRC OUTPUT*/

            //TRACE_DEBUG((0, " PCM96 CS8422_SerialAudioOutput( 0 ) "));
        }
    }
    else /*    PCM is under 96Khz. The PCM exist on AES3 and SRC */
    {
        ret = pSRC_ObjCtrl->getForcesClockControl();
        if ( ret == 1 )
        {
            pSRC_ObjCtrl->ForcesClockControl( 0 );

            //TRACE_DEBUG((0, " PCM48 CS8422_ForcesClockControl( 0 ) "));
        }

        ret = pSRC_ObjCtrl->read_SDOUT1_DataSource();
        if ( ret == 0 ) /*if SDOUT1 is equal to SRC*/ 
        {
            pSRC_ObjCtrl->SerialAudioOutput( 1 );    /*AES3 OUTPUT*/

            //TRACE_DEBUG((0, " PCM48 CS8422_SerialAudioOutput( 1 ) "));
        }
    }
}

/*******************************************************************************
 * Signal Detector
 ******************************************************************************/
static uint8 AudioDeviceManager_getSignalAvailable( void )
{
    return TRUE;
    
    //return SignalAvailable;   //Angus just for temporal
}

static void AudioDeviceManager_SignalDetector_TaskCtrl( bool val )
{
    if ( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.signal_detector_handle ) != pdPASS ) /*task is working*/
        {
            SignalAvailable = 0;/* When task suspened, it means DSP is in format change or not ready*/
            vTaskSuspend( xOS_ADM_Parms.signal_detector_handle);
            ADM_I2C2_sema_mutex_give();/* prevent sema haven't give in the task */
            //TRACE_DEBUG((0, "SUSPEND: SignalDetector "));
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.signal_detector_handle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_ADM_Parms.signal_detector_handle );
            //TRACE_DEBUG((0, "RESUME: SignalDetector"));
        }
    }
}

void AudioDeviceManager_SignalDetectorTask( void *pvParameters )
{
    for( ;; )
    {
        switch( pAudioDevParms->input_src )
        {
            case AUDIO_SOURCE_AUX1:
            case AUDIO_SOURCE_AUX2:
            case AUDIO_SOURCE_BLUETOOTH:
            case AUDIO_SOURCE_USB_PLAYBACK:
            {
                if (pDSP_ObjCtrl->GetLoadrState()==0)
                {
                    if (pDSP_ObjCtrl->GetSignalLevel() == 1 )
                    {
                        SignalAvailable = 1;
                    }
                    else
                    {
                        SignalAvailable = 0;
                    }
                }
                else
                {
                    SignalAvailable = 0;
                }
            }
                break;

            case AUDIO_SOURCE_OPTICAL:
            case AUDIO_SOURCE_COAXIAL:
#if ( configSII_DEV_953x_PORTING == 1 )
            case AUDIO_SOURCE_HDMI_IN:
            case AUDIO_SOURCE_HDMI_ARC:	
#endif
            {
                if (ADM_I2C2_sema_mutex_take() == TRUE )
                { 
                    int idx =0;

                    for (idx = 0 ; idx < 2 ; idx++)
                    {
                        //TRACE_DEBUG((0,"PLL Lock status %d -----> %d",idx, pSRC_ObjCtrl->IsLockPLL()));    

                        if (pSRC_ObjCtrl->IsLockPLL() == TRUE )
                        {                        
                            SignalAvailable = 1;
                            break;
                        }
                        else
                        {
                            if (idx == 0)
                            {
                                vTaskDelay(10);
                                continue;
                            }
                            
                            SignalAvailable = 0;
                        }
                    }
                    
                    ADM_I2C2_sema_mutex_give();
                }
            }
                break;
        }
        vTaskDelay( ADM_SIGNAL_DETECTOR_TIME_TICK );
    }
}

static void AuidoDeviceManager_SignalDetector_CreateTask( void )
{
    if ( xTaskCreate( AudioDeviceManager_SignalDetectorTask, 
            ( portCHAR * ) "ADM_SDT", 
            (STACK_SIZE), NULL, tskADM_SIGNAL_DETECTOR_PRIORITY,&xOS_ADM_Parms.signal_detector_handle) != pdPASS )
    {
        vTaskDelete( xOS_ADM_Parms.signal_detector_handle );
    }
    else
    {
        vTaskSuspend( xOS_ADM_Parms.signal_detector_handle );
    }
}


/*******************************************************************************
 * Mutex
 ******************************************************************************/
static bool ADM_I2C2_sema_mutex_take( void )
{
    //if ( bIsI2C2_enable!= TRUE )
    //{
    //    TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 1111 !! "));
    //    return FALSE;
    //}   
    //return TRUE;
    if ( xOS_ADM_Parms.ADM_I2C2_sema.xSemaphore == NULL )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 2222 !! "));
        return FALSE;
    }

    xOS_ADM_Parms.ADM_I2C2_sema.xBlockTime = portMAX_DELAY;
    if ( xSemaphoreTake( xOS_ADM_Parms.ADM_I2C2_sema.xSemaphore, xOS_ADM_Parms.ADM_I2C2_sema.xBlockTime ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 3333 !! "));
        return FALSE;
    }
    //TRACE_DEBUG((0, "ADM_I2C2_sema_mutex_take "));
    return TRUE;
}

static bool ADM_I2C2_sema_mutex_give( void )
{
    //if ( bIsI2C2_enable!= TRUE )
    //    return FALSE;
    //return TRUE;
    if ( xOS_ADM_Parms.ADM_I2C2_sema.xSemaphore == NULL )
        return FALSE;

    if ( xSemaphoreGive( xOS_ADM_Parms.ADM_I2C2_sema.xSemaphore ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 GIVE BUSY !! "));
        return FALSE;    
    }
    //TRACE_DEBUG((0, "ADM_I2C2_sema_mutex_give "));
    return TRUE;
}

static bool ADM_Call_Back_sema_mutex_take( void )
{
#if 1
    //if ( bIsI2C2_enable!= TRUE )
    //{
    //    TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 1111 !! "));
    //    return FALSE;
    //}   
    //return TRUE;
    if ( xOS_ADM_Parms.ADM_Call_Back_sema.xSemaphore == NULL )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 2222 !! "));
        return FALSE;
    }

    xOS_ADM_Parms.ADM_Call_Back_sema.xBlockTime = portMAX_DELAY;
    if ( xSemaphoreTake( xOS_ADM_Parms.ADM_Call_Back_sema.xSemaphore, xOS_ADM_Parms.ADM_Call_Back_sema.xBlockTime ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 TAKE BUSY 3333 !! "));
        return FALSE;
    }
    //TRACE_DEBUG((0, "ADM_I2C2_sema_mutex_take "));
#endif    
    return TRUE;
}

static bool ADM_Call_Back_sema_mutex_give( void )
{
#if 1
    //if ( bIsI2C2_enable!= TRUE )
    //    return FALSE;
    //return TRUE;
    if ( xOS_ADM_Parms.ADM_Call_Back_sema.xSemaphore == NULL )
        return FALSE;

    if ( xSemaphoreGive( xOS_ADM_Parms.ADM_Call_Back_sema.xSemaphore ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "ADM_I2C2 GIVE BUSY !! "));
        return FALSE;    
    }
    //TRACE_DEBUG((0, "ADM_I2C2_sema_mutex_give "));
#endif    
    return TRUE;
}

/*******************************************************************************
 * Function_Back
 ******************************************************************************/

void AuidoDeviceManager_lowlevel_Function_Back_CreateTask( void )
{
    if ( xTaskCreate( AuidoDeviceManager_lowlevel_BackCtl_task, 
            ( portCHAR * ) "AuidoDeviceManager_lowlevel_BackCtrl", 
            (STACK_SIZE), NULL, tskAUD_BACKCTRL_PRIORITY,&xOS_ADM_Parms.BackCtrlTaskHandle) != pdPASS )
    {
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_BackCtrl task create failure " ));
    }
    else
    {
        vTaskSuspend( xOS_ADM_Parms.BackCtrlTaskHandle);
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_BackCtrl task create OK?! " ));
    }

    AudioDevBackCtlParams.QParams.xQueue = xQueueCreate( AUDIO_DEV_BACKCTL_QUEUE_LENGTH, FRTOS_SIZE(CS49844_QUEUE_TYPE) );
    AudioDevBackCtlParams.QParams.xBlockTime = BLOCK_TIME(0);
    if( AudioDevBackCtlParams.QParams.xQueue == NULL )
    {
        TRACE_ERROR((0, "AudioDevBackCtlParams queue creates failure " ));
    }
    
}

void AuidoDeviceManager_lowlevel_Function_Back_TaskCtrl( bool val )
{

    if ( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.BackCtrlTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_ADM_Parms.BackCtrlTaskHandle );
            TRACE_DEBUG((0, "SUSPEND: AuidoDeviceManager_lowlevel_Function_Back_TaskCtrl"));
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.BackCtrlTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_ADM_Parms.BackCtrlTaskHandle );
            TRACE_DEBUG((0, "RESUME: AuidoDeviceManager_lowlevel_Function_Back_TaskCtrl"));
        }
    }
}

bool AuidoDeviceManager_lowlevel_Function_Back_InstrSender( const void *params ) 
{
   const CS49844_QUEUE_TYPE* pInstr = ( CS49844_QUEUE_TYPE *)params;
   int ret_val = 0;
    if ( pInstr == NULL )
    {
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Function_Back_InstrSender parameters error !! "));
        return FALSE;
    }
    
    if (AudioDevBackCtlParams.QParams.xQueue == NULL )
    {
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Function_Back_InstrSender xqueue is null !! "));
        return FALSE;
    }

    ret_val = xQueueSend( AudioDevBackCtlParams.QParams.xQueue, pInstr, AudioDevBackCtlParams.QParams.xBlockTime );
    if (ret_val != pdPASS)
    {
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Function_Back_InstrSender sends queue failure %d ",ret_val));
        return FALSE;
    }

    return TRUE;

}

static bool AuidoDeviceManager_lowlevel_Function_Back_InstrReceiver( CS49844_QUEUE_TYPE *pInstr ) 
{
    if ( pInstr == NULL )
    {
        TRACE_ERROR((0, " AuidoDeviceManager_lowlevel_Function_Back_InstrReceiver instruction recevier error !! "));
    }
    
    if ( AudioDevBackCtlParams.QParams.xQueue == NULL )
    {
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Function_Back_InstrReceiver queue is null !! "));
        return FALSE;
    }

    if ( xQueueReceive( AudioDevBackCtlParams.QParams.xQueue, pInstr, AudioDevBackCtlParams.QParams.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR((0, " AuidoDeviceManager_lowlevel_Function_Back_InstrReceiver receiver queue is failure "));
        return FALSE;
    }

    return TRUE;
}

static uint8 AudDevBackCtlHandler_GetQueueNumber( void )
{
    return (uint8)uxQueueMessagesWaiting(AudioDevBackCtlParams.QParams.xQueue);
}

void AuidoDeviceManager_lowlevel_BackCtl_task( void *pvParameters )
{
    CS49844_QUEUE_TYPE systemParams;
    
    for(;;)
    {
        //TRACE_DEBUG((0, "AuidoDeviceManager_lowlevel_BackCtl_task--1111111"));

        ADM_Call_Back_sema_mutex_take();
        switch( AudioDevBackCtlParams.taskState )
        {
            case TASK_SUSPENDED:
            {
                if( AudDevBackCtlHandler_GetQueueNumber() > 0 )  
                {
                    //TRACE_DEBUG((0, "AuidoDeviceManager_lowlevel_BackCtl_task--2222222222"));
                    AudioDevBackCtlParams.taskState = TASK_READY;
                }
                else
                {
                    break;
                }
            }
                break;
        
            case TASK_READY:
            {
                if ( AuidoDeviceManager_lowlevel_Function_Back_InstrReceiver( (CS49844_QUEUE_TYPE*)&systemParams ) == TRUE )
                {
                    //TRACE_DEBUG((0,"                                             source_ctrl=%X,audio_type=%X",systemParams.source_ctrl,systemParams.audio_type));
                    //TRACE_DEBUG((0, "AuidoDeviceManager_lowlevel_BackCtl_task"));
                    AudioDevBackCtlParams.taskState = TASK_RUNING;
                }
                else
                {
                    AudioDevBackCtlParams.taskState = TASK_SUSPENDED;
                    break;
                }
            }
                //break;

            case TASK_RUNING:
            {
                if (systemParams.source_ctrl == CS49844_SOURCE_ERROR)
                {
                    pDSP_ObjCtrl->loader_task_ctrl( xOS_setTASK_SUSPENDED );
                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_ULD_ERROR);
                    //AudioDevBackCtlParams.taskState = TASK_BLOCKED;
                    AudioDevBackCtlParams.taskState = TASK_SUSPENDED;
                    break;
                }
                
                switch( pAudioDevParms->input_src )
                {   
                    case AUDIO_SOURCE_AUX1:
                    case AUDIO_SOURCE_AUX2:
                    case AUDIO_SOURCE_BLUETOOTH:
                    case AUDIO_SOURCE_USB_PLAYBACK:
                    {
                        pDSP_ObjCtrl->lowlevel_task_ctrl( xOS_setTASK_RESUME );
                    }
                }
                
                AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_RESUME);
                
                if (systemParams.source_ctrl == CS49844_SOURCE_USER)
                {
                    format_indicator =TRUE; /* TODO: not a good way*/
                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_VOLUME_RAMP);
                }
#if ( configSII_DEV_953x_PORTING == 1 )                
                else if (systemParams.source_ctrl == CS49844_SOURCE_DSP ||systemParams.source_ctrl == CS49844_SOURCE_HDMI)
#else
                else if (systemParams.source_ctrl == CS49844_SOURCE_DSP)
#endif
                {
                    if (CurrentAudioStream != pDSP_ObjCtrl->GetAudioStreamType())
                    {
                        format_indicator =TRUE; /* TODO: not a good way*/
                    }
                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_SET_DEMUTE);
                }
                else if (systemParams.source_ctrl == CS49844_SOURCE_PROCESS_MODE)
                {
                    format_indicator =FALSE; 
                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_SET_DEMUTE);
                }
               
                CurrentAudioStream = pDSP_ObjCtrl->GetAudioStreamType();
               
                pSRC_ObjCtrl->isr_ctrl( FUNC_ENABLE );

                switch( pAudioDevParms->input_src )
                {
                    case AUDIO_SOURCE_OPTICAL:
                    case AUDIO_SOURCE_COAXIAL:
#if ( configSII_DEV_953x_PORTING == 1 )
                    case AUDIO_SOURCE_HDMI_IN:
                    case AUDIO_SOURCE_HDMI_ARC:	
#endif
                    {
                        pDSP_ObjCtrl->fch_task_ctrl( xOS_setTASK_RESUME );
                        AudioDeviceManager_DigitalDetector_TaskCtrl(BTASK_RESUME);
                    }
                }
 
                AudioDevBackCtlParams.taskState = TASK_BLOCKED;
            }
                //break;

            case TASK_BLOCKED:
            {
                AudioDevBackCtlParams.taskState = TASK_SUSPENDED;
            }
                break;    
        }
        ADM_Call_Back_sema_mutex_give();
        vTaskDelay(TASK_MSEC2TICKS(100));    
    }
}

/*******************************************************************************
 * Exception
 ******************************************************************************/
void AuidoDeviceManager_lowlevel_Exception_CreateTask( void )
{
    if ( xTaskCreate( AuidoDeviceManager_lowlevel_Exception_Task, 
            ( portCHAR * ) "AuidoExceptionHandle", 
            (STACK_SIZE), NULL, tskAUD_EXCEPTION_PRIORITY,&xOS_ADM_Parms.Audio_Exception_handle) != pdPASS )
    {
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Exception_Task create failure " ));
    }
    else
    {
        vTaskSuspend( xOS_ADM_Parms.Audio_Exception_handle);
        TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Exception_Task create OK?! " ));
    }

    AudioDevExceptParams.QParams.xQueue = xQueueCreate( AUDIO_DEV_EXCEPTION_QUEUE_LENGTH, FRTOS_SIZE(AUDIO_EXCEPTION_QUEUE_TYPE) );
    AudioDevExceptParams.QParams.xBlockTime = BLOCK_TIME(0);
    if( AudioDevExceptParams.QParams.xQueue == NULL )
    {
        TRACE_ERROR((0, "AudioDevExceptionParams queue creates failure " ));
    }
}

static void AuidoDeviceManager_lowlevel_Exception_TaskCtrl( bool val )
{
    if ( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.Audio_Exception_handle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_ADM_Parms.Audio_Exception_handle );
            //TRACE_DEBUG((0, "SUSPEND: AuidoDeviceManager_lowlevel_Exception_Task"));
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( xOS_ADM_Parms.Audio_Exception_handle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_ADM_Parms.Audio_Exception_handle );
            //TRACE_DEBUG((0, "RESUME: AuidoDeviceManager_lowlevel_Exception_Task"));
        }
    }
}


static bool AuidoDeviceManager_lowlevel_Exception_InstrSender( const void *params ) 
{
   const AUDIO_EXCEPTION_QUEUE_TYPE* pInstr = ( AUDIO_EXCEPTION_QUEUE_TYPE *)params;
   int ret_val = 0;
    if ( pInstr == NULL )
    {
        //TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Exception_InstrSender parameters error !! "));
        return FALSE;
    }
    
    if (AudioDevExceptParams.QParams.xQueue == NULL )
    {
        //TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Exception_InstrSender xqueue is null !! "));
        return FALSE;
    }

    ret_val = xQueueSend( AudioDevExceptParams.QParams.xQueue, pInstr, AudioDevExceptParams.QParams.xBlockTime );
    if (ret_val != pdPASS)
    {
        //TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Exception_InstrSender sends queue failure %d ",ret_val));
        return FALSE;
    }

    return TRUE;

}

static bool AuidoDeviceManager_lowlevel_Exception_InstrReceiver( AUDIO_EXCEPTION_QUEUE_TYPE *pInstr ) 
{
    if ( pInstr == NULL )
    {
        //TRACE_ERROR((0, " AuidoDeviceManager_lowlevel_Exception_InstrReceiver instruction recevier error !! "));
    }
    
    if ( AudioDevExceptParams.QParams.xQueue == NULL )
    {
        //TRACE_ERROR((0, "AuidoDeviceManager_lowlevel_Exception_InstrReceiver queue is null !! "));
        return FALSE;
    }

    if ( xQueueReceive( AudioDevExceptParams.QParams.xQueue, pInstr, AudioDevExceptParams.QParams.xBlockTime ) != pdPASS )
    {
        //TRACE_ERROR((0, " AuidoDeviceManager_lowlevel_Exception_InstrReceiver receiver queue is failure "));
        return FALSE;
    }

    return TRUE;
}

static uint8 AudDevExceptionHandler_GetQueueNumber( void )
{
    return (uint8)uxQueueMessagesWaiting(AudioDevExceptParams.QParams.xQueue);
}

void AuidoDeviceManager_lowlevel_Exception_Task( void *pvParameters )
{

    AUDIO_EXCEPTION_QUEUE_TYPE systemParams;

    for(;;)
    {
        ADM_Call_Back_sema_mutex_take();
        switch( AudioDevExceptParams.taskState )
        {
            case TASK_SUSPENDED:
            {
                if( AudDevExceptionHandler_GetQueueNumber() > 0 )  
                {
                    //TRACE_DEBUG((0, "AuidoDeviceManager_lowlevel_BackCtl_task2222222222"));
                    AudioDevExceptParams.taskState = TASK_READY;
                }
                else
                {
                    break;
                }
            }
                //break;
        
            case TASK_READY:
            {
                if ( AuidoDeviceManager_lowlevel_Exception_InstrReceiver( (AUDIO_EXCEPTION_QUEUE_TYPE*)&systemParams ) == TRUE )
                {
                    //TRACE_DEBUG((0,"                                             source_ctrl=%X,audio_type=%X",systemParams.source_ctrl,systemParams.audio_type));
                    //TRACE_DEBUG((0, "AuidoDeviceManager_lowlevel_BackCtl_task"));
                    AudioDevExceptParams.taskState = TASK_RUNING;
                }
                else
                {
                    AudioDevExceptParams.taskState = TASK_SUSPENDED;
                    break;
                }
                
            }
                //break;

            case TASK_RUNING:
            {
                if (systemParams.event == SRC_BUS_FAULT)
                {
                    AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_SUSPENDED );
                    AudioDeviceManager_DigitalDetector_TaskCtrl(BTASK_SUSPENDED);
                    //TRACE_DEBUG((0,"SRC_BUS_FAULT ADM_I2C2_sema_mutex_take"));
                    if (ADM_I2C2_sema_mutex_take() == TRUE )
                    {
                        pSRC_ObjCtrl->isr_ctrl( FUNC_DISABLE );
                        pAudLowLevel_ObjCtrl->RstSRC(); 
                        ADM_I2C2_sema_mutex_give();
                        pSRC_ObjCtrl->isr_ctrl( FUNC_ENABLE );
                    }
                    AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_RESUME);
                    switch( pAudioDevParms->input_src )
                    {
                        case AUDIO_SOURCE_OPTICAL:
                        case AUDIO_SOURCE_COAXIAL:
#if ( configSII_DEV_953x_PORTING == 1 )
                        case AUDIO_SOURCE_HDMI_IN:
                        case AUDIO_SOURCE_HDMI_ARC:	
#endif
                        {
                            //TRACE_DEBUG((0,"exception- AudioDeviceManager_DigitalDetector_TaskCtrl(BTASK_RESUME);"));
                            AudioDeviceManager_DigitalDetector_TaskCtrl(BTASK_RESUME);
                        }
                        break;
                    }
                    TRACE_DEBUG((0,"SRC_BUS_FAULT"));
                }

                if(systemParams.event == SRC_RECEIVER_ERROR)
                {
                    pDSP_ObjCtrl->setSilenceThreshold(FALSE);/* 0.25 sec*/
                    //TRACE_DEBUG((0,"Silence threshold 0.25~~~~~~~~~~~~~~~"));
                    if( pAudioDevParms->mute == SOUND_DEMUTE )
                    {
                        if (ADM_I2C2_sema_mutex_take() == TRUE )
                        {
                            AudioDeviceManager_lowlevel_setMute( SOUND_DEMUTE );
                            ADM_I2C2_sema_mutex_give();
                        }
                    }
                }
                
                if(systemParams.event == SRC_PCM_SILENCE)
                {
                    pDSP_ObjCtrl->setSilenceThreshold(TRUE);/* 90 sec*/
                    //TRACE_DEBUG((0,"Silence threshold 90s~~~~~~~~~~"));
                }

                if(systemParams.event == SRC_FCH)
                {
                    AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_SUSPENDED );
                    AudioDeviceManager_DigitalDetector_TaskCtrl(BTASK_SUSPENDED);
                    //TRACE_DEBUG((0,"~~~SRC_FCH ADM_I2C2_sema_mutex_take"));
                    if (ADM_I2C2_sema_mutex_take() == TRUE )
                    {
                        pSRC_ObjCtrl->isr_ctrl( FUNC_DISABLE );
                        pAudLowLevel_ObjCtrl->RstSRC(); 
                        ADM_I2C2_sema_mutex_give();
                        pSRC_ObjCtrl->isr_ctrl( FUNC_ENABLE );
                    }
                    AudioDeviceManager_SignalDetector_TaskCtrl( BTASK_RESUME);
                    switch( pAudioDevParms->input_src )
                    {
                        case AUDIO_SOURCE_OPTICAL:
                        case AUDIO_SOURCE_COAXIAL:
#if ( configSII_DEV_953x_PORTING == 1 )
                        case AUDIO_SOURCE_HDMI_IN:
                        case AUDIO_SOURCE_HDMI_ARC:	
#endif
                        {
                            //TRACE_DEBUG((0,"AudioDeviceManager_DigitalDetector_TaskCtrl(BTASK_RESUME);"));
                            AudioDeviceManager_DigitalDetector_TaskCtrl(BTASK_RESUME);
                        }
                        break;
                    }
                    //TRACE_DEBUG((0,"SRC_FCH~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
                }
                AudioDevExceptParams.taskState = TASK_BLOCKED;
            }
                //break;

            case TASK_BLOCKED:
            {
                AudioDevExceptParams.taskState = TASK_SUSPENDED;
            }
                break;    
        }
        ADM_Call_Back_sema_mutex_give();
        vTaskDelay(TASK_MSEC2TICKS(50));    
    }
}

