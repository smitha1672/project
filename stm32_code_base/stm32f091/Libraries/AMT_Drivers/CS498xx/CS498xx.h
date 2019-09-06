#ifndef __CS498xx_H__
#define __CS498xx_H__

#include "Defs.h"
#include "STM32F0xx_board.h"


typedef enum{
    DECODE_PCM = 0,
    DECODE_AC3, 
    DECODE_DTS
}DECODER_TYPE;


#define STATE_BOOT_START       0x00000001
#define STATE_BOOT_SUCCESS     0x00000002
#define STATE_SOFTBOOT_ACK     0x00000005
#define STATE_BOOT_READY       0x00000005
#define CMD_APP_START          0x00000004
#define CMD_SLAVE_BOOT         0x80000000
#define CMD_SOFT_RESET         0x40000000
#define CMD_SOFT_BOOT          0x81000009

#define CS498xx_CMD_BASE 0xf0000000
//! gain_multi_channel_friendly_plus24 (Ls-Rs Gain2) @{
#define RS_G2     ( CS498xx_CMD_BASE + _g_0_Ls_Rs_Gain2151 )
#define LS_G2    ( CS498xx_CMD_BASE + _g_1_Ls_Rs_Gain2151 )

// gain_multi_channel_plus24 (Center Gain 1)
#define CENTER_G1  ( CS498xx_CMD_BASE +  _g_0_Center_Gain_1118 )

// gain_multi_channel_plus24 (Center Gain 2) 
#define CENTER_G2  ( CS498xx_CMD_BASE + _g_0_Center_Gain_2152 )

// gain_multi_channel_friendly_plus24 (Subwoofer Gain 1)
#define SUBWOOFER_G1 ( CS498xx_CMD_BASE + _g_0_Subwoofer_Gain_1120 )  

// gain_multi_channel_friendly_plus24 (Subwoofer Gain 2)
#define SUBWOOFER_G2 ( CS498xx_CMD_BASE + _g_0_Subwoofer_Gain_2132 )

// gain_multi_channel_plus24 (Master Volume)
// g_1 : signed : 5.27 format
//#define MASTER_GAIN1  ( CS498xx_CMD_BASE + _g_0_Gain49 ) 
#define MASTER_GAIN1 0x83000000 
// g_2 : signed : 5.27 format
#define MASTER_GAIN2  ( CS498xx_CMD_BASE + _g_1_Gain49 )
// g_3 : signed : 5.27 format
#define MASTER_GAIN3  ( CS498xx_CMD_BASE + _g_2_Gain49 )


#define CS498xx_READ_BASE 0xf0C00000
#define CS498xx_CMD_READ_PRESENCE_LEFT    ( CS498xx_READ_BASE + _presence_Presence187 )
#define CS498xx_CMD_READ_PRESENCE_RIGHT ( CS498xx_READ_BASE + _presence_Presence186 )    

#define CS498xx_RESPONSE_BASE 0x70C00000
#define PRESENCE_RESPONSE_LEFT ( CS498xx_RESPONSE_BASE + _presence_Presence187 )
#define PRESENCE_RESPONSE_RIGHT    ( CS498xx_RESPONSE_BASE + _presence_Presence186 )

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
#define BASS_LEFT   0xf10001ff
// _bass_level_right : signed : 32.0 format
#define BASS_RIGHT  0xf1000206 

// _treble_level_left : signed : 32.0 format
#define TREBLE_LEFT    0xf10001f1 
// _treble_level_right : signed : 32.0 format
#define TREBLE_RIGHT  0xf10001f8 

#define LR_GAIN1 0xf100020e
#define LR_GAIN2 0xf100020f

#define SUB_GAIN 0xf100021e
#define SUB_GAIN1 0xf100022a

//PEQ
// eq_control : unsigned : 32.0 format
#define PEQ_Enable 0xf100023b //0xd5000000 

// TVHDMC_ENABLE : unsigned : 32.0 format
#define    TVHDMC_ENABLE 0xe0000000

// mpm_b_tshd4 (TSHD4)
// tshd4_enable : unsigned : 32.0 format
 #define SRS_TSHD_Enable 0xb3000000

//delay_proc (Delay Proc.)
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

#define PCM_51_TRUVOL_OFF    1
#define PCM_21_TRUVOL_OFF    0
 
void CS498xx_Mute(AUDIO_CHANNEL channel ,bool value);
void CS498xx_SetVolume(uint8 vol);
void CS498xx_Set_SubVolume(uint8 vol);
#if defined (H4221A)
void CS498xx_SetDialogLevel(uint8 level, AUDIO_FORMAT eAudioFormat);
#endif
void CS498xx_SetCenterVolume(uint8 vol);
void CS498xx_SetRearVolume(uint8 vol);
void CS498xx_Set_Treb( uint8 index );
void CS498xx_Set_Bass( uint8 index );
void CS498xx_Set_Balance(bool vol);


//sean add for DVT output channel reverse issue.
void buffer_write_dsp_OutputChannel_Switch_LsRs(void);
void buffer_write_dsp_OutputChannel_Switch_LR(void);
bool buffer_write_dsp_CS498xx_CrossBar( void );
void buffer_write_dsp_SRS_TSHD_ON(void);
void buffer_write_dsp_SRS_TSHD_OFF(void);

int CS498xx_readPresence(void);
void CS498xx_BASS_MGR_Status(bool vol);
void CS498xx_AQ_Enable(bool vol);
void CS498xx_Set_SubWooferGainSwitch(bool val);
void CS498xx_Set_CenterGainSwitch(bool val);
void CS498xx_Factory_AQBypass(bool val);

//!____________________________________________________________________________________________________________________________@{
// CS495314 public 
bool CS498xx_Initial_uld ( void );
bool softreset( void );
bool CS498xx_PreKickStart(void);
bool CS498xx_KickStart( void );
bool CS498xx_OS (void);
bool CS498xx_softboot( void );
void CS498xx_ProcessMode_Ctrl( bool val );
bool CS498xx_readManualMuteStatus( void );
uint32 CS498xx_SetBalance_PCM( uint8 index );
uint32 CS498xx_SetBalance_AC3( uint8 index );
uint32 CS498xx_SetBalance_DTS( uint8 index );
uint32 CS498xx_Set_Subwoofer_PCM21_Gain( uint8 index );
uint32 CS498xx_Set_Subwoofer_PCM51_Gain( uint8 index );
uint32 CS498xx_Set_Subwoofer_AC3_Gain( uint8 index );
uint32 CS498xx_Set_Subwoofer_DTS_Gain( uint8 index );

void buffer_write_dsp_DataOutput_Normal(void);
void buffer_write_dsp_DataOutput_Woofer(void);
void buffer_write_dsp_DataOutput_L(void);
void buffer_write_dsp_DataOutput_R(void);
bool CS498xx_Initial_PCM_uld( void );
bool CS498xx_Initial_AC3_uld( void );

void CS498xx_SetHDMI(void);
void CS498xx_FormatDecode(uint32 value);
//!@}
#endif /*__CS498xx_H__*/
