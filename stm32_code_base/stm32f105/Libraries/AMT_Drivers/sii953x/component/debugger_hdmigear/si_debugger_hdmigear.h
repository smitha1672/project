//***************************************************************************
//!file     si_debugger_hdmigear.h
//!brief    Silicon Image Starter Kit HDMI-Gear debugger interface.
//
// No part of this work may be reproduced, modified, distributed,
// transmitted, transcribed, or translated into any language or computer
// format, in any form or by any means without written permission of
// Silicon Image, Inc., 1060 East Arques Avenue, Sunnyvale, California 94085
//
// Copyright 2010, Silicon Image, Inc.  All rights reserved.
//***************************************************************************/

#if !defined __SK_DEBUGGER_H__
#define __SK_DEBUGGER_H__
#include "si_common.h"


//------------------------------------------------------------------------------
// Configuration Defines
//------------------------------------------------------------------------------
#define RX_BUF_SIZE     26
#define FUNC_RES_SIZE   33      //in bytes


//------------------------------------------------------------------------------
// Other Defines
//------------------------------------------------------------------------------
#define BEG_FUNC_CALLS          0xA0
#define FUNC_RESULT             0x7F

#define NO_ACK_FROM_IIC_DEV     0xF1
#define COMMAND_NOT_SUPPORTED   0xAA

#define COM_ACK                 0xFF
#define COM_ACK_SII_INTERRUPT   0xFE
#define COM_ACK_IIC_CAPTUTED    0xF0
#define COM_ACK_IIC_NOACK       0xF1
#define COM_ACK_MDDC_CAPTUTED   0xF3
#define COM_ACK_MDDC_NOACK      0xF4

#define FLG_SHORT               0x01    // Used for Ri Short Read
#define FLG_NOSTOP              0x02    // Don't release IIC Bus
#define FLG_CONTD               0x04    // Continued from previous operation

#define CTR_BUSNUM              0x01    // Mask for bus number (only 0 or 1 for now)

#define MDDC_READ       1
#define MDDC_WRITE      0
#define MDDC_NOACK      4

#define IIC_CAPTURED    1
#define IIC_NOACK       2
#define IIC_OK          0

#define START_FRAME_RX  's'
#define START_FRAME_RX_SIMON  0xFF

#define USE_UART_SIMON
enum 
{
	SOURCE_IS_HDMIGEAR,
	SOURCE_IS_SIMON,		
};

// Command indexes:
//=====================
#define IDX_OPCODE						0
#define IDX_PACKET_LEN					1
#define IDX_DEVICE_ID					2
#define IDX_OFFSET						3
#define IDX_DATA_LEN					4
#define IDX_DATA_START					5

enum
{
    SIMON_COMM_MODE =               0x11,
    SIMON_WRITE_REGISTER_BLOCK =    0x60,
    SIMON_READ_REGISTER_BLOCK =     (0x80 | SIMON_WRITE_REGISTER_BLOCK),
    SIMON_GET_COMM_MODE_STATE =     0x91,
    SIMON_FW_GET_VERSION =          0xE2,
    SIMON_FW_UPDATE_USB =           0xF0,
    SIMON_FW_UPDATE_ROM_SERIAL =    0xF1,
    SIMON_FW_UPDATE_FLASH_SERIAL =  0xF2,
    SIMON_FW_IR =                   0xF3,
    SIMON_FW_FIFO_WRITE =           0xF5,
};

#define SII_COMM_MODE		            1


//------------------------------------------------------------------------------
// Local Types
//------------------------------------------------------------------------------
typedef struct
{
    uint8_t Dummy;
    uint8_t NBytes;
    uint8_t Flags;
    uint8_t Ctrl;
    uint8_t SlaveAddr;
    uint8_t RegAddrL;
    uint8_t RegAddrH;
    uint8_t Data[16];
} I2CCommandType;

typedef struct
{
    uint8_t OpCode;
    uint8_t NDataBytes;
    uint8_t NPacketBytes;
    uint8_t Ctrl;
    uint8_t SlaveAddr;
    uint8_t RegAddrL;
    uint8_t RegAddrH;
    uint8_t Data[16];
} I2CCommandTypeSiMon;

typedef struct
{
    uint8_t OpCode;
    uint8_t NPacketBytes;
    uint8_t Ctrl;
    uint8_t NDataBytes;
    uint8_t Data[16];
} I2CSpiSiMon;

typedef struct
{
    uint8_t Cmd;
    uint8_t notused1;
    uint8_t notused2;
    uint8_t Ctrl;
    uint8_t notused4;
    uint8_t notused5;
} ResetControlType;

typedef struct
{
    uint8_t Cmd;
    uint8_t notused1;
    uint8_t notused2;
    uint8_t notused3;
    uint8_t notused4;
    uint8_t notused5;
    uint8_t Data[1];

} MiscControlType;

typedef struct
{
    uint8_t Dummy;
    uint8_t NBytes;
    uint8_t Flags;
    uint8_t Ctrl;
    uint8_t SlaveAddr;
    uint8_t RegAddrL;
    uint8_t RegAddrH;
    uint8_t Data[16];
} FuncCommandType;

typedef struct
{
    uint8_t OpCode;
    uint8_t Dummy[3];
    uint8_t IrKey;
    uint8_t IrAddress;
    uint8_t IrMode;
    uint8_t Data[16];
} IrCmd;


typedef union  CmdUnion
{
    uint8_t cmdbuf[24];

    I2CCommandType I2CCommand;
    ResetControlType ResetControl;
    MiscControlType MiscControl;
    FuncCommandType FuncCommand;
    IrCmd IrUart;
    I2CSpiSiMon SpiUart;
} CmdType;

typedef enum
{
    IIC8BARead,
    IIC10BARead,
    IIC16BARead,
    SystemState,
    VersionInfo,
    IIC8BAWrite = 0x80,
    IIC10BAWrite,
    IIC16BAWrite,
    Resets,
    SetPassiveMode,
    ResetPassiveMode,
    Syncronyze,
    GetErr,
    ResetRX,
    RXAPI_GetNCTS = 0xDB,
    RXAPI_GetABKSV = 0xDC,
    RXAPI_GetIPacket = 0xDD,
    RXAPI_GetVideoInputResolution = 0xDE,
    API_GetLastAPI_ExeTime = 0xDF,
    RXAPI_InitializeSystem = 0xE0,
    RXAPI_GetVersionInfo =  0xE1,
    RXAPI_SetVideoFormat =  0xE2,
    RXAPI_GetVideoFormat =  0xE3,
    RXAPI_GetInfo =         0xE4,
    RXAPI_SetGlobalPower =     0xE5,
    RXAPI_SetAudioVideoMute =  0xE6,
    RXAPI_GetAPIInfo =      0xE7,
    RXAPI_GetSystemInformation = 0xE8,
    RXAPI_GetTasksSchedule =    0xE9,
    RXAPI_SetMasterClock =      0xEA,
    RXAPI_GetAudioInputStatus = 0xEB,
    RXAPI_GetAudioOutputFormat = 0xEC,
    RXAPI_SetAudioOutputFormat = 0xED,
    RXAPI_SetVideoInput = 0xEE,
    API_GetWarningData = 0xEF,
    API_GetErrorData = 0xF0,

    CmdGetGPIO = 0xF1,
    CmdSendGPIO = 0xF2,
    TestSyncSWRst = 0xF3,
    TestPLLId = 0xF4,
    TX_AudioPath = 0xF5,
    AVIInfo = 0xF7,
    TX_VideoPath = 0xF8,
    TX_DE_656_AudioPresetCfg = 0xF9,
    TXVideoAudioCfg = 0xFA,
    MakeRecoveryClock = 0xFB,
    GetAudioStatData = 0xFC,
    GoAudioStatTest = 0xFD,
    OutVCfg = 0xFE,
    GetTimingPrms = 0xFF

} CommandId;

enum ComStates
{
    WaitStartRX = 0,
    ReadyRX = 1,
};


#if ( configSII_DEV_953x_PORTING == 0 )
//------------------------------------------------------------------------------
// Debugger functions
//------------------------------------------------------------------------------

bool_t SkRemoteRequestHandler( void );
bool_t SkSerialConsumedChar ( void );

#endif 



#endif  // __SK_DEBUGGER_H__
