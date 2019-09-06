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

#if ( configSTM32F411_PORTING == 1 )
#include "ChannelCheckManager.h"
#endif
#include "StorageDeviceManager.h"
#include "FactoryCommandHandler.h"
#include "UIDeviceManager.h"
#include "USBMusicManager.h"

#include "HMI_Service.h"

//_________________________________________________________________________________________
#define HMI_QUEUE_LENGTH 20
#define HMI_TIME_TICK TASK_MSEC2TICKS(1)
#define HMI_APC_TIME_TICK TASK_MSEC2TICKS(1000)
#define HMI_CMD_TIME_UP_MSEC TASK_MSEC2TICKS(15000) 
#define HMI_APD_TIMEOUT (60*15)
#define EVENT_FIRST_PRESSED_OR_TIMEOUT TRUE

#define CheckSignalAvaiableTime   1
#define CheckSignalUnAvaiableTime 1
#define HMI_AID_TIME_TICK TASK_MSEC2TICKS(500)
#define AID_TIMEOUT (60*15)*2
#define AnalogThrehold  6 /* check signal after 3 sec */ 
#define DigitalThrehold 4 /* after 2 sec */
#define AutoDetectionLastSource AUDIO_SOURCE_OPTICAL

//_________________________________________________________________________________________
typedef struct HMI_SERVICE_PARAMETERS
{
    xTaskHandle userTaskHandle;
    xTaskHandle apcTaskHandle;
    xTaskHandle aidTaskHandle;
    TaskHandleState userState;
    xQueueParameters serviceQueue;
    xOS_TaskErrIndicator xOS_ErrId;
} xHMISrvClusion;

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

static xHMISystemParams mSystemParams = { 
    SYS_EVENT_NULL,
    MODE_USER,    
    DEFAULT_APD,
    DEFAULT_INPUT_SRC,
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
    DEFAULT_AID,
    DEFAULT_CEC_SAC,
    DEFAULT_CEC_ARC
};

static const xHMISystemParams *pSystemParams = &mSystemParams;
static xHMISrvMasterGainLimit master_gain_limit = { MASTER_GAIN_MIN, MASTER_GAIN_MAX };
static xHMISrvClusion mHMISrvClusion;
static const xHMISrvClusion *pHMISrvClusion =&mHMISrvClusion;
static xBTHandleCommand mBTHandleCommand;

static xHMISrvEventParams aid_parms = {xHMI_EVENT_AID, USER_EVENT_UNDEFINED };
static uint32 mProcessCnt;
static uint8 mSignalAvaiableCnt;
static uint8 mSignalUnavaiableCnt;
static bool LastUserParmsAutoPowerDown = TRUE;
static bool UserParmsAutoPowerDown = TRUE;
static uint16 AID_timeout = AID_TIMEOUT;

#define configMonitorHeapSize 1
#if ( configMonitorHeapSize == 1 )        
size_t free_heap_size = 0;
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
//_________________________________________________________________________________________

extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
#if ( configSTM32F411_PORTING == 1 )
extern USB_MUSIC_MANAGE_OBJECT *pUSB_MMP_ObjCtrl;
#endif
extern BT_HANDLE_OBJECT *pBTHandle_ObjCtrl;
extern RF_HANDLE_OBJECT *pRFHandle_ObjCtrl;

extern BTN_CONF_OBJECT *pBTN_ObjCtrl;

#if ( configSTM32F411_PORTING == 1 )
extern CCK_MANAGE_OBJECT *pCCKManage_ObjCtrl;
#endif
extern AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl;
extern FACTORY_CMD_HANDLE_OBJECT *pFMD_ObjCtrl;
extern UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl;
extern IR_CMD_PROGRAM_OBJECT *pIR_PrgRemote_ObjCtrl;
extern STORAGE_DEVICE_MANAGER_OBJECT *pSDM_ObjCtrl;

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
#if ( configSTM32F411_PORTING == 1 )
    if (pPowerHandle_ObjCtrl == NULL 
        || pBTHandle_ObjCtrl == NULL
        || pASH_ObjCtrl == NULL
        || pUDM_ObjCtrl == NULL
        || pUSB_MMP_ObjCtrl == NULL
        )
    {
        return;
    }
#endif    
    pPowerHandle_ObjCtrl->power_toggle();
    //pBTHandle_ObjCtrl->power_ctl();    // No need to do this here. It is included in BT_HANDLE_RESET_BT event when execute _RST_DEF_SETTING

    if (!b_pwr_on)
    {
        if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
        {
            mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;           
            pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )         
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);          
#endif
        }
    }
    else
    {
        if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
        {
            mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;                 
            pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);     
#endif
        }
    }
}

static void _RST_DEF_SETTING()
{     
#if ( configSTM32F411_PORTING == 1 )  
    if (pBTN_ObjCtrl == NULL 
       || pASH_ObjCtrl == NULL
       )
    {
        return;
    }
#endif
    mBTHandleCommand = BT_HANDLE_RESET_BT;
    pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
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
    mSystemParams.cec_sac= DEFAULT_CEC_SAC;
    mSystemParams.cec_arc= DEFAULT_CEC_ARC;
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
    mSystemParams.cec_sac= DEFAULT_CEC_SAC;
    mSystemParams.cec_arc= DEFAULT_CEC_ARC;
    
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
    mSystemParams.master_gain = 10; /* usb timeout volume as 10 */
    mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event */
#if ( configSTM32F411_PORTING == 1)      
    pASH_ObjCtrl->SendEvent( &mSystemParams );
#endif    
}

static bool HMI_Service_EventSender( void *params ) 
{
    xHMISrvEventParams* pParams = ( xHMISrvEventParams *)params;

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

static void HMI_Service_sendCommand( xHMIUserEvents event )
{
    switch( event )
    {
        case USER_EVENT_CEC_POWER_ON:
        {
            if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON || pPowerHandle_ObjCtrl->get_power_state() == POWER_UP )
                break;
        }

        case USER_EVENT_POWER:
        {
            pPowerHandle_ObjCtrl->power_toggle();  
            
#if ( configSTM32F411_PORTING == 1 )  
            pBTHandle_ObjCtrl->power_ctl();
#endif       
            if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
            {
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN; 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )   
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);

                if (pSystemParams->auto_input_detect == TRUE)
                {    
                    HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
                }  
#endif                
            }
            else if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
            {
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_UP;
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );                   
#if ( configSTM32F411_PORTING == 1 ) 
                if ( event == USER_EVENT_CEC_POWER_ON )
                mSystemParams.sys_event = SYS_EVENT_CEC_SYSTEM_UP;   
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
   

                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);

                if (pSystemParams->auto_input_detect == TRUE)
                {    
                    HMI_AutoInputDetect_TaskCtl(BTASK_RESUME);
                }
 #endif                        
            }
        }
            break;

        case USER_EVENT_APD_TOOGLE:
        {
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;

            if ( pSystemParams->apd == FALSE )
            {
                mSystemParams.apd = TRUE;
            }
            else
            {
                mSystemParams.apd = FALSE;
            }
            pSDM_ObjCtrl->SendEvent( &mSystemParams);          
        }
            break;

        case USER_EVENT_APD_ON:
        {
            if ( pSystemParams->apd == FALSE )
            {
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                mSystemParams.apd = TRUE;               
                pSDM_ObjCtrl->SendEvent( &mSystemParams);          
            }
        }
            break;

        case USER_EVENT_APD_OFF:
        {
            if ( pSystemParams->apd == TRUE )
            {
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                mSystemParams.apd = FALSE;                
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        }
            break;

        case USER_EVENT_SEL_SRC:
        case USER_EVENT_AUTO_SEL_SRC: /*For auto detection*/
        {
            if (event==USER_EVENT_SEL_SRC)
            {
                if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
                {
                    mSystemParams.auto_input_detect = FALSE;                    
                    pSDM_ObjCtrl->SendEvent( &mSystemParams);                 
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
            pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            if ( mSystemParams.input_src == AUDIO_SOURCE_BLUETOOTH )
            {
                mBTHandleCommand = BT_HANDLE_ON;
                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
            }
            else
            {
                mBTHandleCommand = BT_HANDLE_OFF;
                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
            }          
        }
            break;

        case USER_EVENT_SEL_AUX1:
        {         
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        
            mSystemParams.input_src = AUDIO_SOURCE_AUX1;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            mBTHandleCommand = BT_HANDLE_OFF;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);       
        }
            break;

        case USER_EVENT_SEL_AUX2:
        {        
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }        
            
            mSystemParams.input_src = AUDIO_SOURCE_AUX2;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            mBTHandleCommand = BT_HANDLE_OFF;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);       
        }
            break;

        case USER_EVENT_SEL_COAXIAL:
        {      
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
            
            mSystemParams.input_src = AUDIO_SOURCE_COAXIAL;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            mBTHandleCommand = BT_HANDLE_OFF;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);        
        }
            break;    
            
        case USER_EVENT_SEL_OPTICAL:
        {        
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        
            mSystemParams.input_src = AUDIO_SOURCE_OPTICAL;
            
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            mBTHandleCommand = BT_HANDLE_OFF;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);       
        }
            break;

        case USER_EVENT_SEL_USB_PLAYBACK:
        {     
            if (pSystemParams->auto_input_detect == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.auto_input_detect = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        
            mSystemParams.input_src = AUDIO_SOURCE_USB_PLAYBACK;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif            
            mBTHandleCommand = BT_HANDLE_OFF;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);          
        }
            break;

        case USER_EVENT_SEL_BT:
        {
            if (pSystemParams->apd == TRUE)/*Vizio Spec 3.3 3.a.ii.2 exit auto input detect*/
            {
                mSystemParams.apd = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams); 
            }
        
            mSystemParams.input_src = AUDIO_SOURCE_BLUETOOTH;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            mBTHandleCommand = BT_HANDLE_ON;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
        }
            break;

        case USER_EVENT_SEL_HDMI_IN: /*for hdmi debug*/
        {
            mSystemParams.input_src = AUDIO_SOURCE_HDMI_IN;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams); /*TODO: Don't merge for debug*/
#endif
            mBTHandleCommand = BT_HANDLE_OFF;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
        }
            break;

        case USER_EVENT_SEL_HDMI_ARC: /*Smith Implemented: Only for test ARC*/
        {
            mSystemParams.input_src = AUDIO_SOURCE_HDMI_ARC;
            //mSystemParams.cec_arc = TRUE;
            mSystemParams.sys_event = SYS_EVENT_SEL_SRC; /*set audio system event*/
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
            pASH_ObjCtrl->SendEvent( &mSystemParams );
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )            
            pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams); /*TODO: Don't merge for debug*/
#endif
            mBTHandleCommand = BT_HANDLE_OFF;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
        }
            break;

        case USER_EVENT_CCK:
        {   
#if ( configSTM32F411_PORTING == 1 )          
            pCCKManage_ObjCtrl->start(&mSystemParams);
#endif
        }
        break;
        
        case USER_EVENT_VOL_UP:
        {             
            if ( pSystemParams->master_gain < master_gain_limit.max_gain )
            {
                mSystemParams.master_gain ++;
            }

            if( pSystemParams->mute == SOUND_MUTE )
            {
                mSystemParams.mute = SOUND_DEMUTE;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );

                mSystemParams.sys_event = SYS_EVENT_VOLUME_SET; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );   
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
            else
            {
                mSystemParams.sys_event = SYS_EVENT_VOLUME_SET; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );    
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;

        case USER_EVENT_VOL_DN:
        {         
            if ( pSystemParams->master_gain > master_gain_limit.min_gain )
            {
                mSystemParams.master_gain --;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_SET;
                
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_BASS_UP:
        {
            if ( pSystemParams->bass_gain <  BASS_MAX)
            {
                mSystemParams.bass_gain++;

                mSystemParams.sys_event = SYS_EVENT_BASS_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;
            
        case USER_EVENT_BASS_DN:
        {
            if ( pSystemParams->bass_gain > BASS_MIN )
            {
                mSystemParams.bass_gain--;

                mSystemParams.sys_event = SYS_EVENT_BASS_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;    

        case USER_EVENT_TREBLE_UP:
        {
            if ( pSystemParams->treble_gain <  TREBLE_MAX)
            {
                mSystemParams.treble_gain++;

                mSystemParams.sys_event = SYS_EVENT_TREBLE_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;
            
        case USER_EVENT_TREBLE_DN:
        {
            if ( pSystemParams->treble_gain > TREBLE_MIN )
            {
                mSystemParams.treble_gain--;

                mSystemParams.sys_event = SYS_EVENT_TREBLE_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;    

        case USER_EVENT_SUB_UP:
        {
            if ( pSystemParams->sub_gain < SUB_MAX )
            {
                mSystemParams.sub_gain++;

                mSystemParams.sys_event = SYS_EVENT_SUB_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_SUB_DN:
        {
            if ( pSystemParams->sub_gain > SUB_MIN )
            {
                mSystemParams.sub_gain--;

                mSystemParams.sys_event = SYS_EVENT_SUB_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_CENTER_UP:
        {
            if ( pSystemParams->center_gain < CENTER_MAX )
            {
                mSystemParams.center_gain++;

                mSystemParams.sys_event = SYS_EVENT_CENTER_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_CENTER_DN:
        {
            if ( pSystemParams->center_gain > CENTER_MIN )
            {
                mSystemParams.center_gain--;

                mSystemParams.sys_event = SYS_EVENT_CENTER_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_REAR_UP:
        {
            if ( pSystemParams->LsRs_gain < REAR_MAX )
            {
                mSystemParams.LsRs_gain++;
                mSystemParams.sys_event = SYS_EVENT_LSRS_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_REAR_DN:
        {
            if ( pSystemParams->LsRs_gain > REAR_MIN )
            {
                mSystemParams.LsRs_gain--;
                mSystemParams.sys_event = SYS_EVENT_LSRS_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_BALANCE_UP:
        {
            if ( pSystemParams->balance < BALANCE_MAX )
            {
                mSystemParams.balance++;
                mSystemParams.sys_event = SYS_EVENT_BALANCE_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_BALANCE_DN:
        {
            if ( pSystemParams->balance > BALANCE_MIN )
            {
                mSystemParams.balance--;
                mSystemParams.sys_event = SYS_EVENT_BALANCE_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
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
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
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
                mSystemParams.sys_event = SYS_EVENT_AV_DELAY_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_AV_DELAY_DN:
        {
            if ( pSystemParams->av_delay > AV_DELAY_MIN )
            {
                mSystemParams.av_delay--;

                mSystemParams.sys_event = SYS_EVENT_AV_DELAY_SET;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
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
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;

            if(pSystemParams->vizio_rmt == FALSE)
            {
                mSystemParams.vizio_rmt = TRUE;
            }
            else
            {
                mSystemParams.vizio_rmt = FALSE;
            }

            pSDM_ObjCtrl->SendEvent( &mSystemParams );
        }
        break;

        case USER_EVENT_VIZ_RMT_ON:
        {
            if(pSystemParams->vizio_rmt == FALSE)
            {
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                mSystemParams.vizio_rmt = TRUE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;
        
        case USER_EVENT_VIZ_RMT_OFF:
        {
            if(pSystemParams->vizio_rmt == TRUE)
            {
                mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
                mSystemParams.vizio_rmt = FALSE;
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;
        
        case USER_EVENT_MUTE:
        {
            mSystemParams.mute = !(mSystemParams.mute);

            if ( pSystemParams->mute == SOUND_DEMUTE )
            {
                mSystemParams.sys_event = SYS_EVENT_VOLUME_SET;
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            }
            
            mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
            pASH_ObjCtrl->SendEvent( &mSystemParams ); 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;
        
        case USER_EVENT_PREVIOUS:
        {
            if(pSystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK)
            {
                mSystemParams.sys_event = SYS_EVENT_USB_MMS_PREVIOUS;
#if ( configSTM32F411_PORTING == 1 ) 
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            }
            else if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH)
            {
                mBTHandleCommand = BT_HANDLE_PREVIOUS;
                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
            }
        }
        break;
            
        case USER_EVENT_NEXT:
        {
            if(pSystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK)
            {
                mSystemParams.sys_event = SYS_EVENT_USB_MMS_NEXT;
 #if ( configSTM32F411_PORTING == 1 )               
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);       
#endif
            }
            else if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH)
            {
                mBTHandleCommand = BT_HANDLE_NEXT;
                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
            }
        }
        break;
        
        case USER_EVENT_PLAY_PAUSE:
        {
            if(pSystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK)
            {
                mSystemParams.sys_event = SYS_EVENT_USB_MMS_PLAY_PAUSE;
#if ( configSTM32F411_PORTING == 1 )                
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            }
            else if(pSystemParams->input_src == AUDIO_SOURCE_BLUETOOTH)
            {
                mBTHandleCommand = BT_HANDLE_PLAY_PAUSE;
                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
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
#if ( configSTM32F411_PORTING == 1 )                
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
                mBTHandleCommand = BT_HANDLE_ON;
                pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
            }
            
            mBTHandleCommand = BT_HANDLE_PAIRING;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
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
            
            mBTHandleCommand = BT_HANDLE_CLEAN_DEVICE;
            pBTHandle_ObjCtrl->bt_instr_sender(&mBTHandleCommand);
        }
        break;

        case USER_EVENT_DEVICE_RESET:
        {
            //set default value
            _RST_DEF_SETTING();

            //led control
            mSystemParams.sys_event = SYS_EVENT_SYSTEM_RESET; 
            pUDM_ObjCtrl->SendEvent( &mSystemParams );
            
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
                pBTHandle_ObjCtrl->power_ctl();
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )                
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            }
            else
            {
                mSystemParams.op_mode = MODE_DEMO_1;
                HMI_Service_setDemo1n3_SysParams( mSystemParams.op_mode );
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_ENTER_DEMO1);
               
                pBTN_ObjCtrl->register_demo_1n3_cmd();             
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
                
                pPowerHandle_ObjCtrl->power_toggle();
                pBTHandle_ObjCtrl->power_ctl();
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )                
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            }
            else
            {
                mSystemParams.op_mode = MODE_DEMO_2;
                HMI_Service_setDemo1n3_SysParams( mSystemParams.op_mode );
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )                
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_ENTER_DEMO2);
                pBTN_ObjCtrl->register_demo_2_cmd();
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
                pBTHandle_ObjCtrl->power_ctl();
                mSystemParams.sys_event = SYS_EVENT_SYSTEM_DOWN;
                pSDM_ObjCtrl->SendEvent( &mSystemParams ); 
                pASH_ObjCtrl->SendEvent( &mSystemParams );
#if ( configSTM32F411_PORTING == 1 )                
                pUSB_MMP_ObjCtrl->SendEvent(&mSystemParams);
#endif
            }
            else
            {
                mSystemParams.op_mode = MODE_DEMO_3;
                HMI_Service_setDemo1n3_SysParams( mSystemParams.op_mode );
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pSDM_ObjCtrl->SendEvent( &mSystemParams );
                pASH_ObjCtrl->SendEvent( &mSystemParams );
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_ENTER_DEMO3);
                pBTN_ObjCtrl->register_demo_1n3_cmd();
                
                TRACE_DEBUG((0, "MODE - DEMO 3 "));
            }
        }
            break;

        case USER_EVENT_MODE_FACTORY:
        {
            /* swithing to factory mode */
            TRACE_DEBUG((0, "MODE - FACTORY "));
            mSystemParams.sys_event = SYS_EVENT_SAVE_USER_SETTING;
            mSystemParams.op_mode = MODE_FACTORY;
            pSDM_ObjCtrl->SendEvent( &mSystemParams );
        }
            break;

        case USER_EVENT_START_MODE:
        {
            if(pSystemParams->op_mode == MODE_DEMO_2)
            {
#if ( configSTM32F411_PORTING == 1 )            
                pUSB_MMP_ObjCtrl->StartDemoMode2();
#endif
                HMI_Service_setDemo1n3_SysParams(MODE_DEMO_2);
                mSystemParams.master_gain = 8; /* start demo2 volume as 8 */
                mSystemParams.sys_event = SYS_EVENT_DEV_CONFIG; /*set audio system event*/
                pASH_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
        break;

        case USER_EVENT_STOP_MODE:
        {
            if(pSystemParams->op_mode == MODE_DEMO_2)
            {
#if ( configSTM32F411_PORTING == 1 )             
                pUSB_MMP_ObjCtrl->StopDemoMode2();
#endif               
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

        case USER_EVENT_CEC_POWER_OFF:
        {
            if ( pPowerHandle_ObjCtrl->get_power_state() == POWER_ON || pPowerHandle_ObjCtrl->get_power_state() == POWER_UP )
            {
                xHMISrvEventParams srv_parms = {xHMI_EVENT_CEC, USER_EVENT_UNDEFINED };

                srv_parms.event = USER_EVENT_POWER;
                HMI_Service_EventSender( &srv_parms );
            }
        }
            break;

        case USER_EVENT_CEC_SET_MUTE:
        {
            if ( pSystemParams->input_src == AUDIO_SOURCE_HDMI_IN )
            {
                mSystemParams.mute = TRUE;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;
    
        case USER_EVENT_CEC_SET_DEMUTE:
        {
            if ( pSystemParams->input_src == AUDIO_SOURCE_HDMI_IN )
            {
                mSystemParams.mute = FALSE;
                mSystemParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
                pASH_ObjCtrl->SendEvent( &mSystemParams ); 
                pUDM_ObjCtrl->SendEvent( &mSystemParams );
            }
        }
            break;
    
        case USER_EVENT_CEC_SET_SAC_ON:
        {
            mSystemParams.cec_sac = TRUE;
        }
            break;
        
        case USER_EVENT_CEC_SET_SAC_OFF:
        {
            mSystemParams.cec_sac = FALSE;
        }
            break;
        
        case USER_EVENT_CEC_SET_ARC_ON:
        {
            mSystemParams.cec_arc = TRUE;
        }
            break;
        
        case USER_EVENT_CEC_SET_ARC_OFF:
        {
            mSystemParams.cec_arc = FALSE;
        }
            break;
        
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
        case SYS_EVENT_SYSTEM_UP:
        case SYS_EVENT_CEC_SYSTEM_UP:
        case SYS_EVENT_SYSTEM_DOWN:
        case SYS_EVENT_SEL_SRC:
        case SYS_EVENT_VOLUME_MUTE_SET:
        case SYS_EVENT_VOLUME_SET:
        case SYS_EVENT_TREBLE_SET:
        case SYS_EVENT_BASS_SET:    
        {
            pASH_ObjCtrl->SendEvent( params );
            pFMD_ObjCtrl->AckSuccess( NULL );
        }
            break;

        default:
            return;
    }

    mSystemParams = *params;

}

static bool HMI_Service_EventBypassWhenStandby( xHMISrvEventParams *pEventParams )
{
    bool ret = FALSE;

    if( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF ) 
    {
        switch( pEventParams->event )
        {       
            case USER_EVENT_POWER:
            case USER_EVENT_MODE_FACTORY:    
            case USER_EVENT_CEC_POWER_ON:
            {    
                ret = TRUE;
            }
                break;
            case USER_EVENT_VOL_UP:
            case USER_EVENT_VOL_DN:
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
#if ( configSTM32F411_PORTING == 1 )
    switch( event )
    {
        case USER_EVENT_POWER:
        case USER_EVENT_START_MODE:
        case USER_EVENT_STOP_MODE:
        case USER_EVENT_SEL_AUX1:
        case USER_EVENT_SEL_AUX2:
        case USER_EVENT_SEL_COAXIAL:
        case USER_EVENT_SEL_OPTICAL:
        case USER_EVENT_SEL_USB_PLAYBACK:
        case USER_EVENT_SEL_BT:
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
#endif   
}

static bool HMI_Service_DetectConfirmEvent(void *params)
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;
    bool res = TRUE;

    switch(pEventParams->event)
    {
        case USER_EVENT_POWER:
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
        case USER_EVENT_VIZ_RMT_ON:
        case USER_EVENT_VIZ_RMT_OFF:
        case USER_EVENT_MODE_FACTORY:
        case USER_EVENT_START_MODE:
        case USER_EVENT_STOP_MODE:
        case USER_EVENT_SEL_AUX1:
        case USER_EVENT_SEL_AUX2:
        case USER_EVENT_SEL_COAXIAL:
        case USER_EVENT_SEL_OPTICAL:
        case USER_EVENT_SEL_USB_PLAYBACK:
        case USER_EVENT_SEL_BT:
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
            
            case USER_EVENT_SEL_AUX1:
            case USER_EVENT_SEL_AUX2:
            case USER_EVENT_SEL_COAXIAL:
            case USER_EVENT_SEL_OPTICAL:
            case USER_EVENT_SEL_USB_PLAYBACK:
            case USER_EVENT_SEL_BT:
            case USER_EVENT_BT_PAIR:
            case USER_EVENT_RF_PAIR:
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
    
    if(mSystemParams.vizio_rmt == FALSE && pEventParams->event_id== xHMI_EVENT_VIZ_FAC_IR)
        return FALSE;

    return TRUE;
}

static void HMI_Service_handleCommand( void *params )
{
    xHMISrvEventParams *pEventParams = ( xHMISrvEventParams*)params;

    if ( pEventParams->event == USER_EVENT_NULL )
    {
        TRACE_ERROR((0, "Unsupported event "));
        return;
    }
    if ( pEventParams->event_id == xHMI_EVENT_NULL )
    {
        return;
    }
    if(HMI_Service_Filter_VIZ_FAC_Event(pEventParams) == FALSE)
    {
        return;
    }
    if ( HMI_Service_EventBypassWhenStandby( pEventParams ) == FALSE )
    {
        return;
    }
    HMI_Service_FilterDemoModeEvent(pEventParams);
    if(HMI_Service_DetectConfirmEvent(pEventParams) == FALSE)
    {
        pIR_PrgRemote_ObjCtrl->SendEvent( pEventParams->event );
        HMI_Service_sendCommand( pEventParams->event );
    }
    else
    {
        if ( HMI_Service_checkEventTimoutOrFirstPressed( pEventParams->event ) == TRUE )
        {
            /*TODO: could report to UI task*/
            TRACE_DEBUG((0, "EVENT - First Pressed or TimeOut ")); 
        }
        else
        {
            pIR_PrgRemote_ObjCtrl->SendEvent( pEventParams->event );
            HMI_Service_sendCommand( pEventParams->event );
        }
    }
}

static uint8 HMI_Service_InstrGetQueueNumber( void )
{
    return (uint8)uxQueueMessagesWaiting(mHMISrvClusion.serviceQueue.xQueue);
}

void HMI_ServiceUser( void *pvParameters )
{
    xHMISrvEventParams EventParams;

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
                mHMISrvClusion.userState = TASK_READY;
            }
                break;
            
            case TASK_READY:
            {
                if ( HMI_Service_InstrGetQueueNumber() != 0 )
                {
                    mHMISrvClusion.userState = TASK_RUNING;
                }
            }
                break;
        
            case TASK_RUNING:
            {
                if (HMI_Service_EventReceiver( &EventParams ) == TRUE )
                {
                    HMI_Service_handleCommand( &EventParams );
                    
                    mHMISrvClusion.userState = TASK_BLOCKED;
                }
                else
                {
                    mHMISrvClusion.userState = TASK_READY;
                }
            }
                break;

            case TASK_BLOCKED:
            {
                mHMISrvClusion.userState = TASK_READY;
            }
                break;
        }
            
        vTaskDelay(HMI_TIME_TICK);
    }
}

void HMI_AutoPowerControl_Task( void *pvParameters )
{
    xHMISrvEventParams srv_parms = {xHMI_EVENT_APC, USER_EVENT_UNDEFINED };
    static uint16 timeout = HMI_APD_TIMEOUT;

    for( ;; )
    {
        if( pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
        {
            timeout = HMI_APD_TIMEOUT;

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
                    if ( timeout < 1 )
                    {
                        timeout = HMI_APD_TIMEOUT;
                        srv_parms.event = USER_EVENT_POWER;
                        HMI_Service_EventSender( &srv_parms );
                    }
                    else
                    {
                        timeout --;
                        TRACE_DEBUG((0, "APD COUNT = %d", timeout ));
                    }
                }
                else
                {
                    timeout = HMI_APD_TIMEOUT;
                }
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
        }
    }
    else
    {
        if ( xTaskIsTaskSuspended( mHMISrvClusion.aidTaskHandle ) == pdPASS ) /*task is not working*/
        {
            AID_timeout = AID_TIMEOUT;
            mSignalAvaiableCnt = 0;
            mSignalUnavaiableCnt = 0;
            mProcessCnt = 0;  
            vTaskResume( mHMISrvClusion.aidTaskHandle );
            TRACE_DEBUG((0, "RESUME: Auto_Input_Detect_Manager"));
            HMI_AutoPowerControl_TaskCtl(BTASK_SUSPENDED);
            if (  pPowerHandle_ObjCtrl->get_power_state() != POWER_ON )
            {
                aid_parms.event = USER_EVENT_POWER; /* Poweron*/
                HMI_Service_EventSender( &aid_parms );
            }
        }
    }
}

static void HMI_AutoInputDetect_task( void *pvParameters )
{
    uint8 SignalAvaliableThrehold; 
    uint8 SystemResetType;

    for(;;)
    {
        if( pHMISrvClusion->userState == TASK_READY )
        {
            /*Turn off when flag is not set*/
            if ( pSystemParams->auto_input_detect == FALSE)
            {
                HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
            }
            
            if (  pPowerHandle_ObjCtrl->get_power_state() == POWER_OFF )
            {
                aid_parms.event = USER_EVENT_POWER; /* Poweron*/
                HMI_Service_EventSender( &aid_parms );
            }
            
            if (  pPowerHandle_ObjCtrl->get_power_state() == POWER_ON ) 
            {
                mProcessCnt++;

                SystemResetType = pPowerHandle_ObjCtrl->getSystemResetType();

                UserParmsAutoPowerDown = pSystemParams->auto_input_detect;

                /* Terminate the auto detection when it is not User mode. Tony140108 */
                if ( pSystemParams->op_mode != MODE_USER ) 
                {
                    HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
                }

                /* Clean ac on and reset flag */
                if ( SystemResetType == RCC_FLAG_PORRST || SystemResetType == RCC_FLAG_SFTRST )
                {
                    pPowerHandle_ObjCtrl->clrSystemResetType();
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
                    if ( pSystemParams->auto_input_detect == TRUE )
                    {
                        aid_parms.event = USER_EVENT_AUTO_INPUT_DETECT_CLEAN; /*Clean Flag*/
                        HMI_Service_EventSender( &aid_parms );
                        if (  pPowerHandle_ObjCtrl->get_power_state() == POWER_ON )
                        {
                            aid_parms.event = USER_EVENT_POWER; /* Powerdown*/
                        }
                        HMI_Service_EventSender( &aid_parms );
                        TRACE_DEBUG((0, "  Auto detection 15 mins auto power down   !! ")); 
                        AID_timeout = AID_TIMEOUT;
                        HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);
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
                        aid_parms.event = USER_EVENT_AUTO_INPUT_DETECT_CLEAN; /*Clean Flag*/
                        HMI_Service_EventSender( &aid_parms );

                        TRACE_DEBUG((0,"Not a Auto Detection source"));
                        HMI_AutoInputDetect_TaskCtl(BTASK_SUSPENDED);

                        return;
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
        vTaskDelay(HMI_AID_TIME_TICK);
    }     
}


void HMI_Service_CreateTask( void )
{
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

    mHMISrvClusion.serviceQueue.xQueue = xQueueCreate( HMI_QUEUE_LENGTH,(unsigned portBASE_TYPE )(sizeof(xHMISrvEventParams)/sizeof(uint8)) );
    mHMISrvClusion.serviceQueue.xBlockTime = BLOCK_TIME(0);
    if( mHMISrvClusion.serviceQueue.xQueue == NULL )
    {
        mHMISrvClusion.xOS_ErrId = xOS_TASK_QUEUE_CREATE_FAIL;
    }

    pPowerHandle_ObjCtrl->initialize(); 
    pASH_ObjCtrl->CreateTask( );
    pUDM_ObjCtrl->CreateTask(); 
    //pBTHandle_ObjCtrl->CreateTask();      /* Create BTHandle task */
    //pRFHandle_ObjCtrl->CreateTask();      /* Create RFHandle task */
    //pCCKManage_ObjCtrl->CreateTask();
    //pUSB_MMP_ObjCtrl->CreateTask();
    //USBMusicManager_RegisterSetDemoMode2Timeout(HMI_Service_setDemo2_Timeout); /* register set Demo2 timeout function */

    TRACE_DEBUG((0, "EEpromDefaultParams[0] = %d ",EEpromDefaultParams[0]));    /*for code size 232kb */

    //HMI_AutoPowerControl_CreateTask();
    //HMI_AutoInputDetect_CreateTask();
}

static xEventTypeAssort HMI_Service_Event_Assort( xHMIUserEvents cur_events, xHMIUserEvents last_events )
{
    xEventTypeAssort ret = EVENT_ASSORT_NO_NEED_FIRST_CONFIRM;

    switch ( cur_events )
    {
        case USER_EVENT_SEL_SRC:
        case USER_EVENT_AUTO_SEL_SRC:
        {
            if ( (last_events == USER_EVENT_SEL_SRC) || (last_events == USER_EVENT_AUTO_SEL_SRC))
            {
                ret = EVENT_ASSORT_SAME;
            }
            else
            {
                ret = EVENT_ASSORT_DIFFERENT;
            }
        }
            break;

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

        default:
            ret = EVENT_ASSORT_NO_NEED_FIRST_CONFIRM;
    }

    return ret;
}

static xHMISystemParams HMI_Service_getSystemParams( void )
{
    return mSystemParams;
}
