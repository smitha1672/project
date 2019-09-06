#include "Debug.h"
#include "CS5346.h"

#define CS5346_CHIP_ID		0x01
#define CS5346_POWER_CONTROL		0x02
#define CS5346_ADC_CONTROL		0x04
#define CS5346_MCLK_FREQUENCY		0x05
#define CS5346_PGAOUT_CONTROL		 0x06
#define CS5346_PGA_CH_B_GAIN_CONTROL	 0x07
#define CS5346_PGA_CH_A_GAIN_CONTROL	 0x08
#define CS5346_ANALOG_INPUT_CONTROL		 0x09
#define CS5346_ACTIVE_LEVEL_CONTROL		0x0C
#define CS5346_INTERRUPT_STATUS		0x0D
#define CS5346_INTERRUPT_MASK		0x0E
#define CS5346_INTERRUPT_MODE_MSB		0x0F
#define CS5346_INTERRUPT_MODE_LSB		0x10

//_____________________________________________________________________________________________________________________

typedef enum ADC_OUTPUT_FORMAT
{
    LEFT_JUSTIFIED,
    I2S
} ENUM_ADC_OUTPUT_FORMAT;

typedef enum ANALOG_INPUT
{
	ANALOG_INPUT_1,
	ANALOG_INPUT_2,
	ANALOG_INPUT_3,
	ANALOG_INPUT_4,
	ANALOG_INPUT_5,
	ANALOG_INPUT_6
} ENUM_ANALOG_INPUT;

typedef enum DIVIDER
{
	DIVIDER_1,
	DIVIDER_1_5,
	DIVIDER_2,
	DIVIDER_3,
	DIVIDER_4
} ENUM_DIVIDER;


//_____________________________________________________________________________________________________________________
static void CS5346_Initialize( void );

static void CS5346_setInputPath ( uint8 input ) ;

static void CS5346_set_AB_channels_gain( uint8 a_gain, uint8 b_gain );

const ADC_CTRL_OBJECT ADC_ObjCtrl = 
{
    CS5346_Initialize,
    CS5346_setInputPath,
    CS5346_set_AB_channels_gain,
};
const ADC_CTRL_OBJECT *pADC_ObjCtrl = &ADC_ObjCtrl;

//_____________________________________________________________________________________________________________________
void CS5346_PowerUpDown(uint8 bPower)
{
	uint8 val;

	if( bPower == 1 )
	{
        val=0x00;
	}
	else
	{	
        val=0x01;
	}
	
	CS5346_WriteI2C_Byte(CS5346_POWER_CONTROL,val);
}

void CS5346_Mute(uint8 bMute)
{
    uint8 val;

    val = CS5346_ReadI2C_Byte(CS5346_ADC_CONTROL);	
	
    val &= ~0x04;

	if(bMute == 1)
	{
        val |= 0x04;
	}
	else
	{
        val |= 0x00;
	}
	
	CS5346_WriteI2C_Byte(CS5346_ADC_CONTROL,val);

}

void CS5346_Select_Format(uint8 value)
{
	uint8 val;

	val=CS5346_ReadI2C_Byte(CS5346_ADC_CONTROL);	
	val &= ~0x10;

	switch(value) 
	{
	  case LEFT_JUSTIFIED:
		 val |= 0x00;
		break;
	  case I2S:
		 val |= 0x10;
		 break;
	}
	CS5346_WriteI2C_Byte(CS5346_ADC_CONTROL,val);

}

static void CS5346_set_AB_channels_gain( uint8 a_gain, uint8 b_gain )
{
    CS5346_WriteI2C_Byte(CS5346_PGA_CH_A_GAIN_CONTROL, a_gain);
    CS5346_WriteI2C_Byte(CS5346_PGA_CH_B_GAIN_CONTROL, b_gain); 
}

static void CS5346_setInputPath ( uint8 input ) 
{
    uint8 val = 0;
	 
	switch(input) 
	{
        case ANALOG_INPUT_1:
            val=0x19;
            break;
            
		case ANALOG_INPUT_2:
			 val=0x1A;
		    break;
            
		case ANALOG_INPUT_3:
		   val=0x1B;
		    break;
            
		case ANALOG_INPUT_4:
		   val=0x1C;
		    break;
            
		case ANALOG_INPUT_5:
		   val=0x1D;
		    break;
            
		case ANALOG_INPUT_6:
		   val=0x1E;
		    break;
	  }
	
	CS5346_WriteI2C_Byte(CS5346_ANALOG_INPUT_CONTROL, val );

}

uint8 CS5346_input_status (void) 
{
    uint8 val = 0;
    val = CS5346_ReadI2C_Byte(CS5346_ANALOG_INPUT_CONTROL);
	 
    switch(val) 
    {
        case 0x19:
            return ANALOG_INPUT_1;//AUX2
        break;
        case 0x1A:
            return ANALOG_INPUT_2;//
        break;
        case 0x1B:
            return ANALOG_INPUT_3;//AUX1
        break;
        case 0x1C:
            return ANALOG_INPUT_4;//BT
        break;
        case 0x1D:
            return ANALOG_INPUT_5;
        break;
        case 0x1E:
            return ANALOG_INPUT_6;//USB
        break;
        default:
            return ANALOG_INPUT_2; /* S3851 input 2 is empty. Tony140110 */
        break;
    }
}

void CS5346_Select_MASTER_SLAVE(uint8 value)
{
	uint8 val;

	val=CS5346_ReadI2C_Byte(CS5346_ADC_CONTROL);	
	val &= ~0x01;
	 
	switch(value) 
	{
	case 0:
		val|=0x00;
	break;
	case 1:
		val|=0x01;
	break;
	}

	CS5346_WriteI2C_Byte(CS5346_ADC_CONTROL,val);
	
}
void CS5346_Select_BCLK_SCLK(uint8 value)
{
	uint8 val;

	val=CS5346_ReadI2C_Byte(CS5346_ADC_CONTROL);	
	val &= ~0xC0;
	 
	switch(value) 
	{
	case 0:
		val|=0x00;  //SSM
	break;
	case 1:
		val|=0x40; //DSM
	break;
	case 2:
		val|=0x80;  //QSM
	break;
	}

	CS5346_WriteI2C_Byte(CS5346_ADC_CONTROL,val);
}

void CS5346_Select_Divider(uint8 value)
{
	uint8 val;

	val=CS5346_ReadI2C_Byte(CS5346_MCLK_FREQUENCY);	
	val &= ~0x70;
	 
	switch(value) 
	{
		case DIVIDER_1:
			val|=0x00;
		break;
		case DIVIDER_1_5:
			val|=0x10;
		break;
		case DIVIDER_2:
			val|=0x20;
		break;
		case DIVIDER_3:
			val|=0x30;
		break;
		case DIVIDER_4:
			val|=0x40;
		break;
	}

	CS5346_WriteI2C_Byte(CS5346_MCLK_FREQUENCY,val);
	
}

static void CS5346_Initialize( void )
{
    //
    // Send required startup commands, these are apparently required for ADC temperature stability
    //

    CS5346_PowerUpDown(0);
    CS5346_Mute(0);
    CS5346_setInputPath(ANALOG_INPUT_1);

    //0x00 ->0db , 0x06 ->3db
    CS5346_set_AB_channels_gain( 0x04, 0x04 );

    CS5346_Select_MASTER_SLAVE(0);
    CS5346_Select_Format(I2S);
    CS5346_Select_Divider(0);
    CS5346_PowerUpDown(1);
}

