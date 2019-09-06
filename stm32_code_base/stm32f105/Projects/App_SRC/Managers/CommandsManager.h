#ifndef __APP_MANAGERS_COMMANDS_MANAGER_H__
#define __APP_MANAGERS_COMMANDS_MANAGER_H__

#include "Command.h"

typedef enum
{
    CTRL_OPCODE_NULL = 0,
    CTRL_OPCODE_I2C_WRITE,
    CTRL_OPCODE_I2C_READ,
    CTRL_OPCODE_DSP_SPI_RELEASE,
    CTRL_OPCODE_GPIO_OUTPUT_CTRL,
    CTRL_OPCODE_EEPROM_WRITE,
    CTRL_OPCODE_CS8422_RESET,
    CTRL_OPCODE_UNSUPPORT
}CTRL_OPCODE;

typedef struct _CTRL_CMD_CTRL
{
  void (*control_command)(uint8 command, void *params);
}CTRL_CMD_HANDLE_OBJECT;


/**
 * Define the number of bytes to send over the UART for writing the EEDID.
 */
#define EEDID_UART_TRANSFER_BLOCK_SIZE 64

/*@}*/

#endif /* __APP_MANAGERS_COMMANDS_MANAGER_H__ */

