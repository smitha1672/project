#ifndef __STORAGE_DEVICE_DRIVER_H__
#define __STORAGE_DEVICE_DRIVER_H__

#include "CardLowlevel.h"
#include "api_typedef.h"

typedef enum
{
    STORAGE_PARTITION_0,
    STORAGE_PARTITION_1,
    STORAGE_PARTITION_2,
    STORAGE_PARTITION_3,
    STORAGE_PARTITION_4,
    STORAGE_PARTITION_LAST
} STORAGE_PARTITION;

typedef enum
{
	STORAGE_DISK_0, 
	STORAGE_DISK_LAST
} STORAGE_DISK;

typedef struct
{    
    uint16 size;
    uint8  revision;
    uint8  disk;
} storagePartitionType;

typedef struct
{    
	uint8 size;
	uint8 partition;
	STORAGE_DEVICE_CELL id;
} storageCellType;



/* Defines the Storage Device Disk version */
#define STORAGE_DEVICE_DISK_VERSION              1

/**
 * Define the default erase value
 */
#define PARTITION_ERASE_VALUE 0xFF

/**
 * Define the number of retry to read the disk revision before fail
 */
#define DISK_REVISION_FAIL_RETRY 3

/*
 * Storage device initalize, Scan the storage and checks the chekcsum & revision.
 * In case of fail init the storage device;
 *
 * @return  TRUE in case the Sorage Device checksum and revision is correct. 
 *          FALSE, otherwise.
 */
bool StorageDevice_initialize(void);

/*
 * Read a buffer from a specific cell in the storage device.
 *
 * @param   cell    A specific location and size in the storage device where
 *                  the data is going to be read from.
 * @param   buff    A pointer to a container to place the data was read from
 *                  the storage.
 *
 * @return  TRUE in case the Sorage Device checksum is correct. 
 *          FALSE, otherwise.
 */
bool StorageDevice_read(STORAGE_DEVICE_CELL cell, void* buff);

/*
 * Write a buffer to a specific cell in the storage device.
 *
 * @param   cell    A specific location and size in the storage device where
 *                  the buffer is going to be written.
 * @param   buff    A pointer to a buffer of data to be written to the storage.
 *
 * @return  TRUE in case the write operation succeeded. FALSE, otherwise.
 */
bool StorageDevice_write(STORAGE_DEVICE_CELL cell, void* buff);


/*
 * Prints the content of the storage disk
 *
 * @param   disk Storage Disk ID
 *
 * @return  TRUE in case of print succeeded.
 *          FALSE, otherwise.
 */
bool StorageDevice_print(STORAGE_DISK disk);

/*----------------------AMTRAN IMPLEMEMTED---------------------------*/
bool StorageDevice_eraseDeviceCell(STORAGE_DEVICE_CELL cell);

bool StorageDevice_checkPartitionReversion( STORAGE_PARTITION partition );


#endif

