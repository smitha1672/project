#include "fileSearchEngine.h"
#include "ext_flash_driver.h"
#include "fileSearchEngine.h"
#include "NoiseGeneratorDispatcher.h"
#include "VirtualTimer.h"
#include "AudioFormatDispatcher.h"
#include "UserParameterManager.h"
#include "AudioSystemManager.h"

#include "usbMediaHandler.h"


#define NOISE_GENERATOR_EVENT_PERIOD VIRTUAL_TIMER_MSEC2TICKS(100)
#define NOISE_OUTPUT_TIME 25 // 2.5 sec
/*_________________________________________________________________________________________________*/
//extern
extern FSE_FormatCheck FSE_Is_Valid_Wav_File(char* filePath, FSE_WaveFormat* fileFormat,int IsChannelCheck);
extern bool USB_Media_Play(void);
extern bool USB_Media_Stop(void);
extern void USB_Media_Init(void);
extern bool AudioSystem_Data_Output_Select( uint8 val );

/*_________________________________________________________________________________________________*/
//private
static NOISE_GENERATOR_CHECK_EVENT_ASSIGN mNoiseGeneratorCheckEvent = ASSIGN_EVENT_IDLE;
static uint16 mChannelCheckStamp = 0;
static NoiseOutputChannel    mNoiseOutputChannel = NOISE_OUTPUT_NULL;
static uint32 mNoiseInFlashStartAddr = 0;
static uint32 mNoiseInFlashMemAddr[6]={0,0,0,0,0,0};

static bool mNoiseGeneratorMute = FALSE;
void NoiseGenerator_MuteChannel(NoiseOutputChannel Channel,bool MuteState);

/*_________________________________________________________________________________________________*/
FSE_WaveFormat NoiseGeneratorFileList;
uint8 AMT_RAM_Buf[WaveHeaderSize] ={ 0x00 };


void setNoiseGeneratorChannel(NoiseOutputChannel Channel)
{   
    if(Channel == NOISE_OUTPUT_LEFT_CHANNEL)
    {
        mNoiseInFlashStartAddr = mNoiseInFlashMemAddr[Index_L_StartAddr];
        AudioSystem_Data_Output_Select(ROUTER_FL);
    }
    else if(Channel == NOISE_OUTPUT_RIGHT_CHANNEL)
    {
        mNoiseInFlashStartAddr = mNoiseInFlashMemAddr[Index_R_StartAddr];   
        AudioSystem_Data_Output_Select(ROUTER_FR); 
    }
    else if(Channel == NOISE_OUTPUT_CENTER_CHANNEL)
    {
        mNoiseInFlashStartAddr = mNoiseInFlashMemAddr[Index_C_StartAddr]; 
        AudioSystem_Data_Output_Select(ROUTER_CENTER); 
    }
    else if(Channel == NOISE_OUTPUT_LEFT_REAR_CHANNEL)
    {
        mNoiseInFlashStartAddr = mNoiseInFlashMemAddr[Index_Ls_StartAddr]; 
        AudioSystem_Data_Output_Select(ROUTER_RL);
    }
    else if(Channel == NOISE_OUTPUT_RIGHT_REAR_CHANNEL)
    {
        mNoiseInFlashStartAddr = mNoiseInFlashMemAddr[Index_Rs_StartAddr]; 
        AudioSystem_Data_Output_Select(ROUTER_RR);
    }
    else if(Channel == NOISE_OUTPUT_SUBWOOFER_CHANNEL)
    {
        mNoiseInFlashStartAddr = mNoiseInFlashMemAddr[Index_W_StartAddr]; 
        AudioSystem_Data_Output_Select(ROUTER_LFE0);
    }
    else
    {
        AudioSystem_Data_Output_Select(ROUTER_INITIAL);
    }

}




void NoiseGeneratorDispatcher_EventAssign(NOISE_GENERATOR_CHECK_EVENT_ASSIGN event)
{
    UserParameter UserParam;
    UserParameter_GripParam(&UserParam);
    
    if(event == ASSIGN_END_NOISE)
    {
        if(!UserParam.bNoiseMode)
        {
            return;
        }
        AudioSystem_Data_Output_Select(ROUTER_INITIAL);
        Leave_NoiseGeneratorMode();
    }
    else if(event == ASSIGN_INIT_NOISE)
    {
        Enter_NoiseGeneratorMode();
    }

    USB_Media_Stop();
    MEMSET(AMT_RAM_Buf,0xff,sizeof(AMT_RAM_Buf));
    MEMSET(&NoiseGeneratorFileList, 0, sizeof(FSE_WaveFormat));
    
    
    mNoiseOutputChannel = NOISE_OUTPUT_NULL;
    mNoiseGeneratorCheckEvent = event;
    mChannelCheckStamp = 0;
}

void NoiseGeneratorDispatcher_poll(void)
{   
    static uint32 lastTime = 0;
    FSE_FormatCheck amt_check; 
    char path[FS_MAX_PATH_LENGTH] = {0};
    int i;
    
    if (VirtualTimer_now() - lastTime < NOISE_GENERATOR_EVENT_PERIOD) /*execute this process on every 100 ms*/
    {
        return;
    }

    lastTime = VirtualTimer_now();
    
    switch(mNoiseGeneratorCheckEvent)
    {
        case ASSIGN_EVENT_IDLE:
        {
            mChannelCheckStamp = 0;
        }
        break;

        case ASSIGN_INIT_NOISE:
        {
            mChannelCheckStamp++;
             /* It will leak sound if set NoiseGeneratorChannel straight after it switch to Noise source,Angus 2014/1/7 */
            if(mChannelCheckStamp >= 2)
            {
                mNoiseGeneratorCheckEvent = 0;
                mNoiseGeneratorCheckEvent = ASSIGN_START_NOISE;
            }
            
        }
        break;
        
        case ASSIGN_START_NOISE:
        {
            mChannelCheckStamp = 0;
            
            if(mNoiseOutputChannel < NOISE_OUTPUT_SUBWOOFER_CHANNEL)
            {
                mNoiseOutputChannel++;

                setNoiseGeneratorChannel(mNoiseOutputChannel);
           
                for(i=0;i<10;i++)    /* sometime Read the first data = '0xFF' from Flash */
                {
                    sFLASH_ReadBuffer(AMT_RAM_Buf, getNoiseGeneratorStartAddr(), WaveHeaderSize);
                    amt_check = FSE_Is_Valid_Wav_File(path, &(NoiseGeneratorFileList),1);
                    
                    if(amt_check == FSE_Valid_WAVE_File)
                        break;
                }

                if(amt_check == FSE_Valid_WAVE_File)
                {          
                    USB_Media_Init();
                    mNoiseGeneratorCheckEvent = ASSIGN_PLAY_NOISE;
                }
                else
                {
                    mNoiseGeneratorCheckEvent = ASSIGN_END_NOISE;   /* It will stop noise generator if no valid wave file in FLASH (Angus 2014/1/9)*/
                }         
            }
            else
            {
                 mNoiseGeneratorCheckEvent = ASSIGN_END_NOISE;
            }
        }
        break;

        case ASSIGN_PLAY_NOISE:
        {

            if(mChannelCheckStamp == 0)
            {
                USB_Media_Play();
            }
            else if(mChannelCheckStamp >= NOISE_OUTPUT_TIME)
            {
                mNoiseGeneratorCheckEvent = ASSIGN_STOP_NOISE;
            }
            else
            {
                if(USB_Media_isPlayFinish())
                {
                    USB_Media_Play(); 
                 }
            }

            mChannelCheckStamp++;
        }
        break;
        
        case ASSIGN_STOP_NOISE:
        {
            USB_Media_Stop();            
            mNoiseGeneratorCheckEvent = ASSIGN_START_NOISE;
        }
        break;

        case ASSIGN_END_NOISE:
        {
            USB_Media_Stop();  
            NoiseGenerator_SetMute(FALSE);  /* Avoid pop sound while play wave from FLASH */
            
            AudioSystem_Data_Output_Select(ROUTER_INITIAL);
            Leave_NoiseGeneratorMode();
            mNoiseGeneratorCheckEvent = ASSIGN_EVENT_IDLE;
        }
        break;
    }
}



void getNoiseGeneratorMemAddr(void)
{   
    uint8 i;   uint32 addr = 0;
    char path[FS_MAX_PATH_LENGTH] = {0};
    FSE_FormatCheck check; 
    
    for(i=1;i<TotalCheckFile;i++)
    {
        MEMSET(AMT_RAM_Buf,0xff,sizeof(AMT_RAM_Buf));
        sFLASH_ReadBuffer(AMT_RAM_Buf, addr, WaveHeaderSize);
        check = FSE_Is_Valid_Wav_File(path, &(NoiseGeneratorFileList),1);
        
        if(check == FSE_Valid_WAVE_File)
        {
            addr += NoiseGeneratorFileList.RIFFchunksize + BYTE_OF_ChunkID + BYTE_OF_ChunkSize;
            mNoiseInFlashMemAddr[i] = addr;
        }
    }
    
}


uint32 getNoiseGeneratorStartAddr(void)
{
    return mNoiseInFlashStartAddr;
}


void NoiseGenerator_SetMute(bool state)
{

    uint8 vol;
    bool muteState;
    UserParameter UserParam;
    UserParameter_GripParam(&UserParam);

    if(state == TRUE && mNoiseGeneratorMute == FALSE)
    {
        vol = 0;
        muteState = TRUE;   mNoiseGeneratorMute = TRUE;
        
        AudioSystemManager_handleCommand(AUDIO_SET_VOL, &vol);
        AudioSystemManager_handleCommand(AUDIO_SET_MUTE, &muteState);
    }
    else
    {
        if(UserParam.bSoundMute == FALSE && mNoiseGeneratorMute == TRUE)
        {
            vol = UserParam.SoundBarLevel;
            muteState = FALSE;  mNoiseGeneratorMute = FALSE;
            
            AudioSystemManager_handleCommand(AUDIO_SET_MUTE, &muteState);
            AudioSystemManager_handleCommand(AUDIO_SET_VOL, &vol);
        }
    }

}


