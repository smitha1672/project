#ifndef __M_IR_CMD_HANDLER__H__
#define __M_IR_CMD_HANDLER_H__

#include "api_typedef.h"

#define ENABLE_IRCMD_TASK_NEW		1

typedef struct _IR_CMD_HANDLER
{
    void (*CreateTask)(void);  
	uint16 ( *get_queue_number )( void );
#if ENABLE_IRCMD_TASK_NEW
	void ( *insert_queue )( uint8 mode, uint8 index, IR_PROTOCOL_PAYLOAD *p_ir_payload );
#else
	void ( *insert_queue )( IR_PROTOCOL_PAYLOAD ir_payload );
#endif
}IR_CMD_HANDLER_OBJECT;

typedef struct _IR_PROGRAM_HANDLER
{
	void (*initial) ( void );
	void (*SendEvent) ( xHMIUserEvents event );
}IR_CMD_PROGRAM_OBJECT;


#endif
