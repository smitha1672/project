#ifndef __MT8507_INTERFACE_H__
#define __MT8507_INTERFACE_H__

#include "Defs.h"
#include "MT8507.h"
  
#define _WORD    64//Elvis org: 16
#define RX_BUFFER_MAX (1024*2)//Elvis org: (128*2)
#define TX_BUFFER_MAX (_WORD)
//#define FLASH_BUFFER_MAX (1017)//(1024*16)//Elvis org: (128*2)
#define FLASH_BUFFER_MAX (1024*16)//Elvis org: (128*2)

typedef enum{
    MT8507_STATE_IDLE    = 0,
    MT8507_STATE_CMD_IN,
    MT8507_STATE_GET_EVENT_CODE,
    MT8507_STATE_WAIT,
    MT8507_STATE_WATE_TIMEOUT,
}MT8507_ITF_State;

typedef enum{
    MT8507_PAIR_State_IDLE    = 0,
    MT8507_PAIR_State_START,
    MT8507_PAIR_State_SUCCESS,
    MT8507_PAIR_State_FAIL,
    MT8507_PAIR_State_TIMEOUT,
}MT8507_PAIR_State;

typedef enum{
    MCU_IDLE = 0,
    MT8507_Command_Interrupt,
    MT8507_Command_Receiving,
    MT8507_Command_Processing,
    MT8507_Command_Imcomplete,
    MT8507_Command_Receiving_Timeout,
    MT8507_Command_Decoding,
    SPI_Flash_Update_Process_Start,
    SPI_Flash_Update_Process_Wait_for_Next_Data,
    SPI_Flash_Update_Process_Data_Copying,
    SPI_Flash_Update_Process_System_Reboot,
}MCU_Mode;

typedef enum{
    MCU_DSP,
    MCU_Only,
    DSP_Only,
    NONE = 0xf,
}SPI_Flash_Update_Mode;

// Private data type definition
typedef struct
{
    /*when rxPushPtr == rxPopPtr, It indicates all data has been popped from stack*/
    byte rxBuffer[RX_BUFFER_MAX];    /*The buffer store datas from UART.RX */
    uint16 rxPushPtr;                  /*Index ++ when data is pushed to buffer stack*/
    uint16 rxPopPtr;                   /*Index ++ when data is got from buffer stack*/

    byte txBuffer[TX_BUFFER_MAX];     /*The buffer datas need to push to UART.TX */
    byte txPushPtr;
    byte txPopPtr;
} MT8507_stack;

typedef struct
{
    byte header;
    uint8 Opcode;
    uint16 length;
    uint16 DataStartPtr;
    byte event_checksum[2];
}MT8507_EventPacket;

/* Struct for easy bit access */
typedef union DeviceStateBits {
    struct {
        unsigned power_status:1;
        unsigned pairing_mode:1;
        unsigned pairing_status:1;
        unsigned hf_connection_status:1;
        unsigned a2dp_connection_status:1;
        unsigned sco_connection_status:1;
        unsigned avrcp_connection_status:1;
        unsigned report_volume:1;
    } bits;
    unsigned char byte;
} MT8507_DeviceState;



//_______________________________________________________________________________________
bool MT8507_ITF_SendCommand( MT8507_Command *pCMD );

uint16 MT8507_ITF_getReceivedCount( void );

uint16 MT8507_ITF_GetData( uint8_t *dest, uint16 len );

byte MT8507_ITF_GetDeviceState( void );
uint8 MT8507_ITF_GetPowerState( void );

bool MT8507_ITF_IsEventComming( void );

void MT8507_ITF_ClrEventComming();

void MT8507_ITG_ClrDeviceState( void );

bool MT8507_ITF_GetPlayStatus(void);

void MT8507_ITF_SetPairingStatusIdle(void);

MT8507_PAIR_State MT8507_ITF_GetPairingStatus(void);

void MT8507_ProtocolService_DispatcherInitialize( void );

void MT8507_ProtocolService_DispatcherController( bool enable );

#endif /*__MT8507_INTERFACE_H__*/
