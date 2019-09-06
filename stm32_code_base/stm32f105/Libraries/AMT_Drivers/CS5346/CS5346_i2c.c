#include "Debug.h"
#include "CS5346_i2c.h"
#include "freertos_conf.h"
#include "freertos_task.h"
/*____________________________________________________________________________*/
#define CS5346_I2C_ADDR      0x98 
#define CS5346_RETRY_TIME    3  
#define CS5346_INTERVAL_TIME 1
/*____________________________________________________________________________*/

byte CS5346_ReadI2C_Byte(byte RegAddr)
{
    byte  uc;
    bool ret ;
    int i;

    for( i = 0; i < CS5346_RETRY_TIME; i++ )
    {
        ret = I2C2_readSlave(CS5346_I2C_ADDR, RegAddr, &uc, 1, FALSE);

        //vTaskDelay( TASK_MSEC2TICKS(CS5346_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    }
        
    if( ret == FALSE )
    {
        TRACE_ERROR((0, "CS5346 I2C reads error !!"));
        return 0xFF ;
    }

    return uc;
}

bool CS5346_WriteI2C_Byte(byte RegAddr,byte uc)
{
    bool  ret;
    int i;

    for( i = 0; i < CS5346_RETRY_TIME; i++ )
    {  
        ret = I2C2_writeSlave(CS5346_I2C_ADDR, RegAddr, (byte*)&uc, 1, FALSE);

        //vTaskDelay( TASK_MSEC2TICKS(CS5346_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    }

    if ( ret == FALSE )
    {
        TRACE_ERROR((0, " CS5346 I2C writes error !! "));
        return FALSE;
    }

    return TRUE;
}

