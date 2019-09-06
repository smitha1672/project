#ifndef __TAS5713_H__
#define __TAS5713_H__

#include "Defs.h"
#include "device_config.h"

#if ( configDAP_TAS5713 == 1 )
#include "TAS5713_i2c.h"

typedef struct _TAS5713_LOWLEVEL_OBJECT
{
    void (*initialize)( bool Address );
    void (*mute_ctrl)(bool Address,bool val);
}DAP_C_CTRL_OBJECT;

#endif 

#endif /*__TAS5713_H__*/
