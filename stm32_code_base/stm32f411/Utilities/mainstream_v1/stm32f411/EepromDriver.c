#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"

#include "Config.h"
#include "GPIOMiddleLevel.h"
#include "CardLowLevel.h"
#include "Debug.h"
#include "EepromDriver.h"
#include "I2C1LowLevel.h"
//_________________________________________________________________________
#define DEBUG_EEPROM_WRITE 0
#define configEEPROM_VERIFY_WRITE 0

/*------------------------ Private Variables ---------------------------------*/
/** The EEPROM PAGE size. The APP can write EEPROM_PAGE_SIZE amount of
 *  consecutive bytes.
 */


/**
 * Verify the writing.
 *
 * @param device EEPROM device ID
 * @param address A address to get the data from.
 * @param buff  A pointer to the buffer to get the data from.
 * @param size  The amount of bytes to verify.
 *
 * @return      TRUE in case of verify successful. FALSE otherwise.
 */

#if ( configEEPROM_VERIFY_WRITE == 1 )
static bool EepromDriver_verifyWrite(LOGICAL_EEPROM_DEVICE device,
                                     uint8 address, uint8 *buff, uint16 size);
#endif

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
static bool EepromDriver_writeData(LOGICAL_EEPROM_DEVICE device,
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
static bool EepromDriver_readData(LOGICAL_EEPROM_DEVICE device,
                                  uint16 addr,
                                  uint8 *buff,
                                  uint16 size);


/** */
static EEPROM_type m_logicalToPhysicalEeprom[LOGICAL_EEPROM_LAST];

/*------------------------ Public Functions ----------------------------------*/
bool EepromDriver_initialize()
{
    int i;
    for (i = 0; i < LOGICAL_EEPROM_LAST; ++i)
    {
        m_logicalToPhysicalEeprom[i].I2CAddress = EEPROM_UNDEF_ADDRESS;
    }

    return TRUE;
}

/*----------------------------------------------------------------------------*/
bool EepromDriver_mapLogicalToPhysical(LOGICAL_EEPROM_DEVICE device,
                                       uint8 I2CAddr,
                                       uint16 capacity,
                                       uint16 pageSize,
                                       uint8  chunkSize)
{
    if (device >= LOGICAL_EEPROM_LAST)
    {
        ASSERT_BOOL(TRUE);
    }

    m_logicalToPhysicalEeprom[(uint8)device].I2CAddress = I2CAddr;
    m_logicalToPhysicalEeprom[(uint8)device].capacity = capacity;
    m_logicalToPhysicalEeprom[(uint8)device].pageSize = pageSize;
    m_logicalToPhysicalEeprom[(uint8)device].chunkSize = chunkSize;

    return TRUE;
}
/*----------------------------------------------------------------------------*/
bool EepromDriver_write(LOGICAL_EEPROM_DEVICE device,
                        uint16 addr,
                        uint8 *buff,
                        uint16 size,
                        bool verify)
{
    bool retVal = TRUE;

    if (App_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }

    /* Write to EEprom */
    retVal = EepromDriver_writeData(device, addr, buff, size, verify);

    App_I2C1_sema_mutex_give();
    
    return retVal;
}

/*----------------------------------------------------------------------------*/
bool EepromDriver_read(LOGICAL_EEPROM_DEVICE device,
                       uint16 addr,
                       uint8 *buff,
                       uint16 size)
{
    bool retVal = TRUE;

    if (App_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }

    /* Read from EEprom */
    retVal = EepromDriver_readData(device, addr, buff, size);

    App_I2C1_sema_mutex_give();

    return retVal;
}

/*----------------------------------------------------------------------------*/
bool EepromDriver_erase(LOGICAL_EEPROM_DEVICE device)
{
    uint8 eraseValue = EEPROM_DEVICE_ERASE_VALUE;
    bool retVal = TRUE;
    int i;

    if (device >= LOGICAL_EEPROM_LAST)
    {
        ASSERT_BOOL(TRUE);
    }

    if (m_logicalToPhysicalEeprom[device].I2CAddress == EEPROM_UNDEF_ADDRESS)
    {
        return FALSE;
    }

    if (device >= LOGICAL_EEPROM_LAST)
    {
        ASSERT_BOOL(TRUE);
    }

    for (i = 0; i < m_logicalToPhysicalEeprom[device].capacity; ++i)
    {
        retVal &= EepromDriver_write(device, i, &eraseValue, 1, TRUE);

        ASSERT(retVal == FALSE);
    }

    return retVal;
}

/*----------------------------------------------------------------------------*/
#if ( DEBUG_EEPROM_WRITE == 1 )	
static bool vI2C_writeSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr)
{
	int i = 0;

	for( i = 0; i < length; i++ )
	{
		TRACE_DEBUG((0, "device addr = 0x%X, data[0x0%X] = 0x%X", control, (address + i), *(data + i ) ));
		vTaskDelay( 5 );
	}

	return TRUE;
}
#endif 

static bool EepromDriver_writeData(LOGICAL_EEPROM_DEVICE device,
                                   uint16 addr,
                                   uint8 *buff,
                                   uint16 size,
                                   bool verify)
{

	uint8 writeIndexFront = 0;
	uint8 writeIndexRear = size;	
	uint16 address = addr;	
	uint8 deviceAddress = 0;
	uint8 NeedchunkNum = 0;
	uint8 WhichChunk = 0;
	int16 chunkFreeSize = 0;
	uint8 writeSize = 0;
	uint16 offset = 0;
	uint8 chunkSize;
	bool retVal = TRUE;
	uint16 pageSize = 0;

    if (device >= LOGICAL_EEPROM_LAST)
    {
		TRACE_ERROR((0, "Assert failed %s[%d]", __FILE__, __LINE__));
		return FALSE;
    }

    if (m_logicalToPhysicalEeprom[device].I2CAddress == EEPROM_UNDEF_ADDRESS)
    {
        return FALSE;
    }

    pageSize = m_logicalToPhysicalEeprom[device].pageSize;
    chunkSize = m_logicalToPhysicalEeprom[device].chunkSize;

    /** Check if the address bigger then the eeprom capacity */
    if ((addr + size) > m_logicalToPhysicalEeprom[device].capacity )
    {
		TRACE_ERROR((0, "Over EEPROM capacity !!"));
        return FALSE;
    }
	
	GPIOMiddleLevel_Set( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
	
	WhichChunk = ( addr/chunkSize );
	chunkFreeSize =( ( (WhichChunk*chunkSize) + chunkSize ) - addr - size );

#if ( DEBUG_EEPROM_WRITE == 1 )	
	TRACE_DEBUG((0, "which chunk = %d", WhichChunk ));
	TRACE_DEBUG((0, "chunkFreeSize = %d", chunkFreeSize  ));
#endif 

	if ( chunkFreeSize < 0 )
	{
		NeedchunkNum = ( (0 - chunkFreeSize)/chunkSize )+( ((0 - chunkFreeSize)%chunkSize) != 0 );

#if ( DEBUG_EEPROM_WRITE == 1 )		
		TRACE_DEBUG((0, "Need chunk num = %d", NeedchunkNum ));
#endif 
		if ( NeedchunkNum <= 1 )
		{
			writeSize = ( ( (WhichChunk*chunkSize) + chunkSize ) - addr );

#if ( DEBUG_EEPROM_WRITE == 1 )		
			TRACE_DEBUG((0, "writeSize = %d", writeSize ));
#endif 
			if ( address+writeIndexFront >= pageSize )
			{
				deviceAddress = ( m_logicalToPhysicalEeprom[device].I2CAddress + 2 );
				offset = 0x100;
			}
			else
			{
				deviceAddress = m_logicalToPhysicalEeprom[device].I2CAddress;
				offset = 0;
			}

			retVal = I2C_writeSlave(deviceAddress, (address+writeIndexFront-offset), (buff+writeIndexFront), writeSize, FALSE, TRUE );
			if ( retVal == TRUE )
			{
				writeIndexFront = ( writeIndexFront + writeSize);
			}
			else
			{
				GPIOMiddleLevel_Clr( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
				TRACE_ERROR((0, "EEPROM WRITE failed %s[%d]", __FILE__, __LINE__));
				return retVal;
			}

			if ( (writeIndexRear - writeIndexFront) > 0 )
			{
				writeSize = writeIndexRear - writeIndexFront;

				if ( address+writeIndexFront >= pageSize )
				{
					deviceAddress = ( m_logicalToPhysicalEeprom[device].I2CAddress + 2 );
					offset = 0x100;
				}
				else
				{
					deviceAddress = m_logicalToPhysicalEeprom[device].I2CAddress;
					offset = 0;
				}

				retVal = I2C_writeSlave(deviceAddress, (address+writeIndexFront-offset), (buff+writeIndexFront), writeSize, FALSE, TRUE);
				if ( retVal == TRUE )
				{
					writeIndexFront = (writeIndexFront + writeSize); 
					
#if ( DEBUG_EEPROM_WRITE == 1 )		
					TRACE_DEBUG((0, "writeIndexFront = %d; writeIndexRear = %d", writeIndexFront, writeIndexRear ));
#endif 
				}
				else
				{
					GPIOMiddleLevel_Clr( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
					TRACE_ERROR((0, "EEPROM WRITE failed %s[%d]", __FILE__, __LINE__));
					return retVal;
				}
			}
		}
		else if ( NeedchunkNum > 1 )
		{
			writeSize = ( ( (WhichChunk*chunkSize) + chunkSize ) - addr );

#if ( DEBUG_EEPROM_WRITE == 1 )		
			TRACE_DEBUG((0, "2 writeSize = %d", writeSize ));
#endif 
			if ( address+writeIndexFront >= pageSize )
			{
				deviceAddress = ( m_logicalToPhysicalEeprom[device].I2CAddress + 2 );
				offset = 0x100;
			}
			else
			{
				deviceAddress = m_logicalToPhysicalEeprom[device].I2CAddress;
				offset = 0;
			}

			retVal = I2C_writeSlave(deviceAddress, (address+writeIndexFront-offset), (buff+writeIndexFront), writeSize, FALSE, TRUE );
			if (  retVal == TRUE )
			{
				writeIndexFront = ( writeIndexFront + writeSize);
				
#if ( DEBUG_EEPROM_WRITE == 1 )		
				TRACE_DEBUG((0, "2writeIndexFront = %d; writeIndexRear = %d", writeIndexFront, writeIndexRear ));
#endif 
			}
			else
			{
				GPIOMiddleLevel_Clr( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
				TRACE_ERROR((0, "EEPROM WRITE failed %s[%d]", __FILE__, __LINE__));
				return retVal;
			}

			do
			{
				if( (writeIndexRear - writeIndexFront)/ chunkSize )
				{
					if ( address+writeIndexFront >= pageSize )
					{
						deviceAddress = ( m_logicalToPhysicalEeprom[device].I2CAddress + 2 );
						offset = 0x100;
					}
					else
					{
						deviceAddress = m_logicalToPhysicalEeprom[device].I2CAddress;
						offset = 0;
					}

					writeSize = chunkSize;
					retVal = I2C_writeSlave(deviceAddress, (address+writeIndexFront-offset), (buff+writeIndexFront), writeSize, FALSE, TRUE );
					if ( retVal == TRUE )
					{
						writeIndexFront = (writeIndexFront + writeSize);
					}
					else
					{
						GPIOMiddleLevel_Clr( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
						TRACE_ERROR((0, "EEPROM WRITE failed %s[%d]", __FILE__, __LINE__));
						return retVal;
					}
				}
				else
				{
					if ( address+writeIndexFront >= pageSize )
					{
						deviceAddress = ( m_logicalToPhysicalEeprom[device].I2CAddress + 2 );
						offset = 0x100;
					}
					else
					{
						deviceAddress = m_logicalToPhysicalEeprom[device].I2CAddress;
						offset = 0;
					}
					
					writeSize = writeIndexRear - writeIndexFront;
					retVal = I2C_writeSlave(deviceAddress, (address+writeIndexFront-offset), (buff+writeIndexFront), writeSize, FALSE, TRUE);
					if ( retVal == TRUE )
					{
						writeIndexFront = (writeIndexFront + writeSize);
					}
					else
					{
						GPIOMiddleLevel_Clr( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
						TRACE_ERROR((0, "EEPROM WRITE failed %s[%d]", __FILE__, __LINE__));
						return retVal;
					}
					break;
				}
			}while( writeIndexFront < writeIndexRear );

#if ( DEBUG_EEPROM_WRITE == 1 )
			TRACE_DEBUG((0, "2 writeIndexFront = %d; writeIndexRear = %d", writeIndexFront, writeIndexRear ));
#endif 

		}

	}
	else
	{
		deviceAddress = m_logicalToPhysicalEeprom[device].I2CAddress;	
		retVal = I2C_writeSlave(deviceAddress, (address+writeIndexFront), buff, size, FALSE, TRUE);
		if ( retVal == FALSE )
		{
			GPIOMiddleLevel_Clr( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
			TRACE_ERROR((0, "EEPROM WRITE failed %s[%d]", __FILE__, __LINE__));
			return retVal;
		}
	}

	GPIOMiddleLevel_Clr( __O_MCU_EEPROM_WP );  //High EEPROM Unprotect so set Low
    return retVal;
}

/*----------------------------------------------------------------------------*/
static bool EepromDriver_readData(LOGICAL_EEPROM_DEVICE device,
                                  uint16 addr,
                                  uint8 *buff,
                                  uint16 size)
{
    bool retVal = TRUE;
    uint8 deviceAddress;
    uint16 pageSize;
    bool i2c_status;

    if (device >= LOGICAL_EEPROM_LAST)
    {
        ASSERT_BOOL(TRUE);
    }

    if (m_logicalToPhysicalEeprom[device].I2CAddress == EEPROM_UNDEF_ADDRESS)
    {
        return FALSE;
    }

    pageSize = m_logicalToPhysicalEeprom[device].pageSize;
    deviceAddress = m_logicalToPhysicalEeprom[device].I2CAddress;

    /** Check if the address + size bigger then the eeprom capacity */
    if ((addr + size) > m_logicalToPhysicalEeprom[device].capacity)
    {
        ASSERT_BOOL(retVal == FALSE);
    }

    /** Increase the I2C address according to the page size */
    while ((addr - (int16)pageSize) >= 0)
    {
        deviceAddress += 2;
        addr -= pageSize;
    }

    /** Check if the data to read divided into two different pages */
    if ((addr + size) > pageSize)
    {
        uint16 readSize = pageSize - addr;

        /** Reading from more then one page */
        while (size > 0)
        {
            /* Attempt reading data */
            i2c_status = I2C_readSlave(
                             deviceAddress,
                             addr,
                             (byte*)(buff),
                             readSize,
                             FALSE);

            if (i2c_status != TRUE)
            {
                retVal = FALSE;
            }
            ASSERT(retVal == FALSE);

            /** For the next loop */
            addr = 0;
            deviceAddress += 2;

            /** Calculating the amount of data left to read */
            size -= readSize;

            /** Sets the readSize for the next loop */
            if (size >= pageSize)
            {
                /** More the one page left so in the next time
                    we will read the whole page */
                readSize = pageSize;
            }
            else
            {
                /** Just one page left to read, read only the residual*/
                readSize = size;
            }
        }
    }
    else
    {
        /** Normal read opreation */
        /** attempt reading data */
        i2c_status = I2C_readSlave(
                             deviceAddress,
                             addr,
                             (byte*)(buff),
                             size,
                             FALSE);

        if (i2c_status != TRUE)
        {
            retVal = FALSE;
        }

        ASSERT(retVal);
    }

    return retVal;
}

/*----------------------------------------------------------------------------*/
bool EepromDriver_print(LOGICAL_EEPROM_DEVICE device, uint16 addr, uint16 size)
{
    bool retVal = TRUE;
#ifdef EEPROM_DEBUG_ENABLE
    uint8 buff[EEPROM_DEVICE_ITEMS_IN_LINE];
    int i;


    if (device >= LOGICAL_EEPROM_LAST)
    {
        ASSERT_BOOL(TRUE);
    }

    if (m_logicalToPhysicalEeprom[device].I2CAddress == EEPROM_UNDEF_ADDRESS)
    {
        return FALSE;
    }


    if (device >= LOGICAL_EEPROM_LAST)
    {
        ASSERT_BOOL(TRUE);
    }

    TRACE_DEBUG((0, "------------------------------------------------------------"));
    TRACE_DEBUG((0, "        0  |  1   |  2   |  3   |  4   |  5   |  6   |  7  "));

    for (i = 0; i < size; i += EEPROM_DEVICE_ITEMS_IN_LINE)
    {
        /** Reading from eeprom */
        retVal &= EepromDriver_read(device,
                                    (uint16)(addr + i),
                                    buff,
                                    (uint16)EEPROM_DEVICE_ITEMS_IN_LINE);

        ASSERT_BOOL(retVal == FALSE);

        /** printing one line */
        TRACE_DEBUG((STR_ID_NULL, "| "));
        TRACE_DEBUG((STR_ID_NULL,
                     "| %X | 0x%X | 0x%X | 0x%X | 0x%X | 0x%X | 0x%X | 0x%X | 0x%X ",
                     (uint8)i,
                     buff[0], buff[1], buff[2], buff[3],
                     buff[4], buff[5], buff[6], buff[7]));

    }
#endif
    return retVal;
}

/*----------------------------------------------------------------------------*/
uint16 EepromDriver_deviceCapacity(LOGICAL_EEPROM_DEVICE device)
{
    if (device >= LOGICAL_EEPROM_LAST)
    {
        return 0;
    }

    if (m_logicalToPhysicalEeprom[device].I2CAddress == EEPROM_UNDEF_ADDRESS)
    {
        return 0;
    }

    return m_logicalToPhysicalEeprom[device].capacity;
}

/*----------------------------------------------------------------------------*/
#if ( configEEPROM_VERIFY_WRITE == 1 )
static bool EepromDriver_verifyWrite(LOGICAL_EEPROM_DEVICE device,
                                     uint8 address, uint8 *buff, uint16 size)
{
    uint8 data[16];
    bool retVal = TRUE;
    int i;
    int x;
    uint16 verify_size;

    if (size < sizeof(data))
    {
        verify_size = size;
    }
    else
    {
        verify_size = sizeof(data);
    }

    for (i = 0; i < size; i += verify_size)
    {
        retVal &= EepromDriver_readData(device,
                                        (address + i),
                                        data,
                                        verify_size);

        for (x = 0; (x < verify_size) && (x < size); x++)
        {
            if (data[x] != *(buff + i + x))
            {
                retVal &= FALSE;
            }
        }
    }

    return retVal;
}
#endif




