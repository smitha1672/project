#ifndef __GPIO_LOW_LEVEL_H__
#define __GPIO_LOW_LEVEL_H__

#include "Defs.h"

typedef enum { 
	eOUT_PIN	=	0,
	eIN_PIN	= 1
} IO_Pin;

typedef struct
{
  GPIO_TypeDef*	PORTx; 
  uint16_t	PINx;
  IO_Pin	IO;
} GPIO_PhysicalPin;

typedef struct _GPIO_LOWLEVEL_ISR_OBJECT
{
    void (*src_isr_configure)( void );
    void (*src_isr_control )( bool bIsrEnable );
}GPIO_ISR_OBJECT;



#endif /*__GPIO_LOW_LEVEL_H__*/
