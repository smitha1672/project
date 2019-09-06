#ifndef __TAS5713_I2C_H__
#define __TAS5713_I2C_H__

#include "Defs.h"
#include "I2C2LowLevel.h"

byte TAS5713_I2C_read(bool Address,byte reg_addr);

bool TAS5713_I2C_write(bool Address,byte reg_addr, byte value);

bool TAS5713_I2C_read_n_byte(bool Address,byte reg_addr, byte *data, int N);

bool TAS5713_I2C_write_n_byte(bool Address,byte reg_addr, byte* data,int N);

#endif /*__TAS5713_I2C_H__*/
