#ifndef __I2C1_LOW_LEVEL_H__
#define __I2C1_LOW_LEVEL_H__

#include "Defs.h"

/*-------------------------------------------------------------------------------------------------------*/
bool I2C_writeSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr, bool b_DMA_used);

bool I2C_readSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr); 

bool I2C_writeSlave_NoSystemWait(byte control, uint16 address, byte *data, uint16 length, bool longAddr); 

void TWI_initialize( void );

void TWI_Deinitialize( void );

/*-------------------------------------------------------------------------------------------------------*/
/* Add mutex function for app level used*/
bool App_I2C1_sema_mutex_take(void);
bool App_I2C1_sema_mutex_give(void);

/**
 * Wait until a device a ready for I2C operations. 
 *
 * @param control   The slave control ID (I2C Address).   
 *
 * @return  TRUE in case the devie is ready. FALSE in case after a TIMEOUT, the
 *          device is still not ready.
 */
bool I2C_wait4DeviceReady(byte control);


#endif 
