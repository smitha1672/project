#include "Defs.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "api_typedef.h"

#include "usbd_cdc_core.h"
#include "AudioSystemHandler.h"
#include "FactoryCommandHandler.h"
#include "StorageDeviceManager.h"
#include "AudioDeviceManager.h"
#include "BTHandler.h"
#include "RFHandler.h" 
#include "UIDeviceManager.h"
#include "ChannelCheckManager.h"
//_______________________________________________________________
#define configReturnChecksum 1
#define SERVICE_HANLDER_TIME_TICK  TASK_MSEC2TICKS(10)    
#define FC_QUEUE_LENGTH 4
#define FACTORY_PACKAGE_MAX 28 /*header+ command type + data[24] + checksum*/

#define FAC_CMD_TYPE_SET 0x63
#define FAC_CMD_TYPE_GET 0x73    

#define FAC_ACK_STATUS_POS 5
#define FAC_ACK_SUCCESS    0xE0
#define FAC_ACK_CHECKSUM_NG    0xE1
#define FAC_ACK_TIMEOUT    0xE2
#define FAC_ACK_INVALID_CMD_TYPE    0xE3
#define FAC_ACK_INVALID_FUNCTION_CODE    0xE4
#define FAC_ACK_OPCODE_OR_OUT_OF_RANG 0xE5

#define FAC_RECV_CMD_TYPE_POS 1
#define FAC_RECV_CMD_LEN_POS 2
#define FAC_RECV_OPCODE_POS 3 
#define FAC_RECV_DATA_POS 5

//_______________________________________________________________
typedef enum
{
    FAC_OPCODE_POWER_STATUS = 0x01,
    FAC_OPCODE_FACTORY_MODE = 0x02,
    FAC_OPCODE_RESET_SETTING = 0x03,
    FAC_OPCODE_SN = 0x04,
    FAC_OPCODE_KP_ADC_VALUE = 0x05,
    FAC_OPCODE_KP_SWITCH = 0x06,
    FAC_OPCODE_SOURCE = 0x07,
    FAC_OPCODE_MUTE = 0x08,
    FAC_OPCODE_VOLUME = 0x09,
    FAC_OPCODE_RF_PAIRING = 0x0A,
    FAC_OPCODE_TREBLE = 0x0B,
    FAC_OPCODE_BASS = 0x0C,
    FAC_OPCODE_BTM_PAIRING = 0x0D,
    FAC_OPCODE_CHANNEL_TEST = 0x0E,
    FAC_OPCODE_AQ_CONTROL = 0x0F,
    FAC_OPCODE_VERSION = 0x11,
    FAC_OPCODE_LED_CONTROL = 0x12,
    FAC_OPCODE_AUDIO_FORMAT = 0x13,
    FAC_OPCODE_BTM_CLEAN_DEVICE_LIST = 0x14,
    FAC_OPCODE_UNSUPPORT
}FAC_OPCODE;
//_______________________________________________________________
#define FAC_BT_PAIRING_VAL 1
//_______________________________________________________________
typedef struct FACTORY_CMD_HANDLE_PARAMETERS
{
    xTaskHandle handle;
    TaskHandleState taskState;
    xSemaphoreParameters SParams;
    xQueueParameters Qget;
    xQueueParameters Qset;
} xFacServiceHandleParams;
//________________________________________________________________
static xFacServiceHandleParams xFacServiceHandler;
static xBTHandleCommand mFacBTHandleCommand;
static xHMISystemParams mFacSystemParms;

static xHMISystemParams mFactoryParams = { 
    SYS_EVENT_NULL,
    MODE_FACTORY,    
    DEFAULT_APD,
    FACTORY_INPUT_SRC,
    DEFAULT_MUTE_STATE, /*mute*/
    FACTORY_MASTER_GAIN, /*master gain*/
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
    FACTORY_AID,
#if ( configSII_DEV_953x_PORTING == 1 )
    DEFAULT_CEC_SAC,
    DEFAULT_CEC_ARC,
    DEFAULT_CEC_ARC_Tx
#endif
};
const xHMISystemParams *pFacParams = &mFactoryParams;


static uint8 FactoryPackage[FACTORY_PACKAGE_MAX]={0xff};
static uint8 *pPackage = FactoryPackage;
static uint8 mChannelTest = 0;
static bool mAQSwitch = TRUE;
//________________________________________________________________
//extern void AudioDeviceManager_lowlevel_AudioRouter( uint8 idx );
extern void AudioDeviceManager_lowlevel_AudioRouter( AudioRouteCtrl idx );

extern void ButtonCmdDispatcher_switchControl( bool val );

extern uint8 ButtonCmdDispatcher_getSwitchStatus( void );

extern uint8 ButtonLowLevel_get_btn_ADC_Value( void );

//________________________________________________________________
static bool FactoryCmdHandler_transfer2Task( void *params ); 

static void FactoryCmdHandler_ackSuccess( uint8 *pdata );

//________________________________________________________________
extern CDC_IF_Prop_TypeDef  APP_FOPS;
extern HMI_FACTORY_OBJECT *pHFS_ObjCtrl;
extern STORAGE_DEVICE_MANAGER_OBJECT *pSDM_ObjCtrl;
extern AUDIO_DEVICE_MANAGER_OBJECT *pADM_ObjCtrl;
extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;
extern BT_HANDLE_OBJECT *pBTHandle_ObjCtrl;
extern RF_HANDLE_OBJECT *pRFHandle_ObjCtrl; 
extern UI_DEV_MANAGER_OBJECT *pUDM_ObjCtrl;
extern CCK_MANAGE_OBJECT *pCCKManage_ObjCtrl;
extern AUDIO_SYSTEM_HANDLER_OBJECT *pASH_ObjCtrl;

const FACTORY_CMD_HANDLE_OBJECT FactoryHandler =
{
    FactoryCmdHandler_transfer2Task,
    FactoryCmdHandler_ackSuccess
};
const FACTORY_CMD_HANDLE_OBJECT *pFMD_ObjCtrl = &FactoryHandler;
//________________________________________________________________

static bool FactoryCmdHandler_transfer2Task( void *params ) 
{
    byte *pSender = (byte*)params;
    
    if ( xQueueSend( xFacServiceHandler.Qget.xQueue, pSender, xFacServiceHandler.Qget.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR((0, "FactoryCmdHandler_AckCommand sends queue failure "));
        return FALSE;
    }
    
    return TRUE;
}

static bool FactoryCmdHandler_getPacket( uint8 *pdata ) 
{
    if ( pdata == NULL )
    {
        TRACE_ERROR((0, " FactoryCmdHandler_GetCommand instruction recevier error !! "));
    }
    
    if ( xFacServiceHandler.Qget.xQueue == NULL )
    {
        return FALSE;
    }

    if ( xQueueReceive( xFacServiceHandler.Qget.xQueue, pdata, xFacServiceHandler.Qget.xBlockTime ) != pdPASS )
    {
        TRACE_ERROR((0, " FactoryCmdHandler_GetCommand receiver queue is failure "));
        return FALSE;
    }

    return TRUE;
}

static uint8 FactoryCmdHandler_CalculateChecksum( uint8 *pdata )
{
    uint8 length = *( pdata + FAC_RECV_CMD_LEN_POS  ); /*length*/
    uint8 i = 0;
    uint8 checksum = 0x00;

    for ( i = 0; i < (length - 1) ; i++ )
    {
        checksum ^= *( pdata + i );
    }
    
    return checksum;
}

static void FactoryCmdHandler_sendPackage( const void *params )
{
    uint8 *pData = ( uint8 *)params;

    if ( pData == NULL )
        return;        

     /*send UART data to upstream*/
    APP_FOPS.pIf_DataTx( pData, *( pData + FAC_RECV_CMD_LEN_POS ));
}

static bool FactoryCmdHander_ackChecksumError( uint8 *pdata )
{
    uint8 payload_len = 0;
    uint8 checksum = 0x00;

    if ( pdata == NULL )
        return FALSE;

    payload_len = *( pdata + FAC_RECV_CMD_LEN_POS  );
    checksum = FactoryCmdHandler_CalculateChecksum( pdata );
    if( checksum != ( *( pdata + ( payload_len - 1 ) ) )) 
    {
        *( pdata + FAC_RECV_CMD_TYPE_POS  ) = FAC_CMD_TYPE_SET;
        *( pdata + FAC_RECV_CMD_LEN_POS   ) = 0x07;
        *( pdata + FAC_RECV_OPCODE_POS  ) = 0xFE; 
        
#if ( configReturnChecksum == 1 )
        *( pdata + FAC_ACK_STATUS_POS ) = checksum;
#else
        *( pdata + FAC_ACK_STATUS_POS ) = FAC_ACK_CHECKSUM_NG; 
#endif 
        *( pdata + ( payload_len - 1 ) ) = FactoryCmdHandler_CalculateChecksum( pdata );
        FactoryCmdHandler_sendPackage( pdata );
        return FALSE;
    }

    return TRUE;
}

static bool FactoryCmdHandler_ackInvalidCmdType( uint8 *pdata )
{
    uint8 payload_len = 0;

    if ( pdata == NULL )
        return FALSE;

    payload_len = *( pdata + 2 );

    if ( (*( pdata + 1 ) != FAC_CMD_TYPE_SET) && (*( pdata + 1 ) != FAC_CMD_TYPE_GET) )
    {
        *( pdata + FAC_RECV_CMD_TYPE_POS  ) = FAC_CMD_TYPE_SET;
        *( pdata + FAC_RECV_CMD_LEN_POS   ) = 0x07;
        *( pdata + FAC_RECV_OPCODE_POS  ) = 0xFE; 
        *( pdata + FAC_ACK_STATUS_POS ) = FAC_ACK_INVALID_CMD_TYPE; 
        *( pdata + ( payload_len - 1 ) ) = FactoryCmdHandler_CalculateChecksum( pdata );
        FactoryCmdHandler_sendPackage( pdata );
        return FALSE;
    }

    return TRUE;
}

static bool FactoryCmdHandler_ackInvalidFunctionCode( uint8 *pdata )
{
    uint8 payload_len = 0;

    if ( pdata == NULL )
        return FALSE;

    payload_len = *( pdata + 2 );

    if ( *( pdata + 3 ) >= FAC_OPCODE_UNSUPPORT ) 
    {
        *( pdata + FAC_RECV_CMD_TYPE_POS  ) = FAC_CMD_TYPE_SET;
        *( pdata + FAC_RECV_CMD_LEN_POS   ) = 0x07;
        *( pdata + FAC_RECV_OPCODE_POS  ) = 0xFE; 
        *( pdata + FAC_ACK_STATUS_POS ) = FAC_ACK_OPCODE_OR_OUT_OF_RANG ; /*  invalid function code command */
        *( pdata + ( payload_len - 1 ) ) = FactoryCmdHandler_CalculateChecksum( pdata );
        FactoryCmdHandler_sendPackage( pdata );
        return FALSE;
    }

    return TRUE;
}

static void FactoryCmdHandler_ackOutOfRange( uint8 *pdata )
{
    uint8 payload_len = 0;

    if ( pdata == NULL )
        return;

    payload_len = *( pdata + 2 );

    *( pdata + FAC_RECV_CMD_TYPE_POS  ) = FAC_CMD_TYPE_SET;
    *( pdata + FAC_RECV_CMD_LEN_POS   ) = 0x07;
    *( pdata + FAC_RECV_OPCODE_POS  ) = 0xFE; 
    *( pdata + FAC_ACK_STATUS_POS ) = FAC_ACK_OPCODE_OR_OUT_OF_RANG ; /*  invalid function code command */
    *( pdata + ( payload_len - 1 ) ) = FactoryCmdHandler_CalculateChecksum( pdata );
    FactoryCmdHandler_sendPackage( pdata );
}


static void FactoryCmdHandler_ackSuccess( uint8 *pdata )
{
    uint8 payload_len = 0;
    uint8 *ptr = pdata; 

    if ( ptr == NULL )
    {
        ptr = FactoryPackage;
    }

    payload_len = *( ptr + FAC_RECV_CMD_LEN_POS  );
    *( ptr + FAC_ACK_STATUS_POS ) = FAC_ACK_SUCCESS; /*  return success command */
    *( ptr + ( payload_len - 1 ) ) = FactoryCmdHandler_CalculateChecksum( ptr );
    FactoryCmdHandler_sendPackage( ptr );
}

static void FactoryCmdHandler_returnData( uint8 *pdata )
{
    uint8 payload_len = 0;
    uint8 *ptr = pdata; 

    if ( ptr == NULL )
    {
        ptr = FactoryPackage;
    }

    payload_len = *( ptr + FAC_RECV_CMD_LEN_POS  );
    *( ptr + ( payload_len - 1 ) ) = FactoryCmdHandler_CalculateChecksum( ptr );
    FactoryCmdHandler_sendPackage( ptr );
}


static bool FactoryCmdHandler_IsPackageLegal( uint8 *pdata )
{
    if ( pdata == NULL )
        return FALSE;        

    if (FactoryCmdHander_ackChecksumError( pdata ) == FALSE )
    {
        return FALSE;
       }

    if ( FactoryCmdHandler_ackInvalidCmdType( pdata ) == FALSE )
    {
        return FALSE;
    }

    if ( FactoryCmdHandler_ackInvalidFunctionCode( pdata ) == FALSE )
    {
        return FALSE;
    }
      
    return TRUE;
}

static void FactoryCmdHandler_setCommandHandle( uint8 *pdata )
{
    if ( pdata == NULL )
        return;

    if ( *( pdata + FAC_RECV_CMD_TYPE_POS ) != FAC_CMD_TYPE_SET )
        return;

    switch( *( pdata + FAC_RECV_OPCODE_POS) )    
    {
        case FAC_OPCODE_FACTORY_MODE:
        {
            if ( *( pdata + FAC_RECV_DATA_POS ) > 0x01 )
            {
                FactoryCmdHandler_ackOutOfRange( pdata );
            }
            else
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) == 0x01 )
                {
                    mFactoryParams.sys_event = SYS_EVENT_FAC_SET_MODE_ON;
                    mFactoryParams.op_mode = MODE_FACTORY;
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                }
                else if ( ( *( pdata + FAC_RECV_DATA_POS ) == 0x00 ))
                {
                    mFactoryParams.sys_event = SYS_EVENT_FAC_SET_MODE_OFF;
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    mFactoryParams.op_mode = MODE_USER;
                }
            }
        }
            break;

        case FAC_OPCODE_RESET_SETTING:
        {
            if ( *( pdata + FAC_RECV_DATA_POS ) > 0x01 )
            {
                FactoryCmdHandler_ackOutOfRange( pdata );
            }
            else
            {
                if ( ( *( pdata + FAC_RECV_DATA_POS ) == 0x00 ))
                {
                    mFactoryParams.sys_event = SYS_EVENT_SYSTEM_RESET;
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    mFactoryParams.op_mode = MODE_USER;
                }
            }
        }
            break;

        case FAC_OPCODE_SN:
        {
            uint8 length = 0;
            uint8 sn_buf[15];

            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if( *( pdata + FAC_RECV_CMD_LEN_POS ) > 21 )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    length = *( pdata + FAC_RECV_CMD_LEN_POS );
                    if ( length > 7 )
                    {
                        length = length - 6;
                        memcpy(sn_buf, (pdata + FAC_RECV_DATA_POS), length );
                        if ( pSDM_ObjCtrl->write( STORAGE_DEVICE_CELL_PRODUCT_SERIAL_NUMBER , sn_buf) == FALSE )
                        {
                            FactoryCmdHandler_ackOutOfRange( pdata );
                        }
                        else
                        {
                            *( pdata + FAC_RECV_CMD_LEN_POS ) = 7;
                            FactoryCmdHandler_ackSuccess( pdata );
                        }
                    }
                }
            }
        }
            break;

        case FAC_OPCODE_KP_SWITCH:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > 1 )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    if ( *( pdata + FAC_RECV_DATA_POS ) == 1 )
                    {
                        ButtonCmdDispatcher_switchControl( TRUE );
                        FactoryCmdHandler_ackSuccess( pdata );
                    }
                    else if ( *( pdata + FAC_RECV_DATA_POS ) == 0 )
                    {
                        ButtonCmdDispatcher_switchControl( FALSE );
                        FactoryCmdHandler_ackSuccess( pdata );
                    }
                }
            }
        }
            break;

        case FAC_OPCODE_SOURCE:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) >=AUDIO_SOURCE_MAX )  
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {

#if ( configSII_DEV_953x_PORTING == 1 )
                    if ((AUDIO_SOURCE)*( pdata + FAC_RECV_DATA_POS )==4)
                    {
                        mFactoryParams.input_src = AUDIO_SOURCE_USB_PLAYBACK;
                    }
                    else if ((AUDIO_SOURCE)*( pdata + FAC_RECV_DATA_POS )==5)
                    {
                        mFactoryParams.input_src = AUDIO_SOURCE_BLUETOOTH;
                    }
                    else if ((AUDIO_SOURCE)*( pdata + FAC_RECV_DATA_POS )==6)
                    {
                        mFactoryParams.input_src = AUDIO_SOURCE_HDMI_IN;
                    }               
                    else if ((AUDIO_SOURCE)*( pdata + FAC_RECV_DATA_POS )==7)
                    {
                        mFactoryParams.input_src = AUDIO_SOURCE_HDMI_ARC;
                    } 
                    else
                    {
                        mFactoryParams.input_src = (AUDIO_SOURCE)*( pdata + FAC_RECV_DATA_POS );
                    }
#else
                    mFactoryParams.input_src = (AUDIO_SOURCE)*( pdata + FAC_RECV_DATA_POS );
#endif
                    
                    mFactoryParams.sys_event = SYS_EVENT_SEL_SRC;
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                }
            }
        }
            break;

        case FAC_OPCODE_MUTE:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > SOUND_MUTE )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    mFactoryParams.sys_event = SYS_EVENT_VOLUME_MUTE_SET;
                    mFactoryParams.mute = (bool) *( pdata + FAC_RECV_DATA_POS );
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                }
            }
        }
            break;

        case FAC_OPCODE_VOLUME:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > MASTER_GAIN_MAX )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    mFactoryParams.sys_event = SYS_EVENT_VOLUME_SET;
                    mFactoryParams.master_gain= (uint8) *( pdata + FAC_RECV_DATA_POS );
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                }
            }
        }
            break;

        case FAC_OPCODE_RF_PAIRING:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > 1 )
                {
                    TRACE_DEBUG((0,"FactoryCmdHandler_ackOutOfRange"));
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    TRACE_DEBUG((0,"1RF_HANDLE_PAIRING_PHASE1"));
                    xRFHandleCommand mRFHandleCommand;

                    mRFHandleCommand = RF_HANDLE_PAIRING_PHASE1;
                    pRFHandle_ObjCtrl->rf_instr_sender(&mRFHandleCommand);
                    pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_RF_PAIRING);
                    FactoryCmdHandler_ackSuccess( pdata );
                    TRACE_DEBUG((0,"2RF_HANDLE_PAIRING_PHASE1"));
                }
            }
        }
            break;

        case FAC_OPCODE_TREBLE:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > TREBLE_MAX )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    mFactoryParams.sys_event = SYS_EVENT_TREBLE_SET;
                    mFactoryParams.treble_gain= (uint8) *( pdata + FAC_RECV_DATA_POS );
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                }
            }
        }
            break;

        case FAC_OPCODE_BASS:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > BASS_MAX )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    mFactoryParams.sys_event = SYS_EVENT_BASS_SET;
                    mFactoryParams.bass_gain= (uint8) *( pdata + FAC_RECV_DATA_POS );
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                }
            }
        }
            break;

        case FAC_OPCODE_BTM_PAIRING:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) != FAC_BT_PAIRING_VAL )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    mFactoryParams.sys_event = SYS_EVENT_BT_PAIRING;
                    pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                }
            }
        }
            break;

           case FAC_OPCODE_CHANNEL_TEST:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > 4 )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    switch( *( pdata + FAC_RECV_DATA_POS ) )
                    {
                        case 0: /*normal mode*/
                        {
                            mFactoryParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_DISABLE;
                            pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                            mChannelTest = *( pdata + FAC_RECV_DATA_POS );
                        }    
                            break;

                        case 1: /*L/R channel test */
                        {
                            mFactoryParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_FL_FR;
                            pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                            mChannelTest = *( pdata + FAC_RECV_DATA_POS );
                        }
                            break;

                        case 2: /*LS/RS channel test*/
                        {
                            mFactoryParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_RR_RL;
                            pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                            mChannelTest = *( pdata + FAC_RECV_DATA_POS );
                        }
                            break;

                        case 3:/*center channel test*/
                        {
                            mFactoryParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_CENTER;
                            pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                            mChannelTest = *( pdata + FAC_RECV_DATA_POS );
                        }
                            break;  
                            
                        case 4: /*woofer channel test*/
                        {
                            mFactoryParams.sys_event = SYS_EVENT_FAC_ROUTE_CTRL_LFE0;
                            pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                            mChannelTest = *( pdata + FAC_RECV_DATA_POS );
                        }    
                            break;

                        default:
                            break;     
                    }
                }
            }
        }
            break;

        case FAC_OPCODE_AQ_CONTROL:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( *( pdata + FAC_RECV_DATA_POS ) > 1 )
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
                else
                {
                    if ( *( pdata + FAC_RECV_DATA_POS ) == 1 )
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_AQ_CTRL_ON;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                        mAQSwitch = TRUE;
                    }
                    else if ( *( pdata + FAC_RECV_DATA_POS ) == 0 )
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_AQ_CTRL_OFF;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                        mAQSwitch = FALSE;
                    }
                }
            }
        }
            break;
    
         case FAC_OPCODE_LED_CONTROL:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
               switch (*( pdata + FAC_RECV_DATA_POS ))
                {
                    case 0xff: 
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_ALL_LED;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x00:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_NO_LED;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x01:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED1;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x02:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED2;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;

                    case 0x03:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED3;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x04:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED4;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;

                    case 0x05:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED5;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x06:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED6;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x07:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED7;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x08:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED8;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;

                    case 0x09:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED9;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x0A:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED10;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x0B:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED11;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x0C:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED12;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;
                        
                    case 0x0D:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED13;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;

                    case 0x0E:
                    {
                        mFactoryParams.sys_event = SYS_EVENT_FAC_LED14;
                        pHFS_ObjCtrl->cmd_handle( &mFactoryParams );
                    }
                        break;

                    default:
                    {
                        FactoryCmdHandler_ackOutOfRange( pdata );
                    }
                        break;           
                }
            }
        }
            break;

        case FAC_OPCODE_BTM_CLEAN_DEVICE_LIST:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
#if 1
                xHMISystemParams mFacSystemParams;
                mFacSystemParams.sys_event = SYS_EVENT_SYSTEM_RESET;
                pBTHandle_ObjCtrl->SendEvent( &mFacSystemParams );                
#endif
#if 0 //using new BT mechanism
                mFacBTHandleCommand = BT_HANDLE_CLEAN_DEVICE;
                pBTHandle_ObjCtrl->bt_instr_sender(&mFacBTHandleCommand);
                vTaskDelay( 4500 );
                //pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_CLEAN_DEVICE);                
                //vTaskDelay( 1800 );
#endif
                pUDM_ObjCtrl->ExceptionSendEvent(UI_EVENT_BT_CLEAN_DEVICE);                
                vTaskDelay( 1800 );

                FactoryCmdHandler_ackSuccess( pdata );
            }
        }
            break;

        default:
            break;
    }
}

static void FactoryCmdHandler_getCommandHandle( uint8 *pdata )
{
    if ( pdata == NULL )
        return;

    if ( *( pdata + FAC_RECV_CMD_TYPE_POS ) != FAC_CMD_TYPE_GET )
        return;

    switch( *( pdata + FAC_RECV_OPCODE_POS ) )    
    {
        case FAC_OPCODE_FACTORY_MODE:
        {
            if ( pFacParams->op_mode == MODE_USER )
            {
                *( pdata + FAC_RECV_DATA_POS ) = 0;
                FactoryCmdHandler_ackSuccess( pdata );
            }
            else if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = 1;
                FactoryCmdHandler_returnData( pdata );
            }
            else
            {
                FactoryCmdHandler_ackOutOfRange( pdata );
            }
        }
            break;

        case FAC_OPCODE_SN:
        {
            uint8 sn_buf[15];

            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( pSDM_ObjCtrl->read( STORAGE_DEVICE_CELL_PRODUCT_SERIAL_NUMBER , sn_buf) == TRUE )
                {
                    memcpy((pdata + FAC_RECV_DATA_POS), sn_buf, 15 );

                    *( pdata + FAC_RECV_CMD_LEN_POS) = (15+6) ;
                    FactoryCmdHandler_returnData( pdata );
                }
                else
                {
                    FactoryCmdHandler_ackOutOfRange( pdata );
                }
            }
        }
            break;

        case FAC_OPCODE_KP_ADC_VALUE:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = ButtonLowLevel_get_btn_ADC_Value();
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_KP_SWITCH:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = ButtonCmdDispatcher_getSwitchStatus();
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_SOURCE:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = (uint8)pFacParams->input_src;
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_MUTE:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = (uint8)pFacParams->mute;
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_VOLUME:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = (uint8)pFacParams->master_gain;
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_RF_PAIRING:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = RFHandler_Paired_Status();
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_TREBLE:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = (uint8)pFacParams->treble_gain;
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_BASS:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = (uint8)pFacParams->bass_gain;
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_BTM_PAIRING:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if (pBTHandle_ObjCtrl->get_status() == BT_LINKED)
                {
                    *( pdata + FAC_RECV_DATA_POS ) = 1;
                }
                else
                {
                    *( pdata + FAC_RECV_DATA_POS ) = 0;
                }
                
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_CHANNEL_TEST:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = mChannelTest;
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_AQ_CONTROL:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                *( pdata + FAC_RECV_DATA_POS ) = mAQSwitch;
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_AUDIO_FORMAT:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                if ( pAudLowLevel_ObjCtrl->getAudioStreamType() == 1 )
                {
                    *( pdata + FAC_RECV_DATA_POS ) = 0x03;
                }
                else if ( pAudLowLevel_ObjCtrl->getAudioStreamType() == 2 )
                {
                    *( pdata + FAC_RECV_DATA_POS ) = 0x01;
                }
                else if ( pAudLowLevel_ObjCtrl->getAudioStreamType() == 3 )
                {
                    *( pdata + FAC_RECV_DATA_POS ) = 0x0B;
                }
                else 
                {
                    *( pdata + FAC_RECV_DATA_POS ) = 0x03;
                }

                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        case FAC_OPCODE_VERSION:
        {
            if ( pFacParams->op_mode == MODE_FACTORY )
            {
                unsigned char FlashVersionNumber[BYTE_OF_VERSION];
                
                *( pdata + FAC_RECV_DATA_POS ) = (uint8)( ( VERSION&0xFF00 ) >> 8 );
                *( pdata + FAC_RECV_DATA_POS + 1 ) = (uint8)( VERSION&0x00FF ); 
                *( pdata + FAC_RECV_CMD_LEN_POS)=2+6+BYTE_OF_VERSION; 
                
                pCCKManage_ObjCtrl->GetNoiseVersionNum(FlashVersionNumber);
                *(pdata + FAC_RECV_DATA_POS + 2) = *(FlashVersionNumber);
                *(pdata + FAC_RECV_DATA_POS + 3) = *(FlashVersionNumber+1);
                *(pdata + FAC_RECV_DATA_POS + 4) = *(FlashVersionNumber+2);
                *(pdata + FAC_RECV_DATA_POS + 5) = *(FlashVersionNumber+3);
                *(pdata + FAC_RECV_DATA_POS + 6) = *(FlashVersionNumber+4);
                *(pdata + FAC_RECV_DATA_POS + 7) = *(FlashVersionNumber+5);
                
                FactoryCmdHandler_returnData( pdata );
            }
        }
            break;

        default:
            break;

    }
}

void FactoryCmdHandler_ServiceHandle( void *pvParameters )
{
    for( ;; )
    {
        switch( xFacServiceHandler.taskState )
        {
            case TASK_SUSPENDED:
            {
                if ( uxQueueMessagesWaiting(xFacServiceHandler.Qget.xQueue) > 0 )
                {
                    xFacServiceHandler.taskState = TASK_READY;
                }
                else if ( uxQueueMessagesWaiting(xFacServiceHandler.Qset.xQueue) > 0 )
                {
                    xFacServiceHandler.taskState = TASK_RUNING;
                }

            }
                break;
        
            case TASK_READY:
            {
                FactoryCmdHandler_getPacket( pPackage );
                if ( FactoryCmdHandler_IsPackageLegal( pPackage ) == TRUE )
                {
                    xFacServiceHandler.taskState = TASK_RUNING;
                }
                else
                {
                       xFacServiceHandler.taskState = TASK_SUSPENDED;
                }
            }
                break;

            case TASK_RUNING:
            {
                if (*( pPackage + 1 ) == FAC_CMD_TYPE_SET )
                {
                    FactoryCmdHandler_setCommandHandle( pPackage );
                }
                else if (*( pPackage + 1 ) == FAC_CMD_TYPE_GET )
                {
                    FactoryCmdHandler_getCommandHandle( pPackage );
                }
                
                xFacServiceHandler.taskState = TASK_SUSPENDED;
            }
                break;

            default:
            {
                xFacServiceHandler.taskState = TASK_SUSPENDED;
            }
                break;
        }

        vTaskDelay( SERVICE_HANLDER_TIME_TICK );
    }


}

//________________________________________________________________
void FactoryCmdHandler_CreateTask( void )
{
    if ( xTaskCreate( 
        FactoryCmdHandler_ServiceHandle, 
        ( portCHAR * ) "FAC_HANDLER", 
        STACK_SIZE, 
        NULL, 
        tskFCI_SERVICE_PRIORITY, 
        &xFacServiceHandler.handle ) != pdPASS )
    {
        vTaskDelete( xFacServiceHandler.handle );
        //TRACE_ERROR((0, "FAC_HANDLER task create failure " ));
    }
    else
    {
        //vTaskSuspend( xFacServiceHandler.handle );
        TRACE_DEBUG((0, "SUSPEND: FactoryCmdHandler_ServiceHandle "));
    }

    xFacServiceHandler.Qget.xQueue = xQueueCreate( FC_QUEUE_LENGTH, (sizeof(uint8)*FACTORY_PACKAGE_MAX) );
    xFacServiceHandler.Qget.xBlockTime = BLOCK_TIME(0);
    if( xFacServiceHandler.Qget.xQueue == NULL )
    {
       TRACE_ERROR((0, "FAC_HANDLER queue creates failure " ));
    }

    xFacServiceHandler.Qset.xQueue = xQueueCreate( FC_QUEUE_LENGTH, (sizeof(uint8)*FACTORY_PACKAGE_MAX) );
    xFacServiceHandler.Qset.xBlockTime = BLOCK_TIME(0);
    if( xFacServiceHandler.Qset.xQueue == NULL )
    {
       TRACE_ERROR((0, "FAC_HANDLER queue creates failure " ));
    }
}


