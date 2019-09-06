#include "Debug.h"
#include "SPILowLevel.h"
#include "GPIOMiddleLevel.h"
#include "Config.h"

#include "freertos_conf.h"
#include "freertos_task.h"

#include "cs4953x_spi.h"


/*-------------------------------------------------------------------------------------------*/
#define SLAVE_ADDRESS_W_CS4953x	0x80
#define SLAVE_ADDRESS_R_CS4953x	0x81

#define TIMEOUT TASK_MSEC2TICKS(500)
/*-------------------------------------------------------------------------------------------*/

/** Endianity Mode*/
static bool m_endianityMode = SPI_LITTEL_ENDIAN;//SPI_BIG_ENDIAN//;SPI_LITTEL_ENDIAN; /*Smith Modify*/

//____________________________________________________________________________________________

//!	< External Application_________________________________________________@{
//These applicatons is only used on here.

extern bool SPILowLevel_isEnable( void );
//! @}


//___________________________________________________________________________________________________
int8 CS4953xSPI_write_buffer(const byte* data, uint16 length)
{
    uint16	block_num = 0;
    uint16	insufficient_block = 0;
    int i = 0;
    int ret_val = SCP1_PASS;
    
    portTickType busy_timeout = 0;

    if ( SPILowLevel_isEnable( ) == FALSE )
    {
        ret_val = SCP1_BYPASS;
        return ret_val;
    }
    /* SPI lock*/
    if (SPI_mutex_lock() == FALSE)
    {
        ret_val = SCP1_BSY_TIMEOUT;
        return ret_val;
    }

    /* Select SPI chip.*/
    SPI_selectChip(CS4953x_SPI_NPCS);
    SPI_write( SLAVE_ADDRESS_W_CS4953x );

    /*Write 4 data bytes*/
    block_num = length/4; /*1 block is 4 bytes*/
    if ( block_num != 0)
    {
        for ( i = 0; i < block_num; i++ )
        {
            SPI_writeBuffer((data+(i*4)),4, m_endianityMode);
            busy_timeout = xTaskGetTickCount( );
            while( GPIOMiddleLevel_Read_InBit( __I_SCP1_BUSY ) == 0 )
            {
                if ( ((xTaskGetTickCount() - busy_timeout)/portTICK_RATE_MS) > TIMEOUT )
                {
                    busy_timeout = 0;
                    ret_val = SCP1_BSY_TIMEOUT;
            
                    
                    break;
                }
            }

            
            
            if (ret_val == SCP1_BSY_TIMEOUT)
            {
                break;
            }
        }
    }
    
    if (ret_val != SCP1_BSY_TIMEOUT)
    {
    insufficient_block = length%4; /*If data length is insufficient a block, */
    if ( insufficient_block != 0 )  
    {
        SPI_writeBuffer((data+(i*4)),insufficient_block, m_endianityMode);
        busy_timeout = xTaskGetTickCount( );
        while( GPIOMiddleLevel_Read_InBit( __I_SCP1_BUSY ) == 0 )
        {
            if ( ((xTaskGetTickCount() - busy_timeout)/portTICK_RATE_MS) > TIMEOUT )
            {
                busy_timeout = 0;
                ret_val = SCP1_BSY_TIMEOUT;
                break;
            }
        }
    }
    }

    SPI_unselectChip(CS4953x_SPI_NPCS);
    
    /* SPI unlock*/
    SPI_mutex_unlock();

    return ret_val;
    
}


int8 CS4953xSPI_write_ULD_buffer(const byte* data, uint16 length)
{
    int i = 0,j=0;
    portTickType busy_timeout = 0;

    uint16	block_num = 0;
    uint16	insufficient_block = 0;
    int ret_val = SCP1_PASS;


    if ( SPILowLevel_isEnable( ) == FALSE )
    {
        ret_val = SCP1_BYPASS;
        return ret_val;
    }
    /* SPI lock*/
    if (SPI_mutex_lock() == FALSE)
    {
        ret_val = SCP1_BSY_TIMEOUT;
        return ret_val;
    }

    /* Select SPI chip.*/
    SPI_selectChip(CS4953x_SPI_NPCS);
    SPI_write( SLAVE_ADDRESS_W_CS4953x );


    block_num = length/4;


    for ( i = 0; i < block_num; i++ )
    {
        SPI_write(*(data+i*4));
        SPI_write(*(data+i*4+1));
        SPI_write(*(data+i*4+2));
        SPI_write(*(data+i*4+3));
        
        busy_timeout = xTaskGetTickCount( );
        while( GPIOMiddleLevel_Read_InBit( __I_SCP1_BUSY ) == 0 )
        {
            if ( ((xTaskGetTickCount() - busy_timeout)/portTICK_RATE_MS) > TIMEOUT )
            {
                busy_timeout = 0;
                ret_val = SCP1_BSY_TIMEOUT;
                break;
            }
        }

        if (ret_val == SCP1_BSY_TIMEOUT)
        {
            break;
        }
    }

    if (ret_val != SCP1_BSY_TIMEOUT)
    {
        insufficient_block = length%4;
        if(insufficient_block>0)
        {
            for(j=0;j<insufficient_block;j++)
                SPI_write(*(data+i*4)+j);

            busy_timeout = xTaskGetTickCount( );
            while( GPIOMiddleLevel_Read_InBit( __I_SCP1_BUSY ) == 0 )
            {
                if ( ((xTaskGetTickCount() - busy_timeout)/portTICK_RATE_MS) > TIMEOUT )
                {
                    busy_timeout = 0;
                    ret_val = SCP1_BSY_TIMEOUT;
                    break;
                }
            }
        }
    }

    SPI_unselectChip(CS4953x_SPI_NPCS);

    /* SPI unlock*/
    SPI_mutex_unlock();
    
    return ret_val;
    
}


int8 CS4953xSPI_read_buffer( byte* data, uint16 length )
{
    portTickType irq_timeout = 0;
    uint16	block_num = 0;
    int i = 0;
    int ret_val = SCP1_PASS;
   
    if ( SPILowLevel_isEnable () == FALSE )
    {
        ret_val = SCP1_BYPASS;
        return ret_val;
    }

    irq_timeout = xTaskGetTickCount( );
    while( GPIOMiddleLevel_Read_InBit( __I_SCP1_IRQ ) == 1 )
    {
        if ( ((xTaskGetTickCount() - irq_timeout)/portTICK_RATE_MS) > TASK_MSEC2TICKS(100) )
        {
            irq_timeout = 0;
            ret_val = SCP1_BSY_TIMEOUT;
            break;
        }
    }

    if (ret_val != SCP1_BSY_TIMEOUT)
    {
        /* Select SPI chip.*/
        /* SPI lock*/
        if (SPI_mutex_lock() == FALSE)
        {
            ret_val = SCP1_BSY_TIMEOUT;
            return ret_val;
        }

        SPI_selectChip(CS4953x_SPI_NPCS);

        /*Write address of chip*/
        SPI_write(SLAVE_ADDRESS_R_CS4953x);

        /*read dummy byte*/
        SPI_I2S_ReceiveData( _SPI ); 
        SPI_I2S_ReceiveData( _SPI );

        /*Write 4 data bytes*/
        block_num = length/4; /*1 block is 4 bytes*/
        for ( i = 0; i < block_num; i++ )
        {
            SPI_readBuffer((data+(i*4)),4, m_endianityMode );
        }

        /* Unselect SPI chip.*/
        SPI_unselectChip(CS4953x_SPI_NPCS);

        /* SPI unlock*/
        SPI_mutex_unlock();
    }


    if (ret_val != SCP1_BSY_TIMEOUT)
    {
        if ( GPIOMiddleLevel_Read_InBit( __I_SCP1_IRQ ) == 0 )
        {
            ret_val = SCP1_IRQ_LOW; /*when IRQ is still a low, DSP require read more date*/   
        }
    }
    
    return ret_val;
}

int8 CS4953xSPI_nIrq_read_buffer( byte* data, uint16 length )
{
    uint16	block_num = 0;
     int i = 0;
    int ret_val = SCP1_PASS;
   
    if ( SPILowLevel_isEnable () == FALSE )
    {
        ret_val = SCP1_BYPASS;
        return ret_val;
    }
    /* SPI lock*/
    if (SPI_mutex_lock() == FALSE)
    {
        ret_val = SCP1_BSY_TIMEOUT;
        return ret_val;
    }

    /* Select SPI chip.*/
    SPI_selectChip(CS4953x_SPI_NPCS);

    /*Write address of chip*/
    SPI_write(SLAVE_ADDRESS_R_CS4953x);

    /*read dummy byte*/
    SPI_I2S_ReceiveData( _SPI ); 
    SPI_I2S_ReceiveData( _SPI );

    /*Write 4 data bytes*/
    block_num = length/4; /*1 block is 4 bytes*/
    for ( i = 0; i < block_num; i++ )
    {
        SPI_readBuffer((data+(i*4)),4, m_endianityMode );
    }
    /* Unselect SPI chip.*/
    SPI_unselectChip(CS4953x_SPI_NPCS);

    /* SPI unlock*/
    SPI_mutex_unlock();

    return ret_val;

}


/*IRQ no drop, (MSG NOTE 1): Read four bytes from the DSP /IRQ will not drop for this read sequence)*/
int8 CS4953xSPI_MsgNote1( byte* data, uint16 length )
{
    uint16	block_num = 0;
    int i = 0;
    int ret_val = SCP1_PASS;
   
    if ( SPILowLevel_isEnable () == FALSE )
    {
        ret_val = SCP1_BYPASS;
        return ret_val;
    }
        
    /* SPI lock*/
    if (SPI_mutex_lock() == FALSE)
    {
        ret_val = SCP1_BSY_TIMEOUT;
        return ret_val;
    }

    /* Select SPI chip.*/
    if (SPI_selectChip(CS4953x_SPI_NPCS) == FALSE)
    {
        ret_val = SCP1_BSY_TIMEOUT;
        //return ret_val;
    }

    if (ret_val == SCP1_PASS)
    {
        /*Write address of chip*/
        SPI_write(SLAVE_ADDRESS_R_CS4953x);

        /*read dummy byte*/
        SPI_I2S_ReceiveData( _SPI ); 
        SPI_I2S_ReceiveData( _SPI );

        /*Write 4 data bytes*/
        block_num = length/4; /*1 block is 4 bytes*/
        for ( i = 0; i < block_num; i++ )
        {
            SPI_readBuffer((data+(i*4)),4, m_endianityMode );
        }
        /* Unselect SPI chip.*/
        SPI_unselectChip(CS4953x_SPI_NPCS);
    }

    /* SPI unlock*/
    SPI_mutex_unlock();

    return ret_val;
}

bool CS49xxxxSPI_CommandWrite(uint32 cmd, uint32 value)
{
    uint32 command = cmd;
    uint32 val = value;

    if (CS4953xSPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8))) == SCP1_IRQ_TIMEOUT )
    {
        return FALSE;
    }
        
    if (CS4953xSPI_write_buffer((byte*)&val, (sizeof(val)/sizeof(uint8))) == SCP1_IRQ_TIMEOUT )
    {
        return FALSE;
    }

    return TRUE;

}

bool CS49xxxxSPI_ReadSolicited( uint32 cmd, uint32 *pValue )
{
    //bool ret;
    uint32 command = cmd;
    uint32 value = 0;

    if ( pValue == NULL )
        return FALSE;

    if( CS4953xSPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)))== SCP1_IRQ_TIMEOUT)
    {
        return FALSE;
    }

    /*Read response command word*/
    if (CS4953xSPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8))== SCP1_IRQ_TIMEOUT)
    {
        return FALSE;
    }

    /*read respones data word*/
    if (CS4953xSPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8))== SCP1_IRQ_TIMEOUT)
    {
        return FALSE;
    }
    *pValue = value;
        
    return TRUE;

}
