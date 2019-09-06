#include "CS498xx_spi.h"
#include "STM32F0xx_board.h"
#include "Config.h"
#include "Debug.h"
#include "AudioSystemDriver.h"
/*----------------------------------------------------------------------------*/
#define SLAVE_ADDRESS_W_CS498xx    0x80
#define SLAVE_ADDRESS_R_CS498xx    0x81
/*----------------------------------------------------------------------------*/

//!    < External Application_________________________________________________@{
//These applicatons is only used on here.

extern bool SPI1_LowLevel_isEnable( void );
//! @}

/** Endianity Mode*/
static bool m_endianityMode = SPI_LITTEL_ENDIAN;

/** Holds the write command ACK timeout. Set to default value */
static uint32 m_writeCmdTimeOut = SPI_TIME_OUT_WRITE_CMD;

bool CS498xx_SPI_IsDSP_IRQ_LOW(uint32 timeout)
{
    uint32 startTime;
    uint32 elapsedTime;
    bool bFinish = FALSE;
    bool ret = FALSE;
    uint8 status;

    startTime = VirtualTimer_now();
        
    while ( bFinish == FALSE)
    {
        status = GPIO_Read_InBit(__I_SCP1_IRQ);
        if ( status == 1 )
        {
            elapsedTime = VirtualTimer_now() - startTime;
            if ( elapsedTime > timeout)
            {
                //TRACE_DEBUG((0,"DSP IRQ dectection is time out"));
                ret = FALSE;
                bFinish = TRUE;
            }
        }
        else
        {
            ret = TRUE; /*If IRQ is low return TRUE*/
            bFinish = TRUE;
        }
    }

    return ret;
}

static bool CS498xx_SPI_IsDSPBusy(uint32 timeout)
{
    uint32 startTime;
    uint32 elapsedTime;
    bool bDspBusy = FALSE;
    bool bFinish = FALSE;
    bool ret = TRUE;
    uint8 status;

    startTime = VirtualTimer_now();
    
    while ( bFinish == FALSE)
    {
        status = GPIO_Read_InBit(__I_SCP1_BUSY);
        bDspBusy = (bool)status;
        if ( bDspBusy == FALSE)
        {
            elapsedTime = VirtualTimer_now() - startTime;
            if ( elapsedTime > timeout)
            {
                ret = FALSE;
                bFinish = TRUE;
            }
        }
        else
        {
            ret = TRUE;
            bFinish = TRUE;
        }
    }

    return ret;
}

//!    CS498xx interface enable    @{
#ifdef CS498xx_ITF
bool CS498xx_SPI_write_buffer(byte* data, uint16 length)
{
    bool ret = TRUE;
    bool bIsDSPBusy = DSP_IDLE;
    uint16    block_num = 0;
    uint16    insufficient_block = 0;
    int i = 0;
    uint8 busyCnt = 0;

    /* Select SPI chip.*/
    SPI_selectChip(CS498xx_SPI_NPCS);

    /*Write address of chip*/
    if ( SPI1_write(SLAVE_ADDRESS_W_CS498xx) != TRUE)
    {
        SPI_unselectChip(CS498xx_SPI_NPCS);
        ret = FALSE;
        return ret;
    }

    /*Write 4 data bytes*/
    block_num = length/4; /*1 block is 4 bytes*/
    if ( block_num != 0)
    {
        for ( i=0; i<block_num; i++ )
        {
            do{
                if (SPI1_writeBuffer((data+(i*4)),4, m_endianityMode)== SPI_RET_ERROR_LEN)
                {
                    SPI_unselectChip(CS498xx_SPI_NPCS);
                    ret = FALSE;
                    return ret;
                }
                bIsDSPBusy = (bool)CS498xx_SPI_IsDSPBusy(m_writeCmdTimeOut);

                if ( bIsDSPBusy == DSP_BUSY )
                {
                    //work around solution
                    TRACE_ERROR((0, " SPI DATA1 LOSE !! " ));
                    return FALSE;                     
                }
                    
            }while( bIsDSPBusy == DSP_BUSY );
        }
    }
    
    insufficient_block = length%4; /*If data length is insufficient a block, */
    if ( insufficient_block != 0 )        
    {
        do{

            if (SPI1_writeBuffer((data+(i*4)),insufficient_block, m_endianityMode)== SPI_RET_ERROR_LEN)
            {
                SPI_unselectChip(CS498xx_SPI_NPCS);
                ret = FALSE;
                return ret;
            }
            bIsDSPBusy = (bool)CS498xx_SPI_IsDSPBusy(m_writeCmdTimeOut);

            if ( bIsDSPBusy == DSP_BUSY )
            {
                busyCnt ++;

                if ( busyCnt >= 10 )
                {
                    //work around solution
                    TRACE_ERROR((0, " SPI DATA2 LOSE !! " ));                    
                    return FALSE;
                }
            }
            
        }while( bIsDSPBusy == DSP_BUSY );
    }
    
    // Unselect SPI chip.
    SPI_unselectChip(CS498xx_SPI_NPCS);
    return ret;

}
#else
bool CS498xx_SPI_write_buffer(byte* data, uint16 length)
{
    TRACE_DEBUG((0, "CS498xx_ITF has not been definied !! "));
    return FALSE;
}

#endif 
//!    @}

//!    CS498xx_ interface enable    @{
#ifdef CS498xx_ITF
bool CS498xx_SPI_read_buffer_NonIRQ(byte* data, uint16 length )
{
    bool ret = TRUE;
    uint16    block_num = 0;
    int i = 0;
    uint8 count = 0;

    // Select SPI chip.
    SPI_selectChip(CS498xx_SPI_NPCS);

    /*Write address of chip*/
    if ( SPI1_write(SLAVE_ADDRESS_R_CS498xx) != TRUE)
    {
        SPI_unselectChip(CS498xx_SPI_NPCS);
        ret = FALSE;
        return ret;
    }
    else
    {
        /*!< Wait to receive a byte */
        while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_RXNE) == RESET)
        {
            count ++;

            if ( count >= 10 )
            {
                count = 0;
                //TRACE_ERROR((0, "Read NonIRQ SPI INTERFACE HANG UP 1 !!"));
                
                break;
            }
        }

        SPI_ReceiveData8( _SPI );
    }

    //! Read 0xFF for SLAVE_ADDRESS_R_CS498xx by Smith@ Crash point2{
    /*!< Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_RXNE) == RESET)
    {
        count ++;

        if ( count >= 10 )
        {
            count = 0;
            //TRACE_ERROR((0, "Read NonIRQ SPI INTERFACE HANG UP 2 !!"));
            break;
        }
    }

    SPI_ReceiveData8( _SPI );
    //! @}

    /*Write 4 data bytes*/
    block_num = length/4; /*1 block is 4 bytes*/
    for ( i=0; i < block_num; i++ )
    {
        if (SPI1_readBuffer((data+(i*4)),4, m_endianityMode) == SPI_RET_ERROR_LEN )
        {
            SPI_unselectChip(CS498xx_SPI_NPCS);
            ret = FALSE;
            return ret;
        }
    }
    
    /*Unselect SPI chip.*/
    SPI_unselectChip(CS498xx_SPI_NPCS);
    return ret;
}


bool CS498xx_SPI_read_buffer(byte* data, uint16 length)
{
    bool ret = TRUE;
    uint16 block_num = 0;
    int i = 0;

    if ( CS498xx_SPI_IsDSP_IRQ_LOW(m_writeCmdTimeOut) == FALSE )
        return FALSE;

    // Select SPI chip.
    SPI_selectChip(CS498xx_SPI_NPCS);

    /*Write address of chip*/
    if ( SPI1_write(SLAVE_ADDRESS_R_CS498xx) != TRUE)
    {
        SPI_unselectChip(CS498xx_SPI_NPCS);
        ret = FALSE;
        return ret;
    }

    /*Write 4 data bytes*/
    block_num = length/4; /*1 block is 4 bytes*/
    for ( i=0; i < block_num; i++ )
    {
        if (SPI1_readBuffer((data+(i*4)),4, m_endianityMode) == SPI_RET_ERROR_LEN )
        {
            SPI_unselectChip(CS498xx_SPI_NPCS);
            ret = FALSE;
            return ret;
        }
    }
    
    /*Unselect SPI chip.*/
    SPI_unselectChip(CS498xx_SPI_NPCS);
    return ret;

}
#else
bool CS498xx_SPI_read_buffer(byte* data, uint16 length)
{
    TRACE_DEBUG((0, "CS498xx_ITF has not been definied !! "));
    return FALSE;
}

#endif 
//!    @}


bool CS498xx_SPI_isEnable( void )
{
    return SPI1_LowLevel_isEnable( );    
}


bool CS498xx_SPI_CommandWrite(uint32 cmd, uint32 value)
{
    uint32 command = cmd;
    uint32 val = value;
    bool ret;

    ret = CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));

    if ( ret == FALSE )
        return ret;
    
    ret = CS498xx_SPI_write_buffer((byte*)&val, (sizeof(val)/sizeof(uint8)));
    if ( ret == FALSE )
        return ret;
    
    return TRUE;
}

bool CS498xx_SPI_ReadSolicited( uint32 cmd, uint32 *pValue )
{
    bool ret;
    uint32 command = cmd;
    uint32 value = 0;

    if ( pValue == NULL )
        return FALSE;

    ret = CS498xx_SPI_write_buffer((byte*)&command, (sizeof(command)/sizeof(uint8)));

    if ( ret == FALSE )
        return ret; 

    /*Read response command word*/
    ret = CS498xx_SPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8));
    if ( ret == FALSE )
        return ret;

    /*read respones data word*/
    ret = CS498xx_SPI_read_buffer((byte*)&value, sizeof(value)/sizeof(uint8));
    if ( ret == FALSE )
        return ret;

    *pValue = value;
        
    return TRUE;
}
