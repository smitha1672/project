#include "Debug.h"
#include "CardLowLevel_drve_conf.h"

#ifdef EEPROM_ITF	
bool eeprom_writeSlave(uint16 RegAddr, byte *data, uint16 length)
{
	int ret;

	ret = I2C_writeSlave( STORAGE_CONTROL, RegAddr, data, length, FALSE);

	return (ret==0? TRUE: FALSE);
}
#else
bool eeprom_writeSlave(uint16 RegAddr, byte *data, uint16 length){ return FALSE; }
#endif /*EEPROM_ITF*/

#ifdef EEPROM_ITF
bool eeprom_readSlave( uint16 RegAddr, byte *data, uint16 length)
{
	int ret;
	
	ret = I2C_readSlave( STORAGE_CONTROL, RegAddr, data, length, FALSE);

	return (ret==0? TRUE: FALSE);
}
#else
bool eeprom_readSlave( uint16 RegAddr, byte *data, uint16 length){ return FALSE; }
#endif /*EEPROM_ITF*/

bool eeprom_wait4DeviceReady(byte control) {return TRUE; }
