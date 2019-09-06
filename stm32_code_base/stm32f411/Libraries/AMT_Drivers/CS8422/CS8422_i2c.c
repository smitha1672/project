#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "CS8422_i2c.h"
#include "api_typedef.h"
#include "AudioDeviceManager.h"

extern AUDIO_LOWLEVEL_DRIVER_OBJECT *pAudLowLevel_ObjCtrl;

/*____________________________________________________________________________*/
#define CS8422_I2C_ADDR      0x28
#define CS8422_RETRY_TIME    2
#define CS8422_INTERVAL_TIME 1

/*____________________________________________________________________________*/
byte CS8422_ReadI2C_Byte(byte RegAddr)
{
    byte  uc;
    bool ret ;
    AUDIO_EXCEPTION_QUEUE_TYPE tmp_QueueType;
    int i;

    for( i = 0; i < CS8422_RETRY_TIME; i++ )
    {
        ret = I2C2_readSlave(CS8422_I2C_ADDR, RegAddr, &uc, 1, FALSE);

        vTaskDelay( TASK_MSEC2TICKS(CS8422_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    }
    
    if( ret== TRUE )
    {
        return uc;
    }
    else
    {
        TRACE_ERROR((0, "CS8422 I2C reads error !!  Reg -----> %X ",RegAddr));
        tmp_QueueType.event = SRC_BUS_FAULT;
        pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
    }

    return 0xFF ;
}

bool CS8422_WriteI2C_Byte(byte RegAddr,byte uc)
{
    bool  ret;
    AUDIO_EXCEPTION_QUEUE_TYPE tmp_QueueType;
    int i;

    for( i = 0; i < CS8422_RETRY_TIME; i++ )
    {  
        ret = I2C2_writeSlave(CS8422_I2C_ADDR, RegAddr, (byte*)&uc, 1, FALSE);

        vTaskDelay( TASK_MSEC2TICKS(CS8422_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    }

    if ( ret == FALSE )
    {
        //TRACE_ERROR((0, "CS8422 I2C writes error !! Reg -----> %X ",RegAddr));
        tmp_QueueType.event = SRC_BUS_FAULT;
        pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
        
        return FALSE;
    }

    return TRUE;
}
