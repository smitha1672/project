#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"
#include "device_config.h"
#include "StorageDeviceDriver.h"
#include "StorageDeviceManager.h"
#include "IRCmdHandler.h"


//___________________________________________________________________________________________________

#define PARAMETER_SIZER (sizeof(xHMISystemParams)/sizeof(uint8))

//___________________________________________________________________________________________________

static int8 StorageDeiveManager_initialize( xHMISystemParams *params );

static bool StorageDeiveManager_InstrSender( void *params );

static bool StorageDeviceManager_read( STORAGE_DEVICE_CELL cell, void *buff );

static bool StorageDeviceManager_write( STORAGE_DEVICE_CELL cell, void *buff );


//___________________________________________________________________________________________________
extern IR_CMD_PROGRAM_OBJECT *pIR_PrgRemote_ObjCtrl;

const STORAGE_DEVICE_MANAGER_OBJECT STORAGE_OBJ = 
{
    StorageDeiveManager_initialize,
    StorageDeiveManager_InstrSender,
    StorageDeviceManager_read,
    StorageDeviceManager_write,
};
const STORAGE_DEVICE_MANAGER_OBJECT *pSDM_ObjCtrl = &STORAGE_OBJ;

const static xHMISystemParams DefaultSystemParams = { 
    SYS_EVENT_NULL,
    MODE_USER,    
    DEFAULT_APD,
    DEFAULT_INPUT_SRC,
    DEFAULT_LAST_INPUT_SRC, // Added by david, fix lost value.
    DEFAULT_MUTE_STATE, /*mute*/
    DEFAULT_MASTER_GAIN, /*master gain*/
    DEFAULT_BASS_GAIN,
    DEFAULT_TREBLE_GAIN,
    DEFAULT_SUB_GAIN,
    DEFAULT_CENTER_GAIN,
    DEFAULT_REAR_GAIN,
    DEFAULT_BALANCE,
    DEFAULT_SRS_TRUVOL,
    DEFAULT_SRS_TSHD,
    DEFAULT_NIGHT_MODE,
    DEFAULT_AV_DELAY,
    DEFAULT_VIZIO_RMT,
    DEFAULT_AID,
#if ( configSII_DEV_953x_PORTING == 1)
    DEFAULT_CEC_SAC,
    DEFAULT_CEC_ARC,
    DEFAULT_CEC_ARC_Tx
#endif
};

//____________________________________________________________________________________________________
static bool StorageDeiveManager_InstrSender( void *params ) 
{
    bool ret = TRUE;

#if( configAPP_STORAGE_MANAGER == 1)
    if ( params == NULL )
        return (ret&FALSE);    

    
    xHMISystemParams* pSystemParms = (xHMISystemParams*)params;

    switch( pSystemParms->sys_event )
    {
        case SYS_EVENT_DEV_CONFIG:
        case SYS_EVENT_AUTO_SEL_SRC:
        case SYS_EVENT_SEL_SRC:
        case SYS_EVENT_SAVE_USER_SETTING:
        case SYS_EVENT_VOLUME_SET:
        case SYS_EVENT_BASS_SET:
        case SYS_EVENT_TREBLE_SET:
        case SYS_EVENT_SUB_SET:
        case SYS_EVENT_CENTER_SET:
        case SYS_EVENT_LSRS_SET:
        case SYS_EVENT_BALANCE_SET:
        case SYS_EVENT_SRS_TRUVOL_TSHD_SET:
        case SYS_EVENT_SRS_TRUVOL_SET:
        case SYS_EVENT_SRS_TSHD_SET:
        case SYS_EVENT_NIGHT_MODE_SET:
        case SYS_EVENT_AV_DELAY_SET:
        {
            ret = pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_USER_PARAMETERS ,(uint8*)pSystemParms );
        }
            break;

        default:
        {
            ret &=FALSE;
            TRACE_DEBUG((0, "Unsupport store parameter system event = %d", pSystemParms->sys_event ));
        }
            break;
    }        
#endif 

    return ret;

}

static int8 StorageDeiveManager_initialize( xHMISystemParams *params )
{
    int8 ret = 0;

#if( configAPP_STORAGE_MANAGER == 1)
    ret = StorageDevice_initialize( );
    if ( ret == FALSE )
    {
        ret = -1;
    }

    if ( StorageDevice_checkPartitionReversion( STORAGE_PARTITION_1 ) == FALSE )
    {
        if ( StorageDevice_write( STORAGE_DEVICE_CELL_USER_PARAMETERS ,(uint8*)&DefaultSystemParams ) == FALSE )
        {
            ret = -2;
        }
    }

    if ( StorageDevice_read( STORAGE_DEVICE_CELL_USER_PARAMETERS, params ) == FALSE )
    {
        ret = -3;
    }
#endif  

    return ret;
}

static bool StorageDeviceManager_read( STORAGE_DEVICE_CELL cell, void *buff )
{
    bool retVal = TRUE;

#if( configAPP_STORAGE_MANAGER == 1)
    retVal = StorageDevice_read( cell, buff );
#endif 

    return retVal;    
}

static bool StorageDeviceManager_write( STORAGE_DEVICE_CELL cell, void *buff )
{
    bool retVal = TRUE;

#if( configAPP_STORAGE_MANAGER == 1)
    retVal = StorageDevice_write( cell, buff );
#endif 

    return retVal;    
}

