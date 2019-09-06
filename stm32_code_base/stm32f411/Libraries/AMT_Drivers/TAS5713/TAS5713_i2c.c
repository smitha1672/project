#include "Debug.h"
#include "TAS5713_i2c.h"
#include "freertos_conf.h"
#include "freertos_task.h"
/*____________________________________________________________________________*/
#define TAS5713_LOW_DEVICE_ADDR    0x34
#define TAS5713_HIGH_DEVICE_ADDR   0x36
#define TAS5713_RETRY_TIME    3
#define TAS5713_INTERVAL_TIME 1

/*____________________________________________________________________________*/
byte TAS5713_I2C_read(bool Address,byte reg_addr)
{
    byte uc;
    int ret ;
    int i;

    for( i = 0; i < TAS5713_RETRY_TIME; i++ )
    {   
        if(Address)
        {
            ret = I2C2_readSlave(TAS5713_HIGH_DEVICE_ADDR, reg_addr, &uc, 1, FALSE);
        }
        else
        {
            ret = I2C2_readSlave(TAS5713_LOW_DEVICE_ADDR, reg_addr, &uc, 1, FALSE);
        }

        //vTaskDelay( TASK_MSEC2TICKS(TAS5713_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    } 
    
    if( ret== FALSE )
    {
        TRACE_ERROR((0, " TAS5713 I2C reads error !!"));
        return 0xFF ;
    }

    return uc;
}

bool TAS5713_I2C_write(bool Address ,byte reg_addr, byte value)
{
    int ret;
    int i;

    for( i = 0; i < TAS5713_RETRY_TIME; i++ )
    { 
        if(Address)
        {
            ret = I2C2_writeSlave(TAS5713_HIGH_DEVICE_ADDR, reg_addr, (byte*)&value, 1, FALSE);
        }
        else
        {
            ret = I2C2_writeSlave(TAS5713_LOW_DEVICE_ADDR, reg_addr, (byte*)&value, 1, FALSE);
        }

        //vTaskDelay( TASK_MSEC2TICKS(TAS5713_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    } 
    
    if ( ret == FALSE )
    {
        TRACE_ERROR((0, " TAS5713 I2C writes error !! "));
        return FALSE;
    }
         
    return TRUE;
}

bool TAS5713_I2C_read_n_byte(bool Address ,byte reg_addr, byte *data, int N)
{
    int ret;
    int i;

    for( i = 0; i < TAS5713_RETRY_TIME; i++ )
    { 

        if(Address)
        {
            ret = I2C2_readSlave(TAS5713_HIGH_DEVICE_ADDR, reg_addr, data, N, FALSE);
        }
        else
        {
            ret = I2C2_readSlave(TAS5713_LOW_DEVICE_ADDR, reg_addr, data, N, FALSE);
        }

        vTaskDelay( TASK_MSEC2TICKS(TAS5713_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    } 
    
    if ( ret == FALSE )
    {
        TRACE_ERROR((0, " TAS5713 I2C reads error !! "));
        return FALSE;
    }

    return TRUE;
}

bool TAS5713_I2C_write_n_byte(bool Address,byte reg_addr, byte* data,int N)
{
    int ret;
    int i;

    for( i = 0; i < TAS5713_RETRY_TIME; i++ )
    {  
        if(Address)
        {
            ret = I2C2_writeSlave(TAS5713_HIGH_DEVICE_ADDR, reg_addr, data, N, FALSE);
        }
        else
        {
            ret = I2C2_writeSlave(TAS5713_LOW_DEVICE_ADDR, reg_addr, data, N, FALSE);
        }

        vTaskDelay( TASK_MSEC2TICKS(TAS5713_INTERVAL_TIME) );

        if ( ret == TRUE )
        {
            break;
        }   
    } 
    
    if ( ret == FALSE )
    {
        TRACE_ERROR((0, " TAS5713 I2C writes error !! "));
        return FALSE;
    }

    return TRUE;
}
