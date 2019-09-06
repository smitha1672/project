#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"
#include "AudioDeviceManager.h"
#include "VirtualTimer.h"

#include "audio_dsp_uld.h"
#include "audio_gain_parameters.h"

#include "GPIOMiddleLevel.h"
#include "cs4953x_spi.h"
#include "cs4953x.h"
#include "UIDeviceManager.h"
#include "ext_flash_driver.h"

//___________________________________________________________________________________
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;

#if ( configLipSync_12steps == 1)
#define AV_delay 0x280000 /*20ms*/
#define AV_delay_10ms 0x140000
#else
#define AV_delay 0x140000 /*10ms*/
#endif

//___________________________________________________________________________________
typedef enum
{
    LOADER_IDLE = 0,
    LOADER_HANDLE_Q,
    LOADER_RESET_DSP,
    LOADER_BOOT_ASSIST,
    LOADER_SOFT_RESET_DSP_A,
    LOADER_SLAVE_BOOT,  
    LOADER_DECODER_ULD,
    LOADER_CROSSBAR_ULD,
    LOADER_GAIN_ULD,
    LOADER_SOFTBOOT,
    LOADER_SOFT_RESET,
    LOADER_CONIGURATION,
    LOADER_KICK_START,
    LOADER_CONFIG_USER_SETTING,
    LOADER_ERROR,
    LOADER_END
}CS4953x_LoaderState;

typedef struct CS495x_LOADER_PARAMETERS
{
    xTaskHandle loaderTaskHandle;
    xTaskHandle fmtTaskHandle;
    xTaskHandle lowlevelTaskHandle;
    xOS_TaskErrIndicator xOS_ErrId;
    TaskHandleState fch_detecter_state;
    TaskHandleState lowlevel_state;
    xSemaphoreParameters spi_sema;
    xQueueParameters loaderQueue;
} xOS_CS495xParams;

typedef struct _CS4953x_AUTODETECT_RESPONSE
{
    uint8 non_iec61937;
    uint8 stream_descriptor;
}CS4953x_ADECT_MSG;

typedef enum{
    OS_ULD = 0,
    DECODER_ULD,    
    CROSSBAR_ULD,
    GAIN_ULD,
}cs495x_ULD_Type;

//___________________________________________________________________________
/*DAO channel remap parameters; AN288 pg.24 */
#define DAO_CHAN_0_REMAP  0x83000012 
#define DAO_CHAN_1_REMAP  0x83000013
#define DAO_CHAN_2_REMAP  0x83000014
#define DAO_CHAN_3_REMAP  0x83000015
#define DAO_CHAN_4_REMAP  0x83000016
#define DAO_CHAN_5_REMAP  0x83000017
#define DAO_CHAN_6_REMAP  0x83000018
#define DAO_CHAN_7_REMAP  0x83000019

#define DAO_LEFT    0x00000000
#define DAO_RIGHT   0x00000002
#define DAO_LEFT_SURROUND   0x00000003
#define DAO_RIGHT_SURROUND  0x00000004
#define DAO_CENTER  0x00000001
#define DAO_LFE0 0x00000007
#define DAO_RESERVED 0x0000000b

#define IO_BUFF_1_SOURCE 0x9b000002 
#define IO_BUFF_2_SOURCE 0x9b000003
#define IO_BUFF_3_SOURCE 0x9b000004  
#define IO_BUFF_4_SOURCE 0x9b000005 
#define IO_BUFF_5_SOURCE 0x9b000006
#define IO_BUFF_6_SOURCE 0x9b000007
#define IO_BUFF_7_SOURCE 0x9b000008 

/*BOOT READ MESSAGE */
#define BOOT_START              0x00000001
#define BOOT_SUCCESS            0x00000002
#define APP_START               0x00000004
#define BOOT_ERROR_CHECKSUM     0x000000FF
#define INVAILD_BOOOT_TYPE      0x000000FE
#define BOOT_FAILURE            0x000000F8

/*SOFT RESET MESSAGE*/
#define SOFT_RESET              0x40000000
#define SOFT_RESET_DSP_A        0x50000000

/*SLAVE BOOT MESSAGE*/
#define SLAVE_BOOT              0x80000000
#define CMD_SOFT_BOOT		0x81000009
#define SOFTBOOT_ACK            0x00000005

#define CS4953x_CMD_BASE 0xf0000000

//! gain_multi_channel_friendly_plus24 (Ls-Rs Gain2) @{
#define RS_G2 	( CS4953x_CMD_BASE + _g_0_Ls_Rs_Gain2151 )
#define LS_G2	( CS4953x_CMD_BASE + _g_1_Ls_Rs_Gain2151 )

// gain_multi_channel_plus24 (Center Gain 1)
#define CENTER_G1  ( CS4953x_CMD_BASE +  _g_0_Center_Gain_1118 )

// gain_multi_channel_plus24 (Center Gain 2) 
#define CENTER_G2  ( CS4953x_CMD_BASE + _g_0_Center_Gain_2152 )

// gain_multi_channel_friendly_plus24 (Subwoofer Gain 1)
#define SUBWOOFER_G1 ( CS4953x_CMD_BASE + _g_0_Subwoofer_Gain_1120 )  

// gain_multi_channel_friendly_plus24 (Subwoofer Gain 2)
#define SUBWOOFER_G2 ( CS4953x_CMD_BASE + _g_0_Subwoofer_Gain_2132 )

// gain_multi_channel_plus24 (Master Volume)
// g_1 : signed : 5.27 format
#define MASTER_GAIN1  ( CS4953x_CMD_BASE + _g_0_Master_Volume131 ) 
// g_2 : signed : 5.27 format
#define MASTER_GAIN2  ( CS4953x_CMD_BASE + _g_1_Master_Volume131 )
// g_3 : signed : 5.27 format
#define MASTER_GAIN3  ( CS4953x_CMD_BASE + _g_2_Master_Volume131 )
// g_4 : signed : 5.27 format
#define MASTER_GAIN4  ( CS4953x_CMD_BASE + _g_3_Master_Volume131 )
// g_5 : signed : 5.27 format
#define MASTER_GAIN5  ( CS4953x_CMD_BASE + _g_4_Master_Volume131 )
// g_6 : signed : 5.27 format
#define MASTER_GAIN6  ( CS4953x_CMD_BASE + _g_5_Master_Volume131 )

//gain_multi_channel_friendly_plus24 (Ls-Rs Gain 1)
// g_1 : signed : 5.27 format
#define BALANCE_LS  ( CS4953x_CMD_BASE + _g_0_Ls_Rs_Gain_1119 )
// g_2 : signed : 5.27 format
#define BALANCE_RS  ( CS4953x_CMD_BASE + _g_1_Ls_Rs_Gain_1119 )

// tone_control_proc (Tone Control)
#define _treble_level_left 0xd4000001 
#define _treble_level_center   0xd4000002 
#define _treble_level_right 0xd4000003 
#define _treble_level_ls   0xd4000004
#define _treble_level_rs 0xd4000005 

#define _bass_level_left   0xd4000008
#define _bass_level_center   0xd4000009
#define _bass_level_right   0xd400000a
#define _bass_level_ls   0xd400000b
#define _bass_level_rs   0xd400000c
// _bass_level_left : signed : 32.0 format
#define BASS_LEFT  0xd4000008 
// _bass_level_right : signed : 32.0 format
#define BASS_RIGHT  0xd400000a 

#define DELAY_VALUE_LEFT   0xd9000002
#define DELAY_VALUE_CENTER 0xd9000003
#define DELAY_VALUE_RIGHT  0xd9000004
#define DELAY_VALUE_LS     0xd9000005
#define DELAY_VALUE_RS     0xd9000006
#define DELAY_VALUE_LFE    0xd9000009

#define DELAY_TIME_LEFT   0x00333333
#define DELAY_TIME_CENTER 0x00339999
#define DELAY_TIME_RIGHT  0x00333333
#define DELAY_TIME_LS     0x00146666
#define DELAY_TIME_RS     0x00146666
#define DELAY_TIME_LFE    0x00000000

#define CS49XXXX_READ_BASE 0xf0C00000
#define CS49XXXX_CMD_READ_PRESENCE_LEFT	( CS49XXXX_READ_BASE + _presence_Presence122 )
#define CS49XXXX_CMD_READ_PRESENCE_RIGHT ( CS49XXXX_READ_BASE + _presence_Presence73 )	

#define CS49XXXX_RESPONSE_BASE 0x70C00000
#define PRESENCE_RESPONSE_LEFT ( CS49XXXX_RESPONSE_BASE + _presence_Presence122 )
#define PRESENCE_RESPONSE_RIGHT	( CS49XXXX_RESPONSE_BASE + _presence_Presence73 )

#define READ_RESPONSE_MESSAGE 0x6fc00000
#define DSP_CFG_STREAM_TYPE READ_RESPONSE_MESSAGE+0x00000002

#define PCM_ENABLE            0x9b000000
#define TV_ENABLE             0xe0000000
#define TV_BYPASS             0xe0000001
#define AUTODETECT_CONFIG     0x81000003
#define AUTOSWITCH_ENABLE     0x81000005
//___________________________________________________________________________
#define LOADER_QUEUE_LENGTH 2
#define AUTO_DETECTION_RESPONSE (0x81000000)
#define FCH_DETECTOR_TIME_TICK  TASK_MSEC2TICKS(10)
#define LOADER_TIME_TICK TASK_MSEC2TICKS(10)
#define LOWLEVEL_SUSPENDED_TIME TASK_MSEC2TICKS(1)
#define LOWLEVEL_TIME_TICK TASK_MSEC2TICKS(500)
#define SOFTRESET_TIMEOUT TASK_MSEC2TICKS(1000)
//___________________________________________________________________________
CS4953x_ADECT_MSG mAdtectMsg;

//___________________________________________________________________________
static CS4953x_LoaderState mLoaderState = LOADER_IDLE;
static xOS_CS495xParams xOS_CS4953x;
static CS4953x_QUEUE_TYPE mQueueType;
static CS4953xLoadAudioStreamType gCurDSP_ULD_type = CS4953x_LOAD_PCM;
static const CS4953xLoadAudioStreamType *pCurtDSP_ULD_TYPE = &gCurDSP_ULD_type;
static uint8 signal_overthreshold = 1;
static CS4953xMode  mMode;
static const CS4953xMode  *pMode=&mMode;
static xSemaphoreHandle _IRQ_SEMA = NULL;
static uint8 DSPLoadErrorCount = 0;	//Add by Elvis for reinitial count

/* PCM load procedure: OS -> tv_cs-> pcm_black_b */
#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
unsigned int PCM_ULD_LOCATION[4][2] = 
{
    {PCM_INIT_ULD_START_LOCATION,PCM_INIT_ULD_LENGTH}, /* OS */
    {0,0}, /* PCM don't need decoder*/
    {PCM_CROSSBAR_ULD_START_LOCATION,PCM_CROSSBAR_ULD_LENGTH},    
    {PCM_GAIN_ULD_START_LOCATION,PCM_GAIN_ULD_LENGTH},
};

/* AC3 load procedure: OS-> AC3 -> Crossbar-> ac3_black_b */
unsigned int AC3_ULD_LOCATION[4][2] = 
{
    {PCM_INIT_ULD_START_LOCATION,PCM_INIT_ULD_LENGTH}, /* OS */
    {AC3_INIT_ULD_START_LOCATION,AC3_INIT_ULD_LENGTH}, /* Decoder */     
    {AC3_CROSSBAR_ULD_START_LOCATION,AC3_CROSSBAR_ULD_LENGTH},    
    {AC3_GAIN_ULD_START_LOCATION,AC3_GAIN_ULD_LENGTH},
};
/* DTS load procedure: OS-> DTS -> Crossbar-> dts_black_b */
unsigned int DTS_ULD_LOCATION[4][2] = 
{
    {PCM_INIT_ULD_START_LOCATION,PCM_INIT_ULD_LENGTH}, /* OS */   
    {DTS_INIT_ULD_START_LOCATION,DTS_INIT_ULD_LENGTH}, /* Decoder */      
    {DTS_CROSSBAR_ULD_START_LOCATION,DTS_CROSSBAR_ULD_LENGTH},
    {DTS_GAIN_ULD_START_LOCATION,DTS_GAIN_ULD_LENGTH},
};

static unsigned char* ULDLoaderbuf = NULL;
#endif 

//___________________________________________________________________________
static void cs4953x_UpdateFormatIndicator( CS4953xLoadAudioStreamType stream_type );

static void cs495x_RowDataLoader_CreateTask( void );

static bool cs495x_RowDataLoader_InstrReceiver( CS4953x_QUEUE_TYPE *pType ) ;

//static uint8 cs495x_RowDataLoader_GetQueueNumber( void );

static void cs4953x_HardReset( void );

static void cs4953x_FchDetector_CreateTask( void );

static void cs495x_FchDetector_TaskCtrl( xOS_TaskCtrl val );

static void cs495x_FchDetector_setState( TaskHandleState set_state );

static void cs495x_RowDataLoader_TaskCtrl( xOS_TaskCtrl val );

static bool cs495x_RowDataLoader_InstrSender( void *parms ) ;

static void cs495x_LowLevelDetector_CreateTask( void );

static void cs495x_LowLevelDetector_TaskCtrl( xOS_TaskCtrl val );

static void cs4953x_setMute( bool val );

static bool cs4953x_spi_mutex_take( void );

static bool cs4953x_spi_mutex_give( void );

static bool cs4953x_irq_mutex_take( void );

static bool cs4953x_irq_mutex_give( void );

static void cs4953x_setMasterGain( uint32 value );

static void cs4953x_setBassGain( uint32 value );

static void cs4953x_setTrebleGain( uint32 value );

static void cs4953x_setSubGain( uint32 value );

static void cs4953x_setCenterGain( uint32 value );

static void cs4953x_setLsRsGain( uint32 value );

static void cs4953x_setBalanceLs( uint32 value );

static void cs4953x_setBalanceRs( uint32 value );

static void cs4953x_SetTVHDMC_Ctrl( byte* data, uint16 length );

static void cs4953x_setNightMode( uint32 value );

static void cs4953x_setAVDelay( uint32 value );

static void cs4953x_setAudioRoute(CS4953xAudioRoute val);

static void cs4953x_setSilenceThreshold( bool value );

static CS4953xLoadAudioStreamType cs4953x_GetAudioStreamType( void );

static uint8 cs4953x_GetLoadrState( void );

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
static int cs495x_LoadDSP_ULD(unsigned char* buf,CS4953xLoadAudioStreamType Stype ,cs495x_ULD_Type Utype );
#endif 

uint8 cs4953x_getSignalLevel( void );

static void cs4953x_lowlevel_task_setState( TaskHandleState set_state );

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs4953x_Load_DSP_ULD_InternalFlash( CS4953xLoadAudioStreamType type, cs495x_ULD_Type module );
#endif 

extern AUDIO_DEVICE_QUEUE_OBJECT *pADM_QueueObj;

const DSP_CTRL_OBJECT DSP_ObjCtrl = 
{
    cs495x_RowDataLoader_CreateTask,
    cs495x_RowDataLoader_TaskCtrl,
    cs4953x_FchDetector_CreateTask,
    cs495x_FchDetector_TaskCtrl,
    cs495x_FchDetector_setState,    
    cs495x_RowDataLoader_InstrSender,
    cs495x_LowLevelDetector_CreateTask,
    cs495x_LowLevelDetector_TaskCtrl,
    cs4953x_lowlevel_task_setState,
    cs4953x_setMute,
    cs4953x_setMasterGain,
    cs4953x_setBassGain,
    cs4953x_setTrebleGain,
    cs4953x_setSubGain,
    cs4953x_setCenterGain,
    cs4953x_setLsRsGain,
    cs4953x_setBalanceLs,
    cs4953x_setBalanceRs,
    cs4953x_SetTVHDMC_Ctrl,
    cs4953x_setNightMode,
    cs4953x_setAVDelay,
    cs4953x_setAudioRoute,
    cs4953x_setSilenceThreshold,
    cs4953x_GetAudioStreamType,
    cs4953x_GetLoadrState,
    cs4953x_getSignalLevel,
};

const DSP_CTRL_OBJECT *pDSP_ObjCtrl = &DSP_ObjCtrl;

//___________________________________________________________________________
static void cs4953x_setMute( bool val )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        if ( val == TRUE ) /*mute*/
        {
            CS49xxxxSPI_CommandWrite( 0x83000001, 0x00000001 );
            //TRACE_DEBUG((0, "cs4953x_setMute CS49xxxxSPI_CommandWrite mute"));
        }
        else
        {
            CS49xxxxSPI_CommandWrite( 0x83000001, 0x00000000 );
            //TRACE_DEBUG((0, "cs4953x_setMute CS49xxxxSPI_CommandWrite unmute"));
        }

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setMasterGain( uint32 value )
{
    if ( value > 0x08000000 )
        return;        
    
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(MASTER_GAIN1, value );
        CS49xxxxSPI_CommandWrite(MASTER_GAIN2, value );
        CS49xxxxSPI_CommandWrite(MASTER_GAIN3, value );
        CS49xxxxSPI_CommandWrite(MASTER_GAIN4, value );
        CS49xxxxSPI_CommandWrite(MASTER_GAIN5, value );
        CS49xxxxSPI_CommandWrite(MASTER_GAIN6, value );

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setBassGain( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(_bass_level_left, value);
        CS49xxxxSPI_CommandWrite(_bass_level_center, value);
        CS49xxxxSPI_CommandWrite(_bass_level_right, value);
        CS49xxxxSPI_CommandWrite(_bass_level_ls, value);
        CS49xxxxSPI_CommandWrite(_bass_level_rs, value); 

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setTrebleGain( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(_treble_level_left, value);
        CS49xxxxSPI_CommandWrite(_treble_level_center, value);
        CS49xxxxSPI_CommandWrite(_treble_level_right, value);
        CS49xxxxSPI_CommandWrite(_treble_level_ls, value);
        CS49xxxxSPI_CommandWrite(_treble_level_rs, value); 

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setSubGain( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(SUBWOOFER_G2, value );

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setCenterGain( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(CENTER_G2, value);  

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setLsRsGain( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(RS_G2, value);  
        CS49xxxxSPI_CommandWrite(LS_G2, value);  

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setBalanceLs( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(BALANCE_LS, value);     

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setBalanceRs( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(BALANCE_RS, value);     

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_SetTVHDMC_Ctrl( byte* data, uint16 length )
{
	if (cs4953x_spi_mutex_take() == TRUE )
	{
		CS4953xSPI_write_buffer(data, length);
	
		cs4953x_spi_mutex_give();
	}
}

static void cs4953x_setNightMode( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        CS49xxxxSPI_CommandWrite(SUBWOOFER_G1, value );
    
        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setAVDelay( uint32 value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
#if ( configLipSync_12steps == 1)       
        if (value >= AV_DELAY_MAX)
        {
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_LEFT, DELAY_TIME_LEFT + AV_delay * value + AV_delay_10ms);
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_CENTER, DELAY_TIME_CENTER +  AV_delay * value + AV_delay_10ms);
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_RIGHT, DELAY_TIME_RIGHT +  AV_delay * value + AV_delay_10ms); 
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_LS, DELAY_TIME_LS +  AV_delay * value + AV_delay_10ms);
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_RS, DELAY_TIME_RS +  AV_delay * value + AV_delay_10ms);
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_LFE, DELAY_TIME_LFE +  AV_delay * value + AV_delay_10ms);
        }
        else  
#endif  
        {
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_LEFT, DELAY_TIME_LEFT + AV_delay * value );
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_CENTER, DELAY_TIME_CENTER +  AV_delay * value );
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_RIGHT, DELAY_TIME_RIGHT +  AV_delay * value ); 
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_LS, DELAY_TIME_LS +  AV_delay * value );
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_RS, DELAY_TIME_RS +  AV_delay * value );
            CS49xxxxSPI_CommandWrite(DELAY_VALUE_LFE, DELAY_TIME_LFE +  AV_delay * value );
        }
        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setAudioRoute(CS4953xAudioRoute val)
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        if ( val == ROUTE_INITIAL )
        {
            CS4953xSPI_write_buffer((byte*)&DSP_DAO_ROUTER_INITIAL, (sizeof(DSP_DAO_ROUTER_INITIAL)/sizeof(uint8)));
        }

        /*Clean internal channel map*/
        CS49xxxxSPI_CommandWrite(DAO_CHAN_0_REMAP, DAO_RESERVED );
        CS49xxxxSPI_CommandWrite(DAO_CHAN_1_REMAP, DAO_RESERVED );
        CS49xxxxSPI_CommandWrite(DAO_CHAN_2_REMAP, DAO_RESERVED );
        CS49xxxxSPI_CommandWrite(DAO_CHAN_3_REMAP, DAO_RESERVED );
        CS49xxxxSPI_CommandWrite(DAO_CHAN_4_REMAP, DAO_RESERVED );
        CS49xxxxSPI_CommandWrite(DAO_CHAN_5_REMAP, DAO_RESERVED );
        CS49xxxxSPI_CommandWrite(DAO_CHAN_6_REMAP, DAO_RESERVED );
        CS49xxxxSPI_CommandWrite(DAO_CHAN_7_REMAP, DAO_RESERVED );
   
        
        switch( val )
        {
            case ROUTE_FL_FR:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_0_REMAP, DAO_LEFT ); /*FL*/
                CS49xxxxSPI_CommandWrite( DAO_CHAN_1_REMAP, DAO_RIGHT ); /*FR*/
            }
                break;
        
            case ROUTE_RR_RL:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_6_REMAP, DAO_LEFT ); /*RL*/
                CS49xxxxSPI_CommandWrite( DAO_CHAN_7_REMAP, DAO_RIGHT ); /*RR*/
            }
                break;
            
            case ROUTE_FL:
            {
                /*channel 0 outputs left sound*/
                CS49xxxxSPI_CommandWrite( DAO_CHAN_0_REMAP, DAO_LEFT ); /*FL*/           
            }
                break;
        
            case ROUTE_FR:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_1_REMAP, DAO_RIGHT ); /*FR*/
            }
                break;
        
            case ROUTE_RL:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_6_REMAP, DAO_LEFT ); /*RL*/
            }
                break;
        
            case ROUTE_RR:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_7_REMAP, DAO_RIGHT ); /*RR*/
            }
                break;
        
            case ROUTE_CENTER:
            {
                /*Smith: Fix center outputs too huge, when mode was in factory mode */
                CS49xxxxSPI_CommandWrite( DAO_CHAN_3_REMAP, DAO_CENTER );
                CS49xxxxSPI_CommandWrite( DAO_CHAN_2_REMAP, DAO_CENTER ); /*CENTER*/        
            }
                break;
        
            case ROUTE_LFE0:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_4_REMAP, DAO_RIGHT ); /*LFE0*/  
                CS49xxxxSPI_CommandWrite( DAO_CHAN_5_REMAP, DAO_LEFT ); /*LFE0*/                
            }
                break;
        
            case ROUTE_FL_FR_CENTER:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_0_REMAP, DAO_LEFT ); /*FL*/   
                CS49xxxxSPI_CommandWrite( DAO_CHAN_1_REMAP, DAO_RIGHT ); /*FR*/
                
                /*Smith: Fix center outputs too huge, when mode was in factory mode */  
                CS49xxxxSPI_CommandWrite( DAO_CHAN_3_REMAP, DAO_CENTER );
                CS49xxxxSPI_CommandWrite( DAO_CHAN_2_REMAP, DAO_CENTER ); /*CENTER*/
            }
            break;
            
            case ROUTE_RR_RL_WOOFER:
            {
                CS49xxxxSPI_CommandWrite( DAO_CHAN_6_REMAP, DAO_LEFT ); /*RL*/
                CS49xxxxSPI_CommandWrite( DAO_CHAN_7_REMAP, DAO_RIGHT ); /*RR*/
                
                CS49xxxxSPI_CommandWrite( DAO_CHAN_4_REMAP, DAO_RIGHT ); /*LFE0*/  
                CS49xxxxSPI_CommandWrite( DAO_CHAN_5_REMAP, DAO_LEFT ); /*LFE0*/ 
            }
            break;

			case ROUTE_DISABLE:
            default:
            {
                CS4953xSPI_write_buffer((byte*)&DSP_DAO_ROUTER_INITIAL, (sizeof(DSP_DAO_ROUTER_INITIAL)/sizeof(uint8)));
            }
                break;
        }

        cs4953x_spi_mutex_give();
    }
}

static void cs4953x_setSilenceThreshold( bool value )
{
    if (cs4953x_spi_mutex_take() == TRUE )
    {
        if (value)
        {
            CS49xxxxSPI_CommandWrite(0x81000008, 0x0083d600);/* Silence threshold 90s */
        }
        else
        {
            CS49xxxxSPI_CommandWrite(0x81000008, 0x00005dc0);/* Silence_threshold 0.25 second */
        }

        cs4953x_spi_mutex_give();
    }
}

static CS4953xLoadAudioStreamType cs4953x_GetAudioStreamType( void )
{
    return gCurDSP_ULD_type;
}

static uint8 cs4953x_GetLoadrState( void )
{
    return mLoaderState;
}


static int CS4953x_readPresence( uint8 *pValue )
{
    int retCmd = -1;
    int retData = -1;
    int ret = 0;
    
    int errCnt = 0;
    uint32 command = 0; uint32 value = 0;
    uint32 LeftChannelSignal = 0;   uint32 RightChannelSignal = 0;
    
    if ( pValue == NULL )
        return -1;

    if (cs4953x_spi_mutex_take() == TRUE )
    {
        do{
             /*Read Command Word(left)*/
            command = CS49XXXX_CMD_READ_PRESENCE_LEFT;
            if (CS4953xSPI_write_buffer((byte*)&command,(sizeof(command)/sizeof(uint8))) == SCP1_PASS )
            {
                /*Read response command word*/
                retCmd = CS4953xSPI_read_buffer((byte*)&value,(sizeof(command)/sizeof(uint8))); 

                /*Read response Data word*/ 
                retData = CS4953xSPI_read_buffer((byte*)&value,(sizeof(command)/sizeof(uint8)));

            }
            //TRACE_DEBUG((0, "CS49XXXX_CMD_READ_PRESENCE_LEFT :  retCmd = %d, retData = %d",retCmd,retData));
            if( retCmd == SCP1_BSY_TIMEOUT  || retData == SCP1_BSY_TIMEOUT)
            {
                ret = -1;
                errCnt++;
            }
            else
            {
                LeftChannelSignal = value;
                break;
            }
            
        }while(errCnt < 3);
        
        errCnt = 0;      
        do{
             /*Read Command Word(left)*/
            command = CS49XXXX_CMD_READ_PRESENCE_RIGHT;
            if (CS4953xSPI_write_buffer((byte*)&command,(sizeof(command)/sizeof(uint8))) == SCP1_PASS )
            {
                /*Read response command word*/
                retCmd = CS4953xSPI_read_buffer((byte*)&value,(sizeof(command)/sizeof(uint8))); 

                /*Read response Data word*/ 
                retData = CS4953xSPI_read_buffer((byte*)&value,(sizeof(command)/sizeof(uint8)));

            }
            //TRACE_DEBUG((0, "CS49XXXX_CMD_READ_PRESENCE_RIGHT :  retCmd = %d, retData = %d",retCmd,retData));
            if( retCmd == SCP1_BSY_TIMEOUT  || retData == SCP1_BSY_TIMEOUT)
            {
                ret = -1;
                errCnt++;
            }
            else
            {
                RightChannelSignal = value;
                break;
            }
            
        }while(errCnt < 3);

        cs4953x_spi_mutex_give();

  
        //TRACE_DEBUG((0, "LeftChannelSignal = %d , RightChannelSignal = %d",LeftChannelSignal,RightChannelSignal));
        if( ( LeftChannelSignal || RightChannelSignal ) )
        {
            *pValue = 1;
        }
        else
        {
            *pValue = 0;
        }
        
    }
    return ret;
}

uint8 cs4953x_getSignalLevel( void )
{
    return signal_overthreshold;
}

static bool cs4953x_spi_mutex_take( void )
{
    if ( SPILowLevel_isEnable() != TRUE )
    {
        //TRACE_DEBUG((0, "SPI TAKE BUSY 1111 !! "));
        return FALSE;
    }   

    if ( xOS_CS4953x.spi_sema.xSemaphore == NULL )
    {
        //TRACE_DEBUG((0, "SPI TAKE BUSY 2222 !! "));
        return FALSE;
    }
    //xOS_CS4953x.spi_sema.xBlockTime = portMAX_DELAY;
    if ( xSemaphoreTake( xOS_CS4953x.spi_sema.xSemaphore, xOS_CS4953x.spi_sema.xBlockTime ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "SPI TAKE BUSY 3333 !! "));
        return FALSE;
    }

    return TRUE;
}

static bool cs4953x_spi_mutex_give( void )
{
    if ( SPILowLevel_isEnable() != TRUE )
        return FALSE;

    if ( xOS_CS4953x.spi_sema.xSemaphore == NULL )
        return FALSE;

    if ( xSemaphoreGive( xOS_CS4953x.spi_sema.xSemaphore ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "SPI GIVE BUSY !! "));
        return FALSE;	
    }

    return TRUE;
}

static bool cs4953x_irq_mutex_take( void )
{
    if ( _IRQ_SEMA == NULL )
    {
        //TRACE_DEBUG((0, "IRQ TAKE BUSY 2222 !! "));
        return FALSE;
    }

    if ( xSemaphoreTake( _IRQ_SEMA, portMAX_DELAY ) != pdTRUE )
    //if ( xSemaphoreTake( xOS_CS4953x.spi_sema.xSemaphore, 0xff ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "IRQ TAKE BUSY 3333 !! "));
        return FALSE;
    }

    return TRUE;    
}

static bool cs4953x_irq_mutex_give( void )
{
#if 1
    if ( _IRQ_SEMA == NULL )
        return FALSE;

    if ( xSemaphoreGive( _IRQ_SEMA ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "IRQ GIVE BUSY 111 !! "));
        return FALSE;   
    }

    //TRACE_DEBUG((0, "IRQ GIVE OK !! "));
#endif    
    return TRUE;
}

static void cs4953x_lowlevel_task_setState( TaskHandleState set_state )
{
    if ( set_state == TASK_SUSPENDED )
    {
        if ( xOS_CS4953x.lowlevel_state == TASK_RUNING )
        {
            xOS_CS4953x.lowlevel_state = TASK_SUSPENDED;
        }
    }
    else if ( set_state == TASK_RUNING )
    {
        if ( xOS_CS4953x.lowlevel_state == TASK_SUSPENDED )
        {
            xOS_CS4953x.lowlevel_state = TASK_RUNING;
        }
    }
}

void cs4953x_lowlevel_task( void *pvParameters )
{
    int ret = 0;
    static uint8 last_overthreshold = 0;

    for( ;; )
    {
        switch( xOS_CS4953x.lowlevel_state )
        {
            case TASK_SUSPENDED:
            {
            }
                break;

            case TASK_RUNING:
            {
				xADM_QUEUE_Events adm_q_event = ADM_QUEUE_EVENT_NULL;
                if( (*pCurtDSP_ULD_TYPE == CS4953x_LOAD_PCM ) && ( mLoaderState == LOADER_IDLE ) )
                {
                    ret = CS4953x_readPresence( &signal_overthreshold );

					if ( last_overthreshold != signal_overthreshold )
					{
						if ( signal_overthreshold )
						{
							adm_q_event = ADM_QUEUE_EVENT_FROM_DSP_ANALOG_LOCK;
							pADM_QueueObj->SignalDetector_sender( &adm_q_event );
						}
						else
						{
							adm_q_event = ADM_QUEUE_EVENT_FROM_DSP_ANALOG_UNLOCK;
							pADM_QueueObj->SignalDetector_sender( &adm_q_event );
						}
						
						last_overthreshold = signal_overthreshold;
					}
					
                    if ( ret != 0 )
                    {
                        TRACE_ERROR((0, "presence error =%X signal_overthreshold=%X",ret,signal_overthreshold));
                    }
                }
            }
                break;
        }

        vTaskDelay(LOWLEVEL_TIME_TICK);
    }
}

static void cs495x_LowLevelDetector_TaskCtrl( xOS_TaskCtrl val )
{
    signal_overthreshold = 1;

    if ( val == xOS_setTASK_SUSPENDED )
    {
        xOS_CS4953x.lowlevel_state = TASK_SUSPENDED;
        vTaskDelay(LOWLEVEL_SUSPENDED_TIME); /* prevent pressce error*/
        if ( xTaskIsTaskSuspended( xOS_CS4953x.lowlevelTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_CS4953x.lowlevelTaskHandle );
        }
    }
    else if ( val == xOS_setTASK_RESUME )
    {
        if ( xTaskIsTaskSuspended( xOS_CS4953x.lowlevelTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_CS4953x.lowlevelTaskHandle );
        }
    }
}

static void cs495x_LowLevelDetector_CreateTask( void )
{
    signal_overthreshold = 0;
    if ( xTaskCreate( 
            cs4953x_lowlevel_task, 
            ( portCHAR * ) "CS4953x_lowlevel", 
            (STACK_SIZE), 
            NULL, 
            tskCS4953x_LowLevelDetection_PRIORITY, 
            &xOS_CS4953x.lowlevelTaskHandle ) != pdPASS )
    {
        vTaskDelete( xOS_CS4953x.lowlevelTaskHandle );
    }
    else
    {
        vTaskSuspend( xOS_CS4953x.lowlevelTaskHandle );
    }
}   

static CS4953x_ADECT_MSG cs4953x_getAutoDectMsg( uint32 data )
{
    CS4953x_ADECT_MSG msg;

    if ( NON_IEC61937_STREAM_FLAG(data) > 0 ) 
    {
        msg.non_iec61937 = 0x01;
    }
    else if ( NON_IEC61937_STREAM_FLAG(data) == 0 )
    {
        msg.non_iec61937 = 0x00;
    }

    msg.stream_descriptor = (uint8)NON_IEC61937_STREAM_DESCRIPTOR(data);

    return msg;
}

static void cs4953x_parseAudioFMT(CS4953x_QUEUE_TYPE *tmp_queue_data, CS4953x_ADECT_MSG msg)
{
    tmp_queue_data->audio_type = CS4953x_LOAD_NULL;
    
    if ( msg.non_iec61937 == 0x01)/*PCM*/
    {
        switch( msg.stream_descriptor)
        {
            case NON_IEC61937_DTS_FORMAT_16:
            case NON_IEC61937_DTS_FORMAT_14:
            {
                tmp_queue_data->audio_type = CS4953x_LOAD_DTS;
            }
                break;

            case NON_IEC61937_SILENT_INPUT_DATA:
            {
                tmp_queue_data->audio_type = CS4953x_LOAD_PCM;
            }
                break;
            
            default:
            {
                tmp_queue_data->audio_type = CS4953x_LOAD_PCM;
                TRACE_DEBUG((0, "tmp_queue_data->audio_type = CS4953x_LOAD_PCM !! "));
            }
                break;
        }
    }
    else /*NON PCM*/
    {
        switch( msg.stream_descriptor)
        {
            case IEC61937_STREAM_AC3:
            {
                tmp_queue_data->audio_type = CS4953x_LOAD_AC3;
            }
                break;

            case IEC61937_STREAM_DTS_1:
            case IEC61937_STREAM_DTS_2:
            case IEC61937_STREAM_DTS_3:
            {
                tmp_queue_data->audio_type = CS4953x_LOAD_DTS;
            }
                break;
            
            default:
            {
                tmp_queue_data->audio_type = CS4953x_LOAD_PCM;
            }
                break;
        }
    }
}

static void cs4953x_SendEventToLoader( CS4953x_ADECT_MSG msg )
{
    CS4953x_QUEUE_TYPE QUEUE_TYPE;

    memset(&QUEUE_TYPE, 0 , sizeof(CS4953x_QUEUE_TYPE));
    cs4953x_parseAudioFMT(&QUEUE_TYPE, msg);
            
    if (QUEUE_TYPE.audio_type != CS4953x_LOAD_NULL)
    {
        //TRACE_DEBUG((0, "cs4953x_SendEventToLoader !! "));

        //if (cs4953x_spi_mutex_take( ) == TRUE)
        {
            //TRACE_ERROR((0, "send message to data loader ----- !! "));
            QUEUE_TYPE.source_ctrl = CS4953x_SOURCE_DSP;
            cs495x_RowDataLoader_InstrSender(&QUEUE_TYPE);
        }
        //else
        //{
            //TRACE_ERROR((0, "Can't send message to data loader ----- !! "));
        //    cs4953x_irq_mutex_give();
        //}    
    }
    else
    {
        cs4953x_irq_mutex_give();
        //TRACE_ERROR((0, "send message CS4953x_LOAD_NULL ----- !! "));
    }
}

void cs4953x_FchDetector( void *pvParameters )
{
    uint32 u32ReadCommand = 0;
    uint32 u32ReadData;    // 4-byte Unsolicited Read Data
    
    for( ;; )
    {
        switch (xOS_CS4953x.fch_detecter_state )
        {
            case TASK_SUSPENDED:
            {
#if 0 /*Smith fixes bus: the task will be start by set input path; pDSP_ObjCtrl->fch_task_set_state( TASK_RUNING );*/            
                xOS_CS4953x.fch_detecter_state = TASK_RUNING;
#endif                
            }
                break;
          
            case TASK_RUNING:
            {
                if( GPIOMiddleLevel_Read_InBit( __I_SCP1_IRQ ) == 0 )
                {
                    //TRACE_DEBUG((0, "cs4953x_FchDetector IRQ enter !! "));
                    if (cs4953x_irq_mutex_take() == TRUE)
                    {
                        if ( cs4953x_spi_mutex_take() == TRUE )
                        {
                            //TRACE_DEBUG((0, "take spi mutex------- !! "));
                            if (CS4953xSPI_nIrq_read_buffer((byte *)&u32ReadCommand, (sizeof(u32ReadCommand) / sizeof(uint8))) != SCP1_PASS)
                            {
                                //TRACE_DEBUG((0, "break out------- !! "));
                                cs4953x_spi_mutex_give();
                                cs4953x_irq_mutex_give();
                                
                                break;
                            }
                            
                            if (u32ReadCommand == AUTO_DETECTION_RESPONSE)
                            {
                                //TRACE_DEBUG((0, "audio type change------- !! "));
                                CS4953xSPI_nIrq_read_buffer((byte *)&u32ReadData, (sizeof(u32ReadData) / sizeof(uint8)));
                                mAdtectMsg = cs4953x_getAutoDectMsg( u32ReadData );
                                TRACE_DEBUG((0, "Audio stream changes = 0x%X, 0x%X", mAdtectMsg.non_iec61937, mAdtectMsg.stream_descriptor ));
                                cs4953x_spi_mutex_give();
                                cs4953x_SendEventToLoader( mAdtectMsg );
                            }
                            else
                            {
                                TRACE_DEBUG((0,"cs4953x_FchDetector no change----------------"));
                                cs4953x_spi_mutex_give();
                                cs4953x_irq_mutex_give();
                            }
                        }
                    }
                }
            }
                break;
        }

        vTaskDelay( FCH_DETECTOR_TIME_TICK );
    }
}

static void cs4953x_HardReset( void )
{
    GPIOMiddleLevel_Clr(__O_DSP_RES);
    VirtualTimer_sleep(1);

    GPIOMiddleLevel_Set(__O_DSP_RES);
    VirtualTimer_sleep(1);
}

#if 0
static uint8 cs495x_RowDataLoader_GetQueueNumber( void )
{
    uint8 i_qnumber = (uint8)uxQueueMessagesWaiting( xOS_CS4953x.loaderQueue.xQueue );
    //TRACE_DEBUG((0, "Q total size -----------%d !! ",i_qnumber));
    return i_qnumber;
}
#endif

static bool cs495x_RowDataLoader_InstrReceiver( CS4953x_QUEUE_TYPE *pType ) 
{
    if ( pType == NULL )
    {
        return FALSE;
    }
    
    if ( xOS_CS4953x.loaderQueue.xQueue == NULL )
    {
        xOS_CS4953x.xOS_ErrId = xOS_TASK_QUEUE_IS_NULL;
        return FALSE;
    }

    if ( xQueueReceive( xOS_CS4953x.loaderQueue.xQueue, pType, xOS_CS4953x.loaderQueue.xBlockTime ) != pdPASS )
    {
        xOS_CS4953x.xOS_ErrId = xOS_TASK_QUEUE_GET_FAIL;
        return FALSE;
    }

    return TRUE;
}

bool cs495x_RowDataLoader_InstrSender( void *parms ) 
{
    const CS4953x_QUEUE_TYPE* pQueueType = ( CS4953x_QUEUE_TYPE *)parms;

    if ( pQueueType == NULL )
    {
        return FALSE;
    }
    
    if ( xOS_CS4953x.loaderQueue.xQueue == NULL )
    {
        return FALSE;
    }

    if( pQueueType->audio_type == CS4953x_LOAD_NULL)
    {
        return FALSE;
    }

    if ( SPILowLevel_isEnable () == FALSE )
    {
        return FALSE;
    }

    xQueueReset(xOS_CS4953x.loaderQueue.xQueue);
    if ( xQueueSend( xOS_CS4953x.loaderQueue.xQueue, pQueueType, xOS_CS4953x.loaderQueue.xBlockTime ) != pdPASS )
    {
        xOS_CS4953x.xOS_ErrId = xOS_TASK_QUEUE_SET_FAIL;
        return FALSE;
    }

    return TRUE;

}

static void cs4953x_UpdateFormatIndicator( CS4953xLoadAudioStreamType stream_type )
{
	
	xADM_QUEUE_Events adm_q_event = ADM_QUEUE_EVENT_NULL;

	switch( stream_type )
	{
		case CS4953x_LOAD_PCM:
		{
			adm_q_event = ADM_QUEUE_EVENT_STREAM_FORMAT_INDICATOR_PCM;
			pADM_QueueObj->SignalDetector_sender( &adm_q_event );
		}
			break;
			
		case CS4953x_LOAD_AC3:
		{
			adm_q_event = ADM_QUEUE_EVENT_STREAM_FORMAT_INDICATOR_AC3;
			pADM_QueueObj->SignalDetector_sender( &adm_q_event );
					
		}
			break;
		case CS4953x_LOAD_DTS:
		{
			adm_q_event = ADM_QUEUE_EVENT_STREAM_FORMAT_INDICATOR_DTS;
			pADM_QueueObj->SignalDetector_sender( &adm_q_event );
		}
			break;

		default:
			break;

	}

}

static void cs4953x_LoadRowData( void )
{
    uint32 value = SLAVE_BOOT;
    CS4953x_QUEUE_TYPE tmp_QueueType;
	
	for( ;; )
	{
		switch( mLoaderState )
		{
			case LOADER_IDLE:
			{
				mLoaderState = LOADER_HANDLE_Q;
			}
				break;

			case LOADER_HANDLE_Q:
			{
				/*Update mode*/
				if (mQueueType.source_ctrl == CS4953x_SOURCE_USER)
				{
					mMode.EQ = mQueueType.EQ;
					mMode.srs_truvol = mQueueType.srs_truvol;
					mMode.srs_tshd = mQueueType.srs_tshd;
					
#if ( configSII_DEV_953x_PORTING == 1 )                     
					mMode.sample_96k = mQueueType.sample_96k;
					mMode.multi_channel = mQueueType.multi_channel;
					
				}
				else if (mQueueType.source_ctrl == CS4953x_SOURCE_HDMI)
				{
					mMode.sample_96k = mQueueType.sample_96k;
					mMode.multi_channel = mQueueType.multi_channel;
				}
#else
				}
#endif 
				else if (mQueueType.source_ctrl == CS4953x_SOURCE_PROCESS_MODE)
				{
					mMode.EQ = mQueueType.EQ;
					mMode.srs_truvol = mQueueType.srs_truvol;
					mMode.srs_tshd = mQueueType.srs_tshd;
				}

				if ( (gCurDSP_ULD_type == mQueueType.audio_type) && (mQueueType.source_ctrl != CS4953x_SOURCE_USER) && (DSPLoadErrorCount == 0))
				{
					//DSPLoadErrorCount = 0;
					mLoaderState = LOADER_END;
					tmp_QueueType.audio_type = mQueueType.audio_type;
					tmp_QueueType.source_ctrl = mQueueType.source_ctrl;
					
					pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType);
					if (mQueueType.source_ctrl == CS4953x_SOURCE_DSP||mQueueType.source_ctrl == CS4953x_SOURCE_DSP_INITIAL)
					{
						cs4953x_irq_mutex_give( );
					}
				}
				/*If user change the source, detect format first*/
				else if (mQueueType.audio_type == CS4953x_LOAD_USER_CTRL)
				{
					uint32 u32ReadData = 0;    // 4-byte Unsolicited Read Data
					CS4953x_ADECT_MSG mUserCtrlMsg;

					if(mQueueType.source_ctrl==CS4953x_SOURCE_USER)
					{
						/*Read format*/
				  /*Smith fixes bug: no any source is on sound bar, fmt could get AC3 formart.*/
						if ( CS49xxxxSPI_ReadSolicited(DSP_CFG_STREAM_TYPE,&u32ReadData) == FALSE )
						{
							TRACE_DEBUG((0, "CS49xxxxSPI_ReadSolicited u32ReadData error = 0x%X: %X", GET_HIGH_U16(u32ReadData), GET_LOW_U16(u32ReadData)));
							//DSPLoadErrorCount = 0;
							mLoaderState = LOADER_CONFIG_USER_SETTING; /*Smith implements for resume fch*/
							break;
						}
						
						TRACE_DEBUG((0, "CS4953x_SOURCE_USER: read =0x%X: 0x%X", GET_HIGH_U16(u32ReadData), GET_LOW_U16(u32ReadData)));

						mUserCtrlMsg = cs4953x_getAutoDectMsg( u32ReadData );

						TRACE_DEBUG((0, "CS4953x_SOURCE_USER: Audio stream changes = 0x%X, 0x%X", mUserCtrlMsg.non_iec61937, mUserCtrlMsg.stream_descriptor ));
						cs4953x_parseAudioFMT(&mQueueType, mUserCtrlMsg);

						if ( ((gCurDSP_ULD_type == mQueueType.audio_type) || (mQueueType.audio_type == CS4953x_LOAD_NULL)) && (DSPLoadErrorCount == 0))
						{
							//DSPLoadErrorCount = 0;
							mLoaderState = LOADER_END;
							tmp_QueueType.audio_type = mQueueType.audio_type;
							tmp_QueueType.source_ctrl = mQueueType.source_ctrl;
							pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType);
						}
						else
						{
							gCurDSP_ULD_type = mQueueType.audio_type; /*update ULD type*/
#if ( configAPP_INTERNAL_DSP_ULD == 0 )                
							if (ULDLoaderbuf == NULL)
							{
								ULDLoaderbuf = (unsigned char*)pvPortMalloc( DSPLoadSize);
							}
											
							if(ULDLoaderbuf == NULL)
							{
								TRACE_ERROR(( 0, "It isn't enough memory size %d", xPortGetFreeHeapSize() ));
								//DSPLoadErrorCount = 0;
								mLoaderState = LOADER_END;
							}
							else
							{
								mLoaderState = LOADER_RESET_DSP;
							}
#else
							pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_SET_MUTE);
							mLoaderState = LOADER_RESET_DSP;
#endif
						}
					}	 
					else if (mQueueType.source_ctrl==CS4953x_SOURCE_PROCESS_MODE)
					{
						mLoaderState = LOADER_SOFTBOOT; 
						mQueueType.audio_type = gCurDSP_ULD_type;
					}
				}
				else
				{
#if ( configSII_DEV_953x_PORTING == 1 )                   
					if(mQueueType.source_ctrl==CS4953x_SOURCE_HDMI)
					{
						mQueueType.audio_type = CS4953x_LOAD_PCM;
					}
#endif
					gCurDSP_ULD_type = mQueueType.audio_type; /*update ULD type*/

					/*Update format indicator - Smith*/
					cs4953x_UpdateFormatIndicator( gCurDSP_ULD_type );

#if ( configAPP_INTERNAL_DSP_ULD == 0 )                
					if (ULDLoaderbuf == NULL)
					{
						ULDLoaderbuf = (unsigned char*)pvPortMalloc( DSPLoadSize);
					}
									
					if(ULDLoaderbuf == NULL)
					{
						TRACE_ERROR(( 0, "It isn't enough memory size %d", xPortGetFreeHeapSize() ));
						//DSPLoadErrorCount = 0;
						mLoaderState = LOADER_END;
						cs4953x_irq_mutex_give( );
					}
					else
					{
						mLoaderState = LOADER_RESET_DSP;
					}
#else
					pAudLowLevel_ObjCtrl->SetVolRampEvent(AUD_VOL_EVENT_SET_MUTE);
					mLoaderState = LOADER_RESET_DSP;
#endif
				}
			}
				break;

			/*CS4953xx Hardware User's Manual p2-8*/
			case LOADER_RESET_DSP:
			{
				cs4953x_HardReset( );
				mLoaderState = LOADER_BOOT_ASSIST;
			}
				break;

			case LOADER_BOOT_ASSIST:
			{
				//! write slave boot @{
				value = SLAVE_BOOT;
				if (CS4953xSPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				} //@}
				
				if ( CS4953xSPI_MsgNote1((byte*)&value,sizeof(value)) != SCP1_PASS )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}

				if ( value != BOOT_START )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				
#if ( configAPP_INTERNAL_DSP_ULD == 0 )                
				/* load DSP Init ULD */
				if(cs495x_LoadDSP_ULD(ULDLoaderbuf,mQueueType.audio_type, OS_ULD) == 0)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
#else
				if (cs4953x_Load_DSP_ULD_InternalFlash(mQueueType.audio_type,OS_ULD) == -1 )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
#endif
				
				if ( CS4953xSPI_MsgNote1((byte*)&value,sizeof(value)) != SCP1_PASS )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
			   
				if ( value != BOOT_SUCCESS)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}

				mLoaderState = LOADER_SOFT_RESET_DSP_A;
			}
				break;

			case LOADER_SOFT_RESET_DSP_A:
			{
				value = SLAVE_BOOT;
				if (CS4953xSPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
								
				if ( CS4953xSPI_read_buffer((byte*)&value, sizeof(value)) == SCP1_IRQ_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				
				if ( value != BOOT_START)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}

				if (mQueueType.audio_type == CS4953x_LOAD_PCM)
				{
					mLoaderState = LOADER_CROSSBAR_ULD;
					break;
				}
				else
				{
					mLoaderState = LOADER_DECODER_ULD; 
					break;
				}
			}
				break;
			
			case LOADER_DECODER_ULD:
			{				   
				/* load DSP Decoder ULD */
#if ( configAPP_INTERNAL_DSP_ULD == 0 )                
				if(cs495x_LoadDSP_ULD(ULDLoaderbuf,mQueueType.audio_type,DECODER_ULD) == 0)
				{
					TRACE_DEBUG((0,"cs495x_LoadDSP_ULD LOADER_ERROR"));
					mLoaderState = LOADER_ERROR;
					break;
				}
#else
				if (cs4953x_Load_DSP_ULD_InternalFlash(mQueueType.audio_type, DECODER_ULD) == -1 )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
#endif 
				/*BOOT_SUCCESS*/
				if ( CS4953xSPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
				{
					
					mLoaderState = LOADER_ERROR;
					break;
				}
				
				if ( value != BOOT_SUCCESS)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				/* BOOT_START */
				value = SLAVE_BOOT;
				if (CS4953xSPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}

				if ( CS4953xSPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				
				if ( value != BOOT_START)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				mLoaderState = LOADER_CROSSBAR_ULD;
			}
				break;

			case LOADER_CROSSBAR_ULD:
			{
				/* load DSP CrossBar ULD */
#if ( configAPP_INTERNAL_DSP_ULD == 0 )                
				if(cs495x_LoadDSP_ULD(ULDLoaderbuf,mQueueType.audio_type,CROSSBAR_ULD) == 0)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
#else
				if (cs4953x_Load_DSP_ULD_InternalFlash(mQueueType.audio_type, CROSSBAR_ULD) == -1 )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
#endif 
				/*BOOT_SUCCESS*/
				if ( CS4953xSPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				
				if ( value != BOOT_SUCCESS)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				/* BOOT_START */
				value = SLAVE_BOOT;
				if (CS4953xSPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}

				if ( CS4953xSPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				
				if ( value != BOOT_START)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				mLoaderState = LOADER_GAIN_ULD;
			}	
				break;

			case LOADER_GAIN_ULD:
			{
#if ( configAPP_INTERNAL_DSP_ULD == 0 )            
				if(cs495x_LoadDSP_ULD(ULDLoaderbuf,mQueueType.audio_type,GAIN_ULD) == 0)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
#else
				if (cs4953x_Load_DSP_ULD_InternalFlash(mQueueType.audio_type, GAIN_ULD) == -1 )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
#endif 
				/* BOOT_SUCCESS */
				if ( CS4953xSPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
				{
					 mLoaderState = LOADER_ERROR;
					 break;
				}
								
				if ( value != BOOT_SUCCESS)
				{
					mLoaderState = LOADER_ERROR;
					break;
				}
				mLoaderState = LOADER_SOFT_RESET;
			}
				break;

			case LOADER_SOFTBOOT:
			{
				uint32 value = 0;
				uint32 soft_timeout = 0;
				value = SOFT_RESET;
				
				CS49xxxxSPI_CommandWrite(CMD_SOFT_BOOT, 0x00000001);
				soft_timeout = xTaskGetTickCount( );
				while( GPIOMiddleLevel_Read_InBit( __I_SCP1_IRQ ) == 1 )
				{
					if ( ((xTaskGetTickCount() - soft_timeout)/portTICK_RATE_MS) > TASK_MSEC2TICKS(100) )
					{
						soft_timeout = 0;
						mLoaderState = LOADER_ERROR;
						break;
					}
				}

				soft_timeout = xTaskGetTickCount( );
				do
				{
					while( GPIOMiddleLevel_Read_InBit( __I_SCP1_IRQ ) == 0 )
					{
						CS4953xSPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT;
					}
					if ( ((xTaskGetTickCount() - soft_timeout)/portTICK_RATE_MS) > TASK_MSEC2TICKS(1000) )
					{
						soft_timeout = 0;
						mLoaderState = LOADER_ERROR;
						break;
					}

				}while (value != SOFTBOOT_ACK);
				mLoaderState = LOADER_SOFT_RESET;				 
			}
				break;
				
			case LOADER_SOFT_RESET:
			{
				value = SOFT_RESET;
				if (CS4953xSPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;	  
				}
				/* DSP APP START */
				if ( CS4953xSPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break; 
				}

				if ( value != APP_START)
				{
					mLoaderState = LOADER_ERROR;
					break; 
				}

				mLoaderState = LOADER_CONIGURATION;
			}
				break;

		   case LOADER_CONIGURATION:
			{
#if ( configSII_DEV_953x_PORTING == 1 )        
				if(pMode->multi_channel==TRUE)
				{
					TRACE_DEBUG((0,"Multi_channel Enable"));
					if (CS4953xSPI_write_buffer((byte*)&PREKICKSTART_HDMI_CFG, (sizeof(PREKICKSTART_HDMI_CFG)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
					{
						mLoaderState = LOADER_ERROR;
						break;	  
					}
				}
				else
				{
					TRACE_DEBUG((0,"Multi_channel Disable"));
					if (CS4953xSPI_write_buffer((byte*)&PREKICKSTART_CFG, (sizeof(PREKICKSTART_CFG)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
					{
						mLoaderState = LOADER_ERROR;
						break;	  
					}
				}
#else
				if (CS4953xSPI_write_buffer((byte*)&PREKICKSTART_CFG, (sizeof(PREKICKSTART_CFG)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;	  
				}
#endif

				if(mQueueType.audio_type == CS4953x_LOAD_AC3)
				{
					if (CS4953xSPI_write_buffer((byte*)&AC3_INITIAL_CFG, (sizeof(AC3_INITIAL_CFG)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
					{
						mLoaderState = LOADER_ERROR;
						break; 
					}
					
					if ( pMode->srs_tshd )
					{
						if ( pMode->srs_truvol )
						{
							if (CS4953xSPI_write_buffer((byte*)&AC3_5_1_TVOLON, (sizeof(AC3_5_1_TVOLON)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "AC3 5.1 TVHDMC ON "));
						}
						else
						{
							if (CS4953xSPI_write_buffer((byte*)&AC3_5_1_TVOLOFF, (sizeof(AC3_5_1_TVOLOFF)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "AC3 5.1 TVHDMC OFF "));
						}
					}
					else
					{
						if ( pMode->srs_truvol )
						{
							if (CS4953xSPI_write_buffer((byte*)&AC3_2_1_TVOLON, (sizeof(AC3_2_1_TVOLON)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "AC3 2.1 TVHDMC ON "));
						}
						else
						{
							if (CS4953xSPI_write_buffer((byte*)&AC3_2_1_TVOLOFF, (sizeof(AC3_2_1_TVOLOFF)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "AC3 2.1 TVHDMC OFF "));
						}
					}
					
				}
				else if( mQueueType.audio_type == CS4953x_LOAD_DTS )
				{
					if (CS4953xSPI_write_buffer((byte*)&DTS_INITIAL_CFG, (sizeof(DTS_INITIAL_CFG)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
					{
						mLoaderState = LOADER_ERROR;
						break; 
					}

					if ( pMode->srs_tshd )
					{
						if ( pMode->srs_truvol )
						{
							if (CS4953xSPI_write_buffer((byte*)&DTS_5_1_TVOLON, (sizeof(DTS_5_1_TVOLON)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "DTS 5.1 TVHDMC ON "));
						}
						else
						{
							if (CS4953xSPI_write_buffer((byte*)&DTS_5_1_TVOLOFF, (sizeof(DTS_5_1_TVOLOFF)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "DTS 5.1 TVHDMC OFF "));
						}
					}
					else
					{
						if ( pMode->srs_truvol )
						{
							if (CS4953xSPI_write_buffer((byte*)&DTS_2_1_TVOLON, (sizeof(DTS_2_1_TVOLON)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "DTS 2.1 TVHDMC ON "));
						}
						else
						{
							if (CS4953xSPI_write_buffer((byte*)&DTS_2_1_TVOLOFF, (sizeof(DTS_2_1_TVOLOFF)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "DTS 2.1 TVHDMC OFF "));
						}
					}
				}
				else
				{
					if (CS4953xSPI_write_buffer((byte*)&PCM_INITIAL, (sizeof(PCM_INITIAL)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
					{
						mLoaderState = LOADER_ERROR;
						break; 
					}

					if ( pMode->srs_tshd )
					{
						if ( pMode->srs_truvol )
						{
							if (CS4953xSPI_write_buffer((byte*)&_5_1_TRUVOLON, (sizeof(_5_1_TRUVOLON)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "PCM 5.1 TVHDMC ON "));
						}
						else
						{
							if (CS4953xSPI_write_buffer((byte*)&_5_1_TRUVOLOFF, (sizeof(_5_1_TRUVOLOFF)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "PCM 5.1 TVHDMC OFF "));
						}
					}
					else
					{
						if ( pMode->srs_truvol )
						{
							if (CS4953xSPI_write_buffer((byte*)&_2_1_TRUVOLON, (sizeof(_2_1_TRUVOLON)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "PCM 2.1 TVHDMC ON "));
						}
						else
						{
							if (CS4953xSPI_write_buffer((byte*)&_2_1_TRUVOLOFF, (sizeof(_2_1_TRUVOLOFF)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
							{
								mLoaderState = LOADER_ERROR;
								break; 
							}
							TRACE_DEBUG((0, "PCM 2.1 TVHDMC OFF "));
						}
					}

					if (pMode->multi_channel == TRUE)
					{
						if (pMode->sample_96k == TRUE)
						{
							TRACE_DEBUG((0,"96K"));
							CS49xxxxSPI_CommandWrite(PCM_ENABLE, 0x00000101);/* FS Divid  by 2 */
							CS49xxxxSPI_CommandWrite(IO_BUFF_1_SOURCE, 0x00000005);
							CS49xxxxSPI_CommandWrite(IO_BUFF_2_SOURCE, 0x00000004);
							CS49xxxxSPI_CommandWrite(IO_BUFF_3_SOURCE, 0x00000003);
							CS49xxxxSPI_CommandWrite(IO_BUFF_4_SOURCE, 0x00000007);
							CS49xxxxSPI_CommandWrite(IO_BUFF_5_SOURCE, 0x00000002);
							CS49xxxxSPI_CommandWrite(IO_BUFF_6_SOURCE, 0x00000006);
							CS49xxxxSPI_CommandWrite(IO_BUFF_7_SOURCE, 0x00000001);
						}
						else /*48k*/
						{
							CS49xxxxSPI_CommandWrite(IO_BUFF_1_SOURCE, 0x00000003);
							CS49xxxxSPI_CommandWrite(IO_BUFF_2_SOURCE, 0x00000001);
							CS49xxxxSPI_CommandWrite(IO_BUFF_3_SOURCE, 0x00000004);
							CS49xxxxSPI_CommandWrite(IO_BUFF_4_SOURCE, 0x00000005);
							CS49xxxxSPI_CommandWrite(IO_BUFF_5_SOURCE, 0x00000006);
							CS49xxxxSPI_CommandWrite(IO_BUFF_6_SOURCE, 0x00000007);
							CS49xxxxSPI_CommandWrite(IO_BUFF_7_SOURCE, 0x00000002);
						}
						
						TRACE_DEBUG((0,"Multi_channel !!!"));
						CS49xxxxSPI_CommandWrite(TV_ENABLE, 0x00000000);
						CS49xxxxSPI_CommandWrite(TV_BYPASS, 0x00000001);
						CS49xxxxSPI_CommandWrite(AUTODETECT_CONFIG, 0x00000000);
						CS49xxxxSPI_CommandWrite(AUTOSWITCH_ENABLE, 0x00000000);
					}
					
				}

				mLoaderState = LOADER_KICK_START;

			}
				break;

			case LOADER_KICK_START:
			{
				if (CS4953xSPI_write_buffer((byte*)&KICKSTART_CFG, (sizeof(KICKSTART_CFG)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
				{
					mLoaderState = LOADER_ERROR;
					break;
				}

#if ( configAPP_INTERNAL_DSP_ULD == 0 )            
				if(ULDLoaderbuf != NULL)
				{
					vPortFree(ULDLoaderbuf);
					ULDLoaderbuf = NULL;
				}
#endif                 

				gCurDSP_ULD_type = mQueueType.audio_type; /*update ULD type*/
				mLoaderState = LOADER_CONFIG_USER_SETTING;				 
			}
				break;

			case LOADER_CONFIG_USER_SETTING:
			{
				mLoaderState = LOADER_END;
				//DSPLoadErrorCount = 0;
				
				pAudLowLevel_ObjCtrl->AudParmsConf( NULL );
				tmp_QueueType.audio_type = mQueueType.audio_type;
				tmp_QueueType.source_ctrl = mQueueType.source_ctrl;
				
				pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType);
				
				if (mQueueType.source_ctrl == CS4953x_SOURCE_DSP ||mQueueType.source_ctrl == CS4953x_SOURCE_DSP_INITIAL)
				{
					cs4953x_irq_mutex_give( );
				}
			}
				break;

			case LOADER_ERROR:
			{
				TRACE_ERROR((0, "DSP is not working, Reset DSP !! %d ", DSPLoadErrorCount));

#if ( configAPP_INTERNAL_DSP_ULD == 0 )            
				if( ULDLoaderbuf != NULL )
				{
					vPortFree( ULDLoaderbuf );
					ULDLoaderbuf = NULL;
				}
#endif                 
				
				if ( DSPLoadErrorCount >= 10 )
				{
					DSPLoadErrorCount = 0;
                                        tmp_QueueType.source_ctrl = CS4953x_SOURCE_ERROR;
    					pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType); /*The loader task will be suspened - Smith*/
					mLoaderState = LOADER_END;

                                        if (mQueueType.source_ctrl == CS4953x_SOURCE_DSP||mQueueType.source_ctrl == CS4953x_SOURCE_DSP_INITIAL)
                                        {
                                            cs4953x_irq_mutex_give( );
                                        }
				}
                                else
                                {
                                    mLoaderState = LOADER_HANDLE_Q;
                                    DSPLoadErrorCount++;
                                }

			}
				break;

			case LOADER_END:
			{
                                DSPLoadErrorCount = 0;
				cs4953x_irq_mutex_give();

				mLoaderState = LOADER_IDLE;

				TRACE_DEBUG((0, "loader procedure is finished"));

				return; /*Procedure finished - Smith*/
			}
				break;
		}
	
		
	}

}

void cs4953x_RowDataLoader_Task( void *pvParameters )
{
	for( ;; )
	{
		if ( cs495x_RowDataLoader_InstrReceiver( &mQueueType ) == TRUE )
		{
			cs4953x_LoadRowData( );	
		}
	}
}


static void cs495x_FchDetector_setState( TaskHandleState set_state )
{
    if ( set_state == TASK_SUSPENDED )
    {
        if ( xOS_CS4953x.fch_detecter_state == TASK_RUNING )
        {
            xOS_CS4953x.fch_detecter_state = TASK_SUSPENDED;
        }
    }
    else if ( set_state == TASK_RUNING )
    {
        if ( xOS_CS4953x.fch_detecter_state == TASK_SUSPENDED )
        {
            xOS_CS4953x.fch_detecter_state = TASK_RUNING;
        }
    }
}

static void cs495x_FchDetector_TaskCtrl( xOS_TaskCtrl val )
{
    if ( val == xOS_setTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_CS4953x.fmtTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_CS4953x.fmtTaskHandle );
         
            xOS_CS4953x.fch_detecter_state = TASK_SUSPENDED; /*Smith implemets: Source switching cound make system crashing whe task was supended*/
            //TRACE_DEBUG((0, "SUSPEND: cs495x_FchDetector"));
        }
    }
    else if( val == xOS_setTASK_RESUME) 
    {
        if ( xTaskIsTaskSuspended( xOS_CS4953x.fmtTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_CS4953x.fmtTaskHandle );
            //TRACE_DEBUG((0, "RESUME: cs495x_FchDetector"));
        }
    }

}

static void cs4953x_FchDetector_CreateTask( void )
{
    if ( xTaskCreate( 
            cs4953x_FchDetector, 
            ( portCHAR * ) "CS4953x_FCH", 
            (STACK_SIZE), 
            NULL, 
            tskCS4953x_FCH_HANDLER_PRIORITY, 
            &xOS_CS4953x.fmtTaskHandle ) != pdPASS )
    {
        vTaskDelete( xOS_CS4953x.fmtTaskHandle );
        xOS_CS4953x.xOS_ErrId = xOS_TASK_CREATE_FAIL;
    }
    else
    {
        vTaskSuspend( xOS_CS4953x.fmtTaskHandle );
    }
}

static void cs495x_RowDataLoader_TaskCtrl( xOS_TaskCtrl val )
{
    xQueueReset( xOS_CS4953x.loaderQueue.xQueue );

	/*Smith implemets: Initial queue*/
    memset(&mQueueType, 0, sizeof(mQueueType)); /*Smith fixes: Source switching makes system crashing*/
    mLoaderState = LOADER_IDLE;
    gCurDSP_ULD_type = CS4953x_LOAD_NULL;
    mAdtectMsg.non_iec61937 = 0;
    mAdtectMsg.stream_descriptor = 0 ;
    
    if ( val == xOS_setTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_CS4953x.loaderTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_CS4953x.loaderTaskHandle );
            TRACE_DEBUG((0, "SUSPEND: cs495x_RowDataLoader"));
        }
    }
    else if ( val == xOS_setTASK_RESUME )
    {
        if ( xTaskIsTaskSuspended( xOS_CS4953x.loaderTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_CS4953x.loaderTaskHandle );
            TRACE_DEBUG((0, "RESUME cs495x_RowDataLoader"));
        }
    }
}

static void cs495x_RowDataLoader_CreateTask( void )
{
    xOS_CS4953x.loaderQueue.xQueue = xQueueCreate( LOADER_QUEUE_LENGTH, FRTOS_SIZE(CS4953x_QUEUE_TYPE) );
    xOS_CS4953x.loaderQueue.xBlockTime = portMAX_DELAY; /*no wait - Smith*/
    if( xOS_CS4953x.loaderQueue.xQueue == NULL )
    {
       xOS_CS4953x.xOS_ErrId = xOS_TASK_QUEUE_CREATE_FAIL;
    }

    xOS_CS4953x.spi_sema.xSemaphore = xSemaphoreCreateMutex();
    xOS_CS4953x.spi_sema.xBlockTime = portMAX_DELAY;
    
    if ( _IRQ_SEMA == NULL )
    {
        _IRQ_SEMA = xSemaphoreCreateMutex();
    }   
	
    if ( xTaskCreate( 
            cs4953x_RowDataLoader_Task, 
            ( portCHAR * ) "CS4953x_LOADER", 
            (STACK_SIZE), 
            NULL, 
            tskCS4953x_Loader_PRIORITY, 
            &xOS_CS4953x.loaderTaskHandle ) != pdPASS )
    {
        vTaskDelete( xOS_CS4953x.loaderTaskHandle );
        xOS_CS4953x.xOS_ErrId = xOS_TASK_CREATE_FAIL;
    }
    else
    {
        vTaskSuspend( xOS_CS4953x.loaderTaskHandle );
    }

}   

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
/********* For Load DSP ULD from External Flash *********/
static unsigned int cs495x_Get_DSP_ULD_StartLocation(CS4953xLoadAudioStreamType Stype ,cs495x_ULD_Type Utype)
{
    unsigned int sLocation = 0;
    
    if(Stype == CS4953x_LOAD_PCM)
    {
        sLocation = PCM_ULD_LOCATION[Utype][0];
    }
    else if(Stype == CS4953x_LOAD_AC3)
    {
        sLocation = AC3_ULD_LOCATION[Utype][0];
    }
    else if(Stype == CS4953x_LOAD_DTS)
    {
        sLocation = DTS_ULD_LOCATION[Utype][0];
    }
    return sLocation;
}
#endif 

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
static unsigned int cs495x_Get_DSP_ULD_Length(CS4953xLoadAudioStreamType Stype ,cs495x_ULD_Type Utype)
{
    unsigned int sLocation = 0;
    
    if(Stype == CS4953x_LOAD_PCM)
    {
        sLocation = PCM_ULD_LOCATION[Utype][1];
    }
    else if(Stype == CS4953x_LOAD_AC3)
    {
        sLocation = AC3_ULD_LOCATION[Utype][1];
    }
    else if(Stype == CS4953x_LOAD_DTS)
    {
        sLocation = DTS_ULD_LOCATION[Utype][1];
    }
    return sLocation;
}
#endif 

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
static int cs495x_LoadDSP_ULD(unsigned char* buf,CS4953xLoadAudioStreamType Stype ,cs495x_ULD_Type Utype )
{
    int i;
    int8 dsp_spi_ret;
    unsigned int sLocation,ULDLength;  
    unsigned int NumOfDSPLoad,NumOfSingle,Addr;
    int8 ret_val = 1;

    sLocation = cs495x_Get_DSP_ULD_StartLocation(Stype,Utype);
    ULDLength = cs495x_Get_DSP_ULD_Length(Stype,Utype);

    Addr = sLocation;
    NumOfDSPLoad = ULDLength/DSPLoadSize;
    NumOfSingle = ULDLength%DSPLoadSize;

    if( NumOfDSPLoad == 0 )
    {
        sFLASH_ReadBuffer(ULDLoaderbuf, Addr, NumOfSingle);

        dsp_spi_ret = CS4953xSPI_write_ULD_buffer(ULDLoaderbuf, NumOfSingle);
        if( dsp_spi_ret != SCP1_PASS )
        {
            TRACE_DEBUG((0, "Step 1. Write DSP ULD have ERROR !!!\n"));
            ret_val = 0;
        }
    }
    else
    {
        for( i=0; i < NumOfDSPLoad; i++)
        {
            sFLASH_ReadBuffer(ULDLoaderbuf, Addr, DSPLoadSize);

            dsp_spi_ret = CS4953xSPI_write_ULD_buffer(ULDLoaderbuf, DSPLoadSize);
            if ( dsp_spi_ret != SCP1_PASS )
            {
                TRACE_DEBUG((0, "Step 2. Write DSP ULD have ERROR !!!\n"));
                ret_val = 0;
                break;
            }

             Addr += DSPLoadSize;
        }

        if (ret_val != 0)
        {
            sFLASH_ReadBuffer(ULDLoaderbuf, Addr, NumOfSingle);

            dsp_spi_ret = CS4953xSPI_write_ULD_buffer(ULDLoaderbuf, NumOfSingle);
            if ( dsp_spi_ret != SCP1_PASS )
            {
                TRACE_DEBUG((0, "Step 3. Write DSP ULD have ERROR !!!\n"));
                ret_val = 0;
            }
        }
    }
    

    if (ret_val != 0)
    {
#if 1		
        if(Stype == CS4953x_LOAD_PCM)
        {
            if(Utype == GAIN_ULD)
                TRACE_DEBUG((0, "Load DSP  \"PCM \"  Finished !!!\n"));
            else if(Utype == OS_ULD)
                TRACE_DEBUG((0, "Load DSP  \"PCM OS_ULD\"  Finished !!!\n"));
            else if(Utype == CROSSBAR_ULD)
                TRACE_DEBUG((0, "Load DSP  \"PCM CROSSBAR_ULD\"  Finished !!!\n"));
        }
        else if(Stype == CS4953x_LOAD_AC3)
        {
            if(Utype == GAIN_ULD)
                TRACE_DEBUG((0, "Load DSP  \"AC3 \"  Finished !!!\n"));
            else if(Utype == OS_ULD)
                TRACE_DEBUG((0, "Load DSP  \"AC3 OS_ULD\"  Finished !!!\n"));
            else if(Utype == CROSSBAR_ULD)
                TRACE_DEBUG((0, "Load DSP  \"AC3 CROSSBAR_ULD\"  Finished !!!\n"));
            else if(Utype == DECODER_ULD)   
                TRACE_DEBUG((0, "Load DSP  \"AC3 DECODER_ULD\"  Finished !!!\n"));
        }
        else if(Stype == CS4953x_LOAD_DTS)
        {
            if(Utype == GAIN_ULD)
                TRACE_DEBUG((0, "Load DSP  \"DTS \"  Finished !!!\n"));
            else if(Utype == OS_ULD)
                TRACE_DEBUG((0, "Load DSP  \"DTS OS_ULD\"  Finished !!!\n"));
            else if(Utype == CROSSBAR_ULD)
                TRACE_DEBUG((0, "Load DSP  \"DTS CROSSBAR_ULD\"  Finished !!!\n"));
            else if(Utype == DECODER_ULD)   
                TRACE_DEBUG((0, "Load DSP  \"DTS DECODER_ULD\"  Finished !!!\n"));
        }
#endif		
    }
    
    return ret_val;
}
#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs4953x_Load_DSP_AC3_ULD_InternalFlash( cs495x_ULD_Type module )
{
    int8 ret = 0;

    switch( module )
    {
        case GAIN_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if( CS4953xSPI_write_buffer((byte*)&AC3_IN_GAIN_ULD, (sizeof(DTS_IN_GAIN_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }
        
                cs4953x_spi_mutex_give();
            }
        }
            break;

        case OS_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if (CS4953xSPI_write_buffer((byte*)&OS_HD2_RC94_ULD, (sizeof(OS_HD2_RC94_ULD)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;                   
                }

                cs4953x_spi_mutex_give();
            }
        }
            break;

        case CROSSBAR_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if (CS4953xSPI_write_buffer((byte*)&AC3_RC23_ULD, (sizeof(AC3_RC23_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }

                cs4953x_spi_mutex_give();
            }
        }
            break;

    }

     return ret;
}
#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs4953x_Load_DSP_DTS_ULD_InternalFlash( cs495x_ULD_Type module )
{
    int8 ret = 0;

    switch( module )
    {
        case GAIN_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if( CS4953xSPI_write_buffer((byte*)&DTS_IN_GAIN_ULD, (sizeof(DTS_IN_GAIN_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }
        
                cs4953x_spi_mutex_give();
            }
        }
            break;

        case OS_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if (CS4953xSPI_write_buffer((byte*)&OS_HD2_RC94_ULD, (sizeof(OS_HD2_RC94_ULD)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;                   
                }

                cs4953x_spi_mutex_give();
            }
        }
            break;

        case CROSSBAR_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if (CS4953xSPI_write_buffer((byte*)&DTS_RC25_ULD, (sizeof(DTS_RC25_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }

                cs4953x_spi_mutex_give();
            }
        }
            break;
    }

    return ret;

}
#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs4953x_Load_DSP_PCM_ULD_InternalFlash( cs495x_ULD_Type module )
{
    int8 ret = 0; 

    switch( module )
    {
        case GAIN_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if( CS4953xSPI_write_buffer((byte*)&PCM_IN_GAIN_ULD, (sizeof(PCM_IN_GAIN_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }
        
                cs4953x_spi_mutex_give();
            }
        }
            break;

        case OS_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if (CS4953xSPI_write_buffer((byte*)&OS_HD2_RC94_ULD, (sizeof(OS_HD2_RC94_ULD)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;                   
                }

                cs4953x_spi_mutex_give();
            }
        }
            break;

        case CROSSBAR_ULD:
        {
            if ( cs4953x_spi_mutex_take() == TRUE )
            {
                if (CS4953xSPI_write_buffer((byte*)&PCM_TVCSII_ULD, (sizeof(PCM_TVCSII_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }

                cs4953x_spi_mutex_give();
            }
        }
            break;

    }

    return ret;
}
#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs4953x_Load_DSP_ULD_InternalFlash( CS4953xLoadAudioStreamType type, cs495x_ULD_Type module )
{
    int8 ret = 0;

    switch( type )
    {
        case CS4953x_LOAD_PCM:
        {
            ret = cs4953x_Load_DSP_PCM_ULD_InternalFlash( module );
        }
            break;

        case CS4953x_LOAD_AC3:
        {
             ret = cs4953x_Load_DSP_AC3_ULD_InternalFlash( module );
        }
            break;

        case CS4953x_LOAD_DTS:
        {
             ret = cs4953x_Load_DSP_DTS_ULD_InternalFlash( module );
        }
            break;
    }

    return ret;
}
#endif 

