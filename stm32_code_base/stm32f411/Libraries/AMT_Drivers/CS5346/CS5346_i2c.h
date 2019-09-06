#ifndef __CS5346_I2C_H__
#define __CS5346_I2C_H__

#include "Defs.h"
#include "I2C2LowLevel.h"


byte CS5346_ReadI2C_Byte(byte RegAddr);

bool CS5346_WriteI2C_Byte(byte RegAddr,byte uc);


#endif /*__CS5346_I2C_H__*/
