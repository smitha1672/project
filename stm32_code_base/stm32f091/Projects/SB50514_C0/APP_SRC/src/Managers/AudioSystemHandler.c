#include "BSP.h"
#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "audio_gain_parameters.h"
#include "AudioDeviceManager.h"
#include "AudioSystemHandler.h"

//_____________________________________________________________________________
#define SERVICE_HANLDER_TIME_TICK  TASK_MSEC2TICKS(1)    
#define AUDIO_SYS_QUEUE_LENGTH 8

//_____________________________________________________________________________
typedef struct AUDIO_SYS_HANDLE_PARAMETERS
{
    xTaskHandle Handle;
    uint8 power_handle;
    xQueueParameters QParams;
} xAudSysHandleParams;

typedef enum
{
    AUD_SYSTEM_CLR_RESET = 0,
    AUD_SYSTEM_SET_RESET,
}xAudSysRstCtrl;

//_____________________________________________________________________________
extern BSP_GPIO_OBJECT *pBSP_GPIO_OBJECT;


//_____________________________________________________________________________
/*static variable*/
static xAudSysHandleParams AudioSysParams;

//_____________________________________________________________________________
//static API header
static void AudioSystemHandler_CreateTask( void );

static void AudioSystemHandler_HardwareRst( void );

static void AudioSystemHandler_ClrRst( void );

static void AudioSystemHandler_SystemInformation( void );

#if ( configSII_DEV_953x_PORTING == 1 )
static xHMI_SiiAppPowerStatus_t AudioSystemHandler_getHDMIPowerStatus(void);

static bool AudioSystemHandler_IsHDMIPowerOff(void);
#endif

static bool AudioSystemHandler_InstrSender( const void *params ) ;

static xAudioVolParms AudioSystemHandler_AudioVolParmsAdapter( xADMVolEvents event, xHMISystemParams *pSource );

static uint8 AudioSystemHandler_getSignalAvailable( void ) ;

static bool AudioSystemHandler_getRampStatus(void);

static uint8 AudioSystemHandler_getRampVol(void);

static uint8 AudioSystemHandler_getTargetVol(void);

//_____________________________________________________________________________
extern AUDIO_DEVICE_MANAGER_OBJECT *pADM_ObjCtrl;

const AUDIO_SYSTEM_HANDLER_OBJECT AudioSystemHandler =
{
    AudioSystemHandler_CreateTask,
    AudioSystemHandler_InstrSender,
    AudioSystemHandler_getSignalAvailable,
    AudioSystemHandler_getRampStatus,
    AudioSystemHandler_getRampVol,
    AudioSystemHandler_getTargetVol,
#if ( configSII_DEV_953x_PORTING == 1 )
    AudioSystemHandler_getHDMIPowerStatus,
    AudioSystemHandler_IsHDMIPowerOff,
#endif
};
const AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl = &AudioSystemHandler;

//_____________________________________________________________________________
static uint8 AudioSystemHandler_getSignalAvailable( void ) 
{
    uint8 ret = 0;

    ret = pADM_ObjCtrl->GetSignalAvailable();

    return ret;
}

static bool AudioSystemHandler_getRampStatus(void)
{
    bool ret = FALSE;

    ret = pADM_ObjCtrl->VolController_RampStauts();

    return ret;
}

static uint8 AudioSystemHandler_getRampVol(void)
{
    uint8 ret = 0;

    ret = pADM_ObjCtrl->VolController_VolStauts();

    return ret;
}

static uint8 AudioSystemHandler_getTargetVol(void)
{
    uint8 ret = 0;

    ret = pADM_ObjCtrl->VolController_TargetVol();

    return ret;
}

#if ( configSII_DEV_953x_PORTING == 1 )
static xHMI_SiiAppPowerStatus_t AudioSystemHandler_getHDMIPowerStatus(void)
{
    return pHDMI_DM_ObjCtrl->GetHDMIPowerStatus();
}

static bool AudioSystemHandler_IsHDMIPowerOff(void)
{
    return pHDMI_DM_ObjCtrl->ishdmiOff();
}
#endif

static bool AudioSystemHandler_InstrSender( const void *params ) 
{
   xHMISystemParams* pInstr = ( xHMISystemParams *)params;

#if ( configAPP_ASH == 1 )
    if ( pInstr == NULL )
    {
        TRACE_ERROR((0, "AudioSystemHandler_InstrSender parameters error !! "));
        return FALSE;
    }
    
    if (AudioSysParams.QParams.xQueue == NULL )
    {
        TRACE_ERROR((0, "AudioSystemHandler_InstrSender xqueue is null !! "));
        return FALSE;
    }
    
    if ( xQueueSend( AudioSysParams.QParams.xQueue, pInstr, AudioSysParams.QParams.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR((0, "AudioSystemHandler_InstrSender sends queue failure "));
        return FALSE;
    }
#endif 
    return TRUE;

}

static bool AudioSystemHandler_InstrReceiver( xHMISystemParams *pInstr ) 
{
    if ( pInstr == NULL )
    {
        TRACE_ERROR((0, " AudioSystemHandler_InstrReceiver instruction recevier error !! "));
        return FALSE;
    }
    
    if ( AudioSysParams.QParams.xQueue == NULL )
    {
        TRACE_ERROR((0, "AudioSystemHandler_InstrReceiver queue is null !! "));
        return FALSE;
    }

    if ( xQueueReceive( AudioSysParams.QParams.xQueue, pInstr, AudioSysParams.QParams.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR((0, " AudioSystemHandler_InstrReceiver receiver queue is failure "));
        return FALSE;
    }

    return TRUE;
}

static void AudioSystemHandler_RstCtrl( xAudSysRstCtrl RstCtrl )
{
    switch ( RstCtrl )
    {
        case AUD_SYSTEM_SET_RESET:  /*Set device reset pin as High*/
        {
            pBSP_GPIO_OBJECT->set( __O_RST_DSP );
            
        }
            break;

        case AUD_SYSTEM_CLR_RESET:
        {
            pBSP_GPIO_OBJECT->clr( __O_RST_DSP );
        }
            break;

    }
}

static void AudioSystemHandler_HardwareRst( void )
{
	vTaskDelay( 200 );
        
    AudioSystemHandler_RstCtrl( AUD_SYSTEM_SET_RESET );
    vTaskDelay( 100 );
}

static void AudioSystemHandler_ClrRst( void )
{
    AudioSystemHandler_RstCtrl( AUD_SYSTEM_CLR_RESET );
    vTaskDelay( 100 );
}


static xAudDeviceParms AudioSystemHandler_AudioParmsAdapter( const xHMISystemParams *pSource )
{
    /*sync audio parameters between auido device manager and hmi interface */
    xAudDeviceParms dest;

    dest.op_mode = pSource->op_mode;
    dest.input_src = pSource->input_src;
    dest.mute = pSource->mute;
    dest.master_gain = pSource->master_gain;
    dest.bass_gain = pSource->bass_gain;
    dest.treble_gain = pSource->treble_gain;
    dest.sub_gain = pSource->sub_gain;
    dest.center_gain = pSource->center_gain;
    dest.LsRs_gain = pSource->LsRs_gain;
    dest.balance = pSource->balance;
    dest.srs_truvol = pSource->srs_truvol;
    dest.srs_tshd = pSource->srs_tshd;
    dest.night_mode = pSource->night_mode;
    dest.av_delay = pSource->av_delay;
    
    return dest;
}

static void AudioSystemHandler_handleCommand( void *params  )
{
    if ( params == NULL )
        return;        
    
    xHMISystemParams* pSystemParms = (xHMISystemParams*)params;

    switch( pSystemParms->sys_event )
    {
        case SYS_EVENT_SYSTEM_UP:
        {
            xAudDeviceParms Dest; 
            
            AudioSystemHandler_HardwareRst( );

            Dest = AudioSystemHandler_AudioParmsAdapter( pSystemParms );
			
            pADM_ObjCtrl->initialize( &Dest );

            /*Smith fixes: Setup processing mode*/
            pADM_ObjCtrl->SetSRSTruVolTSHD( &Dest );

            AudioSystemHandler_SystemInformation( );
        }
            break;

        case SYS_EVENT_SYSTEM_DOWN:
        {
            pADM_ObjCtrl->Deinitialize( );
            AudioSystemHandler_ClrRst( );

         }
            break;

        case SYS_EVENT_DEV_CONFIG:
        {
            xAudDeviceParms Dest;
            Dest = AudioSystemHandler_AudioParmsAdapter( pSystemParms );
            pADM_ObjCtrl->DeviceConfig( &Dest );
            pADM_ObjCtrl->SetInputPath( Dest.input_src );
        }
            break;

        case SYS_EVENT_SEL_SRC:
        {
            pADM_ObjCtrl->SetInputPath( pSystemParms->input_src );
        }
            break;

        case SYS_EVENT_VOLUME_SET:
        {
            xAudioVolParms Dest;
            Dest = AudioSystemHandler_AudioVolParmsAdapter( AUD_VOL_EVENT_SET_MASTER_GAIN, pSystemParms );
            pADM_ObjCtrl->SetVolumeEvent( &Dest );
        }
            break;

        case SYS_EVENT_VOLUME_MUTE_SET:
        {
            xAudioVolParms Dest;
            Dest = AudioSystemHandler_AudioVolParmsAdapter( AUD_VOL_EVENT_USER_MUTE, pSystemParms );
            pADM_ObjCtrl->SetVolumeEvent( &Dest );
        }
            break;
            
        case SYS_EVENT_VOLUME_DELAY_RELEASE:
        {
            xAudioVolParms Dest;
            Dest = AudioSystemHandler_AudioVolParmsAdapter( AUD_VOL_EVENT_VOLUME_DELAY_RELEASE, pSystemParms );
            pADM_ObjCtrl->SetVolumeEvent( &Dest );
        }
            break;
#if 1   //Fix issue for play channel check will cause pop sound
	    case SYS_EVENT_CHK_VOLUME_DELAY_RELEASE:
        {
            xAudioVolParms Dest;
            Dest = AudioSystemHandler_AudioVolParmsAdapter( AUD_VOL_EVENT_CHK_VOLUME_DELAY_RELEASE, pSystemParms );
            pADM_ObjCtrl->SetVolumeEvent( &Dest );
        }
            break;
#endif			
        case SYS_EVENT_BASS_SET:
        {
            pADM_ObjCtrl->SetBassGain( pSystemParms->bass_gain);
        }
            break;

        case SYS_EVENT_TREBLE_SET:
        {
            pADM_ObjCtrl->SetTrebleGain( pSystemParms->treble_gain);
        }
            break;

        case SYS_EVENT_SUB_SET:
        {
            pADM_ObjCtrl->SetSubGain( pSystemParms->sub_gain);
        }
            break;

        case SYS_EVENT_CENTER_SET:
        {
            pADM_ObjCtrl->SetCenterGain( pSystemParms->center_gain);
        }
            break;

        case SYS_EVENT_LSRS_SET:
        {
            pADM_ObjCtrl->setLsRs( pSystemParms->LsRs_gain);
        }
        break;

        case SYS_EVENT_BALANCE_SET:
        {
            pADM_ObjCtrl->SetBalance( pSystemParms->balance);
        }
            break;
        
        case SYS_EVENT_SRS_TRUVOL_TSHD_SET:
        {
            xAudDeviceParms Dest;
            Dest = AudioSystemHandler_AudioParmsAdapter( pSystemParms );
            pADM_ObjCtrl->SetSRSTruVolTSHD( &Dest );
        }
            break;
            
        case SYS_EVENT_NIGHT_MODE_SET:
        {
            pADM_ObjCtrl->SetNightMode( pSystemParms->night_mode);
        }
        break;

        case SYS_EVENT_AV_DELAY_SET:
        {
            pADM_ObjCtrl->SetAVDelay( pSystemParms->av_delay);
        }
        break;
    
        case SYS_EVENT_FAC_ROUTE_CTRL_CENTER:
        {
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_CENTER);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_LFE0:
        {
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_LFE0);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_FL:
        {
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_FL);
        }
        break;
        
        case SYS_EVENT_FAC_ROUTE_CTRL_FR:
        {
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_FR);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_RR:
        {
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_RR);
        }
        break;
        
        case SYS_EVENT_FAC_ROUTE_CTRL_RL:
        {
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_RL);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_DISABLE:
        {
            //for normal state
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_DISABLE);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_FL_FR:
        {
            //for normal state
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_FL_FR);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_RR_RL:
        {
            //for normal state
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_RR_RL);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_FL_FR_CENTER:
        {
            //for normal state
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_FL_FR_CENTER);
        }
        break;

        case SYS_EVENT_FAC_ROUTE_CTRL_RR_RL_WOOFER:
        {
            //for normal state
            pADM_ObjCtrl->SetAudioRoutCtrl(ROUTE_CTRL_RR_RL_WOOFER);
        }
        break;

        case SYS_EVENT_FAC_AQ_CTRL_OFF:
        {
            pADM_ObjCtrl->Factory_AQBypass(FALSE);
        }
        break;

        case SYS_EVENT_FAC_AQ_CTRL_ON:
        {
            pADM_ObjCtrl->Factory_AQBypass(TRUE);
        }
        break;

    default:
        break;
    }
}

void AudioSystemHandler_ServiceHandle( void *pvParameters )
{
    xHMISystemParams systemParams = {
        SYS_EVENT_SYSTEM_UP,
        MODE_USER,    
        DEFAULT_APD,
        DEFAULT_INPUT_SRC,
        DEFAULT_LAST_INPUT_SRC,
        DEFAULT_MUTE_STATE, /*mute*/
        DEFAULT_MASTER_GAIN, /*master gain*/
        DEFAULT_BASS_GAIN,
        DEFAULT_TREBLE_GAIN,
        DEFAULT_SUB_GAIN,
        DEFAULT_CENTER_GAIN,
        DEFAULT_REAR_GAIN,
        DEFAULT_BALANCE,
        DEFAULT_SRS_TRUVOL,
        DEFAULT_SRS_TSHD,
        DEFAULT_NIGHT_MODE,
        DEFAULT_AV_DELAY,
        DEFAULT_VIZIO_RMT,
        DEFAULT_AID
    };

    AudioSystemHandler_handleCommand( (xHMISystemParams*)&systemParams );
    
    
    for( ;; )
    {
		if ( AudioSystemHandler_InstrReceiver( (xHMISystemParams*)&systemParams ) == TRUE )
		{
			AudioSystemHandler_handleCommand( (xHMISystemParams*)&systemParams );
		}
    }
}

static void AudioSystemHandler_CreateTask( void )
{
#if ( configAPP_ASH == 1 )
    AudioSysParams.QParams.xQueue = xQueueCreate( AUDIO_SYS_QUEUE_LENGTH, FRTOS_SIZE(xHMISystemParams) );
    AudioSysParams.QParams.xBlockTime = portMAX_DELAY; /*no wait - Smith*/
    ASSERT( AudioSysParams.QParams.xQueue != NULL );
    
    if ( xTaskCreate( 
        AudioSystemHandler_ServiceHandle, 
        ( portCHAR * ) "AUD_HANDLER", 
        (STACK_SIZE), 
        NULL, 
        tskAUD_HANDLER_PRIORITY, 
        &AudioSysParams.Handle ) != pdPASS )
    {
        vTaskDelete( AudioSysParams.Handle );
    }

    pADM_ObjCtrl->CreateTask( );

#endif     

}

static xAudioVolParms AudioSystemHandler_AudioVolParmsAdapter( xADMVolEvents event, xHMISystemParams *pSource )
{
    /*sync audio parameters between auido device manager and hmi interface */
    xAudioVolParms destination;

    destination.adm_vol_event = event;
    destination.master_gain= pSource->master_gain;
    destination.mute= pSource->mute;
    destination.srs_truvol = pSource->srs_truvol;
    destination.srs_tshd = pSource->srs_tshd;
    
    return destination;
}

static void AudioSystemHandler_SystemInformation(void)
{
    TRACE_INFO((STR_ID_NULL, "\n"));
    TRACE_INFO((STR_ID_NULL, "##############################################"));
#if defined ( S4051A )    
    TRACE_INFO((STR_ID_NULL, "S4051A-OS Version: %X", VERSION));
#elif defined ( S4051B )    
    TRACE_INFO((STR_ID_NULL, "S4051B-OS Version: %X", VERSION));  
#elif defined ( S4551A ) 
    TRACE_INFO((STR_ID_NULL, "S4551A-OS Version: %X", VERSION));
#elif defined ( S3851C )
    TRACE_INFO((STR_ID_NULL, "S3851C-OS Version: %X", VERSION));
#elif defined ( S3851D )
    TRACE_INFO((STR_ID_NULL, "S3851D-OS Version: %X", VERSION));    
#elif defined ( S3851E )
    TRACE_INFO((STR_ID_NULL, "S3851E-OS Version: %X", VERSION));      
#elif defined ( S3851F )
    TRACE_INFO((STR_ID_NULL, "S3851F-OS Version: %X", VERSION));      
#endif
    TRACE_INFO((STR_ID_NULL, "##############################################"));
    TRACE_INFO((STR_ID_NULL, "\n"));
}

