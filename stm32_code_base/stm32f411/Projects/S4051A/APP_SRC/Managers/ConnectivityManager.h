#ifndef __APP_MANAGERS_CONNECTIVITY_MANAGER_H__
#define __APP_MANAGERS_CONNECTIVITY_MANAGER_H__

#include "Command.h"
#include "ConnectivityManager.h"

/**
 * Handles the given command.
 *
 * @param command       The command to handle.
 * @param params        The command's parameters.
 */
void ConnectivityManager_handleCommand(uint8 command, void *params);

#endif /* __APP_MANAGERS_CONNECTIVITY_MANAGER_H__ */

