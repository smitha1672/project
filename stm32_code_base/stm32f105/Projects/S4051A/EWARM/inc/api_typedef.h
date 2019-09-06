#ifndef __API_TYPEDEF_H__
#define __API_TYPEDEF_H__

#include "Defs.h"
#include "device_config.h"

#define FUNC_ENABLE TRUE
#define FUNC_DISABLE FALSE

#define AUDIO_FORMAT_DETECT_PCM		0x80
#define AUDIO_FORMAT_DETECT_IE61937	0x40
#define AUDIO_FORMAT_DETECT_AC3		0x01
#define AUDIO_FORMAT_DETECT_DTS		0x11
#define AUDIO_FORMAT_DETECT_UNKNOW		0xFF

#define AUDIO_PLL_UNLOCK		0x00
#define AUDIO_ANALOG_INPUT	0x01 	

#define SOUND_MUTE	TRUE
#define SOUND_DEMUTE	FALSE

//! Smith implemented for CS5314 @{
#define NON_IEC61937_STREAM_FLAG(VAL) (VAL & 0x00000020)

#define NON_IEC61937_STREAM_DESCRIPTOR(VAL) (VAL & 0x0000001F)

#define NON_IEC61937_SILENT_INPUT_DATA 0x00
#define NON_IEC61937_DTS_FORMAT_16	0x01
#define NON_IEC61937_DTS_FORMAT_14	0x02
#define NON_IEC61937_LPCM 0x03
#define NON_IEC61937_HDCD_PCM_SYNC_DETECT	0x04
#define NON_IEC61937_HDCD_PCM_SYNC_LOST	0x05

#define IEC61937_STREAM_DESCRIPTOR(VAL) (VAL & 0x0000001F)
#define IEC61937_STREAM_NEVER_REPORTED_00	0x00
#define IEC61937_STREAM_AC3	0x01
#define IEC61937_STREAM_NEVER_REPORTED_03	0x03
#define IEC61937_STREAM_MPGE_1_L1	0x04
#define IEC61937_STREAM_MPGE_1_L23	0x05
#define IEC61937_STREAM_MPGE_2_EXTENSTION	0x06
#define IEC61937_STREAM_MPGE_2_AAC	0x07
#define IEC61937_STREAM_DTS_1 	0x0B
#define IEC61937_STREAM_DTS_2	0x0C
#define IEC61937_STREAM_DTS_3	0x0D
//!@}

//! UART protocol DATA TYPES for Factory@{
#define PC_CMD_HEADER_POS	0
#define PC_CMD_TYPE_POS	1
#define PC_CMD_LENGTH_POS	2
#define PC_CMD_OPCODE_POS	3
#define PC_CMD_VALUE_POS	4

#define PC_CMD_HEADER	0xC5
#define PC_CMD_SET	0x63
#define PC_CMD_GET	0x73

#define PC_ACK_SUCCESS	0xE0
#define PC_ACK_CHECKSUM_NG	0xE1
#define PC_ACK_TIMEOUT	0xE2
#define PC_ACK_INVALID_CMD_TYPE	0xE3
#define PC_ACK_INVALID_FUC_CODE	0xE4
#define PC_ACK_DATA_RANG_ERR	0xE5	/*OP code or data range error*/
#define PC_ACK_UNDEFINE_ERR	0xEF	/*undefine error*/
#define PC_ACK_NACK 0xFE

//!@}

//!	WAVEPLAYER_Private_Defines @{
#define  CHUNK_ID                            0x52494646  /* correspond to the letters 'RIFF' */
#define  FILE_FORMAT                         0x57415645  /* correspond to the letters 'WAVE' */
#define  FORMAT_ID                           0x666D7420  /* correspond to the letters 'fmt ' */
#define  DATA_ID                             0x64617461  /* correspond to the letters 'data' */
#define  FACT_ID                             0x66616374  /* correspond to the letters 'fact' */
#define  WAVE_FORMAT_PCM                     0x01
#define  FORMAT_CHNUK_SIZE                   0x10
#define  CHANNEL_MONO                        0x01
#define  CHANNEL_STEREO                      0x02
#define  SAMPLE_RATE_8000                    8000
#define  SAMPLE_RATE_11025                   11025
#define  SAMPLE_RATE_16000					 16000
#define  SAMPLE_RATE_22050                   22050
#define  SAMPLE_RATE_32000                   32000
#define  SAMPLE_RATE_44100                   44100
#define  SAMPLE_RATE_48000                   48000

#define  BITS_PER_SAMPLE_8                   8
#define  BITS_PER_SAMPLE_16                  16
//!@}

#define MASTER_GAIN_MAX 35
#define MASTER_GAIN_MIN 0

/*Smith comment: Following UI 4.7 - 7.2.2 to modify step*/
#define BASS_MAX 24
#define BASS_MIN 0

/*Smith comment: Following UI 4.7 - 7.2.3 to modify step*/
#define TREBLE_MAX 24
#define TREBLE_MIN 0

/*Smith comment: Following UI 4.7 - 7.2.7 to modify step*/
#define SUB_MAX 24
#define SUB_MIN 0

/*Smith comment: Following UI 4.7 - 7.2.4 to modify step*/
#define CENTER_MAX 24
#define CENTER_MIN 0

/*Smith comment: Following UI 4.7 - 7.2.5 to modify step*/
#define REAR_MAX 24
#define REAR_MIN 0

/*Smith comment: Following UI 4.7 - 7.2.6 to modify step*/
#define BALANCE_MAX 24
#define BALANCE_MIN 0

#if ( configLipSync_12steps == 1)
#define AV_DELAY_MAX 11
#else
#define AV_DELAY_MAX 23
#endif
#define AV_DELAY_MIN 0

#define DEFAULT_OP_MODE MODE_USER
#define DEFAULT_POWER_STATE  POWER_OFF
#define DEFAULT_APD TRUE
#define DEFAULT_LAST_INPUT_SRC    AUDIO_SOURCE_AUX1
#define DEFAULT_INPUT_SRC    AUDIO_SOURCE_OPTICAL//so auto input search can be started from OPTICAL
#define DEFAULT_MUTE_STATE   SOUND_DEMUTE
#define DEFAULT_MASTER_GAIN  14
#define DEFAULT_BASS_GAIN  (BASS_MAX/2)
#define DEFAULT_TREBLE_GAIN  (TREBLE_MAX/2)
#define DEFAULT_SUB_GAIN  (SUB_MAX/2)
#define DEFAULT_CENTER_GAIN  (CENTER_MAX/2)
#define DEFAULT_REAR_GAIN  (REAR_MAX/2)
#define DEFAULT_BALANCE  (BALANCE_MAX/2)
#define DEFAULT_SRS_TRUVOL  0
#define DEFAULT_SRS_TSHD  1
#define DEFAULT_NIGHT_MODE 0
#define DEFAULT_AV_DELAY  0
#define DEFAULT_VIZIO_RMT 1  
#define DEFAULT_AID 1

#if ( configSII_DEV_953x_PORTING == 1 )
#define DEFAULT_CEC_SAC 0	//set 1 for ACT test  
#define DEFAULT_CEC_ARC 0  	//set 1 for ACT test
#define DEFAULT_CEC_ARC_Tx 0xf  
#endif

#define DEMO_1_MASTER_GAIN_MAX 28
#define DEMO_1_MASTER_GAIN_MIN 3
#define DEMO_1_OP_MODE MODE_DEMO_1
#define DEMO_1_APD FALSE
#define DEMO_1_INPUT_SRC    AUDIO_SOURCE_OPTICAL
#define DEMO_1_MUTE_STATE   SOUND_DEMUTE
#define DEMO_1_MASTER_GAIN  21
#define DEMO_1_BASS_GAIN  DEFAULT_BASS_GAIN
#define DEMO_1_TREBLE_GAIN  DEFAULT_TREBLE_GAIN
#define DEMO_1_SUB_GAIN  DEFAULT_SUB_GAIN+3
#define DEMO_1_CENTER_GAIN  DEFAULT_CENTER_GAIN
#define DEMO_1_REAR_GAIN  DEFAULT_REAR_GAIN
#define DEMO_1_BALANCE  DEFAULT_BALANCE
#define DEMO_1_SRS_TRUVOL  0
#define DEMO_1_SRS_TSHD  1
#define DEMO_1_NIGHT_MODE 0
#define DEMO_1_AV_DELAY  0
#define DEMO_1_VIZIO_RMT 1  
#define DEMO_1_AID 0

#define DEMO_2_INPUT_SRC AUDIO_SOURCE_USB_PLAYBACK
#define DEMO_3_INPUT_SRC AUDIO_SOURCE_AUX1

#define FACTORY_INPUT_SRC AUDIO_SOURCE_AUX1
#define FACTORY_MASTER_GAIN 20
#define FACTORY_HALF_GAIN 29
#define FACTORY_GAIN_TOOGLE TRUE
#define FACTORY_AID 0

#if ( configSII_DEV_953x_PORTING == 1 )
#define AUTO_AUDIO_SOURCE_MAX AUDIO_SOURCE_HDMI_ARC+1
#else
#define AUTO_AUDIO_SOURCE_MAX AUDIO_SOURCE_OPTICAL+1
#endif

/* Struct for easy bit access */
typedef union Bits {
    struct {
        unsigned tv_csii:1;
        unsigned tv_tru_volume:1;
        unsigned tshd_IV:1;
        unsigned tv_night:1;
        unsigned bit4:1;
        unsigned bit5:1;
        unsigned bit6:1;
        unsigned bit7:1;
    } bits;
    unsigned char byte;
} ProcessMode;

/**
 * Types of supported audio sources.
 */
typedef enum {
    AUDIO_SOURCE_AUX1 = 0,
    AUDIO_SOURCE_AUX2,
    AUDIO_SOURCE_COAXIAL,
    AUDIO_SOURCE_OPTICAL,
#if ( configSII_DEV_953x_PORTING )
    AUDIO_SOURCE_HDMI_IN,
    AUDIO_SOURCE_HDMI_ARC,
#endif
    AUDIO_SOURCE_USB_PLAYBACK,
    AUDIO_SOURCE_BLUETOOTH,     
    AUDIO_SOURCE_MAX,
    AUDIO_SOURCE_NOISE_CHECK
} AUDIO_SOURCE;

typedef enum{
    POWER_HANDLE_IDLE,
    POWER_HANDLE_UP,
    POWER_HANDLE_DOWN
}xPowerHandleState;

typedef enum{
    POWER_HANDLE_5V_ON,
    POWER_HANDLE_5V_OFF,
    POWER_HANDLE_5V_UNKNOW

}xPowerHandle_5V_Ctrl;

typedef enum{
    POWER_HANDLE_SII953X,
    POWER_HANDLE_OTHER

}xPowerHandle_ID;

typedef struct{
    xPowerHandle_ID pwr_handle_id;
    xPowerHandleState pwr_handle_state;
    xPowerHandle_5V_Ctrl pwr_handle_5v_ctrl;
    bool b_pwr_handle_restart;

}xPowerHandle_Queue;

typedef enum{
    POWER_ON = 0,
    POWER_OFF = 1, 
    POWER_UP = 2,
    POWER_DOWN = 3 
}POWER_STATE;

#if ( configSII_DEV_953x_PORTING )
typedef enum  _xHMI_SiiAppPowerStatus_t
{
    xHMI_POWERSTATUS_ON              = 0x00,
    xHMI_POWERSTATUS_STANDBY         = 0x01,
    xHMI_POWERSTATUS_STANDBY_TO_ON   = 0x02,
    xHMI_POWERSTATUS_ON_TO_STANDBY   = 0x03,
    xHMI_POWERSTATUS_INITIAL         = 0x04,
} xHMI_SiiAppPowerStatus_t;
#endif

typedef enum
{
    xHMI_EVENT_NULL = 0,
    xHMI_EVENT_BUTTON,
    xHMI_EVENT_XRS351_IR,
    xHMI_EVENT_VIZ_FAC_IR,
    xHMI_EVENT_PROGRAM_IR,
    xHMI_EVENT_UART,
    xHMI_EVENT_APC,
    xHMI_EVENT_AID,
#if ( configSII_DEV_953x_PORTING )
    xHMI_EVENT_CEC,
#endif
    xHMI_EVENT_BT
}xHMISrv_EventType; 

typedef enum HMI_USER_EVENT
{
    USER_EVENT_NULL = 0,
    USER_EVENT_POWER,
    USER_EVENT_FORCE_POWER_UP,
    USER_EVENT_APD_TOOGLE,
    USER_EVENT_APD_ON,
    USER_EVENT_APD_OFF,
    USER_EVENT_SEL_LAST_SRC,
    USER_EVENT_SEL_SRC,
    USER_EVENT_START_AUTO_SEL_SRC,
    USER_EVENT_AUTO_SEL_SRC,
    USER_EVENT_SEL_AUX1,
    USER_EVENT_SEL_AUX2,//10
    USER_EVENT_SEL_COAXIAL,
    USER_EVENT_SEL_OPTICAL,
    USER_EVENT_SEL_USB_PLAYBACK,
    USER_EVENT_SEL_BT,
#if ( configSII_DEV_953x_PORTING )
    USER_EVENT_SEL_HDMI_IN,
    USER_EVENT_CEC_SEL_HDMI_IN,
    USER_EVENT_SEL_HDMI_ARC,
    USER_EVENT_CEC_SEL_HDMI_ARC,
#endif
    USER_EVENT_CCK,
    USER_EVENT_VOL_UP,//20
    USER_EVENT_VOL_DN,
    USER_EVENT_MUTE,
    USER_EVENT_PREVIOUS,
    USER_EVENT_NEXT,
    USER_EVENT_PLAY_PAUSE,
    USER_EVENT_BT_PAIR,
    USER_EVENT_BT_CLEAN,
    USER_EVENT_BT_AUTO,
    USER_EVENT_DEVICE_RESET,
    USER_EVENT_BASS_UP,//30
    USER_EVENT_BASS_DN,
    USER_EVENT_TREBLE_UP,
    USER_EVENT_TREBLE_DN,
    USER_EVENT_SUB_UP,
    USER_EVENT_SUB_DN,
    USER_EVENT_CENTER_UP,
    USER_EVENT_CENTER_DN,
    USER_EVENT_REAR_UP,
    USER_EVENT_REAR_DN,
    USER_EVENT_BALANCE_UP,//40
    USER_EVENT_BALANCE_DN,
    USER_EVENT_SRS_TRUVOL_ON,
    USER_EVENT_SRS_TRUVOL_OFF,
    USER_EVENT_SRS_TSHD_ON,
    USER_EVENT_SRS_TSHD_OFF,
    USER_EVENT_NIGHT_MODE_ON,
    USER_EVENT_NIGHT_MODE_OFF,
    USER_EVENT_AV_DELAY_UP,
    USER_EVENT_AV_DELAY_DN,
    USER_EVENT_VIZ_RMT_TOOGLE,//50
    USER_EVENT_VIZ_RMT_ON,
    USER_EVENT_VIZ_RMT_OFF,
    USER_EVENT_MODE_DEMO1,
    USER_EVENT_MODE_DEMO2,
    USER_EVENT_MODE_DEMO3,
    USER_EVENT_MODE_FACTORY,
    USER_EVENT_START_MODE,
    USER_EVENT_STOP_MODE,
    USER_EVENT_RF_PAIR,
    USER_EVENT_PROGRAM_IR_VOL_UP,//60 
    USER_EVENT_PROGRAM_IR_VOL_DN,
    USER_EVENT_PROGRAM_IR_MUTE,
    USER_EVENT_AUTO_INPUT_DETECT_CLEAN,
    USER_EVENT_VERSION,
#if ( configSII_DEV_953x_PORTING )
    USER_EVENT_CEC_POWER_ON,
    USER_EVENT_CEC_POWER_ON_HDMI_ARC,
    USER_EVENT_CEC_POWER_ON_HDMI_IN,
    USER_EVENT_CEC_POWER_OFF,
    USER_EVENT_CEC_SET_MUTE,
    USER_EVENT_CEC_SET_DEMUTE,
    USER_EVENT_CEC_SET_SAC_ON,//70
    USER_EVENT_CEC_SET_SAC_OFF,
    USER_EVENT_CEC_SET_ARC_ON,
    USER_EVENT_CEC_SET_ARC_OFF,
    USER_EVENT_CEC_SET_VOLUME_UP,
    USER_EVENT_CEC_SET_VOLUME_DOWN,
    USER_EVENT_CEC_SET_VOLUME_MUTE,
    USER_EVENT_SAC_WAKE_UP,
#endif
    USER_EVENT_CENTER_ONLY,
    USER_EVENT_WOOFER_ONLY,//80
    USER_EVENT_LEFT_ONLY,
    USER_EVENT_RIGHT_ONLY,
    USER_EVENT_NORMAL_SOUND,
    USER_EVENT_SL_ONLY,
    USER_EVENT_SR_ONLY,
    USER_EVENT_LEFT_CENTER_RIGHT_ONLY,
    USER_EVENT_SL_WOOFER_SR_ONLY,
    USER_EVENT_FAC_OUTPUT_POWER_TOOGLE,
    USER_EVENT_FAC_RF_PAIRING,
    USER_EVENT_FAC_BT_PAIRING,
    USER_EVENT_FAC_SEL_AUX1,
    USER_EVENT_FAC_SEL_AUX2,
    USER_EVENT_FAC_SEL_COAXIAL,
    USER_EVENT_FAC_SEL_OPTICAL,
    USER_EVENT_FAC_SEL_USB_PLAYBACK,
#if ( configSII_DEV_953x_PORTING )
    USER_EVENT_FAC_SEL_HDMI_IN,
    USER_EVENT_FAC_SEL_HDMI_ARC,    
#endif
    USER_EVENT_FAC_LED_CTRL,
    USER_EVENT_FAC_AQ_OFF,    
    USER_EVENT_FAC_AQ_ON,    
    USER_EVENT_FAC_MODE_OFF,
    USER_EVENT_FAC_CLEAN_STORAGE,
    //USER_EVENT_FAC_SPK_BALANCE,
    USER_EVENT_UNDEFINED,
}xHMIUserEvents; /*event is used for IR, button and UART.*/


typedef enum AUD_SYS_EVENT
{
    SYS_EVENT_NULL = 0,
    SYS_EVENT_SYSTEM_UP,
    SYS_EVENT_SYSTEM_DOWN,
    SYS_EVENT_DEV_CONFIG,
    SYS_EVENT_SEL_SRC,
    SYS_EVENT_AUTO_SEL_SRC,
    SYS_EVENT_VOLUME_SET,
    SYS_EVENT_VOLUME_MUTE_SET,
    SYS_EVENT_VOLUME_DELAY_RELEASE,  //mute some time and unmute late
    SYS_EVENT_CHK_VOLUME_DELAY_RELEASE,	//mute a little time for fixing channel check pop sound.
    SYS_EVENT_BASS_SET,
    SYS_EVENT_TREBLE_SET,
    SYS_EVENT_SUB_SET,
    SYS_EVENT_CENTER_SET,
    SYS_EVENT_LSRS_SET,
    SYS_EVENT_BALANCE_SET,
    SYS_EVENT_SRS_TRUVOL_TSHD_SET,
    SYS_EVENT_SRS_TRUVOL_SET,
    SYS_EVENT_SRS_TSHD_SET,
    SYS_EVENT_VIZ_RMT_SET,
    SYS_EVENT_APD_SET,
    SYS_EVENT_NIGHT_MODE_SET,
    SYS_EVENT_AV_DELAY_SET,
    SYS_EVENT_USB_MMS_PLAY,
    SYS_EVENT_USB_MMS_NEXT,
    SYS_EVENT_USB_MMS_PREVIOUS,
    SYS_EVENT_USB_MMS_PLAY_PAUSE,
    SYS_EVENT_SAVE_USER_SETTING,
    SYS_EVENT_BT_PAIRING,
    SYS_EVENT_BT_AVRCP_PLAY_PAUSE,
    SYS_EVENT_BT_AVRCP_NEXT,
    SYS_EVENT_BT_AVRCP_PREVIOUS,
    SYS_EVENT_BT_CLEAN_DEVICE,
    SYS_EVENT_SYSTEM_RESET,
    SYS_EVENT_VERSION,
    SYS_EVENT_OUTPUT_POWER_TOOGLE,
#if ( configSII_DEV_953x_PORTING )
    SYS_EVENT_SAC_REPORT_AUDIO_STATUS,
    SYS_EVENT_HDMI_AUTO_PWR_ON,
    SYS_EVENT_HDMI_AUTO_PWR_OFF,
#endif
//----------------FACTORY-----------------//
    SYS_EVENT_FAC_ROUTE_CTRL_INITIAL,
    SYS_EVENT_FAC_ROUTE_CTRL_FL_FR,
    SYS_EVENT_FAC_ROUTE_CTRL_RR_RL,
    SYS_EVENT_FAC_ROUTE_CTRL_CENTER,
    SYS_EVENT_FAC_ROUTE_CTRL_LFE0,
    SYS_EVENT_FAC_ROUTE_CTRL_FL,
    SYS_EVENT_FAC_ROUTE_CTRL_FR,
    SYS_EVENT_FAC_ROUTE_CTRL_RR,
    SYS_EVENT_FAC_ROUTE_CTRL_RL,
    SYS_EVENT_FAC_ROUTE_CTRL_FL_FR_CENTER,
    SYS_EVENT_FAC_ROUTE_CTRL_RR_RL_WOOFER,
    SYS_EVENT_FAC_ROUTE_CTRL_DISABLE,
    SYS_EVENT_FAC_ALL_LED,
    SYS_EVENT_FAC_ALL_LED_DD,
    SYS_EVENT_FAC_ALL_LED_DTS,
    SYS_EVENT_FAC_LED1,
    SYS_EVENT_FAC_LED2,
    SYS_EVENT_FAC_LED3,
    SYS_EVENT_FAC_LED4,
    SYS_EVENT_FAC_LED5,
    SYS_EVENT_FAC_LED6,
    SYS_EVENT_FAC_LED7,
    SYS_EVENT_FAC_LED8,
    SYS_EVENT_FAC_LED9,
    SYS_EVENT_FAC_LED10,
    SYS_EVENT_FAC_LED11,
    SYS_EVENT_FAC_LED12,
    SYS_EVENT_FAC_LED13,
    SYS_EVENT_FAC_LED14,
    SYS_EVENT_FAC_NO_LED,   
    SYS_EVENT_FAC_AQ_CTRL_ON,
    SYS_EVENT_FAC_AQ_CTRL_OFF,
    SYS_EVENT_FAC_SET_MODE_ON,
    SYS_EVENT_FAC_SET_MODE_OFF,
//----------------FACTORY-----------------//
    SYS_EVENT_UNDEFINED,
}xHMIAudioSysEvents; 

typedef enum
{
    ROUTE_CTRL_INITIAL,
    ROUTE_CTRL_FL_FR,
    ROUTE_CTRL_RR_RL,
    ROUTE_CTRL_CENTER,
    ROUTE_CTRL_LFE0,
    ROUTE_CTRL_FL,
    ROUTE_CTRL_FR,
    ROUTE_CTRL_RR, 
    ROUTE_CTRL_RL,
    ROUTE_CTRL_FL_FR_CENTER,
    ROUTE_CTRL_RR_RL_WOOFER,
    ROUTE_CTRL_DISABLE
} AudioRouteCtrl;

typedef enum HMI_OPERATION_MODE
{
    MODE_NULL = 0,
    MODE_USER,
    MODE_DEMO_1,
    MODE_DEMO_2,
    MODE_DEMO_3,
    MODE_FACTORY,
}xHMIOperationMode;

typedef struct
{
    xHMISrv_EventType event_id;
    xHMIUserEvents event;
    uint8 params;
}xHMISrvEventParams;

typedef struct HMI_SYSTEM_PARMS
{
    xHMIAudioSysEvents sys_event;
    xHMIOperationMode op_mode;
    bool apd;
    AUDIO_SOURCE input_src;
    AUDIO_SOURCE last_input_src;
    bool mute;
    uint8 master_gain;
    uint8 bass_gain;
    uint8 treble_gain;
    uint8 sub_gain;
    uint8 center_gain;
    uint8 LsRs_gain;
    uint8 balance;
    bool srs_truvol;
    bool srs_tshd;
    bool night_mode;
    uint8 av_delay;
    bool vizio_rmt;

    bool auto_input_detect;
#if ( configSII_DEV_953x_PORTING )
    bool cec_sac;
    bool cec_arc;
    uint8 cec_arc_tx;
#endif
}xHMISystemParams;

typedef struct AUD_DEV_PARMS
{
    xHMIOperationMode op_mode;
    AUDIO_SOURCE input_src;
    bool mute;
    uint8 master_gain;
    uint8 bass_gain;
    uint8 treble_gain;
    uint8 sub_gain;
    uint8 center_gain;
    uint8 LsRs_gain;
    uint8 balance;
    bool srs_truvol;
    bool srs_tshd;
    bool night_mode;
    uint8 av_delay;
    bool EQ;
}xAudDeviceParms;

typedef enum UI_EVENT
{
    UI_EVENT_NONE = 0,
    UI_Event_VolRAMP,
    UI_Event_VolRAMP_TARGET,
    UI_EVENT_NORMAL,
    UI_EVENT_PCM,    
    UI_EVENT_DOLBY,
    UI_EVENT_FAC_DOLBY,
    UI_EVENT_DTS,
    UI_EVENT_FAC_DTS,
    UI_EVENT_BT_PAIRING,     
    UI_EVENT_BT_PAIRED,
    UI_EVENT_RF_PAIRING,    
    UI_EVENT_RF_PAIRED,
    UI_EVENT_PRG_REMOTE_LEARNING,
    UI_EVENT_PRG_REMOTE_LEARNED,
    UI_EVENT_ENTER_DEMO1,
    UI_EVENT_ENTER_DEMO2,
    UI_EVENT_ENTER_DEMO3,
    UI_EVENT_EXIT_DEMO,
    UI_EVENT_TIMEOUT_DEMO,
    UI_EVENT_EXIT_TIMEOUT_DEMO,
    UI_EVENT_BT_CLEAN_DEVICE,
    UI_EVENT_FAC_ALL_LED,
    UI_EVENT_FAC_NO_LED,
    UI_EVENT_ROUTE_INITIAL,
    UI_EVENT_ROUTE_FL_FR,
    UI_EVENT_ROUTE_RR_RL,
    UI_EVENT_ROUTE_CENTER,
    UI_EVENT_ROUTE_LFE0,
    UI_EVENT_ROUTE_FL,
    UI_EVENT_ROUTE_FR,
    UI_EVENT_ROUTE_RR, 
    UI_EVENT_ROUTE_RL,
    UI_EVENT_ROUTE_FL_FR_CENTER,
    UI_EVENT_ROUTE_RR_RL_WOOFER,
    UI_EVENT_ROUTE_DISABLE,
    UI_EVENT_ULD_ERROR,
    UI_EVENT_UNSUPPORT,
    
}xUIExceptionEvents; 

typedef struct UI_DEV_PARMS
{
    //bool RF_Paired; /* Tony141027 need to extend for more RF status */
    //bool BT_Paired; /* Tony141027 need to extend for more BT status */
    bool mute;
    uint8 master_gain;
}xUIDeviceParms;

typedef enum AUD_VOL_EVENT
{
    AUD_VOL_EVENT_NULL = 0,
    AUD_VOL_EVENT_USER_MUTE,
    AUD_VOL_EVENT_SET_MUTE,
    AUD_VOL_EVENT_SET_DEMUTE,
    AUD_VOL_EVENT_VOLUME_DELAY_RELEASE,
    AUD_VOL_EVENT_USB_POP_DELAY_RELEASE,	//mute a little time for fixing channel usb pause/start pop sound.
    AUD_VOL_EVENT_CHK_VOLUME_DELAY_RELEASE,	//mute a little time for fixing channel check pop sound.
    AUD_VOL_EVENT_FCH_VOLUME_DELAY_RELEASE,	//mute a little time for fixing change audio format pop sound.
    AUD_VOL_EVENT_SET_MASTER_GAIN,
    AUD_VOL_EVENT_VOLUME_RAMP,
    AUD_VOL_EVENT_UNDEFINED,
}xADMVolEvents; 

typedef struct AUD_VOL_PARMS
{
    xADMVolEvents adm_vol_event;
    bool mute;
    uint8 master_gain;
    bool srs_truvol;
    bool srs_tshd;
}xAudioVolParms;

typedef enum{
    PWR_STATE_SUBSYS_NULL,
    PWR_STATE_SUBSYS_TURN_ON_DONE,
    PWR_STATE_SUBSYS_TURN_OFF_DONE
}xPowerStateSubSystem;

typedef enum{
    DEMO_MODE_NULL_STATE = 0,
    DEMO_MODE_STOP_STATE,
    DEMO_MODE_START_STATE    
}xDemoModeState;

typedef enum {
    IR_PROTOCOL_NULL = 0,
    IR_PROTOCOL_38K_NEC,
    IR_PROTOCOL_38K_SAMSUNG,
    IR_PROTOCOL_38K_PANASONIC,
    IR_PROTOCOL_38K_JVC,
    IR_PROTOCOL_38K_SHARP,
    IR_PROTOCOL_36K_RC5,
    IR_PROTOCOL_36K_RC6,
    IR_PROTOCOL_40K_SONY,
    IR_PROTOCOL_DECODE_ERROR,
    IR_PROTOCOL_UNKNOW
}IR_PROTOCOL_CLASS;

typedef enum {
    IR_PROTOCOL_REPEAT_NULL = 0,
    IR_PROTOCOL_38K_NEC_REPEAT,
    IR_PROTOCOL_38K_SAMSUNG_REPEAT,
    IR_PROTOCOL_38K_PANASONIC_REPEAT,
    IR_PROTOCOL_38K_JVC_REPEAT,
    IR_PROTOCOL_38K_SHARP_REPEAT,
    IR_PROTOCOL_36K_RC5_REPEAT,
    IR_PROTOCOL_36K_RC6_REPEAT,
    IR_PROTOCOL_40K_SONY_REPEAT,
    IR_PROTOCOL_UNKNOW_REPEAT
}IR_PROTOCOL_REPEAT_CLASS;

typedef union IR_PROTOCOL_BITS {
    struct {
        IR_PROTOCOL_REPEAT_CLASS repeat_type: 4;
        IR_PROTOCOL_CLASS protocol_type:4;
    } bits;
    unsigned char byte;
} IR_PROTOCOL_INFO;

typedef struct NEC_PROTOCOL_STRUCTURE
{
    uint8 address;
    uint8 int_address;
    uint8 command;
    uint8 int_command;
}NEC_PROTOCOL_PAYLOAD;

typedef struct JVC_PROTOCOL_STRUCTURE
{
    uint16 reserve;
    uint8 address;
    uint8 command;
}JVC_PROTOCOL_PAYLOAD;


typedef struct IR_PROTOCOL_STRUCTURE
{
    IR_PROTOCOL_INFO info;
    uint32 payload;
}IR_PROTOCOL_PAYLOAD;

typedef struct
{
    uint8 keyCode;
    xHMIUserEvents UserEvent;
}xIR_CMD_KEY_MAP;

typedef struct _IR_PRG_REMOTE_USER_EVENT
{
    IR_PROTOCOL_PAYLOAD ir_payload;
    xHMIUserEvents user_event;
}IR_PRG_REMOTE_USER_EVENT;

/* Defines the supported cells that can be kept on the Storage Device 0 */
typedef enum
{
    STORAGE_DEVICE_CELL_PRODUCT_SERIAL_NUMBER = 0, // 0
    STORAGE_DEVICE_CELL_USER_PARAMETERS,
    STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_UP,
    STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_DN,
    STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_MUTE,
    STORAGE_DEVICE_CELL_END
}STORAGE_DEVICE_CELL;

/* for exception */
typedef enum 
{
    EXCEPTION_NONE = 0,
    SRC_CHIP_FAULT,
    SRC_BUS_FAULT,
    SET_DSP_DEFAULT_SILEN_THRES,
    SET_DSP_PS3_SILEN_THRES,
    SRC_FCH,
    DSP_FORMAT_CHANG,
    EXCEPTION_NUM
}xAudioExceptionEvents;

/* for Call Back control*/
typedef enum 
{
    BACKCTRL_NONE = 0,
    BACKCTRL_START_WAIT,
    BACKCTRL_FCH_COMPLETE,
    BACKCTRL_FCH_NOCHANGE,

    BACKCTRL_NUM
}xBackCtrlEvents;

typedef struct _AUDIO_EXCEPTION_QUEUE_TYPE
{
    xAudioExceptionEvents event;
    int reserve;
}AUDIO_EXCEPTION_QUEUE_TYPE;

typedef enum
{
    PWR_BACK_INDEX_ID = 0,
    PWR_BACK_CLK_SELECTOR,
    PWR_SYSTEM_POWER_STATE = 40,
    PWR_SYSTEM_RESET_TYPE_INDICATOR = 41,
    PWR_BACK_MAX = 42
} PWR_BACK_INDEX;

typedef enum{
    BT_POWER_ON = 0,
    BT_PAIRING,
    BT_LINKED,
    BT_POWER_OFF
}xBTHandleModuleStatus;

typedef enum{
    BT_TIME_INIT = 0,
    BT_TIME_AUTO_PAIRING,
    BT_TIME_ENERGY_SAVING
}xBTTimeStatus;

#endif /*__API_TYPEDEF_H__*/
