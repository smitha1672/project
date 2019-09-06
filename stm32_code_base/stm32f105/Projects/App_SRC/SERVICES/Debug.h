#ifndef __APPLICATION_DEBUG_H__
#define __APPLICATION_DEBUG_H__

#ifdef DEBUG

#include "Defs.h"
#include "DebugStrings.h"
#include <stdarg.h>

/**
 * This enumerator defines the varius debug printout modes.
 * The enum is a bit map and each mode is representated by one bit of the
 * 32 bit enum.
 */
typedef enum
{
    DEBUG_INFO          = 0x00000001,
    DEBUG_ERROR         = 0x00000002,
    DEBUG_DEBUG         = 0x00000004,
    DEBUG_FAC           = 0x00000008,
    DEBUG_CEC           = 0x00000020,
    DEBUG_EEDID         = 0x00000040,
    DEBUG_I2C           = 0x00000080,
    DEBUG_RF            = 0x00000400,
    DEBUG_SPI           = 0x00000800
} DEBUG_FIELDS;

/* default debug trace fields mask */
#define DEBUG_TRACE_DEFAULT \
                DEBUG_INFO|\
                DEBUG_ERROR|\
                DEBUG_FAC|\
                DEBUG_CEC|\
                DEBUG_EEDID|\
                DEBUG_DEBUG|\
                DEBUG_RF

/* Defines the signature value of the printout fields in the storage device.
   It is used to verify whether the printout mask is valid.
*/
#define DEBUG_PRINTOUT_MASK_SIGNATURE  0xBB

/** This MACRO defines an implementation of a generic module specific
    printout function
*/
#define DEBUG_PRINT_DEF(func)          \
void Debug_print ##func(uint16 strId, ...);

/** This MACRO defines the module specific debug function name
*     @param func The soffix of the debug fucntion name. Should be the same
*                 as for the DEBUG_PRINT_DEF MACRO.
*/
#define DEBUG_FUNC_NAME(func, x)    Debug_print ##func x

#define DEBUG_MSG_SETTING ( DEBUG_ERROR|DEBUG_DEBUG|DEBUG_INFO )

/**
 * Initializes the debug services.
 *
 */
void Debug_initialize(void);

/**
 * Disables a debug trace of specifics modules.
 *
 * @param mask    A bitwise map of all enabled modules.
 */
uint32 Debug_register(uint32 mask);

/**
 * Enables a debug trace of specifics modules.
 *
 * @param mask    A bitwise map of all disabled modules.
 */
uint32 Debug_unregister(uint32 mask);

/**
 * Print EEDID data
 *
 * @param buff A pointer tp the data
 *
 * @param size Length of data
 */
void Debug_printEEDIDData(uint8 *buff, uint16 size);

/**
 *    Debug functions definition.
 */
DEBUG_PRINT_DEF(Info);
DEBUG_PRINT_DEF(Error);
DEBUG_PRINT_DEF(I2c);
DEBUG_PRINT_DEF(Fac);
DEBUG_PRINT_DEF(Cec);
DEBUG_PRINT_DEF(Eedid);
DEBUG_PRINT_DEF(Debug);
DEBUG_PRINT_DEF(Hdcp);
DEBUG_PRINT_DEF(Connectivity);
DEBUG_PRINT_DEF(Rf);
DEBUG_PRINT_DEF(SPI);

/**
 *   Debug MACROS definitions
 */
#define TRACE_INFO(x)   DEBUG_FUNC_NAME(Info, x)
#define TRACE_ERROR(x)  DEBUG_FUNC_NAME(Error, x)
#define TRACE_DEBUG(x)  DEBUG_FUNC_NAME(Debug, x)

#define TRACE_RF(x)     DEBUG_FUNC_NAME(Rf, x)

#ifdef DEBUG_TRACE_I2C
#define TRACE_I2C(x)    DEBUG_FUNC_NAME(I2c,x)
#else
#define TRACE_I2C(x)
#endif /*DEBUG_TRACE_I2C*/

#ifdef DEBUG_TRACE_FAC
#define TRACE_FAC(x)     DEBUG_FUNC_NAME(Fac,x)
#else
#define TRACE_FAC(x)
#endif

#ifdef DEBUG_TRACE_CEC
#define TRACE_CEC(x)    DEBUG_FUNC_NAME(Cec, x)
#else
#define TRACE_CEC(x)
#endif

#ifdef DEBUG_TRACE_EEDID
#define TRACE_EEDID(x)  DEBUG_FUNC_NAME(Eedid, x)
#else
#define TRACE_EEDID(x)
#endif

#ifdef DEBUG_TRACE_SPI
#define TRACE_SPI(x)    DEBUG_FUNC_NAME(SPI,x)
#else
#define TRACE_SPI(x)
#endif

#define ASSERT_FUC(f) \
    if ((f) == TRUE)  \
    { \
            TRACE_ERROR((0, "Assert failed %s[%d]", __FUNCTION__, __LINE__));\
    }


#define ASSERT_BOOL(f) \
    if ((f) == FALSE)  \
    { \
            TRACE_ERROR((0, "Assert failed %s[%d]", __FILE__, __LINE__));\
        return FALSE; \
    }

#define ASSERT(f) \
    if ((f) == FALSE)  \
    { \
        TRACE_ERROR((0, "Assert failed %s[%d]", __FILE__, __LINE__));\
        while(1) {/* infinite loop */}; \
    }

#define ASSERT_PRINT(f) \
    if ((f) == FALSE)  \
    { \
        TRACE_ERROR((0, "Assert failed %s[%d]", __FILE__, __LINE__));\
    }

#define ASSERT_TRUE(f) ASSERT(!(f))

#else /* DEBUG */

#define TRACE_INFO(x)
#define TRACE_ERROR(x)
#define TRACE_I2C(x)
#define TRACE_DEBUG(x)
#define TRACE_KEEPALIVE()
#define TRACE_CONNECTIVITY(x)
#define ASSERT_BOOL(f)          if ((f) == TRUE) return FALSE;
#define ASSERT(f)
#define ASSERT_TRUE(f)
#define ASSERT_FUC(f)


#endif

/*--------------------------------TIMERS--------------------------------------*/

/**
 * Start block time measuring. Start time will be saved and will be used to
 * measure the execution time.
 */
#define DEBUG_TIME_START() \
    { \
        uint32 t; \
        t = VirtualTimer_now();

/**
 * End time measuring. Execution time of block in ms will be printed. The
 * macro DEBUG_TIME_START must be called before calling this macro.
 *
 * @param str  Prefix string for printing.
 */
#define DEBUG_TIME_END(str) \
        t = VirtualTimer_now() - t; \
        TRACE_DEBUG(( STR_ID_NULL, "%s : %u", (str), (50 * t) / 1000 )); \
    }

/**
 * End time measuring. Execution time of block in ms will be printed if time
 * has execeeded the guard time. The macro DEBUG_TIME_START must be called
 * before calling this macro.
 *
 * @param str  Prefix string for printing.
 * @param msec Guard time. If execution time of block has excceeded this time,
 *             str will be printed with execution time. Otherwise, no prinitng
 *             will be done.
 */
#define DEBUG_TIME_GUARD(str, msec) \
        t = VirtualTimer_now() - t; \
        if (((t * 50) / 1000) >= msec) \
           { TRACE_DEBUG(( \
                STR_ID_NULL, \
                "%s : %u", \
                (str), (50 * t) / 1000));} \
    }

#endif /* __APPLICATION_DEBUG_H__ */

