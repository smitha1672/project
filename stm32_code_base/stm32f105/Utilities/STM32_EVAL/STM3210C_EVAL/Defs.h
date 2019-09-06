#ifndef __APPLICATION_DEFS_H__
#define __APPLICATION_DEFS_H__

#if defined (__AVR32__)
#include "compiler.h"
#endif 

//! __STM32__	@{
#if defined (__ARM_CORTEX_MX__)
#include <stdlib.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "core_cm3.h"
#endif	//! @}

//#include "Consts.h"
#include "integer.h"

#define VERSION "1.3.0"


//! __STM32__	@{
#if defined ( __ARM_CORTEX_MX__)
typedef uint8_t  		byte;
typedef int8_t          int8;
typedef uint8_t   		uint8;
typedef int16_t         int16;
typedef uint16_t  		uint16;
typedef int32_t        	int32;
typedef uint32_t 		uint32;
//typedef uint8_t 		bool;
#endif //! @}

#if defined ( __ARM_CORTEX_MX__)
//#define TRUE	1
//#define FALSE	0
//#define NULL 0
#endif //! @}


#if defined (__AVR32__)
#define AVR32_GPIO_IN	1
#define AVR32_GPIO_OUT	0

#define INT_PRIORITY_3 AVR32_INTC_INT0 /*Lowest priority*/                           
#define INT_PRIORITY_2 AVR32_INTC_INT1                            
#define INT_PRIORITY_1 AVR32_INTC_INT2                           
#define INT_PRIORITY_0 AVR32_INTC_INT3	/*Highest priority*/                           
#endif /*__AVR32__*/

/**
 * Defs file. Includes various type definitions and consts needed in the
 * system.
 *
 * @author Yoav Amit.
 */

/*Asssumes b > 0 , a > 0 */
#define DivideRoundUp(a, b)  (1 + ((a) - 1) / (b))

/*Asssumes b > 0 , a >= 0 */
#define SafeDivideRoundUp(a, b) \
    ((a) == 0 ? 0 : DivideRoundUp(a, b)) 

/**** MAX ints ****/
#define MAX_UINT8   0xFF
#define MAX_UINT16  0xFFFF
#define MAX_UINT32  0xFFFFFFFF

/**** Bit Sizes ****/
#define BITS_IN_BYTE    8
#define BITS_IN_INT8    8
#define BITS_IN_INT16   16
#define BITS_IN_INT32   32
#define BITS_IN_INT64   64

/**** Byte Sizes ****/
#define BYTES_IN_INT8   (BITS_IN_INT8 / BITS_IN_BYTE)    
#define BYTES_IN_INT16  (BITS_IN_INT16 / BITS_IN_BYTE) 
#define BYTES_IN_INT32  (BITS_IN_INT32 / BITS_IN_BYTE)
#define BYTES_IN_INT64   (BITS_IN_INT64 / BITS_IN_BYTE)

/* Keep Alive timing - Number of polling request */
#define KEEP_ALIVE_TIMING   0xFFF

/* Struct for easy bit access */
typedef union Bits {
    struct {
        unsigned bit0:1;
        unsigned bit1:1;
        unsigned bit2:1;
        unsigned bit3:1;
        unsigned bit4:1;
        unsigned bit5:1;
        unsigned bit6:1;
        unsigned bit7:1;
    } bits;
    unsigned char byte;
} Bits;


/**
 * Types of supported audio sources.
 */
 //! @{
 #if 0
typedef enum {
    AUDIO_SOURCE_AUX1,
    AUDIO_SOURCE_AUX2,
    AUDIO_SOURCE_OPTICAL,
    AUDIO_SORUCE_COAXIAL,
    AUDIO_SOURCE_USB_PLAYBACK,
    AUDIO_SOURCE_NULL
} AUDIO_SOBURCE;
#else	/*Below, remove OTI feature*/
typedef enum {
    AUDIO_SOURCE_AUX1,
    AUDIO_SOURCE_AUX2,
    AUDIO_SORUCE_COAXIAL,
    AUDIO_SOURCE_OPTICAL,
    AUDIO_SOURCE_NULL, 
    AUDIO_SOURCE_USB_PLAYBACK
} AUDIO_SOURCE;
#endif 

typedef enum {
    MODE_NORMAL,
    MODE_DEMO_1,
    MODE_DEMO_2,
} SYSTEM_MODE_SELECTOR;



/*typedef enum{
	POWER_OFF	= 0, 
	POWER_UP = 1,
	POWER_ON = 2,
	POWER_DOWN = 3 
}POWER_STATE;*/

//!	@}

//! I2C definition @{
#define I2C_SUCCESS              0
#define I2C_INVALID_ARGUMENT    -1
#define I2C_ARBITRATION_LOST    -2
#define I2C_NO_CHIP_FOUND       -3
#define I2C_RECEIVE_OVERRUN     -4
#define I2C_RECEIVE_NACK        -5
#define I2C_SEND_OVERRUN        -6
#define I2C_SEND_NACK           -7
//! @}


#define GET_HIGH_U16(X)		(X>>16)
#define GET_LOW_U16(X)		(X&0x0000ffff)
#define GET_HIGH_U8(X)		(X>>8)
#define GET_LOW_U8(X)		(X&0x00ff)

#define LOAD_SYS_PARAM_ON	0x01
#define LOAD_SYS_PARAM_OFF	0xFF

#define isBetween(i,min,max)    ((i >= min) && (i <= max))
//#define MIN(a,b)                (a > b ? b : a)
#define MINIMUM(a,b)            (a > b ? b : a)
#define ADD_MODULO(a,b,m)       a = (a + b) % m
#define SUB_MODULO(a,b,m)       a = (a >= b) ? (a - b) : (a + m - b)
#define Mask32(x)               ((uint32)0x1 << x)
#define Mask16(x) 	        ((uint16)0x1 << x)

#define MEMSET(x, c, l)         {int memset_i; for (memset_i = 0 ; memset_i < l ; memset_i++) ((unsigned char*)(x))[memset_i]=c;}

#define MEMCPY(d, s, l)    { \
    int memcpy_i; \
    for(memcpy_i=0;memcpy_i<(l);memcpy_i++) \
        *(((char*)(d))+memcpy_i) = *(((char*)(s))+memcpy_i); \
}

#define ON 	1
#define OFF 0

//! AUDIO FORMAT DATA TYPES @{
#define AUDIO_FORMAT_PCM		0x80
#define AUDIO_FORMAT_IE61937	0x40
#define AUDIO_FORMAT_AC3		0x01
#define AUDIO_FORMAT_UNKNOW		0xFF

#define AUDIO_PLL_UNLOCK		0x00
//@}

#endif /* __APPLICATION_DEFS_H__ */
