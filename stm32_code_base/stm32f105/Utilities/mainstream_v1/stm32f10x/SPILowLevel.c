#include "SOUND_BAR_V1_PinDefinition.h"
#include "Debug.h"

#include "GPIOMiddleLevel.h"
#include "SPILowLevel.h"

#if defined ( FREE_RTOS )
#include "FreeRTOS.h"
#include "semphr.h"
#endif 

/*_____________________________________________________________________________________________*/
#define spi_write_dummy()	SPI_I2S_SendData(_SPI, 0xFF)

#if 1
#define _SPI_CS0_LOW()		GPIOMiddleLevel_Clr( __O_SPI1_NSS ) //GPIO_ResetBits(_SPI_CS0_GPIO_PORT, _SPI_CS0_PIN)
#define _SPI_CS0_HIGH()    	GPIOMiddleLevel_Set( __O_SPI1_NSS )//GPIO_SetBits(_SPI_CS0_GPIO_PORT, _SPI_CS0_PIN)    
#define _SPI_CS1_LOW()      GPIOMiddleLevel_Clr( __O_EXT_FLASH_CS )
#define _SPI_CS1_HIGH()     GPIOMiddleLevel_Set( __O_EXT_FLASH_CS )
#else
#define _SPI_CS0_LOW()		GPIO_ResetBits(GPIOB, GPIO_Pin_2)
#define _SPI_CS0_HIGH()    	GPIO_SetBits(GPIOB, GPIO_Pin_2)   
#endif /*Smith Modify*/
/*_________________________________________________________________________________________________________*/
// Private Variable
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
  SPI_Cmd(_SPI, DISABLE);
  
  /*!< Configure _SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = _SPI_SCK_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure _SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = _SPI_MISO_PIN;
  GPIO_Init(_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure _SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = _SPI_MOSI_PIN;
  GPIO_Init(_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

}

static void __STM32_SPI_Configuration( void )
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/*!< Setup SPI Periph clock enable */
	RCC_APB2PeriphClockCmd(_SPI_CS0_GPIO_CLK | _SPI_MOSI_GPIO_CLK | _SPI_MISO_GPIO_CLK |_SPI_SCK_GPIO_CLK, ENABLE);

	/*!< _SPI Periph clock enable */
	RCC_APB2PeriphClockCmd(_SPI_CLK, ENABLE);

	/*!< Configure _SPI pins: SCK */
	GPIO_InitStructure.GPIO_Pin = _SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	/* alternate function push-pull */
	GPIO_Init(_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure _SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = _SPI_MOSI_PIN;
	GPIO_Init(_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure _SPI pins: MISO */
	GPIO_InitStructure.GPIO_Pin = _SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
	GPIO_Init(_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/*!< Deselect the SPI: Chip Select high */
	_SPI_CS0_HIGH();
    _SPI_CS1_HIGH();
    
	
	//SPI_I2S_DeInit(_SPI);
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

	/*smith modify SPI clk = 18Mhz, becaseu CS495314 max spi clk is 25Mhz*/
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

	SPI_Init(_SPI, &SPI_InitStructure);

	/*!< Enable the SPI  */
	SPI_Cmd(_SPI, ENABLE);
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
