#ifndef __TAS5711_H__
#define __TAS5711_H__

#include "Defs.h"
#include "device_config.h"

/*-------------------------- Marco defintion -------------------------------------*/
#if ( configDAP_TAS5711 == 1 )
#include "TAS5711_i2c.h"

typedef struct _TAS5711_LOWLEVEL_OBJECT
{
    void (*initialize)( void );
    void (*mute_ctrl)( bool val );
}DAP_C_CTRL_OBJECT;

#endif 

#endif /*__TAS5711_H__*/
