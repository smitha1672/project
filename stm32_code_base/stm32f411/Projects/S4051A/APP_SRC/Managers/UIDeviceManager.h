#ifndef __UI_DEVICE_MANAGER_H__
#define __UI_DEVICE_MANAGER_H__

#include "freertos_conf.h"
#include "HMI_Service.h"

typedef struct _UI_DEVICE_CTRL
{
    void (*CreateTask)( void );
    void (*SendEvent) ( const void *parms );
    void (*ExceptionSendEvent)( xUIExceptionEvents UIExcept );
}UI_DEV_MANAGER_OBJECT;

//______________________________________________________________________________
#endif /*__UI_DEVICE_MANAGER_H__*/
