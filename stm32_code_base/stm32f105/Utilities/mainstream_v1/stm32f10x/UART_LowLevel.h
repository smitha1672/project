#ifndef __APPLICATION_LOWLEVEL_UARTLOWLEVEL_H__
#define __APPLICATION_LOWLEVEL_UARTLOWLEVEL_H__

#include "Defs.h"

/*-------------------AMTRAN IMPLEMENTED---------------------*/
#define SERIAL_PORT_NUM      2
#define SERIAL_PORT_0           0
#define SERIAL_PORT_1           1
#define UART_PORT                        SERIAL_PORT_0
#define VIRTUAL_PORT	SERIAL_PORT_1


byte UARTLowLevel_getDataLength( void );

bool UARTLowLevel_isTxBufferReady(uint8 port);

int UARTLowLevel_readData(uint8 port, void *buffer, int maxLength);


#endif /*__APPLICATION_LOWLEVEL_UARTLOWLEVEL_H__*/
