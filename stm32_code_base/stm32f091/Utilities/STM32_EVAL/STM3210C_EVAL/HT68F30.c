#include "HT68F30.h"
#include "HT68F30_i2c.h"
//#include "Debug.h"

//#define LIGHT_BAR_INVERT

#define MAX_LEVEL_INDICATOR_NUM	12

#define LED_1 0x01
#define LED_2 0x02
#define LED_3 0x03
#define LED_4 0x04
#define LED_5 0x05
#define LED_6 0x06
#define LED_7 0x07
#define LED_8 0x08
#define LED_9 0x09
#define LED_10 0x0A
#define LED_11 0x0B
#define LED_12 0x0C

#define LIGHT_BAR_DUTY_MAX	0x78

#define DUTY_HIGH	50
#define DUTY_MID	10
#define DUTY_LOW	1 /*in IAP state.*/

#ifdef LIGHT_BAR_INVERT
#define LIGHT_BAR_DUTY_HIGH (0x78 - ( (LIGHT_BAR_DUTY_MAX*DUTY_HIGH) /100))
#define LIGHT_BAR_DUTY_MID (0x78 - ( (LIGHT_BAR_DUTY_MAX*DUTY_MID) /100))
#define LIGHT_BAR_DUTY_LOW (0x78 - ( (LIGHT_BAR_DUTY_MAX*DUTY_LOW) /100))
#define LIGHT_BAR_DUTY(X) (0x78 - ( (LIGHT_BAR_DUTY_MAX * X) /100))
#define LIGHT_BAR_DUTY_DARK (LIGHT_BAR_DUTY_MAX)
#else
#define LIGHT_BAR_DUTY_HIGH (0x00 + ((LIGHT_BAR_DUTY_MAX*DUTY_HIGH) /100))
#define LIGHT_BAR_DUTY_MID (0x00 + ( (LIGHT_BAR_DUTY_MAX*DUTY_MID) /100))
#define LIGHT_BAR_DUTY_LOW (0x00 + ((LIGHT_BAR_DUTY_MAX*DUTY_LOW) /100))
#define LIGHT_BAR_DUTY(X) (0x00 + ( (LIGHT_BAR_DUTY_MAX * X) /100))

#define LIGHT_BAR_DUTY_DARK (0x00)

#endif /*LIGHT_BAR_INVERT*/

#define LIGHT_BAR_ALL	0x0F
#define LIGHT_BAR_ALL_OFF	0x00
#define LIGHT_BAR_ALL_ON	0xFF

const static uint8 mLightBar_Array[MAX_LEVEL_INDICATOR_NUM] = {0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
const static uint8 mLightBar_Duty[3] = {LIGHT_BAR_DUTY_LOW, LIGHT_BAR_DUTY_MID, LIGHT_BAR_DUTY_HIGH};


//______________________________________________________________________________________________________
void HT68F30_LightBar_USB_UpgradingDone( void )
{
	uint8 duty = 0;

	duty = mLightBar_Duty[2];
	HT68F30_WriteI2C_Byte(mLightBar_Array[0], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[1], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[2], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[3], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[4], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[5], duty );

	HT68F30_WriteI2C_Byte(mLightBar_Array[6], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[7], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[8], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[9], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[10], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[11], duty );
}

void HT68F30_LightBar_EraseInternalFlash( void )
{
	uint8 duty = 0;

	duty = mLightBar_Duty[2];
	HT68F30_WriteI2C_Byte(mLightBar_Array[0], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[1], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[2], duty );
}

void HT68F30_LightBar_USB_isUpgrading( void )
{
	uint8 duty = 0;

	duty = mLightBar_Duty[2];
	HT68F30_WriteI2C_Byte(mLightBar_Array[0], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[1], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[2], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[3], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[4], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[5], duty );
}

void HT68F30_LightBar_EraseExtFlash( void )
{
	uint8 duty = 0;

	duty = mLightBar_Duty[2];
	HT68F30_WriteI2C_Byte(mLightBar_Array[0], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[1], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[2], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[3], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[4], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[5], duty );
    HT68F30_WriteI2C_Byte(mLightBar_Array[6], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[7], duty );
}

void HT68F30_LightBar_ProgramExtFlash( void )
{
	uint8 duty = 0;

	duty = mLightBar_Duty[2];
	HT68F30_WriteI2C_Byte(mLightBar_Array[0], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[1], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[2], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[3], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[4], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[5], duty );
    HT68F30_WriteI2C_Byte(mLightBar_Array[6], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[7], duty );
    HT68F30_WriteI2C_Byte(mLightBar_Array[8], duty );
	HT68F30_WriteI2C_Byte(mLightBar_Array[9], duty );
}

void HT68F30_LightBar_Set(bool Enable)
{
	uint8 value = 0x00;

	if ( Enable )
	{
		value = LIGHT_BAR_ALL_ON;

#ifdef LIGHT_BAR_INVERT	
		value = (~value);
#endif 
	
		HT68F30_WriteI2C_Byte(LIGHT_BAR_ALL, value);
	}
	else
	{
		value = LIGHT_BAR_ALL_OFF;

#ifdef LIGHT_BAR_INVERT	
		value = (~value);
#endif 
	
		HT68F30_WriteI2C_Byte(LIGHT_BAR_ALL, value);
	}

}


void HT68F30_LightBar_USB_StateError( bool bBlinking )
{
	uint8 duty = 0;

	if ( bBlinking == TRUE )
	{
		duty = mLightBar_Duty[2];
		HT68F30_WriteI2C_Byte(mLightBar_Array[1], duty );
	}
	else
	{
		duty = mLightBar_Duty[0];
		HT68F30_WriteI2C_Byte(mLightBar_Array[1], duty );
	}

}

void HT68F30_LightBar_USB_DeviceAttached( bool bValue )
{
	uint8 duty = 0;

	if ( bValue == TRUE )
	{
		duty = mLightBar_Duty[2];
		HT68F30_WriteI2C_Byte(mLightBar_Array[0], duty );
	}
	else
	{
		duty = mLightBar_Duty[0];
		HT68F30_WriteI2C_Byte(mLightBar_Array[0], duty );
	}
}

void HT68F30_LightBar_PowerIndicator( bool bValue )
{
	uint8 i = 0;

	if ( bValue == TRUE )
	{
		HT68F30_WriteI2C_Byte(mLightBar_Array[0], LIGHT_BAR_DUTY_LOW );
	}
	else
	{
		HT68F30_WriteI2C_Byte(mLightBar_Array[0], LIGHT_BAR_DUTY_LOW );
	}

	/*All LED indicators are dark except power indicator*/
	for( i = 1; i < MAX_LEVEL_INDICATOR_NUM; i++ )
	{
		HT68F30_WriteI2C_Byte(mLightBar_Array[i], LIGHT_BAR_DUTY_DARK );
	
	}
}


