#include "freertos_conf.h"
#include "freertos_task.h"
#include "Debug.h"

#include "SOUND_BAR_V1_PinDefinition.h"
#include "config.h"
#include "I2C2LowLevel.h"
#include "I2C1LowLevel.h"


//_______________________________________________________________________________________________________________________

typedef FlagStatus ( *i2c_get_flag_status )( I2C_TypeDef* I2Cx, uint32_t I2C_FLAG );

typedef ErrorStatus ( *i2c_check_event ) ( I2C_TypeDef* I2Cx, uint32_t I2C_EVENT );

//_______________________________________________________________________________________________________________________
// David, already defined in I2C1LowLevel.h.
//#define sEE_FLAG_TIMEOUT         ((uint32_t)0x3000)
//#define sEE_LONG_TIMEOUT         ((uint32_t)(300 * sEE_FLAG_TIMEOUT))

#define sEE_OK                    0
#define sEE_FAIL                  1

#define I2C_SLAVE_ADDRESS7      0
#define I2C_SPEED               88000

#define I2C_INTERVAL_TIME	5

#define DRV2_mutex 1
//_______________________________________________________________________________________________________________________
static bool bIsI2C2_enable = FALSE;

#if DRV2_mutex
#if 0 // David, fix warning.
static xSemaphoreHandle _DRV_I2C2_SEMA = NULL;
static portTickType _DRV_xI2C2_BlockTime = portMAX_DELAY;
#endif
#endif
//_______________________________________________________________________________________________________________________
static void sEE_Init2(void);
void sEE_LowLevel_DeInit2(void);

extern void ir_decode_wait(uint16 timeout);

//_______________________________________________________________________________________________________________________
static int err_cnt2 = 0;

uint32_t sEE_TIMEOUT_UserCallback2(uint8_t	DevAddr)
{
#if 0
	I2C_InitTypeDef  I2C_InitStructure;
#endif

        err_cnt2++;

#if 0
	//TRACE_ERROR((0, "Device Address 0x%X I2C interface timeout !!", DevAddr));
	
	//I2C_GenerateSTOP(sEE_I2C2, ENABLE);
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
#else
#if 1
 if(err_cnt2 % 50) 
 {
    // When error count over 50, we need to reset 9533 for relase SDA pin
    // TODO
#if 0
   GPIOMiddleLevel_Clr(__O_RST_HDMI_SIL953x);
   for(uint32_t i=0; i<0x5000; i++) __NOP();
   GPIOMiddleLevel_Set(__O_RST_HDMI_SIL953x);
#endif
 }
 
 (void) I2C2->DR;
 I2C2->CR1   &= ~I2C_CR1_ACK;
 
 TRACE_DEBUG((0,"2-1 : CR1 0x%X, CR2 0x%X,SR1 0x%X,SR2 0x%X,",I2C2->CR1,I2C2->CR2,I2C2->SR1,I2C2->SR2));

 uint32_t TRISE = I2C2->TRISE;
 uint32_t   CCR = I2C2->CCR;
 uint32_t  OAR1 = I2C2->OAR1;
 uint32_t   CR1 = I2C2->CR1;
 uint32_t   CR2 = I2C2->CR2;

 RCC->APB1RSTR |=  RCC_APB1RSTR_I2C2RST;
 for(uint32_t i=0; i<0x500; i++) __NOP();
 RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;

 I2C2->CR1 |=  I2C_CR1_SWRST;
 for(uint32_t i=0; i<0x500; i++) __NOP();
 I2C2->CR1 &= ~I2C_CR1_SWRST;
  
 I2C2->TRISE = TRISE;
 I2C2->CCR   = CCR;
 I2C2->OAR1  = OAR1;
 I2C2->CR2   = CR2;
 I2C2->CR1   = 0x401; //CR1|I2C_CR1_ACK;
 
 (void) I2C2->SR1;
 (void) I2C2->SR2;
 //GPIOMiddleLevel_Clr(__O_SP_TEST_PE1);
 
 TRACE_DEBUG((0,"2-2 : CR1 0x%X, CR2 0x%X,SR1 0x%X,SR2 0x%X,",I2C2->CR1,I2C2->CR2,I2C2->SR1,I2C2->SR2));
#endif

#endif
        __enable_irq();//123

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

#if DRV2_mutex
#if 0 // David, fix warning.
static bool _DRV_I2C2_sema_mutex_create(void)
{
    if (_DRV_I2C2_SEMA == NULL)
    {
        _DRV_I2C2_SEMA  = xSemaphoreCreateMutex();

        if (_DRV_I2C2_SEMA)
        {
            _DRV_xI2C2_BlockTime = portMAX_DELAY;
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

static bool _DRV_I2C2_sema_mutex_take(void)
{
#if DRV2_mutex
    if (_DRV_I2C2_sema_mutex_create())
    {
        //take mutex for blocking
        if ( xSemaphoreTake( _DRV_I2C2_SEMA, _DRV_xI2C2_BlockTime ) != pdTRUE )
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

static bool _DRV_I2C2_sema_mutex_give(void)
{
#if DRV2_mutex
    if (_DRV_I2C2_sema_mutex_create())
    {
        //give mutex for unblocking
        if ( xSemaphoreGive( _DRV_I2C2_SEMA) != pdTRUE )
        {
            //TRACE_DEBUG((0, "App_I2C2_sema_mutex_give busy 3333 !! "));
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
#endif

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
      uint32_t ret = sEE_OK;

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
				/* Send STRAT condition */ 
				//I2C_GenerateSTART(sEE_I2C2, ENABLE);
                                sEE_I2C2->CR1 |= ((uint16_t)0x0100);
                          
				//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000001, sEE_FLAG_TIMEOUT );
			}
				break;

			case 2:
			{
				/* Send device address for read */ 
				I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Transmitter); 
				//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000002, sEE_FLAG_TIMEOUT );
				if ( IsTimeout == FALSE )
				{
					/* Clear EV6 by setting again the PE bit */
					I2C_Cmd(sEE_I2C2, ENABLE);
				}
				else
				{
					//TRACE_ERROR((0, "Assert failed %s[%d]", __FUNCTION__, __LINE__));
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
					//I2C_GenerateSTART(sEE_I2C2, ENABLE);
                                        sEE_I2C2->CR1 |= ((uint16_t)0x0100);
                                        
					//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000001, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send device address for read */
					I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Receiver);
					//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000002, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

				}
				else 
				{
					
					/* Send the device's internal address to read to */
					I2C_SendData(sEE_I2C2, (uint8_t)(ReadAddr & 0x00FF));
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_BYTE_TRANSMITTED, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}
					

					/* Send STRAT condition a second time */  
					//I2C_GenerateSTART(sEE_I2C2, ENABLE);
                                        sEE_I2C2->CR1 |= ((uint16_t)0x0100);
                                        
					//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000001, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

					/* Send device address for read */
					I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Receiver);
					//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
					IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000002, sEE_FLAG_TIMEOUT );
					if ( IsTimeout == TRUE )
					{
						break;	
					}

				}

			}
				break;

			case 4:
			{
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
                                      __disable_irq();	
						*pBuffer = I2C_ReceiveData(sEE_I2C2);
                                      __enable_irq();

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
			//TRACE_ERROR((0, "__STM32_I2C2_readSlave state_mach = %d", state_mach ));
			
			TRACE_ERROR((0, "rerr2 %d", state_mach ));
			break;
		}
	}
	
	 /* Re-enable the interrupt mechanism */
	//__enable_irq();

	if ( IsTimeout == TRUE )
	{
		ret= (sEE_TIMEOUT_UserCallback2(DevAddr));
	}

	return ret;
		
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
      uint32_t ret = sEE_OK;


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
				//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_MODE_SELECT, sEE_FLAG_TIMEOUT );
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000001, sEE_FLAG_TIMEOUT );
			}
				break;

			case 2:
			{
				I2C_Send7bitAddress(sEE_I2C2, DevAddr, I2C_Direction_Transmitter);
				//IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, sEE_FLAG_TIMEOUT );
				IsTimeout = i2c2_event_timeout_check( I2C_CheckEvent, sEE_I2C2, 0x00000002, sEE_FLAG_TIMEOUT );
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
				//(void)sEE_I2C2->SR1;
				//(void)sEE_I2C2->SR2;
			}
				break;
		}

		if ( IsTimeout == TRUE )
		{
			//TRACE_ERROR((0, "__STM32_I2C2_writeSlave state_mach = %d", state_mach ));
			TRACE_ERROR((0, "werr2 %d", state_mach ));
			break;
		}

	}

	/* Re-enable the interrupt mechanism */
	//__enable_irq();


	if ( IsTimeout == TRUE )
	{
		ret = (sEE_TIMEOUT_UserCallback2(DevAddr));
	}
	

	/* If all operations OK, return sEE_OK (0) */
	return ret;
}


static void sEE_LowLevel_Init2(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
        

	/*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
	RCC_APB2PeriphClockCmd(sEE_I2C2_SCL_GPIO_CLK | sEE_I2C2_SDA_GPIO_CLK, ENABLE);

	/*!< sEE_I2C2 Periph clock enable */
	RCC_APB1PeriphClockCmd(sEE_I2C2_CLK, ENABLE);

	/*!< GPIO configuration */  
	/*!< Configure sEE_I2C2 pins: SCL */
	GPIO_InitStructure.GPIO_Pin = sEE_I2C2_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//GPIO_Mode_Out_PP;//GPIO_Mode_AF_PP;//GPIO_Mode_AF_OD;
	GPIO_Init(sEE_I2C2_SCL_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure sEE_I2C2 pins: SDA */
	GPIO_InitStructure.GPIO_Pin = sEE_I2C2_SDA_PIN;
	GPIO_Init(sEE_I2C2_SDA_GPIO_PORT, &GPIO_InitStructure); 

}

void sEE_LowLevel_DeInit2(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 

  /* sEE_I2C2 Peripheral Disable */
  I2C_Cmd(sEE_I2C2, DISABLE);
 
  /* sEE_I2C2 DeInit */
  I2C_DeInit(sEE_I2C2);

  /*!< sEE_I2C2 Periph clock disable */
  RCC_APB1PeriphClockCmd(sEE_I2C2_CLK, DISABLE);
    
  /*!< GPIO configuration */  
  /*!< Configure sEE_I2C2 pins: SCL */
  GPIO_InitStructure.GPIO_Pin = sEE_I2C2_SCL_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(sEE_I2C2_SCL_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sEE_I2C2 pins: SDA */
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
	
#if DRV2_mutex
    if (DRV_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }
#endif
	
	pbuff = data;

	status = __STM32_I2C2_writeSlave(control, address, pbuff, &len, longAddr);

#if DRV2_mutex
    DRV_I2C1_sema_mutex_give();
#endif

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
    
#if DRV2_mutex
    if (DRV_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }
#endif
    
    pbuff = data;

    status = __STM32_I2C2_writeSlave( control, address, pbuff, &len, longAddr );

#if DRV2_mutex
    DRV_I2C1_sema_mutex_give();
#endif

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

    ir_decode_wait(100);
    
#if DRV2_mutex
    if (DRV_I2C1_sema_mutex_take() != TRUE)
    {
        return FALSE;
    }
#endif

	pBuffer = data;

	status = __STM32_I2C2_readSlave( control, address, pBuffer, &NumByteToRead, longAddr );

#if DRV2_mutex
    DRV_I2C1_sema_mutex_give();
#endif

	if ( status != sEE_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}
