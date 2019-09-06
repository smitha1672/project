#ifndef __USB_MEDIA_MANAGER_H__
#define __USB_MEDIA_MANAGER_H__

#include "ff.h"       /* FATFS */

#if 0
typedef enum{
    USB_MEDIA_IDLE,
    USB_MEDIA_OPEN_FILE,
    USB_MEDIA_INIT_I2S,
    USB_MEDIA_INIT_PLAY_FILE,
    USB_MEDIA_PLAY_FILE,
    USB_MEDIA_PAUSE,
    USB_MEDIA_STOP,
    USB_MEDIA_FINISH
}xUSBMediaManagerState;

typedef enum
{
    QueueFull = 0,
    QueueEmpty,
    QueueInsertError,
    QueueInsertOK,
    QueueFetchOK
}QueueState;
#endif

typedef enum
{
    QueueFull = 0,
    QueueEmpty,
    QueueInsertError,
    QueueInsertOK,
    QueueFetchOK
}QueueState;


typedef struct _USB_MEDIA_MANAGE
{
    void (*init)(void);
    void (*media_play)(void);  
    void (*media_pause)(void);
    void (*media_stop)(void);
    void (*media_rePlay)(void);
    void (*assign_usb_filelist)(void* filelist);
    void (*mediaManagerTask)(void);
    QueueState (* FetchQueue)(void);
    void (*media_reqWaveNum)(uint8_t num);
    bool (*media_isPlayFinished)(void);
    void (*assign_cck_waveFormat)(void* WaveFormat);
    void (*cck_play)(void);
}USB_MEDIA_MANAGE_OBJECT;


#endif

