#include "Debug.h"
#include "CS498xx_spi.h"
#include "CS498xx.h"
#include "STM32F0xx_board.h"

#include "audio_dsp_uld.h"

bool CS498xx_AC3_Initial(void);

//____________________________________________________________________________@{
#define _TSHD_TSHD4_OUTPUT_MODE    0xb3000005

//____________________________________________________________________________@{

//____________________________________________________________________________@{
static bool buffer_write_dsp_CS498xx_AC3( void );
static bool buffer_write_dsp_CS498xx_AC3_ULD( void );

//____________________________________________________________________________@{
//Smith implemented for new DSP architecture
void CS498xx_AC3_PreAttenuationControl( uint8 val )
{
    if ( val == 0x01 )
    {
        //CS498xx_SPI_CommandWrite( 0xd4000010, 0x7fffffff );
        //CS498xx_SPI_CommandWrite( 0xd4000012, 0x7fffffff );
        //CS498xx_SPI_CommandWrite( 0xd4000013, 0x7fffffff );

    }
    else if ( val == 0x00 )
    {
        //CS498xx_SPI_CommandWrite( 0xd4000010, 0x00000000 );
        //CS498xx_SPI_CommandWrite( 0xd4000012, 0x00000000 );
        //CS498xx_SPI_CommandWrite( 0xd4000013, 0x00000000 );
    }

    return;
}

void CS498xx_ProcessMode_AC3_DownMix_21( uint8 val )
{
    if ( (val&0x01) )
    {
        CS498xx_SPI_write_buffer((byte*)&AC3_5_1_TVOLOFF, (sizeof(AC3_5_1_TVOLOFF)/sizeof(uint8)) );
        TRACE_DEBUG((0, " AC3 5.1 "));
    }
    else if ((val&0x01) == 0)
    {
        CS498xx_SPI_write_buffer((byte*)&AC3_2_1_TVOLOFF, (sizeof(AC3_2_1_TVOLOFF)/sizeof(uint8)) );
        TRACE_DEBUG((0, " AC3 2.1 "));
    }
}

void CS498xx_ProcessMode_AC3_TVHDMC_Ctrl( uint8 val )
{
    if (FALSE == CS498xx_softboot())
    {
        TRACE_ERROR((0, "CS498xx_softboot ERROR !! "));
        return;
    }
    
    if ( CS498xx_PreKickStart() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_PreKickStart ERROR !! "));
        return;
    }
    
    TRACE_DEBUG((0, " AC3 Init "));
    CS498xx_AC3_Initial(); 


    if ( (val&0x01) )
    {
        if( (val&0x02) )
        {
            CS498xx_SPI_write_buffer((byte*)&AC3_5_1_TVOLON, (sizeof(AC3_5_1_TVOLON)/sizeof(uint8)) );
            
            TRACE_DEBUG((0, " AC3 5.1 TVHDMC_ENABLE ON "));
        }
        else if ( (val&0x02) == 0)
        {
            CS498xx_SPI_write_buffer((byte*)&AC3_5_1_TVOLOFF, (sizeof(AC3_5_1_TVOLOFF)/sizeof(uint8)) );
            TRACE_DEBUG((0, " AC3 5.1 TVHDMC_ENABLE OFF "));
        }

    }
    else if ((val&0x01) == 0)
    {
        if( (val&0x02) )
        {
            CS498xx_SPI_write_buffer((byte*)&AC3_2_1_TVOLON, (sizeof(AC3_2_1_TVOLON)/sizeof(uint8)) );
            
            TRACE_DEBUG((0, " AC3 2.1 TVHDMC_ENABLE ON "));
        }
        else if ( (val&0x02) == 0)
        {
            CS498xx_SPI_write_buffer((byte*)&AC3_2_1_TVOLOFF, (sizeof(AC3_2_1_TVOLOFF)/sizeof(uint8)) );
            TRACE_DEBUG((0, " AC3 2.1 TVHDMC_ENABLE OFF "));
        }
    }

    if ( CS498xx_KickStart() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_KickStart ERROR !! "));
        return;
    }

}


static bool buffer_write_dsp_CS498xx_AC3_ULD( void )
{

    bool ret = TRUE;   
#if 0      
    uint32 value = 0;

    mCMD = CMD_SLAVE_BOOT;
        
    /*write slave boot*/
    CS498xx_SPI_write_buffer((byte*)&mCMD, (sizeof(mCMD)/sizeof(uint8)));
        
    /*wait 100ms*/
    VirtualTimer_sleep(10);

    /*Read message; NOTE 1. Read four bytes from the DSP. IRQ will not drop for this read sequence.*/
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if ( value != STATE_BOOT_START)
    {
        ret = FALSE;
        
        TRACE_DEBUG((0, "BOOT START STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx OS Slave boot start fail"));
        return ret; 
    }

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    CS498xx_SPI_write_buffer((byte*)&AC3_IN_GAIN_ULD, (sizeof(AC3_IN_GAIN_ULD)/sizeof(uint8)) );
    
    VirtualTimer_sleep(20);
       /*Read message */
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if (value != STATE_BOOT_SUCCESS )
    {
        ret = FALSE;
        TRACE_DEBUG((0, "BOOT FAIL STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx write TV CSII ULD fail"));
        return ret; 
    }

    TRACE_INFO((0, "buffer_write_dsp_CS498xx_AC3_ULD Finished !! "));
#endif
    return ret;
}



static bool buffer_write_dsp_CS498xx_AC3( void )
{
    bool ret = TRUE;  
#if 0      
    uint32 value = 0;

    mCMD = CMD_SLAVE_BOOT;
        
    /*write slave boot*/
    CS498xx_SPI_write_buffer((byte*)&mCMD, (sizeof(mCMD)/sizeof(uint8)));
        
    /*wait 100ms*/
    VirtualTimer_sleep(10);

    /*Read message; NOTE 1. Read four bytes from the DSP. IRQ will not drop for this read sequence.*/
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if ( value != STATE_BOOT_START)
    {
        ret = FALSE;
        
        TRACE_DEBUG((0, "BOOT START STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx OS Slave boot start fail"));
        return ret; 
    }

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    CS498xx_SPI_write_buffer((byte*)&AC3_RC23_ULD, (sizeof(AC3_RC23_ULD)/sizeof(uint8)) );
    
    VirtualTimer_sleep(20);
       /*Read message */
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if (value != STATE_BOOT_SUCCESS )
    {
        ret = FALSE;
        TRACE_DEBUG((0, "BOOT FAIL STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "AC3_A_495304_20_01_RC23_ULD fail"));
        return ret; 
    }

    TRACE_INFO((0, "AC3_A_495304_20_01_RC23_ULD Finished !! "));
#endif
    return ret;
}

bool CS498xx_AC3_Initial(void) 
{
    bool ret = TRUE;

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    ret = CS498xx_SPI_write_buffer((byte*)&AC3_INITIAL_CFG, (sizeof(AC3_INITIAL_CFG)/sizeof(uint8)) );

    return ret;
}

bool CS498xx_AC3_Input( void )
{
    bool ret = TRUE;

    //Mike, 130401, change the sequence, ac3 > crossbar

    if ( buffer_write_dsp_CS498xx_AC3() != TRUE )
    {
        TRACE_ERROR((0, " buffer_write_dsp_CS498xx_AC3 ERROR !! "));

        ret = FALSE;
        return ret;
    }
    
    if ( buffer_write_dsp_CS498xx_CrossBar() != TRUE )
    {
        TRACE_ERROR((0, " buffer_write_dsp_CS498xx_CrossBar ERROR !! "));

        return FALSE;
    }    

    if ( buffer_write_dsp_CS498xx_AC3_ULD() != TRUE )
    {
        TRACE_ERROR((0, " buffer_write_dsp_CS498xx_AC3_ULD ERROR !! "));

        ret = FALSE;
        return ret;
    }

    return ret;
}

bool CS498xx_Initial_AC3_uld( void )
{
    //! loading ULD @{
    if( CS498xx_OS() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_OS_AC3 ERROR !! "));
        return FALSE;
    }

    //!  DECODER    @{
    if (CS498xx_AC3_Input( ) != TRUE )
    {
        return FALSE;
    }
    //!@}

    if ( CS498xx_PreKickStart() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_PreKickStart ERROR !! "));
        return FALSE;
    }

    //! *INITIAL @{
    if (CS498xx_AC3_Initial( ) != TRUE )
    {
        return FALSE;
    }

    //! @}

    if ( CS498xx_KickStart() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_KickStart ERROR !! "));
        return FALSE;
    }

    return TRUE;
}

