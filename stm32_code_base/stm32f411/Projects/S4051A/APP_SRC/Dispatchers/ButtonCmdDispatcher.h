#ifndef __APP_DISPATCHERS_BUTTON_COMMAND_DISPACHER_H__
#define __APP_DISPATCHERS_BUTTON_COMMAND_DISPACHER_H__

#include "Defs.h"

typedef struct _BTN_HANDLE
{
    void ( *register_user_cmd )(void);
    void ( *register_demo_1n3_cmd )(void);
    void ( *register_demo_2_cmd )(void);
}BTN_CONF_OBJECT;

#endif /* __APP_DISPATCHERS_BUTTON_COMMAND_DISPACHER_H__ */

