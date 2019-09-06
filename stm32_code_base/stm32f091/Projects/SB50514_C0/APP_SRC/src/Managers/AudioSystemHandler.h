#ifndef __AUDIO_SYSTEM_HANDLER_H__
#define __AUDIO_SYSTEM_HANDLER_H__

#include "freertos_conf.h"
#include "device_config.h"

typedef struct AUDIO_SYSTEM_HANDLER_CTRL
{
    void ( *CreateTask ) ( void );
    bool ( *SendEvent ) ( const void *params );
    uint8 ( *IsSignalAvailable )( void );
    bool ( *getRampStatus)(void);
    uint8 (*getRampVolvalue)(void);
    uint8 (*getTargetVol)(void);
 #if ( configSII_DEV_953x_PORTING == 1 )
    xHMI_SiiAppPowerStatus_t (*getHDMIPowerStatus)(void);
    bool (*isHDMIPowerOff)(void);
#endif
}AUDIO_SYSTEM_HANDLER_OBJECT;

//_____________________________________________________________________________________________________

#endif /*__AUDIO_SYSTEM_MANAGER_H__*/
