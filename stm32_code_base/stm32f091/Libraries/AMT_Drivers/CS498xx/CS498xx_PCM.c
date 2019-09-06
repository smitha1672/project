#include "Debug.h"
#include "CS498xx_spi.h"
#include "CS498xx.h"
#include "STM32F0xx_board.h"

//! Smith implemented for CS498xx@{
#include "audio_dsp_uld.h"
#include "audio_gain_parameters.h"
//! @}

//____________________________________________________________________________@{
//Smith implemented for new DSP architecture
#if 0
static bool buffer_write_dsp_CS498xx_TV_CSII (void);
#endif

static bool buffer_write_dsp_CS498xx_PCM_Input( void );

static bool buffer_write_dsp_CS498xx_PCM_Initial(void);
//!@}
//____________________________________________________________________________@{
//Smith implemented for new DSP architecture
void CS498xx_PCM_PreAttenuationControl( uint8 val )
{
    if ( val == 0x01 )
    {
        CS498xx_SPI_CommandWrite( 0xd4000010, 0x7fffffff );
        CS498xx_SPI_CommandWrite( 0xd4000012, 0x7fffffff );
        CS498xx_SPI_CommandWrite( 0xd4000013, 0x7fffffff );

    }
    else if ( val == 0x00 )
    {
        CS498xx_SPI_CommandWrite( 0xd4000010, 0x00000000 );
        CS498xx_SPI_CommandWrite( 0xd4000012, 0x00000000 );
        CS498xx_SPI_CommandWrite( 0xd4000013, 0x00000000 );
    }

    return;
}
#if 0
static bool buffer_write_dsp_SRS_51_TSHD( void )
{
    bool ret = TRUE;

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    ret = CS498xx_SPI_write_buffer((byte*)&PCM_INITIAL, (sizeof(PCM_INITIAL)/sizeof(uint8)));
    if ( ret == TRUE )    
        TRACE_INFO((0, "PCM 5.1 TRUVOL OFF Finished !! "));
    
    return ret;
}
#endif

static bool buffer_write_dsp_CS498xx_PCM_Initial(void) 
{
    bool ret = TRUE;

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    ret = CS498xx_SPI_write_buffer((byte*)&PCM_INITIAL, (sizeof(PCM_INITIAL)/sizeof(uint8)) );
    if ( ret == TRUE )    
        TRACE_INFO((0, "INITIAL_CFG Finished !! "));
    
    return ret;
}



static bool buffer_write_dsp_CS498xx_PCM_Input (void) /*I don't understand why naming is by VIZIO Jason*/
{
#if 0  
    uint32 value = 0;

        mCMD = CMD_SLAVE_BOOT;
        /*write slave boot*/
    CS498xx_SPI_write_buffer((byte*)&mCMD, (sizeof(mCMD)/sizeof(uint8)));
        
    /*wait 100ms*/
    VirtualTimer_sleep(10);

    /*Read message; NOTE 1. Read four bytes from the DSP. IRQ will not drop for this read sequence.*/
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if ( value != STATE_BOOT_START )
    {
        TRACE_DEBUG((0, "BOOT START STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx OS Slave boot start fail"));
        return FALSE; 
    }

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    CS498xx_SPI_write_buffer((byte*)&PCM_IN_GAIN_ULD, (sizeof(PCM_IN_GAIN_ULD)/sizeof(uint8)) );
    
    VirtualTimer_sleep(20);
       /*Read message */
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if (value != STATE_BOOT_SUCCESS )
    {
        TRACE_DEBUG((0, "BOOT FAIL STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        return FALSE; 
    }
#endif
      TRACE_INFO((0, "PCM_IN_SYSTEM16MB_B_495314_20_0100_ULD Finished !! "));
    return TRUE;
}

#if 0
static bool buffer_write_dsp_CS498xx_TV_CSII (void)
{
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
        TRACE_DEBUG((0, "BOOT START STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx OS Slave boot start fail"));
        return FALSE; 
    }

    /*Write BOOT_ASSIT_A.uld file or boot_assist_xtal_div2_a*.uld*/ /*Smith implemented */
    //CS498xx_SPI_write_buffer((byte*)&TV_CS2_48_A_495314_20_00_RC4_ULD, (sizeof(TV_CS2_48_A_495314_20_00_RC4_ULD)/sizeof(uint8)) );
    
    CS498xx_SPI_write_buffer((byte*)&PCM_TVCSII_ULD, (sizeof(PCM_TVCSII_ULD)/sizeof(uint8)) );
    
    VirtualTimer_sleep(20);
       /*Read message */
    CS498xx_SPI_read_buffer((byte*)&value,sizeof(value));
    if (value != STATE_BOOT_SUCCESS )
    {
        TRACE_DEBUG((0, "BOOT FAIL STATUS = 0x%X%X", GET_HIGH_U16(value), GET_LOW_U16(value)));
        TRACE_ERROR((STR_ID_NULL, "CS498xx write TV CSII ULD fail"));
        return FALSE; 
    }

     TRACE_INFO((0, "TV_CS2_48_A_495314_20_00_RC6_ULD Finished !! "));

    return TRUE;
}
#endif

bool CS498xx_PCM_Initial( void )
{
   //bool ret = TRUE;

   if( buffer_write_dsp_CS498xx_PCM_Initial() != TRUE)
   {
        TRACE_ERROR((0, " buffer_write_dsp_CS498xx_PCM_Initial ERROR !! "));
        return FALSE; 
   }

    return TRUE;
}

bool CS498xx_PCM_Input( void )
{
    if ( buffer_write_dsp_CS498xx_PCM_Input() != TRUE )
    {
        TRACE_ERROR((0, " buffer_write_dsp_CS498xx_PCM_Input !! "));

        return FALSE;
    }

    return TRUE;
}

bool CS498xx_Initial_PCM_uld( void )
{
    //! loading ULD @{
    if( CS498xx_OS() != TRUE )
    {
        TRACE_ERROR((0, " CS498xx_OS_AC3 ERROR !! "));
        return FALSE;
    }

    //!  DECODER    @{
    if (CS498xx_PCM_Input( ) != TRUE )
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
    if (CS498xx_PCM_Initial( ) != TRUE )
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


