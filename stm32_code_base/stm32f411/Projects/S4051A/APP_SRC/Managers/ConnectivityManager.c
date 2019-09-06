#include "string.h"
#include "CECLowLevel.h"
#include "Managers.h"
#include "ConnectivityManager.h"

#ifdef EEDID_ENABLE
#include "EEDID.h"
#endif

#ifdef IR_ENABLE
#include "IRLowLevel.h"
#endif

#include "Command.h"
#include "AvDriver_RX.h"
#include "Debug.h"
#include "config.h"
#include "VirtualTimer.h"
#include "CardLowLevel_Platform.h"

static void ConnectivityManager_onMonitorDisconnected(void *params)
{

}

static void ConnectivityManager_onMonitorConnected(void *params)
{

}

void ConnectivityManager_handleCommand(uint8 command, void *params)
{
    switch (command)
    {
        case HD_SINK_DISCONNECTED:
        {
            ConnectivityManager_onMonitorDisconnected(params);
        }
        break;

        case HD_SINK_CONNECTED:
        {
            ConnectivityManager_onMonitorConnected(params);
        }
        break;

        default:
            break;
    }
}

