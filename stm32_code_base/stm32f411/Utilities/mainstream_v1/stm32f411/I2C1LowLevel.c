#include "I2C1LowLevel.h"
#include "config.h"
#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"

/*______________________________________________________________________________________________*/
#if defined ( STM32_IAP )    
#define configI2C1_DMA 1
#define DRV_mutex 0
#define APP_mutex 0
#else
#define configI2C1_DMA 1
#define DRV_mutex 1
#define APP_mutex 1
#endif

#define sEE_I2C                          I2C1
#define sEE_I2C_CLK                      RCC_APB1Periph_I2C1
#define sEE_I2C_SCL_PIN                  GPIO_Pin_6                  /* PB.06 */
#define sEE_I2C_SCL_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C_SCL_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define sEE_I2C_SCL_SOURCE               GPIO_PinSource6
#define sEE_I2C_SCL_AF                   GPIO_AF_I2C1
#define sEE_I2C_SDA_PIN                  GPIO_Pin_7                  /* PB.07 */
#define sEE_I2C_SDA_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C_SDA_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define sEE_I2C_SDA_SOURCE               GPIO_PinSource7
#define sEE_I2C_SDA_AF                   GPIO_AF_I2C1

#define sEE_I2C_DMA                      DMA1   
#define sEE_I2C_DMA_CHANNEL              DMA_Channel_1
#define sEE_I2C_DMA_STREAM_TX            DMA1_Stream6
#define sEE_I2C_DMA_STREAM_RX            DMA1_Stream0   
#define sEE_I2C_DMA_CLK                  RCC_AHB1Periph_DMA1
#define sEE_I2C_DR_Address               ((uint32_t)0x40005410)
   
#define sEE_I2C_DMA_TX_IRQn              DMA1_Stream6_IRQn
#define sEE_I2C_DMA_RX_IRQn              DMA1_Stream0_IRQn
#define sEE_I2C_DMA_TX_IRQHandler        DMA1_Stream6_IRQHandler
#define sEE_I2C_DMA_RX_IRQHandler        DMA1_Stream0_IRQHandler   
#define sEE_I2C_DMA_PREPRIO              0
#define sEE_I2C_DMA_SUBPRIO              0   
   
#define sEE_TX_DMA_FLAG_FEIF             DMA_FLAG_FEIF6
#define sEE_TX_DMA_FLAG_DMEIF            DMA_FLAG_DMEIF6
#define sEE_TX_DMA_FLAG_TEIF             DMA_FLAG_TEIF6
#define sEE_TX_DMA_FLAG_HTIF             DMA_FLAG_HTIF6
#define sEE_TX_DMA_FLAG_TCIF             DMA_FLAG_TCIF6
#define sEE_RX_DMA_FLAG_FEIF             DMA_FLAG_FEIF0
#define sEE_RX_DMA_FLAG_DMEIF            DMA_FLAG_DMEIF0
#define sEE_RX_DMA_FLAG_TEIF             DMA_FLAG_TEIF0
#define sEE_RX_DMA_FLAG_HTIF             DMA_FLAG_HTIF0
#define sEE_RX_DMA_FLAG_TCIF             DMA_FLAG_TCIF0
   
#define sEE_DIRECTION_TX                 0
#define sEE_DIRECTION_RX                 1 

#define sEE_FLAG_TIMEOUT         ((uint32_t)(0x1000/2))
#define sEE_LONG_TIMEOUT         ((uint32_t)(300 * sEE_FLAG_TIMEOUT))

#define sEE_OK                    0
#define sEE_FAIL                  1

#define I2C_SLAVE_ADDRESS7      0
#define I2C_SPEED               90000

#define USE_DEFAULT_TIMEOUT_CALLBACK
#define USE_DEFAULT_CRITICAL_CALLBACK 

#define I2C_INTERVAL_TIME	5
/*______________________________________________________________________________________________*/

typedef FlagStatus ( *i2c_get_flag_status )( I2C_TypeDef* I2Cx, uint32_t I2C_FLAG );

typedef ErrorStatus ( *i2c_check_event ) ( I2C_TypeDef* I2Cx, uint32_t I2C_EVENT );


//! < Private variable    @{
static DMA_InitTypeDef   sEEDMA_InitStructure; 
static __IO uint32_t  sEETimeout = sEE_LONG_TIMEOUT;   
static __IO uint16_t* sEEDataReadPointer;   
static __IO uint16_t*  sEEDataWritePointer;  

static bool bIsI2C_enable = FALSE;


#if DRV_mutex
static xSemaphoreHandle _DRV_I2C1_SEMA = NULL;
static portTickType _DRV_xI2C1_BlockTime = portMAX_DELAY;
#endif

#if APP_mutex
//for i2c1 mutex
static xSemaphoreHandle _I2C1_SEMA = NULL;
static portTickType _xI2C1_BlockTime = portMAX_DELAY;
#endif

//!    @}

static void sEE_Init(void);

static void sEE_LowLevel_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction);

static void sEE_LowLevel_DeInit(void);

/*______________________________________________________________________________________________*/
#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t sEE_TIMEOUT_UserCallback(uint8_t DevAddr )
{
    I2C_SoftwareResetCmd(sEE_I2C,ENABLE); 
    I2C_SoftwareResetCmd(sEE_I2C,DISABLE);
	sEE_LowLevel_DeInit( );
    sEE_Init();
    return sEE_FAIL;
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */

#ifdef USE_DEFAULT_CRITICAL_CALLBACK
/**
  * @brief  Start critical section: these callbacks should be typically used
  *         to disable interrupts when entering a critical section of I2C communication
  *         You may use default callbacks provided into this driver by uncommenting the 
  *         define USE_DEFAULT_CRITICAL_CALLBACK.
  *         Or you can comment that line and implement these callbacks into your 
  *         application.
  * @param  None.
  * @retval None.
  */
void sEE_EnterCriticalSection_UserCallback(void)
{
    __disable_irq();  
}

/**
  * @brief  Start and End of critical section: these callbacks should be typically used
  *         to re-enable interrupts when exiting a critical section of I2C communication
  *         You may use default callbacks provided into this driver by uncommenting the 
  *         define USE_DEFAULT_CRITICAL_CALLBACK.
  *         Or you can comment that line and implement these callbacks into your 
  *         application.
  * @param  None.
  * @retval None.
  */
void sEE_ExitCriticalSection_UserCallback(void)
{
    __enable_irq();
}
#endif /* USE_DEFAULT_CRITICAL_CALLBACK */

#if DRV_mutex
static bool _DRV_I2C1_sema_mutex_create(void)
{
    if (_DRV_I2C1_SEMA == NULL)
    {
        _DRV_I2C1_SEMA  = xSemaphoreCreateMutex();

        if (_DRV_I2C1_SEMA)
        {
            _DRV_xI2C1_BlockTime = portMAX_DELAY;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return TRUE;
    }
}

static bool _DRV_I2C1_sema_mutex_take(void)
{
#if DRV_mutex
    if (_DRV_I2C1_sema_mutex_create())
    {
        //take mutex for blocking
        if ( xSemaphoreTake( _DRV_I2C1_SEMA, _DRV_xI2C1_BlockTime ) != pdTRUE )
        {
            //TRACE_DEBUG((0, "App_I2C1_sema_mutex_take busy 3333 !! "));
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        //false to take mutex
        return FALSE;
    }
#else
        return TRUE;
#endif
}

static bool _DRV_I2C1_sema_mutex_give(void)
{
#if DRV_mutex
    if (_DRV_I2C1_sema_mutex_create())
    {
        //give mutex for unblocking
        if ( xSemaphoreGive( _DRV_I2C1_SEMA) != pdTRUE )
        {
            //TRACE_DEBUG((0, "App_I2C1_sema_mutex_give busy 3333 !! "));
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        //false to give mutex
        return FALSE;
    }
#else
    return TRUE;
#endif
}

#endif

bool i2c1_flag_timeout_check( i2c_get_flag_status i2c_flag_status, I2C_TypeDef* I2Cx, uint32_t I2C_FLAG, uint32 time )
{
	uint32_t timeout = time;
	bool ret = FALSE;

	while( i2c_flag_status( I2Cx, I2C_FLAG ) )
	{
		if ( (timeout--) == 0 )
		{
			ret = TRUE;
			break;
		}
	}
	return ret;
}

bool i2c1_event_timeout_check( i2c_check_event i2c_event_status, I2C_TypeDef* I2Cx, uint32_t I2C_EVENT, uint32 time )
{
	uint32_t timeout = time;
	bool ret = FALSE;

	while( !i2c_event_status( I2Cx, I2C_EVENT ) )
	{
		if ( (timeout--) == 0 )
		{
			ret = TRUE;
			break;
		}
	}
	return ret;
}


static void sEE_LowLevel_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction)
{ 
    /* Initialize the DMA with the new parameters */
    if (Direction == sEE_DIRECTION_TX)
    {
        /* Configure the DMA Tx Stream with the buffer address and the buffer size */
        sEEDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)pBuffer;
        sEEDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;    
        sEEDMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;  
        DMA_Init(sEE_I2C_DMA_STREAM_TX, &sEEDMA_InitStructure);
		
        DMA_Init(sEE_I2C_DMA_STREAM_TX, &sEEDMA_InitStructure);
		/* Enable the DMA Tx Channel */
		DMA_Cmd(sEE_I2C_DMA_STREAM_TX, ENABLE);
    }
    else
    { 
        /* Configure the DMA Rx Stream with the buffer address and the buffer size */
        sEEDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)pBuffer;
        sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        sEEDMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;  

		/*Smith JUNE 14 2013: Fix wave sound cannot be continually outputted when Treble, volume..was controlled by USER.@{*/
		sEEDMA_InitStructure.DMA_Priority = DMA_Priority_High;
		//!@}
#if 0   /*Smith comment: We do not need DMA to control I2C1*/		
        DMA_Init(sEE_I2C_DMA_STREAM_RX, &sEEDMA_InitStructure);
        
		/* Inform the DMA that the next End Of Transfer Signal will be the last one */
        I2C_DMALastTransferCmd(sEE_I2C, ENABLE); 
        
        /* Enable the DMA Rx Channel */
        DMA_Cmd(sEE_I2C_DMA_STREAM_TX, ENABLE);  
#endif        
    }
}

static uint32_t __STM32_I2C_readSlave(    uint8_t    DevAddr,
                                                uint16_t ReadAddr, 
                                                uint8_t *pBuffer, 
                                                uint16_t *NumByteToRead, 
                                                bool longAddr)
{

	uint16_t NumByteToReadN = *NumByteToRead;
	bool IsTimeout = FALSE;
	uint8 state_mach = 0;
	uint32_t ret = sEE_OK;

	/* Disable the interrupts mechanism to prevent the I2C communication from corruption */
	//__disable_irq();
	for( state_mach = 0; state_mach <= 5; state_mach++ )
	{
		IsTimeout = FALSE;	

		switch( state_mach )
		{
			case 0:
			{
				IsTimeout = i2c1_flag_timeout_check( I2C_GetFlagStatus, sEE_I2C, I2C_FLAG_BUSY, sEE_FLAG_TIMEOUT );
			}
				break;

			case 1:
			{
				/* Send STRAT condition */ 
				I2C_GenerateSTART(sEE_I2C, ENABLE); 
				IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
			}
				break;

			case 2:
			{
				/* Send device address for read */ 
				I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter); 
				IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
				if ( IsTimeout == FALSE )
				{
					/* Clear EV6 by setting again the PE bit */
					I2C_Cmd(sEE_I2C, ENABLE);
				}
			}
				break;

			case 3:
			{
				if ( longAddr == TRUE )
				{
					/* Send the device's internal address to read to */
					I2C_SendData(sEE_I2C, (uint8_t)((ReadAddr & 0xFF00) >> 8));
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send the device's internal address to read to */
					I2C_SendData(sEE_I2C, (uint8_t)(ReadAddr & 0x00FF));	
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send STRAT condition a second time */  
					I2C_GenerateSTART(sEE_I2C, ENABLE);
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send device address for read */
					I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Receiver);
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

				}
				else 
				{
					
					/* Send the device's internal address to read to */
					I2C_SendData(sEE_I2C, (uint8_t)(ReadAddr & 0x00FF));
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}
					

					/* Send STRAT condition a second time */  
					I2C_GenerateSTART(sEE_I2C, ENABLE);
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send device address for read */
					I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Receiver);
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

				}

			}
				break;

			case 4:
			{
				__disable_irq();
				/* While there is data to be read */
				while( NumByteToReadN )  
				{
					if( NumByteToReadN == 1 )
					{
						// Disable Acknowledgement 
						I2C_AcknowledgeConfig(sEE_I2C, DISABLE);

						// Send STOP Condition 
						I2C_GenerateSTOP(sEE_I2C, ENABLE);
					}

					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == FALSE )
					{
						// Read a byte from the HMC5843 
						*pBuffer = I2C_ReceiveData(sEE_I2C);

						// Point to the next location where the byte read will be saved 
						pBuffer++; 

						// Decrement the read bytes counter 
						NumByteToReadN--;		 

					}
					else 
					{
						break;
					}
					
				}
				__enable_irq();
			}
				break;

			case 5:
			{
				/* Enable Acknowledgement to be ready for another reception */
				I2C_AcknowledgeConfig(sEE_I2C, ENABLE);
			}
				break;


		}

		if ( IsTimeout == TRUE )
		{
#if !defined ( STM32_IAP ) 
			TRACE_ERROR((0, "__STM32_I2C_readSlave error state = %d; Device Addr = 0x%X; Register = 0x%X", state_mach,  DevAddr, ReadAddr ));
#endif
			break;
		}
	}
	
	 /* Re-enable the interrupt mechanism */
	//__enable_irq();

	 *NumByteToRead = NumByteToReadN; 


	if ( IsTimeout == TRUE )
	{
		ret = sEE_TIMEOUT_UserCallback(DevAddr);		
	}

	
	return ret;
		
}


static uint32_t __STM32_I2C_writeSlave(    uint8_t DevAddr, 
                                                uint16_t WriteAddr, 
                                                uint8_t *pBuffer, 
                                                uint16_t *NumByteToWrite, 
                                                bool longAddr,
                                                bool b_DMA_used)
{ 
    /* Set the pointer to the Number of data to be written. This pointer will be used 
    by the DMA Transfer Completer interrupt Handler in order to reset the 
    variable to 0. User should check on this variable in order to know if the 
    DMA transfer has been complete or not. */
    sEEDataWritePointer = NumByteToWrite;
	uint16_t NumByteForWrite = *NumByteToWrite;
	uint16_t index = 0; /*Smith Fixes Bug: if length > 255, system crash.*/
	bool IsTimeout = FALSE;
	uint8 state_mach = 0;
	uint32_t ret = sEE_OK;


	for( state_mach = 0; state_mach <= 4; state_mach++ )
	{
		IsTimeout = FALSE;	
		
		switch ( state_mach )
		{
			case 0:
			{
				IsTimeout = i2c1_flag_timeout_check( I2C_GetFlagStatus, sEE_I2C, I2C_FLAG_BUSY, sEE_FLAG_TIMEOUT );
			}
				break;

			case 1:
				{
				/*!< Send START condition */
				I2C_GenerateSTART(sEE_I2C, ENABLE);	
				IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
			}
				break;

			case 2:
			{
			    /*!< Send EEPROM address for write */
			    I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter);
				IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
			}
				break;

			case 3:
			{

			    if ( longAddr == TRUE)
			    {
			        /*!< Send the EEPROM's internal address to write to : MSB of the address first */
			        I2C_SendData(sEE_I2C, (uint8_t)((WriteAddr & 0xFF00) >> 8));
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT ); 
					if ( IsTimeout == TRUE )
						break;	

			        /*!< Send the EEPROM's internal address to write to : LSB of the address */
			        I2C_SendData(sEE_I2C, (uint8_t)(WriteAddr & 0x00FF));
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
						break;

			    }
			    else
			    {
			        /*!< Send the EEPROM's internal address to write to : only one byte Address */
			        I2C_SendData(sEE_I2C, WriteAddr);
					IsTimeout = i2c1_event_timeout_check( I2C_CheckEvent, sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
						break;
			    }
			}
				break;

			case 4:
			{
				if (b_DMA_used)
				{				
                                    sEE_LowLevel_DMAConfig((uint32_t)pBuffer, (uint16_t)(*NumByteToWrite), sEE_DIRECTION_TX);
				}
				else
				{
                                    if ( NumByteForWrite < 2 )
                                    {
                                        /* Configure the DMA Tx Channel with the buffer address and the buffer size */
                                        sEE_LowLevel_DMAConfig((uint32_t)pBuffer, (uint16_t)(*NumByteToWrite), sEE_DIRECTION_TX);
                                    }
                                    else
                                    {
                                        /* Disable the interrupts mechanism to prevent the I2C communication from corruption */
                                        __disable_irq();
                                    
                                        for (index = 0; index < NumByteForWrite; index ++ )
                                        {  
                                            /* Prepare the register value to be sent */
                                            I2C_SendData(sEE_I2C, *(pBuffer+index));
                                    
                                            /*!< Wait till all data have been physically transferred on the bus */
                                            sEETimeout = sEE_FLAG_TIMEOUT;
                                            while(!I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BTF))
                                            {
                                                if((sEETimeout--) == 0)
                                                {   
                                                    IsTimeout = TRUE;
                                                    break;
                                                    //sEE_TIMEOUT_UserCallback(DevAddr);
                                                }   
                                            }
                                    
                                            if ( IsTimeout == TRUE )
                                                break;  
                                    
                                            /*!< Decrement the read bytes counter */
                                            (uint16_t)(*NumByteToWrite)--;
                                        }
                                    
                                        /* Re-enable the interrupt mechanism */
                                        __enable_irq();
                                    
                                        /*!< Send STOP condition */ /*Smith 23 Sept.: if STOP condition is never to send that will make I2C flag busy */
                                        I2C_GenerateSTOP(sEE_I2C, ENABLE);
                                    
                                        /* Perform a read on SR1 and SR2 register to clear eventualaly pending flags */
                                        (void)sEE_I2C->SR1;
                                        (void)sEE_I2C->SR2;
                                    
                                    }
				}
				
			}
				break;

			}	

		if ( IsTimeout == TRUE )
		{
#if !defined ( STM32_IAP )               
			TRACE_ERROR((0, "__STM32_I2C_writeSlave error state = %d; Device Addr = 0x%X; Register = 0x%X", state_mach,  DevAddr, WriteAddr ));
#endif			
			break;
		}
		
	}

	if ( IsTimeout == TRUE )
	{
		ret = sEE_TIMEOUT_UserCallback(DevAddr);
	}

    /* If all operations OK, return sEE_OK (0) */
    return ret;
}

void sEE_I2C_DMA_TX_IRQHandler(void)
{
  /* Check if the DMA transfer is complete */
  if(DMA_GetFlagStatus(sEE_I2C_DMA_STREAM_TX, sEE_TX_DMA_FLAG_TCIF) != RESET)
  {  
    /* Disable the DMA Tx Stream and Clear TC flag */  
    DMA_Cmd(sEE_I2C_DMA_STREAM_TX, DISABLE);
    DMA_ClearFlag(sEE_I2C_DMA_STREAM_TX, sEE_TX_DMA_FLAG_TCIF);

    /*!< Wait till all data have been physically transferred on the bus */
    sEETimeout = sEE_LONG_TIMEOUT;
    while(!I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BTF))
    {
      if((sEETimeout--) == 0) return;
    }
    
    /*!< Send STOP condition */
    I2C_GenerateSTOP(sEE_I2C, ENABLE);
    
    /* Reset the variable holding the number of data to be written */
    *sEEDataWritePointer = 0;  
  }
}


void sEE_I2C_DMA_RX_IRQHandler(void)
{
  /* Check if the DMA transfer is complete */
  if(DMA_GetFlagStatus(sEE_I2C_DMA_STREAM_RX, sEE_RX_DMA_FLAG_TCIF) != RESET)
  {      
    /*!< Send STOP Condition */
    I2C_GenerateSTOP(sEE_I2C, ENABLE);    
    
    /* Disable the DMA Rx Stream and Clear TC Flag */  
    DMA_Cmd(sEE_I2C_DMA_STREAM_RX, DISABLE);
    DMA_ClearFlag(sEE_I2C_DMA_STREAM_RX, sEE_RX_DMA_FLAG_TCIF);
    
    /* Reset the variable holding the number of data to be read */
    *sEEDataReadPointer = 0;
  }
}


static void sEE_LowLevel_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;  

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(sEE_I2C_CLK, ENABLE);

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHB1PeriphClockCmd(sEE_I2C_SCL_GPIO_CLK | sEE_I2C_SDA_GPIO_CLK, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Reset sEE_I2C IP */
    RCC_APB1PeriphResetCmd(sEE_I2C_CLK, ENABLE);

    /* Release reset signal of sEE_I2C IP */
    RCC_APB1PeriphResetCmd(sEE_I2C_CLK, DISABLE);

    /*!< GPIO configuration */
    /* Connect PXx to I2C_SCL*/
    GPIO_PinAFConfig(sEE_I2C_SCL_GPIO_PORT, sEE_I2C_SCL_SOURCE, sEE_I2C_SCL_AF);
    /* Connect PXx to I2C_SDA*/
    GPIO_PinAFConfig(sEE_I2C_SDA_GPIO_PORT, sEE_I2C_SDA_SOURCE, sEE_I2C_SDA_AF);  

    /*!< Configure sEE_I2C pins: SCL */   
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(sEE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SDA_PIN;
    GPIO_Init(sEE_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);


    /* Configure and enable I2C DMA TX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_TX_IRQn;
#if defined ( STM32_IAP ) 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
#endif
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  
    /* Configure and enable I2C DMA RX Channel interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_RX_IRQn;
#if defined ( STM32_IAP ) 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
#endif
    NVIC_Init(&NVIC_InitStructure);  

    /*!< I2C DMA TX and RX channels configuration */
    /* Enable the DMA clock */
    RCC_AHB1PeriphClockCmd(sEE_I2C_DMA_CLK, ENABLE);

    /* Clear any pending flag on Rx Stream  */
    DMA_ClearFlag(sEE_I2C_DMA_STREAM_TX, sEE_TX_DMA_FLAG_FEIF | sEE_TX_DMA_FLAG_DMEIF | sEE_TX_DMA_FLAG_TEIF | \
    sEE_TX_DMA_FLAG_HTIF | sEE_TX_DMA_FLAG_TCIF);
    
    /* Disable the EE I2C Tx DMA stream */
    DMA_Cmd(sEE_I2C_DMA_STREAM_TX, DISABLE);
    /* Configure the DMA stream for the EE I2C peripheral TX direction */
    DMA_DeInit(sEE_I2C_DMA_STREAM_TX);
    sEEDMA_InitStructure.DMA_Channel = sEE_I2C_DMA_CHANNEL;
    sEEDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)sEE_I2C_DR_Address;
    sEEDMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;    /* This parameter will be configured durig communication */;
    sEEDMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_BufferSize = 0xFFFF;              /* This parameter will be configured durig communication */
    sEEDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    sEEDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    sEEDMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    sEEDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    sEEDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    sEEDMA_InitStructure.DMA_Priority = DMA_Priority_High;
    sEEDMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    sEEDMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    sEEDMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    sEEDMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(sEE_I2C_DMA_STREAM_TX, &sEEDMA_InitStructure);

    /* Clear any pending flag on Rx Stream */
    DMA_ClearFlag(sEE_I2C_DMA_STREAM_RX, sEE_RX_DMA_FLAG_FEIF | sEE_RX_DMA_FLAG_DMEIF | sEE_RX_DMA_FLAG_TEIF | \
    sEE_RX_DMA_FLAG_HTIF | sEE_RX_DMA_FLAG_TCIF);
    /* Disable the EE I2C DMA Rx stream */
    DMA_Cmd(sEE_I2C_DMA_STREAM_RX, DISABLE);
    /* Configure the DMA stream for the EE I2C peripheral RX direction */
    DMA_DeInit(sEE_I2C_DMA_STREAM_RX);
    DMA_Init(sEE_I2C_DMA_STREAM_RX, &sEEDMA_InitStructure);

    /* Enable the DMA Channels Interrupts */
#if ( configI2C1_DMA == 1 )	
    DMA_ITConfig(sEE_I2C_DMA_STREAM_TX, DMA_IT_TC, ENABLE);
#else
    DMA_ITConfig(sEE_I2C_DMA_STREAM_TX, DMA_IT_TC, DISABLE);
#endif
    DMA_ITConfig(sEE_I2C_DMA_STREAM_RX, DMA_IT_TC, ENABLE);   

}

static void sEE_LowLevel_DeInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;    

    /* sEE_I2C Peripheral Disable */
    I2C_Cmd(sEE_I2C, DISABLE);

    /* sEE_I2C DeInit */
    I2C_DeInit(sEE_I2C);

    /*!< sEE_I2C Periph clock disable */
    RCC_APB1PeriphClockCmd(sEE_I2C_CLK, DISABLE);

    /*!< GPIO configuration */  
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(sEE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C_SDA_PIN;
    GPIO_Init(sEE_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

    /* Configure and enable I2C DMA TX Stream interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_TX_IRQn;
#if defined ( STM32_IAP ) 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
#endif
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure and enable I2C DMA RX Stream interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_RX_IRQn;
#if defined ( STM32_IAP ) 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIB_I2C_DMA_INTERRUPT_PRIORITY;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
#endif    
    NVIC_Init(&NVIC_InitStructure);   

    /* Disable and Deinitialize the DMA Streams */
    DMA_Cmd(sEE_I2C_DMA_STREAM_TX, DISABLE);
    DMA_Cmd(sEE_I2C_DMA_STREAM_RX, DISABLE);
    DMA_DeInit(sEE_I2C_DMA_STREAM_TX);
    DMA_DeInit(sEE_I2C_DMA_STREAM_RX);
}


static void sEE_Init(void)
{ 
    I2C_InitTypeDef  I2C_InitStructure;

    sEE_LowLevel_Init();

    /*!< I2C configuration */
    /* sEE_I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

    /* sEE_I2C Peripheral Enable */
    I2C_Cmd(sEE_I2C, ENABLE);

    /* Apply sEE_I2C configuration after enabling it */
    I2C_Init(sEE_I2C, &I2C_InitStructure);

#if ( configI2C1_DMA == 1 )	
    /* Enable the sEE_I2C peripheral DMA requests */
    I2C_DMACmd(sEE_I2C, ENABLE);
#else
	I2C_DMACmd(sEE_I2C, DISABLE);
#endif
}


/*__________________________________________________________________________________________________________*/
void TWI_initialize( void )
{
    sEE_Init();  
    bIsI2C_enable = TRUE;
}

void TWI_Deinitialize( void )
{
    sEE_LowLevel_DeInit( );
    bIsI2C_enable = FALSE;
}

bool I2C_writeSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr, bool b_DMA_used) 
{
    int status = I2C_SUCCESS;
    uint16 len = length;
    uint8 *pbuff;

    if ( bIsI2C_enable != TRUE )
    {
#if !defined ( STM32_IAP )       
        TRACE_ERROR((0, "I2C interface has not been enable !!"));
#endif
        return FALSE;
    }

	if ( data == NULL )
	{
#if !defined ( STM32_IAP )          
		 TRACE_ERROR((0, "I2C write buffer is null !!"));
#endif
		return FALSE;
	}

#if DRV_mutex
    if (_DRV_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }
#endif
	
    pbuff = data;

    status = __STM32_I2C_writeSlave(control, address, pbuff, &len, longAddr, b_DMA_used);

    /*Don't remove.*/
	if( ( control == 0xA0 ) || ( control == 0x40 ) )
	{
#if !defined ( STM32_IAP ) 	
    	vTaskDelay( TASK_MSEC2TICKS(I2C_INTERVAL_TIME) );
#endif
	}	

#if DRV_mutex
    _DRV_I2C1_sema_mutex_give();
#endif

    if ( status != sEE_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

bool I2C_readSlave(    byte control, 
                        uint16 address, 
                        byte *data,
                        uint16 length, 
                        bool longAddr) 
{
    int status = I2C_SUCCESS;
    uint16 NumByteToRead = length;
    uint8 *pBuffer;

    if ( bIsI2C_enable != TRUE )
    {
#if !defined ( STM32_IAP )          
        TRACE_ERROR((0, "I2C interface has not been enable !!"));
#endif
        return FALSE;
    }

	
	if ( data == NULL )
	{
#if !defined ( STM32_IAP )          
		 TRACE_ERROR((0, "I2C reads buffer is null !!"));
#endif
		return FALSE;
	}

#if DRV_mutex
    if (_DRV_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }
#endif
    pBuffer = data;

    status = __STM32_I2C_readSlave(control, address, pBuffer, &NumByteToRead, longAddr);

    /*Don't remove.*/
	if( ( control == 0xA0 ) || ( control == 0x40 ) )
	{
    	vTaskDelay( TASK_MSEC2TICKS(I2C_INTERVAL_TIME) );
	}

#if DRV_mutex
    _DRV_I2C1_sema_mutex_give();
#endif

    if ( status != sEE_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

bool I2C_writeSlave_NoSystemWait(byte control, uint16 address, byte *data, uint16 length, bool longAddr) 
{
    int status = I2C_SUCCESS;
    uint16 len = length;
    uint8 *pbuff;

    if ( bIsI2C_enable != TRUE )
    {
#if !defined ( STM32_IAP )          
        TRACE_ERROR((0, "I2C interface has not been enable !!"));
#endif
        return FALSE;
    }
    
#if DRV_mutex
        if (_DRV_I2C1_sema_mutex_take() != TRUE)
        {
            return FALSE;
        }
#endif
    
    pbuff = data;

    status = __STM32_I2C_writeSlave(control, address, pbuff, &len, longAddr, FALSE);

#if DRV_mutex
        _DRV_I2C1_sema_mutex_give();
#endif

    if ( status != sEE_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

#if APP_mutex
static bool _App_I2C1_sema_mutex_create(void)
{
#if 1
    if (_I2C1_SEMA == NULL)
    {
        _I2C1_SEMA  = xSemaphoreCreateMutex();

        if (_I2C1_SEMA)
        {
            _xI2C1_BlockTime = portMAX_DELAY;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return TRUE;
    }
#else
    return TRUE;
#endif
}

bool App_I2C1_sema_mutex_take(void)
{
#if APP_mutex
    if (_App_I2C1_sema_mutex_create())
    {
        //take mutex for blocking
        if ( xSemaphoreTake( _I2C1_SEMA, _xI2C1_BlockTime ) != pdTRUE )
        {
            //TRACE_DEBUG((0, "App_I2C1_sema_mutex_take busy 3333 !! "));
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        //false to take mutex
        return FALSE;
    }
#else
        return TRUE;
#endif
}

bool App_I2C1_sema_mutex_give(void)
{
#if APP_mutex
    if (_App_I2C1_sema_mutex_create())
    {
        //give mutex for unblocking
        if ( xSemaphoreGive( _I2C1_SEMA) != pdTRUE )
        {
            //TRACE_DEBUG((0, "App_I2C1_sema_mutex_give busy 3333 !! "));
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        //false to give mutex
        return FALSE;
    }
#else
        return TRUE;
#endif    
}
#endif
