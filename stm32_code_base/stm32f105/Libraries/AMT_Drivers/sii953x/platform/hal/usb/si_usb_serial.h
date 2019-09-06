//***************************************************************************
//! @file     si_usb_serial.c
//! @brief    USB Serial Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#ifndef __SI_USB_SERIAL__H__
#define __SI_USB_SERIAL__H__

//-------------------------------------------------------------------------------
//  USB API functions
//-------------------------------------------------------------------------------

void    SiiPlatformUsbInit(void);
bool_t  SiiPlatformUsbIsComOpen(void);
void    SiiPlatformUsbUARTputc ( unsigned char outChar );
bool_t  SiiPlatformUsbGetBufferAvail(unsigned long *data);
void    SiiPlatformUsbWriteChar(unsigned char ucChar);


//-------------------------------------------------------------------------------
//  USB CDC device callback function prototypes
//-------------------------------------------------------------------------------

unsigned long SiiPlatformUsbRxHandler(void *pvCBData, unsigned long ulEvent,
                        unsigned long ulMsgValue, void *pvMsgData);
unsigned long SiiPlatformUsbTxHandler(void *pvCBData, unsigned long ulEvent,
                        unsigned long ulMsgValue, void *pvMsgData);
unsigned long SiiPlatformUsbControlHandler(void *pvCBData, unsigned long ulEvent,
                             unsigned long ulMsgValue, void *pvMsgData);

#endif // __SI_USB_SERIAL__H__
