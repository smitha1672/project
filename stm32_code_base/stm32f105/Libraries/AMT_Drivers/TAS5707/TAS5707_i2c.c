#include "Debug.h"
#include "TAS5707_i2c.h"
#include "freertos_conf.h"
#include "freertos_task.h"

/*____________________________________________________________________________*/
#define TAS5707_DEVICE_ADDR	0x36
#define TAS5707_RETRY_TIME  3
#define TAS5707_INTERVAL_TIME 1

/*____________________________________________________________________________*/
byte TAS5707_I2C_read( byte reg_addr)
{
    byte uc;
    int ret ;
    int i;

    for( i = 0; i < TAS5707_RETRY_TIME; i++ )
    {
        ret = I2C2_readSlave(TAS5707_DEVICE_ADDR, reg_addr, &uc, 1, FALSE);

        //vTaskDelay( TASK_MSEC2TICKS(TAS5707_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    }
    
    if( ret== FALSE)
    {
        TRACE_ERROR((0, " TAS5707 I2C reads error !!"));
        return 0xFF;
    } 
    
    return uc ;
}

bool TAS5707_I2C_write(  byte reg_addr, byte value)
{
    int  ret;
    int i;

    for( i = 0; i < TAS5707_RETRY_TIME; i++ )
    {  
        ret = I2C2_writeSlave(TAS5707_DEVICE_ADDR, reg_addr, (byte*)&value, 1, FALSE);

        //vTaskDelay( TASK_MSEC2TICKS(TAS5707_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    }
    
    if ( ret == FALSE )
    {
        TRACE_ERROR((0, " TAS5777 I2C writes error !! "));
        return FALSE;
    }
         
    return TRUE;
}

bool TAS5707_I2C_read_n_byte(  byte reg_addr, byte *data, int N)
{
    int  ret;
    int i;

    for( i = 0; i < TAS5707_RETRY_TIME; i++ )
    {  
        ret = I2C2_readSlave(TAS5707_DEVICE_ADDR, reg_addr, data, N, FALSE);

        if ( ret == TRUE )
        {
            break;
        }   
    }
    
    if ( ret == FALSE )
    {
        TRACE_ERROR((0, " TAS5707 I2C reads error !! "));
        return FALSE;
    }

    return TRUE;
}

bool TAS5707_I2C_write_n_byte(  byte reg_addr, byte* data,int N)
{
    int ret;
    int i;
    
    for( i = 0; i < TAS5707_RETRY_TIME; i++ )
    {      
        ret = I2C2_writeSlave(TAS5707_DEVICE_ADDR, reg_addr, data, N, FALSE);

        if ( ret == TRUE )
        {
            break;
        }   
    }
    
    if ( ret == FALSE )
    {
        TRACE_ERROR((0, " TAS5707 I2C writes error !! "));
        return FALSE;
    }

    return TRUE;
}


