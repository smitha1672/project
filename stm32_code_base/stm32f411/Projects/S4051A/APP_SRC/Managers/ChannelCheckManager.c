#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

#include "device_config.h"
#include "ext_flash_driver.h"
#include "USBFileSearch.h"
#include "stm3210c_usb_audio_codec.h"

#include "ChannelCheckManager.h"
#include "USBMediaManager.h"
#include "USBMusicManager.h"
#include "AudioDeviceManager.h"



#define CCK_MANAGER_QUEUE_LENGTH 3

#define BYTE_OF_ChunkID         4
#define BYTE_OF_ChunkSize      4

#define TotalCheckFile  6
#define WaveHeaderSize 50
#define Index_L_StartAddr     0
#define Index_R_StartAddr     1
#define Index_C_StartAddr     2       
#define Index_Ls_StartAddr    3    
#define Index_Rs_StartAddr    4
#define Index_W_StartAddr     5


//____________________________________________________________________________________________________________
/*extern variable*/
extern USB_FILE_SEARCH_OBJECT *pUSBFSE_Object;
extern USB_MUSIC_MANAGE_OBJECT *pUSB_MMP_ObjCtrl;
extern USB_MEDIA_MANAGE_OBJECT *pUSBMedia_ObjCtrl;
extern AUDIO_DEVICE_MANAGER_OBJECT *pADM_ObjCtrl;
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;
extern DSP_CTRL_OBJECT *pDSP_ObjCtrl;

//____________________________________________________________________________________________________________
//static api header
static void CCKManager_CreateTask(void);
static void CCKManager_GetNoiseVersionNum(uint8 * vNum);
static void CCKManager_GetNoiseMemAddr(void);
static void CCKManager_ServiceManage( void *pvParameters );
static uint32 CCKManager_GetNoiseFileStartAddr(void);
static void CCKManager_Start(const void * params);
static void CCKManager_Stop(const void * params);

static void CCKMedia_CreateTask(void);
static void CCKMedia_TaskCtl(bool val);
static void CCKMedia_ServiceManage( void *pvParameters );


//____________________________________________________________________________________________________________
const CCK_MANAGE_OBJECT   CCKManage =
{
    CCKManager_CreateTask,
    CCKManager_GetNoiseVersionNum,
    CCKManager_GetNoiseFileStartAddr,
    CCKManager_Start,
    CCKManager_Stop
};
const CCK_MANAGE_OBJECT *pCCKManage_ObjCtrl = &CCKManage;

typedef struct CCK_MANAGER_PARAMETERS
{
    xTaskHandle TaskHandle;
//    TaskHandleState taskState;
//    xSemaphoreParameters SParams;
//    xQueueParameters QParams;
} xCCKManagerParams;

typedef enum{
    CCK_EVENT_IDLE = 0,
    CCK_INIT_NOISE,
    CCK_START_NOISE,
    CCK_INIT_PLAY_NOISE,
    CCK_PLAY_NOISE,
    CCK_STOP_NOISE,
    CCK_END_NOISE
}xCCKManagerState;

/* The sequence is followed with Vizio UI Spec.(Angus 2014/1/7) */
typedef enum{
    CCK_OUTPUT_NULL = 0,
    CCK_OUTPUT_LEFT_CHANNEL,
    CCK_OUTPUT_CENTER_CHANNEL,
    CCK_OUTPUT_RIGHT_CHANNEL,
    CCK_OUTPUT_RIGHT_REAR_CHANNEL,
    CCK_OUTPUT_LEFT_REAR_CHANNEL,
    CCK_OUTPUT_SUBWOOFER_CHANNEL
}xCCKOutputChannel;

typedef enum{
    CCK_INDEX_L = 0,
    CCK_INDEX_R,
    CCK_INDEX_C,
    CCK_INDEX_Ls,
    CCK_INDEX_Rs,
    CCK_INDEX_W
}xCCKIndexNoiseStartAddr;


typedef struct{
    xCCKOutputChannel outputCH;
    xCCKIndexNoiseStartAddr indexAddr;
    CS4953xAudioRoute route;
}xCCKNoiseIndexMap;
//____________________________________________________________________________________________________________
/*static variable*/
static xCCKManagerParams mCCKManagerParams;
static xCCKManagerParams mCCKMediaParams;
static FSE_WaveFormat mNoiseWaveFormat;
static uint8 WaveHeaderBuf[WaveHeaderSize] ={ 0x00 };
static uint32 mNoiseInFlashStartAddr = 0;
static uint32 mNoiseInFlashMemAddr[6]={0,0,0,0,0,0};
static xCCKManagerState mCCKManagerState = CCK_EVENT_IDLE;
static xCCKOutputChannel mCCKOutputChannel = CCK_OUTPUT_NULL;
static xHMISystemParams* pCCK_HMIParams;

static const xCCKNoiseIndexMap mCCKNoiseIndexMap[] = {
    {CCK_OUTPUT_LEFT_CHANNEL,CCK_INDEX_L,ROUTE_FL},
    {CCK_OUTPUT_CENTER_CHANNEL,CCK_INDEX_C,ROUTE_CENTER},
    {CCK_OUTPUT_RIGHT_CHANNEL,CCK_INDEX_R,ROUTE_FR},
    {CCK_OUTPUT_RIGHT_REAR_CHANNEL,CCK_INDEX_Rs,ROUTE_RR},
    {CCK_OUTPUT_LEFT_REAR_CHANNEL,CCK_INDEX_Ls,ROUTE_RL},
    {CCK_OUTPUT_SUBWOOFER_CHANNEL,CCK_INDEX_W,ROUTE_LFE0}
};
static const uint8 mCCKNoiseIndexMapSize = sizeof(mCCKNoiseIndexMap)/sizeof(mCCKNoiseIndexMap[0]);

//____________________________________________________________________________________________________________
static void CCKManager_Init(void)
{
    mCCKManagerState = CCK_EVENT_IDLE;
    mCCKOutputChannel = CCK_OUTPUT_NULL;
}



static void CCKManager_CreateTask(void)
{
    if ( xTaskCreate( 
             CCKManager_ServiceManage, 
             ( portCHAR * ) "Channel_Check_Manager", 
             (STACK_SIZE), 
             NULL, 
             tskCCK_MANAGER_PRIORITY, 
             &mCCKManagerParams.TaskHandle ) != pdPASS )
     {
         TRACE_ERROR((0, "Channel_Check_Manager task create failure " ));
     }
     else
     {
        /* If SB is play channel check : TaskResume , else :TaskSuspend */
        vTaskSuspend( mCCKManagerParams.TaskHandle );
     }
     
    CCKManager_GetNoiseMemAddr();
    pUSBMedia_ObjCtrl->assign_cck_waveFormat(&mNoiseWaveFormat);

    CCKMedia_CreateTask();
}



static void CCKManager_TaskCtl(bool val)
{

    CCKManager_Init();
    
    if( val == BTASK_SUSPENDED )
    {
         if ( xTaskIsTaskSuspended( mCCKManagerParams.TaskHandle ) != pdPASS ) /*task is working*/
         {
            TRACE_DEBUG((0, "SUSPEND: ChannelCheckManager"));
            vTaskSuspend( mCCKManagerParams.TaskHandle );
         }
    }
    else
    {
        if ( xTaskIsTaskSuspended( mCCKManagerParams.TaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( mCCKManagerParams.TaskHandle );
            TRACE_DEBUG((0, "RESUME: ChannelCheckManager"));
        }
    }
    
}

static void CCKManager_Start(const void * params)
{
    AUDIO_SOURCE src = AUDIO_SOURCE_USB_PLAYBACK;
    pCCK_HMIParams = ( xHMISystemParams *)params;

    xHMISystemParams HMISystemParams; /*smith modifies: Fix warning issue*/
    HMISystemParams = *pCCK_HMIParams; /*Smith modifies: Fix warning issue*/
    
    if(pCCK_HMIParams->input_src == AUDIO_SOURCE_USB_PLAYBACK)
    {
       /* USB source switchs to device mode */
        HMISystemParams.input_src = AUDIO_SOURCE_NOISE_CHECK;
        pUSB_MMP_ObjCtrl->ModeSwtich(&HMISystemParams); 
    }
    Audio_MAL_I2S_Pin_Init();
    
    pADM_ObjCtrl->SetInputPath(src);
    pAudLowLevel_ObjCtrl->AudParmsConf(NULL);
    pAudLowLevel_ObjCtrl->SetLSRS(10);  /* Set Ls & Rs vol , otherwise the sound of CCK is too small at Ls & Rs */

    pDSP_ObjCtrl->lowlevel_task_set_state( TASK_SUSPENDED );
    CCKManager_TaskCtl(BTASK_RESUME);
    CCKMedia_TaskCtl(BTASK_RESUME);
    mCCKManagerState = CCK_INIT_NOISE;
}

static void CCKManager_Stop(const void * params)
{
    pCCK_HMIParams = ( xHMISystemParams *)params;

    if ( xTaskIsTaskSuspended( mCCKMediaParams.TaskHandle ) != pdPASS ) /*task is working*/
    {
        pUSBMedia_ObjCtrl->media_stop();
        pDSP_ObjCtrl->setAudioRoute(ROUTE_INITIAL);
        
        if(pCCK_HMIParams->input_src != AUDIO_SOURCE_USB_PLAYBACK)
        {
#if ( configI2S_OUT == 1 )			
            Audio_MAL_I2S_Pin_PullDown();
#endif 
        }	
        
        mCCKManagerState = CCK_END_NOISE;
    }
}

static void CCKManager_GetNoiseVersionNum(uint8 * vNum)
{
    uint32 addr = 0;
    sFLASH_ReadBuffer(vNum, addr, BYTE_OF_VERSION);
}

static uint32 CCKManager_GetNoiseFileStartAddr(void)
{
    return mNoiseInFlashStartAddr;
}

static void CCKManager_GetNoiseMemAddr(void)
{
    uint8 i;   uint32 addr = 0;
    FSE_FormatCheck check; 

    addr += BYTE_OF_VERSION;    /* version information is stored at first and second byte in Noise Check bin file (Angus 2014/2/12) */
    mNoiseInFlashMemAddr[0] = addr;
    for(i=1;i<TotalCheckFile;i++)
    {
        MEMSET(WaveHeaderBuf,0xff,sizeof(WaveHeaderBuf));
        sFLASH_ReadBuffer(WaveHeaderBuf, addr, WaveHeaderSize);
        check = pUSBFSE_Object->formatCheck(NULL, &(mNoiseWaveFormat),WaveHeaderBuf,1);
        
        if(check == FSE_Valid_WAVE_File)
        {
            addr += mNoiseWaveFormat.RIFFchunksize + BYTE_OF_ChunkID + BYTE_OF_ChunkSize;
            mNoiseInFlashMemAddr[i] = addr;
        }
    }
}

static uint32 CCKManager_GetNoiseStartAddr(xCCKOutputChannel ch)
{
    uint32 startAddr = 0;;
    uint8 i;

    for(i=0;i<mCCKNoiseIndexMapSize;i++)
    {
        if(mCCKNoiseIndexMap[i].outputCH == ch)
        {
            startAddr = mNoiseInFlashMemAddr[mCCKNoiseIndexMap[i].indexAddr];
            pDSP_ObjCtrl->setAudioRoute(mCCKNoiseIndexMap[i].route);
            break;
        }
    }
    return startAddr;
}


static void CCKManager_ServiceManage( void *pvParameters )
{
    int i;  static int repeatCnt = 0;
    FSE_FormatCheck res;
    for(;;)
    {
        switch(mCCKManagerState)
        {
            case CCK_EVENT_IDLE:
            {
            }
            break;

            case CCK_INIT_NOISE:
            {
                mCCKOutputChannel = CCK_OUTPUT_NULL;
                repeatCnt = 0;              
                mCCKManagerState = CCK_START_NOISE;
            }
            break;

            case CCK_START_NOISE:
            {
                if(mCCKOutputChannel < CCK_OUTPUT_SUBWOOFER_CHANNEL)
                {
                    mCCKOutputChannel++;
                    mNoiseInFlashStartAddr = CCKManager_GetNoiseStartAddr(mCCKOutputChannel);  

                    for(i=0;i<10;i++)
                    {
                        sFLASH_ReadBuffer(WaveHeaderBuf, mNoiseInFlashStartAddr, WaveHeaderSize);
                        res = pUSBFSE_Object->formatCheck(NULL, &(mNoiseWaveFormat),WaveHeaderBuf,1);
                        if(res == FSE_Valid_WAVE_File)
                            break;
                    }

                    if(res == FSE_Valid_WAVE_File)
                    {
                        mCCKManagerState = CCK_INIT_PLAY_NOISE;
                    }
                    else
                    {
                        mCCKManagerState = CCK_END_NOISE;
                    }
                }
                else
                {
                    mCCKManagerState = CCK_END_NOISE;
                }
            }
            break;

            case CCK_INIT_PLAY_NOISE:
            {
                pUSBMedia_ObjCtrl->cck_play();
                mCCKManagerState = CCK_PLAY_NOISE;
            }
            break;

            case CCK_PLAY_NOISE:
            {
                if(pUSBMedia_ObjCtrl->media_isPlayFinished())
                {
                    repeatCnt++;
                    if(repeatCnt < 3)
                    {
                        pUSBMedia_ObjCtrl->cck_play();
                    }
                    else
                    {
                        repeatCnt = 0;
                        mCCKManagerState = CCK_STOP_NOISE;
                    }
                }
            }
            break;

            case CCK_STOP_NOISE:
            {
                mCCKManagerState = CCK_START_NOISE;
            }
            break;

            case CCK_END_NOISE:
            {
                pDSP_ObjCtrl->setAudioRoute(ROUTE_INITIAL);
                pADM_ObjCtrl->SetInputPath(pCCK_HMIParams->input_src);
                pAudLowLevel_ObjCtrl->AudParmsConf(NULL);                
                pUSB_MMP_ObjCtrl->ModeSwtich(pCCK_HMIParams);
                
                mCCKManagerState = CCK_EVENT_IDLE;
                pDSP_ObjCtrl->lowlevel_task_set_state( TASK_RUNING);
                CCKMedia_TaskCtl(BTASK_SUSPENDED);
                CCKManager_TaskCtl(BTASK_SUSPENDED);
                
            }
            break;
        }
        vTaskDelay( TASK_MSEC2TICKS(100) );
    }
}

static void CCKMedia_CreateTask(void)
{
    if ( xTaskCreate( 
             CCKMedia_ServiceManage, 
             ( portCHAR * ) "CCKMedia_ServiceManage", 
             (configMINIMAL_STACK_SIZE), 
             NULL, 
             tskUSB_MEDIA_PRIORITY, 
             &mCCKMediaParams.TaskHandle ) != pdPASS )
     {
         TRACE_ERROR((0, "CCKMedia_ServiceManage task create failure " ));
     }
     else
     {
        /* If SB is play channel check : TaskResume , else :TaskSuspend */
         vTaskSuspend( mCCKMediaParams.TaskHandle );
     }

    pUSBMedia_ObjCtrl->init();
}


static void CCKMedia_TaskCtl(bool val)
{
    pUSBMedia_ObjCtrl->init();
    
    if( val == BTASK_SUSPENDED )
    {
         if ( xTaskIsTaskSuspended( mCCKMediaParams.TaskHandle ) != pdPASS ) /*task is working*/
         {
            vTaskSuspend( mCCKMediaParams.TaskHandle );
            TRACE_DEBUG((0, "SUSPEND: CCKMedia_ServiceManage"));
         }
    }
    else
    {
        if ( xTaskIsTaskSuspended( mCCKMediaParams.TaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( mCCKMediaParams.TaskHandle );
            TRACE_DEBUG((0, "RESUME: CCKMedia_ServiceManage"));
        }
    }
}

static void CCKMedia_ServiceManage( void *pvParameters )
{
    for(;;)
    {
        pUSBMedia_ObjCtrl->mediaManagerTask();
        vTaskDelay( TASK_MSEC2TICKS(10) );
    }
}


