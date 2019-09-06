#ifndef __APPLICATION_DEFS_H__
#define __APPLICATION_DEFS_H__

#include <string.h>
#include <stdio.h>

#if defined (__AVR32__)
#include "compiler.h"
#endif 

//! __STM32__	@{
#if defined (__ARM_CORTEX_MX__)
#include "stm32f10x.h"
#include "core_cm3.h"
#endif	//! @}
#include "freertos_conf.h"
#include "freertos_task.h"

#if defined ( S4051A ) || defined ( S4051B ) || defined ( S4551A )
#define VERSION 0x1015
#define AQ_VERSION "130703_01"
#define BT_FW_VERSION "BT_S4251_201301222_1"
#define RF_FW_VERSION "RF_SB_S4251_20130130_1"
#define LED_FW_VERSION ""
#define OTI_FW_VERSION ""
#endif 

#if defined ( S3851E )|| defined ( S3851F )
#define VERSION 0x1015
#define AQ_VERSION "141024_01"
#define BT_FW_VERSION "BT_S4251_201301222_1"
#define RF_FW_VERSION "RF_SB_S4251_20130130_1"
#define LED_FW_VERSION ""
#define OTI_FW_VERSION ""
#endif

#if defined ( S3851C )|| defined ( S3851D )
#define VERSION 0x1005
#define AQ_VERSION "130703_01"
#define BT_FW_VERSION "BT_S4251_201301222_1"
#define RF_FW_VERSION "RF_SB_S4251_20130130_1"
#define LED_FW_VERSION ""
#define OTI_FW_VERSION ""
#endif

//! __STM32__	@{
#if defined ( __ARM_CORTEX_MX__)
typedef uint8_t  		byte;
typedef int8_t          int8;
typedef uint8_t   		uint8;
typedef int16_t         int16;
typedef uint16_t  		uint16;
typedef int32_t        	int32;
typedef uint32_t 		uint32;

#if !defined ( bool )
typedef uint8_t 		bool;
#endif 

#endif //! @}

#if defined ( __ARM_CORTEX_MX__)

#if !defined ( TRUE )
#define TRUE	1
#endif 

#if !defined ( FALSE )
#define FALSE	0
#endif 

//#define NULL 0
#define code 
#endif //! @}


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

#define isBetween(i,min,max)    ((i >= min) && (i <= max))
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


#endif /* __APPLICATION_DEFS_H__ */
