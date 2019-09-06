#ifndef __M_IR_CMD_HANDLER__H__
#define __M_IR_CMD_HANDLER_H__

#include "api_typedef.h"


typedef struct _IR_CMD_HANDLER
{
    void (*CreateTask)(void);  
	uint16 ( *get_queue_number )( void );
	void ( *get_queue )( IR_PROTOCOL_PAYLOAD *pData );
	void ( *insert_queue )( IR_PROTOCOL_PAYLOAD ir_payload );
}IR_CMD_HANDLER_OBJECT;

typedef struct _IR_PROGRAM_HANDLER
{
	void (*initial) ( void );
	void (*SendEvent) ( xHMIUserEvents event );
}IR_CMD_PROGRAM_OBJECT;


#endif
