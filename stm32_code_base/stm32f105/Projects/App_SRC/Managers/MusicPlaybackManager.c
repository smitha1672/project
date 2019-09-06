#include "MusicPlaybackManager.h"
#include "stm3210c_usb_audio_codec.h"

#include "MusicPlaybackDispatcher.h"
#include "OTI3368_playback_driver.h"
#include "UserParameterManager.h"
#include "GPIOMiddleLevel.h"


#include "Debug.h"
#include "usb_host_device.h"
//! < External Application_________________________________________________@{
//These applicatons is only used on here.
extern void MSCPB_Dispatcher_setState(MSCPB_State state);
extern void MSCPB_SetMute(void);

//! @}
#define START_STOP 0

/*_________________________________________________________________________________________________________*/
bool deviceMount = FALSE;
/*_________________________________________________________________________________________________________*/
void MusicPlayback_handleCommand(
    uint8 command,
    void *params
)
{
    UserParameter Parameter;

    switch (command)
    {
        case PLAYBACK_DEVICE_PLAY_STOP:
        {
#if 0			
            OTI_PlayStatus PlayStatus;

            if (OTI3368_PlayPosition(&PlayStatus) != TRUE)
            {
                return;
            }

            TRACE_DEBUG((0, "PlayStatus.PlayStatus = 0x%X", PlayStatus.PlayStatus));

            if (PlayStatus.PlayStatus == PLAY_STATUS_PLAY)
            {
                //OTI3368_PlayingStop( );
                //MSCPB_Dispatcher_setState( MSCPB_STATE_GET_DEVICE_STATUS );
                OTI3368_Pause();
                TRACE_DEBUG((0, "PAUSE"));

                return;

            }

            if (PlayStatus.PlayStatus == PLAY_STATUS_PAUSE)
            {
                //OTI3368_PlayingStop( );
                //MSCPB_Dispatcher_setState( MSCPB_STATE_GET_DEVICE_STATUS );
                OTI3368_RePlayFile();
                TRACE_DEBUG((0, "PLAY"));

                return;
            }

            if (PlayStatus.PlayStatus == PLAY_STATUS_END)
            {
                MSCPB_Dispatcher_setState(MSCPB_STATE_AUTO_PLAY);
                TRACE_DEBUG((0, "PLAY"));

                return;
            }
#endif 	

		MSCPB_Change_Play_State();
#if START_STOP
		MSCPB_SetMute();
		MSCPB_Dispatcher_setState(MSCPB_STATE_START_STOP);
#else
		MSCPB_SetMute();
		MSCPB_Dispatcher_setState(MSCPB_STATE_START_PAUSE);
#endif
        }
        break;

        //scott, add for demo2 play
        case PLAYBACK_DEVICE_DEMO2_PLAY:
        {
#if 0			
            if(canReceivePlayCmd == TRUE)
            {
                TRACE_DEBUG((0, "\033[1;33mDEMO2 PLAY, deviceMount = %d\033[0m", deviceMount));
                if(deviceMount == FALSE)
                {
                    
                    VirtualComPort_Disconnected();
                    OTI3368_DeviceMount(ENABLE);
                    
                    MSCPB_ResetDemo2Counter();
                    deviceMount = TRUE;
                }
                else
                {
                    //MSCPB_Dispatcher_setState(MSCPB_STATE_AUTO_PLAY);
                    //need set new state in MSCPB_STATE_DEMO2_TIME_OUT
                    //note: MUST ASSIGN mPB_state!!! or it will stuck in MSCPB_STATE_DEMO2_TIME_OUT
                    OTI_PlayStatus PlayStatus;
                    OTI3368_PlayPosition(&PlayStatus);

                    TRACE_DEBUG((0, "PlayStatus.PlayStatus = 0x%X", PlayStatus.PlayStatus));

                    if(PlayStatus.PlayStatus == PLAY_STATUS_PAUSE)
                    {
                        OTI3368_RePlayFile();
                    }    
                        
                    MSCPB_ResetDemo2Counter();
                }
                canReceivePlayCmd = FALSE;
            } 
#endif 

		//if it havn't be stop in timeout state ,user pushes play button that will not do anything.
		if(MSCPB_Get_Demo2_Timeout_State() == TRUE){
			MSCPB_SetMute();
			MSCPB_ReSet_Demo2_Timeout_State();
			MSCPB_Dispatcher_setState(MSCPB_STATE_DEMO2_WAIT_PLAY_CMD);
		}
        }
        break;

        //scott, add for demo2 stop, enter time out mode
        case PLAYBACK_DEVICE_DEMO2_STOP:
        {
		MSCPB_SetMute();
		MSCPB_Dispatcher_setState(MSCPB_STATE_DEMO2_TIME_OUT);		
		TRACE_DEBUG((0, "\033[1;33mDEMO2 STOP\033[0m"));
        }
        break;

        case PLAYBACK_DEVICE_NEXT:
        {		
	     	MSCPB_SetMute();
            	MSCPB_Dispatcher_setState(MSCPB_STATE_NEXT);
            	TRACE_DEBUG((0, "NEXT"));
        }
        break;

        case PLAYBACK_DEVICE_PREVIOUS:
        {
		MSCPB_SetMute();
             	MSCPB_Dispatcher_setState(MSCPB_STATE_PRIVIOUS);
            	TRACE_DEBUG((0, "PRIVIOUS"));
        }
        break;

        case AUDIO_SET_INPUT:
        {
            //if in demo2, do not auto play
            UserParameter_GripParam(&Parameter);
            if(Parameter.SystemMode == MODE_DEMO_2)
            {
                deviceMount = FALSE;
                MSCPB_ResetDemo2Counter();
                //return;
            }
            
            AUDIO_SOURCE *pSrc;

            pSrc = (AUDIO_SOURCE *)params;

            if(*pSrc == AUDIO_SOURCE_NOISE_CHECK)
            {
                Audio_MAL_I2S_Pin_Init();
                USB_HOST_DEVICE_Switch( USB_HOST_DEVICE_CMD_VCP_CONNECT );
                MSCPB_DispatcherController(FALSE);
            }
            else if(*pSrc == AUDIO_SOURCE_USB_PLAYBACK)
            {
                Audio_MAL_I2S_Pin_Init();
                USB_HOST_DEVICE_Switch( USB_HOST_DEVICE_CMD_MSC_PROCESS );
                MSCPB_DispatcherController(TRUE);
            }
            else
            {
                USB_HOST_DEVICE_Switch( USB_HOST_DEVICE_CMD_VCP_CONNECT );
                MSCPB_DispatcherController(FALSE);
#if ( configI2S_OUT == 1 )			
                Audio_MAL_I2S_Pin_PullDown();	
#endif 
            }
        }
        break;

        case SYSTEM_POWER_DOWN:
        {
		
            UserParameter_GripParam(&Parameter);

            if (Parameter.AudioSource == AUDIO_SOURCE_USB_PLAYBACK)
            {
		  USB_HOST_DEVICE_Switch( USB_HOST_DEVICE_CMD_VCP_CONNECT );			
                MSCPB_Dispatcher_setState(MSCPB_STATE_GET_DEVICE_STATUS);
                MSCPB_DispatcherController(FALSE);
            }

		/* Angus add,modify Sometimes SB can't re-connect with my BT device.   
		   It needs pull down pines of I2S.    */
#if ( configI2S_OUT == 1 )			
		 Audio_MAL_I2S_Pin_PullDown(); 
#endif 

        				
        }
        break;

        //same with SYSTEM_POWER_DOWN case
        case PLAYBACK_DEVICE_UNMOUNT_IN_DEMO2:
        {
            if(deviceMount == TRUE)
            {
                MSCPB_Dispatcher_setState(MSCPB_STATE_GET_DEVICE_STATUS);
                MSCPB_DispatcherController(FALSE);
            }
        }
        break;
    }
}

