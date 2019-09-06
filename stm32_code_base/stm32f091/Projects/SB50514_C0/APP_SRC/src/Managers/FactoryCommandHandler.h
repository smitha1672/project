#ifndef __FACTORY_COMMAND_HANDLER_H__
#define __FACTORY_COMMAND_HANDLER_H__

#include "Defs.h"
#include "api_typedef.h"

typedef struct _FACTORY_CMD_CTRL
{
  bool (*Transfer2Task)( void *params );
  void (*AckSuccess)( uint8 *pdata );
}FACTORY_CMD_HANDLE_OBJECT;

void FactoryCmdHandler_CreateTask( void );


#endif /*__FACTORY_COMMAND_HANDLER_H__*/
