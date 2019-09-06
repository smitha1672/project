#ifndef _CS5346_H
#define _CS5346_H

#include "Defs.h"
#include "CS5346_i2c.h"

typedef struct _CS5346_LOWLEVEL_OBJECT
{
    void (*initialize)( void );
    void (*input_path)( uint8 idx );
    void (*input_gain)( uint8 a_gain, uint8 b_gain ); 
    void (*HardReset)(void);
}ADC_CTRL_OBJECT;

#endif // _CS5346_H


