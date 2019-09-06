#include "HT68F30_i2c.h"


//#include "CardLowLevel_drve_conf.h"
//#include "Debug.h"

#define HT68F30_I2C_ADDR 0x40


bool HT68F30_WriteI2C_Byte(byte RegAddr,byte uc)
{
    bool  ret;
	
    ret=I2C_writeSlave(HT68F30_I2C_ADDR ,
        RegAddr ,
        (byte*)&uc,
        1,
         FALSE
          );

	if ( ret == FALSE )
	{
		//TRACE_ERROR((0, "HT68F30 I2C writes error !! "));
		return FALSE;
	}

	return TRUE;
}
