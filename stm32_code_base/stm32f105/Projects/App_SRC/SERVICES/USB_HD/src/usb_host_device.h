#ifndef __USB_HOST_DEVICE_H__
#define __USB_HOST_DEVICE_H__

#include "Defs.h"

#define USB_HOST_DEVICE_CMD_MSC_PROCESS	0x82
#define USB_HOST_DEVICE_CMD_MSC_UNMOUNT	0x84
#define USB_HOST_DEVICE_CMD_VCP_CONNECT	0x87
#define	USB_HOST_DEVICE_CMD_VCP_DISCONNECT 0x89

typedef enum{
    USB_DEVICE = 0,
    USB_HOST,
}USB_MODE;

typedef struct _USB_HOST_DEVICE_HANDLE
{
    void (*CreateTask)(void);  
    void (*usb_mode_switch)(uint8 usb_mode);
    USB_MODE ( *get_mode )( void );
}USB_HOST_DEVICE_HANDLE_OBJECT;


#endif /*__USB_HOST_DEVICE_H__*/
