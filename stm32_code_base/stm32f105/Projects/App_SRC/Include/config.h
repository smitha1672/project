#ifndef __APPLICATION_CONFIG_H__
#define __APPLICATION_CONFIG_H__

/**
 * Defines the avaiable EEPROM devices.
 */
typedef enum
{
    LOGICAL_EEPROM_DEVICE_BLOCK0 = 0,

	LOGICAL_EEPROM_LAST,
    
} LOGICAL_EEPROM_DEVICE;

#if defined (__ARM_CORTEX_MX__)
#define NVIC_VECTOR_TABLE()	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x6000)
#else
#define NVIC_VECTOR_TABLE()
#endif 




#endif //__APPLICATION_CONFIG_H__
