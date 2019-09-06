#ifdef DEBUG
#include "Debug.h"


/** Table used for hexadecimal and decimal to char conversion */
static const char CHAR_MAP[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                               };

/** Decimal base for decimal to char conversion */
#define DECIMAL_BASE     10

/** Hexadecimal base */
#define HEXADECIMAL_BASE 0x10

/** Maximal buffer to use for conversion of a number */
#define MAX_NUMBER_BUFFER 10

static uint32 m_traceEnableMask;

//! < External Application ___________________________________________________________________________________@{
//These applicatons is only used on here.
extern void usart_putchar(const char c);
//! @}

/**
 * Small implementation of the printf function.
 *
 * @param   moduleId is one of the enums in DEBUG_FIELDS.
 * @param   moduleStr is the module string to be printout as a prefix.
 * @param   strId is the Static String ID that is sent to the AppCom.
 *
 * @note Implements %s %d %X %u %ld %lX and %lu only.
 *
 * @see printf.
 */
static void Debug_printf(DEBUG_FIELDS moduleId,
                         const char *moduleStr,
                         uint16 strId,
                         va_list args);

/**
 * Writes a single character to the output.
 *
 * @param ch        The character to write.
 */
static void Debug_writeChar(char ch);

/**
 * Writes a single string to the output.
 *
 * @param str       The string to write.
 */
static void Debug_writeString(const char *str);

/**
 * Writes a number to the output according to a given base.
 *
 * @param number    The number to write.
 * @param base      The base to write the number in.
 */
static void Debug_writeNumber(int32 number, int16 base);

/**
 * Writes an unsigned number to the output according to a given base.
 *
 * @param number    The number to write.
 * @param base      The base to write the number in.
 */
static void Debug_writeUnsignedNumber(uint32 number, uint16 base);

/*This MACRO defines an implementation of a generic module specific
  printout function*/
#define DEBUG_PRINT(func, id, prefStr)          \
void Debug_print ##func(uint16 strId, ...)      \
{                                               \
   va_list args;                                \
   va_start(args, strId);                       \
   Debug_printf(id, prefStr, strId, args);      \
   va_end(args);                                \
}

/**************************** Implementation **********************************/
void Debug_initialize(void)
{
    uint32 mask = 0;

    mask |= DEBUG_ERROR | DEBUG_DEBUG | DEBUG_INFO;

    /*All printouts are disabled.*/
    m_traceEnableMask = mask;
}

/*----------------------------------------------------------------------------*/
uint32 Debug_register(uint32 mask)
{
    uint32 old = m_traceEnableMask;
    m_traceEnableMask |= mask;
    return old;
}

/*----------------------------------------------------------------------------*/
uint32 Debug_unregister(uint32 mask)
{
    uint32 old = m_traceEnableMask;
    m_traceEnableMask &= ~mask;
    return old;
}

/*----------------------------------------------------------------------------*/
/* The below code implements the varius module specific printout functions*/
DEBUG_PRINT(Info, DEBUG_INFO,   "INFO");
DEBUG_PRINT(Error, DEBUG_ERROR, "ERR");
DEBUG_PRINT(I2c, DEBUG_I2C,     "I2C");
DEBUG_PRINT(Fac, DEBUG_FAC,     "FAC");
DEBUG_PRINT(Cec, DEBUG_CEC,     "CEC");
DEBUG_PRINT(Eedid, DEBUG_EEDID, "EEDID");
DEBUG_PRINT(Debug, DEBUG_DEBUG, "DBG");
DEBUG_PRINT(Rf, DEBUG_RF, "RF");
DEBUG_PRINT(SPI, DEBUG_SPI, "SPI");

/*----------------------------------------------------------------------------*/
static void Debug_printf(
    DEBUG_FIELDS moduleId,
    const char *moduleStr,
    uint16 strId,
    va_list args
)
{
    char *str;

    /*Check whether the moduleId printouts are enabled*/
    if (!(m_traceEnableMask & moduleId))
    {
        /* The printouts are disabled. Abort this printout function.*/
        return;
    }

    /*In case the String ID is NULL, the printout will be in legacy mode,
      without sending the String ID to the PC*/
    if (STR_ID_NULL != strId)
    {
        /* Send the identification code to the AppCom*/
        /* The AppCom will recognizes this printout as STR ID*/
        Debug_writeChar(STR_ID_CODE_1);
        Debug_writeChar(STR_ID_CODE_2);
        /* Send the String ID to AppCom as a binary coded number*/

        /* Send the high bype of a 16 bits integer*/
        Debug_writeChar(strId >> 8);

        /* Send the high bype of a 16 bits integer*/
        Debug_writeChar(strId & 0xff);
    }

    str = va_arg(args, char *);

    /*Printout the printed module string*/
    Debug_writeString(moduleStr);
    Debug_writeString("\t>>  ");

    while ((*str != '\0') && (str != NULL))
    {
        if (*str == '%')
        {
            str++;

            switch (*str)
            {
                case 'd':
                    Debug_writeNumber(va_arg(args, int), DECIMAL_BASE);
                    break;
                case 'u':
                    Debug_writeUnsignedNumber(
                        va_arg(args, int), DECIMAL_BASE);
                    break;
                case 'X':
                    Debug_writeUnsignedNumber(
                        va_arg(args, int), HEXADECIMAL_BASE);
                    break;
                case 's':
                    Debug_writeString(va_arg(args, const char *));
                    break;
                default:
                    Debug_writeChar(*str);
                    break;
            }
        }
        else
        {
            Debug_writeChar(*str);
        }
        str++;
    }

    Debug_writeChar(' ');
    Debug_writeChar('\r');
    Debug_writeChar('\n');

    va_end(args);
}

void putch(char c)
{
    usart_putchar(c);
}

/*----------------------------------------------------------------------------*/
static void Debug_writeChar(char ch)
{
    putch(ch);
}

/*----------------------------------------------------------------------------*/
static void Debug_writeString(const char *str)
{
    char ch;
    while ((ch = *str++) != '\0')
    {
        Debug_writeChar(ch);
    }
}

/*----------------------------------------------------------------------------*/
static void Debug_writeNumber(int32 number, int16 base)
{
    if (number < 0)
    {
        Debug_writeChar('-');
        /* Possible bug. MIN_INT */
        number *= -1;
    }

    Debug_writeUnsignedNumber(number, base);
}

/*----------------------------------------------------------------------------*/
static void Debug_writeUnsignedNumber(uint32 number, uint16 base)
{
    static char result[MAX_NUMBER_BUFFER];
    uint16 pos = MAX_NUMBER_BUFFER - 1;

    /* For the LSD to the MSD, convert each digit to a char */
    while (number >= base)
    {
        result[pos] = CHAR_MAP[number % base];
        number = number / base;
        pos--;
    }

    result[pos] = CHAR_MAP[number];

    /* Print the characters in the right order */
    while (pos < MAX_NUMBER_BUFFER)
    {
        putch(result[pos]);
        pos++;
    }
}

void Debug_printEEDIDData(uint8 *buff, uint16 size)
{
    int i;
    uint8 data;

    /* Send the identification code to the AppCom*/
    /* The AppCom will recognizes this printout as EEDID data*/
    Debug_writeChar(STR_EEDID_ID_CODE_1);
    Debug_writeChar(STR_EEDID_ID_CODE_2);

    /* Send the high bype of a 16 bits integer*/
    Debug_writeChar(size >> 8);

    /* Send the low bype of a 16 bits integer*/
    Debug_writeChar(size & 0xff);

    /** Print out the buffer */
    for (i = 0; i < size; i++)
    {
        data = *(buff + i);
        Debug_writeUnsignedNumber(data, HEXADECIMAL_BASE);
        putch((char)STR_EEDID_DELIMITER);
    }

    Debug_writeChar('\n');
}




#endif
