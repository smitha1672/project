#include "UART_Lowlevel.h"
#include "SOUND_BAR_V1_PinDefinition.h"

#define RECEIVE_BUFFER_SIZE 136

typedef struct
{
    /** Buffer storing all characters received and pending to be read */
    char m_rxBuffer[RECEIVE_BUFFER_SIZE];

    /** Number of bytes in the received buffer */
    byte m_rxBufferLength;
    
    /** Index to the oldest data in the buffer */
    byte m_rxBufferStart;

    /** */
    int m_rxOverruns;
    
    /** Tx Buffer Ready Flag */
    bool m_txBufferEmpty;
    
} UART_port;

/*Smith modify: remove unnecessary memory*/
static UART_port m_port;	


/*---------------------- AMTRAN IMPLEMENT ----------------------*/
byte UARTLowLevel_getDataLength( void )
{
    UART_port* p = &m_port;

    return ( p->m_rxBufferLength );
}

bool UARTLowLevel_isTxBufferReady(uint8 port)
{
	/*Smith modify: remove unnecessary memory*/
	//UART_port* p = &m_port[port];
    UART_port* p = &m_port;	
	
    return p->m_txBufferEmpty;
}

int UARTLowLevel_readData(uint8 port, void *buffer, int maxLength)
{
	/*Smith modify: remove unnecessary memory*/
	//UART_port* p = &m_port[port];
    UART_port* p = &m_port;	

    /* Calculate the actual length of data to read */
    int lengthToTake = MINIMUM( maxLength, p->m_rxBufferLength );

    if (lengthToTake == 0)
    {
        return 0;
    }
    
    /* If copying the buffer will pass the end of the buffer and wrap around */
    if ( (p->m_rxBufferStart + lengthToTake) > RECEIVE_BUFFER_SIZE )
    {
        /* Then copy in two phases */
        byte firstPhase = RECEIVE_BUFFER_SIZE - p->m_rxBufferStart;
        byte secondPhase = lengthToTake - firstPhase;
        void* secondPhaseStart = (byte*)(buffer) + firstPhase;
		
        MEMCPY(
            buffer,
            (void*)(p->m_rxBuffer + p->m_rxBufferStart), 
            firstPhase
            );
            
        MEMCPY(
            secondPhaseStart,
            (void*)(p->m_rxBuffer),
            secondPhase
            );
    }
    else
    {
    
        MEMCPY(
            buffer,
            p->m_rxBuffer + p->m_rxBufferStart,
            lengthToTake
            );
    }


    /* Position the start pointer onwards... */
    p->m_rxBufferStart += lengthToTake;
    if ( p->m_rxBufferStart >= RECEIVE_BUFFER_SIZE )
    {
        p->m_rxBufferStart -= RECEIVE_BUFFER_SIZE;
    }
    
    /* And update the length */
    p->m_rxBufferLength -= lengthToTake;

    return lengthToTake;
}

static void UARTLowLevel_interruptHandlerCommon(uint8 port, uint8 c) 
{
	/*Smith modify: remove unnecessary memory*/
	//UART_port* p = &m_port[port];
	UART_port* p = &m_port;
	
	uint8 U0RBR = 0;

	U0RBR= c;

	/* If the buffer is full, step on the oldest data */
	if (p->m_rxBufferLength == RECEIVE_BUFFER_SIZE)
	{
		p->m_rxBuffer[p->m_rxBufferStart]=U0RBR;
		ADD_MODULO(p->m_rxBufferStart, 1, RECEIVE_BUFFER_SIZE);
		p->m_rxOverruns++;
	}
	else
	{
		/* Calculate the position to add the value to */
		int pos = p->m_rxBufferStart;
		ADD_MODULO(pos, p->m_rxBufferLength, RECEIVE_BUFFER_SIZE);

		/* Read and save the data */
		p->m_rxBuffer[pos]=U0RBR;

		/* Increase the m_rxBufferLength */
		p->m_rxBufferLength++;
	}
}      


void usart_lowlevel_handler(uint8_t port, uint8_t c)
{
	UARTLowLevel_interruptHandlerCommon( port, c );
}
	

