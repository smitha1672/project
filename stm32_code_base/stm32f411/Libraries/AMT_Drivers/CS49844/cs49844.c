#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"
#include "AudioDeviceManager.h"

#include "audio_dsp_uld.h"
#include "audio_gain_parameters.h"

#include "GPIOMiddleLevel.h"
#include "cs49844_spi.h"
#include "cs49844.h"
#include "UIDeviceManager.h"
#include "ext_flash_driver.h"

//___________________________________________________________________________________
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;

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
    LOADER_ERROR
}CS49844_LoaderState;



typedef enum
{
    ATMOS_LOADER_IDLE = 0,
    ATMOS_LOADER_RESET_DSP,    
}CS49844_ATMOS_LoaderState;

typedef enum{
    ATMOS_APP_ULD = 0,
    ATMOS_DOLBY_OAR_ULD,    
    ATMOS_OS_ULD,
    ATMOS_HDMI_ULD,
    ATMOS_MAT_ULD
}cs49844_ATMOS_ULD_Type;




typedef struct CS49844_LOADER_PARAMETERS
{
    xTaskHandle loaderTaskHandle;
    xTaskHandle fmtTaskHandle;
    xTaskHandle lowlevelTaskHandle;
    xOS_TaskErrIndicator xOS_ErrId;
    TaskHandleState fch_detecter_state;
    TaskHandleState lowlevel_state;
    xSemaphoreParameters spi_sema;
    xQueueParameters loaderQueue;
} xOS_CS49844Params;

typedef struct _CS49844_AUTODETECT_RESPONSE
{
    uint8 non_iec61937;
    uint8 stream_descriptor;
}CS49844_ADECT_MSG;

typedef enum{
    OS_ULD = 0,
    DECODER_ULD,    
    CROSSBAR_ULD,
    GAIN_ULD,
}cs49844_ULD_Type;

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

#define CS49844_CMD_BASE 0xf0000000

//! gain_multi_channel_friendly_plus24 (Ls-Rs Gain2) @{
#define RS_G2 	( CS49844_CMD_BASE + _g_0_Ls_Rs_Gain2151 )
#define LS_G2	( CS49844_CMD_BASE + _g_1_Ls_Rs_Gain2151 )

// gain_multi_channel_plus24 (Center Gain 1)
#define CENTER_G1  ( CS49844_CMD_BASE +  _g_0_Center_Gain_1118 )

// gain_multi_channel_plus24 (Center Gain 2) 
#define CENTER_G2  ( CS49844_CMD_BASE + _g_0_Center_Gain_2152 )

// gain_multi_channel_friendly_plus24 (Subwoofer Gain 1)
#define SUBWOOFER_G1 ( CS49844_CMD_BASE + _g_0_Subwoofer_Gain_1120 )  

// gain_multi_channel_friendly_plus24 (Subwoofer Gain 2)
#define SUBWOOFER_G2 ( CS49844_CMD_BASE + _g_0_Subwoofer_Gain_2132 )

// gain_multi_channel_plus24 (Master Volume)
// g_1 : signed : 5.27 format
#define MASTER_GAIN1  ( CS49844_CMD_BASE + _g_0_Master_Volume131 ) 
// g_2 : signed : 5.27 format
#define MASTER_GAIN2  ( CS49844_CMD_BASE + _g_1_Master_Volume131 )
// g_3 : signed : 5.27 format
#define MASTER_GAIN3  ( CS49844_CMD_BASE + _g_2_Master_Volume131 )
// g_4 : signed : 5.27 format
#define MASTER_GAIN4  ( CS49844_CMD_BASE + _g_3_Master_Volume131 )
// g_5 : signed : 5.27 format
#define MASTER_GAIN5  ( CS49844_CMD_BASE + _g_4_Master_Volume131 )
// g_6 : signed : 5.27 format
#define MASTER_GAIN6  ( CS49844_CMD_BASE + _g_5_Master_Volume131 )

#if defined ( S4051A )
//gain_multi_channel_friendly_plus24 (Ls-Rs Gain 1)
// g_1 : signed : 5.27 format
#define BALANCE_LS  ( CS49844_CMD_BASE + _g_0_Ls_Rs_Gain_1_1160 )
// g_2 : signed : 5.27 format
#define BALANCE_RS  ( CS49844_CMD_BASE + _g_1_Ls_Rs_Gain_1_1160 )

#else
//gain_multi_channel_friendly_plus24 (Ls-Rs Gain 1)
// g_1 : signed : 5.27 format
#define BALANCE_LS  ( CS49844_CMD_BASE + _g_0_Ls_Rs_Gain_1119 )
// g_2 : signed : 5.27 format
#define BALANCE_RS  ( CS49844_CMD_BASE + _g_1_Ls_Rs_Gain_1119 )
#endif

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

#define CS49844_READ_BASE 0xf0C00000
#define CS49844_CMD_READ_PRESENCE_LEFT	( CS49844_READ_BASE + _presence_Presence122 )
#define CS49844_CMD_READ_PRESENCE_RIGHT ( CS49844_READ_BASE + _presence_Presence73 )	

#define CS49844_RESPONSE_BASE 0x70C00000
#define PRESENCE_RESPONSE_LEFT ( CS49844_RESPONSE_BASE + _presence_Presence122 )
#define PRESENCE_RESPONSE_RIGHT	( CS49844_RESPONSE_BASE + _presence_Presence73 )

#define READ_RESPONSE_MESSAGE 0x6fc00000
#define DSP_CFG_STREAM_TYPE READ_RESPONSE_MESSAGE+0x00000002
//___________________________________________________________________________
#define LOADER_QUEUE_LENGTH 2
#define AUTO_DETECTION_RESPONSE (0x81000000)
#define FCH_DETECTOR_TIME_TICK  TASK_MSEC2TICKS(10)
#define LOADER_TIME_TICK TASK_MSEC2TICKS(10)
#define LOWLEVEL_TIME_TICK TASK_MSEC2TICKS(500)
#define SOFTRESET_TIMEOUT TASK_MSEC2TICKS(1000)
//___________________________________________________________________________
CS49844_ADECT_MSG mAdtectMsg;

//___________________________________________________________________________
static CS49844_LoaderState mLoaderState = LOADER_IDLE;
static xOS_CS49844Params xOS_CS49844;  
static CS49844LoadAudioStreamType gCurDSP_ULD_type = CS49844_LOAD_PCM;
static const CS49844LoadAudioStreamType *pCurtDSP_ULD_TYPE = &gCurDSP_ULD_type;
static uint8 signal_overthreshold = 1;
static xSemaphoreHandle _IRQ_SEMA = NULL;

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


/* ATMOS load procedure: APP -> DOLBY -> OS -> ATMOS_HDMI -> ATMOS_MAT */
unsigned int ATMOS_ULD_LOCATION[5][2] = 
{
    {ATM_APP_ULD_START_LOCATION,ATM_APP_ULD_LENGTH}, /* APP */
    {ATM_DOLBY_ULD_START_LOCATION,ATM_DOLBY_ULD_LENGTH}, /* Dolby */     
    {ATM_OS_ULD_START_LOCATION,ATM_OS_ULD_LENGTH},    /* OS */
    {ATM_HDMI_ULD_START_LOCATION,ATM_HDMI_ULD_LENGTH},  /* Hdmi */
    {ATM_MAT_ULD_START_LOCATION,ATM_MAT_ULD_LENGTH} /* Mat */
};

static unsigned char* ULDLoaderbuf = NULL;
#endif 

//___________________________________________________________________________
static void cs49844_RowDataLoader_CreateTask( void );

static bool cs49844_RowDataLoader_InstrReceiver( CS49844_QUEUE_TYPE *pType ) ;

static uint8 cs49844_RowDataLoader_GetQueueNumber( void );

static void cs49844_HardReset( void );

static void cs49844_FchDetector_CreateTask( void );

static void cs49844_FchDetector_TaskCtrl( xOS_TaskCtrl val );

static void cs49844_RowDataLoader_TaskCtrl( xOS_TaskCtrl val );

static bool cs49844_RowDataLoader_InstrSender( void *parms ) ;

static void cs49844_LowLevelDetector_CreateTask( void );

static void cs49844_LowLevelDetector_TaskCtrl( xOS_TaskCtrl val );

static void cs49844_setMute( bool val );

static bool cs49844_spi_mutex_take( void );

static bool cs49844_spi_mutex_give( void );

static bool cs49844_irq_mutex_take( void );

static bool cs49844_irq_mutex_give( void );

static void cs49844_setMasterGain( uint32 value );

static void cs49844_setBassGain( uint32 value );

static void cs49844_setTrebleGain( uint32 value );

static void cs49844_setSubGain( uint32 value );

static void cs49844_setCenterGain( uint32 value );

static void cs49844_setLsRsGain( uint32 value );

static void cs49844_setBalanceLs( uint32 value );

static void cs49844_setBalanceRs( uint32 value );

static void cs49844_SetTVHDMC_Ctrl( byte* data, uint16 length );

static void cs49844_setNightMode( uint32 value );

static void cs49844_setAVDelay( uint32 value );

static void cs49844_setAudioRoute(CS49844AudioRoute val);

static void cs49844_setSilenceThreshold( bool value );

static CS49844LoadAudioStreamType cs49844_GetAudioStreamType( void );

static uint8 cs49844_GetLoadrState( void );

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
static int cs49844_LoadDSP_ULD(unsigned char* buf,CS49844LoadAudioStreamType Stype ,cs49844_ULD_Type Utype );

static int cs49844_LoadDSP_ATMOS_ULD(unsigned char* buf,cs49844_ATMOS_ULD_Type Utype );

#endif 

uint8 cs49844_getSignalLevel( void );

static void cs49844_lowlevel_task_setState( TaskHandleState set_state );

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs49844_Load_DSP_ULD_InternalFlash( CS49844LoadAudioStreamType type, cs49844_ULD_Type module );
#endif 

const DSP_CTRL_OBJECT DSP_ObjCtrl = 
{
    cs49844_RowDataLoader_CreateTask,
    cs49844_RowDataLoader_TaskCtrl,
    cs49844_FchDetector_CreateTask,
    cs49844_FchDetector_TaskCtrl,
    cs49844_RowDataLoader_InstrSender,
    cs49844_LowLevelDetector_CreateTask,
    cs49844_LowLevelDetector_TaskCtrl,
    cs49844_lowlevel_task_setState,
    cs49844_setMute,
    cs49844_setMasterGain,
    cs49844_setBassGain,
    cs49844_setTrebleGain,
    cs49844_setSubGain,
    cs49844_setCenterGain,
    cs49844_setLsRsGain,
    cs49844_setBalanceLs,
    cs49844_setBalanceRs,
    cs49844_SetTVHDMC_Ctrl,
    cs49844_setNightMode,
    cs49844_setAVDelay,
    cs49844_setAudioRoute,
    cs49844_setSilenceThreshold,
    cs49844_GetAudioStreamType,
    cs49844_GetLoadrState,
    cs49844_getSignalLevel,
    cs49844_spi_mutex_take,
    cs49844_spi_mutex_give
};

const DSP_CTRL_OBJECT *pDSP_ObjCtrl = &DSP_ObjCtrl;

//___________________________________________________________________________
static void cs49844_setMute( bool val )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        if ( val == TRUE ) /*mute*/
        {
            CS49844SPI_CommandWrite( 0x83000001, 0x00000001 );
            //TRACE_DEBUG((0, "cs49844_setMute CS49844SPI_CommandWrite mute"));
        }
        else
        {
            CS49844SPI_CommandWrite( 0x83000001, 0x00000000 );
            //TRACE_DEBUG((0, "cs49844_setMute CS49844SPI_CommandWrite unmute"));
        }

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setMasterGain( uint32 value )
{
    if ( value > 0x08000000 )
        return;        
    
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(MASTER_GAIN1, value );
        CS49844SPI_CommandWrite(MASTER_GAIN2, value );
        CS49844SPI_CommandWrite(MASTER_GAIN3, value );
        CS49844SPI_CommandWrite(MASTER_GAIN4, value );
        CS49844SPI_CommandWrite(MASTER_GAIN5, value );
        CS49844SPI_CommandWrite(MASTER_GAIN6, value );
        
        cs49844_spi_mutex_give();
    }
}

static void cs49844_setBassGain( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
 
        CS49844SPI_CommandWrite(_bass_level_left, value);
        CS49844SPI_CommandWrite(_bass_level_center, value);
        CS49844SPI_CommandWrite(_bass_level_right, value);
        CS49844SPI_CommandWrite(_bass_level_ls, value);
        CS49844SPI_CommandWrite(_bass_level_rs, value); 

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setTrebleGain( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(_treble_level_left, value);
        CS49844SPI_CommandWrite(_treble_level_center, value);
        CS49844SPI_CommandWrite(_treble_level_right, value);
        CS49844SPI_CommandWrite(_treble_level_ls, value);
        CS49844SPI_CommandWrite(_treble_level_rs, value); 

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setSubGain( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(SUBWOOFER_G2, value );

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setCenterGain( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(CENTER_G2, value);  

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setLsRsGain( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(RS_G2, value);  
        CS49844SPI_CommandWrite(LS_G2, value);  

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setBalanceLs( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(BALANCE_LS, value);     

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setBalanceRs( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(BALANCE_RS, value);     

        cs49844_spi_mutex_give();
    }
}

static void cs49844_SetTVHDMC_Ctrl( byte* data, uint16 length )
{
    if ( mLoaderState == LOADER_IDLE )
    {
        if (cs49844_spi_mutex_take() == TRUE )
        {
            CS49844SPI_write_buffer(data, length);
        
            cs49844_spi_mutex_give();
        }
    }
}

static void cs49844_setNightMode( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        CS49844SPI_CommandWrite(SUBWOOFER_G1, value );
    
        cs49844_spi_mutex_give();
    }
}

static void cs49844_setAVDelay( uint32 value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
       // CS49844SPI_CommandWrite(DELAY_VALUE_LEFT, DELAY_TIME_LEFT + 0x140000 * value );
        
        //CS49844SPI_CommandWrite(DELAY_VALUE_CENTER, DELAY_TIME_CENTER +  0x140000 * value );
        //CS49844SPI_CommandWrite(DELAY_VALUE_RIGHT, DELAY_TIME_RIGHT +  0x140000 * value ); 
        //CS49844SPI_CommandWrite(DELAY_VALUE_LS, DELAY_TIME_LS +  0x140000 * value );
        //CS49844SPI_CommandWrite(DELAY_VALUE_RS, DELAY_TIME_RS +  0x140000 * value );
        //CS49844SPI_CommandWrite(DELAY_VALUE_LFE, DELAY_TIME_LFE +  0x140000 * value );
    
        cs49844_spi_mutex_give();
    }
}

static void cs49844_setAudioRoute(CS49844AudioRoute val)
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        if ( val == ROUTE_INITIAL )
        {
            CS49844SPI_write_buffer((byte*)&DSP_DAO_ROUTER_INITIAL, (sizeof(DSP_DAO_ROUTER_INITIAL)/sizeof(uint8)));
        }

        /*Clean internal channel map*/
        //CS49844SPI_CommandWrite(DAO_CHAN_0_REMAP, DAO_RESERVED );
        //CS49844SPI_CommandWrite(DAO_CHAN_1_REMAP, DAO_RESERVED );
        //CS49844SPI_CommandWrite(DAO_CHAN_2_REMAP, DAO_RESERVED );
        //CS49844SPI_CommandWrite(DAO_CHAN_3_REMAP, DAO_RESERVED );
        //CS49844SPI_CommandWrite(DAO_CHAN_4_REMAP, DAO_RESERVED );
        //CS49844SPI_CommandWrite(DAO_CHAN_5_REMAP, DAO_RESERVED );
        //CS49844SPI_CommandWrite(DAO_CHAN_6_REMAP, DAO_RESERVED );
        //CS49844SPI_CommandWrite(DAO_CHAN_7_REMAP, DAO_RESERVED );
   
        
        switch( val )
        {
            case ROUTE_FL_FR:
            {
               // CS49844SPI_CommandWrite( DAO_CHAN_0_REMAP, DAO_LEFT ); /*FL*/
               // CS49844SPI_CommandWrite( DAO_CHAN_1_REMAP, DAO_RIGHT ); /*FR*/
            }
                break;
        
            case ROUTE_RR_RL:
            {
              //  CS49844SPI_CommandWrite( DAO_CHAN_6_REMAP, DAO_LEFT ); /*RL*/
              //  CS49844SPI_CommandWrite( DAO_CHAN_7_REMAP, DAO_RIGHT ); /*RR*/
            }
                break;
            
            case ROUTE_FL:
            {
                /*channel 0 outputs left sound*/
               // CS49844SPI_CommandWrite( DAO_CHAN_0_REMAP, DAO_LEFT ); /*FL*/           
            }
                break;
        
            case ROUTE_FR:
            {
               // CS49844SPI_CommandWrite( DAO_CHAN_1_REMAP, DAO_RIGHT ); /*FR*/
            }
                break;
        
            case ROUTE_RL:
            {
              //  CS49844SPI_CommandWrite( DAO_CHAN_6_REMAP, DAO_LEFT ); /*RL*/
            }
                break;
        
            case ROUTE_RR:
            {
                //CS49844SPI_CommandWrite( DAO_CHAN_7_REMAP, DAO_RIGHT ); /*RR*/
            }
                break;
        
            case ROUTE_CENTER:
            {
                /*Smith: Fix center outputs too huge, when mode was in factory mode */
               // CS49844SPI_CommandWrite( DAO_CHAN_3_REMAP, DAO_CENTER );
              //  CS49844SPI_CommandWrite( DAO_CHAN_2_REMAP, DAO_CENTER ); /*CENTER*/        
            }
                break;
        
            case ROUTE_LFE0:
            {
               // CS49844SPI_CommandWrite( DAO_CHAN_4_REMAP, DAO_RIGHT ); /*LFE0*/  
              //  CS49844SPI_CommandWrite( DAO_CHAN_5_REMAP, DAO_LEFT ); /*LFE0*/                
            }
                break;
        
            case ROUTE_FL_FR_CENTER:
            {
              //  CS49844SPI_CommandWrite( DAO_CHAN_0_REMAP, DAO_LEFT ); /*FL*/   
               // CS49844SPI_CommandWrite( DAO_CHAN_1_REMAP, DAO_RIGHT ); /*FR*/
                
                /*Smith: Fix center outputs too huge, when mode was in factory mode */  
              //  CS49844SPI_CommandWrite( DAO_CHAN_3_REMAP, DAO_CENTER );
              //  CS49844SPI_CommandWrite( DAO_CHAN_2_REMAP, DAO_CENTER ); /*CENTER*/
            }
            break;
            
            case ROUTE_RR_RL_WOOFER:
            {
             //   CS49844SPI_CommandWrite( DAO_CHAN_6_REMAP, DAO_LEFT ); /*RL*/
             //   CS49844SPI_CommandWrite( DAO_CHAN_7_REMAP, DAO_RIGHT ); /*RR*/
                
             //   CS49844SPI_CommandWrite( DAO_CHAN_4_REMAP, DAO_RIGHT ); /*LFE0*/  
             //   CS49844SPI_CommandWrite( DAO_CHAN_5_REMAP, DAO_LEFT ); /*LFE0*/ 
            }
            break;

			case ROUTE_DISABLE:
            default:
            {
             //   CS49844SPI_write_buffer((byte*)&DSP_DAO_ROUTER_INITIAL, (sizeof(DSP_DAO_ROUTER_INITIAL)/sizeof(uint8)));
            }
                break;
        }

        cs49844_spi_mutex_give();
    }
}

static void cs49844_setSilenceThreshold( bool value )
{
    if (cs49844_spi_mutex_take() == TRUE )
    {
        if (value)
        {
         //   CS49844SPI_CommandWrite(0x81000008, 0x0083d600);/* Silence threshold 90s */
        }
        else
        {
          //  CS49844SPI_CommandWrite(0x81000008, 0x00005dc0);/* Silence_threshold 0.25 second */
        }

        cs49844_spi_mutex_give();
    }
}

static CS49844LoadAudioStreamType cs49844_GetAudioStreamType( void )
{
    return gCurDSP_ULD_type;
}

static uint8 cs49844_GetLoadrState( void )
{
    return mLoaderState;
}

static int CS49844_readPresence( uint8 *pValue )
{
    int ret = 0;
#if 0    
    int8 spi_rError1 = 0;
    int8 spi_rError2 = 0; 
    uint32 LeftChannelSignal;
    uint32 RightChannelSignal;
    uint32 command = 0;
    uint32 value;
    uint8 cnt;
    if ( pValue == NULL )
        return -1;

    if (cs49844_spi_mutex_take() == TRUE )
    {

        /*AN288 2.1.2 Solicited Read*/
        cnt = 0;
        do 
        {
            /*Read Command Word(left)*/
            command = CS49844_CMD_READ_PRESENCE_LEFT;
            if (CS49844SPI_write_buffer((byte*)&command,(sizeof(command)/sizeof(uint8))) != SCP1_PASS )
            {
                ret = -2;
            }
            /*Read response command word*/
            spi_rError1 = CS49844SPI_read_buffer((byte*)&value,(sizeof(command)/sizeof(uint8))); 

            /*Read response Data word*/ 
            spi_rError2 = CS49844SPI_read_buffer((byte*)&value, (sizeof(command)/sizeof(uint8)));
            
            if ( spi_rError1 == SCP1_PASS && spi_rError2== SCP1_PASS)
            {   
                LeftChannelSignal = value;
                break;
            }
            
            cnt++;
            if (cnt>2)
            {
                ret = -3;
                break;
            }
        }while((spi_rError1!= SCP1_PASS)||(spi_rError2!= SCP1_PASS));        
        cnt=0;
        do 
        {
            /*Read Command Word(right)*/
            command = CS49844_CMD_READ_PRESENCE_RIGHT;
            if (CS49844SPI_write_buffer((byte*)&command,(sizeof(command)/sizeof(uint8))) != SCP1_PASS )
            {
                ret = -4;
            }
                  
            /*Read response command word*/
            spi_rError1 = CS49844SPI_read_buffer((byte*)&value, (sizeof(command)/sizeof(uint8)));

            /*Read response Data word*/ 
            spi_rError2 = CS49844SPI_read_buffer((byte*)&value, (sizeof(command)/sizeof(uint8)));

            if ( spi_rError1 == SCP1_PASS && spi_rError2 == SCP1_PASS)
            {   
                RightChannelSignal = value;
                break;
            }

            cnt++;
            if (cnt>2)
            {
                ret = -5;
                break;
            }
        }while((spi_rError1!= SCP1_PASS)||(spi_rError2!= SCP1_PASS));
        
        if ( ret == 0 )
        {
            if( ( LeftChannelSignal | RightChannelSignal ) )
            {
                *pValue = 1;
            }
            else
            {
                *pValue = 0;
            }
        }
        else
        {
            *pValue = 1;
        }


        cs49844_spi_mutex_give();
    }
#endif
    return ret;
}

uint8 cs49844_getSignalLevel( void )
{
    return signal_overthreshold;
}

static bool cs49844_spi_mutex_take( void )
{
    if ( SPILowLevel_isEnable() != TRUE )
    {
        //TRACE_DEBUG((0, "SPI TAKE BUSY 1111 !! "));
        return FALSE;
    }   

    if ( xOS_CS49844.spi_sema.xSemaphore == NULL )
    {
        //TRACE_DEBUG((0, "SPI TAKE BUSY 2222 !! "));
        return FALSE;
    }

    xOS_CS49844.spi_sema.xBlockTime = portMAX_DELAY;
    if ( xSemaphoreTake( xOS_CS49844.spi_sema.xSemaphore, xOS_CS49844.spi_sema.xBlockTime ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "SPI TAKE BUSY 3333 !! "));
        return FALSE;
    }

    return TRUE;
}

static bool cs49844_spi_mutex_give( void )
{
    if ( SPILowLevel_isEnable() != TRUE )
        return FALSE;

    if ( xOS_CS49844.spi_sema.xSemaphore == NULL )
        return FALSE;

    if ( xSemaphoreGive( xOS_CS49844.spi_sema.xSemaphore ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "SPI GIVE BUSY !! "));
        return FALSE;	
    }

    return TRUE;
}

static bool cs49844_irq_mutex_take( void )
{
    if ( _IRQ_SEMA == NULL )
    {
        //TRACE_DEBUG((0, "IRQ TAKE BUSY 2222 !! "));
        return FALSE;
    }

    if ( xSemaphoreTake( _IRQ_SEMA, 0 ) != pdTRUE )
    //if ( xSemaphoreTake( xOS_CS49844.spi_sema.xSemaphore, 0xff ) != pdTRUE )
    {
        //TRACE_DEBUG((0, "IRQ TAKE BUSY 3333 !! "));
        return FALSE;
    }

    return TRUE;    
}

static bool cs49844_irq_mutex_give( void )
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

static void cs49844_lowlevel_task_setState( TaskHandleState set_state )
{
    if ( set_state == TASK_SUSPENDED )
    {
        if ( xOS_CS49844.lowlevel_state == TASK_RUNING )
        {
            xOS_CS49844.lowlevel_state = TASK_SUSPENDED;
        }
    }
    else if ( set_state == TASK_RUNING )
    {
        if ( xOS_CS49844.lowlevel_state == TASK_SUSPENDED )
        {
            xOS_CS49844.lowlevel_state = TASK_RUNING;
        }
    }
}

void cs49844_lowlevel_task( void *pvParameters )
{
    int ret = 0;

    for( ;; )
    {
        switch( xOS_CS49844.lowlevel_state )
        {
            case TASK_SUSPENDED:
            {
            }
                break;

            case TASK_RUNING:
            {
                if( (*pCurtDSP_ULD_TYPE == CS49844_LOAD_PCM ) && ( mLoaderState == LOADER_IDLE ) )
                {
                    //ret = CS49844_readPresence( &signal_overthreshold );
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

static void cs49844_LowLevelDetector_TaskCtrl( xOS_TaskCtrl val )
{
    signal_overthreshold = 1;

    if ( val == xOS_setTASK_SUSPENDED )
    {
        cs49844_lowlevel_task_setState( TASK_SUSPENDED );
        if ( xTaskIsTaskSuspended( xOS_CS49844.lowlevelTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_CS49844.lowlevelTaskHandle );
        }
    }
    else if ( val == xOS_setTASK_RESUME )
    {
        cs49844_lowlevel_task_setState( TASK_RUNING );
        if ( xTaskIsTaskSuspended( xOS_CS49844.lowlevelTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_CS49844.lowlevelTaskHandle );
            //TRACE_DEBUG((0, "RESUME: CS49844_lowlevel"));
        }
    }
}

static void cs49844_LowLevelDetector_CreateTask( void )
{
    signal_overthreshold = 0;
    if ( xTaskCreate( 
            cs49844_lowlevel_task, 
            ( portCHAR * ) "CS49844_lowlevel", 
            (STACK_SIZE), 
            NULL, 
            tskCS49844_LowLevelDetection_PRIORITY, 
            &xOS_CS49844.lowlevelTaskHandle ) != pdPASS )
    {
        vTaskDelete( xOS_CS49844.lowlevelTaskHandle );
    }
    else
    {
        vTaskSuspend( xOS_CS49844.lowlevelTaskHandle );
    }
}   

static CS49844_ADECT_MSG cs49844_getAutoDectMsg( uint32 data )
{
    CS49844_ADECT_MSG msg;

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

static void cs49844_parseAudioFMT(CS49844_QUEUE_TYPE *tmp_queue_data, CS49844_ADECT_MSG msg)
{
    tmp_queue_data->audio_type = CS49844_LOAD_NULL;
    
    if ( msg.non_iec61937 == 0x01)/*PCM*/
    {
        switch( msg.stream_descriptor)
        {
            case NON_IEC61937_DTS_FORMAT_16:
            case NON_IEC61937_DTS_FORMAT_14:
            {
                tmp_queue_data->audio_type = CS49844_LOAD_DTS;
            }
                break;

            case NON_IEC61937_SILENT_INPUT_DATA:{}break;
            
            default:
            {
                tmp_queue_data->audio_type = CS49844_LOAD_PCM;
                TRACE_DEBUG((0, "tmp_queue_data->audio_type = CS49844_LOAD_PCM !! "));
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
                tmp_queue_data->audio_type = CS49844_LOAD_AC3;
            }
                break;

            case IEC61937_STREAM_DTS_1:
            case IEC61937_STREAM_DTS_2:
            case IEC61937_STREAM_DTS_3:
            {
                tmp_queue_data->audio_type = CS49844_LOAD_DTS;
            }
                break;
            
            default:
            {
                tmp_queue_data->audio_type = CS49844_LOAD_PCM;
            }
                break;
        }
    }
}

static void cs49844_SendEventToLoader( CS49844_ADECT_MSG msg )
{
    CS49844_QUEUE_TYPE QUEUE_TYPE;

    cs49844_parseAudioFMT(&QUEUE_TYPE, msg);
            
    if (QUEUE_TYPE.audio_type != CS49844_LOAD_NULL)
    {
#if 1
        //TRACE_DEBUG((0, "cs49844_SendEventToLoader !! "));

        if (cs49844_spi_mutex_take( ) == TRUE)
        //if ( cs49844_irq_mutex_take() == TRUE )
        {
            //TRACE_ERROR((0, "send message to data loader ----- !! "));
            QUEUE_TYPE.source_ctrl = CS49844_SOURCE_DSP;
            cs49844_RowDataLoader_InstrSender(&QUEUE_TYPE);
        }
        else
        {
            //TRACE_ERROR((0, "Can't send message to data loader ----- !! "));
            cs49844_irq_mutex_give();

        }    
#else
        //TRACE_ERROR((0, "send message to data loader ----- !! "));
        cs49844_RowDataLoader_InstrSender(&QUEUE_TYPE);
#endif
    }
    else
    {
        cs49844_irq_mutex_give();
        //TRACE_ERROR((0, "send message CS49844_LOAD_NULL ----- !! "));
    }
}

void cs49844_FchDetector( void *pvParameters )
{
    uint32 u32ReadCommand = 0;
    uint32 u32ReadData;    // 4-byte Unsolicited Read Data
    
    for( ;; )
    {

        if( GPIOMiddleLevel_Read_InBit( __I_SCP1_IRQ ) == 0 )
        {
            if (cs49844_irq_mutex_take() == TRUE)
            {
                if ( cs49844_spi_mutex_take() == TRUE )
                {
                    if (CS49844SPI_nIrq_read_buffer((byte *)&u32ReadCommand, (sizeof(u32ReadCommand) / sizeof(uint8))) != SCP1_PASS)
                    {
                        //TRACE_DEBUG((0, "Read DSP IRQ error ------- !! "));
                    }
                    cs49844_spi_mutex_give();
                    cs49844_irq_mutex_give();
                }
            }
        }
    #if 0
        switch (xOS_CS49844.fch_detecter_state )
        {
            case TASK_SUSPENDED:
            {
                xOS_CS49844.fch_detecter_state = TASK_RUNING;
            }
                break;
          
            case TASK_RUNING:
            {

                break;
            
                if( GPIOMiddleLevel_Read_InBit( __I_SCP1_IRQ ) == 0 )
                {
                    //if ( cs49844_spi_mutex_give() == TRUE )
                    //TRACE_DEBUG((0, "cs49844_FchDetector IRQ enter !! "));
                    if (cs49844_irq_mutex_take() == TRUE)
                    {
                        if ( cs49844_spi_mutex_take() == TRUE )
                        //if ( cs49844_irq_mutex_take() == TRUE )
                        {
                            //TRACE_DEBUG((0, "take spi mutex------- !! "));
                            if (CS49844SPI_nIrq_read_buffer((byte *)&u32ReadCommand, (sizeof(u32ReadCommand) / sizeof(uint8))) != SCP1_PASS)
                            {
                                //pAudLowLevel_ObjCtrl->RstSRC();
                                //TRACE_DEBUG((0, "break out------- !! "));
                                cs49844_spi_mutex_give();
                                cs49844_irq_mutex_give();
                                
                                break;
                            }
                            
                            if (u32ReadCommand == AUTO_DETECTION_RESPONSE)
                            {
                                //TRACE_DEBUG((0, "audio type change------- !! "));
                                CS49844SPI_nIrq_read_buffer((byte *)&u32ReadData, (sizeof(u32ReadData) / sizeof(uint8)));
                                mAdtectMsg = cs49844_getAutoDectMsg( u32ReadData );
                                //TRACE_DEBUG((0, "-Audio stream changes = 0x%X, 0x%X", mAdtectMsg.non_iec61937, mAdtectMsg.stream_descriptor ));
                                cs49844_spi_mutex_give();
                                //TRACE_DEBUG((0,"cs49844_FchDetector SPI enter"));
                                cs49844_SendEventToLoader( mAdtectMsg );
                            }
                            else
                            {
                                //TRACE_DEBUG((0,"cs49844_FchDetector no change----------------"));
                                cs49844_spi_mutex_give();
                                cs49844_irq_mutex_give();
                            }
                            //TRACE_DEBUG((0, "111111111111break out------- !! "));
                        }
                    }
                }
            }
                break;
             
        }
        #endif  
        vTaskDelay( FCH_DETECTOR_TIME_TICK );
    }

}

static void cs49844_HardReset( void )
{
    GPIOMiddleLevel_Set(__O_DSP_RES);
    vTaskDelay(TASK_MSEC2TICKS(1));
    GPIOMiddleLevel_Clr(__O_DSP_RES);
    vTaskDelay(TASK_MSEC2TICKS(1));
    GPIOMiddleLevel_Set(__O_DSP_RES);
    vTaskDelay(TASK_MSEC2TICKS(1));

    //TRACE_DEBUG((0, "DSP HARDWARE RESET !! "));
}

static uint8 cs49844_RowDataLoader_GetQueueNumber( void )
{
    uint8 i_qnumber = (uint8)uxQueueMessagesWaiting( xOS_CS49844.loaderQueue.xQueue );
    //TRACE_DEBUG((0, "Q total size -----------%d !! ",i_qnumber));
    return i_qnumber;
}

static bool cs49844_RowDataLoader_InstrReceiver( CS49844_QUEUE_TYPE *pType ) 
{
    if ( pType == NULL )
    {
        return FALSE;
    }
    
    if ( xOS_CS49844.loaderQueue.xQueue == NULL )
    {
        xOS_CS49844.xOS_ErrId = xOS_TASK_QUEUE_IS_NULL;
        return FALSE;
    }

    if ( xQueueReceive( xOS_CS49844.loaderQueue.xQueue, pType, xOS_CS49844.loaderQueue.xBlockTime ) != pdPASS )
    {
        xOS_CS49844.xOS_ErrId = xOS_TASK_QUEUE_GET_FAIL;
        return FALSE;
    }

    return TRUE;
}

bool cs49844_RowDataLoader_InstrSender( void *parms ) 
{
    const CS49844_QUEUE_TYPE* pQueueType = ( CS49844_QUEUE_TYPE *)parms;

    if ( pQueueType == NULL )
    {
        return FALSE;
    }
    
    if ( xOS_CS49844.loaderQueue.xQueue == NULL )
    {
        return FALSE;
    }

    //if( pQueueType->audio_type == CS49844_LOAD_NULL || pQueueType->source_ctrl == CS49844_SOURCE_NONE)
    if( pQueueType->audio_type == CS49844_LOAD_NULL)
    {
        return FALSE;
    }

    if ( SPILowLevel_isEnable () == FALSE )
    {
        return FALSE;
    }

#if 0 //repeat type handle 
    if ( gCurDSP_ULD_type == *pType )
        return FALSE;
#endif

    xQueueReset(xOS_CS49844.loaderQueue.xQueue);
    if ( xQueueSend( xOS_CS49844.loaderQueue.xQueue, pQueueType, xOS_CS49844.loaderQueue.xBlockTime ) != pdPASS )
    {
        xOS_CS49844.xOS_ErrId = xOS_TASK_QUEUE_SET_FAIL;
        return FALSE;
    }

    return TRUE;

}


void CS49844_ATMOS_Loader(void)
{

}


void cs49844_RowDataLoader_Task( void *pvParameters )
{
    static CS49844_QUEUE_TYPE QueueType;
    uint32 value = SLAVE_BOOT;
    CS49844_QUEUE_TYPE tmp_QueueType;
    
    for( ;; )
    {
        switch( mLoaderState )
        {
            case LOADER_IDLE:
            {
                if( cs49844_RowDataLoader_GetQueueNumber( ) > 0 ) 
                {
                    mLoaderState = LOADER_HANDLE_Q;
                }
                else
                {
                    //pADM_ObjCtrl->SetBackCtrlStatus(BACKCTRL_FCH_NOCHANGE);
                    cs49844_spi_mutex_give( );
                    cs49844_irq_mutex_give();
                }
            }
                break;

            case LOADER_HANDLE_Q:
            {
                if ( cs49844_RowDataLoader_GetQueueNumber( ) > 0 )
                {
                    cs49844_RowDataLoader_InstrReceiver( &QueueType );
                }

                if ( gCurDSP_ULD_type == QueueType.audio_type)
                {
                    mLoaderState = LOADER_IDLE;
                    tmp_QueueType.audio_type = QueueType.audio_type;
                    tmp_QueueType.source_ctrl = QueueType.source_ctrl;
                    
                    pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType);
                    cs49844_spi_mutex_give( );
                    if (QueueType.source_ctrl == CS49844_SOURCE_DSP)
                    {
                        cs49844_irq_mutex_give( );
                    }
                }
                /*If user change the source, detect format first*/
                else if (QueueType.audio_type == CS49844_LOAD_USER_CTRL)
                {
                    uint32 u32ReadData;    // 4-byte Unsolicited Read Data
                    CS49844_ADECT_MSG mUserCtrlMsg;

                    if(QueueType.source_ctrl==CS49844_SOURCE_USER)
                    {
                        /*Read format*/
                        CS49844SPI_ReadSolicited(DSP_CFG_STREAM_TYPE,&u32ReadData);
                        mUserCtrlMsg = cs49844_getAutoDectMsg( u32ReadData );
                        cs49844_parseAudioFMT(&QueueType, mUserCtrlMsg);

                        if ( (gCurDSP_ULD_type == QueueType.audio_type) || (QueueType.audio_type == CS49844_LOAD_NULL))
                        {
                            mLoaderState = LOADER_IDLE;
                            tmp_QueueType.audio_type = QueueType.audio_type;
                            tmp_QueueType.source_ctrl = QueueType.source_ctrl;

                            pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType);
                            cs49844_spi_mutex_give( );                        
                        }
                        else
                        {
#if ( configAPP_INTERNAL_DSP_ULD == 0 )                
                            if (ULDLoaderbuf == NULL)
                            {
                                ULDLoaderbuf = (unsigned char*)pvPortMalloc( DSPLoadSize);
                            }
                                            
                            if(ULDLoaderbuf == NULL)
                            {
                                TRACE_ERROR(( 0, "It isn't enough memory size %d", xPortGetFreeHeapSize() ));
                                mLoaderState = LOADER_IDLE;
                                cs49844_spi_mutex_give( );
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
                    else if (QueueType.source_ctrl==CS49844_SOURCE_PROCESS_MODE)
                    {
                        mLoaderState = LOADER_SOFTBOOT; 
                        QueueType.audio_type = gCurDSP_ULD_type;
                    }
                }
                else
                {
#if ( configAPP_INTERNAL_DSP_ULD == 0 )                
                    if (ULDLoaderbuf == NULL)
                    {
                        ULDLoaderbuf = (unsigned char*)pvPortMalloc( DSPLoadSize);
                    }
                                    
                    if(ULDLoaderbuf == NULL)
                    {
                        TRACE_ERROR(( 0, "It isn't enough memory size %d", xPortGetFreeHeapSize() ));
                        mLoaderState = LOADER_IDLE;
                        cs49844_spi_mutex_give( );
                        cs49844_irq_mutex_give( );
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

            /*CS49844x Hardware User's Manual p2-8*/
            case LOADER_RESET_DSP:
            {
                cs49844_HardReset( );
                mLoaderState = LOADER_BOOT_ASSIST;
            }
                break;

            case LOADER_BOOT_ASSIST:
            {
                //! write slave boot @{
                value = SLAVE_BOOT;
                if (CS49844SPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                } //@}
                
                vTaskDelay(TASK_MSEC2TICKS(1));
                if ( CS49844SPI_MsgNote1((byte*)&value,sizeof(value)) != SCP1_PASS )
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
                if(cs49844_LoadDSP_ULD(ULDLoaderbuf,QueueType.audio_type, OS_ULD) == 0)
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#else
                if (cs49844_Load_DSP_ULD_InternalFlash(QueueType.audio_type,OS_ULD) == -1 )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#endif
                
                vTaskDelay(TASK_MSEC2TICKS(1));
                if ( CS49844SPI_MsgNote1((byte*)&value,sizeof(value)) != SCP1_PASS )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
               
                if ( value != BOOT_SUCCESS)
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }

                //TRACE_DEBUG((0, "DSP BOOT_ASSIST BOOT SUCCESS !! "));
                mLoaderState = LOADER_SOFT_RESET_DSP_A;
            }
                break;

            case LOADER_SOFT_RESET_DSP_A:
            {
                value = SLAVE_BOOT;
                if (CS49844SPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT)
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
                                
                if ( CS49844SPI_read_buffer((byte*)&value, sizeof(value)) == SCP1_IRQ_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
                
                if ( value != BOOT_START)
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }

                if (QueueType.audio_type == CS49844_LOAD_PCM)
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
                if(cs49844_LoadDSP_ULD(ULDLoaderbuf,QueueType.audio_type,DECODER_ULD) == 0)
                {
                    TRACE_DEBUG((0,"cs49844_LoadDSP_ULD LOADER_ERROR"));
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#else
                if (cs49844_Load_DSP_ULD_InternalFlash(QueueType.audio_type, DECODER_ULD) == -1 )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#endif 
                vTaskDelay(TASK_MSEC2TICKS(1));
                /*BOOT_SUCCESS*/
                if ( CS49844SPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
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
                if (CS49844SPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }

                if ( CS49844SPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
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
                if(cs49844_LoadDSP_ULD(ULDLoaderbuf,QueueType.audio_type,CROSSBAR_ULD) == 0)
                {
                    TRACE_DEBUG((0,"CrossBar ULD LOADER_ERROR"));
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#else
                if (cs49844_Load_DSP_ULD_InternalFlash(QueueType.audio_type, CROSSBAR_ULD) == -1 )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#endif 
                vTaskDelay(TASK_MSEC2TICKS(1));
                /*BOOT_SUCCESS*/
                if ( CS49844SPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
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
                if (CS49844SPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }

                if ( CS49844SPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
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
                if(cs49844_LoadDSP_ULD(ULDLoaderbuf,QueueType.audio_type,GAIN_ULD) == 0)
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#else
                if (cs49844_Load_DSP_ULD_InternalFlash(QueueType.audio_type, GAIN_ULD) == -1 )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
#endif 
                vTaskDelay(TASK_MSEC2TICKS(1));
                /* BOOT_SUCCESS */
                if ( CS49844SPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
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

                CS49844SPI_CommandWrite(CMD_SOFT_BOOT, 0x00000001);
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
                        CS49844SPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT;
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
                if (CS49844SPI_write_buffer((byte*)&value, (sizeof(value)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break;    
                }
                /* DSP APP START */
                if ( CS49844SPI_read_buffer((byte*)&value,sizeof(value)) == SCP1_IRQ_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break; 
                }

                if ( value != APP_START)
                {
                    mLoaderState = LOADER_ERROR;
                    break; 
                }

                //TRACE_DEBUG((0, "DSP APP START  !!"));
                mLoaderState = LOADER_CONIGURATION;
            }
                break;

            case LOADER_CONIGURATION:
            {
                if (CS49844SPI_write_buffer((byte*)&PREKICKSTART_CFG, (sizeof(PREKICKSTART_CFG)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break;    
                }

                //TRACE_DEBUG((0, "PRE KICK START !"));

                if(QueueType.audio_type == CS49844_LOAD_AC3)
                {
                    if (CS49844SPI_write_buffer((byte*)&AC3_INITIAL_CFG, (sizeof(AC3_INITIAL_CFG)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                    {
                        mLoaderState = LOADER_ERROR;
                        break; 
                    }
                }
                else if( QueueType.audio_type == CS49844_LOAD_DTS )
                {
                    if (CS49844SPI_write_buffer((byte*)&DTS_INITIAL_CFG, (sizeof(DTS_INITIAL_CFG)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                    {
                        mLoaderState = LOADER_ERROR;
                        break; 
                    }
                }
                else
                {
                    if (CS49844SPI_write_buffer((byte*)&PCM_INITIAL, (sizeof(PCM_INITIAL)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                    {
                        mLoaderState = LOADER_ERROR;
                        break; 
                    }
                }
                mLoaderState = LOADER_KICK_START;

            }
            break;

            case LOADER_KICK_START:
            {
                if (CS49844SPI_write_buffer((byte*)&KICKSTART_CFG, (sizeof(KICKSTART_CFG)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                {
                    mLoaderState = LOADER_ERROR;
                    break;
                }
                //TRACE_DEBUG((0, "DSP KICK_START !! "));

#if ( configAPP_INTERNAL_DSP_ULD == 0 )            
                if(ULDLoaderbuf != NULL)
                {
                    vPortFree(ULDLoaderbuf);
                                ULDLoaderbuf = NULL;
                    //TRACE_DEBUG((0, "Free ULD Loader Buffer memory space !!"));
                }
#endif                 

                gCurDSP_ULD_type = QueueType.audio_type; /*update ULD type*/
                mLoaderState = LOADER_CONFIG_USER_SETTING;               
            }
                break;

            case LOADER_CONFIG_USER_SETTING:
            {
                mLoaderState = LOADER_IDLE;                           
                if( cs49844_RowDataLoader_GetQueueNumber( ) == 0 ) 
                {
                    //TRACE_ERROR((0, "Q size ----->>>>>> 00000 !! "));
                    cs49844_spi_mutex_give( );
                    pAudLowLevel_ObjCtrl->AudParmsConf( NULL );
                    tmp_QueueType.audio_type = QueueType.audio_type;
                    tmp_QueueType.source_ctrl = QueueType.source_ctrl;
                    
                    pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType);
                    if (QueueType.source_ctrl == CS49844_SOURCE_DSP)
                    {
                        cs49844_irq_mutex_give( );
                    }
                }
            }
                break;

            case LOADER_ERROR:
            {
                TRACE_ERROR((0, "DSP is not working, Reset DSP !! "));

#if ( configAPP_INTERNAL_DSP_ULD == 0 )            
                if( ULDLoaderbuf != NULL )
                {
                    vPortFree( ULDLoaderbuf );
                    ULDLoaderbuf = NULL;
                }
#endif                 
                mLoaderState = LOADER_HANDLE_Q;

                cs49844_spi_mutex_give( );
                if (QueueType.source_ctrl == CS49844_SOURCE_DSP)
                {
                    cs49844_irq_mutex_give( );
                }

                tmp_QueueType.source_ctrl = CS49844_SOURCE_ERROR;
                pAudLowLevel_ObjCtrl->BackCtrlSender(&tmp_QueueType);
            }
                break;
        }
        vTaskDelay(LOADER_TIME_TICK);
    }
}

static void cs49844_FchDetector_TaskCtrl( xOS_TaskCtrl val )
{
    if ( val == xOS_setTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_CS49844.fmtTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_CS49844.fmtTaskHandle );
            //TRACE_DEBUG((0, "SUSPEND: cs49844_FchDetector"));
        }
    }
    else if( val == xOS_setTASK_RESUME) 
    {
        if ( xTaskIsTaskSuspended( xOS_CS49844.fmtTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_CS49844.fmtTaskHandle );
            //TRACE_DEBUG((0, "RESUME: cs49844_FchDetector"));
        }
    }

}

static void cs49844_FchDetector_CreateTask( void )
{
    if ( xTaskCreate( 
            cs49844_FchDetector, 
            ( portCHAR * ) "CS49844_FCH", 
            (STACK_SIZE), 
            NULL, 
            tskCS49844_FCH_HANDLER_PRIORITY, 
            &xOS_CS49844.fmtTaskHandle ) != pdPASS )
    {
        vTaskDelete( xOS_CS49844.fmtTaskHandle );
        xOS_CS49844.xOS_ErrId = xOS_TASK_CREATE_FAIL;
    }
    else
    {
        //vTaskSuspend( xOS_CS49844.fmtTaskHandle );    //Always monitor DSP..
    }



    xOS_CS49844.spi_sema.xSemaphore = xSemaphoreCreateMutex();
    if ( xOS_CS49844.spi_sema.xSemaphore != NULL )
    {
#if 0
        if ( cs49844_spi_mutex_take( ) == TRUE )
        {
            xOS_CS49844.xOS_ErrId = xOS_TASK_ERR_NULL;
        }
#endif        
    }
   
    xOS_CS49844.spi_sema.xBlockTime = 0x0;
    
    if ( _IRQ_SEMA == NULL )
    {
        _IRQ_SEMA = xSemaphoreCreateMutex();
    }  
}

static void cs49844_RowDataLoader_TaskCtrl( xOS_TaskCtrl val )
{
    xQueueReset( xOS_CS49844.loaderQueue.xQueue );
    
    mLoaderState = LOADER_IDLE;
    gCurDSP_ULD_type = CS49844_LOAD_NULL;
    mAdtectMsg.non_iec61937 = 0;
    mAdtectMsg.stream_descriptor = 0 ;
    
    if ( val == xOS_setTASK_SUSPENDED )
    {
        if ( xTaskIsTaskSuspended( xOS_CS49844.loaderTaskHandle ) != pdPASS ) /*task is working*/
        {
            vTaskSuspend( xOS_CS49844.loaderTaskHandle );
            TRACE_DEBUG((0, "SUSPEND: cs49844_RowDataLoader"));
        }
    }
    else if ( val == xOS_setTASK_RESUME )
    {
        if ( xTaskIsTaskSuspended( xOS_CS49844.loaderTaskHandle ) == pdPASS ) /*task is not working*/
        {
            vTaskResume( xOS_CS49844.loaderTaskHandle );
            TRACE_DEBUG((0, "RESUME cs49844_RowDataLoader"));
        }
    }
}

static void cs49844_RowDataLoader_CreateTask( void )
{
    
    if ( xTaskCreate( 
            cs49844_RowDataLoader_Task, 
            ( portCHAR * ) "CS49844_LOADER", 
            (STACK_SIZE), 
            NULL, 
            tskCS49844_Loader_PRIORITY, 
            &xOS_CS49844.loaderTaskHandle ) != pdPASS )
    {
        vTaskDelete( xOS_CS49844.loaderTaskHandle );
        xOS_CS49844.xOS_ErrId = xOS_TASK_CREATE_FAIL;
    }
    else
    {
        vTaskSuspend( xOS_CS49844.loaderTaskHandle );
    }

    xOS_CS49844.loaderQueue.xQueue = xQueueCreate( LOADER_QUEUE_LENGTH, FRTOS_SIZE(CS49844_QUEUE_TYPE) );
    xOS_CS49844.loaderQueue.xBlockTime = BLOCK_TIME(0);
    if( xOS_CS49844.loaderQueue.xQueue == NULL )
    {
       xOS_CS49844.xOS_ErrId = xOS_TASK_QUEUE_CREATE_FAIL;
    }
 
}   

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
/********* For Load DSP ULD from External Flash *********/
static unsigned int cs49844_Get_DSP_ULD_StartLocation(CS49844LoadAudioStreamType Stype ,cs49844_ULD_Type Utype)
{
    unsigned int sLocation = 0;
    
    if(Stype == CS49844_LOAD_PCM)
    {
        sLocation = PCM_ULD_LOCATION[Utype][0];
    }
    else if(Stype == CS49844_LOAD_AC3)
    {
        sLocation = AC3_ULD_LOCATION[Utype][0];
    }
    else if(Stype == CS49844_LOAD_DTS)
    {
        sLocation = DTS_ULD_LOCATION[Utype][0];
    }
    return sLocation;
}


static unsigned int cs49844_Get_DSP_ATMOS_ULD_StartLocation(cs49844_ATMOS_ULD_Type Utype)
{
    unsigned int sLocation = 0;

    sLocation = ATMOS_ULD_LOCATION[Utype][0];

    return sLocation;
}

#endif 

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
static unsigned int cs49844_Get_DSP_ULD_Length(CS49844LoadAudioStreamType Stype ,cs49844_ULD_Type Utype)
{
    unsigned int sLocation = 0;
    
    if(Stype == CS49844_LOAD_PCM)
    {
        sLocation = PCM_ULD_LOCATION[Utype][1];
    }
    else if(Stype == CS49844_LOAD_AC3)
    {
        sLocation = AC3_ULD_LOCATION[Utype][1];
    }
    else if(Stype == CS49844_LOAD_DTS)
    {
        sLocation = DTS_ULD_LOCATION[Utype][1];
    }
    return sLocation;
}


static unsigned int cs49844_Get_DSP_ATMOS_ULD_Length(cs49844_ATMOS_ULD_Type Utype)
{
    unsigned int sLocation = 0;
    
    sLocation = ATMOS_ULD_LOCATION[Utype][1];
    return sLocation;
}

#endif 

#if ( configAPP_SPI_FLASH_DSP_ULD == 1 )
static int cs49844_LoadDSP_ULD(unsigned char* buf,CS49844LoadAudioStreamType Stype ,cs49844_ULD_Type Utype )
{
    int i;
    int8 dsp_spi_ret;
    unsigned int sLocation,ULDLength;  
    unsigned int NumOfDSPLoad,NumOfSingle,Addr;
    int8 ret_val = 1;

    sLocation = cs49844_Get_DSP_ULD_StartLocation(Stype,Utype);
    ULDLength = cs49844_Get_DSP_ULD_Length(Stype,Utype);

    Addr = sLocation;
    NumOfDSPLoad = ULDLength/DSPLoadSize;
    NumOfSingle = ULDLength%DSPLoadSize;

    if( NumOfDSPLoad == 0 )
    {
        sFLASH_ReadBuffer(ULDLoaderbuf, Addr, NumOfSingle);
        vTaskDelay(TASK_MSEC2TICKS(1));

        dsp_spi_ret = CS49844SPI_write_ULD_buffer(ULDLoaderbuf, NumOfSingle);
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
            vTaskDelay(TASK_MSEC2TICKS(1));

            dsp_spi_ret = CS49844SPI_write_ULD_buffer(ULDLoaderbuf, DSPLoadSize);
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
            vTaskDelay(TASK_MSEC2TICKS(1));

            dsp_spi_ret = CS49844SPI_write_ULD_buffer(ULDLoaderbuf, NumOfSingle);
            if ( dsp_spi_ret != SCP1_PASS )
            {
                TRACE_DEBUG((0, "Step 3. Write DSP ULD have ERROR !!!\n"));
                ret_val = 0;
            }
        }
    }
    

    if (ret_val != 0)
    {
        if(Stype == CS49844_LOAD_PCM)
        {
            if(Utype == GAIN_ULD)
                TRACE_DEBUG((0, "Load DSP  \"PCM \"  Finished !!!\n"));
            //else if(Utype == OS_ULD)
                //TRACE_DEBUG((0, "Load DSP  \"PCM OS_ULD\"  Finished !!!\n"));
            //else if(Utype == CROSSBAR_ULD)
                //TRACE_DEBUG((0, "Load DSP  \"PCM CROSSBAR_ULD\"  Finished !!!\n"));
        }
        else if(Stype == CS49844_LOAD_AC3)
        {
            if(Utype == GAIN_ULD)
                TRACE_DEBUG((0, "Load DSP  \"AC3 \"  Finished !!!\n"));
            //else if(Utype == OS_ULD)
                //TRACE_DEBUG((0, "Load DSP  \"AC3 OS_ULD\"  Finished !!!\n"));
            //else if(Utype == CROSSBAR_ULD)
                //TRACE_DEBUG((0, "Load DSP  \"AC3 CROSSBAR_ULD\"  Finished !!!\n"));
            //else if(Utype == DECODER_ULD)   
                //TRACE_DEBUG((0, "Load DSP  \"AC3 DECODER_ULD\"  Finished !!!\n"));
        }
        else if(Stype == CS49844_LOAD_DTS)
        {
            if(Utype == GAIN_ULD)
                TRACE_DEBUG((0, "Load DSP  \"DTS \"  Finished !!!\n"));
            //else if(Utype == OS_ULD)
                //TRACE_DEBUG((0, "Load DSP  \"DTS OS_ULD\"  Finished !!!\n"));
            //else if(Utype == CROSSBAR_ULD)
                //TRACE_DEBUG((0, "Load DSP  \"DTS CROSSBAR_ULD\"  Finished !!!\n"));
            //else if(Utype == DECODER_ULD)   
                //TRACE_DEBUG((0, "Load DSP  \"DTS DECODER_ULD\"  Finished !!!\n"));
        }
    }
    
    return ret_val;
}


static int cs49844_LoadDSP_ATMOS_ULD(unsigned char* buf,cs49844_ATMOS_ULD_Type Utype )
{
    int i;
    int8 dsp_spi_ret;
    unsigned int sLocation,ULDLength;  
    unsigned int NumOfDSPLoad,NumOfSingle,Addr;
    int8 ret_val = 1;

    sLocation = cs49844_Get_DSP_ATMOS_ULD_StartLocation(Utype);
    ULDLength = cs49844_Get_DSP_ATMOS_ULD_Length(Utype);

    Addr = sLocation;
    NumOfDSPLoad = ULDLength/DSPLoadSize;
    NumOfSingle = ULDLength%DSPLoadSize;

    if( NumOfDSPLoad == 0 )
    {
        sFLASH_ReadBuffer(ULDLoaderbuf, Addr, NumOfSingle);
        vTaskDelay(TASK_MSEC2TICKS(1));

        dsp_spi_ret = CS49844SPI_write_ULD_buffer(ULDLoaderbuf, NumOfSingle);
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
            vTaskDelay(TASK_MSEC2TICKS(1));

            dsp_spi_ret = CS49844SPI_write_ULD_buffer(ULDLoaderbuf, DSPLoadSize);
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
            vTaskDelay(TASK_MSEC2TICKS(1));

            dsp_spi_ret = CS49844SPI_write_ULD_buffer(ULDLoaderbuf, NumOfSingle);
            if ( dsp_spi_ret != SCP1_PASS )
            {
                TRACE_DEBUG((0, "Step 3. Write DSP ULD have ERROR !!!\n"));
                ret_val = 0;
            }
        }
    }
    

    if (ret_val != 0)
    {
        if(Utype == ATMOS_APP_ULD)
            TRACE_DEBUG((0, "Load DSP  \"APP_ULD \"  Finished !!!\n"));
        else if(Utype == ATMOS_DOLBY_OAR_ULD)
            TRACE_DEBUG((0, "Load DSP  \"DOLBY_OAR_ULD\"  Finished !!!\n"));
        else if(Utype == ATMOS_OS_ULD)
            TRACE_DEBUG((0, "Load DSP  \"OS_ULD\"  Finished !!!\n"));
        else if(Utype == ATMOS_HDMI_ULD)
            TRACE_DEBUG((0, "Load DSP  \"ATMOS_HDMI_ULD\"  Finished !!!\n"));
        else if(Utype == ATMOS_MAT_ULD)
            TRACE_DEBUG((0, "Load DSP  \"ATMOS_MAT_ULD\"  Finished !!!\n"));
    }
    
    return ret_val;
}

#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs49844_Load_DSP_AC3_ULD_InternalFlash( cs49844_ULD_Type module )
{
    int8 ret = 0;

    switch( module )
    {
        case GAIN_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if( CS49844SPI_write_buffer((byte*)&AC3_IN_GAIN_ULD, (sizeof(DTS_IN_GAIN_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }
        
                cs49844_spi_mutex_give();
            }
        }
            break;

        case OS_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if (CS49844SPI_write_buffer((byte*)&OS_HD2_RC94_ULD, (sizeof(OS_HD2_RC94_ULD)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;                   
                }

                cs49844_spi_mutex_give();
            }
        }
            break;

        case CROSSBAR_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if (CS49844SPI_write_buffer((byte*)&AC3_RC23_ULD, (sizeof(AC3_RC23_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }

                cs49844_spi_mutex_give();
            }
        }
            break;

    }

     return ret;
}
#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs49844_Load_DSP_DTS_ULD_InternalFlash( cs49844_ULD_Type module )
{
    int8 ret = 0;

    switch( module )
    {
        case GAIN_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if( CS49844SPI_write_buffer((byte*)&DTS_IN_GAIN_ULD, (sizeof(DTS_IN_GAIN_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }
        
                cs49844_spi_mutex_give();
            }
        }
            break;

        case OS_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if (CS49844SPI_write_buffer((byte*)&OS_HD2_RC94_ULD, (sizeof(OS_HD2_RC94_ULD)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;                   
                }

                cs49844_spi_mutex_give();
            }
        }
            break;

        case CROSSBAR_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if (CS49844SPI_write_buffer((byte*)&DTS_RC25_ULD, (sizeof(DTS_RC25_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }

                cs49844_spi_mutex_give();
            }
        }
            break;
    }

    return ret;

}
#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs49844_Load_DSP_PCM_ULD_InternalFlash( cs49844_ULD_Type module )
{
    int8 ret = 0; 

    switch( module )
    {
        case GAIN_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if( CS49844SPI_write_buffer((byte*)&PCM_IN_GAIN_ULD, (sizeof(PCM_IN_GAIN_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }
        
                cs49844_spi_mutex_give();
            }
        }
            break;

        case OS_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if (CS49844SPI_write_buffer((byte*)&OS_HD2_RC94_ULD, (sizeof(OS_HD2_RC94_ULD)/sizeof(uint8)) ) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;                   
                }

                cs49844_spi_mutex_give();
            }
        }
            break;

        case CROSSBAR_ULD:
        {
            if ( cs49844_spi_mutex_take() == TRUE )
            {
                if (CS49844SPI_write_buffer((byte*)&PCM_TVCSII_ULD, (sizeof(PCM_TVCSII_ULD)/sizeof(uint8))) == SCP1_BSY_TIMEOUT )
                {
                    ret = -1;
                }

                cs49844_spi_mutex_give();
            }
        }
            break;

    }

    return ret;
}
#endif 

#if (configAPP_INTERNAL_DSP_ULD == 1 )
static int8 cs49844_Load_DSP_ULD_InternalFlash( CS49844LoadAudioStreamType type, cs49844_ULD_Type module )
{
    int8 ret = 0;

    switch( type )
    {
        case CS49844_LOAD_PCM:
        {
            ret = cs49844_Load_DSP_PCM_ULD_InternalFlash( module );
        }
            break;

        case CS49844_LOAD_AC3:
        {
             ret = cs49844_Load_DSP_AC3_ULD_InternalFlash( module );
        }
            break;

        case CS49844_LOAD_DTS:
        {
             ret = cs49844_Load_DSP_DTS_ULD_InternalFlash( module );
        }
            break;
    }

    return ret;
}
#endif 

