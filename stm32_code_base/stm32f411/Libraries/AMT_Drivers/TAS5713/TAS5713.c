#include "freertos_conf.h"
#include "freertos_task.h"

#include "TAS5713.h"

#if ( configDAP_TAS5713 == 1 )

#define TAS5713_ENTER_SHUTDOWN    TRUE
#define TAS5713_EXIT_SHUTDOWN    FALSE

/*-------------------------- Register defintion ------------------------------*/
#define CLOCK_CTRL_REG                      0x00
#define SYS_CONTROL_1_REG                   0x03
#define SYS_CONTROL_2_REG                   0x05
#define SOFT_MUTE_REG                       0x06
#define MUTE_CH1                            (1<<0)
#define MUTE_CH2                            (1<<1)
#define MUTE_CH3                            (1<<2)


#define MASTER_VOL_REG                      0x07
#define CHANNEL_1_VOL_REG                   0x08
#define CHANNEL_2_VOL_REG                   0x09
#define CHANNEL_3_VOL_REG                   0x0A
#define VOL_CONFIG_REG                      0x0E
#define IC_DEALY_CH_1_REG                   0x11
#define IC_DEALY_CH_2_REG                   0x12
#define IC_DEALY_CH_3_REG                   0x13
#define IC_DEALY_CH_4_REG                   0x14
#define PWM_SHUTDOWN_REG                    0x19
#define OSC_TRIM_REG                        0x1B
#define BKND_ERR_REG                        0x1C
#define INPUT_MUX_REG                       0x20
#define PWM_MUX_REG                         0x25
#define CH_1_BQ_0                           0x29
#define CH_1_BQ_1                           0x2A
#define CH_1_BQ_2                           0x2B
#define CH_1_BQ_3                           0x2C
#define CH_1_BQ_4                           0x2D
#define CH_1_BQ_5                           0x2E
#define CH_1_BQ_6                           0x2F
#define CH_2_BQ_0                           0x30
#define CH_2_BQ_1                           0x31
#define CH_2_BQ_2                           0x32
#define CH_2_BQ_3                           0x33
#define CH_2_BQ_4                           0x34
#define CH_2_BQ_5                           0x35
#define CH_2_BQ_6                           0x36
#define DRC_1_SOFTENING_FILTER_AE_OM_REG    0x3B
#define DRC_1_ATT_REL_RATE_REG              0x3C
#define DRC_2_SOFTENING_FILTER_AL_OM_REG    0x3E
#define DRC_2_ATT_REL_RATE_REG              0x3F
#define DRC_1_ATT_REL_THRESHOLD_REG         0x40
#define DRC_2_ATT_DEC_THRESHOLD_REG         0x43     
#define DRC_CTRL_REG                        0x46
#define BANK_SWITCH_CTRL_REG                0x50
#define CH_1_OUTPUT_MIXER_REG               0x51
#define CH_2_OUTPUT_MIXER_REG               0x52
#define CH_1_BQ_7_REG                       0x58
#define CH_1_BQ_8_REG                       0x59
#define CH_4_BQ_0_REG                       0x5A
#define CH_4_BQ_1_REG                       0x5B
#define OUTPUT_POST_SCALE_REG               0x56
#define OUTPUT_PRE_SCALE_REG                0x57
#define CH_1_INLINE_MIXER_REG               0x70
#define INLINE_DRC_EN_MIXER_CH_1_REG        0x71
#define CH_1_RIGHT_CHANNEL_MIXER_REG        0x72
#define CH_1_LEFT_CHANNEL_MIXER_REG         0x73
#define CH_2_INLINE_MIXER_REG               0x74
#define INLINE_DRC_EN_MIXER_CH_2_REG        0x75
#define CH_2_LEFT_CHANNEL_MIXER_REG         0x76
#define CH_2_RIGHT_CHANNEL_MIXER_REG        0x77 


#define MASTER_VOLUME_MUTE 0xFF
/*----------------------------------------------------------------------------*/
static  byte m_PwmMux2[]={0x01, 0x10, 0x32, 0x45}; 

static  byte m_master_vol =0x14;  //c 14db
static  byte m_Drc_ctrl[] = {0x00, 0x00, 0x00, 0x01};
//L/R DRC
static  byte m_Drc1_att_rel_threshold_LR[] = {0x06, 0xA3, 0x00, 0x00, 0x06, 0xA2, 0xFF, 0xFF};
static  byte m_Drc1_sofrening_filter_ae_om_LR[] = {0x00, 0x00, 0x18, 0xF4, 0x00, 0x7F, 0xE7, 0x0C};
static  byte m_Drc1_att_rel_rate_LR[] = {0x00, 0x04, 0x3c, 0xfd, 0xFF, 0xfd, 0x01, 0x3a};
//center DRC
static  byte m_Drc1_att_rel_threshold_C[] = {0x06, 0xA3, 0xD0, 0x00, 0x06, 0xA2, 0xFF, 0xFF};
static  byte m_Drc1_sofrening_filter_ae_om_C[] = {0x00, 0x00, 0x18, 0xF4, 0x00, 0x7F, 0xE7, 0x0C};
static  byte m_Drc1_att_rel_rate_C[] = {0x00, 0x04, 0x3C, 0xFD, 0xFF, 0xFD, 0x01, 0x3A};


/*BD mode or Ternary modulation*/
#ifdef TAS5713_TERNARY
#define TAS5713_ICD1 0x80
#define TAS5713_ICD2 0x7C
#define TAS5713_ICD3 0x80
#define TAS5713_ICD4 0x7C
#define TAS5713_ENTERSHUTDOWN 0x48
#define TAS5713_EXITSHUTDOWN 0x08

#else /*BD mode*/
#define TAS5713_ICD1 0xB8
#define TAS5713_ICD2 0x60
#define TAS5713_ICD3 0xA0
#define TAS5713_ICD4 0x48
#define TAS5713_ENTERSHUTDOWN 0x40
#define TAS5713_EXITSHUTDOWN 0x00


#endif

//____________________________________________________________________________
static void TAS5713_initialization_C( bool Address );

static void TAS5713_Master_Vol_Mute_C(bool Address,bool enable );

//____________________________________________________________________________
DAP_C_CTRL_OBJECT DAP_C_ObjCtrl = 
{
    TAS5713_initialization_C,
    TAS5713_Master_Vol_Mute_C
};
const DAP_C_CTRL_OBJECT *pDAP_C_ObjCtrl = &DAP_C_ObjCtrl;

void TAS5713_C_DRC_enable(bool Address,bool bEnable )
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
    TAS5713_I2C_write_n_byte(Address,DRC_CTRL_REG, Drc_ctrl, 4);
}

static void TAS5713_Master_Vol_Mute_C(bool Address,bool enable )
{
    if ( enable == TRUE )
    {
        TAS5713_I2C_write(Address, SOFT_MUTE_REG, 0x07);
        TAS5713_I2C_write(Address, MASTER_VOL_REG, MASTER_VOLUME_MUTE);
    }
    else
    {
        TAS5713_I2C_write(Address, MASTER_VOL_REG, m_master_vol);
        TAS5713_I2C_write(Address, SOFT_MUTE_REG, 0x00);
    }
}

void TAS5713_shutdown(bool Address,bool value )
{
    if ( value == TAS5713_ENTER_SHUTDOWN)
    {
        TAS5713_I2C_write(Address, SYS_CONTROL_2_REG, 0x40); /*Enter all-channel shutdown (hard mute)*/
        vTaskDelay(TASK_MSEC2TICKS(165)); /*Wait at least 1 ms + 1.3 กั tstop (where tstop is specified by register 0x1A).*/
    }
    else
    {
        TAS5713_I2C_write(Address, SYS_CONTROL_2_REG, 0x02); /*Exit all-channel shutdown (hard mute)*/
        vTaskDelay(TASK_MSEC2TICKS(165)); /*Wait at least 1 ms + 1.3 กั tstart (where tstop is specified by register 0x1A).*/
    }
}

static void TAS5713_initialization_C( bool Address )
{
    //TAS5713_I2C_address(value);    /*Smith Mark*/
    vTaskDelay(TASK_MSEC2TICKS(50));
    
    /*3. Trim oscillator (write 0x00 to register 0x1B) and wait at least 50 ms.*/
    TAS5713_I2C_write(Address, OSC_TRIM_REG, 0x00);
    vTaskDelay(TASK_MSEC2TICKS(50));
	TAS5713_I2C_write_n_byte(Address, PWM_MUX_REG,m_PwmMux2,4);
	TAS5713_I2C_write(Address, PWM_SHUTDOWN_REG,0x3A);
        
    TAS5713_I2C_write(Address, CHANNEL_1_VOL_REG, 0x30);
    TAS5713_I2C_write(Address, CHANNEL_2_VOL_REG, 0x30);
    TAS5713_I2C_write(Address, CHANNEL_3_VOL_REG, 0x30);
    TAS5713_I2C_write(Address, MASTER_VOL_REG, 0xff);

    TAS5713_I2C_write(Address, INPUT_MUX_REG, 0x88);
    TAS5713_I2C_write(Address, IC_DEALY_CH_1_REG, 0xB8);
    TAS5713_I2C_write(Address, IC_DEALY_CH_2_REG, 0x60);
    TAS5713_I2C_write(Address, IC_DEALY_CH_3_REG, 0xA0);
    TAS5713_I2C_write(Address, IC_DEALY_CH_4_REG, 0x48);
    /*Exit shutdown*/
    TAS5713_shutdown(Address, TAS5713_EXIT_SHUTDOWN);
    //DRC
    TAS5713_I2C_write_n_byte(Address, DRC_CTRL_REG,m_Drc_ctrl,4);
    TAS5713_I2C_write_n_byte(Address, DRC_1_ATT_REL_THRESHOLD_REG, m_Drc1_att_rel_threshold_C,8);
    TAS5713_I2C_write_n_byte(Address, DRC_1_SOFTENING_FILTER_AE_OM_REG, m_Drc1_sofrening_filter_ae_om_C,8);
    TAS5713_I2C_write_n_byte(Address, DRC_1_ATT_REL_RATE_REG, m_Drc1_att_rel_rate_C,8);

}

#endif 
