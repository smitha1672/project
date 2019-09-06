#ifndef __APPLICATION_COMMAND_H__
#define __APPLICATION_COMMAND_H__

#include "Defs.h"

typedef struct Command_I2CWriteParams {  /* Params for I2C_WRITE */
    byte whichI2C;          /* I2C - 1 or 2 */
    byte slaveAddress;      /* I2C Address */
    byte innerAddress;      /* Reg Address */
    byte counter;
    byte value[128];             /* Value */
} Command_I2CWriteParams;

typedef struct Command_I2CReadParams {   /* Params for I2C_READ */
    byte whichI2C;          /* I2C - 1 or 2 */
    byte slaveAddress;      /* I2C Address */
    byte innerAddress;      /* Reg Address */
    byte counter;
} Command_I2CReadParams;


typedef struct Command_EepromPrint { /* Params for EEPROM Print */
    uint16  device;          /* EEPROM device ID */
    uint16 address;        /*  address to print from */
    uint16 length;         /* Length of data to print */
} Command_EepromPrintParams;

typedef struct Command_EepromModify { /* Params for EEPROM Print */
    uint16 address;          /* address of modified data */
	uint16 size;
    byte value[128];             /* The new data to be written */
} Command_EepromModifyParams;

typedef struct Command_EepromErase { /* Params for EEPROM erase */
    uint8  device;          /* EEPROM device ID */
} Command_EepromEraseParams;

typedef struct Command_StorageModifyParams { /* Params for Storage Midify Cell */
    uint16 CellID;        /*  Storage Cell ID*/
    byte data[24];          /* Data to send */
} Command_StorageModifyParams;

typedef struct Command_StoragePrintParams { /* Params for Storage Print */
    uint16 DiskID;        /*  Storage disk ID*/
} Command_StoragePrintParams;

typedef struct Command_EEDIDParams { /* Params for EEDID Write\Read */
	/*
		block = 1: byte0 - byte 127
		block = 2: byte128 - byte 255
	*/
	uint8	block;
	uint8	data[256];
} Command_EEDIDParams;


/* Params for CEC Test Command */
typedef struct Command_CecParams { 

	/* 
		len:
		length = header + opcode + params
	*/
    byte len;

    /*  header:
    		bit7 - bit4: Initiator
    		bit3 - bit0: Destination

	*/
    byte header;


    /* CEC Opcode - HEX format */
    byte opcode;

	
    /* Opcode params - HEX format*/
    byte params[14];	/*Parameters maximum is 14 bytes for <Vendor Specific RC Code>*/
} Command_CecParams;

#endif /* __APPLICATION_COMMAND_H__ */
