#include "CS498xx.h"
#include "Debug.h"
#include "CS498xx_spi.h"
#include "STM32F0xx_board.h"
#include "StorageEEPROM.h"

//Smith Implemented
#include "audio_dsp_uld.h"
#include "audio_gain_parameters.h"

#include "AudioSystemDriver.h"
#include "UserParameterManager.h"

//______________________________________________________________________________
static uint32 mCMD;
static bool mAllChannelMuteStatus = SOUND_DEMUTE;

uint32 LeftChannelSignal;
uint32 RightChannelSignal;

//______________________________________________________________________________
#define AUTODETECTION                        0x81000000
#define AUTODETECTION_MASK                   0xFFFF0000

#define DECPDABLE_STREAM_FLAG                0x80000000
#define NON_IEC61937_STREAM_FLAG(VAL)        (VAL & 0x00000020)
#define NON_IEC61937_STREAM_DESCRIPTOR(VAL)  (VAL & 0x0000FFFF)

#define NON_IEC61937_SILENT_INPUT_DATA       0x0020
#define NON_IEC61937_DTS_FORMAT_16           0x0021
#define NON_IEC61937_DTS_FORMAT_14           0x0022
#define NON_IEC61937_LPCM_21_1FS             0x0023
#define NON_IEC61937_LPCM_51_1FS             0x0623
#define NON_IEC61937_LPCM_71_1FS             0x0823
#define NON_IEC61937_LPCM_21_2FS             0x4023
#define NON_IEC61937_LPCM_51_2FS             0x4623
#define NON_IEC61937_LPCM_71_2FS             0x4823
#define NON_IEC61937_LPCM_21_4FS             0xc023
#define NON_IEC61937_LPCM_51_4FS             0xc623
#define NON_IEC61937_LPCM_71_4FS             0xc823
#define NON_IEC61937_HDCD_PCM_SYNC_DETECT    0x0024
#define NON_IEC61937_HDCD_PCM_SYNC_LOST      0x0025

#define IEC61937_STREAM_DESCRIPTOR(VAL)      (VAL & 0x0000004F)
#define IEC61937_NEVER_REPORTED_00           0x00
#define IEC61937_AC3                         0x01
#define IEC61937_NEVER_REPORTED_03           0x03
#define IEC61937_MPGE_1_L1                   0x04
#define IEC61937_MPGE_1_L23                  0x05 /* MPEG-1 Layer 2 or 3 data or MPEG-2 without extension.*/
#define IEC61937_MPGE_2_EXTENSTION           0x06 /* MPEG-2 data with extension.*/
#define IEC61937_MPGE_2_AAC                  0x07 /* MPEG-2 AAC ADTS data*/
#define IEC61937_MPGE_2_L1                   0x08 /* MPEG-2 Layer 1 Low sampling frequency*/
#define IEC61937_MPGE_2_L23                  0x09 /* MPEG-2 Layer 2 or 3 Low sampling frequency*/
#define IEC61937_DTS_1                       0x0B /* DTS-1 data (512-sample bursts-DTS Type I).*/
#define IEC61937_DTS_2                       0x0C /* DTS-2 data (1024-sample bursts-DTS Type II).*/
#define IEC61937_DTS_3                       0x0D /* DTS-3 data (2048-sample bursts-DTS Type III)*/
#define IEC61937_DTS_4                       0x11 /* DTS-3 data (2048-sample bursts-DTS Type III)*/
#define IEC61937_DDP                         0x15 /* Dolby Digital Plus.*/
#define IEC61937_TRUEHD                      0x16 /* Dolby TRUEHD */
#define IEC61937_MPGE_2_AAC_ADTS             0x1C /* MPEG-2 AAC ADTS data */

#define IEC61937_TRUEHD_DESCRIPTOR(VAL)      (VAL & 0x0000F000)
#define IEC61937_TRUEHD_1FS                  0x0000
#define IEC61937_TRUEHD_2FS                  0x4000
#define IEC61937_TRUEHD_4FS                  0xC000

#define IEC61937_DTS_DESCRIPTOR(VAL)         (VAL & 0x00000F00)
#define IEC61937_DTS_HD_HIGH_RESOLUTION      0x0200
#define IEC61937_DTS_EXPRESS_LOW_BIT         0x0300
#define IEC61937_DTS_MASTER_AUDIO_W_CORE     0x0400
#define IEC61937_DTS_MASTER_AUDIO_WO_CORE    0x0C00

#define IEC61937_DTS_FS_DESCRIPTOR(VAL)         (VAL & 0x0000F000)
#define IEC61937_DTS_1FS                     0x0000
#define IEC61937_DTS_2FS                     0x4000
#define IEC61937_DTS_4FS                     0xC000

//______________________________________________________________________________
//! PCM @{
extern bool CS498xx_PCM_Input( void );
extern bool CS498xx_PCM_Initial( void);
extern void CS498xx_PCM_21_TRUVOLUME_Enable( bool enable );
extern void CS498xx_PCM_51_TRUVOLUME_Enable( bool enable );
extern void buffer_write_dsp_SRS_TSHD_ON(void);
extern void buffer_write_dsp_SRS_TSHD_OFF(void);
//!@}

//! AC3 @{
extern bool CS498xx_AC3_Input( void );
extern bool CS498xx_AC3_Initial( void );
//!@}

//extern uint8 Audio_System_format_status(void);
bool AudioStream_getAudioStreamFormat( uint8 *pAudioStream );
//______________________________________________________________________________
bool softreset( void )
{
    uint32 value = 0;

    mCMD = CMD_SOFT_RESET;
    /*Write BOOT*/
    CS498xx_SPI_write_buffer((byte*)&mCMD, (sizeof(mCMD)/sizeof(uint8)));
    
    VirtualTimer_sleep(100);
       /*Read message */
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if ( value != CMD_APP_START )
    {
           TRACE_ERROR((STR_ID_NULL, "CS498xx softreset fail"));

        return FALSE;
    }
    
    TRACE_INFO((STR_ID_NULL, "CS498xx softreset Success"));

    return TRUE;
    
}

void buffer_write_dsp_DataOutput_Normal(void)
{
    //CS49xxxxSPI_write_buffer((byte*)&Data_Output_Normal_CFG, (sizeof(Data_Output_Normal_CFG)/sizeof(uint8)));
    CS498xx_Mute(R_CHANNEL, FALSE); 
    CS498xx_Mute(W_CHANNEL, FALSE); 
    CS498xx_Mute(L_CHANNEL, FALSE); 
}

void buffer_write_dsp_DataOutput_Woofer(void)
{
    //CS49xxxxSPI_write_buffer((byte*)&Data_Output_Woofer_CFG, (sizeof(Data_Output_Woofer_CFG)/sizeof(uint8)));
    CS498xx_Mute(R_CHANNEL, TRUE); 
    CS498xx_Mute(L_CHANNEL, TRUE); 
    CS498xx_Mute(W_CHANNEL, FALSE); 
}

void buffer_write_dsp_DataOutput_L(void)
{
    //CS49xxxxSPI_write_buffer((byte*)&Data_Output_LR_CFG, (sizeof(Data_Output_LR_CFG)/sizeof(uint8)));
    CS498xx_Mute(R_CHANNEL, TRUE); 
    CS498xx_Mute(W_CHANNEL, TRUE); 
    CS498xx_Mute(L_CHANNEL, FALSE); 
}

void buffer_write_dsp_DataOutput_R(void)
{
    //CS49xxxxSPI_write_buffer((byte*)&Data_Output_LR_CFG, (sizeof(Data_Output_LR_CFG)/sizeof(uint8)));
    CS498xx_Mute(W_CHANNEL, TRUE); 
    CS498xx_Mute(L_CHANNEL, TRUE); 
    CS498xx_Mute(R_CHANNEL, FALSE); 
}

void buffer_write_dsp_SRS_VIRTUAL_OFF_AVC_OFF(void)
{
	CS498xx_SPI_write_buffer((byte*)&Virtual_OFF_AVC_OFF, (sizeof(Virtual_OFF_AVC_OFF)/sizeof(uint8)));
}

void buffer_write_dsp_SRS_VIRTUAL_OFF_AVC_ON(void)
{
	CS498xx_SPI_write_buffer((byte*)&Virtual_OFF_AVC_ON, (sizeof(Virtual_OFF_AVC_ON)/sizeof(uint8)));
}

void buffer_write_dsp_SRS_VIRTUAL_ON_AVC_OFF(void)
{
	CS498xx_SPI_write_buffer((byte*)&Virtual_ON_AVC_OFF, (sizeof(Virtual_ON_AVC_OFF)/sizeof(uint8)));
}

void buffer_write_dsp_SRS_VIRTUAL_ON_AVC_ON(void)
{
	CS498xx_SPI_write_buffer((byte*)&Virtual_OFF_AVC_ON, (sizeof(Virtual_OFF_AVC_ON)/sizeof(uint8)));
}

void  CS498xx_Mute(AUDIO_CHANNEL channel ,bool value)
{
    uint32 address=0;
    uint32 command;
 
    switch(channel)
    {
        case L_CHANNEL:
            address=0xf10001c2;
            if(value==TRUE)    
            {
                command=0x00000000;
            }else
            {
                command=0x20000000;
            }

            break;
        case R_CHANNEL:
            address=0xf10001c3;
            if(value==TRUE)    
            {
                command=0x00000000;
            }else
            {
                command=0x20000000;
            }
            break;
        case C_CHANNEL:
            address=0x83000003;
            if(value==TRUE)    
            {
                command=0x00000000;
            }else
            {
                command=0x80000000;
            }
            break;
        case LS_CHANNEL:
            address=0x83000005;
            if(value==TRUE)    
            {
                command=0x00000000;
            }else
            {
                command=0x80000000;
            }
            break;
        case RS_CHANNEL:
            address=0x83000006;
            if(value==TRUE)    
            {
                command=0x00000000;
            }else
            {
                command=0x80000000;
            }
            break;
        case W_CHANNEL:
            address=0xf10001c4;
            if(value==TRUE)    
            {
                command=0x00000000;
            }else
            {
                command=0x20000000;
            }
            break;
        case All_CHANNEL:
            address=0x83000001;
            if(value==TRUE)    
            {
                command=0x00000001;
                mAllChannelMuteStatus = SOUND_MUTE;
            }else
            {
                command=0x00000000;
                mAllChannelMuteStatus = SOUND_DEMUTE;
            }
            break;
            
    }
    
    CS498xx_SPI_write_buffer((byte*)&address, (sizeof(address)/sizeof(uint8)));
    CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));

}

void CS498xx_SetVolume(uint8 vol)
{
    uint32 address ;
    uint32 command;

    command=Volume_table[vol];
    TRACE_DEBUG((0,"CS498xx_SetVolume command = %X",command));
    address=MASTER_GAIN1;
    CS498xx_SPI_write_buffer((byte*)&address, (sizeof(address)/sizeof(uint8)));
    CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
}

void CS498xx_Set_SubVolume( uint8 vol )
{
    uint32 address ;
    uint32 command;

	command=SUB_PCM21_GAIN[vol];
    
    address=SUB_GAIN;
    CS498xx_SPI_write_buffer((byte*)&address, (sizeof(address)/sizeof(uint8)));
    CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
}


void CS498xx_Set_Treb( uint8 vol )
{
	uint32 address_L = TREBLE_LEFT;
	uint32 address_R = TREBLE_RIGHT;
	uint32 command;

	command=Treb_table[vol];
	
	//TRACE_DEBUG((STR_ID_NULL, "Treb = %X",command));
	CS498xx_SPI_write_buffer((byte*)&address_L, (sizeof(address_L)/sizeof(uint8)));
	CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));

	CS498xx_SPI_write_buffer((byte*)&address_R, (sizeof(address_R)/sizeof(uint8)));
	CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
}

void CS498xx_Set_Bass( uint8 vol )
{
	uint32 address_L = BASS_LEFT;
	uint32 address_R = BASS_RIGHT;
	uint32 command;

	command=Bass_table[vol];
	
	//TRACE_DEBUG((STR_ID_NULL, "Bass = %X",command));
	CS498xx_SPI_write_buffer((byte*)&address_L, (sizeof(address_L)/sizeof(uint8)));
	CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));

	CS498xx_SPI_write_buffer((byte*)&address_R, (sizeof(address_R)/sizeof(uint8)));
	CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
}

void CS498xx_BASS_MGR_Status(bool vol)
{
    uint32 address=0xd7000000;
    uint32 command;

    if(vol)
    {
        command =0x00000009;
    }
    else
    {
        command = 0x00000008;
    }
    
    CS498xx_SPI_write_buffer((byte*)&address, (sizeof(address)/sizeof(uint8)));
    CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));

}

void CS498xx_AQ_Enable(bool vol)
{
    uint32 address=PEQ_Enable;
    uint32 command;

    if(vol)
    {
        command =0x00000001;
    }
    else
    {
        command = 0x00000000;
    }
    
    CS498xx_SPI_write_buffer((byte*)&address, (sizeof(address)/sizeof(uint8)));
    CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
}

void CS498xx_Factory_AQBypass(bool val)
{
    if(val)
    {
        TRACE_INFO((0, "Factory_AQBypass_CFG Finished !! "));
        CS498xx_SPI_write_buffer((byte*)&Factory_AQBypass_CFG, (sizeof(Factory_AQBypass_CFG)/sizeof(uint8)));
    }
    else
    {
        TRACE_INFO((0, "Factory_NormalAQ_CFG Finished !! "));
        CS498xx_SPI_write_buffer((byte*)&Factory_NormalAQ_CFG, (sizeof(Factory_NormalAQ_CFG)/sizeof(uint8)));
    }
}

void CS498xx_Set_CenterGainSwitch(bool val)
{
  
}

//______________________________________________________________________________________________
//Smith implemented !!
bool CS498xx_readManualMuteStatus( void )
{
    return mAllChannelMuteStatus;
}

/*Smith 26 July: Read PRESENCE value should wait "PRESENCE_RESPONSE_LEFT" response ater read command*/
int CS498xx_readPresence(void)
{
    bool ret = FALSE;
    uint32 LeftChannelSignal;
    uint32 RightChannelSignal;
    uint32 command = 0;
    uint32 value;
    uint8 cnt = 0;

    command = CS498xx_CMD_READ_PRESENCE_LEFT;
    ret = CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
    if ( ret == FALSE )
        return -1; 

    do
    {
        /*Read response command word*/
        ret = CS498xx_SPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8));
        if ( ret == FALSE )
            return -1;

        if ( cnt >= 5 )
            return -1;

    }while( value != PRESENCE_RESPONSE_LEFT );


    ret = CS498xx_SPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8));
    if ( ret == FALSE )
        return -1;

    LeftChannelSignal = value;

    //________________________________________________________________@{
    command = CS498xx_CMD_READ_PRESENCE_RIGHT;
    ret = CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
    if ( ret == FALSE )
        return -1; 

    do
    {
        /*Read response command word*/
        ret = CS498xx_SPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8));
        if ( ret == FALSE )
            return -1;

        if ( cnt >= 5 )
            return -1;

    }while( value != PRESENCE_RESPONSE_RIGHT );


    ret = CS498xx_SPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8));
    if ( ret == FALSE )
        return -1;

    RightChannelSignal = value;
    //____________@}

    
    if( ( LeftChannelSignal | RightChannelSignal ) != 0 )
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}



//______________________________________________________________________________________________
//Smith implemented !!
bool CS498xx_OS (void)
{
#if 0  
    bool ret = TRUE;  
    uint32 value = 0;

    VirtualTimer_sleep(10);

    /*Read message; NOTE 1. Read four bytes from the DSP. IRQ will not drop for this read sequence.*/
     CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));

    if (value != STATE_BOOT_READY) {
        ret = FALSE;
      //  TRACE_DEBUG((0, "BOOT READY STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_DEBUG((0, "BOOT READY STATUS = 0x%X", value));
        TRACE_ERROR((STR_ID_NULL, "CS49810 check boot ready fail"));
        return ret;
    }

    mCMD = CMD_SLAVE_BOOT;
                
    /*write slave boot*/
    CS498xx_SPI_write_buffer((byte*)&mCMD, (sizeof(mCMD)/sizeof(uint8)));
        
    /*wait 100ms*/
    VirtualTimer_sleep(10);

    /*Read message; NOTE 1. Read four bytes from the DSP. IRQ will not drop for this read sequence.*/
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if ( value != STATE_BOOT_START)
    {
        //TRACE_DEBUG((0, "BOOT START STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
         TRACE_DEBUG((0, "BOOT START STATUS = 0x%X", value));
        TRACE_ERROR((STR_ID_NULL, "CS498xx OS Slave boot start fail"));
       return FALSE; 
    }

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    CS498xx_SPI_write_buffer((byte*)&OS_HD2_RC94_ULD, (sizeof(OS_HD2_RC94_ULD)/sizeof(uint8)) );
    
    VirtualTimer_sleep(20);
       /*Read message */
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if (value != STATE_BOOT_SUCCESS )
    {
        TRACE_DEBUG((0, "BOOT FAIL STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx write OS ULD fail"));
        return FALSE; 
    }

    TRACE_INFO((0, "OS_HD2_AB_495304_20_01_RC94_ULD Finished !! "));
#endif
    return TRUE;
}

bool buffer_write_dsp_CS498xx_CrossBar( void )
{
#if 0  
    uint32 value = 0;

    mCMD = CMD_SLAVE_BOOT;
        
    /*write slave boot*/
    CS498xx_SPI_write_buffer((byte*)&mCMD, (sizeof(mCMD)/sizeof(uint8)));
        
    /*wait 100ms*/
    VirtualTimer_sleep(10);

    /*Read message; NOTE 1. Read four bytes from the DSP. IRQ will not drop for this read sequence.*/
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if ( value != STATE_BOOT_START)
    {
        TRACE_DEBUG((0, "BOOT START STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx OS Slave boot start fail"));
        return FALSE; 
    }

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    CS498xx_SPI_write_buffer((byte*)&CROSSBAR_RC28_ULD, (sizeof(CROSSBAR_RC28_ULD)/sizeof(uint8)) );
    
    VirtualTimer_sleep(20);
       /*Read message */
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if (value != STATE_BOOT_SUCCESS )
    {
        TRACE_DEBUG((0, "BOOT FAIL STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx write CROSSBAR_A_495304_20_01_RC28_ULD ULD fail"));
        return FALSE; 
    }

    TRACE_INFO((0, "buffer_write_dsp_CS498xx_CrossBar Finished !! "));
#endif 
    return TRUE;
}

void CS498xx_ProcessMode_Ctrl( uint8 val )
{
  if ( (val&0x01) )//TSHD_ON
    {
        if( (val&0x02) )//
        {
            TRACE_DEBUG((0, " VIRTUAL_ON_AVC_ON "));
            buffer_write_dsp_SRS_VIRTUAL_ON_AVC_ON();
        }
        else if( (val&0x02) == 0 )
        {
            TRACE_DEBUG((0, " VIRTUAL_ON_AVC_OFF "));
            buffer_write_dsp_SRS_VIRTUAL_ON_AVC_OFF();
        } 
    }
    else if ( (val&0x01) == 0 )//TSHD_OFF
    {
        if( (val&0x02) )
        {
            TRACE_DEBUG((0, " VIRTUAL_OFF_AVC_ON "));
            buffer_write_dsp_SRS_VIRTUAL_OFF_AVC_ON();
        }
        else if ( (val&0x02) == 0)
        {
            TRACE_DEBUG((0, " VIRTUAL_OFF_AVC_OFF "));
            buffer_write_dsp_SRS_VIRTUAL_OFF_AVC_OFF();        
        } 
    } 
}

bool CS498xx_PreKickStart(void)
{
    bool ret = TRUE;    
    
    if ( softreset( ) != TRUE ) /*don't remove*/
    {
        TRACE_ERROR((0, " CS498xx SOFT RESET FAIL !! " ));
        return FALSE;
    }

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    ret = CS498xx_SPI_write_buffer((byte*)&PREKICKSTART_CFG, (sizeof(PREKICKSTART_CFG)/sizeof(uint8)) );

    if ( ret == TRUE )
        TRACE_INFO((0, "PreKickStart Finished !!"));
    
    return ret;
}

bool CS498xx_KickStart( void )
{
    bool ret = TRUE;

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    ret = CS498xx_SPI_write_buffer((byte*)&KICKSTART_CFG, (sizeof(KICKSTART_CFG)/sizeof(uint8)) );
    if ( ret == TRUE )
    {
        TRACE_INFO((0, "****** KICKSTART_CFG Finished 100 !! *******"));

        VirtualTimer_sleep(100); /*smith: fix DSP crash TV selects digital output between PCM and AC3*/
    }

    return ret;
}

bool CS498xx_softboot( void )
{
    uint32 msStart, tickWaitIrqLow, msElapsed; // for time measurement and check timeout
    uint8 readCounter = 0; // for statistic
    uint32 readBuffer;
    
    CS498xx_SPI_CommandWrite(CMD_SOFT_BOOT, 0x00000001);

    msStart = VirtualTimer_nowMs();
    do {
        msElapsed = VirtualTimer_nowMs() - msStart;
        if (msElapsed > 1000) // Read softboot ack timeout
        {
            TRACE_ERROR((0, "%s failed, read softboot ack timeout: %d ms", __FUNCTION__, msElapsed));
            return FALSE;
        }
        
        // Wait IRQ low before reading       
        tickWaitIrqLow = VirtualTimer_nowMs();
        while (GPIO_Read_InBit(__I_SCP1_IRQ) != 0)
        {
            TRACE_DEBUG((0, "%s Wait IRQ Low...", __FUNCTION__));
            msElapsed = VirtualTimer_nowMs() - tickWaitIrqLow;
            if (msElapsed > 500) // Wait IRQ LOW timeout
            {
                TRACE_ERROR((0, "%s failed, wait IRQ low timeout: %d ms", __FUNCTION__, msElapsed));
                return FALSE;
            }
        }

        ASSERT_TRUE(GPIO_Read_InBit(__I_SCP1_IRQ) == 0);
        CS498xx_SPI_read_buffer((byte*)&readBuffer, sizeof(readBuffer));
        readCounter ++;
        TRACE_DEBUG((0, "%s readBuffer = 0x%X", __FUNCTION__, readBuffer));

    } while (STATE_SOFTBOOT_ACK != readBuffer);

    // Successed
    msElapsed = VirtualTimer_nowMs() - msStart;
    TRACE_INFO((0, "%s sucessed, wait IRQ low = %d ms, readCounter = %d", __FUNCTION__, msElapsed, readCounter));
    return TRUE;
}


bool CS498xx_Initial_uld ( void )
{
    //! loading ULD @{
    if( CS498xx_OS() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_OS ERROR !! "));
        return FALSE;
    }

    //!  DECODER    @{
    if (CS498xx_PCM_Input( ) != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_PCM_Input ERROR !! "));
        return FALSE;
    }
    //!@}

    if ( CS498xx_PreKickStart() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_PreKickStart ERROR !! "));
        return FALSE;
    }

    //! *INITIAL @{
    if ( CS498xx_PCM_Initial() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_PCM_Initial ERROR !! "));
        return FALSE;
    }
    //! @}

    if ( CS498xx_KickStart() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_KickStart ERROR !! "));
        return FALSE;
    }

    return TRUE;
}

void CS498xx_SetHDMI(void)
{
    uint32 address=0xef000006;
    uint32 command=0x00000002;

    TRACE_DEBUG((0,"Set HDMI path"));
    CS498xx_SPI_write_buffer((byte*)&address, (sizeof(address)/sizeof(uint8)));
    CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));
}

void CS498xx_NON_IEC61937_DECODE(uint32 value)
{
    switch(NON_IEC61937_STREAM_DESCRIPTOR(value))
    {
        case NON_IEC61937_SILENT_INPUT_DATA:  
            TRACE_DEBUG((0,"SILENT_INPUT_DATA"));
            break;
            
        case NON_IEC61937_DTS_FORMAT_16:
            TRACE_DEBUG((0,"DTS_FORMAT_16"));
            break;
            
        case NON_IEC61937_DTS_FORMAT_14:           
            TRACE_DEBUG((0,"DTS_FORMAT_14"));
            break;
            
        case NON_IEC61937_LPCM_21_1FS:             
            TRACE_DEBUG((0,"LPCM 2.1 1Fs"));
            break;
            
        case NON_IEC61937_LPCM_51_1FS:             
            TRACE_DEBUG((0,"LPCM 5.1 1Fs"));
            break;
            
        case NON_IEC61937_LPCM_71_1FS:             
            TRACE_DEBUG((0,"LPCM 7.1 1Fs"));
            break;
            
        case NON_IEC61937_LPCM_21_2FS:             
            TRACE_DEBUG((0,"LPCM 2.1 2Fs"));
            break;
            
        case NON_IEC61937_LPCM_51_2FS:             
            TRACE_DEBUG((0,"LPCM 5.1 2Fs"));
            break;
            
        case NON_IEC61937_LPCM_71_2FS:             
            TRACE_DEBUG((0,"LPCM 7.1 2Fs"));
            break;
            
        case NON_IEC61937_LPCM_21_4FS:             
            TRACE_DEBUG((0,"LPCM 2.1 4Fs"));
            break;
            
        case NON_IEC61937_LPCM_51_4FS:             
            TRACE_DEBUG((0,"LPCM 5.1 4Fs"));
            break;
            
        case NON_IEC61937_LPCM_71_4FS:             
            TRACE_DEBUG((0,"LPCM 7.1 4Fs"));
            break;
            
        case NON_IEC61937_HDCD_PCM_SYNC_DETECT:    
            TRACE_DEBUG((0,"HDCD PCM SYNC DETECT"));
            break;
            
        case NON_IEC61937_HDCD_PCM_SYNC_LOST:      
            TRACE_DEBUG((0,"HDCD PCM SYNC LOST"));
            break;
                       
        default:
            TRACE_DEBUG((0,"Can't recognize"));
            break;
    }
}


void CS498xx_IEC61937_DECODE(uint32 value)
{
    uint32 descriptor;
    switch(IEC61937_STREAM_DESCRIPTOR(value))
    {
        case IEC61937_NEVER_REPORTED_00:  
            TRACE_DEBUG((0,"IEC61937 NEVER REPORTED 00"));
            break;
            
        case IEC61937_AC3:
            TRACE_DEBUG((0,"AC3"));
            break;
            
        case IEC61937_NEVER_REPORTED_03:           
            TRACE_DEBUG((0,"NEVER REPORTED 03"));
            break;
            
        case IEC61937_MPGE_1_L1:             
            TRACE_DEBUG((0,"MPGE 1 L1"));
            break;
            
        case IEC61937_MPGE_1_L23:             
            TRACE_DEBUG((0,"MPGE 1 L23"));
            break;
            
        case IEC61937_MPGE_2_EXTENSTION:             
            TRACE_DEBUG((0,"MPGE 2 EXTENSTION"));
            break;
            
        case IEC61937_MPGE_2_AAC:             
            TRACE_DEBUG((0,"MPGE 2 AAC"));
            break;
            
        case IEC61937_MPGE_2_L1:             
            TRACE_DEBUG((0,"MPGE 2 L1"));
            break;
            
        case IEC61937_MPGE_2_L23:             
            TRACE_DEBUG((0,"MPGE 2 L23"));
            break;
            
        case IEC61937_DTS_1:             
            TRACE_DEBUG((0,"DTS 1"));
            break;
            
        case IEC61937_DTS_2:             
            TRACE_DEBUG((0,"DTS 2"));
            break;
            
        case IEC61937_DTS_3:             
            TRACE_DEBUG((0,"DTS 3"));
            break;
            
        case IEC61937_DTS_4:  
            descriptor = IEC61937_DTS_DESCRIPTOR (value);
            if( descriptor == IEC61937_DTS_HD_HIGH_RESOLUTION)
                TRACE_DEBUG((0,"DTS HD HIGH RESOLUTION "));            
            else if (descriptor == IEC61937_DTS_EXPRESS_LOW_BIT)
                TRACE_DEBUG((0,"DTS EXPRESS LOW BIT ")); 
            else if (descriptor == IEC61937_DTS_MASTER_AUDIO_W_CORE)
                TRACE_DEBUG((0,"DTS MASTER AUDIO W CORE ")); 
            else if (descriptor == IEC61937_DTS_MASTER_AUDIO_WO_CORE)
                TRACE_DEBUG((0,"DTS MASTER AUDIO WO CORE ")); 
            break;
            
        case IEC61937_DDP:      
            TRACE_DEBUG((0,"Dolby Digital Plus"));
            break;

        case IEC61937_TRUEHD: 
            descriptor = IEC61937_TRUEHD_DESCRIPTOR(value);
            if( descriptor == IEC61937_TRUEHD_1FS)
                TRACE_DEBUG((0,"Dolby TRUEHD_1Fs "));            
            else if (descriptor == IEC61937_TRUEHD_2FS)
                TRACE_DEBUG((0,"Dolby TRUEHD_2Fs ")); 
            else if (descriptor == IEC61937_TRUEHD_4FS)
                TRACE_DEBUG((0,"Dolby TRUEHD_4Fs ")); 
            break;
            
        case IEC61937_MPGE_2_AAC_ADTS:      
            TRACE_DEBUG((0,"MPGE 2 AAC ADTS"));
            break;
                       
        default:
            TRACE_DEBUG((0,"Can't recognize"));
            break;
    }
}

void CS498xx_FormatDecode(uint32 value)
{
    if(NON_IEC61937_STREAM_FLAG(value) == 0)
    {
        CS498xx_IEC61937_DECODE(value);
    }
    else
    {
        CS498xx_NON_IEC61937_DECODE(value);
    }
}
