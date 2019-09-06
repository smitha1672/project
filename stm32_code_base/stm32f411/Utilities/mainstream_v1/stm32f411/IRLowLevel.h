#ifndef __IR_LOW_LEVEL_H__
#define __IR_LOW_LEVEL_H__

#include "Defs.h"

#define IR_ROW_DATA_MAX (68)
#define IR_TIME_BASE 50


typedef struct _IR_LOWLEVEL_OBJECT
{
    uint16 (*get_row_data)( uint16 *pdata );
    uint16 (*get_queue_number )(void );
    void (*clr_rowdata)( void );
}IR_LOWLEVEL_OBJECT;

#endif

