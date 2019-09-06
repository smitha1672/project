#ifndef __AUDIO_DEVICE_MANAGER_H__
#define __AUDIO_DEVICE_MANAGER_H__

#include "Defs.h"
#include "api_typedef.h"
#include "device_config.h"

#include "TAS5727.h"
#include "TAS5711.h"
#include "CS8422.h"
#include "cs4953x.h"
#include "CS5346.h"

typedef enum ADM_QUEUE_EVENT
{
    ADM_QUEUE_EVENT_NULL = 0,
    ADM_QUEUE_EVENT_FROM_ISR_DIGITAL_LOCK,
    ADM_QUEUE_EVENT_FROM_ISR_DIGITAL_UNLOCK,
    ADM_QUEUE_EVENT_FROM_DSP_ANALOG_LOCK,
    ADM_QUEUE_EVENT_FROM_DSP_ANALOG_UNLOCK,
    ADM_QUEUE_EVENT_FROM_DSP_CHECK_PLL,
    ADM_QUEUE_EVENT_FORM_ISR_DIGITAL_OVER_SAMPLE_RATE_48KHZ,
    ADM_QUEUE_EVENT_FORM_ISR_DIGITAL_UNDER_SAMPLE_RATE_96KHZ,
    ADM_QUEUE_EVENT_FORM_DIGITAL_SOURCE_CHECK_PLL,
#if ( configSII_DEV_953x_PORTING == 1 )
    ADM_QUEUE_EVENT_FROM_HDMI_MC_PCM_REQUEST,
    ADM_QUEUE_EVENT_FROM_HDMI_2CH_PCM_REQUEST,
#endif    
    ADM_QUEUE_EVENT_STREAM_FORMAT_INDICATOR_DTS,
    ADM_QUEUE_EVENT_STREAM_FORMAT_INDICATOR_AC3,
    ADM_QUEUE_EVENT_STREAM_FORMAT_INDICATOR_PCM
}xADM_QUEUE_Events; 


typedef struct _AUDIO_DEVICE_CTRL
{
    void (*CreateTask)(void);  
    void (*initialize)( void *parms );
    void (*Deinitialize)(void);
    void (*DeviceConfig)( void *parms );       /* HostLibInitialized */
    xAudDeviceParms (*GetAudioParams)( void );
    void (*SetInputPath)( AUDIO_SOURCE idx );
    void (*CreateVolTask)( void );
    void (*SetVolumeEvent )( void *parms );
    bool (*VolController_RampStauts)(void);
    uint8 (*VolController_VolStauts)(void);
    uint8 (*VolController_TargetVol)(void);
    void (*VolTaskCtrl )( bool val );
    void (*SetBassGain )( uint8 idx );
    void (*SetTrebleGain )( uint8 idx );
    void (*SetSubGain )( uint8 idx );
    void (*SetCenterGain )( uint8 idx );
    void (*setLsRs )(uint8 idx);
    void (*SetBalance )( uint8 idx );
    void (*SetSRSTruVolTSHD)( void *parms );
    void (*SetNightMode )( uint8 idx );
    void (*SetAVDelay )( uint8 idx );
    void (*Factory_AQBypass)( bool EQ );
    uint8 (*GetSignalAvailable)(void);
    void (*SetAudioRoutCtrl)(AudioRouteCtrl router_idx);
}AUDIO_DEVICE_MANAGER_OBJECT;

typedef struct _AUDIO_DEVICE_QUEUE_CTRL
{
	bool (*SignalDetector_sender) ( const void *params );
}AUDIO_DEVICE_QUEUE_OBJECT;


typedef struct _AUDIO_DEVICE_LOWLEVEL_CTRL
{
    void (*AudParmsConf)( void *parms);  
    void (*RstSRC)( void );
    void (*SetVolRampEvent )( xADMVolEvents parms );
    void (*SetLSRS )( uint8 idx );
    uint8(*getAudioStreamType)( void );
    bool (*ExceptionSender )( const void *params );
    bool (*BackCtrlSender)( const void *params );

}AUDIO_LOWLEVEL_DRIVER_OBJECT;

#endif 
