#ifndef __APPLICATION_COMMAND_H__
#define __APPLICATION_COMMAND_H__

#include "Defs.h"

typedef enum{
  AMT_SOURCE_AUX1	= 0,
  AMT_SOURCE_AUX2,
  AMT_SOURCE_COX,
  AMT_SOURCE_SPDIF,
  AMT_SOURCE_USB,
  AMT_SOURCE_BT,
  AMT_SOURCE_HDMI,
  AMT_SOURCE_HDMI_ARC,
  AMT_SOURCE_TV_WIFI,
  AMT_SOURCE_NULL
}AMT_SOURCE;

typedef enum {
	FUC_COMMAND_NULL = 0,

    // !Control command @{
    DSP_CIRRUS_TOOL_MODE,
    DSP_FLASH_UPDATE_MODE,
    DSP_MASTER_BOOT_MODE,
	//!@}
    FLASH_test,
    FLASH_SET_BACK_REG,
    FLASH_DESET_BACK_REG,
    //!    Audio parameter@{
    AUDIO_SET_INPUT,
    AUDIO_SET_VOL,
    AUDIO_SET_LR_VOL,
    AUDIO_SET_SUB_VOL,
    AUDIO_SET_TREB_VOL,
    AUDIO_SET_BASS_VOL,
    AUDIO_SET_SMART_VOL,    
    AUDIO_SET_SOUND_SURROUND_ENABLE,
    AUDIO_SET_DIALOG_LEVEL,
    AUDIO_SET_REAR_VOL,
    AUDIO_SET_BALANCE,
    AUDIO_SET_MUTE,
    AUDIO_SET_MAXXLEVELER,
    AUDIO_SET_MAXX3D,
    AUDIO_SET_MOVIE,
    AUDIO_SET_CSII,
    AUDIO_SET_SRS_TRU_VOLUME,
    AUDIO_SET_TVHD_MC,
    AUDIO_SET_SRS_TSHD,
    AUDIO_SET_NIGHT_MODE,   // JerryLin add for Night mode
    AUDIO_FORMAT_CHANGE,
    AUDIO_FORMAT_DOLBY,
    AUDIO_DATA_TYPE_POLL_CTRL,
    AUDIO_SET_CHANNEL,
    AUDIO_CS8422_RESET,
    AUDIO_CS5346_RESET,
    AUDIO_SET_PROCESS_MODE,
		
	//!@}


	//!	RCU function	@{
	POWER_KEY, 
	AUDIO_CTRL_VOL_UP			,
	AUDIO_CTRL_VOL_DOWN		,
	AUDIO_CTRL_BASS_UP		,
	AUDIO_CTRL_BASS_DOWN	,
	AUDIO_CTRL_TREBLE_UP		,
	AUDIO_CTRL_TREBLE_DOWN	,
	AUDIO_CTRL_SUB_UP			,
	AUDIO_CTRL_SUB_DOWN		,
    AUDIO_CTRL_DIALOG_UP    ,
    AUDIO_CTRL_DIALOG_DOWN  ,
	AUDIO_CTRL_REAR_UP		,
	AUDIO_CTRL_REAR_DOWN	,
	AUDIO_CTRL_BALANCE_UP		,
	AUDIO_CTRL_BALANCE_DOWN		,
	AUDIO_CTRL_AUDIO_VOLUME_STATUS,
	AUDIO_CTRL_INPUT			,	
	AUDIO_CTRL_AUX1,
	AUDIO_CTRL_AUX2,	
	AUDIO_CTRL_COX,
	AUDIO_CTRL_OPT,		
	AUDIO_CTRL_BLUETOOTH,
	AUDIO_CTRL_MUTE			,
	AUDIO_CTRL_SRS_TRUVOL	,
	AUDIO_CTRL_SRS_TSHD		,
	AUDIO_CTRL_CSII			,
    AUDIO_CTRL_NIGHT_MODE, 
	AUDIO_CTRL_BASS_MGR	,
	AUDIO_CTRL_Center_Switch,
	AUDIO_CTRL_SOUND_SURROUND_ENABLE,

	//!	System Power Control @{
	SYSTEM_POWER_UP, 
	SYSTEM_POWER_DOWN,
	SYSTEM_SET_RESET,
	SYSTEM_CLR_RESET,
	SYSTEM_AUTO_POWER_DOWN,
	SYSTEM_SYS_RESET,

	//!	@}

	UNDEFINED_COMMAND                   = 0xFF
} Command;

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

typedef struct Command_IRRawDataParams {
    unsigned* timeGaps;
    int count;
} Command_IRRawDataParams;

typedef struct Command_FrequencyChangeParams { /* Params for FREQUENCY_CHANGE */
    uint16 newFrequency;   /* New Frequency In MHz */
} Command_FrequencyChangeParams;

typedef struct Command_PowerChangeParams { /* Params for RF_POWER_CHANGE */
    uint16 newPower;       /* VGA */
} Command_PowerChangeParams;

typedef struct Command_VideoResolutionParams { /* Params for VIDEO_INPUT_SET_RESOLUTION */
    uint16 resolution;     /* Resolution */
} Command_VideoResolutionParams;

typedef struct Command_MacWriteParams { /* Params for MAC_WRITE */
    uint16 address;        /* Mac address to write to */
    uint16 length;         /* Length of data */
    byte data[1];          /* Data to send */
} Command_MacWriteParams;

typedef struct Command_MacReadParams { /* Params for MAC_READ */
    uint16 address;        /* Mac address to write to */
    uint16 length;         /* Length of data to read */
} Command_MacReadParams;

typedef struct Command_EepromPrint { /* Params for EEPROM Print */
    uint16  device;          /* EEPROM device ID */
    uint16 address;        /*  address to print from */
    uint16 length;         /* Length of data to print */
} Command_EepromPrintParams;

typedef struct Command_EepromModify { /* Params for EEPROM Print */
    uint16 device;           /* EEPROM device ID */
    uint16 address;          /* address of modified data */
    uint16 data;             /* The new data to be written */
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

/* Params for ALBATROSS_READ */
typedef struct Command_AlbatrossReadParams { 
    byte address[4];        /* Albatross address to write to */
} Command_AlbatrossReadParams;

/* Params for ALBATROSS_WRITE */
typedef struct Command_AlbatrossWriteParams { 
    byte address[4];        /* Albatross address to write to */
    byte data[4];           /* Data to send */
} Command_AlbatrossWriteParams;

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

/* Params for Toggle GPIO Command */
typedef struct Command_ToggleGpioParams { 
    /* GPIO ID - Dec format */
    uint8 gpio;
    /* 
      Set/clear operation. 
      0 - clear.
      1 - set.
    */
    uint8 state;
    /*
      Set GPIO direction.
      0 - output.
      1 - input.
    */
    uint8 dir;
} Command_ToggleGpioParams;

/* Params for program new RF freq and power */
typedef struct Command_ProgramFreq
{
    uint16  freq;
    uint16  power;
} Command_ProgramFreq;

typedef struct Command_UpdateMacStartParams{ 	 /* Params for UPDATE_MAC_START */
    byte versionMajor;        /* Update's major version */
    byte versionMinor;        /* Update's minor version */
    byte versionBug;          /* Update's bug version */
    byte key[16];             /* Encryption key */
    uint32 macLength;         /* Length of the mac code */
    uint32 macCrc;            /* CRC of the mac code */
} Command_UpdateMacStartParams;

typedef struct Command_UpdateMacShahafStartParams{ 
                              /* Params for UPDATE_MAC_START */
    byte versionMajor;        /* Update's major version */
    byte versionMinor;        /* Update's minor version */
    byte versionBug;          /* Update's bug version */
    uint32 transferAddress;   /* address to wrtie the data  */
    uint32 macLength;         /* Length of byte*/
    uint8 sector;            /* sector type*/
} Command_UpdateMacShahafStartParams;

typedef struct Command_HandleRegistrationRequest {  /* Params for HANDLE_REGISTRATION_REQUEST */
    byte *macID;          /* The other side's mac id */
    char *macStr;         /* The other side's identification string. NULL Terminated */
} Command_HandleRegistrationRequest;


typedef struct Command_CS49xxxxParams { /* Params for DSP SPI Write\Read */
	byte	command[4];
	byte	value[4];
} Command_CS49xxxxParams;


#endif /* __APPLICATION_COMMAND_H__ */
