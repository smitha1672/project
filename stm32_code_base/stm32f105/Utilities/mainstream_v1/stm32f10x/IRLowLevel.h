#ifndef __IR_LOW_LEVEL_H__
#define __IR_LOW_LEVEL_H__

#include "Defs.h"

#define ENABLE_IR_DECODE_TASK_NEW 1

#define IR_ROW_DATA_MAX (108) //(68)    , Angus modify for panasonic protocol that needs size of 100 at least.(2014/11/11)
#define IR_TIME_BASE 50


typedef struct _IR_LOWLEVEL_OBJECT
{
#if ENABLE_IR_DECODE_TASK_NEW
    uint16 (*get_row_data)( uint16 *pdata , uint16 bufSize, bool bWait);
#else
    uint16 (*get_row_data)( uint16 *pdata );
#endif
    uint16 (*get_queue_number )(void );
    void (*clr_rowdata)( void );
}IR_LOWLEVEL_OBJECT;

#endif

