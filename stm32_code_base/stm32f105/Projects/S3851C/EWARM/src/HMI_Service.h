#ifndef __HMI_SERVICE_H__
#define __HMI_SERVICE_H__

#include "freertos_conf.h"
#include "api_typedef.h"

typedef struct HMI_SERVICE_CTRL
{
	bool ( *SendEvent ) ( void *params );
	void ( *SetSystemParams ) ( const xHMISystemParams *params );
	void ( *ResetSystemParams) ( void );
	xHMISystemParams ( *GetSystemParams )( void );
}HMI_SERVICE_OBJECT;

typedef struct HMI_FACTORY_CTRL
{
	void ( *SetSystemParams ) ( const xHMISystemParams *params );
	void ( *ResetSystemParams) ( void );
	void ( *cmd_handle )( const xHMISystemParams *params );
}HMI_FACTORY_OBJECT;

void HMI_Service_CreateTask( void );


#endif /*__HMI_SERVICE_H__*/
