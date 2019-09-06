#ifndef __CS8422_ISR_HANDLER_H__
#define __CS8422_ISR_HANDLER_H__

#include "Defs.h"

#define CS8422_ISR_FROM_EXTI 0x01

typedef struct _CS8422_ISR_OBJECT
{
    void (*initialize)( void );
    void (*queue_send_from_isr )( uint8 *pIntType );
}SRC_ISR_OBJECT;


#endif 
