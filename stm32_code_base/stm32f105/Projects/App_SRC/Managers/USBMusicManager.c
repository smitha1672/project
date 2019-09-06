#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "device_config.h"

#include "USBFileSearch.h"
#include "USBMusicManager.h"
#include "USBMediaManager.h"

#include "usb_host_device.h"
#include "PowerHandler.h"
#include "stm3210c_usb_audio_codec.h"
#include "AudioSystemHandler.h"

#define DEMO2_TIMEOUT_TIME (300)
#define USB_MANAGER_QUEUE_LENGTH 3

typedef struct USB_MUSIC_HANDLE_PARAMETERS
{
    xTaskHandle USBTaskHandle;
    xTaskHandle USBServiceTaskHandle;
    TaskHandleState TaskState;
    xSemaphoreParameters SParams;
    xQueueParameters QParams;
    xOS_TaskErrIndicator xOS_ErrId;
} xUSBMusicManagerParams;

typedef enum{
    IdleState = 0,
    PlayState,
    PauseState
}xUSBMusicManagerPlayState;


//____________________________________________________________________________________________________________
//static api header
static void USBMusicManager_Init(void);

static void USBMusicManager_Deinit(void);

static void USBMusicManager_CreateTask(void);

static void USBMusicManager_TaskCtrl( xOS_TaskCtrl val );

static void USBMusicManager_ServiceManage( void *pvParameters );

//static void USBMusicManager_Command_Sevice( void *pvParameters );

static void USBMusicManager_AssignUSBFileList(void * FileList) ;

static void USBMusicManager_setTimeoutCnt(void);

static void USBMusicManager_StartDemoMode2(void);

static void USBMusicManager_StopDemoMode2(void);

static void USBMusicManager_ModeSwitch(const void *params);

static void USBMusicManager_handleCommand( const void *params  );

static void USBMuiscManager_CommandHandle( void );

//____________________________________________________________________________________________________________
extern USB_MEDIA_MANAGE_OBJECT *pUSBMedia_ObjCtrl;
extern USB_FILE_SEARCH_OBJECT *pUSBFSE_Object;
extern USB_HOST_DEVICE_HANDLE_OBJECT *pUSB_HD_ObjCtrl;
extern POWER_HANDLE_OBJECT *pPowerHandle_ObjCtrl;
extern AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl;

const USB_MUSIC_MANAGE_OBJECT   USBMusicManage =
{
    USBMusicManager_Init, /*todo: this should be called by low layer*/
    USBMusicManager_Deinit,
    USBMusicManager_CreateTask,
    USBMusicManager_TaskCtrl,
    USBMusicManager_handleCommand,
    USBMusicManager_AssignUSBFileList,
    USBMusicManager_StartDemoMode2,
    USBMusicManager_StopDemoMode2,
    USBMusicManager_ModeSwitch,
};
const USB_MUSIC_MANAGE_OBJECT *pUSB_MMP_ObjCtrl = &USBMusicManage;


//____________________________________________________________________________________________________________
/*static variable*/
static xUSBMusicManagerParams mUSBMusicManagerParams;
static xUSBMusicManagerState USBMusicManagerState = USB_MMS_IDLE;

static FSE_FileList* mMusicManagerUSBFileList;
static uint8_t USBMusicManagerFileNum = 0;
static xUSBMusicManagerPlayState USBMusicManagerPlayState = IdleState;

static const xHMIOperationMode* pSystemMode;
static const uint8_t* pUSBManagerSystemMute; // David, fix warning. // static const bool*
static xHMISystemParams USBSystemParams;

/** for demo2 **/
static xSet_DemoMode2_Timeout SetDemo2TimeoutCallback;
static volatile uint16 Demo2TimeoutCnt = 0;
static xDemoModeState DemoMode2_State = DEMO_MODE_NULL_STATE;

//____________________________________________________________________________________________________________
static void USBMusicManager_Init(void)
{
    USBMusicManagerFileNum = 0;
    USBMusicManagerPlayState = IdleState;
    USBMusicManagerState = USB_MMS_IDLE;

    Demo2TimeoutCnt = 0;                                          /* for cal the time of demo mode 2 timeout */
    DemoMode2_State = DEMO_MODE_NULL_STATE;     /* for judge demo mode 2 is start or not */
    pUSBMedia_ObjCtrl->init();
}

static void USBMusicManager_Deinit(void)
{
    pUSBMedia_ObjCtrl->media_stop();
}

static void USBMusicManager_CreateTask(void)
{
    mUSBMusicManagerParams.QParams.xQueue = xQueueCreate( USB_MANAGER_QUEUE_LENGTH, FRTOS_SIZE(xUSBMusicManagerState) );
    mUSBMusicManagerParams.QParams.xBlockTime = portMAX_DELAY;

    if( mUSBMusicManagerParams.QParams.xQueue == NULL )
    {
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_QUEUE_CREATE_FAIL;
    }

    if ( xTaskCreate( 
             USBMusicManager_ServiceManage, 
             ( portCHAR * ) "USB_Music_Manager", 
             (STACK_SIZE), 
             NULL, 
             tskUSB_MANAGER_PRIORITY, 
             &mUSBMusicManagerParams.USBTaskHandle ) != pdPASS )
     {
        vTaskDelete( mUSBMusicManagerParams.USBTaskHandle );
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_CREATE_FAIL;
     }
     else
     {
        /* If SB is on USB Source : TaskResume , else :TaskSuspend */
         //vTaskSuspend( mUSBMusicManagerParams.USBTaskHandle );
         mUSBMusicManagerParams.TaskState = TASK_SUSPENDED;
     }

#if 0
    if ( xTaskCreate( 
             USBMusicManager_Command_Sevice, 
             ( portCHAR * ) "USB_Command_Service", 
             (STACK_SIZE), 
             NULL, 
             tskUSB_COMMAND_SERVICE_PRIORITY, 
             &mUSBMusicManagerParams.USBServiceTaskHandle ) != pdPASS )
     {
        vTaskDelete( mUSBMusicManagerParams.USBServiceTaskHandle );
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_CREATE_FAIL;
     }
#endif

    pUSBFSE_Object->init();
    pUSB_HD_ObjCtrl->CreateTask(); /* Create USB_Host_Device_Mode_Handle task */
}


static void USBMusicManager_AssignUSBFileList(void * FileList) 
{
    mMusicManagerUSBFileList = (FSE_FileList*)FileList;
}

static void USBMusicManager_TaskCtrl( xOS_TaskCtrl val )
{
    if( val == xOS_setTASK_SUSPENDED )
    {
         //if ( xTaskIsTaskSuspended( mUSBMusicManagerParams.USBTaskHandle ) != pdPASS ) /*task is working*/
         if (mUSBMusicManagerParams.TaskState == TASK_RUNING)
         {
            USBMusicManager_Deinit();
#if ( configI2S_OUT == 1 )		 	
            Audio_MAL_I2S_Pin_PullDown();
#endif 
            pUSB_HD_ObjCtrl->usb_mode_switch(USB_HOST_DEVICE_CMD_VCP_CONNECT);

            xQueueReset(mUSBMusicManagerParams.QParams.xQueue);
            //vTaskSuspend( mUSBMusicManagerParams.USBTaskHandle );
            mUSBMusicManagerParams.TaskState = TASK_SUSPENDED;
         }
    }
    else if ( val == xOS_setTASK_RESUME )
    {
        //if ( xTaskIsTaskSuspended( mUSBMusicManagerParams.USBTaskHandle ) == pdPASS ) /*task is not working*/
        if (mUSBMusicManagerParams.TaskState == TASK_SUSPENDED)
        {
            xQueueReset(mUSBMusicManagerParams.QParams.xQueue);
            USBMusicManager_Init();

#if ( configI2S_OUT == 1 )
            Audio_MAL_I2S_Pin_Init();
#endif 
            pUSB_HD_ObjCtrl->usb_mode_switch(USB_HOST_DEVICE_CMD_MSC_PROCESS);
        
            //vTaskResume( mUSBMusicManagerParams.USBTaskHandle );
            mUSBMusicManagerParams.TaskState = TASK_RUNING;
        }
    }
    
}

static bool USBMusicManager_InstrSender(void * cmds) 
{
    xUSBMusicManagerState* pCmd = ( xUSBMusicManagerState *)cmds;

    if ( pCmd == NULL )
    {
        return FALSE;
    }

    if ( mUSBMusicManagerParams.USBTaskHandle == NULL )
    {
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_CREATE_FAIL;
        return FALSE;
    }

    //if( xTaskIsTaskSuspended( mUSBMusicManagerParams.USBTaskHandle ) == pdPASS ) /*task is not working*/
    if (mUSBMusicManagerParams.TaskState == TASK_SUSPENDED)
    {
        return FALSE;
    }
    
    if( mUSBMusicManagerParams.QParams.xQueue == NULL )
    {
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_QUEUE_IS_NULL;
        return FALSE;
    }
    
    if( xQueueSend(mUSBMusicManagerParams.QParams.xQueue,pCmd,mUSBMusicManagerParams.QParams.xBlockTime ) != pdPASS)
    {
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_QUEUE_SET_FAIL;
        return FALSE;
    }

    return TRUE;
}

static bool USBMusicManager_InstrReceiver( xUSBMusicManagerState *pCmd ) 
{
    if(pCmd == NULL)
    {
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_QUEUE_IS_NULL;
        return FALSE;
    } 
    
    if( mUSBMusicManagerParams.QParams.xQueue == NULL )
    {
        mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_QUEUE_IS_NULL;
        return FALSE;
    }

    //if( xQueueReceive(mUSBMusicManagerParams.QParams.xQueue,pCmd,mUSBMusicManagerParams.QParams.xBlockTime) != pdPASS )
    if( xQueueReceive(mUSBMusicManagerParams.QParams.xQueue,pCmd,BLOCK_TIME(100)) != pdPASS )
    {
        //mUSBMusicManagerParams.xOS_ErrId = xOS_TASK_QUEUE_GET_FAIL;
        return FALSE;
    }

    return TRUE;   
}


static void USBMuiscManager_CommandHandle( void )
{
    switch( USBMusicManagerState )
    {
        case USB_MMS_IDLE:
        {
            USBMusicManagerPlayState = IdleState;
            
            if(mMusicManagerUSBFileList->fileNumber > 0)
            {
                if(*pSystemMode == MODE_DEMO_2)
                {
                    if(DemoMode2_State == DEMO_MODE_START_STATE)
                    {   
                        pUSBMedia_ObjCtrl->init();
                        USBMusicManagerFileNum = 0;
                        USBMusicManagerState = USB_MMS_PLAY;
                    }
                    else
                    {
                        USBMusicManager_setTimeoutCnt();
                    }
                }
                else
                {
                    pUSBMedia_ObjCtrl->init();
                    USBMusicManagerFileNum = 0;
                    USBMusicManagerState = USB_MMS_PLAY;
                }
            }
            else
            {
                if(*pSystemMode == MODE_DEMO_2)
                {
                    USBMusicManager_setTimeoutCnt();
                }
            }

            if(Demo2TimeoutCnt >= DEMO2_TIMEOUT_TIME)
            {
                DemoMode2_State = DEMO_MODE_STOP_STATE;
                Demo2TimeoutCnt = 0;
                SetDemo2TimeoutCallback();                    
            }
        }
        break;

        case USB_MMS_PLAY:
        {
            if(mMusicManagerUSBFileList->fileNumber > 0)
            {
                if(*pSystemMode == MODE_DEMO_2)
                {
                    if(DemoMode2_State != DEMO_MODE_START_STATE)
                    {
                        USBMusicManager_setTimeoutCnt();
                        USBMusicManagerState = USB_MMS_IDLE;                          
                    }
                    else
                    {
                        Demo2TimeoutCnt = 0;
                        USBMusicManagerPlayState = PlayState;
                        pUSBMedia_ObjCtrl->media_reqWaveNum(USBMusicManagerFileNum);
                        pUSBMedia_ObjCtrl->media_play();
                        USBMusicManagerState = USB_MMS_READY_PLAY; 
                    }
                }
                else
                {
                    if (*pUSBManagerSystemMute == FALSE)
                    {
                        USBSystemParams.sys_event = SYS_EVENT_VOLUME_DELAY_RELEASE; /*set audio system event*/
                        pASH_ObjCtrl->SendEvent( &USBSystemParams );
                    }
                    
                    USBMusicManagerPlayState = PlayState;
                    pUSBMedia_ObjCtrl->media_reqWaveNum(USBMusicManagerFileNum);
                    pUSBMedia_ObjCtrl->media_play();
                    USBMusicManagerState = USB_MMS_READY_PLAY;  
                }
            }
            else
            {
                 USBMusicManagerState = USB_MMS_IDLE;
            }
        }
        break;

        case USB_MMS_READY_PLAY:
        {
            if(pUSBMedia_ObjCtrl->media_isPlayFinished() == TRUE)
            {
                USBMusicManagerFileNum++;

                if(USBMusicManagerFileNum >= mMusicManagerUSBFileList->fileNumber)
                {
                    if(*pSystemMode == MODE_DEMO_2)
                    {
                        DemoMode2_State = DEMO_MODE_STOP_STATE;
                        Demo2TimeoutCnt = 0;
                        SetDemo2TimeoutCallback();
                        USBMusicManagerState = USB_MMS_IDLE;
                    }
                    else
                    {
                        pUSBMedia_ObjCtrl->media_stop();
                        USBMusicManagerFileNum = 0; /* rePlay from first song */
                    }
                }
                USBMusicManagerState = USB_MMS_PLAY;
            }
        }
        break;

        case USB_MMS_NEXT:
        {
            pUSBMedia_ObjCtrl->media_stop();
            
            if(++USBMusicManagerFileNum >= mMusicManagerUSBFileList->fileNumber)
            {
                USBMusicManagerFileNum = 0;
            }                
            USBMusicManagerState = USB_MMS_PLAY;
        }
        break;

        case USB_MMS_PREVIOUS:
        {
            pUSBMedia_ObjCtrl->media_stop();
            
            if(USBMusicManagerFileNum > 0)
            {
                USBMusicManagerFileNum--;
            } 
            USBMusicManagerState = USB_MMS_PLAY;
        }
        break;

        case USB_MMS_PLAY_PAUSE:
        {
            if(USBMusicManagerPlayState == IdleState) 
            {
                USBMusicManagerState = USB_MMS_IDLE;
                //Don't do anything
            }
            else if(USBMusicManagerPlayState == PlayState)
            {
                USBMusicManagerPlayState = PauseState;
                USBMusicManagerState = USB_MMS_STOP;

                /*if(*pSystemMode == MODE_DEMO_2)	//Mask for SB4051_C0-176: play&pause function is abnormal in Demo2 
                {
                    pUSBMedia_ObjCtrl->media_stop();
                }
                else*/
                {
                    pUSBMedia_ObjCtrl->media_pause();
                }                  
            }
            else if(USBMusicManagerPlayState == PauseState)
            {
                USBMusicManagerPlayState = PlayState;

                /*if(*pSystemMode == MODE_DEMO_2) //Mask for SB4051_C0-176: play&pause function is abnormal in Demo2 
                {
                    pUSBMedia_ObjCtrl->media_stop();
                    USBMusicManagerState = USB_MMS_PLAY;
                }
                else*/
                {
                    pUSBMedia_ObjCtrl->media_rePlay();
                    USBMusicManagerState = USB_MMS_READY_PLAY;
                }
                
            }
        }
        break;
        
        case USB_MMS_STOP:
        {
        
        }
        break;

        case USB_MMS_STOP_DEMO2:
        {
            Demo2TimeoutCnt = 0;
            pUSBMedia_ObjCtrl->media_stop();
            USBMusicManagerState = USB_MMS_IDLE;
        }
        break;
#if FOLLOW_MP_DEMO2
        case USB_MMS_START_DEMO2:
        {      
            if(mMusicManagerUSBFileList->fileNumber > 0)
            {
                Demo2TimeoutCnt = 0;
                USBMusicManagerPlayState = PlayState;

                if(USBMusicManagerFileNum >= mMusicManagerUSBFileList->fileNumber)
                {
                    USBMusicManagerFileNum = 0;
                }
                pUSBMedia_ObjCtrl->media_reqWaveNum(USBMusicManagerFileNum);
                pUSBMedia_ObjCtrl->media_play();
                USBMusicManagerState = USB_MMS_READY_PLAY;            
             }
            else
            {
                 USBMusicManagerState = USB_MMS_IDLE;
            }
        }
        break;
        
#endif
    }
}

static void USBMusicManager_ServiceManage( void *pvParameters )
{
    static xUSBMusicManagerState recState;
    
    for(;;)
    {
        if (USBMusicManager_InstrReceiver(&recState) == TRUE )
        {
            USBMusicManagerState = recState;
        }
        
        if (mUSBMusicManagerParams.TaskState == TASK_RUNING)
        {
            if ( pUSB_HD_ObjCtrl->get_mode() == USB_HOST )
            {                    
                    USBMuiscManager_CommandHandle( );
            }
        }        
        //vTaskDelay( TASK_MSEC2TICKS(100) );
    }
}
#if 0
static void USBMusicManager_Command_Sevice( void *pvParameters )
{
    for(;;)
    {
        USBMuiscManager_CommandHandle( );
        vTaskDelay( TASK_MSEC2TICKS(100) );
    }
}
#endif
void USBMusicManager_RegisterSetDemoMode2Timeout( xSet_DemoMode2_Timeout callback )
{
    SetDemo2TimeoutCallback = callback;
}

static void USBMusicManager_setTimeoutCnt(void)
{
    if(DemoMode2_State != DEMO_MODE_STOP_STATE)
    {
        Demo2TimeoutCnt++;
    }
    else
    {
        Demo2TimeoutCnt = 0;
    }
}

static void USBMusicManager_StartDemoMode2(void)
{
#if FOLLOW_MP_DEMO2
    DemoMode2_State = DEMO_MODE_START_STATE;  
    USBMusicManagerState = USB_MMS_START_DEMO2;
#else
    DemoMode2_State = DEMO_MODE_START_STATE;
#endif
}

static void USBMusicManager_StopDemoMode2(void)
{
    DemoMode2_State = DEMO_MODE_STOP_STATE;
    USBMusicManagerState = USB_MMS_STOP_DEMO2;
}

static void USBMusicManager_ModeSwitch(const void *params)
{
    const xHMISystemParams* pHMISystemParams = ( xHMISystemParams *)params;

    if (pHMISystemParams->input_src ==AUDIO_SOURCE_USB_PLAYBACK)
    {
        pUSB_HD_ObjCtrl->usb_mode_switch(USB_HOST_DEVICE_CMD_MSC_PROCESS);//usb mode
    }
    else
    {
        pUSB_HD_ObjCtrl->usb_mode_switch(USB_HOST_DEVICE_CMD_VCP_CONNECT);
    }
}

static void USBMusicManager_handleCommand( const void *params  )
{
    const xHMISystemParams* pHMISystemParams = ( xHMISystemParams *)params;
    xUSBMusicManagerState usb_placblack_ctrl;

    if ( params == NULL )
        return;

    pSystemMode = &pHMISystemParams->op_mode;
    pUSBManagerSystemMute = &pHMISystemParams->mute;
    USBSystemParams = *pHMISystemParams;    //Angus fix issue : AMTSD-5 no audio on optical after playing WAV files
    switch( pHMISystemParams->sys_event )
    {
        case SYS_EVENT_SYSTEM_UP:
        {
            if ( pHMISystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK )
            {
                USBMusicManager_TaskCtrl(xOS_setTASK_RESUME);
            }
            else
            {
                USBMusicManager_TaskCtrl(xOS_setTASK_SUSPENDED);
            }
        }
            break;
    
        case SYS_EVENT_SYSTEM_DOWN:
        {
            USBMusicManager_TaskCtrl(xOS_setTASK_SUSPENDED);
        }
            break;

        case SYS_EVENT_SEL_SRC:
        {
            if ( pHMISystemParams->input_src == AUDIO_SOURCE_USB_PLAYBACK )
            {
                USBMusicManager_TaskCtrl(xOS_setTASK_RESUME);
            }
            else
            {
                USBMusicManager_TaskCtrl(xOS_setTASK_SUSPENDED);
            }
        }
            break;

        case SYS_EVENT_USB_MMS_NEXT:
        {
            usb_placblack_ctrl = USB_MMS_NEXT;
            USBMusicManager_InstrSender( &usb_placblack_ctrl );
        }
            break;
            
        case SYS_EVENT_USB_MMS_PLAY:
        {
            usb_placblack_ctrl = USB_MMS_PLAY;
            USBMusicManager_InstrSender( &usb_placblack_ctrl );
        }
            break;
            
        case SYS_EVENT_USB_MMS_PREVIOUS:
        {
            usb_placblack_ctrl = USB_MMS_PREVIOUS;
            USBMusicManager_InstrSender( &usb_placblack_ctrl );
        }
            break;
            
        case SYS_EVENT_USB_MMS_PLAY_PAUSE:
        {
            usb_placblack_ctrl = USB_MMS_PLAY_PAUSE;
            USBMusicManager_InstrSender( &usb_placblack_ctrl );
        }
            break;

        case SYS_EVENT_DEV_CONFIG:
        {
            if (pHMISystemParams->op_mode == MODE_DEMO_2)
            {
                USBMusicManager_TaskCtrl(xOS_setTASK_RESUME);
                usb_placblack_ctrl = USB_MMS_PLAY_PAUSE;
                USBMusicManager_InstrSender( &usb_placblack_ctrl );
            }
            else if((pHMISystemParams->op_mode == MODE_DEMO_1) || (pHMISystemParams->op_mode == MODE_DEMO_3))
            {
                USBMusicManager_TaskCtrl(xOS_setTASK_SUSPENDED);
            }
        }
            break;
    }


}

