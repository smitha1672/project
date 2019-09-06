#include "Debug.h"
#include "freertos_conf.h"
#include "freertos_task.h"

#include "config.h"
#include "I2C2LowLevel.h"

//_______________________________________________________________________________________________________________________
#define sEE_I2C2                          I2C3
#define sEE_I2C2_CLK                      RCC_APB1Periph_I2C3
#define sEE_I2C2_SCL_PIN                  GPIO_Pin_8                  /* PA.08 */
#define sEE_I2C2_SCL_GPIO_PORT            GPIOA                       /* GPIOA */
#define sEE_I2C2_SCL_GPIO_CLK             RCC_AHB1Periph_GPIOA
#define sEE_I2C2_SCL_SOURCE               GPIO_PinSource8
#define sEE_I2C2_SCL_AF                   GPIO_AF_I2C3
#define sEE_I2C2_SDA_PIN                  GPIO_Pin_9                  /* PC.09 */
#define sEE_I2C2_SDA_GPIO_PORT            GPIOC                       /* GPIOC */
#define sEE_I2C2_SDA_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define sEE_I2C2_SDA_SOURCE               GPIO_PinSource9
#define sEE_I2C2_SDA_AF                   GPIO_AF_I2C3

#define sEE_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define sEE_LONG_TIMEOUT         ((uint32_t)(300 * sEE_FLAG_TIMEOUT))

#define sEE_OK                    0
#define sEE_FAIL                  1

#define I2C_SLAVE_ADDRESS7      0
#define I2C_SPEED               90000

#define I2C_INTERVAL_TIME	5

//_______________________________________________________________________________________________________________________

typedef FlagStatus ( *i2c_get_flag_status )( I2C_TypeDef* I2Cx, uint32_t I2C_FLAG );

typedef ErrorStatus ( *i2c_check_event ) ( I2C_TypeDef* I2Cx, uint32_t I2C_EVENT );

//_______________________________________________________________________________________________________________________
static bool bIsI2C2_enable = FALSE;
//_______________________________________________________________________________________________________________________
static void sEE_Init2(void);

//_______________________________________________________________________________________________________________________

uint32_t sEE_TIMEOUT_UserCallback2(uint8_t	DevAddr)
{
	I2C_InitTypeDef  I2C_InitStructure;

	TRACE_ERROR((0, "Device Address 0x%X I2C interface timeout !!", DevAddr));
	
	I2C_GenerateSTOP(sEE_I2C2, ENABLE);
	I2C_SoftwareResetCmd(sEE_I2C2,ENABLE); 
	I2C_SoftwareResetCmd(sEE_I2C2,DISABLE); 
    //sEE_Init2();
	I2C_DeInit(sEE_I2C2);

	/*!< I2C configuration */
	/* sEE_I2C2 configuration */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

	/* sEE_I2C2 Peripheral Enable */
	I2C_Cmd(sEE_I2C2, ENABLE);

	/* Apply sEE_I2C2 configuration after enabling it */
	I2C_Init(sEE_I2C2, &I2C_InitStructure);

	return sEE_FAIL;
}


void sEE_EnterCriticalSection_UserCallback2(void)
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
void sEE_ExitCriticalSection_UserCallback2(void)
{
  __enable_irq();
}

bool i2c2_flag_timeout_check( i2c_get_flag_status i2c_flag_status, I2C_TypeDef* I2Cx, uint32_t I2C_FLAG, uint32 time )
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

bool i2c2_event_timeout_check( i2c_check_event i2c_event_status, I2C_TypeDef* I2Cx, uint32_t I2C_EVENT, uint32 time )
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

static uint32_t __STM32_I2C2_readSlave(	uint8_t	DevAddr,
												uint16_t ReadAddr, 
												uint8_t *pBuffer, 
												uint16_t *NumByteToRead, 
												bool longAddr)
{

	uint16_t NumByteToReadN = *NumByteToRead;
	bool IsTimeout = FALSE;
	uint8 state_mach = 0;

	/* Disable the interrupts mechanism to prevent the I2C communication from corruption */
	//__disable_irq();
	for( state_mach = 0; state_mach <= 5; state_mach++ )
	{

		switch( state_mach )
		{
			case 0:
			{
				IsTimeout = i2c2_flag_timeout_check( I2C_GetFlagStatus, sEE_I2C2, I2C_FLAG_BUSY, sEE_LONG_TIMEOUT );
			}
				break;

			case 1:
			{
				/* Send STRAT condition */ 
				I2C_GenerateSTART(sEE_I2C2, ENABLE); 
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_LONG_TIMEOUT );
			}
				break;

			case 2:
			{
				/* Send device address for read */ 
				I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Transmitter); 
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_LONG_TIMEOUT );
				if ( IsTimeout == FALSE )
				{
					/* Clear EV6 by setting again the PE bit */
					I2C_Cmd(sEE_I2C2, ENABLE);
				}
				else
				{
					TRACE_ERROR((0, "Assert failed %s[%d]", __FUNCTION__, __LINE__));
				}
			}
				break;

			case 3:
			{
				if ( longAddr == TRUE )
				{
					/* Send the device's internal address to read to */
					I2C_SendData(sEE_I2C2, (uint8_t)((ReadAddr & 0xFF00) >> 8));
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send the device's internal address to read to */
					I2C_SendData(sEE_I2C2, (uint8_t)(ReadAddr & 0x00FF));	
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send STRAT condition a second time */  
					I2C_GenerateSTART(sEE_I2C2, ENABLE);
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send device address for read */
					I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Receiver);
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

				}
				else 
				{
					
					/* Send the device's internal address to read to */
					I2C_SendData(sEE_I2C2, (uint8_t)(ReadAddr & 0x00FF));
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_LONG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}
					

					/* Send STRAT condition a second time */  
					I2C_GenerateSTART(sEE_I2C2, ENABLE);
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_LONG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send device address for read */
					I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Receiver);
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_LONG_TIMEOUT );
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
						I2C_AcknowledgeConfig(sEE_I2C2, DISABLE);

						// Send STOP Condition 
						I2C_GenerateSTOP(sEE_I2C2, ENABLE);
					}

					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_RECEIVED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == FALSE )
					{
						// Read a byte from the HMC5843 
						*pBuffer = I2C_ReceiveData(sEE_I2C2);

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
				I2C_AcknowledgeConfig(sEE_I2C2, ENABLE);
			}
				break;


		}

		if ( IsTimeout == TRUE )
		{
			TRACE_ERROR((0, "__STM32_I2C2_readSlave state_mach = %d", state_mach ));
			break;
		}
	}
	
	 /* Re-enable the interrupt mechanism */
	//__enable_irq();

	if ( IsTimeout == TRUE )
	{
		return (sEE_TIMEOUT_UserCallback2(DevAddr));
	}

	
	return sEE_OK;
		
}

  

static uint32_t __STM32_I2C2_writeSlave(	uint8_t DevAddr, 
												uint16_t WriteAddr, 
												const uint8_t *pBuffer, 
												uint8_t *NumByteToWrite, 
												bool longAddr)
{ 
	uint8_t index = 0;
	uint16_t NumByteForWrite = *NumByteToWrite;
	bool IsTimeout = FALSE;
	uint8 state_mach = 0;


	/* Disable the interrupts mechanism to prevent the I2C communication from corruption */
	//__disable_irq();

	for( state_mach = 0; state_mach <= 5; state_mach++ )
	{
		switch( state_mach )
		{
			case 0:
			{
				IsTimeout = i2c2_flag_timeout_check( I2C_GetFlagStatus, sEE_I2C2, I2C_FLAG_BUSY, sEE_FLAG_TIMEOUT );
			}
				break;

			case 1:
			{
				/*!< Send START condition */
				I2C_GenerateSTART(sEE_I2C2, ENABLE);
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
			}
				break;

			case 2:
			{
				I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Transmitter);
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
			}
				break;

			case 3:
			{
				if ( longAddr == TRUE)
				{
					/*!< Send the EEPROM's internal address to write to : MSB of the address first */
					I2C_SendData(sEE_I2C2, (uint8_t)((WriteAddr & 0xFF00) >> 8));
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;
					}

					/*!< Send the EEPROM's internal address to write to : LSB of the address */
					I2C_SendData(sEE_I2C2, (uint8_t)(WriteAddr & 0x00FF));
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;
					}

				}
				else
				{
					/*!< Send the EEPROM's internal address to write to : only one byte Address */
					I2C_SendData(sEE_I2C2, (uint8_t)(WriteAddr & 0x00FF));
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;
					}

				}	
			}
				break;

			case 4:
			{
				for (index = 0; index < NumByteForWrite; index ++ )
				{  
					/* Prepare the register value to be sent */
					I2C_SendData(sEE_I2C2, *(pBuffer+index));

					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;
					}

					/*!< Decrement the read bytes counter */
					(uint16_t)(*NumByteToWrite)--;
				}
			}
				break;

			case 5:
			{
				/*!< Send STOP condition */ /*Smith 23 Sept.: if STOP condition is never to send that will make I2C flag busy */
				I2C_GenerateSTOP(sEE_I2C2, ENABLE);

				/* Perform a read on SR1 and SR2 register to clear eventualaly pending flags */
				(void)sEE_I2C2->SR1;
				(void)sEE_I2C2->SR2;
			}
				break;
		}

		if ( IsTimeout == TRUE )
		{
			TRACE_ERROR((0, "__STM32_I2C2_writeSlave state_mach = %d", state_mach ));
			break;
		}

	}

	/* Re-enable the interrupt mechanism */
	//__enable_irq();


	if ( IsTimeout == TRUE )
	{
		return (sEE_TIMEOUT_UserCallback2(DevAddr));
	}
	

	/* If all operations OK, return sEE_OK (0) */
	return sEE_OK;
}

static void sEE_LowLevel_Init2(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(sEE_I2C2_CLK, ENABLE);

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHB1PeriphClockCmd(sEE_I2C2_SCL_GPIO_CLK | sEE_I2C2_SDA_GPIO_CLK, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Reset sEE_I2C IP */
    RCC_APB1PeriphResetCmd(sEE_I2C2_CLK, ENABLE);

    /* Release reset signal of sEE_I2C IP */
    RCC_APB1PeriphResetCmd(sEE_I2C2_CLK, DISABLE);

    /*!< GPIO configuration */
    /* Connect PXx to I2C_SCL*/
    GPIO_PinAFConfig(sEE_I2C2_SCL_GPIO_PORT, sEE_I2C2_SCL_SOURCE, sEE_I2C2_SCL_AF);
    /* Connect PXx to I2C_SDA*/
    GPIO_PinAFConfig(sEE_I2C2_SDA_GPIO_PORT, sEE_I2C2_SDA_SOURCE, sEE_I2C2_SDA_AF);  

    /*!< Configure sEE_I2C pins: SCL */   
    GPIO_InitStructure.GPIO_Pin = sEE_I2C2_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(sEE_I2C2_SCL_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C2_SDA_PIN;
    GPIO_Init(sEE_I2C2_SDA_GPIO_PORT, &GPIO_InitStructure);

}

void sEE_LowLevel_DeInit2(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure; 

    /* sEE_I2C Peripheral Disable */
    I2C_Cmd(sEE_I2C2, DISABLE);

    /* sEE_I2C DeInit */
    I2C_DeInit(sEE_I2C2);

    /*!< sEE_I2C Periph clock disable */
    RCC_APB1PeriphClockCmd(sEE_I2C2_CLK, DISABLE);

    /*!< GPIO configuration */  
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C2_SCL_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(sEE_I2C2_SCL_GPIO_PORT, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = sEE_I2C2_SDA_PIN;
    GPIO_Init(sEE_I2C2_SDA_GPIO_PORT, &GPIO_InitStructure);

}

static void sEE_Init2(void)
{ 
	I2C_InitTypeDef  I2C_InitStructure;

	sEE_LowLevel_Init2();

	/*!< I2C configuration */
	/* sEE_I2C2 configuration */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;

	/* sEE_I2C2 Peripheral Enable */
	I2C_Cmd(sEE_I2C2, ENABLE);

	/* Apply sEE_I2C2 configuration after enabling it */
	I2C_Init(sEE_I2C2, &I2C_InitStructure);

}

void TWI2_initialize( void )
{
       sEE_Init2();
       bIsI2C2_enable = TRUE;
}

void TWI2_Deinitialize( void )
{
       sEE_LowLevel_DeInit2( );
       bIsI2C2_enable = FALSE;
}


bool I2C2_writeSlave( byte control, uint16 address, const byte *data, uint16 length, bool longAddr ) 
{
 	int status = I2C_SUCCESS;
	uint8 len = length;
	const uint8 *pbuff;

	if ( bIsI2C2_enable != TRUE )
	{
		TRACE_ERROR((0, "I2C2 interface has not been enable !!"));
		return FALSE;
	}
	
	pbuff = data;

	status = __STM32_I2C2_writeSlave(control, address, pbuff, &len, longAddr);

	/*Don't remove.*/
    vTaskDelay( TASK_MSEC2TICKS(I2C_INTERVAL_TIME) );

	if ( status != sEE_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}

bool I2C2_writeSlave_NoSystemWait( byte control, uint16 address, const byte *data, uint16 length, bool longAddr ) 
{
    int status = I2C_SUCCESS;
    uint8 len = length;
    const uint8 *pbuff;

    if ( bIsI2C2_enable != TRUE )
    {
        TRACE_ERROR((0, "I2C2 interface has not been enable !!"));
        return FALSE;
    }
    
    pbuff = data;

    status = __STM32_I2C2_writeSlave( control, address, pbuff, &len, longAddr );

    if ( status != sEE_OK)
    {
        return FALSE;
    }
    
    return TRUE;
}

bool I2C2_readSlave( byte control, uint16 address, byte *data, uint16 length, bool longAddr ) 
{
	int status = I2C_SUCCESS;
	uint16 NumByteToRead = length;
	uint8 *pBuffer;

	if ( bIsI2C2_enable != TRUE )
	{
		TRACE_ERROR((0, "I2C2 interface has not been enable !!"));
		return FALSE;
	}

	pBuffer = data;

	status = __STM32_I2C2_readSlave( control, address, pBuffer, &NumByteToRead, longAddr );

    vTaskDelay( TASK_MSEC2TICKS(5) );

	if ( status != sEE_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}
