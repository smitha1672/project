#ifndef __APP_EEPROMDRIVER_H__
#define __APP_EEPROMDRIVER_H__

#include "Defs.h"
#include "CardLowLevel.h"
#include "Config.h"
#include "device_config.h"

/*-------------------------------- EEPROM ----------------------------------- */

#if ( configEEPROM_512 == 1 )
#define EEPROM_DEVICE_0_CAPACITY_BYTE   512 
#define EEPROM_DEVICE_0_PAGE_SIZE_BYTE   256 
#define EEPROM_DEVICE_0_CHUNK_SIZE_BYTE    16
#else 
#error "EEPROM SIZE has not been define !!"
#endif 


/*--------------------- Public Members and Definitions -----------------------*/

/** EEprom default value after erase operation*/
#define EEPROM_DEVICE_ERASE_VALUE           0xff

/** number of items in line, used in print function */
#define EEPROM_DEVICE_ITEMS_IN_LINE           8

/** Undef Eeprom I2C address*/
#define EEPROM_UNDEF_ADDRESS                  0xFF

/** TBD */
typedef struct
{
    uint8         I2CAddress;
    uint16        capacity;
    uint16        pageSize;
    uint8         chunkSize;
} EEPROM_type;

/*------------------------ Public Functions ----------------------------------*/

/*
 * Initializes the EEPROM driver
 *
 * @return      TRUE in case of init successful. FALSE otherwise.
 */
bool EepromDriver_initialize();


/*
 * Map Logical Eeprom to Physical Eeprom
 *
 * @param device EEPROM device ID
 * @param I2CAddr I2C device Address
 * @param capacity EEPROM capacity
 * @param pageSize EEPROm page size
 * @param chunkSize EEPROm chunk size
 *
 * @return      TRUE in case of init successful. FALSE otherwise.
 */
bool EepromDriver_mapLogicalToPhysical(LOGICAL_EEPROM_DEVICE device,
                                       uint8 I2CAddr,
                                       uint16 capacity,
                                       uint16 pageSize,
                                       uint8  chunkSize);

/*
 * Writes a buffer to the EEPROM.
 *
 * @param device    The EEPROM device ID.
 * @param addr  the EEPROM internal address to start the write operation.
 * @param buff  A pointer to the buffer to get the data from.
 * @param size  The amount of bytes to write.
 * @param verify Verify write
 *
 * @return      TRUE in case of write successful. FALSE otherwise.
 */
bool EepromDriver_write(LOGICAL_EEPROM_DEVICE device,
                        uint16 addr,
                        uint8 *buff,
                        uint16 size,
                        bool verify);

/*
 * Read a buffer from the EEPROM.
 *
 * @param device    The EEPROM device ID.
 * @param addr  the EEPROM internal address to start the read operation.
 * @param buff  A pointer to the buffer to write the data to.
 * @param size  The amount of bytes to read.
 *
 * @return      TRUE in case of read successful. FALSE otherwise.
 */
bool EepromDriver_read(LOGICAL_EEPROM_DEVICE device,
                       uint16 addr,
                       uint8 *buff,
                       uint16 size);
/*
 * Erase the complete EEPROM.
 *
 * @param device    The EEPROM device ID.
 *
 * @return      TRUE in case of erase successful. FALSE otherwise.
 */
bool EepromDriver_erase(LOGICAL_EEPROM_DEVICE device);


/*
 * Print the EEprom content
 *
 * @param device    The EEPROM device ID.
 * @param addr  the EEPROM internal address to start the print
 * @param size  The amount of bytes to read.
 *
 * @return      TRUE in case of erase successful. FALSE otherwise.
 */
bool EepromDriver_print(LOGICAL_EEPROM_DEVICE device, uint16 addr, uint16 size);

/*
 * Returns the specific EEPROM device capacity in bytes.
 *
 * @param device    The EEPROM device ID.
 *
 * @return      The EEPROM device capacity in bytes.
 */
uint16 EepromDriver_deviceCapacity(LOGICAL_EEPROM_DEVICE device);

#endif // of __APP_EEPROMDRIVER_H__

