#ifndef __USB_MUSIC_MANAGER_H__
#define __USB_MUSIC_MANAGER_H__

#include "freertos_conf.h"
#include "freertos_task.h"
#include "freertos_typedef.h"

typedef enum{
    USB_MMS_IDLE,
    USB_MMS_READY_PLAY, 
    USB_MMS_PLAY,
    USB_MMS_NEXT,
    USB_MMS_PREVIOUS,
    USB_MMS_PLAY_PAUSE,
    USB_MMS_STOP,
    USB_MMS_STOP_DEMO2
}xUSBMusicManagerState;

typedef struct _USB_MUSIC_MANAGE
{
    void (*init)(void);  
    void (*Deinit)(void);
    void (*CreateTask)(void);  
    void (*usb_task_ctrl)( xOS_TaskCtrl val );
    void ( *SendEvent ) ( const void *params );
    void (*assign_usb_filelist)(void* filelist);
    void (*StartDemoMode2)(void);
    void (*StopDemoMode2)(void);
    void (*ModeSwtich)(const void *params);
}USB_MUSIC_MANAGE_OBJECT;




typedef  void(* xSet_DemoMode2_Timeout)(void);
void USBMusicManager_RegisterSetDemoMode2Timeout( xSet_DemoMode2_Timeout callback );

#endif

