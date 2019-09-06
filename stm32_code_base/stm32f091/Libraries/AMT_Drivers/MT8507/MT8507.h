#ifndef __MT8507_H
#define __MT8507_H

/********** Includes **********************************************************/
#include "Defs.h"
#include "stm32f0xx.h"
#include <stdio.h>
#include "AudioSystemDriver.h"
#include "STM32F0xx_board.h"

typedef struct
{
    /** Buffer storing all characters received and pending to be read */
//    byte CMD_OUT[16];
//    byte CMD_OUT_Length ;
    uint8 CMD_IN[16];
    uint8 CMD_IN_Length ;
} MT8507_Command;


typedef enum {
    MMI_STOP_MUISC = 0,
    MMI_PLAY_PAUSE,
    MMI_FORWARD,
    MMI_BACKWARD,
    MMI_FAST_ENTER_PAIRING_MODE,
    MMI_DISCONNECT_A2DP_LINK,
    MMI_RESET_DEFAULT
}MMI_MT8507;

void MT8507_Driver_MMI_Action( MMI_MT8507 value );

void MT8507_Driver_PassFactoryCommand( void *params ); //Elvis:

void MT8507_SendReadyToReceiveData( uint16 DataNum );

#endif /* __MT8507_H */
