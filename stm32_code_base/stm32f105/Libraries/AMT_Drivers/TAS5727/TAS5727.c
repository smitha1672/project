#include "freertos_conf.h"
#include "freertos_task.h"

#include "TAS5727.h"

#if ( configDAP_TAS5727 == 1 )

#define TAS5727_ENTER_SHUTDOWN    TRUE
#define TAS5727_EXIT_SHUTDOWN    FALSE


/*-------------------------- Register defintion ------------------------------*/
//      Register Name                             No of Bytes
#define CLOCK_CTRL_REG                       0x00 // 1
#define DEVICE_ID_REG                        0x01 // 1
#define ERROR_STATUS_REG                     0x02 // 1
#define SYS_CONTROL_1_REG                    0x03 // 1
#define SERIAL_DATA_REG                      0x04 // 1
#define SYS_CONTROL_2_REG                    0x05 // 1
#define SOFT_MUTE_REG                        0x06 // 1

#define MUTE_CH1                             (1<<0) 
#define MUTE_CH2                             (1<<1)
#define MUTE_CH3                             (1<<2)

#define MASTER_VOL_REG                       0x07 // 2
#define CHANNEL_1_VOL_REG                    0x08 // 2
#define CHANNEL_2_VOL_REG                    0x09 // 2
#define CHANNEL_3_VOL_REG                    0x0A // 2
#define VOL_CONFIG_REG                       0x0E // 1
#define MODULATION_LIMIT_REG                 0x10 // 1

#define IC_DEALY_CH_1_REG                    0x11 // 1
#define IC_DEALY_CH_2_REG                    0x12 // 1
#define IC_DEALY_CH_3_REG                    0x13 // 1
#define IC_DEALY_CH_4_REG                    0x14 // 1

#define START_STOP_PERIOD_REG                0x1A // 1
#define OSC_TRIM_REG                         0x1B // 1
#define BKND_ERR_REG                         0x1C // 1

#define INPUT_MUX_REG                        0x20 // 4
#define CH_4_SOURCE_SELECT_REG               0x20 // 4
#define PWM_MUX_REG                          0x25 // 4

#define CH_1_BQ_0                            0x26 // 20
#define CH_1_BQ_1                            0x27 // 20
#define CH_1_BQ_2                            0x28 // 20
#define CH_1_BQ_3                            0x29 // 20
#define CH_1_BQ_4                            0x2A // 20
#define CH_1_BQ_5                            0x2B // 20
#define CH_1_BQ_6                            0x2C // 20
#define CH_1_BQ_7                            0x2D // 20
#define CH_1_BQ_8                            0x2E // 20
#define CH_1_BQ_9                            0x2F // 20

#define CH_2_BQ_0                            0x30 // 20
#define CH_2_BQ_1                            0x31 // 20
#define CH_2_BQ_2                            0x32 // 20
#define CH_2_BQ_3                            0x33 // 20
#define CH_2_BQ_4                            0x34 // 20
#define CH_2_BQ_5                            0x35 // 20
#define CH_2_BQ_6                            0x36 // 20
#define CH_2_BQ_7                            0x37 // 20
#define CH_2_BQ_8                            0x38 // 20
#define CH_2_BQ_9                            0x39 // 20

#define DRC_1_SOFTENING_FILTER_AL_OM_REG     0x3B // 8
#define DRC_1_ATT_REL_RATE_REG               0x3C // 8

#define DRC_2_SOFTENING_FILTER_AL_OM_REG     0x3E // 8
#define DRC_2_ATT_REL_RATE_REG               0x3F // 8

#define DRC_1_ATT_REL_THRESHOLD_REG          0x40 // 4
#define DRC_2_ATT_DEC_THRESHOLD_REG          0x43 // 4

#define DRC_CTRL_REG                         0x46 // 4
#define PWM_SWITCH_RATE_REG                  0x4F // 4
#define BANK_SWITCH_CTRL_REG                 0x50 // 4
#define CH_1_OUTPUT_MIXER_REG                0x51 // 8
#define CH_2_OUTPUT_MIXER_REG                0x52 // 8

#define OUTPUT_POST_SCALE                    0x56 // 4
#define OUTPUT_PRE_SCALE                     0x57 // 4

#define CH_1_BQ_10_REG                       0x58 // 20
#define CH_1_BQ_11_REG                       0x59 // 20
#define CH_4_BQ_0_REG                        0x5A // 20
#define CH_4_BQ_1_REG                        0x5B // 20
#define CH_2_BQ_10_REG                       0x5C // 20
#define CH_2_BQ_11_REG                       0x5D // 20
#define CH_3_BQ_0_REG                        0x5E // 20
#define CH_3_BQ_1_REG                        0x5F // 20

#define IDF_POST_SCALE_REG                   0x62 // 4
#define LEFT_CHANNEL_PWM_LEVEL_REG           0x6B // 4
#define RIGHT_CHANNEL_PWM_LEVEL_REG          0x6C // 4

#define CH_1_INLINE_MIXER_REG                0x70 // 4
#define INLINE_DRC_EN_MIXER_CH_1_REG         0x71 // 4
#define CH_1_RIGHT_CHANNEL_MIXER_REG         0x72 // 4
#define CH_1_LEFT_CHANNEL_MIXER_REG          0x73 // 4

#define CH_2_INLINE_MIXER_REG                0x74 // 4
#define INLINE_DRC_EN_MIXER_CH_2_REG         0x75 // 4
#define CH_2_LEFT_CHANNEL_MIXER_REG          0x76 // 4
#define CH_2_RIGHT_CHANNEL_MIXER_REG         0x77 // 4

#define UPDATE_DEV_ADD_KEY                   0xF8 // 4
#define UPDATE_DEV_ADD_REG                   0xF9 // 4

#define MASTER_VOLUME_MUTE 0xFF
/*----------------------------------------------------------------------------*/
static byte m_CHANNEL_1_VOL_REG[]={0x00, 0xC0}; 
static byte m_CHANNEL_2_VOL_REG[]={0x00, 0xC0};
static byte m_CHANNEL_3_VOL_REG[]={0x00, 0xC0};


static byte m_inputMux_Data[]={0x00, 0x89, 0x77, 0x72}; 
static byte m_PwmMux_Data[]={0x01, 0x02, 0x13, 0x45}; 

static byte m_Master_Vol[]={0x00 ,0x90};
static byte m_Master_Vol_mute[]={0x03 ,0xff};

/*BD mode or Ternary modulation*/
#ifdef TAS5727_TERNARY
#define TAS5727_ICD1 0x80
#define TAS5727_ICD2 0x7C
#define TAS5727_ICD3 0x80
#define TAS5727_ICD4 0x7C
#define TAS5727_ENTERSHUTDOWN 0x48
#define TAS5727_EXITSHUTDOWN 0x08

#else /*BD mode*/
#define TAS5727_ICD1 0xB8
#define TAS5727_ICD2 0x60
#define TAS5727_ICD3 0xA0
#define TAS5727_ICD4 0x48
#define TAS5727_ENTERSHUTDOWN 0x40
#define TAS5727_EXITSHUTDOWN 0x00


#endif

//____________________________________________________________________________
static void TAS5727_initialization_LR( void );

static void TAS5727_Master_Vol_Mute_LR( bool enable );

//____________________________________________________________________________
DAP_LR_CTRL_OBJECT DAP_LR_ObjCtrl = 
{
    TAS5727_initialization_LR,
    TAS5727_Master_Vol_Mute_LR
};
const DAP_LR_CTRL_OBJECT *pDAP_LR_ObjCtrl = &DAP_LR_ObjCtrl;



void TAS5727_LR_DRC_enable( bool bEnable )
{
    byte Drc_ctrl[4] = {0x00, 0x00, 0x00, 0x01};

    if ( bEnable == TRUE )
    {
        Drc_ctrl[3] = 0x01;
    }
    else
    {
        Drc_ctrl[3] = 0x00;
    }

    //DRC
    TAS5727_I2C_write_n_byte( DRC_CTRL_REG, Drc_ctrl, 4);
}

static void TAS5727_Master_Vol_Mute_LR( bool enable )
{
    if ( enable == TRUE )
    {
        TAS5727_I2C_write( SOFT_MUTE_REG, 0x07);
        TAS5727_I2C_write_n_byte( MASTER_VOL_REG,m_Master_Vol_mute, 2); /*smith fixes*/
    }
    else
    {
        TAS5727_I2C_write_n_byte( MASTER_VOL_REG,m_Master_Vol,2);
        TAS5727_I2C_write( SOFT_MUTE_REG, 0x00);
    }
}

void TAS5727_shutdown( bool value )
{
    if ( value == TAS5727_ENTER_SHUTDOWN)
    {
        TAS5727_I2C_write( SYS_CONTROL_2_REG, TAS5727_ENTERSHUTDOWN); /*Enter all-channel shutdown (hard mute)*/
        vTaskDelay(TASK_MSEC2TICKS(165));
    }
    else
    {
        TAS5727_I2C_write( SYS_CONTROL_2_REG, TAS5727_EXITSHUTDOWN); /*Exit all-channel shutdown (hard mute)*/
        vTaskDelay(TASK_MSEC2TICKS(165));
    }
}

static void TAS5727_initialization_LR( void )
{
    /*3. Trim oscillator (write 0x00 to register 0x1B) and wait at least 50 ms.*/
    TAS5727_I2C_write( OSC_TRIM_REG, 0x00);
    vTaskDelay(TASK_MSEC2TICKS(50));

    //Input Mux Register(BD mode,opt.)
    
    TAS5727_I2C_write_n_byte( PWM_MUX_REG,m_PwmMux_Data,4);    
    
    TAS5727_I2C_write_n_byte( MASTER_VOL_REG,m_Master_Vol_mute,2);
    TAS5727_I2C_write_n_byte( CHANNEL_1_VOL_REG, m_CHANNEL_1_VOL_REG, 2); //0dB
    TAS5727_I2C_write_n_byte( CHANNEL_2_VOL_REG, m_CHANNEL_2_VOL_REG, 2); //0dB
    TAS5727_I2C_write_n_byte( CHANNEL_3_VOL_REG, m_CHANNEL_3_VOL_REG, 2); //0dB
    
    TAS5727_I2C_write( SOFT_MUTE_REG, 0x07);
    TAS5727_I2C_write( SYS_CONTROL_1_REG, 0x80);
    TAS5727_I2C_write( SERIAL_DATA_REG, 0x05);
    TAS5727_I2C_write( SYS_CONTROL_2_REG, TAS5727_ENTERSHUTDOWN);

    TAS5727_I2C_write( VOL_CONFIG_REG, 0x91);
    TAS5727_I2C_write( MODULATION_LIMIT_REG, 0x02);
    TAS5727_I2C_write( START_STOP_PERIOD_REG, 0x0a);
    TAS5727_I2C_write( BKND_ERR_REG, 0x02);
    TAS5727_I2C_write( IC_DEALY_CH_1_REG, TAS5727_ICD1);
    TAS5727_I2C_write( IC_DEALY_CH_2_REG, TAS5727_ICD2);
    TAS5727_I2C_write( IC_DEALY_CH_3_REG, TAS5727_ICD3);
    TAS5727_I2C_write( IC_DEALY_CH_4_REG, TAS5727_ICD4);
    TAS5727_I2C_write_n_byte( INPUT_MUX_REG,m_inputMux_Data,4); 
    
    TAS5727_shutdown( TAS5727_EXIT_SHUTDOWN);

}

#endif 
