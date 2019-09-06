#ifndef __CS49844_H__
#define __CS49844_H__

#include "Defs.h"

typedef enum
{
    CS49844_LOAD_NULL = 0,
    CS49844_LOAD_PCM,
    CS49844_LOAD_AC3,
    CS49844_LOAD_DTS,
    CS49844_LOAD_USER_CTRL,    
    CS49844_LOAD_HDMI_CTRL,
}CS49844LoadAudioStreamType;

typedef enum
{
    CS49844_SOURCE_NONE = 0,
    CS49844_SOURCE_USER,
    CS49844_SOURCE_DSP,
    CS49844_SOURCE_DSP_INITIAL,
    CS49844_SOURCE_PROCESS_MODE,
    CS49844_SOURCE_HDMI,
    CS49844_SOURCE_ERROR,
    CS49844_SOURCE_NUM
}CS49844SourceCtrl;

typedef struct _CS49844_QUEUE_TYPE
{
    CS49844SourceCtrl source_ctrl;
    CS49844LoadAudioStreamType audio_type;
    bool multi_channel;
    bool sample_96k;
    bool EQ;
    bool srs_tshd;
    bool srs_truvol;
    int reserve;
}CS49844_QUEUE_TYPE;

typedef struct
{
    xHMIOperationMode op;
    bool multi_channel;
    bool sample_96k;
    bool EQ;
    bool srs_tshd;
    bool srs_truvol;
}CS49844Mode;

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
} CS49844AudioRoute;

typedef struct _CS49844_LOWLEVEL_OBJECT
{
    void (*loader_create_task)(void);
    void (*loader_task_ctrl)(xOS_TaskCtrl val);
    void (*fch_create_task)(void);
    void (*fch_task_ctrl)(xOS_TaskCtrl val);
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
    void (*setAudioRoute)(CS49844AudioRoute val);
    void (*setSilenceThreshold)( bool value );
    CS49844LoadAudioStreamType (*GetAudioStreamType)( void );
    uint8 (*GetLoadrState)( void );
    uint8 (*GetSignalLevel)( void );
    bool (*loader_load_fmt_mutex_take)( void );
    bool (*loader_load_fmt_mutex_give)( void );
}DSP_CTRL_OBJECT;

#endif /*__CS49844_H__*/
