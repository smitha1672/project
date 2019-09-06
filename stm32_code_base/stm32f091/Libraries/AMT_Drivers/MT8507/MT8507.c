/********** Includes **********************************************************/
#include "Main.h"
#include "MT8507.h"
#include "MT8507_interface.h"
#include "VirtualTimer.h"

//________________________________________________________________________________________________________
extern MT8507_stack m_MT8507_stack;

//________________________________________________________________________________________________________
#define __MT8507_CMD_MMI_ACTION   0x02

//________________________________________________________________________________________________________
static MT8507_Command mMT8507_Cmd;

//________________________________________________________________________________________________________
static byte MT8507_Driver_Checksum( byte *pBuf, byte length )
{
    byte result = *pBuf; 
    byte i = 0;

    for( i = 1; i < length; i++ )
    {
        result += *(pBuf+i);
    }

    result = 0x100 - result;

    return result; 
}

void MT8507_Driver_GetFrimwareVersion( void )
{
    mMT8507_Cmd.CMD_IN[0] = 0xAA;
    mMT8507_Cmd.CMD_IN[1] = 0x00;
    mMT8507_Cmd.CMD_IN[2] = 0x02;
    mMT8507_Cmd.CMD_IN[3] = 0x08;
    mMT8507_Cmd.CMD_IN[4] = 0x00;    
    mMT8507_Cmd.CMD_IN[5] = MT8507_Driver_Checksum( &mMT8507_Cmd.CMD_IN[1], 4);    /*checksum is from NAD to data[n-2]*/

    mMT8507_Cmd.CMD_IN_Length = 6;

    MT8507_ITF_SendCommand( &mMT8507_Cmd );
}

void MT8507_Driver_PassFactoryCommand( void *params )
{
    uint8 *pSrc = 0;
    int   i = 0;

    if (params == NULL)
    {
        return;
    }

    pSrc = (uint8 *)(params); /*copy params address*/

    mMT8507_Cmd.CMD_IN_Length = *(pSrc+2);

    for ( i = 0; i < mMT8507_Cmd.CMD_IN_Length ; i++ )
    {
        mMT8507_Cmd.CMD_IN[i] = *(pSrc+i);
    }


    MT8507_ITF_SendCommand( &mMT8507_Cmd );
}

void MT8507_Driver_DSPCmdACK( MT8507_EventPacket *packet )
{
    if (packet == NULL)
    {
        return;
    }

    mMT8507_Cmd.CMD_IN_Length = 7;
    mMT8507_Cmd.CMD_IN[0] = 0xA0;
    mMT8507_Cmd.CMD_IN[1] = packet->Opcode;
    mMT8507_Cmd.CMD_IN[2] = 0x07;
    mMT8507_Cmd.CMD_IN[3] = m_MT8507_stack.rxBuffer[packet->DataStartPtr];
    mMT8507_Cmd.CMD_IN[4] = m_MT8507_stack.rxBuffer[packet->DataStartPtr + 1];
    mMT8507_Cmd.CMD_IN[5] = 0xE0;
    mMT8507_Cmd.CMD_IN[6] = packet->event_checksum[0];

    MT8507_ITF_SendCommand( &mMT8507_Cmd );
}

void MT8507_Driver_MMI_Action( MMI_MT8507 value )
{
    uint8 parameter = 0x00;

    switch( value )
    {
        case MMI_STOP_MUISC:
            parameter = 0x33;
            break;

        case MMI_PLAY_PAUSE:
            parameter = 0x32;
            break;
        
        case MMI_BACKWARD:
            parameter = 0x35;
            break;
        
        case MMI_FORWARD:
            parameter = 0x34;   
            break;

        case MMI_RESET_DEFAULT:
            parameter = 0x56;   
            break;

        case MMI_FAST_ENTER_PAIRING_MODE:
            parameter = 0x5D;   
            break;

        case MMI_DISCONNECT_A2DP_LINK:
             parameter = 0x3B;
            break;

        default:
            return;
            break;
    }

    mMT8507_Cmd.CMD_IN[0] = 0xAA;
    mMT8507_Cmd.CMD_IN[1] = 0x00;
    mMT8507_Cmd.CMD_IN[2] = 0x03;
    mMT8507_Cmd.CMD_IN[3] = __MT8507_CMD_MMI_ACTION;
    mMT8507_Cmd.CMD_IN[4] = 0x00;
    mMT8507_Cmd.CMD_IN[5] = parameter; 
    mMT8507_Cmd.CMD_IN[6] = MT8507_Driver_Checksum( &mMT8507_Cmd.CMD_IN[1], 5);    /*checksum is from NAD to data[n-2]*/

    mMT8507_Cmd.CMD_IN_Length = 7;

    MT8507_ITF_SendCommand( &mMT8507_Cmd );
}

void MT8507_SendReadyToReceiveData( uint16 DataNum )
{
    uint8 i;

    mMT8507_Cmd.CMD_IN[0] = 0xB1;
    mMT8507_Cmd.CMD_IN[1] = 0x00;
    mMT8507_Cmd.CMD_IN[2] = 0x06;
    mMT8507_Cmd.CMD_IN_Length = 0x06;

    mMT8507_Cmd.CMD_IN[3] = (uint8)DataNum;
    mMT8507_Cmd.CMD_IN[4] = (uint8)(DataNum >> 8);

    mMT8507_Cmd.CMD_IN[5] = 0;
    for ( i = 0 ; i < 5 ; i++ )
    {
        mMT8507_Cmd.CMD_IN[5] ^= mMT8507_Cmd.CMD_IN[i];
    }

//    total_time_old = VirtualTimer_now() - total_time_old;
//    TRACE_ERROR((0, "Send ACK %d\n", total_time_old));
//    total_time_old = VirtualTimer_now();
	
    MT8507_ITF_SendCommand( &mMT8507_Cmd );
}

