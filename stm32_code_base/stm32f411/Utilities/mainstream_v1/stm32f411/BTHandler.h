#ifndef __BT_HANDLER_H__
#define __BT_HANDLER_H__


typedef enum{
    BT_NOTIFY_POW_SEQ_DONE = 0,
    BT_HANDLE_INIT,
    BT_HANDLE_DINIT,
    BT_HANDLE_ON,
    BT_HANDLE_OFF,
    BT_HANDLE_NEXT,
    BT_HANDLE_PREVIOUS,
    BT_HANDLE_PLAY_PAUSE,
    BT_HANDLE_PAIRING,
    BT_HANDLE_RESET_BT,
    BT_HANDLE_CLEAN_DEVICE
}xBTHandleCommand;

typedef enum{
    BT_MS_IDLE = 0,
    BT_MS_INIT,
    BT_MS_DEINIT,
    BT_MS_POWER_ON,
    BT_MS_POWER_OFF,
    BT_MS_NEXT,
    BT_MS_PREVIOUS,
    BT_MS_PLAY_PAUSE,
    BT_MS_PAIRING,
    BT_MS_RESET_BT,
    BT_MS_CLEAN_DEVICE,
    BT_MS_AUTO_LINK,
    BT_MS_CHECKING_PAIRING_STATUS,
    BT_MS_END
}xBTHandleState;

typedef struct _BT_HANDLE
{
    void (*CreateTask)(void);  
    void ( *SendEvent ) ( void *params );
    bool (*bt_instr_sender)(void *cmds);
    void (*power_ctl)(void);
    xBTHandleModuleStatus (*get_status) (void);
}BT_HANDLE_OBJECT;

#endif

