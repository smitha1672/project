#include "EepromDriver.h"
#include "StorageDeviceDriver.h"
#include "Debug.h"
#include "Config.h"
#include "device_config.h"
#include "api_typedef.h"

//___________________________________________________________________________
#if ( configEEPROM_512 == 1 )
#define EEPROM_PAGE_SIZE 16 /*bytes*/
#else
#warning "EEPROM PAGE has not been defined !! "
#define EEPROM_PAGE_SIZE 0
#endif 

#define DEBUG_STORAGE 0

#define configSTORAGE_UPDATE_CHECKSUM 0

#define USER_PARAMETER_SIZE (sizeof(xHMISystemParams)/sizeof(uint8))
#define PRG_REMOTE_SIZE ( sizeof(IR_PRG_REMOTE_USER_EVENT)/sizeof( uint8 ) )



/************************* Private Members ************************************/

/* This table holds the allocated partition's information for storage device*/
static storagePartitionType m_partitionSpaceAllocation[] =
{
	{ (EEPROM_PAGE_SIZE*2), 0, STORAGE_DISK_0 }, /*factory*/
	{ (EEPROM_PAGE_SIZE*2), 0, STORAGE_DISK_0 }, /*user paramter*/
	{ (EEPROM_PAGE_SIZE*1), 0, STORAGE_DISK_0 }, /*program remote */
	{ (EEPROM_PAGE_SIZE*1), 0, STORAGE_DISK_0 }, /*program remote */
	{ (EEPROM_PAGE_SIZE*1), 0, STORAGE_DISK_0 }, /*program remote */
};


/* This table holds the allocated cells' information for storage device*/
static storageCellType m_storageSpaceAllocation[] =
{
    {15, STORAGE_PARTITION_0, STORAGE_DEVICE_CELL_PRODUCT_SERIAL_NUMBER},

    {USER_PARAMETER_SIZE, STORAGE_PARTITION_1, STORAGE_DEVICE_CELL_USER_PARAMETERS}, 

	{PRG_REMOTE_SIZE, STORAGE_PARTITION_2, STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_UP },

	{PRG_REMOTE_SIZE, STORAGE_PARTITION_3, STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_DN },

	{PRG_REMOTE_SIZE, STORAGE_PARTITION_4, STORAGE_DEVICE_CELL_PROGRAM_KEY_VOLUME_MUTE }
};


/************************** Private Functions *********************************/
/*
 * Calculates the cell's address from the begining of the storage disk
 * @param   cell    A specific location and size in the storage device where
 *                  the data is going to be read from.
 *
 * @param   device  the returned logical Eeprom device
 *
 * @param   size    the returned  cell's size
 * @param   addr    the calculated cell's address
 *
 * @return          TRUE in case that address is valid.
 *                  FALSE otherwise.
 */
static bool StorageDevice_calcCellAddress(STORAGE_DEVICE_CELL cell,
        LOGICAL_EEPROM_DEVICE *device,
        uint16 *size,
        uint16 *addr);

/*
 * Update the checksum
 *
 * @param   cell    A specific location and size in the storage device.
 *
 * @param   buff    A pointer to the data.
 *
 * @return          TRUE in case of checksum updated.
 *                  FALSE otherwise.
 */
#if ( configSTORAGE_UPDATE_CHECKSUM == 1 ) 
static bool StorageDevice_updateChecksum(STORAGE_DEVICE_CELL cell, void *buff);
#endif 

/*
 * Initialize the partition.
 *
 * @param   partition    partition number.
 *
 * @return          TRUE in case of success
 *                  FALSE otherwise.
 */
static bool StorageDevice_initializePartition(STORAGE_PARTITION partition);

/*
 * Calculate the partition checksum
 *
 * @param   partition    partition number.
 *
 * @param   update       Calc and set new checksum.
 * @return          TRUE in case on success
 *                  FALSE otherwise.
 */
static bool StorageDevice_checkPartitionChecksum(STORAGE_PARTITION partition,
        bool update);

/*
 * Checking the disk revision
 *
 * @return          TRUE in case of match revision.
 *                  FALSE otherwise.
 */
static bool StorageDevice_checkDiskRevision(STORAGE_DISK disk);

/*
 * Calculate the partition offset from the begining of the disk
 *
 * @param   partition    partition number.
 *
 * @return  the offset from the begining of the disk
 */
static uint16 StorageDevice_calcPartitionAddress(STORAGE_PARTITION partition);


/************************* Implementation *************************************/
bool StorageDevice_initialize(void)
{
    bool retVal = TRUE;
    int i;
    bool diskRevValid;
    //bool partitionValid;

    /** Checking the disk revision on all disks */
    for (i = 0; i < STORAGE_DISK_LAST; i++)
    {
        diskRevValid = StorageDevice_checkDiskRevision((STORAGE_DISK)i);

        if (diskRevValid == FALSE)
        {
            TRACE_ERROR((STR_ID_NULL, "Disk revision invalid"));

			retVal &= diskRevValid;
        }
    }

#if 0 // smith mark
    for (i = 0; i < STORAGE_PARTITION_LAST; i++)
    {
        /** Checking the partition checksum */
        partitionValid = StorageDevice_checkPartitionChecksum(
                             (STORAGE_PARTITION)i,
                             FALSE);

        if ( partitionValid == FALSE )
        {
            retVal &= StorageDevice_initializePartition((STORAGE_PARTITION)i);
            TRACE_ERROR((STR_ID_NULL, "Partition checksum invalid, reInit Partition: %d", i));
        }
    }
#endif 

    return retVal;
}

bool StorageDevice_read(STORAGE_DEVICE_CELL cell, void *buff)
{
    uint16 addr;
    uint16 size;
    LOGICAL_EEPROM_DEVICE device;
    bool retVal = TRUE;

    /** Calculate the real address in the logical Eeprom*/
    retVal &= StorageDevice_calcCellAddress(cell, &device, &size, &addr);

#if( DEBUG_STORAGE == 1 )
	TRACE_DEBUG((0, "read cell = %d, size = %d, addr = 0x%X", cell, size, addr ));
#endif 
	

    /* Reading the data from the EEPROM*/
    retVal &= EepromDriver_read(device, addr, (uint8 *)buff, size);

	ASSERT( retVal );

    return retVal;
}

bool StorageDevice_write(STORAGE_DEVICE_CELL cell, void *buff)
{
    uint16 addr;
    uint16 size;
    LOGICAL_EEPROM_DEVICE device;
    bool retVal = TRUE;

    /** Calculate the real address in the logical Eeprom*/
    retVal &= StorageDevice_calcCellAddress(cell, &device, &size, &addr);

   
#if 0	/*smith marks: we don't need to calc checksum in every modication*/
	/** Update the checksum accordingly */
    retVal &= StorageDevice_updateChecksum(cell, buff);
#endif

#if( DEBUG_STORAGE == 1 )
	TRACE_DEBUG((0, "write cell = %d, size = %d, addr = 0x%X", cell, size, addr ));
#endif 

	

    /* Writing data to the EEPROM */
    retVal &= EepromDriver_write(device, addr, (uint8 *)buff, size, FALSE);

	ASSERT( retVal );

    return retVal;
}

bool StorageDevice_eraseDeviceCell( STORAGE_DEVICE_CELL cell )
{
    uint16 addr;
    uint16 size;
	uint16 i;
    LOGICAL_EEPROM_DEVICE device;
	uint8 eraseValue = EEPROM_DEVICE_ERASE_VALUE;
    bool retVal = TRUE;

	/** Calculate the real address in the logical Eeprom*/
    retVal &= StorageDevice_calcCellAddress(cell, &device, &size, &addr);

    for (i = 0; i < size; i++)
    {
        retVal &= EepromDriver_write(device, i, &eraseValue, 1, TRUE);

        ASSERT(retVal == FALSE);
    }

    return retVal;

}

bool StorageDevice_print(STORAGE_DISK disk)
{
    bool retVal = TRUE;
#if 0
    int i, j;
    uint8 buf[24]; // max size=24 in m_storageSpaceAllocation, kaomin
    bool retVal = TRUE;

    for (i = 0; i < STORAGE_DEVICE_CELL_END; i++)
    {
        retVal = StorageDevice_read(i, (void *)buf);

        if (1 == m_storageSpaceAllocation[i].size)
        {
            TRACE_DEBUG((0, "Cell-%d: 0x%X", i, buf[0]));
        }
        else if (2 == m_storageSpaceAllocation[i].size)
        {
            TRACE_DEBUG((0, "Cell-%d: 0x%X 0x%X", i, buf[0], buf[1]));
        }
        else if (5 == m_storageSpaceAllocation[i].size)
        {
            TRACE_DEBUG((0, "Cell-%d: 0x%X 0x%X 0x%X 0x%X 0x%X", i, buf[0], buf[1], buf[2], buf[3], buf[4]));
        }
        else
        {
            TRACE_DEBUG((0, "Cell-%d: ", i));
            for (j = 0; j < m_storageSpaceAllocation[i].size; j++)
            {
                TRACE_DEBUG((0, "        0x%X", buf[i]));
            }
        }
    }
#endif
#if 0//def STORAGE_DEBUG_ENABLE
    int i;
    int j;
    uint8 partitionID;
    uint16 size;
    uint8 diskID;
    uint16 strID;
    uint16 address;
    uint8  data;
    LOGICAL_EEPROM_DEVICE device;

    /* Holds the current Debug mask */
    uint32 mask;

    /* Allow INFO traces in any case and keep the current mask */
    mask = Debug_register(DEBUG_INFO);

    /** Print all cells  in the selected disk */
    for (i = 0; i < STORAGE_DEVICE_CELL_END; i++)
    {
        /** Saves the partition , disk ID ,string ID and size*/
        strID = m_storageSpaceAllocation[i].stringNameID;
        partitionID = m_storageSpaceAllocation[i].partition;
        diskID = m_partitionSpaceAllocation[partitionID].disk;

        /** Checks if the cell belonging to the selcted disk */
        if (disk == diskID)
        {
            TRACE_INFO((strID, ""));
            TRACE_INFO((STR_ID_NULL, ""));

            retVal &= StorageDevice_calcCellAddress((STORAGE_DEVICE_CELL)i,
                                                    &device,
                                                    &size,
                                                    &address);

            for (j = 0; j < size; j ++)
            {
                retVal &= EepromDriver_read(device,
                                            (address + j),
                                            &data,
                                            1);

                TRACE_INFO((STR_ID_NULL, "%X ", data));
            }

            TRACE_INFO((STR_ID_SEP_LINE, ""));
        }
    }

    /* Restore the original Debug mask ststate */
    Debug_unregister(DEBUG_INFO);
    Debug_register(mask);
#endif

    return retVal;
}

/*-----------------------------private----------------------------------------*/
static bool StorageDevice_calcCellAddress(STORAGE_DEVICE_CELL cell,
        LOGICAL_EEPROM_DEVICE *device,
        uint16 *size,
        uint16 *addr)
{
    uint16 address = 0;
    uint8 i;

    /* Calculate the offset of the cell from the beginning of the
       storage partition */
    for (i = 0; i < cell; i++)
    {
        /** Check whether the partitions are equal */
        if (m_storageSpaceAllocation[cell].partition ==
                m_storageSpaceAllocation[i].partition)
        {
            address += m_storageSpaceAllocation[i].size;
        }
    }

    /** Calculate the ofset of the cell from the beginning of the storage disk*/
    address += StorageDevice_calcPartitionAddress(
                   (STORAGE_PARTITION)m_storageSpaceAllocation[cell].partition);

    /** Returns the calculated values*/
    *addr = address;
    *size = m_storageSpaceAllocation[cell].size;
    *device = (LOGICAL_EEPROM_DEVICE)(LOGICAL_EEPROM_DEVICE_BLOCK0 +
                                      m_partitionSpaceAllocation[m_storageSpaceAllocation[cell].partition].disk);

    return TRUE;
}


#if ( configSTORAGE_UPDATE_CHECKSUM == 1 ) 
static bool StorageDevice_updateChecksum(STORAGE_DEVICE_CELL cell, void *buff)
{
    int i;
    uint16 addr;
    uint16 size;
    uint16 checksumAddress;
    uint8 checksum;
    uint8 data;
    LOGICAL_EEPROM_DEVICE device;
    bool retVal = TRUE;

    /** Calculate the real address in the logical Eeprom*/
    retVal &= StorageDevice_calcCellAddress(cell, &device, &size, &addr);

    /** Calculate the checksum address */
    checksumAddress = StorageDevice_calcPartitionAddress(
                          (STORAGE_PARTITION)m_storageSpaceAllocation[cell].partition);

    checksumAddress += m_partitionSpaceAllocation[
                           m_storageSpaceAllocation[cell].partition].size - 1;

    /* Reading the checksum from the Eeprom */
    retVal &= EepromDriver_read(device, checksumAddress, &checksum, 1);

    /* Invert the checksum */
    checksum = -checksum;

    /** Runs over the data and calculate the new checksum */
    for (i = 0; i < size; i++)
    {
        /* Reading the on byte of data from the Eeprom */
        retVal &= EepromDriver_read(device, (addr + i), &data, 1);

        /** Caculate the new checksum */
        checksum = checksum - data + *((uint8 *)buff + i);
    }

    /* Invert the checksum */
    checksum = -checksum;

    /* Writing the new checksum Eeprom */
    retVal &= EepromDriver_write(device, checksumAddress, &checksum, 1, TRUE);

    return retVal;
}
#endif 

static bool StorageDevice_initializePartition(STORAGE_PARTITION partition)
{
    bool retVal = TRUE;
    uint16 address;
    int i;
    uint8 data = PARTITION_ERASE_VALUE;

    /** Calculate the partition offset from the begining of the disk */
    address = StorageDevice_calcPartitionAddress(partition);

    /** Writing the ERASE value to all partition */

    for (i = 0; i < (m_partitionSpaceAllocation[partition].size - 2); i++)
    {
        /** Currently don't erase the data on the partition */
        /**
        retVal &= EepromDriver_write(
                (LOGICAL_EEPROM_DEVICE)(LOGICAL_EEPROM_DEVICE_BLOCK0 +
                (int)m_partitionSpaceAllocation[partition].disk),
                (address + i),
                &data,
                sizeof(data),
                TRUE);
                */
    }


    /** Writing the revision number */
    data = m_partitionSpaceAllocation[partition].revision;
    retVal &= EepromDriver_write(
                  (LOGICAL_EEPROM_DEVICE)(LOGICAL_EEPROM_DEVICE_BLOCK0 +
                                          (int)m_partitionSpaceAllocation[partition].disk),
                  (address + i),
                  &data,
                  sizeof(data),
                  TRUE);

    /** Writing the new checksum */
    retVal &= StorageDevice_checkPartitionChecksum(partition, TRUE);

    return retVal;
}

static bool StorageDevice_checkDiskRevision(STORAGE_DISK disk)
{
    bool retVal = TRUE;
    int j;
    uint16 address;
    uint8  revision;
    int diskRevisionFailed = 0;

    /** The disk revision placed in the last Eeprom byte */
    address = EepromDriver_deviceCapacity(
                  (LOGICAL_EEPROM_DEVICE)
                  ((uint8)LOGICAL_EEPROM_DEVICE_BLOCK0 + (uint8)disk)) - 1;

    /** Retry a few times before initialize all Eeprom */
    for (j = 0; j < DISK_REVISION_FAIL_RETRY; j++)
    {
        retVal &= EepromDriver_read(
                      (LOGICAL_EEPROM_DEVICE)
                      ((uint8)LOGICAL_EEPROM_DEVICE_BLOCK0 + (uint8)disk),
                      address,
                      &revision,
                      sizeof(revision));

        /** Checks if the disk revision match */
        if ((revision != STORAGE_DEVICE_DISK_VERSION) && (retVal == TRUE))
        {
            diskRevisionFailed++;
        }
    }

    /** Only in case of all retrys was failed initialize the Eeprom */
    if (diskRevisionFailed == DISK_REVISION_FAIL_RETRY)
    {
        /** Writing the new disk revision */
        revision = STORAGE_DEVICE_DISK_VERSION;
        retVal &= EepromDriver_write(
                      (LOGICAL_EEPROM_DEVICE)
                      ((uint8)LOGICAL_EEPROM_DEVICE_BLOCK0 + (uint8)disk),
                      address,
                      &revision,
                      sizeof(revision),
                      TRUE);

        retVal &= FALSE;

        /** initialize all disk partitions */
        for (j = 0; j < STORAGE_PARTITION_LAST; j++)
        {
            if (m_partitionSpaceAllocation[j].disk == disk)
            {
                retVal &= StorageDevice_initializePartition((STORAGE_PARTITION)j);
            }
        }
    }

    return (retVal == TRUE);
}

bool StorageDevice_checkPartitionReversion( STORAGE_PARTITION partition )
{

	bool retVal = TRUE;
    uint16 address;
    int i;
    uint8 data = PARTITION_ERASE_VALUE;
	LOGICAL_EEPROM_DEVICE device;

    /** Calculate the partition offset from the begining of the disk */
    address = StorageDevice_calcPartitionAddress(partition);
	i = (m_partitionSpaceAllocation[partition].size - 3);

	device = ( (LOGICAL_EEPROM_DEVICE)(LOGICAL_EEPROM_DEVICE_BLOCK0 + (int)m_partitionSpaceAllocation[partition].disk) );

	retVal &= EepromDriver_read(device, (address + i), &data, 1 );

	if ( data != m_partitionSpaceAllocation[partition].revision )
	{
		retVal &= FALSE;
	}

	if ( retVal == FALSE )
	{
		/** Writing the revision number */
	    data = m_partitionSpaceAllocation[partition].revision;
	    retVal &= EepromDriver_write(
	                  device,
	                  (address + i),
	                  &data,
	                  sizeof(data),
	                  FALSE);
	}

	retVal &= EepromDriver_read(device, (address + i), &data, 1 );

	return retVal;

}


static bool StorageDevice_checkPartitionChecksum(STORAGE_PARTITION partition,
        bool update)
{
    bool retVal = TRUE;
    uint8 i;
    uint8 data;
    LOGICAL_EEPROM_DEVICE device;
    uint16 address;
    uint8 checksum = 0;
    uint16 partitionSize;

    partitionSize = m_partitionSpaceAllocation[partition].size;

    /** sets the logical eeprom device */
    device = LOGICAL_EEPROM_DEVICE_BLOCK0;
    device += (int)m_partitionSpaceAllocation[partition].disk;

    /** Calculate the partition offset from the begining of the disk */
    address = StorageDevice_calcPartitionAddress(partition);

    /** Calculate the checksum */
    for (i = 0 ; i < (partitionSize - (uint8)update); i++)
    {
        /* Read data from the EEPROM */
        retVal &= EepromDriver_read(device, address + i, &data, sizeof(data));

        /* Calc checksum */
        checksum += data;
    }

    /** Checking if update is required */
    if (update == TRUE)
    {
        /** Calculate the new checksum in case of update required */
        checksum = -checksum;

        /** Writing the new checksum */
        retVal &= EepromDriver_write(device, address + i, &checksum, sizeof(data), TRUE);
    }
    else
    {
        /** Checks whether the checksum is valid, 0==VALID, Otherwise==FALSE*/
        retVal &= (checksum == 0) ? TRUE : FALSE;
    }

    return retVal;
}


static uint16 StorageDevice_calcPartitionAddress(STORAGE_PARTITION partition)
{
    int i;
    uint16 address = 0;
    uint8 disk;

    /** Svaes the partition's disk */
    disk  = m_partitionSpaceAllocation[partition].disk;

    for (i = 0; i < partition; i++)
    {
        if (m_partitionSpaceAllocation[i].disk == disk)
        {
            address += m_partitionSpaceAllocation[i].size;
        }
    }

    return address;
}

