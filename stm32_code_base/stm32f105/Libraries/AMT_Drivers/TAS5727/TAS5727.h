#ifndef __TAS5727_H__
#define __TAS5727_H__

#include "Defs.h"
#include "device_config.h"

#if ( configDAP_TAS5727 == 1 )
#include "TAS5727_i2c.h"

typedef struct _TAS5727_LOWLEVEL_OBJECT
{
    void (*initialize)( void );
    void (*mute_ctrl)( bool val );
}DAP_LR_CTRL_OBJECT;

#endif 

#endif /*__TAS5727_H__*/
