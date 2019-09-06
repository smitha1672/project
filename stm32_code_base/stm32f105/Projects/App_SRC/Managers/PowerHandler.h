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
    //void (*go_back_power_up) ( void );
    void (*power_ctrl)(xPowerHandle_Queue value );
    POWER_STATE(* get_power_state)(void);
    xPowerHandleState(* get_power_handle_state)(void);
    void (*turn_off_done)( uint8 handle );
    void (*turn_on_done)( uint8 handle );
    uint8 (*getSystemResetType)( void );
    void (*clrSystemResetType)( void );
    void (*power_5v_ctrl)(bool);
    uint8 (*get_backup_power_state)( void );
    uint8 ( *get_system_reset_type ) ( void );
#if ( configSII_DEV_953x_PORTING == 1 )        
    bool (*power_is_hpd_from_tv)(void);
    void (*power_hpd_to_953X)(bool Enable);
    void (*power_hpd_to_dvd)(bool Enable);
    bool (*power_check_5v_from_dvd)(void);
#endif
}POWER_HANDLE_OBJECT;


#endif  /*__POWER_MANAGER_H__*/

