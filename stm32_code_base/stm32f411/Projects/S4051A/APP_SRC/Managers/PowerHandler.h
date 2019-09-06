#ifndef __POWER_HANDLE_H__
#define __POWER_HANDLE_H__

#include "Defs.h"
#include "api_typedef.h"
#include "device_config.h"


typedef struct _Power_HANDLE
{
    void (*initialize)(void); 
    uint8 (*register_handle)(void);
    void (*power_toggle)(void);
    void (*power_ctrl)(uint8 value );
    POWER_STATE(* get_power_state)(void);
    xPowerHandleState(* get_power_handle_state)(void);
    void (*turn_off_done)( uint8 handle );
    void (*turn_on_done)( uint8 handle );
    uint8 (*getSystemResetType)( void );
    void (*clrSystemResetType)( void );
}POWER_HANDLE_OBJECT;


#endif  /*__POWER_MANAGER_H__*/

