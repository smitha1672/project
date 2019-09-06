#ifndef __AMPLIFIER_DEVICE_MANAGER_H__
#define __AMPLIFIER_DEVICE_MANAGER_H__

#include "Defs.h"


typedef struct _AMP_DEVICE_CTRL
{
    void ( *initialize )( void );
    void ( *set_front_mute )( bool val );
    void ( *set_rear_mute )( bool val ); 
}AMP_DEV_MANAGER_OBJECT;

#endif /*__AMPLIFIER_DEVICE_MANAGER_H__*/
