#include "BSP.h"
#include "stm32f0xx_conf.h"
#include "STM32F0xx_board.h"
#include "stm32f0xx.h"
#include "STM32F0xx_board.h"


#define configINTERNAL_RTC 0
#define configSPI_1 1
#define configSPI_2 1

#define APPLICATION_ADDRESS     (uint32_t)0x08003000
#if   (defined ( __CC_ARM ))
  __IO uint32_t VectorTable[48] __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
  __no_init __IO uint32_t VectorTable[48];
#elif defined   (  __GNUC__  )
  __IO uint32_t VectorTable[48] __attribute__((section(".RAMVectorTable")));
#elif defined ( __TASKING__ )
  __IO uint32_t VectorTable[48] __at(0x20000000);
#endif


static bool bIsSPI1_enable = FALSE;
static bool bIsSPI2_enable = FALSE;

#if defined ( FREE_RTOS )
static xSemaphoreHandle _SPI_SEMA = NULL;
#endif 


/* GPIO */
const GPIO_PhysicalPin PhysicalPins[__IO_MAX] = 
{
    {GPIOB, GPIO_Pin_2, eOUT_PIN},  //PB2    __O_EN_1V,
    {GPIOB, GPIO_Pin_8, eOUT_PIN},  //PB8    __O_RST_DSP,
    {GPIOB, GPIO_Pin_0, eIN_PIN},   //PB0    __I_SCP1_IRQ,  
    {GPIOA, GPIO_Pin_4, eOUT_PIN},  //PA4    __O_SPI1_DSP_CS,
    {GPIOA, GPIO_Pin_1, eOUT_PIN},  //PA1    __O_FLASH_CS,  
    {GPIOA, GPIO_Pin_5, eOUT_PIN},  //PA5    __O_SPI1_SCK, 
    {GPIOB, GPIO_Pin_1, eIN_PIN},   //PB1    __I_SCP1_BUSY,     
    {GPIOB, GPIO_Pin_12,eOUT_PIN},  //PB12   __O_SCP2_FLASH_CS, 
    {GPIOA, GPIO_Pin_11,eOUT_PIN},  //PA11   __O_MCU_SLAVE_MODE,
    {GPIOA, GPIO_Pin_12,eOUT_PIN},  //PA12   __O_MCU_MASTER_MODE,
};


const GPIO_PhysicalPin SPI1Pins[__SPI_IO_MAX] = 
{
    {GPIOB, GPIO_Pin_8, eIN_PIN},       /*PB8 __O_DSP_RES,  */     
    {GPIOA, GPIO_Pin_4, eIN_PIN},       /*PA4 __O_SPI1_DSP_CS, */    
    {GPIOA, GPIO_Pin_5, eIN_PIN},       /*PA5 __O_SPI1_SCK, */    
    {GPIOA, GPIO_Pin_6, eIN_PIN},       /*PA6 __O_SPI1_MISO,*/
    {GPIOA, GPIO_Pin_7, eIN_PIN},       /*PA7 __O_SPI1_MOSI,*/
};

const GPIO_PhysicalPin SPI2Pins[__SPI_IO_MAX] = 
{    
    {GPIOB, GPIO_Pin_12, eIN_PIN},       /*PB12 __O_SCP2_FLASH_CS, */         
    {GPIOB, GPIO_Pin_13, eIN_PIN},       /*PB13 __O_SPI2_SCK, */    
    {GPIOB, GPIO_Pin_14, eIN_PIN},       /*PB14 __O_SPI2_MISO,*/
    {GPIOB, GPIO_Pin_15, eIN_PIN},       /*PB15 __O_SPI2_MOSI,*/
};


static void GPIO_Initialization( void );


static void GPIO_Set( VirtualPin pin );


static void GPIO_Clr( VirtualPin pin );


static uint8_t GPIO_Read_InBit( VirtualPin pin );


static void STM32f0xx_board( void );


static void SPI1_LowLevel_initialize( void );


static void SPI1_LowLevel_release_SPI(void);


static bool SPI1_LowLevel_isEnable( void );


static bool SPI1_write( const byte data );


static bool SPI1_read( byte *data );


static int16 SPI1_writeBuffer(const byte *data, uint16 length, bool LittelEndian);


static int16 SPI1_readBuffer( byte *data, uint16 length, bool LittelEndian );


static uint8_t SPI1_LowLevel_ReadDummy( void );


static void SPI2_LowLevel_initialize( void );


static void SPI2_LowLevel_release_SPI (void );


static bool SPI2_write( byte data );


static bool SPI2_read( byte * data );


static int16 SPI2_writeBuffer(byte *data, uint16 length, bool LittelEndian);


static int16 SPI2_readBuffer(byte *data, uint16 length, bool LittelEndian);


static bool SPI_selectChip( void );


static bool SPI_unselectChip( void );


static bool SPI1_mutex_lock( void );


static bool SPI1_mutex_unlock( void );



const BSP_GPIO_OBJECT bsp_gpio_obj = 
{
    GPIO_Initialization,
    GPIO_Set,
    GPIO_Clr,
    GPIO_Read_InBit,
};
const BSP_GPIO_OBJECT *pBSP_GPIO_OBJECT = &bsp_gpio_obj;

const BSP_SPI_OBJECT bsp_spi_obj = 
{
    SPI1_LowLevel_initialize,
    SPI1_LowLevel_release_SPI,
    SPI1_LowLevel_isEnable,
    SPI1_write,
    SPI1_read,
    SPI1_LowLevel_ReadDummy,
    SPI1_writeBuffer,
    SPI1_readBuffer,
    SPI1_mutex_lock,
    SPI1_mutex_unlock,
    SPI2_LowLevel_initialize,
    SPI2_LowLevel_release_SPI,
    SPI2_write,
    SPI2_read,
    SPI2_writeBuffer,
    SPI2_readBuffer,
    SPI_selectChip,
    SPI_unselectChip,
};
const BSP_SPI_OBJECT *pBSP_SPI_OBJECT = &bsp_spi_obj;

const BSP_BOARD_OBJECT bsp_board_obj = 
{
    STM32f0xx_board
};
const BSP_BOARD_OBJECT *pBSP_BOARD_ObjCtrl = &bsp_board_obj;



//_____________________________________________________________________________
static void RTC_backup__Initialization(void)
{
#if ( configINTERNAL_RTC == 1 )
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR , ENABLE); 

    /* Enable write access to Backup domain */
    PWR_BackupAccessCmd(ENABLE);
#endif

}
/*******************************************************************************
 * GPIO
 ******************************************************************************/
static void GPIO_Set( VirtualPin pin )
{
    PhysicalPins[pin].PORTx->BSRR= PhysicalPins[pin].PINx;
};

static void GPIO_Clr( VirtualPin pin )
{
    PhysicalPins[pin].PORTx->BRR = PhysicalPins[pin].PINx;
}

static uint8_t GPIO_Read_InBit( VirtualPin pin )
{
    uint8_t bitstatus = 0x00;

    if( (PhysicalPins[pin].PORTx->IDR & PhysicalPins[pin].PINx) != (uint32_t)Bit_RESET )
    {
        bitstatus = (uint8_t)Bit_SET;
    }
    else
    {
        bitstatus = (uint8_t)Bit_RESET;
    }

    return bitstatus;
}

static void GPIO_Initialization( void )
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


static void SPI1_GPIOLowLevel_DeConfiguration( void )
{
#if ( configSPI_1 == 1 )
    GPIO_InitTypeDef GPIO_InitStructure;

    for(uint8_t i=0; i < __SPI_IO_MAX; i++ )
    {
        GPIO_InitStructure.GPIO_Pin |= SPI1Pins[i].PINx;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(SPI1Pins[i].PORTx, &GPIO_InitStructure);
    }
    
    RCC_AHBPeriphClockCmd( (RCC_AHBPeriph_GPIOA| RCC_AHBPeriph_GPIOB| 
                            RCC_AHBPeriph_GPIOC| RCC_AHBPeriph_GPIOD), 
                            ENABLE);
#endif    

}


static void SPI2_GPIOLowLevel_DeConfiguration( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    for(uint8_t i=0; i < __SPI2_IO_MAX; i++ )
    {
        GPIO_InitStructure.GPIO_Pin |= SPI2Pins[i].PINx;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(SPI2Pins[i].PORTx, &GPIO_InitStructure);
    }
    
    RCC_AHBPeriphClockCmd( (RCC_AHBPeriph_GPIOA| RCC_AHBPeriph_GPIOB| 
                            RCC_AHBPeriph_GPIOC| RCC_AHBPeriph_GPIOD), 
                            ENABLE);

}

/*******************************************************************************
 *  Timmer
 ******************************************************************************/
void Timers_Initialization( void )
{
    uint16_t PrescalerValue = 0;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    /*Enable TIM3 interrupt in every 50us*/
    NVIC_InitTypeDef NVIC_InitStructure;

    /* TIM3 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* Enable the TIM3 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Compute the prescaler value */
    PrescalerValue =(uint16_t) (SystemCoreClock  / 200000) - 1;

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period = 9;
    TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* TIM Interrupts enable */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    /* TIM3 enable counter */
    TIM_Cmd(TIM2, ENABLE);

}

/*******************************************************************************
 *  Uart 1
 ******************************************************************************/
void Uart1_Initialization(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;     
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USARTx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
    */
    RCC_AHBPeriphClockCmd(USART1_TX_GPIO_CLK|USART1_RX_GPIO_CLK, ENABLE);

    RCC_APB2PeriphClockCmd(USART1_CLK,ENABLE);

    GPIO_PinAFConfig(USART1_TX_GPIO_PORT, USART1_TX_SOURCE, USART1_TX_AF);
    GPIO_PinAFConfig(USART1_RX_GPIO_PORT, USART1_RX_SOURCE, USART1_RX_AF);

    /* Configure USART1 pins:  Rx and Tx ----------------------------*/
    GPIO_InitStructure.GPIO_Pin =  USART1_TX_PIN | USART1_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1_EN, &USART_InitStructure);

    USART_Cmd(USART1_EN,ENABLE);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void Uart1_Deinitialization(void)
{
    USART_DeInit(USART1_EN);
}

/*******************************************************************************
 *  Uart 2
 ******************************************************************************/
static void Uart2_Initialization(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USARTx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
    */
  
    RCC_AHBPeriphClockCmd(USART2_TX_GPIO_CLK|USART2_RX_GPIO_CLK, ENABLE);

    RCC_APB1PeriphClockCmd(USART2_CLK,ENABLE);

    GPIO_PinAFConfig(USART2_TX_GPIO_PORT, USART2_TX_SOURCE, USART2_TX_AF);
    GPIO_PinAFConfig(USART2_RX_GPIO_PORT, USART2_RX_SOURCE, USART2_RX_AF);

    /* Configure USART1 pins:  Rx and Tx ----------------------------*/
    GPIO_InitStructure.GPIO_Pin =  USART2_TX_PIN | USART2_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200*8;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2_EN, &USART_InitStructure);

    USART_Cmd(USART2_EN,ENABLE);

    /* Enable the USARTx Receive interrupt: this interrupt is generated when the
    USARTx receive data register is not empty */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void Uart2_Deinitialize(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = USART2_TX_PIN | USART2_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
 *  SPI
 ******************************************************************************/
/* SPI Initialization *********************************************************/
#define _SPI1_CS0_LOW()              GPIO_Clr( __O_SPI1_DSP_CS )
#define _SPI1_CS0_HIGH()             GPIO_Set( __O_SPI1_DSP_CS )

#if defined ( FREE_RTOS )
static bool _SPI_MUTEX_TAKE( void )
{
#if 1
	if ( _SPI_SEMA == NULL )
		return FALSE;

	if ( xSemaphoreTake( _SPI_SEMA, portMAX_DELAY) != pdTRUE )
	{
	    TRACE_DEBUG((0, "DDDDD_SPI TAKE BUSY !! "));
		return FALSE;
	}

        //TRACE_DEBUG((0, "DDDDD_SPI TAKE OK !! "));
        //i_counter++;
	return TRUE;	
#else
        return TRUE;    
#endif
}
#endif 

#if defined ( FREE_RTOS )
static bool _SPI_MUTEX_GIVE( void )
{
#if 1
	if ( _SPI_SEMA == NULL )
		return FALSE;

	if ( xSemaphoreGive( _SPI_SEMA ) != pdTRUE )
	{
	    TRACE_DEBUG((0, "DDDDD_SPI GIVE BUSY !! "));
		return FALSE;	
	}

        //TRACE_DEBUG((0, "DDDDD_SPI GIVE OK !! "));
        //i_counter--;
	return TRUE;
#endif    
}
#endif 

#if defined ( FREE_RTOS )
static bool _SPI_MUTEX_CREATE( void )
{
    if ( _SPI_SEMA == NULL )
    {
        _SPI_SEMA = xSemaphoreCreateMutex();
    }

    if (_SPI_SEMA == NULL)
    {
        TRACE_DEBUG((0, "semaphore create fail !"));
        return FALSE;
    }    

    return TRUE;
}
#endif 


#if defined ( FREE_RTOS )
static void _SPI_MUTEX_RELEASE( void )
{
    if ( _SPI_SEMA )
    {
        vSemaphoreDelete(_SPI_SEMA);
        _SPI_SEMA = NULL;
    }
}
#endif 


#if defined ( FREE_RTOS )
static bool SPI1_mutex_lock( void )
{
    bool ret_val = FALSE;
    
    //TRACE_ERROR((0, "SPI enter --------------- "));
    if ( _SPI_MUTEX_TAKE() == TRUE)
    {
        ret_val = TRUE;
        return ret_val;
    }

    return ret_val;
}
#endif 

#if defined ( FREE_RTOS )
static bool SPI1_mutex_unlock( void )
{
    bool ret_val = FALSE;

    //TRACE_ERROR((0, "SPI_unselectChip loop --------------- "));
    if ( _SPI_MUTEX_GIVE() == TRUE)
    {
        ret_val = TRUE;
    }
    else
    {
        ret_val = FALSE;
    }

    //TRACE_ERROR((0, "SPI outer --------------- %d",i_counter));
    return ret_val;
}
#endif 


static void SPI1_LowLevel_initialize( void )
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    _SPI1_CS0_HIGH();

    /*!< Setup SPI Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    /*!< Configure _SPI pins: SCK */

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    

    GPIO_InitStructure.GPIO_Pin = _SPI_SCK_PIN | _SPI_MISO_PIN | _SPI_MOSI_PIN;    
    GPIO_Init(_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

    GPIO_PinAFConfig(_SPI_SCK_GPIO_PORT, _SPI_SCK_SOURCE, GPIO_AF_0);
    GPIO_PinAFConfig(_SPI_MISO_GPIO_PORT, _SPI_MISO_SOURCE, GPIO_AF_0);
    GPIO_PinAFConfig(_SPI_MOSI_GPIO_PORT, _SPI_MOSI_SOURCE, GPIO_AF_0);    

    /*!< Deselect the SPI: Chip Select high */

    /*!< _SPI Periph clock enable */
    RCC_APB2PeriphClockCmd(_SPI_CLK, ENABLE);

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

    SPI_Init(_SPI, &SPI_InitStructure);
    SPI_RxFIFOThresholdConfig(_SPI, SPI_RxFIFOThreshold_QF);

    /*!< Enable the SPI  */
    SPI_Cmd(_SPI, ENABLE);
    bIsSPI1_enable = TRUE;

    _SPI_MUTEX_CREATE( );
}


static void SPI1_LowLevel_Deinitialize( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*!< Disable the _SPI  */
    SPI_Cmd(_SPI, DISABLE);
    
    GPIO_InitStructure.GPIO_Pin = _SPI_SCK_PIN | _SPI_MISO_PIN | _SPI_MOSI_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
    GPIO_Init(_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
    bIsSPI1_enable = FALSE;

    _SPI_MUTEX_RELEASE( );
}

static uint8_t SPI1_LowLevel_ReadDummy( void )
{
    uint8_t data = 0;

    data = SPI_ReceiveData8(_SPI);
    return data;
}

static bool SPI1_LowLevel_isEnable( void )
{
    return bIsSPI1_enable;
}

static void SPI1_LowLevel_release_SPI(void)
{
    SPI1_LowLevel_Deinitialize();
    SPI1_GPIOLowLevel_DeConfiguration();
}

static bool SPI1_write( const byte data ) 
{
    bool ret = TRUE;

    if ( bIsSPI1_enable != TRUE )
    {
        TRACE_ERROR((0, "SPI interface has not been enable !!"));
        return FALSE;
    }

    //! Read data for slave address, this MISO byte is unavailable; Smith@{
    /*!< Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_TXE) == RESET);

    /*!< Send byte through the SPI1 peripheral */
    SPI_SendData8(_SPI, data);

    /*!< Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_RXNE) == RESET);

    /*rxBuff = */
    SPI_ReceiveData8(_SPI);

    // TRACE_DEBUG((0, "SPI data write = 0x%X", data));

    return ret;
}

static bool SPI1_read( byte *data ) 
{
    bool ret = TRUE;
    byte *ptr;
    uint8_t rxBuff;

    if ( bIsSPI1_enable != TRUE )
    {
        TRACE_ERROR((0, "SPI interface has not been enable !!"));
        return FALSE;
    }
    
    ptr = data;

    /*!< Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_TXE) == RESET);

    /*!< Send byte through the SPI1 peripheral */
    SPI_SendData8(_SPI, 0xFF);

    /*!< Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_RXNE) == RESET);

    rxBuff = SPI_ReceiveData8(_SPI);
    
    //TRACE_DEBUG((0, "SPI data read = 0x%X", rxBuff));

    *ptr = rxBuff;

    return ret;
}

static bool SPI_selectChip( void )
{
    _SPI1_CS0_LOW();
    return TRUE;
}

static bool SPI_unselectChip( void )
{
    _SPI1_CS0_HIGH();
    return TRUE;
}

static int16 SPI1_writeBuffer(const byte *data, uint16 length, bool LittelEndian)
{
    int16 ret = SPI_RET_ERROR_LEN;
    const byte *ptr;
    
    if(LittelEndian)
        ptr = data + length - 1;
    else
        ptr = data;
    
    ret = 0;

    while(length > 0)
    {  
        if(SPI1_write(*ptr) == TRUE)
        {
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


static int16 SPI1_readBuffer( byte *data, uint16 length, bool LittelEndian )
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
        if(SPI1_read(&rxBuff) == TRUE)
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


static void SPI2_LowLevel_initialize( void )
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


static void SPI2_LowLevel_Deinitialize( void )
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


static bool SPI2_LowLevel_isEnable( void )
{
    return bIsSPI2_enable;
}


static void SPI2_LowLevel_release_SPI(void)
{
    SPI2_LowLevel_Deinitialize();
    SPI2_GPIOLowLevel_DeConfiguration();
}


static bool SPI2_write( byte data ) 
{
    bool ret = TRUE;

    if ( bIsSPI2_enable != TRUE )
    {
        TRACE_ERROR((0, "SPI interface has not been enable !!"));
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


static bool SPI2_read( byte *data ) 
{
    bool ret = TRUE;
    byte *ptr;
    uint8_t rxBuff;

    if ( bIsSPI2_enable != TRUE )
    {
        TRACE_ERROR((0, "SPI interface has not been enable !!"));
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


static bool SPI2_selectChip( unsigned char chip )
{
    if ( chip == 0)
    {
        _SPI2_CS0_LOW();
    }
    
    return TRUE;
}


static bool SPI2_unselectChip( unsigned char chip )
{
    if ( chip == 0)
    {
        _SPI2_CS0_HIGH();
    }

    return TRUE;
}

static int16 SPI2_writeBuffer(byte *data, uint16 length, bool LittelEndian)
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


static int16 SPI2_readBuffer(byte *data, uint16 length, bool LittelEndian)
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


/*******************************************************************************
 *  SPI common
 ******************************************************************************/
void SPI_CirrusTool_mode(void)
{
    TRACE_DEBUG((0, "DSP SPI release  !!"));
    GPIO_Initialization();       
    SPI_LowLevel_release_SPI();
    SPI2_LowLevel_release_SPI();
    GPIO_Clr(__O_MCU_MASTER_MODE);
    GPIO_Set(__O_MCU_SLAVE_MODE); /* Set slave mode */
    GPIO_Clr(__O_RST_DSP);    
    //VirtualTimer_sleep_100us(1); /* require 1us */
    GPIO_Set(__O_RST_DSP); 

}

void SPI_FlashUpdate_mode(void)
{
    TRACE_DEBUG((0, "DSP Flash update mode  !!"));
    GPIO_Initialization();       
    SPI1_LowLevel_release_SPI();
    SPI2_LowLevel_initialize();
    GPIO_Clr(__O_MCU_MASTER_MODE);
    GPIO_Set(__O_MCU_SLAVE_MODE); /* Set slave mode */
    GPIO_Clr(__O_RST_DSP);    

}

static void SPI_MasterBoot_mode(void)
{
    TRACE_DEBUG((0, "DSP Master boot  !!"));
    GPIO_Initialization();
    SPI1_LowLevel_initialize();
    SPI2_LowLevel_release_SPI();
    GPIO_Set(__O_MCU_MASTER_MODE); /* Set master mode */
    GPIO_Clr(__O_MCU_SLAVE_MODE);  
    GPIO_Set(__O_EN_1V);       
    GPIO_Clr(__O_RST_DSP);
    //VirtualTimer_sleep_100us(1); /* require 1us */
    GPIO_Set(__O_RST_DSP); 
}

static void SetVectorTable(  void )
{
    uint32_t i = 0;  
    /* Relocate by software the vector table to the internal SRAM at 0x20000000 ***/  

    /* Copy the vector table from the Flash (mapped at the base of the application
    load address 0x08003000) to the base address of the SRAM at 0x20000000. */
    for(i = 0; i < 48; i++)
    {
        VectorTable[i] = *(__IO uint32_t*)(APPLICATION_ADDRESS + (i<<2));
    }

    /* Enable the SYSCFG peripheral clock*/
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE); 
    /* Remap SRAM at 0x00000000 */
    SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);
}


static void STM32f0xx_board(void)
{
    SetVectorTable( );
    GPIO_Initialization();
    Uart1_Initialization();
    Uart2_Initialization();
    SPI1_LowLevel_initialize( );
    SPI2_LowLevel_initialize( );
    Debug_initialize();  
    RTC_backup__Initialization();
}

