#ifndef _CS8422_H
#define _CS8422_H

#include "Defs.h"
#include "device_config.h"

#include "CS8422_i2c.h"

typedef struct _CS8422_LOWLEVEL_OBJECT
{
    void (*initialize)( void );
    void (*isr_ctrl)( bool val );
    void (*set_rx_path)( uint8 rx_path, uint8 src_ctrl );
    void (*set_serial_audio_in)( void );
    bool (*IsLockPLL)( void );
    bool (*getISCLK_ACTIVE)( void );
    bool (*Format_IsPCM)( void );
    bool (*is_96Khz192Khz_AES3)( void );
    uint8(*getForcesClockControl)( void );
    void (*ForcesClockControl)( uint8 index );
    uint8(*read_SDOUT1_DataSource)( void );
    void (*SerialAudioOutput)( uint8 index );
}SRC_CTRL_OBJECT;

#endif // _CS8422_H

