#include "USBFileSearch.h"
#include "USBMediaManager.h"
#include "stm3210c_usb_audio_codec.h"

#include "ext_flash_driver.h"
#include "ChannelCheckManager.h"

#define CHK_POP_SOLUTION 1
#if CHK_POP_SOLUTION//For fix issue for channel check pop sound. Angus 2014/10/28
#include "freertos_conf.h"
#include "freertos_task.h"
#include "AudioDeviceManager.h"
#endif

#define WAVE_READ_BUFFER_SIZE     (1024)

#define MAX_BUFFER_NUM                   (6)//(16)
#define WAVE_FILE_BUFFER_SIZE       (WAVE_READ_BUFFER_SIZE)
#define MAX_SUPPLE_BUFFER_NUM     ((MAX_BUFFER_NUM*2)/3)    /* If catching too much it will lose sound */

#define MAX_EMPTY_FRAME (6)

#if 0 // David, fix warning.
static int counter_empty = 0 ;
#endif

typedef enum
{
    Valid_WAVE_File = 0,
    Unvalid_RIFF_ID,
    Unvalid_WAVE_Format,
    Unvalid_FormatChunk_ID,
    Unsupporetd_FormatTag,
    Unsupporetd_Number_Of_Channel,
    Unsupporetd_Sample_Rate,
    Unsupporetd_Bits_Per_Sample,
    Unvalid_DataChunk_ID,
    Unsupporetd_ExtraFormatBytes,
    Unvalid_FactChunk_ID
}ErrorCode;

typedef enum{
    USB_MEDIA_IDLE,
    USB_MEDIA_OPEN_FILE,
    USB_MEDIA_INIT_I2S,
    USB_MEDIA_INIT_PLAY_FILE,
    USB_MEDIA_PLAY_FILE,
    USB_MEDIA_PAUSE,
    USB_MEDIA_STOP,
    USB_MEDIA_FINISH
}xUSBMediaManagerState;

typedef struct{
	uint32_t ChunkSize;
	uint32_t ChunkIndex;	/*how many chunk*/
	uint32_t Seeker;
}xWaveFileIndicator;

typedef struct{
    __IO uint16_t count;
    uint8_t QueueBuffer[MAX_BUFFER_NUM][WAVE_FILE_BUFFER_SIZE];
    uint16_t bufferReadSize[MAX_BUFFER_NUM];
    uint16_t setIndex;
    uint16_t getIndex;
}xUSBMediaManagerQueue;

#if CHK_POP_SOLUTION//For fix issue for channel check pop sound. Angus 2014/10/28
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;
#endif
//____________________________________________________________________________________________________________
//static api header
static void USBMediaManage_Init(void);
static void USBMediaManage_Play(void);
static void USBMediaManage_Pause(void);
static void USBMediaManage_Stop(void);
static void USBMediaManage_RePlay(void);


static void USBMusicManager_AssignUSBFileList(void * FileList);
static void USBMediaManager_ServiceManager(void);

static QueueState USBMediaManage_FetchQueue(void);
static QueueState USBMediaManage_FetchEmptyQueue(void);

static void USBMediaManage_ReqWaveSong(uint8_t num);
static bool USBMediaManage_IsEndOfSong(void);


static FRESULT USBMediaManage_CloseWaveFile( void );
static void USBMusicManager_AssignCCKWaveFormat(void * WaveFormat) ;
static void USBMediaManage_CCKPlay(void);

static void USBMediaManager_PlayBack_Reset( void ) ;

//____________________________________________________________________________________________________________
extern CCK_MANAGE_OBJECT *pCCKManage_ObjCtrl;

const USB_MEDIA_MANAGE_OBJECT   USBMediaManage =
{
    USBMediaManage_Init,
    USBMediaManage_Play,
    USBMediaManage_Pause,
    USBMediaManage_Stop,
    USBMediaManage_RePlay,
    USBMusicManager_AssignUSBFileList,
    USBMediaManager_ServiceManager,
    USBMediaManage_FetchQueue,
    USBMediaManage_ReqWaveSong,
    USBMediaManage_IsEndOfSong,
    USBMusicManager_AssignCCKWaveFormat,
    USBMediaManage_CCKPlay
};
const USB_MEDIA_MANAGE_OBJECT *pUSBMedia_ObjCtrl = &USBMediaManage;

//____________________________________________________________________________________________________________
/*static variable*/
static xUSBMediaManagerState USBMediaManagerState = USB_MEDIA_IDLE;
static xUSBMediaManagerState USBMediaManagerLastState = USB_MEDIA_IDLE;

static FSE_FileList* mMediaManagerUSBFileList;
static FIL waveFilefatfs;

static uint8_t waveReadBuffer[WAVE_READ_BUFFER_SIZE] ={0};
static uint16_t   wave_read_size = 0;

xUSBMediaManagerQueue USBMediaQueue;

static uint8_t mPlayWaveNum = 0;
static bool USBMediaPlaySuspend = FALSE;

static FSE_WaveFormat* mCCKWaveFormat;
static xWaveFileIndicator mWaveFileIndicator;
static uint8_t Tmp_RAM_Buf[WAVE_READ_BUFFER_SIZE];
static bool mCCKPlay = FALSE;

uint8_t EmptyBuffer[WAVE_FILE_BUFFER_SIZE];
uint16_t EmptyBufferSize = WAVE_FILE_BUFFER_SIZE;

//____________________________________________________________________________________________________________

static void USBMediaManage_Init(void)
{
    USBMediaManagerState = USB_MEDIA_IDLE;
    MEMSET(&USBMediaQueue, 0, sizeof(USBMediaQueue));
    MEMSET(&mWaveFileIndicator, 0, sizeof(mWaveFileIndicator));
}

static void USBMediaManage_Play(void)
{
    USBMediaManagerState = USB_MEDIA_OPEN_FILE;
}

static void USBMediaManage_CCKPlay(void)
{
    USBMediaManagerState = USB_MEDIA_OPEN_FILE;
    mCCKPlay = TRUE;
}


static void USBMediaManage_Pause(void)
{
    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_USB_POP_DELAY_RELEASE); //Fix issue for usb pause/play will cause pop sound (Angus 2014/12/22)
    USBMediaManagerLastState = USBMediaManagerState;
    USBMediaManagerState = USB_MEDIA_PAUSE;
}

static void USBMediaManage_RePlay(void)
{
    if( USBMediaManagerLastState == USB_MEDIA_PLAY_FILE)
    {
        USBMediaManagerState = USB_MEDIA_INIT_PLAY_FILE;
    }
    else if(USBMediaManagerLastState == USB_MEDIA_FINISH || USBMediaManagerLastState == USB_MEDIA_IDLE)
    {
        USBMediaManagerState = USB_MEDIA_OPEN_FILE;
    }
    else
    {
        USBMediaManagerState = USBMediaManagerLastState;
    }
}

static void USBMediaManage_Stop(void)
{
    Audio_MAL_Stop();
    USBMediaManage_CloseWaveFile();
    USBMediaManagerState = USB_MEDIA_STOP;
    mCCKPlay = FALSE;
}


static void USBMediaManage_ReqWaveSong(uint8_t num)
{
    mPlayWaveNum = num;
}


static bool USBMediaManage_IsEndOfSong(void)
{
    if(USBMediaManagerState == USB_MEDIA_FINISH)
        return TRUE;
    
    return FALSE;
}
static void USBMusicManager_AssignUSBFileList(void * FileList) 
{
    mMediaManagerUSBFileList = (FSE_FileList*)FileList;
}

static void USBMusicManager_AssignCCKWaveFormat(void * WaveFormat) 
{
    mCCKWaveFormat = (FSE_WaveFormat*)WaveFormat;
}

static ErrorCode USBMediaManage_OpenWaveFile( char *file_path )
{    
    FRESULT res = FR_OK; 

    res = f_open(&waveFilefatfs, file_path,FA_OPEN_EXISTING | FA_READ);

    if( res != FR_OK)
    {

        return Unvalid_WAVE_Format;
    }

    return Valid_WAVE_File ;
}

static FRESULT USBMediaManage_CloseWaveFile( void )
{    
    FRESULT res = FR_OK; 

    res = f_close(&waveFilefatfs);

    return res ;
}

void  USBMediaManage_MonoToStero(uint8_t src[],uint8_t dest[],int size)
{
    int i;
    for(i=0;i<size;i+=2)
    {
        dest[i*2] = src[i];
        dest[i*2+2] = src[i];
        dest[(i+1)*2 -1] = src[i+1];
        dest[(i+1)*2 +1] = src[i+1];
    }
}

FRESULT USBMediaManage_ReadWaveFile(void)
{
    FRESULT res = FR_OK; 
    if(mCCKPlay == FALSE)
    {
        res = f_read(&waveFilefatfs, waveReadBuffer, WAVE_FILE_BUFFER_SIZE, (void *)&wave_read_size );

        if(f_eof(&waveFilefatfs))
        {
            res = FR_NO_FILE;
        }
    }
    else
    {
        if(mWaveFileIndicator.Seeker >= mCCKWaveFormat->DataSize)    /* Angus debug 2013/12/25 */	
        {
#if CHK_POP_SOLUTION //Fix issue for play channel check will cause pop sound (Angus 2013/10/29)
            pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_CHK_VOLUME_DELAY_RELEASE);
#endif
            return FR_NO_FILE;
        }

        if(mCCKWaveFormat->NumChannels == CHANNEL_MONO)
        {
            if(mWaveFileIndicator.ChunkIndex < mWaveFileIndicator.ChunkSize )
            {                                                                                                               
                sFLASH_ReadBuffer(Tmp_RAM_Buf, mWaveFileIndicator.Seeker+pCCKManage_ObjCtrl->Get_NoiseFile_Start_Addr(), (WAVE_READ_BUFFER_SIZE/2));
                USBMediaManage_MonoToStero(Tmp_RAM_Buf,waveReadBuffer,(WAVE_READ_BUFFER_SIZE/2));
                wave_read_size = WAVE_READ_BUFFER_SIZE;
                res = FR_OK;
        
                mWaveFileIndicator.Seeker = (mWaveFileIndicator.Seeker + (WAVE_READ_BUFFER_SIZE/2)); //next chunk entry pointer//
                mWaveFileIndicator.ChunkIndex +=1;
            }
            else
            {                                                                                                               
                sFLASH_ReadBuffer(Tmp_RAM_Buf, mWaveFileIndicator.Seeker+pCCKManage_ObjCtrl->Get_NoiseFile_Start_Addr(), ((mCCKWaveFormat->DataSize)%(WAVE_READ_BUFFER_SIZE/2)));
                USBMediaManage_MonoToStero(Tmp_RAM_Buf,waveReadBuffer,(WAVE_READ_BUFFER_SIZE/2));
                wave_read_size = ((mCCKWaveFormat->DataSize)%(WAVE_READ_BUFFER_SIZE));
                res = FR_OK;
        
                mWaveFileIndicator.Seeker = (mWaveFileIndicator.Seeker + (mCCKWaveFormat->DataSize)%(WAVE_READ_BUFFER_SIZE/2));
            }
        }
        else
        {
            if(mWaveFileIndicator.ChunkIndex < mWaveFileIndicator.ChunkSize )
            {
                sFLASH_ReadBuffer(waveReadBuffer, mWaveFileIndicator.Seeker, (WAVE_READ_BUFFER_SIZE));
                wave_read_size = WAVE_READ_BUFFER_SIZE;
                res = FR_OK;
        
                mWaveFileIndicator.Seeker = (mWaveFileIndicator.Seeker + (WAVE_READ_BUFFER_SIZE)); //next chunk entry pointer//
                mWaveFileIndicator.ChunkIndex +=1;
            }
            else
            {
                sFLASH_ReadBuffer(waveReadBuffer, mWaveFileIndicator.Seeker, ((mCCKWaveFormat->DataSize)%(WAVE_READ_BUFFER_SIZE)));
                wave_read_size = ((mCCKWaveFormat->DataSize)%(WAVE_READ_BUFFER_SIZE));
                res = FR_OK;
                
                mWaveFileIndicator.Seeker = (mWaveFileIndicator.Seeker + (mCCKWaveFormat->DataSize)%(WAVE_READ_BUFFER_SIZE));
            }
        }
    }
    
    return res;
}

QueueState USBMediaManage_InsertQueue(void)
{
    FRESULT res = FR_OK; 
#if 0 // David, fix warning.
    int idx = 0;
#endif
    
    //if(USBMediaQueue.count >= MAX_BUFFER_NUM)
    if ((USBMediaQueue.getIndex == 0)
        &&((USBMediaQueue.setIndex) == (MAX_BUFFER_NUM -1)))
    {
        return QueueFull;
    }
    else if ((USBMediaQueue.getIndex > USBMediaQueue.setIndex )
        &&((USBMediaQueue.getIndex - USBMediaQueue.setIndex) == 1))
    {
        return QueueFull;
    }

    res = USBMediaManage_ReadWaveFile();
    
    if(res == FR_OK)
    {
        
#if 0 //remove the temp solution for USB playback
        for (idx = 0 ; idx < wave_read_size ; idx++)
        {
            if (waveReadBuffer[idx] == 0)
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (idx == wave_read_size)
        {
            //empty frame
            counter_empty++;
        }
        else
        {
            counter_empty = 0;
        }

        if (counter_empty >= MAX_EMPTY_FRAME)
        {
            //reset
            USBMediaManager_PlayBack_Reset();
            return QueueInsertOK;
        }
#endif        
        
        USBMediaQueue.bufferReadSize[USBMediaQueue.setIndex] = wave_read_size;
        MEMCPY((byte *)&USBMediaQueue.QueueBuffer[USBMediaQueue.setIndex][0],(byte *)&waveReadBuffer ,wave_read_size);

        if(USBMediaQueue.setIndex == MAX_BUFFER_NUM-1)
        {
            USBMediaQueue.setIndex = 0;
        }
        else
        {
            USBMediaQueue.setIndex++;
        }

        if (USBMediaQueue.count <= MAX_BUFFER_NUM)
        {
            USBMediaQueue.count++;
        }
        return QueueInsertOK;
    }
    else
    {
#if 0   //Fix issue for play channel check will cause pop sound
        if(mCCKPlay)
        {   SONG_END_CNT++;
            //USBMediaSystemParams.sys_event = SYS_EVENT_CHK_VOLUME_DELAY_RELEASE;
            //pASH_ObjCtrl->SendEvent(&USBMediaSystemParams);

            pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_CHK_VOLUME_DELAY_RELEASE);
        }
#endif
        USBMediaManager_PlayBack_Reset();
        return QueueInsertError;
    }
}

static QueueState USBMediaManage_FetchQueue(void)
{

    //if(USBMediaQueue.count <= 0)
    if(USBMediaQueue.setIndex == USBMediaQueue.getIndex)
    {
        Audio_MAL_Stop();
        return QueueEmpty;
    }   

    Audio_MAL_Play( (uint32_t)&(USBMediaQueue.QueueBuffer[USBMediaQueue.getIndex]),(USBMediaQueue.bufferReadSize[USBMediaQueue.getIndex]/2) );

    if(USBMediaQueue.getIndex == MAX_BUFFER_NUM-1)
    {
        USBMediaQueue.getIndex = 0;
    }
    else
    {
        USBMediaQueue.getIndex++;
    }

    if (USBMediaQueue.count > 0)
    {
        USBMediaQueue.count--;
    }

    return QueueFetchOK;

}

static QueueState USBMediaManage_FetchEmptyQueue(void)
{
    memset(EmptyBuffer, 0, EmptyBufferSize);
    Audio_MAL_Play( (uint32_t)&(EmptyBuffer[0]), EmptyBufferSize);
    
    return QueueFetchOK;
}

static void USBMediaManager_ServiceManager(void)
{
    ErrorCode err;
    static uint32 sampleRate;
    QueueState resQueue;
    int i;
       
    switch(USBMediaManagerState)
    {
        case USB_MEDIA_IDLE:
        {

        }
        break;
        case USB_MEDIA_OPEN_FILE:
        {
            if(mCCKPlay == FALSE)
            {
                err = USBMediaManage_OpenWaveFile(mMediaManagerUSBFileList->fileData[mPlayWaveNum].filePath);  

                if(err == Valid_WAVE_File)
                {
                    USBMediaManagerState = USB_MEDIA_INIT_I2S;
                    sampleRate = mMediaManagerUSBFileList->fileData[mPlayWaveNum].SampleRate;
                }
            }
            else
            {
                MEMSET(&mWaveFileIndicator, 0, sizeof(mWaveFileIndicator)); 

                if(mCCKWaveFormat->NumChannels == CHANNEL_MONO)
                {
                    mWaveFileIndicator.ChunkSize = (mCCKWaveFormat->DataSize /(WAVE_READ_BUFFER_SIZE/2));
                }
                else
                {
                    mWaveFileIndicator.ChunkSize = (mCCKWaveFormat->DataSize /(WAVE_READ_BUFFER_SIZE));
                }

                USBMediaManagerState = USB_MEDIA_INIT_I2S;
                sampleRate = mCCKWaveFormat->SampleRate;
            }
        }
        break;

        case USB_MEDIA_INIT_I2S:
        {
   
            switch( sampleRate)
            {
                case SAMPLE_RATE_8000:
                    Audio_MAL_I2S_SampleRateConfigure( SAMPLE_RATE_8000 );
                    break;
                case SAMPLE_RATE_11025:
                    Audio_MAL_I2S_SampleRateConfigure( SAMPLE_RATE_11025 );
                    break;
                case SAMPLE_RATE_16000:
                    Audio_MAL_I2S_SampleRateConfigure( SAMPLE_RATE_16000 );
                    break;
                case SAMPLE_RATE_22050: 
                    Audio_MAL_I2S_SampleRateConfigure( SAMPLE_RATE_22050 );
                    break;
                case SAMPLE_RATE_32000:
                    Audio_MAL_I2S_SampleRateConfigure( SAMPLE_RATE_32000 );
                    break;
                case SAMPLE_RATE_44100:
                    Audio_MAL_I2S_SampleRateConfigure( SAMPLE_RATE_44100 );
                    break;
                case SAMPLE_RATE_48000: 
                    Audio_MAL_I2S_SampleRateConfigure( SAMPLE_RATE_48000 );
                    break;
            }

            MEMSET(&USBMediaQueue, 0, sizeof(USBMediaQueue));
            USBMediaManagerState = USB_MEDIA_INIT_PLAY_FILE;

        }
        break;
        
        case USB_MEDIA_INIT_PLAY_FILE:
        {   
 
            resQueue = USBMediaManage_InsertQueue();

            if(resQueue == QueueFull)
            {
#if CHK_POP_SOLUTION //Fix issue for play channel check will cause pop sound (Angus 2013/10/29)
                if(mCCKPlay)
                {  
                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_CHK_VOLUME_DELAY_RELEASE);
                }
                else
                {
                    pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_USB_POP_DELAY_RELEASE); //Fix issue for usb pause/play will cause pop sound (Angus 2014/12/22)
                }
#endif           
                //USBMediaManage_FetchQueue();
                USBMediaManage_FetchEmptyQueue();
                USBMediaManagerState = USB_MEDIA_PLAY_FILE;
            }
            else if(resQueue == QueueInsertError)
            {
                USBMediaManagerState = USB_MEDIA_IDLE;
                USBMediaManage_CloseWaveFile();
            }        

        }
        break;
        case USB_MEDIA_PLAY_FILE:
        {
  
            /* Precaution Step1 for breaking off DMA ISR : I2S transmission.*/
            //if(USBMediaQueue.count == 0)
            if(USBMediaQueue.setIndex == USBMediaQueue.getIndex)
            {
                USBMediaPlaySuspend = TRUE;
            }
            
            
            //resQueue = USBMediaManage_InsertQueue();          
            //if(USBMediaQueue.count < MAX_BUFFER_NUM/2)
            {

                for(i=0;i<MAX_SUPPLE_BUFFER_NUM;i++)
                {
                    resQueue = USBMediaManage_InsertQueue();
                    if(resQueue ==QueueInsertError ||resQueue ==QueueFull)
                        break;
                }
            }


            /* Precaution Step2 for breaking off DMA ISR : I2S transmission.*/
            if(USBMediaPlaySuspend)
            {
                USBMediaPlaySuspend = FALSE;
                //USBMediaManage_FetchQueue();
                USBMediaManage_FetchEmptyQueue();
            }
            
            if(resQueue == QueueInsertError)
            {
                USBMediaManagerState = USB_MEDIA_STOP;
                USBMediaManage_CloseWaveFile();
            }
        }
        break;
        case USB_MEDIA_PAUSE:
        {
            /* Don't do anything for PAUSE play wave music. */
        }
        break;
        
        case USB_MEDIA_STOP:
        {
            Audio_MAL_Stop();
            USBMediaManagerState = USB_MEDIA_FINISH;
            mCCKPlay = FALSE;
        }
        break;
        case USB_MEDIA_FINISH:
        {
            
        }
        break;
    }
}

static void USBMediaManager_PlayBack_Reset( void ) 
{
    int idx = 0;
    
    USBMediaQueue.count = 0;
    USBMediaQueue.getIndex= 0;
    USBMediaQueue.setIndex= 0;
#if 1    
    for(idx = 0 ; idx < MAX_BUFFER_NUM ; idx++)
    {
        USBMediaQueue.bufferReadSize[idx] = 0;
        MEMSET((byte *)&USBMediaQueue.QueueBuffer[idx][0],0 ,wave_read_size);
    }
#endif    
}

