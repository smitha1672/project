#ifndef __CS49XXXX_SPI_H__
#define __CS49XXXX_SPI_H__

#include "Defs.h"
#include "BSP.h"

/*! \name SPI setting for CS9XXXX 
 */
//! @{

#define CS4953x_SPI_NPCS	0
//! @}


#define SCP1_IRQ_TIMEOUT -1
#define SCP1_BSY_TIMEOUT -1
#define SCP1_PASS 0
#define SCP1_IRQ_LOW 1 /* indicate dsp has massge need to dump*/
#define SCP1_BYPASS 2

int8 CS4953xSPI_write_buffer(const byte* data, uint16 length);

int8 CS4953xSPI_read_buffer( byte* data, uint16 length );

int8 CS4953xSPI_nIrq_read_buffer( byte* data, uint16 length );

int8 CS4953xSPI_MsgNote1( byte* data, uint16 length );

bool CS49xxxxSPI_CommandWrite(uint32 cmd, uint32 value);

bool CS49xxxxSPI_ReadSolicited( uint32 cmd, uint32 *pValue );

int8 CS4953xSPI_write_ULD_buffer(const byte* data, uint16 length);


#endif /*__CS49XXXX_SPI_H__*/
