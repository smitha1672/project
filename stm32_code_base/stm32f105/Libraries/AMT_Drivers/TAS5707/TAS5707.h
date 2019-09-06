#ifndef __TAS5707_H__
#define __TAS5707_H__

#include "Defs.h"
#include "device_config.h"

#if ( configDAP_TAS5707 == 1 )
#include "TAS5707_i2c.h"

typedef struct _TAS5707_LOWLEVEL_OBJECT
{
    void (*initialize)( void );
    void (*mute_ctrl)( bool val );
}DAP_LR_CTRL_OBJECT;

#endif 



#endif /*__TAS5707_H__*/
