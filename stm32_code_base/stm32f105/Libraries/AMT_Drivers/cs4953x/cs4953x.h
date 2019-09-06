#ifndef __CS4953x_H__
#define __CS4953x_H__

#include "Defs.h"

typedef enum
{
    CS4953x_LOAD_NULL = 0,
    CS4953x_LOAD_PCM,
    CS4953x_LOAD_AC3,
    CS4953x_LOAD_DTS,
    CS4953x_LOAD_USER_CTRL,    
    CS4953x_LOAD_HDMI_CTRL,
}CS4953xLoadAudioStreamType;

typedef enum
{
    CS4953x_SOURCE_NONE = 0,
    CS4953x_SOURCE_USER,
    CS4953x_SOURCE_DSP,
    CS4953x_SOURCE_DSP_INITIAL,
    CS4953x_SOURCE_PROCESS_MODE,
    CS4953x_SOURCE_HDMI,
    CS4953x_SOURCE_ERROR,
    CS4953x_SOURCE_NUM
}CS4953xSourceCtrl;

typedef struct _CS4953x_QUEUE_TYPE
{
    xHMIOperationMode op;
    CS4953xSourceCtrl source_ctrl;
    CS4953xLoadAudioStreamType audio_type;
    bool multi_channel;
    bool sample_96k;
    bool EQ;
    bool srs_tshd;
    bool srs_truvol;
    int reserve;
}CS4953x_QUEUE_TYPE;

typedef struct
{
    xHMIOperationMode op;
    bool multi_channel;
    bool sample_96k;
    bool EQ;
    bool srs_tshd;
    bool srs_truvol;
}CS4953xMode;

typedef enum
{
    ROUTE_INITIAL,
    ROUTE_FL_FR,
    ROUTE_RR_RL,
    ROUTE_CENTER,
    ROUTE_LFE0,
    ROUTE_FL,
    ROUTE_FR,
    ROUTE_RR, 
    ROUTE_RL,
    ROUTE_FL_FR_CENTER,
    ROUTE_RR_RL_WOOFER,
    ROUTE_DISABLE
} CS4953xAudioRoute;

typedef struct _CS4953x_LOWLEVEL_OBJECT
{
    void (*loader_create_task)(void);
    void (*loader_task_ctrl)(xOS_TaskCtrl val);
    void (*fch_create_task)(void);
    void (*fch_task_ctrl)(xOS_TaskCtrl val);
	void (*fch_task_set_state)( TaskHandleState state );    
    bool (*loader_load_fmt_uld)(void *parms);
	void (*lowlevel_create_task)(void);
	void (*lowlevel_task_ctrl)(xOS_TaskCtrl val);
	void (*lowlevel_task_set_state)( TaskHandleState state );
    void (*setMute)( bool val);
    void (*setMasterGain)( uint32 value);
    void (*setBassGain)( uint32 value);
    void (*setTrebleGain)( uint32 value);
    void (*setSubGain )( uint32 value );
    void (*setCenterGain )( uint32 value );
    void (*setLsRsGain )( uint32 value );
    void (*setBalanceLs )( uint32 value );
    void (*setBalanceRs )( uint32 value );
    void (*SetTVHDMC_Ctrl )( byte* data, uint16 length );
    void (*setNightMode )( uint32 value );
    void (*setAVDelay )( uint32 value );
    void (*setAudioRoute)(CS4953xAudioRoute val);
    void (*setSilenceThreshold)( bool value );
    CS4953xLoadAudioStreamType (*GetAudioStreamType)( void );
    uint8 (*GetLoadrState)( void );
    uint8 (*GetSignalLevel)( void );
}DSP_CTRL_OBJECT;

#endif /*__CS4953x_H__*/
