#ifndef __IR_KEY_CODE_H__
#define __IR_KEY_CODE_H__

#include "api_typedef.h"

#define XRS351_IR_REMOTE

#define XRS351_ADDRESS 0x00FF
#define VIZ_FAC_RMT_ADDRESS   0x04FB

#define IR_KEY_POWER            0x40
#define IR_KEY_VOL_UP           0x41
#define IR_KEY_VOL_DN           0x45
#define IR_KEY_SET_MUTE     0x48
#define IR_KEY_MEDIA_PREVIOUS           0x8A
#define IR_KEY_MEDIA_NEXT                   0x8B
#define IR_KEY_MEDIA_PLAY_PAUSE       0x8E
#define IR_KEY_SEL_SRC          0x44
#define IR_KEY_SRC_AUX1        0xB1
#define IR_KEY_SRC_AUX2        0xB2
#define IR_KEY_SRC_COX          0xC9
#define IR_KEY_SRC_OPT          0xC8
#define IR_KEY_SRC_USB          0xA1
#define IR_KEY_SRC_BT            0x22

#define IR_KEY_SET_CCK          0xAC
#define IR_KEY_BT_PAIR      0x6F
#define IR_KEY_DEVICE_RESET        0x63
#define IR_KEY_SET_BASS_DN     0x6A
#define IR_KEY_SET_BASS_UP     0x6B
#define IR_KEY_SET_TREBLE_DN     0x68
#define IR_KEY_SET_TREBLE_UP     0x69
#define IR_KEY_SET_CENTER_DN     0x6E
#define IR_KEY_SET_CENTER_UP     0x70
#define IR_KEY_SET_SURROUND_DN     0x6C
#define IR_KEY_SET_SURROUND_UP     0x6D
#define IR_KEY_SET_SURRBAL_DN     0xB4
#define IR_KEY_SET_SURRBAL_UP     0xB3
#define IR_KEY_SET_SUBWOOFER_DN     0x51
#define IR_KEY_SET_SUBWOOFER_UP     0x4D
#define IR_KEY_SET_SURND_OFF     0x24
#define IR_KEY_SET_SURND_ON     0x23
#define IR_KEY_SET_TRUVOL_OFF     0x42
#define IR_KEY_SET_TRUVOL_ON     0x28
#define IR_KEY_SET_NIGHT_OFF     0x62
#define IR_KEY_SET_NIGHT_ON     0x61
#define IR_KEY_SET_AV_DELAY_DN     0x71
#define IR_KEY_SET_AV_DELAY_UP     0xD6
#define IR_KEY_SET_VIZRMT_OFF     0x1F
#define IR_KEY_SET_VIZRMT_ON     0x1E
#define IR_KEY_LEARN_VOL_DN     0x09
#define IR_KEY_LEARN_VOL_UP     0x77
#define IR_KEY_LEARN_MUTE         0x72


#define VIZ_FAC_IR_KEY_VOL_UP         0x02
#define VIZ_FAC_IR_KEY_VOL_DN         0x03
#define VIZ_FAC_IR_KEY_VOL_MUTE     0x09

#if defined ( XRS351_IR_REMOTE )
static const xIR_CMD_KEY_MAP IR_CMD_KEY_MAP[] = 
{
    {0x40, USER_EVENT_POWER},
    {0x41, USER_EVENT_VOL_UP},
    {0x45, USER_EVENT_VOL_DN},
    {0x8A, USER_EVENT_PREVIOUS},
    {0x8B, USER_EVENT_NEXT},
    {0x8E, USER_EVENT_PLAY_PAUSE},
    {0x44, USER_EVENT_START_AUTO_SEL_SRC},	/*Change by Elvis for VIZIO new remote behavior v1.3    {0x44, USER_EVENT_SEL_SRC},*/
    {0xB1, USER_EVENT_SEL_AUX1},
    {0xB2, USER_EVENT_SEL_AUX2},
    {0xC9, USER_EVENT_SEL_COAXIAL},
    {0xC8, USER_EVENT_SEL_OPTICAL},
    {0xA1, USER_EVENT_SEL_USB_PLAYBACK,},
    {0x22, USER_EVENT_SEL_BT},
    {0xAC, USER_EVENT_CCK},
    {0x48, USER_EVENT_MUTE},
    {0x6F, USER_EVENT_BT_PAIR},
    {0x63, USER_EVENT_DEVICE_RESET,},
    {0x6A, USER_EVENT_BASS_DN,},
    {0x6B, USER_EVENT_BASS_UP},
    {0x68, USER_EVENT_TREBLE_DN,},
    {0x69, USER_EVENT_TREBLE_UP},
    {0x6E, USER_EVENT_CENTER_DN,},
    {0x70, USER_EVENT_CENTER_UP,},
    {0x6C, USER_EVENT_REAR_DN},
    {0x6D, USER_EVENT_REAR_UP},
    {0xB4, USER_EVENT_BALANCE_DN},
    {0xB3, USER_EVENT_BALANCE_UP},
    {0x51, USER_EVENT_SUB_DN},
    {0x4D, USER_EVENT_SUB_UP},    
    {0x24, USER_EVENT_SRS_TSHD_OFF},
    {0x23, USER_EVENT_SRS_TSHD_ON},
    {0x42, USER_EVENT_SRS_TRUVOL_OFF},
    {0x28, USER_EVENT_SRS_TRUVOL_ON},
    {0x62, USER_EVENT_NIGHT_MODE_OFF},
    {0x61, USER_EVENT_NIGHT_MODE_ON},
    {0x71, USER_EVENT_AV_DELAY_DN},
    {0xD6, USER_EVENT_AV_DELAY_UP},
    {0x1F, USER_EVENT_VIZ_RMT_OFF},
    {0x1E, USER_EVENT_VIZ_RMT_ON},
    {0x77, USER_EVENT_PROGRAM_IR_VOL_UP},
    {0x09, USER_EVENT_PROGRAM_IR_VOL_DN},
    {0x72, USER_EVENT_PROGRAM_IR_MUTE},
    {0x26, USER_EVENT_APD_ON},
    {0x27, USER_EVENT_APD_OFF},
#if ( configSII_DEV_953x_PORTING == 1 )

    {0xAB, USER_EVENT_SEL_HDMI_IN},
    {0x9A, USER_EVENT_SEL_HDMI_ARC},
#endif
    {0xAA, USER_EVENT_SEL_LAST_SRC}
};
#endif 

static xIR_CMD_KEY_MAP VIZ_FAC_IR_CMD_KEY_MAP[]=
{
    {0x02, USER_EVENT_VOL_UP},
    {0x03, USER_EVENT_VOL_DN},
    {0x09, USER_EVENT_MUTE},
    {0xB7, USER_EVENT_FAC_OUTPUT_POWER_TOOGLE},
    {0x45, USER_EVENT_CENTER_ONLY},
    {0x46, USER_EVENT_WOOFER_ONLY},
    {0x47, USER_EVENT_LEFT_ONLY},
    {0x48, USER_EVENT_RIGHT_ONLY},
    {0x44, USER_EVENT_NORMAL_SOUND},
    {0x1C, USER_EVENT_SL_ONLY},
    {0x49, USER_EVENT_SR_ONLY},
    {0xB8, USER_EVENT_LEFT_CENTER_RIGHT_ONLY},
    {0xB9, USER_EVENT_SL_WOOFER_SR_ONLY},
    {0x0A, USER_EVENT_FAC_RF_PAIRING},
    {0x39, USER_EVENT_FAC_BT_PAIRING},
    {0x51, USER_EVENT_FAC_SEL_AUX1},    
    {0x5A, USER_EVENT_FAC_SEL_AUX2},
    {0xBA, USER_EVENT_FAC_SEL_COAXIAL},
    {0xC6, USER_EVENT_FAC_SEL_OPTICAL},
    {0x66, USER_EVENT_FAC_SEL_USB_PLAYBACK},
    {0x62, USER_EVENT_FAC_LED_CTRL},
    {0xB5, USER_EVENT_FAC_AQ_OFF},
    {0xB6, USER_EVENT_FAC_AQ_ON},
    {0xBB, USER_EVENT_FAC_MODE_OFF},
#if ( configSII_DEV_953x_PORTING == 1 )

    {0xBF, USER_EVENT_FAC_CLEAN_STORAGE},  

    {0xE5, USER_EVENT_FAC_SEL_HDMI_IN},
    {0xE6, USER_EVENT_FAC_SEL_HDMI_ARC}  
#else
    {0xBF, USER_EVENT_FAC_CLEAN_STORAGE}

#endif
    //{0xE3, USER_EVENT_FAC_SPK_BALANCE}/*Don't need*/
};

static const uint8 ENTER_FACTORY_KEY_MAP[]={
    0x43,   /* MENU */
    0x12,   /* 2 */
    0x14,   /* 4 */
    0x18,   /* 8 */
    0x19    /* 9 */
};



#endif
