#ifndef __AUDIO_DEVICE_MANAGER_H__
#define __AUDIO_DEVICE_MANAGER_H__

#include "Defs.h"
#include "api_typedef.h"

#include "TAS5727.h"
#include "TAS5711.h"
#include "CS8422.h"
#include "CS49844.h"
#include "CS5346.h"

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

typedef struct _AUDIO_DEVICE_LOWLEVEL_CTRL
{
    void (*AudParmsConf)( void *parms);  
    void (*RstSRC)( void );
    void (*SetVolRampEvent )( xADMVolEvents parms );
    void (*SetLSRS )( uint8 idx );
    uint8(*getAudioStreamType)( void );
    void (*CreateExceptionTask )( void );
    void (*ExceptionTaskCtrl)(bool val);
    bool (*ExceptionSender )( const void *params );
    void (*CreateBackCtrlTask)( void );
    void (*BackCtrlTaskCtrl)(bool val);
    bool (*BackCtrlSender)( const void *params );

}AUDIO_LOWLEVEL_DRIVER_OBJECT;

#endif 
