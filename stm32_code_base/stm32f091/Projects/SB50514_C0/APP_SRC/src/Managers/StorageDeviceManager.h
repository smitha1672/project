#ifndef __STORAGE_DEVICE_MANAGER_H__
#define __STORAGE_DEVICE_MANAGER_H__

#include "Defs.h"
#include "api_typedef.h"

typedef struct _STORAGE_DEVICE_CTRL
{
	int8 ( *Initialize ) ( xHMISystemParams *params );
	bool ( *SendEvent ) ( void *params );
	bool ( *read ) ( STORAGE_DEVICE_CELL cell, void *buff );
	bool ( *write ) ( STORAGE_DEVICE_CELL cell, void *buff );
}STORAGE_DEVICE_MANAGER_OBJECT;


#endif 
