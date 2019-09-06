#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "command.h"
#include "device_config.h"

#include "usb_host_device.h"
#include "AudioSystemHandler.h"
#include "PowerHandler.h"
#include "BTHandler.h"
#include "RFHandler.h"
#include "IRCmdHandler.h"
#include "ButtonCmdDispatcher.h"

#include "ChannelCheckManager.h"
#include "StorageDeviceManager.h"
#include "FactoryCommandHandler.h"
#include "UIDeviceManager.h"
#include "USBMusicManager.h"

#include "HMI_Service.h"

#if ( configSII_DEV_953x_PORTING == 1 )
#include "HdmiDeviceManager.h"
#include "GPIOMiddleLevel.h"
#endif
//_________________________________________________________________________________________
#define HMI_QUEUE_LENGTH 20
#define HMI_TIME_TICK TASK_MSEC2TICKS(1)
#define HMI_APC_TIME_TICK TASK_MSEC2TICKS(1000)
#define HMI_CMD_TIME_UP_MSEC TASK_MSEC2TICKS(15000) 
#define HMI_CMD_TIME_PRESS_MSEC TASK_MSEC2TICKS(215) 
#define HMI_CMD_TIME_LONGPRESS_MSEC TASK_MSEC2TICKS(2000) 
#define HMI_APD_TIMEOUT (60*15)
#define HMI_APD_TIMEOUT_DIGITAL 730
#define EVENT_FIRST_PRESSED_OR_TIMEOUT TRUE

#define CheckSignalAvaiableTime   1
#define CheckSignalUnAvaiableTime 1
#define HMI_AID_TIME_TICK TASK_MSEC2TICKS(500)
#define AID_TIMEOUT (60*15)*2
#define AnalogThrehold  6 /* check signal after 3 sec */ 
#define DigitalThrehold 6 /* after 3 sec */
#define AutoDetectionLastSource AUDIO_SOURCE_OPTICAL

#define PRESS_KEY_DURATION TASK_MSEC2TICKS(200) //100 MSEC
#define DemoModeBounceTime TASK_MSEC2TICKS(6000)

#if ( configSII_DEV_953x_PORTING == 1 )
#define HDMI_PLUGIN_DETECTION_IO    __I_HDMI_WAKE
//#define HDMI_RST_PIN __O_RST_HDMI_SIL953x
#endif

//_________________________________________________________________________________________
typedef struct DEMO1_3_TIMEOUT_DETECT_PARAMETERS
{
    xTaskHandle DetectHandle;
    TaskHandleState TaskState;
} xDemo1_3_TimeOutDetect;

typedef struct HMI_SERVICE_PARAMETERS
{
    xTaskHandle userTaskHandle;
    xTaskHandle apcTaskHandle;
    xTaskHandle aidTaskHandle;
    TaskHandleState userState;
    xQueueParameters serviceQueue;
    xOS_TaskErrIndicator xOS_ErrId;
} xHMISrvClusion;

typedef struct HMI_STANDBY_CTRL_PARAMETERS
{
    xTaskHandle StandbyCtrlTaskHandle;
} xHMIStandbyCtrl;

typedef struct _MASTER_GAIN_LIMIT
{
    uint8 min_gain;
    uint8 max_gain;
}xHMISrvMasterGainLimit;

typedef enum {
    EVENT_ASSORT_NO_NEED_FIRST_CONFIRM,
    EVENT_ASSORT_SAME,
    EVENT_ASSORT_DIFFERENT,
} xEventTypeAssort;

//_________________________________________________________________________________________
extern const uint8 EEpromDefaultParams[];
extern xBTHandleState BTHandlerState;

static xHMISystemParams mDefFactoryParams = { 
    SYS_EVENT_NULL,
    MODE_FACTORY,    
    DEFAULT_APD,
    FACTORY_INPUT_SRC,
    DEFAULT_LAST_INPUT_SRC,
    DEFAULT_MUTE_STATE, /*mute*/
    FACTORY_MASTER_GAIN, /*master gain*/
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

#if ( configSII_DEV_953x_PORTING == 1 )
    DEFAULT_AID,
    DEFAULT_CEC_SAC,
    DEFAULT_CEC_ARC,
    DEFAULT_CEC_ARC_Tx
#else
    DEFAULT_AID
#endif
};

static xHMISystemParams mSystemParams = { 
    SYS_EVENT_NULL,
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

#if ( configSII_DEV_953x_PORTING == 1 )
    DEFAULT_AID,
    DEFAULT_CEC_SAC,
    DEFAULT_CEC_ARC,
    DEFAULT_CEC_ARC_Tx
#else
    DEFAULT_AID
#endif
};
static bool bIsSystemReset = FALSE;
static const xHMISystemParams *pSystemParams = &mSystemParams;
static xHMISrvMasterGainLimit master_gain_limit = { MASTER_GAIN_MIN, MASTER_GAIN_MAX };
static xDemo1_3_TimeOutDetect mDemo1_3_TimeOutDetect;
static xHMISrvClusion mHMISrvClusion;
static xHMIStandbyCtrl mHMIStandbyCtrl;
static const xHMISrvClusion *pHMISrvClusion =&mHMISrvClusion;
#if 0 // David, fix warning.
static xBTHandleCommand mBTHandleCommand;
#endif

static xHMISrvEventParams aid_parms = {xHMI_EVENT_AID, USER_EVENT_UNDEFINED };
static uint32 mProcessCnt;
static uint8 mSignalAvaiableCnt;
static uint8 mSignalUnavaiableCnt;
static bool LastUserParmsAutoPowerDown = TRUE;
static bool UserParmsAutoPowerDown = TRUE;
static uint16 AID_timeout = AID_TIMEOUT;

static uint8 FAC_LED_PWR_ON = 0;
//static bool IS_FAC_EXIST = FALSE;

static portTickType input_key_time = 0;

#define configMonitorHeapSize 1
#if ( configMonitorHeapSize == 1 )        
size_t free_heap_size = 0;
#endif 
static bool Demo1TimeOut = FALSE;
static bool Demo2TimeOut = FALSE;
static bool Demo3TimeOut = FALSE;
static uint8 Demo1_3_TimeOutCnt = 0;
static bool Demo1_3_TimeOut = FALSE;

#if ( configSII_DEV_953x_PORTING == 1 )
static bool IsFirstSystemStartup = FALSE;

#if (configSII_DEV_953x_TX_HPD_CTRL == 1)  // David, fix warning.
static bool Is_mute_HPD = FALSE;
#endif
#endif

//_________________________________________________________________________________________
static bool HMI_Service_EventSender( void *params );

static void HMI_Service_setSystemParams( const xHMISystemParams *params );

static void HMI_Service_rstSystemParams( void );

static void HMI_FMD_CommandHandle( const xHMISystemParams *params );

static xEventTypeAssort HMI_Service_Event_Assort( xHMIUserEvents cur_events, xHMIUserEvents last_events );

static void HMI_AutoInputDetect_CreateTask(void);

static void HMI_AutoInputDetect_TaskCtl(bool val);

static void HMI_AutoInputDetect_task( void *pvParameters );

static xHMISystemParams HMI_Service_getSystemParams( void );

static uint16 HMI_Service_getAPDTimeout( AUDIO_SOURCE  CurrSrc );

static uint16 HMI_Service_getMuteTimeout( AUDIO_SOURCE  CurrSrc );

#if ( configSII_DEV_953x_PORTING == 1 )
static bool HMI_Service_IsPowerKeyAvailable( bool OnOff, bool IsHDMI);
static void HMI_Service_HDMI_StandbyCtrl();
#endif
static void HMI_DemoMode1_3_TimeOutDetectCreateTask(void);
static void HMI_DemoMode1_3_TimeOutDetect_TaskCtrl( bool val );
static void HMI_DemoMode1_3_TimeOutDetectTask( void *pvParameters );

static uint8 HMI_Service_InstrGetQueueNumber( void );
//_________________________________________________________________________________________

extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
extern USB_MUSIC_MANAGE_OBJECT *pUSB_MMP_ObjCtrl;
extern BT_HANDLE_OBJECT *pBTHandle_ObjCtrl;
extern RF_HANDLE_OBJECT *pRFHandle_ObjCtrl;
extern BTN_CONF_OBJECT *pBTN_ObjCtrl;
extern CCK_MANAGE_OBJECT *pCCKManage_ObjCtrl;
extern AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl;
extern FACTORY_CMD_HANDLE_OBJECT *pFMD_ObjCtrl;
extern UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl;
extern IR_CMD_PROGRAM_OBJECT *pIR_PrgRemote_ObjCtrl;
extern STORAGE_DEVICE_MANAGER_OBJECT *pSDM_ObjCtrl;

#if ( configSII_DEV_953x_PORTING == 1 )
extern HDMI_DEVICE_MANAGER_OBJECT *pHDMI_DM_ObjCtrl;
#endif

const HMI_SERVICE_OBJECT HMI_SRV_OBJ = 
{
    HMI_Service_EventSender,
    HMI_Service_setSystemParams,
    HMI_Service_rstSystemParams,
    HMI_Service_getSystemParams
};
const HMI_SERVICE_OBJECT *pHS_ObjCtrl = &HMI_SRV_OBJ;

const HMI_FACTORY_OBJECT HMI_FACTORY_OBJ = 
{
    HMI_Service_setSystemParams,
    HMI_Service_rstSystemParams,
    HMI_FMD_CommandHandle
};
const HMI_FACTORY_OBJECT *pHFS_ObjCtrl = &HMI_FACTORY_OBJ;

//_________________________________________________________________________________________

static void _PWR_ENABLE_CTRL(bool b_pwr_on)
{
    if (pPowerHandle_ObjCtrl == NULL 
        || pBTHandle_ObjCtrl == NULL
        || pASH_ObjCtrl == NULL
        || pUDM_ObjCtrl == NULL
        || pUSB_MMP_ObjCtrl == NULL
        )
    {
        return;
    }
    

    if (!b_pwr_on)
    {
        if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
        {
            mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#if (0)//( configSII_DEV_953x_PORTING == 1 )
            while ( !pASH_ObjCtrl->isHDMIPowerOff() )
            {
                TRACE_DEBUG((0, "Reseting ...HDMI still work, please wait ..."));
            }
#endif

            pPowerHandle_ObjCtrl->power_toggle();
        }
    }
    else
    {
        if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
        {
            pPowerHandle_ObjCtrl->power_toggle();
            
            mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
        }
    }
}

static void _RST_DEF_SETTING()
{
    if (pBTN_ObjCtrl == NULL 
       || pASH_ObjCtrl == NULL
       )
    {
        return;
    }

    
#if 0 //using new BT mechanism
    mBTHandleCommand = BT_HANDLE_RESET_BT;
    pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
#endif

    mSystemParams.op_mode = MODE_USER;
    HMI_Service_rstSystemParams();
    
    /* Store parameters to EEPROM*/
    mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
    pSDM_ObjCtrl->SendEvent( &mSystemParams); 
    
    pBTN_ObjCtrl->register_user_cmd();
}
//_________________________________________________________________________________________
static void HMI_Service_setSystemParams( const xHMISystemParams *params ) 
{
    if ( params != NULL )
    {
        mSystemParams = *params;
    }
}

static void HMI_Service_rstSystemParams( void )
{
    master_gain_limit.max_gain = MASTER_GAIN_MAX;
    master_gain_limit.min_gain = MASTER_GAIN_MIN;
    
    mSystemParams.apd = DEFAULT_APD;
    mSystemParams.input_src = DEFAULT_INPUT_SRC;
    mSystemParams.master_gain = DEFAULT_MASTER_GAIN;
    mSystemParams.mute = DEFAULT_MUTE_STATE;
    mSystemParams.op_mode = DEFAULT_OP_MODE;
    mSystemParams.bass_gain =  DEFAULT_BASS_GAIN;
    mSystemParams.treble_gain = DEFAULT_TREBLE_GAIN;
    mSystemParams.sub_gain = DEFAULT_SUB_GAIN;
    mSystemParams.center_gain = DEFAULT_CENTER_GAIN;
    mSystemParams.LsRs_gain = DEFAULT_REAR_GAIN;
    mSystemParams.balance = DEFAULT_BALANCE;
    mSystemParams.srs_truvol = DEFAULT_SRS_TRUVOL;
    mSystemParams.srs_tshd = DEFAULT_SRS_TSHD;
    mSystemParams.night_mode = DEFAULT_NIGHT_MODE;
    mSystemParams.av_delay = DEFAULT_AV_DELAY;
    mSystemParams.vizio_rmt = DEFAULT_VIZIO_RMT;

    mSystemParams.auto_input_detect = DEFAULT_AID;
#if ( configSII_DEV_953x_PORTING == 1 )
    mSystemParams.cec_sac= DEFAULT_CEC_SAC;
    mSystemParams.cec_arc= DEFAULT_CEC_ARC;
    mSystemParams.cec_arc_tx= DEFAULT_CEC_ARC_Tx;
#endif
}

static void HMI_Service_setDemo1n3_SysParams( xHMIOperationMode op_mode )
{
    master_gain_limit.max_gain = DEMO_1_MASTER_GAIN_MAX;
    master_gain_limit.min_gain = DEMO_1_MASTER_GAIN_MIN;
    
    mSystemParams.apd = DEMO_1_APD;
    mSystemParams.master_gain = DEMO_1_MASTER_GAIN;
    mSystemParams.mute = DEMO_1_MUTE_STATE;
    mSystemParams.bass_gain =  DEMO_1_BASS_GAIN;
    mSystemParams.treble_gain = DEMO_1_TREBLE_GAIN;
    mSystemParams.sub_gain = DEMO_1_SUB_GAIN;
    mSystemParams.center_gain = DEMO_1_CENTER_GAIN;
    mSystemParams.LsRs_gain = DEMO_1_REAR_GAIN;
    mSystemParams.balance = DEMO_1_BALANCE;
    mSystemParams.srs_truvol = DEMO_1_SRS_TRUVOL;
    mSystemParams.srs_tshd = DEMO_1_SRS_TSHD;
    mSystemParams.night_mode = DEMO_1_NIGHT_MODE;
    mSystemParams.av_delay = DEMO_1_AV_DELAY;
    mSystemParams.vizio_rmt = DEMO_1_VIZIO_RMT;

    mSystemParams.auto_input_detect = DEMO_1_AID;
#if ( configSII_DEV_953x_PORTING == 1 )
    mSystemParams.cec_sac= DEFAULT_CEC_SAC;
    mSystemParams.cec_arc= DEFAULT_CEC_ARC;
    mSystemParams.cec_arc_tx= DEFAULT_CEC_ARC_Tx;
#endif
    
    if ( op_mode == MODE_DEMO_1 )
    {
        mSystemParams.input_src = DEMO_1_INPUT_SRC;
    }
       else if ( op_mode == MODE_DEMO_2 )
    {
        mSystemParams.input_src = DEMO_2_INPUT_SRC;
    }
    else if ( op_mode == MODE_DEMO_3 )
    {
        mSystemParams.input_src = DEMO_3_INPUT_SRC;
    }
}

static void HMI_Service_setDemo2_Timeout(void)
{
    /*TODO: Why set here for demo mode 2*/
    HMI_Service_setDemo1n3_SysParams(MODE_DEMO_2); 
    mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event */
    //pASH_ObjCtrl->SendEvent( &mSystemParams );    //Angus remove for issue demo mode 2 output sound abnomal.
    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_TIMEOUT_DEMO);
    Demo2TimeOut = TRUE;
}

static void HMI_Service_setDemo1_3_Timeout(bool signal)
{
    if(signal)
    {
        if(mSystemParams.op_mode == MODE_DEMO_1)
        {
            if(Demo1TimeOut)
            {
                HMI_Service_setDemo1n3_SysParams(MODE_DEMO_1);
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_EXIT_TIMEOUT_DEMO);
                Demo1TimeOut = FALSE;
            }
        }
        else if(mSystemParams.op_mode == MODE_DEMO_3)
        {
            if(Demo3TimeOut)
            {
                HMI_Service_setDemo1n3_SysParams(MODE_DEMO_3);
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_EXIT_TIMEOUT_DEMO);            
                Demo3TimeOut = FALSE;
            }

        }
    }
    else
    {
        if(mSystemParams.op_mode == MODE_DEMO_1)
        {
            if(Demo1TimeOut == FALSE)
            {
                /*TODO: Why set here for demo mode 1*/
                HMI_Service_setDemo1n3_SysParams(MODE_DEMO_1); 
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_TIMEOUT_DEMO);            
                Demo1TimeOut = TRUE;
            }
        }
        else if(mSystemParams.op_mode == MODE_DEMO_3)
        {
            if(Demo3TimeOut == FALSE)
            {
                /*TODO: Why set here for demo mode 3*/
                HMI_Service_setDemo1n3_SysParams(MODE_DEMO_3); 
                #if 0   //It will cause detect siganl error , Angus remove
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                #endif
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_TIMEOUT_DEMO);
                Demo3TimeOut = TRUE;
            }

        }

    }
}


static bool HMI_Service_EventSender( void *params ) 
{
    xHMISrvEventParams* pParams = ( xHMISrvEventParams *)params;

#if 0//( configHDMI_REPEATER == 1 ) /*smith marks*/
 portTickType duration;
    static portTickType xTimeTick = 0;
#endif

    if ( pParams == NULL )
    {
        return FALSE;
    }

    if (mHMISrvClusion.serviceQueue.xQueue == NULL )
    {
        TRACE_ERROR((0, "HMI Service sender error !! "));
        mHMISrvClusion.xOS_ErrId = xOS_TASK_QUEUE_IS_NULL;
        return FALSE;
    }
#if 0//( configHDMI_REPEATER == 1 ) /*smith marks*/
/*Smith improves: to avoid system crash ratio*/
    if( ( pParams->event_id == xHMI_EVENT_XRS351_IR ) || ( pParams->event_id == xHMI_EVENT_BUTTON ))
    {
        switch( pParams->event)
        {
            case USER_EVENT_SEL_AUX1:
            case USER_EVENT_SEL_AUX2:
            case USER_EVENT_SEL_COAXIAL:
            case USER_EVENT_SEL_OPTICAL:
            case USER_EVENT_SEL_USB_PLAYBACK:
            case USER_EVENT_SEL_BT:
            case USER_EVENT_SEL_SRC:
#if ( configSII_DEV_953x_PORTING == 1 )
            case USER_EVENT_SEL_HDMI_IN:
            case USER_EVENT_SEL_HDMI_ARC:
#endif
            {
                duration = ( xTaskGetTickCount( ) - xTimeTick );
                xTimeTick = xTaskGetTickCount( );

                if ( duration < 500 )
                {
                    return FALSE;
                }
            }
                break;

            default: {} break;
        }

    }

#endif
    if ( xQueueSend( mHMISrvClusion.serviceQueue.xQueue, pParams, mHMISrvClusion.serviceQueue.xBlockTime) != pdPASS )
    {
        TRACE_ERROR((0, " HMI_InstrSender send queue is failure "));
        mHMISrvClusion.xOS_ErrId = xOS_TASK_QUEUE_SET_FAIL;
        return FALSE;
    }
    return TRUE;

}

static bool HMI_Service_EventReceiver( xHMISrvEventParams *pEventParams ) 
{
    if ( pEventParams == NULL )
    {
        TRACE_ERROR((0, " HMI service instruction recevier error !! "));
        return FALSE;
    }
    
    if ( mHMISrvClusion.serviceQueue.xQueue == NULL )
    {
        mHMISrvClusion.xOS_ErrId = xOS_TASK_QUEUE_IS_NULL;
        return FALSE;
    }

    if ( xQueueReceive( mHMISrvClusion.serviceQueue.xQueue, pEventParams, mHMISrvClusion.serviceQueue.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR((0, " HMI_InstrSender receiver queue is failure "));
        mHMISrvClusion.xOS_ErrId = xOS_TASK_QUEUE_GET_FAIL;
        return FALSE;
    }

    return TRUE;
}

#if 0 // David, fix warning.
static void HMI_Service_DisplayCurrentParams( xHMIUserEvents event )
{
    if( ( pPowerHandle_ObjCtrl->get_power_state()!= POWER_ON ) && ( event != USER_EVENT_POWER ) )
        return;

    switch( event )
    {
        case USER_EVENT_VOL_UP:
        case USER_EVENT_VOL_DN:
        {
            /*TODO: Send to UI */
            TRACE_DEBUG((0, "Current master gain = %d", mSystemParams.master_gain ));
        }
            break;

        case USER_EVENT_MUTE:
        {
            TRACE_DEBUG((0, "Current mute state = %d", mSystemParams.mute));
        }
            break;

        default:
        {
            TRACE_DEBUG((0, "HMI_Service_reportCommand "));
        }
            break;
    }
}
#endif

static void SendCecReportAudioStatus(xHMISystemParams sysParm)
{
    sysParm.sys_event = SYS_EVENT_SAC_REPORT_AUDIO_STATUS;
    pHDMI_DM_ObjCtrl->SendEvent( &sysParm );
}

static void SyncSacVolume(xHMISystemParams sysParm)
{
    sysParm.sys_event = SYS_EVENT_VOLUME_SET;
    pHDMI_DM_ObjCtrl->SendEvent( &sysParm );
}


static void HMI_Service_sendCommand( xHMISrvEventParams event )
{
    xHMISrvEventParams EventParams;
        //TRACE_ERROR((0, "HMI_Service_sendCommand %d ", event.event ));
    switch( event.event)
    {
#if (0)
        case USER_EVENT_CEC_POWER_ON_HDMI_IN:
        case USER_EVENT_CEC_POWER_ON_HDMI_ARC:
	case USER_EVENT_CEC_POWER_ON:
	    if ( (mSystemParams.op_mode == MODE_USER) ||(mSystemParams.op_mode == MODE_FACTORY) )
	    {
                if ( !HMI_Service_IsPowerKeyAvailable(TRUE, FALSE) )    //if system is on, swithc input source directly
                {
                    //TRACE_DEBUG((0, "POWER Key NOT available"));
                    xHMISrvEventParams srv_parms = {xHMI_EVENT_CEC, USER_EVENT_UNDEFINED, 0 };
                
                    if ( event.event == USER_EVENT_CEC_POWER_ON_HDMI_IN )
                    {
                        //TRACE_DEBUG((0, "POWER Key NOT available: USER_EVENT_CEC_POWER_ON_HDMI_IN"));
                        srv_parms.event = USER_EVENT_CEC_SEL_HDMI_IN;
                        HMI_Service_EventSender( &srv_parms );
                    }
                    else if ( event.event == USER_EVENT_CEC_POWER_ON_HDMI_ARC )
                    {
                        //TRACE_DEBUG((0, "POWER Key NOT available: USER_EVENT_CEC_POWER_ON_HDMI_ARC"));
                        srv_parms.event = USER_EVENT_CEC_SEL_HDMI_ARC;
                        HMI_Service_EventSender( &srv_parms );
                    }
					
                    break;
                }
                else    // system is off, power system on under currect HDMI source
                {
                    //TRACE_DEBUG((0, "POWER Key AVAILABLE"));
                    //xHMISrvEventParams srv_parms = {xHMI_EVENT_CEC, USER_EVENT_UNDEFINED, 0 }; // David, fix warning.
                
                    if ( event.event == USER_EVENT_CEC_POWER_ON_HDMI_IN )
                    {
                        //TRACE_DEBUG((0, "POWER Key AVAILABLE: USER_EVENT_CEC_POWER_ON_HDMI_IN"));
                        //srv_parms.event = USER_EVENT_CEC_SEL_HDMI_IN;
                        //HMI_Service_EventSender( &srv_parms );
                        mSystemParams.input_src = AUDIO_SOURCE_HDMI_IN;
                    }
                    else if ( event.event == USER_EVENT_CEC_POWER_ON_HDMI_ARC )
                    {
                        //TRACE_DEBUG((0, "POWER Key AVAILABLE: USER_EVENT_CEC_POWER_ON_HDMI_ARC"));
                        //srv_parms.event = USER_EVENT_CEC_SEL_HDMI_ARC;
                        //HMI_Service_EventSender( &srv_parms );
                        mSystemParams.input_src = AUDIO_SOURCE_HDMI_ARC;
                    }
        
                    event.event = USER_EVENT_POWER;
                }
	    }
#endif
        case USER_EVENT_SAC_WAKE_UP:
        {
            TRACE_DEBUG((0, "USER_EVENT_SAC_WAKE_UP"));
            mSystemParams.last_input_src = mSystemParams.input_src;
            mSystemParams.input_src = AUDIO_SOURCE_HDMI_ARC;
        }
        case USER_EVENT_POWER:
        {    
            //TRACE_DEBUG((0, "USER_EVENT_POWER"));

            if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
            {
                if (pSystemParams->op_mode == MODE_FACTORY)
                {
                    Debug_initialize( );
                    TRACE_DEBUG((0, "USER_EVENT_POWER turn on User mode "));
                    mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                    mSystemParams.op_mode = MODE_USER;
                    pSDM_ObjCtrl->SendEvent( &mSystemParams );
                    //vTaskDelay( 100 );//need to check
                }                               
                mSystemParams.night_mode = FALSE;/*UI 4.2 If Night Mode is changed to "On", at the next power on, Night mode returns to the "Off" setting.*/
                mSystemParams.mute = SOUND_DEMUTE; /*UI 4.2 When sound bar powers down in "mute" mode, at power up the previous unmuted volume level resume (not muted).  Sound bar does not remain in mute mode after a power cycle to avoid issues with universal remotes.*/
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;

                //call by function
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                //free run to active
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                if (pSystemParams->auto_input_detect == TRUE)
                {    
                    HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
                }

                //Q message to active
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
                
                //BTHandlerState = BT_MS_IDLE; /*Smith removes*/

                //Clear All message from HMI Task
                while(HMI_Service_InstrGetQueueNumber() > 0)
                {
                    HMI_Service_EventReceiver( &EventParams );
                }
#if (0) //( configSII_DEV_953x_PORTING == 1 )
                while ( !pASH_ObjCtrl->isHDMIPowerOff() )
                {
                    TRACE_DEBUG((0, "HDMI still work, please wait ..."));
                }
#endif
                pPowerHandle_ObjCtrl->power_toggle();
            }
            else if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
            {
                //TRACE_DEBUG((0, "POWER ON SYSTEM"));
                pPowerHandle_ObjCtrl->power_toggle();
                
                if (pSystemParams->op_mode == MODE_FACTORY)
                {
                    Debug_unregister( (DEBUG_ERROR| DEBUG_DEBUG | DEBUG_INFO) );
                    HMI_Service_setSystemParams(&mDefFactoryParams);
                }                               
                else if (pSystemParams->op_mode ==MODE_DEMO_1||pSystemParams->op_mode ==MODE_DEMO_2||pSystemParams->op_mode ==MODE_DEMO_3)
                {
                    HMI_Service_setDemo1n3_SysParams(pSystemParams->op_mode);
                }
                
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                if ( pSystemParams->auto_input_detect == TRUE )
                {    
                    HMI_AutoInputDetect_TaskCtl(BTASK_RESUME);
                }

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);

            }
        }
            break;

        case USER_EVENT_FORCE_POWER_UP:
        {
#if ( configSYS_FORCE_POWERUP == 1 ) 
            pPowerHandle_ObjCtrl->go_back_power_up( );
        
            if (pSystemParams->op_mode == MODE_FACTORY)
            {
                Debug_unregister( (DEBUG_ERROR| DEBUG_DEBUG | DEBUG_INFO) );
                HMI_Service_setSystemParams(&mDefFactoryParams);
            }                               
            else if (pSystemParams->op_mode ==MODE_DEMO_1||pSystemParams->op_mode ==MODE_DEMO_2||pSystemParams->op_mode ==MODE_DEMO_3)
            {
                HMI_Service_setDemo1n3_SysParams(pSystemParams->op_mode);
            }

            mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            if ( pSystemParams->auto_input_detect == TRUE )
            {    
                HMI_AutoInputDetect_TaskCtl(BTASK_RESUME);
            }
#endif            
        }
            break;

        case USER_EVENT_APD_TOOGLE:
        {
            mSystemParams.sys_event = SYS_EVENT_APD_SET;

            if ( pSystemParams->apd == FALSE )
            {
                mSystemParams.apd = TRUE;
            }
            else
            {
                mSystemParams.apd = FALSE;
            }
            
            pUDM_ObjCtrl->SendEvent( &mSystemParams);
            
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
            pSDM_ObjCtrl->SendEvent( &mSystemParams); 
        }
            break;

        case USER_EVENT_APD_ON:
        {
            if ( pSystemParams->apd == FALSE )
            {
                mSystemParams.sys_event = SYS_EVENT_APD_SET;
                mSystemParams.apd = TRUE;
                pUDM_ObjCtrl->SendEvent( &mSystemParams);
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        }
            break;

        case USER_EVENT_APD_OFF:
        {
            if ( pSystemParams->apd == TRUE )
            {
                mSystemParams.sys_event = SYS_EVENT_APD_SET;
                mSystemParams.apd = FALSE;
                pUDM_ObjCtrl->SendEvent( &mSystemParams);
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        }
            break;

        case USER_EVENT_SEL_LAST_SRC:
        {
            AUDIO_SOURCE TemoCurrSrc;
    
            TemoCurrSrc = mSystemParams.input_src;
            mSystemParams.input_src = mSystemParams.last_input_src;
            mSystemParams.last_input_src = TemoCurrSrc;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
        }
            break;
        
        case USER_EVENT_START_AUTO_SEL_SRC:
        {
	    if ( mSystemParams.auto_input_detect == FALSE )
	    {
                mSystemParams.auto_input_detect = TRUE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
                HMI_AutoInputDetect_TaskCtl(BTASK_RESUME);
	    }
	    else
	    {
                mSystemParams.sys_event = SYS_EVENT_AUTO_SEL_SRC;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
	    }
        }
            break;
    
        case USER_EVENT_SEL_SRC:
        case USER_EVENT_AUTO_SEL_SRC: /*For auto detection*/
        {
            if (event.event==USER_EVENT_SEL_SRC)
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
                {
                    mSystemParams.auto_input_detect = FALSE;
                }  
                
                if ( mSystemParams.input_src < AUDIO_SOURCE_MAX )
                {
                    mSystemParams.input_src ++;
                }

                if ( mSystemParams.input_src >= AUDIO_SOURCE_MAX )
                {
                    mSystemParams.input_src = AUDIO_SOURCE_AUX1;
                }
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                if ( mSystemParams.input_src < AUTO_AUDIO_SOURCE_MAX )
                {
                    mSystemParams.input_src ++;
                }

                if ( mSystemParams.input_src >= AUTO_AUDIO_SOURCE_MAX )
                {
                    mSystemParams.input_src = AUDIO_SOURCE_AUX1;
                }
                mSystemParams.sys_event = SYS_EVENT_AUTO_SEL_SRC;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            
             /*set audio system event*/
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC;
            pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
           
        }
            break;

        case USER_EVENT_SEL_AUX1:
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }
        
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_AUX1 )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_AUX1;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
        }
            break;

        case USER_EVENT_SEL_AUX2:
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }        
            
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_AUX2 )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_AUX2;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
        }
            break;

        case USER_EVENT_SEL_COAXIAL:
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }
            
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_COAXIAL )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_COAXIAL;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
        }
            break;    
            
        case USER_EVENT_SEL_OPTICAL:
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }
        
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_OPTICAL )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_OPTICAL;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
        }
            break;

        case USER_EVENT_SEL_USB_PLAYBACK:
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }
        
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_USB_PLAYBACK )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_USB_PLAYBACK;

                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
        }
            break;

        case USER_EVENT_SEL_BT:
        {     
            //We need to active the LED scanning after confirm input source BT.
            if( ( pBTHandle_ObjCtrl->get_status() == BT_PAIRING ) && ( mSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH ) )
            {
                TRACE_DEBUG((0, " Reject confirm input  ...  "));
            }
            else
            {
                if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
                {
                    mSystemParams.auto_input_detect = FALSE;
                }
                
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                
                if ( mSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH )
                {
                    pUDM_ObjCtrl->SendEvent( &mSystemParams );
                    pBTHandle_ObjCtrl->SendEvent( &mSystemParams ); //Fix no show pairing issue.
                }
                else
                {
                    mSystemParams.last_input_src = mSystemParams.input_src;
                    mSystemParams.input_src = AUDIO_SOURCE_BLUETOOTH;
                    pSDM_ObjCtrl->SendEvent( &mSystemParams );
                    pUDM_ObjCtrl->SendEvent( &mSystemParams );
                    
                    pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                    
                    pASH_ObjCtrl->SendEvent( &mSystemParams );
                    pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
                }
            }
        }
            break;

#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_SEL_HDMI_IN: /*for hdmi debug*/
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }
        
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_HDMI_IN )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_HDMI_IN;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
        }
            break;

        case USER_EVENT_SEL_HDMI_ARC: /*Smith Implemented: Only for test ARC*/
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }

            
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_HDMI_ARC )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

				/*fix bug: SB always output HDMI5 source sound, DC*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_HDMI_ARC;        
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );

                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

				pASH_ObjCtrl->SendEvent( &mSystemParams );
				pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
        }
            break;
#endif

        case USER_EVENT_CCK:
        {            
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
            pCCKManage_ObjCtrl->start(&mSystemParams);
        }
            break;
        
        case USER_EVENT_VOL_UP:
        {             
#if ( configSII_DEV_953x_PORTING == 1 )
	    if ( (event.event_id == xHMI_EVENT_PROGRAM_IR) && (mSystemParams.cec_sac == TRUE) )
	    {
	        break;
	    }
#endif		
            if ( pSystemParams->master_gain < master_gain_limit.max_gain )
            {
                mSystemParams.master_gain ++;
            }

            if( pSystemParams->mute == SOUND_MUTE )
            {
                mSystemParams.mute = SOUND_DEMUTE;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
            }

            mSystemParams.sys_event = SYS_EVENT_VOLUME_SET; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );   
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);//Do nothing but sync vol information to USBSystemParams
            SendCecReportAudioStatus(mSystemParams);
        }
            break;

        case USER_EVENT_VOL_DN:
        {         
#if ( configSII_DEV_953x_PORTING == 1 )
	    if ( (event.event_id == xHMI_EVENT_PROGRAM_IR) && (mSystemParams.cec_sac == TRUE) )
	    {
	        break;
	    }
#endif		
            if ( pSystemParams->master_gain > master_gain_limit.min_gain )
            {
                if (pASH_ObjCtrl->getRampStatus()== TRUE)
                {
                    mSystemParams.master_gain = pASH_ObjCtrl->getRampVolvalue();
                }
                else
                {
                    mSystemParams.master_gain --;
                }

                mSystemParams.sys_event = SYS_EVENT_VOLUME_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);//Do nothing but sync vol information to USBSystemParams
                SendCecReportAudioStatus(mSystemParams);
            }
        }
            break;

        case USER_EVENT_BASS_UP:
        {
            if ( pSystemParams->bass_gain <  BASS_MAX)
            {
                mSystemParams.bass_gain++;
            }
            mSystemParams.sys_event = SYS_EVENT_BASS_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;
            
        case USER_EVENT_BASS_DN:
        {
            if ( pSystemParams->bass_gain > BASS_MIN )
            {
                mSystemParams.bass_gain--;
            }
            mSystemParams.sys_event = SYS_EVENT_BASS_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;    

        case USER_EVENT_TREBLE_UP:
        {
            if ( pSystemParams->treble_gain <  TREBLE_MAX)
            {
                mSystemParams.treble_gain++;
            }
            mSystemParams.sys_event = SYS_EVENT_TREBLE_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;
            
        case USER_EVENT_TREBLE_DN:
        {
            if ( pSystemParams->treble_gain > TREBLE_MIN )
            {
                mSystemParams.treble_gain--;
            }
            mSystemParams.sys_event = SYS_EVENT_TREBLE_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;    

        case USER_EVENT_SUB_UP:
        {
            if ( pSystemParams->sub_gain < SUB_MAX )
            {
                mSystemParams.sub_gain++;
            }
            mSystemParams.sys_event = SYS_EVENT_SUB_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_SUB_DN:
        {
            if ( pSystemParams->sub_gain > SUB_MIN )
            {
                mSystemParams.sub_gain--;
            }
            mSystemParams.sys_event = SYS_EVENT_SUB_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
        break;

        case USER_EVENT_CENTER_UP:
        {
            if ( pSystemParams->center_gain < CENTER_MAX )
            {
                mSystemParams.center_gain++;
            }
            mSystemParams.sys_event = SYS_EVENT_CENTER_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
        break;

        case USER_EVENT_CENTER_DN:
        {
            if ( pSystemParams->center_gain > CENTER_MIN )
            {
                mSystemParams.center_gain--;
            }
            mSystemParams.sys_event = SYS_EVENT_CENTER_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_REAR_UP:
        {
            if ( pSystemParams->LsRs_gain < REAR_MAX )
            {
                mSystemParams.LsRs_gain++;
            }
            mSystemParams.sys_event = SYS_EVENT_LSRS_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_REAR_DN:
        {
            if ( pSystemParams->LsRs_gain > REAR_MIN )
            {
                mSystemParams.LsRs_gain--;
            }
            mSystemParams.sys_event = SYS_EVENT_LSRS_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_BALANCE_UP:
        {
            if ( pSystemParams->balance < BALANCE_MAX )
            {
                mSystemParams.balance++;
            }
            mSystemParams.sys_event = SYS_EVENT_BALANCE_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_BALANCE_DN:
        {
            if ( pSystemParams->balance > BALANCE_MIN )
            {
                mSystemParams.balance--;
            }
            mSystemParams.sys_event = SYS_EVENT_BALANCE_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_SRS_TRUVOL_ON:
        {
            if ( pSystemParams->srs_truvol== TRUE )
            {
                return;
            }
            else
            {
                mSystemParams.srs_truvol = TRUE;
            }

            mSystemParams.sys_event = SYS_EVENT_SRS_TRUVOL_TSHD_SET;
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            mSystemParams.sys_event = SYS_EVENT_SRS_TRUVOL_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_SRS_TRUVOL_OFF:
        {
            if ( pSystemParams->srs_truvol== FALSE )
            {
                return;
            }
            else
            {
                mSystemParams.srs_truvol = FALSE;
            }

            mSystemParams.sys_event = SYS_EVENT_SRS_TRUVOL_TSHD_SET;
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            mSystemParams.sys_event = SYS_EVENT_SRS_TRUVOL_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_SRS_TSHD_ON:
        {
            if ( pSystemParams->srs_tshd == TRUE )
            {
                return;
            }
            else
            {
                mSystemParams.srs_tshd = TRUE;
            }
            mSystemParams.sys_event = SYS_EVENT_SRS_TRUVOL_TSHD_SET;
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            mSystemParams.sys_event = SYS_EVENT_SRS_TSHD_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_SRS_TSHD_OFF:
        {
            if ( pSystemParams->srs_tshd == FALSE )
            {
                return;
            }
            else
            {
                mSystemParams.srs_tshd = FALSE;
            }
            mSystemParams.sys_event = SYS_EVENT_SRS_TRUVOL_TSHD_SET;
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            mSystemParams.sys_event = SYS_EVENT_SRS_TSHD_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_NIGHT_MODE_ON:
        {
            if ( pSystemParams->night_mode == TRUE )
            {
                return;
            }
            else
            {
                mSystemParams.night_mode = TRUE;
            }
            mSystemParams.sys_event = SYS_EVENT_NIGHT_MODE_SET;
            //pSDM_ObjCtrl->SendEvent( &mSystemParams );/*UI 4.2 If Night Mode is changed to "On", at the next power on, Night mode returns to the "Off" setting.*/
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_NIGHT_MODE_OFF:
        {
            if ( pSystemParams->night_mode == FALSE )
            {
                return;
            }
            else
            {
                mSystemParams.night_mode = FALSE;
            }
            mSystemParams.sys_event = SYS_EVENT_NIGHT_MODE_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_AV_DELAY_UP:
        {
            if ( pSystemParams->av_delay < AV_DELAY_MAX )
            {
                mSystemParams.av_delay++;
            }
            mSystemParams.sys_event = SYS_EVENT_AV_DELAY_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_AV_DELAY_DN:
        {
            if ( pSystemParams->av_delay > AV_DELAY_MIN )
            {
                mSystemParams.av_delay--;
            }
            mSystemParams.sys_event = SYS_EVENT_AV_DELAY_SET;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case  USER_EVENT_RF_PAIR:   /*Elvis*/
        {
            xRFHandleCommand mRFHandleCommand;

            mRFHandleCommand = RF_HANDLE_PAIRING_PHASE1;
            pRFHandle_ObjCtrl->rf_instr_sender(&mRFHandleCommand);
            pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_RF_PAIRING);
        }
            break;
            
        case USER_EVENT_VIZ_RMT_TOOGLE:
        {
            mSystemParams.sys_event = SYS_EVENT_VIZ_RMT_SET;

            if(pSystemParams->vizio_rmt == FALSE)
            {
                mSystemParams.vizio_rmt = TRUE;
            }
            else
            {
                mSystemParams.vizio_rmt = FALSE;
            }
            
            pUDM_ObjCtrl->SendEvent( &mSystemParams);

            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_VIZ_RMT_ON:
        {
            if(pSystemParams->vizio_rmt == FALSE)
            {
                mSystemParams.sys_event = SYS_EVENT_VIZ_RMT_SET;
                mSystemParams.vizio_rmt = TRUE;
                pUDM_ObjCtrl->SendEvent( &mSystemParams);
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;
        
        case USER_EVENT_VIZ_RMT_OFF:
        {
            if(pSystemParams->vizio_rmt == TRUE)
            {
                mSystemParams.sys_event = SYS_EVENT_VIZ_RMT_SET;
                mSystemParams.vizio_rmt = FALSE;
                pUDM_ObjCtrl->SendEvent( &mSystemParams);
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;
        
        case USER_EVENT_MUTE:
        {
#if ( configSII_DEV_953x_PORTING == 1 )
	    if ( (event.event_id == xHMI_EVENT_PROGRAM_IR) && (mSystemParams.cec_sac == TRUE) )
	    {
	        break;
	    }
#endif		
            mSystemParams.mute = !(mSystemParams.mute);

            if ( pSystemParams->mute == SOUND_DEMUTE )
            {
                if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH &&
                    pBTHandle_ObjCtrl->get_BTMute_status() == TRUE)
                {
                    //Don't unmute. Fix noise issue on BT src. Angus 2014/11/24
                }
                else
                {
                    mSystemParams.sys_event = SYS_EVENT_VOLUME_SET;
                    pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                    SendCecReportAudioStatus(mSystemParams);
                }
            }
            
            mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
            
            if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH &&
                pBTHandle_ObjCtrl->get_BTMute_status() == TRUE)
            {
                //Don't unmute. Fix noise issue on BT src.  Angus 2014/11/24
            }
            else
            {
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            }
            
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;
        
        case USER_EVENT_PREVIOUS:
        {
            if(pSystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK)
            {
                mSystemParams.sys_event = SYS_EVENT_USB_MMS_PREVIOUS;
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
            else if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH)
            {
                mSystemParams.sys_event = SYS_EVENT_BT_AVRCP_PREVIOUS;
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;
            
        case USER_EVENT_NEXT:
        {
            if(pSystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK)
            {
                mSystemParams.sys_event = SYS_EVENT_USB_MMS_NEXT;
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);       
            }
            else if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH)
            {
                mSystemParams.sys_event = SYS_EVENT_BT_AVRCP_NEXT;
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;  
        
        case USER_EVENT_PLAY_PAUSE:
        {
            if(pSystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK)
            {
                mSystemParams.sys_event = SYS_EVENT_USB_MMS_PLAY_PAUSE;
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
            else if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH)
            {
                mSystemParams.sys_event = SYS_EVENT_BT_AVRCP_PLAY_PAUSE;
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_BT_PAIR:
        {
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        
            pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
                
            if(pSystemParams->input_src != AUDIO_SOURCE_BLUETOOTH)
            {
                mSystemParams.input_src = AUDIO_SOURCE_BLUETOOTH;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
               
            }
            mSystemParams.sys_event = SYS_EVENT_BT_PAIRING;
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_BT_CLEAN:
        {
            if ( pPowerHandle_ObjCtrl->get_power_state() != POWER_ON )
            {
                TRACE_ERROR((0, "System power has not been powered up !! "));
                return;
            }
            
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }

            mSystemParams.sys_event = SYS_EVENT_BT_CLEAN_DEVICE;
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                      
        }
        break;

        case USER_EVENT_DEVICE_RESET:
        {
            bIsSystemReset = TRUE;
            //led control
            mSystemParams.sys_event = SYS_EVENT_SYSTEM_RESET; 
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            //set default value
            _RST_DEF_SETTING();
            HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
            //power down
            _PWR_ENABLE_CTRL(FALSE);
        }
        break;

        case USER_EVENT_MODE_DEMO1:
        {
            if ( pSystemParams->op_mode == MODE_DEMO_1 )
            {
                mSystemParams.op_mode = MODE_USER;
                HMI_Service_rstSystemParams();

                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_EXIT_DEMO);
                pBTN_ObjCtrl->register_user_cmd();
                TRACE_DEBUG((0, "MODE - USER "));
                
                pPowerHandle_ObjCtrl->power_toggle();
                //Fix issue SB3851_C0-156 : quit demo mode , SB will auto enter demo mode after AC Off/On Angus 2014/11/25
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 

                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
            else
            {
                mSystemParams.op_mode = MODE_DEMO_1;
                HMI_Service_setDemo1n3_SysParams( mSystemParams.op_mode );
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams ); /*Update vol and mute status*/
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_ENTER_DEMO1);
                pBTN_ObjCtrl->register_demo_1n3_cmd();

                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
                TRACE_DEBUG((0, "MODE - DEMO 1 "));
            }
        }
            break;

        case USER_EVENT_MODE_DEMO2:
        {
            /*TODO: Angus*/
            if ( pSystemParams->op_mode == MODE_DEMO_2 )
            {
                mSystemParams.op_mode = MODE_USER;
                HMI_Service_rstSystemParams();

                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_EXIT_DEMO);
                pBTN_ObjCtrl->register_user_cmd();
                TRACE_DEBUG((0, "MODE - USER "));
                
                //pPowerHandle_ObjCtrl->power_toggle();
                //Fix issue SB3851_C0-156 : quit demo mode , SB will auto enter demo mode after AC Off/On Angus 2014/11/25
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING; 
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 

                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );

                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);

                pPowerHandle_ObjCtrl->power_toggle();
            }
            else
            {
                mSystemParams.op_mode = MODE_DEMO_2;
                HMI_Service_setDemo1n3_SysParams( mSystemParams.op_mode );
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pUDM_ObjCtrl->SendEvent( &mSystemParams ); /*Update vol and mute status*/
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_ENTER_DEMO2);                
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                
                pBTN_ObjCtrl->register_demo_2_cmd();
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);

                TRACE_DEBUG((0, "MODE - DEMO 2 ")); 
            }
        }
            break;

        case USER_EVENT_MODE_DEMO3:
        {
            if ( pSystemParams->op_mode == MODE_DEMO_3 )
            {
                mSystemParams.op_mode = MODE_USER;
                HMI_Service_rstSystemParams();

                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_EXIT_DEMO);  
                pBTN_ObjCtrl->register_user_cmd();
       
                TRACE_DEBUG((0, "MODE - USER "));
                pPowerHandle_ObjCtrl->power_toggle();
                //Fix issue SB3851_C0-156 : quit demo mode , SB will auto enter demo mode after AC Off/On Angus 2014/11/25
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 

                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }
            else
            {
                mSystemParams.op_mode = MODE_DEMO_3;
                HMI_Service_setDemo1n3_SysParams( mSystemParams.op_mode );
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams ); /*Update vol and mute status*/
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_ENTER_DEMO3);
                pBTN_ObjCtrl->register_demo_1n3_cmd();
                
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
                TRACE_DEBUG((0, "MODE - DEMO 3 "));
            }
        }
            break;

        case USER_EVENT_MODE_FACTORY:
        {
            /* swithing to factory mode */
            TRACE_DEBUG((0, "MODE - FACTORY "));
         
            Debug_unregister( (DEBUG_ERROR| DEBUG_DEBUG | DEBUG_INFO) );
            HMI_Service_setSystemParams(&mDefFactoryParams);
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
            mSystemParams.op_mode = MODE_FACTORY;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            vTaskDelay( TASK_MSEC2TICKS(100) );

            if( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF)
            {
                _PWR_ENABLE_CTRL(TRUE);
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else if (pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)
            {
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }

            //IS_FAC_EXIST = TRUE;
        }
            break;

        case USER_EVENT_START_MODE:
        {
            if(pSystemParams->op_mode == MODE_DEMO_2)
            {
                pUSB_MMP_ObjCtrl->StartDemoMode2();

                HMI_Service_setDemo1n3_SysParams(MODE_DEMO_2);
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_EXIT_TIMEOUT_DEMO);
            }
        }
        break;

        case USER_EVENT_STOP_MODE:
        {
            if(pSystemParams->op_mode == MODE_DEMO_2)
            {
                 pUSB_MMP_ObjCtrl->StopDemoMode2();
                 HMI_Service_setDemo2_Timeout();
            }
        }
        break;
        
        case USER_EVENT_AUTO_INPUT_DETECT_CLEAN:
        {
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
            mSystemParams.auto_input_detect = FALSE;
            pSDM_ObjCtrl->SendEvent( &mSystemParams); 
        }
            break;

        case USER_EVENT_VERSION:
        {
            //led control
            mSystemParams.sys_event = SYS_EVENT_VERSION; 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_FAC_OUTPUT_POWER_TOOGLE:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                if (pSystemParams->master_gain==FACTORY_MASTER_GAIN)
                {
                    mSystemParams.master_gain = FACTORY_HALF_GAIN; 
                }
                else
                {
                    mSystemParams.master_gain = FACTORY_MASTER_GAIN; 
                }
                
                mSystemParams.sys_event = SYS_EVENT_VOLUME_SET;
                
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                SendCecReportAudioStatus(mSystemParams);
            }
        }
            break;
            
         case USER_EVENT_CENTER_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_CENTER;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_WOOFER_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_LFE0;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_LEFT_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_FL;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_RIGHT_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_FR;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_NORMAL_SOUND:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_DISABLE;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_SL_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_RL;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_SR_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_RR;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_LEFT_CENTER_RIGHT_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_FL_FR_CENTER;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_SL_WOOFER_SR_ONLY:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_RR_RL_WOOFER;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_FAC_RF_PAIRING:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                xRFHandleCommand mRFHandleCommand;
                
                mRFHandleCommand = RF_HANDLE_PAIRING_PHASE1;
                pRFHandle_ObjCtrl->rf_instr_sender(&mRFHandleCommand);
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_RF_PAIRING);
            }
        }
            break;

        case USER_EVENT_FAC_BT_PAIRING:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);
                
                if(pSystemParams->input_src != AUDIO_SOURCE_BLUETOOTH)
                {
                    mSystemParams.input_src = AUDIO_SOURCE_BLUETOOTH;
                    mSystemParams.sys_event = SYS_EVENT_SEL_SRC; 
                    pASH_ObjCtrl->SendEvent( &mSystemParams );
                    pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
                    
                }
                mSystemParams.sys_event = SYS_EVENT_BT_PAIRING;
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                
            }
        }
            break;
            
        case USER_EVENT_FAC_SEL_AUX1:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.input_src = AUDIO_SOURCE_AUX1;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
                
            }
        }
            break;

        case USER_EVENT_FAC_SEL_AUX2:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.input_src = AUDIO_SOURCE_AUX2;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
             
            }
        }
            break;
            
        case USER_EVENT_FAC_SEL_COAXIAL:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.input_src = AUDIO_SOURCE_COAXIAL;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
              
            }
        }
            break;

        case USER_EVENT_FAC_SEL_OPTICAL:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.input_src = AUDIO_SOURCE_OPTICAL;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
                
            }
        }
            break;

        case USER_EVENT_FAC_SEL_USB_PLAYBACK:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.input_src = AUDIO_SOURCE_USB_PLAYBACK;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
               
            }
        }
            break;

#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_FAC_SEL_HDMI_IN: 
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.input_src = AUDIO_SOURCE_HDMI_IN;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams); /*TODO: Don't merge for debug*/
               
            }
        }
            break;

        case USER_EVENT_FAC_SEL_HDMI_ARC: 
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.input_src = AUDIO_SOURCE_HDMI_ARC;
                //mSystemParams.cec_arc = TRUE;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams); /*TODO: Don't merge for debug*/
            }
        }
            break;
#endif
            
        case USER_EVENT_FAC_LED_CTRL:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                if (FAC_LED_PWR_ON == 0)
                {
                    mSystemParams.sys_event = SYS_EVENT_FAC_ALL_LED_DD;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams );
                    FAC_LED_PWR_ON = 1;
                }
                else if (FAC_LED_PWR_ON == 1)
                {
                    mSystemParams.sys_event = SYS_EVENT_FAC_ALL_LED_DTS;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams );
                    FAC_LED_PWR_ON = 2;
                }
                else
                {
                    mSystemParams.sys_event = SYS_EVENT_FAC_NO_LED;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams );
                    FAC_LED_PWR_ON = 0;
                }
            }
        }
            break;

        case USER_EVENT_FAC_AQ_OFF:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_AQ_CTRL_OFF;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_FAC_AQ_ON:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                mSystemParams.sys_event = SYS_EVENT_FAC_AQ_CTRL_ON;
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_FAC_MODE_OFF:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {
                pPowerHandle_ObjCtrl->power_toggle();
                
                Debug_initialize( );
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                mSystemParams.op_mode = MODE_USER;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                vTaskDelay( 100 );
                
                mSystemParams.night_mode = FALSE;/*UI 4.2 If Night Mode is changed to "On", at the next power on, Night mode returns to the "Off" setting.*/
                mSystemParams.mute = SOUND_DEMUTE; /*UI 4.2 When sound bar powers down in "mute" mode, at power up the previous unmuted volume level resume (not muted).  Sound bar does not remain in mute mode after a power cycle to avoid issues with universal remotes.*/
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);

                if (pSystemParams->auto_input_detect == TRUE)
                {    
                    HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
                }
            }
        }
            break;

        case USER_EVENT_FAC_CLEAN_STORAGE:
        {
            if(pSystemParams->op_mode == MODE_FACTORY)
            {               
                //BT clean device                
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_RESET; 
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                //led control
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                //set default value
                _RST_DEF_SETTING();
                HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
                //power down
                _PWR_ENABLE_CTRL(FALSE);
            }
        }
            break;
            
#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_CEC_POWER_OFF:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            if ( HMI_Service_IsPowerKeyAvailable(FALSE, TRUE) )    //system or hdmi is updare
            {
                xHMISrvEventParams srv_parms = {xHMI_EVENT_CEC, USER_EVENT_UNDEFINED, 0 };

                srv_parms.event = USER_EVENT_POWER;
                HMI_Service_EventSender( &srv_parms );
            }
        }
            break;

        case USER_EVENT_CEC_SET_MUTE:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            if ( (pSystemParams->input_src == AUDIO_SOURCE_HDMI_IN) || 
                (pSystemParams->input_src == AUDIO_SOURCE_HDMI_ARC) )
            {
                if ( mSystemParams.mute == FALSE )
                {
                    mSystemParams.mute = TRUE;
                    mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
                    pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                    pUDM_ObjCtrl->SendEvent( &mSystemParams );
                }
            }
        }
            break;
    
        case USER_EVENT_CEC_SET_DEMUTE:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            if ( (pSystemParams->input_src == AUDIO_SOURCE_HDMI_IN) || 
                 (pSystemParams->input_src == AUDIO_SOURCE_HDMI_ARC) )
            {
                if ( mSystemParams.mute == TRUE )
                {
                    mSystemParams.mute = FALSE;
                    mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
                    pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                    pUDM_ObjCtrl->SendEvent( &mSystemParams );
                }
            }
        }
            break;
    
        case USER_EVENT_CEC_SET_SAC_ON:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            mSystemParams.cec_sac = TRUE;
        }
            break;
        
        case USER_EVENT_CEC_SET_SAC_OFF:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            mSystemParams.cec_sac = FALSE;
        }
            break;
        
        case USER_EVENT_CEC_SET_ARC_ON:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            mSystemParams.cec_arc = TRUE;
        }
            break;
        
        case USER_EVENT_CEC_SET_ARC_OFF:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            mSystemParams.cec_arc = FALSE;
        }
            break;

        case USER_EVENT_CEC_SET_VOLUME_UP:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            if ( pSystemParams->master_gain < master_gain_limit.max_gain )
            {
                mSystemParams.master_gain++;// = event.params;
            }
            else
            {
                mSystemParams.master_gain = master_gain_limit.max_gain;
            }

            if( pSystemParams->mute == SOUND_MUTE )
            {
                mSystemParams.mute = SOUND_DEMUTE;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
            }

            mSystemParams.sys_event = SYS_EVENT_VOLUME_SET; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );   
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            SyncSacVolume(mSystemParams);
        }
            break;
        case USER_EVENT_CEC_SET_VOLUME_DOWN:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            if ( pSystemParams->master_gain > master_gain_limit.min_gain )
            {
                TRACE_DEBUG((0, "USER_EVENT_CEC_SET_VOLUME_DOWN = %d", event.params));
                mSystemParams.master_gain--;// = event.params;
            }
            else
            {
                mSystemParams.master_gain = master_gain_limit.min_gain;
            }

#if 0
            if( pSystemParams->mute == SOUND_MUTE )
            {
                mSystemParams.mute = SOUND_DEMUTE;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
            }
#endif
            mSystemParams.sys_event = SYS_EVENT_VOLUME_SET; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );   
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            SyncSacVolume(mSystemParams);
        }
            break;

        case USER_EVENT_CEC_SET_VOLUME_MUTE:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
             //if ( (pSystemParams->input_src == AUDIO_SOURCE_HDMI_IN) || 
             //   (pSystemParams->input_src == AUDIO_SOURCE_HDMI_ARC) )
            {
                mSystemParams.mute = (bool)event.params;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                SyncSacVolume(mSystemParams);
            }
        }
            break;

        case USER_EVENT_CEC_SEL_HDMI_IN:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }
        
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_HDMI_IN )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_HDMI_IN;
                //mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams); /*TODO: Don't merge for debug*/
            }
        }
            break;
    
        case USER_EVENT_CEC_SEL_HDMI_ARC:
        {
	        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
	        {
	            break;
	        }
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
            }
            
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/

            if ( mSystemParams.input_src == AUDIO_SOURCE_HDMI_ARC )
            {
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.last_input_src = mSystemParams.input_src;
                mSystemParams.input_src = AUDIO_SOURCE_HDMI_ARC;        
                //mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams); /*TODO: Don't merge for debug*/
            }
        }
            break;
            
#endif
        
        default:
            break;
     }
}

static void HMI_FMD_CommandHandle( const xHMISystemParams *params )
{
    if ( params == NULL )
        return;
    /*notice: the api is only used to factory mode.*/
    if ( params->op_mode != MODE_FACTORY )
        return;

    switch( params->sys_event)    
    {
        case SYS_EVENT_FAC_SET_MODE_ON:
        {
            Debug_unregister( (DEBUG_ERROR| DEBUG_DEBUG | DEBUG_INFO) );
            HMI_Service_setSystemParams(&mDefFactoryParams);
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
            mSystemParams.op_mode = MODE_FACTORY;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            vTaskDelay( TASK_MSEC2TICKS(100) );

            if( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF)
            {
                _PWR_ENABLE_CTRL(TRUE);
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else if (pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)
            {
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
            }

            pFMD_ObjCtrl->AckSuccess( NULL );
        }
            break;

        case SYS_EVENT_FAC_SET_MODE_OFF:
        {
            Debug_initialize( );
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
            mSystemParams.op_mode = MODE_USER;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            vTaskDelay( 100 );

            mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
            mSystemParams.op_mode = MODE_USER;
            
            pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);

            pFMD_ObjCtrl->AckSuccess( NULL );
            return;
        }
            break;
            
        case SYS_EVENT_SYSTEM_RESET:
        {
            mSystemParams.sys_event = SYS_EVENT_SYSTEM_RESET; 
            //BT clean device                
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            //led control
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            //set default value
            _RST_DEF_SETTING();
            //power down
            _PWR_ENABLE_CTRL(FALSE);
            vTaskDelay( TASK_MSEC2TICKS(3800+5500) );    //Angus add 5500 for requirement of raken that Ack cmd must be sended after led flashing 3 times. 
            pFMD_ObjCtrl->AckSuccess( NULL );
            
            return;
        }
            break;
            
        case SYS_EVENT_BT_PAIRING:
        {
            pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_PAIRING);

            if(pSystemParams->input_src != AUDIO_SOURCE_BLUETOOTH)
            {
                mSystemParams.input_src = AUDIO_SOURCE_BLUETOOTH;
                mSystemParams.sys_event = SYS_EVENT_SEL_SRC; 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);              
            }
                    
            mSystemParams.sys_event = SYS_EVENT_BT_PAIRING;
            pBTHandle_ObjCtrl->SendEvent( &mSystemParams );
            pFMD_ObjCtrl->AckSuccess( NULL );
        }
            break;  
 
        case SYS_EVENT_SYSTEM_UP:
        case SYS_EVENT_SYSTEM_DOWN:
        case SYS_EVENT_SEL_SRC:
        case SYS_EVENT_VOLUME_MUTE_SET:
        case SYS_EVENT_VOLUME_SET:
        case SYS_EVENT_TREBLE_SET:
        case SYS_EVENT_BASS_SET:    
        case SYS_EVENT_FAC_ROUTE_CTRL_DISABLE:
        case SYS_EVENT_FAC_ROUTE_CTRL_FL_FR:
        case SYS_EVENT_FAC_ROUTE_CTRL_RR_RL:
        case SYS_EVENT_FAC_ROUTE_CTRL_CENTER:
        case SYS_EVENT_FAC_ROUTE_CTRL_LFE0:
        case SYS_EVENT_FAC_AQ_CTRL_ON:
        case SYS_EVENT_FAC_AQ_CTRL_OFF:
        case SYS_EVENT_FAC_ALL_LED:
        case SYS_EVENT_FAC_NO_LED:
        case SYS_EVENT_FAC_LED1:
        case SYS_EVENT_FAC_LED2:
        case SYS_EVENT_FAC_LED3:
        case SYS_EVENT_FAC_LED4:
        case SYS_EVENT_FAC_LED5:
        case SYS_EVENT_FAC_LED6:
        case SYS_EVENT_FAC_LED7:
        case SYS_EVENT_FAC_LED8:
        case SYS_EVENT_FAC_LED9:
        case SYS_EVENT_FAC_LED10:
        case SYS_EVENT_FAC_LED11:
        case SYS_EVENT_FAC_LED12:
        case SYS_EVENT_FAC_LED13:
        case SYS_EVENT_FAC_LED14:
        {
            pASH_ObjCtrl->SendEvent( params );
            pUDM_ObjCtrl->SendEvent( params );
            pFMD_ObjCtrl->AckSuccess( NULL );
        }
            break;

        default:
            return;
    }

}

static bool HMI_Service_EventBypassWhenBTPairing( xHMISrvEventParams *pEventParams )
{
    bool ret = TRUE;

    //TRACE_DEBUG((0,"bt STATUS -----%d",pBTHandle_ObjCtrl->get_status()));
    if( ( pBTHandle_ObjCtrl->get_status() == BT_PAIRING ) && ( pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH ) )
    {
        switch( pEventParams->event )
        {       
            case USER_EVENT_SEL_LAST_SRC:
            case USER_EVENT_SEL_SRC:
            case USER_EVENT_AUTO_SEL_SRC:
            case USER_EVENT_START_AUTO_SEL_SRC:
            case USER_EVENT_SEL_AUX1:
            case USER_EVENT_SEL_AUX2:
            case USER_EVENT_SEL_COAXIAL:
            case USER_EVENT_SEL_OPTICAL:
            case USER_EVENT_SEL_USB_PLAYBACK:
            case USER_EVENT_SEL_BT:
#if ( configSII_DEV_953x_PORTING == 1 )
            case USER_EVENT_SEL_HDMI_IN:
            case USER_EVENT_CEC_SEL_HDMI_IN:
            case USER_EVENT_SEL_HDMI_ARC:
            case USER_EVENT_CEC_SEL_HDMI_ARC:
            case USER_EVENT_CEC_POWER_ON_HDMI_IN:
            case USER_EVENT_CEC_POWER_ON_HDMI_ARC:
	    case USER_EVENT_CEC_POWER_ON:
#endif
            case USER_EVENT_POWER:
            case USER_EVENT_FORCE_POWER_UP:
            case USER_EVENT_MODE_FACTORY:
            case USER_EVENT_DEVICE_RESET:
            {
                ret = TRUE;
            }
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    
    return ret;
}

static bool HMI_Service_EventFilterWhenCtrlSpeed( xHMISrvEventParams *pEventParams )
{
    bool ret = TRUE;

    //TRACE_DEBUG((0,"bt STATUS -----%d",pBTHandle_ObjCtrl->get_status()));
    //For select input source
    switch( pEventParams->event )
    {       
        //case USER_EVENT_AUTO_SEL_SRC:
        //case USER_EVENT_START_AUTO_SEL_SRC:
        case USER_EVENT_SEL_AUX1:
        case USER_EVENT_SEL_AUX2:
        case USER_EVENT_SEL_OPTICAL:
        case USER_EVENT_SEL_USB_PLAYBACK:
        case USER_EVENT_SEL_BT:
#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_SEL_HDMI_IN:
        case USER_EVENT_SEL_HDMI_ARC:
#endif
        {
            if ( ((xTaskGetTickCount() - input_key_time)/portTICK_RATE_MS) <= PRESS_KEY_DURATION)
            {
                //PRESS TOO FAST, WE NEED TO DISCARD
                ret = FALSE;
            }
            else
            {
                //SYSTEM WILL HANDLE THE EVENT, AND WE NEED TO RECOUNTER THE TIME FOR NEXT EVENT
                input_key_time = xTaskGetTickCount();
                ret = TRUE;
            }
        }
            break;
        case USER_EVENT_POWER:
        case USER_EVENT_FORCE_POWER_UP:
#if ( configSII_DEV_953x_PORTING == 1 )
    case USER_EVENT_CEC_POWER_ON_HDMI_IN:
    case USER_EVENT_CEC_POWER_ON_HDMI_ARC:
    case USER_EVENT_CEC_POWER_ON:
#endif
        {
            input_key_time = 0;
            ret = TRUE;
        }
            break;
        default:
            ret = TRUE;
            break;
    }

    return ret;


}

static bool HMI_Service_EventBypassWhenStandby( xHMISrvEventParams *pEventParams )
{
    bool ret = FALSE;

    if( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF ) 
    {
        switch( pEventParams->event )
        {       
            case USER_EVENT_POWER:
            case USER_EVENT_FORCE_POWER_UP:
#if ( configSII_DEV_953x_PORTING == 1 )
            case USER_EVENT_CEC_POWER_ON_HDMI_IN:
            case USER_EVENT_CEC_POWER_ON_HDMI_ARC:
            case USER_EVENT_CEC_POWER_ON:
            case USER_EVENT_CEC_SET_SAC_ON:
            case USER_EVENT_CEC_SET_SAC_OFF:
            case USER_EVENT_CEC_SET_ARC_ON:
            case USER_EVENT_CEC_SET_ARC_OFF:
            case USER_EVENT_SAC_WAKE_UP:
#endif
            case USER_EVENT_MODE_FACTORY:    
            {    
                //IS_FAC_EXIST = TRUE;
                ret = TRUE;
            }
                break;
            case USER_EVENT_VOL_UP:
            {
                pEventParams->event = USER_EVENT_POWER;
                ret = TRUE;
            }
                break;
            default:
                ret = FALSE;
                break;
        }
    }
    else if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
    {
        ret = TRUE;
    }
    return ret;
}

static void HMI_Service_DetectStopCCKCommand( xHMIUserEvents event )
{
    switch( event )
    {
        case USER_EVENT_POWER:
#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_CEC_POWER_ON_HDMI_IN:
        case USER_EVENT_CEC_POWER_ON_HDMI_ARC:
        case USER_EVENT_CEC_POWER_ON:
#endif
        case USER_EVENT_START_MODE:
        case USER_EVENT_STOP_MODE:
        case USER_EVENT_SEL_AUX1:
        case USER_EVENT_SEL_AUX2:
        case USER_EVENT_SEL_COAXIAL:
        case USER_EVENT_SEL_OPTICAL:
        case USER_EVENT_SEL_USB_PLAYBACK:
        case USER_EVENT_SEL_BT:
#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_SEL_HDMI_IN:
        case USER_EVENT_CEC_SEL_HDMI_IN:
        case USER_EVENT_SEL_HDMI_ARC:    
        case USER_EVENT_CEC_SEL_HDMI_ARC:    
#endif
        case USER_EVENT_SEL_SRC:
        case USER_EVENT_MODE_DEMO1:
        case USER_EVENT_MODE_DEMO2:
        case USER_EVENT_MODE_DEMO3:
        case USER_EVENT_MODE_FACTORY:
        case USER_EVENT_BT_PAIR:
        case USER_EVENT_DEVICE_RESET:
        case USER_EVENT_BASS_UP:
        case USER_EVENT_BASS_DN:
        case USER_EVENT_TREBLE_UP:
        case USER_EVENT_TREBLE_DN:
        case USER_EVENT_SUB_UP:
        case USER_EVENT_SUB_DN:
        case USER_EVENT_CENTER_UP:
        case USER_EVENT_CENTER_DN:
        case USER_EVENT_REAR_UP:
        case USER_EVENT_REAR_DN:
        case USER_EVENT_SRS_TRUVOL_ON:
        case USER_EVENT_SRS_TRUVOL_OFF:
        case USER_EVENT_SRS_TSHD_ON:
        case USER_EVENT_SRS_TSHD_OFF:
        case USER_EVENT_NIGHT_MODE_ON:
        case USER_EVENT_NIGHT_MODE_OFF:
        case USER_EVENT_AV_DELAY_UP:
        case USER_EVENT_AV_DELAY_DN:
        case USER_EVENT_BALANCE_UP:
        case USER_EVENT_BALANCE_DN:
        {
             pCCKManage_ObjCtrl->stop(&mSystemParams);
        }
            break;
    }
}

static bool HMI_Service_DetectConfirmEvent(void *params)
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;
    bool res = TRUE;

    switch(pEventParams->event)
    {
        case USER_EVENT_POWER:
#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_CEC_POWER_ON_HDMI_IN:
        case USER_EVENT_CEC_POWER_ON_HDMI_ARC:
        case USER_EVENT_CEC_POWER_ON:
        case USER_EVENT_CEC_SEL_HDMI_IN:
        case USER_EVENT_CEC_SEL_HDMI_ARC:
#endif
        case USER_EVENT_MUTE:
        case USER_EVENT_PREVIOUS:
        case USER_EVENT_NEXT:
        case USER_EVENT_PLAY_PAUSE:
        case USER_EVENT_BT_PAIR:
        case USER_EVENT_RF_PAIR:
        case USER_EVENT_DEVICE_RESET:
        case USER_EVENT_MODE_DEMO1:
        case USER_EVENT_MODE_DEMO2:
        case USER_EVENT_MODE_DEMO3:
        case USER_EVENT_VIZ_RMT_TOOGLE:
        case USER_EVENT_APD_TOOGLE:
        case USER_EVENT_VIZ_RMT_ON:
        case USER_EVENT_VIZ_RMT_OFF:
        case USER_EVENT_MODE_FACTORY:
        case USER_EVENT_START_MODE:
        case USER_EVENT_STOP_MODE:
        case USER_EVENT_AUTO_SEL_SRC:
        case USER_EVENT_CCK:
        {
            res = FALSE;
        }
            break;
    }
    HMI_Service_DetectStopCCKCommand(pEventParams->event);
    return res;
}

static bool HMI_Service_checkEventTimoutOrFirstPressed( xHMIUserEvents cur_events )
{
    static portTickType xTimeTick = 0;
    static xHMIUserEvents lastEvent = USER_EVENT_NULL;
    //static bool HasBeenSetVolumeEvent = FALSE;
    portTickType duration;
    bool ret = FALSE;
    xEventTypeAssort ETA = EVENT_ASSORT_NO_NEED_FIRST_CONFIRM;
    
    if( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
        return FALSE;

    ETA = HMI_Service_Event_Assort(cur_events, lastEvent);

    lastEvent = cur_events;
    
    if ( ETA == EVENT_ASSORT_NO_NEED_FIRST_CONFIRM )
    {
        ret = FALSE;
    }
    else
    {
        duration = ( xTaskGetTickCount( ) - xTimeTick );
        xTimeTick = xTaskGetTickCount( );

        if ( ETA == EVENT_ASSORT_DIFFERENT )
        {
            ret = TRUE;
        }
        else // EVENT_ASSORT_SAME
        {
            if ( (duration/portTICK_RATE_MS) > HMI_CMD_TIME_UP_MSEC )    // > 15 sec
            {
                ret = TRUE;
            }
            else    // < 15 sec
            {
                ret = FALSE;
            }
        }
     }
    
    return ret;
}

static bool HMI_Service_checkEventLongPressed( xHMIUserEvents cur_events )
{
    static portTickType xLongPressedTimeTick = 0;
    portTickType duration;
    bool ret = FALSE;
    
    duration = ( xTaskGetTickCount( ) - xLongPressedTimeTick );
    xLongPressedTimeTick = xTaskGetTickCount( );

    switch (cur_events)
    {
        case USER_EVENT_SEL_AUX1:
        case USER_EVENT_SEL_AUX2:
        case USER_EVENT_SEL_COAXIAL:
        case USER_EVENT_SEL_OPTICAL:
        case USER_EVENT_SEL_USB_PLAYBACK:
        case USER_EVENT_SEL_BT:
#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_SEL_HDMI_IN:
        case USER_EVENT_SEL_HDMI_ARC:
#endif
        {
            if ( (duration/portTICK_RATE_MS) < HMI_CMD_TIME_PRESS_MSEC )    
            {
                ret = TRUE;
            }
            else   
            {
                ret = FALSE;
            }
        }
            break;

        default:
        {
            ret = FALSE;
        }
            break;
    }
            
    return ret;
}

static void HMI_Service_FilterDemoModeEvent(void *params)
{
     xHMISrvEventParams* pParams = ( xHMISrvEventParams *)params;
    if(mSystemParams.op_mode == MODE_DEMO_1 || mSystemParams.op_mode == MODE_DEMO_2 ||\
       mSystemParams.op_mode == MODE_DEMO_3)
    {
        switch(pParams->event)
        {
            case USER_EVENT_POWER:
            {
                if(pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
                {
                    pParams->event = USER_EVENT_POWER;           
                }
                else
                {
                    if(mSystemParams.op_mode == MODE_DEMO_2)
                    {
                        pParams->event = USER_EVENT_START_MODE;      
                    }
                    else
                    {
                        pParams->event = USER_EVENT_NULL;
                    }
                }
            }
                break;

            case USER_EVENT_SEL_AUX1:
            case USER_EVENT_SEL_AUX2:
            case USER_EVENT_SEL_COAXIAL:
            case USER_EVENT_SEL_OPTICAL:
            case USER_EVENT_SEL_USB_PLAYBACK:
#if ( configSII_DEV_953x_PORTING == 1 )
            case USER_EVENT_SEL_HDMI_IN:
            case USER_EVENT_SEL_HDMI_ARC:   
#endif
            case USER_EVENT_SEL_SRC:
            {
                if(mSystemParams.op_mode == MODE_DEMO_2)
                {
                    pParams->event = USER_EVENT_STOP_MODE;      
                }
                else
                {
                    pParams->event = USER_EVENT_NULL;
                }
            }
            break;

            case USER_EVENT_START_AUTO_SEL_SRC:
            case USER_EVENT_SEL_BT:
            case USER_EVENT_BT_PAIR:
            case USER_EVENT_RF_PAIR:
            case USER_EVENT_CCK:
            {
                pParams->event = USER_EVENT_NULL;
            }
                break;
        }
    }
}

static bool HMI_Service_Filter_VIZ_FAC_Event(void *params)
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;
    
#if ( configSII_DEV_953x_PORTING == 1 )
    if( pEventParams->event_id== xHMI_EVENT_VIZ_FAC_IR)
    {
        if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
        {
            if (mSystemParams.vizio_rmt == TRUE)
            {
                return TRUE;
            }

            return FALSE;
        }
        else
        {
            if ( mSystemParams.cec_sac == TRUE || mSystemParams.vizio_rmt == FALSE )
            {
                if (pSystemParams->op_mode == MODE_FACTORY)/*Tony150224:Factory remote hot key is not able to work when SAC has been connected.*/
                     return TRUE;
                
                return FALSE;
            }
        }
    }
#else
    if(mSystemParams.vizio_rmt == FALSE && pEventParams->event_id== xHMI_EVENT_VIZ_FAC_IR)
        return FALSE;
#endif

    return TRUE;
}

static bool HMI_Service_Filter_Debounce(void *params)
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;
    static portTickType BounceTick;
    portTickType BounceDuration;
    static xHMIUserEvents LastEvent = USER_EVENT_NULL; // David, fix warning.

    if( LastEvent == USER_EVENT_MODE_DEMO1 ||
         LastEvent == USER_EVENT_MODE_DEMO2 ||
         LastEvent == USER_EVENT_MODE_DEMO3)
    {
        BounceDuration = ( xTaskGetTickCount( ) - BounceTick);
        if ((BounceDuration/portTICK_RATE_MS) < DemoModeBounceTime)
        {
            return FALSE;             
        }
        else
        {         
            LastEvent = pEventParams->event;
        }

    }
    else
    {
        if ( pEventParams->event == USER_EVENT_MODE_DEMO1 ||
             pEventParams->event == USER_EVENT_MODE_DEMO2 ||
             pEventParams->event == USER_EVENT_MODE_DEMO3)
        {
            BounceDuration = xTaskGetTickCount( );
            BounceTick = BounceDuration;
            LastEvent = pEventParams->event;
        }
    }
    return TRUE;
}

static bool HMI_Service_Filter_DemoTimeOutEvent(void *params)
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;
    
    if(Demo1TimeOut || Demo2TimeOut || Demo3TimeOut)//Angus added , follow VIZIO demo mode spec.
    {
        if(pEventParams->event == USER_EVENT_VOL_UP || pEventParams->event == USER_EVENT_VOL_DN)
        {
            return FALSE;
        }
    }

    switch(pEventParams->event)
    {
        case USER_EVENT_POWER:
        {
            if(pSystemParams->op_mode == MODE_DEMO_1 ||
                pSystemParams->op_mode == MODE_DEMO_3)
            {
                HMI_DemoMode1_3_TimeOutDetect_TaskCtrl(TRUE);
            }
        }
        break;
        
        case USER_EVENT_MODE_DEMO1:
        {
            if(pSystemParams->op_mode == MODE_DEMO_1)
            {
                HMI_DemoMode1_3_TimeOutDetect_TaskCtrl(FALSE);
            }
            else
            {
                HMI_DemoMode1_3_TimeOutDetect_TaskCtrl(TRUE);
            }
        }
        break;

        case USER_EVENT_MODE_DEMO2:
        {
            HMI_DemoMode1_3_TimeOutDetect_TaskCtrl(FALSE);
        }
        break;
        
        case USER_EVENT_MODE_DEMO3:
        {
            if(pSystemParams->op_mode == MODE_DEMO_3)
            {
                HMI_DemoMode1_3_TimeOutDetect_TaskCtrl(FALSE);
            }
            else
            {
                HMI_DemoMode1_3_TimeOutDetect_TaskCtrl(TRUE);
            }
        }
        break;

        case USER_EVENT_MODE_FACTORY:
        case USER_EVENT_DEVICE_RESET:
        {
            HMI_DemoMode1_3_TimeOutDetect_TaskCtrl(FALSE);
        }
        break;
    }

    switch(pEventParams->event)
    {
        case USER_EVENT_MODE_DEMO1:
        case USER_EVENT_MODE_DEMO2:
        case USER_EVENT_MODE_DEMO3:
        case USER_EVENT_MODE_FACTORY:
        case USER_EVENT_DEVICE_RESET:
        case USER_EVENT_START_MODE:
        {
            Demo1TimeOut = FALSE;
            Demo2TimeOut = FALSE;
            Demo3TimeOut = FALSE;
        }
        break;
    }
    
    return TRUE;
}

static bool HMI_Service_Filter_WhenSystemReset(void)
{
    if((bIsSystemReset == TRUE) && (pPowerHandle_ObjCtrl->get_power_state() != POWER_OFF))
    {
        return FALSE;
    }
    else
    {
        if(pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF)
        {
            bIsSystemReset = FALSE;
        }
    }

    return TRUE;
}


static bool HMI_Service_Filter_WhenSystemPowerOFF(void *params)
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;
    static bool FiltRes = FALSE;
    static bool startFilter = FALSE;
    
    if(mSystemParams.op_mode != MODE_USER)
        return TRUE;

    if(startFilter)
    {
        if(pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF)
        {     
            startFilter = FALSE;
            FiltRes = TRUE;
        }
        else
        {
            TRACE_ERROR((0, "It will not be tackled because System hasn't power down!!!"));
            FiltRes = FALSE;
        }
    }
    else
    {
        FiltRes = TRUE;
    }
    

    if(pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)
    {
        if(pEventParams->event == USER_EVENT_POWER)
        {
            //It is power down event ,  we will reject to receive any event untill system has been power down. 
            TRACE_ERROR((0, "It is power down event !!!!!!!!!!!"));
            startFilter = TRUE;
        }
    }
    
    return FiltRes;
}

static bool HMI_Service_Filter_PowerEvent_WhenSystemPowerON(void *params)
{

    static bool startTimer = FALSE;
    static portTickType SystemPowerONTime = 0;
    
    bool FiltRes = FALSE;
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;

    if(mSystemParams.op_mode != MODE_USER)
        return TRUE;

    if(startTimer)
    {   //TRACE_ERROR((0, "startTimer   xTaskGetTickCount() = %d , SystemPowerONTime = %d !!!",xTaskGetTickCount(),SystemPowerONTime));
        if(((xTaskGetTickCount() - SystemPowerONTime)/portTICK_RATE_MS) > TASK_MSEC2TICKS(3000)) // 3 SEC
        {
            startTimer = FALSE;
        }
    }

    
    if (pEventParams->event == USER_EVENT_POWER || pEventParams->event == USER_EVENT_VOL_UP)
    {
        if(pPowerHandle_ObjCtrl->get_power_state() != POWER_ON)
        {
            startTimer = TRUE;
            SystemPowerONTime = xTaskGetTickCount();
            
            FiltRes = TRUE;
            TRACE_ERROR((0, "ANGUS !!!! startTimer !!!"));
        }
        else
        {
            if(startTimer == FALSE)
            {
                FiltRes = TRUE;
            }
            else
            {
                TRACE_ERROR((0, "It will not be tackled because the interval of powerOn/Off is lower than 5s."));
                FiltRes = FALSE;  //It will not be tackled because the interval of powerOn/Off is lower than 5s.
            }
        }
        
    }
    else
    {
        FiltRes = TRUE;
    }
    
    return FiltRes;          
}

static void HMI_Service_handleCommand( void *params )
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;

    static uint8 LongPressCnt;

        //TRACE_ERROR((0, "HMI_Service_handleCommand %d ", pEventParams->event ));
    if ( pEventParams->event == USER_EVENT_NULL )
    {
        TRACE_ERROR((0, "Unsupported event "));
        return;
    }
    if ( pEventParams->event_id == xHMI_EVENT_NULL )
    {
        return;
    }

    if(HMI_Service_Filter_PowerEvent_WhenSystemPowerON(pEventParams)  == FALSE)
    {
        return;
    }
    
    if(HMI_Service_Filter_WhenSystemPowerOFF(pEventParams)  == FALSE)
    {
        return;
    }

    if(HMI_Service_Filter_WhenSystemReset() == FALSE)
    {
        //Angus added for prevent keypad bounce to interfere system reset procedure.
        return;
    }
    
    if(HMI_Service_Filter_VIZ_FAC_Event(pEventParams) == FALSE)
    {
        //TRACE_ERROR((0, "HMI_Service_Filter_VIZ_FAC_Event "));
        return;
    }
    if ( HMI_Service_EventBypassWhenStandby( pEventParams ) == FALSE )
    {
        //TRACE_ERROR((0, "HMI_Service_Filter_VIZ_FAC_Event "));
        return;
    }

    if ( HMI_Service_EventBypassWhenBTPairing(pEventParams) == FALSE )
    {
        //TRACE_ERROR((0, "HMI_Service_Filter_VIZ_FAC_Event "));
        return;
    }
    
    if (HMI_Service_EventFilterWhenCtrlSpeed(pEventParams) == FALSE )
    {
        //TRACE_ERROR((0, "HMI_Service_Filter_VIZ_FAC_Event "));
        return;
    }

    if(HMI_Service_Filter_Debounce(pEventParams) == FALSE)
    {
        //TRACE_ERROR((0, "HMI_Service_Filter_VIZ_FAC_Event "));
        return;
    }

    HMI_Service_FilterDemoModeEvent(pEventParams);
    
    if(HMI_Service_Filter_DemoTimeOutEvent(pEventParams) == FALSE)
    {
        //TRACE_ERROR((0, "HMI_Service_Filter_VIZ_FAC_Event "));
        return;
    }
     
    if(HMI_Service_DetectConfirmEvent(pEventParams) == FALSE)
    {
        //TRACE_ERROR((0, "HMI_Service_Filter_VIZ_FAC_Event "));
        if(HMI_Service_checkEventLongPressed( pEventParams->event ) == TRUE)
        {
            LongPressCnt++;
            TRACE_DEBUG((0,"LongPressCnt =%X",LongPressCnt));
            if (LongPressCnt == 0x0A)/*UI4.2 long pree 2sec start auto detect*/
            {   
                mSystemParams.auto_input_detect = TRUE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
                HMI_AutoInputDetect_TaskCtl(BTASK_RESUME);
                LongPressCnt = 0;
            }
        }
        else
        {
            LongPressCnt = 0;
            pIR_PrgRemote_ObjCtrl->SendEvent( pEventParams->event );
            HMI_Service_sendCommand( *pEventParams );
        }
    }
    else
    {
        if ( HMI_Service_checkEventTimoutOrFirstPressed( pEventParams->event ) == TRUE )
        {
        //TRACE_ERROR((0, "HMI_Service_checkEventTimoutOrFirstPressed "));
            switch (pEventParams->event)
            {
                case USER_EVENT_SEL_SRC:
#if (0) // Mask for new remote (v1.3)
                case USER_EVENT_SEL_AUX1:
                case USER_EVENT_SEL_AUX2:
                case USER_EVENT_SEL_COAXIAL:
                case USER_EVENT_SEL_OPTICAL:
                case USER_EVENT_SEL_USB_PLAYBACK:
                case USER_EVENT_SEL_BT:
#if ( configSII_DEV_953x_PORTING == 1 )
                case USER_EVENT_SEL_HDMI_IN:
                case USER_EVENT_SEL_HDMI_ARC:
#endif

#endif  

                {
                    //We need to active the LED scanning after confirm input source BT.
                    if( ( pBTHandle_ObjCtrl->get_status() == BT_PAIRING ) && ( mSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH ) )
                    {
                        TRACE_DEBUG((0, " Reject confirm input  ...  "));
                    }
                    else
                    {
                        mSystemParams.sys_event = SYS_EVENT_SEL_SRC;
                        pUDM_ObjCtrl->SendEvent( &mSystemParams);
                        TRACE_DEBUG((0, " confirm input src"));
                    }
                }
                break;
                
                case USER_EVENT_BASS_UP:
                case USER_EVENT_BASS_DN:
                {
                    mSystemParams.sys_event = SYS_EVENT_BASS_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;
                    
                case USER_EVENT_TREBLE_UP:
                case USER_EVENT_TREBLE_DN:
                {
                    mSystemParams.sys_event = SYS_EVENT_TREBLE_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;
                    
                case USER_EVENT_SUB_UP:
                case USER_EVENT_SUB_DN:
                {
                    mSystemParams.sys_event = SYS_EVENT_SUB_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;
                    
                case USER_EVENT_CENTER_UP:
                case USER_EVENT_CENTER_DN:
                {
                    mSystemParams.sys_event = SYS_EVENT_CENTER_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;
                    
                case USER_EVENT_REAR_UP:
                case USER_EVENT_REAR_DN:
                {
                    mSystemParams.sys_event = SYS_EVENT_LSRS_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;
                    
                case USER_EVENT_AV_DELAY_UP:
                case USER_EVENT_AV_DELAY_DN:
                {
                    mSystemParams.sys_event = SYS_EVENT_AV_DELAY_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;
                    
                case USER_EVENT_BALANCE_UP:
                case USER_EVENT_BALANCE_DN:
                {
                    mSystemParams.sys_event = SYS_EVENT_BALANCE_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;

                case USER_EVENT_APD_ON:
                case USER_EVENT_APD_OFF:
                {
                    mSystemParams.sys_event = SYS_EVENT_APD_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;

                case USER_EVENT_VIZ_RMT_ON:
                case USER_EVENT_VIZ_RMT_OFF:
                {
                    mSystemParams.sys_event = SYS_EVENT_VIZ_RMT_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;

                case USER_EVENT_SRS_TRUVOL_ON:
                case USER_EVENT_SRS_TRUVOL_OFF:
                {
                    mSystemParams.sys_event = SYS_EVENT_SRS_TRUVOL_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;

                case USER_EVENT_SRS_TSHD_ON:
                case USER_EVENT_SRS_TSHD_OFF:
                {
                    mSystemParams.sys_event = SYS_EVENT_SRS_TSHD_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;

                case USER_EVENT_NIGHT_MODE_ON:
                case USER_EVENT_NIGHT_MODE_OFF:
                {
                    mSystemParams.sys_event = SYS_EVENT_NIGHT_MODE_SET;
                    pUDM_ObjCtrl->SendEvent( &mSystemParams);
                }
                    break;

                default:
                    break;
            }
        }
        else
        {
            pIR_PrgRemote_ObjCtrl->SendEvent( pEventParams->event );
            HMI_Service_sendCommand( *pEventParams );
        }
    }
}

#if ( configSII_DEV_953x_PORTING == 1 )
static bool Is_HDMI_PlugIn(void)
{
    if ( GPIOMiddleLevel_Read_InBit(HDMI_PLUGIN_DETECTION_IO) )
        return TRUE;

    return FALSE;
}

static void HMI_Service_HDMI_StandbyCtrl()
{
#if (configHDMI_PLUG_IN_DETECTION == 0) // David, fix warning.
    xHMISystemParams SystemParms;
    xHMISrvEventParams srv_parms = {xHMI_EVENT_APC, USER_EVENT_UNDEFINED, 0};
#endif

#if (configSII_DEV_953x_TX_HPD_CTRL == 1)
    // alaways detect the hpd, and output to Sii 953X
    pPowerHandle_ObjCtrl->power_hpd_to_953X(pPowerHandle_ObjCtrl->power_is_hpd_from_tv());
#endif

    if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
    {
        //Standby mode
        //We need to check the HDMI_port2 status
#if (configHDMI_PLUG_IN_DETECTION == 0)
        //always supply voltage to sii 9533
        if (IsFirstSystemStartup == TRUE)
        {
            //We need to check the power status by ourself
            //pHDMI_DM_ObjCtrl->Is_HDMI_port2_ready(TRUE);
#if 1
            pPowerHandle_ObjCtrl->power_5v_ctrl(TRUE); //When AC on, turn on the 5v for hdmi 9533
             SystemParms.sys_event = SYS_EVENT_HDMI_AUTO_PWR_ON;
            
            pHDMI_DM_ObjCtrl->SendEvent(&SystemParms);
            
            TRACE_DEBUG((0,"Turn on HDMI When Standby mode"));
#endif
            IsFirstSystemStartup = FALSE;
        }
        else
        {
            //TRACE_DEBUG((0,"else HMI_Service_HDMI_StandbyCtrl ----"));
        }
#else
        //Set_HDMI_RST_Standby();
        
#if 0
        if (Is_HDMI_PlugIn() == TRUE && IsFirstSystemStartup == FALSE)
        {
            srv_parms.event = USER_EVENT_POWER;
            HMI_Service_EventSender( &srv_parms );

            IsFirstSystemStartup = TRUE;
            //return;
        }
#endif
        
#if 1        
        if ((Is_HDMI_PlugIn() == TRUE) && ( pASH_ObjCtrl->isHDMIPowerOff()))
        {
            //When standby mode, and detect the 5V
            //We need to turn on HDMI
            //Waiting for HDMI Task ready
#if (configSII_DEV_953x_TX_HPD_CTRL == 1)            
            if (pPowerHandle_ObjCtrl->power_is_hpd_from_tv() == TRUE)
            {
                // tv connected to 9533, we need to turn off the signal
                pPowerHandle_ObjCtrl->power_hpd_to_953X(FALSE);
                Is_mute_HPD = TRUE;
                TRACE_DEBUG((0, "MUTE HPD TO SII 9533 "));
            }
#endif            
            pPowerHandle_ObjCtrl->power_5v_ctrl(TRUE);
            
            TRACE_DEBUG((0,"Detect from MCU ---- plugin"));
#if 0            
            while (pHDMI_DM_ObjCtrl->Is_HDMI_task_ready() == FALSE)
            {
                TRACE_DEBUG((0,"plugin wait for hdmi task ready----"));
                vTaskDelay(1);
            }
#endif
            
            //Send power on event to HDMI Task 
            mSystemParams.sys_event = SYS_EVENT_HDMI_AUTO_PWR_ON;
            TRACE_DEBUG((0,"SYS_EVENT_HDMI_AUTO_PWR_ON "));
            pHDMI_DM_ObjCtrl->SendEvent(&mSystemParams );
            
            // TODO: the waiting is not a good approach, kaomin
            while(pASH_ObjCtrl->isHDMIPowerOff())
            {
                TRACE_DEBUG((0, "HDMI not work, please wait ..."));
                vTaskDelay(1);
            }
            
#if (configSII_DEV_953x_TX_HPD_CTRL == 1)
            if (Is_mute_HPD == TRUE)
            {
                vTaskDelay(1200);
                pPowerHandle_ObjCtrl->power_hpd_to_953X(pPowerHandle_ObjCtrl->power_is_hpd_from_tv());
                Is_mute_HPD = FALSE;
                TRACE_DEBUG((0, "DE-MUTE HPD TO SII 9533 "));
            }
#endif            
        }
        else if ((Is_HDMI_PlugIn() == FALSE) && (!pASH_ObjCtrl->isHDMIPowerOff()))
        {
            //When standby mode, and can't detect the 5V
            //We need to turn off HDMI
            //Waiting for HDMI Task ready
            TRACE_DEBUG((0,"Detect from MCU ---- off"));
#if 0
            while (pHDMI_DM_ObjCtrl->Is_HDMI_task_ready() == FALSE)
            {
                TRACE_DEBUG((0,"plugout wait for hdmi task ready----"));
                vTaskDelay(1);
            }
#endif            
                mSystemParams.sys_event = SYS_EVENT_HDMI_AUTO_PWR_OFF;
                TRACE_DEBUG((0,"SYS_EVENT_HDMI_AUTO_PWR_OFF "));
                pHDMI_DM_ObjCtrl->SendEvent(&mSystemParams );
                
                while(!pASH_ObjCtrl->isHDMIPowerOff())
                {
                    TRACE_DEBUG((0, "111 HDMI still work, please wait ..."));
                    vTaskDelay(1);
                }
            pPowerHandle_ObjCtrl->power_5v_ctrl(FALSE);
        }
#endif
#endif
    }
    else if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)//power on status
    {
        IsFirstSystemStartup = FALSE;
    }
}
#endif

static uint8 HMI_Service_InstrGetQueueNumber( void )
{
    return (uint8)uxQueueMessagesWaiting(mHMISrvClusion.serviceQueue.xQueue);
}

#if ( configSYS_FORCE_POWERUP == 1 ) // David, fix warning.
static void HMI_Service_ForcePowerup( void )
{
    xHMISrvEventParams event_parms = {xHMI_EVENT_APC, USER_EVENT_UNDEFINED };
    
    if( ( pPowerHandle_ObjCtrl->getSystemResetType() == RCC_FLAG_SFTRST ) && ( (POWER_STATE)pPowerHandle_ObjCtrl->get_backup_power_state() == POWER_ON ) )
    {
        event_parms.event = USER_EVENT_FORCE_POWER_UP; 
        HMI_Service_EventSender( &event_parms );    
    
        pPowerHandle_ObjCtrl->clrSystemResetType();
    }
}
#endif                

void HMI_ServiceUser( void *pvParameters )
{
    xHMISrvEventParams EventParams;
#if ( configSII_DEV_953x_PORTING == 1 )
    IsFirstSystemStartup = TRUE;
#endif

    for( ;; )
    {
#if ( configMonitorHeapSize == 1 )        
        free_heap_size = xPortGetFreeHeapSize();    
#endif
        
        switch( mHMISrvClusion.userState )
        {
            case TASK_SUSPENDED:
            {
                pSDM_ObjCtrl->Initialize( &mSystemParams ); /*Reloader user parameters from EEPROM*/
                mHMISrvClusion.userState = TASK_RUNING;
            }
                break;
            
            case TASK_READY:
            {
                if ( HMI_Service_InstrGetQueueNumber() != 0 )
                {
                    mHMISrvClusion.userState = TASK_RUNING;
                }
#if ( configSYS_FORCE_POWERUP == 1 ) // David, fix warning.
                HMI_Service_ForcePowerup( );
#endif
            }
                break;
        
            case TASK_RUNING:
            {
                if (HMI_Service_EventReceiver( &EventParams ) == TRUE )
                {
                    HMI_Service_handleCommand( &EventParams );
                    
                    mHMISrvClusion.userState = TASK_RUNING;
                }
            }
                break;

            case TASK_BLOCKED:
            {
                mHMISrvClusion.userState = TASK_READY;
            }
                break;
        }
            
    }
}

void HMI_AutoPowerControl_Task( void *pvParameters )
{
    xHMISrvEventParams srv_parms = {xHMI_EVENT_APC, USER_EVENT_UNDEFINED, 0};
    static uint16 APDTimeout;
    static uint16 MuteTimeout;

    APDTimeout = HMI_Service_getAPDTimeout(pSystemParams->input_src);
    MuteTimeout = HMI_Service_getMuteTimeout(pSystemParams->input_src);
        
    for( ;; )
    {
        if( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
        {
            APDTimeout = HMI_Service_getAPDTimeout(pSystemParams->input_src);

            switch( pSystemParams->op_mode )
            {
                case MODE_FACTORY:
                {
                    /*TODO*/
                    srv_parms.event_id = xHMI_EVENT_UART; /*ir m2489 or vcp factory command*/
                }
                    break;
                case MODE_DEMO_1:
                {
                    HMI_Service_setDemo1n3_SysParams( pSystemParams->op_mode );
                    pBTN_ObjCtrl->register_demo_1n3_cmd();
                }
                    break;
                    
                case MODE_DEMO_2:
                {
                    HMI_Service_setDemo1n3_SysParams( pSystemParams->op_mode );
                    pBTN_ObjCtrl->register_demo_2_cmd();
                }
                    break;
                    
                case MODE_DEMO_3:
                {
                    HMI_Service_setDemo1n3_SysParams( pSystemParams->op_mode );
                    pBTN_ObjCtrl->register_demo_1n3_cmd();
                }
                    break;
            }

            if ( pSystemParams->op_mode != MODE_USER )
            {
                srv_parms.event = USER_EVENT_POWER;
                HMI_Service_EventSender( &srv_parms );
            }
        
        }
        else if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
        {
            if ( pSystemParams->apd == TRUE )
            {
                if ( pASH_ObjCtrl->IsSignalAvailable() == 0)
                {
                    if ( APDTimeout < 1 )
                    {
                        APDTimeout = HMI_Service_getAPDTimeout(pSystemParams->input_src);
                        srv_parms.event = USER_EVENT_POWER;
                        HMI_Service_EventSender( &srv_parms );
                    }
                    else
                    {
                        APDTimeout --;
                        TRACE_DEBUG((0, "APD COUNT = %d", APDTimeout ));
                    }
                }
                else
                {
                    APDTimeout = HMI_Service_getAPDTimeout(pSystemParams->input_src);
                }
            }
            else
            {
                APDTimeout = HMI_Service_getAPDTimeout(pSystemParams->input_src);
            }


            if(pSystemParams->mute == TRUE)
            {
                if(MuteTimeout > 0)
                {
                    MuteTimeout--;
                    //TRACE_DEBUG((0, "MUTE COUNT = %d", MuteTimeout ));
                }
                else
                {
                    MuteTimeout = HMI_Service_getMuteTimeout(pSystemParams->input_src);
                    srv_parms.event = USER_EVENT_POWER;
                    HMI_Service_EventSender( &srv_parms );
                }
            }
            else
            {
                MuteTimeout = HMI_Service_getMuteTimeout(pSystemParams->input_src);
            }
        }
        
        vTaskDelay(HMI_APC_TIME_TICK);
    }
}

void HMI_AutoPowerControl_CreateTask( void )
{
#if ( configAPP_APD == 1 )
    if ( xTaskCreate( 
        HMI_AutoPowerControl_Task, 
        ( portCHAR * ) "HMI_APC", 
        STACK_SIZE, 
        NULL, 
        tskHMI_APC_PRIORITY, 
        &mHMISrvClusion.apcTaskHandle ) != pdPASS )
    {
        vTaskDelete( mHMISrvClusion.apcTaskHandle );
    }
    else
    {
        vTaskSuspend( mHMISrvClusion.apcTaskHandle );
    }
#endif     
}

static void HMI_AutoPowerControl_TaskCtl(bool val)
{
    if( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( mHMISrvClusion.apcTaskHandle ) != pdPASS ) /*task is working*/
        {
            TRACE_DEBUG((0, "SUSPEND: HMI_AutoPowerControl_TaskCtl"));
            vTaskSuspend( mHMISrvClusion.apcTaskHandle );
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( mHMISrvClusion.apcTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( mHMISrvClusion.apcTaskHandle );
            TRACE_DEBUG((0, "RESUME: HMI_AutoPowerControl_TaskCtl"));
        }
    }
}

/*******************************************************************************
 * Auto input detect
 ******************************************************************************/
static void HMI_AutoInputDetect_CreateTask(void)
{
#if ( configAPP_AID == 1 )
    if ( xTaskCreate( 
                    HMI_AutoInputDetect_task, 
                    ( portCHAR * ) "Auto_Input_Detect_Manager", 
                    (STACK_SIZE), 
                    NULL, 
                    tskAID_MANAGER_PRIORITY, 
                    &mHMISrvClusion.aidTaskHandle ) != pdPASS )
    {
        vTaskDelete( mHMISrvClusion.aidTaskHandle  );
        TRACE_ERROR((0, "Auto_Input_Detect_Manager task create failure " ));
    }
	
    mProcessCnt = 0xffff;	// Used for initial Auto Input Detect
#endif       
}

static void HMI_AutoInputDetect_TaskCtl(bool val)
{
    if( val == BTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( mHMISrvClusion.aidTaskHandle ) != pdPASS ) /*task is working*/
        {
            TRACE_DEBUG((0, "SUSPEND: Auto_Input_Detect_Manager"));
            
            HMI_AutoPowerControl_TaskCtl(BTASK_RESUME);
            vTaskSuspend( mHMISrvClusion.aidTaskHandle );
            //HMI_AutoPowerControl_TaskCtl(BTASK_RESUME);
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( mHMISrvClusion.aidTaskHandle ) == pdPASS ) /*task is not working*/
        {
            AID_timeout = AID_TIMEOUT;
            mSignalAvaiableCnt = 0;
            mSignalUnavaiableCnt = 0;
    mProcessCnt = 0xffff;	// Used for initial Auto Input Detect
            HMI_AutoPowerControl_TaskCtl(BTASK_SUSPENDED);

#if 0
            if (  pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
            {
                //mSystemParams.input_src = AUDIO_SOURCE_MAX;
            mProcessCnt = 0;  
                aid_parms.event = USER_EVENT_START_AUTO_SEL_SRC;//USER_EVENT_AUTO_SEL_SRC;/* Switch source*/
                TRACE_DEBUG((0, "HMI_Service_EventSender: USER_EVENT_START_AUTO_SEL_SRC"));
                HMI_Service_EventSender( &aid_parms );
                vTaskDelay(TASK_MSEC2TICKS(300));
            }

            vTaskResume( mHMISrvClusion.aidTaskHandle );
            TRACE_DEBUG((0, "RESUME: Auto_Input_Detect_Manager"));
            
            if (  pPowerHandle_ObjCtrl->get_power_state() != POWER_ON )
            {
                aid_parms.event = USER_EVENT_POWER; /* Poweron*/
                HMI_Service_EventSender( &aid_parms );
            }
#else
            vTaskResume( mHMISrvClusion.aidTaskHandle );
#endif            
        }
    }
}

static void HMI_AutoInputDetect_task( void *pvParameters )
{
    uint8 SignalAvaliableThrehold; 

    for(;;)
    {
        if( pHMISrvClusion->userState == TASK_RUNING)
        {
            /*Turn off when flag is not set*/
            if ( pSystemParams->auto_input_detect == FALSE || (pSystemParams->op_mode != MODE_USER ))
            {
                HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
            }
            
            if (  pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
            {
                aid_parms.event = USER_EVENT_POWER; /* Poweron*/
                HMI_Service_EventSender( &aid_parms );
            }
            else if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON ) 
            {
                UserParmsAutoPowerDown = pSystemParams->auto_input_detect;

                /* Terminate the auto detection when it is not User mode. Tony140108 */
                if ( pSystemParams->op_mode != MODE_USER ) 
                {
                    HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
                }

                /*If auto power down condition is changed, reset the time stamp */
                if (LastUserParmsAutoPowerDown!=UserParmsAutoPowerDown )
                {
                    AID_timeout = AID_TIMEOUT;
                }
                
                /* Update current AutoPower down flag */
                LastUserParmsAutoPowerDown = UserParmsAutoPowerDown;

                if ( AID_timeout < 1 )
                {      
                    if ( pSystemParams->apd == TRUE )
                    {
                        aid_parms.event = USER_EVENT_AUTO_INPUT_DETECT_CLEAN; /*Clean Flag*/
                        HMI_Service_EventSender( &aid_parms );

                        TRACE_DEBUG((0, "  Auto detection 15 mins auto power down   !! ")); 
                        AID_timeout = AID_TIMEOUT;

                        if (  pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
                        {
                            aid_parms.event = USER_EVENT_POWER; /* Powerdown*/
                            HMI_Service_EventSender( &aid_parms );
                        }
                        
                        //HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
                    }
                    else
                    {
                        AID_timeout = AID_TIMEOUT;
                        TRACE_DEBUG((0, "  Auto detection 15 mins reset   !! "));
                    }
                }
                else
                {
                    AID_timeout --;
                    //TRACE_DEBUG((0, "  AID_timeout =%X ",AID_timeout));
                }

                if ( mProcessCnt == 0xffff )
                {
	                mSignalAvaiableCnt = 0;
	                mSignalUnavaiableCnt = 0;
	                mProcessCnt = 0;  

	                aid_parms.event = USER_EVENT_START_AUTO_SEL_SRC;/* Switch source*/
	                HMI_Service_EventSender( &aid_parms );
                }
		else
		{
			mProcessCnt++;
		
	                switch( pSystemParams->input_src ) 
	                {
	                    case AUDIO_SOURCE_AUX1:
	                    case AUDIO_SOURCE_AUX2:
	                    {
	                        SignalAvaliableThrehold = AnalogThrehold;
	                    }
	                    break;

	                    case AUDIO_SOURCE_COAXIAL:
	                    case AUDIO_SOURCE_OPTICAL:
	                    {
	                        SignalAvaliableThrehold = DigitalThrehold;
	                    }
	                    break;

	                    default:
	                    {
	                        SignalAvaliableThrehold = AnalogThrehold;
	                    }
	                    break;
	                }

	                if ( mProcessCnt > SignalAvaliableThrehold )
	                {
	                    if( pASH_ObjCtrl->IsSignalAvailable() == 1)
	                    {
	                        mSignalAvaiableCnt++;
	                        if ( mSignalAvaiableCnt >= CheckSignalAvaiableTime)
	                        {
	                            aid_parms.event = USER_EVENT_AUTO_INPUT_DETECT_CLEAN; 
	                            HMI_Service_EventSender( &aid_parms );
	                            
	                            HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
	                        } 
	                    }
	                    else
	                    {
	                        //TRACE_DEBUG((0,"mProcessCnt=%X,SignalAvaliableThrehold=%X",mProcessCnt,SignalAvaliableThrehold));
	                        mSignalUnavaiableCnt++;
	                        
	                        if (mSignalUnavaiableCnt >= CheckSignalUnAvaiableTime)
	                        {   
	                            mSignalAvaiableCnt = 0;
	                            mSignalUnavaiableCnt = 0;
	                            mProcessCnt = 0;  
				
	                            aid_parms.event = USER_EVENT_AUTO_SEL_SRC;/* Switch source*/
	                            HMI_Service_EventSender( &aid_parms );
	                        }
	                    }    
	                }
		}
            }    
        }
        vTaskDelay(HMI_AID_TIME_TICK);
    }     
}

#if (configSII_DEV_953x_PORTING == 1)
void HMI_Standby_Monitor_Task(void *pvParameters)
{
    for(;;)
    {
        HMI_Service_HDMI_StandbyCtrl();

        vTaskDelay(200);
    }
}

void HMI_StandbyCTL_CreateTask( void )
{
    if ( xTaskCreate( 
        HMI_Standby_Monitor_Task, 
        ( portCHAR * ) "HMI_STANDBY_CTL", 
        (STACK_SIZE), 
        NULL, 
        tskHMI_STANDBY_CTRL_PRIORITY, 
        &mHMIStandbyCtrl.StandbyCtrlTaskHandle) != pdPASS )
    {
        vTaskDelete( mHMIStandbyCtrl.StandbyCtrlTaskHandle );
    }
}

#endif

void HMI_Service_CreateTask( void )
{
    mHMISrvClusion.serviceQueue.xQueue = xQueueCreate( HMI_QUEUE_LENGTH,(unsigned portBASE_TYPE )(sizeof(xHMISrvEventParams)/sizeof(uint8)) );

    mHMISrvClusion.serviceQueue.xBlockTime = portMAX_DELAY;

    if ( xTaskCreate( 
        HMI_ServiceUser, 
        ( portCHAR * ) "HMI_SRV_USER", 
        (STACK_SIZE), 
        NULL, 
        tskHMI_SERVICE_PRIORITY, 
        &mHMISrvClusion.userTaskHandle ) != pdPASS )
    {
        vTaskDelete( mHMISrvClusion.userTaskHandle );
        mHMISrvClusion.xOS_ErrId = xOS_TASK_CREATE_FAIL;
    }

    if( mHMISrvClusion.serviceQueue.xQueue == NULL )
    {
        mHMISrvClusion.xOS_ErrId = xOS_TASK_QUEUE_CREATE_FAIL;
    }

    pPowerHandle_ObjCtrl->initialize(); 
    pASH_ObjCtrl->CreateTask( );
    pUDM_ObjCtrl->CreateTask(); 
    pBTHandle_ObjCtrl->CreateTask();      /* Create BTHandle task */
    pRFHandle_ObjCtrl->CreateTask();      /* Create RFHandle task */
    pCCKManage_ObjCtrl->CreateTask();
    pUSB_MMP_ObjCtrl->CreateTask();
    USBMusicManager_RegisterSetDemoMode2Timeout(HMI_Service_setDemo2_Timeout); /* register set Demo2 timeout function */

    TRACE_DEBUG((0, "EEpromDefaultParams[0] = %d ",EEpromDefaultParams[0]));    /*for code size 232kb */

    HMI_AutoPowerControl_CreateTask();
    HMI_AutoInputDetect_CreateTask();

    HMI_DemoMode1_3_TimeOutDetectCreateTask();

#if (configSII_DEV_953x_PORTING == 1)
    HMI_StandbyCTL_CreateTask();
#endif    
}

static xEventTypeAssort HMI_Service_Event_Assort( xHMIUserEvents cur_events, xHMIUserEvents last_events )
{
    xEventTypeAssort ret = EVENT_ASSORT_NO_NEED_FIRST_CONFIRM;

    switch ( cur_events )
    {
        case USER_EVENT_SEL_SRC:
#if (0) // Mask for new remote (v1.3)
        case USER_EVENT_AUTO_SEL_SRC:
#endif
        {
            if ( (last_events == USER_EVENT_SEL_SRC) || (last_events == USER_EVENT_AUTO_SEL_SRC) 
#if ( configSII_DEV_953x_PORTING == 1 ) //CEC event will effect input comfirm not following UI spec. Angus 2014/12/1
            ||(last_events >= USER_EVENT_CEC_POWER_ON && last_events <= USER_EVENT_CEC_SET_VOLUME_MUTE)
#endif
            )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;                   
#if (0) // Mask for new remote (v1.3)
        case USER_EVENT_SEL_AUX1:
        case USER_EVENT_SEL_AUX2:
        case USER_EVENT_SEL_COAXIAL:
        case USER_EVENT_SEL_OPTICAL:
        case USER_EVENT_SEL_USB_PLAYBACK:
        case USER_EVENT_SEL_BT:
#if ( configSII_DEV_953x_PORTING == 1 )
        case USER_EVENT_SEL_HDMI_IN:
        case USER_EVENT_SEL_HDMI_ARC:
#endif 
        {
#if ( configSII_DEV_953x_PORTING == 1 )
            if ((last_events == USER_EVENT_SEL_AUX1) || (last_events == USER_EVENT_SEL_AUX2) ||\
                (last_events == USER_EVENT_SEL_COAXIAL) ||(last_events == USER_EVENT_SEL_OPTICAL)||\
                (last_events == USER_EVENT_SEL_USB_PLAYBACK) ||(last_events == USER_EVENT_SEL_BT)||\
                (last_events == USER_EVENT_SEL_HDMI_IN) ||(last_events == USER_EVENT_SEL_HDMI_ARC)) 
#else
            if ((last_events == USER_EVENT_SEL_AUX1) || (last_events == USER_EVENT_SEL_AUX2) ||\
                (last_events == USER_EVENT_SEL_COAXIAL) ||(last_events == USER_EVENT_SEL_OPTICAL)||\
                (last_events == USER_EVENT_SEL_USB_PLAYBACK) ||(last_events == USER_EVENT_SEL_BT)) 
#endif
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
#endif
        case USER_EVENT_BASS_UP:
        case USER_EVENT_BASS_DN:
        {
            if ( (last_events == USER_EVENT_BASS_UP ) || (last_events == USER_EVENT_BASS_DN) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
                    
        case USER_EVENT_TREBLE_UP:
        case USER_EVENT_TREBLE_DN:
        {
            if ( (last_events == USER_EVENT_TREBLE_UP ) || (last_events == USER_EVENT_TREBLE_DN) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
                    
        case USER_EVENT_SUB_UP:
        case USER_EVENT_SUB_DN:
        {
            if ( (last_events == USER_EVENT_SUB_UP ) || (last_events == USER_EVENT_SUB_DN) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
                    
        case USER_EVENT_CENTER_UP:
        case USER_EVENT_CENTER_DN:
        {
            if ( (last_events == USER_EVENT_CENTER_UP ) || (last_events == USER_EVENT_CENTER_DN) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
                    
        case USER_EVENT_REAR_UP:
        case USER_EVENT_REAR_DN:
        {
            if ( (last_events == USER_EVENT_REAR_UP ) || (last_events == USER_EVENT_REAR_DN) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
                    
        case USER_EVENT_AV_DELAY_UP:
        case USER_EVENT_AV_DELAY_DN:
        {
            if ( (last_events == USER_EVENT_AV_DELAY_UP ) || (last_events == USER_EVENT_AV_DELAY_DN) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
                    
        case USER_EVENT_BALANCE_UP:
        case USER_EVENT_BALANCE_DN:
        {
            if ( (last_events == USER_EVENT_BALANCE_UP ) || (last_events == USER_EVENT_BALANCE_DN) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;
                    
        case USER_EVENT_APD_ON:
        case USER_EVENT_APD_OFF:
        {
            if ( (last_events == USER_EVENT_APD_ON)|| (last_events == USER_EVENT_APD_OFF) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;

        case USER_EVENT_SRS_TRUVOL_ON:
        case USER_EVENT_SRS_TRUVOL_OFF:
        {
            if ( (last_events == USER_EVENT_SRS_TRUVOL_ON ) || (last_events == USER_EVENT_SRS_TRUVOL_OFF) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;

        case USER_EVENT_SRS_TSHD_ON:
        case USER_EVENT_SRS_TSHD_OFF:
        {
            if ( (last_events == USER_EVENT_SRS_TSHD_ON ) || (last_events == USER_EVENT_SRS_TSHD_OFF) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;

        case USER_EVENT_NIGHT_MODE_ON:
        case USER_EVENT_NIGHT_MODE_OFF:
        {
            if ( (last_events == USER_EVENT_NIGHT_MODE_ON ) || (last_events == USER_EVENT_NIGHT_MODE_OFF) )
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;

        default:
            ret = EVENT_ASSORT_NO_NEED_FIRST_CONFIRM;
    }
    
    return ret;
}

static xHMISystemParams HMI_Service_getSystemParams( void )
{
    return mSystemParams;
}


static uint16 HMI_Service_getAPDTimeout( AUDIO_SOURCE  CurrSrc )
{ 
    uint16 timeout;
    
    if ( (CurrSrc == AUDIO_SOURCE_COAXIAL) ||(CurrSrc == AUDIO_SOURCE_OPTICAL) )
        timeout = HMI_APD_TIMEOUT_DIGITAL;
    else
        timeout = HMI_APD_TIMEOUT;

    return timeout;
}

static uint16 HMI_Service_getMuteTimeout( AUDIO_SOURCE  CurrSrc )
{ 
    uint16 timeout;
    
    if ( (CurrSrc == AUDIO_SOURCE_COAXIAL) ||(CurrSrc == AUDIO_SOURCE_OPTICAL) )
        timeout = (HMI_APD_TIMEOUT_DIGITAL*4)*4;
    else
        timeout = (HMI_APD_TIMEOUT*4)*4;

    return timeout;
}

#if ( configSII_DEV_953x_PORTING == 1 )
////////////////////////////////////////////////////////////////////////////////////////
//    OnOff: chekc if power on or power off event is available
//    IsHDMI: is current source is HDMI
//    Return TRUE when power on/off event is valide; FALSE when
////////////////////////////////////////////////////////////////////////////////////////
static bool HMI_Service_IsPowerKeyAvailable( bool OnOff, bool IsHDMI)
{
    bool result = FALSE;
    //TRACE_DEBUG((0, "pPowerHandle_ObjCtrl->get_power_state(): %d", pPowerHandle_ObjCtrl->get_power_state() ));
    //TRACE_DEBUG((0, "pPowerHandle_ObjCtrl->get_power_handle_state(): %d", pPowerHandle_ObjCtrl->get_power_handle_state() ));
    //TRACE_DEBUG((0, "pASH_ObjCtrl->getHDMIPowerStatus(): %d", pASH_ObjCtrl->getHDMIPowerStatus() ));
    if ( OnOff )    //check if system can accept power on event
    {
        if (pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF)
        {
            //TRACE_DEBUG((0, "TRUE>>pPowerHandle_ObjCtrl->get_power_state(): POWER_OFF" ));
            if (pPowerHandle_ObjCtrl->get_power_handle_state() == POWER_HANDLE_IDLE )
            {
                //TRACE_DEBUG((0, "TRUE>>pPowerHandle_ObjCtrl->get_power_handle_state(): POWER_HANDLE_IDLE" ));
                if (IsHDMI)
                {
                    if ( pASH_ObjCtrl->getHDMIPowerStatus() == xHMI_POWERSTATUS_STANDBY)    //app.powerState = APP_POWERSTATUS_STANDBY
                    {
                        //TRACE_DEBUG((0, "TRUE>>pASH_ObjCtrl->getHDMIPowerStatus: 1 (xHMI_POWERSTATUS_STANDBY)" ));
                        result = TRUE;
                    }
                    else
                    {
                        result = FALSE;
                    }
                }
                else
                {
                    result = TRUE;
                }
            }
            else
            {
                result = FALSE;
            }
        }
        else
        {
                result = FALSE;
        }   
    }
    else
    {
        if (pPowerHandle_ObjCtrl->get_power_state() == POWER_ON)
        {
            //TRACE_DEBUG((0, "FALSE>>pPowerHandle_ObjCtrl->get_power_state(): POWER_ON" ));
            if (pPowerHandle_ObjCtrl->get_power_handle_state() == POWER_HANDLE_IDLE )
            {
                //TRACE_DEBUG((0, "FALSE>>pPowerHandle_ObjCtrl->get_power_handle_state(): POWER_HANDLE_IDLE" ));
                if (IsHDMI)
                {
                    if ( pASH_ObjCtrl->getHDMIPowerStatus() == xHMI_POWERSTATUS_ON )    //app.powerState = APP_POWERSTATUS_ON
                    {
                        //TRACE_DEBUG((0, "FALSE>>pASH_ObjCtrl->getHDMIPowerStatus: 0 (APP_POWERSTATUS_ON)" ));
                        result = TRUE;
                    }
                    else
                    {
                        result = FALSE;
                    }
                }
                else
                {
                    result = TRUE;
                }
            }
            else
            {
                result = FALSE;
            }
        }
        else
        {
            result = FALSE;
        }
    }
    TRACE_DEBUG((0, "HMI_Service_IsPowerKeyAvailable(): %d",result ));
    return result;
}
#endif

//************************************************************************************
//  Angus added for detect demo mode 1 & 3 timeout(2014/11/6)
//************************************************************************************
static void HMI_DemoMode1_3_TimeOutDetectCreateTask(void)
{
    if ( xTaskCreate( 
             HMI_DemoMode1_3_TimeOutDetectTask, 
             ( portCHAR * ) "HMI_DemoMode1_3_TimeOutDetect", 
             (STACK_SIZE), 
             NULL, 
             tskDemo1_3_TIMEOUT_DETECT_PRIORITY, 
             &mDemo1_3_TimeOutDetect.DetectHandle ) != pdPASS )
     {
        vTaskDelete( mDemo1_3_TimeOutDetect.DetectHandle );
     }
     else
     {
        /* If SB is on Demo Mode 1,3 : TaskResume , else :TaskSuspend */
         vTaskSuspend( mDemo1_3_TimeOutDetect.DetectHandle );
     }

}

static void HMI_DemoMode1_3_TimeOutDetect_TaskCtrl( bool val )
{
    /* Setting default value*/
    Demo1_3_TimeOutCnt = 0;
    Demo1_3_TimeOut = FALSE;
    if(val == FALSE)
    {
        if( xTaskIsTaskSuspended( mDemo1_3_TimeOutDetect.DetectHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( mDemo1_3_TimeOutDetect.DetectHandle );
        }
    }
    else
    {
        if( xTaskIsTaskSuspended( mDemo1_3_TimeOutDetect.DetectHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( mDemo1_3_TimeOutDetect.DetectHandle );
        }
    }
}

static void HMI_DemoMode1_3_TimeOutDetectTask( void *pvParameters )
{
    static bool signal = FALSE;
    for(;;)
    {
        if((mSystemParams.op_mode == MODE_DEMO_1 && mSystemParams.input_src == DEMO_1_INPUT_SRC) ||
            (mSystemParams.op_mode == MODE_DEMO_3 && mSystemParams.input_src == DEMO_3_INPUT_SRC))
        {
            if(pASH_ObjCtrl->IsSignalAvailable() == 0)
            {
                if(Demo1_3_TimeOut == FALSE)
                {
                    Demo1_3_TimeOutCnt++;
                } 
            }
            else
            {
                if(Demo1_3_TimeOut)
                {
                    signal = TRUE;
                    HMI_Service_setDemo1_3_Timeout(signal);
                    Demo1_3_TimeOut = FALSE;
                }
                Demo1_3_TimeOutCnt = 0;
            }

            
            if(Demo1_3_TimeOutCnt >= 2)
            {
                if(Demo1_3_TimeOut == FALSE)
                {
                    signal =  FALSE;
                    HMI_Service_setDemo1_3_Timeout(signal);
                    Demo1_3_TimeOutCnt = 0;
                    Demo1_3_TimeOut = TRUE;            
                }
            }
        }
        vTaskDelay(TASK_MSEC2TICKS(1000));
    }
}

void HMI_CecVolCtrlDirect(xHMIUserEvents event)
{
    switch(event)
    {
    case USER_EVENT_CEC_SET_VOLUME_UP:
        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
        {
            break;
        }
        if ( pSystemParams->master_gain < master_gain_limit.max_gain )
        {
            mSystemParams.master_gain++;// = event.params;
        }
        else
        {
            mSystemParams.master_gain = master_gain_limit.max_gain;
        }
        
        if( pSystemParams->mute == SOUND_MUTE )
        {
            mSystemParams.mute = SOUND_DEMUTE;
            mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET; /*set audio system event*/
            pASH_ObjCtrl->SendEvent( &mSystemParams );
        }
        
        //TRACE_DEBUG((0, "<RAS> HMI vol up =%d", mSystemParams.master_gain));
        
        mSystemParams.sys_event = SYS_EVENT_VOLUME_SET; /*set audio system event*/
        pSDM_ObjCtrl->SendEvent( &mSystemParams );
        pASH_ObjCtrl->SendEvent( &mSystemParams );   
        pUDM_ObjCtrl->SendEvent( &mSystemParams );
        //SyncSacVolume(mSystemParams);
        break;

    case USER_EVENT_CEC_SET_VOLUME_DOWN:
        if ( (mSystemParams.op_mode != MODE_USER) && (mSystemParams.op_mode != MODE_FACTORY) )
        {
            break;
        }
        if ( pSystemParams->master_gain > master_gain_limit.min_gain )
        {
            //TRACE_DEBUG((0, "USER_EVENT_CEC_SET_VOLUME_DOWN = %d", event.params));
            mSystemParams.master_gain--;// = event.params;
        }
        else
        {
            mSystemParams.master_gain = master_gain_limit.min_gain;
        }
        
        #if 0
        if( pSystemParams->mute == SOUND_MUTE )
        {
            mSystemParams.mute = SOUND_DEMUTE;
            mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET; /*set audio system event*/
            pASH_ObjCtrl->SendEvent( &mSystemParams );
        }
        #endif
        //TRACE_DEBUG((0, "<RAS> HMI vol down =%d", mSystemParams.master_gain));
        mSystemParams.sys_event = SYS_EVENT_VOLUME_SET; /*set audio system event*/
        pSDM_ObjCtrl->SendEvent( &mSystemParams );
        pASH_ObjCtrl->SendEvent( &mSystemParams );   
        pUDM_ObjCtrl->SendEvent( &mSystemParams );
        //SyncSacVolume(mSystemParams);
        break;

    default:
        break;
    }
}
