#include "stm32f0xx_conf.h"
#include "STM32F0xx_board.h"

#include "stm32f0xx.h"
#include "ServicesInc.h"

#include "MT8507_interface.h"

#include "string.h"

#include "Ext_flash_driver.h"

//______________________________________________________________________________
#define MT8507_ITF_RX_IRQHandler         USART2_IRQHandler 

#define MT8507_EVENT_COMMAND_TYPE 1
#define MT8507_EVENT_LENGTH 2
#define MT8507_EVENT_PARAMS 3
#define MT8507_FLASH_DATA 5
//#define MT8507_EVENT_START_POS 0
//#define MT8507_EVENT_LENGTH_H_POS 1
//#define MT8507_EVENT_LENGTH_L_POS 2
//#define MT8507_EVENT_LENGTH_PARAMS_POS 3
#define MT8507_DEBUG

#define MCU_IMAGE_START  (uint32_t)0x400000-0x40000 /* give a 4 sector size*/
//#define MCU_IMAGE_START  (uint32_t)0 /* give a 4 sector size*/
#define MCU_IMAGE_END    (uint32_t)0x400000 /*include dummy*/

//______________________________________________________________________________
uint8_t RxBuffer[RX_BUFFER_MAX] = {0};
__IO uint16_t RxCounter = 0; 
uint16_t RxBuffer_Index = 0; 
uint16_t RxBuffer_Counter = 0; 
uint8_t Flash_Buffer[FLASH_BUFFER_MAX] = {0};
uint16_t FlashBuffer_Index = 0; 

uint8_t rFlash_Buffer[1024] = {0};
uint32 Buffter_Round = 0;
uint16 Buffter_Item = 0;
uint32 tFlashDestination = 0;

//Private variable______________________________________________________________
MT8507_stack m_MT8507_stack;
static MT8507_ITF_State mMT8507_UITF_state = MT8507_STATE_GET_EVENT_CODE;
MT8507_DeviceState mMT8507_DeviceStatus;
MT8507_EventPacket mMT8507_EventPacket = {0};
MCU_Mode mMCUMode = MCU_IDLE;
SPI_Flash_Update_Mode Update_Mode = NONE;
uint16 MaxDataNum = 0xFFFF;
uint16 CurrentDataNum = 0xFFFF;
uint32 TotalDataSize = 0xFFFFFFFF;
uint16 CurrentDataSize = 0xFFFF;
uint32 FlashDestination = 0;
uint16 MT8507_Command_Wait_Count = 0;

static bool bIsMT8507UITF_enable = TRUE;
static bool bIsMT8507_ProtocalService = FALSE;
static int mMT8507_DispatcherID = -1;
static bool mIsEventComming = FALSE;
static MT8507_PAIR_State mMT8507_Pairing_Status = MT8507_PAIR_State_IDLE;
static uint16 ReceivedDataNum = 0;
static bool bIsDataDeliving = FALSE;
static bool bIsAllDataTransmitted = FALSE;

uint8 mDebugBT = 0;
static bool bIsMT8507_playing = FALSE;

//______________________________________________________________________________
uint32 InterruptCounter = 0;
//extern uint32 total_time_old;
extern void putch(char ch);

//______________________________________________________________________________
static void MT8507_ITF_SendDatas( uint8 *src, uint16 length );
static void MT8507_DSPCommand_Bypass_Process( void );
static void MT8507_SPIFlash_Update_Process( void );
static void MT8507_SPIFlash_Update_Process_test( void );
static void MT8507_FactoryCommand_Bypass_Process( uint8 PacketLen );
static void MT8507_SPIFlashUpdatingInitial( void );
static void MT8507_FlashData_handle( void );
static void MT8507_FlashData_Write( uint16_t NumByteToWrite );
static void MT8507_FlashData_Read( uint16_t NumByteToWrite );
static uint32 MT8507_GetDataNumberFromStack( uint16 Start, uint8 Length );
static void MT8507_Initial_8507_stack_Rx( void );
static void MT8507_Initial_8507_stack_Tx( void );
static void MT8507_Initial_8507_EventPacket( void );

//______________________________________________________________________________
void MT8507_ITF_RX_IRQHandler(void)
{
    uint16_t data = 0;
    
    if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) == SET)
    {
#if 1
        if( RxBuffer_Counter == RX_BUFFER_MAX )
        {
            //RxCounter = 0;        
            RxBuffer_Counter = 0;
            USART_DeInit(USART2);               
            Uart2_Initialization();
            return;
        }

        RxBuffer[RxBuffer_Counter++] = (USART_ReceiveData(USART2) & 0x00FF);
	RxCounter++;
#else      
        data = (USART_ReceiveData(USART2) & 0x00FF);
        USART_ClearFlag(USART2,USART_FLAG_ORE);
        return;
#endif
    }

    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        if (bIsMT8507_ProtocalService == FALSE )
        {
            data = (USART_ReceiveData(USART2) & 0x00FF);
            USART_ClearITPendingBit(USART2, USART_IT_RXNE);            
            return;
        }
        
        if ( RxBuffer_Counter == RX_BUFFER_MAX )
            RxBuffer_Counter = 0;

        /* Read one byte from the receive data register */
        RxBuffer[RxBuffer_Counter++] = (USART_ReceiveData(USART2) & 0x00FF);
        RxCounter++;
    }
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);    

//    if ( total_time_old == 0xffffffff )
//        total_time_old = VirtualTimer_now();

    //InterruptCounter++;

    if ( RxCounter >= 3 )
    {
        if ( mMT8507_EventPacket.length == 0 )
            mMT8507_UITF_state = MT8507_STATE_GET_EVENT_CODE;//Elvis
        else if ( RxBuffer_Counter >= mMT8507_EventPacket.length )
            mMT8507_UITF_state = MT8507_STATE_GET_EVENT_CODE;//Elvis
        else
            mMT8507_UITF_state = MT8507_STATE_WAIT;//Elvis
    }
}

bool MT8507_ITF_IsEventComming( void )
{
    return mIsEventComming;
}

void MT8507_ITF_ClrEventComming( void )
{
    mIsEventComming = FALSE;
}

byte MT8507_ITF_GetDeviceState( void )
{
    return mMT8507_DeviceStatus.byte;
}

uint8 MT8507_ITF_GetPowerState( void )
{
    return mMT8507_DeviceStatus.bits.power_status;
}

void MT8507_ITG_ClrDeviceState( void )
{
    mMT8507_DeviceStatus.byte = 0;
}

bool MT8507_ITF_GetPlayStatus(void){
    return bIsMT8507_playing;
}

void MT8507_ITF_SetPairingStatusIdle(void)
{
    mMT8507_Pairing_Status = MT8507_PAIR_State_IDLE;
}
    
 MT8507_PAIR_State MT8507_ITF_GetPairingStatus(void)
{
    return mMT8507_Pairing_Status;
}

static void MT8507_Command_Decode( MT8507_EventPacket *packet ) 
{
    byte CmdType = packet->header & 0xF0;

    switch ( CmdType )
    {
        case 0xA0:
        {
            MT8507_DSPCommand_Bypass_Process();
        }
        break;
		
        case 0xB0:
        {
            MT8507_SPIFlash_Update_Process();
        }
        break;
		
        case 0xC0:
        {
            MT8507_FactoryCommand_Bypass_Process((uint8)packet->length);
        }
        break;
		
        default:
        {
            TRACE_DEBUG((0,"MT8507_Command_Decode ERROR!! \n"));
        }
    }
}

static void MT8507_DSPCommand_Bypass_Process( void )
{
        MT8507_Initial_8507_stack_Rx();
        MT8507_Initial_8507_EventPacket();
}

static void MT8507_SPIFlash_Update_Process( void )
{
    switch ( mMT8507_EventPacket.header )
    {
        case 0xB0:
        {
            switch ( mMT8507_EventPacket.Opcode )
            {
                case 0x00:
                {
                    mMCUMode = SPI_Flash_Update_Process_Start;
                    MT8507_SPIFlashUpdatingInitial();
                }
                    break;
					
                case 0x01: //Need consider different cases: Terminated, Timeout and Checksum Error 
                {
                    if ( CurrentDataNum == 0xFFFF )
                    {
                        MT8507_SendReadyToReceiveData(0xFF00);
                    }
                    else
                    {
                        MT8507_SendReadyToReceiveData(CurrentDataNum+1);
                    }
                }
                    break;
					
                default:
                {
                    TRACE_DEBUG((0,"MT8507_SPIFlash_Update_Process ERROR: No matched Opcode \n"));
                }
            }
		
        }
            break;

        case 0xB2:
        {
            if ( (Update_Mode == NONE) || (bIsAllDataTransmitted == TRUE) )
                return;

            CurrentDataNum = (uint16)MT8507_GetDataNumberFromStack(mMT8507_EventPacket.DataStartPtr, 2);
            //CurrentDataNum++;
            CurrentDataSize = MT8507_GetDataNumberFromStack(mMT8507_EventPacket.DataStartPtr+2, 2);
            TotalDataSize -= CurrentDataSize;
            //TotalDataSize -= (CurrentDataSize*2);

            if ( TotalDataSize == 0 )
                bIsAllDataTransmitted = TRUE;

            mMCUMode = SPI_Flash_Update_Process_Data_Copying;            
            MT8507_FlashData_handle();

            MT8507_Initial_8507_stack_Rx();
            MT8507_Initial_8507_EventPacket();

            //if ( CurrentDataNum == MaxDataNum )
            if ( bIsAllDataTransmitted == TRUE )
            {
                mMCUMode = SPI_Flash_Update_Process_System_Reboot;            
                MT8507_SendReadyToReceiveData(0xFFFF);
                SPI_MasterBoot_mode();
                //Start Upgrade Process
            }
            else
            {
                MT8507_SendReadyToReceiveData(CurrentDataNum+1);
                mMCUMode = SPI_Flash_Update_Process_Wait_for_Next_Data;
            }
        }
            break;

        default:
        {
            TRACE_DEBUG((0,"MT8507_SPIFlash_Update_Process ERROR: No matched header \n"));
        }
    }

}

static void MT8507_SPIFlash_Update_Process_test( void )
{
    if ( mMT8507_EventPacket.header == 0xB0 )
    {
            if ( mMT8507_EventPacket.Opcode == 0x00 )
            {
                {
                    mMCUMode = SPI_Flash_Update_Process_Start;
                    MT8507_SPIFlashUpdatingInitial();
                }
            }		
            else
            {
                    if ( CurrentDataNum == 0xFFFF )
                    {
                        MT8507_SendReadyToReceiveData(0xFF00);
                    }
                    else
                    {
                        MT8507_SendReadyToReceiveData(CurrentDataNum+1);
                    }
            }
		
        }

    else
        {
            CurrentDataNum = (uint16)MT8507_GetDataNumberFromStack(mMT8507_EventPacket.DataStartPtr, 2);
            CurrentDataSize = MT8507_GetDataNumberFromStack(mMT8507_EventPacket.DataStartPtr+2, 2);
            TotalDataSize -= CurrentDataSize;

            mMCUMode = SPI_Flash_Update_Process_Data_Copying;            
            //MT8507_FlashData_handle();

            //MT8507_Initial_8507_stack_Rx();
            //MT8507_Initial_8507_EventPacket();

            if ( CurrentDataNum == MaxDataNum )
            {
                MT8507_SendReadyToReceiveData(0xFFFF);
                bIsDataDeliving = FALSE;
                //Start Upgrade Process
            }
            else
            {
                MT8507_SendReadyToReceiveData(CurrentDataNum+1);
                mMCUMode = SPI_Flash_Update_Process_Wait_for_Next_Data;
            }
        }
}
static void MT8507_FactoryCommand_Bypass_Process( uint8 PacketLen )
{
    uint8 i;

    for ( i = 0 ; i < PacketLen ; i++ )
    {
        putch((char) (m_MT8507_stack.rxBuffer[m_MT8507_stack.rxPopPtr+i]));
	m_MT8507_stack.rxPopPtr++;
    }
}

static void MT8507_SPIFlashUpdatingInitial( void )
{
    //Set Image Releated Information
    Update_Mode = m_MT8507_stack.rxBuffer[mMT8507_EventPacket.DataStartPtr];
    //No only DSP
    #if 0
    if ( Update_Mode == 2 )
        FlashDestination = 0;
    else
        FlashDestination = MCU_IMAGE_START;
    #else
	FlashDestination = 0;
    #endif
	
    MaxDataNum = MT8507_GetDataNumberFromStack(mMT8507_EventPacket.DataStartPtr+1, 2);
    TotalDataSize = MT8507_GetDataNumberFromStack(mMT8507_EventPacket.DataStartPtr+3, 4);

    //Initial Buffer and Stack Pointer
    memset(RxBuffer, 0, RX_BUFFER_MAX);
    MT8507_Initial_8507_stack_Rx();
    MT8507_Initial_8507_EventPacket();

    FlashBuffer_Index = 0; 
    memset(Flash_Buffer, 0, FLASH_BUFFER_MAX);

    //Deinitial another UART
    //Uart1_Deinitialization();

    //Ready to Recevie Image Data
    MT8507_SendReadyToReceiveData(0);

    //Update mMCUMode
    mMCUMode = SPI_Flash_Update_Process_Wait_for_Next_Data;
}

static void MT8507_FlashData_handle( void )
{
    bool DataLeft = FALSE;
    uint32 UsedLen = 0;

#if 1
    if ( (FlashBuffer_Index + CurrentDataSize) < FLASH_BUFFER_MAX )
    {
        memcpy(Flash_Buffer+FlashBuffer_Index, m_MT8507_stack.rxBuffer+MT8507_FLASH_DATA, CurrentDataSize);
        FlashBuffer_Index += CurrentDataSize;
    }
    else
    {
        UsedLen = FLASH_BUFFER_MAX-FlashBuffer_Index;
        memcpy(Flash_Buffer+FlashBuffer_Index, m_MT8507_stack.rxBuffer+MT8507_FLASH_DATA, UsedLen);
        FlashBuffer_Index += UsedLen;
        DataLeft = TRUE;
    }

    if ( FlashBuffer_Index == FLASH_BUFFER_MAX )
    {

        MT8507_FlashData_Write((uint16_t)FLASH_BUFFER_MAX);
        //MT8507_FlashData_Read((uint16_t)FLASH_BUFFER_MAX);
        memset(Flash_Buffer, 0, FLASH_BUFFER_MAX);

        if ( DataLeft )
        {
            DataLeft = FALSE;
            FlashBuffer_Index = CurrentDataSize-UsedLen;
            memcpy(Flash_Buffer, m_MT8507_stack.rxBuffer+MT8507_FLASH_DATA+UsedLen, FlashBuffer_Index);
        }
        else
        {
            FlashBuffer_Index = 0;
        }
    }
    else if ( bIsAllDataTransmitted == TRUE )
    {
        MT8507_FlashData_Write((uint16_t)FlashBuffer_Index);
        //MT8507_FlashData_Read((uint16_t)FlashBuffer_Index);
        memset(Flash_Buffer, 0, FlashBuffer_Index);
        bIsDataDeliving = FALSE;
    }
#else
        memcpy(Flash_Buffer, m_MT8507_stack.rxBuffer+MT8507_FLASH_DATA, CurrentDataSize);
        MT8507_FlashData_Write((uint16_t)CurrentDataSize);
        MT8507_FlashData_Read((uint16_t)CurrentDataSize);
        memset(Flash_Buffer, 0, FLASH_BUFFER_MAX);
#endif


}

//static uint8_t ui8_tmp = 0;
static void MT8507_FlashData_Write( uint16_t NumByteToWrite )
{
    uint8_t ui8_ret = 0;
    
    uint32_t ui32_idx = 0;
	
    if (bIsDataDeliving == FALSE)
    {
    	SPI_FlashUpdate_mode();
        sFLASH_DisableWriteProtection();
        sFLASH_EraseChip();
        bIsDataDeliving = TRUE;
    }

#if 1
    if ( FlashDestination >=  MCU_IMAGE_END )
        FlashDestination = 0;
#else
    if ( FlashDestination >=  0x100000 )
    {
        FlashDestination += NumByteToWrite;
	return;
    }
 
#endif
#if 0
    if (FlashDestination == (1024*1024))
    {
        TRACE_DEBUG((0, "Start MT8507_FlashData_Write  %d ",FlashDestination));
        for (ui32_idx = 0 ; ui32_idx < 20 ; ui32_idx++)
        {
          ui8_tmp = Flash_Buffer[ui32_idx];
        }
    }
#endif    
    TRACE_DEBUG((0, "Start MT8507_FlashData_Write  %d ",FlashDestination));    /*for code size 244kb */
 #if 0
    for(ui32_idx = 0 ; ui32_idx < NumByteToWrite; ui32_idx++)	
    {
	    ui8_ret = sFLASH_WriteBuffer(Flash_Buffer,&FlashDestination, 1);
            FlashDestination++;
    }
#endif

    ui8_ret = sFLASH_WriteBuffer(Flash_Buffer,&FlashDestination, NumByteToWrite);
#if 0
    for(ui32_idx = 0 ; ui32_idx < NumByteToWrite; ui32_idx+= 1024)	
    {
	    ui8_ret = sFLASH_WriteBuffer(Flash_Buffer,&FlashDestination, 1024);
            FlashDestination+= 1024;
    }
#endif

    while ( ui8_ret == 0)
    {
        TRACE_DEBUG((0, "Success MT8507_FlashData_Write  %d ",FlashDestination));    /*for code size 244kb */
        return;
    }
    TRACE_DEBUG((0, "Err %d MT8507_FlashData_Write  %d ",ui8_ret ,FlashDestination));    /*for code size 244kb */
}

static void MT8507_FlashData_Read( uint16_t NumByteToWrite )
{
#if 0
    tFlashDestination = FlashDestination-NumByteToWrite;
    
    TRACE_DEBUG((0, "Start MT8507_FlashData_Read"));    /*for code size 244kb */

    for ( Buffter_Round = 0; Buffter_Round < 16; Buffter_Round++ )
    {
        sFLASH_ReadBuffer(rFlash_Buffer, tFlashDestination, (uint16_t)1024);
		
        for ( Buffter_Item = 0; Buffter_Item < 1024; Buffter_Item++ )
        {
            if ( Flash_Buffer[((Buffter_Round*1024)+Buffter_Item)] != rFlash_Buffer[Buffter_Item] )
            {
                TRACE_DEBUG((0, "MT8507_FlashData_Read != Write  %d ",((Buffter_Round*1024)+Buffter_Item)));    /*for code size 244kb */
            }
        }
        memset(rFlash_Buffer, 0, 1024);
        Buffter_Item = 0;
        tFlashDestination += 1024;
    }
    Buffter_Round = 0;
#else
    tFlashDestination = FlashDestination -NumByteToWrite;
    sFLASH_ReadBuffer(rFlash_Buffer, tFlashDestination, (uint16_t)NumByteToWrite);
        for ( Buffter_Item = 0; Buffter_Item < NumByteToWrite; Buffter_Item++ )
        {
            if ( Flash_Buffer[Buffter_Item] != rFlash_Buffer[Buffter_Item] )
            {
                TRACE_DEBUG((0, "MT8507_FlashData_Read != Write  %d ",Buffter_Item));    /*for code size 244kb */
            }
        }
        memset(rFlash_Buffer, 0, 1024);
        Buffter_Item = 0;
#endif
}

static uint32 MT8507_GetDataNumberFromStack( uint16 Start, uint8 Length )
{
    uint32 temp = 0;
    uint8 i;

    for ( i = 0 ; i < Length; i++ )
    {
        temp |= m_MT8507_stack.rxBuffer[Start + i] << 8*i;
    }

    return temp;
}

static void MT8507_Initial_8507_stack_Rx( void )
{
    memset(m_MT8507_stack.rxBuffer, 0, RX_BUFFER_MAX);

    m_MT8507_stack.rxPopPtr = 0;
    m_MT8507_stack.rxPushPtr = 0;
}

static void MT8507_Initial_8507_stack_Tx( void )
{
    memset(m_MT8507_stack.txBuffer, 0, TX_BUFFER_MAX);

    m_MT8507_stack.txPopPtr = 0;
    m_MT8507_stack.txPushPtr = 0;
}

static void MT8507_Initial_8507_EventPacket( void )
{
    mMT8507_EventPacket.header= 0;
    mMT8507_EventPacket.Opcode= 0;
    mMT8507_EventPacket.length= 0;
    mMT8507_EventPacket.DataStartPtr= 0;
    mMT8507_EventPacket.event_checksum[0]= 0;
    mMT8507_EventPacket.event_checksum[1]= 0;
}

static bool MT8507_Command_Checksum_Ckeck( uint8 ByteNum )
 {
    uint16 len = 0; 
    uint32 temp = 0;

    uint32  ui4_remain_num = 0;
    uint32  ui4_count_num = 0;
    uint32  ui4_unit_shift = 0, ui4_index = 0, ui4_checksum = 0;

//    total_time_old = VirtualTimer_now() - total_time_old;
//    TRACE_ERROR((0, "Checksum Check Start %d\n", total_time_old));
//    total_time_old = VirtualTimer_now();

    len = mMT8507_EventPacket.length;

    ui4_count_num = (len - ByteNum) / ByteNum;
    ui4_remain_num = (len - ByteNum) % ByteNum;

    for ( ui4_index = 0 ; ui4_index < ui4_count_num ; ui4_index++ )
    {
        ui4_unit_shift = ui4_index * ByteNum;
        memcpy((uint8 *)(&temp), (uint8 *)(m_MT8507_stack.rxBuffer+ui4_unit_shift), ByteNum);
        ui4_checksum = ui4_checksum ^ temp;
        temp = 0;
    }

    if (ui4_remain_num != 0)
    {
        ui4_unit_shift =  ui4_index * ByteNum;
        temp = 0;

        memcpy((uint8 *)(&temp), (uint8 *)(m_MT8507_stack.rxBuffer+ui4_unit_shift), ui4_remain_num);
                
        ui4_checksum = ui4_checksum ^ temp;
    }

    if ( ui4_checksum == MT8507_GetDataNumberFromStack(m_MT8507_stack.rxPopPtr + mMT8507_EventPacket.length - ByteNum, ByteNum) )
    {
//    total_time_old = VirtualTimer_now() - total_time_old;
//    TRACE_ERROR((0, "Checksum Check End(Pass) %d\n", total_time_old));
//    total_time_old = VirtualTimer_now();
        return TRUE;
    }
    else
    {
//    total_time_old = VirtualTimer_now() - total_time_old;
//    TRACE_ERROR((0, "Checksum Check End(Fail) %d\n", total_time_old));
//    total_time_old = VirtualTimer_now();
        return FALSE;
    }
 }

static void MT8507_ChecksunError_handle( void )
{
    byte CmdType = mMT8507_EventPacket.header & 0xF0;

    ReceivedDataNum = 0;
    RxBuffer_Index = 0;
    RxBuffer_Counter = 0;

    memset(RxBuffer, 0, RX_BUFFER_MAX);

    switch( CmdType )
    {
        case 0xA0:
        {
		//MT8507_SendDSPACK(CommandError);
        }
            break;

        case 0xB0:
        {
            if ( mMT8507_EventPacket.header == 0xB2 )
                MT8507_SendReadyToReceiveData(CurrentDataNum+1);
        }
            break;

        default:
    }

    MT8507_Initial_8507_stack_Rx();
    MT8507_Initial_8507_EventPacket();
}

#if 0
static bool MT8507_ITF_EventSet( MT8507_EventPacket *packet ) 
{
    if ( packet == NULL )
        return FALSE;           
    
    if ( packet->header != 0xaa )
        return FALSE;

    #if 0//#ifdef MT8507_DEBUG

    TRACE_DEBUG((0,"MT8507_UITF_EventSet start !! \n"));
    TRACE_DEBUG((0,"event header = 0x%X\n", packet->header));
    TRACE_DEBUG((0,"event length = 0x%X\n", packet->length));

    for( i = 0; i< packet->length; i++ )
    {
        TRACE_DEBUG((0,"event parameters = 0x%X\n", packet->event_params[i] ));
    }

    TRACE_DEBUG((0,"event checksum = 0x%X\n", packet->event_checksum));
    TRACE_DEBUG((0,"MT8507_UITF_EventSet end !! \n"));
    TRACE_DEBUG((0,"\n"));
    #endif 

    switch( packet->event_params[0]) /*event code */
    {
        case 0x00:
            #ifdef MT8507_DEBUG
            TRACE_DEBUG((0,"[MT8507 EVENT CODE] ACK cmd id = 0x%X\n", packet->event_params[1] ));   
            #endif   
            mDebugBT =  packet->event_params[1];
            break;
            
        case 0x17:
            #ifdef MT8507_DEBUG          
            uint8 idex = 0;
            
            TRACE_DEBUG((0,"[MT8507 EVENT CODE] Remote Device Name \n"));

            for( idex = 0; idex < packet->length; idex++ )
            {   
                TRACE_DEBUG((0,"%s \n", packet->event_params[idex+1]));
            }

            TRACE_DEBUG((0,"\n"));
            #endif 
            break;

        case 0x0F: /*report volume value*/
            mIsEventComming = TRUE;
            mMT8507_DeviceStatus.bits.report_volume = 1;
            
            break;

        case 0x18:
            #ifdef MT8507_DEBUG
            TRACE_DEBUG((0,"[MT8507 EVENT CODE] Get UART version \n"));
            TRACE_DEBUG((0,"%d.%d.%d\n", packet->event_params[1], packet->event_params[2], packet->event_params[3]));
            TRACE_DEBUG((0,"\n"));
            #endif 
            break;
            
        case 0x01:
            mIsEventComming = TRUE;
            
            switch( packet->event_params[1])
            {
                case 0x00: /*DEVICE STATE -> POWER OFF */
                    mMT8507_DeviceStatus.byte = 0;

                    #ifdef MT8507_DEBUG
                    TRACE_DEBUG((0,"[MT8507 EVENT CODE] DEVICE STATE -> POWER OFF \n"));
                    #endif
                    break;

                case 0x01:  /* DEVICE STATE -> PAIRING MODE */
                    mMT8507_DeviceStatus.bits.pairing_mode = 1;
                    mMT8507_DeviceStatus.bits.pairing_status = 0;
                    mMT8507_Pairing_Status = MT8507_PAIR_State_START;
                    //mMT8507_DeviceStatus.bits.a2dp_connection_status = 0;

                    #ifdef MT8507_DEBUG
                    TRACE_DEBUG((0,"[MT8507 EVENT CODE] DEVICE STATE -> PAIRING MODE \n"));
                    #endif
                    break;

                case 0x02: /* DEVICE STATE -> POWER ON */
                    mMT8507_DeviceStatus.byte = 0;
                    mMT8507_DeviceStatus.bits.power_status = 1;

                    #ifdef MT8507_DEBUG
                    TRACE_DEBUG((0,"[MT8507 EVENT CODE] DEVICE STATE -> POWER ON \n"));
                    #endif
                    break;

                case 0x03:  /* DEVICE STATE -> PAIRING COMPLETE */
                    mMT8507_DeviceStatus.bits.pairing_status = 1;
                    mMT8507_DeviceStatus.bits.pairing_mode = 0;
                    mMT8507_Pairing_Status = MT8507_PAIR_State_SUCCESS;

                    #ifdef MT8507_DEBUG
                    TRACE_DEBUG((0,"[MT8507 EVENT CODE] DEVICE STATE -> PAIRING COMPLETE \n"));
                    #endif
                    break;

                case 0x04:  /* DEVICE STATE -> PAIRING FAIL */
                    mMT8507_DeviceStatus.bits.pairing_mode = 0;
                    mMT8507_DeviceStatus.bits.pairing_status = 0;
                    mMT8507_DeviceStatus.bits.a2dp_connection_status = 0;
                    mMT8507_Pairing_Status = MT8507_PAIR_State_FAIL;

                    #ifdef MT8507_DEBUG
                    TRACE_DEBUG((0,"[MT8507 EVENT CODE] DEVICE STATE -> PAIRING FAIL \n"));
                    #endif
                    break;

                case 0x05:  /*DEVICE STATE -> HF Connection Complete*/
                    mMT8507_DeviceStatus.bits.hf_connection_status = 1;
                    break;

                case 0x06:  /* DEVICE STATE -> A2DP Connection Complete */
                    mMT8507_DeviceStatus.bits.a2dp_connection_status = 1;
                    break;

                case 0x07:  /*DEVICE STATE -> HF Diconnection */
                    mMT8507_DeviceStatus.bits.hf_connection_status = 0; 
                    break;

                case 0x08:  /*DEVICE STATE -> A2DP Diconnection */
                    mMT8507_DeviceStatus.bits.a2dp_connection_status = 0;  
                    break;

                case 0x09:  /* DEVICE STATE -> SCO Connection Complete */
                    mMT8507_DeviceStatus.bits.sco_connection_status = 1;
                    break;

                case 0x0a:  /*DEVICE STATE -> SCO Disconnection */
                    mMT8507_DeviceStatus.bits.sco_connection_status = 0;
                    break;

                case 0x0b:  /*DEVICE STATE -> AVRCP Connection Complete */
                    mMT8507_DeviceStatus.bits.avrcp_connection_status = 1;
                    break;

                case 0x0c:  /*DEVICE STATE -> AVRCP Disconnection Complete*/
                    mMT8507_DeviceStatus.bits.avrcp_connection_status = 0;
                    break;
            }
            break;

        case 0x0E:
            #ifdef MT8507_DEBUG
            TRACE_DEBUG((0,"[MT8507 EVENT CODE] DEVICE STATE -> Reset to Default \n"));
            #endif
            break;
    
        case 0x1A:
            #ifdef MT8507_DEBUG  
            //TRACE_DEBUG((0,"[MT8507 EVENT CODE] DEVICE STATE -> Play status changed \n"));
            #endif
                if(packet->event_params[13] == 1){
                    if(bIsMT8507_playing == FALSE){
                    bIsMT8507_playing = TRUE;
                    mIsEventComming = TRUE;
                }
            }
            else if(packet->event_params[13] == 2){
                if(bIsMT8507_playing == TRUE){
                    bIsMT8507_playing = FALSE;
                    mIsEventComming = TRUE;
                }
            }
            break;                                   

        default:
            #ifdef MT8507_DEBUG
            TRACE_DEBUG((0,"[MT8507 EVENT UNKNOW CODE] = 0x%X, length = %d\n", packet->event_params[0], packet->length));
            TRACE_DEBUG((0,"0x%X.0x%X.0x%X\n", packet->event_params[1], packet->event_params[2], packet->event_params[3]));
            #endif
            break;
    }

    return TRUE;
}
#endif

static bool MT8507_ITF_EventSet_Paring( void ) 
{
    uint8 i;
    
    if ( m_MT8507_stack.rxPushPtr == 0 )
        return FALSE; /*No data has to pop*/
	
    while( ReceivedDataNum >= 3 )
    {

        if ( mMCUMode != MT8507_Command_Imcomplete )
        {
            MT8507_Command_Wait_Count = 0;

            mMT8507_EventPacket.header = m_MT8507_stack.rxBuffer[m_MT8507_stack.rxPopPtr];

            if ( mMT8507_EventPacket.header == 0xB2 )
            {
                mMT8507_EventPacket.Opcode = 0;
				
                if ( TotalDataSize - 1017 )
                    mMT8507_EventPacket.length = 1024;
                //if ( TotalDataSize - (1017*2) )
                //    mMT8507_EventPacket.length = 1024*2;
                else
                    mMT8507_EventPacket.length = TotalDataSize+7;
            }
            else
            {
                mMT8507_EventPacket.Opcode = m_MT8507_stack.rxBuffer[m_MT8507_stack.rxPopPtr+MT8507_EVENT_COMMAND_TYPE];
                mMT8507_EventPacket.length = m_MT8507_stack.rxBuffer[m_MT8507_stack.rxPopPtr+MT8507_EVENT_LENGTH];
            }
        }
	    
        if ( mMT8507_EventPacket.length > ReceivedDataNum )
        {
            mMCUMode = MT8507_Command_Imcomplete;
            mMT8507_UITF_state = MT8507_STATE_WAIT;
            return FALSE;
        }
            
        if ( mMT8507_EventPacket.header == 0xB2 )
        {
            mMT8507_EventPacket.DataStartPtr = m_MT8507_stack.rxPopPtr+1;
        }
        else
        {
            mMT8507_EventPacket.DataStartPtr = m_MT8507_stack.rxPopPtr+MT8507_EVENT_PARAMS;
        }

    //TRACE_ERROR((0, "Interrup Count %d\n", InterruptCounter));
	//InterruptCounter = 0;

        //Check Checksum
        if ( mMT8507_EventPacket.header == 0xB2 )
        {
            if ( MT8507_Command_Checksum_Ckeck(2) )
            {
                for ( i = 0; i < 2 ; i++ )
                {
                    mMT8507_EventPacket.event_checksum[i] = RxBuffer[m_MT8507_stack.rxPopPtr+mMT8507_EventPacket.length-2+i];
                }
            }
            else
            {
                mMCUMode = MCU_IDLE;
                MT8507_ChecksunError_handle();
                return FALSE;    
            }
        }
        else if ( mMT8507_EventPacket.header != 0xC5 )
        {
            if ( MT8507_Command_Checksum_Ckeck(1) )
            {
                mMT8507_EventPacket.event_checksum[0] = m_MT8507_stack.rxBuffer[m_MT8507_stack.rxPopPtr+mMT8507_EventPacket.length-1];
            }
            else
            {
                mMCUMode = MCU_IDLE;
                MT8507_ChecksunError_handle();
                return FALSE;    
            }
        }
			
        ReceivedDataNum = ReceivedDataNum - mMT8507_EventPacket.length;
		
        if ( ReceivedDataNum == 0 )
        {
            RxBuffer_Index = 0;
            RxBuffer_Counter = 0;
	
            mMT8507_UITF_state = MT8507_STATE_IDLE;
        }

        mMCUMode = MT8507_Command_Decoding;		
        MT8507_Command_Decode(&mMT8507_EventPacket);
	//MT8507_SPIFlash_Update_Process_test();
    }
    return TRUE;    
}

static bool MT8507_ITF_Pop2TranferData( void )
{
    if ( m_MT8507_stack.txPushPtr == 0 )
        return FALSE; /*no data has to transfer to UART.tx*/

//    total_time_old = VirtualTimer_now() - total_time_old;
//    TRACE_ERROR((0, "Tx Star Transmit %d\n", total_time_old));
//    total_time_old = VirtualTimer_now();
    while( m_MT8507_stack.txPopPtr != m_MT8507_stack.txPushPtr )
    {
        MT8507_ITF_SendDatas(m_MT8507_stack.txBuffer, m_MT8507_stack.txPushPtr);
        m_MT8507_stack.txPopPtr +=m_MT8507_stack.txPushPtr;
    }

//    total_time_old = VirtualTimer_now() - total_time_old;
//    TRACE_ERROR((0, "Total delta time %d\n", total_time_old));
//    total_time_old = 0xffffffff;

    /*Set pointer back to buffer start*/
    m_MT8507_stack.txPushPtr = 0;
    m_MT8507_stack.txPopPtr = 0;

    return TRUE;
}

static bool MT8507_ITF_Command_Input( const byte *pBuf, byte len )
{
    byte *pTxBuf;

    if ( m_MT8507_stack.txPushPtr > (TX_BUFFER_MAX-1) )
        return FALSE; /*Buffer is full*/

    //copy *pBuf to tx stack 
    pTxBuf = (m_MT8507_stack.txBuffer+m_MT8507_stack.txPushPtr);
    memcpy( pTxBuf, pBuf, len );
    m_MT8507_stack.txPushPtr += len;

#if 0
        uint8_t    i = 0;
        for( i = 0; i < len; i++ )
        {
            TRACE_DEBUG((0,"UITF CMD IN[%d] = 0x%X \n", i, *(pTxBuf+i) ));
        }
#endif     
    
    return TRUE;
}


bool MT8507_ITF_SendCommand( MT8507_Command *pCMD )
{
    MT8507_Command *ptr;

    __disable_irq();

    if ( bIsMT8507UITF_enable != TRUE )
    {
        TRACE_ERROR((0, "MT8507 interface has not been enable !!\n"));
        __enable_irq();
        return FALSE;
    }

    ptr = pCMD;

    if ( ptr == NULL )
        return FALSE;

    if ( MT8507_ITF_Command_Input(ptr->CMD_IN , ptr->CMD_IN_Length) == FALSE )
    {
        __enable_irq();
        return FALSE;
    }
    
    mMT8507_UITF_state = MT8507_STATE_CMD_IN;

    __enable_irq();
    return TRUE;

}

static bool MT8507_ITF_Push2ReceiveBuf( void )
{
    //uint8 i = 0;
    uint16 rcv_cnt = 0;
    int msg_cnt = 0;
    
    if ( mMT8507_EventPacket.length == 0 )
        rcv_cnt = RxCounter;
    else
        rcv_cnt = mMT8507_EventPacket.length; /*uart_recv_cnt read only, Don't stroe value in this variable; Smith*/

//    if ( (m_MT8507_stack.rxPushPtr+rcv_cnt) > (RX_BUFFER_MAX-1) )
//    {
//        rcv_cnt = RX_BUFFER_MAX - m_MT8507_stack.rxPushPtr;
//    }

    if ( rcv_cnt > 0 )
    {
        msg_cnt = MT8507_ITF_GetData( m_MT8507_stack.rxBuffer, rcv_cnt );
        
        m_MT8507_stack.rxPushPtr = msg_cnt;
        #if 0//#ifdef MT8507_DEBUG

        TRACE_DEBUG((0,"Packet start !! \n"));
        for( i = 0; i < rcv_cnt; i++ )
        {
            TRACE_DEBUG((0,">> m_MT8507_stack.rxBuffer[%d] = 0x%X\n",(m_MT8507_stack.rxPushPtr-rcv_cnt+i), m_MT8507_stack.rxBuffer[m_MT8507_stack.rxPushPtr-rcv_cnt+i] ));
        }
        TRACE_DEBUG((0,"Packet end !! \n"));
        TRACE_DEBUG((0,"\n"));
        #endif /*MT8507_DEBUG*/
        rcv_cnt = 0;
    }
    else
    {
        return FALSE; /*no available message !! */
    }

    return TRUE;

}

void MT8507_write(uint8_t c)
{
    USART_SendData(USART2,c);
    /* Loop until transmit data register is empty */
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
    {}
}

void MT8507_ITF_SendData( uint8 ch )
{
    USART_SendData( USART2, ch );
    
    /* Loop until transmit data register is empty */
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
}

uint16 MT8507_ITF_getReceivedCount( void )
{
    return RxCounter;
}

uint16 MT8507_ITF_GetData( uint8_t *dest, uint16 len )
{
    uint16 RecLen = 0;

    if ( dest == NULL )
        return 0;

    RecLen = len;
/*
    if( ( len > 0 ) && ( RxCounter > len ))
    {
        data_len = len;
    }
    else
    {
        data_len = RxCounter;
    }
*/

    memcpy(dest, RxBuffer, RecLen);
	
	        RxBuffer_Index = RecLen;
	        ReceivedDataNum = RecLen;
	        RxCounter = 0;

/*
    if ( RxBuffer_Index + RecLen < RX_BUFFER_MAX )
    {
//	    for ( i = 0 ; i < len ; i++ )
//	    {
//	        *(dest+i) = RxBuffer[RxBuffer_Index+i];
//	    }
		
    memcpy(dest, RxBuffer+RxBuffer_Index, RecLen);
	
	        RxBuffer_Index += RecLen;
	        ReceivedDataNum += RecLen;
	        RxCounter -= RecLen;
    }
    else
    {
	    for ( i = 0 ; i < RecLen ; i++ )
	    {
	        *(dest+i) = RxBuffer[RxBuffer_Index];

	        RxBuffer_Index++;
	        ReceivedDataNum++;
	        RxCounter--;

	        if ( RxBuffer_Index == RX_BUFFER_MAX )
	            RxBuffer_Index = 0;
	    }
    }
*/
    return RecLen;
}

static void MT8507_ITF_SendDatas( uint8 *src, uint16 length )
{
    uint16 i = 0;
    
    if ( src == NULL )
        return;
    
    for( i = 0; i < length; i++ )
    {
        MT8507_ITF_SendData( *(src+i) );
    }
}

void MT8507_ITF_ProtocolService(void *data)
{
    if (bIsMT8507_ProtocalService == FALSE ) /*if service is disable, this service is not working.*/
        return;

    switch( mMT8507_UITF_state )
    {
        case MT8507_STATE_IDLE:
            if ( ReceivedDataNum )
                mMT8507_UITF_state = MT8507_STATE_GET_EVENT_CODE;

            break;

        case MT8507_STATE_CMD_IN:
            /*Send CMD to OTI3368*/
            MT8507_ITF_Pop2TranferData( ); 

            if ( ReceivedDataNum )
                mMT8507_UITF_state = MT8507_STATE_GET_EVENT_CODE;
            else
                mMT8507_UITF_state = MT8507_STATE_IDLE;//Elvis:
                
            break;

        case MT8507_STATE_GET_EVENT_CODE:
            if ( (MT8507_ITF_Push2ReceiveBuf( ) == TRUE) || (ReceivedDataNum) )
            {
                MT8507_ITF_EventSet_Paring();
            }
            
            break;
                
        case MT8507_STATE_WAIT:
            if ( mMT8507_EventPacket.length > ReceivedDataNum )
            {
                MT8507_Command_Wait_Count++;
                if ( MT8507_Command_Wait_Count >= 5000 )
                    mMT8507_UITF_state = MT8507_STATE_WATE_TIMEOUT;
            }
            else
            {
                MT8507_Command_Wait_Count = 0;
                mMT8507_UITF_state = MT8507_STATE_GET_EVENT_CODE;
            }
            break;
                
        case MT8507_STATE_WATE_TIMEOUT:
            MT8507_Command_Wait_Count = 0;
            ReceivedDataNum = 0;
	    RxBuffer_Index = 0;
	    RxBuffer_Counter = 0;
	    RxCounter = 0;
        
            memset(RxBuffer, 0, RX_BUFFER_MAX);
            MT8507_Initial_8507_stack_Rx();
            MT8507_Initial_8507_EventPacket();

            if ( TotalDataSize == 0xFFFFFFFF )
            {
                MT8507_SendReadyToReceiveData(CurrentDataNum+1);
                mMCUMode = SPI_Flash_Update_Process_Wait_for_Next_Data;
            }

            break;
    }
}

void MT8507_ProtocolService_DispatcherInitialize( void )
{
    mMT8507_DispatcherID = VirtualTimer_register( VIRTUAL_TIMER_MSEC2TICKS(1),                                                 
                                           MT8507_ITF_ProtocolService,
                                           NULL,
                                           VIRTUALTIMER_PERIODIC|
                                           VIRTUALTIMER_NO_ACTIVE
                                          );
}

void MT8507_ProtocolService_DispatcherController( bool enable )
{
    if ( enable )
    {
        VirtualTimer_reschedule( mMT8507_DispatcherID, VIRTUAL_TIMER_MSEC2TICKS(1) );
        bIsMT8507_ProtocalService = TRUE;
        mMT8507_DeviceStatus.byte = 0;
    }
    else
    {
        VirtualTimer_cancel( mMT8507_DispatcherID );
        bIsMT8507_ProtocalService = FALSE;
        mMT8507_DeviceStatus.byte = 0;
    }   
}

