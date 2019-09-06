#ifndef __SPI_LOW_LEVEL_H__
#define __SPI_LOW_LEVEL_H__

#include "Defs.h"

#define SPIx    SPI4
#define _SPI    SPIx

/** Time out to wait for an SPI write cmd ack */
#define SPI_TIME_OUT_WRITE_CMD      VIRTUAL_TIMER_MSEC2TICKS(10)

/** Time out to wait for an SPI write cmd ack */
#define SPI_TIME_OUT_READ_CMD      VIRTUAL_TIMER_MSEC2TICKS(10)


#define SPI_ACK						0xAA	/** ACK from slave */
#define SPI_NACK					0xBB	/** NACK from slave*/
#define SPI_WRITE_COMMAND			0x01	/** Code for WRITE command*/
#define SPI_READ_COMMAND			0x11	/** Code for READ command*/
#define SPI_DUMMY_BYTE				0x00	/** SPI dummy byte */

#define SPI_LITTEL_ENDIAN			TRUE
#define SPI_BIG_ENDIAN				FALSE
#define SPI_RET_ERROR_LEN			-1


/*
Initalize the WHDI SPI interface.
Parameters:
pba_hz : The main clock used for generate SPI clock 
*/
int16 SPI_writeBuffer(const byte *data, uint16 length, bool LittelEndian);

int16 SPI_readBuffer(byte *data, uint16 length, bool LittelEndian);

bool SPI_write( byte data ); 

bool SPI_read( byte *data );


#if defined ( FREE_RTOS )
bool SPI_mutex_lock( );
#endif 

#if defined ( FREE_RTOS )
bool SPI_mutex_unlock( );
#endif 

bool SPI_unselectChip( unsigned char chip );

bool SPI_selectChip( unsigned char chip );

bool SPI_unselectChip( unsigned char chip );

void SPILowLevel_release_SPI(void);

bool SPILowLevel_isEnable( void );


#endif /*__SPI_LOW_LEVEL_H__*/
