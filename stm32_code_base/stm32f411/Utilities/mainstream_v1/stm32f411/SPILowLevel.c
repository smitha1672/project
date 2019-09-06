#include "Debug.h"

#include "GPIOMiddleLevel.h"
#include "SPILowLevel.h"

#if defined ( FREE_RTOS )
#include "FreeRTOS.h"
#include "semphr.h"
#endif 

/*_____________________________________________________________________________________________*/
#define SPIx                           SPI4
#define SPIx_CLK                       RCC_APB2Periph_SPI4
#define SPIx_CLK_INIT                  RCC_APB2PeriphClockCmd
//#define SPIx_IRQn                      SPI4_IRQn
//#define SPIx_IRQHANDLER                SPI2_IRQHandler

#define SPIx_SCK_PIN                   GPIO_Pin_2
#define SPIx_SCK_GPIO_PORT             GPIOE
#define SPIx_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOE
#define SPIx_SCK_SOURCE                GPIO_PinSource2
#define SPIx_SCK_AF                    GPIO_AF_SPI4

#define SPIx_MISO_PIN                  GPIO_Pin_5
#define SPIx_MISO_GPIO_PORT            GPIOE
#define SPIx_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define SPIx_MISO_SOURCE               GPIO_PinSource5
#define SPIx_MISO_AF                   GPIO_AF_SPI4

#define SPIx_MOSI_PIN                  GPIO_Pin_6
#define SPIx_MOSI_GPIO_PORT            GPIOE
#define SPIx_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define SPIx_MOSI_SOURCE               GPIO_PinSource6
#define SPIx_MOSI_AF                   GPIO_AF_SPI4

#define spi_write_dummy()	SPI_I2S_SendData(_SPI, 0xFF)
#define _SPI_CS0_LOW()		GPIOMiddleLevel_Clr( __O_SPI1_NSS ) 
#define _SPI_CS0_HIGH()    	GPIOMiddleLevel_Set( __O_SPI1_NSS )
#define _SPI_CS1_LOW()      GPIOMiddleLevel_Clr( __O_EXT_FLASH_CS )
#define _SPI_CS1_HIGH()     GPIOMiddleLevel_Set( __O_EXT_FLASH_CS )
/*_________________________________________________________________________________________________________*/
// Private Variable
SPI_InitTypeDef  SPI_InitStructure;
static bool bIsSPI_enable = FALSE;

#if defined ( FREE_RTOS )
static xSemaphoreHandle _SPI_SEMA = NULL;
#endif 

//static int i_counter = 0;
/*_____________________________________________________________________________________________*/
extern void SPI_GPIOLowLevel_DeConfiguration( void );
/*STM32*/
static void __STM32_SPI_DeConfiguration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Disable the _SPI  */
  SPI_Cmd(SPIx, DISABLE);
  
  /*!< Configure _SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure _SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MISO_PIN;
  GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
  GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);

}

static void __STM32_SPI_Configuration( void )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //DMA_InitTypeDef DMA_InitStructure;

    /* Peripheral Clock Enable -------------------------------------------------*/
    /* Enable the SPI clock */
    SPIx_CLK_INIT(SPIx_CLK, ENABLE);

    /* Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);
    
    /* Enable DMA clock */
    //RCC_AHB1PeriphClockCmd(SPIx_DMA_CLK, ENABLE);

    /* SPI GPIO Configuration --------------------------------------------------*/
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    /* SPI SCK pin configuration */
    GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
    GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);


    /* SPI  MOSI pin configuration */
    GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
    GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /* SPI  MISO pin configuration */
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin =  SPIx_MISO_PIN;
    GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);  

    /* Connect SPI pins to AF5 */  
    GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
    GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);    
    GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);

    /*!< Deselect the SPI: Chip Select high */
	_SPI_CS0_HIGH();
    _SPI_CS1_HIGH();

    /* SPI configuration -------------------------------------------------------*/
    SPI_I2S_DeInit(SPIx);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    //SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

#if 0
    /* DMA configuration -------------------------------------------------------*/
    /* Deinitialize DMA Streams */
    DMA_DeInit(SPIx_TX_DMA_STREAM);
    DMA_DeInit(SPIx_RX_DMA_STREAM);

    /* Configure DMA Initialization Structure */
    DMA_InitStructure.DMA_BufferSize = BUFFERSIZE ;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SPIx->DR)) ;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    /* Configure TX DMA */
    DMA_InitStructure.DMA_Channel = SPIx_TX_DMA_CHANNEL ;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aTxBuffer ;
    DMA_Init(SPIx_TX_DMA_STREAM, &DMA_InitStructure);
    /* Configure RX DMA */
    DMA_InitStructure.DMA_Channel = SPIx_RX_DMA_CHANNEL ;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aRxBuffer ; 
    DMA_Init(SPIx_RX_DMA_STREAM, &DMA_InitStructure);
#endif    

    SPI_Init(SPIx, &SPI_InitStructure);

    /* Enable the SPI peripheral */
    SPI_Cmd(SPIx, ENABLE);

}

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
        return FALSE;

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


/*_____________________________________________________________________________________________*/
void SPILowLevel_initialize( void )
{
	__STM32_SPI_Configuration( );

#if defined ( FREE_RTOS )	
	_SPI_MUTEX_CREATE();
#endif 
        
	bIsSPI_enable = TRUE;
}

void SPILowLevel_Deinitialize( void )
{
	__STM32_SPI_DeConfiguration( );

#if defined ( FREE_RTOS )	
	_SPI_MUTEX_RELEASE();
#endif 

	bIsSPI_enable = FALSE;
}

bool SPILowLevel_isEnable( void )
{
	return bIsSPI_enable;
}

void SPILowLevel_release_SPI(void)
{
	SPILowLevel_Deinitialize();
	SPI_GPIOLowLevel_DeConfiguration();
	TRACE_DEBUG((0, "SPI release  !!"));
}

bool SPI_write( byte data ) 
{
	bool ret = TRUE;

	if ( bIsSPI_enable != TRUE )
	{
		return FALSE;
	}

	//! Read data for slave address, this MISO byte is unavailable; Smith@{
	/*!< Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/*!< Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(_SPI, data);

	return ret;
	
}

bool SPI_read( byte *data ) 
{
	bool ret = TRUE;
	byte *ptr;
	uint8_t rxBuff;

	if ( bIsSPI_enable != TRUE )
	{
		return FALSE;
	}
	
	ptr = data;

	/*!< Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/*!< Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(_SPI, 0xFF);	/*Write SPI dummy byte*/

	/*!< Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	rxBuff = (uint8_t)(SPI_I2S_ReceiveData( _SPI )& 0x00FF );
	

	*ptr = rxBuff;

	return ret;
}

bool SPI_selectChip( unsigned char chip )
{
    bool ret_val = FALSE;
	if ( chip == 0)
	{
		_SPI_CS0_LOW();
        _SPI_CS1_HIGH();
            ret_val = TRUE;
	}
    else if ( chip == 1 )
    {
        _SPI_CS1_LOW();
        _SPI_CS0_HIGH();
            ret_val = TRUE;
    }
    else
    {
            ret_val = FALSE;
    }
	
    return ret_val;
}

bool SPI_unselectChip( unsigned char chip )
{
    bool ret_val = FALSE;

	if ( chip == 0)
	{
		_SPI_CS0_HIGH();
        ret_val = TRUE;
	}
    else if ( chip == 1 )
    {
        _SPI_CS1_HIGH();
        ret_val = TRUE;
    }
    else
    {
        ret_val = FALSE;
    }

    return ret_val;
}

#if defined ( FREE_RTOS )
bool SPI_mutex_lock( )
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
bool SPI_mutex_unlock( )
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

int16 SPI_writeBuffer(const byte *data, uint16 length, bool LittelEndian)
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
		if(SPI_write(*ptr) == TRUE)
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

int16 SPI_readBuffer(byte *data, uint16 length, bool LittelEndian)
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
		if(SPI_read(&rxBuff) == TRUE)
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
