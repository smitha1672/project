//***************************************************************************
//!file     si_debugger_hdmigear.c
//!brief    Silicon Image Starter Kit HDMI-Gear debugger interface.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#include "si_debugger_hdmigear.h"
#include "si_platform.h"
#include "si_uartstdio.h"
#include "si_eeprom.h"
#include "si_i2c.h"
#include "si_gpio.h"
#include "si_flash_update.h"

extern char *buildVersion;  // From buildtime.c
extern void SiiMonCbIrCmdSet(uint8_t irKey, uint8_t irAddr); // callback for SIIMON IR commands delivery

//------------------------------------------------------------------------------
// File shared variables
//------------------------------------------------------------------------------

static bool_t l_hgComRequest = false;
static bool_t l_hgSiimonMode = false;

static uint8_t FuncRes[ FUNC_RES_SIZE ];
static uint8_t RXBuffer[ RX_BUF_SIZE ];
static uint8_t bRxIndex   = 0;
static uint8_t bCommState = WaitStartRX;
static uint8_t isBuffInUse = false;
static CmdType *CmdPtr;

static uint8_t DebugSource = SOURCE_IS_HDMIGEAR;

static volatile bool_t consumedChar=false;

I2CCommandTypeSiMon I2CCmd;


//------------------------------------------------------------------------------
// Function:    BlockRead_8BA
// Description: Translate I2C Read from legacy call
//------------------------------------------------------------------------------

static uint8_t BlockRead_8BA(I2CCommandType * I2CCommand)
{
    bool_t result;

    result = SiiPlatformI2cReadBlock(0,
            I2CCommand->SlaveAddr,
            I2CCommand->RegAddrL,
            I2CCommand->Data,
            I2CCommand->NBytes );

    return( (result) ? IIC_OK : IIC_NOACK );
}

//------------------------------------------------------------------------------
// Function:    BlockWrite_8BA
// Description: Translate I2C Write from legacy call
//------------------------------------------------------------------------------

static uint8_t BlockWrite_8BA(I2CCommandType * I2CCommand)
{
    bool_t result;

    result = SiiPlatformI2cWriteBlock(0,
            I2CCommand->SlaveAddr,
            I2CCommand->RegAddrL,
            I2CCommand->Data,
            I2CCommand->NBytes );

    return( (result) ? IIC_OK : IIC_NOACK );
}

//------------------------------------------------------------------------------
// Function:    BlockRead_16BA
// Description: Translate I2C Read from legacy call
//------------------------------------------------------------------------------

static uint8_t BlockRead_16BA(I2CCommandType * I2CCommand)
{
    bool_t      result = IIC_OK;
    uint16_t    addr16;

    addr16 = I2CCommand->RegAddrH;
    addr16 = ( addr16 << 8) | I2CCommand->RegAddrL;

    if (I2CCommand->SlaveAddr == EEPROM_I2C_ADDRESS)
    {
    	result = SiiPlatformEepromReadBlock(
				addr16,
				I2CCommand->Data,
				I2CCommand->NBytes );
    }
    else
    {
		result = SiiPlatformI2cReadBlock16(0,
				I2CCommand->SlaveAddr,
				addr16,
				I2CCommand->Data,
				I2CCommand->NBytes );
    }

    return( (result) ? IIC_OK : IIC_NOACK );
}

//------------------------------------------------------------------------------
// Function:    BlockWrite_16BA
// Description: Translate I2C Write from legacy call
//------------------------------------------------------------------------------

static uint8_t BlockWrite_16BA(I2CCommandType * I2CCommand)
{
    bool_t      result = IIC_OK;
    uint16_t    addr16;

    addr16 = I2CCommand->RegAddrH;
    addr16 = ( addr16 << 8) | I2CCommand->RegAddrL;

    if (I2CCommand->SlaveAddr == EEPROM_I2C_ADDRESS)
    {
    	result = SiiPlatformEepromWriteBlock(
				addr16,
				I2CCommand->Data,
				I2CCommand->NBytes );
    }
    else
    {
		result = SiiPlatformI2cWriteBlock16(0,
				I2CCommand->SlaveAddr,
				addr16,
				I2CCommand->Data,
				I2CCommand->NBytes );
    }

    return( (result) ? IIC_OK : IIC_NOACK );
}

//------------------------------------------------------------------------------
// Function:    CommAccepted
// Description: Sends simple confirmation on received command
//------------------------------------------------------------------------------

static void CommAccepted(void)
{

	 SiiPlatformUartPutcUnfiltered(0xff);
	 SiiPlatformUartPutcUnfiltered(0x60);
	 SiiPlatformUartPutcUnfiltered(0x1);
	 SiiPlatformUartPutcUnfiltered( COM_ACK );
}

//------------------------------------------------------------------------------
// Function:    SendHandShake
// Description: Sends confirmation on received command
//------------------------------------------------------------------------------

static void SendHandShake ( uint8_t IICMDDCErrState )
{
    uint8_t bHandShake = COM_ACK;

    if ( IICMDDCErrState )
    {
        if ( IICMDDCErrState == IIC_CAPTURED )
            bHandShake = COM_ACK_IIC_CAPTUTED;
        else if ( IICMDDCErrState == IIC_NOACK )
            bHandShake = COM_ACK_IIC_NOACK;
        else if ( IICMDDCErrState == MDDC_NOACK )
            bHandShake = COM_ACK_MDDC_NOACK;
    }

    SiiPlatformUartPutcUnfiltered( bHandShake );
}

//------------------------------------------------------------------------------
// Function:    SendDataFrom8BAIICDevice
// Description: Sends data which has been reading from IIC device with 8 bit
//              addressing mode.
//------------------------------------------------------------------------------

static void SendDataFrom8BAIICDevice ( void )
{
    uint8_t i, bCheckSum, bState;

    bCheckSum = 0;
    bState = BlockRead_8BA(&CmdPtr->I2CCommand);
    SendHandShake( bState );
    if ( !bState )   // no errors
    {
        for ( i = 0; i < CmdPtr->I2CCommand.NBytes; i++ )
        {
            bCheckSum += CmdPtr->I2CCommand.Data[i];
            SiiPlatformUartPutcUnfiltered(CmdPtr->I2CCommand.Data[i]);
        }
        SiiPlatformUartPutcUnfiltered( bCheckSum );
    }
}

//------------------------------------------------------------------------------
// Function:    WriteDataTo8BAIICDevice
// Description: Writes data to IIC device with 8 bit addressing mode.
//------------------------------------------------------------------------------

static void WriteDataTo8BAIICDevice(void)
{
    uint8_t bState;

    bState = BlockWrite_8BA((I2CCommandType *)&CmdPtr->I2CCommand);
    SendHandShake( bState );
}

//------------------------------------------------------------------------------
// Function:    SendDataFrom16BAIICDevice
// Description: Sends data which has been reading from IIC device with 16 bit
//              addressing mode.
//------------------------------------------------------------------------------

static void SendDataFrom16BAIICDevice ( void )
{
    uint8_t i; uint8_t bCheckSum, bState;

    bCheckSum = 0;
    bState = BlockRead_16BA(&CmdPtr->I2CCommand);
    SendHandShake( bState );
    if ( !bState )   // no errors
    {
        for ( i = 0; i < CmdPtr->I2CCommand.NBytes; i++ )
        {
            bCheckSum += CmdPtr->I2CCommand.Data[i];
            SiiPlatformUartPutcUnfiltered( CmdPtr->I2CCommand.Data[i] );
        }
        SiiPlatformUartPutcUnfiltered( bCheckSum );
    }
}

//------------------------------------------------------------------------------
// Function:    WriteDataTo16BAIICDevice
// Description: Writes data to IIC device with 16 bit addressing mode.
//------------------------------------------------------------------------------

static void WriteDataTo16BAIICDevice(void)
{
    uint8_t bState;

    bState = BlockWrite_16BA((I2CCommandType *)&CmdPtr->I2CCommand);
    SendHandShake( bState );
}



//------------------------------------------------------------------------------
// Function:    SendResultOfFunction
// Description: Sends function result through UART
//------------------------------------------------------------------------------

static void SendResultOfFunction ( void )
{
    uint8_t bCheckSum, i;

    bCheckSum = 0;
    for (i = 0; i < CmdPtr->FuncCommand.NBytes; i++)
    {
        bCheckSum +=  FuncRes[i];
        SiiPlatformUartPutcUnfiltered( FuncRes[i] );
    }

    SiiPlatformUartPutcUnfiltered( bCheckSum );
}

//------------------------------------------------------------------------------
// Function:    ClearFuncBuff
// Description: clears function result buffer
//------------------------------------------------------------------------------

static void ClearFuncBuff(void)
{
    uint8_t i;

    for ( i = 0; i < FUNC_RES_SIZE; i++)
    {
        FuncRes[i] = 0;
    }
}


//------------------------------------------------------------------------------
// Function:    SiMonUartAckPacketSend
// Description: Sends the ACK packet back to SiIMon to acknowledge data reception.
//------------------------------------------------------------------------------

static void SiMonUartAckPacketSend(void)
{
    // WritePacket Acknowledge
    SiiPlatformUartPutcUnfiltered(0xff);
    SiiPlatformUartPutcUnfiltered(I2CCmd.OpCode);
    SiiPlatformUartPutcUnfiltered(0x1);
    SiiPlatformUartPutcUnfiltered(I2CCmd.NDataBytes);
}

//------------------------------------------------------------------------------
// Function:    SiMonUartSendFwVersion
// Description: Sends firmware version signature
//------------------------------------------------------------------------------

static void SiMonUartSendFwVersion ( void )
{
    uint8_t i;
    uint8_t len = 0;
    char    *pStr = buildVersion;

    // Find the length of the version signature
    while (*pStr++)
    {
        len++;
    }

    SiiPlatformUartPutcUnfiltered(0xff);
    SiiPlatformUartPutcUnfiltered(0xE2);
    SiiPlatformUartPutcUnfiltered(len);

    for (i = 0; i < len; i++)
    {
        SiiPlatformUartPutcUnfiltered(buildVersion[i]);
    }

}

//------------------------------------------------------------------------------
// Function:    SiMonUartSendCommState
// Description: Sends Comm Mode state
//------------------------------------------------------------------------------

static void SiMonUartSendCommState ( void )
{
    SiiPlatformUartPutcUnfiltered(0xff);
    SiiPlatformUartPutcUnfiltered(I2CCmd.OpCode);
    SiiPlatformUartPutcUnfiltered(0x01);
    if( l_hgComRequest )
    {
    	SiiPlatformUartPutcUnfiltered(0x01);
    }
    else
    {
    	SiiPlatformUartPutcUnfiltered(0x00);
    }
}

//------------------------------------------------------------------------------
// Function:    SiMonUartSendData
// Description: Sends data which has been reading from IIC device with 8 bit
//              addressing mode.
//------------------------------------------------------------------------------

static void SiMonUartSendData ( void )
{
	uint8_t i;

	
	SiiPlatformUartPutcUnfiltered(0xff);
	SiiPlatformUartPutcUnfiltered(I2CCmd.OpCode);
	SiiPlatformUartPutcUnfiltered(I2CCmd.NDataBytes);	// Send command length

	BlockRead_8BA((I2CCommandType *)&I2CCmd);

	for (i = 0; i < I2CCmd.NDataBytes; i++) 
	{
		SiiPlatformUartPutcUnfiltered(I2CCmd.Data[i]);
	}

}

//------------------------------------------------------------------------------
// Function:    SiMonUartWriteData
// Description: Writes data to IIC device with 8 bit addressing mode.
//------------------------------------------------------------------------------

static bool_t SiMonUartWriteData(void)
{	
	return (BlockWrite_8BA((I2CCommandType *)&I2CCmd) == IIC_OK);
}

//------------------------------------------------------------------------------
// Function:    SiMonUartWriteOsdData
// Description: Writes a block of data to IIC device in FIFO manner.
//------------------------------------------------------------------------------

static void SiMonUartWriteFifoData(void)
{
    uint8_t i;
    uint8_t data[1];
    bool_t isSuccess = false;
    int numBytes = I2CCmd.NDataBytes;

    for (i = 0; i < numBytes; i++)
    {
        data[0] = I2CCmd.Data[i];
        isSuccess = SiiPlatformI2cWriteBlock(0, I2CCmd.SlaveAddr, I2CCmd.RegAddrL, (uint8_t *) data, 1);
    }

    if (isSuccess == true)
    {
        SiMonUartAckPacketSend();
    }
}

//------------------------------------------------------------------------------
// Function:    ParseCommand
// Description: Used for parsing of commands received through UART interface
//------------------------------------------------------------------------------

static void ParseCommandHdmiGear(void)
{

    if (CmdPtr->cmdbuf[0] >= BEG_FUNC_CALLS) // check for function space Id
    {
        ClearFuncBuff();

        if (CmdPtr->cmdbuf[0] != FUNC_RESULT)
        {
            switch (CmdPtr->cmdbuf[0])
            // function decoding
            {
                default:
                    SiiPlatformUartPutcUnfiltered(COMMAND_NOT_SUPPORTED);
                    break;
            }
        }
    }
    else if (CmdPtr->cmdbuf[0] == FUNC_RESULT)
    {
        CommAccepted();
        SendResultOfFunction();
    }
    else
    {
        switch (CmdPtr->cmdbuf[0])
        // command decoding
        {
            case IIC8BARead:
                SendDataFrom8BAIICDevice();
                break;
            case IIC8BAWrite:
                WriteDataTo8BAIICDevice();
                break;
            case IIC16BARead:
                SendDataFrom16BAIICDevice();
                break;
            case IIC16BAWrite:
                WriteDataTo16BAIICDevice();
                break;
            case SetPassiveMode:
                CommAccepted();
                l_hgComRequest = true;
                break;
            case ResetPassiveMode:
                CommAccepted();
                l_hgComRequest = false;
                break;
            case SystemState: // Taken over to return current com mode state
                CmdPtr->MiscControl.Cmd = SystemState;
                CmdPtr->MiscControl.Data[0] = l_hgComRequest;
                CommAccepted();
                break;
            default:
                SiiPlatformUartPutcUnfiltered(COMMAND_NOT_SUPPORTED);
        }
    }

    SiiPlatformUartFlushTx(false);
    isBuffInUse = false;
}

//------------------------------------------------------------------------------
// Function:    ParseCommand
// Description: Used for parsing of commands received through UART interface
//------------------------------------------------------------------------------

static void ParseCommandSiimon(void)
{
    switch(I2CCmd.OpCode)  // command decoding
    {
        case SIMON_READ_REGISTER_BLOCK:
            SiMonUartSendData();
            // READ is an exception. No ACK needed.
            break;
        case SIMON_WRITE_REGISTER_BLOCK:
            if (SiMonUartWriteData())
            {
                SiMonUartAckPacketSend();
            }
            break;

        case SIMON_FW_GET_VERSION:
            SiMonUartSendFwVersion();
            // No ACK needed.
            break;

        case SIMON_FW_UPDATE_USB:
            SiiPlatformFirmwareUpdate(SI_FLASH_USB_DFU_BOOT_LOADER);
            SiMonUartAckPacketSend();
            break;
        case SIMON_FW_UPDATE_ROM_SERIAL:
            SiiPlatformFirmwareUpdate(SI_ROM_UART_BOOT_LOADER);
            SiMonUartAckPacketSend();
            break;
        case SIMON_FW_UPDATE_FLASH_SERIAL:
            SiiPlatformFirmwareUpdate(SI_FLASH_UART_BOOT_LOADER);
            SiMonUartAckPacketSend();
            break;
        case SIMON_FW_IR:
            SiiMonCbIrCmdSet(CmdPtr->IrUart.IrKey, CmdPtr->IrUart.IrAddress);
			SiMonUartAckPacketSend();
	       	break;
       	case SIMON_FW_FIFO_WRITE:
       		SiMonUartWriteFifoData();
            break;
    	case SIMON_COMM_MODE:
            if (CmdPtr->cmdbuf[2] == SII_COMM_MODE)
            {
                 l_hgComRequest = true;
                 // Disable reaction to device h/w interrupts
                 SiiPlatformInterruptDisable();
            }
            else
            {
                 l_hgComRequest = false;
                 // Recover h/w interrupt processing
                 SiiPlatformInterruptEnable();
            }
            SiMonUartAckPacketSend();
            break;
    	case SIMON_GET_COMM_MODE_STATE:
    		SiMonUartSendCommState();
    		break;
    }

   isBuffInUse = false;
}

//------------------------------------------------------------------------------
// Function:    GetCommand
// Description: Receives command, makes check sum check
//------------------------------------------------------------------------------

static uint8_t GetCommandHdmiGear(void)
{
    uint8_t bCommandLength, bCheckSum, i;
    uint8_t qResult = false;

    if ( bRxIndex == 0 )
        return( false );

    if ( RXBuffer[0] & 0x80 )
        bCommandLength = RXBuffer[1] + 6; // 6 is header length
    else
        bCommandLength = 6;

    if ( bRxIndex >= RX_BUF_SIZE) // PC speed is too fast, it sent more BYTEs than
                                  // processed by MCU
    {
        bRxIndex = 0; bCommState = WaitStartRX;
    }
    else if ((bCommandLength + 2) == bRxIndex)  // it's good, but need to check bCheckSum anyway
    {

        isBuffInUse = true;
        bCheckSum = START_FRAME_RX;
        bRxIndex = 0; bCommState = WaitStartRX;
        CmdPtr = (CmdType *)&RXBuffer[0];

        for ( i = 0; i <= bCommandLength; i++)
            bCheckSum += RXBuffer[i];
        if ( bCheckSum == RXBuffer[bCommandLength + 1] )
            qResult = true;
        else
            isBuffInUse = false;
    }
    return( qResult );
}

//------------------------------------------------------------------------------
// Function:    GetCommand
// Description: Receives command, makes check sum check
//------------------------------------------------------------------------------

static uint8_t GetCommandSiimon(void)
{
    uint8_t bCommandLength, i;
    uint8_t qResult = false;

    if ( bRxIndex == 0 )
        return( false );

    if (bRxIndex >= 2)
    {
        bCommandLength = RXBuffer[IDX_PACKET_LEN];
        if ( bRxIndex >= RX_BUF_SIZE) // PC speed is too fast, it sent more BYTEs than processed by MCU
        {
            bRxIndex = 0; bCommState = WaitStartRX;
        }
        if ((bCommandLength + 2) == bRxIndex)
        {

            isBuffInUse = true;
            bRxIndex = 0;
            bCommState = WaitStartRX;
            CmdPtr = (CmdType *)&RXBuffer[0];

            //get a copy of the packet
            I2CCmd.OpCode = CmdPtr->cmdbuf[IDX_OPCODE];
            I2CCmd.NPacketBytes = CmdPtr->cmdbuf[IDX_PACKET_LEN];
            I2CCmd.SlaveAddr = CmdPtr->cmdbuf[IDX_DEVICE_ID];
            I2CCmd.RegAddrL = CmdPtr->cmdbuf[IDX_OFFSET];
            I2CCmd.RegAddrH = 0x00;
            I2CCmd.NDataBytes = CmdPtr->cmdbuf[IDX_DATA_LEN];
            for (i=0; i < I2CCmd.NDataBytes; i++)
            {
                I2CCmd.Data[i] = CmdPtr->cmdbuf[IDX_DATA_START + i];
            }
            bRxIndex = 0;
            qResult = true;
        }
    }
    return( qResult );
}

//------------------------------------------------------------------------------
//  Function:    SkRemoteRequestHandler
//  Description: Returns TRUE if SiIMon wants control.  Checks for HDMIGear
//               requests and processes them if found.  If HDMIGear requests
//               Passive mode, return TRUE.
//
//  SiIMon      Signals that it wants control by forcing busRequest LOW.
//  HDMI_Gear   Signals that it wants control by sending the SetPassiveMode
//              command, which sets l_hgComRequest true.
//
//------------------------------------------------------------------------------

bool_t SkRemoteRequestHandler ( void )
{

    // If SiIMon requests access, it has priority.
    if ( SiiPlatformSiimonBusRequest())
    {
        if (l_hgSiimonMode == false)
        {
            SiiPlatformAllInterruptsDisable(); // Freeze IR & GPIO interrupt handlers
            SiiPlatformSiimonBusGrant( true ); // Grant access to SiIMon
            l_hgSiimonMode = true;
        }
        return( true );
    }

    if (l_hgSiimonMode == true)
    {
        SiiPlatformSiimonBusGrant( false );   // Release bus for local use.
        SiiPlatformAllInterruptsEnable();     // Unfreeze IRQ handlers
        l_hgSiimonMode = false;
    }

    // Check for HDMIGear commands.
    if (DebugSource == SOURCE_IS_HDMIGEAR)
    {
        if ( GetCommandHdmiGear() )
        {
            ParseCommandHdmiGear();
        }
    }
    else
    {
        if ( GetCommandSiimon() )
        {
            ParseCommandSiimon();
        }
    }

    return( l_hgComRequest );
}


//------------------------------------------------------------------------------
// Function:    SkSerialConsumedChar
// Description: Check if receiving is locked until after finishing of command processing
//------------------------------------------------------------------------------

bool_t SkSerialConsumedChar ( void )
{
    if (!isBuffInUse)
    {
   		consumedChar = false;
    }
    else
    {
		consumedChar = true;
    }
	return consumedChar;
}

//------------------------------------------------------------------------------
// Function:    SkSerialRxHandler
// Description: Handle incoming chars from UART
//------------------------------------------------------------------------------

bool_t SkSerialRxHandler ( uint8_t achar )
{ 			
	if (bCommState == ReadyRX)
	{
	    if (bRxIndex < RX_BUF_SIZE)
	    {
	        RXBuffer[bRxIndex++] = achar;
	    }
	}
	else
	{
	    if ((achar == START_FRAME_RX_SIMON) || (achar == START_FRAME_RX))
	    {
	        bCommState = ReadyRX;
	        bRxIndex = 0;
	        if (achar == START_FRAME_RX_SIMON)
	        {
	            DebugSource = SOURCE_IS_SIMON;
	        }
	        else
	        {
	            DebugSource = SOURCE_IS_HDMIGEAR;
	        }
	    }

	}
	return true;
}

