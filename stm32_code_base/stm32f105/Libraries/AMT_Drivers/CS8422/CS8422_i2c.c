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
#define CS8422_ERROR_TIMER TASK_MSEC2TICKS(1000) // 1 SEC

/*____________________________________________________________________________*/
byte CS8422_ReadI2C_Byte(byte RegAddr)
{
    byte  uc = 0;
    bool ret = TRUE;
    AUDIO_EXCEPTION_QUEUE_TYPE tmp_QueueType = { EXCEPTION_NONE, 0 }; // David, fix warning.
    portTickType start8422Time = 0;

    start8422Time = xTaskGetTickCount();
    while(1)
    {
        ret = I2C2_readSlave(CS8422_I2C_ADDR, RegAddr, &uc, 1, FALSE);

#if 0 /*smith fixes bug: SB3851_C0-227: After system reset, press keypad vol+, system abnormal, audio drops*/
        vTaskDelay( TASK_MSEC2TICKS(CS8422_INTERVAL_TIME) );
#endif

        if ( ret == TRUE )
        {
            break;
        }   
        else
        {
            if(((xTaskGetTickCount() - start8422Time)/portTICK_RATE_MS) > CS8422_ERROR_TIMER)
            {
                TRACE_ERROR((0, "8422 read err OVER TIME %d ",((xTaskGetTickCount() - start8422Time)/portTICK_RATE_MS)));
                break;
            }
        }
    }
    
    if( ret== TRUE )
    {
        return uc;
    }
    else
    {

#if 1  /*DC work arround: Incoming spdif signal include noise. the work arround is able to cover cs8422 crash issue*/
        tmp_QueueType.event = SRC_BUS_FAULT;
        pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
#endif
    }
    return 0xFF ;
}

bool CS8422_WriteI2C_Byte(byte RegAddr,byte uc)
{
    bool  ret;
    AUDIO_EXCEPTION_QUEUE_TYPE tmp_QueueType; // David, fix warning.
    portTickType start8422Time = 0;

    start8422Time = xTaskGetTickCount();
    while(1)
    {  
        ret = I2C2_writeSlave(CS8422_I2C_ADDR, RegAddr, (byte*)&uc, 1, FALSE);

#if 0 /*smith fixes bug: SB3851_C0-227: After system reset, press keypad vol+, system abnormal, audio drops*/
        vTaskDelay( TASK_MSEC2TICKS(CS8422_INTERVAL_TIME) );
#endif

        if ( ret == TRUE )
        {
            break;
        }   
        else
        {
            if(((xTaskGetTickCount() - start8422Time)/portTICK_RATE_MS) > CS8422_ERROR_TIMER)
            {
                TRACE_ERROR((0, "8422 write err OVER TIME %d ",((xTaskGetTickCount() - start8422Time)/portTICK_RATE_MS)));
                break;
            }
        }
    }

    if ( ret == FALSE )
    {
#if 1 /*DC work arround: Incoming spdif signal include noise. the work arround is able to cover cs8422 crash issue*/
        tmp_QueueType.event = SRC_BUS_FAULT;
        pAudLowLevel_ObjCtrl->ExceptionSender(&tmp_QueueType);
#endif        
        return FALSE;
    }

    return TRUE;
}
