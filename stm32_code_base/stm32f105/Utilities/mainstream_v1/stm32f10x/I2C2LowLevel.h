#ifndef __I2C2_LOW_LEVEL_H__
#define __I2C2_LOW_LEVEL_H__

#include "Defs.h"


/*-------------------------------------------------------------------------------------------------------*/
bool I2C2_writeSlave(byte control, uint16 address, const byte *data, uint16 length, bool longAddr); 

bool I2C2_writeSlave_NoSystemWait( byte control, uint16 address, const byte *data, uint16 length, bool longAddr ) ;

bool I2C2_readSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr); 

void TWI2_initialize( void );

void TWI2_Deinitialize( void );


#endif 
