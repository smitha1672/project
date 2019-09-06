#include "stm32f0xx_conf.h"
#include "stm32f0xx.h"
#include "stm320518_eval.h"
#include "STM32F0xx_board.h"
#include "Defs.h"

static bool bIsSPI2_enable = FALSE;

/*******************************************************************************
 *  GPIO
 ******************************************************************************/
/* GPIO */
const GPIO_PhysicalPin PhysicalPins[__IO_MAX] = 
{
    {GPIOB, GPIO_Pin_8, eOUT_PIN},  //PB8    __O_RST_DSP, 
    {GPIOB, GPIO_Pin_12,eOUT_PIN},  //PB12   __O_SCP2_FLASH_CS, 
};

void GPIO_Initialization( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHBPeriphClockCmd( (RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB | 
                            RCC_AHBPeriph_GPIOC | RCC_AHBPeriph_GPIOD | 
                            RCC_AHBPeriph_GPIOF), ENABLE);

    for(uint8_t i=0; i < __IO_MAX; i++ )
    {
        switch( PhysicalPins[i].IO )
        {
            case eOUT_PIN:
                GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
                GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                
                GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
                break;

            case eIN_PIN:
                GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
                break;

            case eOD_PIN:
                GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
                GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
                GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                
                GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
                break;

            default:
                GPIO_InitStructure.GPIO_Pin = PhysicalPins[i].PINx;
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
                GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                GPIO_Init(PhysicalPins[i].PORTx, &GPIO_InitStructure);
                break;
        }
    }
}

void GPIO_Set( VirtualPin pin )
{
    PhysicalPins[pin].PORTx->BSRR= PhysicalPins[pin].PINx;
};

void GPIO_Clr( VirtualPin pin )
{
    PhysicalPins[pin].PORTx->BRR = PhysicalPins[pin].PINx;
}
/*******************************************************************************
 *  UART
 ******************************************************************************/
void ConfigBoot_USART(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* USART resources configuration (Clock, GPIO pins and USART registers) ----*/
    /* USART configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    STM_EVAL_COMInit(COM1, &USART_InitStructure);
} 

/*******************************************************************************
 *  SPI2
 ******************************************************************************/
/* SPI2 Initialization ********************************************************/
void SPI2_LowLevel_initialize( void )
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    _SPI2_CS0_HIGH();

    /*!< Setup SPI Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /*!< Configure _SPI pins: SCK */

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    

    GPIO_InitStructure.GPIO_Pin = _SPI2_SCK_PIN | _SPI2_MISO_PIN | _SPI2_MOSI_PIN;    
    GPIO_Init(_SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(_SPI2_SCK_GPIO_PORT, _SPI2_SCK_SOURCE, GPIO_AF_0);
    GPIO_PinAFConfig(_SPI2_MISO_GPIO_PORT, _SPI2_MISO_SOURCE, GPIO_AF_0);
    GPIO_PinAFConfig(_SPI2_MOSI_GPIO_PORT, _SPI2_MOSI_SOURCE, GPIO_AF_0);    

    /*!< Deselect the SPI: Chip Select high */

    /*!< _SPI Periph clock enable */
    RCC_APB1PeriphClockCmd(_SPI2_CLK, ENABLE);

    /*!< SPI configuration */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    
    /*
        SPI_BaudRatePrescaler_8: SPI SCK is nearly 9Mhz
        SPI_BaudRatePrescaler_4: SPI SCK is nearly 18Mhz
    */

    SPI_InitStructure.SPI_BaudRatePrescaler = 8;//SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

    SPI_Init(_SPI2, &SPI_InitStructure);
    SPI_RxFIFOThresholdConfig(_SPI2, SPI_RxFIFOThreshold_QF);

    /*!< Enable the SPI  */
    SPI_Cmd(_SPI2, ENABLE);
    bIsSPI2_enable = TRUE;
}

void SPI2_LowLevel_Deinitialize( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*!< Disable the _SPI  */
    SPI_Cmd(_SPI2, DISABLE);
    
    GPIO_InitStructure.GPIO_Pin = _SPI2_SCK_PIN | _SPI2_MISO_PIN | _SPI2_MOSI_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_Init(_SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);
    bIsSPI2_enable = FALSE;
}

bool SPI2_LowLevel_isEnable( void )
{
    return bIsSPI2_enable;
}

bool SPI2_write( byte data ) 
{
    bool ret = TRUE;

    if ( bIsSPI2_enable != TRUE )
    {
        return FALSE;
    }

    //! Read data for slave address, this MISO byte is unavailable; Smith@{
    /*!< Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(_SPI2, SPI_I2S_FLAG_TXE) == RESET);

    /*!< Send byte through the SPI1 peripheral */
    SPI_SendData8(_SPI2, data);

    /*!< Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(_SPI2, SPI_I2S_FLAG_RXNE) == RESET);

    /*rxBuff = */
    SPI_ReceiveData8(_SPI2);

    // TRACE_DEBUG((0, "SPI data write = 0x%X", data));

    return ret;
}

bool SPI2_read( byte *data ) 
{
    bool ret = TRUE;
    byte *ptr;
    uint8_t rxBuff;

    if ( bIsSPI2_enable != TRUE )
    {
        return FALSE;
    }
    
    ptr = data;

    /*!< Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(_SPI2, SPI_I2S_FLAG_TXE) == RESET);

    /*!< Send byte through the SPI1 peripheral */
    SPI_SendData8(_SPI2, 0xFF);

    /*!< Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(_SPI2, SPI_I2S_FLAG_RXNE) == RESET);

    rxBuff = SPI_ReceiveData8(_SPI2);
    
    //TRACE_DEBUG((0, "SPI data read = 0x%X", rxBuff));

    *ptr = rxBuff;

    return ret;
}

bool SPI2_selectChip( unsigned char chip )
{
    if ( chip == 0)
    {
        _SPI2_CS0_LOW();
    }
    
    return TRUE;
}

bool SPI2_unselectChip( unsigned char chip )
{
    if ( chip == 0)
    {
        _SPI2_CS0_HIGH();
    }

    return TRUE;
}

int16 SPI2_writeBuffer(byte *data, uint16 length, bool LittelEndian)
{
    int16 ret = SPI_RET_ERROR_LEN;
    byte *ptr;
    
    if(LittelEndian)
        ptr = data + length - 1;
    else
        ptr = data;
    
    ret = 0;

    while(length > 0)
    {  
        if(SPI2_write(*ptr) == TRUE)
        {
            ret++;
        }
        else
        {    
            return ret;
        }

        if(LittelEndian)
            ptr--;
        else
            ptr++;
        
        length--;
    }    
    return ret; 
}

int16 SPI2_readBuffer(byte *data, uint16 length, bool LittelEndian)
{
    int16 ret = SPI_RET_ERROR_LEN;
    byte rxBuff;
    byte *ptr;

    if(LittelEndian)
        ptr = data + length - 1;
    else
        ptr = data;
    
    ret = 0;

    while(length > 0)
    {
        if(SPI2_read(&rxBuff) == TRUE)
        {
            *ptr = rxBuff;
            ret++;
        }
        else
        {
            //ASSERT_BOOL(TRUE);
            return ret;
        }
        
        if(LittelEndian)
            ptr--;
        else
            ptr++;
        
        length--;
    }

    return ret;
}
uint8_t RAM_Buf[256] = {0xff};
uint8_t RAM_Buf2[256] = {0xF2,0x13,0x1F,0x15};
//! < Private Application_________________________________________________@{
bool FlashDriver_test(void)  
{
    bool ret = TRUE;

    uint32_t address = 0;

    sFLASH_DisableWriteProtection();
    //TRACE_DEBUG((0,"Flash status = %X",sFLASH_ReadStatusRegister()));
    sFLASH_EraseChip();    
    sFLASH_WritePage(RAM_Buf2,address,256);
    sFLASH_ReadBuffer (RAM_Buf,address,256);

    return ret;
}

