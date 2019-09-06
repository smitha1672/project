#include "I2C1LowLevel.h"
//#include "SOUND_BAR_V1_PinDefinition.h"
//#include "config.h"
//#include "Debug.h"
//#include "CardLowLevel_drve_conf.h"



//! < Definition	@{
/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define sEE_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define sEE_LONG_TIMEOUT         ((uint32_t)(10 * sEE_FLAG_TIMEOUT))

#define sEE_OK                    0
#define sEE_FAIL                  1

#define I2C_SLAVE_ADDRESS7      0
#define I2C_SPEED               90000

#define USE_DEFAULT_TIMEOUT_CALLBACK
#define USE_DEFAULT_CRITICAL_CALLBACK 

#define I2C_INTERVAL_TIME	5
//!	@}


//! < Private variable	@{
static DMA_InitTypeDef   sEEDMA_InitStructure; 
static __IO uint32_t  sEETimeout = sEE_LONG_TIMEOUT;   
static __IO uint16_t* sEEDataReadPointer;   
static __IO uint8_t*  sEEDataWritePointer;  

static bool bIsI2C_enable = FALSE;
//!	@}
static void sEE_Init(void);
static void sEE_LowLevel_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction);

/*______________________________________________________________________________________________*/
#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t sEE_TIMEOUT_UserCallback(uint8_t	DevAddr)
{
	//TRACE_ERROR((0, "Device Address 0x%X I2C interface timeout !!", DevAddr));
	I2C_SoftwareResetCmd(sEE_I2C,ENABLE); 
    I2C_SoftwareResetCmd(sEE_I2C,DISABLE); 
    sEE_Init();
	return sEE_FAIL;

	/* Block communication and all processes */
	//while (1){}
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

static void sEE_LowLevel_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction)
{ 
  /* Initialize the DMA with the new parameters */
  if (Direction == sEE_DIRECTION_TX)
  {
    /* Configure the DMA Tx Channel with the buffer address and the buffer size */
    sEEDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
    sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    
    sEEDMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;  
    DMA_Init(sEE_I2C_DMA_CHANNEL_TX, &sEEDMA_InitStructure);  
  }
  else
  { 
    /* Configure the DMA Rx Channel with the buffer address and the buffer size */
    sEEDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
    sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    sEEDMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;      
    DMA_Init(sEE_I2C_DMA_CHANNEL_RX, &sEEDMA_InitStructure);    
  }
  
}

static uint32_t __STM32_I2C_readSlave(	uint8_t	DevAddr,
												uint16_t ReadAddr, 
												uint8_t *pBuffer, 
												uint16_t *NumByteToRead, 
												bool longAddr)
{ 

	/* Set the pointer to the Number of data to be read. This pointer will be used 
	by the DMA Transfer Completer interrupt Handler in order to reset the 
	variable to 0. User should check on this variable in order to know if the 
	DMA transfer has been complete or not. */
	sEEDataReadPointer = NumByteToRead;
  
	/*!< While the bus is busy */
	sEETimeout = sEE_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BUSY))
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
	}
  
	/*!< Send START condition */
	I2C_GenerateSTART(sEE_I2C, ENABLE);

	/*!< Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
	sEETimeout = sEE_FLAG_TIMEOUT;
	while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
	}
  
	/*!< Send EEPROM address for write */
	I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter);

	/*!< Test on EV6 and clear it */
	sEETimeout = sEE_FLAG_TIMEOUT;
	while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
	} 


	//!	Send deive internal register address;	smith@{
	if ( longAddr == TRUE )
	{
		/*!< Send the EEPROM's internal address to read from: MSB of the address first */
		I2C_SendData(sEE_I2C, (uint8_t)((ReadAddr & 0xFF00) >> 8));	 

		/*!< Test on EV8 and clear it */
		sEETimeout = sEE_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
		}

		/*!< Send the EEPROM's internal address to read from: LSB of the address */
		I2C_SendData(sEE_I2C, (uint8_t)(ReadAddr & 0x00FF));	  
	}
	else
	{
		/*!< Send the EEPROM's internal address to read from: Only one byte address */
		I2C_SendData(sEE_I2C, ReadAddr);

	}
	/*!< Test on EV8 and clear it */
	sEETimeout = sEE_FLAG_TIMEOUT;
	while(I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BTF) == RESET)
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
	}
	//!	@}
	
	/*!< Send STRAT condition a second time */  
	I2C_GenerateSTART(sEE_I2C, ENABLE);

	/*!< Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
	sEETimeout = sEE_FLAG_TIMEOUT;
	while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
	} 

	/*!< Send EEPROM address for read */
	I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Receiver);  
  
  /* If number of data to be read is 1, then DMA couldn't be used */
  /* One Byte Master Reception procedure (POLLING) ---------------------------*/
	if ((uint16_t)(*NumByteToRead) < 2)
	{
		/* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
		sEETimeout = sEE_FLAG_TIMEOUT;
		while(I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_ADDR) == RESET)
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
		}	  

		/*!< Disable Acknowledgement */
		I2C_AcknowledgeConfig(sEE_I2C, DISABLE);   

		/* Call User callback for critical section start (should typically disable interrupts) */
		sEE_EnterCriticalSection_UserCallback();

		/* Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read) */
		(void)sEE_I2C->SR2;

		/*!< Send STOP Condition */
		I2C_GenerateSTOP(sEE_I2C, ENABLE);

		/* Call User callback for critical section end (should typically re-enable interrupts) */
		sEE_ExitCriticalSection_UserCallback();

		/* Wait for the byte to be received */
		sEETimeout = sEE_FLAG_TIMEOUT;
		while(I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_RXNE) == RESET)
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
		}

		/*!< Read the byte received from the EEPROM */
		*pBuffer = I2C_ReceiveData(sEE_I2C);

		/*!< Decrement the read bytes counter */
		(uint16_t)(*NumByteToRead)--;		 

		/* Wait to make sure that STOP control bit has been cleared */
		sEETimeout = sEE_FLAG_TIMEOUT;
		while(sEE_I2C->CR1 & I2C_CR1_STOP)
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
		}  

		/*!< Re-Enable Acknowledgement to be ready for another reception */
		I2C_AcknowledgeConfig(sEE_I2C, ENABLE);    
	}
	else/* More than one Byte Master Reception procedure (DMA) -----------------*/
	{
		/*!< Test on EV6 and clear it */
		sEETimeout = sEE_FLAG_TIMEOUT;
		while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);
		}  

		/* Configure the DMA Rx Channel with the buffer address and the buffer size */
		sEE_LowLevel_DMAConfig((uint32_t)pBuffer, (uint16_t)(*NumByteToRead), sEE_DIRECTION_RX);

		/* Inform the DMA that the next End Of Transfer Signal will be the last one */
		I2C_DMALastTransferCmd(sEE_I2C, ENABLE); 

		/* Enable the DMA Rx Channel */
		DMA_Cmd(sEE_I2C_DMA_CHANNEL_RX, ENABLE);  
	}
  
	/* If all operations OK, return sEE_OK (0) */
	return sEE_OK;
}

static uint32_t __STM32_I2C_writeSlave(	uint8_t DevAddr, 
												uint16_t WriteAddr, 
												uint8_t *pBuffer, 
												uint8_t *NumByteToWrite, 
												bool longAddr)
{ 
	/* Set the pointer to the Number of data to be written. This pointer will be used 
	by the DMA Transfer Completer interrupt Handler in order to reset the 
	variable to 0. User should check on this variable in order to know if the 
	DMA transfer has been complete or not. */
	sEEDataWritePointer = NumByteToWrite;  
	
	/*!< While the bus is busy */
	sEETimeout = sEE_LONG_TIMEOUT;
	while(I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BUSY))
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);;
	}
  
	/*!< Send START condition */
	I2C_GenerateSTART(sEE_I2C, ENABLE);

	/*!< Test on EV5 and clear it */
	sEETimeout = sEE_FLAG_TIMEOUT;
	while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);;
	}
  
	/*!< Send EEPROM address for write */
	sEETimeout = sEE_FLAG_TIMEOUT;
	I2C_Send7bitAddress(sEE_I2C, DevAddr, I2C_Direction_Transmitter);

	/*!< Test on EV6 and clear it */
	sEETimeout = sEE_FLAG_TIMEOUT;
	while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);;
	}

	if ( longAddr == TRUE)
	{
		/*!< Send the EEPROM's internal address to write to : MSB of the address first */
		I2C_SendData(sEE_I2C, (uint8_t)((WriteAddr & 0xFF00) >> 8));

		/*!< Test on EV8 and clear it */
		sEETimeout = sEE_FLAG_TIMEOUT;  
		while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);;
		}  

		/*!< Send the EEPROM's internal address to write to : LSB of the address */
		I2C_SendData(sEE_I2C, (uint8_t)(WriteAddr & 0x00FF));


		/*!< Test on EV8 and clear it */
		sEETimeout = sEE_FLAG_TIMEOUT; 
		while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);;
		}  

	}
	else
	{
		/*!< Send the EEPROM's internal address to write to : only one byte Address */
		I2C_SendData(sEE_I2C, WriteAddr);

		while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);;
		}  

	}

#if 0
	for( index = 0; index < *sEEDataWritePointer; index ++ )
	{
		I2C_SendData(sEE_I2C, *(pBuffer+index));
		
		while(!I2C_CheckEvent(sEE_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if((sEETimeout--) == 0) return sEE_TIMEOUT_UserCallback(DevAddr);;
		}  
	}
#endif 


	

#if 1
	/* Configure the DMA Tx Channel with the buffer address and the buffer size */
	sEE_LowLevel_DMAConfig((uint32_t)pBuffer, (uint8_t)(*NumByteToWrite), sEE_DIRECTION_TX);

	/* Enable the DMA Tx Channel */
	DMA_Cmd(sEE_I2C_DMA_CHANNEL_TX, ENABLE);

#endif	

	/* If all operations OK, return sEE_OK (0) */
	return sEE_OK;
}

void sEE_I2C_DMA_TX_IRQHandler(void)
{
	/* Check if the DMA transfer is complete */ 
	if(DMA_GetFlagStatus(sEE_I2C_DMA_FLAG_TX_TC) != RESET)
	{  
		/* Disable the DMA Tx Channel and Clear all its Flags */  
		DMA_Cmd(sEE_I2C_DMA_CHANNEL_TX, DISABLE);
		DMA_ClearFlag(sEE_I2C_DMA_FLAG_TX_GL);

		/*!< Wait till all data have been physically transferred on the bus */
		sEETimeout = sEE_LONG_TIMEOUT;
		while(!I2C_GetFlagStatus(sEE_I2C, I2C_FLAG_BTF))
		{
			if((sEETimeout--) == 0) return;
		}

		/*!< Send STOP condition */
		I2C_GenerateSTOP(sEE_I2C, ENABLE);

		/* Perform a read on SR1 and SR2 register to clear eventualaly pending flags */
		(void)sEE_I2C->SR1;
		(void)sEE_I2C->SR2;

		/* Reset the variable holding the number of data to be written */
		*sEEDataWritePointer = 0;  
	}
}

void sEE_I2C_DMA_RX_IRQHandler(void)
{
	/* Check if the DMA transfer is complete */
	if(DMA_GetFlagStatus(sEE_I2C_DMA_FLAG_RX_TC) != RESET)
	{      
		/*!< Send STOP Condition */
		I2C_GenerateSTOP(sEE_I2C, ENABLE);    

		/* Disable the DMA Rx Channel and Clear all its Flags */  
		DMA_Cmd(sEE_I2C_DMA_CHANNEL_RX, DISABLE);
		DMA_ClearFlag(sEE_I2C_DMA_FLAG_RX_GL);

		/* Reset the variable holding the number of data to be read */
		*sEEDataReadPointer = 0;
	}
}

static void sEE_LowLevel_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;  

	/*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
	RCC_APB2PeriphClockCmd(sEE_I2C_SCL_GPIO_CLK | sEE_I2C_SDA_GPIO_CLK, ENABLE);

	/*!< sEE_I2C Periph clock enable */
	RCC_APB1PeriphClockCmd(sEE_I2C_CLK, ENABLE);

	/*!< GPIO configuration */  
	/*!< Configure sEE_I2C pins: SCL */
	GPIO_InitStructure.GPIO_Pin = sEE_I2C_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//GPIO_Mode_Out_PP;//GPIO_Mode_AF_PP;//GPIO_Mode_AF_OD;
	GPIO_Init(sEE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

	/*!< Configure sEE_I2C pins: SDA */
	GPIO_InitStructure.GPIO_Pin = sEE_I2C_SDA_PIN;
	GPIO_Init(sEE_I2C_SDA_GPIO_PORT, &GPIO_InitStructure); 


#if 1 /*mark DMA interrupt*/
	//! Amtran Implemented	@{
	/* Set the Vector Table base location at 0x8000; Don't remove */ 
	//NVIC_VECTOR_TABLE();
	//NVIC_PriorityGroupConfig(PRIORITY_GROP);	
	//!@}


	/* Configure and enable I2C DMA TX Channel interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure and enable I2C DMA RX Channel interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_RX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_Init(&NVIC_InitStructure);  

	/*!< I2C DMA TX and RX channels configuration */
	/* Enable the DMA clock */
	RCC_AHBPeriphClockCmd(sEE_I2C_DMA_CLK, ENABLE);

	/* I2C TX DMA Channel configuration */
	DMA_DeInit(sEE_I2C_DMA_CHANNEL_TX);
	sEEDMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)sEE_I2C_DR_Address;
	sEEDMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;   /* This parameter will be configured durig communication */
	sEEDMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    /* This parameter will be configured durig communication */
	sEEDMA_InitStructure.DMA_BufferSize = 0xFFFF;            /* This parameter will be configured durig communication */
	sEEDMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	sEEDMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	sEEDMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
	sEEDMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	sEEDMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	sEEDMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	sEEDMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(sEE_I2C_DMA_CHANNEL_TX, &sEEDMA_InitStructure);  

	/* I2C RX DMA Channel configuration */
	DMA_DeInit(sEE_I2C_DMA_CHANNEL_RX);
	DMA_Init(sEE_I2C_DMA_CHANNEL_RX, &sEEDMA_InitStructure);  

	/* Enable the DMA Channels Interrupts */
	DMA_ITConfig(sEE_I2C_DMA_CHANNEL_TX, DMA_IT_TC, ENABLE);
	DMA_ITConfig(sEE_I2C_DMA_CHANNEL_RX, DMA_IT_TC, ENABLE); 
#endif 

	
}

#if 0
void sEE_LowLevel_DeInit(void)
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
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(sEE_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = sEE_I2C_SDA_PIN;
  GPIO_Init(sEE_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);
  
  /* Configure and enable I2C DMA TX Channel interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_TX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = sEE_I2C_DMA_PREPRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = sEE_I2C_DMA_SUBPRIO;
  NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Configure and enable I2C DMA RX Channel interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = sEE_I2C_DMA_RX_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = sEE_I2C_DMA_PREPRIO;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = sEE_I2C_DMA_SUBPRIO;
  NVIC_Init(&NVIC_InitStructure);   
  
  /* Disable and Deinitialize the DMA channels */
  DMA_Cmd(sEE_I2C_DMA_CHANNEL_TX, DISABLE);
  DMA_Cmd(sEE_I2C_DMA_CHANNEL_RX, DISABLE);
  DMA_DeInit(sEE_I2C_DMA_CHANNEL_TX);
  DMA_DeInit(sEE_I2C_DMA_CHANNEL_RX);
}
#endif   


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
	
	/* Enable the sEE_I2C peripheral DMA requests */
	I2C_DMACmd(sEE_I2C, ENABLE);

}


/*__________________________________________________________________________________________________________*/
void TWI_initialize( void )
{
	sEE_Init();  
	bIsI2C_enable = TRUE;
}

void TWI_Deinitialize( void )
{
	//sEE_LowLevel_DeInit( );
	bIsI2C_enable = FALSE;
}

bool I2C_writeSlave(byte control, uint16 address, byte *data, uint16 length, bool longAddr) 
{
 	int status = I2C_SUCCESS;
	uint8 len = length;
	uint8 *pbuff;

	if ( bIsI2C_enable != TRUE )
	{
		//TRACE_ERROR((0, "I2C interface has not been enable !!"));
		return FALSE;
	}
	
	pbuff = data;

	status = __STM32_I2C_writeSlave(control, address, pbuff, &len, longAddr);

	/*Don't remove.*/
	//VirtualTimer_sleep(I2C_INTERVAL_TIME);

	if ( status != sEE_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}

bool I2C_readSlave(	byte control, 
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
		//TRACE_ERROR((0, "I2C interface has not been enable !!"));
		return FALSE;
	}

	pBuffer = data;

	status = __STM32_I2C_readSlave(control, address, pBuffer, &NumByteToRead, longAddr);

	/*Don't remove.*/
	//VirtualTimer_sleep(I2C_INTERVAL_TIME);

	if ( status != sEE_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}

bool i2c_write_byte(byte dev_addr_in, uint16 reg_addr_in, byte value)
{
	byte data=value;

	if (I2C_writeSlave(dev_addr_in, reg_addr_in, &data, 1, FALSE) != I2C_SUCCESS)
		return FALSE;

	return TRUE;
}

int i2c_read_byte(byte dev_addr_in, uint16 reg_addr_in) 
{
	byte ret=0;

	if (I2C_readSlave(dev_addr_in, reg_addr_in, &ret, 1, FALSE) != I2C_SUCCESS)
	{
		//TRACE_ERROR((STR_ID_NULL, "i2c_read_byte fail"));
		return I2C_INVALID_ARGUMENT;
	}

	return ret;
}


bool i2c_write_block(byte dev_addr_in, uint16 reg_addr_in, byte *data_in, int count_in) 
{
	if (I2C_writeSlave(dev_addr_in, reg_addr_in, data_in, count_in, FALSE) != I2C_SUCCESS)
		return FALSE;

	return TRUE;
}

bool i2c_read_block(byte dev_addr_in, uint16 reg_addr_in, byte *data_out, int count_in) 
{
	if (I2C_readSlave(dev_addr_in, reg_addr_in, data_out, count_in, FALSE) != I2C_SUCCESS)
		return FALSE;

	return TRUE;
}


