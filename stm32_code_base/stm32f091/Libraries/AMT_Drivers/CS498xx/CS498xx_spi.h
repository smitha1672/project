#ifndef __CS498xx_SPI_H__
#define __CS498xx_SPI_H__

#include "Defs.h"
#include "STM32F0xx_board.h"


#define DSP_BUSY FALSE
#define DSP_IDLE TRUE

void CS498xx_SPI_initialize(long pba_hz);

bool CS498xx_SPI_write_buffer(byte* data, uint16 length);

bool CS498xx_SPI_read_buffer(byte* data, uint16 length);

bool CS498xx_SPI_IsDSP_IRQ_LOW(uint32 timeout);

bool CS498xx_SPI_read_buffer_NonIRQ(byte* data, uint16 length);

bool CS498xx_SPI_isEnable( void );

bool CS498xx_SPI_CommandWrite(uint32 cmd, uint32 value);

bool CS498xx_SPI_ReadSolicited( uint32 cmd, uint32 *pValue );

bool SPI_selectChip( unsigned char chip );

bool SPI_unselectChip( unsigned char chip );


#endif /*__CS498xx_SPI_H__*/
