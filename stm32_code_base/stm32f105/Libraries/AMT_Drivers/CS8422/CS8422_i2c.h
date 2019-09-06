#ifndef __CS8422_I2C_H__
#define __CS8422_I2C_H__

#include "Defs.h"
#include "I2C2LowLevel.h"

byte CS8422_ReadI2C_Byte(byte RegAddr);

bool CS8422_WriteI2C_Byte(byte RegAddr,byte uc);


#endif /*__CS8422_I2C_H__*/
