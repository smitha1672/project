#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"

#include "SPILowLevel.h"
#include "I2C1LowLevel.h"
#include "I2C2LowLevel.h"
#include "StorageDeviceDriver.h"
#include "GPIOMiddleLevel.h"
#include "EepromDriver.h"
#include "AudioDeviceManager.h"
#include "CommandsManager.h"
#if ( configSII_DEV_953x_PORTING == 1 )
#include "HdmiDeviceManager.h"
#endif


//________________________________________________________________________
#define configCTRL_CMD_STORAGE_MODIFY 0
#define configCTRL_CMD_STORAGE_PRINT 0
#define configCTRL_CMD_STORAGE_DEBUG_PRINT_MASK 0

extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;
extern HDMI_DEVICE_MANAGER_OBJECT *pHDMI_DM_ObjCtrl;

//________________________________________________________________________
/* Params for Toggle GPIO Command */
typedef struct Command_GpioSetParams { 
	VirtualPin gpio;
	uint8 value; /*0: clear; 1: set */
} Command_GpioSetParams;


/*----------------------- Private Member Definitions ------------------------ */

/**
 * @defgroup ControlCmdsMgr_Private Advance Control Commands Manager - Private
 * @ingroup ControlCmdsMgr
 *
 * These are the private members of @ref ControlCmdsMgr.
 *
 */
/*@{*/

/**
 * Handles an I2C write command.
 *
 * @param params        The parameters received.
 */
static void ControlCommandsManager_I2CWriteCommand(
    Command_I2CWriteParams *params);

/**
 * Handles an I2C read command.
 *
 * @param params        The parameters received.
 */
static void ControlCommandsManager_I2CReadCommand(
    Command_I2CReadParams *params);

/*
 * Modify one cell in a selected STORAGE
 */
#if ( configCTRL_CMD_STORAGE_MODIFY == 1 )
static void ControlCommandsManager_storageModify(
    Command_StorageModifyParams *params);
#endif 

/*
 * Print the Storage data
 */
#if ( configCTRL_CMD_STORAGE_PRINT == 1 )
static void ControlCommandsManager_storagePrint(
    Command_StoragePrintParams *params);
#endif 


static void ControlCommandsManager_GpioOutputCtrl( Command_GpioSetParams *params );

static void ControlCommandsManager_EEPROM_Write( Command_EepromModifyParams *params );


static void ControlCommandsManager_handleCommand(
    uint8 command,
    void *params
);

const CTRL_CMD_HANDLE_OBJECT CtrlCommand_api =
{
	ControlCommandsManager_handleCommand,
};
const CTRL_CMD_HANDLE_OBJECT *pCtrlCmd_ObjCtrl = &CtrlCommand_api;



/*____________________________________________________________________________________________________*/

/** For the Mac Read command - maximal length of expected data */
#define MAX_MAC_READ_COMMAND_DATA 100

/*@}*/

/*----------------------------- Implementations ----------------------------- */
#if ( configCTRL_CMD_STORAGE_PRINT == 1 )
static void ControlCommandsManager_debugPrintMask(uint32 mask)
{
#ifdef PRINT_LEVEL_ENABLE /*{*/
    uint32 oldMask, newMask, currMask;
    uint8 buff[5];

    /* Call register func in order to get the current mask */
    oldMask = Debug_register(0x0);

    /* Checks whether the current requested mask is already set or clear.
       - In case it is set, the current requested mask is cleared.
       - In case it is cleared, the current requested mask is set.
    */
    if (((oldMask & mask) != 0))
    {
        /* In case of clear default setup, we would like to clear all mask, so
           no printout will be active.
        */
        if (mask == DEBUG_TRACE_DEFAULT)
        {
            mask = 0xFFFFFFFF;
        }

        Debug_unregister(mask);
    }
    else
    {
        Debug_register(mask);
    }

    newMask = Debug_register(0x0);

    /* Set the buffer with the new MASK */
    buff[0] = DEBUG_PRINTOUT_MASK_SIGNATURE;
    buff[1] = (newMask & 0xFF000000) >> 24;
    buff[2] = (newMask & 0x00FF0000) >> 16;
    buff[3] = (newMask & 0x0000FF00) >> 8;
    buff[4] = (newMask & 0x000000FF);

#ifdef AMIMON
    StorageDevice_write(STORAGE_DEVICE_CELL_PRINTOUT_MASK, &buff);
#endif

    /* Enable Debug printouts in order to see the below printout in any case */
    currMask = Debug_register(DEBUG_DEBUG);

    TRACE_DEBUG((STR_ID_NULL,
                 "Print mask OLD: 0x%X NEW: 0x%X",
                 oldMask,
                 newMask));

    /* Restore the original Debug mask ststate */
    Debug_unregister(DEBUG_DEBUG);
    Debug_register(currMask);
#endif /*PRINT_LEVEL_ENABLE @}*/
}
#endif 

static void ControlCommandsManager_handleCommand(
    uint8 command,
    void *params
)
{
  
    switch (command)
    {
        case CTRL_OPCODE_I2C_WRITE :
		{
            ControlCommandsManager_I2CWriteCommand(
                (Command_I2CWriteParams *)params
            );
        }	
            break;

        case CTRL_OPCODE_I2C_READ:
            ControlCommandsManager_I2CReadCommand(
                (Command_I2CReadParams *)params
            );
            break;

		case CTRL_OPCODE_DSP_SPI_RELEASE:
		{
			SPILowLevel_release_SPI();
		}
			break;

		case CTRL_OPCODE_GPIO_OUTPUT_CTRL:
		{
			ControlCommandsManager_GpioOutputCtrl( (Command_GpioSetParams *)params );
		}
			break;

		case CTRL_OPCODE_EEPROM_WRITE:
		{
			ControlCommandsManager_EEPROM_Write( (Command_EepromModifyParams *)params );
		}
			break;
       
		case CTRL_OPCODE_CS8422_RESET:
		{
            pAudLowLevel_ObjCtrl->RstSRC();
		}
			break;
            
#if ( configDEG_NVRAM_EDID == 1 )
		case CTRL_OPCODE_SIL9533_READ_SRAM:
	    {
            xHMISystemParams SystemParms;

            SystemParms.sys_event = SYS_EVENT_HDMI_READ_SRAM;
            pHDMI_DM_ObjCtrl->SendEvent( &SystemParms );
		}
			break;


        case CTRL_OPCODE_SIL9533_READ_NVRAM:
        {
            xHMISystemParams SystemParms;

            SystemParms.sys_event = SYS_EVENT_HDMI_READ_NVRAM;
            pHDMI_DM_ObjCtrl->SendEvent( &SystemParms );
        }
            break;

        case CTRL_OPCODE_SIL9533_NVRAM_INITIAL:
        {
            xHMISystemParams SystemParms;

            TRACE_DEBUG((0, "Reload SIL9533 NVRAM"));

            SystemParms.sys_event = SYS_EVENT_HDMI_NVRAM_INITIAL;
            pHDMI_DM_ObjCtrl->SendEvent( &SystemParms );
        }
            break;
#endif            
            
       
        default:
            break;
    }
}

static void ControlCommandsManager_I2CWriteCommand(
    Command_I2CWriteParams *params
)
{

	if ( params == NULL )
		return;

	if ( params->whichI2C > 1 )
	{
		TRACE_ERROR((0, "I2C BUS NUMBER is over 1"));
		return;
	}


	TRACE_DEBUG((0, "I2C BUS %d write ", params->whichI2C ));
	TRACE_DEBUG((0, "I2C BUS %d slave address = 0x%X", params->whichI2C, params->slaveAddress ));
	TRACE_DEBUG((0, "I2C BUS %d inner address = 0x%X", params->whichI2C, params->innerAddress ));
	TRACE_DEBUG((0, "I2C BUS %d write length = 0x%X", params->whichI2C, params->counter ));
	TRACE_DEBUG((0, "I2C BUS %d write data = 0x%X", params->whichI2C, params->value[0] ));

	
	if( params->whichI2C == 0 )
	{
		if( I2C_writeSlave(params->slaveAddress, params->innerAddress, params->value, params->counter, FALSE, FALSE ) == FALSE )
		{
			TRACE_ERROR((0, "I2C BUS %d slave address 0x%X write 0x%X failure !!", params->whichI2C, params->slaveAddress, params->innerAddress));
                return;
            }
	}
	else if ( params->whichI2C == 1 )
	{
		if( I2C2_writeSlave(params->slaveAddress, params->innerAddress, params->value, params->counter, FALSE ) == FALSE )
		{
			TRACE_ERROR((0, "I2C BUS %d slave address 0x%X write 0x%X failure !!", params->whichI2C, params->slaveAddress, params->innerAddress));
			return;
		}
	}

}

static void ControlCommandsManager_I2CReadCommand(
    Command_I2CReadParams *params
)
{
	uint8 buffer[32] = {0}; //*value;
	uint8 i = 0;

	if ( params == NULL )
		return;

	
	if ( params->whichI2C > 1 )
	{
		TRACE_ERROR((0, "I2C BUS NUMBER is over 1"));
		return;
	}

	if ( params->counter > 32 )
	{
		TRACE_ERROR((0, "I2C read length is over 32 bytes "));
		return;	
	}

	TRACE_DEBUG((0, "I2C BUS %d read ", params->whichI2C ));
	TRACE_DEBUG((0, "I2C BUS %d slave address = 0x%X", params->whichI2C, params->slaveAddress ));
	TRACE_DEBUG((0, "I2C BUS %d inner address = 0x%X", params->whichI2C, params->innerAddress ));
	TRACE_DEBUG((0, "I2C BUS %d read length = 0x%X", params->whichI2C, params->counter ));


	if( params->whichI2C == 0 )
	{
                if( I2C_readSlave(params->slaveAddress, params->innerAddress, buffer, params->counter, FALSE) == FALSE )
                {
                    TRACE_ERROR((0, "I2C BUS %d slave address 0x%X read 0x%X failure !!", params->whichI2C, params->slaveAddress, params->innerAddress));
                    return;
                }
	}
	else if ( params->whichI2C == 1 )
	{
		if ( I2C2_readSlave(params->slaveAddress, params->innerAddress, buffer, params->counter, FALSE) == FALSE )
		{
			TRACE_ERROR((0, "I2C BUS %d slave address 0x%X read 0x%X failure !!", params->whichI2C, params->slaveAddress, params->innerAddress));
			return; 
		}
	}

	for( i = 0; i < params->counter; i++ )
	{
		TRACE_DEBUG((0, "data[%d] = 0x%X", i, buffer[i]));
	}

}


#if ( configCTRL_CMD_STORAGE_MODIFY == 1 )
static void ControlCommandsManager_storageModify(
    Command_StorageModifyParams *params)
{
    if (StorageDevice_write((STORAGE_DEVICE_CELL)params->CellID,
                            (void *)params->data))
    {
        TRACE_INFO((STR_ID_NULL, "Cell %d, modifyed" , params->CellID));
    }
    else
    {
        TRACE_INFO((STR_ID_NULL, "Cell modification failed "));
    }

}
#endif 

#if ( configCTRL_CMD_STORAGE_PRINT == 1 )
static void ControlCommandsManager_storagePrint(
    Command_StoragePrintParams *params)
{
    params->DiskID = Swap16(params->DiskID);

    StorageDevice_print(params->DiskID);
    if (params->DiskID == 0xFF)
    {
        StorageDevice_eraseAll(params->DiskID);
        TRACE_DEBUG((0, "Reset storage as ..."));
        StorageDevice_print(params->DiskID);
    }

    StorageDevice_print((STORAGE_DISK)params->DiskID);
}
#endif 

static void ControlCommandsManager_GpioOutputCtrl( Command_GpioSetParams *params )
{
	if ( params->value )
	{
		GPIOMiddleLevel_Set( params->gpio );

		TRACE_DEBUG((0, "set virtual pin %d as 1 ", params->gpio ));
	}
	else if ( !params->value )
	{
		GPIOMiddleLevel_Clr( params->gpio );

		TRACE_DEBUG((0, "clr virtual pin %d as 0 ", params->gpio ));
	}
}

static void ControlCommandsManager_EEPROM_Write( Command_EepromModifyParams *params )
{
	bool ret = TRUE;
	
	if ( params == NULL )
		return;

	TRACE_DEBUG((0, "eeprom address = 0x%X", params->address ));
	TRACE_DEBUG((0, "eeprom write size = %d", params->size ));

	/*Smith modifies: Fix warning message*/
	ret = EepromDriver_write(LOGICAL_EEPROM_DEVICE_BLOCK0,
		params->address,
		params->value,
		params->size,
		FALSE);

	ASSERT_PRINT(ret);        
}

