//***************************************************************************
//! @file     si_usb_serial.c
//! @brief    USB CDC Serial Driver.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_platform.h"
#include "si_arm.h"


//*****************************************************************************
//
// Globals from si_usb_serial_structs.c
//
//*****************************************************************************

extern volatile tUSBMode g_eUSBMode;
extern const    tUSBBuffer g_sTxBuffer;
extern const    tUSBBuffer g_sRxBuffer;
extern const    tUSBDCDCDevice g_sCDCDevice;

//*****************************************************************************
//
// Flag indicating whether or not a Break condition is currently being sent.
//
//*****************************************************************************
static void UsbGetData(void);
static unsigned char isUsbConnected = false;
static unsigned char isUsbComOpen = false;


unsigned long ulRate = 115200;


//*****************************************************************************
//
// External interface. The following external functions must be implemented
// in the module consuming incoming UART data.
//
//*****************************************************************************

//------------------------------------------------------------------------------
// Function:    SkSerialRxHandler
// Description: Handle incoming chars from UART
//------------------------------------------------------------------------------
extern bool_t SkSerialRxHandler(uint8_t achar);
//------------------------------------------------------------------------------
// Function:    SkSerialRxHandler
// Description: Check if receiving is locked until after finishing of command processing
//------------------------------------------------------------------------------
extern bool_t SkSerialConsumedChar();


//*****************************************************************************
//
// Set the communication parameters to use on the UART.
//
//*****************************************************************************
static tBoolean SetLineCoding(tLineCoding *psLineCoding)
{

	psLineCoding=psLineCoding;
    return(true);

    //
    // Let the caller know if a problem was encountered.
    //
}

//*****************************************************************************
//
// Get the communication parameters in use on the UART.
//
//*****************************************************************************
static void GetLineCoding(tLineCoding *psLineCoding)
{
    psLineCoding->ulRate = ulRate;
    psLineCoding->ucDatabits = 8;
    psLineCoding->ucParity = USB_CDC_PARITY_NONE;
    psLineCoding->ucStop = USB_CDC_STOP_BITS_1;

    //
    // Send the information back to the host.
    //
    USBDCDSendDataEP0(0, (unsigned char *)psLineCoding, sizeof(tLineCoding));
}

//*****************************************************************************
//
// This function sets or clears a break condition on the redirected UART RX
// line.  A break is started when the function is called with \e bSend set to
// \b true and persists until the function is called again with \e bSend set
// to \b false.
//
//*****************************************************************************
static void SendBreak(tBoolean bSend)
{
	bSend=bSend;
}

//*****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the notification event.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned long SiiPlatformUsbControlHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
    //
    // Which event was sent?
    //
    switch(ulEvent)
    {
        //
        // The host has connected.
        //
        case USB_EVENT_CONNECTED:
        {
            //
            // Flush the buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);
            isUsbConnected = true;

            break;
        }

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
        {
        	isUsbConnected = false;

            break;
        }

        //
        // Return the current serial communication parameters.
        //
        case USBD_CDC_EVENT_GET_LINE_CODING:
        {
            GetLineCoding(pvMsgData);
            isUsbConnected = true;
            break;
        }

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_LINE_CODING:
        {
            // Gets called when serial port gets open
            isUsbComOpen = true;
            SetLineCoding(pvMsgData);
            break;
        }

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
        {
            // Gets called when serial port gets open or close
            isUsbComOpen = false;
            break;
        }

        //
        // Send a break condition on the serial line.
        //
        case USBD_CDC_EVENT_SEND_BREAK:
        {
            SendBreak(true);
            break;
        }

        //
        // Clear the break condition on the serial line.
        //
        case USBD_CDC_EVENT_CLEAR_BREAK:
        {
            SendBreak(false);
            break;
        }

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
       		isUsbConnected = false;
        	break;
        case USB_EVENT_RESUME:
        	isUsbConnected = true;
            break;

        //
        // Other events can be safely ignored.
        //
        default:
        {
            break;
        }
    }

    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ulCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the notification event.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned long SiiPlatformUsbTxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
    //
    // Which event was sent?
    //
    switch(ulEvent)
    {
        case USB_EVENT_TX_COMPLETE:
        {
            //
            // There is nothing to do here since it is handled by the
            // USBBuffer.
            //
            break;
        }

        //
        // Other events can be safely ignored.
        //
        default:
        {
            break;
        }
    }
    return(0);
}



//*****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ulCBData is the client-supplied callback data value for this channel.
// \param ulEvent identifies the notification event.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned long SiiPlatformUsbRxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData)
{
    unsigned long ulCount = 0;

    //
    // Which event was sent?
    //
    switch(ulEvent)
    {
        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            //
            // Feed some characters into the debug buffer
            //
            UsbGetData();
        	break;
        }

        //
        // This is a request for how much unprocessed data is still waiting to
        // be processed.  Return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something.  The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        //
        case USB_EVENT_DATA_REMAINING:
        {
            //
            // Get the number of bytes in the buffer and add 1 if some data
            // still has to clear the transmitter.
            //
            return(ulCount);
        }

        //
        // This is a request for a buffer into which the next packet can be
        // read.  This mode of receiving data is not supported so let the
        // driver know by returning 0.  The CDC driver should not be sending
        // this message but this is included just for illustration and
        // completeness.
        //
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }

        //
        // Other events can be safely ignored.
        //
        default:
        {
            break;
        }
    }

    return(0);
}


//*****************************************************************************
//
// Take as many bytes from the transmit buffer as there is space for and move
// them into the debug buffer
//
//*****************************************************************************
static void UsbGetData(void)
{
    unsigned long ulRead;
    unsigned char ucChar;

	//if usb is not connected just return;
	if (!isUsbConnected)
		return;

	while (USBBufferDataAvailable(&g_sRxBuffer))
	{
		if (SkSerialConsumedChar())
			break;
			//return;
		ulRead = USBBufferRead(&g_sRxBuffer, &ucChar, 1);
 		SkSerialRxHandler( (unsigned char)(ucChar & 0xFF));
	}

}


//*****************************************************************************
//
// Return buffer availability.
// 
//
//*****************************************************************************
bool_t SiiPlatformUsbGetBufferAvail(unsigned long *data)
{

	//if usb is not connected just return;
	if (!isUsbConnected)
		return false;

	 *data = USBBufferSpaceAvailable(&g_sTxBuffer);
	 return true;
}

//*****************************************************************************
//
// Return buffer availability.
// 
//
//*****************************************************************************
void SiiPlatformUsbWriteChar(unsigned char ucChar)
{
	 //uint8_t data = USBBufferSpaceAvailable(&g_sTxBuffer);

	//if usb is not connected just return;
	if (!isUsbConnected)
		return;
	USBBufferWrite(&g_sTxBuffer, &ucChar, 1);
}


//*****************************************************************************
//
// This is the main application entry function.
//
//*****************************************************************************

void SiiPlatformUsbUARTputc ( unsigned char outChar )
{
	//if usb is not connected just return;
	if (!isUsbConnected)
		return;
	
    if (USBBufferSpaceAvailable(&g_sTxBuffer))
    {
    	unsigned char ucChar = (unsigned char)(outChar & 0xFF);
    	USBBufferWrite(&g_sTxBuffer, &ucChar, 1);
    }
}

//*****************************************************************************
//
// Return USB Com connection status
//
//*****************************************************************************
bool_t SiiPlatformUsbIsComOpen(void)
{
    return isUsbComOpen;
}

//*****************************************************************************
//
// This is the main application entry function.
//
//*****************************************************************************
void SiiPlatformUsbInit(void)
{
    void *pUSBCDCInstance;

    //
    // Initialize the transmit and receive buffers.
    //
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    //
    // Enable Forced Device mode so that pins USB0VBUS & USB0ID can be used as GPIO or UART pins.
    //
    g_eUSBMode = USB_MODE_FORCE_DEVICE;

    //
    // Pass the device information to the USB library and place the device
    // on the bus.
    //
    pUSBCDCInstance = USBDCDCInit(0, &g_sCDCDevice);

}
