#include "freertos_conf.h"
#include "freertos_task.h"

#include "TAS5711.h"

#if ( configDAP_TAS5711 == 1 )

#define TAS5711_ENTER_SHUTDOWN	TRUE
#define TAS5711_EXIT_SHUTDOWN	FALSE

//_________________________________________________________________________________________________________

#define CLOCK_CTRL_REG		0x00
#define SYS_CONTROL_1_REG 	0x03
#define SERIAL_DATA_REG       0x04
#define SYS_CONTROL_2_REG 	0x05
#define SOFT_MUTE_REG  		0x06
#define MUTE_CH1    			(1<<0)
#define MUTE_CH2   			(1<<1)
#define MUTE_CH3   			(1<<2)
#define MASTER_VOL_REG  	0x07
#define CHANNEL_1_VOL_REG 	0x08
#define CHANNEL_2_VOL_REG 	0x09
#define CHANNEL_3_VOL_REG 	0x0A
#define VOL_CONFIG_REG  	0x0E
#define MODULATION_LIMIT_REG  	0x10
#define IC_DEALY_CH_1_REG 	0x11
#define IC_DEALY_CH_2_REG 	0x12
#define IC_DEALY_CH_3_REG 	0x13
#define IC_DEALY_CH_4_REG 	0x14
#define PWM_SHUTDOWN_REG 0x19
#define START_STOP_PERIOD_REG 	0x1A
#define OSC_TRIM_REG  		0x1B
#define BKND_ERR_REG  		0x1C
#define INPUT_MUX_REG  		0x20
#define PWM_MUX_REG  		0x25
#define CH_1_BQ_0			0x29
#define CH_1_BQ_1			0x2A
#define CH_1_BQ_2			0x2B
#define CH_1_BQ_3			0x2C
#define CH_1_BQ_4			0x2D
#define CH_1_BQ_5			0x2E
#define CH_1_BQ_6			0x2F
#define CH_2_BQ_0			0x30
#define CH_2_BQ_1			0x31
#define CH_2_BQ_2			0x32
#define CH_2_BQ_3			0x33
#define CH_2_BQ_4			0x34
#define CH_2_BQ_5			0x35
#define CH_2_BQ_6			0x36
#define DRC_1_SOFTENING_FILTER_AE_OM_REG	0x3A
#define DRC_1_SOFTENING_FILTER_AA_OM_REG	0x3B
#define DRC_1_SOFTENING_FILTER_AC_OM_REG	0x3C
#define DRC_2_SOFTENING_FILTER_AE_OM_REG	0x3D
#define DRC_2_SOFTENING_FILTER_AA_OM_REG	0x3E
#define DRC_2_SOFTENING_FILTER_AC_OM_REG	0x3F

//#define DRC_1_ATT_REL_THRESHOLD_REG	0x40
#define DRC_1_T	0x40
#define DRC_1_K	0x41
#define DRC_1_O	0x42
#define DRC_2_ATT_DEC_THRESHOLD_REG	0x43	 
#define DRC_CTRL_REG		0x46
#define BANK_SWITCH_CTRL_REG	0x50
#define CH_1_OUTPUT_MIXER_REG	0x51
#define CH_2_OUTPUT_MIXER_REG	0x52
#define CH_1_BQ_7_REG		0x58
#define CH_1_BQ_8_REG		0x59
#define CH_4_BQ_0_REG	0x5A
#define CH_4_BQ_1_REG	0x5B
#define OUTPUT_POST_SCALE_REG	0x56
#define OUTPUT_PRE_SCALE_REG	0x57
#define CH_1_INLINE_MIXER_REG	0x70
#define INLINE_DRC_EN_MIXER_CH_1_REG	0x71
#define CH_1_RIGHT_CHANNEL_MIXER_REG	0x72
#define CH_1_LEFT_CHANNEL_MIXER_REG		0x73
#define CH_2_INLINE_MIXER_REG	0x74
#define INLINE_DRC_EN_MIXER_CH_2_REG	0x75
#define CH_2_LEFT_CHANNEL_MIXER_REG		0x76
#define CH_2_RIGHT_CHANNEL_MIXER_REG	0x77 

#define MASTER_VOLUME_MUTE 0xFF

/*---------------------------------------------------------------------------------------------------------*/
static  byte m_PwmMux[]={0x01, 0x32, 0x10, 0x45};
static  byte m_InputMux[]={0x01, 0x89, 0x77, 0x72}; 

static  byte m_master_vol =0x16;  
static  byte m_Drc_ctrl[] = {0x00, 0x00, 0x00, 0x00};
//L/R DRC
static  byte m_Drc1_sofrening_filter_ae_om_LR[] = {0x00, 0x00, 0x94, 0x43, 0x00, 0x7F, 0x6B, 0xB9};
static  byte m_Drc1_sofrening_filter_aa_om_LR[] = {0x00, 0x00, 0x4A, 0x37, 0x00, 0x7F, 0xB5, 0xC5};
static  byte m_Drc1_sofrening_filter_ac_om_LR[] = {0x00, 0x00, 0x4A, 0x37, 0x00, 0x7F, 0xB5, 0xC5};

static  byte m_Drc2_sofrening_filter_ae_om_LR[] = {0x00, 0x00, 0x94, 0x43, 0x00, 0x7F, 0x6B, 0xB9};
static  byte m_Drc2_sofrening_filter_aa_om_LR[] = {0x00, 0x00, 0x4A, 0x37, 0x00, 0x7F, 0xB5, 0xC5};
static  byte m_Drc2_sofrening_filter_ac_om_LR[] = {0x00, 0x00, 0x4A, 0x37, 0x00, 0x7F, 0xB5, 0xC5};

static  byte m_Drc1_T[] = {0xFD, 0x2D, 0x26, 0x01};
static  byte m_Drc1_K[] = {0x03, 0x80, 0x97, 0x00};
static  byte m_Drc1_O[] = {0x00, 0x0D, 0x16, 0x90};

//__________________________________________________________________________________________________________
static void TAS5711_initialization_C( void );

static void TAS5711_Master_Vol_Mute_C( bool enable );
//__________________________________________________________________________________________________________

DAP_C_CTRL_OBJECT DAP_C_ObjCtrl = 
{
    TAS5711_initialization_C,
    TAS5711_Master_Vol_Mute_C
};
const DAP_C_CTRL_OBJECT *pDAP_C_ObjCtrl = &DAP_C_ObjCtrl;

/*---------------------------------------------------------------------------------------------------------*/
void TAS5711_Center_DRC_enable( bool enable )
{
	byte Drc_ctrl[4] = {0x00, 0x00, 0x00, 0x01};

	if ( enable == TRUE )
	{
		Drc_ctrl[3] = 0x01;
	}
	else
	{
		Drc_ctrl[3] = 0x00;
	}

	//DRC 
	TAS5711_I2C_write_n_byte( DRC_CTRL_REG, Drc_ctrl,4);
}

static void TAS5711_Master_Vol_Mute_C(   bool enable )
{

	if ( enable == TRUE )
	{
		TAS5711_I2C_write( SOFT_MUTE_REG, 0x07);
		TAS5711_I2C_write( MASTER_VOL_REG, MASTER_VOLUME_MUTE);
	}
	else
	{
		TAS5711_I2C_write( MASTER_VOL_REG, m_master_vol);
		TAS5711_I2C_write( SOFT_MUTE_REG, 0x00);
	}
}

void TAS5711_shutdown(  bool value )
{
	if ( value == TAS5711_ENTER_SHUTDOWN)
	{
		TAS5711_I2C_write( SYS_CONTROL_2_REG, 0x40); /*Enter all-channel shutdown (hard mute)*/
		vTaskDelay(TASK_MSEC2TICKS(165)); /*Wait at least 1 ms + 1.3 กั tstop (where tstop is specified by register 0x1A).*/
	}
	else
	{
		TAS5711_I2C_write( SYS_CONTROL_2_REG, 0x02); /*Exit all-channel shutdown (hard mute)*/
		vTaskDelay(TASK_MSEC2TICKS(165)); /*Wait at least 1 ms + 1.3 กั tstart (where tstop is specified by register 0x1A).*/
	}
}

static void TAS5711_initialization_C( void )
{

	/*3. Trim oscillator (write 0x00 to register 0x1B) and wait at least 50 ms.*/
	TAS5711_I2C_write( OSC_TRIM_REG, 0x00);
	vTaskDelay(TASK_MSEC2TICKS(50));

	TAS5711_I2C_write_n_byte( PWM_MUX_REG,m_PwmMux,4);
	TAS5711_I2C_write( PWM_SHUTDOWN_REG,0x3A);
	
	TAS5711_I2C_write( SOFT_MUTE_REG, 0x07);
	TAS5711_I2C_write( CHANNEL_1_VOL_REG, 0x30);
	TAS5711_I2C_write( CHANNEL_2_VOL_REG, 0x30);
	TAS5711_I2C_write( CHANNEL_3_VOL_REG, 0x30);
	TAS5711_I2C_write( MASTER_VOL_REG, 0xff);

	TAS5711_I2C_write_n_byte( INPUT_MUX_REG,m_InputMux, 4);
	TAS5711_I2C_write( IC_DEALY_CH_1_REG, 0xB8);
	TAS5711_I2C_write( IC_DEALY_CH_2_REG, 0x60);
	TAS5711_I2C_write( IC_DEALY_CH_3_REG, 0xA0);
	TAS5711_I2C_write( IC_DEALY_CH_4_REG, 0x48);
	/*Exit shutdown*/
	TAS5711_shutdown( TAS5711_EXIT_SHUTDOWN);

	//DRC 
	TAS5711_I2C_write_n_byte( DRC_CTRL_REG,m_Drc_ctrl,4);
	
	TAS5711_I2C_write_n_byte( DRC_1_SOFTENING_FILTER_AE_OM_REG, m_Drc1_sofrening_filter_ae_om_LR,8);
	TAS5711_I2C_write_n_byte( DRC_1_SOFTENING_FILTER_AA_OM_REG, m_Drc1_sofrening_filter_aa_om_LR,8);
	TAS5711_I2C_write_n_byte( DRC_1_SOFTENING_FILTER_AC_OM_REG, m_Drc1_sofrening_filter_ac_om_LR,8);

	TAS5711_I2C_write_n_byte( DRC_2_SOFTENING_FILTER_AE_OM_REG, m_Drc2_sofrening_filter_ae_om_LR,8);
	TAS5711_I2C_write_n_byte( DRC_2_SOFTENING_FILTER_AA_OM_REG, m_Drc2_sofrening_filter_aa_om_LR,8);
	TAS5711_I2C_write_n_byte( DRC_2_SOFTENING_FILTER_AC_OM_REG, m_Drc2_sofrening_filter_ac_om_LR,8);
	
	TAS5711_I2C_write_n_byte( DRC_1_T, m_Drc1_T,8);
	TAS5711_I2C_write_n_byte( DRC_1_K, m_Drc1_K,8);
	TAS5711_I2C_write_n_byte( DRC_1_O, m_Drc1_O,8);

}

#endif 

