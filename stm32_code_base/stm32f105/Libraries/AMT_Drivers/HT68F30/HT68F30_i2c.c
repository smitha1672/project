#include "freertos_conf.h"
#include "freertos_task.h"
#include "device_config.h"
#include "HT68F30_i2c.h"
#include "I2C1LowLevel.h"

#define HT68F30_I2C_ADDR 0x40
#define HT68_RETRY_TIME 3
#define HT68F30_I2C_INTERVAL_TIME 4

bool HT68F30_WriteI2C_Byte(byte RegAddr,byte uc)
{
    bool  ret = TRUE;
    int i;
    
    for( i = 0; i < HT68_RETRY_TIME; i++ )
    {
#if ( configHDMI_REPEATER == 0 )    /*it divide HDMI and NON-HDMI model*/
        ret = I2C_writeSlave( HT68F30_I2C_ADDR , RegAddr , (byte*)&uc, 1, FALSE, FALSE);
#else
        ret = I2C_writeSlave( HT68F30_I2C_ADDR , RegAddr , (byte*)&uc, 1, FALSE, TRUE);
#endif

        vTaskDelay( TASK_MSEC2TICKS(HT68F30_I2C_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    }

    if ( ret == FALSE )
    {
        //TRACE_ERROR((0, "HT68F30 I2C writes error !! "));
        return FALSE;
    }

    return TRUE;
}
