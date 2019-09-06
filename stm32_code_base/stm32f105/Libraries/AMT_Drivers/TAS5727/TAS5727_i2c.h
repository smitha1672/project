#ifndef __TAS5727_I2C_H__
#define __TAS5727_I2C_H__

#include "Defs.h"
#include "I2C2LowLevel.h"

byte TAS5727_I2C_read(  byte reg_addr);

bool TAS5727_I2C_write(  byte reg_addr, byte value);

bool TAS5727_I2C_read_n_byte(  byte reg_addr, byte *data, int N);

bool TAS5727_I2C_write_n_byte(  byte reg_addr, byte* data,int N);

#endif /*__TAS5727_I2C_H__*/
