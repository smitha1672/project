#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"
#include "GPIOMiddleLevel.h"

#include "AudioDeviceManager.h"

#include "CS8422.h"

//! < Definition_________________________________________________________

//#define CS8422_CHIP_ID        0x01
#define CS8422_CLOCK_CONTROL            0x02 //0x22->0x20
#define CS8422_RECEIVER_INPUT_CONTROL   0x03 //0x80
#define CS8422_RECEIVER_DATA_CONTROL    0x04 //0x0
#define CS8422_GPIO_CONTROL1            0x05 //0x03
#define CS8422_GPIO_CONTROL2            0x06 //0x0
#define CS8422_SAI_CLOCK_CONTROL        0x07 //0x48    
#define CS8422_SRC_SAO_CLOCK_CONTROL    0x08 //0x4a
#define CS8422_RMCK_CNTL_MISC           0x09 //0x40
#define CS8422_DATA_ROUNTING_CONTROL    0x0A //0x52 ->0x40
#define CS8422_SAI_DATA_FORMAT          0x0B //0x88
#define CS8422_SAO1_DATA_FORMAT_TDM     0x0C //0x84
#define CS8422_SAO2_DATA_FORMAT         0x0D //x084
#define CS8422_PERR_UNMASKING           0x0E
#define CS8422_INTERRUPT_UNMASKING      0x0F
#define CS8422_INTERRUPT_MODE           0x10
#define CS8422_RECEIVER_CHANNEL_STATUS  0x11
#define CS8422_FORMAT_DETECT_STATUS     0x12
#define CS8422_RECEIVER_ERROR           0x13
#define CS8422_INTERRUP_STATUS          0x14
#define CS8422_PLL_STATUS               0x15
#define CS8422_RECEIVER_STATUS          0x16
//#define CS8422_FS_XTI_RATIO1          0x17
//#define CS8422_FS_XTI_RATIO2          0x18
#define CS8422_IEC61937_BURST_PREAMBLE  0x2D

#define PCM_96KHZ_DETECTION_IO    __I_CS8422_GPO_0
#define RECEIVER_ERROR_IO    __I_CS8422_GPO_1

//_____________________________________________________________________________________
extern void CS8422_ISR_Control( bool bIsrEnable );

static void CS8422_SRC_Initialize(void);

static void CS8422_SRC_setPath_Rx( uint8 rx_path, uint8 src_ctrl );

static void CS8422_SRC_setPath_SerialAudioInput(void);

static bool CS8422_IsPLL_Lock( void );

static bool CS8422_getISCLK_ACTIVE(void);

static bool CS8422_Format_IsPCM( void );

static bool CS8422_is_96Khz192Khz_AES3( void );

static uint8 CS8422_getForcesClockControl( void );

static void CS8422_ForcesClockControl( uint8 index );

static uint8 CS8422_read_SDOUT1_DataSource( void );

static void CS8422_SerialAudioOutput( uint8 index );

const SRC_CTRL_OBJECT SRCLowLevel_ObjCtrl = 
{
    CS8422_SRC_Initialize,
    CS8422_ISR_Control,
    CS8422_SRC_setPath_Rx,
    CS8422_SRC_setPath_SerialAudioInput,
    CS8422_IsPLL_Lock,
    CS8422_getISCLK_ACTIVE,
    CS8422_Format_IsPCM,
    CS8422_is_96Khz192Khz_AES3,
    CS8422_getForcesClockControl, 
    CS8422_ForcesClockControl,
    CS8422_read_SDOUT1_DataSource,
    CS8422_SerialAudioOutput,
};

const SRC_CTRL_OBJECT *pSRC_ObjCtrl = &SRCLowLevel_ObjCtrl;


//____________________________________________________________________________________________
static void CS8422_SerialAudioInput_SAI_MCLK_Control( uint8 index )
{
    uint8 ret = 0;

    ret = CS8422_ReadI2C_Byte(CS8422_SAI_CLOCK_CONTROL);

    switch( index )
    {
        case 0: /*XTI - XTO*/
        {
            ret &= 0xF0;

            ret |= 0x00;
            CS8422_WriteI2C_Byte(CS8422_SAI_CLOCK_CONTROL, ret);
            
        }
            break;
            
        case 1:    /*RMCK*/
        {
            ret &= 0xF0;

            ret |= 0x08;
            CS8422_WriteI2C_Byte(CS8422_SAI_CLOCK_CONTROL, ret);
        }
            break;
    }
}

static void CS8422_ForcesClockControl( uint8 index )
{
    uint8 ret = 0;

    switch( index )
    {
        case 0: /*Clock signal on XTI is output on RMCK according to the SWCLK bit functionality. */
        {
            ret = CS8422_ReadI2C_Byte(CS8422_CLOCK_CONTROL);
            ret &= 0xBF; 

            ret |=0x00;
            CS8422_WriteI2C_Byte( CS8422_CLOCK_CONTROL, ret );
            
        }
            break;

        case 1:    /*Forces the clock signal on XTI to be output on RMCK regardless of the SWCLK bit functionality.*/
        {
            ret = CS8422_ReadI2C_Byte(CS8422_CLOCK_CONTROL);
            ret &= 0xBF; 

            ret |=0x40;
            CS8422_WriteI2C_Byte( CS8422_CLOCK_CONTROL, ret );
        }
            break;
    }
}

static void CS8422_SerialAudioOutput( uint8 index )
{
    uint8 ret = 0;

    switch( index )
    {
        case 0:    /*SDOUT 1: Sample Rate Convert*/
        {
            ret = CS8422_ReadI2C_Byte(CS8422_DATA_ROUNTING_CONTROL);
            ret &= 0x3D;

            ret |=0x02;
            
            CS8422_WriteI2C_Byte( CS8422_DATA_ROUNTING_CONTROL, ret );
        }
            break;
    
        case 1:    /*SDOUT 1: AES3 Receiver Output*/
        {
            ret = CS8422_ReadI2C_Byte(CS8422_DATA_ROUNTING_CONTROL);
            ret &= 0x3D;

            ret |= 0x40;
            CS8422_WriteI2C_Byte( CS8422_DATA_ROUNTING_CONTROL, ret );
        }
            break;

        case 2:    /*SDOUT 1: SDIN Output*/
        {
            ret = CS8422_ReadI2C_Byte(CS8422_DATA_ROUNTING_CONTROL);
            ret &= 0x3D;

            ret |= 0x80;
            CS8422_WriteI2C_Byte( CS8422_DATA_ROUNTING_CONTROL, ret );
            
        }
            break;
    }
}

static void CS8422_ClockControl_RMCK( uint8 index )
{
    uint8 ret = 0;

    ret = CS8422_ReadI2C_Byte(CS8422_CLOCK_CONTROL);

    switch( index )
    {
        /*00 - RMCK is an output and is derived from the frame rate of incoming AES3 data.*/
        case 0:
        {
            ret &= 0xE7; 

            ret |= 0x00;
            CS8422_WriteI2C_Byte( CS8422_CLOCK_CONTROL, ret );
        }
            break;

        /*
            01 - RMCK is an output and is derived from the ISCLK input frequency divided by 64. Only valid if serial
            audio input port is in slave mode (SIMS = 0 in ¡§Serial Audio Input Data Format (0Bh)¡¨ on page 53).
        */
        case 1:
        {
            ret &= 0xE7; 

            ret |= 0x08;
            CS8422_WriteI2C_Byte( CS8422_CLOCK_CONTROL, ret );
        }
            break;

        /*10 - RMCK is high-impedance.*/
        case 2:
        {
            /*TBD*/
        }
            break;
    }
}

static void CS8422_ReceiverInputControl( uint8 index )
{
    uint8 ret = 0;

    switch( index )
    {
        case 0:    /*RX0*/
        {    
            ret = CS8422_ReadI2C_Byte(CS8422_RECEIVER_INPUT_CONTROL);
            ret &= 0x9F;

            ret |= 0x00;    
            CS8422_WriteI2C_Byte( CS8422_RECEIVER_INPUT_CONTROL, ret );
        }
            break;
            
        case 1: /*RX1*/
        {    
            //! Set Receiver Input Control@{
            ret = CS8422_ReadI2C_Byte(CS8422_RECEIVER_INPUT_CONTROL);
            ret &= 0x9F;

            ret |= 0x20;
            CS8422_WriteI2C_Byte( CS8422_RECEIVER_INPUT_CONTROL, ret );
            //!@}

        }
            break;
            
        case 2: /*RX2*/
        {
            ret = CS8422_ReadI2C_Byte(CS8422_RECEIVER_INPUT_CONTROL);
            ret &= 0x9F;

            ret |=0x40;
            CS8422_WriteI2C_Byte( CS8422_RECEIVER_INPUT_CONTROL, ret );
        }
            break;
            
        case 3: /*RX3*/
        {
            ret = CS8422_ReadI2C_Byte(CS8422_RECEIVER_INPUT_CONTROL);
            ret &= 0x9F;

            ret |=0x60;
            CS8422_WriteI2C_Byte( CS8422_RECEIVER_INPUT_CONTROL, ret );
        }
            break;

    }
}

static bool CS8422_getISCLK_ACTIVE(void)
{
    int8 val =0;
    
    val = CS8422_ReadI2C_Byte(CS8422_PLL_STATUS);
    val &= 0x40;
    if (val)
        return TRUE;
    return FALSE;
}

uint8 CS8422_getReceiverError( void )
{
    uint8 ret = 0;
    
    ret = CS8422_ReadI2C_Byte( CS8422_RECEIVER_ERROR );
    return ret;
}

static bool CS8422_Format_IsPCM( void )
{
    uint8 val = 0;
    
    val = CS8422_ReadI2C_Byte( CS8422_FORMAT_DETECT_STATUS );
    
    val &= 0x80;
    if ( val )
        return TRUE;

    return FALSE;
}

bool CS8422_Format_IsIE61937( void )
{
    uint8 val = 0;
    
    val = CS8422_ReadI2C_Byte( CS8422_FORMAT_DETECT_STATUS );
    val &= 0x40;

    if ( val )
        return TRUE;

    return FALSE;
}

bool CS8422_Format_IsAC3( void )
{
    uint8 val = 0;
    
    val = CS8422_ReadI2C_Byte( CS8422_IEC61937_BURST_PREAMBLE );
    val &= 0x0F;
    
    if ( val == 0x01 )
        return TRUE;

    return FALSE;
}

bool CS8422_Format_IsDTS( void )
{
    uint8 val = 0;
    
    val = CS8422_ReadI2C_Byte( CS8422_IEC61937_BURST_PREAMBLE );
    val &= 0xFF;

    if ((val == 0x0B )||(val==0x0C)||(val==0x0D)||(val==0x11))
        return TRUE;

    return FALSE;
}

static bool CS8422_IsPLL_Lock( void )
{
    uint8 val = 0;

    val = CS8422_ReadI2C_Byte(CS8422_PLL_STATUS);

    if (val == 0xFF)
    {
        return FALSE;
    }
    else
    {
        val &= 0x20;

        if ( val )
        {
            return TRUE;
        }
    
    return FALSE;
    }    
}


byte CS8422_Read_Receiver_Status(void)
{
    uint8 val;
   
    val=CS8422_ReadI2C_Byte(CS8422_RECEIVER_STATUS);
    val &=0x60;
    
    return val;
    
}

byte CS8422_Read_Input_Status(void)
{
    uint8 val;
   
    val=CS8422_ReadI2C_Byte(CS8422_DATA_ROUNTING_CONTROL);
    val &=0xC0;
    
    return val;
    
}

byte CS8422_Format_Detect_Status(void)
{
    uint8 val;
   
    val=CS8422_ReadI2C_Byte(CS8422_FORMAT_DETECT_STATUS);
    val &=0xFF;
    
    return val;
}

static void CS8422_SRC_setPath_Rx( uint8 rx_path, uint8 src_ctrl )
{
    if ( rx_path > 3 )
        return;

    if ( src_ctrl > 1 )
        return;

    CS8422_ClockControl_RMCK( 0 );
    CS8422_ReceiverInputControl( rx_path );
    CS8422_SerialAudioInput_SAI_MCLK_Control( 1 );

    if ( src_ctrl == 1 ) /* SRC enable */
    {
        CS8422_ForcesClockControl( 1 );
        CS8422_SerialAudioOutput( 0 );
    }
    else
    {   
        CS8422_ForcesClockControl( 0 );
        CS8422_SerialAudioOutput( 1 );
    }
}

static void CS8422_SRC_setPath_SerialAudioInput(void)
{
    CS8422_ClockControl_RMCK( 1 ); /*fix analog source outputs "PO PO" noise*/
    CS8422_SerialAudioInput_SAI_MCLK_Control( 0 );    /*fix If optical doesn't connect to DVD player, analog source will lose audio.*/
    CS8422_SerialAudioOutput( 2 );    /*Set serial data output */
}

void CS8422_init(void)
{
     // Here is for crystal 12.288 Mhz @{
    CS8422_WriteI2C_Byte(0x00,0x00);
    CS8422_WriteI2C_Byte(0x01,0x10);

/*    Smith comment: 2013/3/15
    Clock Control (02h)
    FSWCLK[6] ¡V Forces the clock signal on XTI to be output on RMCK regardless of the SWCLK bit functionality or PLL lock.
        0 ¡V Clock signal on XTI is output on RMCK according to the SWCLK bit functionality.
        1 ¡V Forces the clock signal on XTI to be output on RMCK regardless of the SWCLK bit functionality.
        
    SWCLK[5] - Outputs XTI clock signal on RMCK pin when PLL loses lock. Any OSCLK or OLRCK derived from RMCK under normal conditions will be derived from XTI in this case. 
        0 - Disable automatic clock switching. 
        1 - Enable automatic clock switching on PLL unlock. Clock signal selected on XTI is automatically output on RMCK on PLL Unlock.

       INT[1:0] - Interrupt output pin (INT) control
       10 - Open drain, active low. Requires an external pull-up resistor on the INT pin.    
*/    
    //CS8422_WriteI2C_Byte(0x02,0x64); 
    CS8422_WriteI2C_Byte(0x02,0x62); 

    /*Receiver Input Control*/
    CS8422_WriteI2C_Byte(0x03,0x80);
    CS8422_WriteI2C_Byte(0x04,0x04); 

    /*CS8422 GPO0 is 96KHZ indicator; GPO1 is receiver error indicator*/
    //CS8422_WriteI2C_Byte(0x05,0x96);    
    CS8422_WriteI2C_Byte(0x05,0x90);    

    /*GPO2 is for interrupt pin*/
    //CS8422_WriteI2C_Byte(0x06,0x3E);
    CS8422_WriteI2C_Byte(0x06,0x30);

    
    /* Serial Audio Input Clock Control*/
    CS8422_WriteI2C_Byte(0x07,0x48);

    /*SAO_MCLK[3] Selects the MCLK as XTI/O*/
    CS8422_WriteI2C_Byte(0x08,0x40);

    /* Recevered MasterClock Ration Control */
    CS8422_WriteI2C_Byte(0x09,0x48);

    /*Data Routing Control: SRCD[1] 0- Serial Audio Input port(SDIN); 1-AES3 Receiver Output*/
    CS8422_WriteI2C_Byte(0x0A,0x52);    

    /*Serial Audio Input Data Format as 48Khz*/
    CS8422_WriteI2C_Byte(0x0B,0x88);

    /*SDOUT1 - Master mode; I2S output*/
    CS8422_WriteI2C_Byte(0x0C,0x84);

    /*SDOUT2*/
    CS8422_WriteI2C_Byte(0x0D,0x80);

    /*Receiver Error Unmasking*/
    CS8422_WriteI2C_Byte(0x0E,0xFF);

    /*INTERRUPT UNMASK*/
    CS8422_WriteI2C_Byte(0x0F, 0x0A);

    /*Set ERROR is Falling edge*/
    CS8422_WriteI2C_Byte(0x10,0x05);
    
    CS8422_WriteI2C_Byte(0x16,0x10);
}

static void CS8422_SRC_Initialize( void )
{
    CS8422_init();  
}

void CS8422_SoftReboot(void)
{
    uint8 ret = 0;
    uint8 ret1 = 0;
    uint8 ret2 = 0;
    uint8 ret3 = 0;

    //Load current settings
    ret1 = CS8422_ReadI2C_Byte(CS8422_RECEIVER_INPUT_CONTROL);
    ret2 = CS8422_ReadI2C_Byte(CS8422_DATA_ROUNTING_CONTROL);
    ret3 = CS8422_ReadI2C_Byte(CS8422_SAI_CLOCK_CONTROL);

    //CS8422 Off
    ret = CS8422_ReadI2C_Byte(CS8422_CLOCK_CONTROL);
    ret &=0x7f;
    CS8422_WriteI2C_Byte( CS8422_CLOCK_CONTROL, ret );

#if defined ( FREE_RTOS )    
    vTaskDelay(TASK_MSEC2TICKS(2));
#else
    VirtualTimer_sleep(2);
#endif 


    //CS8422 On
    ret = CS8422_ReadI2C_Byte(CS8422_CLOCK_CONTROL);
    ret |=0x80;
    CS8422_WriteI2C_Byte( CS8422_CLOCK_CONTROL, ret );
    
#if defined ( FREE_RTOS )    
    vTaskDelay(TASK_MSEC2TICKS(2));
#else
    VirtualTimer_sleep(2);
#endif 
    
    CS8422_init(); 

    //Set the preloaded settings
    CS8422_WriteI2C_Byte(CS8422_RECEIVER_INPUT_CONTROL, ret1 );
    CS8422_WriteI2C_Byte(CS8422_DATA_ROUNTING_CONTROL, ret2 );
    CS8422_WriteI2C_Byte(CS8422_SAI_CLOCK_CONTROL, ret3);

    TRACE_DEBUG((0, "CS8422 initial!! CS8422_SPDIF_In_To_SPDIF_And_PCM_Out"));
}

//________________________________________________________________________________________________
static uint8 CS8422_getForcesClockControl( void )
{
    uint8 ret = 0;

    ret = CS8422_ReadI2C_Byte(CS8422_CLOCK_CONTROL);
    ret &= 0x40;

    ret >>= 6;

    return ret;
}

/*Smith implemented: 2013/3/18*/
static uint8 CS8422_read_SDOUT1_DataSource( void )    
{
    uint8 ret = 0;

    ret = CS8422_ReadI2C_Byte(CS8422_DATA_ROUNTING_CONTROL);

    ret &= 0xC0; 

    ret >>= 6;
    
    /*
        SDOUT1[1:0] - Controls the data source for SDOUT1
        00 - Sample Rate Converter
        01 - AES3 Receiver Output
        10 - SDIN (SDIN and SDOUT should be synchronous)
        11 - Reserved
    */

    return ret;
}

bool CS8422_is_96Khz( void )
{
    if ( GPIOMiddleLevel_Read_InBit(PCM_96KHZ_DETECTION_IO) )
        return TRUE;

    return FALSE;
}

bool CS8422_is_ReceiverError( void )
{
    if ( GPIOMiddleLevel_Read_InBit( RECEIVER_ERROR_IO ) )
        return TRUE;

    return FALSE;
}

static bool CS8422_is_96Khz192Khz_AES3( void )
{
    uint8 val = 0;

    val = CS8422_ReadI2C_Byte(CS8422_PLL_STATUS);
    val &= 0x18; /*Get 96Khz and 192Khz from 15h*/

    if ( val )
    {
        return TRUE;
    }
    
    return FALSE;
}
