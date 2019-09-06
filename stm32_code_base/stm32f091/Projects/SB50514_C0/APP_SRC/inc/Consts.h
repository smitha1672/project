#ifndef __APPLICATION_CONSTS_H__
#define __APPLICATION_CONSTS_H__


/* General Consts */
#ifdef UINT_MAX
#undef UINT_MAX
#endif
#define UINT_MAX 0xFFFFFFFF

#define BITS_IN_BYTE        8




/*
	E-TRACE system eeprom definition	
*/
#define STORAGE_CONTROL                 (0xA0)
#define EEPROM_CONTROL                  (0xA0)

#endif /* __APPLICATION_CONSTS_H__ */
