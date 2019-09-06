#ifndef __HDMI_DEVICE_MANAGER_H__
#define __HDMI_DEVICE_MANAGER_H__


#include "Defs.h"
#include "api_typedef.h"
#include "device_config.h"

typedef struct _HDMI_DEVICE_CTRL
{
	void (*CreateTask)( void );
	bool ( *SendEvent ) ( void *params );
	bool (*GetMultiChannel)(void);
#if ( configSII_DEV_953x_PORTING == 1 )
	xHMI_SiiAppPowerStatus_t (*GetHDMIPowerStatus)(void);
	bool (*ishdmiOff)( void );
	bool (*hdmiOnOff)( bool enable );
        void (*HDMI_DEV_RST)(void);
#endif
}HDMI_DEVICE_MANAGER_OBJECT;


#endif 
